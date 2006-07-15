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
 *   Pierre Phaneuf <pp@ludusdesign.com>
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

#include "MyService.h"
#include "nsXPCOM.h"
#include "nsIServiceManager.h"
#include "nsIComponentManager.h"
#include "nsServiceManagerUtils.h"
#include <stdio.h>

static NS_DEFINE_CID(kIMyServiceCID, NS_IMYSERVICE_CID);

////////////////////////////////////////////////////////////////////////////////

IMyService* myServ = NULL;

nsresult
BeginTest(int testNumber)
{
    nsresult err;
    NS_ASSERTION(myServ == NULL, "myServ not reset");
    err = CallGetService(kIMyServiceCID, &myServ);
    return err;
}

nsresult
EndTest(int testNumber)
{
    nsresult err = NS_OK;

    if (myServ) {
        err = myServ->Doit();
        if (err != NS_OK) return err;

        NS_RELEASE(myServ);
    }
    
    printf("test %d succeeded\n", testNumber);
    return NS_OK;
}

nsresult
SimpleTest(int testNumber)
{
    // This test just gets a service, uses it and releases it.

    nsresult err;
    err = BeginTest(testNumber);
    if (err != NS_OK) return err;
    err = EndTest(testNumber);
    return err;
}

////////////////////////////////////////////////////////////////////////////////

nsresult
AsyncShutdown(int testNumber)
{
    nsresult err = NS_OK;

    // If the AsyncShutdown was truly asynchronous and happened on another
    // thread, we'd have to protect all accesses to myServ throughout this
    // code with a monitor.

    // XXX-darin: say what?!?

    /*
    err = nsServiceManager::UnregisterService(kIMyServiceCID);
    if (err == NS_ERROR_SERVICE_NOT_AVAILABLE) {
        printf("async shutdown -- service not found\n");
        return NS_OK;
    }
    */

    return err;
}

nsresult
AsyncNoShutdownTest(int testNumber)
{
    // This test gets a service, and also gets an async request for shutdown,
    // but the service doesn't get shut down because some other client (who's
    // not participating in the async shutdown game as he should) is 
    // continuing to hang onto the service. This causes myServ variable to 
    // get set to NULL, but the service doesn't get unloaded right away as
    // it should.

    nsresult err;

    err = BeginTest(testNumber);
    if (err != NS_OK) return err;

    // Create some other user of kIMyServiceCID, preventing it from
    // really going away:
    IMyService* otherClient;
    err = CallGetService(kIMyServiceCID, &otherClient);
    if (err != NS_OK) return err;

    err = AsyncShutdown(testNumber);
    if (err != NS_OK) return err;
    err = EndTest(testNumber);

    // Finally, release the other client.
    NS_RELEASE(otherClient);

    return err;
}

////////////////////////////////////////////////////////////////////////////////
int
main(void)
{
    nsresult err;
    int testNumber = 0;

    err = NS_InitXPCOM2(nsnull, nsnull, nsnull);
    if (NS_FAILED(err)) {
        printf("NS_InitXPCOM2 failed\n");
        return -1;
    }

    err = SimpleTest(++testNumber);
    if (err != NS_OK)
        goto error;

    err = AsyncNoShutdownTest(++testNumber);
    if (err != NS_OK)
        goto error;

    AsyncShutdown(++testNumber);

    printf("there was great success\n");
    return 0;

  error:
    printf("test %d failed\n", testNumber);
    return -1;
}

////////////////////////////////////////////////////////////////////////////////
