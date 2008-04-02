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
#include "nsGkAtoms.h"
#include "nsSVGUtils.h"
#include "nsSVGClipPathElement.h"
#include "gfxContext.h"
#include "nsIDOMSVGRect.h"

//----------------------------------------------------------------------
// Implementation

nsIFrame*
NS_NewSVGClipPathFrame(nsIPresShell* aPresShell, nsIContent* aContent, nsStyleContext* aContext)
{
  nsCOMPtr<nsIDOMSVGClipPathElement> clipPath = do_QueryInterface(aContent);
  if (!clipPath) {
    NS_ERROR("Can't create frame! Content is not an SVG clipPath!");
    return nsnull;
  }

  return new (aPresShell) nsSVGClipPathFrame(aContext);
}

nsIContent *
NS_GetSVGClipPathElement(nsIURI *aURI, nsIContent *aContent)
{
  nsIContent* content = nsContentUtils::GetReferencedElement(aURI, aContent);

  nsCOMPtr<nsIDOMSVGClipPathElement> clipPath = do_QueryInterface(content);

  if (clipPath)
    return content;

  return nsnull;
}

nsresult
nsSVGClipPathFrame::ClipPaint(nsSVGRenderState* aContext,
                              nsISVGChildFrame* aParent,
                              nsIDOMSVGMatrix *aMatrix)
{
  // If the flag is set when we get here, it means this clipPath frame
  // has already been used painting the current clip, and the document
  // has a clip reference loop.
  if (mInUse) {
    NS_WARNING("Clip loop detected!");
    return NS_OK;
  }
  AutoClipPathReferencer clipRef(this);

  mClipParent = aParent,
  mClipParentMatrix = aMatrix;

  PRBool isTrivial = IsTrivial();

  nsAutoSVGRenderMode mode(aContext,
                           isTrivial ? nsSVGRenderState::CLIP
                                     : nsSVGRenderState::CLIP_MASK);

  for (nsIFrame* kid = mFrames.FirstChild(); kid;
       kid = kid->GetNextSibling()) {
    nsISVGChildFrame* SVGFrame = nsnull;
    CallQueryInterface(kid, &SVGFrame);
    if (SVGFrame) {
      // The CTM of each frame referencing us can be different.
      SVGFrame->NotifySVGChanged(nsISVGChildFrame::SUPPRESS_INVALIDATION | 
                                 nsISVGChildFrame::TRANSFORM_CHANGED);
      SVGFrame->PaintSVG(aContext, nsnull);
    }
  }

  if (isTrivial) {
    aContext->GetGfxContext()->Clip();
    aContext->GetGfxContext()->NewPath();
  }

  return NS_OK;
}

PRBool
nsSVGClipPathFrame::ClipHitTest(nsISVGChildFrame* aParent,
                                nsIDOMSVGMatrix *aMatrix,
                                float aX, float aY)
{
  // If the flag is set when we get here, it means this clipPath frame
  // has already been used in hit testing against the current clip,
  // and the document has a clip reference loop.
  if (mInUse) {
    NS_WARNING("Clip loop detected!");
    return PR_FALSE;
  }
  AutoClipPathReferencer clipRef(this);

  nsRect dirty;
  mClipParent = aParent,
  mClipParentMatrix = aMatrix;

  for (nsIFrame* kid = mFrames.FirstChild(); kid;
       kid = kid->GetNextSibling()) {
    nsISVGChildFrame* SVGFrame = nsnull;
    CallQueryInterface(kid, &SVGFrame);
    if (SVGFrame) {
      // Notify the child frame that we may be working with a
      // different transform, so it can update its covered region
      // (used to shortcut hit testing).
      SVGFrame->NotifySVGChanged(nsISVGChildFrame::TRANSFORM_CHANGED);

      nsIFrame *temp = nsnull;
      nsresult rv = SVGFrame->GetFrameForPointSVG(aX, aY, &temp);
      if (NS_SUCCEEDED(rv) && temp)
        return PR_TRUE;
    }
  }
  return PR_FALSE;
}

PRBool
nsSVGClipPathFrame::IsTrivial()
{
  PRBool foundChild = PR_FALSE;

  for (nsIFrame* kid = mFrames.FirstChild(); kid;
       kid = kid->GetNextSibling()) {
    nsISVGChildFrame *svgChild = nsnull;
    CallQueryInterface(kid, &svgChild);

    if (svgChild) {
      // We consider a non-trivial clipPath to be one containing
      // either more than one svg child and/or a svg container
      if (foundChild || svgChild->IsDisplayContainer())
        return PR_FALSE;
      foundChild = PR_TRUE;
    }
  }
  return PR_TRUE;
}

nsIAtom *
nsSVGClipPathFrame::GetType() const
{
  return nsGkAtoms::svgClipPathFrame;
}

already_AddRefed<nsIDOMSVGMatrix>
nsSVGClipPathFrame::GetCanvasTM()
{
  NS_ASSERTION(mClipParentMatrix, "null parent matrix");

  nsSVGClipPathElement *clipPath = static_cast<nsSVGClipPathElement*>
                                              (mContent);

  nsCOMPtr<nsIDOMSVGMatrix> localTM = clipPath->GetLocalTransformMatrix();

  nsCOMPtr<nsIDOMSVGMatrix> canvasTM;

  if (localTM)
    mClipParentMatrix->Multiply(localTM, getter_AddRefs(canvasTM));
  else
    canvasTM = mClipParentMatrix;

  return nsSVGUtils::AdjustMatrixForUnits(canvasTM,
                                          &clipPath->mEnumAttributes[nsSVGClipPathElement::CLIPPATHUNITS],
                                          mClipParent);
}
