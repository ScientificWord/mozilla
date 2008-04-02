/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
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
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   L. David Baron <dbaron@dbaron.org>, Mozilla Corporation
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
 * ***** END LICENSE BLOCK ***** */

/* rendering object for replaced elements with bitmap image data */

#include "nsHTMLParts.h"
#include "nsCOMPtr.h"
#include "nsImageFrame.h"
#include "nsIImageLoadingContent.h"
#include "nsString.h"
#include "nsPrintfCString.h"
#include "nsPresContext.h"
#include "nsIRenderingContext.h"
#include "nsIPresShell.h"
#include "nsIImage.h"
#include "nsIWidget.h"
#include "nsGkAtoms.h"
#include "nsIDocument.h"
#include "nsINodeInfo.h"
#include "nsContentUtils.h"
#include "nsCSSAnonBoxes.h"
#include "nsStyleContext.h"
#include "nsStyleConsts.h"
#include "nsImageMap.h"
#include "nsILinkHandler.h"
#include "nsIURL.h"
#include "nsIIOService.h"
#include "nsIURL.h"
#include "nsILoadGroup.h"
#include "nsISupportsPriority.h"
#include "nsIServiceManager.h"
#include "nsNetUtil.h"
#include "nsHTMLContainerFrame.h"
#include "prprf.h"
#include "nsIFontMetrics.h"
#include "nsCSSRendering.h"
#include "nsILink.h"
#include "nsIDOMHTMLAnchorElement.h"
#include "nsIDOMHTMLImageElement.h"
#include "nsIDeviceContext.h"
#include "nsINameSpaceManager.h"
#include "nsTextFragment.h"
#include "nsIDOMHTMLMapElement.h"
#include "nsImageMapUtils.h"
#include "nsIScriptSecurityManager.h"
#ifdef ACCESSIBILITY
#include "nsIAccessibilityService.h"
#endif
#include "nsIServiceManager.h"
#include "nsIDOMNode.h"
#include "nsGUIEvent.h"
#include "nsLayoutUtils.h"
#include "nsDisplayList.h"

#include "imgIContainer.h"
#include "imgILoader.h"

#include "nsContentPolicyUtils.h"
#include "nsCSSFrameConstructor.h"
#include "nsIPrefBranch2.h"
#include "nsIPrefService.h"
#include "gfxIImageFrame.h"
#include "nsIDOMRange.h"

#include "nsIContentPolicy.h"
#include "nsContentPolicyUtils.h"
#include "nsIEventStateManager.h"
#include "nsLayoutErrors.h"
#include "nsBidiUtils.h"
#include "nsBidiPresUtils.h"

#ifdef DEBUG
#undef NOISY_IMAGE_LOADING
#undef NOISY_ICON_LOADING
#else
#undef NOISY_IMAGE_LOADING
#undef NOISY_ICON_LOADING
#endif

// sizes (pixels) for image icon, padding and border frame
#define ICON_SIZE        (16)
#define ICON_PADDING     (3)
#define ALT_BORDER_WIDTH (1)


//we must add hooks soon
#define IMAGE_EDITOR_CHECK 1

// Default alignment value (so we can tell an unset value from a set value)
#define ALIGN_UNSET PRUint8(-1)

// static icon information
nsImageFrame::IconLoad* nsImageFrame::gIconLoad = nsnull;

// cached IO service for loading icons
nsIIOService* nsImageFrame::sIOService;

// test if the width and height are fixed, looking at the style data
static PRBool HaveFixedSize(const nsStylePosition* aStylePosition)
{
  // check the width and height values in the reflow state's style struct
  // - if width and height are specified as either coord or percentage, then
  //   the size of the image frame is constrained
  nsStyleUnit widthUnit = aStylePosition->mWidth.GetUnit();
  nsStyleUnit heightUnit = aStylePosition->mHeight.GetUnit();

  return ((widthUnit  == eStyleUnit_Coord ||
           widthUnit  == eStyleUnit_Percent) &&
          (heightUnit == eStyleUnit_Coord ||
           heightUnit == eStyleUnit_Percent));
}
// use the data in the reflow state to decide if the image has a constrained size
// (i.e. width and height that are based on the containing block size and not the image size) 
// so we can avoid animated GIF related reflows
inline PRBool HaveFixedSize(const nsHTMLReflowState& aReflowState)
{ 
  NS_ASSERTION(aReflowState.mStylePosition, "crappy reflowState - null stylePosition");
  // when an image has percent css style height or width, but ComputedHeight() 
  // or ComputedWidth() of reflow state is  NS_UNCONSTRAINEDSIZE  
  // it needs to return PR_FALSE to cause an incremental reflow later
  // if an image is inside table like bug 156731 simple testcase III, 
  // during pass 1 reflow, ComputedWidth() is NS_UNCONSTRAINEDSIZE
  // in pass 2 reflow, ComputedWidth() is 0, it also needs to return PR_FALSE
  // see bug 156731
  nsStyleUnit heightUnit = (*(aReflowState.mStylePosition)).mHeight.GetUnit();
  nsStyleUnit widthUnit = (*(aReflowState.mStylePosition)).mWidth.GetUnit();
  return ((eStyleUnit_Percent == heightUnit && NS_UNCONSTRAINEDSIZE == aReflowState.ComputedHeight()) ||
          (eStyleUnit_Percent == widthUnit && (NS_UNCONSTRAINEDSIZE == aReflowState.ComputedWidth() ||
           0 == aReflowState.ComputedWidth())))
          ? PR_FALSE
          : HaveFixedSize(aReflowState.mStylePosition); 
}

nsIFrame*
NS_NewImageFrame(nsIPresShell* aPresShell, nsStyleContext* aContext)
{
  return new (aPresShell) nsImageFrame(aContext);
}


nsImageFrame::nsImageFrame(nsStyleContext* aContext) :
  ImageFrameSuper(aContext),
  mComputedSize(0, 0),
  mIntrinsicSize(0, 0)
{
  // We assume our size is not constrained and we haven't gotten an
  // initial reflow yet, so don't touch those flags.
}

nsImageFrame::~nsImageFrame()
{
}

NS_IMETHODIMP
nsImageFrame::QueryInterface(const nsIID& aIID, void** aInstancePtr)
{
  NS_PRECONDITION(aInstancePtr, "null out param");

  if (aIID.Equals(NS_GET_IID(nsIImageFrame))) {
    *aInstancePtr = static_cast<nsIImageFrame*>(this);
    return NS_OK;
  }

  return ImageFrameSuper::QueryInterface(aIID, aInstancePtr);
}

#ifdef ACCESSIBILITY
NS_IMETHODIMP nsImageFrame::GetAccessible(nsIAccessible** aAccessible)
{
  nsCOMPtr<nsIAccessibilityService> accService = do_GetService("@mozilla.org/accessibilityService;1");

  if (accService) {
    return accService->CreateHTMLImageAccessible(static_cast<nsIFrame*>(this), aAccessible);
  }

  return NS_ERROR_FAILURE;
}
#endif

NS_IMETHODIMP_(nsrefcnt) nsImageFrame::AddRef(void)
{
  NS_WARNING("not supported for frames");
  return 1;
}

NS_IMETHODIMP_(nsrefcnt) nsImageFrame::Release(void)
{
  NS_WARNING("not supported for frames");
  return 1;
}

void
nsImageFrame::Destroy()
{
  // Tell our image map, if there is one, to clean up
  // This causes the nsImageMap to unregister itself as
  // a DOM listener.
  if (mImageMap) {
    mImageMap->Destroy();
    NS_RELEASE(mImageMap);
  }

  // set the frame to null so we don't send messages to a dead object.
  if (mListener) {
    nsCOMPtr<nsIImageLoadingContent> imageLoader = do_QueryInterface(mContent);
    if (imageLoader) {
      imageLoader->RemoveObserver(mListener);
    }
    
    reinterpret_cast<nsImageListener*>(mListener.get())->SetFrame(nsnull);
  }
  
  mListener = nsnull;

  nsSplittableFrame::Destroy();
}



NS_IMETHODIMP
nsImageFrame::Init(nsIContent*      aContent,
                   nsIFrame*        aParent,
                   nsIFrame*        aPrevInFlow)
{
  nsresult rv = nsSplittableFrame::Init(aContent, aParent, aPrevInFlow);
  NS_ENSURE_SUCCESS(rv, rv);

  mListener = new nsImageListener(this);
  if (!mListener) return NS_ERROR_OUT_OF_MEMORY;

  nsCOMPtr<nsIImageLoadingContent> imageLoader = do_QueryInterface(aContent);
  NS_ENSURE_TRUE(imageLoader, NS_ERROR_UNEXPECTED);
  imageLoader->AddObserver(mListener);

  nsPresContext *aPresContext = PresContext();
  
  if (!gIconLoad)
    LoadIcons(aPresContext);

  // Give image loads associated with an image frame a small priority boost!
  nsCOMPtr<imgIRequest> currentRequest;
  imageLoader->GetRequest(nsIImageLoadingContent::CURRENT_REQUEST,
                          getter_AddRefs(currentRequest));
  nsCOMPtr<nsISupportsPriority> p = do_QueryInterface(currentRequest);
  if (p)
    p->AdjustPriority(-1);

  // If we already have an image container, OnStartContainer won't be called
  // Set the animation mode here
  if (currentRequest) {
    nsCOMPtr<imgIContainer> image;
    currentRequest->GetImage(getter_AddRefs(image));
    if (image) {
      image->SetAnimationMode(aPresContext->ImageAnimationMode());
      // Ensure the animation (if any) is started.
      image->StartAnimation();
    }
  }

  return rv;
}

PRBool
nsImageFrame::UpdateIntrinsicSize(imgIContainer* aImage)
{
  NS_PRECONDITION(aImage, "null image");

  PRBool intrinsicSizeChanged = PR_FALSE;
  
  if (aImage) {
    nsSize imageSizeInPx;
    aImage->GetWidth(&imageSizeInPx.width);
    aImage->GetHeight(&imageSizeInPx.height);
    nsSize newSize(nsPresContext::CSSPixelsToAppUnits(imageSizeInPx.width),
                   nsPresContext::CSSPixelsToAppUnits(imageSizeInPx.height));
    if (mIntrinsicSize != newSize) {
      intrinsicSizeChanged = PR_TRUE;
      mIntrinsicSize = newSize;
    }
  }

  return intrinsicSizeChanged;
}

void
nsImageFrame::RecalculateTransform()
{
  // In any case, we need to translate this over appropriately.  Set
  // translation _before_ setting scaling so that it does not get
  // scaled!

  // XXXbz does this introduce rounding errors because of the cast to
  // float?  Should we just manually add that stuff in every time
  // instead?
  nsRect innerArea = GetInnerArea();
  mTransform.SetToTranslate(float(innerArea.x),
                            float(innerArea.y - GetContinuationOffset()));
  
  // Set the scale factors
  if (mIntrinsicSize.width != 0 && mIntrinsicSize.height != 0 &&
      mIntrinsicSize != mComputedSize) {
    mTransform.AddScale(float(mComputedSize.width)  / float(mIntrinsicSize.width),
                        float(mComputedSize.height) / float(mIntrinsicSize.height));
  }
}

/*
 * These two functions basically do the same check.  The first one
 * checks that the given request is the current request for our
 * mContent.  The second checks that the given image container the
 * same as the image container on the current request for our
 * mContent.
 */
PRBool
nsImageFrame::IsPendingLoad(imgIRequest* aRequest) const
{
  // Default to pending load in case of errors
  nsCOMPtr<nsIImageLoadingContent> imageLoader(do_QueryInterface(mContent));
  NS_ASSERTION(imageLoader, "No image loading content?");

  PRInt32 requestType = nsIImageLoadingContent::UNKNOWN_REQUEST;
  imageLoader->GetRequestType(aRequest, &requestType);

  return requestType != nsIImageLoadingContent::CURRENT_REQUEST;
}

PRBool
nsImageFrame::IsPendingLoad(imgIContainer* aContainer) const
{
  //  default to pending load in case of errors
  if (!aContainer) {
    NS_ERROR("No image container!");
    return PR_TRUE;
  }

  nsCOMPtr<nsIImageLoadingContent> imageLoader(do_QueryInterface(mContent));
  NS_ASSERTION(imageLoader, "No image loading content?");
  
  nsCOMPtr<imgIRequest> currentRequest;
  imageLoader->GetRequest(nsIImageLoadingContent::CURRENT_REQUEST,
                          getter_AddRefs(currentRequest));
  if (!currentRequest) {
    NS_ERROR("No current request");
    return PR_TRUE;
  }

  nsCOMPtr<imgIContainer> currentContainer;
  currentRequest->GetImage(getter_AddRefs(currentContainer));

  return currentContainer != aContainer;
  
}

nsRect
nsImageFrame::SourceRectToDest(const nsRect& aRect)
{
  // When scaling the image, row N of the source image may (depending on
  // the scaling function) be used to draw any row in the destination image
  // between floor(F * (N-1)) and ceil(F * (N+1)), where F is the
  // floating-point scaling factor.  The same holds true for columns.
  // So, we start by computing that bound without the floor and ceiling.

  nsRect r(nsPresContext::CSSPixelsToAppUnits(aRect.x - 1),
           nsPresContext::CSSPixelsToAppUnits(aRect.y - 1),
           nsPresContext::CSSPixelsToAppUnits(aRect.width + 2),
           nsPresContext::CSSPixelsToAppUnits(aRect.height + 2));

  mTransform.TransformCoord(&r.x, &r.y, &r.width, &r.height);

  // Now, round the edges out to the pixel boundary.
  int scale = nsPresContext::CSSPixelsToAppUnits(1);
  nscoord right = r.x + r.width;
  nscoord bottom = r.y + r.height;

  r.x -= (scale + (r.x % scale)) % scale;
  r.y -= (scale + (r.y % scale)) % scale;
  r.width = right + ((scale - (right % scale)) % scale) - r.x;
  r.height = bottom + ((scale - (bottom % scale)) % scale) - r.y;

  return r;
}

// Note that we treat NS_EVENT_STATE_SUPPRESSED images as "OK".  This means
// that we'll construct image frames for them as needed if their display is
// toggled from "none" (though we won't paint them, unless their visibility
// is changed too).
#define BAD_STATES (NS_EVENT_STATE_BROKEN | NS_EVENT_STATE_USERDISABLED | \
                    NS_EVENT_STATE_LOADING)

// This is a macro so that we don't evaluate the boolean last arg
// unless we have to; it can be expensive
#define IMAGE_OK(_state, _loadingOK)                        \
   (((_state) & BAD_STATES) == 0 ||                         \
    (((_state) & BAD_STATES) == NS_EVENT_STATE_LOADING &&   \
     (_loadingOK)))

/* static */
PRBool
nsImageFrame::ShouldCreateImageFrameFor(nsIContent* aContent,
                                        nsStyleContext* aStyleContext)
{
  PRInt32 state = aContent->IntrinsicState();
  if (IMAGE_OK(state,
               HaveFixedSize(aStyleContext->GetStylePosition()))) {
    // Image is fine; do the image frame thing
    return PR_TRUE;
  }

  // Check if we want to use a placeholder box with an icon or just
  // let the presShell make us into inline text.  Decide as follows:
  //
  //  - if our special "force icons" style is set, show an icon
  //  - else if our "do not show placeholders" pref is set, skip the icon
  //  - else:
  //  - if QuirksMode, and there is no alt attribute, and this is not an
  //    <object> (which could not possibly have such an attribute), show an
  //    icon.
  //  - if QuirksMode, and the IMG has a size show an icon.
  //  - otherwise, skip the icon
  PRBool useSizedBox;
  
  if (aStyleContext->GetStyleUIReset()->mForceBrokenImageIcon) {
    useSizedBox = PR_TRUE;
  }
  else if (gIconLoad && gIconLoad->mPrefForceInlineAltText) {
    useSizedBox = PR_FALSE;
  }
  else {
    if (aStyleContext->PresContext()->CompatibilityMode() !=
        eCompatibility_NavQuirks) {
      useSizedBox = PR_FALSE;
    }
    else {
      // We are in quirks mode, so we can just check the tag name; no need to
      // check the namespace.
      nsIAtom *localName = aContent->NodeInfo()->NameAtom();

      // Use a sized box if we have no alt text.  This means no alt attribute
      // and the node is not an object or an input (since those always have alt
      // text).
      if (!aContent->HasAttr(kNameSpaceID_None, nsGkAtoms::alt) &&
          localName != nsGkAtoms::object &&
          localName != nsGkAtoms::input) {
        useSizedBox = PR_TRUE;
      }
      else {
        // check whether we have fixed size
        useSizedBox = HaveFixedSize(aStyleContext->GetStylePosition());
      }
    }
  }
  
  return useSizedBox;
}

nsresult
nsImageFrame::OnStartContainer(imgIRequest *aRequest, imgIContainer *aImage)
{
  if (!aImage) return NS_ERROR_INVALID_ARG;

  // handle iconLoads first...
  if (HandleIconLoads(aRequest, PR_FALSE)) {
    return NS_OK;
  }

  /* Get requested animation policy from the pres context:
   *   normal = 0
   *   one frame = 1
   *   one loop = 2
   */
  nsPresContext *presContext = PresContext();
  aImage->SetAnimationMode(presContext->ImageAnimationMode());
  // Ensure the animation (if any) is started.
  aImage->StartAnimation();

  if (IsPendingLoad(aRequest)) {
    // We don't care
    return NS_OK;
  }
  
  UpdateIntrinsicSize(aImage);

  // Now we need to reflow if we have an unconstrained size and have
  // already gotten the initial reflow
  if (!(mState & IMAGE_SIZECONSTRAINED) && (mState & IMAGE_GOTINITIALREFLOW)) { 
    nsIPresShell *presShell = presContext->GetPresShell();
    NS_ASSERTION(presShell, "No PresShell.");
    if (presShell) { 
      presShell->FrameNeedsReflow(this, nsIPresShell::eStyleChange,
                                  NS_FRAME_IS_DIRTY);
    }
  }

  return NS_OK;
}

nsresult
nsImageFrame::OnDataAvailable(imgIRequest *aRequest,
                              gfxIImageFrame *aFrame,
                              const nsRect *aRect)
{
  // XXX do we need to make sure that the reflow from the
  // OnStartContainer has been processed before we start calling
  // invalidate?

  NS_ENSURE_ARG_POINTER(aRect);

  if (!(mState & IMAGE_GOTINITIALREFLOW)) {
    // Don't bother to do anything; we have a reflow coming up!
    return NS_OK;
  }
  
  // XXX We really need to round this out, now that we're doing better
  // image scaling!
  nsRect r = SourceRectToDest(*aRect);

  // handle iconLoads first...
  if (HandleIconLoads(aRequest, PR_FALSE)) {
    // Image changed, invalidate
    Invalidate(r, PR_FALSE);
    return NS_OK;
  }

  if (IsPendingLoad(aRequest)) {
    // We don't care
    return NS_OK;
  }

  // Don't invalidate if the current visible frame isn't the one the data is
  // from
  nsCOMPtr<imgIContainer> container;
  aRequest->GetImage(getter_AddRefs(container));
  if (container) {
    nsCOMPtr<gfxIImageFrame> currentFrame;
    container->GetCurrentFrame(getter_AddRefs(currentFrame));
    if (aFrame != currentFrame) {
      // just bail
      return NS_OK;
    }
  }

#ifdef DEBUG_decode
  printf("Source rect (%d,%d,%d,%d) -> invalidate dest rect (%d,%d,%d,%d)\n",
         aRect->x, aRect->y, aRect->width, aRect->height,
         r.x, r.y, r.width, r.height);
#endif

  Invalidate(r, PR_FALSE);
  
  return NS_OK;
}

nsresult
nsImageFrame::OnStopDecode(imgIRequest *aRequest,
                           nsresult aStatus,
                           const PRUnichar *aStatusArg)
{
  nsPresContext *presContext = PresContext();
  nsIPresShell *presShell = presContext->GetPresShell();
  NS_ASSERTION(presShell, "No PresShell.");

  // handle iconLoads first...
  if (HandleIconLoads(aRequest, NS_SUCCEEDED(aStatus))) {
    return NS_OK;
  }

  // Check what request type we're dealing with
  nsCOMPtr<nsIImageLoadingContent> imageLoader = do_QueryInterface(mContent);
  NS_ASSERTION(imageLoader, "Who's notifying us??");
  PRInt32 loadType = nsIImageLoadingContent::UNKNOWN_REQUEST;
  imageLoader->GetRequestType(aRequest, &loadType);
  if (loadType != nsIImageLoadingContent::CURRENT_REQUEST &&
      loadType != nsIImageLoadingContent::PENDING_REQUEST) {
    return NS_ERROR_FAILURE;
  }

  if (loadType == nsIImageLoadingContent::PENDING_REQUEST) {
    // May have to switch sizes here!
    PRBool intrinsicSizeChanged = PR_TRUE;
    if (NS_SUCCEEDED(aStatus)) {
      nsCOMPtr<imgIContainer> imageContainer;
      aRequest->GetImage(getter_AddRefs(imageContainer));
      NS_ASSERTION(imageContainer, "Successful load with no container?");
      intrinsicSizeChanged = UpdateIntrinsicSize(imageContainer);
    }
    else {
      // Have to size to 0,0 so that GetDesiredSize recalculates the size
      mIntrinsicSize.SizeTo(0, 0);
    }

    if (mState & IMAGE_GOTINITIALREFLOW) { // do nothing if we haven't gotten the initial reflow yet
      if (!(mState & IMAGE_SIZECONSTRAINED) && intrinsicSizeChanged) {
        if (presShell) { 
          presShell->FrameNeedsReflow(this, nsIPresShell::eStyleChange,
                                      NS_FRAME_IS_DIRTY);
        }
      } else {
        nsSize s = GetSize();
        nsRect r(0, 0, s.width, s.height);
        // Update border+content to account for image change
        Invalidate(r, PR_FALSE);
      }
    }
  }

  return NS_OK;
}

nsresult
nsImageFrame::FrameChanged(imgIContainer *aContainer,
                           gfxIImageFrame *aNewFrame,
                           nsRect *aDirtyRect)
{
  if (!GetStyleVisibility()->IsVisible()) {
    return NS_OK;
  }

  if (IsPendingLoad(aContainer)) {
    // We don't care about it
    return NS_OK;
  }
  
  nsRect r = SourceRectToDest(*aDirtyRect);

  // Update border+content to account for image change
  Invalidate(r, PR_FALSE);
  return NS_OK;
}

void
nsImageFrame::EnsureIntrinsicSize(nsPresContext* aPresContext)
{
  // if mIntrinsicSize.width and height are 0, then we should
  // check to see if the size is already known by the image container.
  if (mIntrinsicSize.width == 0 && mIntrinsicSize.height == 0) {
    nsCOMPtr<imgIRequest> currentRequest;
    nsCOMPtr<nsIImageLoadingContent> imageLoader = do_QueryInterface(mContent);
    if (imageLoader) {
      imageLoader->GetRequest(nsIImageLoadingContent::CURRENT_REQUEST,
                              getter_AddRefs(currentRequest));
    }
    nsCOMPtr<imgIContainer> currentContainer;
    if (currentRequest) {
      currentRequest->GetImage(getter_AddRefs(currentContainer));
    }

    if (currentContainer) {
      UpdateIntrinsicSize(currentContainer);
    } else {
      // image request is null or image size not known, probably an
      // invalid image specified
      // - make the image big enough for the icon (it may not be
      // used if inline alt expansion is used instead)
      // XXX: we need this in composer, but it is also good for
      // XXX: general quirks mode to always have room for the icon
      if (aPresContext->CompatibilityMode() == eCompatibility_NavQuirks) {
        mIntrinsicSize.SizeTo(nsPresContext::CSSPixelsToAppUnits(ICON_SIZE+(2*(ICON_PADDING+ALT_BORDER_WIDTH))),
                              nsPresContext::CSSPixelsToAppUnits(ICON_SIZE+(2*(ICON_PADDING+ALT_BORDER_WIDTH))));
      }
    }
  }
}

/* virtual */ nsSize
nsImageFrame::ComputeSize(nsIRenderingContext *aRenderingContext,
                          nsSize aCBSize, nscoord aAvailableWidth,
                          nsSize aMargin, nsSize aBorder, nsSize aPadding,
                          PRBool aShrinkWrap)
{
  nsPresContext *presContext = PresContext();
  EnsureIntrinsicSize(presContext);

  IntrinsicSize intrinsicSize;
  intrinsicSize.width.SetCoordValue(mIntrinsicSize.width);
  intrinsicSize.height.SetCoordValue(mIntrinsicSize.height);

  nsSize& intrinsicRatio = mIntrinsicSize; // won't actually be used

  return nsLayoutUtils::ComputeSizeWithIntrinsicDimensions(
                            aRenderingContext, this,
                            intrinsicSize, intrinsicRatio, aCBSize,
                            aMargin, aBorder, aPadding);
}

nsRect 
nsImageFrame::GetInnerArea() const
{
  return GetContentRect() - GetPosition();
}

// get the offset into the content area of the image where aImg starts if it is a continuation.
nscoord 
nsImageFrame::GetContinuationOffset() const
{
  nscoord offset = 0;
  for (nsIFrame *f = GetPrevInFlow(); f; f = f->GetPrevInFlow()) {
    offset += f->GetContentRect().height;
  }
  NS_ASSERTION(offset >= 0, "bogus GetContentRect");
  return offset;
}

/* virtual */ nscoord
nsImageFrame::GetMinWidth(nsIRenderingContext *aRenderingContext)
{
  // XXX The caller doesn't account for constraints of the height,
  // min-height, and max-height properties.
  nscoord result;
  DISPLAY_MIN_WIDTH(this, result);
  nsPresContext *presContext = PresContext();
  EnsureIntrinsicSize(presContext);
  result = mIntrinsicSize.width;
  return result;
}

/* virtual */ nscoord
nsImageFrame::GetPrefWidth(nsIRenderingContext *aRenderingContext)
{
  // XXX The caller doesn't account for constraints of the height,
  // min-height, and max-height properties.
  nscoord result;
  DISPLAY_PREF_WIDTH(this, result);
  nsPresContext *presContext = PresContext();
  EnsureIntrinsicSize(presContext);
  // convert from normal twips to scaled twips (printing...)
  result = mIntrinsicSize.width;
  return result;
}

/* virtual */ nsSize
nsImageFrame::GetIntrinsicRatio()
{
  EnsureIntrinsicSize(PresContext());
  return mIntrinsicSize;
}

NS_IMETHODIMP
nsImageFrame::Reflow(nsPresContext*          aPresContext,
                     nsHTMLReflowMetrics&     aMetrics,
                     const nsHTMLReflowState& aReflowState,
                     nsReflowStatus&          aStatus)
{
  DO_GLOBAL_REFLOW_COUNT("nsImageFrame");
  DISPLAY_REFLOW(aPresContext, this, aReflowState, aMetrics, aStatus);
  NS_FRAME_TRACE(NS_FRAME_TRACE_CALLS,
                  ("enter nsImageFrame::Reflow: availSize=%d,%d",
                  aReflowState.availableWidth, aReflowState.availableHeight));

  NS_PRECONDITION(mState & NS_FRAME_IN_REFLOW, "frame is not in reflow");

  aStatus = NS_FRAME_COMPLETE;

  // see if we have a frozen size (i.e. a fixed width and height)
  if (HaveFixedSize(aReflowState)) {
    mState |= IMAGE_SIZECONSTRAINED;
  } else {
    mState &= ~IMAGE_SIZECONSTRAINED;
  }

  // XXXldb These two bits are almost exact opposites (except in the
  // middle of the initial reflow); remove IMAGE_GOTINITIALREFLOW.
  if (GetStateBits() & NS_FRAME_FIRST_REFLOW) {
    mState |= IMAGE_GOTINITIALREFLOW;
  }

  mComputedSize = 
    nsSize(aReflowState.ComputedWidth(), aReflowState.ComputedHeight());
  RecalculateTransform();

  aMetrics.width = mComputedSize.width;
  aMetrics.height = mComputedSize.height;

  // add borders and padding
  aMetrics.width  += aReflowState.mComputedBorderPadding.LeftRight();
  aMetrics.height += aReflowState.mComputedBorderPadding.TopBottom();
  
  if (GetPrevInFlow()) {
    aMetrics.width = GetPrevInFlow()->GetSize().width;
    nscoord y = GetContinuationOffset();
    aMetrics.height -= y + aReflowState.mComputedBorderPadding.top;
    aMetrics.height = PR_MAX(0, aMetrics.height);
  }


  // we have to split images if we are:
  //  in Paginated mode, we need to have a constrained height, and have a height larger than our available height
  PRUint32 loadStatus = imgIRequest::STATUS_NONE;
  nsCOMPtr<nsIImageLoadingContent> imageLoader = do_QueryInterface(mContent);
  NS_ASSERTION(imageLoader, "No content node??");
  if (imageLoader) {
    nsCOMPtr<imgIRequest> currentRequest;
    imageLoader->GetRequest(nsIImageLoadingContent::CURRENT_REQUEST,
                            getter_AddRefs(currentRequest));
    if (currentRequest) {
      currentRequest->GetImageStatus(&loadStatus);
    }
  }
  if (aPresContext->IsPaginated() &&
      ((loadStatus & imgIRequest::STATUS_SIZE_AVAILABLE) || (mState & IMAGE_SIZECONSTRAINED)) &&
      NS_UNCONSTRAINEDSIZE != aReflowState.availableHeight && 
      aMetrics.height > aReflowState.availableHeight) { 
    // our desired height was greater than 0, so to avoid infinite
    // splitting, use 1 pixel as the min
    aMetrics.height = PR_MAX(nsPresContext::CSSPixelsToAppUnits(1), aReflowState.availableHeight);
    aStatus = NS_FRAME_NOT_COMPLETE;
  }

  aMetrics.mOverflowArea.SetRect(0, 0, aMetrics.width, aMetrics.height);
  FinishAndStoreOverflow(&aMetrics);

  // Now that that's all done, check whether we're resizing... if we are,
  // invalidate our rect.
  // XXXbz we really only want to do this when reflow is completely done, but
  // we have no way to detect when mRect changes (since SetRect is non-virtual,
  // so this is the best we can do).
  if (mRect.width != aMetrics.width || mRect.height != aMetrics.height) {
    Invalidate(nsRect(0, 0, mRect.width, mRect.height), PR_FALSE);
  }

  NS_FRAME_TRACE(NS_FRAME_TRACE_CALLS,
                  ("exit nsImageFrame::Reflow: size=%d,%d",
                  aMetrics.width, aMetrics.height));
  NS_FRAME_SET_TRUNCATION(aStatus, aReflowState, aMetrics);
  return NS_OK;
}

// Computes the width of the specified string. aMaxWidth specifies the maximum
// width available. Once this limit is reached no more characters are measured.
// The number of characters that fit within the maximum width are returned in
// aMaxFit. NOTE: it is assumed that the fontmetrics have already been selected
// into the rendering context before this is called (for performance). MMP
nscoord
nsImageFrame::MeasureString(const PRUnichar*     aString,
                            PRInt32              aLength,
                            nscoord              aMaxWidth,
                            PRUint32&            aMaxFit,
                            nsIRenderingContext& aContext)
{
  nscoord totalWidth = 0;
  nscoord spaceWidth;
  aContext.SetTextRunRTL(PR_FALSE);
  aContext.GetWidth(' ', spaceWidth);

  aMaxFit = 0;
  while (aLength > 0) {
    // Find the next place we can line break
    PRUint32  len = aLength;
    PRBool    trailingSpace = PR_FALSE;
    for (PRInt32 i = 0; i < aLength; i++) {
      if (XP_IS_SPACE(aString[i]) && (i > 0)) {
        len = i;  // don't include the space when measuring
        trailingSpace = PR_TRUE;
        break;
      }
    }
  
    // Measure this chunk of text, and see if it fits
    nscoord width =
      nsLayoutUtils::GetStringWidth(this, &aContext, aString, len);
    PRBool  fits = (totalWidth + width) <= aMaxWidth;

    // If it fits on the line, or it's the first word we've processed then
    // include it
    if (fits || (0 == totalWidth)) {
      // New piece fits
      totalWidth += width;

      // If there's a trailing space then see if it fits as well
      if (trailingSpace) {
        if ((totalWidth + spaceWidth) <= aMaxWidth) {
          totalWidth += spaceWidth;
        } else {
          // Space won't fit. Leave it at the end but don't include it in
          // the width
          fits = PR_FALSE;
        }

        len++;
      }

      aMaxFit += len;
      aString += len;
      aLength -= len;
    }

    if (!fits) {
      break;
    }
  }
  return totalWidth;
}

// Formats the alt-text to fit within the specified rectangle. Breaks lines
// between words if a word would extend past the edge of the rectangle
void
nsImageFrame::DisplayAltText(nsPresContext*      aPresContext,
                             nsIRenderingContext& aRenderingContext,
                             const nsString&      aAltText,
                             const nsRect&        aRect)
{
  // Set font and color
  aRenderingContext.SetColor(GetStyleColor()->mColor);
  nsLayoutUtils::SetFontFromStyle(&aRenderingContext, mStyleContext);

  // Format the text to display within the formatting rect
  nsIFontMetrics* fm;
  aRenderingContext.GetFontMetrics(fm);

  nscoord maxAscent, maxDescent, height;
  fm->GetMaxAscent(maxAscent);
  fm->GetMaxDescent(maxDescent);
  fm->GetHeight(height);

  // XXX It would be nice if there was a way to have the font metrics tell
  // use where to break the text given a maximum width. At a minimum we need
  // to be able to get the break character...
  const PRUnichar* str = aAltText.get();
  PRInt32          strLen = aAltText.Length();
  nscoord          y = aRect.y;
  // Always show the first line, even if we have to clip it below
  PRBool firstLine = PR_TRUE;
  while ((strLen > 0) && (firstLine || (y + maxDescent) < aRect.YMost())) {
    // Determine how much of the text to display on this line
    PRUint32  maxFit;  // number of characters that fit
    nscoord strWidth = MeasureString(str, strLen, aRect.width, maxFit,
                                     aRenderingContext);
    
    // Display the text
    nsresult rv = NS_ERROR_FAILURE;

    if (aPresContext->BidiEnabled()) {
      nsBidiPresUtils* bidiUtils =  aPresContext->GetBidiUtils();
      
      if (bidiUtils) {
        const nsStyleVisibility* vis = GetStyleVisibility();
        if (vis->mDirection == NS_STYLE_DIRECTION_RTL)
          rv = bidiUtils->RenderText(str, maxFit, NSBIDI_RTL,
                                     aPresContext, aRenderingContext,
                                     aRect.XMost() - strWidth, y + maxAscent);
        else
          rv = bidiUtils->RenderText(str, maxFit, NSBIDI_LTR,
                                     aPresContext, aRenderingContext,
                                     aRect.x, y + maxAscent);
      }
    }
    if (NS_FAILED(rv))
      aRenderingContext.DrawString(str, maxFit, aRect.x, y + maxAscent);

    // Move to the next line
    str += maxFit;
    strLen -= maxFit;
    y += height;
    firstLine = PR_FALSE;
  }

  NS_RELEASE(fm);
}

struct nsRecessedBorder : public nsStyleBorder {
  nsRecessedBorder(nscoord aBorderWidth, nsPresContext* aPresContext)
    : nsStyleBorder(aPresContext)
  {
    NS_FOR_CSS_SIDES(side) {
      // Note: use SetBorderColor here because we want to make sure
      // the "special" flags are unset.
      SetBorderColor(side, NS_RGB(0, 0, 0));
      mBorder.side(side) = aBorderWidth;
      // Note: use SetBorderStyle here because we want to affect
      // mComputedBorder
      SetBorderStyle(side, NS_STYLE_BORDER_STYLE_INSET);
    }
  }
};

void
nsImageFrame::DisplayAltFeedback(nsIRenderingContext& aRenderingContext,
                                 const nsRect&        aDirtyRect,
                                 imgIRequest*         aRequest,
                                 nsPoint              aPt)
{
  // Calculate the inner area
  nsRect  inner = GetInnerArea() + aPt;

  // Display a recessed one pixel border
  nscoord borderEdgeWidth = nsPresContext::CSSPixelsToAppUnits(ALT_BORDER_WIDTH);

  // if inner area is empty, then make it big enough for at least the icon
  if (inner.IsEmpty()){
    inner.SizeTo(2*(nsPresContext::CSSPixelsToAppUnits(ICON_SIZE+ICON_PADDING+ALT_BORDER_WIDTH)),
                 2*(nsPresContext::CSSPixelsToAppUnits(ICON_SIZE+ICON_PADDING+ALT_BORDER_WIDTH)));
  }

  // Make sure we have enough room to actually render the border within
  // our frame bounds
  if ((inner.width < 2 * borderEdgeWidth) || (inner.height < 2 * borderEdgeWidth)) {
    return;
  }

  // Paint the border
  nsRecessedBorder recessedBorder(borderEdgeWidth, PresContext());
  nsCSSRendering::PaintBorder(PresContext(), aRenderingContext, this, inner,
                              inner, recessedBorder, mStyleContext, 0);

  // Adjust the inner rect to account for the one pixel recessed border,
  // and a six pixel padding on each edge
  inner.Deflate(nsPresContext::CSSPixelsToAppUnits(ICON_PADDING+ALT_BORDER_WIDTH), 
                nsPresContext::CSSPixelsToAppUnits(ICON_PADDING+ALT_BORDER_WIDTH));
  if (inner.IsEmpty()) {
    return;
  }

  // Clip so we don't render outside the inner rect
  aRenderingContext.PushState();
  aRenderingContext.SetClipRect(inner, nsClipCombine_kIntersect);

  PRBool dispIcon = gIconLoad ? gIconLoad->mPrefShowPlaceholders : PR_TRUE;

  // Check if we should display image placeholders
  if (dispIcon) {
    const nsStyleVisibility* vis = GetStyleVisibility();
    nscoord size = nsPresContext::CSSPixelsToAppUnits(ICON_SIZE);

    PRBool iconUsed = PR_FALSE;

    // see if the icon images are present...
    if (gIconLoad && gIconLoad->mIconsLoaded) {
      // pick the correct image
      nsCOMPtr<imgIContainer> imgCon;
      if (aRequest) {
        aRequest->GetImage(getter_AddRefs(imgCon));
      }
      if (imgCon) {
        // draw it
        nsRect dest((vis->mDirection == NS_STYLE_DIRECTION_RTL) ?
                    inner.XMost() - size : inner.x,
                    inner.y, size, size);
        nsLayoutUtils::DrawImage(&aRenderingContext, imgCon, dest, aDirtyRect);
        iconUsed = PR_TRUE;
      }
    }

    // if we could not draw the image, then just draw some graffiti
    if (!iconUsed) {
      nscolor oldColor;
      nscoord iconXPos = (vis->mDirection ==   NS_STYLE_DIRECTION_RTL) ?
                         inner.XMost() - size : inner.x;
      nscoord twoPX = nsPresContext::CSSPixelsToAppUnits(2);
      aRenderingContext.DrawRect(iconXPos, inner.y,size,size);
      aRenderingContext.GetColor(oldColor);
      aRenderingContext.SetColor(NS_RGB(0xFF,0,0));
      aRenderingContext.FillEllipse(size/2 + iconXPos, size/2 + inner.y,
                                    size/2 - twoPX, size/2 - twoPX);
      aRenderingContext.SetColor(oldColor);
    }  

    // Reduce the inner rect by the width of the icon, and leave an
    // additional ICON_PADDING pixels for padding
    PRInt32 iconWidth = nsPresContext::CSSPixelsToAppUnits(ICON_SIZE + ICON_PADDING);
    if (vis->mDirection != NS_STYLE_DIRECTION_RTL)
      inner.x += iconWidth;
    inner.width -= iconWidth;
  }

  // If there's still room, display the alt-text
  if (!inner.IsEmpty()) {
    nsIContent* content = GetContent();
    if (content) {
      nsXPIDLString altText;
      nsCSSFrameConstructor::GetAlternateTextFor(content, content->Tag(),
                                                 altText);
      DisplayAltText(PresContext(), aRenderingContext, altText, inner);
    }
  }

  aRenderingContext.PopState();
}

static void PaintAltFeedback(nsIFrame* aFrame, nsIRenderingContext* aCtx,
     const nsRect& aDirtyRect, nsPoint aPt)
{
  nsImageFrame* f = static_cast<nsImageFrame*>(aFrame);
  f->DisplayAltFeedback(*aCtx,
                        aDirtyRect,
                        IMAGE_OK(f->GetContent()->IntrinsicState(), PR_TRUE)
                           ? nsImageFrame::gIconLoad->mLoadingImage
                           : nsImageFrame::gIconLoad->mBrokenImage,
                        aPt);
}

#ifdef NS_DEBUG
static void PaintDebugImageMap(nsIFrame* aFrame, nsIRenderingContext* aCtx,
     const nsRect& aDirtyRect, nsPoint aPt) {
  nsImageFrame* f = static_cast<nsImageFrame*>(aFrame);
  nsRect inner = f->GetInnerArea() + aPt;
  nsPresContext* pc = f->PresContext();

  aCtx->SetColor(NS_RGB(0, 0, 0));
  aCtx->PushState();
  aCtx->Translate(inner.x, inner.y);
  f->GetImageMap(pc)->Draw(pc, *aCtx);
  aCtx->PopState();
}
#endif

/**
 * Note that nsDisplayImage does not receive events. However, an image element
 * is replaced content so its background will be z-adjacent to the
 * image itself, and hence receive events just as if the image itself
 * received events.
 */
class nsDisplayImage : public nsDisplayItem {
public:
  nsDisplayImage(nsImageFrame* aFrame, imgIContainer* aImage)
    : nsDisplayItem(aFrame), mImage(aImage) {
    MOZ_COUNT_CTOR(nsDisplayImage);
  }
  virtual ~nsDisplayImage() {
    MOZ_COUNT_DTOR(nsDisplayImage);
  }
  virtual void Paint(nsDisplayListBuilder* aBuilder, nsIRenderingContext* aCtx,
     const nsRect& aDirtyRect);
  NS_DISPLAY_DECL_NAME("Image")
private:
  nsCOMPtr<imgIContainer> mImage;
};

void
nsDisplayImage::Paint(nsDisplayListBuilder* aBuilder,
     nsIRenderingContext* aCtx, const nsRect& aDirtyRect) {
  static_cast<nsImageFrame*>(mFrame)->
    PaintImage(*aCtx, aBuilder->ToReferenceFrame(mFrame), aDirtyRect, mImage);
}

void
nsImageFrame::PaintImage(nsIRenderingContext& aRenderingContext, nsPoint aPt,
                         const nsRect& aDirtyRect, imgIContainer* aImage)
{
  // Render the image into our content area (the area inside
  // the borders and padding)
  NS_ASSERTION(GetInnerArea().width == mComputedSize.width, "bad width");
  nsRect inner = GetInnerArea() + aPt;
  nsRect clip;
  clip.IntersectRect(inner, aDirtyRect);

  nsRect dest(inner.TopLeft(), mComputedSize);
  dest.y -= GetContinuationOffset();

  nsLayoutUtils::DrawImage(&aRenderingContext, aImage, dest, clip);

  nsPresContext* presContext = PresContext();
  nsImageMap* map = GetImageMap(presContext);
  if (nsnull != map) {
    aRenderingContext.PushState();
    aRenderingContext.SetColor(NS_RGB(0, 0, 0));
    aRenderingContext.SetLineStyle(nsLineStyle_kDotted);
    aRenderingContext.Translate(inner.x, inner.y);
    map->Draw(presContext, aRenderingContext);
    aRenderingContext.PopState();
  }
}

NS_IMETHODIMP
nsImageFrame::BuildDisplayList(nsDisplayListBuilder*   aBuilder,
                               const nsRect&           aDirtyRect,
                               const nsDisplayListSet& aLists)
{
  if (!IsVisibleForPainting(aBuilder))
    return NS_OK;

  // REVIEW: We don't need any special logic here for deciding which layer
  // to put the background in ... it goes in aLists.BorderBackground() and
  // then if we have a block parent, it will put our background in the right
  // place.
  nsresult rv = DisplayBorderBackgroundOutline(aBuilder, aLists);
  NS_ENSURE_SUCCESS(rv, rv);
  // REVIEW: Checking mRect.IsEmpty() makes no sense to me, so I removed it.
  // It can't have been protecting us against bad situations with zero-size
  // images since adding a border would make the rect non-empty.
    
  if (mComputedSize.width != 0 && mComputedSize.height != 0) {
    nsCOMPtr<nsIImageLoadingContent> imageLoader = do_QueryInterface(mContent);
    NS_ASSERTION(imageLoader, "Not an image loading content?");

    nsCOMPtr<imgIRequest> currentRequest;
    if (imageLoader) {
      imageLoader->GetRequest(nsIImageLoadingContent::CURRENT_REQUEST,
                              getter_AddRefs(currentRequest));
    }

    PRInt32 contentState = mContent->IntrinsicState();
    PRBool imageOK = IMAGE_OK(contentState, PR_TRUE);

    nsCOMPtr<imgIContainer> imgCon;
    if (currentRequest) {
      currentRequest->GetImage(getter_AddRefs(imgCon));
    }

    if (!imageOK || !imgCon) {
      // No image yet, or image load failed. Draw the alt-text and an icon
      // indicating the status
      rv = aLists.Content()->AppendNewToTop(new (aBuilder)
          nsDisplayGeneric(this, PaintAltFeedback, "AltFeedback"));
      NS_ENSURE_SUCCESS(rv, rv);
    }
    else {
      rv = aLists.Content()->AppendNewToTop(new (aBuilder)
          nsDisplayImage(this, imgCon));
      NS_ENSURE_SUCCESS(rv, rv);
        
#ifdef DEBUG
      if (GetShowFrameBorders() && GetImageMap(PresContext())) {
        rv = aLists.Outlines()->AppendNewToTop(new (aBuilder)
            nsDisplayGeneric(this, PaintDebugImageMap, "DebugImageMap"));
        NS_ENSURE_SUCCESS(rv, rv);
      }
#endif
    }
  }

  // BBM This is the code that causes images to be overlaid with partially transparent overlays
  // when selected.
  PRInt16 displaySelection = 0;
  nsresult result;
  nsPresContext* presContext = PresContext();
  result = presContext->PresShell()->GetSelectionFlags(&displaySelection);
  if (NS_FAILED(result))
    return result;
  if (!(displaySelection & nsISelectionDisplay::DISPLAY_IMAGES))
    return NS_OK;//no need to check the blue border, we cannot be drawn selected
//insert hook here for image selection drawing
#if IMAGE_EDITOR_CHECK
  //check to see if this frame is in an editor context
  //isEditor check. this needs to be changed to have better way to check
  if (displaySelection == nsISelectionDisplay::DISPLAY_ALL) 
  {
    nsCOMPtr<nsISelectionController> selCon;
    result = GetSelectionController(presContext, getter_AddRefs(selCon));
    if (NS_SUCCEEDED(result) && selCon)
    {
      nsCOMPtr<nsISelection> selection;
      result = selCon->GetSelection(nsISelectionController::SELECTION_NORMAL, getter_AddRefs(selection));
      if (NS_SUCCEEDED(result) && selection)
      {
        PRInt32 rangeCount;
        selection->GetRangeCount(&rangeCount);
        if (rangeCount == 1) //if not one then let code drop to nsFrame::Paint
        {
          nsCOMPtr<nsIContent> parentContent = mContent->GetParent();
          if (parentContent)
          {
            PRInt32 thisOffset = parentContent->IndexOf(mContent);
            nsCOMPtr<nsIDOMNode> parentNode = do_QueryInterface(parentContent);
            nsCOMPtr<nsIDOMNode> rangeNode;
            PRInt32 rangeOffset;
            nsCOMPtr<nsIDOMRange> range;
            selection->GetRangeAt(0,getter_AddRefs(range));
            if (range)
            {
              range->GetStartContainer(getter_AddRefs(rangeNode));
              range->GetStartOffset(&rangeOffset);

              if (parentNode && rangeNode && (rangeNode == parentNode) && rangeOffset == thisOffset)
              {
                range->GetEndContainer(getter_AddRefs(rangeNode));
                range->GetEndOffset(&rangeOffset);
                if ((rangeNode == parentNode) && (rangeOffset == (thisOffset +1))) //+1 since that would mean this whole content is selected only
                  return NS_OK; //do not allow nsFrame do draw any further selection
              }
            }
          }
        }
      }
    }
  }
#endif
  
  return DisplaySelectionOverlay(aBuilder, aLists,
                                 nsISelectionDisplay::DISPLAY_IMAGES);
}

NS_IMETHODIMP
nsImageFrame::GetImageMap(nsPresContext *aPresContext, nsIImageMap **aImageMap)
{
  nsImageMap *map = GetImageMap(aPresContext);
  return CallQueryInterface(map, aImageMap);
}

nsImageMap*
nsImageFrame::GetImageMap(nsPresContext* aPresContext)
{
  if (!mImageMap) {
    nsIDocument* doc = mContent->GetDocument();
    if (!doc) {
      return nsnull;
    }

    nsAutoString usemap;
    mContent->GetAttr(kNameSpaceID_None, nsGkAtoms::usemap, usemap);

    nsCOMPtr<nsIDOMHTMLMapElement> map = nsImageMapUtils::FindImageMap(doc,usemap);
    if (map) {
      mImageMap = new nsImageMap();
      if (mImageMap) {
        NS_ADDREF(mImageMap);
        mImageMap->Init(aPresContext->PresShell(), this, map);
      }
    }
  }

  return mImageMap;
}

PRBool
nsImageFrame::IsServerImageMap()
{
  return mContent->HasAttr(kNameSpaceID_None, nsGkAtoms::ismap);
}

// Translate an point that is relative to our frame
// into a localized pixel coordinate that is relative to the
// content area of this frame (inside the border+padding).
void
nsImageFrame::TranslateEventCoords(const nsPoint& aPoint,
                                   nsIntPoint&     aResult)
{
  nscoord x = aPoint.x;
  nscoord y = aPoint.y;

  // Subtract out border and padding here so that the coordinates are
  // now relative to the content area of this frame.
  nsRect inner = GetInnerArea();
  x -= inner.x;
  y -= inner.y;

  aResult.x = nsPresContext::AppUnitsToIntCSSPixels(x);
  aResult.y = nsPresContext::AppUnitsToIntCSSPixels(y);
}

PRBool
nsImageFrame::GetAnchorHREFTargetAndNode(nsIURI** aHref, nsString& aTarget,
                                         nsIContent** aNode)
{
  PRBool status = PR_FALSE;
  aTarget.Truncate();
  *aHref = nsnull;
  *aNode = nsnull;

  // Walk up the content tree, looking for an nsIDOMAnchorElement
  for (nsIContent* content = mContent->GetParent();
       content; content = content->GetParent()) {
    nsCOMPtr<nsILink> link(do_QueryInterface(content));
    if (link) {
      link->GetHrefURI(aHref);
      status = (*aHref != nsnull);

      nsCOMPtr<nsIDOMHTMLAnchorElement> anchor(do_QueryInterface(content));
      if (anchor) {
        anchor->GetTarget(aTarget);
      }
      NS_ADDREF(*aNode = content);
      break;
    }
  }
  return status;
}

NS_IMETHODIMP  
nsImageFrame::GetContentForEvent(nsPresContext* aPresContext,
                                 nsEvent* aEvent,
                                 nsIContent** aContent)
{
  NS_ENSURE_ARG_POINTER(aContent);
  nsImageMap* map;
  map = GetImageMap(aPresContext);

  if (nsnull != map) {
    nsIntPoint p;
    TranslateEventCoords(
      nsLayoutUtils::GetEventCoordinatesRelativeTo(aEvent, this), p);
    PRBool inside = PR_FALSE;
    nsCOMPtr<nsIContent> area;
    inside = map->IsInside(p.x, p.y, getter_AddRefs(area));
    if (inside && area) {
      *aContent = area;
      NS_ADDREF(*aContent);
      return NS_OK;
    }
  }

  *aContent = GetContent();
  NS_IF_ADDREF(*aContent);
  return NS_OK;
}

// XXX what should clicks on transparent pixels do?
NS_IMETHODIMP
nsImageFrame::HandleEvent(nsPresContext* aPresContext,
                          nsGUIEvent* aEvent,
                          nsEventStatus* aEventStatus)
{
  NS_ENSURE_ARG_POINTER(aEventStatus);
  nsImageMap* map;

  if (aEvent->eventStructType == NS_MOUSE_EVENT &&
      (aEvent->message == NS_MOUSE_BUTTON_UP && 
      static_cast<nsMouseEvent*>(aEvent)->button == nsMouseEvent::eLeftButton) ||
      aEvent->message == NS_MOUSE_MOVE) {
    map = GetImageMap(aPresContext);
    PRBool isServerMap = IsServerImageMap();
    if ((nsnull != map) || isServerMap) {
      nsIntPoint p;
      TranslateEventCoords(
        nsLayoutUtils::GetEventCoordinatesRelativeTo(aEvent, this), p);
      PRBool inside = PR_FALSE;
      // Even though client-side image map triggering happens
      // through content, we need to make sure we're not inside
      // (in case we deal with a case of both client-side and
      // sever-side on the same image - it happens!)
      if (nsnull != map) {
        nsCOMPtr<nsIContent> area;
        inside = map->IsInside(p.x, p.y, getter_AddRefs(area));
      }

      if (!inside && isServerMap) {

        // Server side image maps use the href in a containing anchor
        // element to provide the basis for the destination url.
        nsCOMPtr<nsIURI> uri;
        nsAutoString target;
        nsCOMPtr<nsIContent> anchorNode;
        if (GetAnchorHREFTargetAndNode(getter_AddRefs(uri), target,
                                       getter_AddRefs(anchorNode))) {
          // XXX if the mouse is over/clicked in the border/padding area
          // we should probably just pretend nothing happened. Nav4
          // keeps the x,y coordinates positive as we do; IE doesn't
          // bother. Both of them send the click through even when the
          // mouse is over the border.
          if (p.x < 0) p.x = 0;
          if (p.y < 0) p.y = 0;
          nsCAutoString spec;
          uri->GetSpec(spec);
          spec += nsPrintfCString("?%d,%d", p.x, p.y);
          uri->SetSpec(spec);                
          
          PRBool clicked = PR_FALSE;
          if (aEvent->message == NS_MOUSE_BUTTON_UP) {
            *aEventStatus = nsEventStatus_eConsumeDoDefault; 
            clicked = PR_TRUE;
          }
          nsContentUtils::TriggerLink(anchorNode, aPresContext, uri, target,
                                      clicked, PR_TRUE);
        }
      }
    }
  }

  return nsSplittableFrame::HandleEvent(aPresContext, aEvent, aEventStatus);
}

NS_IMETHODIMP
nsImageFrame::GetCursor(const nsPoint& aPoint,
                        nsIFrame::Cursor& aCursor)
{
  nsPresContext* context = PresContext();
  nsImageMap* map = GetImageMap(context);
  if (nsnull != map) {
    nsIntPoint p;
    TranslateEventCoords(aPoint, p);
    nsCOMPtr<nsIContent> area;
    if (map->IsInside(p.x, p.y, getter_AddRefs(area))) {
      // Use the cursor from the style of the *area* element.
      // XXX Using the image as the parent style context isn't
      // technically correct, but it's probably the right thing to do
      // here, since it means that areas on which the cursor isn't
      // specified will inherit the style from the image.
      nsRefPtr<nsStyleContext> areaStyle = 
        PresContext()->PresShell()->StyleSet()->
          ResolveStyleFor(area, GetStyleContext());
      if (areaStyle) {
        FillCursorInformationFromStyle(areaStyle->GetStyleUserInterface(),
                                       aCursor);
        if (NS_STYLE_CURSOR_AUTO == aCursor.mCursor) {
          aCursor.mCursor = NS_STYLE_CURSOR_DEFAULT;
        }
        return NS_OK;
      }
    }
  }
  return nsFrame::GetCursor(aPoint, aCursor);
}

NS_IMETHODIMP
nsImageFrame::AttributeChanged(PRInt32 aNameSpaceID,
                               nsIAtom* aAttribute,
                               PRInt32 aModType)
{
  nsresult rv = nsSplittableFrame::AttributeChanged(aNameSpaceID,
                                                    aAttribute, aModType);
  if (NS_FAILED(rv)) {
    return rv;
  }
  if (nsGkAtoms::alt == aAttribute)
  {
    PresContext()->PresShell()->FrameNeedsReflow(this,
                                                 nsIPresShell::eStyleChange,
                                                 NS_FRAME_IS_DIRTY);
  }

  return NS_OK;
}

nsIAtom*
nsImageFrame::GetType() const
{
  return nsGkAtoms::imageFrame;
}

#ifdef DEBUG
NS_IMETHODIMP
nsImageFrame::GetFrameName(nsAString& aResult) const
{
  return MakeFrameName(NS_LITERAL_STRING("ImageFrame"), aResult);
}

NS_IMETHODIMP
nsImageFrame::List(FILE* out, PRInt32 aIndent) const
{
  IndentBy(out, aIndent);
  ListTag(out);
#ifdef DEBUG_waterson
  fprintf(out, " [parent=%p]", mParent);
#endif
  if (HasView()) {
    fprintf(out, " [view=%p]", (void*)GetView());
  }
  fprintf(out, " {%d,%d,%d,%d}", mRect.x, mRect.y, mRect.width, 
mRect.height);
  if (0 != mState) {
    fprintf(out, " [state=%08x]", mState);
  }
  fprintf(out, " [content=%p]", (void*)mContent);

  // output the img src url
  nsCOMPtr<nsIImageLoadingContent> imageLoader = do_QueryInterface(mContent);
  if (imageLoader) {
    nsCOMPtr<imgIRequest> currentRequest;
    imageLoader->GetRequest(nsIImageLoadingContent::CURRENT_REQUEST,
                            getter_AddRefs(currentRequest));
    if (currentRequest) {
      nsCOMPtr<nsIURI> uri;
      currentRequest->GetURI(getter_AddRefs(uri));
      nsCAutoString uristr;
      uri->GetAsciiSpec(uristr);
      fprintf(out, " [src=%s]", uristr.get());
    }
  }
  fputs("\n", out);
  return NS_OK;
}
#endif

PRIntn
nsImageFrame::GetSkipSides() const
{
  PRIntn skip = 0;
  if (nsnull != GetPrevInFlow()) {
    skip |= 1 << NS_SIDE_TOP;
  }
  if (nsnull != GetNextInFlow()) {
    skip |= 1 << NS_SIDE_BOTTOM;
  }
  return skip;
}

NS_IMETHODIMP 
nsImageFrame::GetIntrinsicImageSize(nsSize& aSize)
{
  aSize = mIntrinsicSize;
  return NS_OK;
}

nsresult
nsImageFrame::LoadIcon(const nsAString& aSpec,
                       nsPresContext *aPresContext,
                       imgIRequest** aRequest)
{
  nsresult rv = NS_OK;
  NS_PRECONDITION(!aSpec.IsEmpty(), "What happened??");

  if (!sIOService) {
    rv = CallGetService(NS_IOSERVICE_CONTRACTID, &sIOService);
    NS_ENSURE_SUCCESS(rv, rv);
  }

  nsCOMPtr<nsIURI> realURI;
  SpecToURI(aSpec, sIOService, getter_AddRefs(realURI));
 
  nsCOMPtr<imgILoader> il(do_GetService("@mozilla.org/image/loader;1", &rv));
  if (NS_FAILED(rv)) return rv;

  nsCOMPtr<nsILoadGroup> loadGroup;
  GetLoadGroup(aPresContext, getter_AddRefs(loadGroup));

  // For icon loads, we don't need to merge with the loadgroup flags
  nsLoadFlags loadFlags = nsIRequest::LOAD_NORMAL;

  return il->LoadImage(realURI,     /* icon URI */
                       nsnull,      /* initial document URI; this is only
                                       relevant for cookies, so does not
                                       apply to icons. */
                       nsnull,      /* referrer (not relevant for icons) */
                       loadGroup,
                       mListener,
                       nsnull,      /* Not associated with any particular document */
                       loadFlags,
                       nsnull,
                       nsnull,
                       aRequest);
}

void
nsImageFrame::GetDocumentCharacterSet(nsACString& aCharset) const
{
  if (mContent) {
    NS_ASSERTION(mContent->GetDocument(),
                 "Frame still alive after content removed from document!");
    aCharset = mContent->GetDocument()->GetDocumentCharacterSet();
  }
}

void
nsImageFrame::SpecToURI(const nsAString& aSpec, nsIIOService *aIOService,
                         nsIURI **aURI)
{
  nsCOMPtr<nsIURI> baseURI;
  if (mContent) {
    baseURI = mContent->GetBaseURI();
  }
  nsCAutoString charset;
  GetDocumentCharacterSet(charset);
  NS_NewURI(aURI, aSpec, 
            charset.IsEmpty() ? nsnull : charset.get(), 
            baseURI, aIOService);
}

void
nsImageFrame::GetLoadGroup(nsPresContext *aPresContext, nsILoadGroup **aLoadGroup)
{
  if (!aPresContext)
    return;

  NS_PRECONDITION(nsnull != aLoadGroup, "null OUT parameter pointer");

  nsIPresShell *shell = aPresContext->GetPresShell();

  if (!shell)
    return;

  nsIDocument *doc = shell->GetDocument();
  if (!doc)
    return;

  *aLoadGroup = doc->GetDocumentLoadGroup().get();  // already_AddRefed
}

nsresult nsImageFrame::LoadIcons(nsPresContext *aPresContext)
{
  NS_ASSERTION(!gIconLoad, "called LoadIcons twice");

  NS_NAMED_LITERAL_STRING(loadingSrc,"resource://gre/res/loading-image.gif"); 
  NS_NAMED_LITERAL_STRING(brokenSrc,"resource://gre/res/broken-image.gif");

  gIconLoad = new IconLoad(mListener);
  if (!gIconLoad) 
    return NS_ERROR_OUT_OF_MEMORY;
  NS_ADDREF(gIconLoad);

  nsresult rv;
  // create a loader and load the images
  rv = LoadIcon(loadingSrc,
                aPresContext,
                getter_AddRefs(gIconLoad->mLoadingImage));
#ifdef NOISY_ICON_LOADING
  printf("Loading request %p, rv=%u\n",
         gIconLoad->mLoadingImage.get(), rv);
#endif

  if (NS_FAILED(rv)) {
    return rv;
  }

  rv = LoadIcon(brokenSrc,
                aPresContext,
                getter_AddRefs(gIconLoad->mBrokenImage));
#ifdef NOISY_ICON_LOADING
  printf("Loading request %p, rv=%u\n",
         gIconLoad->mBrokenImage.get(), rv);
#endif

  // ImageLoader will callback into OnStartContainer, which will
  // handle the mIconsLoaded flag

  return rv;
}

PRBool nsImageFrame::HandleIconLoads(imgIRequest* aRequest, PRBool aLoaded)
{
  PRBool result = PR_FALSE;

  if (gIconLoad) {
    // check which image it is
    if (aRequest == gIconLoad->mLoadingImage ||
        aRequest == gIconLoad->mBrokenImage) {
      result = PR_TRUE;
      if (aLoaded && (++gIconLoad->mIconsLoaded == 2))
        gIconLoad->mLoadObserver = nsnull;
    }

#ifdef NOISY_ICON_LOADING
    if (gIconLoad->mIconsLoaded && result) {
      printf( "Icons Loaded: request for %s\n",
              aRequest == gIconLoad->mLoadingImage
                ? "mLoadingImage" : "mBrokenImage" );
    }
#endif
  }
  
#ifdef NOISY_ICON_LOADING
  printf( "HandleIconLoads returned %s (%p)\n", result ? "TRUE" : "FALSE", this);
#endif

  return result;
}

NS_IMPL_ISUPPORTS1(nsImageFrame::IconLoad, nsIObserver)

static const char kIconLoadPrefs[][40] = {
  "browser.display.force_inline_alttext",
  "browser.display.show_image_placeholders"
};

nsImageFrame::IconLoad::IconLoad(imgIDecoderObserver *aObserver)
  : mLoadObserver(aObserver),
    mIconsLoaded(0)
{
  nsCOMPtr<nsIPrefBranch2> prefBranch =
    do_QueryInterface(nsContentUtils::GetPrefBranch());

  // register observers
  for (PRUint32 i = 0; i < NS_ARRAY_LENGTH(kIconLoadPrefs); ++i)
    prefBranch->AddObserver(kIconLoadPrefs[i], this, PR_FALSE);

  GetPrefs();
}


NS_IMETHODIMP
nsImageFrame::IconLoad::Observe(nsISupports *aSubject, const char* aTopic,
                                const PRUnichar* aData)
{
  NS_ASSERTION(!nsCRT::strcmp(aTopic, NS_PREFBRANCH_PREFCHANGE_TOPIC_ID),
               "wrong topic");
#ifdef DEBUG
  // assert |aData| is one of our prefs.
  for (PRUint32 i = 0; i < NS_ARRAY_LENGTH(kIconLoadPrefs) ||
                       (NS_NOTREACHED("wrong pref"), PR_FALSE); ++i)
    if (NS_ConvertASCIItoUTF16(kIconLoadPrefs[i]) == nsDependentString(aData))
      break;
#endif

  GetPrefs();
  return NS_OK;
}

void nsImageFrame::IconLoad::GetPrefs()
{
  mPrefForceInlineAltText =
    nsContentUtils::GetBoolPref("browser.display.force_inline_alttext");

  mPrefShowPlaceholders =
    nsContentUtils::GetBoolPref("browser.display.show_image_placeholders",
                                PR_TRUE);
}

NS_IMPL_ISUPPORTS2(nsImageListener, imgIDecoderObserver, imgIContainerObserver)

nsImageListener::nsImageListener(nsImageFrame *aFrame) :
  mFrame(aFrame)
{
}

nsImageListener::~nsImageListener()
{
}

NS_IMETHODIMP nsImageListener::OnStartContainer(imgIRequest *aRequest,
                                                imgIContainer *aImage)
{
  if (!mFrame)
    return NS_ERROR_FAILURE;

  return mFrame->OnStartContainer(aRequest, aImage);
}

NS_IMETHODIMP nsImageListener::OnDataAvailable(imgIRequest *aRequest,
                                               gfxIImageFrame *aFrame,
                                               const nsRect *aRect)
{
  if (!mFrame)
    return NS_ERROR_FAILURE;

  return mFrame->OnDataAvailable(aRequest, aFrame, aRect);
}

NS_IMETHODIMP nsImageListener::OnStopDecode(imgIRequest *aRequest,
                                            nsresult status,
                                            const PRUnichar *statusArg)
{
  if (!mFrame)
    return NS_ERROR_FAILURE;

  return mFrame->OnStopDecode(aRequest, status, statusArg);
}

NS_IMETHODIMP nsImageListener::FrameChanged(imgIContainer *aContainer,
                                            gfxIImageFrame *newframe,
                                            nsRect * dirtyRect)
{
  if (!mFrame)
    return NS_ERROR_FAILURE;

  return mFrame->FrameChanged(aContainer, newframe, dirtyRect);
}

static PRBool
IsInAutoWidthTableCellForQuirk(nsIFrame *aFrame)
{
  if (eCompatibility_NavQuirks != aFrame->PresContext()->CompatibilityMode())
    return PR_FALSE;
  // Check if the parent of the closest nsBlockFrame has auto width.
  nsBlockFrame *ancestor = nsLayoutUtils::FindNearestBlockAncestor(aFrame);
  if (ancestor->GetStyleContext()->GetPseudoType() == nsCSSAnonBoxes::cellContent) {
    // Assume direct parent is a table cell frame.
    nsFrame *grandAncestor = static_cast<nsFrame*>(ancestor->GetParent());
    return grandAncestor &&
      grandAncestor->GetStylePosition()->mWidth.GetUnit() == eStyleUnit_Auto;
  }
  return PR_FALSE;
}

/* virtual */ void
nsImageFrame::AddInlineMinWidth(nsIRenderingContext *aRenderingContext,
                                nsIFrame::InlineMinWidthData *aData)
{

  NS_ASSERTION(GetParent(), "Must have a parent if we get here!");
  
  PRBool canBreak =
    !CanContinueTextRun() &&
    GetParent()->GetStyleText()->WhiteSpaceCanWrap() &&
    !IsInAutoWidthTableCellForQuirk(this);

  if (canBreak)
    aData->OptionallyBreak(aRenderingContext);
 
  aData->trailingWhitespace = 0;
  aData->skipWhitespace = PR_FALSE;
  aData->trailingTextFrame = nsnull;
  aData->currentLine += nsLayoutUtils::IntrinsicForContainer(aRenderingContext,
                            this, nsLayoutUtils::MIN_WIDTH);
  aData->atStartOfLine = PR_FALSE;

  if (canBreak)
    aData->OptionallyBreak(aRenderingContext);

}
