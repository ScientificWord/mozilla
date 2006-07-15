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
 * The Original Code is the mozilla.org LDAP XPCOM SDK.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2000
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Dan Mosedale <dmose@mozilla.org>
 *   Brian Ryner <bryner@brianryner.com>
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

#include "nsLDAPProtocolHandler.h"
#include "nsCRT.h"
#include "nsIComponentManager.h"
#include "nsLDAPURL.h"
#include "nsLDAPChannel.h"

static NS_DEFINE_CID(kLDAPURLCID, NS_LDAPURL_CID);

// QueryInterface, AddRef, and Release
//
NS_IMPL_ISUPPORTS1(nsLDAPProtocolHandler, nsIProtocolHandler)

nsLDAPProtocolHandler::nsLDAPProtocolHandler()
{
}

nsLDAPProtocolHandler::~nsLDAPProtocolHandler()
{
}

// nsIProtocolHandler methods

// getter method for scheme attr
//
NS_IMETHODIMP
nsLDAPProtocolHandler::GetScheme(nsACString &result)
{
  result = "ldap";
  return NS_OK;
}

// getter method for defaultPort attribute
//
NS_IMETHODIMP
nsLDAPProtocolHandler::GetDefaultPort(PRInt32 *result)
{
  *result = 389;
  return NS_OK;
}

// getter method for protocol flags attribute
//
NS_IMETHODIMP
nsLDAPProtocolHandler::GetProtocolFlags(PRUint32 *result)
{
  *result = URI_NORELATIVE;
  return NS_OK;
}

// construct an appropriate URI
//
NS_IMETHODIMP
nsLDAPProtocolHandler::NewURI(const nsACString &aSpec,
                              const char *aOriginCharset, // ignored
                              nsIURI *aBaseURI,
                              nsIURI **result) 
{
    nsCOMPtr<nsILDAPURL> url;
    nsresult rv;

    url = do_CreateInstance(kLDAPURLCID, &rv);
    NS_ENSURE_SUCCESS(rv, rv);

    // XXX - better error handling
    //
    rv = url->SetSpec(aSpec);
    NS_ENSURE_SUCCESS(rv, rv);

    // this is a getter, so we need to AddRef on the way out
    //
    *result = url;
    NS_ADDREF(*result);

    return NS_OK;
}

NS_IMETHODIMP
nsLDAPProtocolHandler::NewChannel(nsIURI* uri, 
                                  nsIChannel* *result)
{
    NS_ENSURE_ARG_POINTER(uri);
    nsresult rv;
    nsLDAPChannel *channel;

    rv = nsLDAPChannel::Create(0, NS_GET_IID(nsIChannel),
                               NS_REINTERPRET_CAST(void **, &channel));
    NS_ENSURE_SUCCESS(rv, rv);
  
    rv = channel->Init(uri);
    if (NS_FAILED(rv)) {
        NS_RELEASE(channel);
        return rv;
    }
    // the channel was already AddRefed for us, and since this function itself
    // is a getter, there's no need to release it here.
    //
    *result = channel;

    return NS_OK;
}

NS_IMETHODIMP 
nsLDAPProtocolHandler::AllowPort(PRInt32 port, const char *scheme, PRBool *_retval)
{
    if (port == 389 || port == 636)  // 636 is LDAP/SSL
        *_retval = PR_TRUE;
    else
        *_retval = PR_FALSE;
    return NS_OK;
}

