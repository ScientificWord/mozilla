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
#include "nsTableRowFrame.h"
#include "nsTableRowGroupFrame.h"
#include "nsIRenderingContext.h"
#include "nsIPresShell.h"
#include "nsPresContext.h"
#include "nsStyleContext.h"
#include "nsStyleConsts.h"
#include "nsHTMLAtoms.h"
#include "nsIContent.h"
#include "nsTableFrame.h"
#include "nsTableCellFrame.h"
#include "nsIView.h"
#include "nsReflowPath.h"
#include "nsCSSRendering.h"
#include "nsLayoutAtoms.h"
#include "nsHTMLParts.h"
#include "nsTableColGroupFrame.h"
#include "nsTableColFrame.h"
#include "nsCOMPtr.h"
#include "nsDisplayList.h"

struct nsTableCellReflowState : public nsHTMLReflowState
{
  nsTableCellReflowState(nsPresContext*          aPresContext,
                         const nsHTMLReflowState& aParentReflowState,
                         nsIFrame*                aFrame,
                         const nsSize&            aAvailableSpace,
                         nsReflowReason           aReason);

  void FixUp(const nsSize& aAvailSpace,
             PRBool        aResetComputedWidth);
};

nsTableCellReflowState::nsTableCellReflowState(nsPresContext*          aPresContext,
                                               const nsHTMLReflowState& aParentRS,
                                               nsIFrame*                aFrame,
                                               const nsSize&            aAvailSpace,
                                               nsReflowReason           aReason)
  :nsHTMLReflowState(aPresContext, aParentRS, aFrame, aAvailSpace, aReason)
{
}

void nsTableCellReflowState::FixUp(const nsSize& aAvailSpace,
                                   PRBool        aResetComputedWidth)
{
  // fix the mComputed values during a pass 2 reflow since the cell can be a percentage base
  if (NS_UNCONSTRAINEDSIZE != aAvailSpace.width) {
    if (aResetComputedWidth) {
      mComputedWidth = NS_UNCONSTRAINEDSIZE;
    }
    else if (NS_UNCONSTRAINEDSIZE != mComputedWidth) {
      mComputedWidth = aAvailSpace.width - mComputedBorderPadding.left - mComputedBorderPadding.right;
      mComputedWidth = PR_MAX(0, mComputedWidth);
    }
    if (NS_UNCONSTRAINEDSIZE != mComputedHeight) {
      if (NS_UNCONSTRAINEDSIZE != aAvailSpace.height) {
        mComputedHeight = aAvailSpace.height - mComputedBorderPadding.top - mComputedBorderPadding.bottom;
        mComputedHeight = PR_MAX(0, mComputedHeight);
      }
    }
  }
}

void
nsTableRowFrame::InitChildReflowState(nsPresContext&         aPresContext,
                                      const nsSize&           aAvailSize,
                                      PRBool                  aBorderCollapse,
                                      float                   aPixelsToTwips,
                                      nsTableCellReflowState& aReflowState,
                                      PRBool                  aResetComputedWidth)
{
  nsMargin collapseBorder;
  nsMargin* pCollapseBorder = nsnull;
  if (aBorderCollapse) {
    // we only reflow cells, so don't need to check frame type
    nsBCTableCellFrame* bcCellFrame = (nsBCTableCellFrame*)aReflowState.frame;
    if (bcCellFrame) {
      pCollapseBorder = bcCellFrame->GetBorderWidth(aPixelsToTwips, collapseBorder);
    }
  }
  aReflowState.Init(&aPresContext, -1, -1, pCollapseBorder);
  aReflowState.FixUp(aAvailSize, aResetComputedWidth);
}

void 
nsTableRowFrame::SetFixedHeight(nscoord aValue)
{
  nscoord height = PR_MAX(0, aValue);
  if (HasFixedHeight()) {
    if (height > mStyleFixedHeight) {
      mStyleFixedHeight = height;
    }
  }
  else {
    mStyleFixedHeight = height;
    if (height > 0) {
      SetHasFixedHeight(PR_TRUE);
    }
  }
}

void 
nsTableRowFrame::SetPctHeight(float  aPctValue,
                              PRBool aForce)
{
  nscoord height = PR_MAX(0, NSToCoordRound(aPctValue * 100.0f));
  if (HasPctHeight()) {
    if ((height > mStylePctHeight) || aForce) {
      mStylePctHeight = height;
    }
  }
  else {
    mStylePctHeight = height;
    if (height > 0.0f) {
      SetHasPctHeight(PR_TRUE);
    }
  }
}

// 'old' is old cached cell's desired size
// 'new' is new cell's size including style constraints
static PRBool
TallestCellGotShorter(nscoord aOld,
                      nscoord aNew,
                      nscoord aTallest)
{
  return ((aNew < aOld) && (aOld == aTallest));
}

/* ----------- nsTableRowFrame ---------- */

nsTableRowFrame::nsTableRowFrame(nsStyleContext* aContext)
  : nsHTMLContainerFrame(aContext)
{
  mBits.mRowIndex = mBits.mFirstInserted = 0;
  ResetHeight(0);
#ifdef DEBUG_TABLE_REFLOW_TIMING
  mTimer = new nsReflowTimer(this);
#endif
}

nsTableRowFrame::~nsTableRowFrame()
{
#ifdef DEBUG_TABLE_REFLOW_TIMING
  nsTableFrame::DebugReflowDone(this);
#endif
}

NS_IMETHODIMP
nsTableRowFrame::Init(nsIContent*      aContent,
                      nsIFrame*        aParent,
                      nsIFrame*        aPrevInFlow)
{
  nsresult  rv;

  // Let the base class do its initialization
  rv = nsHTMLContainerFrame::Init(aContent, aParent, aPrevInFlow);

  // record that children that are ignorable whitespace should be excluded 
  mState |= NS_FRAME_EXCLUDE_IGNORABLE_WHITESPACE;

  if (aPrevInFlow) {
    // Set the row index
    nsTableRowFrame* rowFrame = (nsTableRowFrame*)aPrevInFlow;
    
    SetRowIndex(rowFrame->GetRowIndex());
  }

  return rv;
}


NS_IMETHODIMP
nsTableRowFrame::AppendFrames(nsIAtom*        aListName,
                              nsIFrame*       aFrameList)
{
  NS_ASSERTION(!aListName, "unexpected child list");

  // Append the frames
  mFrames.AppendFrames(nsnull, aFrameList);

  // Add the new cell frames to the table
  nsTableFrame *tableFrame =  nsTableFrame::GetTableFrame(this);
  for (nsIFrame* childFrame = aFrameList; childFrame;
       childFrame = childFrame->GetNextSibling()) {
    if (IS_TABLE_CELL(childFrame->GetType())) {
      // Add the cell to the cell map
      tableFrame->AppendCell((nsTableCellFrame&)*childFrame, GetRowIndex());
      // XXX this could be optimized with some effort
      tableFrame->SetNeedStrategyInit(PR_TRUE);
    }
  }

  // Reflow the new frames. They're already marked dirty, so generate a reflow
  // command that tells us to reflow our dirty child frames
  tableFrame->AppendDirtyReflowCommand(this);

  return NS_OK;
}


NS_IMETHODIMP
nsTableRowFrame::InsertFrames(nsIAtom*        aListName,
                              nsIFrame*       aPrevFrame,
                              nsIFrame*       aFrameList)
{
  NS_ASSERTION(!aListName, "unexpected child list");
  NS_ASSERTION(!aPrevFrame || aPrevFrame->GetParent() == this,
               "inserting after sibling frame with different parent");

  // Get the table frame
  nsTableFrame* tableFrame = nsTableFrame::GetTableFrame(this);
  
  // gather the new frames (only those which are cells) into an array
  nsIAtom* cellFrameType = (tableFrame->IsBorderCollapse()) ? nsLayoutAtoms::bcTableCellFrame : nsLayoutAtoms::tableCellFrame;
  nsTableCellFrame* prevCellFrame = (nsTableCellFrame *)nsTableFrame::GetFrameAtOrBefore(this, aPrevFrame, cellFrameType);
  nsVoidArray cellChildren;
  for (nsIFrame* childFrame = aFrameList; childFrame;
       childFrame = childFrame->GetNextSibling()) {
    if (IS_TABLE_CELL(childFrame->GetType())) {
      cellChildren.AppendElement(childFrame);
      // XXX this could be optimized with some effort
      tableFrame->SetNeedStrategyInit(PR_TRUE);
    }
  }
  // insert the cells into the cell map
  PRInt32 colIndex = -1;
  if (prevCellFrame) {
    prevCellFrame->GetColIndex(colIndex);
  }
  tableFrame->InsertCells(cellChildren, GetRowIndex(), colIndex);

  // Insert the frames in the frame list
  mFrames.InsertFrames(nsnull, aPrevFrame, aFrameList);
  
  // Reflow the new frames. They're already marked dirty, so generate a reflow
  // command that tells us to reflow our dirty child frames
  tableFrame->AppendDirtyReflowCommand(this);

  return NS_OK;
}

NS_IMETHODIMP
nsTableRowFrame::RemoveFrame(nsIAtom*        aListName,
                             nsIFrame*       aOldFrame)
{
  NS_ASSERTION(!aListName, "unexpected child list");

  nsTableFrame* tableFrame = nsTableFrame::GetTableFrame(this);
  if (tableFrame) {
    if (IS_TABLE_CELL(aOldFrame->GetType())) {
      nsTableCellFrame* cellFrame = (nsTableCellFrame*)aOldFrame;
      PRInt32 colIndex;
      cellFrame->GetColIndex(colIndex);
      // remove the cell from the cell map
      tableFrame->RemoveCell(cellFrame, GetRowIndex());
      // XXX this could be optimized with some effort
      tableFrame->SetNeedStrategyInit(PR_TRUE);

      // Remove the frame and destroy it
      mFrames.DestroyFrame(aOldFrame);

      // XXX This could probably be optimized with much effort
      tableFrame->SetNeedStrategyInit(PR_TRUE);
      // Generate a reflow command so we reflow the table itself.
      // Target the row so that it gets a dirty reflow before a resize reflow
      // in case another cell gets added to the row during a reflow coallesce.
      tableFrame->AppendDirtyReflowCommand(this);
    }
    else {
      NS_ERROR("unexpected frame type");
      return NS_ERROR_INVALID_ARG;
    }
  }

  return NS_OK;
}

nscoord 
GetHeightOfRowsSpannedBelowFirst(nsTableCellFrame& aTableCellFrame,
                                 nsTableFrame&     aTableFrame)
{
  nscoord height = 0;
  nscoord cellSpacingY = aTableFrame.GetCellSpacingY();
  PRInt32 rowSpan = aTableFrame.GetEffectiveRowSpan(aTableCellFrame);
  // add in height of rows spanned beyond the 1st one
  nsIFrame* nextRow = aTableCellFrame.GetParent()->GetNextSibling();
  for (PRInt32 rowX = 1; ((rowX < rowSpan) && nextRow);) {
    if (nsLayoutAtoms::tableRowFrame == nextRow->GetType()) {
      height += nextRow->GetSize().height;
      rowX++;
    }
    height += cellSpacingY;
    nextRow = nextRow->GetNextSibling();
  }
  return height;
}

nsTableCellFrame*  
nsTableRowFrame::GetFirstCell() 
{
  nsIFrame* childFrame = mFrames.FirstChild();
  while (childFrame) {
    if (IS_TABLE_CELL(childFrame->GetType())) {
      return (nsTableCellFrame*)childFrame;
    }
    childFrame = childFrame->GetNextSibling();
  }
  return nsnull;
}

/**
 * Post-reflow hook. This is where the table row does its post-processing
 */
void
nsTableRowFrame::DidResize(const nsHTMLReflowState& aReflowState)
{
  // Resize and re-align the cell frames based on our row height
  nsTableFrame* tableFrame = nsTableFrame::GetTableFrame(this);
  if (!tableFrame)
    return;
  
  nsTableIterator iter(*this, eTableDIR);
  nsIFrame* childFrame = iter.First();
  
  nsHTMLReflowMetrics desiredSize(PR_FALSE);
  desiredSize.width = mRect.width;
  desiredSize.height = mRect.height;
  desiredSize.mOverflowArea = nsRect(0, 0, desiredSize.width,
                                      desiredSize.height);

  while (childFrame) {
    if (IS_TABLE_CELL(childFrame->GetType())) {
      nsTableCellFrame* cellFrame = (nsTableCellFrame*)childFrame;
      nscoord cellHeight = mRect.height + GetHeightOfRowsSpannedBelowFirst(*cellFrame, *tableFrame);

      // resize the cell's height
      //if (cellFrameSize.height!=cellHeight)
      {
        // XXX If the cell frame has a view, then we need to resize
        // it as well. We would like to only do that if the cell's size
        // is changing. Why is the 'if' stmt above commented out?
        cellFrame->SetSize(nsSize(cellFrame->GetSize().width, cellHeight));
        // realign cell content based on the new height
        /*nsHTMLReflowMetrics desiredSize(nsnull);
        nsHTMLReflowState kidReflowState(aPresContext, aReflowState,
                                         cellFrame,
                                         nsSize(cellFrameSize.width, cellHeight),
                                         eReflowReason_Resize);*/
        //XXX: the following reflow is necessary for any content of the cell
        //     whose height is a percent of the cell's height (maybe indirectly.)
        //     But some content crashes when this reflow is issued, to be investigated
        //XXX nsReflowStatus status;
        //ReflowChild(cellFrame, aPresContext, desiredSize, kidReflowState, status);

        cellFrame->VerticallyAlignChild(aReflowState, mMaxCellAscent);
        ConsiderChildOverflow(desiredSize.mOverflowArea, cellFrame);
      }
    }
    // Get the next child
    childFrame = iter.Next();
  }
  FinishAndStoreOverflow(&desiredSize);
  if (HasView()) {
    nsContainerFrame::SyncFrameViewAfterReflow(GetPresContext(), this, GetView(), &desiredSize.mOverflowArea, 0);
  }
  // Let our base class do the usual work
}

// returns max-ascent amongst all cells that have 'vertical-align: baseline'
// *including* cells with rowspans
nscoord nsTableRowFrame::GetMaxCellAscent() const
{
  return mMaxCellAscent;
}

nscoord nsTableRowFrame::GetAscent()
{
  if(mMaxCellAscent)
    return mMaxCellAscent;

  // If we don't have a baseline on any of the cells we go for the lowest 
  // content edge of the inner block frames. 
  // Every table cell has a cell frame with its border and padding. Inside
  // the cell is a block frame. The cell is as high as the tallest cell in
  // the parent row. As a consequence the block frame might not touch both
  // the top and the bottom padding of it parent cell frame at the same time.
  // 
  // bbbbbbbbbbbbbbbbbb             cell border:  b
  // bppppppppppppppppb             cell padding: p
  // bpxxxxxxxxxxxxxxpb             inner block:  x
  // bpx            xpb
  // bpx            xpb
  // bpx            xpb
  // bpxxxxxxxxxxxxxxpb  base line
  // bp              pb
  // bp              pb
  // bppppppppppppppppb
  // bbbbbbbbbbbbbbbbbb

  nsTableIterator iter(*this, eTableDIR);
  nsIFrame* childFrame = iter.First();
  nscoord ascent = 0;
   while (childFrame) {
    if (IS_TABLE_CELL(childFrame->GetType())) {
      nsIFrame* firstKid = childFrame->GetFirstChild(nsnull);
      ascent = PR_MAX(ascent, firstKid->GetRect().YMost());
    }
    // Get the next child
    childFrame = iter.Next();
  }
  return ascent;
}
nscoord
nsTableRowFrame::GetHeight(nscoord aPctBasis) const
{
  nscoord height = 0;
  if ((aPctBasis > 0) && HasPctHeight()) {
    height = NSToCoordRound(GetPctHeight() * (float)aPctBasis);
  }
  if (HasFixedHeight()) {
    height = PR_MAX(height, GetFixedHeight());
  }
  return PR_MAX(height, GetContentHeight());
}

void 
nsTableRowFrame::ResetHeight(nscoord aFixedHeight)
{
  SetHasFixedHeight(PR_FALSE);
  SetHasPctHeight(PR_FALSE);
  SetFixedHeight(0);
  SetPctHeight(0);
  SetContentHeight(0);

  if (aFixedHeight > 0) {
    SetFixedHeight(aFixedHeight);
  }

  mMaxCellAscent = 0;
  mMaxCellDescent = 0;
}

void
nsTableRowFrame::UpdateHeight(nscoord           aHeight,
                              nscoord           aAscent,
                              nscoord           aDescent,
                              nsTableFrame*     aTableFrame,
                              nsTableCellFrame* aCellFrame)
{
  if (!aTableFrame || !aCellFrame) {
    NS_ASSERTION(PR_FALSE , "invalid call");
    return;
  }

  if (aHeight != NS_UNCONSTRAINEDSIZE) {
    if (!(aCellFrame->HasVerticalAlignBaseline())) { // only the cell's height matters
      if (GetHeight() < aHeight) {
        PRInt32 rowSpan = aTableFrame->GetEffectiveRowSpan(*aCellFrame);
        if (rowSpan == 1) {
          SetContentHeight(aHeight);
        }
      }
    }
    else { // the alignment on the baseline can change the height
      NS_ASSERTION((aAscent != NS_UNCONSTRAINEDSIZE) && (aDescent != NS_UNCONSTRAINEDSIZE), "invalid call");
      // see if this is a long ascender
      if (mMaxCellAscent < aAscent) {
        mMaxCellAscent = aAscent;
      }
      // see if this is a long descender and without rowspan
      if (mMaxCellDescent < aDescent) {
        PRInt32 rowSpan = aTableFrame->GetEffectiveRowSpan(*aCellFrame);
        if (rowSpan == 1) {
          mMaxCellDescent = aDescent;
        }
      }
      // keep the tallest height in sync
      if (GetHeight() < mMaxCellAscent + mMaxCellDescent) {
        SetContentHeight(mMaxCellAscent + mMaxCellDescent);
      }
    }
  }
}

nscoord
nsTableRowFrame::CalcHeight(const nsHTMLReflowState& aReflowState)
{
  nsTableFrame* tableFrame = nsTableFrame::GetTableFrame(this);
  if (!tableFrame)
    return 0;

  nscoord computedHeight = (NS_UNCONSTRAINEDSIZE == aReflowState.mComputedHeight)
                            ? 0 : aReflowState.mComputedHeight;
  ResetHeight(computedHeight);

  const nsStylePosition* position = GetStylePosition();
  if (eStyleUnit_Coord == position->mHeight.GetUnit()) {
    SetFixedHeight(position->mHeight.GetCoordValue());
  }
  else if (eStyleUnit_Percent == position->mHeight.GetUnit()) {
    SetPctHeight(position->mHeight.GetPercentValue());
  }

  for (nsIFrame* kidFrame = mFrames.FirstChild(); kidFrame;
       kidFrame = kidFrame->GetNextSibling()) {
    if (IS_TABLE_CELL(kidFrame->GetType())) {
      nscoord availWidth = ((nsTableCellFrame *)kidFrame)->GetPriorAvailWidth();
      nsSize desSize = ((nsTableCellFrame *)kidFrame)->GetDesiredSize();
      if ((NS_UNCONSTRAINEDSIZE == aReflowState.availableHeight) && !GetPrevInFlow()) {
        CalculateCellActualSize(kidFrame, desSize.width, desSize.height, availWidth);
      }
      // height may have changed, adjust descent to absorb any excess difference
      nscoord ascent;
       if (!kidFrame->GetFirstChild(nsnull)->GetFirstChild(nsnull))
         ascent = desSize.height;
       else
         ascent = ((nsTableCellFrame *)kidFrame)->GetDesiredAscent();
      nscoord descent = desSize.height - ascent;
      UpdateHeight(desSize.height, ascent, descent, tableFrame, (nsTableCellFrame*)kidFrame);
    }
  }
  return GetHeight();
}

/**
 * We need a custom display item for table row backgrounds. This is only used
 * when the table row is the root of a stacking context (e.g., has 'opacity').
 * Table row backgrounds can extend beyond the row frame bounds, when
 * the row contains row-spanning cells.
 */
class nsDisplayTableRowBackground : public nsDisplayItem {
public:
  nsDisplayTableRowBackground(nsTableRowFrame* aFrame) : nsDisplayItem(aFrame) {
    MOZ_COUNT_CTOR(nsDisplayTableRowBackground);
  }
#ifdef NS_BUILD_REFCNT_LOGGING
  virtual ~nsDisplayTableRowBackground() {
    MOZ_COUNT_DTOR(nsDisplayTableRowBackground);
  }
#endif

  virtual nsRect GetBounds(nsDisplayListBuilder* aBuilder) {
    // the overflow rect contains any row-spanning cells, so it contains
    // our background. Note that this means we may not be opaque even if
    // the background style is a solid color.
    return NS_STATIC_CAST(nsTableRowFrame*, mFrame)->GetOverflowRect() +
      aBuilder->ToReferenceFrame(mFrame);
  }
  virtual void Paint(nsDisplayListBuilder* aBuilder, nsIRenderingContext* aCtx,
     const nsRect& aDirtyRect);
  NS_DISPLAY_DECL_NAME("TableRowBackground")
};

void
nsDisplayTableRowBackground::Paint(nsDisplayListBuilder* aBuilder,
    nsIRenderingContext* aCtx, const nsRect& aDirtyRect) {
  nsTableFrame* tableFrame = nsTableFrame::GetTableFrame(mFrame);

  nsPoint pt = aBuilder->ToReferenceFrame(mFrame);
  nsIRenderingContext::AutoPushTranslation translate(aCtx, pt.x, pt.y);
  TableBackgroundPainter painter(tableFrame,
                                 TableBackgroundPainter::eOrigin_TableRow,
                                 mFrame->GetPresContext(), *aCtx,
                                 aDirtyRect - pt);
  painter.PaintRow(NS_STATIC_CAST(nsTableRowFrame*, mFrame));
}

NS_IMETHODIMP
nsTableRowFrame::BuildDisplayList(nsDisplayListBuilder*   aBuilder,
                                  const nsRect&           aDirtyRect,
                                  const nsDisplayListSet& aLists)
{
  if (!IsVisibleInSelection(aBuilder))
    return NS_OK;

  PRBool isRoot = aBuilder->IsAtRootOfPseudoStackingContext();
  if (isRoot) {
    // This background is created regardless of whether this frame is
    // visible or not. Visibility decisions are delegated to the
    // table background painter.
    // We would use nsDisplayGeneric for this rare case except that we
    // need the background to be larger than the row frame in some
    // cases.
    nsresult rv = aLists.BorderBackground()->AppendNewToTop(new (aBuilder)
        nsDisplayTableRowBackground(this));
    NS_ENSURE_SUCCESS(rv, rv);
  }
  
  return nsTableFrame::DisplayGenericTablePart(aBuilder, this, aDirtyRect, aLists, isRoot);
}

PRIntn
nsTableRowFrame::GetSkipSides() const
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

// Calculate the cell's actual size given its pass2 desired width and height.
// Takes into account the specified height (in the style), and any special logic
// needed for backwards compatibility.
// Modifies the desired width and height that are passed in.
nsresult
nsTableRowFrame::CalculateCellActualSize(nsIFrame* aCellFrame,
                                         nscoord&  aDesiredWidth,
                                         nscoord&  aDesiredHeight,
                                         nscoord   aAvailWidth)
{
  nscoord specifiedHeight = 0;
  
  // Get the height specified in the style information
  const nsStylePosition* position = aCellFrame->GetStylePosition();

  nsTableFrame* tableFrame = nsTableFrame::GetTableFrame(this);
  if (!tableFrame)
    return NS_ERROR_NULL_POINTER;
  
  PRInt32 rowSpan = tableFrame->GetEffectiveRowSpan((nsTableCellFrame&)*aCellFrame);
  
  switch (position->mHeight.GetUnit()) {
    case eStyleUnit_Coord:
      specifiedHeight = position->mHeight.GetCoordValue();
      if (1 == rowSpan) 
        SetFixedHeight(specifiedHeight);
      break;
    case eStyleUnit_Percent: {
      if (1 == rowSpan) 
        SetPctHeight(position->mHeight.GetPercentValue());
      // pct heights are handled when all of the cells are finished, so don't set specifiedHeight 
      break;
    }
    case eStyleUnit_Auto:
    default:
      break;
  }

  // If the specified height is greater than the desired height, then use the specified height
  if (specifiedHeight > aDesiredHeight)
    aDesiredHeight = specifiedHeight;
 
  if ((0 == aDesiredWidth) && (NS_UNCONSTRAINEDSIZE != aAvailWidth)) { // special Nav4 compatibility code for the width
    aDesiredWidth = aAvailWidth;
  } 

  return NS_OK;
}

// Calculates the available width for the table cell based on the known
// column widths taking into account column spans and column spacing
static void 
CalcAvailWidth(nsTableFrame&     aTableFrame,
               nscoord           aTableComputedWidth,
               float             aPixelToTwips,
               nsTableCellFrame& aCellFrame,
               nscoord           aCellSpacingX,
               nscoord&          aColAvailWidth,
               nscoord&          aCellAvailWidth)
{
  aColAvailWidth = aCellAvailWidth = NS_UNCONSTRAINEDSIZE;
  PRInt32 colIndex;
  aCellFrame.GetColIndex(colIndex);
  PRInt32 colspan = aTableFrame.GetEffectiveColSpan(aCellFrame);
  nscoord cellSpacing = 0;

  for (PRInt32 spanX = 0; spanX < colspan; spanX++) {
    nscoord colWidth = aTableFrame.GetColumnWidth(colIndex + spanX);
    if (colWidth != WIDTH_NOT_SET) {
      if (NS_UNCONSTRAINEDSIZE == aColAvailWidth) {
        aColAvailWidth = colWidth; 
      }
      else {
        aColAvailWidth += colWidth;
      }
    }
    if ((spanX > 0) && (aTableFrame.GetNumCellsOriginatingInCol(colIndex + spanX) > 0)) {
      cellSpacing += aCellSpacingX;
    }
  }
  if (NS_UNCONSTRAINEDSIZE != aColAvailWidth) {
    aColAvailWidth += cellSpacing;
  } 
  aCellAvailWidth = aColAvailWidth;

  // for a cell with a colspan > 1, use its fix width (if set) as the avail width 
  // if this is its initial reflow
  if ((aCellFrame.GetStateBits() & NS_FRAME_FIRST_REFLOW)
      && (aTableFrame.GetEffectiveColSpan(aCellFrame) > 1)) {
    // see if the cell has a style width specified
    const nsStylePosition* cellPosition = aCellFrame.GetStylePosition();
    if (eStyleUnit_Coord == cellPosition->mWidth.GetUnit()) {
      // need to add padding into fixed width
      nsMargin borderPadding(0,0,0,0);
      if (NS_UNCONSTRAINEDSIZE != aTableComputedWidth) {
        borderPadding = nsTableFrame::GetBorderPadding(nsSize(aTableComputedWidth, 0), 
                                                       aPixelToTwips,  &aCellFrame);
      }
      nscoord fixWidth = cellPosition->mWidth.GetCoordValue() + borderPadding.left + borderPadding.right;
      if (NS_UNCONSTRAINEDSIZE != aColAvailWidth) {
        aCellAvailWidth = PR_MIN(aColAvailWidth, fixWidth);
      } else {
        aCellAvailWidth = fixWidth;
      }
    }
  }
}

nscoord
GetSpaceBetween(PRInt32       aPrevColIndex,
                PRInt32       aColIndex,
                PRInt32       aColSpan,
                nsTableFrame& aTableFrame,
                nscoord       aCellSpacingX,
                PRBool        aIsLeftToRight,
                PRBool        aCheckVisibility)
{
  nscoord space = 0;
  PRInt32 colX;
  if (aIsLeftToRight) {
    for (colX = aPrevColIndex + 1; aColIndex > colX; colX++) {
      PRBool isCollapsed = PR_FALSE;
      if (!aCheckVisibility) {
        space += aTableFrame.GetColumnWidth(colX);
      }
      else {
        nsTableColFrame* colFrame = aTableFrame.GetColFrame(colX);
        const nsStyleVisibility* colVis = colFrame->GetStyleVisibility();
        PRBool collapseCol = (NS_STYLE_VISIBILITY_COLLAPSE == colVis->mVisible);
        nsIFrame* cgFrame = colFrame->GetParent();
        const nsStyleVisibility* groupVis = cgFrame->GetStyleVisibility();
        PRBool collapseGroup = (NS_STYLE_VISIBILITY_COLLAPSE ==
                                groupVis->mVisible);
        isCollapsed = collapseCol || collapseGroup;
        if (!isCollapsed)
          space += aTableFrame.GetColumnWidth(colX);
      }
      if (!isCollapsed && (aTableFrame.GetNumCellsOriginatingInCol(colX) > 0)) {
        space += aCellSpacingX;
      }
    }
  } 
  else {
    PRInt32 lastCol = aColIndex + aColSpan - 1;
    for (colX = aPrevColIndex - 1; colX > lastCol; colX--) {
      PRBool isCollapsed = PR_FALSE;
      if (!aCheckVisibility) {
        space += aTableFrame.GetColumnWidth(colX);
      }
      else {
        nsTableColFrame* colFrame = aTableFrame.GetColFrame(colX);
        const nsStyleVisibility* colVis = colFrame->GetStyleVisibility();
        PRBool collapseCol = (NS_STYLE_VISIBILITY_COLLAPSE == colVis->mVisible);
        nsIFrame* cgFrame = colFrame->GetParent();
        const nsStyleVisibility* groupVis = cgFrame->GetStyleVisibility();
        PRBool collapseGroup = (NS_STYLE_VISIBILITY_COLLAPSE ==
                                groupVis->mVisible);
        isCollapsed = collapseCol || collapseGroup;
        if (!isCollapsed)
          space += aTableFrame.GetColumnWidth(colX);
      }
      if (!isCollapsed && (aTableFrame.GetNumCellsOriginatingInCol(colX) > 0)) {
        space += aCellSpacingX;
      }
    }
  }
  return space;
}

static nscoord
GetComputedWidth(const nsHTMLReflowState& aReflowState,
                 nsTableFrame&            aTableFrame)
{
  const nsHTMLReflowState* parentReflow = aReflowState.parentReflowState;
  nscoord computedWidth = 0;
  while (parentReflow) {
    if (parentReflow->frame == &aTableFrame) {
      computedWidth = parentReflow->mComputedWidth;
      break;
    }
    parentReflow = parentReflow->parentReflowState;
  }
  return computedWidth;
}

// subtract the heights of aRow's prev in flows from the unpaginated height
static
nscoord CalcHeightFromUnpaginatedHeight(nsPresContext*  aPresContext,
                                        nsTableRowFrame& aRow)
{
  nscoord height = 0;
  nsTableRowFrame* firstInFlow = (nsTableRowFrame*)aRow.GetFirstInFlow();
  if (!firstInFlow) ABORT1(0);
  if (firstInFlow->HasUnpaginatedHeight()) {
    height = firstInFlow->GetUnpaginatedHeight(aPresContext);
    for (nsIFrame* prevInFlow = aRow.GetPrevInFlow(); prevInFlow;
         prevInFlow = prevInFlow->GetPrevInFlow()) {
      height -= prevInFlow->GetSize().height;
    }
  }
  return PR_MAX(height, 0);
}

// Called for a dirty or resize reflow. Reflows all the existing table cell 
// frames unless aDirtyOnly is PR_TRUE in which case only reflow the dirty frames

NS_METHOD 
nsTableRowFrame::ReflowChildren(nsPresContext*          aPresContext,
                                nsHTMLReflowMetrics&     aDesiredSize,
                                const nsHTMLReflowState& aReflowState,
                                nsTableFrame&            aTableFrame,
                                nsReflowStatus&          aStatus,
                                PRBool                   aDirtyOnly)
{
  aStatus = NS_FRAME_COMPLETE;

  GET_PIXELS_TO_TWIPS(aPresContext, p2t);
  PRBool borderCollapse = (((nsTableFrame*)aTableFrame.GetFirstInFlow())->IsBorderCollapse());

  nsIFrame* tablePrevInFlow = aTableFrame.GetPrevInFlow();
  PRBool isPaginated = aPresContext->IsPaginated();

  nsresult rv = NS_OK;

  nscoord cellSpacingX = aTableFrame.GetCellSpacingX();
  PRInt32 cellColSpan = 1;  // must be defined here so it's set properly for non-cell kids
  
  nsTableIteration dir = (aReflowState.availableWidth == NS_UNCONSTRAINEDSIZE)
                         ? eTableLTR : eTableDIR;
  nsTableIterator iter(*this, dir);
  // remember the col index of the previous cell to handle rowspans into this row
  PRInt32 firstPrevColIndex = (iter.IsLeftToRight()) ? -1 : aTableFrame.GetColCount();
  PRInt32 prevColIndex  = firstPrevColIndex;
  nscoord x = 0; // running total of children x offset

  PRBool isAutoLayout = aTableFrame.IsAutoLayout();
  PRBool needToNotifyTable = PR_TRUE;
  nscoord paginatedHeight = 0;
  // If the incremental reflow command is a StyleChanged reflow and
  // it's target is the current frame, then make sure we send
  // StyleChange reflow reasons down to the children so that they
  // don't over-optimize their reflow.

  PRBool notifyStyleChange = PR_FALSE;
  if (eReflowReason_Incremental == aReflowState.reason) {
    nsHTMLReflowCommand* command = aReflowState.path->mReflowCommand;
    if (command) {
      nsReflowType type;
      command->GetType(type);
      if (eReflowType_StyleChanged == type) {
        notifyStyleChange = PR_TRUE;
      }
    }
  }
  // Reflow each of our existing cell frames
  nsIFrame* kidFrame = iter.First();
  while (kidFrame) {
    nsIAtom* frameType = kidFrame->GetType();

    // See if we should only reflow the dirty child frames
    PRBool doReflowChild = PR_TRUE;
    if (aDirtyOnly && ((kidFrame->GetStateBits() & NS_FRAME_IS_DIRTY) == 0)) {
      doReflowChild = PR_FALSE;
    }
    else if ((NS_UNCONSTRAINEDSIZE != aReflowState.availableHeight) && IS_TABLE_CELL(frameType)) {
      // We don't reflow a rowspan >1 cell here with a constrained height. 
      // That happens in nsTableRowGroupFrame::SplitSpanningCells.
      if (aTableFrame.GetEffectiveRowSpan((nsTableCellFrame&)*kidFrame) > 1) {
        doReflowChild = PR_FALSE;
      }
    }
    if (aReflowState.mFlags.mSpecialHeightReflow) {
      if (!isPaginated && (IS_TABLE_CELL(frameType) &&
                           !((nsTableCellFrame*)kidFrame)->NeedSpecialReflow())) {
        kidFrame = iter.Next(); 
        continue;
      }
    }

    // Reflow the child frame
    if (doReflowChild) {
      if (IS_TABLE_CELL(frameType)) {
        nsTableCellFrame* cellFrame = (nsTableCellFrame*)kidFrame;
        PRInt32 cellColIndex;
        cellFrame->GetColIndex(cellColIndex);
        cellColSpan = aTableFrame.GetEffectiveColSpan(*cellFrame);

        // If the adjacent cell is in a prior row (because of a rowspan) add in the space
        if ((iter.IsLeftToRight() && (prevColIndex != (cellColIndex - 1))) ||
            (!iter.IsLeftToRight() && (prevColIndex != cellColIndex + cellColSpan))) {
          x += GetSpaceBetween(prevColIndex, cellColIndex, cellColSpan, aTableFrame, 
                               cellSpacingX, iter.IsLeftToRight(), PR_FALSE);
        }
        // Calculate the available width for the table cell using the known column widths
        nscoord availColWidth, availCellWidth;
        CalcAvailWidth(aTableFrame, GetComputedWidth(aReflowState, aTableFrame), p2t,
                       *cellFrame, cellSpacingX, availColWidth, availCellWidth);

        // remember the rightmost (ltr) or leftmost (rtl) column this cell spans into
        prevColIndex = (iter.IsLeftToRight()) ? cellColIndex + (cellColSpan - 1) : cellColIndex;
        // always request MEW. Since we may turn MEW on for selected cells during incremental
        // reflow, we need to request MEW *now* so that those incremental reflows will be
        // able to build on existing MEW data in the children.
        nsHTMLReflowMetrics desiredSize(PR_TRUE);
  
        // If the avail width is not the same as last time we reflowed the cell or
        // the cell wants to be bigger than what was available last time or
        // it is a style change reflow or we are printing, then we must reflow the
        // cell. Otherwise we can skip the reflow.
        nsSize cellDesiredSize = cellFrame->GetDesiredSize();
        if ((availCellWidth != cellFrame->GetPriorAvailWidth())       ||
            (cellDesiredSize.width > cellFrame->GetPriorAvailWidth()) ||
            (eReflowReason_StyleChange == aReflowState.reason)        ||
            isPaginated                                               ||
            (cellFrame->NeedPass2Reflow() &&
             NS_UNCONSTRAINEDSIZE != aReflowState.availableWidth)     ||
            (aReflowState.mFlags.mSpecialHeightReflow && cellFrame->NeedSpecialReflow()) ||
            (!aReflowState.mFlags.mSpecialHeightReflow && cellFrame->HadSpecialReflow()) ||
            HasPctHeight()                                            ||
            notifyStyleChange) {
          // Reflow the cell to fit the available width, height
          nsSize  kidAvailSize(availColWidth, aReflowState.availableHeight);
          nsReflowReason reason = eReflowReason_Resize;
          PRBool cellToWatch = PR_FALSE;
          // If it's a dirty frame, then check whether it's the initial reflow
          if (kidFrame->GetStateBits() & NS_FRAME_FIRST_REFLOW) {
            reason = eReflowReason_Initial;
            cellToWatch = PR_TRUE;
          }
          else if (eReflowReason_StyleChange == aReflowState.reason) {
            reason = eReflowReason_StyleChange;
            cellToWatch = PR_TRUE;
          }
          else if (notifyStyleChange) {
            reason = eReflowReason_StyleChange;
            cellToWatch = PR_TRUE;
          }
          if (cellToWatch) {
            cellFrame->DidSetStyleContext(); // XXX check this
            if (!tablePrevInFlow && isAutoLayout) {
              // request the maximum width if availWidth is constrained
              // XXX we could just do this always, but blocks have some problems
              if (NS_UNCONSTRAINEDSIZE != availCellWidth) {
                desiredSize.mFlags |= NS_REFLOW_CALC_MAX_WIDTH; 
              }
            }
            else {
              cellToWatch = PR_FALSE;
            }
          }
  
          nscoord oldMaxWidth     = cellFrame->GetMaximumWidth();
          nscoord oldMaxElemWidth = cellFrame->GetPass1MaxElementWidth();

          // Reflow the child
          nsTableCellReflowState kidReflowState(aPresContext, aReflowState, 
                                                kidFrame, kidAvailSize, reason);
          InitChildReflowState(*aPresContext, kidAvailSize, borderCollapse, p2t, kidReflowState);

          nsReflowStatus status;
          rv = ReflowChild(kidFrame, aPresContext, desiredSize, kidReflowState,
                           x, 0, 0, status);

          if (cellToWatch) { 
            nscoord maxWidth = (NS_UNCONSTRAINEDSIZE == availCellWidth) 
                                ? desiredSize.width : desiredSize.mMaximumWidth;
            // save the max element width and max width
            if (desiredSize.mComputeMEW) {
              cellFrame->SetPass1MaxElementWidth(desiredSize.width, desiredSize.mMaxElementWidth);
              if (desiredSize.mMaxElementWidth > desiredSize.width) {
                desiredSize.width = desiredSize.mMaxElementWidth;
              }
            }
            cellFrame->SetMaximumWidth(maxWidth);
          }

          // allow the table to determine if/how the table needs to be rebalanced
          if (cellToWatch && needToNotifyTable) {
            needToNotifyTable = !aTableFrame.CellChangedWidth(*cellFrame, oldMaxWidth, oldMaxElemWidth);
          }

          // If any of the cells are not complete, then we're not complete
          if (NS_FRAME_IS_NOT_COMPLETE(status)) {
            aStatus = NS_FRAME_NOT_COMPLETE;
          }
        }
        else { 
          desiredSize.width = cellDesiredSize.width;
          desiredSize.height = cellDesiredSize.height;
          nsRect *overflowArea =
            cellFrame->GetOverflowAreaProperty();
          if (overflowArea)
            desiredSize.mOverflowArea = *overflowArea;
          else
            desiredSize.mOverflowArea.SetRect(0, 0, cellDesiredSize.width,
                                              cellDesiredSize.height);
          
          // if we are in a floated table, our position is not yet established, so we cannot reposition our views
          // the containing glock will do this for us after positioning the table
          if (!aTableFrame.GetStyleDisplay()->IsFloating()) {
            // Because we may have moved the frame we need to make sure any views are
            // positioned properly. We have to do this, because any one of our parent
            // frames could have moved and we have no way of knowing...
            nsTableFrame::RePositionViews(kidFrame);
          }
        }
        
        if (NS_UNCONSTRAINEDSIZE == aReflowState.availableHeight) {
          if (!GetPrevInFlow()) {
            // Calculate the cell's actual size given its pass2 size. This function
            // takes into account the specified height (in the style), and any special
            // logic needed for backwards compatibility
            CalculateCellActualSize(kidFrame, desiredSize.width, 
                                    desiredSize.height, availCellWidth);
          }
          // height may have changed, adjust descent to absorb any excess difference
          nscoord ascent;
          if (!kidFrame->GetFirstChild(nsnull)->GetFirstChild(nsnull))
            ascent = desiredSize.height;
          else
            ascent = ((nsTableCellFrame *)kidFrame)->GetDesiredAscent();
          nscoord descent = desiredSize.height - ascent;
          UpdateHeight(desiredSize.height, ascent, descent, &aTableFrame, cellFrame);
        }
        else {
          paginatedHeight = PR_MAX(paginatedHeight, desiredSize.height);
          PRInt32 rowSpan = aTableFrame.GetEffectiveRowSpan((nsTableCellFrame&)*kidFrame);
          if (1 == rowSpan) {
            SetContentHeight(paginatedHeight);
          }
        }

        // Place the child
        if (NS_UNCONSTRAINEDSIZE != availColWidth) {
          desiredSize.width = PR_MAX(availCellWidth, availColWidth);
        }

        FinishReflowChild(kidFrame, aPresContext, nsnull, desiredSize, x, 0, 0);
                
        x += desiredSize.width;  
      }
      else {// it's an unknown frame type, give it a generic reflow and ignore the results
        nsTableCellReflowState kidReflowState(aPresContext, aReflowState,
                                              kidFrame, nsSize(0,0), eReflowReason_Resize);
        InitChildReflowState(*aPresContext, nsSize(0,0), PR_FALSE, p2t, kidReflowState);
        nsHTMLReflowMetrics desiredSize(PR_FALSE);
        nsReflowStatus  status;
        ReflowChild(kidFrame, aPresContext, desiredSize, kidReflowState, 0, 0, 0, status);
        kidFrame->DidReflow(aPresContext, nsnull, NS_FRAME_REFLOW_FINISHED);
      }
    }
    else if (IS_TABLE_CELL(frameType)) {
      // we need to account for the cell's width even if it isn't reflowed
      x += kidFrame->GetSize().width;
    }
    ConsiderChildOverflow(aDesiredSize.mOverflowArea, kidFrame);
    kidFrame = iter.Next(); // Get the next child
    x += cellSpacingX;
  }

  // just set our width to what was available. The table will calculate the width and not use our value.
  aDesiredSize.width = aReflowState.availableWidth;

  if (aReflowState.mFlags.mSpecialHeightReflow) {
    aDesiredSize.height = mRect.height;
  }
  else if (NS_UNCONSTRAINEDSIZE == aReflowState.availableHeight) {
    aDesiredSize.height = CalcHeight(aReflowState);
    if (GetPrevInFlow()) {
      nscoord height = CalcHeightFromUnpaginatedHeight(aPresContext, *this);
      aDesiredSize.height = PR_MAX(aDesiredSize.height, height);
    }
    else {
      if (isPaginated && HasStyleHeight()) {
        // set the unpaginated height so next in flows can try to honor it
        SetHasUnpaginatedHeight(PR_TRUE);
        SetUnpaginatedHeight(aPresContext, aDesiredSize.height);
      }
      if (isPaginated && HasUnpaginatedHeight()) {
        aDesiredSize.height = PR_MAX(aDesiredSize.height, GetUnpaginatedHeight(aPresContext));
      }
    }
  }
  else { // constrained height, paginated
    aDesiredSize.height = paginatedHeight;
    if (aDesiredSize.height <= aReflowState.availableHeight) {
      nscoord height = CalcHeightFromUnpaginatedHeight(aPresContext, *this);
      aDesiredSize.height = PR_MAX(aDesiredSize.height, height);
      aDesiredSize.height = PR_MIN(aDesiredSize.height, aReflowState.availableHeight);
    }
  }
  nsRect rowRect(0, 0, aDesiredSize.width, aDesiredSize.height);
  aDesiredSize.mOverflowArea.UnionRect(aDesiredSize.mOverflowArea, rowRect);
  FinishAndStoreOverflow(&aDesiredSize);
  return rv;
}

NS_METHOD nsTableRowFrame::IncrementalReflow(nsPresContext*          aPresContext,
                                             nsHTMLReflowMetrics&     aDesiredSize,
                                             const nsHTMLReflowState& aReflowState,
                                             nsTableFrame&            aTableFrame,
                                             nsReflowStatus&          aStatus)
{
  CalcHeight(aReflowState); // need to recalculate it based on last reflow sizes
 
  // the row is a target if its path has a reflow command
  nsHTMLReflowCommand* command = aReflowState.path->mReflowCommand;
  if (command)
    IR_TargetIsMe(aPresContext, aDesiredSize, aReflowState, aTableFrame, aStatus);

  // see if the chidren are targets as well
  nsReflowPath::iterator iter = aReflowState.path->FirstChild();
  nsReflowPath::iterator end  = aReflowState.path->EndChildren();
  for (; iter != end; ++iter)
    IR_TargetIsChild(aPresContext, aDesiredSize, aReflowState, aTableFrame, aStatus, *iter);

  return NS_OK;
}

NS_METHOD 
nsTableRowFrame::IR_TargetIsMe(nsPresContext*          aPresContext,
                               nsHTMLReflowMetrics&     aDesiredSize,
                               const nsHTMLReflowState& aReflowState,
                               nsTableFrame&            aTableFrame,
                               nsReflowStatus&          aStatus)
{
  nsresult rv = NS_FRAME_COMPLETE;

  nsReflowType type;
  aReflowState.path->mReflowCommand->GetType(type);
  switch (type) {
    case eReflowType_ReflowDirty: 
      // Reflow the dirty child frames. Typically this is newly added frames.
      rv = ReflowChildren(aPresContext, aDesiredSize, aReflowState, aTableFrame, aStatus, PR_TRUE);
      break;
    case eReflowType_StyleChanged :
      rv = IR_StyleChanged(aPresContext, aDesiredSize, aReflowState, aTableFrame, aStatus);
      break;
    case eReflowType_ContentChanged :
      NS_ASSERTION(PR_FALSE, "illegal reflow type: ContentChanged");
      rv = NS_ERROR_ILLEGAL_VALUE;
      break;
    default:
      NS_NOTYETIMPLEMENTED("unexpected reflow command type");
      rv = NS_ERROR_NOT_IMPLEMENTED;
      break;
  }

  return rv;
}

NS_METHOD 
nsTableRowFrame::IR_StyleChanged(nsPresContext*          aPresContext,
                                 nsHTMLReflowMetrics&     aDesiredSize,
                                 const nsHTMLReflowState& aReflowState,
                                 nsTableFrame&            aTableFrame,
                                 nsReflowStatus&          aStatus)
{
  nsresult rv = NS_OK;
  // we presume that all the easy optimizations were done in the nsHTMLStyleSheet before we were called here
  // XXX: we can optimize this when we know which style attribute changed
  aTableFrame.SetNeedStrategyInit(PR_TRUE);
  rv = ReflowChildren(aPresContext, aDesiredSize, aReflowState, aTableFrame, aStatus, PR_FALSE);
  return rv;
}

NS_METHOD 
nsTableRowFrame::IR_TargetIsChild(nsPresContext*          aPresContext,
                                  nsHTMLReflowMetrics&     aDesiredSize,
                                  const nsHTMLReflowState& aReflowState,
                                  nsTableFrame&            aTableFrame,
                                  nsReflowStatus&          aStatus,
                                  nsIFrame*                aNextFrame)

{
  if (!aNextFrame) return NS_ERROR_NULL_POINTER;
  nsresult rv = NS_OK;

  GET_PIXELS_TO_TWIPS(aPresContext, p2t);
  PRBool isAutoLayout = aTableFrame.IsAutoLayout();
  const nsStyleDisplay* childDisplay = aNextFrame->GetStyleDisplay();
  if (NS_STYLE_DISPLAY_TABLE_CELL == childDisplay->mDisplay) {
    nsTableCellFrame* cellFrame = (nsTableCellFrame*)aNextFrame;
    // Get the x coord of the cell
    nsPoint cellOrigin = cellFrame->GetPosition();

    // At this point, we know the column widths. Compute the cell available width
    PRInt32 cellColIndex;
    cellFrame->GetColIndex(cellColIndex);
    nscoord cellSpacingX = aTableFrame.GetCellSpacingX();

    nscoord colAvailWidth, cellAvailWidth;
    CalcAvailWidth(aTableFrame, GetComputedWidth(aReflowState, aTableFrame), p2t,
                   *cellFrame, cellSpacingX, colAvailWidth, cellAvailWidth);

    // Always let the cell be as high as it wants. We ignore the height that's
    // passed in and always place the entire row. Let the row group decide
    // whether we fit or wehther the entire row is pushed
    nsSize  cellAvailSize(cellAvailWidth, NS_UNCONSTRAINEDSIZE);

    // Pass along the reflow command
    // Unless this is a fixed-layout table, then have the cell incrementally
    // update its maximum width. 
    nsHTMLReflowMetrics cellMet(PR_TRUE,
                                isAutoLayout ? NS_REFLOW_CALC_MAX_WIDTH : 0);
    GET_PIXELS_TO_TWIPS(aPresContext, p2t);
    nsTableCellReflowState kidRS(aPresContext, aReflowState, aNextFrame, cellAvailSize, 
                                 aReflowState.reason);
    // If the table will intialize the strategy (and balance) or balance, make the computed 
    // width unconstrained. This avoids having the cell block compute a bogus max width 
    // which will bias the balancing. Leave the avail width alone, since it is a best guess.
    // After the table balances, the cell will get reflowed with the correct computed width.
    PRBool resetComputedWidth = aTableFrame.NeedStrategyInit() || aTableFrame.NeedStrategyBalance();
    if (resetComputedWidth)
      cellFrame->SetNeedPass2Reflow(PR_TRUE);
    InitChildReflowState(*aPresContext, cellAvailSize, aTableFrame.IsBorderCollapse(), 
                         p2t, kidRS, resetComputedWidth); 

    // Remember the current desired size, we'll need it later
    nscoord oldCellMinWidth     = cellFrame->GetPass1MaxElementWidth();
    nscoord oldCellMaximumWidth = cellFrame->GetMaximumWidth();
    nsSize  oldCellDesSize      = cellFrame->GetDesiredSize();
    nscoord oldCellDesAscent    = cellFrame->GetDesiredAscent();
    nscoord oldCellDesDescent   = oldCellDesSize.height - oldCellDesAscent;
    
    // Reflow the cell passing it the incremental reflow command. We can't pass
    // in a max width of NS_UNCONSTRAINEDSIZE, because the max width must match
    // the width of the previous reflow...
    rv = ReflowChild(aNextFrame, aPresContext, cellMet, kidRS,
                     cellOrigin.x, 0, 0, aStatus);
    nsSize initCellDesSize(cellMet.width, cellMet.height);
    nscoord initCellDesAscent = cellMet.ascent;
    nscoord initCellDesDescent = cellMet.descent;
    
    // cache the max-elem and maximum widths
    cellFrame->SetPass1MaxElementWidth(cellMet.width, cellMet.mMaxElementWidth);
    cellFrame->SetMaximumWidth(cellMet.mMaximumWidth);

    // Calculate the cell's actual size given its pass2 size. This function
    // takes into account the specified height (in the style), and any special
    // logic needed for backwards compatibility
    CalculateCellActualSize(aNextFrame, cellMet.width, cellMet.height, cellAvailWidth);

    // height may have changed, adjust descent to absorb any excess difference
    if (!aNextFrame->GetFirstChild(nsnull)->GetFirstChild(nsnull))
      cellMet.ascent = cellMet.height;
    cellMet.descent = cellMet.height - cellMet.ascent;

    // if the cell got shorter and it may have been the tallest, recalc the tallest cell
    PRBool tallestCellGotShorter = PR_FALSE;
    PRBool hasVerticalAlignBaseline = cellFrame->HasVerticalAlignBaseline();
    if (!hasVerticalAlignBaseline) { 
      // only the height matters
      tallestCellGotShorter = 
        TallestCellGotShorter(oldCellDesSize.height, cellMet.height, GetHeight());
    }
    else {
      // the ascent matters
      tallestCellGotShorter = 
        TallestCellGotShorter(oldCellDesAscent, cellMet.ascent, mMaxCellAscent);
      // the descent of cells without rowspan also matters
      if (!tallestCellGotShorter) {
        PRInt32 rowSpan = aTableFrame.GetEffectiveRowSpan(*cellFrame);
        if (rowSpan == 1) {
         tallestCellGotShorter = 
           TallestCellGotShorter(oldCellDesDescent, cellMet.descent, mMaxCellDescent);
        }
      }
    }
    if (tallestCellGotShorter) {
      CalcHeight(aReflowState);
    }
    else {
      UpdateHeight(cellMet.height, cellMet.ascent, cellMet.descent, &aTableFrame, cellFrame);
    }

    // if the cell's desired size didn't changed, our height is unchanged
    aDesiredSize.mNothingChanged = PR_FALSE;
    PRInt32 rowSpan = aTableFrame.GetEffectiveRowSpan(*cellFrame);
    if ((initCellDesSize.width  == oldCellDesSize.width)  &&
        (initCellDesSize.height == oldCellDesSize.height) &&
        (oldCellMaximumWidth    == cellMet.mMaximumWidth)) {
      if (!hasVerticalAlignBaseline) { // only the cell's height matters
        aDesiredSize.mNothingChanged = PR_TRUE;
      }
      else { // cell's ascent and cell's descent matter
        if (initCellDesAscent == oldCellDesAscent) {
          if ((rowSpan == 1) && (initCellDesDescent == oldCellDesDescent)) {
            aDesiredSize.mNothingChanged = PR_TRUE;
          }
        }
      }
    }
    aDesiredSize.height = (aDesiredSize.mNothingChanged) ? mRect.height : GetHeight();
    if (1 == rowSpan) {
      cellMet.height = aDesiredSize.height;
    }
    else {
      nscoord heightOfRows = aDesiredSize.height + GetHeightOfRowsSpannedBelowFirst(*cellFrame, aTableFrame); 
      cellMet.height = PR_MAX(cellMet.height, heightOfRows); 
      // XXX need to check what happens when this height differs from height of the cell's previous mRect.height
    }

    // Now place the child
    cellMet.width = colAvailWidth;

    FinishReflowChild(aNextFrame, aPresContext, nsnull, cellMet, cellOrigin.x, 0, 0);

    // Notify the table if the cell width changed so it can decide whether to rebalance
    if (!aDesiredSize.mNothingChanged) {
      aTableFrame.CellChangedWidth(*cellFrame, oldCellMinWidth, oldCellMaximumWidth); 
    } 

    // Return our desired size. Note that our desired width is just whatever width
    // we were given by the row group frame
    aDesiredSize.width  = aReflowState.availableWidth;
    if (!aDesiredSize.mNothingChanged) {
      if (aDesiredSize.height == mRect.height) { // our height didn't change
        cellFrame->VerticallyAlignChild(aReflowState, mMaxCellAscent);
        nsRect dirtyRect = cellFrame->GetRect();
        dirtyRect.height = mRect.height;
        ConsiderChildOverflow(aDesiredSize.mOverflowArea, cellFrame);
        dirtyRect.UnionRect(dirtyRect, aDesiredSize.mOverflowArea);
        Invalidate(dirtyRect);
      }
    }
    else { // we dont realign vertical but we need to update the overflow area
      nsIFrame* cellKidFrame = cellFrame->GetFirstChild(nsnull);
      if (cellKidFrame) {
        // XXX This is bogus. The code above changes the cell width
        // and height but leaves the overflow area alone. How can that
        // be right?

        // Make sure the overflow area includes the width and height, in any case.
        cellMet.mOverflowArea.UnionRect(cellMet.mOverflowArea,
                                        nsRect(0, 0, cellMet.width, cellMet.height));
        cellFrame->ConsiderChildOverflow(cellMet.mOverflowArea, cellKidFrame);
        cellFrame->FinishAndStoreOverflow(&cellMet);
        if (cellFrame->HasView()) {
          nsContainerFrame::SyncFrameViewAfterReflow(aPresContext, cellFrame, cellFrame->GetView(), &cellMet.mOverflowArea, 0);
        }
      }
    }
  }
  else
  { // pass reflow to unknown frame child
    // aDesiredSize does not change
  }
  
  // recover the overflow area
  aDesiredSize.mOverflowArea = nsRect(0, 0, aDesiredSize.width, aDesiredSize.height);
  for (nsIFrame* cell = mFrames.FirstChild(); cell; cell = cell->GetNextSibling()) {
    ConsiderChildOverflow(aDesiredSize.mOverflowArea, cell);
  }
  FinishAndStoreOverflow(&aDesiredSize);
  // When returning whether we're complete we need to look at each of our cell
  // frames. If any of them has a continuing frame, then we're not complete. We
  // can't just return the status of the cell frame we just reflowed...
  aStatus = NS_FRAME_COMPLETE;
  if (GetNextInFlow()) {
    for (nsIFrame* cell = mFrames.FirstChild(); cell;
         cell = cell->GetNextSibling()) {
      nsIFrame* contFrame = cell->GetNextInFlow();
      if (contFrame) {
        aStatus =  NS_FRAME_NOT_COMPLETE;
        break;
      }
    }
  }
  return rv;
}


/** Layout the entire row.
  * This method stacks cells horizontally according to HTML 4.0 rules.
  */
NS_METHOD
nsTableRowFrame::Reflow(nsPresContext*          aPresContext,
                        nsHTMLReflowMetrics&     aDesiredSize,
                        const nsHTMLReflowState& aReflowState,
                        nsReflowStatus&          aStatus)
{
  DO_GLOBAL_REFLOW_COUNT("nsTableRowFrame", aReflowState.reason);
  DISPLAY_REFLOW(aPresContext, this, aReflowState, aDesiredSize, aStatus);
#if defined DEBUG_TABLE_REFLOW_TIMING
  nsTableFrame::DebugReflow(this, (nsHTMLReflowState&)aReflowState);
#endif
  nsresult rv = NS_OK;

  nsTableFrame* tableFrame = nsTableFrame::GetTableFrame(this);
  if (!tableFrame)
    return NS_ERROR_NULL_POINTER;

  const nsStyleVisibility* rowVis = GetStyleVisibility();
  PRBool collapseRow = (NS_STYLE_VISIBILITY_COLLAPSE == rowVis->mVisible);
  if (collapseRow) {
    tableFrame->SetNeedToCollapse(PR_TRUE);
  }

  // see if a special height reflow needs to occur due to having a pct height
  if (!NeedSpecialReflow()) 
    nsTableFrame::CheckRequestSpecialHeightReflow(aReflowState);

  switch (aReflowState.reason) {
  case eReflowReason_Initial:
    rv = ReflowChildren(aPresContext, aDesiredSize, aReflowState, *tableFrame, aStatus, PR_FALSE);
    break;

  case eReflowReason_Resize:
  case eReflowReason_StyleChange: 
  case eReflowReason_Dirty:
    rv = ReflowChildren(aPresContext, aDesiredSize, aReflowState, *tableFrame, aStatus, PR_FALSE);
    break;

  case eReflowReason_Incremental:
    rv = IncrementalReflow(aPresContext, aDesiredSize, aReflowState, *tableFrame, aStatus);
    break;
  default:
    NS_ASSERTION(PR_FALSE , "we should handle this reflow reason");
    rv = NS_ERROR_NOT_IMPLEMENTED;
    break;
  }

  // just set our width to what was available. The table will calculate the width and not use our value.
  aDesiredSize.width = aReflowState.availableWidth;

  if (aReflowState.mFlags.mSpecialHeightReflow) {
    SetNeedSpecialReflow(PR_FALSE);
  }

#if defined DEBUG_TABLE_REFLOW_TIMING
  nsTableFrame::DebugReflow(this, (nsHTMLReflowState&)aReflowState, &aDesiredSize, aStatus);
#endif
  NS_FRAME_SET_TRUNCATION(aStatus, aReflowState, aDesiredSize);
  return rv;
}

/**
 * This function is called by the row group frame's SplitRowGroup() code when
 * pushing a row frame that has cell frames that span into it. The cell frame
 * should be reflowed with the specified height
 */
nscoord 
nsTableRowFrame::ReflowCellFrame(nsPresContext*          aPresContext,
                                 const nsHTMLReflowState& aReflowState,
                                 nsTableCellFrame*        aCellFrame,
                                 nscoord                  aAvailableHeight,
                                 nsReflowStatus&          aStatus)
{
  nsTableFrame* tableFrame = nsTableFrame::GetTableFrame(this);
  if (!tableFrame)
    ABORT1(NS_ERROR_NULL_POINTER);

  // Reflow the cell frame with the specified height. Use the existing width
  nsSize cellSize = aCellFrame->GetSize();
  
  nsSize  availSize(cellSize.width, aAvailableHeight);
  PRBool borderCollapse = ((nsTableFrame*)tableFrame->GetFirstInFlow())->IsBorderCollapse();
  GET_PIXELS_TO_TWIPS(aPresContext, p2t);
  nsTableCellReflowState cellReflowState(aPresContext, aReflowState, aCellFrame, availSize,
                                         eReflowReason_Resize);
  InitChildReflowState(*aPresContext, availSize, borderCollapse, p2t, cellReflowState);

  nsHTMLReflowMetrics desiredSize(PR_FALSE);

  ReflowChild(aCellFrame, aPresContext, desiredSize, cellReflowState,
              0, 0, NS_FRAME_NO_MOVE_FRAME, aStatus);
  PRBool fullyComplete = NS_FRAME_IS_COMPLETE(aStatus) && !NS_FRAME_IS_TRUNCATED(aStatus);

  aCellFrame->SetSize(
    nsSize(cellSize.width, fullyComplete ? aAvailableHeight : desiredSize.height));

  // XXX What happens if this cell has 'vertical-align: baseline' ?
  // XXX Why is it assumed that the cell's ascent hasn't changed ?
  if (fullyComplete) {
    aCellFrame->VerticallyAlignChild(aReflowState, mMaxCellAscent);
  }
  aCellFrame->DidReflow(aPresContext, nsnull, NS_FRAME_REFLOW_FINISHED);

  return desiredSize.height;
}

nscoord
nsTableRowFrame::CollapseRowIfNecessary(nscoord aRowOffset,
                                        nscoord aWidth,
                                        PRBool  aCollapseGroup,
                                        PRBool& aDidCollapse)
{
  const nsStyleVisibility* rowVis = GetStyleVisibility();
  PRBool collapseRow = (NS_STYLE_VISIBILITY_COLLAPSE == rowVis->mVisible);
  nsTableFrame* tableFrame = nsTableFrame::GetTableFrame(this);
  if (!tableFrame)
      return 0;
  if (collapseRow) {
    tableFrame->SetNeedToCollapse(PR_TRUE);
  }

  nsRect rowRect = GetRect();
  rowRect.y -= aRowOffset;
  rowRect.width  = aWidth;
  nsRect overflowArea(0, 0, 0, 0);
  nscoord shift = 0;
  nscoord cellSpacingX = tableFrame->GetCellSpacingX();
  nscoord cellSpacingY = tableFrame->GetCellSpacingY();

  if (aCollapseGroup || collapseRow) {
    nsTableCellFrame* cellFrame = GetFirstCell();
    aDidCollapse = PR_TRUE;
    shift = rowRect.height + cellSpacingY;
    while (cellFrame) {
      nsRect cRect = cellFrame->GetRect();
      cRect.height = 0;
      cellFrame->SetRect(cRect);
      cellFrame = cellFrame->GetNextCell();
    }
    rowRect.height = 0;
  }
  else { // row is not collapsed
    nsTableIterator iter(*this, eTableDIR);
    // remember the col index of the previous cell to handle rowspans into this
    // row
    PRInt32 firstPrevColIndex = (iter.IsLeftToRight()) ? -1 :
                                tableFrame->GetColCount();
    PRInt32 prevColIndex  = firstPrevColIndex;
    nscoord x = 0; // running total of children x offset

    PRInt32 colIncrement = iter.IsLeftToRight() ? 1 : -1;

    //nscoord x = cellSpacingX;

    nsIFrame* kidFrame = iter.First();
    while (kidFrame) {
      nsIAtom* frameType = kidFrame->GetType();
      if (IS_TABLE_CELL(frameType)) {
        nsTableCellFrame* cellFrame = (nsTableCellFrame*)kidFrame;
        PRInt32 cellColIndex;
        cellFrame->GetColIndex(cellColIndex);
        PRInt32 cellColSpan = tableFrame->GetEffectiveColSpan(*cellFrame);

        // If the adjacent cell is in a prior row (because of a rowspan) add in
        // the space
        if ((iter.IsLeftToRight() && (prevColIndex != (cellColIndex - 1))) ||
            (!iter.IsLeftToRight() &&
             (prevColIndex != cellColIndex + cellColSpan))) {
          x += GetSpaceBetween(prevColIndex, cellColIndex, cellColSpan,
                               *tableFrame, cellSpacingX, iter.IsLeftToRight(),
                               PR_TRUE);
        }
        nsRect cRect(x, 0, 0,rowRect.height);

        // remember the rightmost (ltr) or leftmost (rtl) column this cell
        // spans into
        prevColIndex = (iter.IsLeftToRight()) ?
                       cellColIndex + (cellColSpan - 1) : cellColIndex;
        PRInt32 startIndex = (iter.IsLeftToRight()) ?
                             cellColIndex : cellColIndex + (cellColSpan - 1);
        PRInt32 actualColSpan = cellColSpan;
        PRBool isVisible = PR_FALSE;
        for (PRInt32 colX = startIndex; actualColSpan > 0;
             colX += colIncrement, actualColSpan--) {

          nsTableColFrame* colFrame = tableFrame->GetColFrame(colX);
          const nsStyleVisibility* colVis = colFrame->GetStyleVisibility();
          PRBool collapseCol = (NS_STYLE_VISIBILITY_COLLAPSE ==
                                colVis->mVisible);
          nsIFrame* cgFrame = colFrame->GetParent();
          const nsStyleVisibility* groupVis = cgFrame->GetStyleVisibility();
          PRBool collapseGroup = (NS_STYLE_VISIBILITY_COLLAPSE ==
                                  groupVis->mVisible);
          PRBool isCollapsed = collapseCol || collapseGroup;
          if (isCollapsed) {
            tableFrame->SetNeedToCollapse(PR_TRUE);
          }
          else {
            cRect.width += tableFrame->GetColumnWidth(colX);
            isVisible = PR_TRUE;
          }
          if (!isCollapsed &&  (actualColSpan > 1)) {
            nsTableColFrame* nextColFrame =
              tableFrame->GetColFrame(colX + colIncrement);
            const nsStyleVisibility* nextColVis =
              nextColFrame->GetStyleVisibility();
            if ( (NS_STYLE_VISIBILITY_COLLAPSE != nextColVis->mVisible) &&
                (tableFrame->GetNumCellsOriginatingInCol(colX + colIncrement) > 0)) {
              cRect.width += cellSpacingX;
            }
          }
        }
        x += cRect.width;
        if (isVisible)
          x += cellSpacingX;
        PRInt32 actualRowSpan = tableFrame->GetEffectiveRowSpan(*cellFrame);
        nsTableRowFrame* rowFrame = GetNextRow();
        for (actualRowSpan--; actualRowSpan > 0 && rowFrame; actualRowSpan--) {
          const nsStyleVisibility* nextRowVis = rowFrame->GetStyleVisibility();
          PRBool collapseNextRow = (NS_STYLE_VISIBILITY_COLLAPSE ==
                                    nextRowVis->mVisible);
          if (!collapseNextRow) {
            nsRect nextRect = rowFrame->GetRect();
            cRect.height += nextRect.height + cellSpacingY;
          }
          rowFrame = rowFrame->GetNextRow();
        }
        cellFrame->SetRect(cRect);
        nsRect cellOverflow = nsRect(0, 0, cRect.width, cRect.height);
        cellFrame->FinishAndStoreOverflow(&cellOverflow, nsSize(cRect.width,
                                              cRect.height));
        nsTableFrame::RePositionViews(cellFrame);
        ConsiderChildOverflow(overflowArea, cellFrame);
        
      }
      kidFrame = iter.Next(); // Get the next child
    }
  }
  SetRect(rowRect);
  overflowArea.UnionRect(nsRect(0,0,rowRect.width, rowRect.height),
                         overflowArea);
  FinishAndStoreOverflow(&overflowArea, nsSize(rowRect.width,
                                              rowRect.height));
  nsTableFrame::RePositionViews(this);
  return shift;
}


/**
 * These 3 functions are called by the row group frame's SplitRowGroup() code when
 * it creates a continuing cell frame and wants to insert it into the row's child list
 */
void 
nsTableRowFrame::InsertCellFrame(nsTableCellFrame* aFrame,
                                 nsTableCellFrame* aPrevSibling)
{
  mFrames.InsertFrame(nsnull, aPrevSibling, aFrame);
  aFrame->SetParent(this);
}

void 
nsTableRowFrame::InsertCellFrame(nsTableCellFrame* aFrame,
                                 PRInt32           aColIndex)
{
  // Find the cell frame where col index < aColIndex
  nsTableCellFrame* priorCell = nsnull;
  for (nsIFrame* child = mFrames.FirstChild(); child;
       child = child->GetNextSibling()) {
    if (!IS_TABLE_CELL(child->GetType())) {
      nsTableCellFrame* cellFrame = (nsTableCellFrame*)child;
      PRInt32 colIndex;
      cellFrame->GetColIndex(colIndex);
      if (colIndex < aColIndex) {
        priorCell = cellFrame;
      }
      else break;
    }
  }
  InsertCellFrame(aFrame, priorCell);
}

void 
nsTableRowFrame::RemoveCellFrame(nsTableCellFrame* aFrame)
{
  if (!mFrames.RemoveFrame(aFrame))
    NS_ASSERTION(PR_FALSE, "frame not in list");
}

nsIAtom*
nsTableRowFrame::GetType() const
{
  return nsLayoutAtoms::tableRowFrame;
}

nsTableRowFrame*  
nsTableRowFrame::GetNextRow() const
{
  nsIFrame* childFrame = GetNextSibling();
  while (childFrame) {
    if (nsLayoutAtoms::tableRowFrame == childFrame->GetType()) {
	  NS_ASSERTION(NS_STYLE_DISPLAY_TABLE_ROW == childFrame->GetStyleDisplay()->mDisplay, "wrong display type on rowframe");
      return (nsTableRowFrame*)childFrame;
    }
    childFrame = childFrame->GetNextSibling();
  }
  return nsnull;
}

void 
nsTableRowFrame::SetUnpaginatedHeight(nsPresContext* aPresContext,
                                      nscoord         aValue)
{
  NS_ASSERTION(!GetPrevInFlow(), "program error");
  // Get the property 
  nscoord* value = (nscoord*)nsTableFrame::GetProperty(this, nsLayoutAtoms::rowUnpaginatedHeightProperty, PR_TRUE);
  if (value) {
    *value = aValue;
  }
}

nscoord
nsTableRowFrame::GetUnpaginatedHeight(nsPresContext* aPresContext)
{
  // See if the property is set
  nscoord* value = (nscoord*)nsTableFrame::GetProperty(GetFirstInFlow(), nsLayoutAtoms::rowUnpaginatedHeightProperty);
  if (value) 
    return *value;
  else 
    return 0;
}

void nsTableRowFrame::SetContinuousBCBorderWidth(PRUint8     aForSide,
                                                 BCPixelSize aPixelValue)
{
  switch (aForSide) {
    case NS_SIDE_RIGHT:
      mRightContBorderWidth = aPixelValue;
      return;
    case NS_SIDE_TOP:
      mTopContBorderWidth = aPixelValue;
      return;
    case NS_SIDE_LEFT:
      mLeftContBorderWidth = aPixelValue;
      return;
    default:
      NS_ERROR("invalid NS_SIDE arg");
  }
}

/* ----- global methods ----- */

nsIFrame* 
NS_NewTableRowFrame(nsIPresShell* aPresShell, nsStyleContext* aContext)
{
  return new (aPresShell) nsTableRowFrame(aContext);
}

#ifdef DEBUG
NS_IMETHODIMP
nsTableRowFrame::GetFrameName(nsAString& aResult) const
{
  return MakeFrameName(NS_LITERAL_STRING("TableRow"), aResult);
}
#endif
