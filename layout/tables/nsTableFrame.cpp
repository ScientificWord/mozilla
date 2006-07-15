/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=2 sw=2 et tw=80: */
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
 *   Pierre Phaneuf <pp@ludusdesign.com>
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
#include "nsVoidArray.h"
#include "nsTableFrame.h"
#include "nsIRenderingContext.h"
#include "nsStyleContext.h"
#include "nsStyleConsts.h"
#include "nsIContent.h"
#include "nsCellMap.h"
#include "nsTableCellFrame.h"
#include "nsHTMLParts.h"
#include "nsTableColFrame.h"
#include "nsTableColGroupFrame.h"
#include "nsTableRowFrame.h"
#include "nsTableRowGroupFrame.h"
#include "nsTableOuterFrame.h"
#include "nsTablePainter.h"

#include "BasicTableLayoutStrategy.h"
#include "FixedTableLayoutStrategy.h"

#include "nsPresContext.h"
#include "nsCSSRendering.h"
#include "nsStyleConsts.h"
#include "nsIView.h"
#include "nsHTMLAtoms.h"
#include "nsCSSAnonBoxes.h"
#include "nsHTMLReflowCommand.h"
#include "nsLayoutAtoms.h"
#include "nsIDeviceContext.h"
#include "nsIPresShell.h"
#include "nsIDOMElement.h"
#include "nsIDOMHTMLElement.h"
#include "nsIDOMHTMLBodyElement.h"
#include "nsIScrollableFrame.h"
#include "nsHTMLReflowCommand.h"
#include "nsFrameManager.h"
#include "nsCSSRendering.h"
#include "nsLayoutErrors.h"
#include "nsAutoPtr.h"
#include "nsCSSFrameConstructor.h"
#include "nsStyleSet.h"
#include "nsDisplayList.h"

/********************************************************************************
 ** nsTableReflowState                                                         **
 ********************************************************************************/

struct nsTableReflowState {

  // the real reflow state
  const nsHTMLReflowState& reflowState;

  nsReflowReason reason;

  // The table's available size 
  nsSize availSize;

  // Stationary x-offset
  nscoord x;

  // Running y-offset
  nscoord y;

  // Pointer to the footer in the table
  nsIFrame* footerFrame;

  // The first body section row group frame, i.e. not a header or footer
  nsIFrame* firstBodySection;

  nsTableReflowState(nsPresContext&          aPresContext,
                     const nsHTMLReflowState& aReflowState,
                     nsTableFrame&            aTableFrame,
                     nsReflowReason           aReason,
                     nscoord                  aAvailWidth,
                     nscoord                  aAvailHeight)
    : reflowState(aReflowState)
  {
    Init(aPresContext, aTableFrame, aReason, aAvailWidth, aAvailHeight);
  }

  void Init(nsPresContext& aPresContext,
            nsTableFrame&   aTableFrame,
            nsReflowReason  aReason,
            nscoord         aAvailWidth,
            nscoord         aAvailHeight)
  {
    reason = aReason;

    nsTableFrame* table = (nsTableFrame*)aTableFrame.GetFirstInFlow();
    nsMargin borderPadding = table->GetChildAreaOffset(&reflowState);
    nscoord cellSpacingX = table->GetCellSpacingX();

    x = borderPadding.left + cellSpacingX;
    y = borderPadding.top; //cellspacing added during reflow

    availSize.width  = aAvailWidth;
    if (NS_UNCONSTRAINEDSIZE != availSize.width) {
      availSize.width -= borderPadding.left + borderPadding.right
                         + (2 * cellSpacingX);
      availSize.width = PR_MAX(0, availSize.width);
    }

    availSize.height = aAvailHeight;
    if (NS_UNCONSTRAINEDSIZE != availSize.height) {
      availSize.height -= borderPadding.top + borderPadding.bottom
                          + (2 * table->GetCellSpacingY());
      availSize.height = PR_MAX(0, availSize.height);
    }

    footerFrame      = nsnull;
    firstBodySection = nsnull;
  }

  nsTableReflowState(nsPresContext&          aPresContext,
                     const nsHTMLReflowState& aReflowState,
                     nsTableFrame&            aTableFrame)
    : reflowState(aReflowState)
  {
    Init(aPresContext, aTableFrame, aReflowState.reason, aReflowState.availableWidth, aReflowState.availableHeight);
  }

};

/********************************************************************************
 ** nsTableFrame                                                               **
 ********************************************************************************/
#if defined DEBUG_TABLE_REFLOW_TIMING
static PRInt32 gRflCount = 0;
#endif

struct BCPropertyData
{
  BCPropertyData() { mDamageArea.x = mDamageArea.y = mDamageArea.width = mDamageArea.height =
                     mTopBorderWidth = mRightBorderWidth = mBottomBorderWidth = mLeftBorderWidth = 0; }
  nsRect  mDamageArea;
  BCPixelSize mTopBorderWidth;
  BCPixelSize mRightBorderWidth;
  BCPixelSize mBottomBorderWidth;
  BCPixelSize mLeftBorderWidth;
};

NS_IMETHODIMP 
nsTableFrame::GetParentStyleContextFrame(nsPresContext* aPresContext,
                                         nsIFrame**      aProviderFrame,
                                         PRBool*         aIsChild)
{
  // Since our parent, the table outer frame, returned this frame, we
  // must return whatever our parent would normally have returned.

  NS_PRECONDITION(mParent, "table constructed without outer table");
  return NS_STATIC_CAST(nsFrame*, mParent)->
          DoGetParentStyleContextFrame(aPresContext, aProviderFrame, aIsChild);
}


nsIAtom*
nsTableFrame::GetType() const
{
  return nsLayoutAtoms::tableFrame; 
}


nsTableFrame::nsTableFrame(nsStyleContext* aContext)
  : nsHTMLContainerFrame(aContext),
    mCellMap(nsnull),
    mTableLayoutStrategy(nsnull),
    mPreferredWidth(0)
{
  mBits.mHadInitialReflow       = PR_FALSE;
  mBits.mHaveReflowedColGroups  = PR_FALSE;
  mBits.mNeedStrategyInit       = PR_TRUE;
  mBits.mNeedStrategyBalance    = PR_TRUE;
  mBits.mCellSpansPctCol        = PR_FALSE;
  mBits.mNeedToCalcBCBorders    = PR_FALSE;
  mBits.mIsBorderCollapse       = PR_FALSE;

#ifdef DEBUG_TABLE_REFLOW_TIMING
  mTimer = new nsReflowTimer(this);
  nsReflowTimer* timer = new nsReflowTimer(this);
  mTimer->mNextSibling = timer;
  timer = new nsReflowTimer(this);
  mTimer->mNextSibling->mNextSibling = timer;
  timer = new nsReflowTimer(this);
  mTimer->mNextSibling->mNextSibling->mNextSibling = timer;
  timer = new nsReflowTimer(this);
  mTimer->mNextSibling->mNextSibling->mNextSibling->mNextSibling = timer;
  timer = new nsReflowTimer(this);
  mTimer->mNextSibling->mNextSibling->mNextSibling->mNextSibling->mNextSibling = timer;
#endif
}

NS_IMPL_ADDREF_INHERITED(nsTableFrame, nsHTMLContainerFrame)
NS_IMPL_RELEASE_INHERITED(nsTableFrame, nsHTMLContainerFrame)

nsresult nsTableFrame::QueryInterface(const nsIID& aIID, void** aInstancePtr)
{
  if (NULL == aInstancePtr) {
    return NS_ERROR_NULL_POINTER;
  }
  if (aIID.Equals(NS_GET_IID(nsITableLayout))) 
  { // note there is no addref here, frames are not addref'd
    *aInstancePtr = (void*)(nsITableLayout*)this;
    return NS_OK;
  }
  else {
    return nsHTMLContainerFrame::QueryInterface(aIID, aInstancePtr);
  }
}

NS_IMETHODIMP
nsTableFrame::Init(nsIContent*      aContent,
                   nsIFrame*        aParent,
                   nsIFrame*        aPrevInFlow)
{
  nsresult  rv;

  // Let the base class do its processing
  rv = nsHTMLContainerFrame::Init(aContent, aParent, aPrevInFlow);

  // record that children that are ignorable whitespace should be excluded 
  mState |= NS_FRAME_EXCLUDE_IGNORABLE_WHITESPACE;

  // see if border collapse is on, if so set it
  const nsStyleTableBorder* tableStyle = GetStyleTableBorder();
  PRBool borderCollapse = (NS_STYLE_BORDER_COLLAPSE == tableStyle->mBorderCollapse);
  SetBorderCollapse(borderCollapse);
  // Create the cell map
  // XXX Why do we do this for continuing frames?
  mCellMap = new nsTableCellMap(*this, borderCollapse);
  if (!mCellMap) return NS_ERROR_OUT_OF_MEMORY;

  if (aPrevInFlow) {
    // set my width, because all frames in a table flow are the same width and
    // code in nsTableOuterFrame depends on this being set
    mRect.width = aPrevInFlow->GetSize().width;
  }
  else {
    NS_ASSERTION(!mTableLayoutStrategy, "strategy was created before Init was called");
    // create the strategy
    mTableLayoutStrategy = (IsAutoLayout()) ?
      new BasicTableLayoutStrategy(this,
                eCompatibility_NavQuirks == GetPresContext()->CompatibilityMode())
      : new FixedTableLayoutStrategy(this);
  }

  return rv;
}


nsTableFrame::~nsTableFrame()
{
  if (nsnull!=mCellMap) {
    delete mCellMap; 
    mCellMap = nsnull;
  }

  if (nsnull!=mTableLayoutStrategy) {
    delete mTableLayoutStrategy;
    mTableLayoutStrategy = nsnull;
  }
#ifdef DEBUG_TABLE_REFLOW_TIMING
  nsTableFrame::DebugReflowDone(this);
#endif
}

void
nsTableFrame::Destroy()
{
  mColGroups.DestroyFrames();
  nsHTMLContainerFrame::Destroy();
}

nscoord 
nsTableFrame::RoundToPixel(nscoord       aValue,
                           float         aPixelToTwips,
                           nsPixelRound  aRound)
{
  nscoord fullPixel = NSToCoordRound(aPixelToTwips);
  if (fullPixel <= 0) 
    // We must be rendering to a device that has a resolution greater than Twips! 
    // In that case, aValue is as accurate as it's going to get.
    return aValue;
  
  PRInt32 excess = aValue % fullPixel;
  if (0 == excess) 
    return aValue;

  nscoord halfPixel = NSToCoordRound(aPixelToTwips / 2.0f);
  switch(aRound) {
  case eRoundUpIfHalfOrMore:
    if (excess >= halfPixel) { // eRoundUpIfHalfOrMore
      return aValue + (fullPixel - excess);
    }
  case eAlwaysRoundDown:
    return aValue - excess;
  default: // eAlwaysRoundUp
    return aValue + (fullPixel - excess);
  }
}

// Helper function which marks aFrame as dirty and generates a reflow command
nsresult
nsTableFrame::AppendDirtyReflowCommand(nsIFrame* aFrame)
{
  aFrame->AddStateBits(NS_FRAME_IS_DIRTY);  // mark the table frame as dirty

  return aFrame->GetPresContext()->PresShell()->
          AppendReflowCommand(aFrame, eReflowType_ReflowDirty, nsnull);
}

// Make sure any views are positioned properly
void
nsTableFrame::RePositionViews(nsIFrame* aFrame)
{
  nsContainerFrame::PositionFrameView(aFrame);
  nsContainerFrame::PositionChildViews(aFrame);
}

static PRBool
IsRepeatedFrame(nsIFrame* kidFrame)
{
  return (kidFrame->GetType() == nsLayoutAtoms::tableRowFrame ||
          kidFrame->GetType() == nsLayoutAtoms::tableRowGroupFrame) &&
         (kidFrame->GetStateBits() & NS_REPEATED_ROW_OR_ROWGROUP);
}

PRBool
nsTableFrame::PageBreakAfter(nsIFrame& aSourceFrame,
                             nsIFrame* aNextFrame)
{
  const nsStyleDisplay* display = aSourceFrame.GetStyleDisplay();
  // don't allow a page break after a repeated element ...
  if (display->mBreakAfter && !IsRepeatedFrame(&aSourceFrame)) {
    return !(aNextFrame && IsRepeatedFrame(aNextFrame)); // or before
  }

  if (aNextFrame) {
    display = aNextFrame->GetStyleDisplay();
    // don't allow a page break before a repeated element ...
    if (display->mBreakBefore && !IsRepeatedFrame(aNextFrame)) {
      return !IsRepeatedFrame(&aSourceFrame); // or after
    }
  }
  return PR_FALSE;
}

// XXX this needs to be cleaned up so that the frame constructor breaks out col group
// frames into a separate child list, bug 343048.
NS_IMETHODIMP
nsTableFrame::SetInitialChildList(nsIAtom*        aListName,
                                  nsIFrame*       aChildList)
{

  if (!mFrames.IsEmpty() || !mColGroups.IsEmpty()) {
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
  
  nsIFrame *childFrame = aChildList;
  nsIFrame *prevMainChild = nsnull;
  nsIFrame *prevColGroupChild = nsnull;
  for ( ; nsnull!=childFrame; )
  {
    const nsStyleDisplay* childDisplay = childFrame->GetStyleDisplay();
    // XXX this if should go away
    if (PR_TRUE==IsRowGroup(childDisplay->mDisplay))
    {
      if (mFrames.IsEmpty()) 
        mFrames.SetFrames(childFrame);
      else
        prevMainChild->SetNextSibling(childFrame);
      prevMainChild = childFrame;
    }
    else if (NS_STYLE_DISPLAY_TABLE_COLUMN_GROUP == childDisplay->mDisplay)
    {
      NS_ASSERTION(nsLayoutAtoms::tableColGroupFrame == childFrame->GetType(),
                   "This is not a colgroup");
      if (mColGroups.IsEmpty())
        mColGroups.SetFrames(childFrame);
      else
        prevColGroupChild->SetNextSibling(childFrame);
      prevColGroupChild = childFrame;
    }
    else
    { // unknown frames go on the main list for now
      if (mFrames.IsEmpty())
        mFrames.SetFrames(childFrame);
      else
        prevMainChild->SetNextSibling(childFrame);
      prevMainChild = childFrame;
    }
    nsIFrame *prevChild = childFrame;
    childFrame = childFrame->GetNextSibling();
    prevChild->SetNextSibling(nsnull);
  }
  if (nsnull!=prevMainChild)
    prevMainChild->SetNextSibling(nsnull);
  if (nsnull!=prevColGroupChild)
    prevColGroupChild->SetNextSibling(nsnull);

  // If we have a prev-in-flow, then we're a table that has been split and
  // so don't treat this like an append
  if (!GetPrevInFlow()) {
    // process col groups first so that real cols get constructed before
    // anonymous ones due to cells in rows.
    InsertColGroups(0, mColGroups.FirstChild());
    AppendRowGroups(mFrames.FirstChild());
    // calc collapsing borders if this is the default (row group, col group, child list)
    if (!aChildList && IsBorderCollapse()) {
      nsRect damageArea(0, 0, GetColCount(), GetRowCount());
      SetBCDamageArea(damageArea);
    }
  }

  return NS_OK;
}

/* virtual */ PRBool
nsTableFrame::IsContainingBlock() const
{
  return PR_TRUE;
}

void nsTableFrame::AttributeChangedFor(nsIFrame*       aFrame,
                                       nsIContent*     aContent, 
                                       nsIAtom*        aAttribute)
{
  if (IS_TABLE_CELL(aFrame->GetType())) {
    if ((nsHTMLAtoms::rowspan == aAttribute) || 
        (nsHTMLAtoms::colspan == aAttribute)) {
      nsTableCellMap* cellMap = GetCellMap();
      if (cellMap) {
        // for now just remove the cell from the map and reinsert it
        nsTableCellFrame* cellFrame = (nsTableCellFrame*)aFrame;
        PRInt32 rowIndex, colIndex;
        cellFrame->GetRowIndex(rowIndex);
        cellFrame->GetColIndex(colIndex);
        RemoveCell(cellFrame, rowIndex);
        nsAutoVoidArray cells;
        cells.AppendElement(cellFrame);
        InsertCells(cells, rowIndex, colIndex - 1);

        // XXX This could probably be optimized with some effort
        SetNeedStrategyInit(PR_TRUE);
        AppendDirtyReflowCommand(this);
      }
    }
  }
}


/* ****** CellMap methods ******* */

PRInt32 nsTableFrame::GetRowCount () const
{
  PRInt32 rowCount = 0;
  nsTableCellMap *cellMap = GetCellMap();
  NS_ASSERTION(nsnull!=cellMap, "GetRowCount null cellmap");
  if (nsnull!=cellMap)
    rowCount = cellMap->GetRowCount();
  return rowCount;
}

/* return the col count including dead cols */
PRInt32 nsTableFrame::GetColCount () const
{
  PRInt32 colCount = 0;
  nsTableCellMap* cellMap = GetCellMap();
  NS_ASSERTION(nsnull != cellMap, "GetColCount null cellmap");
  if (nsnull != cellMap) {
    colCount = cellMap->GetColCount();
  }
  return colCount;
}

/* return the effective col count */
PRInt32 nsTableFrame::GetEffectiveColCount() const
{
  PRInt32 colCount = GetColCount();
  // don't count cols at the end that don't have originating cells
  for (PRInt32 colX = colCount - 1; colX >= 0; colX--) {
    if (GetNumCellsOriginatingInCol(colX) <= 0) { 
      colCount--;
    }
    else break;
  }
  return colCount;
}

PRInt32 nsTableFrame::GetIndexOfLastRealCol()
{
  PRInt32 numCols = mColFrames.Count();
  for (PRInt32 colX = numCols; colX >= 0; colX--) { 
    nsTableColFrame* colFrame = GetColFrame(colX);
    if (colFrame) {
      if (eColAnonymousCell != colFrame->GetColType()) {
        return colX;
      }
    }
  }
  return -1; 
}

nsTableColFrame*
nsTableFrame::GetColFrame(PRInt32 aColIndex) const
{
  NS_ASSERTION(!GetPrevInFlow(), "GetColFrame called on next in flow");
  PRInt32 numCols = mColFrames.Count();
  if ((aColIndex >= 0) && (aColIndex < numCols)) {
    return (nsTableColFrame *)mColFrames.ElementAt(aColIndex);
  }
  else {
    //NS_ASSERTION(PR_FALSE, "invalid col index");
    return nsnull;
  }
}

// can return nsnull
nsTableCellFrame* nsTableFrame::GetCellFrameAt(PRInt32 aRowIndex, PRInt32 aColIndex)
{
  nsTableCellMap* cellMap = GetCellMap();
  if (cellMap) 
    return cellMap->GetCellInfoAt(aRowIndex, aColIndex);
  return nsnull;
}


PRInt32 nsTableFrame::GetEffectiveRowSpan(PRInt32                 aRowIndex,
                                          const nsTableCellFrame& aCell) const
{
  nsTableCellMap* cellMap = GetCellMap();
  NS_PRECONDITION (nsnull != cellMap, "bad call, cellMap not yet allocated.");

  PRInt32 colIndex;
  aCell.GetColIndex(colIndex);
  return cellMap->GetEffectiveRowSpan(aRowIndex, colIndex);
}

PRInt32 nsTableFrame::GetEffectiveRowSpan(const nsTableCellFrame& aCell,
                                          nsCellMap*              aCellMap)
{
  nsTableCellMap* tableCellMap = GetCellMap(); if (!tableCellMap) ABORT1(1);

  PRInt32 colIndex, rowIndex;
  aCell.GetColIndex(colIndex);
  aCell.GetRowIndex(rowIndex);
  PRBool ignore;

  if (aCellMap) 
    return aCellMap->GetRowSpan(*tableCellMap, rowIndex, colIndex, PR_TRUE, ignore);
  else
    return tableCellMap->GetEffectiveRowSpan(rowIndex, colIndex);
}

PRInt32 nsTableFrame::GetEffectiveColSpan(const nsTableCellFrame& aCell,
                                          nsCellMap*              aCellMap) const
{
  nsTableCellMap* tableCellMap = GetCellMap(); if (!tableCellMap) ABORT1(1);

  PRInt32 colIndex, rowIndex;
  aCell.GetColIndex(colIndex);
  aCell.GetRowIndex(rowIndex);
  PRBool ignore;

  if (aCellMap) 
    return aCellMap->GetEffectiveColSpan(*tableCellMap, rowIndex, colIndex, ignore);
  else
    return tableCellMap->GetEffectiveColSpan(rowIndex, colIndex);
}

PRBool nsTableFrame::HasMoreThanOneCell(PRInt32 aRowIndex) const
{
  nsTableCellMap* tableCellMap = GetCellMap(); if (!tableCellMap) ABORT1(1);
  return tableCellMap->HasMoreThanOneCell(aRowIndex);
}

PRInt32 nsTableFrame::GetEffectiveCOLSAttribute()
{
  NS_PRECONDITION (GetCellMap(), "null cellMap.");

  PRInt32 result;
  result = GetStyleTable()->mCols;
  PRInt32 numCols = GetColCount();
  if (result > numCols)
    result = numCols;
  return result;
}

void nsTableFrame::AdjustRowIndices(PRInt32         aRowIndex,
                                    PRInt32         aAdjustment)
{
  // Iterate over the row groups and adjust the row indices of all rows 
  // whose index is >= aRowIndex.
  nsAutoVoidArray rowGroups;
  PRUint32 numRowGroups;
  OrderRowGroups(rowGroups, numRowGroups, nsnull);

  for (PRUint32 rgX = 0; rgX < numRowGroups; rgX++) {
    nsIFrame* kidFrame = (nsIFrame*)rowGroups.ElementAt(rgX);
    nsTableRowGroupFrame* rgFrame = GetRowGroupFrame(kidFrame);
    AdjustRowIndices(rgFrame, aRowIndex, aAdjustment);
  }
}

NS_IMETHODIMP nsTableFrame::AdjustRowIndices(nsIFrame*       aRowGroup,
                                             PRInt32         aRowIndex,
                                             PRInt32         anAdjustment)
{
  nsresult rv = NS_OK;
  nsIFrame* rowFrame = aRowGroup->GetFirstChild(nsnull);
  for ( ; rowFrame; rowFrame = rowFrame->GetNextSibling()) {
    if (NS_STYLE_DISPLAY_TABLE_ROW==rowFrame->GetStyleDisplay()->mDisplay) {
      PRInt32 index = ((nsTableRowFrame*)rowFrame)->GetRowIndex();
      if (index >= aRowIndex)
        ((nsTableRowFrame *)rowFrame)->SetRowIndex(index+anAdjustment);
    }
  }
  return rv;
}

void nsTableFrame::ResetRowIndices(void)
{
  // Iterate over the row groups and adjust the row indices of all rows 
  nsAutoVoidArray rowGroups;
  PRUint32 numRowGroups;
  OrderRowGroups(rowGroups, numRowGroups, nsnull);

  PRInt32 rowIndex = 0;
  for (PRUint32 rgX = 0; rgX < numRowGroups; rgX++) {
    nsIFrame* kidFrame = (nsIFrame*)rowGroups.ElementAt(rgX);
    nsTableRowGroupFrame* rgFrame = GetRowGroupFrame(kidFrame);
    nsIFrame* rowFrame = rgFrame->GetFirstChild(nsnull);
    for ( ; rowFrame; rowFrame = rowFrame->GetNextSibling()) {
      if (NS_STYLE_DISPLAY_TABLE_ROW==rowFrame->GetStyleDisplay()->mDisplay) {
        ((nsTableRowFrame *)rowFrame)->SetRowIndex(rowIndex);
        rowIndex++;
      }
    }
  }
}
void nsTableFrame::InsertColGroups(PRInt32         aStartColIndex,
                                   nsIFrame*       aFirstFrame,
                                   nsIFrame*       aLastFrame)
{
  PRInt32 colIndex = aStartColIndex;
  nsTableColGroupFrame* firstColGroupToReset = nsnull;
  nsIFrame* kidFrame = aFirstFrame;
  PRBool didLastFrame = PR_FALSE;
  while (kidFrame) {
    if (nsLayoutAtoms::tableColGroupFrame == kidFrame->GetType()) {
      if (didLastFrame) {
        firstColGroupToReset = (nsTableColGroupFrame*)kidFrame;
        break;
      }
      else {
        nsTableColGroupFrame* cgFrame = (nsTableColGroupFrame*)kidFrame;
        cgFrame->SetStartColumnIndex(colIndex);
        nsIFrame* firstCol = kidFrame->GetFirstChild(nsnull);
        cgFrame->AddColsToTable(colIndex, PR_FALSE, firstCol);
        PRInt32 numCols = cgFrame->GetColCount();
        colIndex += numCols;
      }
    }
    if (kidFrame == aLastFrame) {
      didLastFrame = PR_TRUE;
    }
    kidFrame = kidFrame->GetNextSibling();
  }

  if (firstColGroupToReset) {
    nsTableColGroupFrame::ResetColIndices(firstColGroupToReset, colIndex);
  }
}

void nsTableFrame::InsertCol(nsTableColFrame& aColFrame,
                             PRInt32          aColIndex)
{
  mColFrames.InsertElementAt(&aColFrame, aColIndex);
  nsTableColType insertedColType = aColFrame.GetColType();
  PRInt32 numCacheCols = mColFrames.Count();
  nsTableCellMap* cellMap = GetCellMap();
  if (cellMap) {
    PRInt32 numMapCols = cellMap->GetColCount();
    if (numCacheCols > numMapCols) {
      PRBool removedFromCache = PR_FALSE;
      if (eColAnonymousCell != insertedColType) {
        nsTableColFrame* lastCol = (nsTableColFrame *)mColFrames.ElementAt(numCacheCols - 1);
        if (lastCol) {
          nsTableColType lastColType = lastCol->GetColType();
          if (eColAnonymousCell == lastColType) {
            // remove the col from the cache
            mColFrames.RemoveElementAt(numCacheCols - 1);
            // remove the col from the eColGroupAnonymousCell col group
            nsTableColGroupFrame* lastColGroup = (nsTableColGroupFrame *)mColGroups.LastChild();
            if (lastColGroup) {
              lastColGroup->RemoveChild(*lastCol, PR_FALSE);
            }
            // remove the col group if it is empty
            if (lastColGroup->GetColCount() <= 0) {
              mColGroups.DestroyFrame((nsIFrame*)lastColGroup);
            }
            removedFromCache = PR_TRUE;
          }
        }
      }
      if (!removedFromCache) {
        cellMap->AddColsAtEnd(1);
      }
    }
  }
  // for now, just bail and recalc all of the collapsing borders
  if (IsBorderCollapse()) {
    nsRect damageArea(0, 0, PR_MAX(1, GetColCount()), PR_MAX(1, GetRowCount()));
    SetBCDamageArea(damageArea);
  }
}

void nsTableFrame::RemoveCol(nsTableColGroupFrame* aColGroupFrame,
                             PRInt32               aColIndex,
                             PRBool                aRemoveFromCache,
                             PRBool                aRemoveFromCellMap)
{
  if (aRemoveFromCache) {
    mColFrames.RemoveElementAt(aColIndex);
  }
  if (aRemoveFromCellMap) {
    nsTableCellMap* cellMap = GetCellMap();
    if (cellMap) {
      CreateAnonymousColFrames(1, eColAnonymousCell, PR_TRUE);
    }
  }
  // for now, just bail and recalc all of the collapsing borders
  if (IsBorderCollapse()) {
    nsRect damageArea(0, 0, GetColCount(), GetRowCount());
    SetBCDamageArea(damageArea);
  }
}

/** Get the cell map for this table frame.  It is not always mCellMap.
  * Only the firstInFlow has a legit cell map
  */
nsTableCellMap* nsTableFrame::GetCellMap() const
{
  nsTableFrame* firstInFlow = (nsTableFrame *)GetFirstInFlow();
  if (this == firstInFlow) {
    return mCellMap;
  }
  else {
    return firstInFlow->GetCellMap();
  }
}

nscoord nsTableFrame::GetMinWidth() const
{
  nsTableFrame* firstInFlow = (nsTableFrame *)GetFirstInFlow();
  if (this == firstInFlow) {
    return mMinWidth;
  }
  else {
    return firstInFlow->GetMinWidth();
  }
}

nscoord nsTableFrame::GetDesiredWidth() const
{
  nsTableFrame* firstInFlow = (nsTableFrame *)GetFirstInFlow();
  if (this == firstInFlow) {
    return mDesiredWidth;
  }
  else {
    return firstInFlow->GetDesiredWidth();
  }
}

nscoord nsTableFrame::GetPreferredWidth() const
{
  nsTableFrame* firstInFlow = (nsTableFrame *)GetFirstInFlow();
  if (this == firstInFlow) {
    return mPreferredWidth;
  }
  else {
    return firstInFlow->GetPreferredWidth();
  }
}

// XXX this needs to be moved to nsCSSFrameConstructor
nsTableColGroupFrame*
nsTableFrame::CreateAnonymousColGroupFrame(nsTableColGroupType aColGroupType)
{
  nsIContent* colGroupContent = GetContent();
  nsPresContext* presContext = GetPresContext();
  nsIPresShell *shell = presContext->PresShell();

  nsRefPtr<nsStyleContext> colGroupStyle;
  colGroupStyle = shell->StyleSet()->ResolvePseudoStyleFor(colGroupContent,
                                                           nsCSSAnonBoxes::tableColGroup,
                                                           mStyleContext);
  // Create a col group frame
  nsIFrame* newFrame = NS_NewTableColGroupFrame(shell, colGroupStyle);
  if (newFrame) {
    ((nsTableColGroupFrame *)newFrame)->SetColType(aColGroupType);
    newFrame->Init(colGroupContent, this, nsnull);
  }
  return (nsTableColGroupFrame *)newFrame;
}

void
nsTableFrame::CreateAnonymousColFrames(PRInt32         aNumColsToAdd,
                                       nsTableColType  aColType,
                                       PRBool          aDoAppend,
                                       nsIFrame*       aPrevColIn)
{
  // get the last col group frame
  nsTableColGroupFrame* colGroupFrame = nsnull;
  nsIFrame* childFrame = mColGroups.FirstChild();
  while (childFrame) {
    if (nsLayoutAtoms::tableColGroupFrame == childFrame->GetType()) {
      colGroupFrame = (nsTableColGroupFrame *)childFrame;
    }
    childFrame = childFrame->GetNextSibling();
  }

  nsTableColGroupType lastColGroupType = eColGroupContent; 
  nsTableColGroupType newColGroupType  = eColGroupContent; 
  if (colGroupFrame) {
    lastColGroupType = colGroupFrame->GetColType();
  }
  if (eColAnonymousCell == aColType) {
    if (eColGroupAnonymousCell != lastColGroupType) {
      newColGroupType = eColGroupAnonymousCell;
    }
  }
  else if (eColAnonymousCol == aColType) {
    if (eColGroupAnonymousCol != lastColGroupType) {
      newColGroupType = eColGroupAnonymousCol;
    }
  }
  else {
    NS_ASSERTION(PR_FALSE, "CreateAnonymousColFrames called incorrectly");
    return;
  }

  if (eColGroupContent != newColGroupType) {
    PRInt32 colIndex = (colGroupFrame) ? colGroupFrame->GetStartColumnIndex() + colGroupFrame->GetColCount()
                                       : 0;
    colGroupFrame = CreateAnonymousColGroupFrame(newColGroupType);
    if (!colGroupFrame) {
      return;
    }
    mColGroups.AppendFrame(this, colGroupFrame); // add the new frame to the child list
    colGroupFrame->SetStartColumnIndex(colIndex);
  }

  nsIFrame* prevCol = (aDoAppend) ? colGroupFrame->GetChildList().LastChild() : aPrevColIn;

  nsIFrame* firstNewFrame;
  CreateAnonymousColFrames(colGroupFrame, aNumColsToAdd, aColType,
                           PR_TRUE, prevCol, &firstNewFrame);
}

// XXX this needs to be moved to nsCSSFrameConstructor
// Right now it only creates the col frames at the end 
void
nsTableFrame::CreateAnonymousColFrames(nsTableColGroupFrame* aColGroupFrame,
                                       PRInt32               aNumColsToAdd,
                                       nsTableColType        aColType,
                                       PRBool                aAddToColGroupAndTable,         
                                       nsIFrame*             aPrevFrameIn,
                                       nsIFrame**            aFirstNewFrame)
{
  NS_PRECONDITION(aColGroupFrame, "null frame");
  *aFirstNewFrame = nsnull;
  nsIFrame* lastColFrame = nsnull;
  nsPresContext* presContext = GetPresContext();
  nsIPresShell *shell = presContext->PresShell();

  // Get the last col frame
  nsIFrame* childFrame = aColGroupFrame->GetFirstChild(nsnull);
  while (childFrame) {
    if (nsLayoutAtoms::tableColFrame == childFrame->GetType()) {
      lastColFrame = (nsTableColGroupFrame *)childFrame;
    }
    childFrame = childFrame->GetNextSibling();
  }

  PRInt32 startIndex = mColFrames.Count();
  PRInt32 lastIndex  = startIndex + aNumColsToAdd - 1; 

  for (PRInt32 childX = startIndex; childX <= lastIndex; childX++) {
    nsIContent* iContent;
    nsRefPtr<nsStyleContext> styleContext;
    nsStyleContext* parentStyleContext;

    if ((aColType == eColAnonymousCol) && aPrevFrameIn) {
      // a col due to a span in a previous col uses the style context of the col
      styleContext = aPrevFrameIn->GetStyleContext();
      // fix for bugzilla bug 54454: get the content from the prevFrame 
      iContent = aPrevFrameIn->GetContent();
    }
    else {
      // all other anonymous cols use a pseudo style context of the col group
      iContent = aColGroupFrame->GetContent();
      parentStyleContext = aColGroupFrame->GetStyleContext();
      styleContext = shell->StyleSet()->ResolvePseudoStyleFor(iContent,
                                                              nsCSSAnonBoxes::tableCol,
                                                              parentStyleContext);
    }
    // ASSERTION to check for bug 54454 sneaking back in...
    NS_ASSERTION(iContent, "null content in CreateAnonymousColFrames");

    // create the new col frame
    nsIFrame* colFrame = NS_NewTableColFrame(shell, styleContext);
    ((nsTableColFrame *) colFrame)->SetColType(aColType);
    colFrame->Init(iContent, aColGroupFrame, nsnull);
    colFrame->SetInitialChildList(nsnull, nsnull);

    // Add the col to the sibling chain
    if (lastColFrame) {
      lastColFrame->SetNextSibling(colFrame);
    }
    lastColFrame = colFrame;
    if (childX == startIndex) {
      *aFirstNewFrame = colFrame;
    }
  }
  if (aAddToColGroupAndTable) {
    nsFrameList& cols = aColGroupFrame->GetChildList();
    // the chain already exists, now add it to the col group child list
    if (!aPrevFrameIn) {
      cols.AppendFrames(aColGroupFrame, *aFirstNewFrame);
    }
    // get the starting col index in the cache
    PRInt32 startColIndex = aColGroupFrame->GetStartColumnIndex();
    if (aPrevFrameIn) {
      nsTableColFrame* colFrame = 
        (nsTableColFrame*)nsTableFrame::GetFrameAtOrBefore((nsIFrame*) aColGroupFrame, aPrevFrameIn, 
                                                           nsLayoutAtoms::tableColFrame);
      if (colFrame) {
        startColIndex = colFrame->GetColIndex() + 1;
      }
    }
    aColGroupFrame->AddColsToTable(startColIndex, PR_TRUE, 
                                  *aFirstNewFrame, lastColFrame);
  }
}

void
nsTableFrame::AppendCell(nsTableCellFrame& aCellFrame,
                         PRInt32           aRowIndex)
{
  nsTableCellMap* cellMap = GetCellMap();
  if (cellMap) {
    nsRect damageArea(0,0,0,0);
    cellMap->AppendCell(aCellFrame, aRowIndex, PR_TRUE, damageArea);
    PRInt32 numColsInMap   = GetColCount();
    PRInt32 numColsInCache = mColFrames.Count();
    PRInt32 numColsToAdd = numColsInMap - numColsInCache;
    if (numColsToAdd > 0) {
      // this sets the child list, updates the col cache and cell map
      CreateAnonymousColFrames(numColsToAdd, eColAnonymousCell, PR_TRUE); 
    }
    if (IsBorderCollapse()) {
      SetBCDamageArea(damageArea);
    }
  }
}

void nsTableFrame::InsertCells(nsVoidArray&    aCellFrames, 
                               PRInt32         aRowIndex, 
                               PRInt32         aColIndexBefore)
{
  nsTableCellMap* cellMap = GetCellMap();
  if (cellMap) {
    nsRect damageArea(0,0,0,0);
    cellMap->InsertCells(aCellFrames, aRowIndex, aColIndexBefore, damageArea);
    PRInt32 numColsInMap = GetColCount();
    PRInt32 numColsInCache = mColFrames.Count();
    PRInt32 numColsToAdd = numColsInMap - numColsInCache;
    if (numColsToAdd > 0) {
      // this sets the child list, updates the col cache and cell map
      CreateAnonymousColFrames(numColsToAdd, eColAnonymousCell, PR_TRUE);
    }
    if (IsBorderCollapse()) {
      SetBCDamageArea(damageArea);
    }
  }
}

// this removes the frames from the col group and table, but not the cell map
PRInt32 
nsTableFrame::DestroyAnonymousColFrames(PRInt32 aNumFrames)
{
  // only remove cols that are of type eTypeAnonymous cell (they are at the end)
  PRInt32 endIndex   = mColFrames.Count() - 1;
  PRInt32 startIndex = (endIndex - aNumFrames) + 1;
  PRInt32 numColsRemoved = 0;
  for (PRInt32 colX = endIndex; colX >= startIndex; colX--) {
    nsTableColFrame* colFrame = GetColFrame(colX);
    if (colFrame && (eColAnonymousCell == colFrame->GetColType())) {
      nsTableColGroupFrame* cgFrame =
        NS_STATIC_CAST(nsTableColGroupFrame*, colFrame->GetParent());
      // remove the frame from the colgroup
      cgFrame->RemoveChild(*colFrame, PR_FALSE);
      // remove the frame from the cache, but not the cell map 
      RemoveCol(nsnull, colX, PR_TRUE, PR_FALSE);
      numColsRemoved++;
    }
    else {
      break; 
    }
  }
  return (aNumFrames - numColsRemoved);
}

void nsTableFrame::RemoveCell(nsTableCellFrame* aCellFrame,
                              PRInt32           aRowIndex)
{
  nsTableCellMap* cellMap = GetCellMap();
  if (cellMap) {
    nsRect damageArea(0,0,0,0);
    cellMap->RemoveCell(aCellFrame, aRowIndex, damageArea);
    PRInt32 numColsInMap = GetColCount(); // cell map's notion of num cols
    PRInt32 numColsInCache = mColFrames.Count();
    if (numColsInCache > numColsInMap) {
      PRInt32 numColsNotRemoved = DestroyAnonymousColFrames(numColsInCache - numColsInMap);
      // if the cell map has fewer cols than the cache, correct it
      if (numColsNotRemoved > 0) {
        cellMap->AddColsAtEnd(numColsNotRemoved);
      }
    }
    else NS_ASSERTION(numColsInCache == numColsInMap, "cell map has too many cols");

    if (IsBorderCollapse()) {
      SetBCDamageArea(damageArea);
    }
  }
}

PRInt32
nsTableFrame::GetStartRowIndex(nsTableRowGroupFrame& aRowGroupFrame)
{
  nsAutoVoidArray orderedRowGroups;
  PRUint32 numRowGroups;
  OrderRowGroups(orderedRowGroups, numRowGroups);

  PRInt32 rowIndex = 0;
  for (PRUint32 rgIndex = 0; rgIndex < numRowGroups; rgIndex++) {
    nsTableRowGroupFrame* rgFrame = GetRowGroupFrame((nsIFrame*)orderedRowGroups.ElementAt(rgIndex));
    if (rgFrame == &aRowGroupFrame) {
      break;
    }
    PRInt32 numRows = rgFrame->GetRowCount();
    rowIndex += numRows;
  }
  return rowIndex;
}

// this cannot extend beyond a single row group
void nsTableFrame::AppendRows(nsTableRowGroupFrame& aRowGroupFrame,
                              PRInt32               aRowIndex,
                              nsVoidArray&          aRowFrames)
{
  nsTableCellMap* cellMap = GetCellMap();
  if (cellMap) {
    PRInt32 absRowIndex = GetStartRowIndex(aRowGroupFrame) + aRowIndex;
    InsertRows(aRowGroupFrame, aRowFrames, absRowIndex, PR_TRUE);
  }
}

PRInt32
nsTableFrame::InsertRow(nsTableRowGroupFrame& aRowGroupFrame,
                        nsIFrame&             aRowFrame,
                        PRInt32               aRowIndex,
                        PRBool                aConsiderSpans)
{
  nsAutoVoidArray rows;
  rows.AppendElement(&aRowFrame);
  return InsertRows(aRowGroupFrame, rows, aRowIndex, aConsiderSpans);
}

// this cannot extend beyond a single row group
PRInt32
nsTableFrame::InsertRows(nsTableRowGroupFrame& aRowGroupFrame,
                         nsVoidArray&          aRowFrames,
                         PRInt32               aRowIndex,
                         PRBool                aConsiderSpans)
{
#ifdef DEBUG_TABLE_CELLMAP
  printf("=== insertRowsBefore firstRow=%d \n", aRowIndex);
  Dump(PR_TRUE, PR_FALSE, PR_TRUE);
#endif

  PRInt32 numColsToAdd = 0;
  nsTableCellMap* cellMap = GetCellMap();
  if (cellMap) {
    nsRect damageArea(0,0,0,0);
    PRInt32 origNumRows = cellMap->GetRowCount();
    PRInt32 numNewRows = aRowFrames.Count();
    cellMap->InsertRows(aRowGroupFrame, aRowFrames, aRowIndex, aConsiderSpans, damageArea);
    PRInt32 numColsInMap = GetColCount(); // cell map's notion of num cols
    PRInt32 numColsInCache = mColFrames.Count();
    numColsToAdd = numColsInMap - numColsInCache;
    if (numColsToAdd > 0) {
      // this sets the child list, updates the col cache and cell map
      CreateAnonymousColFrames(numColsToAdd, eColAnonymousCell, PR_TRUE);
    }
    if (aRowIndex < origNumRows) {
      AdjustRowIndices(aRowIndex, numNewRows);
    }
    // assign the correct row indices to the new rows. If they were adjusted above
    // it may not have been done correctly because each row is constructed with index 0
    for (PRInt32 rowX = 0; rowX < numNewRows; rowX++) {
      nsTableRowFrame* rowFrame = (nsTableRowFrame *) aRowFrames.ElementAt(rowX);
      rowFrame->SetRowIndex(aRowIndex + rowX);
    }
    if (IsBorderCollapse()) {
      SetBCDamageArea(damageArea);
    }
  }
#ifdef DEBUG_TABLE_CELLMAP
  printf("=== insertRowsAfter \n");
  Dump(PR_TRUE, PR_FALSE, PR_TRUE);
#endif

  return numColsToAdd;
}

// this cannot extend beyond a single row group
void nsTableFrame::RemoveRows(nsTableRowFrame& aFirstRowFrame,
                              PRInt32          aNumRowsToRemove,
                              PRBool           aConsiderSpans)
{
#ifdef TBD_OPTIMIZATION
  // decide if we need to rebalance. we have to do this here because the row group 
  // cannot do it when it gets the dirty reflow corresponding to the frame being destroyed
  PRBool stopTelling = PR_FALSE;
  for (nsIFrame* kidFrame = aFirstFrame.FirstChild(); (kidFrame && !stopAsking);
       kidFrame = kidFrame->GetNextSibling()) {
    if (IS_TABLE_CELL(kidFrame->GetType())) {
      nsTableCellFrame* cellFrame = (nsTableCellFrame*)kidFrame;
      stopTelling = tableFrame->CellChangedWidth(*cellFrame, cellFrame->GetPass1MaxElementWidth(), 
                                                 cellFrame->GetMaximumWidth(), PR_TRUE);
    }
  }
  // XXX need to consider what happens if there are cells that have rowspans 
  // into the deleted row. Need to consider moving rows if a rebalance doesn't happen
#endif

  PRInt32 firstRowIndex = aFirstRowFrame.GetRowIndex();
#ifdef DEBUG_TABLE_CELLMAP
  printf("=== removeRowsBefore firstRow=%d numRows=%d\n", firstRowIndex, aNumRowsToRemove);
  Dump(PR_TRUE, PR_FALSE, PR_TRUE);
#endif
  nsTableCellMap* cellMap = GetCellMap();
  if (cellMap) {
    nsRect damageArea(0,0,0,0);
    cellMap->RemoveRows(firstRowIndex, aNumRowsToRemove, aConsiderSpans, damageArea);
    // only remove cols that are of type eTypeAnonymous cell (they are at the end)
    PRInt32 numColsInMap = GetColCount(); // cell map's notion of num cols
    PRInt32 numColsInCache = mColFrames.Count();
    if (numColsInCache > numColsInMap) {
      PRInt32 numColsNotRemoved = DestroyAnonymousColFrames(numColsInCache - numColsInMap);
      // if the cell map has fewer cols than the cache, correct it
      if (numColsNotRemoved > 0) {
        cellMap->AddColsAtEnd(numColsNotRemoved);
      }
    }
    else {
      PRInt32 numAnonymousColsToAdd = numColsInMap - numColsInCache;
      if (numAnonymousColsToAdd > 0) {
        // this sets the child list, updates the col cache and cell map
        CreateAnonymousColFrames(numAnonymousColsToAdd,
                               eColAnonymousCell, PR_TRUE);
      }
    }
    if (IsBorderCollapse()) {
      SetBCDamageArea(damageArea);
    }
  }
  AdjustRowIndices(firstRowIndex, -aNumRowsToRemove);
#ifdef DEBUG_TABLE_CELLMAP
  printf("=== removeRowsAfter\n");
  Dump(PR_TRUE, PR_TRUE, PR_TRUE);
#endif
}

void nsTableFrame::AppendRowGroups(nsIFrame* aFirstRowGroupFrame)
{
  if (aFirstRowGroupFrame) {
    nsTableCellMap* cellMap = GetCellMap();
    if (cellMap) {
      nsFrameList newList(aFirstRowGroupFrame);
      InsertRowGroups(aFirstRowGroupFrame, newList.LastChild());
    }
  }
}

nsTableRowGroupFrame*
nsTableFrame::GetRowGroupFrame(nsIFrame* aFrame,
                               nsIAtom*  aFrameTypeIn)
{
  nsIFrame* rgFrame = nsnull;
  nsIAtom* frameType = aFrameTypeIn;
  if (!aFrameTypeIn) {
    frameType = aFrame->GetType();
  }
  if (nsLayoutAtoms::tableRowGroupFrame == frameType) {
    rgFrame = aFrame;
  }
  else if (nsLayoutAtoms::scrollFrame == frameType) {
    nsIScrollableFrame* scrollable = nsnull;
    nsresult rv = CallQueryInterface(aFrame, &scrollable);
    if (NS_SUCCEEDED(rv) && (scrollable)) {
      nsIFrame* scrolledFrame = scrollable->GetScrolledFrame();
      if (scrolledFrame) {
        if (nsLayoutAtoms::tableRowGroupFrame == scrolledFrame->GetType()) {
          rgFrame = scrolledFrame;
        }
      }
    }
  }
  return (nsTableRowGroupFrame*)rgFrame;
}

// collect the rows ancestors of aFrame
PRInt32
nsTableFrame::CollectRows(nsIFrame*       aFrame,
                          nsVoidArray&    aCollection)
{
  if (!aFrame) return 0;
  PRInt32 numRows = 0;
  nsTableRowGroupFrame* rgFrame = GetRowGroupFrame(aFrame);
  if (rgFrame) {
    nsIFrame* childFrame = rgFrame->GetFirstChild(nsnull);
    while (childFrame) {
      if (nsLayoutAtoms::tableRowFrame == childFrame->GetType()) {
        aCollection.AppendElement(childFrame);
        numRows++;
      }
      else {
        numRows += CollectRows(childFrame, aCollection);
      }
      childFrame = childFrame->GetNextSibling();
    }
  }
  return numRows;
}

void
nsTableFrame::InsertRowGroups(nsIFrame* aFirstRowGroupFrame,
                              nsIFrame* aLastRowGroupFrame)
{
#ifdef DEBUG_TABLE_CELLMAP
  printf("=== insertRowGroupsBefore\n");
  Dump(PR_TRUE, PR_FALSE, PR_TRUE);
#endif
  nsTableCellMap* cellMap = GetCellMap();
  if (cellMap) {
    nsAutoVoidArray orderedRowGroups;
    PRUint32 numRowGroups;
    OrderRowGroups(orderedRowGroups, numRowGroups);
    nsAutoVoidArray rows;
    // Loop over the rowgroups and check if some of them are new, if they are
    // insert cellmaps in the order that is predefined by OrderRowGroups,
    for (PRUint32 rgIndex = 0; rgIndex < numRowGroups; rgIndex++) {
      nsIFrame* kidFrame = aFirstRowGroupFrame;
      while (kidFrame) {
        nsTableRowGroupFrame* rgFrame = GetRowGroupFrame(kidFrame);

        if (GetRowGroupFrame((nsIFrame*)orderedRowGroups.ElementAt(rgIndex)) == rgFrame) {
          nsTableRowGroupFrame* priorRG = (0 == rgIndex)
            ? nsnull : GetRowGroupFrame((nsIFrame*)orderedRowGroups.ElementAt(rgIndex - 1)); 
          // create and add the cell map for the row group
          cellMap->InsertGroupCellMap(*rgFrame, priorRG);
          // collect the new row frames in an array and add them to the table
          PRInt32 numRows = CollectRows(kidFrame, rows);
          if (numRows > 0) {
            PRInt32 rowIndex = 0;
            if (priorRG) {
              PRInt32 priorNumRows = priorRG->GetRowCount();
              rowIndex = priorRG->GetStartRowIndex() + priorNumRows;
            }
            InsertRows(*rgFrame, rows, rowIndex, PR_TRUE);
            rows.Clear();
          }
          break;
        }
        else {
          if (kidFrame == aLastRowGroupFrame) {
            break;
          }
          kidFrame = kidFrame->GetNextSibling();
        }
      }
    }
    cellMap->Synchronize(this);
    ResetRowIndices();
  }
#ifdef DEBUG_TABLE_CELLMAP
  printf("=== insertRowGroupsAfter\n");
  Dump(PR_TRUE, PR_TRUE, PR_TRUE);
#endif
}


/////////////////////////////////////////////////////////////////////////////
// Child frame enumeration

nsIFrame*
nsTableFrame::GetFirstChild(nsIAtom* aListName) const
{
  if (aListName == nsLayoutAtoms::colGroupList) {
    return mColGroups.FirstChild();
  }

  return nsHTMLContainerFrame::GetFirstChild(aListName);
}

nsIAtom*
nsTableFrame::GetAdditionalChildListName(PRInt32 aIndex) const
{
  if (aIndex == NS_TABLE_FRAME_COLGROUP_LIST_INDEX) {
    return nsLayoutAtoms::colGroupList;
  }
  return nsnull;
}

class nsDisplayTableBorderBackground : public nsDisplayItem {
public:
  nsDisplayTableBorderBackground(nsTableFrame* aFrame) : nsDisplayItem(aFrame) {
    MOZ_COUNT_CTOR(nsDisplayTableBorderBackground);
  }
#ifdef NS_BUILD_REFCNT_LOGGING
  virtual ~nsDisplayTableBorderBackground() {
    MOZ_COUNT_DTOR(nsDisplayTableBorderBackground);
  }
#endif

  virtual void Paint(nsDisplayListBuilder* aBuilder, nsIRenderingContext* aCtx,
     const nsRect& aDirtyRect);
  // With collapsed borders, parts of the collapsed border can extend outside
  // the table frame, so allow this display element to blow out to our
  // overflow rect.
  virtual nsRect GetBounds(nsDisplayListBuilder* aBuilder) {
    return NS_STATIC_CAST(nsTableFrame*, mFrame)->GetOverflowRect() +
      aBuilder->ToReferenceFrame(mFrame);
  }
  NS_DISPLAY_DECL_NAME("TableBorderBackground")
};

void
nsDisplayTableBorderBackground::Paint(nsDisplayListBuilder* aBuilder,
    nsIRenderingContext* aCtx, const nsRect& aDirtyRect)
{
  NS_STATIC_CAST(nsTableFrame*, mFrame)->
    PaintTableBorderBackground(*aCtx, aDirtyRect,
                               aBuilder->ToReferenceFrame(mFrame));
}

static PRInt32 GetTablePartRank(nsDisplayItem* aItem)
{
  nsIAtom* type = aItem->GetUnderlyingFrame()->GetType();
  if (type == nsGkAtoms::tableFrame)
    return 0;
  if (type == nsGkAtoms::tableRowGroupFrame)
    return 1;
  if (type == nsGkAtoms::tableRowFrame)
    return 2;
  return 3;
}

static PRBool CompareByTablePartRank(nsDisplayItem* aItem1, nsDisplayItem* aItem2,
                                     void* aClosure)
{
  return GetTablePartRank(aItem1) <= GetTablePartRank(aItem2);
}

/* static */ nsresult
nsTableFrame::DisplayGenericTablePart(nsDisplayListBuilder* aBuilder,
                                      nsFrame* aFrame,
                                      const nsRect& aDirtyRect,
                                      const nsDisplayListSet& aLists,
                                      PRBool aIsRoot)
{
  nsDisplayList eventsBorderBackground;
  // If we need to sort the event backgrounds, then we'll put descendants'
  // display items into their own set of lists.
  PRBool sortEventBackgrounds = aIsRoot && aBuilder->IsForEventDelivery();
  nsDisplayListCollection separatedCollection;
  const nsDisplayListSet* lists = sortEventBackgrounds ? &separatedCollection : &aLists;
  
  // Create dedicated background display items per-frame when we're
  // handling events.
  // XXX how to handle collapsed borders?
  if (aBuilder->IsForEventDelivery() &&
      aFrame->IsVisibleForPainting(aBuilder)) {
    nsresult rv = lists->BorderBackground()->AppendNewToTop(new (aBuilder)
        nsDisplayBackground(aFrame));
    NS_ENSURE_SUCCESS(rv, rv);
  }

  // This is similar to what nsContainerFrame::BuildDisplayListForNonBlockChildren
  // does, except that we allow the children's background and borders to go
  // in our BorderBackground list. This doesn't really affect background
  // painting --- the children won't actually draw their own backgrounds
  // because the nsTableFrame already drew them, unless a child has its own
  // stacking context, in which case the child won't use its passed-in
  // BorderBackground list anyway. It does affect cell borders though; this
  // lets us get cell borders into the nsTableFrame's BorderBackground list.
  nsIFrame* kid = aFrame->GetFirstChild(nsnull);
  while (kid) {
    nsresult rv = aFrame->BuildDisplayListForChild(aBuilder, kid, aDirtyRect, *lists);
    NS_ENSURE_SUCCESS(rv, rv);
    kid = kid->GetNextSibling();
  }
  
  if (sortEventBackgrounds) {
    // Ensure that the table frame event background goes before the
    // table rowgroups event backgrounds, before the table row event backgrounds,
    // before everything else (cells and their blocks)
    separatedCollection.BorderBackground()->Sort(aBuilder, CompareByTablePartRank, nsnull);
    separatedCollection.MoveTo(aLists);
  }
  
  return aFrame->DisplayOutline(aBuilder, aLists);
}

// table paint code is concerned primarily with borders and bg color
// SEC: TODO: adjust the rect for captions 
NS_IMETHODIMP
nsTableFrame::BuildDisplayList(nsDisplayListBuilder*   aBuilder,
                               const nsRect&           aDirtyRect,
                               const nsDisplayListSet& aLists)
{
  if (!IsVisibleInSelection(aBuilder))
    return NS_OK;

  // This background is created regardless of whether this frame is
  // visible or not. Visibility decisions are delegated to the
  // table background painter.
  nsresult rv = aLists.BorderBackground()->AppendNewToTop(new (aBuilder)
      nsDisplayTableBorderBackground(this));
  NS_ENSURE_SUCCESS(rv, rv);
  
  return DisplayGenericTablePart(aBuilder, this, aDirtyRect, aLists, PR_TRUE);
}

// XXX We don't put the borders and backgrounds in tree order like we should.
// That requires some major surgery which we aren't going to do right now.
void
nsTableFrame::PaintTableBorderBackground(nsIRenderingContext& aRenderingContext,
                                         const nsRect& aDirtyRect,
                                         nsPoint aPt)
{
  nsPresContext* presContext = GetPresContext();
  nsRect dirtyRect = aDirtyRect - aPt;
  nsIRenderingContext::AutoPushTranslation
    translate(&aRenderingContext, aPt.x, aPt.y);

  TableBackgroundPainter painter(this, TableBackgroundPainter::eOrigin_Table,
                                 presContext, aRenderingContext, dirtyRect);
  nsresult rv;
  
  if (eCompatibility_NavQuirks == presContext->CompatibilityMode()) {
    nsMargin deflate(0,0,0,0);
    if (IsBorderCollapse()) {
      GET_PIXELS_TO_TWIPS(presContext, p2t);
      BCPropertyData* propData =
        (BCPropertyData*)nsTableFrame::GetProperty((nsIFrame*)this,
                                                   nsLayoutAtoms::tableBCProperty,
                                                   PR_FALSE);
      if (propData) {
        deflate.top    = BC_BORDER_TOP_HALF_COORD(p2t, propData->mTopBorderWidth);
        deflate.right  = BC_BORDER_RIGHT_HALF_COORD(p2t, propData->mRightBorderWidth);
        deflate.bottom = BC_BORDER_BOTTOM_HALF_COORD(p2t, propData->mBottomBorderWidth);
        deflate.left   = BC_BORDER_LEFT_HALF_COORD(p2t, propData->mLeftBorderWidth);
      }
    }
    rv = painter.PaintTable(this, &deflate);
    if (NS_FAILED(rv)) return;
  }
  else {
    rv = painter.PaintTable(this, nsnull);
    if (NS_FAILED(rv)) return;
  }

  if (GetStyleVisibility()->IsVisible()) {
    const nsStyleBorder* border = GetStyleBorder();
    nsRect  rect(0, 0, mRect.width, mRect.height);
    if (!IsBorderCollapse()) {
      PRIntn skipSides = GetSkipSides();
      nsCSSRendering::PaintBorder(presContext, aRenderingContext, this,
                                  dirtyRect, rect, *border, mStyleContext,
                                  skipSides);
    }
    else {
      PaintBCBorders(aRenderingContext, dirtyRect);
    }
  }
}

//null range means the whole thing
NS_IMETHODIMP
nsTableFrame::SetSelected(nsPresContext* aPresContext,
                          nsIDOMRange *aRange,
                          PRBool aSelected,
                          nsSpread aSpread)
{
#if 0
  //traverse through children unselect tables
  if ((aSpread == eSpreadDown)){
    nsIFrame* kid = GetFirstChild(nsnull);
    while (kid) {
      kid->SetSelected(nsnull, aSelected, eSpreadDown);
      kid = kid->GetNextSibling();
    }
  }
#endif
  // Must call base class to set mSelected state and trigger repaint of frame
  // Note that in current version, aRange and aSpread are ignored,
  //   only this frame is considered
  nsFrame::SetSelected(aPresContext, aRange, aSelected, aSpread);
  return NS_OK;//return nsFrame::SetSelected(aRange,aSelected,eSpreadNone);
  
}

PRBool nsTableFrame::ParentDisablesSelection() const //override default behavior
{
  PRBool returnval;
  if (NS_FAILED(GetSelected(&returnval)))
    return PR_FALSE;
  if (returnval)
    return PR_TRUE;
  return nsFrame::ParentDisablesSelection();
}

PRIntn
nsTableFrame::GetSkipSides() const
{
  PRIntn skip = 0;
  // frame attribute was accounted for in nsHTMLTableElement::MapTableBorderInto
  // account for pagination
  if (nsnull != GetPrevInFlow()) {
    skip |= 1 << NS_SIDE_TOP;
  }
  if (nsnull != GetNextInFlow()) {
    skip |= 1 << NS_SIDE_BOTTOM;
  }
  return skip;
}

PRBool nsTableFrame::NeedsReflow(const nsHTMLReflowState& aReflowState)
{
  PRBool result = PR_TRUE;
  if (eReflowReason_Resize == aReflowState.reason) {
    if (aReflowState.mFlags.mSpecialHeightReflow &&
        !NeedSpecialReflow()                   &&
        !NeedToInitiateSpecialReflow()) {
      result = PR_FALSE;
    }
  }
  else if ((eReflowReason_Incremental == aReflowState.reason) &&
           (NS_UNCONSTRAINEDSIZE == aReflowState.availableHeight)) {
    // It's an incremental reflow and we're in galley mode. Only
    // do a full reflow if we need to.
    result = NeedStrategyInit() || NeedStrategyBalance();
  }
  return result;
}

// Called by IR_TargetIsChild() after an incremental reflow of
// aKidFrame. Only called if we don't need a full reflow, e.g., the
// column widths haven't changed. Not used for paginated mode, so
// we don't need to worry about split row group frames
//
// Slides all the row groups following aKidFrame by the specified
// amount
nsresult 
nsTableFrame::AdjustSiblingsAfterReflow(nsTableReflowState& aReflowState,
                                        nsIFrame*           aKidFrame,
                                        nscoord             aDeltaY)
{
  NS_PRECONDITION(NS_UNCONSTRAINEDSIZE == aReflowState.reflowState.availableHeight,
                  "we're not in galley mode");

  nscoord yInvalid = NS_UNCONSTRAINEDSIZE;

  // Get the ordered children and find aKidFrame in the list
  nsAutoVoidArray rowGroups;
  PRUint32 numRowGroups;
  OrderRowGroups(rowGroups, numRowGroups, nsnull);
  PRUint32 changeIndex;
  for (changeIndex = 0; changeIndex < numRowGroups; changeIndex++) {
    if (aKidFrame == rowGroups.ElementAt(changeIndex)) {
      break;
    }
  }
  changeIndex++; // set it to the next sibling

  for (PRUint32 rgX = changeIndex; rgX < numRowGroups; rgX++) {
    nsIFrame* kidFrame = (nsIFrame*)rowGroups.ElementAt(rgX);
    // Move the frames that follow aKidFrame by aDeltaY, and update the running
    // y-offset
    nsTableRowGroupFrame* rgFrame = GetRowGroupFrame(kidFrame);
    if (!rgFrame) continue; // skip foreign frames

    // Get the frame's bounding rect
    nsRect kidRect = kidFrame->GetRect();
    yInvalid = PR_MIN(yInvalid, kidRect.y);
  
    // Adjust the running y-offset
    aReflowState.y += kidRect.height;
 
    // Adjust the y-origin if its position actually changed
    if (aDeltaY != 0) {
      kidRect.y += aDeltaY;
      kidFrame->SetPosition(nsPoint(kidRect.x, kidRect.y));
      RePositionViews(kidFrame);
    }
  }
  
  // Invalidate the area we offset.
  if (NS_UNCONSTRAINEDSIZE != yInvalid) {
    nsRect  dirtyRect(0, yInvalid, mRect.width, mRect.height - yInvalid);
    // XXX what if some of the cells have outlines?
    Invalidate(dirtyRect);
  }

  return NS_OK;
}

void
nsTableFrame::SetColumnDimensions(nscoord         aHeight,
                                  const nsMargin& aBorderPadding)
{
  nscoord cellSpacingX = GetCellSpacingX();
  nscoord cellSpacingY = GetCellSpacingY();
  nscoord colHeight = aHeight -= aBorderPadding.top + aBorderPadding.bottom +
                                 2* cellSpacingY;

  nsIFrame* colGroupFrame = mColGroups.FirstChild();
  PRInt32 colX = 0;
  nsPoint colGroupOrigin(aBorderPadding.left + cellSpacingX,
                         aBorderPadding.top + cellSpacingY);
  while (nsnull != colGroupFrame) {
    nscoord colGroupWidth = 0;
    nsIFrame* colFrame = colGroupFrame->GetFirstChild(nsnull);
    nsPoint colOrigin(0,0);
    while (nsnull != colFrame) {
      if (NS_STYLE_DISPLAY_TABLE_COLUMN ==
          colFrame->GetStyleDisplay()->mDisplay) {
        NS_ASSERTION(colX < GetColCount(), "invalid number of columns");
        nscoord colWidth = GetColumnWidth(colX);
        nsRect colRect(colOrigin.x, colOrigin.y, colWidth, colHeight);
        colFrame->SetRect(colRect);
        colOrigin.x += colWidth + cellSpacingX;
        colGroupWidth += colWidth + cellSpacingX;
        colX++;
      }
      colFrame = colFrame->GetNextSibling();
    }
    if (colGroupWidth) {
      colGroupWidth -= cellSpacingX;
    }

    nsRect colGroupRect(colGroupOrigin.x, colGroupOrigin.y, colGroupWidth, colHeight);
    colGroupFrame->SetRect(colGroupRect);
    colGroupFrame = colGroupFrame->GetNextSibling();
    colGroupOrigin.x += colGroupWidth + cellSpacingX;
  }
}

// SEC: TODO need to worry about continuing frames prev/next in flow for splitting across pages.

// XXX this could be made more general to handle row modifications that change the
// table height, but first we need to scrutinize every Invalidate
static void
ProcessRowInserted(nsTableFrame&   aTableFrame,
                   PRBool          aInvalidate,
                   nscoord         aNewHeight)
{
  aTableFrame.SetRowInserted(PR_FALSE); // reset the bit that got us here
  nsAutoVoidArray rowGroups;
  PRUint32 numRowGroups;
  aTableFrame.OrderRowGroups(rowGroups, numRowGroups);
  // find the row group containing the inserted row
  for (PRUint32 rgX = 0; rgX < numRowGroups; rgX++) {
    nsTableRowGroupFrame* rgFrame = (nsTableRowGroupFrame*)rowGroups.ElementAt(rgX);
    if (!rgFrame) continue; // should never happen
    nsIFrame* childFrame = rgFrame->GetFirstChild(nsnull);
    // find the row that was inserted first
    while (childFrame) {
      if (nsLayoutAtoms::tableRowFrame == childFrame->GetType()) {
        nsTableRowFrame* rowFrame = (nsTableRowFrame*)childFrame;
        if (rowFrame->IsFirstInserted()) {
          rowFrame->SetFirstInserted(PR_FALSE);
          if (aInvalidate) {
            // damage the table from the 1st row inserted to the end of the table
            nscoord damageY = rgFrame->GetPosition().y + rowFrame->GetPosition().y;
            nsRect damageRect(0, damageY,
                              aTableFrame.GetSize().width, aNewHeight - damageY);

            aTableFrame.Invalidate(damageRect);
            aTableFrame.SetRowInserted(PR_FALSE);
          }
          return; // found it, so leave
        }
      }
      childFrame = childFrame->GetNextSibling();
    }
  }
}

// Return true if aStylePosition has a pct height
static PRBool 
IsPctStyleHeight(const nsStylePosition* aStylePosition)
{
  return (aStylePosition && 
          (eStyleUnit_Percent == aStylePosition->mHeight.GetUnit()));
}

// Return true if aStylePosition has a coord height
static PRBool 
IsFixedStyleHeight(const nsStylePosition* aStylePosition)
{
  return (aStylePosition && 
          (eStyleUnit_Coord == aStylePosition->mHeight.GetUnit()));
}

// Return true if any of aReflowState.frame's ancestors within the containing table
// have a pct or fixed height
static PRBool
AncestorsHaveStyleHeight(const nsHTMLReflowState& aReflowState)
{
  for (const nsHTMLReflowState* parentRS = aReflowState.parentReflowState;
       parentRS && parentRS->frame; 
       parentRS = parentRS->parentReflowState) {
    nsIAtom* frameType = parentRS->frame->GetType();
    if (IS_TABLE_CELL(frameType)                         ||
        (nsLayoutAtoms::tableRowFrame      == frameType) ||
        (nsLayoutAtoms::tableRowGroupFrame == frameType)) {
      if (::IsPctStyleHeight(parentRS->mStylePosition) || ::IsFixedStyleHeight(parentRS->mStylePosition)) {
        return PR_TRUE;
      }
    }
    else if (nsLayoutAtoms::tableFrame == frameType) {
      // we reached the containing table, so always return
      if (::IsPctStyleHeight(parentRS->mStylePosition) || ::IsFixedStyleHeight(parentRS->mStylePosition)) {
        return PR_TRUE;
      }
      else return PR_FALSE;
    }
  }
  return PR_FALSE;
}

// See if a special height reflow needs to occur and if so, call RequestSpecialHeightReflow
void
nsTableFrame::CheckRequestSpecialHeightReflow(const nsHTMLReflowState& aReflowState)
{
  if (!aReflowState.frame) ABORT0();
  nsIFrame* prevInFlow = aReflowState.frame->GetPrevInFlow();

  if (!prevInFlow                                             &&   // 1st in flow                                            && // 1st in flow
      ((NS_UNCONSTRAINEDSIZE == aReflowState.mComputedHeight) ||   // no computed height
       (0                    == aReflowState.mComputedHeight))  && 
      ::IsPctStyleHeight(aReflowState.mStylePosition)) {           // pct height

    if (::AncestorsHaveStyleHeight(aReflowState)) {
      nsTableFrame::RequestSpecialHeightReflow(aReflowState);
    }
  }
}

// Notify the frame and its ancestors (up to the containing table) that a special
// height reflow will occur. During a special height reflow, a table, row group,
// row, or cell returns the last size it was reflowed at. However, the table may 
// change the height of row groups, rows, cells in DistributeHeightToRows after. 
// And the row group can change the height of rows, cells in CalculateRowHeights.
void
nsTableFrame::RequestSpecialHeightReflow(const nsHTMLReflowState& aReflowState)
{
  // notify the frame and its ancestors of the special reflow, stopping at the containing table
  for (const nsHTMLReflowState* rs = &aReflowState; rs && rs->frame; rs = rs->parentReflowState) {
    nsIAtom* frameType = rs->frame->GetType();
    if (IS_TABLE_CELL(frameType)) {
      ((nsTableCellFrame*)rs->frame)->SetNeedSpecialReflow(PR_TRUE);
    }
    else if (nsLayoutAtoms::tableRowFrame == frameType) {
      ((nsTableRowFrame*)rs->frame)->SetNeedSpecialReflow(PR_TRUE);
    }
    else if (nsLayoutAtoms::tableRowGroupFrame == frameType) {
      ((nsTableRowGroupFrame*)rs->frame)->SetNeedSpecialReflow(PR_TRUE);
    }
    else if (nsLayoutAtoms::tableFrame == frameType) {
      if (rs == &aReflowState) {
        // don't stop because we started with this table 
        ((nsTableFrame*)rs->frame)->SetNeedSpecialReflow(PR_TRUE);
      }
      else {
        ((nsTableFrame*)rs->frame)->SetNeedToInitiateSpecialReflow(PR_TRUE);
        // always stop when we reach a table that we didn't start with
        break;
      }
    }
  }
}

// Return true (and set aMetrics's desiredSize to aRect) if the special height reflow
// was initiated by an ancestor of aReflowState.frame's containing table. In that case, 
// aFrame's containing table will eventually initiate a special height reflow which 
// will cause this method to return false. 
PRBool
nsTableFrame::IsPrematureSpecialHeightReflow(const nsHTMLReflowState& aReflowState,
                                             const nsRect&            aRect,
                                             PRBool                   aNeedSpecialHeightReflow,
                                             nsHTMLReflowMetrics&     aMetrics)
{
  PRBool premature = PR_FALSE; 
  if (aReflowState.mFlags.mSpecialHeightReflow) { 
    if (aNeedSpecialHeightReflow) { 
      nsTableFrame* tableFrame = nsTableFrame::GetTableFrame(aReflowState.frame);
      if (tableFrame && (tableFrame != aReflowState.mPercentHeightReflowInitiator)) { 
        premature = PR_TRUE; 
      } 
    } 
    else { 
      premature = PR_TRUE; 
    } 
    if (premature) { 
      aMetrics.width  = aRect.width; 
      aMetrics.height = aRect.height; 
    } 
  }
  return premature;
}

/******************************************************************************************
 * During the initial reflow the table reflows each child with an unconstrained avail width
 * to get its max element width and maximum width. This is referred to as the pass 1 reflow.
 *
 * After the 1st pass reflow, the table determines the column widths using BalanceColumnWidths()
 * then reflows each child again with a constrained avail width. This reflow is referred to
 * as the pass 2 reflow. 
 *
 * A special height reflow (pass 3 reflow) can occur during an intitial or resize reflow
 * if (a) a row group, row, cell, or a frame inside a cell has a percent height but no computed 
 * height or (b) in paginated mode, a table has a height. (a) supports percent nested tables 
 * contained inside cells whose heights aren't known until after the pass 2 reflow. (b) is 
 * necessary because the table cannot split until after the pass 2 reflow. The mechanics of 
 * the special height reflow (variety a) are as follows: 
 * 
 * 1) Each table related frame (table, row group, row, cell) implements NeedsSpecialReflow()
 *    to indicate that it should get the reflow. It does this when it has a percent height but 
 *    no computed height by calling CheckRequestSpecialHeightReflow(). This method calls
 *    RequestSpecialHeightReflow() which calls SetNeedSpecialReflow() on its ancestors until 
 *    it reaches the containing table and calls SetNeedToInitiateSpecialReflow() on it. For 
 *    percent height frames inside cells, during DidReflow(), the cell's NotifyPercentHeight()
 *    is called (the cell is the reflow state's mPercentHeightObserver in this case). 
 *    NotifyPercentHeight() calls RequestSpecialHeightReflow().
 *
 * 2) After the pass 2 reflow, if the table's NeedToInitiateSpecialReflow(true) was called, it
 *    will do the special height reflow, setting the reflow state's mFlages.mSpecialHeightReflow
 *    to true and mSpecialHeightInitiator to itself. It won't do this if IsPrematureSpecialHeightReflow()
 *    returns true because in that case another special height reflow will be coming along with the
 *    containing table as the mSpecialHeightInitiator. It is only relevant to do the reflow when
 *    the mSpecialHeightInitiator is the containing table, because if it is a remote ancestor, then
 *    appropriate heights will not be known.
 *
 * 3) Since the heights of the table, row groups, rows, and cells was determined during the pass 2
 *    reflow, they return their last desired sizes during the special height reflow. The reflow only
 *    permits percent height frames inside the cells to resize based on the cells height and that height
 *    was determined during the pass 2 reflow.
 *
 * So, in the case of deeply nested tables, all of the tables that were told to initiate a special
 * reflow will do so, but if a table is already in a special reflow, it won't inititate the reflow
 * until the current initiator is its containing table. Since these reflows are only received by
 * frames that need them and they don't cause any rebalancing of tables, the extra overhead is minimal.
 *
 * The type of special reflow that occurs during printing (variety b) follows the same mechanism except
 * that all frames will receive the reflow even if they don't really need them.
 *
 * Open issues with the special height reflow:
 *
 * 1) At some point there should be 2 kinds of special height reflows because (a) and (b) above are 
 *    really quite different. This would avoid unnecessary reflows during printing. 
 * 2) When a cell contains frames whose percent heights > 100%, there is data loss (see bug 115245). 
 *    However, this can also occur if a cell has a fixed height and there is no special height reflow. 
 *
 ******************************************************************************************/

/* Layout the entire inner table. */
NS_METHOD nsTableFrame::Reflow(nsPresContext*          aPresContext,
                               nsHTMLReflowMetrics&     aDesiredSize,
                               const nsHTMLReflowState& aReflowState,
                               nsReflowStatus&          aStatus)
{
  DO_GLOBAL_REFLOW_COUNT("nsTableFrame", aReflowState.reason);
  DISPLAY_REFLOW(aPresContext, this, aReflowState, aDesiredSize, aStatus);
#if defined DEBUG_TABLE_REFLOW_TIMING
  nsTableFrame::DebugReflow(this, (nsHTMLReflowState&)aReflowState);
#endif
  PRBool isPaginated = aPresContext->IsPaginated();

  // If this is a special height reflow, set our desired size to what is was previously and return
  // if we will be getting another special height reflow. In paginated mode, SetNeedSpecialReflow(PR_TRUE) 
  // may not have been called if reflow was a result of having a height on the containing table
  if (IsPrematureSpecialHeightReflow(aReflowState, mRect, NeedSpecialReflow() || isPaginated, aDesiredSize)) 
    return NS_OK;

  aStatus = NS_FRAME_COMPLETE; 
  if (!GetPrevInFlow() && !mTableLayoutStrategy) {
    NS_ASSERTION(PR_FALSE, "strategy should have been created in Init");
    return NS_ERROR_NULL_POINTER;
  }
  nsresult rv = NS_OK;

  // see if a special height reflow needs to occur due to having a pct height
  if (!NeedSpecialReflow()) 
    nsTableFrame::CheckRequestSpecialHeightReflow(aReflowState);

  // see if collapsing borders need to be calculated
  if (!GetPrevInFlow() && IsBorderCollapse() && NeedToCalcBCBorders()) {
    GET_TWIPS_TO_PIXELS(aPresContext, p2t);
    CalcBCBorders();
  }

  aDesiredSize.width = aReflowState.availableWidth;

  nsReflowReason nextReason = aReflowState.reason;

  // Check for an overflow list, and append any row group frames being pushed
  MoveOverflowToChildList(aPresContext);

  // Processes an initial (except when there is mPrevInFlow), incremental, or style 
  // change reflow 1st. resize reflows are processed in the next phase.
  switch (aReflowState.reason) {
    case eReflowReason_Initial: 
    case eReflowReason_StyleChange: {
      if ((eReflowReason_Initial == aReflowState.reason) && HadInitialReflow()) {
        // XXX this could be an assertion and the if removed
        NS_WARNING("table initial reflow called twice");
      }
      else {
        if (!GetPrevInFlow()) { // only do pass1 on a first in flow
          if (IsAutoLayout()) {     
            // only do pass1 reflow on an auto layout table
            nsTableReflowState reflowState(*aPresContext, aReflowState, *this,
                                           aReflowState.reason,
                                           NS_UNCONSTRAINEDSIZE,
                                           NS_UNCONSTRAINEDSIZE);
            // reflow the children
            nsIFrame *lastReflowed;
            nsRect overflowArea;
            ReflowChildren(reflowState, !HaveReflowedColGroups(),
                           PR_FALSE, aStatus, lastReflowed,
                           overflowArea);
          }
          mTableLayoutStrategy->Initialize(aReflowState);
        }
      }
      SetHadInitialReflow(PR_TRUE);
      if (!GetPrevInFlow()) {
        SetNeedStrategyBalance(PR_TRUE); // force a balance and then a pass2 reflow 
        if ((nextReason != eReflowReason_StyleChange) || IsAutoLayout()) 
          nextReason = eReflowReason_Resize;
      }
      else {
        nextReason = eReflowReason_Initial;
      }
      break; 
    }
    case eReflowReason_Incremental:
      NS_ASSERTION(HadInitialReflow(), "intial reflow not called");
      rv = IncrementalReflow(aReflowState, aStatus);
      nextReason = eReflowReason_Resize;
      break;
    case eReflowReason_Resize:
      // do the resize reflow below
      if (!HadInitialReflow()) {
        NS_ASSERTION(HadInitialReflow(), "intial reflow not called");
        nextReason = eReflowReason_Initial;
      }
      if (NS_UNCONSTRAINEDSIZE == aReflowState.availableWidth)
        NS_WARNING("this reflow doesn't do anything");
      SetNeedStrategyBalance(PR_TRUE); 
      break; 
    default:
      break;
  }

  if (NS_FAILED(rv)) return rv;

  PRBool haveDesiredHeight = PR_FALSE;
  PRBool balanced          = PR_FALSE;
  PRBool reflowedChildren  = PR_FALSE;

  // Reflow the entire table (pass 2 and possibly pass 3). This phase is necessary during a 
  // constrained initial reflow and other reflows which require either a strategy init or balance. 
  // This isn't done during an unconstrained reflow, because it will occur later when the parent 
  // reflows with a constrained width.
  if (NeedsReflow(aReflowState) && (NS_UNCONSTRAINEDSIZE != aReflowState.availableWidth)) {
    // see if an extra reflow will be necessary in pagination mode when there is a specified table height 
    if (isPaginated && !GetPrevInFlow() && (NS_UNCONSTRAINEDSIZE != aReflowState.availableHeight)) {
      nscoord tableSpecifiedHeight = CalcBorderBoxHeight(aReflowState);
      if ((tableSpecifiedHeight > 0) && 
          (tableSpecifiedHeight != NS_UNCONSTRAINEDSIZE)) {
        SetNeedToInitiateSpecialReflow(PR_TRUE);
      }
    }
    nsIFrame* lastChildReflowed = nsnull;
    PRBool willInitiateSpecialReflow = 
      ((NeedToInitiateSpecialReflow() || InitiatedSpecialReflow()) && 
       (aReflowState.mFlags.mSpecialHeightReflow || !NeedSpecialReflow()));

    // do the pass 2 reflow unless this is a special height reflow and we will be 
    // initiating a special height reflow
    if (!(aReflowState.mFlags.mSpecialHeightReflow && willInitiateSpecialReflow)) {
      // if we need to initiate a special height reflow, then don't constrain the 
      // height of the reflow before that
      nscoord availHeight = (willInitiateSpecialReflow)
                            ? NS_UNCONSTRAINEDSIZE : aReflowState.availableHeight;

      ReflowTable(aDesiredSize, aReflowState, availHeight, nextReason, 
                  lastChildReflowed, balanced, aStatus);
      nextReason = eReflowReason_Resize;
      reflowedChildren = PR_TRUE;
    }
    // reevaluate special height reflow conditions
    if ((NeedToInitiateSpecialReflow() || InitiatedSpecialReflow()) &&
        (aReflowState.mFlags.mSpecialHeightReflow || !NeedSpecialReflow()) &&
        NS_FRAME_IS_COMPLETE(aStatus)) {
      // distribute extra vertical space to rows
      CalcDesiredHeight(aReflowState, aDesiredSize); 
      ((nsHTMLReflowState::ReflowStateFlags&)aReflowState.mFlags).mSpecialHeightReflow = PR_TRUE;
      // save the previous special height reflow initiator, install us as the new one
      nsIFrame* specialReflowInitiator = aReflowState.mPercentHeightReflowInitiator;
      ((nsHTMLReflowState&)aReflowState).mPercentHeightReflowInitiator = this;

      ((nsHTMLReflowState::ReflowStateFlags&)aReflowState.mFlags).mSpecialHeightReflow = PR_TRUE;
      ReflowTable(aDesiredSize, aReflowState, aReflowState.availableHeight, 
                  nextReason, lastChildReflowed, balanced, aStatus);
      // restore the previous special height reflow initiator
      ((nsHTMLReflowState&)aReflowState).mPercentHeightReflowInitiator = specialReflowInitiator;
      // XXX We should call SetInitiatedSpecialReflow(PR_FALSE) at some point, but it is difficult to tell when
      SetInitiatedSpecialReflow(PR_TRUE);

      if (lastChildReflowed && NS_FRAME_IS_NOT_COMPLETE(aStatus)) {
        // if there is an incomplete child, then set the desired height to include it but not the next one
        nsMargin borderPadding = GetChildAreaOffset(&aReflowState);
        aDesiredSize.height = borderPadding.bottom + GetCellSpacingY() +
                              lastChildReflowed->GetRect().YMost();
      }
      haveDesiredHeight = PR_TRUE;
      reflowedChildren  = PR_TRUE;
    }
  }
  else if (aReflowState.mFlags.mSpecialHeightReflow) {
    aDesiredSize.width  = mRect.width;
    aDesiredSize.height = mRect.height;
#if defined DEBUG_TABLE_REFLOW_TIMING
    nsTableFrame::DebugReflow(this, (nsHTMLReflowState&)aReflowState, &aDesiredSize, aStatus);
#endif
    SetNeedSpecialReflow(PR_FALSE);
    SetNeedToInitiateSpecialReflow(PR_FALSE);
    return NS_OK;
  }

  aDesiredSize.width = GetDesiredWidth();
  if (!haveDesiredHeight) {
    CalcDesiredHeight(aReflowState, aDesiredSize); 
  }
  if (IsRowInserted()) {
    ProcessRowInserted(*this, PR_TRUE, aDesiredSize.height);
  }

  nsMargin borderPadding = GetChildAreaOffset(&aReflowState);
  SetColumnDimensions(aDesiredSize.height, borderPadding);
  if (NeedToCollapse() &&
      (NS_UNCONSTRAINEDSIZE != aReflowState.availableWidth)) {
    AdjustForCollapsingRowsCols(aDesiredSize, borderPadding);
  }

  // See if we need to calc max elem and/or preferred widths. This isn't done on 
  // continuations or if we have balanced (since it was done then) 
  if ((aDesiredSize.mComputeMEW || (aDesiredSize.mFlags & NS_REFLOW_CALC_MAX_WIDTH)) &&
      !GetPrevInFlow() && !balanced) {
    // Since the calculation has some cost, avoid doing it for an unconstrained initial 
    // reflow (it was done when the strategy was initialized in pass 1 above) and most
    // unconstrained resize reflows. XXX The latter optimization could be a problem if the
    // parent of a nested table starts doing unconstrained resize reflows to get max elem/preferred 
    if ((NS_UNCONSTRAINEDSIZE != aReflowState.availableWidth) ||
        (eReflowReason_Incremental == aReflowState.reason)    || 
        (eReflowReason_StyleChange == aReflowState.reason)    ||
        ((eReflowReason_Resize == aReflowState.reason) &&
         HasPctCol() && IsAutoWidth())) {
      nscoord minWidth, prefWidth;
      CalcMinAndPreferredWidths(aReflowState, PR_TRUE, minWidth, prefWidth);
      SetMinWidth(minWidth);
      SetPreferredWidth(prefWidth);
    }
  }
  // See if we need to return our max element size
  if (aDesiredSize.mComputeMEW) {
    aDesiredSize.mMaxElementWidth  = GetMinWidth();
  }
  // See if we need to return our maximum width
  if (aDesiredSize.mFlags & NS_REFLOW_CALC_MAX_WIDTH) {
    aDesiredSize.mMaximumWidth = GetPreferredWidth();
  }
  // make sure the table overflow area does include the table rect.
  nsRect tableRect(0, 0, aDesiredSize.width, aDesiredSize.height) ;
  
  if (!aReflowState.mStyleDisplay->IsTableClip()) {
    // collapsed border may leak out
    nsMargin bcMargin = GetBCMargin();
    tableRect.Inflate(bcMargin);
  }
  aDesiredSize.mOverflowArea.UnionRect(aDesiredSize.mOverflowArea, tableRect);
  
  if (aReflowState.mFlags.mSpecialHeightReflow) {
    SetNeedSpecialReflow(PR_FALSE);
    SetNeedToInitiateSpecialReflow(PR_FALSE);
  }
#if defined DEBUG_TABLE_REFLOW_TIMING
  nsTableFrame::DebugReflow(this, (nsHTMLReflowState&)aReflowState, &aDesiredSize, aStatus);
#endif

  // If we reflowed all the rows, then invalidate the largest possible area that either the
  // table occupied before this reflow or will occupy after.
  if (reflowedChildren) {
    nsRect damage(0, 0, PR_MAX(mRect.width, aDesiredSize.width),
                  PR_MAX(mRect.height, aDesiredSize.height));
    damage.UnionRect(damage, aDesiredSize.mOverflowArea);
    nsRect* oldOverflowArea = GetOverflowAreaProperty();
    if (oldOverflowArea) {
      damage.UnionRect(damage, *oldOverflowArea);
    }
    Invalidate(damage);
  } else {
    // use the old overflow area
     nsRect* oldOverflowArea = GetOverflowAreaProperty();
     if (oldOverflowArea) {
       aDesiredSize.mOverflowArea.UnionRect(aDesiredSize.mOverflowArea, *oldOverflowArea);
     }
  }

  FinishAndStoreOverflow(&aDesiredSize);
  NS_FRAME_SET_TRUNCATION(aStatus, aReflowState, aDesiredSize);
  return rv;
}

nsresult 
nsTableFrame::ReflowTable(nsHTMLReflowMetrics&     aDesiredSize,
                          const nsHTMLReflowState& aReflowState,
                          nscoord                  aAvailHeight,
                          nsReflowReason           aReason,
                          nsIFrame*&               aLastChildReflowed,
                          PRBool&                  aDidBalance,
                          nsReflowStatus&          aStatus)
{
  nsresult rv = NS_OK;
  aDidBalance = PR_FALSE;
  aLastChildReflowed = nsnull;

  PRBool haveReflowedColGroups = PR_TRUE;
  if (!GetPrevInFlow()) {
    if (NeedStrategyInit()) {
      mTableLayoutStrategy->Initialize(aReflowState);
      BalanceColumnWidths(aReflowState); 
      aDidBalance = PR_TRUE;
    }
    if (NeedStrategyBalance()) {
      BalanceColumnWidths(aReflowState);
      aDidBalance = PR_TRUE;
    }
    haveReflowedColGroups = HaveReflowedColGroups();
  }
  // Constrain our reflow width to the computed table width (of the 1st in flow).
  // and our reflow height to our avail height minus border, padding, cellspacing
  aDesiredSize.width = GetDesiredWidth();
  nsTableReflowState reflowState(*GetPresContext(), aReflowState, *this, aReason, 
                                 aDesiredSize.width, aAvailHeight);
  ReflowChildren(reflowState, haveReflowedColGroups, PR_FALSE,
                 aStatus, aLastChildReflowed, aDesiredSize.mOverflowArea);

  if (eReflowReason_Resize == aReflowState.reason) {
    if (!DidResizeReflow()) {
      SetResizeReflow(PR_TRUE);
    }
  }  
  return rv;
}

nsIFrame*
nsTableFrame::GetFirstBodyRowGroupFrame()
{
  nsIFrame* headerFrame = nsnull;
  nsIFrame* footerFrame = nsnull;

  for (nsIFrame* kidFrame = mFrames.FirstChild(); nsnull != kidFrame; ) {
    const nsStyleDisplay* childDisplay = kidFrame->GetStyleDisplay();

    // We expect the header and footer row group frames to be first, and we only
    // allow one header and one footer
    if (NS_STYLE_DISPLAY_TABLE_HEADER_GROUP == childDisplay->mDisplay) {
      if (headerFrame) {
        // We already have a header frame and so this header frame is treated
        // like an ordinary body row group frame
        return kidFrame;
      }
      headerFrame = kidFrame;
    
    } else if (NS_STYLE_DISPLAY_TABLE_FOOTER_GROUP == childDisplay->mDisplay) {
      if (footerFrame) {
        // We already have a footer frame and so this footer frame is treated
        // like an ordinary body row group frame
        return kidFrame;
      }
      footerFrame = kidFrame;

    } else if (NS_STYLE_DISPLAY_TABLE_ROW_GROUP == childDisplay->mDisplay) {
      return kidFrame;
    }

    // Get the next child
    kidFrame = kidFrame->GetNextSibling();
  }

  return nsnull;
}

// Table specific version that takes into account repeated header and footer
// frames when continuing table frames
void
nsTableFrame::PushChildren(const nsAutoVoidArray& aFrames,
                           PRInt32 aPushFrom)
{
  NS_PRECONDITION(aPushFrom > 0, "pushing first child");

  // extract the frames from the array into a sibling list
  nsFrameList frames;
  nsIFrame* lastFrame = nsnull;
  PRUint32 childX;
  nsIFrame* prevSiblingHint =
    NS_STATIC_CAST(nsIFrame*, aFrames.ElementAt(aPushFrom - 1));
  for (childX = aPushFrom; childX < aFrames.Count(); ++childX) {
    nsIFrame* f = NS_STATIC_CAST(nsIFrame*, aFrames.FastElementAt(childX));
    // Don't push repeatable frames, do push non-rowgroup frames
    if (f->GetType() != nsLayoutAtoms::tableRowGroupFrame ||
        !NS_STATIC_CAST(nsTableRowGroupFrame*, f)->IsRepeatable()) {
      mFrames.RemoveFrame(f, prevSiblingHint);
      frames.InsertFrame(nsnull, lastFrame, f);
      lastFrame = f;
    }
  }

  if (nsnull != GetNextInFlow()) {
    nsTableFrame* nextInFlow = (nsTableFrame*)GetNextInFlow();

    // Insert the frames after any repeated header and footer frames
    nsIFrame* firstBodyFrame = nextInFlow->GetFirstBodyRowGroupFrame();
    nsIFrame* prevSibling = nsnull;
    if (firstBodyFrame) {
      prevSibling = nextInFlow->mFrames.GetPrevSiblingFor(firstBodyFrame);
    }
    // When pushing and pulling frames we need to check for whether any
    // views need to be reparented.
    for (nsIFrame* f = frames.FirstChild(); f; f = f->GetNextSibling()) {
      nsHTMLContainerFrame::ReparentFrameView(GetPresContext(), f, this, nextInFlow);
    }
    nextInFlow->mFrames.InsertFrames(GetNextInFlow(), prevSibling, frames.FirstChild());
  }
  else {
    // Add the frames to our overflow list
    SetOverflowFrames(GetPresContext(), frames.FirstChild());
  }
}

// Table specific version that takes into account header and footer row group
// frames that are repeated for continuing table frames
//
// Appends the overflow frames to the end of the child list, just like the
// nsContainerFrame version does, except that there are no assertions that
// the child list is empty (it may not be empty, because there may be repeated
// header/footer frames)
PRBool
nsTableFrame::MoveOverflowToChildList(nsPresContext* aPresContext)
{
  PRBool result = PR_FALSE;

  // Check for an overflow list with our prev-in-flow
  nsTableFrame* prevInFlow = (nsTableFrame*)GetPrevInFlow();
  if (prevInFlow) {
    nsIFrame* prevOverflowFrames = prevInFlow->GetOverflowFrames(aPresContext, PR_TRUE);
    if (prevOverflowFrames) {
      // When pushing and pulling frames we need to check for whether any
      // views need to be reparented.
      for (nsIFrame* f = prevOverflowFrames; f; f = f->GetNextSibling()) {
        nsHTMLContainerFrame::ReparentFrameView(aPresContext, f, prevInFlow, this);
      }
      mFrames.AppendFrames(this, prevOverflowFrames);
      result = PR_TRUE;
    }
  }

  // It's also possible that we have an overflow list for ourselves
  nsIFrame* overflowFrames = GetOverflowFrames(aPresContext, PR_TRUE);
  if (overflowFrames) {
    mFrames.AppendFrames(nsnull, overflowFrames);
    result = PR_TRUE;
  }
  return result;
}



// collapsing row groups, rows, col groups and cols are accounted for after both passes of
// reflow so that it has no effect on the calculations of reflow.
void
nsTableFrame::AdjustForCollapsingRowsCols(nsHTMLReflowMetrics& aDesiredSize,
                                          nsMargin             aBorderPadding)
{
  nscoord yTotalOffset = 0; // total offset among all rows in all row groups

  // reset the bit, it will be set again if row/rowgroup is collapsed
  SetNeedToCollapse(PR_FALSE);
  
  // collapse the rows and/or row groups as necessary
  // Get the ordered children
  nsAutoVoidArray rowGroups;
  PRUint32 numRowGroups;
  OrderRowGroups(rowGroups, numRowGroups);
  nscoord width = GetCollapsedWidth(aBorderPadding);
  nscoord rgWidth = width - 2 * GetCellSpacingX();
  nsRect overflowArea(0, 0, 0, 0);
  // Walk the list of children
  for (PRUint32 childX = 0; childX < numRowGroups; childX++) {
    nsIFrame* childFrame = (nsIFrame*)rowGroups.ElementAt(childX);
    nsTableRowGroupFrame* rgFrame = GetRowGroupFrame(childFrame);
    if (!rgFrame) continue; // skip foreign frame types
    yTotalOffset += rgFrame->CollapseRowGroupIfNecessary(yTotalOffset, rgWidth);
    ConsiderChildOverflow(overflowArea, rgFrame);
  } 

  aDesiredSize.height -= yTotalOffset;
  aDesiredSize.width   = width;
  overflowArea.UnionRect(nsRect(0, 0, aDesiredSize.width, aDesiredSize.height),
                         overflowArea);
  FinishAndStoreOverflow(&overflowArea,
                         nsSize(aDesiredSize.width, aDesiredSize.height));
}

nscoord
nsTableFrame::GetCollapsedWidth(nsMargin aBorderPadding)
{
  nscoord cellSpacingX = GetCellSpacingX();
  nscoord width = cellSpacingX;
  width += aBorderPadding.left + aBorderPadding.right;
  for (nsIFrame* groupFrame = mColGroups.FirstChild(); groupFrame;
         groupFrame = groupFrame->GetNextSibling()) {
    const nsStyleVisibility* groupVis = groupFrame->GetStyleVisibility();
    PRBool collapseGroup = (NS_STYLE_VISIBILITY_COLLAPSE == groupVis->mVisible);
    nsTableColGroupFrame* cgFrame = (nsTableColGroupFrame*)groupFrame;
    for (nsTableColFrame* colFrame = cgFrame->GetFirstColumn(); colFrame;
         colFrame = colFrame->GetNextCol()) {
      const nsStyleDisplay* colDisplay = colFrame->GetStyleDisplay();
      PRInt32 colX = colFrame->GetColIndex();
      if (NS_STYLE_DISPLAY_TABLE_COLUMN == colDisplay->mDisplay) {
        const nsStyleVisibility* colVis = colFrame->GetStyleVisibility();
        PRBool collapseCol = (NS_STYLE_VISIBILITY_COLLAPSE == colVis->mVisible);
        PRInt32 colWidth = GetColumnWidth(colX);
        if (!collapseGroup && !collapseCol) {
          width += colWidth;
          if (GetNumCellsOriginatingInCol(colX) > 0)
            width += cellSpacingX;
        }
      }
    }
  }
  return width;
}




NS_IMETHODIMP
nsTableFrame::AppendFrames(nsIAtom*        aListName,
                           nsIFrame*       aFrameList)
{
  NS_ASSERTION(!aListName || aListName == nsLayoutAtoms::colGroupList,
               "unexpected child list");

  // Because we actually have two child lists, one for col group frames and one
  // for everything else, we need to look at each frame individually
  // XXX The frame construction code should be separating out child frames
  // based on the type, bug 343048.
  nsIFrame* f = aFrameList;
  while (f) {
    // Get the next frame and disconnect this frame from its sibling
    nsIFrame* next = f->GetNextSibling();
    f->SetNextSibling(nsnull);

    // See what kind of frame we have
    const nsStyleDisplay* display = f->GetStyleDisplay();

    if (NS_STYLE_DISPLAY_TABLE_COLUMN_GROUP == display->mDisplay) {
      nsTableColGroupFrame* lastColGroup;
      PRBool doAppend = nsTableColGroupFrame::GetLastRealColGroup(this, (nsIFrame**) &lastColGroup);
      PRInt32 startColIndex = (lastColGroup) 
        ? lastColGroup->GetStartColumnIndex() + lastColGroup->GetColCount() : 0;
      if (doAppend) {
        // Append the new col group frame
        mColGroups.AppendFrame(nsnull, f);
      }
      else {
        // there is a colgroup after the last real one
          mColGroups.InsertFrame(nsnull, lastColGroup, f);
      }
      // Insert the colgroup and its cols into the table
      InsertColGroups(startColIndex, f, f);
    } else if (IsRowGroup(display->mDisplay)) {
      // Append the new row group frame to the sibling chain
      mFrames.AppendFrame(nsnull, f);

      // insert the row group and its rows into the table
      InsertRowGroups(f, f);
    } else {
      // Nothing special to do, just add the frame to our child list
      mFrames.AppendFrame(nsnull, f);
    }

    // Move to the next frame
    f = next;
  }

#ifdef DEBUG_TABLE_CELLMAP
  printf("=== TableFrame::AppendFrames\n");
  Dump(PR_TRUE, PR_TRUE, PR_TRUE);
#endif
  SetNeedStrategyInit(PR_TRUE); // XXX assume the worse
  AppendDirtyReflowCommand(this);

  return NS_OK;
}

NS_IMETHODIMP
nsTableFrame::InsertFrames(nsIAtom*        aListName,
                           nsIFrame*       aPrevFrame,
                           nsIFrame*       aFrameList)
{
  // Asssume there's only one frame being inserted. The problem is that
  // row group frames and col group frames go in separate child lists and
  // so if there's more than one this gets messy...
  // XXX The frame construction code should be separating out child frames
  // based on the type, bug 343048.
  NS_PRECONDITION(!aFrameList->GetNextSibling(), "expected only one child frame");
  NS_ASSERTION(!aPrevFrame || aPrevFrame->GetParent() == this,
               "inserting after sibling frame with different parent");

  // See what kind of frame we have
  const nsStyleDisplay* display = aFrameList->GetStyleDisplay();

  if (NS_STYLE_DISPLAY_TABLE_COLUMN_GROUP == display->mDisplay) {
    NS_ASSERTION(!aListName || aListName == nsLayoutAtoms::colGroupList,
                 "unexpected child list");
    // Insert the column group frame
    nsFrameList frames(aFrameList); // convience for getting last frame
    nsIFrame* lastFrame = frames.LastChild();
    mColGroups.InsertFrame(nsnull, aPrevFrame, aFrameList);
    // find the starting col index for the first new col group
    PRInt32 startColIndex = 0;
    if (aPrevFrame) {
      nsTableColGroupFrame* prevColGroup = 
        (nsTableColGroupFrame*)GetFrameAtOrBefore(this, aPrevFrame,
                                                  nsLayoutAtoms::tableColGroupFrame);
      if (prevColGroup) {
        startColIndex = prevColGroup->GetStartColumnIndex() + prevColGroup->GetColCount();
      }
    }
    InsertColGroups(startColIndex, aFrameList, lastFrame);
    SetNeedStrategyInit(PR_TRUE);
  } else if (IsRowGroup(display->mDisplay)) {
    NS_ASSERTION(!aListName, "unexpected child list");
    nsFrameList newList(aFrameList);
    nsIFrame* lastSibling = newList.LastChild();
    // Insert the frames in the sibling chain
    mFrames.InsertFrame(nsnull, aPrevFrame, aFrameList);

    InsertRowGroups(aFrameList, lastSibling);
    SetNeedStrategyInit(PR_TRUE);
  } else {
    NS_ASSERTION(!aListName, "unexpected child list");
    // Just insert the frame and don't worry about reflowing it
    mFrames.InsertFrame(nsnull, aPrevFrame, aFrameList);
    return NS_OK;
  }

  AppendDirtyReflowCommand(this);
#ifdef DEBUG_TABLE_CELLMAP
  printf("=== TableFrame::InsertFrames\n");
  Dump(PR_TRUE, PR_TRUE, PR_TRUE);
#endif
  return NS_OK;
}

NS_IMETHODIMP
nsTableFrame::RemoveFrame(nsIAtom*        aListName,
                          nsIFrame*       aOldFrame)
{
  // See what kind of frame we have
  const nsStyleDisplay* display = aOldFrame->GetStyleDisplay();

  // XXX The frame construction code should be separating out child frames
  // based on the type, bug 343048.
  if (NS_STYLE_DISPLAY_TABLE_COLUMN_GROUP == display->mDisplay) {
    NS_ASSERTION(!aListName || aListName == nsLayoutAtoms::colGroupList,
                 "unexpected child list");
    nsIFrame* nextColGroupFrame = aOldFrame->GetNextSibling();
    nsTableColGroupFrame* colGroup = (nsTableColGroupFrame*)aOldFrame;
    PRInt32 firstColIndex = colGroup->GetStartColumnIndex();
    PRInt32 lastColIndex  = firstColIndex + colGroup->GetColCount() - 1;
    mColGroups.DestroyFrame(aOldFrame);
    nsTableColGroupFrame::ResetColIndices(nextColGroupFrame, firstColIndex);
    // remove the cols from the table
    PRInt32 colX;
    for (colX = lastColIndex; colX >= firstColIndex; colX--) {
      nsTableColFrame* colFrame = (nsTableColFrame*)mColFrames.SafeElementAt(colX);
      if (colFrame) {
        RemoveCol(colGroup, colX, PR_TRUE, PR_FALSE);
      }
    }

    PRInt32 numAnonymousColsToAdd = GetColCount() - mColFrames.Count();
    if (numAnonymousColsToAdd > 0) {
      // this sets the child list, updates the col cache and cell map
      CreateAnonymousColFrames(numAnonymousColsToAdd,
                               eColAnonymousCell, PR_TRUE);
    }

    // XXX This could probably be optimized with much effort
    SetNeedStrategyInit(PR_TRUE);
    AppendDirtyReflowCommand(this);
  } else {
    NS_ASSERTION(!aListName, "unexpected child list");
    nsTableRowGroupFrame* rgFrame = GetRowGroupFrame(aOldFrame);
    if (rgFrame) {
      PRInt32 startRowIndex = rgFrame->GetStartRowIndex();
      PRInt32 numRows = rgFrame->GetRowCount();
      // remove the row group from the cell map
      nsTableCellMap* cellMap = GetCellMap();
      if (cellMap) {
        cellMap->RemoveGroupCellMap(rgFrame);
      }
      // only remove cols that are of type eTypeAnonymous cell (they are at the end)
      PRInt32 numColsInMap = GetColCount(); // cell map's notion of num cols
      PRInt32 numColsInCache = mColFrames.Count();
      if (numColsInCache > numColsInMap) {
        PRInt32 numColsNotRemoved = DestroyAnonymousColFrames(numColsInCache - numColsInMap);
        // if the cell map has fewer cols than the cache, correct it
        if (numColsNotRemoved > 0 && cellMap) {
          cellMap->AddColsAtEnd(numColsNotRemoved);
        }
      }
      else NS_ASSERTION(numColsInCache == numColsInMap, "cell map has too many cols");

      AdjustRowIndices(startRowIndex, -numRows);
      // remove the row group frame from the sibling chain
      mFrames.DestroyFrame(aOldFrame);
      if (cellMap) {
        cellMap->Synchronize(this);
        ResetRowIndices();
      }
      // XXX This could probably be optimized with much effort
      SetNeedStrategyInit(PR_TRUE);
      AppendDirtyReflowCommand(this);
    } else {
      // Just remove the frame
      mFrames.DestroyFrame(aOldFrame);
    }
  }
#ifdef DEBUG_TABLE_CELLMAP
  printf("=== TableFrame::RemoveFrame\n");
  Dump(PR_TRUE, PR_TRUE, PR_TRUE);
#endif
  return NS_OK;
}

NS_METHOD 
nsTableFrame::IncrementalReflow(const nsHTMLReflowState& aReflowState,
                                nsReflowStatus&          aStatus)
{
  // Constrain our reflow width to the computed table width. Note: this is
  // based on the width of the first-in-flow
  PRInt32 lastWidth = mRect.width;
  if (GetPrevInFlow()) {
    nsTableFrame* table = (nsTableFrame*)GetFirstInFlow();
    lastWidth = table->mRect.width;
  }
  nsTableReflowState state(*GetPresContext(), aReflowState, *this, eReflowReason_Incremental,
                           lastWidth, aReflowState.availableHeight); 

  // the table is a target if its path has a reflow command
  nsHTMLReflowCommand* command = aReflowState.path->mReflowCommand;
  if (command)
    IR_TargetIsMe(state, aStatus);

  // see if the chidren are targets as well
  nsReflowPath::iterator iter = aReflowState.path->FirstChild();
  nsReflowPath::iterator end  = aReflowState.path->EndChildren();
  for (; iter != end; ++iter)
    IR_TargetIsChild(state, aStatus, *iter);

  return NS_OK;
}

NS_METHOD 
nsTableFrame::IR_TargetIsMe(nsTableReflowState&  aReflowState,
                            nsReflowStatus&      aStatus)
{
  nsresult rv = NS_OK;
  aStatus = NS_FRAME_COMPLETE;

  switch (aReflowState.reflowState.path->mReflowCommand->Type()) {
    case eReflowType_StyleChanged :
      rv = IR_StyleChanged(aReflowState, aStatus);
      break;
    case eReflowType_ContentChanged :
      NS_ASSERTION(PR_FALSE, "illegal reflow type: ContentChanged");
      rv = NS_ERROR_ILLEGAL_VALUE;
      break;
    case eReflowType_ReflowDirty: {
      // reflow the dirty children
      nsTableReflowState reflowState(*GetPresContext(), aReflowState.reflowState, *this, eReflowReason_Initial,
                                     aReflowState.availSize.width, aReflowState.availSize.height); 
      nsIFrame* lastReflowed;
      PRBool reflowedAtLeastOne; 
      nsRect overflowArea;
      ReflowChildren(reflowState, PR_FALSE, PR_TRUE, aStatus,
                     lastReflowed, overflowArea, &reflowedAtLeastOne);
      if (!reflowedAtLeastOne)
        // XXX For now assume the worse
        SetNeedStrategyInit(PR_TRUE);
      }
      break;
    default:
      NS_NOTYETIMPLEMENTED("unexpected reflow command type");
      rv = NS_ERROR_NOT_IMPLEMENTED;
      break;
  }

  return rv;
}

NS_METHOD nsTableFrame::IR_StyleChanged(nsTableReflowState&  aReflowState,
                                        nsReflowStatus&      aStatus)
{
  nsTableReflowState reflowState(*GetPresContext(), aReflowState.reflowState, *this, eReflowReason_StyleChange,
                                 aReflowState.availSize.width, aReflowState.availSize.height); 
  nsIFrame* lastReflowed;
  nsRect overflowArea;
  nsresult rv = ReflowChildren(reflowState, PR_FALSE, PR_FALSE, aStatus, lastReflowed, overflowArea);
  SetNeedStrategyInit(PR_TRUE);
  return rv;
}

static void
DivideBCBorderSize(nscoord  aPixelSize,
                   nscoord& aSmallHalf,
                   nscoord& aLargeHalf)
{
  aSmallHalf = aPixelSize / 2;
  aLargeHalf = aPixelSize - aSmallHalf;
}

nsMargin
nsTableFrame::GetBCBorder() const
{
  nsMargin border(0, 0, 0, 0);
  nsPresContext *presContext = GetPresContext();
  GET_PIXELS_TO_TWIPS(GetPresContext(), p2t);
  BCPropertyData* propData = 
    (BCPropertyData*)nsTableFrame::GetProperty((nsIFrame*)this, nsLayoutAtoms::tableBCProperty, PR_FALSE);
  if (propData) {
    if (eCompatibility_NavQuirks != presContext->CompatibilityMode()) {
      border.top += BC_BORDER_BOTTOM_HALF_COORD(p2t, propData->mTopBorderWidth);
      border.right += BC_BORDER_LEFT_HALF_COORD(p2t, propData->mRightBorderWidth);
      border.bottom += BC_BORDER_TOP_HALF_COORD(p2t, propData->mBottomBorderWidth);
      border.left += BC_BORDER_RIGHT_HALF_COORD(p2t, propData->mLeftBorderWidth);
    }
    else {
      border.top    += NSToCoordRound(p2t * (float)propData->mTopBorderWidth);
      border.right  += NSToCoordRound(p2t * (float)propData->mRightBorderWidth);
      border.bottom += NSToCoordRound(p2t * (float)propData->mBottomBorderWidth);
      border.left   += NSToCoordRound(p2t * (float)propData->mLeftBorderWidth);
    }
  }
  return border;
}

nsMargin
nsTableFrame::GetBCMargin() const
{
  nsMargin overflow(0, 0, 0, 0);
  nsPresContext* presContext = GetPresContext();
  GET_PIXELS_TO_TWIPS(presContext, p2t);
  BCPropertyData* propData =
    (BCPropertyData*)nsTableFrame::GetProperty((nsIFrame*)this,
                                               nsLayoutAtoms::tableBCProperty,
                                               PR_FALSE);
  if (propData) {
    if (eCompatibility_NavQuirks != presContext->CompatibilityMode()) {
      overflow.top += BC_BORDER_TOP_HALF_COORD(p2t, propData->mTopBorderWidth);
      overflow.right += BC_BORDER_RIGHT_HALF_COORD(p2t, propData->mRightBorderWidth);
      overflow.bottom += BC_BORDER_BOTTOM_HALF_COORD(p2t, propData->mBottomBorderWidth);
      overflow.left += BC_BORDER_LEFT_HALF_COORD(p2t, propData->mLeftBorderWidth);
    }
  }
  return overflow;
}
static
void GetSeparateModelBorderPadding(const nsHTMLReflowState* aReflowState,
                                   nsStyleContext&          aStyleContext,
                                   nsMargin&                aBorderPadding)
{
  // XXXbz Either we _do_ have a reflow state and then we can use its
  // mComputedBorderPadding or we don't and then we get the padding
  // wrong!
  const nsStyleBorder* border = aStyleContext.GetStyleBorder();
  aBorderPadding = border->GetBorder();
  if (aReflowState) {
    aBorderPadding += aReflowState->mComputedPadding;
  }
}

nsMargin 
nsTableFrame::GetChildAreaOffset(const nsHTMLReflowState* aReflowState) const
{
  nsMargin offset(0,0,0,0);
  if (IsBorderCollapse()) {
    nsPresContext* presContext = GetPresContext();
    if (eCompatibility_NavQuirks == presContext->CompatibilityMode()) {
      nsTableFrame* firstInFlow = (nsTableFrame*)GetFirstInFlow(); if (!firstInFlow) ABORT1(offset);
      GET_PIXELS_TO_TWIPS(presContext, p2t);
      BCPropertyData* propData = 
        (BCPropertyData*)nsTableFrame::GetProperty((nsIFrame*)firstInFlow, nsLayoutAtoms::tableBCProperty, PR_FALSE);
      if (!propData) ABORT1(offset);

      offset.top += BC_BORDER_TOP_HALF_COORD(p2t, propData->mTopBorderWidth);
      offset.right += BC_BORDER_RIGHT_HALF_COORD(p2t, propData->mRightBorderWidth);
      offset.bottom += BC_BORDER_BOTTOM_HALF_COORD(p2t, propData->mBottomBorderWidth);
      offset.left += BC_BORDER_LEFT_HALF_COORD(p2t, propData->mLeftBorderWidth);
    }
  }
  else {
    GetSeparateModelBorderPadding(aReflowState, *mStyleContext, offset);
  }
  return offset;
}

nsMargin 
nsTableFrame::GetContentAreaOffset(const nsHTMLReflowState* aReflowState) const
{
  nsMargin offset(0,0,0,0);
  if (IsBorderCollapse()) {
    offset = GetBCBorder();
  }
  else {
    GetSeparateModelBorderPadding(aReflowState, *mStyleContext, offset);
  }
  return offset;
}

// Recovers the reflow state to what it should be if aKidFrame is about to be 
// reflowed. Restores y, footerFrame, firstBodySection and availSize.height (if
// the height is constrained)
nsresult
nsTableFrame::RecoverState(nsTableReflowState& aReflowState,
                           nsIFrame*           aKidFrame)
{
  nsMargin borderPadding = GetChildAreaOffset(&aReflowState.reflowState);
  aReflowState.y = borderPadding.top;

  nscoord cellSpacingY = GetCellSpacingY();
  // Get the ordered children and find aKidFrame in the list
  nsAutoVoidArray rowGroups;
  PRUint32 numRowGroups;
  OrderRowGroups(rowGroups, numRowGroups, &aReflowState.firstBodySection);
  
  // Walk the list of children looking for aKidFrame
  for (PRUint32 childX = 0; childX < numRowGroups; childX++) {
    nsIFrame* childFrame = (nsIFrame*)rowGroups.ElementAt(childX);
    nsTableRowGroupFrame* rgFrame = GetRowGroupFrame(childFrame);
    if (!rgFrame) continue; // skip foreign frame types
   
    // If this is a footer row group, remember it
    const nsStyleDisplay* display = rgFrame->GetStyleDisplay();

    // We only allow a single footer frame
    if ((NS_STYLE_DISPLAY_TABLE_FOOTER_GROUP == display->mDisplay) &&
        !aReflowState.footerFrame) {
      aReflowState.footerFrame = childFrame;    
    }
    else {
      if ((NS_STYLE_DISPLAY_TABLE_ROW_GROUP == display->mDisplay) &&
          !aReflowState.firstBodySection) {
        aReflowState.firstBodySection = childFrame;
      }
    }
    aReflowState.y += cellSpacingY;
    
    // See if this is the frame we're looking for
    if (childFrame == aKidFrame) {
      break;
    }

    // Get the frame's height
    nsSize kidSize = childFrame->GetSize();
    
    // If our height is constrained then update the available height. Do
    // this for all frames including the footer frame
    if (NS_UNCONSTRAINEDSIZE != aReflowState.availSize.height) {
      aReflowState.availSize.height -= kidSize.height;
    }

    // Update the running y-offset. Don't do this for the footer frame
    if (childFrame != aReflowState.footerFrame) {
      aReflowState.y += kidSize.height;
    }
  }

  return NS_OK;
}

void
nsTableFrame::InitChildReflowState(nsHTMLReflowState& aReflowState)                                    
{
  nsMargin collapseBorder;
  nsMargin padding(0,0,0,0);
  nsMargin* pCollapseBorder = nsnull;
  nsPresContext* presContext = GetPresContext();
  if (IsBorderCollapse()) {
    nsTableRowGroupFrame* rgFrame = GetRowGroupFrame(aReflowState.frame);
    if (rgFrame) {
      GET_PIXELS_TO_TWIPS(presContext, p2t);
      pCollapseBorder = rgFrame->GetBCBorderWidth(p2t, collapseBorder);
    }
  }
  aReflowState.Init(presContext, -1, -1, pCollapseBorder, &padding);
}

NS_METHOD 
nsTableFrame::IR_TargetIsChild(nsTableReflowState&  aReflowState,
                               nsReflowStatus&      aStatus,
                               nsIFrame*            aNextFrame)

{
  nsresult rv;
  // Recover the state as if aNextFrame is about to be reflowed
  RecoverState(aReflowState, aNextFrame);

  // Remember the old rect
  nsRect oldKidRect = aNextFrame->GetRect();

  // Pass along the reflow command, don't request a max element size, rows will do that
  nsHTMLReflowMetrics desiredSize(PR_FALSE);

  nsSize kidAvailSize(aReflowState.availSize);
  nsPresContext* presContext = GetPresContext();
  nsHTMLReflowState kidReflowState(presContext, aReflowState.reflowState, aNextFrame, 
                                   kidAvailSize, aReflowState.reason);
  InitChildReflowState(kidReflowState);

  rv = ReflowChild(aNextFrame, presContext, desiredSize, kidReflowState,
                   aReflowState.x, aReflowState.y, 0, aStatus);

  // Place the row group frame. Don't use PlaceChild(), because it moves
  // the footer frame as well. We'll adjust the footer frame later on in
  // AdjustSiblingsAfterReflow()
  nsRect  kidRect(aReflowState.x, aReflowState.y, desiredSize.width, desiredSize.height);
  FinishReflowChild(aNextFrame, presContext, nsnull, desiredSize, aReflowState.x, aReflowState.y, 0);

  // Adjust the running y-offset
  aReflowState.y += desiredSize.height + GetCellSpacingY();

  // If our height is constrained, then update the available height
  if (NS_UNCONSTRAINEDSIZE != aReflowState.availSize.height) {
    aReflowState.availSize.height -= desiredSize.height;
  }

  // If the column width info is valid, then adjust the row group frames
  // that follow. Otherwise, return and we'll recompute the column widths
  // and reflow all the row group frames
  if (!NeedsReflow(aReflowState.reflowState)) {
    // If the row group frame changed height, then damage the horizontal strip
    // that was either added or went away
    if (desiredSize.height != oldKidRect.height) {
      nsRect dirtyRect;
      dirtyRect.x = 0;
      dirtyRect.y = PR_MIN(oldKidRect.YMost(), kidRect.YMost());
      dirtyRect.width = mRect.width;
      dirtyRect.height = PR_MAX(oldKidRect.YMost(), kidRect.YMost()) - dirtyRect.y;
      Invalidate(dirtyRect);
    }

    // Adjust the row groups that follow
    AdjustSiblingsAfterReflow(aReflowState, aNextFrame, 
                              desiredSize.height - oldKidRect.height);

    // recover the overflow area from all children
    desiredSize.mOverflowArea = nsRect(0, 0, desiredSize.width, desiredSize.height);
    for (nsIFrame* kidFrame = mFrames.FirstChild(); kidFrame; kidFrame = kidFrame->GetNextSibling()) {
      ConsiderChildOverflow(desiredSize.mOverflowArea, kidFrame);
    }  
    FinishAndStoreOverflow(&desiredSize.mOverflowArea,
                           nsSize(desiredSize.width, desiredSize.height));
  }
  return rv;
}

// Position and size aKidFrame and update our reflow state. The origin of
// aKidRect is relative to the upper-left origin of our frame
void nsTableFrame::PlaceChild(nsTableReflowState&  aReflowState,
                              nsIFrame*            aKidFrame,
                              nsHTMLReflowMetrics& aKidDesiredSize)
{
  
  // Place and size the child
  FinishReflowChild(aKidFrame, GetPresContext(), nsnull, aKidDesiredSize,
                    aReflowState.x, aReflowState.y, 0);

  // Adjust the running y-offset
  aReflowState.y += aKidDesiredSize.height;

  // If our height is constrained, then update the available height
  if (NS_UNCONSTRAINEDSIZE != aReflowState.availSize.height) {
    aReflowState.availSize.height -= aKidDesiredSize.height;
  }

  const nsStyleDisplay* childDisplay = aKidFrame->GetStyleDisplay();

  // We only allow a single footer frame, and the footer frame must occur before
  // any body section row groups
  if ((NS_STYLE_DISPLAY_TABLE_FOOTER_GROUP == childDisplay->mDisplay) &&
      !aReflowState.footerFrame && !aReflowState.firstBodySection) {
    aReflowState.footerFrame = aKidFrame;
  }
  else if (aReflowState.footerFrame) {
    // put the non footer where the footer was
    nsPoint origin = aReflowState.footerFrame->GetPosition();
    aKidFrame->SetPosition(origin);

    // put the footer below the non footer
    origin.y = aReflowState.y - aReflowState.footerFrame->GetSize().height;
    aReflowState.footerFrame->SetPosition(origin);
  }
}

void
nsTableFrame::OrderRowGroups(nsVoidArray&           aChildren,
                             PRUint32&              aNumRowGroups,
                             nsIFrame**             aFirstBody,
                             nsTableRowGroupFrame** aHead,
                             nsTableRowGroupFrame** aFoot) const
{
  aChildren.Clear();
  nsIFrame* head = nsnull;
  nsIFrame* foot = nsnull;
  // initialize out parameters, if present
  if (aFirstBody) *aFirstBody = nsnull;
  if (aHead)      *aHead      = nsnull;
  if (aFoot)      *aFoot      = nsnull;
  
  nsIFrame* kidFrame = mFrames.FirstChild();
  nsAutoVoidArray nonRowGroups;
  // put the tbodies first, and the non row groups last
  while (kidFrame) {
    const nsStyleDisplay* kidDisplay = kidFrame->GetStyleDisplay();
    if (IsRowGroup(kidDisplay->mDisplay)) {
      switch(kidDisplay->mDisplay) {
      case NS_STYLE_DISPLAY_TABLE_HEADER_GROUP:
        if (head) { // treat additional thead like tbody
          aChildren.AppendElement(kidFrame);
        }
        else {
          head = kidFrame;
          if (aHead) {
            *aHead = (nsTableRowGroupFrame*)head;
          }
        }
        break;
      case NS_STYLE_DISPLAY_TABLE_FOOTER_GROUP:
        if (foot) {
          aChildren.AppendElement(kidFrame);
        }
        else {
          foot = kidFrame;
          if (aFoot) {
            *aFoot = (nsTableRowGroupFrame*)foot;
          }
        }
        break;
      default:
        aChildren.AppendElement(kidFrame);
        if (aFirstBody && !*aFirstBody) {
          *aFirstBody = kidFrame;
        }
      }
    }
    else {
      nonRowGroups.AppendElement(kidFrame);
    }
    // Get the next sibling but skip it if it's also the next-in-flow, since
    // a next-in-flow will not be part of the current table.
    while (kidFrame) {
      nsIFrame* nif = kidFrame->GetNextInFlow();
      kidFrame = kidFrame->GetNextSibling();
      if (kidFrame != nif) 
        break;
    }
  }
  aNumRowGroups = aChildren.Count();
  // put the thead first
  if (head) {
    aChildren.InsertElementAt(head, 0);
    aNumRowGroups++;
  }
  // put the tfoot after the last tbody
  if (foot) {
    aChildren.InsertElementAt(foot, aNumRowGroups);
    aNumRowGroups++;
  }
  // put the non row groups at the end
  PRInt32 numNonRowGroups = nonRowGroups.Count();
  for (PRInt32 i = 0; i < numNonRowGroups; i++) {
    aChildren.AppendElement(nonRowGroups.ElementAt(i));
  }
}

static PRBool
IsRepeatable(nsTableRowGroupFrame& aHeaderOrFooter,
             nscoord               aPageHeight)
{
  return aHeaderOrFooter.GetSize().height < (aPageHeight / 4);
}

// Reflow the children based on the avail size and reason in aReflowState
// update aReflowMetrics a aStatus
NS_METHOD 
nsTableFrame::ReflowChildren(nsTableReflowState& aReflowState,
                             PRBool              aDoColGroups,
                             PRBool              aDirtyOnly,
                             nsReflowStatus&     aStatus,
                             nsIFrame*&          aLastChildReflowed,
                             nsRect&             aOverflowArea,
                             PRBool*             aReflowedAtLeastOne)
{
  aStatus = NS_FRAME_COMPLETE;
  aLastChildReflowed = nsnull;

  nsIFrame* prevKidFrame = nsnull;
  nsresult  rv = NS_OK;
  nscoord   cellSpacingY = GetCellSpacingY();

  nsPresContext* presContext = GetPresContext();
  PRBool isPaginated = presContext->IsPaginated();

  aOverflowArea = nsRect (0, 0, 0, 0);
  
  nsAutoVoidArray rowGroups;
  PRUint32 numRowGroups;
  nsTableRowGroupFrame *thead, *tfoot;
  OrderRowGroups(rowGroups, numRowGroups, &aReflowState.firstBodySection, &thead, &tfoot);
  PRBool haveReflowedRowGroup = PR_FALSE;
  PRBool pageBreak = PR_FALSE;
  for (PRUint32 childX = 0; ((PRInt32)childX) < rowGroups.Count(); childX++) {
    nsIFrame* kidFrame = (nsIFrame*)rowGroups.ElementAt(childX);
    // Get the frame state bits
    // See if we should only reflow the dirty child frames
    PRBool doReflowChild = PR_TRUE;
    if (aDirtyOnly && ((kidFrame->GetStateBits() & NS_FRAME_IS_DIRTY) == 0)) {
      doReflowChild = PR_FALSE;
    }

    if (doReflowChild) {
      if (pageBreak) {
        PushChildren(rowGroups, childX);
        aStatus = NS_FRAME_NOT_COMPLETE;
        break;
      }

      nsSize kidAvailSize(aReflowState.availSize);
      // if the child is a tbody in paginated mode reduce the height by a repeated footer
      nsIFrame* repeatedFooter = nsnull;
      nscoord repeatedFooterHeight = 0;
      if (isPaginated && (NS_UNCONSTRAINEDSIZE != kidAvailSize.height)) {
        if (NS_STYLE_DISPLAY_TABLE_ROW_GROUP == kidFrame->GetStyleDisplay()->mDisplay) { // the child is a tbody
          nsIFrame* lastChild = (nsIFrame*)rowGroups.ElementAt(numRowGroups - 1);
          if (NS_STYLE_DISPLAY_TABLE_FOOTER_GROUP == lastChild->GetStyleDisplay()->mDisplay) { // the last child is a tfoot
            if (((nsTableRowGroupFrame*)lastChild)->IsRepeatable()) {
              repeatedFooterHeight = lastChild->GetSize().height;
              if (repeatedFooterHeight + cellSpacingY < kidAvailSize.height) {
                repeatedFooter = lastChild;
                kidAvailSize.height -= repeatedFooterHeight + cellSpacingY;
              }
            }
          }
        }
      }

      nsHTMLReflowMetrics desiredSize(PR_FALSE);
      desiredSize.width = desiredSize.height = desiredSize.ascent = desiredSize.descent = 0;
  
      if (childX < numRowGroups) {  
        // Reflow the child into the available space
        nsHTMLReflowState  kidReflowState(presContext, aReflowState.reflowState, kidFrame, 
                                          kidAvailSize, aReflowState.reason);
        InitChildReflowState(kidReflowState);
        // XXX fix up bad mComputedWidth for scroll frame
        kidReflowState.mComputedWidth = PR_MAX(kidReflowState.mComputedWidth, 0);
  
        // If this isn't the first row group, then we can't be at the top of the page
        if (childX > 0) {
          kidReflowState.mFlags.mIsTopOfPage = PR_FALSE;
        }
        aReflowState.y += cellSpacingY;
        if (NS_UNCONSTRAINEDSIZE != aReflowState.availSize.height) {
          aReflowState.availSize.height -= cellSpacingY;
        }
        // record the presence of a next in flow, it might get destroyed so we
        // need to reorder the row group array
        nsIFrame* kidNextInFlow = kidFrame->GetNextInFlow();
        PRBool reorder = PR_FALSE;
        if (kidFrame->GetNextInFlow())
          reorder = PR_TRUE;
      
        rv = ReflowChild(kidFrame, presContext, desiredSize, kidReflowState,
                         aReflowState.x, aReflowState.y, 0, aStatus);
        haveReflowedRowGroup = PR_TRUE;

        if (reorder) {
          // reorder row groups the reflow may have changed the nextinflows
          OrderRowGroups(rowGroups, numRowGroups, &aReflowState.firstBodySection, &thead, &tfoot);
          for (childX = 0; childX < numRowGroups; childX++) {
            if (kidFrame == (nsIFrame*)rowGroups.ElementAt(childX))
              break;
          }
        }
        // see if the rowgroup did not fit on this page might be pushed on
        // the next page
        if (NS_FRAME_IS_COMPLETE(aStatus) && isPaginated &&
            (NS_UNCONSTRAINEDSIZE != kidReflowState.availableHeight) &&
            kidReflowState.availableHeight < desiredSize.height) {
          // if we are on top of the page place with dataloss
          if (kidReflowState.mFlags.mIsTopOfPage) {
            if (childX+1 < numRowGroups) {
              nsIFrame* nextRowGroupFrame = (nsIFrame*) rowGroups.ElementAt(childX +1);
              if (nextRowGroupFrame) {
                PlaceChild(aReflowState, kidFrame, desiredSize);
                aStatus = NS_FRAME_NOT_COMPLETE;
                PushChildren(rowGroups, childX + 1);
                aLastChildReflowed = kidFrame;
                break;
              }
            }
          }
          else { // we are not on top, push this rowgroup onto the next page
            if (prevKidFrame) { // we had a rowgroup before so push this
              aStatus = NS_FRAME_NOT_COMPLETE;
              PushChildren(rowGroups, childX);
              aLastChildReflowed = prevKidFrame;
              break;
            }
          }
        }

        aLastChildReflowed   = kidFrame;

        pageBreak = PR_FALSE;
        // see if there is a page break after this row group or before the next one
        if (NS_FRAME_IS_COMPLETE(aStatus) && isPaginated && 
            (NS_UNCONSTRAINEDSIZE != kidReflowState.availableHeight)) {
          nsIFrame* nextKid = (childX + 1 < numRowGroups) ? (nsIFrame*)rowGroups.ElementAt(childX + 1) : nsnull;
          pageBreak = PageBreakAfter(*kidFrame, nextKid);
        }

        // Place the child
        PlaceChild(aReflowState, kidFrame, desiredSize);
  
        // Remember where we just were in case we end up pushing children
        prevKidFrame = kidFrame;
 
        // Special handling for incomplete children
        if (NS_FRAME_IS_NOT_COMPLETE(aStatus)) {         
          kidNextInFlow = kidFrame->GetNextInFlow();
          if (!kidNextInFlow) {
            // The child doesn't have a next-in-flow so create a continuing
            // frame. This hooks the child into the flow
            nsIFrame*     continuingFrame;

            presContext->PresShell()->FrameConstructor()->
              CreateContinuingFrame(presContext, kidFrame, this,
                                    &continuingFrame);
  
            // Add the continuing frame to the sibling list
            continuingFrame->SetNextSibling(kidFrame->GetNextSibling());
            kidFrame->SetNextSibling(continuingFrame);
            // Update rowGroups with the new rowgroup, just as it
            // would have been if we had called OrderRowGroups
            // again. Note that rowGroups doesn't get used again after
            // we PushChildren below, anyway.
            rowGroups.InsertElementAt(continuingFrame, childX + 1);
          }
          // We've used up all of our available space so push the remaining
          // children to the next-in-flow
          nsIFrame* nextSibling = kidFrame->GetNextSibling();
          if (nsnull != nextSibling) {
            PushChildren(rowGroups, childX + 1);
          }
          if (repeatedFooter) {
            kidAvailSize.height = repeatedFooterHeight;
            nsHTMLReflowState footerReflowState(presContext, aReflowState.reflowState, repeatedFooter, 
                                                kidAvailSize, aReflowState.reason);
            InitChildReflowState(footerReflowState);
            aReflowState.y += cellSpacingY;
            nsReflowStatus footerStatus;
            rv = ReflowChild(repeatedFooter, presContext, desiredSize, footerReflowState,
                             aReflowState.x, aReflowState.y, 0, footerStatus);
            PlaceChild(aReflowState, repeatedFooter, desiredSize);
          }
          break;
        }
      }
    }
    else if (childX < numRowGroups) { // it is a row group but isn't being reflowed
      nsRect kidRect = kidFrame->GetRect();
      if (haveReflowedRowGroup) { 
        if (kidRect.y != aReflowState.y) {
          Invalidate(kidRect); // invalidate the old position
          kidRect.y = aReflowState.y;
          kidFrame->SetRect(kidRect);        // move to the new position
          Invalidate(kidRect); // invalidate the new position
        }
      }
      aReflowState.y += cellSpacingY + kidRect.height;
    }
    ConsiderChildOverflow(aOverflowArea, kidFrame);
  }
  
  // if required, give the colgroups their initial reflows
  if (aDoColGroups) {
    nsHTMLReflowMetrics kidMet(PR_FALSE);
    for (nsIFrame* kidFrame = mColGroups.FirstChild(); kidFrame;
         kidFrame = kidFrame->GetNextSibling()) {
      nsHTMLReflowState kidReflowState(presContext, aReflowState.reflowState, kidFrame,
                                       aReflowState.availSize, aReflowState.reason);
      nsReflowStatus cgStatus;
      ReflowChild(kidFrame, presContext, kidMet, kidReflowState, 0, 0, 0, cgStatus);
      FinishReflowChild(kidFrame, presContext, nsnull, kidMet, 0, 0, 0);
    }
    SetHaveReflowedColGroups(PR_TRUE);
  }

  // set the repeatablility of headers and footers in the original table during its first reflow
  // the repeatability of header and footers on continued tables is handled when they are created
  if (isPaginated && !GetPrevInFlow() && (NS_UNCONSTRAINEDSIZE == aReflowState.availSize.height)) {
    nscoord height = presContext->GetPageSize().height;
    // don't repeat the thead or tfoot unless it is < 25% of the page height
    if (thead && height != NS_UNCONSTRAINEDSIZE) {
      thead->SetRepeatable(IsRepeatable(*thead, height));
    }
    if (tfoot && height != NS_UNCONSTRAINEDSIZE) {
      tfoot->SetRepeatable(IsRepeatable(*tfoot, height));
    }
  }

  if (aReflowedAtLeastOne) {
    *aReflowedAtLeastOne = haveReflowedRowGroup;
  }
  return rv;
}

/**
  Now I've got all the cells laid out in an infinite space.
  For each column, use the min size for each cell in that column
  along with the attributes of the table, column group, and column
  to assign widths to each column.
  */
// use the cell map to determine which cell is in which column.
void nsTableFrame::BalanceColumnWidths(const nsHTMLReflowState& aReflowState)
{
  NS_ASSERTION(!GetPrevInFlow(), "never ever call me on a continuing frame!");

  // fixed-layout tables need to reinitialize the layout strategy. When there are scroll bars
  // reflow gets called twice and the 2nd time has the correct space available.
  // XXX this is very bad and needs to be changed
  if (!IsAutoLayout()) {
    mTableLayoutStrategy->Initialize(aReflowState);
  }

  // need to figure out the overall table width constraint
  // default case, get 100% of available space

  mTableLayoutStrategy->BalanceColumnWidths(aReflowState);
  //Dump(PR_TRUE, PR_TRUE);
  SetNeedStrategyBalance(PR_FALSE);                    // we have just balanced
  // cache the min, desired, and preferred widths
  nscoord minWidth, prefWidth;
  CalcMinAndPreferredWidths(aReflowState, PR_FALSE, minWidth, prefWidth);
  SetMinWidth(minWidth); 
  nscoord desWidth = CalcDesiredWidth(aReflowState);
  SetDesiredWidth(desWidth);          
  SetPreferredWidth(prefWidth); 

}

// This width is based on the column widths array of the table.
// sum the width of each column and add in table insets
nscoord 
nsTableFrame::CalcDesiredWidth(const nsHTMLReflowState& aReflowState)
{
  NS_ASSERTION(!GetPrevInFlow(), "never ever call me on a continuing frame!");
  nsTableCellMap* cellMap = GetCellMap();
  if (!cellMap) {
    NS_ASSERTION(PR_FALSE, "never ever call me until the cell map is built!");
    return 0;
  }

  nscoord cellSpacing = GetCellSpacingX();
  PRInt32 tableWidth  = 0;

  PRInt32 numCols = GetColCount();
  for (PRInt32 colIndex = 0; colIndex < numCols; colIndex++) {
    nscoord totalColWidth = GetColumnWidth(colIndex);
    if (GetNumCellsOriginatingInCol(colIndex) > 0) { // skip degenerate cols
      totalColWidth += cellSpacing;           // add cell spacing to left of col
    }
    tableWidth += totalColWidth;
  }

  if (numCols > 0)
    tableWidth += cellSpacing; // add last cellspacing

  PRBool isPctWidth = PR_FALSE;
  nscoord compWidth = aReflowState.mComputedWidth;
  if (!IsAutoWidth(&isPctWidth) &&
      (NS_UNCONSTRAINEDSIZE != compWidth) && !isPctWidth)
    tableWidth = PR_MAX(tableWidth, compWidth);

  // Add the width between the border edge and the child area
  nsMargin childOffset = GetChildAreaOffset(&aReflowState);
  tableWidth += childOffset.left + childOffset.right;

  return tableWidth;
}


void 
nsTableFrame::CalcDesiredHeight(const nsHTMLReflowState& aReflowState, nsHTMLReflowMetrics& aDesiredSize) 
{
  nsTableCellMap* cellMap = GetCellMap();
  if (!cellMap) {
    NS_ASSERTION(PR_FALSE, "never ever call me until the cell map is built!");
    aDesiredSize.height = 0;
    return;
  }
  nscoord  cellSpacingY = GetCellSpacingY();
  nsMargin borderPadding = GetChildAreaOffset(&aReflowState);

  // get the natural height based on the last child's (row group or scroll frame) rect
  nsAutoVoidArray rowGroups;
  PRUint32 numRowGroups;
  OrderRowGroups(rowGroups, numRowGroups, nsnull);
  if (numRowGroups <= 0) {
    // tables can be used as rectangular items without content
    nscoord tableSpecifiedHeight = CalcBorderBoxHeight(aReflowState);
    if ((NS_UNCONSTRAINEDSIZE != tableSpecifiedHeight) &&
        (tableSpecifiedHeight > 0) &&
        eCompatibility_NavQuirks != GetPresContext()->CompatibilityMode()) {
          // empty tables should not have a size in quirks mode
      aDesiredSize.height = tableSpecifiedHeight;
    } 
    else
      aDesiredSize.height = 0;
    return;
  }
  PRInt32 rowCount = cellMap->GetRowCount();
  PRInt32 colCount = cellMap->GetColCount();
  nscoord desiredHeight = borderPadding.top + borderPadding.bottom;
  if (rowCount > 0 && colCount > 0) {
    desiredHeight += cellSpacingY;
    for (PRUint32 rgX = 0; rgX < numRowGroups; rgX++) {
      nsIFrame* rg = (nsIFrame*)rowGroups.ElementAt(rgX);
      if (rg) {
        desiredHeight += rg->GetSize().height + cellSpacingY;
      }
    }
  }

  // see if a specified table height requires dividing additional space to rows
  if (!GetPrevInFlow()) {
    nscoord tableSpecifiedHeight = CalcBorderBoxHeight(aReflowState);
    if ((tableSpecifiedHeight > 0) && 
        (tableSpecifiedHeight != NS_UNCONSTRAINEDSIZE) &&
        (tableSpecifiedHeight > desiredHeight)) {
      // proportionately distribute the excess height to unconstrained rows in each
      // unconstrained row group.We don't need to do this if it's an unconstrained reflow
      if (NS_UNCONSTRAINEDSIZE != aReflowState.availableWidth) { 
        DistributeHeightToRows(aReflowState, tableSpecifiedHeight - desiredHeight);
        // this might have changed the overflow area incorporate the childframe overflow area.
        for (nsIFrame* kidFrame = mFrames.FirstChild(); kidFrame; kidFrame = kidFrame->GetNextSibling()) {
          ConsiderChildOverflow(aDesiredSize.mOverflowArea, kidFrame);
        } 
      }
      desiredHeight = tableSpecifiedHeight;
    }
  }
  aDesiredSize.height = desiredHeight;
}

static
void ResizeCells(nsTableFrame&            aTableFrame,
                 const nsHTMLReflowState& aReflowState)
{
  nsAutoVoidArray rowGroups;
  PRUint32 numRowGroups;
  aTableFrame.OrderRowGroups(rowGroups, numRowGroups, nsnull);
  nsHTMLReflowMetrics tableDesiredSize(PR_FALSE);
  nsRect tableRect = aTableFrame.GetRect();
  tableDesiredSize.width = tableRect.width;
  tableDesiredSize.height = tableRect.height;
  tableDesiredSize.mOverflowArea = nsRect(0, 0, tableRect.width,
                                          tableRect.height);

  for (PRUint32 rgX = 0; (rgX < numRowGroups); rgX++) {
    nsTableRowGroupFrame* rgFrame = aTableFrame.GetRowGroupFrame((nsIFrame*)rowGroups.ElementAt(rgX));
   
    nsRect rowGroupRect = rgFrame->GetRect();
    nsHTMLReflowMetrics groupDesiredSize(PR_FALSE);
    groupDesiredSize.width = rowGroupRect.width;
    groupDesiredSize.height = rowGroupRect.height;
    groupDesiredSize.mOverflowArea = nsRect(0, 0, groupDesiredSize.width,
                                      groupDesiredSize.height);
    nsTableRowFrame* rowFrame = rgFrame->GetFirstRow();
    while (rowFrame) {
      rowFrame->DidResize(aReflowState);
      rgFrame->ConsiderChildOverflow(groupDesiredSize.mOverflowArea, rowFrame);
      rowFrame = rowFrame->GetNextRow();
    }
    rgFrame->FinishAndStoreOverflow(&groupDesiredSize.mOverflowArea,
                                    nsSize(groupDesiredSize.width, groupDesiredSize.height));
    // make the coordinates of |desiredSize.mOverflowArea| incorrect
    // since it's about to go away:
    groupDesiredSize.mOverflowArea.MoveBy(rgFrame->GetPosition());
    tableDesiredSize.mOverflowArea.UnionRect(tableDesiredSize.mOverflowArea, groupDesiredSize.mOverflowArea);
  }
  aTableFrame.FinishAndStoreOverflow(&tableDesiredSize.mOverflowArea,
                                     nsSize(tableDesiredSize.width, tableDesiredSize.height));
}

void
nsTableFrame::DistributeHeightToRows(const nsHTMLReflowState& aReflowState,
                                     nscoord                  aAmount)
{ 
 
  GET_PIXELS_TO_TWIPS(GetPresContext(), p2t);

  nscoord cellSpacingY = GetCellSpacingY();

  nsMargin borderPadding = GetChildAreaOffset(&aReflowState);
  
  nsVoidArray rowGroups;
  PRUint32 numRowGroups;
  OrderRowGroups(rowGroups, numRowGroups, nsnull);

  nscoord amountUsed = 0;
  // distribute space to each pct height row whose row group doesn't have a computed 
  // height, and base the pct on the table height. If the row group had a computed 
  // height, then this was already done in nsTableRowGroupFrame::CalculateRowHeights
  nscoord pctBasis = aReflowState.mComputedHeight - (GetCellSpacingY() * (GetRowCount() + 1));
  nscoord yOriginRG = borderPadding.top + GetCellSpacingY();
  nscoord yEndRG = yOriginRG;
  PRUint32 rgX;
  for (rgX = 0; (rgX < numRowGroups); rgX++) {
    nsTableRowGroupFrame* rgFrame = GetRowGroupFrame((nsIFrame*)rowGroups.ElementAt(rgX));
    nscoord amountUsedByRG = 0;
    nscoord yOriginRow = 0;
    nsRect rgRect = rgFrame->GetRect();
    if (rgFrame && !rgFrame->HasStyleHeight()) {
      nsTableRowFrame* rowFrame = rgFrame->GetFirstRow();
      while (rowFrame) {
        nsRect rowRect = rowFrame->GetRect();
        if ((amountUsed < aAmount) && rowFrame->HasPctHeight()) {
          nscoord pctHeight = nsTableFrame::RoundToPixel(rowFrame->GetHeight(pctBasis), p2t);
          nscoord amountForRow = PR_MIN(aAmount - amountUsed, pctHeight - rowRect.height);
          if (amountForRow > 0) {
            rowRect.height += amountForRow;
            rowFrame->SetRect(rowRect);
            yOriginRow += rowRect.height + cellSpacingY;
            yEndRG += rowRect.height + cellSpacingY;
            amountUsed += amountForRow;
            amountUsedByRG += amountForRow;
            //rowFrame->DidResize(aReflowState);        
            nsTableFrame::RePositionViews(rowFrame);
          }
        }
        else {
          if (amountUsed > 0) {
            rowFrame->SetPosition(nsPoint(rowRect.x, yOriginRow));
            nsTableFrame::RePositionViews(rowFrame);
          }
          yOriginRow += rowRect.height + cellSpacingY;
          yEndRG += rowRect.height + cellSpacingY;
        }
        rowFrame = rowFrame->GetNextRow();
      }
      if (amountUsed > 0) {
        rgRect.y = yOriginRG;
        rgRect.height += amountUsedByRG;
        rgFrame->SetRect(rgRect);
      }
    }
    else if (amountUsed > 0) {
      rgFrame->SetPosition(nsPoint(0, yOriginRG));
      // Make sure child views are properly positioned
      nsTableFrame::RePositionViews(rgFrame);
    }
    yOriginRG = yEndRG;
  }

  if (amountUsed >= aAmount) {
    ResizeCells(*this, aReflowState);
    return;
  }

  // get the first row without a style height where its row group has an unconstrianed height
  nsTableRowGroupFrame* firstUnStyledRG  = nsnull;
  nsTableRowFrame*      firstUnStyledRow = nsnull;
  for (rgX = 0; (rgX < numRowGroups) && !firstUnStyledRG; rgX++) {
    nsTableRowGroupFrame* rgFrame = GetRowGroupFrame((nsIFrame*)rowGroups.ElementAt(rgX));
    if (rgFrame && !rgFrame->HasStyleHeight()) {
      nsTableRowFrame* rowFrame = rgFrame->GetFirstRow();
      while (rowFrame) {
        if (!rowFrame->HasStyleHeight()) {
          firstUnStyledRG = rgFrame;
          firstUnStyledRow = rowFrame;
          break;
        }
        rowFrame = rowFrame->GetNextRow();
      }
    }
  }

  nsTableRowFrame* lastElligibleRow = nsnull;
  // accumulate the correct divisor. This will be the total of all unstyled rows inside 
  // unstyled row groups, unless there are none, in which case, it will be all rows
  nscoord divisor = 0;
  for (rgX = 0; rgX < numRowGroups; rgX++) {
    nsTableRowGroupFrame* rgFrame = GetRowGroupFrame((nsIFrame*)rowGroups.ElementAt(rgX));
    if (rgFrame && (!firstUnStyledRG || !rgFrame->HasStyleHeight())) {
      nsTableRowFrame* rowFrame = rgFrame->GetFirstRow();
      while (rowFrame) {
        if (!firstUnStyledRG || !rowFrame->HasStyleHeight()) {
          divisor += rowFrame->GetSize().height;
          lastElligibleRow = rowFrame;
        }
        rowFrame = rowFrame->GetNextRow();
      }
    }
  }
  if (divisor <= 0) {
    NS_ERROR("invalid divisor");
    return;
  }

  // allocate the extra height to the unstyled row groups and rows
  pctBasis = aAmount - amountUsed;
  yOriginRG = borderPadding.top + cellSpacingY;
  yEndRG = yOriginRG;
  for (rgX = 0; rgX < numRowGroups; rgX++) {
    nsTableRowGroupFrame* rgFrame = GetRowGroupFrame((nsIFrame*)rowGroups.ElementAt(rgX));
    if (!rgFrame) continue; 
    nscoord amountUsedByRG = 0;
    nscoord yOriginRow = 0;
    nsRect rgRect = rgFrame->GetRect();
    // see if there is an eligible row group
    if (!firstUnStyledRG || !rgFrame->HasStyleHeight()) {
      nsTableRowFrame* rowFrame = rgFrame->GetFirstRow();
      while (rowFrame) {
        nsRect rowRect = rowFrame->GetRect();
        // see if there is an eligible row
        if (!firstUnStyledRow || !rowFrame->HasStyleHeight()) {
          // The amount of additional space each row gets is proportional to its height
          float percent = rowRect.height / ((float)divisor);
          // give rows their percentage, except for the last row which gets the remainder
          nscoord amountForRow = (rowFrame == lastElligibleRow) 
                                 ? aAmount - amountUsed : NSToCoordRound(((float)(pctBasis)) * percent);
          amountForRow = PR_MIN(nsTableFrame::RoundToPixel(amountForRow, p2t), aAmount - amountUsed);
          // update the row height
          nsRect newRowRect(rowRect.x, yOriginRow, rowRect.width, rowRect.height + amountForRow);
          rowFrame->SetRect(newRowRect);
          yOriginRow += newRowRect.height + cellSpacingY;
          yEndRG += newRowRect.height + cellSpacingY;

          amountUsed += amountForRow;
          amountUsedByRG += amountForRow;
          NS_ASSERTION((amountUsed <= aAmount), "invalid row allocation");
          //rowFrame->DidResize(aReflowState);        
          nsTableFrame::RePositionViews(rowFrame);
        }
        else {
          if (amountUsed > 0) {
            rowFrame->SetPosition(nsPoint(rowRect.x, yOriginRow));
            nsTableFrame::RePositionViews(rowFrame);
          }
          yOriginRow += rowRect.height + cellSpacingY;
          yEndRG += rowRect.height + cellSpacingY;
        }
        rowFrame = rowFrame->GetNextRow();
      }
      if (amountUsed > 0) {
        rgRect.y = yOriginRG;
        rgRect.height += amountUsedByRG;
        rgFrame->SetRect(rgRect);
      }
      // Make sure child views are properly positioned
      // XXX what happens if childFrame is a scroll frame and this gets skipped? see also below
    }
    else if (amountUsed > 0) {
      rgFrame->SetPosition(nsPoint(0, yOriginRG));
      // Make sure child views are properly positioned
      nsTableFrame::RePositionViews(rgFrame);
    }
    yOriginRG = yEndRG;
  }

  ResizeCells(*this, aReflowState);
}

static void
UpdateCol(nsTableFrame&           aTableFrame,
          nsTableColFrame&        aColFrame,
          const nsTableCellFrame& aCellFrame,
          nscoord                 aColMaxWidth,
          PRBool                  aColMaxGetsBigger)
{
  if (aColMaxGetsBigger) {
    // update the columns's new min width
    aColFrame.SetWidth(DES_CON, aColMaxWidth);
  }
  else {
    // determine the new max width
    PRInt32 numRows = aTableFrame.GetRowCount();
    PRInt32 colIndex = aColFrame.GetColIndex();
    PRBool originates;
    PRInt32 colSpan;
    nscoord maxWidth = 0;
    for (PRInt32 rowX = 0; rowX < numRows; rowX++) {
      nsTableCellFrame* cellFrame = aTableFrame.GetCellInfoAt(rowX, colIndex, &originates, &colSpan);
      if (cellFrame && originates && (1 == colSpan)) {
        maxWidth = PR_MAX(maxWidth, cellFrame->GetMaximumWidth());
      }
    }
    // update the columns's new max width
    aColFrame.SetWidth(DES_CON, maxWidth);
  }
}

PRBool 
nsTableFrame::IsPctHeight(nsStyleContext* aStyleContext) 
{
  PRBool result = PR_FALSE;
  if (aStyleContext) {
    result = (eStyleUnit_Percent ==
              aStyleContext->GetStylePosition()->mHeight.GetUnit());
  }
  return result;
}

PRBool 
nsTableFrame::CellChangedWidth(const nsTableCellFrame& aCellFrame,
                               nscoord                 aPrevCellMin,
                               nscoord                 aPrevCellMax,
                               PRBool                  aCellWasDestroyed)
{
  if (NeedStrategyInit() || !IsAutoLayout()) {
    // if the strategy needs to be initialized, all of the col info will be updated later
    // fixed layout tables do not cause any rebalancing
    return PR_TRUE;
  }

  nscoord colSpan = GetEffectiveColSpan(aCellFrame);
  if (colSpan > 1) {
    // colspans are too complicated to optimize, so just bail out
    SetNeedStrategyInit(PR_TRUE);
    return PR_TRUE;
  }

  PRInt32 rowX, colIndex, numRows;
  aCellFrame.GetColIndex(colIndex);
  
  PRBool originates;

  nsTableColFrame* colFrame = GetColFrame(colIndex);
  if (!colFrame) return PR_TRUE; // should never happen

  nscoord cellMin = (aCellWasDestroyed) ? 0 : aCellFrame.GetPass1MaxElementWidth();
  nscoord cellMax = (aCellWasDestroyed) ? 0 : aCellFrame.GetMaximumWidth();
  nscoord colMin  = colFrame->GetWidth(MIN_CON);
  nscoord colMax  = colFrame->GetWidth(DES_CON);

  PRBool colMinGetsBigger  = (cellMin > colMin);
  PRBool colMinGetsSmaller = (cellMin < colMin) && (colMin == aPrevCellMin);

  if (colMinGetsBigger || colMinGetsSmaller) {
    if (ColIsSpannedInto(colIndex) || ColHasSpanningCells(colIndex)) {
      // bail out if a colspan is involved
      SetNeedStrategyInit(PR_TRUE);
      return PR_TRUE;
    }
    if (colMinGetsBigger) {
      // update the columns's min width
      colFrame->SetWidth(MIN_CON, cellMin);
    }
    else if (colMinGetsSmaller) {
      // determine the new min width
      numRows = GetRowCount();
      nscoord minWidth = 0;
      for (rowX = 0; rowX < numRows; rowX++) {
        nsTableCellFrame* cellFrame = GetCellInfoAt(rowX, colIndex, &originates, &colSpan);
        if (cellFrame && originates && (1 == colSpan)) {
          minWidth = PR_MAX(minWidth, cellFrame->GetPass1MaxElementWidth());
        }
      }
      // update the columns's new min width
      colFrame->SetWidth(MIN_CON, minWidth);
    }
    // we should rebalance in case the min width determines the column width
    SetNeedStrategyBalance(PR_TRUE);
  }

  PRBool colMaxGetsBigger  = (cellMax > colMax);
  PRBool colMaxGetsSmaller = (cellMax < colMax) && (colMax == aPrevCellMax);

  if (colMaxGetsBigger || colMaxGetsSmaller) {
    if (ColIsSpannedInto(colIndex)) {
      // bail out if a colspan is involved
      SetNeedStrategyInit(PR_TRUE);
      return PR_TRUE;
    }
    // see if the max width will be not be overshadowed by a pct, fix, or proportional width
    if ((colFrame->GetWidth(PCT) <= 0) && (colFrame->GetWidth(FIX) <= 0) &&
        (colFrame->GetWidth(MIN_PRO) <= 0)) {
      // see if the doesn't have a pct width
      const nsStylePosition* cellPosition = aCellFrame.GetStylePosition();
      // see if there isn't a pct width on the cell
      PRBool havePct = PR_FALSE;
      if (eStyleUnit_Percent == cellPosition->mWidth.GetUnit()) {
        float percent = cellPosition->mWidth.GetPercentValue();
        if (percent > 0.0f) {
          havePct = PR_TRUE;
        }
      }
      if (!havePct) {
        // see if there isn't a fix width on the cell
        PRBool haveFix = PR_FALSE;
        if (eStyleUnit_Coord == cellPosition->mWidth.GetUnit()) {
          nscoord coordValue = cellPosition->mWidth.GetCoordValue();
          if (coordValue > 0) { 
            haveFix = PR_TRUE;
          }
        }
        if (!haveFix) {
          // see if there isn't a prop width on the cell
          PRBool haveProp = PR_FALSE;
          if (eStyleUnit_Proportional == cellPosition->mWidth.GetUnit()) {
            nscoord intValue = cellPosition->mWidth.GetIntValue();
            if (intValue > 0) { 
              haveProp = PR_TRUE;
            }
          }
          if (!haveProp) {
            UpdateCol(*this, *colFrame, aCellFrame, cellMax, colMaxGetsBigger);
            // we should rebalance in case the max width determines the column width
            SetNeedStrategyBalance(PR_TRUE);
          }
        }
      }
    }
    else {
      UpdateCol(*this, *colFrame, aCellFrame, cellMax, colMaxGetsBigger);
    }
  }
  return PR_FALSE;
}

void nsTableFrame::SetNeedStrategyBalance(PRBool aValue)
{
  nsTableFrame* firstInFlow = (nsTableFrame *)GetFirstInFlow();
  NS_ASSERTION(firstInFlow, "illegal state -- no first in flow");
  firstInFlow->mBits.mNeedStrategyBalance = aValue;
}

PRBool nsTableFrame::NeedStrategyBalance() const
{
  nsTableFrame* firstInFlow = (nsTableFrame *)GetFirstInFlow();
  NS_ASSERTION(firstInFlow, "illegal state -- no first in flow");
  return (PRBool)firstInFlow->mBits.mNeedStrategyBalance;
}

void nsTableFrame::SetNeedStrategyInit(PRBool aValue)
{
  nsTableFrame* firstInFlow = (nsTableFrame *)GetFirstInFlow();
  NS_ASSERTION(firstInFlow, "illegal state -- no first in flow");
  firstInFlow->mBits.mNeedStrategyInit = aValue;
}

PRBool nsTableFrame::NeedStrategyInit() const
{
  nsTableFrame* firstInFlow = (nsTableFrame *)GetFirstInFlow();
  NS_ASSERTION(firstInFlow, "illegal state -- no first in flow");
  return (PRBool)firstInFlow->mBits.mNeedStrategyInit;
}

void nsTableFrame::SetResizeReflow(PRBool aValue)
{
  nsTableFrame* firstInFlow = (nsTableFrame *)GetFirstInFlow();
  NS_ASSERTION(firstInFlow, "illegal state -- no first in flow");
  firstInFlow->mBits.mDidResizeReflow = aValue;
}

PRBool nsTableFrame::DidResizeReflow() const
{
  nsTableFrame* firstInFlow = (nsTableFrame *)GetFirstInFlow();
  NS_ASSERTION(firstInFlow, "illegal state -- no first in flow");
  return (PRBool)firstInFlow->mBits.mDidResizeReflow;
}

PRInt32 nsTableFrame::GetColumnWidth(PRInt32 aColIndex)
{
  nsTableFrame * firstInFlow = (nsTableFrame *)GetFirstInFlow();
  NS_ASSERTION(firstInFlow, "illegal state -- no first in flow");
  PRInt32 result = 0;
  if (this == firstInFlow) {
    nsTableColFrame* colFrame = GetColFrame(aColIndex);
    if (colFrame) {
      result = colFrame->GetWidth(FINAL);
    }
  }
  else {
    result = firstInFlow->GetColumnWidth(aColIndex);
  }

  return result;
}

void nsTableFrame::SetColumnWidth(PRInt32 aColIndex, nscoord aWidth)
{
  nsTableFrame* firstInFlow = (nsTableFrame *)GetFirstInFlow();
  NS_ASSERTION(firstInFlow, "illegal state -- no first in flow");

  if (this == firstInFlow) {
    nsTableColFrame* colFrame = GetColFrame(aColIndex);
    if (colFrame) {
      colFrame->SetWidth(FINAL, aWidth);
    }
    else {
      NS_ASSERTION(PR_FALSE, "null col frame");
    }
  }
  else {
    firstInFlow->SetColumnWidth(aColIndex, aWidth);
  }
}


nscoord 
CalcPercentPadding(nscoord      aBasis,
                   nsStyleCoord aStyleCoord)
{
  float percent = (NS_UNCONSTRAINEDSIZE == aBasis)
                  ? 0 : aStyleCoord.GetPercentValue();
  return NSToCoordRound(((float)aBasis) * percent);
}

void 
GetPaddingFor(const nsSize&         aBasis, 
              const nsStylePadding& aPaddingData, 
              nsMargin&             aPadding)
{
  nsStyleCoord styleCoord;
  aPaddingData.mPadding.GetTop(styleCoord);
  if (eStyleUnit_Percent == aPaddingData.mPadding.GetTopUnit()) {
    aPadding.top = CalcPercentPadding(aBasis.height, styleCoord);
  }
  else if (eStyleUnit_Coord == aPaddingData.mPadding.GetTopUnit()) {
    aPadding.top = styleCoord.GetCoordValue();
  }

  aPaddingData.mPadding.GetRight(styleCoord);
  if (eStyleUnit_Percent == aPaddingData.mPadding.GetRightUnit()) {
    aPadding.right = CalcPercentPadding(aBasis.width, styleCoord);
  }
  else if (eStyleUnit_Coord == aPaddingData.mPadding.GetTopUnit()) {
    aPadding.right = styleCoord.GetCoordValue();
  }

  aPaddingData.mPadding.GetBottom(styleCoord);
  if (eStyleUnit_Percent == aPaddingData.mPadding.GetBottomUnit()) {
    aPadding.bottom = CalcPercentPadding(aBasis.height, styleCoord);
  }
  else if (eStyleUnit_Coord == aPaddingData.mPadding.GetTopUnit()) {
    aPadding.bottom = styleCoord.GetCoordValue();
  }

  aPaddingData.mPadding.GetLeft(styleCoord);
  if (eStyleUnit_Percent == aPaddingData.mPadding.GetLeftUnit()) {
    aPadding.left = CalcPercentPadding(aBasis.width, styleCoord);
  }
  else if (eStyleUnit_Coord == aPaddingData.mPadding.GetTopUnit()) {
    aPadding.left = styleCoord.GetCoordValue();
  }
}

nsMargin
nsTableFrame::GetBorderPadding(const nsHTMLReflowState& aReflowState,
                               float                    aPixelToTwips,
                               const nsTableCellFrame*  aCellFrame)
{
  const nsStylePadding* paddingData = aCellFrame->GetStylePadding();
  nsMargin padding(0,0,0,0);
  if (!paddingData->GetPadding(padding)) {
    const nsHTMLReflowState* parentRS = aReflowState.parentReflowState;
    while (parentRS) {
      if (parentRS->frame) {
        if (nsLayoutAtoms::tableFrame == parentRS->frame->GetType()) {
          nsSize basis(parentRS->mComputedWidth, parentRS->mComputedHeight);
          GetPaddingFor(basis, *paddingData, padding);
          break;
        }
      }
      parentRS = parentRS->parentReflowState;
    }
  }
  nsMargin border;
  aCellFrame->GetBorderWidth(aPixelToTwips, border);
  padding += border;
  return padding;
}

nsMargin
nsTableFrame::GetBorderPadding(const nsSize&           aBasis,
                               float                   aPixelToTwips,
                               const nsTableCellFrame* aCellFrame)
{
  const nsStylePadding* paddingData = aCellFrame->GetStylePadding();
  nsMargin padding(0,0,0,0);
  if (!paddingData->GetPadding(padding)) {
    GetPaddingFor(aBasis, *paddingData, padding);
  }
  nsMargin border;
  aCellFrame->GetBorderWidth(aPixelToTwips, border);
  padding += border;
  return padding;
}

// XXX: could cache this.  But be sure to check style changes if you do!
nscoord nsTableFrame::GetCellSpacingX()
{
  if (IsBorderCollapse())
    return 0;

  NS_ASSERTION(GetStyleTableBorder()->mBorderSpacingX.GetUnit() == eStyleUnit_Coord,
               "Not a coord value!");
  return GetStyleTableBorder()->mBorderSpacingX.GetCoordValue();
}

// XXX: could cache this. But be sure to check style changes if you do!
nscoord nsTableFrame::GetCellSpacingY()
{
  if (IsBorderCollapse())
    return 0;

  NS_ASSERTION(GetStyleTableBorder()->mBorderSpacingY.GetUnit() == eStyleUnit_Coord,
               "Not a coord value!");
  return GetStyleTableBorder()->mBorderSpacingY.GetCoordValue();
}


nscoord nsTableFrame::GetAscent()
{
  nscoord ascent = 0;
  nsAutoVoidArray orderedRowGroups;
  PRUint32 numRowGroups;
  OrderRowGroups(orderedRowGroups, numRowGroups);
  nsTableRowFrame* firstRow = nsnull;
  for (PRUint32 rgIndex = 0; rgIndex < numRowGroups; rgIndex++) {
    nsTableRowGroupFrame* rgFrame = GetRowGroupFrame((nsIFrame*)orderedRowGroups.ElementAt(rgIndex));
    if (rgFrame->GetRowCount()) {
      firstRow = rgFrame->GetFirstRow(); 
      ascent = rgFrame->GetRect().y + firstRow->GetRect().y + firstRow->GetAscent();
      break;
    }
  }
  if (!firstRow)
    ascent = GetRect().height;
  return ascent;
}
/* ----- global methods ----- */

nsIFrame*
NS_NewTableFrame(nsIPresShell* aPresShell, nsStyleContext* aContext)
{
  return new (aPresShell) nsTableFrame(aContext);
}

nsTableFrame*
nsTableFrame::GetTableFrame(nsIFrame* aSourceFrame)
{
  if (aSourceFrame) {
    // "result" is the result of intermediate calls, not the result we return from this method
    for (nsIFrame* parentFrame = aSourceFrame->GetParent(); parentFrame;
         parentFrame = parentFrame->GetParent()) {
      if (nsLayoutAtoms::tableFrame == parentFrame->GetType()) {
        return (nsTableFrame*)parentFrame;
      }
    }
  }
  NS_NOTREACHED("unable to find table parent");
  return nsnull;
}

PRBool 
nsTableFrame::IsAutoWidth(PRBool* aIsPctWidth)
{
  return nsTableOuterFrame::IsAutoWidth(*this, aIsPctWidth);
}

PRBool 
nsTableFrame::IsAutoHeight()
{
  PRBool isAuto = PR_TRUE;  // the default

  const nsStylePosition* position = GetStylePosition();

  switch (position->mHeight.GetUnit()) {
    case eStyleUnit_Auto:         // specified auto width
    case eStyleUnit_Proportional: // illegal for table, so ignored
      break;
    case eStyleUnit_Coord:
      isAuto = PR_FALSE;
      break;
    case eStyleUnit_Percent:
      if (position->mHeight.GetPercentValue() > 0.0f) {
        isAuto = PR_FALSE;
      }
      break;
    default:
      break;
  }

  return isAuto; 
}

nscoord 
nsTableFrame::CalcBorderBoxWidth(const nsHTMLReflowState& aState)
{
  nscoord width = aState.mComputedWidth;

  if (eStyleUnit_Auto == aState.mStylePosition->mWidth.GetUnit()) {
    if (0 == width) {
      width = aState.availableWidth;
    }
    if (NS_UNCONSTRAINEDSIZE != aState.availableWidth) {
      width = aState.availableWidth;
    }
  }
  else if (width != NS_UNCONSTRAINEDSIZE) {
    nsMargin borderPadding = GetContentAreaOffset(&aState);
    width += borderPadding.left + borderPadding.right;
  }
  width = PR_MAX(width, 0);

  if (NS_UNCONSTRAINEDSIZE != width) {
    GET_PIXELS_TO_TWIPS(GetPresContext(), p2t);
    width = RoundToPixel(width, p2t, eRoundUpIfHalfOrMore);
  }

  return width;
}

nscoord 
nsTableFrame::CalcBorderBoxHeight(const nsHTMLReflowState& aState)
{
  nscoord height = aState.mComputedHeight;
  if (NS_AUTOHEIGHT != height) {
    nsMargin borderPadding = GetContentAreaOffset(&aState);
    height += borderPadding.top + borderPadding.bottom;
  }
  height = PR_MAX(0, height);

  return height;
}

nscoord 
nsTableFrame::GetMinCaptionWidth()
{
  return NS_STATIC_CAST(nsTableOuterFrame*, GetParent())->GetMinCaptionWidth();
}

PRBool 
nsTableFrame::IsAutoLayout()
{
  if (NS_STYLE_TABLE_LAYOUT_FIXED == GetStyleTable()->mLayoutStrategy) {
    // a fixed-layout table must have a width
    if (eStyleUnit_Auto != GetStylePosition()->mWidth.GetUnit()) {
      return PR_FALSE;
    }
  }
  return PR_TRUE;
}

#ifdef DEBUG
NS_IMETHODIMP
nsTableFrame::GetFrameName(nsAString& aResult) const
{
  return MakeFrameName(NS_LITERAL_STRING("Table"), aResult);
}
#endif


void 
nsTableFrame::CalcMinAndPreferredWidths(const           nsHTMLReflowState& aReflowState,
                                        PRBool          aCalcPrefWidthIfAutoWithPctCol,
                                        nscoord&        aMinWidth,
                                        nscoord&        aPrefWidth) 
{
  aMinWidth = aPrefWidth = 0;

  nscoord spacingX = GetCellSpacingX();
  PRInt32 numCols = GetColCount();

  for (PRInt32 colX = 0; colX < numCols; colX++) { 
    nsTableColFrame* colFrame = GetColFrame(colX);
    if (!colFrame) continue;
    aMinWidth += colFrame->GetMinWidth();
    nscoord width = colFrame->GetFixWidth();
    if (width <= 0) {
      width = colFrame->GetDesWidth();
    }
    aPrefWidth += width;
    if (GetNumCellsOriginatingInCol(colX) > 0) {
      aMinWidth  += spacingX;
      aPrefWidth += spacingX;
    }
  }
  // if it is not a degenerate table, add the last spacing on the right and the borderPadding
  if (numCols > 0) {
    nsMargin childAreaOffset = GetChildAreaOffset( &aReflowState);
    nscoord extra = spacingX + childAreaOffset.left + childAreaOffset.right;
    aMinWidth  += extra;
    aPrefWidth += extra;
  }
  aPrefWidth = PR_MAX(aMinWidth, aPrefWidth);

  PRBool isPctWidth = PR_FALSE;
  if (IsAutoWidth(&isPctWidth)) {
    if (HasPctCol() && aCalcPrefWidthIfAutoWithPctCol && 
        (NS_UNCONSTRAINEDSIZE != aReflowState.availableWidth)) {
      // for an auto table with a pct cell, use the strategy's CalcPctAdjTableWidth
      nscoord availWidth = CalcBorderBoxWidth(aReflowState);
      availWidth = PR_MIN(availWidth, aReflowState.availableWidth);
      if (mTableLayoutStrategy && IsAutoLayout()) {
        aPrefWidth = mTableLayoutStrategy->CalcPctAdjTableWidth(aReflowState, availWidth);
      }
    }
    if (0 == numCols) { // degenerate case
      aMinWidth = aPrefWidth = 0;
    }
  }
  else { // a specified fix width becomes the min or preferred width
    nscoord compWidth = aReflowState.mComputedWidth;
    if ((NS_UNCONSTRAINEDSIZE != compWidth) && (0 != compWidth) && !isPctWidth) {
      nsMargin contentOffset = GetContentAreaOffset(&aReflowState);
      compWidth += contentOffset.left + contentOffset.right;
      aMinWidth = PR_MAX(aMinWidth, compWidth);
      aPrefWidth = PR_MAX(aMinWidth, compWidth);
    }
  }
}


// Find the closet sibling before aPriorChildFrame (including aPriorChildFrame) that
// is of type aChildType
nsIFrame* 
nsTableFrame::GetFrameAtOrBefore(nsIFrame*       aParentFrame,
                                 nsIFrame*       aPriorChildFrame,
                                 nsIAtom*        aChildType)
{
  nsIFrame* result = nsnull;
  if (!aPriorChildFrame) {
    return result;
  }
  if (aChildType == aPriorChildFrame->GetType()) {
    return aPriorChildFrame;
  }

  // aPriorChildFrame is not of type aChildType, so we need start from 
  // the beginnng and find the closest one 
  nsIFrame* lastMatchingFrame = nsnull;
  nsIFrame* childFrame = aParentFrame->GetFirstChild(nsnull);
  while (childFrame && (childFrame != aPriorChildFrame)) {
    if (aChildType == childFrame->GetType()) {
      lastMatchingFrame = childFrame;
    }
    childFrame = childFrame->GetNextSibling();
  }
  return lastMatchingFrame;
}

#ifdef DEBUG
void 
nsTableFrame::DumpRowGroup(nsIFrame* aKidFrame)
{
  nsTableRowGroupFrame* rgFrame = GetRowGroupFrame(aKidFrame);
  if (rgFrame) {
    nsIFrame* rowFrame = rgFrame->GetFirstChild(nsnull);
    while (rowFrame) {
      if (nsLayoutAtoms::tableRowFrame == rowFrame->GetType()) {
        printf("row(%d)=%p ", ((nsTableRowFrame*)rowFrame)->GetRowIndex(), rowFrame);
        nsIFrame* cellFrame = rowFrame->GetFirstChild(nsnull);
        while (cellFrame) {
          if (IS_TABLE_CELL(cellFrame->GetType())) {
            PRInt32 colIndex;
            ((nsTableCellFrame*)cellFrame)->GetColIndex(colIndex);
            printf("cell(%d)=%p ", colIndex, cellFrame);
          }
          cellFrame = cellFrame->GetNextSibling();
        }
        printf("\n");
      }
      else {
        DumpRowGroup(rowFrame);
      }
      rowFrame = rowFrame->GetNextSibling();
    }
  }
}

void 
nsTableFrame::Dump(PRBool          aDumpRows,
                   PRBool          aDumpCols, 
                   PRBool          aDumpCellMap)
{
  printf("***START TABLE DUMP*** \n");
  // dump the columns widths array
  printf("mColWidths=");
  PRInt32 numCols = GetColCount();
  PRInt32 colX;
  for (colX = 0; colX < numCols; colX++) {
    printf("%d ", GetColumnWidth(colX));
  }
  printf("\n");

  if (aDumpRows) {
    nsIFrame* kidFrame = mFrames.FirstChild();
    while (kidFrame) {
      DumpRowGroup(kidFrame);
      kidFrame = kidFrame->GetNextSibling();
    }
  }

  if (aDumpCols) {
	  // output col frame cache
    printf("\n col frame cache ->");
	   for (colX = 0; colX < numCols; colX++) {
      nsTableColFrame* colFrame = (nsTableColFrame *)mColFrames.ElementAt(colX);
      if (0 == (colX % 8)) {
        printf("\n");
      }
      printf ("%d=%p ", colX, colFrame);
      nsTableColType colType = colFrame->GetColType();
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
    }
    printf("\n colgroups->");
    for (nsIFrame* childFrame = mColGroups.FirstChild(); childFrame;
         childFrame = childFrame->GetNextSibling()) {
      if (nsLayoutAtoms::tableColGroupFrame == childFrame->GetType()) {
        nsTableColGroupFrame* colGroupFrame = (nsTableColGroupFrame *)childFrame;
        colGroupFrame->Dump(1);
      }
    }
    for (colX = 0; colX < numCols; colX++) {
      printf("\n");
      nsTableColFrame* colFrame = GetColFrame(colX);
      colFrame->Dump(1);
    }
  }
  if (aDumpCellMap) {
    nsTableCellMap* cellMap = GetCellMap();
    cellMap->Dump();
  }
  printf(" ***END TABLE DUMP*** \n");
}
#endif

// nsTableIterator
nsTableIterator::nsTableIterator(nsIFrame&        aSource,
                                 nsTableIteration aType)
{
  nsIFrame* firstChild = aSource.GetFirstChild(nsnull);
  Init(firstChild, aType);
}

nsTableIterator::nsTableIterator(nsFrameList&     aSource,
                                 nsTableIteration aType)
{
  nsIFrame* firstChild = aSource.FirstChild();
  Init(firstChild, aType);
}

void nsTableIterator::Init(nsIFrame*        aFirstChild,
                           nsTableIteration aType)
{
  mFirstListChild = aFirstChild;
  mFirstChild     = aFirstChild;
  mCurrentChild   = nsnull;
  mLeftToRight    = (eTableRTL == aType) ? PR_FALSE : PR_TRUE; 
  mCount          = -1;

  if (!mFirstChild) {
    return;
  }
  if (eTableDIR == aType) {
    nsTableFrame* table = nsTableFrame::GetTableFrame(mFirstChild);
    if (table) {
      mLeftToRight = (NS_STYLE_DIRECTION_LTR ==
                      table->GetStyleVisibility()->mDirection);
    }
    else {
      NS_ASSERTION(PR_FALSE, "source of table iterator is not part of a table");
      return;
    }
  }
  if (!mLeftToRight) {
    mCount = 0;
    nsIFrame* nextChild = mFirstChild->GetNextSibling();
    while (nsnull != nextChild) {
      mCount++;
      mFirstChild = nextChild;
      nextChild = nextChild->GetNextSibling();
    }
  } 
}

nsIFrame* nsTableIterator::First()
{
  mCurrentChild = mFirstChild;
  return mCurrentChild;
}
      
nsIFrame* nsTableIterator::Next()
{
  if (!mCurrentChild) {
    return nsnull;
  }

  if (mLeftToRight) {
    mCurrentChild = mCurrentChild->GetNextSibling();
    return mCurrentChild;
  }
  else {
    nsIFrame* targetChild = mCurrentChild;
    mCurrentChild = nsnull;
    nsIFrame* child = mFirstListChild;
    while (child && (child != targetChild)) {
      mCurrentChild = child;
      child = child->GetNextSibling();
    }
    return mCurrentChild;
  }
}

PRBool nsTableIterator::IsLeftToRight()
{
  return mLeftToRight;
}

PRInt32 nsTableIterator::Count()
{
  if (-1 == mCount) {
    mCount = 0;
    nsIFrame* child = mFirstListChild;
    while (nsnull != child) {
      mCount++;
      child = child->GetNextSibling();
    }
  }
  return mCount;
}

nsTableCellFrame* nsTableFrame::GetCellInfoAt(PRInt32            aRowX, 
                                              PRInt32            aColX, 
                                              PRBool*            aOriginates, 
                                              PRInt32*           aColSpan)
{
  nsTableCellMap* cellMap = GetCellMap();
  return cellMap->GetCellInfoAt(aRowX, aColX, aOriginates, aColSpan);
}

/*------------------ nsITableLayout methods ------------------------------*/
NS_IMETHODIMP 
nsTableFrame::GetCellDataAt(PRInt32        aRowIndex, 
                            PRInt32        aColIndex,
                            nsIDOMElement* &aCell,   //out params
                            PRInt32&       aStartRowIndex, 
                            PRInt32&       aStartColIndex, 
                            PRInt32&       aRowSpan, 
                            PRInt32&       aColSpan,
                            PRInt32&       aActualRowSpan, 
                            PRInt32&       aActualColSpan,
                            PRBool&        aIsSelected)
{
  // Initialize out params
  aCell = nsnull;
  aStartRowIndex = 0;
  aStartColIndex = 0;
  aRowSpan = 0;
  aColSpan = 0;
  aIsSelected = PR_FALSE;

  nsTableCellMap* cellMap = GetCellMap();
  if (!cellMap) { return NS_ERROR_NOT_INITIALIZED;}

  PRBool originates;
  PRInt32 colSpan; // Is this the "effective" or "html" value?

  nsTableCellFrame *cellFrame = cellMap->GetCellInfoAt(aRowIndex, aColIndex, &originates, &colSpan);
  if (!cellFrame) return NS_TABLELAYOUT_CELL_NOT_FOUND;

  nsresult result= cellFrame->GetRowIndex(aStartRowIndex);
  if (NS_FAILED(result)) return result;
  result = cellFrame->GetColIndex(aStartColIndex);
  if (NS_FAILED(result)) return result;
  //This returns HTML value, which may be 0
  aRowSpan = cellFrame->GetRowSpan();
  aColSpan = cellFrame->GetColSpan();
  aActualRowSpan = GetEffectiveRowSpan(*cellFrame);
  aActualColSpan = GetEffectiveColSpan(*cellFrame);

  // If these aren't at least 1, we have a cellmap error
  if (aActualRowSpan == 0 || aActualColSpan == 0)
    return NS_ERROR_FAILURE;

  result = cellFrame->GetSelected(&aIsSelected);
  if (NS_FAILED(result)) return result;

  // do this last, because it addrefs, 
  // and we don't want the caller leaking it on error
  nsIContent* content = cellFrame->GetContent();
  if (!content) return NS_ERROR_FAILURE;   
  
  return CallQueryInterface(content, &aCell);                                      
}

NS_IMETHODIMP nsTableFrame::GetTableSize(PRInt32& aRowCount, PRInt32& aColCount)
{
  nsTableCellMap* cellMap = GetCellMap();
  // Initialize out params
  aRowCount = 0;
  aColCount = 0;
  if (!cellMap) { return NS_ERROR_NOT_INITIALIZED;}

  aRowCount = cellMap->GetRowCount();
  aColCount = cellMap->GetColCount();
  return NS_OK;
}

/*---------------- end of nsITableLayout implementation ------------------*/

PRInt32 nsTableFrame::GetNumCellsOriginatingInCol(PRInt32 aColIndex) const
{
  nsTableCellMap* cellMap = GetCellMap();
  if (cellMap) 
    return cellMap->GetNumCellsOriginatingInCol(aColIndex);
  else
    return 0;
}

PRInt32 nsTableFrame::GetNumCellsOriginatingInRow(PRInt32 aRowIndex) const
{
  nsTableCellMap* cellMap = GetCellMap();
  if (cellMap) 
    return cellMap->GetNumCellsOriginatingInRow(aRowIndex);
  else
    return 0;
}

static void
CheckFixDamageArea(PRInt32 aNumRows,
                   PRInt32 aNumCols,
                   nsRect& aDamageArea)
{
  if (((aDamageArea.XMost() > aNumCols) && (aDamageArea.width  != 1) && (aNumCols != 0)) || 
      ((aDamageArea.YMost() > aNumRows) && (aDamageArea.height != 1) && (aNumRows != 0))) {
    // the damage area was set incorrectly, just be safe and make it the entire table
    NS_ASSERTION(PR_FALSE, "invalid BC damage area");
    aDamageArea.x      = 0;
    aDamageArea.y      = 0;
    aDamageArea.width  = aNumCols;
    aDamageArea.height = aNumRows;
  }
}

/********************************************************************************
 * Collapsing Borders
 *
 *  The CSS spec says to resolve border conflicts in this order:
 *  1) any border with the style HIDDEN wins
 *  2) the widest border with a style that is not NONE wins
 *  3) the border styles are ranked in this order, highest to lowest precedence: 
 *     double, solid, dashed, dotted, ridge, outset, groove, inset
 *  4) borders that are of equal width and style (differ only in color) have this precedence:
 *     cell, row, rowgroup, col, colgroup, table
 *  5) if all border styles are NONE, then that's the computed border style.
 *******************************************************************************/

void 
nsTableFrame::SetBCDamageArea(const nsRect& aValue)
{
  nsRect newRect(aValue);
  newRect.width  = PR_MAX(1, newRect.width);
  newRect.height = PR_MAX(1, newRect.height);

  if (!IsBorderCollapse()) {
    NS_ASSERTION(PR_FALSE, "invalid call - not border collapse model");
    return;
  }
  SetNeedToCalcBCBorders(PR_TRUE);
  // Get the property 
  BCPropertyData* value = (BCPropertyData*)nsTableFrame::GetProperty(this, nsLayoutAtoms::tableBCProperty, PR_TRUE);
  if (value) {
    // for now just construct a union of the new and old damage areas
    value->mDamageArea.UnionRect(value->mDamageArea, newRect);
    CheckFixDamageArea(GetRowCount(), GetColCount(), value->mDamageArea);
  }
}
/*****************************************************************
 *  BCMapCellIterator
 ****************************************************************/
struct BCMapCellInfo 
{
  BCMapCellInfo();
  void Reset();

  CellData*             cellData;
  nsCellMap*            cellMap;

  nsTableRowGroupFrame* rg;

  nsTableRowFrame*      topRow;
  nsTableRowFrame*      bottomRow;

  nsTableColGroupFrame* cg;
 
  nsTableColFrame*      leftCol;
  nsTableColFrame*      rightCol;

  nsBCTableCellFrame*   cell;

  PRInt32               rowIndex;
  PRInt32               rowSpan;
  PRInt32               colIndex;
  PRInt32               colSpan;

  PRPackedBool          rgTop;
  PRPackedBool          rgBottom;
  PRPackedBool          cgLeft;
  PRPackedBool          cgRight;
};

BCMapCellInfo::BCMapCellInfo()
{
  Reset();
}

void BCMapCellInfo::Reset()
{
  cellData  = nsnull;
  rg        = nsnull;
  topRow    = nsnull;
  bottomRow = nsnull;
  cg        = nsnull;
  leftCol   = nsnull;
  rightCol  = nsnull;
  cell      = nsnull;
  rowIndex = rowSpan = colIndex = colSpan = 0;
  rgTop = rgBottom = cgLeft = cgRight = PR_FALSE;
}

class BCMapCellIterator
{
public:
  BCMapCellIterator(nsTableFrame& aTableFrame,
                  const nsRect& aDamageArea);

  void First(BCMapCellInfo& aMapCellInfo);

  void Next(BCMapCellInfo& aMapCellInfo);

  void PeekRight(BCMapCellInfo& aRefInfo,
                 PRUint32     aRowIndex,
                 BCMapCellInfo& aAjaInfo);

  void PeekBottom(BCMapCellInfo& aRefInfo,
                  PRUint32     aColIndex,
                  BCMapCellInfo& aAjaInfo);

  PRBool IsNewRow() { return mIsNewRow; }

  nsTableRowFrame* GetPrevRow() const { return mPrevRow; }

  PRInt32    mRowGroupStart;
  PRInt32    mRowGroupEnd;
  PRBool     mAtEnd;
  nsCellMap* mCellMap;

private:
  void SetInfo(nsTableRowFrame* aRow,
               PRInt32          aColIndex,
               CellData*        aCellData,
               BCMapCellInfo&   aMapInfo,
               nsCellMap*       aCellMap = nsnull);

  PRBool SetNewRow(nsTableRowFrame* row = nsnull);
  PRBool SetNewRowGroup(PRBool aFindFirstDamagedRow);

  nsTableFrame&         mTableFrame;
  nsTableCellMap*       mTableCellMap;
  nsVoidArray           mRowGroups;
  nsTableRowGroupFrame* mRowGroup;
  PRInt32               mRowGroupIndex;
  PRUint32              mNumRows;
  nsTableRowFrame*      mRow;
  nsTableRowFrame*      mPrevRow;
  PRBool                mIsNewRow;
  PRInt32               mRowIndex;
  PRUint32              mNumCols;
  PRInt32               mColIndex;
  nsPoint               mAreaStart;
  nsPoint               mAreaEnd;
};

BCMapCellIterator::BCMapCellIterator(nsTableFrame& aTableFrame,
                                     const nsRect& aDamageArea)
:mTableFrame(aTableFrame)
{
  mTableCellMap  = aTableFrame.GetCellMap();

  mAreaStart.x   = aDamageArea.x;
  mAreaStart.y   = aDamageArea.y;
  mAreaEnd.y     = aDamageArea.y + aDamageArea.height - 1;
  mAreaEnd.x     = aDamageArea.x + aDamageArea.width - 1;

  mNumRows       = mTableFrame.GetRowCount();
  mRow           = nsnull;
  mRowIndex      = 0;
  mNumCols       = mTableFrame.GetColCount();
  mColIndex      = 0;
  mRowGroupIndex = -1;

  // Get the ordered row groups 
  PRUint32 numRowGroups;
  aTableFrame.OrderRowGroups(mRowGroups, numRowGroups, nsnull);

  mAtEnd = PR_TRUE; // gets reset when First() is called
}

void 
BCMapCellIterator::SetInfo(nsTableRowFrame* aRow,
                           PRInt32          aColIndex,
                           CellData*        aCellData,
                           BCMapCellInfo&   aCellInfo,
                           nsCellMap*       aCellMap)
{
  aCellInfo.cellData = aCellData;
  aCellInfo.cellMap = (aCellMap) ? aCellMap : mCellMap;
  aCellInfo.colIndex = aColIndex;

  // row frame info
  aCellInfo.rowIndex = 0;
  if (aRow) {
    aCellInfo.topRow = aRow; 
    aCellInfo.rowIndex = aRow->GetRowIndex();
  }

  // cell frame info
  aCellInfo.cell      = nsnull;
  aCellInfo.rowSpan   = 1;
  aCellInfo.colSpan  = 1;
  if (aCellData) {
    aCellInfo.cell = (nsBCTableCellFrame*)aCellData->GetCellFrame(); 
    if (aCellInfo.cell) {
      if (!aCellInfo.topRow) {
        aCellInfo.topRow = NS_STATIC_CAST(nsTableRowFrame*,
                                          aCellInfo.cell->GetParent());
        if (!aCellInfo.topRow) ABORT0();
        aCellInfo.rowIndex = aCellInfo.topRow->GetRowIndex();
      }
      aCellInfo.colSpan = mTableFrame.GetEffectiveColSpan(*aCellInfo.cell, aCellMap); 
      aCellInfo.rowSpan = mTableFrame.GetEffectiveRowSpan(*aCellInfo.cell, aCellMap);
    }
  }
  if (!aCellInfo.topRow) {
    aCellInfo.topRow = mRow;
  }

  if (1 == aCellInfo.rowSpan) {
    aCellInfo.bottomRow = aCellInfo.topRow;
  }
  else {
    aCellInfo.bottomRow = aCellInfo.topRow->GetNextRow();
    if (aCellInfo.bottomRow) {
      for (PRInt32 spanX = 2; aCellInfo.bottomRow && (spanX < aCellInfo.rowSpan); spanX++) {
        aCellInfo.bottomRow = aCellInfo.bottomRow->GetNextRow();
      }
      NS_ASSERTION(aCellInfo.bottomRow, "program error");
    }
    else {
      NS_ASSERTION(PR_FALSE, "error in cell map");
      aCellInfo.rowSpan = 1;
      aCellInfo.bottomRow = aCellInfo.topRow;
    }
  }

  // row group frame info
  PRUint32 rgStart  = mRowGroupStart;
  PRUint32 rgEnd    = mRowGroupEnd;
  aCellInfo.rg = mTableFrame.GetRowGroupFrame(aCellInfo.topRow->GetParent());
  if (aCellInfo.rg != mRowGroup) {
    rgStart = aCellInfo.rg->GetStartRowIndex();
    rgEnd   = rgStart + aCellInfo.rg->GetRowCount() - 1;
  }
  PRUint32 rowIndex  = aCellInfo.topRow->GetRowIndex();
  aCellInfo.rgTop    = (rgStart == rowIndex);
  aCellInfo.rgBottom = (rgEnd == rowIndex + aCellInfo.rowSpan - 1);

  // col frame info
  aCellInfo.leftCol = mTableFrame.GetColFrame(aColIndex); if (!aCellInfo.leftCol) ABORT0();

  aCellInfo.rightCol = aCellInfo.leftCol;
  if (aCellInfo.colSpan > 1) {
    for (PRInt32 spanX = 1; spanX < aCellInfo.colSpan; spanX++) {
      nsTableColFrame* colFrame = mTableFrame.GetColFrame(aColIndex + spanX); if (!colFrame) ABORT0();
      aCellInfo.rightCol = colFrame;
    }
  }

  // col group frame info
  aCellInfo.cg = NS_STATIC_CAST(nsTableColGroupFrame*,
                                aCellInfo.leftCol->GetParent());
  PRInt32 cgStart  = aCellInfo.cg->GetStartColumnIndex();
  PRInt32 cgEnd    = PR_MAX(0, cgStart + aCellInfo.cg->GetColCount() - 1);
  aCellInfo.cgLeft  = (cgStart == aColIndex);
  aCellInfo.cgRight = (cgEnd == aColIndex + (PRInt32)aCellInfo.colSpan - 1);
}

PRBool
BCMapCellIterator::SetNewRow(nsTableRowFrame* aRow)
{
  mAtEnd   = PR_TRUE;
  mPrevRow = mRow;
  if (aRow) {
    mRow = aRow;
  }
  else if (mRow) {
    mRow = mRow->GetNextRow();
  }
  if (mRow) {
    mRowIndex = mRow->GetRowIndex();
    // get to the first entry with an originating cell
    PRInt32 rgRowIndex = mRowIndex - mRowGroupStart;
    nsVoidArray* row = (nsVoidArray*)mCellMap->mRows.ElementAt(rgRowIndex); if (!row) ABORT1(PR_FALSE);
    PRInt32 rowSize = row->Count();
    for (mColIndex = mAreaStart.x; mColIndex <= mAreaEnd.x; mColIndex++) {
      CellData* cellData = (mColIndex < rowSize) ? (CellData*)row->ElementAt(mColIndex) : nsnull;
      if (!cellData) { // add a dead cell data
        nsRect damageArea;
        cellData = mCellMap->AppendCell(*mTableCellMap, nsnull, rgRowIndex, PR_FALSE, damageArea); if (!cellData) ABORT1(PR_FALSE);
      }
      if (cellData && (cellData->IsOrig() || cellData->IsDead())) {
        break;
      }
    }
    mIsNewRow = PR_TRUE;
    mAtEnd    = PR_FALSE;
  }
  else ABORT1(PR_FALSE);

  return !mAtEnd;
}

PRBool
BCMapCellIterator::SetNewRowGroup(PRBool aFindFirstDamagedRow)
{
  mAtEnd = PR_TRUE;
  mRowGroupIndex++;
  PRInt32 numRowGroups = mRowGroups.Count();
  for (PRInt32 rgX = mRowGroupIndex; rgX < numRowGroups; rgX++) {
    nsIFrame* frame = (nsTableRowGroupFrame*)mRowGroups.ElementAt(mRowGroupIndex); if (!frame) ABORT1(PR_FALSE);
    mRowGroup = mTableFrame.GetRowGroupFrame(frame); if (!mRowGroup) ABORT1(PR_FALSE);
    PRInt32 rowCount = mRowGroup->GetRowCount();
    mRowGroupStart = mRowGroup->GetStartRowIndex();
    mRowGroupEnd   = mRowGroupStart + rowCount - 1;
    if (rowCount > 0) {
      mCellMap = mTableCellMap->GetMapFor(*mRowGroup); if (!mCellMap) ABORT1(PR_FALSE);
      nsTableRowFrame* firstRow = mRowGroup->GetFirstRow();
      if (aFindFirstDamagedRow) {
        if ((mAreaStart.y >= mRowGroupStart) && (mAreaStart.y <= mRowGroupEnd)) {
          // the damage area starts in the row group 
          if (aFindFirstDamagedRow) {
            // find the correct first damaged row
            PRInt32 numRows = mAreaStart.y - mRowGroupStart;
            for (PRInt32 i = 0; i < numRows; i++) {
              firstRow = firstRow->GetNextRow(); if (!frame) ABORT1(PR_FALSE);
            }
          }
        }
        else {
          mRowGroupIndex++;
          continue;
        }
      }
      if (SetNewRow(firstRow)) { // sets mAtEnd
        break;
      }
    }
  }
    
  return !mAtEnd;
}

void 
BCMapCellIterator::First(BCMapCellInfo& aMapInfo)
{
  aMapInfo.Reset();

  SetNewRowGroup(PR_TRUE); // sets mAtEnd
  while (!mAtEnd) {
    if ((mAreaStart.y >= mRowGroupStart) && (mAreaStart.y <= mRowGroupEnd)) {
      CellData* cellData = mCellMap->GetDataAt(*mTableCellMap, mAreaStart.y - mRowGroupStart, mAreaStart.x, PR_FALSE);
      if (cellData && cellData->IsOrig()) {
        SetInfo(mRow, mAreaStart.x, cellData, aMapInfo);
      }
      else {
        NS_ASSERTION(((0 == mAreaStart.x) && (mRowGroupStart == mAreaStart.y)) , "damage area expanded incorrectly");
        mAtEnd = PR_TRUE;
      }
      break;
    }
    SetNewRowGroup(PR_TRUE); // sets mAtEnd
  } 
}

void 
BCMapCellIterator::Next(BCMapCellInfo& aMapInfo)
{
  if (mAtEnd) ABORT0();
  aMapInfo.Reset();

  mIsNewRow = PR_FALSE;
  mColIndex++;
  while ((mRowIndex <= mAreaEnd.y) && !mAtEnd) {
    for (; mColIndex <= mAreaEnd.x; mColIndex++) {
      PRInt32 rgRowIndex = mRowIndex - mRowGroupStart;
      CellData* cellData = mCellMap->GetDataAt(*mTableCellMap, rgRowIndex, mColIndex, PR_FALSE);
      if (!cellData) { // add a dead cell data
        nsRect damageArea;
        cellData = mCellMap->AppendCell(*mTableCellMap, nsnull, rgRowIndex, PR_FALSE, damageArea); if (!cellData) ABORT0();
      }
      if (cellData && (cellData->IsOrig() || cellData->IsDead())) {
        SetInfo(mRow, mColIndex, cellData, aMapInfo);
        return;
      }
    }
    if (mRowIndex >= mRowGroupEnd) {
      SetNewRowGroup(PR_FALSE); // could set mAtEnd
    }
    else {
      SetNewRow(); // could set mAtEnd
    }
  }
  mAtEnd = PR_TRUE;
}

void 
BCMapCellIterator::PeekRight(BCMapCellInfo&   aRefInfo,
                             PRUint32         aRowIndex,
                             BCMapCellInfo&   aAjaInfo)
{
  aAjaInfo.Reset();
  PRInt32 colIndex = aRefInfo.colIndex + aRefInfo.colSpan;
  PRUint32 rgRowIndex = aRowIndex - mRowGroupStart;

  CellData* cellData = mCellMap->GetDataAt(*mTableCellMap, rgRowIndex, colIndex, PR_FALSE);
  if (!cellData) { // add a dead cell data
    NS_ASSERTION(colIndex < mTableCellMap->GetColCount(), "program error");
    nsRect damageArea;
    cellData = mCellMap->AppendCell(*mTableCellMap, nsnull, rgRowIndex, PR_FALSE, damageArea); if (!cellData) ABORT0();
  }
  nsTableRowFrame* row = nsnull;
  if (cellData->IsRowSpan()) {
    rgRowIndex -= cellData->GetRowSpanOffset();
    cellData = mCellMap->GetDataAt(*mTableCellMap, rgRowIndex, colIndex, PR_FALSE); if (!cellData) ABORT0();
  }
  else {
    row = mRow;
  }
  SetInfo(row, colIndex, cellData, aAjaInfo);
}

void 
BCMapCellIterator::PeekBottom(BCMapCellInfo&   aRefInfo,
                              PRUint32         aColIndex,
                              BCMapCellInfo&   aAjaInfo)
{
  aAjaInfo.Reset();
  PRInt32 rowIndex = aRefInfo.rowIndex + aRefInfo.rowSpan;
  PRInt32 rgRowIndex = rowIndex - mRowGroupStart;
  nsTableRowGroupFrame* rg = mRowGroup;
  nsCellMap* cellMap = mCellMap;
  nsTableRowFrame* nextRow = nsnull;
  if (rowIndex > mRowGroupEnd) {
    PRInt32 nextRgIndex = mRowGroupIndex;
    do {
      nextRgIndex++;
      nsIFrame* frame = (nsTableRowGroupFrame*)mRowGroups.ElementAt(nextRgIndex); if (!frame) ABORT0();
      rg = mTableFrame.GetRowGroupFrame(frame);
      if (rg) {
        cellMap = mTableCellMap->GetMapFor(*rg); if (!cellMap) ABORT0();
        rgRowIndex = 0;
        nextRow = rg->GetFirstRow();
      }
    }
    while (rg && !nextRow);
    if(!rg) return;
  }
  else {
    // get the row within the same row group
    nextRow = mRow;
    for (PRInt32 i = 0; i < aRefInfo.rowSpan; i++) {
      nextRow = nextRow->GetNextRow(); if (!nextRow) ABORT0();
    }
  }

  CellData* cellData = cellMap->GetDataAt(*mTableCellMap, rgRowIndex, aColIndex, PR_FALSE);
  if (!cellData) { // add a dead cell data
    NS_ASSERTION(rgRowIndex < cellMap->GetRowCount(), "program error");
    nsRect damageArea;
    cellData = cellMap->AppendCell(*mTableCellMap, nsnull, rgRowIndex, PR_FALSE, damageArea); if (!cellData) ABORT0();
  }
  if (cellData->IsColSpan()) {
    aColIndex -= cellData->GetColSpanOffset();
    cellData = cellMap->GetDataAt(*mTableCellMap, rgRowIndex, aColIndex, PR_FALSE);
  }
  SetInfo(nextRow, aColIndex, cellData, aAjaInfo, cellMap);
}

// Assign priorities to border styles. For example, styleToPriority(NS_STYLE_BORDER_STYLE_SOLID)
// will return the priority of NS_STYLE_BORDER_STYLE_SOLID. 
static PRUint8 styleToPriority[13] = { 0,  // NS_STYLE_BORDER_STYLE_NONE
                                       3,  // NS_STYLE_BORDER_STYLE_GROOVE
                                       6,  // NS_STYLE_BORDER_STYLE_RIDGE
                                       7,  // NS_STYLE_BORDER_STYLE_DOTTED
                                       8,  // NS_STYLE_BORDER_STYLE_DASHED
                                       10, // NS_STYLE_BORDER_STYLE_SOLID
                                       11, // NS_STYLE_BORDER_STYLE_DOUBLE
                                       2,  // NS_STYLE_BORDER_STYLE_INSET
                                       5,  // NS_STYLE_BORDER_STYLE_OUTSET
                                       12, // NS_STYLE_BORDER_STYLE_HIDDEN
                                       1,  // NS_STYLE_BORDER_STYLE_BG_INSET
                                       4,  // NS_STYLE_BORDER_STYLE_BG_OUTSET
                                       9 };// NS_STYLE_BORDER_STYLE_BG_SOLID
// priority rules follow CSS 2.1 spec
// 'hidden', 'double', 'solid', 'dashed', 'dotted', 'ridge', 'outset', 'groove',
// and the lowest: 'inset'. none is even weaker
#define CELL_CORNER PR_TRUE

/** return the border style, border color for a given frame and side
  * @param aFrame           - query the info for this frame 
  * @param aSide            - the side of the frame
  * @param aStyle           - the border style
  * @param aColor           - the border color
  * @param aTableIsLTR      - table direction is LTR
  * @param aIgnoreTableEdge - if is a table edge any borders set for the purpose
  *                           of satisfying the rules attribute should be ignored
  */
static void 
GetColorAndStyle(const nsIFrame*  aFrame,
                 PRUint8          aSide,
                 PRUint8&         aStyle,
                 nscolor&         aColor,
                 PRBool           aTableIsLTR,
                 PRBool           aIgnoreTableEdge)
{
  NS_PRECONDITION(aFrame, "null frame");
  // initialize out arg
  aColor = 0;
  const nsStyleBorder* styleData = aFrame->GetStyleBorder();
  if(!aTableIsLTR) { // revert the directions
    if (NS_SIDE_RIGHT == aSide) {
      aSide = NS_SIDE_LEFT;
    }
    else if (NS_SIDE_LEFT == aSide) {
      aSide = NS_SIDE_RIGHT;
    }
  }
  aStyle = styleData->GetBorderStyle(aSide);

  // if the rules marker is set, set the style either to none or remove the mask
  if (NS_STYLE_BORDER_STYLE_RULES_MARKER & aStyle) {
    if (aIgnoreTableEdge) {
      aStyle = NS_STYLE_BORDER_STYLE_NONE;
      return;
    }
    else {
      aStyle &= ~NS_STYLE_BORDER_STYLE_RULES_MARKER;
    }
  }

  if ((NS_STYLE_BORDER_STYLE_NONE == aStyle) ||
      (NS_STYLE_BORDER_STYLE_HIDDEN == aStyle)) {
    return;
  }
  PRBool transparent, foreground;
  styleData->GetBorderColor(aSide, aColor, transparent, foreground);
  if (transparent) { 
    aColor = 0;
  }
  else if (foreground) {
    aColor = aFrame->GetStyleColor()->mColor;
  }
}

/** coerce the paint style as required by CSS2.1
  * @param aFrame           - query the info for this frame 
  * @param aSide            - the side of the frame
  * @param aStyle           - the border style
  * @param aColor           - the border color
  * @param aTableIsLTR      - table direction is LTR
  * @param aIgnoreTableEdge - if is a table edge any borders set for the purpose
  *                           of satisfying the rules attribute should be ignored
  */
static void
GetPaintStyleInfo(const nsIFrame*  aFrame,
                  PRUint8          aSide,
                  PRUint8&         aStyle,
                  nscolor&         aColor,
                  PRBool           aTableIsLTR,
                  PRBool           aIgnoreTableEdge)
{
  GetColorAndStyle(aFrame, aSide, aStyle, aColor, aTableIsLTR, aIgnoreTableEdge);
  if ((NS_STYLE_BORDER_STYLE_INSET    == aStyle) || 
      (NS_STYLE_BORDER_STYLE_BG_INSET == aStyle)) {
    aStyle = NS_STYLE_BORDER_STYLE_RIDGE;
  }
  else if ((NS_STYLE_BORDER_STYLE_OUTSET    == aStyle) || 
           (NS_STYLE_BORDER_STYLE_BG_OUTSET == aStyle)) {
    aStyle = NS_STYLE_BORDER_STYLE_GROOVE;
  }
}

/** return the border style, border color and the width in pixel for a given
  * frame and side
  * @param aFrame           - query the info for this frame 
  * @param aSide            - the side of the frame
  * @param aStyle           - the border style
  * @param aColor           - the border color
  * @param aTableIsLTR      - table direction is LTR
  * @param aIgnoreTableEdge - if is a table edge any borders set for the purpose
  *                           of satisfying the rules attribute should be ignored
  * @param aWidth           - the border width in px.
  * @param aTwipsToPixels   - conversion factor from twips to pixel
  */
static void
GetColorAndStyle(const nsIFrame*  aFrame,
                 PRUint8          aSide,
                 PRUint8&         aStyle,
                 nscolor&         aColor,
                 PRBool           aTableIsLTR,
                 PRBool           aIgnoreTableEdge,
                 nscoord&         aWidth,
                 float            aTwipsToPixels)
{
  GetColorAndStyle(aFrame, aSide, aStyle, aColor, aTableIsLTR, aIgnoreTableEdge);
  if ((NS_STYLE_BORDER_STYLE_NONE == aStyle) ||
      (NS_STYLE_BORDER_STYLE_HIDDEN == aStyle)) {
    aWidth = 0;
    return;
  }
  const nsStyleBorder* styleData = aFrame->GetStyleBorder();
  nscoord width;
  if(!aTableIsLTR) { // revert the directions
    if (NS_SIDE_RIGHT == aSide) {
      aSide = NS_SIDE_LEFT;
    }
    else if (NS_SIDE_LEFT == aSide) {
      aSide = NS_SIDE_RIGHT;
    }
  }
  styleData->CalcBorderFor(aFrame, aSide, width);
  aWidth = NSToCoordRound(aTwipsToPixels * (float)width);
}
 
 
/* BCCellBorder represents a border segment which can be either a horizontal
 * or a vertical segment. For each segment we need to know the color, width,
 * style, who owns it and how long it is in cellmap coordinates.
 * Ownership of these segments is  important to calculate which corners should
 * be bevelled. This structure has dual use, its used first to compute the
 * dominant border for horizontal and vertical segments and to store the
 * preliminary computed border results in the BCCellBorders structure.
 * This temporary storage is not symmetric with respect to horizontal and
 * vertical border segments, its always column oriented. For each column in
 * the cellmap there is a temporary stored vertical and horizontal segment.
 * XXX_Bernd this asymmetry is the root of those rowspan bc border errors
 */
struct BCCellBorder
{
  BCCellBorder() { Reset(0, 1); }
  void Reset(PRUint32 aRowIndex, PRUint32 aRowSpan);
  nscolor       color;    // border segment color
  nscoord       width;    // border segment width in pixel coordinates !!
  PRUint8       style;    // border segment style, possible values are defined
                          // in nsStyleConsts.h as NS_STYLE_BORDER_STYLE_*
  BCBorderOwner owner;    // border segment owner, possible values are defined
                          // in celldata.h. In the cellmap for each border
                          // segment we store the owner and later when
                          // painting we know the owner and can retrieve the
                          // style info from the corresponding frame
  PRInt32       rowIndex; // rowIndex of temporary stored horizontal border segments
  PRInt32       rowSpan;  // row span of temporary stored horizontal border segments
};

void
BCCellBorder::Reset(PRUint32 aRowIndex,
                    PRUint32 aRowSpan)
{
  style = NS_STYLE_BORDER_STYLE_NONE;
  color = 0;
  width = 0;
  owner = eTableOwner;
  rowIndex = aRowIndex;
  rowSpan  = aRowSpan;
}

// Compare two border segments, this comparison depends whether the two
// segments meet at a corner and whether the second segment is horizontal.
// The return value is whichever of aBorder1 or aBorder2 dominates.
static const BCCellBorder&
CompareBorders(PRBool              aIsCorner, // Pass PR_TRUE for corner calculations
               const BCCellBorder& aBorder1,
               const BCCellBorder& aBorder2,
               PRBool              aSecondIsHorizontal,
               PRBool*             aFirstDominates = nsnull)
{
  PRBool firstDominates = PR_TRUE;
  
  if (NS_STYLE_BORDER_STYLE_HIDDEN == aBorder1.style) {
    firstDominates = (aIsCorner) ? PR_FALSE : PR_TRUE;
  }
  else if (NS_STYLE_BORDER_STYLE_HIDDEN == aBorder2.style) {
    firstDominates = (aIsCorner) ? PR_TRUE : PR_FALSE;
  }
  else if (aBorder1.width < aBorder2.width) {
    firstDominates = PR_FALSE;
  }
  else if (aBorder1.width == aBorder2.width) {
    if (styleToPriority[aBorder1.style] < styleToPriority[aBorder2.style]) {
      firstDominates = PR_FALSE;
    }
    else if (styleToPriority[aBorder1.style] == styleToPriority[aBorder2.style]) {
      if (aBorder1.owner == aBorder2.owner) {
        firstDominates = !aSecondIsHorizontal;
      }
      else if (aBorder1.owner < aBorder2.owner) {
        firstDominates = PR_FALSE;
      }
    }
  }

  if (aFirstDominates)
    *aFirstDominates = firstDominates;

  if (firstDominates)
    return aBorder1;
  return aBorder2;
}

/** calc the dominant border by considering the table, row/col group, row/col,
  * cell. At the table edges borders coming from the 'rules' attribute should
  * be ignored as they are only inner borders.
  * Depending on whether the side is vertical or horizontal and whether
  * adjacent frames are taken into account the ownership of a single border
  * segment is defined. The return value is the dominating border
  * The cellmap stores only top and left borders for each cellmap position.
  * If the cell border is owned by the cell that is left of the border
  * it will be an adjacent owner aka eAjaCellOwner. See celldata.h for the other
  * scenarios with a adjacent owner.
  * @param xxxFrame         - the frame for style information, might be zero if
  *                           it should not be considered
  * @param aIgnoreTableEdge - if true the border should be ignored at the table
  *                           edge, as rules can be drawn only inside the table
  * @param aSide            - side of the frames that should be considered
  * @param aAja             - the border comparison takes place from the point of
  *                           a frame that is adjacent to the cellmap entry, for
  *                           when a cell owns its lower border it will be the
  *                           adjacent owner as in the cellmap only top and left
  *                           borders are stored. 
  * @param aTwipsToPixels   - conversion factor as borders need to be drawn pixel
  *                           aligned.
  */
static BCCellBorder
CompareBorders(const nsIFrame*  aTableFrame,
               const nsIFrame*  aColGroupFrame,
               const nsIFrame*  aColFrame,
               const nsIFrame*  aRowGroupFrame,
               const nsIFrame*  aRowFrame,
               const nsIFrame*  aCellFrame,
               PRBool           aTableIsLTR,
               PRBool           aIgnoreTableEdge,
               PRUint8          aSide,
               PRBool           aAja,
               float            aTwipsToPixels)
{
  BCCellBorder border, tempBorder;
  PRBool horizontal = (NS_SIDE_TOP == aSide) || (NS_SIDE_BOTTOM == aSide);

  // start with the table as dominant if present
  if (aTableFrame) {
    GetColorAndStyle(aTableFrame, aSide, border.style, border.color, aTableIsLTR, aIgnoreTableEdge, border.width, aTwipsToPixels);
    border.owner = eTableOwner;
    if (NS_STYLE_BORDER_STYLE_HIDDEN == border.style) {
      return border;
    }
  }
  // see if the colgroup is dominant
  if (aColGroupFrame) {
    GetColorAndStyle(aColGroupFrame, aSide, tempBorder.style, tempBorder.color, aTableIsLTR, aIgnoreTableEdge, tempBorder.width, aTwipsToPixels);
    tempBorder.owner = (aAja && !horizontal) ? eAjaColGroupOwner : eColGroupOwner;
    // pass here and below PR_FALSE for aSecondIsHorizontal as it is only used for corner calculations.
    border = CompareBorders(!CELL_CORNER, border, tempBorder, PR_FALSE);
    if (NS_STYLE_BORDER_STYLE_HIDDEN == border.style) {
      return border;
    }
  }
  // see if the col is dominant
  if (aColFrame) {
    GetColorAndStyle(aColFrame, aSide, tempBorder.style, tempBorder.color, aTableIsLTR, aIgnoreTableEdge, tempBorder.width, aTwipsToPixels);
    tempBorder.owner = (aAja && !horizontal) ? eAjaColOwner : eColOwner;
    border = CompareBorders(!CELL_CORNER, border, tempBorder, PR_FALSE);
    if (NS_STYLE_BORDER_STYLE_HIDDEN == border.style) {
      return border;
    }
  }
  // see if the rowgroup is dominant
  if (aRowGroupFrame) {
    GetColorAndStyle(aRowGroupFrame, aSide, tempBorder.style, tempBorder.color, aTableIsLTR, aIgnoreTableEdge, tempBorder.width, aTwipsToPixels);
    tempBorder.owner = (aAja && horizontal) ? eAjaRowGroupOwner : eRowGroupOwner;
    border = CompareBorders(!CELL_CORNER, border, tempBorder, PR_FALSE);
    if (NS_STYLE_BORDER_STYLE_HIDDEN == border.style) {
      return border;
    }
  }
  // see if the row is dominant
  if (aRowFrame) {
    GetColorAndStyle(aRowFrame, aSide, tempBorder.style, tempBorder.color, aTableIsLTR, aIgnoreTableEdge, tempBorder.width, aTwipsToPixels);
    tempBorder.owner = (aAja && horizontal) ? eAjaRowOwner : eRowOwner;
    border = CompareBorders(!CELL_CORNER, border, tempBorder, PR_FALSE);
    if (NS_STYLE_BORDER_STYLE_HIDDEN == border.style) {
      return border;
    }
  }
  // see if the cell is dominant
  if (aCellFrame) {
    GetColorAndStyle(aCellFrame, aSide, tempBorder.style, tempBorder.color, aTableIsLTR, aIgnoreTableEdge, tempBorder.width, aTwipsToPixels);
    tempBorder.owner = (aAja) ? eAjaCellOwner : eCellOwner;
    border = CompareBorders(!CELL_CORNER, border, tempBorder, PR_FALSE);
  }
  return border;
}

static PRBool 
Perpendicular(PRUint8 aSide1, 
              PRUint8 aSide2)
{
  switch (aSide1) {
  case NS_SIDE_TOP:
    return (NS_SIDE_BOTTOM != aSide2);
  case NS_SIDE_RIGHT:
    return (NS_SIDE_LEFT != aSide2);
  case NS_SIDE_BOTTOM:
    return (NS_SIDE_TOP != aSide2);
  default: // NS_SIDE_LEFT
    return (NS_SIDE_RIGHT != aSide2);
  }
}

// XXX allocate this as number-of-cols+1 instead of number-of-cols+1 * number-of-rows+1
struct BCCornerInfo 
{
  BCCornerInfo() { ownerColor = 0; ownerWidth = subWidth = ownerSide = ownerElem = subSide = 
                   subElem = hasDashDot = numSegs = bevel = 0;
                   ownerStyle = 0xFF; subStyle = NS_STYLE_BORDER_STYLE_SOLID;  }
  void Set(PRUint8       aSide,
           BCCellBorder  border);

  void Update(PRUint8       aSide,
              BCCellBorder  border);

  nscolor   ownerColor;     // color of borderOwner
  PRUint16  ownerWidth;     // pixel width of borderOwner 
  PRUint16  subWidth;       // pixel width of the largest border intersecting the border perpendicular 
                            // to ownerSide
  PRUint32  ownerSide:2;    // side (e.g NS_SIDE_TOP, NS_SIDE_RIGHT, etc) of the border owning 
                            // the corner relative to the corner
  PRUint32  ownerElem:3;    // elem type (e.g. eTable, eGroup, etc) owning the corner
  PRUint32  ownerStyle:8;   // border style of ownerElem
  PRUint32  subSide:2;      // side of border with subWidth relative to the corner
  PRUint32  subElem:3;      // elem type (e.g. eTable, eGroup, etc) of sub owner
  PRUint32  subStyle:8;     // border style of subElem
  PRUint32  hasDashDot:1;   // does a dashed, dotted segment enter the corner, they cannot be beveled
  PRUint32  numSegs:3;      // number of segments entering corner
  PRUint32  bevel:1;        // is the corner beveled (uses the above two fields together with subWidth)
  PRUint32  unused:1;
};

void 
BCCornerInfo::Set(PRUint8       aSide,
                  BCCellBorder  aBorder)
{
  ownerElem  = aBorder.owner;
  ownerStyle = aBorder.style;
  ownerWidth = aBorder.width;
  ownerColor = aBorder.color;
  ownerSide  = aSide;
  hasDashDot = 0;
  numSegs    = 0;
  if (aBorder.width > 0) {
    numSegs++;
    hasDashDot = (NS_STYLE_BORDER_STYLE_DASHED == aBorder.style) ||
                 (NS_STYLE_BORDER_STYLE_DOTTED == aBorder.style);
  }
  bevel      = 0;
  subWidth   = 0;
  // the following will get set later
  subSide    = ((aSide == NS_SIDE_LEFT) || (aSide == NS_SIDE_RIGHT)) ? NS_SIDE_TOP : NS_SIDE_LEFT; 
  subElem    = eTableOwner;
  subStyle   = NS_STYLE_BORDER_STYLE_SOLID; 
}

void 
BCCornerInfo::Update(PRUint8       aSide,
                     BCCellBorder  aBorder)
{
  PRBool existingWins = PR_FALSE;
  if (0xFF == ownerStyle) { // initial value indiating that it hasn't been set yet
    Set(aSide, aBorder);
  }
  else {
    PRBool horizontal = (NS_SIDE_LEFT == aSide) || (NS_SIDE_RIGHT == aSide); // relative to the corner
    BCCellBorder oldBorder, tempBorder;
    oldBorder.owner  = (BCBorderOwner) ownerElem;
    oldBorder.style =  ownerStyle;
    oldBorder.width =  ownerWidth;
    oldBorder.color =  ownerColor;

    PRUint8 oldSide  = ownerSide;
    
    tempBorder = CompareBorders(CELL_CORNER, oldBorder, aBorder, horizontal, &existingWins); 
                         
    ownerElem  = tempBorder.owner;
    ownerStyle = tempBorder.style;
    ownerWidth = tempBorder.width;
    ownerColor = tempBorder.color;
    if (existingWins) { // existing corner is dominant
      if (::Perpendicular(ownerSide, aSide)) {
        // see if the new sub info replaces the old
        BCCellBorder subBorder;
        subBorder.owner = (BCBorderOwner) subElem;
        subBorder.style =  subStyle;
        subBorder.width =  subWidth;
        subBorder.color = 0; // we are not interested in subBorder color
        PRBool firstWins;

        tempBorder = CompareBorders(CELL_CORNER, subBorder, aBorder, horizontal, &firstWins);
        
        subElem  = tempBorder.owner;
        subStyle = tempBorder.style;
        subWidth = tempBorder.width;
        if (!firstWins) {
          subSide = aSide; 
        }
      }
    }
    else { // input args are dominant
      ownerSide = aSide;
      if (::Perpendicular(oldSide, ownerSide)) {
        subElem  = oldBorder.owner;
        subStyle = oldBorder.style;
        subWidth = oldBorder.width;
        subSide  = oldSide;
      }
    }
    if (aBorder.width > 0) {
      numSegs++;
      if (!hasDashDot && ((NS_STYLE_BORDER_STYLE_DASHED == aBorder.style) ||
                          (NS_STYLE_BORDER_STYLE_DOTTED == aBorder.style))) {
        hasDashDot = 1;
      }
    }
  
    // bevel the corner if only two perpendicular non dashed/dotted segments enter the corner
    bevel = (2 == numSegs) && (subWidth > 1) && (0 == hasDashDot);
  }
}

struct BCCorners
{
  BCCorners(PRInt32 aNumCorners,
            PRInt32 aStartIndex);

  ~BCCorners() { delete [] corners; }
  
  BCCornerInfo& operator [](PRInt32 i) const
  { NS_ASSERTION((i >= startIndex) && (i <= endIndex), "program error");
    return corners[PR_MAX(PR_MIN(i, endIndex), startIndex) - startIndex]; }

  PRInt32       startIndex;
  PRInt32       endIndex;
  BCCornerInfo* corners;
};
  
BCCorners::BCCorners(PRInt32 aNumCorners,
                     PRInt32 aStartIndex)
{
  NS_ASSERTION((aNumCorners > 0) && (aStartIndex >= 0), "program error");
  startIndex = aStartIndex;
  endIndex   = aStartIndex + aNumCorners - 1;
  corners    = new BCCornerInfo[aNumCorners]; 
}


struct BCCellBorders
{
  BCCellBorders(PRInt32 aNumBorders,
                PRInt32 aStartIndex);

  ~BCCellBorders() { delete [] borders; }
  
  BCCellBorder& operator [](PRInt32 i) const
  { NS_ASSERTION((i >= startIndex) && (i <= endIndex), "program error");
    return borders[PR_MAX(PR_MIN(i, endIndex), startIndex) - startIndex]; }

  PRInt32       startIndex;
  PRInt32       endIndex;
  BCCellBorder* borders;
};
  
BCCellBorders::BCCellBorders(PRInt32 aNumBorders,
                             PRInt32 aStartIndex)
{
  NS_ASSERTION((aNumBorders > 0) && (aStartIndex >= 0), "program error");
  startIndex = aStartIndex;
  endIndex   = aStartIndex + aNumBorders - 1;
  borders    = new BCCellBorder[aNumBorders]; 
}

// this function sets the new border properties and returns true if the border
// segment will start a new segment and not prolong the existing segment.
static PRBool
SetBorder(const BCCellBorder&   aNewBorder,
          BCCellBorder&         aBorder)
{
  PRBool changed = (aNewBorder.style != aBorder.style) ||
                   (aNewBorder.width != aBorder.width) ||
                   (aNewBorder.color != aBorder.color);
  aBorder.color        = aNewBorder.color;
  aBorder.width        = aNewBorder.width;
  aBorder.style        = aNewBorder.style;
  aBorder.owner        = aNewBorder.owner;

  return changed;
}

// this function will set the horizontal border. It will return true if the 
// existing segment will not be continued. Having a vertical owner of a corner
// should also start a new segment.
static PRBool
SetHorBorder(const BCCellBorder& aNewBorder,
             const BCCornerInfo& aCorner,
             BCCellBorder&       aBorder)
{
  PRBool startSeg = ::SetBorder(aNewBorder, aBorder);
  if (!startSeg) {
    startSeg = ((NS_SIDE_LEFT != aCorner.ownerSide) && (NS_SIDE_RIGHT != aCorner.ownerSide));
  }
  return startSeg;
}

// Make the damage area larger on the top and bottom by at least one row and on the left and right 
// at least one column. This is done so that adjacent elements are part of the border calculations. 
// The extra segments and borders outside the actual damage area will not be updated in the cell map, 
// because they in turn would need info from adjacent segments outside the damage area to be accurate.
void
nsTableFrame::ExpandBCDamageArea(nsRect& aRect) const
{
  PRInt32 numRows = GetRowCount();
  PRInt32 numCols = GetColCount();

  PRInt32 dStartX = aRect.x;
  PRInt32 dEndX   = aRect.XMost() - 1;
  PRInt32 dStartY = aRect.y;
  PRInt32 dEndY   = aRect.YMost() - 1;

  // expand the damage area in each direction
  if (dStartX > 0) {
    dStartX--;
  }
  if (dEndX < (numCols - 1)) {
    dEndX++;
  }
  if (dStartY > 0) {
    dStartY--;
  }
  if (dEndY < (numRows - 1)) {
    dEndY++;
  }
  // Check the damage area so that there are no cells spanning in or out. If there are any then
  // make the damage area as big as the table, similarly to the way the cell map decides whether
  // to rebuild versus expand. This could be optimized to expand to the smallest area that contains
  // no spanners, but it may not be worth the effort in general, and it would need to be done in the
  // cell map as well.
  PRBool haveSpanner = PR_FALSE;
  if ((dStartX > 0) || (dEndX < (numCols - 1)) || (dStartY > 0) || (dEndY < (numRows - 1))) {
    nsTableCellMap* tableCellMap = GetCellMap(); if (!tableCellMap) ABORT0();
    // Get the ordered row groups 
    PRUint32 numRowGroups;
    nsVoidArray rowGroups;
    OrderRowGroups(rowGroups, numRowGroups, nsnull);
    for (PRUint32 rgX = 0; rgX < numRowGroups; rgX++) {
      nsIFrame* kidFrame = (nsIFrame*)rowGroups.ElementAt(rgX);
      nsTableRowGroupFrame* rgFrame = GetRowGroupFrame(kidFrame); if (!rgFrame) ABORT0();
      PRInt32 rgStartY = rgFrame->GetStartRowIndex();
      PRInt32 rgEndY   = rgStartY + rgFrame->GetRowCount() - 1;
      if (dEndY < rgStartY) 
        break;
      nsCellMap* cellMap = tableCellMap->GetMapFor(*rgFrame); if (!cellMap) ABORT0();
      // check for spanners from above and below
      if ((dStartY > 0) && (dStartY >= rgStartY) && (dStartY <= rgEndY)) {
        nsVoidArray* row = (nsVoidArray*)cellMap->mRows.ElementAt(dStartY - rgStartY); if (!row) ABORT0();
        for (PRInt32 x = dStartX; x <= dEndX; x++) {
          CellData* cellData = (row->Count() > x) ? (CellData*)row->ElementAt(x) : nsnull;
          if (cellData && (cellData->IsRowSpan())) {
             haveSpanner = PR_TRUE;
             break;
          }
        }
        if (dEndY < rgEndY) {
          row = (nsVoidArray*)cellMap->mRows.ElementAt(dEndY + 1 - rgStartY); if (!row) ABORT0();
          for (PRInt32 x = dStartX; x <= dEndX; x++) {
            CellData* cellData = (CellData*)row->SafeElementAt(x);
            if (cellData && (cellData->IsRowSpan())) {
              haveSpanner = PR_TRUE;
              break;
            }
          }
        }
      }
      // check for spanners on the left and right
      PRInt32 iterStartY = -1;
      PRInt32 iterEndY   = -1;
      if ((dStartY >= rgStartY) && (dStartY <= rgEndY)) {
        // the damage area starts in the row group
        iterStartY = dStartY;
        iterEndY   = PR_MIN(dEndY, rgEndY);
      }
      else if ((dEndY >= rgStartY) && (dEndY <= rgEndY)) {
        // the damage area ends in the row group
        iterStartY = rgStartY;
        iterEndY   = PR_MIN(dEndY, rgStartY);
      }
      else if ((rgStartY >= dStartY) && (rgEndY <= dEndY)) {
        // the damage area contains the row group
        iterStartY = rgStartY;
        iterEndY   = rgEndY;
      }
      if ((iterStartY >= 0) && (iterEndY >= 0)) {
        for (PRInt32 y = iterStartY; y <= iterEndY; y++) {
          nsVoidArray* row = (nsVoidArray*)cellMap->mRows.ElementAt(y - rgStartY); if (!row) ABORT0();
          CellData* cellData = (CellData*)row->SafeElementAt(dStartX);
          if (cellData && (cellData->IsColSpan())) {
            haveSpanner = PR_TRUE;
            break;
          }
          if (dEndX < (numCols - 1)) {
            cellData = (CellData*)row->SafeElementAt(dEndX + 1);
            if (cellData && (cellData->IsColSpan())) {
              haveSpanner = PR_TRUE;
              break;
            }
          }
        }
      }
    }
  }
  if (haveSpanner) {
    // make the damage area the whole table
    aRect.x      = 0;
    aRect.y      = 0;
    aRect.width  = numCols;
    aRect.height = numRows;
  }
  else {
    aRect.x      = dStartX;
    aRect.y      = dStartY;
    aRect.width  = 1 + dEndX - dStartX;
    aRect.height = 1 + dEndY - dStartY;
  }
}

#define MAX_TABLE_BORDER_WIDTH 255
static PRUint8
LimitBorderWidth(PRUint16 aWidth)
{
  return PR_MIN(MAX_TABLE_BORDER_WIDTH, aWidth);
}

/* Here is the order for storing border edges in the cell map as a cell is processed. There are 
   n=colspan top and bottom border edges per cell and n=rowspan left and right border edges per cell.

   1) On the top edge of the table, store the top edge. Never store the top edge otherwise, since
      a bottom edge from a cell above will take care of it.
   2) On the left edge of the table, store the left edge. Never store the left edge othewise, since
      a right edge from a cell to the left will take care of it.
   3) Store the right edge (or edges if a row span) 
   4) Store the bottom edge (or edges if a col span)
    
   Since corners are computed with only an array of BCCornerInfo indexed by the number-of-cols, corner
   calculations are somewhat complicated. Using an array with number-of-rows * number-of-col entries
   would simplify this, but at an extra in memory cost of nearly 12 bytes per cell map entry. Collapsing 
   borders already have about an extra 8 byte per cell map entry overhead (this could be
   reduced to 4 bytes if we are willing to not store border widths in nsTableCellFrame), Here are the 
   rules in priority order for storing cornes in the cell map as a cell is processed. top-left means the
   left endpoint of the border edge on the top of the cell. There are n=colspan top and bottom border 
   edges per cell and n=rowspan left and right border edges per cell.

   1) On the top edge of the table, store the top-left corner, unless on the left edge of the table.
      Never store the top-right corner, since it will get stored as a right-top corner.
   2) On the left edge of the table, store the left-top corner. Never store the left-bottom corner,
      since it will get stored as a bottom-left corner.
   3) Store the right-top corner if (a) it is the top right corner of the table or (b) it is not on
      the top edge of the table. Never store the right-bottom corner since it will get stored as a 
      bottom-right corner.
   4) Store the bottom-right corner, if it is the bottom right corner of the table. Never store it 
      otherwise, since it will get stored as either a right-top corner by a cell below or
      a bottom-left corner from a cell to the right.
   5) Store the bottom-left corner, if (a) on the bottom edge of the table or (b) if the left edge hits 
      the top side of a colspan in its interior. Never store the corner otherwise, since it will 
      get stored as a right-top corner by a cell from below.

   XXX the BC-RTL hack - The correct fix would be a rewrite as described in bug 203686.
   In order to draw borders in rtl conditions somehow correct, the existing structure which relies
   heavily on the assumption that the next cell sibling will be on the right side, has been modified.
   We flip the border during painting and during style lookup. Look for tableIsLTR for places where
   the flipping is done.
 */

#define TOP_DAMAGED(aRowIndex)    ((aRowIndex) >= propData->mDamageArea.y) 
#define RIGHT_DAMAGED(aColIndex)  ((aColIndex) <  propData->mDamageArea.XMost()) 
#define BOTTOM_DAMAGED(aRowIndex) ((aRowIndex) <  propData->mDamageArea.YMost()) 
#define LEFT_DAMAGED(aColIndex)   ((aColIndex) >= propData->mDamageArea.x) 

#define TABLE_EDGE  PR_TRUE
#define ADJACENT    PR_TRUE
#define HORIZONTAL  PR_TRUE

// Calc the dominant border at every cell edge and corner within the current damage area
void 
nsTableFrame::CalcBCBorders()
{
  nsTableCellMap* tableCellMap = GetCellMap(); if (!tableCellMap) ABORT0();
  PRInt32 numRows = GetRowCount();
  PRInt32 numCols = GetColCount();
  
  // Get the property holding the table damage area and border widths
  BCPropertyData* propData = 
    (BCPropertyData*)nsTableFrame::GetProperty(this, nsLayoutAtoms::tableBCProperty, PR_FALSE);
  if (!propData) ABORT0();

  PRBool tableIsLTR = GetStyleVisibility()->mDirection == NS_STYLE_DIRECTION_LTR;
  PRUint8 firstSide, secondSide;
  if (tableIsLTR) {
    firstSide  = NS_SIDE_LEFT;
    secondSide = NS_SIDE_RIGHT;
  }
  else {
    firstSide  = NS_SIDE_RIGHT;
    secondSide = NS_SIDE_LEFT;
  }
  CheckFixDamageArea(numRows, numCols, propData->mDamageArea);
  // calculate an expanded damage area 
  nsRect damageArea(propData->mDamageArea);
  ExpandBCDamageArea(damageArea);

  // segments that are on the table border edges need to be initialized only once
  PRBool tableBorderReset[4];
  for (PRUint32 sideX = NS_SIDE_TOP; sideX <= NS_SIDE_LEFT; sideX++) {
    tableBorderReset[sideX] = PR_FALSE;
  }
  GET_TWIPS_TO_PIXELS(GetPresContext(), t2p);

  // vertical borders indexed in x-direction (cols)
  BCCellBorders lastVerBorders(damageArea.width + 1, damageArea.x); if (!lastVerBorders.borders) ABORT0();
  BCCellBorder  lastTopBorder, lastBottomBorder;
  // horizontal borders indexed in x-direction (cols)
  BCCellBorders lastBottomBorders(damageArea.width + 1, damageArea.x); if (!lastBottomBorders.borders) ABORT0();
  PRBool startSeg;
  PRBool gotRowBorder = PR_FALSE;

  BCMapCellInfo  info, ajaInfo;
  BCCellBorder currentBorder, adjacentBorder;
  PRInt32   cellEndRowIndex = -1;
  PRInt32   cellEndColIndex = -1;
  BCCorners topCorners(damageArea.width + 1, damageArea.x); if (!topCorners.corners) ABORT0();
  BCCorners bottomCorners(damageArea.width + 1, damageArea.x); if (!bottomCorners.corners) ABORT0();

  BCMapCellIterator iter(*this, damageArea);
  for (iter.First(info); !iter.mAtEnd; iter.Next(info)) {

    cellEndRowIndex = info.rowIndex + info.rowSpan - 1;
    cellEndColIndex = info.colIndex + info.colSpan - 1;
    
    PRBool bottomRowSpan = PR_FALSE;
    // see if lastTopBorder, lastBottomBorder need to be reset
    if (iter.IsNewRow()) { 
      gotRowBorder = PR_FALSE;
      lastTopBorder.Reset(info.rowIndex, info.rowSpan);
      lastBottomBorder.Reset(cellEndRowIndex + 1, info.rowSpan);
    }
    else if (info.colIndex > damageArea.x) {
      lastBottomBorder = lastBottomBorders[info.colIndex - 1];
      if (info.rowIndex > lastBottomBorder.rowIndex - lastBottomBorder.rowSpan) { 
        // the top border's left edge butts against the middle of a rowspan
        lastTopBorder.Reset(info.rowIndex, info.rowSpan);
      }
      if (lastBottomBorder.rowIndex > (cellEndRowIndex + 1)) {
        // the bottom border's left edge butts against the middle of a rowspan
        lastBottomBorder.Reset(cellEndRowIndex + 1, info.rowSpan);
        bottomRowSpan = PR_TRUE;
      }
    }

    // find the dominant border considering the cell's top border and the table, row group, row
    // if the border is at the top of the table, otherwise it was processed in a previous row
    if (0 == info.rowIndex) {
      if (!tableBorderReset[NS_SIDE_TOP]) {
        propData->mTopBorderWidth = 0;
        tableBorderReset[NS_SIDE_TOP] = PR_TRUE;
      }
      for (PRInt32 colX = info.colIndex; colX <= cellEndColIndex; colX++) {
        nsIFrame* colFrame = GetColFrame(colX); if (!colFrame) ABORT0();
        nsIFrame* cgFrame = colFrame->GetParent(); if (!cgFrame) ABORT0();
        currentBorder = CompareBorders(this, cgFrame, colFrame, info.rg, info.topRow,
                                       info.cell, tableIsLTR, TABLE_EDGE, NS_SIDE_TOP,
                                       !ADJACENT, t2p);
        // update/store the top left & top right corners of the seg 
        BCCornerInfo& tlCorner = topCorners[colX]; // top left
        if (0 == colX) {
          tlCorner.Set(NS_SIDE_RIGHT, currentBorder); // we are on right hand side of the corner
        }
        else {
          tlCorner.Update(NS_SIDE_RIGHT, currentBorder);
          tableCellMap->SetBCBorderCorner(eTopLeft, *info.cellMap, 0, 0, colX,
                                          tlCorner.ownerSide, tlCorner.subWidth, tlCorner.bevel);
        }
        topCorners[colX + 1].Set(NS_SIDE_LEFT, currentBorder); // top right
        // update lastTopBorder and see if a new segment starts
        startSeg = SetHorBorder(currentBorder, tlCorner, lastTopBorder);
        // store the border segment in the cell map
        tableCellMap->SetBCBorderEdge(NS_SIDE_TOP, *info.cellMap, 0, 0, colX,
                                      1, currentBorder.owner, currentBorder.width, startSeg);
        // update the affected borders of the cell, row, and table
        if (info.cell) {
          info.cell->SetBorderWidth(NS_SIDE_TOP, PR_MAX(currentBorder.width, info.cell->GetBorderWidth(NS_SIDE_TOP)));
        }
        if (info.topRow) {
          BCPixelSize half = BC_BORDER_BOTTOM_HALF(currentBorder.width);
          info.topRow->SetTopBCBorderWidth(PR_MAX(half, info.topRow->GetTopBCBorderWidth()));
        }
        propData->mTopBorderWidth = LimitBorderWidth(PR_MAX(propData->mTopBorderWidth, (PRUint8)currentBorder.width));
        //calculate column continuous borders
        //we only need to do this once, so we'll do it only on the first row
        currentBorder = CompareBorders(this, cgFrame, colFrame, info.rg,
                                       info.topRow, nsnull, tableIsLTR, 
                                       TABLE_EDGE, NS_SIDE_TOP, !ADJACENT, t2p);
        ((nsTableColFrame*)colFrame)->SetContinuousBCBorderWidth(NS_SIDE_TOP,
                                                                 currentBorder.width);
        if (numCols == cellEndColIndex + 1) {
          currentBorder = CompareBorders(this, cgFrame, colFrame, nsnull,
                                         nsnull, nsnull, tableIsLTR, TABLE_EDGE,
                                         NS_SIDE_RIGHT, !ADJACENT, t2p);
        }
        else {
          currentBorder = CompareBorders(nsnull, cgFrame, colFrame, nsnull,
                                         nsnull, nsnull, tableIsLTR, !TABLE_EDGE,
                                         NS_SIDE_RIGHT, !ADJACENT, t2p);
        }
        ((nsTableColFrame*)colFrame)->SetContinuousBCBorderWidth(NS_SIDE_RIGHT,
                                                                 currentBorder.width);
        
      }
      //calculate continuous top first row & rowgroup border: special case
      //because it must include the table in the collapse
      if (info.topRow) {
        currentBorder = CompareBorders(this, nsnull, nsnull, info.rg,
                                       info.topRow, nsnull, tableIsLTR,
                                       TABLE_EDGE, NS_SIDE_TOP, !ADJACENT, t2p);
        info.topRow->SetContinuousBCBorderWidth(NS_SIDE_TOP, currentBorder.width);
      }
      if (info.cgRight && info.cg) {
        //calculate continuous top colgroup border once per colgroup
        currentBorder = CompareBorders(this, info.cg, nsnull, info.rg,
                                       info.topRow, nsnull, tableIsLTR, 
                                       TABLE_EDGE, NS_SIDE_TOP, !ADJACENT, t2p);
        info.cg->SetContinuousBCBorderWidth(NS_SIDE_TOP, currentBorder.width);
      }
      if (0 == info.colIndex) {
        currentBorder = CompareBorders(this, info.cg, info.leftCol, nsnull,
                                       nsnull, nsnull, tableIsLTR, TABLE_EDGE,
                                       NS_SIDE_LEFT, !ADJACENT, t2p);
        mBits.mLeftContBCBorder = currentBorder.width;
      }
    }
    else {
      // see if the top border needs to be the start of a segment due to a vertical border owning the corner
      if (info.colIndex > 0) {
        BCData& data = ((BCCellData*)info.cellData)->mData;
        if (!data.IsTopStart()) {
          PRUint8 cornerSide;
          PRPackedBool bevel;
          data.GetCorner(cornerSide, bevel);
          if ((NS_SIDE_TOP == cornerSide) || (NS_SIDE_BOTTOM == cornerSide)) {
            data.SetTopStart(PR_TRUE);
          }
        }
      }  
    }

    // find the dominant border considering the cell's left border and the table, col group, col  
    // if the border is at the left of the table, otherwise it was processed in a previous col
    if (0 == info.colIndex) {
      if (!tableBorderReset[NS_SIDE_LEFT]) {
        propData->mLeftBorderWidth = 0;
        tableBorderReset[NS_SIDE_LEFT] = PR_TRUE;
      }
      nsTableRowFrame* rowFrame = nsnull;
      for (PRInt32 rowX = info.rowIndex; rowX <= cellEndRowIndex; rowX++) {
        rowFrame = (rowX == info.rowIndex) ? info.topRow : rowFrame->GetNextRow();
        currentBorder = CompareBorders(this, info.cg, info.leftCol, info.rg, rowFrame, info.cell, 
                                       tableIsLTR, TABLE_EDGE, NS_SIDE_LEFT, !ADJACENT, t2p);
        BCCornerInfo& tlCorner = (0 == rowX) ? topCorners[0] : bottomCorners[0]; // top left
        tlCorner.Update(NS_SIDE_BOTTOM, currentBorder);
        tableCellMap->SetBCBorderCorner(eTopLeft, *info.cellMap, iter.mRowGroupStart, rowX, 
                                        0, tlCorner.ownerSide, tlCorner.subWidth, tlCorner.bevel);
        bottomCorners[0].Set(NS_SIDE_TOP, currentBorder); // bottom left             
        // update lastVerBordersBorder and see if a new segment starts
        startSeg = SetBorder(currentBorder, lastVerBorders[0]);
        // store the border segment in the cell map 
        tableCellMap->SetBCBorderEdge(NS_SIDE_LEFT, *info.cellMap, iter.mRowGroupStart, rowX, 
                                      info.colIndex, 1, currentBorder.owner, currentBorder.width, startSeg);
        // update the left border of the cell, col and table
        if (info.cell) {
          info.cell->SetBorderWidth(firstSide, PR_MAX(currentBorder.width, info.cell->GetBorderWidth(firstSide)));
        }
        if (info.leftCol) {
          BCPixelSize half = BC_BORDER_RIGHT_HALF(currentBorder.width);
          info.leftCol->SetLeftBorderWidth(PR_MAX(half, info.leftCol->GetLeftBorderWidth()));
        }
        propData->mLeftBorderWidth = LimitBorderWidth(PR_MAX(propData->mLeftBorderWidth, currentBorder.width));
        //get row continuous borders
        if (rowFrame) {
          currentBorder = CompareBorders(this, info.cg, info.leftCol,
                                         info.rg, rowFrame, nsnull, tableIsLTR,
                                         TABLE_EDGE, NS_SIDE_LEFT, !ADJACENT, t2p);
          rowFrame->SetContinuousBCBorderWidth(firstSide, currentBorder.width);
        }
      }
      //get row group continuous borders
      if (info.rgBottom && info.rg) { //once per row group, so check for bottom
        currentBorder = CompareBorders(this, info.cg, info.leftCol, info.rg, nsnull,
                                       nsnull, tableIsLTR, TABLE_EDGE, NS_SIDE_LEFT,
                                       !ADJACENT, t2p);
        info.rg->SetContinuousBCBorderWidth(firstSide, currentBorder.width);
      }
    }

    // find the dominant border considering the cell's right border, adjacent cells and the table, row group, row
    if (numCols == cellEndColIndex + 1) { // touches right edge of table
      if (!tableBorderReset[NS_SIDE_RIGHT]) {
        propData->mRightBorderWidth = 0;
        tableBorderReset[NS_SIDE_RIGHT] = PR_TRUE;
      }
      nsTableRowFrame* rowFrame = nsnull;
      for (PRInt32 rowX = info.rowIndex; rowX <= cellEndRowIndex; rowX++) {
        rowFrame = (rowX == info.rowIndex) ? info.topRow : rowFrame->GetNextRow();
        currentBorder = CompareBorders(this, info.cg, info.rightCol, info.rg, rowFrame, info.cell, 
                                       tableIsLTR, TABLE_EDGE, NS_SIDE_RIGHT, ADJACENT, t2p);
        // update/store the top right & bottom right corners 
        BCCornerInfo& trCorner = (0 == rowX) ? topCorners[cellEndColIndex + 1] : bottomCorners[cellEndColIndex + 1]; 
        trCorner.Update(NS_SIDE_BOTTOM, currentBorder);   // top right
        tableCellMap->SetBCBorderCorner(eTopRight, *info.cellMap, iter.mRowGroupStart, rowX, 
                                        cellEndColIndex, trCorner.ownerSide, trCorner.subWidth, trCorner.bevel);
        BCCornerInfo& brCorner = bottomCorners[cellEndColIndex + 1];
        brCorner.Set(NS_SIDE_TOP, currentBorder); // bottom right
        tableCellMap->SetBCBorderCorner(eBottomRight, *info.cellMap, iter.mRowGroupStart, rowX,
                                        cellEndColIndex, brCorner.ownerSide, brCorner.subWidth, brCorner.bevel);
        // update lastVerBorders and see if a new segment starts
        startSeg = SetBorder(currentBorder, lastVerBorders[cellEndColIndex + 1]);
        // store the border segment in the cell map and update cellBorders
        tableCellMap->SetBCBorderEdge(NS_SIDE_RIGHT, *info.cellMap, iter.mRowGroupStart, rowX,
                                      cellEndColIndex, 1, currentBorder.owner, currentBorder.width, startSeg);
        // update the affected borders of the cell, col, and table
        if (info.cell) {
          info.cell->SetBorderWidth(secondSide, PR_MAX(currentBorder.width, info.cell->GetBorderWidth(secondSide)));
        }
        if (info.rightCol) {
          BCPixelSize half = BC_BORDER_LEFT_HALF(currentBorder.width);
          info.rightCol->SetRightBorderWidth(PR_MAX(half, info.rightCol->GetRightBorderWidth()));
        }
        propData->mRightBorderWidth = LimitBorderWidth(PR_MAX(propData->mRightBorderWidth, currentBorder.width));
        //get row continuous borders
        if (rowFrame) {
          currentBorder = CompareBorders(this, info.cg, info.rightCol, info.rg,
                                         rowFrame, nsnull, tableIsLTR, TABLE_EDGE,
                                         NS_SIDE_RIGHT, ADJACENT, t2p);
          rowFrame->SetContinuousBCBorderWidth(secondSide, currentBorder.width);
        }
      }
      //get row group continuous borders
      if (info.rgBottom && info.rg) { //once per rg, so check for bottom
        currentBorder = CompareBorders(this, info.cg, info.rightCol, info.rg, 
                                       nsnull, nsnull, tableIsLTR, TABLE_EDGE,
                                       NS_SIDE_RIGHT, ADJACENT, t2p);
        info.rg->SetContinuousBCBorderWidth(secondSide, currentBorder.width);
      }
    }
    else {
      PRInt32 segLength = 0;
      BCMapCellInfo priorAjaInfo;
      for (PRInt32 rowX = info.rowIndex; rowX <= cellEndRowIndex; rowX += segLength) {
        iter.PeekRight(info, rowX, ajaInfo);
        const nsIFrame* cg = (info.cgRight) ? info.cg : nsnull;
        currentBorder = CompareBorders(nsnull, cg, info.rightCol, nsnull, nsnull, info.cell,
                                       tableIsLTR, !TABLE_EDGE, NS_SIDE_RIGHT, ADJACENT, t2p);
        cg = (ajaInfo.cgLeft) ? ajaInfo.cg : nsnull;
        adjacentBorder = CompareBorders(nsnull, cg, ajaInfo.leftCol, nsnull, nsnull, ajaInfo.cell, 
                                        tableIsLTR, !TABLE_EDGE, NS_SIDE_LEFT, !ADJACENT, t2p);
        currentBorder = CompareBorders(!CELL_CORNER, currentBorder, adjacentBorder, !HORIZONTAL);
                          
        segLength = PR_MAX(1, ajaInfo.rowIndex + ajaInfo.rowSpan - rowX);
        segLength = PR_MIN(segLength, info.rowIndex + info.rowSpan - rowX);

        // update lastVerBorders and see if a new segment starts
        startSeg = SetBorder(currentBorder, lastVerBorders[cellEndColIndex + 1]);
        // store the border segment in the cell map and update cellBorders
        if (RIGHT_DAMAGED(cellEndColIndex) && TOP_DAMAGED(rowX) && BOTTOM_DAMAGED(rowX)) {
          tableCellMap->SetBCBorderEdge(NS_SIDE_RIGHT, *info.cellMap, iter.mRowGroupStart, rowX, 
                                        cellEndColIndex, segLength, currentBorder.owner, currentBorder.width, startSeg);
          // update the borders of the cells and cols affected 
          if (info.cell) {
            info.cell->SetBorderWidth(secondSide, PR_MAX(currentBorder.width, info.cell->GetBorderWidth(secondSide)));
          }
          if (info.rightCol) {
            BCPixelSize half = BC_BORDER_LEFT_HALF(currentBorder.width);
            info.rightCol->SetRightBorderWidth(PR_MAX(half, info.rightCol->GetRightBorderWidth()));
          }
          if (ajaInfo.cell) {
            ajaInfo.cell->SetBorderWidth(firstSide, PR_MAX(currentBorder.width, ajaInfo.cell->GetBorderWidth(firstSide)));
          }
          if (ajaInfo.leftCol) {
            BCPixelSize half = BC_BORDER_RIGHT_HALF(currentBorder.width);
            ajaInfo.leftCol->SetLeftBorderWidth(PR_MAX(half, ajaInfo.leftCol->GetLeftBorderWidth()));
          }
        }
        // update the top right corner
        PRBool hitsSpanOnRight = (rowX > ajaInfo.rowIndex) && (rowX < ajaInfo.rowIndex + ajaInfo.rowSpan);
        BCCornerInfo* trCorner = ((0 == rowX) || hitsSpanOnRight) 
                                 ? &topCorners[cellEndColIndex + 1] : &bottomCorners[cellEndColIndex + 1]; 
        trCorner->Update(NS_SIDE_BOTTOM, currentBorder);
        // if this is not the first time through, consider the segment to the right
        if (rowX != info.rowIndex) {
          const nsIFrame* rg = (priorAjaInfo.rgBottom) ? priorAjaInfo.rg : nsnull;
          currentBorder = CompareBorders(nsnull, nsnull, nsnull, rg, priorAjaInfo.bottomRow, priorAjaInfo.cell,
                                         tableIsLTR, !TABLE_EDGE, NS_SIDE_BOTTOM, ADJACENT, t2p);
          rg = (ajaInfo.rgTop) ? ajaInfo.rg : nsnull;
          adjacentBorder = CompareBorders(nsnull, nsnull, nsnull, rg, ajaInfo.topRow, ajaInfo.cell,
                                          tableIsLTR, !TABLE_EDGE, NS_SIDE_TOP, !ADJACENT, t2p);
          currentBorder = CompareBorders(!CELL_CORNER, currentBorder, adjacentBorder, HORIZONTAL);
          trCorner->Update(NS_SIDE_RIGHT, currentBorder);
        }
        // store the top right corner in the cell map 
        if (RIGHT_DAMAGED(cellEndColIndex) && TOP_DAMAGED(rowX)) {
          if (0 != rowX) {
            tableCellMap->SetBCBorderCorner(eTopRight, *info.cellMap, iter.mRowGroupStart, rowX, cellEndColIndex, 
                                            trCorner->ownerSide, trCorner->subWidth, trCorner->bevel);
          }
          // store any corners this cell spans together with the aja cell
          for (PRInt32 rX = rowX + 1; rX < rowX + segLength; rX++) {
            tableCellMap->SetBCBorderCorner(eBottomRight, *info.cellMap, iter.mRowGroupStart, rX, 
                                            cellEndColIndex, trCorner->ownerSide, trCorner->subWidth, PR_FALSE);
          }
        }
        // update bottom right corner, topCorners, bottomCorners
        hitsSpanOnRight = (rowX + segLength < ajaInfo.rowIndex + ajaInfo.rowSpan);
        BCCornerInfo& brCorner = (hitsSpanOnRight) ? topCorners[cellEndColIndex + 1] 
                                                   : bottomCorners[cellEndColIndex + 1];
        brCorner.Set(NS_SIDE_TOP, currentBorder);
        priorAjaInfo = ajaInfo;
      }
    }
    for (PRInt32 colX = info.colIndex + 1; colX <= cellEndColIndex; colX++) {
      lastVerBorders[colX].Reset(0,1);
    }

    // find the dominant border considering the cell's bottom border, adjacent cells and the table, row group, row
    if (numRows == cellEndRowIndex + 1) { // touches bottom edge of table
      if (!tableBorderReset[NS_SIDE_BOTTOM]) {
        propData->mBottomBorderWidth = 0;
        tableBorderReset[NS_SIDE_BOTTOM] = PR_TRUE;
      }
      for (PRInt32 colX = info.colIndex; colX <= cellEndColIndex; colX++) {
        nsIFrame* colFrame = GetColFrame(colX); if (!colFrame) ABORT0();
        nsIFrame* cgFrame = colFrame->GetParent(); if (!cgFrame) ABORT0();
        currentBorder = CompareBorders(this, cgFrame, colFrame, info.rg, info.bottomRow, info.cell,
                                       tableIsLTR, TABLE_EDGE, NS_SIDE_BOTTOM, ADJACENT, t2p);
        // update/store the bottom left & bottom right corners 
        BCCornerInfo& blCorner = bottomCorners[colX]; // bottom left
        blCorner.Update(NS_SIDE_RIGHT, currentBorder);
        tableCellMap->SetBCBorderCorner(eBottomLeft, *info.cellMap, iter.mRowGroupStart, cellEndRowIndex,                
                                        colX, blCorner.ownerSide, blCorner.subWidth, blCorner.bevel); 
        BCCornerInfo& brCorner = bottomCorners[colX + 1]; // bottom right
        brCorner.Update(NS_SIDE_LEFT, currentBorder);
        if (numCols == colX + 1) { // lower right corner of the table
          tableCellMap->SetBCBorderCorner(eBottomRight, *info.cellMap, iter.mRowGroupStart, cellEndRowIndex,               
                                          colX, brCorner.ownerSide, brCorner.subWidth, brCorner.bevel, PR_TRUE);  
        }
        // update lastBottomBorder and see if a new segment starts
        startSeg = SetHorBorder(currentBorder, blCorner, lastBottomBorder);
        if (!startSeg) { 
           // make sure that we did not compare apples to oranges i.e. the current border 
           // should be a continuation of the lastBottomBorder, as it is a bottom border 
           // add 1 to the cellEndRowIndex
           startSeg = (lastBottomBorder.rowIndex != cellEndRowIndex + 1);
        }
        // store the border segment in the cell map and update cellBorders
        tableCellMap->SetBCBorderEdge(NS_SIDE_BOTTOM, *info.cellMap, iter.mRowGroupStart, cellEndRowIndex, 
                                      colX, 1, currentBorder.owner, currentBorder.width, startSeg);
        // update the bottom borders of the cell, the bottom row, and the table 
        if (info.cell) {
          info.cell->SetBorderWidth(NS_SIDE_BOTTOM, PR_MAX(currentBorder.width, info.cell->GetBorderWidth(NS_SIDE_BOTTOM)));
        }
        if (info.bottomRow) {
          BCPixelSize half = BC_BORDER_TOP_HALF(currentBorder.width);
          info.bottomRow->SetBottomBCBorderWidth(PR_MAX(half, info.bottomRow->GetBottomBCBorderWidth()));
        }
        propData->mBottomBorderWidth = LimitBorderWidth(PR_MAX(propData->mBottomBorderWidth, currentBorder.width));
        // update lastBottomBorders
        lastBottomBorder.rowIndex = cellEndRowIndex + 1;
        lastBottomBorder.rowSpan = info.rowSpan;
        lastBottomBorders[colX] = lastBottomBorder;
        //get col continuous border
        currentBorder = CompareBorders(this, cgFrame, colFrame, info.rg, info.bottomRow,
                                       nsnull, tableIsLTR, TABLE_EDGE, NS_SIDE_BOTTOM,
                                       ADJACENT, t2p);
        ((nsTableColFrame*)colFrame)->SetContinuousBCBorderWidth(NS_SIDE_BOTTOM,
                                                                currentBorder.width);
      }
      //get row group/col group continuous border
      if (info.rg) {
        currentBorder = CompareBorders(this, nsnull, nsnull, info.rg, info.bottomRow,
                                       nsnull, tableIsLTR, TABLE_EDGE, NS_SIDE_BOTTOM,
                                       ADJACENT, t2p);
        info.rg->SetContinuousBCBorderWidth(NS_SIDE_BOTTOM, currentBorder.width);
      }
      if (info.cg) {
        currentBorder = CompareBorders(this, info.cg, nsnull, info.rg, info.bottomRow,
                                       nsnull, tableIsLTR, TABLE_EDGE, NS_SIDE_BOTTOM,
                                       ADJACENT, t2p);
        info.cg->SetContinuousBCBorderWidth(NS_SIDE_BOTTOM, currentBorder.width);
      }
    }
    else {
      PRInt32 segLength = 0;
      for (PRInt32 colX = info.colIndex; colX <= cellEndColIndex; colX += segLength) {
        iter.PeekBottom(info, colX, ajaInfo);
        const nsIFrame* rg = (info.rgBottom) ? info.rg : nsnull;
        currentBorder = CompareBorders(nsnull, nsnull, nsnull, rg, info.bottomRow, info.cell, 
                                       tableIsLTR, !TABLE_EDGE, NS_SIDE_BOTTOM, ADJACENT, t2p);
        rg = (ajaInfo.rgTop) ? ajaInfo.rg : nsnull;
        adjacentBorder = CompareBorders(nsnull, nsnull, nsnull, rg, ajaInfo.topRow, ajaInfo.cell, 
                                        tableIsLTR, !TABLE_EDGE, NS_SIDE_TOP, !ADJACENT, t2p);
        currentBorder = CompareBorders(!CELL_CORNER, currentBorder, adjacentBorder, HORIZONTAL);
        segLength = PR_MAX(1, ajaInfo.colIndex + ajaInfo.colSpan - colX);
        segLength = PR_MIN(segLength, info.colIndex + info.colSpan - colX);

        // update, store the bottom left corner
        BCCornerInfo& blCorner = bottomCorners[colX]; // bottom left
        PRBool hitsSpanBelow = (colX > ajaInfo.colIndex) && (colX < ajaInfo.colIndex + ajaInfo.colSpan);
        PRBool update = PR_TRUE;
        if ((colX == info.colIndex) && (colX > damageArea.x)) {
          PRInt32 prevRowIndex = lastBottomBorders[colX - 1].rowIndex;
          if (prevRowIndex > cellEndRowIndex + 1) { // hits a rowspan on the right
            update = PR_FALSE; // the corner was taken care of during the cell on the left
          }
          else if (prevRowIndex < cellEndRowIndex + 1) { // spans below the cell to the left
            topCorners[colX] = blCorner;
            blCorner.Set(NS_SIDE_RIGHT, currentBorder);
            update = PR_FALSE;
          }
        }
        if (update) {
          blCorner.Update(NS_SIDE_RIGHT, currentBorder);
        }
        if (BOTTOM_DAMAGED(cellEndRowIndex) && LEFT_DAMAGED(colX)) {
          if (hitsSpanBelow) {
            tableCellMap->SetBCBorderCorner(eBottomLeft, *info.cellMap, iter.mRowGroupStart, cellEndRowIndex, colX,
                                            blCorner.ownerSide, blCorner.subWidth, blCorner.bevel);
          }
          // store any corners this cell spans together with the aja cell
          for (PRInt32 cX = colX + 1; cX < colX + segLength; cX++) {
            BCCornerInfo& corner = bottomCorners[cX];
            corner.Set(NS_SIDE_RIGHT, currentBorder);
            tableCellMap->SetBCBorderCorner(eBottomLeft, *info.cellMap, iter.mRowGroupStart, cellEndRowIndex,
                                            cX, corner.ownerSide, corner.subWidth, PR_FALSE);
          }
        }
        // update lastBottomBorders and see if a new segment starts
        startSeg = SetHorBorder(currentBorder, blCorner, lastBottomBorder);
        if (!startSeg) { 
           // make sure that we did not compare apples to oranges i.e. the current border 
           // should be a continuation of the lastBottomBorder, as it is a bottom border 
           // add 1 to the cellEndRowIndex
           startSeg = (lastBottomBorder.rowIndex != cellEndRowIndex + 1);
        }
        lastBottomBorder.rowIndex = cellEndRowIndex + 1;
        lastBottomBorder.rowSpan = info.rowSpan;
        for (PRInt32 cX = colX; cX < colX + segLength; cX++) {
          lastBottomBorders[cX] = lastBottomBorder;
        }

        // store the border segment the cell map and update cellBorders
        if (BOTTOM_DAMAGED(cellEndRowIndex) && LEFT_DAMAGED(colX) && RIGHT_DAMAGED(colX)) {
          tableCellMap->SetBCBorderEdge(NS_SIDE_BOTTOM, *info.cellMap, iter.mRowGroupStart, cellEndRowIndex,
                                        colX, segLength, currentBorder.owner, currentBorder.width, startSeg);
          // update the borders of the affected cells and rows
          if (info.cell) {
            info.cell->SetBorderWidth(NS_SIDE_BOTTOM, PR_MAX(currentBorder.width, info.cell->GetBorderWidth(NS_SIDE_BOTTOM)));
          }
          if (info.bottomRow) {
            BCPixelSize half = BC_BORDER_TOP_HALF(currentBorder.width);
            info.bottomRow->SetBottomBCBorderWidth(PR_MAX(half, info.bottomRow->GetBottomBCBorderWidth()));
          }
          if (ajaInfo.cell) {
            ajaInfo.cell->SetBorderWidth(NS_SIDE_TOP, PR_MAX(currentBorder.width, ajaInfo.cell->GetBorderWidth(NS_SIDE_TOP)));
          }
          if (ajaInfo.topRow) {
            BCPixelSize half = BC_BORDER_BOTTOM_HALF(currentBorder.width);
            ajaInfo.topRow->SetTopBCBorderWidth(PR_MAX(half, ajaInfo.topRow->GetTopBCBorderWidth()));
          }
        }
        // update bottom right corner
        BCCornerInfo& brCorner = bottomCorners[colX + segLength];
        brCorner.Update(NS_SIDE_LEFT, currentBorder);
      }
      if (!gotRowBorder && 1 == info.rowSpan && (ajaInfo.topRow || info.rgBottom)) {
        //get continuous row/row group border
        //we need to check the row group's bottom border if this is
        //the last row in the row group, but only a cell with rowspan=1
        //will know whether *this* row is at the bottom
        const nsIFrame* rg = (info.rgBottom) ? info.rg : nsnull;
        currentBorder = CompareBorders(nsnull, nsnull, nsnull, rg, info.bottomRow,
                                       nsnull, tableIsLTR, !TABLE_EDGE, NS_SIDE_BOTTOM,
                                       ADJACENT, t2p);
        rg = (ajaInfo.rgTop) ? ajaInfo.rg : nsnull;
        adjacentBorder = CompareBorders(nsnull, nsnull, nsnull, rg, ajaInfo.topRow,
                                        nsnull, tableIsLTR, !TABLE_EDGE, NS_SIDE_TOP,
                                        !ADJACENT, t2p);
        currentBorder = CompareBorders(PR_FALSE, currentBorder, adjacentBorder, HORIZONTAL);
        if (ajaInfo.topRow) {
          ajaInfo.topRow->SetContinuousBCBorderWidth(NS_SIDE_TOP, currentBorder.width);
        }
        if (info.rgBottom && info.rg) {
          info.rg->SetContinuousBCBorderWidth(NS_SIDE_BOTTOM, currentBorder.width);
        }
        gotRowBorder = PR_TRUE;
      }
    }

    // see if the cell to the right had a rowspan and its lower left border needs be joined with this one's bottom
    if ((numCols != cellEndColIndex + 1) &&                  // there is a cell to the right
        (lastBottomBorders[cellEndColIndex + 1].rowSpan > 1)) { // cell to right was a rowspan
      BCCornerInfo& corner = bottomCorners[cellEndColIndex + 1];
      if ((NS_SIDE_TOP != corner.ownerSide) && (NS_SIDE_BOTTOM != corner.ownerSide)) { // not a vertical owner
        BCCellBorder& thisBorder = lastBottomBorder;
        BCCellBorder& nextBorder = lastBottomBorders[info.colIndex + 1];
        if ((thisBorder.color == nextBorder.color) && (thisBorder.width == nextBorder.width) &&
            (thisBorder.style == nextBorder.style)) {
          // set the flag on the next border indicating it is not the start of a new segment
          if (iter.mCellMap) {
            BCData* bcData = tableCellMap->GetBCData(NS_SIDE_BOTTOM, *iter.mCellMap, cellEndRowIndex, 
                                                     cellEndColIndex + 1);
            if (bcData) {
              bcData->SetTopStart(PR_FALSE);
            }
          }
        }
      }
    }
  } // for (iter.First(info); info.cell; iter.Next(info)) {

  // reset the bc flag and damage area
  SetNeedToCalcBCBorders(PR_FALSE);
  propData->mDamageArea.x = propData->mDamageArea.y = propData->mDamageArea.width = propData->mDamageArea.height = 0;
#ifdef DEBUG_TABLE_CELLMAP
  mCellMap->Dump();
#endif
}

// Iterates over borders (left border, corner, top border) in the cell map within a damage area
// from left to right, top to bottom. All members are in terms of the 1st in flow frames, except 
// where suffixed by InFlow.  
class BCMapBorderIterator
{
public:
  BCMapBorderIterator(nsTableFrame&         aTableFrame,
                      nsTableRowGroupFrame& aRowGroupFrame,
                      nsTableRowFrame&      aRowFrame,
                      const nsRect&         aDamageArea);
  void Reset(nsTableFrame&         aTableFrame,
             nsTableRowGroupFrame& aRowGroupFrame,
             nsTableRowFrame&      aRowFrame,
             const nsRect&         aDamageArea);
  void First();
  void Next();

  nsTableFrame*         table;
  nsTableCellMap*       tableCellMap;
  nsCellMap*            cellMap;

  nsVoidArray           rowGroups;
  nsTableRowGroupFrame* prevRg;
  nsTableRowGroupFrame* rg;
  PRInt32               rowGroupIndex;
  PRInt32               fifRowGroupStart;
  PRInt32               rowGroupStart;
  PRInt32               rowGroupEnd;
  PRInt32               numRows; // number of rows in the table and all continuations

  nsTableRowFrame*      prevRow;
  nsTableRowFrame*      row;
  PRInt32               numCols;
  PRInt32               x;
  PRInt32               y;

  nsTableCellFrame*     prevCell;
  nsTableCellFrame*     cell;  
  BCCellData*           prevCellData;
  BCCellData*           cellData;
  BCData*               bcData;

  PRBool                IsTopMostTable()    { return (y == 0) && !table->GetPrevInFlow(); }
  PRBool                IsRightMostTable()  { return (x >= numCols); }
  PRBool                IsBottomMostTable() { return (y >= numRows) && !table->GetNextInFlow(); }
  PRBool                IsLeftMostTable()   { return (x == 0); }
  PRBool                IsTopMost()    { return (y == startY); }
  PRBool                IsRightMost()  { return (x >= endX); }
  PRBool                IsBottomMost() { return (y >= endY); }
  PRBool                IsLeftMost()   { return (x == startX); }
  PRBool                isNewRow;

  PRInt32               startX;
  PRInt32               startY;
  PRInt32               endX;
  PRInt32               endY;
  PRBool                isRepeatedHeader;
  PRBool                isRepeatedFooter;
  PRBool                atEnd;

private:

  PRBool SetNewRow(nsTableRowFrame* aRow = nsnull);
  PRBool SetNewRowGroup();
  void   SetNewData(PRInt32 aY, PRInt32 aX);

};

BCMapBorderIterator::BCMapBorderIterator(nsTableFrame&         aTable,
                                         nsTableRowGroupFrame& aRowGroup,
                                         nsTableRowFrame&      aRow,
                                         const nsRect&         aDamageArea)
{
  Reset(aTable, aRowGroup, aRow, aDamageArea);
}

void
BCMapBorderIterator::Reset(nsTableFrame&         aTable,
                           nsTableRowGroupFrame& aRowGroup,
                           nsTableRowFrame&      aRow,
                           const nsRect&         aDamageArea)
{
  atEnd = PR_TRUE; // gets reset when First() is called

  table   = &aTable;
  rg      = &aRowGroup; 
  prevRow = nsnull;
  row     = &aRow;                     

  nsTableFrame* tableFif = (nsTableFrame*)table->GetFirstInFlow(); if (!tableFif) ABORT0();
  tableCellMap = tableFif->GetCellMap();

  startX   = aDamageArea.x;
  startY   = aDamageArea.y;
  endY     = aDamageArea.y + aDamageArea.height;
  endX     = aDamageArea.x + aDamageArea.width;

  numRows       = tableFif->GetRowCount();
  y             = 0;
  numCols       = tableFif->GetColCount();
  x             = 0;
  rowGroupIndex = -1;
  prevCell      = nsnull;
  cell          = nsnull;
  prevCellData  = nsnull;
  cellData      = nsnull;
  bcData        = nsnull;

  // Get the ordered row groups 
  PRUint32 numRowGroups;
  table->OrderRowGroups(rowGroups, numRowGroups, nsnull);
}

void 
BCMapBorderIterator::SetNewData(PRInt32 aY,
                                PRInt32 aX)
{
  if (!tableCellMap || !tableCellMap->mBCInfo) ABORT0();

  x            = aX;
  y            = aY;
  prevCellData = cellData;
  if (IsRightMost() && IsBottomMost()) {
    cell = nsnull;
    bcData = &tableCellMap->mBCInfo->mLowerRightCorner;
  }
  else if (IsRightMost()) {
    cellData = nsnull;
    bcData = (BCData*)tableCellMap->mBCInfo->mRightBorders.ElementAt(aY);
  }
  else if (IsBottomMost()) {
    cellData = nsnull;
    bcData = (BCData*)tableCellMap->mBCInfo->mBottomBorders.ElementAt(aX);
  }
  else {
    bcData = nsnull;
    nsVoidArray* row = (nsVoidArray*)cellMap->mRows.ElementAt(y - fifRowGroupStart);
    if (row) {
      cellData = (row->Count() > x) ? (BCCellData*)row->ElementAt(x) : nsnull;
      if (cellData) {
        bcData = &cellData->mData;
        if (!cellData->IsOrig()) {
          if (cellData->IsRowSpan()) {
            aY -= cellData->GetRowSpanOffset();
          }
          if (cellData->IsColSpan()) {
            aX -= cellData->GetColSpanOffset();
          }
          if ((aX >= 0) && (aY >= 0)) {
            row = (nsVoidArray*)cellMap->mRows.ElementAt(aY - fifRowGroupStart);
            if (row) {
              cellData = (BCCellData*)row->ElementAt(aX);
            }
          }
        }
        if (cellData->IsOrig()) {
          prevCell = cell;
          cell = cellData->GetCellFrame();
        }
      }
    }
  }
}

PRBool
BCMapBorderIterator::SetNewRow(nsTableRowFrame* aRow)
{
  prevRow = row;
  row      = (aRow) ? aRow : row->GetNextRow();
 
  if (row) {
    isNewRow = PR_TRUE;
    y = row->GetRowIndex();
    x = startX;
  }
  else {
    atEnd = PR_TRUE;
  }
  return !atEnd;
}


PRBool
BCMapBorderIterator::SetNewRowGroup()
{
  rowGroupIndex++;

  isRepeatedHeader = PR_FALSE;
  isRepeatedFooter = PR_FALSE;

  if (rowGroupIndex < rowGroups.Count()) {
    prevRg = rg;
    nsIFrame* frame = (nsTableRowGroupFrame*)rowGroups.ElementAt(rowGroupIndex); if (!frame) ABORT1(PR_FALSE);
    rg = table->GetRowGroupFrame(frame); if (!rg) ABORT1(PR_FALSE);
    fifRowGroupStart = ((nsTableRowGroupFrame*)rg->GetFirstInFlow())->GetStartRowIndex();
    rowGroupStart    = rg->GetStartRowIndex(); 
    rowGroupEnd      = rowGroupStart + rg->GetRowCount() - 1;

    if (SetNewRow(rg->GetFirstRow())) {
      cellMap = tableCellMap->GetMapFor(*(nsTableRowGroupFrame*)rg->GetFirstInFlow()); if (!cellMap) ABORT1(PR_FALSE);
    }
    if (rg && table->GetPrevInFlow() && !rg->GetPrevInFlow()) {
      // if rg doesn't have a prev in flow, then it may be a repeated header or footer
      const nsStyleDisplay* display = rg->GetStyleDisplay();
      if (y == startY) {
        isRepeatedHeader = (NS_STYLE_DISPLAY_TABLE_HEADER_GROUP == display->mDisplay);
      }
      else {
        isRepeatedFooter = (NS_STYLE_DISPLAY_TABLE_FOOTER_GROUP == display->mDisplay);
      }
    }
  }
  else {
    atEnd = PR_TRUE;
  }
  return !atEnd;
}

void 
BCMapBorderIterator::First()
{
  if (!table || (startX >= numCols) || (startY >= numRows)) ABORT0();

  atEnd = PR_FALSE;

  PRUint32 numRowGroups = rowGroups.Count();
  for (PRUint32 rgX = 0; rgX < numRowGroups; rgX++) { 
    nsIFrame* frame = (nsIFrame*)rowGroups.ElementAt(rgX);
    nsTableRowGroupFrame* rowG = table->GetRowGroupFrame(frame);
    if (rowG) {
      PRInt32 start = rowG->GetStartRowIndex();
      PRInt32 end   = start + rowG->GetRowCount() - 1;
      if ((startY >= start) && (startY <= end)) {
        rowGroupIndex = rgX - 1; // SetNewRowGroup increments rowGroupIndex
        if (SetNewRowGroup()) { 
          while ((y < startY) && !atEnd) {
            SetNewRow();
          }
          if (!atEnd) {
            SetNewData(startY, startX);
          }
        }
        return;
      }
    }
  }
  atEnd = PR_TRUE;
}

void 
BCMapBorderIterator::Next()
{
  if (atEnd) ABORT0();
  isNewRow = PR_FALSE;

  x++;
  if (x > endX) {
    y++;
    if (y == endY) {
      x = startX;
    }
    else if (y < endY) {
      if (y <= rowGroupEnd) {
        SetNewRow();
      }
      else {
        SetNewRowGroup();
      }
    }
    else {
      atEnd = PR_TRUE;
    }
  }
  if (!atEnd) {
    SetNewData(y, x);
  }
}

// XXX if CalcVerCornerOffset and CalcHorCornerOffset remain similar, combine them
static nscoord
CalcVerCornerOffset(PRUint8 aCornerOwnerSide,
                    nscoord aCornerSubWidth,
                    nscoord aHorWidth,
                    PRBool  aIsStartOfSeg,
                    PRBool  aIsBevel,
                    float   aPixelsToTwips)
{
  nscoord offset = 0;
  // XXX These should be replaced with appropriate side-specific macros (which?).
  nscoord smallHalf, largeHalf;
  if ((NS_SIDE_TOP == aCornerOwnerSide) || (NS_SIDE_BOTTOM == aCornerOwnerSide)) {
    DivideBCBorderSize(aCornerSubWidth, smallHalf, largeHalf);
    if (aIsBevel) {
      offset = (aIsStartOfSeg) ? -largeHalf : smallHalf;
    }
    else {
      offset = (NS_SIDE_TOP == aCornerOwnerSide) ? smallHalf : -largeHalf;
    }
  }
  else {
    DivideBCBorderSize(aHorWidth, smallHalf, largeHalf);
    if (aIsBevel) {
      offset = (aIsStartOfSeg) ? -largeHalf : smallHalf;
    }
    else {
      offset = (aIsStartOfSeg) ? smallHalf : -largeHalf;
    }
  }
  return NSToCoordRound(aPixelsToTwips * (float)offset);
}

/** Compute the horizontal offset of a horizontal border segment
  * @param aCornerOwnerSide - which side owns the corner
  * @param aCornerSubWidth  - how wide is the nonwinning side of the corner
  * @param aVerWidth        - how wide is the vertical edge of the corner
  * @param aIsStartOfSeg    - does this corner start a new segment
  * @param aIsBevel         - is this corner beveled
  * @param aPixelsToTwips   - conversion factor
  * @param aTableIsLTR      - direction, the computation depends on ltr or rtl
  * @return                 - offset in pixel
  */
static nscoord
CalcHorCornerOffset(PRUint8 aCornerOwnerSide,
                    nscoord aCornerSubWidth,
                    nscoord aVerWidth,
                    PRBool  aIsStartOfSeg,
                    PRBool  aIsBevel,
                    float   aPixelsToTwips,
                    PRBool  aTableIsLTR)
{
  nscoord offset = 0;
  // XXX These should be replaced with appropriate side-specific macros (which?).
  nscoord smallHalf, largeHalf;
  if ((NS_SIDE_LEFT == aCornerOwnerSide) || (NS_SIDE_RIGHT == aCornerOwnerSide)) {
    if (aTableIsLTR) {
      DivideBCBorderSize(aCornerSubWidth, smallHalf, largeHalf);
    }
    else {
      DivideBCBorderSize(aCornerSubWidth, largeHalf, smallHalf);
    }
    if (aIsBevel) {
      offset = (aIsStartOfSeg) ? -largeHalf : smallHalf;
    }
    else {
      offset = (NS_SIDE_LEFT == aCornerOwnerSide) ? smallHalf : -largeHalf;
    }
  }
  else {
    if (aTableIsLTR) {
      DivideBCBorderSize(aVerWidth, smallHalf, largeHalf);
    }
    else {
      DivideBCBorderSize(aVerWidth, largeHalf, smallHalf);
    }
    if (aIsBevel) {
      offset = (aIsStartOfSeg) ? -largeHalf : smallHalf;
    }
    else {
      offset = (aIsStartOfSeg) ? smallHalf : -largeHalf;
    }
  }
  return NSToCoordRound(aPixelsToTwips * (float)offset);
}

struct BCVerticalSeg
{
  BCVerticalSeg();
 
  void Start(BCMapBorderIterator& aIter,
             BCBorderOwner        aBorderOwner,
             nscoord              aVerSegWidth,
             nscoord              aPrevHorSegHeight,
             nscoord              aHorSegHeight,
             float                aPixelsToTwips,
             BCVerticalSeg*       aVerInfoArray);
  
  union {
    nsTableColFrame*  col;
    PRInt32           colWidth;
  };
  PRInt32               colX;
  nsTableCellFrame*     ajaCell;
  nsTableCellFrame*     firstCell;  // cell at the start of the segment
  nsTableRowGroupFrame* firstRowGroup; // row group at the start of the segment
  nsTableRowFrame*      firstRow; // row at the start of the segment
  nsTableCellFrame*     lastCell;   // cell at the current end of the segment
  PRInt32               segY;
  PRInt32               segHeight;
  PRInt16               segWidth;   // width in pixels
  PRUint8               owner;
  PRUint8               bevelSide;
  PRUint16              bevelOffset;
};

BCVerticalSeg::BCVerticalSeg() 
{ 
  col = nsnull; firstCell = lastCell = ajaCell = nsnull; colX = segY = segHeight = 0;
  segWidth = bevelOffset = 0; bevelSide = 0; owner = eCellOwner; 
}
 
void
BCVerticalSeg::Start(BCMapBorderIterator& aIter,
                     BCBorderOwner        aBorderOwner,
                     nscoord              aVerSegWidth,
                     nscoord              aPrevHorSegHeight,
                     nscoord              aHorSegHeight,
                     float                aPixelsToTwips,
                     BCVerticalSeg*       aVerInfoArray)
{
  PRUint8      ownerSide = 0;
  PRPackedBool bevel     = PR_FALSE;
  PRInt32      xAdj      = aIter.x - aIter.startX;

  nscoord cornerSubWidth  = (aIter.bcData) ? aIter.bcData->GetCorner(ownerSide, bevel) : 0;
  PRBool  topBevel        = (aVerSegWidth > 0) ? bevel : PR_FALSE;
  nscoord maxHorSegHeight = PR_MAX(aPrevHorSegHeight, aHorSegHeight);
  nscoord offset          = CalcVerCornerOffset(ownerSide, cornerSubWidth, maxHorSegHeight, 
                                                PR_TRUE, topBevel, aPixelsToTwips);

  bevelOffset   = (topBevel) ? maxHorSegHeight : 0;
  bevelSide     = (aHorSegHeight > 0) ? NS_SIDE_RIGHT : NS_SIDE_LEFT;
  segY         += offset;
  segHeight     = -offset;
  segWidth      = aVerSegWidth;
  owner         = aBorderOwner;
  firstCell     = aIter.cell;
  firstRowGroup = aIter.rg;
  firstRow      = aIter.row;
  if (xAdj > 0) {
    ajaCell = aVerInfoArray[xAdj - 1].lastCell;
  }
}

struct BCHorizontalSeg
{
  BCHorizontalSeg();

  void Start(BCMapBorderIterator& aIter,
             BCBorderOwner        aBorderOwner,
             PRUint8              aCornerOwnerSide,
             nscoord              aSubWidth,
             PRBool               aBevel,
             nscoord              aTopVerSegWidth,
             nscoord              aBottomVerSegWidth,
             nscoord              aHorSegHeight,
             nsTableCellFrame*    aLastCell,
             float                aPixelsToTwips,
             PRBool               aTableIsLTR);
  
  nscoord            x;
  nscoord            y;
  nscoord            width;
  nscoord            height;
  PRBool             leftBevel;
  nscoord            leftBevelOffset;
  PRUint8            leftBevelSide;
  PRUint8            owner;
  nsTableCellFrame*  firstCell; // cell at the start of the segment
  nsTableCellFrame*  ajaCell;
};

BCHorizontalSeg::BCHorizontalSeg() 
{ 
  x = y = width = height = leftBevel = leftBevelOffset = leftBevelSide = 0; 
  firstCell = ajaCell = nsnull;
}
  
/** Initialize a horizontal border segment for painting
  * @param aIter              - iterator storing the current and adjacent frames
  * @param aBorderOwner       - which frame owns the border
  * @param aCornerOwnerSide   - which side owns the starting corner
  * @param aSubWidth          - how wide is the nonowning width of the corner
  * @param aBevel             - is the corner beveled
  * @param aTopVerSegWidth    - vertical segment width going down
  * @param aBottomVerSegWidth - vertical segment width coming from up
  * @param aHorSegHeight      - the height of the segment
  * @param aLastCell          - cell frame above this segment
  * @param aPixelsToTwips     - conversion factor
  * @param aTableIsLTR        - direction, the computation depends on ltr or rtl
  */
void
BCHorizontalSeg::Start(BCMapBorderIterator& aIter,
                       BCBorderOwner        aBorderOwner,
                       PRUint8              aCornerOwnerSide,
                       nscoord              aSubWidth,
                       PRBool               aBevel,
                       nscoord              aTopVerSegWidth,
                       nscoord              aBottomVerSegWidth,
                       nscoord              aHorSegHeight,
                       nsTableCellFrame*    aLastCell,
                       float                aPixelsToTwips,
                       PRBool               aTableIsLTR)
{
  owner = aBorderOwner;
  leftBevel = (aHorSegHeight > 0) ? aBevel : PR_FALSE;
  nscoord maxVerSegWidth = PR_MAX(aTopVerSegWidth, aBottomVerSegWidth);
  nscoord offset = CalcHorCornerOffset(aCornerOwnerSide, aSubWidth, maxVerSegWidth, 
                                       PR_TRUE, leftBevel, aPixelsToTwips, aTableIsLTR);
  leftBevelOffset = (leftBevel && (aHorSegHeight > 0)) ? maxVerSegWidth : 0;
  leftBevelSide   = (aBottomVerSegWidth > 0) ? NS_SIDE_BOTTOM : NS_SIDE_TOP;
  if (aTableIsLTR) {
    x            += offset;
  }
  else {
    x            -= offset;
  }
  width           = -offset;
  height          = aHorSegHeight;
  firstCell       = aIter.cell;
  ajaCell         = (aIter.IsTopMost()) ? nsnull : aLastCell; 
}

void 
nsTableFrame::PaintBCBorders(nsIRenderingContext& aRenderingContext,
                             const nsRect&        aDirtyRect)
{
  nsMargin childAreaOffset = GetChildAreaOffset(nsnull);
  nsTableFrame* firstInFlow = (nsTableFrame*)GetFirstInFlow(); if (!firstInFlow) ABORT0();
  GET_PIXELS_TO_TWIPS(GetPresContext(), p2t);

  PRInt32 startRowY = (GetPrevInFlow()) ? 0 : childAreaOffset.top; // y position of first row in damage area

  const nsStyleBackground* bgColor = nsCSSRendering::FindNonTransparentBackground(mStyleContext);
  // determine the damage area in terms of rows and columns and finalize startColX and startRowY
  PRUint32 startRowIndex, endRowIndex, startColIndex, endColIndex;
  startRowIndex = endRowIndex = startColIndex = endColIndex = 0;

  nsAutoVoidArray rowGroups;
  PRUint32 numRowGroups;
  OrderRowGroups(rowGroups, numRowGroups, nsnull);
  PRBool done = PR_FALSE;
  PRBool haveIntersect = PR_FALSE;
  nsTableRowGroupFrame* inFlowRG  = nsnull;
  nsTableRowFrame*      inFlowRow = nsnull;
  // find startRowIndex, endRowIndex, startRowY
  nscoord onePixel = NSToCoordRound(p2t);
  PRInt32 rowY = startRowY;
  for (PRUint32 rgX = 0; (rgX < numRowGroups) && !done; rgX++) {
    nsIFrame* kidFrame = (nsIFrame*)rowGroups.ElementAt(rgX);
    nsTableRowGroupFrame* rgFrame = GetRowGroupFrame(kidFrame); if (!rgFrame) ABORT0();
    for (nsTableRowFrame* rowFrame = rgFrame->GetFirstRow(); rowFrame; rowFrame = rowFrame->GetNextRow()) {
      // conservatively estimate the half border widths outside the row
      nscoord topBorderHalf    = (GetPrevInFlow()) ? 0 : rowFrame->GetTopBCBorderWidth(&p2t) + onePixel; 
      nscoord bottomBorderHalf = (GetNextInFlow()) ? 0 : rowFrame->GetBottomBCBorderWidth(&p2t) + onePixel;
      // get the row rect relative to the table rather than the row group
      nsSize rowSize = rowFrame->GetSize();
      if (haveIntersect) {
        if (aDirtyRect.YMost() >= (rowY - topBorderHalf)) {
          nsTableRowFrame* fifRow = (nsTableRowFrame*)rowFrame->GetFirstInFlow(); if (!fifRow) ABORT0();
          endRowIndex = fifRow->GetRowIndex();
        }
        else done = PR_TRUE;
      }
      else {
        if ((rowY + rowSize.height + bottomBorderHalf) >= aDirtyRect.y) {
          inFlowRG  = rgFrame;
          inFlowRow = rowFrame;
          nsTableRowFrame* fifRow = (nsTableRowFrame*)rowFrame->GetFirstInFlow(); if (!fifRow) ABORT0();
          startRowIndex = endRowIndex = fifRow->GetRowIndex();
          haveIntersect = PR_TRUE;
        }
        else {
          startRowY += rowSize.height;
        }
      }
      rowY += rowSize.height; 
    }
  }
  // outer table borders overflow the table, so the table might be
  // target to other areas as the NS_FRAME_OUTSIDE_CHILDREN is set
  // on the table
  if (!haveIntersect)
    return;  
  if (!inFlowRG || !inFlowRow) ABORT0();

  PRInt32 startColX;
  // find startColIndex, endColIndex, startColX
  haveIntersect = PR_FALSE;
  PRUint32 numCols = GetColCount();
  if (0 == numCols) return;

  PRInt32 leftCol, rightCol, colInc; // columns are in the range [leftCol, rightCol)
  PRBool tableIsLTR = GetStyleVisibility()->mDirection == NS_STYLE_DIRECTION_LTR;
  if (tableIsLTR) {
    startColX = childAreaOffset.left; // x position of first col in damage area
    leftCol = 0;
    rightCol = numCols;
    colInc = 1;
  } else {
    startColX = mRect.width - childAreaOffset.right; // x position of first col in damage area
    leftCol = numCols-1;
    rightCol = -1;
    colInc = -1;
  }

  nscoord x = 0;
  PRInt32 colX;
  for (colX = leftCol; colX != rightCol; colX += colInc) {
    nsTableColFrame* colFrame = firstInFlow->GetColFrame(colX);
    if (!colFrame) ABORT0();
    // conservatively estimate the half border widths outside the col
    nscoord leftBorderHalf    = colFrame->GetLeftBorderWidth(&p2t) + onePixel; 
    nscoord rightBorderHalf   = colFrame->GetRightBorderWidth(&p2t) + onePixel;
    // get the col rect relative to the table rather than the col group
    nsSize size = colFrame->GetSize();
    if (haveIntersect) {
      if (aDirtyRect.XMost() >= (x - leftBorderHalf)) {
        endColIndex = colX;
      }
      else break;
    }
    else {
      if ((x + size.width + rightBorderHalf) >= aDirtyRect.x) {
        startColIndex = endColIndex = colX;
        haveIntersect = PR_TRUE;
      }
      else {
        startColX += colInc * size.width;
      }
    }
    x += size.width;
  }

  if (!tableIsLTR) {
    PRUint32 temp;
    startColX = mRect.width - childAreaOffset.right;
    temp = startColIndex; startColIndex = endColIndex; endColIndex = temp;
    for (PRUint32 column = 0; column < startColIndex; column++) {
      nsTableColFrame* colFrame = firstInFlow->GetColFrame(column);
      if (!colFrame) ABORT0();
      nsSize size = colFrame->GetSize();
      startColX += colInc * size.width;
    }
  }
  if (!haveIntersect)
    return;
  // iterate the cell map and build up border segments
  nsRect damageArea(startColIndex, startRowIndex,
                    1 + PR_ABS(PRInt32(endColIndex - startColIndex)), 
                    1 + endRowIndex - startRowIndex);
  BCVerticalSeg* verInfo = new BCVerticalSeg[damageArea.width + 1]; if (!verInfo) ABORT0();

  BCBorderOwner borderOwner, ignoreBorderOwner;
  PRUint8 ownerSide;
  nscoord cornerSubWidth, smallHalf, largeHalf;
  nsRect rowRect(0,0,0,0);
  PRBool isSegStart, ignoreSegStart;
  nscoord prevHorSegHeight = 0;
  PRPackedBool bevel;
  PRInt32 repeatedHeaderY = -99;
  PRBool  afterRepeatedHeader = PR_FALSE;
  PRBool  startRepeatedFooter = PR_FALSE;

  // First, paint all of the vertical borders from top to bottom and left to right as they become complete
  // They are painted first, since they are less efficient to paint than horizontal segments. They were 
  // stored with as few segments as possible (since horizontal borders are painted last and possibly over them).
  BCMapBorderIterator iter(*this, *inFlowRG, *inFlowRow, damageArea); 
  for (iter.First(); !iter.atEnd; iter.Next()) {
    nscoord verSegWidth = (iter.bcData) ? iter.bcData->GetLeftEdge(borderOwner, isSegStart) : 0;
    nscoord horSegHeight = (iter.bcData) ? iter.bcData->GetTopEdge(ignoreBorderOwner, ignoreSegStart) : 0;

    PRInt32 xAdj = iter.x - iter.startX;
    if (iter.isNewRow) {
      prevHorSegHeight = 0;
      rowRect = iter.row->GetRect();
      if (iter.isRepeatedHeader) {
        repeatedHeaderY = iter.y;
      }
      afterRepeatedHeader = !iter.isRepeatedHeader && (iter.y == (repeatedHeaderY + 1));
      startRepeatedFooter = iter.isRepeatedFooter && (iter.y == iter.rowGroupStart) && (iter.y != iter.startY);
    }
    BCVerticalSeg& info = verInfo[xAdj];
    if (!info.col) { // on the first damaged row and the first segment in the col
      info.col = iter.IsRightMostTable() ? verInfo[xAdj - 1].col : firstInFlow->GetColFrame(iter.x);
      if (!info.col) ABORT0();
      if (0 == xAdj) {
        info.colX = startColX;
      }
      // set colX for the next column
      if (!iter.IsRightMost()) {
        verInfo[xAdj + 1].colX = info.colX + colInc * info.col->GetSize().width;
      }
      info.segY = startRowY; 
      info.Start(iter, borderOwner, verSegWidth, prevHorSegHeight, horSegHeight, p2t, verInfo);
      info.lastCell = iter.cell;
    }

    if (!iter.IsTopMost() && (isSegStart || iter.IsBottomMost() || afterRepeatedHeader || startRepeatedFooter)) {
      // paint the previous seg or the current one if iter.IsBottomMost()
      if (info.segHeight > 0) {
        if (iter.bcData) {
          cornerSubWidth = iter.bcData->GetCorner(ownerSide, bevel);
        } else {
          cornerSubWidth = 0;
          ownerSide = 0; // ???
          bevel = PR_FALSE; // ???
        }
        PRBool endBevel = (info.segWidth > 0) ? bevel : PR_FALSE; 
        nscoord bottomHorSegHeight = PR_MAX(prevHorSegHeight, horSegHeight); 
        nscoord endOffset = CalcVerCornerOffset(ownerSide, cornerSubWidth, bottomHorSegHeight, 
                                                PR_FALSE, endBevel, p2t);
        info.segHeight += endOffset;
        if (info.segWidth > 0) {     
          // get the border style, color and paint the segment
          PRUint8 side = (iter.IsRightMost()) ? NS_SIDE_RIGHT : NS_SIDE_LEFT;
          nsTableRowFrame* row           = info.firstRow;
          nsTableRowGroupFrame* rowGroup = info.firstRowGroup;
          nsTableColFrame* col           = info.col; if (!col) ABORT0();
          nsTableCellFrame* cell         = info.firstCell; 
          PRUint8 style = NS_STYLE_BORDER_STYLE_SOLID;
          nscolor color = 0xFFFFFFFF;
          PRBool ignoreIfRules = (iter.IsRightMostTable() || iter.IsLeftMostTable());

          switch (info.owner) {
          case eTableOwner:
            ::GetPaintStyleInfo(this, side, style, color, tableIsLTR, PR_FALSE);
            break;
          case eAjaColGroupOwner: 
            side = NS_SIDE_RIGHT;
            if (!iter.IsRightMostTable() && (xAdj > 0)) {
              col = verInfo[xAdj - 1].col; 
            } // and fall through
          case eColGroupOwner:
            if (col) {
              nsIFrame* cg = col->GetParent();
              if (cg) {
                ::GetPaintStyleInfo(cg, side, style, color, tableIsLTR, ignoreIfRules);
              }
            }
            break;
          case eAjaColOwner: 
            side = NS_SIDE_RIGHT;
            if (!iter.IsRightMostTable() && (xAdj > 0)) {
              col = verInfo[xAdj - 1].col; 
            } // and fall through
          case eColOwner:
            if (col) {
              ::GetPaintStyleInfo(col, side, style, color, tableIsLTR, ignoreIfRules);
            }
            break;
          case eAjaRowGroupOwner:
            NS_ASSERTION(PR_FALSE, "program error"); // and fall through
          case eRowGroupOwner:
            NS_ASSERTION(iter.IsLeftMostTable() || iter.IsRightMostTable(), "program error");
            if (rowGroup) {
              ::GetPaintStyleInfo(rowGroup, side, style, color, tableIsLTR, ignoreIfRules);
            }
            break;
          case eAjaRowOwner:
            NS_ASSERTION(PR_FALSE, "program error"); // and fall through
          case eRowOwner: 
            NS_ASSERTION(iter.IsLeftMostTable() || iter.IsRightMostTable(), "program error");
            if (row) {
              ::GetPaintStyleInfo(row, side, style, color, tableIsLTR, ignoreIfRules);
            }
            break;
          case eAjaCellOwner:
            side = NS_SIDE_RIGHT;
            cell = info.ajaCell; // and fall through
          case eCellOwner:
            if (cell) {
              ::GetPaintStyleInfo(cell, side, style, color, tableIsLTR, PR_FALSE);
            }
            break;
          }
          DivideBCBorderSize(info.segWidth, smallHalf, largeHalf);
          nsRect segRect(info.colX - NSToCoordRound(p2t * (float)largeHalf), info.segY, 
                         NSToCoordRound(p2t * (float)info.segWidth), info.segHeight);
          nscoord bottomBevelOffset = (endBevel) ? NSToCoordRound(p2t * (float)bottomHorSegHeight) : 0;
          PRUint8 bottomBevelSide = ((horSegHeight > 0) ^ !tableIsLTR) ? NS_SIDE_RIGHT : NS_SIDE_LEFT;
          PRUint8 topBevelSide = ((info.bevelSide == NS_SIDE_RIGHT) ^ !tableIsLTR)?  NS_SIDE_RIGHT : NS_SIDE_LEFT;
          nsCSSRendering::DrawTableBorderSegment(aRenderingContext, style, color, bgColor, segRect, p2t, 
                                                 topBevelSide, NSToCoordRound(p2t * (float)info.bevelOffset), 
                                                 bottomBevelSide, bottomBevelOffset);
        } // if (info.segWidth > 0) 
        info.segY = info.segY + info.segHeight - endOffset;
      } // if (info.segHeight > 0)
      info.Start(iter, borderOwner, verSegWidth, prevHorSegHeight, horSegHeight, p2t, verInfo);
    } // if (!iter.IsTopMost() && (isSegStart || iter.IsBottomMost()))

    info.lastCell   = iter.cell;
    info.segHeight += rowRect.height;
    prevHorSegHeight = horSegHeight;
  } // for (iter.First(); !iter.atEnd; iter.Next())

  // Next, paint all of the horizontal border segments from top to bottom reuse the verInfo 
  // array to keep tract of col widths and vertical segments for corner calculations
  memset(verInfo, 0, damageArea.width * sizeof(BCVerticalSeg)); // XXX reinitialize properly
  for (PRInt32 xIndex = 0; xIndex < damageArea.width; xIndex++) {
    verInfo[xIndex].colWidth = -1;
  }
  PRInt32 nextY = startRowY;
  BCHorizontalSeg horSeg;

  iter.Reset(*this, *inFlowRG, *inFlowRow, damageArea);
  for (iter.First(); !iter.atEnd; iter.Next()) {
    nscoord leftSegWidth = (iter.bcData) ? iter.bcData->GetLeftEdge(ignoreBorderOwner, ignoreSegStart) : 0;
    nscoord topSegHeight = (iter.bcData) ? iter.bcData->GetTopEdge(borderOwner, isSegStart) : 0;

    PRInt32 xAdj = iter.x - iter.startX;
    // store the current col width if it hasn't been already
    if (verInfo[xAdj].colWidth < 0) {
      if (iter.IsRightMostTable()) {
        verInfo[xAdj].colWidth = verInfo[xAdj - 1].colWidth;
      }
      else {
        nsTableColFrame* col = firstInFlow->GetColFrame(iter.x); if (!col) ABORT0();
        verInfo[xAdj].colWidth = col->GetSize().width;
      }
    }
    cornerSubWidth = (iter.bcData) ? iter.bcData->GetCorner(ownerSide, bevel) : 0;
    nscoord verWidth = PR_MAX(verInfo[xAdj].segWidth, leftSegWidth);
    if (iter.isNewRow || (iter.IsLeftMost() && iter.IsBottomMost())) {
      horSeg.y = nextY;
      nextY    = nextY + iter.row->GetSize().height;
      horSeg.x = startColX;
      horSeg.Start(iter, borderOwner, ownerSide, cornerSubWidth, bevel, verInfo[xAdj].segWidth, 
                   leftSegWidth, topSegHeight, verInfo[xAdj].lastCell, p2t, tableIsLTR);
    }
    PRBool verOwnsCorner = (NS_SIDE_TOP == ownerSide) || (NS_SIDE_BOTTOM == ownerSide);
    if (!iter.IsLeftMost() && (isSegStart || iter.IsRightMost() || verOwnsCorner)) {
      // paint the previous seg or the current one if iter.IsRightMost()
      if (horSeg.width > 0) {
        PRBool endBevel = (horSeg.height > 0) ? bevel : 0;
        nscoord endOffset = CalcHorCornerOffset(ownerSide, cornerSubWidth, verWidth, PR_FALSE, endBevel, p2t, tableIsLTR);
        horSeg.width += endOffset;
        if (horSeg.height > 0) {
          // get the border style, color and paint the segment
          PRUint8 side = (iter.IsBottomMost()) ? NS_SIDE_BOTTOM : NS_SIDE_TOP;
          nsIFrame* rg   = iter.rg;          if (!rg) ABORT0();
          nsIFrame* row  = iter.row;         if (!row) ABORT0();
          nsIFrame* cell = horSeg.firstCell; if (!cell) ABORT0();
          nsIFrame* col;

          PRUint8 style = NS_STYLE_BORDER_STYLE_SOLID; 
          nscolor color = 0xFFFFFFFF;
          PRBool ignoreIfRules = (iter.IsTopMostTable() || iter.IsBottomMostTable());

          switch (horSeg.owner) {
          case eTableOwner:
            ::GetPaintStyleInfo(this, side, style, color, tableIsLTR, PR_FALSE);
            break;
          case eAjaColGroupOwner: 
            NS_ASSERTION(PR_FALSE, "program error"); // and fall through
          case eColGroupOwner: {
            NS_ASSERTION(iter.IsTopMostTable() || iter.IsBottomMostTable(), "program error");
            col = firstInFlow->GetColFrame(iter.x - 1); if (!col) ABORT0();
            nsIFrame* cg = col->GetParent(); if (!cg) ABORT0();
            ::GetPaintStyleInfo(cg, side, style, color, tableIsLTR, ignoreIfRules);
            break;
          }
          case eAjaColOwner: 
            NS_ASSERTION(PR_FALSE, "program error"); // and fall through
          case eColOwner:
            NS_ASSERTION(iter.IsTopMostTable() || iter.IsBottomMostTable(), "program error");
            col = firstInFlow->GetColFrame(iter.x - 1); if (!col) ABORT0();
            ::GetPaintStyleInfo(col, side, style, color, tableIsLTR, ignoreIfRules);
            break;
          case eAjaRowGroupOwner: 
            side = NS_SIDE_BOTTOM;
            rg = (iter.IsBottomMostTable()) ? iter.rg : iter.prevRg; // and fall through
          case eRowGroupOwner:
            if (rg) {
              ::GetPaintStyleInfo(rg, side, style, color, tableIsLTR, ignoreIfRules);
            }
            break;
          case eAjaRowOwner: 
            side = NS_SIDE_BOTTOM;
            row = (iter.IsBottomMostTable()) ? iter.row : iter.prevRow; // and fall through
          case eRowOwner:
            if (row) {
              ::GetPaintStyleInfo(row, side, style, color, tableIsLTR, iter.IsBottomMostTable());
            }
            break;
          case eAjaCellOwner:
            side = NS_SIDE_BOTTOM;
            // if this is null due to the damage area origin-y > 0, then the border won't show up anyway
            cell = horSeg.ajaCell; 
            // and fall through
          case eCellOwner:
            if (cell) {
              ::GetPaintStyleInfo(cell, side, style, color, tableIsLTR, PR_FALSE);
            }
            break;
          }
          
          DivideBCBorderSize(horSeg.height, smallHalf, largeHalf);
          nsRect segRect(horSeg.x, horSeg.y - NSToCoordRound(p2t * (float)largeHalf), horSeg.width, 
                         NSToCoordRound(p2t * (float)horSeg.height));
           if (!tableIsLTR)
            segRect.x -= segRect.width;

          nscoord rightBevelOffset = (endBevel) ? NSToCoordRound(p2t * (float)verWidth) : 0;
          PRUint8 rightBevelSide = (leftSegWidth > 0) ? NS_SIDE_BOTTOM : NS_SIDE_TOP;
          if (tableIsLTR) {
            nsCSSRendering::DrawTableBorderSegment(aRenderingContext, style, color, bgColor, segRect, p2t, horSeg.leftBevelSide,
                                                 NSToCoordRound(p2t * (float)horSeg.leftBevelOffset), 
                                                 rightBevelSide, rightBevelOffset);
          }
          else {
            nsCSSRendering::DrawTableBorderSegment(aRenderingContext, style, color, bgColor, segRect, p2t, rightBevelSide, rightBevelOffset,
                                                 horSeg.leftBevelSide,  NSToCoordRound(p2t * (float)horSeg.leftBevelOffset));
          }

        } // if (horSeg.height > 0)
        horSeg.x += colInc * (horSeg.width - endOffset);
      } // if (horSeg.width > 0)
      horSeg.Start(iter, borderOwner, ownerSide, cornerSubWidth, bevel, verInfo[xAdj].segWidth, 
                   leftSegWidth, topSegHeight, verInfo[xAdj].lastCell, p2t, tableIsLTR);
    } // if (!iter.IsLeftMost() && (isSegStart || iter.IsRightMost() || verOwnsCorner))
    horSeg.width += verInfo[xAdj].colWidth;
    verInfo[xAdj].segWidth = leftSegWidth;
    verInfo[xAdj].lastCell = iter.cell;
  }
  delete [] verInfo;
}

/********************************************************************************
 ** DEBUG_TABLE_REFLOW_TIMING                                                  **
 ********************************************************************************/

#ifdef DEBUG

static PRBool 
GetFrameTypeName(nsIAtom* aFrameType,
                 char*    aName)
{
  PRBool isTable = PR_FALSE;
  if (nsLayoutAtoms::tableOuterFrame == aFrameType) 
    strcpy(aName, "Tbl");
  else if (nsLayoutAtoms::tableFrame == aFrameType) {
    strcpy(aName, "Tbl");
    isTable = PR_TRUE;
  }
  else if (nsLayoutAtoms::tableRowGroupFrame == aFrameType) 
    strcpy(aName, "RowG");
  else if (nsLayoutAtoms::tableRowFrame == aFrameType) 
    strcpy(aName, "Row");
  else if (IS_TABLE_CELL(aFrameType)) 
    strcpy(aName, "Cell");
  else if (nsLayoutAtoms::blockFrame == aFrameType) 
    strcpy(aName, "Block");
  else 
    NS_ASSERTION(PR_FALSE, "invalid call to GetFrameTypeName");

  return isTable;
}
#endif

#if defined DEBUG_TABLE_REFLOW_TIMING

#define INDENT_PER_LEVEL 1

void PrettyUC(nscoord aSize,
              char*   aBuf)
{
  if (NS_UNCONSTRAINEDSIZE == aSize) {
    strcpy(aBuf, "UC");
  }
  else {
    sprintf(aBuf, "%d", aSize);
  }
}

nsReflowTimer* GetFrameTimer(nsIFrame* aFrame,
                             nsIAtom*  aFrameType)
{
  if (nsLayoutAtoms::tableOuterFrame == aFrameType) 
    return ((nsTableOuterFrame*)aFrame)->mTimer;
  else if (nsLayoutAtoms::tableFrame == aFrameType) 
    return ((nsTableFrame*)aFrame)->mTimer;
  else if (nsLayoutAtoms::tableRowGroupFrame == aFrameType) 
    return ((nsTableRowGroupFrame*)aFrame)->mTimer;
  else if (nsLayoutAtoms::tableRowFrame == aFrameType) 
    return ((nsTableRowFrame*)aFrame)->mTimer;
  else if (IS_TABLE_CELL(aFrameType)) 
    return ((nsTableCellFrame*)aFrame)->mTimer;
  else if (nsLayoutAtoms::blockFrame == aFrameType) { 
    nsIFrame* parentFrame = aFrame->GetParent();
    if (IS_TABLE_CELL(parentFrame->GetType())) {
      nsTableCellFrame* cellFrame = (nsTableCellFrame*)parentFrame;
      // fix up the block timer, which may be referring to the cell
      if (cellFrame->mBlockTimer->mFrame == parentFrame) {
        cellFrame->mBlockTimer->mFrame = aFrame;
        NS_IF_RELEASE(cellFrame->mBlockTimer->mFrameType);
        cellFrame->mBlockTimer->mFrameType = nsLayoutAtoms::blockFrame;
        NS_ADDREF(cellFrame->mBlockTimer->mFrameType);
      }
      return cellFrame->mBlockTimer;
    }
  }
  return nsnull;
}

void DebugReflowPrintAuxTimer(char*          aMes, 
                              nsReflowTimer* aTimer)
{
  if (aTimer->mNumStarts > 0) {
    printf("%s %dms", aMes, aTimer->Elapsed());
    if (aTimer->mNumStarts > 1) {
      printf(" times=%d", aTimer->mNumStarts);
    }
  }
}

void DebugReflowPrint(nsReflowTimer& aTimer,
                      PRUint32       aLevel,
                      PRBool         aSummary)
{
  // set up the indentation
  char indentChar[128];
  PRInt32 indent = INDENT_PER_LEVEL * aLevel;
  memset (indentChar, ' ', indent);
  indentChar[indent] = 0;

  // get the frame type
  char fName[128];
  PRBool isTable = GetFrameTypeName(aTimer.mFrameType, fName);

  // print the timer
  printf("\n%s%s %dms %p", indentChar, fName, aTimer.Elapsed(), aTimer.mFrame);
  if (aSummary) {
    printf(" times=%d", aTimer.mNumStarts);
    if (isTable) {
      printf("\n%s", indentChar);
      DebugReflowPrintAuxTimer("init", aTimer.mNextSibling);
      DebugReflowPrintAuxTimer(" balanceCols", aTimer.mNextSibling->mNextSibling);
      DebugReflowPrintAuxTimer(" nonPctCols", aTimer.mNextSibling->mNextSibling->mNextSibling);
      DebugReflowPrintAuxTimer(" nonPctColspans", aTimer.mNextSibling->mNextSibling->mNextSibling->mNextSibling);
      DebugReflowPrintAuxTimer(" pctCols", aTimer.mNextSibling->mNextSibling->mNextSibling->mNextSibling->mNextSibling);
    }
  }
  else {
    char avWidth[16];
    char avHeight[16];
    char compWidth[16];
    char compHeight[16];
    char desWidth[16];
    char desHeight[16];
    PrettyUC(aTimer.mAvailWidth, avWidth);
    PrettyUC(aTimer.mAvailWidth, avHeight);
    PrettyUC(aTimer.mComputedWidth, compWidth);
    PrettyUC(aTimer.mComputedHeight, compHeight);
    PrettyUC(aTimer.mDesiredWidth, desWidth);
    PrettyUC(aTimer.mDesiredHeight, desHeight);
    printf(" r=%d", aTimer.mReason); 
    if (aTimer.mReflowType >= 0) {
      printf(",%d", aTimer.mReflowType);
    }
    printf(" a=%s,%s c=%s,%s d=%s,%s", avWidth, avHeight, compWidth, compHeight, desWidth, desHeight); 
    if (aTimer.mMaxElementWidth >= 0) {
      PrettyUC(aTimer.mMaxElementWidth, avWidth);
      printf(" me=%s", avWidth);
    }
    if (aTimer.mMaxWidth >= 0) {
      PrettyUC(aTimer.mMaxWidth, avWidth);
      printf(" m=%s", avWidth);
    }
    if (NS_FRAME_IS_NOT_COMPLETE(aTimer.mStatus)) {
      printf(" status=%d", aTimer.mStatus);
    }
    printf(" cnt=%d", aTimer.mCount);
    if (isTable) {
      printf("\n%s", indentChar);
      DebugReflowPrintAuxTimer("init", aTimer.mNextSibling);
      DebugReflowPrintAuxTimer(" balanceCols", aTimer.mNextSibling->mNextSibling);
      DebugReflowPrintAuxTimer(" nonPctCols", aTimer.mNextSibling->mNextSibling->mNextSibling);
      DebugReflowPrintAuxTimer(" nonPctColspans", aTimer.mNextSibling->mNextSibling->mNextSibling->mNextSibling);
      DebugReflowPrintAuxTimer(" pctCols", aTimer.mNextSibling->mNextSibling->mNextSibling->mNextSibling->mNextSibling);
    }
  }
  // print the timer's children
  nsVoidArray& children = aTimer.mChildren;
  PRInt32 numChildren = children.Count();
  for (PRInt32 childX = 0; childX < numChildren; childX++) {
    nsReflowTimer* child = (nsReflowTimer*)children.ElementAt(childX);
    if (child) {
      DebugReflowPrint(*child, aLevel + 1, aSummary);
    }
    else NS_ASSERTION(PR_FALSE, "bad DebugTimeReflow");
  }
}

void nsTableFrame::DebugReflow(nsIFrame*            aFrame,
                               nsHTMLReflowState&   aState,
                               nsHTMLReflowMetrics* aMetrics,
                               nsReflowStatus       aStatus)
{
#ifdef DEBUG_TABLE_REFLOW_TIMING_DETAIL
  // get the parent timer 
  const nsHTMLReflowState* parentRS = aState.parentReflowState;
  nsReflowTimer* parentTimer = nsnull;
  while (parentRS) {
    parentTimer = (nsReflowTimer *)parentRS->mDebugHook;
    if (parentTimer) break;
    parentRS = parentRS->parentReflowState;
  }
#endif
  // get the frame summary timer
  nsIAtom* frameType = aFrame->GetType();
  nsReflowTimer* frameTimer = GetFrameTimer(aFrame, frameType);
  if (!frameTimer) {NS_ASSERTION(PR_FALSE, "no frame timer");return;}
  if (!aMetrics) { // start
#ifdef DEBUG_TABLE_REFLOW_TIMING_DETAIL
    // create the reflow timer
    nsReflowTimer* timer = new nsReflowTimer(aFrame);
    // create the aux table timers if they don't exist
    if ((nsLayoutAtoms::tableFrame == frameType) && !timer->mNextSibling) {
      timer->mNextSibling = new nsReflowTimer(aFrame);
      timer->mNextSibling->mNextSibling = new nsReflowTimer(aFrame);
      timer->mNextSibling->mNextSibling->mNextSibling = new nsReflowTimer(aFrame);
      timer->mNextSibling->mNextSibling->mNextSibling->NextSibling = new nsReflowTimer(aFrame);
      timer->mNextSibling->mNextSibling->mNextSibling->NextSibling->mNextSibling = new nsReflowTimer(aFrame);
    }
    timer->mReason = aState.reason;
    timer->mAvailWidth = aState.availableWidth;
    timer->mComputedWidth = aState.mComputedWidth;
    timer->mComputedHeight = aState.mComputedHeight;
    timer->mCount = gRflCount++; 
    nsHTMLReflowCommand* reflowCommand = aState.reflowCommand;
    if (reflowCommand) {
      timer->mReflowType = reflowType->Type();
    }
    timer->Start();
    aState.mDebugHook = timer;
    if (parentTimer) {
      parentTimer->mChildren.AppendElement(timer);
    }
#endif
    // start the frame summary timer
    frameTimer->Start();
  }
  else {
#ifdef DEBUG_TABLE_REFLOW_TIMING_DETAIL
    // stop the reflow timer
    nsReflowTimer* timer = (nsReflowTimer *)aState.mDebugHook; 
    if (timer) {
      timer->Stop();
      timer->mDesiredWidth  = aMetrics->width;
      timer->mDesiredHeight = aMetrics->height;
      timer->mMaxElementWidth = (aMetrics->mComputeMEW) 
        ? aMetrics->mMaxElementWidth : -1;
      timer->mMaxWidth = (aMetrics->mFlags & NS_REFLOW_CALC_MAX_WIDTH) 
        ? aMetrics->mMaximumWidth : -1;
      timer->mStatus = aStatus;
    }
    else {NS_ASSERTION(PR_FALSE, "bad DebugTimeReflow");return;}
    // stop the frame summary timer
#endif
    frameTimer->Stop();
#ifdef DEBUG_TABLE_REFLOW_TIMING_DETAIL
    if (!parentTimer) {
      // print out all of the reflow timers
      DebugReflowPrint(*timer, 0, PR_FALSE);
      timer->Destroy();
    }
#endif
  }
}

void nsTableFrame::DebugTimeMethod(nsMethod           aMethod,
                                   nsTableFrame&      aFrame,
                                   nsHTMLReflowState& aState,
                                   PRBool             aStart)
{
  nsReflowTimer* baseTimer = (nsReflowTimer*)aState.mDebugHook;
  nsReflowTimer* timer;
  PRInt32 offset = aMethod;
  PRInt32 idx;
  if (aStart) {
#ifdef DEBUG_TABLE_REFLOW_TIMING_DETAIL
    timer = baseTimer;
    for (idx = 0; idx <= offset; idx++) {
      timer = timer->mNextSibling;
    }
    timer->Start();
#endif
    timer = aFrame.mTimer;
    for (idx = 0; idx <= offset; idx++) {
      timer = timer->mNextSibling;
    }
    timer->Start();
  }
  else {
#ifdef DEBUG_TABLE_REFLOW_TIMING_DETAIL
    timer = baseTimer;
    for (idx = 0; idx <= offset; idx++) {
      timer = timer->mNextSibling;
    }
    timer->Stop();
#endif
    timer = aFrame.mTimer;
    for (idx = 0; idx <= offset; idx++) {
      timer = timer->mNextSibling;
    }
    timer->Stop();
  }
}

void nsTableFrame::DebugReflowDone(nsIFrame* aFrame)
{
  // get the timer of aFrame
  nsReflowTimer* thisTimer = GetFrameTimer(aFrame, aFrame->GetType());

  // get the nearest ancestor frame with a timer
  nsReflowTimer* ancestorTimer;
  nsIFrame* ancestorFrame = aFrame->GetParent();
  while (ancestorFrame) {
    ancestorTimer = GetFrameTimer(ancestorFrame, ancestorFrame->GetType());
    if (ancestorTimer) break;
    ancestorFrame = ancestorFrame->GetParent();
  }
  if (ancestorTimer) { // add this timer to its parent
    ancestorTimer->mChildren.AppendElement(thisTimer);
    if (IS_TABLE_CELL(aFrame->GetType())) {
      // add the cell block timer as a child of the cell timer
      nsTableCellFrame* cellFrame = (nsTableCellFrame*)aFrame;
      cellFrame->mTimer->mChildren.AppendElement(cellFrame->mBlockTimer);
    }
  }
  else { // print out all of the frame timers
    printf("\n\nSUMMARY OF REFLOW BY FRAME\n");
    DebugReflowPrint(*thisTimer, 0, PR_TRUE);
    thisTimer->Destroy();
  }
}

#endif //DEBUG_TABLE_REFLOW_TIMING


PRBool nsTableFrame::RowHasSpanningCells(PRInt32 aRowIndex, PRInt32 aNumEffCols)
{
  PRBool result = PR_FALSE;
  nsTableCellMap* cellMap = GetCellMap();
  NS_PRECONDITION (cellMap, "bad call, cellMap not yet allocated.");
  if (cellMap) {
    result = cellMap->RowHasSpanningCells(aRowIndex, aNumEffCols);
  }
  return result;
}

PRBool nsTableFrame::RowIsSpannedInto(PRInt32 aRowIndex, PRInt32 aNumEffCols)
{
  PRBool result = PR_FALSE;
  nsTableCellMap* cellMap = GetCellMap();
  NS_PRECONDITION (cellMap, "bad call, cellMap not yet allocated.");
  if (cellMap) {
    result = cellMap->RowIsSpannedInto(aRowIndex, aNumEffCols);
  }
  return result;
}

PRBool nsTableFrame::ColHasSpanningCells(PRInt32 aColIndex)
{
  PRBool result = PR_FALSE;
  nsTableCellMap * cellMap = GetCellMap();
  NS_PRECONDITION (cellMap, "bad call, cellMap not yet allocated.");
  if (cellMap) {
    result = cellMap->ColHasSpanningCells(aColIndex);
  }
  return result;
}

PRBool nsTableFrame::ColIsSpannedInto(PRInt32 aColIndex)
{
  PRBool result = PR_FALSE;
  nsTableCellMap * cellMap = GetCellMap();
  NS_PRECONDITION (cellMap, "bad call, cellMap not yet allocated.");
  if (cellMap) {
    result = cellMap->ColIsSpannedInto(aColIndex);
  }
  return result;
}

// Destructor function for nscoord properties
static void
DestroyCoordFunc(void*           aFrame,
                 nsIAtom*        aPropertyName,
                 void*           aPropertyValue,
                 void*           aDtorData)
{
  delete NS_STATIC_CAST(nscoord*, aPropertyValue);
}

// Destructor function point properties
static void
DestroyPointFunc(void*           aFrame,
                 nsIAtom*        aPropertyName,
                 void*           aPropertyValue,
                 void*           aDtorData)
{
  delete NS_STATIC_CAST(nsPoint*, aPropertyValue);
}

// Destructor function for nscoord properties
static void
DestroyBCPropertyDataFunc(void*           aFrame,
                          nsIAtom*        aPropertyName,
                          void*           aPropertyValue,
                          void*           aDtorData)
{
  delete NS_STATIC_CAST(BCPropertyData*, aPropertyValue);
}

void*
nsTableFrame::GetProperty(nsIFrame*            aFrame,
                          nsIAtom*             aPropertyName,
                          PRBool               aCreateIfNecessary)
{
  nsPropertyTable *propTable = aFrame->GetPresContext()->PropertyTable();
  void *value = propTable->GetProperty(aFrame, aPropertyName);
  if (value) {
    return (nsPoint*)value;  // the property already exists
  }
  if (aCreateIfNecessary) {
    // The property isn't set yet, so allocate a new value, set the property,
    // and return the newly allocated value
    NSPropertyDtorFunc dtorFunc = nsnull;
    if (aPropertyName == nsLayoutAtoms::collapseOffsetProperty) {
      value = new nsPoint(0, 0);
      dtorFunc = DestroyPointFunc;
    }
    else if (aPropertyName == nsLayoutAtoms::rowUnpaginatedHeightProperty) {
      value = new nscoord;
      dtorFunc = DestroyCoordFunc;
    }
    else if (aPropertyName == nsLayoutAtoms::tableBCProperty) {
      value = new BCPropertyData;
      dtorFunc = DestroyBCPropertyDataFunc;
    }
    if (value) {
      propTable->SetProperty(aFrame, aPropertyName, value, dtorFunc, nsnull);
    }
    return value;
  }
  return nsnull;
}

#ifdef DEBUG
#define MAX_SIZE  128
#define MIN_INDENT 30

static 
void DumpTableFramesRecur(nsIFrame*       aFrame,
                          PRUint32        aIndent)
{
  char indent[MAX_SIZE + 1];
  aIndent = PR_MIN(aIndent, MAX_SIZE - MIN_INDENT);
  memset (indent, ' ', aIndent + MIN_INDENT);
  indent[aIndent + MIN_INDENT] = 0;

  char fName[MAX_SIZE];
  nsIAtom* fType = aFrame->GetType();
  GetFrameTypeName(fType, fName);

  printf("%s%s %p", indent, fName, aFrame);
  nsIFrame* flowFrame = aFrame->GetPrevInFlow();
  if (flowFrame) {
    printf(" pif=%p", flowFrame);
  }
  flowFrame = aFrame->GetNextInFlow();
  if (flowFrame) {
    printf(" nif=%p", flowFrame);
  }
  printf("\n");

  if (nsLayoutAtoms::tableFrame         == fType ||
      nsLayoutAtoms::tableRowGroupFrame == fType ||
      nsLayoutAtoms::tableRowFrame      == fType ||
      IS_TABLE_CELL(fType)) {
    nsIFrame* child = aFrame->GetFirstChild(nsnull);
    while(child) {
      DumpTableFramesRecur(child, aIndent+1);
      child = child->GetNextSibling();
    }
  }
}
  
void
nsTableFrame::DumpTableFrames(nsIFrame* aFrame)
{
  nsTableFrame* tableFrame = nsnull;

  if (nsLayoutAtoms::tableFrame == aFrame->GetType()) { 
    tableFrame = NS_STATIC_CAST(nsTableFrame*, aFrame);
  }
  else {
    tableFrame = nsTableFrame::GetTableFrame(aFrame);
  }
  tableFrame = NS_STATIC_CAST(nsTableFrame*, tableFrame->GetFirstInFlow());
  while (tableFrame) {
    DumpTableFramesRecur(tableFrame, 0);
    tableFrame = NS_STATIC_CAST(nsTableFrame*, tableFrame->GetNextInFlow());
  }
}

#endif



