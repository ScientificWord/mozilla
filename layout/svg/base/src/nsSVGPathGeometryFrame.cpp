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
#include "nsIDOMSVGDocument.h"
#include "nsIDOMElement.h"
#include "nsIDocument.h"
#include "nsISVGRenderer.h"
#include "nsISVGValueUtils.h"
#include "nsSVGContainerFrame.h"
#include "nsReadableUtils.h"
#include "nsUnicharUtils.h"
#include "nsSVGAtoms.h"
#include "nsCRT.h"
#include "prdtoa.h"
#include "nsSVGMarkerFrame.h"
#include "nsIViewManager.h"
#include "nsSVGMatrix.h"
#include "nsSVGClipPathFrame.h"
#include "nsISVGRendererCanvas.h"
#include "nsIViewManager.h"
#include "nsSVGUtils.h"
#include "nsSVGFilterFrame.h"
#include "nsSVGMaskFrame.h"
#include "nsISVGRendererSurface.h"
#include "nsINameSpaceManager.h"
#include "nsSVGGraphicElement.h"
#include "nsSVGOuterSVGFrame.h"
#include "nsSVGRect.h"
#include "nsISVGCairoCanvas.h"
#include "nsSVGPathGeometryElement.h"

struct nsSVGMarkerProperty {
  nsSVGMarkerFrame *mMarkerStart;
  nsSVGMarkerFrame *mMarkerMid;
  nsSVGMarkerFrame *mMarkerEnd;

  nsSVGMarkerProperty() 
      : mMarkerStart(nsnull),
        mMarkerMid(nsnull),
        mMarkerEnd(nsnull)
  {}
};

//----------------------------------------------------------------------
// Implementation

nsIFrame*
NS_NewSVGPathGeometryFrame(nsIPresShell* aPresShell,
                           nsIContent* aContent,
                           nsStyleContext* aContext)
{
  return new (aPresShell) nsSVGPathGeometryFrame(aContext);
}

////////////////////////////////////////////////////////////////////////
// nsSVGPathGeometryFrame

nsSVGPathGeometryFrame::nsSVGPathGeometryFrame(nsStyleContext* aContext)
  : nsSVGPathGeometryFrameBase(aContext),
    mPropagateTransform(PR_TRUE)
{
#ifdef DEBUG
//  printf("nsSVGPathGeometryFrame %p CTOR\n", this);
#endif
}

nsSVGPathGeometryFrame::~nsSVGPathGeometryFrame()
{
#ifdef DEBUG
//  printf("~nsSVGPathGeometryFrame %p\n", this);
#endif
  
  if (GetStateBits() & NS_STATE_SVG_HAS_MARKERS) {
    DeleteProperty(nsGkAtoms::marker);
  }
}

//----------------------------------------------------------------------
// nsISupports methods

NS_INTERFACE_MAP_BEGIN(nsSVGPathGeometryFrame)
  NS_INTERFACE_MAP_ENTRY(nsISVGChildFrame)
NS_INTERFACE_MAP_END_INHERITING(nsSVGPathGeometryFrameBase)

//----------------------------------------------------------------------
// nsIFrame methods

NS_IMETHODIMP
nsSVGPathGeometryFrame::Init(nsIContent*      aContent,
                             nsIFrame*        aParent,
                             nsIFrame*        aPrevInFlow)
{
  mContent = aContent;
  NS_IF_ADDREF(mContent);
  mParent = aParent;

  if (mContent) {
    mContent->SetMayHaveFrame(PR_TRUE);
  }

  InitSVG();
  DidSetStyleContext();
    
  return NS_OK;
}

NS_IMETHODIMP
nsSVGPathGeometryFrame::AttributeChanged(PRInt32         aNameSpaceID,
                                         nsIAtom*        aAttribute,
                                         PRInt32         aModType)
{
  if (aNameSpaceID == kNameSpaceID_None &&
      (NS_STATIC_CAST(nsSVGPathGeometryElement*,
                      mContent)->IsDependentAttribute(aAttribute) ||
       aAttribute == nsGkAtoms::transform))
    UpdateGraphic();

  return NS_OK;
}

NS_IMETHODIMP
nsSVGPathGeometryFrame::DidSetStyleContext()
{
  nsSVGPathGeometryFrameBase::DidSetStyleContext();

  nsSVGUtils::StyleEffects(this);

  if (GetStateBits() & NS_STATE_SVG_HAS_MARKERS) {
    DeleteProperty(nsGkAtoms::marker);
    RemoveStateBits(NS_STATE_SVG_HAS_MARKERS);
  }

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
  return nsLayoutAtoms::svgPathGeometryFrame;
}

PRBool
nsSVGPathGeometryFrame::IsFrameOfType(PRUint32 aFlags) const
{
  return !(aFlags & ~nsIFrame::eSVG);
}

// marker helper
static void
RemoveMarkerObserver(nsSVGMarkerProperty *property,
                     nsIFrame            *aFrame,
                     nsISVGValue         *marker)
{
  if (!marker) return;
  if (property->mMarkerStart == marker)
    property->mMarkerStart = nsnull;
  if (property->mMarkerMid == marker)
    property->mMarkerMid = nsnull;
  if (property->mMarkerEnd == marker)
    property->mMarkerEnd = nsnull;
  nsSVGUtils::RemoveObserver(aFrame, marker);
}

static void
MarkerPropertyDtor(void *aObject, nsIAtom *aPropertyName,
                   void *aPropertyValue, void *aData)
{
  nsSVGMarkerProperty *property = NS_STATIC_CAST(nsSVGMarkerProperty *,
                                                 aPropertyValue);
  nsIFrame *frame = NS_STATIC_CAST(nsIFrame *, aObject);
  RemoveMarkerObserver(property, frame, property->mMarkerStart);
  RemoveMarkerObserver(property, frame, property->mMarkerMid);
  RemoveMarkerObserver(property, frame, property->mMarkerEnd);
  delete property;
}

nsSVGMarkerProperty *
nsSVGPathGeometryFrame::GetMarkerProperty()
{
  if (GetStateBits() & NS_STATE_SVG_HAS_MARKERS)
    return NS_STATIC_CAST(nsSVGMarkerProperty *,
                          GetProperty(nsGkAtoms::marker));

  return nsnull;
}

void
nsSVGPathGeometryFrame::GetMarkerFromStyle(nsSVGMarkerFrame   **aResult,
                                           nsSVGMarkerProperty *property,
                                           nsIURI              *aURI)
{
  if (aURI && !*aResult) {
    nsSVGMarkerFrame *marker;
    NS_GetSVGMarkerFrame(&marker, aURI, GetContent());
    if (marker) {
      if (property->mMarkerStart != marker &&
          property->mMarkerMid != marker &&
          property->mMarkerEnd != marker)
        nsSVGUtils::AddObserver(NS_STATIC_CAST(nsIFrame *, this),
                                NS_STATIC_CAST(nsSVGValue *, marker));
      *aResult = marker;
    }
  }
}

void
nsSVGPathGeometryFrame::UpdateMarkerProperty()
{
  const nsStyleSVG *style = GetStyleSVG();

  if (style->mMarkerStart || style->mMarkerMid || style->mMarkerEnd) {

    nsSVGMarkerProperty *property;
    if (GetStateBits() & NS_STATE_SVG_HAS_MARKERS) {
      property = NS_STATIC_CAST(nsSVGMarkerProperty *,
                                GetProperty(nsGkAtoms::marker));
    } else {
      property = new nsSVGMarkerProperty;
      if (!property) {
        NS_ERROR("Could not create marker property");
        return;
      }
      SetProperty(nsGkAtoms::marker, property, MarkerPropertyDtor);
      AddStateBits(NS_STATE_SVG_HAS_MARKERS);
    }
    GetMarkerFromStyle(&property->mMarkerStart, property, style->mMarkerStart);
    GetMarkerFromStyle(&property->mMarkerMid, property, style->mMarkerMid);
    GetMarkerFromStyle(&property->mMarkerEnd, property, style->mMarkerEnd);
  }
}

//----------------------------------------------------------------------
// nsISVGChildFrame methods

NS_IMETHODIMP
nsSVGPathGeometryFrame::PaintSVG(nsISVGRendererCanvas* canvas,
                                 nsRect *aDirtyRect)
{
  if (!GetStyleVisibility()->IsVisible())
    return NS_OK;

  /* render */
  Render(canvas);

  if (NS_STATIC_CAST(nsSVGPathGeometryElement*, mContent)->IsMarkable()) {
    // Marker Property is added lazily and may have been removed by a restyle
    UpdateMarkerProperty();
    nsSVGMarkerProperty *property = GetMarkerProperty();
      
    if (property &&
        (property->mMarkerEnd ||
         property->mMarkerMid ||
         property->mMarkerStart)) {
      float strokeWidth = GetStrokeWidth();
        
      nsTArray<nsSVGMark> marks;
      NS_STATIC_CAST(nsSVGPathGeometryElement*,
                     mContent)->GetMarkPoints(&marks);
        
      PRUint32 num = marks.Length();
        
      if (num && property->mMarkerStart)
        property->mMarkerStart->PaintMark(canvas, this,
                                          &marks[0], strokeWidth);
        
      if (num && property->mMarkerMid)
        for (PRUint32 i = 1; i < num - 1; i++)
          property->mMarkerMid->PaintMark(canvas, this,
                                          &marks[i], strokeWidth);
        
      if (num && property->mMarkerEnd)
        property->mMarkerEnd->PaintMark(canvas, this,
                                        &marks[num-1], strokeWidth);
    }
  }

  return NS_OK;
}

NS_IMETHODIMP
nsSVGPathGeometryFrame::GetFrameForPointSVG(float x, float y, nsIFrame** hit)
{
  *hit = nsnull;

  PRUint16 mask = GetHittestMask();
  if (!mask || !mRect.Contains(x, y))
    return NS_OK;

  PRBool isHit = PR_FALSE;

  cairo_t *ctx = cairo_create(nsSVGUtils::GetCairoComputationalSurface());
  cairo_set_tolerance(ctx, 1.0);

  GeneratePath(ctx, nsnull);
  double xx = x, yy = y;
  cairo_device_to_user(ctx, &xx, &yy);

  if (IsClipChild()) {
    if (GetClipRule() == NS_STYLE_FILL_RULE_EVENODD)
      cairo_set_fill_rule(ctx, CAIRO_FILL_RULE_EVEN_ODD);
    else
      cairo_set_fill_rule(ctx, CAIRO_FILL_RULE_WINDING);
  }

  if (mask & HITTEST_MASK_FILL)
    isHit = cairo_in_fill(ctx, xx, yy);
  if (!isHit && (mask & HITTEST_MASK_STROKE))
    isHit = cairo_in_stroke(ctx, xx, yy);

  cairo_destroy(ctx);

  if (isHit && nsSVGUtils::HitTestClip(this, x, y))
    *hit = this;

  return NS_OK;
}

NS_IMETHODIMP_(nsRect)
nsSVGPathGeometryFrame::GetCoveredRegion()
{
  if (NS_STATIC_CAST(nsSVGPathGeometryElement*, mContent)->IsMarkable()) {
    nsSVGMarkerProperty *property = GetMarkerProperty();

    if (!property ||
        (!property->mMarkerEnd &&
         !property->mMarkerMid &&
         !property->mMarkerStart))
      return mRect;

    nsRect rect(mRect);

    float strokeWidth = GetStrokeWidth();

    nsTArray<nsSVGMark> marks;
    NS_STATIC_CAST(nsSVGPathGeometryElement*, mContent)->GetMarkPoints(&marks);

    PRUint32 num = marks.Length();

    if (num && property->mMarkerStart) {
      nsRect mark;
      mark = property->mMarkerStart->RegionMark(this,
                                                &marks[0],
                                                strokeWidth);
      rect.UnionRect(rect, mark);
    }

    if (num && property->mMarkerMid)
      for (PRUint32 i = 1; i < num - 1; i++) {
        nsRect mark;
        mark = property->mMarkerMid->RegionMark(this,
                                                &marks[i],
                                                strokeWidth);
        rect.UnionRect(rect, mark);
      }

    if (num && property->mMarkerEnd) {
      nsRect mark;
      mark = property->mMarkerEnd->RegionMark(this,
                                              &marks[num-1],
                                              strokeWidth);

      rect.UnionRect(rect, mark);
    }

    return rect;
  }

  return mRect;
}

NS_IMETHODIMP
nsSVGPathGeometryFrame::UpdateCoveredRegion()
{
  mRect.Empty();

  PRBool hasFill = HasFill();
  PRBool hasStroke = HasStroke();

  if (hasFill || hasStroke) {
    cairo_t *ctx = cairo_create(nsSVGUtils::GetCairoComputationalSurface());

    GeneratePath(ctx, nsnull);

    double xmin, ymin, xmax, ymax;

    if (hasStroke) {
      SetupCairoStrokeGeometry(ctx);
      cairo_stroke_extents(ctx, &xmin, &ymin, &xmax, &ymax);
    } else
      cairo_fill_extents(ctx, &xmin, &ymin, &xmax, &ymax);

    cairo_user_to_device(ctx, &xmin, &ymin);
    cairo_user_to_device(ctx, &xmax, &ymax);

    cairo_destroy(ctx);

    mRect = nsSVGUtils::ToBoundingPixelRect(xmin, ymin, xmax, ymax);
  }

  // Add in markers
  mRect = GetCoveredRegion();

  return NS_OK;
}

NS_IMETHODIMP
nsSVGPathGeometryFrame::InitialUpdate()
{
  UpdateGraphic();

  return NS_OK;
}

NS_IMETHODIMP
nsSVGPathGeometryFrame::NotifyCanvasTMChanged(PRBool suppressInvalidation)
{
  UpdateGraphic(suppressInvalidation);
  
  return NS_OK;
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

NS_IMETHODIMP
nsSVGPathGeometryFrame::GetBBox(nsIDOMSVGRect **_retval)
{
  double xmin, ymin, xmax, ymax;

  cairo_t *ctx = cairo_create(nsSVGUtils::GetCairoComputationalSurface());
  GeneratePath(ctx, nsnull);

  cairo_fill_extents(ctx, &xmin, &ymin, &xmax, &ymax);

  /* cairo_fill_extents doesn't work on degenerate paths */
  if (xmin ==  32767 &&
      ymin ==  32767 &&
      xmax == -32768 &&
      ymax == -32768) {
    /* cairo_stroke_extents doesn't work with stroke width zero, fudge */
    cairo_set_line_width(ctx, 0.0001);
    cairo_stroke_extents(ctx, &xmin, &ymin, &xmax, &ymax);
  }

  cairo_user_to_device(ctx, &xmin, &ymin);
  cairo_user_to_device(ctx, &xmax, &ymax);

  cairo_destroy(ctx);

  return NS_NewSVGRect(_retval, xmin, ymin, xmax - xmin, ymax - ymin);
}

//----------------------------------------------------------------------
// nsISVGValueObserver methods:

NS_IMETHODIMP
nsSVGPathGeometryFrame::WillModifySVGObservable(nsISVGValue* observable,
                                                nsISVGValue::modificationType aModType)
{
  nsSVGUtils::WillModifyEffects(this, observable, aModType);

  return NS_OK;
}


NS_IMETHODIMP
nsSVGPathGeometryFrame::DidModifySVGObservable (nsISVGValue* observable,
                                                nsISVGValue::modificationType aModType)
{
  nsSVGUtils::DidModifyEffects(this, observable, aModType);

  nsSVGPathGeometryFrameBase::DidModifySVGObservable(observable, aModType);

  nsIFrame *frame = nsnull;
  CallQueryInterface(observable, &frame);
  if (!frame)
    return NS_OK;

  if (frame->GetType() == nsGkAtoms::svgFilterFrame) {
    UpdateGraphic();
  } else if (frame->GetType() == nsGkAtoms::svgMarkerFrame) {
    if (aModType == nsISVGValue::mod_die)
      RemoveMarkerObserver(NS_STATIC_CAST(nsSVGMarkerProperty *, 
                                          GetProperty(nsGkAtoms::marker)),
                           this,
                           observable);
    UpdateGraphic();
  }

  return NS_OK;
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

  nsSVGContainerFrame *containerFrame = NS_STATIC_CAST(nsSVGContainerFrame*,
                                                       mParent);
  nsCOMPtr<nsIDOMSVGMatrix> parentTM = containerFrame->GetCanvasTM();
  NS_ASSERTION(parentTM, "null TM");

  // append our local transformations if we have any:
  nsSVGGraphicElement *element =
    NS_STATIC_CAST(nsSVGGraphicElement*, mContent);
  nsCOMPtr<nsIDOMSVGMatrix> localTM = element->GetLocalTransformMatrix();

  if (localTM)
    return parentTM->Multiply(localTM, aCTM);

  *aCTM = parentTM;
  NS_ADDREF(*aCTM);
  return NS_OK;
}

//----------------------------------------------------------------------
// nsSVGPathGeometryFrame methods:

void
nsSVGPathGeometryFrame::Render(nsISVGRendererCanvas *aCanvas)
{
  nsCOMPtr<nsISVGCairoCanvas> cairoCanvas = do_QueryInterface(aCanvas);
  NS_ASSERTION(cairoCanvas, "wrong svg render context for geometry!");
  if (!cairoCanvas)
    return;

  cairo_t *ctx = cairoCanvas->GetContext();

  PRUint16 renderMode;
  aCanvas->GetRenderMode(&renderMode);

  /* save/pop the state so we don't screw up the xform */
  cairo_save(ctx);

  GeneratePath(ctx, cairoCanvas);

  if (renderMode != nsISVGRendererCanvas::SVG_RENDER_MODE_NORMAL) {
    cairo_restore(ctx);

    if (GetClipRule() == NS_STYLE_FILL_RULE_EVENODD)
      cairo_set_fill_rule(ctx, CAIRO_FILL_RULE_EVEN_ODD);
    else
      cairo_set_fill_rule(ctx, CAIRO_FILL_RULE_WINDING);

    if (renderMode == nsISVGRendererCanvas::SVG_RENDER_MODE_CLIP_MASK) {
      cairo_set_antialias(ctx, CAIRO_ANTIALIAS_NONE);
      cairo_set_source_rgba(ctx, 1.0f, 1.0f, 1.0f, 1.0f);
      cairo_fill(ctx);
    }

    return;
  }

  switch (GetStyleSVG()->mShapeRendering) {
  case NS_STYLE_SHAPE_RENDERING_OPTIMIZESPEED:
  case NS_STYLE_SHAPE_RENDERING_CRISPEDGES:
    cairo_set_antialias(ctx, CAIRO_ANTIALIAS_NONE);
    break;
  default:
    cairo_set_antialias(ctx, CAIRO_ANTIALIAS_DEFAULT);
    break;
  }

  void *closure;
  if (HasFill() && NS_SUCCEEDED(SetupCairoFill(aCanvas, ctx, &closure))) {
    cairo_fill_preserve(ctx);
    CleanupCairoFill(ctx, closure);
  }

  if (HasStroke() && NS_SUCCEEDED(SetupCairoStroke(aCanvas, ctx, &closure))) {
    cairo_stroke(ctx);
    CleanupCairoStroke(ctx, closure);
  }

  cairo_new_path(ctx);

  cairo_restore(ctx);
}

void
nsSVGPathGeometryFrame::GeneratePath(cairo_t *ctx, nsISVGCairoCanvas* aCanvas)
{
  nsCOMPtr<nsIDOMSVGMatrix> ctm;
  GetCanvasTM(getter_AddRefs(ctm));
  NS_ASSERTION(ctm, "graphic source didn't specify a ctm");

  cairo_matrix_t matrix = NS_ConvertSVGMatrixToCairo(ctm);
  if (aCanvas) {
    aCanvas->AdjustMatrixForInitialTransform(&matrix);
  }

  cairo_matrix_t inverse = matrix;
  if (cairo_matrix_invert(&inverse)) {
    cairo_identity_matrix(ctx);
    cairo_new_path(ctx);
    return;
  }
  cairo_set_matrix(ctx, &matrix);

  cairo_new_path(ctx);
  NS_STATIC_CAST(nsSVGPathGeometryElement*, mContent)->ConstructPath(ctx);
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
        mask |= HITTEST_MASK_FILL;
      }
      break;
    case NS_STYLE_POINTER_EVENTS_VISIBLESTROKE:
      if (GetStyleVisibility()->IsVisible()) {
        mask |= HITTEST_MASK_STROKE;
      }
      break;
    case NS_STYLE_POINTER_EVENTS_VISIBLE:
      if (GetStyleVisibility()->IsVisible()) {
        mask |= HITTEST_MASK_FILL;
        mask |= HITTEST_MASK_STROKE;
      }
      break;
    case NS_STYLE_POINTER_EVENTS_PAINTED:
      if (GetStyleSVG()->mFill.mType != eStyleSVGPaintType_None)
        mask |= HITTEST_MASK_FILL;
      if (GetStyleSVG()->mStroke.mType != eStyleSVGPaintType_None)
        mask |= HITTEST_MASK_STROKE;
      break;
    case NS_STYLE_POINTER_EVENTS_FILL:
      mask |= HITTEST_MASK_FILL;
      break;
    case NS_STYLE_POINTER_EVENTS_STROKE:
      mask |= HITTEST_MASK_STROKE;
      break;
    case NS_STYLE_POINTER_EVENTS_ALL:
      mask |= HITTEST_MASK_FILL;
      mask |= HITTEST_MASK_STROKE;
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

  PRBool suspended;
  outerSVGFrame->IsRedrawSuspended(&suspended);
  if (suspended) {
    AddStateBits(NS_STATE_SVG_DIRTY);
  } else {
    RemoveStateBits(NS_STATE_SVG_DIRTY);

    if (suppressInvalidation)
      return NS_OK;

    outerSVGFrame->InvalidateRect(mRect);
    UpdateCoveredRegion();

    nsRect filterRect;
    filterRect = nsSVGUtils::FindFilterInvalidation(this);
    if (!filterRect.IsEmpty()) {
      outerSVGFrame->InvalidateRect(filterRect);
    } else {
      outerSVGFrame->InvalidateRect(mRect);
    }
  }

  return NS_OK;
}


