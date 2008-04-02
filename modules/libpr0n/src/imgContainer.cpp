/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
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
 *   Stuart Parmenter <pavlov@netscape.com>
 *   Chris Saari <saari@netscape.com>
 *   Asko Tontti <atontti@cc.hut.fi>
 *   Arron Mogge <paper@animecity.nu>
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

#include "nsComponentManagerUtils.h"
#include "imgIContainerObserver.h"
#include "ImageErrors.h"
#include "nsIImage.h"
#include "imgILoad.h"
#include "imgIDecoder.h"
#include "imgIDecoderObserver.h"
#include "imgContainer.h"
#include "nsIInterfaceRequestor.h"
#include "nsIInterfaceRequestorUtils.h"
#include "nsAutoPtr.h"
#include "nsStringStream.h"
#include "prmem.h"
#include "prlog.h"
#include "prenv.h"

#include "gfxContext.h"

/* Accounting for compressed data */
#if defined(PR_LOGGING)
static PRLogModuleInfo *gCompressedImageAccountingLog = PR_NewLogModule ("CompressedImageAccounting");
#else
#define gCompressedImageAccountingLog
#endif

static int num_containers_with_discardable_data;
static PRInt64 num_compressed_image_bytes;


NS_IMPL_ISUPPORTS3(imgContainer, imgIContainer, nsITimerCallback, nsIProperties)

//******************************************************************************
imgContainer::imgContainer() :
  mSize(0,0),
  mNumFrames(0),
  mAnim(nsnull),
  mAnimationMode(kNormalAnimMode),
  mLoopCount(-1),
  mObserver(nsnull),
  mDiscardable(PR_FALSE),
  mDiscarded(PR_FALSE),
  mRestoreDataDone(PR_FALSE),
  mDiscardTimer(nsnull)
{
}

//******************************************************************************
imgContainer::~imgContainer()
{
  if (mAnim)
    delete mAnim;

  if (!mRestoreData.IsEmpty()) {
    num_containers_with_discardable_data--;
    num_compressed_image_bytes -= mRestoreData.Length();

    PR_LOG (gCompressedImageAccountingLog, PR_LOG_DEBUG,
            ("CompressedImageAccounting: destroying imgContainer %p.  "
             "Compressed containers: %d, Compressed data bytes: %lld",
             this,
             num_containers_with_discardable_data,
             num_compressed_image_bytes));
  }

  if (mDiscardTimer) {
    mDiscardTimer->Cancel ();
    mDiscardTimer = nsnull;
  }
}

//******************************************************************************
/* void init (in PRInt32 aWidth, in PRInt32 aHeight, 
              in imgIContainerObserver aObserver); */
NS_IMETHODIMP imgContainer::Init(PRInt32 aWidth, PRInt32 aHeight,
                                 imgIContainerObserver *aObserver)
{
  if (aWidth <= 0 || aHeight <= 0) {
    NS_WARNING("error - negative image size\n");
    return NS_ERROR_FAILURE;
  }

  mSize.SizeTo(aWidth, aHeight);
  
  // As we are reloading it means we are no longer in 'discarded' state
  mDiscarded = PR_FALSE;

  mObserver = do_GetWeakReference(aObserver);
  
  return NS_OK;
}

//******************************************************************************
/* readonly attribute gfx_format preferredAlphaChannelFormat; */
NS_IMETHODIMP imgContainer::GetPreferredAlphaChannelFormat(gfx_format *aFormat)
{
  NS_ASSERTION(aFormat, "imgContainer::GetPreferredAlphaChannelFormat; Invalid Arg");
  if (!aFormat)
    return NS_ERROR_INVALID_ARG;

  /* default.. platforms should probably overwrite this */
  *aFormat = gfxIFormats::RGB_A8;
  return NS_OK;
}

//******************************************************************************
/* readonly attribute PRInt32 width; */
NS_IMETHODIMP imgContainer::GetWidth(PRInt32 *aWidth)
{
  NS_ASSERTION(aWidth, "imgContainer::GetWidth; Invalid Arg");
  if (!aWidth)
    return NS_ERROR_INVALID_ARG;

  *aWidth = mSize.width;
  return NS_OK;
}

//******************************************************************************
/* readonly attribute PRInt32 height; */
NS_IMETHODIMP imgContainer::GetHeight(PRInt32 *aHeight)
{
  NS_ASSERTION(aHeight, "imgContainer::GetHeight; Invalid Arg");
  if (!aHeight)
    return NS_ERROR_INVALID_ARG;

  *aHeight = mSize.height;
  return NS_OK;
}

nsresult imgContainer::GetCurrentFrameNoRef(gfxIImageFrame **aFrame)
{
  nsresult result;

  result = RestoreDiscardedData();
  if (NS_FAILED (result)) {
    PR_LOG (gCompressedImageAccountingLog, PR_LOG_DEBUG,
            ("CompressedImageAccounting: imgContainer::GetCurrentFrameNoRef(): error %d in RestoreDiscardedData(); "
             "returning a null frame from imgContainer %p",
             result,
             this));

    *aFrame = nsnull;
    return result;
  }

  if (!mAnim)
    *aFrame = mFrames.SafeObjectAt(0);
  else if (mAnim->lastCompositedFrameIndex == mAnim->currentAnimationFrameIndex)
    *aFrame = mAnim->compositingFrame;
  else
    *aFrame = mFrames.SafeObjectAt(mAnim->currentAnimationFrameIndex);

  if (!*aFrame)
    PR_LOG (gCompressedImageAccountingLog, PR_LOG_DEBUG,
            ("CompressedImageAccounting: imgContainer::GetCurrentFrameNoRef(): returning null frame from imgContainer %p "
             "(no errors when restoring data)",
              this));

  return NS_OK;
}

//******************************************************************************
/* readonly attribute gfxIImageFrame currentFrame; */
NS_IMETHODIMP imgContainer::GetCurrentFrame(gfxIImageFrame **aCurrentFrame)
{
  nsresult result;

  NS_ASSERTION(aCurrentFrame, "imgContainer::GetCurrentFrame; Invalid Arg");
  if (!aCurrentFrame)
    return NS_ERROR_INVALID_POINTER;
  
  result = GetCurrentFrameNoRef (aCurrentFrame);
  if (NS_FAILED (result))
    return result;

  if (!*aCurrentFrame)
    return NS_ERROR_FAILURE;

  NS_ADDREF(*aCurrentFrame);
  
  return NS_OK;
}

//******************************************************************************
/* readonly attribute unsigned long numFrames; */
NS_IMETHODIMP imgContainer::GetNumFrames(PRUint32 *aNumFrames)
{
  NS_ASSERTION(aNumFrames, "imgContainer::GetNumFrames; Invalid Arg");
  if (!aNumFrames)
    return NS_ERROR_INVALID_ARG;

  *aNumFrames = mNumFrames;
  
  return NS_OK;
}

//******************************************************************************
/* gfxIImageFrame getFrameAt (in unsigned long index); */
NS_IMETHODIMP imgContainer::GetFrameAt(PRUint32 index, gfxIImageFrame **_retval)
{
  nsresult result;

  NS_ASSERTION(_retval, "imgContainer::GetFrameAt; Invalid Arg");
  if (!_retval)
    return NS_ERROR_INVALID_POINTER;

  if (mNumFrames == 0) {
    *_retval = nsnull;
    return NS_OK;
  }

  NS_ENSURE_ARG((int) index < mNumFrames);

  result = RestoreDiscardedData ();
  if (NS_FAILED (result)) {
    *_retval = nsnull;
    return result;
  }
  
  if (!(*_retval = mFrames[index]))
    return NS_ERROR_FAILURE;

  NS_ADDREF(*_retval);
  
  return NS_OK;
}

//******************************************************************************
/* void appendFrame (in gfxIImageFrame item); */
NS_IMETHODIMP imgContainer::AppendFrame(gfxIImageFrame *item)
{
  NS_ASSERTION(item, "imgContainer::AppendFrame; Invalid Arg");
  if (!item)
    return NS_ERROR_INVALID_ARG;

  if (mFrames.Count() == 0) {
    // This may not be an animated image, don't do all the animation stuff.
    mFrames.AppendObject(item);

    mNumFrames++;

    return NS_OK;
  }
  
  if (mFrames.Count() == 1) {
    // Since we're about to add our second frame, initialize animation stuff
    if (!ensureAnimExists())
      return NS_ERROR_OUT_OF_MEMORY;
    
    // If we dispose of the first frame by clearing it, then the
    // First Frame's refresh area is all of itself.
    // RESTORE_PREVIOUS is invalid (assumed to be DISPOSE_CLEAR)
    PRInt32 frameDisposalMethod;
    mFrames[0]->GetFrameDisposalMethod(&frameDisposalMethod);
    if (frameDisposalMethod == imgIContainer::kDisposeClear ||
        frameDisposalMethod == imgIContainer::kDisposeRestorePrevious)
      mFrames[0]->GetRect(mAnim->firstFrameRefreshArea);
  }
  
  // Calculate firstFrameRefreshArea
  // Some gifs are huge but only have a small area that they animate
  // We only need to refresh that small area when Frame 0 comes around again
  nsIntRect itemRect;
  item->GetRect(itemRect);
  mAnim->firstFrameRefreshArea.UnionRect(mAnim->firstFrameRefreshArea, 
                                         itemRect);
  
  mFrames.AppendObject(item);

  mNumFrames++;
  
  // If this is our second frame (We've just added our second frame above),
  // count should now be 2.  This must be called after we AppendObject 
  // because StartAnimation checks for > 1 frames
  if (mFrames.Count() == 2)
    StartAnimation();
  
  return NS_OK;
}

//******************************************************************************
/* void removeFrame (in gfxIImageFrame item); */
NS_IMETHODIMP imgContainer::RemoveFrame(gfxIImageFrame *item)
{
  /* Remember to decrement mNumFrames if you implement this */
  return NS_ERROR_NOT_IMPLEMENTED;
}

//******************************************************************************
/* void endFrameDecode (in unsigned long framenumber, in unsigned long timeout); */
NS_IMETHODIMP imgContainer::EndFrameDecode(PRUint32 aFrameNum, PRUint32 aTimeout)
{
  // Assume there's another frame.
  // currentDecodingFrameIndex is 0 based, aFrameNum is 1 based
  if (mAnim)
    mAnim->currentDecodingFrameIndex = aFrameNum;
  
  return NS_OK;
}

//******************************************************************************
/* void decodingComplete (); */
NS_IMETHODIMP imgContainer::DecodingComplete(void)
{
  if (mAnim)
    mAnim->doneDecoding = PR_TRUE;
  // If there's only 1 frame, optimize it.
  // Optimizing animated images is not supported
  if (mNumFrames == 1)
    mFrames[0]->SetMutable(PR_FALSE);
  return NS_OK;
}

//******************************************************************************
/* void clear (); */
NS_IMETHODIMP imgContainer::Clear()
{
  return NS_ERROR_NOT_IMPLEMENTED;
}

//******************************************************************************
/* attribute unsigned short animationMode; */
NS_IMETHODIMP imgContainer::GetAnimationMode(PRUint16 *aAnimationMode)
{
  NS_ASSERTION(aAnimationMode, "imgContainer::GetAnimationMode; Invalid Arg");
  if (!aAnimationMode)
    return NS_ERROR_INVALID_ARG;
  
  *aAnimationMode = mAnimationMode;
  return NS_OK;
}

//******************************************************************************
/* attribute unsigned short animationMode; */
NS_IMETHODIMP imgContainer::SetAnimationMode(PRUint16 aAnimationMode)
{
  NS_ASSERTION(aAnimationMode == imgIContainer::kNormalAnimMode ||
               aAnimationMode == imgIContainer::kDontAnimMode ||
               aAnimationMode == imgIContainer::kLoopOnceAnimMode,
               "Wrong Animation Mode is being set!");
  
  switch (mAnimationMode = aAnimationMode) {
    case kDontAnimMode:
      StopAnimation();
      break;
    case kNormalAnimMode:
      if (mLoopCount != 0 || 
          (mAnim && (mAnim->currentAnimationFrameIndex + 1 < mNumFrames)))
        StartAnimation();
      break;
    case kLoopOnceAnimMode:
      if (mAnim && (mAnim->currentAnimationFrameIndex + 1 < mNumFrames))
        StartAnimation();
      break;
  }
  
  return NS_OK;
}

//******************************************************************************
/* void startAnimation () */
NS_IMETHODIMP imgContainer::StartAnimation()
{
  if (mAnimationMode == kDontAnimMode || 
      (mAnim && (mAnim->timer || mAnim->animating)))
    return NS_OK;
  
  if (mNumFrames > 1) {
    if (!ensureAnimExists())
      return NS_ERROR_OUT_OF_MEMORY;
    
    PRInt32 timeout;
    nsresult result;
    gfxIImageFrame *currentFrame;

    result = GetCurrentFrameNoRef (&currentFrame);
    if (NS_FAILED (result))
      return result;

    if (currentFrame) {
      currentFrame->GetTimeout(&timeout);
      if (timeout <= 0) // -1 means display this frame forever
        return NS_OK;
    } else
      timeout = 100; // XXX hack.. the timer notify code will do the right
                     //     thing, so just get that started
    
    mAnim->timer = do_CreateInstance("@mozilla.org/timer;1");
    if (!mAnim->timer)
      return NS_ERROR_OUT_OF_MEMORY;
    
    // The only way animating becomes true is if the timer is created
    mAnim->animating = PR_TRUE;
    mAnim->timer->InitWithCallback(static_cast<nsITimerCallback*>(this),
                                   timeout, nsITimer::TYPE_REPEATING_SLACK);
  }
  
  return NS_OK;
}

//******************************************************************************
/* void stopAnimation (); */
NS_IMETHODIMP imgContainer::StopAnimation()
{
  if (mAnim) {
    mAnim->animating = PR_FALSE;

    if (!mAnim->timer)
      return NS_OK;

    mAnim->timer->Cancel();
    mAnim->timer = nsnull;
  }

  return NS_OK;
}

//******************************************************************************
/* void resetAnimation (); */
NS_IMETHODIMP imgContainer::ResetAnimation()
{
  if (mAnimationMode == kDontAnimMode || 
      !mAnim || mAnim->currentAnimationFrameIndex == 0)
    return NS_OK;

  PRBool oldAnimating = mAnim->animating;

  if (mAnim->animating) {
    nsresult rv = StopAnimation();
    if (NS_FAILED(rv))
      return rv;
  }

  mAnim->lastCompositedFrameIndex = -1;
  mAnim->currentAnimationFrameIndex = 0;
  // Update display
  nsCOMPtr<imgIContainerObserver> observer(do_QueryReferent(mObserver));
  if (observer) {
    nsresult result;

    result = RestoreDiscardedData ();
    if (NS_FAILED (result))
      return result;

    observer->FrameChanged(this, mFrames[0], &(mAnim->firstFrameRefreshArea));
  }

  if (oldAnimating)
    return StartAnimation();
  else
    return NS_OK;
}

//******************************************************************************
/* attribute long loopCount; */
NS_IMETHODIMP imgContainer::GetLoopCount(PRInt32 *aLoopCount)
{
  NS_ASSERTION(aLoopCount, "imgContainer::GetLoopCount() called with null ptr");
  if (!aLoopCount)
    return NS_ERROR_INVALID_ARG;
  
  *aLoopCount = mLoopCount;
  
  return NS_OK;
}

//******************************************************************************
/* attribute long loopCount; */
NS_IMETHODIMP imgContainer::SetLoopCount(PRInt32 aLoopCount)
{
  // -1  infinite
  //  0  no looping, one iteration
  //  1  one loop, two iterations
  //  ...
  mLoopCount = aLoopCount;

  return NS_OK;
}

static PRBool
DiscardingEnabled(void)
{
  static PRBool inited;
  static PRBool enabled;

  if (!inited) {
    inited = PR_TRUE;

    enabled = (PR_GetEnv("MOZ_DISABLE_IMAGE_DISCARD") == nsnull);
  }

  return enabled;
}

//******************************************************************************
/* void setDiscardable(in string mime_type); */
NS_IMETHODIMP imgContainer::SetDiscardable(const char* aMimeType)
{
  NS_ASSERTION(aMimeType, "imgContainer::SetDiscardable() called with null aMimeType");

  if (!DiscardingEnabled())
    return NS_OK;

  if (mDiscardable) {
    NS_WARNING ("imgContainer::SetDiscardable(): cannot change an imgContainer which is already discardable");
    return NS_ERROR_FAILURE;
  }

  mDiscardableMimeType.Assign(aMimeType);
  mDiscardable = PR_TRUE;

  num_containers_with_discardable_data++;
  PR_LOG (gCompressedImageAccountingLog, PR_LOG_DEBUG,
          ("CompressedImageAccounting: Making imgContainer %p (%s) discardable.  "
           "Compressed containers: %d, Compressed data bytes: %lld",
           this,
           aMimeType,
           num_containers_with_discardable_data,
           num_compressed_image_bytes));

  return NS_OK;
}

//******************************************************************************
/* void addRestoreData(in nsIInputStream aInputStream, in unsigned long aCount); */
NS_IMETHODIMP imgContainer::AddRestoreData(const char *aBuffer, PRUint32 aCount)
{
  NS_ASSERTION(aBuffer, "imgContainer::AddRestoreData() called with null aBuffer");

  if (!mDiscardable)
    return NS_OK;

  if (mRestoreDataDone) {
    /* We are being called from the decoder while the data is being restored
     * (i.e. we were fully loaded once, then we discarded the image data, then
     * we are being restored).  We don't want to save the compressed data again,
     * since we already have it.
     */
    return NS_OK;
  }

  if (!mRestoreData.AppendElements(aBuffer, aCount))
    return NS_ERROR_OUT_OF_MEMORY;

  num_compressed_image_bytes += aCount;

  PR_LOG (gCompressedImageAccountingLog, PR_LOG_DEBUG,
          ("CompressedImageAccounting: Added compressed data to imgContainer %p (%s).  "
           "Compressed containers: %d, Compressed data bytes: %lld",
           this,
           mDiscardableMimeType.get(),
           num_containers_with_discardable_data,
           num_compressed_image_bytes));

  return NS_OK;
}

/* Note!  buf must be declared as char buf[9]; */
// just used for logging and hashing the header
static void
get_header_str (char *buf, char *data, PRSize data_len)
{
  int i;
  int n;
  static char hex[] = "0123456789abcdef";

  n = data_len < 4 ? data_len : 4;

  for (i = 0; i < n; i++) {
    buf[i * 2]     = hex[(data[i] >> 4) & 0x0f];
    buf[i * 2 + 1] = hex[data[i] & 0x0f];
  }

  buf[i * 2] = 0;
}

//******************************************************************************
/* void restoreDataDone(); */
NS_IMETHODIMP imgContainer::RestoreDataDone (void)
{
  // If image is not discardable, don't start discard timer
  if (!mDiscardable)
    return NS_OK;

  if (mRestoreDataDone)
    return NS_OK;

  mRestoreData.Compact();

  mRestoreDataDone = PR_TRUE;

  if (PR_LOG_TEST(gCompressedImageAccountingLog, PR_LOG_DEBUG)) {
    char buf[9];
    get_header_str(buf, mRestoreData.Elements(), mRestoreData.Length());
    PR_LOG (gCompressedImageAccountingLog, PR_LOG_DEBUG,
            ("CompressedImageAccounting: imgContainer::RestoreDataDone() - data is done for container %p (%s), %d real frames (cached as %d frames) - header %p is 0x%s (length %d)",
             this,
             mDiscardableMimeType.get(),
             mFrames.Count (),
             mNumFrames,
             mRestoreData.Elements(),
             buf,
             mRestoreData.Length()));
  }

  return ResetDiscardTimer();
}

//******************************************************************************
/* void notify(in nsITimer timer); */
NS_IMETHODIMP imgContainer::Notify(nsITimer *timer)
{
  nsresult rv = RestoreDiscardedData();
  NS_ENSURE_SUCCESS(rv, rv);

  // This should never happen since the timer is only set up in StartAnimation()
  // after mAnim is checked to exist.
  NS_ENSURE_TRUE(mAnim, NS_ERROR_UNEXPECTED);
  NS_ASSERTION(mAnim->timer == timer,
               "imgContainer::Notify() called with incorrect timer");

  if (!(mAnim->animating) || !(mAnim->timer))
    return NS_OK;

  nsCOMPtr<imgIContainerObserver> observer(do_QueryReferent(mObserver));
  if (!observer) {
    // the imgRequest that owns us is dead, we should die now too.
    StopAnimation();
    return NS_OK;
  }

  if (mNumFrames == 0)
    return NS_OK;
  
  gfxIImageFrame *nextFrame = nsnull;
  PRInt32 previousFrameIndex = mAnim->currentAnimationFrameIndex;
  PRInt32 nextFrameIndex = mAnim->currentAnimationFrameIndex + 1;
  PRInt32 timeout = 0;

  // If we're done decoding the next frame, go ahead and display it now and
  // reinit the timer with the next frame's delay time.
  // currentDecodingFrameIndex is not set until the second frame has
  // finished decoding (see EndFrameDecode)
  if (mAnim->doneDecoding || 
      (nextFrameIndex < mAnim->currentDecodingFrameIndex)) {
    if (mNumFrames == nextFrameIndex) {
      // End of Animation

      // If animation mode is "loop once", it's time to stop animating
      if (mAnimationMode == kLoopOnceAnimMode || mLoopCount == 0) {
        StopAnimation();
        return NS_OK;
      } else {
        // We may have used compositingFrame to build a frame, and then copied
        // it back into mFrames[..].  If so, delete composite to save memory
        if (mAnim->compositingFrame && mAnim->lastCompositedFrameIndex == -1)
          mAnim->compositingFrame = nsnull;
      }

      nextFrameIndex = 0;
      if (mLoopCount > 0)
        mLoopCount--;
    }

    if (!(nextFrame = mFrames[nextFrameIndex])) {
      // something wrong with the next frame, skip it
      mAnim->currentAnimationFrameIndex = nextFrameIndex;
      mAnim->timer->SetDelay(100);
      return NS_OK;
    }
    nextFrame->GetTimeout(&timeout);

  } else if (nextFrameIndex == mAnim->currentDecodingFrameIndex) {
    // Uh oh, the frame we want to show is currently being decoded (partial)
    // Wait a bit and try again
    mAnim->timer->SetDelay(100);
    return NS_OK;
  } else { //  (nextFrameIndex > currentDecodingFrameIndex)
    // We shouldn't get here. However, if we are requesting a frame
    // that hasn't been decoded yet, go back to the last frame decoded
    NS_WARNING("imgContainer::Notify()  Frame is passed decoded frame");
    nextFrameIndex = mAnim->currentDecodingFrameIndex;
    if (!(nextFrame = mFrames[nextFrameIndex])) {
      // something wrong with the next frame, skip it
      mAnim->currentAnimationFrameIndex = nextFrameIndex;
      mAnim->timer->SetDelay(100);
      return NS_OK;
    }
    nextFrame->GetTimeout(&timeout);
  }

  if (timeout > 0)
    mAnim->timer->SetDelay(timeout);
  else
    StopAnimation();

  nsIntRect dirtyRect;
  gfxIImageFrame *frameToUse = nsnull;

  if (nextFrameIndex == 0) {
    frameToUse = nextFrame;
    dirtyRect = mAnim->firstFrameRefreshArea;
  } else {
    gfxIImageFrame *prevFrame = mFrames[previousFrameIndex];
    if (!prevFrame)
      return NS_OK;

    // Change frame and announce it
    if (NS_FAILED(DoComposite(&frameToUse, &dirtyRect, prevFrame,
                              nextFrame, nextFrameIndex))) {
      // something went wrong, move on to next
      NS_WARNING("imgContainer::Notify(): Composing Frame Failed\n");
      mAnim->currentAnimationFrameIndex = nextFrameIndex;
      return NS_OK;
    }
  }
  // Set currentAnimationFrameIndex at the last possible moment
  mAnim->currentAnimationFrameIndex = nextFrameIndex;
  // Refreshes the screen
  observer->FrameChanged(this, frameToUse, &dirtyRect);
  
  return NS_OK;
}

//******************************************************************************
// DoComposite gets called when the timer for animation get fired and we have to
// update the composited frame of the animation.
nsresult imgContainer::DoComposite(gfxIImageFrame** aFrameToUse,
                                   nsIntRect* aDirtyRect,
                                   gfxIImageFrame* aPrevFrame,
                                   gfxIImageFrame* aNextFrame,
                                   PRInt32 aNextFrameIndex)
{
  NS_ASSERTION(aDirtyRect, "imgContainer::DoComposite aDirtyRect is null");
  NS_ASSERTION(aPrevFrame, "imgContainer::DoComposite aPrevFrame is null");
  NS_ASSERTION(aNextFrame, "imgContainer::DoComposite aNextFrame is null");
  NS_ASSERTION(aFrameToUse, "imgContainer::DoComposite aFrameToUse is null");
  
  PRInt32 prevFrameDisposalMethod;
  aPrevFrame->GetFrameDisposalMethod(&prevFrameDisposalMethod);

  if (prevFrameDisposalMethod == imgIContainer::kDisposeRestorePrevious &&
      !mAnim->compositingPrevFrame)
    prevFrameDisposalMethod = imgIContainer::kDisposeClear;
  nsIntRect prevFrameRect;
  aPrevFrame->GetRect(prevFrameRect);
  PRBool isFullPrevFrame = (prevFrameRect.x == 0 && prevFrameRect.y == 0 &&
                            prevFrameRect.width == mSize.width &&
                            prevFrameRect.height == mSize.height);

  // Optimization: DisposeClearAll if the previous frame is the same size as
  //               container and it's clearing itself
  if (isFullPrevFrame && 
      (prevFrameDisposalMethod == imgIContainer::kDisposeClear))
    prevFrameDisposalMethod = imgIContainer::kDisposeClearAll;

  PRInt32 nextFrameDisposalMethod;
  nsIntRect nextFrameRect;
  aNextFrame->GetFrameDisposalMethod(&nextFrameDisposalMethod);
  aNextFrame->GetRect(nextFrameRect);
  PRBool isFullNextFrame = (nextFrameRect.x == 0 && nextFrameRect.y == 0 &&
                            nextFrameRect.width == mSize.width &&
                            nextFrameRect.height == mSize.height);

  gfx_format nextFormat;
  aNextFrame->GetFormat(&nextFormat);
  if (nextFormat != gfxIFormats::PAL && nextFormat != gfxIFormats::PAL_A1) {
    // Optimization: Skip compositing if the previous frame wants to clear the
    //               whole image
    if (prevFrameDisposalMethod == imgIContainer::kDisposeClearAll) {
      aDirtyRect->SetRect(0, 0, mSize.width, mSize.height);
      *aFrameToUse = aNextFrame;
      return NS_OK;
    }
  
    // Optimization: Skip compositing if this frame is the same size as the
    //               container and it's fully drawing over prev frame (no alpha)
    if (isFullNextFrame &&
        (nextFrameDisposalMethod != imgIContainer::kDisposeRestorePrevious) &&
        (nextFormat == gfxIFormats::RGB)) {
      aDirtyRect->SetRect(0, 0, mSize.width, mSize.height);
      *aFrameToUse = aNextFrame;
      return NS_OK;
    }
  }

  // Calculate area that needs updating
  switch (prevFrameDisposalMethod) {
    default:
    case imgIContainer::kDisposeNotSpecified:
    case imgIContainer::kDisposeKeep:
      *aDirtyRect = nextFrameRect;
      break;

    case imgIContainer::kDisposeClearAll:
      // Whole image container is cleared
      aDirtyRect->SetRect(0, 0, mSize.width, mSize.height);
      break;

    case imgIContainer::kDisposeClear:
      // Calc area that needs to be redrawn (the combination of previous and
      // this frame)
      // XXX - This could be done with multiple framechanged calls
      //       Having prevFrame way at the top of the image, and nextFrame
      //       way at the bottom, and both frames being small, we'd be
      //       telling framechanged to refresh the whole image when only two
      //       small areas are needed.
      aDirtyRect->UnionRect(nextFrameRect, prevFrameRect);
      break;

    case imgIContainer::kDisposeRestorePrevious:
      aDirtyRect->SetRect(0, 0, mSize.width, mSize.height);
      break;
  }

  // Optimization:
  //   Skip compositing if the last composited frame is this frame
  //   (Only one composited frame was made for this animation.  Example:
  //    Only Frame 3 of a 10 frame image required us to build a composite frame
  //    On the second loop, we do not need to rebuild the frame
  //    since it's still sitting in compositingFrame)
  if (mAnim->lastCompositedFrameIndex == aNextFrameIndex) {
    *aFrameToUse = mAnim->compositingFrame;
    return NS_OK;
  }

  PRBool needToBlankComposite = PR_FALSE;

  // Create the Compositing Frame
  if (!mAnim->compositingFrame) {
    nsresult rv;
    mAnim->compositingFrame = do_CreateInstance("@mozilla.org/gfx/image/frame;2", &rv);
    if (NS_FAILED(rv))
      return rv;
    rv = mAnim->compositingFrame->Init(0, 0, mSize.width, mSize.height,
                                       gfxIFormats::RGB_A1, 24);
    if (NS_FAILED(rv)) {
      NS_WARNING("Failed to init compositingFrame!\n");
      mAnim->compositingFrame = nsnull;
      return rv;
    }
    needToBlankComposite = PR_TRUE;
  } else if (aNextFrameIndex == 1) {
    // When we are looping the compositing frame needs to be cleared.
    needToBlankComposite = PR_TRUE;
  }

  // More optimizations possible when next frame is not transparent
  PRBool doDisposal = PR_TRUE;
  if ((nextFormat == gfxIFormats::RGB)||(nextFormat == gfxIFormats::PAL)) {
    if (isFullNextFrame) {
      // Optimization: No need to dispose prev.frame when 
      // next frame is full frame and not transparent.
      doDisposal = PR_FALSE;
      // No need to blank the composite frame
      needToBlankComposite = PR_FALSE;
    } else {
      if ((prevFrameRect.x >= nextFrameRect.x) &&
          (prevFrameRect.y >= nextFrameRect.y) &&
          (prevFrameRect.x + prevFrameRect.width <= nextFrameRect.x + nextFrameRect.width) &&
          (prevFrameRect.y + prevFrameRect.height <= nextFrameRect.y + nextFrameRect.height)) {
        // Optimization: No need to dispose prev.frame when 
        // next frame fully overlaps previous frame.
        doDisposal = PR_FALSE;  
      }
    }      
  }

  if (doDisposal) {
    // Dispose of previous: clear, restore, or keep (copy)
    switch (prevFrameDisposalMethod) {
      case imgIContainer::kDisposeClear:
        if (needToBlankComposite) {
          // If we just created the composite, it could have anything in it's
          // buffer. Clear whole frame
          ClearFrame(mAnim->compositingFrame);
        } else {
          // Only blank out previous frame area (both color & Mask/Alpha)
          ClearFrame(mAnim->compositingFrame, prevFrameRect);
        }
        break;
  
      case imgIContainer::kDisposeClearAll:
        ClearFrame(mAnim->compositingFrame);
        break;
  
      case imgIContainer::kDisposeRestorePrevious:
        // It would be better to copy only the area changed back to
        // compositingFrame.
        if (mAnim->compositingPrevFrame) {
          CopyFrameImage(mAnim->compositingPrevFrame, mAnim->compositingFrame);
  
          // destroy only if we don't need it for this frame's disposal
          if (nextFrameDisposalMethod != imgIContainer::kDisposeRestorePrevious)
            mAnim->compositingPrevFrame = nsnull;
        } else {
          ClearFrame(mAnim->compositingFrame);
        }
        break;
      
      default:
        // Copy previous frame into compositingFrame before we put the new frame on top
        // Assumes that the previous frame represents a full frame (it could be
        // smaller in size than the container, as long as the frame before it erased
        // itself)
        // Note: Frame 1 never gets into DoComposite(), so (aNextFrameIndex - 1) will
        // always be a valid frame number.
        if (mAnim->lastCompositedFrameIndex != aNextFrameIndex - 1) {
          gfx_format prevFormat;
          aPrevFrame->GetFormat(&prevFormat);
          if (isFullPrevFrame && 
              prevFormat != gfxIFormats::PAL && prevFormat != gfxIFormats::PAL_A1) {
            // Just copy the bits
            CopyFrameImage(aPrevFrame, mAnim->compositingFrame);
          } else {
            if (needToBlankComposite) {
              // Only blank composite when prev is transparent or not full.
              if (!isFullPrevFrame ||
                  (prevFormat != gfxIFormats::RGB && prevFormat != gfxIFormats::PAL)) {
                ClearFrame(mAnim->compositingFrame);
              }
            }
            DrawFrameTo(aPrevFrame, mAnim->compositingFrame, prevFrameRect);
          }
        }
    }
  } else if (needToBlankComposite) {
    // If we just created the composite, it could have anything in it's
    // buffers. Clear them
    ClearFrame(mAnim->compositingFrame);
  }

  // Check if the frame we are composing wants the previous image restored afer
  // it is done. Don't store it (again) if last frame wanted its image restored
  // too
  if ((nextFrameDisposalMethod == imgIContainer::kDisposeRestorePrevious) &&
      (prevFrameDisposalMethod != imgIContainer::kDisposeRestorePrevious)) {
    // We are storing the whole image.
    // It would be better if we just stored the area that nextFrame is going to
    // overwrite.
    if (!mAnim->compositingPrevFrame) {
      nsresult rv;
      mAnim->compositingPrevFrame = do_CreateInstance("@mozilla.org/gfx/image/frame;2",
                                                       &rv);
      if (NS_FAILED(rv))
        return rv;
      rv = mAnim->compositingPrevFrame->Init(0, 0, mSize.width, mSize.height,
                                              gfxIFormats::RGB_A1, 24);
      if (NS_FAILED(rv))
        return rv;
    }
    CopyFrameImage(mAnim->compositingFrame, mAnim->compositingPrevFrame);
  }

  // blit next frame into it's correct spot
  DrawFrameTo(aNextFrame, mAnim->compositingFrame, nextFrameRect);
  // Set timeout of CompositeFrame to timeout of frame we just composed
  // Bug 177948
  PRInt32 timeout;
  aNextFrame->GetTimeout(&timeout);
  mAnim->compositingFrame->SetTimeout(timeout);

  // Tell the image that it is fully 'downloaded'.
  nsIntRect r;
  mAnim->compositingFrame->GetRect(r);
  nsCOMPtr<nsIImage> img = do_GetInterface(mAnim->compositingFrame);
  img->ImageUpdated(nsnull, nsImageUpdateFlags_kBitsChanged, &r);

  // We don't want to keep composite images for 8bit frames...
  if (isFullNextFrame && mAnimationMode == kNormalAnimMode && mLoopCount != 0 &&
      nextFormat != gfxIFormats::PAL && nextFormat != gfxIFormats::PAL_A1) {
    // We have a composited full frame
    // Store the composited frame into the mFrames[..] so we don't have to
    // continuously re-build it
    // Then set the previous frame's disposal to CLEAR_ALL so we just draw the
    // frame next time around
    if (CopyFrameImage(mAnim->compositingFrame, aNextFrame)) {
      aPrevFrame->SetFrameDisposalMethod(imgIContainer::kDisposeClearAll);
      mAnim->lastCompositedFrameIndex = -1;
      *aFrameToUse = aNextFrame;
      return NS_OK;
    }
  }

  mAnim->lastCompositedFrameIndex = aNextFrameIndex;
  *aFrameToUse = mAnim->compositingFrame;

  return NS_OK;
}

//******************************************************************************
// Fill aFrame with black. Does also clears the mask.
void imgContainer::ClearFrame(gfxIImageFrame *aFrame)
{
  if (!aFrame)
    return;

  nsCOMPtr<nsIImage> img(do_GetInterface(aFrame));
  nsRefPtr<gfxASurface> surf;
  img->GetSurface(getter_AddRefs(surf));

  // Erase the surface to transparent
  gfxContext ctx(surf);
  ctx.SetOperator(gfxContext::OPERATOR_CLEAR);
  ctx.Paint();
}

//******************************************************************************
void imgContainer::ClearFrame(gfxIImageFrame *aFrame, nsIntRect &aRect)
{
  if (!aFrame || aRect.width <= 0 || aRect.height <= 0) {
    return;
  }

  nsCOMPtr<nsIImage> img(do_GetInterface(aFrame));
  nsRefPtr<gfxASurface> surf;
  img->GetSurface(getter_AddRefs(surf));

  // Erase the destination rectangle to transparent
  gfxContext ctx(surf);
  ctx.SetOperator(gfxContext::OPERATOR_CLEAR);
  ctx.Rectangle(gfxRect(aRect.x, aRect.y, aRect.width, aRect.height));
  ctx.Fill();
}


//******************************************************************************
// Whether we succeed or fail will not cause a crash, and there's not much
// we can do about a failure, so there we don't return a nsresult
PRBool imgContainer::CopyFrameImage(gfxIImageFrame *aSrcFrame,
                                    gfxIImageFrame *aDstFrame)
{
  PRUint8* aDataSrc;
  PRUint8* aDataDest;
  PRUint32 aDataLengthSrc;
  PRUint32 aDataLengthDest;

  if (!aSrcFrame || !aDstFrame)
    return PR_FALSE;

  if (NS_FAILED(aDstFrame->LockImageData()))
    return PR_FALSE;

  // Copy Image Over
  aSrcFrame->GetImageData(&aDataSrc, &aDataLengthSrc);
  aDstFrame->GetImageData(&aDataDest, &aDataLengthDest);
  if (!aDataDest || !aDataSrc || aDataLengthDest != aDataLengthSrc) {
    aDstFrame->UnlockImageData();
    return PR_FALSE;
  }
  memcpy(aDataDest, aDataSrc, aDataLengthSrc);
  aDstFrame->UnlockImageData();

  return PR_TRUE;
}

//******************************************************************************
/* 
 * aSrc is the current frame being drawn,
 * aDst is the composition frame where the current frame is drawn into.
 * aSrcRect is the size of the current frame, and the position of that frame
 *          in the composition frame.
 */
nsresult imgContainer::DrawFrameTo(gfxIImageFrame *aSrc,
                                   gfxIImageFrame *aDst, 
                                   nsIntRect& aSrcRect)
{
  NS_ENSURE_ARG_POINTER(aSrc);
  NS_ENSURE_ARG_POINTER(aDst);

  nsIntRect dstRect;
  aDst->GetRect(dstRect);

  // According to both AGIF and APNG specs, offsets are unsigned
  if (aSrcRect.x < 0 || aSrcRect.y < 0) {
    NS_WARNING("imgContainer::DrawFrameTo: negative offsets not allowed");
    return NS_ERROR_FAILURE;
  }
  // Outside the destination frame, skip it
  if ((aSrcRect.x > dstRect.width) || (aSrcRect.y > dstRect.height)) {
    return NS_OK;
  }
  gfx_format format;
  aSrc->GetFormat(&format);
  if (format == gfxIFormats::PAL || format == gfxIFormats::PAL_A1) {
    // Larger than the destination frame, clip it
    PRInt32 width = PR_MIN(aSrcRect.width, dstRect.width - aSrcRect.x);
    PRInt32 height = PR_MIN(aSrcRect.height, dstRect.height - aSrcRect.y);

    // The clipped image must now fully fit within destination image frame
    NS_ASSERTION((aSrcRect.x >= 0) && (aSrcRect.y >= 0) &&
                 (aSrcRect.x + width <= dstRect.width) &&
                 (aSrcRect.y + height <= dstRect.height),
                "imgContainer::DrawFrameTo: Invalid aSrcRect");

    // clipped image size may be smaller than source, but not larger
    NS_ASSERTION((width <= aSrcRect.width) && (height <= aSrcRect.height),
                 "imgContainer::DrawFrameTo: source must be smaller than dest");

    if (NS_FAILED(aDst->LockImageData()))
      return NS_ERROR_FAILURE;
    // Get pointers to image data
    PRUint32 size;
    PRUint8 *srcPixels;
    gfx_color *colormap;
    gfx_color *dstPixels;

    aSrc->GetImageData(&srcPixels, &size);
    aDst->GetImageData((PRUint8**)&dstPixels, &size);
    aSrc->GetPaletteData(&colormap, &size);
    if (!srcPixels || !dstPixels || !colormap) {
      aDst->UnlockImageData();
      return NS_ERROR_FAILURE;
    }

    // Skip to the right offset
    dstPixels += aSrcRect.x + (aSrcRect.y * dstRect.width);
    if (format == gfxIFormats::PAL) {
      for (PRInt32 r = height; r > 0; --r) {
        for (PRInt32 c = 0; c < width; c++) {
          dstPixels[c] = colormap[srcPixels[c]];
        }
        // Go to the next row in the source resp. destination image
        srcPixels += aSrcRect.width;
        dstPixels += dstRect.width;
      }
    } else {
      // With transparent source, skip transparent pixels
      for (PRInt32 r = height; r > 0; --r) {
        for (PRInt32 c = 0; c < width; c++) {
          const PRUint32 color = colormap[srcPixels[c]];
          if (color)
            dstPixels[c] = color;
        }
        // Go to the next row in the source resp. destination image
        srcPixels += aSrcRect.width;
        dstPixels += dstRect.width;
      }
    }
    aDst->UnlockImageData();
    return NS_OK;
  }

  nsCOMPtr<nsIImage> srcImg(do_GetInterface(aSrc));
  nsRefPtr<gfxASurface> srcSurf;
  srcImg->GetSurface(getter_AddRefs(srcSurf));

  nsCOMPtr<nsIImage> dstImg(do_GetInterface(aDst));
  nsRefPtr<gfxASurface> dstSurf;
  dstImg->GetSurface(getter_AddRefs(dstSurf));

  gfxContext dst(dstSurf);
  dst.Translate(gfxPoint(aSrcRect.x, aSrcRect.y));
  dst.Rectangle(gfxRect(0, 0, aSrcRect.width, aSrcRect.height), PR_TRUE);
  
  // first clear the surface if the blend flag says so
  PRInt32 blendMethod;
  aSrc->GetBlendMethod(&blendMethod);
  if (blendMethod == imgIContainer::kBlendSource) {
    gfxContext::GraphicsOperator defaultOperator = dst.CurrentOperator();
    dst.SetOperator(gfxContext::OPERATOR_CLEAR);
    dst.Fill();
    dst.SetOperator(defaultOperator);
  }
  dst.SetSource(srcSurf);
  dst.Paint();

  return NS_OK;
}


/********* Methods to implement lazy allocation of nsIProperties object *************/
NS_IMETHODIMP imgContainer::Get(const char *prop, const nsIID & iid, void * *result)
{
  if (!mProperties)
    return NS_ERROR_FAILURE;
  return mProperties->Get(prop, iid, result);
}

NS_IMETHODIMP imgContainer::Set(const char *prop, nsISupports *value)
{
  if (!mProperties)
    mProperties = do_CreateInstance("@mozilla.org/properties;1");
  if (!mProperties)
    return NS_ERROR_OUT_OF_MEMORY;
  return mProperties->Set(prop, value);
}

NS_IMETHODIMP imgContainer::Has(const char *prop, PRBool *_retval)
{
  NS_ENSURE_ARG_POINTER(_retval);
  if (!mProperties) {
    *_retval = PR_FALSE;
    return NS_OK;
  }
  return mProperties->Has(prop, _retval);
}

NS_IMETHODIMP imgContainer::Undefine(const char *prop)
{
  if (!mProperties)
    return NS_ERROR_FAILURE;
  return mProperties->Undefine(prop);
}

NS_IMETHODIMP imgContainer::GetKeys(PRUint32 *count, char ***keys)
{
  if (!mProperties) {
    *count = 0;
    *keys = nsnull;
    return NS_OK;
  }
  return mProperties->GetKeys(count, keys);
}

static int
get_discard_timer_ms (void)
{
  /* FIXME: don't hardcode this */
  return 15000; /* 15 seconds */
}

void
imgContainer::sDiscardTimerCallback(nsITimer *aTimer, void *aClosure)
{
  imgContainer *self = (imgContainer *) aClosure;

  NS_ASSERTION(aTimer == self->mDiscardTimer,
               "imgContainer::DiscardTimerCallback() got a callback for an unknown timer");

  self->mDiscardTimer = nsnull;

  int old_frame_count = self->mFrames.Count();

  if (self->mAnim) {
    delete self->mAnim;
    self->mAnim = nsnull;
  }

  self->mFrames.Clear();

  self->mDiscarded = PR_TRUE;

  PR_LOG(gCompressedImageAccountingLog, PR_LOG_DEBUG,
         ("CompressedImageAccounting: discarded uncompressed image data from imgContainer %p (%s) - %d frames (cached count: %d); "
          "Compressed containers: %d, Compressed data bytes: %lld",
          self,
          self->mDiscardableMimeType.get(),
          old_frame_count,
          self->mNumFrames,
          num_containers_with_discardable_data,
          num_compressed_image_bytes));
}

nsresult
imgContainer::ResetDiscardTimer (void)
{
  if (!DiscardingEnabled())
    return NS_OK;

  if (!mDiscardTimer) {
    mDiscardTimer = do_CreateInstance("@mozilla.org/timer;1");
    NS_ENSURE_TRUE(mDiscardTimer, NS_ERROR_OUT_OF_MEMORY);
  } else {
    nsresult rv = mDiscardTimer->Cancel();
    NS_ENSURE_SUCCESS(rv, NS_ERROR_FAILURE);
  }

  return mDiscardTimer->InitWithFuncCallback(sDiscardTimerCallback,
                                             (void *) this,
                                             get_discard_timer_ms (),
                                             nsITimer::TYPE_ONE_SHOT);
}

nsresult
imgContainer::RestoreDiscardedData(void)
{
  // mRestoreDataDone = PR_TRUE means that we want to timeout and then discard the image frames
  // So, we only need to restore, if mRestoreDataDone is true, and then only when the frames are discarded...
  if (!mRestoreDataDone) 
    return NS_OK;

  // Reset timer, as the frames are accessed
  nsresult rv = ResetDiscardTimer();
  NS_ENSURE_SUCCESS(rv, rv);

  if (!mDiscarded)
    return NS_OK;

  int num_expected_frames = mNumFrames;

  // To prevent that ReloadImages is called multiple times, reset the flag before reloading
  mDiscarded = PR_FALSE;

  rv = ReloadImages();
  NS_ENSURE_SUCCESS(rv, rv);

  NS_ASSERTION (mNumFrames == mFrames.Count(),
                "number of restored image frames doesn't match");
  NS_ASSERTION (num_expected_frames == mNumFrames,
                "number of restored image frames doesn't match the original number of frames!");
  
  PR_LOG (gCompressedImageAccountingLog, PR_LOG_DEBUG,
          ("CompressedImageAccounting: imgContainer::RestoreDiscardedData() restored discarded data "
           "for imgContainer %p (%s) - %d image frames.  "
           "Compressed containers: %d, Compressed data bytes: %lld",
           this,
           mDiscardableMimeType.get(),
           mNumFrames,
           num_containers_with_discardable_data,
           num_compressed_image_bytes));

  return NS_OK;
}

class ContainerLoader : public imgILoad,
                        public imgIDecoderObserver,
                        public nsSupportsWeakReference
{
public:

  NS_DECL_ISUPPORTS
  NS_DECL_IMGILOAD
  NS_DECL_IMGIDECODEROBSERVER
  NS_DECL_IMGICONTAINEROBSERVER

  ContainerLoader(void);

private:

  nsCOMPtr<imgIContainer> mContainer;
};

NS_IMPL_ISUPPORTS4 (ContainerLoader, imgILoad, imgIDecoderObserver, imgIContainerObserver, nsISupportsWeakReference)

ContainerLoader::ContainerLoader (void)
{
}

/* Implement imgILoad::image getter */
NS_IMETHODIMP
ContainerLoader::GetImage(imgIContainer **aImage)
{
  *aImage = mContainer;
  NS_IF_ADDREF (*aImage);
  return NS_OK;
}

/* Implement imgILoad::image setter */
NS_IMETHODIMP
ContainerLoader::SetImage(imgIContainer *aImage)
{
  mContainer = aImage;
  return NS_OK;
}

/* Implement imgILoad::isMultiPartChannel getter */
NS_IMETHODIMP
ContainerLoader::GetIsMultiPartChannel(PRBool *aIsMultiPartChannel)
{
  *aIsMultiPartChannel = PR_FALSE; /* FIXME: is this always right? */
  return NS_OK;
}

/* Implement imgIDecoderObserver::onStartRequest() */
NS_IMETHODIMP
ContainerLoader::OnStartRequest(imgIRequest *aRequest)
{
  return NS_OK;
}

/* Implement imgIDecoderObserver::onStartDecode() */
NS_IMETHODIMP
ContainerLoader::OnStartDecode(imgIRequest *aRequest)
{
  return NS_OK;
}

/* Implement imgIDecoderObserver::onStartContainer() */
NS_IMETHODIMP
ContainerLoader::OnStartContainer(imgIRequest *aRequest, imgIContainer *aContainer)
{
  return NS_OK;
}

/* Implement imgIDecoderObserver::onStartFrame() */
NS_IMETHODIMP
ContainerLoader::OnStartFrame(imgIRequest *aRequest, gfxIImageFrame *aFrame)
{
  return NS_OK;
}

/* Implement imgIDecoderObserver::onDataAvailable() */
NS_IMETHODIMP
ContainerLoader::OnDataAvailable(imgIRequest *aRequest, gfxIImageFrame *aFrame, const nsIntRect * aRect)
{
  return NS_OK;
}

/* Implement imgIDecoderObserver::onStopFrame() */
NS_IMETHODIMP
ContainerLoader::OnStopFrame(imgIRequest *aRequest, gfxIImageFrame *aFrame)
{
  return NS_OK;
}

/* Implement imgIDecoderObserver::onStopContainer() */
NS_IMETHODIMP
ContainerLoader::OnStopContainer(imgIRequest *aRequest, imgIContainer *aContainer)
{
  return NS_OK;
}

/* Implement imgIDecoderObserver::onStopDecode() */
NS_IMETHODIMP
ContainerLoader::OnStopDecode(imgIRequest *aRequest, nsresult status, const PRUnichar *statusArg)
{
  return NS_OK;
}

/* Implement imgIDecoderObserver::onStopRequest() */
NS_IMETHODIMP
ContainerLoader::OnStopRequest(imgIRequest *aRequest, PRBool aIsLastPart)
{
  return NS_OK;
}

/* implement imgIContainerObserver::frameChanged() */
NS_IMETHODIMP
ContainerLoader::FrameChanged(imgIContainer *aContainer, gfxIImageFrame *aFrame, nsIntRect * aDirtyRect)
{
  return NS_OK;
}

nsresult
imgContainer::ReloadImages(void)
{
  NS_ASSERTION(!mRestoreData.IsEmpty(),
               "imgContainer::ReloadImages(): mRestoreData should not be empty");
  NS_ASSERTION(mRestoreDataDone,
               "imgContainer::ReloadImages(): mRestoreDataDone shoudl be true!");

  mNumFrames = 0;
  NS_ASSERTION(mFrames.Count() == 0,
               "imgContainer::ReloadImages(): mFrames should be empty");

  nsCAutoString decoderCID(NS_LITERAL_CSTRING("@mozilla.org/image/decoder;2?type=") + mDiscardableMimeType);

  nsCOMPtr<imgIDecoder> decoder = do_CreateInstance(decoderCID.get());
  if (!decoder) {
    PR_LOG(gCompressedImageAccountingLog, PR_LOG_WARNING,
           ("CompressedImageAccounting: imgContainer::ReloadImages() could not create decoder for %s",
            mDiscardableMimeType.get()));
    return NS_IMAGELIB_ERROR_NO_DECODER;
  }

  nsCOMPtr<imgILoad> loader = new ContainerLoader();
  if (!loader) {
    PR_LOG(gCompressedImageAccountingLog, PR_LOG_WARNING,
           ("CompressedImageAccounting: imgContainer::ReloadImages() could not allocate ContainerLoader "
            "when reloading the images for container %p",
            this));
    return NS_ERROR_OUT_OF_MEMORY;
  }

  loader->SetImage(this);

  nsresult result = decoder->Init(loader);
  if (NS_FAILED(result)) {
    PR_LOG(gCompressedImageAccountingLog, PR_LOG_WARNING,
           ("CompressedImageAccounting: imgContainer::ReloadImages() image container %p "
            "failed to initialize the decoder (%s)",
            this,
            mDiscardableMimeType.get()));
    return result;
  }

  nsCOMPtr<nsIInputStream> stream;
  result = NS_NewByteInputStream(getter_AddRefs(stream), mRestoreData.Elements(), mRestoreData.Length(), NS_ASSIGNMENT_DEPEND);
  NS_ENSURE_SUCCESS(result, result);

  if (PR_LOG_TEST(gCompressedImageAccountingLog, PR_LOG_DEBUG)) {
    char buf[9];
    get_header_str(buf, mRestoreData.Elements(), mRestoreData.Length());
    PR_LOG(gCompressedImageAccountingLog, PR_LOG_WARNING,
           ("CompressedImageAccounting: imgContainer::ReloadImages() starting to restore images for container %p (%s) - "
            "header %p is 0x%s (length %d)",
            this,
            mDiscardableMimeType.get(),
            mRestoreData.Elements(),
            buf,
            mRestoreData.Length()));
  }

  PRUint32 written;
  result = decoder->WriteFrom(stream, mRestoreData.Length(), &written);
  NS_ENSURE_SUCCESS(result, result);

  result = decoder->Flush();
  NS_ENSURE_SUCCESS(result, result);

  result = decoder->Close();
  NS_ENSURE_SUCCESS(result, result);

  NS_ASSERTION(mFrames.Count() == mNumFrames,
               "imgContainer::ReloadImages(): the restored mFrames.Count() doesn't match mNumFrames!");

  return result;
}
