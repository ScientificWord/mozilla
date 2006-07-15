/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is the Mozilla OS/2 libraries.
 *
 * The Initial Developer of the Original Code is
 * John Fairhurst, <john_fairhurst@iname.com>.
 * Portions created by the Initial Developer are Copyright (C) 1999
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either of the GNU General Public License Version 2 or later (the "GPL"),
 * or the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK *****
 *
 * This Original Code has been modified by IBM Corporation. Modifications made by IBM 
 * described herein are Copyright (c) International Business Machines Corporation, 2000.
 * Modifications to Mozilla code or documentation identified per MPL Section 3.3
 *
 * Date             Modified by     Description of modification
 * 05/11/2000       IBM Corp.      Make it look more like Windows.
 */

#include "nsGfxDefs.h"
#include <stdlib.h>

#include "nsImageOS2.h"
#include "nsRenderingContextOS2.h"
#include "nsDeviceContextOS2.h"
#include "imgScaler.h"

#define MAX_BUFFER_WIDTH        128
#define MAX_BUFFER_HEIGHT       128

#ifdef XP_OS2_VACPP
// Needed to zero the stack since there is an uninitialized var in the engine
// If you find that printing bitmaps with transparency results in black images then
// take off the # define and use for all OS2 compilers
// This was discovered when the temp bitmap bit depth used in images2::Draw was changed
// from 8 to 24.  It worked in debug but failed in retail.  Investigation revealed GpiErase
// was failing due to a function receiving an uninitialized variable and the stack had invalid
// values (for retail). This function will zero the stack and return leaving a clean stack.
// A parameter is used to ensure the optimizer will compile the code.  Make sure the parameter
// is not larger than the buff size
// The OS2 defect 272592 will fix this in future releases of the engine
extern "C" int zeroStack(int index)
{
  #define CLEAR_BUF_SIZE 1024
  BYTE buf[CLEAR_BUF_SIZE];
  memset(buf, 0, CLEAR_BUF_SIZE * sizeof(BYTE));
  return buf[index];
}
#else
#define zeroStack(x)
#endif

struct MONOBITMAPINFO
{
   BITMAPINFOHEADER2 bmpInfo;
   RGB2 argbColor [2];

   operator PBITMAPINFO2 ()       { return (PBITMAPINFO2) &bmpInfo; }
   operator PBITMAPINFOHEADER2 () { return &bmpInfo; }

   MONOBITMAPINFO( PBITMAPINFO2 pBI)
   {
      memcpy( &bmpInfo, pBI, sizeof( BITMAPINFOHEADER2));
      bmpInfo.cBitCount = 1;

      argbColor [0].bRed      = 0;
      argbColor [0].bGreen    = 0;
      argbColor [0].bBlue     = 0;
      argbColor [0].fcOptions = 0;
      argbColor [1].bRed      = 255;
      argbColor [1].bGreen    = 255;
      argbColor [1].bBlue     = 255;
      argbColor [1].fcOptions = 0;
   }
};


PRUint8 nsImageOS2::gBlenderLookup [65536];    // Global table for fast alpha blending
PRBool  nsImageOS2::gBlenderReady = PR_FALSE;



NS_IMPL_ISUPPORTS1(nsImageOS2, nsIImage)

//------------------------------------------------------------
nsImageOS2::nsImageOS2()
: mInfo(0)
, mDeviceDepth(0)
, mRowBytes(0)
, mImageBits(0)
, mIsOptimized(PR_FALSE)
, mColorMap(0)
, mDecodedRect()
, mAlphaBits(0)
, mAlphaDepth(0)
, mARowBytes(0)
{
   if (gBlenderReady != PR_TRUE)
     BuildBlenderLookup ();
}

nsImageOS2::~nsImageOS2()
{
   CleanUp(PR_TRUE);
}

nsresult nsImageOS2::Init( PRInt32 aWidth, PRInt32 aHeight, PRInt32 aDepth,
                           nsMaskRequirements aMaskRequirements)
{
   // gfxIImageFrame only allows one init of nsImageOS2

   // (copying windows code - what about monochrome?  Oh well.)
   NS_ASSERTION( aDepth == 24 || aDepth == 8, "Bad image depth");

   // Work out size of bitmap to allocate
   mRowBytes = RASWIDTH(aWidth,aDepth);

   mImageBits = new PRUint8 [ aHeight * mRowBytes ];

   // Set up bitmapinfo header
   int cols = -1;
   if( aDepth == 8) cols = COLOR_CUBE_SIZE;
   else if( aDepth <= 32) cols = 0;

   int szStruct = sizeof( BITMAPINFOHEADER2) + cols * sizeof( RGB2);

   mInfo = (PBITMAPINFO2) calloc( szStruct, 1);
   mInfo->cbFix = sizeof( BITMAPINFOHEADER2);
   mInfo->cx = aWidth;
   mInfo->cy = aHeight;
   mInfo->cPlanes = 1;
   mInfo->cBitCount = (USHORT) aDepth;

   // We can't set up the bitmap colour table yet.

   // init color map.
   // XP will update the color map & then call ImageUpdated(), at which
   // point we can change the color table in the bitmapinfo.
   if( aDepth == 8)
   {
      mColorMap = new nsColorMap;
      mColorMap->NumColors = COLOR_CUBE_SIZE;
      mColorMap->Index = new PRUint8[3 * mColorMap->NumColors];
   }

   // Allocate stuff for mask bitmap
   if( aMaskRequirements != nsMaskRequirements_kNoMask)
   {
      if( aMaskRequirements == nsMaskRequirements_kNeeds1Bit)
      {
         mAlphaDepth = 1;
      }
      else
      {
         NS_ASSERTION( nsMaskRequirements_kNeeds8Bit == aMaskRequirements,
                       "unexpected mask depth");
         mAlphaDepth = 8;
      }

      // 32-bit align each row
      mARowBytes = RASWIDTH (aWidth, mAlphaDepth);

      mAlphaBits = new PRUint8 [ aHeight * mARowBytes];
   }

   return NS_OK;
}

void nsImageOS2::CleanUp(PRBool aCleanUpAll)
{
   // OS2TODO to handle aCleanUpAll param

   if( mImageBits) {
      delete [] mImageBits; 
      mImageBits = 0;
   }
   if( mInfo) {
      free( mInfo);
      mInfo = 0;
   }
   if( mColorMap) {
      if( mColorMap->Index)
         delete [] mColorMap->Index;
      delete mColorMap;
      mColorMap = 0;
   }
   if( mAlphaBits) {
      delete [] mAlphaBits; 
      mAlphaBits = 0;
   }
}

void nsImageOS2::ImageUpdated( nsIDeviceContext *aContext,
                               PRUint8 aFlags, nsRect *aUpdateRect)
{
   mDecodedRect.UnionRect(mDecodedRect, *aUpdateRect);

   if (!aContext) {
      return;
   } /* endif */
   // This is where we can set the bitmap colour table, as the XP code
   // has filled in the colour map.  It would be cute to be able to alias
   // the bitmap colour table as the mColorMap->Index thing, but the formats
   // are unfortunately different.  Rats.

   aContext->GetDepth( mDeviceDepth);

   if( (aFlags & nsImageUpdateFlags_kColorMapChanged) && mInfo->cBitCount == 8)
   {
      PRGB2 pBmpEntry  = mInfo->argbColor;
      PRUint8 *pMapByte = mColorMap->Index;

      for( PRInt32 i = 0; i < mColorMap->NumColors; i++, pBmpEntry++)
      {
         pBmpEntry->bRed   = *pMapByte++;
         pBmpEntry->bGreen = *pMapByte++;
         pBmpEntry->bBlue  = *pMapByte++;
      }
   }
   else if( aFlags & nsImageUpdateFlags_kBitsChanged)
   {
      // jolly good...
   }
}

/** ---------------------------------------------------
 *  See documentation in nsIImage.h
 */
PRBool nsImageOS2::GetIsImageComplete() {
  return mInfo &&
         mDecodedRect.x == 0 &&
         mDecodedRect.y == 0 &&
         mDecodedRect.width == mInfo->cx &&
         mDecodedRect.height == mInfo->cy;
}

void nsImageOS2::BuildBlenderLookup (void)
{
  for (int y = 0 ; y < 256 ; y++)
    for (int x = 0 ; x < 256 ; x++)
      gBlenderLookup [y * 256 + x] = y * x / 255;

  gBlenderReady = PR_TRUE;
}

nsresult nsImageOS2::Draw( nsIRenderingContext &aContext,
                           nsIDrawingSurface* aSurface,
                           PRInt32 aX, PRInt32 aY,
                           PRInt32 aWidth, PRInt32 aHeight)
{
   return Draw( aContext, aSurface,
                0, 0, mInfo->cx, mInfo->cy,
                aX, aY, aWidth, aHeight);
}

/** ---------------------------------------------------
 *  This is a helper routine to do the blending for the Draw method
 */
void nsImageOS2 :: DrawComposited24(unsigned char *aBits,
			     PRUint8 *aImageRGB, PRUint32 aStrideRGB,
			     PRUint8 *aImageAlpha, PRUint32 aStrideAlpha,
			     int aWidth, int aHeight)
{
  PRInt32 targetRowBytes = ((aWidth * 3) + 3) & ~3;

  for (int y = 0; y < aHeight; y++) {
    unsigned char *targetRow = aBits + y * targetRowBytes;
    unsigned char *imageRow = aImageRGB + y * aStrideRGB;
    unsigned char *alphaRow = aImageAlpha + y * aStrideAlpha;

    for (int x = 0; x < aWidth;
         x++, targetRow += 3, imageRow += 3, alphaRow++) {
      unsigned alpha = *alphaRow;
      MOZ_BLEND(targetRow[0], targetRow[0], imageRow[0], alpha);
      MOZ_BLEND(targetRow[1], targetRow[1], imageRow[1], alpha);
      MOZ_BLEND(targetRow[2], targetRow[2], imageRow[2], alpha);
    }
  }
}

NS_IMETHODIMP 
nsImageOS2 :: Draw(nsIRenderingContext &aContext, nsIDrawingSurface* aSurface,
                  PRInt32 aSX, PRInt32 aSY, PRInt32 aSWidth, PRInt32 aSHeight,
                  PRInt32 aDX, PRInt32 aDY, PRInt32 aDWidth, PRInt32 aDHeight)
{
  nsresult rv = NS_OK;

   PRInt32 origSHeight = aSHeight, origDHeight = aDHeight;
   PRInt32 origSWidth = aSWidth, origDWidth = aDWidth;

   if (mInfo == nsnull || aSWidth < 0 || aDWidth < 0 || aSHeight < 0 || aDHeight < 0) 
      return NS_ERROR_FAILURE;

   if (0 == aSWidth || 0 == aDWidth || 0 == aSHeight || 0 == aDHeight)
      return NS_OK;

   // limit the size of the blit to the amount of the image read in
   PRInt32 aSX2 = aSX + aSWidth;

   if (aSX2 > mDecodedRect.XMost()) 
      aSX2 = mDecodedRect.XMost();

   if (aSX < mDecodedRect.x) {
      aDX += (mDecodedRect.x - aSX) * origDWidth / origSWidth;
      aSX = mDecodedRect.x;
   }
  
   aSWidth = aSX2 - aSX;
   aDWidth -= (origSWidth - aSWidth) * origDWidth / origSWidth;
  
   if (aSWidth <= 0 || aDWidth <= 0)
      return NS_OK;

   PRInt32 aSY2 = aSY + aSHeight;

   if (aSY2 > mDecodedRect.YMost())
      aSY2 = mDecodedRect.YMost();

   if (aSY < mDecodedRect.y) {
      aDY += (mDecodedRect.y - aSY) * origDHeight / origSHeight;
      aSY = mDecodedRect.y;
   }

   aSHeight = aSY2 - aSY;
   aDHeight -= (origSHeight - aSHeight) * origDHeight / origSHeight;

   if (aSHeight <= 0 || aDHeight <= 0)
      return NS_OK;

   nsDrawingSurfaceOS2 *surf = (nsDrawingSurfaceOS2*) aSurface;
 

   nsRect trect( aDX, aDY, aDWidth, aDHeight);
   RECTL  rcl;
   surf->NS2PM_ININ (trect, rcl);

   // Set up blit coord array
   POINTL aptl[ 4] = { { rcl.xLeft, rcl.yBottom },              // TLL
                       { rcl.xRight, rcl.yTop },                // TUR
                       { aSX, mInfo->cy - (aSY + aSHeight) },   // SLL
                       { aSX + aSWidth, mInfo->cy - aSY } };    // SUR
                       
   PRBool fPrinting = PR_FALSE;
   nsIDeviceContext*  context;
   aContext.GetDeviceContext(context);
   if (((nsDeviceContextOS2 *)context)->mPrintDC) {
      fPrinting = PR_TRUE;
   }

   if( mAlphaDepth == 0)
   {
      // no transparency, just blit it
      GFX (::GpiDrawBits (surf->GetPS (), mImageBits, mInfo, 4, aptl, ROP_SRCCOPY, BBO_IGNORE), GPI_ERROR);
   }
   else if( mAlphaDepth == 1)
   {
      if (!fPrinting) {
         // > The transparent areas of the mask are coloured black (0).
         // > the transparent areas of the pixmap are coloured black (0).
         // > Note this does *not* mean that all black pels are transparent!
         // >
         // > Thus all we need to do is AND the mask onto the target, taking
         // > out pels that are not transparent, and then OR the image onto
         // > the target.
         // >
         // > For monochrome bitmaps GPI replaces 1 with IMAGEBUNDLE foreground
         // > color and 0 with background color. To make this work with ROP_SRCAND
         // > we set foreground to black and background to white. Thus AND with 
         // > 1 (opaque) in mask maps to AND with IMAGEBUNDLE foreground (which is 0)
         // > always gives 0 - clears opaque region to zeros. 
   
         // Apply mask to target, clear pels we will fill in from the image
         MONOBITMAPINFO MaskBitmapInfo (mInfo);
         GFX (::GpiDrawBits (surf->GetPS (), mAlphaBits, MaskBitmapInfo, 4, aptl, ROP_SRCAND, BBO_IGNORE), GPI_ERROR);
   
         // Now combine image with target
         GFX (::GpiDrawBits (surf->GetPS (), mImageBits, mInfo, 4, aptl, ROP_SRCPAINT, BBO_IGNORE), GPI_ERROR);
      } else {
         // Find the compatible device context and create a memory one
         HDC hdcCompat = GFX (::GpiQueryDevice (surf->GetPS ()), HDC_ERROR);
   
         rv = NS_ERROR_FAILURE;
       
         // create non-inclusive rect for GpiBitBlt
         RECTL dest;
         surf->NS2PM_INEX (trect, dest);
   
         DEVOPENSTRUC dop = { 0, 0, 0, 0, 0 };
         HDC MemDC = ::DevOpenDC( (HAB)0, OD_MEMORY, "*", 5, (PDEVOPENDATA) &dop, hdcCompat);
   
         if( MemDC != DEV_ERROR )
         {   
           // create the PS
           SIZEL sizel = { 0, 0 };
           HPS MemPS = GFX (::GpiCreatePS (0, MemDC, &sizel, PU_PELS | GPIT_MICRO | GPIA_ASSOC), GPI_ERROR);
   
           if( MemPS != GPI_ERROR )
           {
             GFX (::GpiCreateLogColorTable (MemPS, 0, LCOLF_RGB, 0, 0, 0), FALSE);
   
             // now create a bitmap of the right size
             HBITMAP hMemBmp;
             BITMAPINFOHEADER2 bihMem = { 0 };
   
             bihMem.cbFix = sizeof (BITMAPINFOHEADER2);
             bihMem.cx = aSWidth;
             bihMem.cy = aSHeight;
             bihMem.cPlanes = 1;
             LONG lBitCount = 0;
             GFX (::DevQueryCaps( hdcCompat, CAPS_COLOR_BITCOUNT, 1, &lBitCount), FALSE);
             lBitCount = 24; // For printing
             bihMem.cBitCount = lBitCount;
             
             hMemBmp = GFX (::GpiCreateBitmap (MemPS, &bihMem, 0, 0, 0), GPI_ERROR);
   
             if( hMemBmp != GPI_ERROR )
             {
               GFX (::GpiSetBitmap (MemPS, hMemBmp), HBM_ERROR);
               zeroStack(10);
               GpiErase(MemPS);
   
               // Now combine image with target
               // Set up blit coord array
               POINTL aptlNew[ 4] = { { 0, 0 },              // TLL
                                      { bihMem.cx, bihMem.cy },                // TUR
                                      { aSX, mInfo->cy - (aSY + aSHeight) },   // SLL
                                      { aSX + aSWidth+1, mInfo->cy - aSY+1 } };    // SUR
   
               // Apply mask to target, clear pels we will fill in from the image
               MONOBITMAPINFO MaskBitmapInfo (mInfo);                                   
               GFX (::GpiDrawBits (MemPS, mAlphaBits, MaskBitmapInfo, 4, aptlNew, ROP_SRCAND, BBO_IGNORE), GPI_ERROR);
          
               // Now combine image with target
               GFX (::GpiDrawBits (MemPS, mImageBits, mInfo, 4, aptlNew, ROP_SRCPAINT, BBO_IGNORE), GPI_ERROR);
   
               // Transfer bitmap from memory bitmap back to device
               POINTL aptlMemToDev [4] = { {dest.xLeft, dest.yBottom},   // TLL - device (Dx1, Dy2)
                                           {dest.xRight, dest.yTop},     // TUR - device (Dx2, Dy1)
                                           {0, 0},                       // SLL - mem bitmap (0, 0)
                                           {bihMem.cx, bihMem.cy} };      // SUR - mem bitmap (cx, cy)
   
               GFX (::GpiBitBlt (surf->GetPS (), MemPS, 4, aptlMemToDev, ROP_SRCCOPY, BBO_IGNORE), GPI_ERROR);
               
               rv = NS_OK;
                                                                   
               GFX (::GpiSetBitmap (MemPS, NULLHANDLE), HBM_ERROR);
               GFX (::GpiDeleteBitmap (hMemBmp), FALSE);
             }
           
             GFX (::GpiDestroyPS (MemPS), FALSE);
           }
       
           ::DevCloseDC (MemDC);
         }
      }
   } else
   {
      // Find the compatible device context and create a memory one
      HDC hdcCompat = GFX (::GpiQueryDevice (surf->GetPS ()), HDC_ERROR);

      rv = NS_ERROR_FAILURE;
    
       // create non-inclusive rect for GpiBitBlt
      RECTL dest;
      surf->NS2PM_INEX (trect, dest);

      DEVOPENSTRUC dop = { 0, 0, 0, 0, 0 };
      HDC MemDC = ::DevOpenDC( (HAB)0, OD_MEMORY, "*", 5, (PDEVOPENDATA) &dop, hdcCompat);

      if( MemDC != DEV_ERROR )
      {   
        // create the PS
        SIZEL sizel = { 0, 0 };
        HPS MemPS = GFX (::GpiCreatePS (0, MemDC, &sizel, PU_PELS | GPIT_MICRO | GPIA_ASSOC), GPI_ERROR);

        if( MemPS != GPI_ERROR )
        {
          GFX (::GpiCreateLogColorTable (MemPS, 0, LCOLF_RGB, 0, 0, 0), FALSE);

          // now create a bitmap of the right size
          HBITMAP hMemBmp;
          BITMAPINFOHEADER2 bihMem = { 0 };

          bihMem.cbFix = sizeof (BITMAPINFOHEADER2);
          bihMem.cx = aDWidth;
          bihMem.cy = aDHeight;
          bihMem.cPlanes = 1;
          LONG lBitCount = 0;
          GFX (::DevQueryCaps( hdcCompat, CAPS_COLOR_BITCOUNT, 1, &lBitCount), FALSE);
          if (!fPrinting)
          {
            bihMem.cBitCount = (USHORT) lBitCount;
          }
          else  // Printing
          {
            // bihMem.cBitCount = (USHORT) lBitCount;
            bihMem.cBitCount = 24;
          }

          hMemBmp = GFX (::GpiCreateBitmap (MemPS, &bihMem, 0, 0, 0), GPI_ERROR);

          if( hMemBmp != GPI_ERROR )
          {
            GFX (::GpiSetBitmap (MemPS, hMemBmp), HBM_ERROR);

            POINTL aptlDevToMem [4] = { {0, 0},                       // TLL - mem bitmap (0, 0)
                                        {bihMem.cx, bihMem.cy},       // TUR - mem bitmap (cx, cy)
                                        {dest.xLeft, dest.yBottom},   // SLL - device (Dx1, Dy2)
                                        {dest.xRight, dest.yTop} };   // SUR - device (Dx2, Dy1)

            GFX (::GpiBitBlt (MemPS, surf->GetPS (), 4, aptlDevToMem, ROP_SRCCOPY, BBO_IGNORE), GPI_ERROR);

            // Now we want direct access to bitmap raw data. 
            // Must copy data again because GpiSetBitmap doesn't provide pointer to
            // start of raw bit data ?? (DJ)
            BITMAPINFOHEADER2 bihDirect = { 0 };
            bihDirect.cbFix   = sizeof (BITMAPINFOHEADER2);
            bihDirect.cPlanes = 1;
            bihDirect.cBitCount = 24;

            int RawDataSize = bihMem.cy * RASWIDTH (bihMem.cx, 24);
            PRUint8* pRawBitData = (PRUint8*)malloc (RawDataSize);

            if( pRawBitData )
            {
              LONG rc = GFX (::GpiQueryBitmapBits (MemPS, 0, bihMem.cy, (PBYTE)pRawBitData, (PBITMAPINFO2)&bihDirect), GPI_ALTERROR);

              if( rc != GPI_ALTERROR )
              {
		PRUint8 *imageRGB, *imageAlpha;
		PRUint32 strideRGB, strideAlpha;

		/* Both scaled and unscaled images come through this code - save
		   work if not scaling */
		if ((aSWidth != aDWidth) || (aSHeight != aDHeight)) {
		  /* Scale our image to match */
		  imageRGB = (PRUint8 *)nsMemory::Alloc(3*aDWidth*aDHeight);
		  imageAlpha = (PRUint8 *)nsMemory::Alloc(aDWidth*aDHeight);
		      
		  if (!imageRGB || !imageAlpha) {
		    if (imageRGB)
		      nsMemory::Free(imageRGB);
		    if (imageAlpha)
		      nsMemory::Free(imageAlpha);

		    free(pRawBitData);
		    GFX (::GpiSetBitmap (MemPS, NULLHANDLE), HBM_ERROR);
		    GFX (::GpiDeleteBitmap (hMemBmp), FALSE);
		    GFX (::GpiDestroyPS (MemPS), FALSE);
		    ::DevCloseDC (MemDC);

		    return NS_ERROR_FAILURE;
		  }
		      
		  strideRGB = 3 * aDWidth;
		  strideAlpha = aDWidth;
		  RectStretch(aSWidth, aSHeight, aDWidth, aDHeight, 0, 0, aDWidth-1, aDHeight-1,
			      mImageBits, mRowBytes, imageRGB, strideRGB, 24);
		  RectStretch(aSWidth, aSHeight, aDWidth, aDHeight, 0, 0, aDWidth-1, aDHeight-1,
			      mAlphaBits, mARowBytes, imageAlpha, strideAlpha, 8);
		} else {
                  PRUint32 srcy = mInfo->cy - (aSY + aSHeight);
		  imageRGB = mImageBits + srcy * mRowBytes + aSX * 3;
		  imageAlpha = mAlphaBits + srcy * mARowBytes + aSX;
		  strideRGB = mRowBytes;
		  strideAlpha = mARowBytes;
		}

		/* Do composite */
		DrawComposited24(pRawBitData, imageRGB, strideRGB, imageAlpha, strideAlpha,
				 aDWidth, aDHeight);
		  
		if ((aSWidth != aDWidth) || (aSHeight != aDHeight)) {
		  /* Free scaled images */
		  nsMemory::Free(imageRGB);
		  nsMemory::Free(imageAlpha);
		}
        
                // Copy modified memory back to memory bitmap
                GFX (::GpiSetBitmapBits (MemPS, 0, bihMem.cy, (PBYTE)pRawBitData, (PBITMAPINFO2)&bihDirect), GPI_ALTERROR);
              
                // Transfer bitmap from memory bitmap back to device
                POINTL aptlMemToDev [4] = { {dest.xLeft, dest.yBottom},   // TLL - device (Dx1, Dy2)
                                            {dest.xRight, dest.yTop},     // TUR - device (Dx2, Dy1)
                                            {0, 0},                       // SLL - mem bitmap (0, 0)
                                            {bihMem.cx, bihMem.cy} };      // SUR - mem bitmap (cx, cy)

                GFX (::GpiBitBlt (surf->GetPS (), MemPS, 4, aptlMemToDev, ROP_SRCCOPY, BBO_IGNORE), GPI_ERROR);
            
                rv = NS_OK;
              }

              free (pRawBitData);
            }

            GFX (::GpiSetBitmap (MemPS, NULLHANDLE), HBM_ERROR);
            GFX (::GpiDeleteBitmap (hMemBmp), FALSE);
          }
        
          GFX (::GpiDestroyPS (MemPS), FALSE);
        }
    
        ::DevCloseDC (MemDC);
      }
   }

   return rv;
}

nsresult nsImageOS2::Optimize( nsIDeviceContext* aContext)
{
   // Defer this until we have a PS...
   mIsOptimized = PR_TRUE;
   return NS_OK;
}

//------------------------------------------------------------
// lock the image pixels. implement this if you need it
NS_IMETHODIMP
nsImageOS2::LockImagePixels(PRBool aMaskPixels)
{
  return NS_OK;
}

//------------------------------------------------------------
// unlock the image pixels. implement this if you need it
NS_IMETHODIMP
nsImageOS2::UnlockImagePixels(PRBool aMaskPixels)
{
  return NS_OK;
} 

void
nsImageOS2::BuildTile (HPS hpsTile, PRUint8* pImageBits, PBITMAPINFO2 pBitmapInfo,
                       nscoord aTileWidth, nscoord aTileHeight, float scale)
{
   // If bitmap not fully loaded, then first fill area with background color.
   if (nsRect (0, 0, mInfo->cx, mInfo->cy) != mDecodedRect)
   {
      POINTL pt1 = { 0, 0 };                                                   // LL - in
      POINTL pt2 = { mInfo->cx, mInfo->cy };                                   // UR - ex

#ifdef DEBUG
      GFX (::GpiSetColor (hpsTile, MK_RGB (255, 255, 0)), FALSE);              // yellow eye-catcher
#else
      GFX (::GpiSetColor (hpsTile, MK_RGB (255, 255, 255)), FALSE);
#endif
      GFX (::GpiMove (hpsTile, &pt1), FALSE);
      GFX (::GpiBox (hpsTile, DRO_FILL, &pt2, 0, 0), GPI_ERROR);
   }

   // Set up blit coord array
   POINTL aptl [4] = { {mDecodedRect.x, mDecodedRect.y},                         // TLL - in
                       {mDecodedRect.XMost () - 1, mDecodedRect.YMost () - 1},   // TUR - in
                       {mDecodedRect.x, mDecodedRect.y},                         // SLL - in
                       {mDecodedRect.XMost (), mDecodedRect.YMost ()} };         // SUR - ex

   // Scale up
   aptl[0].x = (LONG)(aptl[0].x * scale);
   aptl[0].y = (LONG)(aptl[0].y * scale);
   aptl[1].x = (LONG)(mDecodedRect.XMost() * scale) - 1;
   aptl[1].y = (LONG)(mDecodedRect.YMost() * scale) - 1;
   
   // Draw bitmap once into temporary PS
   GFX (::GpiDrawBits (hpsTile, (PBYTE)pImageBits, pBitmapInfo, 4, aptl, ROP_SRCCOPY, BBO_IGNORE), GPI_ERROR);

   PRInt32 DestWidth  = (PRInt32)(mInfo->cx * scale);
   PRInt32 DestHeight = (PRInt32)(mInfo->cy * scale);

   // Copy bitmap horizontally, doubling each time
   if (DestWidth > 0) {
      while (DestWidth < aTileWidth)
      {
         POINTL aptlCopy [3] = { {DestWidth, 0},                     // TLL - in
                                 {2 * DestWidth, DestHeight},        // TUR - ex
                                 {0, 0} };                           // SLL - in

         GFX (::GpiBitBlt (hpsTile, hpsTile, 3, aptlCopy, ROP_SRCCOPY, 0L), GPI_ERROR);
         DestWidth *= 2;
      }
   }

   // Copy bitmap vertically, doubling each time
   if (DestHeight > 0) {
      while (DestHeight < aTileHeight)
      {
         POINTL aptlCopy [4] = { {0, DestHeight},                    // TLL - in
                                 {DestWidth, 2 * DestHeight},        // TUR - ex
                                 {0, 0} };                           // SLL - in

         GFX (::GpiBitBlt (hpsTile, hpsTile, 3, aptlCopy, ROP_SRCCOPY, 0L), GPI_ERROR);
         DestHeight *= 2;
      }
   }
}

/** ---------------------------------------------------
 *  See documentation in nsIRenderingContext.h
 *  @update 3/16/00 dwc
 */
NS_IMETHODIMP nsImageOS2::DrawTile(nsIRenderingContext &aContext,
                                   nsIDrawingSurface* aSurface,
                                   PRInt32 aSXOffset, PRInt32 aSYOffset,
                                   PRInt32 aPadX, PRInt32 aPadY,
                                   const nsRect &aTileRect)
{
   if (aTileRect.IsEmpty ())
      return NS_OK;

   PRBool didTile = PR_FALSE;
   PRInt32 ImageWidth = mInfo->cx;
   PRInt32 ImageHeight = mInfo->cy;
   PRBool padded = (aPadX || aPadY);

   // Get the scale - if greater than 1 then do slow tile which
   nsIDeviceContext *theDeviceContext;
   float scale;
   aContext.GetDeviceContext(theDeviceContext);
   theDeviceContext->GetCanonicalPixelScale(scale);

   nsRect ValidRect (0, 0, ImageWidth, ImageHeight);
   ValidRect.IntersectRect (ValidRect, mDecodedRect);
   PRInt32 DestScaledWidth = PR_MAX(PRInt32(ValidRect.width * scale), 1);
   PRInt32 DestScaledHeight = PR_MAX(PRInt32(ValidRect.height * scale), 1);

   nsRect DrawRect = aTileRect;
   DrawRect.MoveBy (-aSXOffset, -aSYOffset);
   DrawRect.SizeBy (aSXOffset, aSYOffset);

   // Don't bother tiling if we only have to draw the bitmap a couple of times
   // Can't tile with 8bit alpha masks because need access destination bitmap values
   if ((ImageWidth < DrawRect.width / 2 || ImageHeight < DrawRect.height / 2) &&
       (ImageWidth <= MAX_BUFFER_WIDTH) && (ImageHeight <= MAX_BUFFER_HEIGHT) &&
       mAlphaDepth <= 1 &&
       !padded)
   {
      nsDrawingSurfaceOS2 *surf = (nsDrawingSurfaceOS2*) aSurface;

      // Find the compatible device context and create a memory one
      HDC hdcCompat = GFX (::GpiQueryDevice (surf->GetPS ()), HDC_ERROR);

      DEVOPENSTRUC dop = { 0, 0, 0, 0, 0 };
      HDC MemDC = GFX (::DevOpenDC( (HAB)0, OD_MEMORY, "*", 5, (PDEVOPENDATA) &dop, hdcCompat), DEV_ERROR);

      if( DEV_ERROR != MemDC)
      {
         // create the PS
         SIZEL sizel = { 0, 0 };
         HPS MemPS = GFX (::GpiCreatePS (0, MemDC, &sizel, PU_PELS | GPIT_MICRO | GPIA_ASSOC), GPI_ERROR);

         if( GPI_ERROR != MemPS)
         {
            GFX (::GpiCreateLogColorTable (MemPS, 0, LCOLF_RGB, 0, 0, 0), FALSE);

            // now create a bitmap of the right size
            BITMAPINFOHEADER2 hdr = { 0 };

            hdr.cbFix = sizeof( BITMAPINFOHEADER2);
            // Maximum size of tiled area (could do this better)
            PRInt32 endWidth = DestScaledWidth;
            while( endWidth < DrawRect.width)
               endWidth *= 2;

            PRInt32 endHeight = DestScaledHeight;
            while( endHeight < DrawRect.height)
               endHeight *= 2;

            hdr.cx = endWidth;
            hdr.cy = endHeight;
            hdr.cPlanes = 1;

            // find bitdepth
            LONG lBitCount = 0;
            GFX (::DevQueryCaps( hdcCompat, CAPS_COLOR_BITCOUNT, 1, &lBitCount), FALSE);
            hdr.cBitCount = (USHORT) lBitCount;

            RECTL  rcl;
            surf->NS2PM_INEX (aTileRect, rcl);

            POINTL aptlTile [3] = { {rcl.xLeft, rcl.yBottom},                                 // TLL - in
                                    {rcl.xRight, rcl.yTop},                                   // TUR - ex
                                    {aSXOffset, endHeight - aTileRect.height - aSYOffset} };  // SLL - in
            // For some reason offset does not work well with scaled output
            if (scale > 1.0)
            {
               aptlTile[2].x = 0;
               aptlTile[2].y = endHeight - aTileRect.height;
            }
            HBITMAP hMemBmp = GFX (::GpiCreateBitmap (MemPS, &hdr, 0, 0, 0), GPI_ERROR);
            if (hMemBmp != GPI_ERROR)
            {
               LONG ImageROP = ROP_SRCCOPY;

               GFX (::GpiSetBitmap (MemPS, hMemBmp), HBM_ERROR);

               if (mAlphaDepth == 1)
               {
                  LONG BlackColor = GFX (::GpiQueryColorIndex (MemPS, 0, MK_RGB (0x00, 0x00, 0x00)), GPI_ALTERROR);    // CLR_BLACK;
                  LONG WhiteColor = GFX (::GpiQueryColorIndex (MemPS, 0, MK_RGB (0xFF, 0xFF, 0xFF)), GPI_ALTERROR);    // CLR_WHITE;

                  // WORKAROUND:
                  // PostScript drivers up to version 30.732 have problems with GpiQueryColorIndex.
                  // If we are in LCOL_RGB mode it doesn't return passed RGB color but returns error instead.

                  if (BlackColor == GPI_ALTERROR) BlackColor = MK_RGB (0x00, 0x00, 0x00);
                  if (WhiteColor == GPI_ALTERROR) WhiteColor = MK_RGB (0xFF, 0xFF, 0xFF);

                  // Set image foreground and background colors. These are used in transparent images for blitting 1-bit masks.
                  // To invert colors on ROP_SRCAND we map 1 to black and 0 to white
                  IMAGEBUNDLE ib;
                  ib.lColor     = BlackColor;        // map 1 in mask to 0x000000 (black) in destination
                  ib.lBackColor = WhiteColor;        // map 0 in mask to 0xFFFFFF (white) in destination
                  ib.usMixMode  = FM_OVERPAINT;
                  ib.usBackMixMode = BM_OVERPAINT;
                  GFX (::GpiSetAttrs (MemPS, PRIM_IMAGE, IBB_COLOR | IBB_BACK_COLOR | IBB_MIX_MODE | IBB_BACK_MIX_MODE, 0, (PBUNDLE)&ib), FALSE);

                  MONOBITMAPINFO MaskBitmapInfo (mInfo);
                  BuildTile (MemPS, mAlphaBits, MaskBitmapInfo, DrawRect.width, DrawRect.height, scale);

                  // Apply mask to target, clear pels we will fill in from the image
                  GFX (::GpiBitBlt (surf->GetPS (), MemPS, 3, aptlTile, ROP_SRCAND, 0L), GPI_ERROR);

                  ImageROP = ROP_SRCPAINT;    // Original image must be combined with mask
               }

               BuildTile (MemPS, mImageBits, mInfo, DrawRect.width, DrawRect.height, scale);

               GFX (::GpiBitBlt (surf->GetPS (), MemPS, 3, aptlTile, ImageROP, 0L), GPI_ERROR);

               didTile = PR_TRUE;

               // Must deselect bitmap from PS before freeing bitmap and PS.
               GFX (::GpiSetBitmap (MemPS, NULLHANDLE), HBM_ERROR);
               GFX (::GpiDeleteBitmap (hMemBmp), FALSE);
            }
            GFX (::GpiDestroyPS (MemPS), FALSE);
         }
         GFX (::DevCloseDC (MemDC), DEV_ERROR);
      }
   }

   // If we failed to tile the bitmap, then use the old, slow, reliable way
   if( didTile == PR_FALSE)
   {
      // put the DestRect into absolute coordintes of the device
      PRInt32 y0 = aTileRect.y - aSYOffset;
      PRInt32 x0 = aTileRect.x - aSXOffset;
      PRInt32 y1 = aTileRect.y + aTileRect.height;
      PRInt32 x1 = aTileRect.x + aTileRect.width;

      // this is the width and height of the image in pixels
      // we need to map this to the pixel height of the device
      nscoord ScaledTileWidth = PR_MAX(PRInt32(ImageWidth*scale), 1);
      nscoord ScaledTileHeight = PR_MAX(PRInt32(ImageHeight*scale), 1);

      for (PRInt32 y = y0; y < y1; y += (PRInt32)(ScaledTileHeight + aPadY * scale))
      {
        for (PRInt32 x = x0; x < x1;  x += (PRInt32)(ScaledTileWidth + aPadX * scale))
        {
          Draw(aContext, aSurface,
               0, 0, PR_MIN(ValidRect.width, x1 - x), PR_MIN(ValidRect.height, y1 - y),
               x, y, PR_MIN(DestScaledWidth, x1-x), PR_MIN(DestScaledHeight, y1 - y));
        }
      }
   }
   return NS_OK;
}


void nsImageOS2::NS2PM_ININ( const nsRect &in, RECTL &rcl)
{
  PRUint32 ulHeight = GetHeight ();

  rcl.xLeft = in.x;
  rcl.xRight = in.x + in.width - 1;
  rcl.yTop = ulHeight - in.y - 1;
  rcl.yBottom = rcl.yTop - in.height + 1;
}


// ---------------------------------------------------
//	Update mImageBits with content of new mBitmap
//
NS_IMETHODIMP nsImageOS2::UpdateImageBits( HPS aPS )
{
  BITMAPINFOHEADER2 rawInfo = { 0 };
  rawInfo.cbFix   = sizeof (BITMAPINFOHEADER2);
  rawInfo.cPlanes = 1;
  rawInfo.cBitCount = mInfo->cBitCount;

  int RawDataSize = mInfo->cy * RASWIDTH (mInfo->cx, mInfo->cBitCount);
  PRUint8* pRawBitData = new PRUint8 [RawDataSize];

  if (pRawBitData)
  {
    GFX (::GpiQueryBitmapBits (aPS, 0, mInfo->cy, (PBYTE)pRawBitData, (PBITMAPINFO2)&rawInfo), GPI_ALTERROR);
    delete [] mImageBits;
    mImageBits = pRawBitData;
    return NS_OK;
  }
  else
    return NS_ERROR_OUT_OF_MEMORY;
}

NS_IMETHODIMP nsImageOS2::DrawToImage(nsIImage* aDstImage,
                                      nscoord aDX, nscoord aDY,
                                      nscoord aDWidth, nscoord aDHeight)
{
  nsresult rc = NS_OK;

  DEVOPENSTRUC dop = { 0, 0, 0, 0, 0 };
  SIZEL sizel = { 0, 0 };

  if (mInfo == nsnull || aDWidth < 0 || aDHeight < 0) 
    return NS_ERROR_FAILURE;

  if (0 == aDWidth || 0 == aDHeight)
    return NS_OK;

  // Create a memory DC that is compatible with the screen
  HDC MemDC = GFX (::DevOpenDC( 0/*hab*/, OD_MEMORY, "*", 5,
                   (PDEVOPENDATA) &dop, (HDC)0), DEV_ERROR);

  // create the PS
  HPS MemPS = GFX (::GpiCreatePS (0/*hab*/, MemDC, &sizel, PU_PELS | GPIT_MICRO | GPIA_ASSOC), GPI_ERROR);

  GFX (::GpiCreateLogColorTable (MemPS, 0, LCOLF_RGB, 0, 0, 0), FALSE);

  nsImageOS2* destImg = NS_STATIC_CAST(nsImageOS2*, aDstImage); 

  HBITMAP hTmpBitmap = GFX (::GpiCreateBitmap (MemPS, (PBITMAPINFOHEADER2)destImg->mInfo,
                                               CBM_INIT, (PBYTE)destImg->mImageBits,
                                               destImg->mInfo), GPI_ERROR);
  GFX (::GpiSetBitmap (MemPS, hTmpBitmap), HBM_ERROR);
  
  nsRect trect( aDX, aDY, aDWidth, aDHeight);
  RECTL  rcl;
  destImg->NS2PM_ININ (trect, rcl);

  // Set up blit coord array
  POINTL aptl [4] = { {rcl.xLeft, rcl.yBottom},              // TLL - in
                      {rcl.xRight, rcl.yTop},                // TUR - in
                      {0, 0},                                // SLL - in
                      {mInfo->cx, mInfo->cy} };              // SUR - ex

  if( 1==mAlphaDepth && mAlphaBits)
  {
    LONG BlackColor = GFX (::GpiQueryColorIndex (MemPS, 0, MK_RGB (0x00, 0x00, 0x00)), GPI_ALTERROR);    // CLR_BLACK;
    LONG WhiteColor = GFX (::GpiQueryColorIndex (MemPS, 0, MK_RGB (0xFF, 0xFF, 0xFF)), GPI_ALTERROR);    // CLR_WHITE;
    if (BlackColor == GPI_ALTERROR) BlackColor = MK_RGB (0x00, 0x00, 0x00);
    if (WhiteColor == GPI_ALTERROR) WhiteColor = MK_RGB (0xFF, 0xFF, 0xFF);

    // Set image foreground and background colors. These are used in transparent images for blitting 1-bit masks.
    // To invert colors on ROP_SRCAND we map 1 to black and 0 to white
    IMAGEBUNDLE ib;
    ib.lColor     = BlackColor;        // map 1 in mask to 0x000000 (black) in destination
    ib.lBackColor = WhiteColor;        // map 0 in mask to 0xFFFFFF (white) in destination
    ib.usMixMode  = FM_OVERPAINT;
    ib.usBackMixMode = BM_OVERPAINT;
    GFX (::GpiSetAttrs (MemPS, PRIM_IMAGE, IBB_COLOR | IBB_BACK_COLOR | IBB_MIX_MODE | IBB_BACK_MIX_MODE, 0, (PBUNDLE)&ib), FALSE);

    // Apply mask to target, clear pels we will fill in from the image
    MONOBITMAPINFO MaskBitmapInfo (mInfo);
    GFX (::GpiDrawBits (MemPS, mAlphaBits, MaskBitmapInfo, 4, aptl, ROP_SRCAND,
                        BBO_IGNORE), GPI_ERROR);

    // Now combine image with target
    GFX (::GpiDrawBits (MemPS, mImageBits, mInfo, 4, aptl, ROP_SRCPAINT, 
                        BBO_IGNORE), GPI_ERROR);
  } else {
    // alpha depth of 8 not used (yet?)
    NS_ASSERTION( mAlphaDepth != 8, "Alpha depth of 8 not implemented in DrawToImage" );

    // no transparency, just blit it
    GFX (::GpiDrawBits (MemPS, mImageBits, mInfo, 4, aptl, ROP_SRCCOPY,
                        BBO_IGNORE), GPI_ERROR);
  }

  rc = destImg->UpdateImageBits (MemPS);

  GFX (::GpiSetBitmap (MemPS, NULLHANDLE), HBM_ERROR);
  GFX (::GpiDeleteBitmap (hTmpBitmap), FALSE);
  GFX (::GpiDestroyPS (MemPS), FALSE);
  GFX (::DevCloseDC (MemDC), DEV_ERROR);

  return rc;
}
