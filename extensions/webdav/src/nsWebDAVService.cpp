/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
// vim:expandtab:ts=4 sw=4:
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
 * Oracle Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2004
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Mike Shaver <shaver@off.net> (original author)
 *   Gary van der Merwe <garyvdm@gmail.com>
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

#include "nsWebDAVInternal.h"

#include "nsIWebDAVService.h"
#include "nsWebDAVServiceCID.h"

#include "nsServiceManagerUtils.h"
#include "nsIClassInfoImpl.h"

#include "nsIHttpChannel.h"
#include "nsIIOService.h"
#include "nsNetUtil.h"
#include "nsIStorageStream.h"
#include "nsIUploadChannel.h"
#include "nsIURL.h"

#include "nsContentCID.h"

#include "nsIDOMXMLDocument.h"
#include "nsIDOMElement.h"
#include "nsIDOMNodeList.h"
#include "nsIDOM3Node.h"
#include "nsIPrivateDOMImplementation.h" // I don't even pretend any more
#include "nsIDOMDOMImplementation.h"

#include "nsIDocument.h"
#include "nsIDocumentEncoder.h"
#include "nsContentCID.h"

#include "nsIDOMParser.h"

#include "nsIGenericFactory.h"

class nsWebDAVService : public nsIWebDAVService
{
public:
    NS_DECL_ISUPPORTS
    NS_DECL_NSIWEBDAVSERVICE
    
    nsWebDAVService();
    virtual ~nsWebDAVService();
protected:
    nsresult EnsureIOService();
    nsresult ChannelFromResource(nsIWebDAVResource *resource,
                                 nsIInterfaceRequestor *notificationCallbacks,
                                 nsIHttpChannel** channel,
                                 nsIURI ** resourceURI = 0);

    nsresult CreatePropfindDocument(nsIURI *resourceURI,
                                    nsIDOMDocument **requestDoc,
                                    nsIDOMElement **propfindElt);

    nsresult PropfindInternal(nsIWebDAVResource *resource, PRUint32 propCount,
                              const char **properties, PRBool withDepth,
                              nsIWebDAVOperationListener *listener,
                              nsIInterfaceRequestor *notificationCallbacks,
                              nsISupports *closure, PRBool namesOnly);

    nsresult SendDocumentToChannel(nsIDocument *doc, nsIHttpChannel *channel,
                                   const char *const method,
                                   nsIStreamListener *listener,
                                   PRBool withDepth);
    nsCOMPtr<nsIIOService> mIOService; // XXX weak?
    nsAutoString mDAVNSString; // "DAV:"
};

NS_IMPL_ISUPPORTS1_CI(nsWebDAVService, nsIWebDAVService)

#define ENSURE_IO_SERVICE()                     \
{                                               \
    nsresult rv_ = EnsureIOService();           \
    NS_ENSURE_SUCCESS(rv_, rv_);                \
}

nsresult
nsWebDAVService::EnsureIOService()
{
    if (!mIOService) {
        nsresult rv;
        mIOService = do_GetIOService(&rv);
        if (!mIOService)
            return rv;
    }

    return NS_OK;
}

nsresult
nsWebDAVService::SendDocumentToChannel(nsIDocument *doc,
                                       nsIHttpChannel *channel, 
                                       const char *const method,
                                       nsIStreamListener *listener,
                                       PRBool withDepth)
{
    // Why do I have to pick values for these?  I just want to store some data
    // for stream access!  (And how would script set these?)
    nsresult rv;
    nsCOMPtr<nsIStorageStream> const storageStream(
        do_CreateInstance("@mozilla.org/storagestream;1", &rv));
    NS_ENSURE_SUCCESS(rv, rv);
    rv = storageStream->Init(4096, PR_UINT32_MAX, nsnull);
    NS_ENSURE_SUCCESS(rv, rv);

    nsCOMPtr<nsIOutputStream> storageOutputStream;
    rv = storageStream->GetOutputStream(0,
                                        getter_AddRefs(storageOutputStream));
    NS_ENSURE_SUCCESS(rv, rv);

    nsCOMPtr<nsIDocumentEncoder> encoder =
        do_CreateInstance(NS_DOC_ENCODER_CONTRACTID_BASE "text/xml", &rv);
    NS_ENSURE_SUCCESS(rv, rv);
    
    nsCOMPtr<nsIDOMDocument> domDoc = do_QueryInterface(doc);
    NS_ASSERTION(domDoc, "Need a document");

    rv = encoder->Init(domDoc, NS_LITERAL_STRING("text/xml"),
                       nsIDocumentEncoder::OutputEncodeBasicEntities);
    NS_ENSURE_SUCCESS(rv, rv);

    encoder->SetCharset(NS_LITERAL_CSTRING("UTF-8"));
    rv =  encoder->EncodeToStream(storageOutputStream);
    NS_ENSURE_SUCCESS(rv, rv);

    storageOutputStream->Close();

    // You gotta really want it.
#ifdef PR_LOGGING
    if (PR_LOG_TEST(gDAVLog, 5)) {
        nsCOMPtr<nsIInputStream> logInputStream;
        rv = storageStream->NewInputStream(0, getter_AddRefs(logInputStream));
        NS_ENSURE_SUCCESS(rv, rv);

        PRUint32 len, read;
        logInputStream->Available(&len);

        char *buf = new char[len+1];
        memset(buf, 0, len+1);
        logInputStream->Read(buf, len, &read);
        NS_ASSERTION(len == read, "short read on closed storage stream?");
        LOG(("XML:\n\n%*s\n\n", len, buf));
        
        delete [] buf;
    }
#endif

    nsCOMPtr<nsIInputStream> inputStream;
    rv = storageStream->NewInputStream(0, getter_AddRefs(inputStream));
    NS_ENSURE_SUCCESS(rv, rv);

    nsCOMPtr<nsIUploadChannel> uploadChannel = do_QueryInterface(channel, &rv);
    NS_ENSURE_SUCCESS(rv, rv);
    rv = uploadChannel->SetUploadStream(inputStream,
                                        NS_LITERAL_CSTRING("text/xml"), -1);
    NS_ENSURE_SUCCESS(rv, rv);
    
    channel->SetRequestMethod(nsDependentCString(method));
    channel->SetRequestHeader(NS_LITERAL_CSTRING("Content-Type"),
                              NS_LITERAL_CSTRING("text/xml; charset=utf-8"),
                              PR_FALSE);
    channel->SetRequestHeader(NS_LITERAL_CSTRING("Accept"),
                              NS_LITERAL_CSTRING("text/xml"),
                              PR_FALSE);
    channel->SetRequestHeader(NS_LITERAL_CSTRING("Accept-Charset"),
                              NS_LITERAL_CSTRING("utf-8,*;q=0.1"),
                              PR_FALSE);
    

    if (withDepth) {
        channel->SetRequestHeader(NS_LITERAL_CSTRING("Depth"),
                                  NS_LITERAL_CSTRING("1"), PR_FALSE);
    } else {
        channel->SetRequestHeader(NS_LITERAL_CSTRING("Depth"),
                                  NS_LITERAL_CSTRING("0"), PR_FALSE);
    }

#ifdef PR_LOGGING
    if (LOG_ENABLED()) {
        nsCOMPtr<nsIURI> uri;
        channel->GetURI(getter_AddRefs(uri));
        nsCAutoString spec;
        uri->GetSpec(spec);
        LOG(("%s starting for %s", method, spec.get()));
    }
#endif

    return channel->AsyncOpen(listener, channel);
}

nsresult
nsWebDAVService::CreatePropfindDocument(nsIURI *resourceURI,
                                        nsIDOMDocument **requestDoc,
                                        nsIDOMElement **propfindElt)
{
    nsresult rv;
    static NS_DEFINE_CID(kDOMDOMDOMDOMImplementationCID,
                         NS_DOM_IMPLEMENTATION_CID);
    nsCOMPtr<nsIDOMDOMImplementation>
        implementation(do_CreateInstance(kDOMDOMDOMDOMImplementationCID, &rv));
    NS_ENSURE_SUCCESS(rv, rv);

    nsCOMPtr<nsIPrivateDOMImplementation> 
        privImpl(do_QueryInterface(implementation));
    // XXXbz I doubt this is right, but I have no idea what this code is doing
    // or why it's creating documents without a useful principal... so I'm just
    // going to make the fact that those documents have no principal very
    // explicit, and if this causes issues then someone familiar with this code
    // should figure out what principals this _should_ be using.
    privImpl->Init(resourceURI, resourceURI, nsnull);

    nsCOMPtr<nsIDOMDocument> doc;
    rv = implementation->CreateDocument(mDAVNSString, EmptyString(), nsnull,
                                        getter_AddRefs(doc));
    NS_ENSURE_SUCCESS(rv, rv);

    nsCOMPtr<nsIDocument> baseDoc = do_QueryInterface(doc);
    baseDoc->SetXMLDeclaration(NS_LITERAL_STRING("1.0").get(),
                               EmptyString().get(), -1);
    baseDoc->SetDocumentURI(resourceURI);

    nsCOMPtr<nsIDOMElement> elt;
    rv = NS_WD_AppendElementWithNS(doc, doc, mDAVNSString,
                                   NS_LITERAL_STRING("propfind"),
                                   getter_AddRefs(elt));
    elt->SetPrefix(NS_LITERAL_STRING("D"));
    NS_ENSURE_SUCCESS(rv, rv);

    *requestDoc = doc.get();
    NS_ADDREF(*requestDoc);
    *propfindElt = elt.get();
    NS_ADDREF(*propfindElt);

    return NS_OK;
}

nsresult
nsWebDAVService::ChannelFromResource(nsIWebDAVResource *aResource,
                                     nsIInterfaceRequestor *notificationCallbacks,
                                     nsIHttpChannel **aChannel,
                                     nsIURI **aResourceURI)
{
    ENSURE_IO_SERVICE();

    nsCOMPtr<nsIURL> resourceURL;

    nsresult rv = aResource->GetResourceURL(getter_AddRefs(resourceURL));
    NS_ENSURE_SUCCESS(rv, rv);

    nsCOMPtr<nsIChannel> baseChannel;
    rv = mIOService->NewChannelFromURI(resourceURL, getter_AddRefs(baseChannel));
    NS_ENSURE_SUCCESS(rv, rv);

    rv = baseChannel->SetNotificationCallbacks(notificationCallbacks);
    NS_ENSURE_SUCCESS(rv, rv);

    nsLoadFlags loadFlags;
    rv = baseChannel->GetLoadFlags(&loadFlags);
    if (NS_SUCCEEDED(rv)) {
        rv = baseChannel->SetLoadFlags(loadFlags | 
                                       nsIRequest::VALIDATE_ALWAYS);
        NS_ASSERTION(NS_SUCCEEDED(rv),
                     "nsWebDavService::ChannelFromResource(): "
                     "Couldn't set loadflags on channel");
    }

    rv = CallQueryInterface(baseChannel, aChannel);

    if (NS_SUCCEEDED(rv) && aResourceURI) {
        *aResourceURI = resourceURL.get();
        NS_ADDREF(*aResourceURI);
    }

    return rv;
}

nsWebDAVService::nsWebDAVService() :
    mDAVNSString(NS_LITERAL_STRING("DAV:"))

{
#ifdef PR_LOGGING
    gDAVLog = PR_NewLogModule("webdav");
#endif
}

nsWebDAVService::~nsWebDAVService()
{
  /* destructor code */
}

NS_IMETHODIMP
nsWebDAVService::LockResources(PRUint32 count, nsIWebDAVResource **resources,
                               nsIWebDAVOperationListener *listener,
                               nsIInterfaceRequestor *notificationCallbacks,
                               nsISupports *closure)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP
nsWebDAVService::UnlockResources(PRUint32 count, nsIWebDAVResource **resources,
                                 nsIWebDAVOperationListener *listener,
                                 nsIInterfaceRequestor *notificationCallbacks,
                                 nsISupports *closure)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP
nsWebDAVService::GetResourcePropertyNames(nsIWebDAVResource *resource,
                                          PRBool withDepth,
                                          nsIWebDAVOperationListener *listener,
                                          nsIInterfaceRequestor *notificationCallbacks,
                                          nsISupports *closure)
{
    return PropfindInternal(resource, 0, nsnull, withDepth,
                            listener, notificationCallbacks, closure, PR_TRUE);
}

NS_IMETHODIMP
nsWebDAVService::GetResourceProperties(nsIWebDAVResource *resource,
                                       PRUint32 propCount,
                                       const char **properties,
                                       PRBool withDepth,
                                       nsIWebDAVOperationListener *listener,
                                       nsIInterfaceRequestor *notificationCallbacks,
                                       nsISupports *closure)
{
    return PropfindInternal(resource, propCount, properties, withDepth,
                            listener, notificationCallbacks, closure, PR_FALSE);
}

nsresult
nsWebDAVService::PropfindInternal(nsIWebDAVResource *resource,
                                  PRUint32 propCount,
                                  const char **properties,
                                  PRBool withDepth,
                                  nsIWebDAVOperationListener *listener,
                                  nsIInterfaceRequestor *notificationCallbacks,
                                  nsISupports *closure,
                                  PRBool namesOnly)
{
    nsresult rv;

    NS_ENSURE_ARG(resource);
    NS_ENSURE_ARG(listener);

    nsCOMPtr<nsIURI> resourceURI;
    nsCOMPtr<nsIHttpChannel> channel;
    rv = ChannelFromResource(resource, notificationCallbacks, getter_AddRefs(channel),
                             getter_AddRefs(resourceURI));
    if (NS_FAILED(rv))
        return rv;

    nsCOMPtr<nsIDOMDocument> requestDoc;
    nsCOMPtr<nsIDOMElement> propfindElt;
    rv = CreatePropfindDocument(resourceURI, getter_AddRefs(requestDoc),
                                getter_AddRefs(propfindElt));
    NS_ENSURE_SUCCESS(rv, rv);

    if (namesOnly) {
        nsCOMPtr<nsIDOMElement> allpropElt;
        rv = NS_WD_AppendElementWithNS(requestDoc, propfindElt,
                                       mDAVNSString, NS_LITERAL_STRING("propname"),
                                       getter_AddRefs(allpropElt));
        NS_ENSURE_SUCCESS(rv, rv);
    } else if (propCount == 0) {
        nsCOMPtr<nsIDOMElement> allpropElt;
        rv = NS_WD_AppendElementWithNS(requestDoc, propfindElt,
                                       mDAVNSString, NS_LITERAL_STRING("allprop"),
                                       getter_AddRefs(allpropElt));
        NS_ENSURE_SUCCESS(rv, rv);
    } else {
        nsCOMPtr<nsIDOMElement> propElt;
        rv = NS_WD_AppendElementWithNS(requestDoc, propfindElt,
                                       mDAVNSString, NS_LITERAL_STRING("prop"),
                                       getter_AddRefs(propElt));
        NS_ENSURE_SUCCESS(rv, rv);

        for (PRUint32 i = 0; i < propCount; i++) {
            nsDependentCString fullpropName(properties[i]);

            PRInt32 const index = fullpropName.RFindChar(' ');
            if (index < 0) {
                nsCAutoString msg(NS_LITERAL_CSTRING("Illegal property name "));
                msg += fullpropName;
                msg.Append('\n');
                NS_WARNING(msg.get());
                return NS_ERROR_INVALID_ARG;
            }

            nsDependentCSubstring const propNamespace_(fullpropName, 0, index);
            nsDependentCSubstring const propName_(fullpropName, index + 1);
#ifdef PR_LOGGING
            if (LOG_ENABLED()) {
                nsCString const ns(propNamespace_);
                nsCString const name(propName_);
                LOG(("prop ns: '%s', name: '%s'", ns.get(), name.get()));
            }
#endif
            NS_ConvertASCIItoUTF16 const propNamespace(propNamespace_);
            NS_ConvertASCIItoUTF16 const propName(propName_);

            nsCOMPtr<nsIDOMElement> junk;
            rv = NS_WD_AppendElementWithNS(requestDoc, propElt, propNamespace,
                                           propName, getter_AddRefs(junk));
            NS_ENSURE_SUCCESS(rv, rv);
        }
    }

    nsCOMPtr<nsIStreamListener> streamListener = 
        NS_WD_NewPropfindStreamListener(resource, listener, closure,
                                        namesOnly);

    if (!streamListener)
        return NS_ERROR_OUT_OF_MEMORY;
    
    nsCOMPtr<nsIDocument> requestBaseDoc = do_QueryInterface(requestDoc);
    return SendDocumentToChannel(requestBaseDoc, channel, "PROPFIND",
                                 streamListener, withDepth);
}

NS_IMETHODIMP
nsWebDAVService::GetResourceOptions(nsIWebDAVResource *resource,
                                    nsIWebDAVOperationListener *listener,
                                    nsIInterfaceRequestor *notificationCallbacks,
                                    nsISupports *closure)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP
nsWebDAVService::Get(nsIWebDAVResource *resource, nsIStreamListener *listener,
                     nsIInterfaceRequestor *notificationCallbacks)
{
    nsresult rv;
    nsCOMPtr<nsIHttpChannel> channel;
    rv = ChannelFromResource(resource, notificationCallbacks, getter_AddRefs(channel));
    if (NS_FAILED(rv))
        return rv;

    return channel->AsyncOpen(listener, channel);
}

NS_IMETHODIMP
nsWebDAVService::GetToOutputStream(nsIWebDAVResource *resource,
                                   nsIOutputStream *stream,
                                   nsIWebDAVOperationListener *listener,
                                   nsIInterfaceRequestor *notificationCallbacks,
                                   nsISupports *closure)
{
    nsCOMPtr<nsIRequestObserver> getObserver;
    nsresult rv;

    rv = NS_WD_NewGetOperationRequestObserver(resource, listener, closure, 
                                              stream,
                                              getter_AddRefs(getObserver));
    NS_ENSURE_SUCCESS(rv, rv);

    nsCOMPtr<nsIStreamListener> streamListener;
    rv = NS_NewSimpleStreamListener(getter_AddRefs(streamListener),
                                    stream, getObserver);
    NS_ENSURE_SUCCESS(rv, rv);

    return Get(resource, streamListener, notificationCallbacks);
}

NS_IMETHODIMP
nsWebDAVService::GetToString(nsIWebDAVResource *resource,
                             nsIWebDAVOperationListener *listener,
                             nsIInterfaceRequestor *notificationCallbacks,
                             nsISupports *closure)
{
    nsCOMPtr<nsIStreamListener> getListener;
    nsresult rv;

    rv = NS_WD_NewGetToStringOperationRequestObserver(resource, listener,
                                                      closure,
                                                      getter_AddRefs(getListener));
    NS_ENSURE_SUCCESS(rv, rv);

    nsCOMPtr<nsIHttpChannel> channel;
    rv = ChannelFromResource(resource, notificationCallbacks, getter_AddRefs(channel));
    NS_ENSURE_SUCCESS(rv, rv);

    return channel->AsyncOpen(getListener, channel);
}

NS_IMETHODIMP
nsWebDAVService::Put(nsIWebDAVResource *resource,
                     const nsACString& contentType, nsIInputStream *data,
                     nsIWebDAVOperationListener *listener,
                     nsIInterfaceRequestor *notificationCallbacks,
                     nsISupports *closure)
{
    nsCOMPtr<nsIHttpChannel> channel;

    nsresult rv = ChannelFromResource(resource, notificationCallbacks, getter_AddRefs(channel));
    NS_ENSURE_SUCCESS(rv, rv);
    
    nsCOMPtr<nsIUploadChannel> uploadChannel = do_QueryInterface(channel, &rv);
    NS_ENSURE_SUCCESS(rv, rv);

    rv = uploadChannel->SetUploadStream(data, contentType, -1);
    NS_ENSURE_SUCCESS(rv, rv);

    nsCOMPtr<nsIStreamListener> streamListener;
    rv = NS_WD_NewOperationStreamListener(resource, listener, closure,
                                          nsIWebDAVOperationListener::PUT,
                                          getter_AddRefs(streamListener));
    NS_ENSURE_SUCCESS(rv, rv);

    channel->SetRequestMethod(NS_LITERAL_CSTRING("PUT"));

#ifdef PR_LOGGING
    if (LOG_ENABLED()) {
        nsCOMPtr<nsIURI> uri;
        channel->GetURI(getter_AddRefs(uri));
        nsCAutoString spec;
        uri->GetSpec(spec);
        LOG(("PUT starting for %s", spec.get()));
    }
#endif

    return channel->AsyncOpen(streamListener, channel);
}

NS_IMETHODIMP
nsWebDAVService::PutFromString(nsIWebDAVResource *resource,
                               const nsACString& contentType,
                               const nsACString& data,
                               nsIWebDAVOperationListener *listener,
                               nsIInterfaceRequestor *notificationCallbacks,
                               nsISupports *closure)
{
    nsresult rv;
    nsCOMPtr<nsIStringInputStream> dataStream = 
        do_CreateInstance("@mozilla.org/io/string-input-stream;1", &rv);
    NS_ENSURE_SUCCESS(rv, rv);

    dataStream->SetData(PromiseFlatCString(data).get(),
                        data.Length());
    return Put(resource, contentType, dataStream, listener, 
               notificationCallbacks, closure);
}

NS_IMETHODIMP
nsWebDAVService::Remove(nsIWebDAVResource *resource,
                        nsIWebDAVOperationListener *listener,
                        nsIInterfaceRequestor *notificationCallbacks,
                        nsISupports *closure)
{
    nsCOMPtr<nsIHttpChannel> channel;
    nsresult rv = ChannelFromResource(resource, notificationCallbacks, getter_AddRefs(channel));
    NS_ENSURE_SUCCESS(rv, rv);

    nsCOMPtr<nsIStreamListener> streamListener;
    rv = NS_WD_NewOperationStreamListener(resource, listener, closure,
                                          nsIWebDAVOperationListener::REMOVE,
                                          getter_AddRefs(streamListener));
    NS_ENSURE_SUCCESS(rv, rv);

    channel->SetRequestMethod(NS_LITERAL_CSTRING("DELETE"));

#ifdef PR_LOGGING
    if (LOG_ENABLED()) {
        nsCOMPtr<nsIURI> uri;
        channel->GetURI(getter_AddRefs(uri));
        nsCAutoString spec;
        uri->GetSpec(spec);
        LOG(("DELETE starting for %s", spec.get()));
    }
#endif

    return channel->AsyncOpen(streamListener, channel);
}

NS_IMETHODIMP
nsWebDAVService::MakeCollection(nsIWebDAVResource *resource,
                                nsIWebDAVOperationListener *listener,
                                nsIInterfaceRequestor *notificationCallbacks,
                                nsISupports *closure)
{
    nsCOMPtr<nsIHttpChannel> channel;
    nsresult rv = ChannelFromResource(resource, notificationCallbacks, getter_AddRefs(channel));
    NS_ENSURE_SUCCESS(rv, rv);

    nsCOMPtr<nsIStreamListener> streamListener;
    rv = NS_WD_NewOperationStreamListener(resource, listener, closure,
                                          nsIWebDAVOperationListener::MAKE_COLLECTION,
                                          getter_AddRefs(streamListener));
    NS_ENSURE_SUCCESS(rv, rv);

    channel->SetRequestMethod(NS_LITERAL_CSTRING("MKCOL"));

#ifdef PR_LOGGING
    if (LOG_ENABLED()) {
        nsCOMPtr<nsIURI> uri;
        channel->GetURI(getter_AddRefs(uri));
        nsCAutoString spec;
        uri->GetSpec(spec);
        LOG(("MKCOL starting for %s", spec.get()));
    }
#endif

    return channel->AsyncOpen(streamListener, channel);
}

NS_IMETHODIMP
nsWebDAVService::MoveTo(nsIWebDAVResource *resource,
                        const nsACString &destination,
                        PRBool overwrite,
                        nsIWebDAVOperationListener *listener,
                        nsIInterfaceRequestor *notificationCallbacks,
                        nsISupports *closure)
{
    nsCOMPtr<nsIHttpChannel> channel;
    nsresult rv = ChannelFromResource(resource, notificationCallbacks, getter_AddRefs(channel));
    NS_ENSURE_SUCCESS(rv, rv);

    nsCOMPtr<nsIStreamListener> streamListener;
    rv = NS_WD_NewOperationStreamListener(resource, listener, closure,
                                          nsIWebDAVOperationListener::COPY,
                                          getter_AddRefs(streamListener));
    NS_ENSURE_SUCCESS(rv, rv);

    channel->SetRequestMethod(NS_LITERAL_CSTRING("MOVE"));

    if (!overwrite) {
        channel->SetRequestHeader(NS_LITERAL_CSTRING("Overwrite"),
                                  NS_LITERAL_CSTRING("F"),
                                  PR_FALSE);
    } else {
        channel->SetRequestHeader(NS_LITERAL_CSTRING("Overwrite"),
                                  NS_LITERAL_CSTRING("F"),
                                  PR_FALSE);
    }

    channel->SetRequestHeader(NS_LITERAL_CSTRING("Destination"),
                              destination, PR_FALSE);

#ifdef PR_LOGGING
    if (LOG_ENABLED()) {
        nsCOMPtr<nsIURI> uri;
        channel->GetURI(getter_AddRefs(uri));
        nsCAutoString spec;
        uri->GetSpec(spec);
        LOG(("MOVE starting for %s -> %s", spec.get(),
             nsCAutoString(destination).get()));
    }
#endif

    return channel->AsyncOpen(streamListener, channel);
}

NS_IMETHODIMP
nsWebDAVService::CopyTo(nsIWebDAVResource *resource,
                        const nsACString &destination,
                        PRBool recursive, PRBool overwrite,
                        nsIWebDAVOperationListener *listener,
                        nsIInterfaceRequestor *notificationCallbacks,
                        nsISupports *closure)

{
    nsCOMPtr<nsIHttpChannel> channel;
    nsresult rv = ChannelFromResource(resource, notificationCallbacks, getter_AddRefs(channel));
    NS_ENSURE_SUCCESS(rv, rv);

    nsCOMPtr<nsIStreamListener> streamListener;
    rv = NS_WD_NewOperationStreamListener(resource, listener, closure,
                                          nsIWebDAVOperationListener::COPY,
                                          getter_AddRefs(streamListener));
    NS_ENSURE_SUCCESS(rv, rv);

    channel->SetRequestMethod(NS_LITERAL_CSTRING("COPY"));
    if (!overwrite) {
        channel->SetRequestHeader(NS_LITERAL_CSTRING("Overwrite"),
                                  NS_LITERAL_CSTRING("F"),
                                  PR_FALSE);
    } else {
        channel->SetRequestHeader(NS_LITERAL_CSTRING("Overwrite"),
                                  NS_LITERAL_CSTRING("F"),
                                  PR_FALSE);
    }

    if (recursive) {
        channel->SetRequestHeader(NS_LITERAL_CSTRING("Depth"),
                                  NS_LITERAL_CSTRING("infinity"), PR_FALSE);
    } else {
        channel->SetRequestHeader(NS_LITERAL_CSTRING("Depth"),
                                  NS_LITERAL_CSTRING("0"), PR_FALSE);
    }

    channel->SetRequestHeader(NS_LITERAL_CSTRING("Destination"),
                              destination, PR_FALSE);

#ifdef PR_LOGGING
    if (LOG_ENABLED()) {
        nsCOMPtr<nsIURI> uri;
        channel->GetURI(getter_AddRefs(uri));
        nsCAutoString spec;
        uri->GetSpec(spec);
        LOG(("COPY starting for %s -> %s", spec.get(),
             nsCAutoString(destination).get()));
    }
#endif

    return channel->AsyncOpen(streamListener, channel);
}


NS_IMETHODIMP
nsWebDAVService::Report(nsIWebDAVResource *resource, nsIDOMDocument *query, 
                        PRBool withDepth, 
                        nsIWebDAVOperationListener *listener, 
                        nsIInterfaceRequestor *notificationCallbacks,
                        nsISupports *closure)
{
    nsresult rv;

    NS_ENSURE_ARG(resource);
    NS_ENSURE_ARG(query);
    NS_ENSURE_ARG(listener);

    nsCOMPtr<nsIDocument> queryDoc = do_QueryInterface(query, &rv);
    NS_ENSURE_SUCCESS(rv, rv);
    
    nsCOMPtr<nsIURI> resourceURI;
    nsCOMPtr<nsIHttpChannel> channel;
    rv = ChannelFromResource(resource, notificationCallbacks, getter_AddRefs(channel),
                             getter_AddRefs(resourceURI));
    if (NS_FAILED(rv))
        return rv;

    nsCOMPtr<nsIStreamListener> streamListener = 
        NS_WD_NewReportStreamListener(resource, listener, closure);
    if (!streamListener)
        return NS_ERROR_OUT_OF_MEMORY;

    return SendDocumentToChannel(queryDoc, channel, "REPORT", streamListener,
                                 withDepth);
}

NS_GENERIC_FACTORY_CONSTRUCTOR(nsWebDAVService)

NS_DECL_CLASSINFO(nsWebDAVService)

static const nsModuleComponentInfo components[] =
{
    { "WebDAV Service", NS_WEBDAVSERVICE_CID, NS_WEBDAVSERVICE_CONTRACTID,
      nsWebDAVServiceConstructor,
      NULL, NULL, NULL,
      NS_CI_INTERFACE_GETTER_NAME(nsWebDAVService),
      NULL,
      &NS_CLASSINFO_NAME(nsWebDAVService)
    }
};

NS_IMPL_NSGETMODULE(nsWebDAVService, components)
