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

#include "nsJPEGDecoder.h"

#include "imgIContainerObserver.h"

#include "nsIComponentManager.h"
#include "nsIInputStream.h"

#include "nspr.h"
#include "nsCRT.h"
#include "ImageLogging.h"
#include "nsIImage.h"
#include "nsIInterfaceRequestorUtils.h"

#include "jerror.h"

NS_IMPL_ISUPPORTS1(nsJPEGDecoder, imgIDecoder)

#if defined(PR_LOGGING)
PRLogModuleInfo *gJPEGlog = PR_NewLogModule("JPEGDecoder");
#else
#define gJPEGlog
#endif


METHODDEF(void) init_source (j_decompress_ptr jd);
METHODDEF(boolean) fill_input_buffer (j_decompress_ptr jd);
METHODDEF(void) skip_input_data (j_decompress_ptr jd, long num_bytes);
METHODDEF(void) term_source (j_decompress_ptr jd);
METHODDEF(void) my_error_exit (j_common_ptr cinfo);

/* Normal JFIF markers can't have more bytes than this. */
#define MAX_JPEG_MARKER_LENGTH  (((PRUint32)1 << 16) - 1)


/* Possible states for JPEG source manager */
enum data_source_state {
    READING_BACK = 0, /* Must be zero for init purposes */
    READING_NEW
};

/*
 *  Implementation of a JPEG src object that understands our state machine
 */
typedef struct {
  /* public fields; must be first in this struct! */
  struct jpeg_source_mgr pub;

  nsJPEGDecoder *decoder;

} decoder_source_mgr;


nsJPEGDecoder::nsJPEGDecoder()
{
  mState = JPEG_HEADER;
  mFillState = READING_BACK;

  mSamples = nsnull;
#ifndef MOZ_CAIRO_GFX
  mRGBRow = nsnull;
#endif

  mBytesToSkip = 0;
  
  memset(&mInfo, 0, sizeof(jpeg_decompress_struct));

  mCompletedPasses = 0;

  mBuffer = nsnull;
  mBufferLen = mBufferSize = 0;

  mBackBuffer = nsnull;
  mBackBufferLen = mBackBufferSize = mBackBufferUnreadLen = 0;

}

nsJPEGDecoder::~nsJPEGDecoder()
{
  if (mBuffer)
    PR_Free(mBuffer);
  if (mBackBuffer)
    PR_Free(mBackBuffer);
#ifndef MOZ_CAIRO_GFX
  if (mRGBRow)
    PR_Free(mRGBRow);
#endif
}


/** imgIDecoder methods **/

/* void init (in imgILoad aLoad); */
NS_IMETHODIMP nsJPEGDecoder::Init(imgILoad *aLoad)
{
  mImageLoad = aLoad;
  mObserver = do_QueryInterface(aLoad);

  /* We set up the normal JPEG error routines, then override error_exit. */
  mInfo.err = jpeg_std_error(&mErr.pub);
  /*   mInfo.err = jpeg_std_error(&mErr.pub); */
  mErr.pub.error_exit = my_error_exit;
  /* Establish the setjmp return context for my_error_exit to use. */
  if (setjmp(mErr.setjmp_buffer)) {
    /* If we get here, the JPEG code has signaled an error.
     * We need to clean up the JPEG object, close the input file, and return.
     */
    return NS_ERROR_FAILURE;
  }

  /* Step 1: allocate and initialize JPEG decompression object */
  jpeg_create_decompress(&mInfo);
  
  decoder_source_mgr *src;
  if (mInfo.src == NULL) {
    src = PR_NEWZAP(decoder_source_mgr);
    if (!src) {
      mState = JPEG_ERROR;
      return NS_ERROR_OUT_OF_MEMORY;
    }

    mInfo.src = NS_REINTERPRET_CAST(struct jpeg_source_mgr *, src);
  }

  /* Step 2: specify data source (eg, a file) */

  /* Setup callback functions. */
  src->pub.init_source = init_source;
  src->pub.fill_input_buffer = fill_input_buffer;
  src->pub.skip_input_data = skip_input_data;
  src->pub.resync_to_restart = jpeg_resync_to_restart;
  src->pub.term_source = term_source;

  src->decoder = this;

  return NS_OK;
}


/* void close (); */
NS_IMETHODIMP nsJPEGDecoder::Close()
{
  PR_LOG(gJPEGlog, PR_LOG_DEBUG,
         ("[this=%p] nsJPEGDecoder::Close\n", this));

  if (mState != JPEG_DONE && mState != JPEG_SINK_NON_JPEG_TRAILER)
    NS_WARNING("Never finished decoding the JPEG.");

  /* Step 8: Release JPEG decompression object */
  decoder_source_mgr *src = NS_REINTERPRET_CAST(decoder_source_mgr *, mInfo.src);
  PR_FREEIF(src);
  mInfo.src = nsnull;

  jpeg_destroy_decompress(&mInfo);

  return NS_OK;
}

/* void flush (); */
NS_IMETHODIMP nsJPEGDecoder::Flush()
{
  LOG_SCOPE(gJPEGlog, "nsJPEGDecoder::Flush");

  PRUint32 ret;
  if (mState != JPEG_DONE && mState != JPEG_SINK_NON_JPEG_TRAILER && mState != JPEG_ERROR)
    return this->WriteFrom(nsnull, 0, &ret);

  return NS_OK;
}

/* unsigned long writeFrom (in nsIInputStream inStr, in unsigned long count); */
NS_IMETHODIMP nsJPEGDecoder::WriteFrom(nsIInputStream *inStr, PRUint32 count, PRUint32 *_retval)
{
  LOG_SCOPE_WITH_PARAM(gJPEGlog, "nsJPEGDecoder::WriteFrom", "count", count);

  if (inStr) {
    if (!mBuffer) {
      mBuffer = (JOCTET *)PR_Malloc(count);
      mBufferSize = count;
    } else if (count > mBufferSize) {
      mBuffer = (JOCTET *)PR_Realloc(mBuffer, count);
      mBufferSize = count;
    }

    nsresult rv = inStr->Read((char*)mBuffer, count, &mBufferLen);
    *_retval = mBufferLen;

    //nsresult rv = mOutStream->WriteFrom(inStr, count, _retval);

    NS_ASSERTION(NS_SUCCEEDED(rv), "nsJPEGDecoder::WriteFrom -- mOutStream->WriteFrom failed");
  }
  // else no input stream.. Flush() ?

  /* Return here if there is a fatal error. */
  nsresult error_code;
  if ((error_code = setjmp(mErr.setjmp_buffer)) != 0) {
    mState = JPEG_SINK_NON_JPEG_TRAILER;
    if (error_code == NS_ERROR_FAILURE) {
      /* Error due to corrupt stream - return NS_OK so that libpr0n
         doesn't throw away a partial image load */
      return NS_OK;
    } else {
      /* Error due to reasons external to the stream (probably out of
         memory) - let libpr0n attempt to clean up, even though
         mozilla is seconds away from falling flat on its face. */
      return error_code;
    }
  }

  PR_LOG(gJPEGlog, PR_LOG_DEBUG,
         ("[this=%p] nsJPEGDecoder::WriteFrom -- processing JPEG data\n", this));

  switch (mState) {
  case JPEG_HEADER:
  {
    LOG_SCOPE(gJPEGlog, "nsJPEGDecoder::WriteFrom -- entering JPEG_HEADER case");

    /* Step 3: read file parameters with jpeg_read_header() */
    if (jpeg_read_header(&mInfo, TRUE) == JPEG_SUSPENDED)
      return NS_OK; /* I/O suspension */

    /* let libjpeg take care of gray->RGB and YCbCr->RGB conversions */
    switch (mInfo.jpeg_color_space) {
      case JCS_GRAYSCALE:
      case JCS_RGB:
      case JCS_YCbCr:
        mInfo.out_color_space = JCS_RGB;
        break;
      case JCS_CMYK:
      case JCS_YCCK:
      default:
        mState = JPEG_ERROR;
        return NS_ERROR_UNEXPECTED;
    }

    /*
     * Don't allocate a giant and superfluous memory buffer
     * when the image is a sequential JPEG.
     */
    mInfo.buffered_image = jpeg_has_multiple_scans(&mInfo);

    /* Used to set up image size so arrays can be allocated */
    jpeg_calc_output_dimensions(&mInfo);

    mObserver->OnStartDecode(nsnull);

    /* Check if the request already has an image container.
       this is the case when multipart/x-mixed-replace is being downloaded
       if we already have one and it has the same width and height, reuse it.
     */
    mImageLoad->GetImage(getter_AddRefs(mImage));
    if (mImage) {
      PRInt32 width, height;
      mImage->GetWidth(&width);
      mImage->GetHeight(&height);
      if ((width != (PRInt32)mInfo.image_width) ||
          (height != (PRInt32)mInfo.image_height)) {
        mImage = nsnull;
      }
    }

    if (!mImage) {
      mImage = do_CreateInstance("@mozilla.org/image/container;1");
      if (!mImage) {
        mState = JPEG_ERROR;
        return NS_ERROR_OUT_OF_MEMORY;
      }
      mImageLoad->SetImage(mImage);
      mImage->Init(mInfo.image_width, mInfo.image_height, mObserver);
    }

    mObserver->OnStartContainer(nsnull, mImage);

    mImage->GetFrameAt(0, getter_AddRefs(mFrame));

    PRBool createNewFrame = PR_TRUE;

    if (mFrame) {
      PRInt32 width, height;
      mFrame->GetWidth(&width);
      mFrame->GetHeight(&height);

      if ((width == (PRInt32)mInfo.image_width) &&
          (height == (PRInt32)mInfo.image_height)) {
        createNewFrame = PR_FALSE;
      } else {
        mImage->Clear();
      }
    }

    if (createNewFrame) {
      mFrame = do_CreateInstance("@mozilla.org/gfx/image/frame;2");
      if (!mFrame) {
        mState = JPEG_ERROR;
        return NS_ERROR_OUT_OF_MEMORY;
      }

      gfx_format format = gfxIFormats::RGB;
#if defined(XP_WIN) || defined(XP_OS2) || defined(XP_BEOS)
      format = gfxIFormats::BGR;
#endif

      if (NS_FAILED(mFrame->Init(0, 0, mInfo.image_width, mInfo.image_height, format, 24))) {
        mState = JPEG_ERROR;
        return NS_ERROR_OUT_OF_MEMORY;
      }

      mImage->AppendFrame(mFrame);
    }      

    mObserver->OnStartFrame(nsnull, mFrame);

    /*
     * Make a one-row-high sample array that will go away
     * when done with image. Always make it big enough to
     * hold an RGB row.  Since this uses the IJG memory
     * manager, it must be allocated before the call to
     * jpeg_start_compress().
     */
    int row_stride;

    // Note! row_stride here must match the row_stride in
    // nsJPEGDecoder::OutputScanlines
#if defined(MOZ_CAIRO_GFX) || defined(XP_MAC) || defined(XP_MACOSX)
    row_stride = mInfo.output_width * 4;
#else
    row_stride = mInfo.output_width * 3;
#endif

    mSamples = (*mInfo.mem->alloc_sarray)((j_common_ptr) &mInfo,
                                           JPOOL_IMAGE,
                                           row_stride, 1);

#ifndef MOZ_CAIRO_GFX
#if defined(XP_WIN) || defined(XP_OS2) || defined(XP_BEOS) || defined(XP_MAC) || defined(XP_MACOSX) || defined(MOZ_WIDGET_PHOTON)
    // allocate buffer to do byte flipping / padding
    mRGBRow = (PRUint8*) PR_MALLOC(row_stride);
#endif
#endif

    mState = JPEG_START_DECOMPRESS;
  }

  case JPEG_START_DECOMPRESS:
  {
    LOG_SCOPE(gJPEGlog, "nsJPEGDecoder::WriteFrom -- entering JPEG_START_DECOMPRESS case");
    /* Step 4: set parameters for decompression */

    /* FIXME -- Should reset dct_method and dither mode
     * for final pass of progressive JPEG
     */
    mInfo.dct_method =  JDCT_ISLOW;
    mInfo.dither_mode = JDITHER_FS;
    mInfo.do_fancy_upsampling = TRUE;
    mInfo.enable_2pass_quant = FALSE;
    mInfo.do_block_smoothing = TRUE;

    /* Step 5: Start decompressor */
    if (jpeg_start_decompress(&mInfo) == FALSE)
      return NS_OK; /* I/O suspension */

    /* If this is a progressive JPEG ... */
    if (mInfo.buffered_image) {
      mState = JPEG_DECOMPRESS_PROGRESSIVE;
    } else {
      mState = JPEG_DECOMPRESS_SEQUENTIAL;
    }
  }

  case JPEG_DECOMPRESS_SEQUENTIAL:
  {
    if (mState == JPEG_DECOMPRESS_SEQUENTIAL)
    {
      LOG_SCOPE(gJPEGlog, "nsJPEGDecoder::WriteFrom -- JPEG_DECOMPRESS_SEQUENTIAL case");
      
      if (OutputScanlines() == PR_FALSE)
        return NS_OK; /* I/O suspension */
      
      /* If we've completed image output ... */
      NS_ASSERTION(mInfo.output_scanline == mInfo.output_height, "We didn't process all of the data!");
      mState = JPEG_DONE;
    }
  }

  case JPEG_DECOMPRESS_PROGRESSIVE:
  {
    if (mState == JPEG_DECOMPRESS_PROGRESSIVE)
    {
      LOG_SCOPE(gJPEGlog, "nsJPEGDecoder::WriteFrom -- JPEG_DECOMPRESS_PROGRESSIVE case");

      int status;
      do {
        status = jpeg_consume_input(&mInfo);
      } while ((status != JPEG_SUSPENDED) &&
               (status != JPEG_REACHED_EOI));

      for (;;) {
        if (mInfo.output_scanline == 0) {
          int scan = mInfo.input_scan_number;

          /* if we haven't displayed anything yet (output_scan_number==0)
             and we have enough data for a complete scan, force output
             of the last full scan */
          if ((mInfo.output_scan_number == 0) &&
              (scan > 1) &&
              (status != JPEG_REACHED_EOI))
            scan--;

          if (!jpeg_start_output(&mInfo, scan))
            return NS_OK; /* I/O suspension */
        }

        if (mInfo.output_scanline == 0xffffff)
          mInfo.output_scanline = 0;

        if (OutputScanlines() == PR_FALSE) {
          if (mInfo.output_scanline == 0) {
            /* didn't manage to read any lines - flag so we don't call
               jpeg_start_output() multiple times for the same scan */
            mInfo.output_scanline = 0xffffff;
          }
          return NS_OK; /* I/O suspension */
        }

        if (mInfo.output_scanline == mInfo.output_height)
        {
          if (!jpeg_finish_output(&mInfo))
            return NS_OK; /* I/O suspension */

          if (jpeg_input_complete(&mInfo) &&
              (mInfo.input_scan_number == mInfo.output_scan_number))
            break;

          mInfo.output_scanline = 0;
        }
      }

      mState = JPEG_DONE;
    }
  }

  case JPEG_DONE:
  {
    LOG_SCOPE(gJPEGlog, "nsJPEGDecoder::WriteFrom -- entering JPEG_DONE case");

    /* Step 7: Finish decompression */

    if (jpeg_finish_decompress(&mInfo) == FALSE)
      return NS_OK; /* I/O suspension */

    mState = JPEG_SINK_NON_JPEG_TRAILER;

    /* we're done dude */
    break;
  }
  case JPEG_SINK_NON_JPEG_TRAILER:
    PR_LOG(gJPEGlog, PR_LOG_DEBUG,
           ("[this=%p] nsJPEGDecoder::WriteFrom -- entering JPEG_SINK_NON_JPEG_TRAILER case\n", this));

    break;

  case JPEG_ERROR:
    PR_LOG(gJPEGlog, PR_LOG_DEBUG,
           ("[this=%p] nsJPEGDecoder::WriteFrom -- entering JPEG_ERROR case\n", this));

    break;
  }

  return NS_OK;
}


int
nsJPEGDecoder::OutputScanlines()
{
  PRUint32 top = mInfo.output_scanline;
  PRBool rv = PR_TRUE;

  PRUint32 bpr;
  mFrame->GetImageBytesPerRow(&bpr);

  // Note! row_stride here must match the row_stride in
  // nsJPEGDecoder::WriteFrom
#if defined(MOZ_CAIRO_GFX) || defined(XP_MAC) || defined(XP_MACOSX)
  const int row_stride = mInfo.output_width << 2; // * 4
#else
  const int row_stride = mInfo.output_width * 3;
#endif

#if defined(MOZ_CAIRO_GFX)
  // we're thebes. we can write stuff directly to the data
  PRUint8 *imageData;
  PRUint32 imageDataLength;
  mFrame->GetImageData(&imageData, &imageDataLength);
  nsCOMPtr<nsIImage> img(do_GetInterface(mFrame));
  nsIntRect r(0, 0, mInfo.output_width, 1);
#endif

  while ((mInfo.output_scanline < mInfo.output_height)) {
#ifndef MOZ_CAIRO_GFX
      JSAMPROW samples;
#endif

      /* Request one scanline.  Returns 0 or 1 scanlines. */
      int ns = jpeg_read_scanlines(&mInfo, mSamples, 1);
      
      if (ns != 1) {
        rv = PR_FALSE; /* suspend */
        break;
      }

#if defined(MOZ_CAIRO_GFX)
      PRUint32 offset = (mInfo.output_scanline - 1) * bpr;
      PRUint32 *ptrOutputBuf = (PRUint32*)(imageData + offset);
      JSAMPLE *j1 = mSamples[0];
      for (PRUint32 i=0; i < mInfo.output_width; ++i) {
        PRUint8 r = *j1++;
        PRUint8 g = *j1++;
        PRUint8 b = *j1++;
        *ptrOutputBuf++ = (0xFF << 24) | (r << 16) | (g << 8) | b;
      }
      r.y = mInfo.output_scanline - 1;
      img->ImageUpdated(nsnull, nsImageUpdateFlags_kBitsChanged, &r);
#else

#if defined(XP_WIN) || defined(XP_OS2) || defined(XP_BEOS) || defined(MOZ_WIDGET_PHOTON)
      PRUint8 *ptrOutputBuf = mRGBRow;

      JSAMPLE *j1 = mSamples[0];
      for (PRUint32 i=0;i<mInfo.output_width;++i) {
        ptrOutputBuf[2] = *j1++;
        ptrOutputBuf[1] = *j1++;
        ptrOutputBuf[0] = *j1++;
        ptrOutputBuf += 3;
      }

      samples = mRGBRow;
#elif defined(XP_MAC) || defined(XP_MACOSX)
      PRUint8 *ptrOutputBuf = mRGBRow;

      JSAMPLE *j1 = mSamples[0];
      for (PRUint32 i=0;i<mInfo.output_width;++i) {
        ptrOutputBuf[0] = 0;
        ptrOutputBuf[1] = *j1++;
        ptrOutputBuf[2] = *j1++;
        ptrOutputBuf[3] = *j1++;
        ptrOutputBuf += 4;
      }

      samples = mRGBRow;
#else
      samples = mSamples[0];
#endif

      mFrame->SetImageData(
        samples,             // data
        row_stride,          // length
        (mInfo.output_scanline-1) * bpr); // offset
#endif
  }

  if (top != mInfo.output_scanline) {
      nsIntRect r(0, top, mInfo.output_width, mInfo.output_scanline-top);
      mObserver->OnDataAvailable(nsnull, mFrame, &r);
  }

  return rv;
}


/* Override the standard error method in the IJG JPEG decoder code. */
METHODDEF(void)
my_error_exit (j_common_ptr cinfo)
{
  nsresult error_code;
  decoder_error_mgr *err = (decoder_error_mgr *) cinfo->err;

  /* Convert error to a browser error code */
  switch (cinfo->err->msg_code) {
  case JERR_OUT_OF_MEMORY:
      error_code = NS_ERROR_OUT_OF_MEMORY;
  default:
      error_code = NS_ERROR_FAILURE;
  }

#ifdef DEBUG
  char buffer[JMSG_LENGTH_MAX];

  /* Create the message */
  (*cinfo->err->format_message) (cinfo, buffer);

  fprintf(stderr, "JPEG decoding error:\n%s\n", buffer);
#endif

  /* Return control to the setjmp point. */
  longjmp(err->setjmp_buffer, error_code);
}

/******************************************************************************/
/*-----------------------------------------------------------------------------
 * This is the callback routine from the IJG JPEG library used to supply new
 * data to the decompressor when its input buffer is exhausted.  It juggles
 * multiple buffers in an attempt to avoid unnecessary copying of input data.
 *
 * (A simpler scheme is possible: It's much easier to use only a single
 * buffer; when fill_input_buffer() is called, move any unconsumed data
 * (beyond the current pointer/count) down to the beginning of this buffer and
 * then load new data into the remaining buffer space.  This approach requires
 * a little more data copying but is far easier to get right.)
 *
 * At any one time, the JPEG decompressor is either reading from the necko
 * input buffer, which is volatile across top-level calls to the IJG library,
 * or the "backtrack" buffer.  The backtrack buffer contains the remaining
 * unconsumed data from the necko buffer after parsing was suspended due
 * to insufficient data in some previous call to the IJG library.
 *
 * When suspending, the decompressor will back up to a convenient restart
 * point (typically the start of the current MCU). The variables
 * next_input_byte & bytes_in_buffer indicate where the restart point will be
 * if the current call returns FALSE.  Data beyond this point must be
 * rescanned after resumption, so it must be preserved in case the decompressor
 * decides to backtrack.
 *
 * Returns:
 *  TRUE if additional data is available, FALSE if no data present and
 *   the JPEG library should therefore suspend processing of input stream
 *---------------------------------------------------------------------------*/

/******************************************************************************/
/* data source manager method                                                 */
/******************************************************************************/


/******************************************************************************/
/* data source manager method 
        Initialize source.  This is called by jpeg_read_header() before any
        data is actually read.  May leave
        bytes_in_buffer set to 0 (in which case a fill_input_buffer() call
        will occur immediately).
*/
METHODDEF(void)
init_source (j_decompress_ptr jd)
{
}

/******************************************************************************/
/* data source manager method
        Skip num_bytes worth of data.  The buffer pointer and count should
        be advanced over num_bytes input bytes, refilling the buffer as
        needed.  This is used to skip over a potentially large amount of
        uninteresting data (such as an APPn marker).  In some applications
        it may be possible to optimize away the reading of the skipped data,
        but it's not clear that being smart is worth much trouble; large
        skips are uncommon.  bytes_in_buffer may be zero on return.
        A zero or negative skip count should be treated as a no-op.
*/
METHODDEF(void)
skip_input_data (j_decompress_ptr jd, long num_bytes)
{
  decoder_source_mgr *src = (decoder_source_mgr *)jd->src;

  if (num_bytes > (long)src->pub.bytes_in_buffer) {
    /*
     * Can't skip it all right now until we get more data from
     * network stream. Set things up so that fill_input_buffer
     * will skip remaining amount.
     */
    src->decoder->mBytesToSkip = (size_t)num_bytes - src->pub.bytes_in_buffer;
    src->pub.next_input_byte += src->pub.bytes_in_buffer;
    src->pub.bytes_in_buffer = 0;

  } else {
    /* Simple case. Just advance buffer pointer */

    src->pub.bytes_in_buffer -= (size_t)num_bytes;
    src->pub.next_input_byte += num_bytes;
  }
}


/******************************************************************************/
/* data source manager method
        This is called whenever bytes_in_buffer has reached zero and more
        data is wanted.  In typical applications, it should read fresh data
        into the buffer (ignoring the current state of next_input_byte and
        bytes_in_buffer), reset the pointer & count to the start of the
        buffer, and return TRUE indicating that the buffer has been reloaded.
        It is not necessary to fill the buffer entirely, only to obtain at
        least one more byte.  bytes_in_buffer MUST be set to a positive value
        if TRUE is returned.  A FALSE return should only be used when I/O
        suspension is desired.
*/
METHODDEF(boolean)
fill_input_buffer (j_decompress_ptr jd)
{
  decoder_source_mgr *src = (decoder_source_mgr *)jd->src;

  unsigned char *new_buffer = (unsigned char *)src->decoder->mBuffer;
  PRUint32 new_buflen = src->decoder->mBufferLen;
  PRUint32 bytesToSkip = src->decoder->mBytesToSkip;

  switch(src->decoder->mFillState) {
  case READING_BACK:
    {
      if (!new_buffer || new_buflen == 0)
        return PR_FALSE; /* suspend */

      src->decoder->mBufferLen = 0;

      if (bytesToSkip != 0) {
        if (bytesToSkip < new_buflen) {
          /* All done skipping bytes; Return what's left. */
          new_buffer += bytesToSkip;
          new_buflen -= bytesToSkip;
          src->decoder->mBytesToSkip = 0;
        } else {
          /* Still need to skip some more data in the future */
          src->decoder->mBytesToSkip -= (size_t)new_buflen;
          return PR_FALSE; /* suspend */
        }
      }

      src->decoder->mBackBufferUnreadLen = src->pub.bytes_in_buffer;

      src->pub.next_input_byte = new_buffer;
      src->pub.bytes_in_buffer = (size_t)new_buflen;
      src->decoder->mFillState = READING_NEW;

      return PR_TRUE;
    }
    break;

  case READING_NEW:
    {
      if (src->pub.next_input_byte != src->decoder->mBuffer) {
        /* Backtrack data has been permanently consumed. */
        src->decoder->mBackBufferUnreadLen = 0;
        src->decoder->mBackBufferLen = 0;
      }

      /* Save remainder of netlib buffer in backtrack buffer */
      PRUint32 new_backtrack_buflen = src->pub.bytes_in_buffer + src->decoder->mBackBufferLen;


      /* Make sure backtrack buffer is big enough to hold new data. */
      if (src->decoder->mBackBufferSize < new_backtrack_buflen) {

        /* Round up to multiple of 16 bytes. */
        PRUint32 roundup_buflen = ((new_backtrack_buflen + 15) >> 4) << 4;
        if (src->decoder->mBackBufferSize) {
            src->decoder->mBackBuffer =
          (JOCTET *)PR_REALLOC(src->decoder->mBackBuffer, roundup_buflen);
        } else {
          src->decoder->mBackBuffer = (JOCTET*)PR_MALLOC(roundup_buflen);
        }

        /* Check for OOM */
        if (!src->decoder->mBackBuffer) {
#if 0
          j_common_ptr cinfo = (j_common_ptr)(&src->js->jd);
          cinfo->err->msg_code = JERR_OUT_OF_MEMORY;
          my_error_exit(cinfo);
#endif
        }
          
        src->decoder->mBackBufferSize = (size_t)roundup_buflen;

        /* Check for malformed MARKER segment lengths. */
        if (new_backtrack_buflen > MAX_JPEG_MARKER_LENGTH) {
          my_error_exit((j_common_ptr)(&src->decoder->mInfo));
        }
      }


      /* Copy remainder of netlib buffer into backtrack buffer. */
      memmove(src->decoder->mBackBuffer + src->decoder->mBackBufferLen,
              src->pub.next_input_byte,
              src->pub.bytes_in_buffer);


      /* Point to start of data to be rescanned. */
      src->pub.next_input_byte = src->decoder->mBackBuffer + src->decoder->mBackBufferLen - src->decoder->mBackBufferUnreadLen;
      src->pub.bytes_in_buffer += src->decoder->mBackBufferUnreadLen;
      src->decoder->mBackBufferLen = (size_t)new_backtrack_buflen;
    
      src->decoder->mFillState = READING_BACK;

      return PR_FALSE;
    }
    break;
  }

  return PR_FALSE;
}

/******************************************************************************/
/* data source manager method */
/*
 * Terminate source --- called by jpeg_finish_decompress() after all
 * data has been read to clean up JPEG source manager. NOT called by 
 * jpeg_abort() or jpeg_destroy().
 */
METHODDEF(void)
term_source (j_decompress_ptr jd)
{
  decoder_source_mgr *src = (decoder_source_mgr *)jd->src;

  if (src->decoder->mObserver) {
    src->decoder->mObserver->OnStopFrame(nsnull, src->decoder->mFrame);
    src->decoder->mObserver->OnStopContainer(nsnull, src->decoder->mImage);
    src->decoder->mObserver->OnStopDecode(nsnull, NS_OK, nsnull);
  }

  PRBool isMutable = PR_FALSE;
  if (src->decoder->mImageLoad) 
      src->decoder->mImageLoad->GetIsMultiPartChannel(&isMutable);
  src->decoder->mFrame->SetMutable(isMutable);
}
