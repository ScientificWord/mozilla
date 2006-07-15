/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * 
 * ***** BEGIN LICENSE BLOCK *****
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
 * The Original Code is mozilla.org Code.
 *
 * The Initial Developer of the Original Code is
 * Aaron Leventhal.
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

#include "nsIGenericFactory.h"
#include "nsAccessProxy.h"
#include "nsIServiceManager.h"
#include "nsIRegistry.h"
#include "prprf.h"
#include "nsCRT.h"
#include "nsICategoryManager.h"

////////////////////////////////////////////////////////////////////////
// Define a table of CIDs implemented by this module along with other
// information like the function to create an instance, contractid, and
// class name.
//
// The Registration and Unregistration proc are optional in the structure.
//


// This function is called at component registration time
static NS_METHOD nsAccessProxyRegistrationProc(nsIComponentManager *aCompMgr,
  nsIFile *aPath, const char *registryLocation, const char *componentType,
  const nsModuleComponentInfo *info)
{
  // This function performs the extra step of installing us as
  // an application component. This makes sure that we're
  // initialized on application startup.

  nsresult rv;
  nsCOMPtr<nsICategoryManager> categoryManager(do_GetService(NS_CATEGORYMANAGER_CONTRACTID, &rv));
  if (NS_SUCCEEDED(rv)) 
    rv = categoryManager->AddCategoryEntry(APPSTARTUP_CATEGORY, "Access Proxy", 
      "service," NS_ACCESSPROXY_CONTRACTID, PR_TRUE, PR_TRUE, nsnull);
  return rv;
}


NS_GENERIC_FACTORY_SINGLETON_CONSTRUCTOR(nsAccessProxy,nsAccessProxy::GetInstance)

static void PR_CALLBACK AccessProxyModuleDtor(nsIModule* self)
{
    nsAccessProxy::ReleaseInstance();
}

static const nsModuleComponentInfo components[] =
{
  { "AccessProxy Component", NS_ACCESSPROXY_CID, NS_ACCESSPROXY_CONTRACTID,
    nsAccessProxyConstructor, nsAccessProxyRegistrationProc,
    nsnull  // Unregistration proc
  }
};

NS_IMPL_NSGETMODULE_WITH_DTOR(nsAccessProxy, components, AccessProxyModuleDtor)




