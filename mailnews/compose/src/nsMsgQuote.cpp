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
 *   Pierre Phaneuf <pp@ludusdesign.com>
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

#include "nsIURL.h"
#include "nsIInputStream.h"
#include "nsIOutputStream.h"
#include "nsIGenericFactory.h"
#include "nsIServiceManager.h"
#include "nsIStreamListener.h"
#include "nsIStreamConverter.h"
#include "nsIStreamConverterService.h"
#include "nsIMimeStreamConverter.h"
#include "nsMimeTypes.h"
#include "nsICharsetConverterManager.h"
#include "prprf.h"
#include "nsMsgQuote.h" 
#include "nsMsgCompUtils.h"
#include "nsIMsgMessageService.h"
#include "nsMsgUtils.h"
#include "nsMsgDeliveryListener.h"
#include "nsNetUtil.h"
#include "nsMsgMimeCID.h"
#include "nsMsgCompCID.h"
#include "nsMsgCompose.h"
#include "nsMsgMailNewsUrl.h"
#include "nsXPIDLString.h"

NS_IMPL_THREADSAFE_ADDREF(nsMsgQuoteListener)
NS_IMPL_THREADSAFE_RELEASE(nsMsgQuoteListener)

NS_INTERFACE_MAP_BEGIN(nsMsgQuoteListener)
   NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(nsISupports, nsIMsgQuoteListener)
   NS_INTERFACE_MAP_ENTRY(nsIMimeStreamConverterListener)
   NS_INTERFACE_MAP_ENTRY(nsIMsgQuoteListener)
NS_INTERFACE_MAP_END

nsMsgQuoteListener::nsMsgQuoteListener()
{
}

nsMsgQuoteListener::~nsMsgQuoteListener()
{
}

NS_IMETHODIMP nsMsgQuoteListener::SetMsgQuote(nsIMsgQuote * msgQuote)
{
	mMsgQuote = do_GetWeakReference(msgQuote);
  return NS_OK;
}

NS_IMETHODIMP nsMsgQuoteListener::GetMsgQuote(nsIMsgQuote ** aMsgQuote)
{
  nsresult rv = NS_OK;
  if (aMsgQuote)
  {
    nsCOMPtr<nsIMsgQuote> msgQuote = do_QueryReferent(mMsgQuote);
    *aMsgQuote = msgQuote;
    NS_IF_ADDREF(*aMsgQuote);
  }
  else
    rv = NS_ERROR_NULL_POINTER;

  return rv;
}

nsresult nsMsgQuoteListener::OnHeadersReady(nsIMimeHeaders * headers)
{
  nsCOMPtr<nsIStreamListener> aStreamListener;
  nsCOMPtr<nsIMsgQuote> msgQuote = do_QueryReferent(mMsgQuote);

  if (msgQuote)
    msgQuote->GetStreamListener(getter_AddRefs(aStreamListener));

  if (aStreamListener)
    NS_STATIC_CAST(QuotingOutputStreamListener*, NS_STATIC_CAST(nsIStreamListener*, aStreamListener))->SetMimeHeaders(headers);
  return NS_OK;
}

//
// Implementation...
//
nsMsgQuote::nsMsgQuote()
{
  mQuoteHeaders = PR_FALSE;
  mQuoteListener = nsnull;
}

nsMsgQuote::~nsMsgQuote()
{
}

NS_IMPL_THREADSAFE_ADDREF(nsMsgQuote)
NS_IMPL_THREADSAFE_RELEASE(nsMsgQuote)

NS_INTERFACE_MAP_BEGIN(nsMsgQuote)
   NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(nsISupports, nsIMsgQuote)
   NS_INTERFACE_MAP_ENTRY(nsIMsgQuote)
   NS_INTERFACE_MAP_ENTRY(nsISupportsWeakReference)
NS_INTERFACE_MAP_END

NS_IMETHODIMP nsMsgQuote::GetStreamListener(nsIStreamListener ** aStreamListener)
{
  nsresult rv = NS_OK;
  if (aStreamListener)
  {
    *aStreamListener = mStreamListener;
    NS_IF_ADDREF(*aStreamListener);
  }
  else
    rv = NS_ERROR_NULL_POINTER;

  return rv;
}

nsresult
nsMsgQuote::QuoteMessage(const char *msgURI, PRBool quoteHeaders, nsIStreamListener * aQuoteMsgStreamListener,
                         const char * aMsgCharSet, PRBool headersOnly)
{
  nsresult  rv;
  if (!msgURI)
    return NS_ERROR_INVALID_ARG;

  mQuoteHeaders = quoteHeaders;
  mStreamListener = aQuoteMsgStreamListener;

  nsCAutoString msgUri(msgURI);
  PRBool fileUrl = !strncmp(msgURI, "file:", 5);
  PRBool forwardedMessage = PL_strstr(msgURI, "&realtype=message/rfc822") != nsnull;
  nsCOMPtr<nsIURI> aURL;
  if (fileUrl || forwardedMessage)
    rv = NS_NewURI(getter_AddRefs(aURL), msgURI);
  else
  {
    nsCOMPtr <nsIMsgMessageService> msgService;
    rv = GetMessageServiceFromURI(msgURI, getter_AddRefs(msgService));
    if (NS_FAILED(rv)) return rv;
    rv = msgService->GetUrlForUri(msgURI, getter_AddRefs(aURL), nsnull);
  }
  if (NS_FAILED(rv)) return rv;

  nsCOMPtr <nsIURL> mailNewsUrl = do_QueryInterface(aURL, &rv);
  NS_ENSURE_SUCCESS(rv,rv);

  nsCAutoString queryPart;
  rv = mailNewsUrl->GetQuery(queryPart);
  if (!queryPart.IsEmpty())
    queryPart.Append('&');

  if (headersOnly) /* We don't need to quote the message body but we still need to extract the headers */
    queryPart.Append("header=only");
  else if (quoteHeaders)
    queryPart.Append("header=quote");
  else
    queryPart.Append("header=quotebody");
  rv = mailNewsUrl->SetQuery(queryPart);
  NS_ENSURE_SUCCESS(rv,rv);

  // if we were given a non empty charset, then use it
  if (aMsgCharSet && *aMsgCharSet)
  {
    nsCOMPtr<nsIMsgI18NUrl> i18nUrl (do_QueryInterface(aURL));
    if (i18nUrl)
      i18nUrl->SetCharsetOverRide(aMsgCharSet);
  }

  mQuoteListener = do_CreateInstance(NS_MSGQUOTELISTENER_CONTRACTID, &rv);
  if (NS_FAILED(rv)) return rv;
  mQuoteListener->SetMsgQuote(this);

  // funky magic go get the isupports for this class which inherits from multiple interfaces.
  nsISupports * supports;
  QueryInterface(NS_GET_IID(nsISupports), (void **) &supports);
  nsCOMPtr<nsISupports> quoteSupport = supports;
  NS_IF_RELEASE(supports);

  // now we want to create a necko channel for this url and we want to open it
  mQuoteChannel = nsnull;
  nsCOMPtr<nsIIOService> netService(do_GetService(NS_IOSERVICE_CONTRACTID, &rv));
  if (NS_FAILED(rv)) return rv;
  rv = netService->NewChannelFromURI(aURL, getter_AddRefs(mQuoteChannel));
  if (NS_FAILED(rv)) return rv;
  nsCOMPtr<nsISupports> ctxt = do_QueryInterface(aURL);

  nsCOMPtr<nsIStreamConverterService> streamConverterService = 
           do_GetService("@mozilla.org/streamConverters;1", &rv);
  NS_ENSURE_SUCCESS(rv,rv);

  nsCOMPtr<nsIStreamListener> convertedListener;
  rv = streamConverterService->AsyncConvertData("message/rfc822",
                                                "application/vnd.mozilla.xul+xml",
                                                mStreamListener,
                                                quoteSupport,
                                                getter_AddRefs(convertedListener));
  if (NS_FAILED(rv)) return rv;

  //  now try to open the channel passing in our display consumer as the listener 
  rv = mQuoteChannel->AsyncOpen(convertedListener, ctxt);
  return rv;
}

NS_IMETHODIMP
nsMsgQuote::GetQuoteListener(nsIMimeStreamConverterListener** aQuoteListener)
{
    if (!aQuoteListener || !mQuoteListener)
        return NS_ERROR_NULL_POINTER;
    *aQuoteListener = mQuoteListener;
    NS_ADDREF(*aQuoteListener);
    return NS_OK;
}

NS_IMETHODIMP
nsMsgQuote::GetQuoteChannel(nsIChannel** aQuoteChannel)
{
    if (!aQuoteChannel || !mQuoteChannel)
        return NS_ERROR_NULL_POINTER;
    *aQuoteChannel = mQuoteChannel;
    NS_ADDREF(*aQuoteChannel);
    return NS_OK;
}
