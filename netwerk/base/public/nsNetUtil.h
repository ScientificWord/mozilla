/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* vim:set ts=4 sw=4 sts=4 et cin: */
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
 *   Bradley Baetz <bbaetz@student.usyd.edu.au>
 *   Malcolm Smith <malsmith@cs.rmit.edu.au>
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

#ifndef nsNetUtil_h__
#define nsNetUtil_h__

#include "nsNetError.h"
#include "nsNetCID.h"
#include "nsStringGlue.h"
#include "nsMemory.h"
#include "nsCOMPtr.h"
#include "prio.h" // for read/write flags, permissions, etc.

#include "nsIURI.h"
#include "nsIInputStream.h"
#include "nsIOutputStream.h"
#include "nsISafeOutputStream.h"
#include "nsIStreamListener.h"
#include "nsIRequestObserverProxy.h"
#include "nsISimpleStreamListener.h"
#include "nsILoadGroup.h"
#include "nsIInterfaceRequestor.h"
#include "nsIInterfaceRequestorUtils.h"
#include "nsIIOService.h"
#include "nsIServiceManager.h"
#include "nsIChannel.h"
#include "nsIInputStreamChannel.h"
#include "nsITransport.h"
#include "nsIStreamTransportService.h"
#include "nsIHttpChannel.h"
#include "nsIDownloader.h"
#include "nsIStreamLoader.h"
#include "nsIUnicharStreamLoader.h"
#include "nsIPipe.h"
#include "nsIProtocolHandler.h"
#include "nsIFileProtocolHandler.h"
#include "nsIStringStream.h"
#include "nsILocalFile.h"
#include "nsIFileStreams.h"
#include "nsIProtocolProxyService.h"
#include "nsIProxyInfo.h"
#include "nsIFileStreams.h"
#include "nsIBufferedStreams.h"
#include "nsIInputStreamPump.h"
#include "nsIAsyncStreamCopier.h"
#include "nsIPersistentProperties2.h"
#include "nsISyncStreamListener.h"
#include "nsInterfaceRequestorAgg.h"
#include "nsInt64.h"
#include "nsINetUtil.h"
#include "nsComponentManagerUtils.h"
#include "nsServiceManagerUtils.h"
#include "nsINestedURI.h"
#include "nsIMutable.h"

// Helper, to simplify getting the I/O service.
inline const nsGetServiceByContractIDWithError
do_GetIOService(nsresult* error = 0)
{
    return nsGetServiceByContractIDWithError(NS_IOSERVICE_CONTRACTID, error);
}

// private little helper function... don't call this directly!
inline nsresult
net_EnsureIOService(nsIIOService **ios, nsCOMPtr<nsIIOService> &grip)
{
    nsresult rv = NS_OK;
    if (!*ios) {
        grip = do_GetIOService(&rv);
        *ios = grip;
    }
    return rv;
}

inline nsresult
NS_NewURI(nsIURI **result, 
          const nsACString &spec, 
          const char *charset = nsnull,
          nsIURI *baseURI = nsnull,
          nsIIOService *ioService = nsnull)     // pass in nsIIOService to optimize callers
{
    nsresult rv;
    nsCOMPtr<nsIIOService> grip;
    rv = net_EnsureIOService(&ioService, grip);
    if (ioService)
        rv = ioService->NewURI(spec, charset, baseURI, result);
    return rv; 
}

inline nsresult
NS_NewURI(nsIURI* *result, 
          const nsAString& spec, 
          const char *charset = nsnull,
          nsIURI* baseURI = nsnull,
          nsIIOService* ioService = nsnull)     // pass in nsIIOService to optimize callers
{
    return NS_NewURI(result, NS_ConvertUTF16toUTF8(spec), charset, baseURI, ioService);
}

inline nsresult
NS_NewURI(nsIURI* *result, 
          const char *spec,
          nsIURI* baseURI = nsnull,
          nsIIOService* ioService = nsnull)     // pass in nsIIOService to optimize callers
{
    return NS_NewURI(result, nsDependentCString(spec), nsnull, baseURI, ioService);
}

inline nsresult
NS_NewFileURI(nsIURI* *result, 
              nsIFile* spec, 
              nsIIOService* ioService = nsnull)     // pass in nsIIOService to optimize callers
{
    nsresult rv;
    nsCOMPtr<nsIIOService> grip;
    rv = net_EnsureIOService(&ioService, grip);
    if (ioService)
        rv = ioService->NewFileURI(spec, result);
    return rv;
}

inline nsresult
NS_NewChannel(nsIChannel           **result, 
              nsIURI                *uri,
              nsIIOService          *ioService = nsnull,    // pass in nsIIOService to optimize callers
              nsILoadGroup          *loadGroup = nsnull,
              nsIInterfaceRequestor *callbacks = nsnull,
              PRUint32               loadFlags = nsIRequest::LOAD_NORMAL)
{
    nsresult rv;
    nsCOMPtr<nsIIOService> grip;
    rv = net_EnsureIOService(&ioService, grip);
    if (ioService) {
        nsIChannel *chan;
        rv = ioService->NewChannelFromURI(uri, &chan);
        if (NS_SUCCEEDED(rv)) {
            if (loadGroup)
                rv |= chan->SetLoadGroup(loadGroup);
            if (callbacks)
                rv |= chan->SetNotificationCallbacks(callbacks);
            if (loadFlags != nsIRequest::LOAD_NORMAL)
                rv |= chan->SetLoadFlags(loadFlags);
            if (NS_SUCCEEDED(rv))
                *result = chan;
            else
                NS_RELEASE(chan);
        }
    }
    return rv;
}

// Use this function with CAUTION. It creates a stream that blocks when you
// Read() from it and blocking the UI thread is a bad idea. If you don't want
// to implement a full blown asynchronous consumer (via nsIStreamListener) look
// at nsIStreamLoader instead.
inline nsresult
NS_OpenURI(nsIInputStream       **result,
           nsIURI                *uri,
           nsIIOService          *ioService = nsnull,     // pass in nsIIOService to optimize callers
           nsILoadGroup          *loadGroup = nsnull,
           nsIInterfaceRequestor *callbacks = nsnull,
           PRUint32               loadFlags = nsIRequest::LOAD_NORMAL)
{
    nsresult rv;
    nsCOMPtr<nsIChannel> channel;
    rv = NS_NewChannel(getter_AddRefs(channel), uri, ioService,
                       loadGroup, callbacks, loadFlags);
    if (NS_SUCCEEDED(rv)) {
        nsIInputStream *stream;
        rv = channel->Open(&stream);
        if (NS_SUCCEEDED(rv))
            *result = stream;
    }
    return rv;
}

inline nsresult
NS_OpenURI(nsIStreamListener     *listener, 
           nsISupports           *context, 
           nsIURI                *uri,
           nsIIOService          *ioService = nsnull,     // pass in nsIIOService to optimize callers
           nsILoadGroup          *loadGroup = nsnull,
           nsIInterfaceRequestor *callbacks = nsnull,
           PRUint32               loadFlags = nsIRequest::LOAD_NORMAL)
{
    nsresult rv;
    nsCOMPtr<nsIChannel> channel;
    rv = NS_NewChannel(getter_AddRefs(channel), uri, ioService,
                       loadGroup, callbacks, loadFlags);
    if (NS_SUCCEEDED(rv))
        rv = channel->AsyncOpen(listener, context);
    return rv;
}

inline nsresult
NS_MakeAbsoluteURI(nsACString       &result,
                   const nsACString &spec, 
                   nsIURI           *baseURI, 
                   nsIIOService     *unused = nsnull)
{
    nsresult rv;
    if (!baseURI) {
        NS_WARNING("It doesn't make sense to not supply a base URI");
        result = spec;
        rv = NS_OK;
    }
    else if (spec.IsEmpty())
        rv = baseURI->GetSpec(result);
    else
        rv = baseURI->Resolve(spec, result);
    return rv;
}

inline nsresult
NS_MakeAbsoluteURI(char        **result,
                   const char   *spec, 
                   nsIURI       *baseURI, 
                   nsIIOService *unused = nsnull)
{
    nsresult rv;
    nsCAutoString resultBuf;
    rv = NS_MakeAbsoluteURI(resultBuf, nsDependentCString(spec), baseURI);
    if (NS_SUCCEEDED(rv)) {
        *result = ToNewCString(resultBuf);
        if (!*result)
            rv = NS_ERROR_OUT_OF_MEMORY;
    }
    return rv;
}

inline nsresult
NS_MakeAbsoluteURI(nsAString       &result,
                   const nsAString &spec, 
                   nsIURI          *baseURI,
                   nsIIOService    *unused = nsnull)
{
    nsresult rv;
    if (!baseURI) {
        NS_WARNING("It doesn't make sense to not supply a base URI");
        result = spec;
        rv = NS_OK;
    }
    else {
        nsCAutoString resultBuf;
        if (spec.IsEmpty())
            rv = baseURI->GetSpec(resultBuf);
        else
            rv = baseURI->Resolve(NS_ConvertUTF16toUTF8(spec), resultBuf);
        if (NS_SUCCEEDED(rv))
            CopyUTF8toUTF16(resultBuf, result);
    }
    return rv;
}

inline nsresult
NS_NewInputStreamChannel(nsIChannel      **result,
                         nsIURI           *uri,
                         nsIInputStream   *stream,
                         const nsACString &contentType,
                         const nsACString *contentCharset)
{
    nsresult rv;
    nsCOMPtr<nsIInputStreamChannel> isc =
        do_CreateInstance(NS_INPUTSTREAMCHANNEL_CONTRACTID, &rv);
    if (NS_FAILED(rv))
        return rv;
    rv |= isc->SetURI(uri);
    rv |= isc->SetContentStream(stream);
    if (NS_FAILED(rv))
        return rv;
    nsCOMPtr<nsIChannel> chan = do_QueryInterface(isc, &rv);
    if (NS_FAILED(rv))
        return rv;
    if (!contentType.IsEmpty())
        rv |= chan->SetContentType(contentType);
    if (contentCharset && !contentCharset->IsEmpty())
        rv |= chan->SetContentCharset(*contentCharset);
    if (NS_SUCCEEDED(rv)) {
        *result = nsnull;
        chan.swap(*result);
    }
    return rv;
}

inline nsresult
NS_NewInputStreamChannel(nsIChannel      **result,
                         nsIURI           *uri,
                         nsIInputStream   *stream,
                         const nsACString &contentType    = EmptyCString())
{
    return NS_NewInputStreamChannel(result, uri, stream, contentType, nsnull);
}

inline nsresult
NS_NewInputStreamChannel(nsIChannel      **result,
                         nsIURI           *uri,
                         nsIInputStream   *stream,
                         const nsACString &contentType,
                         const nsACString &contentCharset)
{
    return NS_NewInputStreamChannel(result, uri, stream, contentType,
                                    &contentCharset);
}

inline nsresult
NS_NewInputStreamPump(nsIInputStreamPump **result,
                      nsIInputStream      *stream,
                      PRInt64              streamPos = nsInt64(-1),
                      PRInt64              streamLen = nsInt64(-1),
                      PRUint32             segsize = 0,
                      PRUint32             segcount = 0,
                      PRBool               closeWhenDone = PR_FALSE)
{
    nsresult rv;
    nsCOMPtr<nsIInputStreamPump> pump =
        do_CreateInstance(NS_INPUTSTREAMPUMP_CONTRACTID, &rv);
    if (NS_SUCCEEDED(rv)) {
        rv = pump->Init(stream, streamPos, streamLen,
                        segsize, segcount, closeWhenDone);
        if (NS_SUCCEEDED(rv)) {
            *result = nsnull;
            pump.swap(*result);
        }
    }
    return rv;
}

// NOTE: you will need to specify whether or not your streams are buffered
// (i.e., do they implement ReadSegments/WriteSegments).  the default
// assumption of TRUE for both streams might not be right for you!
inline nsresult
NS_NewAsyncStreamCopier(nsIAsyncStreamCopier **result,
                        nsIInputStream        *source,
                        nsIOutputStream       *sink,
                        nsIEventTarget        *target,
                        PRBool                 sourceBuffered = PR_TRUE,
                        PRBool                 sinkBuffered = PR_TRUE,
                        PRUint32               chunkSize = 0)
{
    nsresult rv;
    nsCOMPtr<nsIAsyncStreamCopier> copier =
        do_CreateInstance(NS_ASYNCSTREAMCOPIER_CONTRACTID, &rv);
    if (NS_SUCCEEDED(rv)) {
        rv = copier->Init(source, sink, target, sourceBuffered, sinkBuffered, chunkSize);
        if (NS_SUCCEEDED(rv)) {
            *result = nsnull;
            copier.swap(*result);
        }
    }
    return rv;
}

inline nsresult
NS_NewLoadGroup(nsILoadGroup      **result,
                nsIRequestObserver *obs)
{
    nsresult rv;
    nsCOMPtr<nsILoadGroup> group =
        do_CreateInstance(NS_LOADGROUP_CONTRACTID, &rv);
    if (NS_SUCCEEDED(rv)) {
        rv = group->SetGroupObserver(obs);
        if (NS_SUCCEEDED(rv)) {
            *result = nsnull;
            group.swap(*result);
        }
    }
    return rv;
}

inline nsresult
NS_NewDownloader(nsIStreamListener   **result,
                 nsIDownloadObserver  *observer,
                 nsIFile              *downloadLocation = nsnull)
{
    nsresult rv;
    nsCOMPtr<nsIDownloader> downloader =
        do_CreateInstance(NS_DOWNLOADER_CONTRACTID, &rv);
    if (NS_SUCCEEDED(rv)) {
        rv = downloader->Init(observer, downloadLocation);
        if (NS_SUCCEEDED(rv))
            NS_ADDREF(*result = downloader);
    }
    return rv;
}

inline nsresult
NS_NewStreamLoader(nsIStreamLoader        **result,
                   nsIChannel              *channel,
                   nsIStreamLoaderObserver *observer,
                   nsISupports             *context)
{
    nsresult rv;
    nsCOMPtr<nsIStreamLoader> loader =
        do_CreateInstance(NS_STREAMLOADER_CONTRACTID, &rv);
    if (NS_SUCCEEDED(rv)) {
        rv = loader->Init(channel, observer, context);
        if (NS_SUCCEEDED(rv)) {
            *result = nsnull;
            loader.swap(*result);
        }
    }
    return rv;
}

inline nsresult
NS_NewStreamLoader(nsIStreamLoader        **result,
                   nsIURI                  *uri,
                   nsIStreamLoaderObserver *observer,
                   nsISupports             *context   = nsnull,
                   nsILoadGroup            *loadGroup = nsnull,
                   nsIInterfaceRequestor   *callbacks = nsnull,
                   PRUint32                 loadFlags = nsIRequest::LOAD_NORMAL,
                   nsIURI                  *referrer  = nsnull)
{
    nsresult rv;
    nsCOMPtr<nsIChannel> channel;
    rv = NS_NewChannel(getter_AddRefs(channel),
                       uri,
                       nsnull,
                       loadGroup,
                       callbacks,
                       loadFlags);
    if (NS_SUCCEEDED(rv)) {
        nsCOMPtr<nsIHttpChannel> httpChannel(do_QueryInterface(channel));
        if (httpChannel)
            httpChannel->SetReferrer(referrer);
        rv = NS_NewStreamLoader(result, channel, observer, context);
    }
    return rv;
}

inline nsresult
NS_NewUnicharStreamLoader(nsIUnicharStreamLoader        **result,
                          nsIChannel                     *channel,
                          nsIUnicharStreamLoaderObserver *observer,
                          nsISupports                    *context     = nsnull,
                          PRUint32                        segmentSize = nsIUnicharStreamLoader::DEFAULT_SEGMENT_SIZE)
{
    nsresult rv;
    nsCOMPtr<nsIUnicharStreamLoader> loader =
        do_CreateInstance(NS_UNICHARSTREAMLOADER_CONTRACTID, &rv);
    if (NS_SUCCEEDED(rv)) {
        rv = loader->Init(channel, observer, context, segmentSize);
        if (NS_SUCCEEDED(rv)) {
            *result = nsnull;
            loader.swap(*result);
        }
    }
    return rv;
}

inline nsresult
NS_NewSyncStreamListener(nsIStreamListener **result,
                         nsIInputStream    **stream)
{
    nsresult rv;
    nsCOMPtr<nsISyncStreamListener> listener =
        do_CreateInstance(NS_SYNCSTREAMLISTENER_CONTRACTID, &rv);
    if (NS_SUCCEEDED(rv)) {
        rv = listener->GetInputStream(stream);
        if (NS_SUCCEEDED(rv))
            NS_ADDREF(*result = listener);  // cannot use nsCOMPtr::swap
    }
    return rv;
}

/**
 * Implement the nsIChannel::Open(nsIInputStream**) method using the channel's
 * AsyncOpen method.
 *
 * NOTE: Reading from the returned nsIInputStream may spin the current
 * thread's event queue, which could result in any event being processed.
 */
inline nsresult
NS_ImplementChannelOpen(nsIChannel      *channel,
                        nsIInputStream **result)
{
    nsCOMPtr<nsIStreamListener> listener;
    nsCOMPtr<nsIInputStream> stream;
    nsresult rv = NS_NewSyncStreamListener(getter_AddRefs(listener),
                                           getter_AddRefs(stream));
    if (NS_SUCCEEDED(rv)) {
        rv = channel->AsyncOpen(listener, nsnull);
        if (NS_SUCCEEDED(rv)) {
            PRUint32 n;
            // block until the initial response is received or an error occurs.
            rv = stream->Available(&n);
            if (NS_SUCCEEDED(rv)) {
                *result = nsnull;
                stream.swap(*result);
            }
        }
    }
    return rv;
}

inline nsresult
NS_NewRequestObserverProxy(nsIRequestObserver **result,
                           nsIRequestObserver  *observer,
                           nsIEventTarget      *target = nsnull)
{
    nsresult rv;
    nsCOMPtr<nsIRequestObserverProxy> proxy =
        do_CreateInstance(NS_REQUESTOBSERVERPROXY_CONTRACTID, &rv);
    if (NS_SUCCEEDED(rv)) {
        rv = proxy->Init(observer, target);
        if (NS_SUCCEEDED(rv))
            NS_ADDREF(*result = proxy);  // cannot use nsCOMPtr::swap
    }
    return rv;
}

inline nsresult
NS_NewSimpleStreamListener(nsIStreamListener **result,
                           nsIOutputStream    *sink,
                           nsIRequestObserver *observer = nsnull)
{
    nsresult rv;
    nsCOMPtr<nsISimpleStreamListener> listener = 
        do_CreateInstance(NS_SIMPLESTREAMLISTENER_CONTRACTID, &rv);
    if (NS_SUCCEEDED(rv)) {
        rv = listener->Init(sink, observer);
        if (NS_SUCCEEDED(rv))
            NS_ADDREF(*result = listener);  // cannot use nsCOMPtr::swap
    }
    return rv;
}

inline nsresult
NS_CheckPortSafety(PRInt32       port,
                   const char   *scheme,
                   nsIIOService *ioService = nsnull)
{
    nsresult rv;
    nsCOMPtr<nsIIOService> grip;
    rv = net_EnsureIOService(&ioService, grip);
    if (ioService) {
        PRBool allow;
        rv = ioService->AllowPort(port, scheme, &allow);
        if (NS_SUCCEEDED(rv) && !allow) {
            NS_WARNING("port blocked");
            rv = NS_ERROR_PORT_ACCESS_NOT_ALLOWED;
        }
    }
    return rv;
}

// Determine if this URI is using a safe port.
inline nsresult
NS_CheckPortSafety(nsIURI *uri) {
    PRInt32 port;
    nsresult rv = uri->GetPort(&port);
    if (NS_FAILED(rv) || port == -1)  // port undefined or default-valued
        return NS_OK;
    nsCAutoString scheme;
    uri->GetScheme(scheme);
    return NS_CheckPortSafety(port, scheme.get());
}

inline nsresult
NS_NewProxyInfo(const nsACString &type,
                const nsACString &host,
                PRInt32           port,
                PRUint32          flags,
                nsIProxyInfo    **result)
{
    nsresult rv;
    nsCOMPtr<nsIProtocolProxyService> pps =
            do_GetService(NS_PROTOCOLPROXYSERVICE_CONTRACTID, &rv);
    if (NS_SUCCEEDED(rv))
        rv = pps->NewProxyInfo(type, host, port, flags, PR_UINT32_MAX, nsnull,
                               result);
    return rv; 
}

inline nsresult
NS_GetFileProtocolHandler(nsIFileProtocolHandler **result,
                          nsIIOService            *ioService = nsnull)
{
    nsresult rv;
    nsCOMPtr<nsIIOService> grip;
    rv = net_EnsureIOService(&ioService, grip);
    if (ioService) {
        nsCOMPtr<nsIProtocolHandler> handler;
        rv = ioService->GetProtocolHandler("file", getter_AddRefs(handler));
        if (NS_SUCCEEDED(rv))
            rv = CallQueryInterface(handler, result);
    }
    return rv;
}

inline nsresult
NS_GetFileFromURLSpec(const nsACString  &inURL,
                      nsIFile          **result,
                      nsIIOService      *ioService = nsnull)
{
    nsresult rv;
    nsCOMPtr<nsIFileProtocolHandler> fileHandler;
    rv = NS_GetFileProtocolHandler(getter_AddRefs(fileHandler), ioService);
    if (NS_SUCCEEDED(rv))
        rv = fileHandler->GetFileFromURLSpec(inURL, result);
    return rv;
}

inline nsresult
NS_GetURLSpecFromFile(nsIFile      *file,
                      nsACString   &url,
                      nsIIOService *ioService = nsnull)
{
    nsresult rv;
    nsCOMPtr<nsIFileProtocolHandler> fileHandler;
    rv = NS_GetFileProtocolHandler(getter_AddRefs(fileHandler), ioService);
    if (NS_SUCCEEDED(rv))
        rv = fileHandler->GetURLSpecFromFile(file, url);
    return rv;
}

#ifdef MOZILLA_INTERNAL_API
inline nsresult
NS_ExamineForProxy(const char    *scheme,
                   const char    *host,
                   PRInt32        port, 
                   nsIProxyInfo **proxyInfo)
{
    nsresult rv;
    nsCOMPtr<nsIProtocolProxyService> pps =
            do_GetService(NS_PROTOCOLPROXYSERVICE_CONTRACTID, &rv);
    if (NS_SUCCEEDED(rv)) {
        nsCAutoString spec(scheme);
        spec.Append("://");
        spec.Append(host);
        spec.Append(':');
        spec.AppendInt(port);
        // XXXXX - Under no circumstances whatsoever should any code which
        // wants a uri do this. I do this here because I do not, in fact,
        // actually want a uri (the dummy uris created here may not be 
        // syntactically valid for the specific protocol), and all we need
        // is something which has a valid scheme, hostname, and a string
        // to pass to PAC if needed - bbaetz
        nsCOMPtr<nsIURI> uri =
                do_CreateInstance(NS_STANDARDURL_CONTRACTID, &rv);
        if (NS_SUCCEEDED(rv)) {
            rv = uri->SetSpec(spec);
            if (NS_SUCCEEDED(rv))
                rv = pps->Resolve(uri, 0, proxyInfo);
        }
    }
    return rv;
}
#endif

inline nsresult
NS_ParseContentType(const nsACString &rawContentType,
                    nsCString        &contentType,
                    nsCString        &contentCharset)
{
    // contentCharset is left untouched if not present in rawContentType
    nsresult rv;
    nsCOMPtr<nsINetUtil> util = do_GetIOService(&rv);
    NS_ENSURE_SUCCESS(rv, rv);
    nsCString charset;
    PRBool hadCharset;
    rv = util->ParseContentType(rawContentType, charset, &hadCharset,
                                contentType);
    if (NS_SUCCEEDED(rv) && hadCharset)
        contentCharset = charset;
    return rv;
}

inline nsresult
NS_NewLocalFileInputStream(nsIInputStream **result,
                           nsIFile         *file,
                           PRInt32          ioFlags       = -1,
                           PRInt32          perm          = -1,
                           PRInt32          behaviorFlags = 0)
{
    nsresult rv;
    nsCOMPtr<nsIFileInputStream> in =
        do_CreateInstance(NS_LOCALFILEINPUTSTREAM_CONTRACTID, &rv);
    if (NS_SUCCEEDED(rv)) {
        rv = in->Init(file, ioFlags, perm, behaviorFlags);
        if (NS_SUCCEEDED(rv))
            NS_ADDREF(*result = in);  // cannot use nsCOMPtr::swap
    }
    return rv;
}

inline nsresult
NS_NewLocalFileOutputStream(nsIOutputStream **result,
                            nsIFile          *file,
                            PRInt32           ioFlags       = -1,
                            PRInt32           perm          = -1,
                            PRInt32           behaviorFlags = 0)
{
    nsresult rv;
    nsCOMPtr<nsIFileOutputStream> out =
        do_CreateInstance(NS_LOCALFILEOUTPUTSTREAM_CONTRACTID, &rv);
    if (NS_SUCCEEDED(rv)) {
        rv = out->Init(file, ioFlags, perm, behaviorFlags);
        if (NS_SUCCEEDED(rv))
            NS_ADDREF(*result = out);  // cannot use nsCOMPtr::swap
    }
    return rv;
}

// returns a file output stream which can be QI'ed to nsISafeOutputStream.
inline nsresult
NS_NewSafeLocalFileOutputStream(nsIOutputStream **result,
                                nsIFile          *file,
                                PRInt32           ioFlags       = -1,
                                PRInt32           perm          = -1,
                                PRInt32           behaviorFlags = 0)
{
    nsresult rv;
    nsCOMPtr<nsIFileOutputStream> out =
        do_CreateInstance(NS_SAFELOCALFILEOUTPUTSTREAM_CONTRACTID, &rv);
    if (NS_SUCCEEDED(rv)) {
        rv = out->Init(file, ioFlags, perm, behaviorFlags);
        if (NS_SUCCEEDED(rv))
            NS_ADDREF(*result = out);  // cannot use nsCOMPtr::swap
    }
    return rv;
}

// returns the input end of a pipe.  the output end of the pipe
// is attached to the original stream.  data from the original
// stream is read into the pipe on a background thread.
inline nsresult
NS_BackgroundInputStream(nsIInputStream **result,
                         nsIInputStream  *stream,
                         PRUint32         segmentSize  = 0,
                         PRUint32         segmentCount = 0)
{
    nsresult rv;
    nsCOMPtr<nsIStreamTransportService> sts =
        do_GetService(NS_STREAMTRANSPORTSERVICE_CONTRACTID, &rv);
    if (NS_SUCCEEDED(rv)) {
        nsCOMPtr<nsITransport> inTransport;
        rv = sts->CreateInputTransport(stream, nsInt64(-1), nsInt64(-1),
                                       PR_TRUE, getter_AddRefs(inTransport));
        if (NS_SUCCEEDED(rv))
            rv = inTransport->OpenInputStream(nsITransport::OPEN_BLOCKING,
                                              segmentSize, segmentCount,
                                              result);
    }
    return rv;
}

// returns the output end of a pipe.  the input end of the pipe
// is attached to the original stream.  data written to the pipe
// is copied to the original stream on a background thread.
inline nsresult
NS_BackgroundOutputStream(nsIOutputStream **result,
                          nsIOutputStream  *stream,
                          PRUint32          segmentSize  = 0,
                          PRUint32          segmentCount = 0)
{
    nsresult rv;
    nsCOMPtr<nsIStreamTransportService> sts =
        do_GetService(NS_STREAMTRANSPORTSERVICE_CONTRACTID, &rv);
    if (NS_SUCCEEDED(rv)) {
        nsCOMPtr<nsITransport> inTransport;
        rv = sts->CreateOutputTransport(stream, nsInt64(-1), nsInt64(-1),
                                        PR_TRUE, getter_AddRefs(inTransport));
        if (NS_SUCCEEDED(rv))
            rv = inTransport->OpenOutputStream(nsITransport::OPEN_BLOCKING,
                                               segmentSize, segmentCount,
                                               result);
    }
    return rv;
}

inline nsresult
NS_NewBufferedInputStream(nsIInputStream **result,
                          nsIInputStream  *str,
                          PRUint32         bufferSize)
{
    nsresult rv;
    nsCOMPtr<nsIBufferedInputStream> in =
        do_CreateInstance(NS_BUFFEREDINPUTSTREAM_CONTRACTID, &rv);
    if (NS_SUCCEEDED(rv)) {
        rv = in->Init(str, bufferSize);
        if (NS_SUCCEEDED(rv))
            NS_ADDREF(*result = in);  // cannot use nsCOMPtr::swap
    }
    return rv;
}

// note: the resulting stream can be QI'ed to nsISafeOutputStream iff the
// provided stream supports it.
inline nsresult
NS_NewBufferedOutputStream(nsIOutputStream **result,
                           nsIOutputStream  *str,
                           PRUint32          bufferSize)
{
    nsresult rv;
    nsCOMPtr<nsIBufferedOutputStream> out =
        do_CreateInstance(NS_BUFFEREDOUTPUTSTREAM_CONTRACTID, &rv);
    if (NS_SUCCEEDED(rv)) {
        rv = out->Init(str, bufferSize);
        if (NS_SUCCEEDED(rv))
            NS_ADDREF(*result = out);  // cannot use nsCOMPtr::swap
    }
    return rv;
}

// returns an input stream compatible with nsIUploadChannel::SetUploadStream()
inline nsresult
NS_NewPostDataStream(nsIInputStream  **result,
                     PRBool            isFile,
                     const nsACString &data,
                     PRUint32          encodeFlags,
                     nsIIOService     *unused = nsnull)
{
    nsresult rv;

    if (isFile) {
        nsCOMPtr<nsILocalFile> file;
        nsCOMPtr<nsIInputStream> fileStream;

        rv = NS_NewNativeLocalFile(data, PR_FALSE, getter_AddRefs(file));
        if (NS_SUCCEEDED(rv)) {
            rv = NS_NewLocalFileInputStream(getter_AddRefs(fileStream), file);
            if (NS_SUCCEEDED(rv)) {
                // wrap the file stream with a buffered input stream
                rv = NS_NewBufferedInputStream(result, fileStream, 8192);
            }
        }
        return rv;
    }

    // otherwise, create a string stream for the data (copies)
    nsCOMPtr<nsIStringInputStream> stream
        (do_CreateInstance("@mozilla.org/io/string-input-stream;1", &rv));
    if (NS_FAILED(rv))
        return rv;

    rv = stream->SetData(data.BeginReading(), data.Length());
    if (NS_FAILED(rv))
        return rv;

    NS_ADDREF(*result = stream);
    return NS_OK;
}

inline nsresult
NS_LoadPersistentPropertiesFromURI(nsIPersistentProperties **result,
                                   nsIURI                   *uri,
                                   nsIIOService             *ioService = nsnull)
{
    nsCOMPtr<nsIInputStream> in;
    nsresult rv = NS_OpenURI(getter_AddRefs(in), uri, ioService);
    if (NS_SUCCEEDED(rv)) {
        nsCOMPtr<nsIPersistentProperties> properties = 
            do_CreateInstance(NS_PERSISTENTPROPERTIES_CONTRACTID, &rv);
        if (NS_SUCCEEDED(rv)) {
            rv = properties->Load(in);
            if (NS_SUCCEEDED(rv)) {
                *result = nsnull;
                properties.swap(*result);
            }
        }
    }
    return rv;
}

inline nsresult
NS_LoadPersistentPropertiesFromURISpec(nsIPersistentProperties **result,
                                       const nsACString        &spec,
                                       const char              *charset = nsnull,
                                       nsIURI                  *baseURI = nsnull,
                                       nsIIOService            *ioService = nsnull)     
{
    nsCOMPtr<nsIURI> uri;
    nsresult rv = 
        NS_NewURI(getter_AddRefs(uri), spec, charset, baseURI, ioService);

    if (NS_SUCCEEDED(rv))
        rv = NS_LoadPersistentPropertiesFromURI(result, uri, ioService);

    return rv;
}

/**
 * NS_QueryNotificationCallbacks implements the canonical algorithm for
 * querying interfaces from a channel's notification callbacks.  It first
 * searches the channel's notificationCallbacks attribute, and if the interface
 * is not found there, then it inspects the notificationCallbacks attribute of
 * the channel's loadGroup.
 */
inline void
NS_QueryNotificationCallbacks(nsIChannel   *channel,
                              const nsIID  &iid,
                              void        **result)
{
    NS_PRECONDITION(channel, "null channel");
    *result = nsnull;

    nsCOMPtr<nsIInterfaceRequestor> cbs;
    channel->GetNotificationCallbacks(getter_AddRefs(cbs));
    if (cbs)
        cbs->GetInterface(iid, result);
    if (!*result) {
        // try load group's notification callbacks...
        nsCOMPtr<nsILoadGroup> loadGroup;
        channel->GetLoadGroup(getter_AddRefs(loadGroup));
        if (loadGroup) {
            loadGroup->GetNotificationCallbacks(getter_AddRefs(cbs));
            if (cbs)
                cbs->GetInterface(iid, result);
        }
    }
}

/* template helper */
template <class T> inline void
NS_QueryNotificationCallbacks(nsIChannel  *channel,
                              nsCOMPtr<T> &result)
{
    NS_QueryNotificationCallbacks(channel, NS_GET_TEMPLATE_IID(T),
                                  getter_AddRefs(result));
}

/**
 * Alternate form of NS_QueryNotificationCallbacks designed for use by
 * nsIChannel implementations.
 */
inline void
NS_QueryNotificationCallbacks(nsIInterfaceRequestor  *callbacks,
                              nsILoadGroup           *loadGroup,
                              const nsIID            &iid,
                              void                  **result)
{
    *result = nsnull;

    if (callbacks)
        callbacks->GetInterface(iid, result);
    if (!*result) {
        // try load group's notification callbacks...
        if (loadGroup) {
            nsCOMPtr<nsIInterfaceRequestor> cbs;
            loadGroup->GetNotificationCallbacks(getter_AddRefs(cbs));
            if (cbs)
                cbs->GetInterface(iid, result);
        }
    }
}

/* template helper */
template <class T> inline void
NS_QueryNotificationCallbacks(nsIInterfaceRequestor *callbacks,
                              nsILoadGroup          *loadGroup,
                              nsCOMPtr<T>           &result)
{
    NS_QueryNotificationCallbacks(callbacks, loadGroup,
                                  NS_GET_TEMPLATE_IID(T),
                                  getter_AddRefs(result));
}

/* template helper */
template <class T> inline void
NS_QueryNotificationCallbacks(const nsCOMPtr<nsIInterfaceRequestor> &aCallbacks,
                              const nsCOMPtr<nsILoadGroup>          &aLoadGroup,
                              nsCOMPtr<T>                           &aResult)
{
    NS_QueryNotificationCallbacks(aCallbacks.get(), aLoadGroup.get(), aResult);
}

/* template helper */
template <class T> inline void
NS_QueryNotificationCallbacks(const nsCOMPtr<nsIChannel> &aChannel,
                              nsCOMPtr<T>                &aResult)
{
    NS_QueryNotificationCallbacks(aChannel.get(), aResult);
}

/**
 * This function returns a nsIInterfaceRequestor instance that returns the
 * same result as NS_QueryNotificationCallbacks when queried.  It is useful
 * as the value for nsISocketTransport::securityCallbacks.
 */
inline nsresult
NS_NewNotificationCallbacksAggregation(nsIInterfaceRequestor  *callbacks,
                                       nsILoadGroup           *loadGroup,
                                       nsIInterfaceRequestor **result)
{
    nsCOMPtr<nsIInterfaceRequestor> cbs;
    if (loadGroup)
        loadGroup->GetNotificationCallbacks(getter_AddRefs(cbs));
    return NS_NewInterfaceRequestorAggregation(callbacks, cbs, result);
}

/**
 * Helper function for testing online/offline state of the browser.
 */
inline PRBool
NS_IsOffline()
{
    PRBool offline = PR_TRUE;
    nsCOMPtr<nsIIOService> ios = do_GetIOService();
    if (ios)
        ios->GetOffline(&offline);
    return offline;
}

/**
 * Helper functions for implementing nsINestedURI::innermostURI.
 *
 * Note that NS_DoImplGetInnermostURI is "private" -- call
 * NS_ImplGetInnermostURI instead.
 */
inline nsresult
NS_DoImplGetInnermostURI(nsINestedURI* nestedURI, nsIURI** result)
{
    NS_PRECONDITION(nestedURI, "Must have a nested URI!");
    NS_PRECONDITION(!*result, "Must have null *result");
    
    nsCOMPtr<nsIURI> inner;
    nsresult rv = nestedURI->GetInnerURI(getter_AddRefs(inner));
    NS_ENSURE_SUCCESS(rv, rv);

    nsCOMPtr<nsINestedURI> nestedInner(do_QueryInterface(inner));
    if (!nestedInner) {
        // Found the innermost one
        inner.swap(*result);
        return NS_OK;
    }

    return NS_DoImplGetInnermostURI(nestedInner, result);
}

inline nsresult
NS_ImplGetInnermostURI(nsINestedURI* nestedURI, nsIURI** result)
{
    // Make it safe to use swap()
    *result = nsnull;

    return NS_DoImplGetInnermostURI(nestedURI, result);
}

/**
 * Helper function that ensures that |result| is a URI that's safe to
 * return.  If |uri| is immutable, just returns it, otherwise returns
 * a clone.  |uri| must not be null.
 */
inline nsresult
NS_EnsureSafeToReturn(nsIURI* uri, nsIURI** result)
{
    NS_PRECONDITION(uri, "Must have a URI");
    
    // Assume mutable until told otherwise
    PRBool isMutable = PR_TRUE;
    nsCOMPtr<nsIMutable> mutableObj(do_QueryInterface(uri));
    if (mutableObj) {
        nsresult rv = mutableObj->GetMutable(&isMutable);
        isMutable = NS_FAILED(rv) || isMutable;
    }

    if (!isMutable) {
        NS_ADDREF(*result = uri);
        return NS_OK;
    }

    return uri->Clone(result);
}

/**
 * Helper function that tries to set the argument URI to be immutable
 */  
inline void
NS_TryToSetImmutable(nsIURI* uri)
{
    nsCOMPtr<nsIMutable> mutableObj(do_QueryInterface(uri));
    if (mutableObj) {
        mutableObj->SetMutable(PR_FALSE);
    }
}

/**
 * Helper function for calling ToImmutableURI.  If all else fails, returns
 * the input URI.  The optional second arg indicates whether we had to fall
 * back to the input URI.  Passing in a null URI is ok.
 */
inline already_AddRefed<nsIURI>
NS_TryToMakeImmutable(nsIURI* uri,
                      nsresult* outRv = nsnull)
{
    nsresult rv;
    nsCOMPtr<nsINetUtil> util = do_GetIOService(&rv);
    nsIURI* result = nsnull;
    if (NS_SUCCEEDED(rv)) {
        NS_ASSERTION(util, "do_GetIOService lied");
        rv = util->ToImmutableURI(uri, &result);
    }

    if (NS_FAILED(rv)) {
        NS_IF_ADDREF(result = uri);
    }

    if (outRv) {
        *outRv = rv;
    }

    return result;
}

/**
 * Helper function for testing whether the given URI, or any of its
 * inner URIs, has all the given protocol flags.
 */
inline nsresult
NS_URIChainHasFlags(nsIURI   *uri,
                    PRUint32  flags,
                    PRBool   *result)
{
    nsresult rv;
    nsCOMPtr<nsINetUtil> util = do_GetIOService(&rv);
    NS_ENSURE_SUCCESS(rv, rv);

    return util->URIChainHasFlags(uri, flags, result);
}

/**
 * Helper function for getting the innermost URI for a given URI.  The return
 * value could be just the object passed in if it's not a nested URI.
 */
inline already_AddRefed<nsIURI>
NS_GetInnermostURI(nsIURI *uri)
{
    NS_PRECONDITION(uri, "Must have URI");
    
    nsCOMPtr<nsINestedURI> nestedURI(do_QueryInterface(uri));
    if (!nestedURI) {
        NS_ADDREF(uri);
        return uri;
    }

    nsresult rv = nestedURI->GetInnermostURI(&uri);
    if (NS_FAILED(rv)) {
        return nsnull;
    }

    return uri;
}

#endif // !nsNetUtil_h__
