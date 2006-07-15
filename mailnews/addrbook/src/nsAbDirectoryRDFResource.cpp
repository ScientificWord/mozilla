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
 * Sun Microsystems, Inc.
 * Portions created by the Initial Developer are Copyright (C) 2001
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Paul Sandoz <paul.sandoz@sun.com>
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

#include "nsAbDirectoryRDFResource.h"
#include "nsIURL.h"
#include "nsNetCID.h"
#include "nsIServiceManager.h"

nsAbDirectoryRDFResource::nsAbDirectoryRDFResource () :
    nsRDFResource (),
    mIsValidURI (PR_FALSE),
    mIsQueryURI (PR_FALSE)
{
}

nsAbDirectoryRDFResource::~nsAbDirectoryRDFResource ()
{
}

NS_IMPL_ISUPPORTS_INHERITED0(nsAbDirectoryRDFResource, nsRDFResource)

NS_IMETHODIMP nsAbDirectoryRDFResource::Init(const char* aURI)
{
  nsresult rv = nsRDFResource::Init (aURI);
    NS_ENSURE_SUCCESS(rv, rv);

    mURINoQuery = aURI;

    nsCOMPtr<nsIURI> uri = do_CreateInstance (NS_STANDARDURL_CONTRACTID, &rv);
    NS_ENSURE_SUCCESS(rv, rv);

    rv = uri->SetSpec(nsDependentCString(aURI));
    NS_ENSURE_SUCCESS(rv, rv);

    mIsValidURI = PR_TRUE;

  nsCOMPtr<nsIURL> url = do_QueryInterface(uri, &rv);
    NS_ENSURE_SUCCESS(rv, rv);

    nsCAutoString queryString;
    rv = url->GetQuery (queryString);
  NS_ENSURE_SUCCESS(rv, rv);

    nsCAutoString path;
    rv = url->GetPath (path);
  NS_ENSURE_SUCCESS(rv, rv);

    mPath = path;

  if (!queryString.IsEmpty())
    {
    mPath.Truncate(path.Length() - queryString.Length() - 1);

    mURINoQuery.Truncate(mURINoQuery.Length() - queryString.Length() - 1);

        mQueryString = queryString;

        mIsQueryURI = PR_TRUE;
    }
    else 
      mIsQueryURI = PR_FALSE;

    return rv;
}
