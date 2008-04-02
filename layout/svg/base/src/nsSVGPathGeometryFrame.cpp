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
 * Portions created by the Initial Developer are Copyright (C) 2002
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

#include "nsSVGPathGeometryFrame.h"
#include "nsSVGContainerFrame.h"
#include "nsReadableUtils.h"
#include "nsUnicharUtils.h"
#include "nsGkAtoms.h"
#include "nsSVGMarkerFrame.h"
#include "nsSVGMatrix.h"
#include "nsSVGUtils.h"
#include "nsSVGGraphicElement.h"
#include "nsSVGOuterSVGFrame.h"
#include "nsSVGRect.h"
#include "nsSVGPathGeometryElement.h"
#include "gfxContext.h"

class nsSVGMarkerProperty : public nsStubMutationObserver {
public:
  nsSVGMarkerProperty(nsIURI                 *aMarkerStart,
                      nsIURI                 *aMarkerMid,
                      nsIURI                 *aMarkerEnd,
                      nsSVGPathGeometryFrame *aMarkedFrame);
  virtual ~nsSVGMarkerProperty();

  nsSVGMarkerFrame *GetMarkerStartFrame() {
    return GetMarkerFrame(mObservedMarkerStart);
  }
  nsSVGMarkerFrame *GetMarkerMidFrame() {
    return GetMarkerFrame(mObservedMarkerMid);
  }
  nsSVGMarkerFrame *GetMarkerEndFrame() {
    return GetMarkerFrame(mObservedMarkerEnd);
  }

  // nsISupports
  NS_DECL_ISUPPORTS

  // nsIMutationObserver
  NS_DECL_NSIMUTATIONOBSERVER_ATTRIBUTECHANGED
  NS_DECL_NSIMUTATIONOBSERVER_CONTENTAPPENDED
  NS_DECL_NSIMUTATIONOBSERVER_CONTENTINSERTED
  NS_DECL_NSIMUTATIONOBSERVER_CONTENTREMOVED
  NS_DECL_NSIMUTATIONOBSERVER_PARENTCHAINCHANGED

private:
  nsSVGMarkerFrame *GetMarkerFrame(nsWeakPtr aObservedMarker);
  already_AddRefed<nsIWeakReference>
  AddMutationObserver(nsIURI *aURI, nsIContent *aContent);
  void RemoveMutationObserver(nsWeakPtr aObservedMarker);
  void DoUpdate();

  nsWeakPtr mObservedMarkerStart, mObservedMarkerMid, mObservedMarkerEnd;
  nsSVGPathGeometryFrame *mFrame;  // frame being marked
};

NS_IMPL_ISUPPORTS1(nsSVGMarkerProperty, nsIMutationObserver)

nsSVGMarkerProperty::nsSVGMarkerProperty(nsIURI                 *aMarkerStart,
                                         nsIURI                 *aMarkerMid,
                                         nsIURI                 *aMarkerEnd,
                                         nsSVGPathGeometryFrame *aMarkedFrame)
  : mFrame(aMarkedFrame)
{
  nsIContent *content = mFrame->GetContent();

  mObservedMarkerStart = AddMutationObserver(aMarkerStart, content);
  mObservedMarkerMid = AddMutationObserver(aMarkerMid, content);
  mObservedMarkerEnd = AddMutationObserver(aMarkerEnd, content);

  NS_ADDREF(this); // addref to allow QI - SupportsDtorFunc releases
  mFrame->SetProperty(nsGkAtoms::marker,
                      static_cast<nsISupports*>(this),
                      nsPropertyTable::SupportsDtorFunc);

  mFrame->AddStateBits(NS_STATE_SVG_HAS_MARKERS);
}

nsSVGMarkerProperty::~nsSVGMarkerProperty()
{
  RemoveMutationObserver(mObservedMarkerStart);
  RemoveMutationObserver(mObservedMarkerMid);
  RemoveMutationObserver(mObservedMarkerEnd);

  mFrame->RemoveStateBits(NS_STATE_SVG_HAS_MARKERS);
}

nsSVGMarkerFrame *
nsSVGMarkerProperty::GetMarkerFrame(nsWeakPtr aObservedMarker)
{
  nsCOMPtr<nsIContent> marker = do_QueryReferent(aObservedMarker);
  if (marker) {
    nsIFrame *frame =
      static_cast<nsGenericElement*>(marker.get())->GetPrimaryFrame();
    if (frame && frame->GetType() == nsGkAtoms::svgMarkerFrame)
      return static_cast<nsSVGMarkerFrame*>(frame);
  }
  return nsnull;
}

already_AddRefed<nsIWeakReference>
nsSVGMarkerProperty::AddMutationObserver(nsIURI      *aURI,
                                         nsIContent  *aContent)
{
  if (!aURI)
    return nsnull;

  nsIContent *marker = NS_GetSVGMarkerElement(aURI, aContent);
  if (marker) {
    marker->AddMutationObserver(this);
    return do_GetWeakReference(marker);
  }
  return nsnull;
}

void
nsSVGMarkerProperty::RemoveMutationObserver(nsWeakPtr aObservedMarker)
{
  if (!aObservedMarker)
    return;

  nsCOMPtr<nsIContent> marker = do_QueryReferent(aObservedMarker);
  if (marker)
    marker->RemoveMutationObserver(this);
}

void
nsSVGMarkerProperty::DoUpdate()
{
  mFrame->UpdateGraphic();
}

void
nsSVGMarkerProperty::AttributeChanged(nsIDocument *aDocument,
                                      nsIContent *aContent,
                                      PRInt32 aNameSpaceID,
                                      nsIAtom *aAttribute,
                                      PRInt32 aModType,
                                      PRUint32 aStateMask)
{
  DoUpdate();
}

void
nsSVGMarkerProperty::ContentAppended(nsIDocument *aDocument,
                                     nsIContent *aContainer,
                                     PRInt32 aNewIndexInContainer)
{
  DoUpdate();
}

void
nsSVGMarkerProperty::ContentInserted(nsIDocument *aDocument,
                                     nsIContent *aContainer,
                                     nsIContent *aChild,
                                     PRInt32 aIndexInContainer)
{
  DoUpdate();
}

void
nsSVGMarkerProperty::ContentRemoved(nsIDocument *aDocument,
                                    nsIContent *aContainer,
                                    nsIContent *aChild,
                                    PRInt32 aIndexInContainer)
{
  DoUpdate();
}

void
nsSVGMarkerProperty::ParentChainChanged(nsIContent *aContent)
{
  if (aContent->IsInDoc())
    return;

  mFrame->DeleteProperty(nsGkAtoms::marker);
}

//----------------------------------------------------------------------
// Implementation

nsIFrame*
NS_NewSVGPathGeometryFrame(nsIPresShell* aPresShell,
                           nsIContent* aContent,
                           nsStyleContext* aContext)
{
  return new (aPresShell) nsSVGPathGeometryFrame(aContext);
}

//----------------------------------------------------------------------
// nsISupports methods

NS_INTERFACE_MAP_BEGIN(nsSVGPathGeometryFrame)
  NS_INTERFACE_MAP_ENTRY(nsISVGChildFrame)
NS_INTERFACE_MAP_END_INHERITING(nsSVGPathGeometryFrameBase)

//----------------------------------------------------------------------
// nsIFrame methods

void
nsSVGPathGeometryFrame::Destroy()
{
  RemovePathProperties();
  nsSVGPathGeometryFrameBase::Destroy();
}

NS_IMETHODIMP
nsSVGPathGeometryFrame::AttributeChanged(PRInt32         aNameSpaceID,
                                         nsIAtom*        aAttribute,
                                         PRInt32         aModType)
{
  if (aNameSpaceID == kNameSpaceID_None &&
      (static_cast<nsSVGPathGeometryElement*>
                  (mContent)->IsDependentAttribute(aAttribute) ||
       aAttribute == nsGkAtoms::transform))
    UpdateGraphic();

  return NS_OK;
}

NS_IMETHODIMP
nsSVGPathGeometryFrame::DidSetStyleContext()
{
  nsSVGPathGeometryFrameBase::DidSetStyleContext();

  nsSVGOuterSVGFrame *outerSVGFrame = nsSVGUtils::GetOuterSVGFrame(this);
  if (outerSVGFrame) {
    // invalidate here while we still have the filter information
    outerSVGFrame->InvalidateRect(nsSVGUtils::FindFilterInvalidation(this));
  }

  RemovePathProperties();

  // XXX: we'd like to use the style_hint mechanism and the
  // ContentStateChanged/AttributeChanged functions for style changes
  // to get slightly finer granularity, but unfortunately the
  // style_hints don't map very well onto svg. Here seems to be the
  // best place to deal with style changes:

  UpdateGraphic();

  return NS_OK;
}

nsIAtom *
nsSVGPathGeometryFrame::GetType() const
{
  return nsGkAtoms::svgPathGeometryFrame;
}

//----------------------------------------------------------------------
// nsISVGChildFrame methods

NS_IMETHODIMP
nsSVGPathGeometryFrame::PaintSVG(nsSVGRenderState *aContext,
                                 nsRect *aDirtyRect)
{
  if (!GetStyleVisibility()->IsVisible())
    return NS_OK;

  /* render */
  Render(aContext);

  if (static_cast<nsSVGPathGeometryElement*>(mContent)->IsMarkable()) {
    nsSVGMarkerProperty *property = GetMarkerProperty();
      
    if (property) {
      float strokeWidth = GetStrokeWidth();
        
      nsTArray<nsSVGMark> marks;
      static_cast<nsSVGPathGeometryElement*>
                 (mContent)->GetMarkPoints(&marks);
        
      PRUint32 num = marks.Length();

      if (num) {
        nsSVGMarkerFrame *frame = property->GetMarkerStartFrame();
        if (frame)
          frame->PaintMark(aContext, this, &marks[0], strokeWidth);

        frame = property->GetMarkerMidFrame();
        if (frame) {
          for (PRUint32 i = 1; i < num - 1; i++)
            frame->PaintMark(aContext, this, &marks[i], strokeWidth);
        }

        frame = property->GetMarkerEndFrame();
        if (frame)
          frame->PaintMark(aContext, this, &marks[num-1], strokeWidth);
      }
    }
  }

  return NS_OK;
}

NS_IMETHODIMP
nsSVGPathGeometryFrame::GetFrameForPointSVG(float x, float y, nsIFrame** hit)
{
  *hit = nsnull;

  PRUint16 fillRule, mask;
  // check if we're a clipPath - cheaper than IsClipChild(), and we shouldn't
  // get in here for other nondisplay children
  if (GetStateBits() & NS_STATE_SVG_NONDISPLAY_CHILD) {
    NS_ASSERTION(IsClipChild(), "should be in clipPath but we're not");
    mask = HITTEST_MASK_FILL;
    fillRule = GetClipRule();
  } else {
    mask = GetHittestMask();
    if (!mask || (!(mask & HITTEST_MASK_FORCE_TEST) &&
                  !mRect.Contains(nscoord(x), nscoord(y))))
      return NS_OK;
    fillRule = GetStyleSVG()->mFillRule;
  }

  PRBool isHit = PR_FALSE;

  gfxContext context(nsSVGUtils::GetThebesComputationalSurface());

  GeneratePath(&context);
  gfxPoint devicePoint = context.DeviceToUser(gfxPoint(x, y));

  if (fillRule == NS_STYLE_FILL_RULE_EVENODD)
    context.SetFillRule(gfxContext::FILL_RULE_EVEN_ODD);
  else
    context.SetFillRule(gfxContext::FILL_RULE_WINDING);

  if (mask & HITTEST_MASK_FILL)
    isHit = context.PointInFill(devicePoint);
  if (!isHit && (mask & HITTEST_MASK_STROKE)) {
    SetupCairoStrokeHitGeometry(&context);
    isHit = context.PointInStroke(devicePoint);
  }

  if (isHit && nsSVGUtils::HitTestClip(this, x, y))
    *hit = this;

  return NS_OK;
}

NS_IMETHODIMP_(nsRect)
nsSVGPathGeometryFrame::GetCoveredRegion()
{
  if (static_cast<nsSVGPathGeometryElement*>(mContent)->IsMarkable()) {
    nsSVGMarkerProperty *property = GetMarkerProperty();

    if (!property)
      return mRect;

    nsRect rect(mRect);

    float strokeWidth = GetStrokeWidth();

    nsTArray<nsSVGMark> marks;
    static_cast<nsSVGPathGeometryElement*>(mContent)->GetMarkPoints(&marks);

    PRUint32 num = marks.Length();

    if (num) {
      nsSVGMarkerFrame *frame = property->GetMarkerStartFrame();
      if (frame) {
        nsRect mark = frame->RegionMark(this, &marks[0], strokeWidth);
        rect.UnionRect(rect, mark);
      }

      frame = property->GetMarkerMidFrame();
      if (frame) {
        for (PRUint32 i = 1; i < num - 1; i++) {
          nsRect mark = frame->RegionMark(this, &marks[i], strokeWidth);
          rect.UnionRect(rect, mark);
        }
      }

      frame = property->GetMarkerEndFrame();
      if (frame) {
        nsRect mark = frame->RegionMark(this, &marks[num-1], strokeWidth);
        rect.UnionRect(rect, mark);
      }
    }

    return rect;
  }

  return mRect;
}

NS_IMETHODIMP
nsSVGPathGeometryFrame::UpdateCoveredRegion()
{
  mRect.Empty();

  gfxContext context(nsSVGUtils::GetThebesComputationalSurface());

  GeneratePath(&context);

  gfxRect extent;

  if (HasStroke()) {
    SetupCairoStrokeGeometry(&context);
    extent = context.GetUserStrokeExtent();
    if (!IsDegeneratePath(extent)) {
      extent = context.UserToDevice(extent);
      mRect = nsSVGUtils::ToBoundingPixelRect(extent);
    }
  } else {
    context.IdentityMatrix();
    extent = context.GetUserPathExtent();
    if (!IsDegeneratePath(extent)) {
      mRect = nsSVGUtils::ToBoundingPixelRect(extent);
    }
  }

  // Add in markers
  mRect = GetCoveredRegion();

  return NS_OK;
}

NS_IMETHODIMP
nsSVGPathGeometryFrame::InitialUpdate()
{
  NS_ASSERTION(GetStateBits() & NS_FRAME_FIRST_REFLOW,
               "Yikes! We've been called already! Hopefully we weren't called "
               "before our nsSVGOuterSVGFrame's initial Reflow()!!!");

  UpdateGraphic();

  NS_ASSERTION(!(mState & NS_FRAME_IN_REFLOW),
               "We don't actually participate in reflow");
  
  // Do unset the various reflow bits, though.
  mState &= ~(NS_FRAME_FIRST_REFLOW | NS_FRAME_IS_DIRTY |
              NS_FRAME_HAS_DIRTY_CHILDREN);
  return NS_OK;
}

void
nsSVGPathGeometryFrame::NotifySVGChanged(PRUint32 aFlags)
{
  UpdateGraphic((aFlags & SUPPRESS_INVALIDATION) != 0);
}

NS_IMETHODIMP
nsSVGPathGeometryFrame::NotifyRedrawSuspended()
{
  // XXX should we cache the fact that redraw is suspended?
  return NS_OK;
}

NS_IMETHODIMP
nsSVGPathGeometryFrame::NotifyRedrawUnsuspended()
{
  if (GetStateBits() & NS_STATE_SVG_DIRTY)
    UpdateGraphic();

  return NS_OK;
}

NS_IMETHODIMP
nsSVGPathGeometryFrame::SetMatrixPropagation(PRBool aPropagate)
{
  mPropagateTransform = aPropagate;
  return NS_OK;
}

NS_IMETHODIMP
nsSVGPathGeometryFrame::SetOverrideCTM(nsIDOMSVGMatrix *aCTM)
{
  mOverrideCTM = aCTM;
  return NS_OK;
}

already_AddRefed<nsIDOMSVGMatrix>
nsSVGPathGeometryFrame::GetOverrideCTM()
{
  nsIDOMSVGMatrix *matrix = mOverrideCTM.get();
  NS_IF_ADDREF(matrix);
  return matrix;
}

NS_IMETHODIMP
nsSVGPathGeometryFrame::GetBBox(nsIDOMSVGRect **_retval)
{
  gfxContext context(nsSVGUtils::GetThebesComputationalSurface());

  GeneratePath(&context);
  context.IdentityMatrix();

  return NS_NewSVGRect(_retval, context.GetUserPathExtent());
}

//----------------------------------------------------------------------
// nsSVGGeometryFrame methods:

/* readonly attribute nsIDOMSVGMatrix canvasTM; */
NS_IMETHODIMP
nsSVGPathGeometryFrame::GetCanvasTM(nsIDOMSVGMatrix * *aCTM)
{
  *aCTM = nsnull;

  if (!mPropagateTransform) {
    if (mOverrideCTM) {
      *aCTM = mOverrideCTM;
      NS_ADDREF(*aCTM);
      return NS_OK;
    }
    return NS_NewSVGMatrix(aCTM);
  }

  nsSVGContainerFrame *containerFrame = static_cast<nsSVGContainerFrame*>
                                                   (mParent);
  nsCOMPtr<nsIDOMSVGMatrix> parentTM = containerFrame->GetCanvasTM();
  NS_ASSERTION(parentTM, "null TM");

  // append our local transformations if we have any:
  nsSVGGraphicElement *element =
    static_cast<nsSVGGraphicElement*>(mContent);
  nsCOMPtr<nsIDOMSVGMatrix> localTM = element->GetLocalTransformMatrix();

  if (localTM)
    return parentTM->Multiply(localTM, aCTM);

  *aCTM = parentTM;
  NS_ADDREF(*aCTM);
  return NS_OK;
}

//----------------------------------------------------------------------
// nsSVGPathGeometryFrame methods:

nsSVGMarkerProperty *
nsSVGPathGeometryFrame::GetMarkerProperty()
{
  if (GetStateBits() & NS_STATE_SVG_HAS_MARKERS)
    return static_cast<nsSVGMarkerProperty *>
                      (GetProperty(nsGkAtoms::marker));

  return nsnull;
}

void
nsSVGPathGeometryFrame::UpdateMarkerProperty()
{
  if (GetStateBits() & NS_STATE_SVG_HAS_MARKERS)
    return;

  const nsStyleSVG *style = GetStyleSVG();

  if ((style->mMarkerStart || style->mMarkerMid || style->mMarkerEnd) &&
      !new nsSVGMarkerProperty(style->mMarkerStart,
                               style->mMarkerMid,
                               style->mMarkerEnd,
                               this)) {
    NS_ERROR("Could not create marker property");
    return;
  }
}

void
nsSVGPathGeometryFrame::RemovePathProperties()
{
  nsSVGUtils::StyleEffects(this);

  if (GetStateBits() & NS_STATE_SVG_HAS_MARKERS)
    DeleteProperty(nsGkAtoms::marker);
}

void
nsSVGPathGeometryFrame::Render(nsSVGRenderState *aContext)
{
  gfxContext *gfx = aContext->GetGfxContext();

  PRUint16 renderMode = aContext->GetRenderMode();

  /* save/pop the state so we don't screw up the xform */
  gfx->Save();

  GeneratePath(gfx);

  if (renderMode != nsSVGRenderState::NORMAL) {
    gfx->Restore();

    if (GetClipRule() == NS_STYLE_FILL_RULE_EVENODD)
      gfx->SetFillRule(gfxContext::FILL_RULE_EVEN_ODD);
    else
      gfx->SetFillRule(gfxContext::FILL_RULE_WINDING);

    if (renderMode == nsSVGRenderState::CLIP_MASK) {
      gfx->SetAntialiasMode(gfxContext::MODE_ALIASED);
      gfx->SetColor(gfxRGBA(1.0f, 1.0f, 1.0f, 1.0f));
      gfx->Fill();
      gfx->NewPath();
    }

    return;
  }

  switch (GetStyleSVG()->mShapeRendering) {
  case NS_STYLE_SHAPE_RENDERING_OPTIMIZESPEED:
  case NS_STYLE_SHAPE_RENDERING_CRISPEDGES:
    gfx->SetAntialiasMode(gfxContext::MODE_ALIASED);
    break;
  default:
    gfx->SetAntialiasMode(gfxContext::MODE_COVERAGE);
    break;
  }

  if (HasFill() && SetupCairoFill(gfx)) {
    gfx->Fill();
  }

  if (HasStroke() && SetupCairoStroke(gfx)) {
    gfx->Stroke();
  }

  gfx->NewPath();

  gfx->Restore();
}

void
nsSVGPathGeometryFrame::GeneratePath(gfxContext* aContext)
{
  nsCOMPtr<nsIDOMSVGMatrix> ctm;
  GetCanvasTM(getter_AddRefs(ctm));
  NS_ASSERTION(ctm, "graphic source didn't specify a ctm");

  gfxMatrix matrix = nsSVGUtils::ConvertSVGMatrixToThebes(ctm);

  if (matrix.IsSingular()) {
    aContext->IdentityMatrix();
    aContext->NewPath();
    return;
  }

  aContext->Multiply(matrix);

  aContext->NewPath();
  static_cast<nsSVGPathGeometryElement*>(mContent)->ConstructPath(aContext);
}

PRUint16
nsSVGPathGeometryFrame::GetHittestMask()
{
  PRUint16 mask = 0;

  switch(GetStyleSVG()->mPointerEvents) {
    case NS_STYLE_POINTER_EVENTS_NONE:
      break;
    case NS_STYLE_POINTER_EVENTS_VISIBLEPAINTED:
      if (GetStyleVisibility()->IsVisible()) {
        if (GetStyleSVG()->mFill.mType != eStyleSVGPaintType_None)
          mask |= HITTEST_MASK_FILL;
        if (GetStyleSVG()->mStroke.mType != eStyleSVGPaintType_None)
          mask |= HITTEST_MASK_STROKE;
      }
      break;
    case NS_STYLE_POINTER_EVENTS_VISIBLEFILL:
      if (GetStyleVisibility()->IsVisible()) {
        mask |= HITTEST_MASK_FILL | HITTEST_MASK_FORCE_TEST;
      }
      break;
    case NS_STYLE_POINTER_EVENTS_VISIBLESTROKE:
      if (GetStyleVisibility()->IsVisible()) {
        mask |= HITTEST_MASK_STROKE | HITTEST_MASK_FORCE_TEST;
      }
      break;
    case NS_STYLE_POINTER_EVENTS_VISIBLE:
      if (GetStyleVisibility()->IsVisible()) {
        mask |=
          HITTEST_MASK_FILL |
          HITTEST_MASK_STROKE |
          HITTEST_MASK_FORCE_TEST;
      }
      break;
    case NS_STYLE_POINTER_EVENTS_PAINTED:
      if (GetStyleSVG()->mFill.mType != eStyleSVGPaintType_None)
        mask |= HITTEST_MASK_FILL;
      if (GetStyleSVG()->mStroke.mType != eStyleSVGPaintType_None)
        mask |= HITTEST_MASK_STROKE;
      break;
    case NS_STYLE_POINTER_EVENTS_FILL:
      mask |= HITTEST_MASK_FILL | HITTEST_MASK_FORCE_TEST;
      break;
    case NS_STYLE_POINTER_EVENTS_STROKE:
      mask |= HITTEST_MASK_STROKE | HITTEST_MASK_FORCE_TEST;
      break;
    case NS_STYLE_POINTER_EVENTS_ALL:
      mask |=
        HITTEST_MASK_FILL |
        HITTEST_MASK_STROKE |
        HITTEST_MASK_FORCE_TEST;
      break;
    default:
      NS_ERROR("not reached");
      break;
  }

  return mask;
}

//---------------------------------------------------------------------- 

nsresult
nsSVGPathGeometryFrame::UpdateGraphic(PRBool suppressInvalidation)
{
  if (GetStateBits() & NS_STATE_SVG_NONDISPLAY_CHILD)
    return NS_OK;

  nsSVGOuterSVGFrame *outerSVGFrame = nsSVGUtils::GetOuterSVGFrame(this);
  if (!outerSVGFrame) {
    NS_ERROR("null outerSVGFrame");
    return NS_ERROR_FAILURE;
  }

  if (outerSVGFrame->IsRedrawSuspended()) {
    AddStateBits(NS_STATE_SVG_DIRTY);
  } else {
    RemoveStateBits(NS_STATE_SVG_DIRTY);

    if (suppressInvalidation)
      return NS_OK;

    outerSVGFrame->InvalidateRect(nsSVGUtils::FindFilterInvalidation(this));

    UpdateMarkerProperty();
    UpdateCoveredRegion();
    nsSVGUtils::UpdateFilterRegion(this);

    outerSVGFrame->InvalidateRect(nsSVGUtils::FindFilterInvalidation(this));
  }

  return NS_OK;
}


