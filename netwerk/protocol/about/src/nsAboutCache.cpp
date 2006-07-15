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
 *   Alexey Chernyak <alexeyc@bigfoot.com> (XHTML 1.1 conversion)
 *   Henrik Gemal <mozilla@gemal.dk>
 *   Darin Fisher <darin@netscape.com>
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

#include "nsAboutCache.h"
#include "nsIIOService.h"
#include "nsIServiceManager.h"
#include "nsIInputStream.h"
#include "nsIStorageStream.h"
#include "nsISimpleEnumerator.h"
#include "nsXPIDLString.h"
#include "nsIURI.h"
#include "nsCOMPtr.h"
#include "nsNetUtil.h"
#include "prtime.h"
#include "nsEscape.h"

#include "nsICacheService.h"

static PRTime SecondsToPRTime(PRUint32 t_sec)
{
    PRTime t_usec, usec_per_sec;
    LL_I2L(t_usec, t_sec);
    LL_I2L(usec_per_sec, PR_USEC_PER_SEC);
    LL_MUL(t_usec, t_usec, usec_per_sec);
    return t_usec;
}
static void PrintTimeString(char *buf, PRUint32 bufsize, PRUint32 t_sec)
{
    PRExplodedTime et;
    PRTime t_usec = SecondsToPRTime(t_sec);
    PR_ExplodeTime(t_usec, PR_LocalTimeParameters, &et);
    PR_FormatTime(buf, bufsize, "%Y-%m-%d %H:%M:%S", &et);
}


NS_IMPL_ISUPPORTS2(nsAboutCache, nsIAboutModule, nsICacheVisitor)

NS_IMETHODIMP
nsAboutCache::NewChannel(nsIURI *aURI, nsIChannel **result)
{
    NS_ENSURE_ARG_POINTER(aURI);
    nsresult rv;
    PRUint32 bytesWritten;

    *result = nsnull;
    // Get the cache manager service
    nsCOMPtr<nsICacheService> cacheService = 
             do_GetService(NS_CACHESERVICE_CONTRACTID, &rv);
    if (NS_FAILED(rv)) return rv;

    nsCOMPtr<nsIStorageStream> storageStream;
    nsCOMPtr<nsIOutputStream> outputStream;

    // Init: (block size, maximum length)
    rv = NS_NewStorageStream(256, (PRUint32)-1, getter_AddRefs(storageStream));
    if (NS_FAILED(rv)) return rv;

    rv = storageStream->GetOutputStream(0, getter_AddRefs(outputStream));
    if (NS_FAILED(rv)) return rv;

    mBuffer.AssignLiteral(
                   "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                   "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.1//EN\"\n"
                   "    \"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd\">\n"
                   "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n"
                   "<head>\n<title>Information about the Cache Service</title>\n</head>\n"
                   "<body>\n<div>\n");

    outputStream->Write(mBuffer.get(), mBuffer.Length(), &bytesWritten);

    rv = ParseURI(aURI, mDeviceID);
    if (NS_FAILED(rv)) return rv;

    mStream = outputStream;
    rv = cacheService->VisitEntries(this);
    mBuffer.Truncate();
    if (rv == NS_ERROR_NOT_AVAILABLE) {
        mBuffer.AppendLiteral("<h2>The cache is disabled.</h2>\n");
    }
    else if (NS_FAILED(rv)) {
        return rv;
    }

    if (!mDeviceID.IsEmpty()) {
        mBuffer.AppendLiteral("</pre>\n");
    }
    mBuffer.AppendLiteral("</div>\n</body>\n</html>\n");
    outputStream->Write(mBuffer.get(), mBuffer.Length(), &bytesWritten);
        
    nsCOMPtr<nsIInputStream> inStr;

    rv = storageStream->NewInputStream(0, getter_AddRefs(inStr));
    if (NS_FAILED(rv)) return rv;

    nsIChannel* channel;
    rv = NS_NewInputStreamChannel(&channel, aURI, inStr,
                                  NS_LITERAL_CSTRING("text/html"),
                                  NS_LITERAL_CSTRING("utf-8"));
    if (NS_FAILED(rv)) return rv;

    *result = channel;
    return rv;
}

NS_IMETHODIMP
nsAboutCache::GetURIFlags(nsIURI *aURI, PRUint32 *result)
{
    *result = 0;
    return NS_OK;
}

NS_IMETHODIMP
nsAboutCache::VisitDevice(const char *deviceID,
                          nsICacheDeviceInfo *deviceInfo,
                          PRBool *visitEntries)
{
    PRUint32 bytesWritten, value;
    nsXPIDLCString str;

    *visitEntries = PR_FALSE;

    if (mDeviceID.IsEmpty() || mDeviceID.Equals(deviceID)) {

        // We need mStream for this
        if (!mStream)
          return NS_ERROR_FAILURE;
	
        // Write out the Cache Name
        deviceInfo->GetDescription(getter_Copies(str));

        mBuffer.AssignLiteral("<h2>");
        mBuffer.Append(str);
        mBuffer.AppendLiteral("</h2>\n<br />\n"
                              "<table>\n");

        // Write out cache info

        mBuffer.AppendLiteral("\n<tr>\n<td><b>Number of entries:</b></td>\n");
        value = 0;
        deviceInfo->GetEntryCount(&value);
        mBuffer.AppendLiteral("<td><tt>");
        mBuffer.AppendInt(value);
        mBuffer.AppendLiteral("</tt></td>\n</tr>\n"
                              "\n<tr>\n<td><b>Maximum storage size:</b></td>\n");
        value = 0;
        deviceInfo->GetMaximumSize(&value);
        mBuffer.AppendLiteral("<td><tt>");
        mBuffer.AppendInt(value/1024);
        mBuffer.AppendLiteral(" KiB</tt></td>\n</tr>\n"
                              "\n<tr>\n<td><b>Storage in use:</b></td>\n"
                              "<td><tt>");
        value = 0;
        deviceInfo->GetTotalSize(&value);
        mBuffer.AppendInt(value/1024);
        mBuffer.AppendLiteral(" KiB</tt></td>\n</tr>\n");

        deviceInfo->GetUsageReport(getter_Copies(str));
        mBuffer.Append(str);
        mBuffer.AppendLiteral("</table>\n\n<br />");

        if (mDeviceID.IsEmpty()) {
            mBuffer.AppendLiteral("\n<a href=\"about:cache?device=");
            mBuffer.Append(deviceID);
            mBuffer.AppendLiteral("\">List Cache Entries</a>\n"
                                  "<hr />\n");
        } else {
            *visitEntries = PR_TRUE;
            mBuffer.AppendLiteral("<hr />\n<pre>\n");
        }
        
        mStream->Write(mBuffer.get(), mBuffer.Length(), &bytesWritten);
    }

    return NS_OK;
}

NS_IMETHODIMP
nsAboutCache::VisitEntry(const char *deviceID,
                         nsICacheEntryInfo *entryInfo,
                         PRBool *visitNext)
{
    // We need mStream for this
    if (!mStream)
      return NS_ERROR_FAILURE;

    nsresult        rv;
    PRUint32        bytesWritten;
    nsCAutoString   key;
    nsXPIDLCString  clientID;
    PRBool          streamBased;
    
    rv = entryInfo->GetKey(key);
    if (NS_FAILED(rv))  return rv;

    rv = entryInfo->GetClientID(getter_Copies(clientID));
    if (NS_FAILED(rv))  return rv;

    rv = entryInfo->IsStreamBased(&streamBased);
    if (NS_FAILED(rv)) return rv;

    // Generate a about:cache-entry URL for this entry...
    nsCAutoString url;
    url.AssignLiteral("about:cache-entry?client=");
    url += clientID;
    url.AppendLiteral("&amp;sb=");
    url += streamBased ? '1' : '0';
    url.AppendLiteral("&amp;key=");
    char* escapedKey = nsEscapeHTML(key.get());
    url += escapedKey; // key

    // Entry start...

    // URI
    mBuffer.AssignLiteral("<b>           Key:</b> <a href=\"");
    mBuffer.Append(url);
    mBuffer.AppendLiteral("\">");
    mBuffer.Append(escapedKey);
    nsMemory::Free(escapedKey);
    mBuffer.AppendLiteral("</a>");

    // Content length
    PRUint32 length = 0;
    entryInfo->GetDataSize(&length);

    mBuffer.AppendLiteral("\n<b>     Data size:</b> ");
    mBuffer.AppendInt(length);
    mBuffer.AppendLiteral(" bytes");

    // Number of accesses
    PRInt32 fetchCount = 0;
    entryInfo->GetFetchCount(&fetchCount);

    mBuffer.AppendLiteral("\n<b>   Fetch count:</b> ");
    mBuffer.AppendInt(fetchCount);

    // vars for reporting time
    char buf[255];
    PRUint32 t;

    // Last modified time
    mBuffer.AppendLiteral("\n<b> Last modified:</b> ");
    entryInfo->GetLastModified(&t);
    if (t) {
        PrintTimeString(buf, sizeof(buf), t);
        mBuffer.Append(buf);
    } else
        mBuffer.AppendLiteral("No last modified time");

    // Expires time
    mBuffer.AppendLiteral("\n<b>       Expires:</b> ");
    entryInfo->GetExpirationTime(&t);
    if (t < 0xFFFFFFFF) {
        PrintTimeString(buf, sizeof(buf), t);
        mBuffer.Append(buf);
    } else {
        mBuffer.AppendLiteral("No expiration time");
    }

    // Entry is done...
    mBuffer.AppendLiteral("\n\n");

    mStream->Write(mBuffer.get(), mBuffer.Length(), &bytesWritten);

    *visitNext = PR_TRUE;
    return NS_OK;
}


nsresult
nsAboutCache::ParseURI(nsIURI * uri, nsCString &deviceID)
{
    //
    // about:cache[?device=string]
    //
    nsresult rv;

    deviceID.Truncate();

    nsCAutoString path;
    rv = uri->GetPath(path);
    if (NS_FAILED(rv)) return rv;

    nsACString::const_iterator start, valueStart, end;
    path.BeginReading(start);
    path.EndReading(end);

    valueStart = end;
    if (!FindInReadable(NS_LITERAL_CSTRING("?device="), start, valueStart))
        return NS_OK;

    deviceID.Assign(Substring(valueStart, end));
    return NS_OK;
}


NS_METHOD
nsAboutCache::Create(nsISupports *aOuter, REFNSIID aIID, void **aResult)
{
    nsAboutCache* about = new nsAboutCache();
    if (about == nsnull)
        return NS_ERROR_OUT_OF_MEMORY;
    NS_ADDREF(about);
    nsresult rv = about->QueryInterface(aIID, aResult);
    NS_RELEASE(about);
    return rv;
}



////////////////////////////////////////////////////////////////////////////////
