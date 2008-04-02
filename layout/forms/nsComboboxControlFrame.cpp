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
 * ***** END LICENSE BLOCK ***** */
#include "nsCOMPtr.h"
#include "nsReadableUtils.h"
#include "nsComboboxControlFrame.h"
#include "nsIDOMEventTarget.h"
#include "nsFrameManager.h"
#include "nsFormControlFrame.h"
#include "nsGfxButtonControlFrame.h"
#include "nsGkAtoms.h"
#include "nsCSSAnonBoxes.h"
#include "nsHTMLParts.h"
#include "nsIFormControl.h"
#include "nsINameSpaceManager.h"
#include "nsIDOMElement.h"
#include "nsIListControlFrame.h"
#include "nsIDOMHTMLCollection.h" 
#include "nsIDOMHTMLSelectElement.h" 
#include "nsIDOMHTMLOptionElement.h" 
#include "nsIDOMNSHTMLOptionCollectn.h" 
#include "nsIPresShell.h"
#include "nsIDeviceContext.h"
#include "nsIView.h"
#include "nsIViewManager.h"
#include "nsIScrollableView.h"
#include "nsEventDispatcher.h"
#include "nsIEventStateManager.h"
#include "nsIEventListenerManager.h"
#include "nsIDOMNode.h"
#include "nsIPrivateDOMEvent.h"
#include "nsISelectControlFrame.h"
#include "nsXPCOM.h"
#include "nsISupportsPrimitives.h"
#include "nsIComponentManager.h"
#include "nsContentUtils.h"
#include "nsTextFragment.h"
#include "nsCSSFrameConstructor.h"
#include "nsIDocument.h"
#include "nsINodeInfo.h"
#include "nsIScrollableFrame.h"
#include "nsListControlFrame.h"
#include "nsContentCID.h"
#ifdef ACCESSIBILITY
#include "nsIAccessibilityService.h"
#endif
#include "nsIServiceManager.h"
#include "nsIDOMNode.h"
#include "nsGUIEvent.h"
#include "nsAutoPtr.h"
#include "nsStyleSet.h"
#include "nsNodeInfoManager.h"
#include "nsContentCreatorFunctions.h"
#include "nsLayoutUtils.h"
#include "nsDisplayList.h"
#include "nsBoxLayoutState.h"
#include "nsITheme.h"
#include "nsThemeConstants.h"

NS_IMETHODIMP
nsComboboxControlFrame::RedisplayTextEvent::Run()
{
  if (mControlFrame)
    mControlFrame->HandleRedisplayTextEvent();
  return NS_OK;
}

class nsPresState;

#define FIX_FOR_BUG_53259

// Drop down list event management.
// The combo box uses the following strategy for managing the drop-down list.
// If the combo box or it's arrow button is clicked on the drop-down list is displayed
// If mouse exit's the combo box with the drop-down list displayed the drop-down list
// is asked to capture events
// The drop-down list will capture all events including mouse down and up and will always
// return with ListWasSelected method call regardless of whether an item in the list was
// actually selected.
// The ListWasSelected code will turn off mouse-capture for the drop-down list.
// The drop-down list does not explicitly set capture when it is in the drop-down mode.


//XXX: This is temporary. It simulates pseudo states by using a attribute selector on 

const PRInt32 kSizeNotSet = -1;

/**
 * Helper class that listens to the combo boxes button. If the button is pressed the 
 * combo box is toggled to open or close. this is used by Accessibility which presses
 * that button Programmatically.
 */
class nsComboButtonListener: public nsIDOMMouseListener
{
  public:

  NS_DECL_ISUPPORTS
  NS_IMETHOD HandleEvent(nsIDOMEvent* anEvent) { return PR_FALSE; }
  NS_IMETHOD MouseDown(nsIDOMEvent* aMouseEvent) { return PR_FALSE; }
  NS_IMETHOD MouseUp(nsIDOMEvent* aMouseEvent) { return PR_FALSE; }
  NS_IMETHOD MouseDblClick(nsIDOMEvent* aMouseEvent) { return PR_FALSE; }
  NS_IMETHOD MouseOver(nsIDOMEvent* aMouseEvent) { return PR_FALSE; }
  NS_IMETHOD MouseOut(nsIDOMEvent* aMouseEvent) { return PR_FALSE; }

  NS_IMETHOD MouseClick(nsIDOMEvent* aMouseEvent) 
  {
    mComboBox->ShowDropDown(!mComboBox->IsDroppedDown());
    return NS_OK; 
  }

  nsComboButtonListener(nsComboboxControlFrame* aCombobox) 
  { 
    mComboBox = aCombobox; 
  }

  virtual ~nsComboButtonListener() {}

  nsComboboxControlFrame* mComboBox;
};

NS_IMPL_ISUPPORTS1(nsComboButtonListener, nsIDOMMouseListener)

// static class data member for Bug 32920
nsComboboxControlFrame * nsComboboxControlFrame::mFocused = nsnull;

nsIFrame*
NS_NewComboboxControlFrame(nsIPresShell* aPresShell, nsStyleContext* aContext, PRUint32 aStateFlags)
{
  nsComboboxControlFrame* it = new (aPresShell) nsComboboxControlFrame(aContext);

  if (it) {
    // set the state flags (if any are provided)
    it->AddStateBits(aStateFlags);
  }

  return it;
}

//-----------------------------------------------------------
// Reflow Debugging Macros
// These let us "see" how many reflow counts are happening
//-----------------------------------------------------------
#ifdef DO_REFLOW_COUNTER

#define MAX_REFLOW_CNT 1024
static PRInt32 gTotalReqs    = 0;;
static PRInt32 gTotalReflows = 0;;
static PRInt32 gReflowControlCntRQ[MAX_REFLOW_CNT];
static PRInt32 gReflowControlCnt[MAX_REFLOW_CNT];
static PRInt32 gReflowInx = -1;

#define REFLOW_COUNTER() \
  if (mReflowId > -1) \
    gReflowControlCnt[mReflowId]++;

#define REFLOW_COUNTER_REQUEST() \
  if (mReflowId > -1) \
    gReflowControlCntRQ[mReflowId]++;

#define REFLOW_COUNTER_DUMP(__desc) \
  if (mReflowId > -1) {\
    gTotalReqs    += gReflowControlCntRQ[mReflowId];\
    gTotalReflows += gReflowControlCnt[mReflowId];\
    printf("** Id:%5d %s RF: %d RQ: %d   %d/%d  %5.2f\n", \
           mReflowId, (__desc), \
           gReflowControlCnt[mReflowId], \
           gReflowControlCntRQ[mReflowId],\
           gTotalReflows, gTotalReqs, float(gTotalReflows)/float(gTotalReqs)*100.0f);\
  }

#define REFLOW_COUNTER_INIT() \
  if (gReflowInx < MAX_REFLOW_CNT) { \
    gReflowInx++; \
    mReflowId = gReflowInx; \
    gReflowControlCnt[mReflowId] = 0; \
    gReflowControlCntRQ[mReflowId] = 0; \
  } else { \
    mReflowId = -1; \
  }

// reflow messages
#define REFLOW_DEBUG_MSG(_msg1) printf((_msg1))
#define REFLOW_DEBUG_MSG2(_msg1, _msg2) printf((_msg1), (_msg2))
#define REFLOW_DEBUG_MSG3(_msg1, _msg2, _msg3) printf((_msg1), (_msg2), (_msg3))
#define REFLOW_DEBUG_MSG4(_msg1, _msg2, _msg3, _msg4) printf((_msg1), (_msg2), (_msg3), (_msg4))

#else //-------------

#define REFLOW_COUNTER_REQUEST() 
#define REFLOW_COUNTER() 
#define REFLOW_COUNTER_DUMP(__desc) 
#define REFLOW_COUNTER_INIT() 

#define REFLOW_DEBUG_MSG(_msg) 
#define REFLOW_DEBUG_MSG2(_msg1, _msg2) 
#define REFLOW_DEBUG_MSG3(_msg1, _msg2, _msg3) 
#define REFLOW_DEBUG_MSG4(_msg1, _msg2, _msg3, _msg4) 


#endif

//------------------------------------------
// This is for being VERY noisy
//------------------------------------------
#ifdef DO_VERY_NOISY
#define REFLOW_NOISY_MSG(_msg1) printf((_msg1))
#define REFLOW_NOISY_MSG2(_msg1, _msg2) printf((_msg1), (_msg2))
#define REFLOW_NOISY_MSG3(_msg1, _msg2, _msg3) printf((_msg1), (_msg2), (_msg3))
#define REFLOW_NOISY_MSG4(_msg1, _msg2, _msg3, _msg4) printf((_msg1), (_msg2), (_msg3), (_msg4))
#else
#define REFLOW_NOISY_MSG(_msg) 
#define REFLOW_NOISY_MSG2(_msg1, _msg2) 
#define REFLOW_NOISY_MSG3(_msg1, _msg2, _msg3) 
#define REFLOW_NOISY_MSG4(_msg1, _msg2, _msg3, _msg4) 
#endif

//------------------------------------------
// Displays value in pixels or twips
//------------------------------------------
#ifdef DO_PIXELS
#define PX(__v) __v / 15
#else
#define PX(__v) __v 
#endif

//------------------------------------------------------
//-- Done with macros
//------------------------------------------------------

nsComboboxControlFrame::nsComboboxControlFrame(nsStyleContext* aContext)
  : nsAreaFrame(aContext),
    mDisplayWidth(0)
{
  mListControlFrame            = nsnull;
  mDroppedDown                 = PR_FALSE;
  mDisplayFrame                = nsnull;
  mButtonFrame                 = nsnull;
  mDropdownFrame               = nsnull;

  mInRedisplayText = PR_FALSE;

  mRecentSelectedIndex = NS_SKIP_NOTIFY_INDEX;

  REFLOW_COUNTER_INIT()
}

//--------------------------------------------------------------
nsComboboxControlFrame::~nsComboboxControlFrame()
{
  REFLOW_COUNTER_DUMP("nsCCF");
}

//--------------------------------------------------------------
// Frames are not refcounted, no need to AddRef
NS_IMETHODIMP
nsComboboxControlFrame::QueryInterface(const nsIID& aIID, void** aInstancePtr)
{
  NS_PRECONDITION(aInstancePtr, "null out param");

  if (aIID.Equals(NS_GET_IID(nsIComboboxControlFrame))) {
    *aInstancePtr = static_cast<nsIComboboxControlFrame*>(this);
    return NS_OK;
  } else if (aIID.Equals(NS_GET_IID(nsIFormControlFrame))) {
    *aInstancePtr = static_cast<nsIFormControlFrame*>(this);
    return NS_OK;
  } else if (aIID.Equals(NS_GET_IID(nsIAnonymousContentCreator))) {                                         
    *aInstancePtr = static_cast<nsIAnonymousContentCreator*>(this);
    return NS_OK;   
  } else if (aIID.Equals(NS_GET_IID(nsISelectControlFrame))) {
    *aInstancePtr = static_cast<nsISelectControlFrame*>(this);
    return NS_OK;
  } else if (aIID.Equals(NS_GET_IID(nsIStatefulFrame))) {
    *aInstancePtr = static_cast<nsIStatefulFrame*>(this);
    return NS_OK;
  } else if (aIID.Equals(NS_GET_IID(nsIRollupListener))) {
    *aInstancePtr = static_cast<nsIRollupListener*>(this);
    return NS_OK;
  } else if (aIID.Equals(NS_GET_IID(nsIScrollableViewProvider))) {
    *aInstancePtr = static_cast<nsIScrollableViewProvider*>(this);
    return NS_OK;
  } 
  
  return nsAreaFrame::QueryInterface(aIID, aInstancePtr);
}

#ifdef ACCESSIBILITY
NS_IMETHODIMP nsComboboxControlFrame::GetAccessible(nsIAccessible** aAccessible)
{
  nsCOMPtr<nsIAccessibilityService> accService = do_GetService("@mozilla.org/accessibilityService;1");

  if (accService) {
    nsCOMPtr<nsIDOMNode> node = do_QueryInterface(mContent);
    nsCOMPtr<nsIWeakReference> weakShell(do_GetWeakReference(PresContext()->PresShell()));
    return accService->CreateHTMLComboboxAccessible(node, weakShell, aAccessible);
  }

  return NS_ERROR_FAILURE;
}
#endif

void 
nsComboboxControlFrame::SetFocus(PRBool aOn, PRBool aRepaint)
{
  nsWeakFrame weakFrame(this);
  if (aOn) {
    nsListControlFrame::ComboboxFocusSet();
    mFocused = this;
  } else {
    mFocused = nsnull;
    if (mDroppedDown) {
      mListControlFrame->ComboboxFinish(mDisplayedIndex); // might destroy us
      if (!weakFrame.IsAlive()) {
        return;
      }
    }
    // May delete |this|.
    mListControlFrame->FireOnChange();
  }

  if (!weakFrame.IsAlive()) {
    return;
  }

  // This is needed on a temporary basis. It causes the focus
  // rect to be drawn. This is much faster than ReResolvingStyle
  // Bug 32920
  Invalidate(nsRect(0,0,mRect.width,mRect.height), PR_FALSE);

  // Make sure the content area gets updated for where the dropdown was
  // This is only needed for embedding, the focus may go to 
  // the chrome that is not part of the Gecko system (Bug 83493)
  // XXX this is rather inefficient
  nsIViewManager* vm = PresContext()->GetViewManager();
  if (vm) {
    vm->UpdateAllViews(NS_VMREFRESH_NO_SYNC);
  }
}

void
nsComboboxControlFrame::ShowPopup(PRBool aShowPopup)
{
  nsIView* view = mDropdownFrame->GetView();
  nsIViewManager* viewManager = view->GetViewManager();

  if (aShowPopup) {
    nsRect rect = mDropdownFrame->GetRect();
    rect.x = rect.y = 0;
    viewManager->ResizeView(view, rect);
    viewManager->SetViewVisibility(view, nsViewVisibility_kShow);
  } else {
    viewManager->SetViewVisibility(view, nsViewVisibility_kHide);
    nsRect emptyRect(0, 0, 0, 0);
    viewManager->ResizeView(view, emptyRect);
  }

  // fire a popup dom event
  nsEventStatus status = nsEventStatus_eIgnore;
  nsMouseEvent event(PR_TRUE, aShowPopup ?
                     NS_XUL_POPUP_SHOWING : NS_XUL_POPUP_HIDING, nsnull,
                     nsMouseEvent::eReal);

  nsCOMPtr<nsIPresShell> shell = PresContext()->GetPresShell();
  if (shell) 
    shell->HandleDOMEventWithTarget(mContent, &event, &status);
}

PRBool
nsComboboxControlFrame::ShowList(nsPresContext* aPresContext, PRBool aShowList)
{
  nsCOMPtr<nsIPresShell> shell = PresContext()->GetPresShell();

  nsWeakFrame weakFrame(this);
  ShowPopup(aShowList);  // might destroy us
  if (!weakFrame.IsAlive()) {
    return PR_FALSE;
  }

  mDroppedDown = aShowList;
  if (mDroppedDown) {
    // The listcontrol frame will call back to the nsComboboxControlFrame's
    // ListWasSelected which will stop the capture.
    mListControlFrame->AboutToDropDown();
    mListControlFrame->CaptureMouseEvents(PR_TRUE);
  }

  // XXXbz so why do we need to flush here, exactly?
  shell->GetDocument()->FlushPendingNotifications(Flush_Layout);
  if (!weakFrame.IsAlive()) {
    return PR_FALSE;
  }

  nsIFrame* listFrame;
  CallQueryInterface(mListControlFrame, &listFrame);
  if (listFrame) {
    nsIView* view = listFrame->GetView();
    NS_ASSERTION(view, "nsComboboxControlFrame view is null");
    if (view) {
      nsIWidget* widget = view->GetWidget();
      if (widget)
        widget->CaptureRollupEvents(this, mDroppedDown, mDroppedDown);
    }
  }

  return weakFrame.IsAlive();
}

nsresult
nsComboboxControlFrame::ReflowDropdown(nsPresContext*  aPresContext, 
                                       const nsHTMLReflowState& aReflowState)
{
  // All we want out of it later on, really, is the height of a row, so we
  // don't even need to cache mDropdownFrame's ascent or anything.  If we don't
  // need to reflow it, just bail out here.
  if (!aReflowState.ShouldReflowAllKids() &&
      !NS_SUBTREE_DIRTY(mDropdownFrame)) {
    return NS_OK;
  }

  // XXXbz this will, for small-height dropdowns, have extra space on the right
  // edge for the scrollbar we don't show... but that's the best we can do here
  // for now.
  nsSize availSize(aReflowState.availableWidth, NS_UNCONSTRAINEDSIZE);
  nsHTMLReflowState kidReflowState(aPresContext, aReflowState, mDropdownFrame,
                                   availSize);

  // If the dropdown's intrinsic width is narrower than our specified width,
  // then expand it out.  We want our border-box width to end up the same as
  // the dropdown's so account for both sets of mComputedBorderPadding.
  nscoord forcedWidth = aReflowState.ComputedWidth() +
    aReflowState.mComputedBorderPadding.LeftRight() -
    kidReflowState.mComputedBorderPadding.LeftRight();
  kidReflowState.SetComputedWidth(PR_MAX(kidReflowState.ComputedWidth(),
                                         forcedWidth));

  // ensure we start off hidden
  if (GetStateBits() & NS_FRAME_FIRST_REFLOW) {
    nsIView* view = mDropdownFrame->GetView();
    nsIViewManager* viewManager = view->GetViewManager();
    viewManager->SetViewVisibility(view, nsViewVisibility_kHide);
    nsRect emptyRect(0, 0, 0, 0);
    viewManager->ResizeView(view, emptyRect);
  }
  
  // Allow the child to move/size/change-visibility its view if it's currently
  // dropped down
  PRInt32 flags = NS_FRAME_NO_MOVE_VIEW | NS_FRAME_NO_VISIBILITY | NS_FRAME_NO_SIZE_VIEW;
  if (mDroppedDown) {
    flags = 0;
  }
  nsRect rect = mDropdownFrame->GetRect();
  nsHTMLReflowMetrics desiredSize;
  nsReflowStatus ignoredStatus;
  nsresult rv = ReflowChild(mDropdownFrame, aPresContext, desiredSize,
                            kidReflowState, rect.x, rect.y, flags,
                            ignoredStatus);
 
   // Set the child's width and height to it's desired size
  FinishReflowChild(mDropdownFrame, aPresContext, &kidReflowState,
                    desiredSize, rect.x, rect.y, flags);
  return rv;
}

void
nsComboboxControlFrame::AbsolutelyPositionDropDown()
{
   // Position the dropdown list. It is positioned below the display frame if there is enough
   // room on the screen to display the entire list. Otherwise it is placed above the display
   // frame.

   // Note: As first glance, it appears that you could simply get the absolute bounding box for the
   // dropdown list by first getting its view, then getting the view's nsIWidget, then asking the nsIWidget
   // for it's AbsoluteBounds. The problem with this approach, is that the dropdown lists y location can
   // change based on whether the dropdown is placed below or above the display frame.
   // The approach, taken here is to get use the absolute position of the display frame and use it's location
   // to determine if the dropdown will go offscreen.

   // Use the height calculated for the area frame so it includes both
   // the display and button heights.
  nscoord dropdownYOffset = GetRect().height;
  nsPresContext* presContext = PresContext();
// XXX: Enable this code to debug popping up above the display frame, rather than below it
  nsSize dropdownSize = mDropdownFrame->GetSize();

  nscoord screenHeightInPixels = 0;
  if (NS_SUCCEEDED(nsFormControlFrame::GetScreenHeight(presContext, screenHeightInPixels))) {
    // Get the height of the dropdown list in pixels.
    nscoord absoluteDropDownHeight = presContext->AppUnitsToDevPixels(dropdownSize.height);
    // Check to see if the drop-down list will go offscreen
    if (GetScreenRect().YMost() + absoluteDropDownHeight > screenHeightInPixels) {
      // move the dropdown list up
      dropdownYOffset = - (dropdownSize.height);
    }
  }

  nsPoint dropdownPosition;
  const nsStyleVisibility* vis = GetStyleVisibility();
  if (vis->mDirection == NS_STYLE_DIRECTION_RTL) {
    // Align the right edge of the drop-down with the right edge of the control.
    dropdownPosition.x = GetRect().width - dropdownSize.width;
  } else {
    dropdownPosition.x = 0;
  }
  dropdownPosition.y = dropdownYOffset; 

  mDropdownFrame->SetPosition(dropdownPosition);
}

//----------------------------------------------------------
// 
//----------------------------------------------------------
#ifdef DO_REFLOW_DEBUG
static int myCounter = 0;

static void printSize(char * aDesc, nscoord aSize) 
{
  printf(" %s: ", aDesc);
  if (aSize == NS_UNCONSTRAINEDSIZE) {
    printf("UC");
  } else {
    printf("%d", PX(aSize));
  }
}
#endif

//-------------------------------------------------------------------
//-- Main Reflow for the Combobox
//-------------------------------------------------------------------

nscoord
nsComboboxControlFrame::GetIntrinsicWidth(nsIRenderingContext* aRenderingContext,
                                          nsLayoutUtils::IntrinsicWidthType aType)
{
  // get the scrollbar width, we'll use this later
  nscoord scrollbarWidth = 0;
  nsPresContext* presContext = PresContext();
  if (mListControlFrame) {
    nsIScrollableFrame* scrollable;
    CallQueryInterface(mListControlFrame, &scrollable);
    NS_ASSERTION(scrollable, "List must be a scrollable frame");
    nsBoxLayoutState bls(presContext, aRenderingContext);
    scrollbarWidth = scrollable->GetDesiredScrollbarSizes(&bls).LeftRight();
  }

  nscoord displayWidth = 0;
  if (NS_LIKELY(mDisplayFrame)) {
    displayWidth = nsLayoutUtils::IntrinsicForContainer(aRenderingContext,
                                                        mDisplayFrame,
                                                        aType);
  }

  if (mDropdownFrame) {
    nscoord dropdownContentWidth;
    if (aType == nsLayoutUtils::MIN_WIDTH) {
      dropdownContentWidth = mDropdownFrame->GetMinWidth(aRenderingContext);
    } else {
      NS_ASSERTION(aType == nsLayoutUtils::PREF_WIDTH, "Unexpected type");
      dropdownContentWidth = mDropdownFrame->GetPrefWidth(aRenderingContext);
    }
    dropdownContentWidth = NSCoordSaturatingSubtract(dropdownContentWidth, 
                                                     scrollbarWidth,
                                                     nscoord_MAX);
  
    displayWidth = PR_MAX(dropdownContentWidth, displayWidth);
  }

  // add room for the dropmarker button if there is one
  if (!IsThemed() || presContext->GetTheme()->ThemeNeedsComboboxDropmarker())
    displayWidth += scrollbarWidth;

  return displayWidth;

}

nscoord
nsComboboxControlFrame::GetMinWidth(nsIRenderingContext *aRenderingContext)
{
  nscoord minWidth;
  DISPLAY_MIN_WIDTH(this, minWidth);
  minWidth = GetIntrinsicWidth(aRenderingContext, nsLayoutUtils::MIN_WIDTH);
  return minWidth;
}

nscoord
nsComboboxControlFrame::GetPrefWidth(nsIRenderingContext *aRenderingContext)
{
  nscoord prefWidth;
  DISPLAY_PREF_WIDTH(this, prefWidth);
  prefWidth = GetIntrinsicWidth(aRenderingContext, nsLayoutUtils::PREF_WIDTH);
  return prefWidth;
}

NS_IMETHODIMP 
nsComboboxControlFrame::Reflow(nsPresContext*          aPresContext, 
                               nsHTMLReflowMetrics&     aDesiredSize,
                               const nsHTMLReflowState& aReflowState, 
                               nsReflowStatus&          aStatus)
{
  // Constraints we try to satisfy:

  // 1) Default width of button is the vertical scrollbar size
  // 2) If the width of button is bigger than our width, set width of
  //    button to 0.
  // 3) Default height of button is height of display area
  // 4) Width of display area is whatever is left over from our width after
  //    allocating width for the button.
  // 5) Height of display area is GetHeightOfARow() on the
  //    mListControlFrame.

  if (!mDisplayFrame || !mButtonFrame || !mDropdownFrame) {
    NS_ERROR("Why did the frame constructor allow this to happen?  Fix it!!");
    return NS_ERROR_UNEXPECTED;
  }

  // Make sure the displayed text is the same as the selected option, bug 297389.
  PRInt32 selectedIndex;
  nsAutoString selectedOptionText;
  if (!mDroppedDown) {
    selectedIndex = mListControlFrame->GetSelectedIndex();
  }
  else {
    // In dropped down mode the "selected index" is the hovered menu item,
    // we want the last selected item which is |mDisplayedIndex| in this case.
    selectedIndex = mDisplayedIndex;
  }
  if (selectedIndex != -1) {
    mListControlFrame->GetOptionText(selectedIndex, selectedOptionText);
  }
  if (mDisplayedOptionText != selectedOptionText) {
    RedisplayText(selectedIndex);
  }

  // First reflow our dropdown so that we know how tall we should be.
  ReflowDropdown(aPresContext, aReflowState);

  // Get the width of the vertical scrollbar.  That will be the width of the
  // dropdown button.
  nscoord buttonWidth;
  const nsStyleDisplay *disp = GetStyleDisplay();
  if (IsThemed(disp) && !aPresContext->GetTheme()->ThemeNeedsComboboxDropmarker()) {
    buttonWidth = 0;
  }
  else {
    nsIScrollableFrame* scrollable;
    CallQueryInterface(mListControlFrame, &scrollable);
    NS_ASSERTION(scrollable, "List must be a scrollable frame");
    nsBoxLayoutState bls(PresContext(), aReflowState.rendContext);
    buttonWidth = scrollable->GetDesiredScrollbarSizes(&bls).LeftRight();
    if (buttonWidth > aReflowState.ComputedWidth()) {
      buttonWidth = 0;
    }
  }

  mDisplayWidth = aReflowState.ComputedWidth() - buttonWidth;

  nsresult rv = nsAreaFrame::Reflow(aPresContext, aDesiredSize, aReflowState,
                                    aStatus);
  NS_ENSURE_SUCCESS(rv, rv);

  // Now set the correct width and height on our button.  The width we need to
  // set always, the height only if we had an auto height.
  nsRect buttonRect = mButtonFrame->GetRect();
  // If we have a non-intrinsic computed height, our kids should have sized
  // themselves properly on their own.
  if (aReflowState.ComputedHeight() == NS_INTRINSICSIZE) {
    // The display frame is going to be the right height and width at this
    // point. Use its height as the button height.
    nsRect displayRect = mDisplayFrame->GetRect();
    buttonRect.height = displayRect.height;
    buttonRect.y = displayRect.y;
  }
#ifdef DEBUG
  else {
    nscoord buttonHeight = mButtonFrame->GetSize().height;
    nscoord displayHeight = mDisplayFrame->GetSize().height;

    // The button and display area should be equal heights, unless the computed
    // height on the combobox is too small to fit their borders and padding.
    NS_ASSERTION(buttonHeight == displayHeight ||
                 (aReflowState.ComputedHeight() < buttonHeight &&
                  buttonHeight ==
                    mButtonFrame->GetUsedBorderAndPadding().TopBottom()) ||
                 (aReflowState.ComputedHeight() < displayHeight &&
                  displayHeight ==
                    mDisplayFrame->GetUsedBorderAndPadding().TopBottom()),
                 "Different heights?");
  }
#endif
  
  if (GetStyleVisibility()->mDirection == NS_STYLE_DIRECTION_RTL) {
    // Make sure the right edge of the button frame stays where it is now
    buttonRect.x -= buttonWidth - buttonRect.width;
  }
  buttonRect.width = buttonWidth;
  mButtonFrame->SetRect(buttonRect);
  
  return rv;
}

//--------------------------------------------------------------

nsIAtom*
nsComboboxControlFrame::GetType() const
{
  return nsGkAtoms::comboboxControlFrame; 
}

#ifdef NS_DEBUG
NS_IMETHODIMP
nsComboboxControlFrame::GetFrameName(nsAString& aResult) const
{
  return MakeFrameName(NS_LITERAL_STRING("ComboboxControl"), aResult);
}
#endif


//----------------------------------------------------------------------
// nsIComboboxControlFrame
//----------------------------------------------------------------------
void
nsComboboxControlFrame::ShowDropDown(PRBool aDoDropDown) 
{
  if (mContent->HasAttr(kNameSpaceID_None, nsGkAtoms::disabled)) {
    return;
  }

  if (!mDroppedDown && aDoDropDown) {
    if (mListControlFrame) {
      mListControlFrame->SyncViewWithFrame();
    }
    ShowList(PresContext(), aDoDropDown); // might destroy us
  } else if (mDroppedDown && !aDoDropDown) {
    ShowList(PresContext(), aDoDropDown); // might destroy us
  }
}

void
nsComboboxControlFrame::SetDropDown(nsIFrame* aDropDownFrame)
{
  mDropdownFrame = aDropDownFrame;
 
  CallQueryInterface(mDropdownFrame, &mListControlFrame);
}

nsIFrame*
nsComboboxControlFrame::GetDropDown() 
{
  return mDropdownFrame;
}

///////////////////////////////////////////////////////////////

NS_IMETHODIMP
nsComboboxControlFrame::RedisplaySelectedText()
{
  return RedisplayText(mListControlFrame->GetSelectedIndex());
}

nsresult
nsComboboxControlFrame::RedisplayText(PRInt32 aIndex)
{
  // Get the text to display
  if (aIndex != -1) {
    mListControlFrame->GetOptionText(aIndex, mDisplayedOptionText);
  } else {
    mDisplayedOptionText.Truncate();
  }
  mDisplayedIndex = aIndex;

  REFLOW_DEBUG_MSG2("RedisplayText \"%s\"\n",
                    NS_LossyConvertUTF16toASCII(mDisplayedOptionText).get());

  // Send reflow command because the new text maybe larger
  nsresult rv = NS_OK;
  if (mDisplayContent) {
    // Don't call ActuallyDisplayText(PR_TRUE) directly here since that
    // could cause recursive frame construction. See bug 283117 and the comment in
    // HandleRedisplayTextEvent() below.

    // Revoke outstanding events to avoid out-of-order events which could mean
    // displaying the wrong text.
    mRedisplayTextEvent.Revoke();

    nsRefPtr<RedisplayTextEvent> event = new RedisplayTextEvent(this);
    rv = NS_DispatchToCurrentThread(event);
    if (NS_SUCCEEDED(rv))
      mRedisplayTextEvent = event;
  }
  return rv;
}

void
nsComboboxControlFrame::HandleRedisplayTextEvent()
{
  // First, make sure that the content model is up to date and we've
  // constructed the frames for all our content in the right places.
  // Otherwise they'll end up under the wrong insertion frame when we
  // ActuallyDisplayText, since that flushes out the content sink by
  // calling SetText on a DOM node with aNotify set to true.  See bug
  // 289730.
  PresContext()->Document()->
    FlushPendingNotifications(Flush_ContentAndNotify);
  
  // Redirect frame insertions during this method (see GetContentInsertionFrame())
  // so that any reframing that the frame constructor forces upon us is inserted
  // into the correct parent (mDisplayFrame). See bug 282607.
  NS_PRECONDITION(!mInRedisplayText, "Nested RedisplayText");
  mInRedisplayText = PR_TRUE;
  mRedisplayTextEvent.Forget();

  ActuallyDisplayText(PR_TRUE);
  // XXXbz This should perhaps be eResize.  Check.
  PresContext()->PresShell()->FrameNeedsReflow(mDisplayFrame,
                                               nsIPresShell::eStyleChange,
                                               NS_FRAME_IS_DIRTY);

  mInRedisplayText = PR_FALSE;
}

void
nsComboboxControlFrame::ActuallyDisplayText(PRBool aNotify)
{
  if (mDisplayedOptionText.IsEmpty()) {
    // Have to use a non-breaking space for line-height calculations
    // to be right
    static const PRUnichar space = 0xA0;
    mDisplayContent->SetText(&space, 1, aNotify);
  } else {
    mDisplayContent->SetText(mDisplayedOptionText, aNotify);
  }
}

PRInt32
nsComboboxControlFrame::GetIndexOfDisplayArea()
{
  return mDisplayedIndex;
}

//----------------------------------------------------------------------
// nsISelectControlFrame
//----------------------------------------------------------------------
NS_IMETHODIMP
nsComboboxControlFrame::DoneAddingChildren(PRBool aIsDone)
{
  nsISelectControlFrame* listFrame = nsnull;
  nsresult rv = NS_ERROR_FAILURE;
  if (mDropdownFrame != nsnull) {
    rv = CallQueryInterface(mDropdownFrame, &listFrame);
    if (listFrame) {
      rv = listFrame->DoneAddingChildren(aIsDone);
    }
  }
  return rv;
}

NS_IMETHODIMP
nsComboboxControlFrame::AddOption(nsPresContext* aPresContext, PRInt32 aIndex)
{
  if (aIndex <= mDisplayedIndex) {
    ++mDisplayedIndex;
  }

  nsListControlFrame* lcf = static_cast<nsListControlFrame*>(mDropdownFrame);
  return lcf->AddOption(aPresContext, aIndex);
}
  

NS_IMETHODIMP
nsComboboxControlFrame::RemoveOption(nsPresContext* aPresContext, PRInt32 aIndex)
{
  if (mListControlFrame->GetNumberOfOptions() > 0) {
    if (aIndex < mDisplayedIndex) {
      --mDisplayedIndex;
    } else if (aIndex == mDisplayedIndex) {
      mDisplayedIndex = 0; // IE6 compat
      RedisplayText(mDisplayedIndex);
    }
  }
  else {
    // If we removed the last option, we need to blank things out
    RedisplayText(-1);
  }

  nsListControlFrame* lcf = static_cast<nsListControlFrame*>(mDropdownFrame);
  return lcf->RemoveOption(aPresContext, aIndex);
}

NS_IMETHODIMP
nsComboboxControlFrame::GetOptionSelected(PRInt32 aIndex, PRBool* aValue)
{
  nsISelectControlFrame* listFrame = nsnull;
  NS_ASSERTION(mDropdownFrame, "No dropdown frame!");

  CallQueryInterface(mDropdownFrame, &listFrame);
  NS_ASSERTION(listFrame, "No list frame!");

  return listFrame->GetOptionSelected(aIndex, aValue);
}

NS_IMETHODIMP
nsComboboxControlFrame::OnSetSelectedIndex(PRInt32 aOldIndex, PRInt32 aNewIndex)
{
  RedisplayText(aNewIndex);
  
  nsISelectControlFrame* listFrame = nsnull;
  NS_ASSERTION(mDropdownFrame, "No dropdown frame!");

  CallQueryInterface(mDropdownFrame, &listFrame);
  NS_ASSERTION(listFrame, "No list frame!");

  return listFrame->OnSetSelectedIndex(aOldIndex, aNewIndex);
}

// End nsISelectControlFrame
//----------------------------------------------------------------------

NS_IMETHODIMP 
nsComboboxControlFrame::HandleEvent(nsPresContext* aPresContext, 
                                       nsGUIEvent*     aEvent,
                                       nsEventStatus*  aEventStatus)
{
  NS_ENSURE_ARG_POINTER(aEventStatus);

  if (nsEventStatus_eConsumeNoDefault == *aEventStatus) {
    return NS_OK;
  }
  if (mContent->HasAttr(kNameSpaceID_None, nsGkAtoms::disabled)) {
    return NS_OK;
  }

  // If we have style that affects how we are selected, feed event down to
  // nsFrame::HandleEvent so that selection takes place when appropriate.
  const nsStyleUserInterface* uiStyle = GetStyleUserInterface();
  if (uiStyle->mUserInput == NS_STYLE_USER_INPUT_NONE || uiStyle->mUserInput == NS_STYLE_USER_INPUT_DISABLED)
    return nsAreaFrame::HandleEvent(aPresContext, aEvent, aEventStatus);
    
  return NS_OK;
}


nsresult
nsComboboxControlFrame::SetFormProperty(nsIAtom* aName, const nsAString& aValue)
{
  nsIFormControlFrame* fcFrame = nsnull;
  nsresult result = CallQueryInterface(mDropdownFrame, &fcFrame);
  if (NS_FAILED(result)) {
    return result;
  }
  if (fcFrame) {
    return fcFrame->SetFormProperty(aName, aValue);
  }
  return NS_OK;
}

nsresult 
nsComboboxControlFrame::GetFormProperty(nsIAtom* aName, nsAString& aValue) const
{
  nsIFormControlFrame* fcFrame = nsnull;
  nsresult result = CallQueryInterface(mDropdownFrame, &fcFrame);
  if(NS_FAILED(result)) {
    return result;
  }
  if (fcFrame) {
    return fcFrame->GetFormProperty(aName, aValue);
  }
  return NS_OK;
}

nsIFrame*
nsComboboxControlFrame::GetContentInsertionFrame() {
  return mInRedisplayText ? mDisplayFrame : mDropdownFrame->GetContentInsertionFrame();
}

nsresult
nsComboboxControlFrame::CreateAnonymousContent(nsTArray<nsIContent*>& aElements)
{
  // The frames used to display the combo box and the button used to popup the dropdown list
  // are created through anonymous content. The dropdown list is not created through anonymous
  // content because it's frame is initialized specifically for the drop-down case and it is placed
  // a special list referenced through NS_COMBO_FRAME_POPUP_LIST_INDEX to keep separate from the
  // layout of the display and button. 
  //
  // Note: The value attribute of the display content is set when an item is selected in the dropdown list.
  // If the content specified below does not honor the value attribute than nothing will be displayed.

  // For now the content that is created corresponds to two input buttons. It would be better to create the
  // tag as something other than input, but then there isn't any way to create a button frame since it
  // isn't possible to set the display type in CSS2 to create a button frame.

    // create content used for display
  //nsIAtom* tag = NS_NewAtom("mozcombodisplay");

  // Add a child text content node for the label

  nsNodeInfoManager *nimgr = mContent->NodeInfo()->NodeInfoManager();

  NS_NewTextNode(getter_AddRefs(mDisplayContent), nimgr);
  if (!mDisplayContent)
    return NS_ERROR_OUT_OF_MEMORY;

  // set the value of the text node
  mDisplayedIndex = mListControlFrame->GetSelectedIndex();
  if (mDisplayedIndex != -1) {
    mListControlFrame->GetOptionText(mDisplayedIndex, mDisplayedOptionText);
  }
  ActuallyDisplayText(PR_FALSE);

  if (!aElements.AppendElement(mDisplayContent))
    return NS_ERROR_OUT_OF_MEMORY;

  nsCOMPtr<nsINodeInfo> nodeInfo;
  nimgr->GetNodeInfo(nsGkAtoms::input, nsnull, kNameSpaceID_None,
                     getter_AddRefs(nodeInfo));

  // create button which drops the list down
  NS_NewHTMLElement(getter_AddRefs(mButtonContent), nodeInfo);
  if (!mButtonContent)
    return NS_ERROR_OUT_OF_MEMORY;

  // make someone to listen to the button. If its pressed by someone like Accessibility
  // then open or close the combo box.
  mButtonListener = new nsComboButtonListener(this);
  if (!mButtonListener)
    return NS_ERROR_OUT_OF_MEMORY;
  mButtonContent->AddEventListenerByIID(mButtonListener,
                                        NS_GET_IID(nsIDOMMouseListener));

  mButtonContent->SetAttr(kNameSpaceID_None, nsGkAtoms::type,
                          NS_LITERAL_STRING("button"), PR_FALSE);
  // Set tabindex="-1" so that the button is not tabbable
  mButtonContent->SetAttr(kNameSpaceID_None, nsGkAtoms::tabindex,
                          NS_LITERAL_STRING("-1"), PR_FALSE);

  if (!aElements.AppendElement(mButtonContent))
    return NS_ERROR_OUT_OF_MEMORY;

  return NS_OK;
}

// XXXbz this is a for-now hack.  Now that display:inline-block works,
// need to revisit this.
class nsComboboxDisplayFrame : public nsBlockFrame {
public:
  nsComboboxDisplayFrame (nsStyleContext* aContext,
                          nsComboboxControlFrame* aComboBox)
    : nsBlockFrame(aContext),
      mComboBox(aComboBox)
  {}

  // Need this so that line layout knows that this block's width
  // depends on the available width.
  virtual nsIAtom* GetType() const;

  virtual PRBool IsFrameOfType(PRUint32 aFlags) const
  {
    return nsBlockFrame::IsFrameOfType(aFlags &
      ~(nsIFrame::eReplacedContainsBlock));
  }

  NS_IMETHOD Reflow(nsPresContext*           aPresContext,
                    nsHTMLReflowMetrics&     aDesiredSize,
                    const nsHTMLReflowState& aReflowState,
                    nsReflowStatus&          aStatus);

  NS_IMETHOD BuildDisplayList(nsDisplayListBuilder*   aBuilder,
                              const nsRect&           aDirtyRect,
                              const nsDisplayListSet& aLists);

protected:
  nsComboboxControlFrame* mComboBox;
};

nsIAtom*
nsComboboxDisplayFrame::GetType() const
{
  return nsGkAtoms::comboboxDisplayFrame;
}

NS_IMETHODIMP
nsComboboxDisplayFrame::Reflow(nsPresContext*           aPresContext,
                               nsHTMLReflowMetrics&     aDesiredSize,
                               const nsHTMLReflowState& aReflowState,
                               nsReflowStatus&          aStatus)
{
  nsHTMLReflowState state(aReflowState);
  if (state.ComputedHeight() == NS_INTRINSICSIZE) {
    // Note that the only way we can have a computed height here is if the
    // combobox had a specified height.  If it didn't, size based on what our
    // rows look like, for lack of anything better.
    state.SetComputedHeight(mComboBox->mListControlFrame->GetHeightOfARow());
  }
  nscoord computedWidth = mComboBox->mDisplayWidth -
    state.mComputedBorderPadding.LeftRight(); 
  if (computedWidth < 0) {
    computedWidth = 0;
  }
  state.SetComputedWidth(computedWidth);

  return nsBlockFrame::Reflow(aPresContext, aDesiredSize, state, aStatus);
}

NS_IMETHODIMP
nsComboboxDisplayFrame::BuildDisplayList(nsDisplayListBuilder*   aBuilder,
                                         const nsRect&           aDirtyRect,
                                         const nsDisplayListSet& aLists)
{
  nsDisplayListCollection set;
  nsresult rv = nsBlockFrame::BuildDisplayList(aBuilder, aDirtyRect, set);
  if (NS_FAILED(rv))
    return rv;

  // remove background items if parent frame is themed
  if (mComboBox->IsThemed()) {
    set.BorderBackground()->DeleteAll();
  }

  set.MoveTo(aLists);

  return NS_OK;
}

nsIFrame*
nsComboboxControlFrame::CreateFrameFor(nsIContent*      aContent)
{ 
  NS_PRECONDITION(nsnull != aContent, "null ptr");

  NS_ASSERTION(mDisplayContent, "mDisplayContent can't be null!");

  if (mDisplayContent != aContent) {
    // We only handle the frames for mDisplayContent here
    return nsnull;
  }
  
  // Get PresShell
  nsIPresShell *shell = PresContext()->PresShell();
  nsStyleSet *styleSet = shell->StyleSet();

  // create the style contexts for the anonymous block frame and text frame
  nsRefPtr<nsStyleContext> styleContext;
  styleContext = styleSet->
    ResolvePseudoStyleFor(mContent, 
                          nsCSSAnonBoxes::mozDisplayComboboxControlFrame,
                          mStyleContext);
  if (NS_UNLIKELY(!styleContext)) {
    return nsnull;
  }

  nsRefPtr<nsStyleContext> textStyleContext;
  textStyleContext = styleSet->ResolveStyleForNonElement(mStyleContext);
  if (NS_UNLIKELY(!textStyleContext)) {
    return nsnull;
  }

  // Start by by creating our anonymous block frame
  mDisplayFrame = new (shell) nsComboboxDisplayFrame(styleContext, this);
  if (NS_UNLIKELY(!mDisplayFrame)) {
    return nsnull;
  }

  nsresult rv = mDisplayFrame->Init(mContent, this, nsnull);
  if (NS_FAILED(rv)) {
    mDisplayFrame->Destroy();
    mDisplayFrame = nsnull;
    return nsnull;
  }

  // Create a text frame and put it inside the block frame
  mTextFrame = NS_NewTextFrame(shell, textStyleContext);
  if (NS_UNLIKELY(!mTextFrame)) {
    return nsnull;
  }

  // initialize the text frame
  rv = mTextFrame->Init(aContent, mDisplayFrame, nsnull);
  if (NS_FAILED(rv)) {
    mDisplayFrame->Destroy();
    mDisplayFrame = nsnull;
    mTextFrame->Destroy();
    mTextFrame = nsnull;
    return nsnull;
  }

  mDisplayFrame->SetInitialChildList(nsnull, mTextFrame);
  return mDisplayFrame;
}

void
nsComboboxControlFrame::Destroy()
{
  // Revoke any pending RedisplayTextEvent
  mRedisplayTextEvent.Revoke();

  nsFormControlFrame::RegUnRegAccessKey(static_cast<nsIFrame*>(this), PR_FALSE);

  if (mDroppedDown) {
    // Get parent view
    nsIFrame * listFrame;
    if (NS_OK == mListControlFrame->QueryInterface(NS_GET_IID(nsIFrame), (void **)&listFrame)) {
      nsIView* view = listFrame->GetView();
      NS_ASSERTION(view, "nsComboboxControlFrame view is null");
      if (view) {
        nsIWidget* widget = view->GetWidget();
        if (widget)
          widget->CaptureRollupEvents(this, PR_FALSE, PR_TRUE);
      }
    }
  }

  // Cleanup frames in popup child list
  mPopupFrames.DestroyFrames();
  nsContentUtils::DestroyAnonymousContent(&mDisplayContent);
  nsContentUtils::DestroyAnonymousContent(&mButtonContent);
  nsAreaFrame::Destroy();
}


nsIFrame*
nsComboboxControlFrame::GetFirstChild(nsIAtom* aListName) const
{
  if (nsGkAtoms::selectPopupList == aListName) {
    return mPopupFrames.FirstChild();
  }
  return nsAreaFrame::GetFirstChild(aListName);
}

NS_IMETHODIMP
nsComboboxControlFrame::SetInitialChildList(nsIAtom*        aListName,
                                            nsIFrame*       aChildList)
{
  nsresult rv = NS_OK;
  if (nsGkAtoms::selectPopupList == aListName) {
    mPopupFrames.SetFrames(aChildList);
  } else {
    rv = nsAreaFrame::SetInitialChildList(aListName, aChildList);

    for (nsIFrame * child = aChildList; child;
         child = child->GetNextSibling()) {
      nsCOMPtr<nsIFormControl> formControl = do_QueryInterface(child->GetContent());
      if (formControl && formControl->GetType() == NS_FORM_INPUT_BUTTON) {
        mButtonFrame = child;
        break;
      }
    }
    NS_ASSERTION(mButtonFrame, "missing button frame in initial child list");
  }
  return rv;
}

#define NS_COMBO_FRAME_POPUP_LIST_INDEX   (NS_BLOCK_LIST_COUNT)

nsIAtom*
nsComboboxControlFrame::GetAdditionalChildListName(PRInt32 aIndex) const
{
   // Maintain a separate child list for the dropdown list (i.e. popup listbox)
   // This is necessary because we don't want the listbox to be included in the layout
   // of the combox's children because it would take up space, when it is suppose to
   // be floating above the display.
  if (aIndex < NS_BLOCK_LIST_COUNT) {
    return nsAreaFrame::GetAdditionalChildListName(aIndex);
  }
  
  if (NS_COMBO_FRAME_POPUP_LIST_INDEX == aIndex) {
    return nsGkAtoms::selectPopupList;
  }
  return nsnull;
}

//----------------------------------------------------------------------
  //nsIRollupListener
//----------------------------------------------------------------------
NS_IMETHODIMP 
nsComboboxControlFrame::Rollup(nsIContent** aLastRolledUp)
{
  if (aLastRolledUp)
    *aLastRolledUp = nsnull;

  if (mDroppedDown) {
    nsWeakFrame weakFrame(this);
    mListControlFrame->AboutToRollup(); // might destroy us
    if (!weakFrame.IsAlive())
      return NS_OK;
    ShowDropDown(PR_FALSE); // might destroy us
    if (!weakFrame.IsAlive())
      return NS_OK;
    mListControlFrame->CaptureMouseEvents(PR_FALSE);
  }
  return NS_OK;
}

void
nsComboboxControlFrame::RollupFromList()
{
  if (ShowList(PresContext(), PR_FALSE))
    mListControlFrame->CaptureMouseEvents(PR_FALSE);
}

PRInt32
nsComboboxControlFrame::UpdateRecentIndex(PRInt32 aIndex)
{
  PRInt32 index = mRecentSelectedIndex;
  if (mRecentSelectedIndex == NS_SKIP_NOTIFY_INDEX || aIndex == NS_SKIP_NOTIFY_INDEX)
    mRecentSelectedIndex = aIndex;
  return index;
}

class nsDisplayComboboxFocus : public nsDisplayItem {
public:
  nsDisplayComboboxFocus(nsComboboxControlFrame* aFrame)
    : nsDisplayItem(aFrame) {
    MOZ_COUNT_CTOR(nsDisplayComboboxFocus);
  }
#ifdef NS_BUILD_REFCNT_LOGGING
  virtual ~nsDisplayComboboxFocus() {
    MOZ_COUNT_DTOR(nsDisplayComboboxFocus);
  }
#endif

  virtual void Paint(nsDisplayListBuilder* aBuilder, nsIRenderingContext* aCtx,
     const nsRect& aDirtyRect);
  NS_DISPLAY_DECL_NAME("ComboboxFocus")
};

void nsDisplayComboboxFocus::Paint(nsDisplayListBuilder* aBuilder,
     nsIRenderingContext* aCtx, const nsRect& aDirtyRect)
{
  static_cast<nsComboboxControlFrame*>(mFrame)
    ->PaintFocus(*aCtx, aBuilder->ToReferenceFrame(mFrame));
}

NS_IMETHODIMP
nsComboboxControlFrame::BuildDisplayList(nsDisplayListBuilder*   aBuilder,
                                         const nsRect&           aDirtyRect,
                                         const nsDisplayListSet& aLists)
{
#ifdef NOISY
  printf("%p paint at (%d, %d, %d, %d)\n", this,
    aDirtyRect.x, aDirtyRect.y, aDirtyRect.width, aDirtyRect.height);
#endif

  if (aBuilder->IsForEventDelivery()) {
    // Don't allow children to receive events.
    // REVIEW: following old GetFrameForPoint
    nsresult rv = DisplayBorderBackgroundOutline(aBuilder, aLists);
    NS_ENSURE_SUCCESS(rv, rv);
  } else {
    // REVIEW: Our in-flow child frames are inline-level so they will paint in our
    // content list, so we don't need to mess with layers.
    nsresult rv = nsAreaFrame::BuildDisplayList(aBuilder, aDirtyRect, aLists);
    NS_ENSURE_SUCCESS(rv, rv);
  }

  nsPresContext *presContext = PresContext();
  const nsStyleDisplay *disp = GetStyleDisplay();
  if ((!IsThemed(disp) ||
       !presContext->GetTheme()->ThemeDrawsFocusForWidget(presContext, this, disp->mAppearance)) &&
      mDisplayFrame && IsVisibleForPainting(aBuilder)) {
    nsresult rv = aLists.Content()->AppendNewToTop(new (aBuilder)
                                                   nsDisplayComboboxFocus(this));
    NS_ENSURE_SUCCESS(rv, rv);
  }

  return DisplaySelectionOverlay(aBuilder, aLists);
}

void nsComboboxControlFrame::PaintFocus(nsIRenderingContext& aRenderingContext,
                                        nsPoint aPt) {
  aRenderingContext.PushState();
  nsRect clipRect = mDisplayFrame->GetRect() + aPt;
  aRenderingContext.SetClipRect(clipRect, nsClipCombine_kIntersect);

  // REVIEW: Why does the old code paint mDisplayFrame again? We've
  // already painted it in the children above. So clipping it here won't do
  // us much good.

  /////////////////////
  // draw focus
  // XXX This is only temporary
  if (!mContent->HasAttr(kNameSpaceID_None, nsGkAtoms::disabled) &&
      mFocused == this) {
    aRenderingContext.SetLineStyle(nsLineStyle_kDotted);
    aRenderingContext.SetColor(GetStyleColor()->mColor);
  } else {
    aRenderingContext.SetColor(GetStyleBackground()->mBackgroundColor);
    aRenderingContext.SetLineStyle(nsLineStyle_kSolid);
  }
  //aRenderingContext.DrawRect(clipRect);
  nscoord onePixel = nsPresContext::CSSPixelsToAppUnits(1);
  clipRect.width -= onePixel;
  clipRect.height -= onePixel;
  aRenderingContext.DrawLine(clipRect.x, clipRect.y, 
                             clipRect.x+clipRect.width, clipRect.y);
  aRenderingContext.DrawLine(clipRect.x+clipRect.width, clipRect.y, 
                             clipRect.x+clipRect.width, clipRect.y+clipRect.height);
  aRenderingContext.DrawLine(clipRect.x+clipRect.width, clipRect.y+clipRect.height, 
                             clipRect.x, clipRect.y+clipRect.height);
  aRenderingContext.DrawLine(clipRect.x, clipRect.y+clipRect.height, 
                             clipRect.x, clipRect.y);
  aRenderingContext.DrawLine(clipRect.x, clipRect.y+clipRect.height, 
                             clipRect.x, clipRect.y);

  aRenderingContext.PopState();
}

//----------------------------------------------------------------------
  //nsIScrollableViewProvider
//----------------------------------------------------------------------
nsIScrollableView* nsComboboxControlFrame::GetScrollableView()
{
  if (!mDropdownFrame)
    return nsnull;

  nsIScrollableFrame* scrollable = nsnull;
  nsresult rv = CallQueryInterface(mDropdownFrame, &scrollable);
  if (NS_FAILED(rv))
    return nsnull;

  return scrollable->GetScrollableView();
}

//---------------------------------------------------------
// gets the content (an option) by index and then set it as
// being selected or not selected
//---------------------------------------------------------
NS_IMETHODIMP
nsComboboxControlFrame::OnOptionSelected(nsPresContext* aPresContext,
                                         PRInt32 aIndex,
                                         PRBool aSelected)
{
  if (mDroppedDown) {
    nsCOMPtr<nsISelectControlFrame> selectFrame
                                     = do_QueryInterface(mListControlFrame);
    if (selectFrame) {
      selectFrame->OnOptionSelected(aPresContext, aIndex, aSelected);
    }
  } else {
    if (aSelected) {
      RedisplayText(aIndex);
    } else {
      RedisplaySelectedText();
      FireValueChangeEvent(); // Fire after old option is unselected
    }
  }

  return NS_OK;
}

void nsComboboxControlFrame::FireValueChangeEvent()
{
  // Fire ValueChange event to indicate data value of combo box has changed
  nsCOMPtr<nsIDOMEvent> event;
  nsPresContext* presContext = PresContext();
  if (NS_SUCCEEDED(nsEventDispatcher::CreateEvent(presContext, nsnull,
                                                  NS_LITERAL_STRING("Events"),
                                                  getter_AddRefs(event)))) {
    event->InitEvent(NS_LITERAL_STRING("ValueChange"), PR_TRUE, PR_TRUE);

    nsCOMPtr<nsIPrivateDOMEvent> privateEvent(do_QueryInterface(event));
    privateEvent->SetTrusted(PR_TRUE);
    nsEventDispatcher::DispatchDOMEvent(mContent, nsnull, event, nsnull,
                                        nsnull);
  }
}

void
nsComboboxControlFrame::OnContentReset()
{
  if (mListControlFrame) {
    mListControlFrame->OnContentReset();
  }
}


//--------------------------------------------------------
// nsIStatefulFrame
//--------------------------------------------------------
NS_IMETHODIMP
nsComboboxControlFrame::SaveState(SpecialStateID aStateID,
                                  nsPresState** aState)
{
  if (!mListControlFrame)
    return NS_ERROR_FAILURE;

  nsIStatefulFrame* stateful;
  CallQueryInterface(mListControlFrame, &stateful);
  return stateful->SaveState(aStateID, aState);
}

NS_IMETHODIMP
nsComboboxControlFrame::RestoreState(nsPresState* aState)
{
  if (!mListControlFrame)
    return NS_ERROR_FAILURE;

  nsIStatefulFrame* stateful;
  nsresult rv = CallQueryInterface(mListControlFrame, &stateful);
  NS_ASSERTION(NS_SUCCEEDED(rv), "Must implement nsIStatefulFrame");
  rv = stateful->RestoreState(aState);
  return rv;
}


//
// Camino uses a native widget for the combobox
// popup, which affects drawing and event
// handling here and in nsListControlFrame.
// 

/* static */
PRBool
nsComboboxControlFrame::ToolkitHasNativePopup()
{
#ifdef XP_MACOSX
  return nsContentUtils::GetBoolPref("ui.use_native_popup_windows");
#else
  return PR_FALSE;
#endif
}

