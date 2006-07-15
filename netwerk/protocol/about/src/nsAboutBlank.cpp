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

#include "nsAboutBlank.h"
#include "nsIIOService.h"
#include "nsIServiceManager.h"
#include "nsStringStream.h"
#include "nsNetUtil.h"

NS_IMPL_ISUPPORTS1(nsAboutBlank, nsIAboutModule)

static const char kBlankPage[] = "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">"
"<html><head><title></title></head><body></body></html>";

NS_IMETHODIMP
nsAboutBlank::NewChannel(nsIURI *aURI, nsIChannel **result)
{
    NS_ENSURE_ARG_POINTER(aURI);
    nsresult rv;
    nsIChannel* channel;

    nsCOMPtr<nsIInputStream> in;
    rv = NS_NewCStringInputStream(getter_AddRefs(in), NS_LITERAL_CSTRING(kBlankPage));
    if (NS_FAILED(rv)) return rv;

    rv = NS_NewInputStreamChannel(&channel, aURI, in,
                                  NS_LITERAL_CSTRING("text/html"),
                                  NS_LITERAL_CSTRING("utf-8"));
    if (NS_FAILED(rv)) return rv;

    *result = channel;
    return rv;
}

NS_IMETHODIMP
nsAboutBlank::GetURIFlags(nsIURI *aURI, PRUint32 *result)
{
    *result = nsIAboutModule::URI_SAFE_FOR_UNTRUSTED_CONTENT;
    return NS_OK;
}

NS_METHOD
nsAboutBlank::Create(nsISupports *aOuter, REFNSIID aIID, void **aResult)
{
    nsAboutBlank* about = new nsAboutBlank();
    if (about == nsnull)
        return NS_ERROR_OUT_OF_MEMORY;
    NS_ADDREF(about);
    nsresult rv = about->QueryInterface(aIID, aResult);
    NS_RELEASE(about);
    return rv;
}

////////////////////////////////////////////////////////////////////////////////
