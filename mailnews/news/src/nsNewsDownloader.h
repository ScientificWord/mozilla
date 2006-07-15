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

#ifndef _nsNewsDownloader_H_
#define _nsNewsDownloader_H_


#include "nsIMsgDatabase.h"
#include "nsIUrlListener.h"
#include "nsIMsgFolder.h"
#include "nsIMsgHdr.h"
#include "nsIMsgWindow.h"
#include "nsIMsgSearchNotify.h"
#include "nsIMsgSearchSession.h"

// base class for downloading articles in a single newsgroup. Keys to download are passed in
// to DownloadArticles method.
class nsNewsDownloader : public nsIUrlListener, public nsIMsgSearchNotify
{
public:
  nsNewsDownloader(nsIMsgWindow *window, nsIMsgDatabase *db, nsIUrlListener *listener);
  virtual ~nsNewsDownloader();

  NS_DECL_ISUPPORTS
  NS_DECL_NSIURLLISTENER
  NS_DECL_NSIMSGSEARCHNOTIFY

  virtual nsresult DownloadArticles(nsIMsgWindow *window, nsIMsgFolder *folder, nsMsgKeyArray *pKeyArray);
  
  PRBool ShouldAbort() const { return m_abort; }
	
protected:
  virtual PRInt32 Write(const char * /*block*/, PRInt32 length) {return length;}
  virtual void Abort();
  virtual void Complete();
  virtual PRBool GetNextHdrToRetrieve();
  virtual nsresult DownloadNext(PRBool firstTimeP);
  virtual PRInt32 FinishDownload() {return 0;}
  virtual PRInt32	StartDownload() {return 0;}
  virtual nsresult ShowProgress(const PRUnichar *progressString, PRInt32 percent);

  nsMsgKeyArray			m_keysToDownload;
  nsCOMPtr <nsIMsgFolder>	m_folder;
  nsCOMPtr <nsIMsgDatabase> m_newsDB;
  nsCOMPtr <nsIUrlListener> m_listener;
  PRPackedBool m_downloadFromKeys;
  PRPackedBool m_existedP;
  PRPackedBool m_wroteAnyP;
  PRPackedBool m_summaryValidP;
  PRPackedBool m_abort;
  PRInt32     m_numwrote;
  nsMsgKey    m_keyToDownload;
  nsCOMPtr <nsIMsgWindow> m_window;
  nsCOMPtr <nsIMsgStatusFeedback> m_statusFeedback;
  nsCOMPtr <nsIMsgSearchSession> m_searchSession;
  PRInt32 m_lastPercent;
  PRInt64 m_lastProgressTime;				
  nsresult  m_status;
};


// class for downloading articles in a single newsgroup to the offline store.
class DownloadNewsArticlesToOfflineStore : public nsNewsDownloader
{
public:
  DownloadNewsArticlesToOfflineStore(nsIMsgWindow *window, nsIMsgDatabase *db, nsIUrlListener *listener);
  virtual ~DownloadNewsArticlesToOfflineStore();

  NS_IMETHOD OnStartRunningUrl(nsIURI* url);
  NS_IMETHOD OnStopRunningUrl(nsIURI* url, nsresult exitCode);
protected:
  virtual PRInt32	StartDownload();
  virtual PRInt32 FinishDownload();
  virtual PRBool GetNextHdrToRetrieve();

  nsCOMPtr <nsISimpleEnumerator>	m_headerEnumerator;
  nsCOMPtr <nsIMsgDBHdr>	m_newsHeader;
};

// class for downloading all the articles that match the passed in search criteria
// for a single newsgroup.
class DownloadMatchingNewsArticlesToNewsDB : public DownloadNewsArticlesToOfflineStore
{
public:
  DownloadMatchingNewsArticlesToNewsDB(nsIMsgWindow *window, nsIMsgFolder *folder, nsIMsgDatabase *newsDB,  nsIUrlListener *listener);
  virtual ~DownloadMatchingNewsArticlesToNewsDB();
  nsresult RunSearch(nsIMsgFolder *folder, nsIMsgDatabase *newsDB, nsIMsgSearchSession *searchSession);
protected:
};

// this class iterates all the news servers for each group on the server that's configured for
// offline use, downloads the messages that meet the download criteria for that newsgroup/server
class nsMsgDownloadAllNewsgroups : public nsIUrlListener
{
public:
  nsMsgDownloadAllNewsgroups(nsIMsgWindow *window, nsIUrlListener *listener);
  virtual ~nsMsgDownloadAllNewsgroups();

  NS_DECL_ISUPPORTS
  NS_DECL_NSIURLLISTENER

  nsresult ProcessNextGroup();

protected:
  nsresult AdvanceToNextServer(PRBool *done);
  nsresult AdvanceToNextGroup(PRBool *done);
  nsresult DownloadMsgsForCurrentGroup();

  DownloadMatchingNewsArticlesToNewsDB *m_downloaderForGroup;

  nsCOMPtr <nsIMsgFolder> m_currentFolder;
  nsCOMPtr <nsIMsgWindow> m_window;
  nsCOMPtr <nsISupportsArray> m_allServers;
  nsCOMPtr <nsISupportsArray> m_allFolders;
  nsCOMPtr <nsIMsgIncomingServer> m_currentServer;
  nsCOMPtr <nsIEnumerator> m_serverEnumerator;
  nsCOMPtr <nsIUrlListener> m_listener;
  nsCOMPtr <nsISupportsArray> m_termList;

  PRBool m_downloadedHdrsForCurGroup;
};

#endif
