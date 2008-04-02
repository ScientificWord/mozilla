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

#include "nsAbout.h"
#include "nsIIOService.h"
#include "nsIServiceManager.h"
#include "nsIChannel.h"
#include "nsCOMPtr.h"
#include "nsIURI.h"
#include "nsNetCID.h"
#include "nsIScriptSecurityManager.h"
#include "nsLiteralString.h"

NS_IMPL_ISUPPORTS1(nsAbout, nsIAboutModule)

static const char kURI[] = "chrome://global/content/about.xhtml";

NS_IMETHODIMP
nsAbout::NewChannel(nsIURI *aURI, nsIChannel **result)
{
    nsresult rv;
    nsCOMPtr<nsIIOService> ioService(do_GetService(NS_IOSERVICE_CONTRACTID, &rv));
    NS_ENSURE_SUCCESS(rv, rv);

    nsCOMPtr<nsIChannel> tempChannel;
    rv = ioService->NewChannel(NS_LITERAL_CSTRING(kURI), nsnull, nsnull, 
                               getter_AddRefs(tempChannel));
    NS_ENSURE_SUCCESS(rv, rv);

    nsCOMPtr<nsIScriptSecurityManager> securityManager = 
             do_GetService(NS_SCRIPTSECURITYMANAGER_CONTRACTID, &rv);
    NS_ENSURE_SUCCESS(rv, rv);

    nsCOMPtr<nsIPrincipal> principal;
    rv = securityManager->GetCodebasePrincipal(aURI, getter_AddRefs(principal));
    NS_ENSURE_SUCCESS(rv, rv);

    nsCOMPtr<nsISupports> owner = do_QueryInterface(principal);
    rv = tempChannel->SetOwner(owner);
    *result = tempChannel.get();
    NS_ADDREF(*result);
    return rv;
}

NS_IMETHODIMP
nsAbout::GetURIFlags(nsIURI *aURI, PRUint32 *result)
{
    *result = nsIAboutModule::ALLOW_SCRIPT;
    return NS_OK;
}

NS_METHOD
nsAbout::Create(nsISupports *aOuter, REFNSIID aIID, void **aResult)
{
    nsAbout* about = new nsAbout();
    if (about == nsnull)
        return NS_ERROR_OUT_OF_MEMORY;
    NS_ADDREF(about);
    nsresult rv = about->QueryInterface(aIID, aResult);
    NS_RELEASE(about);
    return rv;
}

