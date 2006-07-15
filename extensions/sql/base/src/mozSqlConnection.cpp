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
 * The Initial Developer of the Original Code is Jan Varga
 * Portions created by the Initial Developer are Copyright (C) 2003
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
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

#include "nsIProxyObjectManager.h"
#include "mozSqlRequest.h"
#include "mozSqlConnection.h"
#include "nsThreadUtils.h"

mozSqlConnection::mozSqlConnection()
  : mLock(nsnull),
    mCondVar(nsnull),
    mThread(nsnull),
    mShutdown(PR_FALSE),
    mWaiting(PR_FALSE)
{
  mExecLock = PR_NewLock();
}

mozSqlConnection::~mozSqlConnection()
{
  mRequests.Clear();

  if (mCondVar)
    PR_DestroyCondVar(mCondVar);
  PR_DestroyLock(mExecLock);
  if (mLock)
    PR_DestroyLock(mLock);
}

// We require a special implementation of Release, which knows about
// a circular strong reference
NS_IMPL_THREADSAFE_ADDREF(mozSqlConnection)
NS_IMPL_THREADSAFE_QUERY_INTERFACE3(mozSqlConnection,
                                    mozISqlConnection,
                                    nsIRunnable,
				    nsISupportsWeakReference)
NS_IMETHODIMP_(nsrefcnt)
mozSqlConnection::Release()
{
  PR_AtomicDecrement((PRInt32*)&mRefCnt);
  // Delete if the last reference is our strong circular reference.
  if (mThread && mRefCnt == 1) {
    PR_Lock(mLock);
    mRequests.Clear();
    mShutdown = PR_TRUE;
    if (mWaiting)
      PR_NotifyCondVar(mCondVar);
    else
      CancelExec();
    PR_Unlock(mLock);
    return 0;
  }
  else if (mRefCnt == 0) {
    delete this;
    return 0;
  }
  return mRefCnt;
}

NS_IMETHODIMP
mozSqlConnection::GetServerVersion(nsAString& aServerVersion) 
{
  aServerVersion = mServerVersion;
  return NS_OK;
}

NS_IMETHODIMP
mozSqlConnection::GetErrorMessage(nsAString& aErrorMessage) 
{
  aErrorMessage = mErrorMessage;
  return NS_OK;
}

NS_IMETHODIMP
mozSqlConnection::GetLastID(PRInt32* aLastID) 
{
  *aLastID = mLastID;
  return NS_OK;
}

NS_IMETHODIMP
mozSqlConnection::Init(const nsAString & aHost, PRInt32 aPort,
                       const nsAString & aDatabase, const nsAString & aUsername,
                       const nsAString & aPassword)
{
  // descendants have to implement this themselves
  return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP
mozSqlConnection::ExecuteQuery(const nsAString& aQuery, mozISqlResult** _retval)
{
  PR_Lock(mExecLock);
  nsresult rv = RealExec(aQuery, _retval, nsnull);
  PR_Unlock(mExecLock);
  return rv;
}

NS_IMETHODIMP
mozSqlConnection::ExecuteUpdate(const nsAString& aUpdate, PRInt32* _retval)
{
  PR_Lock(mExecLock);
  nsresult rv = RealExec(aUpdate, nsnull, _retval);
  PR_Unlock(mExecLock);
  return rv;
}

NS_IMETHODIMP
mozSqlConnection::AsyncExecuteQuery(const nsAString& aQuery, nsISupports* aCtxt, 
                                    mozISqlRequestObserver* aObserver,
                                    mozISqlRequest **_retval)
{
  if (!mThread) {
    mLock = PR_NewLock();
    mCondVar = PR_NewCondVar(mLock);
    NS_NewThread(getter_AddRefs(mThread), this);
  }

  mozSqlRequest* request = new mozSqlRequest(this);
  if (! request)
    return NS_ERROR_OUT_OF_MEMORY;

  request->mIsQuery = PR_TRUE;
  request->mQuery = aQuery;
  request->mCtxt = aCtxt;

  nsresult rv = NS_GetProxyForObject(NS_PROXY_TO_CURRENT_THREAD,
                                     NS_GET_IID(mozISqlRequestObserver),
                                     aObserver,
                                     NS_PROXY_SYNC | NS_PROXY_ALWAYS,
                                     getter_AddRefs(request->mObserver));
  if (NS_FAILED(rv))
    return rv;

  PR_Lock(mLock);
  mRequests.AppendObject(request);
  if (mWaiting && mRequests.Count() == 1)
    PR_NotifyCondVar(mCondVar);
  PR_Unlock(mLock);

  NS_ADDREF(*_retval = request);

  return NS_OK;
}

NS_IMETHODIMP
mozSqlConnection::AsyncExecuteUpdate(const nsAString& aQuery, nsISupports* aCtxt, 
                                     mozISqlRequestObserver* aObserver,
                                     mozISqlRequest **_retval)
{
  return NS_OK;
}

NS_IMETHODIMP
mozSqlConnection::BeginTransaction()
{
  PRInt32 affectedRows;
  return ExecuteUpdate(NS_LITERAL_STRING("begin"), &affectedRows);
}

NS_IMETHODIMP
mozSqlConnection::CommitTransaction()
{
  PRInt32 affectedRows;
  return ExecuteUpdate(NS_LITERAL_STRING("commit"), &affectedRows);
}

NS_IMETHODIMP
mozSqlConnection::RollbackTransaction()
{
  PRInt32 affectedRows;
  return ExecuteUpdate(NS_LITERAL_STRING("rollback"), &affectedRows);
}

NS_IMETHODIMP
mozSqlConnection::GetPrimaryKeys(const nsAString& aSchema, const nsAString& aTable, mozISqlResult** _retval)
{
  // descendants have to implement this themselves
  return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP
mozSqlConnection::Run()
{
  PR_Lock(mLock);
  while(!mShutdown) {

    while (mRequests.Count()) {
      mCurrentRequest = mRequests[0];
      mRequests.RemoveObjectAt(0);


      mozSqlRequest* r = (mozSqlRequest*)mCurrentRequest.get();

      PR_Unlock(mLock);
      r->mObserver->OnStartRequest(mCurrentRequest, r->mCtxt);
      PR_Lock(mLock);

      r->mStatus = mozISqlRequest::STATUS_EXECUTED;

      PR_Unlock(mLock);

      nsresult rv = ExecuteQuery(r->mQuery, getter_AddRefs(r->mResult));

      PR_Lock(mLock);

      if (NS_SUCCEEDED(rv))
        r->mStatus = mozISqlRequest::STATUS_COMPLETE;
      else {
        r->mStatus = mozISqlRequest::STATUS_ERROR;
	GetErrorMessage(r->mErrorMessage);
      }
        
      PR_Unlock(mLock);
      r->mObserver->OnStopRequest(mCurrentRequest, r->mCtxt);
      PR_Lock(mLock);

      mCurrentRequest = nsnull;

    }
    
    mWaiting = PR_TRUE;
    PR_WaitCondVar(mCondVar, PR_INTERVAL_NO_TIMEOUT);
    mWaiting = PR_FALSE;
  }
  PR_Unlock(mLock);

  // Shutdown self from main thread (cannot shutdown directly)
  nsCOMPtr<nsIThread> proxy;
  NS_GetProxyForObject(NS_PROXY_TO_MAIN_THREAD,
                       NS_GET_IID(nsIThread),
                       NS_GetCurrentThread(),
                       NS_PROXY_ASYNC,
                       getter_AddRefs(proxy));
  if (proxy) {
    proxy->Shutdown();
  } else {
    NS_WARNING("leaking thread");
  }
  return NS_OK;
}

nsresult
mozSqlConnection::CancelRequest(mozISqlRequest* aRequest)
{
  PR_Lock(mLock);
  if (mCurrentRequest == aRequest)
    CancelExec();
  else {
    if (mRequests.RemoveObject(aRequest))
      ((mozSqlRequest*)aRequest)->mStatus = mozISqlRequest::STATUS_CANCELLED;
  }
  PR_Unlock(mLock);

  return NS_OK;
}
