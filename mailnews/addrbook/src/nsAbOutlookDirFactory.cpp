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
 *   Created by Cyrille Moureaux <Cyrille.Moureaux@sun.com>
 *   Mark Banner <mark@standard8.demon.co.uk>
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
#include "nsAbOutlookDirFactory.h"
#include "nsAbWinHelper.h"
#include "nsIAbDirectory.h"

#include "nsIRDFService.h"
#include "nsIRDFResource.h"
#include "nsRDFResource.h"
#include "nsEnumeratorUtils.h"
#include "nsServiceManagerUtils.h"

#include "nsAbBaseCID.h"

#include "prlog.h"

#ifdef PR_LOGGING
static PRLogModuleInfo* gAbOutlookDirFactoryLog
    = PR_NewLogModule("nsAbOutlookDirFactoryLog");
#endif

#define PRINTF(args) PR_LOG(nsAbOutlookDirFactoryLog, PR_LOG_DEBUG, args)


// In case someone is wondering WHY I have to undefine CreateDirectory,
// it's because the windows files winbase.h and wininet.h define this
// to CreateDirectoryA/W (for reasons best left unknown) and with the
// MAPI stuff, I end up including this, which wreaks havoc on my symbol
// table.
#ifdef CreateDirectory 
#  undef CreateDirectory
#endif // CreateDirectory

NS_IMPL_ISUPPORTS1(nsAbOutlookDirFactory, nsIAbDirFactory)

nsAbOutlookDirFactory::nsAbOutlookDirFactory(void)
{
}

nsAbOutlookDirFactory::~nsAbOutlookDirFactory(void)
{
}

extern const char *kOutlookDirectoryScheme ;

static nsresult parseProperties(nsIAbDirectoryProperties *aProperties, nsAbWinType& aWinType)
{
    aWinType = nsAbWinType_Unknown ;

    nsXPIDLCString uri;
    nsresult rv = aProperties->GetURI(getter_Copies(uri));
    NS_ENSURE_SUCCESS(rv,rv);
    
            nsCString stub ;
            nsCString entry ;

            aWinType = getAbWinType(kOutlookDirectoryScheme, uri.get(), stub, entry) ;
    return NS_OK;
}

NS_IMETHODIMP nsAbOutlookDirFactory::CreateDirectory(nsIAbDirectoryProperties *aProperties, 
                                                     nsISimpleEnumerator **aDirectories)
{
    NS_ENSURE_ARG_POINTER(aProperties);
    NS_ENSURE_ARG_POINTER(aDirectories);

    *aDirectories = nsnull ;
    nsresult retCode = NS_OK ;
    nsAbWinType abType = nsAbWinType_Unknown ;

    retCode = parseProperties(aProperties, abType) ;
    NS_ENSURE_SUCCESS(retCode, retCode);

    if (abType == nsAbWinType_Unknown) {
        return NS_ERROR_FAILURE ;
    }
    nsAbWinHelperGuard mapiAddBook (abType) ;
    nsMapiEntryArray folders ;
    ULONG nbFolders = 0 ;
    nsCOMPtr<nsISupportsArray> directories ;
    
    retCode = NS_NewISupportsArray(getter_AddRefs(directories)) ;
    NS_ENSURE_SUCCESS(retCode, retCode) ;
    if (!mapiAddBook->IsOK() || !mapiAddBook->GetFolders(folders)) {
        return NS_ERROR_FAILURE ;
    }
    nsCOMPtr<nsIRDFService> rdf = do_GetService (NS_RDF_CONTRACTID "/rdf-service;1", &retCode);
    NS_ENSURE_SUCCESS(retCode, retCode) ;
    nsCAutoString entryId ;
    nsCAutoString uri ;
    nsCOMPtr<nsIRDFResource> resource ;

    for (ULONG i = 0 ; i < folders.mNbEntries ; ++ i) {
        folders.mEntries [i].ToString(entryId) ;
        buildAbWinUri(kOutlookDirectoryScheme, abType, uri) ;
        uri.Append(entryId) ;
        
        retCode = rdf->GetResource(uri, getter_AddRefs(resource)) ;
        NS_ENSURE_SUCCESS(retCode, retCode) ;
        directories->AppendElement(resource) ;
    }
    return NS_NewArrayEnumerator(aDirectories, directories) ;
}

// No actual deletion, since you cannot create the address books from Mozilla.
NS_IMETHODIMP nsAbOutlookDirFactory::DeleteDirectory(nsIAbDirectory *aDirectory)
{
    return NS_OK ;
}

