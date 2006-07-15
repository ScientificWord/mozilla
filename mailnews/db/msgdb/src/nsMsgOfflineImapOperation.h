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
 * Portions created by the Initial Developer are Copyright (C) 2001
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
#ifndef _nsMsgOfflineImapOperation_H_

#include "nsIMsgOfflineImapOperation.h"
#include "mdb.h"
#include "nsMsgDatabase.h"
#include "nsXPIDLString.h"
#include "prlog.h"

class nsMsgOfflineImapOperation : public nsIMsgOfflineImapOperation
{
public:
  /** Instance Methods **/
  nsMsgOfflineImapOperation(nsMsgDatabase *db, nsIMdbRow *row);
  virtual   ~nsMsgOfflineImapOperation();
  NS_DECL_ISUPPORTS
  NS_DECL_NSIMSGOFFLINEIMAPOPERATION

	
  nsIMdbRow   *GetMDBRow() {return m_mdbRow;}
  nsresult    GetCopiesFromDB();
  nsresult    SetCopiesToDB();
  void        Log(PRLogModuleInfo *logFile);
protected:
  nsresult AddKeyword(const char *aKeyword, nsCString &addList, const char *addProp,
                      nsCString &removeList, const char *removeProp);

  nsOfflineImapOperationType m_operation;
  nsMsgKey          m_messageKey; 	
  nsMsgKey          m_sourceMessageKey; 	
  PRUint32          m_operationFlags;	// what to do on sync
  imapMessageFlagsType	m_newFlags;			// used for kFlagsChanged
	
  // these are URI's, and are escaped. Thus, we can use a delimter like ' '
  // because the real spaces should be escaped.
  nsXPIDLCString      m_sourceFolder;
  nsXPIDLCString      m_moveDestination;	
  nsCStringArray      m_copyDestinations;

  nsXPIDLCString      m_keywordsToAdd;
  nsXPIDLCString      m_keywordsToRemove;

  // nsMsgOfflineImapOperation will have to know what db and row they belong to, since they are really
  // just a wrapper around the offline operation row in the mdb.
  // though I hope not.
  nsMsgDatabase    *m_mdb;
  nsCOMPtr <nsIMdbRow> m_mdbRow;
};
	


#endif /* _nsMsgOfflineImapOperation_H_ */

