/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: se cin sw=2 ts=2 et : */
#ifndef nsDownloadScanner_h_
#define nsDownloadScanner_h_

#ifdef WIN32_LEAN_AND_MEAN
#undef WIN32_LEAN_AND_MEAN
#endif
#define INITGUID
#include <Windows.h>
#define AVVENDOR
#include <msoav.h>
// To cope with both msvs8 header and sdk6 header
#ifdef _WIN32_IE_IE60SP2
#undef _WIN32_IE
#define _WIN32_IE _WIN32_IE_IE60SP2
#endif
#include <shlobj.h>

#include "nsAutoPtr.h"
#include "nsThreadUtils.h"
#include "nsDownloadManager.h"
#include "nsTArray.h"

enum AVScanState
{
  AVSCAN_NOTSTARTED = 0,
  AVSCAN_SCANNING,
  AVSCAN_GOOD,
  AVSCAN_BAD,
  AVSCAN_UGLY,
  AVSCAN_FAILED,
  AVSCAN_TIMEDOUT
};

// See nsDownloadScanner.cpp for declaration and definition
class nsDownloadScannerWatchdog;

class nsDownloadScanner
{
public:
  nsDownloadScanner();
  ~nsDownloadScanner();
  nsresult Init();
  nsresult ScanDownload(nsDownload *download);

private:
  PRBool mHaveAVScanner;
  PRBool mHaveAttachmentExecute;
  nsTArray<CLSID> mScanCLSID;
  PRBool IsAESAvailable();
  PRInt32 ListCLSID();

  nsAutoPtr<nsDownloadScannerWatchdog> mWatchdog;

  static unsigned int __stdcall ScannerThreadFunction(void *p);
  class Scan : public nsRunnable
  {
  public:
    Scan(nsDownloadScanner *scanner, nsDownload *download);
    ~Scan();
    nsresult Start();

    // Returns the time that Start was called
    PRTime GetStartTime() const { return mStartTime; }
    // Returns a copy of the thread handle that can be waited on, but not
    // terminated
    // The caller is responsible for closing the handle
    // If the thread has terminated, then this will return the pseudo-handle
    // INVALID_HANDLE_VALUE
    HANDLE GetWaitableThreadHandle() const;

    // Called on a secondary thread to notify the scan that it has timed out
    // this is used only by the watchdog thread
    PRBool NotifyTimeout();

  private:
    nsDownloadScanner *mDLScanner;
    PRTime mStartTime;
    HANDLE mThread;
    nsRefPtr<nsDownload> mDownload;
    // Guards mStatus
    CRITICAL_SECTION mStateSync;
    AVScanState mStatus;
    nsString mPath;
    nsString mName;
    nsString mOrigin;
    // Also true if it is an ftp download
    PRBool mIsHttpDownload;
    PRBool mIsReadOnlyRequest;

    /* @summary Sets the Scan's state to newState if the current state is
                expectedState
     * @param newState The new state of the scan
     * @param expectedState The state that the caller expects the scan to be in
     * @return If the old state matched expectedState
     */
    PRBool CheckAndSetState(AVScanState newState, AVScanState expectedState);

    NS_IMETHOD Run();

    void DoScan();
    PRBool DoScanAES();
    PRBool DoScanOAV();

    friend unsigned int __stdcall nsDownloadScanner::ScannerThreadFunction(void *);
  };
  // Used to give access to Scan
  friend class nsDownloadScannerWatchdog;
};
#endif
