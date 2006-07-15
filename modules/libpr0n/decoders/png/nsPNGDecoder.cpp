/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * ***** BEGIN LICENSE BLOCK *****
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
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2001
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Stuart Parmenter <stuart@mozilla.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#include "nsPNGDecoder.h"

#include "nsMemory.h"
#include "nsRect.h"

#include "nsIComponentManager.h"
#include "nsIInputStream.h"

#include "imgIContainerObserver.h"
#include "nsIImage.h"
#include "nsIInterfaceRequestorUtils.h"

#include "nsColor.h"

#include "nspr.h"
#include "png.h"

static void PNGAPI info_callback(png_structp png_ptr, png_infop info_ptr);
static void PNGAPI row_callback(png_structp png_ptr, png_bytep new_row,
                           png_uint_32 row_num, int pass);
static void PNGAPI end_callback(png_structp png_ptr, png_infop info_ptr);
static void PNGAPI error_callback(png_structp png_ptr, png_const_charp error_msg);
static void PNGAPI warning_callback(png_structp png_ptr, png_const_charp warning_msg);

#ifdef PR_LOGGING
PRLogModuleInfo *gPNGLog = PR_NewLogModule("PNGDecoder");
#endif

NS_IMPL_ISUPPORTS1(nsPNGDecoder, imgIDecoder)

nsPNGDecoder::nsPNGDecoder() :
  mPNG(nsnull), mInfo(nsnull),
#ifndef MOZ_CAIRO_GFX
  colorLine(nsnull),
  alphaLine(nsnull),
#endif
  interlacebuf(nsnull), ibpr(0),
  mError(PR_FALSE)
{
}

nsPNGDecoder::~nsPNGDecoder()
{
#ifndef MOZ_CAIRO_GFX
  if (colorLine)
    nsMemory::Free(colorLine);
  if (alphaLine)
    nsMemory::Free(alphaLine);
#endif
  if (interlacebuf)
    nsMemory::Free(interlacebuf);
}


/** imgIDecoder methods **/

/* void init (in imgILoad aLoad); */
NS_IMETHODIMP nsPNGDecoder::Init(imgILoad *aLoad)
{
#if defined(PNG_UNKNOWN_CHUNKS_SUPPORTED)
  static png_byte unused_chunks[]=
       { 98,  75,  71,  68, '\0',   /* bKGD */
         99,  72,  82,  77, '\0',   /* cHRM */
        104,  73,  83,  84, '\0',   /* hIST */
        105,  67,  67,  80, '\0',   /* iCCP */
        105,  84,  88, 116, '\0',   /* iTXt */
        111,  70,  70, 115, '\0',   /* oFFs */
        112,  67,  65,  76, '\0',   /* pCAL */
        115,  67,  65,  76, '\0',   /* sCAL */
        112,  72,  89, 115, '\0',   /* pHYs */
        115,  66,  73,  84, '\0',   /* sBIT */
        115,  80,  76,  84, '\0',   /* sPLT */
        116,  69,  88, 116, '\0',   /* tEXt */
        116,  73,  77,  69, '\0',   /* tIME */
        122,  84,  88, 116, '\0'};  /* zTXt */
#endif

  mImageLoad = aLoad;
  mObserver = do_QueryInterface(aLoad);  // we're holding 2 strong refs to the request.

  /* do png init stuff */

  /* Initialize the container's source image header. */
  /* Always decode to 24 bit pixdepth */

  mPNG = png_create_read_struct(PNG_LIBPNG_VER_STRING, 
                                NULL, error_callback, warning_callback);
  if (!mPNG) {
    return NS_ERROR_OUT_OF_MEMORY;
  }

  mInfo = png_create_info_struct(mPNG);
  if (!mInfo) {
    png_destroy_read_struct(&mPNG, NULL, NULL);
    return NS_ERROR_OUT_OF_MEMORY;
  }

#if defined(PNG_UNKNOWN_CHUNKS_SUPPORTED)
  /* Ignore unused chunks */
  png_set_keep_unknown_chunks(mPNG, 0, unused_chunks,
     (int)sizeof(unused_chunks)/5);   
#endif

  /* use this as libpng "progressive pointer" (retrieve in callbacks) */
  png_set_progressive_read_fn(mPNG, NS_STATIC_CAST(png_voidp, this),
                              info_callback, row_callback, end_callback);

  return NS_OK;
}

/* void close (); */
NS_IMETHODIMP nsPNGDecoder::Close()
{
  if (mPNG)
    png_destroy_read_struct(&mPNG, mInfo ? &mInfo : NULL, NULL);

  return NS_OK;
}

/* void flush (); */
NS_IMETHODIMP nsPNGDecoder::Flush()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}


static NS_METHOD ReadDataOut(nsIInputStream* in,
                             void* closure,
                             const char* fromRawSegment,
                             PRUint32 toOffset,
                             PRUint32 count,
                             PRUint32 *writeCount)
{
  nsPNGDecoder *decoder = NS_STATIC_CAST(nsPNGDecoder*, closure);

  if (decoder->mError) {
    *writeCount = 0;
    return NS_ERROR_FAILURE;
  }

  // we need to do the setjmp here otherwise bad things will happen
  if (setjmp(decoder->mPNG->jmpbuf)) {
    png_destroy_read_struct(&decoder->mPNG, &decoder->mInfo, NULL);

    decoder->mError = PR_TRUE;
    *writeCount = 0;
    return NS_ERROR_FAILURE;
  }

  png_process_data(decoder->mPNG, decoder->mInfo,
                   NS_REINTERPRET_CAST(unsigned char *, NS_CONST_CAST(char *, fromRawSegment)), count);

  *writeCount = count;
  return NS_OK;
}


/* unsigned long writeFrom (in nsIInputStream inStr, in unsigned long count); */
NS_IMETHODIMP nsPNGDecoder::WriteFrom(nsIInputStream *inStr, PRUint32 count, PRUint32 *_retval)
{
  NS_ASSERTION(inStr, "Got a null input stream!");

  nsresult rv;

  if (!mError)
    rv = inStr->ReadSegments(ReadDataOut, this, count, _retval);

  if (mError) {
    *_retval = 0;
    rv = NS_ERROR_FAILURE;
  }

  return rv;
}


void
info_callback(png_structp png_ptr, png_infop info_ptr)
{
/*  int number_passes;   NOT USED  */
  png_uint_32 width, height;
  int bit_depth, color_type, interlace_type, compression_type, filter_type;
  int channels;
  double aGamma;

  png_bytep trans=NULL;
  int num_trans =0;

  /* always decode to 24-bit RGB or 32-bit RGBA  */
  png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
               &interlace_type, &compression_type, &filter_type);

  /* limit image dimensions (bug #251381) */
#define MOZ_PNG_MAX_DIMENSION 1000000L
  if (width > MOZ_PNG_MAX_DIMENSION || height > MOZ_PNG_MAX_DIMENSION) {
    nsPNGDecoder *decoder = NS_STATIC_CAST(nsPNGDecoder*,
                                           png_get_progressive_ptr(png_ptr));
    longjmp(decoder->mPNG->jmpbuf, 1);
  }
#undef MOZ_PNG_MAX_DIMENSION

  if (color_type == PNG_COLOR_TYPE_PALETTE)
    png_set_expand(png_ptr);

  if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
    png_set_expand(png_ptr);

  if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
    png_get_tRNS(png_ptr, info_ptr, &trans, &num_trans, NULL);
    png_set_expand(png_ptr);
  }

  if (bit_depth == 16)
    png_set_strip_16(png_ptr);

  if (color_type == PNG_COLOR_TYPE_GRAY ||
      color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
      png_set_gray_to_rgb(png_ptr);


#if !defined(MOZ_CAIRO_GFX) && (defined(XP_WIN) || defined(XP_OS2) || defined(XP_BEOS) || defined(MOZ_WIDGET_PHOTON))
  // windows likes BGR
  png_set_bgr(png_ptr);
#endif

  if (png_get_gAMA(png_ptr, info_ptr, &aGamma)) {
      if ((aGamma <= 0.0) || (aGamma > 21474.83)) {
          aGamma = 0.45455;
          png_set_gAMA(png_ptr, info_ptr, aGamma);
      }
      png_set_gamma(png_ptr, 2.2, aGamma);
  }
  else
      png_set_gamma(png_ptr, 2.2, 0.45455);

  /* let libpng expand interlaced images */
  if (interlace_type == PNG_INTERLACE_ADAM7) {
    /* number_passes = */
    png_set_interlace_handling(png_ptr);
  }

  /* now all of those things we set above are used to update various struct
   * members and whatnot, after which we can get channels, rowbytes, etc. */
  png_read_update_info(png_ptr, info_ptr);
  channels = png_get_channels(png_ptr, info_ptr);
  PR_ASSERT(channels == 3 || channels == 4);

  /*---------------------------------------------------------------*/
  /* copy PNG info into imagelib structs (formerly png_set_dims()) */
  /*---------------------------------------------------------------*/

  PRInt32 alpha_bits = 1;

  if (channels > 3) {
    /* check if alpha is coming from a tRNS chunk and is binary */
    if (num_trans) {
      /* if it's not a indexed color image, tRNS means binary */
      if (color_type == PNG_COLOR_TYPE_PALETTE) {
        for (int i=0; i<num_trans; i++) {
          if ((trans[i] != 0) && (trans[i] != 255)) {
            alpha_bits = 8;
            break;
          }
        }
      }
    } else {
      alpha_bits = 8;
    }
  }

  nsPNGDecoder *decoder = NS_STATIC_CAST(nsPNGDecoder*, png_get_progressive_ptr(png_ptr));

  if (decoder->mObserver)
    decoder->mObserver->OnStartDecode(nsnull);

  decoder->mImage = do_CreateInstance("@mozilla.org/image/container;1");
  if (!decoder->mImage)
    longjmp(decoder->mPNG->jmpbuf, 5); // NS_ERROR_OUT_OF_MEMORY

  decoder->mImageLoad->SetImage(decoder->mImage);

  // since the png is only 1 frame, initialize the container to the width and height of the frame
  decoder->mImage->Init(width, height, decoder->mObserver);

  if (decoder->mObserver)
    decoder->mObserver->OnStartContainer(nsnull, decoder->mImage);

  decoder->mFrame = do_CreateInstance("@mozilla.org/gfx/image/frame;2");
  if (!decoder->mFrame)
    longjmp(decoder->mPNG->jmpbuf, 5); // NS_ERROR_OUT_OF_MEMORY

  gfx_format format;

  if (channels == 3) {
    format = gfxIFormats::RGB;
  } else if (channels > 3) {
    if (alpha_bits == 8) {
      decoder->mImage->GetPreferredAlphaChannelFormat(&format);
    } else if (alpha_bits == 1) {
      format = gfxIFormats::RGB_A1;
    }
  }

#if !defined(MOZ_CAIRO_GFX) && (defined(XP_WIN) || defined(XP_OS2) || defined(XP_BEOS) || defined(MOZ_WIDGET_PHOTON))
  // XXX this works...
  format += 1; // RGB to BGR
#endif

  // then initialize the frame and append it to the container
  nsresult rv = decoder->mFrame->Init(0, 0, width, height, format, 24);
  if (NS_FAILED(rv))
    longjmp(decoder->mPNG->jmpbuf, 5); // NS_ERROR_OUT_OF_MEMORY

  decoder->mImage->AppendFrame(decoder->mFrame);

  if (decoder->mObserver)
    decoder->mObserver->OnStartFrame(nsnull, decoder->mFrame);

  PRUint32 bpr;
  decoder->mFrame->GetImageBytesPerRow(&bpr);
#ifndef MOZ_CAIRO_GFX
  PRUint32 abpr;
  decoder->mFrame->GetAlphaBytesPerRow(&abpr);
  decoder->colorLine = (PRUint8 *)nsMemory::Alloc(bpr);
  if (channels > 3)
    decoder->alphaLine = (PRUint8 *)nsMemory::Alloc(abpr);
#endif
  if (interlace_type == PNG_INTERLACE_ADAM7) {
    if (channels > 3)
      decoder->ibpr = channels*width;
    else
      decoder->ibpr = bpr;
    decoder->interlacebuf = (PRUint8 *)nsMemory::Alloc(decoder->ibpr*height);
    if (!decoder->interlacebuf) {
      longjmp(decoder->mPNG->jmpbuf, 5); // NS_ERROR_OUT_OF_MEMORY
    }            
  }

  return;
}





void
row_callback(png_structp png_ptr, png_bytep new_row,
             png_uint_32 row_num, int pass)
{
  /* libpng comments:
   *
   * this function is called for every row in the image.  If the
   * image is interlacing, and you turned on the interlace handler,
   * this function will be called for every row in every pass.
   * Some of these rows will not be changed from the previous pass.
   * When the row is not changed, the new_row variable will be NULL.
   * The rows and passes are called in order, so you don't really
   * need the row_num and pass, but I'm supplying them because it
   * may make your life easier.
   *
   * For the non-NULL rows of interlaced images, you must call
   * png_progressive_combine_row() passing in the row and the
   * old row.  You can call this function for NULL rows (it will
   * just return) and for non-interlaced images (it just does the
   * memcpy for you) if it will make the code easier.  Thus, you
   * can just do this for all cases:
   *
   *    png_progressive_combine_row(png_ptr, old_row, new_row);
   *
   * where old_row is what was displayed for previous rows.  Note
   * that the first pass (pass == 0 really) will completely cover
   * the old row, so the rows do not have to be initialized.  After
   * the first pass (and only for interlaced images), you will have
   * to pass the current row, and the function will combine the
   * old row and the new row.
   */
  nsPNGDecoder *decoder = NS_STATIC_CAST(nsPNGDecoder*, png_get_progressive_ptr(png_ptr));

  PRUint32 bpr, abpr;
  decoder->mFrame->GetImageBytesPerRow(&bpr);
  decoder->mFrame->GetAlphaBytesPerRow(&abpr);

  png_bytep line;
  if (decoder->interlacebuf) {
    line = decoder->interlacebuf+(row_num*decoder->ibpr);
    png_progressive_combine_row(png_ptr, line, new_row);
  }
  else
    line = new_row;

  if (new_row) {
    PRInt32 width;
    decoder->mFrame->GetWidth(&width);
    PRUint32 iwidth = width;

    gfx_format format;
    decoder->mFrame->GetFormat(&format);
#ifndef MOZ_CAIRO_GFX
    PRUint8 *aptr, *cptr;
#else
    // we're thebes. we can write stuff directly to the data
    PRUint8 *imageData;
    PRUint32 imageDataLength;
    decoder->mFrame->GetImageData(&imageData, &imageDataLength);
    nsCOMPtr<nsIImage> img(do_GetInterface(decoder->mFrame));
#endif
    nsIntRect r(0, row_num, width, 1);


    // The mac specific ifdefs in the code below are there to make sure we
    // always fill in 4 byte pixels right now, which is what the mac always
    // allocates for its pixel buffers in true color mode. This will change
    // when we start storing images with color palettes when they don't need
    // true color support (GIFs).
    switch (format) {
    case gfxIFormats::RGB:
    case gfxIFormats::BGR:
      {
#if defined(MOZ_CAIRO_GFX)
        PRUint32 *cptr32 = (PRUint32*)(imageData + (row_num*bpr));
        for (PRUint32 x=0; x<iwidth; x++) {
          PRUint8 r = *line++;
          PRUint8 g = *line++;
          PRUint8 b = *line++;
          *cptr32++ = (0xFF << 24) | (r << 16) | (g << 8) | b;
        }
        r.y = row_num;
        img->ImageUpdated(nsnull, nsImageUpdateFlags_kBitsChanged, &r);

#elif defined(XP_MAC) || defined(XP_MACOSX)
        cptr = decoder->colorLine;
        for (PRUint32 x=0; x<iwidth; x++) {
          *cptr++ = 0;
          *cptr++ = *line++;
          *cptr++ = *line++;
          *cptr++ = *line++;
        }
        decoder->mFrame->SetImageData(decoder->colorLine, bpr, row_num*bpr);
#else
        decoder->mFrame->SetImageData((PRUint8*)line, bpr, row_num*bpr);
#endif
      }
      break;
    case gfxIFormats::RGB_A1:
    case gfxIFormats::BGR_A1:
      {
#if defined(MOZ_CAIRO_GFX)
        PRUint32 *cptr32 = (PRUint32*)(imageData + (row_num*bpr));
        for (PRUint32 x=0; x<iwidth; x++) {
          if (line[3]) {
            PRUint8 r = *line++;
            PRUint8 g = *line++;
            PRUint8 b = *line++;
            line++;
            *cptr32++ = (0xFF << 24) | (r << 16) | (g << 8) | b;
          } else {
            *cptr32++ = 0x00000000;
            line += 4;
          }
        }
        r.y = row_num;
        img->ImageUpdated(nsnull, nsImageUpdateFlags_kBitsChanged, &r);

#else
        cptr = decoder->colorLine;
        aptr = decoder->alphaLine;
        memset(aptr, 0, abpr);
        for (PRUint32 x=0; x<iwidth; x++) {
#if !defined(MOZ_CAIRO_GFX) && (defined(XP_MAC) || defined(XP_MACOSX))
          *cptr++ = 0;
#endif
          if (line[3]) {
            *cptr++ = *line++;
            *cptr++ = *line++;
            *cptr++ = *line++;
            aptr[x>>3] |= 1<<(7-x&0x7);
            line++;
          } else {
            *cptr++ = 0;
            *cptr++ = 0;
            *cptr++ = 0;
            line += 4;
          }
        }
        decoder->mFrame->SetAlphaData(decoder->alphaLine, abpr, row_num*abpr);
        decoder->mFrame->SetImageData(decoder->colorLine, bpr, row_num*bpr);
#endif
      }
      break;
    case gfxIFormats::RGB_A8:
    case gfxIFormats::BGR_A8:
      {
#if defined(MOZ_CAIRO_GFX)
        PRUint32 *cptr32 = (PRUint32*)(imageData + (row_num*bpr));
        for (PRUint32 x=0; x<iwidth; x++) {
          PRUint8 r = *line++;
          PRUint8 g = *line++;
          PRUint8 b = *line++;
          PRUint8 a = *line++;
          if (a == 0xFF) {
            *cptr32++ = (0xFF << 24) | (r << 16) | (g << 8) | b;
          } else if (a == 0x00) {
            *cptr32++ = 0x00000000;
          } else {
            FAST_DIVIDE_BY_255(r, r*a);
            FAST_DIVIDE_BY_255(g, g*a);
            FAST_DIVIDE_BY_255(b, b*a);
            *cptr32++ = (a << 24) | (r << 16) | (g << 8) | b;
          }
        }

        r.y = row_num;
        img->ImageUpdated(nsnull, nsImageUpdateFlags_kBitsChanged, &r);
#else
        cptr = decoder->colorLine;
        aptr = decoder->alphaLine;
        for (PRUint32 x=0; x<iwidth; x++) {
#if defined(XP_MAC) || defined(XP_MACOSX)
          *cptr++ = 0;
#endif
          *cptr++ = *line++;
          *cptr++ = *line++;
          *cptr++ = *line++;
          *aptr++ = *line++;
        }
        decoder->mFrame->SetAlphaData(decoder->alphaLine, abpr, row_num*abpr);
        decoder->mFrame->SetImageData(decoder->colorLine, bpr, row_num*bpr);
#endif
      }
      break;
    }

    decoder->mObserver->OnDataAvailable(nsnull, decoder->mFrame, &r);
  }
}



void
end_callback(png_structp png_ptr, png_infop info_ptr)
{
  /* libpng comments:
   *
   * this function is called when the whole image has been read,
   * including any chunks after the image (up to and including
   * the IEND).  You will usually have the same info chunk as you
   * had in the header, although some data may have been added
   * to the comments and time fields.
   *
   * Most people won't do much here, perhaps setting a flag that
   * marks the image as finished.
   */

  nsPNGDecoder *decoder = NS_STATIC_CAST(nsPNGDecoder*, png_get_progressive_ptr(png_ptr));

  if (decoder->mObserver) {
    decoder->mObserver->OnStopFrame(nsnull, decoder->mFrame);
    decoder->mObserver->OnStopContainer(nsnull, decoder->mImage);
    decoder->mObserver->OnStopDecode(nsnull, NS_OK, nsnull);
  }

  // We are never going to change the data of this frame again.  Let the OS
  // do what it wants with this image.
  decoder->mFrame->SetMutable(PR_FALSE);
}


void
error_callback(png_structp png_ptr, png_const_charp error_msg)
{
  PR_LOG(gPNGLog, PR_LOG_ERROR, ("libpng error: %s\n", error_msg));
  longjmp(png_ptr->jmpbuf, 1);
}


void
warning_callback(png_structp png_ptr, png_const_charp warning_msg)
{
  /* convert tRNS warning to error (bug #251381) */
  if (strncmp(warning_msg, "Missing PLTE before tRNS", 24) == 0)
    png_error(png_ptr, warning_msg);
  PR_LOG(gPNGLog, PR_LOG_WARNING, ("libpng warning: %s\n", warning_msg));
}
