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
#ifndef nsTableFrame_h__
#define nsTableFrame_h__

#include "nscore.h"
#include "nsVoidArray.h"
#include "nsHTMLContainerFrame.h"
#include "nsStyleCoord.h"
#include "nsStyleConsts.h"
#include "nsITableLayout.h"
#include "nsTableColFrame.h"
#include "nsTableColGroupFrame.h"
#include "nsCellMap.h"
#include "nsGkAtoms.h"

class nsTableCellFrame;
class nsTableColFrame;
class nsTableRowGroupFrame;
class nsTableRowFrame;
class nsTableColGroupFrame;
class nsITableLayoutStrategy;
class nsStyleContext;

struct nsTableReflowState;
struct nsStylePosition;

/**
 * Child list name indices
 * @see #GetAdditionalChildListName()
 */
#define NS_TABLE_FRAME_COLGROUP_LIST_INDEX 0
#define NS_TABLE_FRAME_OVERFLOW_LIST_INDEX 1
#define NS_TABLE_FRAME_LAST_LIST_INDEX    NS_TABLE_FRAME_OVERFLOW_LIST_INDEX

static inline PRBool IS_TABLE_CELL(nsIAtom* frameType) {
  return nsGkAtoms::tableCellFrame == frameType ||
    nsGkAtoms::bcTableCellFrame == frameType;
}

/* ============================================================================ */

/** nsTableFrame maps the inner portion of a table (everything except captions.)
  * Used as a pseudo-frame within nsTableOuterFrame, it may also be used
  * stand-alone as the top-level frame.
  *
  * The flowed child list contains row group frames. There is also an additional
  * named child list:
  * - "ColGroup-list" which contains the col group frames
  *
  * @see nsGkAtoms::colGroupList
  */
class nsTableFrame : public nsHTMLContainerFrame, public nsITableLayout
{
public:

  // nsISupports
  NS_DECL_ISUPPORTS_INHERITED

  /** nsTableOuterFrame has intimate knowledge of the inner table frame */
  friend class nsTableOuterFrame;

  /** instantiate a new instance of nsTableRowFrame.
    * @param aPresShell the pres shell for this frame
    *
    * @return           the frame that was created
    */
  friend nsIFrame* NS_NewTableFrame(nsIPresShell* aPresShell, nsStyleContext* aContext);

  /** sets defaults for table-specific style.
    * @see nsIFrame::Init 
    */
  NS_IMETHOD Init(nsIContent*      aContent,
                  nsIFrame*        aParent,
                  nsIFrame*        aPrevInFlow);


  static void* GetProperty(nsIFrame*            aFrame,
                           nsIAtom*             aPropertyName,
                           PRBool               aCreateIfNecessary = PR_FALSE);

  static float GetTwipsToPixels(nsPresContext* aPresContext);

  // Return true if aParentReflowState.frame or any of its ancestors within
  // the containing table have non-auto height. (e.g. pct or fixed height)
  static PRBool AncestorsHaveStyleHeight(const nsHTMLReflowState& aParentReflowState);

  // See if a special height reflow will occur due to having a pct height when
  // the pct height basis may not yet be valid.
  static void CheckRequestSpecialHeightReflow(const nsHTMLReflowState& aReflowState);

  // Notify the frame and its ancestors (up to the containing table) that a special
  // height reflow will occur. 
  static void RequestSpecialHeightReflow(const nsHTMLReflowState& aReflowState);

  virtual PRBool IsContainingBlock() const;

  static void RePositionViews(nsIFrame* aFrame);

  static PRBool PageBreakAfter(nsIFrame& aSourceFrame,
                               nsIFrame* aNextFrame);

  nsPoint GetFirstSectionOrigin(const nsHTMLReflowState& aReflowState) const;
  /*
   * Notification that aAttribute has changed for content inside a table (cell, row, etc)
   */
  void AttributeChangedFor(nsIFrame*       aFrame,
                           nsIContent*     aContent, 
                           nsIAtom*        aAttribute); 

  /** @see nsIFrame::Destroy */
  virtual void Destroy();

  NS_IMETHOD AppendFrames(nsIAtom*        aListName,
                          nsIFrame*       aFrameList);
  NS_IMETHOD InsertFrames(nsIAtom*        aListName,
                          nsIFrame*       aPrevFrame,
                          nsIFrame*       aFrameList);
  NS_IMETHOD RemoveFrame(nsIAtom*        aListName,
                         nsIFrame*       aOldFrame);

  virtual nsMargin GetUsedBorder() const;
  virtual nsMargin GetUsedPadding() const;

  // Get the offset from the border box to the area where the row groups fit
  nsMargin GetChildAreaOffset(const nsHTMLReflowState* aReflowState) const;

  // Get the offset from the border box to the area where the content fits
  nsMargin GetContentAreaOffset(const nsHTMLReflowState* aReflowState) const;

  /** helper method to find the table parent of any table frame object */
  static nsTableFrame* GetTableFrame(nsIFrame* aSourceFrame);
                                 
  typedef nsresult (* DisplayGenericTablePartTraversal)
      (nsDisplayListBuilder* aBuilder, nsFrame* aFrame,
       const nsRect& aDirtyRect, const nsDisplayListSet& aLists);
  static nsresult GenericTraversal(nsDisplayListBuilder* aBuilder, nsFrame* aFrame,
                                   const nsRect& aDirtyRect, const nsDisplayListSet& aLists);

  /**
   * Helper method to handle display common to table frames, rowgroup frames
   * and row frames. It creates a background display item for handling events
   * if necessary, an outline display item if necessary, and displays
   * all the the frame's children.
   * @param aIsRoot true if aFrame is the table frame or a table part which
   * happens to be the root of a stacking context
   * @param aTraversal a function that gets called to traverse the table
   * part's child frames and add their display list items to a
   * display list set.
   */
  static nsresult DisplayGenericTablePart(nsDisplayListBuilder* aBuilder,
                                          nsFrame* aFrame,
                                          const nsRect& aDirtyRect,
                                          const nsDisplayListSet& aLists,
                                          PRBool aIsRoot,
                                          DisplayGenericTablePartTraversal aTraversal = GenericTraversal);

  // Return the closest sibling of aPriorChildFrame (including aPriroChildFrame)
  // of type aChildType.
  static nsIFrame* GetFrameAtOrBefore(nsIFrame*       aParentFrame,
                                      nsIFrame*       aPriorChildFrame,
                                      nsIAtom*        aChildType);
  PRBool IsAutoWidth(PRBool* aIsPctWidth = nsnull);
  PRBool IsAutoHeight();
  static PRBool IsPctHeight(nsStyleContext* aStyleContext);
  
  /** @return PR_TRUE if aDisplayType represents a rowgroup of any sort
    * (header, footer, or body)
    */
  PRBool IsRowGroup(PRInt32 aDisplayType) const;

  /** Initialize the table frame with a set of children.
    * @see nsIFrame::SetInitialChildList 
    */
  NS_IMETHOD SetInitialChildList(nsIAtom*        aListName,
                                 nsIFrame*       aChildList);

  /** return the first child belonging to the list aListName. 
    * @see nsIFrame::GetFirstChild
    */
  virtual nsIFrame* GetFirstChild(nsIAtom* aListName) const;

  /** @see nsIFrame::GetAdditionalChildListName */
  virtual nsIAtom* GetAdditionalChildListName(PRInt32 aIndex) const;

  NS_IMETHOD BuildDisplayList(nsDisplayListBuilder*   aBuilder,
                              const nsRect&           aDirtyRect,
                              const nsDisplayListSet& aLists);

  /**
   * Paint the background of the table and its parts (column groups,
   * columns, row groups, rows, and cells), and the table border, and all
   * internal borders if border-collapse is on.
   */
  void PaintTableBorderBackground(nsIRenderingContext& aRenderingContext,
                                  const nsRect& aDirtyRect,
                                  nsPoint aPt);

  // Get the outer half (i.e., the part outside the height and width of
  // the table) of the largest segment (?) of border-collapsed border on
  // the table on each side, or 0 for non border-collapsed tables.
  nsMargin GetOuterBCBorder() const;

  // Same as above, but only if it's included from the border-box width
  // of the table (nonzero only in quirks mode).
  nsMargin GetIncludedOuterBCBorder() const;

  // Same as above, but only if it's excluded from the border-box width
  // of the table (nonzero only in standards mode).  This is the area
  // that leaks out into the margin (or potentially past it, if there is
  // no margin).
  nsMargin GetExcludedOuterBCBorder() const;

  /** Get width of table + colgroup + col collapse: elements that
   *  continue along the length of the whole left side.
   *  see nsTablePainter about continuous borders
   *  @param aPixelsToTwips - conversion factor
   *  @param aGetInner - get only inner half of border width
   */
  nscoord GetContinuousLeftBCBorderWidth() const;

  void SetBCDamageArea(const nsRect& aValue);

  void PaintBCBorders(nsIRenderingContext& aRenderingContext,
                      const nsRect&        aDirtyRect);

  /** nsIFrame method overridden to handle table specifics
  */
  NS_IMETHOD SetSelected(nsPresContext* aPresContext,
                         nsIDOMRange *aRange,
                         PRBool aSelected,
                         nsSpread aSpread);

  virtual void MarkIntrinsicWidthsDirty();
  // For border-collapse tables, the caller must not add padding and
  // border to the results of these functions.
  virtual nscoord GetMinWidth(nsIRenderingContext *aRenderingContext);
  virtual nscoord GetPrefWidth(nsIRenderingContext *aRenderingContext);
  virtual IntrinsicWidthOffsetData
    IntrinsicWidthOffsets(nsIRenderingContext* aRenderingContext);

  virtual nsSize ComputeSize(nsIRenderingContext *aRenderingContext,
                             nsSize aCBSize, nscoord aAvailableWidth,
                             nsSize aMargin, nsSize aBorder, nsSize aPadding,
                             PRBool aShrinkWrap);
  virtual nsSize ComputeAutoSize(nsIRenderingContext *aRenderingContext,
                                 nsSize aCBSize, nscoord aAvailableWidth,
                                 nsSize aMargin, nsSize aBorder,
                                 nsSize aPadding, PRBool aShrinkWrap);
  /**
   * A copy of nsFrame::ShrinkWidthToFit that calls a different
   * GetPrefWidth, since tables have two different ones.
   */
  nscoord TableShrinkWidthToFit(nsIRenderingContext *aRenderingContext,
                                nscoord aWidthInCB);

  // XXXldb REWRITE THIS COMMENT!
  /** inner tables are reflowed in two steps.
    * <pre>
    * if mFirstPassValid is false, this is our first time through since content was last changed
    *   set pass to 1
    *   do pass 1
    *     get min/max info for all cells in an infinite space
    *   do column balancing
    *   set mFirstPassValid to true
    *   do pass 2
    *     use column widths to Reflow cells
    * </pre>
    *
    * @see nsIFrame::Reflow
    */
  NS_IMETHOD Reflow(nsPresContext*          aPresContext,
                    nsHTMLReflowMetrics&     aDesiredSize,
                    const nsHTMLReflowState& aReflowState,
                    nsReflowStatus&          aStatus);

  nsresult ReflowTable(nsHTMLReflowMetrics&     aDesiredSize,
                       const nsHTMLReflowState& aReflowState,
                       nscoord                  aAvailHeight,
                       nsIFrame*&               aLastChildReflowed,
                       nsReflowStatus&          aStatus);

  nsFrameList& GetColGroups();

  NS_IMETHOD GetParentStyleContextFrame(nsPresContext* aPresContext,
                                        nsIFrame**      aProviderFrame,
                                        PRBool*         aIsChild);

  /**
   * Get the "type" of the frame
   *
   * @see nsGkAtoms::tableFrame
   */
  virtual nsIAtom* GetType() const;

#ifdef DEBUG
  /** @see nsIFrame::GetFrameName */
  NS_IMETHOD GetFrameName(nsAString& aResult) const;
#endif

  /** return the width of the column at aColIndex    */
  virtual PRInt32 GetColumnWidth(PRInt32 aColIndex);

  /** set the width of the column at aColIndex to aWidth    */
  virtual void SetColumnWidth(PRInt32 aColIndex, nscoord aWidth);

  /** helper to get the cell spacing X style value */
  virtual nscoord GetCellSpacingX();

  /** helper to get the cell spacing Y style value */
  virtual nscoord GetCellSpacingY();
 
  virtual nscoord GetBaseline() const;
  /** return the row span of a cell, taking into account row span magic at the bottom
    * of a table. The row span equals the number of rows spanned by aCell starting at
    * aStartRowIndex, and can be smaller if aStartRowIndex is greater than the row
    * index in which aCell originates.
    *
    * @param aStartRowIndex the cell
    * @param aCell          the cell
    *
    * @return  the row span, correcting for row spans that extend beyond the bottom
    *          of the table.
    */
  virtual PRInt32  GetEffectiveRowSpan(PRInt32                 aStartRowIndex,
                                       const nsTableCellFrame& aCell) const;
  virtual PRInt32  GetEffectiveRowSpan(const nsTableCellFrame& aCell,
                                       nsCellMap*              aCellMap = nsnull);

  /** return the col span of a cell, taking into account col span magic at the edge
    * of a table.
    *
    * @param aCell      the cell
    *
    * @return  the col span, correcting for col spans that extend beyond the edge
    *          of the table.
    */
  virtual PRInt32  GetEffectiveColSpan(const nsTableCellFrame& aCell,
                                       nsCellMap*              aCellMap = nsnull) const;

  /** indicate whether the row has more than one cell that either originates
    * or is spanned from the rows above
    */
  PRBool HasMoreThanOneCell(PRInt32 aRowIndex) const;

  /** return the value of the COLS attribute, adjusted for the 
    * actual number of columns in the table
    */
  PRInt32 GetEffectiveCOLSAttribute();

  /** return the column frame associated with aColIndex
    * returns nsnull if the col frame has not yet been allocated, or if
    * aColIndex is out of range
    */
  nsTableColFrame* GetColFrame(PRInt32 aColIndex) const;

  /** Insert a col frame reference into the colframe cache and adapt the cellmap
    * @param aColFrame    - the column frame
    * @param aColIndex    - index where the column should be inserted into the
    *                       colframe cache
    */
  void InsertCol(nsTableColFrame& aColFrame,
                 PRInt32          aColIndex);

  nsTableColGroupFrame* CreateAnonymousColGroupFrame(nsTableColGroupType aType);

  PRInt32 DestroyAnonymousColFrames(PRInt32 aNumFrames);

  void CreateAnonymousColFrames(PRInt32         aNumColsToAdd,
                                nsTableColType  aColType,
                                PRBool          aDoAppend,
                                nsIFrame*       aPrevCol = nsnull);

  void CreateAnonymousColFrames(nsTableColGroupFrame* aColGroupFrame,
                                PRInt32               aNumColsToAdd,
                                nsTableColType        aColType,
                                PRBool                aAddToColGroupAndTable,
                                nsIFrame*             aPrevCol,
                                nsIFrame**            aFirstNewFrame);

  void MatchCellMapToColCache(nsTableCellMap* aCellMap);
  /** empty the column frame cache */
  void ClearColCache();

  void DidResizeColumns();

  virtual void AppendCell(nsTableCellFrame& aCellFrame,
                          PRInt32           aRowIndex);

  virtual void InsertCells(nsVoidArray&    aCellFrames, 
                           PRInt32         aRowIndex, 
                           PRInt32         aColIndexBefore);

  virtual void RemoveCell(nsTableCellFrame* aCellFrame,
                          PRInt32           aRowIndex);

  void AppendRows(nsTableRowGroupFrame& aRowGroupFrame,
                  PRInt32               aRowIndex,
                  nsVoidArray&          aRowFrames);

  PRInt32 InsertRow(nsTableRowGroupFrame& aRowGroupFrame,
                    nsIFrame&             aFrame,
                    PRInt32               aRowIndex,
                    PRBool                aConsiderSpans);

  PRInt32 InsertRows(nsTableRowGroupFrame& aRowGroupFrame,
                     nsVoidArray&          aFrames,
                     PRInt32               aRowIndex,
                     PRBool                aConsiderSpans);

  virtual void RemoveRows(nsTableRowFrame& aFirstRowFrame,
                          PRInt32          aNumRowsToRemove,
                          PRBool           aConsiderSpans);

  /** Insert multiple rowgroups into the table cellmap handling
    * @param aFirstRowGroupFrame - first row group to be inserted all siblings
    *                              will be appended too.
    */
  void AppendRowGroups(nsIFrame* aFirstRowGroupFrame);

  /** Insert multiple rowgroups into the table cellmap handling
    * @param aFirstRowGroupFrame - first row group to be inserted
    * @param aLastRowGroupFrame  - when inserting the siblings of 
    *                              aFirstRowGroupFrame stop at this row group
    */
  void InsertRowGroups(nsIFrame*       aFirstRowGroupFrame,
                       nsIFrame*       aLastRowGroupFrame);

  void InsertColGroups(PRInt32         aColIndex,
                       nsIFrame*       aFirstFrame,
                       nsIFrame*       aLastFrame = nsnull);

  virtual void RemoveCol(nsTableColGroupFrame* aColGroupFrame,
                         PRInt32               aColIndex,
                         PRBool                aRemoveFromCache,
                         PRBool                aRemoveFromCellMap);

  NS_IMETHOD GetIndexByRowAndColumn(PRInt32 aRow, PRInt32 aColumn, PRInt32 *aIndex);
  NS_IMETHOD GetRowAndColumnByIndex(PRInt32 aIndex, PRInt32 *aRow, PRInt32 *aColumn);
  PRInt32 GetNumCellsOriginatingInCol(PRInt32 aColIndex) const;
  PRInt32 GetNumCellsOriginatingInRow(PRInt32 aRowIndex) const;

  PRBool HasPctCol() const;
  void SetHasPctCol(PRBool aValue);

  PRBool HasCellSpanningPctCol() const;
  void SetHasCellSpanningPctCol(PRBool aValue);

  /**
   * To be called on a frame by its parent after setting its size/position and
   * calling DidReflow (possibly via FinishReflowChild()).  This can also be
   * used for child frames which are not being reflown but did have their size
   * or position changed.
   *
   * @param aFrame The frame to invalidate
   * @param aOrigRect The original rect of aFrame (before the change).
   * @param aIsFirstReflow True if the size/position change is due to the
   *                       first reflow of aFrame.
   */
  static void InvalidateFrame(nsIFrame* aFrame, const nsRect& aOrigRect,
                              PRBool aIsFirstReflow);

protected:

  /** protected constructor. 
    * @see NewFrame
    */
  nsTableFrame(nsStyleContext* aContext);

  /** destructor, responsible for mColumnLayoutData */
  virtual ~nsTableFrame();

  void InitChildReflowState(nsHTMLReflowState& aReflowState);

  /** implement abstract method on nsHTMLContainerFrame */
  virtual PRIntn GetSkipSides() const;

  virtual PRBool ParentDisablesSelection() const; //override default behavior

public:
  PRBool IsRowInserted() const;
  void   SetRowInserted(PRBool aValue);

protected:
    
  // A helper function to reflow a header or footer with unconstrained height
  // to see if it should be made repeatable and also to determine its desired
  // height.
  nsresult SetupHeaderFooterChild(const nsTableReflowState& aReflowState,
                                  nsTableRowGroupFrame* aFrame,
                                  nscoord* aDesiredHeight);

  NS_METHOD ReflowChildren(nsTableReflowState&  aReflowState,
                           nsReflowStatus&      aStatus,
                           nsIFrame*&           aLastChildReflowed,
                           nsRect&              aOverflowArea);

  // This calls the col group and column reflow methods, which do two things:
  //  (1) set all the dimensions to 0
  //  (2) notify the table about colgroups or columns with hidden visibility
  void ReflowColGroups(nsIRenderingContext* aRenderingContext);

  /** return the width of the table taking into account visibility collapse
    * on columns and colgroups
    * @param aBorderPadding  the border and padding of the table
    */
  nscoord GetCollapsedWidth(nsMargin aBorderPadding);

  
  /** Adjust the table for visibilty.collapse set on rowgroups, rows, colgroups
    * and cols
    * @param aDesiredSize    the metrics of the table
    * @param aBorderPadding  the border and padding of the table
    */
  void AdjustForCollapsingRowsCols(nsHTMLReflowMetrics& aDesiredSize,
                                   nsMargin             aBorderPadding);

  nsITableLayoutStrategy* LayoutStrategy() {
    return static_cast<nsTableFrame*>(GetFirstInFlow())->
      mTableLayoutStrategy;
  }

private:
  /* Handle a row that got inserted during reflow.  aNewHeight is the
     new height of the table after reflow. */
  void ProcessRowInserted(nscoord aNewHeight);

  // WIDTH AND HEIGHT CALCULATION

public:

  // calculate the computed height of aFrame including its border and padding given 
  // its reflow state.
  nscoord CalcBorderBoxHeight(const nsHTMLReflowState& aReflowState);

protected:

  // update the  desired height of this table taking into account the current
  // reflow state, the table attributes and the content driven rowgroup heights
  // this function can change the overflow area
  void CalcDesiredHeight(const nsHTMLReflowState& aReflowState, nsHTMLReflowMetrics& aDesiredSize);

  // The following is a helper for CalcDesiredHeight 
 
  void DistributeHeightToRows(const nsHTMLReflowState& aReflowState,
                              nscoord                  aAmount);

  void PlaceChild(nsTableReflowState&  aReflowState,
                  nsIFrame*            aKidFrame,
                  nsHTMLReflowMetrics& aKidDesiredSize,
                  const nsRect&        aOriginalKidRect);

  nsIFrame* GetFirstBodyRowGroupFrame();
  PRBool MoveOverflowToChildList(nsPresContext* aPresContext);
  /**
   * Push all our child frames from the aFrames array, in order, starting from the
   * frame at aPushFrom to the end of the array. The frames are put on our overflow
   * list or moved directly to our next-in-flow if one exists.
   */
  typedef nsAutoTPtrArray<nsIFrame, 8> FrameArray;
  void PushChildren(const FrameArray& aFrames, PRInt32 aPushFrom);

public:
  // put the children frames in the display order (e.g. thead before tbodies
  // before tfoot). This will handle calling GetRowGroupFrame() on the
  // children, and not append nulls, so the array is guaranteed to contain
  // nsTableRowGroupFrames.  If there are multiple theads or tfoots, all but
  // the first one are treated as tbodies instead.
  typedef nsAutoTPtrArray<nsTableRowGroupFrame, 8> RowGroupArray;
  void OrderRowGroups(RowGroupArray& aChildren) const;

  // Return the thead, if any
  nsTableRowGroupFrame* GetTHead() const;

  // Return the tfoot, if any
  nsTableRowGroupFrame* GetTFoot() const;

protected:
  // As above, but does NOT actually call GetRowGroupFrame() on the kids, so
  // returns an array of nsIFrames.  This is to be used when you really want
  // the flowable kids of the table, not the rowgroups.  This outputs the thead
  // and tfoot if they happen to be rowgroups.  All the child nsIFrames of the
  // table that return null if you call GetRowGroupFrame() on them will appear
  // at the end of the array, after the tfoot, if any.
  //
  // aHead and aFoot must not be null.
  //
  // @return the number of frames in aChildren which return non-null if you
  // call GetRowGroupFrame() on them.
  //
  // XXXbz why do we really care about the non-rowgroup kids?
  PRUint32 OrderRowGroups(FrameArray& aChildren,
                          nsTableRowGroupFrame** aHead,
                          nsTableRowGroupFrame** aFoot) const;

public:
  // Returns PR_TRUE if there are any cells above the row at
  // aRowIndex and spanning into the row at aRowIndex, the number of
  // effective columns limits the search up to that column
  PRBool RowIsSpannedInto(PRInt32 aRowIndex, PRInt32 aNumEffCols);

  // Returns PR_TRUE if there is a cell originating in aRowIndex
  // which spans into the next row,  the number of effective
  // columns limits the search up to that column
  PRBool RowHasSpanningCells(PRInt32 aRowIndex, PRInt32 aNumEffCols);

  // Returns PR_TRUE if there are any cells to the left of the column at
  // aColIndex and spanning into the column at aColIndex     
  PRBool ColIsSpannedInto(PRInt32 aColIndex);

  // Returns PR_TRUE if there is a cell originating in aColIndex
  // which spans into the next col
  PRBool ColHasSpanningCells(PRInt32 aColIndex);

protected:

  PRBool HaveReflowedColGroups() const;
  void   SetHaveReflowedColGroups(PRBool aValue);

public:
  PRBool IsBorderCollapse() const;

  PRBool NeedToCalcBCBorders() const;
  void SetNeedToCalcBCBorders(PRBool aValue);

  PRBool NeedToCollapse() const;
  void SetNeedToCollapse(PRBool aValue);

  PRBool HasZeroColSpans() const;
  void SetHasZeroColSpans(PRBool aValue);

  PRBool NeedColSpanExpansion() const;
  void SetNeedColSpanExpansion(PRBool aValue);

  /** The GeometryDirty bit is similar to the NS_FRAME_IS_DIRTY frame
    * state bit, which implies that all descendants are dirty.  The
    * GeometryDirty still implies that all the parts of the table are
    * dirty, but resizing optimizations should still apply to the
    * contents of the individual cells.
    */
  void SetGeometryDirty() { mBits.mGeometryDirty = PR_TRUE; }
  void ClearGeometryDirty() { mBits.mGeometryDirty = PR_FALSE; }
  PRBool IsGeometryDirty() const { return mBits.mGeometryDirty; }

  /** Get the cell map for this table frame.  It is not always mCellMap.
    * Only the firstInFlow has a legit cell map
    */
  virtual nsTableCellMap* GetCellMap() const;

  /** Iterate over the row groups and adjust the row indices of all rows 
    * whose index is >= aRowIndex.  
    * @param aRowIndex   - start adjusting with this index
    * @param aAdjustment - shift the row index by this amount
    */
  void AdjustRowIndices(PRInt32 aRowIndex,
                        PRInt32 aAdjustment);

  /** Reset the rowindices of all rows as they might have changed due to 
    * rowgroup reordering, exclude new row group frames that show in the
    * reordering but are not yet inserted into the cellmap
    * @param aFirstRowGroupFrame - first row group to be excluded
    * @param aLastRowGroupFrame  - last sibling of aFirstRowGroupFrame that
    *                              should be excluded when reseting the row
    *                              indices.
    */
  void ResetRowIndices(nsIFrame* aFirstRowGroupFrame = nsnull,
                       nsIFrame* aLastRowGroupFrame = nsnull);

  nsVoidArray& GetColCache();

  /** Return aFrame's child if aFrame is an nsScrollFrame, otherwise return aFrame
    */
  static nsTableRowGroupFrame* GetRowGroupFrame(nsIFrame* aFrame,
                                                nsIAtom*  aFrameTypeIn = nsnull);

protected:

  void SetBorderCollapse(PRBool aValue);

  void CalcBCBorders();

  void ExpandBCDamageArea(nsRect& aRect) const;

  void SetColumnDimensions(nscoord         aHeight,
                           const nsMargin& aReflowState);

  PRInt32 CollectRows(nsIFrame*       aFrame,
                      nsVoidArray&    aCollection);

public: /* ----- Cell Map public methods ----- */

  PRInt32 GetStartRowIndex(nsTableRowGroupFrame& aRowGroupFrame);

  /** returns the number of rows in this table.
    */
  PRInt32 GetRowCount () const
  {
    return GetCellMap()->GetRowCount();
  }

  /** returns the number of columns in this table after redundant columns have been removed 
    */
  PRInt32 GetEffectiveColCount() const;

  /* return the col count including dead cols */
  PRInt32 GetColCount () const
  {
    return GetCellMap()->GetColCount();
  }

  // return the last col index which isn't of type eColAnonymousCell
  PRInt32 GetIndexOfLastRealCol();

  /** returns PR_TRUE if table-layout:auto  */
  virtual PRBool IsAutoLayout();

  /*---------------- nsITableLayout methods ------------------------*/
  
  /** Get the cell and associated data for a table cell from the frame's cellmap */
  NS_IMETHOD GetCellDataAt(PRInt32 aRowIndex, PRInt32 aColIndex, 
                           nsIDOMElement* &aCell,   //out params
                           PRInt32& aStartRowIndex, PRInt32& aStartColIndex, 
                           PRInt32& aRowSpan, PRInt32& aColSpan,
                           PRInt32& aActualRowSpan, PRInt32& aActualColSpan,
                           PRBool& aIsSelected);

  /** Get the number of rows and column for a table from the frame's cellmap 
    *  Some rows may not have enough cells (the number returned is the maximum possible),
    *  which displays as a ragged-right edge table
    */
  NS_IMETHOD GetTableSize(PRInt32& aRowCount, PRInt32& aColCount);

  /*------------end of nsITableLayout methods -----------------------*/

public:
 
#ifdef DEBUG
  void Dump(PRBool          aDumpRows,
            PRBool          aDumpCols, 
            PRBool          aDumpCellMap);
  static void DumpTableFrames(nsIFrame* aFrame);
#endif

protected:
#ifdef DEBUG
  void DumpRowGroup(nsIFrame* aChildFrame);
#endif
  // DATA MEMBERS
  nsAutoVoidArray mColFrames;  

  struct TableBits {
    PRUint32 mHaveReflowedColGroups:1; // have the col groups gotten their initial reflow
    PRUint32 mHasPctCol:1;             // does any cell or col have a pct width
    PRUint32 mCellSpansPctCol:1;       // does any cell span a col with a pct width (or containing a cell with a pct width)
    PRUint32 mIsBorderCollapse:1;      // border collapsing model vs. separate model
    PRUint32 mRowInserted:1;
    PRUint32 mNeedToCalcBCBorders:1;
    PRUint32 mGeometryDirty:1;
    PRUint32 mLeftContBCBorder:8;
    PRUint32 mNeedToCollapse:1;    // rows, cols that have visibility:collapse need to be collapsed
    PRUint32 mHasZeroColSpans:1;
    PRUint32 mNeedColSpanExpansion:1;
    PRUint32 mResizedColumns:1;        // have we resized columns since last reflow?
  } mBits;

  nsTableCellMap*         mCellMap;            // maintains the relationships between rows, cols, and cells
  nsITableLayoutStrategy* mTableLayoutStrategy;// the layout strategy for this frame
  nsFrameList             mColGroups;          // the list of colgroup frames
};


inline PRBool nsTableFrame::IsRowGroup(PRInt32 aDisplayType) const
{
  return PRBool((NS_STYLE_DISPLAY_TABLE_HEADER_GROUP == aDisplayType) ||
                (NS_STYLE_DISPLAY_TABLE_FOOTER_GROUP == aDisplayType) ||
                (NS_STYLE_DISPLAY_TABLE_ROW_GROUP    == aDisplayType));
}

inline void nsTableFrame::SetHaveReflowedColGroups(PRBool aValue)
{
  mBits.mHaveReflowedColGroups = aValue;
}

inline PRBool nsTableFrame::HaveReflowedColGroups() const
{
  return (PRBool)mBits.mHaveReflowedColGroups;
}

inline PRBool nsTableFrame::HasPctCol() const
{
  return (PRBool)mBits.mHasPctCol;
}

inline void nsTableFrame::SetHasPctCol(PRBool aValue)
{
  mBits.mHasPctCol = (unsigned)aValue;
}

inline PRBool nsTableFrame::HasCellSpanningPctCol() const
{
  return (PRBool)mBits.mCellSpansPctCol;
}

inline void nsTableFrame::SetHasCellSpanningPctCol(PRBool aValue)
{
  mBits.mCellSpansPctCol = (unsigned)aValue;
}

inline PRBool nsTableFrame::IsRowInserted() const
{
  return (PRBool)mBits.mRowInserted;
}

inline void nsTableFrame::SetRowInserted(PRBool aValue)
{
  mBits.mRowInserted = (unsigned)aValue;
}

inline void nsTableFrame::SetNeedToCollapse(PRBool aValue)
{
  mBits.mNeedToCollapse = (unsigned)aValue;
}

inline PRBool nsTableFrame::NeedToCollapse() const
{
  return (PRBool)mBits.mNeedToCollapse;
}

inline void nsTableFrame::SetHasZeroColSpans(PRBool aValue)
{
  mBits.mHasZeroColSpans = (unsigned)aValue;
}

inline PRBool nsTableFrame::HasZeroColSpans() const
{
  return (PRBool)mBits.mHasZeroColSpans;
}

inline void nsTableFrame::SetNeedColSpanExpansion(PRBool aValue)
{
  mBits.mNeedColSpanExpansion = (unsigned)aValue;
}

inline PRBool nsTableFrame::NeedColSpanExpansion() const
{
  return (PRBool)mBits.mNeedColSpanExpansion;
}


inline nsFrameList& nsTableFrame::GetColGroups()
{
  return static_cast<nsTableFrame*>(GetFirstInFlow())->mColGroups;
}

inline nsVoidArray& nsTableFrame::GetColCache()
{
  return mColFrames;
}

inline PRBool nsTableFrame::IsBorderCollapse() const
{
  return (PRBool)mBits.mIsBorderCollapse;
}

inline void nsTableFrame::SetBorderCollapse(PRBool aValue) 
{
  mBits.mIsBorderCollapse = aValue;
}

inline PRBool nsTableFrame::NeedToCalcBCBorders() const
{
  return (PRBool)mBits.mNeedToCalcBCBorders;
}

inline void nsTableFrame::SetNeedToCalcBCBorders(PRBool aValue)
{
  mBits.mNeedToCalcBCBorders = (unsigned)aValue;
}

inline nscoord
nsTableFrame::GetContinuousLeftBCBorderWidth() const
{
  PRInt32 aPixelsToTwips = nsPresContext::AppUnitsPerCSSPixel();
  return BC_BORDER_RIGHT_HALF_COORD(aPixelsToTwips, mBits.mLeftContBCBorder);
}

class nsTableIterator
{
public:
  nsTableIterator(nsIFrame& aSource);
  nsTableIterator(nsFrameList& aSource);
  nsIFrame* First();
  nsIFrame* Next();
  PRBool    IsLeftToRight();
  PRInt32   Count();

protected:
  void Init(nsIFrame* aFirstChild);
  PRBool    mLeftToRight;
  nsIFrame* mFirstListChild;
  nsIFrame* mFirstChild;
  nsIFrame* mCurrentChild;
  PRInt32   mCount;
};

#define ABORT0() \
{NS_ASSERTION(PR_FALSE, "CellIterator program error"); \
return;}

#define ABORT1(aReturn) \
{NS_ASSERTION(PR_FALSE, "CellIterator program error"); \
return aReturn;} 

#endif
