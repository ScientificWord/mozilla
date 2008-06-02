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
 *   Vilya Harvey <vilya@nag.co.uk>
 *   Shyjan Mahamud <mahamud@cs.cmu.edu>
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

#include "nsMathMLmrootFrame.h"
#include "nsMathCursorUtils.h"

//
// <msqrt> and <mroot> -- form a radical - implementation
//

//NOTE:
//  The code assumes that TeX fonts are picked.
//  There is no fall-back to draw the branches of the sqrt explicitly
//  in the case where TeX fonts are not there. In general, there are no
//  fall-back(s) in MathML when some (freely-downloadable) fonts are missing.
//  Otherwise, this will add much work and unnecessary complexity to the core
//  MathML  engine. Assuming that authors have the free fonts is part of the
//  deal. We are not responsible for cases of misconfigurations out there.

// additional style context to be used by our MathMLChar.
#define NS_SQR_CHAR_STYLE_CONTEXT_INDEX   0

static const PRUnichar kSqrChar = PRUnichar(0x221A);

nsIFrame*
NS_NewMathMLmrootFrame(nsIPresShell* aPresShell, nsStyleContext* aContext)
{
  return new (aPresShell) nsMathMLmrootFrame(aContext);
}

nsMathMLmrootFrame::nsMathMLmrootFrame(nsStyleContext* aContext) :
  nsMathMLContainerFrame(aContext),
  mSqrChar(),
  mBarRect()
{
}

nsMathMLmrootFrame::~nsMathMLmrootFrame()
{
}

NS_IMETHODIMP
nsMathMLmrootFrame::Init(nsIContent*      aContent,
                         nsIFrame*        aParent,
                         nsIFrame*        aPrevInFlow)
{
  nsresult rv = nsMathMLContainerFrame::Init(aContent, aParent, aPrevInFlow);
  
  nsPresContext *presContext = PresContext();

  // No need to tract the style context given to our MathML char. 
  // The Style System will use Get/SetAdditionalStyleContext() to keep it
  // up-to-date if dynamic changes arise.
  nsAutoString sqrChar; sqrChar.Assign(kSqrChar);
  mSqrChar.SetData(presContext, sqrChar);
  ResolveMathMLCharStyle(presContext, mContent, mStyleContext, &mSqrChar, PR_TRUE);

  return rv;
}

NS_IMETHODIMP
nsMathMLmrootFrame::TransmitAutomaticData()
{
  // 1. The REC says:
  //    The <mroot> element increments scriptlevel by 2, and sets displaystyle to
  //    "false", within index, but leaves both attributes unchanged within base.
  // 2. The TeXbook (Ch 17. p.141) says \sqrt is compressed
  UpdatePresentationDataFromChildAt(1, 1,
    ~NS_MATHML_DISPLAYSTYLE | NS_MATHML_COMPRESSED,
     NS_MATHML_DISPLAYSTYLE | NS_MATHML_COMPRESSED);
  UpdatePresentationDataFromChildAt(0, 0,
     NS_MATHML_COMPRESSED, NS_MATHML_COMPRESSED);

  return NS_OK;
}

NS_IMETHODIMP
nsMathMLmrootFrame::BuildDisplayList(nsDisplayListBuilder*   aBuilder,
                                     const nsRect&           aDirtyRect,
                                     const nsDisplayListSet& aLists)
{
  /////////////
  DisplaySelectionOverlay(aBuilder, aLists,  nsISelectionDisplay::DISPLAY_ALL);
  // paint the content we are square-rooting
  nsresult rv = nsMathMLContainerFrame::BuildDisplayList(aBuilder, aDirtyRect, aLists);
  NS_ENSURE_SUCCESS(rv, rv);
  
  /////////////
  // paint the sqrt symbol
  if (!NS_MATHML_HAS_ERROR(mPresentationData.flags)) {
    rv = mSqrChar.Display(aBuilder, this, aLists);
    NS_ENSURE_SUCCESS(rv, rv);

    rv = DisplayBar(aBuilder, this, mBarRect, aLists);
    NS_ENSURE_SUCCESS(rv, rv);

#if defined(NS_DEBUG) && defined(SHOW_BOUNDING_BOX)
    // for visual debug
    nsRect rect;
    mSqrChar.GetRect(rect);
    nsBoundingMetrics bm;
    mSqrChar.GetBoundingMetrics(bm);
    rv = DisplayBoundingMetrics(aBuilder, this, rect.TopLeft(), bm, aLists);
#endif
  }

  return rv;
}

NS_IMETHODIMP
nsMathMLmrootFrame::Reflow(nsPresContext*          aPresContext,
                           nsHTMLReflowMetrics&     aDesiredSize,
                           const nsHTMLReflowState& aReflowState,
                           nsReflowStatus&          aStatus)
{
  nsresult rv = NS_OK;
  nsSize availSize(aReflowState.ComputedWidth(), NS_UNCONSTRAINEDSIZE);
  nsReflowStatus childStatus;

  aDesiredSize.width = aDesiredSize.height = 0;
  aDesiredSize.ascent = 0;

  nsBoundingMetrics bmSqr, bmBase, bmIndex;
  nsIRenderingContext& renderingContext = *aReflowState.rendContext;

  //////////////////
  // Reflow Children

  PRInt32 count = 0;
  nsIFrame* baseFrame = nsnull;
  nsIFrame* indexFrame = nsnull;
  nsHTMLReflowMetrics baseSize;
  nsHTMLReflowMetrics indexSize;
  nsIFrame* childFrame = mFrames.FirstChild();
  while (childFrame) {
    // ask our children to compute their bounding metrics 
    nsHTMLReflowMetrics childDesiredSize(aDesiredSize.mFlags
                                         | NS_REFLOW_CALC_BOUNDING_METRICS);
    nsHTMLReflowState childReflowState(aPresContext, aReflowState,
                                       childFrame, availSize);
    rv = ReflowChild(childFrame, aPresContext,
                     childDesiredSize, childReflowState, childStatus);
    //NS_ASSERTION(NS_FRAME_IS_COMPLETE(childStatus), "bad status");
    if (NS_FAILED(rv)) {
      // Call DidReflow() for the child frames we successfully did reflow.
      DidReflowChildren(mFrames.FirstChild(), childFrame);
      return rv;
    }
    if (0 == count) {
      // base 
      baseFrame = childFrame;
      baseSize = childDesiredSize;
      bmBase = childDesiredSize.mBoundingMetrics;
    }
    else if (1 == count) {
      // index
      indexFrame = childFrame;
      indexSize = childDesiredSize;
      bmIndex = childDesiredSize.mBoundingMetrics;
    }
    count++;
    childFrame = childFrame->GetNextSibling();
  }
  if (2 != count) {
    // report an error, encourage people to get their markups in order
    NS_WARNING("invalid markup");
    rv = ReflowError(renderingContext, aDesiredSize);
    aStatus = NS_FRAME_COMPLETE;
    NS_FRAME_SET_TRUNCATION(aStatus, aReflowState, aDesiredSize);
    return rv;
  }

  ////////////
  // Prepare the radical symbol and the overline bar

  renderingContext.SetFont(GetStyleFont()->mFont, nsnull);
  nsCOMPtr<nsIFontMetrics> fm;
  renderingContext.GetFontMetrics(*getter_AddRefs(fm));

  // For radical glyphs from TeX fonts and some of the radical glyphs from
  // Mathematica fonts, the thickness of the overline can be obtained from the
  // ascent of the glyph.  Most fonts however have radical glyphs above the
  // baseline so no assumption can be made about the meaning of the ascent.
  nscoord ruleThickness, leading, em;
  GetRuleThickness(renderingContext, fm, ruleThickness);

  nsBoundingMetrics bmOne;
  renderingContext.GetBoundingMetrics(NS_LITERAL_STRING("1").get(), 1, bmOne);

  // get the leading to be left at the top of the resulting frame
  // this seems more reliable than using fm->GetLeading() on suspicious fonts
  GetEmHeight(fm, em);
  leading = nscoord(0.2f * em); 

  // Rule 11, App. G, TeXbook
  // psi = clearance between rule and content
  nscoord phi = 0, psi = 0;
  if (NS_MATHML_IS_DISPLAYSTYLE(mPresentationData.flags))
    fm->GetXHeight(phi);
  else
    phi = ruleThickness;
  psi = ruleThickness + phi/4;

  // built-in: adjust clearance psi to emulate \mathstrut using '1' (TexBook, p.131)
  if (bmOne.ascent > bmBase.ascent)
    psi += bmOne.ascent - bmBase.ascent;

  // make sure that the rule appears on on screen
  nscoord onePixel = nsPresContext::CSSPixelsToAppUnits(1);
  if (ruleThickness < onePixel) {
    ruleThickness = onePixel;
  }

  // adjust clearance psi to get an exact number of pixels -- this
  // gives a nicer & uniform look on stacked radicals (bug 130282)
  nscoord delta = psi % onePixel;
  if (delta)
    psi += onePixel - delta; // round up

  // Stretch the radical symbol to the appropriate height if it is not big enough.
  nsBoundingMetrics contSize = bmBase;
  contSize.descent = bmBase.ascent + bmBase.descent + psi;
  contSize.ascent = ruleThickness;

  // height(radical) should be >= height(base) + psi + ruleThickness
  nsBoundingMetrics radicalSize;
  mSqrChar.Stretch(aPresContext, renderingContext,
                   NS_STRETCH_DIRECTION_VERTICAL, 
                   contSize, radicalSize,
                   NS_STRETCH_LARGER);
  // radicalSize have changed at this point, and should match with
  // the bounding metrics of the char
  mSqrChar.GetBoundingMetrics(bmSqr);

  // Update the desired size for the container (like msqrt, index is not yet included)
  // the baseline will be that of the base.
  mBoundingMetrics.ascent = bmBase.ascent + psi + ruleThickness;
  mBoundingMetrics.descent = 
    PR_MAX(bmBase.descent,
           (bmSqr.ascent + bmSqr.descent - mBoundingMetrics.ascent));
  mBoundingMetrics.width = bmSqr.width + bmBase.width;
  mBoundingMetrics.leftBearing = bmSqr.leftBearing;
  mBoundingMetrics.rightBearing = bmSqr.width + 
    PR_MAX(bmBase.width, bmBase.rightBearing); // take also care of the rule

  aDesiredSize.ascent = mBoundingMetrics.ascent + leading;
  aDesiredSize.height = aDesiredSize.ascent +
    PR_MAX(baseSize.height - baseSize.ascent,
           mBoundingMetrics.descent + ruleThickness);
  aDesiredSize.width = mBoundingMetrics.width;

  /////////////
  // Re-adjust the desired size to include the index.
  
  // the index is raised by some fraction of the height
  // of the radical, see \mroot macro in App. B, TexBook
  nscoord raiseIndexDelta = NSToCoordRound(0.6f * (bmSqr.ascent + bmSqr.descent));
  nscoord indexRaisedAscent = mBoundingMetrics.ascent // top of radical 
    - (bmSqr.ascent + bmSqr.descent) // to bottom of radical
    + raiseIndexDelta + bmIndex.ascent + bmIndex.descent; // to top of raised index

  nscoord indexClearance = 0;
  if (mBoundingMetrics.ascent < indexRaisedAscent) {
    indexClearance = 
      indexRaisedAscent - mBoundingMetrics.ascent; // excess gap introduced by a tall index 
    mBoundingMetrics.ascent = indexRaisedAscent;
    nscoord descent = aDesiredSize.height - aDesiredSize.ascent;
    aDesiredSize.ascent = mBoundingMetrics.ascent + leading;
    aDesiredSize.height = aDesiredSize.ascent + descent;
  }

  // the index is tucked in closer to the radical while making sure
  // that the kern does not make the index and radical collide
  nscoord dxIndex, dxSqr, dx, dy;
  nscoord xHeight = 0;
  fm->GetXHeight(xHeight);
  nscoord indexRadicalKern = NSToCoordRound(1.35f * xHeight);
  if (indexRadicalKern > bmIndex.width) {
    dxIndex = indexRadicalKern - bmIndex.width;
    dxSqr = 0;
  }
  else {
    dxIndex = 0;
    dxSqr = bmIndex.width - indexRadicalKern;
  }
  // avoid collision by leaving a minimun space between index and radical
  nscoord minimumClearance = bmSqr.width/2;
  if (dxIndex + bmIndex.width + minimumClearance > dxSqr + bmSqr.width) {
    if (bmIndex.width + minimumClearance < bmSqr.width) {
      dxIndex = bmSqr.width - (bmIndex.width + minimumClearance);
      dxSqr = 0;
    }
    else {
      dxIndex = 0;
      dxSqr = (bmIndex.width + minimumClearance) - bmSqr.width;
    }
  }

  // place the index
  dx = dxIndex;
  dy = aDesiredSize.ascent - (indexRaisedAscent + indexSize.ascent - bmIndex.ascent);
  FinishReflowChild(indexFrame, aPresContext, nsnull, indexSize, dx, dy, 0);

  // place the radical symbol and the radical bar
  dx = dxSqr;
  dy = indexClearance + leading; // leave a leading at the top
  mSqrChar.SetRect(nsRect(dx, dy, bmSqr.width, bmSqr.ascent + bmSqr.descent));
  dx += bmSqr.width;
  mBarRect.SetRect(dx, dy, bmBase.width, ruleThickness);

  // place the base
  dy = aDesiredSize.ascent - baseSize.ascent;
  FinishReflowChild(baseFrame, aPresContext, nsnull, baseSize, dx, dy, 0);

  mReference.x = 0;
  mReference.y = aDesiredSize.ascent;

  mBoundingMetrics.width = dx + bmBase.width;
  mBoundingMetrics.leftBearing = 
    PR_MIN(dxIndex + bmIndex.leftBearing, dxSqr + bmSqr.leftBearing);
  mBoundingMetrics.rightBearing = dx +
    PR_MAX(bmBase.width, bmBase.rightBearing);

  aDesiredSize.width = mBoundingMetrics.width;
  aDesiredSize.mBoundingMetrics = mBoundingMetrics;
  GatherAndStoreOverflow(&aDesiredSize);

  aStatus = NS_FRAME_COMPLETE;
  NS_FRAME_SET_TRUNCATION(aStatus, aReflowState, aDesiredSize);
  return NS_OK;
}


// ----------------------
// the Style System will use these to pass the proper style context to our MathMLChar
nsStyleContext*
nsMathMLmrootFrame::GetAdditionalStyleContext(PRInt32 aIndex) const
{
  switch (aIndex) {
  case NS_SQR_CHAR_STYLE_CONTEXT_INDEX:
    return mSqrChar.GetStyleContext();
    break;
  default:
    return nsnull;
  }
}

void
nsMathMLmrootFrame::SetAdditionalStyleContext(PRInt32          aIndex, 
                                              nsStyleContext*  aStyleContext)
{
  switch (aIndex) {
  case NS_SQR_CHAR_STYLE_CONTEXT_INDEX:
    mSqrChar.SetStyleContext(aStyleContext);
    break;
  }
}

nsresult
nsMathMLmrootFrame::MoveOutToRight(nsIFrame * leavingFrame, nsIFrame** aOutFrame, PRInt32* aOutOffset, PRUint32& count)
{
  printf("mroot MoveOutToRight, count = %d\n", count);
  nsIFrame * pBaseFrame = GetFirstChild(nsnull);
  if (!pBaseFrame)
  {
    printf("Malformed mroot -- no children\n");
    return NS_OK; //BBM fix this!!
  }
  nsIFrame * pIndexFrame = pBaseFrame->GetNextSibling();
  if (!pIndexFrame)
  {
    printf("Malformed mroot -- no index\n");
    return NS_OK; //BBM fix this!!
  }
  nsCOMPtr<nsMathMLFrame> pMathMLFrame;
  if (count > 0) count--;
  if (leavingFrame == nsnull)  EnterFromLeft(aOutFrame, aOutOffset, count);// entering mroot; we want the cursor to go to the index first
  else if (pIndexFrame && pIndexFrame == leavingFrame) // we are leaving the index; go into the base
  {
    pMathMLFrame = do_QueryInterface(pBaseFrame);
    if (pMathMLFrame) pMathMLFrame->EnterFromLeft(aOutFrame, aOutOffset, count);
    else printf("Malformed mroot -- base not math\n");
  }
  else
  {
    nsIFrame * pParent;
    // leaving the base position
    pParent = GetParent();
    pMathMLFrame = do_QueryInterface(pParent);
    if (pMathMLFrame) 
    {
      if (count > 0) pMathMLFrame->MoveOutToRight(this, aOutFrame, aOutOffset, count);
      else PlaceCursorAfter(this, PR_FALSE, aOutFrame, aOutOffset, count);
    }
    else printf("mroot not a child of mathml???\n");
  }  
  return NS_OK;  
}


nsresult
nsMathMLmrootFrame::MoveOutToLeft(nsIFrame * leavingFrame, nsIFrame** aOutFrame, PRInt32* aOutOffset, PRUint32& count)
{
  printf("mroot MoveOutToLeft, count = %d\n", count);
  return NS_OK;
}


nsresult
nsMathMLmrootFrame::EnterFromLeft(nsIFrame** aOutFrame, PRInt32* aOutOffset, PRUint32& count)
{
  printf("mroot EnterFromLeft, count = %d\n", count);
  nsIFrame * pBaseFrame = GetFirstChild(nsnull);
  if (!pBaseFrame)
  {
    printf("Malformed mroot -- no children\n");
    return NS_OK; //BBM fix this!!
  }
  nsIFrame * pIndexFrame = pBaseFrame->GetNextSibling();
  if (!pIndexFrame)
  {
    printf("Malformed mroot -- no index\n");
    return NS_OK; //BBM fix this!!
  }
  nsCOMPtr<nsMathMLFrame> pMathMLFrame;
  pMathMLFrame = do_QueryInterface(pIndexFrame);
  count = 0;
  if (pMathMLFrame) pMathMLFrame->EnterFromLeft(aOutFrame, aOutOffset, count);
  else printf("Malformed root -- index not math\n");
  return NS_OK;
}


nsresult
nsMathMLmrootFrame::EnterFromRight(nsIFrame** aOutFrame, PRInt32* aOutOffset, PRUint32& count)
{
  printf("mroot EnterFromRight, count = %d\n", count);
  return NS_OK;
}
