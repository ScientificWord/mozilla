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
 * The Initial Developer of the Original Code is IBM Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2004
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
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

#include "nsIDOMDocument.h"
#include "nsIDocument.h"
#include "nsIDOMSVGClipPathElement.h"
#include "nsSVGClipPathFrame.h"
#include "nsISVGRendererCanvas.h"
#include "nsIDOMSVGAnimatedEnum.h"
#include "nsISVGRendererSurface.h"
#include "nsSVGContainerFrame.h"
#include "nsSVGAtoms.h"
#include "nsSVGUtils.h"
#include "nsSVGGraphicElement.h"

typedef nsSVGContainerFrame nsSVGClipPathFrameBase;

class nsSVGClipPathFrame : public nsSVGClipPathFrameBase,
                           public nsISVGClipPathFrame
{
  friend nsIFrame*
  NS_NewSVGClipPathFrame(nsIPresShell* aPresShell, nsIContent* aContent, nsStyleContext* aContext);

  virtual ~nsSVGClipPathFrame();
  NS_IMETHOD InitSVG();

 public:
  nsSVGClipPathFrame(nsStyleContext* aContext) : nsSVGClipPathFrameBase(aContext) {}

  // nsISupports interface:
  NS_IMETHOD QueryInterface(const nsIID& aIID, void** aInstancePtr);
  NS_IMETHOD_(nsrefcnt) AddRef() { return NS_OK; }
  NS_IMETHOD_(nsrefcnt) Release() { return NS_OK; }

  // nsISVGClipPathFrame interface:
  NS_IMETHOD ClipPaint(nsISVGRendererCanvas* canvas,
                       nsISVGRendererSurface* aClipSurface,
                       nsISVGChildFrame* aParent,
                       nsCOMPtr<nsIDOMSVGMatrix> aMatrix);

  NS_IMETHOD ClipHitTest(nsISVGChildFrame* aParent,
                         nsCOMPtr<nsIDOMSVGMatrix> aMatrix,
                         float aX, float aY, PRBool *aHit);

  NS_IMETHOD IsTrivial(PRBool *aTrivial);

  /**
   * Get the "type" of the frame
   *
   * @see nsLayoutAtoms::svgClipPathFrame
   */
  virtual nsIAtom* GetType() const;

#ifdef DEBUG
  NS_IMETHOD GetFrameName(nsAString& aResult) const
  {
    return MakeFrameName(NS_LITERAL_STRING("SVGClipPath"), aResult);
  }
#endif

 private:
  nsISVGChildFrame *mClipParent;
  nsCOMPtr<nsIDOMSVGMatrix> mClipParentMatrix;

  // nsSVGContainerFrame methods:
  virtual already_AddRefed<nsIDOMSVGMatrix> GetCanvasTM();

  // recursion prevention flag
  PRPackedBool mInUse;
};

NS_INTERFACE_MAP_BEGIN(nsSVGClipPathFrame)
  NS_INTERFACE_MAP_ENTRY(nsISVGClipPathFrame)
NS_INTERFACE_MAP_END_INHERITING(nsSVGClipPathFrameBase)

//----------------------------------------------------------------------
// Implementation

nsIFrame*
NS_NewSVGClipPathFrame(nsIPresShell* aPresShell, nsIContent* aContent, nsStyleContext* aContext)
{
  nsCOMPtr<nsIDOMSVGTransformable> transformable = do_QueryInterface(aContent);
  if (!transformable) {
#ifdef DEBUG
    printf("warning: trying to construct an SVGClipPathFrame for a content element that doesn't support the right interfaces\n");
#endif
    return nsnull;
  }

  return new (aPresShell) nsSVGClipPathFrame(aContext);
}

nsresult
NS_GetSVGClipPathFrame(nsISVGClipPathFrame **aResult,
                       nsIURI *aURI, nsIContent *aContent)
{
  *aResult = nsnull;

  // Get the PresShell
  nsIDocument *myDoc = aContent->GetCurrentDoc();
  if (!myDoc) {
    NS_WARNING("No document for this content!");
    return NS_ERROR_FAILURE;
  }
  nsIPresShell *presShell = myDoc->GetShellAt(0);
  if (!presShell) {
    NS_WARNING("no presshell");
    return NS_ERROR_FAILURE;
  }

  // Find the referenced frame
  nsIFrame *cpframe;
  if (!NS_SUCCEEDED(nsSVGUtils::GetReferencedFrame(&cpframe, aURI, aContent, presShell)))
    return NS_ERROR_FAILURE;

  nsIAtom* frameType = cpframe->GetType();
  if (frameType != nsLayoutAtoms::svgClipPathFrame)
    return NS_ERROR_FAILURE;

  *aResult = (nsSVGClipPathFrame *)cpframe;
  return NS_OK;
}

nsSVGClipPathFrame::~nsSVGClipPathFrame()
{
}

NS_IMETHODIMP
nsSVGClipPathFrame::InitSVG()
{
  nsresult rv = nsSVGClipPathFrameBase::InitSVG();
  if (NS_FAILED(rv))
    return rv;

  mClipParentMatrix = NULL;

  return NS_OK;
}

NS_IMETHODIMP
nsSVGClipPathFrame::ClipPaint(nsISVGRendererCanvas* canvas,
                              nsISVGRendererSurface* aClipSurface,
                              nsISVGChildFrame* aParent,
                              nsCOMPtr<nsIDOMSVGMatrix> aMatrix)
{
  // If the flag is set when we get here, it means this clipPath frame
  // has already been used painting the current clip, and the document
  // has a clip reference loop.
  if (mInUse) {
    NS_WARNING("Clip loop detected!");
    return NS_OK;
  }
  mInUse = PR_TRUE;

  nsRect dirty;
  nsresult rv;

  mClipParent = aParent,
  mClipParentMatrix = aMatrix;

  PRBool isTrivial;
  IsTrivial(&isTrivial);

  if (isTrivial)
    rv = canvas->SetRenderMode(nsISVGRendererCanvas::SVG_RENDER_MODE_CLIP);
  else {
    rv = canvas->SetRenderMode(nsISVGRendererCanvas::SVG_RENDER_MODE_CLIP_MASK);

    canvas->PushSurface(aClipSurface, PR_TRUE);
  }

  if (NS_FAILED(rv))
    return NS_ERROR_FAILURE;

  for (nsIFrame* kid = mFrames.FirstChild(); kid;
       kid = kid->GetNextSibling()) {
    nsISVGChildFrame* SVGFrame = nsnull;
    CallQueryInterface(kid, &SVGFrame);
    if (SVGFrame) {
      SVGFrame->NotifyCanvasTMChanged(PR_TRUE);
      SVGFrame->PaintSVG(canvas, nsnull);
    }
  }

  if (!isTrivial)
    canvas->PopSurface();

  canvas->SetRenderMode(nsISVGRendererCanvas::SVG_RENDER_MODE_NORMAL);

  mInUse = PR_FALSE;

  return NS_OK;
}

NS_IMETHODIMP
nsSVGClipPathFrame::ClipHitTest(nsISVGChildFrame* aParent,
                                nsCOMPtr<nsIDOMSVGMatrix> aMatrix,
                                float aX, float aY, PRBool *aHit)
{
  *aHit = PR_FALSE;

  // If the flag is set when we get here, it means this clipPath frame
  // has already been used in hit testing against the current clip,
  // and the document has a clip reference loop.
  if (mInUse) {
    NS_WARNING("Clip loop detected!");
    return NS_OK;
  }
  mInUse = PR_TRUE;

  nsRect dirty;
  mClipParent = aParent,
  mClipParentMatrix = aMatrix;

  for (nsIFrame* kid = mFrames.FirstChild(); kid;
       kid = kid->GetNextSibling()) {
    nsISVGChildFrame* SVGFrame=nsnull;
    kid->QueryInterface(NS_GET_IID(nsISVGChildFrame),(void**)&SVGFrame);
    if (SVGFrame) {
      // Notify the child frame that we may be working with a
      // different transform, so it can update its covered region
      // (used to shortcut hit testing).
      SVGFrame->NotifyCanvasTMChanged(PR_FALSE);

      nsIFrame *temp = nsnull;
      nsresult rv = SVGFrame->GetFrameForPointSVG(aX, aY, &temp);
      if (NS_SUCCEEDED(rv) && temp) {
        *aHit = PR_TRUE;
        mInUse = PR_FALSE;
        return NS_OK;
      }
    }
  }

  mInUse = PR_FALSE;

  return NS_OK;
}

NS_IMETHODIMP
nsSVGClipPathFrame::IsTrivial(PRBool *aTrivial)
{
  *aTrivial = PR_TRUE;
  PRBool foundChild = PR_FALSE;

  for (nsIFrame* kid = mFrames.FirstChild(); kid;
       kid = kid->GetNextSibling()) {
    nsISVGChildFrame *svgChild = nsnull;
    CallQueryInterface(kid, &svgChild);

    if (svgChild) {
      // We consider a non-trivial clipPath to be one containing
      // either more than one svg child and/or a svg container
      if (foundChild || svgChild->IsDisplayContainer()) {
        *aTrivial = PR_FALSE;
        return NS_OK;
      }
      foundChild = PR_TRUE;
    }
  }

  return NS_OK;
}

nsIAtom *
nsSVGClipPathFrame::GetType() const
{
  return nsLayoutAtoms::svgClipPathFrame;
}

already_AddRefed<nsIDOMSVGMatrix>
nsSVGClipPathFrame::GetCanvasTM()
{
  NS_ASSERTION(mClipParentMatrix, "null parent matrix");

  nsSVGGraphicElement *element =
    NS_STATIC_CAST(nsSVGGraphicElement*, mContent);
  nsCOMPtr<nsIDOMSVGMatrix> localTM = element->GetLocalTransformMatrix();

  nsCOMPtr<nsIDOMSVGMatrix> canvasTM;

  if (localTM)
    mClipParentMatrix->Multiply(localTM, getter_AddRefs(canvasTM));
  else
    canvasTM = mClipParentMatrix;

  /* object bounding box? */
  PRUint16 units;
  nsCOMPtr<nsIDOMSVGClipPathElement> path = do_QueryInterface(mContent);
  nsCOMPtr<nsIDOMSVGAnimatedEnumeration> aEnum;
  path->GetClipPathUnits(getter_AddRefs(aEnum));
  aEnum->GetAnimVal(&units);
  
  if (mClipParent &&
      units == nsIDOMSVGClipPathElement::SVG_CPUNITS_OBJECTBOUNDINGBOX) {
    nsCOMPtr<nsIDOMSVGRect> rect;
    nsresult rv = mClipParent->GetBBox(getter_AddRefs(rect));

    if (NS_SUCCEEDED(rv)) {
      float minx, miny, width, height;
      rect->GetX(&minx);
      rect->GetY(&miny);
      rect->GetWidth(&width);
      rect->GetHeight(&height);

      nsCOMPtr<nsIDOMSVGMatrix> tmp, fini;
      canvasTM->Translate(minx, miny, getter_AddRefs(tmp));
      tmp->ScaleNonUniform(width, height, getter_AddRefs(fini));
      canvasTM = fini;
    }
  }

  nsIDOMSVGMatrix* retval = canvasTM.get();
  NS_IF_ADDREF(retval);
  return retval;
}
