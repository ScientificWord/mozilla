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

/* This class encapsulates the global information about a folder stored in the
	summary file.
*/
#ifndef _nsDBFolderInfo_H
#define _nsDBFolderInfo_H

#include "nsString.h"
#include "MailNewsTypes.h"
#include "mdb.h"
#include "nsMsgKeyArray.h"
#include "nsIDBFolderInfo.h"
#include <time.h>

class nsMsgDatabase;

// again, this could inherit from nsISupports, but I don't see the need as of yet.
// I'm not sure it needs to be ref-counted (but I think it does).

// I think these getters and setters really need to go through mdb and not rely on the object
// caching the values. If this somehow turns out to be prohibitively expensive, we can invent
// some sort of dirty mechanism, but I think it turns out that these values will be cached by 
// the MSG_FolderInfo's anyway.
class nsDBFolderInfo : public nsIDBFolderInfo
{
public:
  friend class nsMsgDatabase;
  
  nsDBFolderInfo(nsMsgDatabase *mdb);
  virtual ~nsDBFolderInfo();
  
  NS_DECL_ISUPPORTS
    // interface methods.
    NS_DECL_NSIDBFOLDERINFO
    // create the appropriate table and row in a new db.
    nsresult			AddToNewMDB();
  // accessor methods.
  
  PRBool    TestFlag(PRInt32 flags);
  PRInt16   GetIMAPHierarchySeparator() ;
  void      SetIMAPHierarchySeparator(PRInt16 hierarchySeparator) ;
  void      ChangeImapTotalPendingMessages(PRInt32 delta);
  void      ChangeImapUnreadPendingMessages(PRInt32 delta) ;
  
  nsresult      InitFromExistingDB();
  // get and set arbitrary property, aka row cell value.
  nsresult	SetPropertyWithToken(mdb_token aProperty, const nsAString &propertyStr);
  nsresult	SetUint32PropertyWithToken(mdb_token aProperty, PRUint32 propertyValue);
  nsresult	SetInt32PropertyWithToken(mdb_token aProperty, PRInt32 propertyValue);
  nsresult	GetPropertyWithToken(mdb_token aProperty, nsAString &resultProperty);
  nsresult	GetUint32PropertyWithToken(mdb_token aProperty, PRUint32 &propertyValue, PRUint32 defaultValue = 0);
  nsresult	GetInt32PropertyWithToken(mdb_token aProperty, PRInt32 &propertyValue, PRInt32 defaultValue = 0);
  nsresult      GetConstCharPtrCharacterSet(const char**result);
  
  
  nsMsgKeyArray m_lateredKeys;		// list of latered messages
  
protected:
  
  // initialize from appropriate table and row in existing db.
  nsresult InitMDBInfo();
  nsresult LoadMemberVariables();
  
  void ReleaseExternalReferences(); // let go of any references to other objects.
  
  PRInt32   m_folderSize;
  PRInt32   m_expungedBytes;	// sum of size of deleted messages in folder
  PRUint32  m_folderDate;
  nsMsgKey  m_highWaterMessageKey;	// largest news article number or imap uid whose header we've seen
  
  //  m_numUnreadMessages and m_numMessages can never be negative. 0 means 'no msgs'.
  PRInt32   m_numUnreadMessages;
  PRInt32   m_numMessages;    // includes expunged and ignored messages
  
  PRInt32   m_flags;  // folder specific flags. This holds things like re-use thread pane,
  // configured for off-line use, use default retrieval, purge article/header options
  
  PRUint16    m_version;                // for upgrading...
  PRInt16     m_IMAPHierarchySeparator;	// imap path separator
  
  // mail only (for now)
  
  // IMAP only
  PRInt32     m_ImapUidValidity;
  PRInt32     m_totalPendingMessages;
  PRInt32     m_unreadPendingMessages;
  
  // news only (for now)
  nsMsgKey    m_expiredMark;		// Highest invalid article number in group - for expiring
  // the db folder info will have to know what db and row it belongs to, since it is really
  // just a wrapper around the singleton folder info row in the mdb. 
  nsMsgDatabase *m_mdb;
  nsIMdbTable   *m_mdbTable;	// singleton table in db
  nsIMdbRow     *m_mdbRow;	// singleton row in table;
  
  nsCString     m_charSet;
  PRBool        m_charSetOverride;
  PRBool        m_mdbTokensInitialized;
  
  mdb_token     m_rowScopeToken;
  mdb_token     m_tableKindToken;
  // tokens for the pre-set columns - we cache these for speed, which may be silly
  mdb_token     m_mailboxNameColumnToken;
  mdb_token     m_numMessagesColumnToken;
  mdb_token     m_numUnreadMessagesColumnToken;
  mdb_token     m_flagsColumnToken;
  mdb_token     m_folderSizeColumnToken;
  mdb_token     m_expungedBytesColumnToken;
  mdb_token     m_folderDateColumnToken;
  mdb_token     m_highWaterMessageKeyColumnToken;
  
  mdb_token     m_imapUidValidityColumnToken;
  mdb_token     m_totalPendingMessagesColumnToken;
  mdb_token     m_unreadPendingMessagesColumnToken;
  mdb_token     m_expiredMarkColumnToken;
  mdb_token     m_versionColumnToken;
};

#endif
