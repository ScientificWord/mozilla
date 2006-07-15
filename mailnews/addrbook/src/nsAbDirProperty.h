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
 *   Seth Spitzer <sspitzer@netscape.com>
 *   Mark Banner <mark@standard8.demon.co.uk>
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

/********************************************************************************************************
 
   Interface for representing Address Book Directory
 
*********************************************************************************************************/

#ifndef nsAbDirProperty_h__
#define nsAbDirProperty_h__

#include "nsIAbDirectory.h" /* include the interface we are going to support */
#include "nsIAbCard.h"
#include "nsISupportsArray.h"
#include "nsCOMPtr.h"
#include "nsDirPrefs.h"
#include "nsIAddrDatabase.h"
#include "nsString.h"
#include "nsIPrefBranch.h"

 /* 
  * Address Book Directory
  */ 

class nsAbDirProperty: public nsIAbDirectory
{
public: 
	nsAbDirProperty(void);
	virtual ~nsAbDirProperty(void);

	NS_DECL_ISUPPORTS
	NS_DECL_NSIABDIRECTORY

protected:
  /**
   * Initialise the directory prefs for this branch
   */
  nsresult InitDirectoryPrefs();

	nsresult GetAttributeName(PRUnichar **aName, nsString& value);
	nsresult SetAttributeName(const PRUnichar *aName, nsString& arrtibute);

	nsString m_DirName;
	PRUint32 m_LastModifiedDate;

	nsString m_ListName;
	nsString m_ListNickName;
	nsString m_Description;
	PRBool   m_IsMailList;
  nsCString m_DirPrefId;  // ie,"ldap_2.servers.pab"
  nsCOMPtr<nsIPrefBranch> m_DirectoryPrefs;
  nsCOMPtr<nsISupportsArray> m_AddressList;
};

class nsAbDirectoryProperties: public nsIAbDirectoryProperties
{
public: 
  nsAbDirectoryProperties(void);
  virtual ~nsAbDirectoryProperties(void);

  NS_DECL_ISUPPORTS
  NS_DECL_NSIABDIRECTORYPROPERTIES

private:
  nsString  mDescription;
  nsCString mURI;
  nsCString mFileName;
  nsCString mPrefName;
  PRUint32  mDirType;
  PRUint32  mMaxHits;
  nsCString mAuthDn;
  PRUint32  mSyncTimeStamp;
  PRInt32   mCategoryId;
  PRInt32   mPosition;
};
#endif
