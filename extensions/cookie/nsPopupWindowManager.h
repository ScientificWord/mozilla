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
 * Portions created by the Initial Developer are Copyright (C) 2002
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
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

#ifndef nsPopupWindowManager_h__
#define nsPopupWindowManager_h__

#include "nsCOMPtr.h"

#include "nsIObserver.h"
#include "nsIPermissionManager.h"
#include "nsIPopupWindowManager.h"
#include "nsWeakReference.h"

class nsIURI;

class nsPopupWindowManager : public nsIPopupWindowManager,
                             public nsIObserver,
                             public nsSupportsWeakReference {

public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIPOPUPWINDOWMANAGER
  NS_DECL_NSIOBSERVER

  nsPopupWindowManager();
  virtual ~nsPopupWindowManager();
  nsresult Init();

private:
  PRUint32                       mPolicy;
  nsCOMPtr<nsIPermissionManager> mPermissionManager;
};

// {822bcd11-6432-48be-9e9d-36f7804b7747}
#define NS_POPUPWINDOWMANAGER_CID \
 {0x822bcd11, 0x6432, 0x48be, {0x9e, 0x9d, 0x36, 0xf7, 0x80, 0x4b, 0x77, 0x47}}

#endif /* nsPopupWindowManager_h__ */
