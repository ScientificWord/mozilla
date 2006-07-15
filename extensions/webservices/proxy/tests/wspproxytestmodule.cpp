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
 * Portions created by the Initial Developer are Copyright (C) 2001
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   John Bandhauer (jband@netscape.com)
 *   Vidur Apparao (vidur@netscape.com)
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
#include "nsIGenericFactory.h"
#include "nsIServiceManager.h"
#include "nsICategoryManager.h"
#include "nsIScriptNameSpaceManager.h"
#include "nsStringAPI.h"
#include "wspproxytest.h"
#include "nsXPCOMCID.h"
#include "nsServiceManagerUtils.h"
#include "nsComponentManagerUtils.h"
#include "nsIClassInfoImpl.h"

NS_GENERIC_FACTORY_CONSTRUCTOR(WSPProxyTest)
NS_DECL_CLASSINFO(WSPProxyTest)

static NS_METHOD 
RegisterWSPProxyTest(nsIComponentManager *aCompMgr,
                     nsIFile *aPath,
                     const char *registryLocation,
                     const char *componentType,
                     const nsModuleComponentInfo *info)
{
  nsresult rv = NS_OK;

  nsCOMPtr<nsICategoryManager> catman =
    do_GetService(NS_CATEGORYMANAGER_CONTRACTID, &rv);

  if (NS_FAILED(rv))
    return rv;

  nsCString previous;
  rv = catman->AddCategoryEntry(JAVASCRIPT_GLOBAL_CONSTRUCTOR_CATEGORY,
                                "WebServiceProxyTest",
                                NS_WSPPROXYTEST_CONTRACTID,
                                PR_TRUE, PR_TRUE, getter_Copies(previous));
  return rv;
}

static const nsModuleComponentInfo components[] = {
  { "WebServiceProxyTest", NS_WSPPROXYTEST_CID, NS_WSPPROXYTEST_CONTRACTID,
    WSPProxyTestConstructor, RegisterWSPProxyTest, nsnull, nsnull, 
    NS_CI_INTERFACE_GETTER_NAME(WSPProxyTest), 
    nsnull, &NS_CLASSINFO_NAME(WSPProxyTest), 
    nsIClassInfo::DOM_OBJECT }
};

NS_IMPL_NSGETMODULE(WSPProxyTestModule, components)
