/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
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
#include "nsMsgCopy.h"

#include "nsXPIDLString.h"
#include "nsCOMPtr.h"
#include "nsMsgBaseCID.h"
#include "nsMsgFolderFlags.h"
#include "nsIMsgFolder.h"
#include "nsIMsgAccountManager.h"
#include "nsIMsgFolder.h"
#include "nsISupportsArray.h"
#include "nsIMsgIncomingServer.h"
#include "nsISupports.h"
#include "nsIRDFService.h"
#include "nsIRDFResource.h"
#include "nsRDFCID.h"
#include "nsIURL.h"
#include "nsNetCID.h"
#include "nsMsgComposeStringBundle.h"
#include "nsMsgCompUtils.h"
#include "prcmon.h"
#include "nsIMsgImapMailFolder.h"
#include "nsThreadUtils.h"
#include "nsMsgSimulateError.h"
#include "nsIMsgWindow.h"
#include "nsIMsgProgress.h"

static NS_DEFINE_CID(kRDFServiceCID, NS_RDFSERVICE_CID);

////////////////////////////////////////////////////////////////////////////////////
// This is the listener class for the copy operation. We have to create this class 
// to listen for message copy completion and eventually notify the caller
////////////////////////////////////////////////////////////////////////////////////
NS_IMPL_THREADSAFE_ADDREF(CopyListener)
NS_IMPL_THREADSAFE_RELEASE(CopyListener)

NS_INTERFACE_MAP_BEGIN(CopyListener)
   NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(nsISupports, nsIMsgCopyServiceListener)
   NS_INTERFACE_MAP_ENTRY(nsIMsgCopyServiceListener)
NS_INTERFACE_MAP_END_THREADSAFE

CopyListener::CopyListener(void) 
{ 
  mCopyInProgress = PR_FALSE;
}

CopyListener::~CopyListener(void) 
{
}

nsresult
CopyListener::OnStartCopy()
{
#ifdef NS_DEBUG
  printf("CopyListener::OnStartCopy()\n");
#endif

  if (mComposeAndSend)
    mComposeAndSend->NotifyListenerOnStartCopy();
  return NS_OK;
}
  
nsresult
CopyListener::OnProgress(PRUint32 aProgress, PRUint32 aProgressMax)
{
#ifdef NS_DEBUG
  printf("CopyListener::OnProgress() %d of %d\n", aProgress, aProgressMax);
#endif

  if (mComposeAndSend)
    mComposeAndSend->NotifyListenerOnProgressCopy(aProgress, aProgressMax);

  return NS_OK;
}

nsresult
CopyListener::SetMessageKey(PRUint32 aMessageKey)
{
  if (mComposeAndSend)
      mComposeAndSend->SetMessageKey(aMessageKey);
  return NS_OK;
}

nsresult
CopyListener::GetMessageId(nsCString *aMessageId)
{
  if (mComposeAndSend)
      mComposeAndSend->GetMessageId(aMessageId);
  return NS_OK;
}

nsresult
CopyListener::OnStopCopy(nsresult aStatus)
{
  if (NS_SUCCEEDED(aStatus))
  {
#ifdef NS_DEBUG
    printf("CopyListener: SUCCESSFUL ON THE COPY OPERATION!\n");
#endif
  }
  else
  {
#ifdef NS_DEBUG
    printf("CopyListener: COPY OPERATION FAILED!\n");
#endif
  }

  if (mCopyInProgress)
  {
      PR_CEnterMonitor(this);
      PR_CNotifyAll(this);
      mCopyInProgress = PR_FALSE;
      PR_CExitMonitor(this);
  }
  if (mComposeAndSend)
    mComposeAndSend->NotifyListenerOnStopCopy(aStatus);

  return NS_OK;
}

nsresult
CopyListener::SetMsgComposeAndSendObject(nsIMsgSend *obj)
{
  if (obj)
    mComposeAndSend = obj;

  return NS_OK;
}

////////////////////////////////////////////////////////////////////////////////////
// END  END  END  END  END  END  END  END  END  END  END  END  END  END  END 
// This is the listener class for the copy operation. We have to create this class 
// to listen for message copy completion and eventually notify the caller
////////////////////////////////////////////////////////////////////////////////////

NS_IMPL_ISUPPORTS1(nsMsgCopy, nsIUrlListener)

nsMsgCopy::nsMsgCopy()
{
  mFileSpec = nsnull;
  mMode = nsIMsgSend::nsMsgDeliverNow;
  mSavePref = nsnull;
}

nsMsgCopy::~nsMsgCopy()
{
  PR_Free(mSavePref);
}

nsresult
nsMsgCopy::StartCopyOperation(nsIMsgIdentity       *aUserIdentity,
                              nsIFileSpec          *aFileSpec, 
                              nsMsgDeliverMode     aMode,
                              nsIMsgSend           *aMsgSendObj,
                              const char           *aSavePref,
                              nsIMsgDBHdr            *aMsgToReplace)
{
  nsCOMPtr<nsIMsgFolder>  dstFolder;
  PRBool                  isDraft = PR_FALSE;
  PRBool                  waitForUrl = PR_FALSE;
  nsresult                rv;

  if (!aMsgSendObj)
    return NS_ERROR_INVALID_ARG;

  // Store away the server location...
  if (aSavePref)
    mSavePref = PL_strdup(aSavePref);

  //
  // Vars for implementation...
  //
  if (aMode == nsIMsgSend::nsMsgQueueForLater)       // QueueForLater (Outbox)
  {
    rv = GetUnsentMessagesFolder(aUserIdentity, getter_AddRefs(dstFolder), &waitForUrl);
    isDraft = PR_FALSE;
    if (!dstFolder || NS_FAILED(rv)) {
      return NS_MSG_UNABLE_TO_SEND_LATER;
    } 
  }
  else if (aMode == nsIMsgSend::nsMsgSaveAsDraft)    // SaveAsDraft (Drafts)
  {
    rv = GetDraftsFolder(aUserIdentity, getter_AddRefs(dstFolder), &waitForUrl);
    isDraft = PR_TRUE;
    if (!dstFolder || NS_FAILED(rv))
      return NS_MSG_UNABLE_TO_SAVE_DRAFT;
  }
  else if (aMode == nsIMsgSend::nsMsgSaveAsTemplate) // SaveAsTemplate (Templates)
  {
    rv = GetTemplatesFolder(aUserIdentity, getter_AddRefs(dstFolder), &waitForUrl);
    isDraft = PR_FALSE;
    if (!dstFolder || NS_FAILED(rv) || CHECK_SIMULATED_ERROR(SIMULATED_SEND_ERROR_5))
	    return NS_MSG_UNABLE_TO_SAVE_TEMPLATE;
  }
  else // SaveInSentFolder (Sent) -  nsMsgDeliverNow or nsMsgSendUnsent
  {
    rv = GetSentFolder(aUserIdentity, getter_AddRefs(dstFolder), &waitForUrl);
    isDraft = PR_FALSE;
    if (!dstFolder || NS_FAILED(rv)) 
      return NS_MSG_COULDNT_OPEN_FCC_FOLDER;
  }

  nsCOMPtr <nsIMsgWindow> msgWindow;

  if (aMsgSendObj)
  {
    nsCOMPtr <nsIMsgProgress> progress;
    aMsgSendObj->GetProgress(getter_AddRefs(progress));
    if (progress)
      progress->GetMsgWindow(getter_AddRefs(msgWindow));
  }

  mMode = aMode;
  mFileSpec = aFileSpec;
  mDstFolder = dstFolder;
  mMsgToReplace = aMsgToReplace;
  mIsDraft = isDraft;
  mMsgSendObj = aMsgSendObj;
  if (!waitForUrl)
  {
    // cache info needed for DoCopy and call DoCopy when OnStopUrl is called.
    rv = DoCopy(aFileSpec, dstFolder, aMsgToReplace, isDraft, msgWindow, aMsgSendObj);
    // N.B. "this" may be deleted when this call returns.
  }
  return rv;
}

nsresult 
nsMsgCopy::DoCopy(nsIFileSpec *aDiskFile, nsIMsgFolder *dstFolder,
                  nsIMsgDBHdr *aMsgToReplace, PRBool aIsDraft,
                  nsIMsgWindow *msgWindow,
                  nsIMsgSend   *aMsgSendObj)
{
  nsresult rv = NS_OK;

  // Check sanity
  if ((!aDiskFile) || (!dstFolder))
    return NS_ERROR_INVALID_ARG;

  //Call copyservice with dstFolder, disk file, and txnManager
  if(NS_SUCCEEDED(rv))
  {
    nsRefPtr<CopyListener> copyListener = new CopyListener();
    if (!copyListener)
      return NS_ERROR_OUT_OF_MEMORY;

    copyListener->SetMsgComposeAndSendObject(aMsgSendObj);
    nsIThread *thread = nsnull;

    if (aIsDraft)
    {
        nsCOMPtr<nsIMsgImapMailFolder> imapFolder =
            do_QueryInterface(dstFolder);
        nsCOMPtr<nsIMsgAccountManager> accountManager = 
                 do_GetService(NS_MSGACCOUNTMANAGER_CONTRACTID, &rv);
        if (NS_FAILED(rv)) return rv;
        PRBool shutdownInProgress = PR_FALSE;
        rv = accountManager->GetShutdownInProgress(&shutdownInProgress);
        
        if (NS_SUCCEEDED(rv) && shutdownInProgress && imapFolder)
        { 
          // set the following only when we were in the middle of shutdown
          // process
            copyListener->mCopyInProgress = PR_TRUE;
            thread = NS_GetCurrentThread();
        }
    }
    nsCOMPtr<nsIMsgCopyService> copyService = do_GetService(NS_MSGCOPYSERVICE_CONTRACTID, &rv);
    NS_ENSURE_SUCCESS(rv, rv);

    rv = copyService->CopyFileMessage(aDiskFile, dstFolder, aMsgToReplace, 
                                      aIsDraft, MSG_FLAG_READ, copyListener, msgWindow);
    // copyListener->mCopyInProgress can only be set when we are in the
    // middle of the shutdown process
    while (copyListener->mCopyInProgress)
    {
        PR_CEnterMonitor(copyListener);
        PR_CWait(copyListener, PR_MicrosecondsToInterval(1000UL));
        PR_CExitMonitor(copyListener);
        if (thread)
            NS_ProcessPendingEvents(thread);
    }
  }

  return rv;
}

// nsIUrlListener methods
NS_IMETHODIMP
nsMsgCopy::OnStartRunningUrl(nsIURI * aUrl)
{
    return NS_OK;
}

NS_IMETHODIMP
nsMsgCopy::OnStopRunningUrl(nsIURI * aUrl, nsresult aExitCode)
{
  nsresult rv = aExitCode;
  if (NS_SUCCEEDED(aExitCode))
  {
    rv = DoCopy(mFileSpec, mDstFolder, mMsgToReplace, mIsDraft, nsnull, mMsgSendObj);
  }
  return rv;
}

nsresult
nsMsgCopy::GetUnsentMessagesFolder(nsIMsgIdentity   *userIdentity, nsIMsgFolder **folder, PRBool *waitForUrl)
{
  nsresult ret = LocateMessageFolder(userIdentity, nsIMsgSend::nsMsgQueueForLater, mSavePref, folder);
  CreateIfMissing(folder, waitForUrl);
  return ret;
}
 
nsresult 
nsMsgCopy::GetDraftsFolder(nsIMsgIdentity *userIdentity, nsIMsgFolder **folder, PRBool *waitForUrl)
{
  nsresult ret = LocateMessageFolder(userIdentity, nsIMsgSend::nsMsgSaveAsDraft, mSavePref, folder);
  CreateIfMissing(folder, waitForUrl);
  return ret;
}

nsresult 
nsMsgCopy::GetTemplatesFolder(nsIMsgIdentity *userIdentity, nsIMsgFolder **folder, PRBool *waitForUrl)
{
  nsresult ret = LocateMessageFolder(userIdentity, nsIMsgSend::nsMsgSaveAsTemplate, mSavePref, folder);
  CreateIfMissing(folder, waitForUrl);
  return ret;
}

nsresult 
nsMsgCopy::GetSentFolder(nsIMsgIdentity *userIdentity, nsIMsgFolder **folder, PRBool *waitForUrl)
{
  nsresult ret = LocateMessageFolder(userIdentity, nsIMsgSend::nsMsgDeliverNow, mSavePref, folder);
  CreateIfMissing(folder, waitForUrl);
  return ret;
}

nsresult 
nsMsgCopy::CreateIfMissing(nsIMsgFolder **folder, PRBool *waitForUrl)
{
  nsresult ret = NS_OK;
  if (folder && *folder)
  {
    nsCOMPtr <nsIMsgFolder> parent;
    (*folder)->GetParent(getter_AddRefs(parent));
    if (!parent)
    {
      nsCOMPtr <nsIFileSpec> folderPath;
      // for local folders, path is to the berkeley mailbox. 
      // for imap folders, path needs to have .msf appended to the name
      (*folder)->GetPath(getter_AddRefs(folderPath));
        PRBool isImapFolder = !nsCRT::strncasecmp(mSavePref, "imap:", 5);
      // if we can't get the path from the folder, then try to create the storage.
      // for imap, it doesn't matter if the .msf file exists - it still might not
      // exist on the server, so we should try to create it
      PRBool exists = PR_FALSE;
      if (!isImapFolder && folderPath)
        folderPath->Exists(&exists);
        if (!exists)
        {
          (*folder)->CreateStorageIfMissing(this);
          if (isImapFolder)
            *waitForUrl = PR_TRUE;
 
          ret = NS_OK;
        }
      }
    }
  return ret;
}
////////////////////////////////////////////////////////////////////////////////////
// Utility Functions for MsgFolders
////////////////////////////////////////////////////////////////////////////////////
nsresult
LocateMessageFolder(nsIMsgIdentity   *userIdentity, 
                    nsMsgDeliverMode aFolderType,
                    const char       *aFolderURI,
                    nsIMsgFolder     **msgFolder)
{
  nsresult                  rv = NS_OK;

  RETURN_SIMULATED_ERROR(SIMULATED_SEND_ERROR_5, NS_ERROR_FAILURE)
  
  if (!msgFolder) return NS_ERROR_NULL_POINTER;
  *msgFolder = nsnull;

  if (!aFolderURI || !*aFolderURI)
    return NS_ERROR_INVALID_ARG;

  // as long as it doesn't start with anyfolder://
  if (PL_strncasecmp(ANY_SERVER, aFolderURI, strlen(aFolderURI)) != 0)
  {
    nsCOMPtr<nsIRDFService> rdf(do_GetService(kRDFServiceCID, &rv));
    if (NS_FAILED(rv)) return rv;

    // get the corresponding RDF resource
    // RDF will create the folder resource if it doesn't already exist
    nsCOMPtr<nsIRDFResource> resource;
    rv = rdf->GetResource(nsDependentCString(aFolderURI), getter_AddRefs(resource));
    if (NS_FAILED(rv)) return rv;

    nsCOMPtr <nsIMsgFolder> folderResource;
    folderResource = do_QueryInterface(resource, &rv);
    if (NS_SUCCEEDED(rv) && folderResource) 
    {
      // don't check validity of folder - caller will handle creating it
      nsCOMPtr<nsIMsgIncomingServer> server; 
      //make sure that folder hierarchy is built so that legitimate parent-child relationship is established
      rv = folderResource->GetServer(getter_AddRefs(server));
      NS_ENSURE_SUCCESS(rv,rv);
      return server->GetMsgFolderFromURI(folderResource, aFolderURI, msgFolder);
    }
    else 
    {
      return NS_ERROR_FAILURE;
    }
  }
  else 
  {
    PRUint32                  cnt = 0;
    PRUint32                  i;

    if (!userIdentity)
      return NS_ERROR_INVALID_ARG;

    // get the account manager
    nsCOMPtr<nsIMsgAccountManager> accountManager = 
             do_GetService(NS_MSGACCOUNTMANAGER_CONTRACTID, &rv);
    if (NS_FAILED(rv)) return rv;

    // if anyfolder will do, go look for one.
    nsCOMPtr<nsISupportsArray> retval; 
    accountManager->GetServersForIdentity(userIdentity, getter_AddRefs(retval)); 
    if (!retval) return NS_ERROR_FAILURE;
    
    // Ok, we have to look through the servers and try to find the server that
    // has a valid folder of the type that interests us...
    rv = retval->Count(&cnt);
    if (NS_FAILED(rv)) return rv;
    
    for (i=0; i<cnt; i++) {
      // Now that we have the server...we need to get the named message folder
      nsCOMPtr<nsIMsgIncomingServer> inServer; 
      
      inServer = do_QueryElementAt(retval, i, &rv);
      if(NS_FAILED(rv) || (!inServer))
        continue;
      
      //
      // If aFolderURI is passed in, then the user has chosen a specific
      // mail folder to save the message, but if it is null, just find the
      // first one and make that work. The folder is specified as a URI, like
      // the following:
      //
      //		  mailbox://nobody@Local Folders/Sent
      //                  imap://rhp@nsmail-2/Drafts
      //                  newsgroup://news.mozilla.org/netscape.test
      //
      char *serverURI = nsnull;
      rv = inServer->GetServerURI(&serverURI);
      if ( NS_FAILED(rv) || (!serverURI) || !(*serverURI) )
        continue;
      
      nsCOMPtr<nsIMsgFolder> rootFolder;
      rv = inServer->GetRootFolder(getter_AddRefs(rootFolder));
      
      if(NS_FAILED(rv) || (!rootFolder))
        continue;
      
      PRUint32 numFolders = 0;
      
      // use the defaults by getting the folder by flags
      if (aFolderType == nsIMsgSend::nsMsgQueueForLater)       // QueueForLater (Outbox)
        {
          rv = rootFolder->GetFoldersWithFlag(MSG_FOLDER_FLAG_QUEUE, 1, &numFolders, msgFolder);
        }
      else if (aFolderType == nsIMsgSend::nsMsgSaveAsDraft)    // SaveAsDraft (Drafts)
        {
          rv = rootFolder->GetFoldersWithFlag(MSG_FOLDER_FLAG_DRAFTS, 1, &numFolders, msgFolder);
        }
      else if (aFolderType == nsIMsgSend::nsMsgSaveAsTemplate) // SaveAsTemplate (Templates)
        {
          rv = rootFolder->GetFoldersWithFlag(MSG_FOLDER_FLAG_TEMPLATES, 1, &numFolders, msgFolder);
        }
      else // SaveInSentFolder (Sent) -  nsMsgDeliverNow or nsMsgSendUnsent
        {
          rv = rootFolder->GetFoldersWithFlag(MSG_FOLDER_FLAG_SENTMAIL, 1, &numFolders, msgFolder);
        }

      if (NS_SUCCEEDED(rv) && *msgFolder) {
	return NS_OK;
      }
    }
  }
  
  return NS_ERROR_FAILURE;
}

//
// Figure out if a folder is local or not and return a boolean to 
// say so.
//
nsresult
MessageFolderIsLocal(nsIMsgIdentity   *userIdentity, 
                     nsMsgDeliverMode aFolderType,
                     const char       *aFolderURI,
		     PRBool 	      *aResult)
{
  nsresult rv;

  if (!aFolderURI) return NS_ERROR_NULL_POINTER;

  nsCOMPtr <nsIURL> url = do_CreateInstance(NS_STANDARDURL_CONTRACTID, &rv);
  if (NS_FAILED(rv)) return rv;

  rv = url->SetSpec(nsDependentCString(aFolderURI));
  if (NS_FAILED(rv)) return rv;
 
  /* mailbox:/ means its local (on disk) */
  rv = url->SchemeIs("mailbox", aResult);
  if (NS_FAILED(rv)) return rv;
  return NS_OK;
}

