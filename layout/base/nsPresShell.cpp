/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=2 sw=2 et tw=78:
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
 *   H�kan Waara <hwaara@chello.se>
 *   Dan Rosen <dr@netscape.com>
 *   Daniel Glazman <glazman@netscape.com>
 *   Mats Palmgren <mats.palmgren@bredband.net>
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
 * 05/03/2000   IBM Corp.       Observer events for reflow states
 */ 

/* a presentation of a document, part 2 */

#define PL_ARENA_CONST_ALIGN_MASK 3

#include "nsIPresShell.h"
#include "nsPresContext.h"
#include "nsIContent.h"
#include "nsIDocument.h"
#include "nsIDOMXULDocument.h"
#include "nsStubDocumentObserver.h"
#include "nsStyleSet.h"
#include "nsICSSStyleSheet.h" // XXX for UA sheet loading hack, can this go away please?
#include "nsIDOMCSSStyleSheet.h"  // for Pref-related rule management (bugs 22963,20760,31816)
#include "nsINameSpaceManager.h"  // for Pref-related rule management (bugs 22963,20760,31816)
#include "nsIServiceManager.h"
#include "nsFrame.h"
#include "nsHTMLReflowCommand.h"
#include "nsIViewManager.h"
#include "nsCRT.h"
#include "prlog.h"
#include "prmem.h"
#include "prprf.h"
#include "prinrval.h"
#include "nsVoidArray.h"
#include "nsCOMArray.h"
#include "nsHashtable.h"
#include "nsIViewObserver.h"
#include "nsContainerFrame.h"
#include "nsIDeviceContext.h"
#include "nsEventStateManager.h"
#include "nsDOMEvent.h"
#include "nsGUIEvent.h"
#include "nsHTMLParts.h"
#include "nsContentUtils.h"
#include "nsISelection.h"
#include "nsISelectionController.h"
#include "nsReflowPath.h"
#include "nsLayoutCID.h"
#include "nsLayoutAtoms.h"
#include "nsIDOMRange.h"
#include "nsIDOMDocument.h"
#include "nsIDOMNode.h"
#include "nsIDOM3Node.h"
#include "nsIDOMNodeList.h"
#include "nsIDOMElement.h"
#include "nsHTMLAtoms.h"
#include "nsCSSPseudoElements.h"
#include "nsCOMPtr.h"
#include "nsAutoPtr.h"
#include "nsReadableUtils.h"
#include "nsUnicharUtils.h"
#include "nsWeakReference.h"
#include "nsIPageSequenceFrame.h"
#include "nsICaret.h"
#include "nsIDOMHTMLDocument.h"
#include "nsIXPointer.h"
#include "nsIDOMXMLDocument.h"
#include "nsIScrollableView.h"
#include "nsIParser.h"
#include "nsParserCIID.h"
#include "nsFrameSelection.h"
#include "nsIDOMNSHTMLInputElement.h" //optimization for ::DoXXX commands
#include "nsIDOMNSHTMLTextAreaElement.h"
#include "nsViewsCID.h"
#include "nsFrameManager.h"
#include "nsXPCOM.h"
#include "nsISupportsPrimitives.h"
#include "nsILayoutHistoryState.h"
#include "nsIScrollPositionListener.h"
#include "nsICompositeListener.h"
#include "nsILineIterator.h" // for ScrollFrameIntoView
#include "nsTimer.h"
#include "nsWeakPtr.h"
#include "plarena.h"
#include "pldhash.h"
#include "nsIObserverService.h"
#include "nsIObserver.h"
#include "nsIDocShell.h"        // for reflow observation
#include "nsIBaseWindow.h"
#include "nsLayoutErrors.h"
#include "nsLayoutUtils.h"
#include "nsCSSRendering.h"
#ifdef NS_DEBUG
#include "nsIFrameDebug.h"
#endif
  // for |#ifdef DEBUG| code
#include "nsSpaceManager.h"
#include "prenv.h"
#include "nsIAttribute.h"
#include "nsIGlobalHistory2.h"
#include "nsDisplayList.h"
#include "nsRegion.h"

#ifdef MOZ_REFLOW_PERF_DSP
#include "nsIRenderingContext.h"
#include "nsIFontMetrics.h"
#endif

#include "nsIReflowCallback.h"

#include "nsPIDOMWindow.h"
#include "nsIFocusController.h"
#include "nsIPluginInstance.h"
#include "nsIObjectFrame.h"
#include "nsIObjectLoadingContent.h"
#include "nsNetUtil.h"
#include "nsEventDispatcher.h"
#include "nsThreadUtils.h"
#include "nsStyleSheetService.h"

// Drag & Drop, Clipboard
#include "nsWidgetsCID.h"
#include "nsIClipboard.h"
#include "nsIClipboardHelper.h"
#include "nsIDocShellTreeItem.h"
#include "nsIURI.h"
#include "nsIScrollableFrame.h"
#include "prtime.h"
#include "prlong.h"
#include "nsIDragService.h"
#include "nsCopySupport.h"
#include "nsIDOMHTMLAnchorElement.h"
#include "nsIDOMHTMLAreaElement.h"
#include "nsIDOMHTMLLinkElement.h"
#include "nsITimer.h"
#ifdef ACCESSIBILITY
#include "nsIAccessibilityService.h"
#include "nsIAccessible.h"
#include "nsIAccessibleEvent.h"
#endif

// For style data reconstruction
#include "nsStyleChangeList.h"
#include "nsCSSFrameConstructor.h"
#include "nsIBindingManager.h"
#ifdef MOZ_XUL
#include "nsIMenuFrame.h"
#include "nsITreeBoxObject.h"
#endif
#include "nsPlaceholderFrame.h"

// Content viewer interfaces
#include "nsIContentViewer.h"

#ifdef IBMBIDI
#include "nsIBidiKeyboard.h"
#endif // IBMBIDI

#include "nsContentCID.h"
static NS_DEFINE_CID(kCSSStyleSheetCID, NS_CSS_STYLESHEET_CID);
static NS_DEFINE_IID(kRangeCID,     NS_RANGE_CID);

// convert a color value to a string, in the CSS format #RRGGBB
// *  - initially created for bugs 31816, 20760, 22963
static void ColorToString(nscolor aColor, nsAutoString &aString);

// Class ID's
static NS_DEFINE_CID(kFrameSelectionCID, NS_FRAMESELECTION_CID);

#undef NOISY

// ----------------------------------------------------------------------

#ifdef NS_DEBUG
// Set the environment variable GECKO_VERIFY_REFLOW_FLAGS to one or
// more of the following flags (comma separated) for handy debug
// output.
static PRUint32 gVerifyReflowFlags;

struct VerifyReflowFlags {
  const char*    name;
  PRUint32 bit;
};

static const VerifyReflowFlags gFlags[] = {
  { "verify",                VERIFY_REFLOW_ON },
  { "reflow",                VERIFY_REFLOW_NOISY },
  { "all",                   VERIFY_REFLOW_ALL },
  { "list-commands",         VERIFY_REFLOW_DUMP_COMMANDS },
  { "noisy-commands",        VERIFY_REFLOW_NOISY_RC },
  { "really-noisy-commands", VERIFY_REFLOW_REALLY_NOISY_RC },
  { "space-manager",         VERIFY_REFLOW_INCLUDE_SPACE_MANAGER },
  { "resize",                VERIFY_REFLOW_DURING_RESIZE_REFLOW },
};

#define NUM_VERIFY_REFLOW_FLAGS (sizeof(gFlags) / sizeof(gFlags[0]))

static void
ShowVerifyReflowFlags()
{
  printf("Here are the available GECKO_VERIFY_REFLOW_FLAGS:\n");
  const VerifyReflowFlags* flag = gFlags;
  const VerifyReflowFlags* limit = gFlags + NUM_VERIFY_REFLOW_FLAGS;
  while (flag < limit) {
    printf("  %s\n", flag->name);
    ++flag;
  }
  printf("Note: GECKO_VERIFY_REFLOW_FLAGS is a comma separated list of flag\n");
  printf("names (no whitespace)\n");
}
#endif

//========================================================================
//========================================================================
//========================================================================
#ifdef MOZ_REFLOW_PERF
class ReflowCountMgr;

static const char kGrandTotalsStr[] = "Grand Totals";
#define NUM_REFLOW_TYPES 5

// Counting Class
class ReflowCounter {
public:
  ReflowCounter(ReflowCountMgr * aMgr = nsnull);
  ~ReflowCounter();

  void ClearTotals();
  void DisplayTotals(const char * aStr);
  void DisplayDiffTotals(const char * aStr);
  void DisplayHTMLTotals(const char * aStr);

  void Add(nsReflowReason aType)                  { mTotals[aType]++;         }
  void Add(nsReflowReason aType, PRUint32 aTotal) { mTotals[aType] += aTotal; }

  void CalcDiffInTotals();
  void SetTotalsCache();

  void SetMgr(ReflowCountMgr * aMgr) { mMgr = aMgr; }

  PRUint32 GetTotalByType(nsReflowReason aType) { if (aType >= eReflowReason_Initial && aType <= eReflowReason_Dirty) return mTotals[aType]; else return 0; }
  
protected:
  void DisplayTotals(PRUint32 * aArray, const char * aTitle);
  void DisplayHTMLTotals(PRUint32 * aArray, const char * aTitle);

  PRUint32 mTotals[NUM_REFLOW_TYPES];
  PRUint32 mCacheTotals[NUM_REFLOW_TYPES];

  ReflowCountMgr * mMgr; // weak reference (don't delete)
};

// Counting Class
class IndiReflowCounter {
public:
  IndiReflowCounter(ReflowCountMgr * aMgr = nsnull)
    : mFrame(nsnull),
      mCount(0),
      mMgr(aMgr),
      mCounter(aMgr),
      mHasBeenOutput(PR_FALSE)
    {}
  virtual ~IndiReflowCounter() {}

  nsAutoString mName;
  nsIFrame *   mFrame;   // weak reference (don't delete)
  PRInt32      mCount;

  ReflowCountMgr * mMgr; // weak reference (don't delete)

  ReflowCounter mCounter;
  PRBool        mHasBeenOutput;

};

//--------------------
// Manager Class
//--------------------
class ReflowCountMgr {
public:
  ReflowCountMgr();
  virtual ~ReflowCountMgr();

  void ClearTotals();
  void ClearGrandTotals();
  void DisplayTotals(const char * aStr);
  void DisplayHTMLTotals(const char * aStr);
  void DisplayDiffsInTotals(const char * aStr);

  void Add(const char * aName, nsReflowReason aType, nsIFrame * aFrame);
  ReflowCounter * LookUp(const char * aName);

  void PaintCount(const char * aName, nsIRenderingContext* aRenderingContext, nsPresContext* aPresContext, nsIFrame * aFrame, PRUint32 aColor);

  FILE * GetOutFile() { return mFD; }

  PLHashTable * GetIndiFrameHT() { return mIndiFrameCounts; }

  void SetPresContext(nsPresContext * aPresContext) { mPresContext = aPresContext; } // weak reference
  void SetPresShell(nsIPresShell* aPresShell) { mPresShell= aPresShell; } // weak reference

  void SetDumpFrameCounts(PRBool aVal)         { mDumpFrameCounts = aVal; }
  void SetDumpFrameByFrameCounts(PRBool aVal)  { mDumpFrameByFrameCounts = aVal; }
  void SetPaintFrameCounts(PRBool aVal)        { mPaintFrameByFrameCounts = aVal; }

protected:
  void DisplayTotals(PRUint32 * aArray, PRUint32 * aDupArray, char * aTitle);
  void DisplayHTMLTotals(PRUint32 * aArray, PRUint32 * aDupArray, char * aTitle);

  PR_STATIC_CALLBACK(PRIntn) RemoveItems(PLHashEntry *he, PRIntn i, void *arg);
  PR_STATIC_CALLBACK(PRIntn) RemoveIndiItems(PLHashEntry *he, PRIntn i, void *arg);
  void CleanUp();

  // stdout Output Methods
  PR_STATIC_CALLBACK(PRIntn) DoSingleTotal(PLHashEntry *he, PRIntn i, void *arg);
  PR_STATIC_CALLBACK(PRIntn) DoSingleIndi(PLHashEntry *he, PRIntn i, void *arg);

  void DoGrandTotals();
  void DoIndiTotalsTree();

  // HTML Output Methods
  PR_STATIC_CALLBACK(PRIntn) DoSingleHTMLTotal(PLHashEntry *he, PRIntn i, void *arg);
  void DoGrandHTMLTotals();

  // Zero Out the Totals
  PR_STATIC_CALLBACK(PRIntn) DoClearTotals(PLHashEntry *he, PRIntn i, void *arg);

  // Displays the Diff Totals
  PR_STATIC_CALLBACK(PRIntn) DoDisplayDiffTotals(PLHashEntry *he, PRIntn i, void *arg);

  PLHashTable * mCounts;
  PLHashTable * mIndiFrameCounts;
  FILE * mFD;
  
  PRBool mDumpFrameCounts;
  PRBool mDumpFrameByFrameCounts;
  PRBool mPaintFrameByFrameCounts;

  PRBool mCycledOnce;

  // Root Frame for Individual Tracking
  nsPresContext * mPresContext;
  nsIPresShell*    mPresShell;

  // ReflowCountMgr gReflowCountMgr;
};
#endif
//========================================================================

// comment out to hide caret
#define SHOW_CARET

// The upper bound on the amount of time to spend reflowing, in
// microseconds.  When this bound is exceeded and reflow commands are
// still queued up, a reflow event is posted.  The idea is for reflow
// to not hog the processor beyond the time specifed in
// gMaxRCProcessingTime.  This data member is initialized from the
// layout.reflow.timeslice pref.
#define NS_MAX_REFLOW_TIME    1000000
static PRInt32 gMaxRCProcessingTime = -1;

// Largest chunk size we recycle
static const size_t gMaxRecycledSize = 400;

#define MARK_INCREMENT 50
#define BLOCK_INCREMENT 4044 /* a bit under 4096, for malloc overhead */

/**A block of memory that the stack will 
 * chop up and hand out
 */
struct StackBlock {
   
   // a block of memory.  Note that this must be first so that it will
   // be aligned.
   char mBlock[BLOCK_INCREMENT];

   // another block of memory that would only be created
   // if our stack overflowed. Yes we have the ability
   // to grow on a stack overflow
   StackBlock* mNext;

   StackBlock() : mNext(nsnull) { }
   ~StackBlock() { }
};

/* we hold an array of marks. A push pushes a mark on the stack
 * a pop pops it off.
 */
struct StackMark {
   // the block of memory we are currently handing out chunks of
   StackBlock* mBlock;
   
   // our current position in the memory
   size_t mPos;
};


/* A stack arena allows a stack based interface to a block of memory.
 * It should be used when you need to allocate some temporary memory that
 * you will immediately return.
 */
class StackArena {
public:
  StackArena();
  ~StackArena();

  // Memory management functions
  nsresult  Allocate(size_t aSize, void** aResult);
  nsresult  Push();
  nsresult  Pop();

private:
  // our current position in memory
  size_t mPos;

  // a list of memory block. Usually there is only one
  // but if we overrun our stack size we can get more memory.
  StackBlock* mBlocks;

  // the current block of memory we are passing our chucks of
  StackBlock* mCurBlock;

  // our stack of mark where push has been called
  StackMark* mMarks;

  // the current top of the mark list
  PRUint32 mStackTop;

  // the size of the mark array
  PRUint32 mMarkLength;
};



StackArena::StackArena()
{
  // allocate the marks array
  mMarkLength = MARK_INCREMENT;
  mMarks = new StackMark[mMarkLength];

  // allocate our stack memory
  mBlocks = new StackBlock();
  mCurBlock = mBlocks;

  mStackTop = 0;
  mPos = 0;
}

StackArena::~StackArena()
{
  // free up our data
  delete[] mMarks;
  while(mBlocks)
  {
    StackBlock* toDelete = mBlocks;
    mBlocks = mBlocks->mNext;
    delete toDelete;
  }
} 

nsresult
StackArena::Push()
{
  // if the we overrun our mark array. Resize it.
  if (mStackTop + 1 >= mMarkLength)
  {
    StackMark* oldMarks = mMarks;
    PRUint32 oldLength = mMarkLength;
    mMarkLength += MARK_INCREMENT;
    mMarks = new StackMark[mMarkLength];
    memcpy(mMarks, oldMarks, sizeof(StackMark)*oldLength);

    delete[] oldMarks;
  }

  // set a mark at the top
  mMarks[mStackTop].mBlock = mCurBlock;
  mMarks[mStackTop].mPos = mPos;

  mStackTop++;

  return NS_OK;
}

nsresult
StackArena::Allocate(size_t aSize, void** aResult)
{
  NS_ASSERTION(mStackTop > 0, "Error allocate called before push!!!");

  // make sure we are aligned. Beard said 8 was safer then 4. 
  // Round size to multiple of 8
  aSize = PR_ROUNDUP(aSize, 8);

  // if the size makes the stack overflow. Grab another block for the stack
  if (mPos + aSize >= BLOCK_INCREMENT)
  {
    NS_ASSERTION(aSize <= BLOCK_INCREMENT,"Requested memory is greater that our block size!!");
    if (mCurBlock->mNext == nsnull)
      mCurBlock->mNext = new StackBlock();

    mCurBlock =  mCurBlock->mNext;
    mPos = 0;
  }

  // return the chunk they need.
  *aResult = mCurBlock->mBlock + mPos;
  mPos += aSize;

  return NS_OK;
}

nsresult
StackArena::Pop()
{
  // pop off the mark
  NS_ASSERTION(mStackTop > 0, "Error Pop called 1 too many times");
  mStackTop--;

#ifdef DEBUG
  // Mark the "freed" memory with 0xdd to help with debugging of memory
  // allocation problems.
  {
    StackBlock *block = mMarks[mStackTop].mBlock, *block_end = mCurBlock;
    size_t pos = mMarks[mStackTop].mPos;
    for (; block != block_end; block = block->mNext, pos = 0) {
      memset(block->mBlock + pos, 0xdd, sizeof(block->mBlock) - pos);
    }
    memset(block->mBlock + pos, 0xdd, mPos - pos);
  }
#endif

  mCurBlock = mMarks[mStackTop].mBlock;
  mPos      = mMarks[mStackTop].mPos;

  return NS_OK;
}

// Uncomment this to disable the frame arena.
//#define DEBUG_TRACEMALLOC_FRAMEARENA 1

// Memory is allocated 4-byte aligned. We have recyclers for chunks up to
// 200 bytes
class FrameArena {
public:
  FrameArena(PRUint32 aArenaSize = 4096);
  ~FrameArena();

  // Memory management functions
  NS_HIDDEN_(void*) AllocateFrame(size_t aSize);
  NS_HIDDEN_(void)  FreeFrame(size_t aSize, void* aPtr);

private:
#if !defined(DEBUG_TRACEMALLOC_FRAMEARENA)
  // Underlying arena pool
  PLArenaPool mPool;

  // The recycler array is sparse with the indices being multiples of 4,
  // i.e., 0, 4, 8, 12, 16, 20, ...
  void*       mRecyclers[gMaxRecycledSize >> 2];
#endif
};

FrameArena::FrameArena(PRUint32 aArenaSize)
{
#if !defined(DEBUG_TRACEMALLOC_FRAMEARENA)
  // Initialize the arena pool
  PL_INIT_ARENA_POOL(&mPool, "FrameArena", aArenaSize);

  // Zero out the recyclers array
  memset(mRecyclers, 0, sizeof(mRecyclers));
#endif
}

FrameArena::~FrameArena()
{
#if !defined(DEBUG_TRACEMALLOC_FRAMEARENA)
  // Free the arena in the pool and finish using it
  PL_FinishArenaPool(&mPool);
#endif
} 

void*
FrameArena::AllocateFrame(size_t aSize)
{
#if defined(DEBUG_TRACEMALLOC_FRAMEARENA)
  return PR_Malloc(aSize);
#else
  void* result = nsnull;
  
  // Ensure we have correct alignment for pointers.  Important for Tru64
  aSize = PR_ROUNDUP(aSize, sizeof(void*));

  // Check recyclers first
  if (aSize < gMaxRecycledSize) {
    const int   index = aSize >> 2;

    result = mRecyclers[index];
    if (result) {
      // Need to move to the next object
      void* next = *((void**)result);
      mRecyclers[index] = next;
    }
  }

  if (!result) {
    // Allocate a new chunk from the arena
    PL_ARENA_ALLOCATE(result, &mPool, aSize);
  }

  return result;
#endif
}

void
FrameArena::FreeFrame(size_t aSize, void* aPtr)
{
#ifdef DEBUG
  // Mark the memory with 0xdd in DEBUG builds so that there will be
  // problems if someone tries to access memory that they've freed.
  memset(aPtr, 0xdd, aSize);
#endif
#if defined(DEBUG_TRACEMALLOC_FRAMEARENA)
  PR_Free(aPtr);
#else
  // Ensure we have correct alignment for pointers.  Important for Tru64
  aSize = PR_ROUNDUP(aSize, sizeof(void*));

  // See if it's a size that we recycle
  if (aSize < gMaxRecycledSize) {
    const int   index = aSize >> 2;
    void*       currentTop = mRecyclers[index];
    mRecyclers[index] = aPtr;
    *((void**)aPtr) = currentTop;
  }
#ifdef DEBUG_dbaron
  else {
    fprintf(stderr,
            "WARNING: FrameArena::FreeFrame leaking chunk of %d bytes.\n",
            aSize);
  }
#endif
#endif
}

class PresShellViewEventListener : public nsIScrollPositionListener,
                                   public nsICompositeListener
{
public:
  PresShellViewEventListener();
  virtual ~PresShellViewEventListener();

  NS_DECL_ISUPPORTS

  // nsIScrollPositionListener methods
  NS_IMETHOD ScrollPositionWillChange(nsIScrollableView *aView, nscoord aX, nscoord aY);
  NS_IMETHOD ScrollPositionDidChange(nsIScrollableView *aView, nscoord aX, nscoord aY);

  // nsICompositeListener methods
  NS_IMETHOD WillRefreshRegion(nsIViewManager *aViewManager,
                               nsIView *aView,
                               nsIRenderingContext *aContext,
                               nsIRegion *aRegion,
                               PRUint32 aUpdateFlags);

  NS_IMETHOD DidRefreshRegion(nsIViewManager *aViewManager,
                              nsIView *aView,
                              nsIRenderingContext *aContext,
                              nsIRegion *aRegion,
                              PRUint32 aUpdateFlags);

  NS_IMETHOD WillRefreshRect(nsIViewManager *aViewManager,
                             nsIView *aView,
                             nsIRenderingContext *aContext,
                             const nsRect *aRect,
                             PRUint32 aUpdateFlags);

  NS_IMETHOD DidRefreshRect(nsIViewManager *aViewManager,
                            nsIView *aView,
                            nsIRenderingContext *aContext,
                            const nsRect *aRect,
                            PRUint32 aUpdateFlags);

  nsresult SetPresShell(nsIPresShell *aPresShell);

private:

  nsresult HideCaret();
  nsresult RestoreCaretVisibility();

  nsIPresShell *mPresShell;
  PRBool        mWasVisible;
  PRInt32       mCallCount;
};

struct nsDOMEventRequest 
{
  nsIContent* content;
  nsEvent* event;
  nsDOMEventRequest* next;
};

struct nsAttributeChangeRequest 
{
  nsIContent* content;
  PRInt32 nameSpaceID;
  nsIAtom* name;
  nsAutoString value;
  PRBool notify;
  nsAttributeChangeType type;
  nsAttributeChangeRequest* next;
};

struct nsCallbackEventRequest
{
  nsIReflowCallback* callback;
  nsCallbackEventRequest* next;
};


// ----------------------------------------------------------------------------

/**
 * Used to build and maintain the incremental reflow tree, and
 * dispatch incremental reflows to individual reflow roots.
 */
class IncrementalReflow
{
public:
  ~IncrementalReflow();

  /**
   * Add a reflow command to the set of commands that are to be
   * dispatched in the incremental reflow.
   */
  enum AddCommandResult {
    eEnqueued, // the command was successfully added
    eTryLater, // the command could not be added; try again
    eCancel,   // the command was not added; delete it
    eOOM       // Out of memory.
  };
  AddCommandResult
  AddCommand(nsPresContext      *aPresContext,
             nsHTMLReflowCommand *aCommand);

  /**
   * Dispatch the incremental reflow.
   */
  void
  Dispatch(nsPresContext      *aPresContext,
           nsHTMLReflowMetrics &aDesiredSize,
           const nsSize        &aMaxSize,
           nsIRenderingContext &aRendContext);

#ifdef NS_DEBUG
  /**
   * Dump the incremental reflow state.
   */
  void
  Dump(nsPresContext *aPresContext) const;
#endif

protected:
  /**
   * The set of incremental reflow roots.
   */
  nsAutoVoidArray mRoots;
};

IncrementalReflow::~IncrementalReflow()
{
  for (PRInt32 i = mRoots.Count() - 1; i >= 0; --i)
    delete NS_STATIC_CAST(nsReflowPath *, mRoots[i]);
}

void
IncrementalReflow::Dispatch(nsPresContext      *aPresContext,
                            nsHTMLReflowMetrics &aDesiredSize,
                            const nsSize        &aMaxSize,
                            nsIRenderingContext &aRendContext)
{
  for (PRInt32 i = mRoots.Count() - 1; i >= 0; --i) {
    // Send an incremental reflow notification to the first frame in the
    // path.
    nsReflowPath *path = NS_STATIC_CAST(nsReflowPath *, mRoots[i]);
    nsIFrame *first = path->mFrame;

    nsIFrame* root = aPresContext->PresShell()->FrameManager()->GetRootFrame();

    first->WillReflow(aPresContext);
    nsContainerFrame::PositionFrameView(first);

    // If the first frame in the path is the root of the frame
    // hierarchy, then use all the available space. If it's simply a
    // `reflow root', then use the first frame's size as the available
    // space.
    nsSize size;
    if (first == root)
      size = aMaxSize;
    else
      size = first->GetSize();

    nsHTMLReflowState reflowState(aPresContext, first, path,
                                  &aRendContext, size);

    nsReflowStatus status;
    first->Reflow(aPresContext, aDesiredSize, reflowState, status);

    // If an incremental reflow is initiated at a frame other than the
    // root frame, then its desired size had better not change!
    NS_ASSERTION(first == root ||
                 (aDesiredSize.width == size.width && aDesiredSize.height == size.height),
                 "non-root frame's desired size changed during an incremental reflow");

    first->SetSize(nsSize(aDesiredSize.width, aDesiredSize.height));

    nsContainerFrame::SyncFrameViewAfterReflow(aPresContext, first, first->GetView(),
                                               &aDesiredSize.mOverflowArea);

    first->DidReflow(aPresContext, nsnull, NS_FRAME_REFLOW_FINISHED);
  }
}

IncrementalReflow::AddCommandResult
IncrementalReflow::AddCommand(nsPresContext      *aPresContext,
                              nsHTMLReflowCommand *aCommand)
{
  nsIFrame *frame;
  aCommand->GetTarget(frame);
  NS_ASSERTION(frame != nsnull, "reflow command with no target");

  // Construct the reflow path by walking up the through the frames'
  // parent chain until we reach either a `reflow root' or the root
  // frame in the frame hierarchy.
  nsAutoVoidArray path;
  do {
    path.AppendElement(frame);
  } while (!(frame->GetStateBits() & NS_FRAME_REFLOW_ROOT) &&
           (frame = frame->GetParent()) != nsnull);

  // Pop off the root, add it to the set if it's not there already.
  PRInt32 lastIndex = path.Count() - 1;
  nsIFrame *rootFrame = NS_STATIC_CAST(nsIFrame *, path[lastIndex]);
  path.RemoveElementAt(lastIndex);

  // Prevent an incremental reflow from being posted inside a reflow
  // root if the reflow root's container has not yet been reflowed.
  // This can cause problems like bug 228156.
  if (rootFrame->GetParent() &&
      (rootFrame->GetParent()->GetStateBits() & NS_FRAME_FIRST_REFLOW)) {
    return eCancel;
  }

  nsReflowPath *root = nsnull;

  PRInt32 i;
  for (i = mRoots.Count() - 1; i >= 0; --i) {
    nsReflowPath *r = NS_STATIC_CAST(nsReflowPath *, mRoots[i]);
    if (r->mFrame == rootFrame) {
      root = r;
      break;
    }
  }

  if (! root) {
    root = new nsReflowPath(rootFrame);
    if (! root)
      return eOOM;

    root->mReflowCommand = nsnull;
    mRoots.AppendElement(root);
  }

  // Now walk the path from the root to the leaf, adding to the reflow
  // tree as necessary.
  nsReflowPath *target = root;
  for (i = path.Count() - 1; i >= 0; --i) {
    nsIFrame *f = NS_STATIC_CAST(nsIFrame *, path[i]);
    target = target->EnsureSubtreeFor(f);

    // Out of memory. Ugh.
    if (! target)
      return eOOM;
  }

  // Place the reflow command in the leaf, if one isn't there already.
  if (target->mReflowCommand) {
    // XXXwaterson it's probably possible to have some notion of
    // `promotion' here that would avoid any re-queuing; for example,
    // promote a dirty reflow to a style changed. For now, let's punt
    // and not worry about it.
#ifdef NS_DEBUG
    if (gVerifyReflowFlags & VERIFY_REFLOW_NOISY_RC)
      printf("requeuing command %p because %p was already scheduled "
             "for the same frame",
             (void*)aCommand, (void*)target->mReflowCommand);
#endif

    return eTryLater;
  }

  target->mReflowCommand = aCommand;
  return eEnqueued;
}


#ifdef NS_DEBUG
void
IncrementalReflow::Dump(nsPresContext *aPresContext) const
{
  for (PRInt32 i = mRoots.Count() - 1; i >= 0; --i)
    NS_STATIC_CAST(nsReflowPath *, mRoots[i])->Dump(aPresContext, stdout, 0);
}
#endif

// ----------------------------------------------------------------------------

struct ReflowCommandEntry : public PLDHashEntryHdr
{
  nsHTMLReflowCommand* mCommand;
};

PR_STATIC_CALLBACK(const void *)
ReflowCommandHashGetKey(PLDHashTable *table, PLDHashEntryHdr *entry)
{
  ReflowCommandEntry *e = NS_STATIC_CAST(ReflowCommandEntry *, entry);
  
  return e->mCommand;
}

PR_STATIC_CALLBACK(PLDHashNumber)
ReflowCommandHashHashKey(PLDHashTable *table, const void *key)
{
  const nsHTMLReflowCommand* command =
    NS_STATIC_CAST(const nsHTMLReflowCommand*, key);
  
  // The target is going to be reasonably unique, if we shift out the
  // always-zero low-order bits, the type comes from an enum and we just don't
  // have that many types, and the child list name is either null or has the
  // same high-order bits as all the other child list names.
  return
    (NS_PTR_TO_INT32(command->GetTarget()) >> 2) ^
    (command->Type() << 17) ^
    (NS_PTR_TO_INT32(command->GetChildListName()) << 20);
}

PR_STATIC_CALLBACK(PRBool)
ReflowCommandHashMatchEntry(PLDHashTable *table, const PLDHashEntryHdr *entry,
                            const void *key)
{
   const ReflowCommandEntry *e =
     NS_STATIC_CAST(const ReflowCommandEntry *, entry);
   const nsHTMLReflowCommand *command = e->mCommand;
   const nsHTMLReflowCommand *command2 =
     NS_STATIC_CAST(const nsHTMLReflowCommand *, key);

   return
     command->GetTarget() == command2->GetTarget() &&
     command->Type() == command2->Type() &&
     command->GetChildListName() == command2->GetChildListName();
}

// ----------------------------------------------------------------------------
class nsPresShellEventCB;

class PresShell : public nsIPresShell, public nsIViewObserver,
                  public nsStubDocumentObserver,
                  public nsISelectionController, public nsIObserver,
                  public nsSupportsWeakReference
{
public:
  PresShell();

  NS_DECL_AND_IMPL_ZEROING_OPERATOR_NEW

  // nsISupports
  NS_DECL_ISUPPORTS

  // nsIPresShell
  NS_IMETHOD Init(nsIDocument* aDocument,
                  nsPresContext* aPresContext,
                  nsIViewManager* aViewManager,
                  nsStyleSet* aStyleSet,
                  nsCompatibility aCompatMode);
  NS_IMETHOD Destroy();

  virtual NS_HIDDEN_(void*) AllocateFrame(size_t aSize);
  virtual NS_HIDDEN_(void)  FreeFrame(size_t aSize, void* aFreeChunk);

  // Dynamic stack memory allocation
  NS_IMETHOD PushStackMemory();
  NS_IMETHOD PopStackMemory();
  NS_IMETHOD AllocateStackMemory(size_t aSize, void** aResult);

  NS_IMETHOD GetActiveAlternateStyleSheet(nsString& aSheetTitle);
  NS_IMETHOD SelectAlternateStyleSheet(const nsString& aSheetTitle);
  NS_IMETHOD ListAlternateStyleSheets(nsStringArray& aTitleList);
  NS_IMETHOD SetPreferenceStyleRules(PRBool aForceReflow);
  
  NS_IMETHOD GetSelection(SelectionType aType, nsISelection** aSelection);
  virtual nsISelection* GetCurrentSelection(SelectionType aType);

  NS_IMETHOD SetDisplaySelection(PRInt16 aToggle);
  NS_IMETHOD GetDisplaySelection(PRInt16 *aToggle);
  NS_IMETHOD ScrollSelectionIntoView(SelectionType aType, SelectionRegion aRegion, PRBool aIsSynchronous);
  NS_IMETHOD RepaintSelection(SelectionType aType);

  NS_IMETHOD BeginObservingDocument();
  NS_IMETHOD EndObservingDocument();
  NS_IMETHOD GetDidInitialReflow(PRBool *aDidInitialReflow);
  NS_IMETHOD InitialReflow(nscoord aWidth, nscoord aHeight);
  NS_IMETHOD ResizeReflow(nscoord aWidth, nscoord aHeight);
  NS_IMETHOD StyleChangeReflow();
  NS_IMETHOD GetPageSequenceFrame(nsIPageSequenceFrame** aResult) const;
  virtual NS_HIDDEN_(nsIFrame*) GetPrimaryFrameFor(nsIContent* aContent) const;

  NS_IMETHOD GetLayoutObjectFor(nsIContent*   aContent,
                                nsISupports** aResult) const;
  NS_IMETHOD GetPlaceholderFrameFor(nsIFrame*  aFrame,
                                    nsIFrame** aPlaceholderFrame) const;
  NS_IMETHOD AppendReflowCommand(nsIFrame*    aTargetFrame,
                                 nsReflowType aReflowType,
                                 nsIAtom*     aChildListName);
  NS_IMETHOD CancelReflowCommand(nsIFrame*     aTargetFrame, 
                                 nsReflowType* aCmdType);  
  void CancelReflowCommandInternal(nsIFrame*     aTargetFrame, 
                                   nsReflowType* aCmdType);
  NS_IMETHOD CancelAllReflowCommands();
  NS_IMETHOD IsSafeToFlush(PRBool& aIsSafeToFlush);
  NS_IMETHOD FlushPendingNotifications(mozFlushType aType);

  /**
   * Recreates the frames for a node
   */
  NS_IMETHOD RecreateFramesFor(nsIContent* aContent);

  /**
   * Post a request to handle a DOM event after Reflow has finished.
   */
  NS_IMETHOD PostDOMEvent(nsIContent* aContent, nsEvent* aEvent);

  /**
   * Post a request to set and attribute after reflow has finished.
   */
  NS_IMETHOD PostAttributeChange(nsIContent* aContent,
                                 PRInt32 aNameSpaceID, 
                                 nsIAtom* aName,
                                 const nsString& aValue,
                                 PRBool aNotify,
                                 nsAttributeChangeType aType);

  /**
   * Post a callback that should be handled after reflow has finished.
   */
  NS_IMETHOD PostReflowCallback(nsIReflowCallback* aCallback);
  NS_IMETHOD CancelReflowCallback(nsIReflowCallback* aCallback);

  /**
   * Reflow batching
   */   
  NS_IMETHOD BeginReflowBatching();
  NS_IMETHOD EndReflowBatching(PRBool aFlushPendingReflows);  
  NS_IMETHOD GetReflowBatchingStatus(PRBool* aBatch);
  
  NS_IMETHOD ClearFrameRefs(nsIFrame* aFrame);
  NS_IMETHOD CreateRenderingContext(nsIFrame *aFrame,
                                    nsIRenderingContext** aContext);
  NS_IMETHOD GoToAnchor(const nsAString& aAnchorName, PRBool aScroll);

  NS_IMETHOD ScrollFrameIntoView(nsIFrame *aFrame,
                                 PRIntn   aVPercent, 
                                 PRIntn   aHPercent) const;

  NS_IMETHOD SetIgnoreFrameDestruction(PRBool aIgnore);
  NS_IMETHOD NotifyDestroyingFrame(nsIFrame* aFrame);
  
  NS_IMETHOD DoCopy();
  NS_IMETHOD GetSelectionForCopy(nsISelection** outSelection);

  NS_IMETHOD GetLinkLocation(nsIDOMNode* aNode, nsAString& aLocationString);
  NS_IMETHOD DoGetContents(const nsACString& aMimeType, PRUint32 aFlags, PRBool aSelectionOnly, nsAString& outValue);

  NS_IMETHOD CaptureHistoryState(nsILayoutHistoryState** aLayoutHistoryState, PRBool aLeavingPage);

  NS_IMETHOD GetGeneratedContentIterator(nsIContent*          aContent,
                                         GeneratedContentType aType,
                                         nsIContentIterator** aIterator) const;
 
  NS_IMETHOD SetAnonymousContentFor(nsIContent* aContent, nsISupportsArray* aAnonymousElements);
  NS_IMETHOD GetAnonymousContentFor(nsIContent* aContent, nsISupportsArray** aAnonymousElements);
  NS_IMETHOD ReleaseAnonymousContent();

  NS_IMETHOD IsPaintingSuppressed(PRBool* aResult);
  NS_IMETHOD UnsuppressPainting();
  
  NS_IMETHOD DisableThemeSupport();
  virtual PRBool IsThemeSupportEnabled();

  virtual nsresult GetAgentStyleSheets(nsCOMArray<nsIStyleSheet>& aSheets);
  virtual nsresult SetAgentStyleSheets(const nsCOMArray<nsIStyleSheet>& aSheets);

  virtual nsresult AddOverrideStyleSheet(nsIStyleSheet *aSheet);
  virtual nsresult RemoveOverrideStyleSheet(nsIStyleSheet *aSheet);

  NS_IMETHOD HandleEventWithTarget(nsEvent* aEvent, nsIFrame* aFrame,
                                   nsIContent* aContent,
                                   nsEventStatus* aStatus);
  NS_IMETHOD GetEventTargetFrame(nsIFrame** aFrame);
  NS_IMETHOD GetEventTargetContent(nsEvent* aEvent, nsIContent** aContent);

  NS_IMETHOD IsReflowLocked(PRBool* aIsLocked);  

  virtual nsresult ReconstructFrames(void);
  virtual void Freeze();
  virtual void Thaw();
  
  NS_IMETHOD RenderOffscreen(nsRect aRect, PRBool aUntrusted,
                             PRBool aIgnoreViewportScrolling,
                             nscolor aBackgroundColor,
                             nsIRenderingContext** aRenderedContext);

#ifdef IBMBIDI
  NS_IMETHOD SetCaretBidiLevel(PRUint8 aLevel);
  NS_IMETHOD GetCaretBidiLevel(PRUint8 *aOutLevel);
  NS_IMETHOD UndefineCaretBidiLevel();
  NS_IMETHOD BidiStyleChangeReflow();
#endif

  //nsIViewObserver interface

  NS_IMETHOD Paint(nsIView *aView,
                   nsIRenderingContext* aRenderingContext,
                   const nsRegion& aDirtyRegion);
  NS_IMETHOD ComputeRepaintRegionForCopy(nsIView*      aRootView,
                                         nsIView*      aMovingView,
                                         nsPoint       aDelta,
                                         const nsRect& aCopyRect,
                                         nsRegion*     aRepaintRegion);
  NS_IMETHOD HandleEvent(nsIView*        aView,
                         nsGUIEvent*     aEvent,
                         nsEventStatus*  aEventStatus);
  NS_IMETHOD HandleDOMEventWithTarget(nsIContent* aTargetContent,
                                      nsEvent* aEvent,
                                      nsEventStatus* aStatus);
  NS_IMETHOD ResizeReflow(nsIView *aView, nscoord aWidth, nscoord aHeight);
  NS_IMETHOD_(PRBool) IsVisible();
  NS_IMETHOD_(void) WillPaint();

  // caret handling
  NS_IMETHOD GetCaret(nsICaret **aOutCaret);
  NS_IMETHOD SetCaretEnabled(PRBool aInEnable);
  NS_IMETHOD SetCaretReadOnly(PRBool aReadOnly);
  NS_IMETHOD GetCaretEnabled(PRBool *aOutEnabled);
  NS_IMETHOD SetCaretVisibilityDuringSelection(PRBool aVisibility);
  virtual already_AddRefed<nsICaret> SetCaret(nsICaret *aNewCaret);

  NS_IMETHOD SetSelectionFlags(PRInt16 aInEnable);
  NS_IMETHOD GetSelectionFlags(PRInt16 *aOutEnable);

  // nsISelectionController

  NS_IMETHOD CharacterMove(PRBool aForward, PRBool aExtend);
  NS_IMETHOD WordMove(PRBool aForward, PRBool aExtend);
  NS_IMETHOD LineMove(PRBool aForward, PRBool aExtend);
  NS_IMETHOD IntraLineMove(PRBool aForward, PRBool aExtend);
  NS_IMETHOD PageMove(PRBool aForward, PRBool aExtend);
  NS_IMETHOD ScrollPage(PRBool aForward);
  NS_IMETHOD ScrollLine(PRBool aForward);
  NS_IMETHOD ScrollHorizontal(PRBool aLeft);
  NS_IMETHOD CompleteScroll(PRBool aForward);
  NS_IMETHOD CompleteMove(PRBool aForward, PRBool aExtend);
  NS_IMETHOD SelectAll();
  NS_IMETHOD CheckVisibility(nsIDOMNode *node, PRInt16 startOffset, PRInt16 EndOffset, PRBool *_retval);

  // nsIDocumentObserver
  virtual void BeginUpdate(nsIDocument* aDocument, nsUpdateType aUpdateType);
  virtual void EndUpdate(nsIDocument* aDocument, nsUpdateType aUpdateType);
  virtual void BeginLoad(nsIDocument* aDocument);
  virtual void EndLoad(nsIDocument* aDocument);
  virtual void CharacterDataChanged(nsIDocument* aDocument,
                                    nsIContent* aContent,
                                    PRBool aAppend);
  virtual void ContentStatesChanged(nsIDocument* aDocument,
                                    nsIContent* aContent1,
                                    nsIContent* aContent2,
                                    PRInt32 aStateMask);
  virtual void AttributeChanged(nsIDocument* aDocument, nsIContent* aContent,
                                PRInt32 aNameSpaceID, nsIAtom* aAttribute,
                                PRInt32 aModType);
  virtual void ContentAppended(nsIDocument* aDocument, nsIContent* aContainer,
                               PRInt32 aNewIndexInContainer);
  virtual void ContentInserted(nsIDocument* aDocument, nsIContent* aContainer,
                               nsIContent* aChild, PRInt32 aIndexInContainer);
  virtual void ContentRemoved(nsIDocument* aDocument, nsIContent* aContainer,
                              nsIContent* aChild, PRInt32 aIndexInContainer);
  virtual void StyleSheetAdded(nsIDocument* aDocument,
                               nsIStyleSheet* aStyleSheet,
                               PRBool aDocumentSheet);
  virtual void StyleSheetRemoved(nsIDocument* aDocument,
                                 nsIStyleSheet* aStyleSheet,
                                 PRBool aDocumentSheet);
  virtual void StyleSheetApplicableStateChanged(nsIDocument* aDocument,
                                                nsIStyleSheet* aStyleSheet,
                                                PRBool aApplicable);
  virtual void StyleRuleChanged(nsIDocument* aDocument,
                                nsIStyleSheet* aStyleSheet,
                                nsIStyleRule* aOldStyleRule,
                                nsIStyleRule* aNewStyleRule);
  virtual void StyleRuleAdded(nsIDocument* aDocument,
                              nsIStyleSheet* aStyleSheet,
                              nsIStyleRule* aStyleRule);
  virtual void StyleRuleRemoved(nsIDocument* aDocument,
                                nsIStyleSheet* aStyleSheet,
                                nsIStyleRule* aStyleRule);

  NS_DECL_NSIOBSERVER

#ifdef MOZ_REFLOW_PERF
  NS_IMETHOD DumpReflows();
  NS_IMETHOD CountReflows(const char * aName, PRUint32 aType, nsIFrame * aFrame);
  NS_IMETHOD PaintCount(const char * aName, nsIRenderingContext* aRenderingContext, nsPresContext* aPresContext, nsIFrame * aFrame, PRUint32 aColor);

  NS_IMETHOD SetPaintFrameCount(PRBool aOn);
  
#endif

#ifdef DEBUG
  virtual void ListStyleContexts(nsIFrame *aRootFrame, FILE *out,
                                 PRInt32 aIndent = 0);

  virtual void ListStyleSheets(FILE *out, PRInt32 aIndent = 0);
  virtual void VerifyStyleTree();
#endif

#ifdef PR_LOGGING
  static PRLogModuleInfo* gLog;
#endif

protected:
  virtual ~PresShell();

  void HandlePostedDOMEvents();
  void HandlePostedAttributeChanges();
  void HandlePostedReflowCallbacks();

  void UnsuppressAndInvalidate();

  nsresult ReflowCommandAdded(nsHTMLReflowCommand* aRC);
  nsresult ReflowCommandRemoved(nsHTMLReflowCommand* aRC);

  // This method should be called after reflow commands have been
  // removed from the queue, but after the state in the presshell is
  // such that it's safe to flush (i.e. mIsReflowing == PR_FALSE)
  // If there are no load-created reflow commands and we blocked
  // onload on the document, we'll unblock it.
  void DoneRemovingReflowCommands();

  void     WillCauseReflow() { ++mChangeNestCount; }
  nsresult DidCauseReflow();
  void     WillDoReflow();
  void     DidDoReflow();
  nsresult ProcessReflowCommands(PRBool aInterruptible);
  void     ClearReflowEventStatus();
  void     PostReflowEvent();

  // Note: when PR_FALSE is returned, AlreadyInQueue assumes the command will
  // in fact be added to the queue.  If it's not, it needs to be removed from
  // mReflowCommandTable (AlreadyInQueue will insert it in that table).
  PRBool   AlreadyInQueue(nsHTMLReflowCommand* aReflowCommand);
  
  friend class nsPresShellEventCB;

  class ReflowEvent;
  friend class ReflowEvent;

  class ReflowEvent : public nsRunnable {
  public:
    NS_DECL_NSIRUNNABLE
    ReflowEvent(PresShell *aPresShell) : mPresShell(aPresShell) {
      NS_ASSERTION(aPresShell, "Null parameters!");
    }
    void Revoke() { mPresShell = nsnull; }
  private:  
    PresShell *mPresShell;
  };

  // Utility to determine if we're in the middle of a drag.
  PRBool IsDragInProgress ( ) const ;

  // Utility to find which view to scroll.
  nsIScrollableView* GetViewToScroll(nsLayoutUtils::Direction aDirection);

  PRBool mCaretEnabled;
#ifdef IBMBIDI
  PRUint8   mBidiLevel; // The Bidi level of the cursor
  nsCOMPtr<nsIBidiKeyboard> mBidiKeyboard;
#endif // IBMBIDI
#ifdef NS_DEBUG
  nsresult CloneStyleSet(nsStyleSet* aSet, nsStyleSet** aResult);
  PRBool VerifyIncrementalReflow();
  PRBool mInVerifyReflow;
  void ShowEventTargetDebug();
#endif

    /**
    * methods that manage rules that are used to implement the associated preferences
    *  - initially created for bugs 31816, 20760, 22963
    */
  nsresult ClearPreferenceStyleRules(void);
  nsresult CreatePreferenceStyleSheet(void);
  nsresult SetPrefColorRules(void);
  nsresult SetPrefLinkRules(void);
  nsresult SetPrefFocusRules(void);
  nsresult SetPrefNoScriptRule();
  nsresult SetPrefNoFramesRule(void);

  /**
   * Methods to handle changes to user and UA sheet lists that we get
   * notified about.
   */
  void AddUserSheet(nsISupports* aSheet);
  void AddAgentSheet(nsISupports* aSheet);
  void RemoveSheet(nsStyleSet::sheetType aType, nsISupports* aSheet);

  nsICSSStyleSheet*         mPrefStyleSheet; // mStyleSet owns it but we maintain a ref, may be null
#ifdef DEBUG
  PRUint32                  mUpdateCount;
#endif
  // normal reflow commands
  nsVoidArray               mReflowCommands;
  PLDHashTable              mReflowCommandTable;

  PRPackedBool mDocumentLoading;
  PRPackedBool mDocumentOnloadBlocked;
  PRPackedBool mIsReflowing;
  PRPackedBool mIsReleasingAnonymousContent;

  PRPackedBool mIgnoreFrameDestruction;
  PRPackedBool mHaveShutDown;

  // This is used to protect ourselves from triggering reflow while in the
  // middle of frame construction and the like... it really shouldn't be
  // needed, one hopes, but it is for now.
  PRUint32  mChangeNestCount;
  
  nsIFrame*   mCurrentEventFrame;
  nsCOMPtr<nsIContent> mCurrentEventContent;
  nsVoidArray mCurrentEventFrameStack;
  nsCOMArray<nsIContent> mCurrentEventContentStack;
  nsSupportsHashtable* mAnonymousContentTable;

#ifdef NS_DEBUG
  nsRect mCurrentTargetRect;
  nsIView* mCurrentTargetView;
#endif
  
  nsCOMPtr<nsICaret>            mCaret;
  PRInt16                       mSelectionFlags;
  PRPackedBool                  mBatchReflows;  // When set to true, the pres shell batches reflow commands.
  PresShellViewEventListener    *mViewEventListener;
  FrameArena                    mFrameArena;
  StackArena*                   mStackArena;
  nsCOMPtr<nsIDragService>      mDragService;
  PRInt32                       mRCCreatedDuringLoad; // Counter to keep track of reflow commands created during doc
  
  nsRevocableEventPtr<ReflowEvent> mReflowEvent;

  // used for list of posted events and attribute changes. To be done
  // after reflow.
  nsDOMEventRequest* mFirstDOMEventRequest;
  nsDOMEventRequest* mLastDOMEventRequest;
  nsAttributeChangeRequest* mFirstAttributeRequest;
  nsAttributeChangeRequest* mLastAttributeRequest;
  nsCallbackEventRequest* mFirstCallbackEventRequest;
  nsCallbackEventRequest* mLastCallbackEventRequest;

  PRPackedBool      mIsThemeSupportDisabled;  // Whether or not form controls should use nsITheme in this shell.

  PRPackedBool      mIsDocumentGone;      // We've been disconnected from the document.
  PRPackedBool      mPaintingSuppressed;  // For all documents we initially lock down painting.
                                          // We will refuse to paint the document until either
                                          // (a) our timer fires or (b) all frames are constructed.
  PRPackedBool      mShouldUnsuppressPainting;  // Indicates that it is safe to unlock painting once all pending
                                                // reflows have been processed.
  nsCOMPtr<nsITimer> mPaintSuppressionTimer; // This timer controls painting suppression.  Until it fires
                                             // or all frames are constructed, we won't paint anything but
                                             // our <body> background and scrollbars.
#define PAINTLOCK_EVENT_DELAY 250 // 250ms.  This is actually
                                  // pref-controlled, but we use this
                                  // value if we fail to get the pref
                                  // for any reason.

  static void sPaintSuppressionCallback(nsITimer* aTimer, void* aPresShell); // A callback for the timer.

  MOZ_TIMER_DECLARE(mReflowWatch)  // Used for measuring time spent in reflow
  MOZ_TIMER_DECLARE(mFrameCreationWatch)  // Used for measuring time spent in frame creation 

#ifdef MOZ_REFLOW_PERF
  ReflowCountMgr * mReflowCountMgr;
#endif

private:

  PRBool InZombieDocument(nsIContent *aContent);
  nsresult RetargetEventToParent(nsIView *aView, nsGUIEvent* aEvent, 
                                 nsEventStatus*  aEventStatus,
                                 nsIContent *aZombieFocusedContent);

  void FreeDynamicStack();

  //helper funcs for event handling
protected:
  //protected because nsPresShellEventCB needs this.
  nsIFrame* GetCurrentEventFrame();
private:
  void PushCurrentEventInfo(nsIFrame* aFrame, nsIContent* aContent);
  void PopCurrentEventInfo();
  nsresult HandleEventInternal(nsEvent* aEvent, nsIView* aView,
                               nsEventStatus *aStatus);
  nsresult HandlePositionedEvent(nsIView*       aView,
                                 nsIFrame*      aTargetFrame,
                                 nsGUIEvent*    aEvent,
                                 nsEventStatus* aEventStatus);

  //help funcs for resize events
  void CreateResizeEventTimer();
  void KillResizeEventTimer();
  void FireResizeEvent();
  static void sResizeEventCallback(nsITimer* aTimer, void* aPresShell) ;
  nsCOMPtr<nsITimer> mResizeEventTimer;

  typedef void (*nsPluginEnumCallback)(PresShell*, nsIContent*);
  void EnumeratePlugins(nsIDOMDocument *aDocument,
                        const nsString &aPluginTag,
                        nsPluginEnumCallback aCallback);
};

class nsPresShellEventCB : public nsDispatchingCallback
{
public:
  nsPresShellEventCB(PresShell* aPresShell) : mPresShell(aPresShell) {}

  virtual void HandleEvent(nsEventChainPostVisitor& aVisitor)
  {
    if (aVisitor.mPresContext && aVisitor.mEvent->eventStructType != NS_EVENT) {
      nsIFrame* frame = mPresShell->GetCurrentEventFrame();
      if (frame) {
        frame->HandleEvent(aVisitor.mPresContext,
                           (nsGUIEvent*) aVisitor.mEvent,
                           &aVisitor.mEventStatus);
      }
    }
  }

  nsRefPtr<PresShell> mPresShell;
};

#ifdef PR_LOGGING
PRLogModuleInfo* PresShell::gLog;
#endif

#ifdef NS_DEBUG
static void
VerifyStyleTree(nsPresContext* aPresContext, nsFrameManager* aFrameManager)
{
  if (nsIFrameDebug::GetVerifyStyleTreeEnable()) {
    nsIFrame* rootFrame = aFrameManager->GetRootFrame();
    aFrameManager->DebugVerifyStyleTree(rootFrame);
  }
}
#define VERIFY_STYLE_TREE ::VerifyStyleTree(mPresContext, FrameManager())
#else
#define VERIFY_STYLE_TREE
#endif

static PRBool gVerifyReflowEnabled;

PRBool
nsIPresShell::GetVerifyReflowEnable()
{
#ifdef NS_DEBUG
  static PRBool firstTime = PR_TRUE;
  if (firstTime) {
    firstTime = PR_FALSE;
    char* flags = PR_GetEnv("GECKO_VERIFY_REFLOW_FLAGS");
    if (flags) {
      PRBool error = PR_FALSE;

      for (;;) {
        char* comma = PL_strchr(flags, ',');
        if (comma)
          *comma = '\0';

        PRBool found = PR_FALSE;
        const VerifyReflowFlags* flag = gFlags;
        const VerifyReflowFlags* limit = gFlags + NUM_VERIFY_REFLOW_FLAGS;
        while (flag < limit) {
          if (PL_strcasecmp(flag->name, flags) == 0) {
            gVerifyReflowFlags |= flag->bit;
            found = PR_TRUE;
            break;
          }
          ++flag;
        }

        if (! found)
          error = PR_TRUE;

        if (! comma)
          break;

        *comma = ',';
        flags = comma + 1;
      }

      if (error)
        ShowVerifyReflowFlags();
    }

    if (VERIFY_REFLOW_ON & gVerifyReflowFlags) {
      gVerifyReflowEnabled = PR_TRUE;
    }
    printf("Note: verifyreflow is %sabled",
           gVerifyReflowEnabled ? "en" : "dis");
    if (VERIFY_REFLOW_NOISY & gVerifyReflowFlags) {
      printf(" (noisy)");
    }
    if (VERIFY_REFLOW_ALL & gVerifyReflowFlags) {
      printf(" (all)");
    }
    if (VERIFY_REFLOW_DUMP_COMMANDS & gVerifyReflowFlags) {
      printf(" (show reflow commands)");
    }
    if (VERIFY_REFLOW_NOISY_RC & gVerifyReflowFlags) {
      printf(" (noisy reflow commands)");
      if (VERIFY_REFLOW_REALLY_NOISY_RC & gVerifyReflowFlags) {
        printf(" (REALLY noisy reflow commands)");
      }
    }
    printf("\n");
  }
#endif
  return gVerifyReflowEnabled;
}

void
nsIPresShell::SetVerifyReflowEnable(PRBool aEnabled)
{
  gVerifyReflowEnabled = aEnabled;
}

PRInt32
nsIPresShell::GetVerifyReflowFlags()
{
#ifdef NS_DEBUG
  return gVerifyReflowFlags;
#else
  return 0;
#endif
}

void
nsIPresShell::AddWeakFrame(nsWeakFrame* aWeakFrame)
{
  if (aWeakFrame->GetFrame()) {
    aWeakFrame->GetFrame()->AddStateBits(NS_FRAME_EXTERNAL_REFERENCE);
  }
  aWeakFrame->SetPreviousWeakFrame(mWeakFrames);
  mWeakFrames = aWeakFrame;
}

void
nsIPresShell::RemoveWeakFrame(nsWeakFrame* aWeakFrame)
{
  if (mWeakFrames == aWeakFrame) {
    mWeakFrames = aWeakFrame->GetPreviousWeakFrame();
    return;
  }
  nsWeakFrame* nextWeak = mWeakFrames;
  while (nextWeak && nextWeak->GetPreviousWeakFrame() != aWeakFrame) {
    nextWeak = nextWeak->GetPreviousWeakFrame();
  }
  if (nextWeak) {
    nextWeak->SetPreviousWeakFrame(aWeakFrame->GetPreviousWeakFrame());
  }
}

//----------------------------------------------------------------------

nsresult
NS_NewPresShell(nsIPresShell** aInstancePtrResult)
{
  NS_PRECONDITION(nsnull != aInstancePtrResult, "null ptr");
  if (nsnull == aInstancePtrResult) {
    return NS_ERROR_NULL_POINTER;
  }
  PresShell* it = new PresShell();
  if (nsnull == it) {
    return NS_ERROR_OUT_OF_MEMORY;
  }
  return it->QueryInterface(NS_GET_IID(nsIPresShell),
                            (void **) aInstancePtrResult);
}

PresShell::PresShell()
#ifdef IBMBIDI
  : mBidiLevel(BIDI_LEVEL_UNDEFINED)
#endif
{
  mIsAccessibilityActive = PR_FALSE;
  mSelection = nsnull;
#ifdef MOZ_REFLOW_PERF
  mReflowCountMgr = new ReflowCountMgr();
  mReflowCountMgr->SetPresContext(mPresContext);
  mReflowCountMgr->SetPresShell(this);
#endif
#ifdef IBMBIDI
  mBidiLevel = BIDI_LEVEL_UNDEFINED;
#endif
#ifdef PR_LOGGING
  if (! gLog)
    gLog = PR_NewLogModule("PresShell");
#endif
  mSelectionFlags = nsISelectionDisplay::DISPLAY_TEXT | nsISelectionDisplay::DISPLAY_IMAGES;
  mIsThemeSupportDisabled = PR_FALSE;
  mIsReleasingAnonymousContent = PR_FALSE;

  new (this) nsFrameManager();
}

NS_IMPL_ISUPPORTS8(PresShell, nsIPresShell, nsIDocumentObserver,
                   nsIViewObserver, nsISelectionController,
                   nsISelectionDisplay, nsIObserver, nsISupportsWeakReference,
                   nsIMutationObserver)

PresShell::~PresShell()
{
  if (!mHaveShutDown) {
    NS_NOTREACHED("Someone did not call nsIPresShell::destroy");
    Destroy();
  }

  NS_ASSERTION(mCurrentEventContentStack.Count() == 0,
               "Huh, event content left on the stack in pres shell dtor!");
  NS_ASSERTION(mFirstDOMEventRequest == nsnull &&
               mLastDOMEventRequest == nsnull &&
               mFirstAttributeRequest == nsnull &&
               mLastAttributeRequest == nsnull &&
               mFirstCallbackEventRequest == nsnull &&
               mLastCallbackEventRequest == nsnull,
               "post-reflow queues not empty.  This means we're leaking");
 
  delete mStyleSet;
  delete mFrameConstructor;

  mCurrentEventContent = nsnull;

  // if we allocated any stack memory free it.
  FreeDynamicStack();

  NS_IF_RELEASE(mPresContext);
  NS_IF_RELEASE(mDocument);
  NS_IF_RELEASE(mSelection);
}

/**
 * Initialize the presentation shell. Create view manager and style
 * manager.
 */
NS_IMETHODIMP
PresShell::Init(nsIDocument* aDocument,
                nsPresContext* aPresContext,
                nsIViewManager* aViewManager,
                nsStyleSet* aStyleSet,
                nsCompatibility aCompatMode)
{
  NS_PRECONDITION(nsnull != aDocument, "null ptr");
  NS_PRECONDITION(nsnull != aPresContext, "null ptr");
  NS_PRECONDITION(nsnull != aViewManager, "null ptr");

  if ((nsnull == aDocument) || (nsnull == aPresContext) ||
      (nsnull == aViewManager)) {
    return NS_ERROR_NULL_POINTER;
  }
  if (mDocument) {
    NS_WARNING("PresShell double init'ed");
    return NS_ERROR_ALREADY_INITIALIZED;
  }

  mDocument = aDocument;
  NS_ADDREF(mDocument);
  mViewManager = aViewManager;

  // Create our frame constructor.
  mFrameConstructor = new nsCSSFrameConstructor(mDocument, this);
  NS_ENSURE_TRUE(mFrameConstructor, NS_ERROR_OUT_OF_MEMORY);

  // The document viewer owns both view manager and pres shell.
  mViewManager->SetViewObserver(this);

  // Bind the context to the presentation shell.
  mPresContext = aPresContext;
  NS_ADDREF(mPresContext);
  aPresContext->SetShell(this);

  // Create our reflow command hashtable
  static PLDHashTableOps reflowCommandOps =
    {
      PL_DHashAllocTable,
      PL_DHashFreeTable,
      ReflowCommandHashGetKey,
      ReflowCommandHashHashKey,
      ReflowCommandHashMatchEntry,
      PL_DHashMoveEntryStub,
      PL_DHashClearEntryStub,
      PL_DHashFinalizeStub
    };

  if (!PL_DHashTableInit(&mReflowCommandTable, &reflowCommandOps,
                         nsnull, sizeof(ReflowCommandEntry), 16)) {
    mReflowCommandTable.ops = nsnull;
    return NS_ERROR_OUT_OF_MEMORY;
  }
  
  // Now we can initialize the style set.
  nsresult result = aStyleSet->Init(aPresContext);
  NS_ENSURE_SUCCESS(result, result);

  // From this point on, any time we return an error we need to make
  // sure to null out mStyleSet first, since an error return from this
  // method will cause the caller to delete the style set, so we don't
  // want to delete it in our destructor.
  mStyleSet = aStyleSet;

  // Set the compatibility mode after attaching the pres context and
  // style set, but before creating any frames.
  mPresContext->SetCompatibilityMode(aCompatMode);

  // setup the preference style rules (no forced reflow), and do it
  // before creating any frames.
  SetPreferenceStyleRules(PR_FALSE);

  result = CallCreateInstance(kFrameSelectionCID, &mSelection);
  if (NS_FAILED(result)) {
    mStyleSet = nsnull;
    return result;
  }

  // Create and initialize the frame manager
  result = FrameManager()->Init(this, mStyleSet);
  if (NS_FAILED(result)) {
    NS_WARNING("Frame manager initialization failed");
    mStyleSet = nsnull;
    return result;
  }

  mSelection->Init(this, nsnull);

  // Important: this has to happen after the selection has been set up
#ifdef SHOW_CARET
  // make the caret
  nsresult  err = NS_NewCaret(getter_AddRefs(mCaret));
  if (NS_SUCCEEDED(err))
  {
    mCaret->Init(this);
  }

  //SetCaretEnabled(PR_TRUE);       // make it show in browser windows
#endif  
  //set up selection to be displayed in document
  // Don't enable selection for print media
  nsPresContext::nsPresContextType type = aPresContext->Type();
  if (type != nsPresContext::eContext_PrintPreview &&
      type != nsPresContext::eContext_Print)
    SetDisplaySelection(nsISelectionController::SELECTION_DISABLED);
  
  if (gMaxRCProcessingTime == -1) {
    gMaxRCProcessingTime =
      nsContentUtils::GetIntPref("layout.reflow.timeslice",
                                 NS_MAX_REFLOW_TIME);
  }

  {
    nsCOMPtr<nsIObserverService> os =
      do_GetService("@mozilla.org/observer-service;1", &result);
    if (os) {
      os->AddObserver(this, NS_LINK_VISITED_EVENT_TOPIC, PR_FALSE);
      os->AddObserver(this, "agent-sheet-added", PR_FALSE);
      os->AddObserver(this, "user-sheet-added", PR_FALSE);
      os->AddObserver(this, "agent-sheet-removed", PR_FALSE);
      os->AddObserver(this, "user-sheet-removed", PR_FALSE);
#ifdef MOZ_XUL
      os->AddObserver(this, "chrome-flush-skin-caches", PR_FALSE);
#endif
    }
  }

  // cache the drag service so we can check it during reflows
  mDragService = do_GetService("@mozilla.org/widget/dragservice;1");

#ifdef IBMBIDI
  mBidiKeyboard = do_GetService("@mozilla.org/widget/bidikeyboard;1");
#endif

#ifdef MOZ_REFLOW_PERF
    if (mReflowCountMgr) {
      PRBool paintFrameCounts =
        nsContentUtils::GetBoolPref("layout.reflow.showframecounts");

      PRBool dumpFrameCounts =
        nsContentUtils::GetBoolPref("layout.reflow.dumpframecounts");

      PRBool dumpFrameByFrameCounts =
        nsContentUtils::GetBoolPref("layout.reflow.dumpframebyframecounts");

      mReflowCountMgr->SetDumpFrameCounts(dumpFrameCounts);
      mReflowCountMgr->SetDumpFrameByFrameCounts(dumpFrameByFrameCounts);
      mReflowCountMgr->SetPaintFrameCounts(paintFrameCounts);
    }
#endif

  return NS_OK;
}

NS_IMETHODIMP
PresShell::Destroy()
{
#ifdef MOZ_REFLOW_PERF
  DumpReflows();
  if (mReflowCountMgr) {
    delete mReflowCountMgr;
    mReflowCountMgr = nsnull;
  }
#endif

  if (mHaveShutDown)
    return NS_OK;

  if (mPresContext) {
    // We need to notify the destroying the nsPresContext to ESM for
    // suppressing to use from ESM.
    mPresContext->EventStateManager()->NotifyDestroyPresContext(mPresContext);
  }

  {
    nsCOMPtr<nsIObserverService> os =
      do_GetService("@mozilla.org/observer-service;1");
    if (os) {
      os->RemoveObserver(this, NS_LINK_VISITED_EVENT_TOPIC);
      os->RemoveObserver(this, "agent-sheet-added");
      os->RemoveObserver(this, "user-sheet-added");
      os->RemoveObserver(this, "agent-sheet-removed");
      os->RemoveObserver(this, "user-sheet-removed");
#ifdef MOZ_XUL
      os->RemoveObserver(this, "chrome-flush-skin-caches");
#endif
    }
  }

  // If our paint suppression timer is still active, kill it.
  if (mPaintSuppressionTimer) {
    mPaintSuppressionTimer->Cancel();
    mPaintSuppressionTimer = nsnull;
  }

  if (mCaret) {
    mCaret->Terminate();
    mCaret = nsnull;
  }
  
  // release our pref style sheet, if we have one still
  ClearPreferenceStyleRules();

  // free our table of anonymous content
  ReleaseAnonymousContent();

  mIsDestroying = PR_TRUE;

  // We can't release all the event content in
  // mCurrentEventContentStack here since there might be code on the
  // stack that will release the event content too. Double release
  // bad!

  // The frames will be torn down, so remove them from the current
  // event frame stack (since they'd be dangling references if we'd
  // leave them in) and null out the mCurrentEventFrame pointer as
  // well.

  mCurrentEventFrame = nsnull;

  PRInt32 i, count = mCurrentEventFrameStack.Count();
  for (i = 0; i < count; i++) {
    mCurrentEventFrameStack.ReplaceElementAt(nsnull, i);
  }

  if (mViewManager) {
    // Clear the view manager's weak pointer back to |this| in case it
    // was leaked.
    mViewManager->SetViewObserver(nsnull);
    mViewManager = nsnull;
  }

  mStyleSet->BeginShutdown(mPresContext);

  // This shell must be removed from the document before the frame
  // hierarchy is torn down to avoid finding deleted frames through
  // this presshell while the frames are being torn down
  if (mDocument) {
    mDocument->DeleteShell(this);
  }

  // Destroy the frame manager. This will destroy the frame hierarchy
  mFrameConstructor->WillDestroyFrameTree();
  FrameManager()->Destroy();

  NS_WARN_IF_FALSE(!mWeakFrames, "Weak frames alive after destroying FrameManager");
  while (mWeakFrames) {
    mWeakFrames->Clear(this);
  }

  // Let the style set do its cleanup.
  mStyleSet->Shutdown(mPresContext);

  if (mPresContext) {
    // Clear out the prescontext's property table -- since our frame tree is
    // now dead, we shouldn't be looking up any more properties in that table.
    // We want to do this before we call SetShell() on the prescontext, so
    // property destructors can usefully call GetPresShell() on the
    // prescontext.
    mPresContext->PropertyTable()->DeleteAllProperties();

    // We hold a reference to the pres context, and it holds a weak link back
    // to us. To avoid the pres context having a dangling reference, set its 
    // pres shell to NULL
    mPresContext->SetShell(nsnull);

    // Clear the link handler (weak reference) as well
    mPresContext->SetLinkHandler(nsnull);
  }

  if (mViewEventListener) {
    mViewEventListener->SetPresShell((nsIPresShell*)nsnull);
    NS_RELEASE(mViewEventListener);
  }

  // Revoke any pending reflow event
  mReflowEvent.Revoke();

  CancelAllReflowCommands();

  NS_ASSERTION(!mDocumentOnloadBlocked,
               "CancelAllReflowCommands() didn't unblock onload?");

  KillResizeEventTimer();

  // Now that mReflowCommandTable won't be accessed anymore, finish it
  if (mReflowCommandTable.ops) {
    PL_DHashTableFinish(&mReflowCommandTable);
  }
  
  mHaveShutDown = PR_TRUE;

  return NS_OK;
}

                  // Dynamic stack memory allocation
NS_IMETHODIMP
PresShell::PushStackMemory()
{
  if (!mStackArena) {
    mStackArena = new StackArena();
    if (!mStackArena)
      return NS_ERROR_OUT_OF_MEMORY;
  }

  return mStackArena->Push();
}

NS_IMETHODIMP
PresShell::PopStackMemory()
{
  NS_ENSURE_TRUE(mStackArena, NS_ERROR_UNEXPECTED);

  return mStackArena->Pop();
}

NS_IMETHODIMP
PresShell::AllocateStackMemory(size_t aSize, void** aResult)
{
  if (!mStackArena) {
    mStackArena = new StackArena();
    if (!mStackArena)
      return NS_ERROR_OUT_OF_MEMORY;
  }

  return mStackArena->Allocate(aSize, aResult);
}

void
PresShell::FreeDynamicStack()
{
  if (mStackArena) {
    delete mStackArena;
    mStackArena = nsnull;
  }
}
 

void
PresShell::FreeFrame(size_t aSize, void* aPtr)
{
  mFrameArena.FreeFrame(aSize, aPtr);
}

void*
PresShell::AllocateFrame(size_t aSize)
{
  return mFrameArena.AllocateFrame(aSize);
}

NS_IMETHODIMP
PresShell::GetActiveAlternateStyleSheet(nsString& aSheetTitle)
{ // first non-html sheet in style set that has title
  if (mStyleSet) {
    PRInt32 count = mStyleSet->SheetCount(nsStyleSet::eDocSheet);
    PRInt32 index;
    NS_NAMED_LITERAL_STRING(textHtml, "text/html");
    for (index = 0; index < count; index++) {
      nsIStyleSheet* sheet = mStyleSet->StyleSheetAt(nsStyleSet::eDocSheet,
                                                     index);
      if (nsnull != sheet) {
        nsAutoString type;
        sheet->GetType(type);
        if (PR_FALSE == type.Equals(textHtml)) {
          nsAutoString title;
          sheet->GetTitle(title);
          if (!title.IsEmpty()) {
            aSheetTitle = title;
            index = count;  // stop looking
          }
        }
      }
    }
  }
  return NS_OK;
}

NS_IMETHODIMP
PresShell::SelectAlternateStyleSheet(const nsString& aSheetTitle)
{
  if (mDocument && mStyleSet) {
    mStyleSet->BeginUpdate();
    PRInt32 count = mDocument->GetNumberOfStyleSheets();
    PRInt32 index;
    NS_NAMED_LITERAL_STRING(textHtml,"text/html");
    for (index = 0; index < count; index++) {
      nsIStyleSheet *sheet = mDocument->GetStyleSheetAt(index);
      PRBool complete;
      sheet->GetComplete(complete);
      if (complete) {
        nsAutoString type;
        sheet->GetType(type);
        if (!type.Equals(textHtml)) {
          nsAutoString title;
          sheet->GetTitle(title);
          if (!title.IsEmpty()) {
            if (title.Equals(aSheetTitle)) {
              mStyleSet->AddDocStyleSheet(sheet, mDocument);
            }
            else {
              mStyleSet->RemoveStyleSheet(nsStyleSet::eDocSheet, sheet);
            }
          }
        }
      }
    }

    mStyleSet->EndUpdate();
    ReconstructStyleData();
  }
  return NS_OK;
}

NS_IMETHODIMP
PresShell::ListAlternateStyleSheets(nsStringArray& aTitleList)
{
  // XXX should this be returning incomplete sheets?  Probably.
  if (mDocument) {
    PRInt32 count = mDocument->GetNumberOfStyleSheets();
    PRInt32 index;
    NS_NAMED_LITERAL_STRING(textHtml,"text/html");
    for (index = 0; index < count; index++) {
      nsIStyleSheet *sheet = mDocument->GetStyleSheetAt(index);
      if (sheet) {
        nsAutoString type;
        sheet->GetType(type);
        if (PR_FALSE == type.Equals(textHtml)) {
          nsAutoString  title;
          sheet->GetTitle(title);
          if (!title.IsEmpty()) {
            if (-1 == aTitleList.IndexOf(title)) {
              aTitleList.AppendString(title);
            }
          }
        }
      }
    }
  }
  return NS_OK;
}

void
nsIPresShell::SetAuthorStyleDisabled(PRBool aStyleDisabled)
{
  if (aStyleDisabled != mStyleSet->GetAuthorStyleDisabled()) {
    mStyleSet->SetAuthorStyleDisabled(aStyleDisabled);
    ReconstructStyleData();
  }
}

PRBool
nsIPresShell::GetAuthorStyleDisabled()
{
  return mStyleSet->GetAuthorStyleDisabled();
}

NS_IMETHODIMP
PresShell::SetPreferenceStyleRules(PRBool aForceReflow)
{
  if (!mDocument) {
    return NS_ERROR_NULL_POINTER;
  }

  nsPIDOMWindow *window = mDocument->GetWindow();

  // If the document doesn't have a window there's no need to notify
  // its presshell about changes to preferences since the document is
  // in a state where it doesn't matter any more (see
  // DocumentViewerImpl::Close()).

  if (!window) {
    return NS_ERROR_NULL_POINTER;
  } 

  NS_PRECONDITION(mPresContext, "presContext cannot be null");
  if (mPresContext) {
    nsresult result = NS_OK;

    // first, make sure this is not a chrome shell 
    nsCOMPtr<nsISupports> container = mPresContext->GetContainer();
    if (container) {
      nsCOMPtr<nsIDocShellTreeItem> docShell(do_QueryInterface(container, &result));
      if (NS_SUCCEEDED(result) && docShell){
        PRInt32 docShellType;
        result = docShell->GetItemType(&docShellType);
        if (NS_SUCCEEDED(result)){
          if (nsIDocShellTreeItem::typeChrome == docShellType){
            return NS_OK;
          }
        }      
      }
    }
    if (NS_SUCCEEDED(result)) {
    
#ifdef DEBUG_attinasi
      printf("Setting Preference Style Rules:\n");
#endif
      // if here, we need to create rules for the prefs
      // - this includes the background-color, the text-color,
      //   the link color, the visited link color and the link-underlining
    
      // first clear any exising rules
      result = ClearPreferenceStyleRules();
      
      // now do the color rules
      if (NS_SUCCEEDED(result)) {
        result = SetPrefColorRules();
      }

      // now the link rules (must come after the color rules, or links will not be correct color!)
      // XXX - when there is both an override and agent pref stylesheet this won't matter,
      //       as the color rules will be overrides and the links rules will be agent
      if (NS_SUCCEEDED(result)) {
        result = SetPrefLinkRules();
      }
      if (NS_SUCCEEDED(result)) {
        result = SetPrefFocusRules();
      }
      if (NS_SUCCEEDED(result)) {
        result = SetPrefNoScriptRule();
      }
      if (NS_SUCCEEDED(result)) {
        result = SetPrefNoFramesRule();
      }
    }
#ifdef DEBUG_attinasi
    printf( "Preference Style Rules set: error=%ld\n", (long)result);
#endif    

    if (aForceReflow){
      mPresContext->ClearStyleDataAndReflow();
    }

    return result;
  }

  return NS_ERROR_NULL_POINTER;
}

nsresult PresShell::ClearPreferenceStyleRules(void)
{
  nsresult result = NS_OK;
  if (mPrefStyleSheet) {
    NS_ASSERTION(mStyleSet, "null styleset entirely unexpected!");
    if (mStyleSet) {
      // remove the sheet from the styleset: 
      // - note that we have to check for success by comparing the count before and after...
#ifdef NS_DEBUG
      PRInt32 numBefore = mStyleSet->SheetCount(nsStyleSet::eUserSheet);
      NS_ASSERTION(numBefore > 0, "no user stylesheets in styleset, but we have one!");
#endif
      mStyleSet->RemoveStyleSheet(nsStyleSet::eUserSheet, mPrefStyleSheet);

#ifdef DEBUG_attinasi
      NS_ASSERTION((numBefore - 1) == mStyleSet->GetNumberOfUserStyleSheets(),
                   "Pref stylesheet was not removed");
      printf("PrefStyleSheet removed\n");
#endif
      // clear the sheet pointer: it is strictly historical now
      NS_RELEASE(mPrefStyleSheet);
    }
  }
  return result;
}

nsresult PresShell::CreatePreferenceStyleSheet(void)
{
  NS_ASSERTION(!mPrefStyleSheet, "prefStyleSheet already exists");
  nsresult result = CallCreateInstance(kCSSStyleSheetCID, &mPrefStyleSheet);
  if (NS_SUCCEEDED(result)) {
    NS_ASSERTION(mPrefStyleSheet, "null but no error");
    nsCOMPtr<nsIURI> uri;
    result = NS_NewURI(getter_AddRefs(uri), "about:PreferenceStyleSheet", nsnull);
    if (NS_SUCCEEDED(result)) {
      NS_ASSERTION(uri, "null but no error");
      result = mPrefStyleSheet->SetURIs(uri, uri);
      if (NS_SUCCEEDED(result)) {
        mPrefStyleSheet->SetComplete();
        nsCOMPtr<nsIDOMCSSStyleSheet> sheet(do_QueryInterface(mPrefStyleSheet));
        if (sheet) {
          PRUint32 index;
          result = sheet->InsertRule(NS_LITERAL_STRING("@namespace url(http://www.w3.org/1999/xhtml);"),
                                     0, &index);
          NS_ENSURE_SUCCESS(result, result);
        }
        mStyleSet->AppendStyleSheet(nsStyleSet::eUserSheet, mPrefStyleSheet);
      }
    }
  } else {
    result = NS_ERROR_OUT_OF_MEMORY;
  }

#ifdef DEBUG_attinasi
  printf("CreatePrefStyleSheet completed: error=%ld\n",(long)result);
#endif

  return result;
}

// XXX We want these after the @namespace rule.  Does order matter
// for these rules, or can we call nsICSSStyleRule::StyleRuleCount()
// and just "append"?
static PRUint32 sInsertPrefSheetRulesAt = 1;

nsresult PresShell::SetPrefColorRules(void)
{
  NS_ASSERTION(mPresContext,"null prescontext not allowed");
  if (mPresContext) {
    nsresult result = NS_OK;

    // see if we need to create the rules first
    PRBool useDocColors =
      mPresContext->GetCachedBoolPref(kPresContext_UseDocumentColors);
    if (!useDocColors) {

#ifdef DEBUG_attinasi
      printf(" - Creating rules for document colors\n");
#endif

      // OK, not using document colors, so we have to force the user's colors via style rules
      if (!mPrefStyleSheet) {
        result = CreatePreferenceStyleSheet();
      }
      if (NS_SUCCEEDED(result)) {
        NS_ASSERTION(mPrefStyleSheet, "prefstylesheet should not be null");

        nscolor bgColor = mPresContext->DefaultBackgroundColor();
        nscolor textColor = mPresContext->DefaultColor();

        // get the DOM interface to the stylesheet
        nsCOMPtr<nsIDOMCSSStyleSheet> sheet(do_QueryInterface(mPrefStyleSheet,&result));
        if (NS_SUCCEEDED(result)) {
          PRUint32 index = 0;
          nsAutoString strColor, strBackgroundColor;

          // create a rule for background and foreground color and
          // add it to the style sheet              
          // - the rule is !important so it overrides all but author
          //   important rules (when put into an agent stylesheet) and 
          //   all (even author important) when put into an override stylesheet

          ///////////////////////////////////////////////////////////////
          // - default colors: ':root {color:#RRGGBB !important;
          //                           background: #RRGGBB !important;}'
          ColorToString(textColor,strColor);
          ColorToString(bgColor,strBackgroundColor);
          result = sheet->InsertRule(NS_LITERAL_STRING("*|*:root {color:") +
                                     strColor +
                                     NS_LITERAL_STRING(" !important; ") +
                                     NS_LITERAL_STRING("border-color: -moz-use-text-color !important; ") +
                                     NS_LITERAL_STRING("background:") +
                                     strBackgroundColor +
                                     NS_LITERAL_STRING(" !important; }"),
                                     sInsertPrefSheetRulesAt, &index);
          NS_ENSURE_SUCCESS(result, result);

          ///////////////////////////////////////////////////////////////
          // - everything else inherits the color
          // (the background color will be handled in 
          //  nsRuleNode::ComputeBackgroundData)
          result = sheet->InsertRule(NS_LITERAL_STRING("*|* {color: inherit !important; border-color: -moz-use-text-color !important; background-image: none !important; } "),
                                     sInsertPrefSheetRulesAt, &index);
        }
      }
    }
    return result;
  } else {
    return NS_ERROR_FAILURE;
  }
}

nsresult
PresShell::SetPrefNoScriptRule()
{
  nsresult rv = NS_OK;

  PRBool scriptEnabled = mDocument->IsScriptEnabled() ||
    (mPresContext->Type() == nsPresContext::eContext_PrintPreview &&
     NS_PTR_TO_INT32(mDocument->GetProperty(
                       nsLayoutAtoms::scriptEnabledBeforePrintPreview)));

  if (scriptEnabled) {
    if (!mPrefStyleSheet) {
      rv = CreatePreferenceStyleSheet();
      NS_ENSURE_SUCCESS(rv, rv);
    }
    // get the DOM interface to the stylesheet
    nsCOMPtr<nsIDOMCSSStyleSheet> sheet(do_QueryInterface(mPrefStyleSheet, &rv));
    NS_ENSURE_SUCCESS(rv, rv);
    PRUint32 index = 0;
    rv = sheet->InsertRule(NS_LITERAL_STRING("noscript{display:none!important}"),
                           sInsertPrefSheetRulesAt, &index);
  }

  return rv;
}

nsresult PresShell::SetPrefNoFramesRule(void)
{
  NS_ASSERTION(mPresContext,"null prescontext not allowed");
  if (!mPresContext) {
    return NS_ERROR_FAILURE;
  }

  nsresult rv = NS_OK;
  
  if (!mPrefStyleSheet) {
    rv = CreatePreferenceStyleSheet();
    NS_ENSURE_SUCCESS(rv, rv);
  }
  
  NS_ASSERTION(mPrefStyleSheet, "prefstylesheet should not be null");
  
  // get the DOM interface to the stylesheet
  nsCOMPtr<nsIDOMCSSStyleSheet> sheet(do_QueryInterface(mPrefStyleSheet, &rv));
  NS_ENSURE_SUCCESS(rv, rv);

  PRBool allowSubframes = PR_TRUE;
  nsCOMPtr<nsISupports> container = mPresContext->GetContainer();     
  nsCOMPtr<nsIDocShell> docShell(do_QueryInterface(container));
  if (docShell) {
    docShell->GetAllowSubframes(&allowSubframes);
  }
  if (!allowSubframes) {
    PRUint32 index = 0;
    rv = sheet->InsertRule(NS_LITERAL_STRING("noframes{display:block}"),
                           sInsertPrefSheetRulesAt, &index);
    NS_ENSURE_SUCCESS(rv, rv);
    rv = sheet->InsertRule(NS_LITERAL_STRING("frame, frameset, iframe {display:none!important}"),
                           sInsertPrefSheetRulesAt, &index);
  }
  return rv;
}
  
nsresult PresShell::SetPrefLinkRules(void)
{
  NS_ASSERTION(mPresContext,"null prescontext not allowed");
  if (!mPresContext) {
    return NS_ERROR_FAILURE;
  }

  nsresult rv = NS_OK;
  
  if (!mPrefStyleSheet) {
    rv = CreatePreferenceStyleSheet();
    NS_ENSURE_SUCCESS(rv, rv);
  }
  
  NS_ASSERTION(mPrefStyleSheet, "prefstylesheet should not be null");
  
  // get the DOM interface to the stylesheet
  nsCOMPtr<nsIDOMCSSStyleSheet> sheet(do_QueryInterface(mPrefStyleSheet, &rv));
  NS_ENSURE_SUCCESS(rv, rv);
  
  // support default link colors: 
  //   this means the link colors need to be overridable, 
  //   which they are if we put them in the agent stylesheet,
  //   though if using an override sheet this will cause authors grief still
  //   In the agent stylesheet, they are !important when we are ignoring document colors
  
  nscolor linkColor(mPresContext->DefaultLinkColor());
  nscolor activeColor(mPresContext->DefaultActiveLinkColor());
  nscolor visitedColor(mPresContext->DefaultVisitedLinkColor());
  
  PRBool useDocColors =
    mPresContext->GetCachedBoolPref(kPresContext_UseDocumentColors);
  NS_NAMED_LITERAL_STRING(notImportantStr, "}");
  NS_NAMED_LITERAL_STRING(importantStr, "!important}");
  const nsAString& ruleClose = useDocColors ? notImportantStr : importantStr;
  PRUint32 index = 0;
  nsAutoString strColor;

  // insert a rule to color links: '*|*:link {color: #RRGGBB [!important];}'
  ColorToString(linkColor, strColor);
  rv = sheet->InsertRule(NS_LITERAL_STRING("*|*:link{color:") +
                         strColor + ruleClose,
                         sInsertPrefSheetRulesAt, &index);
  NS_ENSURE_SUCCESS(rv, rv);

  // - visited links: '*|*:visited {color: #RRGGBB [!important];}'
  ColorToString(visitedColor, strColor);
  rv = sheet->InsertRule(NS_LITERAL_STRING("*|*:visited{color:") +
                         strColor + ruleClose,
                         sInsertPrefSheetRulesAt, &index);
  NS_ENSURE_SUCCESS(rv, rv);

  // - active links: '*|*:-moz-any-link:active {color: #RRGGBB [!important];}'
  ColorToString(activeColor, strColor);
  rv = sheet->InsertRule(NS_LITERAL_STRING("*|*:-moz-any-link:active{color:") +
                         strColor + ruleClose,
                         sInsertPrefSheetRulesAt, &index);
  NS_ENSURE_SUCCESS(rv, rv);

  PRBool underlineLinks =
    mPresContext->GetCachedBoolPref(kPresContext_UnderlineLinks);

  if (underlineLinks) {
    // create a rule to make underlining happen
    //  '*|*:-moz-any-link {text-decoration:[underline|none];}'
    // no need for important, we want these to be overridable
    // NOTE: these must go in the agent stylesheet or they cannot be
    //       overridden by authors
    rv = sheet->InsertRule(NS_LITERAL_STRING("*|*:-moz-any-link{text-decoration:underline}"),
                           sInsertPrefSheetRulesAt, &index);
  } else {
    rv = sheet->InsertRule(NS_LITERAL_STRING("*|*:-moz-any-link{text-decoration:none}"),
                           sInsertPrefSheetRulesAt, &index);
  }

  return rv;          
}

nsresult PresShell::SetPrefFocusRules(void)
{
  NS_ASSERTION(mPresContext,"null prescontext not allowed");
  nsresult result = NS_OK;

  if (!mPresContext)
    result = NS_ERROR_FAILURE;

  if (NS_SUCCEEDED(result) && !mPrefStyleSheet)
    result = CreatePreferenceStyleSheet();

  if (NS_SUCCEEDED(result)) {
    NS_ASSERTION(mPrefStyleSheet, "prefstylesheet should not be null");

    // get the DOM interface to the stylesheet
    nsCOMPtr<nsIDOMCSSStyleSheet> sheet(do_QueryInterface(mPrefStyleSheet,&result));
    if (NS_SUCCEEDED(result)) {
      if (mPresContext->GetUseFocusColors()) {
        nscolor focusBackground(mPresContext->FocusBackgroundColor());
        nscolor focusText(mPresContext->FocusTextColor());

        // insert a rule to make focus the preferred color
        PRUint32 index = 0;
        nsAutoString strRule, strColor;

        ///////////////////////////////////////////////////////////////
        // - focus: '*:focus
        ColorToString(focusText,strColor);
        strRule.AppendLiteral("*:focus,*:focus>font {color: ");
        strRule.Append(strColor);
        strRule.AppendLiteral(" !important; background-color: ");
        ColorToString(focusBackground,strColor);
        strRule.Append(strColor);
        strRule.AppendLiteral(" !important; } ");
        // insert the rules
        result = sheet->InsertRule(strRule, sInsertPrefSheetRulesAt, &index);
      }
      PRUint8 focusRingWidth = mPresContext->FocusRingWidth();
      PRBool focusRingOnAnything = mPresContext->GetFocusRingOnAnything();

      if ((NS_SUCCEEDED(result) && focusRingWidth != 1 && focusRingWidth <= 4 ) || focusRingOnAnything) {
        PRUint32 index = 0;
        nsAutoString strRule;
        if (!focusRingOnAnything)
          strRule.AppendLiteral("*|*:link:focus, *|*:visited");    // If we only want focus rings on the normal things like links
        strRule.AppendLiteral(":focus {outline: ");     // For example 3px dotted WindowText (maximum 4)
        strRule.AppendInt(focusRingWidth);
        strRule.AppendLiteral("px dotted WindowText !important; } ");     // For example 3px dotted WindowText
        // insert the rules
        result = sheet->InsertRule(strRule, sInsertPrefSheetRulesAt, &index);
        NS_ENSURE_SUCCESS(result, result);
        if (focusRingWidth != 1) {
          // If the focus ring width is different from the default, fix buttons with rings
          strRule.AssignLiteral("button::-moz-focus-inner, input[type=\"reset\"]::-moz-focus-inner,");
          strRule.AppendLiteral("input[type=\"button\"]::-moz-focus-inner, ");
          strRule.AppendLiteral("input[type=\"submit\"]::-moz-focus-inner { padding: 1px 2px 1px 2px; border: ");
          strRule.AppendInt(focusRingWidth);
          strRule.AppendLiteral("px dotted transparent !important; } ");
          result = sheet->InsertRule(strRule, sInsertPrefSheetRulesAt, &index);
          NS_ENSURE_SUCCESS(result, result);
          
          strRule.AssignLiteral("button:focus::-moz-focus-inner, input[type=\"reset\"]:focus::-moz-focus-inner,");
          strRule.AppendLiteral("input[type=\"button\"]:focus::-moz-focus-inner, input[type=\"submit\"]:focus::-moz-focus-inner {");
          strRule.AppendLiteral("border-color: ButtonText !important; }");
          result = sheet->InsertRule(strRule, sInsertPrefSheetRulesAt, &index);
        }
      }
    }
  }
  return result;
}

void
PresShell::AddUserSheet(nsISupports* aSheet)
{
  // Make sure this does what DocumentViewerImpl::CreateStyleSet does wrt
  // ordering. We want this new sheet to come after all the existing stylesheet
  // service sheets, but before other user sheets; see nsIStyleSheetService.idl
  // for the ordering.  Just remove and readd all the nsStyleSheetService
  // sheets.
  nsCOMPtr<nsIStyleSheetService> dummy =
    do_GetService(NS_STYLESHEETSERVICE_CONTRACTID);

  mStyleSet->BeginUpdate();
  
  nsStyleSheetService *sheetService = nsStyleSheetService::gInstance;
  nsCOMArray<nsIStyleSheet> & userSheets = *sheetService->UserStyleSheets();
  PRInt32 i;
  // Iterate forwards when removing so the searches for RemoveStyleSheet are as
  // short as possible.
  for (i = 0; i < userSheets.Count(); ++i) {
    mStyleSet->RemoveStyleSheet(nsStyleSet::eUserSheet, userSheets[i]);
  }

  // Now iterate backwards, so that the order of userSheets will be the same as
  // the order of sheets from it in the style set.
  for (i = userSheets.Count() - 1; i >= 0; --i) {
    mStyleSet->PrependStyleSheet(nsStyleSet::eUserSheet, userSheets[i]);
  }

  mStyleSet->EndUpdate();

  ReconstructStyleData();
}

void
PresShell::AddAgentSheet(nsISupports* aSheet)
{
  // Make sure this does what DocumentViewerImpl::CreateStyleSet does
  // wrt ordering.
  nsCOMPtr<nsIStyleSheet> sheet = do_QueryInterface(aSheet);
  if (!sheet) {
    return;
  }

  mStyleSet->AppendStyleSheet(nsStyleSet::eAgentSheet, sheet);
  ReconstructStyleData();
}

void
PresShell::RemoveSheet(nsStyleSet::sheetType aType, nsISupports* aSheet)
{
  nsCOMPtr<nsIStyleSheet> sheet = do_QueryInterface(aSheet);
  if (!sheet) {
    return;
  }

  mStyleSet->RemoveStyleSheet(aType, sheet);
  ReconstructStyleData();
}

NS_IMETHODIMP
PresShell::SetDisplaySelection(PRInt16 aToggle)
{
  mSelection->SetDisplaySelection(aToggle);
  return NS_OK;
}

NS_IMETHODIMP
PresShell::GetDisplaySelection(PRInt16 *aToggle)
{
  *aToggle = mSelection->GetDisplaySelection();
  return NS_OK;
}

NS_IMETHODIMP
PresShell::GetSelection(SelectionType aType, nsISelection **aSelection)
{
  if (!aSelection || !mSelection)
    return NS_ERROR_NULL_POINTER;

  *aSelection = mSelection->GetSelection(aType);

  if (!(*aSelection))
    return NS_ERROR_INVALID_ARG;

  NS_ADDREF(*aSelection);

  return NS_OK;
}

nsISelection*
PresShell::GetCurrentSelection(SelectionType aType)
{
  if (!mSelection)
    return nsnull;

  return mSelection->GetSelection(aType);
}

NS_IMETHODIMP
PresShell::ScrollSelectionIntoView(SelectionType aType, SelectionRegion aRegion, PRBool aIsSynchronous)
{
  if (!mSelection)
    return NS_ERROR_NULL_POINTER;

  return mSelection->ScrollSelectionIntoView(aType, aRegion, aIsSynchronous);
}

NS_IMETHODIMP
PresShell::RepaintSelection(SelectionType aType)
{
  if (!mSelection)
    return NS_ERROR_NULL_POINTER;

  return mSelection->RepaintSelection(aType);
}

// Make shell be a document observer
NS_IMETHODIMP
PresShell::BeginObservingDocument()
{
  if (mDocument) {
    mDocument->AddObserver(this);
    if (mIsDocumentGone) {
      NS_WARNING("Adding a presshell that was disconnected from the document "
                 "as a document observer?  Sounds wrong...");
      mIsDocumentGone = PR_FALSE;
    }
  }
  return NS_OK;
}

// Make shell stop being a document observer
NS_IMETHODIMP
PresShell::EndObservingDocument()
{
  // XXXbz do we need to tell the frame constructor that the document
  // is gone, perhaps?  Except for printing it's NOT gone, sometimes.
  mIsDocumentGone = PR_TRUE;
  if (mDocument) {
    mDocument->RemoveObserver(this);
  }
  return NS_OK;
}

#ifdef DEBUG_kipp
char* nsPresShell_ReflowStackPointerTop;
#endif

static void CheckForFocus(nsPIDOMWindow* aOurWindow,
                          nsIFocusController* aFocusController,
                          nsIDocument* aDocument)
{
  NS_ASSERTION(aOurWindow->IsOuterWindow(),
               "Uh, our window has to be an outer window!");

  // Now that we have a root frame, we can set focus on the presshell.
  // We do this only if our DOM window is currently focused or is an
  // an ancestor of a previously focused window.

  if (!aFocusController)
    return;

  nsCOMPtr<nsIDOMWindowInternal> ourWin = do_QueryInterface(aOurWindow);

  nsCOMPtr<nsIDOMWindowInternal> focusedWindow;
  aFocusController->GetFocusedWindow(getter_AddRefs(focusedWindow));
  if (!focusedWindow) {
    // This should never really happen, but if it does, assume
    // we can focus ourself to keep the window from being keydead.
    focusedWindow = ourWin;
  }

  // Walk up the document chain, starting with focusedWindow's document.
  // We stop walking when we find a document that has a null DOMWindow
  // (meaning that the DOMWindow has a new document now) or find ourWin
  // as the document's window.  We also stop if we hit aDocument, since
  // that means there is a child document which loaded before us that's
  // already been given focus.

  nsCOMPtr<nsIDOMDocument> focusedDOMDoc;
  focusedWindow->GetDocument(getter_AddRefs(focusedDOMDoc));

  nsCOMPtr<nsIDocument> curDoc = do_QueryInterface(focusedDOMDoc);
  if (!curDoc) {
    // This can happen if the previously focused DOM window has been
    // unhooked from its document during document teardown.  We don't
    // really have any other information to help us determine where
    // focusedWindow fits into the DOM window hierarchy.  For now, we'll
    // go ahead and allow this window to take focus, so that something
    // ends up focused.

    curDoc = aDocument;
  }

  while (curDoc) {
    nsPIDOMWindow *curWin = curDoc->GetWindow();

    if (!curWin || curWin == ourWin)
      break;

    curDoc = curDoc->GetParentDocument();
    if (curDoc == aDocument)
      return;
  }

  if (!curDoc) {
    // We reached the top of the document chain, and did not encounter ourWin
    // or a windowless document. So, focus should be unaffected by this
    // document load.
    return;
  }

  PRBool active;
  aFocusController->GetActive(&active);
  if (active)
    ourWin->Focus();

  // We need to ensure that the focus controller is updated, since it may be
  // suppressed when this function is called.
  aFocusController->SetFocusedWindow(ourWin);
}

NS_IMETHODIMP
PresShell::GetDidInitialReflow(PRBool *aDidInitialReflow)
{
  if (!aDidInitialReflow)
    return NS_ERROR_FAILURE;

  *aDidInitialReflow = mDidInitialReflow;

  return NS_OK;
}

NS_IMETHODIMP
PresShell::InitialReflow(nscoord aWidth, nscoord aHeight)
{
  nsCOMPtr<nsIPresShell> kungFuDeathGrip(this);
  mDidInitialReflow = PR_TRUE;

#ifdef NS_DEBUG
  if (VERIFY_REFLOW_NOISY_RC & gVerifyReflowFlags) {
    if (mDocument) {
      nsIURI *uri = mDocument->GetDocumentURI();
      if (uri) {
        nsCAutoString url;
        uri->GetSpec(url);
        printf("*** PresShell::InitialReflow (this=%p, url='%s')\n", (void*)this, url.get());
      }
    }
  }
#endif

  if (mCaret)
    mCaret->EraseCaret();

  NS_ASSERTION(mViewManager, "Should have view manager");
  // Painting should be suppressed for the initial reflow, so this won't
  // really do anything right now, but it will be useful when we
  // start batching widget changes
  mViewManager->BeginUpdateViewBatch();

  WillCauseReflow();
  WillDoReflow();

  if (mPresContext) {
    nsRect r(0, 0, aWidth, aHeight);
    mPresContext->SetVisibleArea(r);
  }

  nsIContent *root = mDocument ? mDocument->GetRootContent() : nsnull;

  // Get the root frame from the frame manager
  nsIFrame* rootFrame = FrameManager()->GetRootFrame();
  
  if (root) {
    MOZ_TIMER_DEBUGLOG(("Reset and start: Frame Creation: PresShell::InitialReflow(), this=%p\n",
                        (void*)this));
    MOZ_TIMER_RESET(mFrameCreationWatch);
    MOZ_TIMER_START(mFrameCreationWatch);

    if (!rootFrame) {
      // Have style sheet processor construct a frame for the
      // precursors to the root content object's frame
      mFrameConstructor->ConstructRootFrame(root, &rootFrame);
      FrameManager()->SetRootFrame(rootFrame);
    }

    // Have the style sheet processor construct frame for the root
    // content object down
    mFrameConstructor->ContentInserted(nsnull, root, 0,
                                       nsnull, PR_FALSE);
    VERIFY_STYLE_TREE;
    MOZ_TIMER_DEBUGLOG(("Stop: Frame Creation: PresShell::InitialReflow(), this=%p\n",
                        (void*)this));
    MOZ_TIMER_STOP(mFrameCreationWatch);

    // Something in mFrameConstructor->ContentInserted may have caused
    // Destroy() to get called, bug 337586.
    NS_ENSURE_STATE(!mHaveShutDown);
  }

  if (rootFrame) {
    MOZ_TIMER_DEBUGLOG(("Reset and start: Reflow: PresShell::InitialReflow(), this=%p\n",
                        (void*)this));
    MOZ_TIMER_RESET(mReflowWatch);
    MOZ_TIMER_START(mReflowWatch);
    // Kick off a top-down reflow
    NS_FRAME_LOG(NS_FRAME_TRACE_CALLS,
                 ("enter nsPresShell::InitialReflow: %d,%d", aWidth, aHeight));
#ifdef NS_DEBUG
    if (nsIFrameDebug::GetVerifyTreeEnable()) {
      nsIFrameDebug*  frameDebug;

      if (NS_SUCCEEDED(rootFrame->QueryInterface(NS_GET_IID(nsIFrameDebug),
                                                (void**)&frameDebug))) {
        frameDebug->VerifyTree();
      }
    }
#endif
#ifdef DEBUG_kipp
    nsPresShell_ReflowStackPointerTop = (char*) &aWidth;
#endif
    nsRect                bounds = mPresContext->GetVisibleArea();
    nsSize                maxSize(bounds.width, bounds.height);
    nsHTMLReflowMetrics   desiredSize(nsnull);
    nsReflowStatus        status;
    nsIRenderingContext*  rcx = nsnull;

    nsresult rv=CreateRenderingContext(rootFrame, &rcx);
    if (NS_FAILED(rv)) return rv;

    AUTO_LAYOUT_PHASE_ENTRY_POINT(GetPresContext(), Reflow);
    mIsReflowing = PR_TRUE;

    nsHTMLReflowState reflowState(mPresContext, rootFrame,
                                  eReflowReason_Initial, rcx, maxSize);
    rootFrame->WillReflow(mPresContext);
    nsContainerFrame::PositionFrameView(rootFrame);
    rootFrame->Reflow(mPresContext, desiredSize, reflowState, status);
    rootFrame->SetSize(nsSize(desiredSize.width, desiredSize.height));
    mPresContext->SetVisibleArea(nsRect(0,0,desiredSize.width,desiredSize.height));

    nsContainerFrame::SyncFrameViewAfterReflow(mPresContext, rootFrame, rootFrame->GetView(),
                                               nsnull);
    rootFrame->DidReflow(mPresContext, nsnull, NS_FRAME_REFLOW_FINISHED);
      
#ifdef NS_DEBUG
    if (nsIFrameDebug::GetVerifyTreeEnable()) {
      nsIFrameDebug*  frameDebug;

      if (NS_SUCCEEDED(rootFrame->QueryInterface(NS_GET_IID(nsIFrameDebug),
                                                 (void**)&frameDebug))) {
        frameDebug->VerifyTree();
      }
    }
#endif
    VERIFY_STYLE_TREE;
    NS_IF_RELEASE(rcx);
    NS_FRAME_LOG(NS_FRAME_TRACE_CALLS, ("exit nsPresShell::InitialReflow"));
    MOZ_TIMER_DEBUGLOG(("Stop: Reflow: PresShell::InitialReflow(), this=%p\n", (void*)this));
    MOZ_TIMER_STOP(mReflowWatch);

    mIsReflowing = PR_FALSE;
  }

  DidCauseReflow();
  DidDoReflow();

  mViewManager->EndUpdateViewBatch(NS_VMREFRESH_NO_SYNC);

  if (mViewManager && mCaret && !mViewEventListener) {
    nsIScrollableView* scrollingView = nsnull;
    mViewManager->GetRootScrollableView(&scrollingView);

    if (scrollingView) {
      mViewEventListener = new PresShellViewEventListener;

      if (!mViewEventListener)
        return NS_ERROR_OUT_OF_MEMORY;

      NS_ADDREF(mViewEventListener);
      mViewEventListener->SetPresShell(this);
      scrollingView->AddScrollPositionListener((nsIScrollPositionListener *)mViewEventListener);
      mViewManager->AddCompositeListener((nsICompositeListener *)mViewEventListener);
    }
  }

  // For printing, we just immediately unsuppress.
  if (!mPresContext->IsPaginated()) {
    // Kick off a one-shot timer based off our pref value.  When this timer
    // fires, if painting is still locked down, then we will go ahead and
    // trigger a full invalidate and allow painting to proceed normally.
    mPaintingSuppressed = PR_TRUE;
    mPaintSuppressionTimer = do_CreateInstance("@mozilla.org/timer;1");
    if (!mPaintSuppressionTimer)
      // Uh-oh.  We must be out of memory.  No point in keeping painting locked down.
      mPaintingSuppressed = PR_FALSE;
    else {
      // Initialize the timer.

      // Default to PAINTLOCK_EVENT_DELAY if we can't get the pref value.
      PRInt32 delay =
        nsContentUtils::GetIntPref("nglayout.initialpaint.delay",
                                   PAINTLOCK_EVENT_DELAY);

      mPaintSuppressionTimer->InitWithFuncCallback(sPaintSuppressionCallback,
                                                   this, delay, 
                                                   nsITimer::TYPE_ONE_SHOT);
    }
  }

  return NS_OK; //XXX this needs to be real. MMP
}

void
PresShell::sPaintSuppressionCallback(nsITimer *aTimer, void* aPresShell)
{
  PresShell* self = NS_STATIC_CAST(PresShell*, aPresShell);
  if (self)
    self->UnsuppressPainting();
}

NS_IMETHODIMP
PresShell::ResizeReflow(nscoord aWidth, nscoord aHeight)
{
  PRBool firstReflow = PR_FALSE;

  NS_ASSERTION(mViewManager, "Must have view manager");
  mViewManager->BeginUpdateViewBatch();

  WillCauseReflow();
  WillDoReflow();

  // If we don't have a root frame yet, that means we haven't had our initial
  // reflow... If that's the case, and aWidth or aHeight is unconstrained,
  // ignore them altogether.
  nsIFrame* rootFrame = FrameManager()->GetRootFrame();

  if (!rootFrame &&
      (aWidth == NS_UNCONSTRAINEDSIZE || aHeight == NS_UNCONSTRAINEDSIZE)) {
    return NS_ERROR_NOT_AVAILABLE;
  }
  
  if (mPresContext) {
    nsRect r(0, 0, aWidth, aHeight);
    mPresContext->SetVisibleArea(r);
  }

  if (rootFrame) {
    // Kick off a top-down reflow
    NS_FRAME_LOG(NS_FRAME_TRACE_CALLS,
                 ("enter nsPresShell::ResizeReflow: %d,%d", aWidth, aHeight));
#ifdef NS_DEBUG
    if (nsIFrameDebug::GetVerifyTreeEnable()) {
      nsIFrameDebug*  frameDebug;

      if (NS_SUCCEEDED(rootFrame->QueryInterface(NS_GET_IID(nsIFrameDebug),
                                                 (void**)&frameDebug))) {
        frameDebug->VerifyTree();
      }
    }
#endif
#ifdef DEBUG_kipp
    nsPresShell_ReflowStackPointerTop = (char*) &aWidth;
#endif
    nsRect                bounds = mPresContext->GetVisibleArea();
    nsSize                maxSize(bounds.width, bounds.height);
    nsHTMLReflowMetrics   desiredSize(nsnull);
    nsReflowStatus        status;
    nsIRenderingContext*  rcx = nsnull;

    nsresult rv=CreateRenderingContext(rootFrame, &rcx);
    if (NS_FAILED(rv)) return rv;

    AUTO_LAYOUT_PHASE_ENTRY_POINT(GetPresContext(), Reflow);
    // XXXldb Set mIsReflowing (and unset it later)?

    nsHTMLReflowState reflowState(mPresContext, rootFrame,
                                  eReflowReason_Resize, rcx, maxSize);

    rootFrame->WillReflow(mPresContext);
    nsContainerFrame::PositionFrameView(rootFrame);
    rootFrame->Reflow(mPresContext, desiredSize, reflowState, status);
    rootFrame->SetSize(nsSize(desiredSize.width, desiredSize.height));
    mPresContext->SetVisibleArea(nsRect(0,0,desiredSize.width,desiredSize.height));

    nsContainerFrame::SyncFrameViewAfterReflow(mPresContext, rootFrame, rootFrame->GetView(),
                                               nsnull);
    rootFrame->DidReflow(mPresContext, nsnull, NS_FRAME_REFLOW_FINISHED);
#ifdef NS_DEBUG
    if (nsIFrameDebug::GetVerifyTreeEnable()) {
      nsIFrameDebug*  frameDebug;

      if (NS_SUCCEEDED(rootFrame->QueryInterface(NS_GET_IID(nsIFrameDebug),
                                                 (void**)&frameDebug))) {
        frameDebug->VerifyTree();
      }
    }
#endif
    VERIFY_STYLE_TREE;
    NS_IF_RELEASE(rcx);
    NS_FRAME_LOG(NS_FRAME_TRACE_CALLS, ("exit nsPresShell::ResizeReflow"));

    // XXX if debugging then we should assert that the cache is empty
  } else {
    firstReflow = PR_TRUE;
#ifdef NOISY
    printf("PresShell::ResizeReflow: null root frame\n");
#endif
  }
  DidCauseReflow();
  // if the proper flag is set, VerifyReflow now
#ifdef NS_DEBUG
  if (GetVerifyReflowEnable() && (VERIFY_REFLOW_DURING_RESIZE_REFLOW & gVerifyReflowFlags))
  {
    mInVerifyReflow = PR_TRUE;
    /*PRBool ok = */VerifyIncrementalReflow();
    mInVerifyReflow = PR_FALSE;
  }
#endif
  
  DidDoReflow();

  mViewManager->EndUpdateViewBatch(NS_VMREFRESH_NO_SYNC);

  if (!firstReflow) {
    //Set resize event timer
    CreateResizeEventTimer();
  }
  
  return NS_OK; //XXX this needs to be real. MMP
}

#define RESIZE_EVENT_DELAY 200

void
PresShell::CreateResizeEventTimer ()
{
  KillResizeEventTimer();

  if (mIsDocumentGone)
    return;

  mResizeEventTimer = do_CreateInstance("@mozilla.org/timer;1");
  if (mResizeEventTimer) {
    mResizeEventTimer->InitWithFuncCallback(sResizeEventCallback, this, RESIZE_EVENT_DELAY, 
                                            nsITimer::TYPE_ONE_SHOT);
  }
}

void
PresShell::KillResizeEventTimer()
{
  if(mResizeEventTimer) {
    mResizeEventTimer->Cancel();
    mResizeEventTimer = nsnull;
  }
}

void
PresShell::sResizeEventCallback(nsITimer *aTimer, void* aPresShell)
{
  PresShell* self = NS_STATIC_CAST(PresShell*, aPresShell);
  if (self) {
    self->FireResizeEvent();  
  }
}

void
PresShell::FireResizeEvent()
{
  if (mIsDocumentGone)
    return;

  //Send resize event from here.
  nsEvent event(PR_TRUE, NS_RESIZE_EVENT);
  nsEventStatus status = nsEventStatus_eIgnore;

  nsPIDOMWindow *window = mDocument->GetWindow();
  if (window) {
    nsEventDispatcher::Dispatch(window, mPresContext, &event, nsnull, &status);
  }
}

NS_IMETHODIMP
PresShell::SetIgnoreFrameDestruction(PRBool aIgnore)
{
  mIgnoreFrameDestruction = aIgnore;
  return NS_OK;
}

NS_IMETHODIMP
PresShell::NotifyDestroyingFrame(nsIFrame* aFrame)
{
  if (!mIgnoreFrameDestruction) {
    mFrameConstructor->NotifyDestroyingFrame(aFrame);

    // Cancel any pending reflow commands targeted at this frame
    CancelReflowCommandInternal(aFrame, nsnull);

    // Notify the frame manager
    FrameManager()->NotifyDestroyingFrame(aFrame);

    // Remove frame properties
    mPresContext->PropertyTable()->DeleteAllPropertiesFor(aFrame);
  }

  return NS_OK;
}

// note that this can return a null caret, but NS_OK
NS_IMETHODIMP PresShell::GetCaret(nsICaret **outCaret)
{
  NS_ENSURE_ARG_POINTER(outCaret);
  
  *outCaret = mCaret;
  NS_IF_ADDREF(*outCaret);
  return NS_OK;
}

already_AddRefed<nsICaret> PresShell::SetCaret(nsICaret *aNewCaret)
{
  nsICaret *oldCaret = nsnull;
  mCaret.swap(oldCaret);
  mCaret = aNewCaret;
  return oldCaret;
}

NS_IMETHODIMP PresShell::SetCaretEnabled(PRBool aInEnable)
{
  nsresult result = NS_OK;
  PRBool   oldEnabled = mCaretEnabled;

  mCaretEnabled = aInEnable;

  if (mCaret && (mCaretEnabled != oldEnabled))
  {
/*  Don't change the caret's selection here! This was an evil side-effect of SetCaretEnabled()
    nsCOMPtr<nsIDOMSelection> domSel;
    if (NS_SUCCEEDED(GetSelection(nsISelectionController::SELECTION_NORMAL, getter_AddRefs(domSel))) && domSel)
      mCaret->SetCaretDOMSelection(domSel);
*/
    result = mCaret->SetCaretVisible(mCaretEnabled);
  }

  return result;
}

NS_IMETHODIMP PresShell::SetCaretReadOnly(PRBool aReadOnly)
{
  if (mCaret)
    mCaret->SetCaretReadOnly(aReadOnly);
  return NS_OK;
}

NS_IMETHODIMP PresShell::GetCaretEnabled(PRBool *aOutEnabled)
{
  NS_ENSURE_ARG_POINTER(aOutEnabled);
  *aOutEnabled = mCaretEnabled;
  return NS_OK;
}

NS_IMETHODIMP PresShell::SetCaretVisibilityDuringSelection(PRBool aVisibility)
{
  if (mCaret)
    mCaret->SetVisibilityDuringSelection(aVisibility);
  return NS_OK;
}

NS_IMETHODIMP PresShell::SetSelectionFlags(PRInt16 aInEnable)
{
  mSelectionFlags = aInEnable;
  return NS_OK;
}

NS_IMETHODIMP PresShell::GetSelectionFlags(PRInt16 *aOutEnable)
{
  if (!aOutEnable)
    return NS_ERROR_INVALID_ARG;
  *aOutEnable = mSelectionFlags;
  return NS_OK;
}

//implementation of nsISelectionController

NS_IMETHODIMP 
PresShell::CharacterMove(PRBool aForward, PRBool aExtend)
{
  return mSelection->CharacterMove(aForward, aExtend);  
}

NS_IMETHODIMP 
PresShell::WordMove(PRBool aForward, PRBool aExtend)
{
  return mSelection->WordMove(aForward, aExtend);  
}

NS_IMETHODIMP 
PresShell::LineMove(PRBool aForward, PRBool aExtend)
{
  nsresult result = mSelection->LineMove(aForward, aExtend);  
// if we can't go down/up any more we must then move caret completely to 
// end/beginning respectively.
  if (NS_FAILED(result)) 
    result = CompleteMove(aForward,aExtend);
  return result;
}

NS_IMETHODIMP 
PresShell::IntraLineMove(PRBool aForward, PRBool aExtend)
{
  return mSelection->IntraLineMove(aForward, aExtend);  
}



NS_IMETHODIMP 
PresShell::PageMove(PRBool aForward, PRBool aExtend)
{
  nsresult result;
  nsIViewManager* viewManager = GetViewManager();
  nsIScrollableView *scrollableView;
  if (!viewManager) 
    return NS_ERROR_UNEXPECTED;
  result = viewManager->GetRootScrollableView(&scrollableView);
  if (NS_FAILED(result)) 
    return result;
  if (!scrollableView) 
    return NS_ERROR_UNEXPECTED;
  nsIView *scrolledView;
  result = scrollableView->GetScrolledView(scrolledView);
  mSelection->CommonPageMove(aForward, aExtend, scrollableView);
  // do ScrollSelectionIntoView()
  return ScrollSelectionIntoView(nsISelectionController::SELECTION_NORMAL, nsISelectionController::SELECTION_FOCUS_REGION, PR_TRUE);
}



NS_IMETHODIMP 
PresShell::ScrollPage(PRBool aForward)
{
  nsIScrollableView* scrollView = GetViewToScroll(nsLayoutUtils::eVertical);
  if (scrollView) {
    scrollView->ScrollByPages(0, aForward ? 1 : -1);
  }
  return NS_OK;
}

NS_IMETHODIMP
PresShell::ScrollLine(PRBool aForward)
{
  nsIScrollableView* scrollView = GetViewToScroll(nsLayoutUtils::eVertical);
  if (scrollView) {
#ifdef MOZ_WIDGET_COCOA
    // Emulate the Mac IE behavior of scrolling a minimum of 2 lines
    // rather than 1.  This vastly improves scrolling speed.
    scrollView->ScrollByLines(0, aForward ? 2 : -2);
#else
    scrollView->ScrollByLines(0, aForward ? 1 : -1);
#endif
      
//NEW FOR LINES    
    // force the update to happen now, otherwise multiple scrolls can
    // occur before the update is processed. (bug #7354)

  // I'd use Composite here, but it doesn't always work.
    // vm->Composite();
    nsIViewManager* viewManager = GetViewManager();
    if (viewManager) {
      viewManager->ForceUpdate();
    }
  }
  return NS_OK;
}

NS_IMETHODIMP
PresShell::ScrollHorizontal(PRBool aLeft)
{
  nsIScrollableView* scrollView = GetViewToScroll(nsLayoutUtils::eHorizontal);
  if (scrollView) {
    scrollView->ScrollByLines(aLeft ? -1 : 1, 0);
//NEW FOR LINES    
    // force the update to happen now, otherwise multiple scrolls can
    // occur before the update is processed. (bug #7354)

  // I'd use Composite here, but it doesn't always work.
    // vm->Composite();
    nsIViewManager* viewManager = GetViewManager();
    if (viewManager) {
      viewManager->ForceUpdate();
    }
  }
  return NS_OK;
}

NS_IMETHODIMP
PresShell::CompleteScroll(PRBool aForward)
{
  nsIScrollableView* scrollView = GetViewToScroll(nsLayoutUtils::eVertical);
  if (scrollView) {
    scrollView->ScrollByWhole(!aForward);//TRUE = top, aForward TRUE=bottom
  }
  return NS_OK;
}

NS_IMETHODIMP
PresShell::CompleteMove(PRBool aForward, PRBool aExtend)
{
  nsIScrollableView *scrollableView;
  if (!mViewManager) 
    return NS_ERROR_UNEXPECTED;
  nsresult result = mViewManager->GetRootScrollableView(&scrollableView);
  if (NS_FAILED(result)) 
    return result;
  if (!scrollableView) 
    return NS_ERROR_UNEXPECTED;
  nsIView *scrolledView;
  result = scrollableView->GetScrolledView(scrolledView);
  // get a frame
  nsIFrame *frame = (nsIFrame*)scrolledView->GetClientData();
  if (!frame)
    return NS_ERROR_FAILURE;
  //we need to get to the area frame.
  nsIAtom* frameType;
  do 
  {
    frameType = frame->GetType();
    if (frameType != nsLayoutAtoms::areaFrame)
    {
      frame = frame->GetFirstChild(nsnull);
      if (!frame)
        break;
    }
  }while(frameType != nsLayoutAtoms::areaFrame);
  
  if (!frame)
    return NS_ERROR_FAILURE; //could not find an area frame.

  nsPeekOffsetStruct pos = frame->GetExtremeCaretPosition(!aForward);

  mSelection->HandleClick(pos.mResultContent ,pos.mContentOffset ,pos.mContentOffset/*End*/ ,aExtend, PR_FALSE, aForward);
  return ScrollSelectionIntoView(nsISelectionController::SELECTION_NORMAL, nsISelectionController::SELECTION_FOCUS_REGION, PR_TRUE);
}

NS_IMETHODIMP 
PresShell::SelectAll()
{
  return mSelection->SelectAll();
}

NS_IMETHODIMP
PresShell::CheckVisibility(nsIDOMNode *node, PRInt16 startOffset, PRInt16 EndOffset, PRBool *_retval)
{
  if (!node || startOffset>EndOffset || !_retval || startOffset<0 || EndOffset<0)
    return NS_ERROR_INVALID_ARG;
  *_retval = PR_FALSE; //initialize return parameter
  nsCOMPtr<nsIContent> content(do_QueryInterface(node));
  if (!content)
    return NS_ERROR_FAILURE;
  nsIFrame *frame = GetPrimaryFrameFor(content);
  if (!frame) //no frame to look at so it must not be visible
    return NS_OK;  
  //start process now to go through all frames to find startOffset. then check chars after that to see 
  //if anything until EndOffset is visible.
  PRBool finished = PR_FALSE;
  frame->CheckVisibility(mPresContext,startOffset,EndOffset,PR_TRUE,&finished, _retval);
  return NS_OK;//dont worry about other return val
}

//end implementations nsISelectionController


static void UpdateViewProperties(nsPresContext* aPresContext, nsIViewManager* aVM,
                                 nsIView* aView) {
  nsIViewManager* thisVM = aView->GetViewManager();
  if (thisVM != aVM) {
    return;
  }

  nsIFrame* frame = NS_STATIC_CAST(nsIFrame*, aView->GetClientData());
  if (frame) {
    nsContainerFrame::SyncFrameViewProperties(aPresContext, frame, nsnull, aView);
  }

  for (nsIView* child = aView->GetFirstChild(); child;
       child = child->GetNextSibling()) {
    UpdateViewProperties(aPresContext, aVM, child);
  }
}

NS_IMETHODIMP
PresShell::StyleChangeReflow()
{
  WillCauseReflow();
  WillDoReflow();

  nsIFrame* rootFrame = FrameManager()->GetRootFrame();
  if (rootFrame) {
    // Kick off a top-down reflow
    NS_FRAME_LOG(NS_FRAME_TRACE_CALLS,
                 ("enter nsPresShell::StyleChangeReflow"));
#ifdef NS_DEBUG
    if (nsIFrameDebug::GetVerifyTreeEnable()) {
      nsIFrameDebug*  frameDebug;

      if (NS_SUCCEEDED(rootFrame->QueryInterface(NS_GET_IID(nsIFrameDebug),
                                                 (void**)&frameDebug))) {
        frameDebug->VerifyTree();
      }
    }
#endif
    nsRect                bounds = mPresContext->GetVisibleArea();
    nsSize                maxSize(bounds.width, bounds.height);
    nsHTMLReflowMetrics   desiredSize(nsnull);
    nsReflowStatus        status;
    nsIRenderingContext*  rcx = nsnull;

    nsresult rv=CreateRenderingContext(rootFrame, &rcx);
    if (NS_FAILED(rv)) return rv;

    AUTO_LAYOUT_PHASE_ENTRY_POINT(GetPresContext(), Reflow);
    // XXXldb Set mIsReflowing (and unset it later)?

    nsHTMLReflowState reflowState(mPresContext, rootFrame,
                                  eReflowReason_StyleChange, rcx, maxSize);

    rootFrame->WillReflow(mPresContext);
    nsContainerFrame::PositionFrameView(rootFrame);
    rootFrame->Reflow(mPresContext, desiredSize, reflowState, status);
    rootFrame->SetSize(nsSize(desiredSize.width, desiredSize.height));
    mPresContext->SetVisibleArea(nsRect(0,0,desiredSize.width,desiredSize.height));
    nsIView* view = rootFrame->GetView();
    nsContainerFrame::SyncFrameViewAfterReflow(mPresContext, rootFrame, view,
                                               nsnull);
    rootFrame->DidReflow(mPresContext, nsnull, NS_FRAME_REFLOW_FINISHED);
#ifdef NS_DEBUG
    if (nsIFrameDebug::GetVerifyTreeEnable()) {
      nsIFrameDebug*  frameDebug;

      if (NS_SUCCEEDED(rootFrame->QueryInterface(NS_GET_IID(nsIFrameDebug),
                                                 (void**)&frameDebug))) {
        frameDebug->VerifyTree();
      }
    }
#endif
    VERIFY_STYLE_TREE;
    NS_IF_RELEASE(rcx);
    NS_FRAME_LOG(NS_FRAME_TRACE_CALLS, ("exit nsPresShell::StyleChangeReflow"));
    
    // The following two calls are needed to make sure we reacquire any needed
    // style structs that were cleared by the caller

    // Update properties of all views to reflect style changes
    UpdateViewProperties(mPresContext, mViewManager, view);

    // Repaint everything just to be sure
    mViewManager->UpdateAllViews(NS_VMREFRESH_NO_SYNC);
  }

  DidCauseReflow();
  DidDoReflow();

  return NS_OK; //XXX this needs to be real. MMP
}

nsIFrame*
nsIPresShell::GetRootFrame() const
{
  return FrameManager()->GetRootFrame();
}

nsIFrame*
nsIPresShell::GetRootScrollFrame() const
{
  nsIFrame* rootFrame = FrameManager()->GetRootFrame();
  // Ensure root frame is a viewport frame
  if (!rootFrame || nsLayoutAtoms::viewportFrame != rootFrame->GetType())
    return nsnull;
  nsIFrame* theFrame = rootFrame->GetFirstChild(nsnull);
  if (!theFrame || nsLayoutAtoms::scrollFrame != theFrame->GetType())
    return nsnull;
  return theFrame;
}

NS_IMETHODIMP
PresShell::GetPageSequenceFrame(nsIPageSequenceFrame** aResult) const
{
  NS_PRECONDITION(nsnull != aResult, "null ptr");
  if (nsnull == aResult) {
    return NS_ERROR_NULL_POINTER;
  }

  *aResult = nsnull;
  nsIFrame* frame = mFrameConstructor->GetPageSequenceFrame();
  if (frame) {
    CallQueryInterface(frame, aResult);
  }
  return *aResult ? NS_OK : NS_ERROR_FAILURE;
}

void
PresShell::BeginUpdate(nsIDocument *aDocument, nsUpdateType aUpdateType)
{
#ifdef DEBUG
  mUpdateCount++;
#endif
  mFrameConstructor->BeginUpdate();

  if (aUpdateType & UPDATE_STYLE)
    mStyleSet->BeginUpdate();
}

void
PresShell::EndUpdate(nsIDocument *aDocument, nsUpdateType aUpdateType)
{
#ifdef DEBUG
  NS_PRECONDITION(0 != mUpdateCount, "too many EndUpdate's");
  --mUpdateCount;
#endif

  if (aUpdateType & UPDATE_STYLE) {
    mStyleSet->EndUpdate();
    if (mStylesHaveChanged)
      ReconstructStyleData();
  }

  mFrameConstructor->EndUpdate();
}

void
PresShell::BeginLoad(nsIDocument *aDocument)
{  
#ifdef MOZ_PERF_METRICS
  // Reset style resolution stopwatch maintained by style set
  MOZ_TIMER_DEBUGLOG(("Reset: Style Resolution: PresShell::BeginLoad(), this=%p\n", (void*)this));
#endif  
  mDocumentLoading = PR_TRUE;
}

void
PresShell::EndLoad(nsIDocument *aDocument)
{

  // Restore frame state for the root scroll frame
  nsCOMPtr<nsILayoutHistoryState> historyState =
    aDocument->GetLayoutHistoryState();
  // Make sure we don't reenter reflow via the sync paint that happens while
  // we're scrolling to our restored position.  Entering reflow for the
  // scrollable frame will cause it to reenter ScrollToRestoredPosition(), and
  // it'll get all confused.
  ++mChangeNestCount;

  if (historyState) {
    nsIFrame* scrollFrame = GetRootScrollFrame();
    if (scrollFrame) {
      nsIScrollableFrame* scrollableFrame;
      CallQueryInterface(scrollFrame, &scrollableFrame);
      if (scrollableFrame) {
        FrameManager()->RestoreFrameStateFor(scrollFrame, historyState,
                                             nsIStatefulFrame::eDocumentScrollState);
        scrollableFrame->ScrollToRestoredPosition();
      }
    }
  }

  --mChangeNestCount;
  
#ifdef MOZ_PERF_METRICS
  // Dump reflow, style resolution and frame construction times here.
  MOZ_TIMER_DEBUGLOG(("Stop: Reflow: PresShell::EndLoad(), this=%p\n", this));
  MOZ_TIMER_STOP(mReflowWatch);
  MOZ_TIMER_LOG(("Reflow time (this=%p): ", this));
  MOZ_TIMER_PRINT(mReflowWatch);  

  MOZ_TIMER_DEBUGLOG(("Stop: Frame Creation: PresShell::EndLoad(), this=%p\n", this));
  MOZ_TIMER_STOP(mFrameCreationWatch);
  MOZ_TIMER_LOG(("Frame construction plus style resolution time (this=%p): ", this));
  MOZ_TIMER_PRINT(mFrameCreationWatch);

  // Print style resolution stopwatch maintained by style set
  MOZ_TIMER_DEBUGLOG(("Stop: Style Resolution: PresShell::EndLoad(), this=%p\n", this));
#endif  
  mDocumentLoading = PR_FALSE;
}

// aReflowCommand is considered to be already in the queue if the
// frame it targets is targeted by a pre-existing reflow command in
// the queue.
PRBool
PresShell::AlreadyInQueue(nsHTMLReflowCommand* aReflowCommand)
{
  if (!mReflowCommandTable.ops) {
    // We're already destroyed
    NS_ERROR("We really shouldn't be posting reflow commands here");
  }

  ReflowCommandEntry* e =
    NS_STATIC_CAST(ReflowCommandEntry*,
                   PL_DHashTableOperate(&mReflowCommandTable, aReflowCommand,
                                        PL_DHASH_ADD));

  if (!e) {
    // We lie no matter what we say here
    return PR_FALSE;
  }

  // We're using the stub ClearEntry, which zeros out entries, so a
  // non-null mCommand means we're in the queue already.
  if (e->mCommand) {
#ifdef DEBUG
    if (VERIFY_REFLOW_NOISY_RC & gVerifyReflowFlags) {
      printf("*** PresShell::AlreadyInQueue(): Discarding reflow command: this=%p\n", (void*)this);
      aReflowCommand->List(stdout);
    }
#endif
    return PR_TRUE;
  }

  e->mCommand = aReflowCommand;
  return PR_FALSE;
}

NS_IMETHODIMP
PresShell::AppendReflowCommand(nsIFrame*    aTargetFrame,
                               nsReflowType aReflowType,
                               nsIAtom*     aChildListName)
{
  // If we've not yet done the initial reflow, then don't bother
  // enqueuing a reflow command yet.
  if (! mDidInitialReflow)
    return NS_OK;

  // If we're already destroying, don't bother with this either.
  if (mIsDestroying)
    return NS_OK;

#ifdef DEBUG
  //printf("gShellCounter: %d\n", gShellCounter++);
  if (mInVerifyReflow) {
    return NS_OK;
  }
#endif

  nsHTMLReflowCommand* command = new nsHTMLReflowCommand(aTargetFrame,
                                                         aReflowType,
                                                         aChildListName);
  if (!command) {
    return NS_ERROR_OUT_OF_MEMORY;
  }
  
#ifdef DEBUG
  if (VERIFY_REFLOW_NOISY_RC & gVerifyReflowFlags) {
    printf("\nPresShell@%p: adding reflow command\n", (void*)this);
    command->List(stdout);
    if (VERIFY_REFLOW_REALLY_NOISY_RC & gVerifyReflowFlags) {
      printf("Current content model:\n");
      nsIContent *rootContent = mDocument->GetRootContent();
      if (rootContent) {
        rootContent->List(stdout, 0);
      }
    }
  }  
#endif

  // Add the reflow command to the queue
  nsresult rv = NS_OK;
  if (!AlreadyInQueue(command)) {
    if (mReflowCommands.AppendElement(command)) {
      ReflowCommandAdded(command);
    } else {
      // Drop this command.... we're out of memory
      PL_DHashTableOperate(&mReflowCommandTable, command,
                           PL_DHASH_REMOVE);
      delete command;
      rv = NS_ERROR_OUT_OF_MEMORY;
    }
  }
  else {
    // We're not going to process this reflow command.
    delete command;
  }

  // Post a reflow event if we are not batching reflow commands.
  if (!mBatchReflows) {
    // If we're in the middle of a drag, process it right away (needed for mac,
    // might as well do it on all platforms just to keep the code paths the same).
    // XXXbz but how does this actually "process it right away"?
    // Isn't this more like "never process it"?
    if ( !IsDragInProgress() )
      PostReflowEvent();
  }

  return rv;
}


//
// IsDragInProgress
//
// Ask the drag service if we're in the middle of a drag
//
PRBool
PresShell :: IsDragInProgress ( ) const
{
  PRBool dragInProgress = PR_FALSE;
  if ( mDragService ) {
    nsCOMPtr<nsIDragSession> session;
    mDragService->GetCurrentSession ( getter_AddRefs(session) );
    if ( session )
      dragInProgress = PR_TRUE;
  }
  
  return dragInProgress;

} // IsDragInProgress

nsIScrollableView*
PresShell::GetViewToScroll(nsLayoutUtils::Direction aDirection)
{
  nsCOMPtr<nsIEventStateManager> esm = mPresContext->EventStateManager();
  nsIScrollableView* scrollView = nsnull;
  nsCOMPtr<nsIContent> focusedContent;
  esm->GetFocusedContent(getter_AddRefs(focusedContent));
  if (!focusedContent && mSelection) {
    nsISelection* domSelection = mSelection->
      GetSelection(nsISelectionController::SELECTION_NORMAL);
    if (domSelection) {
      nsCOMPtr<nsIDOMNode> focusedNode;
      domSelection->GetFocusNode(getter_AddRefs(focusedNode));
      focusedContent = do_QueryInterface(focusedNode);
    }
  }
  if (focusedContent) {
    nsIFrame* startFrame = GetPrimaryFrameFor(focusedContent);
    if (startFrame) {
      nsCOMPtr<nsIScrollableViewProvider> svp = do_QueryInterface(startFrame);
      // If this very frame provides a scroll view, start there instead of frame's
      // closest view, because the scroll view may be inside a child frame.
      // For example, this happens in the case of overflow:scroll.
      // In that case we still use GetNearestScrollingView() because
      // we need a scrolling view that matches aDirection.
      nsIScrollableView* sv;
      nsIView* startView = svp && (sv = svp->GetScrollableView()) ? sv->View() : startFrame->GetClosestView();
      NS_ASSERTION(startView, "No view to start searching for scrollable view from");
      scrollView = nsLayoutUtils::GetNearestScrollingView(startView, aDirection);
    }
  }
  if (!scrollView) {
    nsIViewManager* viewManager = GetViewManager();
    if (viewManager) {
      viewManager->GetRootScrollableView(&scrollView);
    }
  }
  return scrollView;
}

void
PresShell::CancelReflowCommandInternal(nsIFrame*     aTargetFrame, 
                                       nsReflowType* aCmdType)
{
  PRInt32 i, n = mReflowCommands.Count();
  for (i = 0; i < n; i++) {
    nsHTMLReflowCommand* rc = (nsHTMLReflowCommand*) mReflowCommands.ElementAt(i);
    if (rc && rc->GetTarget() == aTargetFrame &&
        (!aCmdType || rc->Type() == *aCmdType)) {
#ifdef DEBUG
      if (VERIFY_REFLOW_NOISY_RC & gVerifyReflowFlags) {
        printf("PresShell: removing rc=%p for frame ", (void*)rc);
        nsFrame::ListTag(stdout, aTargetFrame);
        printf("\n");
      }
#endif
      mReflowCommands.RemoveElementAt(i);
      ReflowCommandRemoved(rc);
      delete rc;
      n--;
      i--;
    }
  }

  DoneRemovingReflowCommands();
}

NS_IMETHODIMP
PresShell::CancelReflowCommand(nsIFrame*     aTargetFrame, 
                               nsReflowType* aCmdType)
{
  CancelReflowCommandInternal(aTargetFrame, aCmdType);
  return NS_OK;
}


NS_IMETHODIMP
PresShell::CancelAllReflowCommands()
{
  PRInt32 n = mReflowCommands.Count();
  nsHTMLReflowCommand* rc;
  PRInt32 i;
  for (i = 0; i < n; i++) {
    rc = NS_STATIC_CAST(nsHTMLReflowCommand*, mReflowCommands.ElementAt(i));
    ReflowCommandRemoved(rc);
    delete rc;
  }
  NS_ASSERTION(n == mReflowCommands.Count(),"reflow command list changed during cancel!");
  mReflowCommands.Clear();

  DoneRemovingReflowCommands();

  return NS_OK;
}

#ifdef ACCESSIBILITY
void nsIPresShell::InvalidateAccessibleSubtree(nsIContent *aContent)
{
  if (mIsAccessibilityActive) {
    nsCOMPtr<nsIAccessibilityService> accService = 
      do_GetService("@mozilla.org/accessibilityService;1");
    if (accService) {
      accService->InvalidateSubtreeFor(this, aContent,
                                       nsIAccessibleEvent::EVENT_REORDER);
    }
  }
}
#endif

NS_IMETHODIMP
PresShell::RecreateFramesFor(nsIContent* aContent)
{
  NS_ENSURE_TRUE(mPresContext, NS_ERROR_FAILURE);

  // Don't call RecreateFramesForContent since that is not exported and we want
  // to keep the number of entrypoints down.

  NS_ASSERTION(mViewManager, "Should have view manager");
  mViewManager->BeginUpdateViewBatch();

  // Have to make sure that the content notifications are flushed before we
  // start messing with the frame model; otherwise we can get content doubling.
  mDocument->FlushPendingNotifications(Flush_ContentAndNotify);

  nsStyleChangeList changeList;
  changeList.AppendChange(nsnull, aContent, nsChangeHint_ReconstructFrame);

  nsresult rv = mFrameConstructor->ProcessRestyledFrames(changeList);
  mViewManager->EndUpdateViewBatch(NS_VMREFRESH_NO_SYNC);
#ifdef ACCESSIBILITY
  InvalidateAccessibleSubtree(aContent);
#endif
  return rv;
}

NS_IMETHODIMP
PresShell::ClearFrameRefs(nsIFrame* aFrame)
{
  mPresContext->EventStateManager()->ClearFrameRefs(aFrame);
  
  if (aFrame == mCurrentEventFrame) {
    mCurrentEventContent = aFrame->GetContent();
    mCurrentEventFrame = nsnull;
  }

  for (int i=0; i<mCurrentEventFrameStack.Count(); i++) {
    if (aFrame == (nsIFrame*)mCurrentEventFrameStack.ElementAt(i)) {
      //One of our stack frames was deleted.  Get its content so that when we
      //pop it we can still get its new frame from its content
      nsIContent *currentEventContent = aFrame->GetContent();
      mCurrentEventContentStack.ReplaceObjectAt(currentEventContent, i);
      mCurrentEventFrameStack.ReplaceElementAt(nsnull, i);
    }
  }

  nsWeakFrame* weakFrame = mWeakFrames;
  while (weakFrame) {
    nsWeakFrame* prev = weakFrame->GetPreviousWeakFrame();
    if (weakFrame->GetFrame() == aFrame) {
      // This removes weakFrame from mWeakFrames.
      weakFrame->Clear(this);
    }
    weakFrame = prev;
  }

  return NS_OK;
}

NS_IMETHODIMP
PresShell::CreateRenderingContext(nsIFrame *aFrame,
                                  nsIRenderingContext** aResult)
{
  NS_PRECONDITION(nsnull != aResult, "null ptr");
  if (nsnull == aResult) {
    return NS_ERROR_NULL_POINTER;
  }

  nsIWidget* widget = nsnull;
  // Never pass a widget to a print rendering context
  if (mPresContext->IsScreen())
    widget = aFrame->GetWindow();

  nsresult rv;
  nsIRenderingContext* result = nsnull;
  nsIDeviceContext *deviceContext = mPresContext->DeviceContext();
  if (widget) {
    rv = deviceContext->CreateRenderingContext(widget, result);
  }
  else {
    rv = deviceContext->CreateRenderingContext(result);
  }
  *aResult = result;

  return rv;
}

NS_IMETHODIMP
PresShell::GoToAnchor(const nsAString& aAnchorName, PRBool aScroll)
{
  if (!mDocument) {
    return NS_ERROR_FAILURE;
  }
  
  // Hold a reference to the ESM in case event dispatch tears us down.
  nsCOMPtr<nsIEventStateManager> esm = mPresContext->EventStateManager();

  if (aAnchorName.IsEmpty()) {
    NS_ASSERTION(!aScroll, "can't scroll to empty anchor name");
    esm->SetContentState(nsnull, NS_EVENT_STATE_URLTARGET);
    return NS_OK;
  }

  nsCOMPtr<nsIDOMDocument> doc = do_QueryInterface(mDocument);
  nsCOMPtr<nsIDOMHTMLDocument> htmlDoc = do_QueryInterface(mDocument);
  nsresult rv = NS_OK;
  nsCOMPtr<nsIContent> content;

  // Search for an element with a matching "id" attribute
  if (doc) {    
    nsCOMPtr<nsIDOMElement> element;
    rv = doc->GetElementById(aAnchorName, getter_AddRefs(element));
    if (NS_SUCCEEDED(rv) && element) {
      // Get the nsIContent interface, because that's what we need to
      // get the primary frame
      content = do_QueryInterface(element);
    }
  }

  // Search for an anchor element with a matching "name" attribute
  if (!content && htmlDoc) {
    nsCOMPtr<nsIDOMNodeList> list;
    // Find a matching list of named nodes
    rv = htmlDoc->GetElementsByName(aAnchorName, getter_AddRefs(list));
    if (NS_SUCCEEDED(rv) && list) {
      PRUint32 i;
      // Loop through the named nodes looking for the first anchor
      for (i = 0; PR_TRUE; i++) {
        nsCOMPtr<nsIDOMNode> node;
        rv = list->Item(i, getter_AddRefs(node));
        if (!node) {  // End of list
          break;
        }
        // Ensure it's an anchor element
        content = do_QueryInterface(node);
        if (content) {
          if (content->Tag() == nsHTMLAtoms::a &&
              content->IsNodeOfType(nsINode::eHTML)) {
            break;
          }
          content = nsnull;
        }
      }
    }
  }

  // Search for anchor in the HTML namespace with a matching name
  if (!content && !htmlDoc)
  {
    nsCOMPtr<nsIDOMNodeList> list;
    NS_NAMED_LITERAL_STRING(nameSpace, "http://www.w3.org/1999/xhtml");
    // Get the list of anchor elements
    rv = doc->GetElementsByTagNameNS(nameSpace, NS_LITERAL_STRING("a"), getter_AddRefs(list));
    if (NS_SUCCEEDED(rv) && list) {
      PRUint32 i;
      // Loop through the named nodes looking for the first anchor
      for (i = 0; PR_TRUE; i++) {
        nsCOMPtr<nsIDOMNode> node;
        rv = list->Item(i, getter_AddRefs(node));
        if (!node) { // End of list
          break;
        }
        // Compare the name attribute
        nsCOMPtr<nsIDOMElement> element = do_QueryInterface(node);
        nsAutoString value;
        if (element && NS_SUCCEEDED(element->GetAttribute(NS_LITERAL_STRING("name"), value))) {
          if (value.Equals(aAnchorName)) {
            content = do_QueryInterface(element);
            break;
          }
        }
      }
    }
  }

  nsCOMPtr<nsIDOMRange> jumpToRange;
  nsCOMPtr<nsIXPointerResult> xpointerResult;
  if (!content) {
    nsCOMPtr<nsIDOMXMLDocument> xmldoc = do_QueryInterface(mDocument);
    if (xmldoc) {
      // Try XPointer
      xmldoc->EvaluateXPointer(aAnchorName, getter_AddRefs(xpointerResult));
      if (xpointerResult) {
        xpointerResult->Item(0, getter_AddRefs(jumpToRange));
        if (!jumpToRange) {
          // We know it was an XPointer, so there is no point in
          // trying any other pointer types, let's just return
          // an error.
          return NS_ERROR_FAILURE;
        }
      }

      // Finally try FIXptr
      if (!jumpToRange) {
        xmldoc->EvaluateFIXptr(aAnchorName,getter_AddRefs(jumpToRange));
      }

      if (jumpToRange) {
        nsCOMPtr<nsIDOMNode> node;
        jumpToRange->GetStartContainer(getter_AddRefs(node));
        if (node) {
          PRUint16 nodeType;
          node->GetNodeType(&nodeType);
          PRInt32 offset = -1;
          jumpToRange->GetStartOffset(&offset);
          switch (nodeType) {
            case nsIDOMNode::ATTRIBUTE_NODE:
            {
              // XXX Assuming jumping to the ownerElement is the sanest action.
              nsCOMPtr<nsIAttribute> attr = do_QueryInterface(node);
              content = attr->GetContent();
              break;
            }
            case nsIDOMNode::DOCUMENT_NODE:
            {
              if (offset >= 0) {
                nsCOMPtr<nsIDocument> document = do_QueryInterface(node);
                content = document->GetChildAt(offset);
              }
              break;
            }
            case nsIDOMNode::DOCUMENT_FRAGMENT_NODE:
            case nsIDOMNode::ELEMENT_NODE:
            case nsIDOMNode::ENTITY_REFERENCE_NODE:
            {
              if (offset >= 0) {
                nsCOMPtr<nsIContent> parent = do_QueryInterface(node);
                content = parent->GetChildAt(offset);
              }
              break;
            }
            case nsIDOMNode::CDATA_SECTION_NODE:
            case nsIDOMNode::COMMENT_NODE:
            case nsIDOMNode::TEXT_NODE:
            case nsIDOMNode::PROCESSING_INSTRUCTION_NODE:
            {
              // XXX This should scroll to a specific position in the text.
              content = do_QueryInterface(node);
              break;
            }
          }
        }
      }
    }
  }

  esm->SetContentState(content, NS_EVENT_STATE_URLTARGET);

  if (content) {
    // Flush notifications so we scroll to the right place
    if (aScroll) {
      mDocument->FlushPendingNotifications(Flush_Layout);   
      // Get the primary frame
      nsIFrame* frame = GetPrimaryFrameFor(content);
      NS_ENSURE_TRUE(frame, NS_ERROR_FAILURE);
      rv = ScrollFrameIntoView(frame, NS_PRESSHELL_SCROLL_TOP,
                               NS_PRESSHELL_SCROLL_ANYWHERE);
      NS_ENSURE_SUCCESS(rv, rv);
    }

    // Should we select the target? This action is controlled by a
    // preference: the default is to not select.
    PRBool selectAnchor = nsContentUtils::GetBoolPref("layout.selectanchor");

    // Even if select anchor pref is false, we must still move the
    // caret there. That way tabbing will start from the new
    // location
    if (!jumpToRange) {
      jumpToRange = do_CreateInstance(kRangeCID);
      if (jumpToRange) {
        while (content && content->GetChildCount() > 0) {
          content = content->GetChildAt(0);
        }
        nsCOMPtr<nsIDOMNode> node(do_QueryInterface(content));
        NS_ASSERTION(node, "No nsIDOMNode for descendant of anchor");
        jumpToRange->SelectNodeContents(node);
      }
    }
    if (jumpToRange) {
      // Select the anchor
      nsISelection* sel = mSelection->
        GetSelection(nsISelectionController::SELECTION_NORMAL);
      if (sel) {
        sel->RemoveAllRanges();
        sel->AddRange(jumpToRange);
        if (!selectAnchor) {
          // Use a caret (collapsed selection) at the start of the anchor
          sel->CollapseToStart();
        }
      }  

      if (selectAnchor && xpointerResult) {
        // Select the rest (if any) of the ranges in XPointerResult
        PRUint32 count, i;
        xpointerResult->GetLength(&count);
        for (i = 1; i < count; i++) { // jumpToRange is i = 0
          nsCOMPtr<nsIDOMRange> range;
          xpointerResult->Item(i, getter_AddRefs(range));
          sel->AddRange(range);
        }
      }
      // Selection is at anchor.
      // Now focus the document itself if focus is on an element within it.
      nsPIDOMWindow *win = mDocument->GetWindow();

      if (win) {
        nsCOMPtr<nsIFocusController> focusController = win->GetRootFocusController();
        if (focusController) {
          nsCOMPtr<nsIDOMWindowInternal> focusedWin;
          focusController->GetFocusedWindow(getter_AddRefs(focusedWin));
          if (SameCOMIdentity(win, focusedWin)) {
            esm->ChangeFocusWith(nsnull, nsIEventStateManager::eEventFocusedByApplication);
          }
        }
      }
    }
  } else {
    rv = NS_ERROR_FAILURE; //changed to NS_OK in quirks mode if ScrollTo is called
    
    // Scroll to the top/left if the anchor can not be
    // found and it is labelled top (quirks mode only). @see bug 80784
    if ((NS_LossyConvertUTF16toASCII(aAnchorName).LowerCaseEqualsLiteral("top")) &&
        (mPresContext->CompatibilityMode() == eCompatibility_NavQuirks)) {
      rv = NS_OK;
      // Check |aScroll| after setting |rv| so we set |rv| to the same
      // thing whether or not |aScroll| is true.
      if (aScroll && mViewManager) {
        // Get the viewport scroller
        nsIScrollableView* scrollingView;
        mViewManager->GetRootScrollableView(&scrollingView);
        if (scrollingView) {
          // Scroll to the top of the page
          scrollingView->ScrollTo(0, 0, NS_VMREFRESH_IMMEDIATE);
        }
      }
    }
  }

  return rv;
}

/**
 * This function takes a scrolling view, a rect, and a scroll position and
 * attempts to scroll that rect to that position in that view.  The rect
 * should be in the coordinate system of the _scrolled_ view.
 */
static void ScrollViewToShowRect(nsIScrollableView* aScrollingView,
                                 nsRect &           aRect,
                                 PRIntn             aVPercent,
                                 PRIntn             aHPercent)
{
  // Determine the visible rect in the scrolling view's coordinate space.
  // The size of the visible area is the clip view size
  nsRect visibleRect = aScrollingView->View()->GetBounds(); // get width and height
  aScrollingView->GetScrollPosition(visibleRect.x, visibleRect.y);

  // The actual scroll offsets
  nscoord scrollOffsetX = visibleRect.x;
  nscoord scrollOffsetY = visibleRect.y;

  nscoord lineHeight;
  aScrollingView->GetLineHeight(&lineHeight);
  
  // See how the rect should be positioned vertically
  if (NS_PRESSHELL_SCROLL_ANYWHERE == aVPercent) {
    // The caller doesn't care where the frame is positioned vertically,
    // so long as it's fully visible
    if (aRect.y < visibleRect.y) {
      // Scroll up so the frame's top edge is visible
      scrollOffsetY = aRect.y;
    } else if (aRect.YMost() > visibleRect.YMost()) {
      // Scroll down so the frame's bottom edge is visible. Make sure the
      // frame's top edge is still visible
      scrollOffsetY += aRect.YMost() - visibleRect.YMost();
      if (scrollOffsetY > aRect.y) {
        scrollOffsetY = aRect.y;
      }
    }
  } else if (NS_PRESSHELL_SCROLL_IF_NOT_VISIBLE == aVPercent) {
    // Scroll only if no part of the frame is visible in this view
    if (aRect.YMost() - lineHeight < visibleRect.y) {
      // Scroll up so the frame's top edge is visible
      scrollOffsetY = aRect.y;
    }  else if (aRect.y + lineHeight > visibleRect.YMost()) {
      // Scroll down so the frame's bottom edge is visible. Make sure the
      // frame's top edge is still visible
      scrollOffsetY += aRect.YMost() - visibleRect.YMost();
      if (scrollOffsetY > aRect.y) {
        scrollOffsetY = aRect.y;
      }
    }
  } else {
    // Align the frame edge according to the specified percentage
    nscoord frameAlignY =
      NSToCoordRound(aRect.y + aRect.height * (aVPercent / 100.0f));
    scrollOffsetY =
      NSToCoordRound(frameAlignY - visibleRect.height * (aVPercent / 100.0f));
  }

  // See how the frame should be positioned horizontally
  if (NS_PRESSHELL_SCROLL_ANYWHERE == aHPercent) {
    // The caller doesn't care where the frame is positioned horizontally,
    // so long as it's fully visible
    if (aRect.x < visibleRect.x) {
      // Scroll left so the frame's left edge is visible
      scrollOffsetX = aRect.x;
    } else if (aRect.XMost() > visibleRect.XMost()) {
      // Scroll right so the frame's right edge is visible. Make sure the
      // frame's left edge is still visible
      scrollOffsetX += aRect.XMost() - visibleRect.XMost();
      if (scrollOffsetX > aRect.x) {
        scrollOffsetX = aRect.x;
      }
    }
  } else if (NS_PRESSHELL_SCROLL_IF_NOT_VISIBLE == aHPercent) {
    // Scroll only if no part of the frame is visible in this view
    // XXXbz using the line height here is odd, but there are no
    // natural dimensions to use here, really....
    if (aRect.XMost() - lineHeight < visibleRect.x) {
      // Scroll left so the frame's left edge is visible
      scrollOffsetX = aRect.x;
    }  else if (aRect.x + lineHeight > visibleRect.XMost()) {
      // Scroll right so the frame's right edge is visible. Make sure the
      // frame's left edge is still visible
      scrollOffsetX += aRect.XMost() - visibleRect.XMost();
      if (scrollOffsetX > aRect.x) {
        scrollOffsetX = aRect.x;
      }
    }
  } else {
    // Align the frame edge according to the specified percentage
    nscoord frameAlignX =
      NSToCoordRound(aRect.x + (aRect.width) * (aHPercent / 100.0f));
    scrollOffsetX =
      NSToCoordRound(frameAlignX - visibleRect.width * (aHPercent / 100.0f));
  }

  aScrollingView->ScrollTo(scrollOffsetX, scrollOffsetY,
                           NS_VMREFRESH_IMMEDIATE);
}

NS_IMETHODIMP
PresShell::ScrollFrameIntoView(nsIFrame *aFrame,
                               PRIntn   aVPercent, 
                               PRIntn   aHPercent) const
{
  if (!aFrame) {
    return NS_ERROR_NULL_POINTER;
  }

  // Before we scroll the frame into view, ask the command dispatcher
  // if we're resetting focus because a window just got an activate
  // event. If we are, we do not want to scroll the frame into view.
  // Example: The user clicks on an anchor, and then deactivates the 
  // window. When they reactivate the window, the expected behavior
  // is not for the anchor link to scroll back into view. That is what
  // this check is preventing.
  // XXX: The dependency on the command dispatcher needs to be fixed.
  nsIContent* content = aFrame->GetContent();
  if (content) {
    nsIDocument* document = content->GetDocument();
    if (document){
      nsPIDOMWindow *ourWindow = document->GetWindow();
      if(ourWindow) {
        nsIFocusController *focusController =
          ourWindow->GetRootFocusController();
        if (focusController) {
          PRBool dontScroll;
          focusController->GetSuppressFocusScroll(&dontScroll);
          if(dontScroll)
            return NS_OK;
        }
      }
    }
  }

  // Flush out pending reflows to make sure we scroll to the right place
  mDocument->FlushPendingNotifications(Flush_OnlyReflow);
  
  // This is a two-step process.
  // Step 1: Find the bounds of the rect we want to scroll into view.  For
  //         example, for an inline frame we may want to scroll in the whole
  //         line.
  // Step 2: Walk the views that are parents of the frame and scroll them
  //         appropriately.
  
  nsRect  frameBounds = aFrame->GetRect();
  nsPoint offset;
  nsIView* closestView;
  aFrame->GetOffsetFromView(offset, &closestView);
  frameBounds.MoveTo(offset);

  // If this is an inline frame and either the bounds height is 0 (quirks
  // layout model) or aVPercent is not NS_PRESSHELL_SCROLL_ANYWHERE, we need to
  // change the top of the bounds to include the whole line.
  if (frameBounds.height == 0 || aVPercent != NS_PRESSHELL_SCROLL_ANYWHERE) {
    nsIAtom* frameType = NULL;
    nsIFrame *prevFrame = aFrame;
    nsIFrame *frame = aFrame;

    while (frame &&
           (frameType = frame->GetType()) == nsLayoutAtoms::inlineFrame) {
      prevFrame = frame;
      frame = prevFrame->GetParent();
    }

    if (frame != aFrame &&
        frame &&
        frameType == nsLayoutAtoms::blockFrame) {
      // find the line containing aFrame and increase the top of |offset|.
      nsCOMPtr<nsILineIterator> lines( do_QueryInterface(frame) );

      if (lines) {
        PRInt32 index = -1;
        lines->FindLineContaining(prevFrame, &index);
        if (index >= 0) {
          nsIFrame *trash1;
          PRInt32 trash2;
          nsRect lineBounds;
          PRUint32 trash3;

          if (NS_SUCCEEDED(lines->GetLine(index, &trash1, &trash2,
                                          lineBounds, &trash3))) {
            nsPoint blockOffset;
            nsIView* blockView;
            frame->GetOffsetFromView(blockOffset, &blockView);

            if (blockView == closestView) {
              // XXX If views not equal, this is hard.  Do we want to bother?
              nscoord newoffset = lineBounds.y + blockOffset.y;

              if (newoffset < frameBounds.y)
                frameBounds.y = newoffset;
            }
          }
        }
      }
    }
  }

  NS_ASSERTION(closestView && !closestView->ToScrollableView(),
               "What happened to the scrolled view?  "
               "The frame should not be directly in the scrolling view!");
  
  // Walk up the view hierarchy.  Make sure to add the view's position
  // _after_ we get the parent and see whether it's scrollable.  We want to
  // make sure to get the scrolled view's position after it has been scrolled.
  nsIScrollableView* scrollingView = nsnull;
  while (closestView) {
    nsIView* parent = closestView->GetParent();
    if (parent) {
      scrollingView = parent->ToScrollableView();
      if (scrollingView) {
        ScrollViewToShowRect(scrollingView, frameBounds, aVPercent, aHPercent);
      }
    }
    frameBounds += closestView->GetPosition();;
    closestView = parent;
  }

  return NS_OK;
}

// GetLinkLocation: copy link location to clipboard
NS_IMETHODIMP PresShell::GetLinkLocation(nsIDOMNode* aNode, nsAString& aLocationString)
{
#ifdef DEBUG_dr
  printf("dr :: PresShell::GetLinkLocation\n");
#endif

  NS_ENSURE_ARG_POINTER(aNode);
  nsresult rv;
  nsAutoString anchorText;
  static char strippedChars[] = {'\t','\r','\n'};

  // are we an anchor?
  nsCOMPtr<nsIDOMHTMLAnchorElement> anchor(do_QueryInterface(aNode));
  nsCOMPtr<nsIDOMHTMLAreaElement> area;
  nsCOMPtr<nsIDOMHTMLLinkElement> link;
  nsAutoString xlinkType;
  if (anchor) {
    rv = anchor->GetHref(anchorText);
    NS_ENSURE_SUCCESS(rv, rv);
  } else {
    // area?
    area = do_QueryInterface(aNode);
    if (area) {
      rv = area->GetHref(anchorText);
      NS_ENSURE_SUCCESS(rv, rv);
    } else {
      // link?
      link = do_QueryInterface(aNode);
      if (link) {
        rv = link->GetHref(anchorText);
        NS_ENSURE_SUCCESS(rv, rv);
      } else {
        // Xlink?
        nsCOMPtr<nsIDOMElement> element(do_QueryInterface(aNode));
        if (element) {
          NS_NAMED_LITERAL_STRING(xlinkNS,"http://www.w3.org/1999/xlink");
          element->GetAttributeNS(xlinkNS,NS_LITERAL_STRING("type"),xlinkType);
          if (xlinkType.EqualsLiteral("simple")) {
            element->GetAttributeNS(xlinkNS,NS_LITERAL_STRING("href"),anchorText);
            if (!anchorText.IsEmpty()) {
              // Resolve the full URI using baseURI property

              nsAutoString base;
              nsCOMPtr<nsIDOM3Node> node(do_QueryInterface(aNode,&rv));
              NS_ENSURE_SUCCESS(rv, rv);
              node->GetBaseURI(base);

              nsCOMPtr<nsIIOService>
                ios(do_GetService("@mozilla.org/network/io-service;1", &rv));
              NS_ENSURE_SUCCESS(rv, rv);

              nsCOMPtr<nsIURI> baseURI;
              rv = ios->NewURI(NS_ConvertUTF16toUTF8(base),nsnull,nsnull,getter_AddRefs(baseURI));
              NS_ENSURE_SUCCESS(rv, rv);

              nsCAutoString spec;
              rv = baseURI->Resolve(NS_ConvertUTF16toUTF8(anchorText),spec);
              NS_ENSURE_SUCCESS(rv, rv);

              CopyUTF8toUTF16(spec, anchorText);
            }
          }
        }
      }
    }
  }

  if (anchor || area || link || xlinkType.EqualsLiteral("simple")) {
    //Remove all the '\t', '\r' and '\n' from 'anchorText'
    anchorText.StripChars(strippedChars);

    aLocationString = anchorText;

    return NS_OK;
  }

  // if no link, fail.
  return NS_ERROR_FAILURE;
}

NS_IMETHODIMP
PresShell::GetSelectionForCopy(nsISelection** outSelection)
{
  nsresult rv;

  *outSelection = nsnull;

  if (!mDocument) return NS_ERROR_FAILURE;

  nsCOMPtr<nsIContent> content;
  nsPIDOMWindow *ourWindow = mDocument->GetWindow();
  if (ourWindow) {
    nsIFocusController *focusController = ourWindow->GetRootFocusController();
    if (focusController) {
      nsCOMPtr<nsIDOMElement> focusedElement;
      focusController->GetFocusedElement(getter_AddRefs(focusedElement));
      content = do_QueryInterface(focusedElement);
    }
  }

  nsCOMPtr<nsISelection> sel;
  if (content)
  {
    //check to see if we need to get selection from frame
    //optimization that MAY need to be expanded as more things implement their own "selection"
    nsCOMPtr<nsIDOMNSHTMLInputElement>    htmlInputElement(do_QueryInterface(content));
    nsCOMPtr<nsIDOMNSHTMLTextAreaElement> htmlTextAreaElement(do_QueryInterface(content));
    if (htmlInputElement || htmlTextAreaElement)
    {
      nsIFrame *htmlInputFrame = GetPrimaryFrameFor(content);
      if (!htmlInputFrame) return NS_ERROR_FAILURE;

      nsCOMPtr<nsISelectionController> selCon;
      rv = htmlInputFrame->
        GetSelectionController(mPresContext,getter_AddRefs(selCon));
      if (NS_FAILED(rv)) return rv;
      if (!selCon) return NS_ERROR_FAILURE;

      rv = selCon->GetSelection(nsISelectionController::SELECTION_NORMAL,
                                getter_AddRefs(sel));
    }
  }
  if (!sel) {
    sel = mSelection->GetSelection(nsISelectionController::SELECTION_NORMAL);
    rv = NS_OK;
  }

  *outSelection = sel;
  NS_IF_ADDREF(*outSelection);
  return rv;
}


NS_IMETHODIMP
PresShell::DoGetContents(const nsACString& aMimeType, PRUint32 aFlags, PRBool aSelectionOnly, nsAString& aOutValue)
{
  aOutValue.Truncate();
  
  if (!mDocument) return NS_ERROR_FAILURE;

  nsresult rv;
  nsCOMPtr<nsISelection> sel;

  // Now we have the selection.  Make sure it's nonzero:
  if (aSelectionOnly)
  {
    rv = GetSelectionForCopy(getter_AddRefs(sel));
    if (NS_FAILED(rv)) return rv;
    if (!sel) return NS_ERROR_FAILURE;
  
    PRBool isCollapsed;
    sel->GetIsCollapsed(&isCollapsed);
    if (isCollapsed)
      return NS_OK;
  }
  
  // call the copy code
  return nsCopySupport::GetContents(aMimeType, aFlags, sel,
                                    mDocument, aOutValue);
}

NS_IMETHODIMP
PresShell::DoCopy()
{
  if (!mDocument) return NS_ERROR_FAILURE;

  nsCOMPtr<nsISelection> sel;
  nsresult rv = GetSelectionForCopy(getter_AddRefs(sel));
  if (NS_FAILED(rv)) 
    return rv;
  if (!sel) 
    return NS_ERROR_FAILURE;

  // Now we have the selection.  Make sure it's nonzero:
  PRBool isCollapsed;
  sel->GetIsCollapsed(&isCollapsed);
  if (isCollapsed)
    return NS_OK;

  // call the copy code
  rv = nsCopySupport::HTMLCopy(sel, mDocument, nsIClipboard::kGlobalClipboard);
  if (NS_FAILED(rv))
    return rv;

  // Now that we have copied, update the Paste menu item
  nsPIDOMWindow *domWindow = mDocument->GetWindow();
  if (domWindow)
  {
    domWindow->UpdateCommands(NS_LITERAL_STRING("clipboard"));
  }
  
  return NS_OK;
}

NS_IMETHODIMP
PresShell::CaptureHistoryState(nsILayoutHistoryState** aState, PRBool aLeavingPage)
{
  nsresult rv = NS_OK;

  NS_PRECONDITION(nsnull != aState, "null state pointer");

  // We actually have to mess with the docshell here, since we want to
  // store the state back in it.
  // XXXbz this isn't really right, since this is being called in the
  // content viewer's Hide() method...  by that point the docshell's
  // state could be wrong.  We should sort out a better ownership
  // model for the layout history state.
  nsCOMPtr<nsISupports> container = mPresContext->GetContainer();
  if (!container)
    return NS_ERROR_FAILURE;

  nsCOMPtr<nsIDocShell> docShell(do_QueryInterface(container));
  if (!docShell)
    return NS_ERROR_FAILURE;

  nsCOMPtr<nsILayoutHistoryState> historyState;
  docShell->GetLayoutHistoryState(getter_AddRefs(historyState));
  if (!historyState) {
    // Create the document state object
    rv = NS_NewLayoutHistoryState(getter_AddRefs(historyState));
  
    if (NS_FAILED(rv)) { 
      *aState = nsnull;
      return rv;
    }    

    docShell->SetLayoutHistoryState(historyState);
  }

  *aState = historyState;
  NS_IF_ADDREF(*aState);
  
  // Capture frame state for the entire frame hierarchy
  nsIFrame* rootFrame = FrameManager()->GetRootFrame();
  if (!rootFrame) return NS_OK;
  // Capture frame state for the root scroll frame
  // Don't capture state when first creating doc element hierarchy
  // As the scroll position is 0 and this will cause us to loose
  // our previously saved place!
  if (aLeavingPage) {
    nsIFrame* scrollFrame = GetRootScrollFrame();
    if (scrollFrame) {
      FrameManager()->CaptureFrameStateFor(scrollFrame, historyState,
                                           nsIStatefulFrame::eDocumentScrollState);
    }
  }

  FrameManager()->CaptureFrameState(rootFrame, historyState);  
 
  return NS_OK;
}

NS_IMETHODIMP
PresShell::GetGeneratedContentIterator(nsIContent*          aContent,
                                       GeneratedContentType aType,
                                       nsIContentIterator** aIterator) const
{
  nsresult  rv = NS_OK;

  // Initialize OUT parameter
  *aIterator = nsnull;

  // Get the primary frame associated with the content object
  nsIFrame* primaryFrame = GetPrimaryFrameFor(aContent);
  if (primaryFrame) {
    // See whether it's a request for the before or after generated content
    if (Before == aType) {
      nsIFrame* beforeFrame = nsLayoutUtils::GetBeforeFrame(primaryFrame);
      if (beforeFrame) {
        // Create an iterator
        rv = NS_NewFrameContentIterator(mPresContext, beforeFrame, aIterator);
      }
      
    } else {
      // Avoid finding the :after frame unless we need to (it's
      // expensive). Instead probe for the existence of the pseudo-element
      nsStyleContext *styleContext;
      
      styleContext = primaryFrame->GetStyleContext();
      if (nsLayoutUtils::HasPseudoStyle(aContent, styleContext,
                                        nsCSSPseudoElements::after,
                                        mPresContext)) {
        nsIFrame* afterFrame = nsLayoutUtils::GetAfterFrame(primaryFrame);
        if (afterFrame)
        {
          NS_ASSERTION(afterFrame->IsGeneratedContentFrame(),
                       "can't find generated content frame");
          // Create an iterator
          rv = NS_NewFrameContentIterator(mPresContext, afterFrame, aIterator);
        }
      }
    }
  }

  return rv;
}


NS_IMETHODIMP
PresShell::SetAnonymousContentFor(nsIContent* aContent, nsISupportsArray* aAnonymousElements)
{
  NS_PRECONDITION(aContent != nsnull, "null ptr");
  if (! aContent)
    return NS_ERROR_NULL_POINTER;

  if (! mAnonymousContentTable) {
    mAnonymousContentTable = new nsSupportsHashtable;
    if (! mAnonymousContentTable)
      return NS_ERROR_OUT_OF_MEMORY;
  }

  nsISupportsKey key(aContent);

  nsCOMPtr<nsISupportsArray> oldAnonymousElements =
    getter_AddRefs(NS_STATIC_CAST(nsISupportsArray*, mAnonymousContentTable->Get(&key)));

  if (!oldAnonymousElements) {
    if (aAnonymousElements) {
      mAnonymousContentTable->Put(&key, aAnonymousElements);
    }
  } else {
    if (aAnonymousElements) {
      oldAnonymousElements->AppendElements(aAnonymousElements);
    } else {
      // If we're trying to clear anonymous content for an element that
      // already had anonymous content, then we need to be sure to clean
      // up after the old content. (This can happen, for example, when a
      // reframe occurs.)
      PRUint32 count;
      oldAnonymousElements->Count(&count);
      
      while (PRInt32(--count) >= 0) {
        nsCOMPtr<nsIContent> content = do_QueryElementAt(oldAnonymousElements,
                                                         count);
        NS_ASSERTION(content != nsnull, "not an nsIContent");
        if (! content)
          continue;
        
        content->UnbindFromTree();
      }

      if (!mIsReleasingAnonymousContent)
        mAnonymousContentTable->Remove(&key);
    }
  }

  return NS_OK;
}


NS_IMETHODIMP
PresShell::GetAnonymousContentFor(nsIContent* aContent, nsISupportsArray** aAnonymousElements)
{
  if (! mAnonymousContentTable) {
    *aAnonymousElements = nsnull;
    return NS_OK;
  }

  nsISupportsKey key(aContent);
  *aAnonymousElements =
    NS_REINTERPRET_CAST(nsISupportsArray*, mAnonymousContentTable->Get(&key)); // addrefs

  return NS_OK;
}


static PRBool PR_CALLBACK
ClearDocumentEnumerator(nsHashKey* aKey, void* aData, void* aClosure)
{
  nsISupportsArray* anonymousElements =
    NS_STATIC_CAST(nsISupportsArray*, aData);

  PRUint32 count;
  anonymousElements->Count(&count);
  while (PRInt32(--count) >= 0) {
    nsCOMPtr<nsIContent> content = do_QueryElementAt(anonymousElements, count);
    NS_ASSERTION(content != nsnull, "not an nsIContent");
    if (! content)
      continue;

    content->UnbindFromTree();
  }

  return PR_TRUE;
}


NS_IMETHODIMP
PresShell::ReleaseAnonymousContent()
{
  if (mAnonymousContentTable) {
    mIsReleasingAnonymousContent = PR_TRUE;
    mAnonymousContentTable->Enumerate(ClearDocumentEnumerator);
    delete mAnonymousContentTable;
    mAnonymousContentTable = nsnull;
  }
  return NS_OK;
}

NS_IMETHODIMP
PresShell::IsPaintingSuppressed(PRBool* aResult)
{
  *aResult = mPaintingSuppressed;
  return NS_OK;
}

void
PresShell::UnsuppressAndInvalidate()
{
  if (!mPresContext->EnsureVisible(PR_FALSE)) {
    // No point; we're about to be torn down anyway.
    return;
  }
  
  mPaintingSuppressed = PR_FALSE;
  nsIFrame* rootFrame = FrameManager()->GetRootFrame();
  if (rootFrame) {
    // let's assume that outline on a root frame is not supported
    nsRect rect(nsPoint(0, 0), rootFrame->GetSize());
    rootFrame->Invalidate(rect, PR_FALSE);
  }

  // This makes sure to get the same thing that nsPresContext::EnsureVisible()
  // got.
  nsCOMPtr<nsISupports> container = mPresContext->GetContainer();
  nsCOMPtr<nsPIDOMWindow> ourWindow = do_GetInterface(container);
  nsCOMPtr<nsIFocusController> focusController =
    ourWindow ? ourWindow->GetRootFocusController() : nsnull;

  if (ourWindow)
    CheckForFocus(ourWindow, focusController, mDocument);

  if (focusController) // Unsuppress now that we've shown the new window and focused it.
    focusController->SetSuppressFocus(PR_FALSE, "PresShell suppression on Web page loads");

  if (mViewManager)
    mViewManager->SynthesizeMouseMove(PR_FALSE);
}

NS_IMETHODIMP
PresShell::UnsuppressPainting()
{
  if (mPaintSuppressionTimer) {
    mPaintSuppressionTimer->Cancel();
    mPaintSuppressionTimer = nsnull;
  }

  if (mIsDocumentGone || !mPaintingSuppressed)
    return NS_OK;

  // If we have reflows pending, just wait until we process
  // the reflows and get all the frames where we want them
  // before actually unlocking the painting.  Otherwise
  // go ahead and unlock now.
  if (mReflowCommands.Count() > 0)
    mShouldUnsuppressPainting = PR_TRUE;
  else
    UnsuppressAndInvalidate();
  return NS_OK;
}

NS_IMETHODIMP
PresShell::DisableThemeSupport()
{
  // Doesn't have to be dynamic.  Just set the bool.
  mIsThemeSupportDisabled = PR_TRUE;
  return NS_OK;
}

PRBool 
PresShell::IsThemeSupportEnabled()
{
  return !mIsThemeSupportDisabled;
}

// Post a request to handle an arbitrary callback after reflow has finished.
NS_IMETHODIMP
PresShell::PostReflowCallback(nsIReflowCallback* aCallback)
{
  void* result = AllocateFrame(sizeof(nsCallbackEventRequest));
  if (NS_UNLIKELY(!result)) {
    return NS_ERROR_OUT_OF_MEMORY;
  }
  nsCallbackEventRequest* request = (nsCallbackEventRequest*)result;

  request->callback = aCallback;
  NS_ADDREF(aCallback);
  request->next = nsnull;

  if (mLastCallbackEventRequest) {
    mLastCallbackEventRequest = mLastCallbackEventRequest->next = request;
  } else {
    mFirstCallbackEventRequest = request;
    mLastCallbackEventRequest = request;
  }
 
  return NS_OK;
}

NS_IMETHODIMP
PresShell::CancelReflowCallback(nsIReflowCallback* aCallback)
{
   nsCallbackEventRequest* before = nsnull;
   nsCallbackEventRequest* node = mFirstCallbackEventRequest;
   while(node)
   {
      nsIReflowCallback* callback = node->callback;

      if (callback == aCallback) 
      {
        nsCallbackEventRequest* toFree = node;
        if (node == mFirstCallbackEventRequest) {
          node = node->next;
          mFirstCallbackEventRequest = node;
          NS_ASSERTION(before == nsnull, "impossible");
        } else {
          node = node->next;
          before->next = node;
        }

        if (toFree == mLastCallbackEventRequest) {
          mLastCallbackEventRequest = before;
        }

        FreeFrame(sizeof(nsCallbackEventRequest), toFree);
        NS_RELEASE(callback);
      } else {
        before = node;
        node = node->next;
      }
   }

   return NS_OK;
}

/**
* Post a request to handle a DOM event after Reflow has finished.
* The event must have been created with the "new" operator.
*/
NS_IMETHODIMP
PresShell::PostDOMEvent(nsIContent* aContent, nsEvent* aEvent)
{
 // ok we have a list of events to handle. Queue them up and handle them
 // after we finish reflow.

  void* result = AllocateFrame(sizeof(nsDOMEventRequest));
  if (NS_UNLIKELY(!result)) {
    return NS_ERROR_OUT_OF_MEMORY;
  }
  nsDOMEventRequest* request = (nsDOMEventRequest*)result;

  request->content = aContent;
  NS_ADDREF(aContent);

  request->event = aEvent;
  request->next = nsnull;

  if (mLastDOMEventRequest) {
    mLastDOMEventRequest = mLastDOMEventRequest->next = request;
  } else {
    mFirstDOMEventRequest = request;
    mLastDOMEventRequest = request;
  }
 
  return NS_OK;
}


/**
 * Post a request to set and attribute after reflow has finished.
 */
NS_IMETHODIMP
PresShell::PostAttributeChange(nsIContent* aContent,
                               PRInt32 aNameSpaceID, 
                               nsIAtom* aName,
                               const nsString& aValue,
                               PRBool aNotify,
                               nsAttributeChangeType aType)
{
 // ok we have a list of events to handle. Queue them up and handle them
 // after we finish reflow.

  void* result = AllocateFrame(sizeof(nsAttributeChangeRequest));
  if (NS_UNLIKELY(!result)) {
    return NS_ERROR_OUT_OF_MEMORY;
  }
  nsAttributeChangeRequest* request = new (result) nsAttributeChangeRequest();

  request->content = aContent;
  NS_ADDREF(aContent);

  request->nameSpaceID = aNameSpaceID;
  request->name = aName;
  request->value = aValue;
  request->notify = aNotify;
  request->type = aType;
  request->next = nsnull;

  if (mLastAttributeRequest) {
    mLastAttributeRequest = mLastAttributeRequest->next = request;
  } else {
    mFirstAttributeRequest = request;
    mLastAttributeRequest = request;
  }

  return NS_OK;
}


void
PresShell::HandlePostedReflowCallbacks()
{
   PRBool shouldFlush = PR_FALSE;

   while (mFirstCallbackEventRequest) {
     nsCallbackEventRequest* node = mFirstCallbackEventRequest;
     mFirstCallbackEventRequest = node->next;
     if (!mFirstCallbackEventRequest) {
       mLastCallbackEventRequest = nsnull;
     }
     nsIReflowCallback* callback = node->callback;
     FreeFrame(sizeof(nsCallbackEventRequest), node);
     if (callback)
       callback->ReflowFinished(this, &shouldFlush);
     NS_IF_RELEASE(callback);
   }

   if (shouldFlush)
     FlushPendingNotifications(Flush_Layout);
}

void
PresShell::HandlePostedDOMEvents()
{
   while(mFirstDOMEventRequest)
   {
      /* pull the node from the event request list. Be prepared for reentrant access to the list
         from within Dispatch and its callees! */
      nsDOMEventRequest* node = mFirstDOMEventRequest;
      nsEventStatus status = nsEventStatus_eIgnore;

      mFirstDOMEventRequest = node->next;
      if (nsnull == mFirstDOMEventRequest) {
        mLastDOMEventRequest = nsnull;
      }

      nsEventDispatcher::Dispatch(node->content, mPresContext, node->event,
                                  nsnull, &status);
      NS_RELEASE(node->content);
      delete node->event;
      node->nsDOMEventRequest::~nsDOMEventRequest(); // doesn't do anything, but just in case
      FreeFrame(sizeof(nsDOMEventRequest), node);
   }
}

void
PresShell::HandlePostedAttributeChanges()
{
   while(mFirstAttributeRequest)
   {
      /* pull the node from the request list. Be prepared for reentrant access to the list
         from within SetAttribute/UnsetAttribute and its callees! */
      nsAttributeChangeRequest* node = mFirstAttributeRequest;

      mFirstAttributeRequest = node->next;
      if (nsnull == mFirstAttributeRequest) {
        mLastAttributeRequest = nsnull;
      }

      if (node->type == eChangeType_Set)
         node->content->SetAttr(node->nameSpaceID, node->name, node->value, node->notify);   
      else
         node->content->UnsetAttr(node->nameSpaceID, node->name, node->notify);   

      NS_RELEASE(node->content);
      node->nsAttributeChangeRequest::~nsAttributeChangeRequest();
      FreeFrame(sizeof(nsAttributeChangeRequest), node);
   }
}

NS_IMETHODIMP 
PresShell::IsSafeToFlush(PRBool& aIsSafeToFlush)
{
  aIsSafeToFlush = PR_TRUE;

  if (mIsReflowing || mChangeNestCount) {
    // Not safe if we are reflowing or in the middle of frame construction
    aIsSafeToFlush = PR_FALSE;
  } else {
    // Not safe if we are painting
    nsIViewManager* viewManager = GetViewManager();
    if (viewManager) {
      PRBool isPainting = PR_FALSE;
      viewManager->IsPainting(isPainting);
      if (isPainting) {
        aIsSafeToFlush = PR_FALSE;
      }
    }
  }
  return NS_OK;
}


NS_IMETHODIMP 
PresShell::FlushPendingNotifications(mozFlushType aType)
{
  NS_ASSERTION(aType & (Flush_StyleReresolves | Flush_OnlyReflow |
                        Flush_OnlyPaint),
               "Why did we get called?");
  
  PRBool isSafeToFlush;
  IsSafeToFlush(isSafeToFlush);

  NS_ASSERTION(!isSafeToFlush || mViewManager, "Must have view manager");
  if (isSafeToFlush && mViewManager) {
    // Style reresolves not in conjunction with reflows can't cause
    // painting or geometry changes, so don't bother with view update
    // batching if we only have style reresolve
    mViewManager->BeginUpdateViewBatch();

    if (aType & Flush_StyleReresolves) {
      mFrameConstructor->ProcessPendingRestyles();
    }

    if (aType & Flush_OnlyReflow) {
      ProcessReflowCommands(PR_FALSE);
    }

    PRUint32 updateFlags = NS_VMREFRESH_NO_SYNC;
    if (aType & Flush_OnlyPaint) {
      // Flushing paints, so perform the invalidates and drawing
      // immediately
      updateFlags = NS_VMREFRESH_IMMEDIATE;
    }
    else if (!(aType & Flush_OnlyReflow)) {
      // Not flushing reflows, so do deferred invalidates.  This will keep us
      // from possibly flushing out reflows due to invalidates being processed
      // at the end of this view batch.
      updateFlags = NS_VMREFRESH_DEFERRED;
    }
    mViewManager->EndUpdateViewBatch(updateFlags);
  }

  return NS_OK;
}

NS_IMETHODIMP
PresShell::IsReflowLocked(PRBool* aIsReflowLocked) 
{
  *aIsReflowLocked = mIsReflowing;
  return NS_OK;
}

NS_IMETHODIMP 
PresShell::BeginReflowBatching()
{  
  mBatchReflows = PR_TRUE;
  return NS_OK;
}

NS_IMETHODIMP 
PresShell::EndReflowBatching(PRBool aFlushPendingReflows)
{  
  nsresult rv = NS_OK;
  mBatchReflows = PR_FALSE;
  if (aFlushPendingReflows) {
    rv = FlushPendingNotifications(Flush_OnlyReflow);
  }
  else {
    PostReflowEvent();
  }
  return rv;
}


NS_IMETHODIMP
PresShell::GetReflowBatchingStatus(PRBool* aIsBatching)
{
  *aIsBatching = mBatchReflows;
  return NS_OK;
}

void
PresShell::CharacterDataChanged(nsIDocument *aDocument,
                                nsIContent*  aContent,
                                PRBool aAppend)
{
  NS_PRECONDITION(!mIsDocumentGone, "Unexpected CharacterDataChanged");
  NS_PRECONDITION(aDocument == mDocument, "Unexpected aDocument");

  WillCauseReflow();
  mFrameConstructor->CharacterDataChanged(aContent, aAppend);
  VERIFY_STYLE_TREE;
  DidCauseReflow();
}

void
PresShell::ContentStatesChanged(nsIDocument* aDocument,
                                nsIContent* aContent1,
                                nsIContent* aContent2,
                                PRInt32 aStateMask)
{
  NS_PRECONDITION(!mIsDocumentGone, "Unexpected ContentStatesChanged");
  NS_PRECONDITION(aDocument == mDocument, "Unexpected aDocument");

  WillCauseReflow();
  mFrameConstructor->ContentStatesChanged(aContent1, aContent2, aStateMask);
  VERIFY_STYLE_TREE;
  DidCauseReflow();
}


void
PresShell::AttributeChanged(nsIDocument* aDocument,
                            nsIContent*  aContent,
                            PRInt32      aNameSpaceID,
                            nsIAtom*     aAttribute,
                            PRInt32      aModType)
{
  NS_PRECONDITION(!mIsDocumentGone, "Unexpected AttributeChanged");
  NS_PRECONDITION(aDocument == mDocument, "Unexpected aDocument");

  // XXXwaterson it might be more elegant to wait until after the
  // initial reflow to begin observing the document. That would
  // squelch any other inappropriate notifications as well.
  if (mDidInitialReflow) {
    WillCauseReflow();
    mFrameConstructor->AttributeChanged(aContent, aNameSpaceID,
                                        aAttribute, aModType);
    VERIFY_STYLE_TREE;
    DidCauseReflow();
  }
}

void
PresShell::ContentAppended(nsIDocument *aDocument,
                           nsIContent* aContainer,
                           PRInt32     aNewIndexInContainer)
{
  NS_PRECONDITION(!mIsDocumentGone, "Unexpected ContentAppended");
  NS_PRECONDITION(aDocument == mDocument, "Unexpected aDocument");
  
  if (!mDidInitialReflow) {
    return;
  }
  
  WillCauseReflow();
  MOZ_TIMER_DEBUGLOG(("Start: Frame Creation: PresShell::ContentAppended(), this=%p\n", this));
  MOZ_TIMER_START(mFrameCreationWatch);

  mFrameConstructor->ContentAppended(aContainer, aNewIndexInContainer);
  VERIFY_STYLE_TREE;

  MOZ_TIMER_DEBUGLOG(("Stop: Frame Creation: PresShell::ContentAppended(), this=%p\n", this));
  MOZ_TIMER_STOP(mFrameCreationWatch);
  DidCauseReflow();
}

void
PresShell::ContentInserted(nsIDocument* aDocument,
                           nsIContent*  aContainer,
                           nsIContent*  aChild,
                           PRInt32      aIndexInContainer)
{
  NS_PRECONDITION(!mIsDocumentGone, "Unexpected ContentInserted");
  NS_PRECONDITION(aDocument == mDocument, "Unexpected aDocument");

  if (!mDidInitialReflow) {
    return;
  }
  
  WillCauseReflow();
  mFrameConstructor->ContentInserted(aContainer, aChild,
                                     aIndexInContainer, nsnull, PR_FALSE);
  VERIFY_STYLE_TREE;
  DidCauseReflow();
}

void
PresShell::ContentRemoved(nsIDocument *aDocument,
                          nsIContent* aContainer,
                          nsIContent* aChild,
                          PRInt32     aIndexInContainer)
{
  NS_PRECONDITION(!mIsDocumentGone, "Unexpected ContentRemoved");
  NS_PRECONDITION(aDocument == mDocument, "Unexpected aDocument");

  // Make sure that the caret doesn't leave a turd where the child used to be.
  if (mCaret) {
    mCaret->InvalidateOutsideCaret();
  }

  // Notify the ESM that the content has been removed, so that
  // it can clean up any state related to the content.
  mPresContext->EventStateManager()->ContentRemoved(aChild);

  WillCauseReflow();
  mFrameConstructor->ContentRemoved(aContainer, aChild,
                                    aIndexInContainer, PR_FALSE);

  VERIFY_STYLE_TREE;
  DidCauseReflow();
}

nsresult
PresShell::ReconstructFrames(void)
{
  nsresult rv = NS_OK;
          
  WillCauseReflow();
  rv = mFrameConstructor->ReconstructDocElementHierarchy();
  VERIFY_STYLE_TREE;
  DidCauseReflow();

  return rv;
}

void
nsIPresShell::ReconstructStyleDataInternal()
{
  mStylesHaveChanged = PR_FALSE;

  if (!mDidInitialReflow) {
    // Nothing to do here, since we have no frames yet
    return;
  }

  if (mIsDestroying) {
    // We don't want to mess with restyles at this point
    return;
  }

  nsIContent* root = mDocument->GetRootContent();
  if (!root) {
    // No content to restyle
    return;
  }
  
  mFrameConstructor->PostRestyleEvent(root, eReStyle_Self, NS_STYLE_HINT_NONE);

#ifdef ACCESSIBILITY
  InvalidateAccessibleSubtree(nsnull);
#endif
}

void
nsIPresShell::ReconstructStyleDataExternal()
{
  ReconstructStyleDataInternal();
}

void
PresShell::StyleSheetAdded(nsIDocument *aDocument,
                           nsIStyleSheet* aStyleSheet,
                           PRBool aDocumentSheet)
{
  // We only care when enabled sheets are added
  NS_PRECONDITION(aStyleSheet, "Must have a style sheet!");
  PRBool applicable;
  aStyleSheet->GetApplicable(applicable);

  if (applicable && aStyleSheet->HasRules()) {
    mStylesHaveChanged = PR_TRUE;
  }
}

void 
PresShell::StyleSheetRemoved(nsIDocument *aDocument,
                             nsIStyleSheet* aStyleSheet,
                             PRBool aDocumentSheet)
{
  // We only care when enabled sheets are removed
  NS_PRECONDITION(aStyleSheet, "Must have a style sheet!");
  PRBool applicable;
  aStyleSheet->GetApplicable(applicable);
  if (applicable && aStyleSheet->HasRules()) {
    mStylesHaveChanged = PR_TRUE;
  }
}

void
PresShell::StyleSheetApplicableStateChanged(nsIDocument *aDocument,
                                            nsIStyleSheet* aStyleSheet,
                                            PRBool aApplicable)
{
  if (aStyleSheet->HasRules()) {
    mStylesHaveChanged = PR_TRUE;
  }
}

void
PresShell::StyleRuleChanged(nsIDocument *aDocument,
                            nsIStyleSheet* aStyleSheet,
                            nsIStyleRule* aOldStyleRule,
                            nsIStyleRule* aNewStyleRule)
{
  mStylesHaveChanged = PR_TRUE;
}

void
PresShell::StyleRuleAdded(nsIDocument *aDocument,
                          nsIStyleSheet* aStyleSheet,
                          nsIStyleRule* aStyleRule) 
{
  mStylesHaveChanged = PR_TRUE;
}

void
PresShell::StyleRuleRemoved(nsIDocument *aDocument,
                            nsIStyleSheet* aStyleSheet,
                            nsIStyleRule* aStyleRule) 
{
  mStylesHaveChanged = PR_TRUE;
}

nsIFrame*
PresShell::GetPrimaryFrameFor(nsIContent* aContent) const
{
  return FrameManager()->GetPrimaryFrameFor(aContent);
}

NS_IMETHODIMP
PresShell::GetLayoutObjectFor(nsIContent*   aContent,
                              nsISupports** aResult) const 
{
  nsresult result = NS_ERROR_NULL_POINTER;
  if ((nsnull!=aResult) && (nsnull!=aContent))
  {
    *aResult = nsnull;
    nsIFrame *primaryFrame = GetPrimaryFrameFor(aContent);
    if (primaryFrame)
    {
      result = primaryFrame->QueryInterface(NS_GET_IID(nsISupports),
                                            (void**)aResult);
    }
  }
  return result;
}

NS_IMETHODIMP
PresShell::GetPlaceholderFrameFor(nsIFrame*  aFrame,
                                  nsIFrame** aResult) const
{
  *aResult = FrameManager()->GetPlaceholderFrameFor(aFrame);
  return NS_OK;
}

#ifdef IBMBIDI
NS_IMETHODIMP
PresShell::SetCaretBidiLevel(PRUint8 aLevel)
{
  // If the current level is undefined, we have just inserted new text.
  // In this case, we don't want to reset the keyboard language
  PRBool afterInsert = mBidiLevel & BIDI_LEVEL_UNDEFINED;
  mBidiLevel = aLevel;

//  PRBool parityChange = ((mBidiLevel ^ aLevel) & 1); // is the parity of the new level different from the current level?
//  if (parityChange)                                  // if so, change the keyboard language
  if (mBidiKeyboard && !afterInsert)
    mBidiKeyboard->SetLangFromBidiLevel(aLevel);
  return NS_OK;
}

NS_IMETHODIMP
PresShell::GetCaretBidiLevel(PRUint8 *aOutLevel)
{
  if (!aOutLevel) { return NS_ERROR_INVALID_ARG; }
  *aOutLevel = mBidiLevel;
  return NS_OK;    
}

NS_IMETHODIMP
PresShell::UndefineCaretBidiLevel()
{
  mBidiLevel |= BIDI_LEVEL_UNDEFINED;
  return NS_OK;
}

NS_IMETHODIMP
PresShell::BidiStyleChangeReflow()
{
  // Have the root frame's style context remap its style
  nsIFrame* rootFrame = FrameManager()->GetRootFrame();
  if (rootFrame) {
    mStyleSet->ClearStyleData(mPresContext);
    ReconstructFrames();
  }
  return NS_OK;
}
#endif // IBMBIDI

//nsIViewObserver

NS_IMETHODIMP
PresShell::ComputeRepaintRegionForCopy(nsIView*      aRootView,
                                       nsIView*      aMovingView,
                                       nsPoint       aDelta,
                                       const nsRect& aCopyRect,
                                       nsRegion*     aRepaintRegion)
{
  return nsLayoutUtils::ComputeRepaintRegionForCopy(
      NS_STATIC_CAST(nsIFrame*, aRootView->GetClientData()),
      NS_STATIC_CAST(nsIFrame*, aMovingView->GetClientData()),
      aDelta, aCopyRect, aRepaintRegion);
}

NS_IMETHODIMP
PresShell::RenderOffscreen(nsRect aRect, PRBool aUntrusted,
                           PRBool aIgnoreViewportScrolling,
                           nscolor aBackgroundColor,
                           nsIRenderingContext** aRenderedContext)
{
  nsIView* rootView;
  mViewManager->GetRootView(rootView);
  NS_ASSERTION(rootView, "No root view?");
  nsIWidget* rootWidget = rootView->GetWidget();
  NS_ASSERTION(rootWidget, "No root widget?");

  *aRenderedContext = nsnull;

  NS_ASSERTION(!aUntrusted, "We don't support untrusted yet");
  if (aUntrusted)
    return NS_ERROR_NOT_IMPLEMENTED;

  nsCOMPtr<nsIRenderingContext> tmpContext;
  mPresContext->DeviceContext()->CreateRenderingContext(rootWidget, 
      *getter_AddRefs(tmpContext));
  if (!tmpContext)
    return NS_ERROR_FAILURE;

  nsRect bounds(nsPoint(0, 0), aRect.Size());
  bounds.ScaleRoundOut(mPresContext->TwipsToPixels());
  
  nsIDrawingSurface* surface;
  nsresult rv
    = tmpContext->CreateDrawingSurface(bounds, NS_CREATEDRAWINGSURFACE_FOR_PIXEL_ACCESS,
                                       surface);
  if (NS_FAILED(rv))
    return NS_ERROR_FAILURE;
  nsCOMPtr<nsIRenderingContext> localcx;
  rv = nsLayoutUtils::CreateOffscreenContext(mPresContext->DeviceContext(),
      surface, aRect, getter_AddRefs(localcx));
  if (NS_FAILED(rv)) {
    tmpContext->DestroyDrawingSurface(surface);
    return NS_ERROR_FAILURE;
  }
  // clipping and translation is set by CreateOffscreenContext

  localcx->SetColor(aBackgroundColor);
  localcx->FillRect(aRect);

  nsIFrame* rootFrame = FrameManager()->GetRootFrame();
  if (!rootFrame) {
    localcx.swap(*aRenderedContext);
    return NS_OK;
  }
  
  nsDisplayListBuilder builder(rootFrame, PR_FALSE, PR_FALSE);
  nsDisplayList list;
  nsIScrollableView* scrollingView = nsnull;
  mViewManager->GetRootScrollableView(&scrollingView);
  nsRect r = aRect;
  if (aIgnoreViewportScrolling && scrollingView) {
    nscoord x, y;
    scrollingView->GetScrollPosition(x, y);
    localcx->Translate(x, y);
    r.MoveBy(-x, -y);
    builder.SetIgnoreScrollFrame(GetRootScrollFrame());
  }

  builder.EnterPresShell(rootFrame, r);

  rv = rootFrame->BuildDisplayListForStackingContext(&builder, r, &list);

  builder.LeavePresShell(rootFrame, r);
  NS_ENSURE_SUCCESS(rv, rv);

  nsRegion region(r);
  list.OptimizeVisibility(&builder, &region);
  list.Paint(&builder, localcx, r);
  // Flush the list so we don't trigger the IsEmpty-on-destruction assertion
  list.DeleteAll();

  localcx.swap(*aRenderedContext);
  return NS_OK;
}

NS_IMETHODIMP
PresShell::Paint(nsIView*             aView,
                 nsIRenderingContext* aRenderingContext,
                 const nsRegion&      aDirtyRegion)
{
  AUTO_LAYOUT_PHASE_ENTRY_POINT(GetPresContext(), Paint);
  nsIFrame* frame;
  nsresult  rv = NS_OK;

  if (mIsDestroying) {
    NS_ASSERTION(PR_FALSE, "A paint message was dispatched to a destroyed PresShell");
    return NS_OK;
  }

  NS_ASSERTION(!(nsnull == aView), "null view");

  frame = NS_STATIC_CAST(nsIFrame*, aView->GetClientData());
  nscolor backgroundColor;
  mViewManager->GetDefaultBackgroundColor(&backgroundColor);
  nsIView* rootView;
  mViewManager->GetRootView(rootView);
  if (!rootView->GetParent() && rootView->HasWidget()) {
    PRBool widgetIsTranslucent;
    rootView->GetWidget()->GetWindowTranslucency(widgetIsTranslucent);
    if (widgetIsTranslucent) {
      backgroundColor = NS_RGBA(0,0,0,0);
    }
  }
  
  if (!frame) {
    if (NS_GET_A(backgroundColor) > 0) {
      aRenderingContext->SetColor(backgroundColor);
      aRenderingContext->FillRect(aDirtyRegion.GetBounds());
    }
    return NS_OK;
  }

  nsLayoutUtils::PaintFrame(aRenderingContext, frame, aDirtyRegion,
                            backgroundColor);

#ifdef NS_DEBUG
  // Draw a border around the frame
  if (nsIFrameDebug::GetShowFrameBorders()) {
    nsRect r = frame->GetRect();
    aRenderingContext->SetColor(NS_RGB(0,0,255));
    aRenderingContext->DrawRect(0, 0, r.width, r.height);
  }
  // Draw a border around the current event target
  if ((nsIFrameDebug::GetShowEventTargetFrameBorder()) && (aView == mCurrentTargetView)) {
    aRenderingContext->SetColor(NS_RGB(128,0,128));
    aRenderingContext->DrawRect(mCurrentTargetRect.x, mCurrentTargetRect.y, mCurrentTargetRect.width, mCurrentTargetRect.height);
  }
#endif

  return rv;
}

nsIFrame*
PresShell::GetCurrentEventFrame()
{
  if (!mCurrentEventFrame && mCurrentEventContent) {
    // Make sure the content still has a document reference. If not,
    // then we assume it is no longer in the content tree and the
    // frame shouldn't get an event, nor should we even assume its
    // safe to try and find the frame.
    if (mCurrentEventContent->GetDocument()) {
      mCurrentEventFrame = GetPrimaryFrameFor(mCurrentEventContent);
    }
  }

  return mCurrentEventFrame;
}

NS_IMETHODIMP
PresShell::GetEventTargetFrame(nsIFrame** aFrame)
{
  *aFrame = GetCurrentEventFrame();
  return NS_OK;
}

NS_IMETHODIMP
PresShell::GetEventTargetContent(nsEvent* aEvent, nsIContent** aContent)
{
  if (mCurrentEventContent) {
    *aContent = mCurrentEventContent;
    NS_IF_ADDREF(*aContent);
  } else {
    nsIFrame* currentEventFrame = GetCurrentEventFrame();
    if (currentEventFrame) {
      currentEventFrame->GetContentForEvent(mPresContext, aEvent, aContent);
    } else {
      *aContent = nsnull;
    }
  }
  return NS_OK;
}

void
PresShell::PushCurrentEventInfo(nsIFrame* aFrame, nsIContent* aContent)
{
  if (mCurrentEventFrame || mCurrentEventContent) {
    mCurrentEventFrameStack.InsertElementAt((void*)mCurrentEventFrame, 0);
    mCurrentEventContentStack.InsertObjectAt(mCurrentEventContent, 0);
  }
  mCurrentEventFrame = aFrame;
  mCurrentEventContent = aContent;
}

void
PresShell::PopCurrentEventInfo()
{
  mCurrentEventFrame = nsnull;
  mCurrentEventContent = nsnull;

  if (0 != mCurrentEventFrameStack.Count()) {
    mCurrentEventFrame = (nsIFrame*)mCurrentEventFrameStack.ElementAt(0);
    mCurrentEventFrameStack.RemoveElementAt(0);
    mCurrentEventContent = mCurrentEventContentStack.ObjectAt(0);
    mCurrentEventContentStack.RemoveObjectAt(0);
  }
}

PRBool PresShell::InZombieDocument(nsIContent *aContent)
{
  // If a content node points to a null document, or the document is not
  // attached to a window, then it is possibly in a zombie document,
  // about to be replaced by a newly loading document.
  // Such documents cannot handle DOM events.
  // It might actually be in a node not attached to any document,
  // in which case there is not parent presshell to retarget it to.
  nsIDocument *doc = aContent->GetDocument();
  return !doc || !doc->GetWindow();
}

nsresult PresShell::RetargetEventToParent(nsIView         *aView,
                                          nsGUIEvent*     aEvent,
                                          nsEventStatus*  aEventStatus,
                                          nsIContent*     aZombieFocusedContent)
{
  // Send this events straight up to the parent pres shell.
  // We do this for non-mouse events in zombie documents.
  // That way at least the UI key bindings can work.

  // First, eliminate the focus ring in the current docshell, which 
  // is  now a zombie. If we key navigate, it won't be within this
  // docshell, until the newly loading document is displayed.

  nsCOMPtr<nsIPresShell> kungFuDeathGrip(this);
  // hold a reference to the ESM across event dispatch
  nsCOMPtr<nsIEventStateManager> esm = mPresContext->EventStateManager();

  esm->SetContentState(nsnull, NS_EVENT_STATE_FOCUS);
  esm->SetFocusedContent(nsnull);
  ContentStatesChanged(mDocument, aZombieFocusedContent,
                       nsnull, NS_EVENT_STATE_FOCUS);

  // Next, update the display so the old focus ring is no longer visible

  nsCOMPtr<nsISupports> container = mPresContext->GetContainer();
  if (!container)
    container = do_QueryReferent(mForwardingContainer);

  nsCOMPtr<nsIDocShell> docShell(do_QueryInterface(container));
  if (!docShell) {
    // We don't have any place to send this event.
    NS_WARNING("dropping event, no forwarding shell");
    return NS_OK;
  }

  nsCOMPtr<nsIContentViewer> contentViewer;
  docShell->GetContentViewer(getter_AddRefs(contentViewer));
  if (contentViewer) {
    nsCOMPtr<nsIContentViewer> zombieViewer;
    contentViewer->GetPreviousViewer(getter_AddRefs(zombieViewer));
    if (zombieViewer) {
      zombieViewer->Show();
    }
  }

  // Now, find the parent pres shell and send the event there
  nsCOMPtr<nsIDocShellTreeItem> treeItem = 
    do_QueryInterface(container);
  NS_ASSERTION(treeItem, "No tree item for container.");
  nsCOMPtr<nsIDocShellTreeItem> parentTreeItem;
  treeItem->GetParent(getter_AddRefs(parentTreeItem));
  nsCOMPtr<nsIDocShell> parentDocShell = 
    do_QueryInterface(parentTreeItem);
  if (!parentDocShell || treeItem == parentTreeItem) {
    return NS_ERROR_FAILURE;
  }

  nsCOMPtr<nsIPresShell> parentPresShell;
  parentDocShell->GetPresShell(getter_AddRefs(parentPresShell));
  nsCOMPtr<nsIViewObserver> parentViewObserver = 
    do_QueryInterface(parentPresShell);
  if (!parentViewObserver) {
    return NS_ERROR_FAILURE;
  }

  PopCurrentEventInfo();
  return parentViewObserver->HandleEvent(aView, aEvent, 
                                         aEventStatus);
}

NS_IMETHODIMP
PresShell::HandleEvent(nsIView         *aView,
                       nsGUIEvent*     aEvent,
                       nsEventStatus*  aEventStatus)
{
  NS_ASSERTION(aView, "null view");

  if (mIsDestroying || mIsReflowing || mChangeNestCount) {
    return NS_OK;
  }

#ifdef ACCESSIBILITY
  if (aEvent->eventStructType == NS_ACCESSIBLE_EVENT) {
    return HandleEventInternal(aEvent, aView, aEventStatus);
  }
#endif

  // Check for a theme change up front, since the frame type is irrelevant
  if (aEvent->message == NS_THEMECHANGED && mPresContext) {
    mPresContext->ThemeChanged();
    return NS_OK;
  }

  // Check for a system color change up front, since the frame type is
  // irrelevant
  if ((aEvent->message == NS_SYSCOLORCHANGED) && mPresContext) {
    nsIViewManager* vm = GetViewManager();
    if (vm) {
      // Only dispatch system color change when the message originates from
      // from the root views widget. This is necessary to prevent us from 
      // dispatching the SysColorChanged notification for each child window 
      // which may be redundant.
      nsIView *view;
      vm->GetRootView(view);
      if (view == aView) {
        *aEventStatus = nsEventStatus_eConsumeDoDefault;
        mPresContext->SysColorChanged();
        return NS_OK;
      }
    }
    return NS_OK;
  }
  
  nsIFrame* frame = NS_STATIC_CAST(nsIFrame*, aView->GetClientData());

  PRBool dispatchUsingCoordinates =
      !NS_IS_KEY_EVENT(aEvent) && !NS_IS_IME_EVENT(aEvent) &&
         aEvent->message != NS_CONTEXTMENU_KEY && !NS_IS_FOCUS_EVENT(aEvent);

  // if this event has no frame, we need to retarget it at a parent
  // view that has a frame.
  if (!frame &&
      (dispatchUsingCoordinates || NS_IS_KEY_EVENT(aEvent) ||
       NS_IS_IME_EVENT(aEvent))) {
    nsIView* targetView = aView;
    while (targetView && !targetView->GetClientData()) {
      targetView = targetView->GetParent();
    }
    
    if (targetView) {
      aView = targetView;
      frame = NS_STATIC_CAST(nsIFrame*, aView->GetClientData());
    }
  }

  if (dispatchUsingCoordinates) {
    NS_ASSERTION(frame, "Nothing to handle this event!");
    if (!frame)
      return NS_OK;
      
    nsPoint eventPoint
        = nsLayoutUtils::GetEventCoordinatesRelativeTo(aEvent, frame);
    nsIFrame* targetFrame = nsLayoutUtils::GetFrameForPoint(frame, eventPoint);
    if (targetFrame) {
      PresShell* shell =
          NS_STATIC_CAST(PresShell*, targetFrame->GetPresContext()->PresShell());
      if (shell != this) {
        // Handle the event in the correct shell.
        // Prevent deletion until we're done with event handling (bug 336582).
        nsRefPtr<nsIPresShell> kungFuDeathGrip(shell);
        nsIView* subshellRootView;
        shell->GetViewManager()->GetRootView(subshellRootView);
        // We pass the subshell's root view as the view to start from. This is
        // the only correct alternative; if the event was captured then it
        // must have been captured by us or some ancestor shell and we
        // now ask the subshell to dispatch it normally.
        return shell->HandlePositionedEvent(subshellRootView, targetFrame,
                                            aEvent, aEventStatus);
      }
    }
    
    if (!targetFrame) {
      targetFrame = frame;
    }
    return HandlePositionedEvent(aView, targetFrame, aEvent, aEventStatus);
  }
  
  nsresult rv = NS_OK;
  
  if (frame) {
    PushCurrentEventInfo(nsnull, nsnull);

    // key and IME events go to the focused frame
    nsIEventStateManager *esm = mPresContext->EventStateManager();

    if (NS_IS_KEY_EVENT(aEvent) || NS_IS_IME_EVENT(aEvent) ||
        aEvent->message == NS_CONTEXTMENU_KEY) {
      esm->GetFocusedFrame(&mCurrentEventFrame);
      if (mCurrentEventFrame) {
        esm->GetFocusedContent(getter_AddRefs(mCurrentEventContent));
      } else {
#if defined(MOZ_X11) || defined(XP_WIN)
#if defined(MOZ_X11)
        if (NS_IS_IME_EVENT(aEvent)) {
          // bug 52416 (MOZ_X11)
          // Lookup region (candidate window) of UNIX IME grabs
          // input focus from Mozilla but wants to send IME event
          // to redraw pre-edit (composed) string
          // If Mozilla does not have input focus and event is IME,
          // sends IME event to pre-focused element
#else
        if (NS_IS_KEY_EVENT(aEvent) || NS_IS_IME_EVENT(aEvent)) {
          // bug 292263 (XP_WIN)
          // If software keyboard has focus, it may send the key messages and
          // the IME messages to pre-focused window. Therefore, if Mozilla
          // doesn't have focus and event is key event or IME event, we should
          // send the events to pre-focused element.
#endif /* defined(MOZ_X11) */
          nsPIDOMWindow *ourWindow = mDocument->GetWindow();
          if (ourWindow) {
            nsIFocusController *focusController =
              ourWindow->GetRootFocusController();
            if (focusController) {
              PRBool active = PR_FALSE;
              // check input focus is in Mozilla
              focusController->GetActive(&active);
              if (!active) {
                // if not, search for pre-focused element
                nsCOMPtr<nsIDOMElement> focusedElement;
                focusController->GetFocusedElement(getter_AddRefs(focusedElement));
                if (focusedElement) {
                  // get mCurrentEventContent from focusedElement
                  mCurrentEventContent = do_QueryInterface(focusedElement);
                }
              }
            }
          }
        }
#endif /* defined(MOZ_X11) || defined(XP_WIN) */
        if (!mCurrentEventContent) {
          mCurrentEventContent = mDocument->GetRootContent();
        }
        mCurrentEventFrame = nsnull; // XXXldb Isn't it already?
      }
      if (mCurrentEventContent && InZombieDocument(mCurrentEventContent)) {
        return RetargetEventToParent(aView, aEvent, aEventStatus,
                                     mCurrentEventContent);
      }
    } else {
      mCurrentEventFrame = frame;
    }
    if (GetCurrentEventFrame()) {
      rv = HandleEventInternal(aEvent, aView, aEventStatus);
    }
  
#ifdef NS_DEBUG
    ShowEventTargetDebug();
#endif
    PopCurrentEventInfo();
  } else {
    // Focus events need to be dispatched even if no frame was found, since
    // we don't want the focus controller to be out of sync.

    if (!NS_EVENT_NEEDS_FRAME(aEvent)) {
      mCurrentEventFrame = nsnull;
      return HandleEventInternal(aEvent, aView, aEventStatus);
    }
    else if (NS_IS_KEY_EVENT(aEvent)) {
      // Keypress events in new blank tabs should not be completely thrown away.
      // Retarget them -- the parent chrome shell might make use of them.
      return RetargetEventToParent(aView, aEvent, aEventStatus,
                                   mCurrentEventContent);
    }
  }

  return rv;
}

#ifdef NS_DEBUG
void
PresShell::ShowEventTargetDebug()
{
  if ((nsIFrameDebug::GetShowEventTargetFrameBorder()) &&
      (GetCurrentEventFrame())) {
    nsIView *oldView = mCurrentTargetView;
    nsPoint offset(0,0);
    nsRect oldTargetRect(mCurrentTargetRect);
    mCurrentTargetRect = mCurrentEventFrame->GetRect();
    mCurrentTargetView = mCurrentEventFrame->GetView();
    if (!mCurrentTargetView ) {
      mCurrentEventFrame->GetOffsetFromView(offset, &mCurrentTargetView);
    }
    if (mCurrentTargetView) {
      mCurrentTargetRect.x = offset.x;
      mCurrentTargetRect.y = offset.y;
      // use aView or mCurrentTargetView??
      if ((mCurrentTargetRect != oldTargetRect) ||
          (mCurrentTargetView != oldView)) {

        nsIViewManager* vm = GetViewManager();
        if (vm) {
          vm->UpdateView(mCurrentTargetView,mCurrentTargetRect,0);
          if (oldView)
            vm->UpdateView(oldView,oldTargetRect,0);
        }
      }
    }
  }
}
#endif

nsresult
PresShell::HandlePositionedEvent(nsIView*       aView,
                                 nsIFrame*      aTargetFrame,
                                 nsGUIEvent*    aEvent,
                                 nsEventStatus* aEventStatus)
{
  nsresult rv = NS_OK;
  
  PushCurrentEventInfo(nsnull, nsnull);
  
  mCurrentEventFrame = aTargetFrame;

  if (mCurrentEventFrame) {
    nsCOMPtr<nsIContent> targetElement;
    mCurrentEventFrame->GetContentForEvent(mPresContext, aEvent,
                                           getter_AddRefs(targetElement));

    // If there is no content for this frame, target it anyway.  Some
    // frames can be targeted but do not have content, particularly
    // windows with scrolling off.
    if (targetElement) {
      // Bug 103055, bug 185889: mouse events apply to *elements*, not all
      // nodes.  Thus we get the nearest element parent here.
      // XXX we leave the frame the same even if we find an element
      // parent, so that the text frame will receive the event (selection
      // and friends are the ones who care about that anyway)
      //
      // We use weak pointers because during this tight loop, the node
      // will *not* go away.  And this happens on every mousemove.
      while (targetElement &&
             !targetElement->IsNodeOfType(nsINode::eELEMENT)) {
        targetElement = targetElement->GetParent();
      }

      // If we found an element, target it.  Otherwise, target *nothing*.
      if (!targetElement) {
        mCurrentEventContent = nsnull;
        mCurrentEventFrame = nsnull;
      } else if (targetElement != mCurrentEventContent) {
        mCurrentEventContent = targetElement;
      }
    }
  }

  if (GetCurrentEventFrame()) {
    rv = HandleEventInternal(aEvent, aView, aEventStatus);
  }

#ifdef NS_DEBUG
  ShowEventTargetDebug();
#endif
  PopCurrentEventInfo();
  return rv;
}

NS_IMETHODIMP
PresShell::HandleEventWithTarget(nsEvent* aEvent, nsIFrame* aFrame,
                                 nsIContent* aContent, nsEventStatus* aStatus)
{
  nsresult ret;

  PushCurrentEventInfo(aFrame, aContent);
  ret = HandleEventInternal(aEvent, nsnull, aStatus);
  PopCurrentEventInfo();
  return NS_OK;
}

inline PRBool
IsSynthesizedMouseMove(nsEvent* aEvent)
{
  return aEvent->eventStructType == NS_MOUSE_EVENT &&
         NS_STATIC_CAST(nsMouseEvent*, aEvent)->reason != nsMouseEvent::eReal;
}

nsresult
PresShell::HandleEventInternal(nsEvent* aEvent, nsIView *aView,
                               nsEventStatus* aStatus)
{
#ifdef ACCESSIBILITY
  if (aEvent->eventStructType == NS_ACCESSIBLE_EVENT)
  {
    NS_STATIC_CAST(nsAccessibleEvent*, aEvent)->accessible = nsnull;
    nsCOMPtr<nsIAccessibilityService> accService = 
      do_GetService("@mozilla.org/accessibilityService;1");
    if (accService) {
      nsCOMPtr<nsISupports> container = mPresContext->GetContainer();
      if (!container) {
        // This presshell is not active. This often happens when a
        // preshell is being held onto for fastback.
        return NS_OK;
      }
      nsIAccessible* acc;
      nsCOMPtr<nsIDOMNode> domNode(do_QueryInterface(mDocument));
      NS_ASSERTION(domNode, "No dom node for doc");
      accService->GetAccessibleInShell(domNode, this, &acc);
      // Addref this - it's not a COM Ptr
      // We'll make sure the right number of Addref's occur before
      // handing this back to the accessibility client
      NS_STATIC_CAST(nsAccessibleEvent*, aEvent)->accessible = acc;
      mIsAccessibilityActive = PR_TRUE;
      return NS_OK;
    }
  }
#endif

  nsCOMPtr<nsIEventStateManager> manager = mPresContext->EventStateManager();
  nsresult rv = NS_OK;

  // Grab the offset of aView from our root view.  We can use this to adjust
  // coordinates as needed.  Note that we can't just adjust them later, because
  // event dispatch can destroy aView.  Also note that at times aView is null
  // here.  When that happens, there isn't much we can do about getting
  // coordinates right....
  nsPoint offsetOfaView(0,0);
  if (aView) {
    nsIView* rootView;
    mViewManager->GetRootView(rootView);
    offsetOfaView = aView->GetOffsetTo(rootView);
  }

  if (!NS_EVENT_NEEDS_FRAME(aEvent) || GetCurrentEventFrame()) {
    PRBool isHandlingUserInput = PR_FALSE;

    if (NS_IS_TRUSTED_EVENT(aEvent)) {
      switch (aEvent->message) {
      case NS_GOTFOCUS:
      case NS_LOSTFOCUS:
      case NS_ACTIVATE:
      case NS_DEACTIVATE:
        // Treat focus/blur events as user input if they happen while
        // executing trusted script, or no script at all. If they
        // happen during execution of non-trusted script, then they
        // should not be considered user input.
        if (!nsContentUtils::IsCallerChrome()) {
          break;
        }
      case NS_MOUSE_LEFT_BUTTON_DOWN:
      case NS_MOUSE_MIDDLE_BUTTON_DOWN:
      case NS_MOUSE_RIGHT_BUTTON_DOWN:
      case NS_MOUSE_LEFT_BUTTON_UP:
      case NS_MOUSE_RIGHT_BUTTON_UP:
      case NS_MOUSE_MIDDLE_BUTTON_UP:
      case NS_KEY_PRESS:
      case NS_KEY_DOWN:
      case NS_KEY_UP:
        isHandlingUserInput = PR_TRUE;
      }
    }

    nsAutoHandlingUserInputStatePusher userInpStatePusher(isHandlingUserInput);

    nsAutoPopupStatePusher popupStatePusher(nsDOMEvent::GetEventPopupControlState(aEvent));

    // FIXME. If the event was reused, we need to clear the old target,
    // bug 329430
    aEvent->target = nsnull;

    // 1. Give event to event manager for pre event state changes and
    //    generation of synthetic events.
    rv = manager->PreHandleEvent(mPresContext, aEvent, mCurrentEventFrame,
                                 aStatus, aView);

    // 2. Give event to the DOM for third party and JS use.
    if ((GetCurrentEventFrame()) && NS_SUCCEEDED(rv)) {
      // We want synthesized mouse moves to cause mouseover and mouseout
      // DOM events (PreHandleEvent above), but not mousemove DOM events.
      if (!IsSynthesizedMouseMove(aEvent)) {
        nsPresShellEventCB eventCB(this);
        if (mCurrentEventContent) {
          nsEventDispatcher::Dispatch(mCurrentEventContent, mPresContext,
                                      aEvent, nsnull, aStatus, &eventCB);
        }
        else {
          nsCOMPtr<nsIContent> targetContent;
          rv = mCurrentEventFrame->GetContentForEvent(mPresContext, aEvent,
                                                      getter_AddRefs(targetContent));
          if (NS_SUCCEEDED(rv) && targetContent) {
            nsEventDispatcher::Dispatch(targetContent, mPresContext, aEvent,
                                        nsnull, aStatus, &eventCB);
          }
        }
      }

      // 3. Give event to event manager for post event state changes and
      //    generation of synthetic events.
      if (NS_SUCCEEDED (rv) &&
          (GetCurrentEventFrame() || !NS_EVENT_NEEDS_FRAME(aEvent))) {
        rv = manager->PostHandleEvent(mPresContext, aEvent, mCurrentEventFrame,
                                      aStatus, aView);
      }
    }
  }
  return rv;
}

// Dispatch event to content only (NOT full processing)
// See also HandleEventWithTarget which does full event processing.
NS_IMETHODIMP
PresShell::HandleDOMEventWithTarget(nsIContent* aTargetContent, nsEvent* aEvent,
                                    nsEventStatus* aStatus)
{
  PushCurrentEventInfo(nsnull, aTargetContent);

  // Bug 41013: Check if the event should be dispatched to content.
  // It's possible that we are in the middle of destroying the window
  // and the js context is out of date. This check detects the case
  // that caused a crash in bug 41013, but there may be a better way
  // to handle this situation!
  nsCOMPtr<nsISupports> container = mPresContext->GetContainer();
  if (container) {

    // Dispatch event to content
    nsEventDispatcher::Dispatch(aTargetContent, mPresContext, aEvent, nsnull,
                                aStatus);
  }

  PopCurrentEventInfo();
  return NS_OK;
}

NS_IMETHODIMP
PresShell::ResizeReflow(nsIView *aView, nscoord aWidth, nscoord aHeight)
{
  return ResizeReflow(aWidth, aHeight);
}

NS_IMETHODIMP_(PRBool)
PresShell::IsVisible()
{
  nsCOMPtr<nsISupports> container = mPresContext->GetContainer();
  nsCOMPtr<nsIBaseWindow> bw = do_QueryInterface(container);
  if (!bw)
    return PR_FALSE;
  PRBool res = PR_TRUE;
  bw->GetVisibility(&res);
  return res;
}

NS_IMETHODIMP_(void)
PresShell::WillPaint()
{
  // Don't reenter reflow and don't reflow during frame construction.  Also
  // don't bother reflowing if some viewmanager in our tree is painting while
  // we still have painting suppressed.
  if (mIsReflowing || mChangeNestCount || mPaintingSuppressed) {
    return;
  }
  
  // Process reflows, if we have them, to reduce flicker due to invalidates and
  // reflow being interspersed.  Note that we _do_ allow this to be
  // interruptible; if we can't do all the reflows it's better to flicker a bit
  // than to freeze up.
  // XXXbz this update batch may not be strictly necessary, but it's good form.
  // XXXbz should we be flushing out style changes here?  Probably not, I'd say.
  NS_ASSERTION(mViewManager, "Something weird is going on");
  mViewManager->BeginUpdateViewBatch();
  ProcessReflowCommands(PR_TRUE);
  mViewManager->EndUpdateViewBatch(NS_VMREFRESH_NO_SYNC);
}

nsresult
PresShell::GetAgentStyleSheets(nsCOMArray<nsIStyleSheet>& aSheets)
{
  aSheets.Clear();
  PRInt32 sheetCount = mStyleSet->SheetCount(nsStyleSet::eAgentSheet);

  for (PRInt32 i = 0; i < sheetCount; ++i) {
    nsIStyleSheet *sheet = mStyleSet->StyleSheetAt(nsStyleSet::eAgentSheet, i);
    if (!aSheets.AppendObject(sheet))
      return NS_ERROR_OUT_OF_MEMORY;
  }

  return NS_OK;
}

nsresult
PresShell::SetAgentStyleSheets(const nsCOMArray<nsIStyleSheet>& aSheets)
{
  return mStyleSet->ReplaceSheets(nsStyleSet::eAgentSheet, aSheets);
}

nsresult
PresShell::AddOverrideStyleSheet(nsIStyleSheet *aSheet)
{
  return mStyleSet->PrependStyleSheet(nsStyleSet::eOverrideSheet, aSheet);
}

nsresult
PresShell::RemoveOverrideStyleSheet(nsIStyleSheet *aSheet)
{
  return mStyleSet->RemoveStyleSheet(nsStyleSet::eOverrideSheet, aSheet);
}

static void
StopPluginInstance(PresShell *aShell, nsIContent *aContent)
{
  nsIFrame *frame = aShell->FrameManager()->GetPrimaryFrameFor(aContent);

  nsIObjectFrame *objectFrame = nsnull;
  if (frame)
    CallQueryInterface(frame, &objectFrame);
  if (!objectFrame)
    return;

  objectFrame->StopPlugin();
}

PR_STATIC_CALLBACK(PRBool)
FreezeSubDocument(nsIDocument *aDocument, void *aData)
{
  nsIPresShell *shell = aDocument->GetShellAt(0);
  if (shell)
    shell->Freeze();

  return PR_TRUE;
}

void
PresShell::Freeze()
{
  nsCOMPtr<nsIDOMDocument> domDoc = do_QueryInterface(mDocument);
  if (domDoc) {
    EnumeratePlugins(domDoc, NS_LITERAL_STRING("object"), StopPluginInstance);
    EnumeratePlugins(domDoc, NS_LITERAL_STRING("applet"), StopPluginInstance);
    EnumeratePlugins(domDoc, NS_LITERAL_STRING("embed"), StopPluginInstance);
  }

  if (mCaret)
    mCaret->SetCaretVisible(PR_FALSE);

  mPaintingSuppressed = PR_TRUE;

  if (mDocument)
    mDocument->EnumerateSubDocuments(FreezeSubDocument, nsnull);
}

static void
StartPluginInstance(PresShell *aShell, nsIContent *aContent)
{
  nsCOMPtr<nsIObjectLoadingContent> objlc(do_QueryInterface(aContent));
  if (!objlc)
    return;

  nsCOMPtr<nsIPluginInstance> inst;
  objlc->EnsureInstantiation(getter_AddRefs(inst));
}

PR_STATIC_CALLBACK(PRBool)
ThawSubDocument(nsIDocument *aDocument, void *aData)
{
  nsIPresShell *shell = aDocument->GetShellAt(0);
  if (shell)
    shell->Thaw();

  return PR_TRUE;
}

void
PresShell::Thaw()
{
  nsCOMPtr<nsIDOMDocument> domDoc = do_QueryInterface(mDocument);
  if (domDoc) {
    EnumeratePlugins(domDoc, NS_LITERAL_STRING("object"), StartPluginInstance);
    EnumeratePlugins(domDoc, NS_LITERAL_STRING("applet"), StartPluginInstance);
    EnumeratePlugins(domDoc, NS_LITERAL_STRING("embed"), StartPluginInstance);
  }

  if (mDocument)
    mDocument->EnumerateSubDocuments(ThawSubDocument, nsnull);

  UnsuppressPainting();
}

//--------------------------------------------------------
// Start of protected and private methods on the PresShell
//--------------------------------------------------------

//-------------- Begin Reflow Event Definition ------------------------

NS_IMETHODIMP
PresShell::ReflowEvent::Run() {    
  // Take an owning reference to the PresShell during this call to ensure
  // that it doesn't get killed off prematurely.
  nsRefPtr<PresShell> ps = mPresShell;
  if (ps) {
#ifdef DEBUG
    if (VERIFY_REFLOW_NOISY_RC & gVerifyReflowFlags) {
       printf("\n*** Handling reflow event: PresShell=%p, event=%p\n", (void*)ps, (void*)this);
    }
#endif
    // NOTE: the ReflowEvent class is a friend of the PresShell class
    PRBool isBatching;
    ps->ClearReflowEventStatus();
    ps->GetReflowBatchingStatus(&isBatching);
    if (!isBatching) {
      // Set a kung fu death grip on the view manager associated with the pres shell
      // before processing that pres shell's reflow commands.  Fixes bug 54868.
      nsCOMPtr<nsIViewManager> viewManager = ps->GetViewManager();

      viewManager->BeginUpdateViewBatch();
      ps->ProcessReflowCommands(PR_TRUE);
      viewManager->EndUpdateViewBatch(NS_VMREFRESH_NO_SYNC);

      // Now, explicitly release the pres shell before the view manager
      ps = nsnull;
      viewManager = nsnull;
    }
  }
  return NS_OK;
}

//-------------- End Reflow Event Definition ---------------------------

void
PresShell::PostReflowEvent()
{
  if (mReflowEvent.IsPending() || mIsDestroying || mIsReflowing ||
      mReflowCommands.Count() == 0)
    return;

  nsRefPtr<ReflowEvent> ev = new ReflowEvent(this);
  if (NS_FAILED(NS_DispatchToCurrentThread(ev))) {
    NS_WARNING("failed to dispatch reflow event");
  } else {
    mReflowEvent = ev;
#ifdef DEBUG
    if (VERIFY_REFLOW_NOISY_RC & gVerifyReflowFlags) {
      printf("\n*** PresShell::PostReflowEvent(), this=%p, event=%p\n", (void*)this, (void*)ev);
    }
#endif    
  }
}

nsresult
PresShell::DidCauseReflow()
{
  NS_ASSERTION(mChangeNestCount != 0, "Unexpected call to DidCauseReflow()");
  if (--mChangeNestCount == 0) {
    // We may have had more reflow commands appended to the queue during
    // our reflow.  Make sure these get processed at some point.
    PostReflowEvent();
  }

  return NS_OK;
}

void
PresShell::WillDoReflow()
{
  // We just reflowed, tell the caret that its frame might have moved.
  if (mCaret) {
    mCaret->UpdateCaretPosition();
    mCaret->InvalidateOutsideCaret();
  }
}

void
PresShell::DidDoReflow()
{
  HandlePostedDOMEvents();
  HandlePostedAttributeChanges();
  HandlePostedReflowCallbacks();
  // Null-check mViewManager in case this happens during Destroy.  See
  // bugs 244435 and 238546.
  if (!mPaintingSuppressed && mViewManager)
    mViewManager->SynthesizeMouseMove(PR_FALSE);
  if (mCaret)
    mCaret->InvalidateOutsideCaret();
}

nsresult
PresShell::ProcessReflowCommands(PRBool aInterruptible)
{
  MOZ_TIMER_DEBUGLOG(("Start: Reflow: PresShell::ProcessReflowCommands(), this=%p\n", this));
  MOZ_TIMER_START(mReflowWatch);  

  if (0 != mReflowCommands.Count()) {
    nsHTMLReflowMetrics   desiredSize(nsnull);
    nsIRenderingContext*  rcx;
    nsIFrame*             rootFrame = FrameManager()->GetRootFrame();
    nsSize          maxSize = rootFrame->GetSize();

    nsresult rv=CreateRenderingContext(rootFrame, &rcx);
    if (NS_FAILED(rv)) return rv;

#ifdef DEBUG
    if (GetVerifyReflowEnable()) {
      if (VERIFY_REFLOW_ALL & gVerifyReflowFlags) {
        printf("ProcessReflowCommands: begin incremental reflow\n");
      }
    }
    if (VERIFY_REFLOW_DUMP_COMMANDS & gVerifyReflowFlags) {   
      PRInt32 i, n = mReflowCommands.Count();
      printf("\nPresShell::ProcessReflowCommands: this=%p, count=%d\n", (void*)this, n);
      for (i = 0; i < n; i++) {
        nsHTMLReflowCommand* rc = (nsHTMLReflowCommand*)
          mReflowCommands.ElementAt(i);
        rc->List(stdout);
      }
    }
#endif

    WillDoReflow();

    // If reflow is interruptible, then make a note of our deadline.
    const PRIntervalTime deadline = aInterruptible
        ? PR_IntervalNow() + PR_MicrosecondsToInterval(gMaxRCProcessingTime)
        : (PRIntervalTime)0;

    // force flushing of any pending notifications
    mDocument->BeginUpdate(UPDATE_ALL);
    mDocument->EndUpdate(UPDATE_ALL);

    // Scope for the reflow entry point
    {
      AUTO_LAYOUT_PHASE_ENTRY_POINT(GetPresContext(), Reflow);
      mIsReflowing = PR_TRUE;

      do {
        // Coalesce the reflow commands into a tree.
        IncrementalReflow reflow;
        for (PRInt32 i = mReflowCommands.Count() - 1; i >= 0; --i) {
          nsHTMLReflowCommand *command =
            NS_STATIC_CAST(nsHTMLReflowCommand *, mReflowCommands[i]);

          IncrementalReflow::AddCommandResult res =
            reflow.AddCommand(mPresContext, command);
          if (res == IncrementalReflow::eEnqueued ||
              res == IncrementalReflow::eCancel) {
            // Remove the command from the queue.
            mReflowCommands.RemoveElementAt(i);
            ReflowCommandRemoved(command);
            if (res == IncrementalReflow::eCancel)
              delete command;
          }
          else {
            // The reflow command couldn't be added to the tree; leave
            // it in the queue, and we'll handle it next time.
#ifdef DEBUG
            printf("WARNING: Couldn't add reflow command, so splitting.\n");
#endif
          }
        }

#ifdef DEBUG
        if (VERIFY_REFLOW_NOISY_RC & gVerifyReflowFlags) {
          printf("Incremental reflow tree:\n");
          reflow.Dump(mPresContext);
        }
#endif

        // Dispatch an incremental reflow.
        reflow.Dispatch(mPresContext, desiredSize, maxSize, *rcx);

        // Keep going until we're out of reflow commands, or we've run
        // past our deadline.
      } while (mReflowCommands.Count() &&
               (!aInterruptible || PR_IntervalNow() < deadline));

      // XXXwaterson for interruptible reflow, examine the tree and
      // re-enqueue any unflowed reflow targets.

      mIsReflowing = PR_FALSE;
    }

    NS_IF_RELEASE(rcx);

    // If any new reflow commands were enqueued during the reflow,
    // schedule another reflow event to process them.
    if (mReflowCommands.Count())
      PostReflowEvent();
    
#ifdef DEBUG
    if (VERIFY_REFLOW_DUMP_COMMANDS & gVerifyReflowFlags) {
      printf("\nPresShell::ProcessReflowCommands() finished: this=%p\n", (void*)this);
    }

    if (nsIFrameDebug::GetVerifyTreeEnable()) {
      nsIFrameDebug*  frameDebug;

      if (NS_SUCCEEDED(rootFrame->QueryInterface(NS_GET_IID(nsIFrameDebug),
                                                 (void**)&frameDebug))) {
        frameDebug->VerifyTree();
      }
    }
    if (GetVerifyReflowEnable()) {
      // First synchronously render what we have so far so that we can
      // see it.
      nsIView* rootView;
      mViewManager->GetRootView(rootView);
      mViewManager->UpdateView(rootView, NS_VMREFRESH_IMMEDIATE);

      mInVerifyReflow = PR_TRUE;
      PRBool ok = VerifyIncrementalReflow();
      mInVerifyReflow = PR_FALSE;
      if (VERIFY_REFLOW_ALL & gVerifyReflowFlags) {
        printf("ProcessReflowCommands: finished (%s)\n",
               ok ? "ok" : "failed");
      }

      if (0 != mReflowCommands.Count()) {
        printf("XXX yikes! reflow commands queued during verify-reflow\n");
      }
    }
#endif

    // If there are no more reflow commands in the queue, we'll want
    // to unblock onload.
    DoneRemovingReflowCommands();

    DidDoReflow();
  }
  
  MOZ_TIMER_DEBUGLOG(("Stop: Reflow: PresShell::ProcessReflowCommands(), this=%p\n", this));
  MOZ_TIMER_STOP(mReflowWatch);  

  if (mShouldUnsuppressPainting && mReflowCommands.Count() == 0) {
    // We only unlock if we're out of reflows.  It's pointless
    // to unlock if reflows are still pending, since reflows
    // are just going to thrash the frames around some more.  By
    // waiting we avoid an overeager "jitter" effect.
    mShouldUnsuppressPainting = PR_FALSE;
    UnsuppressAndInvalidate();
  }

  return NS_OK;
}

void
PresShell::ClearReflowEventStatus()
{
  mReflowEvent.Forget();
}

nsresult
PresShell::ReflowCommandAdded(nsHTMLReflowCommand* aRC)
{

  NS_PRECONDITION(mRCCreatedDuringLoad >= 0, "PresShell's reflow command queue is in a bad state.");
  if (mDocumentLoading) {
    aRC->AddFlagBits(NS_RC_CREATED_DURING_DOCUMENT_LOAD);
    mRCCreatedDuringLoad++;

#ifdef PR_LOGGING
    if (PR_LOG_TEST(gLog, PR_LOG_DEBUG)) {
      nsIFrame* target;
      aRC->GetTarget(target);

      nsIAtom* type = target->GetType();
      NS_ASSERTION(type, "frame didn't override GetType()");

      nsAutoString typeStr(NS_LITERAL_STRING("unknown"));
      if (type)
        type->ToString(typeStr);

      PR_LOG(gLog, PR_LOG_DEBUG,
             ("presshell=%p, ReflowCommandAdded(%p) target=%p[%s] mRCCreatedDuringLoad=%d\n",
              this, aRC, target, NS_ConvertUTF16toUTF8(typeStr).get(), mRCCreatedDuringLoad));
    }
#endif

    if (!mDocumentOnloadBlocked) {
      mDocument->BlockOnload();
      mDocumentOnloadBlocked = PR_TRUE;
    }
  }
  return NS_OK;
}

nsresult
PresShell::ReflowCommandRemoved(nsHTMLReflowCommand* aRC)
{
  NS_PRECONDITION(mReflowCommandTable.ops, "How did that happen?");
  
  PL_DHashTableOperate(&mReflowCommandTable, aRC, PL_DHASH_REMOVE);
  
  NS_PRECONDITION(mRCCreatedDuringLoad >= 0, "PresShell's reflow command queue is in a bad state.");  
  if (aRC->GetFlagBits() & NS_RC_CREATED_DURING_DOCUMENT_LOAD) {
    mRCCreatedDuringLoad--;

    PR_LOG(gLog, PR_LOG_DEBUG,
           ("presshell=%p, ReflowCommandRemoved(%p) mRCCreatedDuringLoad=%d\n",
            this, aRC, mRCCreatedDuringLoad));
  }
  return NS_OK;
}

void
PresShell::DoneRemovingReflowCommands()
{
  // We want to unblock here even if we're destroying, since onload
  // can well fire with no presentation in sight.  So just check
  // whether we actually blocked onload.
  if (mRCCreatedDuringLoad == 0 && mDocumentOnloadBlocked) {
    mDocument->UnblockOnload(PR_FALSE);
    mDocumentOnloadBlocked = PR_FALSE;
  }
}

#ifdef MOZ_XUL
/*
 * It's better to add stuff to the |DidSetStyleContext| method of the
 * relevant frames than adding it here.  These methods should (ideally,
 * anyway) go away.
 */

// Return value says whether to walk children.
typedef PRBool (* PR_CALLBACK frameWalkerFn)(nsIFrame *aFrame, void *aClosure);
   
PR_STATIC_CALLBACK(PRBool)
ReResolveMenusAndTrees(nsIFrame *aFrame, void *aClosure)
{
  // Trees have a special style cache that needs to be flushed when
  // the theme changes.
  nsCOMPtr<nsITreeBoxObject> treeBox(do_QueryInterface(aFrame));
  if (treeBox)
    treeBox->ClearStyleAndImageCaches();

  // We deliberately don't re-resolve style on a menu's popup
  // sub-content, since doing so slows menus to a crawl.  That means we
  // have to special-case them on a skin switch, and ensure that the
  // popup frames just get destroyed completely.
  nsCOMPtr<nsIMenuFrame> menuFrame(do_QueryInterface(aFrame));
  if (menuFrame) {
    menuFrame->UngenerateMenu();  
    menuFrame->OpenMenu(PR_FALSE);
  }
  return PR_TRUE;
}

PR_STATIC_CALLBACK(PRBool)
ReframeImageBoxes(nsIFrame *aFrame, void *aClosure)
{
  nsStyleChangeList *list = NS_STATIC_CAST(nsStyleChangeList*, aClosure);
  if (aFrame->GetType() == nsLayoutAtoms::imageBoxFrame) {
    list->AppendChange(aFrame, aFrame->GetContent(),
                       NS_STYLE_HINT_FRAMECHANGE);
    return PR_FALSE; // don't walk descendants
  }
  return PR_TRUE; // walk descendants
}

static void
WalkFramesThroughPlaceholders(nsPresContext *aPresContext, nsIFrame *aFrame,
                              frameWalkerFn aFunc, void *aClosure)
{
  PRBool walkChildren = (*aFunc)(aFrame, aClosure);
  if (!walkChildren)
    return;

  PRInt32 listIndex = 0;
  nsIAtom* childList = nsnull;

  do {
    nsIFrame *child = aFrame->GetFirstChild(childList);
    while (child) {
      if (!(child->GetStateBits() & NS_FRAME_OUT_OF_FLOW)) {
        // only do frames that are in flow, and recur through the
        // out-of-flows of placeholders.
        WalkFramesThroughPlaceholders(aPresContext,
                                      nsPlaceholderFrame::GetRealFrameFor(child),
                                      aFunc, aClosure);
      }
      child = child->GetNextSibling();
    }

    childList = aFrame->GetAdditionalChildListName(listIndex++);
  } while (childList);
}
#endif

NS_IMETHODIMP
PresShell::Observe(nsISupports* aSubject, 
                   const char* aTopic,
                   const PRUnichar* aData)
{
#ifdef MOZ_XUL
  if (!nsCRT::strcmp(aTopic, "chrome-flush-skin-caches")) {
    nsIFrame *rootFrame = FrameManager()->GetRootFrame();
    // Need to null-check because "chrome-flush-skin-caches" can happen
    // at interesting times during startup.
    if (rootFrame) {
      NS_ASSERTION(mViewManager, "View manager must exist");
      mViewManager->BeginUpdateViewBatch();

      WalkFramesThroughPlaceholders(mPresContext, rootFrame,
                                    &ReResolveMenusAndTrees, nsnull);

      // Because "chrome:" URL equality is messy, reframe image box
      // frames (hack!).
      nsStyleChangeList changeList;
      WalkFramesThroughPlaceholders(mPresContext, rootFrame,
                                    ReframeImageBoxes, &changeList);
      mFrameConstructor->ProcessRestyledFrames(changeList);

      mViewManager->EndUpdateViewBatch(NS_VMREFRESH_NO_SYNC);
#ifdef ACCESSIBILITY
      InvalidateAccessibleSubtree(nsnull);
#endif
    }
    return NS_OK;
  }
#endif

  if (!nsCRT::strcmp(aTopic, NS_LINK_VISITED_EVENT_TOPIC)) {
    nsCOMPtr<nsIURI> uri = do_QueryInterface(aSubject);
    if (uri && mDocument) {
      mDocument->NotifyURIVisitednessChanged(uri);
    }
    return NS_OK;
  }

  if (!nsCRT::strcmp(aTopic, "agent-sheet-added") && mStyleSet) {
    AddAgentSheet(aSubject);
    return NS_OK;
  }

  if (!nsCRT::strcmp(aTopic, "user-sheet-added") && mStyleSet) {
    AddUserSheet(aSubject);
    return NS_OK;
  }

  if (!nsCRT::strcmp(aTopic, "agent-sheet-removed") && mStyleSet) {
    RemoveSheet(nsStyleSet::eAgentSheet, aSubject);
    return NS_OK;
  }

  if (!nsCRT::strcmp(aTopic, "user-sheet-removed") && mStyleSet) {
    RemoveSheet(nsStyleSet::eUserSheet, aSubject);
    return NS_OK;
  }

  NS_WARNING("unrecognized topic in PresShell::Observe");
  return NS_ERROR_FAILURE;
}

void
PresShell::EnumeratePlugins(nsIDOMDocument *aDocument,
                            const nsString &aPluginTag,
                            nsPluginEnumCallback aCallback)
{
  nsCOMPtr<nsIDOMNodeList> nodes;
  aDocument->GetElementsByTagName(aPluginTag, getter_AddRefs(nodes));
  if (!nodes)
    return;

  PRUint32 length;
  nodes->GetLength(&length);

  for (PRUint32 i = 0; i < length; ++i) {
    nsCOMPtr<nsIDOMNode> node;
    nodes->Item(i, getter_AddRefs(node));

    nsCOMPtr<nsIContent> content = do_QueryInterface(node);
    if (content)
      aCallback(this, content);
  }
}

//------------------------------------------------------
// End of protected and private methods on the PresShell
//------------------------------------------------------

// Start of DEBUG only code

#ifdef NS_DEBUG
#include "nsViewsCID.h"
#include "nsWidgetsCID.h"
#include "nsIDeviceContext.h"
#include "nsIURL.h"
#include "nsILinkHandler.h"

static NS_DEFINE_CID(kViewManagerCID, NS_VIEW_MANAGER_CID);
static NS_DEFINE_CID(kWidgetCID, NS_CHILD_CID);

static void
LogVerifyMessage(nsIFrame* k1, nsIFrame* k2, const char* aMsg)
{
  printf("verifyreflow: ");
  nsAutoString name;
  if (nsnull != k1) {
    nsIFrameDebug*  frameDebug;

    if (NS_SUCCEEDED(k1->QueryInterface(NS_GET_IID(nsIFrameDebug),
                                        (void**)&frameDebug))) {
     frameDebug->GetFrameName(name);
    }
  }
  else {
    name.Assign(NS_LITERAL_STRING("(null)"));
  }
  fputs(NS_LossyConvertUTF16toASCII(name).get(), stdout);

  printf(" != ");

  if (nsnull != k2) {
    nsIFrameDebug*  frameDebug;

    if (NS_SUCCEEDED(k2->QueryInterface(NS_GET_IID(nsIFrameDebug),
                                        (void**)&frameDebug))) {
      frameDebug->GetFrameName(name);
    }
  }
  else {
    name.Assign(NS_LITERAL_STRING("(null)"));
  }
  fputs(NS_LossyConvertUTF16toASCII(name).get(), stdout);

  printf(" %s", aMsg);
}

static void
LogVerifyMessage(nsIFrame* k1, nsIFrame* k2, const char* aMsg,
                 const nsRect& r1, const nsRect& r2)
{
  printf("VerifyReflow Error:\n");
  nsAutoString name;
  nsIFrameDebug*  frameDebug;

  if (NS_SUCCEEDED(k1->QueryInterface(NS_GET_IID(nsIFrameDebug),
                                      (void**)&frameDebug))) {
    fprintf(stdout, "  ");
    frameDebug->GetFrameName(name);
    fputs(NS_LossyConvertUTF16toASCII(name).get(), stdout);
    fprintf(stdout, " %p ", (void*)k1);
  }
  printf("{%d, %d, %d, %d}", r1.x, r1.y, r1.width, r1.height);

  printf(" != \n");

  if (NS_SUCCEEDED(k2->QueryInterface(NS_GET_IID(nsIFrameDebug),
                                      (void**)&frameDebug))) {
    fprintf(stdout, "  ");
    frameDebug->GetFrameName(name);
    fputs(NS_LossyConvertUTF16toASCII(name).get(), stdout);
    fprintf(stdout, " %p ", (void*)k2);
  }
  printf("{%d, %d, %d, %d}\n", r2.x, r2.y, r2.width, r2.height);

  printf("  %s\n", aMsg);
}

static PRBool
CompareTrees(nsPresContext* aFirstPresContext, nsIFrame* aFirstFrame, 
             nsPresContext* aSecondPresContext, nsIFrame* aSecondFrame)
{
  if (!aFirstPresContext || !aFirstFrame || !aSecondPresContext || !aSecondFrame)
    return PR_TRUE;
  PRBool ok = PR_TRUE;
  nsIAtom* listName = nsnull;
  PRInt32 listIndex = 0;
  do {
    nsIFrame* k1 = aFirstFrame->GetFirstChild(listName);
    nsIFrame* k2 = aSecondFrame->GetFirstChild(listName);
    PRInt32 l1 = nsContainerFrame::LengthOf(k1);
    PRInt32 l2 = nsContainerFrame::LengthOf(k2);
    if (l1 != l2) {
      ok = PR_FALSE;
      LogVerifyMessage(k1, k2, "child counts don't match: ");
      printf("%d != %d\n", l1, l2);
      if (0 == (VERIFY_REFLOW_ALL & gVerifyReflowFlags)) {
        break;
      }
    }

    nsRect r1, r2;
    nsIView* v1, *v2;
    for (;;) {
      if (((nsnull == k1) && (nsnull != k2)) ||
          ((nsnull != k1) && (nsnull == k2))) {
        ok = PR_FALSE;
        LogVerifyMessage(k1, k2, "child lists are different\n");
        break;
      }
      else if (nsnull != k1) {
        // Verify that the frames are the same size
        if (k1->GetRect() != k2->GetRect()) {
          ok = PR_FALSE;
          LogVerifyMessage(k1, k2, "(frame rects)", k1->GetRect(), k2->GetRect());
        }

        // Make sure either both have views or neither have views; if they
        // do have views, make sure the views are the same size. If the
        // views have widgets, make sure they both do or neither does. If
        // they do, make sure the widgets are the same size.
        v1 = k1->GetView();
        v2 = k2->GetView();
        if (((nsnull == v1) && (nsnull != v2)) ||
            ((nsnull != v1) && (nsnull == v2))) {
          ok = PR_FALSE;
          LogVerifyMessage(k1, k2, "child views are not matched\n");
        }
        else if (nsnull != v1) {
          if (v1->GetBounds() != v2->GetBounds()) {
            LogVerifyMessage(k1, k2, "(view rects)", v1->GetBounds(), v2->GetBounds());
          }

          nsIWidget* w1 = v1->GetWidget();
          nsIWidget* w2 = v2->GetWidget();
          if (((nsnull == w1) && (nsnull != w2)) ||
              ((nsnull != w1) && (nsnull == w2))) {
            ok = PR_FALSE;
            LogVerifyMessage(k1, k2, "child widgets are not matched\n");
          }
          else if (nsnull != w1) {
            w1->GetBounds(r1);
            w2->GetBounds(r2);
            if (r1 != r2) {
              LogVerifyMessage(k1, k2, "(widget rects)", r1, r2);
            }
          }
        }
        if (!ok && (0 == (VERIFY_REFLOW_ALL & gVerifyReflowFlags))) {
          break;
        }

        // verify that neither frame has a space manager,
        // or they both do and the space managers are equivalent
        nsSpaceManager *sm1 = NS_STATIC_CAST(nsSpaceManager*,
                         k1->GetProperty(nsLayoutAtoms::spaceManagerProperty));

        // look at the test frame
        nsSpaceManager *sm2 = NS_STATIC_CAST(nsSpaceManager*,
                         k2->GetProperty(nsLayoutAtoms::spaceManagerProperty));

        // now compare the space managers
        if (((nsnull == sm1) && (nsnull != sm2)) ||
            ((nsnull != sm1) && (nsnull == sm2))) {   // one is null, and the other is not
          ok = PR_FALSE;
          LogVerifyMessage(k1, k2, "space managers are not matched\n");
        }
        else if (sm1 && sm2) {  // both are not null, compare them
          // first, compare yMost
          nscoord yMost1, yMost2;
          nsresult smresult = sm1->YMost(yMost1);
          if (NS_ERROR_ABORT != smresult)
          {
            NS_ASSERTION(NS_SUCCEEDED(smresult), "bad result");
            smresult = sm2->YMost(yMost2);
            NS_ASSERTION(NS_SUCCEEDED(smresult), "bad result");
            if (yMost1 != yMost2) {
              LogVerifyMessage(k1, k2, "yMost of space managers differs\n");
            }
            // now compare bands by sampling
            PRInt32 yIncrement = yMost1/100;
            if (0==yIncrement) {
              yIncrement = 1;   // guarantee we make progress in the loop below
            }
            nscoord yOffset = 0;
            for ( ; ok && yOffset < yMost1; yOffset += yIncrement)
            {
              nscoord small=5, large=100;
              nsBandData band1, band2;
              nsBandTrapezoid trap1[20], trap2[20];
              band1.mSize = band2.mSize = 20;
              band1.mTrapezoids = trap1;  
              band2.mTrapezoids = trap2;
              sm1->GetBandData(yOffset, nsSize(small,small), band1);
              sm2->GetBandData(yOffset, nsSize(small,small), band2);
              if (band1.mCount != band2.mCount) 
              { // count mismatch, stop comparing
                LogVerifyMessage(k1, k2, "band.mCount of space managers differs\n");
                printf("count1= %d, count2=%d, yOffset = %d, size=%d\n",
                        band1.mCount, band2.mCount, yOffset, small);
                ok = PR_FALSE;
                      
              }
              else   // band counts match, compare individual traps
              { 
                PRInt32 trapIndex=0;
                for ( ;trapIndex<band1.mCount; trapIndex++)
                {
                  PRBool match = (trap1[trapIndex].EqualGeometry(trap2[trapIndex])) && 
                    trap1[trapIndex].mState == trap2[trapIndex].mState;
                  if (!match)
                  {
                    LogVerifyMessage(k1, k2, "band.mTrapezoids of space managers differs\n");
                    printf ("index %d\n", trapIndex);
                  }
                }
              }
              // test the larger maxSize
              sm1->GetBandData(yOffset, nsSize(large,large), band1);
              sm2->GetBandData(yOffset, nsSize(large,large), band2);
              if (band1.mCount != band2.mCount) 
              { // count mismatch, stop comparing
                LogVerifyMessage(k1, k2, "band.mCount of space managers differs\n");
                printf("count1= %d, count2=%d, yOffset = %d, size=%d\n",
                        band1.mCount, band2.mCount, yOffset, small);
                ok = PR_FALSE;
                      
              }
              else   // band counts match, compare individual traps
              { 
                PRInt32 trapIndex=0;
                for ( ; trapIndex<band1.mCount; trapIndex++)
                {
                  PRBool match = (trap1[trapIndex].EqualGeometry(trap2[trapIndex])) && 
                    trap1[trapIndex].mState == trap2[trapIndex].mState;
                  if (!match)
                  {
                    LogVerifyMessage(k1, k2, "band.mTrapezoids of space managers differs\n");
                    printf ("index %d\n", trapIndex);
                  }
                }
              }
            }
          }
        }




        // Compare the sub-trees too
        if (!CompareTrees(aFirstPresContext, k1, aSecondPresContext, k2)) {
          ok = PR_FALSE;
          if (0 == (VERIFY_REFLOW_ALL & gVerifyReflowFlags)) {
            break;
          }
        }

        // Advance to next sibling
        k1 = k1->GetNextSibling();
        k2 = k2->GetNextSibling();
      }
      else {
        break;
      }
    }
    if (!ok && (0 == (VERIFY_REFLOW_ALL & gVerifyReflowFlags))) {
      break;
    }

    nsIAtom* listName1 = aFirstFrame->GetAdditionalChildListName(listIndex);
    nsIAtom* listName2 = aSecondFrame->GetAdditionalChildListName(listIndex);
    listIndex++;
    if (listName1 != listName2) {
      if (0 == (VERIFY_REFLOW_ALL & gVerifyReflowFlags)) {
        ok = PR_FALSE;
      }
      LogVerifyMessage(k1, k2, "child list names are not matched: ");
      nsAutoString tmp;
      if (nsnull != listName1) {
        listName1->ToString(tmp);
        fputs(NS_LossyConvertUTF16toASCII(tmp).get(), stdout);
      }
      else
        fputs("(null)", stdout);
      printf(" != ");
      if (nsnull != listName2) {
        listName2->ToString(tmp);
        fputs(NS_LossyConvertUTF16toASCII(tmp).get(), stdout);
      }
      else
        fputs("(null)", stdout);
      printf("\n");
      break;
    }
    listName = listName1;
  } while (ok && (listName != nsnull));

  return ok;
}
#endif

#if 0
static nsIFrame*
FindTopFrame(nsIFrame* aRoot)
{
  if (aRoot) {
    nsIContent* content = aRoot->GetContent();
    if (content) {
      nsIAtom* tag;
      content->GetTag(tag);
      if (nsnull != tag) {
        NS_RELEASE(tag);
        return aRoot;
      }
    }

    // Try one of the children
    nsIFrame* kid = aRoot->GetFirstChild(nsnull);
    while (nsnull != kid) {
      nsIFrame* result = FindTopFrame(kid);
      if (nsnull != result) {
        return result;
      }
      kid = kid->GetNextSibling();
    }
  }
  return nsnull;
}
#endif


#ifdef DEBUG

nsresult
PresShell::CloneStyleSet(nsStyleSet* aSet, nsStyleSet** aResult)
{
  nsStyleSet *clone = new nsStyleSet();
  if (!clone) {
    return NS_ERROR_OUT_OF_MEMORY;
  }

  PRInt32 i, n = aSet->SheetCount(nsStyleSet::eOverrideSheet);
  for (i = 0; i < n; i++) {
    nsIStyleSheet* ss = aSet->StyleSheetAt(nsStyleSet::eOverrideSheet, i);
    if (ss)
      clone->AppendStyleSheet(nsStyleSet::eOverrideSheet, ss);
  }

  n = aSet->SheetCount(nsStyleSet::eDocSheet);
  for (i = 0; i < n; i++) {
    nsIStyleSheet* ss = aSet->StyleSheetAt(nsStyleSet::eDocSheet, i);
    if (ss)
      clone->AddDocStyleSheet(ss, mDocument);
  }
  n = aSet->SheetCount(nsStyleSet::eUserSheet);
  for (i = 0; i < n; i++) {
    nsIStyleSheet* ss = aSet->StyleSheetAt(nsStyleSet::eUserSheet, i);
    if (ss)
      clone->AppendStyleSheet(nsStyleSet::eUserSheet, ss);
  }

  n = aSet->SheetCount(nsStyleSet::eAgentSheet);
  for (i = 0; i < n; i++) {
    nsIStyleSheet* ss = aSet->StyleSheetAt(nsStyleSet::eAgentSheet, i);
    if (ss)
      clone->AppendStyleSheet(nsStyleSet::eAgentSheet, ss);
  }
  *aResult = clone;
  return NS_OK;
}

// After an incremental reflow, we verify the correctness by doing a
// full reflow into a fresh frame tree.
PRBool
PresShell::VerifyIncrementalReflow()
{
   if (VERIFY_REFLOW_NOISY & gVerifyReflowFlags) {
     printf("Building Verification Tree...\n");
   }
  // All the stuff we are creating that needs releasing
  nsPresContext* cx;
  nsIViewManager* vm;
  nsIPresShell* sh;

  // Create a presentation context to view the new frame tree
  NS_IF_ADDREF(cx = new nsPresContext(mDocument,
                                      mPresContext->IsPaginated() ?
                                       nsPresContext::eContext_PrintPreview :
                                       nsPresContext::eContext_Galley));

  if (!cx)
    return NS_ERROR_OUT_OF_MEMORY;

  nsCOMPtr<nsISupports> container = mPresContext->GetContainer();
  if (container) {
    cx->SetContainer(container);
    nsCOMPtr<nsILinkHandler> lh = do_QueryInterface(container);
    if (lh) {
      cx->SetLinkHandler(lh);
    }
  }

  nsIDeviceContext *dc = mPresContext->DeviceContext();
  nsresult rv = cx->Init(dc);
  NS_ENSURE_SUCCESS(rv, rv);

  // Get our scrolling preference
  nsIView* rootView;
  mViewManager->GetRootView(rootView);
  void* nativeParentWidget = rootView->GetWidget()->GetNativeData(NS_NATIVE_WIDGET);

  // Create a new view manager.
  rv = CallCreateInstance(kViewManagerCID, &vm);
  NS_ASSERTION(NS_SUCCEEDED (rv), "failed to create view manager");
  rv = vm->Init(dc);
  NS_ASSERTION(NS_SUCCEEDED (rv), "failed to init view manager");

  // Create a child window of the parent that is our "root view/window"
  // Create a view
  nsRect tbounds = mPresContext->GetVisibleArea();
  nsIView* view = vm->CreateView(tbounds, nsnull);
  NS_ASSERTION(view, "failed to create view");
  if (!view)
    return PR_FALSE;

  //now create the widget for the view
  rv = view->CreateWidget(kWidgetCID, nsnull, nativeParentWidget, PR_TRUE);
  NS_ASSERTION(NS_SUCCEEDED(rv), "failed to create scroll view widget");

  // Setup hierarchical relationship in view manager
  vm->SetRootView(view);

  // Make the new presentation context the same size as our
  // presentation context.
  nsRect r = mPresContext->GetVisibleArea();
  cx->SetVisibleArea(r);

  // Create a new presentation shell to view the document. Use the
  // exact same style information that this document has.
  nsAutoPtr<nsStyleSet> newSet;
  rv = CloneStyleSet(mStyleSet, getter_Transfers(newSet));
  NS_ASSERTION(NS_SUCCEEDED(rv), "failed to clone style set");
  rv = mDocument->CreateShell(cx, vm, newSet, &sh);
  sh->SetVerifyReflowEnable(PR_FALSE); // turn off verify reflow while we're reflowing the test frame tree
  NS_ASSERTION(NS_SUCCEEDED (rv), "failed to create presentation shell");
  vm->SetViewObserver((nsIViewObserver *)((PresShell*)sh));
  rv = sh->InitialReflow(r.width, r.height);
  NS_ENSURE_SUCCESS(rv, rv);
  sh->SetVerifyReflowEnable(PR_TRUE);  // turn on verify reflow again now that we're done reflowing the test frame tree
  if (VERIFY_REFLOW_NOISY & gVerifyReflowFlags) {
     printf("Verification Tree built, comparing...\n");
  }

  // Now that the document has been reflowed, use its frame tree to
  // compare against our frame tree.
  nsIFrame* root1 = FrameManager()->GetRootFrame();
  nsIFrame* root2 = sh->FrameManager()->GetRootFrame();
  PRBool ok = CompareTrees(mPresContext, root1, cx, root2);
  if (!ok && (VERIFY_REFLOW_NOISY & gVerifyReflowFlags)) {
    printf("Verify reflow failed, primary tree:\n");
    nsIFrameDebug*  frameDebug;

    if (NS_SUCCEEDED(root1->QueryInterface(NS_GET_IID(nsIFrameDebug),
                                           (void**)&frameDebug))) {
      frameDebug->List(stdout, 0);
    }
    printf("Verification tree:\n");
    if (NS_SUCCEEDED(root2->QueryInterface(NS_GET_IID(nsIFrameDebug),
                                           (void**)&frameDebug))) {
      frameDebug->List(stdout, 0);
    }
  }

  cx->SetContainer(nsnull);
  NS_RELEASE(cx);
  sh->EndObservingDocument();
  NS_RELEASE(sh);
  NS_RELEASE(vm);
  if (VERIFY_REFLOW_NOISY & gVerifyReflowFlags) {
    printf("Finished Verifying Reflow...\n");
  }

  return ok;
}

// Layout debugging hooks
void
PresShell::ListStyleContexts(nsIFrame *aRootFrame, FILE *out, PRInt32 aIndent)
{
  nsStyleContext *sc = aRootFrame->GetStyleContext();
  if (sc)
    sc->List(out, aIndent);
}

void
PresShell::ListStyleSheets(FILE *out, PRInt32 aIndent)
{
  PRInt32 sheetCount = mStyleSet->SheetCount(nsStyleSet::eDocSheet);
  for (PRInt32 i = 0; i < sheetCount; ++i) {
    mStyleSet->StyleSheetAt(nsStyleSet::eDocSheet, i)->List(out, aIndent);
    fputs("\n", out);
  }
}

void
PresShell::VerifyStyleTree()
{
  VERIFY_STYLE_TREE;
}
#endif

// PresShellViewEventListener

NS_IMPL_ISUPPORTS2(PresShellViewEventListener, nsIScrollPositionListener, nsICompositeListener)

PresShellViewEventListener::PresShellViewEventListener()
{
  mPresShell  = 0;
  mWasVisible = PR_FALSE;
  mCallCount  = 0;
}

PresShellViewEventListener::~PresShellViewEventListener()
{
  mPresShell  = 0;
}

nsresult
PresShellViewEventListener::SetPresShell(nsIPresShell *aPresShell)
{
  mPresShell = aPresShell;
  return NS_OK;
}

nsresult
PresShellViewEventListener::HideCaret()
{
  nsresult result = NS_OK;

  if (mPresShell && 0 == mCallCount)
  {
    nsCOMPtr<nsISelectionController> selCon = do_QueryInterface(mPresShell);
    if (selCon)
    {
      result = selCon->GetCaretEnabled(&mWasVisible);

      if (NS_SUCCEEDED(result) && mWasVisible)
        result = selCon->SetCaretEnabled(PR_FALSE);
    }
  }

  ++mCallCount;

  return result;
}

nsresult
PresShellViewEventListener::RestoreCaretVisibility()
{
  nsresult result = NS_OK;

  --mCallCount;

  if (mPresShell && 0 == mCallCount && mWasVisible)
  {
    nsCOMPtr<nsISelectionController> selCon = do_QueryInterface(mPresShell);
    if (selCon)
      result = selCon->SetCaretEnabled(PR_TRUE);
  }

  return result;
}

NS_IMETHODIMP
PresShellViewEventListener::ScrollPositionWillChange(nsIScrollableView *aView, nscoord aX, nscoord aY)
{
  return HideCaret();
}

NS_IMETHODIMP
PresShellViewEventListener::ScrollPositionDidChange(nsIScrollableView *aView, nscoord aX, nscoord aY)
{
  return RestoreCaretVisibility();
}

NS_IMETHODIMP
PresShellViewEventListener::WillRefreshRegion(nsIViewManager *aViewManager,
                                     nsIView *aView,
                                     nsIRenderingContext *aContext,
                                     nsIRegion *aRegion,
                                     PRUint32 aUpdateFlags)
{
  return NS_OK;
}

NS_IMETHODIMP
PresShellViewEventListener::DidRefreshRegion(nsIViewManager *aViewManager,
                                    nsIView *aView,
                                    nsIRenderingContext *aContext,
                                    nsIRegion *aRegion,
                                    PRUint32 aUpdateFlags)
{
  nsCSSRendering::DidPaint();

  return NS_OK;
}

NS_IMETHODIMP
PresShellViewEventListener::WillRefreshRect(nsIViewManager *aViewManager,
                                   nsIView *aView,
                                   nsIRenderingContext *aContext,
                                   const nsRect *aRect,
                                   PRUint32 aUpdateFlags)
{
  return NS_OK;
}

NS_IMETHODIMP
PresShellViewEventListener::DidRefreshRect(nsIViewManager *aViewManager,
                                  nsIView *aView,
                                  nsIRenderingContext *aContext,
                                  const nsRect *aRect,
                                  PRUint32 aUpdateFlags)
{
  nsCSSRendering::DidPaint();

  return NS_OK;
}


//=============================================================
//=============================================================
//-- Debug Reflow Counts
//=============================================================
//=============================================================
#ifdef MOZ_REFLOW_PERF
//-------------------------------------------------------------
NS_IMETHODIMP
PresShell::DumpReflows()
{
  if (mReflowCountMgr) {
    nsCAutoString uriStr;
    if (mDocument) {
      nsIURI *uri = mDocument->GetDocumentURI();
      if (uri) {
        uri->GetPath(uriStr);
      }
    }
    mReflowCountMgr->DisplayTotals(uriStr.get());
    mReflowCountMgr->DisplayHTMLTotals(uriStr.get());
    mReflowCountMgr->DisplayDiffsInTotals("Differences");
  }
  return NS_OK;
}

//-------------------------------------------------------------
NS_IMETHODIMP
PresShell::CountReflows(const char * aName, PRUint32 aType, nsIFrame * aFrame)
{
  if (mReflowCountMgr) {
    mReflowCountMgr->Add(aName, (nsReflowReason)aType, aFrame);
  }

  return NS_OK;
}

//-------------------------------------------------------------
NS_IMETHODIMP
PresShell::PaintCount(const char * aName, nsIRenderingContext* aRenderingContext, nsPresContext* aPresContext, nsIFrame * aFrame, PRUint32 aColor)
{
  if (mReflowCountMgr) {
    mReflowCountMgr->PaintCount(aName, aRenderingContext, aPresContext, aFrame, aColor);
  }
  return NS_OK;
}

//-------------------------------------------------------------
NS_IMETHODIMP
PresShell::SetPaintFrameCount(PRBool aPaintFrameCounts)
{ 
  if (mReflowCountMgr) {
    mReflowCountMgr->SetPaintFrameCounts(aPaintFrameCounts);
  }
  return NS_OK; 
}

//------------------------------------------------------------------
//-- Reflow Counter Classes Impls
//------------------------------------------------------------------

//------------------------------------------------------------------
ReflowCounter::ReflowCounter(ReflowCountMgr * aMgr) :
  mMgr(aMgr)
{
  ClearTotals();
  SetTotalsCache();
}

//------------------------------------------------------------------
ReflowCounter::~ReflowCounter()
{
  
}

//------------------------------------------------------------------
void ReflowCounter::ClearTotals()
{
  for (PRUint32 i=0;i<NUM_REFLOW_TYPES;i++) {
    mTotals[i] = 0;
  }
}

//------------------------------------------------------------------
void ReflowCounter::SetTotalsCache()
{
  for (PRUint32 i=0;i<NUM_REFLOW_TYPES;i++) {
    mCacheTotals[i] = mTotals[i];
  }
}

//------------------------------------------------------------------
void ReflowCounter::CalcDiffInTotals()
{
  for (PRUint32 i=0;i<NUM_REFLOW_TYPES;i++) {
    mCacheTotals[i] = mTotals[i] - mCacheTotals[i];
  }
}

//------------------------------------------------------------------
void ReflowCounter::DisplayTotals(const char * aStr)
{
  DisplayTotals(mTotals, aStr?aStr:"Totals");
}

//------------------------------------------------------------------
void ReflowCounter::DisplayDiffTotals(const char * aStr)
{
  DisplayTotals(mCacheTotals, aStr?aStr:"Diff Totals");
}

//------------------------------------------------------------------
void ReflowCounter::DisplayHTMLTotals(const char * aStr)
{
  DisplayHTMLTotals(mTotals, aStr?aStr:"Totals");
}

//------------------------------------------------------------------
void ReflowCounter::DisplayTotals(PRUint32 * aArray, const char * aTitle)
{
  // figure total
  PRUint32 total = 0;
  PRUint32 i;
  for (i=0;i<NUM_REFLOW_TYPES;i++) {
    total += aArray[i];
  }

  if (total == 0) {
    return;
  }
  ReflowCounter * gTots = (ReflowCounter *)mMgr->LookUp(kGrandTotalsStr);

  printf("%25s\t", aTitle);
  for (i=0;i<NUM_REFLOW_TYPES;i++) {
    printf("%d\t", aArray[i]);
    if (gTots != this &&  aArray[i] > 0) {
      gTots->Add((nsReflowReason)i, aArray[i]);
    }
  }
  printf("%d\n", total);
}

//------------------------------------------------------------------
void ReflowCounter::DisplayHTMLTotals(PRUint32 * aArray, const char * aTitle)
{
  // figure total
  PRUint32 total = 0;
  PRUint32 i;
  for (i=0;i<NUM_REFLOW_TYPES;i++) {
    total += aArray[i];
  }

  if (total == 0) {
    return;
  }

  ReflowCounter * gTots = (ReflowCounter *)mMgr->LookUp(kGrandTotalsStr);
  FILE * fd = mMgr->GetOutFile();
  if (!fd) {
    return;
  }

  fprintf(fd, "<tr><td><center>%s</center></td>", aTitle);
  for (i=0;i<NUM_REFLOW_TYPES;i++) {
    fprintf(fd, "<td><center>");
    if (aArray[i]) {
      fprintf(fd, "%d", aArray[i]);
    } else {
      fprintf(fd, "&nbsp;");
    }
    fprintf(fd, "</center></td>");
    if (gTots != this &&  aArray[i] > 0) {
      gTots->Add((nsReflowReason)i, aArray[i]);
    }
  }
  fprintf(fd, "<td><center>%d</center></td></tr>\n", total);
}

//------------------------------------------------------------------
//-- ReflowCountMgr
//------------------------------------------------------------------
ReflowCountMgr::ReflowCountMgr()
{
  mCounts = PL_NewHashTable(10, PL_HashString, PL_CompareStrings, 
                                PL_CompareValues, nsnull, nsnull);
  mIndiFrameCounts = PL_NewHashTable(10, PL_HashString, PL_CompareStrings, 
                                     PL_CompareValues, nsnull, nsnull);
  mCycledOnce              = PR_FALSE;
  mDumpFrameCounts         = PR_FALSE;
  mDumpFrameByFrameCounts  = PR_FALSE;
  mPaintFrameByFrameCounts = PR_FALSE;
}

//------------------------------------------------------------------
ReflowCountMgr::~ReflowCountMgr()
{
  CleanUp();
}

//------------------------------------------------------------------
ReflowCounter * ReflowCountMgr::LookUp(const char * aName)
{
  if (nsnull != mCounts) {
    ReflowCounter * counter = (ReflowCounter *)PL_HashTableLookup(mCounts, aName);
    return counter;
  }
  return nsnull;

}

//------------------------------------------------------------------
void ReflowCountMgr::Add(const char * aName, nsReflowReason aType, nsIFrame * aFrame)
{
  NS_ASSERTION(aName != nsnull, "Name shouldn't be null!");

  if (mDumpFrameCounts && nsnull != mCounts) {
    ReflowCounter * counter = (ReflowCounter *)PL_HashTableLookup(mCounts, aName);
    if (counter == nsnull) {
      counter = new ReflowCounter(this);
      NS_ASSERTION(counter != nsnull, "null ptr");
      char * name = nsCRT::strdup(aName);
      NS_ASSERTION(name != nsnull, "null ptr");
      PL_HashTableAdd(mCounts, name, counter);
    }
    counter->Add(aType);
  }

  if ((mDumpFrameByFrameCounts || mPaintFrameByFrameCounts) && 
      nsnull != mIndiFrameCounts && 
      aFrame != nsnull) {
    char * key = new char[16];
    sprintf(key, "%p", (void*)aFrame);
    IndiReflowCounter * counter = (IndiReflowCounter *)PL_HashTableLookup(mIndiFrameCounts, key);
    if (counter == nsnull) {
      counter = new IndiReflowCounter(this);
      NS_ASSERTION(counter != nsnull, "null ptr");
      counter->mFrame = aFrame;
      counter->mName.AssignASCII(aName);
      PL_HashTableAdd(mIndiFrameCounts, key, counter);
    }
    // this eliminates extra counts from super classes
    if (counter != nsnull && counter->mName.EqualsASCII(aName)) {
      counter->mCount++;
      counter->mCounter.Add(aType, 1);
    }
  }
}

//------------------------------------------------------------------
void ReflowCountMgr::PaintCount(const char *    aName, 
                                nsIRenderingContext* aRenderingContext, 
                                nsPresContext* aPresContext, 
                                nsIFrame*       aFrame, 
                                PRUint32        aColor)
{
  if (mPaintFrameByFrameCounts && 
      nsnull != mIndiFrameCounts && 
      aFrame != nsnull) {
    char * key = new char[16];
    sprintf(key, "%p", (void*)aFrame);
    IndiReflowCounter * counter = (IndiReflowCounter *)PL_HashTableLookup(mIndiFrameCounts, key);
    if (counter != nsnull && counter->mName.EqualsASCII(aName)) {
      aRenderingContext->PushState();
      nsFont font("Times", NS_FONT_STYLE_NORMAL,NS_FONT_VARIANT_NORMAL,
                  NS_FONT_WEIGHT_NORMAL,0,NSIntPointsToTwips(8));

      nsCOMPtr<nsIFontMetrics> fm = aPresContext->GetMetricsFor(font);
      aRenderingContext->SetFont(fm);
      char buf[16];
      sprintf(buf, "%d", counter->mCount);
      nscoord x = 0, y;
      nscoord width, height;
      aRenderingContext->GetWidth((char*)buf, width);
      fm->GetHeight(height);
      fm->GetMaxAscent(y);

      PRUint32 color;
      PRUint32 color2;
      if (aColor != 0) {
        color  = aColor;
        color2 = NS_RGB(0,0,0);
      } else {
        PRUint8 rc = 0, gc = 0, bc = 0;
        if (counter->mCount < 5) {
          rc = 255;
          gc = 255;
        } else if ( counter->mCount < 11) {
          gc = 255;
        } else {
          rc = 255;
        }
        color  = NS_RGB(rc,gc,bc);
        color2 = NS_RGB(rc/2,gc/2,bc/2);
      }

      nsRect rect(0,0, width+15, height+15);
      aRenderingContext->SetColor(NS_RGB(0,0,0));
      aRenderingContext->FillRect(rect);
      aRenderingContext->SetColor(color2);
      aRenderingContext->DrawString(buf, strlen(buf), x+15,y+15);
      aRenderingContext->SetColor(color);
      aRenderingContext->DrawString(buf, strlen(buf), x,y);

      aRenderingContext->PopState();
    }
  }
}

//------------------------------------------------------------------
PRIntn ReflowCountMgr::RemoveItems(PLHashEntry *he, PRIntn i, void *arg)
{
  char *str = (char *)he->key;
  ReflowCounter * counter = (ReflowCounter *)he->value;
  delete counter;
  delete [] str;

  return HT_ENUMERATE_REMOVE;
}

//------------------------------------------------------------------
PRIntn ReflowCountMgr::RemoveIndiItems(PLHashEntry *he, PRIntn i, void *arg)
{
  char *str = (char *)he->key;
  IndiReflowCounter * counter = (IndiReflowCounter *)he->value;
  delete counter;
  delete [] str;

  return HT_ENUMERATE_REMOVE;
}

//------------------------------------------------------------------
void ReflowCountMgr::CleanUp()
{
  if (nsnull != mCounts) {
    PL_HashTableEnumerateEntries(mCounts, RemoveItems, nsnull);
    PL_HashTableDestroy(mCounts);
    mCounts = nsnull;
  }

  if (nsnull != mIndiFrameCounts) {
    PL_HashTableEnumerateEntries(mIndiFrameCounts, RemoveIndiItems, nsnull);
    PL_HashTableDestroy(mIndiFrameCounts);
    mIndiFrameCounts = nsnull;
  }
}

//------------------------------------------------------------------
PRIntn ReflowCountMgr::DoSingleTotal(PLHashEntry *he, PRIntn i, void *arg)
{
  char *str = (char *)he->key;
  ReflowCounter * counter = (ReflowCounter *)he->value;

  counter->DisplayTotals(str);

  return HT_ENUMERATE_NEXT;
}

//------------------------------------------------------------------
void ReflowCountMgr::DoGrandTotals()
{
  if (nsnull != mCounts) {
    ReflowCounter * gTots = (ReflowCounter *)PL_HashTableLookup(mCounts, kGrandTotalsStr);
    if (gTots == nsnull) {
      gTots = new ReflowCounter(this);
      PL_HashTableAdd(mCounts, nsCRT::strdup(kGrandTotalsStr), gTots);
    } else {
      gTots->ClearTotals();
    }

    static const char * title[] = {"Init", "Incrm", "Resze", "Style", "Dirty", "Total"};
    printf("\t\t\t");
    PRUint32 i;
    for (i=0;i<NUM_REFLOW_TYPES+1;i++) {
      printf("\t%s", title[i]);
    }
    printf("\n");
    for (i=0;i<78;i++) {
      printf("-");
    }
    printf("\n");
    PL_HashTableEnumerateEntries(mCounts, DoSingleTotal, this);
  }
}

static void RecurseIndiTotals(nsPresContext* aPresContext, 
                              PLHashTable *   aHT, 
                              nsIFrame *      aParentFrame,
                              PRInt32         aLevel)
{
  if (aParentFrame == nsnull) {
    return;
  }

  char key[16];
  sprintf(key, "%p", (void*)aParentFrame);
  IndiReflowCounter * counter = (IndiReflowCounter *)PL_HashTableLookup(aHT, key);
  if (counter) {
    counter->mHasBeenOutput = PR_TRUE;
    char * name = ToNewCString(counter->mName);
    for (PRInt32 i=0;i<aLevel;i++) printf(" ");
    printf("%s - %p   [%d][", name, (void*)aParentFrame, counter->mCount);
    for (PRInt32 inx=0;inx<5;inx++) {
      if (inx != 0) printf(",");
      printf("%d", counter->mCounter.GetTotalByType(nsReflowReason(inx)));
    }
    printf("]\n");
    nsMemory::Free(name);
  }

  nsIFrame* child = aParentFrame->GetFirstChild(nsnull);
  while (child) {
    RecurseIndiTotals(aPresContext, aHT, child, aLevel+1);
    child = child->GetNextSibling();
  }

}

//------------------------------------------------------------------
PRIntn ReflowCountMgr::DoSingleIndi(PLHashEntry *he, PRIntn i, void *arg)
{
  IndiReflowCounter * counter = (IndiReflowCounter *)he->value;
  if (counter && !counter->mHasBeenOutput) {
    char * name = ToNewCString(counter->mName);
    printf("%s - %p   [%d][", name, (void*)counter->mFrame, counter->mCount);
    for (PRInt32 inx=0;inx<5;inx++) {
      if (inx != 0) printf(",");
      printf("%d", counter->mCounter.GetTotalByType(nsReflowReason(inx)));
    }
    printf("]\n");
    nsMemory::Free(name);
  }
  return HT_ENUMERATE_NEXT;
}

//------------------------------------------------------------------
void ReflowCountMgr::DoIndiTotalsTree()
{
  if (nsnull != mCounts) {
    printf("\n------------------------------------------------\n");
    printf("-- Individual Frame Counts\n");
    printf("------------------------------------------------\n");

    if (mPresShell) {
      nsIFrame * rootFrame = mPresShell->FrameManager()->GetRootFrame();
      RecurseIndiTotals(mPresContext, mIndiFrameCounts, rootFrame, 0);
      printf("------------------------------------------------\n");
      printf("-- Individual Counts of Frames not in Root Tree\n");
      printf("------------------------------------------------\n");
      PL_HashTableEnumerateEntries(mIndiFrameCounts, DoSingleIndi, this);
    }
  }
}

//------------------------------------------------------------------
PRIntn ReflowCountMgr::DoSingleHTMLTotal(PLHashEntry *he, PRIntn i, void *arg)
{
  char *str = (char *)he->key;
  ReflowCounter * counter = (ReflowCounter *)he->value;

  counter->DisplayHTMLTotals(str);

  return HT_ENUMERATE_NEXT;
}

//------------------------------------------------------------------
void ReflowCountMgr::DoGrandHTMLTotals()
{
  if (nsnull != mCounts) {
    ReflowCounter * gTots = (ReflowCounter *)PL_HashTableLookup(mCounts, kGrandTotalsStr);
    if (gTots == nsnull) {
      gTots = new ReflowCounter(this);
      PL_HashTableAdd(mCounts, nsCRT::strdup(kGrandTotalsStr), gTots);
    } else {
      gTots->ClearTotals();
    }

    static const char * title[] = {"Class", "Init", "Incrm", "Resze", "Style", "Dirty", "Total"};
    fprintf(mFD, "<tr>");
    PRUint32 i;
    for (i=0;i<NUM_REFLOW_TYPES+2;i++) {
      fprintf(mFD, "<td><center><b>%s<b></center></td>", title[i]);
    }
    fprintf(mFD, "</tr>\n");
    PL_HashTableEnumerateEntries(mCounts, DoSingleHTMLTotal, this);
  }
}

//------------------------------------
void ReflowCountMgr::DisplayTotals(const char * aStr)
{
#ifdef DEBUG_rods
  printf("%s\n", aStr?aStr:"No name");
#endif
  if (mDumpFrameCounts) {
    DoGrandTotals();
  }
  if (mDumpFrameByFrameCounts) {
    DoIndiTotalsTree();
  }

}
//------------------------------------
void ReflowCountMgr::DisplayHTMLTotals(const char * aStr)
{
#ifdef WIN32x // XXX NOT XP!
  char name[1024];
  
  char * sptr = strrchr(aStr, '/');
  if (sptr) {
    sptr++;
    strcpy(name, sptr);
    char * eptr = strrchr(name, '.');
    if (eptr) {
      *eptr = 0;
    }
    strcat(name, "_stats.html");
  }
  mFD = fopen(name, "w");
  if (mFD) {
    fprintf(mFD, "<html><head><title>Reflow Stats</title></head><body>\n");
    const char * title = aStr?aStr:"No name";
    fprintf(mFD, "<center><b>%s</b><br><table border=1 style=\"background-color:#e0e0e0\">", title);
    DoGrandHTMLTotals();
    fprintf(mFD, "</center></table>\n");
    fprintf(mFD, "</body></html>\n");
    fclose(mFD);
    mFD = nsnull;
  }
#endif // not XP!
}

//------------------------------------------------------------------
PRIntn ReflowCountMgr::DoClearTotals(PLHashEntry *he, PRIntn i, void *arg)
{
  ReflowCounter * counter = (ReflowCounter *)he->value;
  counter->ClearTotals();

  return HT_ENUMERATE_NEXT;
}

//------------------------------------------------------------------
void ReflowCountMgr::ClearTotals()
{
  PL_HashTableEnumerateEntries(mCounts, DoClearTotals, this);
}

//------------------------------------------------------------------
void ReflowCountMgr::ClearGrandTotals()
{
  if (nsnull != mCounts) {
    ReflowCounter * gTots = (ReflowCounter *)PL_HashTableLookup(mCounts, kGrandTotalsStr);
    if (gTots == nsnull) {
      gTots = new ReflowCounter(this);
      PL_HashTableAdd(mCounts, nsCRT::strdup(kGrandTotalsStr), gTots);
    } else {
      gTots->ClearTotals();
      gTots->SetTotalsCache();
    }
  }
}

//------------------------------------------------------------------
PRIntn ReflowCountMgr::DoDisplayDiffTotals(PLHashEntry *he, PRIntn i, void *arg)
{
  PRBool cycledOnce = (arg != 0);

  char *str = (char *)he->key;
  ReflowCounter * counter = (ReflowCounter *)he->value;

  if (cycledOnce) {
    counter->CalcDiffInTotals();
    counter->DisplayDiffTotals(str);
  }
  counter->SetTotalsCache();

  return HT_ENUMERATE_NEXT;
}

//------------------------------------------------------------------
void ReflowCountMgr::DisplayDiffsInTotals(const char * aStr)
{
  if (mCycledOnce) {
    printf("Differences\n");
    for (PRInt32 i=0;i<78;i++) {
      printf("-");
    }
    printf("\n");
    ClearGrandTotals();
  }
  PL_HashTableEnumerateEntries(mCounts, DoDisplayDiffTotals, (void *)mCycledOnce);

  mCycledOnce = PR_TRUE;
}

#endif // MOZ_REFLOW_PERF

// make a color string like #RRGGBB
void ColorToString(nscolor aColor, nsAutoString &aString)
{
  char buf[8];

  PR_snprintf(buf, sizeof(buf), "#%02x%02x%02x",
              NS_GET_R(aColor), NS_GET_G(aColor), NS_GET_B(aColor));
  CopyASCIItoUTF16(buf, aString);
}
