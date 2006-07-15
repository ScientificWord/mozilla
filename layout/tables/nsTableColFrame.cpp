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
#include "nsCOMPtr.h"
#include "nsTableColFrame.h"
#include "nsTableFrame.h"
#include "nsContainerFrame.h"
#include "nsStyleContext.h"
#include "nsStyleConsts.h"
#include "nsPresContext.h"
#include "nsHTMLAtoms.h"
#include "nsCSSRendering.h"
#include "nsLayoutAtoms.h"
#include "nsIContent.h"
#include "nsIDOMHTMLTableColElement.h"

#define COL_TYPE_BITS                 0xF0000000 // uses bits 29-32 from mState
#define COL_TYPE_OFFSET               28

#define COL_CONSTRAINT_BITS           0x07000000 // uses bits 25-27
#define COL_CONSTRAINT_OFFSET         24

nsTableColFrame::nsTableColFrame(nsStyleContext* aContext) : nsFrame(aContext)
{
  SetColType(eColContent);
  ResetSizingInfo();
}

nsTableColFrame::~nsTableColFrame()
{
}

nsTableColType 
nsTableColFrame::GetColType() const 
{
  return (nsTableColType)((mState & COL_TYPE_BITS) >> COL_TYPE_OFFSET);
}

void 
nsTableColFrame::SetColType(nsTableColType aType) 
{
  PRUint32 type = aType - eColContent;
  mState |= (type << COL_TYPE_OFFSET);
}

nsColConstraint 
nsTableColFrame::GetConstraint() const
{ 
  return (nsColConstraint)((mState & COL_CONSTRAINT_BITS) >> COL_CONSTRAINT_OFFSET);
}

void 
nsTableColFrame::SetConstraint(nsColConstraint aConstraint)
{ 
  PRUint32 con = aConstraint - eNoConstraint;
  mState |= (con << COL_CONSTRAINT_OFFSET);
}

// XXX what about other style besides width
nsStyleCoord nsTableColFrame::GetStyleWidth() const
{
  nsStyleCoord styleWidth = GetStylePosition()->mWidth;
  if (eStyleUnit_Auto == styleWidth.GetUnit()) {
    styleWidth = GetParent()->GetStylePosition()->mWidth;
  }

  nsStyleCoord returnWidth;
  returnWidth.mUnit  = styleWidth.mUnit;
  returnWidth.mValue = styleWidth.mValue;
  return returnWidth;
}

void nsTableColFrame::SetContinuousBCBorderWidth(PRUint8     aForSide,
                                                 BCPixelSize aPixelValue)
{
  switch (aForSide) {
    case NS_SIDE_TOP:
      mTopContBorderWidth = aPixelValue;
      return;
    case NS_SIDE_RIGHT:
      mRightContBorderWidth = aPixelValue;
      return;
    case NS_SIDE_BOTTOM:
      mBottomContBorderWidth = aPixelValue;
      return;
    default:
      NS_ERROR("invalid side arg");
  }
}

void nsTableColFrame::ResetSizingInfo()
{
  memset(mWidths, WIDTH_NOT_SET, NUM_WIDTHS * sizeof(PRInt32));
  SetConstraint(eNoConstraint);
}

NS_METHOD nsTableColFrame::Reflow(nsPresContext*          aPresContext,
                                  nsHTMLReflowMetrics&     aDesiredSize,
                                  const nsHTMLReflowState& aReflowState,
                                  nsReflowStatus&          aStatus)
{
  DO_GLOBAL_REFLOW_COUNT("nsTableColFrame", aReflowState.reason);
  DISPLAY_REFLOW(aPresContext, this, aReflowState, aDesiredSize, aStatus);
  aDesiredSize.width=0;
  aDesiredSize.height=0;
  const nsStyleVisibility* colVis = GetStyleVisibility();
  PRBool collapseCol = (NS_STYLE_VISIBILITY_COLLAPSE == colVis->mVisible);
  if (collapseCol) {
    nsTableFrame* tableFrame = nsTableFrame::GetTableFrame(this);
    if (tableFrame)  {
      tableFrame->SetNeedToCollapse(PR_TRUE);
    }    
  }
  if (aDesiredSize.mComputeMEW)
  {
    aDesiredSize.mMaxElementWidth=0;
  }
  aStatus = NS_FRAME_COMPLETE;
  NS_FRAME_SET_TRUNCATION(aStatus, aReflowState, aDesiredSize);
  return NS_OK;
}

PRInt32 nsTableColFrame::GetSpan()
{
  return GetStyleTable()->mSpan;
}

nscoord nsTableColFrame::GetWidth(PRUint32 aWidthType)
{
  NS_ASSERTION(aWidthType < NUM_WIDTHS, "GetWidth: bad width type");
  return mWidths[aWidthType];
}

void nsTableColFrame::SetWidth(PRUint32 aWidthType,
                               nscoord  aWidth)
{
  NS_ASSERTION(aWidthType < NUM_WIDTHS, "SetWidth: bad width type");
  mWidths[aWidthType] = aWidth;
#ifdef MY_DEBUG
  if (aWidth > 0) {
    nscoord minWidth = GetMinWidth();
    if ((MIN_CON != aWidthType) && (aWidth < minWidth)) {
      printf("non min width set to lower than min \n");
    }
  }
#endif
}

nscoord nsTableColFrame::GetMinWidth()
{
  return PR_MAX(mWidths[MIN_CON], mWidths[MIN_ADJ]);
}

nscoord nsTableColFrame::GetDesWidth()
{
  return PR_MAX(mWidths[DES_CON], mWidths[DES_ADJ]);
}

nscoord nsTableColFrame::GetFixWidth()
{
  return PR_MAX(mWidths[FIX], mWidths[FIX_ADJ]);
}

nscoord nsTableColFrame::GetPctWidth()
{
  return PR_MAX(mWidths[PCT], mWidths[PCT_ADJ]);
}

#ifdef DEBUG
void nsTableColFrame::Dump(PRInt32 aIndent)
{
  char* indent = new char[aIndent + 1];
  if (!indent) return;
  for (PRInt32 i = 0; i < aIndent + 1; i++) {
    indent[i] = ' ';
  }
  indent[aIndent] = 0;

  printf("%s**START COL DUMP**\n%s colIndex=%d constraint=%d coltype=",
    indent, indent, mColIndex, GetConstraint());
  nsTableColType colType = GetColType();
  switch (colType) {
  case eColContent:
    printf(" content ");
    break;
  case eColAnonymousCol: 
    printf(" anonymous-column ");
    break;
  case eColAnonymousColGroup:
    printf(" anonymous-colgroup ");
    break;
  case eColAnonymousCell: 
    printf(" anonymous-cell ");
    break;
  }
  printf("\n%s widths=", indent);
  for (PRInt32 widthX = 0; widthX < NUM_WIDTHS; widthX++) {
    printf("%d ", mWidths[widthX]);
  }
  printf("\n%s**END COL DUMP** ", indent);
  delete [] indent;
}
#endif
/* ----- global methods ----- */

nsIFrame* 
NS_NewTableColFrame(nsIPresShell* aPresShell, nsStyleContext* aContext)
{
  return new (aPresShell) nsTableColFrame(aContext);
}

NS_IMETHODIMP
nsTableColFrame::Init(nsIContent*      aContent,
                      nsIFrame*        aParent,
                      nsIFrame*        aPrevInFlow)
{
  // Let the base class do its initialization
  nsresult rv = nsFrame::Init(aContent, aParent, aPrevInFlow);

  // record that children that are ignorable whitespace should be excluded 
  mState |= NS_FRAME_EXCLUDE_IGNORABLE_WHITESPACE;

  return rv;
}

nsTableColFrame*  
nsTableColFrame::GetNextCol() const
{
  nsIFrame* childFrame = GetNextSibling();
  while (childFrame) {
    if (nsLayoutAtoms::tableColFrame == childFrame->GetType()) {
      return (nsTableColFrame*)childFrame;
    }
    childFrame = childFrame->GetNextSibling();
  }
  return nsnull;
}

nsIAtom*
nsTableColFrame::GetType() const
{
  return nsLayoutAtoms::tableColFrame;
}

#ifdef DEBUG
NS_IMETHODIMP
nsTableColFrame::GetFrameName(nsAString& aResult) const
{
  return MakeFrameName(NS_LITERAL_STRING("TableCol"), aResult);
}
#endif
