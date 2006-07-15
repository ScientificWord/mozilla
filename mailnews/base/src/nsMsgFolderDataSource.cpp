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
 * Portions created by the Initial Developer are Copyright (C) 1999
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Pierre Phaneuf <pp@ludusdesign.com>
 *   David Bienvenu <bienvenu@nventure.com>
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

#include "msgCore.h"    // precompiled header...
#include "prlog.h"

#include "nsMsgFolderDataSource.h"
#include "nsMsgFolderFlags.h"

#include "nsMsgRDFUtils.h"

#include "rdf.h"
#include "nsIRDFService.h"
#include "nsRDFCID.h"
#include "nsIRDFNode.h"
#include "nsEnumeratorUtils.h"
#include "nsAdapterEnumerator.h"

#include "nsString.h"
#include "nsCOMPtr.h"
#include "nsXPIDLString.h"
#include "nsReadableUtils.h"

#include "nsIMsgMailSession.h"
#include "nsIMsgCopyService.h"
#include "nsMsgBaseCID.h"
#include "nsIInputStream.h"
#include "nsIMsgHdr.h"
#include "nsTraceRefcnt.h"
#include "nsIMsgFolder.h" // TO include biffState enum. Change to bool later...
#include "nsIMutableArray.h"
#include "nsIPop3IncomingServer.h"
#include "nsINntpIncomingServer.h"
#include "nsTextFormatter.h"
#include "nsIStringBundle.h"
#include "nsIPrompt.h"
#include "nsIMsgAccountManager.h"

#define MESSENGER_STRING_URL       "chrome://messenger/locale/messenger.properties"

nsIRDFResource* nsMsgFolderDataSource::kNC_Child = nsnull;
nsIRDFResource* nsMsgFolderDataSource::kNC_Folder= nsnull;
nsIRDFResource* nsMsgFolderDataSource::kNC_Name= nsnull;
nsIRDFResource* nsMsgFolderDataSource::kNC_Open = nsnull;
nsIRDFResource* nsMsgFolderDataSource::kNC_FolderTreeName= nsnull;
nsIRDFResource* nsMsgFolderDataSource::kNC_FolderTreeSimpleName= nsnull;
nsIRDFResource* nsMsgFolderDataSource::kNC_NameSort= nsnull;
nsIRDFResource* nsMsgFolderDataSource::kNC_FolderTreeNameSort= nsnull;
nsIRDFResource* nsMsgFolderDataSource::kNC_SpecialFolder= nsnull;
nsIRDFResource* nsMsgFolderDataSource::kNC_ServerType = nsnull;
nsIRDFResource* nsMsgFolderDataSource::kNC_IsDeferred = nsnull;
nsIRDFResource* nsMsgFolderDataSource::kNC_RedirectorType = nsnull;
nsIRDFResource* nsMsgFolderDataSource::kNC_CanCreateFoldersOnServer = nsnull;
nsIRDFResource* nsMsgFolderDataSource::kNC_CanFileMessagesOnServer = nsnull;
nsIRDFResource* nsMsgFolderDataSource::kNC_IsServer = nsnull;
nsIRDFResource* nsMsgFolderDataSource::kNC_IsSecure = nsnull;
nsIRDFResource* nsMsgFolderDataSource::kNC_CanSubscribe = nsnull;
nsIRDFResource* nsMsgFolderDataSource::kNC_SupportsOffline = nsnull;
nsIRDFResource* nsMsgFolderDataSource::kNC_CanFileMessages = nsnull;
nsIRDFResource* nsMsgFolderDataSource::kNC_CanCreateSubfolders = nsnull;
nsIRDFResource* nsMsgFolderDataSource::kNC_CanRename = nsnull;
nsIRDFResource* nsMsgFolderDataSource::kNC_CanCompact = nsnull;
nsIRDFResource* nsMsgFolderDataSource::kNC_TotalMessages= nsnull;
nsIRDFResource* nsMsgFolderDataSource::kNC_TotalUnreadMessages= nsnull;
nsIRDFResource* nsMsgFolderDataSource::kNC_FolderSize = nsnull;
nsIRDFResource* nsMsgFolderDataSource::kNC_Charset = nsnull;
nsIRDFResource* nsMsgFolderDataSource::kNC_BiffState = nsnull;
nsIRDFResource* nsMsgFolderDataSource::kNC_HasUnreadMessages = nsnull;
nsIRDFResource* nsMsgFolderDataSource::kNC_NewMessages = nsnull;
nsIRDFResource* nsMsgFolderDataSource::kNC_SubfoldersHaveUnreadMessages = nsnull;
nsIRDFResource* nsMsgFolderDataSource::kNC_NoSelect = nsnull;
nsIRDFResource* nsMsgFolderDataSource::kNC_VirtualFolder = nsnull;
nsIRDFResource* nsMsgFolderDataSource::kNC_InVFEditSearchScope = nsnull; 
nsIRDFResource* nsMsgFolderDataSource::kNC_ImapShared = nsnull;
nsIRDFResource* nsMsgFolderDataSource::kNC_Synchronize = nsnull;
nsIRDFResource* nsMsgFolderDataSource::kNC_SyncDisabled = nsnull;
nsIRDFResource* nsMsgFolderDataSource::kNC_CanSearchMessages = nsnull;
nsIRDFResource* nsMsgFolderDataSource::kNC_UnreadFolders = nsnull;
nsIRDFResource* nsMsgFolderDataSource::kNC_FavoriteFolders = nsnull;
nsIRDFResource* nsMsgFolderDataSource::kNC_RecentFolders = nsnull;

// commands
nsIRDFResource* nsMsgFolderDataSource::kNC_Delete= nsnull;
nsIRDFResource* nsMsgFolderDataSource::kNC_ReallyDelete= nsnull;
nsIRDFResource* nsMsgFolderDataSource::kNC_NewFolder= nsnull;
nsIRDFResource* nsMsgFolderDataSource::kNC_GetNewMessages= nsnull;
nsIRDFResource* nsMsgFolderDataSource::kNC_Copy= nsnull;
nsIRDFResource* nsMsgFolderDataSource::kNC_Move= nsnull;
nsIRDFResource* nsMsgFolderDataSource::kNC_CopyFolder= nsnull;
nsIRDFResource* nsMsgFolderDataSource::kNC_MoveFolder= nsnull;
nsIRDFResource* nsMsgFolderDataSource::kNC_MarkAllMessagesRead= nsnull;
nsIRDFResource* nsMsgFolderDataSource::kNC_Compact= nsnull;
nsIRDFResource* nsMsgFolderDataSource::kNC_CompactAll= nsnull;
nsIRDFResource* nsMsgFolderDataSource::kNC_Rename= nsnull;
nsIRDFResource* nsMsgFolderDataSource::kNC_EmptyTrash= nsnull;
nsIRDFResource* nsMsgFolderDataSource::kNC_DownloadFlagged= nsnull;

nsrefcnt nsMsgFolderDataSource::gFolderResourceRefCnt = 0;

nsIAtom * nsMsgFolderDataSource::kBiffStateAtom = nsnull;
nsIAtom * nsMsgFolderDataSource::kNewMessagesAtom = nsnull;
nsIAtom * nsMsgFolderDataSource::kTotalMessagesAtom = nsnull;
nsIAtom * nsMsgFolderDataSource::kTotalUnreadMessagesAtom = nsnull;
nsIAtom * nsMsgFolderDataSource::kFolderSizeAtom = nsnull;
nsIAtom * nsMsgFolderDataSource::kNameAtom = nsnull;
nsIAtom * nsMsgFolderDataSource::kSynchronizeAtom = nsnull;
nsIAtom * nsMsgFolderDataSource::kOpenAtom = nsnull;
nsIAtom * nsMsgFolderDataSource::kIsDeferredAtom = nsnull;
nsIAtom * nsMsgFolderDataSource::kIsSecureAtom = nsnull;
nsIAtom * nsMsgFolderDataSource::kCanFileMessagesAtom = nsnull;
nsIAtom * nsMsgFolderDataSource::kInVFEditSearchScopeAtom = nsnull;

PRUnichar * nsMsgFolderDataSource::kKiloByteString = nsnull;
PRUnichar * nsMsgFolderDataSource::kMegaByteString = nsnull;

static const PRUint32 kDisplayBlankCount = 0xFFFFFFFE;
static const PRUint32 kDisplayQuestionCount = 0xFFFFFFFF;

nsMsgFolderDataSource::nsMsgFolderDataSource()
{
  // one-time initialization here
  nsIRDFService* rdf = getRDFService();
  
  if (gFolderResourceRefCnt++ == 0) {
    nsresult res = NS_OK;
    nsCOMPtr<nsIStringBundle> sMessengerStringBundle;

    rdf->GetResource(NS_LITERAL_CSTRING(NC_RDF_CHILD),   &kNC_Child);
    rdf->GetResource(NS_LITERAL_CSTRING(NC_RDF_FOLDER),  &kNC_Folder);
    rdf->GetResource(NS_LITERAL_CSTRING(NC_RDF_NAME),    &kNC_Name);
    rdf->GetResource(NS_LITERAL_CSTRING(NC_RDF_OPEN),    &kNC_Open);
    rdf->GetResource(NS_LITERAL_CSTRING(NC_RDF_FOLDERTREENAME),    &kNC_FolderTreeName);
    rdf->GetResource(NS_LITERAL_CSTRING("mailnewsunreadfolders:/"),    &kNC_UnreadFolders);
    rdf->GetResource(NS_LITERAL_CSTRING("mailnewsfavefolders:/"),    &kNC_FavoriteFolders);
    rdf->GetResource(NS_LITERAL_CSTRING("mailnewsrecentfolders:/"),    &kNC_RecentFolders);

    rdf->GetResource(NS_LITERAL_CSTRING(NC_RDF_FOLDERTREESIMPLENAME),    &kNC_FolderTreeSimpleName);
    rdf->GetResource(NS_LITERAL_CSTRING(NC_RDF_NAME_SORT),    &kNC_NameSort);
    rdf->GetResource(NS_LITERAL_CSTRING(NC_RDF_FOLDERTREENAME_SORT),    &kNC_FolderTreeNameSort);
    rdf->GetResource(NS_LITERAL_CSTRING(NC_RDF_SPECIALFOLDER), &kNC_SpecialFolder);
    rdf->GetResource(NS_LITERAL_CSTRING(NC_RDF_SERVERTYPE), &kNC_ServerType);
    rdf->GetResource(NS_LITERAL_CSTRING(NC_RDF_ISDEFERRED),&kNC_IsDeferred);
    rdf->GetResource(NS_LITERAL_CSTRING(NC_RDF_REDIRECTORTYPE), &kNC_RedirectorType);
    rdf->GetResource(NS_LITERAL_CSTRING(NC_RDF_CANCREATEFOLDERSONSERVER), &kNC_CanCreateFoldersOnServer);
    rdf->GetResource(NS_LITERAL_CSTRING(NC_RDF_CANFILEMESSAGESONSERVER), &kNC_CanFileMessagesOnServer);
    rdf->GetResource(NS_LITERAL_CSTRING(NC_RDF_ISSERVER), &kNC_IsServer);
    rdf->GetResource(NS_LITERAL_CSTRING(NC_RDF_ISSECURE), &kNC_IsSecure);
    rdf->GetResource(NS_LITERAL_CSTRING(NC_RDF_CANSUBSCRIBE), &kNC_CanSubscribe);
    rdf->GetResource(NS_LITERAL_CSTRING(NC_RDF_SUPPORTSOFFLINE), &kNC_SupportsOffline);
    rdf->GetResource(NS_LITERAL_CSTRING(NC_RDF_CANFILEMESSAGES), &kNC_CanFileMessages);
    rdf->GetResource(NS_LITERAL_CSTRING(NC_RDF_CANCREATESUBFOLDERS), &kNC_CanCreateSubfolders);
    rdf->GetResource(NS_LITERAL_CSTRING(NC_RDF_CANRENAME), &kNC_CanRename);
    rdf->GetResource(NS_LITERAL_CSTRING(NC_RDF_CANCOMPACT), &kNC_CanCompact);
    rdf->GetResource(NS_LITERAL_CSTRING(NC_RDF_TOTALMESSAGES), &kNC_TotalMessages);
    rdf->GetResource(NS_LITERAL_CSTRING(NC_RDF_TOTALUNREADMESSAGES), &kNC_TotalUnreadMessages);
    rdf->GetResource(NS_LITERAL_CSTRING(NC_RDF_FOLDERSIZE), &kNC_FolderSize);
    rdf->GetResource(NS_LITERAL_CSTRING(NC_RDF_CHARSET), &kNC_Charset);
    rdf->GetResource(NS_LITERAL_CSTRING(NC_RDF_BIFFSTATE), &kNC_BiffState);
    rdf->GetResource(NS_LITERAL_CSTRING(NC_RDF_HASUNREADMESSAGES), &kNC_HasUnreadMessages);
    rdf->GetResource(NS_LITERAL_CSTRING(NC_RDF_NEWMESSAGES), &kNC_NewMessages);
    rdf->GetResource(NS_LITERAL_CSTRING(NC_RDF_SUBFOLDERSHAVEUNREADMESSAGES), &kNC_SubfoldersHaveUnreadMessages);
    rdf->GetResource(NS_LITERAL_CSTRING(NC_RDF_NOSELECT), &kNC_NoSelect);
    rdf->GetResource(NS_LITERAL_CSTRING(NC_RDF_VIRTUALFOLDER), &kNC_VirtualFolder);
    rdf->GetResource(NS_LITERAL_CSTRING(NC_RDF_INVFEDITSEARCHSCOPE), &kNC_InVFEditSearchScope);    
    rdf->GetResource(NS_LITERAL_CSTRING(NC_RDF_IMAPSHARED), &kNC_ImapShared);
    rdf->GetResource(NS_LITERAL_CSTRING(NC_RDF_SYNCHRONIZE), &kNC_Synchronize);
    rdf->GetResource(NS_LITERAL_CSTRING(NC_RDF_SYNCDISABLED), &kNC_SyncDisabled);
    rdf->GetResource(NS_LITERAL_CSTRING(NC_RDF_CANSEARCHMESSAGES), &kNC_CanSearchMessages);
    
    rdf->GetResource(NS_LITERAL_CSTRING(NC_RDF_DELETE), &kNC_Delete);
    rdf->GetResource(NS_LITERAL_CSTRING(NC_RDF_REALLY_DELETE), &kNC_ReallyDelete);
    rdf->GetResource(NS_LITERAL_CSTRING(NC_RDF_NEWFOLDER), &kNC_NewFolder);
    rdf->GetResource(NS_LITERAL_CSTRING(NC_RDF_GETNEWMESSAGES), &kNC_GetNewMessages);
    rdf->GetResource(NS_LITERAL_CSTRING(NC_RDF_COPY), &kNC_Copy);
    rdf->GetResource(NS_LITERAL_CSTRING(NC_RDF_MOVE), &kNC_Move);
    rdf->GetResource(NS_LITERAL_CSTRING(NC_RDF_COPYFOLDER), &kNC_CopyFolder);
    rdf->GetResource(NS_LITERAL_CSTRING(NC_RDF_MOVEFOLDER), &kNC_MoveFolder);
    rdf->GetResource(NS_LITERAL_CSTRING(NC_RDF_MARKALLMESSAGESREAD),
                             &kNC_MarkAllMessagesRead);
    rdf->GetResource(NS_LITERAL_CSTRING(NC_RDF_COMPACT), &kNC_Compact);
    rdf->GetResource(NS_LITERAL_CSTRING(NC_RDF_COMPACTALL), &kNC_CompactAll);
    rdf->GetResource(NS_LITERAL_CSTRING(NC_RDF_RENAME), &kNC_Rename);
    rdf->GetResource(NS_LITERAL_CSTRING(NC_RDF_EMPTYTRASH), &kNC_EmptyTrash);
    rdf->GetResource(NS_LITERAL_CSTRING(NC_RDF_DOWNLOADFLAGGED), &kNC_DownloadFlagged);

    kTotalMessagesAtom           = NS_NewAtom("TotalMessages");
    kTotalUnreadMessagesAtom     = NS_NewAtom("TotalUnreadMessages");
    kFolderSizeAtom              = NS_NewAtom("FolderSize");
    kBiffStateAtom               = NS_NewAtom("BiffState");
    kNewMessagesAtom             = NS_NewAtom("NewMessages");
    kNameAtom                    = NS_NewAtom("Name");
    kSynchronizeAtom             = NS_NewAtom("Synchronize");
    kOpenAtom                    = NS_NewAtom("open");
    kIsDeferredAtom              = NS_NewAtom("isDeferred");
    kIsSecureAtom                = NS_NewAtom("isSecure");
    kCanFileMessagesAtom         = NS_NewAtom("canFileMessages");
    kInVFEditSearchScopeAtom     = NS_NewAtom("inVFEditSearchScope");

    nsCOMPtr<nsIStringBundleService> sBundleService = do_GetService(NS_STRINGBUNDLE_CONTRACTID, &res);

    if (NS_SUCCEEDED(res) && sBundleService) 
      res = sBundleService->CreateBundle(MESSENGER_STRING_URL, getter_AddRefs(sMessengerStringBundle));

    if (NS_SUCCEEDED(res) && sMessengerStringBundle)
    {
      if (!NS_SUCCEEDED(sMessengerStringBundle->GetStringFromName(NS_LITERAL_STRING("kiloByteAbbreviation").get(), &kKiloByteString)))
        kKiloByteString = ToNewUnicode(NS_LITERAL_STRING("kiloByteAbbreviation"));

      if (!NS_SUCCEEDED(sMessengerStringBundle->GetStringFromName(NS_LITERAL_STRING("megaByteAbbreviation").get(), &kMegaByteString)))
        kMegaByteString = ToNewUnicode(NS_LITERAL_STRING("megaByteAbbreviation"));
    }
  }
  
  CreateLiterals(rdf);
  CreateArcsOutEnumerator();
}

nsMsgFolderDataSource::~nsMsgFolderDataSource (void)
{
  if (--gFolderResourceRefCnt == 0)
  {
    nsrefcnt refcnt;
    NS_RELEASE2(kNC_Child, refcnt);
    NS_RELEASE2(kNC_Folder, refcnt);
    NS_RELEASE2(kNC_Name, refcnt);
    NS_RELEASE2(kNC_Open, refcnt);
    NS_RELEASE2(kNC_FolderTreeName, refcnt);
    NS_RELEASE2(kNC_FolderTreeSimpleName, refcnt);
    NS_RELEASE2(kNC_NameSort, refcnt);
    NS_RELEASE2(kNC_FolderTreeNameSort, refcnt);
    NS_RELEASE2(kNC_SpecialFolder, refcnt);
    NS_RELEASE2(kNC_ServerType, refcnt);
    NS_RELEASE2(kNC_IsDeferred, refcnt);
    NS_RELEASE2(kNC_RedirectorType, refcnt);
    NS_RELEASE2(kNC_CanCreateFoldersOnServer, refcnt);
    NS_RELEASE2(kNC_CanFileMessagesOnServer, refcnt);
    NS_RELEASE2(kNC_IsServer, refcnt);
    NS_RELEASE2(kNC_IsSecure, refcnt);
    NS_RELEASE2(kNC_CanSubscribe, refcnt);
    NS_RELEASE2(kNC_SupportsOffline, refcnt);
    NS_RELEASE2(kNC_CanFileMessages, refcnt);
    NS_RELEASE2(kNC_CanCreateSubfolders, refcnt);
    NS_RELEASE2(kNC_CanRename, refcnt);
    NS_RELEASE2(kNC_CanCompact, refcnt);
    NS_RELEASE2(kNC_TotalMessages, refcnt);
    NS_RELEASE2(kNC_TotalUnreadMessages, refcnt);
    NS_RELEASE2(kNC_FolderSize, refcnt);
    NS_RELEASE2(kNC_Charset, refcnt);
    NS_RELEASE2(kNC_BiffState, refcnt);
    NS_RELEASE2(kNC_HasUnreadMessages, refcnt);
    NS_RELEASE2(kNC_NewMessages, refcnt);
    NS_RELEASE2(kNC_SubfoldersHaveUnreadMessages, refcnt);
    NS_RELEASE2(kNC_NoSelect, refcnt);
    NS_RELEASE2(kNC_VirtualFolder, refcnt);
    NS_RELEASE2(kNC_InVFEditSearchScope, refcnt);
    NS_RELEASE2(kNC_ImapShared, refcnt);
    NS_RELEASE2(kNC_Synchronize, refcnt);
    NS_RELEASE2(kNC_SyncDisabled, refcnt);
    NS_RELEASE2(kNC_CanSearchMessages, refcnt);
    
    NS_RELEASE2(kNC_Delete, refcnt);
    NS_RELEASE2(kNC_ReallyDelete, refcnt);
    NS_RELEASE2(kNC_NewFolder, refcnt);
    NS_RELEASE2(kNC_GetNewMessages, refcnt);
    NS_RELEASE2(kNC_Copy, refcnt);
    NS_RELEASE2(kNC_Move, refcnt);
    NS_RELEASE2(kNC_CopyFolder, refcnt);
    NS_RELEASE2(kNC_MoveFolder, refcnt);
    NS_RELEASE2(kNC_MarkAllMessagesRead, refcnt);
    NS_RELEASE2(kNC_Compact, refcnt);
    NS_RELEASE2(kNC_CompactAll, refcnt);
    NS_RELEASE2(kNC_Rename, refcnt);
    NS_RELEASE2(kNC_EmptyTrash, refcnt);
    NS_RELEASE2(kNC_DownloadFlagged, refcnt);
    
    NS_RELEASE(kTotalMessagesAtom);
    NS_RELEASE(kTotalUnreadMessagesAtom);
    NS_RELEASE(kFolderSizeAtom);
    NS_RELEASE(kBiffStateAtom);
    NS_RELEASE(kNewMessagesAtom);
    NS_RELEASE(kNameAtom);
    NS_RELEASE(kSynchronizeAtom);
    NS_RELEASE(kOpenAtom);
    NS_RELEASE(kIsDeferredAtom);
    NS_RELEASE(kIsSecureAtom);
    NS_RELEASE(kCanFileMessagesAtom);
    NS_RELEASE(kInVFEditSearchScopeAtom);

    nsMemory::Free(kKiloByteString);
    nsMemory::Free(kMegaByteString);
  }
}

nsresult nsMsgFolderDataSource::CreateLiterals(nsIRDFService *rdf)
{
  createNode(NS_LITERAL_STRING("true").get(),
    getter_AddRefs(kTrueLiteral), rdf);
  createNode(NS_LITERAL_STRING("false").get(),
    getter_AddRefs(kFalseLiteral), rdf);

  return NS_OK;
}

nsresult nsMsgFolderDataSource::Init()
{
  nsresult rv;
  
  rv = nsMsgRDFDataSource::Init();
  if (NS_FAILED(rv))
    return rv;

  nsCOMPtr<nsIMsgMailSession> mailSession = 
    do_GetService(NS_MSGMAILSESSION_CONTRACTID, &rv); 

  if(NS_SUCCEEDED(rv))
    mailSession->AddFolderListener(this, 
      nsIFolderListener::added |
      nsIFolderListener::removed |
      nsIFolderListener::intPropertyChanged |
      nsIFolderListener::boolPropertyChanged |
      nsIFolderListener::unicharPropertyChanged);

  return NS_OK;
}

void nsMsgFolderDataSource::Cleanup()
{
  nsresult rv;
  if (!m_shuttingDown)
  {
    nsCOMPtr<nsIMsgMailSession> mailSession =
      do_GetService(NS_MSGMAILSESSION_CONTRACTID, &rv);
    
    if(NS_SUCCEEDED(rv))
      mailSession->RemoveFolderListener(this);
  }
  
  nsMsgRDFDataSource::Cleanup();
}

nsresult nsMsgFolderDataSource::CreateArcsOutEnumerator()
{
  nsresult rv;

  rv = getFolderArcLabelsOut(getter_AddRefs(kFolderArcsOutArray));
  if(NS_FAILED(rv)) return rv;

  return rv;
}

NS_IMPL_ADDREF_INHERITED(nsMsgFolderDataSource, nsMsgRDFDataSource)
NS_IMPL_RELEASE_INHERITED(nsMsgFolderDataSource, nsMsgRDFDataSource)

NS_IMETHODIMP
nsMsgFolderDataSource::QueryInterface(REFNSIID iid, void** result)
{
  if (! result)
    return NS_ERROR_NULL_POINTER;
  
  *result = nsnull;
  if(iid.Equals(NS_GET_IID(nsIFolderListener)))
  {
    *result = NS_STATIC_CAST(nsIFolderListener*, this);
    NS_ADDREF(this);
    return NS_OK;
  }
  else
    return nsMsgRDFDataSource::QueryInterface(iid, result);
}

 // nsIRDFDataSource methods
NS_IMETHODIMP nsMsgFolderDataSource::GetURI(char* *uri)
{
  if ((*uri = nsCRT::strdup("rdf:mailnewsfolders")) == nsnull)
    return NS_ERROR_OUT_OF_MEMORY;
  else
    return NS_OK;
}

NS_IMETHODIMP nsMsgFolderDataSource::GetSource(nsIRDFResource* property,
                                               nsIRDFNode* target,
                                               PRBool tv,
                                               nsIRDFResource** source /* out */)
{
  NS_ASSERTION(PR_FALSE, "not implemented");
  return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP nsMsgFolderDataSource::GetTarget(nsIRDFResource* source,
                                               nsIRDFResource* property,
                                               PRBool tv,
                                               nsIRDFNode** target)
{
  nsresult rv = NS_RDF_NO_VALUE;

  // we only have positive assertions in the mail data source.
  if (! tv)
    return NS_RDF_NO_VALUE;

  nsCOMPtr<nsIMsgFolder> folder(do_QueryInterface(source));
  if (folder) 
  {
    rv = createFolderNode(folder, property, target);
#if 0
    nsXPIDLCString srcval;
    nsXPIDLCString propval;
    nsXPIDLCString targetval;
    source->GetValue(getter_Copies(srcval));
    property->GetValue(getter_Copies(propval));
    //    (*target)->GetValue(getter_Copies(targetval));

    printf("nsMsgFolderDataSource::GetTarget(%s, %s, %s, (%s))\n",
           (const char*)srcval,
           (const char*)propval, tv ? "TRUE" : "FALSE",
           (const char*)"");
#endif
    
  }
  else
    return NS_RDF_NO_VALUE;
  return rv;
}


NS_IMETHODIMP nsMsgFolderDataSource::GetSources(nsIRDFResource* property,
                                                nsIRDFNode* target,
                                                PRBool tv,
                                                nsISimpleEnumerator** sources)
{
  return NS_RDF_NO_VALUE;
}

NS_IMETHODIMP nsMsgFolderDataSource::GetTargets(nsIRDFResource* source,
                                                nsIRDFResource* property,    
                                                PRBool tv,
                                                nsISimpleEnumerator** targets)
{
  nsresult rv = NS_RDF_NO_VALUE;
  if(!targets)
    return NS_ERROR_NULL_POINTER;
  
#if 0
  nsXPIDLCString srcval;
  nsXPIDLCString propval;
  nsXPIDLCString targetval;
  source->GetValue(getter_Copies(srcval));
  property->GetValue(getter_Copies(propval));
  //    (*target)->GetValue(getter_Copies(targetval));
  
  printf("nsMsgFolderDataSource::GetTargets(%s, %s, %s, (%s))\n",
    (const char*)srcval,
    (const char*)propval, tv ? "TRUE" : "FALSE",
    (const char*)"");
#endif
  *targets = nsnull;
  
  nsCOMPtr<nsIMsgFolder> folder(do_QueryInterface(source, &rv));
  if (NS_SUCCEEDED(rv))
  {
    if ((kNC_Child == property))
    {
      nsCOMPtr<nsIEnumerator> subFolders;
      rv = folder->GetSubFolders(getter_AddRefs(subFolders));
      if(NS_SUCCEEDED(rv))
      {
        nsAdapterEnumerator* cursor =
          new nsAdapterEnumerator(subFolders);
        if (cursor == nsnull)
          return NS_ERROR_OUT_OF_MEMORY;
        NS_ADDREF(cursor);
        *targets = cursor;
        rv = NS_OK;
      }
    }
    else if ((kNC_Name == property) ||
      (kNC_Open == property) ||
      (kNC_FolderTreeName == property) ||
      (kNC_FolderTreeSimpleName == property) ||
      (kNC_SpecialFolder == property) ||
      (kNC_IsServer == property) ||
      (kNC_IsSecure == property) ||
      (kNC_CanSubscribe == property) ||
      (kNC_SupportsOffline == property) ||
      (kNC_CanFileMessages == property) ||
      (kNC_CanCreateSubfolders == property) ||
      (kNC_CanRename == property) ||
      (kNC_CanCompact == property) ||
      (kNC_ServerType == property) ||
      (kNC_IsDeferred == property) ||
      (kNC_RedirectorType == property) ||
      (kNC_CanCreateFoldersOnServer == property) ||
      (kNC_CanFileMessagesOnServer == property) ||
      (kNC_NoSelect == property) ||
      (kNC_VirtualFolder == property) ||
      (kNC_InVFEditSearchScope == property) ||
      (kNC_ImapShared == property) ||
      (kNC_Synchronize == property) ||
      (kNC_SyncDisabled == property) ||
      (kNC_CanSearchMessages == property))
    {
      nsSingletonEnumerator* cursor =
        new nsSingletonEnumerator(property);
      if (cursor == nsnull)
        return NS_ERROR_OUT_OF_MEMORY;
      NS_ADDREF(cursor);
      *targets = cursor;
      rv = NS_OK;
    }
  }
  if(!*targets)
  {
    //create empty cursor
    rv = NS_NewEmptyEnumerator(targets);
  }
  
  return rv;
}

NS_IMETHODIMP nsMsgFolderDataSource::Assert(nsIRDFResource* source,
                      nsIRDFResource* property, 
                      nsIRDFNode* target,
                      PRBool tv)
{
  nsresult rv;
  nsCOMPtr<nsIMsgFolder> folder(do_QueryInterface(source, &rv));
  //We don't handle tv = PR_FALSE at the moment.
  if(NS_SUCCEEDED(rv) && tv)
    return DoFolderAssert(folder, property, target);
  else
    return NS_ERROR_FAILURE;
}

NS_IMETHODIMP nsMsgFolderDataSource::Unassert(nsIRDFResource* source,
                        nsIRDFResource* property,
                        nsIRDFNode* target)
{
  nsresult rv;
  nsCOMPtr<nsIMsgFolder> folder(do_QueryInterface(source, &rv));
  NS_ENSURE_SUCCESS(rv,rv);
  return DoFolderUnassert(folder, property, target);
}


NS_IMETHODIMP nsMsgFolderDataSource::HasAssertion(nsIRDFResource* source,
                            nsIRDFResource* property,
                            nsIRDFNode* target,
                            PRBool tv,
                            PRBool* hasAssertion)
{
  nsresult rv;
#if 0
  nsXPIDLCString sourceval;
  nsXPIDLCString propval;
  nsXPIDLCString targetval;
  source->GetValue(getter_Copies(sourceval));
  property->GetValue(getter_Copies(propval));
  /*  target->GetValue(getter_Copies(targetval)); */
  printf("HasAssertion(%s, %s, ??...)\n", (const char*)sourceval, (const char*)propval);
#endif
  
  nsCOMPtr<nsIMsgFolder> folder(do_QueryInterface(source, &rv));
  if(NS_SUCCEEDED(rv))
    return DoFolderHasAssertion(folder, property, target, tv, hasAssertion);
  else
    *hasAssertion = PR_FALSE;
  return NS_OK;
}


NS_IMETHODIMP 
nsMsgFolderDataSource::HasArcOut(nsIRDFResource *aSource, nsIRDFResource *aArc, PRBool *result)
{
  nsresult rv;
  nsCOMPtr<nsIMsgFolder> folder(do_QueryInterface(aSource, &rv));
  if (NS_SUCCEEDED(rv)) 
  {
    *result = (aArc == kNC_Name ||
      aArc == kNC_Open ||
      aArc == kNC_FolderTreeName ||
      aArc == kNC_FolderTreeSimpleName ||
      aArc == kNC_SpecialFolder ||
      aArc == kNC_ServerType ||
      aArc == kNC_IsDeferred ||
      aArc == kNC_RedirectorType ||
      aArc == kNC_CanCreateFoldersOnServer ||
      aArc == kNC_CanFileMessagesOnServer ||
      aArc == kNC_IsServer ||
      aArc == kNC_IsSecure ||
      aArc == kNC_CanSubscribe ||
      aArc == kNC_SupportsOffline ||
      aArc == kNC_CanFileMessages ||
      aArc == kNC_CanCreateSubfolders ||
      aArc == kNC_CanRename ||
      aArc == kNC_CanCompact ||
      aArc == kNC_TotalMessages ||
      aArc == kNC_TotalUnreadMessages ||
      aArc == kNC_FolderSize ||
      aArc == kNC_Charset ||
      aArc == kNC_BiffState ||
      aArc == kNC_Child ||
      aArc == kNC_NoSelect ||
      aArc == kNC_VirtualFolder ||
      aArc == kNC_InVFEditSearchScope ||
      aArc == kNC_ImapShared ||
      aArc == kNC_Synchronize ||
      aArc == kNC_SyncDisabled ||
      aArc == kNC_CanSearchMessages);
  }
  else 
  {
    *result = PR_FALSE;
  }
  return NS_OK;
}

NS_IMETHODIMP nsMsgFolderDataSource::ArcLabelsIn(nsIRDFNode* node,
                                                 nsISimpleEnumerator** labels)
{
  return nsMsgRDFDataSource::ArcLabelsIn(node, labels);
}

NS_IMETHODIMP nsMsgFolderDataSource::ArcLabelsOut(nsIRDFResource* source,
                                                  nsISimpleEnumerator** labels)
{
  nsresult rv = NS_RDF_NO_VALUE;
  nsCOMPtr<nsISupportsArray> arcsArray;
  
  nsCOMPtr<nsIMsgFolder> folder(do_QueryInterface(source, &rv));
  if (NS_SUCCEEDED(rv)) 
  {
    arcsArray = kFolderArcsOutArray;
    rv = NS_NewArrayEnumerator(labels, arcsArray);
  }
  else 
  {
    rv = NS_NewEmptyEnumerator(labels);
  }
  
  return rv;
}

nsresult
nsMsgFolderDataSource::getFolderArcLabelsOut(nsISupportsArray **arcs)
{
  nsresult rv;
  rv = NS_NewISupportsArray(arcs);
  if(NS_FAILED(rv))
    return rv;
  
  (*arcs)->AppendElement(kNC_Name);
  (*arcs)->AppendElement(kNC_Open);
  (*arcs)->AppendElement(kNC_FolderTreeName);
  (*arcs)->AppendElement(kNC_FolderTreeSimpleName);
  (*arcs)->AppendElement(kNC_SpecialFolder);
  (*arcs)->AppendElement(kNC_ServerType);
  (*arcs)->AppendElement(kNC_IsDeferred);
  (*arcs)->AppendElement(kNC_RedirectorType);
  (*arcs)->AppendElement(kNC_CanCreateFoldersOnServer);
  (*arcs)->AppendElement(kNC_CanFileMessagesOnServer);
  (*arcs)->AppendElement(kNC_IsServer);
  (*arcs)->AppendElement(kNC_IsSecure);
  (*arcs)->AppendElement(kNC_CanSubscribe);
  (*arcs)->AppendElement(kNC_SupportsOffline);
  (*arcs)->AppendElement(kNC_CanFileMessages);
  (*arcs)->AppendElement(kNC_CanCreateSubfolders);
  (*arcs)->AppendElement(kNC_CanRename);
  (*arcs)->AppendElement(kNC_CanCompact);
  (*arcs)->AppendElement(kNC_TotalMessages);
  (*arcs)->AppendElement(kNC_TotalUnreadMessages);
  (*arcs)->AppendElement(kNC_FolderSize);
  (*arcs)->AppendElement(kNC_Charset);
  (*arcs)->AppendElement(kNC_BiffState);
  (*arcs)->AppendElement(kNC_Child);
  (*arcs)->AppendElement(kNC_NoSelect);
  (*arcs)->AppendElement(kNC_VirtualFolder);
  (*arcs)->AppendElement(kNC_InVFEditSearchScope);
  (*arcs)->AppendElement(kNC_ImapShared);
  (*arcs)->AppendElement(kNC_Synchronize);
  (*arcs)->AppendElement(kNC_SyncDisabled);
  (*arcs)->AppendElement(kNC_CanSearchMessages);
  
  return NS_OK;
}

NS_IMETHODIMP
nsMsgFolderDataSource::GetAllResources(nsISimpleEnumerator** aCursor)
{
  NS_NOTYETIMPLEMENTED("sorry!");
  return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP
nsMsgFolderDataSource::GetAllCmds(nsIRDFResource* source,
                                      nsISimpleEnumerator/*<nsIRDFResource>*/** commands)
{
  NS_NOTYETIMPLEMENTED("no one actually uses me");
  nsresult rv;

  nsCOMPtr<nsIMsgFolder> folder(do_QueryInterface(source, &rv));
  if (NS_FAILED(rv)) return rv;

  nsCOMPtr<nsIMutableArray> cmds =
    do_CreateInstance(NS_ARRAY_CONTRACTID);
  NS_ENSURE_STATE(cmds);

  cmds->AppendElement(kNC_Delete, PR_FALSE);
  cmds->AppendElement(kNC_ReallyDelete, PR_FALSE);
  cmds->AppendElement(kNC_NewFolder, PR_FALSE);
  cmds->AppendElement(kNC_GetNewMessages, PR_FALSE);
  cmds->AppendElement(kNC_Copy, PR_FALSE);
  cmds->AppendElement(kNC_Move, PR_FALSE);
  cmds->AppendElement(kNC_CopyFolder, PR_FALSE);
  cmds->AppendElement(kNC_MoveFolder, PR_FALSE);
  cmds->AppendElement(kNC_MarkAllMessagesRead, PR_FALSE);
  cmds->AppendElement(kNC_Compact, PR_FALSE);
  cmds->AppendElement(kNC_CompactAll, PR_FALSE);
  cmds->AppendElement(kNC_Rename, PR_FALSE);
  cmds->AppendElement(kNC_EmptyTrash, PR_FALSE);
  cmds->AppendElement(kNC_DownloadFlagged, PR_FALSE);

  return cmds->Enumerate(commands);
}

NS_IMETHODIMP
nsMsgFolderDataSource::IsCommandEnabled(nsISupportsArray/*<nsIRDFResource>*/* aSources,
                                        nsIRDFResource*   aCommand,
                                        nsISupportsArray/*<nsIRDFResource>*/* aArguments,
                                        PRBool* aResult)
{
  nsresult rv;
  nsCOMPtr<nsIMsgFolder> folder;

  PRUint32 cnt;
  rv = aSources->Count(&cnt);
  if (NS_FAILED(rv)) return rv;
  for (PRUint32 i = 0; i < cnt; i++) 
  {
    folder = do_QueryElementAt(aSources, i, &rv);
    if (NS_SUCCEEDED(rv)) 
    {
      // we don't care about the arguments -- folder commands are always enabled
      if (!((aCommand == kNC_Delete) ||
            (aCommand == kNC_ReallyDelete) ||
            (aCommand == kNC_NewFolder) ||
            (aCommand == kNC_Copy) ||
            (aCommand == kNC_Move) ||
            (aCommand == kNC_CopyFolder) ||
            (aCommand == kNC_MoveFolder) ||
            (aCommand == kNC_GetNewMessages) ||
            (aCommand == kNC_MarkAllMessagesRead) ||
            (aCommand == kNC_Compact) || 
            (aCommand == kNC_CompactAll) || 
            (aCommand == kNC_Rename) ||
            (aCommand == kNC_EmptyTrash) ||
            (aCommand == kNC_DownloadFlagged) )) 
      {
        *aResult = PR_FALSE;
        return NS_OK;
      }
    }
  }
  *aResult = PR_TRUE;
  return NS_OK; // succeeded for all sources
}

NS_IMETHODIMP
nsMsgFolderDataSource::DoCommand(nsISupportsArray/*<nsIRDFResource>*/* aSources,
                                 nsIRDFResource*   aCommand,
                                 nsISupportsArray/*<nsIRDFResource>*/* aArguments)
{
  nsresult rv = NS_OK;
  nsCOMPtr<nsISupports> supports;
  nsCOMPtr<nsIMsgWindow> window;

  // callers can pass in the msgWindow as the last element of the arguments
  // array. If they do, we'll use that as the msg window for progress, etc.
  if (aArguments)
  {
    PRUint32 numArgs;
    aArguments->Count(&numArgs);
    if (numArgs > 1)
      window = do_QueryElementAt(aArguments, numArgs - 1);
  }
  if (!window)
    window = mWindow;

  // XXX need to handle batching of command applied to all sources

  PRUint32 cnt = 0;
  PRUint32 i = 0;

  rv = aSources->Count(&cnt);
  if (NS_FAILED(rv)) return rv;

  for ( ; i < cnt; i++) 
  {
    nsCOMPtr<nsIMsgFolder> folder = do_QueryElementAt(aSources, i, &rv);
    if (NS_SUCCEEDED(rv)) 
    {
      if ((aCommand == kNC_Delete))
      {
        rv = DoDeleteFromFolder(folder, aArguments, window, PR_FALSE);
      }
      if ((aCommand == kNC_ReallyDelete))
      {
        rv = DoDeleteFromFolder(folder, aArguments, window, PR_TRUE);
      }
      else if((aCommand == kNC_NewFolder)) 
      {
        rv = DoNewFolder(folder, aArguments, window);
      }
      else if((aCommand == kNC_GetNewMessages))
      {
        nsCOMPtr<nsIMsgIncomingServer> server = do_QueryElementAt(aArguments, i, &rv);
        NS_ENSURE_SUCCESS(rv, rv);
        rv = server->GetNewMessages(folder, window, nsnull);
      }
      else if((aCommand == kNC_Copy))
      {
        rv = DoCopyToFolder(folder, aArguments, window, PR_FALSE);
      }
      else if((aCommand == kNC_Move))
      {
        rv = DoCopyToFolder(folder, aArguments, window, PR_TRUE);
      }
      else if((aCommand == kNC_CopyFolder))
      {
        rv = DoFolderCopyToFolder(folder, aArguments, window, PR_FALSE);
      }
      else if((aCommand == kNC_MoveFolder))
      {
        rv = DoFolderCopyToFolder(folder, aArguments, window, PR_TRUE);
      }
      else if((aCommand == kNC_MarkAllMessagesRead))
      {
        rv = folder->MarkAllMessagesRead();
      }
      else if ((aCommand == kNC_Compact))
      {
        rv = folder->Compact(nsnull, window);
      }
      else if ((aCommand == kNC_CompactAll))
      {
        rv = folder->CompactAll(nsnull, window, nsnull, PR_TRUE, nsnull);
      }
      else if ((aCommand == kNC_EmptyTrash))
      {
          rv = folder->EmptyTrash(window, nsnull);
      }
      else if ((aCommand == kNC_Rename))
      {
        nsCOMPtr<nsIRDFLiteral> literal = do_QueryElementAt(aArguments, 0, &rv);
        if(NS_SUCCEEDED(rv))
        {
          nsXPIDLString name;
          literal->GetValue(getter_Copies(name));

          rv = folder->Rename(name.get(), window);
        }
      }
    }
    else 
    {
      rv = NS_ERROR_NOT_IMPLEMENTED;
    }
  }
  //for the moment return NS_OK, because failure stops entire DoCommand process.
  return rv;
  //return NS_OK;
}

NS_IMETHODIMP nsMsgFolderDataSource::OnItemAdded(nsIRDFResource *parentItem, nsISupports *item)
{
  return OnItemAddedOrRemoved(parentItem, item, PR_TRUE);
}

NS_IMETHODIMP nsMsgFolderDataSource::OnItemRemoved(nsIRDFResource *parentItem, nsISupports *item)
{
  return OnItemAddedOrRemoved(parentItem, item, PR_FALSE);
}


nsresult nsMsgFolderDataSource::OnItemAddedOrRemoved(nsIRDFResource *parentItem, nsISupports *item, PRBool added)
{
  nsCOMPtr<nsIRDFNode> itemNode(do_QueryInterface(item));
  if (itemNode)
  {
    NotifyObservers(parentItem, kNC_Child, itemNode, nsnull, added, PR_FALSE);
  }
  return NS_OK;
}

NS_IMETHODIMP
nsMsgFolderDataSource::OnItemPropertyChanged(nsIRDFResource *resource,
                                             nsIAtom *property,
                                             const char *oldValue,
                                             const char *newValue)

{
  return NS_OK;
}

NS_IMETHODIMP
nsMsgFolderDataSource::OnItemIntPropertyChanged(nsIRDFResource *resource,
                                                nsIAtom *property,
                                                PRInt32 oldValue,
                                                PRInt32 newValue)
{
  if (kTotalMessagesAtom == property)
    OnTotalMessagePropertyChanged(resource, oldValue, newValue);
  else if (kTotalUnreadMessagesAtom == property)
    OnUnreadMessagePropertyChanged(resource, oldValue, newValue);
  else if (kFolderSizeAtom == property)
    OnFolderSizePropertyChanged(resource, oldValue, newValue);
  else if (kBiffStateAtom == property) {
    // be careful about skipping if oldValue == newValue
    // see the comment in nsMsgFolder::SetBiffState() about filters

    nsCOMPtr<nsIRDFNode> biffNode;
    nsresult rv = createBiffStateNodeFromFlag(newValue, getter_AddRefs(biffNode));
    NS_ENSURE_SUCCESS(rv,rv);

    NotifyPropertyChanged(resource, kNC_BiffState, biffNode);
  }
  return NS_OK;
}

NS_IMETHODIMP
nsMsgFolderDataSource::OnItemUnicharPropertyChanged(nsIRDFResource *resource,
                                                    nsIAtom *property,
                                                    const PRUnichar *oldValue,
                                                    const PRUnichar *newValue)
{
  if (kNameAtom == property) 
  {
    nsCOMPtr<nsIMsgFolder> folder(do_QueryInterface(resource));
    if (folder) 
    {
      PRInt32 numUnread;
      folder->GetNumUnread(PR_FALSE, &numUnread);
      NotifyFolderTreeNameChanged(folder, resource, numUnread);
      NotifyFolderTreeSimpleNameChanged(folder, resource);
      NotifyFolderNameChanged(folder, resource);
    }
  }
  return NS_OK;
}

NS_IMETHODIMP
nsMsgFolderDataSource::OnItemBoolPropertyChanged(nsIRDFResource *resource,
                                                 nsIAtom *property,
                                                 PRBool oldValue,
                                                 PRBool newValue)
{
  if (newValue != oldValue) {
    nsIRDFNode* literalNode = newValue?kTrueLiteral:kFalseLiteral;
    nsIRDFNode* oldLiteralNode = oldValue?kTrueLiteral:kFalseLiteral;
    if (kNewMessagesAtom == property)
      NotifyPropertyChanged(resource, kNC_NewMessages, literalNode); 
    else if (kSynchronizeAtom == property)
      NotifyPropertyChanged(resource, kNC_Synchronize, literalNode); 
    else if (kOpenAtom == property)
      NotifyPropertyChanged(resource, kNC_Open, literalNode);
    else if (kIsDeferredAtom == property) 
      NotifyPropertyChanged(resource, kNC_IsDeferred, literalNode, oldLiteralNode);
    else if (kIsSecureAtom == property)
      NotifyPropertyChanged(resource, kNC_IsSecure, literalNode, oldLiteralNode);
    else if (kCanFileMessagesAtom == property)
      NotifyPropertyChanged(resource, kNC_CanFileMessages, literalNode, oldLiteralNode);
    else if (kInVFEditSearchScopeAtom == property)
      NotifyPropertyChanged(resource, kNC_InVFEditSearchScope, literalNode); 
  } 

  return NS_OK;
}

NS_IMETHODIMP
nsMsgFolderDataSource::OnItemPropertyFlagChanged(nsIMsgDBHdr *item,
                                                 nsIAtom *property,
                                                 PRUint32 oldFlag,
                                                 PRUint32 newFlag)
{
  return NS_OK;
}

NS_IMETHODIMP
nsMsgFolderDataSource::OnItemEvent(nsIMsgFolder *aFolder, nsIAtom *aEvent)
{
  return NS_OK;
}


nsresult nsMsgFolderDataSource::createFolderNode(nsIMsgFolder* folder,
                                                 nsIRDFResource* property,
                                                 nsIRDFNode** target)
{
  nsresult rv = NS_RDF_NO_VALUE;

  if (kNC_NameSort == property)
    rv = createFolderNameNode(folder, target, PR_TRUE);
  else if(kNC_FolderTreeNameSort == property)
    rv = createFolderNameNode(folder, target, PR_TRUE);
  else if (kNC_Name == property)
    rv = createFolderNameNode(folder, target, PR_FALSE);
  else if(kNC_Open == property)
    rv = createFolderOpenNode(folder, target);
  else if (kNC_FolderTreeName == property)
    rv = createFolderTreeNameNode(folder, target);
  else if (kNC_FolderTreeSimpleName == property)
    rv = createFolderTreeSimpleNameNode(folder, target);
  else if ((kNC_SpecialFolder == property))
    rv = createFolderSpecialNode(folder,target);
  else if ((kNC_ServerType == property))
    rv = createFolderServerTypeNode(folder, target);
  else if ((kNC_IsDeferred == property))
    rv = createServerIsDeferredNode(folder, target);
  else if ((kNC_RedirectorType == property))
    rv = createFolderRedirectorTypeNode(folder, target);
  else if ((kNC_CanCreateFoldersOnServer == property))
    rv = createFolderCanCreateFoldersOnServerNode(folder, target);
  else if ((kNC_CanFileMessagesOnServer == property))
    rv = createFolderCanFileMessagesOnServerNode(folder, target);
  else if ((kNC_IsServer == property))
    rv = createFolderIsServerNode(folder, target);
  else if ((kNC_IsSecure == property))
    rv = createFolderIsSecureNode(folder, target);
  else if ((kNC_CanSubscribe == property))
    rv = createFolderCanSubscribeNode(folder, target);
  else if ((kNC_SupportsOffline == property))
    rv = createFolderSupportsOfflineNode(folder, target);
  else if ((kNC_CanFileMessages == property))
    rv = createFolderCanFileMessagesNode(folder, target);
  else if ((kNC_CanCreateSubfolders == property))
    rv = createFolderCanCreateSubfoldersNode(folder, target);
  else if ((kNC_CanRename == property))
    rv = createFolderCanRenameNode(folder, target);
  else if ((kNC_CanCompact == property))
    rv = createFolderCanCompactNode(folder, target);
  else if ((kNC_TotalMessages == property))
    rv = createTotalMessagesNode(folder, target);
  else if ((kNC_TotalUnreadMessages == property))
    rv = createUnreadMessagesNode(folder, target);
  else if ((kNC_FolderSize == property))
    rv = createFolderSizeNode(folder, target);
  else if ((kNC_Charset == property))
    rv = createCharsetNode(folder, target);
  else if ((kNC_BiffState == property))
    rv = createBiffStateNodeFromFolder(folder, target);
  else if ((kNC_HasUnreadMessages == property))
    rv = createHasUnreadMessagesNode(folder, PR_FALSE, target);
  else if ((kNC_NewMessages == property))
    rv = createNewMessagesNode(folder, target);
  else if ((kNC_SubfoldersHaveUnreadMessages == property))
    rv = createHasUnreadMessagesNode(folder, PR_TRUE, target);
  else if ((kNC_Child == property))
    rv = createFolderChildNode(folder, target);
  else if ((kNC_NoSelect == property))
    rv = createFolderNoSelectNode(folder, target);
  else if ((kNC_VirtualFolder == property))
    rv = createFolderVirtualNode(folder, target);
  else if (kNC_InVFEditSearchScope == property)
    rv = createInVFEditSearchScopeNode(folder, target);
  else if ((kNC_ImapShared == property))
    rv = createFolderImapSharedNode(folder, target);
  else if ((kNC_Synchronize == property))
    rv = createFolderSynchronizeNode(folder, target);
  else if ((kNC_SyncDisabled == property))
    rv = createFolderSyncDisabledNode(folder, target);
  else if ((kNC_CanSearchMessages == property))
    rv = createCanSearchMessages(folder, target);


  if (NS_FAILED(rv)) return NS_RDF_NO_VALUE;
  return rv;
}


nsresult 
nsMsgFolderDataSource::createFolderNameNode(nsIMsgFolder *folder,
                                            nsIRDFNode **target, PRBool sort)
{
  nsresult rv;
  if (sort) 
  {
    PRUint8 *sortKey=nsnull;
    PRUint32 sortKeyLength;
    rv = folder->GetSortKey(&sortKey, &sortKeyLength);
    NS_ENSURE_SUCCESS(rv, rv);
    createBlobNode(sortKey, sortKeyLength, target, getRDFService());
    PR_Free(sortKey);
  }
  else 
  {
    nsXPIDLString name;
    rv = folder->GetName(getter_Copies(name));
    if (NS_FAILED(rv)) 
      return rv;
    createNode(name.get(), target, getRDFService());
  }
    
  return NS_OK;
}

nsresult nsMsgFolderDataSource::GetFolderDisplayName(nsIMsgFolder *folder, PRUnichar **folderName)
{
  return folder->GetAbbreviatedName(folderName);
}

nsresult 
nsMsgFolderDataSource::createFolderTreeNameNode(nsIMsgFolder *folder,
                                                nsIRDFNode **target)
{
  nsXPIDLString name;
  nsresult rv = GetFolderDisplayName(folder, getter_Copies(name));
  if (NS_FAILED(rv)) return rv;
  nsAutoString nameString(name);
  PRInt32 unreadMessages;

  rv = folder->GetNumUnread(PR_FALSE, &unreadMessages);
  if(NS_SUCCEEDED(rv)) 
    CreateUnreadMessagesNameString(unreadMessages, nameString);	

  createNode(nameString.get(), target, getRDFService());
  return NS_OK;
}

nsresult nsMsgFolderDataSource::createFolderTreeSimpleNameNode(nsIMsgFolder * folder, nsIRDFNode **target)
{
  nsXPIDLString name;
  nsresult rv = GetFolderDisplayName(folder, getter_Copies(name));
  if (NS_FAILED(rv)) return rv;

  createNode(name.get(), target, getRDFService());
  return NS_OK;
}

nsresult nsMsgFolderDataSource::CreateUnreadMessagesNameString(PRInt32 unreadMessages, nsAutoString &nameString)
{
  //Only do this if unread messages are positive
  if(unreadMessages > 0)
  {
    nameString.Append(NS_LITERAL_STRING(" (").get());
    nameString.AppendInt(unreadMessages);
    nameString.Append(NS_LITERAL_STRING(")").get());
  }
  return NS_OK;
}

nsresult
nsMsgFolderDataSource::createFolderSpecialNode(nsIMsgFolder *folder,
                                               nsIRDFNode **target)
{
  PRUint32 flags;
  nsresult rv = folder->GetFlags(&flags);
  if(NS_FAILED(rv)) 
    return rv;
  
  nsAutoString specialFolderString;
  if (flags & MSG_FOLDER_FLAG_INBOX)
    specialFolderString.AssignLiteral("Inbox");
  else if (flags & MSG_FOLDER_FLAG_TRASH)
    specialFolderString.AssignLiteral("Trash");
  else if (flags & MSG_FOLDER_FLAG_QUEUE)
    specialFolderString.AssignLiteral("Unsent Messages");
  else if (flags & MSG_FOLDER_FLAG_SENTMAIL)
    specialFolderString.AssignLiteral("Sent");
  else if (flags & MSG_FOLDER_FLAG_DRAFTS)
    specialFolderString.AssignLiteral("Drafts");
  else if (flags & MSG_FOLDER_FLAG_TEMPLATES)
    specialFolderString.AssignLiteral("Templates");
  else if (flags & MSG_FOLDER_FLAG_JUNK)
    specialFolderString.AssignLiteral("Junk");
  else if (flags & MSG_FOLDER_FLAG_VIRTUAL)
    specialFolderString.AssignLiteral("Virtual");
  else {
    // XXX why do this at all? or just ""
    specialFolderString.AssignLiteral("none");
  }
  
  createNode(specialFolderString.get(), target, getRDFService());
  return NS_OK;
}

nsresult
nsMsgFolderDataSource::createFolderServerTypeNode(nsIMsgFolder* folder,
                                                  nsIRDFNode **target)
{
  nsresult rv;
  nsCOMPtr<nsIMsgIncomingServer> server;
  rv = folder->GetServer(getter_AddRefs(server));
  if (NS_FAILED(rv) || !server) return NS_ERROR_FAILURE;

  nsXPIDLCString serverType;
  rv = server->GetType(getter_Copies(serverType));
  if (NS_FAILED(rv)) return rv;

  createNode(NS_ConvertASCIItoUTF16(serverType).get(), target, getRDFService());
  return NS_OK;
}

nsresult
nsMsgFolderDataSource::createServerIsDeferredNode(nsIMsgFolder* folder,
                                                  nsIRDFNode **target)
{
  PRBool isDeferred = PR_FALSE;
  nsCOMPtr <nsIMsgIncomingServer> incomingServer;
  folder->GetServer(getter_AddRefs(incomingServer));
  if (incomingServer)
  {
    nsCOMPtr <nsIPop3IncomingServer> pop3Server = do_QueryInterface(incomingServer);
    if (pop3Server)
    {
      nsXPIDLCString deferredToServer;
      pop3Server->GetDeferredToAccount(getter_Copies(deferredToServer));
      isDeferred = !deferredToServer.IsEmpty();
    }
  }
  *target = (isDeferred) ? kTrueLiteral : kFalseLiteral;
  NS_IF_ADDREF(*target);
  return NS_OK;
}

nsresult
nsMsgFolderDataSource::createFolderRedirectorTypeNode(nsIMsgFolder* folder,
                                                  nsIRDFNode **target)
{
  nsresult rv;
  nsCOMPtr<nsIMsgIncomingServer> server;
  rv = folder->GetServer(getter_AddRefs(server));
  if (NS_FAILED(rv) || !server) return NS_ERROR_FAILURE;

  nsXPIDLCString redirectorType;
  rv = server->GetRedirectorType(getter_Copies(redirectorType));
  if (NS_FAILED(rv)) return rv;

  createNode(NS_ConvertASCIItoUTF16(redirectorType).get(), target, getRDFService());
  return NS_OK;
}

nsresult
nsMsgFolderDataSource::createFolderCanCreateFoldersOnServerNode(nsIMsgFolder* folder,
                                                                 nsIRDFNode **target)
{
  nsresult rv;

  nsCOMPtr<nsIMsgIncomingServer> server;
  rv = folder->GetServer(getter_AddRefs(server));
  if (NS_FAILED(rv) || !server) return NS_ERROR_FAILURE;
  
  PRBool canCreateFoldersOnServer;
  rv = server->GetCanCreateFoldersOnServer(&canCreateFoldersOnServer);
  if (NS_FAILED(rv)) return rv;

  if (canCreateFoldersOnServer)
    *target = kTrueLiteral;
  else
    *target = kFalseLiteral;
  NS_IF_ADDREF(*target);

  return NS_OK;
}

nsresult
nsMsgFolderDataSource::createFolderCanFileMessagesOnServerNode(nsIMsgFolder* folder,
                                                                 nsIRDFNode **target)
{
  nsresult rv;

  nsCOMPtr<nsIMsgIncomingServer> server;
  rv = folder->GetServer(getter_AddRefs(server));
  if (NS_FAILED(rv) || !server) return NS_ERROR_FAILURE;

  PRBool canFileMessagesOnServer;
  rv = server->GetCanFileMessagesOnServer(&canFileMessagesOnServer);
  if (NS_FAILED(rv)) return rv;
  
  *target = (canFileMessagesOnServer) ? kTrueLiteral : kFalseLiteral;
  NS_IF_ADDREF(*target);

  return NS_OK;
}


nsresult
nsMsgFolderDataSource::createFolderIsServerNode(nsIMsgFolder* folder,
                                                  nsIRDFNode **target)
{
  nsresult rv;
  PRBool isServer;
  rv = folder->GetIsServer(&isServer);
  if (NS_FAILED(rv)) return rv;

  *target = nsnull;

  if (isServer)
    *target = kTrueLiteral;
  else
    *target = kFalseLiteral;
  NS_IF_ADDREF(*target);
  return NS_OK;
}

nsresult
nsMsgFolderDataSource::createFolderNoSelectNode(nsIMsgFolder* folder,
                                                  nsIRDFNode **target)
{
  nsresult rv;
  PRBool noSelect;
  rv = folder->GetNoSelect(&noSelect);
  if (NS_FAILED(rv)) return rv;

  *target = (noSelect) ? kTrueLiteral : kFalseLiteral;
  NS_IF_ADDREF(*target);
  return NS_OK;
}

nsresult
nsMsgFolderDataSource::createInVFEditSearchScopeNode(nsIMsgFolder* folder,
                                                  nsIRDFNode **target)
{
  PRBool inVFEditSearchScope = PR_FALSE;
  folder->GetInVFEditSearchScope(&inVFEditSearchScope);

  *target = inVFEditSearchScope ? kTrueLiteral : kFalseLiteral;
  NS_IF_ADDREF(*target);
  return NS_OK;
}

nsresult
nsMsgFolderDataSource::createFolderVirtualNode(nsIMsgFolder* folder,
                                                  nsIRDFNode **target)
{
  PRUint32 folderFlags;
  folder->GetFlags(&folderFlags);

  *target = (folderFlags & MSG_FOLDER_FLAG_VIRTUAL) ? kTrueLiteral : kFalseLiteral;
  NS_IF_ADDREF(*target);
  return NS_OK;
}


nsresult
nsMsgFolderDataSource::createFolderImapSharedNode(nsIMsgFolder* folder,
                                                  nsIRDFNode **target)
{
  nsresult rv;
  PRBool imapShared; 
  rv = folder->GetImapShared(&imapShared);
  if (NS_FAILED(rv)) return rv;

  *target = (imapShared) ? kTrueLiteral : kFalseLiteral;
  NS_IF_ADDREF(*target);
  return NS_OK;
}


nsresult
nsMsgFolderDataSource::createFolderSynchronizeNode(nsIMsgFolder* folder,
                                                  nsIRDFNode **target)
{
  nsresult rv;
  PRBool sync;
  rv = folder->GetFlag(MSG_FOLDER_FLAG_OFFLINE, &sync);		
  if (NS_FAILED(rv)) return rv;

  *target = nsnull;

  *target = (sync) ? kTrueLiteral : kFalseLiteral;
  NS_IF_ADDREF(*target);
  return NS_OK;
}

nsresult
nsMsgFolderDataSource::createFolderSyncDisabledNode(nsIMsgFolder* folder,
                                                  nsIRDFNode **target)
{
  
  nsresult rv;
  PRBool isServer;
  nsCOMPtr<nsIMsgIncomingServer> server;

  rv = folder->GetIsServer(&isServer);
  if (NS_FAILED(rv)) return rv;
    
  rv = folder->GetServer(getter_AddRefs(server));
  if (NS_FAILED(rv) || !server) return NS_ERROR_FAILURE;

  nsXPIDLCString serverType;
  rv = server->GetType(getter_Copies(serverType));
  if (NS_FAILED(rv)) return rv;

  *target = nsnull;

  if (nsCRT::strcasecmp(serverType, "none")==0 || nsCRT::strcasecmp(serverType, "pop3")==0
      || isServer)
    *target = kTrueLiteral;
  else
    *target = kFalseLiteral;
  NS_IF_ADDREF(*target);
  return NS_OK;
}

nsresult
nsMsgFolderDataSource::createCanSearchMessages(nsIMsgFolder* folder,
                                                                 nsIRDFNode **target)
{
  nsresult rv;

  nsCOMPtr<nsIMsgIncomingServer> server;
  rv = folder->GetServer(getter_AddRefs(server));
  if (NS_FAILED(rv) || !server) return NS_ERROR_FAILURE;

  PRBool canSearchMessages;
  rv = server->GetCanSearchMessages(&canSearchMessages);
  if (NS_FAILED(rv)) return rv;
  
  *target = (canSearchMessages) ? kTrueLiteral : kFalseLiteral;
  NS_IF_ADDREF(*target);

  return NS_OK;
}

nsresult
nsMsgFolderDataSource::createFolderOpenNode(nsIMsgFolder *folder, nsIRDFNode **target)
{
  NS_ENSURE_ARG_POINTER(target);

  // call GetSubFolders() to ensure mFlags is set correctly 
  // from the folder cache on startup
  nsCOMPtr<nsIEnumerator> subFolders;
  nsresult rv = folder->GetSubFolders(getter_AddRefs(subFolders));
  if (NS_FAILED(rv))
    return NS_RDF_NO_VALUE;

  PRBool closed;
  rv = folder->GetFlag(MSG_FOLDER_FLAG_ELIDED, &closed);		
  if (NS_FAILED(rv)) 
    return rv;

  *target = (closed) ? kFalseLiteral : kTrueLiteral;

  NS_IF_ADDREF(*target);
  return NS_OK;
}

nsresult
nsMsgFolderDataSource::createFolderIsSecureNode(nsIMsgFolder* folder,
                                                  nsIRDFNode **target)
{
  nsresult rv;
  PRBool isSecure = PR_FALSE;

  nsCOMPtr<nsIMsgIncomingServer> server;
  rv = folder->GetServer(getter_AddRefs(server));

  if (NS_SUCCEEDED(rv) && server) {
    nsCOMPtr<nsINntpIncomingServer> nntpIncomingServer = do_QueryInterface(server);

    if(nntpIncomingServer)  
      rv = server->GetIsSecure(&isSecure);
    else {
      PRInt32 socketType;
      rv = server->GetSocketType(&socketType);
      if (NS_SUCCEEDED(rv) && (socketType == nsIMsgIncomingServer::alwaysUseTLS || 
                              socketType == nsIMsgIncomingServer::useSSL))
        isSecure = PR_TRUE;
    }
  }

  *target = (isSecure) ? kTrueLiteral : kFalseLiteral;
  NS_IF_ADDREF(*target);
  return NS_OK;
}


nsresult
nsMsgFolderDataSource::createFolderCanSubscribeNode(nsIMsgFolder* folder,
                                                  nsIRDFNode **target)
{
  nsresult rv;
  PRBool canSubscribe;
  rv = folder->GetCanSubscribe(&canSubscribe);
  if (NS_FAILED(rv)) return rv;

  *target = (canSubscribe) ? kTrueLiteral : kFalseLiteral;
  NS_IF_ADDREF(*target);
  return NS_OK;
}

nsresult
nsMsgFolderDataSource::createFolderSupportsOfflineNode(nsIMsgFolder* folder,
                                                  nsIRDFNode **target)
{
  nsresult rv;
  PRBool supportsOffline;
  rv = folder->GetSupportsOffline(&supportsOffline);
  NS_ENSURE_SUCCESS(rv,rv);
 
  *target = (supportsOffline) ? kTrueLiteral : kFalseLiteral;
  NS_IF_ADDREF(*target);
  return NS_OK;
}

nsresult
nsMsgFolderDataSource::createFolderCanFileMessagesNode(nsIMsgFolder* folder,
                                                  nsIRDFNode **target)
{
  nsresult rv;
  PRBool canFileMessages;
  rv = folder->GetCanFileMessages(&canFileMessages);
  if (NS_FAILED(rv)) return rv;

  *target = (canFileMessages) ? kTrueLiteral : kFalseLiteral;
  NS_IF_ADDREF(*target);
  return NS_OK;
}

nsresult
nsMsgFolderDataSource::createFolderCanCreateSubfoldersNode(nsIMsgFolder* folder,
                                                  nsIRDFNode **target)
{
  nsresult rv;
  PRBool canCreateSubfolders;
  rv = folder->GetCanCreateSubfolders(&canCreateSubfolders);
  if (NS_FAILED(rv)) return rv;

  *target = (canCreateSubfolders) ? kTrueLiteral : kFalseLiteral;
  NS_IF_ADDREF(*target);
  return NS_OK;
}

nsresult
nsMsgFolderDataSource::createFolderCanRenameNode(nsIMsgFolder* folder,
                                                  nsIRDFNode **target)
{
  PRBool canRename;
  nsresult rv = folder->GetCanRename(&canRename);
  if (NS_FAILED(rv)) return rv;

  *target = (canRename) ? kTrueLiteral : kFalseLiteral;
  NS_IF_ADDREF(*target);
  return NS_OK;
}

nsresult
nsMsgFolderDataSource::createFolderCanCompactNode(nsIMsgFolder* folder,
                                                  nsIRDFNode **target)
{
  PRBool canCompact;
  nsresult rv = folder->GetCanCompact(&canCompact);
  if (NS_FAILED(rv)) return rv;

  *target = (canCompact) ? kTrueLiteral : kFalseLiteral;
  NS_IF_ADDREF(*target);
  return NS_OK;
}


nsresult
nsMsgFolderDataSource::createTotalMessagesNode(nsIMsgFolder *folder,
                                               nsIRDFNode **target)
{

  PRBool isServer;
  nsresult rv = folder->GetIsServer(&isServer);
  if (NS_FAILED(rv)) return rv;
  
  PRInt32 totalMessages;
  if(isServer)
    totalMessages = kDisplayBlankCount;
  else
  {
    rv = folder->GetTotalMessages(PR_FALSE, &totalMessages);
    if(NS_FAILED(rv)) return rv;
  }
  GetNumMessagesNode(totalMessages, target);
  
  return rv;
}

nsresult
nsMsgFolderDataSource::createFolderSizeNode(nsIMsgFolder *folder, nsIRDFNode **target)
{
  PRBool isServer;
  nsresult rv = folder->GetIsServer(&isServer);
  NS_ENSURE_SUCCESS(rv, rv);
  
  PRInt32 folderSize;
  if(isServer)
    folderSize = kDisplayBlankCount;
  else
  {
    // XXX todo, we are asserting here for news
    // for offline news, we'd know the size on disk, right?
    rv = folder->GetSizeOnDisk((PRUint32 *) &folderSize);
    NS_ENSURE_SUCCESS(rv, rv);
  }
  GetFolderSizeNode(folderSize, target);
  
  return rv;
}

nsresult
nsMsgFolderDataSource::createCharsetNode(nsIMsgFolder *folder, nsIRDFNode **target)
{
  nsXPIDLCString charset;
  nsresult rv = folder->GetCharset(getter_Copies(charset));
  if (NS_SUCCEEDED(rv))
    createNode(NS_ConvertASCIItoUTF16(charset).get(), target, getRDFService());
  else
    createNode(EmptyString().get(), target, getRDFService());
  return NS_OK;
}

nsresult
nsMsgFolderDataSource::createBiffStateNodeFromFolder(nsIMsgFolder *folder, nsIRDFNode **target)
{
  PRUint32 biffState;
  nsresult rv = folder->GetBiffState(&biffState);
  if(NS_FAILED(rv)) return rv;

  rv = createBiffStateNodeFromFlag(biffState, target);
  NS_ENSURE_SUCCESS(rv,rv);

  return NS_OK;
}

nsresult
nsMsgFolderDataSource::createBiffStateNodeFromFlag(PRUint32 flag, nsIRDFNode **target)
{
  const PRUnichar *biffStateStr;

  switch (flag) {
    case nsIMsgFolder::nsMsgBiffState_NewMail:
      biffStateStr = NS_LITERAL_STRING("NewMail").get();
      break;
    case nsIMsgFolder::nsMsgBiffState_NoMail:
      biffStateStr = NS_LITERAL_STRING("NoMail").get();
      break;
    default:
      biffStateStr = NS_LITERAL_STRING("UnknownMail").get();
      break;
  }

  createNode(biffStateStr, target, getRDFService());
  return NS_OK;
}

nsresult 
nsMsgFolderDataSource::createUnreadMessagesNode(nsIMsgFolder *folder,
												nsIRDFNode **target)
{
  PRBool isServer;
  nsresult rv = folder->GetIsServer(&isServer);
  if (NS_FAILED(rv)) return rv;
  
  PRInt32 totalUnreadMessages;
  if(isServer)
    totalUnreadMessages = kDisplayBlankCount;
  else
  {
    rv = folder->GetNumUnread(PR_FALSE, &totalUnreadMessages);
    if(NS_FAILED(rv)) return rv;
  }
  GetNumMessagesNode(totalUnreadMessages, target);
  
  return NS_OK;
}

nsresult
nsMsgFolderDataSource::createHasUnreadMessagesNode(nsIMsgFolder *folder, PRBool aIncludeSubfolders, nsIRDFNode **target)
{
  PRBool isServer;
  nsresult rv = folder->GetIsServer(&isServer);
  if (NS_FAILED(rv)) return rv;

  *target = kFalseLiteral;

  PRInt32 totalUnreadMessages;
  if(!isServer)
  {
    rv = folder->GetNumUnread(aIncludeSubfolders, &totalUnreadMessages);
    if(NS_FAILED(rv)) return rv;
    // if we're including sub-folders, we're trying to find out if child folders
    // have unread. If so, we subtract the unread msgs in the current folder.
    if (aIncludeSubfolders)
    {
      PRInt32 numUnreadInFolder;
      rv = folder->GetNumUnread(PR_FALSE, &numUnreadInFolder);
      NS_ENSURE_SUCCESS(rv, rv);
      // don't subtract if numUnread is negative (which means we don't know the unread count)
      if (numUnreadInFolder > 0) 
        totalUnreadMessages -= numUnreadInFolder;
    }
    *target = (totalUnreadMessages > 0) ? kTrueLiteral : kFalseLiteral;
  }

  NS_IF_ADDREF(*target);
  return NS_OK;
}

nsresult
nsMsgFolderDataSource::OnUnreadMessagePropertyChanged(nsIRDFResource *folderResource, PRInt32 oldValue, PRInt32 newValue)
{
  nsCOMPtr<nsIMsgFolder> folder = do_QueryInterface(folderResource);
  if(folder)
  {
    //First send a regular unread message changed notification
    nsCOMPtr<nsIRDFNode> newNode;

    GetNumMessagesNode(newValue, getter_AddRefs(newNode));
    NotifyPropertyChanged(folderResource, kNC_TotalUnreadMessages, newNode);
	
    //Now see if hasUnreadMessages has changed
    if(oldValue <=0 && newValue >0)
    {
      NotifyPropertyChanged(folderResource, kNC_HasUnreadMessages, kTrueLiteral);
      NotifyAncestors(folder, kNC_SubfoldersHaveUnreadMessages, kTrueLiteral);
    }
    else if(oldValue > 0 && newValue <= 0)
    {
      NotifyPropertyChanged(folderResource, kNC_HasUnreadMessages, kFalseLiteral);
      // this isn't quite right - parents could still have other children with 
      // unread messages. NotifyAncestors will have to figure that out...
      NotifyAncestors(folder, kNC_SubfoldersHaveUnreadMessages, kFalseLiteral);
    }

    //We will have to change the folderTreeName if the unread column is hidden
    NotifyFolderTreeNameChanged(folder, folderResource, newValue);
  }
  return NS_OK;
}

nsresult
nsMsgFolderDataSource::NotifyFolderNameChanged(nsIMsgFolder* aFolder, nsIRDFResource *folderResource)
{
  nsXPIDLString name;
  nsresult rv = aFolder->GetName(getter_Copies(name));

  if (NS_SUCCEEDED(rv)) {
    nsCOMPtr<nsIRDFNode> newNameNode;
    createNode(name.get(), getter_AddRefs(newNameNode), getRDFService());
    NotifyPropertyChanged(folderResource, kNC_Name, newNameNode);
  }
  return NS_OK;
}

nsresult
nsMsgFolderDataSource::NotifyFolderTreeSimpleNameChanged(nsIMsgFolder* aFolder, nsIRDFResource *folderResource)
{
  nsXPIDLString abbreviatedName;
  nsresult rv = GetFolderDisplayName(aFolder, getter_Copies(abbreviatedName));
  if (NS_SUCCEEDED(rv)) {
    nsCOMPtr<nsIRDFNode> newNameNode;
    createNode(abbreviatedName.get(), getter_AddRefs(newNameNode), getRDFService());
    NotifyPropertyChanged(folderResource, kNC_FolderTreeSimpleName, newNameNode);
  }

  return NS_OK;
}

nsresult
nsMsgFolderDataSource::NotifyFolderTreeNameChanged(nsIMsgFolder* aFolder,
                                                   nsIRDFResource* aFolderResource,
                                                   PRInt32 aUnreadMessages)
{
  nsXPIDLString name;
  nsresult rv = GetFolderDisplayName(aFolder, getter_Copies(name));
  if (NS_SUCCEEDED(rv)) {
    nsAutoString newNameString(name);
			
    CreateUnreadMessagesNameString(aUnreadMessages, newNameString);	
			
    nsCOMPtr<nsIRDFNode> newNameNode;
    createNode(newNameString.get(), getter_AddRefs(newNameNode), getRDFService());
    NotifyPropertyChanged(aFolderResource, kNC_FolderTreeName, newNameNode);
  }
  return NS_OK;
}

nsresult 
nsMsgFolderDataSource::NotifyAncestors(nsIMsgFolder *aFolder, nsIRDFResource *aPropertyResource, nsIRDFNode *aNode)
{
  PRBool isServer = PR_FALSE;
  nsresult rv = aFolder->GetIsServer(&isServer);
  NS_ENSURE_SUCCESS(rv,rv);
 
  if (isServer)
    // done, stop
    return NS_OK;
  
  nsCOMPtr <nsIMsgFolder> parentMsgFolder;
  rv = aFolder->GetParentMsgFolder(getter_AddRefs(parentMsgFolder));
  NS_ENSURE_SUCCESS(rv,rv);
  if (!parentMsgFolder)
    return NS_OK;

  rv = parentMsgFolder->GetIsServer(&isServer);
  NS_ENSURE_SUCCESS(rv,rv);
 
  // don't need to notify servers either.
  if (isServer) 
    // done, stop
    return NS_OK;

  nsCOMPtr<nsIRDFResource> parentFolderResource = do_QueryInterface(parentMsgFolder,&rv);
  NS_ENSURE_SUCCESS(rv,rv);

  // if we're setting the subFoldersHaveUnreadMessages property to false, check
  // if the folder really doesn't have subfolders with unread messages.
  if (aPropertyResource == kNC_SubfoldersHaveUnreadMessages && aNode == kFalseLiteral)
  {
    nsCOMPtr <nsIRDFNode> unreadMsgsNode;
    createHasUnreadMessagesNode(parentMsgFolder, PR_TRUE, getter_AddRefs(unreadMsgsNode));
    aNode = unreadMsgsNode;
  }
  NotifyPropertyChanged(parentFolderResource, aPropertyResource, aNode);

  return NotifyAncestors(parentMsgFolder, aPropertyResource, aNode);
}

// New Messages

nsresult
nsMsgFolderDataSource::createNewMessagesNode(nsIMsgFolder *folder, nsIRDFNode **target)
{
  
  nsresult rv;
  
  PRBool isServer;
  rv = folder->GetIsServer(&isServer);
  if (NS_FAILED(rv)) return rv;
  
  *target = kFalseLiteral;
  
  //PRInt32 totalNewMessages;
  PRBool isNewMessages;
  if(!isServer)
  {
    rv = folder->GetHasNewMessages(&isNewMessages);
    if(NS_FAILED(rv)) return rv;
    *target = (isNewMessages) ? kTrueLiteral : kFalseLiteral;
  }
  NS_IF_ADDREF(*target);
  return NS_OK;
}

/**
nsresult
nsMsgFolderDataSource::OnUnreadMessagePropertyChanged(nsIMsgFolder *folder, PRInt32 oldValue, PRInt32 newValue)
{
	nsCOMPtr<nsIRDFResource> folderResource = do_QueryInterface(folder);
	if(folderResource)
	{
		//First send a regular unread message changed notification
		nsCOMPtr<nsIRDFNode> newNode;

		GetNumMessagesNode(newValue, getter_AddRefs(newNode));
		NotifyPropertyChanged(folderResource, kNC_TotalUnreadMessages, newNode);
	
		//Now see if hasUnreadMessages has changed
		nsCOMPtr<nsIRDFNode> oldHasUnreadMessages;
		nsCOMPtr<nsIRDFNode> newHasUnreadMessages;
		if(oldValue <=0 && newValue >0)
		{
			oldHasUnreadMessages = kFalseLiteral;
			newHasUnreadMessages = kTrueLiteral;
			NotifyPropertyChanged(folderResource, kNC_HasUnreadMessages, newHasUnreadMessages);
		}
		else if(oldValue > 0 && newValue <= 0)
		{
			newHasUnreadMessages = kFalseLiteral;
			NotifyPropertyChanged(folderResource, kNC_HasUnreadMessages, newHasUnreadMessages);
		}
  }
  return NS_OK;
}

**/

nsresult
nsMsgFolderDataSource::OnFolderSizePropertyChanged(nsIRDFResource *folderResource, PRInt32 oldValue, PRInt32 newValue)
{
  nsCOMPtr<nsIRDFNode> newNode;
  GetFolderSizeNode(newValue, getter_AddRefs(newNode));
  NotifyPropertyChanged(folderResource, kNC_FolderSize, newNode);
  return NS_OK;
}

nsresult
nsMsgFolderDataSource::OnTotalMessagePropertyChanged(nsIRDFResource *folderResource, PRInt32 oldValue, PRInt32 newValue)
{
  nsCOMPtr<nsIRDFNode> newNode;
  GetNumMessagesNode(newValue, getter_AddRefs(newNode));
  NotifyPropertyChanged(folderResource, kNC_TotalMessages, newNode);
  return NS_OK;
}

nsresult 
nsMsgFolderDataSource::GetNumMessagesNode(PRInt32 aNumMessages, nsIRDFNode **node)
{
  PRUint32 numMessages = aNumMessages;
  if(numMessages == kDisplayQuestionCount)
    createNode(NS_LITERAL_STRING("???").get(), node, getRDFService());
  else if (numMessages == kDisplayBlankCount || numMessages == 0)
    createNode(EmptyString().get(), node, getRDFService());
  else
    createIntNode(numMessages, node, getRDFService());
  return NS_OK;
}

#define DIVISIONWITHCEIL(num, div) (num/div+((num%div>0)?1:0))

nsresult 
nsMsgFolderDataSource::GetFolderSizeNode(PRInt32 aFolderSize, nsIRDFNode **aNode)
{
  PRUint32 folderSize = aFolderSize;
  if (folderSize == kDisplayBlankCount || folderSize == 0)
    createNode(EmptyString().get(), aNode, getRDFService());
  else if(folderSize == kDisplayQuestionCount)
    createNode(NS_LITERAL_STRING("???").get(), aNode, getRDFService());
  else
  {
    nsAutoString sizeString;
    // use Round or Ceil - bug #251202
    folderSize = DIVISIONWITHCEIL(folderSize, 1024);  // normalize into k;
    PRBool sizeInMB = (folderSize > 999); // 999, not 1024 - bug #251204

    // kKiloByteString/kMegaByteString are localized strings that we use
    // to get the right format to add on the "KB"/"MB" or equivalent
    nsTextFormatter::ssprintf(sizeString,
                              (sizeInMB) ? kMegaByteString : kKiloByteString,
                              (sizeInMB) ? DIVISIONWITHCEIL(folderSize, 1024) : folderSize);
    createNode(sizeString.get(), aNode, getRDFService());
  }
  return NS_OK;
}

nsresult
nsMsgFolderDataSource::createFolderChildNode(nsIMsgFolder *folder,
                                             nsIRDFNode **target)
{
  nsCOMPtr<nsIEnumerator> subFolders;
  nsresult rv = folder->GetSubFolders(getter_AddRefs(subFolders));
  if (NS_FAILED(rv))
    return NS_RDF_NO_VALUE;
  
  rv = subFolders->First();
  if (NS_SUCCEEDED(rv)) 
  {
    nsCOMPtr<nsISupports> firstFolder;
    rv = subFolders->CurrentItem(getter_AddRefs(firstFolder));
    if (NS_SUCCEEDED(rv)) 
      firstFolder->QueryInterface(NS_GET_IID(nsIRDFResource), (void**)target);
  }
  return NS_FAILED(rv) ? NS_RDF_NO_VALUE : rv;
}


nsresult nsMsgFolderDataSource::DoCopyToFolder(nsIMsgFolder *dstFolder, nsISupportsArray *arguments,
											   nsIMsgWindow *msgWindow, PRBool isMove)
{
  nsresult rv;
  PRUint32 itemCount;
  rv = arguments->Count(&itemCount);
  if (NS_FAILED(rv)) return rv;
  
  //need source folder and at least one item to copy
  if(itemCount < 2)
    return NS_ERROR_FAILURE;
  
  
  nsCOMPtr<nsIMsgFolder> srcFolder(do_QueryElementAt(arguments, 0));
  if(!srcFolder)
    return NS_ERROR_FAILURE;
  
  arguments->RemoveElementAt(0);
  itemCount--;
  
  nsCOMPtr<nsISupportsArray> messageArray;
  NS_NewISupportsArray(getter_AddRefs(messageArray));
  
  for(PRUint32 i = 0; i < itemCount; i++)
  {
    
    nsCOMPtr<nsISupports> supports = getter_AddRefs(arguments->ElementAt(i));
    nsCOMPtr<nsIMsgDBHdr> message(do_QueryInterface(supports));
    if (message)
    {
      messageArray->AppendElement(supports);
    }
    
  }
  
  //Call copyservice with dstFolder, srcFolder, messages, isMove, and txnManager
  nsCOMPtr<nsIMsgCopyService> copyService = 
    do_GetService(NS_MSGCOPYSERVICE_CONTRACTID, &rv);
  NS_ENSURE_SUCCESS(rv,rv);
  
  return copyService->CopyMessages(srcFolder, messageArray, dstFolder, isMove, 
    nsnull, msgWindow, PR_TRUE/* allowUndo */);
}

nsresult nsMsgFolderDataSource::DoFolderCopyToFolder(nsIMsgFolder *dstFolder, nsISupportsArray *arguments,
                                                     nsIMsgWindow *msgWindow, PRBool isMoveFolder)
{
  nsresult rv;
  PRUint32 itemCount;
  rv = arguments->Count(&itemCount);
  if (NS_FAILED(rv)) return rv;
  
  //need at least one item to copy
  if(itemCount < 1)
    return NS_ERROR_FAILURE;
  
  if (!isMoveFolder)   // copy folder not on the same server
  {
    //Call copyservice with dstFolder, srcFolder, folders and isMoveFolder
    nsCOMPtr<nsIMsgCopyService> copyService = do_GetService(NS_MSGCOPYSERVICE_CONTRACTID, &rv); 
    if(NS_SUCCEEDED(rv))
    {
      rv = copyService->CopyFolders(arguments, dstFolder, isMoveFolder, 
        nsnull, msgWindow);
      
    }
  }
  else    //within the same server therefore no need for copy service 
  {
    
    nsCOMPtr<nsIMsgFolder> msgFolder;
    for (PRUint32 i=0;i< itemCount; i++)
    {
      msgFolder = do_QueryElementAt(arguments, i, &rv);
      if (NS_SUCCEEDED(rv))
      {
        rv = dstFolder->CopyFolder(msgFolder, isMoveFolder , msgWindow, nsnull);
        NS_ASSERTION((NS_SUCCEEDED(rv)),"Copy folder failed.");
      }
    }
  }
  
  return rv;
  //return NS_OK;
}

nsresult nsMsgFolderDataSource::DoDeleteFromFolder(
                                                   nsIMsgFolder *folder, nsISupportsArray *arguments, 
                                                   nsIMsgWindow *msgWindow, PRBool reallyDelete)
{
  nsresult rv = NS_OK;
  PRUint32 itemCount;
  rv = arguments->Count(&itemCount);
  if (NS_FAILED(rv)) return rv;
  
  nsCOMPtr<nsISupportsArray> messageArray, folderArray;
  NS_NewISupportsArray(getter_AddRefs(messageArray));
  NS_NewISupportsArray(getter_AddRefs(folderArray));
  
  //Split up deleted items into different type arrays to be passed to the folder
  //for deletion.
  for(PRUint32 item = 0; item < itemCount; item++)
  {
    nsCOMPtr<nsISupports> supports = getter_AddRefs(arguments->ElementAt(item));
    nsCOMPtr<nsIMsgDBHdr> deletedMessage(do_QueryInterface(supports));
    nsCOMPtr<nsIMsgFolder> deletedFolder(do_QueryInterface(supports));
    if (deletedMessage)
    {
      messageArray->AppendElement(supports);
    }
    else if(deletedFolder)
    {
      folderArray->AppendElement(supports);
    }
  }
  PRUint32 cnt;
  rv = messageArray->Count(&cnt);
  if (NS_FAILED(rv)) return rv;
  if (cnt > 0)
    rv = folder->DeleteMessages(messageArray, msgWindow, reallyDelete, PR_FALSE, nsnull, PR_TRUE /*allowUndo*/);
  
  rv = folderArray->Count(&cnt);
  if (NS_FAILED(rv)) return rv;
  if (cnt > 0)
  {
    nsCOMPtr<nsIMsgFolder> folderToDelete = do_QueryElementAt(folderArray, 0);
    PRUint32 folderFlags = 0;
    if (folderToDelete)
    {
      folderToDelete->GetFlags(&folderFlags);
      if (folderFlags & MSG_FOLDER_FLAG_VIRTUAL)
      {
        NS_ENSURE_ARG_POINTER(msgWindow);
        nsCOMPtr<nsIStringBundleService> sBundleService = do_GetService(NS_STRINGBUNDLE_CONTRACTID, &rv);
        nsCOMPtr<nsIStringBundle> sMessengerStringBundle;
        nsXPIDLString confirmMsg;

        if (NS_SUCCEEDED(rv) && sBundleService) 
          rv = sBundleService->CreateBundle(MESSENGER_STRING_URL, getter_AddRefs(sMessengerStringBundle));
        NS_ENSURE_SUCCESS(rv, rv);
        sMessengerStringBundle->GetStringFromName(NS_LITERAL_STRING("confirmSavedSearchDeleteMessage").get(), getter_Copies(confirmMsg));

        nsCOMPtr<nsIPrompt> dialog;
        rv = msgWindow->GetPromptDialog(getter_AddRefs(dialog));
        if (NS_SUCCEEDED(rv))
        {
          PRBool dialogResult;
          rv = dialog->Confirm(nsnull, confirmMsg, &dialogResult);
          if (!dialogResult)
            return NS_OK;
        }
      }
    }
    rv = folder->DeleteSubFolders(folderArray, msgWindow);
  }
  return rv;
}

nsresult nsMsgFolderDataSource::DoNewFolder(nsIMsgFolder *folder, nsISupportsArray *arguments, nsIMsgWindow *window)
{
  nsresult rv = NS_OK;
  nsCOMPtr<nsIRDFLiteral> literal = do_QueryElementAt(arguments, 0, &rv);
  if(NS_SUCCEEDED(rv))
  {
    nsXPIDLString name;
    literal->GetValue(getter_Copies(name));
    
    rv = folder->CreateSubfolder(name, window);
    
  }
  return rv;
}

nsresult nsMsgFolderDataSource::DoFolderAssert(nsIMsgFolder *folder, nsIRDFResource *property, nsIRDFNode *target)
{
  nsresult rv = NS_ERROR_FAILURE;

  if((kNC_Charset == property))
  {
    nsCOMPtr<nsIRDFLiteral> literal(do_QueryInterface(target));
    if(literal)
    {
      const PRUnichar* value;
      rv = literal->GetValueConst(&value);
      if(NS_SUCCEEDED(rv))
        rv = folder->SetCharset(NS_LossyConvertUTF16toASCII(value).get());
    }
    else
      rv = NS_ERROR_FAILURE;
  }
  else if (kNC_Open == property && target == kTrueLiteral)
      rv = folder->ClearFlag(MSG_FOLDER_FLAG_ELIDED);

  return rv;
}

nsresult nsMsgFolderDataSource::DoFolderUnassert(nsIMsgFolder *folder, nsIRDFResource *property, nsIRDFNode *target)
{
  nsresult rv = NS_ERROR_FAILURE;

  if((kNC_Open == property) && target == kTrueLiteral)
      rv = folder->SetFlag(MSG_FOLDER_FLAG_ELIDED);

  return rv;
}

nsresult nsMsgFolderDataSource::DoFolderHasAssertion(nsIMsgFolder *folder,
                                                     nsIRDFResource *property,
                                                     nsIRDFNode *target,
                                                     PRBool tv,
                                                     PRBool *hasAssertion)
{
  nsresult rv = NS_OK;
  if(!hasAssertion)
    return NS_ERROR_NULL_POINTER;
  
  //We're not keeping track of negative assertions on folders.
  if(!tv)
  {
    *hasAssertion = PR_FALSE;
    return NS_OK;
  }
  
  if((kNC_Child == property))
  {
    nsCOMPtr<nsIMsgFolder> childFolder(do_QueryInterface(target, &rv));
    if(NS_SUCCEEDED(rv))
    {
      nsCOMPtr<nsIMsgFolder> childsParent;
      rv = childFolder->GetParent(getter_AddRefs(childsParent));
      *hasAssertion = (NS_SUCCEEDED(rv) && childsParent && folder
        && (childsParent.get() == folder));
    }
  }
  else if ((kNC_Name == property) ||
    (kNC_Open == property) ||
    (kNC_FolderTreeName == property) ||
    (kNC_FolderTreeSimpleName == property) ||
    (kNC_SpecialFolder == property) ||
    (kNC_ServerType == property) ||
    (kNC_IsDeferred == property) ||
    (kNC_RedirectorType == property) ||
    (kNC_CanCreateFoldersOnServer == property) ||
    (kNC_CanFileMessagesOnServer == property) ||
    (kNC_IsServer == property) ||
    (kNC_IsSecure == property) ||
    (kNC_CanSubscribe == property) ||
    (kNC_SupportsOffline == property) ||
    (kNC_CanFileMessages == property) ||
    (kNC_CanCreateSubfolders == property) ||
    (kNC_CanRename == property) ||
    (kNC_CanCompact == property) ||
    (kNC_TotalMessages == property) ||
    (kNC_TotalUnreadMessages == property) ||
    (kNC_FolderSize == property) ||
    (kNC_Charset == property) ||
    (kNC_BiffState == property) ||
    (kNC_HasUnreadMessages == property) ||
    (kNC_NoSelect == property)  ||
    (kNC_Synchronize == property) ||
    (kNC_SyncDisabled == property) ||
    (kNC_VirtualFolder == property) ||
    (kNC_CanSearchMessages == property))
  {
    nsCOMPtr<nsIRDFResource> folderResource(do_QueryInterface(folder, &rv));
    
    if(NS_FAILED(rv))
      return rv;
    
    rv = GetTargetHasAssertion(this, folderResource, property, tv, target, hasAssertion);
  }
  else 
    *hasAssertion = PR_FALSE;
  
  return rv;
  
  
}

nsMsgFlatFolderDataSource::nsMsgFlatFolderDataSource()
{
}

nsMsgFlatFolderDataSource::~nsMsgFlatFolderDataSource()
{
}

nsresult nsMsgFlatFolderDataSource::Init()
{
  nsCOMPtr<nsIMsgAccountManager> am;

  // get a weak ref to the account manager
  if (!mAccountManager) 
  {
    nsresult rv;
    am = do_GetService(NS_MSGACCOUNTMANAGER_CONTRACTID, &rv);
    NS_ENSURE_SUCCESS(rv, rv);
    mAccountManager = do_GetWeakReference(am);
  }
  else
    am = do_QueryReferent(mAccountManager);
  return nsMsgFolderDataSource::Init();
}

void nsMsgFlatFolderDataSource::Cleanup()
{
  m_folders.Clear();
  nsMsgFolderDataSource::Cleanup();
}

NS_IMETHODIMP nsMsgFlatFolderDataSource::GetTarget(nsIRDFResource* source,
                       nsIRDFResource* property,
                       PRBool tv,
                       nsIRDFNode** target)
{
  return (property == kNC_Child) 
    ? NS_RDF_NO_VALUE
    : nsMsgFolderDataSource::GetTarget(source, property, tv, target);
}


NS_IMETHODIMP nsMsgFlatFolderDataSource::GetTargets(nsIRDFResource* source,
                                                nsIRDFResource* property,    
                                                PRBool tv,
                                                nsISimpleEnumerator** targets)
{
  if (kNC_Child != property)
    return nsMsgFolderDataSource::GetTargets(source, property, tv, targets);

  nsresult rv = NS_RDF_NO_VALUE;
  if(!targets)
    return NS_ERROR_NULL_POINTER;

  if (ResourceIsOurRoot(source))
  {
    // need an enumerator that gives all folders with unread
    nsCOMPtr <nsIMsgAccountManager> accountManager = do_GetService(NS_MSGACCOUNTMANAGER_CONTRACTID, &rv);
    NS_ENSURE_SUCCESS(rv,rv);

    nsCOMPtr<nsISupportsArray> allServers;
    rv = accountManager->GetAllServers(getter_AddRefs(allServers));
    nsCOMPtr <nsISupportsArray> allFolders = do_CreateInstance(NS_SUPPORTSARRAY_CONTRACTID, &rv);;
    if (NS_SUCCEEDED(rv) && allServers)
    {
      PRUint32 count = 0;
      allServers->Count(&count);
      PRUint32 i;
      for (i = 0; i < count; i++) 
      {
        nsCOMPtr<nsIMsgIncomingServer> server = do_QueryElementAt(allServers, i);
        if (server)
        {
          nsCOMPtr <nsIMsgFolder> rootFolder;
          server->GetRootFolder(getter_AddRefs(rootFolder));
          if (rootFolder)
          {
            nsCOMPtr<nsIEnumerator> subFolders;
            rv = rootFolder->GetSubFolders(getter_AddRefs(subFolders));

            PRUint32 lastEntry;
            allFolders->Count(&lastEntry);
            rv = rootFolder->ListDescendents(allFolders);
            PRUint32 newLastEntry;
            allFolders->Count(&newLastEntry);
            for (PRUint32 newEntryIndex = lastEntry; newEntryIndex < newLastEntry;)
            {
              nsCOMPtr <nsIMsgFolder> curFolder = do_QueryElementAt(allFolders, newEntryIndex);
              if (!WantsThisFolder(curFolder))
              {
                allFolders->RemoveElementAt(newEntryIndex);
                newLastEntry--;
              }
              else
              {
                // unfortunately, we need a separate array for this since
                // ListDescendents takes an nsISupportsArray. But we want
                // to use an nsCOMArrray for the DS since that's the 
                // preferred mechanism.
                m_folders.AppendObject(curFolder);
                newEntryIndex++;
              }
            }
          }
        }
      }
      return NS_NewArrayEnumerator(targets, allFolders);
    }
  }
  nsSingletonEnumerator* cursor = new nsSingletonEnumerator(property);
  if (cursor == nsnull)
    return NS_ERROR_OUT_OF_MEMORY;
  NS_ADDREF(*targets = cursor);
  return NS_OK;
}


NS_IMETHODIMP nsMsgFlatFolderDataSource::GetURI(char* *aUri)
{
  nsCAutoString uri("rdf:");
  uri.Append(m_dsName);
  return (*aUri = ToNewCString(uri))
    ? NS_OK : NS_ERROR_OUT_OF_MEMORY;
}

NS_IMETHODIMP nsMsgFlatFolderDataSource::HasAssertion(nsIRDFResource* source,
                            nsIRDFResource* property,
                            nsIRDFNode* target,
                            PRBool tv,
                            PRBool* hasAssertion)
{
  nsresult rv;
  
  nsCOMPtr<nsIMsgFolder> folder(do_QueryInterface(source, &rv));
  // we need to check if the folder belongs in this datasource.
  if (NS_SUCCEEDED(rv) && property != kNC_Open && property != kNC_Child)
  {
    if (WantsThisFolder(folder) && (kNC_Child != property))
      return DoFolderHasAssertion(folder, property, target, tv, hasAssertion);
  }
  else if (property == kNC_Child && ResourceIsOurRoot(source)) // check if source is us
  {
    folder = do_QueryInterface(target);
    if (WantsThisFolder(folder))
    {
      *hasAssertion = PR_TRUE;
      return NS_OK;
    }
  }
  *hasAssertion = PR_FALSE;
  return NS_OK;
}

PRBool nsMsgFlatFolderDataSource::ResourceIsOurRoot(nsIRDFResource *resource)
{
  const char *uri;
  nsCAutoString dsUri(m_dsName);
  dsUri.Append(":/");

  resource->GetValueConst(&uri);
  return dsUri.Equals(uri);
}

PRBool nsMsgFlatFolderDataSource::WantsThisFolder(nsIMsgFolder *folder)
{
  NS_ASSERTION(PR_FALSE, "must be overridden");
  return PR_FALSE;
}

nsresult nsMsgFlatFolderDataSource::GetFolderDisplayName(nsIMsgFolder *folder, PRUnichar **folderName)
{
  nsXPIDLString curFolderName;
  folder->GetName(getter_Copies(curFolderName));
  PRUint32 foldersCount = m_folders.Count();
  nsXPIDLString otherFolderName;
  for (PRUint32 index = 0; index < foldersCount; index++)
  {
    if (folder == m_folders[index]) // ignore ourselves.
      continue;
    m_folders[index]->GetName(getter_Copies(otherFolderName));
    if (otherFolderName.Equals(curFolderName))
    {
      nsCOMPtr <nsIMsgIncomingServer> server;
      folder->GetServer(getter_AddRefs(server));
      if (server)
      {
        nsXPIDLString serverName;
        server->GetPrettyName(getter_Copies(serverName));
        curFolderName.Append(NS_LITERAL_STRING(" - "));
        curFolderName.Append(serverName);
        *folderName = ToNewUnicode(curFolderName);
        return (*folderName) ? NS_OK : NS_ERROR_OUT_OF_MEMORY;
      }
    }
  }
  // check if folder name is unique - if not, append account name
  return folder->GetAbbreviatedName(folderName);
}


PRBool nsMsgUnreadFoldersDataSource::WantsThisFolder(nsIMsgFolder *folder)
{
  PRInt32 numUnread;
  folder->GetNumUnread(PR_FALSE, &numUnread);
  return numUnread > 0;
}

nsresult nsMsgUnreadFoldersDataSource::NotifyPropertyChanged(nsIRDFResource *resource, 
                    nsIRDFResource *property, nsIRDFNode *newNode, 
                    nsIRDFNode *oldNode)
{
  // check if it's the has unread property that's changed; if so, see if we need
  // to add this folder to the view.
  // Then, call base class.
  if (kNC_HasUnreadMessages == property)
  {
    nsCOMPtr<nsIMsgFolder> folder(do_QueryInterface(resource));
    if (folder)
    {
      PRInt32 numUnread;
      folder->GetNumUnread(PR_FALSE, &numUnread);
      if (numUnread > 0)
      {
        if (m_folders.IndexOf(folder) == kNotFound)
          m_folders.AppendObject(folder);
        NotifyObservers(kNC_UnreadFolders, kNC_Child, resource, nsnull, PR_TRUE, PR_FALSE);
      }
    }
  }
  return nsMsgFolderDataSource::NotifyPropertyChanged(resource, property, 
                                                newNode, oldNode);
}

PRBool nsMsgFavoriteFoldersDataSource::WantsThisFolder(nsIMsgFolder *folder)
{
  PRUint32 folderFlags;
  folder->GetFlags(&folderFlags);
  return folderFlags & MSG_FOLDER_FLAG_FAVORITE;
}


void nsMsgRecentFoldersDataSource::Cleanup()
{
  m_builtRecentFolders = PR_FALSE;
  m_cutOffDate = 0;
  nsMsgFlatFolderDataSource::Cleanup();
}


PRBool nsMsgRecentFoldersDataSource::WantsThisFolder(nsIMsgFolder *folder)
{
  if (!m_builtRecentFolders)
  {
    nsresult rv;
    nsCOMPtr <nsIMsgAccountManager> accountManager = do_GetService(NS_MSGACCOUNTMANAGER_CONTRACTID, &rv);
    NS_ENSURE_SUCCESS(rv,rv);

    nsCOMPtr<nsISupportsArray> allServers;
    rv = accountManager->GetAllServers(getter_AddRefs(allServers));
    nsCOMPtr <nsISupportsArray> allFolders = do_CreateInstance(NS_SUPPORTSARRAY_CONTRACTID, &rv);;
    if (NS_SUCCEEDED(rv) && allServers)
    {
      PRUint32 count = 0;
      allServers->Count(&count);
      PRUint32 i;
      for (i = 0; i < count; i++) 
      {
        nsCOMPtr<nsIMsgIncomingServer> server = do_QueryElementAt(allServers, i);
        if (server)
        {
          nsCOMPtr <nsIMsgFolder> rootFolder;
          server->GetRootFolder(getter_AddRefs(rootFolder));
          if (rootFolder)
          {
            nsCOMPtr<nsIEnumerator> subFolders;
            rv = rootFolder->GetSubFolders(getter_AddRefs(subFolders));

            PRUint32 lastEntry;
            allFolders->Count(&lastEntry);
            rv = rootFolder->ListDescendents(allFolders);
            PRUint32 newLastEntry;
            allFolders->Count(&newLastEntry);
            for (PRUint32 newEntryIndex = lastEntry; newEntryIndex < newLastEntry; newEntryIndex++)
            {
              nsCOMPtr <nsIMsgFolder> curFolder = do_QueryElementAt(allFolders, newEntryIndex);
              nsXPIDLCString dateStr;
              PRInt32 err;
              curFolder->GetStringProperty(MRU_TIME_PROPERTY, getter_Copies(dateStr));
              PRUint32 curFolderDate = (PRUint32) dateStr.ToInteger(&err);
              if (err)
                curFolderDate = 0;
              if (curFolderDate > m_cutOffDate)
              {
                // if m_folders is "full", replace oldest folder with this folder,
                // and adjust m_cutOffDate so that it's the mrutime 
                // of the "new" oldest folder.
                PRUint32 curFaveFoldersCount = m_folders.Count();
                if (curFaveFoldersCount > m_maxNumFolders)
                {
                  PRUint32 indexOfOldestFolder = 0;
                  PRUint32 oldestFaveDate = 0;
                  PRUint32 newOldestFaveDate = 0;
                  for (PRUint32 index = 0; index < curFaveFoldersCount; )
                  {
                    nsXPIDLCString curFaveFolderDateStr;
                    m_folders[index]->GetStringProperty(MRU_TIME_PROPERTY, getter_Copies(curFaveFolderDateStr));
                    PRUint32 curFaveFolderDate = (PRUint32) curFaveFolderDateStr.ToInteger(&err);
                    if (!oldestFaveDate || curFaveFolderDate < oldestFaveDate)
                    {
                      indexOfOldestFolder = index;
                      newOldestFaveDate = oldestFaveDate;
                      oldestFaveDate = curFaveFolderDate;
                    }
                    if (!newOldestFaveDate || (index != indexOfOldestFolder
                                                && curFaveFolderDate < newOldestFaveDate))
                      newOldestFaveDate = curFaveFolderDate;
                    index++;
                  }
                  if (curFolderDate > oldestFaveDate && m_folders.IndexOf(curFolder) == kNotFound)
                    m_folders.ReplaceObjectAt(curFolder, indexOfOldestFolder);

                  NS_ASSERTION(newOldestFaveDate >= m_cutOffDate, "cutoff date should be getting bigger");
                  m_cutOffDate = newOldestFaveDate;
                }
                else if (m_folders.IndexOf(curFolder) == kNotFound)
                  m_folders.AppendObject(curFolder);
              }
#ifdef DEBUG_David_Bienvenu
              else
              {
                for (PRUint32 index = 0; index < m_folders.Count(); index++)
                {
                  nsXPIDLCString curFaveFolderDateStr;
                  m_folders[index]->GetStringProperty(MRU_TIME_PROPERTY, getter_Copies(curFaveFolderDateStr));
                  PRUint32 curFaveFolderDate = (PRUint32) curFaveFolderDateStr.ToInteger(&err);
                  NS_ASSERTION(curFaveFolderDate > curFolderDate, "folder newer then faves but not added");
                }
              }
#endif
            }
          }
        }
      }
    }
  }
  m_builtRecentFolders = PR_TRUE;
  return m_folders.IndexOf(folder) != kNotFound;
}



nsresult nsMsgRecentFoldersDataSource::NotifyPropertyChanged(nsIRDFResource *resource, 
                    nsIRDFResource *property, nsIRDFNode *newNode, 
                    nsIRDFNode *oldNode)
{
  // check if it's the has new property that's changed; if so, see if we need
  // to add this folder to the view.
  // Then, call base class.
  if (kNC_NewMessages == property)
  {
    nsCOMPtr<nsIMsgFolder> folder(do_QueryInterface(resource));
    if (folder)
    {
      PRBool hasNewMessages;
      folder->GetHasNewMessages(&hasNewMessages);
      if (hasNewMessages > 0)
      {
        if (m_folders.IndexOf(folder) == kNotFound)
        {
          m_folders.AppendObject(folder);
          NotifyObservers(kNC_RecentFolders, kNC_Child, resource, nsnull, PR_TRUE, PR_FALSE);
        }
      }
    }
  }
  return nsMsgFolderDataSource::NotifyPropertyChanged(resource, property, 
                                                newNode, oldNode);
}
