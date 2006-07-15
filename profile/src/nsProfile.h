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

#include "nsIProfile.h"
#include "nsIProfileInternal.h"
#include "nsIProfileStartupListener.h"
#include "nsIProfileChangeStatus.h"
#include "nsCOMPtr.h"
#include "nsISupports.h"
#include "nsIRegistry.h"
#include "nsString.h"
#include "nsICmdLineService.h"
#include "nsProfileAccess.h"

// Interfaces Needed
#include "nsIXULWindow.h"
#include "nsIInterfaceRequestor.h"
#include "nsIInterfaceRequestorUtils.h"
#include "nsIURIContentListener.h"
#include "nsIDirectoryService.h"

#define _MAX_LENGTH   256

class nsProfile: public nsIProfileInternal,
                 public nsIProfileChangeStatus
{
    NS_DECL_ISUPPORTS
    NS_DECL_NSIPROFILE
    NS_DECL_NSIPROFILEINTERNAL
    NS_DECL_NSIPROFILECHANGESTATUS

private:
    nsresult ProcessArgs(nsICmdLineService *service,
                         PRBool canInteract,
                         PRBool *profileDirSet,
                         nsCString & profileURLStr);
    nsresult LoadDefaultProfileDir(nsCString & profileURLStr, PRBool canInterract);
	nsresult ConfirmAutoMigration(PRBool canInteract, PRBool *confirmed);
	nsresult CopyDefaultFile(nsIFile *profDefaultsDir,
	                         nsIFile *newProfDir,
                             const nsACString &fileName);
	nsresult LoadNewProfilePrefs();
    nsresult SetProfileDir(const PRUnichar *profileName, nsIFile *profileDir);
								
    nsresult AddLevelOfIndirection(nsIFile *aDir);
    nsresult IsProfileDirSalted(nsIFile *profileDir, PRBool *isSalted);
    nsresult DefineLocaleDefaultsDir();
    nsresult Update4xProfileInfo();
    nsresult GetOldRegLocation(nsIFile **aOldRegFile);
    nsresult UpdateCurrentProfileModTime(PRBool updateRegistry);
    nsresult MigrateProfileInternal(const PRUnichar *profileName,
                                    nsIFile *oldProfDir, nsIFile *newProfDir);

    nsresult GetLocalProfileDir(const PRUnichar *profileName, nsIFile** localDir);

    PRBool mStartingUp;
    PRBool mAutomigrate;
    PRBool mOutofDiskSpace;
    PRBool mDiskSpaceErrorQuitCalled;
    PRBool mProfileChangeVetoed;
    PRBool mProfileChangeFailed;

    nsString mCurrentProfileName;
    PRBool mCurrentProfileAvailable;

    PRBool mIsUILocaleSpecified;
    nsCString mUILocaleName;

    PRBool mIsContentLocaleSpecified;
    nsCString mContentLocaleName;
    
    PRBool mShutdownProfileToreDownNetwork;
    
public:
    nsProfile();
    virtual ~nsProfile();
    
    nsresult Init();

    // Copies all the registry keys from old profile to new profile
    nsresult CopyRegKey(const PRUnichar *oldProfile, const PRUnichar *newProfile);

    nsresult AutoMigrate();

    nsresult ShowProfileWizard(void);
};


