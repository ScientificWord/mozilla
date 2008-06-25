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
 * The University of Queensland.
 * Portions created by the Initial Developer are Copyright (C) 1999
 * the Initial Developer. All Rights Reserved.
 * 
 * Contributor(s): 
 *   Roger B. Sidje <rbs@maths.uq.edu.au>
 *   Karl Tomlinson <karlt+@karlt.net>, Mozilla Corporation
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

#include "nsCOMPtr.h"
#include "nsFrame.h"
#include "nsPresContext.h"
#include "nsStyleContext.h"
#include "nsStyleConsts.h"
#include "nsIRenderingContext.h"
#include "nsIFontMetrics.h"
#include "nsContentUtils.h"
#include "nsCSSFrameConstructor.h"
#include "nsMathMLTokenFrame.h"

nsIFrame*
NS_NewMathMLTokenFrame(nsIPresShell* aPresShell, nsStyleContext* aContext)
{
  return new (aPresShell) nsMathMLTokenFrame(aContext);
}
nsMathMLTokenFrame::~nsMathMLTokenFrame()
{
}

eMathMLFrameType
nsMathMLTokenFrame::GetMathMLFrameType()
{
  // treat everything other than <mi> as ordinary...
  if (mContent->Tag() != nsGkAtoms::mi_) {
    return eMathMLFrameType_Ordinary;
  }

  // for <mi>, distinguish between italic and upright...
  // Don't use nsMathMLFrame::GetAttribute for mathvariant or fontstyle as
  // default values are not inherited.
  nsAutoString style;
  // mathvariant overrides fontstyle
  // http://www.w3.org/TR/2003/REC-MathML2-20031021/chapter3.html#presm.deprecatt
  mContent->GetAttr(kNameSpaceID_None, nsGkAtoms::MOZfontstyle, style) ||
    mContent->GetAttr(kNameSpaceID_None, nsGkAtoms::mathvariant_, style) ||
    mContent->GetAttr(kNameSpaceID_None, nsGkAtoms::fontstyle_, style);

  if (style.EqualsLiteral("italic") || style.EqualsLiteral("bold-italic") ||
      style.EqualsLiteral("script") || style.EqualsLiteral("bold-script") ||
      style.EqualsLiteral("sans-serif-italic") ||
      style.EqualsLiteral("sans-serif-bold-italic")) {
    return eMathMLFrameType_ItalicIdentifier;
  }
  else if(style.EqualsLiteral("invariant")) {
    nsAutoString data;
    nsContentUtils::GetNodeTextContent(mContent, PR_FALSE, data);
    eMATHVARIANT variant = nsMathMLOperators::LookupInvariantChar(data);

    switch (variant) {
    case eMATHVARIANT_italic:
    case eMATHVARIANT_bold_italic:
    case eMATHVARIANT_script:
    case eMATHVARIANT_bold_script:
    case eMATHVARIANT_sans_serif_italic:
    case eMATHVARIANT_sans_serif_bold_italic:
      return eMathMLFrameType_ItalicIdentifier;
    default:
      ; // fall through to upright
    }
  }
  return eMathMLFrameType_UprightIdentifier;
}

static void
CompressWhitespace(nsIContent* aContent)
{
  PRUint32 numKids = aContent->GetChildCount();
  for (PRUint32 kid = 0; kid < numKids; kid++) {
    nsIContent* cont = aContent->GetChildAt(kid);
    if (cont && cont->IsNodeOfType(nsINode::eTEXT)) {
      nsAutoString text;
      cont->AppendTextTo(text);
      text.CompressWhitespace();
      cont->SetText(text, PR_FALSE); // not meant to be used if notify is needed
    }
  }
}

NS_IMETHODIMP
nsMathMLTokenFrame::Init(nsIContent*      aContent,
                         nsIFrame*        aParent,
                         nsIFrame*        aPrevInFlow)
{
  // leading and trailing whitespace doesn't count -- bug 15402
  // brute force removal for people who do <mi> a </mi> instead of <mi>a</mi>
  // XXX the best fix is to skip these in nsTextFrame
  CompressWhitespace(aContent);

  // let the base class do its Init()
  return nsMathMLContainerFrame::Init(aContent, aParent, aPrevInFlow);
}

NS_IMETHODIMP
nsMathMLTokenFrame::SetInitialChildList(nsIAtom*        aListName,
                                        nsIFrame*       aChildList)
{
  // First, let the base class do its work
  nsresult rv = nsMathMLContainerFrame::SetInitialChildList(aListName, aChildList);
  if (NS_FAILED(rv))
    return rv;

  SetQuotes();
  ProcessTextData();
  return rv;
}

nsresult
nsMathMLTokenFrame::Reflow(nsPresContext*          aPresContext,
                           nsHTMLReflowMetrics&     aDesiredSize,
                           const nsHTMLReflowState& aReflowState,
                           nsReflowStatus&          aStatus)
{
  nsresult rv = NS_OK;

  // initializations needed for empty markup like <mtag></mtag>
  aDesiredSize.width = aDesiredSize.height = 0;
  aDesiredSize.ascent = 0;
  aDesiredSize.mBoundingMetrics.Clear();

  nsSize availSize(aReflowState.ComputedWidth(), NS_UNCONSTRAINEDSIZE);
  nsIFrame* childFrame = GetFirstChild(nsnull);
  while (childFrame) {
    // ask our children to compute their bounding metrics
    nsHTMLReflowMetrics childDesiredSize(aDesiredSize.mFlags
                                         | NS_REFLOW_CALC_BOUNDING_METRICS);
    nsHTMLReflowState childReflowState(aPresContext, aReflowState,
                                       childFrame, availSize);
    rv = ReflowChild(childFrame, aPresContext, childDesiredSize,
                     childReflowState, aStatus);
    //NS_ASSERTION(NS_FRAME_IS_COMPLETE(aStatus), "bad status");
    if (NS_FAILED(rv)) {
      // Call DidReflow() for the child frames we successfully did reflow.
      DidReflowChildren(GetFirstChild(nsnull), childFrame);
      return rv;
    }

    SaveReflowAndBoundingMetricsFor(childFrame, childDesiredSize,
                                    childDesiredSize.mBoundingMetrics);

    childFrame = childFrame->GetNextSibling();
  }


  // place and size children
  FinalizeReflow(*aReflowState.rendContext, aDesiredSize);

  aStatus = NS_FRAME_COMPLETE;
  NS_FRAME_SET_TRUNCATION(aStatus, aReflowState, aDesiredSize);
  return NS_OK;
}

// For token elements, mBoundingMetrics is computed at the ReflowToken
// pass, it is not computed here because our children may be text frames
// that do not implement the GetBoundingMetrics() interface.
/* virtual */ nsresult
nsMathMLTokenFrame::Place(nsIRenderingContext& aRenderingContext,
                          PRBool               aPlaceOrigin,
                          nsHTMLReflowMetrics& aDesiredSize)
{
  mBoundingMetrics.Clear();
  nsIFrame* childFrame = GetFirstChild(nsnull);
  while (childFrame) {
    nsHTMLReflowMetrics childSize;
    GetReflowAndBoundingMetricsFor(childFrame, childSize,
                                   childSize.mBoundingMetrics, nsnull);
    // compute and cache the bounding metrics
    mBoundingMetrics += childSize.mBoundingMetrics;

    childFrame = childFrame->GetNextSibling();
  }

  nsCOMPtr<nsIFontMetrics> fm =
    PresContext()->GetMetricsFor(GetStyleFont()->mFont);
  nscoord ascent, descent;
  fm->GetMaxAscent(ascent);
  fm->GetMaxDescent(descent);

  aDesiredSize.mBoundingMetrics = mBoundingMetrics;
  aDesiredSize.width = mBoundingMetrics.width;
  aDesiredSize.ascent = PR_MAX(mBoundingMetrics.ascent, ascent);
  aDesiredSize.height = aDesiredSize.ascent +
                        PR_MAX(mBoundingMetrics.descent, descent);

  if (aPlaceOrigin) {
    nscoord dy, dx = 0;
    nsIFrame* childFrame = GetFirstChild(nsnull);
    while (childFrame) {
      nsHTMLReflowMetrics childSize;
      GetReflowAndBoundingMetricsFor(childFrame, childSize,
                                     childSize.mBoundingMetrics);

      // place and size the child; (dx,0) makes the caret happy - bug 188146
      dy = childSize.height == 0 ? 0 : aDesiredSize.ascent - childSize.ascent;
      FinishReflowChild(childFrame, PresContext(), nsnull, childSize, dx, dy, 0);
      dx += childSize.width;
      childFrame = childFrame->GetNextSibling();
    }
  }

  SetReference(nsPoint(0, aDesiredSize.ascent));

  return NS_OK;
}

/* virtual */ void
nsMathMLTokenFrame::MarkIntrinsicWidthsDirty()
{
  // this could be called due to changes in the nsTextFrame beneath us
  // when something changed in the text content. So re-process our text
  ProcessTextData();

  nsMathMLContainerFrame::MarkIntrinsicWidthsDirty();
}

NS_IMETHODIMP
nsMathMLTokenFrame::AttributeChanged(PRInt32         aNameSpaceID,
                                     nsIAtom*        aAttribute,
                                     PRInt32         aModType)
{
  if (nsGkAtoms::lquote_ == aAttribute ||
      nsGkAtoms::rquote_ == aAttribute) {
    SetQuotes();
  }

  return nsMathMLContainerFrame::
         AttributeChanged(aNameSpaceID, aAttribute, aModType);
}

void
nsMathMLTokenFrame::ProcessTextData()
{
  // see if the style changes from normal to italic or vice-versa
  if (!SetTextStyle())
    return;

  // explicitly request a re-resolve to pick up the change of style
  PresContext()->PresShell()->FrameConstructor()->
    PostRestyleEvent(mContent, eReStyle_Self, NS_STYLE_HINT_NONE);
}

///////////////////////////////////////////////////////////////////////////
// For <mi>, if the content is not a single character, turn the font to
// normal (this function will also query attributes from the mstyle hierarchy)
// Returns PR_TRUE if there is a style change.
//
// http://www.w3.org/TR/2003/REC-MathML2-20031021/chapter3.html#presm.commatt
//
//  "It is important to note that only certain combinations of
//   character data and mathvariant attribute values make sense.
//   ...
//   By design, the only cases that have an unambiguous
//   interpretation are exactly the ones that correspond to SMP Math
//   Alphanumeric Symbol characters, which are enumerated in Section
//   6.2.3 Mathematical Alphanumeric Symbols Characters. In all other
//   cases, it is suggested that renderers ignore the value of the
//   mathvariant attribute if it is present."
//
// There are no corresponding characters for mathvariant=normal, suggesting
// that this value should be ignored, but this (from the same section of
// Chapter 3) implies that font-style should not be inherited, but set to
// normal for mathvariant=normal:
//
//  "In particular, inheritance of the mathvariant attribute does not follow
//   the CSS model. The default value for this attribute is "normal"
//   (non-slanted) for all tokens except mi. ... (The deprecated fontslant
//   attribute also behaves this way.)"

PRBool
nsMathMLTokenFrame::SetTextStyle()
{
  if (mContent->Tag() != nsGkAtoms::mi_)
    return PR_FALSE;

  if (!mFrames.FirstChild())
    return PR_FALSE;

  // Get the text content that we enclose and its length
  nsAutoString data;
  nsContentUtils::GetNodeTextContent(mContent, PR_FALSE, data);
  PRInt32 length = data.Length();
  if (!length)
    return PR_FALSE;

  nsAutoString fontstyle;
  PRBool isSingleCharacter =
    length == 1 ||
    (length == 2 && NS_IS_HIGH_SURROGATE(data[0]));
  if (isSingleCharacter &&
      nsMathMLOperators::LookupInvariantChar(data) != eMATHVARIANT_NONE) {
    // bug 65951 - a non-stylable character has its own intrinsic appearance
    fontstyle.AssignLiteral("invariant");
  }
  else {
    // Attributes override the default behavior.
    if (!(mContent->HasAttr(kNameSpaceID_None, nsGkAtoms::mathvariant_) ||
          mContent->HasAttr(kNameSpaceID_None, nsGkAtoms::fontstyle_))) {
      if (!isSingleCharacter) {
        fontstyle.AssignLiteral("normal");
      }
      else if (length == 1 && // BMP
               !nsMathMLOperators::
                TransformVariantChar(data[0], eMATHVARIANT_italic).
                Equals(data)) {
        // Transformation exists.  Try to make the BMP character look like the
        // styled character using the style system until bug 114365 is resolved.
        fontstyle.AssignLiteral("italic");
      }
      // else single character but there is no corresponding Math Alphanumeric
      // Symbol character: "ignore the value of the [default] mathvariant
      // attribute".
    }
  }

  // set the -moz-math-font-style attribute without notifying that we want a reflow
  if (fontstyle.IsEmpty()) {
    if (mContent->HasAttr(kNameSpaceID_None, nsGkAtoms::MOZfontstyle)) {
      mContent->UnsetAttr(kNameSpaceID_None, nsGkAtoms::MOZfontstyle, PR_FALSE);
      return PR_TRUE;
    }
  }
  else if (!mContent->AttrValueIs(kNameSpaceID_None, nsGkAtoms::MOZfontstyle,
                                  fontstyle, eCaseMatters)) {
    mContent->SetAttr(kNameSpaceID_None, nsGkAtoms::MOZfontstyle,
                      fontstyle, PR_FALSE);
    return PR_TRUE;
  }

  return PR_FALSE;
}

///////////////////////////////////////////////////////////////////////////
// For <ms>, it is assumed that the mathml.css file contains two rules:
// ms:before { content: open-quote; }
// ms:after { content: close-quote; }
// With these two rules, the frame construction code will
// create inline frames that contain text frames which themselves
// contain the text content of the quotes.
// So the main idea in this code is to see if there are lquote and 
// rquote attributes. If these are there, we ovewrite the default
// quotes in the text frames.
//
// But what if the mathml.css file wasn't loaded? 
// We also check that we are not relying on null pointers...

static void
SetQuote(nsIFrame*       aFrame, 
         nsString&       aValue)
{
  nsIFrame* textFrame;
  do {
    // walk down the hierarchy of first children because they could be wrapped
    textFrame = aFrame->GetFirstChild(nsnull);
    if (textFrame) {
      if (textFrame->GetType() == nsGkAtoms::textFrame)
        break;
    }
    aFrame = textFrame;
  } while (textFrame);
  if (textFrame) {
    nsIContent* quoteContent = textFrame->GetContent();
    if (quoteContent && quoteContent->IsNodeOfType(nsINode::eTEXT)) {
      quoteContent->SetText(aValue, PR_FALSE); // no notify since we don't want a reflow yet
    }
  }
}

void
nsMathMLTokenFrame::SetQuotes()
{
  if (mContent->Tag() != nsGkAtoms::ms_)
    return;

  nsIFrame* rightFrame = nsnull;
  nsIFrame* baseFrame = nsnull;
  nsIFrame* leftFrame = mFrames.FirstChild();
  if (leftFrame)
    baseFrame = leftFrame->GetNextSibling();
  if (baseFrame)
    rightFrame = baseFrame->GetNextSibling();
  if (!leftFrame || !baseFrame || !rightFrame)
    return;

  nsAutoString value;
  // lquote
  if (GetAttribute(mContent, mPresentationData.mstyle,
                   nsGkAtoms::lquote_, value)) {
    SetQuote(leftFrame, value);
  }
  // rquote
  if (GetAttribute(mContent, mPresentationData.mstyle,
                   nsGkAtoms::rquote_, value)) {
    SetQuote(rightFrame, value);
  }
}
