/* vim:set ts=4 sw=4 sts=4 et cin: */
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
 * The Original Code is Mozilla.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2002
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Darin Fisher <darin@netscape.com>
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

#ifndef nsHttpConnectionMgr_h__
#define nsHttpConnectionMgr_h__

#include "nsHttpConnectionInfo.h"
#include "nsHttpConnection.h"
#include "nsHttpTransaction.h"
#include "nsVoidArray.h"
#include "nsThreadUtils.h"
#include "nsHashtable.h"
#include "nsAutoPtr.h"
#include "prmon.h"

class nsHttpPipeline;

//-----------------------------------------------------------------------------

class nsHttpConnectionMgr
{
public:

    // parameter names
    enum nsParamName {
        MAX_CONNECTIONS,
        MAX_CONNECTIONS_PER_HOST,
        MAX_CONNECTIONS_PER_PROXY,
        MAX_PERSISTENT_CONNECTIONS_PER_HOST,
        MAX_PERSISTENT_CONNECTIONS_PER_PROXY,
        MAX_REQUEST_DELAY,
        MAX_PIPELINED_REQUESTS
    };

    //-------------------------------------------------------------------------
    // NOTE: functions below may only be called on the main thread.
    //-------------------------------------------------------------------------

    nsHttpConnectionMgr();

    nsresult Init(PRUint16 maxConnections,
                  PRUint16 maxConnectionsPerHost,
                  PRUint16 maxConnectionsPerProxy,
                  PRUint16 maxPersistentConnectionsPerHost,
                  PRUint16 maxPersistentConnectionsPerProxy,
                  PRUint16 maxRequestDelay,
                  PRUint16 maxPipelinedRequests);
    nsresult Shutdown();

    //-------------------------------------------------------------------------
    // NOTE: functions below may be called on any thread.
    //-------------------------------------------------------------------------

    nsrefcnt AddRef()
    {
        return PR_AtomicIncrement(&mRef);
    }

    nsrefcnt Release()
    {
        nsrefcnt n = PR_AtomicDecrement(&mRef);
        if (n == 0)
            delete this;
        return n;
    }

    // adds a transaction to the list of managed transactions.
    nsresult AddTransaction(nsHttpTransaction *, PRInt32 priority);

    // called to reschedule the given transaction.  it must already have been
    // added to the connection manager via AddTransaction.
    nsresult RescheduleTransaction(nsHttpTransaction *, PRInt32 priority);

    // cancels a transaction w/ the given reason.
    nsresult CancelTransaction(nsHttpTransaction *, nsresult reason);

    // called to force the connection manager to prune its list of idle
    // connections.
    nsresult PruneDeadConnections();

    // called to get a reference to the socket transport service.  the socket
    // transport service is not available when the connection manager is down.
    nsresult GetSocketThreadTarget(nsIEventTarget **);

    // called when a connection is done processing a transaction.  if the 
    // connection can be reused then it will be added to the idle list, else
    // it will be closed.
    nsresult ReclaimConnection(nsHttpConnection *conn);

    // called to update a parameter after the connection manager has already
    // been initialized.
    nsresult UpdateParam(nsParamName name, PRUint16 value);

    //-------------------------------------------------------------------------
    // NOTE: functions below may be called only on the socket thread.
    //-------------------------------------------------------------------------

    // removes the next transaction for the specified connection from the
    // pending transaction queue.
    void AddTransactionToPipeline(nsHttpPipeline *);

    // called to force the transaction queue to be processed once more, giving
    // preference to the specified connection.
    nsresult ProcessPendingQ(nsHttpConnectionInfo *);

private:
    virtual ~nsHttpConnectionMgr();

    // nsConnectionEntry
    //
    // mCT maps connection info hash key to nsConnectionEntry object, which
    // contains list of active and idle connections as well as the list of
    // pending transactions.
    //
    struct nsConnectionEntry
    {
        nsConnectionEntry(nsHttpConnectionInfo *ci)
            : mConnInfo(ci)
        {
            NS_ADDREF(mConnInfo);
        }
       ~nsConnectionEntry() { NS_RELEASE(mConnInfo); }

        nsHttpConnectionInfo *mConnInfo;
        nsVoidArray           mPendingQ;    // pending transaction queue
        nsVoidArray           mActiveConns; // active connections
        nsVoidArray           mIdleConns;   // idle persistent connections
    };

    // nsConnectionHandle
    //
    // thin wrapper around a real connection, used to keep track of references
    // to the connection to determine when the connection may be reused.  the
    // transaction (or pipeline) owns a reference to this handle.  this extra
    // layer of indirection greatly simplifies consumer code, avoiding the
    // need for consumer code to know when to give the connection back to the
    // connection manager.
    //
    class nsConnectionHandle : public nsAHttpConnection
    {
    public:
        NS_DECL_ISUPPORTS
        NS_DECL_NSAHTTPCONNECTION

        nsConnectionHandle(nsHttpConnection *conn) { NS_ADDREF(mConn = conn); }
        virtual ~nsConnectionHandle();

        nsHttpConnection *mConn;
    };

    //-------------------------------------------------------------------------
    // NOTE: these members may be accessed from any thread (use mMonitor)
    //-------------------------------------------------------------------------

    PRInt32                      mRef;
    PRMonitor                   *mMonitor;
    nsCOMPtr<nsIEventTarget>     mSocketThreadTarget;

    // connection limits
    PRUint16 mMaxConns;
    PRUint16 mMaxConnsPerHost;
    PRUint16 mMaxConnsPerProxy;
    PRUint16 mMaxPersistConnsPerHost;
    PRUint16 mMaxPersistConnsPerProxy;
    PRUint16 mMaxRequestDelay; // in seconds
    PRUint16 mMaxPipelinedRequests;

    //-------------------------------------------------------------------------
    // NOTE: these members are only accessed on the socket transport thread
    //-------------------------------------------------------------------------

    static PRIntn PR_CALLBACK ProcessOneTransactionCB(nsHashKey *, void *, void *);
    static PRIntn PR_CALLBACK PurgeOneIdleConnectionCB(nsHashKey *, void *, void *);
    static PRIntn PR_CALLBACK PruneDeadConnectionsCB(nsHashKey *, void *, void *);
    static PRIntn PR_CALLBACK ShutdownPassCB(nsHashKey *, void *, void *);

    PRBool   ProcessPendingQForEntry(nsConnectionEntry *);
    PRBool   AtActiveConnectionLimit(nsConnectionEntry *, PRUint8 caps);
    void     GetConnection(nsConnectionEntry *, PRUint8 caps, nsHttpConnection **);
    nsresult DispatchTransaction(nsConnectionEntry *, nsAHttpTransaction *,
                                 PRUint8 caps, nsHttpConnection *);
    PRBool   BuildPipeline(nsConnectionEntry *, nsAHttpTransaction *, nsHttpPipeline **);
    nsresult ProcessNewTransaction(nsHttpTransaction *);

    // message handlers have this signature
    typedef void (nsHttpConnectionMgr:: *nsConnEventHandler)(PRInt32, void *);

    // nsConnEvent
    //
    // subclass of nsRunnable used to marshall events to the socket transport
    // thread.  this class is used to implement PostEvent.
    //
    class nsConnEvent;
    friend class nsConnEvent;
    class nsConnEvent : public nsRunnable
    {
    public:
        nsConnEvent(nsHttpConnectionMgr *mgr,
                    nsConnEventHandler handler,
                    PRInt32 iparam,
                    void *vparam)
            : mMgr(mgr)
            , mHandler(handler)
            , mIParam(iparam)
            , mVParam(vparam)
        {
            NS_ADDREF(mMgr);
        }

        NS_IMETHOD Run()
        {
            (mMgr->*mHandler)(mIParam, mVParam);
            return NS_OK;
        }

    private:
        virtual ~nsConnEvent()
        {
            NS_RELEASE(mMgr);
        }

        nsHttpConnectionMgr *mMgr;
        nsConnEventHandler   mHandler;
        PRInt32              mIParam;
        void                *mVParam;
    };

    nsresult PostEvent(nsConnEventHandler  handler,
                       PRInt32             iparam = 0,
                       void               *vparam = nsnull);

    // message handlers
    void OnMsgShutdown             (PRInt32, void *);
    void OnMsgNewTransaction       (PRInt32, void *);
    void OnMsgReschedTransaction   (PRInt32, void *);
    void OnMsgCancelTransaction    (PRInt32, void *);
    void OnMsgProcessPendingQ      (PRInt32, void *);
    void OnMsgPruneDeadConnections (PRInt32, void *);
    void OnMsgReclaimConnection    (PRInt32, void *);
    void OnMsgUpdateParam          (PRInt32, void *);

    // counters
    PRUint16 mNumActiveConns;
    PRUint16 mNumIdleConns;

    //
    // the connection table
    //
    // this table is indexed by connection key.  each entry is a
    // ConnectionEntry object.
    //
    nsHashtable mCT;
};

#endif // !nsHttpConnectionMgr_h__
