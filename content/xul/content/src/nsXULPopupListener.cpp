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
 *   Original Author: David W. Hyatt (hyatt@netscape.com)
 *   Dean Tessman <dean_tessman@hotmail.com>
 *   Pierre Phaneuf <pp@ludusdesign.com>
 *   Robert O'Callahan <roc+moz@cs.cmu.edu>
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

/*
  This file provides the implementation for xul popup listener which
  tracks xul popups and context menus
 */

#include "nsXULPopupListener.h"
#include "nsCOMPtr.h"
#include "nsGkAtoms.h"
#include "nsIDOMElement.h"
#include "nsIDOMXULElement.h"
#include "nsIDOMNodeList.h"
#include "nsIDOMDocument.h"
#include "nsIDOMDocumentXBL.h"
#include "nsContentCID.h"
#include "nsContentUtils.h"
#include "nsXULPopupManager.h"

#include "nsIScriptContext.h"
#include "nsIDOMWindowInternal.h"
#include "nsIDOMXULDocument.h"
#include "nsIDocument.h"
#include "nsIDOMNSUIEvent.h"
#include "nsIDOMEventTarget.h"
#include "nsIDOMNSEvent.h"
#include "nsServiceManagerUtils.h"
#include "nsIPrincipal.h"
#include "nsIScriptSecurityManager.h"
#include "nsLayoutUtils.h"
#include "nsFrameManager.h"
#include "nsHTMLReflowState.h"

// for event firing in context menus
#include "nsPresContext.h"
#include "nsIPresShell.h"
#include "nsIEventStateManager.h"
#include "nsIFocusController.h"
#include "nsPIDOMWindow.h"
#include "nsIViewManager.h"
#include "nsDOMError.h"
#include "nsIMenuFrame.h"

// on win32 and os/2, context menus come up on mouse up. On other platforms,
// they appear on mouse down. Certain bits of code care about this difference.
#if !defined(XP_WIN) && !defined(XP_OS2)
#define NS_CONTEXT_MENU_IS_MOUSEUP 1
#endif

nsXULPopupListener::nsXULPopupListener(nsIDOMElement *aElement, PRBool aIsContext)
  : mElement(aElement), mPopupContent(nsnull), mIsContext(aIsContext)
{
}

nsXULPopupListener::~nsXULPopupListener(void)
{
  ClosePopup();
}

NS_IMPL_CYCLE_COLLECTION_2(nsXULPopupListener, mElement, mPopupContent)
NS_IMPL_CYCLE_COLLECTING_ADDREF(nsXULPopupListener)
NS_IMPL_CYCLE_COLLECTING_RELEASE(nsXULPopupListener)

NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(nsXULPopupListener)
  NS_INTERFACE_MAP_ENTRY(nsIDOMMouseListener)
  NS_INTERFACE_MAP_ENTRY(nsIDOMContextMenuListener)
  NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(nsIDOMEventListener, nsIDOMMouseListener)
NS_INTERFACE_MAP_END

////////////////////////////////////////////////////////////////
// nsIDOMMouseListener

nsresult
nsXULPopupListener::MouseDown(nsIDOMEvent* aMouseEvent)
{
  if(!mIsContext)
    return PreLaunchPopup(aMouseEvent);
  else
    return NS_OK;
}

nsresult
nsXULPopupListener::ContextMenu(nsIDOMEvent* aMouseEvent)
{
  if(mIsContext)
    return PreLaunchPopup(aMouseEvent);
  else 
    return NS_OK;
}

nsresult
nsXULPopupListener::PreLaunchPopup(nsIDOMEvent* aMouseEvent)
{
  PRUint16 button;

  nsCOMPtr<nsIDOMMouseEvent> mouseEvent;
  mouseEvent = do_QueryInterface(aMouseEvent);
  if (!mouseEvent) {
    //non-ui event passed in.  bad things.
    return NS_OK;
  }

  // check if someone has attempted to prevent this action.
  nsCOMPtr<nsIDOMNSUIEvent> nsUIEvent;
  nsUIEvent = do_QueryInterface(mouseEvent);
  if (!nsUIEvent) {
    return NS_OK;
  }

  // Get the node that was clicked on.
  nsCOMPtr<nsIDOMEventTarget> target;
  mouseEvent->GetTarget(getter_AddRefs(target));
  nsCOMPtr<nsIDOMNode> targetNode = do_QueryInterface(target);

  if (!targetNode && mIsContext) {
    // Not a DOM node, see if it's the DOM window (bug 380818).
    nsCOMPtr<nsIDOMWindow> domWin = do_QueryInterface(target);
    if (!domWin) {
      return NS_ERROR_DOM_WRONG_TYPE_ERR;
    }
    // Try to use the root node as target node.
    nsCOMPtr<nsIDOMDocument> domDoc;
    domWin->GetDocument(getter_AddRefs(domDoc));

    nsCOMPtr<nsIDocument> doc = do_QueryInterface(domDoc);
    if (doc)
      targetNode = do_QueryInterface(doc->GetRootContent());
    if (!targetNode) {
      return NS_ERROR_FAILURE;
    }
  }

  PRBool preventDefault;
  nsUIEvent->GetPreventDefault(&preventDefault);
  if (preventDefault && targetNode && mIsContext) {
    // Someone called preventDefault on a context menu.
    // Let's make sure they are allowed to do so.
    PRBool eventEnabled =
      nsContentUtils::GetBoolPref("dom.event.contextmenu.enabled", PR_TRUE);
    if (!eventEnabled) {
      // The user wants his contextmenus.  Let's make sure that this is a website
      // and not chrome since there could be places in chrome which don't want
      // contextmenus.
      nsCOMPtr<nsINode> node = do_QueryInterface(targetNode);
      if (node) {
        nsCOMPtr<nsIPrincipal> system;
        nsContentUtils::GetSecurityManager()->
          GetSystemPrincipal(getter_AddRefs(system));
        if (node->NodePrincipal() != system) {
          // This isn't chrome.  Cancel the preventDefault() and
          // let the event go forth.
          preventDefault = PR_FALSE;
        }
      }
    }
  }

  if (preventDefault) {
    // someone called preventDefault. bail.
    return NS_OK;
  }

  // prevent popups on menu and menuitems as they handle their own popups
  // This was added for bug 96920.
  // If a menu item child was clicked on that leads to a popup needing
  // to show, we know (guaranteed) that we're dealing with a menu or
  // submenu of an already-showing popup.  We don't need to do anything at all.
  nsCOMPtr<nsIContent> targetContent = do_QueryInterface(target);
  if (!mIsContext) {
    nsIAtom *tag = targetContent ? targetContent->Tag() : nsnull;
    if (tag == nsGkAtoms::menu || tag == nsGkAtoms::menuitem)
      return NS_OK;
  }

  // Get the document with the popup.
  nsCOMPtr<nsIContent> content = do_QueryInterface(mElement);

  // Turn the document into a XUL document so we can use SetPopupNode.
  nsCOMPtr<nsIDOMXULDocument> xulDocument = do_QueryInterface(content->GetDocument());
  if (!xulDocument) {
    return NS_ERROR_FAILURE;
  }

  // Store clicked-on node in xul document for context menus and menu popups.
  xulDocument->SetPopupNode(targetNode);

  nsCOMPtr<nsIDOMNSEvent> nsevent(do_QueryInterface(aMouseEvent));

  if (mIsContext) {
#ifndef NS_CONTEXT_MENU_IS_MOUSEUP
    // If the context menu launches on mousedown,
    // we have to fire focus on the content we clicked on
    FireFocusOnTargetContent(targetNode);
#endif
  }
  else {
    // Only open popups when the left mouse button is down.
    mouseEvent->GetButton(&button);
    if (button != 0)
      return NS_OK;
  }

  // Open the popup and cancel the default handling of the event.
  LaunchPopup(aMouseEvent, targetContent);
  aMouseEvent->StopPropagation();
  aMouseEvent->PreventDefault();

  return NS_OK;
}

nsresult
nsXULPopupListener::FireFocusOnTargetContent(nsIDOMNode* aTargetNode)
{
  nsresult rv;
  nsCOMPtr<nsIDOMDocument> domDoc;
  rv = aTargetNode->GetOwnerDocument(getter_AddRefs(domDoc));
  if(NS_SUCCEEDED(rv) && domDoc)
  {
    nsCOMPtr<nsIDocument> doc = do_QueryInterface(domDoc);

    // Get nsIDOMElement for targetNode
    nsIPresShell *shell = doc->GetPrimaryShell();
    if (!shell)
      return NS_ERROR_FAILURE;

    // strong reference to keep this from going away between events
    nsCOMPtr<nsPresContext> context = shell->GetPresContext();
 
    nsCOMPtr<nsIContent> content = do_QueryInterface(aTargetNode);
    nsIFrame* targetFrame = shell->GetPrimaryFrameFor(content);
    if (!targetFrame) return NS_ERROR_FAILURE;
      
    PRBool suppressBlur = PR_FALSE;
    const nsStyleUserInterface* ui = targetFrame->GetStyleUserInterface();
    suppressBlur = (ui->mUserFocus == NS_STYLE_USER_FOCUS_IGNORE);

    nsCOMPtr<nsIDOMElement> element;
    nsCOMPtr<nsIContent> newFocus = do_QueryInterface(content);

    nsIFrame* currFrame = targetFrame;
    // Look for the nearest enclosing focusable frame.
    while (currFrame) {
        PRInt32 tabIndexUnused;
        if (currFrame->IsFocusable(&tabIndexUnused, PR_TRUE)) {
          newFocus = currFrame->GetContent();
          nsCOMPtr<nsIDOMElement> domElement(do_QueryInterface(newFocus));
          if (domElement) {
            element = domElement;
            break;
          }
        }
        currFrame = currFrame->GetParent();
    } 
    nsCOMPtr<nsIContent> focusableContent = do_QueryInterface(element);
    nsIEventStateManager *esm = context->EventStateManager();

    if (focusableContent) {
      // Lock to scroll by SetFocus. See bug 309075.
      nsFocusScrollSuppressor scrollSuppressor;
      nsPIDOMWindow *ourWindow = doc->GetWindow();
      if (ourWindow) {
        scrollSuppressor.Init(ourWindow->GetRootFocusController());
      }

      focusableContent->SetFocus(context);
    } else if (!suppressBlur)
      esm->SetContentState(nsnull, NS_EVENT_STATE_FOCUS);

    esm->SetContentState(focusableContent, NS_EVENT_STATE_ACTIVE);
  }
  return rv;
}

// ClosePopup
//
// Do everything needed to shut down the popup.
//
// NOTE: This routine is safe to call even if the popup is already closed.
//
void
nsXULPopupListener::ClosePopup()
{
  if (mPopupContent) {
    // this is called when the listener is going away, so make sure that the
    // popup is hidden. Use asynchronous hiding just to be safe so we don't
    // fire events during destruction.  
    nsXULPopupManager* pm = nsXULPopupManager::GetInstance();
    if (pm)
      pm->HidePopup(mPopupContent, PR_FALSE, PR_TRUE, PR_TRUE);
    mPopupContent = nsnull;  // release the popup
  }
} // ClosePopup

static void
GetImmediateChild(nsIContent* aContent, nsIAtom *aTag, nsIContent** aResult) 
{
  *aResult = nsnull;
  PRInt32 childCount = aContent->GetChildCount();
  for (PRInt32 i = 0; i < childCount; i++) {
    nsIContent *child = aContent->GetChildAt(i);
    if (child->Tag() == aTag) {
      *aResult = child;
      NS_ADDREF(*aResult);
      return;
    }
  }

  return;
}

//
// LaunchPopup
//
// Given the element on which the event was triggered and the mouse locations in
// Client and widget coordinates, popup a new window showing the appropriate 
// content.
//
// aTargetContent is the target of the mouse event aEvent that triggered the
// popup. mElement is the element that the popup menu is attached to.
// aTargetContent may be equal to mElement or it may be a descendant.
//
// This looks for an attribute on |mElement| of the appropriate popup type 
// (popup, context) and uses that attribute's value as an ID for
// the popup content in the document.
//
nsresult
nsXULPopupListener::LaunchPopup(nsIDOMEvent* aEvent, nsIContent* aTargetContent)
{
  nsresult rv = NS_OK;

  nsAutoString type(NS_LITERAL_STRING("popup"));
  if (mIsContext)
    type.AssignLiteral("context");

  nsAutoString identifier;
  mElement->GetAttribute(type, identifier);

  if (identifier.IsEmpty()) {
    if (type.EqualsLiteral("popup"))
      mElement->GetAttribute(NS_LITERAL_STRING("menu"), identifier);
    else if (type.EqualsLiteral("context"))
      mElement->GetAttribute(NS_LITERAL_STRING("contextmenu"), identifier);
    if (identifier.IsEmpty())
      return rv;
  }

  // Try to find the popup content and the document.
  nsCOMPtr<nsIContent> content = do_QueryInterface(mElement);
  nsCOMPtr<nsIDocument> document = content->GetDocument();

  // Turn the document into a DOM document so we can use getElementById
  nsCOMPtr<nsIDOMDocument> domDocument = do_QueryInterface(document);
  if (!domDocument) {
    NS_ERROR("Popup attached to an element that isn't in XUL!");
    return NS_ERROR_FAILURE;
  }

  // Handle the _child case for popups and context menus
  nsCOMPtr<nsIDOMElement> popupElement;

  if (identifier.EqualsLiteral("_child")) {
    nsCOMPtr<nsIContent> popup;

    GetImmediateChild(content, nsGkAtoms::menupopup, getter_AddRefs(popup));
    if (popup)
      popupElement = do_QueryInterface(popup);
    else {
      nsCOMPtr<nsIDOMDocumentXBL> nsDoc(do_QueryInterface(domDocument));
      nsCOMPtr<nsIDOMNodeList> list;
      nsDoc->GetAnonymousNodes(mElement, getter_AddRefs(list));
      if (list) {
        PRUint32 ctr,listLength;
        nsCOMPtr<nsIDOMNode> node;
        list->GetLength(&listLength);
        for (ctr = 0; ctr < listLength; ctr++) {
          list->Item(ctr, getter_AddRefs(node));
          nsCOMPtr<nsIContent> childContent(do_QueryInterface(node));

          if (childContent->NodeInfo()->Equals(nsGkAtoms::menupopup,
                                               kNameSpaceID_XUL)) {
            popupElement = do_QueryInterface(childContent);
            break;
          }
        }
      }
    }
  }
  else if (NS_FAILED(rv = domDocument->GetElementById(identifier,
                                              getter_AddRefs(popupElement)))) {
    // Use getElementById to obtain the popup content and gracefully fail if 
    // we didn't find any popup content in the document. 
    NS_ERROR("GetElementById had some kind of spasm.");
    return rv;
  }

  // return if no popup was found or the popup is the element itself.
  if ( !popupElement || popupElement == mElement)
    return NS_OK;

  // Submenus can't be used as context menus or popups, bug 288763.
  // Similar code also in nsXULTooltipListener::GetTooltipFor.
  nsCOMPtr<nsIContent> popup = do_QueryInterface(popupElement);
  nsIContent* parent = popup->GetParent();
  if (parent) {
    nsIDocument* doc = parent->GetCurrentDoc();
    nsIPresShell* presShell = doc ? doc->GetPrimaryShell() : nsnull;
    nsIFrame* frame = presShell ? presShell->GetPrimaryFrameFor(parent) : nsnull;
    if (frame && frame->GetType() == nsGkAtoms::menuFrame)
      return NS_OK;
  }

  nsXULPopupManager* pm = nsXULPopupManager::GetInstance();
  if (!pm)
    return NS_OK;

  // For left-clicks, if the popup has an position attribute, or both the
  // popupanchor and popupalign attributes are used, anchor the popup to the
  // element, otherwise just open it at the screen position where the mouse
  // was clicked. Context menus always open at the mouse position.
  mPopupContent = popup;
  if (!mIsContext &&
      (mPopupContent->HasAttr(kNameSpaceID_None, nsGkAtoms::position) ||
       (mPopupContent->HasAttr(kNameSpaceID_None, nsGkAtoms::popupanchor) &&
        mPopupContent->HasAttr(kNameSpaceID_None, nsGkAtoms::popupalign)))) {
    pm->ShowPopup(mPopupContent, content, EmptyString(), 0, 0,
                  PR_FALSE, PR_TRUE, PR_FALSE, aEvent);
  }
  else {
    PRInt32 xPos = 0, yPos = 0;
    nsCOMPtr<nsIDOMMouseEvent> mouseEvent = do_QueryInterface(aEvent);
    mouseEvent->GetScreenX(&xPos);
    mouseEvent->GetScreenY(&yPos);

    if (mIsContext) {
      // position the menu two pixels down and to the right from the current
      // mouse position. This makes it easier to dismiss the menu by just clicking
      xPos += 2;
      yPos += 2;
    }

    pm->ShowPopupAtScreen(mPopupContent, xPos, yPos, mIsContext, aEvent);
  }

  return NS_OK;
}

////////////////////////////////////////////////////////////////
nsresult
NS_NewXULPopupListener(nsIDOMElement* aElement, PRBool aIsContext,
                       nsIDOMEventListener** aListener)
{
    nsXULPopupListener* pl = new nsXULPopupListener(aElement, aIsContext);
    if (!pl)
      return NS_ERROR_OUT_OF_MEMORY;

    *aListener = static_cast<nsIDOMMouseListener *>(pl);
    NS_ADDREF(*aListener);
    return NS_OK;
}
