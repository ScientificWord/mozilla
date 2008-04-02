/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 ci et: */
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
 * The Original Code is the Mozilla browser.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications, Inc.
 * Portions created by the Initial Developer are Copyright (C) 1999
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Travis Bogard <travis@netscape.com>
 *   Dan Rosen <dr@netscape.com>
 *   Ben Goodger <ben@netscape.com>
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

// Local includes
#include "nsXULWindow.h"

// Helper classes
#include "nsString.h"
#include "nsWidgetsCID.h"
#include "prprf.h"
#include "nsCRT.h"
#include "nsThreadUtils.h"
#include "nsNetCID.h"

//Interfaces needed to be included
#include "nsIAppShell.h"
#include "nsIAppShellService.h"
#include "nsIServiceManager.h"
#include "nsIDocumentViewer.h"
#include "nsIDocument.h"
#include "nsIDOMBarProp.h"
#include "nsIDOMDocument.h"
#include "nsIDOMDocumentEvent.h"
#include "nsIDOMXULDocument.h"
#include "nsIDOMElement.h"
#include "nsIPrivateDOMEvent.h"
#include "nsIDOMEventTarget.h"
#include "nsIDOMXULElement.h"
#include "nsPIDOMWindow.h"
#include "nsIDOMScreen.h"
#include "nsIEmbeddingSiteWindow.h"
#include "nsIEmbeddingSiteWindow2.h"
#include "nsIInterfaceRequestor.h"
#include "nsIInterfaceRequestorUtils.h"
#include "nsIIOService.h"
#include "nsIJSContextStack.h"
#include "nsIMarkupDocumentViewer.h"
#include "nsIObserverService.h"
#include "nsIWindowMediator.h"
#include "nsIScreenManager.h"
#include "nsIScreen.h"
#include "nsIScrollable.h"
#include "nsIPrefService.h"
#include "nsIPrefBranch.h"
#include "nsIScriptSecurityManager.h"
#include "nsIWindowWatcher.h"
#include "nsIURI.h"
#include "nsIDOMDocumentView.h"
#include "nsIDOMViewCSS.h"
#include "nsIDOMCSSStyleDeclaration.h"
#include "nsITimelineService.h"
#include "nsAppShellCID.h"
#include "nsReadableUtils.h"
#include "nsStyleConsts.h"

#include "nsWebShellWindow.h" // get rid of this one, too...

#define SIZEMODE_NORMAL    NS_LITERAL_STRING("normal")
#define SIZEMODE_MAXIMIZED NS_LITERAL_STRING("maximized")
#define SIZEMODE_MINIMIZED NS_LITERAL_STRING("minimized")

#define WINDOWTYPE_ATTRIBUTE NS_LITERAL_STRING("windowtype")

#define PERSIST_ATTRIBUTE  NS_LITERAL_STRING("persist")
#define SCREENX_ATTRIBUTE  NS_LITERAL_STRING("screenX")
#define SCREENY_ATTRIBUTE  NS_LITERAL_STRING("screenY")
#define WIDTH_ATTRIBUTE    NS_LITERAL_STRING("width")
#define HEIGHT_ATTRIBUTE   NS_LITERAL_STRING("height")
#define MODE_ATTRIBUTE     NS_LITERAL_STRING("sizemode")
#define ZLEVEL_ATTRIBUTE   NS_LITERAL_STRING("zlevel")

//*****************************************************************************
//***    nsXULWindow: Object Management
//*****************************************************************************

nsXULWindow::nsXULWindow()
  : mChromeTreeOwner(nsnull), 
    mContentTreeOwner(nsnull),
    mPrimaryContentTreeOwner(nsnull),
    mModalStatus(NS_OK),
    mContinueModalLoop(PR_FALSE),
    mDebuting(PR_FALSE),
    mChromeLoaded(PR_FALSE), 
    mShowAfterLoad(PR_FALSE),
    mIntrinsicallySized(PR_FALSE),
    mCenterAfterLoad(PR_FALSE),
    mIsHiddenWindow(PR_FALSE),
    mLockedUntilChromeLoad(PR_FALSE),
    mContextFlags(0),
    mBlurSuppressionLevel(0),
    mPersistentAttributesDirty(0),
    mPersistentAttributesMask(0),
    mChromeFlags(nsIWebBrowserChrome::CHROME_ALL)
{
}

nsXULWindow::~nsXULWindow()
{
  Destroy();
}

//*****************************************************************************
// nsXULWindow::nsISupports
//*****************************************************************************

NS_IMPL_THREADSAFE_ADDREF(nsXULWindow)
NS_IMPL_THREADSAFE_RELEASE(nsXULWindow)

NS_INTERFACE_MAP_BEGIN(nsXULWindow)
  NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(nsISupports, nsIXULWindow)
  NS_INTERFACE_MAP_ENTRY(nsIXULWindow)
  NS_INTERFACE_MAP_ENTRY(nsIBaseWindow)
  NS_INTERFACE_MAP_ENTRY(nsIInterfaceRequestor)
  NS_INTERFACE_MAP_ENTRY(nsISupportsWeakReference)
  if (aIID.Equals(NS_GET_IID(nsXULWindow)))
    foundInterface = reinterpret_cast<nsISupports*>(this);
  else
NS_INTERFACE_MAP_END

//*****************************************************************************
// nsXULWindow::nsIIntefaceRequestor
//*****************************************************************************   

NS_IMETHODIMP nsXULWindow::GetInterface(const nsIID& aIID, void** aSink)
{
  nsresult rv;

  NS_ENSURE_ARG_POINTER(aSink);

  if (aIID.Equals(NS_GET_IID(nsIPrompt))) {
    rv = EnsurePrompter();
    if (NS_FAILED(rv)) return rv;
    return mPrompter->QueryInterface(aIID, aSink);
  }   
  if (aIID.Equals(NS_GET_IID(nsIAuthPrompt))) {
    rv = EnsureAuthPrompter();
    if (NS_FAILED(rv)) return rv;
    return mAuthPrompter->QueryInterface(aIID, aSink);
  }
  if (aIID.Equals(NS_GET_IID(nsIWebBrowserChrome)) && 
    NS_SUCCEEDED(EnsureContentTreeOwner()) &&
    NS_SUCCEEDED(mContentTreeOwner->QueryInterface(aIID, aSink)))
    return NS_OK;

  if (aIID.Equals(NS_GET_IID(nsIEmbeddingSiteWindow)) && 
    NS_SUCCEEDED(EnsureContentTreeOwner()) &&
    NS_SUCCEEDED(mContentTreeOwner->QueryInterface(aIID, aSink)))
    return NS_OK;
  if (aIID.Equals(NS_GET_IID(nsIEmbeddingSiteWindow2)) && 
    NS_SUCCEEDED(EnsureContentTreeOwner()) &&
    NS_SUCCEEDED(mContentTreeOwner->QueryInterface(aIID, aSink)))
    return NS_OK;

  return QueryInterface(aIID, aSink);
}

//*****************************************************************************
// nsXULWindow::nsIXULWindow
//*****************************************************************************   

NS_IMETHODIMP nsXULWindow::GetDocShell(nsIDocShell** aDocShell)
{
  NS_ENSURE_ARG_POINTER(aDocShell);

  *aDocShell = mDocShell;
  NS_IF_ADDREF(*aDocShell);
  return NS_OK;
}

NS_IMETHODIMP nsXULWindow::GetZLevel(PRUint32 *outLevel)
{
  nsCOMPtr<nsIWindowMediator> mediator(do_GetService(NS_WINDOWMEDIATOR_CONTRACTID));
  if (mediator)
    mediator->GetZLevel(this, outLevel);
  else
    *outLevel = normalZ;
  return NS_OK;
}

NS_IMETHODIMP nsXULWindow::SetZLevel(PRUint32 aLevel)
{
  nsCOMPtr<nsIWindowMediator> mediator(do_GetService(NS_WINDOWMEDIATOR_CONTRACTID));
  if (!mediator)
    return NS_ERROR_FAILURE;

  PRUint32 zLevel;
  mediator->GetZLevel(this, &zLevel);
  if (zLevel == aLevel)
    return NS_OK;

  /* refuse to raise a maximized window above the normal browser level,
     for fear it could hide newly opened browser windows */
  if (aLevel > nsIXULWindow::normalZ) {
    PRInt32 sizeMode;
    if (mWindow) {
      mWindow->GetSizeMode(&sizeMode);
      if (sizeMode == nsSizeMode_Maximized)
        return NS_ERROR_FAILURE;
    }
  }

  // disallow user script
  nsCOMPtr<nsIScriptSecurityManager> secMan =
           do_GetService(NS_SCRIPTSECURITYMANAGER_CONTRACTID);
  if (!secMan)
    return NS_ERROR_FAILURE;
  PRBool inChrome;
  if (NS_FAILED(secMan->SubjectPrincipalIsSystem(&inChrome)) || !inChrome)
    return NS_ERROR_FAILURE;

  // do it
  mediator->SetZLevel(this, aLevel);
  PersistentAttributesDirty(PAD_MISC);
  SavePersistentAttributes();

  // finally, send a notification DOM event
  nsCOMPtr<nsIContentViewer> cv;
  mDocShell->GetContentViewer(getter_AddRefs(cv));
  nsCOMPtr<nsIDocumentViewer> dv(do_QueryInterface(cv));
  if (dv) {
    nsCOMPtr<nsIDocument> doc;
    dv->GetDocument(getter_AddRefs(doc));
    nsCOMPtr<nsIDOMDocumentEvent> docEvent(do_QueryInterface(doc));
    if (docEvent) {
      nsCOMPtr<nsIDOMEvent> event;
      docEvent->CreateEvent(NS_LITERAL_STRING("Events"), getter_AddRefs(event));
      if (event) {
        event->InitEvent(NS_LITERAL_STRING("windowZLevel"), PR_TRUE, PR_FALSE);

        nsCOMPtr<nsIPrivateDOMEvent> privateEvent(do_QueryInterface(event));
        privateEvent->SetTrusted(PR_TRUE);

        nsCOMPtr<nsIDOMEventTarget> targ(do_QueryInterface(doc));
        if (targ) {
          PRBool defaultActionEnabled;
          targ->DispatchEvent(event, &defaultActionEnabled);
        }
      }
    }
  }
  
  return NS_OK;
}

NS_IMETHODIMP nsXULWindow::GetContextFlags(PRUint32 *aContextFlags)
{
  NS_ENSURE_ARG_POINTER(aContextFlags);
  *aContextFlags = mContextFlags;
  return NS_OK;
}

NS_IMETHODIMP nsXULWindow::SetContextFlags(PRUint32 aContextFlags)
{
  mContextFlags = aContextFlags;
  return NS_OK;
}

NS_IMETHODIMP nsXULWindow::GetChromeFlags(PRUint32 *aChromeFlags)
{
  NS_ENSURE_ARG_POINTER(aChromeFlags);
  *aChromeFlags = mChromeFlags;
  /* mChromeFlags is kept up to date, except for scrollbar visibility.
     That can be changed directly by the content DOM window, which
     doesn't know to update the chrome window. So that we must check
     separately. */

  // however, it's pointless to ask if the window isn't set up yet
  if (!mChromeLoaded)
    return NS_OK;

  if (GetContentScrollbarVisibility())
    *aChromeFlags |= nsIWebBrowserChrome::CHROME_SCROLLBARS;
  else
    *aChromeFlags &= ~nsIWebBrowserChrome::CHROME_SCROLLBARS;

  return NS_OK;
}

NS_IMETHODIMP nsXULWindow::SetChromeFlags(PRUint32 aChromeFlags)
{
  mChromeFlags = aChromeFlags;
  if (mChromeLoaded)
    NS_ENSURE_SUCCESS(ApplyChromeFlags(), NS_ERROR_FAILURE);
  return NS_OK;
}

NS_IMETHODIMP nsXULWindow::SetIntrinsicallySized(PRBool aIntrinsicallySized)
{
  mIntrinsicallySized = aIntrinsicallySized;
  return NS_OK;
}

NS_IMETHODIMP nsXULWindow::GetIntrinsicallySized(PRBool* aIntrinsicallySized)
{
  NS_ENSURE_ARG_POINTER(aIntrinsicallySized);

  *aIntrinsicallySized = mIntrinsicallySized;
  return NS_OK;
}

NS_IMETHODIMP nsXULWindow::GetPrimaryContentShell(nsIDocShellTreeItem** 
   aDocShellTreeItem)
{
  NS_ENSURE_ARG_POINTER(aDocShellTreeItem);
  NS_IF_ADDREF(*aDocShellTreeItem = mPrimaryContentShell);
  return NS_OK;
}

NS_IMETHODIMP nsXULWindow::GetContentShellById(const PRUnichar* aID, 
   nsIDocShellTreeItem** aDocShellTreeItem)
{
  NS_ENSURE_ARG_POINTER(aDocShellTreeItem);
  *aDocShellTreeItem = nsnull;

  PRInt32 count = mContentShells.Count();
  for (PRInt32 i = 0; i < count; i++) {
    nsContentShellInfo* shellInfo = (nsContentShellInfo*)mContentShells.ElementAt(i);
    if (shellInfo->id.Equals(aID)) {
      *aDocShellTreeItem = nsnull;
      if (shellInfo->child)
        CallQueryReferent(shellInfo->child.get(), aDocShellTreeItem);
      return NS_OK;
    }
  }
  return NS_ERROR_FAILURE;
}

NS_IMETHODIMP nsXULWindow::AddChildWindow(nsIXULWindow *aChild)
{
  // we're not really keeping track of this right now
  return NS_OK;
}

NS_IMETHODIMP nsXULWindow::RemoveChildWindow(nsIXULWindow *aChild)
{
  // we're not really keeping track of this right now
  return NS_OK;
}

NS_IMETHODIMP nsXULWindow::ShowModal()
{
  // Store locally so it doesn't die on us
  nsCOMPtr<nsIWidget> window = mWindow;
  nsCOMPtr<nsIXULWindow> tempRef = this;  

  window->SetModal(PR_TRUE);
  mContinueModalLoop = PR_TRUE;
  EnableParent(PR_FALSE);

  nsCOMPtr<nsIAppShellService> appShellService(do_GetService(NS_APPSHELLSERVICE_CONTRACTID));
  if (appShellService)
      appShellService->TopLevelWindowIsModal(
                         static_cast<nsIXULWindow*>(this), PR_TRUE);

  nsCOMPtr<nsIJSContextStack> stack(do_GetService("@mozilla.org/js/xpc/ContextStack;1"));
  if (stack && NS_SUCCEEDED(stack->Push(nsnull))) {
    nsIThread *thread = NS_GetCurrentThread();
    while (mContinueModalLoop) {
      if (!NS_ProcessNextEvent(thread))
        break;
    }
    JSContext* cx;
    stack->Pop(&cx);
    NS_ASSERTION(cx == nsnull, "JSContextStack mismatch");
  }

  mContinueModalLoop = PR_FALSE;
  window->SetModal(PR_FALSE);
  if (appShellService)
      appShellService->TopLevelWindowIsModal(
                         static_cast<nsIXULWindow*>(this), PR_FALSE);
  /*   Note there's no EnableParent(PR_TRUE) here to match the PR_FALSE one
     above. That's done in ExitModalLoop. It's important that the parent
     be re-enabled before this window is made invisible; to do otherwise
     causes bizarre z-ordering problems. At this point, the window is
     already invisible.
       No known current implementation of Enable would have a problem with
     re-enabling the parent twice, so we could do it again here without
     breaking any current implementation. But that's unnecessary if the
     modal loop is always exited using ExitModalLoop (the other way would be
     to change the protected member variable directly.)
  */

  return mModalStatus;
}

//*****************************************************************************
// nsXULWindow::nsIBaseWindow
//*****************************************************************************   

NS_IMETHODIMP nsXULWindow::InitWindow(nativeWindow aParentNativeWindow,
   nsIWidget* parentWidget, PRInt32 x, PRInt32 y, PRInt32 cx, PRInt32 cy)   
{
  //XXX First Check In
  NS_ASSERTION(PR_FALSE, "Not Yet Implemented");
  return NS_OK;
}

NS_IMETHODIMP nsXULWindow::Create()
{
  //XXX First Check In
  NS_ASSERTION(PR_FALSE, "Not Yet Implemented");
  return NS_OK;
}

NS_IMETHODIMP nsXULWindow::Destroy()
{
  if(!mWindow)
     return NS_OK;

  nsCOMPtr<nsIAppShellService> appShell(do_GetService(NS_APPSHELLSERVICE_CONTRACTID));
  NS_ASSERTION(appShell, "Couldn't get appShell... xpcom shutdown?");
  if (appShell)
    appShell->UnregisterTopLevelWindow(static_cast<nsIXULWindow*>(this));

  nsCOMPtr<nsIXULWindow> parentWindow(do_QueryReferent(mParentWindow));
  if (parentWindow)
    parentWindow->RemoveChildWindow(this);

  // let's make sure the window doesn't get deleted out from under us
  // while we are trying to close....this can happen if the docshell
  // we close ends up being the last owning reference to this xulwindow

  // XXXTAB This shouldn't be an issue anymore because the ownership model
  // only goes in one direction.  When webshell container is fully removed
  // try removing this...

  nsCOMPtr<nsIXULWindow> placeHolder = this;

  // Remove modality (if any) and hide while destroying. More than
  // a convenience, the hide prevents user interaction with the partially
  // destroyed window. This is especially necessary when the eldest window
  // in a stack of modal windows is destroyed first. It happens.
  ExitModalLoop(NS_OK);
  if (mWindow)
    mWindow->Show(PR_FALSE);

#if defined(XP_WIN) || defined(XP_OS2)
  // We need to explicitly set the focus on Windows
  nsCOMPtr<nsIBaseWindow> parent(do_QueryReferent(mParentWindow));
  if (parent) {
    nsCOMPtr<nsIBaseWindow> baseHiddenWindow;
    if (appShell) {
      nsCOMPtr<nsIXULWindow> hiddenWindow;
      appShell->GetHiddenWindow(getter_AddRefs(hiddenWindow));
      if (hiddenWindow)
        baseHiddenWindow = do_GetInterface(hiddenWindow);
    }
    // somebody screwed up somewhere. hiddenwindow shouldn't be anybody's
    // parent. still, when it happens, skip activating it.
    if (baseHiddenWindow != parent) {
      nsCOMPtr<nsIWidget> parentWidget;
      parent->GetMainWidget(getter_AddRefs(parentWidget));
      if (parentWidget)
        parentWidget->PlaceBehind(eZPlacementTop, 0, PR_TRUE);
    }
  }
#endif
   
  mDOMWindow = nsnull;
  if (mDocShell) {
    nsCOMPtr<nsIBaseWindow> shellAsWin(do_QueryInterface(mDocShell));
    shellAsWin->Destroy();
    mDocShell = nsnull; // this can cause reentrancy of this function
  }

  // Remove our ref on the content shells
  PRInt32 count;
  count = mContentShells.Count();
  for (PRInt32 i = 0; i < count; i++) {
    nsContentShellInfo* shellInfo =
        static_cast<nsContentShellInfo *>(mContentShells.ElementAt(i));
    delete shellInfo;
  }
  mContentShells.Clear();
  mPrimaryContentShell = nsnull;

  if(mContentTreeOwner) {
    mContentTreeOwner->XULWindow(nsnull);
    NS_RELEASE(mContentTreeOwner);
  }
  if(mPrimaryContentTreeOwner) {
    mPrimaryContentTreeOwner->XULWindow(nsnull);
    NS_RELEASE(mPrimaryContentTreeOwner);
  }
  if(mChromeTreeOwner) {
    mChromeTreeOwner->XULWindow(nsnull);
    NS_RELEASE(mChromeTreeOwner);
  }
  if(mWindow) {
    mWindow->SetClientData(0); // nsWebShellWindow hackery
    mWindow = nsnull;
  }

  if (!mIsHiddenWindow) {
    /* Inform appstartup we've destroyed this window and it could
       quit now if it wanted. This must happen at least after mDocShell
       is destroyed, because onunload handlers fire then, and those being
       script, anything could happen. A new window could open, even.
       See bug 130719. */
    nsCOMPtr<nsIObserverService> obssvc =
        do_GetService("@mozilla.org/observer-service;1");
    NS_ASSERTION(obssvc, "Couldn't get observer service?");

    if (obssvc)
      obssvc->NotifyObservers(nsnull, "xul-window-destroyed", nsnull);
  }

  return NS_OK;
}

NS_IMETHODIMP nsXULWindow::SetPosition(PRInt32 aX, PRInt32 aY)
{
  /* any attempt to set the window's size or position overrides the window's
     zoom state. this is important when these two states are competing while
     the window is being opened. but it should probably just always be so. */
  mWindow->SetSizeMode(nsSizeMode_Normal);
    
  NS_ENSURE_SUCCESS(mWindow->Move(aX, aY), NS_ERROR_FAILURE);
  PersistentAttributesDirty(PAD_POSITION);
  SavePersistentAttributes();
  return NS_OK;
}

NS_IMETHODIMP nsXULWindow::GetPosition(PRInt32* aX, PRInt32* aY)
{
  return GetPositionAndSize(aX, aY, nsnull, nsnull);
}

NS_IMETHODIMP nsXULWindow::SetSize(PRInt32 aCX, PRInt32 aCY, PRBool aRepaint)
{
  /* any attempt to set the window's size or position overrides the window's
     zoom state. this is important when these two states are competing while
     the window is being opened. but it should probably just always be so. */
  mWindow->SetSizeMode(nsSizeMode_Normal);

  mIntrinsicallySized = PR_FALSE;

  NS_ENSURE_SUCCESS(mWindow->Resize(aCX, aCY, aRepaint), NS_ERROR_FAILURE);
  PersistentAttributesDirty(PAD_SIZE);
  SavePersistentAttributes();
  return NS_OK;
}

NS_IMETHODIMP nsXULWindow::GetSize(PRInt32* aCX, PRInt32* aCY)
{
  return GetPositionAndSize(nsnull, nsnull, aCX, aCY);
}

NS_IMETHODIMP nsXULWindow::SetPositionAndSize(PRInt32 aX, PRInt32 aY, 
   PRInt32 aCX, PRInt32 aCY, PRBool aRepaint)
{
  /* any attempt to set the window's size or position overrides the window's
     zoom state. this is important when these two states are competing while
     the window is being opened. but it should probably just always be so. */
  mWindow->SetSizeMode(nsSizeMode_Normal);

  mIntrinsicallySized = PR_FALSE;

  NS_ENSURE_SUCCESS(mWindow->Resize(aX, aY, aCX, aCY, aRepaint), NS_ERROR_FAILURE);
  PersistentAttributesDirty(PAD_POSITION | PAD_SIZE);
  SavePersistentAttributes();
  return NS_OK;
}

NS_IMETHODIMP nsXULWindow::GetPositionAndSize(PRInt32* x, PRInt32* y, PRInt32* cx,
   PRInt32* cy)
{
  nsRect rect;

  if (!mWindow)
    return NS_ERROR_FAILURE;

  mWindow->GetScreenBounds(rect);

  if(x)
    *x = rect.x;
  if(y)
    *y = rect.y;
  if(cx)
    *cx = rect.width;
  if(cy)
    *cy = rect.height;

  return NS_OK;
}

NS_IMETHODIMP nsXULWindow::Center(nsIXULWindow *aRelative, PRBool aScreen, PRBool aAlert) {

  PRInt32  left, top, width, height,
           ourWidth, ourHeight;
  PRBool   screenCoordinates =  PR_FALSE,
           windowCoordinates =  PR_FALSE;
  nsresult result;

  if (!mChromeLoaded) {
    // note we lose the parameters. at time of writing, this isn't a problem.
    mCenterAfterLoad = PR_TRUE;
    return NS_OK;
  }

  if (!aScreen && !aRelative)
    return NS_ERROR_INVALID_ARG;

  nsCOMPtr<nsIScreenManager> screenmgr = do_GetService("@mozilla.org/gfx/screenmanager;1", &result);
  if (NS_FAILED(result))
    return result;

  nsCOMPtr<nsIScreen> screen;

  if (aRelative) {
    nsCOMPtr<nsIBaseWindow> base(do_QueryInterface(aRelative, &result));
    if (base) {
      // get window rect
      result = base->GetPositionAndSize(&left, &top, &width, &height);
      if (NS_SUCCEEDED(result)) {
        // if centering on screen, convert that to the corresponding screen
        if (aScreen)
          screenmgr->ScreenForRect(left, top, width, height, getter_AddRefs(screen));
        else
          windowCoordinates = PR_TRUE;
      } else {
        // something's wrong with the reference window.
        // fall back to the primary screen
        aRelative = 0;
        aScreen = PR_TRUE;
      }
    }
  }
  if (!aRelative)
    screenmgr->GetPrimaryScreen(getter_AddRefs(screen));

  if (aScreen && screen) {
    screen->GetAvailRect(&left, &top, &width, &height);
    screenCoordinates = PR_TRUE;
  }

  if (screenCoordinates || windowCoordinates) {
    GetSize(&ourWidth, &ourHeight);
    left += (width - ourWidth) / 2;
    top += (height - ourHeight) / (aAlert ? 3 : 2);
    if (windowCoordinates)
      mWindow->ConstrainPosition(PR_FALSE, &left, &top);
    SetPosition(left, top);
    return NS_OK;
  }
  return NS_ERROR_FAILURE;
}

NS_IMETHODIMP nsXULWindow::Repaint(PRBool aForce)
{
  //XXX First Check In
  NS_ASSERTION(PR_FALSE, "Not Yet Implemented");
  return NS_OK;
}

NS_IMETHODIMP nsXULWindow::GetParentWidget(nsIWidget** aParentWidget)
{
  NS_ENSURE_ARG_POINTER(aParentWidget);
  NS_ENSURE_STATE(mWindow);

  NS_IF_ADDREF(*aParentWidget = mWindow->GetParent());
  return NS_OK;
}

NS_IMETHODIMP nsXULWindow::SetParentWidget(nsIWidget* aParentWidget)
{
  //XXX First Check In
  NS_ASSERTION(PR_FALSE, "Not Yet Implemented");
  return NS_OK;
}

NS_IMETHODIMP nsXULWindow::GetParentNativeWindow(nativeWindow* aParentNativeWindow)
{
  NS_ENSURE_ARG_POINTER(aParentNativeWindow);

  nsCOMPtr<nsIWidget> parentWidget;
  NS_ENSURE_SUCCESS(GetParentWidget(getter_AddRefs(parentWidget)), NS_ERROR_FAILURE);

  *aParentNativeWindow = parentWidget->GetNativeData(NS_NATIVE_WIDGET);
   
  return NS_OK;
}

NS_IMETHODIMP nsXULWindow::SetParentNativeWindow(nativeWindow aParentNativeWindow)
{
  //XXX First Check In
  NS_ASSERTION(PR_FALSE, "Not Yet Implemented");
  return NS_OK;
}

NS_IMETHODIMP nsXULWindow::GetVisibility(PRBool* aVisibility)
{
  NS_ENSURE_ARG_POINTER(aVisibility);

  // Always claim to be visible for now. See bug
  // https://bugzilla.mozilla.org/show_bug.cgi?id=306245.

  *aVisibility = PR_TRUE;

  return NS_OK;
}

NS_IMETHODIMP nsXULWindow::SetVisibility(PRBool aVisibility)
{
  NS_TIMELINE_ENTER("nsXULWindow::SetVisibility.");
  if (!mChromeLoaded) {
    mShowAfterLoad = aVisibility;
    NS_TIMELINE_LEAVE("nsXULWindow::SetVisibility");
    return NS_OK;
  }

  if (mDebuting) {
    NS_TIMELINE_LEAVE("nsXULWindow::SetVisibility");
    return NS_OK;
  }
  mDebuting = PR_TRUE;  // (Show / Focus is recursive)

  //XXXTAB Do we really need to show docshell and the window?  Isn't 
  // the window good enough?
  nsCOMPtr<nsIBaseWindow> shellAsWin(do_QueryInterface(mDocShell));
  shellAsWin->SetVisibility(aVisibility);
  mWindow->Show(aVisibility);

  nsCOMPtr<nsIWindowMediator> windowMediator(do_GetService(NS_WINDOWMEDIATOR_CONTRACTID));
  if (windowMediator)
     windowMediator->UpdateWindowTimeStamp(static_cast<nsIXULWindow*>(this));

  // notify observers so that we can hide the splash screen if possible
  nsCOMPtr<nsIObserverService> obssvc
    (do_GetService("@mozilla.org/observer-service;1"));
  NS_ASSERTION(obssvc, "Couldn't get observer service.");
  if (obssvc) {
    obssvc->NotifyObservers(nsnull, "xul-window-visible", nsnull); 
  }

  mDebuting = PR_FALSE;
  NS_TIMELINE_LEAVE("nsXULWindow::SetVisibility");
  return NS_OK;
}

NS_IMETHODIMP nsXULWindow::GetEnabled(PRBool *aEnabled)
{
  NS_ENSURE_ARG_POINTER(aEnabled);
  if (mWindow)
    return mWindow->IsEnabled(aEnabled);

  *aEnabled = PR_TRUE; // better guess than most
  return NS_ERROR_FAILURE;
}

NS_IMETHODIMP nsXULWindow::SetEnabled(PRBool aEnable)
{
  if (mWindow) {
    mWindow->Enable(aEnable);
    return NS_OK;
  }
  return NS_ERROR_FAILURE;
}

NS_IMETHODIMP nsXULWindow::GetBlurSuppression(PRBool *aBlurSuppression)
{
  NS_ENSURE_ARG_POINTER(aBlurSuppression);
  *aBlurSuppression = mBlurSuppressionLevel > 0;
  return NS_OK;
}

NS_IMETHODIMP nsXULWindow::SetBlurSuppression(PRBool aBlurSuppression)
{
  if (aBlurSuppression)
    ++mBlurSuppressionLevel;
  else {
    NS_ASSERTION(mBlurSuppressionLevel > 0, "blur over-allowed");
    if (mBlurSuppressionLevel > 0)
      --mBlurSuppressionLevel;
  }
  return NS_OK;

  /* XXX propagate this information to the widget? It has its own
     independent concept of blur suppression. Each is used on
     a different platform, so at time of writing it's not necessary
     to keep them in sync. (And there's no interface for doing so.) */
}

NS_IMETHODIMP nsXULWindow::GetMainWidget(nsIWidget** aMainWidget)
{
  NS_ENSURE_ARG_POINTER(aMainWidget);
   
  *aMainWidget = mWindow;
  NS_IF_ADDREF(*aMainWidget);
  return NS_OK;
}

NS_IMETHODIMP nsXULWindow::SetFocus()
{
  //XXX First Check In
  NS_ASSERTION(PR_FALSE, "Not Yet Implemented");
  return NS_OK;
}

NS_IMETHODIMP nsXULWindow::GetTitle(PRUnichar** aTitle)
{
  NS_ENSURE_ARG_POINTER(aTitle);

  *aTitle = ToNewUnicode(mTitle);
  if (!*aTitle)
    return NS_ERROR_OUT_OF_MEMORY;
  return NS_OK;
}

NS_IMETHODIMP nsXULWindow::SetTitle(const PRUnichar* aTitle)
{
  NS_ENSURE_STATE(mWindow);
  mTitle.Assign(aTitle);
  mTitle.StripChars("\n\r");
  NS_ENSURE_SUCCESS(mWindow->SetTitle(mTitle), NS_ERROR_FAILURE);

  // Tell the window mediator that a title has changed
  nsCOMPtr<nsIWindowMediator> windowMediator(do_GetService(NS_WINDOWMEDIATOR_CONTRACTID));
  if(!windowMediator)
    return NS_OK;

  windowMediator->UpdateWindowTitle(static_cast<nsIXULWindow*>(this), aTitle);

  return NS_OK;
}


//*****************************************************************************
// nsXULWindow: Helpers
//*****************************************************************************   

NS_IMETHODIMP nsXULWindow::EnsureChromeTreeOwner()
{
  if (mChromeTreeOwner)
    return NS_OK;

  mChromeTreeOwner = new nsChromeTreeOwner();
  NS_ENSURE_TRUE(mChromeTreeOwner, NS_ERROR_OUT_OF_MEMORY);

  NS_ADDREF(mChromeTreeOwner);
  mChromeTreeOwner->XULWindow(this);

  return NS_OK;
}

NS_IMETHODIMP nsXULWindow::EnsureContentTreeOwner()
{
  if (mContentTreeOwner)
    return NS_OK;

  mContentTreeOwner = new nsContentTreeOwner(PR_FALSE);
  NS_ENSURE_TRUE(mContentTreeOwner, NS_ERROR_FAILURE);

  NS_ADDREF(mContentTreeOwner);
  mContentTreeOwner->XULWindow(this);
   
  return NS_OK;
}

NS_IMETHODIMP nsXULWindow::EnsurePrimaryContentTreeOwner()
{
  if (mPrimaryContentTreeOwner)
    return NS_OK;

  mPrimaryContentTreeOwner = new nsContentTreeOwner(PR_TRUE);
  NS_ENSURE_TRUE(mPrimaryContentTreeOwner, NS_ERROR_FAILURE);

  NS_ADDREF(mPrimaryContentTreeOwner);
  mPrimaryContentTreeOwner->XULWindow(this);

  return NS_OK;
}

NS_IMETHODIMP nsXULWindow::EnsurePrompter()
{
  if (mPrompter)
    return NS_OK;
   
  nsCOMPtr<nsIDOMWindowInternal> ourWindow;
  nsresult rv = GetWindowDOMWindow(getter_AddRefs(ourWindow));
  if (NS_SUCCEEDED(rv)) {
    nsCOMPtr<nsIWindowWatcher> wwatch = 
        do_GetService(NS_WINDOWWATCHER_CONTRACTID);
    if (wwatch)
      wwatch->GetNewPrompter(ourWindow, getter_AddRefs(mPrompter));
  }
  return mPrompter ? NS_OK : NS_ERROR_FAILURE;
}

NS_IMETHODIMP nsXULWindow::EnsureAuthPrompter()
{
  if (mAuthPrompter)
    return NS_OK;
      
  nsCOMPtr<nsIDOMWindowInternal> ourWindow;
  nsresult rv = GetWindowDOMWindow(getter_AddRefs(ourWindow));
  if (NS_SUCCEEDED(rv)) {
    nsCOMPtr<nsIWindowWatcher> wwatch(do_GetService(NS_WINDOWWATCHER_CONTRACTID));
    if (wwatch)
      wwatch->GetNewAuthPrompter(ourWindow, getter_AddRefs(mAuthPrompter));
  }
  return mAuthPrompter ? NS_OK : NS_ERROR_FAILURE;
}
 
void nsXULWindow::OnChromeLoaded()
{
  nsresult rv = EnsureContentTreeOwner();

  if (NS_SUCCEEDED(rv)) {
    mChromeLoaded = PR_TRUE;
    ApplyChromeFlags();

    LoadChromeHidingFromXUL();
    LoadWindowClassFromXUL();
    LoadIconFromXUL();
    LoadSizeFromXUL();
    if(mIntrinsicallySized) {
      // (if LoadSizeFromXUL set the size, mIntrinsicallySized will be false)
      nsCOMPtr<nsIContentViewer> cv;
      mDocShell->GetContentViewer(getter_AddRefs(cv));
      nsCOMPtr<nsIMarkupDocumentViewer> markupViewer(do_QueryInterface(cv));
      if(markupViewer)
        markupViewer->SizeToContent();
    }

    PRBool positionSet = PR_TRUE;
    nsCOMPtr<nsIXULWindow> parentWindow(do_QueryReferent(mParentWindow));
#if defined(XP_UNIX) && !defined(XP_MACOSX)
    // don't override WM placement on unix for independent, top-level windows
    // (however, we think the benefits of intelligent dependent window placement
    // trump that override.)
    if (!parentWindow)
      positionSet = PR_FALSE;
#endif
    if (positionSet)
      positionSet = LoadPositionFromXUL();
    LoadMiscPersistentAttributesFromXUL();

    //LoadContentAreas();

    if (mCenterAfterLoad && !positionSet)
      Center(parentWindow, parentWindow ? PR_FALSE : PR_TRUE, PR_FALSE);

    if(mShowAfterLoad)
      SetVisibility(PR_TRUE);
  }
  mPersistentAttributesMask |= PAD_POSITION | PAD_SIZE | PAD_MISC;
}

nsresult nsXULWindow::LoadChromeHidingFromXUL()
{
  NS_ENSURE_STATE(mWindow);

  // Get <window> element.
  nsCOMPtr<nsIDOMElement> windowElement;
  GetWindowDOMElement(getter_AddRefs(windowElement));
  NS_ENSURE_TRUE(windowElement, NS_ERROR_FAILURE);

  nsAutoString attr;
  nsresult rv = windowElement->GetAttribute(NS_LITERAL_STRING("hidechrome"), attr);

  if (NS_SUCCEEDED(rv) && attr.LowerCaseEqualsLiteral("true")) {
    mWindow->HideWindowChrome(PR_TRUE);
  }

  return NS_OK;
}

PRBool nsXULWindow::LoadPositionFromXUL()
{
  nsresult rv;
  PRBool   gotPosition = PR_FALSE;
  
  // if we're the hidden window, don't try to validate our size/position. We're
  // special.
  if (mIsHiddenWindow)
    return PR_FALSE;

  nsCOMPtr<nsIDOMElement> windowElement;
  GetWindowDOMElement(getter_AddRefs(windowElement));
  NS_ASSERTION(windowElement, "no xul:window");
  if (!windowElement)
    return PR_FALSE;

  PRInt32 currX = 0;
  PRInt32 currY = 0;
  PRInt32 currWidth = 0;
  PRInt32 currHeight = 0;
  PRInt32 errorCode;
  PRInt32 temp;

  GetPositionAndSize(&currX, &currY, &currWidth, &currHeight);

  // Obtain the position information from the <xul:window> element.
  PRInt32 specX = currX;
  PRInt32 specY = currY;
  nsAutoString posString;

  rv = windowElement->GetAttribute(NS_LITERAL_STRING("screenX"), posString);
  if (NS_SUCCEEDED(rv)) {
    temp = posString.ToInteger(&errorCode);
    if (NS_SUCCEEDED(errorCode)) {
      specX = temp;
      gotPosition = PR_TRUE;
    }
  }
  rv = windowElement->GetAttribute(NS_LITERAL_STRING("screenY"), posString);
  if (NS_SUCCEEDED(rv)) {
    temp = posString.ToInteger(&errorCode);
    if (NS_SUCCEEDED(errorCode)) {
      specY = temp;
      gotPosition = PR_TRUE;
    }
  }
    
  if (gotPosition) {
    // our position will be relative to our parent, if any
    nsCOMPtr<nsIBaseWindow> parent(do_QueryReferent(mParentWindow));
    if (parent) {
      PRInt32 parentX, parentY;
      if (NS_SUCCEEDED(parent->GetPosition(&parentX, &parentY))) {
        specX += parentX;
        specY += parentY;
      }
    }
    else {
      StaggerPosition(specX, specY, currWidth, currHeight);
    }
  }
  mWindow->ConstrainPosition(PR_FALSE, &specX, &specY);
  if (specX != currX || specY != currY)
    SetPosition(specX, specY);

  return gotPosition;
}

PRBool nsXULWindow::LoadSizeFromXUL()
{
  nsresult rv;
  PRBool   gotSize = PR_FALSE;
  
  // if we're the hidden window, don't try to validate our size/position. We're
  // special.
  if (mIsHiddenWindow)
    return PR_FALSE;

  nsCOMPtr<nsIDOMElement> windowElement;
  GetWindowDOMElement(getter_AddRefs(windowElement));
  NS_ASSERTION(windowElement, "no xul:window");
  if (!windowElement)
    return PR_FALSE;

  PRInt32 currWidth = 0;
  PRInt32 currHeight = 0;
  PRInt32 errorCode;
  PRInt32 temp;

  GetSize(&currWidth, &currHeight);

  // Obtain the position and sizing information from the <xul:window> element.
  PRInt32 specWidth = currWidth;
  PRInt32 specHeight = currHeight;
  nsAutoString sizeString;

  rv = windowElement->GetAttribute(NS_LITERAL_STRING("width"), sizeString);
  if (NS_SUCCEEDED(rv)) {
    temp = sizeString.ToInteger(&errorCode);
    if (NS_SUCCEEDED(errorCode) && temp > 0) {
      specWidth = temp > 100 ? temp : 100;
      gotSize = PR_TRUE;
    }
  }
  rv = windowElement->GetAttribute(NS_LITERAL_STRING("height"), sizeString);
  if (NS_SUCCEEDED(rv)) {
    temp = sizeString.ToInteger(&errorCode);
    if (NS_SUCCEEDED(errorCode) && temp > 0) {
      specHeight = temp > 100 ? temp : 100;
      gotSize = PR_TRUE;
    }
  }

  if (gotSize) {
    // constrain to screen size
    nsCOMPtr<nsIDOMWindowInternal> domWindow;
    GetWindowDOMWindow(getter_AddRefs(domWindow));
    if (domWindow) {
      nsCOMPtr<nsIDOMScreen> screen;
      domWindow->GetScreen(getter_AddRefs(screen));
      if (screen) {
        PRInt32 screenWidth;
        PRInt32 screenHeight;
        screen->GetAvailWidth(&screenWidth);
        screen->GetAvailHeight(&screenHeight);
        if (specWidth > screenWidth)
          specWidth = screenWidth;
        if (specHeight > screenHeight)
          specHeight = screenHeight;
      }
    }

    mIntrinsicallySized = PR_FALSE;
    if (specWidth != currWidth || specHeight != currHeight)
      SetSize(specWidth, specHeight, PR_FALSE);
  }

  return gotSize;
}

/* Miscellaneous persistent attributes are attributes named in the
   |persist| attribute, other than size and position. Those are special
   because it's important to load those before one of the misc
   attributes (sizemode) and they require extra processing. */
PRBool nsXULWindow::LoadMiscPersistentAttributesFromXUL()
{
  nsresult rv;
  PRBool   gotState = PR_FALSE;
  
  /* There are no misc attributes of interest to the hidden window.
     It's especially important not to try to validate that window's
     size or position, because some platforms (Mac OS X) need to
     make it visible and offscreen. */
  if (mIsHiddenWindow)
    return PR_FALSE;

  nsCOMPtr<nsIDOMElement> windowElement;
  GetWindowDOMElement(getter_AddRefs(windowElement));
  NS_ASSERTION(windowElement, "no xul:window");
  if (!windowElement)
    return PR_FALSE;

  nsAutoString stateString;

  // sizemode
  rv = windowElement->GetAttribute(NS_LITERAL_STRING("sizemode"), stateString);
  if (NS_SUCCEEDED(rv)) {
    PRInt32 sizeMode = nsSizeMode_Normal;
    /* ignore request to minimize, to not confuse novices
    if (stateString.Equals(SIZEMODE_MINIMIZED))
      sizeMode = nsSizeMode_Minimized;
    */
    if (stateString.Equals(SIZEMODE_MAXIMIZED)) {
      /* Honor request to maximize only if the window is sizable.
         An unsizable, unmaximizable, yet maximized window confuses
         Windows OS and is something of a travesty, anyway. */
      if (mChromeFlags & nsIWebBrowserChrome::CHROME_WINDOW_RESIZE) {
        mIntrinsicallySized = PR_FALSE;
        sizeMode = nsSizeMode_Maximized;
      }
    }
    // the widget had better be able to deal with not becoming visible yet
    mWindow->SetSizeMode(sizeMode);
    gotState = PR_TRUE;
  }

  // zlevel
  rv = windowElement->GetAttribute(NS_LITERAL_STRING("zlevel"), stateString);
  if (NS_SUCCEEDED(rv) && stateString.Length() > 0) {
    PRInt32  errorCode;
    PRUint32 zLevel = stateString.ToInteger(&errorCode);
    if (NS_SUCCEEDED(errorCode) && zLevel >= lowestZ && zLevel <= highestZ)
      SetZLevel(zLevel);
  }

  return gotState;
}

/* Stagger windows of the same type so they don't appear on top of each other.
   This code does have a scary double loop -- it'll keep passing through
   the entire list of open windows until it finds a non-collision. Doesn't
   seem to be a problem, but it deserves watching.
*/
void nsXULWindow::StaggerPosition(PRInt32 &aRequestedX, PRInt32 &aRequestedY,
                                  PRInt32 aSpecWidth, PRInt32 aSpecHeight)
{
  const PRInt32 kOffset = 22;
  const PRInt32 kSlop = 4;
  nsresult rv;
  PRBool   keepTrying;
  int      bouncedX = 0, // bounced off vertical edge of screen
           bouncedY = 0; // bounced off horizontal edge

  // look for any other windows of this type
  nsCOMPtr<nsIWindowMediator> wm(do_GetService(NS_WINDOWMEDIATOR_CONTRACTID));
  if (!wm)
    return;

  nsCOMPtr<nsIDOMElement> windowElement;
  GetWindowDOMElement(getter_AddRefs(windowElement));
  nsCOMPtr<nsIXULWindow> ourXULWindow(this);

  nsAutoString windowType;
  rv = windowElement->GetAttribute(WINDOWTYPE_ATTRIBUTE, windowType);
  if (NS_FAILED(rv))
    return;

  PRInt32 screenTop = 0,    // it's pointless to initialize these ...
          screenRight = 0,  // ... but to prevent oversalubrious and ...
          screenBottom = 0, // ... underbright compilers from ...
          screenLeft = 0;   // ... issuing warnings.
  PRBool  gotScreen = PR_FALSE;

  { // fetch screen coordinates
    nsCOMPtr<nsIScreenManager> screenMgr(do_GetService(
                                         "@mozilla.org/gfx/screenmanager;1"));
    if (screenMgr) {
      nsCOMPtr<nsIScreen> ourScreen;
      screenMgr->ScreenForRect(aRequestedX, aRequestedY,
                               aSpecWidth, aSpecHeight,
                               getter_AddRefs(ourScreen));
      if (ourScreen) {
        PRInt32 screenWidth, screenHeight;
        ourScreen->GetAvailRect(&screenLeft, &screenTop,
                                &screenWidth, &screenHeight);
        screenBottom = screenTop + screenHeight;
        screenRight = screenLeft + screenWidth;
        gotScreen = PR_TRUE;
      }
    }
  }

  // One full pass through all windows of this type, repeat until no collisions.
  do {
    keepTrying = PR_FALSE;
    nsCOMPtr<nsISimpleEnumerator> windowList;
    wm->GetXULWindowEnumerator(windowType.get(), getter_AddRefs(windowList));

    if (!windowList)
      break;

    // One full pass through all windows of this type, offset and stop on collision.
    do {
      PRBool more;
      windowList->HasMoreElements(&more);
      if (!more)
        break;

      nsCOMPtr<nsISupports> supportsWindow;
      windowList->GetNext(getter_AddRefs(supportsWindow));

      nsCOMPtr<nsIXULWindow> listXULWindow(do_QueryInterface(supportsWindow));
      if (listXULWindow != ourXULWindow) {
        PRInt32 listX, listY;
        nsCOMPtr<nsIBaseWindow> listBaseWindow(do_QueryInterface(supportsWindow));
        listBaseWindow->GetPosition(&listX, &listY);

        if (PR_ABS(listX - aRequestedX) <= kSlop &&
            PR_ABS(listY - aRequestedY) <= kSlop) {
          // collision! offset and start over
          if (bouncedX & 0x1)
            aRequestedX -= kOffset;
          else
            aRequestedX += kOffset;
          aRequestedY += kOffset;

          if (gotScreen) {
            // if we're moving to the right and we need to bounce...
            if (!(bouncedX & 0x1) && ((aRequestedX + aSpecWidth) > screenRight)) {
              aRequestedX = screenRight - aSpecWidth;
              ++bouncedX;
            }

            // if we're moving to the left and we need to bounce...
            if ((bouncedX & 0x1) && aRequestedX < screenLeft) {
              aRequestedX = screenLeft;
              ++bouncedX;
            }

            // if we hit the bottom then bounce to the top
            if (aRequestedY + aSpecHeight > screenBottom) {
              aRequestedY = screenTop;
              ++bouncedY;
            }
          }

          /* loop around again,
             but it's time to give up once we've covered the screen.
             there's a potential infinite loop with lots of windows. */
          keepTrying = bouncedX < 2 || bouncedY == 0;
          break;
        }
      }
    } while(1);
  } while (keepTrying);
}

NS_IMETHODIMP nsXULWindow::LoadWindowClassFromXUL()
{
  nsCOMPtr<nsIDOMElement> docShellElement;
  GetWindowDOMElement(getter_AddRefs(docShellElement));
  NS_ENSURE_TRUE(docShellElement, NS_ERROR_FAILURE);

  nsAutoString windowType;

  docShellElement->GetAttribute(NS_LITERAL_STRING("windowtype"),
                                windowType);

  if (!windowType.IsEmpty()) {
    mWindow->SetWindowClass(windowType);
  }

  return NS_OK;
}

NS_IMETHODIMP nsXULWindow::LoadIconFromXUL()
{
  NS_ENSURE_STATE(mWindow);

  // Get <window> element.
  nsCOMPtr<nsIDOMElement> windowElement;
  GetWindowDOMElement(getter_AddRefs(windowElement));
  NS_ENSURE_TRUE(windowElement, NS_ERROR_FAILURE);

  // XXX The following code is being #if 0'd out since it
  // basically does nothing until bug 70974 is fixed.
  // After bug 70974 is fixed, we will also need to implement
  // computed style for that property before this will
  // be of any use.  And even then, it will *still* 
  // do nothing on platforms which don't implement 
  // nsWindow::SetIcon(). See bug 76211 for that.
  // Also see bug 57576 and its dependency tree.
#if 0
  // Get document in which this <window> is contained.
  nsCOMPtr<nsIDOMDocument> document;
  windowElement->GetOwnerDocument(getter_AddRefs(document));
  NS_ENSURE_TRUE(document, NS_ERROR_FAILURE);

  // Get document view.
  nsCOMPtr<nsIDOMDocumentView> docView(do_QueryInterface(document));
  NS_ENSURE_TRUE(docView, NS_ERROR_FAILURE);

  // Get default/abstract view.
  nsCOMPtr<nsIDOMAbstractView> abstractView;
  docView->GetDefaultView(getter_AddRefs(abstractView));
  NS_ENSURE_TRUE(abstractView, NS_ERROR_FAILURE);

  // Get "view CSS."
  nsCOMPtr<nsIDOMViewCSS> viewCSS(do_QueryInterface(abstractView));
  NS_ENSURE_TRUE(viewCSS, NS_ERROR_FAILURE);

  // Next, get CSS style declaration.
  nsCOMPtr<nsIDOMCSSStyleDeclaration> cssDecl;
  viewCSS->GetComputedStyle(windowElement, EmptyString(),
                            getter_AddRefs(cssDecl));
  NS_ENSURE_TRUE(cssDecl, NS_ERROR_FAILURE);

  // Whew.  Now get "list-style-image" property value.
  nsAutoString windowIcon;
  windowIcon.AssignLiteral("-moz-window-icon");
  nsAutoString icon;
  cssDecl->GetPropertyValue(windowIcon, icon);
#endif

  nsAutoString id;
  windowElement->GetAttribute(NS_LITERAL_STRING("id"), id);

  if (id.IsEmpty()) {
    id.AssignLiteral("default");
  }

  mWindow->SetIcon(id);
  return NS_OK;
}

NS_IMETHODIMP nsXULWindow::SavePersistentAttributes()
{
  // can happen when the persistence timer fires at an inopportune time
  // during window shutdown
  if (!mDocShell)
    return NS_ERROR_FAILURE;

  nsCOMPtr<nsIDOMElement> docShellElement;
  GetWindowDOMElement(getter_AddRefs(docShellElement));
  if(!docShellElement)
    return NS_ERROR_FAILURE;

  nsAutoString   persistString;
  docShellElement->GetAttribute(PERSIST_ATTRIBUTE, persistString);
  if (persistString.IsEmpty()) { // quick check which sometimes helps
    mPersistentAttributesDirty = 0;
    return NS_OK;
  }

  PRInt32 x, y, cx, cy;
  PRInt32 sizeMode;

  // get our size, position and mode to persist
  NS_ENSURE_SUCCESS(GetPositionAndSize(&x, &y, &cx, &cy), NS_ERROR_FAILURE);
  mWindow->GetSizeMode(&sizeMode);

  // make our position relative to our parent, if any
  nsCOMPtr<nsIBaseWindow> parent(do_QueryReferent(mParentWindow));
  if (parent) {
    PRInt32 parentX, parentY;
    if (NS_SUCCEEDED(parent->GetPosition(&parentX, &parentY))) {
      x -= parentX;
      y -= parentY;
    }
  }

  char                        sizeBuf[10];
  nsAutoString                sizeString;
  nsAutoString                windowElementId;
  nsCOMPtr<nsIDOMXULDocument> ownerXULDoc;

  { // fetch docShellElement's ID and XUL owner document
    nsCOMPtr<nsIDOMDocument> ownerDoc;
    docShellElement->GetOwnerDocument(getter_AddRefs(ownerDoc));
    ownerXULDoc = do_QueryInterface(ownerDoc);
    nsCOMPtr<nsIDOMXULElement> XULElement(do_QueryInterface(docShellElement));
    if (XULElement)
      XULElement->GetId(windowElementId);
  }

  // (only for size elements which are persisted)
  if ((mPersistentAttributesDirty & PAD_POSITION) &&
      sizeMode == nsSizeMode_Normal) {
    if(persistString.Find("screenX") >= 0) {
      PR_snprintf(sizeBuf, sizeof(sizeBuf), "%ld", (long)x);
      sizeString.AssignWithConversion(sizeBuf);
      docShellElement->SetAttribute(SCREENX_ATTRIBUTE, sizeString);
      if (ownerXULDoc) // force persistence in case the value didn't change
        ownerXULDoc->Persist(windowElementId, SCREENX_ATTRIBUTE);
    }
    if(persistString.Find("screenY") >= 0) {
      PR_snprintf(sizeBuf, sizeof(sizeBuf), "%ld", (long)y);
      sizeString.AssignWithConversion(sizeBuf);
      docShellElement->SetAttribute(SCREENY_ATTRIBUTE, sizeString);
      if (ownerXULDoc)
        ownerXULDoc->Persist(windowElementId, SCREENY_ATTRIBUTE);
    }
  }

  if ((mPersistentAttributesDirty & PAD_SIZE) &&
      sizeMode == nsSizeMode_Normal) {
    if(persistString.Find("width") >= 0) {
      PR_snprintf(sizeBuf, sizeof(sizeBuf), "%ld", (long)cx);
      sizeString.AssignWithConversion(sizeBuf);
      docShellElement->SetAttribute(WIDTH_ATTRIBUTE, sizeString);
      if (ownerXULDoc)
        ownerXULDoc->Persist(windowElementId, WIDTH_ATTRIBUTE);
    }
    if(persistString.Find("height") >= 0) {
      PR_snprintf(sizeBuf, sizeof(sizeBuf), "%ld", (long)cy);
      sizeString.AssignWithConversion(sizeBuf);
      docShellElement->SetAttribute(HEIGHT_ATTRIBUTE, sizeString);
      if (ownerXULDoc)
        ownerXULDoc->Persist(windowElementId, HEIGHT_ATTRIBUTE);
    }
  }

  if (mPersistentAttributesDirty & PAD_MISC) {
    if (sizeMode != nsSizeMode_Minimized &&
        persistString.Find("sizemode") >= 0) {
      if (sizeMode == nsSizeMode_Maximized)
        sizeString.Assign(SIZEMODE_MAXIMIZED);
      else
        sizeString.Assign(SIZEMODE_NORMAL);
      docShellElement->SetAttribute(MODE_ATTRIBUTE, sizeString);
      if (ownerXULDoc)
        ownerXULDoc->Persist(windowElementId, MODE_ATTRIBUTE);
    }
    if (persistString.Find("zlevel") >= 0) {
      PRUint32 zLevel;
      nsCOMPtr<nsIWindowMediator> mediator(do_GetService(NS_WINDOWMEDIATOR_CONTRACTID));
      if (mediator) {
        mediator->GetZLevel(this, &zLevel);
        PR_snprintf(sizeBuf, sizeof(sizeBuf), "%lu", (unsigned long)zLevel);
        sizeString.AssignWithConversion(sizeBuf);
        docShellElement->SetAttribute(ZLEVEL_ATTRIBUTE, sizeString);
        ownerXULDoc->Persist(windowElementId, ZLEVEL_ATTRIBUTE);
      }
    }
  }

  mPersistentAttributesDirty = 0;
  return NS_OK;
}

NS_IMETHODIMP nsXULWindow::GetWindowDOMWindow(nsIDOMWindowInternal** aDOMWindow)
{
  NS_ENSURE_STATE(mDocShell);

  if (!mDOMWindow)
    mDOMWindow = do_GetInterface(mDocShell);
  NS_ENSURE_TRUE(mDOMWindow, NS_ERROR_FAILURE);

  *aDOMWindow = mDOMWindow;
  NS_ADDREF(*aDOMWindow);
  return NS_OK;
}

NS_IMETHODIMP nsXULWindow::GetWindowDOMElement(nsIDOMElement** aDOMElement)
{
  NS_ENSURE_STATE(mDocShell);
  NS_ENSURE_ARG_POINTER(aDOMElement);

  *aDOMElement = nsnull;

  nsCOMPtr<nsIContentViewer> cv;

  mDocShell->GetContentViewer(getter_AddRefs(cv));
  NS_ENSURE_TRUE(cv, NS_ERROR_FAILURE);

  nsCOMPtr<nsIDocumentViewer> docv(do_QueryInterface(cv));
  NS_ENSURE_TRUE(docv, NS_ERROR_FAILURE);

  nsCOMPtr<nsIDocument> doc;
  docv->GetDocument(getter_AddRefs(doc));
  nsCOMPtr<nsIDOMDocument> domdoc(do_QueryInterface(doc));
  NS_ENSURE_TRUE(domdoc, NS_ERROR_FAILURE);

  domdoc->GetDocumentElement(aDOMElement);
  NS_ENSURE_TRUE(*aDOMElement, NS_ERROR_FAILURE);

  return NS_OK;
}

NS_IMETHODIMP nsXULWindow::GetDOMElementById(char* aID, nsIDOMElement** aDOMElement)
{
  NS_ENSURE_STATE(mDocShell);
  NS_ENSURE_ARG_POINTER(aDOMElement);

  *aDOMElement = nsnull;

  nsCOMPtr<nsIContentViewer> cv;

  mDocShell->GetContentViewer(getter_AddRefs(cv));
  if(!cv)
    return NS_ERROR_FAILURE;

  nsCOMPtr<nsIDocumentViewer> docv(do_QueryInterface(cv));
  if(!docv)   
    return NS_ERROR_FAILURE;

  nsCOMPtr<nsIDocument> doc;
  docv->GetDocument(getter_AddRefs(doc));
  nsCOMPtr<nsIDOMDocument> domdoc(do_QueryInterface(doc));
  if(!domdoc) 
    return NS_ERROR_FAILURE;

  NS_ENSURE_SUCCESS(domdoc->GetElementById(NS_ConvertASCIItoUTF16(aID), aDOMElement), NS_ERROR_FAILURE);

  return NS_OK;
}

nsresult nsXULWindow::ContentShellAdded(nsIDocShellTreeItem* aContentShell,
   PRBool aPrimary, PRBool aTargetable, const nsAString& aID)
{
  nsContentShellInfo* shellInfo = nsnull;

  PRInt32 count = mContentShells.Count();
  PRInt32 i;
  nsWeakPtr contentShellWeak = do_GetWeakReference(aContentShell);
  for (i = 0; i < count; i++) {
    nsContentShellInfo* info = (nsContentShellInfo*)mContentShells.ElementAt(i);
    if (info->id.Equals(aID)) {
      // We already exist. Do a replace.
      info->child = contentShellWeak;
      shellInfo = info;
    }
    else if (info->child == contentShellWeak)
      info->child = nsnull;
  }

  if (!shellInfo) {
    shellInfo = new nsContentShellInfo(aID, contentShellWeak);
    mContentShells.AppendElement((void*)shellInfo);
  }
    
  // Set the default content tree owner
  if (aPrimary) {
    NS_ENSURE_SUCCESS(EnsurePrimaryContentTreeOwner(), NS_ERROR_FAILURE);
    aContentShell->SetTreeOwner(mPrimaryContentTreeOwner);
    mPrimaryContentShell = aContentShell;
  }
  else {
    NS_ENSURE_SUCCESS(EnsureContentTreeOwner(), NS_ERROR_FAILURE);
    aContentShell->SetTreeOwner(mContentTreeOwner);
    if (mPrimaryContentShell == aContentShell)
      mPrimaryContentShell = nsnull;
  }

  if (aTargetable) {
#ifdef DEBUG
    PRInt32 debugCount = mTargetableShells.Count();
    PRInt32 debugCounter;
    for (debugCounter = debugCount - 1; debugCounter >= 0; --debugCounter) {
      nsCOMPtr<nsIDocShellTreeItem> curItem =
        do_QueryReferent(mTargetableShells[debugCounter]);
      NS_ASSERTION(!SameCOMIdentity(curItem, aContentShell),
                   "Adding already existing item to mTargetableShells");
    }
#endif
    
    // put the new shell at the start of the targetable shells list if either
    // it's the new primary shell or there is no existing primary shell (which
    // means that chances are this one just stopped being primary).  If we
    // really cared, we could keep track of the "last no longer primary shell"
    // explicitly, but it probably doesn't matter enough: the difference would
    // only be felt in a situation where all shells were non-primary, which
    // doesn't happen much.  In a situation where there is one and only one
    // primary shell, and in which shells get unmarked as primary before some
    // other shell gets marked as primary, this effectively stores the list of
    // targetable shells in "most recently primary first" order.
    PRBool inserted;
    if (aPrimary || !mPrimaryContentShell) {
      inserted = mTargetableShells.InsertObjectAt(contentShellWeak, 0);
    } else {
      inserted = mTargetableShells.AppendObject(contentShellWeak);
    }
    NS_ENSURE_TRUE(inserted, NS_ERROR_OUT_OF_MEMORY);
  }

  return NS_OK;
}

nsresult nsXULWindow::ContentShellRemoved(nsIDocShellTreeItem* aContentShell)
{
  if (mPrimaryContentShell == aContentShell) {
    mPrimaryContentShell = nsnull;
  }

  PRInt32 count = mContentShells.Count();
  PRInt32 i;
  for (i = count - 1; i >= 0; --i) {
    nsContentShellInfo* info = (nsContentShellInfo*)mContentShells.ElementAt(i);
    nsCOMPtr<nsIDocShellTreeItem> curItem = do_QueryReferent(info->child);
    if (!curItem || SameCOMIdentity(curItem, aContentShell)) {
      mContentShells.RemoveElementAt(i);
      delete info;
    }
  }

  count = mTargetableShells.Count();
  for (i = count - 1; i >= 0; --i) {
    nsCOMPtr<nsIDocShellTreeItem> curItem =
      do_QueryReferent(mTargetableShells[i]);
    if (!curItem || SameCOMIdentity(curItem, aContentShell)) {
      mTargetableShells.RemoveObjectAt(i);
    }
  }
  
  return NS_OK;
}

NS_IMETHODIMP nsXULWindow::SizeShellTo(nsIDocShellTreeItem* aShellItem,
   PRInt32 aCX, PRInt32 aCY)
{
  // XXXTAB This is wrong, we should actually reflow based on the passed in
  // shell.  For now we are hacking and doing delta sizing.  This is bad
  // because it assumes all size we add will go to the shell which probably
  // won't happen.

  nsCOMPtr<nsIBaseWindow> shellAsWin(do_QueryInterface(aShellItem));
  NS_ENSURE_TRUE(shellAsWin, NS_ERROR_FAILURE);

  PRInt32 width = 0;
  PRInt32 height = 0;
  shellAsWin->GetSize(&width, &height);

  PRInt32 widthDelta = aCX - width;
  PRInt32 heightDelta = aCY - height;

  if (widthDelta || heightDelta) {
    PRInt32 winCX = 0;
    PRInt32 winCY = 0;

    GetSize(&winCX, &winCY);
    SetSize(winCX + widthDelta, winCY + heightDelta, PR_TRUE);
  }

  return NS_OK;
}

NS_IMETHODIMP nsXULWindow::ExitModalLoop(nsresult aStatus)
{
  if (mContinueModalLoop)
    EnableParent(PR_TRUE);
  mContinueModalLoop = PR_FALSE;
  mModalStatus = aStatus;
  return NS_OK;
}

// top-level function to create a new window
NS_IMETHODIMP nsXULWindow::CreateNewWindow(PRInt32 aChromeFlags,
                             nsIAppShell* aAppShell, nsIXULWindow **_retval)
{
  NS_ENSURE_ARG_POINTER(_retval);

  if (aChromeFlags & nsIWebBrowserChrome::CHROME_OPENAS_CHROME)
    return CreateNewChromeWindow(aChromeFlags, aAppShell, _retval);
  return CreateNewContentWindow(aChromeFlags, aAppShell, _retval);
}

NS_IMETHODIMP nsXULWindow::CreateNewChromeWindow(PRInt32 aChromeFlags,
   nsIAppShell* aAppShell, nsIXULWindow **_retval)
{
  NS_TIMELINE_ENTER("nsXULWindow::CreateNewChromeWindow");
  nsCOMPtr<nsIAppShellService> appShell(do_GetService(NS_APPSHELLSERVICE_CONTRACTID));
  NS_ENSURE_TRUE(appShell, NS_ERROR_FAILURE);

  // Just do a normal create of a window and return.
  //XXXTAB remove this when appshell talks in terms of nsIXULWindow
  nsCOMPtr<nsIXULWindow> parent;
  if(aChromeFlags & nsIWebBrowserChrome::CHROME_DEPENDENT)
    parent = this;

  nsCOMPtr<nsIXULWindow> newWindow;
  appShell->CreateTopLevelWindow(parent, nsnull, aChromeFlags,
                                 nsIAppShellService::SIZE_TO_CONTENT,
                                 nsIAppShellService::SIZE_TO_CONTENT,
                                 aAppShell, getter_AddRefs(newWindow));

  NS_ENSURE_TRUE(newWindow, NS_ERROR_FAILURE);

  newWindow->SetChromeFlags(aChromeFlags);

  *_retval = newWindow;
  NS_ADDREF(*_retval);

  NS_TIMELINE_LEAVE("nsXULWindow::CreateNewChromeWindow done");
  return NS_OK;
}

NS_IMETHODIMP nsXULWindow::CreateNewContentWindow(PRInt32 aChromeFlags,
   nsIAppShell* aAppShell, nsIXULWindow **_retval)
{
  NS_TIMELINE_ENTER("nsXULWindow::CreateNewContentWindow");

  nsCOMPtr<nsIAppShellService> appShell(do_GetService(NS_APPSHELLSERVICE_CONTRACTID));
  NS_ENSURE_TRUE(appShell, NS_ERROR_FAILURE);

  nsCOMPtr<nsIXULWindow> parent;
  if (aChromeFlags & nsIWebBrowserChrome::CHROME_DEPENDENT)
    parent = this;

  // We need to create a new top level window and then enter a nested
  // loop. Eventually the new window will be told that it has loaded,
  // at which time we know it is safe to spin out of the nested loop
  // and allow the opening code to proceed.

  nsCOMPtr<nsIURI> uri;

  nsCOMPtr<nsIPrefBranch> prefs(do_GetService(NS_PREFSERVICE_CONTRACTID));
  if (prefs) {
    nsXPIDLCString urlStr;
    nsresult prefres;
    prefres = prefs->GetCharPref("browser.chromeURL", getter_Copies(urlStr));
    if (NS_SUCCEEDED(prefres) && urlStr.IsEmpty())
      prefres = NS_ERROR_FAILURE;
    if (NS_FAILED(prefres))
      urlStr.AssignLiteral("chrome://navigator/content/navigator.xul");

    nsCOMPtr<nsIIOService> service(do_GetService(NS_IOSERVICE_CONTRACTID));
    if (service)
      service->NewURI(urlStr, nsnull, nsnull, getter_AddRefs(uri));
  }
  NS_ENSURE_TRUE(uri, NS_ERROR_FAILURE);

  nsCOMPtr<nsIXULWindow> newWindow;
  appShell->CreateTopLevelWindow(parent, uri,
                                aChromeFlags, 615, 480, aAppShell,
                                getter_AddRefs(newWindow));

  NS_ENSURE_TRUE(newWindow, NS_ERROR_FAILURE);

  newWindow->SetChromeFlags(aChromeFlags);

  // Specify that we want the window to remain locked until the chrome has loaded.
  nsXULWindow *xulWin = static_cast<nsXULWindow*>
                                   (static_cast<nsIXULWindow*>
                                               (newWindow));

  xulWin->LockUntilChromeLoad();

  // Push nsnull onto the JSContext stack before we dispatch a native event.
  nsCOMPtr<nsIJSContextStack> stack(do_GetService("@mozilla.org/js/xpc/ContextStack;1"));
  if (stack && NS_SUCCEEDED(stack->Push(nsnull))) {
    nsIThread *thread = NS_GetCurrentThread();
    while (xulWin->IsLocked()) {
      if (!NS_ProcessNextEvent(thread))
        break;
    }
    JSContext *cx;
    stack->Pop(&cx);
    NS_ASSERTION(cx == nsnull, "JSContextStack mismatch");
  }

  *_retval = newWindow;
  NS_ADDREF(*_retval);

  NS_TIMELINE_LEAVE("nsXULWindow::CreateNewContentWindow");
  return NS_OK;
}

void nsXULWindow::EnableParent(PRBool aEnable)
{
  nsCOMPtr<nsIBaseWindow> parentWindow;
  nsCOMPtr<nsIWidget>     parentWidget;

  parentWindow = do_QueryReferent(mParentWindow);
  if (parentWindow)
    parentWindow->GetMainWidget(getter_AddRefs(parentWidget));
  if (parentWidget)
    parentWidget->Enable(aEnable);
}

// Constrain the window to its proper z-level
PRBool nsXULWindow::ConstrainToZLevel(
                      PRBool      aImmediate,
                      nsWindowZ  *aPlacement,
                      nsIWidget  *aReqBelow,
                      nsIWidget **aActualBelow) {

#if 0
  /* Do we have a parent window? This means our z-order is already constrained,
     since we're a dependent window. Our window list isn't hierarchical,
     so we can't properly calculate placement for such a window.
     Should we just abort? */
  nsCOMPtr<nsIBaseWindow> parentWindow = do_QueryReferent(mParentWindow);
  if (parentWindow)
    return PR_FALSE;
#endif

  nsCOMPtr<nsIWindowMediator> mediator(do_GetService(NS_WINDOWMEDIATOR_CONTRACTID));
  if(!mediator)
    return PR_FALSE;

  PRBool         altered;
  PRUint32       position,
                 newPosition,
                 zLevel;
  nsIXULWindow  *us = this;

  altered = PR_FALSE;
  mediator->GetZLevel(this, &zLevel);

  // translate from nsGUIEvent to nsIWindowMediator constants
  position = nsIWindowMediator::zLevelTop;
  if (*aPlacement == nsWindowZBottom || zLevel == nsIXULWindow::lowestZ)
    position = nsIWindowMediator::zLevelBottom;
  else if (*aPlacement == nsWindowZRelative)
    position = nsIWindowMediator::zLevelBelow;

  if (NS_SUCCEEDED(mediator->CalculateZPosition(us, position, aReqBelow,
                               &newPosition, aActualBelow, &altered))) {

    /* If we were asked to move to the top but constrained to remain
       below one of our other windows, first move all windows in that
       window's layer and above to the top. This allows the user to
       click a window which can't be topmost and still bring mozilla
       to the foreground. */
    if (altered &&
        (position == nsIWindowMediator::zLevelTop ||
         position == nsIWindowMediator::zLevelBelow && aReqBelow == 0))

      PlaceWindowLayersBehind(zLevel+1, nsIXULWindow::highestZ, 0);

    if (*aPlacement != nsWindowZBottom &&
        position == nsIWindowMediator::zLevelBottom)
      altered = PR_TRUE;
    if (altered || aImmediate) {
      if (newPosition == nsIWindowMediator::zLevelTop)
        *aPlacement = nsWindowZTop;
      else if (newPosition == nsIWindowMediator::zLevelBottom)
        *aPlacement = nsWindowZBottom;
      else
        *aPlacement = nsWindowZRelative;

      if (aImmediate) {
        nsCOMPtr<nsIBaseWindow> ourBase = do_QueryInterface(static_cast<nsIXULWindow *>(this));
        if (ourBase) {
          nsCOMPtr<nsIWidget> ourWidget;
          ourBase->GetMainWidget(getter_AddRefs(ourWidget));
          ourWidget->PlaceBehind(*aPlacement == nsWindowZBottom ?
                                   eZPlacementBottom : eZPlacementBelow,
                                 *aActualBelow, PR_FALSE);
        }
      }
    }

    /* (CalculateZPosition can tell us to be below nothing, because it tries
       not to change something it doesn't recognize. A request to verify
       being below an unrecognized window, then, is treated as a request
       to come to the top (below null) */
    nsCOMPtr<nsIXULWindow> windowAbove;
    if (newPosition == nsIWindowMediator::zLevelBelow && *aActualBelow) {
      void *data;
      (*aActualBelow)->GetClientData(data);
      if (data) {
        windowAbove = reinterpret_cast<nsWebShellWindow*>(data);
      }
    }

    mediator->SetZPosition(us, newPosition, windowAbove);
  }

  return altered;
}

/* Re-z-position all windows in the layers from aLowLevel to aHighLevel,
   inclusive, to be behind aBehind. aBehind of null means on top.
   Note this method actually does nothing to our relative window positions.
   (And therefore there's no need to inform WindowMediator we're moving
   things, because we aren't.) This method is useful for, say, moving
   a range of layers of our own windows relative to windows belonging to
   external applications.
*/
void nsXULWindow::PlaceWindowLayersBehind(PRUint32 aLowLevel,
                                          PRUint32 aHighLevel,
                                          nsIXULWindow *aBehind) {

  // step through windows in z-order from top to bottommost window

  nsCOMPtr<nsIWindowMediator> mediator(do_GetService(NS_WINDOWMEDIATOR_CONTRACTID));
  if(!mediator)
    return;

  nsCOMPtr<nsISimpleEnumerator> windowEnumerator;
  mediator->GetZOrderXULWindowEnumerator(0, PR_TRUE,
              getter_AddRefs(windowEnumerator));
  if (!windowEnumerator)
    return;

  // each window will be moved behind previousHighWidget, itself
  // a moving target. initialize it.
  nsCOMPtr<nsIWidget> previousHighWidget;
  if (aBehind) {
    nsCOMPtr<nsIBaseWindow> highBase(do_QueryInterface(aBehind));
    if (highBase)
      highBase->GetMainWidget(getter_AddRefs(previousHighWidget));
  }

  // get next lower window
  PRBool more;
  while (windowEnumerator->HasMoreElements(&more), more) {
    PRUint32 nextZ; // z-level of nextWindow
    nsCOMPtr<nsISupports> nextWindow;
    windowEnumerator->GetNext(getter_AddRefs(nextWindow));
    nsCOMPtr<nsIXULWindow> nextXULWindow(do_QueryInterface(nextWindow));
    nextXULWindow->GetZLevel(&nextZ);
    if (nextZ < aLowLevel)
      break; // we've processed all windows through aLowLevel
    
    // move it just below its next higher window
    nsCOMPtr<nsIBaseWindow> nextBase(do_QueryInterface(nextXULWindow));
    if (nextBase) {
      nsCOMPtr<nsIWidget> nextWidget;
      nextBase->GetMainWidget(getter_AddRefs(nextWidget));
      if (nextZ <= aHighLevel)
        nextWidget->PlaceBehind(eZPlacementBelow, previousHighWidget, PR_FALSE);
      previousHighWidget = nextWidget;
    }
  }
}

void nsXULWindow::SetContentScrollbarVisibility(PRBool aVisible)
{
  nsCOMPtr<nsIDOMWindow> contentWin(do_GetInterface(mPrimaryContentShell));
  if (contentWin) {
    nsCOMPtr<nsIDOMBarProp> scrollbars;
    contentWin->GetScrollbars(getter_AddRefs(scrollbars));
    if (scrollbars)
      scrollbars->SetVisible(aVisible);
  }
}

PRBool nsXULWindow::GetContentScrollbarVisibility()
{
  // This code already exists in dom/src/base/nsBarProp.cpp, but we
  // can't safely get to that from here as this function is called
  // while the DOM window is being set up, and we need the DOM window
  // to get to that code.
  nsCOMPtr<nsIScrollable> scroller(do_QueryInterface(mPrimaryContentShell));

  if (scroller) {
    PRInt32 prefValue;
    scroller->GetDefaultScrollbarPreferences(
                  nsIScrollable::ScrollOrientation_Y, &prefValue);
    if (prefValue == nsIScrollable::Scrollbar_Never) // try the other way
      scroller->GetDefaultScrollbarPreferences(
                  nsIScrollable::ScrollOrientation_X, &prefValue);

    if (prefValue == nsIScrollable::Scrollbar_Never)
      return PR_FALSE;
  }

  return PR_TRUE;
}

// during spinup, attributes that haven't been loaded yet can't be dirty
void nsXULWindow::PersistentAttributesDirty(PRUint32 aDirtyFlags)
{
  mPersistentAttributesDirty |= aDirtyFlags & mPersistentAttributesMask;
}

NS_IMETHODIMP nsXULWindow::ApplyChromeFlags()
{
  nsCOMPtr<nsIDOMElement> window;
  GetWindowDOMElement(getter_AddRefs(window));
  NS_ENSURE_TRUE(window, NS_ERROR_FAILURE);

  if (mChromeLoaded) {
    // The two calls in this block don't need to happen early because they
    // don't cause a global restyle on the document.  Not only that, but the
    // scrollbar stuff needs a content area to toggle the scrollbars on anyway.
    // So just don't do these until mChromeLoaded is true.
    
    // menubar has its own special treatment
    mWindow->ShowMenuBar(mChromeFlags & nsIWebBrowserChrome::CHROME_MENUBAR ? 
                         PR_TRUE : PR_FALSE);

    // Scrollbars have their own special treatment.
    SetContentScrollbarVisibility(mChromeFlags &
                                  nsIWebBrowserChrome::CHROME_SCROLLBARS ?
                                    PR_TRUE : PR_FALSE);
  }

  /* the other flags are handled together. we have style rules
     in navigator.css that trigger visibility based on
     the 'chromehidden' attribute of the <window> tag. */
  nsAutoString newvalue;

  if (! (mChromeFlags & nsIWebBrowserChrome::CHROME_MENUBAR))
    newvalue.AppendLiteral("menubar ");

  if (! (mChromeFlags & nsIWebBrowserChrome::CHROME_TOOLBAR))
    newvalue.AppendLiteral("toolbar ");

  if (! (mChromeFlags & nsIWebBrowserChrome::CHROME_LOCATIONBAR))
    newvalue.AppendLiteral("location ");

  if (! (mChromeFlags & nsIWebBrowserChrome::CHROME_PERSONAL_TOOLBAR))
    newvalue.AppendLiteral("directories ");

  if (! (mChromeFlags & nsIWebBrowserChrome::CHROME_STATUSBAR))
    newvalue.AppendLiteral("status ");

  if (! (mChromeFlags & nsIWebBrowserChrome::CHROME_EXTRA))
    newvalue.AppendLiteral("extrachrome ");

  // Note that if we're not actually changing the value this will be a no-op,
  // so no need to compare to the old value.
  window->SetAttribute(NS_LITERAL_STRING("chromehidden"), newvalue);

  return NS_OK;
}

NS_IMETHODIMP nsXULWindow::GetXULBrowserWindow(nsIXULBrowserWindow * *aXULBrowserWindow)
{
  NS_IF_ADDREF(*aXULBrowserWindow = mXULBrowserWindow);
  return NS_OK;
}

NS_IMETHODIMP nsXULWindow::SetXULBrowserWindow(nsIXULBrowserWindow * aXULBrowserWindow)
{
  mXULBrowserWindow = aXULBrowserWindow;
  return NS_OK;
}

//*****************************************************************************
// nsXULWindow: Accessors
//*****************************************************************************

//*****************************************************************************
//*** nsContentShellInfo: Object Management
//*****************************************************************************   

nsContentShellInfo::nsContentShellInfo(const nsAString& aID,
                                       nsIWeakReference* aContentShell)
  : id(aID),
    child(aContentShell)
{
  MOZ_COUNT_CTOR(nsContentShellInfo);
}

nsContentShellInfo::~nsContentShellInfo()
{
  MOZ_COUNT_DTOR(nsContentShellInfo);
   //XXX Set Tree Owner to null if the tree owner is nsXULWindow->mContentTreeOwner
} 
