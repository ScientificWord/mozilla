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

#include "nsIHttpChannel.h"
#include "nsIIOService.h"
#include "nsNetUtil.h"
#include "nsIURL.h"

#include "nsIDOM3Node.h"
#include "nsIDOMDocument.h"
#include "nsIDOMElement.h"
#include "nsIDOMNodeList.h"
#include "nsIDOMRange.h"

#include "nsIDocument.h"
#include "nsIDocumentEncoder.h"
#include "nsContentCID.h"

#include "nsIWebDAVResource.h"
#include "nsIWebDAVListener.h"

#include "nsISupportsPrimitives.h"

class OperationStreamListener : public nsIStreamListener
{
public:
    NS_DECL_ISUPPORTS
    NS_DECL_NSIREQUESTOBSERVER
    NS_DECL_NSISTREAMLISTENER
    
    static NS_METHOD StreamReaderCallback(nsIInputStream *in, void *closure,
                                          const char *fromRawSegment,
                                          PRUint32 toOffset, PRUint32 count,
                                          PRUint32 *writeCount);

    OperationStreamListener(nsIWebDAVResource *resource,
                            nsIWebDAVOperationListener *listener,
                            nsISupports *closure,
                            nsIOutputStream *outstream,
                            PRUint32 mode) :
        mResource(resource), mListener(listener), mClosure(closure), 
        mOutputStream(outstream), mOperation(mode) { }

    virtual ~OperationStreamListener() { }
    
protected:
    virtual nsresult SignalCompletion(PRUint32 status)
    {    
        mListener->OnOperationComplete(status, mResource, mOperation,
                                       mClosure);
        if (mOutputStream)
            return mOutputStream->Flush();
        return NS_OK;
    }

    virtual void SignalDetail(PRUint32 statusCode, const nsACString &resource,
                              nsISupports *detail);

    virtual nsresult ProcessResponse(nsIDOMElement *responseElt);

    nsresult StatusAndHrefFromResponse(nsIDOMElement *responseElt,
                                       nsACString &href,
                                       PRUint32 *statusCode);

    nsCOMPtr<nsIWebDAVResource>          mResource;
    nsCOMPtr<nsIWebDAVOperationListener> mListener;
    nsCOMPtr<nsISupports>                mClosure;
    nsCOMPtr<nsIOutputStream>            mOutputStream;
    PRUint32                             mOperation;
    nsCString                            mBody;
    nsCOMPtr<nsIDOMDocument>             mXMLDoc;
};

NS_IMPL_ADDREF(OperationStreamListener)
NS_IMPL_RELEASE(OperationStreamListener)
NS_INTERFACE_MAP_BEGIN(OperationStreamListener)
  NS_INTERFACE_MAP_ENTRY(nsIStreamListener)
  NS_INTERFACE_MAP_ENTRY(nsIRequestObserver)
  NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(nsISupports, nsIStreamListener)
NS_INTERFACE_MAP_END

NS_IMETHODIMP
OperationStreamListener::OnStartRequest(nsIRequest *aRequest,
                                        nsISupports *aContext)
{
    LOG(("OperationStreamListener::OnStartRequest() entered"));
    mBody.Truncate();
    return NS_OK;
}

NS_IMETHODIMP
OperationStreamListener::OnStopRequest(nsIRequest *aRequest,
                                       nsISupports *aContext,
                                       nsresult aStatusCode)
{
    PRUint32 status, rv;
    nsCOMPtr<nsIHttpChannel> channel = do_QueryInterface(aContext);

    LOG(("OperationStreamListener::OnStopRequest() entered"));

    rv = channel ? channel->GetResponseStatus(&status) : NS_ERROR_UNEXPECTED;

    if (NS_FAILED(rv))
        return SignalCompletion(rv);

    if (status != 207)
        return SignalCompletion(status);

    // ZZZ Check content-length against mBody.Length()

    // Now we parse!
    nsCOMPtr<nsIDOMNodeList> responseList;
    PRUint32 length;
    rv = NS_WD_GetDocAndResponseListFromBuffer(mBody, getter_AddRefs(mXMLDoc),
                                               getter_AddRefs(responseList),
                                               &length);
    NS_ENSURE_SUCCESS(rv, SignalCompletion(rv));

    LOG(("found %d responses", length));
    
    for (PRUint32 i = 0; i < length; i++) {
        nsCOMPtr<nsIDOMNode> responseNode;
        rv = responseList->Item(i, getter_AddRefs(responseNode));
        NS_ENSURE_SUCCESS(rv, SignalCompletion(rv));

        nsCOMPtr<nsIDOMElement> responseElt =
            do_QueryInterface(responseNode, &rv);
        NS_ENSURE_SUCCESS(rv, SignalCompletion(rv));
    
        rv = ProcessResponse(responseElt);
        NS_ENSURE_SUCCESS(rv, SignalCompletion(rv));
    }

    SignalCompletion(status);
    return NS_OK;
}

NS_METHOD
OperationStreamListener::StreamReaderCallback(nsIInputStream *aInputStream,
                                              void *aClosure,
                                              const char *aRawSegment,
                                              PRUint32 aToOffset,
                                              PRUint32 aCount,
                                              PRUint32 *aWriteCount)
{
    OperationStreamListener *osl = static_cast<OperationStreamListener *>
                                              (aClosure);
    osl->mBody.Append(aRawSegment, aCount);
    *aWriteCount = aCount;
    return NS_OK;
}

NS_IMETHODIMP
OperationStreamListener::OnDataAvailable(nsIRequest *aRequest,
                                         nsISupports *aContext,
                                         nsIInputStream *aInputStream,
                                         PRUint32 offset, PRUint32 count)
{
    LOG(("OperationStreamListener::OnDataAvailable() entered"));
    PRUint32 result;
    nsCOMPtr<nsIHttpChannel> channel = do_QueryInterface(aContext);
    if (!channel)
        result = NS_ERROR_UNEXPECTED;

    PRBool succeeded = PR_FALSE;
    channel->GetRequestSucceeded(&succeeded);
    if (!succeeded) {
        aRequest->Cancel(NS_BINDING_ABORTED);
        return NS_BINDING_ABORTED;
    }

    PRUint32 totalRead;
    return aInputStream->ReadSegments(StreamReaderCallback, this, count,
                                      &totalRead);
}

void
OperationStreamListener::SignalDetail(PRUint32 statusCode,
                                      const nsACString &resource,
                                      nsISupports *detail)
{
    nsCOMPtr<nsIURL> resourceURL, detailURL;
    nsCOMPtr<nsIURI> detailURI;
    if (NS_FAILED(mResource->GetResourceURL(getter_AddRefs(resourceURL))))
        return;
    if (resource.IsEmpty()) {
        detailURL = resourceURL;
    } else {
        // if this URL is relative, resolve it.
        nsresult rv;
        nsCAutoString resolvedSpec;
        rv = resourceURL->Resolve(resource, resolvedSpec);

        // XXX better error handling
        NS_ASSERTION(NS_SUCCEEDED(rv), "Failed to resolve remote URL!");

        if (NS_FAILED(resourceURL->Clone(getter_AddRefs(detailURI))) ||
            !(detailURL = do_QueryInterface(detailURI)) ||
            NS_FAILED(detailURI->SetSpec(resolvedSpec))) {
            return;
        }
    }
    mListener->OnOperationDetail(statusCode, detailURL, mOperation, detail,
                                 mClosure);
}

nsresult
OperationStreamListener::ProcessResponse(nsIDOMElement *responseElt)
{
    nsCAutoString href;
    PRUint32 statusCode;
    nsresult rv = StatusAndHrefFromResponse(responseElt, href, &statusCode);
    NS_ENSURE_SUCCESS(rv, rv);

    SignalDetail(statusCode, href, nsnull);
    return NS_OK;
}

nsresult
OperationStreamListener::StatusAndHrefFromResponse(nsIDOMElement *responseElt,
                                                   nsACString &href,
                                                   PRUint32 *statusCode)
{
    nsAutoString hrefString;
    nsresult rv = NS_WD_ElementTextChildValue(responseElt,
                                              NS_LITERAL_STRING("href"),
                                              hrefString);
    NS_ENSURE_SUCCESS(rv, rv);
    href = NS_ConvertUTF16toUTF8(hrefString);
    
    nsAutoString statusString;
    rv = NS_WD_ElementTextChildValue(responseElt, NS_LITERAL_STRING("status"),
                                     statusString);
    // XXX if we don't find a status element (or hit any other parse error,
    // for that matter) we need to signal this back to the caller
    //
    NS_ENSURE_SUCCESS(rv, rv);

    NS_ConvertUTF16toUTF8 statusUTF8(statusString);
    LOG(("status: %s", statusUTF8.get()));
    PRInt32 statusVal = nsCAutoString(Substring(statusUTF8, 8)).ToInteger(&rv, 10);
    NS_ENSURE_SUCCESS(rv, rv);
    
    *statusCode = (PRUint32)statusVal;
    
    return NS_OK;
}

class PropfindStreamListener : public OperationStreamListener
{
public:
    NS_DECL_ISUPPORTS_INHERITED

    PropfindStreamListener(nsIWebDAVResource *resource,
                           nsIWebDAVOperationListener *listener,
                           nsISupports *closure,
                           PRBool isPropname) :
        OperationStreamListener(resource, listener, closure, nsnull,
                                isPropname ?
                                (PRUint32)nsIWebDAVOperationListener::GET_PROPERTY_NAMES :
                                (PRUint32)nsIWebDAVOperationListener::GET_PROPERTIES) { }
    virtual ~PropfindStreamListener() { }
protected:

    virtual nsresult ProcessResponse(nsIDOMElement *responseElt);
    virtual nsresult PropertiesFromPropElt(nsIDOMElement *elt,
                                           nsIProperties **retProps);
};

NS_IMPL_ISUPPORTS_INHERITED0(PropfindStreamListener, OperationStreamListener)

nsresult
PropfindStreamListener::PropertiesFromPropElt(nsIDOMElement *propElt,
                                              nsIProperties **retProps)
{
    nsresult rv = CallCreateInstance(NS_PROPERTIES_CONTRACTID, retProps);
    NS_ENSURE_SUCCESS(rv, rv);

    nsIProperties *props = *retProps;

    nsCOMPtr<nsIDOMNodeList> list;
    rv = propElt->GetChildNodes(getter_AddRefs(list));
    NS_ENSURE_SUCCESS(rv, rv);

    PRUint32 length;
    rv = list->GetLength(&length);
    NS_ENSURE_SUCCESS(rv, rv);

    LOG(("%d properties found", length));

    PRUint32 realProps = 0;

    for (PRUint32 i = 0; i < length; i++) {
        nsCOMPtr<nsIDOMNode> node;
        nsCOMPtr<nsIDOM3Node> node3;
        rv = list->Item(i, getter_AddRefs(node));
        NS_ENSURE_SUCCESS(rv, rv);

        PRUint16 type;
        node->GetNodeType(&type);
        if (type != nsIDOMNode::ELEMENT_NODE)
            continue;

        realProps++;
        
        nsCOMPtr<nsIDOMRange> range = 
          do_CreateInstance("@mozilla.org/content/range;1", &rv);
        NS_ENSURE_SUCCESS(rv, rv);

        rv = range->SelectNodeContents(node);
        NS_ENSURE_SUCCESS(rv, rv);
        
        nsString nsStr;
        rv = node->GetNamespaceURI(nsStr);
        NS_ENSURE_SUCCESS(rv, rv);

        nsString propName;
        rv = node->GetLocalName(propName);
        NS_ENSURE_SUCCESS(rv, rv);

        nsStr.Append(' ');
        nsStr += propName;
        NS_ConvertUTF16toUTF8 const propkey(nsStr);
        if (mOperation == nsIWebDAVOperationListener::GET_PROPERTY_NAMES) {
            LOG(("  propname: %s", propkey.get()));
            rv = props->Set(propkey.get(), nsnull);
            NS_ENSURE_SUCCESS(rv, rv);
            continue;
        }

        nsCOMPtr<nsIDocumentEncoder> encoder =
          do_CreateInstance(NS_DOC_ENCODER_CONTRACTID_BASE "text/xml", &rv);

        // This method will fail if no document
        rv = encoder->Init(mXMLDoc, NS_LITERAL_STRING("text/xml"),
                           nsIDocumentEncoder::OutputEncodeBasicEntities);
        NS_ENSURE_SUCCESS(rv, rv);
        
        rv = encoder->SetRange(range);
        NS_ENSURE_SUCCESS(rv, rv);

        nsString valueStr;
        encoder->EncodeToString(valueStr);

        nsCOMPtr<nsISupportsString>
            suppString(do_CreateInstance("@mozilla.org/supports-string;1",
                                         &rv));
        NS_ENSURE_SUCCESS(rv, rv);
        suppString->SetData(valueStr);

        LOG(("  %s = %s", propkey.get(),
             NS_ConvertUTF16toUTF8(valueStr).get()));
        rv = props->Set(propkey.get(), suppString);
        NS_ENSURE_SUCCESS(rv, rv);
    }

    LOG(("%d real properties added", realProps));
    
    return NS_OK;
}

nsresult
PropfindStreamListener::ProcessResponse(nsIDOMElement *responseElt)
{
    nsCAutoString href;
    PRUint32 statusCode;
    nsresult rv = StatusAndHrefFromResponse(responseElt, href, &statusCode);
    NS_ENSURE_SUCCESS(rv, rv);

    LOG(("response for %s: %d", href.get(), statusCode));

    nsCOMPtr<nsIDOMNodeList> proplist;
    rv = responseElt->GetElementsByTagNameNS(NS_LITERAL_STRING("DAV:"),
                                             NS_LITERAL_STRING("propstat"),
                                             getter_AddRefs(proplist));
    NS_ENSURE_SUCCESS(rv, rv);

    PRUint32 length;
    rv = proplist->GetLength(&length);
    NS_ENSURE_SUCCESS(rv, rv);

    nsCOMPtr<nsIDOMNode> node;
    for (PRUint32 i = 0; i < length; i++) {
        rv = proplist->Item(i, getter_AddRefs(node));
        NS_ENSURE_SUCCESS(rv, rv);

        nsCOMPtr<nsIDOMElement> propstatElt = do_QueryInterface(node, &rv);
        NS_ENSURE_SUCCESS(rv, rv);

        nsCOMPtr<nsIDOMElement> elt;
        rv = NS_WD_GetElementByTagName(propstatElt, NS_LITERAL_STRING("prop"),
                                       getter_AddRefs(elt));
        NS_ENSURE_SUCCESS(rv, rv);

        nsCOMPtr<nsIProperties> props;
        rv = PropertiesFromPropElt(elt, getter_AddRefs(props));
        NS_ENSURE_SUCCESS(rv, rv);
        
        SignalDetail(statusCode, href, props);
    }
    return NS_OK;
}


class ReportStreamListener : public OperationStreamListener
{
public:
    NS_DECL_ISUPPORTS_INHERITED

    ReportStreamListener(nsIWebDAVResource *resource,
                         nsIWebDAVOperationListener *listener,
                         nsISupports *closure) :
        OperationStreamListener(resource, listener, closure, nsnull,
                                nsIWebDAVOperationListener::REPORT) { }
    virtual ~ReportStreamListener() { }

protected:
    virtual nsresult ProcessResponse(nsIDOMElement *responseElt);
};

NS_IMPL_ISUPPORTS_INHERITED0(ReportStreamListener, OperationStreamListener)

nsresult
ReportStreamListener::ProcessResponse(nsIDOMElement *responseElt)
{
    nsCAutoString href;
    PRUint32 statusCode;
    nsresult rv = StatusAndHrefFromResponse(responseElt, href, &statusCode);
    NS_ENSURE_SUCCESS(rv, rv);

    LOG(("response for %s: %d", href.get(), statusCode));

    SignalDetail(statusCode, href, responseElt);
    return NS_OK;
}


class GetToStringStreamListener : public OperationStreamListener
{
public:
    NS_DECL_ISUPPORTS_INHERITED

    GetToStringStreamListener(nsIWebDAVResource *resource,
                              nsIWebDAVOperationListener *listener,
                              nsISupports *closure) :
        OperationStreamListener(resource, listener, closure, nsnull,
                                nsIWebDAVOperationListener::GET_TO_STRING)
    { }

    virtual ~GetToStringStreamListener() { }
protected:
    NS_IMETHOD OnStopRequest(nsIRequest *aRequest,
                                   nsISupports *aContext,
                                   nsresult aStatusCode);
};

NS_IMPL_ISUPPORTS_INHERITED0(GetToStringStreamListener, OperationStreamListener)

NS_IMETHODIMP
GetToStringStreamListener::OnStopRequest(nsIRequest *aRequest,
                                         nsISupports *aContext,
                                         nsresult aStatusCode)
{
    PRUint32 status, rv;
    nsCOMPtr<nsIHttpChannel> channel = do_QueryInterface(aContext);

    LOG(("OperationStreamListener::OnStopRequest() entered"));

    rv = channel ? channel->GetResponseStatus(&status) : NS_ERROR_UNEXPECTED;

    if (NS_FAILED(rv))
        return SignalCompletion(rv);

    if (status != 200)
        return SignalCompletion(status);

    nsCOMPtr<nsISupportsCString>
        suppString(do_CreateInstance("@mozilla.org/supports-cstring;1",
                                     &rv));
    NS_ENSURE_SUCCESS(rv, rv);
    suppString->SetData(mBody);

    SignalDetail(status, nsCAutoString(""), suppString);
    SignalCompletion(status);
    return NS_OK;
}

nsIStreamListener *
NS_WD_NewPropfindStreamListener(nsIWebDAVResource *resource,
                                nsIWebDAVOperationListener *listener,
                                nsISupports *closure,
                                PRBool isPropname)
{
    return new PropfindStreamListener(resource, listener, closure, isPropname);
}

nsIStreamListener *
NS_WD_NewReportStreamListener(nsIWebDAVResource *resource,
                              nsIWebDAVOperationListener *listener,
                              nsISupports *closure)
{
    return new ReportStreamListener(resource, listener, closure);
}

nsresult
NS_WD_NewOperationStreamListener(nsIWebDAVResource *resource,
                                 nsIWebDAVOperationListener *listener,
                                 nsISupports *closure,
                                 PRUint32 operation,
                                 nsIStreamListener **streamListener)
{
    nsCOMPtr<nsIRequestObserver> osl = 
        new OperationStreamListener(resource, listener, closure, nsnull,
                                    operation);
    if (!osl)
        return NS_ERROR_OUT_OF_MEMORY;
    return CallQueryInterface(osl, streamListener);
}

nsresult
NS_WD_NewGetOperationRequestObserver(nsIWebDAVResource *resource,
                                     nsIWebDAVOperationListener *listener,
                                     nsISupports *closure,
                                     nsIOutputStream *outstream,
                                     nsIRequestObserver **observer)
{
    nsCOMPtr<nsIRequestObserver> osl = 
        new OperationStreamListener(resource, listener, closure, outstream,
                                    nsIWebDAVOperationListener::GET);
    if (!osl)
        return NS_ERROR_OUT_OF_MEMORY;
    return CallQueryInterface(osl, observer);
}

nsresult
NS_WD_NewGetToStringOperationRequestObserver(nsIWebDAVResource *resource,
                                             nsIWebDAVOperationListener *listener,
                                             nsISupports *closure,
                                             nsIStreamListener **streamListener)
{
    nsCOMPtr<nsIRequestObserver> osl = 
        new GetToStringStreamListener(resource, listener, closure);
    if (!osl)
        return NS_ERROR_OUT_OF_MEMORY;
    return CallQueryInterface(osl, streamListener);
}

