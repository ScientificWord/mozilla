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
 * The Original Code is Mozilla Communicator.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications.
 * Portions created by the Initial Developer are Copyright (C) 2002
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *  Mike Pinkerton <pinkerton@netscape.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
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

#include "nsReadableUtils.h"

// Local Includes
#include "nsContentAreaDragDrop.h"

// Helper Classes
#include "nsString.h"

// Interfaces needed to be included
#include "nsIDOMNSUIEvent.h"
#include "nsIDOMUIEvent.h"
#include "nsISelection.h"
#include "nsIDOMNode.h"
#include "nsIDOMNodeList.h"
#include "nsIDOMEvent.h"
#include "nsIDOMNSEvent.h"
#include "nsIDOMMouseEvent.h"
#include "nsIDOMAbstractView.h"
#include "nsPIDOMWindow.h"
#include "nsIDOMDocument.h"
#include "nsIDOMDocumentRange.h"
#include "nsIDOMRange.h"
#include "nsIDocumentEncoder.h"
#include "nsIFormControl.h"
#include "nsISelectionPrivate.h"
#include "nsIDOMHTMLAreaElement.h"
#include "nsIDOMHTMLAnchorElement.h"
#include "nsITransferable.h"
#include "nsIDragService.h"
#include "nsIDragSession.h"
#include "nsComponentManagerUtils.h"
#include "nsXPCOM.h"
#include "nsISupportsPrimitives.h"
#include "nsServiceManagerUtils.h"
#include "nsNetUtil.h"
#include "nsIFile.h"
#include "nsIWebNavigation.h"
#include "nsIClipboardDragDropHooks.h"
#include "nsIClipboardDragDropHookList.h"
#include "nsIDocShell.h"
#include "nsIContent.h"
#include "nsIImageLoadingContent.h"
#include "nsINameSpaceManager.h"
#include "nsUnicharUtils.h"
#include "nsIURL.h"
#include "nsIImage.h"
#include "nsIDocument.h"
#include "nsIScriptSecurityManager.h"
#include "nsIPresShell.h"
#include "nsPresContext.h"
#include "nsIDocShellTreeItem.h"
#include "nsIFrame.h"
#include "nsRange.h"
#include "nsIWebBrowserPersist.h"
#include "nsEscape.h"
#include "nsContentUtils.h"
#include "nsIMIMEService.h"
#include "imgIRequest.h"
#include "nsContentCID.h"
#include "nsISelectionController.h"
#include "nsFrameSelection.h"
#include "nsIDOMEventTarget.h"
#include "nsWidgetsCID.h"

static NS_DEFINE_CID(kHTMLConverterCID,        NS_HTMLFORMATCONVERTER_CID);

// private clipboard data flavors for html copy, used by editor when pasting
#define kHTMLContext   "text/_moz_htmlcontext"
#define kHTMLInfo      "text/_moz_htmlinfo"


NS_IMPL_ADDREF(nsContentAreaDragDrop)
NS_IMPL_RELEASE(nsContentAreaDragDrop)

NS_INTERFACE_MAP_BEGIN(nsContentAreaDragDrop)
    NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(nsISupports, nsIDOMDragListener)
    NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(nsIDOMEventListener, nsIDOMDragListener)
    NS_INTERFACE_MAP_ENTRY(nsIDOMDragListener)
    NS_INTERFACE_MAP_ENTRY(nsIFlavorDataProvider)
    NS_INTERFACE_MAP_ENTRY(nsIDragDropHandler)
NS_INTERFACE_MAP_END


class nsTransferableFactory
{
public:
  nsTransferableFactory(nsIDOMEvent* inMouseEvent,
                        nsIFlavorDataProvider *inFlavorDataProvider);
  nsresult Produce(PRBool *aDragSelection, nsITransferable** outTrans);

private:
  nsresult ConvertStringsToTransferable(nsITransferable** outTrans);
  static nsresult GetDraggableSelectionData(nsISelection* inSelection,
                                            nsIDOMNode* inRealTargetNode,
                                            nsIDOMNode **outImageOrLinkNode,
                                            PRBool* outDragSelectedText);
  static already_AddRefed<nsIDOMNode> FindParentLinkNode(nsIDOMNode* inNode);
  static void GetAnchorURL(nsIDOMNode* inNode, nsAString& outURL);
  static void GetNodeString(nsIDOMNode* inNode, nsAString & outNodeString);
  static void CreateLinkText(const nsAString& inURL, const nsAString & inText,
                              nsAString& outLinkText);
  static void GetSelectedLink(nsISelection* inSelection,
                              nsIDOMNode **outLinkNode);

  // if inNode is null, use the selection from the window
  static nsresult SerializeNodeOrSelection(nsIDOMWindow* inWindow,
                                           nsIDOMNode* inNode,
                                           nsAString& outResultString,
                                           nsAString& outHTMLContext,
                                           nsAString& outHTMLInfo);

  PRBool mInstanceAlreadyUsed;

  nsCOMPtr<nsIDOMEvent> mMouseEvent;
  nsCOMPtr<nsIFlavorDataProvider> mFlavorDataProvider;

  nsString mUrlString;
  nsString mImageSourceString;
  nsString mImageDestFileName;
  nsString mTitleString;
  // will be filled automatically if you fill urlstring
  nsString mHtmlString;
  nsString mContextString;
  nsString mInfoString;

  PRBool mIsAnchor;
  nsCOMPtr<nsIImage> mImage;
};


//
// nsContentAreaDragDrop ctor
//
nsContentAreaDragDrop::nsContentAreaDragDrop()
  : mListenerInstalled(PR_FALSE), mNavigator(nsnull)
{
} // ctor


//
// ChromeTooltipListener dtor
//
nsContentAreaDragDrop::~nsContentAreaDragDrop()
{
  RemoveDragListener();

} // dtor


NS_IMETHODIMP
nsContentAreaDragDrop::HookupTo(nsIDOMEventTarget *inAttachPoint,
                                nsIWebNavigation* inNavigator)
{
  NS_ASSERTION(inAttachPoint, "Can't hookup Drag Listeners to NULL receiver");
  mEventTarget = do_QueryInterface(inAttachPoint);
  NS_ASSERTION(mEventTarget,
               "Target doesn't implement nsPIDOMEventTarget as needed");
  mNavigator = inNavigator;

  return AddDragListener();
}


NS_IMETHODIMP
nsContentAreaDragDrop::Detach()
{
  return RemoveDragListener();
}


//
// AddDragListener
//
// Subscribe to the events that will allow us to track drags.
//
nsresult
nsContentAreaDragDrop::AddDragListener()
{
  nsresult rv = NS_ERROR_FAILURE;

  if (mEventTarget) {
    nsIDOMDragListener *pListener = static_cast<nsIDOMDragListener *>(this);
    rv = mEventTarget->AddEventListenerByIID(pListener,
                                             NS_GET_IID(nsIDOMDragListener));
    if (NS_SUCCEEDED(rv))
      mListenerInstalled = PR_TRUE;
  }

  return rv;
}


//
// RemoveDragListener
//
// Unsubscribe from all the various drag events that we were listening to.
//
nsresult
nsContentAreaDragDrop::RemoveDragListener()
{
  nsresult rv = NS_ERROR_FAILURE;

  if (mEventTarget) {
    nsIDOMDragListener *pListener = static_cast<nsIDOMDragListener *>(this);
    rv =
      mEventTarget->RemoveEventListenerByIID(pListener,
                                             NS_GET_IID(nsIDOMDragListener));
    if (NS_SUCCEEDED(rv))
      mListenerInstalled = PR_FALSE;
    mEventTarget = nsnull;
  }

  return rv;
}



//
// DragEnter
//
// Called when an OS drag is in process and the mouse enters a gecko
// window. We don't care so much about dragEnters.
//
NS_IMETHODIMP
nsContentAreaDragDrop::DragEnter(nsIDOMEvent* aMouseEvent)
{
  // nothing really to do here.
  return NS_OK;
}


//
// DragOver
//
// Called when an OS drag is in process and the mouse is over a gecko
// window.  The main purpose of this routine is to set the |canDrop|
// property on the drag session to false if we want to disallow the
// drop so that the OS can provide the appropriate feedback. All this
// does is show feedback, it doesn't actually cancel the drop; that
// comes later.
//
NS_IMETHODIMP
nsContentAreaDragDrop::DragOver(nsIDOMEvent* inEvent)
{
  // first check that someone hasn't already handled this event
  PRBool preventDefault = PR_TRUE;
  nsCOMPtr<nsIDOMNSUIEvent> nsuiEvent(do_QueryInterface(inEvent));
  if ( nsuiEvent )
    nsuiEvent->GetPreventDefault(&preventDefault);
  if ( preventDefault )
    return NS_OK;

  // if the drag originated w/in this content area, bail
  // early. This avoids loading a URL dragged from the content
  // area into the very same content area (which is almost never
  // the desired action).
  nsCOMPtr<nsIDragService> dragService =
    do_GetService("@mozilla.org/widget/dragservice;1");
  if (!dragService)
    return NS_ERROR_FAILURE;

  nsCOMPtr<nsIDragSession> session;
  dragService->GetCurrentSession(getter_AddRefs(session));

  if (session) {
    // if the client has provided an override callback, check if we
    // the drop is allowed. If it allows it, we should still protect
    // against dropping w/in the same document.
    PRBool dropAllowed = PR_TRUE;
    nsCOMPtr<nsISimpleEnumerator> enumerator;
    GetHookEnumeratorFromEvent(inEvent, getter_AddRefs(enumerator));

    if (enumerator) {
      PRBool hasMoreHooks = PR_FALSE;
      while (NS_SUCCEEDED(enumerator->HasMoreElements(&hasMoreHooks))
             && hasMoreHooks) {
        nsCOMPtr<nsISupports> isupp;
        if (NS_FAILED(enumerator->GetNext(getter_AddRefs(isupp)))) {
          break;
        }

        nsCOMPtr<nsIClipboardDragDropHooks> override =
          do_QueryInterface(isupp);

        if (override) {
#ifdef DEBUG
          nsresult hookResult =
#endif
          override->AllowDrop(inEvent, session, &dropAllowed);
          NS_ASSERTION(NS_SUCCEEDED(hookResult), "hook failure in AllowDrop");

          if (!dropAllowed) {
            break;
          }
        }
      }
    }

    nsCOMPtr<nsIDOMDocument> sourceDoc;
    session->GetSourceDocument(getter_AddRefs(sourceDoc));
    nsCOMPtr<nsIDOMDocument> eventDoc;
    GetEventDocument(inEvent, getter_AddRefs(eventDoc));

    if (sourceDoc == eventDoc) {  // common case
      dropAllowed = PR_FALSE;
    } else if (sourceDoc && eventDoc) {
      // dig deeper
      // XXXbz we need better ways to get from a document to the docshell!
      nsCOMPtr<nsIDocument> sourceDocument(do_QueryInterface(sourceDoc));
      nsCOMPtr<nsIDocument> eventDocument(do_QueryInterface(eventDoc));
      NS_ASSERTION(sourceDocument, "Confused document object");
      NS_ASSERTION(eventDocument, "Confused document object");

      nsPIDOMWindow* sourceWindow = sourceDocument->GetWindow();
      nsPIDOMWindow* eventWindow = eventDocument->GetWindow();

      if (sourceWindow && eventWindow) {
        nsCOMPtr<nsIDocShellTreeItem> sourceShell =
          do_QueryInterface(sourceWindow->GetDocShell());
        nsCOMPtr<nsIDocShellTreeItem> eventShell =
          do_QueryInterface(eventWindow->GetDocShell());

        if (sourceShell && eventShell) {
          // Whew.  Almost there.  Get the roots that are of the same type
          // (otherwise we'll always end up with the root docshell for the
          // window, and drag/drop from chrom to content won't work).
          nsCOMPtr<nsIDocShellTreeItem> sourceRoot;
          nsCOMPtr<nsIDocShellTreeItem> eventRoot;
          sourceShell->GetSameTypeRootTreeItem(getter_AddRefs(sourceRoot));
          eventShell->GetSameTypeRootTreeItem(getter_AddRefs(eventRoot));

          if (sourceRoot && sourceRoot == eventRoot) {
            dropAllowed = PR_FALSE;
          }
        }
      }
    }

    session->SetCanDrop(dropAllowed);
  }

  return NS_OK;
}


//
// DragExit
//
// Called when an OS drag is in process and the mouse is over a gecko
// window. We don't care so much about dragExits.
//
NS_IMETHODIMP
nsContentAreaDragDrop::DragExit(nsIDOMEvent* aMouseEvent)
{
  // nothing really to do here.
  return NS_OK;
}


NS_IMETHODIMP
nsContentAreaDragDrop::Drag(nsIDOMEvent* aMouseEvent)
{
  // nothing really to do here.
  return NS_OK;
}


NS_IMETHODIMP
nsContentAreaDragDrop::DragEnd(nsIDOMEvent* aMouseEvent)
{
  // nothing really to do here.
  return NS_OK;
}


//
// ExtractURLFromData
//
// build up a url from whatever data we get from the OS. How we
// interpret the data depends on the flavor as it tells us the
// nsISupports* primitive type we have.
//
void
nsContentAreaDragDrop::ExtractURLFromData(const nsACString & inFlavor,
                                          nsISupports* inDataWrapper,
                                          PRUint32 inDataLen,
                                          nsAString & outURL)
{
  if (!inDataWrapper) {
    return;
  }

  outURL.Truncate();

  if (inFlavor.Equals(kUnicodeMime)  || inFlavor.Equals(kURLDataMime)) {
    // the data is regular unicode, just go with what we get. It may
    // be a url, it may not be. *shrug*
    nsCOMPtr<nsISupportsString> stringData(do_QueryInterface(inDataWrapper));
    if (stringData) {
      stringData->GetData(outURL);
    }
  }
  else if (inFlavor.Equals(kURLMime)) {
    // the data is an internet shortcut of the form
    // <url>\n<title>. Strip out the url piece and return that.
    nsCOMPtr<nsISupportsString> stringData(do_QueryInterface(inDataWrapper));

    if (stringData) {
      nsAutoString data;
      stringData->GetData(data);
      PRInt32 separator = data.FindChar('\n');

      if (separator >= 0)
        outURL = Substring(data, 0, separator);
      else
        outURL = data;
    }
  }
  else if (inFlavor.Equals(kFileMime)) {
    // the data is a file. Use the necko parsing utils to get a file:// url
    // from the OS data.
    nsCOMPtr<nsIFile> file(do_QueryInterface(inDataWrapper));
    if (file) {
      nsCAutoString url;
      NS_GetURLSpecFromFile(file, url);
      CopyUTF8toUTF16(url, outURL);
    }
  }
}


//
// DragDrop
//
// Called when an OS drag is in process and the mouse is released a
// gecko window.  Extract the data from the OS and do something with
// it.
//
NS_IMETHODIMP
nsContentAreaDragDrop::DragDrop(nsIDOMEvent* inMouseEvent)
{
  // if we don't have a nsIWebNavigation object to do anything with,
  // just bail. The client will have to have another way to deal with it
  if (!mNavigator) {
    return NS_OK;
  }

  // check that someone hasn't already handled this event
  PRBool preventDefault = PR_TRUE;
  nsCOMPtr<nsIDOMNSUIEvent> nsuiEvent(do_QueryInterface(inMouseEvent));
  if (nsuiEvent) {
    nsuiEvent->GetPreventDefault(&preventDefault);
  }

  if (preventDefault) {
    return NS_OK;
  }

  // pull the transferable out of the drag service. at the moment, we
  // only care about the first item of the drag. We don't allow
  // dropping multiple items into a content area.
  nsCOMPtr<nsIDragService> dragService =
    do_GetService("@mozilla.org/widget/dragservice;1");
  if (!dragService) {
    return NS_ERROR_FAILURE;
  }

  nsCOMPtr<nsIDragSession> session;
  dragService->GetCurrentSession(getter_AddRefs(session));
  if (!session) {
    return NS_ERROR_FAILURE;
  }

  nsCOMPtr<nsITransferable> trans =
    do_CreateInstance("@mozilla.org/widget/transferable;1");
  if (!trans) {
    return NS_ERROR_FAILURE;
  }

  // add the relevant flavors. order is important (highest fidelity to lowest)
  trans->AddDataFlavor(kURLDataMime);
  trans->AddDataFlavor(kURLMime);
  trans->AddDataFlavor(kFileMime);
  trans->AddDataFlavor(kUnicodeMime);

  // again, we only care about the first object
  nsresult rv = session->GetData(trans, 0);

  if (NS_SUCCEEDED(rv)) {
    // if the client has provided an override callback, call it. It may
    // still return that we should continue processing.
    nsCOMPtr<nsISimpleEnumerator> enumerator;
    GetHookEnumeratorFromEvent(inMouseEvent, getter_AddRefs(enumerator));

    if (enumerator) {
      PRBool actionCanceled = PR_TRUE;
      PRBool hasMoreHooks = PR_FALSE;
      while (NS_SUCCEEDED(enumerator->HasMoreElements(&hasMoreHooks))
             && hasMoreHooks) {
        nsCOMPtr<nsISupports> isupp;
        if (NS_FAILED(enumerator->GetNext(getter_AddRefs(isupp))))
          break;
        nsCOMPtr<nsIClipboardDragDropHooks> override =
          do_QueryInterface(isupp);

        if (override) {
#ifdef DEBUG
          nsresult hookResult =
#endif
          override->OnPasteOrDrop(inMouseEvent, trans, &actionCanceled);
          NS_ASSERTION(NS_SUCCEEDED(hookResult),
                       "hook failure in OnPasteOrDrop");
          if (!actionCanceled)
            return NS_OK;
        }
      }
    }

    nsXPIDLCString flavor;
    nsCOMPtr<nsISupports> dataWrapper;
    PRUint32 dataLen = 0;
    rv = trans->GetAnyTransferData(getter_Copies(flavor),
                                   getter_AddRefs(dataWrapper), &dataLen);
    if (NS_SUCCEEDED(rv) && dataLen > 0) {
      // get the url from one of several possible formats
      nsAutoString url;
      ExtractURLFromData(flavor, dataWrapper, dataLen, url);
      NS_ASSERTION(!url.IsEmpty(), "Didn't get anything we can use as a url");

      // valid urls don't have spaces. bail if this does.
      if (url.IsEmpty() || url.FindChar(' ') >= 0)
        return NS_OK;

      nsCOMPtr<nsIURI> uri;
      NS_NewURI(getter_AddRefs(uri), url);
      if (!uri) {
        // Not actually a URI
        return NS_OK;
      }

      nsCOMPtr<nsIDOMDocument> sourceDocument;
      session->GetSourceDocument(getter_AddRefs(sourceDocument));

      nsCOMPtr<nsIDocument> sourceDoc(do_QueryInterface(sourceDocument));
      if (sourceDoc) {
        rv = nsContentUtils::GetSecurityManager()->
          CheckLoadURIWithPrincipal(sourceDoc->NodePrincipal(), uri,
                                    nsIScriptSecurityManager::STANDARD);

        if (NS_FAILED(rv)) {
          // Security check failed, stop event propagation right here
          // and return the error.
          inMouseEvent->StopPropagation();

          return rv;
        }
      }

      // ok, we have the url, load it.
      mNavigator->LoadURI(url.get(), nsIWebNavigation::LOAD_FLAGS_NONE, nsnull,
                          nsnull, nsnull);
    }
  }

  return NS_OK;
}

//
// NormalizeSelection
//
void
nsContentAreaDragDrop::NormalizeSelection(nsIDOMNode* inBaseNode,
                                          nsISelection* inSelection)
{
  nsCOMPtr<nsIDOMNode> parent;
  inBaseNode->GetParentNode(getter_AddRefs(parent));
  if (!parent || !inSelection)
    return;

  nsCOMPtr<nsIDOMNodeList> childNodes;
  parent->GetChildNodes(getter_AddRefs(childNodes));
  if (!childNodes)
    return;
  PRUint32 listLen = 0;
  childNodes->GetLength(&listLen);

  PRUint32 index = 0;
  for (; index < listLen; ++index) {
    nsCOMPtr<nsIDOMNode> indexedNode;
    childNodes->Item(index, getter_AddRefs(indexedNode));
    if (indexedNode == inBaseNode) {
      break;
    }
  }

  if (index >= listLen) {
    return;
  }

  // now make the selection contain all of |inBaseNode|'s siblings up
  // to and including |inBaseNode|
  inSelection->Collapse(parent, index);
  inSelection->Extend(parent, index+1);
}


//
// GetEventDocument
//
// Get the DOM document associated with a given DOM event
//
void
nsContentAreaDragDrop::GetEventDocument(nsIDOMEvent* inEvent,
                                        nsIDOMDocument** outDocument)
{
  *outDocument = nsnull;

  nsCOMPtr<nsIDOMUIEvent> uiEvent(do_QueryInterface(inEvent));
  if (uiEvent) {
    nsCOMPtr<nsIDOMAbstractView> view;
    uiEvent->GetView(getter_AddRefs(view));
    nsCOMPtr<nsIDOMWindow> window(do_QueryInterface(view));

    if (window) {
      window->GetDocument(outDocument);
    }
  }
}

nsresult
nsContentAreaDragDrop::GetHookEnumeratorFromEvent(nsIDOMEvent* inEvent,
                                                  nsISimpleEnumerator **outEnumerator)
{
  *outEnumerator = nsnull;

  nsCOMPtr<nsIDOMDocument> domdoc;
  GetEventDocument(inEvent, getter_AddRefs(domdoc));
  nsCOMPtr<nsIDocument> doc = do_QueryInterface(domdoc);
  NS_ENSURE_TRUE(doc, NS_ERROR_FAILURE);

  nsCOMPtr<nsISupports> container = doc->GetContainer();
  nsCOMPtr<nsIDocShell> docShell = do_QueryInterface(container);
  NS_ENSURE_TRUE(docShell, NS_ERROR_FAILURE);

  nsCOMPtr<nsIClipboardDragDropHookList> hookList = do_GetInterface(docShell);
  NS_ENSURE_TRUE(hookList, NS_ERROR_FAILURE);
  nsCOMPtr<nsISimpleEnumerator> enumerator;
  hookList->GetHookEnumerator(getter_AddRefs(enumerator));
  NS_ENSURE_TRUE(enumerator, NS_ERROR_FAILURE);

  *outEnumerator = enumerator;
  NS_ADDREF(*outEnumerator);

  return NS_OK;
}

//
// DragGesture
//
// Determine if the user has started to drag something and kick off
// an OS-level drag if it's applicable
//
NS_IMETHODIMP
nsContentAreaDragDrop::DragGesture(nsIDOMEvent* inMouseEvent)
{
  // first check that someone hasn't already handled this event
  PRBool preventDefault = PR_TRUE;
  nsCOMPtr<nsIDOMNSUIEvent> nsuiEvent(do_QueryInterface(inMouseEvent));
  if (nsuiEvent) {
    nsuiEvent->GetPreventDefault(&preventDefault);
  }

  if (preventDefault) {
    return NS_OK;
  }

  // if the client has provided an override callback, check if we
  // should continue
  nsCOMPtr<nsISimpleEnumerator> enumerator;
  GetHookEnumeratorFromEvent(inMouseEvent, getter_AddRefs(enumerator));

  if (enumerator) {
    PRBool allow = PR_TRUE;
    PRBool hasMoreHooks = PR_FALSE;
    while (NS_SUCCEEDED(enumerator->HasMoreElements(&hasMoreHooks))
           && hasMoreHooks) {
      nsCOMPtr<nsISupports> isupp;
      if (NS_FAILED(enumerator->GetNext(getter_AddRefs(isupp))))
        break;

      nsCOMPtr<nsIClipboardDragDropHooks> override = do_QueryInterface(isupp);
      if (override) {
#ifdef DEBUG
        nsresult hookResult =
#endif
        override->AllowStartDrag(inMouseEvent, &allow);
        NS_ASSERTION(NS_SUCCEEDED(hookResult),
                     "hook failure in AllowStartDrag");

        if (!allow)
          return NS_OK;
      }
    }
  }

  PRBool isSelection = PR_FALSE;
  nsCOMPtr<nsITransferable> trans;
  nsTransferableFactory factory(inMouseEvent, static_cast<nsIFlavorDataProvider*>(this));
  factory.Produce(&isSelection, getter_AddRefs(trans));

  if (trans) {
    // if the client has provided an override callback, let them manipulate
    // the flavors or drag data
    nsCOMPtr<nsISimpleEnumerator> enumerator;
    GetHookEnumeratorFromEvent(inMouseEvent, getter_AddRefs(enumerator));
    if (enumerator) {
      PRBool hasMoreHooks = PR_FALSE;
      PRBool doContinueDrag = PR_TRUE;
      while (NS_SUCCEEDED(enumerator->HasMoreElements(&hasMoreHooks))
             && hasMoreHooks) {
        nsCOMPtr<nsISupports> isupp;
        if (NS_FAILED(enumerator->GetNext(getter_AddRefs(isupp))))
          break;
        nsCOMPtr<nsIClipboardDragDropHooks> override =
          do_QueryInterface(isupp);

        if (override) {
#ifdef DEBUG
          nsresult hookResult =
#endif
          override->OnCopyOrDrag(inMouseEvent, trans, &doContinueDrag);
          NS_ASSERTION(NS_SUCCEEDED(hookResult),
                       "hook failure in OnCopyOrDrag");

          if (!doContinueDrag) {
            return NS_OK;
          }
        }
      }
    }

    nsCOMPtr<nsISupportsArray> transArray =
      do_CreateInstance("@mozilla.org/supports-array;1");
    if (!transArray) {
      return NS_ERROR_FAILURE;
    }

    transArray->InsertElementAt(trans, 0);

    // kick off the drag
    nsCOMPtr<nsIDOMEventTarget> target;
    inMouseEvent->GetTarget(getter_AddRefs(target));
    nsCOMPtr<nsIDragService> dragService =
      do_GetService("@mozilla.org/widget/dragservice;1");

    if (!dragService) {
      return NS_ERROR_FAILURE;
    }

    PRUint32 action = nsIDragService::DRAGDROP_ACTION_COPY +
                      nsIDragService::DRAGDROP_ACTION_MOVE +
                      nsIDragService::DRAGDROP_ACTION_LINK;

    nsCOMPtr<nsIDOMMouseEvent> mouseEvent(do_QueryInterface(inMouseEvent));

    if (isSelection) {
      nsCOMPtr<nsIContent> targetContent(do_QueryInterface(target));
      nsIDocument* doc = targetContent->GetCurrentDoc();
      if (doc) {
        nsIPresShell* presShell = doc->GetPrimaryShell();
        if (presShell) {
          nsISelection* selection =
            presShell->GetCurrentSelection(nsISelectionController::SELECTION_NORMAL);
          return dragService->InvokeDragSessionWithSelection(selection,
                                                             transArray,
                                                             action,
                                                             mouseEvent);
        }
      }
    }

    nsCOMPtr<nsIDOMNode> targetNode(do_QueryInterface(target));
    dragService->InvokeDragSessionWithImage(targetNode, transArray, nsnull,
                                            action, nsnull, 0, 0, mouseEvent);
  }

  return NS_OK;
}


NS_IMETHODIMP
nsContentAreaDragDrop::HandleEvent(nsIDOMEvent *event)
{
  return NS_OK;

}

#if 0
#pragma mark -
#endif

// SaveURIToFile
// used on platforms where it's possible to drag items (e.g. images)
// into the file system
nsresult
nsContentAreaDragDrop::SaveURIToFile(nsAString& inSourceURIString,
                                     nsIFile* inDestFile)
{
  nsCOMPtr<nsIURI> sourceURI;
  nsresult rv = NS_NewURI(getter_AddRefs(sourceURI), inSourceURIString);
  if (NS_FAILED(rv)) {
    return NS_ERROR_FAILURE;
  }

  nsCOMPtr<nsIURL> sourceURL = do_QueryInterface(sourceURI);
  if (!sourceURL) {
    return NS_ERROR_NO_INTERFACE;
  }

  rv = inDestFile->CreateUnique(nsIFile::NORMAL_FILE_TYPE, 0600);
  NS_ENSURE_SUCCESS(rv, rv);

  // we rely on the fact that the WPB is refcounted by the channel etc,
  // so we don't keep a ref to it. It will die when finished.
  nsCOMPtr<nsIWebBrowserPersist> persist =
    do_CreateInstance("@mozilla.org/embedding/browser/nsWebBrowserPersist;1",
                      &rv);
  NS_ENSURE_SUCCESS(rv, rv);

  return persist->SaveURI(sourceURI, nsnull, nsnull, nsnull, nsnull,
                          inDestFile);
}

// This is our nsIFlavorDataProvider callback. There are several
// assumptions here that make this work:
//
// 1. Someone put a kFilePromiseURLMime flavor into the transferable
//    with the source URI of the file to save (as a string). We did
//    that above.
//
// 2. Someone put a kFilePromiseDirectoryMime flavor into the
//    transferable with an nsILocalFile for the directory we are to
//    save in. That has to be done by platform-specific code (in
//    widget), // which gets the destination directory from
//    OS-specific drag // information.
//
NS_IMETHODIMP
nsContentAreaDragDrop::GetFlavorData(nsITransferable *aTransferable,
                                     const char *aFlavor, nsISupports **aData,
                                     PRUint32 *aDataLen)
{
  NS_ENSURE_ARG_POINTER(aData && aDataLen);
  *aData = nsnull;
  *aDataLen = 0;

  nsresult rv = NS_ERROR_NOT_IMPLEMENTED;

  if (strcmp(aFlavor, kFilePromiseMime) == 0) {
    // get the URI from the kFilePromiseURLMime flavor
    NS_ENSURE_ARG(aTransferable);
    nsCOMPtr<nsISupports> tmp;
    PRUint32 dataSize = 0;
    aTransferable->GetTransferData(kFilePromiseURLMime,
                                   getter_AddRefs(tmp), &dataSize);
    nsCOMPtr<nsISupportsString> supportsString =
      do_QueryInterface(tmp);
    if (!supportsString)
      return NS_ERROR_FAILURE;

    nsAutoString sourceURLString;
    supportsString->GetData(sourceURLString);
    if (sourceURLString.IsEmpty())
      return NS_ERROR_FAILURE;

    aTransferable->GetTransferData(kFilePromiseDestFilename,
                                   getter_AddRefs(tmp), &dataSize);
    supportsString = do_QueryInterface(tmp);
    if (!supportsString)
      return NS_ERROR_FAILURE;

    nsAutoString targetFilename;
    supportsString->GetData(targetFilename);
    if (targetFilename.IsEmpty())
      return NS_ERROR_FAILURE;

    // get the target directory from the kFilePromiseDirectoryMime
    // flavor
    nsCOMPtr<nsISupports> dirPrimitive;
    dataSize = 0;
    aTransferable->GetTransferData(kFilePromiseDirectoryMime,
                                   getter_AddRefs(dirPrimitive), &dataSize);
    nsCOMPtr<nsILocalFile> destDirectory = do_QueryInterface(dirPrimitive);
    if (!destDirectory)
      return NS_ERROR_FAILURE;

    nsCOMPtr<nsIFile> file;
    rv = destDirectory->Clone(getter_AddRefs(file));
    NS_ENSURE_SUCCESS(rv, rv);

    file->Append(targetFilename);

    // now save the file
    rv = SaveURIToFile(sourceURLString, file);

    // send back an nsILocalFile
    if (NS_SUCCEEDED(rv)) {
      CallQueryInterface(file, aData);
      *aDataLen = sizeof(nsIFile*);
    }
  }

  return rv;
}

nsTransferableFactory::nsTransferableFactory(nsIDOMEvent* inMouseEvent,
                                             nsIFlavorDataProvider *dataProvider)
  : mInstanceAlreadyUsed(PR_FALSE),
    mMouseEvent(inMouseEvent),
    mFlavorDataProvider(dataProvider)
{
}


//
// FindParentLinkNode
//
// Finds the parent with the given link tag starting at |inNode|. If
// it gets up to the root without finding it, we stop looking and
// return null.
//
already_AddRefed<nsIDOMNode>
nsTransferableFactory::FindParentLinkNode(nsIDOMNode* inNode)
{
  nsCOMPtr<nsIContent> content(do_QueryInterface(inNode));
  if (!content) {
    // That must have been the document node; nothing else to do here;
    return nsnull;
  }

  for (; content; content = content->GetParent()) {
    if (nsContentUtils::IsDraggableLink(content)) {
      nsIDOMNode* node = nsnull;
      CallQueryInterface(content, &node);
      return node;
    }
  }

  return nsnull;
}


//
// GetAnchorURL
//
void
nsTransferableFactory::GetAnchorURL(nsIDOMNode* inNode, nsAString& outURL)
{
  nsCOMPtr<nsIURI> linkURI;
  nsCOMPtr<nsIContent> content = do_QueryInterface(inNode);
  if (!content || !content->IsLink(getter_AddRefs(linkURI))) {
    // Not a link
    outURL.Truncate();
    return;
  }

  nsCAutoString spec;
  linkURI->GetSpec(spec);
  CopyUTF8toUTF16(spec, outURL);
}


//
// CreateLinkText
//
// Creates the html for an anchor in the form
//  <a href="inURL">inText</a>
//
void
nsTransferableFactory::CreateLinkText(const nsAString& inURL,
                                      const nsAString & inText,
                                      nsAString& outLinkText)
{
  // use a temp var in case |inText| is the same string as
  // |outLinkText| to avoid overwriting it while building up the
  // string in pieces.
  nsAutoString linkText(NS_LITERAL_STRING("<a href=\"") +
                        inURL +
                        NS_LITERAL_STRING("\">") +
                        inText +
                        NS_LITERAL_STRING("</a>") );

  outLinkText = linkText;
}


//
// GetNodeString
//
// Gets the text associated with a node
//
void
nsTransferableFactory::GetNodeString(nsIDOMNode* inNode,
                                     nsAString & outNodeString)
{
  outNodeString.Truncate();

  // use a range to get the text-equivalent of the node
  nsCOMPtr<nsIDOMDocument> doc;
  inNode->GetOwnerDocument(getter_AddRefs(doc));
  nsCOMPtr<nsIDOMDocumentRange> docRange(do_QueryInterface(doc));
  if (docRange) {
    nsCOMPtr<nsIDOMRange> range;
    docRange->CreateRange(getter_AddRefs(range));
    if (range) {
      range->SelectNode(inNode);
      range->ToString(outNodeString);
    }
  }
}


nsresult
nsTransferableFactory::Produce(PRBool* aDragSelection,
                               nsITransferable** outTrans)
{
  if (mInstanceAlreadyUsed) {
    return NS_ERROR_FAILURE;
  }

  if (!outTrans || !mMouseEvent || !mFlavorDataProvider) {
    return NS_ERROR_FAILURE;
  }

  mInstanceAlreadyUsed = PR_TRUE;
  *outTrans = nsnull;

  nsCOMPtr<nsIDOMWindow> window;
  PRBool isAltKeyDown = PR_FALSE;
  mIsAnchor = PR_FALSE;

  {
    nsCOMPtr<nsIDOMUIEvent> uiEvent(do_QueryInterface(mMouseEvent));
    if (!uiEvent) {
      return NS_OK;
    }

    // find the selection to see what we could be dragging and if
    // what we're dragging is in what is selected.
    nsCOMPtr<nsIDOMAbstractView> view;
    uiEvent->GetView(getter_AddRefs(view));
    window = do_QueryInterface(view);
    if (!window) {
      return NS_OK;
    }
  }

  {
    nsCOMPtr<nsIDOMMouseEvent> mouseEvent(do_QueryInterface(mMouseEvent));
    if (mouseEvent) {
      mouseEvent->GetAltKey(&isAltKeyDown);
    }
  }

  nsCOMPtr<nsISelection> selection;
  window->GetSelection(getter_AddRefs(selection));
  if (!selection) {
    return NS_OK;
  }

  // if set, serialize the content under this node
  nsCOMPtr<nsIDOMNode> nodeToSerialize;
  PRBool useSelectedText = PR_FALSE;
  *aDragSelection = PR_FALSE;

  {
    PRBool haveSelectedContent = PR_FALSE;

    // possible parent link node
    nsCOMPtr<nsIDOMNode> parentLink;
    nsCOMPtr<nsIDOMNode> draggedNode;

    {
      nsCOMPtr<nsIDOMEventTarget> target;
      mMouseEvent->GetTarget(getter_AddRefs(target));

      // only drag form elements by using the alt key,
      // otherwise buttons and select widgets are hard to use

      // Note that while <object> elements implement nsIFormControl, we should
      // really allow dragging them if they happen to be images.
      nsCOMPtr<nsIFormControl> form(do_QueryInterface(target));
      if (form && !isAltKeyDown && form->GetType() != NS_FORM_OBJECT) {
        return NS_OK;
      }

      draggedNode = do_QueryInterface(target);
    }

    nsCOMPtr<nsIDOMHTMLAreaElement>   area;   // client-side image map
    nsCOMPtr<nsIImageLoadingContent>  image;
    nsCOMPtr<nsIDOMHTMLAnchorElement> link;

    {
      // Get the real target and see if it is in the selection
      nsCOMPtr<nsIDOMNode> realTargetNode;

      {
        nsCOMPtr<nsIDOMNSEvent> internalEvent = do_QueryInterface(mMouseEvent);
        if (internalEvent) {
          nsCOMPtr<nsIDOMEventTarget> realTarget;
          internalEvent->GetExplicitOriginalTarget(getter_AddRefs(realTarget));
          realTargetNode = do_QueryInterface(realTarget);
        }
      }

      {
        nsCOMPtr<nsIDOMNode> selectedImageOrLinkNode;
        GetDraggableSelectionData(selection, realTargetNode,
                                  getter_AddRefs(selectedImageOrLinkNode),
                                  &haveSelectedContent);

        // either plain text or anchor text is selected
        if (haveSelectedContent) {
          link = do_QueryInterface(selectedImageOrLinkNode);
          if (link && isAltKeyDown) {
            return NS_OK;
          }

          useSelectedText = PR_TRUE;
          *aDragSelection = PR_TRUE;
        } else if (selectedImageOrLinkNode) {
          // an image is selected
          image = do_QueryInterface(selectedImageOrLinkNode);
        } else {
          // nothing is selected -
          //
          // look for draggable elements under the mouse
          //
          // if the alt key is down, don't start a drag if we're in an
          // anchor because we want to do selection.
          parentLink = FindParentLinkNode(draggedNode);
          if (parentLink && isAltKeyDown) {
            return NS_OK;
          }

          area  = do_QueryInterface(draggedNode);
          image = do_QueryInterface(draggedNode);
          link  = do_QueryInterface(draggedNode);
        }
      }
    }

    {
      // set for linked images, and links
      nsCOMPtr<nsIDOMNode> linkNode;

      if (area) {
        // use the alt text (or, if missing, the href) as the title
        area->GetAttribute(NS_LITERAL_STRING("alt"), mTitleString);
        if (mTitleString.IsEmpty()) {
          // this can be a relative link
          area->GetAttribute(NS_LITERAL_STRING("href"), mTitleString);
        }

        // we'll generate HTML like <a href="absurl">alt text</a>
        mIsAnchor = PR_TRUE;

        // gives an absolute link
        GetAnchorURL(area, mUrlString);

        mHtmlString.AssignLiteral("<a href=\"");
        mHtmlString.Append(mUrlString);
        mHtmlString.AppendLiteral("\">");
        mHtmlString.Append(mTitleString);
        mHtmlString.AppendLiteral("</a>");
      } else if (image) {
        mIsAnchor = PR_TRUE;
        // grab the href as the url, use alt text as the title of the
        // area if it's there.  the drag data is the image tag and src
        // attribute.
        nsCOMPtr<nsIURI> imageURI;
        image->GetCurrentURI(getter_AddRefs(imageURI));
        if (imageURI) {
          nsCAutoString spec;
          imageURI->GetSpec(spec);
          CopyUTF8toUTF16(spec, mUrlString);
        }

        nsCOMPtr<nsIDOMElement> imageElement(do_QueryInterface(image));
        // XXXbz Shouldn't we use the "title" attr for title?  Using
        // "alt" seems very wrong....
        if (imageElement) {
          imageElement->GetAttribute(NS_LITERAL_STRING("alt"), mTitleString);
        }

        if (mTitleString.IsEmpty()) {
          mTitleString = mUrlString;
        }

        nsCOMPtr<imgIRequest> imgRequest;

        // grab the image data, and its request.
        nsCOMPtr<nsIImage> img =
          nsContentUtils::GetImageFromContent(image,
                                              getter_AddRefs(imgRequest));

        nsCOMPtr<nsIMIMEService> mimeService =
          do_GetService("@mozilla.org/mime;1");

        // Fix the file extension in the URL if necessary
        if (imgRequest && mimeService) {
          nsCOMPtr<nsIURI> imgUri;
          imgRequest->GetURI(getter_AddRefs(imgUri));

          nsCOMPtr<nsIURL> imgUrl(do_QueryInterface(imgUri));

          if (imgUrl) {
            nsCAutoString extension;
            imgUrl->GetFileExtension(extension);

            nsXPIDLCString mimeType;
            imgRequest->GetMimeType(getter_Copies(mimeType));

            nsCOMPtr<nsIMIMEInfo> mimeInfo;
            mimeService->GetFromTypeAndExtension(mimeType, EmptyCString(),
                                                 getter_AddRefs(mimeInfo));

            if (mimeInfo) {
              nsCAutoString spec;
              imgUrl->GetSpec(spec);

              // pass out the image source string
              CopyUTF8toUTF16(spec, mImageSourceString);

              PRBool validExtension;
              if (extension.IsEmpty() || 
                  NS_FAILED(mimeInfo->ExtensionExists(extension,
                                                      &validExtension)) ||
                  !validExtension) {
                // Fix the file extension in the URL
                nsresult rv = imgUrl->Clone(getter_AddRefs(imgUri));
                NS_ENSURE_SUCCESS(rv, rv);

                imgUrl = do_QueryInterface(imgUri);

                nsCAutoString primaryExtension;
                mimeInfo->GetPrimaryExtension(primaryExtension);

                imgUrl->SetFileExtension(primaryExtension);
              }

              nsCAutoString fileName;
              imgUrl->GetFileName(fileName);

              NS_UnescapeURL(fileName);

              // make the filename safe for the filesystem
              fileName.ReplaceChar(FILE_PATH_SEPARATOR FILE_ILLEGAL_CHARACTERS,
                                   '-');

              CopyUTF8toUTF16(fileName, mImageDestFileName);

              // and the image object
              mImage = img;
            }
          }
        }

        if (parentLink) {
          // If we are dragging around an image in an anchor, then we
          // are dragging the entire anchor
          linkNode = parentLink;
          nodeToSerialize = linkNode;
        } else {
          nodeToSerialize = draggedNode;
        }
      } else if (link) {
        // set linkNode. The code below will handle this
        linkNode = link;    // XXX test this
        GetNodeString(draggedNode, mTitleString);
      } else if (parentLink) {
        // parentLink will always be null if there's selected content
        linkNode = parentLink;
        nodeToSerialize = linkNode;
      } else if (!haveSelectedContent) {
        // nothing draggable
        return NS_OK;
      }

      if (linkNode) {
        mIsAnchor = PR_TRUE;
        GetAnchorURL(linkNode, mUrlString);
      }
    }
  }

  if (nodeToSerialize || useSelectedText) {
    // if we have selected text, use it in preference to the node
    if (useSelectedText) {
      nodeToSerialize = nsnull;
    }

    SerializeNodeOrSelection(window, nodeToSerialize,
                             mHtmlString, mContextString, mInfoString);

    nsCOMPtr<nsIFormatConverter> htmlConverter =
      do_CreateInstance(kHTMLConverterCID);
    NS_ENSURE_TRUE(htmlConverter, NS_ERROR_FAILURE);

    nsCOMPtr<nsISupportsString> html =
      do_CreateInstance(NS_SUPPORTS_STRING_CONTRACTID);
    NS_ENSURE_TRUE(html, NS_ERROR_FAILURE);
    html->SetData(mHtmlString);

    nsCOMPtr<nsISupportsString> text;
    PRUint32 textLen;
    htmlConverter->Convert(kHTMLMime, html, mHtmlString.Length() * 2,
                           kUnicodeMime, getter_AddRefs(text), &textLen);
    NS_ENSURE_TRUE(text, NS_ERROR_FAILURE);
    text->GetData(mTitleString);

#ifdef CHANGE_SELECTION_ON_DRAG
    // We used to change the selection to wrap the dragged node (mainly
    // to work around now-fixed issues with dragging unselected elements).
    // There is no reason to do this any more.
    NormalizeSelection(selectionNormalizeNode, selection);
#endif
  }

  // default text value is the URL
  if (mTitleString.IsEmpty()) {
    mTitleString = mUrlString;
  }

  // if we haven't constructed a html version, make one now
  if (mHtmlString.IsEmpty() && !mUrlString.IsEmpty())
    CreateLinkText(mUrlString, mTitleString, mHtmlString);

  return ConvertStringsToTransferable(outTrans);
}

nsresult
nsTransferableFactory::ConvertStringsToTransferable(nsITransferable** outTrans)
{
  // now create the transferable and stuff data into it.
  nsCOMPtr<nsITransferable> trans =
    do_CreateInstance("@mozilla.org/widget/transferable;1");
  NS_ENSURE_TRUE(trans, NS_ERROR_FAILURE);

  // add a special flavor if we're an anchor to indicate that we have
  // a URL in the drag data
  if (!mUrlString.IsEmpty() && mIsAnchor) {
    nsAutoString dragData(mUrlString);
    dragData.AppendLiteral("\n");
    dragData += mTitleString;

    nsCOMPtr<nsISupportsString> urlPrimitive =
      do_CreateInstance(NS_SUPPORTS_STRING_CONTRACTID);
    NS_ENSURE_TRUE(urlPrimitive, NS_ERROR_FAILURE);

    urlPrimitive->SetData(dragData);
    trans->SetTransferData(kURLMime, urlPrimitive,
                           dragData.Length() * sizeof(PRUnichar));

    nsCOMPtr<nsISupportsString> urlDataPrimitive =
      do_CreateInstance(NS_SUPPORTS_STRING_CONTRACTID);
    NS_ENSURE_TRUE(urlDataPrimitive, NS_ERROR_FAILURE);

    urlDataPrimitive->SetData(mUrlString);
    trans->SetTransferData(kURLDataMime, urlDataPrimitive,
                           mUrlString.Length() * sizeof(PRUnichar));

    nsCOMPtr<nsISupportsString> urlDescPrimitive =
      do_CreateInstance(NS_SUPPORTS_STRING_CONTRACTID);
    NS_ENSURE_TRUE(urlDescPrimitive, NS_ERROR_FAILURE);

    urlDescPrimitive->SetData(mTitleString);
    trans->SetTransferData(kURLDescriptionMime, urlDescPrimitive,
                           mTitleString.Length() * sizeof(PRUnichar));
  }

  // add a special flavor, even if we don't have html context data
  nsCOMPtr<nsISupportsString> context =
    do_CreateInstance(NS_SUPPORTS_STRING_CONTRACTID);
  NS_ENSURE_TRUE(context, NS_ERROR_FAILURE);

  nsAutoString contextData(mContextString);
  context->SetData(contextData);
  trans->SetTransferData(kHTMLContext, context, contextData.Length() * 2);

  // add a special flavor if we have html info data
  if (!mInfoString.IsEmpty()) {
    nsCOMPtr<nsISupportsString> info =
      do_CreateInstance(NS_SUPPORTS_STRING_CONTRACTID);
    NS_ENSURE_TRUE(info, NS_ERROR_FAILURE);

    nsAutoString infoData(mInfoString);
    info->SetData(infoData);
    trans->SetTransferData(kHTMLInfo, info, infoData.Length() * 2);
  }

  // add the full html
  nsCOMPtr<nsISupportsString> htmlPrimitive =
    do_CreateInstance(NS_SUPPORTS_STRING_CONTRACTID);
  NS_ENSURE_TRUE(htmlPrimitive, NS_ERROR_FAILURE);

  htmlPrimitive->SetData(mHtmlString);
  trans->SetTransferData(kHTMLMime, htmlPrimitive,
                         mHtmlString.Length() * sizeof(PRUnichar));

  // add the plain (unicode) text. we use the url for text/unicode
  // data if an anchor is being dragged, rather than the title text of
  // the link or the alt text for an anchor image.
  nsCOMPtr<nsISupportsString> textPrimitive =
    do_CreateInstance(NS_SUPPORTS_STRING_CONTRACTID);
  NS_ENSURE_TRUE(textPrimitive, NS_ERROR_FAILURE);

  textPrimitive->SetData(mIsAnchor ? mUrlString : mTitleString);
  trans->SetTransferData(kUnicodeMime, textPrimitive,
                         (mIsAnchor ? mUrlString.Length() :
                          mTitleString.Length()) * sizeof(PRUnichar));

  // add image data, if present. For now, all we're going to do with
  // this is turn it into a native data flavor, so indicate that with
  // a new flavor so as not to confuse anyone who is really registered
  // for image/gif or image/jpg.
  if (mImage) {
    nsCOMPtr<nsISupportsInterfacePointer> ptrPrimitive =
      do_CreateInstance(NS_SUPPORTS_INTERFACE_POINTER_CONTRACTID);
    NS_ENSURE_TRUE(ptrPrimitive, NS_ERROR_FAILURE);

    ptrPrimitive->SetData(mImage);
    trans->SetTransferData(kNativeImageMime, ptrPrimitive,
                           sizeof(nsISupportsInterfacePointer*));
    // assume the image comes from a file, and add a file promise. We
    // register ourselves as a nsIFlavorDataProvider, and will use the
    // GetFlavorData callback to save the image to disk.
    trans->SetTransferData(kFilePromiseMime, mFlavorDataProvider,
                           nsITransferable::kFlavorHasDataProvider);

    nsCOMPtr<nsISupportsString> imageUrlPrimitive =
      do_CreateInstance(NS_SUPPORTS_STRING_CONTRACTID);
    NS_ENSURE_TRUE(imageUrlPrimitive, NS_ERROR_FAILURE);

    imageUrlPrimitive->SetData(mImageSourceString);
    trans->SetTransferData(kFilePromiseURLMime, imageUrlPrimitive,
                           mImageSourceString.Length() * sizeof(PRUnichar));

    nsCOMPtr<nsISupportsString> imageFileNamePrimitive =
      do_CreateInstance(NS_SUPPORTS_STRING_CONTRACTID);
    NS_ENSURE_TRUE(imageFileNamePrimitive, NS_ERROR_FAILURE);

    imageFileNamePrimitive->SetData(mImageDestFileName);
    trans->SetTransferData(kFilePromiseDestFilename, imageFileNamePrimitive,
                           mImageDestFileName.Length() * sizeof(PRUnichar));

    // if not an anchor, add the image url
    if (!mIsAnchor) {
      nsCOMPtr<nsISupportsString> urlDataPrimitive =
        do_CreateInstance(NS_SUPPORTS_STRING_CONTRACTID);
      NS_ENSURE_TRUE(urlDataPrimitive, NS_ERROR_FAILURE);

      urlDataPrimitive->SetData(mUrlString);
      trans->SetTransferData(kURLDataMime, urlDataPrimitive,
                             mUrlString.Length() * sizeof(PRUnichar));
    }
  }

  *outTrans = trans;
  NS_IF_ADDREF(*outTrans);

  return NS_OK;
}

// note that this can return NS_OK, but a null out param (by design)
// static
nsresult
nsTransferableFactory::GetDraggableSelectionData(nsISelection* inSelection,
                                                 nsIDOMNode* inRealTargetNode,
                                                 nsIDOMNode **outImageOrLinkNode,
                                                 PRBool* outDragSelectedText)
{
  NS_ENSURE_ARG(inSelection);
  NS_ENSURE_ARG(inRealTargetNode);
  NS_ENSURE_ARG_POINTER(outImageOrLinkNode);

  *outImageOrLinkNode = nsnull;
  *outDragSelectedText = PR_FALSE;

  PRBool selectionContainsTarget = PR_FALSE;

  PRBool isCollapsed = PR_FALSE;
  inSelection->GetIsCollapsed(&isCollapsed);
  if (!isCollapsed) {
    inSelection->ContainsNode(inRealTargetNode, PR_FALSE,
                              &selectionContainsTarget);

    if (selectionContainsTarget) {
      // track down the anchor node, if any, for the url
      nsCOMPtr<nsIDOMNode> selectionStart;
      inSelection->GetAnchorNode(getter_AddRefs(selectionStart));

      nsCOMPtr<nsIDOMNode> selectionEnd;
      inSelection->GetFocusNode(getter_AddRefs(selectionEnd));

      // look for a selection around a single node, like an image.
      // in this case, drag the image, rather than a serialization of the HTML
      // XXX generalize this to other draggable element types?
      if (selectionStart == selectionEnd) {
        PRBool hasChildren;
        selectionStart->HasChildNodes(&hasChildren);
        if (hasChildren) {
          // see if just one node is selected
          PRInt32 anchorOffset, focusOffset;
          inSelection->GetAnchorOffset(&anchorOffset);
          inSelection->GetFocusOffset(&focusOffset);
          if (abs(anchorOffset - focusOffset) == 1) {
            nsCOMPtr<nsIContent> selStartContent =
              do_QueryInterface(selectionStart);

            if (selStartContent) {
              PRInt32 childOffset =
                (anchorOffset < focusOffset) ? anchorOffset : focusOffset;
              nsIContent *childContent =
                selStartContent->GetChildAt(childOffset);
              // if we find an image, we'll fall into the node-dragging code,
              // rather the the selection-dragging code
              if (nsContentUtils::IsDraggableImage(childContent)) {
                CallQueryInterface(childContent, outImageOrLinkNode);

                return NS_OK;
              }
            }
          }
        }
      }

      // see if the selection is a link;  if so, its node will be returned
      GetSelectedLink(inSelection, outImageOrLinkNode);

      // indicate that a link or text is selected
      *outDragSelectedText = PR_TRUE;
    }
  }

  return NS_OK;
}

// static
void nsTransferableFactory::GetSelectedLink(nsISelection* inSelection,
                                            nsIDOMNode **outLinkNode)
{
  *outLinkNode = nsnull;

  nsCOMPtr<nsIDOMNode> selectionStart;
  inSelection->GetAnchorNode(getter_AddRefs(selectionStart));
  nsCOMPtr<nsIDOMNode> selectionEnd;
  inSelection->GetFocusNode(getter_AddRefs(selectionEnd));

  // simple case:  only one node is selected
  // see if it or its parent is an anchor, then exit

  if (selectionStart == selectionEnd) {
    nsCOMPtr<nsIDOMNode> link = FindParentLinkNode(selectionStart);
    if (link) {
      link.swap(*outLinkNode);
    }

    return;
  }

  // more complicated case:  multiple nodes are selected

  // Unless you use the Alt key while selecting anchor text, it is
  // nearly impossible to avoid overlapping into adjacent nodes.
  // Deal with this by trimming off the leading and/or trailing
  // nodes of the selection if the strings they produce are empty.

  // first, use a range determine if the selection was marked LTR or RTL;
  // if the latter, swap endpoints so we trim in the right direction

  PRInt32 startOffset, endOffset;
  {
    nsCOMPtr<nsIDOMRange> range;
    inSelection->GetRangeAt(0, getter_AddRefs(range));
    if (!range) {
      return;
    }

    nsCOMPtr<nsIDOMNode> tempNode;
    range->GetStartContainer( getter_AddRefs(tempNode));
    if (tempNode != selectionStart) {
      selectionEnd = selectionStart;
      selectionStart = tempNode;
      inSelection->GetAnchorOffset(&endOffset);
      inSelection->GetFocusOffset(&startOffset);
    } else {
      inSelection->GetAnchorOffset(&startOffset);
      inSelection->GetFocusOffset(&endOffset);
    }
  }

  // trim leading node if the string is empty or
  // the selection starts at the end of the text

  nsAutoString nodeStr;
  selectionStart->GetNodeValue(nodeStr);
  if (nodeStr.IsEmpty() ||
      startOffset+1 >= static_cast<PRInt32>(nodeStr.Length())) {
    nsCOMPtr<nsIDOMNode> curr = selectionStart;
    nsIDOMNode* next;

    while (curr) {
      curr->GetNextSibling(&next);

      if (next) {
        selectionStart = dont_AddRef(next);
        break;
      }

      curr->GetParentNode(&next);
      curr = dont_AddRef(next);
    }
  }

  // trim trailing node if the selection ends before its text begins

  if (endOffset == 0) {
    nsCOMPtr<nsIDOMNode> curr = selectionEnd;
    nsIDOMNode* next;

    while (curr) {
      curr->GetPreviousSibling(&next);

      if (next){
        selectionEnd = dont_AddRef(next);
        break;
      }

      curr->GetParentNode(&next);
      curr = dont_AddRef(next);
    }
  }

  // see if the leading & trailing nodes are part of the
  // same anchor - if so, return the anchor node
  nsCOMPtr<nsIDOMNode> link = FindParentLinkNode(selectionStart);
  if (link) {
    nsCOMPtr<nsIDOMNode> link2 = FindParentLinkNode(selectionEnd);

    if (link == link2) {
      NS_IF_ADDREF(*outLinkNode = link);
    }
  }

  return;
}

// static
nsresult
nsTransferableFactory::SerializeNodeOrSelection(nsIDOMWindow* inWindow,
                                                nsIDOMNode* inNode,
                                                nsAString& outResultString,
                                                nsAString& outContext,
                                                nsAString& outInfo)
{
  NS_ENSURE_ARG_POINTER(inWindow);

  nsresult rv;
  nsCOMPtr<nsIDocumentEncoder> encoder =
    do_CreateInstance(NS_HTMLCOPY_ENCODER_CONTRACTID);
  NS_ENSURE_TRUE(encoder, NS_ERROR_FAILURE);

  nsCOMPtr<nsIDOMDocument> domDoc;
  inWindow->GetDocument(getter_AddRefs(domDoc));
  NS_ENSURE_TRUE(domDoc, NS_ERROR_FAILURE);

  PRUint32 flags = nsIDocumentEncoder::OutputAbsoluteLinks |
                   nsIDocumentEncoder::OutputEncodeW3CEntities;
  nsCOMPtr<nsIDOMRange> range;
  nsCOMPtr<nsISelection> selection;
  if (inNode) {
    // make a range around this node
    rv = NS_NewRange(getter_AddRefs(range));
    NS_ENSURE_SUCCESS(rv, rv);
    rv = range->SelectNode(inNode);
    NS_ENSURE_SUCCESS(rv, rv);
  } else {
    inWindow->GetSelection(getter_AddRefs(selection));
    flags |= nsIDocumentEncoder::OutputSelectionOnly;
  }

  rv = encoder->Init(domDoc, NS_LITERAL_STRING(kHTMLMime), flags);
  NS_ENSURE_SUCCESS(rv, rv);

  if (range) {
    encoder->SetRange(range);
  } else if (selection) {
    encoder->SetSelection(selection);
  }

  return encoder->EncodeToStringWithContext(outContext, outInfo,
                                            outResultString);
}
