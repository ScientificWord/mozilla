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
 * Rajiv Dayal <rdayal@netscape.com>
 * Portions created by the Initial Developer are Copyright (C) 2002
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Dan Mosedale <dmose@netscape.com>
 *   Mark Banner <mark@standard8.demon.co.uk>
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
#include "nsAbLDAPReplicationQuery.h"
#include "nsAbLDAPReplicationService.h"
#include "nsAbLDAPReplicationData.h"
#include "nsILDAPURL.h"
#include "nsAbBaseCID.h"
#include "nsProxiedService.h"
#include "nsAbUtils.h"
#include "nsDirPrefs.h"
#include "nsCRT.h"
#include "prmem.h"
#include "nsIRDFService.h"

NS_IMPL_ISUPPORTS1(nsAbLDAPReplicationQuery, nsIAbLDAPReplicationQuery)

nsAbLDAPReplicationQuery::nsAbLDAPReplicationQuery()
    :  mInitialized(PR_FALSE)
{
}

nsresult nsAbLDAPReplicationQuery::InitLDAPData()
{
  nsresult rv;

  nsCOMPtr<nsIRDFService> rdfService = do_GetService("@mozilla.org/rdf/rdf-service;1", &rv);
  NS_ENSURE_SUCCESS(rv, rv);
 
  nsCAutoString resourceURI(kLDAPDirectoryRoot);
  resourceURI.Append(mDirPrefName);
 
  nsCOMPtr<nsIRDFResource> resource;
  rv = rdfService->GetResource(resourceURI, getter_AddRefs(resource));
  NS_ENSURE_SUCCESS(rv, rv);
 
  mDirectory = do_QueryInterface(resource, &rv);
  NS_ENSURE_SUCCESS(rv, rv);
 
  nsCAutoString fileName;
  rv = mDirectory->GetReplicationFileName(fileName);
  NS_ENSURE_SUCCESS(rv, rv);

  // this is done here to take care of the problem related to bug # 99124.
  // earlier versions of Mozilla could have the fileName associated with the directory
  // to be abook.mab which is the profile's personal addressbook. If the pref points to
  // it, calls nsDirPrefs to generate a new server filename.
  if (fileName.IsEmpty() || fileName.Equals(NS_LITERAL_CSTRING(kPersonalAddressbook)))
  {
    // Ensure fileName is empty for DIR_GenerateAbFileName to work
    // correctly.
    fileName.Truncate();
    // XXX This should be replaced by a local function at some stage.
    // For now we'll continue using the nsDirPrefs version.
    DIR_Server* server = DIR_GetServerFromList(mDirPrefName.get());
    if (server)
    {
      DIR_SetServerFileName(server);
      // Now ensure the prefs are saved
      DIR_SavePrefsForOneServer(server);
    }
  }
 
  rv = mDirectory->SetReplicationFileName(fileName);
  NS_ENSURE_SUCCESS(rv, rv);
 
  mURL = do_CreateInstance(NS_LDAPURL_CONTRACTID, &rv);
  if (NS_FAILED(rv)) 
    return rv;

  nsCOMPtr<nsIAbDirectory> abDirectory(do_QueryInterface(mDirectory, &rv));
  NS_ENSURE_SUCCESS(rv, rv);

  nsCAutoString uri;
  rv = abDirectory->GetURI(uri);
  NS_ENSURE_SUCCESS(rv, rv);

  rv = mURL->SetSpec(uri);
  if (NS_FAILED(rv)) 
    return rv;

  mConnection = do_CreateInstance(NS_LDAPCONNECTION_CONTRACTID, &rv);
  if (NS_FAILED(rv)) 
    return rv;

  mOperation = do_CreateInstance(NS_LDAPOPERATION_CONTRACTID, &rv);

  return rv;
}

nsresult nsAbLDAPReplicationQuery::CreateNewLDAPOperation()
{
  nsresult rv;
  nsCOMPtr <nsILDAPMessageListener> oldListener;
  mOperation->GetMessageListener(getter_AddRefs(oldListener));
  // release old and create a new instance
  mOperation = do_CreateInstance(NS_LDAPOPERATION_CONTRACTID, &rv);
  NS_ENSURE_SUCCESS(rv, rv);
  return mOperation->Init(mConnection, oldListener, nsnull);
}


NS_IMETHODIMP nsAbLDAPReplicationQuery::ConnectToLDAPServer(nsILDAPURL *aURL, const nsACString & aAuthDN)
{
    NS_ENSURE_ARG_POINTER(aURL);
    if (!mInitialized) 
        return NS_ERROR_NOT_INITIALIZED;

    nsCAutoString host;
    nsresult rv = aURL->GetHost(host);
    if (NS_FAILED(rv)) 
        return rv;
    if (host.IsEmpty())
        return NS_ERROR_UNEXPECTED;

    PRInt32 port;
    rv = aURL->GetPort(&port);
    if (NS_FAILED(rv)) 
        return rv;
    if (!port)
        return NS_ERROR_UNEXPECTED;

    PRUint32 options;
    rv = aURL->GetOptions(&options);
    if (NS_FAILED(rv))
      return NS_ERROR_UNEXPECTED;

    // Initiate LDAP message listener to the current thread
    nsCOMPtr<nsILDAPMessageListener> listener;
    rv = NS_GetProxyForObject(NS_PROXY_TO_CURRENT_THREAD,
                  NS_GET_IID(nsILDAPMessageListener), 
                  NS_STATIC_CAST(nsILDAPMessageListener*, mDataProcessor),
                  NS_PROXY_SYNC | NS_PROXY_ALWAYS, 
                  getter_AddRefs(listener));
    if (!listener) 
        return NS_ERROR_FAILURE;

    // this could be a rebind call
    PRInt32 replicationState = nsIAbLDAPProcessReplicationData::kIdle;
    rv = mDataProcessor->GetReplicationState(&replicationState);
    if(NS_FAILED(rv)) 
        return rv;
    if((replicationState != nsIAbLDAPProcessReplicationData::kIdle))
    {
        // release old and create a new instance
        mConnection = do_CreateInstance(NS_LDAPCONNECTION_CONTRACTID, &rv);
        if(NS_FAILED(rv)) 
            return rv;

        // release old and create a new instance
        mOperation = do_CreateInstance(NS_LDAPOPERATION_CONTRACTID, &rv);
        if(NS_FAILED(rv)) 
            return rv;
    }

    PRUint32 protocolVersion;
    rv = mDirectory->GetProtocolVersion(&protocolVersion);
    NS_ENSURE_SUCCESS(rv, rv);

    // initialize the LDAP connection
    return mConnection->Init(host.get(), port, 
                             (options & nsILDAPURL::OPT_SECURE) ? PR_TRUE : PR_FALSE,
                             aAuthDN, listener, nsnull, protocolVersion);
}

NS_IMETHODIMP nsAbLDAPReplicationQuery::Init(const nsACString & aPrefName, nsIWebProgressListener *aProgressListener)
{
    if (aPrefName.IsEmpty())
        return NS_ERROR_UNEXPECTED;

    mDirPrefName = aPrefName;

    nsresult rv = InitLDAPData();
    if (NS_FAILED(rv)) 
        return rv;

    mDataProcessor =  do_CreateInstance(NS_ABLDAP_PROCESSREPLICATIONDATA_CONTRACTID, &rv);
    if (NS_FAILED(rv)) 
        return rv;

    // 'this' initialized
    mInitialized = PR_TRUE;

    return mDataProcessor->Init(this, aProgressListener);
}

NS_IMETHODIMP nsAbLDAPReplicationQuery::DoReplicationQuery()
{
    return ConnectToLDAPServer(mURL, EmptyCString());
}

NS_IMETHODIMP nsAbLDAPReplicationQuery::QueryAllEntries()
{
    if (!mInitialized) 
        return NS_ERROR_NOT_INITIALIZED;

    // get the search filter associated with the directory server url; 
    //
    nsCAutoString urlFilter;
    nsresult rv = mURL->GetFilter(urlFilter);
    if (NS_FAILED(rv)) 
        return rv;

    nsCAutoString dn;
    rv = mURL->GetDn(dn);
    if (NS_FAILED(rv)) 
        return rv;
    if (dn.IsEmpty())
        return NS_ERROR_UNEXPECTED;

    PRInt32 scope;
    rv = mURL->GetScope(&scope);
    if (NS_FAILED(rv)) 
        return rv;

    CharPtrArrayGuard attributes;
    rv = mURL->GetAttributes(attributes.GetSizeAddr(), attributes.GetArrayAddr());
    if (NS_FAILED(rv)) 
        return rv;

    rv = CreateNewLDAPOperation();
    NS_ENSURE_SUCCESS(rv, rv);
    return mOperation->SearchExt(dn, scope, urlFilter, 
                               attributes.GetSize(), attributes.GetArray(),
                               0, 0);
}

NS_IMETHODIMP nsAbLDAPReplicationQuery::CancelQuery()
{
    if (!mInitialized) 
        return NS_ERROR_NOT_INITIALIZED;

    return mDataProcessor->Abort();
}

NS_IMETHODIMP nsAbLDAPReplicationQuery::Done(PRBool aSuccess)
{
   if (!mInitialized) 
       return NS_ERROR_NOT_INITIALIZED;
   
   nsresult rv = NS_OK;
   nsCOMPtr<nsIAbLDAPReplicationService> replicationService = 
                            do_GetService(NS_ABLDAP_REPLICATIONSERVICE_CONTRACTID, &rv);
   if (NS_SUCCEEDED(rv))
      replicationService->Done(aSuccess);

   return rv;
}


NS_IMETHODIMP nsAbLDAPReplicationQuery::GetOperation(nsILDAPOperation * *aOperation)
{
   NS_ENSURE_ARG_POINTER(aOperation);
   if (!mInitialized) 
       return NS_ERROR_NOT_INITIALIZED;
   
   NS_IF_ADDREF(*aOperation = mOperation);

   return NS_OK;
}

NS_IMETHODIMP nsAbLDAPReplicationQuery::GetConnection(nsILDAPConnection * *aConnection)
{
   NS_ENSURE_ARG_POINTER(aConnection);
   if (!mInitialized) 
       return NS_ERROR_NOT_INITIALIZED;

   NS_IF_ADDREF(*aConnection = mConnection); 

   return NS_OK;
}

NS_IMETHODIMP nsAbLDAPReplicationQuery::GetReplicationURL(nsILDAPURL * *aReplicationURL)
{
   NS_ENSURE_ARG_POINTER(aReplicationURL);
   if (!mInitialized) 
       return NS_ERROR_NOT_INITIALIZED;

   NS_IF_ADDREF(*aReplicationURL = mURL); 

   return NS_OK;
}

NS_IMETHODIMP nsAbLDAPReplicationQuery::GetLDAPDirectory(nsIAbLDAPDirectory * *aDirectory)
{
  NS_ENSURE_ARG_POINTER(aDirectory);

  NS_IF_ADDREF(*aDirectory = mDirectory);

  return NS_OK;
}
