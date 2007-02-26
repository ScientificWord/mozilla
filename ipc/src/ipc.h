/*
 * The contents of this file are subject to the Mozilla Public
 * License Version 1.1 (the "MPL"); you may not use this file
 * except in compliance with the MPL. You may obtain a copy of
 * the MPL at http://www.mozilla.org/MPL/
 *
 * Software distributed under the MPL is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the MPL for the specific language governing
 * rights and limitations under the MPL.
 *
 * The Original Code is ipc.
 *
 * The Initial Developer of the Original Code is Patrick Brunschwig
 * Portions created by Patrick Brunschwig <patrick.brunschwig@gmx.net>
 * are Copyright (C) 2005 Patrick Brunschwig. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the
 * terms of the GNU General Public License (the "GPL"), in which case
 * the provisions of the GPL are applicable instead of
 * those above. If you wish to allow use of your version of this
 * file only under the terms of the GPL and not to allow
 * others to use your version of this file under the MPL, indicate
 * your decision by deleting the provisions above and replace them
 * with the notice and other provisions required by the GPL.
 * If you do not delete the provisions above, a recipient
 * may use your version of this file under either the MPL or the
 * GPL.
 */

#ifndef ipc_h__
#define ipc_h__

#define MOZILLA_INTERNAL_API

#if MOZILLA_MAJOR_VERSION==1 && MOZILLA_MINOR_VERSION<9
#define _IPC_MOZILLA_1_8

// some compatibility re-definitions
#define NS_PROXY_SYNC PROXY_SYNC
#define NS_PROXY_ASYNC PROXY_ASYNC
#define NS_PROXY_ALWAYS PROXY_ALWAYS
#define NS_PROXY_TO_CURRENT_THREAD NS_CURRENT_EVENTQ
#define NS_PROXY_TO_MAIN_THREAD NS_UI_THREAD_EVENTQ

#else
#define _IPC_MOZILLA_1_9
#endif

#ifdef FORCE_PR_LOG
#include "nsIThread.h"
#ifdef _IPC_MOZILLA_1_8
// Mozilla 1.8
#define IPC_GET_THREAD(myThread) nsIThread::GetCurrent(getter_AddRefs(myThread))

#else
// Mozilla 1.9
#include "nsThreadUtils.h"

#define IPC_GET_THREAD(myThread) NS_GetCurrentThread(getter_AddRefs(myThread))

#endif
#endif
#endif
