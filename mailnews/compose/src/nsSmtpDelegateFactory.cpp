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
 *   Alec Flett <alecf@netscape.com>
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

#include "nsSmtpDelegateFactory.h"

#include "nsCOMPtr.h"
#include "nsXPIDLString.h"
#include "nsString.h"
#include "nsIServiceManager.h"

#include "nsIURL.h"
#include "nsNetCID.h"

#include "nsIRDFResource.h"

#include "nsISmtpServer.h"
#include "nsISmtpService.h"
#include "nsMsgCompCID.h"
#include "nsReadableUtils.h"
#include "nsEscape.h"
#include "prmem.h"

NS_IMPL_ISUPPORTS1(nsSmtpDelegateFactory, nsIRDFDelegateFactory)

nsSmtpDelegateFactory::nsSmtpDelegateFactory()
{
}

nsSmtpDelegateFactory::~nsSmtpDelegateFactory()
{
}
    
NS_IMETHODIMP
nsSmtpDelegateFactory::CreateDelegate(nsIRDFResource *aOuter,
                                      const char *aKey,
                                      const nsIID & aIID, void * *aResult)
{
  nsresult rv;
  const char* uri;
  aOuter->GetValueConst(&uri);

  nsCOMPtr<nsIURL> url = do_CreateInstance(NS_STANDARDURL_CONTRACTID, &rv);
  NS_ENSURE_SUCCESS(rv, rv);
  
  rv = url->SetSpec(nsDependentCString(uri));

  // separate out username, hostname
  nsCAutoString hostname;
  nsCAutoString userpass;

  rv = url->GetUserPass(userpass);
  NS_ENSURE_SUCCESS(rv, rv);
  
  url->GetAsciiHost(hostname);
  NS_ENSURE_SUCCESS(rv, rv);

  nsCOMPtr<nsISmtpService> smtpService = do_GetService(NS_SMTPSERVICE_CONTRACTID, &rv);
  NS_ENSURE_SUCCESS(rv, rv);

  char *unescapedUserPass = ToNewCString(userpass);
  if (!unescapedUserPass)
    return NS_ERROR_OUT_OF_MEMORY;

  nsUnescape(unescapedUserPass);
  nsCOMPtr<nsISmtpServer> smtpServer;
  rv = smtpService->FindServer(unescapedUserPass, hostname.get(), getter_AddRefs(smtpServer));
  PR_FREEIF(unescapedUserPass);

  // no server, it's a failure!
  if (NS_FAILED(rv)) return rv;
  if (!smtpServer) return NS_ERROR_FAILURE;

  return smtpServer->QueryInterface(aIID, aResult);
}
