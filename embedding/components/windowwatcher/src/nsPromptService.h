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
 * Portions created by the Initial Developer are Copyright (C) 2001
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

#ifndef __nsPromptService_h
#define __nsPromptService_h

// {A2112D6A-0E28-421f-B46A-25C0B308CBD0}
#define NS_PROMPTSERVICE_CID \
 {0xa2112d6a, 0x0e28, 0x421f, {0xb4, 0x6a, 0x25, 0xc0, 0xb3, 0x8, 0xcb, 0xd0}}
// {150E7415-72D7-11DA-A924-00039386357A}
#define NS_NONBLOCKINGALERTSERVICE_CID \
 {0x150e7415, 0x72d7, 0x11da, {0xa9, 0x24, 0x00, 0x03, 0x93, 0x86, 0x35, 0x7a}}

#include "nsCOMPtr.h"
#include "nsIPromptService.h"
#include "nsPIPromptService.h"
#include "nsINonBlockingAlertService.h"
#include "nsIWindowWatcher.h"

class nsIDOMWindow;
class nsIDialogParamBlock;

class nsPromptService: public nsIPromptService,
                       public nsPIPromptService,
                       public nsINonBlockingAlertService {

public:

  nsPromptService();
  virtual ~nsPromptService();

  nsresult Init();

  NS_DECL_NSIPROMPTSERVICE
  NS_DECL_NSPIPROMPTSERVICE
  NS_DECL_NSINONBLOCKINGALERTSERVICE
  NS_DECL_ISUPPORTS

private:
  nsresult GetLocaleString(const char *aKey, PRUnichar **aResult);

  nsCOMPtr<nsIWindowWatcher> mWatcher;
};

#endif

