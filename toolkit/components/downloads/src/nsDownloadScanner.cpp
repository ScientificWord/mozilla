/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: se cin sw=2 ts=2 et : */
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
 * The Original Code is download manager code.
 *
 * The Initial Developer of the Original Code is
 * Mozilla Corporation
 *
 * Contributor(s):
 *   Rob Arnold <robarnold@mozilla.com> (Original Author)
 *   Masatoshi Kimura <VYV03354@nifty.ne.jp>
 *   Jim Mathies <jmathies@mozilla.com>
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
 
#include "nsDownloadScanner.h"
#include <comcat.h>
#include <process.h>
#include "nsDownloadManager.h"
#include "nsIXULAppInfo.h"
#include "nsXULAppAPI.h"
#include "nsIPrefService.h"
#include "nsNetUtil.h"
#include "nsDeque.h"

/**
 * Code overview
 *
 * Download scanner attempts to make use of one of two different virus
 * scanning interfaces available on Windows - IOfficeAntiVirus (Windows
 * 95/NT 4 and IE 5) and IAttachmentExecute (XPSP2 and up).  The latter
 * interface supports calling IOfficeAntiVirus internally, while also
 * adding support for XPSP2+ ADS forks which define security related
 * prompting on downloaded content.  
 *
 * Both interfaces are synchronous and can take a while, so it is not a
 * good idea to call either from the main thread. Some antivirus scanners can
 * take a long time to scan or the call might block while the scanner shows
 * its UI so if the user were to download many files that finished around the
 * same time, they would have to wait a while if the scanning were done on
 * exactly one other thread. Since the overhead of creating a thread is
 * relatively small compared to the time it takes to download a file and scan
 * it, a new thread is spawned for each download that is to be scanned. Since
 * most of the mozilla codebase is not threadsafe, all the information needed
 * for the scanner is gathered in the main thread in nsDownloadScanner::Scan::Start.
 * The only function of nsDownloadScanner::Scan which is invoked on another
 * thread is DoScan.
 *
 * Watchdog overview
 *
 * The watchdog is used internally by the scanner. It maintains a queue of
 * current download scans. In a separate thread, it dequeues the oldest scan
 * and waits on that scan's thread with a timeout given by WATCHDOG_TIMEOUT
 * (default is 30 seconds). If the wait times out, then the watchdog notifies
 * the Scan that it has timed out. If the scan really has timed out, then the
 * Scan object will dispatch its run method to the main thread; this will
 * release the watchdog thread's addref on the Scan. If it has not timed out
 * (i.e. the Scan just finished in time), then the watchdog dispatches a 
 * ReleaseDispatcher to release its ref of the Scan on the main thread.
 *
 * In order to minimize execution time, there are two events used to notify the
 * watchdog thread of a non-empty queue and a quit event. Every blocking wait 
 * that the watchdog thread does waits on the quit event; this lets the thread
 * quickly exit when shutting down. Also, the download scan queue will be empty
 * most of the time; rather than use a spin loop, a simple event is triggered
 * by the main thread when a new scan is added to an empty queue. When the
 * watchdog thread knows that it has run out of elements in the queue, it will
 * wait on the new item event.
 *
 * Memory/resource leaks due to timeout:
 * In the event of a timeout, the thread must remain alive; terminating it may
 * very well cause the antivirus scanner to crash or be put into an
 * inconsistent state; COM resources may also not be cleaned up. The downside
 * is that we need to leave the thread running; suspending it may lead to a
 * deadlock. Because the scan call may be ongoing, it may be dependent on the
 * memory referenced by the MSOAVINFO structure, so we cannot free mName, mPath
 * or mOrigin; this means that we cannot free the Scan object since doing so
 * will deallocate that memory. Note that mDownload is set to null upon timeout
 * or completion, so the download itself is never leaked. If the scan does
 * eventually complete, then the all the memory and resources will be freed.
 * It is possible, however extremely rare, that in the event of a timeout, the
 * mStateSync critical section will leak its event; this will happen only if
 * the scanning thread, watchdog thread or main thread try to enter the
 * critical section when one of the others is already in it.
 *
 * Reasoning for CheckAndSetState - there exists a race condition between the time when
 * either the timeout or normal scan sets the state and when Scan::Run is
 * executed on the main thread. Ex: mStatus could be set by Scan::DoScan* which
 * then queues a dispatch on the main thread. Before that dispatch is executed,
 * the timeout code fires and sets mStatus to AVSCAN_TIMEDOUT which then queues
 * its dispatch to the main thread (the same function as DoScan*). Both
 * dispatches run and both try to set the download state to AVSCAN_TIMEDOUT
 * which is incorrect.
 *
 * There are 5 possible outcomes of the virus scan:
 *    AVSCAN_GOOD     => the file is clean
 *    AVSCAN_BAD      => the file has a virus
 *    AVSCAN_UGLY     => the file had a virus, but it was cleaned
 *    AVSCAN_FAILED   => something else went wrong with the virus scanner.
 *    AVSCAN_TIMEDOUT => the scan (thread setup + execution) took too long
 *
 * Both the good and ugly states leave the user with a benign file, so they
 * transition to the finished state. Bad files are sent to the blocked state.
 * The failed and timedout states transition to finished downloads.
 *
 * Possible Future enhancements:
 *  * Create an interface for scanning files in general
 *  * Make this a service
 *  * Get antivirus scanner status via WMI/registry
 */

#define PREF_BDA_DONTCLEAN "browser.download.antivirus.dontclean"

// IAttachementExecute supports user definable settings for certain
// security related prompts. This defines a general GUID for use in
// all projects. Individual projects can define an individual guid
// if they want to.
#ifndef MOZ_VIRUS_SCANNER_PROMPT_GUID
#define MOZ_VIRUS_SCANNER_PROMPT_GUID \
  { 0xb50563d1, 0x16b6, 0x43c2, { 0xa6, 0x6a, 0xfa, 0xe6, 0xd2, 0x11, 0xf2, \
  0xea } }
#endif
static const GUID GUID_MozillaVirusScannerPromptGeneric =
  MOZ_VIRUS_SCANNER_PROMPT_GUID;

// Initial timeout is 30 seconds
#define WATCHDOG_TIMEOUT (30*PR_USEC_PER_SEC)

class nsDownloadScannerWatchdog 
{
  typedef nsDownloadScanner::Scan Scan;
public:
  nsDownloadScannerWatchdog();
  ~nsDownloadScannerWatchdog();

  nsresult Init();
  nsresult Shutdown();

  void Watch(Scan *scan);
private:
  static unsigned int __stdcall WatchdogThread(void *p);
  CRITICAL_SECTION mQueueSync;
  nsDeque mScanQueue;
  HANDLE mThread;
  HANDLE mNewItemEvent;
  HANDLE mQuitEvent;
};

nsDownloadScanner::nsDownloadScanner()
  : mHaveAVScanner(PR_FALSE), mHaveAttachmentExecute(PR_FALSE)
{
}
 
// This destructor appeases the compiler; it would otherwise complain about an
// incomplete type for nsDownloadWatchdog in the instantiation of
// nsAutoPtr::~nsAutoPtr
// Plus, it's a handy location to call nsDownloadScannerWatchdog::Shutdown from
nsDownloadScanner::~nsDownloadScanner() {
  if (mWatchdog)
    (void)mWatchdog->Shutdown();
}

nsresult
nsDownloadScanner::Init()
{
  // This CoInitialize/CoUninitialize pattern seems to be common in the Mozilla
  // codebase. All other COM calls/objects are made on different threads.
  nsresult rv = NS_OK;
  CoInitialize(NULL);
  if (!IsAESAvailable() && ListCLSID() < 0)
    rv = NS_ERROR_NOT_AVAILABLE;
  CoUninitialize();
  if (NS_SUCCEEDED(rv)) {
    mWatchdog = new nsDownloadScannerWatchdog();
    if (mWatchdog) {
      rv = mWatchdog->Init();
      if (FAILED(rv))
        mWatchdog = nsnull;
    } else {
      rv = NS_ERROR_OUT_OF_MEMORY;
    }
  }

  return rv;
}

PRBool
nsDownloadScanner::IsAESAvailable()
{
  nsRefPtr<IAttachmentExecute> ae;
  HRESULT hr;
  hr = CoCreateInstance(CLSID_AttachmentServices, NULL, CLSCTX_INPROC,
                        IID_IAttachmentExecute, getter_AddRefs(ae));
  if (FAILED(hr)) {
    NS_WARNING("Could not instantiate attachment execution service\n");
    return PR_FALSE;
  }

  mHaveAVScanner = PR_TRUE;
  mHaveAttachmentExecute = PR_TRUE;
  return PR_TRUE;
}

PRInt32
nsDownloadScanner::ListCLSID()
{
  nsRefPtr<ICatInformation> catInfo;
  HRESULT hr;
  hr = CoCreateInstance(CLSID_StdComponentCategoriesMgr, NULL, CLSCTX_INPROC,
                        IID_ICatInformation, getter_AddRefs(catInfo));
  if (FAILED(hr)) {
    NS_WARNING("Could not create category information class\n");
    return -1;
  }
  nsRefPtr<IEnumCLSID> clsidEnumerator;
  GUID guids [1] = { CATID_MSOfficeAntiVirus };
  hr = catInfo->EnumClassesOfCategories(1, guids, 0, NULL,
      getter_AddRefs(clsidEnumerator));
  if (FAILED(hr)) {
    NS_WARNING("Could not get class enumerator for category\n");
    return -2;
  }

  ULONG nReceived;
  CLSID clsid;
  while(clsidEnumerator->Next(1, &clsid, &nReceived) == S_OK && nReceived == 1)
    mScanCLSID.AppendElement(clsid);

  if (mScanCLSID.Length() == 0) {
    // No installed Anti Virus program
    return -3;
  }

  mHaveAVScanner = PR_TRUE;
  return 0;
}

#ifndef THREAD_MODE_BACKGROUND_BEGIN
#define THREAD_MODE_BACKGROUND_BEGIN 0x00010000
#endif

#ifndef THREAD_MODE_BACKGROUND_END
#define THREAD_MODE_BACKGROUND_END 0x00020000
#endif

unsigned int __stdcall
nsDownloadScanner::ScannerThreadFunction(void *p)
{
  HANDLE currentThread = GetCurrentThread();
  NS_ASSERTION(!NS_IsMainThread(), "Antivirus scan should not be run on the main thread");
  nsDownloadScanner::Scan *scan = static_cast<nsDownloadScanner::Scan*>(p);
  if (!SetThreadPriority(currentThread, THREAD_MODE_BACKGROUND_BEGIN))
    (void)SetThreadPriority(currentThread, THREAD_PRIORITY_IDLE);
  scan->DoScan();
  (void)SetThreadPriority(currentThread, THREAD_MODE_BACKGROUND_END);
  _endthreadex(0);
  return 0;
}

// The sole purpose of this class is to release an object on the main thread
// It assumes that its creator will addref it and it will release itself on
// the main thread too
class ReleaseDispatcher : public nsRunnable {
public:
  ReleaseDispatcher(nsISupports *ptr)
    : mPtr(ptr) {}
  NS_IMETHOD Run();
private:
  nsISupports *mPtr;
};

nsresult ReleaseDispatcher::Run() {
  NS_ASSERTION(NS_IsMainThread(), "Antivirus scan release dispatch should be run on the main thread");
  NS_RELEASE(mPtr);
  NS_RELEASE_THIS();
  return NS_OK;
}

nsDownloadScanner::Scan::Scan(nsDownloadScanner *scanner, nsDownload *download)
  : mDLScanner(scanner), mThread(NULL), 
    mDownload(download), mStatus(AVSCAN_NOTSTARTED)
{
  InitializeCriticalSection(&mStateSync);
}

nsDownloadScanner::Scan::~Scan() {
  DeleteCriticalSection(&mStateSync);
}

nsresult
nsDownloadScanner::Scan::Start()
{
  mStartTime = PR_Now();

  mThread = (HANDLE)_beginthreadex(NULL, 0, ScannerThreadFunction,
      this, CREATE_SUSPENDED, NULL);
  if (!mThread)
    return NS_ERROR_OUT_OF_MEMORY;

  nsresult rv = NS_OK;

  // Default is to try to clean downloads
  mIsReadOnlyRequest = PR_FALSE;

  nsCOMPtr<nsIPrefBranch> pref =
    do_GetService(NS_PREFSERVICE_CONTRACTID);
  if (pref)
    rv = pref->GetBoolPref(PREF_BDA_DONTCLEAN, &mIsReadOnlyRequest);

  // Get the path to the file on disk
  nsCOMPtr<nsILocalFile> file;
  rv = mDownload->GetTargetFile(getter_AddRefs(file));
  NS_ENSURE_SUCCESS(rv, rv);
  rv = file->GetPath(mPath);
  NS_ENSURE_SUCCESS(rv, rv);

  // Grab the app name
  nsCOMPtr<nsIXULAppInfo> appinfo =
    do_GetService(XULAPPINFO_SERVICE_CONTRACTID, &rv);
  NS_ENSURE_SUCCESS(rv, rv);

  nsCAutoString name;
  rv = appinfo->GetName(name);
  NS_ENSURE_SUCCESS(rv, rv);
  CopyUTF8toUTF16(name, mName);

  // Get the origin
  nsCOMPtr<nsIURI> uri;
  rv = mDownload->GetSource(getter_AddRefs(uri));
  NS_ENSURE_SUCCESS(rv, rv);

  nsCAutoString origin;
  rv = uri->GetSpec(origin);
  NS_ENSURE_SUCCESS(rv, rv);

  CopyUTF8toUTF16(origin, mOrigin);

  // We count https/ftp/http as an http download
  PRBool isHttp(PR_FALSE), isFtp(PR_FALSE), isHttps(PR_FALSE);
  nsCOMPtr<nsIURI> innerURI = NS_GetInnermostURI(uri);
  (void)innerURI->SchemeIs("http", &isHttp);
  (void)innerURI->SchemeIs("ftp", &isFtp);
  (void)innerURI->SchemeIs("https", &isHttps);
  mIsHttpDownload = isHttp || isFtp || isHttps;

  // ResumeThread returns the previous suspend count
  if (1 != ::ResumeThread(mThread)) {
    CloseHandle(mThread);
    return NS_ERROR_UNEXPECTED;
  }
  return NS_OK;
}

nsresult
nsDownloadScanner::Scan::Run()
{
  NS_ASSERTION(NS_IsMainThread(), "Antivirus scan dispatch should be run on the main thread");

  // Cleanup our thread
  if (mStatus != AVSCAN_TIMEDOUT)
    WaitForSingleObject(mThread, INFINITE);
  CloseHandle(mThread);

  DownloadState downloadState = 0;
  EnterCriticalSection(&mStateSync);
  switch (mStatus) {
    case AVSCAN_BAD:
      downloadState = nsIDownloadManager::DOWNLOAD_DIRTY;
      break;
    default:
    case AVSCAN_FAILED:
    case AVSCAN_GOOD:
    case AVSCAN_UGLY:
    case AVSCAN_TIMEDOUT:
      downloadState = nsIDownloadManager::DOWNLOAD_FINISHED;
      break;
  }
  LeaveCriticalSection(&mStateSync);
  // Download will be null if we already timed out
  if (mDownload)
    (void)mDownload->SetState(downloadState);

  // Clean up some other variables
  // In the event of a timeout, our destructor won't be called
  mDownload = nsnull;

  NS_RELEASE_THIS();
  return NS_OK;
}

static DWORD
ExceptionFilterFunction(DWORD exceptionCode) {
  switch(exceptionCode) {
    case EXCEPTION_ACCESS_VIOLATION:
    case EXCEPTION_ILLEGAL_INSTRUCTION:
    case EXCEPTION_IN_PAGE_ERROR:
    case EXCEPTION_PRIV_INSTRUCTION:
    case EXCEPTION_STACK_OVERFLOW:
      return EXCEPTION_EXECUTE_HANDLER;
    default:
      return EXCEPTION_CONTINUE_SEARCH;
  }
}

PRBool
nsDownloadScanner::Scan::DoScanAES()
{
  // This warning is for the destructor of ae which will not be invoked in the
  // event of a win32 exception
#pragma warning(disable: 4509)
  HRESULT hr;
  nsRefPtr<IAttachmentExecute> ae;
  __try {
    hr = CoCreateInstance(CLSID_AttachmentServices, NULL, CLSCTX_ALL,
                          IID_IAttachmentExecute, getter_AddRefs(ae));
  } __except(ExceptionFilterFunction(GetExceptionCode())) {
    return CheckAndSetState(AVSCAN_NOTSTARTED,AVSCAN_FAILED);
  }

  // If we (somehow) already timed out, then don't bother scanning
  if (CheckAndSetState(AVSCAN_SCANNING, AVSCAN_NOTSTARTED)) {
    AVScanState newState;
    if (SUCCEEDED(hr)) {
      PRBool gotException = PR_FALSE;
      __try {
        (void)ae->SetClientGuid(GUID_MozillaVirusScannerPromptGeneric);
        (void)ae->SetLocalPath(mPath.BeginWriting());
        (void)ae->SetSource(mOrigin.BeginWriting());

        // Save() will invoke the scanner
        hr = ae->Save();
      } __except(ExceptionFilterFunction(GetExceptionCode())) {
        gotException = PR_TRUE;
      }

      __try {
        ae = NULL;
      } __except(ExceptionFilterFunction(GetExceptionCode())) {
        gotException = PR_TRUE;
      }

      if(gotException) {
        newState = AVSCAN_FAILED;
      }
      else if (SUCCEEDED(hr)) { // Passed the scan
        newState = AVSCAN_GOOD;
      }
      else if (HRESULT_CODE(hr) == ERROR_FILE_NOT_FOUND) {
        NS_WARNING("Downloaded file disappeared before it could be scanned");
        newState = AVSCAN_FAILED;
      }
      else { 
        newState = AVSCAN_UGLY;
      }
    }
    else {
      newState = AVSCAN_FAILED;
    }
    return CheckAndSetState(newState, AVSCAN_SCANNING);
  }
  return PR_FALSE;
}
#pragma warning(default: 4509)

PRBool
nsDownloadScanner::Scan::DoScanOAV()
{
  HRESULT hr;
  MSOAVINFO info;
  info.cbsize = sizeof(MSOAVINFO);
  info.fPath = TRUE;
  info.fInstalled = FALSE;
  info.fReadOnlyRequest = mIsReadOnlyRequest;
  info.fHttpDownload = mIsHttpDownload;
  info.hwnd = NULL;

  info.pwzHostName = mName.BeginWriting();
  info.u.pwzFullPath = mPath.BeginWriting();
  info.pwzOrigURL = mOrigin.BeginWriting();

  AVScanState newState = AVSCAN_GOOD;
  // If we (somehow) already timed out, then don't bother scanning
  if (CheckAndSetState(AVSCAN_SCANNING, AVSCAN_NOTSTARTED)) {
    // This warning is for the destructor of vScanner which will not be invoked
    // in the event of a win32 exception
#pragma warning(disable: 4509)
    for (PRUint32 i = 0; i < mDLScanner->mScanCLSID.Length(); i++) {
      nsRefPtr<IOfficeAntiVirus> vScanner;
      __try {
        hr = CoCreateInstance(mDLScanner->mScanCLSID[i], NULL, CLSCTX_ALL,
                              IID_IOfficeAntiVirus, getter_AddRefs(vScanner));
      } __except(ExceptionFilterFunction(GetExceptionCode())) {
        newState = AVSCAN_FAILED;
        // Try the next one if there is one
        continue;
      }
      if (FAILED(hr)) {
        NS_WARNING("Could not instantiate antivirus scanner");
        newState = AVSCAN_FAILED;
      } else {
        PRBool gotException = PR_FALSE;
        newState = AVSCAN_SCANNING;

        __try {
          hr = vScanner->Scan(&info);
        } __except(ExceptionFilterFunction(GetExceptionCode())) {
          gotException = PR_TRUE;
        }

        // Invoke destructor
        __try {
          vScanner = NULL;
        } __except(ExceptionFilterFunction(GetExceptionCode())) {
          gotException = PR_TRUE;
        }

        if(gotException) {
          newState = AVSCAN_FAILED;
          continue;
        } else if (hr == S_OK) { // Passed the scan
          newState = AVSCAN_GOOD;
          continue;
        }
        else if (hr == S_FALSE) { // Failed but cleaned up
          newState = AVSCAN_UGLY;
          continue;
        }
        else if (hr == ERROR_FILE_NOT_FOUND) {
          NS_WARNING("Downloaded file disappeared before it could be scanned");
          newState = AVSCAN_FAILED;
          break;
        }
        else if (hr == E_FAIL) { // Failed
          newState = AVSCAN_BAD;
          break;
        }
        else {
          newState = AVSCAN_FAILED;
          break;
        }
      }
    }
#pragma warning(default: 4509)
  }

  // If the previous CheckAndSetState call failed, then this one will too
  return CheckAndSetState(newState, AVSCAN_SCANNING);
}

void
nsDownloadScanner::Scan::DoScan()
{
  CoInitialize(NULL);

  if (mDLScanner->mHaveAttachmentExecute ? DoScanAES() : DoScanOAV()) {
    // We need to do a few more things on the main thread
    NS_DispatchToMainThread(this);
  } else {
    // We timed out, so just release
    ReleaseDispatcher* releaser = new ReleaseDispatcher(this);
    if(releaser) {
      NS_ADDREF(releaser);
      NS_DispatchToMainThread(releaser);
    }
  }

  __try {
    CoUninitialize();
  } __except(ExceptionFilterFunction(GetExceptionCode())) {
    // Not much we can do at this point...
  }
}

HANDLE
nsDownloadScanner::Scan::GetWaitableThreadHandle() const
{
  HANDLE targetHandle = INVALID_HANDLE_VALUE;
  (void)DuplicateHandle(GetCurrentProcess(), mThread,
                        GetCurrentProcess(), &targetHandle,
                        SYNCHRONIZE, // Only allow clients to wait on this handle
                        FALSE, // cannot be inherited by child processes
                        0);
  return targetHandle;
}

PRBool
nsDownloadScanner::Scan::NotifyTimeout()
{
  PRBool didTimeout = CheckAndSetState(AVSCAN_TIMEDOUT, AVSCAN_SCANNING) ||
                      CheckAndSetState(AVSCAN_TIMEDOUT, AVSCAN_NOTSTARTED);
  if (didTimeout) {
    // We need to do a few more things on the main thread
    NS_DispatchToMainThread(this);
  }
  return didTimeout;
}

PRBool
nsDownloadScanner::Scan::CheckAndSetState(AVScanState newState, AVScanState expectedState) {
  PRBool gotExpectedState = PR_FALSE;
  EnterCriticalSection(&mStateSync);
  if(gotExpectedState = (mStatus == expectedState))
    mStatus = newState;
  LeaveCriticalSection(&mStateSync);
  return gotExpectedState;
}

nsresult
nsDownloadScanner::ScanDownload(nsDownload *download)
{
  if (!mHaveAVScanner)
    return NS_ERROR_NOT_AVAILABLE;

  // No ref ptr, see comment below
  Scan *scan = new Scan(this, download);
  if (!scan)
    return NS_ERROR_OUT_OF_MEMORY;

  NS_ADDREF(scan);

  nsresult rv = scan->Start();

  // Note that we only release upon error. On success, the scan is passed off
  // to a new thread. It is eventually released in Scan::Run on the main thread.
  if (NS_FAILED(rv))
    NS_RELEASE(scan);
  else
    // Notify the watchdog
    mWatchdog->Watch(scan);

  return rv;
}

nsDownloadScannerWatchdog::nsDownloadScannerWatchdog() 
  : mNewItemEvent(NULL), mQuitEvent(NULL) {
  InitializeCriticalSection(&mQueueSync);
}
nsDownloadScannerWatchdog::~nsDownloadScannerWatchdog() {
  DeleteCriticalSection(&mQueueSync);
}

nsresult
nsDownloadScannerWatchdog::Init() {
  // Both events are auto-reset
  mNewItemEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
  if (INVALID_HANDLE_VALUE == mNewItemEvent)
    return NS_ERROR_OUT_OF_MEMORY;
  mQuitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
  if (INVALID_HANDLE_VALUE == mQuitEvent) {
    (void)CloseHandle(mNewItemEvent);
    return NS_ERROR_OUT_OF_MEMORY;
  }

  // This thread is always running, however it will be asleep
  // for most of the dlmgr's lifetime
  mThread = (HANDLE)_beginthreadex(NULL, 0, WatchdogThread,
                                   this, 0, NULL);
  if (!mThread) {
    (void)CloseHandle(mNewItemEvent);
    (void)CloseHandle(mQuitEvent);
    return NS_ERROR_OUT_OF_MEMORY;
  }

  return NS_OK;
}

nsresult
nsDownloadScannerWatchdog::Shutdown() {
  // Tell the watchdog thread to quite
  (void)SetEvent(mQuitEvent);
  (void)WaitForSingleObject(mThread, INFINITE);
  (void)CloseHandle(mThread);
  // Manually clear and release the queued scans
  while (mScanQueue.GetSize() != 0) {
    Scan *scan = reinterpret_cast<Scan*>(mScanQueue.Pop());
    NS_RELEASE(scan);
  }
  (void)CloseHandle(mNewItemEvent);
  (void)CloseHandle(mQuitEvent);
  return NS_OK;
}

void
nsDownloadScannerWatchdog::Watch(Scan *scan) {
  PRBool wasEmpty;
  // Note that there is no release in this method
  // The scan will be released by the watchdog ALWAYS on the main thread
  // when either the watchdog thread processes the scan or the watchdog
  // is shut down
  NS_ADDREF(scan);
  EnterCriticalSection(&mQueueSync);
  wasEmpty = mScanQueue.GetSize()==0;
  mScanQueue.Push(scan);
  LeaveCriticalSection(&mQueueSync);
  // If the queue was empty, then the watchdog thread is/will be asleep
  if (wasEmpty)
    (void)SetEvent(mNewItemEvent);
}

unsigned int
__stdcall
nsDownloadScannerWatchdog::WatchdogThread(void *p) {
  NS_ASSERTION(!NS_IsMainThread(), "Antivirus scan watchdog should not be run on the main thread");
  nsDownloadScannerWatchdog *watchdog = (nsDownloadScannerWatchdog*)p;
  HANDLE waitHandles[3] = {watchdog->mNewItemEvent, watchdog->mQuitEvent, INVALID_HANDLE_VALUE};
  DWORD waitStatus;
  DWORD queueItemsLeft = 0;
  // Loop until quit event or error
  while (0 != queueItemsLeft ||
         (WAIT_OBJECT_0 + 1) !=
           (waitStatus =
              WaitForMultipleObjects(2, waitHandles, FALSE, INFINITE)) &&
         waitStatus != WAIT_FAILED) {
    Scan *scan = NULL;
    PRTime startTime, expectedEndTime, now;
    DWORD waitTime;

    // Pop scan from queue
    EnterCriticalSection(&watchdog->mQueueSync);
    scan = reinterpret_cast<Scan*>(watchdog->mScanQueue.Pop());
    queueItemsLeft = watchdog->mScanQueue.GetSize();
    LeaveCriticalSection(&watchdog->mQueueSync);

    // Calculate expected end time
    startTime = scan->GetStartTime();
    expectedEndTime = WATCHDOG_TIMEOUT + startTime;
    now = PR_Now();
    // PRTime is not guaranteed to be a signed integral type (afaik), but
    // currently it is
    if (now > expectedEndTime) {
      waitTime = 0;
    } else {
      // This is a positive value, and we know that it will not overflow
      // (bounded by WATCHDOG_TIMEOUT)
      // waitTime is in milliseconds, nspr uses microseconds
      waitTime = static_cast<DWORD>((expectedEndTime - now)/PR_USEC_PER_MSEC);
    }
    HANDLE hThread = waitHandles[2] = scan->GetWaitableThreadHandle();

    // Wait for the thread (obj 1) or quit event (obj 0)
    waitStatus = WaitForMultipleObjects(2, (waitHandles+1), FALSE, waitTime);
    CloseHandle(hThread);

    ReleaseDispatcher* releaser = new ReleaseDispatcher(scan);
    if(!releaser)
      continue;
    NS_ADDREF(releaser);
    // Got quit event or error
    if (waitStatus == WAIT_FAILED || waitStatus == WAIT_OBJECT_0) {
      NS_DispatchToMainThread(releaser);
      break;
    // Thread exited normally
    } else if (waitStatus == (WAIT_OBJECT_0+1)) {
      NS_DispatchToMainThread(releaser);
      continue;
    // Timeout case
    } else { 
      NS_ASSERTION(waitStatus == WAIT_TIMEOUT, "Unexpected wait status in dlmgr watchdog thread");
      if (!scan->NotifyTimeout()) {
        // If we didn't time out, then release the thread
        NS_DispatchToMainThread(releaser);
      } else {
        // NotifyTimeout did a dispatch which will release the scan, so we
        // don't need to release the scan
        NS_RELEASE(releaser);
      }
    }
  }
  _endthreadex(0);
  return 0;
}
