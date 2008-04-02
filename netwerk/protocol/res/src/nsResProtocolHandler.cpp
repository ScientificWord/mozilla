/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
 *   Darin Fisher <darin@netscape.com>
 *   Benjamin Smedberg <bsmedberg@covad.net>
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

#include "nsResProtocolHandler.h"
#include "nsAutoLock.h"
#include "nsIURL.h"
#include "nsIIOService.h"
#include "nsIServiceManager.h"
#include "nsILocalFile.h"
#include "prenv.h"
#include "prmem.h"
#include "prprf.h"
#include "nsXPIDLString.h"
#include "nsIFile.h"
#include "nsDirectoryServiceDefs.h"
#include "nsNetUtil.h"
#include "nsURLHelper.h"
#include "nsEscape.h"

static NS_DEFINE_CID(kResURLCID, NS_RESURL_CID);

static nsResProtocolHandler *gResHandler = nsnull;

#if defined(PR_LOGGING)
//
// Log module for Resource Protocol logging...
//
// To enable logging (see prlog.h for full details):
//
//    set NSPR_LOG_MODULES=nsResProtocol:5
//    set NSPR_LOG_FILE=log.txt
//
// this enables PR_LOG_ALWAYS level information and places all output in
// the file log.txt
//
static PRLogModuleInfo *gResLog;
#endif
#define LOG(args) PR_LOG(gResLog, PR_LOG_DEBUG, args)

//----------------------------------------------------------------------------
// nsResURL : overrides nsStandardURL::GetFile to provide nsIFile resolution
//----------------------------------------------------------------------------

nsresult
nsResURL::EnsureFile()
{
    nsresult rv;

    NS_ENSURE_TRUE(gResHandler, NS_ERROR_NOT_AVAILABLE);

    nsCAutoString spec;
    rv = gResHandler->ResolveURI(this, spec);
    if (NS_FAILED(rv)) return rv;

    rv = net_GetFileFromURLSpec(spec, getter_AddRefs(mFile));
#ifdef DEBUG_bsmedberg
    if (NS_SUCCEEDED(rv)) {
        PRBool exists = PR_TRUE;
        mFile->Exists(&exists);
        if (!exists) {
            printf("resource %s doesn't exist!\n", spec.get());
        }
    }
#endif

    return rv;
}

/* virtual */ nsStandardURL*
nsResURL::StartClone()
{
    nsResURL *clone;
    NS_NEWXPCOM(clone, nsResURL);
    return clone;
}

NS_IMETHODIMP 
nsResURL::GetClassIDNoAlloc(nsCID *aClassIDNoAlloc)
{
    *aClassIDNoAlloc = kResURLCID;
    return NS_OK;
}

//----------------------------------------------------------------------------
// nsResProtocolHandler <public>
//----------------------------------------------------------------------------

nsResProtocolHandler::nsResProtocolHandler()
{
#if defined(PR_LOGGING)
    gResLog = PR_NewLogModule("nsResProtocol");
#endif

    NS_ASSERTION(!gResHandler, "res handler already created!");
    gResHandler = this;
}

nsResProtocolHandler::~nsResProtocolHandler()
{
    gResHandler = nsnull;
}

nsresult
nsResProtocolHandler::AddSpecialDir(const char* aSpecialDir, const nsACString& aSubstitution)
{
    nsCOMPtr<nsIFile> file;
    nsresult rv = NS_GetSpecialDirectory(aSpecialDir, getter_AddRefs(file));
    NS_ENSURE_SUCCESS(rv, rv);

    nsCOMPtr<nsIURI> uri;
    rv = mIOService->NewFileURI(file, getter_AddRefs(uri));
    NS_ENSURE_SUCCESS(rv, rv);

    return SetSubstitution(aSubstitution, uri);
}

nsresult
nsResProtocolHandler::Init()
{
    if (!mSubstitutions.Init(32))
        return NS_ERROR_UNEXPECTED;

    nsresult rv;

    mIOService = do_GetIOService(&rv);
    NS_ENSURE_SUCCESS(rv, rv);

    //
    // make resource:/// point to the application directory
    //
    rv = AddSpecialDir(NS_OS_CURRENT_PROCESS_DIR, EmptyCString());
    NS_ENSURE_SUCCESS(rv, rv);

    //
    // make resource://gre/ point to the GRE directory
    //
    rv = AddSpecialDir(NS_GRE_DIR, NS_LITERAL_CSTRING("gre"));
    NS_ENSURE_SUCCESS(rv, rv);

    //XXXbsmedberg Neil wants a resource://pchrome/ for the profile chrome dir...
    // but once I finish multiple chrome registration I'm not sure that it is needed

    return rv;
}

//----------------------------------------------------------------------------
// nsResProtocolHandler::nsISupports
//----------------------------------------------------------------------------

NS_IMPL_THREADSAFE_ISUPPORTS3(nsResProtocolHandler,
                              nsIResProtocolHandler,
                              nsIProtocolHandler,
                              nsISupportsWeakReference)

//----------------------------------------------------------------------------
// nsResProtocolHandler::nsIProtocolHandler
//----------------------------------------------------------------------------

NS_IMETHODIMP
nsResProtocolHandler::GetScheme(nsACString &result)
{
    result.AssignLiteral("resource");
    return NS_OK;
}

NS_IMETHODIMP
nsResProtocolHandler::GetDefaultPort(PRInt32 *result)
{
    *result = -1;        // no port for res: URLs
    return NS_OK;
}

NS_IMETHODIMP
nsResProtocolHandler::GetProtocolFlags(PRUint32 *result)
{
    // XXXbz Is this really true for all resource: URIs?  Could we
    // somehow give different flags to some of them?
    *result = URI_STD | URI_IS_UI_RESOURCE;
    return NS_OK;
}

NS_IMETHODIMP
nsResProtocolHandler::NewURI(const nsACString &aSpec,
                             const char *aCharset,
                             nsIURI *aBaseURI,
                             nsIURI **result)
{
    nsresult rv;

    nsResURL *resURL;
    NS_NEWXPCOM(resURL, nsResURL);
    if (!resURL)
        return NS_ERROR_OUT_OF_MEMORY;
    NS_ADDREF(resURL);

    rv = resURL->Init(nsIStandardURL::URLTYPE_STANDARD, -1, aSpec, aCharset, aBaseURI);
    if (NS_SUCCEEDED(rv))
        rv = CallQueryInterface(resURL, result);
    NS_RELEASE(resURL);
    return rv;
}

NS_IMETHODIMP
nsResProtocolHandler::NewChannel(nsIURI* uri, nsIChannel* *result)
{
    NS_ENSURE_ARG_POINTER(uri);
    nsresult rv;
    nsCAutoString spec;

    rv = ResolveURI(uri, spec);
    if (NS_FAILED(rv)) return rv;

    rv = mIOService->NewChannel(spec, nsnull, nsnull, result);
    if (NS_FAILED(rv)) return rv;

    return (*result)->SetOriginalURI(uri);
}

NS_IMETHODIMP 
nsResProtocolHandler::AllowPort(PRInt32 port, const char *scheme, PRBool *_retval)
{
    // don't override anything.  
    *_retval = PR_FALSE;
    return NS_OK;
}

//----------------------------------------------------------------------------
// nsResProtocolHandler::nsIResProtocolHandler
//----------------------------------------------------------------------------

NS_IMETHODIMP
nsResProtocolHandler::SetSubstitution(const nsACString& root, nsIURI *baseURI)
{
    if (!baseURI) {
        mSubstitutions.Remove(root);
        return NS_OK;
    }

    return mSubstitutions.Put(root, baseURI) ? NS_OK : NS_ERROR_UNEXPECTED;
}

NS_IMETHODIMP
nsResProtocolHandler::GetSubstitution(const nsACString& root, nsIURI **result)
{
    NS_ENSURE_ARG_POINTER(result);

    if (mSubstitutions.Get(root, result))
        return NS_OK;

    // try invoking the directory service for "resource:root"

    nsCAutoString key;
    key.AssignLiteral("resource:");
    key.Append(root);

    nsCOMPtr<nsIFile> file;
    nsresult rv = NS_GetSpecialDirectory(key.get(), getter_AddRefs(file));
    if (NS_FAILED(rv))
        return NS_ERROR_NOT_AVAILABLE;
        
    rv = mIOService->NewFileURI(file, result);
    if (NS_FAILED(rv))
        return NS_ERROR_NOT_AVAILABLE;

    return NS_OK;
}

NS_IMETHODIMP
nsResProtocolHandler::HasSubstitution(const nsACString& root, PRBool *result)
{
    NS_ENSURE_ARG_POINTER(result);

    *result = mSubstitutions.Get(root, nsnull);
    return NS_OK;
}

NS_IMETHODIMP
nsResProtocolHandler::ResolveURI(nsIURI *uri, nsACString &result)
{
    nsresult rv;

    nsCOMPtr<nsIURL> url(do_QueryInterface(uri));
    if (!url)
        return NS_NOINTERFACE;

    nsCAutoString host;
    nsCAutoString path;

    rv = uri->GetAsciiHost(host);
    if (NS_FAILED(rv)) return rv;

    rv = uri->GetPath(path);
    if (NS_FAILED(rv)) return rv;

    nsCAutoString filepath;
    url->GetFilePath(filepath);

    // Don't misinterpret the filepath as an absolute URI.
    if (filepath.FindChar(':') != -1)
        return NS_ERROR_MALFORMED_URI;

    NS_UnescapeURL(filepath);
    if (filepath.FindChar('\\') != -1)
        return NS_ERROR_MALFORMED_URI;

    const char *p = path.get() + 1; // path always starts with a slash
    NS_ASSERTION(*(p-1) == '/', "Path did not begin with a slash!");

    if (*p == '/')
        return NS_ERROR_MALFORMED_URI;

    nsCOMPtr<nsIURI> baseURI;
    rv = GetSubstitution(host, getter_AddRefs(baseURI));
    if (NS_FAILED(rv)) return rv;

    rv = baseURI->Resolve(nsDependentCString(p, path.Length()-1), result);

#if defined(PR_LOGGING)
    if (PR_LOG_TEST(gResLog, PR_LOG_DEBUG)) {
        nsCAutoString spec;
        uri->GetAsciiSpec(spec);
        LOG(("%s\n -> %s\n", spec.get(), PromiseFlatCString(result).get()));
    }
#endif
    return rv;
}
