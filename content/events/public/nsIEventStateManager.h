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

#ifndef nsIEventStateManager_h__
#define nsIEventStateManager_h__

#include "nsEvent.h"
#include "nsISupports.h"

class nsIContent;
class nsPresContext;
class nsIDOMEvent;
class nsIFrame;
class nsIView;
class nsIWidget;
class imgIContainer;

/*
 * Event state manager interface.
 */
// {14E5E066-E60A-446b-965F-A9ECAA7098F3}
#define NS_IEVENTSTATEMANAGER_IID \
{ 0x14e5e066, 0xe60a, 0x446b, \
  { 0x96, 0x5f, 0xa9, 0xec, 0xaa, 0x70, 0x98, 0xf3 } }


#define NS_EVENT_NEEDS_FRAME(event) (!NS_IS_FOCUS_EVENT(event))

class nsIEventStateManager : public nsISupports {

public:
  enum EFocusedWithType {
    eEventFocusedByUnknown,     // focus gained via unknown method
    eEventFocusedByMouse,       // focus gained via mouse
    eEventFocusedByKey,         // focus gained via key press (like tab)
    eEventFocusedByContextMenu, // focus gained via context menu
    eEventFocusedByApplication  // focus gained via Application (like script)
  };

  NS_DECLARE_STATIC_IID_ACCESSOR(NS_IEVENTSTATEMANAGER_IID)

  NS_IMETHOD Init() = 0;

  NS_IMETHOD PreHandleEvent(nsPresContext* aPresContext, 
                            nsEvent *aEvent, 
                            nsIFrame* aTargetFrame,
                            nsEventStatus* aStatus,
                            nsIView* aView) = 0;

  NS_IMETHOD PostHandleEvent(nsPresContext* aPresContext, 
                             nsEvent *aEvent, 
                             nsIFrame* aTargetFrame,
                             nsEventStatus* aStatus,
                             nsIView* aView) = 0;

  NS_IMETHOD SetPresContext(nsPresContext* aPresContext) = 0;
  NS_IMETHOD ClearFrameRefs(nsIFrame* aFrame) = 0;

  NS_IMETHOD GetEventTarget(nsIFrame **aFrame) = 0;
  NS_IMETHOD GetEventTargetContent(nsEvent* aEvent, nsIContent** aContent) = 0;
  NS_IMETHOD GetEventRelatedContent(nsIContent** aContent) = 0;

  NS_IMETHOD GetContentState(nsIContent *aContent, PRInt32& aState) = 0;

  /**
   * Notify that the given NS_EVENT_STATE_* bit has changed for this content.
   * @param aContent Content which has changed states
   * @param aState   Corresponding state flags such as NS_EVENT_STATE_FOCUS 
   *                 defined in the nsIEventStateManager interface
   * @return  Whether the content was able to change all states. Returns PR_FALSE
   *                  if a resulting DOM event causes the content node passed in
   *                  to not change states. Note, the frame for the content may
   *                  change as a result of the content state change, because of
   *                  frame reconstructions that may occur, but this does not
   *                  affect the return value.
   */
  virtual PRBool SetContentState(nsIContent *aContent, PRInt32 aState) = 0;

  NS_IMETHOD GetFocusedContent(nsIContent **aContent) = 0;
  NS_IMETHOD SetFocusedContent(nsIContent* aContent) = 0;

  // Get the previously-focused content node for this document
  NS_IMETHOD GetLastFocusedContent(nsIContent **aContent) = 0;

  NS_IMETHOD GetFocusedFrame(nsIFrame **aFrame) = 0;

  NS_IMETHOD ContentRemoved(nsIContent* aContent) = 0;
  NS_IMETHOD EventStatusOK(nsGUIEvent* aEvent, PRBool *aOK) = 0;

  // Return whether browse with caret is enabled or not
  virtual PRBool GetBrowseWithCaret() = 0;

  // This is called after find text or when a cursor movement key is pressed
  // If aCanFocusDoc == PR_TRUE, the current document will be focused if caret is not on a focusable element
  NS_IMETHOD MoveFocusToCaret(PRBool aCanFocusDoc, PRBool *aIsSelectionWithFocus) = 0;
  NS_IMETHOD MoveCaretToFocus() = 0;

  // Set focus on any element that can receive focus, or on document via aFocusContent == nsnull
  // Must supply method that focus is being set with
  NS_IMETHOD ChangeFocusWith(nsIContent *aFocusContent, EFocusedWithType aFocusedWith) = 0;

  // Access Key Registration
  NS_IMETHOD RegisterAccessKey(nsIContent* aContent, PRUint32 aKey) = 0;
  NS_IMETHOD UnregisterAccessKey(nsIContent* aContent, PRUint32 aKey) = 0;

  NS_IMETHOD SetCursor(PRInt32 aCursor, imgIContainer* aContainer,
                       PRBool aHaveHotspot, float aHotspotX, float aHotspotY,
                       nsIWidget* aWidget, PRBool aLockCursor) = 0;

  // Method for moving the focus forward/back.
  NS_IMETHOD ShiftFocus(PRBool aDirection, nsIContent* aStart)=0;

  NS_IMETHOD NotifyDestroyPresContext(nsPresContext* aPresContext) = 0;
};

NS_DEFINE_STATIC_IID_ACCESSOR(nsIEventStateManager, NS_IEVENTSTATEMANAGER_IID)

#define NS_EVENT_STATE_ACTIVE        0x00000001 // mouse is down on content
#define NS_EVENT_STATE_FOCUS         0x00000002 // content has focus
#define NS_EVENT_STATE_HOVER         0x00000004 // mouse is hovering over content
#define NS_EVENT_STATE_DRAGOVER      0x00000008 // drag  is hovering over content
#define NS_EVENT_STATE_URLTARGET     0x00000010 // content is URL's target (ref)

// The following states are used only for ContentStatesChanged

#define NS_EVENT_STATE_CHECKED       0x00000020 // CSS3-Selectors
#define NS_EVENT_STATE_ENABLED       0x00000040 // CSS3-Selectors
#define NS_EVENT_STATE_DISABLED      0x00000080 // CSS3-Selectors
#define NS_EVENT_STATE_REQUIRED      0x00000100 // CSS3-UI
#define NS_EVENT_STATE_OPTIONAL      0x00000200 // CSS3-UI
#define NS_EVENT_STATE_VISITED       0x00000400 // CSS2
#define NS_EVENT_STATE_VALID         0x00000800 // CSS3-UI
#define NS_EVENT_STATE_INVALID       0x00001000 // CSS3-UI
#define NS_EVENT_STATE_INRANGE       0x00002000 // CSS3-UI
#define NS_EVENT_STATE_OUTOFRANGE    0x00004000 // CSS3-UI
// these two are temporary (see bug 302188)
#define NS_EVENT_STATE_MOZ_READONLY  0x00008000 // CSS3-UI
#define NS_EVENT_STATE_MOZ_READWRITE 0x00010000 // CSS3-UI

// Content could not be rendered (image/object/etc).
#define NS_EVENT_STATE_BROKEN        0x00020000
// Content disabled by the user (images turned off, say)
#define NS_EVENT_STATE_USERDISABLED  0x00040000
// Content suppressed by the user (ad blocking, etc)
#define NS_EVENT_STATE_SUPPRESSED    0x00080000
// Content is still loading such that there is nothing to show the
// user (eg an image which hasn't started coming in yet)
#define NS_EVENT_STATE_LOADING       0x00100000
// Content is of a type that gecko can't handle
#define NS_EVENT_STATE_TYPE_UNSUPPORTED \
                                     0x00200000

#endif // nsIEventStateManager_h__
