/*
 * ***** BEGIN LICENSE BLOCK *****
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
 * a mozilla.org contributor.
 * Portions created by the Initial Developer are Copyright (C) 2006
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

#ifndef nsFrameSelection_h___
#define nsFrameSelection_h___
 
#include "nsIFrame.h"
#include "nsIContent.h"
#include "nsISelectionController.h"
#include "nsIScrollableViewProvider.h"
#include "nsITableLayout.h"
#include "nsITableCellLayout.h"
#include "nsIDOMElement.h"
#include "nsGUIEvent.h"

// IID for the nsFrameSelection interface
// d78edc5a-28d0-48f0-8abb-1597b1591556
#define NS_FRAME_SELECTION_IID      \
{ 0xd78edc5a, 0x28d0, 0x48f0, \
  { 0x8a, 0xbb, 0x15, 0x97, 0xb1, 0x59, 0x15, 0x56 } }

#ifdef IBMBIDI // Constant for Set/Get CaretBidiLevel
#define BIDI_LEVEL_UNDEFINED 0x80
#endif

//----------------------------------------------------------------------

// Selection interface

struct SelectionDetails
{
  PRInt32 mStart;
  PRInt32 mEnd;
  SelectionType mType;
  SelectionDetails *mNext;
};

class nsIPresShell;

enum EWordMovementType { eStartWord, eEndWord, eDefaultBehavior };

/** PeekOffsetStruct is used to group various arguments (both input and output)
 *  that are passed to nsFrame::PeekOffset(). See below for the description of
 *  individual arguments.
 */
struct nsPeekOffsetStruct
{
  void SetData(nsSelectionAmount aAmount,
               nsDirection aDirection,
               PRInt32 aStartOffset,
               nscoord aDesiredX,
               PRBool aJumpLines,
               PRBool aScrollViewStop,
               PRBool aIsKeyboardSelect,
               PRBool aVisual,
               EWordMovementType aWordMovementType = eDefaultBehavior)

  {
    mAmount = aAmount;
    mDirection = aDirection;
    mStartOffset = aStartOffset;
    mDesiredX = aDesiredX;
    mJumpLines = aJumpLines;
    mScrollViewStop = aScrollViewStop;
    mIsKeyboardSelect = aIsKeyboardSelect;
    mVisual = aVisual;
    mWordMovementType = aWordMovementType;
  }

  // Note: Most arguments (input and output) are only used with certain values
  // of mAmount. These values are indicated for each argument below.
  // Arguments with no such indication are used with all values of mAmount.

  /*** Input arguments ***/
  // Note: The value of some of the input arguments may be changed upon exit.

  // mAmount: The type of movement requested (by character, word, line, etc.)
  nsSelectionAmount mAmount;

  // mDirection: eDirPrevious or eDirNext.
  //             * Note for visual bidi movement:
  //             eDirPrevious means 'left-then-up' if the containing block is LTR, 
  //             'right-then-up' if it is RTL.
  //             eDirNext means 'right-then-down' if the containing block is LTR, 
  //             'left-then-down' if it is RTL.
  //             Between paragraphs, eDirPrevious means "go to the visual end of the 
  //             previous paragraph", and eDirNext means "go to the visual beginning
  //             of the next paragraph".
  //             Used with: eSelectCharacter, eSelectWord, eSelectLine, eSelectParagraph.
  nsDirection mDirection;

  // mStartOffset: Offset into the content of the current frame where the peek starts.
  //               Used with: eSelectCharacter, eSelectWord
  PRInt32 mStartOffset;
  
  // mDesiredX: The desired x coordinate for the caret.
  //            Used with: eSelectLine.
  nscoord mDesiredX;

  // mJumpLines: Whether to allow jumping across line boundaries.
  //             Used with: eSelectCharacter, eSelectWord.
  PRBool mJumpLines;

  // mScrollViewStop: Whether to stop when reaching a scroll view boundary.
  //                  Used with: eSelectCharacter, eSelectWord, eSelectLine.
  PRBool mScrollViewStop;

  // mIsKeyboardSelect: Whether the peeking is done in response to a keyboard action.
  //                    Used with: eSelectWord.
  PRBool mIsKeyboardSelect;

  // mVisual: Whether bidi caret behavior is visual (PR_TRUE) or logical (PR_FALSE).
  //          Used with: eSelectCharacter, eSelectWord, eSelectBeginLine, eSelectEndLine.
  PRBool mVisual;

  // mWordMovementType: An enum that determines whether to prefer the start or end of a word
  //                    or to use the default beahvior, which is a combination of 
  //                    direction and the platform-based pref
  //                    "layout.word_select.eat_space_to_next_word"
  EWordMovementType mWordMovementType;

  /*** Output arguments ***/

  // mResultContent: Content reached as a result of the peek.
  nsCOMPtr<nsIContent> mResultContent;

  // mContentOffset: Offset into content reached as a result of the peek.
  PRInt32 mContentOffset;

  // mResultFrame: Frame reached as a result of the peek.
  //               Used with: eSelectCharacter, eSelectWord.
  nsIFrame *mResultFrame;

  // mAttachForward: When the result position is between two frames,
  //                 indicates which of the two frames the caret should be painted in.
  //                 PR_FALSE means "the end of the frame logically before the caret", 
  //                 PR_TRUE means "the beginning of the frame logically after the caret".
  //                 Used with: eSelectLine, eSelectBeginLine, eSelectEndLine.
  PRBool mAttachForward;
};

struct nsPrevNextBidiLevels
{
  void SetData(nsIFrame* aFrameBefore,
               nsIFrame* aFrameAfter,
               PRUint8 aLevelBefore,
               PRUint8 aLevelAfter)
  {
    mFrameBefore = aFrameBefore;
    mFrameAfter = aFrameAfter;
    mLevelBefore = aLevelBefore;
    mLevelAfter = aLevelAfter;
  }
  nsIFrame* mFrameBefore;
  nsIFrame* mFrameAfter;
  PRUint8 mLevelBefore;
  PRUint8 mLevelAfter;
};

class nsTypedSelection;
class nsIScrollableView;

/**
 * Methods which are marked with *unsafe* should be handled with special care.
 * They may cause nsFrameSelection to be deleted, if strong pointer isn't used,
 * or they may cause other objects to be deleted.
 */

class nsFrameSelection : public nsISupports {
public:
  NS_DECLARE_STATIC_IID_ACCESSOR(NS_FRAME_SELECTION_IID)
  enum HINT { HINTLEFT = 0, HINTRIGHT = 1};  //end of this line or beginning of next
  /*interfaces for addref and release and queryinterface*/
  
  NS_DECL_ISUPPORTS

  /** Init will initialize the frame selector with the necessary pres shell to 
   *  be used by most of the methods
   *  @param aShell is the parameter to be used for most of the other calls for callbacks etc
   *  @param aLimiter limits the selection to nodes with aLimiter parents
   */
  void Init(nsIPresShell *aShell, nsIContent *aLimiter);

  /**
   * SetScrollableViewProvider sets the scroll view provider.
   * @param aProvider The provider of the scroll view.
   */
  void SetScrollableViewProvider(nsIScrollableViewProvider* aProvider)
  {
    mScrollableViewProvider = aProvider;
  }

  /**
   * GetScrollableView returns the current scroll view.
   */
  nsIScrollableView* GetScrollableView() const
  {
    return mScrollableViewProvider
      ? mScrollableViewProvider->GetScrollableView()
      : nsnull;
  }

  /** HandleClick will take the focus to the new frame at the new offset and 
   *  will either extend the selection from the old anchor, or replace the old anchor.
   *  the old anchor and focus position may also be used to deselect things
   *  @param aNewfocus is the content that wants the focus
   *  @param aContentOffset is the content offset of the parent aNewFocus
   *  @param aContentOffsetEnd is the content offset of the parent aNewFocus and is specified different
   *                           when you need to select to and include both start and end points
   *  @param aContinueSelection is the flag that tells the selection to keep the old anchor point or not.
   *  @param aMultipleSelection will tell the frame selector to replace /or not the old selection. 
   *         cannot coexist with aContinueSelection
   *  @param aHint will tell the selection which direction geometrically to actually show the caret on. 
   *         1 = end of this line 0 = beginning of this line
   */
  /*unsafe*/
  nsresult HandleClick(nsIContent *aNewFocus,
                       PRUint32 aContentOffset,
                       PRUint32 aContentEndOffset,
                       PRBool aContinueSelection,
                       PRBool aMultipleSelection,
                       PRBool aHint);

  /** HandleDrag extends the selection to contain the frame closest to aPoint.
   *  @param aPresContext is the context to use when figuring out what frame contains the point.
   *  @param aFrame is the parent of all frames to use when searching for the closest frame to the point.
   *  @param aPoint is relative to aFrame
   */
  /*unsafe*/
  void HandleDrag(nsIFrame *aFrame, nsPoint aPoint);

  /** HandleTableSelection will set selection to a table, cell, etc
   *   depending on information contained in aFlags
   *  @param aParentContent is the paretent of either a table or cell that user clicked or dragged the mouse in
   *  @param aContentOffset is the offset of the table or cell
   *  @param aTarget indicates what to select (defined in nsISelectionPrivate.idl/nsISelectionPrivate.h):
   *    TABLESELECTION_CELL      We should select a cell (content points to the cell)
   *    TABLESELECTION_ROW       We should select a row (content points to any cell in row)
   *    TABLESELECTION_COLUMN    We should select a row (content points to any cell in column)
   *    TABLESELECTION_TABLE     We should select a table (content points to the table)
   *    TABLESELECTION_ALLCELLS  We should select all cells (content points to any cell in table)
   *  @param aMouseEvent         passed in so we can get where event occurred and what keys are pressed
   */
  /*unsafe*/
  nsresult HandleTableSelection(nsIContent *aParentContent,
                                PRInt32 aContentOffset,
                                PRInt32 aTarget,
                                nsMouseEvent *aMouseEvent);

  /** StartAutoScrollTimer is responsible for scrolling views so that aPoint is always
   *  visible, and for selecting any frame that contains aPoint. The timer will also reset
   *  itself to fire again if we have not scrolled to the end of the document.
   *  @param aView is view to use when searching for the closest frame to the point,
   *  which is the view that is capturing the mouse
   *  @param aPoint is relative to the view.
   *  @param aDelay is the timer's interval.
   */
  nsresult StartAutoScrollTimer(nsIView *aView,
                                nsPoint aPoint,
                                PRUint32 aDelay);

  /** StopAutoScrollTimer stops any active auto scroll timer.
   */
  void StopAutoScrollTimer();

  /** Lookup Selection
   *  returns in frame coordinates the selection beginning and ending with the type of selection given
   * @param aContent is the content asking
   * @param aContentOffset is the starting content boundary
   * @param aContentLength is the length of the content piece asking
   * @param aReturnDetails linkedlist of return values for the selection. 
   * @param aSlowCheck will check using slow method with no shortcuts
   */
  SelectionDetails* LookUpSelection(nsIContent *aContent,
                                    PRInt32 aContentOffset,
                                    PRInt32 aContentLength,
                                    PRBool aSlowCheck) const;

  /** SetMouseDownState(PRBool);
   *  sets the mouse state to aState for resons of drag state.
   * @param aState is the new state of mousedown
   */
  /*unsafe*/
  void SetMouseDownState(PRBool aState);

  /** GetMouseDownState(PRBool *);
   *  gets the mouse state to aState for resons of drag state.
   * @param aState will hold the state of mousedown
   */
  PRBool GetMouseDownState() const { return mMouseDownState; }

  /**
    if we are in table cell selection mode. aka ctrl click in table cell
   */
  PRBool GetTableCellSelection() const { return mSelectingTableCellMode != 0; }
  void ClearTableCellSelection() { mSelectingTableCellMode = 0; }

  /** GetSelection
   * no query interface for selection. must use this method now.
   * @param aSelectionType enum value defined in nsISelection for the seleciton you want.
   */
  nsISelection* GetSelection(SelectionType aType) const;

  /**
   * ScrollSelectionIntoView scrolls a region of the selection,
   * so that it is visible in the scrolled view.
   *
   * @param aType the selection to scroll into view.
   * @param aRegion the region inside the selection to scroll into view.
   * @param aIsSynchronous when PR_TRUE, scrolls the selection into view
   * at some point after the method returns.request which is processed
   */
  nsresult ScrollSelectionIntoView(SelectionType aType,
                                   SelectionRegion aRegion,
                                   PRBool aIsSynchronous) const;

  /** RepaintSelection repaints the selected frames that are inside the selection
   *  specified by aSelectionType.
   * @param aSelectionType enum value defined in nsISelection for the seleciton you want.
   */
  nsresult RepaintSelection(SelectionType aType) const;

  /** GetFrameForNodeOffset given a node and its child offset, return the nsIFrame and
   *  the offset into that frame. 
   * @param aNode input parameter for the node to look at
   * @param aOffset offset into above node.
   * @param aReturnOffset will contain offset into frame.
   */
  virtual nsIFrame* GetFrameForNodeOffset(nsIContent *aNode,
                                          PRInt32     aOffset,
                                          HINT        aHint,
                                          PRInt32    *aReturnOffset) const;

  /**
   * Scrolling then moving caret placement code in common to text areas and 
   * content areas should be located in the implementer
   * This method will accept the following parameters and perform the scroll 
   * and caret movement.  It remains for the caller to call the final 
   * ScrollCaretIntoView if that called wants to be sure the caret is always
   * visible.
   *
   * @param aForward if PR_TRUE, scroll forward if not scroll backward
   * @param aExtend  if PR_TRUE, extend selection to the new point
   * @param aScrollableView the view that needs the scrolling
   */
  /*unsafe*/
  void CommonPageMove(PRBool aForward,
                      PRBool aExtend,
                      nsIScrollableView *aScrollableView);

  void SetHint(HINT aHintRight) { mHint = aHintRight; }
  HINT GetHint() const { return mHint; }
  
#ifdef IBMBIDI
  /** SetCaretBidiLevel sets the caret bidi level
   *  @param aLevel the caret bidi level
   *  This method is virtual since it gets called from outside of layout.
   */
  virtual void SetCaretBidiLevel (PRUint8 aLevel);
  /** GetCaretBidiLevel gets the caret bidi level
   *  This method is virtual since it gets called from outside of layout.
   */
  virtual PRUint8 GetCaretBidiLevel() const;
  /** UndefineCaretBidiLevel sets the caret bidi level to "undefined"
   *  This method is virtual since it gets called from outside of layout.
   */
  virtual void UndefineCaretBidiLevel();
#endif

  /** CharacterMove will generally be called from the nsiselectioncontroller implementations.
   *  the effect being the selection will move one character left or right.
   * @param aForward move forward in document.
   * @param aExtend continue selection
   */
  /*unsafe*/
  nsresult CharacterMove(PRBool aForward, PRBool aExtend);

  /** CharacterExtendForDelete extends the selection forward (logically) to
   * the next character cell, so that the selected cell can be deleted.
   */
  /*unsafe*/
  nsresult CharacterExtendForDelete();

  /** WordMove will generally be called from the nsiselectioncontroller implementations.
   *  the effect being the selection will move one word left or right.
   * @param aForward move forward in document.
   * @param aExtend continue selection
   */
  /*unsafe*/
  nsresult WordMove(PRBool aForward, PRBool aExtend);

  /** WordExtendForDelete extends the selection backward or forward (logically) to the
   *  next word boundary, so that the selected word can be deleted.
   * @param aForward select forward in document.
   */
  /*unsafe*/
  nsresult WordExtendForDelete(PRBool aForward);
  
  /** LineMove will generally be called from the nsiselectioncontroller implementations.
   *  the effect being the selection will move one line up or down.
   * @param aForward move forward in document.
   * @param aExtend continue selection
   */
  /*unsafe*/
  nsresult LineMove(PRBool aForward, PRBool aExtend);

  /** IntraLineMove will generally be called from the nsiselectioncontroller implementations.
   *  the effect being the selection will move to beginning or end of line
   * @param aForward move forward in document.
   * @param aExtend continue selection
   */
  /*unsafe*/
  nsresult IntraLineMove(PRBool aForward, PRBool aExtend); 

  /** Select All will generally be called from the nsiselectioncontroller implementations.
   *  it will select the whole doc
   */
  /*unsafe*/
  nsresult SelectAll();

  /** Sets/Gets The display selection enum.
   */
  void SetDisplaySelection(PRInt16 aState) { mDisplaySelection = aState; }
  PRInt16 GetDisplaySelection() const { return mDisplaySelection; }

  /** This method can be used to store the data received during a MouseDown
   *  event so that we can place the caret during the MouseUp event.
   * @aMouseEvent the event received by the selection MouseDown
   *  handling method. A NULL value can be use to tell this method
   *  that any data is storing is no longer valid.
   */
  void SetDelayedCaretData(nsMouseEvent *aMouseEvent);

  /** Get the delayed MouseDown event data necessary to place the
   *  caret during MouseUp processing.
   * @return a pointer to the event received
   *  by the selection during MouseDown processing. It can be NULL
   *  if the data is no longer valid.
   */
  nsMouseEvent* GetDelayedCaretData();

  /** Get the content node that limits the selection
   *  When searching up a nodes for parents, as in a text edit field
   *    in an browser page, we must stop at this node else we reach into the 
   *    parent page, which is very bad!
   */
  nsIContent* GetLimiter() const { return mLimiter; }

  nsIContent* GetAncestorLimiter() const { return mAncestorLimiter; }
  /*unsafe*/
  void SetAncestorLimiter(nsIContent *aLimiter);

  /** This will tell the frame selection that a double click has been pressed 
   *  so it can track abort future drags if inside the same selection
   *  @aDoubleDown has the double click down happened
   */
  void SetMouseDoubleDown(PRBool aDoubleDown) { mMouseDoubleDownState = aDoubleDown; }
  
  /** This will return whether the double down flag was set.
   *  @return whether the double down flag was set
   */
  PRBool GetMouseDoubleDown() const { return mMouseDoubleDownState; }

  /** GetPrevNextBidiLevels will return the frames and associated Bidi levels of the characters
   *   logically before and after a (collapsed) selection.
   *  @param aNode is the node containing the selection
   *  @param aContentOffset is the offset of the selection in the node
   *  @param aJumpLines If PR_TRUE, look across line boundaries.
   *                    If PR_FALSE, behave as if there were base-level frames at line edges.  
   *
   *  @return A struct holding the before/after frame and the before/after level.
   *
   *  At the beginning and end of each line there is assumed to be a frame with
   *   Bidi level equal to the paragraph embedding level.
   *  In these cases the before frame and after frame respectively will be 
   *   nsnull.
   *
   *  This method is virtual since it gets called from outside of layout. 
   */
  virtual nsPrevNextBidiLevels GetPrevNextBidiLevels(nsIContent *aNode,
                                                     PRUint32 aContentOffset,
                                                     PRBool aJumpLines) const;

  /** GetFrameFromLevel will scan in a given direction
   *   until it finds a frame with a Bidi level less than or equal to a given level.
   *   It will return the last frame before this.
   *  @param aPresContext is the context to use
   *  @param aFrameIn is the frame to start from
   *  @param aDirection is the direction to scan
   *  @param aBidiLevel is the level to search for
   *  @param aFrameOut will hold the frame returned
   */
  nsresult GetFrameFromLevel(nsIFrame *aFrameIn,
                             nsDirection aDirection,
                             PRUint8 aBidiLevel,
                             nsIFrame **aFrameOut) const;

  /**
   * MaintainSelection will track the current selection as being "sticky".
   * Dragging or extending selection will never allow for a subset
   * (or the whole) of the maintained selection to become unselected.
   * Primary use: double click selecting then dragging on second click
   * @param aAmount the initial amount of text selected (word, line or paragraph).
   *                For "line", use eSelectBeginLine.
   */
  nsresult MaintainSelection(nsSelectionAmount aAmount = eSelectNoAmount);


  nsFrameSelection();
  virtual ~nsFrameSelection();

  void StartBatchChanges();
  void EndBatchChanges();
  /*unsafe*/
  nsresult DeleteFromDocument();

  nsIPresShell *GetShell()const  { return mShell; }

  void DisconnectFromPresShell() { mShell = nsnull; }
private:
  nsresult TakeFocus(nsIContent *aNewFocus,
                     PRUint32 aContentOffset,
                     PRUint32 aContentEndOffset, 
                     PRBool aContinueSelection,
                     PRBool aMultipleSelection);

  void BidiLevelFromMove(nsIPresShell* aPresShell,
                         nsIContent *aNode,
                         PRUint32 aContentOffset,
                         PRUint32 aKeycode,
                         HINT aHint);
  void BidiLevelFromClick(nsIContent *aNewFocus, PRUint32 aContentOffset);
  nsPrevNextBidiLevels GetPrevNextBidiLevels(nsIContent *aNode,
                                             PRUint32 aContentOffset,
                                             HINT aHint,
                                             PRBool aJumpLines) const;
#ifdef VISUALSELECTION
  NS_IMETHOD VisualSelectFrames(nsIFrame* aCurrentFrame,
                                nsPeekOffsetStruct aPos);
  NS_IMETHOD VisualSequence(nsIFrame* aSelectFrame,
                            nsIFrame* aCurrentFrame,
                            nsPeekOffsetStruct* aPos,
                            PRBool* aNeedVisualSelection);
  NS_IMETHOD SelectToEdge(nsIFrame *aFrame,
                          nsIContent *aContent,
                          PRInt32 aOffset,
                          PRInt32 aEdge,
                          PRBool aMultipleSelection);
  NS_IMETHOD SelectLines(nsDirection aSelectionDirection,
                         nsIDOMNode *aAnchorNode,
                         nsIFrame* aAnchorFrame,
                         PRInt32 aAnchorOffset,
                         nsIDOMNode *aCurrentNode,
                         nsIFrame* aCurrentFrame,
                         PRInt32 aCurrentOffset,
                         nsPeekOffsetStruct aPos);
#endif // VISUALSELECTION

  PRBool AdjustForMaintainedSelection(nsIContent *aContent, PRInt32 aOffset);

// post and pop reasons for notifications. we may stack these later
  void    PostReason(PRInt16 aReason) { mSelectionChangeReason = aReason; }
  PRInt16 PopReason()
  {
    PRInt16 retval = mSelectionChangeReason;
    mSelectionChangeReason = 0;
    return retval;
  }

  friend class nsTypedSelection; 
#ifdef DEBUG
  void printSelection();       // for debugging
#endif /* DEBUG */

  void ResizeBuffer(PRUint32 aNewBufSize);
/*HELPER METHODS*/
  nsresult     MoveCaret(PRUint32 aKeycode, PRBool aContinueSelection, nsSelectionAmount aAmount);

  nsresult     FetchDesiredX(nscoord &aDesiredX); //the x position requested by the Key Handling for up down
  void         InvalidateDesiredX(); //do not listen to mDesiredX you must get another.
  void         SetDesiredX(nscoord aX); //set the mDesiredX

  nsresult     GetRootForContentSubtree(nsIContent *aContent, nsIContent **aParent);
  nsresult     ConstrainFrameAndPointToAnchorSubtree(nsIFrame *aFrame, nsPoint& aPoint, nsIFrame **aRetFrame, nsPoint& aRetPoint);

  PRUint32     GetBatching() const {return mBatching; }
  PRBool       GetNotifyFrames() const { return mNotifyFrames; }
  void         SetDirty(PRBool aDirty=PR_TRUE){if (mBatching) mChangesDuringBatching = aDirty;}

  // nsFrameSelection may get deleted when calling this,
  // so remember to use nsCOMPtr when needed.
  nsresult     NotifySelectionListeners(SelectionType aType);     // add parameters to say collapsed etc?

  nsTypedSelection *mDomSelections[nsISelectionController::NUM_SELECTIONTYPES];

  // Table selection support.
  // Interfaces that let us get info based on cellmap locations
  nsITableLayout* GetTableLayout(nsIContent *aTableContent) const;
  nsITableCellLayout* GetCellLayout(nsIContent *aCellContent) const;

  nsresult SelectBlockOfCells(nsIContent *aStartNode, nsIContent *aEndNode);
  nsresult SelectRowOrColumn(nsIContent *aCellContent, PRUint32 aTarget);
  nsresult GetCellIndexes(nsIContent *aCell, PRInt32 &aRowIndex, PRInt32 &aColIndex);

  nsresult GetFirstSelectedCellAndRange(nsIDOMNode **aCell, nsIDOMRange **aRange);
  nsresult GetNextSelectedCellAndRange(nsIDOMNode **aCell, nsIDOMRange **aRange);
  nsresult GetFirstCellNodeInRange(nsIDOMRange *aRange,
                                   nsIDOMNode **aCellNode) const;
  // aTableNode may be null if table isn't needed to be returned
  PRBool   IsInSameTable(nsIContent *aContent1, nsIContent *aContent2,
                         nsIContent **aTableNode) const;
  nsresult GetParentTable(nsIContent *aCellNode,
                          nsIContent **aTableNode) const;
  nsresult SelectCellElement(nsIDOMElement* aCellElement);
  nsresult CreateAndAddRange(nsIDOMNode *aParentNode, PRInt32 aOffset);
  nsresult ClearNormalSelection();

  nsCOMPtr<nsIDOMNode> mCellParent; //used to snap to table selection
  nsCOMPtr<nsIContent> mStartSelectedCell;
  nsCOMPtr<nsIContent> mEndSelectedCell;
  nsCOMPtr<nsIContent> mAppendStartSelectedCell;
  nsCOMPtr<nsIContent> mUnselectCellOnMouseUp;
  PRInt32  mSelectingTableCellMode;
  PRInt32  mSelectedCellIndex;

  // maintain selection
  nsCOMPtr<nsIDOMRange> mMaintainRange;
  nsSelectionAmount mMaintainedAmount;

  //batching
  PRInt32 mBatching;
    
  nsIContent *mLimiter;     //limit selection navigation to a child of this node.
  nsIContent *mAncestorLimiter; // Limit selection navigation to a descendant of
                                // this node.
  nsIPresShell *mShell;

  PRInt16 mSelectionChangeReason; // reason for notifications of selection changing
  PRInt16 mDisplaySelection; //for visual display purposes.

  HINT  mHint;   //hint to tell if the selection is at the end of this line or beginning of next
#ifdef IBMBIDI
  PRUint8 mCaretBidiLevel;
#endif

  PRInt32 mDesiredX;
  nsIScrollableViewProvider* mScrollableViewProvider;

  nsMouseEvent mDelayedMouseEvent;

  PRPackedBool mDelayedMouseEventValid;

  PRPackedBool mChangesDuringBatching;
  PRPackedBool mNotifyFrames;
  PRPackedBool mIsEditor;
  PRPackedBool mDragSelectingCells;
  PRPackedBool mMouseDownState;   //for drag purposes
  PRPackedBool mMouseDoubleDownState; //has the doubleclick down happened
  PRPackedBool mDesiredXSet;

  PRInt8 mCaretMovementStyle;
};

NS_DEFINE_STATIC_IID_ACCESSOR(nsFrameSelection, NS_FRAME_SELECTION_IID)

#endif /* nsFrameSelection_h___ */
