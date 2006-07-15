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
 * Sun Microsystem.
 * Portions created by the Initial Developer are Copyright (C) 2003
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Joshua Xia <joshua.xia@sun.com>
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

#include "plstr.h"
#include "nsNetUtil.h"
#include "nsJVMAuthTools.h"
#include "nsIHttpAuthManager.h"

static NS_DEFINE_CID(kHttpAuthManagerCID, NS_HTTPAUTHMANAGER_CID);

//---------------------------------------------------
// implementation of interface nsIAuthenticationInfo
// --------------------------------------------------
NS_IMPL_ISUPPORTS1(nsAuthenticationInfoImp, nsIAuthenticationInfo)

nsAuthenticationInfoImp::nsAuthenticationInfoImp(char* username,
                                                 char* password)
{
    mUserName = username;
    mPassWord = password;
}

nsAuthenticationInfoImp::~nsAuthenticationInfoImp()
{
    if (mUserName)
        nsMemory::Free(mUserName);
        
    if (mPassWord)
        nsMemory::Free(mPassWord);
}

/* readonly attribute string username; */
NS_IMETHODIMP nsAuthenticationInfoImp::GetUsername(const char * *aUsername)
{
    *aUsername = mUserName;
    return NS_OK;
}

/* readonly attribute string password; */
NS_IMETHODIMP nsAuthenticationInfoImp::GetPassword(const char * *aPassword)
{
    *aPassword = mPassWord;
    return NS_OK;
}


//---------------------------------------------------
// implementation of interface nsIJVMAuthTools
// --------------------------------------------------
NS_IMPL_AGGREGATED(nsJVMAuthTools)

nsJVMAuthTools::nsJVMAuthTools(nsISupports* outer)
{
    NS_INIT_AGGREGATED(outer);
}

nsJVMAuthTools::~nsJVMAuthTools(void)
{
}

NS_INTERFACE_MAP_BEGIN_AGGREGATED(nsJVMAuthTools)
    NS_INTERFACE_MAP_ENTRY(nsIJVMAuthTools)
NS_INTERFACE_MAP_END

NS_METHOD
nsJVMAuthTools::GetAuthenticationInfo(const char* protocol,
                                      const char* host,
                                      PRInt32 port, 
                                      const char* scheme,
                                      const char* realm,
                                      nsIAuthenticationInfo **_retval)
{
    if (!protocol || !host || !scheme || !realm)
        return NS_ERROR_INVALID_ARG;

    if (!PL_strcasecmp(protocol, "HTTP") && !PL_strcasecmp(protocol, "HTTPS"))
        return NS_ERROR_INVALID_ARG;

    nsCOMPtr<nsIHttpAuthManager> authManager = do_GetService(kHttpAuthManagerCID);
    if (!authManager)
        return NS_ERROR_FAILURE;
    
    nsDependentCString protocolString(protocol);
    nsDependentCString hostString(host);
    nsDependentCString schemeString(scheme);
    nsDependentCString realmString(realm);
    nsAutoString       domainString, username, password;
    
    nsresult rv = authManager->GetAuthIdentity(protocolString,
                                               hostString,
                                               port,
                                               schemeString,
                                               realmString,
                                               EmptyCString(), 
                                               domainString,
                                               username,
                                               password);
    if (NS_FAILED(rv))
        return NS_ERROR_FAILURE;

    nsAuthenticationInfoImp* authInfo = new nsAuthenticationInfoImp(
                                            ToNewUTF8String(username),
                                            ToNewUTF8String(password));
    NS_ENSURE_TRUE(authInfo, NS_ERROR_OUT_OF_MEMORY);
    NS_ADDREF(authInfo);
    *_retval = authInfo;
    
    return NS_OK;
}

NS_METHOD
nsJVMAuthTools::SetAuthenticationInfo(const char* protocol,
                                      const char* host,
                                      PRInt32 port, 
                                      const char* scheme,
                                      const char* realm,
                                      const char *username,
                                      const char *password)
{
    if (!protocol || !host || !scheme || !realm)
        return NS_ERROR_INVALID_ARG;
    
    if (!PL_strcasecmp(protocol, "HTTP") && !PL_strcasecmp(protocol, "HTTPS"))
        return NS_ERROR_INVALID_ARG;

    nsCOMPtr<nsIHttpAuthManager> authManager = do_GetService(kHttpAuthManagerCID);
    if (!authManager)
        return NS_ERROR_FAILURE;
    
    nsDependentCString protocolString(protocol);
    nsDependentCString hostString(host);
    nsDependentCString schemeString(scheme);
    nsDependentCString realmString(realm);
    
    nsresult rv = authManager->SetAuthIdentity(protocolString,
                                               hostString,
                                               port,
                                               schemeString,
                                               realmString,
                                               EmptyCString(),
                                               EmptyString(), 
                                               NS_ConvertUTF8toUTF16(username),
                                               NS_ConvertUTF8toUTF16(password));
    return rv;
}

