/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=2 sw=2 et tw=78: */
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
 * Portions created by the Initial Developer are Copyright (C) 2001
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Harshal Pradhan <keeda@hotpop.com>
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

//#define USEWEAKREFS // (haven't quite figured that out yet)

#include "nsWindowWatcher.h"

#include "nsAutoLock.h"
#include "nsCRT.h"
#include "nsNetUtil.h"
#include "nsPrompt.h"
#include "nsPromptService.h"
#include "nsWWJSUtils.h"
#include "plstr.h"

#include "nsIBaseWindow.h"
#include "nsIDocShell.h"
#include "nsIDocShellLoadInfo.h"
#include "nsIDocShellTreeItem.h"
#include "nsIDocShellTreeOwner.h"
#include "nsIDocumentLoader.h"
#include "nsIDocument.h"
#include "nsIDOMDocument.h"
#include "nsIDOMWindow.h"
#include "nsIDOMChromeWindow.h"
#include "nsIDOMWindowInternal.h"
#include "nsIScriptObjectPrincipal.h"
#include "nsIScreen.h"
#include "nsIScreenManager.h"
#include "nsIScriptContext.h"
#include "nsIGenericFactory.h"
#include "nsIJSContextStack.h"
#include "nsIObserverService.h"
#include "nsIScriptGlobalObject.h"
#include "nsIScriptSecurityManager.h"
#include "nsXPCOM.h"
#include "nsIURI.h"
#include "nsIWebBrowser.h"
#include "nsIWebBrowserChrome.h"
#include "nsIWebNavigation.h"
#include "nsIWindowCreator.h"
#include "nsIWindowCreator2.h"
#include "nsIXPConnect.h"
#include "nsPIDOMWindow.h"
#include "nsIMarkupDocumentViewer.h"
#include "nsIContentViewer.h"
#include "nsIDocumentViewer.h"
#include "nsIWindowProvider.h"
#include "nsIMutableArray.h"
#include "nsISupportsArray.h"
#include "nsIDeviceContext.h"

#include "nsIPrefBranch.h"
#include "nsIPrefService.h"

#include "jsinterp.h" // for js_AllocStack() and js_FreeStack()

#ifdef USEWEAKREFS
#include "nsIWeakReference.h"
#endif

static const char *sJSStackContractID="@mozilla.org/js/xpc/ContextStack;1";

/****************************************************************
 ******************** nsWatcherWindowEntry **********************
 ****************************************************************/

class nsWindowWatcher;

struct nsWatcherWindowEntry {

  nsWatcherWindowEntry(nsIDOMWindow *inWindow, nsIWebBrowserChrome *inChrome) {
#ifdef USEWEAKREFS
    mWindow = do_GetWeakReference(inWindow);
#else
    mWindow = inWindow;
#endif
    nsCOMPtr<nsISupportsWeakReference> supportsweak(do_QueryInterface(inChrome));
    if (supportsweak) {
      supportsweak->GetWeakReference(getter_AddRefs(mChromeWeak));
    } else {
      mChrome = inChrome;
      mChromeWeak = 0;
    }
    ReferenceSelf();
  }
  ~nsWatcherWindowEntry() {}

  void InsertAfter(nsWatcherWindowEntry *inOlder);
  void Unlink();
  void ReferenceSelf();

#ifdef USEWEAKREFS
  nsCOMPtr<nsIWeakReference> mWindow;
#else // still not an owning ref
  nsIDOMWindow              *mWindow;
#endif
  nsIWebBrowserChrome       *mChrome;
  nsWeakPtr                  mChromeWeak;
  // each struct is in a circular, doubly-linked list
  nsWatcherWindowEntry      *mYounger, // next younger in sequence
                            *mOlder;
};

void nsWatcherWindowEntry::InsertAfter(nsWatcherWindowEntry *inOlder)
{
  if (inOlder) {
    mOlder = inOlder;
    mYounger = inOlder->mYounger;
    mOlder->mYounger = this;
    if (mOlder->mOlder == mOlder)
      mOlder->mOlder = this;
    mYounger->mOlder = this;
    if (mYounger->mYounger == mYounger)
      mYounger->mYounger = this;
  }
}

void nsWatcherWindowEntry::Unlink() {

  mOlder->mYounger = mYounger;
  mYounger->mOlder = mOlder;
  ReferenceSelf();
}

void nsWatcherWindowEntry::ReferenceSelf() {

  mYounger = this;
  mOlder = this;
}

/****************************************************************
 ****************** nsWatcherWindowEnumerator *******************
 ****************************************************************/

class nsWatcherWindowEnumerator : public nsISimpleEnumerator {

public:
  nsWatcherWindowEnumerator(nsWindowWatcher *inWatcher);
  virtual ~nsWatcherWindowEnumerator();
  NS_IMETHOD HasMoreElements(PRBool *retval);
  NS_IMETHOD GetNext(nsISupports **retval);

  NS_DECL_ISUPPORTS

private:
  friend class nsWindowWatcher;

  nsWatcherWindowEntry *FindNext();
  void WindowRemoved(nsWatcherWindowEntry *inInfo);

  nsWindowWatcher      *mWindowWatcher;
  nsWatcherWindowEntry *mCurrentPosition;
};

NS_IMPL_ADDREF(nsWatcherWindowEnumerator)
NS_IMPL_RELEASE(nsWatcherWindowEnumerator)
NS_IMPL_QUERY_INTERFACE1(nsWatcherWindowEnumerator, nsISimpleEnumerator)

nsWatcherWindowEnumerator::nsWatcherWindowEnumerator(nsWindowWatcher *inWatcher)
  : mWindowWatcher(inWatcher),
    mCurrentPosition(inWatcher->mOldestWindow)
{
  mWindowWatcher->AddEnumerator(this);
  mWindowWatcher->AddRef();
}

nsWatcherWindowEnumerator::~nsWatcherWindowEnumerator()
{
  mWindowWatcher->RemoveEnumerator(this);
  mWindowWatcher->Release();
}

NS_IMETHODIMP
nsWatcherWindowEnumerator::HasMoreElements(PRBool *retval)
{
  if (!retval)
    return NS_ERROR_INVALID_ARG;

  *retval = mCurrentPosition? PR_TRUE : PR_FALSE;
  return NS_OK;
}
    
NS_IMETHODIMP
nsWatcherWindowEnumerator::GetNext(nsISupports **retval)
{
  if (!retval)
    return NS_ERROR_INVALID_ARG;

  *retval = NULL;

#ifdef USEWEAKREFS
  while (mCurrentPosition) {
    CallQueryReferent(mCurrentPosition->mWindow, retval);
    if (*retval) {
      mCurrentPosition = FindNext();
      break;
    } else // window is gone!
      mWindowWatcher->RemoveWindow(mCurrentPosition);
  }
  NS_IF_ADDREF(*retval);
#else
  if (mCurrentPosition) {
    CallQueryInterface(mCurrentPosition->mWindow, retval);
    mCurrentPosition = FindNext();
  }
#endif
  return NS_OK;
}

nsWatcherWindowEntry *
nsWatcherWindowEnumerator::FindNext()
{
  nsWatcherWindowEntry *info;

  if (!mCurrentPosition)
    return 0;

  info = mCurrentPosition->mYounger;
  return info == mWindowWatcher->mOldestWindow ? 0 : info;
}

// if a window is being removed adjust the iterator's current position
void nsWatcherWindowEnumerator::WindowRemoved(nsWatcherWindowEntry *inInfo) {

  if (mCurrentPosition == inInfo)
    mCurrentPosition = mCurrentPosition != inInfo->mYounger ?
                       inInfo->mYounger : 0;
}

/****************************************************************
 ********************** JSContextAutoPopper *********************
 ****************************************************************/

class JSContextAutoPopper {
public:
  JSContextAutoPopper();
  ~JSContextAutoPopper();

  nsresult   Push(JSContext *cx = nsnull);
  JSContext *get() { return mContext; }

protected:
  nsCOMPtr<nsIThreadJSContextStack>  mService;
  JSContext                         *mContext;
  nsCOMPtr<nsIScriptContext>         mContextKungFuDeathGrip;
};

JSContextAutoPopper::JSContextAutoPopper() : mContext(nsnull)
{
}

JSContextAutoPopper::~JSContextAutoPopper()
{
  JSContext *cx;
  nsresult   rv;

  if(mContext) {
    rv = mService->Pop(&cx);
    NS_ASSERTION(NS_SUCCEEDED(rv) && cx == mContext, "JSContext push/pop mismatch");
  }
}

nsresult JSContextAutoPopper::Push(JSContext *cx)
{
  if (mContext) // only once
    return NS_ERROR_FAILURE;

  mService = do_GetService(sJSStackContractID);
  if(mService) {
    // Get the safe context if we're not provided one.
    if (!cx && NS_FAILED(mService->GetSafeJSContext(&cx))) {
      cx = nsnull;
    }

    // Save cx in mContext to indicate need to pop.
    if (cx && NS_SUCCEEDED(mService->Push(cx))) {
      mContext = cx;
      mContextKungFuDeathGrip = nsWWJSUtils::GetDynamicScriptContext(cx);
    }
  }
  return mContext ? NS_OK : NS_ERROR_FAILURE;
}

/****************************************************************
 *********************** nsWindowWatcher ************************
 ****************************************************************/

NS_IMPL_ADDREF(nsWindowWatcher)
NS_IMPL_RELEASE(nsWindowWatcher)
NS_IMPL_QUERY_INTERFACE4(nsWindowWatcher,
                         nsIWindowWatcher,
                         nsIPromptFactory,
                         nsIAuthPromptAdapterFactory,
                         nsPIWindowWatcher)

nsWindowWatcher::nsWindowWatcher() :
        mEnumeratorList(),
        mOldestWindow(0),
        mActiveWindow(0),
        mListLock(0)
{
}

nsWindowWatcher::~nsWindowWatcher()
{
  // delete data
  while (mOldestWindow)
    RemoveWindow(mOldestWindow);

  if (mListLock)
    PR_DestroyLock(mListLock);
}

nsresult
nsWindowWatcher::Init()
{
  mListLock = PR_NewLock();
  if (!mListLock)
    return NS_ERROR_OUT_OF_MEMORY;
  return NS_OK;
}

NS_IMETHODIMP
nsWindowWatcher::OpenWindow(nsIDOMWindow *aParent,
                            const char *aUrl,
                            const char *aName,
                            const char *aFeatures,
                            nsISupports *aArguments,
                            nsIDOMWindow **_retval)
{
  nsCOMPtr<nsIArray> argsArray;
  PRUint32 argc = 0;
  if (aArguments) {
    // aArguments is allowed to be either an nsISupportsArray or an nsIArray
    // (in which case it is treated as argv) or any other COM object (in which
    // case it becomes argv[0]).
    nsresult rv;

    nsCOMPtr<nsISupportsArray> supArray(do_QueryInterface(aArguments));
    if (!supArray) {
      nsCOMPtr<nsIArray> array(do_QueryInterface(aArguments));
      if (!array) {
        nsCOMPtr<nsIMutableArray> muteArray;
        argsArray = muteArray = do_CreateInstance(NS_ARRAY_CONTRACTID, &rv);
        if (NS_FAILED(rv))
          return rv;
        rv = muteArray->AppendElement(aArguments, PR_FALSE);
        if (NS_FAILED(rv))
          return rv;
        argc = 1;
      } else {
        rv = array->GetLength(&argc);
        if (NS_FAILED(rv))
          return rv;
        if (argc > 0)
          argsArray = array;
      }
    } else {
      // nsISupports array - copy into nsIArray...
      rv = supArray->Count(&argc);
      if (NS_FAILED(rv))
        return rv;
      // But only create an arguments array if there's at least one element in
      // the supports array.
      if (argc > 0) {
        nsCOMPtr<nsIMutableArray> muteArray;
        argsArray = muteArray = do_CreateInstance(NS_ARRAY_CONTRACTID, &rv);
        if (NS_FAILED(rv))
          return rv;
        for (PRUint32 i = 0; i < argc; i++) {
          nsCOMPtr<nsISupports> elt(dont_AddRef(supArray->ElementAt(i)));
          rv = muteArray->AppendElement(elt, PR_FALSE);
          if (NS_FAILED(rv))
            return rv;
        }
      }
    }
  }

  PRBool dialog = (argc != 0);
  return OpenWindowJSInternal(aParent, aUrl, aName, aFeatures, dialog, 
                              argsArray, PR_FALSE, _retval);
}

struct SizeSpec {
  SizeSpec() :
    mLeftSpecified(PR_FALSE),
    mTopSpecified(PR_FALSE),
    mOuterWidthSpecified(PR_FALSE),
    mOuterHeightSpecified(PR_FALSE),
    mInnerWidthSpecified(PR_FALSE),
    mInnerHeightSpecified(PR_FALSE),
    mUseDefaultWidth(PR_FALSE),
    mUseDefaultHeight(PR_FALSE)
  {}
  
  PRInt32 mLeft;
  PRInt32 mTop;
  PRInt32 mOuterWidth;  // Total window width
  PRInt32 mOuterHeight; // Total window height
  PRInt32 mInnerWidth;  // Content area width
  PRInt32 mInnerHeight; // Content area height

  PRPackedBool mLeftSpecified;
  PRPackedBool mTopSpecified;
  PRPackedBool mOuterWidthSpecified;
  PRPackedBool mOuterHeightSpecified;
  PRPackedBool mInnerWidthSpecified;
  PRPackedBool mInnerHeightSpecified;

  // If these booleans are true, don't look at the corresponding width values
  // even if they're specified -- they'll be bogus
  PRPackedBool mUseDefaultWidth;
  PRPackedBool mUseDefaultHeight;

  PRBool PositionSpecified() const {
    return mLeftSpecified || mTopSpecified;
  }
  
  PRBool SizeSpecified() const {
    return mOuterWidthSpecified || mOuterHeightSpecified ||
      mInnerWidthSpecified || mInnerHeightSpecified;
  }
};

NS_IMETHODIMP
nsWindowWatcher::OpenWindowJS(nsIDOMWindow *aParent,
                              const char *aUrl,
                              const char *aName,
                              const char *aFeatures,
                              PRBool aDialog,
                              nsIArray *argv,
                              nsIDOMWindow **_retval)
{
  if (argv) {
    PRUint32 argc;
    nsresult rv = argv->GetLength(&argc);
    NS_ENSURE_SUCCESS(rv, rv);

    // For compatibility with old code, no arguments implies that we shouldn't
    // create an arguments object on the new window at all.
    if (argc == 0)
      argv = nsnull;
  }

  return OpenWindowJSInternal(aParent, aUrl, aName, aFeatures, aDialog,
                              argv, PR_TRUE, _retval);
}

nsresult
nsWindowWatcher::OpenWindowJSInternal(nsIDOMWindow *aParent,
                                      const char *aUrl,
                                      const char *aName,
                                      const char *aFeatures,
                                      PRBool aDialog,
                                      nsIArray *argv,
                                      PRBool aCalledFromJS,
                                      nsIDOMWindow **_retval)
{
  nsresult                        rv = NS_OK;
  PRBool                          nameSpecified,
                                  featuresSpecified,
                                  isNewToplevelWindow = PR_FALSE,
                                  windowIsNew = PR_FALSE,
                                  windowNeedsName = PR_FALSE,
                                  windowIsModal = PR_FALSE,
                                  uriToLoadIsChrome = PR_FALSE,
                                  windowIsModalContentDialog = PR_FALSE;
  PRUint32                        chromeFlags;
  nsAutoString                    name;             // string version of aName
  nsCAutoString                   features;         // string version of aFeatures
  nsCOMPtr<nsIURI>                uriToLoad;        // from aUrl, if any
  nsCOMPtr<nsIDocShellTreeOwner>  parentTreeOwner;  // from the parent window, if any
  nsCOMPtr<nsIDocShellTreeItem>   newDocShellItem;  // from the new window
  JSContextAutoPopper             callerContextGuard;

  NS_ENSURE_ARG_POINTER(_retval);
  *_retval = 0;

  GetWindowTreeOwner(aParent, getter_AddRefs(parentTreeOwner));

  if (aUrl) {
    rv = URIfromURL(aUrl, aParent, getter_AddRefs(uriToLoad));
    if (NS_FAILED(rv))
      return rv;
    uriToLoad->SchemeIs("chrome", &uriToLoadIsChrome);
  }

  nameSpecified = PR_FALSE;
  if (aName) {
    CopyUTF8toUTF16(aName, name);
#ifdef DEBUG
    CheckWindowName(name);
#endif
    nameSpecified = PR_TRUE;
  }

  featuresSpecified = PR_FALSE;
  if (aFeatures) {
    features.Assign(aFeatures);
    featuresSpecified = PR_TRUE;
    features.StripWhitespace();
  }

  // try to find an extant window with the given name
  nsCOMPtr<nsIDOMWindow> foundWindow;
  SafeGetWindowByName(name, aParent, getter_AddRefs(foundWindow));
  GetWindowTreeItem(foundWindow, getter_AddRefs(newDocShellItem));

  // no extant window? make a new one.

  nsCOMPtr<nsIDOMChromeWindow> chromeParent(do_QueryInterface(aParent));

  // Make sure we call CalculateChromeFlags() *before* we push the
  // callee context onto the context stack so that
  // CalculateChromeFlags() sees the actual caller when doing it's
  // security checks.
  chromeFlags = CalculateChromeFlags(features.get(), featuresSpecified,
                                     aDialog, uriToLoadIsChrome,
                                     !aParent || chromeParent);

  if ((chromeFlags & nsIWebBrowserChrome::CHROME_MODAL) &&
      !(chromeFlags & nsIWebBrowserChrome::CHROME_OPENAS_CHROME)) {
    windowIsModalContentDialog = PR_TRUE;
  }

  SizeSpec sizeSpec;
  CalcSizeSpec(features.get(), sizeSpec);

  PRBool isCallerChrome = PR_FALSE;
  nsCOMPtr<nsIScriptSecurityManager>
    sm(do_GetService(NS_SCRIPTSECURITYMANAGER_CONTRACTID));
  if (sm)
    sm->SubjectPrincipalIsSystem(&isCallerChrome);

  JSContext *cx = GetJSContextFromWindow(aParent);

  if (isCallerChrome && !chromeParent && cx) {
    // open() is called from chrome on a non-chrome window, push
    // the context of the callee onto the context stack to
    // prevent the caller's priveleges from leaking into code
    // that runs while opening the new window.

    callerContextGuard.Push(cx);
  }

  if (!newDocShellItem) {
    // We're going to either open up a new window ourselves or ask a
    // nsIWindowProvider for one.  In either case, we'll want to set the right
    // name on it.
    windowNeedsName = PR_TRUE;

    // Now check whether it's ok to ask a window provider for a window.  Don't
    // do it if we're opening a dialog or if our parent is a chrome window or
    // if we're opening something that has modal, dialog, or chrome flags set.
    nsCOMPtr<nsIDOMChromeWindow> chromeWin = do_QueryInterface(aParent);
    if (!aDialog && !chromeWin &&
        !(chromeFlags & (nsIWebBrowserChrome::CHROME_MODAL         |
                         nsIWebBrowserChrome::CHROME_OPENAS_DIALOG | 
                         nsIWebBrowserChrome::CHROME_OPENAS_CHROME))) {
      nsCOMPtr<nsIWindowProvider> provider = do_GetInterface(parentTreeOwner);
      if (provider) {
        NS_ASSERTION(aParent, "We've _got_ to have a parent here!");

        nsCOMPtr<nsIDOMWindow> newWindow;
        rv = provider->ProvideWindow(aParent, chromeFlags,
                                     sizeSpec.PositionSpecified(),
                                     sizeSpec.SizeSpecified(),
                                     uriToLoad, name, features, &windowIsNew,
                                     getter_AddRefs(newWindow));
        if (NS_SUCCEEDED(rv)) {
          GetWindowTreeItem(newWindow, getter_AddRefs(newDocShellItem));
          if (windowIsNew && newDocShellItem) {
            // Make sure to stop any loads happening in this window that the
            // window provider might have started.  Otherwise if our caller
            // manipulates the window it just opened and then the load
            // completes their stuff will get blown away.
            nsCOMPtr<nsIWebNavigation> webNav =
              do_QueryInterface(newDocShellItem);
            webNav->Stop(nsIWebNavigation::STOP_NETWORK);
          }
        }
      }
    }
  }
  
  if (!newDocShellItem) {
    windowIsNew = PR_TRUE;
    isNewToplevelWindow = PR_TRUE;

    nsCOMPtr<nsIWebBrowserChrome> parentChrome(do_GetInterface(parentTreeOwner));

    // is the parent (if any) modal? if so, we must be, too.
    PRBool weAreModal = (chromeFlags & nsIWebBrowserChrome::CHROME_MODAL) != 0;
    if (!weAreModal && parentChrome)
      parentChrome->IsWindowModal(&weAreModal);

    if (weAreModal) {
      windowIsModal = PR_TRUE;
      // in case we added this because weAreModal
      chromeFlags |= nsIWebBrowserChrome::CHROME_MODAL |
        nsIWebBrowserChrome::CHROME_DEPENDENT;
    }

    NS_ASSERTION(mWindowCreator,
                 "attempted to open a new window with no WindowCreator");
    rv = NS_ERROR_FAILURE;
    if (mWindowCreator) {
      nsCOMPtr<nsIWebBrowserChrome> newChrome;

      /* If the window creator is an nsIWindowCreator2, we can give it
         some hints. The only hint at this time is whether the opening window
         is in a situation that's likely to mean this is an unrequested
         popup window we're creating. However we're not completely honest:
         we clear that indicator if the opener is chrome, so that the
         downstream consumer can treat the indicator to mean simply
         that the new window is subject to popup control. */
      nsCOMPtr<nsIWindowCreator2> windowCreator2(do_QueryInterface(mWindowCreator));
      if (windowCreator2) {
        PRUint32 contextFlags = 0;
        PRBool popupConditions = PR_FALSE;

        // is the parent under popup conditions?
        nsCOMPtr<nsPIDOMWindow> piWindow(do_QueryInterface(aParent));
        if (piWindow)
          popupConditions = piWindow->IsLoadingOrRunningTimeout();

        // chrome is always allowed, so clear the flag if the opener is chrome
        if (popupConditions) {
          popupConditions = !isCallerChrome;
        }

        if (popupConditions)
          contextFlags |= nsIWindowCreator2::PARENT_IS_LOADING_OR_RUNNING_TIMEOUT;

        PRBool cancel = PR_FALSE;
        rv = windowCreator2->CreateChromeWindow2(parentChrome, chromeFlags,
                                                 contextFlags, uriToLoad,
                                                 &cancel,
                                                 getter_AddRefs(newChrome));
        if (NS_SUCCEEDED(rv) && cancel) {
          newChrome = 0; // just in case
          rv = NS_ERROR_ABORT;
        }
      }
      else
        rv = mWindowCreator->CreateChromeWindow(parentChrome, chromeFlags,
                                                getter_AddRefs(newChrome));
      if (newChrome) {
        /* It might be a chrome nsXULWindow, in which case it won't have
            an nsIDOMWindow (primary content shell). But in that case, it'll
            be able to hand over an nsIDocShellTreeItem directly. */
        nsCOMPtr<nsIDOMWindow> newWindow(do_GetInterface(newChrome));
        if (newWindow)
          GetWindowTreeItem(newWindow, getter_AddRefs(newDocShellItem));
        if (!newDocShellItem)
          newDocShellItem = do_GetInterface(newChrome);
        if (!newDocShellItem)
          rv = NS_ERROR_FAILURE;
      }
    }
  }

  // better have a window to use by this point
  if (!newDocShellItem)
    return rv;

  nsCOMPtr<nsIDocShell> newDocShell(do_QueryInterface(newDocShellItem));
  NS_ENSURE_TRUE(newDocShell, NS_ERROR_UNEXPECTED);
  
  rv = ReadyOpenedDocShellItem(newDocShellItem, aParent, windowIsNew, _retval);
  if (NS_FAILED(rv))
    return rv;

  /* disable persistence of size/position in popups (determined by
     determining whether the features parameter specifies width or height
     in any way). We consider any overriding of the window's size or position
     in the open call as disabling persistence of those attributes.
     Popup windows (which should not persist size or position) generally set
     the size. */
  if (isNewToplevelWindow) {
    /* at the moment, the strings "height=" or "width=" never happen
       outside a size specification, so we can do this the Q&D way. */

    if (PL_strcasestr(features.get(), "width=") || PL_strcasestr(features.get(), "height=")) {

      nsCOMPtr<nsIDocShellTreeOwner> newTreeOwner;
      newDocShellItem->GetTreeOwner(getter_AddRefs(newTreeOwner));
      if (newTreeOwner)
        newTreeOwner->SetPersistence(PR_FALSE, PR_FALSE, PR_FALSE);
    }
  }

  if ((aDialog || windowIsModalContentDialog) && argv) {
    // Set the args on the new window.
    nsCOMPtr<nsIScriptGlobalObject> scriptGlobal(do_QueryInterface(*_retval));
    NS_ENSURE_TRUE(scriptGlobal, NS_ERROR_UNEXPECTED);
    rv = scriptGlobal->SetNewArguments(argv);
    NS_ENSURE_SUCCESS(rv, rv);
  }

  /* allow a window that we found by name to keep its name (important for cases
     like _self where the given name is different (and invalid)).  Also, _blank
     is not a window name. */
  if (windowNeedsName)
    newDocShellItem->SetName(nameSpecified &&
                             !name.LowerCaseEqualsLiteral("_blank") ?
                             name.get() : nsnull);


  // Inherit the right character set into the new window to use as a fallback
  // in the event the document being loaded does not specify a charset.  When
  // aCalledFromJS is true, we want to use the character set of the document in
  // the caller; otherwise we want to use the character set of aParent's
  // docshell. Failing to set this charset is not fatal, so we want to continue
  // in the face of errors.
  nsCOMPtr<nsIContentViewer> newCV;
  newDocShell->GetContentViewer(getter_AddRefs(newCV));
  nsCOMPtr<nsIMarkupDocumentViewer> newMuCV = do_QueryInterface(newCV);
  if (newMuCV) {
    nsCOMPtr<nsIDocShellTreeItem> parentItem;
    GetWindowTreeItem(aParent, getter_AddRefs(parentItem));

    if (aCalledFromJS) {
      nsCOMPtr<nsIDocShellTreeItem> callerItem = GetCallerTreeItem(parentItem);
      nsCOMPtr<nsPIDOMWindow> callerWin = do_GetInterface(callerItem);
      if (callerWin) {
        nsCOMPtr<nsIDocument> doc =
          do_QueryInterface(callerWin->GetExtantDocument());
        if (doc) {
          newMuCV->SetDefaultCharacterSet(doc->GetDocumentCharacterSet());
        }
      }
    }
    else {
      nsCOMPtr<nsIDocShell> parentDocshell = do_QueryInterface(parentItem);
      // parentDocshell may be null if the parent got closed in the meantime
      if (parentDocshell) {
        nsCOMPtr<nsIContentViewer> parentCV;
        parentDocshell->GetContentViewer(getter_AddRefs(parentCV));
        nsCOMPtr<nsIMarkupDocumentViewer> parentMuCV =
          do_QueryInterface(parentCV);
        if (parentMuCV) {
          nsCAutoString charset;
          nsresult res = parentMuCV->GetDefaultCharacterSet(charset);
          if (NS_SUCCEEDED(res)) {
            newMuCV->SetDefaultCharacterSet(charset);
          }
          res = parentMuCV->GetPrevDocCharacterSet(charset);
          if (NS_SUCCEEDED(res)) {
            newMuCV->SetPrevDocCharacterSet(charset);
          }
        }
      }
    }
  }

  if (isNewToplevelWindow) {
    // Notify observers that the window is open and ready.
    // The window has not yet started to load a document.
    nsCOMPtr<nsIObserverService> obsSvc =
      do_GetService("@mozilla.org/observer-service;1");
    if (obsSvc) {
      obsSvc->NotifyObservers(*_retval, "toplevel-window-ready", nsnull);
    }
  }

  // Now we have to set the right opener principal on the new window.  Note
  // that we have to do this _before_ starting any URI loads, thanks to the
  // sync nature of javascript: loads.  Since this is the only place where we
  // set said opener principal, we need to do it for all URIs, including
  // chrome ones.  So to deal with the mess that is bug 79775, just press on in
  // a reasonable way even if GetSubjectPrincipal fails.  In that case, just
  // use a null subjectPrincipal.
  nsCOMPtr<nsIPrincipal> subjectPrincipal;
  if (NS_FAILED(sm->GetSubjectPrincipal(getter_AddRefs(subjectPrincipal)))) {
    subjectPrincipal = nsnull;
  }

  if (windowIsNew) {
    // Now set the opener principal on the new window.  Note that we need to do
    // this no matter whether we were opened from JS; if there is nothing on
    // the JS stack, just use the principal of our parent window.  In those
    // cases we do _not_ set the parent window principal as the owner of the
    // load--since we really don't know who the owner is, just leave it null.
    nsIPrincipal* newWindowPrincipal = subjectPrincipal;
    if (!newWindowPrincipal && aParent) {
      nsCOMPtr<nsIScriptObjectPrincipal> sop(do_QueryInterface(aParent));
      if (sop) {
        newWindowPrincipal = sop->GetPrincipal();
      }
    }

    PRBool isSystem;
    rv = sm->IsSystemPrincipal(newWindowPrincipal, &isSystem);
    if (NS_FAILED(rv) || isSystem) {
      // Don't pass this principal along to content windows
      PRInt32 itemType;
      rv = newDocShellItem->GetItemType(&itemType);
      if (NS_FAILED(rv) || itemType != nsIDocShellTreeItem::typeChrome) {
        newWindowPrincipal = nsnull;        
      }
    }

    nsCOMPtr<nsPIDOMWindow> newWindow = do_QueryInterface(*_retval);
#ifdef DEBUG
    nsCOMPtr<nsPIDOMWindow> newDebugWindow = do_GetInterface(newDocShell);
    NS_ASSERTION(newWindow == newDebugWindow, "Different windows??");
#endif
    if (newWindow) {
      newWindow->SetOpenerScriptPrincipal(newWindowPrincipal);
    }
  }

  if (uriToLoad) { // get the script principal and pass it to docshell
    JSContextAutoPopper contextGuard;

    cx = GetJSContextFromCallStack();

    // get the security manager
    if (!cx)
      cx = GetJSContextFromWindow(aParent);
    if (!cx) {
      rv = contextGuard.Push();
      if (NS_FAILED(rv))
        return rv;
      cx = contextGuard.get();
    }

    nsCOMPtr<nsIDocShellLoadInfo> loadInfo;
    newDocShell->CreateLoadInfo(getter_AddRefs(loadInfo));
    NS_ENSURE_TRUE(loadInfo, NS_ERROR_FAILURE);

    if (subjectPrincipal) {
      loadInfo->SetOwner(subjectPrincipal);
    }

    // Set the new window's referrer from the calling context's document:

    // get the calling context off the JS context stack
    nsCOMPtr<nsIJSContextStack> stack = do_GetService(sJSStackContractID);

    JSContext* ccx = nsnull;

    // get its document, if any
    if (stack && NS_SUCCEEDED(stack->Peek(&ccx)) && ccx) {
      nsIScriptGlobalObject *sgo = nsWWJSUtils::GetDynamicScriptGlobal(ccx);

      nsCOMPtr<nsPIDOMWindow> w(do_QueryInterface(sgo));
      if (w) {
        /* use the URL from the *extant* document, if any. The usual accessor
           GetDocument will synchronously create an about:blank document if
           it has no better answer, and we only care about a real document.
           Also using GetDocument to force document creation seems to
           screw up focus in the hidden window; see bug 36016.
        */
        nsCOMPtr<nsIDocument> doc(do_QueryInterface(w->GetExtantDocument()));
        if (doc) { 
          // Set the referrer
          loadInfo->SetReferrer(doc->GetDocumentURI());
        }
      }
    }

    newDocShell->LoadURI(uriToLoad, loadInfo,
      windowIsNew ? nsIWebNavigation::LOAD_FLAGS_FIRST_LOAD :
                    nsIWebNavigation::LOAD_FLAGS_NONE, PR_TRUE);
  }

  if (isNewToplevelWindow)
    SizeOpenedDocShellItem(newDocShellItem, aParent, sizeSpec);

  if (windowIsModal || windowIsModalContentDialog) {
    nsCOMPtr<nsIDocShellTreeOwner> newTreeOwner;
    newDocShellItem->GetTreeOwner(getter_AddRefs(newTreeOwner));
    nsCOMPtr<nsIWebBrowserChrome> newChrome(do_GetInterface(newTreeOwner));

    // Throw an exception here if no web browser chrome is available,
    // we need that to show a modal window.
    NS_ENSURE_TRUE(newChrome, NS_ERROR_NOT_AVAILABLE);

    nsCOMPtr<nsPIDOMWindow> modalContentWindow;

    // Dispatch dialog events etc, but we only want to do that if
    // we're opening a modal content window (the helper classes are
    // no-ops if given no window), for chrome dialogs we don't want to
    // do any of that (it's done elsewhere for us).

    if (windowIsModalContentDialog) {
      modalContentWindow = do_QueryInterface(*_retval);
    }

    nsAutoWindowStateHelper windowStateHelper(aParent);

    if (!windowStateHelper.DefaultEnabled()) {
      // Default to cancel not opening the modal window.
      NS_RELEASE(*_retval);

      return NS_OK;
    }

    // Reset popup state while opening a modal dialog, and firing
    // events about the dialog, to prevent the current state from
    // being active the whole time a modal dialog is open.
    nsAutoPopupStatePusher popupStatePusher(modalContentWindow, openAbused);

    newChrome->ShowAsModal();
  }

  return NS_OK;
}

NS_IMETHODIMP
nsWindowWatcher::RegisterNotification(nsIObserver *aObserver)
{
  // just a convenience method; it delegates to nsIObserverService
  nsresult rv;

  if (!aObserver)
    return NS_ERROR_INVALID_ARG;
  
  nsCOMPtr<nsIObserverService> os(do_GetService("@mozilla.org/observer-service;1", &rv));
  if (os) {
    rv = os->AddObserver(aObserver, "domwindowopened", PR_FALSE);
    if (NS_SUCCEEDED(rv))
      rv = os->AddObserver(aObserver, "domwindowclosed", PR_FALSE);
  }
  return rv;
}

NS_IMETHODIMP
nsWindowWatcher::UnregisterNotification(nsIObserver *aObserver)
{
  // just a convenience method; it delegates to nsIObserverService
  nsresult rv;

  if (!aObserver)
    return NS_ERROR_INVALID_ARG;
  
  nsCOMPtr<nsIObserverService> os(do_GetService("@mozilla.org/observer-service;1", &rv));
  if (os) {
    os->RemoveObserver(aObserver, "domwindowopened");
    os->RemoveObserver(aObserver, "domwindowclosed");
  }
  return rv;
}

NS_IMETHODIMP
nsWindowWatcher::GetWindowEnumerator(nsISimpleEnumerator** _retval)
{
  if (!_retval)
    return NS_ERROR_INVALID_ARG;

  nsAutoLock lock(mListLock);
  nsWatcherWindowEnumerator *enumerator = new nsWatcherWindowEnumerator(this);
  if (enumerator)
    return CallQueryInterface(enumerator, _retval);

  return NS_ERROR_OUT_OF_MEMORY;
}
    
NS_IMETHODIMP
nsWindowWatcher::GetNewPrompter(nsIDOMWindow *aParent, nsIPrompt **_retval)
{
  return NS_NewPrompter(_retval, aParent);
}

NS_IMETHODIMP
nsWindowWatcher::GetNewAuthPrompter(nsIDOMWindow *aParent, nsIAuthPrompt **_retval)
{
  return NS_NewAuthPrompter(_retval, aParent);
}

NS_IMETHODIMP
nsWindowWatcher::GetPrompt(nsIDOMWindow *aParent, const nsIID& aIID,
                           void **_retval)
{
  if (aIID.Equals(NS_GET_IID(nsIPrompt)))
    return NS_NewPrompter(reinterpret_cast<nsIPrompt**>(_retval), aParent);
  if (aIID.Equals(NS_GET_IID(nsIAuthPrompt)))
    return NS_NewAuthPrompter(reinterpret_cast<nsIAuthPrompt**>(_retval),
                              aParent);
  if (aIID.Equals(NS_GET_IID(nsIAuthPrompt2))) {
    nsresult rv = NS_NewAuthPrompter2(reinterpret_cast<nsIAuthPrompt2**>
                                                      (_retval),
                                      aParent);
    if (rv == NS_NOINTERFACE) {
      // Return an wrapped nsIAuthPrompt (if we can)
      nsCOMPtr<nsIAuthPrompt> prompt;
      rv = NS_NewAuthPrompter(getter_AddRefs(prompt), aParent);
      if (NS_SUCCEEDED(rv)) {
        NS_WrapAuthPrompt(prompt,
                          reinterpret_cast<nsIAuthPrompt2**>(_retval));
        if (!*_retval)
          rv = NS_ERROR_NOT_AVAILABLE;
      }
    }

    return rv;
  }

  return NS_NOINTERFACE;
}

NS_IMETHODIMP
nsWindowWatcher::CreateAdapter(nsIAuthPrompt* aPrompt, nsIAuthPrompt2** _retval)
{
  *_retval = new AuthPromptWrapper(aPrompt);
  if (!*_retval)
    return NS_ERROR_OUT_OF_MEMORY;
  NS_ADDREF(*_retval);
  return NS_OK;
}

NS_IMETHODIMP
nsWindowWatcher::SetWindowCreator(nsIWindowCreator *creator)
{
  mWindowCreator = creator; // it's an nsCOMPtr, so this is an ownership ref
  return NS_OK;
}

NS_IMETHODIMP
nsWindowWatcher::GetActiveWindow(nsIDOMWindow **aActiveWindow)
{
  if (!aActiveWindow)
    return NS_ERROR_INVALID_ARG;

  *aActiveWindow = mActiveWindow;
  NS_IF_ADDREF(mActiveWindow);
  return NS_OK;
}

NS_IMETHODIMP
nsWindowWatcher::SetActiveWindow(nsIDOMWindow *aActiveWindow)
{
#ifdef DEBUG
  {
    nsCOMPtr<nsPIDOMWindow> win(do_QueryInterface(aActiveWindow));

    NS_ASSERTION(!win || win->IsOuterWindow(),
                 "Uh, the active window must be an outer window!");
  }
#endif

  if (FindWindowEntry(aActiveWindow)) {
    mActiveWindow = aActiveWindow;
    return NS_OK;
  }
  NS_ERROR("invalid active window");
  return NS_ERROR_FAILURE;
}

NS_IMETHODIMP
nsWindowWatcher::AddWindow(nsIDOMWindow *aWindow, nsIWebBrowserChrome *aChrome)
{
  nsresult rv;

  if (!aWindow)
    return NS_ERROR_INVALID_ARG;

#ifdef DEBUG
  {
    nsCOMPtr<nsPIDOMWindow> win(do_QueryInterface(aWindow));

    NS_ASSERTION(win->IsOuterWindow(),
                 "Uh, the active window must be an outer window!");
  }
#endif

  {
    nsWatcherWindowEntry *info;
    nsAutoLock lock(mListLock);

    // if we already have an entry for this window, adjust
    // its chrome mapping and return
    info = FindWindowEntry(aWindow);
    if (info) {
      nsCOMPtr<nsISupportsWeakReference> supportsweak(do_QueryInterface(aChrome));
      if (supportsweak) {
        supportsweak->GetWeakReference(getter_AddRefs(info->mChromeWeak));
      } else {
        info->mChrome = aChrome;
        info->mChromeWeak = 0;
      }
      return NS_OK;
    }
  
    // create a window info struct and add it to the list of windows
    info = new nsWatcherWindowEntry(aWindow, aChrome);
    if (!info)
      return NS_ERROR_OUT_OF_MEMORY;

    if (mOldestWindow)
      info->InsertAfter(mOldestWindow->mOlder);
    else
      mOldestWindow = info;
  } // leave the mListLock

  // a window being added to us signifies a newly opened window.
  // send notifications.
  nsCOMPtr<nsIObserverService> os(do_GetService("@mozilla.org/observer-service;1", &rv));
  if (os) {
    nsCOMPtr<nsISupports> domwin(do_QueryInterface(aWindow));
    rv = os->NotifyObservers(domwin, "domwindowopened", 0);
  }

  return rv;
}

NS_IMETHODIMP
nsWindowWatcher::RemoveWindow(nsIDOMWindow *aWindow)
{
  // find the corresponding nsWatcherWindowEntry, remove it

  if (!aWindow)
    return NS_ERROR_INVALID_ARG;

  nsWatcherWindowEntry *info = FindWindowEntry(aWindow);
  if (info) {
    RemoveWindow(info);
    return NS_OK;
  }
  NS_WARNING("requested removal of nonexistent window");
  return NS_ERROR_INVALID_ARG;
}

nsWatcherWindowEntry *
nsWindowWatcher::FindWindowEntry(nsIDOMWindow *aWindow)
{
  // find the corresponding nsWatcherWindowEntry
  nsWatcherWindowEntry *info,
                       *listEnd;
#ifdef USEWEAKREFS
  nsresult    rv;
  PRBool      found;
#endif

  info = mOldestWindow;
  listEnd = 0;
#ifdef USEWEAKREFS
  rv = NS_OK;
  found = PR_FALSE;
  while (info != listEnd && NS_SUCCEEDED(rv)) {
    nsCOMPtr<nsIDOMWindow> infoWindow(do_QueryReferent(info->mWindow));
    if (!infoWindow) { // clean up dangling reference, while we're here
      rv = RemoveWindow(info);
    }
    else if (infoWindow.get() == aWindow)
      return info;

    info = info->mYounger;
    listEnd = mOldestWindow;
  }
  return 0;
#else
  while (info != listEnd) {
    if (info->mWindow == aWindow)
      return info;
    info = info->mYounger;
    listEnd = mOldestWindow;
  }
  return 0;
#endif
}

nsresult nsWindowWatcher::RemoveWindow(nsWatcherWindowEntry *inInfo)
{
  PRInt32  ctr,
           count = mEnumeratorList.Count();
  nsresult rv;

  {
    // notify the enumerators
    nsAutoLock lock(mListLock);
    for (ctr = 0; ctr < count; ++ctr) 
      ((nsWatcherWindowEnumerator*)mEnumeratorList[ctr])->WindowRemoved(inInfo);

    // remove the element from the list
    if (inInfo == mOldestWindow)
      mOldestWindow = inInfo->mYounger == mOldestWindow ? 0 : inInfo->mYounger;
    inInfo->Unlink();

    // clear the active window, if they're the same
    if (mActiveWindow == inInfo->mWindow)
      mActiveWindow = 0;
  }

  // a window being removed from us signifies a newly closed window.
  // send notifications.
  nsCOMPtr<nsIObserverService> os(do_GetService("@mozilla.org/observer-service;1", &rv));
  if (os) {
#ifdef USEWEAKREFS
    nsCOMPtr<nsISupports> domwin(do_QueryReferent(inInfo->mWindow));
    if (domwin)
      rv = os->NotifyObservers(domwin, "domwindowclosed", 0);
    // else bummer. since the window is gone, there's nothing to notify with.
#else
    nsCOMPtr<nsISupports> domwin(do_QueryInterface(inInfo->mWindow));
    rv = os->NotifyObservers(domwin, "domwindowclosed", 0);
#endif
  }

  delete inInfo;
  return NS_OK;
}

NS_IMETHODIMP
nsWindowWatcher::GetChromeForWindow(nsIDOMWindow *aWindow, nsIWebBrowserChrome **_retval)
{
  if (!aWindow || !_retval)
    return NS_ERROR_INVALID_ARG;
  *_retval = 0;

  nsAutoLock lock(mListLock);
  nsWatcherWindowEntry *info = FindWindowEntry(aWindow);
  if (info) {
    if (info->mChromeWeak != nsnull) {
      return info->mChromeWeak->
                            QueryReferent(NS_GET_IID(nsIWebBrowserChrome),
                                          reinterpret_cast<void**>(_retval));
    }
    *_retval = info->mChrome;
    NS_IF_ADDREF(*_retval);
  }
  return NS_OK;
}

NS_IMETHODIMP
nsWindowWatcher::GetWindowByName(const PRUnichar *aTargetName, 
                                 nsIDOMWindow *aCurrentWindow,
                                 nsIDOMWindow **aResult)
{
  if (!aResult) {
    return NS_ERROR_INVALID_ARG;
  }

  *aResult = nsnull;

  nsCOMPtr<nsIDocShellTreeItem> treeItem;

  nsCOMPtr<nsIDocShellTreeItem> startItem;
  GetWindowTreeItem(aCurrentWindow, getter_AddRefs(startItem));
  if (startItem) {
    // Note: original requestor is null here, per idl comments
    startItem->FindItemWithName(aTargetName, nsnull, nsnull,
                                getter_AddRefs(treeItem));
  }
  else {
    // Note: original requestor is null here, per idl comments
    FindItemWithName(aTargetName, nsnull, nsnull, getter_AddRefs(treeItem));
  }

  nsCOMPtr<nsIDOMWindow> domWindow = do_GetInterface(treeItem);
  domWindow.swap(*aResult);

  return NS_OK;
}

PRBool
nsWindowWatcher::AddEnumerator(nsWatcherWindowEnumerator* inEnumerator)
{
  // (requires a lock; assumes it's called by someone holding the lock)
  return mEnumeratorList.AppendElement(inEnumerator);
}

PRBool
nsWindowWatcher::RemoveEnumerator(nsWatcherWindowEnumerator* inEnumerator)
{
  // (requires a lock; assumes it's called by someone holding the lock)
  return mEnumeratorList.RemoveElement(inEnumerator);
}

nsresult
nsWindowWatcher::URIfromURL(const char *aURL,
                            nsIDOMWindow *aParent,
                            nsIURI **aURI)
{
  nsCOMPtr<nsIDOMWindow> baseWindow;

  /* build the URI relative to the calling JS Context, if any.
     (note this is the same context used to make the security check
     in nsGlobalWindow.cpp.) */
  JSContext *cx = GetJSContextFromCallStack();
  if (cx) {
    nsIScriptContext *scriptcx = nsWWJSUtils::GetDynamicScriptContext(cx);
    if (scriptcx) {
      baseWindow = do_QueryInterface(scriptcx->GetGlobalObject());
    }
  }

  // failing that, build it relative to the parent window, if possible
  if (!baseWindow)
    baseWindow = aParent;

  // failing that, use the given URL unmodified. It had better not be relative.

  nsIURI *baseURI = nsnull;

  // get baseWindow's document URI
  if (baseWindow) {
    nsCOMPtr<nsIDOMDocument> domDoc;
    baseWindow->GetDocument(getter_AddRefs(domDoc));
    if (domDoc) {
      nsCOMPtr<nsIDocument> doc;
      doc = do_QueryInterface(domDoc);
      if (doc) {
        baseURI = doc->GetBaseURI();
      }
    }
  }

  // build and return the absolute URI
  return NS_NewURI(aURI, aURL, baseURI);
}

#ifdef DEBUG
/* Check for an illegal name e.g. frame3.1
   This just prints a warning message an continues; we open the window anyway,
   (see bug 32898). */
void nsWindowWatcher::CheckWindowName(nsString& aName)
{
  nsReadingIterator<PRUnichar> scan;
  nsReadingIterator<PRUnichar> endScan;

  aName.EndReading(endScan);
  for (aName.BeginReading(scan); scan != endScan; ++scan)
    if (!nsCRT::IsAsciiAlpha(*scan) && !nsCRT::IsAsciiDigit(*scan) &&
        *scan != '_') {

      // Don't use js_ReportError as this will cause the application
      // to shut down (JS_ASSERT calls abort())  See bug 32898
      nsCAutoString warn;
      warn.AssignLiteral("Illegal character in window name ");
      AppendUTF16toUTF8(aName, warn);
      NS_WARNING(warn.get());
      break;
    }
}
#endif // DEBUG

#define NS_CALCULATE_CHROME_FLAG_FOR(feature, flag)               \
    prefBranch->GetBoolPref(feature, &forceEnable);               \
    if (forceEnable && !(aDialog && isChrome) &&                  \
        !(isChrome && aHasChromeParent) && !aChromeURL) {         \
      chromeFlags |= flag;                                        \
    } else {                                                      \
      chromeFlags |= WinHasOption(aFeatures, feature,             \
                                  0, &presenceFlag)               \
                     ? flag : 0;                                  \
    }

/**
 * Calculate the chrome bitmask from a string list of features.
 * @param aFeatures a string containing a list of named chrome features
 * @param aNullFeatures true if aFeatures was a null pointer (which fact
 *                      is lost by its conversion to a string in the caller)
 * @param aDialog affects the assumptions made about unnamed features
 * @return the chrome bitmask
 */
// static
PRUint32 nsWindowWatcher::CalculateChromeFlags(const char *aFeatures,
                                               PRBool aFeaturesSpecified,
                                               PRBool aDialog,
                                               PRBool aChromeURL,
                                               PRBool aHasChromeParent)
{
   if(!aFeaturesSpecified || !aFeatures) {
      if(aDialog)
         return nsIWebBrowserChrome::CHROME_ALL | 
                nsIWebBrowserChrome::CHROME_OPENAS_DIALOG | 
                nsIWebBrowserChrome::CHROME_OPENAS_CHROME;
      else
         return nsIWebBrowserChrome::CHROME_ALL;
   }

  /* This function has become complicated since browser windows and
     dialogs diverged. The difference is, browser windows assume all
     chrome not explicitly mentioned is off, if the features string
     is not null. Exceptions are some OS border chrome new with Mozilla.
     Dialogs interpret a (mostly) empty features string to mean
     "OS's choice," and also support an "all" flag explicitly disallowed
     in the standards-compliant window.(normal)open. */

  PRUint32 chromeFlags = 0;
  PRBool presenceFlag = PR_FALSE;

  chromeFlags = nsIWebBrowserChrome::CHROME_WINDOW_BORDERS;
  if (aDialog && WinHasOption(aFeatures, "all", 0, &presenceFlag))
    chromeFlags = nsIWebBrowserChrome::CHROME_ALL;

  /* Next, allow explicitly named options to override the initial settings */

  nsCOMPtr<nsIScriptSecurityManager>
    securityManager(do_GetService(NS_SCRIPTSECURITYMANAGER_CONTRACTID));
  NS_ENSURE_TRUE(securityManager, NS_ERROR_FAILURE);

  PRBool isChrome = PR_FALSE;
  securityManager->SubjectPrincipalIsSystem(&isChrome);

  nsCOMPtr<nsIPrefBranch> prefBranch;
  nsresult rv;
  nsCOMPtr<nsIPrefService> prefs = do_GetService(NS_PREFSERVICE_CONTRACTID, &rv);
  NS_ENSURE_SUCCESS(rv, PR_TRUE);

  rv = prefs->GetBranch("dom.disable_window_open_feature.", getter_AddRefs(prefBranch));
  NS_ENSURE_SUCCESS(rv, PR_TRUE);

  PRBool forceEnable = PR_FALSE;

  NS_CALCULATE_CHROME_FLAG_FOR("titlebar",
                               nsIWebBrowserChrome::CHROME_TITLEBAR);
  NS_CALCULATE_CHROME_FLAG_FOR("close",
                               nsIWebBrowserChrome::CHROME_WINDOW_CLOSE);
  NS_CALCULATE_CHROME_FLAG_FOR("toolbar",
                               nsIWebBrowserChrome::CHROME_TOOLBAR);
  NS_CALCULATE_CHROME_FLAG_FOR("location",
                               nsIWebBrowserChrome::CHROME_LOCATIONBAR);
  NS_CALCULATE_CHROME_FLAG_FOR("directories",
                               nsIWebBrowserChrome::CHROME_PERSONAL_TOOLBAR);
  NS_CALCULATE_CHROME_FLAG_FOR("personalbar",
                               nsIWebBrowserChrome::CHROME_PERSONAL_TOOLBAR);
  NS_CALCULATE_CHROME_FLAG_FOR("status",
                               nsIWebBrowserChrome::CHROME_STATUSBAR);
  NS_CALCULATE_CHROME_FLAG_FOR("menubar",
                               nsIWebBrowserChrome::CHROME_MENUBAR);
  NS_CALCULATE_CHROME_FLAG_FOR("scrollbars",
                               nsIWebBrowserChrome::CHROME_SCROLLBARS);
  NS_CALCULATE_CHROME_FLAG_FOR("resizable",
                               nsIWebBrowserChrome::CHROME_WINDOW_RESIZE);
  NS_CALCULATE_CHROME_FLAG_FOR("minimizable",
                               nsIWebBrowserChrome::CHROME_WINDOW_MIN);

  chromeFlags |= WinHasOption(aFeatures, "popup", 0, &presenceFlag)
                 ? nsIWebBrowserChrome::CHROME_WINDOW_POPUP : 0; 

  /* OK.
     Normal browser windows, in spite of a stated pattern of turning off
     all chrome not mentioned explicitly, will want the new OS chrome (window
     borders, titlebars, closebox) on, unless explicitly turned off.
     Dialogs, on the other hand, take the absence of any explicit settings
     to mean "OS' choice." */

  // default titlebar and closebox to "on," if not mentioned at all
  if (!PL_strcasestr(aFeatures, "titlebar"))
    chromeFlags |= nsIWebBrowserChrome::CHROME_TITLEBAR;
  if (!PL_strcasestr(aFeatures, "close"))
    chromeFlags |= nsIWebBrowserChrome::CHROME_WINDOW_CLOSE;

  if (aDialog && !presenceFlag)
    chromeFlags = nsIWebBrowserChrome::CHROME_DEFAULT;

  /* Finally, once all the above normal chrome has been divined, deal
     with the features that are more operating hints than appearance
     instructions. (Note modality implies dependence.) */

  if (WinHasOption(aFeatures, "alwaysLowered", 0, nsnull) ||
      WinHasOption(aFeatures, "z-lock", 0, nsnull))
    chromeFlags |= nsIWebBrowserChrome::CHROME_WINDOW_LOWERED;
  else if (WinHasOption(aFeatures, "alwaysRaised", 0, nsnull))
    chromeFlags |= nsIWebBrowserChrome::CHROME_WINDOW_RAISED;

  chromeFlags |= WinHasOption(aFeatures, "chrome", 0, nsnull) ?
    nsIWebBrowserChrome::CHROME_OPENAS_CHROME : 0;
  chromeFlags |= WinHasOption(aFeatures, "extrachrome", 0, nsnull) ?
    nsIWebBrowserChrome::CHROME_EXTRA : 0;
  chromeFlags |= WinHasOption(aFeatures, "centerscreen", 0, nsnull) ?
    nsIWebBrowserChrome::CHROME_CENTER_SCREEN : 0;
  chromeFlags |= WinHasOption(aFeatures, "dependent", 0, nsnull) ?
    nsIWebBrowserChrome::CHROME_DEPENDENT : 0;
  chromeFlags |= WinHasOption(aFeatures, "modal", 0, nsnull) ?
    (nsIWebBrowserChrome::CHROME_MODAL | nsIWebBrowserChrome::CHROME_DEPENDENT) : 0;
  chromeFlags |= WinHasOption(aFeatures, "dialog", 0, nsnull) ?
    nsIWebBrowserChrome::CHROME_OPENAS_DIALOG : 0;

  /* and dialogs need to have the last word. assume dialogs are dialogs,
     and opened as chrome, unless explicitly told otherwise. */
  if (aDialog) {
    if (!PL_strcasestr(aFeatures, "dialog"))
      chromeFlags |= nsIWebBrowserChrome::CHROME_OPENAS_DIALOG;
    if (!PL_strcasestr(aFeatures, "chrome"))
      chromeFlags |= nsIWebBrowserChrome::CHROME_OPENAS_CHROME;
  }

  /* missing
     chromeFlags->copy_history
   */

  // Check security state for use in determing window dimensions
  PRBool enabled;
  nsresult res =
    securityManager->IsCapabilityEnabled("UniversalBrowserWrite", &enabled);

  if (NS_FAILED(res) || !enabled || (isChrome && !aHasChromeParent)) {
    // If priv check fails (or if we're called from chrome, but the
    // parent is not a chrome window), set all elements to minimum
    // reqs., else leave them alone.
    chromeFlags |= nsIWebBrowserChrome::CHROME_TITLEBAR;
    chromeFlags |= nsIWebBrowserChrome::CHROME_WINDOW_CLOSE;
    chromeFlags &= ~nsIWebBrowserChrome::CHROME_WINDOW_LOWERED;
    chromeFlags &= ~nsIWebBrowserChrome::CHROME_WINDOW_RAISED;
    chromeFlags &= ~nsIWebBrowserChrome::CHROME_WINDOW_POPUP;
    /* Untrusted script is allowed to pose modal windows with a chrome
       scheme. This check could stand to be better. But it effectively
       prevents untrusted script from opening modal windows in general
       while still allowing alerts and the like. */
    if (!aChromeURL)
      chromeFlags &= ~nsIWebBrowserChrome::CHROME_OPENAS_CHROME;
  }

  if (!(chromeFlags & nsIWebBrowserChrome::CHROME_OPENAS_CHROME)) {
    // Remove the dependent flag if we're not opening as chrome
    chromeFlags &= ~nsIWebBrowserChrome::CHROME_DEPENDENT;
  }

  return chromeFlags;
}

// static
PRInt32
nsWindowWatcher::WinHasOption(const char *aOptions, const char *aName,
                              PRInt32 aDefault, PRBool *aPresenceFlag)
{
  if (!aOptions)
    return 0;

  char *comma, *equal;
  PRInt32 found = 0;

#ifdef DEBUG
    nsCAutoString options(aOptions);
    NS_ASSERTION(options.FindCharInSet(" \n\r\t") == kNotFound, 
                  "There should be no whitespace in this string!");
#endif

  while (PR_TRUE) {
    comma = PL_strchr(aOptions, ',');
    if (comma)
      *comma = '\0';
    equal = PL_strchr(aOptions, '=');
    if (equal)
      *equal = '\0';
    if (nsCRT::strcasecmp(aOptions, aName) == 0) {
      if (aPresenceFlag)
        *aPresenceFlag = PR_TRUE;
      if (equal)
        if (*(equal + 1) == '*')
          found = aDefault;
        else if (nsCRT::strcasecmp(equal + 1, "yes") == 0)
          found = 1;
        else
          found = atoi(equal + 1);
      else
        found = 1;
    }
    if (equal)
      *equal = '=';
    if (comma)
      *comma = ',';
    if (found || !comma)
      break;
    aOptions = comma + 1;
  }
  return found;
}

/* try to find an nsIDocShellTreeItem with the given name in any
   known open window. a failure to find the item will not
   necessarily return a failure method value. check aFoundItem.
*/
NS_IMETHODIMP
nsWindowWatcher::FindItemWithName(const PRUnichar* aName,
                                  nsIDocShellTreeItem* aRequestor,
                                  nsIDocShellTreeItem* aOriginalRequestor,
                                  nsIDocShellTreeItem** aFoundItem)
{
  *aFoundItem = 0;

  /* special cases */
  if(!aName || !*aName)
    return NS_OK;

  nsDependentString name(aName);
  
  nsCOMPtr<nsISimpleEnumerator> windows;
  GetWindowEnumerator(getter_AddRefs(windows));
  if (!windows)
    return NS_ERROR_FAILURE;

  PRBool   more;
  nsresult rv = NS_OK;

  do {
    windows->HasMoreElements(&more);
    if (!more)
      break;
    nsCOMPtr<nsISupports> nextSupWindow;
    windows->GetNext(getter_AddRefs(nextSupWindow));
    nsCOMPtr<nsIDOMWindow> nextWindow(do_QueryInterface(nextSupWindow));
    if (nextWindow) {
      nsCOMPtr<nsIDocShellTreeItem> treeItem;
      GetWindowTreeItem(nextWindow, getter_AddRefs(treeItem));
      if (treeItem) {
        // Get the root tree item of same type, since roots are the only
        // things that call into the treeowner to look for named items.
        nsCOMPtr<nsIDocShellTreeItem> root;
        treeItem->GetSameTypeRootTreeItem(getter_AddRefs(root));
        NS_ASSERTION(root, "Must have root tree item of same type");
        // Make sure not to call back into aRequestor
        if (root != aRequestor) {
          // Get the tree owner so we can pass it in as the requestor so
          // the child knows not to call back up, since we're walking
          // all windows already.
          nsCOMPtr<nsIDocShellTreeOwner> rootOwner;
          // Note: if we have no aRequestor, then we want to also look for
          // "special" window names, so pass a null requestor.  This will mean
          // that the treeitem calls back up to us, effectively (with a
          // non-null aRequestor), so break the loop immediately after the
          // call in that case.
          if (aRequestor) {
            root->GetTreeOwner(getter_AddRefs(rootOwner));
          }
          rv = root->FindItemWithName(aName, rootOwner, aOriginalRequestor,
                                      aFoundItem);
          if (NS_FAILED(rv) || *aFoundItem || !aRequestor)
            break;
        }
      }
    }
  } while(1);

  return rv;
}

already_AddRefed<nsIDocShellTreeItem>
nsWindowWatcher::GetCallerTreeItem(nsIDocShellTreeItem* aParentItem)
{
  nsCOMPtr<nsIJSContextStack> stack =
    do_GetService(sJSStackContractID);

  JSContext *cx = nsnull;

  if (stack) {
    stack->Peek(&cx);
  }

  nsIDocShellTreeItem* callerItem = nsnull;

  if (cx) {
    nsCOMPtr<nsIWebNavigation> callerWebNav =
      do_GetInterface(nsWWJSUtils::GetDynamicScriptGlobal(cx));

    if (callerWebNav) {
      CallQueryInterface(callerWebNav, &callerItem);
    }
  }

  if (!callerItem) {
    NS_IF_ADDREF(callerItem = aParentItem);
  }

  return callerItem;
}

nsresult
nsWindowWatcher::SafeGetWindowByName(const nsAString& aName,
                                     nsIDOMWindow* aCurrentWindow,
                                     nsIDOMWindow** aResult)
{
  *aResult = nsnull;
  
  nsCOMPtr<nsIDocShellTreeItem> startItem;
  GetWindowTreeItem(aCurrentWindow, getter_AddRefs(startItem));

  nsCOMPtr<nsIDocShellTreeItem> callerItem = GetCallerTreeItem(startItem);

  const nsAFlatString& flatName = PromiseFlatString(aName);

  nsCOMPtr<nsIDocShellTreeItem> foundItem;
  if (startItem) {
    startItem->FindItemWithName(flatName.get(), nsnull, callerItem,
                                getter_AddRefs(foundItem));
  }
  else {
    FindItemWithName(flatName.get(), nsnull, callerItem,
                     getter_AddRefs(foundItem));
  }

  nsCOMPtr<nsIDOMWindow> foundWin = do_GetInterface(foundItem);
  foundWin.swap(*aResult);
  return NS_OK;
}

/* Fetch the nsIDOMWindow corresponding to the given nsIDocShellTreeItem.
   This forces the creation of a script context, if one has not already
   been created. Note it also sets the window's opener to the parent,
   if applicable -- because it's just convenient, that's all. null aParent
   is acceptable. */
nsresult
nsWindowWatcher::ReadyOpenedDocShellItem(nsIDocShellTreeItem *aOpenedItem,
                                         nsIDOMWindow        *aParent,
                                         PRBool              aWindowIsNew,
                                         nsIDOMWindow        **aOpenedWindow)
{
  nsresult rv = NS_ERROR_FAILURE;

  *aOpenedWindow = 0;
  nsCOMPtr<nsPIDOMWindow> piOpenedWindow(do_GetInterface(aOpenedItem));
  if (piOpenedWindow) {
    if (aParent) {
      nsCOMPtr<nsIDOMWindowInternal> internalParent(do_QueryInterface(aParent));
      piOpenedWindow->SetOpenerWindow(internalParent, aWindowIsNew); // damnit

      if (aWindowIsNew) {
#ifdef DEBUG
        // Assert that we're not loading things right now.  If we are, when
        // that load completes it will clobber whatever principals we set up
        // on this new window!
        nsCOMPtr<nsIDocumentLoader> docloader =
          do_QueryInterface(aOpenedItem);
        NS_ASSERTION(docloader, "How can we not have a docloader here?");

        nsCOMPtr<nsIChannel> chan;
        docloader->GetDocumentChannel(getter_AddRefs(chan));
        NS_ASSERTION(!chan, "Why is there a document channel?");
#endif

        nsCOMPtr<nsIDocument> doc =
          do_QueryInterface(piOpenedWindow->GetExtantDocument());
        if (doc) {
          doc->SetIsInitialDocument(PR_TRUE);
        }
      }
    }
    rv = CallQueryInterface(piOpenedWindow, aOpenedWindow);
  }
  return rv;
}

// static
void
nsWindowWatcher::CalcSizeSpec(const char* aFeatures, SizeSpec& aResult)
{
  // Parse position spec, if any, from aFeatures
  PRBool  present;
  PRInt32 temp;

  present = PR_FALSE;
  if ((temp = WinHasOption(aFeatures, "left", 0, &present)) || present)
    aResult.mLeft = temp;
  else if ((temp = WinHasOption(aFeatures, "screenX", 0, &present)) || present)
    aResult.mLeft = temp;
  aResult.mLeftSpecified = present;

  present = PR_FALSE;
  if ((temp = WinHasOption(aFeatures, "top", 0, &present)) || present)
    aResult.mTop = temp;
  else if ((temp = WinHasOption(aFeatures, "screenY", 0, &present)) || present)
    aResult.mTop = temp;
  aResult.mTopSpecified = present;

  // Parse size spec, if any. Chrome size overrides content size.
  if ((temp = WinHasOption(aFeatures, "outerWidth", PR_INT32_MIN, nsnull))) {
    if (temp == PR_INT32_MIN) {
      aResult.mUseDefaultWidth = PR_TRUE;
    }
    else {
      aResult.mOuterWidth = temp;
    }
    aResult.mOuterWidthSpecified = PR_TRUE;
  } else if ((temp = WinHasOption(aFeatures, "width", PR_INT32_MIN, nsnull)) ||
             (temp = WinHasOption(aFeatures, "innerWidth", PR_INT32_MIN,
                                  nsnull))) {
    if (temp == PR_INT32_MIN) {
      aResult.mUseDefaultWidth = PR_TRUE;
    } else {
      aResult.mInnerWidth = temp;
    }
    aResult.mInnerWidthSpecified = PR_TRUE;
  }

  if ((temp = WinHasOption(aFeatures, "outerHeight", PR_INT32_MIN, nsnull))) {
    if (temp == PR_INT32_MIN) {
      aResult.mUseDefaultHeight = PR_TRUE;
    }
    else {
      aResult.mOuterHeight = temp;
    }
    aResult.mOuterHeightSpecified = PR_TRUE;
  } else if ((temp = WinHasOption(aFeatures, "height", PR_INT32_MIN,
                                  nsnull)) ||
             (temp = WinHasOption(aFeatures, "innerHeight", PR_INT32_MIN,
                                  nsnull))) {
    if (temp == PR_INT32_MIN) {
      aResult.mUseDefaultHeight = PR_TRUE;
    } else {
      aResult.mInnerHeight = temp;
    }
    aResult.mInnerHeightSpecified = PR_TRUE;
  }
}

/* Size and position the new window according to aSizeSpec. This method
   is assumed to be called after the window has already been given
   a default position and size; thus its current position and size are
   accurate defaults. The new window is made visible at method end.
*/
void
nsWindowWatcher::SizeOpenedDocShellItem(nsIDocShellTreeItem *aDocShellItem,
                                        nsIDOMWindow *aParent,
                                        const SizeSpec & aSizeSpec)
{
  // position and size of window
  PRInt32 left = 0,
          top = 0,
          width = 100,
          height = 100;
  // difference between chrome and content size
  PRInt32 chromeWidth = 0,
          chromeHeight = 0;
  // whether the window size spec refers to chrome or content
  PRBool  sizeChromeWidth = PR_TRUE,
          sizeChromeHeight = PR_TRUE;

  // get various interfaces for aDocShellItem, used throughout this method
  nsCOMPtr<nsIDocShellTreeOwner> treeOwner;
  aDocShellItem->GetTreeOwner(getter_AddRefs(treeOwner));
  nsCOMPtr<nsIBaseWindow> treeOwnerAsWin(do_QueryInterface(treeOwner));
  if (!treeOwnerAsWin) // we'll need this to actually size the docshell
    return;
    
  float devPixelsPerCSSPixel = 1.0;
  nsCOMPtr<nsIWidget> mainWidget;
  treeOwnerAsWin->GetMainWidget(getter_AddRefs(mainWidget));
  if (!mainWidget) {
    // Some embedding clients don't support nsIDocShellTreeOwner's
    // GetMainWidget, so try going through nsIBaseWindow's GetParentWidget
    nsCOMPtr<nsIBaseWindow> shellWindow(do_QueryInterface(aDocShellItem));
    if (shellWindow)
      shellWindow->GetParentWidget(getter_AddRefs(mainWidget));
  }
  if (mainWidget) {
    nsCOMPtr<nsIDeviceContext> ctx = mainWidget->GetDeviceContext();
    devPixelsPerCSSPixel = float(ctx->AppUnitsPerCSSPixel()) / ctx->AppUnitsPerDevPixel();
  }

  /* The current position and size will be unchanged if not specified
     (and they fit entirely onscreen). Also, calculate the difference
     between chrome and content sizes on aDocShellItem's window.
     This latter point becomes important if chrome and content
     specifications are mixed in aFeatures, and when bringing the window
     back from too far off the right or bottom edges of the screen. */

  treeOwnerAsWin->GetPositionAndSize(&left, &top, &width, &height);
  { // scope shellWindow why not
    nsCOMPtr<nsIBaseWindow> shellWindow(do_QueryInterface(aDocShellItem));
    if (shellWindow) {
      PRInt32 cox, coy;
      shellWindow->GetSize(&cox, &coy);
      chromeWidth = width - cox;
      chromeHeight = height - coy;
    }
  }

  // Set up left/top
  if (aSizeSpec.mLeftSpecified) {
    left = NSToIntRound(aSizeSpec.mLeft * devPixelsPerCSSPixel);
  }

  if (aSizeSpec.mTopSpecified) {
    top = NSToIntRound(aSizeSpec.mTop * devPixelsPerCSSPixel);
  }

  // Set up width
  if (aSizeSpec.mOuterWidthSpecified) {
    if (!aSizeSpec.mUseDefaultWidth) {
      width = NSToIntRound(aSizeSpec.mOuterWidth * devPixelsPerCSSPixel);
    } // Else specified to default; just use our existing width
  }
  else if (aSizeSpec.mInnerWidthSpecified) {
    sizeChromeWidth = PR_FALSE;
    if (aSizeSpec.mUseDefaultWidth) {
      width = width - chromeWidth;
    } else {
      width = NSToIntRound(aSizeSpec.mInnerWidth * devPixelsPerCSSPixel);
    }
  }

  // Set up height
  if (aSizeSpec.mOuterHeightSpecified) {
    if (!aSizeSpec.mUseDefaultHeight) {
      height = NSToIntRound(aSizeSpec.mOuterHeight * devPixelsPerCSSPixel);
    } // Else specified to default; just use our existing height
  }
  else if (aSizeSpec.mInnerHeightSpecified) {
    sizeChromeHeight = PR_FALSE;
    if (aSizeSpec.mUseDefaultHeight) {
      height = height - chromeHeight;
    } else {
      height = NSToIntRound(aSizeSpec.mInnerHeight * devPixelsPerCSSPixel);
    }
  }

  PRBool positionSpecified = aSizeSpec.PositionSpecified();
  
  nsresult res;
  PRBool enabled = PR_FALSE;

  // Check security state for use in determing window dimensions
  nsCOMPtr<nsIScriptSecurityManager>
    securityManager(do_GetService(NS_SCRIPTSECURITYMANAGER_CONTRACTID));
  if (securityManager) {
    res = securityManager->IsCapabilityEnabled("UniversalBrowserWrite",
                                               &enabled);
    if (NS_FAILED(res))
      enabled = PR_FALSE;
    else if (enabled && aParent) {
      nsCOMPtr<nsIDOMChromeWindow> chromeWin(do_QueryInterface(aParent));

      PRBool isChrome = PR_FALSE;
      securityManager->SubjectPrincipalIsSystem(&isChrome);

      // Only enable special priveleges for chrome when chrome calls
      // open() on a chrome window
      enabled = !(isChrome && chromeWin == nsnull);
    }
  }

  if (!enabled) {

    // Security check failed.  Ensure all args meet minimum reqs.

    PRInt32 oldTop = top,
            oldLeft = left;

    // We'll also need the screen dimensions
    nsCOMPtr<nsIScreen> screen;
    nsCOMPtr<nsIScreenManager> screenMgr(do_GetService(
                                         "@mozilla.org/gfx/screenmanager;1"));
    if (screenMgr)
      screenMgr->ScreenForRect(left, top, width, height,
                               getter_AddRefs(screen));
    if (screen) {
      PRInt32 screenLeft, screenTop, screenWidth, screenHeight;
      PRInt32 winWidth = width + (sizeChromeWidth ? 0 : chromeWidth),
              winHeight = height + (sizeChromeHeight ? 0 : chromeHeight);

      screen->GetAvailRect(&screenLeft, &screenTop,
                           &screenWidth, &screenHeight);

      if (aSizeSpec.SizeSpecified()) {
        /* Unlike position, force size out-of-bounds check only if
           size actually was specified. Otherwise, intrinsically sized
           windows are broken. */
        if (height < 100)
          height = 100;
        if (winHeight > screenHeight)
          height = screenHeight - (sizeChromeHeight ? 0 : chromeHeight);
        if (width < 100)
          width = 100;
        if (winWidth > screenWidth)
          width = screenWidth - (sizeChromeWidth ? 0 : chromeWidth);
      }

      if (left+winWidth > screenLeft+screenWidth)
        left = screenLeft+screenWidth - winWidth;
      if (left < screenLeft)
        left = screenLeft;
      if (top+winHeight > screenTop+screenHeight)
        top = screenTop+screenHeight - winHeight;
      if (top < screenTop)
        top = screenTop;
      if (top != oldTop || left != oldLeft)
        positionSpecified = PR_TRUE;
    }
  }

  // size and position the window

  if (positionSpecified)
    treeOwnerAsWin->SetPosition(left, top);
  if (aSizeSpec.SizeSpecified()) {
    /* Prefer to trust the interfaces, which think in terms of pure
       chrome or content sizes. If we have a mix, use the chrome size
       adjusted by the chrome/content differences calculated earlier. */
    if (!sizeChromeWidth && !sizeChromeHeight)
      treeOwner->SizeShellTo(aDocShellItem, width, height);
    else {
      if (!sizeChromeWidth)
        width += chromeWidth;
      if (!sizeChromeHeight)
        height += chromeHeight;
      treeOwnerAsWin->SetSize(width, height, PR_FALSE);
    }
  }
  treeOwnerAsWin->SetVisibility(PR_TRUE);
}

void
nsWindowWatcher::GetWindowTreeItem(nsIDOMWindow *inWindow,
                                   nsIDocShellTreeItem **outTreeItem)
{
  *outTreeItem = 0;

  nsCOMPtr<nsPIDOMWindow> window(do_QueryInterface(inWindow));
  if (window) {
    nsIDocShell *docshell = window->GetDocShell();
    if (docshell)
      CallQueryInterface(docshell, outTreeItem);
  }
}

void
nsWindowWatcher::GetWindowTreeOwner(nsIDOMWindow *inWindow,
                                    nsIDocShellTreeOwner **outTreeOwner)
{
  *outTreeOwner = 0;

  nsCOMPtr<nsIDocShellTreeItem> treeItem;
  GetWindowTreeItem(inWindow, getter_AddRefs(treeItem));
  if (treeItem)
    treeItem->GetTreeOwner(outTreeOwner);
}

JSContext *
nsWindowWatcher::GetJSContextFromCallStack()
{
  JSContext *cx = 0;

  nsCOMPtr<nsIThreadJSContextStack> cxStack(do_GetService(sJSStackContractID));
  if (cxStack)
    cxStack->Peek(&cx);

  return cx;
}

JSContext *
nsWindowWatcher::GetJSContextFromWindow(nsIDOMWindow *aWindow)
{
  JSContext *cx = 0;

  if (aWindow) {
    nsCOMPtr<nsIScriptGlobalObject> sgo(do_QueryInterface(aWindow));
    if (sgo) {
      nsIScriptContext *scx = sgo->GetContext();
      if (scx)
        cx = (JSContext *) scx->GetNativeContext();
    }
    /* (off-topic note:) the nsIScriptContext can be retrieved by
    nsCOMPtr<nsIScriptContext> scx;
    nsJSUtils::GetDynamicScriptContext(cx, getter_AddRefs(scx));
    */
  }

  return cx;
}
