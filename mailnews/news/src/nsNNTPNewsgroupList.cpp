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
 *   Henrik Gemal <mozilla@gemal.dk>
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

/*
 * formerly listngst.cpp
 * This class should ultimately be part of a news group listing
 * state machine - either by inheritance or delegation.
 * Currently, a folder pane owns one and libnet news group listing
 * related messages get passed to this object.
 */

#include "msgCore.h"    // precompiled header...
#include "MailNewsTypes.h"
#include "nsCOMPtr.h"
#include "nsIDBFolderInfo.h"
#include "nsINewsDatabase.h"
#include "nsIMsgStatusFeedback.h"
#include "nsCOMPtr.h"
#include "nsIDOMWindowInternal.h"

#include "nsXPIDLString.h"
#include "nsIMsgAccountManager.h"
#include "nsIMsgIncomingServer.h"
#include "nsINntpIncomingServer.h"
#include "nsMsgBaseCID.h"

#include "nsNNTPNewsgroupList.h"

#include "nsINNTPArticleList.h"
#include "nsMsgKeySet.h"

#include "nntpCore.h"
#include "nsIStringBundle.h"

#include "plstr.h"
#include "prmem.h"
#include "prprf.h"

#include "nsCRT.h"
#include "nsMsgUtils.h"

#include "nsMsgDatabase.h"

#include "nsIDBFolderInfo.h"

#include "nsNewsUtils.h"

#include "nsMsgDBCID.h"

#include "nsINewsDownloadDialogArgs.h"

#include "nsXPCOM.h"
#include "nsISupportsPrimitives.h"
#include "nsIInterfaceRequestor.h"
#include "nsIInterfaceRequestorUtils.h"
#include "nsIMsgWindow.h"
#include "nsIDocShell.h"

// update status on header download once per second
#define MIN_STATUS_UPDATE_INTERVAL PR_USEC_PER_SEC


nsNNTPNewsgroupList::nsNNTPNewsgroupList()
  : m_finishingXover(PR_FALSE),
    m_getOldMessages(PR_FALSE),
    m_promptedAlready(PR_FALSE),
    m_downloadAll(PR_FALSE),
    m_maxArticles(0),
    m_lastPercent(-1),
    m_lastProcessedNumber(0),
    m_firstMsgNumber(0),
    m_lastMsgNumber(0),
    m_firstMsgToDownload(0),
    m_lastMsgToDownload(0),
    m_set(nsnull)
{
    memset(&m_knownArts, 0, sizeof(m_knownArts));
    m_lastStatusUpdate = LL_Zero();
}

nsNNTPNewsgroupList::~nsNNTPNewsgroupList()
{
  CleanUp();
}

NS_IMPL_ISUPPORTS2(nsNNTPNewsgroupList, nsINNTPNewsgroupList, nsIMsgFilterHitNotify)

nsresult
nsNNTPNewsgroupList::Initialize(nsINntpUrl *runningURL, nsIMsgNewsFolder *newsFolder)
{
  m_newsFolder = newsFolder;
  m_runningURL = runningURL;
  m_knownArts.set = nsMsgKeySet::Create();

  return NS_OK;
}

nsresult
nsNNTPNewsgroupList::CleanUp() 
{
  if (m_newsDB) {
    m_newsDB->Commit(nsMsgDBCommitType::kSessionCommit);
    m_newsDB->Close(PR_TRUE);
    m_newsDB = nsnull;
  }

  if (m_knownArts.set) {
    delete m_knownArts.set;
    m_knownArts.set = nsnull;
  }

  m_newsFolder = nsnull;
  m_runningURL = nsnull;
    
  return NS_OK;
}

#ifdef HAVE_CHANGELISTENER
void nsNNTPNewsgroupList::OnAnnouncerGoingAway (ChangeAnnouncer *instigator)
{
}
#endif

static nsresult 
openWindow(nsIMsgWindow *aMsgWindow, const char *chromeURL,
           nsINewsDownloadDialogArgs *param) 
{
    nsresult rv;

    NS_ENSURE_ARG_POINTER(aMsgWindow);

	nsCOMPtr<nsIDocShell> docShell;
	rv = aMsgWindow->GetRootDocShell(getter_AddRefs(docShell));
    if (NS_FAILED(rv))
        return rv;

   	nsCOMPtr<nsIDOMWindowInternal> parentWindow(do_GetInterface(docShell));
	NS_ENSURE_TRUE(parentWindow, NS_ERROR_FAILURE);

    nsCOMPtr<nsISupportsInterfacePointer> ifptr =
        do_CreateInstance(NS_SUPPORTS_INTERFACE_POINTER_CONTRACTID, &rv);
    NS_ENSURE_SUCCESS(rv, rv);

    ifptr->SetData(param);
    ifptr->SetDataIID(&NS_GET_IID(nsINewsDownloadDialogArgs));

    nsCOMPtr<nsIDOMWindow> dialogWindow;
    rv = parentWindow->OpenDialog(NS_ConvertASCIItoUTF16(chromeURL),
                                  NS_LITERAL_STRING("_blank"),
                                  NS_LITERAL_STRING("centerscreen,chrome,modal,titlebar"),
                                  ifptr, getter_AddRefs(dialogWindow));

    return rv;
}       

nsresult
nsNNTPNewsgroupList::GetRangeOfArtsToDownload(nsIMsgWindow *aMsgWindow,
                                              PRInt32 first_possible,
                                              PRInt32 last_possible,
                                              PRInt32 maxextra,
                                              PRInt32 *first,
                                              PRInt32 *last,
                                              PRInt32 *status)
{
	nsresult rv = NS_OK;

    NS_ENSURE_ARG_POINTER(first);
    NS_ENSURE_ARG_POINTER(last);
    NS_ENSURE_ARG_POINTER(status);
    
	*first = 0;
	*last = 0;

    nsCOMPtr <nsIMsgFolder> folder = do_QueryInterface(m_newsFolder, &rv);
    NS_ENSURE_SUCCESS(rv,rv);

    m_msgWindow = aMsgWindow;

	if (!m_newsDB) {
      rv = folder->GetMsgDatabase(nsnull /* use m_msgWindow? */, getter_AddRefs(m_newsDB));
	}
	
    nsCOMPtr<nsINewsDatabase> db(do_QueryInterface(m_newsDB, &rv));
    NS_ENSURE_SUCCESS(rv,rv);
            
	rv = db->GetReadSet(&m_set);
    if (NS_FAILED(rv) || !m_set) {
       return rv;
    }
            
	m_set->SetLastMember(last_possible);	// make sure highwater mark is valid.

    nsCOMPtr <nsIDBFolderInfo> newsGroupInfo;
	rv = m_newsDB->GetDBFolderInfo(getter_AddRefs(newsGroupInfo));
	if (NS_SUCCEEDED(rv) && newsGroupInfo) {
      nsXPIDLCString knownArtsString;
      nsMsgKey mark;
      newsGroupInfo->GetKnownArtsSet(getter_Copies(knownArtsString));
      
      rv = newsGroupInfo->GetHighWater(&mark);
      NS_ENSURE_SUCCESS(rv,rv);

      if (last_possible < ((PRInt32)mark))
        newsGroupInfo->SetHighWater(last_possible, PR_TRUE);
      if (m_knownArts.set) {
        delete m_knownArts.set;
      }
      m_knownArts.set = nsMsgKeySet::Create(knownArtsString.get());
    }
    else
    {	
      if (m_knownArts.set) {
        delete m_knownArts.set;
      }
      m_knownArts.set = nsMsgKeySet::Create();
      nsMsgKey low, high;
      rv = m_newsDB->GetLowWaterArticleNum(&low);
      NS_ENSURE_SUCCESS(rv,rv);
      rv = m_newsDB->GetHighWaterArticleNum(&high);
      NS_ENSURE_SUCCESS(rv,rv);
      
      m_knownArts.set->AddRange(low,high);
    }
    
    if (m_knownArts.set->IsMember(last_possible)) {
      nsXPIDLString statusString;
      nsCOMPtr<nsIStringBundleService> bundleService = do_GetService(NS_STRINGBUNDLE_CONTRACTID, &rv);
      NS_ENSURE_SUCCESS(rv, rv);
      
      nsCOMPtr<nsIStringBundle> bundle;
      rv = bundleService->CreateBundle(NEWS_MSGS_URL, getter_AddRefs(bundle));
      NS_ENSURE_SUCCESS(rv, rv);
      
      rv = bundle->GetStringFromName(NS_LITERAL_STRING("noNewMessages").get(), getter_Copies(statusString));
      NS_ENSURE_SUCCESS(rv, rv);

      SetProgressStatus(statusString);
    }
    
    if (maxextra <= 0 || last_possible < first_possible || last_possible < 1) 
    {
      *status=0;
      return NS_OK;
    }

    m_knownArts.first_possible = first_possible;
    m_knownArts.last_possible = last_possible;

    nsCOMPtr <nsIMsgIncomingServer> server;
    rv = folder->GetServer(getter_AddRefs(server));
    NS_ENSURE_SUCCESS(rv,rv);
		
    nsCOMPtr<nsINntpIncomingServer> nntpServer = do_QueryInterface(server, &rv);
    NS_ENSURE_SUCCESS(rv,rv);

	/* Determine if we only want to get just new articles or more messages.
	If there are new articles at the end we haven't seen, we always want to get those first.  
	Otherwise, we get the newest articles we haven't gotten, if we're getting more. 
	My thought for now is that opening a newsgroup should only try to get new articles.
	Selecting "More Messages" will first try to get unseen messages, then old messages. */

	if (m_getOldMessages || !m_knownArts.set->IsMember(last_possible)) 
	{
		PRBool notifyMaxExceededOn = PR_TRUE;
		rv = nntpServer->GetNotifyOn(&notifyMaxExceededOn);
		if (NS_FAILED(rv)) notifyMaxExceededOn = PR_TRUE;

		// if the preference to notify when downloading more than x headers is not on,
		// and we're downloading new headers, set maxextra to a very large number.
		if (!m_getOldMessages && !notifyMaxExceededOn)
			maxextra = 0x7FFFFFFFL;
        int result =
            m_knownArts.set->LastMissingRange(first_possible, last_possible,
                                              first, last);
		if (result < 0) {
            *status=result;
			return NS_ERROR_NOT_INITIALIZED;
        }
		if (*first > 0 && *last - *first >= maxextra) 
		{
			if (!m_getOldMessages && !m_promptedAlready && notifyMaxExceededOn)
			{
				m_downloadAll = PR_FALSE;   
			
                nsCOMPtr<nsINewsDownloadDialogArgs> args = do_CreateInstance("@mozilla.org/messenger/newsdownloaddialogargs;1", &rv);
                if (NS_FAILED(rv)) return rv;
                NS_ENSURE_SUCCESS(rv,rv);

                rv = args->SetArticleCount(*last - *first + 1);
                NS_ENSURE_SUCCESS(rv,rv);
        
                nsXPIDLString groupName;
                rv = m_newsFolder->GetUnicodeName(groupName);
                NS_ENSURE_SUCCESS(rv,rv);

                rv = args->SetGroupName(groupName);
                NS_ENSURE_SUCCESS(rv,rv);

                // get the server key
                nsXPIDLCString serverKey;
                rv = server->GetKey(getter_Copies(serverKey));
                NS_ENSURE_SUCCESS(rv,rv);

                rv = args->SetServerKey((const char *)serverKey);
                NS_ENSURE_SUCCESS(rv,rv);

                // we many not have a msgWindow if we are running an autosubscribe url from the browser
                // and there isn't a 3 pane open.
                //
                // if we don't have one, bad things will happen when we fail to open up the "download headers dialog"
                // (we will subscribe to the newsgroup, but it will appear like there are no messages!)
                //
                // for now, act like the "download headers dialog" came up, and the user hit cancel.  (very safe)
                //
                // TODO, figure out why we aren't opening and using a 3 pane when the autosubscribe url is run.
                // perhaps we can find an available 3 pane, and use it.

                PRBool download = PR_FALSE;  

                if (aMsgWindow) {
			  	  rv = openWindow(aMsgWindow, DOWNLOAD_HEADERS_URL, args);
                  NS_ENSURE_SUCCESS(rv,rv);

                  rv = args->GetHitOK(&download);
                  NS_ENSURE_SUCCESS(rv,rv);
                }

				if (download) {
                    rv = args->GetDownloadAll(&m_downloadAll);
                    NS_ENSURE_SUCCESS(rv,rv);

					m_maxArticles = 0;

                    rv = nntpServer->GetMaxArticles(&m_maxArticles); 
                    NS_ENSURE_SUCCESS(rv,rv);
                    
					maxextra = m_maxArticles;
					if (!m_downloadAll)
					{
						PRBool markOldRead = PR_FALSE;

						rv = nntpServer->GetMarkOldRead(&markOldRead);
                        if (NS_FAILED(rv)) markOldRead = PR_FALSE;

						if (markOldRead && m_set)
							m_set->AddRange(*first, *last - maxextra); 
						*first = *last - maxextra + 1;
					}
				}
				else
					*first = *last = 0;
				m_promptedAlready = PR_TRUE;
			}
			else if (m_promptedAlready && !m_downloadAll)
				*first = *last - m_maxArticles + 1;
			else if (!m_downloadAll)
				*first = *last - maxextra + 1;
		}
	}

	m_firstMsgToDownload = *first;
	m_lastMsgToDownload = *last;
    *status=0;
	return NS_OK;
}

nsresult
nsNNTPNewsgroupList::AddToKnownArticles(PRInt32 first, PRInt32 last)
{
  int		status;
  
  if (!m_knownArts.set) 
  {
    m_knownArts.set = nsMsgKeySet::Create();
    
    if (!m_knownArts.set) {
      return NS_ERROR_OUT_OF_MEMORY;
    }
    
  }
  
  status = m_knownArts.set->AddRange(first, last);
  
  if (m_newsDB) {
    nsresult rv = NS_OK;
    nsCOMPtr <nsIDBFolderInfo> newsGroupInfo;
    rv = m_newsDB->GetDBFolderInfo(getter_AddRefs(newsGroupInfo));
    if (NS_SUCCEEDED(rv) && newsGroupInfo) {
      nsXPIDLCString output;
      status = m_knownArts.set->Output(getter_Copies(output));
      if (output) {
        newsGroupInfo->SetKnownArtsSet(output);
      }
    }
  }
  
  return status;
}




nsresult
nsNNTPNewsgroupList::InitXOVER(PRInt32 first_msg, PRInt32 last_msg)
{
	int		status = 0;

	// Tell the FE to show the GetNewMessages progress dialog
#ifdef HAVE_PANES
	FE_PaneChanged (m_pane, PR_FALSE, MSG_PanePastPasswordCheck, 0);
#endif
	/* Consistency checks, not that I know what to do if it fails (it will
	 probably handle it OK...) */
	NS_ASSERTION(first_msg <= last_msg, "first > last");

	/* If any XOVER lines from the last time failed to come in, mark those
	   messages as read. */
	if (m_lastProcessedNumber < m_lastMsgNumber) 
	{
		m_set->AddRange(m_lastProcessedNumber + 1, m_lastMsgNumber);
	}
	m_firstMsgNumber = first_msg;
	m_lastMsgNumber = last_msg;
	m_lastProcessedNumber = first_msg > 1 ? first_msg - 1 : 1;

	return status;
}

// from RFC 822, don't translate 
#define FROM_HEADER "From: "
#define SUBECT_HEADER "Subject: "
#define DATE_HEADER "Date: "

nsresult
nsNNTPNewsgroupList::ParseLine(char *line, PRUint32 * message_number) 
{
  nsresult rv = NS_OK;
  nsCOMPtr <nsIMsgDBHdr> newMsgHdr;
  char *dateStr = nsnull;  // keep track of date str, for filters
  char *authorStr = nsnull; // keep track of author str, for filters

  if (!line || !message_number) {
    return NS_ERROR_NULL_POINTER;
  }
  
  char *next = line;
  
#define GET_TOKEN()								\
  line = next;									\
  next = (line ? PL_strchr (line, '\t') : 0);	\
  if (next) *next++ = 0
  
  GET_TOKEN (); /* message number */
  *message_number = atol(line);
  
  if (atol(line) == 0)					/* bogus xover data */
    return NS_ERROR_UNEXPECTED;
  
  m_newsDB->CreateNewHdr(*message_number, getter_AddRefs(newMsgHdr));      
  
  NS_ASSERTION(newMsgHdr, "CreateNewHdr didn't fail, but it returned a null newMsgHdr");
  if (!newMsgHdr) 
    return NS_ERROR_NULL_POINTER;
  
  GET_TOKEN (); /* subject */
  if (line) {
    const char *subject = line;  /* #### const evilness */
    PRUint32 subjectLen = strlen(line);
    
    PRUint32 flags = 0;
    // ### should call IsHeaderRead here...
    /* strip "Re: " */
    nsXPIDLCString modifiedSubject;
    if (NS_MsgStripRE(&subject, &subjectLen, getter_Copies(modifiedSubject)))
      (void) newMsgHdr->OrFlags(MSG_FLAG_HAS_RE, &flags); // this will make sure read flags agree with newsrc
    
    if (! (flags & MSG_FLAG_READ))
      rv = newMsgHdr->OrFlags(MSG_FLAG_NEW, &flags);
    
    rv = newMsgHdr->SetSubject(modifiedSubject.IsEmpty() ? subject : modifiedSubject.get());
    if (NS_FAILED(rv)) 
      return rv;
  }
  
  GET_TOKEN ();											/* author */
  if (line) {
    authorStr = line;
    rv = newMsgHdr->SetAuthor(line);
    if (NS_FAILED(rv)) 
      return rv;
  }
  
  GET_TOKEN ();	
  if (line) {
    dateStr = line;
    PRTime date;
    PRStatus status = PR_ParseTimeString (line, PR_FALSE, &date);
    if (PR_SUCCESS == status) {      
      rv = newMsgHdr->SetDate(date);					/* date */
      if (NS_FAILED(rv)) 
        return rv;
    }
  }
  
  GET_TOKEN ();											/* message id */
  if (line) {
    char *strippedId = line;
    
    if (strippedId[0] == '<')
      strippedId++;
    
    char * lastChar = strippedId + PL_strlen(strippedId) -1;
    
    if (*lastChar == '>')
      *lastChar = '\0';
    
    rv = newMsgHdr->SetMessageId(strippedId);
    if (NS_FAILED(rv)) 
      return rv;           
  }
  
  GET_TOKEN ();											/* references */
  if (line) {
    rv = newMsgHdr->SetReferences(line);
    if (NS_FAILED(rv)) 
      return rv;           
  }
  
  GET_TOKEN ();											/* bytes */
  if (line) {
    PRUint32 msgSize = 0;
    msgSize = (line) ? atol (line) : 0;
    
    rv = newMsgHdr->SetMessageSize(msgSize);
    if (NS_FAILED(rv)) return rv;           
  }
  
  GET_TOKEN ();											/* lines */
  if (line) {
    PRUint32 numLines = 0;
    numLines = line ? atol (line) : 0;
    rv = newMsgHdr->SetLineCount(numLines);
    if (NS_FAILED(rv)) return rv;           
  }
  
  GET_TOKEN (); /* xref */
  
  // apply filters
  // XXX TODO
  // do spam classification for news

  nsCOMPtr <nsIMsgFolder> folder = do_QueryInterface(m_newsFolder, &rv);
  NS_ENSURE_SUCCESS(rv,rv);
  
  if (!m_filterList) 
  {
    rv = folder->GetFilterList(m_msgWindow, getter_AddRefs(m_filterList));
    NS_ENSURE_SUCCESS(rv,rv);
  }
  
  if (!m_serverFilterList)
  {
    nsCOMPtr<nsIMsgIncomingServer> server;
    rv = folder->GetServer(getter_AddRefs(server));
    NS_ENSURE_SUCCESS(rv,rv);

    rv = server->GetFilterList(m_msgWindow, getter_AddRefs(m_serverFilterList));
    NS_ENSURE_SUCCESS(rv,rv);
  }

  // flag for kill
  // if the action is Delete, and we get a hit (see ApplyFilterHit())
  // we set this to PR_FALSE.  if false, we won't add it to the db.
  m_addHdrToDB = PR_TRUE;

  PRUint32 filterCount = 0;
  if (m_filterList) {
  	rv = m_filterList->GetFilterCount(&filterCount);
    NS_ENSURE_SUCCESS(rv,rv);
  }

  PRUint32 serverFilterCount = 0;
  if (m_serverFilterList) {
  	rv = m_serverFilterList->GetFilterCount(&serverFilterCount);
    NS_ENSURE_SUCCESS(rv,rv);
  }

  // only do this if we have filters
  if (filterCount || serverFilterCount) 
  {
    // build up a "headers" for filter code
    nsXPIDLCString subject;
    rv = newMsgHdr->GetSubject(getter_Copies(subject));
    NS_ENSURE_SUCCESS(rv,rv);
    
    PRUint32 headersSize = 0;
 
    // +1 to separate headers with a null byte 
    if (authorStr)
      headersSize += strlen(FROM_HEADER) + strlen(authorStr) + 1;
    
    if (!(subject.IsEmpty()))
      headersSize += strlen(SUBECT_HEADER) + subject.Length() + 1;

    if (dateStr)
     headersSize += strlen(DATE_HEADER) + strlen(dateStr) + 1;
    
    if (headersSize) {
      char *headers = (char *)PR_Malloc(headersSize);
      char *headerPos = headers;
      if (!headers)
        return NS_ERROR_OUT_OF_MEMORY;
    
      if (authorStr) {
        PL_strcpy(headerPos, FROM_HEADER);
        headerPos += strlen(FROM_HEADER);
    
        PL_strcpy(headerPos, authorStr);
        headerPos += strlen(authorStr);
    
        *headerPos = '\0';
        headerPos++;
      }

      if (!(subject.IsEmpty())) {
        PL_strcpy(headerPos, SUBECT_HEADER);
        headerPos += strlen(SUBECT_HEADER);
        
        PL_strcpy(headerPos, subject.get());
        headerPos += subject.Length();
        
        *headerPos = '\0';
        headerPos++;
      }

      if (dateStr) {        
        PL_strcpy(headerPos, DATE_HEADER);
        headerPos += strlen(DATE_HEADER);
        
        PL_strcpy(headerPos, dateStr);
        headerPos += strlen(dateStr);
        
        *headerPos = '\0';
        headerPos++;
      }

      // on a filter hit (see ApplyFilterHit()), we'll be modifying the header 
      // so keep track of the header
      m_newMsgHdr = newMsgHdr;
      
      // the per-newsgroup filters should probably go first. It doesn't matter
      // right now since nothing stops filter execution for newsgroups, but if something
      // does, like adding a "stop execution" action, then users should be able to
      // override the global filters in the per-newsgroup filters.
      if (filterCount)
      {
        rv = m_filterList->ApplyFiltersToHdr(nsMsgFilterType::NewsRule, newMsgHdr, folder, m_newsDB, 
          headers, headersSize, this, m_msgWindow, nsnull);
      }
      if (serverFilterCount)
      {
        rv = m_serverFilterList->ApplyFiltersToHdr(nsMsgFilterType::NewsRule, newMsgHdr, folder, m_newsDB, 
          headers, headersSize, this, m_msgWindow, nsnull);
      }

      PR_Free ((void*) headers);
      NS_ENSURE_SUCCESS(rv,rv);
    }
  }
  
  // if we deleted it, don't add it
  if (m_addHdrToDB) {
    rv = m_newsDB->AddNewHdrToDB(newMsgHdr, PR_TRUE);
    NS_ENSURE_SUCCESS(rv,rv);
  }
  
  return NS_OK;
}

NS_IMETHODIMP nsNNTPNewsgroupList::ApplyFilterHit(nsIMsgFilter *aFilter, nsIMsgWindow *aMsgWindow, PRBool *aApplyMore)
{
  NS_ENSURE_ARG_POINTER(aFilter);
  NS_ENSURE_ARG_POINTER(aApplyMore);
  NS_ENSURE_TRUE(m_newMsgHdr, NS_ERROR_UNEXPECTED);
  NS_ENSURE_TRUE(m_newsDB, NS_ERROR_UNEXPECTED);
   
  // you can't move news messages, so applyMore is always true
  *aApplyMore = PR_TRUE;
  
  nsCOMPtr<nsISupportsArray> filterActionList;
  nsresult rv = NS_NewISupportsArray(getter_AddRefs(filterActionList));
  NS_ENSURE_SUCCESS(rv, rv);
  rv = aFilter->GetSortedActionList(filterActionList);
  NS_ENSURE_SUCCESS(rv, rv);

  PRUint32 numActions;
  rv = filterActionList->Count(&numActions);
  NS_ENSURE_SUCCESS(rv, rv);

  PRBool loggingEnabled = PR_FALSE;
  nsCOMPtr<nsIMsgFilterList> currentFilterList;
  rv = aFilter->GetFilterList(getter_AddRefs(currentFilterList));
  if (NS_SUCCEEDED(rv) && currentFilterList && numActions)
    currentFilterList->GetLoggingEnabled(&loggingEnabled);

  for (PRUint32 actionIndex = 0; actionIndex < numActions; actionIndex++)
  {
    nsCOMPtr<nsIMsgRuleAction> filterAction;
    filterActionList->QueryElementAt(actionIndex, NS_GET_IID(nsIMsgRuleAction), getter_AddRefs(filterAction));
    if (!filterAction)
      continue;
    
    nsMsgRuleActionType actionType;
    if (NS_SUCCEEDED(filterAction->GetType(&actionType)))
    {  
      switch (actionType)
      {
      case nsMsgFilterAction::Delete:
        m_addHdrToDB = PR_FALSE;
        break;
      case nsMsgFilterAction::MarkRead:
        m_newsDB->MarkHdrRead(m_newMsgHdr, PR_TRUE, nsnull);
        break;
      case nsMsgFilterAction::KillThread:
        {
          PRUint32 newFlags;
          // The db will check for this flag when a hdr gets added to the db, and set the flag appropriately on the thread object
          m_newMsgHdr->OrFlags(MSG_FLAG_IGNORED, &newFlags);
        }
        break;
      case nsMsgFilterAction::WatchThread:
        {
          PRUint32 newFlags;
          m_newMsgHdr->OrFlags(MSG_FLAG_WATCHED, &newFlags);
        }
        break;
      case nsMsgFilterAction::MarkFlagged:
        m_newMsgHdr->MarkFlagged(PR_TRUE);
        break;
      case nsMsgFilterAction::ChangePriority:
        {
          nsMsgPriorityValue filterPriority;
          filterAction->GetPriority(&filterPriority);
          m_newMsgHdr->SetPriority(filterPriority);
        }
        break;
      case nsMsgFilterAction::Label:
        {
          nsMsgLabelValue filterLabel;
          filterAction->GetLabel(&filterLabel);
          nsMsgKey msgKey;
          m_newMsgHdr->GetMessageKey(&msgKey);
          m_newsDB->SetLabel(msgKey, filterLabel);
        }
        break;
      default:
        NS_ASSERTION(0, "unexpected action");
        break;
      }
      
      if (loggingEnabled)
        (void) aFilter->LogRuleHit(filterAction, m_newMsgHdr);
    }
  }
  return NS_OK;
}

nsresult
nsNNTPNewsgroupList::ProcessXOVERLINE(const char *line, PRUint32 *status)
{
  PRUint32 message_number=0;
  //  PRInt32 lines;
  PRBool read_p = PR_FALSE;
  nsresult rv = NS_OK;

  NS_ASSERTION(line, "null ptr");
  if (!line)
    return NS_ERROR_NULL_POINTER;

  if (m_newsDB)
  {
    char *xoverline = PL_strdup(line);
    if (!xoverline) 
      return NS_ERROR_OUT_OF_MEMORY;
    rv = ParseLine(xoverline, &message_number);
    PL_strfree(xoverline);
    xoverline = nsnull;
    if (NS_FAILED(rv))
      return rv;
  }
  else
    return NS_ERROR_NOT_INITIALIZED;

  NS_ASSERTION(message_number > m_lastProcessedNumber ||
               message_number == 1, "bad message_number");
  if (m_set && message_number > m_lastProcessedNumber + 1)
  {
  /* There are some articles that XOVER skipped; they must no longer
     exist.  Mark them as read in the newsrc, so we don't include them
     next time in our estimated number of unread messages. */
    if (m_set->AddRange(m_lastProcessedNumber + 1, message_number - 1)) 
    {
    /* This isn't really an important enough change to warrant causing
       the newsrc file to be saved; we haven't gathered any information
       that won't also be gathered for free next time.  */
    }
  }

  m_lastProcessedNumber = message_number;
  if (m_knownArts.set) 
  {
    int result = m_knownArts.set->Add(message_number);
    if (result < 0) {
      if (status) 
        *status = result;
      return NS_ERROR_NOT_INITIALIZED;
    }
  }

  if (message_number > m_lastMsgNumber)
    m_lastMsgNumber = message_number;
  else if (message_number < m_firstMsgNumber)
    m_firstMsgNumber = message_number;

  if (m_set) {
    read_p = m_set->IsMember(message_number);
  }

  /* Update the progress meter with a percentage of articles retrieved */
  if (m_lastMsgNumber > m_firstMsgNumber)
  {
    PRInt32 totalToDownload = m_lastMsgToDownload - m_firstMsgToDownload + 1;
    PRInt32 lastIndex = m_lastProcessedNumber - m_firstMsgNumber + 1;
    PRInt32 numDownloaded = lastIndex;
    PRInt32 totIndex = m_lastMsgNumber - m_firstMsgNumber + 1;

    PRInt32  percent = (totIndex) ? (PRInt32)(100.0 * (double)numDownloaded / (double)totalToDownload) : 0;

    PRTime elapsedTime;

    LL_SUB(elapsedTime, PR_Now(), m_lastStatusUpdate);

    if (LL_CMP(elapsedTime, >, MIN_STATUS_UPDATE_INTERVAL) || 
        lastIndex == totIndex)
    {
      nsAutoString numDownloadedStr;
      numDownloadedStr.AppendInt(numDownloaded);

      nsAutoString totalToDownloadStr;
      totalToDownloadStr.AppendInt(totalToDownload);

      nsXPIDLString statusString;
      nsCOMPtr<nsIStringBundleService> bundleService = do_GetService(NS_STRINGBUNDLE_CONTRACTID, &rv);
      NS_ENSURE_SUCCESS(rv, rv);

      nsCOMPtr<nsIStringBundle> bundle;
      rv = bundleService->CreateBundle(NEWS_MSGS_URL, getter_AddRefs(bundle));
      NS_ENSURE_SUCCESS(rv, rv);

      const PRUnichar *formatStrings[2] = { numDownloadedStr.get(), totalToDownloadStr.get() };
      rv = bundle->FormatStringFromName(NS_LITERAL_STRING("downloadingHeaders").get(), formatStrings, 2, getter_Copies(statusString));
      NS_ENSURE_SUCCESS(rv, rv);
      
#ifdef DEBUG_NEWS
      PRInt32 elapsed;
      LL_L2I(elapsed, elapsedTime);
      printf("usecs elapsed since last update: %d\n", elapsed);
#endif

      SetProgressStatus(statusString);
      m_lastStatusUpdate = PR_Now();

      // only update the progress meter if it has changed
      if (percent != m_lastPercent) {
        SetProgressBarPercent(percent);
        m_lastPercent = percent;
      }
    }
  }
  return NS_OK;
}

nsresult
nsNNTPNewsgroupList::ResetXOVER()
{
  m_lastMsgNumber = m_firstMsgNumber;
  m_lastProcessedNumber = m_lastMsgNumber;
  return 0;
}

/* When we don't have XOVER, but use HEAD, this is called instead.
   It reads lines until it has a whole header block, then parses the
   headers; then takes selected headers and creates an XOVER line
   from them.  This is more for simplicity and code sharing than
   anything else; it means we end up parsing some things twice.
   But if we don't have XOVER, things are going to be so horribly
   slow anyway that this just doesn't matter.
 */

nsresult
nsNNTPNewsgroupList::ProcessNonXOVER (const char * /*line*/)
{
  // ### dmb write me
  return NS_ERROR_NOT_IMPLEMENTED;
}

nsresult
nsNNTPNewsgroupList::FinishXOVERLINE(int status, int *newstatus)
{
  nsresult rv;
  struct MSG_NewsKnown* k;

  /* If any XOVER lines from the last time failed to come in, mark those
     messages as read. */

  if (status >= 0 && m_lastProcessedNumber < m_lastMsgNumber) {
    m_set->AddRange(m_lastProcessedNumber + 1, m_lastMsgNumber);
  }

  if (m_lastProcessedNumber)
    AddToKnownArticles(m_firstMsgNumber, m_lastProcessedNumber);
  if (m_newsDB) {
    m_newsDB->Close(PR_TRUE);
    m_newsDB = nsnull;
  }

  k = &m_knownArts;

  if (k && k->set) 
  {
    PRInt32 n = k->set->FirstNonMember();
    if (n < k->first_possible || n > k->last_possible) 
    {
      /* We know we've gotten all there is to know.  
         Take advantage of that to update our counts... */
      // ### dmb
    }
  }

  if (!m_finishingXover)
  {
    // turn on m_finishingXover - this is a horrible hack to avoid recursive 
    // calls which happen when the fe selects a message as a result of getting EndingUpdate,
    // which interrupts this url right before it was going to finish and causes FinishXOver
    // to get called again.
    m_finishingXover = PR_TRUE;

    // XXX is this correct?
    m_runningURL = nsnull;

    if (m_lastMsgNumber > 0) {
      nsAutoString firstStr;
      firstStr.AppendInt(m_lastProcessedNumber - m_firstMsgNumber + 1);

      nsAutoString lastStr;
      lastStr.AppendInt(m_lastMsgNumber - m_firstMsgNumber + 1);

      nsXPIDLString statusString;
      nsCOMPtr<nsIStringBundleService> bundleService = do_GetService(NS_STRINGBUNDLE_CONTRACTID, &rv);
      NS_ENSURE_SUCCESS(rv, rv);

      nsCOMPtr<nsIStringBundle> bundle;
      rv = bundleService->CreateBundle(NEWS_MSGS_URL, getter_AddRefs(bundle));
      NS_ENSURE_SUCCESS(rv, rv);

      const PRUnichar *formatStrings[2] = { firstStr.get(), lastStr.get() };
      rv = bundle->FormatStringFromName(NS_LITERAL_STRING("downloadingArticles").get(), formatStrings, 2, getter_Copies(statusString));
      NS_ENSURE_SUCCESS(rv, rv);

      SetProgressStatus(statusString);
    }
  }

  if (newstatus) 
    *newstatus=0;

  return NS_OK;
}

nsresult
nsNNTPNewsgroupList::ClearXOVERState()
{
    return NS_OK;
}

void
nsNNTPNewsgroupList::SetProgressBarPercent(PRInt32 percent)
{
  if (!m_runningURL) 
    return;

  nsCOMPtr <nsIMsgMailNewsUrl> mailnewsUrl = do_QueryInterface(m_runningURL);
  if (mailnewsUrl) {
    nsCOMPtr <nsIMsgStatusFeedback> feedback;
    mailnewsUrl->GetStatusFeedback(getter_AddRefs(feedback));

    if (feedback) {
      feedback->ShowProgress(percent);
    }
  }
} 

void
nsNNTPNewsgroupList::SetProgressStatus(const PRUnichar *message)
{
  if (!m_runningURL) 
    return;

  nsCOMPtr <nsIMsgMailNewsUrl> mailnewsUrl = do_QueryInterface(m_runningURL);
  if (mailnewsUrl) {
    nsCOMPtr <nsIMsgStatusFeedback> feedback;
    mailnewsUrl->GetStatusFeedback(getter_AddRefs(feedback));

    if (feedback) {
      feedback->ShowStatusString(message);
    }
  }
}     

NS_IMETHODIMP nsNNTPNewsgroupList::SetGetOldMessages(PRBool aGetOldMessages) 
{
	m_getOldMessages = aGetOldMessages;
	return NS_OK;
}

NS_IMETHODIMP nsNNTPNewsgroupList::GetGetOldMessages(PRBool *aGetOldMessages) 
{
	NS_ENSURE_ARG(aGetOldMessages);

	*aGetOldMessages = m_getOldMessages;
	return NS_OK;
}
