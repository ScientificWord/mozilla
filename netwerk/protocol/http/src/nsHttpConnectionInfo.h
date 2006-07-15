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
 * The Original Code is Mozilla.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications.
 * Portions created by the Initial Developer are Copyright (C) 2001
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Darin Fisher <darin@netscape.com> (original author)
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

#ifndef nsHttpConnectionInfo_h__
#define nsHttpConnectionInfo_h__

#include "nsHttp.h"
#include "nsProxyInfo.h"
#include "nsCOMPtr.h"
#include "nsDependentString.h"
#include "nsString.h"
#include "plstr.h"
#include "nsCRT.h"

//-----------------------------------------------------------------------------
// nsHttpConnectionInfo - holds the properties of a connection
//-----------------------------------------------------------------------------

class nsHttpConnectionInfo
{
public:
    nsHttpConnectionInfo(const nsACString &host, PRInt32 port,
                         nsProxyInfo* proxyInfo,
                         PRBool usingSSL=PR_FALSE)
        : mRef(0)
        , mProxyInfo(proxyInfo)
        , mUsingSSL(usingSSL) 
    {
        LOG(("Creating nsHttpConnectionInfo @%x\n", this));

        mUsingHttpProxy = (proxyInfo && !nsCRT::strcmp(proxyInfo->Type(), "http"));

        SetOriginServer(host, port);
    }
    
   ~nsHttpConnectionInfo()
    {
        LOG(("Destroying nsHttpConnectionInfo @%x\n", this));
    }

    nsrefcnt AddRef()
    {
        return PR_AtomicIncrement((PRInt32 *) &mRef);
    }

    nsrefcnt Release()
    {
        nsrefcnt n = PR_AtomicDecrement((PRInt32 *) &mRef);
        if (n == 0)
            delete this;
        return n;
    }

    const nsAFlatCString &HashKey() const { return mHashKey; }

    void SetOriginServer(const nsACString &host, PRInt32 port);

    void SetOriginServer(const char *host, PRInt32 port)
    {
        SetOriginServer(nsDependentCString(host), port);
    }

    const char *ProxyHost() const { return mProxyInfo ? mProxyInfo->Host().get() : nsnull; }
    PRInt32     ProxyPort() const { return mProxyInfo ? mProxyInfo->Port() : -1; }
    const char *ProxyType() const { return mProxyInfo ? mProxyInfo->Type() : nsnull; }

    // Compare this connection info to another...
    // Two connections are 'equal' if they end up talking the same
    // protocol to the same server. This is needed to properly manage
    // persistent connections to proxies
    // Note that we don't care about transparent proxies - 
    // it doesn't matter if we're talking via socks or not, since
    // a request will end up at the same host.
    PRBool Equals(const nsHttpConnectionInfo *info)
    {
        return mHashKey.Equals(info->HashKey());
    }

    const char   *Host() const           { return mHost.get(); }
    PRInt32       Port() const           { return mPort; }
    nsProxyInfo  *ProxyInfo()            { return mProxyInfo; }
    PRBool        UsingHttpProxy() const { return mUsingHttpProxy; }
    PRBool        UsingSSL() const       { return mUsingSSL; }
    PRInt32       DefaultPort() const    { return mUsingSSL ? NS_HTTPS_DEFAULT_PORT : NS_HTTP_DEFAULT_PORT; }
            
private:
    nsrefcnt               mRef;
    nsCString              mHashKey;
    nsCString              mHost;
    PRInt32                mPort;
    nsCOMPtr<nsProxyInfo>  mProxyInfo;
    PRPackedBool           mUsingHttpProxy;
    PRPackedBool           mUsingSSL;
};

#endif // nsHttpConnectionInfo_h__
