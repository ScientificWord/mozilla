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
 * Portions created by the Initial Developer are Copyright (C) 1999
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
#include "prprf.h"
#include "prmem.h"
#include "nsCOMPtr.h"
#include "nsString.h"
#include "nsIStringBundle.h"
#include "nsLocalStringBundle.h"
#include "nsIServiceManager.h"
#include "nsIURI.h"

#define LOCAL_MSGS_URL       "chrome://messenger/locale/localMsgs.properties"

nsLocalStringService::nsLocalStringService()
{
}

nsLocalStringService::~nsLocalStringService()
{}

NS_IMPL_ADDREF(nsLocalStringService)
NS_IMPL_RELEASE(nsLocalStringService)

NS_INTERFACE_MAP_BEGIN(nsLocalStringService)
   NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(nsISupports, nsIMsgStringService)
   NS_INTERFACE_MAP_ENTRY(nsIMsgStringService)
NS_INTERFACE_MAP_END

NS_IMETHODIMP 
nsLocalStringService::GetStringByID(PRInt32 aStringID, PRUnichar ** aString)
{
  nsresult rv = NS_OK;
  
  if (!mLocalStringBundle)
    rv = InitializeStringBundle();

  NS_ENSURE_TRUE(mLocalStringBundle, NS_ERROR_UNEXPECTED);
  NS_ENSURE_SUCCESS(mLocalStringBundle->GetStringFromID(aStringID, aString), NS_ERROR_UNEXPECTED);

  return rv;
}

NS_IMETHODIMP
nsLocalStringService::GetBundle(nsIStringBundle **aResult)
{
  NS_ENSURE_ARG_POINTER(aResult);
  nsresult rv = NS_OK;
  if (!mLocalStringBundle)
    rv = InitializeStringBundle();
  NS_ENSURE_SUCCESS(rv, rv);

  *aResult = mLocalStringBundle;
  NS_IF_ADDREF(*aResult);
  return NS_OK;
}

nsresult
nsLocalStringService::InitializeStringBundle()
{
  nsresult rv;
  nsCOMPtr<nsIStringBundleService> stringService = do_GetService(NS_STRINGBUNDLE_CONTRACTID, &rv);
  NS_ENSURE_SUCCESS(rv, rv);
  NS_ENSURE_TRUE(stringService, NS_ERROR_FAILURE);

  rv = stringService->CreateBundle(LOCAL_MSGS_URL, getter_AddRefs(mLocalStringBundle));
  return rv;
}

