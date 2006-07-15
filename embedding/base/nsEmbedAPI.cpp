/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * ***** BEGIN LICENSE BLOCK *****
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
 * The Original Code is the Mozilla browser.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications, Inc.
 * Portions created by the Initial Developer are Copyright (C) 1999
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

#include "nsIServiceManager.h"
#include "nsIComponentManager.h"
#include "nsIComponentRegistrar.h"
#include "nsIAppStartupNotifier.h"
#include "nsIStringBundle.h"

#include "nsIDirectoryService.h"
#include "nsDirectoryServiceDefs.h"

#include "nsXPCOM.h"
#include "nsEmbedAPI.h"

static nsIServiceManager *sServiceManager = nsnull;
static PRBool             sRegistryInitializedFlag = PR_FALSE;
static PRUint32           sInitCounter = 0;

#define HACK_AROUND_THREADING_ISSUES
//#define HACK_AROUND_NONREENTRANT_INITXPCOM

#ifdef HACK_AROUND_NONREENTRANT_INITXPCOM
// XXX hack class to clean up XPCOM when this module is unloaded
class XPCOMCleanupHack
{
public:
    PRBool mCleanOnExit;

    XPCOMCleanupHack() : mCleanOnExit(PR_FALSE) {}
    ~XPCOMCleanupHack()
    {
        if (mCleanOnExit)
        {
            if (sInitCounter > 0)
            {
                sInitCounter = 1;
                NS_TermEmbedding();
            }
            // XXX Global destructors and NS_ShutdownXPCOM don't seem to mix
//          NS_ShutdownXPCOM(sServiceManager);
        }
    }
};
static PRBool sXPCOMInitializedFlag = PR_FALSE;
static XPCOMCleanupHack sXPCOMCleanupHack;
#endif


NS_METHOD
NS_InitEmbedding(nsILocalFile *mozBinDirectory,
                 nsIDirectoryServiceProvider *appFileLocProvider,
                 nsStaticModuleInfo const *aStaticComponents,
                 PRUint32 aStaticComponentCount)
{
    nsresult rv;

    // Reentrant calls to this method do nothing except increment a counter
    sInitCounter++;
    if (sInitCounter > 1)
        return NS_OK;

    // Initialise XPCOM
#ifdef HACK_AROUND_NONREENTRANT_INITXPCOM
    // Can't call NS_InitXPCom more than once or things go boom!
    if (!sXPCOMInitializedFlag)
#endif
    {
        // Initialise XPCOM
        rv = NS_InitXPCOM3(&sServiceManager, mozBinDirectory, appFileLocProvider,
                           aStaticComponents, aStaticComponentCount);
        NS_ENSURE_SUCCESS(rv, rv);
                
#ifdef HACK_AROUND_NONREENTRANT_INITXPCOM
        sXPCOMInitializedFlag = PR_TRUE;
        sXPCOMCleanupHack.mCleanOnExit = PR_TRUE;
#endif
    }
    // Register components
    if (!sRegistryInitializedFlag)
    {
#ifdef DEBUG
        nsIComponentRegistrar *registrar;
        rv = sServiceManager->QueryInterface(NS_GET_IID(nsIComponentRegistrar),
                                             (void **) &registrar);
        if (NS_FAILED(rv))
        {
            NS_WARNING("Could not QI to registrar");
            return rv;
        }
        rv = registrar->AutoRegister(nsnull);
        if (NS_FAILED(rv))
        {
            NS_WARNING("Could not AutoRegister");
        }
        else
        {
            // If the application is using an GRE, then, auto register components
            // in the GRE directory as well.
            //
            // The application indicates that it's using an GRE by returning a
            // valid nsIFile when queried (via appFileLocProvider) for the
            // NS_GRE_DIR atom as shown below

            if (appFileLocProvider)
            {
                nsIFile *greDir = nsnull;
                PRBool persistent = PR_TRUE;

                appFileLocProvider->GetFile(NS_GRE_DIR, &persistent,
                                            &greDir);
                if (greDir)
                {
                    rv = registrar->AutoRegister(greDir);
                    if (NS_FAILED(rv))
                        NS_WARNING("Could not AutoRegister GRE components");
                    NS_RELEASE(greDir);
                }
            }
        }
        NS_RELEASE(registrar);
        if (NS_FAILED(rv))
            return rv;
#endif
        sRegistryInitializedFlag = PR_TRUE;
    }

    nsIComponentManager *compMgr;
    rv = sServiceManager->QueryInterface(NS_GET_IID(nsIComponentManager),
                                         (void **) &compMgr);
    if (NS_FAILED(rv))
        return rv;

    nsIObserver *startupNotifier;
    rv = compMgr->CreateInstanceByContractID(NS_APPSTARTUPNOTIFIER_CONTRACTID,
                                             NULL,
                                             NS_GET_IID(nsIObserver),
                                             (void **) &startupNotifier);
    NS_RELEASE(compMgr);
    if (NS_FAILED(rv))
        return rv;

	  startupNotifier->Observe(nsnull, APPSTARTUP_TOPIC, nsnull);
    NS_RELEASE(startupNotifier);

#ifdef HACK_AROUND_THREADING_ISSUES
    // XXX force certain objects to be created on the main thread
    nsIStringBundleService *bundleService;
    rv = sServiceManager->GetServiceByContractID(NS_STRINGBUNDLE_CONTRACTID,
                                                 NS_GET_IID(nsIStringBundleService),
                                                 (void **) &bundleService);
    if (NS_SUCCEEDED(rv))
    {
        nsIStringBundle *stringBundle;
        const char propertyURL[] = "chrome://necko/locale/necko.properties";
        rv = bundleService->CreateBundle(propertyURL, &stringBundle);
        NS_RELEASE(stringBundle);
        NS_RELEASE(bundleService);
    }
#endif

    return NS_OK;
}

NS_METHOD
NS_TermEmbedding()
{
    // Reentrant calls to this method do nothing except decrement a counter
    if (sInitCounter > 1)
    {
        sInitCounter--;
        return NS_OK;
    }
    sInitCounter = 0;

    NS_IF_RELEASE(sServiceManager);

    // Terminate XPCOM & cleanup
#ifndef HACK_AROUND_NONREENTRANT_INITXPCOM
    nsresult rv = NS_ShutdownXPCOM(sServiceManager);
    NS_ENSURE_SUCCESS(rv, rv);
#endif

    return NS_OK;
}
