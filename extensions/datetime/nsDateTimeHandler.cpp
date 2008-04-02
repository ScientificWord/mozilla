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

#include "nspr.h"
#include "nsDateTimeChannel.h"
#include "nsDateTimeHandler.h"
#include "nsIURL.h"
#include "nsCRT.h"
#include "nsIComponentManager.h"
#include "nsIServiceManager.h"
#include "nsIInterfaceRequestor.h"
#include "nsIInterfaceRequestorUtils.h"
#include "nsIProgressEventSink.h"
#include "nsNetCID.h"

////////////////////////////////////////////////////////////////////////////////

nsDateTimeHandler::nsDateTimeHandler() {
}

nsDateTimeHandler::~nsDateTimeHandler() {
}

NS_IMPL_ISUPPORTS2(nsDateTimeHandler,
                   nsIProtocolHandler,
                   nsIProxiedProtocolHandler)

NS_METHOD
nsDateTimeHandler::Create(nsISupports* aOuter, const nsIID& aIID, void* *aResult) {

    nsDateTimeHandler* ph = new nsDateTimeHandler();
    if (ph == nsnull)
        return NS_ERROR_OUT_OF_MEMORY;
    NS_ADDREF(ph);
    nsresult rv = ph->QueryInterface(aIID, aResult);
    NS_RELEASE(ph);
    return rv;
}
    
////////////////////////////////////////////////////////////////////////////////
// nsIProtocolHandler methods:

NS_IMETHODIMP
nsDateTimeHandler::GetScheme(nsACString &result) {
    result = "datetime";
    return NS_OK;
}

NS_IMETHODIMP
nsDateTimeHandler::GetDefaultPort(PRInt32 *result) {
    *result = DATETIME_PORT;
    return NS_OK;
}

NS_IMETHODIMP
nsDateTimeHandler::GetProtocolFlags(PRUint32 *result) {
    *result = URI_NORELATIVE | URI_NOAUTH | ALLOWS_PROXY | URI_DANGEROUS_TO_LOAD;
    return NS_OK;
}

NS_IMETHODIMP
nsDateTimeHandler::NewURI(const nsACString &aSpec,
                          const char *aCharset, // ignore charset info
                          nsIURI *aBaseURI,
                          nsIURI **result) {
    nsresult rv;

    nsIURI* url;
    rv = CallCreateInstance(NS_SIMPLEURI_CONTRACTID, &url);
    if (NS_FAILED(rv)) return rv;

    rv = url->SetSpec(aSpec);
    if (NS_FAILED(rv)) {
        NS_RELEASE(url);
        return rv;
    }

    *result = url;
    return rv;
}

NS_IMETHODIMP
nsDateTimeHandler::NewChannel(nsIURI* url, nsIChannel* *result)
{
    return NewProxiedChannel(url, nsnull, result);
}

NS_IMETHODIMP
nsDateTimeHandler::NewProxiedChannel(nsIURI* url, nsIProxyInfo* proxyInfo,
                                     nsIChannel* *result)
{
    NS_ENSURE_ARG_POINTER(url);
    nsresult rv;
    
    nsDateTimeChannel *chan = new nsDateTimeChannel();
    if (!chan)
        return NS_ERROR_OUT_OF_MEMORY;
    NS_ADDREF(chan);

    rv = chan->Init(url, proxyInfo);
    if (NS_FAILED(rv)) {
        NS_RELEASE(chan);
        return rv;
    }

    *result = chan;
    return NS_OK;
}


NS_IMETHODIMP 
nsDateTimeHandler::AllowPort(PRInt32 port, const char *scheme, PRBool *_retval)
{
    if (port == DATETIME_PORT)
        *_retval = PR_TRUE;
    else
        *_retval = PR_FALSE;
    return NS_OK;
}
////////////////////////////////////////////////////////////////////////////////
