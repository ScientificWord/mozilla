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
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is
 * Brian Ryner.
 * Portions created by the Initial Developer are Copyright (C) 2000
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
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

// DateTime implementation

#include "nsDateTimeChannel.h"
#include "nsIServiceManager.h"
#include "nsILoadGroup.h"
#include "nsIInterfaceRequestor.h"
#include "nsIInterfaceRequestorUtils.h"
#include "nsXPIDLString.h"
#include "nsISocketTransportService.h"
#include "nsIStringStream.h"
#include "nsMimeTypes.h"
#include "nsIStreamConverterService.h"
#include "nsITXTToHTMLConv.h"
#include "nsIProgressEventSink.h"
#include "nsThreadUtils.h"
#include "nsNetUtil.h"
#include "nsCRT.h"

// nsDateTimeChannel methods
nsDateTimeChannel::nsDateTimeChannel()
    : mLoadFlags(LOAD_NORMAL)
    , mStatus(NS_OK)
    , mPort(-1)
{
}

nsDateTimeChannel::~nsDateTimeChannel()
{
}

NS_IMPL_ISUPPORTS5(nsDateTimeChannel, 
                   nsIChannel, 
                   nsIRequest,
                   nsIStreamListener, 
                   nsIRequestObserver,
                   nsIProxiedChannel)

nsresult
nsDateTimeChannel::Init(nsIURI *uri, nsIProxyInfo *proxyInfo)
{
    nsresult rv;

    NS_ASSERTION(uri, "no uri");

    mURI = uri;
    mProxyInfo = proxyInfo;

    rv = mURI->GetPort(&mPort);
    if (NS_FAILED(rv) || mPort < 1)
        mPort = DATETIME_PORT;

    rv = mURI->GetPath(mHost);
    if (NS_FAILED(rv)) return rv;

    if (mHost.IsEmpty())
        return NS_ERROR_MALFORMED_URI;

    mContentType.AssignLiteral(TEXT_HTML); // expected content-type
    return NS_OK;
}

////////////////////////////////////////////////////////////////////////////////
// nsIRequest methods:

NS_IMETHODIMP
nsDateTimeChannel::GetName(nsACString &result)
{
    return mURI->GetSpec(result);
}

NS_IMETHODIMP
nsDateTimeChannel::IsPending(PRBool *result)
{
    *result = (mPump != nsnull);
    return NS_OK;
}

NS_IMETHODIMP
nsDateTimeChannel::GetStatus(nsresult *status)
{
    if (NS_SUCCEEDED(mStatus) && mPump)
        mPump->GetStatus(status);
    else
        *status = mStatus;
    return NS_OK;
}

NS_IMETHODIMP
nsDateTimeChannel::Cancel(nsresult status)
{
    NS_ASSERTION(NS_FAILED(status), "shouldn't cancel with a success code");

    mStatus = status;
    if (mPump)
        mPump->Cancel(status);

    return NS_ERROR_UNEXPECTED;
}

NS_IMETHODIMP
nsDateTimeChannel::Suspend()
{
    if (mPump)
        return mPump->Suspend();
    return NS_ERROR_UNEXPECTED;
}

NS_IMETHODIMP
nsDateTimeChannel::Resume()
{
    if (mPump)
        return mPump->Resume();
    return NS_ERROR_UNEXPECTED;
}

////////////////////////////////////////////////////////////////////////////////
// nsIChannel methods:

NS_IMETHODIMP
nsDateTimeChannel::GetOriginalURI(nsIURI* *aURI)
{
    *aURI = mOriginalURI ? mOriginalURI : mURI;
    NS_ADDREF(*aURI);
    return NS_OK;
}

NS_IMETHODIMP
nsDateTimeChannel::SetOriginalURI(nsIURI* aURI)
{
    mOriginalURI = aURI;
    return NS_OK;
}

NS_IMETHODIMP
nsDateTimeChannel::GetURI(nsIURI* *aURI)
{
    *aURI = mURI;
    NS_IF_ADDREF(*aURI);
    return NS_OK;
}

NS_IMETHODIMP
nsDateTimeChannel::Open(nsIInputStream **_retval)
{
    return NS_ImplementChannelOpen(this, _retval);
}

NS_IMETHODIMP
nsDateTimeChannel::AsyncOpen(nsIStreamListener *aListener, nsISupports *ctxt)
{
    nsresult rv = NS_CheckPortSafety(mPort, "datetime");
    if (NS_FAILED(rv)) return rv;

    nsCOMPtr<nsIThread> thread = do_GetCurrentThread();
    NS_ENSURE_STATE(thread);

    //
    // create transport
    //
    nsCOMPtr<nsISocketTransportService> sts = 
             do_GetService(NS_SOCKETTRANSPORTSERVICE_CONTRACTID, &rv);
    if (NS_FAILED(rv)) return rv;

    rv = sts->CreateTransport(nsnull, 0, mHost, mPort, mProxyInfo,
                              getter_AddRefs(mTransport));
    if (NS_FAILED(rv)) return rv;

    // not fatal if this fails
    mTransport->SetEventSink(this, thread);

    //
    // create TXT to HTML stream converter
    //
    nsCOMPtr<nsIStreamConverterService> scs = 
             do_GetService(NS_STREAMCONVERTERSERVICE_CONTRACTID, &rv);
    if (NS_FAILED(rv)) return rv;

    nsCOMPtr<nsIStreamListener> convListener;
    rv = scs->AsyncConvertData("text/plain", "text/html", this, nsnull,
                               getter_AddRefs(convListener));
    if (NS_FAILED(rv)) return rv;

    nsCOMPtr<nsITXTToHTMLConv> conv = do_QueryInterface(convListener);
    if (conv) {
        nsCAutoString userHost;
        rv = mURI->GetPath(userHost);

        nsAutoString title;
        title.AppendLiteral("DateTime according to ");
        AppendUTF8toUTF16(mHost, title);

        conv->SetTitle(title.get());
        conv->PreFormatHTML(PR_TRUE);
    }

    //
    // open input stream, and create input stream pump...
    //
    nsCOMPtr<nsIInputStream> sockIn;
    rv = mTransport->OpenInputStream(0, 0, 0, getter_AddRefs(sockIn));
    if (NS_FAILED(rv)) return rv;

    rv = NS_NewInputStreamPump(getter_AddRefs(mPump), sockIn);
    if (NS_FAILED(rv)) return rv;

    rv = mPump->AsyncRead(convListener, nsnull);
    if (NS_FAILED(rv)) return rv;

    if (mLoadGroup)
        mLoadGroup->AddRequest(this, nsnull);

    mListener = aListener;
    mListenerContext = ctxt;
    return NS_OK;
}

NS_IMETHODIMP
nsDateTimeChannel::GetLoadFlags(PRUint32 *aLoadFlags)
{
    *aLoadFlags = mLoadFlags;
    return NS_OK;
}

NS_IMETHODIMP
nsDateTimeChannel::SetLoadFlags(PRUint32 aLoadFlags)
{
    mLoadFlags = aLoadFlags;
    return NS_OK;
}

NS_IMETHODIMP
nsDateTimeChannel::GetContentType(nsACString &aContentType)
{
    aContentType = mContentType;
    return NS_OK;
}

NS_IMETHODIMP
nsDateTimeChannel::SetContentType(const nsACString &aContentType)
{
    mContentType = aContentType;
    return NS_OK;
}

NS_IMETHODIMP
nsDateTimeChannel::GetContentCharset(nsACString &aContentCharset)
{
    aContentCharset = mContentCharset;
    return NS_OK;
}

NS_IMETHODIMP
nsDateTimeChannel::SetContentCharset(const nsACString &aContentCharset)
{
    mContentCharset = aContentCharset;
    return NS_OK;
}

NS_IMETHODIMP
nsDateTimeChannel::GetContentLength(PRInt32 *aContentLength)
{
    *aContentLength = -1;
    return NS_OK;
}

NS_IMETHODIMP
nsDateTimeChannel::SetContentLength(PRInt32 aContentLength)
{
    // silently ignore this...
    return NS_OK;
}

NS_IMETHODIMP
nsDateTimeChannel::GetLoadGroup(nsILoadGroup* *aLoadGroup)
{
    *aLoadGroup = mLoadGroup;
    NS_IF_ADDREF(*aLoadGroup);
    return NS_OK;
}

NS_IMETHODIMP
nsDateTimeChannel::SetLoadGroup(nsILoadGroup* aLoadGroup)
{
    mLoadGroup = aLoadGroup;
    mProgressSink = nsnull;
    return NS_OK;
}

NS_IMETHODIMP
nsDateTimeChannel::GetOwner(nsISupports* *aOwner)
{
    *aOwner = mOwner.get();
    NS_IF_ADDREF(*aOwner);
    return NS_OK;
}

NS_IMETHODIMP
nsDateTimeChannel::SetOwner(nsISupports* aOwner)
{
    mOwner = aOwner;
    return NS_OK;
}

NS_IMETHODIMP
nsDateTimeChannel::GetNotificationCallbacks(nsIInterfaceRequestor* *aNotificationCallbacks)
{
    *aNotificationCallbacks = mCallbacks.get();
    NS_IF_ADDREF(*aNotificationCallbacks);
    return NS_OK;
}

NS_IMETHODIMP
nsDateTimeChannel::SetNotificationCallbacks(nsIInterfaceRequestor* aNotificationCallbacks)
{
    mCallbacks = aNotificationCallbacks;
    mProgressSink = nsnull;
    return NS_OK;
}

NS_IMETHODIMP 
nsDateTimeChannel::GetSecurityInfo(nsISupports **aSecurityInfo)
{
    if (mTransport)
        return mTransport->GetSecurityInfo(aSecurityInfo);

    *aSecurityInfo = nsnull;
    return NS_OK;
}

//-----------------------------------------------------------------------------
// nsIRequestObserver methods
//-----------------------------------------------------------------------------

NS_IMETHODIMP
nsDateTimeChannel::OnStartRequest(nsIRequest *req, nsISupports *ctx)
{
    return mListener->OnStartRequest(this, mListenerContext);
}

NS_IMETHODIMP
nsDateTimeChannel::OnStopRequest(nsIRequest *req, nsISupports *ctx, nsresult status)
{
    if (NS_SUCCEEDED(mStatus))
        mStatus = status;

    mListener->OnStopRequest(this, mListenerContext, mStatus);
    mListener = 0;
    mListenerContext = 0;

    if (mLoadGroup)
        mLoadGroup->RemoveRequest(this, nsnull, mStatus);

    mPump = 0;
    mTransport = 0;

    // Drop notification callbacks to prevent cycles.
    mCallbacks = 0;
    mProgressSink = 0;

    return NS_OK;
}

NS_IMETHODIMP
nsDateTimeChannel::OnDataAvailable(nsIRequest *req, nsISupports *ctx,
                                 nsIInputStream *stream, PRUint32 offset,
                                 PRUint32 count)
{
    return mListener->OnDataAvailable(this, mListenerContext, stream, offset, count);
}

//-----------------------------------------------------------------------------
// nsITransportEventSink methods
//-----------------------------------------------------------------------------

NS_IMETHODIMP
nsDateTimeChannel::OnTransportStatus(nsITransport *trans, nsresult status,
                                     PRUint64 progress, PRUint64 progressMax)
{
    if (!mProgressSink)
        NS_QueryNotificationCallbacks(mCallbacks, mLoadGroup, mProgressSink);

    // suppress status notification if channel is no longer pending!
    if (mProgressSink && NS_SUCCEEDED(mStatus) && mPump && !(mLoadFlags & LOAD_BACKGROUND)) {
        NS_ConvertUTF8toUTF16 host(mHost);
        mProgressSink->OnStatus(this, nsnull, status, host.get());

        if (status == nsISocketTransport::STATUS_RECEIVING_FROM ||
            status == nsISocketTransport::STATUS_SENDING_TO) {
            mProgressSink->OnProgress(this, nsnull, progress, progressMax);
        }
    }
    return NS_OK;
}

//-----------------------------------------------------------------------------
// nsIProxiedChannel methods
//-----------------------------------------------------------------------------

NS_IMETHODIMP
nsDateTimeChannel::GetProxyInfo(nsIProxyInfo** aProxyInfo)
{
    *aProxyInfo = mProxyInfo;
    NS_IF_ADDREF(*aProxyInfo);
    return NS_OK;
}

