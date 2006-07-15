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
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
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

/* rendering object for HTML <spacer> element */

#include "nsHTMLParts.h"
#include "nsFrame.h"
#include "nsLineLayout.h"
#include "nsPresContext.h"
#include "nsHTMLAtoms.h"
#include "nsUnitConversion.h"
#include "nsStyleConsts.h"
#include "nsINameSpaceManager.h"

// Spacer type's
#define TYPE_WORD  0            // horizontal space
#define TYPE_LINE  1            // line-break + vertical space
#define TYPE_IMAGE 2            // acts like a sized image with nothing to see

class SpacerFrame : public nsFrame {
public:
  friend nsIFrame* NS_NewSpacerFrame(nsIPresShell* aPresShell, nsStyleContext* aContext);

  // nsIHTMLReflow
  NS_IMETHOD Reflow(nsPresContext*          aPresContext,
                    nsHTMLReflowMetrics&     aDesiredSize,
                    const nsHTMLReflowState& aReflowState,
                    nsReflowStatus&          aStatus);

  PRUint8 GetType();

protected:
  SpacerFrame(nsStyleContext* aContext) : nsFrame(aContext) {}
  virtual ~SpacerFrame();
};

nsIFrame*
NS_NewSpacerFrame(nsIPresShell* aPresShell, nsStyleContext* aContext)
{
  return new (aPresShell) SpacerFrame(aContext);
}

SpacerFrame::~SpacerFrame()
{
}

NS_IMETHODIMP
SpacerFrame::Reflow(nsPresContext*          aPresContext,
                    nsHTMLReflowMetrics&     aMetrics,
                    const nsHTMLReflowState& aReflowState,
                    nsReflowStatus&          aStatus)
{
  DO_GLOBAL_REFLOW_COUNT("SpacerFrame", aReflowState.reason);
  DISPLAY_REFLOW(aPresContext, this, aReflowState, aMetrics, aStatus);
  aStatus = NS_FRAME_COMPLETE;

  // By default, we have no area
  aMetrics.width = 0;
  aMetrics.height = 0;
  aMetrics.ascent = 0;
  aMetrics.descent = 0;

  const nsStylePosition* position = GetStylePosition();

  PRUint8 type = GetType();
  switch (type) {
  case TYPE_WORD:
    break;

  case TYPE_LINE:
    aStatus = NS_INLINE_LINE_BREAK_AFTER(NS_FRAME_COMPLETE);
    if (eStyleUnit_Coord == position->mHeight.GetUnit()) {
      aMetrics.width = position->mHeight.GetCoordValue();
    }
    aMetrics.ascent = aMetrics.height;
    break;

  case TYPE_IMAGE:
    // width
    nsStyleUnit unit = position->mWidth.GetUnit();
    if (eStyleUnit_Coord == unit) {
      aMetrics.width = position->mWidth.GetCoordValue();
    }
    else if (eStyleUnit_Percent == unit) 
    {
      if (NS_UNCONSTRAINEDSIZE != aReflowState.availableWidth)
      {
        float factor = position->mWidth.GetPercentValue();
        aMetrics.width = NSToCoordRound (factor * aReflowState.availableWidth);
      }
    }

    // height
    unit = position->mHeight.GetUnit();
    if (eStyleUnit_Coord == unit) {
      aMetrics.height = position->mHeight.GetCoordValue();
    }
    else if (eStyleUnit_Percent == unit) 
    {
      if (NS_UNCONSTRAINEDSIZE != aReflowState.availableHeight)
      {
        float factor = position->mHeight.GetPercentValue();
        aMetrics.width = NSToCoordRound (factor * aReflowState.availableHeight);
      }
    }
    // accent
    aMetrics.ascent = aMetrics.height;
    break;
  }

  if (aMetrics.width || aMetrics.height) {
    // Make sure that the other dimension is non-zero
    if (!aMetrics.width) aMetrics.width = 1;
    if (!aMetrics.height) aMetrics.height = 1;
  }

  if (aMetrics.mComputeMEW) {
    aMetrics.SetMEWToActualWidth(aReflowState.mStylePosition->mWidth.GetUnit());
  }

  NS_FRAME_SET_TRUNCATION(aStatus, aReflowState, aMetrics);
  return NS_OK;
}

PRUint8
SpacerFrame::GetType()
{
  PRUint8 type = TYPE_WORD;
  static nsIContent::AttrValuesArray strings[] =
    {&nsHTMLAtoms::line, &nsHTMLAtoms::vert, &nsHTMLAtoms::vertical,
     &nsHTMLAtoms::block, nsnull};
  switch (mContent->FindAttrValueIn(kNameSpaceID_None, nsHTMLAtoms::type,
                                    strings, eIgnoreCase)) {
    case 0:
    case 1:
    case 2:
      return TYPE_LINE;
    case 3:
      return TYPE_IMAGE;
  }
  return type;
}
