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

/* base class of all rendering objects */

#ifndef nsFrame_h___
#define nsFrame_h___

#include "nsBox.h"
#include "nsRect.h"
#include "nsString.h"
#include "prlog.h"
#ifdef NS_DEBUG
#include "nsIFrameDebug.h"
#endif

#include "nsIPresShell.h"
#include "nsHTMLReflowCommand.h"
#include "nsFrameSelection.h"
#include "nsHTMLReflowState.h"
#include "nsHTMLReflowMetrics.h"

/**
 * nsFrame logging constants. We redefine the nspr
 * PRLogModuleInfo.level field to be a bitfield.  Each bit controls a
 * specific type of logging. Each logging operation has associated
 * inline methods defined below.
 */
#define NS_FRAME_TRACE_CALLS        0x1
#define NS_FRAME_TRACE_PUSH_PULL    0x2
#define NS_FRAME_TRACE_CHILD_REFLOW 0x4
#define NS_FRAME_TRACE_NEW_FRAMES   0x8

#define NS_FRAME_LOG_TEST(_lm,_bit) (PRIntn((_lm)->level) & (_bit))

#ifdef NS_DEBUG
#define NS_FRAME_LOG(_bit,_args)                                \
  PR_BEGIN_MACRO                                                \
    if (NS_FRAME_LOG_TEST(nsIFrameDebug::GetLogModuleInfo(),_bit)) { \
      PR_LogPrint _args;                                        \
    }                                                           \
  PR_END_MACRO
#else
#define NS_FRAME_LOG(_bit,_args)
#endif

// XXX Need to rework this so that logging is free when it's off
#ifdef NS_DEBUG
#define NS_FRAME_TRACE_IN(_method) Trace(_method, PR_TRUE)

#define NS_FRAME_TRACE_OUT(_method) Trace(_method, PR_FALSE)

// XXX remove me
#define NS_FRAME_TRACE_MSG(_bit,_args)                          \
  PR_BEGIN_MACRO                                                \
    if (NS_FRAME_LOG_TEST(nsIFrameDebug::GetLogModuleInfo(),_bit)) { \
      TraceMsg _args;                                           \
    }                                                           \
  PR_END_MACRO

#define NS_FRAME_TRACE(_bit,_args)                              \
  PR_BEGIN_MACRO                                                \
    if (NS_FRAME_LOG_TEST(nsIFrameDebug::GetLogModuleInfo(),_bit)) { \
      TraceMsg _args;                                           \
    }                                                           \
  PR_END_MACRO

#define NS_FRAME_TRACE_REFLOW_IN(_method) Trace(_method, PR_TRUE)

#define NS_FRAME_TRACE_REFLOW_OUT(_method, _status) \
  Trace(_method, PR_FALSE, _status)

#else
#define NS_FRAME_TRACE(_bits,_args)
#define NS_FRAME_TRACE_IN(_method)
#define NS_FRAME_TRACE_OUT(_method)
#define NS_FRAME_TRACE_MSG(_bits,_args)
#define NS_FRAME_TRACE_REFLOW_IN(_method)
#define NS_FRAME_TRACE_REFLOW_OUT(_method, _status)
#endif

// handy utilities
void SetFontFromStyle(nsIRenderingContext* aRC, nsStyleContext* aSC);

//----------------------------------------------------------------------

struct nsBoxLayoutMetrics;

/**
 * Implementation of a simple frame that's not splittable and has no
 * child frames.
 *
 * Sets the NS_FRAME_SYNCHRONIZE_FRAME_AND_VIEW bit, so the default
 * behavior is to keep the frame and view position and size in sync.
 */
class nsFrame : public nsBox
#ifdef NS_DEBUG
  , public nsIFrameDebug
#endif
{
public:
  /**
   * Create a new "empty" frame that maps a given piece of content into a
   * 0,0 area.
   */
  friend nsIFrame* NS_NewEmptyFrame(nsIPresShell* aShell, nsStyleContext* aContext);

  // Overloaded new operator. Initializes the memory to 0 and relies on an arena
  // (which comes from the presShell) to perform the allocation.
  void* operator new(size_t sz, nsIPresShell* aPresShell) CPP_THROW_NEW;

  // Overridden to prevent the global delete from being called, since the memory
  // came out of an arena instead of the global delete operator's heap.  
  // XXX Would like to make this private some day, but our UNIX compilers can't 
  // deal with it.
  void operator delete(void* aPtr, size_t sz);

  // We compute and store the HTML content's overflow area. So don't
  // try to compute it in the box code.
  virtual PRBool ComputesOwnOverflowArea() { return PR_TRUE; }

private:
  // The normal operator new is disallowed on nsFrames.
  void* operator new(size_t sz) CPP_THROW_NEW { return nsnull; };

public:

  // nsISupports
  NS_IMETHOD  QueryInterface(const nsIID& aIID, void** aInstancePtr);

  // nsIFrame
  NS_IMETHOD  Init(nsIContent*      aContent,
                   nsIFrame*        aParent,
                   nsIFrame*        asPrevInFlow);
  NS_IMETHOD  SetInitialChildList(nsIAtom*        aListName,
                                  nsIFrame*       aChildList);
  NS_IMETHOD  AppendFrames(nsIAtom*        aListName,
                           nsIFrame*       aFrameList);
  NS_IMETHOD  InsertFrames(nsIAtom*        aListName,
                           nsIFrame*       aPrevFrame,
                           nsIFrame*       aFrameList);
  NS_IMETHOD  RemoveFrame(nsIAtom*        aListName,
                          nsIFrame*       aOldFrame);
  virtual void Destroy();
  NS_IMETHOD  CalcBorderPadding(nsMargin& aBorderPadding) const;
  virtual nsStyleContext* GetAdditionalStyleContext(PRInt32 aIndex) const;
  virtual void SetAdditionalStyleContext(PRInt32 aIndex,
                                         nsStyleContext* aStyleContext);
  NS_IMETHOD  SetParent(const nsIFrame* aParent);
  virtual nsIAtom* GetAdditionalChildListName(PRInt32 aIndex) const;
  virtual nsIFrame* GetFirstChild(nsIAtom* aListName) const;
  NS_IMETHOD  HandleEvent(nsPresContext* aPresContext, 
                          nsGUIEvent*     aEvent,
                          nsEventStatus*  aEventStatus);
  NS_IMETHOD  GetContentForEvent(nsPresContext* aPresContext,
                                 nsEvent* aEvent,
                                 nsIContent** aContent);
  NS_IMETHOD  GetCursor(const nsPoint&    aPoint,
                        nsIFrame::Cursor& aCursor);

  NS_IMETHOD  GetPointFromOffset(nsPresContext*        inPresContext,
                                 nsIRenderingContext*   inRendContext,
                                 PRInt32                inOffset,
                                 nsPoint*               outPoint);

  NS_IMETHOD  GetChildFrameContainingOffset(PRInt32     inContentOffset,
                                 PRBool                 inHint,
                                 PRInt32*               outFrameContentOffset,
                                 nsIFrame*              *outChildFrame);

  static nsresult  GetNextPrevLineFromeBlockFrame(nsPresContext* aPresContext,
                                        nsPeekOffsetStruct *aPos, 
                                        nsIFrame *aBlockFrame, 
                                        PRInt32 aLineStart, 
                                        PRInt8 aOutSideLimit
                                        );

  /**
   * Find the nearest frame with a mouse capturer. If no
   * parent has mouse capture this will return null.
   * @param aFrame Frame drag began in.
   * @return Nearest capturing frame.
   */
  static nsIFrame* GetNearestCapturingFrame(nsIFrame* aFrame);

  NS_IMETHOD  CharacterDataChanged(nsPresContext* aPresContext,
                                   nsIContent*     aChild,
                                   PRBool          aAppend);
  NS_IMETHOD  AttributeChanged(PRInt32         aNameSpaceID,
                               nsIAtom*        aAttribute,
                               PRInt32         aModType);
  NS_IMETHOD  IsSplittable(nsSplittableType& aIsSplittable) const;
  virtual nsIFrame* GetPrevContinuation() const;
  NS_IMETHOD  SetPrevContinuation(nsIFrame*);
  virtual nsIFrame* GetNextContinuation() const;
  NS_IMETHOD  SetNextContinuation(nsIFrame*);
  virtual nsIFrame* GetPrevInFlowVirtual() const;
  NS_IMETHOD  SetPrevInFlow(nsIFrame*);
  virtual nsIFrame* GetNextInFlowVirtual() const;
  NS_IMETHOD  SetNextInFlow(nsIFrame*);
  NS_IMETHOD  GetOffsetFromView(nsPoint& aOffset, nsIView** aView) const;
  NS_IMETHOD  GetOriginToViewOffset(nsPoint& aOffset, nsIView **aView) const;
  virtual nsIAtom* GetType() const;
  virtual PRBool IsContainingBlock() const;
#ifdef NS_DEBUG
  NS_IMETHOD  List(FILE* out, PRInt32 aIndent) const;
  NS_IMETHOD  GetFrameName(nsAString& aResult) const;
  NS_IMETHOD_(nsFrameState) GetDebugStateBits() const;
  NS_IMETHOD  DumpRegressionData(nsPresContext* aPresContext, FILE* out, PRInt32 aIndent, PRBool aIncludeStyleData);
  NS_IMETHOD  VerifyTree() const;
#endif

  NS_IMETHOD  SetSelected(nsPresContext* aPresContext, nsIDOMRange *aRange,PRBool aSelected, nsSpread aSpread);
  NS_IMETHOD  GetSelected(PRBool *aSelected) const;
  NS_IMETHOD  IsSelectable(PRBool* aIsSelectable, PRUint8* aSelectStyle) const;

  NS_IMETHOD  GetSelectionController(nsPresContext *aPresContext, nsISelectionController **aSelCon);
  NS_IMETHOD  PeekOffset(nsPresContext* aPresContext, nsPeekOffsetStruct *aPos) ;
  NS_IMETHOD  CheckVisibility(nsPresContext* aContext, PRInt32 aStartIndex, PRInt32 aEndIndex, PRBool aRecurse, PRBool *aFinished, PRBool *_retval);

  NS_IMETHOD  PeekOffsetParagraph(nsPresContext* aPresContext,
                                  nsPeekOffsetStruct *aPos);
  NS_IMETHOD  GetOffsets(PRInt32 &aStart, PRInt32 &aEnd) const;
  NS_IMETHOD  ReflowDirtyChild(nsIPresShell* aPresShell, nsIFrame* aChild);

#ifdef ACCESSIBILITY
  NS_IMETHOD  GetAccessible(nsIAccessible** aAccessible);
#endif

  NS_IMETHOD GetParentStyleContextFrame(nsPresContext* aPresContext,
                                        nsIFrame**      aProviderFrame,
                                        PRBool*         aIsChild);

  virtual PRBool IsEmpty();
  virtual PRBool IsSelfEmpty();

  // nsIHTMLReflow
  NS_IMETHOD  WillReflow(nsPresContext* aPresContext);
  NS_IMETHOD  Reflow(nsPresContext*          aPresContext,
                     nsHTMLReflowMetrics&     aDesiredSize,
                     const nsHTMLReflowState& aReflowState,
                     nsReflowStatus&          aStatus);
  NS_IMETHOD  DidReflow(nsPresContext*           aPresContext,
                        const nsHTMLReflowState*  aReflowState,
                        nsDidReflowStatus         aStatus);
  NS_IMETHOD CanContinueTextRun(PRBool& aContinueTextRun) const;
  NS_IMETHOD AdjustFrameSize(nscoord aExtraSpace, nscoord& aUsedSpace);
  NS_IMETHOD TrimTrailingWhiteSpace(nsPresContext* aPresContext,
                                    nsIRenderingContext& aRC,
                                    nscoord& aDeltaWidth,
                                    PRBool& aLastCharIsJustifiable);

  // Selection Methods
  // XXX Doc me... (in nsIFrame.h puhleeze)
  // XXX If these are selection specific, then the name should imply selection
  // rather than generic event processing, e.g., SelectionHandlePress...
  NS_IMETHOD HandlePress(nsPresContext* aPresContext,
                         nsGUIEvent *    aEvent,
                         nsEventStatus*  aEventStatus);

  NS_IMETHOD HandleMultiplePress(nsPresContext* aPresContext,
                         nsGUIEvent *    aEvent,
                         nsEventStatus*  aEventStatus);

  NS_IMETHOD HandleDrag(nsPresContext* aPresContext,
                        nsGUIEvent *    aEvent,
                        nsEventStatus*  aEventStatus);

  NS_IMETHOD HandleRelease(nsPresContext* aPresContext,
                           nsGUIEvent *    aEvent,
                           nsEventStatus*  aEventStatus);

  NS_IMETHOD PeekBackwardAndForward(nsSelectionAmount aAmountBack,
                                    nsSelectionAmount aAmountForward,
                                    PRInt32 aStartPos,
                                    nsPresContext* aPresContext,
                                    PRBool aJumpLines);


  // Helper for GetContentAndOffsetsFromPoint; calculation of content offsets
  // in this function assumes there is no child frame that can be targeted.
  virtual ContentOffsets CalcContentOffsetsFromFramePoint(nsPoint aPoint);

  // Box layout methods
  NS_IMETHOD GetPrefSize(nsBoxLayoutState& aBoxLayoutState, nsSize& aSize);
  NS_IMETHOD GetMinSize(nsBoxLayoutState& aBoxLayoutState, nsSize& aSize);
  NS_IMETHOD GetMaxSize(nsBoxLayoutState& aBoxLayoutState, nsSize& aSize);
  NS_IMETHOD GetFlex(nsBoxLayoutState& aBoxLayoutState, nscoord& aFlex);
  NS_IMETHOD GetAscent(nsBoxLayoutState& aBoxLayoutState, nscoord& aAscent);
  NS_IMETHOD SetIncludeOverflow(PRBool aInclude);
  NS_IMETHOD GetOverflow(nsSize& aOverflow);
  NS_IMETHOD NeedsRecalc();

  //--------------------------------------------------
  // Additional methods

  /**
   * Helper method to invalidate portions of a standard container frame if the
   * reflow state indicates that the size has changed (specifically border,
   * background and outline).
   * We assume that the difference between the old frame area and the new
   * frame area is invalidated by some other means.
   * @param aPresContext the presentation context
   * @param aDesiredSize the new size of the frame
   * @param aReflowState the reflow that was just done on this frame
   */
  void CheckInvalidateSizeChange(nsPresContext*          aPresContext,
                                 nsHTMLReflowMetrics&     aDesiredSize,
                                 const nsHTMLReflowState& aReflowState);

  // Helper function that tests if the frame tree is too deep; if it
  // is it marks the frame as "unflowable" and zeros out the metrics
  // and returns PR_TRUE. Otherwise, the frame is unmarked
  // "unflowable" and the metrics are not touched and PR_FALSE is
  // returned.
  PRBool IsFrameTreeTooDeep(const nsHTMLReflowState& aReflowState,
                            nsHTMLReflowMetrics& aMetrics);

  // Do the work for getting the parent style context frame so that
  // other frame's |GetParentStyleContextFrame| methods can call this
  // method on *another* frame.  (This function handles out-of-flow
  // frames by using the frame manager's placeholder map and it also
  // handles block-within-inline and generated content wrappers.)
  nsresult DoGetParentStyleContextFrame(nsPresContext* aPresContext,
                                        nsIFrame**      aProviderFrame,
                                        PRBool*         aIsChild);

  // incorporate the child overflow area into the parent overflow area
  // if the child does not have a overflow use the child area
  void ConsiderChildOverflow(nsRect&   aOverflowArea,
                             nsIFrame* aChildFrame);

  //Mouse Capturing code used by the frames to tell the view to capture all the following events
  NS_IMETHOD CaptureMouse(nsPresContext* aPresContext, PRBool aGrabMouseEvents);
  PRBool   IsMouseCaptured(nsPresContext* aPresContext);

  virtual const nsStyleStruct* GetStyleDataExternal(nsStyleStructID aSID) const;


#ifdef NS_DEBUG
  /**
   * Tracing method that writes a method enter/exit routine to the
   * nspr log using the nsIFrame log module. The tracing is only
   * done when the NS_FRAME_TRACE_CALLS bit is set in the log module's
   * level field.
   */
  void Trace(const char* aMethod, PRBool aEnter);
  void Trace(const char* aMethod, PRBool aEnter, nsReflowStatus aStatus);
  void TraceMsg(const char* fmt, ...);

  // Helper function that verifies that each frame in the list has the
  // NS_FRAME_IS_DIRTY bit set
  static void VerifyDirtyBitSet(nsIFrame* aFrameList);

  // Helper function to return the index in parent of the frame's content
  // object. Returns -1 on error or if the frame doesn't have a content object
  static PRInt32 ContentIndexInContainer(const nsIFrame* aFrame);

  void ListTag(FILE* out) const {
    ListTag(out, (nsIFrame*)this);
  }

  static void ListTag(FILE* out, nsIFrame* aFrame) {
    nsAutoString tmp;
    nsIFrameDebug*  frameDebug;

    if (NS_SUCCEEDED(aFrame->QueryInterface(NS_GET_IID(nsIFrameDebug), (void**)&frameDebug))) {
      frameDebug->GetFrameName(tmp);
    }
    fputs(NS_LossyConvertUTF16toASCII(tmp).get(), out);
    fprintf(out, "@%p", NS_STATIC_CAST(void*, aFrame));
  }

  static void IndentBy(FILE* out, PRInt32 aIndent) {
    while (--aIndent >= 0) fputs("  ", out);
  }
  
  static void XMLQuote(nsString& aString);

  /**
   * Dump out the "base classes" regression data. This should dump
   * out the interior data, not the "frame" XML container. And it
   * should call the base classes same named method before doing
   * anything specific in a derived class. This means that derived
   * classes need not override DumpRegressionData unless they need
   * some custom behavior that requires changing how the outer "frame"
   * XML container is dumped.
   */
  virtual void DumpBaseRegressionData(nsPresContext* aPresContext, FILE* out, PRInt32 aIndent, PRBool aIncludeStyleData);
  
  nsresult MakeFrameName(const nsAString& aKind, nsAString& aResult) const;

  // Display Reflow Debugging 
  static void* DisplayReflowEnter(nsPresContext*          aPresContext,
                                  nsIFrame*                aFrame,
                                  const nsHTMLReflowState& aReflowState);
  static void  DisplayReflowExit(nsPresContext*      aPresContext,
                                 nsIFrame*            aFrame,
                                 nsHTMLReflowMetrics& aMetrics,
                                 PRUint32             aStatus,
                                 void*                aFrameTreeNode);

  static void DisplayReflowStartup();
  static void DisplayReflowShutdown();
#endif

  /**
   * Adds display items for standard CSS borders, background and outline for
   * for this frame, as necessary. Checks IsVisibleForPainting and won't
   * display anything if the frame is not visible.
   * @param aForceBackground draw the background even if the frame
   * background style appears to have no background --- this is useful
   * for frames that might receive a propagated background via
   * nsCSSRendering::FindBackground
   */
  nsresult DisplayBorderBackgroundOutline(nsDisplayListBuilder*   aBuilder,
                                          const nsDisplayListSet& aLists,
                                          PRBool aForceBackground = PR_FALSE);
  /**
   * Add a display item for the CSS outline. Does not check visibility.
   */
  nsresult DisplayOutlineUnconditional(nsDisplayListBuilder*   aBuilder,
                                       const nsDisplayListSet& aLists);
  /**
   * Add a display item for the CSS outline, after calling
   * IsVisibleForPainting to confirm we are visible.
   */
  nsresult DisplayOutline(nsDisplayListBuilder*   aBuilder,
                          const nsDisplayListSet& aLists);

protected:
  // Protected constructor and destructor
  nsFrame(nsStyleContext* aContext);
  virtual ~nsFrame();

  /**
   * @return PR_FALSE if this frame definitely has no borders at all
   */                 
  PRBool HasBorder() {
    const nsStyleBorder* border = GetStyleBorder();
    for (PRInt32 i = 0; i < 4; ++i) {
      if (border->GetBorderStyle(i) != NS_STYLE_BORDER_STYLE_NONE)
        return PR_TRUE;
    }
    return PR_FALSE;
  }

  /**
   * To be called by |BuildDisplayLists| of this class or derived classes to add
   * a translucent overlay if this frame's content is selected.
   * @param aContentType an nsISelectionDisplay DISPLAY_ constant identifying
   * which kind of content this is for
   */
  nsresult DisplaySelectionOverlay(nsDisplayListBuilder* aBuilder,
      const nsDisplayListSet& aLists, PRUint16 aContentType = nsISelectionDisplay::DISPLAY_FRAMES);

  PRInt16 DisplaySelection(nsPresContext* aPresContext, PRBool isOkToTurnOn = PR_FALSE);
  
  //this will modify aPos and return the next frame etc.
  NS_IMETHOD GetFrameFromDirection(nsPresContext* aPresContext, nsPeekOffsetStruct *aPos);

  // Style post processing hook
  NS_IMETHOD DidSetStyleContext();

  // return the line number of the aFrame, and (optionally) the containing block frame.
  static PRInt32 GetLineNumber(nsIFrame *aFrame, nsIFrame** aContainingBlock = nsnull);

public:
  //given a frame five me the first/last leaf available
  //XXX Robert O'Callahan wants to move these elsewhere
  static void GetLastLeaf(nsPresContext* aPresContext, nsIFrame **aFrame);
  static void GetFirstLeaf(nsPresContext* aPresContext, nsIFrame **aFrame);
  
protected:

  // Test if we are selecting a table object:
  //  Most table/cell selection requires that Ctrl (Cmd on Mac) key is down 
  //   during a mouse click or drag. Exception is using Shift+click when
  //   already in "table/cell selection mode" to extend a block selection
  //  Get the parent content node and offset of the frame 
  //   of the enclosing cell or table (if not inside a cell)
  //  aTarget tells us what table element to select (currently only cell and table supported)
  //  (enums for this are defined in nsIFrame.h)
  NS_IMETHOD GetDataForTableSelection(nsFrameSelection *aFrameSelection,
                                      nsIPresShell *aPresShell, nsMouseEvent *aMouseEvent, 
                                      nsIContent **aParentContent, PRInt32 *aContentOffset, 
                                      PRInt32 *aTarget);

  virtual PRBool ParentDisablesSelection() const;

  // Fills aCursor with the appropriate information from ui
  static void FillCursorInformationFromStyle(const nsStyleUserInterface* ui,
                                             nsIFrame::Cursor& aCursor);
  NS_IMETHOD DoLayout(nsBoxLayoutState& aBoxLayoutState);

#ifdef DEBUG_LAYOUT
  virtual void GetBoxName(nsAutoString& aName);
#endif
  virtual PRBool HasStyleChange();
  virtual void SetStyleChangeFlag(PRBool aDirty);

  virtual PRBool GetWasCollapsed(nsBoxLayoutState& aState);
  virtual void SetWasCollapsed(nsBoxLayoutState& aState, PRBool aWas);

  void InitBoxMetrics(PRBool aClear);
  nsBoxLayoutMetrics* BoxMetrics() const;

  // Fire DOM event. If no aContent argument use frame's mContent.
  void FireDOMEvent(const nsAString& aDOMEventName, nsIContent *aContent = nsnull);

private:
  nsresult BoxReflow(nsBoxLayoutState& aState,
                     nsPresContext*    aPresContext,
                     nsHTMLReflowMetrics&     aDesiredSize,
                     const nsHTMLReflowState& aReflowState,
                     nsReflowStatus&          aStatus,
                     nscoord aX,
                     nscoord aY,
                     nscoord aWidth,
                     nscoord aHeight,
                     PRBool aMoveFrame = PR_TRUE);

  void HandleIncrementalReflow(nsBoxLayoutState& aState, 
                               const nsHTMLReflowState& aReflowState, 
                               nsReflowReason& aReason,
                               nsReflowPath** aReflowPath,
                               PRBool& aRedrawNow,
                               PRBool& aNeedReflow,
                               PRBool& aRedrawAfterReflow,
                               PRBool& aMoveFrame);

  PRBool CanSetMaxElementWidth(nsBoxLayoutState& aState,
                               nsReflowReason& aReason,
                               nsReflowPath **aReflowPath);

  NS_IMETHODIMP RefreshSizeCache(nsBoxLayoutState& aState);

protected:
  NS_IMETHOD_(nsrefcnt) AddRef(void);
  NS_IMETHOD_(nsrefcnt) Release(void);
};

// Start Display Reflow Debugging
#ifdef DEBUG

  struct DR_cookie {
    DR_cookie(nsPresContext*          aPresContext,
              nsIFrame*                aFrame, 
              const nsHTMLReflowState& aReflowState,
              nsHTMLReflowMetrics&     aMetrics,
              nsReflowStatus&          aStatus);     
    ~DR_cookie();
    void Change() const;

    nsPresContext*          mPresContext;
    nsIFrame*                mFrame;
    const nsHTMLReflowState& mReflowState;
    nsHTMLReflowMetrics&     mMetrics;
    nsReflowStatus&          mStatus;    
    void*                    mValue;
  };
  
#define DISPLAY_REFLOW(dr_pres_context, dr_frame, dr_rf_state, dr_rf_metrics, dr_rf_status) \
  DR_cookie dr_cookie(dr_pres_context, dr_frame, dr_rf_state, dr_rf_metrics, dr_rf_status); 
#define DISPLAY_REFLOW_CHANGE() \
  dr_cookie.Change();

#else

#define DISPLAY_REFLOW(dr_pres_context, dr_frame, dr_rf_state, dr_rf_metrics, dr_rf_status) 
#define DISPLAY_REFLOW_CHANGE() 
  
#endif
// End Display Reflow Debugging

// similar to NS_ENSURE_TRUE but with no return value
#define ENSURE_TRUE(x)                                        \
  PR_BEGIN_MACRO                                              \
    if (!(x)) {                                               \
       NS_WARNING("ENSURE_TRUE(" #x ") failed");              \
       return;                                                \
    }                                                         \
  PR_END_MACRO
#endif /* nsFrame_h___ */
