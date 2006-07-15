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

#include "imgContainer.h"

NS_IMPL_ISUPPORTS2(imgContainer, imgIContainer, nsIProperties)

//******************************************************************************
imgContainer::imgContainer() :
  mSize(0,0),
  mFrame(nsnull)
{
  mProperties = do_CreateInstance("@mozilla.org/properties;1");
}

//******************************************************************************
imgContainer::~imgContainer()
{
  mFrame = nsnull;
}

//******************************************************************************
/* void init (in PRIn32 aWidth, in PRInt32 aHeight, in imgIContainerObserver aObserver); */
NS_IMETHODIMP imgContainer::Init(PRInt32 aWidth, PRInt32 aHeight, imgIContainerObserver *aObserver)
{
  if (aWidth <= 0 || aHeight <= 0) {
    NS_WARNING("error - negative image size\n");
    return NS_ERROR_FAILURE;
  }

  mSize.SizeTo(aWidth, aHeight);

  return NS_OK;
}

//******************************************************************************
/* readonly attribute gfx_format preferredAlphaChannelFormat; */
NS_IMETHODIMP imgContainer::GetPreferredAlphaChannelFormat(gfx_format *aFormat)
{
  NS_ASSERTION(aFormat, "imgContainer::GetPreferredAlphaChannelFormat; Invalid Arg");
  if (!aFormat)
    return NS_ERROR_INVALID_ARG;

  /* default.. platform's should probably overwrite this */
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

//******************************************************************************
/* readonly attribute gfxIImageFrame currentFrame; */
NS_IMETHODIMP imgContainer::GetCurrentFrame(gfxIImageFrame **aCurrentFrame)
{
  NS_ASSERTION(aCurrentFrame, "imgContainer::GetCurrentFrame; Invalid Arg");
  if (!aCurrentFrame)
    return NS_ERROR_INVALID_POINTER;

  if (mFrame) {
    *aCurrentFrame = mFrame;
    NS_ADDREF(*aCurrentFrame);
    return NS_OK;
  } else {
    *aCurrentFrame = nsnull;
    return NS_ERROR_FAILURE;
  }
}

//******************************************************************************
/* readonly attribute unsigned long numFrames; */
NS_IMETHODIMP imgContainer::GetNumFrames(PRUint32 *aNumFrames)
{
  NS_ASSERTION(aNumFrames, "imgContainer::GetNumFrames; Invalid Arg");
  if (!aNumFrames)
    return NS_ERROR_INVALID_ARG;

  *aNumFrames = mFrame ? 1 : 0;
  return NS_OK;
}

//******************************************************************************
/* gfxIImageFrame getFrameAt (in unsigned long index); */
NS_IMETHODIMP imgContainer::GetFrameAt(PRUint32 index, gfxIImageFrame **_retval)
{
  NS_ASSERTION(_retval, "imgContainer::GetFrameAt; Invalid Arg");
  if (!_retval)
    return NS_ERROR_INVALID_POINTER;

  if (!mFrame || index != 0)
    return NS_ERROR_FAILURE;
  *_retval = mFrame;
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
  NS_ASSERTION(!mFrame, "imgContainer::AppendFrame; Tried to add another frame when only 1 is allowed");

  if (mFrame)
    return NS_ERROR_UNEXPECTED;

  mFrame = item;
  return NS_OK;
}

//******************************************************************************
/* void removeFrame (in gfxIImageFrame item); */
NS_IMETHODIMP imgContainer::RemoveFrame(gfxIImageFrame *item)
{
  return NS_ERROR_NOT_IMPLEMENTED;
}

//******************************************************************************
/* void endFrameDecode (in gfxIImageFrame item, in unsigned long timeout); */
NS_IMETHODIMP imgContainer::EndFrameDecode(PRUint32 aFrameNum, PRUint32 aTimeout)
{
  NS_ASSERTION(aFrameNum == 0, "Received an EndFrameDecode call with an invalid frame number");

  if (aFrameNum != 0 || !mFrame)
    return NS_ERROR_UNEXPECTED;

  mFrame->SetTimeout(aTimeout);

  return NS_OK;
}

//******************************************************************************
/* void decodingComplete (); */
NS_IMETHODIMP imgContainer::DecodingComplete(void)
{
  return NS_OK;
}

//******************************************************************************
/* void clear (); */
NS_IMETHODIMP imgContainer::Clear()
{
  mFrame = nsnull;
  return NS_OK;
}

//******************************************************************************
NS_IMETHODIMP imgContainer::GetAnimationMode(PRUint16 *aAnimationMode)
{
  NS_ASSERTION(aAnimationMode, "imgContainer::GetAnimationMode; Invalid Arg");
  if (!aAnimationMode)
    return NS_ERROR_INVALID_ARG;

  *aAnimationMode = kDontAnimMode;
  return NS_OK;
}

NS_IMETHODIMP imgContainer::SetAnimationMode(PRUint16 aAnimationMode)
{
  // Ignore setting of animation mode
  return NS_OK;
}

//******************************************************************************
/* void startAnimation () */
NS_IMETHODIMP imgContainer::StartAnimation()
{
  return NS_OK;
}

//******************************************************************************
/* void stopAnimation (); */
NS_IMETHODIMP imgContainer::StopAnimation()
{
  return NS_OK;
}

//******************************************************************************
/* attribute long loopCount; */
NS_IMETHODIMP imgContainer::GetLoopCount(PRInt32 *aLoopCount)
{
  return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP imgContainer::SetLoopCount(PRInt32 aLoopCount)
{
  return NS_ERROR_NOT_IMPLEMENTED;
}

//******************************************************************************
/* void resetAnimation (); */
NS_IMETHODIMP imgContainer::ResetAnimation()
{
  return NS_ERROR_NOT_IMPLEMENTED;
}
