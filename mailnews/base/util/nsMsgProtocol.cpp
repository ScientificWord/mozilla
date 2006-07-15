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

#include "msgCore.h"
#include "nsReadableUtils.h"
#include "nsMsgProtocol.h"
#include "nsIMsgMailNewsUrl.h"
#include "nsIStreamTransportService.h"
#include "nsISocketTransportService.h"
#include "nsISocketTransport.h"
#include "nsXPIDLString.h"
#include "nsSpecialSystemDirectory.h"
#include "nsILoadGroup.h"
#include "nsIIOService.h"
#include "nsNetUtil.h"
#include "nsIFileURL.h"
#include "nsFileStream.h"
#include "nsIMsgWindow.h"
#include "nsIMsgStatusFeedback.h"
#include "nsIWebProgressListener.h"
#include "nsIPipe.h"
#include "nsIPrompt.h"
#include "prprf.h"
#include "plbase64.h"
#include "nsIStringBundle.h"
#include "nsIProtocolProxyService.h"
#include "nsIProxyInfo.h"
#include "nsThreadUtils.h"
#include "nsIPrefBranch.h"
#include "nsIPrefService.h"

NS_IMPL_THREADSAFE_ADDREF(nsMsgProtocol)
NS_IMPL_THREADSAFE_RELEASE(nsMsgProtocol)

NS_INTERFACE_MAP_BEGIN(nsMsgProtocol)
   NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(nsISupports, nsIChannel)
   NS_INTERFACE_MAP_ENTRY(nsIStreamListener)
   NS_INTERFACE_MAP_ENTRY(nsIRequestObserver)
   NS_INTERFACE_MAP_ENTRY(nsIChannel)
   NS_INTERFACE_MAP_ENTRY(nsIRequest)
   NS_INTERFACE_MAP_ENTRY(nsITransportEventSink)
NS_INTERFACE_MAP_END_THREADSAFE

static PRUnichar *FormatStringWithHostNameByID(PRInt32 stringID, nsIMsgMailNewsUrl *msgUri);


nsMsgProtocol::nsMsgProtocol(nsIURI * aURL)
{
  m_flags = 0;
  m_readCount = 0;
  mLoadFlags = 0;
  m_socketIsOpen = PR_FALSE;
		
  m_tempMsgFileSpec = nsSpecialSystemDirectory(nsSpecialSystemDirectory::OS_TemporaryDirectory);
  m_tempMsgFileSpec += "tempMessage.eml";
  mSuppressListenerNotifications = PR_FALSE;
  InitFromURI(aURL);
}

nsresult nsMsgProtocol::InitFromURI(nsIURI *aUrl)
{
  m_url = aUrl;
  
  nsCOMPtr <nsIMsgMailNewsUrl> mailUrl = do_QueryInterface(aUrl);
  if (mailUrl)
  {
    mailUrl->GetLoadGroup(getter_AddRefs(m_loadGroup));
    nsCOMPtr<nsIMsgStatusFeedback> statusFeedback;
    mailUrl->GetStatusFeedback(getter_AddRefs(statusFeedback));
    mProgressEventSink = do_QueryInterface(statusFeedback);
  }
  return NS_OK;
}

nsMsgProtocol::~nsMsgProtocol()
{}


static PRBool gGotTimeoutPref;
static PRInt32 gSocketTimeout = 60;

nsresult
nsMsgProtocol::OpenNetworkSocketWithInfo(const char * aHostName,
                                         PRInt32 aGetPort,
                                         const char *connectionType,
                                         nsIProxyInfo *aProxyInfo,
                                         nsIInterfaceRequestor* callbacks)
{
  NS_ENSURE_ARG(aHostName);

  nsresult rv = NS_OK;
  nsCOMPtr<nsISocketTransportService> socketService (do_GetService(NS_SOCKETTRANSPORTSERVICE_CONTRACTID));
  NS_ENSURE_TRUE(socketService, NS_ERROR_FAILURE);
  
  // with socket connections we want to read as much data as arrives
  m_readCount = -1;

  nsCOMPtr<nsISocketTransport> strans;
  rv = socketService->CreateTransport(&connectionType, connectionType != nsnull,
                                      nsDependentCString(aHostName),
                                      aGetPort, aProxyInfo,
                                      getter_AddRefs(strans));
  if (NS_FAILED(rv)) return rv;

  strans->SetSecurityCallbacks(callbacks);

  // creates cyclic reference!
  strans->SetEventSink(this, NS_GetCurrentThread());

  m_socketIsOpen = PR_FALSE;
  m_transport = strans;

  if (!gGotTimeoutPref)
  {
    nsCOMPtr<nsIPrefBranch> prefBranch = do_GetService(NS_PREFSERVICE_CONTRACTID, &rv);
    if (prefBranch)
    {
      prefBranch->GetIntPref("mailnews.tcptimeout", &gSocketTimeout);
      gGotTimeoutPref = PR_TRUE;
    }
  }
  strans->SetTimeout(nsISocketTransport::TIMEOUT_CONNECT, gSocketTimeout + 60);  
  strans->SetTimeout(nsISocketTransport::TIMEOUT_READ_WRITE, gSocketTimeout);  


  return SetupTransportState();
}

// open a connection on this url
nsresult
nsMsgProtocol::OpenNetworkSocket(nsIURI * aURL, const char *connectionType,
                                 nsIInterfaceRequestor* callbacks) 
{
  NS_ENSURE_ARG(aURL);

  nsCAutoString hostName;
  PRInt32 port = 0;

  aURL->GetPort(&port);
  aURL->GetAsciiHost(hostName);

  nsCOMPtr<nsIProxyInfo> proxyInfo;

  nsCOMPtr<nsIProtocolProxyService> pps =
      do_GetService("@mozilla.org/network/protocol-proxy-service;1");

  NS_ASSERTION(pps, "Couldn't get the protocol proxy service!");

  if (pps) 
  {
      nsresult rv = NS_OK;

      // Yes, this is ugly. But necko needs to grap a protocol handler
      // to ask for flags, and smtp isn't registered as a handler, only
      // mailto.
      // Note that I cannot just clone, and call SetSpec, since Clone on
      // nsSmtpUrl calls nsStandardUrl's clone method, which fails
      // because smtp isn't a registered protocol.
      // So we cheat. Whilst creating a uri manually is valid here,
      // do _NOT_ copy this to use in your own code - bbaetz
      nsCOMPtr<nsIURI> proxyUri = aURL;
      PRBool isSMTP = PR_FALSE;
      if (NS_SUCCEEDED(aURL->SchemeIs("smtp", &isSMTP)) && isSMTP) 
      {
          nsCAutoString spec;
          rv = aURL->GetSpec(spec);
          if (NS_SUCCEEDED(rv)) 
              proxyUri = do_CreateInstance(NS_STANDARDURL_CONTRACTID, &rv);

          if (NS_SUCCEEDED(rv))
              rv = proxyUri->SetSpec(spec);
          if (NS_SUCCEEDED(rv))
              rv = proxyUri->SetScheme(NS_LITERAL_CSTRING("mailto"));
      }
      //
      // XXX(darin): Consider using AsyncResolve instead to avoid blocking
      //             the calling thread in cases where PAC may call into
      //             our DNS resolver.
      //
      if (NS_SUCCEEDED(rv))
          rv = pps->Resolve(proxyUri, 0, getter_AddRefs(proxyInfo));
      NS_ASSERTION(NS_SUCCEEDED(rv), "Couldn't successfully resolve a proxy");
      if (NS_FAILED(rv)) proxyInfo = nsnull;
  }

  return OpenNetworkSocketWithInfo(hostName.get(), port, connectionType,
                                   proxyInfo, callbacks);
}

nsresult nsMsgProtocol::GetFileFromURL(nsIURI * aURL, nsIFile **aResult)
{
  NS_ENSURE_ARG_POINTER(aURL);
  NS_ENSURE_ARG_POINTER(aResult);
  // extract the file path from the uri...
  nsCAutoString urlSpec;
  aURL->GetPath(urlSpec);
  urlSpec.Insert(NS_LITERAL_CSTRING("file://"), 0);
  nsresult rv;

// dougt - there should be an easier way!
  nsCOMPtr<nsIURI> uri;
  if (NS_FAILED(rv = NS_NewURI(getter_AddRefs(uri), urlSpec.get())))
      return rv;

  nsCOMPtr<nsIFileURL>    fileURL = do_QueryInterface(uri);
  if (!fileURL)   return NS_ERROR_FAILURE;

  return fileURL->GetFile(aResult);
  // dougt
}

nsresult nsMsgProtocol::OpenFileSocket(nsIURI * aURL, PRUint32 aStartPosition, PRInt32 aReadCount)
{
  // mscott - file needs to be encoded directly into aURL. I should be able to get
  // rid of this method completely.

  nsresult rv = NS_OK;
  m_readCount = aReadCount;
  nsCOMPtr <nsIFile> file;

  rv = GetFileFromURL(aURL, getter_AddRefs(file));
  NS_ENSURE_SUCCESS(rv, rv);

  nsCOMPtr<nsIInputStream> stream;
  rv = NS_NewLocalFileInputStream(getter_AddRefs(stream), file);
  if (NS_FAILED(rv)) return rv;

  // create input stream transport
  nsCOMPtr<nsIStreamTransportService> sts =
      do_GetService(NS_STREAMTRANSPORTSERVICE_CONTRACTID, &rv);
  if (NS_FAILED(rv)) return rv;

  rv = sts->CreateInputTransport(stream, nsInt64(aStartPosition),
                                 nsInt64(aReadCount), PR_TRUE,
                                 getter_AddRefs(m_transport));

  m_socketIsOpen = PR_FALSE;
	return rv;
}

nsresult nsMsgProtocol::SetupTransportState()
{
  if (!m_socketIsOpen && m_transport)
  {
    nsresult rv;
    
    // open buffered, blocking output stream
    rv = m_transport->OpenOutputStream(nsITransport::OPEN_BLOCKING, 0, 0, getter_AddRefs(m_outputStream));
    if (NS_FAILED(rv)) return rv;
    // we want to open the stream 
  } // if m_transport
  
  return NS_OK;
}

nsresult nsMsgProtocol::CloseSocket()
{
  nsresult rv = NS_OK;
  // release all of our socket state
  m_socketIsOpen = PR_FALSE;
  m_inputStream = nsnull;
  m_outputStream = nsnull;
  if (m_transport) {
    nsCOMPtr<nsISocketTransport> strans = do_QueryInterface(m_transport);
    if (strans) {
      strans->SetSecurityCallbacks(nsnull);
      strans->SetEventSink(nsnull, nsnull); // break cyclic reference!
    }
  }
  // we need to call Cancel so that we remove the socket transport from the mActiveTransportList.  see bug #30648
  if (m_request) {
    rv = m_request->Cancel(NS_BINDING_ABORTED);
  }
  m_request = 0;
  if (m_transport) {
    m_transport->Close(NS_BINDING_ABORTED);
    m_transport = 0;
  }
  
  return rv;
}

/*
* Writes the data contained in dataBuffer into the current output stream. It also informs
* the transport layer that this data is now available for transmission.
* Returns a positive number for success, 0 for failure (not all the bytes were written to the
* stream, etc). We need to make another pass through this file to install an error system (mscott)
*
* No logging is done in the base implementation, so aSuppressLogging is ignored.
*/

PRInt32 nsMsgProtocol::SendData(nsIURI * aURL, const char * dataBuffer, PRBool aSuppressLogging)
{
  PRUint32 writeCount = 0; 
  PRInt32 status = 0; 
  
  //	NS_PRECONDITION(m_outputStream, "oops....we don't have an output stream...how did that happen?");
  if (dataBuffer && m_outputStream)
    status = m_outputStream->Write(dataBuffer, PL_strlen(dataBuffer), &writeCount);
  
  return status;
}

// Whenever data arrives from the connection, core netlib notifices the protocol by calling
// OnDataAvailable. We then read and process the incoming data from the input stream. 
NS_IMETHODIMP nsMsgProtocol::OnDataAvailable(nsIRequest *request, nsISupports *ctxt, nsIInputStream *inStr, PRUint32 sourceOffset, PRUint32 count)
{
  // right now, this really just means turn around and churn through the state machine
  nsCOMPtr<nsIURI> uri = do_QueryInterface(ctxt);
  return ProcessProtocolState(uri, inStr, sourceOffset, count);
}

NS_IMETHODIMP nsMsgProtocol::OnStartRequest(nsIRequest *request, nsISupports *ctxt)
{
  nsresult rv = NS_OK;
  nsCOMPtr <nsIMsgMailNewsUrl> aMsgUrl = do_QueryInterface(ctxt, &rv);
  if (NS_SUCCEEDED(rv) && aMsgUrl)
  {
    rv = aMsgUrl->SetUrlState(PR_TRUE, NS_OK);
    if (m_loadGroup)
      m_loadGroup->AddRequest(NS_STATIC_CAST(nsIRequest *, this), nsnull /* context isupports */);
  }
  
  // if we are set up as a channel, we should notify our channel listener that we are starting...
  // so pass in ourself as the channel and not the underlying socket or file channel the protocol
  // happens to be using
  if (!mSuppressListenerNotifications && m_channelListener)
  {
    if (!m_channelContext)
      m_channelContext = do_QueryInterface(ctxt);
    rv = m_channelListener->OnStartRequest(this, m_channelContext);
  }
  
  nsCOMPtr<nsISocketTransport> strans = do_QueryInterface(m_transport);

  if (strans)
    strans->SetTimeout(nsISocketTransport::TIMEOUT_READ_WRITE, gSocketTimeout);  

  NS_ENSURE_SUCCESS(rv, rv);
  return rv;
}

// stop binding is a "notification" informing us that the stream associated with aURL is going away. 
NS_IMETHODIMP nsMsgProtocol::OnStopRequest(nsIRequest *request, nsISupports *ctxt, nsresult aStatus)
{
  nsresult rv = NS_OK;

  // if we are set up as a channel, we should notify our channel listener that we are starting...
  // so pass in ourself as the channel and not the underlying socket or file channel the protocol
  // happens to be using
  if (!mSuppressListenerNotifications && m_channelListener)
    rv = m_channelListener->OnStopRequest(this, m_channelContext, aStatus);

  nsCOMPtr <nsIMsgMailNewsUrl> msgUrl = do_QueryInterface(ctxt, &rv);
  if (NS_SUCCEEDED(rv) && msgUrl)
  {
    rv = msgUrl->SetUrlState(PR_FALSE, aStatus);
    if (m_loadGroup)
      m_loadGroup->RemoveRequest(NS_STATIC_CAST(nsIRequest *, this), nsnull, aStatus);
    
    // !NS_BINDING_ABORTED because we don't want to see an alert if the user 
    // cancelled the operation.  also, we'll get here because we call Cancel()
    // to force removal of the nsSocketTransport.  see CloseSocket()
    // bugs #30775 and #30648 relate to this
    if (NS_FAILED(aStatus) && (aStatus != NS_BINDING_ABORTED)) 
    {
      nsCOMPtr<nsIPrompt> msgPrompt;
      GetPromptDialogFromUrl(msgUrl , getter_AddRefs(msgPrompt));
      NS_ENSURE_TRUE(msgPrompt, NS_ERROR_FAILURE);

      PRInt32 errorID;
      switch (aStatus) 
      {
          case NS_ERROR_UNKNOWN_HOST:
          case NS_ERROR_UNKNOWN_PROXY_HOST:
             errorID = UNKNOWN_HOST_ERROR;
             break;
          case NS_ERROR_CONNECTION_REFUSED:
          case NS_ERROR_PROXY_CONNECTION_REFUSED:
             errorID = CONNECTION_REFUSED_ERROR;
             break;
          case NS_ERROR_NET_TIMEOUT:
             errorID = NET_TIMEOUT_ERROR;
             break;
          default: 
             errorID = UNKNOWN_ERROR;
             break;
      }
      
      NS_ASSERTION(errorID != UNKNOWN_ERROR, "unknown error, but don't alert user.");
      if (errorID != UNKNOWN_ERROR) 
      {
        PRUnichar *errorMsg = FormatStringWithHostNameByID(errorID, msgUrl);
        if (errorMsg == nsnull) 
        {
          nsAutoString resultString(NS_LITERAL_STRING("[StringID "));
          resultString.AppendInt(errorID);
          resultString.AppendLiteral("?]");
          errorMsg = ToNewUnicode(resultString);
        }
        rv = msgPrompt->Alert(nsnull, errorMsg);
        nsMemory::Free(errorMsg);
      }
    } // if we got an error code
  } // if we have a mailnews url.

  // Drop notification callbacks to prevent cycles.
  mCallbacks = 0;
  mProgressEventSink = 0;
  // Call CloseSocket(), in case we got here because the server dropped the 
  // connection while reading, and we never get a chance to get back into 
  // the protocol state machine via OnDataAvailable. 
  if (m_socketIsOpen)
    CloseSocket();

  return rv;
}

nsresult nsMsgProtocol::GetPromptDialogFromUrl(nsIMsgMailNewsUrl * aMsgUrl, nsIPrompt ** aPromptDialog)
{
  // get the nsIPrompt interface from the message window associated wit this url.
  nsCOMPtr<nsIMsgWindow> msgWindow;
  aMsgUrl->GetMsgWindow(getter_AddRefs(msgWindow));
  NS_ENSURE_TRUE(msgWindow, NS_ERROR_FAILURE);

  msgWindow->GetPromptDialog(aPromptDialog);

  NS_ENSURE_TRUE(*aPromptDialog, NS_ERROR_FAILURE);

  return NS_OK;
}

nsresult nsMsgProtocol::LoadUrl(nsIURI * aURL, nsISupports * aConsumer)
{
  // okay now kick us off to the next state...
  // our first state is a process state so drive the state machine...
  nsresult rv = NS_OK;
  nsCOMPtr <nsIMsgMailNewsUrl> aMsgUrl = do_QueryInterface(aURL, &rv);
  
  if (NS_SUCCEEDED(rv) && aMsgUrl)
  {
    PRBool msgIsInLocalCache;
    aMsgUrl->GetMsgIsInLocalCache(&msgIsInLocalCache);
    
    rv = aMsgUrl->SetUrlState(PR_TRUE, NS_OK); // set the url as a url currently being run...
    
    // if the url is given a stream consumer then we should use it to forward calls to...
    if (!m_channelListener && aConsumer) // if we don't have a registered listener already
    {
      m_channelListener = do_QueryInterface(aConsumer);
      if (!m_channelContext)
        m_channelContext = do_QueryInterface(aURL);
    }
    
    if (!m_socketIsOpen)
    {
      nsCOMPtr<nsISupports> urlSupports = do_QueryInterface(aURL);
      if (m_transport)
      {
        // don't open the input stream more than once
        if (!m_inputStream)
        {
          // open buffered, asynchronous input stream
          rv = m_transport->OpenInputStream(0, 0, 0, getter_AddRefs(m_inputStream));
          if (NS_FAILED(rv)) return rv;
        }
        
        nsCOMPtr<nsIInputStreamPump> pump;
        rv = NS_NewInputStreamPump(getter_AddRefs(pump),
          m_inputStream, nsInt64(-1), m_readCount);
        if (NS_FAILED(rv)) return rv;
        
        m_request = pump; // keep a reference to the pump so we can cancel it
        
        // put us in a state where we are always notified of incoming data
        rv = pump->AsyncRead(this, urlSupports);
        NS_ASSERTION(NS_SUCCEEDED(rv), "AsyncRead failed");
        m_socketIsOpen = PR_TRUE; // mark the channel as open
      }
    } // if we got an event queue service
    else if (!msgIsInLocalCache) // the connection is already open so we should begin processing our new url...
      rv = ProcessProtocolState(aURL, nsnull, 0, 0); 
  }
  
  return rv;
}

///////////////////////////////////////////////////////////////////////
// The rest of this file is mostly nsIChannel mumbo jumbo stuff
///////////////////////////////////////////////////////////////////////

nsresult nsMsgProtocol::SetUrl(nsIURI * aURL)
{
  m_url = aURL;
  return NS_OK;
}

NS_IMETHODIMP nsMsgProtocol::SetLoadGroup(nsILoadGroup * aLoadGroup)
{
  m_loadGroup = aLoadGroup;
  return NS_OK;
}

NS_IMETHODIMP nsMsgProtocol::GetOriginalURI(nsIURI* *aURI)
{
    *aURI = m_originalUrl ? m_originalUrl : m_url;
    NS_IF_ADDREF(*aURI);
    return NS_OK;
}
 
NS_IMETHODIMP nsMsgProtocol::SetOriginalURI(nsIURI* aURI)
{
    m_originalUrl = aURI;
    return NS_OK;
}
 
NS_IMETHODIMP nsMsgProtocol::GetURI(nsIURI* *aURI)
{
    *aURI = m_url;
    NS_IF_ADDREF(*aURI);
    return NS_OK;
}
 
NS_IMETHODIMP nsMsgProtocol::Open(nsIInputStream **_retval)
{
  NS_NOTREACHED("Open");
  return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP nsMsgProtocol::AsyncOpen(nsIStreamListener *listener, nsISupports *ctxt)
{
    PRInt32 port;
    nsresult rv = m_url->GetPort(&port);
    if (NS_FAILED(rv))
        return rv;
    
    nsCAutoString scheme;
    rv = m_url->GetScheme(scheme);
    if (NS_FAILED(rv))
        return rv;
    

    rv = NS_CheckPortSafety(port, scheme.get());
    if (NS_FAILED(rv))
        return rv;

    // set the stream listener and then load the url
    m_channelContext = ctxt;
    m_channelListener = listener;
    return LoadUrl(m_url, nsnull);
}

NS_IMETHODIMP nsMsgProtocol::GetLoadFlags(nsLoadFlags *aLoadFlags)
{
    *aLoadFlags = mLoadFlags;
    return NS_OK;
}

NS_IMETHODIMP nsMsgProtocol::SetLoadFlags(nsLoadFlags aLoadFlags)
{
  mLoadFlags = aLoadFlags;
  return NS_OK;       // don't fail when trying to set this
}

NS_IMETHODIMP nsMsgProtocol::GetContentType(nsACString &aContentType)
{
  // as url dispatching matures, we'll be intelligent and actually start
  // opening the url before specifying the content type. This will allow
  // us to optimize the case where the message url actual refers to  
  // a part in the message that has a content type that is not message/rfc822

  if (m_ContentType.IsEmpty())
    aContentType.AssignLiteral("message/rfc822");
  else
    aContentType = m_ContentType;
  return NS_OK;
}

NS_IMETHODIMP nsMsgProtocol::SetContentType(const nsACString &aContentType)
{
    nsCAutoString charset;
    return NS_ParseContentType(aContentType, m_ContentType, charset);
}

NS_IMETHODIMP nsMsgProtocol::GetContentCharset(nsACString &aContentCharset)
{
  aContentCharset.Truncate();
  return NS_OK;
}

NS_IMETHODIMP nsMsgProtocol::SetContentCharset(const nsACString &aContentCharset)
{
  NS_WARNING("nsMsgProtocol::SetContentCharset() not implemented");
  return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP nsMsgProtocol::GetContentLength(PRInt32 * aContentLength)
{
  *aContentLength = -1;
  return NS_OK;
}

NS_IMETHODIMP nsMsgProtocol::GetSecurityInfo(nsISupports * *aSecurityInfo)
{
  *aSecurityInfo = nsnull;
  return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP nsMsgProtocol::GetName(nsACString &aName)
{
  return NS_ERROR_NOT_IMPLEMENTED;
}


NS_IMETHODIMP
nsMsgProtocol::SetContentLength(PRInt32 aContentLength)
{
  return NS_ERROR_NOT_IMPLEMENTED;
}
 
NS_IMETHODIMP nsMsgProtocol::GetOwner(nsISupports * *aPrincipal)
{
  *aPrincipal = mOwner;
  NS_IF_ADDREF(*aPrincipal);
  return NS_OK;
}

NS_IMETHODIMP nsMsgProtocol::SetOwner(nsISupports * aPrincipal)
{
  mOwner = aPrincipal;
  return NS_OK;
}

NS_IMETHODIMP nsMsgProtocol::GetLoadGroup(nsILoadGroup * *aLoadGroup)
{
  *aLoadGroup = m_loadGroup;
  NS_IF_ADDREF(*aLoadGroup);
  return NS_OK;
}

NS_IMETHODIMP
nsMsgProtocol::GetNotificationCallbacks(nsIInterfaceRequestor* *aNotificationCallbacks)
{
  *aNotificationCallbacks = mCallbacks.get();
  NS_IF_ADDREF(*aNotificationCallbacks);
  return NS_OK;
}

NS_IMETHODIMP
nsMsgProtocol::SetNotificationCallbacks(nsIInterfaceRequestor* aNotificationCallbacks)
{
  mCallbacks = aNotificationCallbacks;
  return NS_OK;
}

NS_IMETHODIMP
nsMsgProtocol::OnTransportStatus(nsITransport *transport, nsresult status,
                                 PRUint64 progress, PRUint64 progressMax)
{
  if ((mLoadFlags & LOAD_BACKGROUND) || !m_url)
    return NS_OK;

  // these transport events should not generate any status messages
  if (status == nsISocketTransport::STATUS_RECEIVING_FROM ||
      status == nsISocketTransport::STATUS_SENDING_TO)
    return NS_OK;

  if (!mProgressEventSink)
  {
    NS_QueryNotificationCallbacks(mCallbacks, m_loadGroup, mProgressEventSink);
    if (!mProgressEventSink)
      return NS_OK;
  }

  nsCAutoString host;
  m_url->GetHost(host);

  nsCOMPtr<nsIMsgMailNewsUrl> mailnewsUrl = do_QueryInterface(m_url);
  if (mailnewsUrl) 
  {
    nsCOMPtr<nsIMsgIncomingServer> server;
    mailnewsUrl->GetServer(getter_AddRefs(server));
    if (server)
    {
      char *realHostName = nsnull;
      server->GetRealHostName(&realHostName);
      if (realHostName)
        host.Adopt(realHostName);
    }
  }
  mProgressEventSink->OnStatus(this, nsnull, status,
                               NS_ConvertUTF8toUTF16(host).get());

  return NS_OK;
}

////////////////////////////////////////////////////////////////////////////////
// From nsIRequest
////////////////////////////////////////////////////////////////////////////////

NS_IMETHODIMP nsMsgProtocol::IsPending(PRBool *result)
{
    *result = m_channelListener != nsnull;
    return NS_OK; 
}

NS_IMETHODIMP nsMsgProtocol::GetStatus(nsresult *status)
{
  if (m_request)
    return m_request->GetStatus(status);

  *status = NS_OK;
  return *status;
}

NS_IMETHODIMP nsMsgProtocol::Cancel(nsresult status)
{
  NS_ASSERTION(m_request,"no channel");
  if (!m_request)
    return NS_ERROR_FAILURE;
  
  return m_request->Cancel(status);
}

NS_IMETHODIMP nsMsgProtocol::Suspend()
{
  if (m_request)
    return m_request->Suspend();

  NS_WARNING("no request to suspend");
  return NS_ERROR_NOT_AVAILABLE;
}

NS_IMETHODIMP nsMsgProtocol::Resume()
{
  if (m_request)
    return m_request->Resume();

  NS_WARNING("no request to resume");
  return NS_ERROR_NOT_AVAILABLE;
}

nsresult nsMsgProtocol::PostMessage(nsIURI* url, nsIFileSpec *fileSpec)
{
    if (!url || !fileSpec) return NS_ERROR_NULL_POINTER;

#define POST_DATA_BUFFER_SIZE 2048

    // mscott -- this function should be re-written to use the file url code
    // so it can be asynch
    nsFileSpec afileSpec;
    fileSpec->GetFileSpec(&afileSpec);
    nsInputFileStream * fileStream = new nsInputFileStream(afileSpec,
                                                           PR_RDONLY, 00700);
    if (fileStream && fileStream->is_open())
    {
        PRInt32 amtInBuffer = 0; 
        PRBool lastLineWasComplete = PR_TRUE;
        
        PRBool quoteLines = PR_TRUE;  // it is always true but I'd like to
                                      // generalize this function and then it
                                      // might not be 
        char buffer[POST_DATA_BUFFER_SIZE];
        
        if (quoteLines /* || add_crlf_to_line_endings */)
        {
            char *line;
            char * b = buffer;
            PRInt32 bsize = POST_DATA_BUFFER_SIZE;
            amtInBuffer =  0;
            do {
                lastLineWasComplete = PR_TRUE;
                PRInt32 L = 0;
                if (fileStream->eof())
                {
                    line = nsnull;
                    break;
                }
                
                if (!fileStream->readline(b, bsize-5)) 
                    lastLineWasComplete = PR_FALSE;
                line = b;
                
                L = PL_strlen(line);
                
                /* escape periods only if quote_lines_p is set
                 */
                if (quoteLines && lastLineWasComplete && line[0] == '.')
                {
                    /* This line begins with "." so we need to quote it
                       by adding another "." to the beginning of the line.
                       */
                    PRInt32 i;
                    line[L+1] = 0;
                    for (i = L; i > 0; i--)
                        line[i] = line[i-1];
                    L++;
                }
                
                if (!lastLineWasComplete || (L > 1 && line[L-2] == nsCRT::CR &&
                                             line[L-1] == nsCRT::LF))
                {
                    /* already ok */
                }
                else if(L > 0 /* && (line[L-1] == nsCRT::LF || line[L-1] == nsCRT::CR) */)
                {
                    /* only add the crlf if required
                     * we still need to do all the
                     * if comparisons here to know
                     * if the line was complete
                     */
                    if(/* add_crlf_to_line_endings */ PR_TRUE)
                    {
                        /* Change newline to CRLF. */
                          line[L++] = nsCRT::CR;
                          line[L++] = nsCRT::LF;
                          line[L] = 0;
                    }
                }
                else if (L == 0 && !fileStream->eof()
                         /* && add_crlf_to_line_endings */)
                {
                    // jt ** empty line; output CRLF
                    line[L++] = nsCRT::CR;
                    line[L++] = nsCRT::LF;
                    line[L] = 0;
                }
                
                bsize -= L;
                b += L;
                amtInBuffer += L;
                // test hack by mscott. If our buffer is almost full, then
                // send it off & reset ourselves 
                // to make more room.
                if (bsize < 100) // i chose 100 arbitrarily.
                {
                    if (*buffer)
                        SendData(url, buffer);
                    buffer[0] = '\0';
                    b = buffer; // reset buffer
                    bsize = POST_DATA_BUFFER_SIZE;
                }
                
            } while (line /* && bsize > 100 */);
        }
        
        SendData(url, buffer); 
        delete fileStream;
    }
    return NS_OK;
}

nsresult nsMsgProtocol::DoGSSAPIStep1(const char *service, const char *username, nsCString &response)
{
    nsresult rv;

    // if this fails, then it means that we cannot do GSSAPI SASL.
    m_authModule = do_CreateInstance(NS_AUTH_MODULE_CONTRACTID_PREFIX "sasl-gssapi", &rv);
    NS_ENSURE_SUCCESS(rv,rv);

    m_authModule->Init(service, nsIAuthModule::REQ_DEFAULT, nsnull, NS_ConvertUTF8toUTF16(username).get(), nsnull);

    void *outBuf;
    PRUint32 outBufLen;
    rv = m_authModule->GetNextToken((void *)nsnull, 0, &outBuf, &outBufLen);
    if (NS_SUCCEEDED(rv) && outBuf)
    {
        char *base64Str = PL_Base64Encode((char *)outBuf, outBufLen, nsnull);
        if (base64Str)
            response.Adopt(base64Str);
        else 
            rv = NS_ERROR_OUT_OF_MEMORY;
        nsMemory::Free(outBuf);
    }

    return rv;
}

nsresult nsMsgProtocol::DoGSSAPIStep2(nsCString &commandResponse, nsCString &response)
{
    nsresult rv;
    void *inBuf, *outBuf;
    PRUint32 inBufLen, outBufLen;
    PRUint32 len = commandResponse.Length();

    // Cyrus SASL may send us zero length tokens (grrrr)
    if (len > 0) {
        // decode into the input secbuffer
        inBufLen = (len * 3)/4;      // sufficient size (see plbase64.h)
        inBuf = nsMemory::Alloc(inBufLen);
        if (!inBuf)
            return NS_ERROR_OUT_OF_MEMORY;

        // strip off any padding (see bug 230351)
        const char *challenge = commandResponse.get();
        while (challenge[len - 1] == '=')
            len--;

        // We need to know the exact length of the decoded string to give to
        // the GSSAPI libraries. But NSPR's base64 routine doesn't seem capable
        // of telling us that. So, we figure it out for ourselves.

        // For every 4 characters, add 3 to the destination
        // If there are 3 remaining, add 2
        // If there are 2 remaining, add 1
        // 1 remaining is an error
        inBufLen = (len / 4)*3 + ((len % 4 == 3)?2:0) + ((len % 4 == 2)?1:0);

        rv = (PL_Base64Decode(challenge, len, (char *)inBuf))
             ? m_authModule->GetNextToken(inBuf, inBufLen, &outBuf, &outBufLen)
             : NS_ERROR_FAILURE;

        nsMemory::Free(inBuf);
    }
    else 
    {
        rv = m_authModule->GetNextToken(NULL, 0, &outBuf, &outBufLen);
    }
    if (NS_SUCCEEDED(rv)) 
    {
        // And in return, we may need to send Cyrus zero length tokens back
        if (outBuf) 
        {
            char *base64Str = PL_Base64Encode((char *)outBuf, outBufLen, nsnull);
            if (base64Str)
                response.Adopt(base64Str);
            else 
                rv = NS_ERROR_OUT_OF_MEMORY;
        }
        else
            response.Adopt((char *)nsMemory::Clone("",1));
    }

    return rv;
}

nsresult nsMsgProtocol::DoNtlmStep1(const char *username, const char *password, nsCString &response)
{
    nsresult rv;

    m_authModule = do_CreateInstance(NS_AUTH_MODULE_CONTRACTID_PREFIX "ntlm", &rv);
    // if this fails, then it means that we cannot do NTLM auth.
    if (NS_FAILED(rv) || !m_authModule)
        return rv;

    m_authModule->Init(nsnull, 0, nsnull, NS_ConvertUTF8toUTF16(username).get(),
                       NS_ConvertUTF8toUTF16(password).get());

    void *outBuf;
    PRUint32 outBufLen;
    rv = m_authModule->GetNextToken((void *)nsnull, 0, &outBuf, &outBufLen);
    if (NS_SUCCEEDED(rv) && outBuf)
    {
        char *base64Str = PL_Base64Encode((char *)outBuf, outBufLen, nsnull);
        if (base64Str)
          response.Adopt(base64Str);
        else 
          rv = NS_ERROR_OUT_OF_MEMORY;
        nsMemory::Free(outBuf);
    }

    return rv;
}

nsresult nsMsgProtocol::DoNtlmStep2(nsCString &commandResponse, nsCString &response)
{
    nsresult rv;
    void *inBuf, *outBuf;
    PRUint32 inBufLen, outBufLen;
    PRUint32 len = commandResponse.Length();

    // decode into the input secbuffer
    inBufLen = (len * 3)/4;      // sufficient size (see plbase64.h)
    inBuf = nsMemory::Alloc(inBufLen);
    if (!inBuf)
        return NS_ERROR_OUT_OF_MEMORY;

    // strip off any padding (see bug 230351)
    const char *challenge = commandResponse.get();
    while (challenge[len - 1] == '=')
        len--;

    rv = (PL_Base64Decode(challenge, len, (char *)inBuf))
         ? m_authModule->GetNextToken(inBuf, inBufLen, &outBuf, &outBufLen)
         : NS_ERROR_FAILURE;

    nsMemory::Free(inBuf);
    if (NS_SUCCEEDED(rv) && outBuf)
    {
        char *base64Str = PL_Base64Encode((char *)outBuf, outBufLen, nsnull);
        if (base64Str)
          response.Adopt(base64Str);
        else 
          rv = NS_ERROR_OUT_OF_MEMORY;
    }

    if (NS_FAILED(rv))
        response = "*";

    return rv;
}

/////////////////////////////////////////////////////////////////////
// nsMsgAsyncWriteProtocol subclass and related helper classes
/////////////////////////////////////////////////////////////////////

class nsMsgProtocolStreamProvider : public nsIOutputStreamCallback 
{
public:
    NS_DECL_ISUPPORTS

    nsMsgProtocolStreamProvider() { }
    virtual ~nsMsgProtocolStreamProvider() {}

    void Init(nsMsgAsyncWriteProtocol *aProtInstance, nsIInputStream *aInputStream)
    {
      mMsgProtocol = aProtInstance;
      mInStream = aInputStream;
    }

    //
    // nsIOutputStreamCallback implementation ...
    //
    NS_IMETHODIMP OnOutputStreamReady(nsIAsyncOutputStream *aOutStream)
    { 
        NS_ASSERTION(mInStream, "not initialized");

        nsresult rv;
        PRUint32 avail;

        // Write whatever is available in the pipe. If the pipe is empty, then
        // return NS_BASE_STREAM_WOULD_BLOCK; we will resume the write when there
        // is more data.

        rv = mInStream->Available(&avail);
        if (NS_FAILED(rv)) return rv;

        if (avail == 0) 
        {
          // ok, stop writing...
          mMsgProtocol->mSuspendedWrite = PR_TRUE;
          return NS_OK;
        }

        PRUint32 bytesWritten;
        rv =  aOutStream->WriteFrom(mInStream, PR_MIN(avail, 4096), &bytesWritten);
        // if were full at the time, the input stream may be backed up and we need to read any remains from the last ODA call
        // before we'll get more ODA calls
        if (mMsgProtocol->mSuspendedRead)
          mMsgProtocol->UnblockPostReader();

        mMsgProtocol->UpdateProgress(bytesWritten);

        // try to write again...
        if (NS_SUCCEEDED(rv))
          rv = aOutStream->AsyncWait(this, 0, 0, mMsgProtocol->mProviderThread);

        NS_ASSERTION(NS_SUCCEEDED(rv) || rv == NS_BINDING_ABORTED, "unexpected error writing stream");
        return NS_OK;
    }

    
protected:
  nsMsgAsyncWriteProtocol * mMsgProtocol;
  nsCOMPtr<nsIInputStream>  mInStream;
};

// XXX this probably doesn't need to be threadsafe
NS_IMPL_THREADSAFE_ISUPPORTS1(nsMsgProtocolStreamProvider,
                              nsIOutputStreamCallback)

class nsMsgFilePostHelper : public nsIStreamListener 
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIREQUESTOBSERVER
  NS_DECL_NSISTREAMLISTENER

  nsMsgFilePostHelper() { mSuspendedPostFileRead = PR_FALSE;}
  nsresult Init(nsIOutputStream * aOutStream, nsMsgAsyncWriteProtocol * aProtInstance, nsIFile *aFileToPost);
  virtual ~nsMsgFilePostHelper() {}
  nsCOMPtr<nsIRequest> mPostFileRequest;
  PRBool mSuspendedPostFileRead;
  void CloseSocket() { mProtInstance = nsnull; }
protected:
  nsCOMPtr<nsIOutputStream> mOutStream;
  nsMsgAsyncWriteProtocol * mProtInstance;
};

NS_IMPL_THREADSAFE_ADDREF(nsMsgFilePostHelper)
NS_IMPL_THREADSAFE_RELEASE(nsMsgFilePostHelper)

NS_INTERFACE_MAP_BEGIN(nsMsgFilePostHelper)
  NS_INTERFACE_MAP_ENTRY(nsIStreamListener)
  NS_INTERFACE_MAP_ENTRY(nsIRequestObserver)
NS_INTERFACE_MAP_END_THREADSAFE

nsresult nsMsgFilePostHelper::Init(nsIOutputStream * aOutStream, nsMsgAsyncWriteProtocol * aProtInstance, nsIFile *aFileToPost)
{
  nsresult rv = NS_OK;
  mOutStream = aOutStream;
  mProtInstance = aProtInstance; // mscott work out ref counting issue

  nsCOMPtr<nsIInputStream> stream;
  rv = NS_NewLocalFileInputStream(getter_AddRefs(stream), aFileToPost);
  if (NS_FAILED(rv)) return rv;

  nsCOMPtr<nsIInputStreamPump> pump;
  rv = NS_NewInputStreamPump(getter_AddRefs(pump), stream);
  if (NS_FAILED(rv)) return rv;

  rv = pump->AsyncRead(this, nsnull);
  if (NS_FAILED(rv)) return rv;

  mPostFileRequest = pump;
  return NS_OK;
}

NS_IMETHODIMP nsMsgFilePostHelper::OnStartRequest(nsIRequest * aChannel, nsISupports *ctxt)
{
  return NS_OK;
}

NS_IMETHODIMP nsMsgFilePostHelper::OnStopRequest(nsIRequest * aChannel, nsISupports *ctxt, nsresult aStatus)
{
  if (!mProtInstance) return NS_OK; 

  if (!mSuspendedPostFileRead)
    mProtInstance->PostDataFinished();

  mSuspendedPostFileRead = PR_FALSE;
  mProtInstance->mFilePostHelper = nsnull;
  return NS_OK;
}

NS_IMETHODIMP nsMsgFilePostHelper::OnDataAvailable(nsIRequest * /* aChannel */, nsISupports *ctxt, nsIInputStream *inStr, PRUint32 sourceOffset, PRUint32 count)
{
  if (!mProtInstance) return NS_OK; 

  if (mSuspendedPostFileRead)
  {
    mProtInstance->UpdateSuspendedReadBytes(count, mProtInstance->mInsertPeriodRequired);
    return NS_OK;
  }

  mProtInstance->ProcessIncomingPostData(inStr, count);

  if (mProtInstance->mSuspendedWrite) 
  {
    // if we got here then we had suspended the write 'cause we didn't have anymore
    // data to write (i.e. the pipe went empty). So resume the channel to kick
    // things off again.
    mProtInstance->mSuspendedWrite = PR_FALSE;
    mProtInstance->mAsyncOutStream->AsyncWait(mProtInstance->mProvider, 0, 0,
                                              mProtInstance->mProviderThread);
  } 

  return NS_OK;
}

NS_IMPL_ADDREF_INHERITED(nsMsgAsyncWriteProtocol, nsMsgProtocol)
NS_IMPL_RELEASE_INHERITED(nsMsgAsyncWriteProtocol, nsMsgProtocol)

NS_INTERFACE_MAP_BEGIN(nsMsgAsyncWriteProtocol)
NS_INTERFACE_MAP_END_INHERITING(nsMsgProtocol)

nsMsgAsyncWriteProtocol::nsMsgAsyncWriteProtocol(nsIURI * aURL) : nsMsgProtocol(aURL)
{
  mSuspendedWrite = PR_FALSE;
  mSuspendedReadBytes = 0;
  mSuspendedRead = PR_FALSE;
  mInsertPeriodRequired = PR_FALSE;
  mGenerateProgressNotifications = PR_FALSE;
  mSuspendedReadBytesPostPeriod = 0;
  mFilePostHelper = nsnull;
}

nsMsgAsyncWriteProtocol::~nsMsgAsyncWriteProtocol()
{}

NS_IMETHODIMP nsMsgAsyncWriteProtocol::Cancel(nsresult status)
{
  mGenerateProgressNotifications = PR_FALSE;

  if (m_request)
    m_request->Cancel(status);

  if (mAsyncOutStream)
    mAsyncOutStream->CloseWithStatus(status);

  return NS_OK;
}

nsresult nsMsgAsyncWriteProtocol::PostMessage(nsIURI* url, nsIFileSpec *fileSpec)
{
  // convert the file spec into a nsIFile....
  nsFileSpec spec;
  fileSpec->GetFileSpec(&spec);

  nsCOMPtr<nsILocalFile> file;
  NS_FileSpecToIFile(&spec, getter_AddRefs(file));

  nsCOMPtr<nsIStreamListener> listener;
  NS_NEWXPCOM(listener, nsMsgFilePostHelper);
  if (!listener) return NS_ERROR_OUT_OF_MEMORY;

  // be sure to initialize some state before posting
  mSuspendedReadBytes = 0;
  mNumBytesPosted = 0;
  fileSpec->GetFileSize(&mFilePostSize);
  mSuspendedRead = PR_FALSE;
  mInsertPeriodRequired = PR_FALSE;
  mSuspendedReadBytesPostPeriod = 0;
  mGenerateProgressNotifications = PR_TRUE;

  mFilePostHelper = NS_STATIC_CAST(nsMsgFilePostHelper*,NS_STATIC_CAST(nsIStreamListener*, listener));

  NS_STATIC_CAST(nsMsgFilePostHelper*,NS_STATIC_CAST(nsIStreamListener*, listener))->Init(m_outputStream, this, file);

  return NS_OK;
}

nsresult nsMsgAsyncWriteProtocol::SuspendPostFileRead()
{
#ifdef DEBUG_mscott
  printf("suspending post read during send\n");
#endif
  if (mFilePostHelper && !mFilePostHelper->mSuspendedPostFileRead)
  {
    // uhoh we need to pause reading in the file until we get unblocked...
    mFilePostHelper->mPostFileRequest->Suspend();
    mFilePostHelper->mSuspendedPostFileRead = PR_TRUE;
  }

  return NS_OK;
}

nsresult nsMsgAsyncWriteProtocol::ResumePostFileRead()
{
#ifdef DEBUG_mscott
  printf("resuming post read during send\n");
#endif

  if (mFilePostHelper)
  {
    if (mFilePostHelper->mSuspendedPostFileRead)
    {
      mFilePostHelper->mPostFileRequest->Resume();
      mFilePostHelper->mSuspendedPostFileRead = PR_FALSE;
    }
  }
  else // we must be done with the download so send the '.'
  {
    PostDataFinished(); 
  }
  
  return NS_OK;
}

nsresult nsMsgAsyncWriteProtocol::UpdateSuspendedReadBytes(PRUint32 aNewBytes, PRBool aAddToPostPeriodByteCount)
{
  // depending on our current state, we'll either add aNewBytes to mSuspendedReadBytes
  // or mSuspendedReadBytesAfterPeriod. 

  mSuspendedRead = PR_TRUE;
  if (aAddToPostPeriodByteCount) 
    mSuspendedReadBytesPostPeriod += aNewBytes;
  else
    mSuspendedReadBytes += aNewBytes;

  return NS_OK;
}

nsresult nsMsgAsyncWriteProtocol::PostDataFinished()
{
  SendData(nsnull, "." CRLF);
  mGenerateProgressNotifications = PR_FALSE;
  mPostDataStream = nsnull;
  return NS_OK;
}

nsresult nsMsgAsyncWriteProtocol::ProcessIncomingPostData(nsIInputStream *inStr, PRUint32 count)
{
  if (!m_socketIsOpen) return NS_OK; // kick out if the socket was canceled

  // We need to quote any '.' that occur at the beginning of a line.
  // but I don't want to waste time reading out the data into a buffer and searching
  // let's try to leverage nsIBufferedInputStream and see if we can "peek" into the 
  // current contents for this particular case.

  nsCOMPtr<nsISearchableInputStream> bufferInputStr = do_QueryInterface(inStr);
  NS_ASSERTION(bufferInputStr, "i made a wrong assumption about the type of stream we are getting");
  NS_ASSERTION(mSuspendedReadBytes == 0, "oops, I missed something");

  if (!mPostDataStream) mPostDataStream = inStr;

  if (bufferInputStr)
  { 
    PRUint32 amountWritten;

    while (count > 0)
    {
      PRBool found = PR_FALSE;
      PRUint32 offset = 0;
      bufferInputStr->Search("\012.", PR_TRUE,  &found, &offset); // LF.

      if (!found || offset > count)
      {
        // push this data into the output stream
        m_outputStream->WriteFrom(inStr, count, &amountWritten);
        // store any remains which need read out at a later date
        if (count > amountWritten) // stream will block
        {
          UpdateSuspendedReadBytes(count - amountWritten, PR_FALSE);
          SuspendPostFileRead();
        }
        break;
      }
      else
      {
        // count points to the LF in a LF followed by a '.'
        // go ahead and write up to offset..
        m_outputStream->WriteFrom(inStr, offset + 1, &amountWritten);
        count -= amountWritten;
        if (offset+1 > amountWritten)
        {
          UpdateSuspendedReadBytes(offset+1 - amountWritten, PR_FALSE);
          mInsertPeriodRequired = PR_TRUE;
          UpdateSuspendedReadBytes(count, mInsertPeriodRequired);
          SuspendPostFileRead();
          break;
        }

        // write out the extra '.'
        m_outputStream->Write(".", 1, &amountWritten);
        if (amountWritten != 1)
        {
          mInsertPeriodRequired = PR_TRUE;
          // once we do write out the '.',  if we are now blocked we need to remember the remaining count that comes
          // after the '.' so we can perform processing on that once we become unblocked.
          UpdateSuspendedReadBytes(count, mInsertPeriodRequired);
          SuspendPostFileRead();
          break;
        }
      }
    } // while count > 0
  }

  return NS_OK;
}
nsresult nsMsgAsyncWriteProtocol::UnblockPostReader()
{
  PRUint32 amountWritten = 0;
  
  if (!m_socketIsOpen) return NS_OK; // kick out if the socket was canceled

  if (mSuspendedRead)
  {
    // (1) attempt to write out any remaining read bytes we need in order to unblock the reader
    if (mSuspendedReadBytes > 0 && mPostDataStream)
    {
      PRUint32 avail = 0;
      mPostDataStream->Available(&avail);

      m_outputStream->WriteFrom(mPostDataStream, PR_MIN(avail, mSuspendedReadBytes), &amountWritten);
      // hmm sometimes my mSuspendedReadBytes is getting out of whack...so for now, reset it
      // if necessary.
      if (mSuspendedReadBytes > avail)
        mSuspendedReadBytes = avail;

      if (mSuspendedReadBytes > amountWritten)
        mSuspendedReadBytes -= amountWritten;
      else
        mSuspendedReadBytes = 0;
    }

    // (2) if we are now unblocked, and we need to insert a '.' then do so now...
    if (mInsertPeriodRequired && mSuspendedReadBytes == 0)
    {
      amountWritten = 0;
      m_outputStream->Write(".", 1, &amountWritten);
      if (amountWritten == 1) // if we succeeded then clear pending '.' flag
        mInsertPeriodRequired = PR_FALSE;
    }

    // (3) if we inserted a '.' and we still have bytes after the '.' which need processed before the stream is unblocked
    // then fake an ODA call to handle this now...
    if (!mInsertPeriodRequired && mSuspendedReadBytesPostPeriod > 0)
    {
      // these bytes actually need processed for extra '.''s.....
      PRUint32 postbytes = mSuspendedReadBytesPostPeriod;
      mSuspendedReadBytesPostPeriod = 0;
      ProcessIncomingPostData(mPostDataStream, postbytes);      
    }

    // (4) determine if we are out of the suspended read state...
    if (mSuspendedReadBytes == 0 && !mInsertPeriodRequired && mSuspendedReadBytesPostPeriod == 0)
    {
      mSuspendedRead = PR_FALSE;
      ResumePostFileRead();
    }

  } // if we are in the suspended read state

  return NS_OK;
}

nsresult nsMsgAsyncWriteProtocol::SetupTransportState()
{
  nsresult rv = NS_OK;
  
  if (!m_outputStream && m_transport)
  {
    // first create a pipe which we'll use to write the data we want to send
    // into. 
    rv = NS_NewPipe(getter_AddRefs(mInStream), getter_AddRefs(m_outputStream),
      1024,  // segmentSize
      1024*8, // maxSize
      PR_TRUE, 
      PR_TRUE);
    
    rv = NS_GetCurrentThread(getter_AddRefs(mProviderThread));
    if (NS_FAILED(rv)) return rv;
    
    nsMsgProtocolStreamProvider *provider;
    NS_NEWXPCOM(provider, nsMsgProtocolStreamProvider);
    if (!provider) return NS_ERROR_OUT_OF_MEMORY;
    
    provider->Init(this, mInStream);
    mProvider = provider; // ADDREF
    
    nsCOMPtr<nsIOutputStream> stream;
    rv = m_transport->OpenOutputStream(0, 0, 0, getter_AddRefs(stream));
    if (NS_FAILED(rv)) return rv;
    
    mAsyncOutStream = do_QueryInterface(stream, &rv);
    if (NS_FAILED(rv)) return rv;
    
    // wait for the output stream to become writable
    rv = mAsyncOutStream->AsyncWait(mProvider, 0, 0, mProviderThread);
	} // if m_transport

	return rv;
}

nsresult nsMsgAsyncWriteProtocol::CloseSocket()
{
  nsresult rv = NS_OK;
  if (mAsyncOutStream)
    mAsyncOutStream->CloseWithStatus(NS_BINDING_ABORTED);

  nsMsgProtocol::CloseSocket(); 

  if (mFilePostHelper)
  {
    mFilePostHelper->CloseSocket();
    mFilePostHelper = nsnull;
  }

  mAsyncOutStream = 0;
  mProvider = 0;
  mProviderThread = 0;
	return rv;
}

void nsMsgAsyncWriteProtocol::UpdateProgress(PRUint32 aNewBytes)
{
  if (!mGenerateProgressNotifications) return;

  mNumBytesPosted += aNewBytes;
  if (mFilePostSize > 0)
  {
    nsCOMPtr <nsIMsgMailNewsUrl> mailUrl = do_QueryInterface(m_url);
    if (!mailUrl) return;

    nsCOMPtr<nsIMsgStatusFeedback> statusFeedback;
    mailUrl->GetStatusFeedback(getter_AddRefs(statusFeedback));
    if (!statusFeedback) return;

    nsCOMPtr<nsIWebProgressListener> webProgressListener (do_QueryInterface(statusFeedback));
    if (!webProgressListener) return;

    // XXX not sure if m_request is correct here
    webProgressListener->OnProgressChange(nsnull, m_request, mNumBytesPosted, mFilePostSize, mNumBytesPosted, mFilePostSize);
  }

  return;
}
    

PRInt32 nsMsgAsyncWriteProtocol::SendData(nsIURI * aURL, const char * dataBuffer, PRBool aSuppressLogging)
{
  PRUint32 len = strlen(dataBuffer);
  PRUint32 cnt;
  nsresult rv = m_outputStream->Write(dataBuffer, len, &cnt);
  if (NS_SUCCEEDED(rv) && len==cnt) 
  {
    if (mSuspendedWrite) 
    {
      // if we got here then we had suspended the write 'cause we didn't have anymore
      // data to write (i.e. the pipe went empty). So resume the channel to kick
      // things off again.
      mSuspendedWrite = PR_FALSE;
      mAsyncOutStream->AsyncWait(mProvider, 0, 0, mProviderThread);
    } 
    return NS_OK;
  }
  else
    return NS_ERROR_FAILURE;
}

#define MSGS_URL    "chrome://messenger/locale/messenger.properties"

PRUnichar *FormatStringWithHostNameByID(PRInt32 stringID, nsIMsgMailNewsUrl *msgUri)
{
  if (! msgUri)
    return nsnull;

  nsresult rv;
  nsCOMPtr <nsIStringBundle> sBundle = nsnull;

  nsCOMPtr<nsIStringBundleService> sBundleService = 
          do_GetService(NS_STRINGBUNDLE_CONTRACTID, &rv);
  if (NS_FAILED(rv) || (! sBundleService)) 
    return nsnull;

  rv = sBundleService->CreateBundle(MSGS_URL, getter_AddRefs(sBundle));
  if (NS_FAILED(rv)) 
    return nsnull;

  PRUnichar *ptrv = nsnull;
  nsXPIDLCString hostName;
  nsCOMPtr<nsIMsgIncomingServer> server;
  rv = msgUri->GetServer(getter_AddRefs(server));

  if (NS_SUCCEEDED(rv) && server)
   rv = server->GetRealHostName(getter_Copies(hostName));

  nsAutoString hostStr;
  hostStr.AssignWithConversion(hostName.get());
  const PRUnichar *params[] = { hostStr.get() };
  rv = sBundle->FormatStringFromID(stringID, params, 1, &ptrv);
  if (NS_FAILED(rv)) 
    return nsnull;

  return (ptrv);
}

// vim: ts=2 sw=2
