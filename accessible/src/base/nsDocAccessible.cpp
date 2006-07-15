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
 * Portions created by the Initial Developer are Copyright (C) 2003
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Original Author: Aaron Leventhal (aaronl@netscape.com)
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

#include "nsDocAccessible.h"
#include "nsAccessibilityAtoms.h"
#include "nsAccessibleEventData.h"
#include "nsIAccessibilityService.h"
#include "nsIMutableArray.h"
#include "nsICommandManager.h"
#include "nsIDocShell.h"
#include "nsIDocShellTreeItem.h"
#include "nsIDocument.h"
#include "nsIDOMAttr.h"
#include "nsIDOMCharacterData.h"
#include "nsIDOMDocument.h"
#include "nsIDOMDocumentType.h"
#include "nsIDOMNSDocument.h"
#include "nsIDOMNSHTMLDocument.h"
#include "nsIDOMMutationEvent.h"
#include "nsPIDOMWindow.h"
#include "nsIDOMXULPopupElement.h"
#include "nsIEditingSession.h"
#include "nsIEventStateManager.h"
#include "nsIFrame.h"
#include "nsHTMLSelectAccessible.h"
#include "nsIInterfaceRequestorUtils.h"
#include "nsINameSpaceManager.h"
#include "nsIObserverService.h"
#include "nsIPresShell.h"
#include "nsIServiceManager.h"
#include "nsIScrollableView.h"
#include "nsUnicharUtils.h"
#include "nsIURI.h"
#include "nsIWebNavigation.h"
#ifdef MOZ_XUL
#include "nsIXULDocument.h"
#endif

//=============================//
// nsDocAccessible  //
//=============================//

//-----------------------------------------------------
// construction
//-----------------------------------------------------
nsDocAccessible::nsDocAccessible(nsIDOMNode *aDOMNode, nsIWeakReference* aShell):
  nsHyperTextAccessible(aDOMNode, aShell), mWnd(nsnull),
  mScrollPositionChangedTicks(0), mIsContentLoaded(PR_FALSE)
{
  // Because of the way document loading happens, the new nsIWidget is created before
  // the old one is removed. Since it creates the nsDocAccessible, for a brief moment
  // there can be 2 nsDocAccessible's for the content area, although for 2 different
  // pres shells.

  nsCOMPtr<nsIPresShell> shell(do_QueryReferent(mWeakShell));
  if (shell) {
    mDocument = shell->GetDocument();
    nsIViewManager* vm = shell->GetViewManager();
    if (vm) {
      nsCOMPtr<nsIWidget> widget;
      vm->GetWidget(getter_AddRefs(widget));
      if (widget) {
        mWnd = widget->GetNativeData(NS_NATIVE_WINDOW);
      }
    }
  }

  // XXX aaronl should we use an algorithm for the initial cache size?
  mAccessNodeCache.Init(kDefaultCacheSize);

  nsCOMPtr<nsIDocShellTreeItem> docShellTreeItem =
    GetDocShellTreeItemFor(mDOMNode);
  if (docShellTreeItem) {
    PRInt32 itemType;
    docShellTreeItem->GetItemType(&itemType);
    if (itemType == nsIDocShellTreeItem::typeChrome) {
      mIsContentLoaded = PR_TRUE;
    }
  }
}

//-----------------------------------------------------
// destruction
//-----------------------------------------------------
nsDocAccessible::~nsDocAccessible()
{
}

NS_INTERFACE_MAP_BEGIN(nsDocAccessible)
  NS_INTERFACE_MAP_ENTRY(nsIAccessibleDocument)
  NS_INTERFACE_MAP_ENTRY(nsPIAccessibleDocument)
  NS_INTERFACE_MAP_ENTRY(nsIDocumentObserver)
  NS_INTERFACE_MAP_ENTRY(nsIMutationObserver)
  NS_INTERFACE_MAP_ENTRY(nsIScrollPositionListener)
  NS_INTERFACE_MAP_ENTRY(nsISupportsWeakReference)
  NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(nsISupports, nsIAccessibleDocument)
  NS_INTERFACE_MAP_ENTRY(nsIObserver)
NS_INTERFACE_MAP_END_INHERITING(nsHyperTextAccessible)

NS_IMPL_ADDREF_INHERITED(nsDocAccessible, nsHyperTextAccessible)
NS_IMPL_RELEASE_INHERITED(nsDocAccessible, nsHyperTextAccessible)

NS_IMETHODIMP nsDocAccessible::GetName(nsAString& aName)
{
  nsresult rv = NS_OK;
  aName.Truncate();
  if (mRoleMapEntry) {
    rv = nsAccessible::GetName(aName);
  }
  if (aName.IsEmpty()) {
    rv = GetTitle(aName);
  }
  if (aName.IsEmpty() && mParent) {
    rv = mParent->GetName(aName);
  }

  return rv;
}

NS_IMETHODIMP nsDocAccessible::GetRole(PRUint32 *aRole)
{
  *aRole = ROLE_PANE; // Fall back

  nsCOMPtr<nsIDocShellTreeItem> docShellTreeItem =
    GetDocShellTreeItemFor(mDOMNode);
  if (docShellTreeItem) {
    nsCOMPtr<nsIDocShellTreeItem> sameTypeRoot;
    docShellTreeItem->GetSameTypeRootTreeItem(getter_AddRefs(sameTypeRoot));
    if (sameTypeRoot == docShellTreeItem) {
      // Root of content or chrome tree
      PRInt32 itemType;
      docShellTreeItem->GetItemType(&itemType);
      if (itemType == nsIDocShellTreeItem::typeChrome) {
        *aRole = ROLE_APPLICATION;
      }
      else if (itemType == nsIDocShellTreeItem::typeContent) {
#ifdef MOZ_XUL
        nsCOMPtr<nsIXULDocument> xulDoc(do_QueryInterface(mDocument));
        *aRole = xulDoc ? ROLE_APPLICATION : ROLE_DOCUMENT;
#else
        *aRole = ROLE_DOCUMENT;
#endif
      }
    }
  }

  return NS_OK;
}

NS_IMETHODIMP nsDocAccessible::GetValue(nsAString& aValue)
{
  return GetURL(aValue);
}

NS_IMETHODIMP nsDocAccessible::GetState(PRUint32 *aState)
{
  if (!mDOMNode) {
    return NS_ERROR_FAILURE;
  }
  nsAccessible::GetState(aState);
  *aState |= STATE_FOCUSABLE;

  if (!mIsContentLoaded) {
    *aState |= STATE_BUSY;
  }

  // Is it visible?
  nsCOMPtr<nsIPresShell> shell(do_QueryReferent(mWeakShell));
  nsCOMPtr<nsIWidget> widget;
  if (shell) {
    nsIViewManager* vm = shell->GetViewManager();
    if (vm) {
      vm->GetWidget(getter_AddRefs(widget));
    }
  }
  PRBool isVisible = (widget != nsnull);
  while (widget && isVisible) {
    widget->IsVisible(isVisible);
    widget = widget->GetParent();
  }
  if (!isVisible) {
    *aState |= STATE_INVISIBLE;
  }

  nsCOMPtr<nsIEditor> editor = GetEditor();
  if (!editor) {
    *aState |= STATE_READONLY;
  }

  return NS_OK;
}

NS_IMETHODIMP nsDocAccessible::GetFocusedChild(nsIAccessible **aFocusedChild)
{
  if (!gLastFocusedNode) {
    *aFocusedChild = nsnull;
    return NS_OK;
  }

  // Return an accessible for the current global focus, which does not have to
  // be contained within the current document.
  nsCOMPtr<nsIAccessibilityService> accService =
    do_GetService("@mozilla.org/accessibilityService;1");
  return accService->GetAccessibleFor(gLastFocusedNode, aFocusedChild);
}

// ------- nsIAccessibleDocument Methods (5) ---------------

NS_IMETHODIMP nsDocAccessible::GetURL(nsAString& aURL)
{
  if (!mDocument) {
    return NS_ERROR_FAILURE; // Document has been shut down
  }
  nsCOMPtr<nsISupports> container = mDocument->GetContainer();
  nsCOMPtr<nsIWebNavigation> webNav(do_GetInterface(container));
  nsCAutoString theURL;
  if (webNav) {
    nsCOMPtr<nsIURI> pURI;
    webNav->GetCurrentURI(getter_AddRefs(pURI));
    if (pURI)
      pURI->GetSpec(theURL);
  }
  CopyUTF8toUTF16(theURL, aURL);
  return NS_OK;
}

NS_IMETHODIMP nsDocAccessible::GetTitle(nsAString& aTitle)
{
  if (mDocument) {
    aTitle = mDocument->GetDocumentTitle();
    return NS_OK;
  }

  return NS_ERROR_FAILURE;
}

NS_IMETHODIMP nsDocAccessible::GetMimeType(nsAString& aMimeType)
{
  nsCOMPtr<nsIDOMNSDocument> domnsDocument(do_QueryInterface(mDocument));
  if (domnsDocument) {
    return domnsDocument->GetContentType(aMimeType);
  }
  return NS_ERROR_FAILURE;
}

NS_IMETHODIMP nsDocAccessible::GetDocType(nsAString& aDocType)
{
  nsCOMPtr<nsIDOMDocument> domDoc(do_QueryInterface(mDocument));
  nsCOMPtr<nsIDOMDocumentType> docType;

#ifdef MOZ_XUL
  nsCOMPtr<nsIXULDocument> xulDoc(do_QueryInterface(mDocument));
  if (xulDoc) {
    aDocType.AssignLiteral("window"); // doctype not implemented for XUL at time of writing - causes assertion
    return NS_OK;
  } else
#endif
  if (domDoc && NS_SUCCEEDED(domDoc->GetDoctype(getter_AddRefs(docType))) && docType) {
    return docType->GetPublicId(aDocType);
  }

  return NS_ERROR_FAILURE;
}

NS_IMETHODIMP nsDocAccessible::GetNameSpaceURIForID(PRInt16 aNameSpaceID, nsAString& aNameSpaceURI)
{
  if (mDocument) {
    nsCOMPtr<nsINameSpaceManager> nameSpaceManager =
        do_GetService(NS_NAMESPACEMANAGER_CONTRACTID);
    if (nameSpaceManager)
      return nameSpaceManager->GetNameSpaceURI(aNameSpaceID, aNameSpaceURI);
  }
  return NS_ERROR_FAILURE;
}

NS_IMETHODIMP nsDocAccessible::GetCaretAccessible(nsIAccessible **aCaretAccessible)
{
  // We only have a caret accessible on the root document
  *aCaretAccessible = nsnull;
  return NS_OK;
}

NS_IMETHODIMP nsDocAccessible::GetWindowHandle(void **aWindow)
{
  *aWindow = mWnd;
  return NS_OK;
}

NS_IMETHODIMP nsDocAccessible::GetWindow(nsIDOMWindow **aDOMWin)
{
  *aDOMWin = nsnull;
  if (!mDocument) {
    return NS_ERROR_FAILURE;  // Accessible is Shutdown()
  }
  *aDOMWin = mDocument->GetWindow();

  if (!*aDOMWin)
    return NS_ERROR_FAILURE;  // No DOM Window

  NS_ADDREF(*aDOMWin);

  return NS_OK;
}

NS_IMETHODIMP nsDocAccessible::GetDocument(nsIDOMDocument **aDOMDoc)
{
  nsCOMPtr<nsIDOMDocument> domDoc(do_QueryInterface(mDocument));
  *aDOMDoc = domDoc;

  if (domDoc) {
    NS_ADDREF(*aDOMDoc);
    return NS_OK;
  }

  return NS_ERROR_FAILURE;
}

void nsDocAccessible::SetEditor(nsIEditor* aEditor)
{
  mEditor = aEditor;
  if (mEditor)
    mEditor->AddEditActionListener(this);
}

void nsDocAccessible::CheckForEditor()
{
  if (mEditor) {
    return;  // Already have editor, don't need to check
  }
  if (!mDocument) {
    return;  // No document -- we've been shut down
  }

  nsCOMPtr<nsISupports> container = mDocument->GetContainer();
  nsCOMPtr<nsIEditingSession> editingSession(do_GetInterface(container));
  if (!editingSession)
    return; // No editing session interface

  editingSession->GetEditorForWindow(mDocument->GetWindow(),
                                     getter_AddRefs(mEditor));
  if (mEditor) {
    // State readonly is now clear
#ifdef MOZ_ACCESSIBILITY_ATK
    AtkStateChange stateData;
    stateData.enable = PR_TRUE;
    stateData.state = STATE_READONLY; // Will be translated to ATK_STATE_EDITABLE
    FireToolkitEvent(nsIAccessibleEvent::EVENT_STATE_CHANGE, this, &stateData);
#else
    FireToolkitEvent(nsIAccessibleEvent::EVENT_STATE_CHANGE, this, nsnull);
#endif
  }
}

NS_IMETHODIMP nsDocAccessible::GetCachedAccessNode(void *aUniqueID, nsIAccessNode **aAccessNode)
{
  GetCacheEntry(mAccessNodeCache, aUniqueID, aAccessNode); // Addrefs for us
  return NS_OK;
}

NS_IMETHODIMP nsDocAccessible::CacheAccessNode(void *aUniqueID, nsIAccessNode *aAccessNode)
{
  PutCacheEntry(mAccessNodeCache, aUniqueID, aAccessNode);
  return NS_OK;
}

NS_IMETHODIMP nsDocAccessible::GetParent(nsIAccessible **aParent)
{
  // Hook up our new accessible with our parent
  if (!mParent) {
    nsIDocument *parentDoc = mDocument->GetParentDocument();
    if (parentDoc) {
      nsIContent *ownerContent = parentDoc->FindContentForSubDocument(mDocument);
      nsCOMPtr<nsIDOMNode> ownerNode(do_QueryInterface(ownerContent));
      if (ownerNode) {
        nsCOMPtr<nsIAccessibilityService> accService =
          do_GetService("@mozilla.org/accessibilityService;1");
        if (accService) {
          // XXX aaronl: ideally we would traverse the presshell chain
          // Since there's no easy way to do that, we cheat and use
          // the document hierarchy. GetAccessibleFor() is bad because
          // it doesn't support our concept of multiple presshells per doc.
          // It should be changed to use GetAccessibleInWeakShell()
          accService->GetAccessibleFor(ownerNode, getter_AddRefs(mParent));
        }
      }
    }
  }
  return mParent ? nsAccessible::GetParent(aParent) : NS_ERROR_FAILURE;
}

NS_IMETHODIMP nsDocAccessible::Init()
{
  PutCacheEntry(gGlobalDocAccessibleCache, mWeakShell, this);

  AddEventListeners();

  nsresult rv = nsHyperTextAccessible::Init();

  if (mRoleMapEntry && mRoleMapEntry->role != ROLE_DIALOG &&
      mRoleMapEntry->role != ROLE_APPLICATION &&
      mRoleMapEntry->role != ROLE_ALERT &&
      mRoleMapEntry->role != ROLE_DOCUMENT) {
    // Document accessible can only have certain roles
    // This was set in nsAccessible::Init() based on dynamic role attribute
    mRoleMapEntry = nsnull; // role attribute is not valid for a document
  }

  return rv;
}


NS_IMETHODIMP nsDocAccessible::Destroy()
{
  gGlobalDocAccessibleCache.Remove(NS_STATIC_CAST(void*, mWeakShell));
  return Shutdown();
}

NS_IMETHODIMP nsDocAccessible::Shutdown()
{
  if (!mWeakShell) {
    return NS_OK;  // Already shutdown
  }

  if (mEditor) {
    mEditor->RemoveEditActionListener(this);
    mEditor = nsnull;
  }

  RemoveEventListeners();

  mWeakShell = nsnull;  // Avoid reentrancy

  if (mFireEventTimer) {
    mFireEventTimer->Cancel();
    mFireEventTimer = nsnull;
  }
  mEventsToFire.Clear();

  ClearCache(mAccessNodeCache);

  mDocument = nsnull;

  return nsHyperTextAccessible::Shutdown();
}

nsIFrame* nsDocAccessible::GetFrame()
{
  nsCOMPtr<nsIPresShell> shell(do_QueryReferent(mWeakShell));

  nsIFrame* root = nsnull;
  if (shell)
    root = shell->GetRootFrame();

  return root;
}

void nsDocAccessible::GetBoundsRect(nsRect& aBounds, nsIFrame** aRelativeFrame)
{
  *aRelativeFrame = GetFrame();

  nsIDocument *document = mDocument;
  nsIDocument *parentDoc = nsnull;

  while (document) {
    nsIPresShell *presShell = document->GetShellAt(0);
    if (!presShell) {
      return;
    }
    nsIViewManager* vm = presShell->GetViewManager();
    if (!vm) {
      return;
    }

    nsIScrollableView* scrollableView = nsnull;
    vm->GetRootScrollableView(&scrollableView);

    nsRect viewBounds(0, 0, 0, 0);
    if (scrollableView) {
      viewBounds = scrollableView->View()->GetBounds();
    }
    else {
      nsIView *view;
      vm->GetRootView(view);
      if (view) {
        viewBounds = view->GetBounds();
      }
    }

    if (parentDoc) {  // After first time thru loop
      aBounds.IntersectRect(viewBounds, aBounds);
    }
    else {  // First time through loop
      aBounds = viewBounds;
    }

    document = parentDoc = document->GetParentDocument();
  }
}


nsresult nsDocAccessible::AddEventListeners()
{
  // 1) Set up scroll position listener
  // 2) Check for editor and listen for changes to editor

  nsCOMPtr<nsIPresShell> presShell(GetPresShell());
  NS_ENSURE_TRUE(presShell, NS_ERROR_FAILURE);

  nsCOMPtr<nsISupports> container = mDocument->GetContainer();
  nsCOMPtr<nsIDocShellTreeItem> docShellTreeItem(do_QueryInterface(container));
  NS_ENSURE_TRUE(docShellTreeItem, NS_ERROR_FAILURE);

  // Make sure we're a content docshell
  // We don't want to listen to chrome progress
  PRInt32 itemType;
  docShellTreeItem->GetItemType(&itemType);

  PRBool isContent = (itemType == nsIDocShellTreeItem::typeContent);

  if (isContent) {
    CheckForEditor();

    if (!mEditor) {
      // We're not an editor yet, but we might become one
      nsCOMPtr<nsICommandManager> commandManager = do_GetInterface(docShellTreeItem);
      if (commandManager) {
        commandManager->AddCommandObserver(this, "obs_documentCreated");
      }
    }
  }

  // add document observer
  mDocument->AddObserver(this);
  return NS_OK;
}

nsresult nsDocAccessible::RemoveEventListeners()
{
  // Remove listeners associated with content documents
  // Remove scroll position listener
  RemoveScrollListener();

  // Remove document observer
  mDocument->RemoveObserver(this);

  if (mScrollWatchTimer) {
    mScrollWatchTimer->Cancel();
    mScrollWatchTimer = nsnull;
  }

  nsCOMPtr<nsISupports> container = mDocument->GetContainer();
  nsCOMPtr<nsIDocShellTreeItem> docShellTreeItem(do_QueryInterface(container));
  NS_ENSURE_TRUE(docShellTreeItem, NS_ERROR_FAILURE);

  PRInt32 itemType;
  docShellTreeItem->GetItemType(&itemType);
  if (itemType == nsIDocShellTreeItem::typeContent) {
    nsCOMPtr<nsICommandManager> commandManager = do_GetInterface(docShellTreeItem);
    if (commandManager) {
      commandManager->RemoveCommandObserver(this, "obs_documentCreated");
    }
  }

  return NS_OK;
}

NS_IMETHODIMP nsDocAccessible::FireAnchorJumpEvent()
{
  return NS_OK;
}

NS_IMETHODIMP nsDocAccessible::FireDocLoadingEvent(PRBool aIsFinished)
{
  if (!mDocument || !mWeakShell) {
    return NS_OK;  // Document has been shut down
  }

  if (mIsContentLoaded == aIsFinished) {
    return NS_OK;
  }
  mIsContentLoaded = aIsFinished;

  if (aIsFinished) {
    // Need to wait until scrollable view is available
    AddScrollListener();
    nsCOMPtr<nsIAccessible> parent;
    GetParent(getter_AddRefs(parent));
    nsCOMPtr<nsPIAccessible> privateAccessible(do_QueryInterface(parent));
    if (privateAccessible) {
      // Make the parent forget about the old document as a child
      privateAccessible->InvalidateChildren();
    }
  }

  return NS_OK;
}

void nsDocAccessible::ScrollTimerCallback(nsITimer *aTimer, void *aClosure)
{
  nsDocAccessible *docAcc = NS_REINTERPRET_CAST(nsDocAccessible*, aClosure);

  if (docAcc && docAcc->mScrollPositionChangedTicks &&
      ++docAcc->mScrollPositionChangedTicks > 2) {
    // Whenever scroll position changes, mScrollPositionChangeTicks gets reset to 1
    // We only want to fire accessibilty scroll event when scrolling stops or pauses
    // Therefore, we wait for no scroll events to occur between 2 ticks of this timer
    // That indicates a pause in scrolling, so we fire the accessibilty scroll event
    docAcc->FireToolkitEvent(nsIAccessibleEvent::EVENT_SCROLLINGEND, docAcc, nsnull);
    docAcc->mScrollPositionChangedTicks = 0;
    if (docAcc->mScrollWatchTimer) {
      docAcc->mScrollWatchTimer->Cancel();
      docAcc->mScrollWatchTimer = nsnull;
    }
  }
}

void nsDocAccessible::AddScrollListener()
{
  nsCOMPtr<nsIPresShell> presShell(do_QueryReferent(mWeakShell));

  nsIViewManager* vm = nsnull;
  if (presShell)
    vm = presShell->GetViewManager();

  nsIScrollableView* scrollableView = nsnull;
  if (vm)
    vm->GetRootScrollableView(&scrollableView);

  if (scrollableView)
    scrollableView->AddScrollPositionListener(this);
}

void nsDocAccessible::RemoveScrollListener()
{
  nsCOMPtr<nsIPresShell> presShell(do_QueryReferent(mWeakShell));

  nsIViewManager* vm = nsnull;
  if (presShell)
    vm = presShell->GetViewManager();

  nsIScrollableView* scrollableView = nsnull;
  if (vm)
    vm->GetRootScrollableView(&scrollableView);

  if (scrollableView)
    scrollableView->RemoveScrollPositionListener(this);
}

NS_IMETHODIMP nsDocAccessible::ScrollPositionWillChange(nsIScrollableView *aView, nscoord aX, nscoord aY)
{
  return NS_OK;
}

NS_IMETHODIMP nsDocAccessible::ScrollPositionDidChange(nsIScrollableView *aScrollableView, nscoord aX, nscoord aY)
{
  // Start new timer, if the timer cycles at least 1 full cycle without more scroll position changes,
  // then the ::Notify() method will fire the accessibility event for scroll position changes
  const PRUint32 kScrollPosCheckWait = 50;
  if (mScrollWatchTimer) {
    mScrollWatchTimer->SetDelay(kScrollPosCheckWait);  // Create new timer, to avoid leaks
  }
  else {
    mScrollWatchTimer = do_CreateInstance("@mozilla.org/timer;1");
    if (mScrollWatchTimer) {
      mScrollWatchTimer->InitWithFuncCallback(ScrollTimerCallback, this,
                                              kScrollPosCheckWait,
                                              nsITimer::TYPE_REPEATING_SLACK);
    }
  }
  mScrollPositionChangedTicks = 1;
  return NS_OK;
}

NS_IMETHODIMP nsDocAccessible::Observe(nsISupports *aSubject, const char *aTopic,
                                       const PRUnichar *aData)
{
  if (!nsCRT::strcmp(aTopic,"obs_documentCreated")) {
    CheckForEditor();
    NS_ASSERTION(mEditor, "Should have editor if we see obs_documentCreated");
  }

  return NS_OK;
}

///////////////////////////////////////////////////////////////////////
// nsIDocumentObserver

NS_IMPL_NSIDOCUMENTOBSERVER_CORE_STUB(nsDocAccessible)
NS_IMPL_NSIDOCUMENTOBSERVER_LOAD_STUB(nsDocAccessible)
NS_IMPL_NSIDOCUMENTOBSERVER_STYLE_STUB(nsDocAccessible)

void
nsDocAccessible::AttributeChanged(nsIDocument *aDocument, nsIContent* aContent,
                                  PRInt32 aNameSpaceID, nsIAtom* aAttribute,
                                  PRInt32 aModType)
{
  // XXX todo
  // We still need to handle special HTML cases here
  // For example, if an <img>'s usemap attribute is modified
  // Otherwise it may just be a state change, for example an object changing
  // its visibility

  nsCOMPtr<nsISupports> container = mDocument->GetContainer();
  nsCOMPtr<nsIDocShell> docShell = do_QueryInterface(container);
  if (!docShell) {
    return;
  }
  PRUint32 busyFlags;
  docShell->GetBusyFlags(&busyFlags);
  if (busyFlags) {
    return; // Still loading, ignore setting of initial attributes
  }

  nsCOMPtr<nsIPresShell> shell = GetPresShell();
  if (!shell) {
    return; // Document has been shut down
  }

  nsCOMPtr<nsIDOMNode> targetNode(do_QueryInterface(aContent));
  NS_ASSERTION(targetNode, "No node for attr modified");
  if (!targetNode) {
    return;
  }

  if (aNameSpaceID == kNameSpaceID_XHTML2_Unofficial ||
      aNameSpaceID == kNameSpaceID_XHTML) {
    if (aAttribute == nsAccessibilityAtoms::role) {
      InvalidateCacheSubtree(aContent, nsIAccessibleEvent::EVENT_REORDER);
    }
    return;
  }

  if (aAttribute == nsAccessibilityAtoms::href || aAttribute == nsAccessibilityAtoms::onclick) {
    InvalidateCacheSubtree(aContent, nsIAccessibleEvent::EVENT_REORDER);
    return;
  }

  PRUint32 eventType = 0;
  if (aAttribute == nsAccessibilityAtoms::selected) {
    // DHTML or XUL selection
    nsCOMPtr<nsIAccessible> multiSelect = GetMultiSelectFor(targetNode);
    // Multi selects use selection_add and selection_remove
    // Single select widgets just mirror event_selection for
    // whatever gets event_focus, which is done in
    // nsRootAccessible::FireAccessibleFocusEvent()
    // So right here we make sure only to deal with multi selects
    if (multiSelect) {
      // Need to find the right event to use here, SELECTION_WITHIN would
      // seem right but we had started using it for something else
      nsCOMPtr<nsIAccessNode> multiSelectAccessNode =
        do_QueryInterface(multiSelect);
      nsCOMPtr<nsIDOMNode> multiSelectDOMNode;
      multiSelectAccessNode->GetDOMNode(getter_AddRefs(multiSelectDOMNode));
      NS_ASSERTION(multiSelectDOMNode, "A new accessible without a DOM node!");
      FireDelayedToolkitEvent(nsIAccessibleEvent::EVENT_SELECTION_WITHIN,
                              multiSelectDOMNode, nsnull, PR_TRUE);
      static nsIContent::AttrValuesArray strings[] =
        {&nsAccessibilityAtoms::_empty, &nsAccessibilityAtoms::_false, nsnull};
      if (aContent->FindAttrValueIn(kNameSpaceID_None,
                                    nsAccessibilityAtoms::selected,
                                    strings, eCaseMatters) !=
          nsIContent::ATTR_VALUE_NO_MATCH) {
        eventType = nsIAccessibleEvent::EVENT_SELECTION_REMOVE;
      }
      else {
        eventType = nsIAccessibleEvent::EVENT_SELECTION_ADD;
      }
    }
  }
  else if (aNameSpaceID == kNameSpaceID_WAIProperties) {
    // DHTML accessibility attributes
    if (HasRoleAttribute(aContent)) {
      // We don't care about DHTML state changes unless there is
      // a DHTML role set for the element
      return;
    }
    if (aAttribute == nsAccessibilityAtoms::checked ||
        aAttribute == nsAccessibilityAtoms::expanded) {
      eventType = nsIAccessibleEvent::EVENT_STATE_CHANGE;
    }
    else if (aAttribute == nsAccessibilityAtoms::readonly ||
             aAttribute == nsAccessibilityAtoms::disabled ||
             aAttribute == nsAccessibilityAtoms::required ||
             aAttribute == nsAccessibilityAtoms::invalid) {
      eventType = nsIAccessibleEvent::EVENT_STATE_CHANGE;
    }
    else if (aAttribute == nsAccessibilityAtoms::valuenow) {
      eventType = nsIAccessibleEvent::EVENT_VALUE_CHANGE;
    }
    else if (aAttribute == nsAccessibilityAtoms::multiselect) {
      // This affects whether the accessible supports nsIAccessibleSelectable.
      // COM says we cannot change what interfaces are supported on-the-fly,
      // so invalidate this object. A new one will be created on demand.
      if (HasRoleAttribute(aContent)) {
        // The multiselect and other waistate attributes only take affect
        // when dynamic content role is present
        InvalidateCacheSubtree(aContent, nsIAccessibleEvent::EVENT_REORDER);
      }
    }
  }

  // Fire after short timer, because we need to wait for
  // DOM attribute & resulting layout to actually change.
  // Otherwise, assistive technology
  // will retrieve the wrong state/value/selection info.
  if (eventType) {
    // The attribute change occured on an accessible node
    FireDelayedToolkitEvent(eventType, targetNode, nsnull);
  }
}

void nsDocAccessible::ContentAppended(nsIDocument *aDocument,
                                      nsIContent* aContainer,
                                      PRInt32 aNewIndexInContainer)
{
  // InvalidateCacheSubtree will not fire the EVENT_SHOW for the new node
  // unless an accessible can be created for the passed in node, which it
  // can't do unless the node is visible. The right thing happens there so
  // no need for an extra visibility check here.
  PRUint32 childCount = aContainer->GetChildCount();
  for (PRUint32 index = aNewIndexInContainer; index < childCount; index ++) {
    InvalidateCacheSubtree(aContainer->GetChildAt(index),
                           nsIAccessibleEvent::EVENT_SHOW);
  }
}

void nsDocAccessible::ContentStatesChanged(nsIDocument* aDocument,
                                           nsIContent* aContent1,
                                           nsIContent* aContent2,
                                           PRInt32 aStateMask)
{
  if (0 == (aStateMask & NS_EVENT_STATE_CHECKED)) {
    return;
  }

  nsHTMLSelectOptionAccessible::SelectionChangedIfOption(aContent1);
  nsHTMLSelectOptionAccessible::SelectionChangedIfOption(aContent2);
}

void nsDocAccessible::CharacterDataChanged(nsIDocument *aDocument,
                                           nsIContent* aContent,
                                           PRBool aAppend)
{
  InvalidateCacheSubtree(aContent, nsIAccessibleEvent::EVENT_REORDER);
}

void
nsDocAccessible::ContentInserted(nsIDocument *aDocument, nsIContent* aContainer,
                                 nsIContent* aChild, PRInt32 aIndexInContainer)
{
  // InvalidateCacheSubtree will not fire the EVENT_SHOW for the new node
  // unless an accessible can be created for the passed in node, which it
  // can't do unless the node is visible. The right thing happens there so
  // no need for an extra visibility check here.
  InvalidateCacheSubtree(aChild, nsIAccessibleEvent::EVENT_SHOW);
}

void
nsDocAccessible::ContentRemoved(nsIDocument *aDocument, nsIContent* aContainer,
                                nsIContent* aChild, PRInt32 aIndexInContainer)
{
  InvalidateCacheSubtree(aChild, nsIAccessibleEvent::EVENT_HIDE);
}

nsresult nsDocAccessible::FireDelayedToolkitEvent(PRUint32 aEvent,
                                                  nsIDOMNode *aDOMNode,
                                                  void *aData,
                                                  PRBool aAllowDupes)
{
  PRBool isTimerStarted = PR_TRUE;
  PRInt32 numQueuedEvents = mEventsToFire.Count();
  if (!mFireEventTimer) {
    // Do not yet have a timer going for firing another event.
    mFireEventTimer = do_CreateInstance("@mozilla.org/timer;1");
    NS_ENSURE_TRUE(mFireEventTimer, NS_ERROR_OUT_OF_MEMORY);
  }
  if (numQueuedEvents == 0) {
    isTimerStarted = PR_FALSE;
  }
  else if (!aAllowDupes) {
    // Check for repeat events. If a redundant event exists remove
    // original and put the new event at the end of the queue
    // so it is fired after the others
    for (PRInt32 index = 0; index < numQueuedEvents; index ++) {
      nsIAccessibleEvent *accessibleEvent = mEventsToFire[index];
      NS_ASSERTION(accessibleEvent, "Array item is not an accessible event");
      if (!accessibleEvent) {
        continue;
      }
      PRUint32 eventType;
      accessibleEvent->GetEventType(&eventType);
      if (eventType == aEvent) {
        nsCOMPtr<nsIDOMNode> domNode;
        accessibleEvent->GetDOMNode(getter_AddRefs(domNode));
        if (domNode == aDOMNode) {
          mEventsToFire.RemoveObjectAt(index);
          -- index;
          -- numQueuedEvents;
        }
      }
    }
  }

  // XXX Add related data for ATK support.
  // For example, state change event should provide what state has changed,
  // as well as the old and new value.
  nsCOMPtr<nsIAccessibleEvent> event =
    new nsAccessibleEventData(aEvent, aDOMNode, this, aData);
  NS_ENSURE_TRUE(event, NS_ERROR_OUT_OF_MEMORY);
  mEventsToFire.AppendObject(event);
  if (!isTimerStarted) {
    // This is be the first delayed event in queue, start timer
    // so that event gets fired via FlushEventsCallback
    mFireEventTimer->InitWithFuncCallback(FlushEventsCallback,
                                          NS_STATIC_CAST(nsPIAccessibleDocument*, this),
                                          0, nsITimer::TYPE_ONE_SHOT);
  }
  return NS_OK;
}

NS_IMETHODIMP nsDocAccessible::FlushPendingEvents()
{
  PRUint32 length = mEventsToFire.Count();
  NS_ASSERTION(length, "How did we get here without events to fire?");
  PRUint32 index;
  for (index = 0; index < length; index ++) {
    nsIAccessibleEvent *accessibleEvent = mEventsToFire[index];
    NS_ASSERTION(accessibleEvent, "Array item is not an accessible event");
    nsCOMPtr<nsIAccessible> accessible;
    accessibleEvent->GetAccessible(getter_AddRefs(accessible));
    if (accessible) {
      PRUint32 eventType;
      accessibleEvent->GetEventType(&eventType);
      if (eventType == nsIAccessibleEvent::EVENT_INTERNAL_LOAD) {
        nsCOMPtr<nsPIAccessibleDocument> docAccessible =
          do_QueryInterface(accessible);
        NS_ASSERTION(docAccessible, "No doc accessible for doc load event");
        if (docAccessible) {
          docAccessible->FireDocLoadingEvent(PR_TRUE);
        }
      }
      else {
        FireToolkitEvent(eventType, accessible, nsnull);
      }
    }
  }
  mEventsToFire.Clear(); // Clear out array
  return NS_OK;
}

void nsDocAccessible::FlushEventsCallback(nsITimer *aTimer, void *aClosure)
{
  nsPIAccessibleDocument *accessibleDoc = NS_STATIC_CAST(nsPIAccessibleDocument*, aClosure);
  NS_ASSERTION(accessibleDoc, "How did we get here without an accessible document?");
  accessibleDoc->FlushPendingEvents();
}

void nsDocAccessible::RefreshNodes(nsIDOMNode *aStartNode, PRUint32 aChangeEvent)
{
  nsCOMPtr<nsIDOMNode> iterNode(aStartNode), nextNode;
  nsCOMPtr<nsIAccessNode> accessNode;

  do {
    GetCachedAccessNode(iterNode, getter_AddRefs(accessNode));
    if (accessNode) {
      // Accessibles that implement their own subtrees,
      // like html combo boxes and xul trees must shutdown all of their own
      // children when they override Shutdown()

      // Don't shutdown our doc object!
      if (accessNode != NS_STATIC_CAST(nsIAccessNode*, this)) {
        if (aChangeEvent != nsIAccessibleEvent::EVENT_SHOW) {
          nsCOMPtr<nsIAccessible> accessible(do_QueryInterface(accessNode));
          if (accessible) {
            // Fire menupopupend events for menu popups that go away
            PRUint32 role, event = 0;
            accessible->GetFinalRole(&role);
            if (role == ROLE_MENUPOPUP) {
              nsCOMPtr<nsIDOMNode> domNode;
              accessNode->GetDOMNode(getter_AddRefs(domNode));
              nsCOMPtr<nsIDOMXULPopupElement> popup(do_QueryInterface(domNode));
              if (!popup) {
                // Popup elements already fire these via DOMMenuInactive
                // handling in nsRootAccessible::HandleEvent
                event = nsIAccessibleEvent::EVENT_MENUPOPUPEND;
              }
            }
            else if (role == ROLE_PROGRESSBAR && iterNode != aStartNode) {
              // Make sure EVENT_HIDE gets fired for progress meters
              event = nsIAccessibleEvent::EVENT_HIDE;
            }
            if (event) {
              FireToolkitEvent(event, accessible, nsnull);
            }
          }
        }
        void *uniqueID;
        accessNode->GetUniqueID(&uniqueID);
        nsCOMPtr<nsPIAccessNode> privateAccessNode(do_QueryInterface(accessNode));
        privateAccessNode->Shutdown();
        // Remove from hash table as well
        mAccessNodeCache.Remove(uniqueID);
      }
    }

    iterNode->GetFirstChild(getter_AddRefs(nextNode));
    if (nextNode) {
      iterNode = nextNode;
      continue;
    }

    if (iterNode == aStartNode)
      break;
    iterNode->GetNextSibling(getter_AddRefs(nextNode));
    if (nextNode) {
      iterNode = nextNode;
      continue;
    }

    do {
      iterNode->GetParentNode(getter_AddRefs(nextNode));
      if (!nextNode || nextNode == aStartNode) {
        return;
      }
      nextNode->GetNextSibling(getter_AddRefs(iterNode));
      if (iterNode)
        break;
      iterNode = nextNode;
    } while (PR_TRUE);
  }
  while (iterNode && iterNode != aStartNode);
}

NS_IMETHODIMP nsDocAccessible::InvalidateCacheSubtree(nsIContent *aChild,
                                                      PRUint32 aChangeEventType)
{
  NS_ASSERTION(aChangeEventType == nsIAccessibleEvent::EVENT_REORDER ||
               aChangeEventType == nsIAccessibleEvent::EVENT_SHOW ||
               aChangeEventType == nsIAccessibleEvent::EVENT_HIDE,
               "Incorrect aChangeEventType passed in");

  // Invalidate cache subtree
  // We have to check for accessibles for each dom node by traversing DOM tree
  // instead of just the accessible tree, although that would be faster
  // Otherwise we might miss the nsAccessNode's that are not nsAccessible's.

  nsCOMPtr<nsIDOMNode> childNode = aChild ? do_QueryInterface(aChild) : mDOMNode;

  if (!mIsContentLoaded && mAccessNodeCache.Count() <= 1) {
    return NS_OK; // Still loading and nothing to invalidate yet
  }

  else if (aChangeEventType == nsIAccessibleEvent::EVENT_HIDE) {
    // Fire EVENT_HIDE or EVENT_MENUPOPUPEND if previous accessible existed
    // for node being hidden. Fire this before the accessible goes away
    nsCOMPtr<nsIAccessNode> childAccessNode;
    GetCachedAccessNode(childNode, getter_AddRefs(childAccessNode));
    nsCOMPtr<nsIAccessible> childAccessible = do_QueryInterface(childAccessNode);
    if (childAccessible) {
      nsCOMPtr<nsPIAccessible> privateChildAccessible =
        do_QueryInterface(childAccessible);
      NS_ASSERTION(privateChildAccessible, "No nsPIAccessible for an nsIAccessible");
      privateChildAccessible->FireToolkitEvent(nsIAccessibleEvent::EVENT_HIDE,
                                               childAccessible, nsnull);
    }
  }

  // Shutdown nsIAccessNode's or nsIAccessibles for any DOM nodes in this subtree
  if (aChangeEventType != nsIAccessibleEvent::EVENT_SHOW) {
    RefreshNodes(childNode, aChangeEventType);
  }

  // We need to get an accessible for the mutation event's container node
  // If there is no accessible for that node, we need to keep moving up the parent
  // chain so there is some accessible.
  // We will use this accessible to fire the accessible mutation event.
  // We're guaranteed success, because we will eventually end up at the doc accessible,
  // and there is always one of those.

  nsCOMPtr<nsIAccessible> containerAccessible;
  if (childNode == mDOMNode) {
    // Don't get parent accessible if already at the root of a docshell chain like UI or content
    // Don't fire any other events if doc is still loading
    nsCOMPtr<nsISupports> container = mDocument->GetContainer();
    nsCOMPtr<nsIDocShellTreeItem> docShellTreeItem(do_QueryInterface(container));
    NS_ENSURE_TRUE(docShellTreeItem, NS_ERROR_FAILURE);
    nsCOMPtr<nsIDocShellTreeItem> sameTypeRoot;
    docShellTreeItem->GetSameTypeRootTreeItem(getter_AddRefs(sameTypeRoot));
    if (sameTypeRoot == docShellTreeItem) {
      containerAccessible = this;  // At the root of UI or content
    }
  }
  if (!containerAccessible) {
    GetAccessibleInParentChain(childNode, getter_AddRefs(containerAccessible));
  }
  nsCOMPtr<nsPIAccessible> privateContainerAccessible =
    do_QueryInterface(containerAccessible);
  if (privateContainerAccessible) {
    privateContainerAccessible->InvalidateChildren();
  }

  if (!mIsContentLoaded) {
    return NS_OK;
  }

  // Fire an event so the assistive technology knows the objects it is holding onto
  // in this part of the subtree are no longer useful and should be released.
  // However, they still won't crash if the AT tries to use them, because a stub of the
  // object still exists as long as it is refcounted, even from outside of Gecko.
  nsCOMPtr<nsIAccessNode> containerAccessNode =
    do_QueryInterface(containerAccessible);
  if (containerAccessNode) {
    nsCOMPtr<nsIDOMNode> containerNode;
    containerAccessNode->GetDOMNode(getter_AddRefs(containerNode));
    if (containerNode) {
      FireDelayedToolkitEvent(nsIAccessibleEvent::EVENT_REORDER,
                              containerNode, nsnull);
    }
  }

  if (aChangeEventType == nsIAccessibleEvent::EVENT_SHOW && aChild) {
    // Fire EVENT_SHOW, EVENT_MENUPOPUPSTART for newly visible content.
    // Fire after a short timer, because we want to make sure the view has been
    // updated to make this accessible content visible. If we don't wait,
    // the assistive technology may receive the event and then retrieve
    // STATE_INVISIBLE for the event's accessible object.
    FireDelayedToolkitEvent(nsIAccessibleEvent::EVENT_SHOW, childNode, nsnull);
    nsAutoString role;
    if (GetRoleAttribute(aChild, role) &&
        StringEndsWith(role, NS_LITERAL_STRING(":menu"), nsCaseInsensitiveStringComparator())) {
      FireDelayedToolkitEvent(nsIAccessibleEvent::EVENT_MENUPOPUPSTART, childNode, nsnull);
    }
  }

  // Check to see if change occured inside an alert, and fire an EVENT_ALERT if it did
  if (aChangeEventType != nsIAccessibleEvent::EVENT_HIDE) {
    nsIContent *ancestor = aChild;
    nsAutoString role;
    while (ancestor) {
      if (GetRoleAttribute(ancestor, role) &&
          StringEndsWith(role, NS_LITERAL_STRING(":alert"), nsCaseInsensitiveStringComparator())) {
        nsCOMPtr<nsIDOMNode> alertNode(do_QueryInterface(ancestor));
        FireDelayedToolkitEvent(nsIAccessibleEvent::EVENT_ALERT, alertNode, nsnull);
        break;
      }
      ancestor = ancestor->GetParent();
    }
  }

  return NS_OK;
}

NS_IMETHODIMP
nsDocAccessible::GetAccessibleInParentChain(nsIDOMNode *aNode,
                                            nsIAccessible **aAccessible)
{
  // Find a pre-existing accessible in parent chain of DOM nodes, or return null
  *aAccessible = nsnull;
  nsCOMPtr<nsIDOMNode> currentNode(aNode), parentNode;
  nsCOMPtr<nsIAccessNode> accessNode;

  do  {
    GetCachedAccessNode(currentNode, getter_AddRefs(accessNode));
    nsCOMPtr<nsIAccessible> accessible(do_QueryInterface(accessNode));
    if (accessible) {
      if (currentNode == aNode) {
        // We don't want an accessible for the passed-innode --
        // it must be from an ancestor
        return accessible->GetParent(aAccessible);
      }
      NS_ADDREF(*aAccessible = accessible);
      break;
    }
    currentNode->GetParentNode(getter_AddRefs(parentNode));
    currentNode = parentNode;
  }
  while (currentNode);

  return NS_OK;
}

NS_IMETHODIMP nsDocAccessible::FireToolkitEvent(PRUint32 aEvent, nsIAccessible* aAccessible, void* aData)
{
  nsCOMPtr<nsIObserverService> obsService =
    do_GetService("@mozilla.org/observer-service;1");
  if (!obsService) {
    return NS_ERROR_FAILURE;
  }

  nsCOMPtr<nsIAccessibleEvent> accEvent = new nsAccessibleEventData(aEvent, aAccessible, this, aData);
  NS_ENSURE_TRUE(accEvent, NS_ERROR_OUT_OF_MEMORY);

  return obsService->NotifyObservers(accEvent, NS_ACCESSIBLE_EVENT_TOPIC, nsnull);
}

