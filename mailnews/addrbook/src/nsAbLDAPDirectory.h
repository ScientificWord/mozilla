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
 * Paul Sandoz <paul.sandoz@sun.com> Sun Microsystems, Inc.
 * Portions created by the Initial Developer are Copyright (C) 2001
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Seth Spitzer <sspitzer@netscape.com>
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

#ifndef nsAbLDAPDirectory_h__
#define nsAbLDAPDirectory_h__

#include "nsAbDirectoryRDFResource.h"
#include "nsAbDirProperty.h"
#include "nsAbLDAPDirectoryQuery.h"
#include "nsIAbDirectorySearch.h"
#include "nsAbDirSearchListener.h"
#include "nsIAbLDAPDirectory.h"
#include "nsIMutableArray.h"
#include "nsHashtable.h"

class nsAbLDAPDirectory :
    public nsAbDirectoryRDFResource,    // nsIRDFResource
    public nsAbDirProperty,            // nsIAbDirectory
    public nsAbLDAPDirectoryQuery,        // nsIAbDirectoryQuery
    public nsIAbDirectorySearch,
    public nsAbDirSearchListenerContext,
    public nsIAbLDAPDirectory
{
public:
    NS_DECL_ISUPPORTS_INHERITED

    nsAbLDAPDirectory();
    virtual ~nsAbLDAPDirectory();

    NS_IMETHOD Init(const char *aUri);

  // nsIAbDirectory methods
  NS_IMETHOD GetURI(nsACString &aURI);
    NS_IMETHOD GetOperations(PRInt32 *aOperations);
    NS_IMETHOD GetChildNodes(nsISimpleEnumerator* *result);
    NS_IMETHOD GetChildCards(nsISimpleEnumerator* *result);
     NS_IMETHOD HasCard(nsIAbCard *cards, PRBool *hasCard);
    NS_IMETHOD GetSupportsMailingLists(PRBool *aSupportsMailingsLists);
    NS_IMETHOD GetIsRemote(PRBool *aIsRemote);
    NS_IMETHOD GetIsSecure(PRBool *aIsRemote);
    NS_IMETHOD GetSearchDuringLocalAutocomplete(PRBool *aSearchDuringLocalAutocomplete);

    // nsAbLDAPDirectoryQuery methods
    nsresult GetLDAPConnection (nsILDAPConnection** connection);
    nsresult GetLDAPURL (nsILDAPURL** url);
    nsresult CreateCard (nsILDAPURL* uri, const char* dn, nsIAbCard** card);

    // nsIAbDirectorySearch methods
    NS_DECL_NSIABDIRECTORYSEARCH

    // nsAbDirSearchListenerContext methods
    nsresult OnSearchFinished (PRInt32 result);
    nsresult OnSearchFoundCard (nsIAbCard* card);

    NS_DECL_NSIABLDAPDIRECTORY

protected:

    // historically, these classes weren't always friends, but since they
    // are now (and should be), code and interface contortions that are used
    // can probably go away.
    friend class nsAbLDAPDirectoryQuery;
    friend class nsAbQueryLDAPMessageListener;

    nsresult Initiate ();
    nsresult InitiateConnection ();

    PRPackedBool mInitialized;
    PRPackedBool mInitializedConnection;
    PRPackedBool mPerformingQuery;
    PRInt32 mContext;
    PRInt32 mMaxHits;
    nsCOMPtr<nsILDAPURL> mURL ;
    nsCOMPtr<nsILDAPConnection> mConnection;

    nsCOMPtr<nsIAbBooleanExpression> mExpression;
    nsSupportsHashtable mCache;

    PRLock* mLock;

    nsCOMPtr<nsIMutableArray> mSearchServerControls;
    nsCOMPtr<nsIMutableArray> mSearchClientControls;
};

#endif
