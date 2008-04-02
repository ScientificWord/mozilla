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
 * Scooter Morris.
 * Portions created by the Initial Developer are Copyright (C) 2004
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Scooter Morris <scootermorris@comcast.net>
 *   Jonathan Watt <jwatt@jwatt.org>
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

#include "nsIDOMSVGAnimatedNumber.h"
#include "nsIDOMSVGAnimatedString.h"
#include "nsIDOMSVGAnimTransformList.h"
#include "nsSVGTransformList.h"
#include "nsSVGMatrix.h"
#include "nsIDOMSVGStopElement.h"
#include "nsSVGGradientElement.h"
#include "nsSVGGeometryFrame.h"
#include "nsSVGGradientFrame.h"
#include "gfxContext.h"
#include "nsIDOMSVGRect.h"
#include "gfxPattern.h"

//----------------------------------------------------------------------
// Implementation

nsSVGGradientFrame::nsSVGGradientFrame(nsStyleContext* aContext,
                                       nsIDOMSVGURIReference *aRef) :
  nsSVGGradientFrameBase(aContext),
  mNextGrad(nsnull), 
  mLoopFlag(PR_FALSE),
  mInitialized(PR_FALSE) 
{
  if (aRef) {
    // Get the href
    aRef->GetHref(getter_AddRefs(mHref));
  }
}

nsSVGGradientFrame::~nsSVGGradientFrame()
{
  WillModify(mod_die);
  // Notify the world that we're dying
  DidModify(mod_die);

  if (mNextGrad) 
    mNextGrad->RemoveObserver(this);
}

//----------------------------------------------------------------------
// nsISupports methods:

NS_INTERFACE_MAP_BEGIN(nsSVGGradientFrame)
  NS_INTERFACE_MAP_ENTRY(nsISVGValueObserver)
  NS_INTERFACE_MAP_ENTRY(nsISupportsWeakReference)
NS_INTERFACE_MAP_END_INHERITING(nsSVGGradientFrameBase)

//----------------------------------------------------------------------
// nsISVGValueObserver methods:
NS_IMETHODIMP
nsSVGGradientFrame::WillModifySVGObservable(nsISVGValue* observable,
                                            modificationType aModType)
{
  // return if we have an mObservers loop
  if (mLoopFlag) {
    // XXXjwatt: we should really send an error to the JavaScript Console here:
    NS_WARNING("gradient reference loop detected while notifying observers!");
    return NS_OK;
  }

  // Don't pass on mod_die - our gradient observers would stop observing us!
  if (aModType == mod_die)
    aModType = mod_other;

  WillModify(aModType);
  return NS_OK;
}
                                                                                
NS_IMETHODIMP
nsSVGGradientFrame::DidModifySVGObservable(nsISVGValue* observable, 
                                           nsISVGValue::modificationType aModType)
{
  // return if we have an mObservers loop
  if (mLoopFlag) {
    // XXXjwatt: we should really send an error to the JavaScript Console here:
    NS_WARNING("gradient reference loop detected while notifying observers!");
    return NS_OK;
  }

  // If we reference another gradient and it's going away, null out mNextGrad
  if (mNextGrad && aModType == nsISVGValue::mod_die) {
    nsIFrame *gradient = nsnull;
    CallQueryInterface(observable, &gradient);
    if (mNextGrad == gradient)
      mNextGrad = nsnull;
  }

  // Don't pass on mod_die - our gradient observers would stop observing us!
  if (aModType == mod_die)
    aModType = mod_other;

  DidModify(aModType);
  return NS_OK;
}

//----------------------------------------------------------------------
// nsIFrame methods:

NS_IMETHODIMP
nsSVGGradientFrame::DidSetStyleContext()
{
  WillModify();
  DidModify();
  return NS_OK;
}

NS_IMETHODIMP
nsSVGGradientFrame::RemoveFrame(nsIAtom*        aListName,
                                nsIFrame*       aOldFrame)
{
  WillModify();
  DidModify();
  PRBool result = mFrames.DestroyFrame(aOldFrame);
  return result ? NS_OK : NS_ERROR_FAILURE;
}

nsIAtom*
nsSVGGradientFrame::GetType() const
{
  return nsGkAtoms::svgGradientFrame;
}

NS_IMETHODIMP
nsSVGGradientFrame::AttributeChanged(PRInt32         aNameSpaceID,
                                     nsIAtom*        aAttribute,
                                     PRInt32         aModType)
{
  if (aNameSpaceID == kNameSpaceID_None &&
      (aAttribute == nsGkAtoms::gradientUnits ||
       aAttribute == nsGkAtoms::gradientTransform ||
       aAttribute == nsGkAtoms::spreadMethod)) {
    WillModify();
    DidModify();
    return NS_OK;
  } 

  if (aNameSpaceID == kNameSpaceID_XLink &&
      aAttribute == nsGkAtoms::href) {
    if (mNextGrad)
      mNextGrad->RemoveObserver(this);
    WillModify();
    GetRefedGradientFromHref();
    DidModify();
    return NS_OK;
  }

  return nsSVGGradientFrameBase::AttributeChanged(aNameSpaceID,
                                                  aAttribute, aModType);
}

//----------------------------------------------------------------------

PRUint32
nsSVGGradientFrame::GetStopCount()
{
  return GetStopFrame(-1, nsnull);
}

void
nsSVGGradientFrame::GetStopInformation(PRInt32 aIndex,
                                       float *aOffset,
                                       nscolor *aStopColor,
                                       float *aStopOpacity)
{
  *aOffset = 0.0f;
  *aStopColor = NS_RGBA(0, 0, 0, 0);
  *aStopOpacity = 1.0f;

  nsIFrame *stopFrame = nsnull;
  GetStopFrame(aIndex, &stopFrame);
  nsCOMPtr<nsIDOMSVGStopElement> stopElement =
    do_QueryInterface(stopFrame->GetContent());

  if (stopElement) {
    nsCOMPtr<nsIDOMSVGAnimatedNumber> aNum;
    stopElement->GetOffset(getter_AddRefs(aNum));

    aNum->GetAnimVal(aOffset);
    if (*aOffset < 0.0f)
      *aOffset = 0.0f;
    else if (*aOffset > 1.0f)
      *aOffset = 1.0f;
  }

  if (stopFrame) {
    *aStopColor   = stopFrame->GetStyleSVGReset()->mStopColor;
    *aStopOpacity = stopFrame->GetStyleSVGReset()->mStopOpacity;
  }
#ifdef DEBUG
  // One way or another we have an implementation problem if we get here
  else if (stopElement) {
    NS_WARNING("We *do* have a stop but can't use it because it doesn't have "
               "a frame - we need frame free gradients and stops!");
  }
  else {
    NS_ERROR("Don't call me with an invalid stop index!");
  }
#endif
}

gfxMatrix
nsSVGGradientFrame::GetGradientTransform(nsSVGGeometryFrame *aSource)
{
  gfxMatrix bboxMatrix;

  PRUint16 gradientUnits = GetGradientUnits();
  nsIAtom *callerType = aSource->GetType();
  if (gradientUnits == nsIDOMSVGUnitTypes::SVG_UNIT_TYPE_USERSPACEONUSE) {
    // If this gradient is applied to text, our caller
    // will be the glyph, which is not a container, so we
    // need to get the parent
    if (callerType ==  nsGkAtoms::svgGlyphFrame)
      mSourceContent = static_cast<nsSVGElement*>
                                  (aSource->GetContent()->GetParent());
    else
      mSourceContent = static_cast<nsSVGElement*>(aSource->GetContent());
    NS_ASSERTION(mSourceContent, "Can't get content for gradient");
  }
  else {
    NS_ASSERTION(gradientUnits == nsIDOMSVGUnitTypes::SVG_UNIT_TYPE_OBJECTBOUNDINGBOX,
                 "Unknown gradientUnits type");
    // objectBoundingBox is the default anyway

    nsISVGChildFrame *frame = nsnull;
    if (aSource) {
      if (callerType == nsGkAtoms::svgGlyphFrame)
        CallQueryInterface(aSource->GetParent(), &frame);
      else
        CallQueryInterface(aSource, &frame);
    }
    nsCOMPtr<nsIDOMSVGRect> rect;
    if (frame) {
      nsCOMPtr<nsIDOMSVGMatrix> matrix = frame->GetOverrideCTM();
      frame->SetMatrixPropagation(PR_FALSE);
      frame->SetOverrideCTM(nsnull);
      frame->NotifySVGChanged(nsISVGChildFrame::SUPPRESS_INVALIDATION |
                              nsISVGChildFrame::TRANSFORM_CHANGED);
      frame->GetBBox(getter_AddRefs(rect));
      frame->SetMatrixPropagation(PR_TRUE);
      frame->SetOverrideCTM(matrix);
      frame->NotifySVGChanged(nsISVGChildFrame::SUPPRESS_INVALIDATION |
                              nsISVGChildFrame::TRANSFORM_CHANGED);
    }
    if (rect) {
      float x, y, width, height;
      rect->GetX(&x);
      rect->GetY(&y);
      rect->GetWidth(&width);
      rect->GetHeight(&height);
      bboxMatrix = gfxMatrix(width, 0, 0, height, x, y);
    }
  }

  nsIContent *gradient = GetGradientWithAttr(nsGkAtoms::gradientTransform);
  if (!gradient)
    gradient = mContent;  // use our gradient to get the correct default value

  nsSVGGradientElement *gradElement = static_cast<nsSVGGradientElement*>
                                                 (gradient);
  nsCOMPtr<nsIDOMSVGTransformList> trans;
  gradElement->mGradientTransform->GetAnimVal(getter_AddRefs(trans));
  nsCOMPtr<nsIDOMSVGMatrix> gradientTransform =
    nsSVGTransformList::GetConsolidationMatrix(trans);

  if (!gradientTransform)
    return bboxMatrix;

  return nsSVGUtils::ConvertSVGMatrixToThebes(gradientTransform) * bboxMatrix;
}

PRUint16
nsSVGGradientFrame::GetSpreadMethod()
{
  nsIContent *gradient = GetGradientWithAttr(nsGkAtoms::spreadMethod);
  if (!gradient)
    gradient = mContent;  // use our gradient to get the correct default value

  nsSVGGradientElement *gradElement = static_cast<nsSVGGradientElement*>
                                                 (gradient);

  return gradElement->mEnumAttributes[nsSVGGradientElement::SPREADMETHOD].GetAnimValue();
}

//----------------------------------------------------------------------
// nsSVGPaintServerFrame methods:

PRBool
nsSVGGradientFrame::SetupPaintServer(gfxContext *aContext,
                                     nsSVGGeometryFrame *aSource,
                                     float aGraphicOpacity)
{
  // Get the transform list (if there is one)
  gfxMatrix patternMatrix = GetGradientTransform(aSource);

  if (patternMatrix.IsSingular())
    return PR_FALSE;

  PRUint32 nStops = GetStopCount();

  // SVG specification says that no stops should be treated like
  // the corresponding fill or stroke had "none" specified.
  if (nStops == 0) {
    aContext->SetColor(gfxRGBA(0, 0, 0, 0));
    return PR_TRUE;
  }

  patternMatrix.Invert();

  nsRefPtr<gfxPattern> gradient = CreateGradient();
  if (!gradient)
    return PR_FALSE;

  PRUint16 aSpread = GetSpreadMethod();
  if (aSpread == nsIDOMSVGGradientElement::SVG_SPREADMETHOD_PAD)
    gradient->SetExtend(gfxPattern::EXTEND_PAD);
  else if (aSpread == nsIDOMSVGGradientElement::SVG_SPREADMETHOD_REFLECT)
    gradient->SetExtend(gfxPattern::EXTEND_REFLECT);
  else if (aSpread == nsIDOMSVGGradientElement::SVG_SPREADMETHOD_REPEAT)
    gradient->SetExtend(gfxPattern::EXTEND_REPEAT);

  gradient->SetMatrix(patternMatrix);

  // setup stops
  float lastOffset = 0.0f;

  for (PRUint32 i = 0; i < nStops; i++) {
    float offset, stopOpacity;
    nscolor stopColor;

    GetStopInformation(i, &offset, &stopColor, &stopOpacity);

    if (offset < lastOffset)
      offset = lastOffset;
    else
      lastOffset = offset;

    gradient->AddColorStop(offset,
                           gfxRGBA(NS_GET_R(stopColor)/255.0,
                                   NS_GET_G(stopColor)/255.0,
                                   NS_GET_B(stopColor)/255.0,
                                   NS_GET_A(stopColor)/255.0 *
                                     stopOpacity * aGraphicOpacity));
  }

  aContext->SetPattern(gradient);

  return PR_TRUE;
}

// Private (helper) methods

void
nsSVGGradientFrame::GetRefedGradientFromHref()
{
  mNextGrad = nsnull;
  mInitialized = PR_TRUE;

  // Fetch our gradient element's xlink:href attribute
  nsAutoString href;
  mHref->GetAnimVal(href);
  if (href.IsEmpty()) {
    return; // no URL
  }

  // Convert href to an nsIURI
  nsCOMPtr<nsIURI> targetURI;
  nsCOMPtr<nsIURI> base = mContent->GetBaseURI();
  nsContentUtils::NewURIWithDocumentCharset(getter_AddRefs(targetURI), href,
                                            mContent->GetCurrentDoc(), base);

  // Fetch and store a pointer to the referenced gradient element's frame.
  // Note that we are using *our* frame tree for this call, otherwise we're
  // going to have to get the PresShell in each call
  nsIFrame *nextGrad;
  if (NS_SUCCEEDED(nsSVGUtils::GetReferencedFrame(&nextGrad, targetURI, mContent,
                                                  PresContext()->PresShell()))) {
    nsIAtom* frameType = nextGrad->GetType();
    if (frameType != nsGkAtoms::svgLinearGradientFrame && 
        frameType != nsGkAtoms::svgRadialGradientFrame)
      return;

    mNextGrad = reinterpret_cast<nsSVGGradientFrame*>(nextGrad);

    // Add ourselves to the observer list
    if (mNextGrad) {
      // Can't use the NS_ADD macro here because of nsISupports ambiguity
      mNextGrad->AddObserver(this);
    }
  }
}

// This is implemented to return nsnull if the attribute is not set so that
// GetFx and GetFy can use the values of cx and cy instead of the defaults.
nsIContent*
nsSVGGradientFrame::GetGradientWithAttr(nsIAtom *aAttrName)
{
  if (mContent->HasAttr(kNameSpaceID_None, aAttrName))
    return mContent;

  if (!mInitialized)  // make sure mNextGrad has been initialized
    GetRefedGradientFromHref();

  if (!mNextGrad)
    return nsnull;

  nsIContent *grad = nsnull;

  // Set mLoopFlag before checking mNextGrad->mLoopFlag in case we are mNextGrad
  mLoopFlag = PR_TRUE;
  // XXXjwatt: we should really send an error to the JavaScript Console here:
  NS_WARN_IF_FALSE(!mNextGrad->mLoopFlag, "gradient reference loop detected "
                                          "while inheriting attribute!");
  if (!mNextGrad->mLoopFlag)
    grad = mNextGrad->GetGradientWithAttr(aAttrName);
  mLoopFlag = PR_FALSE;

  return grad;
}

nsIContent*
nsSVGGradientFrame::GetGradientWithAttr(nsIAtom *aAttrName, nsIAtom *aGradType)
{
  if (GetType() == aGradType && mContent->HasAttr(kNameSpaceID_None, aAttrName))
    return mContent;

  if (!mInitialized)
    GetRefedGradientFromHref();  // make sure mNextGrad has been initialized

  if (!mNextGrad)
    return nsnull;

  nsIContent *grad = nsnull;

  // Set mLoopFlag before checking mNextGrad->mLoopFlag in case we are mNextGrad
  mLoopFlag = PR_TRUE;
  // XXXjwatt: we should really send an error to the JavaScript Console here:
  NS_WARN_IF_FALSE(!mNextGrad->mLoopFlag, "gradient reference loop detected "
                                          "while inheriting attribute!");
  if (!mNextGrad->mLoopFlag)
    grad = mNextGrad->GetGradientWithAttr(aAttrName, aGradType);
  mLoopFlag = PR_FALSE;

  return grad;
}

PRInt32 
nsSVGGradientFrame::GetStopFrame(PRInt32 aIndex, nsIFrame * *aStopFrame)
{
  PRInt32 stopCount = 0;
  nsIFrame *stopFrame = nsnull;
  for (stopFrame = mFrames.FirstChild(); stopFrame;
       stopFrame = stopFrame->GetNextSibling()) {
    if (stopFrame->GetType() == nsGkAtoms::svgStopFrame) {
      // Is this the one we're looking for?
      if (stopCount++ == aIndex)
        break; // Yes, break out of the loop
    }
  }
  if (stopCount > 0) {
    if (aStopFrame)
      *aStopFrame = stopFrame;
    return stopCount;
  }

  // Our gradient element doesn't have stops - try to "inherit" them

  if (!mInitialized)
    GetRefedGradientFromHref();  // make sure mNextGrad has been initialized

  if (!mNextGrad) {
    if (aStopFrame)
      *aStopFrame = nsnull;
    return 0;
  }

  // Set mLoopFlag before checking mNextGrad->mLoopFlag in case we are mNextGrad
  mLoopFlag = PR_TRUE;
  // XXXjwatt: we should really send an error to the JavaScript Console here:
  NS_WARN_IF_FALSE(!mNextGrad->mLoopFlag, "gradient reference loop detected "
                                          "while inheriting stop!");
  if (!mNextGrad->mLoopFlag)
    stopCount = mNextGrad->GetStopFrame(aIndex, aStopFrame);
  mLoopFlag = PR_FALSE;

  return stopCount;
}

PRUint16
nsSVGGradientFrame::GetGradientUnits()
{
  // This getter is called every time the others are called - maybe cache it?

  nsIContent *gradient = GetGradientWithAttr(nsGkAtoms::gradientUnits);
  if (!gradient)
    gradient = mContent;  // use our gradient to get the correct default value

  nsSVGGradientElement *gradElement = static_cast<nsSVGGradientElement*>
                                                 (gradient);

  return gradElement->mEnumAttributes[nsSVGGradientElement::GRADIENTUNITS].GetAnimValue();
}

// -------------------------------------------------------------------------
// Linear Gradients
// -------------------------------------------------------------------------

nsIAtom*
nsSVGLinearGradientFrame::GetType() const
{
  return nsGkAtoms::svgLinearGradientFrame;
}

NS_IMETHODIMP
nsSVGLinearGradientFrame::AttributeChanged(PRInt32         aNameSpaceID,
                                           nsIAtom*        aAttribute,
                                           PRInt32         aModType)
{
  if (aNameSpaceID == kNameSpaceID_None &&
      (aAttribute == nsGkAtoms::x1 ||
       aAttribute == nsGkAtoms::y1 ||
       aAttribute == nsGkAtoms::x2 ||
       aAttribute == nsGkAtoms::y2)) {
    WillModify();
    DidModify();
    return NS_OK;
  }

  return nsSVGGradientFrame::AttributeChanged(aNameSpaceID,
                                              aAttribute, aModType);
}

//----------------------------------------------------------------------

float
nsSVGLinearGradientFrame::GradientLookupAttribute(nsIAtom *aAtomName,
                                                  PRUint16 aEnumName)
{
  nsIContent *gradient = GetLinearGradientWithAttr(aAtomName);
  if (!gradient)
    gradient = mContent;  // use our gradient to get the correct default value

  nsSVGLinearGradientElement *element =
    static_cast<nsSVGLinearGradientElement*>(gradient);

  // Object bounding box units are handled by setting the appropriate
  // transform in GetGradientTransform, but we need to handle user
  // space units as part of the individual Get* routines.  Fixes 323669.

  PRUint16 gradientUnits = GetGradientUnits();
  if (gradientUnits == nsIDOMSVGUnitTypes::SVG_UNIT_TYPE_USERSPACEONUSE) {
    return nsSVGUtils::UserSpace(mSourceContent,
                                 &element->mLengthAttributes[aEnumName]);
  }

  NS_ASSERTION(gradientUnits == nsIDOMSVGUnitTypes::SVG_UNIT_TYPE_OBJECTBOUNDINGBOX,
               "Unknown gradientUnits type");

  return element->mLengthAttributes[aEnumName].
    GetAnimValue(static_cast<nsSVGSVGElement*>(nsnull));
}

already_AddRefed<gfxPattern>
nsSVGLinearGradientFrame::CreateGradient()
{
  float x1, y1, x2, y2;

  x1 = GradientLookupAttribute(nsGkAtoms::x1, nsSVGLinearGradientElement::X1);
  y1 = GradientLookupAttribute(nsGkAtoms::y1, nsSVGLinearGradientElement::Y1);
  x2 = GradientLookupAttribute(nsGkAtoms::x2, nsSVGLinearGradientElement::X2);
  y2 = GradientLookupAttribute(nsGkAtoms::y2, nsSVGLinearGradientElement::Y2);

  gfxPattern *pattern = new gfxPattern(x1, y1, x2, y2);
  NS_IF_ADDREF(pattern);
  return pattern;
}

// -------------------------------------------------------------------------
// Radial Gradients
// -------------------------------------------------------------------------

nsIAtom*
nsSVGRadialGradientFrame::GetType() const
{
  return nsGkAtoms::svgRadialGradientFrame;
}

NS_IMETHODIMP
nsSVGRadialGradientFrame::AttributeChanged(PRInt32         aNameSpaceID,
                                           nsIAtom*        aAttribute,
                                           PRInt32         aModType)
{
  if (aNameSpaceID == kNameSpaceID_None &&
      (aAttribute == nsGkAtoms::r ||
       aAttribute == nsGkAtoms::cx ||
       aAttribute == nsGkAtoms::cy ||
       aAttribute == nsGkAtoms::fx ||
       aAttribute == nsGkAtoms::fy)) {
    WillModify();
    DidModify();
    return NS_OK;
  }

  return nsSVGGradientFrame::AttributeChanged(aNameSpaceID,
                                              aAttribute, aModType);
}

//----------------------------------------------------------------------

float
nsSVGRadialGradientFrame::GradientLookupAttribute(nsIAtom *aAtomName,
                                                  PRUint16 aEnumName,
                                                  nsIContent *aElement)
{
  nsIContent *gradient;

  if (aElement) {
    gradient = aElement;
  } else {
    gradient = GetRadialGradientWithAttr(aAtomName);
    if (!gradient)
      gradient = mContent;  // use our gradient to get the correct default value
  }

  nsSVGRadialGradientElement *element =
    static_cast<nsSVGRadialGradientElement*>(gradient);

  // Object bounding box units are handled by setting the appropriate
  // transform in GetGradientTransform, but we need to handle user
  // space units as part of the individual Get* routines.  Fixes 323669.

  PRUint16 gradientUnits = GetGradientUnits();
  if (gradientUnits == nsIDOMSVGUnitTypes::SVG_UNIT_TYPE_USERSPACEONUSE) {
    return nsSVGUtils::UserSpace(mSourceContent,
                                 &element->mLengthAttributes[aEnumName]);
  }

  NS_ASSERTION(gradientUnits == nsIDOMSVGUnitTypes::SVG_UNIT_TYPE_OBJECTBOUNDINGBOX,
               "Unknown gradientUnits type");

  return element->mLengthAttributes[aEnumName].
    GetAnimValue(static_cast<nsSVGSVGElement*>(nsnull));
}

already_AddRefed<gfxPattern>
nsSVGRadialGradientFrame::CreateGradient()
{
  float cx, cy, r, fx, fy;

  cx = GradientLookupAttribute(nsGkAtoms::cx, nsSVGRadialGradientElement::CX);
  cy = GradientLookupAttribute(nsGkAtoms::cy, nsSVGRadialGradientElement::CY);
  r  = GradientLookupAttribute(nsGkAtoms::r,  nsSVGRadialGradientElement::R);

  nsIContent *gradient;

  if (!(gradient = GetRadialGradientWithAttr(nsGkAtoms::fx)))
    fx = cx;  // if fx isn't set, we must use cx
  else
    fx = GradientLookupAttribute(nsGkAtoms::fx, nsSVGRadialGradientElement::FX, gradient);

  if (!(gradient = GetRadialGradientWithAttr(nsGkAtoms::fy)))
    fy = cy;  // if fy isn't set, we must use cy
  else
    fy = GradientLookupAttribute(nsGkAtoms::fy, nsSVGRadialGradientElement::FY, gradient);

  if (fx != cx || fy != cy) {
    // The focal point (fFx and fFy) must be clamped to be *inside* - not on -
    // the circumference of the gradient or we'll get rendering anomalies. We
    // calculate the distance from the focal point to the gradient center and
    // make sure it is *less* than the gradient radius. 0.999 is used as the
    // factor of the radius because it's close enough to 1 that we won't get a
    // fringe at the edge of the gradient if we clamp, but not so close to 1
    // that rounding error will give us the same results as using fR itself.
    double dMax = 0.999 * r;
    float dx = fx - cx;
    float dy = fy - cy;
    double d = sqrt((dx * dx) + (dy * dy));
    if (d > dMax) {
      double angle = atan2(dy, dx);
      fx = (float)(dMax * cos(angle)) + cx;
      fy = (float)(dMax * sin(angle)) + cy;
    }
  }

  gfxPattern *pattern = new gfxPattern(fx, fy, 0, cx, cy, r);
  NS_IF_ADDREF(pattern);
  return pattern;
}

// -------------------------------------------------------------------------
// Public functions
// -------------------------------------------------------------------------

nsIFrame* 
NS_NewSVGLinearGradientFrame(nsIPresShell*   aPresShell,
                             nsIContent*     aContent,
                             nsStyleContext* aContext)
{
  nsCOMPtr<nsIDOMSVGLinearGradientElement> grad = do_QueryInterface(aContent);
  if (!grad) {
    NS_ERROR("Can't create frame! Content is not an SVG linearGradient");
    return nsnull;
  }
  
  nsCOMPtr<nsIDOMSVGURIReference> aRef = do_QueryInterface(aContent);
  NS_ASSERTION(aRef, "NS_NewSVGLinearGradientFrame -- Content doesn't support nsIDOMSVGURIReference");

  return new (aPresShell) nsSVGLinearGradientFrame(aContext, aRef);
}

nsIFrame*
NS_NewSVGRadialGradientFrame(nsIPresShell*   aPresShell,
                             nsIContent*     aContent,
                             nsStyleContext* aContext)
{
  nsCOMPtr<nsIDOMSVGRadialGradientElement> grad = do_QueryInterface(aContent);
  if (!grad) {
    NS_ERROR("Can't create frame! Content is not an SVG radialGradient");
    return nsnull;
  }
  
  nsCOMPtr<nsIDOMSVGURIReference> aRef = do_QueryInterface(aContent);
  NS_ASSERTION(aRef, "NS_NewSVGRadialGradientFrame -- Content doesn't support nsIDOMSVGURIReference");

  return new (aPresShell) nsSVGRadialGradientFrame(aContext, aRef);
}
