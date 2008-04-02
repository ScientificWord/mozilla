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
 *   Benjamin Smedberg <benjamin@smedbergs.us>
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

/*
  Implementation for a file system RDF data store.
 */

#include "nsFileSystemDataSource.h"

#include <ctype.h> // for toupper()
#include <stdio.h>
#include "nsIEnumerator.h"
#include "nsIRDFDataSource.h"
#include "nsIRDFObserver.h"
#include "nsIServiceManager.h"
#include "nsVoidArray.h"
#include "nsXPIDLString.h"
#include "nsRDFCID.h"
#include "rdfutil.h"
#include "plhash.h"
#include "plstr.h"
#include "prlong.h"
#include "prlog.h"
#include "prmem.h"
#include "prprf.h"
#include "prio.h"
#include "rdf.h"
#include "nsEnumeratorUtils.h"
#include "nsIURL.h"
#include "nsIFileURL.h"
#include "nsNetUtil.h"
#include "nsIChannel.h"
#include "nsIFile.h"
#include "nsEscape.h"
#include "nsCRTGlue.h"
#include "nsAutoPtr.h"

#ifdef  XP_WIN
#include "windef.h"
#include "winbase.h"
#include "nsILineInputStream.h"
#endif

#ifdef  XP_BEOS
#include <File.h>
#include <NodeInfo.h>
#endif

#if defined(XP_WIN) || defined(XP_BEOS)
#include "nsDirectoryServiceDefs.h"
#endif

#ifdef XP_OS2
#define INCL_DOSFILEMGR
#include <os2.h>
#endif

#define NS_MOZICON_SCHEME           "moz-icon:"

static const char kFileProtocol[]         = "file://";

PRBool
FileSystemDataSource::isFileURI(nsIRDFResource *r)
{
    PRBool      isFileURIFlag = PR_FALSE;
    const char  *uri = nsnull;
    
    r->GetValueConst(&uri);
    if ((uri) && (!strncmp(uri, kFileProtocol, sizeof(kFileProtocol) - 1)))
    {
        // XXX HACK HACK HACK
        if (!strchr(uri, '#'))
        {
            isFileURIFlag = PR_TRUE;
        }
    }
    return(isFileURIFlag);
}



PRBool
FileSystemDataSource::isDirURI(nsIRDFResource* source)
{
    nsresult    rv;
    const char  *uri = nsnull;

    rv = source->GetValueConst(&uri);
    if (NS_FAILED(rv)) return(PR_FALSE);

    nsCOMPtr<nsIFile> aDir;

    rv = NS_GetFileFromURLSpec(nsDependentCString(uri), getter_AddRefs(aDir));
    if (NS_FAILED(rv)) return(PR_FALSE);

    PRBool isDirFlag = PR_FALSE;

    rv = aDir->IsDirectory(&isDirFlag);
    if (NS_FAILED(rv)) return(PR_FALSE);

#ifdef  XP_MAC
    // Hide directory structure of packages under Mac OS 9/X
    nsCOMPtr<nsILocalFileMac>   aMacFile = do_QueryInterface(aDir);
    if (aMacFile)
    {
        PRBool isPackageFlag = PR_FALSE;
        rv = aMacFile->IsPackage(&isPackageFlag);
        if (NS_SUCCEEDED(rv) && (isPackageFlag == PR_TRUE))
        {
            isDirFlag = PR_FALSE;
        }
    }
#endif

    return(isDirFlag);
}


nsresult
FileSystemDataSource::Init()
{
    nsresult rv;

    mRDFService = do_GetService("@mozilla.org/rdf/rdf-service;1");
    NS_ENSURE_TRUE(mRDFService, NS_ERROR_FAILURE);

    rv =  mRDFService->GetResource(NS_LITERAL_CSTRING("NC:FilesRoot"),
                                   getter_AddRefs(mNC_FileSystemRoot));
    rv |= mRDFService->GetResource(NS_LITERAL_CSTRING(NC_NAMESPACE_URI  "child"),
                                   getter_AddRefs(mNC_Child));
    rv |= mRDFService->GetResource(NS_LITERAL_CSTRING(NC_NAMESPACE_URI  "Name"),
                                   getter_AddRefs(mNC_Name));
    rv |= mRDFService->GetResource(NS_LITERAL_CSTRING(NC_NAMESPACE_URI  "URL"),
                                   getter_AddRefs(mNC_URL));
    rv |= mRDFService->GetResource(NS_LITERAL_CSTRING(NC_NAMESPACE_URI  "Icon"),
                                   getter_AddRefs(mNC_Icon));
    rv |= mRDFService->GetResource(NS_LITERAL_CSTRING(NC_NAMESPACE_URI  "Content-Length"),
                                   getter_AddRefs(mNC_Length));
    rv |= mRDFService->GetResource(NS_LITERAL_CSTRING(NC_NAMESPACE_URI  "IsDirectory"),
                                   getter_AddRefs(mNC_IsDirectory));
    rv |= mRDFService->GetResource(NS_LITERAL_CSTRING(WEB_NAMESPACE_URI "LastModifiedDate"),
                                   getter_AddRefs(mWEB_LastMod));
    rv |= mRDFService->GetResource(NS_LITERAL_CSTRING(NC_NAMESPACE_URI  "FileSystemObject"),
                                   getter_AddRefs(mNC_FileSystemObject));
    rv |= mRDFService->GetResource(NS_LITERAL_CSTRING(NC_NAMESPACE_URI  "pulse"),
                                   getter_AddRefs(mNC_pulse));
    rv |= mRDFService->GetResource(NS_LITERAL_CSTRING(RDF_NAMESPACE_URI "instanceOf"),
                                   getter_AddRefs(mRDF_InstanceOf));
    rv |= mRDFService->GetResource(NS_LITERAL_CSTRING(RDF_NAMESPACE_URI "type"),
                                   getter_AddRefs(mRDF_type));

    static const PRUnichar kTrue[] = {'t','r','u','e','\0'};
    static const PRUnichar kFalse[] = {'f','a','l','s','e','\0'};

    rv |= mRDFService->GetLiteral(kTrue, getter_AddRefs(mLiteralTrue));
    rv |= mRDFService->GetLiteral(kFalse, getter_AddRefs(mLiteralFalse));
    NS_ENSURE_SUCCESS(rv, NS_ERROR_FAILURE);

#ifdef USE_NC_EXTENSION
    rv = mRDFService->GetResource(NS_LITERAL_CSTRING(NC_NAMESPACE_URI "extension"),
                                  getter_AddRefs(mNC_extension));
    NS_ENSURE_SUCCESS(rv, rv);
#endif

#ifdef XP_WIN
    rv =  mRDFService->GetResource(NS_LITERAL_CSTRING(NC_NAMESPACE_URI "IEFavorite"),
                                  getter_AddRefs(mNC_IEFavoriteObject));
    rv |= mRDFService->GetResource(NS_LITERAL_CSTRING(NC_NAMESPACE_URI "IEFavoriteFolder"),
                                   getter_AddRefs(mNC_IEFavoriteFolder));
    NS_ENSURE_SUCCESS(rv, NS_ERROR_FAILURE);

    nsCOMPtr<nsIFile> file;
    NS_GetSpecialDirectory(NS_WIN_FAVORITES_DIR, getter_AddRefs(file));
    if (file)
    {
        nsCOMPtr<nsIURI> furi;
        NS_NewFileURI(getter_AddRefs(furi), file);
        NS_ENSURE_TRUE(furi, NS_ERROR_FAILURE);

        file->GetNativePath(ieFavoritesDir);
    }
#endif

#ifdef XP_BEOS
    nsCOMPtr<nsIFile> file;
    NS_GetSpecialDirectory(NS_BEOS_SETTINGS_DIR, getter_AddRefs(file));
    if (file)
    {
        file->AppendNative(NS_LITERAL_CSTRING("NetPositive"));
        file->AppendNative(NS_LITERAL_CSTRING("Bookmarks"));

        nsCOMPtr<nsIURI> furi;
        NS_NewFileURI(getter_AddRefs(furi), file);
        NS_ENSURE_TRUE(furi, NS_ERROR_FAILURE);

        file->GetNativePath(netPositiveDir);
    }
#endif

    return NS_OK;
}

//static
nsresult
FileSystemDataSource::Create(nsISupports* aOuter, const nsIID& aIID, void **aResult)
{
    NS_ENSURE_NO_AGGREGATION(aOuter);

    nsRefPtr<FileSystemDataSource> self = new FileSystemDataSource();
    if (!self)
        return NS_ERROR_OUT_OF_MEMORY;
     
    nsresult rv = self->Init();
    NS_ENSURE_SUCCESS(rv, rv);

    return self->QueryInterface(aIID, aResult);
}

NS_IMPL_ISUPPORTS1(FileSystemDataSource, nsIRDFDataSource)

NS_IMETHODIMP
FileSystemDataSource::GetURI(char **uri)
{
    NS_PRECONDITION(uri != nsnull, "null ptr");
    if (! uri)
        return NS_ERROR_NULL_POINTER;

    if ((*uri = NS_strdup("rdf:files")) == nsnull)
        return NS_ERROR_OUT_OF_MEMORY;

    return NS_OK;
}



NS_IMETHODIMP
FileSystemDataSource::GetSource(nsIRDFResource* property,
                                nsIRDFNode* target,
                                PRBool tv,
                                nsIRDFResource** source /* out */)
{
    NS_PRECONDITION(property != nsnull, "null ptr");
    if (! property)
        return NS_ERROR_NULL_POINTER;

    NS_PRECONDITION(target != nsnull, "null ptr");
    if (! target)
        return NS_ERROR_NULL_POINTER;

    NS_PRECONDITION(source != nsnull, "null ptr");
    if (! source)
        return NS_ERROR_NULL_POINTER;

    *source = nsnull;
    return NS_RDF_NO_VALUE;
}



NS_IMETHODIMP
FileSystemDataSource::GetSources(nsIRDFResource *property,
                                 nsIRDFNode *target,
                                 PRBool tv,
                                 nsISimpleEnumerator **sources /* out */)
{
//  NS_NOTYETIMPLEMENTED("write me");
    return NS_ERROR_NOT_IMPLEMENTED;
}



NS_IMETHODIMP
FileSystemDataSource::GetTarget(nsIRDFResource *source,
                                nsIRDFResource *property,
                                PRBool tv,
                                nsIRDFNode **target /* out */)
{
    NS_PRECONDITION(source != nsnull, "null ptr");
    if (! source)
        return NS_ERROR_NULL_POINTER;

    NS_PRECONDITION(property != nsnull, "null ptr");
    if (! property)
        return NS_ERROR_NULL_POINTER;

    NS_PRECONDITION(target != nsnull, "null ptr");
    if (! target)
        return NS_ERROR_NULL_POINTER;

    *target = nsnull;

    nsresult        rv = NS_RDF_NO_VALUE;

    // we only have positive assertions in the file system data source.
    if (! tv)
        return NS_RDF_NO_VALUE;

    if (source == mNC_FileSystemRoot)
    {
        if (property == mNC_pulse)
        {
            nsIRDFLiteral   *pulseLiteral;
            mRDFService->GetLiteral(NS_LITERAL_STRING("12").get(), &pulseLiteral);
            *target = pulseLiteral;
            return NS_OK;
        }
    }
    else if (isFileURI(source))
    {
        if (property == mNC_Name)
        {
            nsCOMPtr<nsIRDFLiteral> name;
            rv = GetName(source, getter_AddRefs(name));
            if (NS_FAILED(rv)) return(rv);
            if (!name)  rv = NS_RDF_NO_VALUE;
            if (rv == NS_RDF_NO_VALUE)  return(rv);
            return name->QueryInterface(NS_GET_IID(nsIRDFNode), (void**) target);
        }
        else if (property == mNC_URL)
        {
            nsCOMPtr<nsIRDFLiteral> url;
            rv = GetURL(source, nsnull, getter_AddRefs(url));
            if (NS_FAILED(rv)) return(rv);
            if (!url)   rv = NS_RDF_NO_VALUE;
            if (rv == NS_RDF_NO_VALUE)  return(rv);

            return url->QueryInterface(NS_GET_IID(nsIRDFNode), (void**) target);
        }
        else if (property == mNC_Icon)
        {
            nsCOMPtr<nsIRDFLiteral> url;
            PRBool isFavorite = PR_FALSE;
            rv = GetURL(source, &isFavorite, getter_AddRefs(url));
            if (NS_FAILED(rv)) return(rv);
            if (isFavorite || !url) rv = NS_RDF_NO_VALUE;
            if (rv == NS_RDF_NO_VALUE)  return(rv);
            
            const PRUnichar *uni = nsnull;
            url->GetValueConst(&uni);
            if (!uni)   return(NS_RDF_NO_VALUE);
            nsAutoString    urlStr;
            urlStr.Assign(NS_LITERAL_STRING(NS_MOZICON_SCHEME).get());
            urlStr.Append(uni);

            rv = mRDFService->GetLiteral(urlStr.get(), getter_AddRefs(url));
            if (NS_FAILED(rv) || !url)    return(NS_RDF_NO_VALUE);
            return url->QueryInterface(NS_GET_IID(nsIRDFNode), (void**) target);
        }
        else if (property == mNC_Length)
        {
            nsCOMPtr<nsIRDFInt> fileSize;
            rv = GetFileSize(source, getter_AddRefs(fileSize));
            if (NS_FAILED(rv)) return(rv);
            if (!fileSize)  rv = NS_RDF_NO_VALUE;
            if (rv == NS_RDF_NO_VALUE)  return(rv);

            return fileSize->QueryInterface(NS_GET_IID(nsIRDFNode), (void**) target);
        }
        else  if (property == mNC_IsDirectory)
        {
            *target = (isDirURI(source)) ? mLiteralTrue : mLiteralFalse;
            NS_ADDREF(*target);
            return NS_OK;
        }
        else if (property == mWEB_LastMod)
        {
            nsCOMPtr<nsIRDFDate> lastMod;
            rv = GetLastMod(source, getter_AddRefs(lastMod));
            if (NS_FAILED(rv)) return(rv);
            if (!lastMod)   rv = NS_RDF_NO_VALUE;
            if (rv == NS_RDF_NO_VALUE)  return(rv);

            return lastMod->QueryInterface(NS_GET_IID(nsIRDFNode), (void**) target);
        }
        else if (property == mRDF_type)
        {
            nsCString type;
            rv = mNC_FileSystemObject->GetValueUTF8(type);
            if (NS_FAILED(rv)) return(rv);

#ifdef  XP_WIN
            // under Windows, if its an IE favorite, return that type
            if (!ieFavoritesDir.IsEmpty())
            {
                nsCString uri;
                rv = source->GetValueUTF8(uri);
                if (NS_FAILED(rv)) return(rv);

                NS_ConvertUTF8toUTF16 theURI(uri);

                if (theURI.Find(ieFavoritesDir) == 0)
                {
                    if (theURI[theURI.Length() - 1] == '/')
                    {
                        rv = mNC_IEFavoriteFolder->GetValueUTF8(type);
                    }
                    else
                    {
                        rv = mNC_IEFavoriteObject->GetValueUTF8(type);
                    }
                    if (NS_FAILED(rv)) return(rv);
                }
            }
#endif

            NS_ConvertUTF8toUTF16 url(type);
            nsCOMPtr<nsIRDFLiteral> literal;
            mRDFService->GetLiteral(url.get(), getter_AddRefs(literal));
            rv = literal->QueryInterface(NS_GET_IID(nsIRDFNode), (void**) target);
            return(rv);
        }
        else if (property == mNC_pulse)
        {
            nsCOMPtr<nsIRDFLiteral> pulseLiteral;
            mRDFService->GetLiteral(NS_LITERAL_STRING("12").get(), getter_AddRefs(pulseLiteral));
            rv = pulseLiteral->QueryInterface(NS_GET_IID(nsIRDFNode), (void**) target);
            return(rv);
        }
        else if (property == mNC_Child)
        {
            // Oh this is evil. Somebody kill me now.
            nsCOMPtr<nsISimpleEnumerator> children;
            rv = GetFolderList(source, PR_FALSE, PR_TRUE, getter_AddRefs(children));
            if (NS_FAILED(rv) || (rv == NS_RDF_NO_VALUE)) return(rv);

            PRBool hasMore;
            rv = children->HasMoreElements(&hasMore);
            if (NS_FAILED(rv)) return(rv);

            if (hasMore)
            {
                nsCOMPtr<nsISupports> isupports;
                rv = children->GetNext(getter_AddRefs(isupports));
                if (NS_FAILED(rv)) return(rv);

                return isupports->QueryInterface(NS_GET_IID(nsIRDFNode), (void**) target);
            }
        }
#ifdef USE_NC_EXTENSION
        else if (property == mNC_extension)
        {
            nsCOMPtr<nsIRDFLiteral> extension;
            rv = GetExtension(source, getter_AddRefs(extension));
            if (!extension)    rv = NS_RDF_NO_VALUE;
            if (rv == NS_RDF_NO_VALUE) return(rv);
            return extension->QueryInterface(NS_GET_IID(nsIRDFNode), (void**) target);
        }
#endif
    }

    return(NS_RDF_NO_VALUE);
}



NS_IMETHODIMP
FileSystemDataSource::GetTargets(nsIRDFResource *source,
                nsIRDFResource *property,
                PRBool tv,
                nsISimpleEnumerator **targets /* out */)
{
    NS_PRECONDITION(source != nsnull, "null ptr");
    if (! source)
        return NS_ERROR_NULL_POINTER;

    NS_PRECONDITION(property != nsnull, "null ptr");
    if (! property)
        return NS_ERROR_NULL_POINTER;

    NS_PRECONDITION(targets != nsnull, "null ptr");
    if (! targets)
        return NS_ERROR_NULL_POINTER;

    *targets = nsnull;

    // we only have positive assertions in the file system data source.
    if (! tv)
        return NS_RDF_NO_VALUE;

    nsresult rv;

    if (source == mNC_FileSystemRoot)
    {
        if (property == mNC_Child)
        {
            return GetVolumeList(targets);
        }
        else if (property == mNC_pulse)
        {
            nsCOMPtr<nsIRDFLiteral> pulseLiteral;
            mRDFService->GetLiteral(NS_LITERAL_STRING("12").get(),
                                    getter_AddRefs(pulseLiteral));
            return NS_NewSingletonEnumerator(targets, pulseLiteral);
        }
    }
    else if (isFileURI(source))
    {
        if (property == mNC_Child)
        {
            return GetFolderList(source, PR_FALSE, PR_FALSE, targets);
        }
        else if (property == mNC_Name)
        {
            nsCOMPtr<nsIRDFLiteral> name;
            rv = GetName(source, getter_AddRefs(name));
            if (NS_FAILED(rv)) return rv;

            return NS_NewSingletonEnumerator(targets, name);
        }
        else if (property == mNC_URL)
        {
            nsCOMPtr<nsIRDFLiteral> url;
            rv = GetURL(source, nsnull, getter_AddRefs(url));
            if (NS_FAILED(rv)) return rv;

            return NS_NewSingletonEnumerator(targets, url);
        }
        else if (property == mRDF_type)
        {
            nsCString uri;
            rv = mNC_FileSystemObject->GetValueUTF8(uri);
            if (NS_FAILED(rv)) return rv;

            NS_ConvertUTF8toUTF16 url(uri);

            nsCOMPtr<nsIRDFLiteral> literal;
            rv = mRDFService->GetLiteral(url.get(), getter_AddRefs(literal));
            if (NS_FAILED(rv)) return rv;

            return NS_NewSingletonEnumerator(targets, literal);
        }
        else if (property == mNC_pulse)
        {
            nsCOMPtr<nsIRDFLiteral> pulseLiteral;
            rv = mRDFService->GetLiteral(NS_LITERAL_STRING("12").get(),
                getter_AddRefs(pulseLiteral));
            if (NS_FAILED(rv)) return rv;

            return NS_NewSingletonEnumerator(targets, pulseLiteral);
        }
    }

    return NS_NewEmptyEnumerator(targets);
}



NS_IMETHODIMP
FileSystemDataSource::Assert(nsIRDFResource *source,
                       nsIRDFResource *property,
                       nsIRDFNode *target,
                       PRBool tv)
{
    return NS_RDF_ASSERTION_REJECTED;
}



NS_IMETHODIMP
FileSystemDataSource::Unassert(nsIRDFResource *source,
                         nsIRDFResource *property,
                         nsIRDFNode *target)
{
    return NS_RDF_ASSERTION_REJECTED;
}



NS_IMETHODIMP
FileSystemDataSource::Change(nsIRDFResource* aSource,
                             nsIRDFResource* aProperty,
                             nsIRDFNode* aOldTarget,
                             nsIRDFNode* aNewTarget)
{
    return NS_RDF_ASSERTION_REJECTED;
}



NS_IMETHODIMP
FileSystemDataSource::Move(nsIRDFResource* aOldSource,
                           nsIRDFResource* aNewSource,
                           nsIRDFResource* aProperty,
                           nsIRDFNode* aTarget)
{
    return NS_RDF_ASSERTION_REJECTED;
}



NS_IMETHODIMP
FileSystemDataSource::HasAssertion(nsIRDFResource *source,
                             nsIRDFResource *property,
                             nsIRDFNode *target,
                             PRBool tv,
                             PRBool *hasAssertion /* out */)
{
    NS_PRECONDITION(source != nsnull, "null ptr");
    if (! source)
        return NS_ERROR_NULL_POINTER;

    NS_PRECONDITION(property != nsnull, "null ptr");
    if (! property)
        return NS_ERROR_NULL_POINTER;

    NS_PRECONDITION(target != nsnull, "null ptr");
    if (! target)
        return NS_ERROR_NULL_POINTER;

    NS_PRECONDITION(hasAssertion != nsnull, "null ptr");
    if (! hasAssertion)
        return NS_ERROR_NULL_POINTER;

    // we only have positive assertions in the file system data source.
    *hasAssertion = PR_FALSE;

    if (! tv) {
        return NS_OK;
    }

    if ((source == mNC_FileSystemRoot) || isFileURI(source))
    {
        if (property == mRDF_type)
        {
            nsCOMPtr<nsIRDFResource> resource( do_QueryInterface(target) );
            if (resource.get() == mRDF_type)
            {
                *hasAssertion = PR_TRUE;
            }
        }
#ifdef USE_NC_EXTENSION
        else if (property == mNC_extension)
        {
            // Cheat just a little here by making dirs always match
            if (isDirURI(source))
            {
                *hasAssertion = PR_TRUE;
            }
            else
            {
                nsCOMPtr<nsIRDFLiteral> extension;
                GetExtension(source, getter_AddRefs(extension));
                if (extension.get() == target)
                {
                    *hasAssertion = PR_TRUE;
                }
            }
        }
#endif
        else if (property == mNC_IsDirectory)
        {
            PRBool isDir = isDirURI(source);
            PRBool isEqual = PR_FALSE;
            target->EqualsNode(mLiteralTrue, &isEqual);
            if (isEqual)
            {
                *hasAssertion = isDir;
            }
            else
            {
                target->EqualsNode(mLiteralFalse, &isEqual);
                if (isEqual)
                    *hasAssertion = !isDir;
            }
        }
    }

    return NS_OK;
}



NS_IMETHODIMP 
FileSystemDataSource::HasArcIn(nsIRDFNode *aNode, nsIRDFResource *aArc, PRBool *result)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}



NS_IMETHODIMP 
FileSystemDataSource::HasArcOut(nsIRDFResource *aSource, nsIRDFResource *aArc, PRBool *result)
{
    *result = PR_FALSE;

    if (aSource == mNC_FileSystemRoot)
    {
        *result = (aArc == mNC_Child || aArc == mNC_pulse);
    }
    else if (isFileURI(aSource))
    {
        if (aArc == mNC_pulse)
        {
            *result = PR_TRUE;
        }
        else if (isDirURI(aSource))
        {
#ifdef  XP_WIN
            *result = isValidFolder(aSource);
#else
            *result = PR_TRUE;
#endif
        }
        else if (aArc == mNC_pulse || aArc == mNC_Name || aArc == mNC_Icon ||
                 aArc == mNC_URL || aArc == mNC_Length || aArc == mWEB_LastMod ||
                 aArc == mNC_FileSystemObject || aArc == mRDF_InstanceOf ||
                 aArc == mRDF_type)
        {
            *result = PR_TRUE;
        }
    }
    return NS_OK;
}



NS_IMETHODIMP
FileSystemDataSource::ArcLabelsIn(nsIRDFNode *node,
                            nsISimpleEnumerator ** labels /* out */)
{
//  NS_NOTYETIMPLEMENTED("write me");
    return NS_ERROR_NOT_IMPLEMENTED;
}



NS_IMETHODIMP
FileSystemDataSource::ArcLabelsOut(nsIRDFResource *source,
                   nsISimpleEnumerator **labels /* out */)
{
    NS_PRECONDITION(source != nsnull, "null ptr");
    if (! source)
    return NS_ERROR_NULL_POINTER;

    NS_PRECONDITION(labels != nsnull, "null ptr");
    if (! labels)
    return NS_ERROR_NULL_POINTER;

    nsresult rv;

    if (source == mNC_FileSystemRoot)
    {
        nsCOMPtr<nsISupportsArray> array;
        rv = NS_NewISupportsArray(getter_AddRefs(array));
        if (NS_FAILED(rv)) return rv;

        array->AppendElement(mNC_Child);
        array->AppendElement(mNC_pulse);

        return NS_NewArrayEnumerator(labels, array);
    }
    else if (isFileURI(source))
    {
        nsCOMPtr<nsISupportsArray> array;
        rv = NS_NewISupportsArray(getter_AddRefs(array));
        if (NS_FAILED(rv)) return rv;

        if (isDirURI(source))
        {
#ifdef  XP_WIN
            if (isValidFolder(source) == PR_TRUE)
            {
                array->AppendElement(mNC_Child);
            }
#else
            array->AppendElement(mNC_Child);
#endif
            array->AppendElement(mNC_pulse);
        }

        return NS_NewArrayEnumerator(labels, array);
    }

    return NS_NewEmptyEnumerator(labels);
}



NS_IMETHODIMP
FileSystemDataSource::GetAllResources(nsISimpleEnumerator** aCursor)
{
    NS_NOTYETIMPLEMENTED("sorry!");
    return NS_ERROR_NOT_IMPLEMENTED;
}



NS_IMETHODIMP
FileSystemDataSource::AddObserver(nsIRDFObserver *n)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}



NS_IMETHODIMP
FileSystemDataSource::RemoveObserver(nsIRDFObserver *n)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}



NS_IMETHODIMP
FileSystemDataSource::GetAllCmds(nsIRDFResource* source,
                                     nsISimpleEnumerator/*<nsIRDFResource>*/** commands)
{
    return(NS_NewEmptyEnumerator(commands));
}



NS_IMETHODIMP
FileSystemDataSource::IsCommandEnabled(nsISupportsArray/*<nsIRDFResource>*/* aSources,
                                       nsIRDFResource*   aCommand,
                                       nsISupportsArray/*<nsIRDFResource>*/* aArguments,
                                       PRBool* aResult)
{
    return(NS_ERROR_NOT_IMPLEMENTED);
}



NS_IMETHODIMP
FileSystemDataSource::DoCommand(nsISupportsArray/*<nsIRDFResource>*/* aSources,
                                nsIRDFResource*   aCommand,
                                nsISupportsArray/*<nsIRDFResource>*/* aArguments)
{
    return(NS_ERROR_NOT_IMPLEMENTED);
}



NS_IMETHODIMP
FileSystemDataSource::BeginUpdateBatch()
{
    return NS_OK;
}



NS_IMETHODIMP
FileSystemDataSource::EndUpdateBatch()
{
    return NS_OK;
}



nsresult
FileSystemDataSource::GetVolumeList(nsISimpleEnumerator** aResult)
{
    nsresult rv;
    nsCOMPtr<nsISupportsArray> volumes;

    rv = NS_NewISupportsArray(getter_AddRefs(volumes));
    if (NS_FAILED(rv)) return rv;

    nsCOMPtr<nsIRDFResource> vol;

#ifdef  XP_MAC
    StrFileName     fname;
    HParamBlockRec  pb;
    for (int16 volNum = 1; ; volNum++)
    {
        pb.volumeParam.ioCompletion = NULL;
        pb.volumeParam.ioVolIndex = volNum;
        pb.volumeParam.ioNamePtr = (StringPtr)fname;
        if (PBHGetVInfo(&pb,FALSE) != noErr)
            break;
        FSSpec fss(pb.volumeParam.ioVRefNum, fsRtParID, fname);
        nsCOMPtr<nsILocalFileMac> lf;
        NS_NewLocalFileWithFSSpec(fss, true, getter_AddRefs(lf));

        nsCOMPtr<nsIURI> furi;
        NS_NewFileURI(getter_AddRefs(furi), lf); 

        nsXPIDLCString spec;
        furi->GetSpec(getter_Copies(spec);

        rv = mRDFService->GetResource(spec, getter_AddRefs(vol));
        if (NS_FAILED(rv)) return rv;

        volumes->AppendElement(vol);
    }
#endif

#if defined (XP_WIN) && !defined (WINCE)

    PRInt32         driveType;
    char            drive[32];
    PRInt32         volNum;
    char            *url;

    for (volNum = 0; volNum < 26; volNum++)
    {
        sprintf(drive, "%c:\\", volNum + 'A');
        driveType = GetDriveType(drive);
        if (driveType != DRIVE_UNKNOWN && driveType != DRIVE_NO_ROOT_DIR)
        {
            if (nsnull != (url = PR_smprintf("file:///%c|/", volNum + 'A')))
            {
                rv = mRDFService->GetResource(nsDependentCString(url),
                                              getter_AddRefs(vol));
                PR_Free(url);

                if (NS_FAILED(rv)) return rv;
                volumes->AppendElement(vol);
            }
        }
    }
#endif

#if defined(XP_UNIX) || defined(XP_BEOS)
    mRDFService->GetResource(NS_LITERAL_CSTRING("file:///"), getter_AddRefs(vol));
    volumes->AppendElement(vol);
#endif

#ifdef XP_OS2
    ULONG ulDriveNo = 0;
    ULONG ulDriveMap = 0;
    char *url;

    rv = DosQueryCurrentDisk(&ulDriveNo, &ulDriveMap);
    if (NS_FAILED(rv))
        return rv;

    for (int volNum = 0; volNum < 26; volNum++)
    {
        if (((ulDriveMap << (31 - volNum)) >> 31))
        {
            if (nsnull != (url = PR_smprintf("file:///%c|/", volNum + 'A')))
            {
                rv = mRDFService->GetResource(nsDependentCString(url), getter_AddRefs(vol));
                PR_Free(url);

                if (NS_FAILED(rv)) return rv;
                volumes->AppendElement(vol);
            }
        }

    }
#endif

    return NS_NewArrayEnumerator(aResult, volumes);
}



#ifdef  XP_WIN
PRBool
FileSystemDataSource::isValidFolder(nsIRDFResource *source)
{
    PRBool  isValid = PR_TRUE;
    if (ieFavoritesDir.IsEmpty())    return(isValid);

    nsresult        rv;
    nsCString       uri;
    rv = source->GetValueUTF8(uri);
    if (NS_FAILED(rv)) return(isValid);

    NS_ConvertUTF8toUTF16 theURI(uri);
    if (theURI.Find(ieFavoritesDir) == 0)
    {
        isValid = PR_FALSE;

        nsCOMPtr<nsISimpleEnumerator>   folderEnum;
        if (NS_SUCCEEDED(rv = GetFolderList(source, PR_TRUE, PR_FALSE, getter_AddRefs(folderEnum))))
        {
            PRBool      hasAny = PR_FALSE, hasMore;
            while (NS_SUCCEEDED(folderEnum->HasMoreElements(&hasMore)) &&
                    (hasMore == PR_TRUE))
            {
                hasAny = PR_TRUE;

                nsCOMPtr<nsISupports>       isupports;
                if (NS_FAILED(rv = folderEnum->GetNext(getter_AddRefs(isupports))))
                    break;
                nsCOMPtr<nsIRDFResource>    res = do_QueryInterface(isupports);
                if (!res)   break;

                nsCOMPtr<nsIRDFLiteral>     nameLiteral;
                if (NS_FAILED(rv = GetName(res, getter_AddRefs(nameLiteral))))
                    break;
                
                const PRUnichar         *uniName;
                if (NS_FAILED(rv = nameLiteral->GetValueConst(&uniName)))
                    break;
                nsAutoString            name(uniName);

                // An empty folder, or a folder that contains just "desktop.ini",
                // is considered to be a IE Favorite; otherwise, its a folder
                if (!name.LowerCaseEqualsLiteral("desktop.ini"))
                {
                    isValid = PR_TRUE;
                    break;
                }
            }
            if (hasAny == PR_FALSE) isValid = PR_TRUE;
        }
    }
    return(isValid);
}
#endif



nsresult
FileSystemDataSource::GetFolderList(nsIRDFResource *source, PRBool allowHidden,
                PRBool onlyFirst, nsISimpleEnumerator** aResult)
{
    if (!isDirURI(source))
        return(NS_RDF_NO_VALUE);

    nsresult                    rv;
    nsCOMPtr<nsISupportsArray>  nameArray;

    rv = NS_NewISupportsArray(getter_AddRefs(nameArray));
    if (NS_FAILED(rv))
        return(rv);

    const char      *parentURI = nsnull;
    rv = source->GetValueConst(&parentURI);
    if (NS_FAILED(rv))
        return(rv);
    if (!parentURI)
        return(NS_ERROR_UNEXPECTED);

    nsCOMPtr<nsIURI>    aIURI;
    if (NS_FAILED(rv = NS_NewURI(getter_AddRefs(aIURI), nsDependentCString(parentURI))))
        return(rv);

    nsCOMPtr<nsIFileURL>    fileURL = do_QueryInterface(aIURI);
    if (!fileURL)
        return(PR_FALSE);

    nsCOMPtr<nsIFile>   aDir;
    if (NS_FAILED(rv = fileURL->GetFile(getter_AddRefs(aDir))))
        return(rv);

    // ensure that we DO NOT resolve aliases
    nsCOMPtr<nsILocalFile>  aDirLocal = do_QueryInterface(aDir);
    if (aDirLocal)
        aDirLocal->SetFollowLinks(PR_FALSE);

    nsCOMPtr<nsISimpleEnumerator>   dirContents;
    if (NS_FAILED(rv = aDir->GetDirectoryEntries(getter_AddRefs(dirContents))))
        return(rv);
    if (!dirContents)
        return(NS_ERROR_UNEXPECTED);

    PRBool          hasMore;
    while(NS_SUCCEEDED(rv = dirContents->HasMoreElements(&hasMore)) &&
        (hasMore == PR_TRUE))
    {
        nsCOMPtr<nsISupports>   isupports;
        if (NS_FAILED(rv = dirContents->GetNext(getter_AddRefs(isupports))))
            break;

        nsCOMPtr<nsIFile>   aFile = do_QueryInterface(isupports);
        if (!aFile)
            break;

        if (allowHidden == PR_FALSE)
        {
            PRBool          hiddenFlag = PR_FALSE;
            if (NS_FAILED(rv = aFile->IsHidden(&hiddenFlag)))
                break;
            if (hiddenFlag == PR_TRUE)
                continue;
        }

        nsAutoString leafStr;
        if (NS_FAILED(rv = aFile->GetLeafName(leafStr)))
            break;
        if (leafStr.IsEmpty())
            continue;
  
        nsCAutoString           fullURI;
        fullURI.Assign(parentURI);
        if (fullURI.Last() != '/')
        {
            fullURI.Append('/');
        }

        char    *escLeafStr = nsEscape(NS_ConvertUTF16toUTF8(leafStr).get(), url_Path);
        leafStr.Truncate();

        if (!escLeafStr)
            continue;
  
        nsCAutoString           leaf(escLeafStr);
        NS_Free(escLeafStr);
        escLeafStr = nsnull;

        // using nsEscape() [above] doesn't escape slashes, so do that by hand
        PRInt32         aOffset;
        while ((aOffset = leaf.FindChar('/')) >= 0)
        {
            leaf.Cut((PRUint32)aOffset, 1);
            leaf.Insert("%2F", (PRUint32)aOffset);
        }

        // append the encoded name
        fullURI.Append(leaf);

        PRBool          dirFlag = PR_FALSE;
        rv = aFile->IsDirectory(&dirFlag);
        if (NS_SUCCEEDED(rv) && (dirFlag == PR_TRUE))
        {
            fullURI.Append('/');
        }

        nsCOMPtr<nsIRDFResource>    fileRes;
        mRDFService->GetResource(fullURI, getter_AddRefs(fileRes));

        nameArray->AppendElement(fileRes);

        if (onlyFirst == PR_TRUE)
            break;
    }

    return NS_NewArrayEnumerator(aResult, nameArray);
}

nsresult
FileSystemDataSource::GetLastMod(nsIRDFResource *source, nsIRDFDate **aResult)
{
    *aResult = nsnull;

    nsresult        rv;
    const char      *uri = nsnull;

    rv = source->GetValueConst(&uri);
    if (NS_FAILED(rv)) return(rv);
    if (!uri)
        return(NS_ERROR_UNEXPECTED);

    nsCOMPtr<nsIURI>    aIURI;
    if (NS_FAILED(rv = NS_NewURI(getter_AddRefs(aIURI), nsDependentCString(uri))))
        return(rv);

    nsCOMPtr<nsIFileURL>    fileURL = do_QueryInterface(aIURI);
    if (!fileURL)
        return(PR_FALSE);

    nsCOMPtr<nsIFile>   aFile;
    if (NS_FAILED(rv = fileURL->GetFile(getter_AddRefs(aFile))))
        return(rv);
    if (!aFile)
        return(NS_ERROR_UNEXPECTED);

    // ensure that we DO NOT resolve aliases
    nsCOMPtr<nsILocalFile>  aFileLocal = do_QueryInterface(aFile);
    if (aFileLocal)
        aFileLocal->SetFollowLinks(PR_FALSE);

    PRInt64 lastModDate;
    if (NS_FAILED(rv = aFile->GetLastModifiedTime(&lastModDate)))
        return(rv);

    // convert from milliseconds to seconds
    PRTime      temp64, thousand;
    LL_I2L(thousand, PR_MSEC_PER_SEC);
    LL_MUL(temp64, lastModDate, thousand);

    mRDFService->GetDateLiteral(temp64, aResult);

    return(NS_OK);
}



nsresult
FileSystemDataSource::GetFileSize(nsIRDFResource *source, nsIRDFInt **aResult)
{
    *aResult = nsnull;

    nsresult        rv;
    const char      *uri = nsnull;

    rv = source->GetValueConst(&uri);
    if (NS_FAILED(rv))
        return(rv);
    if (!uri)
        return(NS_ERROR_UNEXPECTED);

    nsCOMPtr<nsIURI>    aIURI;
    if (NS_FAILED(rv = NS_NewURI(getter_AddRefs(aIURI), nsDependentCString(uri))))
        return(rv);

    nsCOMPtr<nsIFileURL>    fileURL = do_QueryInterface(aIURI);
    if (!fileURL)
        return(PR_FALSE);

    nsCOMPtr<nsIFile>   aFile;
    if (NS_FAILED(rv = fileURL->GetFile(getter_AddRefs(aFile))))
        return(rv);
    if (!aFile)
        return(NS_ERROR_UNEXPECTED);

    // ensure that we DO NOT resolve aliases
    nsCOMPtr<nsILocalFile>  aFileLocal = do_QueryInterface(aFile);
    if (aFileLocal)
        aFileLocal->SetFollowLinks(PR_FALSE);

    // don't do anything with directories
    PRBool  isDir = PR_FALSE;
    if (NS_FAILED(rv = aFile->IsDirectory(&isDir)))
        return(rv);
    if (isDir == PR_TRUE)
        return(NS_RDF_NO_VALUE);

    PRInt64     aFileSize64;
#ifdef  XP_MAC
    // on Mac, get total file size (data + resource fork)
    nsCOMPtr<nsILocalFileMac>   aMacFile = do_QueryInterface(aFile);
    if (!aMacFile)
        return(NS_ERROR_UNEXPECTED);
    if (NS_FAILED(rv = aMacFile->GetFileSizeWithResFork(&aFileSize64)))
        return(rv);
#else
    if (NS_FAILED(rv = aFile->GetFileSize(&aFileSize64)))
        return(rv);
#endif

    // convert 64bits to 32bits
    PRInt32     aFileSize32 = 0;
    LL_L2I(aFileSize32, aFileSize64);

    mRDFService->GetIntLiteral(aFileSize32, aResult);

    return(NS_OK);
}



nsresult
FileSystemDataSource::GetName(nsIRDFResource *source, nsIRDFLiteral **aResult)
{
    nsresult        rv;
    const char      *uri = nsnull;

    rv = source->GetValueConst(&uri);
    if (NS_FAILED(rv))
        return(rv);
    if (!uri)
        return(NS_ERROR_UNEXPECTED);

    nsCOMPtr<nsIURI>    aIURI;
    if (NS_FAILED(rv = NS_NewURI(getter_AddRefs(aIURI), nsDependentCString(uri))))
        return(rv);

    nsCOMPtr<nsIFileURL>    fileURL = do_QueryInterface(aIURI);
    if (!fileURL)
        return(PR_FALSE);

    nsCOMPtr<nsIFile>   aFile;
    if (NS_FAILED(rv = fileURL->GetFile(getter_AddRefs(aFile))))
        return(rv);
    if (!aFile)
        return(NS_ERROR_UNEXPECTED);

    // ensure that we DO NOT resolve aliases
    nsCOMPtr<nsILocalFile>  aFileLocal = do_QueryInterface(aFile);
    if (aFileLocal)
        aFileLocal->SetFollowLinks(PR_FALSE);

    nsAutoString name;
    if (NS_FAILED(rv = aFile->GetLeafName(name)))
        return(rv);
    if (name.IsEmpty())
        return(NS_ERROR_UNEXPECTED);

#ifdef  XP_MAC
    nsCOMPtr<nsILocalFileMac>   aMacFile = do_QueryInterface(aFile);
    if (aMacFile)
    {
        PRBool isPackageFlag = PR_FALSE;
        rv = aMacFile->IsPackage(&isPackageFlag);
        if (NS_SUCCEEDED(rv) && (isPackageFlag == PR_TRUE))
        {
            // mungle package names under Mac OS 9/X
            PRUint32 len = name.Length();
            if (name.RFind(".app", PR_TRUE) == len - 4)
            {
                name.SetLength(len-4);
            }
        }
    }
#endif

#ifdef  XP_WIN
    // special hack for IE favorites under Windows; strip off the
    // trailing ".url" or ".lnk" at the end of IE favorites names
    PRInt32 nameLen = name.Length();
    if ((strncmp(uri, ieFavoritesDir.get(), ieFavoritesDir.Length()) == 0) && (nameLen > 4))
    {
        nsAutoString extension;
        name.Right(extension, 4);
        if (extension.LowerCaseEqualsLiteral(".url") ||
            extension.LowerCaseEqualsLiteral(".lnk"))
        {
            name.Truncate(nameLen - 4);
        }
    }
#endif

#ifdef  XP_BEOS
    // under BEOS, try and get the "META:title" attribute (if its a file)
    if (strstr(uri, netPositiveDir.get()) != 0)
    {
        PRBool value;
        if ((NS_SUCCEEDED(aFileLocal->IsFile(&value) && value)) ||
            (NS_SUCCEEDED(aFileLocal->IsDirectory(&value) && value)))
        {
            nsXPIDLCString nativePath;
            aFileLocal->GetNativePath(nativePath);

            rv = NS_ERROR_FAILURE;
            if (nativePath) 
            {
                BFile   bf(nativePath.get(), B_READ_ONLY);
                if (bf.InitCheck() == B_OK)
                {
                    char        beNameAttr[4096];
                    ssize_t     len;

                    if ((len = bf.ReadAttr("META:title", B_STRING_TYPE,
                        0, beNameAttr, sizeof(beNameAttr)-1)) > 0)
                    {
                        beNameAttr[len] = '\0';
                        CopyUTF8toUTF16(beNameAttr, name);
                        rv = NS_OK;
                    }
                }
            }
            if (NS_OK != rv)
            {
                nsCAutoString leafName;
                rv = aFileLocal->GetNativeLeafName(leafName);
                if (NS_SUCCEEDED(rv)) {
                    CopyUTF8toUTF16(leafName, name);
                    rv = NS_OK;
                }
            }
        }
    }
#endif

    mRDFService->GetLiteral(name.get(), aResult);

    return NS_OK;
}



#ifdef USE_NC_EXTENSION
nsresult
FileSystemDataSource::GetExtension(nsIRDFResource *source, nsIRDFLiteral **aResult)
{
    nsCOMPtr<nsIRDFLiteral> name;
    nsresult rv = GetName(source, getter_AddRefs(name));
    if (NS_FAILED(rv))
        return rv;

    const PRUnichar* unicodeLeafName;
    rv = name->GetValueConst(&unicodeLeafName);
    if (NS_FAILED(rv))
        return rv;

    nsAutoString filename(unicodeLeafName);
    PRInt32 lastDot = filename.RFindChar('.');
    if (lastDot == -1)
    {
        mRDFService->GetLiteral(EmptyString().get(), aResult);
    }
    else
    {
        nsAutoString extension;
        filename.Right(extension, (filename.Length() - lastDot));
        mRDFService->GetLiteral(extension.get(), aResult);
    }

    return NS_OK;
}
#endif

#ifdef  XP_WIN
nsresult
FileSystemDataSource::getIEFavoriteURL(nsIRDFResource *source, nsString aFileURL, nsIRDFLiteral **urlLiteral)
{
    nsresult        rv = NS_OK;
    
    *urlLiteral = nsnull;

    nsCOMPtr<nsIFile> f;
    NS_GetFileFromURLSpec(NS_ConvertUTF16toUTF8(aFileURL), getter_AddRefs(f)); 

    PRBool value;

    if (NS_SUCCEEDED(f->IsDirectory(&value)) && value)
    {
        if (isValidFolder(source))
            return(NS_RDF_NO_VALUE);
        aFileURL.AppendLiteral("desktop.ini");
    }
    else if (aFileURL.Length() > 4)
    {
        nsAutoString    extension;

        aFileURL.Right(extension, 4);
        if (!extension.LowerCaseEqualsLiteral(".url"))
        {
            return(NS_RDF_NO_VALUE);
        }
    }

    nsCOMPtr<nsIInputStream> strm;
    NS_NewLocalFileInputStream(getter_AddRefs(strm),f);
    nsCOMPtr<nsILineInputStream> linereader = do_QueryInterface(strm, &rv);

    nsAutoString    line;
    nsCAutoString   cLine;
    while(NS_SUCCEEDED(rv))
    {
        PRBool  isEOF;
        rv = linereader->ReadLine(cLine, &isEOF);
        CopyASCIItoUTF16(cLine, line);

        if (isEOF)
        {
            if (line.Find("URL=", PR_TRUE) == 0)
            {
                line.Cut(0, 4);
                rv = mRDFService->GetLiteral(line.get(), urlLiteral);
                break;
            }
            else if (line.Find("CDFURL=", PR_TRUE) == 0)
            {
                line.Cut(0, 7);
                rv = mRDFService->GetLiteral(line.get(), urlLiteral);
                break;
            }
            line.Truncate();
        }
    }

    return(rv);
}
#endif



nsresult
FileSystemDataSource::GetURL(nsIRDFResource *source, PRBool *isFavorite, nsIRDFLiteral** aResult)
{
    if (isFavorite) *isFavorite = PR_FALSE;

    nsresult        rv;
    nsCString       uri;
	
    rv = source->GetValueUTF8(uri);
    if (NS_FAILED(rv))
        return(rv);

    NS_ConvertUTF8toUTF16 url(uri);

#ifdef  XP_WIN
    // under Windows, if its an IE favorite, munge the URL
    if (!ieFavoritesDir.IsEmpty())
    {
        if (url.Find(ieFavoritesDir) == 0)
        {
            if (isFavorite) *isFavorite = PR_TRUE;
            rv = getIEFavoriteURL(source, url, aResult);
            return(rv);
        }
    }
#endif

#ifdef  XP_BEOS
    // under BEOS, try and get the "META:url" attribute
    if (!netPositiveDir.IsEmpty())
    {
        if (strstr(uri.get(), netPositiveDir.get()) != 0)
        {
            if (isFavorite) *isFavorite = PR_TRUE;
            rv = getNetPositiveURL(source, url, aResult);
            return(rv);
        }
    }
#endif

    // if we fall through to here, its not any type of bookmark
    // stored in the platform native file system, so just set the URL

    mRDFService->GetLiteral(url.get(), aResult);

    return(NS_OK);
}



#ifdef  XP_BEOS

nsresult
FileSystemDataSource::getNetPositiveURL(nsIRDFResource *source, nsString aFileURL, nsIRDFLiteral **urlLiteral)
{
    nsresult        rv = NS_RDF_NO_VALUE;

    *urlLiteral = nsnull;


    nsCOMPtr<nsIFile> f;
    NS_GetFileFromURLSpec(NS_ConvertUTF16toUTF8(aFileURL), getter_AddRefs(f)); 



    nsXPIDLCString nativePath;
    f->GetNativePath(nativePath);

    PRBool value;
    if (NS_SUCCEEDED(f->IsFile(&value) && value))
    {
        if (nativePath)
        {
            BFile   bf(nativePath.get(), B_READ_ONLY);
            if (bf.InitCheck() == B_OK)
            {
                char        beURLattr[4096];
                ssize_t     len;

                if ((len = bf.ReadAttr("META:url", B_STRING_TYPE,
                    0, beURLattr, sizeof(beURLattr)-1)) > 0)
                {
                    beURLattr[len] = '\0';
                    nsAutoString    bookmarkURL;
                    CopyUTF8toUTF16(beURLattr, bookmarkURL);
                    rv = mRDFService->GetLiteral(bookmarkURL.get(),
                                                 urlLiteral);
                }
            }
        }
    }
    return(rv);
}

#endif
