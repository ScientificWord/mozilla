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
 * The Original Code is Mozilla Communicator client code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Pierre Phaneuf <pp@ludusdesign.com>
 *   Travis Bogard <travis@netscape.com>
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

// Local Includes
#include "nsBrowserInstance.h"

// Helper Includes
#include "nsIGenericFactory.h"

// Interfaces Needed
#include "nsIBaseWindow.h"
#include "nsIWebNavigationInfo.h"
#include "nsDocShellCID.h"
#include "nsIDocShell.h"
#include "nsIDocShellTreeItem.h"
#include "nsIHttpProtocolHandler.h"
#include "nsISHistory.h"
#include "nsIWebNavigation.h"

///  Unsorted Includes

#include "nsIObserver.h"
#include "pratom.h"
#include "prprf.h"
#include "nsIComponentManager.h"
#include "nsCRT.h"

#include "nsIDOMDocument.h"
#include "nsPIDOMWindow.h"

#include "nsIContentViewer.h"
#include "nsIContentViewerEdit.h"
#include "nsIWebBrowserChrome.h"
#include "nsIWindowWatcher.h"
#include "nsCOMPtr.h"
#include "nsAutoPtr.h"
#include "nsXPIDLString.h"
#include "nsReadableUtils.h"
#include "nsThreadUtils.h"

#include "nsIPrefService.h"
#include "nsIPrefBranch.h"
#include "nsIPrefLocalizedString.h"
#include "nsIServiceManager.h"
#include "nsIURL.h"
#include "nsIIOService.h"
#include "nsIWidget.h"
#include "plstr.h"

#include "nsIAppStartup.h"

#ifdef MOZ_XUL
#include "nsIBrowserHistory.h"
#endif
#include "nsDocShellCID.h"

#include "nsIObserverService.h"

#include "nsILocalFile.h"
#include "nsDirectoryServiceDefs.h"

#include "nsNetUtil.h"
#ifndef MOZ_XUL_APP
#include "nsICmdLineService.h"
#endif

// Stuff to implement file download dialog.
#include "nsIProxyObjectManager.h" 

#include "nsXPFEComponentsCID.h"

// If DEBUG, NS_BUILD_REFCNT_LOGGING, MOZ_PERF_METRICS, or MOZ_JPROF is
// defined, enable the PageCycler.
#if defined(DEBUG) || defined(NS_BUILD_REFCNT_LOGGING) || defined(MOZ_PERF_METRICS) || defined(MOZ_JPROF)
#define ENABLE_PAGE_CYCLER
#endif

#ifdef DEBUG                                                           
static int APP_DEBUG = 0; // Set to 1 in debugger to turn on debugging.
#else                                                                  
#define APP_DEBUG 0                                                    
#endif                                                                 


#define PREF_HOMEPAGE_OVERRIDE_URL "startup.homepage_override_url"
#define PREF_HOMEPAGE_OVERRIDE_MSTONE "browser.startup.homepage_override.mstone"
#define PREF_BROWSER_STARTUP_PAGE "browser.startup.page"
#define PREF_BROWSER_STARTUP_HOMEPAGE "browser.startup.homepage"

const char *kIgnoreOverrideMilestone = "ignore";

//*****************************************************************************
//***    PageCycler: Object Management
//*****************************************************************************

#ifdef ENABLE_PAGE_CYCLER
#include "nsITimer.h"

static void TimesUp(nsITimer *aTimer, void *aClosure);
  // Timer callback: called when the timer fires

class PageCycler : public nsIObserver {
public:
  NS_DECL_ISUPPORTS

  struct PageCyclerEvent : public nsRunnable {
    PageCyclerEvent(PageCycler *pc) : mPageCycler(pc) {}
    NS_IMETHOD Run() {
      mPageCycler->DoCycle();
      return NS_OK;
    }
    nsRefPtr<PageCycler> mPageCycler;
  };

  PageCycler(nsBrowserInstance* appCore, const char *aTimeoutValue = nsnull, const char *aWaitValue = nsnull)
    : mAppCore(appCore), mBuffer(nsnull), mCursor(nsnull), mTimeoutValue(0), mWaitValue(1 /*sec*/) { 
    NS_ADDREF(mAppCore);
    if (aTimeoutValue){
      mTimeoutValue = atol(aTimeoutValue);
    }
    if (aWaitValue) {
      mWaitValue = atol(aWaitValue);
    }
  }

  virtual ~PageCycler() { 
    if (mBuffer) delete[] mBuffer;
    NS_RELEASE(mAppCore);
  }

  nsresult Init(const char* nativePath) {
    nsresult rv;
    if (!mFile) {
      nsCOMPtr<nsIFile> aFile;
      rv = NS_GetSpecialDirectory(NS_XPCOM_CURRENT_PROCESS_DIR,
                                  getter_AddRefs(aFile));
      if (NS_FAILED(rv)) return rv;

      mFile = do_QueryInterface(aFile);
      NS_ASSERTION(mFile, "QI to nsILocalFile should not fail");
      rv = mFile->AppendRelativeNativePath(nsDependentCString(nativePath));
      if (NS_FAILED(rv)) return rv;
    }

    nsCOMPtr<nsIInputStream> inStr;
    rv = NS_NewLocalFileInputStream(getter_AddRefs(inStr), mFile);
    if (NS_FAILED(rv)) return rv;

    PRUint32 avail;
    rv = inStr->Available(&avail);
    if (NS_FAILED(rv)) return rv;

    mBuffer = new char[avail + 1];
    if (mBuffer == nsnull)
      return NS_ERROR_OUT_OF_MEMORY;
    PRUint32 amt;
    rv = inStr->Read(mBuffer, avail, &amt);
    if (NS_FAILED(rv)) return rv;
    NS_ASSERTION(amt == avail, "Didn't get the whole file.");
    mBuffer[avail] = '\0';
    mCursor = mBuffer;

    nsCOMPtr<nsIObserverService> obsServ = 
             do_GetService("@mozilla.org/observer-service;1", &rv);
    if (NS_FAILED(rv)) return rv;
    rv = obsServ->AddObserver(this, "EndDocumentLoad", PR_FALSE );
    NS_ASSERTION(NS_SUCCEEDED(rv), "unable to add self to observer service");
    return rv; 
  }

  nsresult GetNextURL(nsString& result) {
    result.AssignWithConversion(mCursor);
    PRInt32 pos = result.Find(NS_LINEBREAK);
    if (pos > 0) {
      result.Truncate(pos);
      mLastRequest.Assign(result);
      mCursor += pos + NS_LINEBREAK_LEN;
      return NS_OK;
    }
    else if ( !result.IsEmpty() ) {
      // no more URLs after this one
      mCursor += result.Length(); // Advance cursor to terminating '\0'
      mLastRequest.Assign(result);
      return NS_OK;
    }
    else {
      // no more URLs, so quit the browser
      nsresult rv;
      // make sure our timer is stopped first
      StopTimer();

      // XXX This needs to do a whole bunch of other stuff that
      // globalOverlay.js's goQuitApplication does.
      nsCOMPtr<nsIAppStartup> appStartup = 
               do_GetService(NS_APPSTARTUP_CONTRACTID, &rv);
      if(NS_FAILED(rv)) return rv;
      nsCOMPtr<nsIAppStartup> appStartupProxy;
      rv = NS_GetProxyForObject(NS_PROXY_TO_CURRENT_THREAD,
                                NS_GET_IID(nsIAppStartup), appStartup,
                                NS_PROXY_ASYNC | NS_PROXY_ALWAYS,
                                getter_AddRefs(appStartupProxy));

      (void)appStartupProxy->Quit(nsIAppStartup::eAttemptQuit);
      return NS_ERROR_FAILURE;
    }
  }

  NS_IMETHOD Observe(nsISupports* aSubject, 
                     const char* aTopic,
                     const PRUnichar* someData) {
    nsresult rv = NS_OK;
    nsString data(someData);
    if (data.Find(mLastRequest) == 0) {
      char* dataStr = ToNewCString(data);
      printf("########## PageCycler loaded (%d ms): %s\n", 
             PR_IntervalToMilliseconds(PR_IntervalNow() - mIntervalTime), 
             dataStr);
      NS_Free(dataStr);

      nsAutoString url;
      rv = GetNextURL(url);
      if (NS_SUCCEEDED(rv)) {
        // stop previous timer
        StopTimer();
        if (aSubject == this){
          // if the aSubject arg is 'this' then we were called by the Timer
          // Stop the current load and move on to the next
          nsCOMPtr<nsIDocShell> docShell;
          mAppCore->GetContentAreaDocShell(getter_AddRefs(docShell));

          nsCOMPtr<nsIWebNavigation> webNav(do_QueryInterface(docShell));
          if(webNav)
            webNav->Stop(nsIWebNavigation::STOP_ALL);
        }

        // We need to enqueue an event to load the next page,
        // otherwise we'll run the risk of confusing the docshell
        // (which notifies observers before propagating the
        // DocumentEndLoad up to parent docshells).
        nsCOMPtr<nsIRunnable> ev = new PageCyclerEvent(this);
        if (ev)
          rv = NS_DispatchToCurrentThread(ev);

        if (NS_FAILED(rv)) {
          printf("######### PageCycler couldn't asynchronously load: %s\n", NS_ConvertUTF16toUTF8(mLastRequest).get());
        }
      }
    }
    else {
      char* dataStr = ToNewCString(data);
      printf("########## PageCycler possible failure for: %s\n", dataStr);
      NS_Free(dataStr);
    }
    return rv;
  }

  void DoCycle()
  {
    // load the URL
    const PRUnichar* url = mLastRequest.get();
    printf("########## PageCycler starting: %s\n", NS_ConvertUTF16toUTF8(url).get());

    mIntervalTime = PR_IntervalNow();
    mAppCore->LoadUrl(url);

    // start new timer
    StartTimer();
  }

  const nsAutoString &GetLastRequest( void )
  { 
    return mLastRequest; 
  }

  // StartTimer: if mTimeoutValue is positive, then create a new timer
  //             that will call the callback fcn 'TimesUp' to keep us cycling
  //             through the URLs
  nsresult StartTimer(void)
  {
    nsresult rv=NS_OK;
    if (mTimeoutValue > 0){
      mShutdownTimer = do_CreateInstance("@mozilla.org/timer;1", &rv);
      NS_ASSERTION(NS_SUCCEEDED(rv), "unable to create timer for PageCycler...");
      if (NS_SUCCEEDED(rv) && mShutdownTimer){
        mShutdownTimer->InitWithFuncCallback(TimesUp, (void *)this, mTimeoutValue*1000,
                                             nsITimer::TYPE_ONE_SHOT);
      }
    }
    return rv;
  }

  // StopTimer: if there is a timer, cancel it
  nsresult StopTimer(void)
  {
    if (mShutdownTimer){
      mShutdownTimer->Cancel();
    }
    return NS_OK;
  }

protected:
  nsBrowserInstance*    mAppCore;
  nsCOMPtr<nsILocalFile> mFile;
  char*                 mBuffer;
  char*                 mCursor;
  nsAutoString          mLastRequest;
  nsCOMPtr<nsITimer>    mShutdownTimer;
  PRUint32              mTimeoutValue;
  PRUint32              mWaitValue;
  PRIntervalTime        mIntervalTime;
};

NS_IMPL_ISUPPORTS1(PageCycler, nsIObserver)

// TimesUp: callback for the PageCycler timer: called when we have
// waited too long for the page to finish loading.
// 
// The aClosure argument is actually the PageCycler, so use it to stop
// the timer and call the Observe fcn to move on to the next URL.  Note
// that we pass the PageCycler instance as the aSubject argument to the
// Observe function. This is our indication that the Observe method is
// being called after a timeout, allowing the PageCycler to do any
// necessary processing before moving on.

void TimesUp(nsITimer *aTimer, void *aClosure)
{
  if(aClosure){
    PageCycler *pCycler = (PageCycler *)aClosure;
    pCycler->StopTimer();
    const nsAutoString &url  = pCycler->GetLastRequest();
    fprintf(stderr,"########## PageCycler Timeout on URL: %s\n",
            NS_LossyConvertUTF16toASCII(url).get());
    pCycler->Observe( pCycler, nsnull, url.get() );
  }
}

#endif //ENABLE_PAGE_CYCLER

PRBool nsBrowserInstance::sCmdLineURLUsed = PR_FALSE;

//*****************************************************************************
//***    nsBrowserInstance: Object Management
//*****************************************************************************

nsBrowserInstance::nsBrowserInstance() : mIsClosed(PR_FALSE)
{
  mDOMWindow            = nsnull;
  mContentAreaDocShellWeak  = nsnull;
}

nsBrowserInstance::~nsBrowserInstance()
{
  Close();
}

void
nsBrowserInstance::ReinitializeContentVariables()
{
  NS_ASSERTION(mDOMWindow,"Reinitializing Content Variables without a window will cause a crash. see Bugzilla Bug 46454");
  if (!mDOMWindow)
    return;

  nsCOMPtr<nsIDOMWindow> contentWindow;
  mDOMWindow->GetContent(getter_AddRefs(contentWindow));

  nsCOMPtr<nsPIDOMWindow> pcontentWindow(do_QueryInterface(contentWindow));

  if (pcontentWindow) {
    nsIDocShell *docShell = pcontentWindow->GetDocShell();

    mContentAreaDocShellWeak = do_GetWeakReference(docShell); // Weak reference

    if (APP_DEBUG) {
      nsCOMPtr<nsIDocShellTreeItem> docShellAsItem(do_QueryInterface(docShell));
      if (docShellAsItem) {
        nsXPIDLString name;
        docShellAsItem->GetName(getter_Copies(name));
        printf("Attaching to Content WebShell [%s]\n", NS_LossyConvertUTF16toASCII(name).get());
      }
    }
  }
}

nsresult nsBrowserInstance::GetContentAreaDocShell(nsIDocShell** outDocShell)
{
  nsCOMPtr<nsIDocShell> docShell(do_QueryReferent(mContentAreaDocShellWeak));
  if (!mIsClosed && docShell) {
    // we're still alive and the docshell still exists. but has it been destroyed?
    nsCOMPtr<nsIBaseWindow> hack = do_QueryInterface(docShell);
    if (hack) {
      nsCOMPtr<nsIWidget> parent;
      hack->GetParentWidget(getter_AddRefs(parent));
      if (!parent)
        // it's a zombie. a new one is in place. set up to use it.
        docShell = 0;
    }
  }
  if (!mIsClosed && !docShell)
    ReinitializeContentVariables();

  docShell = do_QueryReferent(mContentAreaDocShellWeak);
  *outDocShell = docShell;
  NS_IF_ADDREF(*outDocShell);
  return NS_OK;
}
    
//*****************************************************************************
//    nsBrowserInstance: nsISupports
//*****************************************************************************

NS_IMPL_ADDREF(nsBrowserInstance)
NS_IMPL_RELEASE(nsBrowserInstance)

NS_INTERFACE_MAP_BEGIN(nsBrowserInstance)
   NS_INTERFACE_MAP_ENTRY(nsIBrowserInstance)
   NS_INTERFACE_MAP_ENTRY(nsISupportsWeakReference)
   NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(nsISupports, nsIBrowserInstance)
NS_INTERFACE_MAP_END

//*****************************************************************************
//    nsBrowserInstance: nsIBrowserInstance
//*****************************************************************************

nsresult
nsBrowserInstance::LoadUrl(const PRUnichar * urlToLoad)
{
  nsresult rv = NS_OK;

  nsCOMPtr<nsIDocShell> docShell;
  GetContentAreaDocShell(getter_AddRefs(docShell));

  /* Ask nsWebShell to load the URl */
  nsCOMPtr<nsIWebNavigation> webNav(do_QueryInterface(docShell));
    
  // Normal browser.
  rv = webNav->LoadURI( urlToLoad,                          // URI string
                        nsIWebNavigation::LOAD_FLAGS_NONE,  // Load flags
                        nsnull,                             // Referring URI
                        nsnull,                             // Post data
                        nsnull );                           // Extra headers

  return rv;
}

NS_IMETHODIMP    
nsBrowserInstance::SetCmdLineURLUsed(PRBool aCmdLineURLUsed)
{
  sCmdLineURLUsed = aCmdLineURLUsed;
  return NS_OK;
}

NS_IMETHODIMP    
nsBrowserInstance::GetCmdLineURLUsed(PRBool* aCmdLineURLUsed)
{
  NS_ASSERTION(aCmdLineURLUsed, "aCmdLineURLUsed can't be null");
  if (!aCmdLineURLUsed)
    return NS_ERROR_NULL_POINTER;

  *aCmdLineURLUsed = sCmdLineURLUsed;
  return NS_OK;
}

NS_IMETHODIMP    
nsBrowserInstance::StartPageCycler(PRBool* aIsPageCycling)
{
  *aIsPageCycling = PR_FALSE;

#ifndef MOZ_XUL_APP
  nsresult rv;

  if (!sCmdLineURLUsed) {
    nsCOMPtr<nsICmdLineService> cmdLineArgs = 
             do_GetService(NS_COMMANDLINESERVICE_CONTRACTID, &rv);
    if (NS_FAILED(rv)) {
      if (APP_DEBUG) fprintf(stderr, "Could not obtain CmdLine processing service\n");
      return NS_ERROR_FAILURE;
    }

#ifdef ENABLE_PAGE_CYCLER
    // First, check if there's a URL file to load (for testing), and if there 
    // is, process it instead of anything else.
    nsAutoString urlstr;
    nsXPIDLCString file;
    rv = cmdLineArgs->GetCmdLineValue("-f", getter_Copies(file));
    if (NS_SUCCEEDED(rv) && (const char*)file) {

      // see if we have a timeout value corresponding to the url-file
      nsXPIDLCString timeoutVal;
      rv = cmdLineArgs->GetCmdLineValue("-ftimeout", getter_Copies(timeoutVal));
      // see if we have a wait value corresponding to the url-file
      nsXPIDLCString waitVal;
      rv = cmdLineArgs->GetCmdLineValue("-fwait", getter_Copies(waitVal));
      // cereate the cool PageCycler instance
      PageCycler* bb = new PageCycler(this, timeoutVal, waitVal);
      if (bb == nsnull)
        return NS_ERROR_OUT_OF_MEMORY;

      NS_ADDREF(bb);
      rv = bb->Init(file);
      if (NS_FAILED(rv)) return rv;

      rv = bb->GetNextURL(urlstr);
      NS_RELEASE(bb);

      *aIsPageCycling = PR_TRUE;
    }

    if (!urlstr.IsEmpty()) {
      // A url was provided. Load it
      if (APP_DEBUG) printf("Got Command line URL to load %s\n", NS_ConvertUTF16toUTF8(urlstr).get());
      rv = LoadUrl( urlstr.get() );
      sCmdLineURLUsed = PR_TRUE;
      return rv;
    }
#endif //ENABLE_PAGE_CYCLER
  }
#endif // MOZ_XUL_APP
  return NS_OK;
}


NS_IMETHODIMP    
nsBrowserInstance::SetWebShellWindow(nsIDOMWindowInternal* aWin)
{
  NS_ENSURE_ARG(aWin);
  mDOMWindow = aWin;

  nsCOMPtr<nsPIDOMWindow> win( do_QueryInterface(aWin) );
  if (!win) {
    return NS_ERROR_FAILURE;
  }

  if (APP_DEBUG) {
    nsCOMPtr<nsIDocShellTreeItem> docShellAsItem =
      do_QueryInterface(win->GetDocShell());

    if (docShellAsItem) {
      // inform our top level webshell that we are its parent URI content listener...
      nsXPIDLString name;
      docShellAsItem->GetName(getter_Copies(name));
      printf("Attaching to WebShellWindow[%s]\n", NS_LossyConvertUTF16toASCII(name).get());
    }
  }

  ReinitializeContentVariables();

  return NS_OK;
}

NS_IMETHODIMP    
nsBrowserInstance::Close()
{ 
  // if we have already been closed....then just return
  if (mIsClosed) 
    return NS_OK;
  else
    mIsClosed = PR_TRUE;

  return NS_OK;
}

#ifndef MOZ_XUL_APP
//*****************************************************************************
// nsBrowserInstance: Helpers
//*****************************************************************************

////////////////////////////////////////////////////////////////////////
// browserCntHandler is a content handler component that registers
// the browse as the preferred content handler for various content
// types like text/html, image/jpeg, etc. When the uri loader
// has a content type that no currently open window wants to handle, 
// it will ask the registry for a registered content handler for this
// type. If the browser is registered to handle that type, we'll end
// up in here where we create a new browser window for the url.
//
// I had intially written this as a JS component and would like to do
// so again. However, JS components can't access xpconnect objects that
// return DOM objects. And we need a dom window to bootstrap the browser
/////////////////////////////////////////////////////////////////////////

NS_IMPL_ISUPPORTS1(nsChromeStartupHandler, nsICmdLineHandler)
CMDLINEHANDLER_IMPL(nsChromeStartupHandler, "-chrome", "", "",
                    "Load the specified chrome.",
                    NS_CHROMESTARTUPHANDLER_CONTRACTID,
                    "Chrome Startup Handler", PR_TRUE, "", PR_FALSE)

NS_IMPL_ISUPPORTS2(nsBrowserContentHandler, nsIContentHandler, nsICmdLineHandler)
CMDLINEHANDLER_OTHERS_IMPL(nsBrowserContentHandler, "-browser",
                           "general.startup.browser", "Load the browser.",
                           PR_TRUE, PR_TRUE)
CMDLINEHANDLER_REGISTERPROC_IMPL(nsBrowserContentHandler,
                                 "Browser Startup Handler",
                                 NS_BROWSERSTARTUPHANDLER_CONTRACTID)
NS_IMETHODIMP nsBrowserContentHandler::GetChromeUrlForTask(char **aChromeUrlForTask) {

  if (!aChromeUrlForTask)
    return NS_ERROR_NULL_POINTER;

  nsresult rv = NS_ERROR_FAILURE;
  nsCOMPtr<nsIPrefBranch> prefs(do_GetService(NS_PREFSERVICE_CONTRACTID));
  if (prefs) {
    rv = prefs->GetCharPref("browser.chromeURL", aChromeUrlForTask);
    if (NS_SUCCEEDED(rv) && (*aChromeUrlForTask)[0] == '\0') {
      PL_strfree(*aChromeUrlForTask);
      rv = NS_ERROR_FAILURE;
    }
  }
  if (NS_FAILED(rv))
    *aChromeUrlForTask = PL_strdup("chrome://navigator/content/navigator.xul");

  return NS_OK;
}

PRBool nsBrowserContentHandler::NeedHomepageOverride(nsIPrefBranch *aPrefService)
{
  NS_ASSERTION(aPrefService, "Null pointer to prefs service!");

  // get saved milestone from user's prefs
  nsXPIDLCString savedMilestone;
  aPrefService->GetCharPref(PREF_HOMEPAGE_OVERRIDE_MSTONE, 
                            getter_Copies(savedMilestone));
  // Mozilla never saves this value, but a fed-up advanced user might
  if (savedMilestone.Equals(kIgnoreOverrideMilestone))
    return PR_FALSE;

  // get browser's current milestone
  nsCOMPtr<nsIHttpProtocolHandler> httpHandler(
      do_GetService("@mozilla.org/network/protocol;1?name=http"));
  if (!httpHandler)
    return PR_TRUE;

  nsCAutoString currMilestone;
  httpHandler->GetMisc(currMilestone);

  // failed to get pref -or- saved milestone older than current milestone, 
  // write out known current milestone and show URL this time
  if (!(currMilestone.Equals(savedMilestone))) {
    // update milestone in "homepage override" pref
    aPrefService->SetCharPref(PREF_HOMEPAGE_OVERRIDE_MSTONE, 
                              currMilestone.get());
    return PR_TRUE;
  }
  
  // don't override if saved and current are same
  return PR_FALSE;
}

nsresult GetHomePageGroup(nsIPrefBranch* aPref, PRUnichar** aResult)
{
  nsresult rv;

  nsCOMPtr<nsIPrefLocalizedString> uri;
  rv = aPref->GetComplexValue(PREF_BROWSER_STARTUP_HOMEPAGE,
                              NS_GET_IID(nsIPrefLocalizedString),
                              getter_AddRefs(uri));
  if (NS_FAILED(rv))
    return rv;

  PRInt32 count = 0;
  rv = aPref->GetIntPref("browser.startup.homepage.count", &count);

  // if we couldn't get the pref (unlikely) or only have one homepage
  if (NS_FAILED(rv) || count <= 1) {
    return uri->ToString(aResult);
  }

  // The "homepage" is a group of pages, put them in uriList separated by '\n'
  nsString uriList;
  rv = uri->GetData(getter_Copies(uriList));
  if (NS_FAILED(rv))
    return rv;

  for (PRInt32 i = 1; i < count; ++i) {
    nsCAutoString pref(NS_LITERAL_CSTRING("browser.startup.homepage."));
    pref.AppendInt(i);

    rv = aPref->GetComplexValue(pref.get(),
                                NS_GET_IID(nsIPrefLocalizedString),
                                getter_AddRefs(uri));
    if (NS_FAILED(rv))
      return rv;

    nsString uriString;
    rv = uri->GetData(getter_Copies(uriString));
    if (NS_FAILED(rv))
      return rv;

    uriList.Append(PRUnichar('\n'));
    uriList.Append(uriString);
  }

  *aResult = ToNewUnicode(uriList);
  return NS_OK;
}

NS_IMETHODIMP nsBrowserContentHandler::GetDefaultArgs(PRUnichar **aDefaultArgs)
{
  if (!aDefaultArgs)
    return NS_ERROR_NULL_POINTER;

  nsresult rv;

  nsCOMPtr<nsIPrefBranch> prefs(do_GetService(NS_PREFSERVICE_CONTRACTID));
  if (prefs) {
    if (NeedHomepageOverride(prefs)) {
      nsCOMPtr<nsIPrefLocalizedString> overrideURL;
      rv = prefs->GetComplexValue(PREF_HOMEPAGE_OVERRIDE_URL,
                                  NS_GET_IID(nsIPrefLocalizedString),
                                  getter_AddRefs(overrideURL));
      if (NS_SUCCEEDED(rv)) {
        rv = overrideURL->ToString(aDefaultArgs);
        if (NS_SUCCEEDED(rv) && *aDefaultArgs)
          return NS_OK;
      }
    }

    PRInt32 choice = 0;
    rv = prefs->GetIntPref(PREF_BROWSER_STARTUP_PAGE, &choice);
    if (NS_SUCCEEDED(rv)) {
      switch (choice) {
        case 1: {
          // skip the code below
          rv = GetHomePageGroup(prefs, aDefaultArgs);
          if (NS_SUCCEEDED(rv) && *aDefaultArgs)
            return NS_OK;
        }
        case 2: {
          nsCOMPtr<nsIBrowserHistory> history(do_GetService(NS_GLOBALHISTORY2_CONTRACTID));
          if (history) {
            nsCAutoString curl;
            rv = history->GetLastPageVisited(curl);
            if (NS_SUCCEEDED(rv)) {
              *aDefaultArgs = UTF8ToNewUnicode(curl);
              if (*aDefaultArgs) return NS_OK;
            }
          }
        }
      }
    }
  }
    
  // the default, in case we fail somewhere
  *aDefaultArgs = ToNewUnicode(NS_LITERAL_STRING("about:blank"));
  if (!*aDefaultArgs) return NS_ERROR_OUT_OF_MEMORY;

  return NS_OK;
}

NS_IMETHODIMP nsBrowserContentHandler::HandleContent(const char * aContentType,
                                                     nsIInterfaceRequestor * aWindowContext,
                                                     nsIRequest * aRequest)
{
  NS_PRECONDITION(aContentType, "Must have a content type");

  // Verify that we can handle this content, to avoid infinite window opening
  // loops
  nsresult rv;
  nsCOMPtr<nsIWebNavigationInfo> webNavInfo =
    do_GetService(NS_WEBNAVIGATION_INFO_CONTRACTID, &rv);
  NS_ENSURE_SUCCESS(rv, rv);

  PRUint32 typeSupported;
  rv = webNavInfo->IsTypeSupported(nsDependentCString(aContentType), nsnull,
                                   &typeSupported);
  NS_ENSURE_SUCCESS(rv, rv);

  if (!typeSupported)
      return NS_ERROR_WONT_HANDLE_CONTENT;

  // create a new browser window to handle the content
  NS_ENSURE_ARG(aRequest);
  nsCOMPtr<nsIDOMWindow> parentWindow;

  if (aWindowContext)
    parentWindow = do_GetInterface(aWindowContext);

  nsCOMPtr<nsIChannel> aChannel = do_QueryInterface(aRequest);
  if (!aChannel) return NS_ERROR_FAILURE;

  nsCOMPtr<nsIURI> uri;
  aChannel->GetURI(getter_AddRefs(uri));
  NS_ENSURE_TRUE(uri, NS_ERROR_FAILURE);
  nsCAutoString spec;
  uri->GetSpec(spec);

  nsCOMPtr<nsIWindowWatcher> wwatch(do_GetService(NS_WINDOWWATCHER_CONTRACTID));
  if (wwatch) {
    nsCOMPtr<nsIDOMWindow> newWindow;
    wwatch->OpenWindow(parentWindow, spec.get(), "", 0, 0,
              getter_AddRefs(newWindow));
  }

  // now abort the current channel load...
  aRequest->Cancel(NS_BINDING_ABORTED);

  return NS_OK;
}

#endif // MOZ_XUL_APP
