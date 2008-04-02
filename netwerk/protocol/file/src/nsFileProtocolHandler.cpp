/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
// vim:ts=4 sw=4 sts=4 et cin:
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
 *   Rich Walsh <dragtext@e-vertise.com>
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

#include "nsFileProtocolHandler.h"
#include "nsFileChannel.h"
#include "nsInputStreamChannel.h"
#include "nsStandardURL.h"
#include "nsURLHelper.h"
#include "nsNetCID.h"

#include "nsIServiceManager.h"
#include "nsIURL.h"

#include "nsNetUtil.h"

// URL file handling, copied and modified from xpfe/components/bookmarks/src/nsBookmarksService.cpp
#ifdef XP_WIN
#include <shlobj.h>
#include <intshcut.h>
#include "nsIFileURL.h"
#ifdef CompareString
#undef CompareString
#endif
#endif

// URL file handling for OS/2
#ifdef XP_OS2
#include "prio.h"
#include "nsIFileURL.h"
#include "nsILocalFileOS2.h"
#endif

// URL file handling for freedesktop.org
#ifdef XP_UNIX
#include "nsINIParser.h"
#define DESKTOP_ENTRY_SECTION "Desktop Entry"
#endif

//-----------------------------------------------------------------------------

nsFileProtocolHandler::nsFileProtocolHandler()
{
}

nsresult
nsFileProtocolHandler::Init()
{
    return NS_OK;
}

NS_IMPL_THREADSAFE_ISUPPORTS3(nsFileProtocolHandler,
                              nsIFileProtocolHandler,
                              nsIProtocolHandler,
                              nsISupportsWeakReference)

//-----------------------------------------------------------------------------
// nsIProtocolHandler methods:

#if defined(XP_WIN)
NS_IMETHODIMP
nsFileProtocolHandler::ReadURLFile(nsIFile* aFile, nsIURI** aURI)
{
// IUniformResourceLocator isn't supported by VC5 (bless its little heart)
#if _MSC_VER < 1200 || defined (WINCE)
    return NS_ERROR_NOT_AVAILABLE;
#else
    nsAutoString path;
    nsresult rv = aFile->GetPath(path);
    if (NS_FAILED(rv))
        return rv;

    if (path.Length() < 4)
        return NS_ERROR_NOT_AVAILABLE;
    if (!StringTail(path, 4).LowerCaseEqualsLiteral(".url"))
        return NS_ERROR_NOT_AVAILABLE;

    HRESULT result;

    rv = NS_ERROR_NOT_AVAILABLE;

    IUniformResourceLocator* urlLink = nsnull;
    result = ::CoCreateInstance(CLSID_InternetShortcut, NULL, CLSCTX_INPROC_SERVER,
                                IID_IUniformResourceLocator, (void**)&urlLink);
    if (SUCCEEDED(result) && urlLink) {
        IPersistFile* urlFile = nsnull;
        result = urlLink->QueryInterface(IID_IPersistFile, (void**)&urlFile);
        if (SUCCEEDED(result) && urlFile) {
            result = urlFile->Load(path.get(), STGM_READ);
            if (SUCCEEDED(result) ) {
                LPSTR lpTemp = nsnull;

                // The URL this method will give us back seems to be already
                // escaped. Hence, do not do escaping of our own.
                result = urlLink->GetURL(&lpTemp);
                if (SUCCEEDED(result) && lpTemp) {
                    rv = NS_NewURI(aURI, lpTemp);

                    // free the string that GetURL alloc'd
                    CoTaskMemFree(lpTemp);
                }
            }
            urlFile->Release();
        }
        urlLink->Release();
    }
    return rv;

#endif //_MSC_VER < 1200 || defined (WINCE)
}

#elif defined(XP_OS2)
NS_IMETHODIMP
nsFileProtocolHandler::ReadURLFile(nsIFile* aFile, nsIURI** aURI)
{
    nsresult rv;

    nsCOMPtr<nsILocalFileOS2> os2File (do_QueryInterface(aFile, &rv));
    if (NS_FAILED(rv))
        return NS_ERROR_NOT_AVAILABLE;

    // see if this file is a WPS UrlObject
    PRBool isUrl;
    rv = os2File->IsFileType(NS_LITERAL_CSTRING("UniformResourceLocator"),
                             &isUrl);
    if (NS_FAILED(rv) || !isUrl)
        return NS_ERROR_NOT_AVAILABLE;

    // if so, open it & get its size
    PRFileDesc *file;
    rv = os2File->OpenNSPRFileDesc(PR_RDONLY, 0, &file);
    if (NS_FAILED(rv))
        return NS_ERROR_NOT_AVAILABLE;

    PRInt64 fileSize;
    os2File->GetFileSize(&fileSize);
    rv = NS_ERROR_NOT_AVAILABLE;

    // get a buffer, read the entire file, then create
    // an nsURI;  we assume the string is already escaped
    char * buffer = (char*)NS_Alloc(fileSize+1);
    if (buffer) {
        PRInt32 cnt = PR_Read(file, buffer, fileSize);
        if (cnt > 0) {
            buffer[cnt] = '\0';
            if (NS_SUCCEEDED(NS_NewURI(aURI, nsDependentCString(buffer))))
                rv = NS_OK;
        }
        NS_Free(buffer);
    }
    PR_Close(file);

    return rv;
}

#elif defined(XP_UNIX)
NS_IMETHODIMP
nsFileProtocolHandler::ReadURLFile(nsIFile* aFile, nsIURI** aURI)
{
    // We only support desktop files that end in ".desktop" like the spec says:
    // http://standards.freedesktop.org/desktop-entry-spec/latest/ar01s02.html
    nsCAutoString leafName;
    nsresult rv = aFile->GetNativeLeafName(leafName);
    if (NS_FAILED(rv) ||
	!StringEndsWith(leafName, NS_LITERAL_CSTRING(".desktop")))
        return NS_ERROR_NOT_AVAILABLE;

    nsCOMPtr<nsILocalFile> file(do_QueryInterface(aFile, &rv));
    if (NS_FAILED(rv))
        return rv;

    nsINIParser parser;
    rv = parser.Init(file);
    if (NS_FAILED(rv))
        return rv;

    nsCAutoString type;
    parser.GetString(DESKTOP_ENTRY_SECTION, "Type", type);
    if (!type.EqualsLiteral("Link"))
        return NS_ERROR_NOT_AVAILABLE;

    nsCAutoString url;
    rv = parser.GetString(DESKTOP_ENTRY_SECTION, "URL", url);
    if (NS_FAILED(rv) || url.IsEmpty())
        return NS_ERROR_NOT_AVAILABLE;

    return NS_NewURI(aURI, url);
}

#else // other platforms
NS_IMETHODIMP
nsFileProtocolHandler::ReadURLFile(nsIFile* aFile, nsIURI** aURI)
{
    return NS_ERROR_NOT_AVAILABLE;
}
#endif // ReadURLFile()

NS_IMETHODIMP
nsFileProtocolHandler::GetScheme(nsACString &result)
{
    result.AssignLiteral("file");
    return NS_OK;
}

NS_IMETHODIMP
nsFileProtocolHandler::GetDefaultPort(PRInt32 *result)
{
    *result = -1;        // no port for file: URLs
    return NS_OK;
}

NS_IMETHODIMP
nsFileProtocolHandler::GetProtocolFlags(PRUint32 *result)
{
    *result = URI_NOAUTH | URI_IS_LOCAL_FILE;
    return NS_OK;
}

NS_IMETHODIMP
nsFileProtocolHandler::NewURI(const nsACString &spec,
                              const char *charset,
                              nsIURI *baseURI,
                              nsIURI **result)
{
    nsCOMPtr<nsIStandardURL> url = new nsStandardURL(PR_TRUE);
    if (!url)
        return NS_ERROR_OUT_OF_MEMORY;

    const nsACString *specPtr = &spec;

#if defined(XP_WIN) || defined(XP_OS2)
    nsCAutoString buf;
    if (net_NormalizeFileURL(spec, buf))
        specPtr = &buf;
#endif

    nsresult rv = url->Init(nsIStandardURL::URLTYPE_NO_AUTHORITY, -1,
                            *specPtr, charset, baseURI);
    if (NS_FAILED(rv)) return rv;

    return CallQueryInterface(url, result);
}

NS_IMETHODIMP
nsFileProtocolHandler::NewChannel(nsIURI *uri, nsIChannel **result)
{
    nsresult rv;

    // This file may be a url file
    nsCOMPtr<nsIFileURL> url(do_QueryInterface(uri));
    if (url) {
        nsCOMPtr<nsIFile> file;
        rv = url->GetFile(getter_AddRefs(file));
        if (NS_SUCCEEDED(rv)) {
            nsCOMPtr<nsIURI> uri;
            rv = ReadURLFile(file, getter_AddRefs(uri));
            if (NS_SUCCEEDED(rv)) {
                rv = NS_NewChannel(result, uri);
                if (NS_SUCCEEDED(rv))
                    return rv;
            }
        }
    }

    nsFileChannel *chan = new nsFileChannel(uri);
    if (!chan)
        return NS_ERROR_OUT_OF_MEMORY;
    NS_ADDREF(chan);

    rv = chan->Init();
    if (NS_FAILED(rv)) {
        NS_RELEASE(chan);
        return rv;
    }

    *result = chan;
    return NS_OK;
}

NS_IMETHODIMP 
nsFileProtocolHandler::AllowPort(PRInt32 port, const char *scheme, PRBool *result)
{
    // don't override anything.  
    *result = PR_FALSE;
    return NS_OK;
}

//-----------------------------------------------------------------------------
// nsIFileProtocolHandler methods:

NS_IMETHODIMP
nsFileProtocolHandler::NewFileURI(nsIFile *file, nsIURI **result)
{
    NS_ENSURE_ARG_POINTER(file);
    nsresult rv;

    nsCOMPtr<nsIFileURL> url = new nsStandardURL(PR_TRUE);
    if (!url)
        return NS_ERROR_OUT_OF_MEMORY;

    // NOTE: the origin charset is assigned the value of the platform
    // charset by the SetFile method.
    rv = url->SetFile(file);
    if (NS_FAILED(rv)) return rv;

    return CallQueryInterface(url, result);
}

NS_IMETHODIMP
nsFileProtocolHandler::GetURLSpecFromFile(nsIFile *file, nsACString &result)
{
    NS_ENSURE_ARG_POINTER(file);
    return net_GetURLSpecFromFile(file, result);
}

NS_IMETHODIMP
nsFileProtocolHandler::GetFileFromURLSpec(const nsACString &spec, nsIFile **result)
{
    return net_GetFileFromURLSpec(spec, result);
}
