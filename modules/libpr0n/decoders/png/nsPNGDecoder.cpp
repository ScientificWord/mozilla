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
 *   Andrew Smith
 *   Federico Mena-Quintero <federico@novell.com>
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

#include "gfxColor.h"
#include "nsColor.h"

#include "nspr.h"
#include "png.h"

#include "gfxPlatform.h"

static void PNGAPI info_callback(png_structp png_ptr, png_infop info_ptr);
static void PNGAPI row_callback(png_structp png_ptr, png_bytep new_row,
                           png_uint_32 row_num, int pass);
static void PNGAPI frame_info_callback(png_structp png_ptr, png_uint_32 frame_num);
static void PNGAPI end_callback(png_structp png_ptr, png_infop info_ptr);
static void PNGAPI error_callback(png_structp png_ptr, png_const_charp error_msg);
static void PNGAPI warning_callback(png_structp png_ptr, png_const_charp warning_msg);

#ifdef PR_LOGGING
static PRLogModuleInfo *gPNGLog = PR_NewLogModule("PNGDecoder");
static PRLogModuleInfo *gPNGDecoderAccountingLog = PR_NewLogModule("PNGDecoderAccounting");
#endif

NS_IMPL_ISUPPORTS1(nsPNGDecoder, imgIDecoder)

nsPNGDecoder::nsPNGDecoder() :
  mPNG(nsnull), mInfo(nsnull),
  mCMSLine(nsnull), interlacebuf(nsnull),
  mInProfile(nsnull), mTransform(nsnull),
  mChannels(0), mError(PR_FALSE), mFrameIsHidden(PR_FALSE)
{
}

nsPNGDecoder::~nsPNGDecoder()
{
  if (mCMSLine)
    nsMemory::Free(mCMSLine);
  if (interlacebuf)
    nsMemory::Free(interlacebuf);
  if (mInProfile) {
    cmsCloseProfile(mInProfile);

    /* mTransform belongs to us only if mInProfile is non-null */
    if (mTransform)
      cmsDeleteTransform(mTransform);
  }
}

// CreateFrame() is used for both simple and animated images
void nsPNGDecoder::CreateFrame(png_uint_32 x_offset, png_uint_32 y_offset, 
                                PRInt32 width, PRInt32 height, gfx_format format)
{
  mFrame = do_CreateInstance("@mozilla.org/gfx/image/frame;2");
  if (!mFrame)
    longjmp(mPNG->jmpbuf, 5); // NS_ERROR_OUT_OF_MEMORY

  nsresult rv = mFrame->Init(x_offset, y_offset, width, height, format, 24);
  if (NS_FAILED(rv))
    longjmp(mPNG->jmpbuf, 5); // NS_ERROR_OUT_OF_MEMORY

  if (png_get_valid(mPNG, mInfo, PNG_INFO_acTL))
    SetAnimFrameInfo();
  
  mImage->AppendFrame(mFrame);
  
  if (mObserver)
    mObserver->OnStartFrame(nsnull, mFrame);

 
  PR_LOG(gPNGDecoderAccountingLog, PR_LOG_DEBUG,
         ("PNGDecoderAccounting: nsPNGDecoder::CreateFrame -- created image frame with %dx%d pixels in container %p",
          width, height,
          mImage.get ()));

  mFrameHasNoAlpha = PR_TRUE;
}

// set timeout and frame disposal method for the current frame
void nsPNGDecoder::SetAnimFrameInfo()
{
  png_uint_16 delay_num, delay_den;
  /* delay, in seconds is delay_num/delay_den */
  png_byte dispose_op;
  png_byte blend_op;
  PRInt32 timeout; /* in milliseconds */
  
  delay_num = png_get_next_frame_delay_num(mPNG, mInfo);
  delay_den = png_get_next_frame_delay_den(mPNG, mInfo);
  dispose_op = png_get_next_frame_dispose_op(mPNG, mInfo);
  blend_op = png_get_next_frame_blend_op(mPNG, mInfo);

  if (delay_num == 0) {
    timeout = 0; // gfxImageFrame::SetTimeout() will set to a minimum
  } else {
    if (delay_den == 0)
      delay_den = 100; // so says the APNG spec
    
    // Need to cast delay_num to float to have a proper division and
    // the result to int to avoid compiler warning
    timeout = static_cast<PRInt32>
                         (static_cast<PRFloat64>(delay_num) * 1000 / delay_den);
  }
  mFrame->SetTimeout(timeout);
  
  if (dispose_op == PNG_DISPOSE_OP_PREVIOUS)
      mFrame->SetFrameDisposalMethod(imgIContainer::kDisposeRestorePrevious);
  else if (dispose_op == PNG_DISPOSE_OP_BACKGROUND)
      mFrame->SetFrameDisposalMethod(imgIContainer::kDisposeClear);
  else
      mFrame->SetFrameDisposalMethod(imgIContainer::kDisposeKeep);
  
  if (blend_op == PNG_BLEND_OP_SOURCE)
      mFrame->SetBlendMethod(imgIContainer::kBlendSource);
  /*else // 'over' is the default for a gfxImageFrame
      mFrame->SetBlendMethod(imgIContainer::kBlendOver); */
}

// set timeout and frame disposal method for the current frame
void nsPNGDecoder::EndImageFrame()
{
  if (mFrameHasNoAlpha) {
    nsCOMPtr<nsIImage> img(do_GetInterface(mFrame));
    img->SetHasNoAlpha();
  }

  // First tell the container that this frame is complete
  PRInt32 timeout = 100;
  PRUint32 numFrames = 0;
  mFrame->GetTimeout(&timeout);
  mImage->GetNumFrames(&numFrames);

  // We can't use mPNG->num_frames_read as it may be one ahead.
  if (numFrames > 1) {
    // Tell the image renderer that the frame is complete
    PRInt32 width, height;
    mFrame->GetWidth(&width);
    mFrame->GetWidth(&height);

    nsIntRect r(0, 0, width, height);
    nsCOMPtr<nsIImage> img(do_GetInterface(mFrame));
    img->ImageUpdated(nsnull, nsImageUpdateFlags_kBitsChanged, &r);
    mObserver->OnDataAvailable(nsnull, mFrame, &r);
  }

  mImage->EndFrameDecode(numFrames, timeout);
  if (mObserver)
    mObserver->OnStopFrame(nsnull, mFrame);
}


/** imgIDecoder methods **/

/* void init (in imgILoad aLoad); */
NS_IMETHODIMP nsPNGDecoder::Init(imgILoad *aLoad)
{
#if defined(PNG_UNKNOWN_CHUNKS_SUPPORTED)
  static png_byte color_chunks[]=
       { 99,  72,  82,  77, '\0',   /* cHRM */
        105,  67,  67,  80, '\0'};  /* iCCP */
  static png_byte unused_chunks[]=
       { 98,  75,  71,  68, '\0',   /* bKGD */
        104,  73,  83,  84, '\0',   /* hIST */
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
  if (!gfxPlatform::IsCMSEnabled()) {
    png_set_keep_unknown_chunks(mPNG, 1, color_chunks, 2);
  }
  png_set_keep_unknown_chunks(mPNG, 1, unused_chunks,
     (int)sizeof(unused_chunks)/5);   
#endif

  /* use this as libpng "progressive pointer" (retrieve in callbacks) */
  png_set_progressive_read_fn(mPNG, static_cast<png_voidp>(this),
                              info_callback, row_callback, end_callback);


  /* The image container may already exist if it is reloading itself from us.
   * Check that it has the same width/height; otherwise create a new container.
   */
  mImageLoad->GetImage(getter_AddRefs(mImage));
  if (!mImage) {
    mImage = do_CreateInstance("@mozilla.org/image/container;1");
    if (!mImage)
      return NS_ERROR_OUT_OF_MEMORY;
      
    mImageLoad->SetImage(mImage);
    if (NS_FAILED(mImage->SetDiscardable("image/png"))) {
      PR_LOG(gPNGDecoderAccountingLog, PR_LOG_DEBUG,
             ("PNGDecoderAccounting: info_callback(): failed to set image container %p as discardable",
              mImage.get()));
      return NS_ERROR_FAILURE;
    }
  }

  return NS_OK;
}

/* void close (); */
NS_IMETHODIMP nsPNGDecoder::Close()
{
  if (mPNG)
    png_destroy_read_struct(&mPNG, mInfo ? &mInfo : NULL, NULL);

  if (mImage) { // mImage could be null in the case of an error
    nsresult result = mImage->RestoreDataDone();
    if (NS_FAILED(result)) {
        PR_LOG(gPNGDecoderAccountingLog, PR_LOG_DEBUG,
            ("PNGDecoderAccounting: nsPNGDecoder::Close(): failure in RestoreDataDone() for image container %p",
                mImage.get()));

        mError = PR_TRUE;
        return result;
    }

    PR_LOG(gPNGDecoderAccountingLog, PR_LOG_DEBUG,
            ("PNGDecoderAccounting: nsPNGDecoder::Close(): image container %p is now with RestoreDataDone",
            mImage.get()));
  }
  return NS_OK;
}

/* void flush (); */
NS_IMETHODIMP nsPNGDecoder::Flush()
{
  return NS_OK;
}


static NS_METHOD ReadDataOut(nsIInputStream* in,
                             void* closure,
                             const char* fromRawSegment,
                             PRUint32 toOffset,
                             PRUint32 count,
                             PRUint32 *writeCount)
{
  nsPNGDecoder *decoder = static_cast<nsPNGDecoder*>(closure);

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
                   reinterpret_cast<unsigned char *>(const_cast<char *>(fromRawSegment)), count);

  nsresult result = decoder->mImage->AddRestoreData((char *) fromRawSegment, count);
  if (NS_FAILED (result)) {
    PR_LOG(gPNGDecoderAccountingLog, PR_LOG_DEBUG,
           ("PNGDecoderAccounting: ReadDataOut(): failed to add restore data to image container %p",
            decoder->mImage.get()));

    decoder->mError = PR_TRUE;
    *writeCount = 0;
    return result;
  }

  PR_LOG(gPNGDecoderAccountingLog, PR_LOG_DEBUG,
         ("PNGDecoderAccounting: ReadDataOut(): Added restore data to image container %p",
          decoder->mImage.get()));

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

// Adapted from http://www.littlecms.com/pngchrm.c example code
static cmsHPROFILE
PNGGetColorProfile(png_structp png_ptr, png_infop info_ptr,
                   int color_type, PRUint32 *inType, PRUint32 *intent)
{
  cmsHPROFILE profile = nsnull;
  *intent = INTENT_PERCEPTUAL;   // XXX: should this be the default?

#ifndef PNG_NO_READ_iCCP
  // First try to see if iCCP chunk is present
  if (png_get_valid(png_ptr, info_ptr, PNG_INFO_iCCP)) {
    png_uint_32 profileLen;
    char *profileData, *profileName;
    int compression;

    png_get_iCCP(png_ptr, info_ptr, &profileName, &compression,
                 &profileData, &profileLen);

    profile = cmsOpenProfileFromMem(profileData, profileLen);
    PRUint32 profileSpace = cmsGetColorSpace(profile);

#ifdef DEBUG_tor
    fprintf(stderr, "PNG profileSpace: 0x%08X\n", profileSpace);
#endif

    PRBool mismatch = PR_FALSE;
    if (color_type & PNG_COLOR_MASK_COLOR) {
      if (profileSpace != icSigRgbData)
        mismatch = PR_TRUE;
    } else {
      if (profileSpace == icSigRgbData)
        png_set_gray_to_rgb(png_ptr);
      else if (profileSpace != icSigGrayData)
        mismatch = PR_TRUE;
    }

    if (mismatch) {
      cmsCloseProfile(profile);
      profile = nsnull;
    } else {
      *intent = cmsTakeRenderingIntent(profile);
    }
  }
#endif

#ifndef PNG_NO_READ_sRGB
  // Check sRGB chunk
  if (!profile && png_get_valid(png_ptr, info_ptr, PNG_INFO_sRGB)) {
    profile = cmsCreate_sRGBProfile();

    if (profile) {
      int fileIntent;
      png_set_gray_to_rgb(png_ptr); 
      png_get_sRGB(png_ptr, info_ptr, &fileIntent);
      PRUint32 map[] = { INTENT_PERCEPTUAL, INTENT_RELATIVE_COLORIMETRIC,
                         INTENT_SATURATION, INTENT_ABSOLUTE_COLORIMETRIC };
      *intent = map[fileIntent];
    }
  }
#endif

  // Check gAMA/cHRM chunks
  if (!profile && png_get_valid(png_ptr, info_ptr, PNG_INFO_gAMA)) {
    cmsCIExyY whitePoint = {0.3127, 0.3290, 1.0};         // D65
    cmsCIExyYTRIPLE primaries = {
      {0.6400, 0.3300, 1.0},
      {0.3000, 0.6000, 1.0},
      {0.1500, 0.0600, 1.0}
    };

#ifndef PNG_NO_READ_cHRM
    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_cHRM)) {
      png_get_cHRM(png_ptr, info_ptr,
                   &whitePoint.x, &whitePoint.y,
                   &primaries.Red.x,   &primaries.Red.y,
                   &primaries.Green.x, &primaries.Green.y,
                   &primaries.Blue.x,  &primaries.Blue.y);

      whitePoint.Y =
        primaries.Red.Y = primaries.Green.Y = primaries.Blue.Y = 1.0;
    }
#endif

    double gammaOfFile;
    LPGAMMATABLE gammaTable[3];

    png_get_gAMA(png_ptr, info_ptr, &gammaOfFile);

    gammaTable[0] = gammaTable[1] = gammaTable[2] =
      cmsBuildGamma(256, 1/gammaOfFile);

    if (!gammaTable[0])
      return nsnull;

    profile = cmsCreateRGBProfile(&whitePoint, &primaries, gammaTable);

    if (profile)
      png_set_gray_to_rgb(png_ptr);

    cmsFreeGamma(gammaTable[0]);
  }

  if (profile) {
    PRUint32 profileSpace = cmsGetColorSpace(profile);
    if (profileSpace == icSigGrayData) {
      if (color_type & PNG_COLOR_MASK_ALPHA)
        *inType = TYPE_GRAYA_8;
      else
        *inType = TYPE_GRAY_8;
    } else {
      if (color_type & PNG_COLOR_MASK_ALPHA ||
          png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
        *inType = TYPE_RGBA_8;
      else
        *inType = TYPE_RGB_8;
    }
  }

  return profile;
}

void
info_callback(png_structp png_ptr, png_infop info_ptr)
{
/*  int number_passes;   NOT USED  */
  png_uint_32 width, height;
  int bit_depth, color_type, interlace_type, compression_type, filter_type;
  int channels;
  double aGamma;

  png_bytep trans = NULL;
  int num_trans = 0;

  nsPNGDecoder *decoder = static_cast<nsPNGDecoder*>(png_get_progressive_ptr(png_ptr));

  /* always decode to 24-bit RGB or 32-bit RGBA  */
  png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
               &interlace_type, &compression_type, &filter_type);
  
  /* limit image dimensions (bug #251381) */
#define MOZ_PNG_MAX_DIMENSION 1000000L
  if (width > MOZ_PNG_MAX_DIMENSION || height > MOZ_PNG_MAX_DIMENSION)
    longjmp(decoder->mPNG->jmpbuf, 1);
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

  PRUint32 inType, intent;
  if (gfxPlatform::IsCMSEnabled()) {
    decoder->mInProfile = PNGGetColorProfile(png_ptr, info_ptr,
                                             color_type, &inType, &intent);
  }
  if (decoder->mInProfile && gfxPlatform::GetCMSOutputProfile()) {
    PRUint32 outType;

    if (color_type & PNG_COLOR_MASK_ALPHA || num_trans)
      outType = TYPE_RGBA_8;
    else
      outType = TYPE_RGB_8;

    decoder->mTransform = cmsCreateTransform(decoder->mInProfile,
                                             inType,
                                             gfxPlatform::GetCMSOutputProfile(),
                                             outType,
                                             intent,
                                             0);
  } else {
    png_set_gray_to_rgb(png_ptr);
    if (gfxPlatform::IsCMSEnabled()) {
      if (color_type & PNG_COLOR_MASK_ALPHA || num_trans)
        decoder->mTransform = gfxPlatform::GetCMSRGBATransform();
      else
        decoder->mTransform = gfxPlatform::GetCMSRGBTransform();
    }
  }

  if (!decoder->mTransform) {
    png_set_gray_to_rgb(png_ptr);
    if (png_get_gAMA(png_ptr, info_ptr, &aGamma)) {
      if ((aGamma <= 0.0) || (aGamma > 21474.83)) {
        aGamma = 0.45455;
        png_set_gAMA(png_ptr, info_ptr, aGamma);
      }
      png_set_gamma(png_ptr, 2.2, aGamma);
    }
    else
      png_set_gamma(png_ptr, 2.2, 0.45455);
  }

  /* let libpng expand interlaced images */
  if (interlace_type == PNG_INTERLACE_ADAM7) {
    /* number_passes = */
    png_set_interlace_handling(png_ptr);
  }

  /* now all of those things we set above are used to update various struct
   * members and whatnot, after which we can get channels, rowbytes, etc. */
  png_read_update_info(png_ptr, info_ptr);
  decoder->mChannels = channels = png_get_channels(png_ptr, info_ptr);

  /*---------------------------------------------------------------*/
  /* copy PNG info into imagelib structs (formerly png_set_dims()) */
  /*---------------------------------------------------------------*/

  PRInt32 alpha_bits = 1;

  if (channels == 2 || channels == 4) {
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

  if (decoder->mObserver)
    decoder->mObserver->OnStartDecode(nsnull);

  /* The image container may already exist if it is reloading itself from us.
   * Check that it has the same width/height; otherwise create a new container.
   */
  PRInt32 containerWidth, containerHeight;
  decoder->mImage->GetWidth(&containerWidth);
  decoder->mImage->GetHeight(&containerHeight);
  if (containerWidth == 0 && containerHeight == 0) {
    // the image hasn't been inited yet
    decoder->mImage->Init(width, height, decoder->mObserver);
  } else if (containerWidth != width || containerHeight != height) {
    longjmp(decoder->mPNG->jmpbuf, 5); // NS_ERROR_UNEXPECTED
  }

  if (decoder->mObserver)
    decoder->mObserver->OnStartContainer(nsnull, decoder->mImage);

  if (channels == 1 || channels == 3) {
    decoder->format = gfxIFormats::RGB;
  } else if (channels == 2 || channels == 4) {
    if (alpha_bits == 8) {
      decoder->format = gfxIFormats::RGB_A8;
    } else if (alpha_bits == 1) {
      decoder->format = gfxIFormats::RGB_A1;
    }
  }

  if (png_get_valid(png_ptr, info_ptr, PNG_INFO_acTL))
    png_set_progressive_frame_fn(png_ptr, frame_info_callback, NULL);
  
  if (png_get_first_frame_is_hidden(png_ptr, info_ptr)) {
    decoder->mFrameIsHidden = PR_TRUE;
  } else {
    decoder->CreateFrame(0, 0, width, height, decoder->format);
  }
  
  if (decoder->mTransform &&
      (channels <= 2 || interlace_type == PNG_INTERLACE_ADAM7)) {
    PRUint32 bpp[] = { 0, 3, 4, 3, 4 };
    decoder->mCMSLine =
      (PRUint8 *)nsMemory::Alloc(bpp[channels] * width);
    if (!decoder->mCMSLine)
      longjmp(decoder->mPNG->jmpbuf, 5); // NS_ERROR_OUT_OF_MEMORY
  }

  if (interlace_type == PNG_INTERLACE_ADAM7) {
    decoder->interlacebuf = (PRUint8 *)nsMemory::Alloc(channels * width * height);
    if (!decoder->interlacebuf) {
      longjmp(decoder->mPNG->jmpbuf, 5); // NS_ERROR_OUT_OF_MEMORY
    }
  }
  
  if (png_get_first_frame_is_hidden(png_ptr, info_ptr))
    decoder->mFrame = nsnull;
  
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
  nsPNGDecoder *decoder = static_cast<nsPNGDecoder*>(png_get_progressive_ptr(png_ptr));
  
  // skip this frame
  if (decoder->mFrameIsHidden)
    return;

  if (new_row) {
    PRInt32 width;
    decoder->mFrame->GetWidth(&width);
    PRUint32 iwidth = width;

    png_bytep line = new_row;
    if (decoder->interlacebuf) {
      line = decoder->interlacebuf + (row_num * decoder->mChannels * width);
      png_progressive_combine_row(png_ptr, line, new_row);
    }

    // we're thebes. we can write stuff directly to the data
    PRUint8 *imageData;
    PRUint32 imageDataLength, bpr = width * sizeof(PRUint32);
    decoder->mFrame->GetImageData(&imageData, &imageDataLength);
    PRUint32 *cptr32 = (PRUint32*)(imageData + (row_num*bpr));
    PRBool rowHasNoAlpha = PR_TRUE;

    if (decoder->mTransform) {
      if (decoder->mCMSLine) {
        cmsDoTransform(decoder->mTransform, line, decoder->mCMSLine, iwidth);
        /* copy alpha over */
        PRUint32 channels = decoder->mChannels;
        if (channels == 2 || channels == 4) {
          for (PRUint32 i = 0; i < iwidth; i++)
            decoder->mCMSLine[4 * i + 3] = line[channels * i + channels - 1];
        }
        line = decoder->mCMSLine;
      } else {
        cmsDoTransform(decoder->mTransform, line, line, iwidth);
       }
     }

    switch (decoder->format) {
    case gfxIFormats::RGB:
      {
        // counter for while() loops below
        PRUint32 idx = iwidth;

        // copy as bytes until source pointer is 32-bit-aligned
        for (; (NS_PTR_TO_UINT32(line) & 0x3) && idx; --idx) {
          *cptr32++ = GFX_PACKED_PIXEL(0xFF, line[0], line[1], line[2]);
          line += 3; 
        }

        // copy pixels in blocks of 4
        while (idx >= 4) {
          GFX_BLOCK_RGB_TO_FRGB(line, cptr32);
          idx    -=  4;
          line   += 12;
          cptr32 +=  4;
        }

        // copy remaining pixel(s)
        while (idx--) {
          // 32-bit read of final pixel will exceed buffer, so read bytes
          *cptr32++ = GFX_PACKED_PIXEL(0xFF, line[0], line[1], line[2]);
          line += 3;
        }
      }
      break;
    case gfxIFormats::RGB_A1:
      {
        for (PRUint32 x=iwidth; x>0; --x) {
          *cptr32++ = GFX_PACKED_PIXEL(line[3]?0xFF:0x00, line[0], line[1], line[2]);
          if (line[3] == 0)
            rowHasNoAlpha = PR_FALSE;
          line += 4;
        }
      }
      break;
    case gfxIFormats::RGB_A8:
      {
        for (PRUint32 x=width; x>0; --x) {
          *cptr32++ = GFX_PACKED_PIXEL(line[3], line[0], line[1], line[2]);
          if (line[3] != 0xff)
            rowHasNoAlpha = PR_FALSE;
          line += 4;
        }
      }
      break;
    }

    if (!rowHasNoAlpha)
      decoder->mFrameHasNoAlpha = PR_FALSE;

    PRUint32 numFrames = 0;
    decoder->mImage->GetNumFrames(&numFrames);
    if (numFrames <= 1) {
      // Only do incremental image display for the first frame
      nsIntRect r(0, row_num, width, 1);
      nsCOMPtr<nsIImage> img(do_GetInterface(decoder->mFrame));
      img->ImageUpdated(nsnull, nsImageUpdateFlags_kBitsChanged, &r);
      decoder->mObserver->OnDataAvailable(nsnull, decoder->mFrame, &r);
    }
  }
}

// got the header of a new frame that's coming
void
frame_info_callback(png_structp png_ptr, png_uint_32 frame_num)
{
  png_uint_32 x_offset, y_offset;
  PRInt32 width, height;
  
  nsPNGDecoder *decoder = static_cast<nsPNGDecoder*>(png_get_progressive_ptr(png_ptr));
  
  // old frame is done
  if (!decoder->mFrameIsHidden)
    decoder->EndImageFrame();
  
  decoder->mFrameIsHidden = PR_FALSE;
  
  x_offset = png_get_next_frame_x_offset(png_ptr, decoder->mInfo);
  y_offset = png_get_next_frame_y_offset(png_ptr, decoder->mInfo);
  width = png_get_next_frame_width(png_ptr, decoder->mInfo);
  height = png_get_next_frame_height(png_ptr, decoder->mInfo);
  
  decoder->CreateFrame(x_offset, y_offset, width, height, decoder->format);
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

  nsPNGDecoder *decoder = static_cast<nsPNGDecoder*>(png_get_progressive_ptr(png_ptr));
  
  if (png_get_valid(png_ptr, info_ptr, PNG_INFO_acTL)) {
    PRInt32 num_plays = png_get_num_plays(png_ptr, info_ptr);
    decoder->mImage->SetLoopCount(num_plays - 1);
  }
  
  if (!decoder->mFrameIsHidden)
    decoder->EndImageFrame();
  
  decoder->mImage->DecodingComplete();

  if (decoder->mObserver) {
    decoder->mObserver->OnStopContainer(nsnull, decoder->mImage);
    decoder->mObserver->OnStopDecode(nsnull, NS_OK, nsnull);
  }
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
  PR_LOG(gPNGLog, PR_LOG_WARNING, ("libpng warning: %s\n", warning_msg));
}
