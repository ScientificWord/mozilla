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
#include "nsIModule.h"
#include "nsIGenericFactory.h"
#include "nsWalletService.h"
#include "nsPasswordManager.h"
#include "nsCPasswordManager.h"

// Define the constructor function for the nsWalletlibService
NS_GENERIC_FACTORY_CONSTRUCTOR_INIT(nsWalletlibService, Init)
NS_GENERIC_FACTORY_CONSTRUCTOR_INIT(nsPasswordManager, Init)
NS_GENERIC_FACTORY_CONSTRUCTOR_INIT(nsSingleSignOnPrompt, Init)

// The list of components we register
static const nsModuleComponentInfo components[] = {
    { NS_WALLETSERVICE_CLASSNAME, NS_WALLETSERVICE_CID,
      NS_WALLETSERVICE_CONTRACTID, nsWalletlibServiceConstructor,
      nsWalletlibService::RegisterProc,
      nsWalletlibService::UnregisterProc
    },
    { NS_PASSWORDMANAGER_CLASSNAME, NS_PASSWORDMANAGER_CID,
      NS_PASSWORDMANAGER_CONTRACTID, nsPasswordManagerConstructor },
    { NS_SINGLESIGNONPROMPT_CLASSNAME, NS_SINGLESIGNONPROMPT_CID,
      NS_SINGLESIGNONPROMPT_CONTRACTID, nsSingleSignOnPromptConstructor }
};

NS_IMPL_NSGETMODULE(nsWalletModule, components)
