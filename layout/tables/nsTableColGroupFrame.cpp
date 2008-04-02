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
#include "nsTableColGroupFrame.h"
#include "nsTableColFrame.h"
#include "nsTableFrame.h"
#include "nsIDOMHTMLTableColElement.h"
#include "nsStyleContext.h"
#include "nsStyleConsts.h"
#include "nsPresContext.h"
#include "nsHTMLParts.h"
#include "nsGkAtoms.h"
#include "nsCOMPtr.h"
#include "nsCSSRendering.h"
#include "nsIPresShell.h"

#define COL_GROUP_TYPE_BITS          0xC0000000 // uses bits 31-32 from mState
#define COL_GROUP_TYPE_OFFSET        30

nsTableColGroupType 
nsTableColGroupFrame::GetColType() const 
{
  return (nsTableColGroupType)((mState & COL_GROUP_TYPE_BITS) >> COL_GROUP_TYPE_OFFSET);
}

void nsTableColGroupFrame::SetColType(nsTableColGroupType aType) 
{
  PRUint32 type = aType - eColGroupContent;
  mState |= (type << COL_GROUP_TYPE_OFFSET);
}

void nsTableColGroupFrame::ResetColIndices(nsIFrame*       aFirstColGroup,
                                           PRInt32         aFirstColIndex,
                                           nsIFrame*       aStartColFrame)
{
  nsTableColGroupFrame* colGroupFrame = (nsTableColGroupFrame*)aFirstColGroup;
  PRInt32 colIndex = aFirstColIndex;
  while (colGroupFrame) {
    if (nsGkAtoms::tableColGroupFrame == colGroupFrame->GetType()) {
      // reset the starting col index for the first cg only if we should reset
      // the whole colgroup (aStartColFrame defaults to nsnull) or if
      // aFirstColIndex is smaller than the existing starting col index
      if ((colIndex != aFirstColIndex) ||
          (colIndex < colGroupFrame->GetStartColumnIndex()) ||
          !aStartColFrame) {
        colGroupFrame->SetStartColumnIndex(colIndex);
      }
      nsIFrame* colFrame = aStartColFrame; 
      if (!colFrame || (colIndex != aFirstColIndex)) {
        colFrame = colGroupFrame->GetFirstChild(nsnull);
      }
      while (colFrame) {
        if (nsGkAtoms::tableColFrame == colFrame->GetType()) {
          ((nsTableColFrame*)colFrame)->SetColIndex(colIndex);
          colIndex++;
        }
        colFrame = colFrame->GetNextSibling();
      }
    }
    colGroupFrame = static_cast<nsTableColGroupFrame*>
                               (colGroupFrame->GetNextSibling());
  }
}


nsresult
nsTableColGroupFrame::AddColsToTable(PRInt32          aFirstColIndex,
                                     PRBool           aResetSubsequentColIndices,
                                     nsIFrame*        aFirstFrame,
                                     nsIFrame*        aLastFrame)
{
  nsresult rv = NS_OK;
  nsTableFrame* tableFrame = nsTableFrame::GetTableFrame(this);
  if (!tableFrame || !aFirstFrame)
    return NS_ERROR_NULL_POINTER;

  // set the col indices of the col frames and and add col info to the table
  PRInt32 colIndex = aFirstColIndex;
  nsIFrame* kidFrame = aFirstFrame;
  PRBool foundLastFrame = PR_FALSE;
  while (kidFrame) {
    if (nsGkAtoms::tableColFrame == kidFrame->GetType()) {
      ((nsTableColFrame*)kidFrame)->SetColIndex(colIndex);
      if (!foundLastFrame) {
        mColCount++;
        tableFrame->InsertCol((nsTableColFrame &)*kidFrame, colIndex);
      }
      colIndex++;
    }
    if (kidFrame == aLastFrame) {
      foundLastFrame = PR_TRUE;
    }
    kidFrame = kidFrame->GetNextSibling(); 
  }
  // We have already set the colindex for all the colframes in this
  // colgroup that come after the first inserted colframe, but there could
  // be other colgroups following this one and their colframes need
  // correct colindices too.
  if (aResetSubsequentColIndices && GetNextSibling()) {
    ResetColIndices(GetNextSibling(), colIndex);
  }

  return rv;
}


PRBool
nsTableColGroupFrame::GetLastRealColGroup(nsTableFrame* aTableFrame, 
                                          nsIFrame**    aLastColGroup)
{
  *aLastColGroup = nsnull;
  nsFrameList colGroups = aTableFrame->GetColGroups();

  nsIFrame* nextToLastColGroup = nsnull;
  nsIFrame* lastColGroup       = colGroups.FirstChild();
  while(lastColGroup) {
    nsIFrame* next = lastColGroup->GetNextSibling();
    if (next) {
      nextToLastColGroup = lastColGroup;
      lastColGroup = next;
    }
    else {
      break;
    }
  }

  if (!lastColGroup) return PR_TRUE; // there are no col group frames
 
  nsTableColGroupType lastColGroupType =
    ((nsTableColGroupFrame *)lastColGroup)->GetColType();
  if (eColGroupAnonymousCell == lastColGroupType) {
    *aLastColGroup = nextToLastColGroup;
    return PR_FALSE;
  }
  else {
    *aLastColGroup = lastColGroup;
    return PR_TRUE;
  }
}

// don't set mColCount here, it is done in AddColsToTable
NS_IMETHODIMP
nsTableColGroupFrame::SetInitialChildList(nsIAtom*        aListName,
                                          nsIFrame*       aChildList)
{
  if (!mFrames.IsEmpty()) {
    // We already have child frames which means we've already been
    // initialized
    NS_NOTREACHED("unexpected second call to SetInitialChildList");
    return NS_ERROR_UNEXPECTED;
  }
  if (aListName) {
    // All we know about is the unnamed principal child list
    NS_NOTREACHED("unknown frame list");
    return NS_ERROR_INVALID_ARG;
  } 
  nsTableFrame* tableFrame = nsTableFrame::GetTableFrame(this);
  if (!tableFrame)
    return NS_ERROR_NULL_POINTER;

  if (!aChildList) {
    nsIFrame* firstChild;
    tableFrame->CreateAnonymousColFrames(this, GetSpan(), eColAnonymousColGroup, 
                                         PR_FALSE, nsnull, &firstChild);
    if (firstChild) {
      SetInitialChildList(aListName, firstChild);
    }
    return NS_OK; 
  }

  mFrames.AppendFrames(this, aChildList);
  return NS_OK;
}

NS_IMETHODIMP
nsTableColGroupFrame::AppendFrames(nsIAtom*        aListName,
                                   nsIFrame*       aFrameList)
{
  NS_ASSERTION(!aListName, "unexpected child list");

  nsTableColFrame* col = GetFirstColumn();
  nsTableColFrame* nextCol;
  while (col && col->GetColType() == eColAnonymousColGroup) {
    // this colgroup spans one or more columns but now that there is a
    // real column below, spanned anonymous columns should be removed,
    // since the HTML spec says to ignore the span of a colgroup if it
    // has content columns in it.
    nextCol = col->GetNextCol();
    RemoveFrame(nsnull, col);
    col = nextCol;
  }

  mFrames.AppendFrames(this, aFrameList);
  InsertColsReflow(GetStartColumnIndex() + mColCount, aFrameList);
  return NS_OK;
}

NS_IMETHODIMP
nsTableColGroupFrame::InsertFrames(nsIAtom*        aListName,
                                   nsIFrame*       aPrevFrame,
                                   nsIFrame*       aFrameList)
{
  NS_ASSERTION(!aListName, "unexpected child list");
  NS_ASSERTION(!aPrevFrame || aPrevFrame->GetParent() == this,
               "inserting after sibling frame with different parent");

  nsFrameList frames(aFrameList); // convience for getting last frame
  nsIFrame* lastFrame = frames.LastChild();

  nsTableColFrame* col = GetFirstColumn();
  nsTableColFrame* nextCol;
  while (col && col->GetColType() == eColAnonymousColGroup) {
    // this colgroup spans one or more columns but now that there is a
    // real column below, spanned anonymous columns should be removed,
    // since the HTML spec says to ignore the span of a colgroup if it
    // has content columns in it.
    NS_ASSERTION(col != aPrevFrame, "Bad aPrevFrame");
    nextCol = col->GetNextCol();
    RemoveFrame(nsnull, col);
    col = nextCol;
  }

  NS_ASSERTION(!aPrevFrame || aPrevFrame == aPrevFrame->GetLastContinuation(),
               "Prev frame should be last in continuation chain");
  NS_ASSERTION(!aPrevFrame || !GetNextColumn(aPrevFrame) ||
               GetNextColumn(aPrevFrame)->GetColType() != eColAnonymousCol,
               "Shouldn't be inserting before a spanned colframe");

  mFrames.InsertFrames(this, aPrevFrame, aFrameList);
  nsIFrame* prevFrame = nsTableFrame::GetFrameAtOrBefore(this, aPrevFrame,
                                                         nsGkAtoms::tableColFrame);

  PRInt32 colIndex = (prevFrame) ? ((nsTableColFrame*)prevFrame)->GetColIndex() + 1 : GetStartColumnIndex();
  InsertColsReflow(colIndex, aFrameList, lastFrame);

  return NS_OK;
}

void
nsTableColGroupFrame::InsertColsReflow(PRInt32         aColIndex,
                                       nsIFrame*       aFirstFrame,
                                       nsIFrame*       aLastFrame)
{
  AddColsToTable(aColIndex, PR_TRUE, aFirstFrame, aLastFrame);

  nsTableFrame* tableFrame = nsTableFrame::GetTableFrame(this);
  if (!tableFrame)
    return;

  PresContext()->PresShell()->FrameNeedsReflow(tableFrame,
                                               nsIPresShell::eTreeChange,
                                               NS_FRAME_HAS_DIRTY_CHILDREN);
}

void
nsTableColGroupFrame::RemoveChild(nsTableColFrame& aChild,
                                  PRBool           aResetSubsequentColIndices)
{
  PRInt32 colIndex = 0;
  nsIFrame* nextChild = nsnull;
  if (aResetSubsequentColIndices) {
    colIndex = aChild.GetColIndex();
    nextChild = aChild.GetNextSibling();
  }
  if (mFrames.DestroyFrame((nsIFrame*)&aChild)) {
    mColCount--;
    if (aResetSubsequentColIndices) {
      if (nextChild) { // reset inside this and all following colgroups
        ResetColIndices(this, colIndex, nextChild);
      }
      else {
        nsIFrame* nextGroup = GetNextSibling();
        if (nextGroup) // reset next and all following colgroups
          ResetColIndices(nextGroup, colIndex);
      }
    }
  }
  nsTableFrame* tableFrame = nsTableFrame::GetTableFrame(this);
  if (!tableFrame)
    return;

  PresContext()->PresShell()->FrameNeedsReflow(tableFrame,
                                               nsIPresShell::eTreeChange,
                                               NS_FRAME_HAS_DIRTY_CHILDREN);
}

NS_IMETHODIMP
nsTableColGroupFrame::RemoveFrame(nsIAtom*        aListName,
                                  nsIFrame*       aOldFrame)
{
  NS_ASSERTION(!aListName, "unexpected child list");

  if (!aOldFrame) return NS_OK;

  if (nsGkAtoms::tableColFrame == aOldFrame->GetType()) {
    nsTableColFrame* colFrame = (nsTableColFrame*)aOldFrame;
    if (colFrame->GetColType() == eColContent) {
      // Remove any anonymous column frames this <col> produced via a colspan
      nsTableColFrame* col = colFrame->GetNextCol();
      nsTableColFrame* nextCol;
      while (col && col->GetColType() == eColAnonymousCol) {
        NS_ASSERTION(col->GetStyleContext() == colFrame->GetStyleContext() &&
                     col->GetContent() == colFrame->GetContent(),
                     "How did that happen??");
        nextCol = col->GetNextCol();
        RemoveFrame(nsnull, col);
        col = nextCol;
      }
    }
    
    PRInt32 colIndex = colFrame->GetColIndex();
    RemoveChild(*colFrame, PR_TRUE);
    
    nsTableFrame* tableFrame = nsTableFrame::GetTableFrame(this);
    if (!tableFrame)
      return NS_ERROR_NULL_POINTER;

    tableFrame->RemoveCol(this, colIndex, PR_TRUE, PR_TRUE);

    PresContext()->PresShell()->FrameNeedsReflow(tableFrame,
                                                 nsIPresShell::eTreeChange,
                                                 NS_FRAME_HAS_DIRTY_CHILDREN);
  }
  else {
    mFrames.DestroyFrame(aOldFrame);
  }

  return NS_OK;
}

PRIntn
nsTableColGroupFrame::GetSkipSides() const
{
  PRIntn skip = 0;
  if (nsnull != GetPrevInFlow()) {
    skip |= 1 << NS_SIDE_TOP;
  }
  if (nsnull != GetNextInFlow()) {
    skip |= 1 << NS_SIDE_BOTTOM;
  }
  return skip;
}

NS_METHOD nsTableColGroupFrame::Reflow(nsPresContext*          aPresContext,
                                       nsHTMLReflowMetrics&     aDesiredSize,
                                       const nsHTMLReflowState& aReflowState,
                                       nsReflowStatus&          aStatus)
{
  DO_GLOBAL_REFLOW_COUNT("nsTableColGroupFrame");
  DISPLAY_REFLOW(aPresContext, this, aReflowState, aDesiredSize, aStatus);
  NS_ASSERTION(nsnull!=mContent, "bad state -- null content for frame");
  nsresult rv=NS_OK;
  
  const nsStyleVisibility* groupVis = GetStyleVisibility();
  PRBool collapseGroup = (NS_STYLE_VISIBILITY_COLLAPSE == groupVis->mVisible);
  if (collapseGroup) {
    nsTableFrame* tableFrame = nsTableFrame::GetTableFrame(this);
    if (tableFrame)  {
      tableFrame->SetNeedToCollapse(PR_TRUE);;
    }
  }
  // for every content child that (is a column thingy and does not already have a frame)
  // create a frame and adjust it's style
  
  for (nsIFrame *kidFrame = mFrames.FirstChild(); kidFrame;
       kidFrame = kidFrame->GetNextSibling()) {
    // Give the child frame a chance to reflow, even though we know it'll have 0 size
    nsHTMLReflowMetrics kidSize;
    nsHTMLReflowState kidReflowState(aPresContext, aReflowState, kidFrame,
                                     nsSize(0,0));

    nsReflowStatus status;
    ReflowChild(kidFrame, aPresContext, kidSize, kidReflowState, 0, 0, 0, status);
    FinishReflowChild(kidFrame, aPresContext, nsnull, kidSize, 0, 0, 0);
  }

  aDesiredSize.width=0;
  aDesiredSize.height=0;
  aStatus = NS_FRAME_COMPLETE;
  NS_FRAME_SET_TRUNCATION(aStatus, aReflowState, aDesiredSize);
  return rv;
}

/* virtual */ PRBool
nsTableColGroupFrame::IsContainingBlock() const
{
  return PR_TRUE;
}

nsTableColFrame * nsTableColGroupFrame::GetFirstColumn()
{
  return GetNextColumn(nsnull);
}

nsTableColFrame * nsTableColGroupFrame::GetNextColumn(nsIFrame *aChildFrame)
{
  nsTableColFrame *result = nsnull;
  nsIFrame *childFrame = aChildFrame;
  if (!childFrame) {
    childFrame = mFrames.FirstChild();
  }
  else {
    childFrame = childFrame->GetNextSibling();
  }
  while (childFrame)
  {
    if (NS_STYLE_DISPLAY_TABLE_COLUMN ==
        childFrame->GetStyleDisplay()->mDisplay)
    {
      result = (nsTableColFrame *)childFrame;
      break;
    }
    childFrame = childFrame->GetNextSibling();
  }
  return result;
}

PRInt32 nsTableColGroupFrame::GetSpan()
{
  return GetStyleTable()->mSpan;
}

void nsTableColGroupFrame::SetContinuousBCBorderWidth(PRUint8     aForSide,
                                                      BCPixelSize aPixelValue)
{
  switch (aForSide) {
    case NS_SIDE_TOP:
      mTopContBorderWidth = aPixelValue;
      return;
    case NS_SIDE_BOTTOM:
      mBottomContBorderWidth = aPixelValue;
      return;
    default:
      NS_ERROR("invalid side arg");
  }
}

void nsTableColGroupFrame::GetContinuousBCBorderWidth(nsMargin& aBorder)
{
  PRInt32 aPixelsToTwips = nsPresContext::AppUnitsPerCSSPixel();
  nsTableFrame* table = nsTableFrame::GetTableFrame(this);
  nsTableColFrame* col = table->GetColFrame(mStartColIndex + mColCount - 1);
  col->GetContinuousBCBorderWidth(aBorder);
  aBorder.top = BC_BORDER_BOTTOM_HALF_COORD(aPixelsToTwips,
                                            mTopContBorderWidth);
  aBorder.bottom = BC_BORDER_TOP_HALF_COORD(aPixelsToTwips,
                                            mBottomContBorderWidth);
  return;
}

/* ----- global methods ----- */

nsIFrame*
NS_NewTableColGroupFrame(nsIPresShell* aPresShell, nsStyleContext* aContext)
{
  return new (aPresShell) nsTableColGroupFrame(aContext);
}

NS_IMETHODIMP
nsTableColGroupFrame::Init(nsIContent*      aContent,
                           nsIFrame*        aParent,
                           nsIFrame*        aPrevInFlow)
{
  nsresult  rv;

  // Let the base class do its processing
  rv = nsHTMLContainerFrame::Init(aContent, aParent, aPrevInFlow);

  // record that children that are ignorable whitespace should be excluded 
  mState |= NS_FRAME_EXCLUDE_IGNORABLE_WHITESPACE;

  return rv;
}

nsIAtom*
nsTableColGroupFrame::GetType() const
{
  return nsGkAtoms::tableColGroupFrame;
}

#ifdef DEBUG
NS_IMETHODIMP
nsTableColGroupFrame::GetFrameName(nsAString& aResult) const
{
  return MakeFrameName(NS_LITERAL_STRING("TableColGroup"), aResult);
}

void nsTableColGroupFrame::Dump(PRInt32 aIndent)
{
  char* indent = new char[aIndent + 1];
  if (!indent) return;
  for (PRInt32 i = 0; i < aIndent + 1; i++) {
    indent[i] = ' ';
  }
  indent[aIndent] = 0;

  printf("%s**START COLGROUP DUMP**\n%s startcolIndex=%d  colcount=%d span=%d coltype=",
    indent, indent, GetStartColumnIndex(),  GetColCount(), GetSpan());
  nsTableColGroupType colType = GetColType();
  switch (colType) {
  case eColGroupContent:
    printf(" content ");
    break;
  case eColGroupAnonymousCol: 
    printf(" anonymous-column  ");
    break;
  case eColGroupAnonymousCell: 
    printf(" anonymous-cell ");
    break;
  }
  // verify the colindices
  PRInt32 j = GetStartColumnIndex();
  nsTableColFrame* col = GetFirstColumn();
  while (col) {
    NS_ASSERTION(j == col->GetColIndex(), "wrong colindex on col frame");
    col = col->GetNextCol();
    j++;
  }
  NS_ASSERTION((j - GetStartColumnIndex()) == GetColCount(),
               "number of cols out of sync");
  printf("\n%s**END COLGROUP DUMP** ", indent);
  delete [] indent;
}
#endif
