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
 * The Initial Developer of the Original Code is IBM Corporation.
 * Portions created by IBM Corporation are Copyright (C) 2003
 * IBM Corporation. All Rights Reserved.
 *
 * Contributor(s):
 *   IBM Corp.
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

#include "TestCommon.h"
#include "nsXPCOM.h"
#include "nsIServiceManager.h"
#include "nsServiceManagerUtils.h"
#include "nsIEventTarget.h"
#include "nsCOMPtr.h"
#include "nsNetCID.h"
#include "prlog.h"

#if defined(PR_LOGGING)
//
// set NSPR_LOG_MODULES=Test:5
//
static PRLogModuleInfo *gTestLog = nsnull;
#endif
#define LOG(args) PR_LOG(gTestLog, PR_LOG_DEBUG, args)

class nsIOEvent : public nsIRunnable {
public:
    NS_DECL_ISUPPORTS

    nsIOEvent(int i) : mIndex(i) {}

    NS_IMETHOD Run() {
        LOG(("Run [%d]\n", mIndex));
        return NS_OK;
    }

private:
    int mIndex;
};
NS_IMPL_THREADSAFE_ISUPPORTS1(nsIOEvent, nsIRunnable)

static nsresult RunTest()
{
    nsresult rv;
    nsCOMPtr<nsIEventTarget> target =
        do_GetService(NS_STREAMTRANSPORTSERVICE_CONTRACTID, &rv);
    if (NS_FAILED(rv))
        return rv;

    for (int i=0; i<10; ++i) {
        nsCOMPtr<nsIRunnable> event = new nsIOEvent(i); 
        LOG(("Dispatch %d\n", i));
        target->Dispatch(event, NS_DISPATCH_NORMAL);
    }

    return NS_OK;
}

int main(int argc, char **argv)
{
    if (test_common_init(&argc, &argv) != 0)
        return -1;

    nsresult rv;

#if defined(PR_LOGGING)
    gTestLog = PR_NewLogModule("Test");
#endif

    rv = NS_InitXPCOM2(nsnull, nsnull, nsnull);
    if (NS_FAILED(rv))
        return rv;

    rv = RunTest();
    if (NS_FAILED(rv))
        LOG(("RunTest failed [rv=%x]\n", rv));

    LOG(("sleeping main thread for 2 seconds...\n"));
    PR_Sleep(PR_SecondsToInterval(2));
    
    NS_ShutdownXPCOM(nsnull);
    return 0;
}
