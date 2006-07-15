/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* vim:set sw=4 sts=4 et cin: */
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
 *   Bradley Baetz <bbaetz@cs.mcgill.ca>
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


/*

  The converts a filesystem directory into an "HTTP index" stream per
  Lou Montulli's original spec:

  http://www.mozilla.org/projects/netlib/dirindexformat.html

 */

#include "nsEscape.h"
#include "nsDirectoryIndexStream.h"
#include "nsXPIDLString.h"
#include "prio.h"
#include "prlog.h"
#include "prlong.h"
#ifdef PR_LOGGING
static PRLogModuleInfo* gLog;
#endif

#include "nsISimpleEnumerator.h"
#include "nsICollation.h"
#include "nsILocale.h"
#include "nsILocaleService.h"
#include "nsCollationCID.h"
#include "nsIPlatformCharset.h"
#include "nsReadableUtils.h"
#include "nsURLHelper.h"
#include "nsNetUtil.h"
#include "nsCRT.h"
#include "nsNativeCharsetUtils.h"

// NOTE: This runs on the _file transport_ thread.
// The problem is that now that we're actually doing something with the data,
// we want to do stuff like i18n sorting. However, none of the collation stuff
// is threadsafe.
// So THIS CODE IS ASCII ONLY!!!!!!!! This is no worse than the current
// behaviour, though. See bug 99382.
// When this is fixed, #define THREADSAFE_I18N to get this code working

//#define THREADSAFE_I18N

nsDirectoryIndexStream::nsDirectoryIndexStream()
    : mOffset(0), mStatus(NS_OK), mPos(0)
{
#ifdef PR_LOGGING
    if (! gLog)
        gLog = PR_NewLogModule("nsDirectoryIndexStream");
#endif

    PR_LOG(gLog, PR_LOG_DEBUG,
           ("nsDirectoryIndexStream[%p]: created", this));
}

static int PR_CALLBACK compare(nsIFile* aElement1,
                               nsIFile* aElement2,
                               void* aData)
{
    if (!NS_IsNativeUTF8()) {
        // don't check for errors, because we can't report them anyway
        nsAutoString name1, name2;
        aElement1->GetLeafName(name1);
        aElement2->GetLeafName(name2);

        // Note - we should do the collation to do sorting. Why don't we?
        // Because that is _slow_. Using TestProtocols to list file:///dev/
        // goes from 3 seconds to 22. (This may be why nsXULSortService is
        // so slow as well).
        // Does this have bad effects? Probably, but since nsXULTree appears
        // to use the raw RDF literal value as the sort key (which ammounts to an
        // strcmp), it won't be any worse, I think.
        // This could be made faster, by creating the keys once,
        // but CompareString could still be smarter - see bug 99383 - bbaetz
        // NB - 99393 has been WONTFIXed. So if the I18N code is ever made
        // threadsafe so that this matters, we'd have to pass through a
        // struct { nsIFile*, PRUint8* } with the pre-calculated key.
        return Compare(name1, name2);
    }

    nsCAutoString name1, name2;
    aElement1->GetNativeLeafName(name1);
    aElement2->GetNativeLeafName(name2);

    return Compare(name1, name2);
}

nsresult
nsDirectoryIndexStream::Init(nsIFile* aDir)
{
    nsresult rv;
    PRBool isDir;
    rv = aDir->IsDirectory(&isDir);
    if (NS_FAILED(rv)) return rv;
    NS_PRECONDITION(isDir, "not a directory");
    if (!isDir)
        return NS_ERROR_ILLEGAL_VALUE;

#ifdef PR_LOGGING
    if (PR_LOG_TEST(gLog, PR_LOG_DEBUG)) {
        nsCAutoString path;
        aDir->GetNativePath(path);
        PR_LOG(gLog, PR_LOG_DEBUG,
               ("nsDirectoryIndexStream[%p]: initialized on %s",
                this, path.get()));
    }
#endif

    // Sigh. We have to allocate on the heap because there are no
    // assignment operators defined.
    nsCOMPtr<nsISimpleEnumerator> iter;
    rv = aDir->GetDirectoryEntries(getter_AddRefs(iter));
    if (NS_FAILED(rv)) return rv;

    // Now lets sort, because clients expect it that way
    // XXX - should we do so here, or when the first item is requested?
    // XXX - use insertion sort instead?

    PRBool more;
    nsCOMPtr<nsISupports> elem;
    while (NS_SUCCEEDED(iter->HasMoreElements(&more)) && more) {
        rv = iter->GetNext(getter_AddRefs(elem));
        if (NS_SUCCEEDED(rv)) {
            nsCOMPtr<nsIFile> file = do_QueryInterface(elem);
            if (file)
                mArray.AppendObject(file); // addrefs
        }
    }

#ifdef THREADSAFE_I18N
    nsCOMPtr<nsILocaleService> ls = do_GetService(NS_LOCALESERVICE_CONTRACTID,
                                                  &rv);
    if (NS_FAILED(rv)) return rv;

    nsCOMPtr<nsILocale> locale;
    rv = ls->GetApplicationLocale(getter_AddRefs(locale));
    if (NS_FAILED(rv)) return rv;
    
    nsCOMPtr<nsICollationFactory> cf = do_CreateInstance(NS_COLLATIONFACTORY_CONTRACTID,
                                                         &rv);
    if (NS_FAILED(rv)) return rv;

    nsCOMPtr<nsICollation> coll;
    rv = cf->CreateCollation(locale, getter_AddRefs(coll));
    if (NS_FAILED(rv)) return rv;

    mArray.Sort(compare, coll);
#else
    mArray.Sort(compare, nsnull);
#endif

    mBuf.AppendLiteral("300: ");
    nsCAutoString url;
    rv = net_GetURLSpecFromFile(aDir, url);
    if (NS_FAILED(rv)) return rv;
    mBuf.Append(url);
    mBuf.Append('\n');

    mBuf.AppendLiteral("200: filename content-length last-modified file-type\n");

    return NS_OK;
}

nsDirectoryIndexStream::~nsDirectoryIndexStream()
{
    PR_LOG(gLog, PR_LOG_DEBUG,
           ("nsDirectoryIndexStream[%p]: destroyed", this));
}

nsresult
nsDirectoryIndexStream::Create(nsIFile* aDir, nsIInputStream** aResult)
{
    nsDirectoryIndexStream* result = new nsDirectoryIndexStream();
    if (! result)
        return NS_ERROR_OUT_OF_MEMORY;

    nsresult rv;
    rv = result->Init(aDir);
    if (NS_FAILED(rv)) {
        delete result;
        return rv;
    }

    *aResult = result;
    NS_ADDREF(*aResult);
    return NS_OK;
}

NS_IMPL_THREADSAFE_ISUPPORTS1(nsDirectoryIndexStream, nsIInputStream)

// The below routines are proxied to the UI thread!
NS_IMETHODIMP
nsDirectoryIndexStream::Close()
{
    mStatus = NS_BASE_STREAM_CLOSED;
    return NS_OK;
}

NS_IMETHODIMP
nsDirectoryIndexStream::Available(PRUint32* aLength)
{
    if (NS_FAILED(mStatus))
        return mStatus;

    // If there's data in our buffer, use that
    if (mOffset < (PRInt32)mBuf.Length()) {
        *aLength = mBuf.Length() - mOffset;
        return NS_OK;
    }

    // Returning one byte is not ideal, but good enough
    *aLength = (mPos < mArray.Count()) ? 1 : 0;
    return NS_OK;
}

NS_IMETHODIMP
nsDirectoryIndexStream::Read(char* aBuf, PRUint32 aCount, PRUint32* aReadCount)
{
    if (mStatus == NS_BASE_STREAM_CLOSED) {
        *aReadCount = 0;
        return NS_OK;
    }
    if (NS_FAILED(mStatus))
        return mStatus;

    PRUint32 nread = 0;

    // If anything is enqueued (or left-over) in mBuf, then feed it to
    // the reader first.
    while (mOffset < (PRInt32)mBuf.Length() && aCount != 0) {
        *(aBuf++) = char(mBuf.CharAt(mOffset++));
        --aCount;
        ++nread;
    }

    // Room left?
    if (aCount > 0) {
        mOffset = 0;
        mBuf.Truncate();

        // Okay, now we'll suck stuff off of our iterator into the mBuf...
        while (PRUint32(mBuf.Length()) < aCount) {
            PRBool more = mPos < mArray.Count();
            if (!more) break;

            // don't addref, for speed - an addref happened when it
            // was placed in the array, so it's not going to go stale
            nsIFile* current = mArray.ObjectAt(mPos);
            ++mPos;

#ifdef PR_LOGGING
            if (PR_LOG_TEST(gLog, PR_LOG_DEBUG)) {
                nsCAutoString path;
                current->GetNativePath(path);
                PR_LOG(gLog, PR_LOG_DEBUG,
                       ("nsDirectoryIndexStream[%p]: iterated %s",
                        this, path.get()));
            }
#endif

            // rjc: don't return hidden files/directories!
            // bbaetz: why not?
            nsresult rv;
#ifndef XP_UNIX
            PRBool hidden = PR_FALSE;
            current->IsHidden(&hidden);
            if (hidden) {
                PR_LOG(gLog, PR_LOG_DEBUG,
                       ("nsDirectoryIndexStream[%p]: skipping hidden file/directory",
                        this));
                continue;
            }
#endif

            PRInt64 fileSize = 0;
            current->GetFileSize( &fileSize );

            PRInt64 fileInfoModifyTime = 0;
            current->GetLastModifiedTime( &fileInfoModifyTime );
            fileInfoModifyTime *= PR_USEC_PER_MSEC;

            mBuf.AppendLiteral("201: ");

            // The "filename" field
            char* escaped = nsnull;
            if (!NS_IsNativeUTF8()) {
                nsAutoString leafname;
                rv = current->GetLeafName(leafname);
                if (NS_FAILED(rv)) return rv;
                if (!leafname.IsEmpty())
                    escaped = nsEscape(NS_ConvertUTF16toUTF8(leafname).get(), url_Path);
            } else {
                nsCAutoString leafname;
                rv = current->GetNativeLeafName(leafname);
                if (NS_FAILED(rv)) return rv;
                if (!leafname.IsEmpty())
                    escaped = nsEscape(leafname.get(), url_Path);
            }
            if (escaped) {
                mBuf += escaped;
                mBuf.Append(' ');
                nsMemory::Free(escaped);
            }

            // The "content-length" field
            mBuf.AppendInt(fileSize, 10);
            mBuf.Append(' ');

            // The "last-modified" field
            PRExplodedTime tm;
            PR_ExplodeTime(fileInfoModifyTime, PR_GMTParameters, &tm);
            {
                char buf[64];
                PR_FormatTimeUSEnglish(buf, sizeof(buf), "%a,%%20%d%%20%b%%20%Y%%20%H:%M:%S%%20GMT ", &tm);
                mBuf.Append(buf);
            }

            // The "file-type" field
            PRBool isFile = PR_TRUE;
            current->IsFile(&isFile);
            if (isFile) {
                mBuf.AppendLiteral("FILE ");
            }
            else {
                PRBool isDir;
                rv = current->IsDirectory(&isDir);
                if (NS_FAILED(rv)) return rv; 
                if (isDir) {
                    mBuf.AppendLiteral("DIRECTORY ");
                }
                else {
                    PRBool isLink;
                    rv = current->IsSymlink(&isLink);
                    if (NS_FAILED(rv)) return rv; 
                    if (isLink) {
                        mBuf.AppendLiteral("SYMBOLIC-LINK ");
                    }
                }
            }

            mBuf.Append('\n');
        }

        // ...and once we've either run out of directory entries, or
        // filled up the buffer, then we'll push it to the reader.
        while (mOffset < (PRInt32)mBuf.Length() && aCount != 0) {
            *(aBuf++) = char(mBuf.CharAt(mOffset++));
            --aCount;
            ++nread;
        }
    }

    *aReadCount = nread;
    return NS_OK;
}

NS_IMETHODIMP
nsDirectoryIndexStream::ReadSegments(nsWriteSegmentFun writer, void * closure, PRUint32 count, PRUint32 *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP
nsDirectoryIndexStream::IsNonBlocking(PRBool *aNonBlocking)
{
    *aNonBlocking = PR_FALSE;
    return NS_OK;
}
