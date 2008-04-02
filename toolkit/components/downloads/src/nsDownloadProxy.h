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
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Blake Ross <blaker@netscape.com> (Original Author)
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
 
#ifndef downloadproxy___h___
#define downloadproxy___h___

#include "nsIDownloadManager.h"
#include "nsIPrefBranch.h"
#include "nsIPrefService.h"
#include "nsIMIMEInfo.h"
#include "nsIFileURL.h"
#include "nsIDownloadManagerUI.h"

#define PREF_BDM_SHOWWHENSTARTING "browser.download.manager.showWhenStarting"
#define PREF_BDM_USEWINDOW "browser.download.manager.useWindow"
#define PREF_BDM_FOCUSWHENSTARTING "browser.download.manager.focusWhenStarting"

class nsDownloadProxy : public nsITransfer
{
public:

  nsDownloadProxy() { }
  virtual ~nsDownloadProxy() { }

  NS_DECL_ISUPPORTS

  NS_IMETHODIMP Init(nsIURI* aSource,
                     nsIURI* aTarget,
                     const nsAString& aDisplayName,
                     nsIMIMEInfo *aMIMEInfo,
                     PRTime aStartTime,
                     nsILocalFile* aTempFile,
                     nsICancelable* aCancelable) {
    nsresult rv;
    nsCOMPtr<nsIDownloadManager> dm = do_GetService("@mozilla.org/download-manager;1", &rv);
    NS_ENSURE_SUCCESS(rv, rv);
    
    rv = dm->AddDownload(nsIDownloadManager::DOWNLOAD_TYPE_DOWNLOAD, aSource,
                         aTarget, aDisplayName, aMIMEInfo, aStartTime,
                         aTempFile, aCancelable, getter_AddRefs(mInner));
    NS_ENSURE_SUCCESS(rv, rv);

    nsCOMPtr<nsIPrefService> prefs = do_GetService("@mozilla.org/preferences-service;1", &rv);
    NS_ENSURE_SUCCESS(rv, rv);
    nsCOMPtr<nsIPrefBranch> branch = do_QueryInterface(prefs);

    PRBool showDM = PR_TRUE;
    if (branch)
      branch->GetBoolPref(PREF_BDM_SHOWWHENSTARTING , &showDM);

    PRBool useWindow = PR_TRUE;
    if (branch)
      branch->GetBoolPref(PREF_BDM_USEWINDOW, &useWindow);
    
    if (showDM && useWindow) {
      PRUint32 id;
      mInner->GetId(&id);

      nsCOMPtr<nsIDownloadManagerUI> dmui =
        do_GetService("@mozilla.org/download-manager-ui;1", &rv);
      NS_ENSURE_SUCCESS(rv, rv);

      PRBool visible;
      rv = dmui->GetVisible(&visible);
      NS_ENSURE_SUCCESS(rv, rv);

      PRBool focus = PR_TRUE;
      if (branch)
        (void)branch->GetBoolPref(PREF_BDM_FOCUSWHENSTARTING, &focus);

      if (visible && !focus)
        return dmui->GetAttention();

      return dmui->Show(nsnull, id);
    }
    return rv;
  }

  NS_IMETHODIMP OnStateChange(nsIWebProgress* aWebProgress,
                              nsIRequest* aRequest, PRUint32 aStateFlags,
                              PRUint32 aStatus)
  {
    return mInner->OnStateChange(aWebProgress, aRequest, aStateFlags, aStatus);
  }
  
  NS_IMETHODIMP OnStatusChange(nsIWebProgress *aWebProgress,
                               nsIRequest *aRequest, nsresult aStatus,
                               const PRUnichar *aMessage)
  {
    return mInner->OnStatusChange(aWebProgress, aRequest, aStatus, aMessage);
  }

  NS_IMETHODIMP OnLocationChange(nsIWebProgress *aWebProgress,
                                 nsIRequest *aRequest, nsIURI *aLocation)
  {
    return mInner->OnLocationChange(aWebProgress, aRequest, aLocation);
  }
  
  NS_IMETHODIMP OnProgressChange(nsIWebProgress *aWebProgress,
                                 nsIRequest *aRequest,
                                 PRInt32 aCurSelfProgress,
                                 PRInt32 aMaxSelfProgress,
                                 PRInt32 aCurTotalProgress,
                                 PRInt32 aMaxTotalProgress)
  {
    return mInner->OnProgressChange(aWebProgress, aRequest,
                                    aCurSelfProgress,
                                    aMaxSelfProgress,
                                    aCurTotalProgress,
                                    aMaxTotalProgress);
  }

  NS_IMETHODIMP OnProgressChange64(nsIWebProgress *aWebProgress,
                                   nsIRequest *aRequest,
                                   PRInt64 aCurSelfProgress,
                                   PRInt64 aMaxSelfProgress,
                                   PRInt64 aCurTotalProgress,
                                   PRInt64 aMaxTotalProgress)
  {
    return mInner->OnProgressChange64(aWebProgress, aRequest,
                                      aCurSelfProgress,
                                      aMaxSelfProgress,
                                      aCurTotalProgress,
                                      aMaxTotalProgress);
  }

  NS_IMETHODIMP OnRefreshAttempted(nsIWebProgress *aWebProgress,
                                   nsIURI *aUri,
                                   PRInt32 aDelay,
                                   PRBool aSameUri,
                                   PRBool *allowRefresh)
  {
    *allowRefresh = PR_TRUE;
    return NS_OK;
  }

  NS_IMETHODIMP OnSecurityChange(nsIWebProgress *aWebProgress,
                                 nsIRequest *aRequest, PRUint32 aState)
  {
    return mInner->OnSecurityChange(aWebProgress, aRequest, aState);
  }

private:
  nsCOMPtr<nsIDownload> mInner;
};

NS_IMPL_ISUPPORTS3(nsDownloadProxy, nsITransfer,
                   nsIWebProgressListener, nsIWebProgressListener2)

#endif
