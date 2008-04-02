/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
 *   Bradley Baetz <bbaetz@student.usyd.edu.au>
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

#ifndef __nsFtpState__h_
#define __nsFtpState__h_

#include "ftpCore.h"
#include "nsFTPChannel.h"
#include "nsBaseContentStream.h"

#include "nsInt64.h"
#include "nsIThread.h"
#include "nsIRunnable.h"
#include "nsISocketTransportService.h"
#include "nsISocketTransport.h"
#include "nsIServiceManager.h"
#include "nsIStreamListener.h"
#include "nsICacheListener.h"
#include "nsIURI.h"
#include "prtime.h"
#include "nsString.h"
#include "nsIFTPChannel.h"
#include "nsIProtocolHandler.h"
#include "nsCOMPtr.h"
#include "nsIAsyncInputStream.h"
#include "nsIOutputStream.h"
#include "nsAutoLock.h"
#include "nsAutoPtr.h"
#include "nsIPrompt.h"
#include "nsITransport.h"
#include "nsIProxyInfo.h"

#include "nsFtpControlConnection.h"

#include "nsICacheEntryDescriptor.h"
#include "nsICacheListener.h"

// ftp server types
#define FTP_GENERIC_TYPE     0
#define FTP_UNIX_TYPE        1
#define FTP_VMS_TYPE         8
#define FTP_NT_TYPE          9
#define FTP_OS2_TYPE         11

// ftp states
typedef enum _FTP_STATE {
///////////////////////
//// Internal states
    FTP_INIT,
    FTP_WAIT_CACHE,
    FTP_READ_CACHE,
    FTP_COMMAND_CONNECT,
    FTP_READ_BUF,
    FTP_ERROR,
    FTP_COMPLETE,

///////////////////////
//// Command channel connection setup states
    FTP_S_USER, FTP_R_USER,
    FTP_S_PASS, FTP_R_PASS,
    FTP_S_SYST, FTP_R_SYST,
    FTP_S_ACCT, FTP_R_ACCT,
    FTP_S_TYPE, FTP_R_TYPE,
    FTP_S_CWD,  FTP_R_CWD,
    FTP_S_SIZE, FTP_R_SIZE,
    FTP_S_MDTM, FTP_R_MDTM,
    FTP_S_REST, FTP_R_REST,
    FTP_S_RETR, FTP_R_RETR,
    FTP_S_STOR, FTP_R_STOR,
    FTP_S_LIST, FTP_R_LIST,
    FTP_S_PASV, FTP_R_PASV,
    FTP_S_PWD,  FTP_R_PWD
} FTP_STATE;

// higher level ftp actions
typedef enum _FTP_ACTION {GET, PUT} FTP_ACTION;

class nsFtpChannel;

// The nsFtpState object is the content stream for the channel.  It implements
// nsIInputStreamCallback, so it can read data from the control connection.  It
// implements nsITransportEventSink so it can mix status events from both the
// control connection and the data connection.

class nsFtpState : public nsBaseContentStream,
                   public nsIInputStreamCallback,
                   public nsITransportEventSink,
                   public nsICacheListener,
                   public nsIRequestObserver,
                   public nsFtpControlConnectionListener {
public:
    NS_DECL_ISUPPORTS_INHERITED
    NS_DECL_NSIINPUTSTREAMCALLBACK
    NS_DECL_NSITRANSPORTEVENTSINK
    NS_DECL_NSICACHELISTENER
    NS_DECL_NSIREQUESTOBSERVER

    // Override input stream methods:
    NS_IMETHOD CloseWithStatus(nsresult status);
    NS_IMETHOD Available(PRUint32 *result);
    NS_IMETHOD ReadSegments(nsWriteSegmentFun fun, void *closure,
                            PRUint32 count, PRUint32 *result);

    // nsFtpControlConnectionListener methods:
    virtual void OnControlDataAvailable(const char *data, PRUint32 dataLen);
    virtual void OnControlError(nsresult status);

    nsFtpState();
    nsresult Init(nsFtpChannel *channel);

protected:
    // Notification from nsBaseContentStream::AsyncWait
    virtual void OnCallbackPending();

private:
    virtual ~nsFtpState();

    ///////////////////////////////////
    // BEGIN: STATE METHODS
    nsresult        S_user(); FTP_STATE       R_user();
    nsresult        S_pass(); FTP_STATE       R_pass();
    nsresult        S_syst(); FTP_STATE       R_syst();
    nsresult        S_acct(); FTP_STATE       R_acct();

    nsresult        S_type(); FTP_STATE       R_type();
    nsresult        S_cwd();  FTP_STATE       R_cwd();

    nsresult        S_size(); FTP_STATE       R_size();
    nsresult        S_mdtm(); FTP_STATE       R_mdtm();
    nsresult        S_list(); FTP_STATE       R_list();

    nsresult        S_rest(); FTP_STATE       R_rest();
    nsresult        S_retr(); FTP_STATE       R_retr();
    nsresult        S_stor(); FTP_STATE       R_stor();
    nsresult        S_pasv(); FTP_STATE       R_pasv();
    nsresult        S_pwd();  FTP_STATE       R_pwd();
    // END: STATE METHODS
    ///////////////////////////////////

    // internal methods
    void        MoveToNextState(FTP_STATE nextState);
    nsresult    Process();

    void KillControlConnection();
    nsresult StopProcessing();
    nsresult EstablishControlConnection();
    nsresult SendFTPCommand(const nsCSubstring& command);
    void ConvertFilespecToVMS(nsCString& fileSpec);
    void ConvertDirspecToVMS(nsCString& fileSpec);
    void ConvertDirspecFromVMS(nsCString& fileSpec);
    nsresult BuildStreamConverter(nsIStreamListener** convertStreamListener);
    nsresult SetContentType();

    /**
     * This method is called to kick-off the FTP state machine.  mState is
     * reset to FTP_COMMAND_CONNECT, and the FTP state machine progresses from
     * there.  This method is initially called (indirectly) from the channel's
     * AsyncOpen implementation.
     */
    void Connect();

    /**
     * This method opens a cache entry for reading or writing depending on the
     * state of the channel and of the system (e.g., opened for reading if we
     * are offline).  This method is responsible for setting mCacheEntry if
     * there is a cache entry that can be used.  It returns true if it ends up
     * waiting (asynchronously) for access to the cache entry.  In that case,
     * the nsFtpState's OnCacheEntryAvailable method will be called once the
     * cache entry is available or if an error occurs.
     */
    PRBool CheckCache();

    /**
     * This method returns true if the data for this URL can be read from the
     * cache.  This method assumes that mCacheEntry is non-null.
     */
    PRBool CanReadCacheEntry();

    /**
     * This method causes the cache entry to be read.  Data from the cache
     * entry will be fed to the channel's listener.  This method returns true
     * if successfully reading from the cache.  This method assumes that
     * mCacheEntry is non-null and opened with read access.
     */
    PRBool ReadCacheEntry();

    /**
     * This method configures mDataStream with an asynchronous input stream to
     * the cache entry.  The cache entry is read on a background thread.  This
     * method assumes that mCacheEntry is non-null and opened with read access.
     */
    nsresult OpenCacheDataStream();

    /**
     * This method inserts the cache entry's output stream into the stream
     * listener chain for the FTP channel.  As a result, the cache entry
     * receives data as data is pushed to the channel's listener.  This method
     * assumes that mCacheEntry is non-null and opened with write access. 
     */
    nsresult InstallCacheListener();

    ///////////////////////////////////
    // Private members

        // ****** state machine vars
    FTP_STATE           mState;             // the current state
    FTP_STATE           mNextState;         // the next state
    PRPackedBool        mKeepRunning;       // thread event loop boolean
    PRInt32             mResponseCode;      // the last command response code
    nsCString           mResponseMsg;       // the last command response text

        // ****** channel/transport/stream vars 
    nsRefPtr<nsFtpControlConnection> mControlConnection;       // cacheable control connection (owns mCPipe)
    PRPackedBool                    mReceivedControlData;  
    PRPackedBool                    mTryingCachedControl;     // retrying the password
    PRPackedBool                    mRETRFailed;              // Did we already try a RETR and it failed?
    PRUint64                        mFileSize;
    nsCString                       mModTime;

        // ****** consumer vars
    nsRefPtr<nsFtpChannel>          mChannel;         // our owning FTP channel we pass through our events
    nsCOMPtr<nsIProxyInfo>          mProxyInfo;

        // ****** connection cache vars
    PRInt32             mServerType;    // What kind of server are we talking to

        // ****** protocol interpretation related state vars
    nsString            mUsername;      // username
    nsString            mPassword;      // password
    FTP_ACTION          mAction;        // the higher level action (GET/PUT)
    PRPackedBool        mAnonymous;     // try connecting anonymous (default)
    PRPackedBool        mRetryPass;     // retrying the password
    nsresult            mInternalError; // represents internal state errors

        // ****** URI vars
    PRInt32                mPort;       // the port to connect to
    nsString               mFilename;   // url filename (if any)
    nsCString              mPath;       // the url's path
    nsCString              mPwd;        // login Path

        // ****** other vars
    nsCOMPtr<nsITransport>        mDataTransport;
    nsCOMPtr<nsIAsyncInputStream> mDataStream;
    nsCOMPtr<nsIRequest>    mUploadRequest;
    PRPackedBool            mAddressChecked;
    PRPackedBool            mServerIsIPv6;
    
    static PRUint32         mSessionStartTime;

    char                    mServerAddress[64];

    // ***** control read gvars
    nsresult                mControlStatus;
    nsCString               mControlReadCarryOverBuf;

    nsCOMPtr<nsICacheEntryDescriptor> mCacheEntry;
    
    nsCString mSuppliedEntityID;
};

#endif //__nsFtpState__h_
