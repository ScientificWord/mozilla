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
 *   Seth Spitzer <sspitzer@netscape.com>
 *   David Bienvenu <bienvenu@nventure.com>
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

#include "nsNntpIncomingServer.h"
#include "nsXPIDLString.h"
#include "nsIPrefBranch.h"
#include "nsIPrefService.h"
#include "nsIMsgNewsFolder.h"
#include "nsIMsgFolder.h"
#include "nsIFileSpec.h"
#include "nsCOMPtr.h"
#include "nsINntpService.h"
#include "nsINNTPProtocol.h"
#include "nsMsgNewsCID.h"
#include "nsNNTPProtocol.h"
#include "nsIDirectoryService.h"
#include "nsAppDirectoryServiceDefs.h"
#include "nsInt64.h"
#include "nsMsgUtils.h"
#include "nsIPrompt.h"
#include "nsIStringBundle.h"
#include "nntpCore.h"
#include "nsIWindowWatcher.h"
#include "nsITreeColumns.h"
#include "nsIDOMElement.h"
#include "nsMsgFolderFlags.h"
#include "nsMsgI18N.h"
#include "nsUnicharUtils.h"
#include "nsEscape.h"
#include "nsISupportsObsolete.h"

#define INVALID_VERSION         0
#define VALID_VERSION           1
#define NEW_NEWS_DIR_NAME       "News"
#define PREF_MAIL_NEWSRC_ROOT   "mail.newsrc_root"
#define PREF_MAIL_NEWSRC_ROOT_REL "mail.newsrc_root-rel"
#define PREF_MAILNEWS_VIEW_DEFAULT_CHARSET "mailnews.view_default_charset"
#define HOSTINFO_FILE_NAME      "hostinfo.dat"

#define NEWS_DELIMITER          '.'

// this platform specific junk is so the newsrc filenames we create 
// will resemble the migrated newsrc filenames.
#if defined(XP_UNIX) || defined(XP_BEOS)
#define NEWSRC_FILE_PREFIX "newsrc-"
#define NEWSRC_FILE_SUFFIX ""
#else
#define NEWSRC_FILE_PREFIX ""
#define NEWSRC_FILE_SUFFIX ".rc"
#endif /* XP_UNIX || XP_BEOS */

// ###tw  This really ought to be the most
// efficient file reading size for the current
// operating system.
#define HOSTINFO_FILE_BUFFER_SIZE 1024

static NS_DEFINE_CID(kSubscribableServerCID, NS_SUBSCRIBABLESERVER_CID);

NS_IMPL_ADDREF_INHERITED(nsNntpIncomingServer, nsMsgIncomingServer)
NS_IMPL_RELEASE_INHERITED(nsNntpIncomingServer, nsMsgIncomingServer)

NS_INTERFACE_MAP_BEGIN(nsNntpIncomingServer)
    NS_INTERFACE_MAP_ENTRY(nsINntpIncomingServer)
    NS_INTERFACE_MAP_ENTRY(nsIUrlListener)
    NS_INTERFACE_MAP_ENTRY(nsISubscribableServer)
    NS_INTERFACE_MAP_ENTRY(nsITreeView)
NS_INTERFACE_MAP_END_INHERITING(nsMsgIncomingServer)

nsNntpIncomingServer::nsNntpIncomingServer() : nsMsgLineBuffer(nsnull, PR_FALSE)
{    
  mNewsrcHasChanged = PR_FALSE;
  mGroupsEnumerator = nsnull;
  NS_NewISupportsArray(getter_AddRefs(m_connectionCache));

  mHostInfoLoaded = PR_FALSE;
  mHostInfoHasChanged = PR_FALSE;
  mVersion = INVALID_VERSION;

  mHostInfoStream = nsnull;

  mLastGroupDate = 0;
  mUniqueId = 0;
  mHasSeenBeginGroups = PR_FALSE;
  mPostingAllowed = PR_FALSE;
  m_userAuthenticated = PR_FALSE;
  mLastUpdatedTime = 0;

  // these atoms are used for subscribe search
  mSubscribedAtom = do_GetAtom("subscribed");
  mNntpAtom = do_GetAtom("nntp");

  // we have server wide and per group filters
  m_canHaveFilters = PR_TRUE;

  SetupNewsrcSaveTimer();
}

nsNntpIncomingServer::~nsNntpIncomingServer()
{
    nsresult rv;

    if (mGroupsEnumerator) {
        delete mGroupsEnumerator;
        mGroupsEnumerator = nsnull;
    }

    if (mNewsrcSaveTimer) {
        mNewsrcSaveTimer->Cancel();
        mNewsrcSaveTimer = nsnull;
    }

    if (mHostInfoStream) {
        mHostInfoStream->close();
        delete mHostInfoStream;
        mHostInfoStream = nsnull;
    }

    rv = ClearInner();
    NS_ASSERTION(NS_SUCCEEDED(rv), "ClearInner failed");

    rv = CloseCachedConnections();
    NS_ASSERTION(NS_SUCCEEDED(rv), "CloseCachedConnections failed");
}

NS_IMPL_SERVERPREF_BOOL(nsNntpIncomingServer, NotifyOn, "notify.on")
NS_IMPL_SERVERPREF_BOOL(nsNntpIncomingServer, MarkOldRead, "mark_old_read")
NS_IMPL_SERVERPREF_BOOL(nsNntpIncomingServer, Abbreviate, "abbreviate")
NS_IMPL_SERVERPREF_BOOL(nsNntpIncomingServer, PushAuth, "always_authenticate")
NS_IMPL_SERVERPREF_BOOL(nsNntpIncomingServer, SingleSignon, "singleSignon")
NS_IMPL_SERVERPREF_INT(nsNntpIncomingServer, MaxArticles, "max_articles")

NS_IMETHODIMP
nsNntpIncomingServer::GetNewsrcFilePath(nsIFileSpec **aNewsrcFilePath)
{
  nsresult rv;
  if (mNewsrcFilePath)
  {
    *aNewsrcFilePath = mNewsrcFilePath;
    NS_IF_ADDREF(*aNewsrcFilePath);
    return NS_OK;
  }

  rv = GetFileValue("newsrc.file", aNewsrcFilePath);
  if (NS_SUCCEEDED(rv) && *aNewsrcFilePath) 
  {
    mNewsrcFilePath = *aNewsrcFilePath;
    return rv;
  }

  rv = GetNewsrcRootPath(getter_AddRefs(mNewsrcFilePath));
  if (NS_FAILED(rv)) return rv;

  nsXPIDLCString hostname;
  rv = GetHostName(getter_Copies(hostname));
  if (NS_FAILED(rv)) return rv;

  // set the leaf name to "dummy", and then call MakeUnique with a suggested leaf name
  rv = mNewsrcFilePath->AppendRelativeUnixPath("dummy");
  if (NS_FAILED(rv)) return rv;
  nsCAutoString newsrcFileName(NEWSRC_FILE_PREFIX);
  newsrcFileName.Append(hostname);
  newsrcFileName.Append(NEWSRC_FILE_SUFFIX);
  rv = mNewsrcFilePath->MakeUniqueWithSuggestedName(newsrcFileName.get());
  if (NS_FAILED(rv)) return rv;

  rv = SetNewsrcFilePath(mNewsrcFilePath);
  if (NS_FAILED(rv)) return rv;

  *aNewsrcFilePath = mNewsrcFilePath;
  NS_ADDREF(*aNewsrcFilePath);

  return NS_OK;
}     

NS_IMETHODIMP
nsNntpIncomingServer::SetNewsrcFilePath(nsIFileSpec *spec)
{
    nsresult rv;
    if (!spec) {
      return NS_ERROR_FAILURE;
    }

    PRBool exists;

    rv = spec->Exists(&exists);
    if (!exists) {
      rv = spec->Touch();
      if (NS_FAILED(rv)) return rv;
    }
    return SetFileValue("newsrc.file", spec);
}          

NS_IMETHODIMP
nsNntpIncomingServer::GetLocalStoreType(char **type)
{
    NS_ENSURE_ARG_POINTER(type);
    *type = nsCRT::strdup("news");
    return NS_OK;
}

NS_IMETHODIMP
nsNntpIncomingServer::SetNewsrcRootPath(nsIFileSpec *aNewsrcRootPath)
{
    NS_ENSURE_ARG(aNewsrcRootPath);
    nsFileSpec spec;

    nsresult rv = aNewsrcRootPath->GetFileSpec(&spec);
    NS_ENSURE_SUCCESS(rv, rv);
    nsCOMPtr<nsILocalFile> localFile;
    NS_FileSpecToIFile(&spec, getter_AddRefs(localFile));
    if (!localFile) return NS_ERROR_FAILURE;
    
    return NS_SetPersistentFile(PREF_MAIL_NEWSRC_ROOT_REL, PREF_MAIL_NEWSRC_ROOT, localFile);
}

NS_IMETHODIMP
nsNntpIncomingServer::GetNewsrcRootPath(nsIFileSpec **aNewsrcRootPath)
{
    NS_ENSURE_ARG_POINTER(aNewsrcRootPath);
    *aNewsrcRootPath = nsnull;
    
    PRBool havePref;
    nsCOMPtr<nsILocalFile> localFile;    
    nsresult rv = NS_GetPersistentFile(PREF_MAIL_NEWSRC_ROOT_REL,
                              PREF_MAIL_NEWSRC_ROOT,
                              NS_APP_NEWS_50_DIR,
                              havePref,
                              getter_AddRefs(localFile));

    NS_ENSURE_SUCCESS(rv, rv);

    PRBool exists;
    rv = localFile->Exists(&exists);
    if (NS_SUCCEEDED(rv) && !exists)
        rv = localFile->Create(nsIFile::DIRECTORY_TYPE, 0775);
    NS_ENSURE_SUCCESS(rv, rv);
    
    // Make the resulting nsIFileSpec
    // TODO: Convert arg to nsILocalFile and avoid this
    nsCOMPtr<nsIFileSpec> outSpec;
    rv = NS_NewFileSpecFromIFile(localFile, getter_AddRefs(outSpec));
    NS_ENSURE_SUCCESS(rv, rv);
    
    if (!havePref || !exists) 
    {
        rv = NS_SetPersistentFile(PREF_MAIL_NEWSRC_ROOT_REL, PREF_MAIL_NEWSRC_ROOT, localFile);
        NS_ASSERTION(NS_SUCCEEDED(rv), "Failed to set root dir pref.");
    }
        
    NS_IF_ADDREF(*aNewsrcRootPath = outSpec);
    return rv;
}

/* static */ void nsNntpIncomingServer::OnNewsrcSaveTimer(nsITimer *timer, void *voidIncomingServer)
{
	nsNntpIncomingServer *incomingServer = (nsNntpIncomingServer*)voidIncomingServer;
	incomingServer->WriteNewsrcFile();		
}


nsresult nsNntpIncomingServer::SetupNewsrcSaveTimer()
{
	nsInt64 ms(300000);   // hard code, 5 minutes.
	//Convert biffDelay into milliseconds
	PRUint32 timeInMSUint32 = (PRUint32)ms;
	//Can't currently reset a timer when it's in the process of
	//calling Notify. So, just release the timer here and create a new one.
	if(mNewsrcSaveTimer)
	{
		mNewsrcSaveTimer->Cancel();
	}
    mNewsrcSaveTimer = do_CreateInstance("@mozilla.org/timer;1");
	mNewsrcSaveTimer->InitWithFuncCallback(OnNewsrcSaveTimer, (void*)this, timeInMSUint32, 
                                           nsITimer::TYPE_REPEATING_SLACK);

    return NS_OK;
}

NS_IMETHODIMP
nsNntpIncomingServer::SetCharset(const nsACString & aCharset)
{
	nsresult rv;
	rv = SetCharValue("charset", PromiseFlatCString(aCharset).get());
	return rv;
}

NS_IMETHODIMP
nsNntpIncomingServer::GetCharset(nsACString & aCharset)
{
    nsresult rv; 
    nsXPIDLCString serverCharset;
    //first we get the per-server settings mail.server.<serverkey>.charset
    rv = GetCharValue("charset",getter_Copies(serverCharset));

    //if the per-server setting is empty,we get the default charset from 
    //mailnews.view_default_charset setting and set it as per-server preference.
    if(serverCharset.IsEmpty()){
        nsXPIDLString defaultCharset;
        rv = NS_GetLocalizedUnicharPreferenceWithDefault(nsnull,
             PREF_MAILNEWS_VIEW_DEFAULT_CHARSET,
             NS_LITERAL_STRING("ISO-8859-1"), defaultCharset);
        LossyCopyUTF16toASCII(defaultCharset,serverCharset);
        SetCharset(serverCharset);
    }
#ifdef DEBUG_holywen
        printf("default charset for the server is %s\n", 
               (const char *)serverCharset);
#endif
    aCharset = serverCharset;
    return NS_OK;
}

NS_IMETHODIMP
nsNntpIncomingServer::WriteNewsrcFile()
{
    nsresult rv;

    PRBool newsrcHasChanged;
    rv = GetNewsrcHasChanged(&newsrcHasChanged);
    if (NS_FAILED(rv)) return rv;

#ifdef DEBUG_NEWS
	nsXPIDLCString hostname;
	rv = GetHostName(getter_Copies(hostname));
	if (NS_FAILED(rv)) return rv;
#endif /* DEBUG_NEWS */

    if (newsrcHasChanged) {        
#ifdef DEBUG_NEWS
        printf("write newsrc file for %s\n", (const char *)hostname);
#endif
        nsCOMPtr <nsIFileSpec> newsrcFile;
        rv = GetNewsrcFilePath(getter_AddRefs(newsrcFile));
	    if (NS_FAILED(rv)) return rv;

        nsFileSpec newsrcFileSpec;
        rv = newsrcFile->GetFileSpec(&newsrcFileSpec);
        if (NS_FAILED(rv)) return rv;

        nsIOFileStream newsrcStream(newsrcFileSpec, (PR_RDWR | PR_CREATE_FILE | PR_TRUNCATE));

        nsCOMPtr<nsIEnumerator> subFolders;
        nsCOMPtr<nsIMsgFolder> rootFolder;
        rv = GetRootFolder(getter_AddRefs(rootFolder));
        if (NS_FAILED(rv)) return rv;

        nsCOMPtr <nsIMsgNewsFolder> newsFolder = do_QueryInterface(rootFolder, &rv);
        if (NS_FAILED(rv)) return rv;

        nsXPIDLCString optionLines;
        rv = newsFolder->GetOptionLines(getter_Copies(optionLines));
        if (NS_SUCCEEDED(rv) && ((const char *)optionLines)) {
               newsrcStream << (const char *)optionLines;
#ifdef DEBUG_NEWS
               printf("option lines:\n%s",(const char *)optionLines);
#endif /* DEBUG_NEWS */
        }
#ifdef DEBUG_NEWS
        else {
            printf("no option lines to write out\n");
        }
#endif /* DEBUG_NEWS */

        nsXPIDLCString unsubscribedLines;
        rv = newsFolder->GetUnsubscribedNewsgroupLines(getter_Copies(unsubscribedLines));
        if (NS_SUCCEEDED(rv) && ((const char *)unsubscribedLines)) {
               newsrcStream << (const char *)unsubscribedLines;
#ifdef DEBUG_NEWS
               printf("unsubscribedLines:\n%s",(const char *)unsubscribedLines);
#endif /* DEBUG_NEWS */
        }
#ifdef DEBUG_NEWS
        else {
            printf("no unsubscribed lines to write out\n");
        } 
#endif /* DEBUG_NEWS */

        rv = rootFolder->GetSubFolders(getter_AddRefs(subFolders));
        if (NS_FAILED(rv)) return rv;

        nsAdapterEnumerator *simpleEnumerator = new nsAdapterEnumerator(subFolders);
        if (simpleEnumerator == nsnull) return NS_ERROR_OUT_OF_MEMORY;

        PRBool moreFolders;
        
        while (NS_SUCCEEDED(simpleEnumerator->HasMoreElements(&moreFolders)) && moreFolders) {
            nsCOMPtr<nsISupports> child;
            rv = simpleEnumerator->GetNext(getter_AddRefs(child));
            if (NS_SUCCEEDED(rv) && child) {
                newsFolder = do_QueryInterface(child, &rv);
                if (NS_SUCCEEDED(rv) && newsFolder) {
                    nsXPIDLCString newsrcLine;
                    rv = newsFolder->GetNewsrcLine(getter_Copies(newsrcLine));
                    if (NS_SUCCEEDED(rv) && ((const char *)newsrcLine)) {
                        // write the line to the newsrc file
                        newsrcStream << (const char *)newsrcLine;
                    }
                }
            }
        }
        delete simpleEnumerator;

        newsrcStream.close();
        
        rv = SetNewsrcHasChanged(PR_FALSE);
		if (NS_FAILED(rv)) return rv;
    }
#ifdef DEBUG_NEWS
    else {
        printf("no need to write newsrc file for %s, it was not dirty\n", (const char *)hostname);
    }
#endif /* DEBUG_NEWS */

    return NS_OK;
}

NS_IMETHODIMP
nsNntpIncomingServer::SetNewsrcHasChanged(PRBool aNewsrcHasChanged)
{
    mNewsrcHasChanged = aNewsrcHasChanged;
    return NS_OK;
}

NS_IMETHODIMP
nsNntpIncomingServer::GetNewsrcHasChanged(PRBool *aNewsrcHasChanged)
{
    if (!aNewsrcHasChanged) return NS_ERROR_NULL_POINTER;

    *aNewsrcHasChanged = mNewsrcHasChanged;
    return NS_OK;
}

NS_IMPL_GETSET(nsNntpIncomingServer, UserAuthenticated, PRBool, m_userAuthenticated)

NS_IMETHODIMP
nsNntpIncomingServer::CloseCachedConnections()
{
  nsresult rv;
  PRUint32 cnt;
  nsCOMPtr<nsINNTPProtocol> connection;

  // iterate through the connection cache and close the connections.
  if (m_connectionCache)
  {
    rv = m_connectionCache->Count(&cnt);
    if (NS_FAILED(rv)) return rv;
    for (PRUint32 i = 0; i < cnt; i++) 
	  {
      connection = do_QueryElementAt(m_connectionCache, 0);
		  if (connection)
      {
    	  rv = connection->CloseConnection();
        RemoveConnection(connection);
      }
	  }
  }
  rv = WriteNewsrcFile();
  if (NS_FAILED(rv)) return rv;

  rv = WriteHostInfoFile(); 
  if (NS_FAILED(rv)) return rv;
	
  return NS_OK;
}

NS_IMPL_SERVERPREF_INT(nsNntpIncomingServer, MaximumConnectionsNumber,
                       "max_cached_connections")

PRBool
nsNntpIncomingServer::ConnectionTimeOut(nsINNTPProtocol* aConnection)
{
    PRBool retVal = PR_FALSE;
    if (!aConnection) return retVal;
    nsresult rv;

    PRTime cacheTimeoutLimits;

    LL_I2L(cacheTimeoutLimits, 170 * 1000000); // 170 seconds in microseconds
    PRTime lastActiveTimeStamp;
    rv = aConnection->GetLastActiveTimeStamp(&lastActiveTimeStamp);

    PRTime elapsedTime;
    LL_SUB(elapsedTime, PR_Now(), lastActiveTimeStamp);
    PRTime t;
    LL_SUB(t, elapsedTime, cacheTimeoutLimits);
    if (LL_GE_ZERO(t))
    {
#ifdef DEBUG_seth
      printf("XXX connection timed out, close it, and remove it from the connection cache\n");
#endif
      aConnection->CloseConnection();
            m_connectionCache->RemoveElement(aConnection);
            retVal = PR_TRUE;
    }
    return retVal;
}


nsresult
nsNntpIncomingServer::CreateProtocolInstance(nsINNTPProtocol ** aNntpConnection, nsIURI *url,
                                             nsIMsgWindow *aMsgWindow)
{
  // create a new connection and add it to the connection cache
  // we may need to flag the protocol connection as busy so we don't get
  // a race 
  // condition where someone else goes through this code 
  nsNNTPProtocol * protocolInstance = new nsNNTPProtocol(url, aMsgWindow);
  if (!protocolInstance)
    return NS_ERROR_OUT_OF_MEMORY;

  nsresult rv = protocolInstance->QueryInterface(NS_GET_IID(nsINNTPProtocol), (void **) aNntpConnection);
  // take the protocol instance and add it to the connectionCache
  if (NS_SUCCEEDED(rv) && *aNntpConnection)
    m_connectionCache->AppendElement(*aNntpConnection);
  return rv;
}

/* By default, allow the user to open at most this many connections to one news host */
#define kMaxConnectionsPerHost 2

NS_IMETHODIMP
nsNntpIncomingServer::GetNntpConnection(nsIURI * aUri, nsIMsgWindow *aMsgWindow,
                                           nsINNTPProtocol ** aNntpConnection)
{
  nsresult rv = NS_OK;
  nsCOMPtr<nsINNTPProtocol> connection;
  nsCOMPtr<nsINNTPProtocol> freeConnection;
  PRBool isBusy = PR_TRUE;


  PRInt32 maxConnections = kMaxConnectionsPerHost; 
  rv = GetMaximumConnectionsNumber(&maxConnections);
  if (NS_FAILED(rv) || maxConnections == 0)
  {
    maxConnections = kMaxConnectionsPerHost;
    rv = SetMaximumConnectionsNumber(maxConnections);
  }
  else if (maxConnections < 1)
  {   // forced to use at least 1
    maxConnections = 1;
    rv = SetMaximumConnectionsNumber(maxConnections);
  }

  *aNntpConnection = nsnull;
  // iterate through the connection cache for a connection that can handle this url.
  PRUint32 cnt;

  rv = m_connectionCache->Count(&cnt);
  if (NS_FAILED(rv)) return rv;
#ifdef DEBUG_seth
  printf("XXX there are %d nntp connections in the conn cache.\n", (int)cnt);
#endif
  for (PRUint32 i = 0; i < cnt && isBusy; i++) 
  {
    connection = do_QueryElementAt(m_connectionCache, i);
    if (connection)
        rv = connection->GetIsBusy(&isBusy);
    if (NS_FAILED(rv)) 
    {
        connection = nsnull;
        continue;
    }
    if (!freeConnection && !isBusy && connection)
    {
       freeConnection = connection;
    }
  }
    
  if (ConnectionTimeOut(freeConnection))
      freeConnection = nsnull;

  // if we got here and we have a connection, then we should return it!
  if (!isBusy && freeConnection)
  {
    *aNntpConnection = freeConnection;
    freeConnection->SetIsCachedConnection(PR_TRUE);
    NS_IF_ADDREF(*aNntpConnection);
  }
  else // have no queueing mechanism - just create the protocol instance.
  {
    rv = CreateProtocolInstance(aNntpConnection, aUri, aMsgWindow);
  }
  return rv;
}


/* void RemoveConnection (in nsINNTPProtocol aNntpConnection); */
NS_IMETHODIMP nsNntpIncomingServer::RemoveConnection(nsINNTPProtocol *aNntpConnection)
{
    if (aNntpConnection)
        m_connectionCache->RemoveElement(aNntpConnection);

    return NS_OK;
}


NS_IMETHODIMP 
nsNntpIncomingServer::PerformExpand(nsIMsgWindow *aMsgWindow)
{
  // Get news.update_unread_on_expand pref
  nsresult rv; 
  PRBool updateUnreadOnExpand = PR_TRUE;
  nsCOMPtr<nsIPrefBranch> prefBranch = do_GetService(NS_PREFSERVICE_CONTRACTID, &rv);
  if (NS_SUCCEEDED(rv))
    prefBranch->GetBoolPref("news.update_unread_on_expand", &updateUnreadOnExpand);
  
  // Only if news.update_unread_on_expand is true do we update the unread counts
  if (updateUnreadOnExpand) 
  {
    // a user might have a new server without any groups.
    // if so, bail out.  no need to establish a connection to the server
    PRInt32 numGroups = 0;
    rv = GetNumGroupsNeedingCounts(&numGroups);
    NS_ENSURE_SUCCESS(rv,rv);
    
    if (!numGroups)
      return NS_OK;
    
    nsCOMPtr<nsINntpService> nntpService = do_GetService(NS_NNTPSERVICE_CONTRACTID, &rv);
    NS_ENSURE_SUCCESS(rv,rv);
  
    rv = nntpService->UpdateCounts(this, aMsgWindow);
    NS_ENSURE_SUCCESS(rv,rv);
  }
  return NS_OK;
}

NS_IMETHODIMP 
nsNntpIncomingServer::GetNumGroupsNeedingCounts(PRInt32 *aNumGroupsNeedingCounts)
{
    nsCOMPtr<nsIEnumerator> subFolders;
    nsCOMPtr<nsIMsgFolder> rootFolder;
 
    nsresult rv = GetRootFolder(getter_AddRefs(rootFolder));
    if (NS_FAILED(rv)) return rv;

    PRBool hasSubFolders = PR_FALSE;
    rv = rootFolder->GetHasSubFolders(&hasSubFolders);
    if (NS_FAILED(rv)) return rv;

    if (!hasSubFolders) {
        *aNumGroupsNeedingCounts = 0;
        return NS_OK;
    }

    rv = rootFolder->GetSubFolders(getter_AddRefs(subFolders));
    if (NS_FAILED(rv)) return rv;

    if (mGroupsEnumerator) {
        delete mGroupsEnumerator;
        mGroupsEnumerator = nsnull;
    }
    mGroupsEnumerator = new nsAdapterEnumerator(subFolders);
    if (mGroupsEnumerator == nsnull) return NS_ERROR_OUT_OF_MEMORY;

	PRUint32 count = 0;
	rv = rootFolder->Count(&count);
    if (NS_FAILED(rv)) return rv;
		
	*aNumGroupsNeedingCounts = (PRInt32) count;
	return NS_OK;
}

NS_IMETHODIMP
nsNntpIncomingServer::GetFirstGroupNeedingCounts(nsISupports **aFirstGroupNeedingCounts)
{
  nsresult rv;
  
  if (!aFirstGroupNeedingCounts) return NS_ERROR_NULL_POINTER;
  
  PRBool moreFolders;
  if (!mGroupsEnumerator) return NS_ERROR_FAILURE;
  
  rv = mGroupsEnumerator->HasMoreElements(&moreFolders);
  if (NS_FAILED(rv)) return rv;
  
  if (!moreFolders) 
  {
    *aFirstGroupNeedingCounts = nsnull;
    delete mGroupsEnumerator;
    mGroupsEnumerator = nsnull;
    return NS_OK; // this is not an error - it just means we reached the end of the groups.
  }
  
  do 
  {
    rv = mGroupsEnumerator->GetNext(aFirstGroupNeedingCounts);
    if (NS_FAILED(rv)) return rv;
    if (!*aFirstGroupNeedingCounts) return NS_ERROR_FAILURE;
    nsCOMPtr <nsIMsgFolder> folder;
    (*aFirstGroupNeedingCounts)->QueryInterface(NS_GET_IID(nsIMsgFolder), getter_AddRefs(folder));
    PRUint32 folderFlags;
    folder->GetFlags(&folderFlags);
    if (folderFlags & MSG_FOLDER_FLAG_VIRTUAL)
      continue;
    else
      break;
  }
  while (PR_TRUE);
  return NS_OK;
}

NS_IMETHODIMP
nsNntpIncomingServer::DisplaySubscribedGroup(nsIMsgNewsFolder *aMsgFolder, PRInt32 aFirstMessage, PRInt32 aLastMessage, PRInt32 aTotalMessages)
{
	nsresult rv;

	if (!aMsgFolder) return NS_ERROR_NULL_POINTER;
#ifdef DEBUG_NEWS
	printf("DisplaySubscribedGroup(...,%ld,%ld,%ld)\n",aFirstMessage,aLastMessage,aTotalMessages);
#endif
	rv = aMsgFolder->UpdateSummaryFromNNTPInfo(aFirstMessage,aLastMessage,aTotalMessages);
	return rv;
}

NS_IMETHODIMP
nsNntpIncomingServer::PerformBiff(nsIMsgWindow *aMsgWindow)
{
#ifdef DEBUG_NEWS
	printf("PerformBiff for nntp\n");
#endif
	return PerformExpand(nsnull);
}

NS_IMETHODIMP nsNntpIncomingServer::GetServerRequiresPasswordForBiff(PRBool *aServerRequiresPasswordForBiff)
{
  NS_ENSURE_ARG_POINTER(aServerRequiresPasswordForBiff);
	*aServerRequiresPasswordForBiff = PR_FALSE;  // for news, biff is getting the unread counts
	return NS_OK;
}

NS_IMETHODIMP
nsNntpIncomingServer::OnStartRunningUrl(nsIURI *url)
{
	return NS_OK;
}

NS_IMETHODIMP
nsNntpIncomingServer::OnStopRunningUrl(nsIURI *url, nsresult exitCode)
{
	nsresult rv;
	rv = UpdateSubscribed();
	if (NS_FAILED(rv)) return rv;

	rv = StopPopulating(mMsgWindow);
	if (NS_FAILED(rv)) return rv;

	return NS_OK;
}


PRBool
checkIfSubscribedFunction(nsCString &aElement, void *aData)
{
    if (aElement.Equals(*NS_STATIC_CAST(nsACString *, aData))) {
        return PR_FALSE;
    }
    else {
        return PR_TRUE;
    }
}


NS_IMETHODIMP
nsNntpIncomingServer::ContainsNewsgroup(const nsACString &name,
                                        PRBool *containsGroup)
{
    if (name.IsEmpty()) return NS_ERROR_FAILURE;
    nsCAutoString unescapedName;
    NS_UnescapeURL(PromiseFlatCString(name), 
                   esc_FileBaseName|esc_Forced|esc_AlwaysCopy, unescapedName);

    *containsGroup = !(mSubscribedNewsgroups.EnumerateForwards(
                       nsCStringArrayEnumFunc(checkIfSubscribedFunction),
                       (void *) &unescapedName));
    return NS_OK;
}

NS_IMETHODIMP
nsNntpIncomingServer::SubscribeToNewsgroup(const nsACString &aName)
{
    NS_ASSERTION(!aName.IsEmpty(), "no name");
    if (aName.IsEmpty()) return NS_ERROR_FAILURE;

    nsCOMPtr<nsIMsgFolder> msgfolder;
    nsresult rv = GetRootMsgFolder(getter_AddRefs(msgfolder));
    if (NS_FAILED(rv)) return rv;
    if (!msgfolder) return NS_ERROR_FAILURE;

    rv = msgfolder->CreateSubfolder(NS_ConvertUTF8toUTF16(aName).get(), nsnull);
    if (NS_FAILED(rv)) return rv;

    return NS_OK;
}

PRBool
writeGroupToHostInfoFile(nsCString &aElement, void *aData)
{
    nsIOFileStream *stream;
    stream = (nsIOFileStream *)aData;
    NS_ASSERTION(stream, "no stream");
    if (!stream) {
        // stop, something is bad.
        return PR_FALSE;
    }

    // XXX todo ",,1,0,0" is a temporary hack, fix it
    *stream << aElement.get() << ",,1,0,0" << MSG_LINEBREAK;
    return PR_TRUE;
}

nsresult
nsNntpIncomingServer::WriteHostInfoFile()
{
    nsresult rv = NS_OK;

    if (!mHostInfoHasChanged) {
        return NS_OK;
    }

    PRInt32 firstnewdate;

    LL_L2I(firstnewdate, mFirstNewDate);

    nsXPIDLCString hostname;
    rv = GetHostName(getter_Copies(hostname));
    NS_ENSURE_SUCCESS(rv,rv);
    
    nsFileSpec hostinfoFileSpec;

    if (!mHostInfoFile) 
        return NS_ERROR_UNEXPECTED;

    rv = mHostInfoFile->GetFileSpec(&hostinfoFileSpec);
    NS_ENSURE_SUCCESS(rv,rv);

    if (mHostInfoStream) {
        mHostInfoStream->close();
        delete mHostInfoStream;
    }

    mHostInfoStream = new nsIOFileStream(hostinfoFileSpec, (PR_RDWR | PR_CREATE_FILE | PR_TRUNCATE));
    if (!mHostInfoStream)
        return NS_ERROR_OUT_OF_MEMORY;

    // todo, missing some formatting, see the 4.x code
    *mHostInfoStream
         << "# News host information file." << MSG_LINEBREAK
         << "# This is a generated file!  Do not edit." << MSG_LINEBREAK
         << "" << MSG_LINEBREAK
         << "version=" << VALID_VERSION << MSG_LINEBREAK
         << "newsrcname=" << (const char*)hostname << MSG_LINEBREAK
         << "lastgroupdate=" << mLastGroupDate << MSG_LINEBREAK
         << "firstnewdate=" << firstnewdate << MSG_LINEBREAK
         << "uniqueid=" << mUniqueId << MSG_LINEBREAK
         << "" << MSG_LINEBREAK
         << "begingroups" << MSG_LINEBREAK;

    // XXX todo, sort groups first?

    mGroupsOnServer.EnumerateForwards((nsCStringArrayEnumFunc)writeGroupToHostInfoFile, (void *)mHostInfoStream);

    mHostInfoStream->close();
    delete mHostInfoStream;
    mHostInfoStream = nsnull;

    mHostInfoHasChanged = PR_FALSE;
    return NS_OK;
}

nsresult
nsNntpIncomingServer::LoadHostInfoFile()
{
	nsresult rv;
	
    // we haven't loaded it yet
    mHostInfoLoaded = PR_FALSE;

	rv = GetLocalPath(getter_AddRefs(mHostInfoFile));
	if (NS_FAILED(rv)) return rv;
	if (!mHostInfoFile) return NS_ERROR_FAILURE;

	rv = mHostInfoFile->AppendRelativeUnixPath(HOSTINFO_FILE_NAME);
	if (NS_FAILED(rv)) return rv;

	PRBool exists;
	rv = mHostInfoFile->Exists(&exists);
	if (NS_FAILED(rv)) return rv;

	// it is ok if the hostinfo.dat file does not exist.
	if (!exists) return NS_OK;

    char *buffer = nsnull;
    rv = mHostInfoFile->OpenStreamForReading();
    NS_ENSURE_SUCCESS(rv,rv);

    PRInt32 numread = 0;

    if (NS_FAILED(mHostInfoInputStream.GrowBuffer(HOSTINFO_FILE_BUFFER_SIZE))) {
    	return NS_ERROR_FAILURE;
    }
	
	mHasSeenBeginGroups = PR_FALSE;

    while (1) {
        buffer = mHostInfoInputStream.GetBuffer();
        rv = mHostInfoFile->Read(&buffer, HOSTINFO_FILE_BUFFER_SIZE, &numread);
        NS_ENSURE_SUCCESS(rv,rv);
        if (numread == 0) {
      		break;
    	}
    	else {
      		rv = BufferInput(mHostInfoInputStream.GetBuffer(), numread);
      		if (NS_FAILED(rv)) {
        		break;
      		}
    	}
  	}

    mHostInfoFile->CloseStream();
     
	rv = UpdateSubscribed();
	if (NS_FAILED(rv)) return rv;

	return NS_OK;
}

NS_IMETHODIMP
nsNntpIncomingServer::StartPopulatingWithUri(nsIMsgWindow *aMsgWindow, PRBool aForceToServer, const char *uri)
{
	nsresult rv = NS_OK;

#ifdef DEBUG_seth
	printf("StartPopulatingWithUri(%s)\n",uri);
#endif

    rv = EnsureInner();
    NS_ENSURE_SUCCESS(rv,rv);
    rv = mInner->StartPopulatingWithUri(aMsgWindow, aForceToServer, uri);
    NS_ENSURE_SUCCESS(rv,rv);

	rv = StopPopulating(mMsgWindow);
	if (NS_FAILED(rv)) return rv;

	return NS_OK;
}

NS_IMETHODIMP
nsNntpIncomingServer::SubscribeCleanup()
{
	nsresult rv = NS_OK;
    rv = ClearInner();
    NS_ENSURE_SUCCESS(rv,rv);
	return NS_OK;
}

NS_IMETHODIMP
nsNntpIncomingServer::StartPopulating(nsIMsgWindow *aMsgWindow, PRBool aForceToServer)
{
  nsresult rv;

  mMsgWindow = aMsgWindow;

  rv = EnsureInner();
  NS_ENSURE_SUCCESS(rv,rv);

  rv = mInner->StartPopulating(aMsgWindow, aForceToServer);
  NS_ENSURE_SUCCESS(rv,rv);

  rv = SetDelimiter(NEWS_DELIMITER);
  if (NS_FAILED(rv)) return rv;
    
  rv = SetShowFullName(PR_TRUE);
  if (NS_FAILED(rv)) return rv;

  nsCOMPtr<nsINntpService> nntpService = do_GetService(NS_NNTPSERVICE_CONTRACTID, &rv);
  NS_ENSURE_SUCCESS(rv,rv);

  mHostInfoLoaded = PR_FALSE;
  mVersion = INVALID_VERSION;
  mGroupsOnServer.Clear();

  if (!aForceToServer) {
	rv = LoadHostInfoFile();	
  	if (NS_FAILED(rv)) return rv;
  }

  // mHostInfoLoaded can be false if we failed to load anything
  if (!mHostInfoLoaded || (mVersion != VALID_VERSION)) {
    // set these to true, so when we are done and we call WriteHostInfoFile() 
    // we'll write out to hostinfo.dat
	mHostInfoHasChanged = PR_TRUE;
	mVersion = VALID_VERSION;

	mGroupsOnServer.Clear();

	rv = nntpService->GetListOfGroupsOnServer(this, aMsgWindow);
	if (NS_FAILED(rv)) return rv;
  }
  else {
	rv = StopPopulating(aMsgWindow);
	if (NS_FAILED(rv)) return rv;
  }

  return NS_OK;
}

/** 
 * This method is the entry point for |nsNNTPProtocol| class. |aName| is now 
 * encoded in the serverside character encoding, but we need to handle
 * newsgroup names in UTF-8 internally, So we convert |aName| to 
 * UTF-8 here for later use.
 **/
NS_IMETHODIMP
nsNntpIncomingServer::AddNewsgroupToList(const char *aName)
{
    nsresult rv;

    nsAutoString newsgroupName;
    nsCAutoString dataCharset;
    rv = GetCharset(dataCharset);
    NS_ENSURE_SUCCESS(rv,rv);

    rv = nsMsgI18NConvertToUnicode(dataCharset.get(), 
                                   nsDependentCString(aName),
                                   newsgroupName);
#ifdef DEBUG_jungshik
    NS_ASSERTION(NS_SUCCEEDED(rv), "newsgroup name conversion failed");
#endif
    if (NS_FAILED(rv)) {
        CopyASCIItoUTF16(aName, newsgroupName);
    }

    rv = AddTo(NS_ConvertUTF16toUTF8(newsgroupName),
               PR_FALSE, PR_TRUE, PR_TRUE);
    if (NS_FAILED(rv)) return rv;
    return NS_OK;
}

NS_IMETHODIMP
nsNntpIncomingServer::SetIncomingServer(nsIMsgIncomingServer *aServer)
{
    nsresult rv = EnsureInner();
    NS_ENSURE_SUCCESS(rv,rv);
	return mInner->SetIncomingServer(aServer);
}

NS_IMETHODIMP
nsNntpIncomingServer::SetShowFullName(PRBool showFullName)
{
    nsresult rv = EnsureInner();
    NS_ENSURE_SUCCESS(rv,rv);
	return mInner->SetShowFullName(showFullName);
}

nsresult
nsNntpIncomingServer::ClearInner()
{
    nsresult rv = NS_OK;

    if (mInner) {
        rv = mInner->SetSubscribeListener(nsnull);
        NS_ENSURE_SUCCESS(rv,rv);

        rv = mInner->SetIncomingServer(nsnull);
        NS_ENSURE_SUCCESS(rv,rv);

        mInner = nsnull;
    }
    return NS_OK;
}

nsresult
nsNntpIncomingServer::EnsureInner()
{
    nsresult rv = NS_OK;

    if (mInner) return NS_OK;

    mInner = do_CreateInstance(kSubscribableServerCID,&rv);
    NS_ENSURE_SUCCESS(rv,rv);
    if (!mInner) return NS_ERROR_FAILURE;

    rv = SetIncomingServer(this);
    NS_ENSURE_SUCCESS(rv,rv);

    return NS_OK;
}

NS_IMETHODIMP
nsNntpIncomingServer::GetDelimiter(char *aDelimiter)
{
    nsresult rv = EnsureInner();
    NS_ENSURE_SUCCESS(rv,rv);
    return mInner->GetDelimiter(aDelimiter);
}

NS_IMETHODIMP
nsNntpIncomingServer::SetDelimiter(char aDelimiter)
{
    nsresult rv = EnsureInner();
    NS_ENSURE_SUCCESS(rv,rv);
    return mInner->SetDelimiter(aDelimiter);
}

NS_IMETHODIMP
nsNntpIncomingServer::SetAsSubscribed(const nsACString &path)
{
    mTempSubscribed.AppendCString(path);

    nsresult rv = EnsureInner();
    NS_ENSURE_SUCCESS(rv,rv);
    return mInner->SetAsSubscribed(path);
}

PRBool
setAsSubscribedFunction(nsCString &aElement, void *aData)
{
    nsresult rv = NS_OK;
    nsNntpIncomingServer *server;
    server = (nsNntpIncomingServer *)aData;
    NS_ASSERTION(server, "no server");
    if (!server) {
        return PR_FALSE;
    }
 
    rv = server->SetAsSubscribed(aElement);
    NS_ASSERTION(NS_SUCCEEDED(rv),"SetAsSubscribed failed");
    return PR_TRUE;
}

NS_IMETHODIMP
nsNntpIncomingServer::UpdateSubscribed()
{
    nsresult rv = EnsureInner();
    NS_ENSURE_SUCCESS(rv,rv);
	mTempSubscribed.Clear();
	mSubscribedNewsgroups.EnumerateForwards((nsCStringArrayEnumFunc)setAsSubscribedFunction, (void *)this);
	return NS_OK;
}

NS_IMETHODIMP
nsNntpIncomingServer::AddTo(const nsACString &aName, PRBool addAsSubscribed,
                            PRBool aSubscribable, PRBool changeIfExists)
{
    NS_ASSERTION(IsUTF8(aName), "Non-UTF-8 newsgroup name");
    nsresult rv = EnsureInner();
    NS_ENSURE_SUCCESS(rv,rv);

    rv = AddGroupOnServer(aName);
    NS_ENSURE_SUCCESS(rv,rv);
 
    rv = mInner->AddTo(aName, addAsSubscribed, aSubscribable, changeIfExists);
    NS_ENSURE_SUCCESS(rv,rv);

    return rv;
}

NS_IMETHODIMP
nsNntpIncomingServer::StopPopulating(nsIMsgWindow *aMsgWindow)
{
	nsresult rv = NS_OK;

    nsCOMPtr<nsISubscribeListener> listener;
	rv = GetSubscribeListener(getter_AddRefs(listener));
    NS_ENSURE_SUCCESS(rv,rv);
	if (!listener) return NS_ERROR_FAILURE;

	rv = listener->OnDonePopulating();
    NS_ENSURE_SUCCESS(rv,rv);

    rv = EnsureInner();
    NS_ENSURE_SUCCESS(rv,rv);
	rv = mInner->StopPopulating(aMsgWindow);
    NS_ENSURE_SUCCESS(rv,rv);

    rv = WriteHostInfoFile();
    if (NS_FAILED(rv)) return rv;

	//xxx todo when do I set this to null?
	//rv = ClearInner();
    //NS_ENSURE_SUCCESS(rv,rv);
	return NS_OK;
}

NS_IMETHODIMP
nsNntpIncomingServer::SetSubscribeListener(nsISubscribeListener *aListener)
{	
    nsresult rv = EnsureInner();
    NS_ENSURE_SUCCESS(rv,rv);
	return mInner->SetSubscribeListener(aListener);
}

NS_IMETHODIMP
nsNntpIncomingServer::GetSubscribeListener(nsISubscribeListener **aListener)
{
    nsresult rv = EnsureInner();
    NS_ENSURE_SUCCESS(rv,rv);
    return mInner->GetSubscribeListener(aListener);
}

NS_IMETHODIMP
nsNntpIncomingServer::Subscribe(const PRUnichar *aUnicharName)
{
  return SubscribeToNewsgroup(NS_ConvertUTF16toUTF8(aUnicharName));
}

NS_IMETHODIMP
nsNntpIncomingServer::Unsubscribe(const PRUnichar *aUnicharName)
{
  nsresult rv;

  nsCOMPtr <nsIMsgFolder> serverFolder;
  rv = GetRootMsgFolder(getter_AddRefs(serverFolder));
  if (NS_FAILED(rv)) 
    return rv;

  if (!serverFolder) 
    return NS_ERROR_FAILURE;
 
  // to handle non-ASCII newsgroup names, we store them internally as escaped.
  // so we need to escape and encode the name, in order to find it.
  nsCAutoString escapedName;
  rv = NS_MsgEscapeEncodeURLPath(nsDependentString(aUnicharName), escapedName);

  nsCOMPtr <nsIMsgFolder> newsgroupFolder;
  rv = serverFolder->FindSubFolder(escapedName,
                                   getter_AddRefs(newsgroupFolder));

  if (NS_FAILED(rv)) 
    return rv;

  if (!newsgroupFolder) 
    return NS_ERROR_FAILURE;

  rv = serverFolder->PropagateDelete(newsgroupFolder, PR_TRUE /* delete storage */, nsnull);
  if (NS_FAILED(rv)) 
    return rv;

  // since we've unsubscribed to a newsgroup, the newsrc needs to be written out
  rv = SetNewsrcHasChanged(PR_TRUE);
  if (NS_FAILED(rv)) 
    return rv;

  return NS_OK;
}

PRInt32
nsNntpIncomingServer::HandleLine(char* line, PRUint32 line_size)
{
  NS_ASSERTION(line, "line is null");
  if (!line) return 0;

  // skip blank lines and comments
  if (line[0] == '#' || line[0] == '\0') return 0;
	
  line[line_size] = 0;

  if (mHasSeenBeginGroups) {
    char *commaPos = PL_strchr(line,',');
    if (commaPos) *commaPos = 0;

        // newsrc entries are all in UTF-8
#ifdef DEBUG_jungshik
    NS_ASSERTION(IsUTF8(nsDependentCString(line)), "newsrc line is not utf-8");
#endif
    nsresult rv = AddTo(nsDependentCString(line), PR_FALSE, PR_TRUE, PR_TRUE);
    NS_ASSERTION(NS_SUCCEEDED(rv),"failed to add line");
    if (NS_SUCCEEDED(rv)) {
      // since we've seen one group, we can claim we've loaded the
      // hostinfo file
      mHostInfoLoaded = PR_TRUE;
    }
  }
  else {
		if (nsCRT::strncmp(line,"begingroups", 11) == 0) {
			mHasSeenBeginGroups = PR_TRUE;
		}
		char*equalPos = PL_strchr(line, '=');	
		if (equalPos) {
			*equalPos++ = '\0';
			if (PL_strcmp(line, "lastgroupdate") == 0) {
				mLastGroupDate = strtol(equalPos, nsnull, 16);
			} else if (PL_strcmp(line, "firstnewdate") == 0) {
				PRInt32 firstnewdate = strtol(equalPos, nsnull, 16);
				LL_I2L(mFirstNewDate, firstnewdate);
			} else if (PL_strcmp(line, "uniqueid") == 0) {
				mUniqueId = strtol(equalPos, nsnull, 16);
			} else if (PL_strcmp(line, "version") == 0) {
				mVersion = strtol(equalPos, nsnull, 16);
			}
		}	
	}

	return 0;
}

nsresult
nsNntpIncomingServer::AddGroupOnServer(const nsACString &aName)
{
	mGroupsOnServer.AppendCString(aName); 
	return NS_OK;
}

NS_IMETHODIMP
nsNntpIncomingServer::AddNewsgroup(const nsAString &aName)
{
    // handle duplicates?
    mSubscribedNewsgroups.AppendCString(NS_ConvertUTF16toUTF8(aName));
    return NS_OK;
}

NS_IMETHODIMP
nsNntpIncomingServer::RemoveNewsgroup(const nsAString &aName)
{
    // handle duplicates?
    mSubscribedNewsgroups.RemoveCString(NS_ConvertUTF16toUTF8(aName));
    return NS_OK;
}

NS_IMETHODIMP
nsNntpIncomingServer::SetState(const nsACString &path, PRBool state,
                               PRBool *stateChanged)
{
    nsresult rv = EnsureInner();
    NS_ENSURE_SUCCESS(rv,rv);

    rv = mInner->SetState(path, state, stateChanged);
    if (*stateChanged) {
      if (state)
        mTempSubscribed.AppendCString(path);
      else
        mTempSubscribed.RemoveCString(path);
    }
    return rv;
}

NS_IMETHODIMP
nsNntpIncomingServer::HasChildren(const nsACString &path, PRBool *aHasChildren)
{
    nsresult rv = EnsureInner();
    NS_ENSURE_SUCCESS(rv,rv);
    return mInner->HasChildren(path, aHasChildren);
}

NS_IMETHODIMP
nsNntpIncomingServer::IsSubscribed(const nsACString &path,
                                   PRBool *aIsSubscribed)
{
    nsresult rv = EnsureInner();
    NS_ENSURE_SUCCESS(rv,rv);
    return mInner->IsSubscribed(path, aIsSubscribed);
}

NS_IMETHODIMP
nsNntpIncomingServer::IsSubscribable(const nsACString &path,
                                     PRBool *aIsSubscribable)
{
    nsresult rv = EnsureInner();
    NS_ENSURE_SUCCESS(rv,rv);
    return mInner->IsSubscribable(path, aIsSubscribable);
}

NS_IMETHODIMP
nsNntpIncomingServer::GetLeafName(const nsACString &path, nsAString &aLeafName)
{
    nsresult rv = EnsureInner();
    NS_ENSURE_SUCCESS(rv,rv);
    return mInner->GetLeafName(path, aLeafName);
}

NS_IMETHODIMP
nsNntpIncomingServer::GetFirstChildURI(const nsACString &path, nsACString &aResult)
{
    nsresult rv = EnsureInner();
    NS_ENSURE_SUCCESS(rv,rv);
    return mInner->GetFirstChildURI(path, aResult);
}

NS_IMETHODIMP
nsNntpIncomingServer::GetChildren(const nsACString &path,
                                  nsISupportsArray *array)
{
    nsresult rv = EnsureInner();
    NS_ENSURE_SUCCESS(rv,rv);
    return mInner->GetChildren(path, array);
}

NS_IMETHODIMP
nsNntpIncomingServer::CommitSubscribeChanges()
{
    nsresult rv;

    // we force the newrc to be dirty, so it will get written out when
    // we call WriteNewsrcFile()
    rv = SetNewsrcHasChanged(PR_TRUE);
    NS_ENSURE_SUCCESS(rv,rv);
    return WriteNewsrcFile();
}

NS_IMETHODIMP
nsNntpIncomingServer::ForgetPassword()
{
    nsresult rv;

    // clear password of root folder (for the news account)
    nsCOMPtr<nsIMsgFolder> rootFolder;
    rv = GetRootFolder(getter_AddRefs(rootFolder));
    NS_ENSURE_SUCCESS(rv,rv);
    if (!rootFolder) return NS_ERROR_FAILURE;

    nsCOMPtr <nsIMsgNewsFolder> newsFolder = do_QueryInterface(rootFolder, &rv);
    NS_ENSURE_SUCCESS(rv,rv);
    if (!newsFolder) return NS_ERROR_FAILURE;

    rv = newsFolder->ForgetGroupUsername();
    NS_ENSURE_SUCCESS(rv,rv);
    rv = newsFolder->ForgetGroupPassword();
    NS_ENSURE_SUCCESS(rv,rv);

    // clear password of all child folders
    nsCOMPtr<nsIEnumerator> subFolders;

    rv = rootFolder->GetSubFolders(getter_AddRefs(subFolders));
    NS_ENSURE_SUCCESS(rv,rv);

    nsAdapterEnumerator *simpleEnumerator = new nsAdapterEnumerator(subFolders);
    if (!simpleEnumerator) return NS_ERROR_OUT_OF_MEMORY;

    PRBool moreFolders = PR_FALSE;
        
    nsresult return_rv = NS_OK;

    while (NS_SUCCEEDED(simpleEnumerator->HasMoreElements(&moreFolders)) && moreFolders) {
        nsCOMPtr<nsISupports> child;
        rv = simpleEnumerator->GetNext(getter_AddRefs(child));
        if (NS_SUCCEEDED(rv) && child) {
            newsFolder = do_QueryInterface(child, &rv);
            if (NS_SUCCEEDED(rv) && newsFolder) {
                rv = newsFolder->ForgetGroupUsername();
                if (NS_FAILED(rv)) return_rv = rv;
                rv = newsFolder->ForgetGroupPassword();
                if (NS_FAILED(rv)) return_rv = rv;
            }
            else {
                return_rv = NS_ERROR_FAILURE;
            }
        }
    }
    delete simpleEnumerator;

    return return_rv;
}

NS_IMETHODIMP
nsNntpIncomingServer::GetSupportsExtensions(PRBool *aSupportsExtensions)
{
  NS_ASSERTION(0,"not implemented");
  return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP
nsNntpIncomingServer::SetSupportsExtensions(PRBool aSupportsExtensions)
{
  NS_ASSERTION(0,"not implemented");
  return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP
nsNntpIncomingServer::AddExtension(const char *extension)
{
  NS_ASSERTION(0,"not implemented");
  return NS_ERROR_NOT_IMPLEMENTED;
}
   
NS_IMETHODIMP
nsNntpIncomingServer::QueryExtension(const char *extension, PRBool *result)
{
#ifdef DEBUG_seth
  printf("no extension support yet\n");
#endif
  *result = PR_FALSE;
  return NS_OK;
}

NS_IMETHODIMP
nsNntpIncomingServer::GetPostingAllowed(PRBool *aPostingAllowed)
{
  *aPostingAllowed = mPostingAllowed;
  return NS_OK;
}

NS_IMETHODIMP
nsNntpIncomingServer::SetPostingAllowed(PRBool aPostingAllowed)
{
  mPostingAllowed = aPostingAllowed;
  return NS_OK;
}

NS_IMETHODIMP
nsNntpIncomingServer::GetLastUpdatedTime(PRUint32 *aLastUpdatedTime)
{
  *aLastUpdatedTime = mLastUpdatedTime;
  return NS_OK;
}

NS_IMETHODIMP
nsNntpIncomingServer::SetLastUpdatedTime(PRUint32 aLastUpdatedTime)
{
  NS_ASSERTION(0,"not implemented");
  return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP
nsNntpIncomingServer::AddPropertyForGet(const char *name, const char *value)
{
  NS_ASSERTION(0,"not implemented");
  return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP
nsNntpIncomingServer::QueryPropertyForGet(const char *name, char **value)
{
  NS_ASSERTION(0,"not implemented");
  return NS_ERROR_NOT_IMPLEMENTED;
}
  
NS_IMETHODIMP
nsNntpIncomingServer::AddSearchableGroup(const nsAString &name)
{
  NS_ASSERTION(0,"not implemented");
  return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP
nsNntpIncomingServer::QuerySearchableGroup(const nsAString &name, PRBool *result)
{
  NS_ASSERTION(0,"not implemented");
  return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP
nsNntpIncomingServer::AddSearchableHeader(const char *name)
{
  NS_ASSERTION(0,"not implemented");
  return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP
nsNntpIncomingServer::QuerySearchableHeader(const char *name, PRBool *result)
{
  NS_ASSERTION(0,"not implemented");
  return NS_ERROR_NOT_IMPLEMENTED;
}
  
NS_IMETHODIMP
nsNntpIncomingServer::FindGroup(const nsACString &name, nsIMsgNewsFolder **result)
{
  NS_ENSURE_ARG_POINTER(result);

  nsresult rv;
  nsCOMPtr <nsIMsgFolder> serverFolder;
  rv = GetRootMsgFolder(getter_AddRefs(serverFolder));
  NS_ENSURE_SUCCESS(rv,rv);

  if (!serverFolder) return NS_ERROR_FAILURE;

  nsCOMPtr <nsIMsgFolder> subFolder;
  rv = serverFolder->FindSubFolder(name, getter_AddRefs(subFolder));
  NS_ENSURE_SUCCESS(rv,rv);
  if (!subFolder) return NS_ERROR_FAILURE;

  rv = subFolder->QueryInterface(NS_GET_IID(nsIMsgNewsFolder), (void**)result);
  NS_ENSURE_SUCCESS(rv,rv);
  if (!*result) return NS_ERROR_FAILURE;
  return NS_OK;
}

NS_IMETHODIMP
nsNntpIncomingServer::GetFirstGroupNeedingExtraInfo(nsACString &result)
{
  NS_ASSERTION(0,"not implemented");
  return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP
nsNntpIncomingServer::SetGroupNeedsExtraInfo(const nsACString &name,
                                             PRBool needsExtraInfo)
{
  NS_ASSERTION(0,"not implemented");
  return NS_ERROR_NOT_IMPLEMENTED;
}


NS_IMETHODIMP
nsNntpIncomingServer::GroupNotFound(nsIMsgWindow *aMsgWindow,
                                    const nsAString &aName, PRBool aOpening)
{
  nsresult rv;
  nsCOMPtr <nsIPrompt> prompt;

  if (aMsgWindow) {
    rv = aMsgWindow->GetPromptDialog(getter_AddRefs(prompt));
    NS_ASSERTION(NS_SUCCEEDED(rv), "no prompt from the msg window");
  }

  if (!prompt) {
    nsCOMPtr<nsIWindowWatcher> wwatch(do_GetService(NS_WINDOWWATCHER_CONTRACTID));
    rv = wwatch->GetNewPrompter(nsnull, getter_AddRefs(prompt));
    NS_ENSURE_SUCCESS(rv,rv);
  }

  nsCOMPtr <nsIStringBundleService> bundleService = do_GetService(NS_STRINGBUNDLE_CONTRACTID,&rv);
  NS_ENSURE_SUCCESS(rv,rv);
  
  nsCOMPtr <nsIStringBundle> bundle;
  rv = bundleService->CreateBundle(NEWS_MSGS_URL, getter_AddRefs(bundle));
  NS_ENSURE_SUCCESS(rv,rv);

  nsXPIDLCString hostname;
  rv = GetHostName(getter_Copies(hostname));
  NS_ENSURE_SUCCESS(rv,rv);

  NS_ConvertUTF8toUTF16 hostStr(hostname); 

  nsAFlatString groupName = PromiseFlatString(aName);
  const PRUnichar *formatStrings[2] = { groupName.get(), hostStr.get() };
  nsXPIDLString confirmText;
  rv = bundle->FormatStringFromName(
                    NS_LITERAL_STRING("autoUnsubscribeText").get(),
                    formatStrings, 2,
                    getter_Copies(confirmText));
  NS_ENSURE_SUCCESS(rv,rv);

  PRBool confirmResult = PR_FALSE;
  rv = prompt->Confirm(nsnull, confirmText, &confirmResult);
  NS_ENSURE_SUCCESS(rv,rv);

  if (confirmResult) {
    rv = Unsubscribe(groupName.get());
    NS_ENSURE_SUCCESS(rv,rv);
  }
  
  return rv;
}

NS_IMETHODIMP
nsNntpIncomingServer::SetPrettyNameForGroup(const nsAString &name,
                                            const nsAString &prettyName)
{
  NS_ASSERTION(0,"not implemented");
  return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP
nsNntpIncomingServer::GetCanSearchMessages(PRBool *canSearchMessages)
{
    NS_ENSURE_ARG_POINTER(canSearchMessages);
    *canSearchMessages = PR_TRUE;
    return NS_OK;
}

NS_IMETHODIMP
nsNntpIncomingServer::GetOfflineSupportLevel(PRInt32 *aSupportLevel)
{
    NS_ENSURE_ARG_POINTER(aSupportLevel);
    nsresult rv;
    
    rv = GetIntValue("offline_support_level", aSupportLevel);
    if (*aSupportLevel != OFFLINE_SUPPORT_LEVEL_UNDEFINED) return rv;
    
    // set default value
    *aSupportLevel = OFFLINE_SUPPORT_LEVEL_EXTENDED;
    return NS_OK;
}

NS_IMETHODIMP
nsNntpIncomingServer::GetDefaultCopiesAndFoldersPrefsToServer(PRBool *aCopiesAndFoldersOnServer)
{
    NS_ENSURE_ARG_POINTER(aCopiesAndFoldersOnServer);

    /**
     * When a news account is created, the copies and folder prefs for the 
     * associated identity don't point to folders on the server. 
     * This makes sense, since there is no "Drafts" folder on a news server.
     * They'll point to the ones on "Local Folders"
     */

    *aCopiesAndFoldersOnServer = PR_FALSE;
    return NS_OK;
}

NS_IMETHODIMP
nsNntpIncomingServer::GetCanCreateFoldersOnServer(PRBool *aCanCreateFoldersOnServer)
{
    NS_ENSURE_ARG_POINTER(aCanCreateFoldersOnServer);

    // No folder creation on news servers. Return false.
    *aCanCreateFoldersOnServer = PR_FALSE;
    return NS_OK;
}

PRBool
buildSubscribeSearchResult(nsCString &aElement, void *aData)
{
    nsresult rv = NS_OK;
    nsNntpIncomingServer *server;
    server = (nsNntpIncomingServer *)aData;
    NS_ASSERTION(server, "no server");
    if (!server) {
        return PR_FALSE;
    }
 
    rv = server->AppendIfSearchMatch(aElement);
    NS_ASSERTION(NS_SUCCEEDED(rv),"AddSubscribeSearchResult failed");
    return PR_TRUE;
}

nsresult
nsNntpIncomingServer::AppendIfSearchMatch(nsCString& newsgroupName)
{
    NS_ConvertUTF8toUTF16 groupName(newsgroupName);
    nsAString::const_iterator start, end;
    groupName.BeginReading(start);
    groupName.EndReading(end);
    if (FindInReadable(mSearchValue, start, end, 
                       nsCaseInsensitiveStringComparator())) 
        mSubscribeSearchResult.AppendCString(newsgroupName);
    return NS_OK;
}

NS_IMETHODIMP
nsNntpIncomingServer::SetSearchValue(const nsAString &searchValue)
{
    mSearchValue = searchValue;

    if (mTree) {
        mTree->BeginUpdateBatch();
        mTree->RowCountChanged(0, -mSubscribeSearchResult.Count());
    }

    mSubscribeSearchResult.Clear();
    mGroupsOnServer.
        EnumerateForwards(nsCStringArrayEnumFunc(buildSubscribeSearchResult),
                          (void *)this);
    mSubscribeSearchResult.SortIgnoreCase();

    if (mTree) {
        mTree->RowCountChanged(0, mSubscribeSearchResult.Count());
        mTree->EndUpdateBatch();
    }
    return NS_OK;
}

NS_IMETHODIMP
nsNntpIncomingServer::GetSupportsSubscribeSearch(PRBool *retVal)
{
    *retVal = PR_TRUE;
    return NS_OK;
}

NS_IMETHODIMP 
nsNntpIncomingServer::GetRowCount(PRInt32 *aRowCount)
{
    *aRowCount = mSubscribeSearchResult.Count();
    return NS_OK;
}

NS_IMETHODIMP 
nsNntpIncomingServer::GetSelection(nsITreeSelection * *aSelection)
{
  *aSelection = mTreeSelection;
  NS_IF_ADDREF(*aSelection);
  return NS_OK;
}

NS_IMETHODIMP 
nsNntpIncomingServer::SetSelection(nsITreeSelection * aSelection)
{
  mTreeSelection = aSelection;
  return NS_OK;
}

NS_IMETHODIMP 
nsNntpIncomingServer::GetRowProperties(PRInt32 index, nsISupportsArray *properties)
{
    return NS_OK;
}

NS_IMETHODIMP 
nsNntpIncomingServer::GetCellProperties(PRInt32 row, nsITreeColumn* col, nsISupportsArray *properties)
{
    if (!IsValidRow(row))
      return NS_ERROR_UNEXPECTED;

    const PRUnichar* colID;
    col->GetIdConst(&colID);
    if (colID[0] == 's') { 
        // if <name> is in our temporary list of subscribed groups
        // add the "subscribed" property so the check mark shows up
        // in the "subscribedCol"
        nsCString name;
        if (mSearchResultSortDescending)
          row = mSubscribeSearchResult.Count() + ~row;
        mSubscribeSearchResult.CStringAt(row, name);
        if (mTempSubscribed.IndexOf(name) != -1) {
          properties->AppendElement(mSubscribedAtom); 
        }
    }
    else if (colID[0] == 'n') {
      // add the "nntp" property to the "nameCol" 
      // so we get the news folder icon in the search view
      properties->AppendElement(mNntpAtom); 
    }
    return NS_OK;
}

NS_IMETHODIMP 
nsNntpIncomingServer::GetColumnProperties(nsITreeColumn* col, nsISupportsArray *properties)
{
    return NS_OK;
}

NS_IMETHODIMP 
nsNntpIncomingServer::IsContainer(PRInt32 index, PRBool *_retval)
{
    *_retval = PR_FALSE;
    return NS_OK;
}

NS_IMETHODIMP 
nsNntpIncomingServer::IsContainerOpen(PRInt32 index, PRBool *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP 
nsNntpIncomingServer::IsContainerEmpty(PRInt32 index, PRBool *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP 
nsNntpIncomingServer::IsSeparator(PRInt32 index, PRBool *_retval)
{
    *_retval = PR_FALSE;
    return NS_OK;
}

NS_IMETHODIMP 
nsNntpIncomingServer::IsSorted(PRBool *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP 
nsNntpIncomingServer::CanDrop(PRInt32 index, PRInt32 orientation, PRBool *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP 
nsNntpIncomingServer::Drop(PRInt32 row, PRInt32 orientation)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP 
nsNntpIncomingServer::GetParentIndex(PRInt32 rowIndex, PRInt32 *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP 
nsNntpIncomingServer::HasNextSibling(PRInt32 rowIndex, PRInt32 afterIndex, PRBool *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP 
nsNntpIncomingServer::GetLevel(PRInt32 index, PRInt32 *_retval)
{
    *_retval = 0;
    return NS_OK;
}

nsresult 
nsNntpIncomingServer::IsValidRow(PRInt32 row)
{
    return ((row >= 0) && (row < mSubscribeSearchResult.Count()));
}

NS_IMETHODIMP 
nsNntpIncomingServer::GetImageSrc(PRInt32 row, nsITreeColumn* col, nsAString& _retval)
{
  return NS_OK;
}

NS_IMETHODIMP 
nsNntpIncomingServer::GetProgressMode(PRInt32 row, nsITreeColumn* col, PRInt32* _retval)
{
  return NS_OK;
}

NS_IMETHODIMP 
nsNntpIncomingServer::GetCellValue(PRInt32 row, nsITreeColumn* col, nsAString& _retval)
{
  return NS_OK;
}

NS_IMETHODIMP 
nsNntpIncomingServer::GetCellText(PRInt32 row, nsITreeColumn* col, nsAString& _retval)
{
    if (!IsValidRow(row))
      return NS_ERROR_UNEXPECTED;

    const PRUnichar* colID;
    col->GetIdConst(&colID);

    nsresult rv = NS_OK;
    if (colID[0] == 'n') {
      nsCAutoString str;
      if (mSearchResultSortDescending)
        row = mSubscribeSearchResult.Count() + ~row;
      mSubscribeSearchResult.CStringAt(row, str);
      // some servers have newsgroup names that are non ASCII.  we store 
      // those as escaped. unescape here so the UI is consistent
      rv = NS_MsgDecodeUnescapeURLPath(str, _retval);
    }
    return rv;
}

NS_IMETHODIMP 
nsNntpIncomingServer::SetTree(nsITreeBoxObject *tree)
{
  mTree = tree;
  if (!tree)
      return NS_OK;

  nsCOMPtr<nsITreeColumns> cols;
  tree->GetColumns(getter_AddRefs(cols));
  if (!cols)
      return NS_OK;

  nsCOMPtr<nsITreeColumn> col;
  cols->GetKeyColumn(getter_AddRefs(col));
  if (!col)
      return NS_OK;

  nsCOMPtr<nsIDOMElement> element;
  col->GetElement(getter_AddRefs(element));
  if (!element)
      return NS_OK;

  nsAutoString dir;
  element->GetAttribute(NS_LITERAL_STRING("sortDirection"), dir);
  mSearchResultSortDescending = dir.EqualsLiteral("descending");
  return NS_OK;
}

NS_IMETHODIMP 
nsNntpIncomingServer::ToggleOpenState(PRInt32 index)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP 
nsNntpIncomingServer::CycleHeader(nsITreeColumn* col)
{
    PRBool cycler;
    col->GetCycler(&cycler);
    if (!cycler) {
        NS_NAMED_LITERAL_STRING(dir, "sortDirection");
        nsCOMPtr<nsIDOMElement> element;
        col->GetElement(getter_AddRefs(element));
        mSearchResultSortDescending = !mSearchResultSortDescending;
        element->SetAttribute(dir, mSearchResultSortDescending ?
            NS_LITERAL_STRING("descending") : NS_LITERAL_STRING("ascending"));
        mTree->Invalidate();
    }
    return NS_OK;
}

NS_IMETHODIMP 
nsNntpIncomingServer::SelectionChanged()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP 
nsNntpIncomingServer::CycleCell(PRInt32 row, nsITreeColumn* col)
{
    return NS_OK;
}

NS_IMETHODIMP 
nsNntpIncomingServer::IsEditable(PRInt32 row, nsITreeColumn* col, PRBool *_retval)
{
    *_retval = PR_FALSE;
    return NS_OK;
}

NS_IMETHODIMP 
nsNntpIncomingServer::IsSelectable(PRInt32 row, nsITreeColumn* col, PRBool *_retval)
{
    *_retval = PR_FALSE;
    return NS_OK;
}

NS_IMETHODIMP 
nsNntpIncomingServer::SetCellValue(PRInt32 row, nsITreeColumn* col, const nsAString& value)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP 
nsNntpIncomingServer::SetCellText(PRInt32 row, nsITreeColumn* col, const nsAString& value)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP 
nsNntpIncomingServer::PerformAction(const PRUnichar *action)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP 
nsNntpIncomingServer::PerformActionOnRow(const PRUnichar *action, PRInt32 row)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP 
nsNntpIncomingServer::PerformActionOnCell(const PRUnichar *action, PRInt32 row, nsITreeColumn* col)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP
nsNntpIncomingServer::GetCanFileMessagesOnServer(PRBool *aCanFileMessagesOnServer)
{
    NS_ENSURE_ARG_POINTER(aCanFileMessagesOnServer);

    // No folder creation on news servers. Return false.
    *aCanFileMessagesOnServer = PR_FALSE;
    return NS_OK;
}

NS_IMETHODIMP
nsNntpIncomingServer::GetFilterScope(nsMsgSearchScopeValue *filterScope)
{
   NS_ENSURE_ARG_POINTER(filterScope);

   *filterScope = nsMsgSearchScope::newsFilter;
   return NS_OK;
}

NS_IMETHODIMP
nsNntpIncomingServer::GetSearchScope(nsMsgSearchScopeValue *searchScope)
{
   NS_ENSURE_ARG_POINTER(searchScope);

   if (WeAreOffline()) {
     *searchScope = nsMsgSearchScope::localNews;
   }
   else {
     *searchScope = nsMsgSearchScope::news;
   }
   return NS_OK;
}

NS_IMETHODIMP
nsNntpIncomingServer::OnUserOrHostNameChanged(const char *oldName, const char *newName)
{
  nsresult rv;
  // 1. Do common things in the base class.
  rv = nsMsgIncomingServer::OnUserOrHostNameChanged(oldName, newName);
  NS_ENSURE_SUCCESS(rv,rv);

  // 2. Remove file hostinfo.dat so that the new subscribe 
  //    list will be reloaded from the new server.
  nsCOMPtr <nsIFileSpec> hostInfoFile;
  rv = GetLocalPath(getter_AddRefs(hostInfoFile));
  NS_ENSURE_SUCCESS(rv, rv);
  rv = hostInfoFile->AppendRelativeUnixPath(HOSTINFO_FILE_NAME);
  NS_ENSURE_SUCCESS(rv, rv);
  hostInfoFile->Delete(PR_FALSE);

  // 3.Unsubscribe and then subscribe the existing groups to clean up the article numbers
  //   in the rc file (this is because the old and new servers may maintain different 
  //   numbers for the same articles if both servers handle the same groups).
  nsCOMPtr<nsIEnumerator> subFolders;

  nsCOMPtr <nsIMsgFolder> serverFolder;
  rv = GetRootMsgFolder(getter_AddRefs(serverFolder));
  NS_ENSURE_SUCCESS(rv,rv);

  rv = serverFolder->GetSubFolders(getter_AddRefs(subFolders));
  NS_ENSURE_SUCCESS(rv,rv);

  nsStringArray groupList;
  nsXPIDLString folderName;
  nsCOMPtr<nsISupports> aItem;
  nsCOMPtr <nsIMsgFolder> newsgroupFolder;

  // Prepare the group list
  while (subFolders->IsDone() != NS_OK)
  {
    rv = subFolders->CurrentItem(getter_AddRefs(aItem));
    NS_ENSURE_SUCCESS(rv,rv);
    newsgroupFolder = do_QueryInterface(aItem, &rv);
    NS_ENSURE_SUCCESS(rv,rv);
    rv = newsgroupFolder->GetName(getter_Copies(folderName));
    NS_ENSURE_SUCCESS(rv,rv);
    groupList.AppendString(folderName);
    if (! NS_SUCCEEDED(subFolders->Next()))
      break;
  }

  // If nothing subscribed then we're done.
  if (groupList.Count() == 0)
    return NS_OK;

  // Now unsubscribe & subscribe.
  int i, cnt=groupList.Count();
  nsAutoString groupStr;
  nsCAutoString cname;
  for (i=0; i<cnt; i++)
  {
    // unsubscribe.
    groupList.StringAt(i, groupStr);
    rv = Unsubscribe(groupStr.get());
    NS_ENSURE_SUCCESS(rv,rv);
  }

  for (i=0; i<cnt; i++)
  {
    // subscribe.
    groupList.StringAt(i, groupStr);
    rv = SubscribeToNewsgroup(NS_ConvertUTF16toUTF8(groupStr));
    NS_ENSURE_SUCCESS(rv,rv);
  }

  groupList.Clear();
  
  // Force updating the rc file.
  rv = CommitSubscribeChanges();
  return rv;
}
