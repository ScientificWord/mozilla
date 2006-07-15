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

#include "nsISVGRendererCanvas.h"
#include "nsISVGValue.h"
#include "nsIDOMSVGGElement.h"
#include "nsIDOMSVGForeignObjectElem.h"
#include "nsIDOMSVGMatrix.h"
#include "nsIDOMSVGSVGElement.h"
#include "nsIDOMSVGPoint.h"
#include "nsSpaceManager.h"
#include "nsISVGRenderer.h"
#include "nsSVGOuterSVGFrame.h"
#include "nsISVGValueUtils.h"
#include "nsRegion.h"
#include "nsLayoutAtoms.h"
#include "nsLayoutUtils.h"
#include "nsSVGUtils.h"
#include "nsIURI.h"
#include "nsSVGPoint.h"
#include "nsSVGRect.h"
#include "nsSVGMatrix.h"
#include "nsINameSpaceManager.h"
#include "nsGkAtoms.h"
#include "nsISVGRendererSurface.h"
#include "nsSVGForeignObjectElement.h"
#include "nsSVGContainerFrame.h"

//----------------------------------------------------------------------
// Implementation

nsIFrame*
NS_NewSVGForeignObjectFrame(nsIPresShell* aPresShell, nsIContent* aContent, nsStyleContext* aContext)
{
  nsCOMPtr<nsIDOMSVGForeignObjectElement> foreignObject = do_QueryInterface(aContent);
  if (!foreignObject) {
#ifdef DEBUG
    printf("warning: trying to construct an SVGForeignObjectFrame for a content element that doesn't support the right interfaces\n");
#endif
    return nsnull;
  }

  return new (aPresShell) nsSVGForeignObjectFrame(aContext);
}

nsSVGForeignObjectFrame::nsSVGForeignObjectFrame(nsStyleContext* aContext)
  : nsSVGForeignObjectFrameBase(aContext),
    mPropagateTransform(PR_TRUE), mInReflow(PR_FALSE)
{
}

//----------------------------------------------------------------------
// nsISupports methods

NS_INTERFACE_MAP_BEGIN(nsSVGForeignObjectFrame)
  NS_INTERFACE_MAP_ENTRY(nsISVGChildFrame)
  NS_INTERFACE_MAP_ENTRY(nsISupportsWeakReference)
  NS_INTERFACE_MAP_ENTRY(nsISVGValueObserver)
NS_INTERFACE_MAP_END_INHERITING(nsSVGForeignObjectFrameBase)

//----------------------------------------------------------------------
// nsIFrame methods

nsIAtom *
nsSVGForeignObjectFrame::GetType() const
{
  return nsLayoutAtoms::svgForeignObjectFrame;
}

PRBool
nsSVGForeignObjectFrame::IsFrameOfType(PRUint32 aFlags) const
{
  return !(aFlags & ~(nsIFrame::eSVG | nsIFrame::eSVGForeignObject));
}

NS_IMETHODIMP
nsSVGForeignObjectFrame::AttributeChanged(PRInt32         aNameSpaceID,
                                          nsIAtom*        aAttribute,
                                          PRInt32         aModType)
{
  if (aNameSpaceID == kNameSpaceID_None) {
    if (aAttribute == nsGkAtoms::width ||
        aAttribute == nsGkAtoms::height) {
      PostReflowCommand();
      UpdateCoveredRegion();
      UpdateGraphic();
    } else if (aAttribute == nsGkAtoms::x ||
               aAttribute == nsGkAtoms::y ||
               aAttribute == nsGkAtoms::transform) {
      UpdateCoveredRegion();
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

//----------------------------------------------------------------------
// nsISVGValueObserver methods:

NS_IMETHODIMP
nsSVGForeignObjectFrame::WillModifySVGObservable(nsISVGValue* observable,
                                                 nsISVGValue::modificationType aModType)
{
  nsSVGUtils::WillModifyEffects(this, observable, aModType);

  return NS_OK;
}


NS_IMETHODIMP
nsSVGForeignObjectFrame::DidModifySVGObservable (nsISVGValue* observable,
                                                 nsISVGValue::modificationType aModType)
{
  nsSVGUtils::DidModifyEffects(this, observable, aModType);
  UpdateGraphic();
   
  return NS_OK;
}


//----------------------------------------------------------------------
// nsISVGChildFrame methods

/**
 * Transform a rectangle with a given matrix. Since the image of the
 * rectangle may not be a rectangle, the output rectangle is the
 * bounding box of the true image.
 */
static void
TransformRect(float* aX, float *aY, float* aWidth, float *aHeight,
              nsIDOMSVGMatrix* aMatrix)
{
  float x[4], y[4];
  x[0] = *aX;
  y[0] = *aY;
  x[1] = x[0] + *aWidth;
  y[1] = y[0];
  x[2] = x[0] + *aWidth;
  y[2] = y[0] + *aHeight;
  x[3] = x[0];
  y[3] = y[0] + *aHeight;
 
  int i;
  for (i = 0; i < 4; i++) {
    nsSVGUtils::TransformPoint(aMatrix, &x[i], &y[i]);
  }

  float xmin, xmax, ymin, ymax;
  xmin = xmax = x[0];
  ymin = ymax = y[0];
  for (i=1; i<4; i++) {
    if (x[i] < xmin)
      xmin = x[i];
    if (y[i] < ymin)
      ymin = y[i];
    if (x[i] > xmax)
      xmax = x[i];
    if (y[i] > ymax)
      ymax = y[i];
  }
 
  *aX = xmin;
  *aY = ymin;
  *aWidth = xmax - xmin;
  *aHeight = ymax - ymin;
}

NS_IMETHODIMP
nsSVGForeignObjectFrame::PaintSVG(nsISVGRendererCanvas* canvas,
                                  nsRect *aDirtyRect)
{
  nsIFrame* kid = GetFirstChild(nsnull);
  if (!kid)
    return NS_OK;

  nsCOMPtr<nsIDOMSVGMatrix> tm = GetTMIncludingOffset();

  nsCOMPtr<nsIRenderingContext> ctx;
  canvas->LockRenderingContext(tm, getter_AddRefs(ctx));
  
  if (!ctx) {
    NS_WARNING("Can't render foreignObject element!");
    return NS_ERROR_FAILURE;
  }
    
  nsresult rv = nsLayoutUtils::PaintFrame(ctx, kid, nsRegion(kid->GetRect()),
                                          NS_RGBA(0,0,0,0));
  
  ctx = nsnull;
  canvas->UnlockRenderingContext();
  
  return rv;
}

nsresult
nsSVGForeignObjectFrame::TransformPointFromOuterPx(float aX, float aY, nsPoint* aOut)
{
  nsCOMPtr<nsIDOMSVGMatrix> tm = GetTMIncludingOffset();
  nsCOMPtr<nsIDOMSVGMatrix> inverse;
  nsresult rv = tm->Inverse(getter_AddRefs(inverse));
  if (NS_FAILED(rv))
    return rv;
   
  nsSVGUtils::TransformPoint(inverse, &aX, &aY);
  float twipsPerPx = GetTwipsPerPx();
  *aOut = nsPoint(NSToCoordRound(aX*twipsPerPx),
                  NSToCoordRound(aY*twipsPerPx));
  return NS_OK;
}
 
NS_IMETHODIMP
nsSVGForeignObjectFrame::GetFrameForPointSVG(float x, float y, nsIFrame** hit)
{
  nsIFrame* kid = GetFirstChild(nsnull);
  if (!kid) {
    *hit = nsnull;
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
  float pxPerTwips = GetPxPerTwips();
  nsPoint pt(NS_UNCONSTRAINEDSIZE, NS_UNCONSTRAINEDSIZE);
  TransformPointFromOuterPx(aPt.x*pxPerTwips, aPt.y*pxPerTwips, &pt);
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
  float x, y, w, h;
  GetBBoxInternal(&x, &y, &w, &h);

  mRect = nsSVGUtils::ToBoundingPixelRect(x, y, x + w, y + h);

  return NS_OK;
}

NS_IMETHODIMP
nsSVGForeignObjectFrame::InitialUpdate()
{
  UpdateCoveredRegion();
  DoReflow();
  return NS_OK;
}

NS_IMETHODIMP
nsSVGForeignObjectFrame::NotifyCanvasTMChanged(PRBool suppressInvalidation)
{
  mCanvasTM = nsnull;
  UpdateCoveredRegion();
  UpdateGraphic();
  return NS_OK;
}

NS_IMETHODIMP
nsSVGForeignObjectFrame::NotifyRedrawSuspended()
{
  return NS_OK;
}

NS_IMETHODIMP
nsSVGForeignObjectFrame::NotifyRedrawUnsuspended()
{
  FlushDirtyRegion();
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

void
nsSVGForeignObjectFrame::GetBBoxInternal(float* aX, float *aY, float* aWidth,
                                         float *aHeight)
{
  nsCOMPtr<nsIDOMSVGMatrix> ctm = GetCanvasTM();
  if (!ctm)
    return;
  
  nsSVGForeignObjectElement *fO = NS_STATIC_CAST(nsSVGForeignObjectElement*,
                                                 mContent);
  fO->GetAnimatedLengthValues(aX, aY, aWidth, aHeight, nsnull);
  
  TransformRect(aX, aY, aWidth, aHeight, ctm);
}
  
NS_IMETHODIMP
nsSVGForeignObjectFrame::GetBBox(nsIDOMSVGRect **_retval)
{
  float x, y, w, h;
  GetBBoxInternal(&x, &y, &w, &h);
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
    NS_STATIC_CAST(nsSVGForeignObjectElement*, mContent);
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
    nsSVGContainerFrame *containerFrame = NS_STATIC_CAST(nsSVGContainerFrame*,
                                                         mParent);
    nsCOMPtr<nsIDOMSVGMatrix> parentTM = containerFrame->GetCanvasTM();
    NS_ASSERTION(parentTM, "null TM");

    // got the parent tm, now check for local tm:
    nsSVGGraphicElement *element =
      NS_STATIC_CAST(nsSVGGraphicElement*, mContent);
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

void nsSVGForeignObjectFrame::PostReflowCommand()
{
  nsIFrame* kid = GetFirstChild(nsnull);
  if (!kid)
    return;
  GetPresContext()->PresShell()->
    AppendReflowCommand(kid, eReflowType_StyleChanged, nsnull);
}

void nsSVGForeignObjectFrame::UpdateGraphic()
{
  nsSVGOuterSVGFrame *outerSVGFrame = nsSVGUtils::GetOuterSVGFrame(this);
  if (!outerSVGFrame) {
    NS_ERROR("null outerSVGFrame");
    return;
  }
  
  PRBool suspended;
  outerSVGFrame->IsRedrawSuspended(&suspended);
  if (!suspended) {
    nsRect rect = nsSVGUtils::FindFilterInvalidation(this);
    if (rect.IsEmpty()) {
      rect = mRect;
    }
    outerSVGFrame->InvalidateRect(rect);
  }
}

void
nsSVGForeignObjectFrame::DoReflow()
{
#ifdef DEBUG
  printf("**nsSVGForeignObjectFrame::DoReflow()\n");
#endif

  nsPresContext *presContext = GetPresContext();
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
  
  float twipsPerPx = GetTwipsPerPx();
  
  nsSVGForeignObjectElement *fO = NS_STATIC_CAST(nsSVGForeignObjectElement*,
                                                 mContent);

  float width =
    fO->mLengthAttributes[nsSVGForeignObjectElement::WIDTH].GetAnimValue(fO);
  float height =
    fO->mLengthAttributes[nsSVGForeignObjectElement::HEIGHT].GetAnimValue(fO);

  nsSize size(NSFloatPixelsToTwips(width, twipsPerPx),
              NSFloatPixelsToTwips(height, twipsPerPx));

  mInReflow = PR_TRUE;

  // create a new reflow state, setting our max size to (width,height):
  // Make up a potentially reasonable but perhaps too destructive reflow
  // reason.
  nsReflowReason reason = (kid->GetStateBits() & NS_FRAME_FIRST_REFLOW)
                            ? eReflowReason_Initial
                            : eReflowReason_StyleChange;
  nsHTMLReflowState reflowState(presContext, kid, reason,
                                renderingContext, size);
  nsHTMLReflowMetrics desiredSize(nsnull);
  nsReflowStatus status;
  
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
nsSVGForeignObjectFrame::FlushDirtyRegion() {
  if (mDirtyRegion.IsEmpty() || mInReflow)
    return;

  nsSVGOuterSVGFrame *outerSVGFrame = nsSVGUtils::GetOuterSVGFrame(this);
  if (!outerSVGFrame) {
    NS_ERROR("null outerSVGFrame");
    return;
  }

  PRBool suspended;
  outerSVGFrame->IsRedrawSuspended(&suspended);
  if (suspended)
    return;

  nsRect rect = nsSVGUtils::FindFilterInvalidation(this);
  if (!rect.IsEmpty()) {
    outerSVGFrame->InvalidateRect(rect);
    return;
  }
  
  nsCOMPtr<nsIDOMSVGMatrix> tm = GetTMIncludingOffset();
  nsRect r = mDirtyRegion.GetBounds();
  r.ScaleRoundOut(GetPxPerTwips());
  float x = r.x, y = r.y, w = r.width, h = r.height;
  TransformRect(&x, &y, &w, &h, tm);
  r = nsSVGUtils::ToBoundingPixelRect(x, y, x+w, y+h);
  outerSVGFrame->InvalidateRect(r);

  mDirtyRegion.SetEmpty();
}

void
nsSVGForeignObjectFrame::InvalidateInternal(const nsRect& aDamageRect,
                                            nscoord aX, nscoord aY, nsIFrame* aForChild,
                                            PRBool aImmediate)
{
  mDirtyRegion.Or(mDirtyRegion, aDamageRect + nsPoint(aX, aY));
  FlushDirtyRegion();
}

float nsSVGForeignObjectFrame::GetPxPerTwips()
{
  float val = GetTwipsPerPx();
  
  NS_ASSERTION(val!=0.0f, "invalid px/twips");  
  if (val == 0.0) val = 1e-20f;
  
  return 1.0f/val;
}

float nsSVGForeignObjectFrame::GetTwipsPerPx()
{
  return GetPresContext()->ScaledPixelsToTwips();
}
