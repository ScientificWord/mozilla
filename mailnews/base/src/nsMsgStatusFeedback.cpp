/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
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
 * Portions created by the Initial Developer are Copyright (C) 1999
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

#include "nsXPIDLString.h"

#include "nsIWebProgress.h"
#include "nsIDOMWindowInternal.h"
#include "nsPIDOMWindow.h"
#include "nsIXULBrowserWindow.h"
#include "nsMsgStatusFeedback.h"
#include "nsIDocumentViewer.h"
#include "nsIDocument.h"
#include "nsIDOMElement.h"
#include "nsIDocShell.h"
#include "nsIDocShellTreeItem.h"
#include "nsIChannel.h"
#include "prinrval.h"
#include "nsInt64.h"
#include "nsITimelineService.h"
#include "nsIMsgMailNewsUrl.h"
#include "nsIMimeMiscStatus.h"
#include "nsIMsgWindow.h"
#include "nsMsgUtils.h"
#include "nsIMsgHdr.h"

#define MSGFEEDBACK_TIMER_INTERVAL 500

nsMsgStatusFeedback::nsMsgStatusFeedback() :
  m_lastPercent(0)
{
	LL_I2L(m_lastProgressTime, 0);

    nsresult rv;
    nsCOMPtr<nsIStringBundleService> bundleService =
        do_GetService(NS_STRINGBUNDLE_CONTRACTID, &rv);

    if (NS_SUCCEEDED(rv))
        bundleService->CreateBundle("chrome://messenger/locale/messenger.properties",
                                    getter_AddRefs(mBundle));

    m_msgLoadedAtom = do_GetAtom("msgLoaded");
}

nsMsgStatusFeedback::~nsMsgStatusFeedback()
{
  mBundle = nsnull;
}

NS_IMPL_THREADSAFE_ADDREF(nsMsgStatusFeedback)
NS_IMPL_THREADSAFE_RELEASE(nsMsgStatusFeedback)

NS_INTERFACE_MAP_BEGIN(nsMsgStatusFeedback)
   NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(nsISupports, nsIMsgStatusFeedback)
   NS_INTERFACE_MAP_ENTRY(nsIMsgStatusFeedback)
   NS_INTERFACE_MAP_ENTRY(nsIProgressEventSink) 
   NS_INTERFACE_MAP_ENTRY(nsIWebProgressListener) 
   NS_INTERFACE_MAP_ENTRY(nsISupportsWeakReference)
NS_INTERFACE_MAP_END

//////////////////////////////////////////////////////////////////////////////////
// nsMsgStatusFeedback::nsIWebProgressListener
//////////////////////////////////////////////////////////////////////////////////

NS_IMETHODIMP
nsMsgStatusFeedback::OnProgressChange(nsIWebProgress* aWebProgress,
                                      nsIRequest* aRequest,
                                      PRInt32 aCurSelfProgress,
                                      PRInt32 aMaxSelfProgress, 
                                      PRInt32 aCurTotalProgress,
                                      PRInt32 aMaxTotalProgress)
{
  PRInt32 percentage = 0;
  if (aMaxTotalProgress > 0)
  {
    percentage =  (aCurTotalProgress * 100) / aMaxTotalProgress;
    if (percentage)
      ShowProgress(percentage);
  }

   return NS_OK;
}
      
NS_IMETHODIMP
nsMsgStatusFeedback::OnStateChange(nsIWebProgress* aWebProgress,
                                   nsIRequest* aRequest,
                                   PRUint32 aProgressStateFlags,
                                   nsresult aStatus)
{
  nsresult rv;

  NS_ENSURE_TRUE(mBundle, NS_ERROR_NULL_POINTER);
  if (aProgressStateFlags & STATE_IS_NETWORK)
  {
    if (aProgressStateFlags & STATE_START)
    {
      NS_TIMELINE_START_TIMER("Start Msg Loading");
      NS_TIMELINE_ENTER("Start Msg Loading in progress");
      m_lastPercent = 0;
      StartMeteors();
      nsXPIDLString loadingDocument;
      rv = mBundle->GetStringFromName(NS_LITERAL_STRING("documentLoading").get(),
                                      getter_Copies(loadingDocument));
      if (NS_SUCCEEDED(rv))
        ShowStatusString(loadingDocument);
    }
    else if (aProgressStateFlags & STATE_STOP)
    {
      NS_TIMELINE_STOP_TIMER("Start Msg Loading");
      NS_TIMELINE_LEAVE("Start Msg Loading is finished");
      NS_TIMELINE_MARK_TIMER("Start Msg Loading");
      NS_TIMELINE_RESET_TIMER("Start Msg Loading");

      // if we are loading message for display purposes, this STATE_STOP notification is 
      // the only notification we get when layout is actually done rendering the message. We need
      // to fire the appropriate msgHdrSink notification in this particular case.
      nsCOMPtr<nsIChannel> channel = do_QueryInterface(aRequest);
      if (channel) 
      {
        nsCOMPtr<nsIURI> uri; 
        channel->GetURI(getter_AddRefs(uri));
        nsCOMPtr<nsIMsgMailNewsUrl> mailnewsUrl (do_QueryInterface(uri));
        if (mailnewsUrl)
        {
          // get the url type
          PRBool messageDisplayUrl;
          mailnewsUrl->IsUrlType(nsIMsgMailNewsUrl::eDisplay, &messageDisplayUrl);

          if (messageDisplayUrl)
          {              
            // get the header sink
            nsCOMPtr<nsIMsgWindow> msgWindow;
            mailnewsUrl->GetMsgWindow(getter_AddRefs(msgWindow));
            if (msgWindow)
            {
              nsCOMPtr<nsIMsgHeaderSink> hdrSink;
              msgWindow->GetMsgHeaderSink(getter_AddRefs(hdrSink));
              if (hdrSink)
                hdrSink->OnEndMsgDownload(mailnewsUrl);
            }
            // get the folder and notify that the msg has been loaded. We're 
            // using NotifyPropertyFlagChanged. To be completely consistent,
            // we'd send a similar notification that the old message was
            // unloaded.
            nsXPIDLCString spec;
            nsCOMPtr <nsIMsgDBHdr> msgHdr;
            nsCOMPtr <nsIMsgFolder> msgFolder;
            mailnewsUrl->GetFolder(getter_AddRefs(msgFolder));
            nsCOMPtr <nsIMsgMessageUrl> msgUrl = do_QueryInterface(mailnewsUrl);
            if (msgUrl)
            {
              // not sending this notification is not a fatal error...
              (void) msgUrl->GetMessageHeader(getter_AddRefs(msgHdr));
              if (msgFolder && msgHdr)
                msgFolder->NotifyPropertyFlagChanged(msgHdr, m_msgLoadedAtom, 0, 1);
            }
          }
        }
      }
      StopMeteors();
      nsXPIDLString documentDone;
      rv = mBundle->GetStringFromName(NS_LITERAL_STRING("documentDone").get(),
                                      getter_Copies(documentDone));
      if (NS_SUCCEEDED(rv))
        ShowStatusString(documentDone);
    }
  }
  return NS_OK;
}

NS_IMETHODIMP nsMsgStatusFeedback::OnLocationChange(nsIWebProgress* aWebProgress,
                                                    nsIRequest* aRequest,
                                                    nsIURI* aLocation)
{
   return NS_OK;
}

NS_IMETHODIMP 
nsMsgStatusFeedback::OnStatusChange(nsIWebProgress* aWebProgress,
                                    nsIRequest* aRequest,
                                    nsresult aStatus,
                                    const PRUnichar* aMessage)
{
    return NS_OK;
}


NS_IMETHODIMP 
nsMsgStatusFeedback::OnSecurityChange(nsIWebProgress *aWebProgress, 
                                    nsIRequest *aRequest, 
                                    PRUint32 state)
{
    return NS_OK;
}


NS_IMETHODIMP
nsMsgStatusFeedback::ShowStatusString(const PRUnichar *status)
{
  if (mStatusFeedback)
    mStatusFeedback->ShowStatusString(status);
	return NS_OK;
}

NS_IMETHODIMP
nsMsgStatusFeedback::SetStatusString(const PRUnichar *status)
{
  nsCOMPtr <nsIXULBrowserWindow> xulBrowserWindow = do_QueryInterface(mStatusFeedback);
  if (xulBrowserWindow)
    xulBrowserWindow->SetJSDefaultStatus(nsDependentString(status));
  return NS_OK;
}

NS_IMETHODIMP
nsMsgStatusFeedback::ShowProgress(PRInt32 percentage)
{
  // if the percentage hasn't changed...OR if we are going from 0 to 100% in one step
  // then don't bother....just fall out....
	if (percentage == m_lastPercent || (m_lastPercent == 0 && percentage >= 100))
		return NS_OK;
  
  m_lastPercent = percentage;

	PRInt64 nowMS;
	LL_I2L(nowMS, 0);
	if (percentage < 100)	// always need to do 100%
	{
		int64 minIntervalBetweenProgress;

		LL_I2L(minIntervalBetweenProgress, 250);
		int64 diffSinceLastProgress;
		LL_I2L(nowMS, PR_IntervalToMilliseconds(PR_IntervalNow()));
		LL_SUB(diffSinceLastProgress, nowMS, m_lastProgressTime); // r = a - b
		LL_SUB(diffSinceLastProgress, diffSinceLastProgress, minIntervalBetweenProgress); // r = a - b
		if (!LL_GE_ZERO(diffSinceLastProgress))
			return NS_OK;
	}

	m_lastProgressTime = nowMS;
  
  if (mStatusFeedback)
    mStatusFeedback->ShowProgress(percentage);
	return NS_OK;
}

NS_IMETHODIMP
nsMsgStatusFeedback::StartMeteors()
{
  if (mStatusFeedback)
    mStatusFeedback->StartMeteors();
  return NS_OK;
}

NS_IMETHODIMP
nsMsgStatusFeedback::StopMeteors()
{
  if (mStatusFeedback)
    mStatusFeedback->StopMeteors();
  return NS_OK;
}

NS_IMETHODIMP nsMsgStatusFeedback::CloseWindow()
{
  mWindow = nsnull;
  mStatusFeedback = nsnull;

  return NS_OK;
}

NS_IMETHODIMP nsMsgStatusFeedback::SetDocShell(nsIDocShell *shell, nsIDOMWindow *aWindow)
{

  nsCOMPtr<nsPIDOMWindow> piDOMWindow(do_QueryInterface(aWindow));
  if (piDOMWindow)
  {
    nsCOMPtr<nsISupports> xpConnectObj;
    piDOMWindow->GetObjectProperty(NS_LITERAL_STRING("MsgStatusFeedback").get(), getter_AddRefs(xpConnectObj));
    mStatusFeedback = do_QueryInterface(xpConnectObj);
  }

  mWindow = aWindow;
  return NS_OK;
}

NS_IMETHODIMP nsMsgStatusFeedback::OnProgress(nsIRequest *request, nsISupports* ctxt, 
                                          PRUint64 aProgress, PRUint64 aProgressMax)
{
  // XXX: What should the nsIWebProgress be?
  // XXX: this truncates 64-bit to 32-bit
  return OnProgressChange(nsnull, request, nsUint64(aProgress), nsUint64(aProgressMax), 
                          nsUint64(aProgress) /* current total progress */, nsUint64(aProgressMax) /* max total progress */);
}

NS_IMETHODIMP nsMsgStatusFeedback::OnStatus(nsIRequest *request, nsISupports* ctxt, 
                                            nsresult aStatus, const PRUnichar* aStatusArg)
{
  nsresult rv;
  nsCOMPtr<nsIStringBundleService> sbs = do_GetService(NS_STRINGBUNDLE_CONTRACTID, &rv);
  if (NS_FAILED(rv)) return rv;
  nsXPIDLString str;
  rv = sbs->FormatStatusMessage(aStatus, aStatusArg, getter_Copies(str));
  if (NS_FAILED(rv)) return rv;
  nsAutoString msg(NS_STATIC_CAST(const PRUnichar*, str));
  return ShowStatusString(msg.get());
}
