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
 *   Ben Turner <mozilla@songbirdnest.com
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

#include "nsCOMPtr.h"
#include "nsString.h"
#include "nsXPIDLString.h"
#include "nsIServiceManager.h"
#include "nsICategoryManager.h"
#include "nsXPCOM.h"
#include "nsISupportsPrimitives.h"
#include "nsAppStartupNotifier.h"

NS_IMPL_ISUPPORTS1(nsAppStartupNotifier, nsIObserver)

nsAppStartupNotifier::nsAppStartupNotifier()
{
}

nsAppStartupNotifier::~nsAppStartupNotifier()
{
}

NS_IMETHODIMP nsAppStartupNotifier::Observe(nsISupports *aSubject, const char *aTopic, const PRUnichar *someData)
{
    NS_ENSURE_ARG(aTopic);
    nsresult rv;

    // now initialize all startup listeners
    nsCOMPtr<nsICategoryManager> categoryManager =
                    do_GetService(NS_CATEGORYMANAGER_CONTRACTID, &rv);
    NS_ENSURE_SUCCESS(rv, rv);

    nsCOMPtr<nsISimpleEnumerator> enumerator;
    rv = categoryManager->EnumerateCategory(aTopic,
                               getter_AddRefs(enumerator));
    if (NS_FAILED(rv)) return rv;

    nsCOMPtr<nsISupports> entry;
    while (NS_SUCCEEDED(enumerator->GetNext(getter_AddRefs(entry)))) {
        nsCOMPtr<nsISupportsCString> category = do_QueryInterface(entry, &rv);

        if (NS_SUCCEEDED(rv)) {
            nsCAutoString categoryEntry;
            rv = category->GetData(categoryEntry);

            nsXPIDLCString contractId;
            categoryManager->GetCategoryEntry(aTopic, 
                                              categoryEntry.get(),
                                              getter_Copies(contractId));

            if (NS_SUCCEEDED(rv)) {

                // If we see the word "service," in the beginning
                // of the contractId then we create it as a service
                // if not we do a createInstance

                nsCOMPtr<nsISupports> startupInstance;
                if (Substring(contractId, 0, 8).EqualsLiteral("service,"))
                    startupInstance = do_GetService(contractId.get() + 8, &rv);
                else
                    startupInstance = do_CreateInstance(contractId, &rv);

                if (NS_SUCCEEDED(rv)) {
                    // Try to QI to nsIObserver
                    nsCOMPtr<nsIObserver> startupObserver =
                        do_QueryInterface(startupInstance, &rv);
                    if (NS_SUCCEEDED(rv)) {
                        rv = startupObserver->Observe(nsnull, aTopic, nsnull);
     
                        // mainly for debugging if you want to know if your observer worked.
                        NS_ASSERTION(NS_SUCCEEDED(rv), "Startup Observer failed!\n");
                    }
                }
                else {
                  #ifdef NS_DEBUG
                    nsCAutoString warnStr("Cannot create startup observer : ");
                    warnStr += contractId.get();
                    NS_WARNING(warnStr.get());
                  #endif
                }
            }
        }
    }

    return NS_OK;
}
