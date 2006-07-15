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
 * The Original Code is Mozilla.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications.
 * Portions created by the Initial Developer are Copyright (C) 2001
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Darin Fisher <darin@netscape.com> (original author)
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

#ifndef nsHttpHeaderArray_h__
#define nsHttpHeaderArray_h__

#include "nsTArray.h"
#include "nsIHttpChannel.h"
#include "nsIHttpHeaderVisitor.h"
#include "nsCOMPtr.h"
#include "nsString.h"
#include "nsHttp.h"

class nsHttpHeaderArray
{
public:
    nsHttpHeaderArray() {}
   ~nsHttpHeaderArray() { Clear(); }

    const char *PeekHeader(nsHttpAtom header);

    nsresult SetHeader(nsHttpAtom header, const nsACString &value, PRBool merge = PR_FALSE);
    nsresult GetHeader(nsHttpAtom header, nsACString &value);
    void     ClearHeader(nsHttpAtom h);

    // Find the location of the given header value, or null if none exists.
    const char *FindHeaderValue(nsHttpAtom header, const char *value) {
        return nsHttp::FindToken(PeekHeader(header), value,
                                 HTTP_HEADER_VALUE_SEPS);
    }

    // Determine if the given header value exists.
    PRBool HasHeaderValue(nsHttpAtom header, const char *value) {
        return FindHeaderValue(header, value) != nsnull;
    }

    nsresult VisitHeaders(nsIHttpHeaderVisitor *visitor);

    // parse a header line, return the header atom and a pointer to the 
    // header value (the substring of the header line -- do not free).
    void ParseHeaderLine(char *line, nsHttpAtom *header=nsnull, char **value=nsnull);

    void Flatten(nsACString &, PRBool pruneProxyHeaders=PR_FALSE);

    PRUint32 Count() { return mHeaders.Length(); }

    const char *PeekHeaderAt(PRUint32 i, nsHttpAtom &header);

    void Clear();

private:
    struct nsEntry
    {
        nsEntry() {}

        nsHttpAtom header;
        nsCString  value;

        struct MatchHeader {
          PRBool Equals(const nsEntry &entry, const nsHttpAtom &header) const {
            return entry.header == header;
          }
        };
    };

    PRInt32 LookupEntry(nsHttpAtom header, nsEntry **);
    PRBool  CanAppendToHeader(nsHttpAtom header);

private:
    nsTArray<nsEntry> mHeaders;
};

#endif
