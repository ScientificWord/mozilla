/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
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
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Steve Clark <buster@netscape.com>
 *   Dan Rosen <dr@netscape.com>
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
 * ***** END LICENSE BLOCK *****
 *
 * This Original Code has been modified by IBM Corporation.
 * Modifications made by IBM described herein are
 * Copyright (c) International Business Machines
 * Corporation, 2000
 *
 * Modifications to Mozilla code or documentation
 * identified per MPL Section 3.3
 *
 * Date         Modified by     Description of modification
 * 05/03/2000   IBM Corp.       Observer related defines for reflow
 */

/* a presentation of a document, part 2 */

#ifndef nsIPresShell_h___
#define nsIPresShell_h___

#include "nsISupports.h"
#include "nsCoord.h"
#include "nsRect.h"
#include "nsColor.h"
#include "nsEvent.h"
#include "nsReflowType.h"
#include "nsCompatibility.h"
#include "nsFrameManagerBase.h"
#include "mozFlushType.h"
#include "nsWeakReference.h"
#include <stdio.h> // for FILE definition

class nsIAtom;
class nsIContent;
class nsIContentIterator;
class nsIDocument;
class nsIDocumentObserver;
class nsIFrame;
class nsPresContext;
class nsStyleSet;
class nsIViewManager;
class nsIDeviceContext;
class nsIRenderingContext;
class nsIPageSequenceFrame;
class nsString;
class nsAString;
class nsStringArray;
class nsICaret;
class nsStyleContext;
class nsFrameSelection;
class nsFrameManager;
class nsILayoutHistoryState;
class nsIReflowCallback;
class nsISupportsArray;
class nsIDOMNode;
class nsIStyleFrameConstruction;
class nsIStyleSheet;
class nsCSSFrameConstructor;
class nsISelection;
template<class E> class nsCOMArray;
class nsWeakFrame;

typedef short SelectionType;

#define NS_IPRESSHELL_IID     \
{ 0x67880b18, 0xaf91, 0x431b, \
  { 0x89, 0x69, 0xfa, 0xd9, 0x2b, 0x3c, 0x33, 0x32 } }


// Constants uses for ScrollFrameIntoView() function
#define NS_PRESSHELL_SCROLL_TOP      0
#define NS_PRESSHELL_SCROLL_BOTTOM   100
#define NS_PRESSHELL_SCROLL_LEFT     0
#define NS_PRESSHELL_SCROLL_RIGHT    100
#define NS_PRESSHELL_SCROLL_CENTER   50
#define NS_PRESSHELL_SCROLL_ANYWHERE -1
#define NS_PRESSHELL_SCROLL_IF_NOT_VISIBLE -2

// debug VerifyReflow flags
#define VERIFY_REFLOW_ON              0x01
#define VERIFY_REFLOW_NOISY           0x02
#define VERIFY_REFLOW_ALL             0x04
#define VERIFY_REFLOW_DUMP_COMMANDS   0x08
#define VERIFY_REFLOW_NOISY_RC        0x10
#define VERIFY_REFLOW_REALLY_NOISY_RC 0x20
#define VERIFY_REFLOW_INCLUDE_SPACE_MANAGER 0x40
#define VERIFY_REFLOW_DURING_RESIZE_REFLOW  0x80

#ifdef IBMBIDI // Constant for Set/Get CaretBidiLevel
#define BIDI_LEVEL_UNDEFINED 0x80
#endif

// for PostAttributeChanged
enum nsAttributeChangeType {
  eChangeType_Set = 0,       // Set attribute
  eChangeType_Remove = 1     // Remove attribute
};

/**
 * Presentation shell interface. Presentation shells are the
 * controlling point for managing the presentation of a document. The
 * presentation shell holds a live reference to the document, the
 * presentation context, the style manager, the style set and the root
 * frame. <p>
 *
 * When this object is Release'd, it will release the document, the
 * presentation context, the style manager, the style set and the root
 * frame.
 */

// hack to make egcs / gcc 2.95.2 happy
class nsIPresShell_base : public nsISupports
{
public:
  NS_DECLARE_STATIC_IID_ACCESSOR(NS_IPRESSHELL_IID)
};

NS_DEFINE_STATIC_IID_ACCESSOR(nsIPresShell_base, NS_IPRESSHELL_IID)

class nsIPresShell : public nsIPresShell_base
{
public:
  NS_IMETHOD Init(nsIDocument* aDocument,
                  nsPresContext* aPresContext,
                  nsIViewManager* aViewManager,
                  nsStyleSet* aStyleSet,
                  nsCompatibility aCompatMode) = 0;

  /**
   * All callers are responsible for calling |Destroy| after calling
   * |EndObservingDocument|.  It needs to be separate only because form
   * controls incorrectly store their data in the frames rather than the
   * content model and printing calls |EndObservingDocument| multiple
   * times to make form controls behave nicely when printed.
   */
  NS_IMETHOD Destroy() = 0;

  // All frames owned by the shell are allocated from an arena.  They are also recycled
  // using free lists (separate free lists being maintained for each size_t).
  // Methods for recycling frames.
  virtual void* AllocateFrame(size_t aSize) = 0;
  virtual void  FreeFrame(size_t aSize, void* aFreeChunk) = 0;

  // Dynamic stack memory allocation
  NS_IMETHOD PushStackMemory() = 0;
  NS_IMETHOD PopStackMemory() = 0;
  NS_IMETHOD AllocateStackMemory(size_t aSize, void** aResult) = 0;
  
  nsIDocument* GetDocument() { return mDocument; }

  nsPresContext* GetPresContext() { return mPresContext; }

  nsIViewManager* GetViewManager() { return mViewManager; }

#ifdef _IMPL_NS_LAYOUT
  nsStyleSet*  StyleSet() { return mStyleSet; }

  nsCSSFrameConstructor* FrameConstructor()
  {
    return mFrameConstructor;
  }

  nsFrameManager* FrameManager() const {
    return NS_REINTERPRET_CAST(nsFrameManager*,
      &NS_CONST_CAST(nsIPresShell*, this)->mFrameManager);
  }

#endif

  // These two methods are used only by viewer
  NS_IMETHOD GetActiveAlternateStyleSheet(nsString& aSheetTitle) = 0;

  NS_IMETHOD SelectAlternateStyleSheet(const nsString& aSheetTitle) = 0;


  /* Enable/disable author style level. Disabling author style disables the entire
   * author level of the cascade, including the HTML preshint level.
   */
  // XXX these could easily be inlined, but there is a circular #include
  // problem with nsStyleSet.
  NS_HIDDEN_(void) SetAuthorStyleDisabled(PRBool aDisabled);
  NS_HIDDEN_(PRBool) GetAuthorStyleDisabled();

  /*
   * Called when stylesheets are added/removed/enabled/disabled to rebuild
   * all style data for a given pres shell without necessarily reconstructing
   * all of the frames.  This will not reconstruct style synchronously; if
   * you need to do that, call FlushPendingNotifications to flush out style
   * reresolves.
   * // XXXbz why do we have this on the interface anyway?  The only consumer
   * is calling AddOverrideStyleSheet/RemoveOverrideStyleSheet, and I think
   * those should just handle reconstructing style data...
   */
  virtual NS_HIDDEN_(void) ReconstructStyleDataExternal();
  NS_HIDDEN_(void) ReconstructStyleDataInternal();
#ifdef _IMPL_NS_LAYOUT
  void ReconstructStyleData() { ReconstructStyleDataInternal(); }
#else
  void ReconstructStyleData() { ReconstructStyleDataExternal(); }
#endif

  /** Setup all style rules required to implement preferences
   * - used for background/text/link colors and link underlining
   *    may be extended for any prefs that are implemented via style rules
   * - aForceReflow argument is used to force a full reframe to make the rules show
   *   (only used when the current page needs to reflect changed pref rules)
   *
   * - initially created for bugs 31816, 20760, 22963
   */
  NS_IMETHOD SetPreferenceStyleRules(PRBool aForceReflow) = 0;

  /**
   * Gather titles of all selectable (alternate and preferred) style sheets
   * fills void array with nsString* caller must free strings
   */
  NS_IMETHOD ListAlternateStyleSheets(nsStringArray& aTitleList) = 0;

  /**
   * FrameSelection will return the Frame based selection API.
   * You cannot go back and forth anymore with QI between nsIDOM sel and
   * nsIFrame sel.
   */
  nsFrameSelection* FrameSelection() { return mSelection; }

  // Make shell be a document observer
  NS_IMETHOD BeginObservingDocument() = 0;

  // Make shell stop being a document observer
  NS_IMETHOD EndObservingDocument() = 0;

  /**
   * Determine if InitialReflow() was previously called.
   * @param aDidInitialReflow PR_TRUE if InitalReflow() was previously called,
   * PR_FALSE otherwise.
   */
  NS_IMETHOD GetDidInitialReflow(PRBool *aDidInitialReflow) = 0;

  /**
   * Perform the initial reflow. Constructs the frame for the root content
   * object and then reflows the frame model into the specified width and
   * height.
   *
   * The coordinates for aWidth and aHeight must be in standard nscoord's.
   */
  NS_IMETHOD InitialReflow(nscoord aWidth, nscoord aHeight) = 0;

  /**
   * Reflow the frame model into a new width and height.  The
   * coordinates for aWidth and aHeight must be in standard nscoord's.
   */
  NS_IMETHOD ResizeReflow(nscoord aWidth, nscoord aHeight) = 0;

  /**
   * Reflow the frame model with a reflow reason of eReflowReason_StyleChange
   */
  NS_IMETHOD StyleChangeReflow() = 0;

  /**
   * This calls through to the frame manager to get the root frame.
   * Callers inside of gklayout should use FrameManager()->GetRootFrame()
   * instead, as it's more efficient.
   */
  virtual NS_HIDDEN_(nsIFrame*) GetRootFrame() const;

  /*
   * Get root scroll frame from FrameManager()->GetRootFrame().
   */
  nsIFrame* GetRootScrollFrame() const;

  /**
   * Returns the page sequence frame associated with the frame hierarchy.
   * Returns NULL if not a paginated view.
   */
  NS_IMETHOD GetPageSequenceFrame(nsIPageSequenceFrame** aResult) const = 0;

  /**
   * Gets the primary frame associated with the content object. This is a
   * helper function that just forwards the request to the frame manager.
   *
   * The primary frame is the frame that is most closely associated with the
   * content. A frame is more closely associated with the content that another
   * frame if the one frame contains directly or indirectly the other frame (e.g.,
   * when a frame is scrolled there is a scroll frame that contains the frame
   * being scrolled). The primary frame is always the first-in-flow.
   *
   * In the case of absolutely positioned elements and floated elements,
   * the primary frame is the frame that is out of the flow and not the
   * placeholder frame.
   */
  virtual NS_HIDDEN_(nsIFrame*) GetPrimaryFrameFor(nsIContent* aContent) const = 0;

  /**
   * Returns a layout object associated with the primary frame for the content object.
   *
   * @param aContent   the content object for which we seek a layout object
   * @param aResult    the resulting layout object as an nsISupports, if found.  Refcounted.
   */
  NS_IMETHOD GetLayoutObjectFor(nsIContent*   aContent,
                                nsISupports** aResult) const = 0;

  /**
   * Gets the placeholder frame associated with the specified frame. This is
   * a helper frame that forwards the request to the frame manager.
   */
  NS_IMETHOD GetPlaceholderFrameFor(nsIFrame*  aFrame,
                                    nsIFrame** aPlaceholderFrame) const = 0;

  /**
   * Reflow commands
   */
  NS_IMETHOD AppendReflowCommand(nsIFrame*    aTargetFrame,
                                 nsReflowType aReflowType,
                                 nsIAtom*     aChildListName) = 0;
  // XXXbz don't we need a child list name on this too?
  NS_IMETHOD CancelReflowCommand(nsIFrame* aTargetFrame, nsReflowType* aCmdType) = 0;
  NS_IMETHOD CancelAllReflowCommands() = 0;

  /**
   * Recreates the frames for a node
   */
  NS_IMETHOD RecreateFramesFor(nsIContent* aContent) = 0;

  /**
   * Determine if it is safe to flush all pending notifications
   * @param aIsSafeToFlush PR_TRUE if it is safe, PR_FALSE otherwise.
   * 
   */
  NS_IMETHOD IsSafeToFlush(PRBool& aIsSafeToFlush) = 0;

  /**
   * Flush pending notifications of the type specified.  This method
   * will not affect the content model; it'll just affect style and
   * frames. Callers that actually want up-to-date presentation (other
   * than the document itself) should probably be calling
   * nsIDocument::FlushPendingNotifications.
   *
   * @param aType the type of notifications to flush
   */
  NS_IMETHOD FlushPendingNotifications(mozFlushType aType) = 0;

  /**
   * Post a request to handle a DOM event after Reflow has finished.
   */
  NS_IMETHOD PostDOMEvent(nsIContent* aContent, nsEvent* aEvent)=0;

  /**
   * Post a request to set and attribute after reflow has finished.
   */
  NS_IMETHOD PostAttributeChange(nsIContent* aContent,
                                 PRInt32 aNameSpaceID, 
                                 nsIAtom* aName,
                                 const nsString& aValue,
                                 PRBool aNotify,
                                 nsAttributeChangeType aType) = 0;

  NS_IMETHOD PostReflowCallback(nsIReflowCallback* aCallback) = 0;
  NS_IMETHOD CancelReflowCallback(nsIReflowCallback* aCallback) = 0;
 /**
   * Reflow batching
   */   
  NS_IMETHOD BeginReflowBatching() = 0;
  NS_IMETHOD EndReflowBatching(PRBool aFlushPendingReflows) = 0;
  NS_IMETHOD GetReflowBatchingStatus(PRBool* aIsBatching) = 0;

  NS_IMETHOD ClearFrameRefs(nsIFrame* aFrame) = 0;

  /**
   * Given a frame, create a rendering context suitable for use with
   * the frame.
   */
  NS_IMETHOD CreateRenderingContext(nsIFrame *aFrame,
                                    nsIRenderingContext** aContext) = 0;

  /**
   * Informs the pres shell that the document is now at the anchor with
   * the given name.  If |aScroll| is true, scrolls the view of the
   * document so that the anchor with the specified name is displayed at
   * the top of the window.  If |aAnchorName| is empty, then this informs
   * the pres shell that there is no current target, and |aScroll| must
   * be false.
   */
  NS_IMETHOD GoToAnchor(const nsAString& aAnchorName, PRBool aScroll) = 0;

  /**
   * Scrolls the view of the document so that the frame is displayed at the 
   * top of the window.
   *
   * @param aFrame    The frame to scroll into view
   * @param aVPercent How to align the frame vertically. A value of 0
   *                    (NS_PRESSHELL_SCROLL_TOP) means the frame's upper edge is
   *                    aligned with the top edge of the visible area. A value of
   *                    100 (NS_PRESSHELL_SCROLL_BOTTOM) means the frame's bottom
   *                    edge is aligned with the bottom edge of the visible area.
   *                    For values in between, the point "aVPercent" down the frame
   *                    is placed at the point "aVPercent" down the visible area. A
   *                    value of 50 (NS_PRESSHELL_SCROLL_CENTER) centers the frame
   *                    vertically. A value of NS_PRESSHELL_SCROLL_ANYWHERE means move
   *                    the frame the minimum amount necessary in order for the entire
   *                    frame to be visible vertically (if possible)
   * @param aHPercent How to align the frame horizontally. A value of 0
   *                    (NS_PRESSHELL_SCROLL_LEFT) means the frame's left edge is
   *                    aligned with the left edge of the visible area. A value of
   *                    100 (NS_PRESSHELL_SCROLL_RIGHT) means the frame's right
   *                    edge is aligned with the right edge of the visible area.
   *                    For values in between, the point "aVPercent" across the frame
   *                    is placed at the point "aVPercent" across the visible area.
   *                    A value of 50 (NS_PRESSHELL_SCROLL_CENTER) centers the frame
   *                    horizontally . A value of NS_PRESSHELL_SCROLL_ANYWHERE means move
   *                    the frame the minimum amount necessary in order for the entire
   *                    frame to be visible horizontally (if possible)
   */
  NS_IMETHOD ScrollFrameIntoView(nsIFrame *aFrame,
                                 PRIntn   aVPercent, 
                                 PRIntn   aHPercent) const = 0;

  /**
   * Suppress notification of the frame manager that frames are
   * being destroyed.
   */
  NS_IMETHOD SetIgnoreFrameDestruction(PRBool aIgnore) = 0;

  /**
   * Notification sent by a frame informing the pres shell that it is about to
   * be destroyed.
   * This allows any outstanding references to the frame to be cleaned up
   */
  NS_IMETHOD NotifyDestroyingFrame(nsIFrame* aFrame) = 0;

  /**
   * Notify the Clipboard that we have something to copy.
   */
  NS_IMETHOD DoCopy() = 0;

  /**
   * Get the selection of the focussed element (either the page selection,
   * or the selection for a text field).
   */
  NS_IMETHOD GetSelectionForCopy(nsISelection** outSelection) = 0;

  /**
   * Get link location.
   */
  NS_IMETHOD GetLinkLocation(nsIDOMNode* aNode, nsAString& aLocation) = 0;

  /**
   * Get the doc or the selection as text or html.
   */
  NS_IMETHOD DoGetContents(const nsACString& aMimeType, PRUint32 aFlags, PRBool aSelectionOnly, nsAString& outValue) = 0;

  /**
   * Get the caret, if it exists. AddRefs it.
   */
  NS_IMETHOD GetCaret(nsICaret **aOutCaret) = 0;

  /**
   * Set the current caret to a new caret. Returns the old caret.
   */
  virtual already_AddRefed<nsICaret> SetCaret(nsICaret *aNewCaret) = 0;

  /**
   * Should the images have borders etc.  Actual visual effects are determined
   * by the frames.  Visual effects may not effect layout, only display.
   * Takes effect on next repaint, does not force a repaint itself.
   *
   * @param aEnabled  if PR_TRUE, visual selection effects are enabled
   *                  if PR_FALSE visual selection effects are disabled
   * @return  always NS_OK
   */
  NS_IMETHOD SetSelectionFlags(PRInt16 aInEnable) = 0;

  /** 
    * Gets the current state of non text selection effects
    * @param aEnabled  [OUT] set to the current state of non text selection,
    *                  as set by SetDisplayNonTextSelection
    * @return   if aOutEnabled==null, returns NS_ERROR_INVALID_ARG
    *           else NS_OK
    */
  NS_IMETHOD GetSelectionFlags(PRInt16 *aOutEnabled) = 0;
  
  virtual nsISelection* GetCurrentSelection(SelectionType aType) = 0;

  /**
    * Interface to dispatch events via the presshell
    */
  NS_IMETHOD HandleEventWithTarget(nsEvent* aEvent,
                                   nsIFrame* aFrame,
                                   nsIContent* aContent,
                                   nsEventStatus* aStatus) = 0;

  /**
   * Dispatch event to content only (NOT full processing)
   */
  NS_IMETHOD HandleDOMEventWithTarget(nsIContent* aTargetContent,
                                      nsEvent* aEvent,
                                      nsEventStatus* aStatus) = 0;

  /**
    * Gets the current target event frame from the PresShell
    */
  NS_IMETHOD GetEventTargetFrame(nsIFrame** aFrame) = 0;

  /**
    * Gets the current target event frame from the PresShell
    */
  NS_IMETHOD GetEventTargetContent(nsEvent* aEvent, nsIContent** aContent) = 0;

  /**
   * Get and set the history state for the current document 
   */

  NS_IMETHOD CaptureHistoryState(nsILayoutHistoryState** aLayoutHistoryState, PRBool aLeavingPage = PR_FALSE) = 0;

  /**
   * Determine if reflow is currently locked
   * @param aIsReflowLocked returns PR_TRUE if reflow is locked, PR_FALSE otherwise
   */
  NS_IMETHOD IsReflowLocked(PRBool* aIsLocked) = 0;  

  /**
   * Returns a content iterator to iterate the generated content nodes.
   * You must specify whether you want to iterate the "before" generated
   * content or the "after" generated content. If there is no generated
   * content of the specified type for the promary frame associated with
   * with the content object then NULL is returned
   */
  enum GeneratedContentType {Before, After};
  NS_IMETHOD GetGeneratedContentIterator(nsIContent*          aContent,
                                         GeneratedContentType aType,
                                         nsIContentIterator** aIterator) const = 0;


  /**
   * Store the nsIAnonymousContentCreator-generated anonymous
   * content that's associated with an element. The new anonymous content
   * is added to whatever anonymous content might already be associated with
   * the element.
   * @param aContent the element with which the anonymous
   *   content is to be associated with
   * @param aAnonymousElements an array of nsIContent
   *   objects, or null to indicate that any anonymous
   *   content should be dissociated from the aContent
   */
  NS_IMETHOD SetAnonymousContentFor(nsIContent* aContent, nsISupportsArray* aAnonymousElements) = 0;

  /**
   * Retrieve the nsIAnonymousContentCreator-generated anonymous
   * content that's associated with an element.
   * @param aContent the element for which to retrieve the
   *   associated anonymous content
   * @param aAnonymousElements an array of nsIContent objects,
   *   or null to indicate that there are no anonymous elements
   *   associated with aContent
   */
  NS_IMETHOD GetAnonymousContentFor(nsIContent* aContent, nsISupportsArray** aAnonymousElements) = 0;

  /**
   * Release all nsIAnonymousContentCreator-generated
   * anonymous content associated with the shell.
   */
  NS_IMETHOD ReleaseAnonymousContent() = 0;

  /**
   * Called to find out if painting is suppressed for this presshell.  If it is suppressd,
   * we don't allow the painting of any layer but the background, and we don't
   * recur into our children.
   */
  NS_IMETHOD IsPaintingSuppressed(PRBool* aResult)=0;

  /**
   * Unsuppress painting.
   */
  NS_IMETHOD UnsuppressPainting() = 0;

  /**
   * Called to disable nsITheme support in a specific presshell.
   */
  NS_IMETHOD DisableThemeSupport() = 0;

  /**
   * Indicates whether theme support is enabled.
   */
  virtual PRBool IsThemeSupportEnabled() = 0;

  /**
   * Get the set of agent style sheets for this presentation
   */
  virtual nsresult GetAgentStyleSheets(nsCOMArray<nsIStyleSheet>& aSheets) = 0;

  /**
   * Replace the set of agent style sheets
   */
  virtual nsresult SetAgentStyleSheets(const nsCOMArray<nsIStyleSheet>& aSheets) = 0;

  /**
   * Add an override style sheet for this presentation
   */
  virtual nsresult AddOverrideStyleSheet(nsIStyleSheet *aSheet) = 0;

  /**
   * Remove an override style sheet
   */
  virtual nsresult RemoveOverrideStyleSheet(nsIStyleSheet *aSheet) = 0;

  /**
   * Reconstruct frames for all elements in the document
   */
  virtual nsresult ReconstructFrames() = 0;

  /**
   * See if reflow verification is enabled. To enable reflow verification add
   * "verifyreflow:1" to your NSPR_LOG_MODULES environment variable
   * (any non-zero debug level will work). Or, call SetVerifyReflowEnable
   * with PR_TRUE.
   */
  static PRBool GetVerifyReflowEnable();

  /**
   * Set the verify-reflow enable flag.
   */
  static void SetVerifyReflowEnable(PRBool aEnabled);

  /**
   * Get the flags associated with the VerifyReflow debug tool
   */
  static PRInt32 GetVerifyReflowFlags();

#ifdef MOZ_REFLOW_PERF
  NS_IMETHOD DumpReflows() = 0;
  NS_IMETHOD CountReflows(const char * aName, PRUint32 aType, nsIFrame * aFrame) = 0;
  NS_IMETHOD PaintCount(const char * aName, 
                        nsIRenderingContext* aRenderingContext, 
                        nsPresContext * aPresContext, 
                        nsIFrame * aFrame,
                        PRUint32 aColor) = 0;
  NS_IMETHOD SetPaintFrameCount(PRBool aOn) = 0;
#endif

#ifdef IBMBIDI
  /**
   * SetCaretBidiLevel will set the Bidi embedding level for the cursor. 0-63
   */
  NS_IMETHOD SetCaretBidiLevel(PRUint8 aLevel) = 0;

  /**
   * GetCaretBidiLevel will get the Bidi embedding level for the cursor. 0-63
   */
  NS_IMETHOD GetCaretBidiLevel(PRUint8 *aOutLevel) = 0;

  /**
   * UndefineCaretBidiLevel will set the Bidi embedding level for the cursor to an out-of-range value
   */
  NS_IMETHOD UndefineCaretBidiLevel(void) = 0;

  /**
   * Reconstruct and reflow frame model 
   */
  NS_IMETHOD BidiStyleChangeReflow(void) = 0;
#endif

#ifdef DEBUG
  // Debugging hooks
  virtual void ListStyleContexts(nsIFrame *aRootFrame, FILE *out,
                                 PRInt32 aIndent = 0) = 0;

  virtual void ListStyleSheets(FILE *out, PRInt32 aIndent = 0) = 0;
  virtual void VerifyStyleTree() = 0;
#endif

  PRBool IsAccessibilityActive() { return mIsAccessibilityActive; }

  /**
   * Stop all active elements (plugins and the caret) in this presentation and
   * in the presentations of subdocuments.  Resets painting to a suppressed state.
   * XXX this should include image animations
   */
  virtual void Freeze() = 0;

  /**
   * Restarts active elements (plugins) in this presentation and in the
   * presentations of subdocuments, then do a full invalidate of the content area.
   */
  virtual void Thaw() = 0;

  /**
   * When this shell is disconnected from its containing docshell, we
   * lose our container pointer.  However, we'd still like to be able to target
   * user events at the docshell's parent.  This pointer allows us to do that.
   * It should not be used for any other purpose.
   */
  void SetForwardingContainer(nsWeakPtr aContainer)
  {
    mForwardingContainer = aContainer;
  }
  
  /**
   * Dump window contents into a new offscreen rendering context.
   * @param aRect is the region to capture into the offscreen buffer, in the
   * root frame's coordinate system (if aIgnoreViewportScrolling is false)
   * or in the root scrolled frame's coordinate system
   * (if aIgnoreViewportScrolling is true)
   * @param aUntrusted set to PR_TRUE if the contents may be passed to malicious
   * agents. E.g. we might choose not to paint the contents of sensitive widgets
   * such as the file name in a file upload widget, and we might choose not
   * to paint themes.
   * @param aIgnoreViewportScrolling ignore clipping/scrolling/scrollbar painting
   * due to scrolling in the viewport
   * @param aBackgroundColor a background color to render onto
   * @param aRenderedContext gets set to a rendering context whose offscreen
   * buffer can be locked to get the data. The buffer's size will be aRect's size.
   * In all cases the caller must clean it up by calling
   * cx->DestroyDrawingSurface(cx->GetDrawingSurface()).
   */
  NS_IMETHOD RenderOffscreen(nsRect aRect, PRBool aUntrusted,
                             PRBool aIgnoreViewportScrolling,
                             nscolor aBackgroundColor,
                             nsIRenderingContext** aRenderedContext) = 0;

  void AddWeakFrame(nsWeakFrame* aWeakFrame);
  void RemoveWeakFrame(nsWeakFrame* aWeakFrame);
protected:
  // IMPORTANT: The ownership implicit in the following member variables
  // has been explicitly checked.  If you add any members to this class,
  // please make the ownership explicit (pinkerton, scc).

  // these are the same Document and PresContext owned by the DocViewer.
  // we must share ownership.
  nsIDocument*              mDocument;      // [STRONG]
  nsPresContext*            mPresContext;   // [STRONG]
  nsStyleSet*               mStyleSet;      // [OWNS]
  nsCSSFrameConstructor*    mFrameConstructor; // [OWNS]
  nsIViewManager*           mViewManager;   // [WEAK] docViewer owns it so I don't have to
  nsFrameSelection*         mSelection;
  nsFrameManagerBase        mFrameManager;  // [OWNS]
  nsWeakPtr                 mForwardingContainer;

  PRPackedBool              mStylesHaveChanged;
  PRPackedBool              mDidInitialReflow;
  PRPackedBool              mIsDestroying;

#ifdef ACCESSIBILITY
  /**
   * Call this when there have been significant changes in the rendering for
   * a content subtree, so the matching accessibility subtree can be invalidated
   */
  void InvalidateAccessibleSubtree(nsIContent *aContent);
#endif

  // Set to true when the accessibility service is being used to mirror
  // the dom/layout trees
  PRPackedBool              mIsAccessibilityActive;

  // A list of weak frames. This is a pointer to the last item in the list.
  nsWeakFrame*              mWeakFrames;
};

/**
 * Create a new empty presentation shell. Upon success, call Init
 * before attempting to use the shell.
 */
nsresult
NS_NewPresShell(nsIPresShell** aInstancePtrResult);

#endif /* nsIPresShell_h___ */
