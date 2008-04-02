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
 * Petr Kostka.
 * Portions created by the Initial Developer are Copyright (C) 2007
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

#include "pk11func.h"
#include "nsCOMPtr.h"
#include "nsAutoPtr.h"
#include "nsProxiedService.h"
#include "nsString.h"
#include "nsReadableUtils.h"
#include "nsPKCS11Slot.h"
#include "nsProtectedAuthThread.h"

NS_IMPL_THREADSAFE_ISUPPORTS1(nsProtectedAuthThread, nsIProtectedAuthThread)

static void PR_CALLBACK nsProtectedAuthThreadRunner(void *arg)
{
    nsProtectedAuthThread *self = static_cast<nsProtectedAuthThread *>(arg);
    self->Run();
}

nsProtectedAuthThread::nsProtectedAuthThread()
: mMutex(nsnull)
, mIAmRunning(PR_FALSE)
, mStatusObserverNotified(PR_FALSE)
, mLoginReady(PR_FALSE)
, mThreadHandle(nsnull)
, mSlot(0)
, mLoginResult(SECFailure)
{
    NS_INIT_ISUPPORTS();
    mMutex = PR_NewLock();
}

nsProtectedAuthThread::~nsProtectedAuthThread()
{
    if (mMutex)
        PR_DestroyLock(mMutex);
}

NS_IMETHODIMP nsProtectedAuthThread::Login(nsIObserver *aObserver)
{
    NS_ENSURE_ARG(aObserver);

    if (!mMutex)
        return NS_ERROR_FAILURE;
    
    if (!mSlot)
        // We need pointer to the slot
        return NS_ERROR_FAILURE;

    nsCOMPtr<nsIObserver> observerProxy;
    nsresult rv = NS_GetProxyForObject(NS_PROXY_TO_MAIN_THREAD,
                                       NS_GET_IID(nsIObserver),
                                       aObserver,
                                       NS_PROXY_SYNC | NS_PROXY_ALWAYS,
                                       getter_AddRefs(observerProxy));
    if (NS_FAILED(rv))
        return rv;

    PR_Lock(mMutex);
    
    if (mIAmRunning || mLoginReady) {
        PR_Unlock(mMutex);
        return NS_OK;
    }

    observerProxy.swap(mStatusObserver);
    mIAmRunning = PR_TRUE;
    
    mThreadHandle = PR_CreateThread(PR_USER_THREAD, nsProtectedAuthThreadRunner, static_cast<void*>(this), 
        PR_PRIORITY_NORMAL, PR_LOCAL_THREAD, PR_JOINABLE_THREAD, 0);
    
    // bool thread_started_ok = (threadHandle != nsnull);
    // we might want to return "thread started ok" to caller in the future
    NS_ASSERTION(mThreadHandle, "Could not create nsProtectedAuthThreadRunner thread\n");
    
    PR_Unlock(mMutex);
    
    return NS_OK;
}

NS_IMETHODIMP nsProtectedAuthThread::GetTokenName(nsAString &_retval)
{
    PR_Lock(mMutex);

    // Get token name
    CopyUTF8toUTF16(nsDependentCString(PK11_GetTokenName(mSlot)), _retval);

    PR_Unlock(mMutex);

    return NS_OK;
}

NS_IMETHODIMP nsProtectedAuthThread::GetSlot(nsIPKCS11Slot **_retval)
{
    PR_Lock(mMutex);

    nsRefPtr<nsPKCS11Slot> slot = new nsPKCS11Slot(mSlot);

    PR_Unlock(mMutex);

    if (!slot)
      return NS_ERROR_OUT_OF_MEMORY;

    return CallQueryInterface (slot.get(), _retval);
}

void nsProtectedAuthThread::SetParams(PK11SlotInfo* aSlot)
{
    PR_Lock(mMutex);

    mSlot = (aSlot) ? PK11_ReferenceSlot(aSlot) : 0;
    
    PR_Unlock(mMutex);
}

SECStatus nsProtectedAuthThread::GetResult()
{
    return mLoginResult;
}

void nsProtectedAuthThread::Run(void)
{
    // Login with null password. This call will also do C_Logout() but 
    // it is harmless here
    mLoginResult = PK11_CheckUserPassword(mSlot, 0);

    nsIObserver *observer = nsnull;
    
    PR_Lock(mMutex);
    
    mLoginReady = PR_TRUE;
    mIAmRunning = PR_FALSE;

    // Forget the slot
    if (mSlot)
    {
        PK11_FreeSlot(mSlot);
        mSlot = 0;
    }
    
    if (!mStatusObserverNotified)
    {
        observer = mStatusObserver;
    }

    mStatusObserver = nsnull;
    mStatusObserverNotified = PR_TRUE;
    
    PR_Unlock(mMutex);
    
    if (observer)
        observer->Observe(nsnull, "operation-completed", nsnull);
}

void nsProtectedAuthThread::Join()
{
    if (!mThreadHandle)
        return;
    
    PR_JoinThread(mThreadHandle);
    mThreadHandle = nsnull;
}
