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

#ifndef __NS_SVGPATHGEOMETRYFRAME_H__
#define __NS_SVGPATHGEOMETRYFRAME_H__

#include "nsFrame.h"
#include "nsISVGChildFrame.h"
#include "nsWeakReference.h"
#include "nsGkAtoms.h"
#include "nsSVGGeometryFrame.h"
#include "gfxRect.h"

class nsPresContext;
class nsIDOMSVGMatrix;
class nsSVGMarkerFrame;
class nsSVGMarkerProperty;

typedef nsSVGGeometryFrame nsSVGPathGeometryFrameBase;

#define HITTEST_MASK_FILL        0x01
#define HITTEST_MASK_STROKE      0x02
#define HITTEST_MASK_FORCE_TEST  0x04

class nsSVGPathGeometryFrame : public nsSVGPathGeometryFrameBase,
                               public nsISVGChildFrame
{
  friend nsIFrame*
  NS_NewSVGPathGeometryFrame(nsIPresShell* aPresShell, nsIContent* aContent,
                             nsStyleContext* aContext);
protected:
  nsSVGPathGeometryFrame(nsStyleContext* aContext) :
    nsSVGPathGeometryFrameBase(aContext),
    mPropagateTransform(PR_TRUE) {}

public:
  // nsISupports interface:
  NS_IMETHOD QueryInterface(const nsIID& aIID, void** aInstancePtr);
private:
  NS_IMETHOD_(nsrefcnt) AddRef() { return 1; }
  NS_IMETHOD_(nsrefcnt) Release() { return 1; }

public:
  // nsIFrame interface:
  virtual void Destroy();
  NS_IMETHOD  AttributeChanged(PRInt32         aNameSpaceID,
                               nsIAtom*        aAttribute,
                               PRInt32         aModType);

  NS_IMETHOD DidSetStyleContext();

  /**
   * Get the "type" of the frame
   *
   * @see nsGkAtoms::svgPathGeometryFrame
   */
  virtual nsIAtom* GetType() const;

#ifdef DEBUG
  NS_IMETHOD GetFrameName(nsAString& aResult) const
  {
    return MakeFrameName(NS_LITERAL_STRING("SVGPathGeometry"), aResult);
  }
#endif

  // nsISVGGeometrySource interface:
  NS_IMETHOD GetCanvasTM(nsIDOMSVGMatrix * *aCTM);
  virtual nsresult UpdateGraphic(PRBool suppressInvalidation = PR_FALSE);

protected:
  // nsISVGChildFrame interface:
  NS_IMETHOD PaintSVG(nsSVGRenderState *aContext, nsRect *aDirtyRect);
  NS_IMETHOD GetFrameForPointSVG(float x, float y, nsIFrame** hit);
  NS_IMETHOD_(nsRect) GetCoveredRegion();
  NS_IMETHOD UpdateCoveredRegion();
  NS_IMETHOD InitialUpdate();
  virtual void NotifySVGChanged(PRUint32 aFlags);
  NS_IMETHOD NotifyRedrawSuspended();
  NS_IMETHOD NotifyRedrawUnsuspended();
  NS_IMETHOD SetMatrixPropagation(PRBool aPropagate);
  NS_IMETHOD SetOverrideCTM(nsIDOMSVGMatrix *aCTM);
  virtual already_AddRefed<nsIDOMSVGMatrix> GetOverrideCTM();
  NS_IMETHOD GetBBox(nsIDOMSVGRect **_retval);
  NS_IMETHOD_(PRBool) IsDisplayContainer() { return PR_FALSE; }
  NS_IMETHOD_(PRBool) HasValidCoveredRect() { return PR_TRUE; }

protected:
  virtual PRUint16 GetHittestMask();

private:
  void Render(nsSVGRenderState *aContext);
  void GeneratePath(gfxContext *aContext);

  /*
   * Check for what cairo returns for the fill extents of a degenerate path
   *
   * @return PR_TRUE if the path is degenerate
   */
  static PRBool
  IsDegeneratePath(const gfxRect& rect)
  {
    return (rect.X() == 0 && rect.Y() == 0 &&
            rect.Width() == 0 && rect.Height() == 0);
  }

  nsSVGMarkerProperty *GetMarkerProperty();
  void UpdateMarkerProperty();

  void RemovePathProperties();

  nsCOMPtr<nsIDOMSVGMatrix> mOverrideCTM;
  PRPackedBool mPropagateTransform;
};

#endif // __NS_SVGPATHGEOMETRYFRAME_H__
