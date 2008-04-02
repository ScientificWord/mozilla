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
 * The Original Code is the Mozilla SVG project.
 *
 * The Initial Developer of the Original Code is
 * Crocodile Clips Ltd..
 * Portions created by the Initial Developer are Copyright (C) 2001
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Alex Fritze <alex.fritze@crocodile-clips.com> (original author)
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

#include "nsSVGForeignObjectFrame.h"

#include "nsIDOMSVGForeignObjectElem.h"
#include "nsIDOMSVGMatrix.h"
#include "nsIDOMSVGSVGElement.h"
#include "nsSpaceManager.h"
#include "nsSVGOuterSVGFrame.h"
#include "nsRegion.h"
#include "nsGkAtoms.h"
#include "nsLayoutUtils.h"
#include "nsSVGUtils.h"
#include "nsIURI.h"
#include "nsSVGRect.h"
#include "nsSVGMatrix.h"
#include "nsINameSpaceManager.h"
#include "nsSVGForeignObjectElement.h"
#include "nsSVGContainerFrame.h"
#include "gfxContext.h"
#include "gfxMatrix.h"

//----------------------------------------------------------------------
// Implementation

nsIFrame*
NS_NewSVGForeignObjectFrame(nsIPresShell   *aPresShell,
                            nsIContent     *aContent,
                            nsStyleContext *aContext)
{
  nsCOMPtr<nsIDOMSVGForeignObjectElement> foreignObject = do_QueryInterface(aContent);
  if (!foreignObject) {
    NS_ERROR("Can't create frame! Content is not an SVG foreignObject!");
    return nsnull;
  }

  return new (aPresShell) nsSVGForeignObjectFrame(aContext);
}

nsSVGForeignObjectFrame::nsSVGForeignObjectFrame(nsStyleContext* aContext)
  : nsSVGForeignObjectFrameBase(aContext),
    mPropagateTransform(PR_TRUE),
    mInReflow(PR_FALSE)
{
  AddStateBits(NS_FRAME_REFLOW_ROOT);
}

//----------------------------------------------------------------------
// nsISupports methods

NS_INTERFACE_MAP_BEGIN(nsSVGForeignObjectFrame)
  NS_INTERFACE_MAP_ENTRY(nsISVGChildFrame)
NS_INTERFACE_MAP_END_INHERITING(nsSVGForeignObjectFrameBase)

//----------------------------------------------------------------------
// nsIFrame methods

NS_IMETHODIMP
nsSVGForeignObjectFrame::Init(nsIContent* aContent,
                              nsIFrame*   aParent,
                              nsIFrame*   aPrevInFlow)
{
  nsresult rv = nsSVGForeignObjectFrameBase::Init(aContent, aParent, aPrevInFlow);
  if (NS_SUCCEEDED(rv)) {
    nsSVGUtils::GetOuterSVGFrame(this)->RegisterForeignObject(this);
  }
  return rv;
}

void nsSVGForeignObjectFrame::Destroy()
{
  nsSVGUtils::GetOuterSVGFrame(this)->UnregisterForeignObject(this);
  // Delete any clipPath/filter/mask properties _before_ we die. The properties
  // and property hash table have weak pointers to us that are dereferenced
  // when the properties are destroyed.
  nsSVGUtils::StyleEffects(this);
  nsSVGForeignObjectFrameBase::Destroy();
}

nsIAtom *
nsSVGForeignObjectFrame::GetType() const
{
  return nsGkAtoms::svgForeignObjectFrame;
}

NS_IMETHODIMP
nsSVGForeignObjectFrame::AttributeChanged(PRInt32  aNameSpaceID,
                                          nsIAtom *aAttribute,
                                          PRInt32  aModType)
{
  if (aNameSpaceID == kNameSpaceID_None) {
    if (aAttribute == nsGkAtoms::width ||
        aAttribute == nsGkAtoms::height) {
      UpdateGraphic(); // update mRect before requesting reflow
      // XXXjwatt: why mark intrinsic widths dirty? can't we just use eResize?
      RequestReflow(nsIPresShell::eStyleChange);
    } else if (aAttribute == nsGkAtoms::x ||
               aAttribute == nsGkAtoms::y) {
      UpdateGraphic();
    } else if (aAttribute == nsGkAtoms::transform) {
      // make sure our cached transform matrix gets (lazily) updated
      mCanvasTM = nsnull;
      UpdateGraphic();
    }
  }

  return NS_OK;
}

NS_IMETHODIMP
nsSVGForeignObjectFrame::DidSetStyleContext()
{
  nsSVGUtils::StyleEffects(this);
  return NS_OK;
}

NS_IMETHODIMP
nsSVGForeignObjectFrame::Reflow(nsPresContext*           aPresContext,
                                nsHTMLReflowMetrics&     aDesiredSize,
                                const nsHTMLReflowState& aReflowState,
                                nsReflowStatus&          aStatus)
{
  // InitialUpdate and AttributeChanged make sure mRect is up to date before
  // we're called (UpdateCoveredRegion sets mRect).

  NS_ASSERTION(!aReflowState.parentReflowState,
               "should only get reflow from being reflow root");
  NS_ASSERTION(aReflowState.ComputedWidth() == GetSize().width &&
               aReflowState.ComputedHeight() == GetSize().height,
               "reflow roots should be reflown at existing size and "
               "svg.css should ensure we have no padding/border/margin");

  DoReflow();

  // XXX why don't we convert from CSS pixels to app units? How does this work?
  aDesiredSize.width = aReflowState.ComputedWidth();
  aDesiredSize.height = aReflowState.ComputedHeight();
  aDesiredSize.mOverflowArea =
    nsRect(nsPoint(0, 0), nsSize(aDesiredSize.width, aDesiredSize.height));
  aStatus = NS_FRAME_COMPLETE;

  return NS_OK;
}

void
nsSVGForeignObjectFrame::InvalidateInternal(const nsRect& aDamageRect,
                                            nscoord aX, nscoord aY,
                                            nsIFrame* aForChild,
                                            PRBool aImmediate)
{
  if (mParent->GetStateBits() & NS_STATE_SVG_NONDISPLAY_CHILD)
    return;

  mDirtyRegion.Or(mDirtyRegion, aDamageRect + nsPoint(aX, aY));
  FlushDirtyRegion();
}


//----------------------------------------------------------------------
// nsISVGChildFrame methods

/**
 * Gets the rectangular region in rounded out CSS pixels that encloses the
 * rectangle after it has been transformed by aMatrix. Useful in
 * UpdateCoveredRegion/FlushDirtyRegion.
 */
static nsRect
GetTransformedRegion(float aX, float aY, float aWidth, float aHeight,
                     nsIDOMSVGMatrix* aMatrix)
{
  float x[4], y[4];
  x[0] = aX;
  y[0] = aY;
  x[1] = aX + aWidth;
  y[1] = aY;
  x[2] = aX + aWidth;
  y[2] = aY + aHeight;
  x[3] = aX;
  y[3] = aY + aHeight;
 
  for (int i = 0; i < 4; i++) {
    nsSVGUtils::TransformPoint(aMatrix, &x[i], &y[i]);
  }

  float xmin, xmax, ymin, ymax;
  xmin = xmax = x[0];
  ymin = ymax = y[0];
  for (int i = 1; i < 4; i++) {
    if (x[i] < xmin)
      xmin = x[i];
    else if (x[i] > xmax)
      xmax = x[i];
    if (y[i] < ymin)
      ymin = y[i];
    else if (y[i] > ymax)
      ymax = y[i];
  }
 
  return nsSVGUtils::ToBoundingPixelRect(xmin, ymin, xmax, ymax);
}

NS_IMETHODIMP
nsSVGForeignObjectFrame::PaintSVG(nsSVGRenderState *aContext,
                                  nsRect *aDirtyRect)
{
  if (IsDisabled())
    return NS_OK;

  nsIFrame* kid = GetFirstChild(nsnull);
  if (!kid)
    return NS_OK;

  nsCOMPtr<nsIDOMSVGMatrix> tm = GetTMIncludingOffset();

  gfxMatrix matrix = nsSVGUtils::ConvertSVGMatrixToThebes(tm);

  nsIRenderingContext *ctx = aContext->GetRenderingContext();

  if (!ctx || matrix.IsSingular()) {
    NS_WARNING("Can't render foreignObject element!");
    return NS_ERROR_FAILURE;
  }

  gfxContext *gfx = aContext->GetGfxContext();

  gfx->Save();

  if (GetStyleDisplay()->IsScrollableOverflow()) {
    float x, y, width, height;
    static_cast<nsSVGElement*>(mContent)->
      GetAnimatedLengthValues(&x, &y, &width, &height, nsnull);

    // tm already includes the x,y offset
    nsSVGUtils::SetClipRect(gfx, tm, 0.0f, 0.0f, width, height);
  }

  gfx->Multiply(matrix);

  nsresult rv = nsLayoutUtils::PaintFrame(ctx, kid, nsRegion(kid->GetRect()),
                                          NS_RGBA(0,0,0,0));

  gfx->Restore();

  return rv;
}

nsresult
nsSVGForeignObjectFrame::TransformPointFromOuterPx(float aX, float aY, nsPoint* aOut)
{
  if (mParent->GetStateBits() & NS_STATE_SVG_NONDISPLAY_CHILD)
    return NS_ERROR_FAILURE;

  nsCOMPtr<nsIDOMSVGMatrix> tm = GetTMIncludingOffset();
  nsCOMPtr<nsIDOMSVGMatrix> inverse;
  nsresult rv = tm->Inverse(getter_AddRefs(inverse));
  if (NS_FAILED(rv))
    return rv;

  nsSVGUtils::TransformPoint(inverse, &aX, &aY);
  PRInt32 appUnitsPerDevPixel = PresContext()->AppUnitsPerDevPixel();
  *aOut = nsPoint(NSToCoordRound(aX * appUnitsPerDevPixel),
                  NSToCoordRound(aY * appUnitsPerDevPixel));
  return NS_OK;
}
 
NS_IMETHODIMP
nsSVGForeignObjectFrame::GetFrameForPointSVG(float x, float y, nsIFrame** hit)
{
  *hit = nsnull;

  if (IsDisabled())
    return NS_OK;

  nsIFrame* kid = GetFirstChild(nsnull);
  if (!kid) {
    return NS_OK;
  }
  nsPoint pt;
  nsresult rv = TransformPointFromOuterPx(x, y, &pt);
  if (NS_FAILED(rv))
    return rv;
  *hit = nsLayoutUtils::GetFrameForPoint(kid, pt);
  return NS_OK;
}

nsPoint
nsSVGForeignObjectFrame::TransformPointFromOuter(nsPoint aPt)
{
  nsPoint pt(NS_UNCONSTRAINEDSIZE, NS_UNCONSTRAINEDSIZE);
  float appUnitsPerDevPixel = PresContext()->AppUnitsPerDevPixel();
  TransformPointFromOuterPx(aPt.x / appUnitsPerDevPixel,
                            aPt.y / appUnitsPerDevPixel,
                            &pt);
  return pt;
}

NS_IMETHODIMP_(nsRect)
nsSVGForeignObjectFrame::GetCoveredRegion()
{
  return mRect;
}

NS_IMETHODIMP
nsSVGForeignObjectFrame::UpdateCoveredRegion()
{
  if (mParent->GetStateBits() & NS_STATE_SVG_NONDISPLAY_CHILD)
    return NS_ERROR_FAILURE;

  nsCOMPtr<nsIDOMSVGMatrix> ctm = GetCanvasTM();
  if (!ctm)
    return NS_ERROR_FAILURE;

  float x, y, w, h;
  static_cast<nsSVGForeignObjectElement*>(mContent)->
    GetAnimatedLengthValues(&x, &y, &w, &h, nsnull);

  // If mRect's width or height are negative, reflow blows up! We must clamp!
  if (w < 0.0f) w = 0.0f;
  if (h < 0.0f) h = 0.0f;

  // XXXjwatt: _this_ is where we should reflow _if_ mRect.width has changed!
  // we should not unconditionally reflow in AttributeChanged
  mRect = GetTransformedRegion(x, y, w, h, ctm);
  return NS_OK;
}

NS_IMETHODIMP
nsSVGForeignObjectFrame::InitialUpdate()
{
  NS_ASSERTION(GetStateBits() & NS_FRAME_FIRST_REFLOW,
               "Yikes! We've been called already! Hopefully we weren't called "
               "before our nsSVGOuterSVGFrame's initial Reflow()!!!");

  UpdateCoveredRegion();
  DoReflow();

  NS_ASSERTION(!(mState & NS_FRAME_IN_REFLOW),
               "We don't actually participate in reflow");
  
  // Do unset the various reflow bits, though.
  mState &= ~(NS_FRAME_FIRST_REFLOW | NS_FRAME_IS_DIRTY |
              NS_FRAME_HAS_DIRTY_CHILDREN);

  return NS_OK;
}

void
nsSVGForeignObjectFrame::NotifySVGChanged(PRUint32 aFlags)
{
  PRBool reflow = PR_FALSE;

  if (aFlags & TRANSFORM_CHANGED) {
    // Perhaps unexpectedly, we reflow if our CTM changes. This is because
    // glyph metrics do not necessarily scale uniformly with change in scale
    // and, as a result, CTM changes may require text to break at different
    // points.
    // XXX roc says we shouldn't do this. See bug 381285 comment 20.
    reflow = PR_TRUE;
    mCanvasTM = nsnull;

  } else if (aFlags & COORD_CONTEXT_CHANGED) {
    // Our coordinate context's width/height has changed. If we have a
    // percentage width/height our dimensions will change so we must reflow.
    nsSVGForeignObjectElement *fO =
      static_cast<nsSVGForeignObjectElement*>(mContent);
    if (fO->mLengthAttributes[nsSVGForeignObjectElement::WIDTH].IsPercentage() ||
        fO->mLengthAttributes[nsSVGForeignObjectElement::HEIGHT].IsPercentage()) {
      reflow = PR_TRUE;
    }
  }

  if (reflow) {
    // If we're called while the PresShell is handling reflow events then we
    // must have been called as a result of the NotifyViewportChange() call in
    // our nsSVGOuterSVGFrame's Reflow() method. We must not call RequestReflow
    // at this point (i.e. during reflow) because it could confuse the
    // PresShell and prevent it from reflowing us properly in future. Besides
    // that, nsSVGOuterSVGFrame::DidReflow will take care of reflowing us
    // synchronously, so there's no need.
    PRBool reflowing;
    PresContext()->PresShell()->IsReflowLocked(&reflowing);
    if (!reflowing) {
      UpdateGraphic(); // update mRect before requesting reflow
      RequestReflow(nsIPresShell::eResize);
    }
  }
}

NS_IMETHODIMP
nsSVGForeignObjectFrame::NotifyRedrawSuspended()
{
  return NS_OK;
}

NS_IMETHODIMP
nsSVGForeignObjectFrame::NotifyRedrawUnsuspended()
{
  if (!(mParent->GetStateBits() & NS_STATE_SVG_NONDISPLAY_CHILD)) {
    if (GetStateBits() & NS_STATE_SVG_DIRTY) {
      UpdateGraphic();
    } else {
      FlushDirtyRegion();
    }
  }
  return NS_OK;
}

NS_IMETHODIMP
nsSVGForeignObjectFrame::SetMatrixPropagation(PRBool aPropagate)
{
  mPropagateTransform = aPropagate;
  return NS_OK;
}

NS_IMETHODIMP
nsSVGForeignObjectFrame::SetOverrideCTM(nsIDOMSVGMatrix *aCTM)
{
  mOverrideCTM = aCTM;
  return NS_OK;
}

already_AddRefed<nsIDOMSVGMatrix>
nsSVGForeignObjectFrame::GetOverrideCTM()
{
  nsIDOMSVGMatrix *matrix = mOverrideCTM.get();
  NS_IF_ADDREF(matrix);
  return matrix;
}

NS_IMETHODIMP
nsSVGForeignObjectFrame::GetBBox(nsIDOMSVGRect **_retval)
{
  if (mParent->GetStateBits() & NS_STATE_SVG_NONDISPLAY_CHILD) {
    *_retval = nsnull;
    return NS_ERROR_FAILURE;
  }

  float x, y, w, h;
  static_cast<nsSVGForeignObjectElement*>(mContent)->
    GetAnimatedLengthValues(&x, &y, &w, &h, nsnull);

  if (w < 0.0f) w = 0.0f;
  if (h < 0.0f) h = 0.0f;

  return NS_NewSVGRect(_retval, x, y, w, h);
}

//----------------------------------------------------------------------

already_AddRefed<nsIDOMSVGMatrix>
nsSVGForeignObjectFrame::GetTMIncludingOffset()
{
  nsCOMPtr<nsIDOMSVGMatrix> ctm = GetCanvasTM();
  if (!ctm)
    return nsnull;

  nsSVGForeignObjectElement *fO =
    static_cast<nsSVGForeignObjectElement*>(mContent);
  float x, y;
  fO->GetAnimatedLengthValues(&x, &y, nsnull);
  nsIDOMSVGMatrix* matrix;
  ctm->Translate(x, y, &matrix);
  return matrix;
}

already_AddRefed<nsIDOMSVGMatrix>
nsSVGForeignObjectFrame::GetCanvasTM()
{
  if (!mPropagateTransform) {
    nsIDOMSVGMatrix *retval;
    if (mOverrideCTM) {
      retval = mOverrideCTM;
      NS_ADDREF(retval);
    } else {
      NS_NewSVGMatrix(&retval);
    }
    return retval;
  }

  if (!mCanvasTM) {
    // get our parent's tm and append local transforms (if any):
    NS_ASSERTION(mParent, "null parent");
    nsSVGContainerFrame *containerFrame = static_cast<nsSVGContainerFrame*>
                                                     (mParent);
    nsCOMPtr<nsIDOMSVGMatrix> parentTM = containerFrame->GetCanvasTM();
    NS_ASSERTION(parentTM, "null TM");

    // got the parent tm, now check for local tm:
    nsSVGGraphicElement *element =
      static_cast<nsSVGGraphicElement*>(mContent);
    nsCOMPtr<nsIDOMSVGMatrix> localTM = element->GetLocalTransformMatrix();
    
    if (localTM)
      parentTM->Multiply(localTM, getter_AddRefs(mCanvasTM));
    else
      mCanvasTM = parentTM;
  }

  nsIDOMSVGMatrix* retval = mCanvasTM.get();
  NS_IF_ADDREF(retval);
  return retval;
}

//----------------------------------------------------------------------
// Implementation helpers

void nsSVGForeignObjectFrame::RequestReflow(nsIPresShell::IntrinsicDirty aType)
{
  if (GetStateBits() & NS_FRAME_FIRST_REFLOW)
    // If we haven't had an InitialUpdate yet, nothing to do.
    return;

  nsIFrame* kid = GetFirstChild(nsnull);
  if (!kid)
    return;

  PresContext()->PresShell()->FrameNeedsReflow(kid, aType, NS_FRAME_IS_DIRTY);
}

void nsSVGForeignObjectFrame::UpdateGraphic()
{
  if (GetStateBits() & NS_STATE_SVG_NONDISPLAY_CHILD)
    return;

  nsSVGOuterSVGFrame *outerSVGFrame = nsSVGUtils::GetOuterSVGFrame(this);
  if (!outerSVGFrame) {
    NS_ERROR("null outerSVGFrame");
    return;
  }
  
  if (outerSVGFrame->IsRedrawSuspended()) {
    AddStateBits(NS_STATE_SVG_DIRTY);
  } else {
    RemoveStateBits(NS_STATE_SVG_DIRTY);

    // Invalidate the area we used to cover
    // XXXjwatt: if we fix the following XXX, try to subtract the new region from the old here.
    // hmm, if x then y is changed, our second call could invalidate an "old" area we never actually painted to.
    outerSVGFrame->InvalidateRect(mRect);

    UpdateCoveredRegion();

    // Invalidate the area we now cover
    // XXXjwatt: when we're called due to an event that also requires reflow,
    // we want to let reflow trigger this rasterization so it doesn't happen twice.
    nsRect filterRect = nsSVGUtils::FindFilterInvalidation(this);
    if (!filterRect.IsEmpty()) {
      outerSVGFrame->InvalidateRect(filterRect);
    } else {
      outerSVGFrame->InvalidateRect(mRect);
    }
  }

  // Clear any layout dirty region since we invalidated our whole area.
  mDirtyRegion.SetEmpty();
}

void
nsSVGForeignObjectFrame::MaybeReflowFromOuterSVGFrame()
{
  // If we're already scheduled to reflow (i.e. our kid is dirty) we don't
  // want to reflow now or else our presShell will do extra work trying to
  // reflow us a second time. (It will also complain if it finds that a reflow
  // root scheduled for reflow isn't dirty).

  nsIFrame* kid = GetFirstChild(nsnull);
  if (kid->GetStateBits() & NS_FRAME_IS_DIRTY) {
    return;
  }
  kid->AddStateBits(NS_FRAME_IS_DIRTY); // we must be fully marked dirty
  if (kid->GetStateBits() & NS_FRAME_HAS_DIRTY_CHILDREN) {
    return;
  }
  DoReflow();
}

void
nsSVGForeignObjectFrame::DoReflow()
{
#ifdef DEBUG
  printf("**nsSVGForeignObjectFrame::DoReflow()\n");
#endif

  NS_ASSERTION(!(nsSVGUtils::GetOuterSVGFrame(this)->
                             GetStateBits() & NS_FRAME_FIRST_REFLOW),
               "Calling InitialUpdate too early - must not call DoReflow!!!");

  if (IsDisabled())
    return;

  if (mParent->GetStateBits() & NS_STATE_SVG_NONDISPLAY_CHILD)
    return;

  nsPresContext *presContext = PresContext();
  nsIFrame* kid = GetFirstChild(nsnull);
  if (!kid)
    return;

  // initiate a synchronous reflow here and now:  
  nsSize availableSpace(NS_UNCONSTRAINEDSIZE, NS_UNCONSTRAINEDSIZE);
  nsCOMPtr<nsIRenderingContext> renderingContext;
  nsIPresShell* presShell = presContext->PresShell();
  NS_ASSERTION(presShell, "null presShell");
  presShell->CreateRenderingContext(this,getter_AddRefs(renderingContext));
  if (!renderingContext)
    return;

  nsSVGForeignObjectElement *fO = static_cast<nsSVGForeignObjectElement*>
                                             (mContent);

  float width =
    fO->mLengthAttributes[nsSVGForeignObjectElement::WIDTH].GetAnimValue(fO);
  float height =
    fO->mLengthAttributes[nsSVGForeignObjectElement::HEIGHT].GetAnimValue(fO);

  nsSize size(nsPresContext::CSSPixelsToAppUnits(width),
              nsPresContext::CSSPixelsToAppUnits(height));

  mInReflow = PR_TRUE;

  nsHTMLReflowState reflowState(presContext, kid,
                                renderingContext,
                                nsSize(size.width, NS_UNCONSTRAINEDSIZE));
  nsHTMLReflowMetrics desiredSize;
  nsReflowStatus status;

  // We don't use size.height above because that tells the child to do
  // page/column breaking at that height.
  NS_ASSERTION(reflowState.mComputedBorderPadding == nsMargin(0, 0, 0, 0) &&
               reflowState.mComputedMargin == nsMargin(0, 0, 0, 0),
               "style system should ensure that :-moz-svg-foreign content "
               "does not get styled");
  NS_ASSERTION(reflowState.ComputedWidth() == size.width,
               "reflow state made child wrong size");
  reflowState.SetComputedHeight(size.height);
  
  ReflowChild(kid, presContext, desiredSize, reflowState, 0, 0,
              NS_FRAME_NO_MOVE_FRAME, status);
  NS_ASSERTION(size.width == desiredSize.width &&
               size.height == desiredSize.height, "unexpected size");
  FinishReflowChild(kid, presContext, &reflowState, desiredSize, 0, 0,
                    NS_FRAME_NO_MOVE_FRAME);
  
  mInReflow = PR_FALSE;
  FlushDirtyRegion();
}

void
nsSVGForeignObjectFrame::FlushDirtyRegion()
{
  if (mDirtyRegion.IsEmpty() || mInReflow)
    return;

  nsSVGOuterSVGFrame *outerSVGFrame = nsSVGUtils::GetOuterSVGFrame(this);
  if (!outerSVGFrame) {
    NS_ERROR("null outerSVGFrame");
    return;
  }

  if (outerSVGFrame->IsRedrawSuspended())
    return;

  nsRect rect = nsSVGUtils::FindFilterInvalidation(this);
  if (!rect.IsEmpty()) {
    outerSVGFrame->InvalidateRect(rect);
    mDirtyRegion.SetEmpty();
    return;
  }
  
  nsCOMPtr<nsIDOMSVGMatrix> tm = GetTMIncludingOffset();
  nsRect r = mDirtyRegion.GetBounds();
  r.ScaleRoundOut(1.0f / PresContext()->AppUnitsPerDevPixel());
  float x = r.x, y = r.y, w = r.width, h = r.height;
  r = GetTransformedRegion(x, y, w, h, tm);
  outerSVGFrame->InvalidateRect(r);

  mDirtyRegion.SetEmpty();
}

