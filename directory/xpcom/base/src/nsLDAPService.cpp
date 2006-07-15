/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
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
 * The Original Code is the mozilla.org LDAP XPCOM SDK.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2000
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Leif Hedstrom <leif@netscape.com>
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

#include "nsLDAPInternal.h"
#include "nsLDAPService.h"
#include "nsLDAPConnection.h"
#include "nsLDAPOperation.h"
#include "nsXPIDLString.h"
#include "nsReadableUtils.h"
#include "nsIServiceManager.h"
#include "nsIConsoleService.h"
#include "nsILDAPURL.h"
#include "nsAutoLock.h"
#include "nsCRT.h"
#include "nsILDAPErrors.h"

// Constants for CIDs used here.
//
static NS_DEFINE_CID(kLDAPConnectionCID, NS_LDAPCONNECTION_CID);
static NS_DEFINE_CID(kLDAPOperationCID, NS_LDAPOPERATION_CID);

// First we provide all the methods for the "local" class
// nsLDAPServiceEntry.
//

// constructor
//
nsLDAPServiceEntry::nsLDAPServiceEntry()
    : mLeases(0),
      mDelete(PR_FALSE),
      mRebinding(PR_FALSE)

{
    mTimestamp = LL_Zero();
}

// destructor
//
nsLDAPServiceEntry::~nsLDAPServiceEntry()
{
}

// Init function
//
PRBool nsLDAPServiceEntry::Init()
{
    return PR_TRUE;
}

// Set/Get the timestamp when this server was last used. We might have
// to use an "interval" here instead, see Bug #76887.
//
PRTime nsLDAPServiceEntry::GetTimestamp()
{
    return mTimestamp;
}
void nsLDAPServiceEntry::SetTimestamp()
{
    mTimestamp = PR_Now();
}


// Increment, decrement and Get the leases. This code might go away
// with bug #75954.
//
void nsLDAPServiceEntry::IncrementLeases()
{
    mLeases++;
}
PRBool nsLDAPServiceEntry::DecrementLeases()
{
    if (!mLeases) {
        return PR_FALSE;
    }
    mLeases--;

    return PR_TRUE;
}
PRUint32 nsLDAPServiceEntry::GetLeases()
{
    return mLeases;
}

// Get/Set the nsLDAPServer object for this entry.
//
already_AddRefed<nsILDAPServer> nsLDAPServiceEntry::GetServer()
{
    nsILDAPServer *server;

    NS_IF_ADDREF(server = mServer);
    return server;
}
PRBool nsLDAPServiceEntry::SetServer(nsILDAPServer *aServer)
{
    if (!aServer) {
        return PR_FALSE;
    }
    mServer = aServer;

    return PR_TRUE;
}

// Get/Set/Clear the nsLDAPConnection object for this entry.
//
already_AddRefed<nsILDAPConnection> nsLDAPServiceEntry::GetConnection()
{
    nsILDAPConnection *conn;

    NS_IF_ADDREF(conn = mConnection);
    return conn;
}
void nsLDAPServiceEntry::SetConnection(nsILDAPConnection *aConnection)
{
    mConnection = aConnection;
}

// Get/Set the nsLDAPMessage object for this entry (it's a "cache").
//
already_AddRefed<nsILDAPMessage> nsLDAPServiceEntry::GetMessage()
{
    nsILDAPMessage *message;

    NS_IF_ADDREF(message = mMessage);
    return message;
}
void nsLDAPServiceEntry::SetMessage(nsILDAPMessage *aMessage)
{
    mMessage = aMessage;
}

// Push/Pop pending listeners/callback for this server entry. This is
// implemented as a "stack" on top of the nsVoidArrays, since we can
// potentially have more than one listener waiting for the connection
// to be available for consumption.
//
already_AddRefed<nsILDAPMessageListener> nsLDAPServiceEntry::PopListener()
{
    nsILDAPMessageListener *listener;
    PRInt32 count = mListeners.Count();
    if (!count) {
        return 0;
    }

    listener = mListeners[0];
    NS_ADDREF(listener); // keep it alive
    mListeners.RemoveObjectAt(0);

    return listener;
}
PRBool nsLDAPServiceEntry::PushListener(nsILDAPMessageListener *listener)
{
    PRBool ret;
    ret = mListeners.InsertObjectAt(listener, mListeners.Count());

    return ret;
}

// Mark this server to currently be rebinding. This is to avoid a
// race condition where multiple consumers could potentially request
// to reconnect the connection.
//
PRBool nsLDAPServiceEntry::IsRebinding()
{
    return mRebinding;
}
void nsLDAPServiceEntry::SetRebinding(PRBool aState)
{
    mRebinding = aState;
}

// Mark a service entry for deletion, this is "dead" code right now,
// see bug #75966.
//
PRBool nsLDAPServiceEntry::DeleteEntry()
{
    mDelete = PR_TRUE;

    return PR_TRUE;
}
// This is the end of the nsLDAPServiceEntry class


// Here begins the implementation for nsLDAPService
// 
NS_IMPL_THREADSAFE_ISUPPORTS2(nsLDAPService,
                              nsILDAPService,
                              nsILDAPMessageListener)


// constructor
//
nsLDAPService::nsLDAPService()
    : mLock(0),
      mServers(0),
      mConnections(0)
{
}

// destructor
//
nsLDAPService::~nsLDAPService()
{
    // Delete the hash table holding the entries
    if (mServers) {
        delete mServers;
    }

    // Delete the hash holding the "reverse" lookups from conn to server
    if (mConnections) {
        delete mConnections;
    }

    // Delete the lock object
    if (mLock) {
        PR_DestroyLock(mLock);
    }
}

// Initializer, create some internal hash tables etc.
//
nsresult nsLDAPService::Init()
{
    if (!mLock) {
        mLock = PR_NewLock();
        if (!mLock) {
            NS_ERROR("nsLDAPService::Init: out of memory ");
            return NS_ERROR_OUT_OF_MEMORY;
        }
    }

    if (!mServers) {
        mServers = new nsHashtable(16, PR_FALSE);
        if (!mServers) {
            NS_ERROR("nsLDAPService::Init: out of memory ");
            return NS_ERROR_OUT_OF_MEMORY;
        }
    }

    if (!mConnections) {
        mConnections = new nsHashtable(16, PR_FALSE);
        if (!mConnections) {
            NS_ERROR("nsLDAPService::Init: out of memory ");
            return NS_ERROR_OUT_OF_MEMORY;
        }
    }

    return NS_OK;
}

// void addServer (in nsILDAPServer aServer);
NS_IMETHODIMP nsLDAPService::AddServer(nsILDAPServer *aServer)
{
    nsLDAPServiceEntry *entry;
    nsXPIDLString key;
    nsresult rv;
    
    if (!aServer) {
        NS_ERROR("nsLDAPService::AddServer: null pointer ");
        return NS_ERROR_NULL_POINTER;
    }

    // Set up the hash key for the server entry
    //
    rv = aServer->GetKey(getter_Copies(key));
    if (NS_FAILED(rv)) {
        switch (rv) {
        // Only pass along errors we are aware of
        //
        case NS_ERROR_OUT_OF_MEMORY:
        case NS_ERROR_NULL_POINTER:
            return rv;

        default:
            return NS_ERROR_FAILURE;
        }
    }

    // Create the new service server entry, and add it into the hash table
    //
    entry = new nsLDAPServiceEntry;
    if (!entry) {
        NS_ERROR("nsLDAPService::AddServer: out of memory ");
        return NS_ERROR_OUT_OF_MEMORY;
    }
    if (!entry->Init()) {
        delete entry;
        NS_ERROR("nsLDAPService::AddServer: out of memory ");
        return NS_ERROR_OUT_OF_MEMORY;
    }

    if (!entry->SetServer(aServer)) {
        delete entry;
        return NS_ERROR_FAILURE;
    }

    // We increment the refcount here for the server entry, when
    // we purge a server completely from the service (TBD), we
    // need to decrement the counter as well.
    //
    {
        nsStringKey hashKey(key);
        nsAutoLock lock(mLock);

        if (mServers->Exists(&hashKey)) {
            // Collision detected, lets just throw away this service entry
            // and keep the old one.
            //
            delete entry;
            return NS_ERROR_FAILURE;
        }
        mServers->Put(&hashKey, entry);
    }
    NS_ADDREF(aServer);

    return NS_OK;
}

// void deleteServer (in wstring aKey);
NS_IMETHODIMP nsLDAPService::DeleteServer(const PRUnichar *aKey)
{
    nsLDAPServiceEntry *entry;
    nsStringKey hashKey(aKey, -1, nsStringKey::NEVER_OWN);
    nsAutoLock lock(mLock);
        
    // We should probably rename the key for this entry now that it's
    // "deleted", so that we can add in a new one with the same ID.
    // This is bug #77669.
    //
    entry = NS_STATIC_CAST(nsLDAPServiceEntry *, mServers->Get(&hashKey));
    if (entry) {
        if (entry->GetLeases() > 0) {
            return NS_ERROR_FAILURE;
        }
        entry->DeleteEntry();
    } else {
        // There is no Server entry for this key
        //
        return NS_ERROR_FAILURE;
    }

    return NS_OK;
}

// nsILDAPServer getServer (in wstring aKey);
NS_IMETHODIMP nsLDAPService::GetServer(const PRUnichar *aKey,
                                       nsILDAPServer **_retval)
{
    nsLDAPServiceEntry *entry;
    nsStringKey hashKey(aKey, -1, nsStringKey::NEVER_OWN);
    nsAutoLock lock(mLock);

    if (!_retval) {
        NS_ERROR("nsLDAPService::GetServer: null pointer ");
        return NS_ERROR_NULL_POINTER;
    }

    entry = NS_STATIC_CAST(nsLDAPServiceEntry *, mServers->Get(&hashKey));
    if (!entry) {
        *_retval = 0;
        return NS_ERROR_FAILURE;
    }
    if (!(*_retval = entry->GetServer().get())) {
        return NS_ERROR_FAILURE;
    }

    return NS_OK;
}

//void requestConnection (in wstring aKey,
//                        in nsILDAPMessageListener aMessageListener);
NS_IMETHODIMP nsLDAPService::RequestConnection(const PRUnichar *aKey,
                                 nsILDAPMessageListener *aListener)
{
    nsLDAPServiceEntry *entry;
    nsCOMPtr<nsILDAPConnection> conn;
    nsCOMPtr<nsILDAPMessage> message;
    nsresult rv;
    nsStringKey hashKey(aKey, -1, nsStringKey::NEVER_OWN);

    if (!aListener) {
        NS_ERROR("nsLDAPService::RequestConection: null pointer ");
        return NS_ERROR_NULL_POINTER;
    }

    // Try to find a possibly cached connection and LDAP message.
    //
    {
        nsAutoLock lock(mLock);

        entry = NS_STATIC_CAST(nsLDAPServiceEntry *, mServers->Get(&hashKey));
        if (!entry) {
            return NS_ERROR_FAILURE;
        }
        entry->SetTimestamp();

        conn = entry->GetConnection();
        message = entry->GetMessage();
    }

    if (conn) {
        if (message) {
            // We already have the connection, and the message, ready to
            // be used. This might be confusing, since we actually call
            // the listener before this function returns, see bug #75899.
            //
            aListener->OnLDAPMessage(message);
            return NS_OK;
        }
    } else {
        rv = EstablishConnection(entry, aListener);
        if (NS_FAILED(rv)) {
            return rv;
        }

    }

    // We got a new connection, now push the listeners on our stack,
    // until we get the LDAP message back.
    //
    {
        nsAutoLock lock(mLock);
            
        entry = NS_STATIC_CAST(nsLDAPServiceEntry *, mServers->Get(&hashKey));
        if (!entry ||
            !entry->PushListener(NS_STATIC_CAST(nsILDAPMessageListener *,
                                                aListener))) {
            return NS_ERROR_FAILURE;
        }
    }

    return NS_OK;
}

// nsILDAPConnection getConnection (in wstring aKey);
NS_IMETHODIMP nsLDAPService::GetConnection(const PRUnichar *aKey,
                                           nsILDAPConnection **_retval)
{
    nsLDAPServiceEntry *entry;
    nsStringKey hashKey(aKey, -1, nsStringKey::NEVER_OWN);
    nsAutoLock lock(mLock);

    if (!_retval) {
        NS_ERROR("nsLDAPService::GetConnection: null pointer ");
        return NS_ERROR_NULL_POINTER;
    }

    entry = NS_STATIC_CAST(nsLDAPServiceEntry *, mServers->Get(&hashKey));
    if (!entry) {
        *_retval = 0;
        return NS_ERROR_FAILURE;
    }
    entry->SetTimestamp();
    entry->IncrementLeases();
    if (!(*_retval = entry->GetConnection().get())){
        return NS_ERROR_FAILURE;
    }

    return NS_OK;
}

// void releaseConnection (in wstring aKey);
NS_IMETHODIMP nsLDAPService::ReleaseConnection(const PRUnichar *aKey)
{
    nsLDAPServiceEntry *entry;
    nsStringKey hashKey(aKey, -1, nsStringKey::NEVER_OWN);
    nsAutoLock lock(mLock);

    entry = NS_STATIC_CAST(nsLDAPServiceEntry *, mServers->Get(&hashKey));
    if (!entry) {
        return NS_ERROR_FAILURE;
    }

    if (entry->GetLeases() > 0) {
        entry->SetTimestamp();
        entry->DecrementLeases();
    } else {
        // Releasing a non-leased connection is currently a No-Op.
        //
    }

    return NS_OK;
}

// void reconnectConnection (in wstring aKey,
//                           in nsILDAPMessageListener aMessageListener);
NS_IMETHODIMP nsLDAPService::ReconnectConnection(const PRUnichar *aKey,
                                 nsILDAPMessageListener *aListener)
{
    nsLDAPServiceEntry *entry;
    nsresult rv;
    nsStringKey hashKey(aKey, -1, nsStringKey::NEVER_OWN);

    if (!aListener) {
        NS_ERROR("nsLDAPService::ReconnectConnection: null pointer ");
        return NS_ERROR_NULL_POINTER;
    }

    {
        nsAutoLock lock(mLock);
        
        entry = NS_STATIC_CAST(nsLDAPServiceEntry *, mServers->Get(&hashKey));
        if (!entry) {
            return NS_ERROR_FAILURE;
        }
        entry->SetTimestamp();

        if (entry->IsRebinding()) {
            if (!entry->PushListener(aListener)) {
                return NS_ERROR_FAILURE;
            }

            return NS_OK;
        }

        // Clear the old connection and message, which should get garbaged
        // collected now. We mark this as being "rebinding" now, and it
        // we be marked as finished either if there's an error condition,
        // or if the OnLDAPMessage() method gets called (i.e. bind() done).
        //
        entry->SetMessage(0);
        entry->SetConnection(0);

        // Get a new connection
        //
        entry->SetRebinding(PR_TRUE);
    }

    rv = EstablishConnection(entry, aListener);
    if (NS_FAILED(rv)) {
        return rv;
    }

    {
        nsAutoLock lock(mLock);
        
        if (!entry->PushListener(NS_STATIC_CAST(nsILDAPMessageListener *,
                                                aListener))) {
            entry->SetRebinding(PR_FALSE);
            return NS_ERROR_FAILURE;
        }
    }

    return NS_OK;
}

/**
 * Messages received are passed back via this function.
 *
 * @arg aMessage  The message that was returned, 0 if none was.
 *
 * void OnLDAPMessage (in nsILDAPMessage aMessage)
 */
NS_IMETHODIMP 
nsLDAPService::OnLDAPMessage(nsILDAPMessage *aMessage)
{
    nsCOMPtr<nsILDAPOperation> operation;
    nsCOMPtr<nsILDAPConnection> connection;
    PRInt32 messageType;

    // XXXleif: NULL messages are supposedly allowed, but the semantics
    // are not definted (yet?). This is something to look out for...
    //


    // figure out what sort of message was returned
    //
    nsresult rv = aMessage->GetType(&messageType);
    if (NS_FAILED(rv)) {
        NS_ERROR("nsLDAPService::OnLDAPMessage(): unexpected error in "
                 "nsLDAPMessage::GetType()");
        return NS_ERROR_UNEXPECTED;
    }

    switch (messageType) {
    case LDAP_RES_BIND:
        // a bind has completed
        //
        rv = aMessage->GetOperation(getter_AddRefs(operation));
        if (NS_FAILED(rv)) {
            NS_ERROR("nsLDAPService::OnLDAPMessage(): unexpected error in "
                     "nsLDAPMessage::GetOperation()");
            return NS_ERROR_UNEXPECTED;
        }

        rv = operation->GetConnection(getter_AddRefs(connection));
        if (NS_FAILED(rv)) {
            NS_ERROR("nsLDAPService::OnLDAPMessage(): unexpected error in "
                     "nsLDAPOperation::GetConnection()");
            return NS_ERROR_UNEXPECTED;
        }

        // Now we have the connection, lets find the corresponding
        // server entry in the Service.
        //
        {
            nsCOMPtr<nsILDAPMessageListener> listener;
            nsCOMPtr<nsILDAPMessage> message;
            nsLDAPServiceEntry *entry;
            nsVoidKey connKey(NS_STATIC_CAST(nsILDAPConnection *,
                                             connection));
            nsAutoLock lock(mLock);

            entry = NS_STATIC_CAST(nsLDAPServiceEntry *,
                                   mConnections->Get(&connKey));
            if (!entry) {
                return NS_ERROR_FAILURE;
            }

            message = entry->GetMessage();
            if (message) {
                // We already have a message, lets keep that one.
                //
                return NS_ERROR_FAILURE;
            }

            entry->SetRebinding(PR_FALSE);
            entry->SetMessage(aMessage);

            // Now process all the pending callbacks/listeners. We
            // have to make sure to unlock before calling a listener,
            // since it's likely to call back into us again.
            //
            while (listener = entry->PopListener()) {
                lock.unlock();
                listener->OnLDAPMessage(aMessage);
                lock.lock();
            }
        }
        break;

    default:
        NS_WARNING("nsLDAPService::OnLDAPMessage(): unexpected LDAP message "
                   "received");

        // get the console service so we can log a message
        //
        nsCOMPtr<nsIConsoleService> consoleSvc = 
            do_GetService("@mozilla.org/consoleservice;1", &rv);
        if (NS_FAILED(rv)) {
            NS_ERROR("nsLDAPChannel::OnLDAPMessage() couldn't get console "
                     "service");
            break;
        }

        // log the message
        //
        rv = consoleSvc->LogStringMessage(
            NS_LITERAL_STRING("LDAP: WARNING: nsLDAPService::OnLDAPMessage(): Unexpected LDAP message received").get());
        NS_ASSERTION(NS_SUCCEEDED(rv), "nsLDAPService::OnLDAPMessage(): "
                     "consoleSvc->LogStringMessage() failed");
        break;
    }

    return NS_OK;
}

// void onLDAPInit (in nsILDAPConnection aConn, in nsresult aStatus); */
//
NS_IMETHODIMP
nsLDAPService::OnLDAPInit(nsILDAPConnection *aConn, nsresult aStatus)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

// Helper function to establish an LDAP connection properly.
//
nsresult
nsLDAPService::EstablishConnection(nsLDAPServiceEntry *aEntry,
                                   nsILDAPMessageListener *aListener)
{
    nsCOMPtr<nsILDAPOperation> operation;
    nsCOMPtr<nsILDAPServer> server;
    nsCOMPtr<nsILDAPURL> url;
    nsCOMPtr<nsILDAPConnection> conn, conn2;
    nsCOMPtr<nsILDAPMessage> message;
    nsCAutoString host;
    nsCAutoString binddn;
    nsCAutoString password;
    PRInt32 port;
    PRUint32 options;
    PRUint32 protocolVersion;
    nsresult rv;

    server = aEntry->GetServer();
    if (!server) {
        return NS_ERROR_FAILURE;
    }

    // Get username, password, and protocol version from the server entry.
    //
    rv = server->GetBinddn(binddn);
    if (NS_FAILED(rv)) {
        return NS_ERROR_FAILURE;
    }
    rv = server->GetPassword(password);
    if (NS_FAILED(rv)) {
        return NS_ERROR_FAILURE;
    }
    rv = server->GetProtocolVersion(&protocolVersion);
    if (NS_FAILED(rv)) {
        return NS_ERROR_FAILURE;
    }

    // Get the host and port out of the URL, which is in the server entry.
    //
    rv = server->GetUrl(getter_AddRefs(url));
    if (NS_FAILED(rv)) {
        return NS_ERROR_FAILURE;
    }
    rv = url->GetAsciiHost(host);
    if (NS_FAILED(rv)) {
        return NS_ERROR_FAILURE;
    }
    rv = url->GetPort(&port);
    if (NS_FAILED(rv)) {
        return NS_ERROR_FAILURE;
    }

    rv = url->GetOptions(&options);
    if (NS_FAILED(rv)) {
      return NS_ERROR_FAILURE;
    }
    // Create a new connection for this server.
    //
    conn = do_CreateInstance(kLDAPConnectionCID, &rv);
    if (NS_FAILED(rv)) {
        NS_ERROR("nsLDAPService::EstablishConnection(): could not create "
                 "@mozilla.org/network/ldap-connection;1");
        return NS_ERROR_FAILURE;
    }

    rv = conn->Init(host.get(), port, 
                    (options & nsILDAPURL::OPT_SECURE) ? PR_TRUE : PR_FALSE, 
                    binddn, this, nsnull, protocolVersion);
    if (NS_FAILED(rv)) {
        switch (rv) {
        // Only pass along errors we are aware of
        //
        case NS_ERROR_OUT_OF_MEMORY:
        case NS_ERROR_NOT_AVAILABLE:
        case NS_ERROR_FAILURE:
            return rv;

        case NS_ERROR_ILLEGAL_VALUE:
        default:
            return NS_ERROR_UNEXPECTED;
        }
    }

    // Try to detect a collision, i.e. someone established a connection
    // just before we did. If so, we drop ours. This code is somewhat
    // similar to what happens in RequestConnection(), i.e. we try to
    // call the listener directly if possible, and if not, push it on
    // the stack of pending requests.
    //
    {
        nsAutoLock lock(mLock);

        conn2 = aEntry->GetConnection();
        message = aEntry->GetMessage();
    }

    if (conn2) {
        // Drop the new connection, we can't use it.
        //
        conn = 0;
        if (message) {
            aListener->OnLDAPMessage(message);
            return NS_OK;
        }

        {
            nsAutoLock lock(mLock);

            if (!aEntry->PushListener(NS_STATIC_CAST(nsILDAPMessageListener *,
                                                     aListener))) {
                return NS_ERROR_FAILURE;
            }
        }

        return NS_OK;
    }

    // We made the connection, lets store it to the server entry,
    // and also update the reverse lookup tables (for finding the
    // server entry related to a particular connection).
    //
    {
        nsVoidKey connKey(NS_STATIC_CAST(nsILDAPConnection *, conn));
        nsAutoLock lock(mLock);

        aEntry->SetConnection(conn);
        mConnections->Put(&connKey, aEntry);
    }

    // Setup the bind() operation.
    //
    operation = do_CreateInstance(kLDAPOperationCID, &rv);
    if (NS_FAILED(rv)) {
        return NS_ERROR_FAILURE;
    }

    rv = operation->Init(conn, this, nsnull);
    if (NS_FAILED(rv)) {
        return NS_ERROR_UNEXPECTED; // this should never happen
    }

    // Start a bind operation 
    //
    // Here we need to support the password, see bug #75990
    // 
    rv = operation->SimpleBind(password);
    if (NS_FAILED(rv)) {
        switch (rv) {
        // Only pass along errors we are aware of
        //
        case NS_ERROR_LDAP_ENCODING_ERROR:
        case NS_ERROR_FAILURE:
        case NS_ERROR_OUT_OF_MEMORY:
            return rv;

        default:
            return NS_ERROR_UNEXPECTED;
        }
    }

    return NS_OK;
}

/* AString createFilter (in unsigned long aMaxSize, in AString aPattern, in AString aPrefix, in AString aSuffix, in AString aAttr, in AString aValue); */
NS_IMETHODIMP nsLDAPService::CreateFilter(PRUint32 aMaxSize, 
                                          const nsACString & aPattern,
                                          const nsACString & aPrefix,
                                          const nsACString & aSuffix,
                                          const nsACString & aAttr,
                                          const nsACString & aValue,
                                          nsACString & _retval)
{
    if (!aMaxSize) {
        return NS_ERROR_INVALID_ARG;
    }

    // prepare to tokenize |value| for %vM ... %vN
    //
    nsReadingIterator<char> iter, iterEnd; // setup the iterators
    aValue.BeginReading(iter);
    aValue.EndReading(iterEnd);

    // figure out how big of an array we're going to need for the tokens,
    // including a trailing NULL, and allocate space for it.
    //
    PRUint32 numTokens = CountTokens(iter, iterEnd); 
    char **valueWords;
    valueWords = NS_STATIC_CAST(char **, 
                                nsMemory::Alloc((numTokens + 1) *
                                                sizeof(char *)));
    if (!valueWords) {
        return NS_ERROR_OUT_OF_MEMORY;
    }

    // build the array of values
    //
    PRUint32 curToken = 0;
    while (iter != iterEnd && curToken < numTokens ) {
        valueWords[curToken] = NextToken(iter, iterEnd);
        if ( !valueWords[curToken] ) {
            NS_FREE_XPCOM_ALLOCATED_POINTER_ARRAY(curToken, valueWords);
            return NS_ERROR_OUT_OF_MEMORY;
        }
        curToken++;
    }
    valueWords[numTokens] = 0;  // end of array signal to LDAP C SDK

    // make buffer to be used for construction 
    //
    char *buffer = NS_STATIC_CAST(char *, 
                                  nsMemory::Alloc(aMaxSize * sizeof(char)));
    if (!buffer) {
        NS_FREE_XPCOM_ALLOCATED_POINTER_ARRAY(numTokens, valueWords);
        return NS_ERROR_OUT_OF_MEMORY;
    }

    // create the filter itself
    //
    nsresult rv;
    int result = ldap_create_filter(buffer, aMaxSize, 
                   NS_CONST_CAST(char *, PromiseFlatCString(aPattern).get()),
                   NS_CONST_CAST(char *, PromiseFlatCString(aPrefix).get()),
                   NS_CONST_CAST(char *, PromiseFlatCString(aSuffix).get()),
                   NS_CONST_CAST(char *, PromiseFlatCString(aAttr).get()),
                   NS_CONST_CAST(char *, PromiseFlatCString(aValue).get()),
                   valueWords);
    switch (result) {
    case LDAP_SUCCESS:
        rv = NS_OK;
        break;

    case LDAP_SIZELIMIT_EXCEEDED:
        PR_LOG(gLDAPLogModule, PR_LOG_DEBUG, 
                   ("nsLDAPService::CreateFilter(): "
                    "filter longer than max size of %d generated", 
                    aMaxSize));
        rv = NS_ERROR_NOT_AVAILABLE;
        break;

    case LDAP_PARAM_ERROR:
        rv = NS_ERROR_INVALID_ARG;
        break;

    default:
        NS_ERROR("nsLDAPService::CreateFilter(): ldap_create_filter() "
                 "returned unexpected error");
        rv = NS_ERROR_UNEXPECTED;
        break;
    }

    _retval.Assign(buffer);

    // done with the array and the buffer
    //
    NS_FREE_XPCOM_ALLOCATED_POINTER_ARRAY(numTokens, valueWords);
    nsMemory::Free(buffer);

    return rv;
}

// Count the number of space-separated tokens between aIter and aIterEnd
//
PRUint32
nsLDAPService::CountTokens(nsReadingIterator<char> aIter,
                           nsReadingIterator<char> aIterEnd)
{
    PRUint32 count(0);

    // keep iterating through the string until we hit the end
    //
    while (aIter != aIterEnd) {
    
        // move past any leading spaces
        //
        while (aIter != aIterEnd &&
               ldap_utf8isspace(NS_CONST_CAST(char *, aIter.get()))){
            ++aIter;
        }

        // move past all chars in this token
        //
        while (aIter != aIterEnd) {

            if (ldap_utf8isspace(NS_CONST_CAST(char *, aIter.get()))) {
                ++count;    // token finished; increment the count
                ++aIter;    // move past the space
                break;
            }

            ++aIter; // move to next char

            // if we've hit the end of this token and the end of this 
            // iterator simultaneous, be sure to bump the count, since we're
            // never gonna hit the IsAsciiSpace where it's normally done.
            //
            if (aIter == aIterEnd) {
                ++count;
            }

        }
    }

    return count;
}

// return the next token in this iterator
//
char*
nsLDAPService::NextToken(nsReadingIterator<char> & aIter,
                         nsReadingIterator<char> & aIterEnd)
{
    // move past any leading whitespace
    //
    while (aIter != aIterEnd &&
           ldap_utf8isspace(NS_CONST_CAST(char *, aIter.get()))) {
        ++aIter;
    }

    nsACString::const_iterator start(aIter);

    // copy the token into our local variable
    //
    while (aIter != aIterEnd &&
           !ldap_utf8isspace(NS_CONST_CAST(char *, aIter.get()))) {
        ++aIter;
    }

    return ToNewCString(Substring(start, aIter));
}
