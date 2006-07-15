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
 * The Original Code is Mozilla Communicator client code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Alec Flett <alecf@netscape.com>
 *   Brian Nesse <bnesse@netscape.com>
 *   Benjamin Smedberg <benjamin@smedbergs.us>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
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

#include "nsCOMPtr.h"
#include "nsIObserver.h"
#include "nsIPrefBranch.h"
#include "nsIPrefBranchInternal.h"
#include "nsIPrefLocalizedString.h"
#include "nsISecurityPref.h"
#include "nsISupportsArray.h"
#include "nsXPCOM.h"
#include "nsISupportsPrimitives.h"
#include "nsIRelativeFilePref.h"
#include "nsILocalFile.h"
#include "nsString.h"
#include "nsVoidArray.h"
#include "nsWeakReference.h"

class nsPrefBranch : public nsIPrefBranchInternal,
                     public nsISecurityPref,
                     public nsIObserver,
                     public nsSupportsWeakReference
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIPREFBRANCH
  NS_DECL_NSIPREFBRANCH2
  NS_DECL_NSISECURITYPREF
  NS_DECL_NSIOBSERVER

  nsPrefBranch(const char *aPrefRoot, PRBool aDefaultBranch);
  virtual ~nsPrefBranch();

  PRInt32 GetRootLength() { return mPrefRootLength; }

protected:
  nsPrefBranch()	/* disallow use of this constructer */
    { };

  nsresult   GetDefaultFromPropertiesFile(const char *aPrefName, PRUnichar **return_buf);
  const char *getPrefName(const char *aPrefName);
  nsresult   getValidatedPrefName(const char *aPrefName, const char **_retval);
  void       freeObserverList(void);

private:
  PRInt32         mPrefRootLength;
  nsAutoVoidArray *mObservers;
  nsCString       mPrefRoot;
  nsCStringArray  mObserverDomains;
  PRBool          mIsDefault;

};


class nsPrefLocalizedString : public nsIPrefLocalizedString,
                              public nsISupportsString
{
public:
  nsPrefLocalizedString();
  virtual ~nsPrefLocalizedString();

  NS_DECL_ISUPPORTS
  NS_FORWARD_NSISUPPORTSSTRING(mUnicodeString->)
  NS_FORWARD_NSISUPPORTSPRIMITIVE(mUnicodeString->)

  nsresult Init();

private:
  NS_IMETHOD GetData(PRUnichar**);
  NS_IMETHOD SetData(const PRUnichar* aData);
  NS_IMETHOD SetDataWithLength(PRUint32 aLength, const PRUnichar *aData);
                               
  nsCOMPtr<nsISupportsString> mUnicodeString;
};


class nsRelativeFilePref : public nsIRelativeFilePref
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIRELATIVEFILEPREF
  
                nsRelativeFilePref();
  virtual       ~nsRelativeFilePref();
  
private:
  nsCOMPtr<nsILocalFile> mFile;
  nsCAutoString mRelativeToKey; // An nsCAutoString because length is always very short.
                                // While this makes the object larger, avoids allocation.
};
