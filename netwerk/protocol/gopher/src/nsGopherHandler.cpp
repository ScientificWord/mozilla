
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
 * The Original Code is the Gopher protocol code.
 *
 * The Initial Developer of the Original Code is
 * Bradley Baetz.
 * Portions created by the Initial Developer are Copyright (C) 2000
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Bradley Baetz <bbaetz@student.usyd.edu.au>
 *   Darin Fisher <darin@meer.net>
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

#include "nsGopherChannel.h"
#include "nsGopherHandler.h"
#include "nsIURL.h"
#include "nsIComponentManager.h"
#include "nsIServiceManager.h"
#include "nsIStandardURL.h"
#include "nsStandardURL.h"

//-----------------------------------------------------------------------------

NS_IMPL_THREADSAFE_ISUPPORTS2(nsGopherHandler,
                              nsIProxiedProtocolHandler,
                              nsIProtocolHandler)

//-----------------------------------------------------------------------------

NS_IMETHODIMP
nsGopherHandler::GetScheme(nsACString &result)
{
    result.AssignLiteral("gopher");
    return NS_OK;
}

NS_IMETHODIMP
nsGopherHandler::GetDefaultPort(PRInt32 *result)
{
    *result = GOPHER_PORT;
    return NS_OK;
}

NS_IMETHODIMP
nsGopherHandler::GetProtocolFlags(PRUint32 *result)
{
    *result = URI_NORELATIVE | ALLOWS_PROXY | ALLOWS_PROXY_HTTP |
      URI_LOADABLE_BY_ANYONE;
    return NS_OK;
}

NS_IMETHODIMP
nsGopherHandler::NewURI(const nsACString &spec, const char *originCharset,
                        nsIURI *baseURI, nsIURI **result)
{
    nsStandardURL *url = new nsStandardURL();
    if (!url)
        return NS_ERROR_OUT_OF_MEMORY;
    NS_ADDREF(url);

    nsresult rv = url->Init(nsIStandardURL::URLTYPE_STANDARD, GOPHER_PORT,
                            spec, originCharset, baseURI);
    if (NS_FAILED(rv)) {
        NS_RELEASE(url);
        return rv;
    }

    *result = url;  // no QI needed
    return NS_OK;
}

NS_IMETHODIMP
nsGopherHandler::NewProxiedChannel(nsIURI *uri, nsIProxyInfo *proxyInfo,
                                   nsIChannel **result)
{
    NS_ENSURE_ARG_POINTER(uri);
    nsGopherChannel *chan = new nsGopherChannel(uri, proxyInfo);
    if (!chan)
        return NS_ERROR_OUT_OF_MEMORY;
    NS_ADDREF(chan);

    nsresult rv = chan->Init();
    if (NS_FAILED(rv)) {
        NS_RELEASE(chan);
        return rv;
    }
    
    *result = chan;
    return NS_OK;
}

NS_IMETHODIMP
nsGopherHandler::NewChannel(nsIURI *uri, nsIChannel **result)
{
    return NewProxiedChannel(uri, nsnull, result);
}

NS_IMETHODIMP 
nsGopherHandler::AllowPort(PRInt32 port, const char *scheme, PRBool *result)
{
    *result = (port == GOPHER_PORT);
    return NS_OK;
}
