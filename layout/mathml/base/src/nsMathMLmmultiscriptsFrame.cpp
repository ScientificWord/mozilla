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

#include "nsMathMLmmultiscriptsFrame.h"

//
// <mmultiscripts> -- attach prescripts and tensor indices to a base - implementation
//

nsIFrame*
NS_NewMathMLmmultiscriptsFrame(nsIPresShell* aPresShell, nsStyleContext* aContext)
{
  return new (aPresShell) nsMathMLmmultiscriptsFrame(aContext);
}

nsMathMLmmultiscriptsFrame::~nsMathMLmmultiscriptsFrame()
{
}

NS_IMETHODIMP
nsMathMLmmultiscriptsFrame::TransmitAutomaticData()
{
  // if our base is an embellished operator, let its state bubble to us
  mPresentationData.baseFrame = mFrames.FirstChild();
  GetEmbellishDataFrom(mPresentationData.baseFrame, mEmbellishData);

  // The REC says:
  // The <mmultiscripts> element increments scriptlevel by 1, and sets
  // displaystyle to "false", within each of its arguments except base
  UpdatePresentationDataFromChildAt(1, -1,
    ~NS_MATHML_DISPLAYSTYLE, NS_MATHML_DISPLAYSTYLE);

  // The TeXbook (Ch 17. p.141) says the superscript inherits the compression
  // while the subscript is compressed. So here we collect subscripts and set
  // the compression flag in them.
  PRInt32 count = 0;
  PRBool isSubScript = PR_FALSE;
  nsAutoVoidArray subScriptFrames;
  nsIFrame* childFrame = mFrames.FirstChild();
  while (childFrame) {
    if (childFrame->GetContent()->Tag() == nsGkAtoms::mprescripts_) {
      // mprescripts frame
    }
    else if (0 == count) {
      // base frame
    }
    else {
      // super/subscript block
      if (isSubScript) {
        // subscript
        subScriptFrames.AppendElement(childFrame);
      }
      else {
        // superscript
      }
      isSubScript = !isSubScript;
    }
    count++;
    childFrame = childFrame->GetNextSibling();
  }
  for (PRInt32 i = subScriptFrames.Count() - 1; i >= 0; i--) {
    childFrame = (nsIFrame*)subScriptFrames[i];
    PropagatePresentationDataFor(childFrame,
      NS_MATHML_COMPRESSED, NS_MATHML_COMPRESSED);
  }

  return NS_OK;
}

void
nsMathMLmmultiscriptsFrame::ProcessAttributes()
{
  mSubScriptShift = 0;
  mSupScriptShift = 0;

  // check if the subscriptshift attribute is there
  nsAutoString value;
  GetAttribute(mContent, mPresentationData.mstyle,
               nsGkAtoms::subscriptshift_, value);
  if (!value.IsEmpty()) {
    nsCSSValue cssValue;
    if (ParseNumericValue(value, cssValue) && cssValue.IsLengthUnit()) {
      mSubScriptShift = CalcLength(PresContext(), mStyleContext, cssValue);
    }
  }
  // check if the superscriptshift attribute is there
  GetAttribute(mContent, mPresentationData.mstyle,
               nsGkAtoms::superscriptshift_, value);
  if (!value.IsEmpty()) {
    nsCSSValue cssValue;
    if (ParseNumericValue(value, cssValue) && cssValue.IsLengthUnit()) {
      mSupScriptShift = CalcLength(PresContext(), mStyleContext, cssValue);
    }
  }
}

/* virtual */ nsresult
nsMathMLmmultiscriptsFrame::Place(nsIRenderingContext& aRenderingContext,
                                  PRBool               aPlaceOrigin,
                                  nsHTMLReflowMetrics& aDesiredSize)
{
  ////////////////////////////////////
  // Get the children's desired sizes

  nscoord minShiftFromXHeight, subDrop, supDrop;

  ////////////////////////////////////////
  // Initialize super/sub shifts that
  // depend only on the current font
  ////////////////////////////////////////

  ProcessAttributes();

  // get x-height (an ex)
  const nsStyleFont* font = GetStyleFont();
  aRenderingContext.SetFont(font->mFont, nsnull);
  nsCOMPtr<nsIFontMetrics> fm;
  aRenderingContext.GetFontMetrics(*getter_AddRefs(fm));

  nscoord xHeight;
  fm->GetXHeight (xHeight);

  nscoord ruleSize;
  GetRuleThickness (aRenderingContext, fm, ruleSize);

  // scriptspace from TeX for extra spacing after sup/subscript (0.5pt in plain TeX)
  // forced to be at least 1 pixel here
  nscoord onePixel = nsPresContext::CSSPixelsToAppUnits(1);
  nscoord scriptSpace = PR_MAX(PresContext()->PointsToAppUnits(0.5f), onePixel);

  /////////////////////////////////////
  // first the shift for the subscript

  // subScriptShift{1,2}
  // = minimum amount to shift the subscript down
  // = sub{1,2} in TeXbook
  // subScriptShift1 = subscriptshift attribute * x-height
  nscoord subScriptShift1, subScriptShift2;

  // Get subScriptShift{1,2} default from font
  GetSubScriptShifts (fm, subScriptShift1, subScriptShift2);
  if (0 < mSubScriptShift) {
    // the user has set the subscriptshift attribute
    float scaler = ((float) subScriptShift2) / subScriptShift1;
    subScriptShift1 = PR_MAX(subScriptShift1, mSubScriptShift);
    subScriptShift2 = NSToCoordRound(scaler * subScriptShift1);
  }
  // the font dependent shift
  nscoord subScriptShift = PR_MAX(subScriptShift1,subScriptShift2);

  /////////////////////////////////////
  // next the shift for the superscript

  // supScriptShift{1,2,3}
  // = minimum amount to shift the supscript up
  // = sup{1,2,3} in TeX
  // supScriptShift1 = superscriptshift attribute * x-height
  // Note that there are THREE values for supscript shifts depending
  // on the current style
  nscoord supScriptShift1, supScriptShift2, supScriptShift3;
  // Set supScriptShift{1,2,3} default from font
  GetSupScriptShifts (fm, supScriptShift1, supScriptShift2, supScriptShift3);
  if (0 < mSupScriptShift) {
    // the user has set the superscriptshift attribute
    float scaler2 = ((float) supScriptShift2) / supScriptShift1;
    float scaler3 = ((float) supScriptShift3) / supScriptShift1;
    supScriptShift1 = PR_MAX(supScriptShift1, mSupScriptShift);
    supScriptShift2 = NSToCoordRound(scaler2 * supScriptShift1);
    supScriptShift3 = NSToCoordRound(scaler3 * supScriptShift1);
  }

  // get sup script shift depending on current script level and display style
  // Rule 18c, App. G, TeXbook
  nscoord supScriptShift;
  if ( font->mScriptLevel == 0 &&
       NS_MATHML_IS_DISPLAYSTYLE(mPresentationData.flags) &&
      !NS_MATHML_IS_COMPRESSED(mPresentationData.flags)) {
    // Style D in TeXbook
    supScriptShift = supScriptShift1;
  }
  else if (NS_MATHML_IS_COMPRESSED(mPresentationData.flags)) {
    // Style C' in TeXbook = D',T',S',SS'
    supScriptShift = supScriptShift3;
  }
  else {
    // everything else = T,S,SS
    supScriptShift = supScriptShift2;
  }

  ////////////////////////////////////
  // Get the children's sizes
  ////////////////////////////////////

  nscoord width = 0, prescriptsWidth = 0, rightBearing = 0;
  nsIFrame* mprescriptsFrame = nsnull; // frame of <mprescripts/>, if there.
  PRBool isSubScript = PR_FALSE;
  nscoord minSubScriptShift = 0, minSupScriptShift = 0;
  nscoord trySubScriptShift = subScriptShift;
  nscoord trySupScriptShift = supScriptShift;
  nscoord maxSubScriptShift = subScriptShift;
  nscoord maxSupScriptShift = supScriptShift;
  PRInt32 count = 0;
  nsHTMLReflowMetrics baseSize;
  nsHTMLReflowMetrics subScriptSize;
  nsHTMLReflowMetrics supScriptSize;
  nsIFrame* baseFrame = nsnull;
  nsIFrame* subScriptFrame = nsnull;
  nsIFrame* supScriptFrame = nsnull;

  PRBool firstPrescriptsPair = PR_FALSE;
  nsBoundingMetrics bmBase, bmSubScript, bmSupScript;
  nscoord italicCorrection = 0;

  mBoundingMetrics.width = 0;
  mBoundingMetrics.ascent = mBoundingMetrics.descent = -0x7FFFFFFF;
  nscoord ascent = -0x7FFFFFFF, descent = -0x7FFFFFFF;
  aDesiredSize.width = aDesiredSize.height = 0;

  nsIFrame* childFrame = mFrames.FirstChild();
  while (childFrame) {
    if (childFrame->GetContent()->Tag() == nsGkAtoms::mprescripts_) {
      if (mprescriptsFrame) {
        // duplicate <mprescripts/> found
        // report an error, encourage people to get their markups in order
        NS_WARNING("invalid markup");
        return ReflowError(aRenderingContext, aDesiredSize);
      }
      mprescriptsFrame = childFrame;
      firstPrescriptsPair = PR_TRUE;
    }
    else {
      if (0 == count) {
        // base
        baseFrame = childFrame;
        GetReflowAndBoundingMetricsFor(baseFrame, baseSize, bmBase);
        GetItalicCorrection(bmBase, italicCorrection);
        // for the superscript, we always add "a little to spare"
        italicCorrection += onePixel;

        // we update mBoundingMetrics.{ascent,descent} with that
        // of the baseFrame only after processing all the sup/sub pairs
        // XXX need italic correction only *if* there are postscripts ?
        mBoundingMetrics.width = bmBase.width + italicCorrection;
        mBoundingMetrics.rightBearing = bmBase.rightBearing;
        mBoundingMetrics.leftBearing = bmBase.leftBearing; // until overwritten
      }
      else {
        // super/subscript block
        if (isSubScript) {
          // subscript
          subScriptFrame = childFrame;
          GetReflowAndBoundingMetricsFor(subScriptFrame, subScriptSize, bmSubScript);
          // get the subdrop from the subscript font
          GetSubDropFromChild (subScriptFrame, subDrop);
          // parameter v, Rule 18a, App. G, TeXbook
          minSubScriptShift = bmBase.descent + subDrop;
          trySubScriptShift = PR_MAX(minSubScriptShift,subScriptShift);
          mBoundingMetrics.descent =
            PR_MAX(mBoundingMetrics.descent,bmSubScript.descent);
          descent = PR_MAX(descent,subScriptSize.height - subScriptSize.ascent);
          width = bmSubScript.width + scriptSpace;
          rightBearing = bmSubScript.rightBearing;
        }
        else {
          // supscript
          supScriptFrame = childFrame;
          GetReflowAndBoundingMetricsFor(supScriptFrame, supScriptSize, bmSupScript);
          // get the supdrop from the supscript font
          GetSupDropFromChild (supScriptFrame, supDrop);
          // parameter u, Rule 18a, App. G, TeXbook
          minSupScriptShift = bmBase.ascent - supDrop;
          // get min supscript shift limit from x-height
          // = d(x) + 1/4 * sigma_5, Rule 18c, App. G, TeXbook
          minShiftFromXHeight = NSToCoordRound
            ((bmSupScript.descent + (1.0f/4.0f) * xHeight));
          trySupScriptShift =
            PR_MAX(minSupScriptShift,PR_MAX(minShiftFromXHeight,supScriptShift));
          mBoundingMetrics.ascent =
            PR_MAX(mBoundingMetrics.ascent,bmSupScript.ascent);
          ascent = PR_MAX(ascent,supScriptSize.ascent);
          width = PR_MAX(width, bmSupScript.width + scriptSpace);
          rightBearing = PR_MAX(rightBearing, bmSupScript.rightBearing);

          if (!mprescriptsFrame) { // we are still looping over base & postscripts
            mBoundingMetrics.rightBearing = mBoundingMetrics.width + rightBearing;
            mBoundingMetrics.width += width;
          }
          else {
            prescriptsWidth += width;
            if (firstPrescriptsPair) {
              firstPrescriptsPair = PR_FALSE;
              mBoundingMetrics.leftBearing =
                PR_MIN(bmSubScript.leftBearing, bmSupScript.leftBearing);
            }
          }
          width = rightBearing = 0;

          // negotiate between the various shifts so that
          // there is enough gap between the sup and subscripts
          // Rule 18e, App. G, TeXbook
          nscoord gap =
            (trySupScriptShift - bmSupScript.descent) -
            (bmSubScript.ascent - trySubScriptShift);
          if (gap < 4.0f * ruleSize) {
            // adjust trySubScriptShift to get a gap of (4.0 * ruleSize)
            trySubScriptShift += NSToCoordRound ((4.0f * ruleSize) - gap);
          }

          // next we want to ensure that the bottom of the superscript
          // will be > (4/5) * x-height above baseline
          gap = NSToCoordRound ((4.0f/5.0f) * xHeight -
                  (trySupScriptShift - bmSupScript.descent));
          if (gap > 0.0f) {
            trySupScriptShift += gap;
            trySubScriptShift -= gap;
          }
          
          maxSubScriptShift = PR_MAX(maxSubScriptShift, trySubScriptShift);
          maxSupScriptShift = PR_MAX(maxSupScriptShift, trySupScriptShift);

          trySubScriptShift = subScriptShift;
          trySupScriptShift = supScriptShift;
        }
      }

      isSubScript = !isSubScript;
    }
    count++;
    childFrame = childFrame->GetNextSibling();
  }
  // note: width=0 if all sup-sub pairs match correctly
  if ((0 != width) || !baseFrame || !subScriptFrame || !supScriptFrame) {
    // report an error, encourage people to get their markups in order
    NS_WARNING("invalid markup");
    return ReflowError(aRenderingContext, aDesiredSize);
  }

  // we left out the width of prescripts, so ...
  mBoundingMetrics.rightBearing += prescriptsWidth;
  mBoundingMetrics.width += prescriptsWidth;

  // we left out the base during our bounding box updates, so ...
  mBoundingMetrics.ascent =
    PR_MAX(mBoundingMetrics.ascent+maxSupScriptShift,bmBase.ascent);
  mBoundingMetrics.descent =
    PR_MAX(mBoundingMetrics.descent+maxSubScriptShift,bmBase.descent);

  // get the reflow metrics ...
  aDesiredSize.ascent =
    PR_MAX(ascent+maxSupScriptShift,baseSize.ascent);
  aDesiredSize.height = aDesiredSize.ascent +
    PR_MAX(descent+maxSubScriptShift,baseSize.height - baseSize.ascent);
  aDesiredSize.width = mBoundingMetrics.width;
  aDesiredSize.mBoundingMetrics = mBoundingMetrics;

  mReference.x = 0;
  mReference.y = aDesiredSize.ascent;

  //////////////////
  // Place Children

  // Place prescripts, followed by base, and then postscripts.
  // The list of frames is in the order: {base} {postscripts} {prescripts}
  // We go over the list in a circular manner, starting at <prescripts/>

  if (aPlaceOrigin) {
    nscoord dx = 0, dy = 0;

    count = 0;
    childFrame = mprescriptsFrame;
    do {
      if (!childFrame) { // end of prescripts,
        // place the base ...
        childFrame = baseFrame;
        dy = aDesiredSize.ascent - baseSize.ascent;
        FinishReflowChild (baseFrame, PresContext(), nsnull, baseSize, dx, dy, 0);
        dx += bmBase.width + italicCorrection;
      }
      else if (mprescriptsFrame != childFrame) {
        // process each sup/sub pair
        if (0 == count) {
          subScriptFrame = childFrame;
          count = 1;
        }
        else if (1 == count) {
          supScriptFrame = childFrame;
          count = 0;

          // get the ascent/descent of sup/subscripts stored in their rects
          // rect.x = descent, rect.y = ascent
          GetReflowAndBoundingMetricsFor(subScriptFrame, subScriptSize, bmSubScript);
          GetReflowAndBoundingMetricsFor(supScriptFrame, supScriptSize, bmSupScript);

          // center w.r.t. largest width
          width = PR_MAX(subScriptSize.width, supScriptSize.width);

          dy = aDesiredSize.ascent - subScriptSize.ascent +
            maxSubScriptShift;
          FinishReflowChild (subScriptFrame, PresContext(), nsnull, subScriptSize,
                             dx + (width-subScriptSize.width)/2, dy, 0);

          dy = aDesiredSize.ascent - supScriptSize.ascent -
            maxSupScriptShift;
          FinishReflowChild (supScriptFrame, PresContext(), nsnull, supScriptSize,
                             dx + (width-supScriptSize.width)/2, dy, 0);

          dx += width + scriptSpace;
        }
      }
      childFrame = childFrame->GetNextSibling();
    } while (mprescriptsFrame != childFrame);
  }

  return NS_OK;
}
