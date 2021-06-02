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
 * The Original Code is Mozilla MathML Project.
 *
 * The Initial Developer of the Original Code is
 * The University Of Queensland.
 * Portions created by the Initial Developer are Copyright (C) 1999
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Roger B. Sidje <rbs@maths.uq.edu.au>
 *   David J. Fiddes <D.J.Fiddes@hw.ac.uk>
 *   Shyjan Mahamud <mahamud@cs.cmu.edu> (added TeX rendering rules)
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

#include "nsCOMPtr.h"
#include "nsFrame.h"
#include "nsPresContext.h"
#include "nsStyleContext.h"
#include "nsStyleConsts.h"
#include "nsIRenderingContext.h"
#include "nsIFontMetrics.h"
#include "nsMathCursorUtils.h"
#include "nsTextFrame.h"

#include "nsMathMLmsupFrame.h"

//
// <msup> -- attach a superscript to a base - implementation
//

nsIFrame*
NS_NewMathMLmsupFrame(nsIPresShell* aPresShell, nsStyleContext* aContext)
{
  return new (aPresShell) nsMathMLmsupFrame(aContext);
}

nsMathMLmsupFrame::~nsMathMLmsupFrame()
{
}

NS_IMETHODIMP
nsMathMLmsupFrame::TransmitAutomaticData()
{
  // if our base is an embellished operator, its flags bubble to us
  mPresentationData.baseFrame = mFrames.FirstChild();
  GetEmbellishDataFrom(mPresentationData.baseFrame, mEmbellishData);

  // 1. The REC says:
  // The <msup> element increments scriptlevel by 1, and sets displaystyle to
  // "false", within superscript, but leaves both attributes unchanged within base.
  // 2. The TeXbook (Ch 17. p.141) says the superscript *inherits* the compression,
  // so we don't set the compression flag. Our parent will propagate its own.
  UpdatePresentationDataFromChildAt(1, -1,
    ~NS_MATHML_DISPLAYSTYLE,
     NS_MATHML_DISPLAYSTYLE);

  return NS_OK;
}

/* virtual */ nsresult
nsMathMLmsupFrame::Place(nsIRenderingContext& aRenderingContext,
                         PRBool               aPlaceOrigin,
                         nsHTMLReflowMetrics& aDesiredSize)
{
  // extra spacing after sup/subscript
  nscoord scriptSpace = PresContext()->PointsToAppUnits(0.5f); // 0.5pt as in plain TeX

  // check if the superscriptshift attribute is there
  nsAutoString value;
  nscoord supScriptShift = 0;
  GetAttribute(mContent, mPresentationData.mstyle,
               nsGkAtoms::superscriptshift_, value);
  if (!value.IsEmpty()) {
    nsCSSValue cssValue;
    if (ParseNumericValue(value, cssValue) && cssValue.IsLengthUnit()) {
      supScriptShift = CalcLength(PresContext(), mStyleContext, cssValue);
    }
  }

  return nsMathMLmsupFrame::PlaceSuperScript(PresContext(),
                                             aRenderingContext,
                                             aPlaceOrigin,
                                             aDesiredSize,
                                             this,
                                             supScriptShift,
                                             scriptSpace);
}

// exported routine that both mover and msup share.
// mover uses this when movablelimits is set.
nsresult
nsMathMLmsupFrame::PlaceSuperScript(nsPresContext*      aPresContext,
                                    nsIRenderingContext& aRenderingContext,
                                    PRBool               aPlaceOrigin,
                                    nsHTMLReflowMetrics& aDesiredSize,
                                    nsMathMLContainerFrame* aFrame,
                                    nscoord              aUserSupScriptShift,
                                    nscoord              aScriptSpace)
{
  // force the scriptSpace to be at least 1 pixel
  nscoord onePixel = nsPresContext::CSSPixelsToAppUnits(1);
  aScriptSpace = PR_MAX(onePixel, aScriptSpace);

  ////////////////////////////////////
  // Get the children's desired sizes

  nsHTMLReflowMetrics baseSize;
  nsHTMLReflowMetrics supScriptSize;
  nsBoundingMetrics bmBase, bmSupScript;
  nsIFrame* supScriptFrame = nsnull;
  nsIFrame* baseFrame = aFrame->GetFirstChild(nsnull);
  if (baseFrame)
    supScriptFrame = baseFrame->GetNextSibling();
  if (!baseFrame || !supScriptFrame || supScriptFrame->GetNextSibling()) {
    // report an error, encourage people to get their markups in order
    NS_WARNING("invalid markup");
    return static_cast<nsMathMLContainerFrame*>(aFrame)->ReflowError(aRenderingContext,
                                               aDesiredSize);
  }
  GetReflowAndBoundingMetricsFor(baseFrame, baseSize, bmBase);
  GetReflowAndBoundingMetricsFor(supScriptFrame, supScriptSize, bmSupScript);

  // get the supdrop from the supscript font
  nscoord supDrop;
  GetSupDropFromChild(supScriptFrame, supDrop);
  // parameter u, Rule 18a, App. G, TeXbook
  nscoord minSupScriptShift = bmBase.ascent - supDrop;

  //////////////////
  // Place Children

  // get min supscript shift limit from x-height
  // = d(x) + 1/4 * sigma_5, Rule 18c, App. G, TeXbook
  nscoord xHeight = 0;
  nsCOMPtr<nsIFontMetrics> fm =
    aPresContext->GetMetricsFor(baseFrame->GetStyleFont()->mFont);

  fm->GetXHeight (xHeight);
  nscoord minShiftFromXHeight = (nscoord)
    (bmSupScript.descent + (1.0f/4.0f) * xHeight);
  nscoord italicCorrection;
  GetItalicCorrection(bmBase, italicCorrection);

  // supScriptShift{1,2,3}
  // = minimum amount to shift the supscript up
  // = sup{1,2,3} in TeX
  // supScriptShift1 = superscriptshift attribute * x-height
  // Note that there are THREE values for supscript shifts depending
  // on the current style
  nscoord supScriptShift1, supScriptShift2, supScriptShift3;
  // Set supScriptShift{1,2,3} default from font
  GetSupScriptShifts (fm, supScriptShift1, supScriptShift2, supScriptShift3);

  if (0 < aUserSupScriptShift) {
    // the user has set the superscriptshift attribute
    float scaler2 = ((float) supScriptShift2) / supScriptShift1;
    float scaler3 = ((float) supScriptShift3) / supScriptShift1;
    supScriptShift1 =
      PR_MAX(supScriptShift1, aUserSupScriptShift);
    supScriptShift2 = NSToCoordRound(scaler2 * supScriptShift1);
    supScriptShift3 = NSToCoordRound(scaler3 * supScriptShift1);
  }

  // get sup script shift depending on current script level and display style
  // Rule 18c, App. G, TeXbook
  nscoord supScriptShift;
  nsPresentationData presentationData;
  aFrame->GetPresentationData (presentationData);
  if ( aFrame->GetStyleFont()->mScriptLevel == 0 &&
       NS_MATHML_IS_DISPLAYSTYLE(presentationData.flags) &&
      !NS_MATHML_IS_COMPRESSED(presentationData.flags)) {
    // Style D in TeXbook
    supScriptShift = supScriptShift1;
  }
  else if (NS_MATHML_IS_COMPRESSED(presentationData.flags)) {
    // Style C' in TeXbook = D',T',S',SS'
    supScriptShift = supScriptShift3;
  }
  else {
    // everything else = T,S,SS
    supScriptShift = supScriptShift2;
  }

  // get actual supscriptshift to be used
  // Rule 18c, App. G, TeXbook
  nscoord actualSupScriptShift =
    PR_MAX(minSupScriptShift,PR_MAX(supScriptShift,minShiftFromXHeight));

  // bounding box
  nsBoundingMetrics boundingMetrics;
  boundingMetrics.ascent =
    PR_MAX(bmBase.ascent, (bmSupScript.ascent + actualSupScriptShift));
  boundingMetrics.descent =
    PR_MAX(bmBase.descent, (bmSupScript.descent - actualSupScriptShift));

  // leave aScriptSpace after superscript
  // add italicCorrection between base and superscript
  // add "a little to spare" as well (see TeXbook Ch.11, p.64), as we
  // estimate the italic creation ourselves and it isn't the same as TeX
  italicCorrection += onePixel;
  boundingMetrics.width = bmBase.width + italicCorrection +
                          bmSupScript.width + aScriptSpace;
  boundingMetrics.leftBearing = bmBase.leftBearing;
  boundingMetrics.rightBearing = bmBase.width + italicCorrection +
                                 bmSupScript.rightBearing;
  aFrame->SetBoundingMetrics(boundingMetrics);

  // reflow metrics
  aDesiredSize.ascent =
    PR_MAX(baseSize.ascent, (supScriptSize.ascent + actualSupScriptShift));
  aDesiredSize.height = aDesiredSize.ascent +
    PR_MAX(baseSize.height - baseSize.ascent,
           (supScriptSize.height - supScriptSize.ascent - actualSupScriptShift));
  aDesiredSize.width = boundingMetrics.width;
  aDesiredSize.mBoundingMetrics = boundingMetrics;

  aFrame->SetReference(nsPoint(0, aDesiredSize.ascent));

  if (aPlaceOrigin) {
    nscoord dx, dy;
    // now place the base ...
    dx = 0; dy = aDesiredSize.ascent - baseSize.ascent;
    FinishReflowChild (baseFrame, aPresContext, nsnull, baseSize, dx, dy, 0);
    // ... and supscript
    dx = bmBase.width + italicCorrection;
    dy = aDesiredSize.ascent - (supScriptSize.ascent + actualSupScriptShift);
    FinishReflowChild (supScriptFrame, aPresContext, nsnull, supScriptSize, dx, dy, 0);
  }

  return NS_OK;
}


nsresult
nsMathMLmsupFrame::EnterFromLeft(nsIFrame *leavingFrame, nsIFrame** aOutFrame, PRInt32* aOutOffset, PRInt32 count, PRBool* fBailing,
    PRInt32 *_retval)
{
#ifdef debug_barry
  printf("msup EnterFromLeft, count = %d\n", count);
#endif  
  nsIFrame * pFrame = GetFirstChild(nsnull);
  nsCOMPtr<nsIMathMLCursorMover> pMCM;
  if (pFrame)
  {
    pMCM = GetMathCursorMover(pFrame);
    if (pMCM) pMCM->EnterFromLeft(nsnull, aOutFrame, aOutOffset, count, fBailing,  _retval);
    else // child frame is not a math frame. Probably a text frame. We'll assume this for now
    {
      PlaceCursorBefore(pFrame, PR_TRUE, aOutFrame, aOutOffset, count);
      *_retval = 0;
      return NS_OK;
    }
  }
#ifdef debug_barry  
  else
  {
    printf("Found msup frame with no children\n");
  }
#endif  
  return NS_OK;
}

nsresult
nsMathMLmsupFrame::EnterFromRight(nsIFrame *leavingFrame, nsIFrame** aOutFrame, PRInt32* aOutOffset, PRInt32 count,
    PRBool* fBailingOut, PRInt32 *_retval)
{
#ifdef debug_barry
  printf("msup EnterFromRight, count = %d\n", count);
#endif  
  if (count > 0)
  {
    nsIFrame * pFrame = GetFirstChild(nsnull); // the base
    pFrame = pFrame->GetNextSibling();
    nsCOMPtr<nsIMathMLCursorMover> pMCM;
    if (pFrame)
    {
      pMCM = GetMathCursorMover(pFrame);
      count--;
      *_retval = count;
      if (pMCM) pMCM->EnterFromRight(nsnull, aOutFrame, aOutOffset, count, fBailingOut, _retval);
      else // child frame is not a math frame. Probably a text frame. We'll assume this for now
      {
        PlaceCursorAfter(pFrame, PR_TRUE, aOutFrame, aOutOffset, count);
        *_retval = 0;
        return NS_OK;
      }
    }
#ifdef debug_barry    
    else
    {
      printf("Found msup frame with no superscript\n");
    }
#endif    
  }
#ifdef debug_barry  
  else
  {
    printf("msub EnterFromRight called with count == 0\n");
    PlaceCursorAfter(this, PR_FALSE, aOutFrame, aOutOffset, count);
  }
#endif  
  return NS_OK;
}


nsresult
nsMathMLmsupFrame::MoveOutToRight(nsIFrame * leavingFrame, nsIFrame** aOutFrame, PRInt32* aOutOffset, PRInt32 count,
    PRBool* fBailingOut, PRInt32 *_retval)
{
#ifdef debug_barry  
  printf("msup MoveOutToRight, count = %d\n", count);
#endif  
  nsIFrame * pChild = GetFirstChild(nsnull);
  nsIFrame * pParent;
  nsCOMPtr<nsIMathMLCursorMover> pMCM;
  if (leavingFrame != pChild)
  {
	  // leavingFrame is the superscript. We are leaving the structure. Put the cursor just after the msup
    count = 0;
    *aOutFrame = this->GetParent();
    *aOutOffset = 1 + mmlFrameGetIndexInParent(this, *aOutFrame);
    *_retval = 0; 
  }
  else
  {
    // leaving base
    count= 0;
    pChild = pChild->GetNextSibling();
    pMCM = GetMathCursorMover(pChild);
    if (pMCM) pMCM->EnterFromLeft(this, aOutFrame, aOutOffset, count, fBailingOut, _retval);
#ifdef debug_barry    
    else printf("msup MoveOutToRight: no superscript\n");
#endif    
    *_retval = 0;
  }
  return NS_OK;
}

nsresult
nsMathMLmsupFrame::MoveOutToLeft(nsIFrame * leavingFrame, nsIFrame** aOutFrame, PRInt32* aOutOffset, PRInt32 count,
    PRBool* fBailingOut, PRInt32 *_retval)
{
#ifdef debug_barry
  printf("msup MoveOutToLeft, count = %d\n", count);
#endif
  // if the cursor is leaving either of its children, the cursor goes past the end of the fraction if count > 0
  nsIFrame * pChild = GetFirstChild(nsnull);
  nsCOMPtr<nsIMathMLCursorMover> pMCM;
  if (leavingFrame == nsnull || leavingFrame == pChild)
  {
    nsIFrame * pParent = GetParent();
    pMCM = GetMathCursorMover(pParent);
    if (pMCM) pMCM->MoveOutToLeft(this, aOutFrame, aOutOffset, count, fBailingOut, _retval);
    else  // parent isn't math??? shouldn't happen
    {
      *_retval = count;
      *aOutFrame = nsnull;  // should allow default Mozilla code to take over
      return NS_OK;
    }
  }
  else
  {
    // leaving superscript. Place the cursor just after the base.
    count= 0;
    *aOutFrame = this;
    *aOutOffset = 1;
//    PlaceCursorAfter(pChild, PR_TRUE, amsOutFrame, aOutOffset, count);
    //pMCM = GetMathCursorMover(pChild);
    //if (pMCM) pMCM->EnterFromRight(nsnull, aOutFrame, aOutOffset, count, fBailingOut, _retval);
    *_retval = 0;
  }
  return NS_OK;
}
