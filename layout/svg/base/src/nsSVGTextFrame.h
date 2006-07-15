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

#ifndef NS_SVGTEXTFRAME_H
#define NS_SVGTEXTFRAME_H

#include "nsSVGTextContainerFrame.h"
#include "nsISVGValueObserver.h"
#include "nsWeakReference.h"

typedef nsSVGTextContainerFrame nsSVGTextFrameBase;

class nsSVGTextFrame : public nsSVGTextFrameBase,
                       public nsISVGValueObserver,
                       public nsSupportsWeakReference
{
  friend nsIFrame*
  NS_NewSVGTextFrame(nsIPresShell* aPresShell, nsIContent* aContent, nsStyleContext* aContext);
protected:
  nsSVGTextFrame(nsStyleContext* aContext);

   // nsISupports interface:
  NS_IMETHOD QueryInterface(const nsIID& aIID, void** aInstancePtr);
private:
  NS_IMETHOD_(nsrefcnt) AddRef() { return NS_OK; }
  NS_IMETHOD_(nsrefcnt) Release() { return NS_OK; }  

public:
  // nsIFrame:
  NS_IMETHOD  RemoveFrame(nsIAtom*        aListName,
                          nsIFrame*       aOldFrame);
  
  NS_IMETHOD  AttributeChanged(PRInt32         aNameSpaceID,
                               nsIAtom*        aAttribute,
                               PRInt32         aModType);

  NS_IMETHOD DidSetStyleContext();

  /**
   * Get the "type" of the frame
   *
   * @see nsLayoutAtoms::svgTextFrame
   */
  virtual nsIAtom* GetType() const;

#ifdef DEBUG
  NS_IMETHOD GetFrameName(nsAString& aResult) const
  {
    return MakeFrameName(NS_LITERAL_STRING("SVGText"), aResult);
  }
#endif

  // nsISVGValueObserver
  NS_IMETHOD WillModifySVGObservable(nsISVGValue* observable,
                                     nsISVGValue::modificationType aModType);
  NS_IMETHOD DidModifySVGObservable (nsISVGValue* observable,
                                     nsISVGValue::modificationType aModType);

  // nsISupportsWeakReference
  // implementation inherited from nsSupportsWeakReference
  
  // nsISVGChildFrame interface:
  NS_IMETHOD SetMatrixPropagation(PRBool aPropagate);
  NS_IMETHOD SetOverrideCTM(nsIDOMSVGMatrix *aCTM);
  NS_IMETHOD NotifyCanvasTMChanged(PRBool suppressInvalidation);
  NS_IMETHOD NotifyRedrawSuspended();
  NS_IMETHOD NotifyRedrawUnsuspended();
  NS_IMETHOD GetBBox(nsIDOMSVGRect **_retval);
  
  // nsSVGContainerFrame methods:
  virtual already_AddRefed<nsIDOMSVGMatrix> GetCanvasTM();
  
  // nsISVGTextContentMetrics
  NS_IMETHOD GetNumberOfChars(PRInt32 *_retval);
  NS_IMETHOD GetComputedTextLength(float *_retval);
  NS_IMETHOD GetSubStringLength(PRUint32 charnum, PRUint32 nchars, float *_retval);
  NS_IMETHOD GetStartPositionOfChar(PRUint32 charnum, nsIDOMSVGPoint **_retval);
  NS_IMETHOD GetEndPositionOfChar(PRUint32 charnum, nsIDOMSVGPoint **_retval);
  NS_IMETHOD GetExtentOfChar(PRUint32 charnum, nsIDOMSVGRect **_retval);
  NS_IMETHOD GetRotationOfChar(PRUint32 charnum, float *_retval);
  NS_IMETHOD GetCharNumAtPosition(nsIDOMSVGPoint *point, PRInt32 *_retval);

  void NotifyGlyphMetricsChange(nsISVGGlyphFragmentNode* caller);
  void NotifyGlyphFragmentTreeChange(nsISVGGlyphFragmentNode* caller);
  PRBool IsMetricsSuspended();
  PRBool IsGlyphFragmentTreeSuspended();

private:
  void EnsureFragmentTreeUpToDate();
  void UpdateFragmentTree();
  void UpdateGlyphPositioning();

  nsCOMPtr<nsIDOMSVGMatrix> mCanvasTM;
  nsCOMPtr<nsIDOMSVGMatrix> mOverrideCTM;

  enum UpdateState{
    unsuspended,
    suspended,
    updating};
  UpdateState mFragmentTreeState;
  UpdateState mMetricsState;

  PRPackedBool mFragmentTreeDirty;
  PRPackedBool mPropagateTransform;
  PRPackedBool mPositioningDirty;
};

#endif
