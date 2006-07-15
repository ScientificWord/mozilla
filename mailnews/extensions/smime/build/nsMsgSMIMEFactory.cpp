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
 *   Scott MacGregor <mscott@netscape.com>
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

#include "nsISupports.h"
#include "nsCOMPtr.h"

#include "nsIFactory.h"
#include "nsIGenericFactory.h"
#include "nsIServiceManager.h"
#include "nsIModule.h"

#include "pratom.h"
#include "nsMsgSMIMECID.h"
#include "nsMsgCompCID.h"

/* Include all of the interfaces our factory can generate components for */
#include "nsMsgComposeSecure.h"
#include "nsSMimeJSHelper.h"
#include "nsEncryptedSMIMEURIsService.h"

NS_GENERIC_FACTORY_CONSTRUCTOR(nsMsgComposeSecure)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsMsgSMIMEComposeFields)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsSMimeJSHelper)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsEncryptedSMIMEURIsService)

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

static const nsModuleComponentInfo components[] =
{
  { "Msg Compose Secure",
    NS_MSGCOMPOSESECURE_CID,
    NS_MSGCOMPOSESECURE_CONTRACTID,
    nsMsgComposeSecureConstructor },
  { "Msg SMIME Compose Fields",
    NS_MSGSMIMECOMPFIELDS_CID,
    NS_MSGSMIMECOMPFIELDS_CONTRACTID,
    nsMsgSMIMEComposeFieldsConstructor },
  { "SMIME JS Helper",
    NS_SMIMEJSJELPER_CID,
    NS_SMIMEJSHELPER_CONTRACTID,
    nsSMimeJSHelperConstructor },
  { "SMIME Encrypted URI Cache Service",
    NS_SMIMEENCRYPTURISERVICE_CID,
    NS_SMIMEENCRYPTURISERVICE_CONTRACTID,
    nsEncryptedSMIMEURIsServiceConstructor }
};

  
NS_IMPL_NSGETMODULE(nsMsgSMIMEModule, components)
