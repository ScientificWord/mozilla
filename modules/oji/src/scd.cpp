/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

////////////////////////////////////////////////////////////////////////////////
// Plugin Manager Methods to support the JVM Plugin API
////////////////////////////////////////////////////////////////////////////////

#include "nsJVMManager.h"

NS_IMPL_AGGREGATED(nsSymantecDebugManager)

nsSymantecDebugManager::nsSymantecDebugManager(nsISupports* outer, nsJVMManager* jvmMgr)
    : fJVMMgr(jvmMgr)
{
    NS_INIT_AGGREGATED(outer);
}

nsSymantecDebugManager::~nsSymantecDebugManager()
{
}

NS_INTERFACE_MAP_BEGIN_AGGREGATED(nsSymantecDebugManager)
    NS_INTERFACE_MAP_ENTRY(nsISymantecDebugManager)
NS_INTERFACE_MAP_END

NS_METHOD
nsSymantecDebugManager::Create(nsISupports* outer, const nsIID& aIID, void* *aInstancePtr, 
                               nsJVMManager* jvmMgr)
{
    if (!aInstancePtr)
        return NS_ERROR_INVALID_POINTER;
    NS_ENSURE_PROPER_AGGREGATION(outer, aIID);

    nsSymantecDebugManager* dbgr = new nsSymantecDebugManager(outer, jvmMgr);
    if (dbgr == NULL)
        return NS_ERROR_OUT_OF_MEMORY;

    nsISupports* inner = dbgr->InnerObject();
    nsresult rv = inner->QueryInterface(aIID, aInstancePtr);
    if (NS_FAILED(rv)) {
       delete dbgr;
    }
    return rv;
}

#if defined(XP_WIN) && defined(_WIN32)
extern "C" HWND FindNavigatorHiddenWindow(void);
#endif

NS_METHOD
nsSymantecDebugManager::SetDebugAgentPassword(PRInt32 pwd)
{
#if defined(XP_WIN) && defined(_WIN32)
    HWND win = NULL;
    /*
    ** TODO:amusil Get to a hidden window for symantec debugger to get its password from.
    HWND win = FindNavigatorHiddenWindow();
    */
    HANDLE sem;
    long err;

    /* set up by aHiddenFrameClass in CNetscapeApp::InitInstance */
    err = SetWindowLong(win, 0, pwd);	
    if (err == 0) {
//        PR_LOG(NSJAVA, PR_LOG_ALWAYS,
//               ("SetWindowLong returned %ld (err=%d)\n", err, GetLastError()));
        /* continue so that we try to wake up the DebugManager */
    }
    sem = OpenSemaphore(SEMAPHORE_MODIFY_STATE, FALSE, "Netscape-Symantec Debugger");
    if (sem) {
        ReleaseSemaphore(sem, 1, NULL);
        CloseHandle(sem);
    }
    return NS_OK;
#else
    return NS_ERROR_FAILURE;
#endif
}

////////////////////////////////////////////////////////////////////////////////
