/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * ***** BEGIN LICENSE BLOCK *****
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
 * The Original Code is part of the mozilla.org LDAP XPCOM SDK.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2001
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Xiaobin Lu <Xiaobin.Lu@eng.Sun.com>
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

#include "nsCNullSecurityContext.h"
#include "nsString.h"

NS_IMPL_ISUPPORTS1(nsCNullSecurityContext, nsISecurityContext)

NS_IMETHODIMP nsCNullSecurityContext::Implies(const char* target, const char* action, PRBool* bActionAllowed)
{    
    /* Avoid security check, returns true always */
    *bActionAllowed = PR_TRUE;
    return NS_OK;	
}
   
NS_IMETHODIMP nsCNullSecurityContext::GetOrigin(char* buf, int len)
{
    if (buf == NULL)
       return NS_ERROR_NULL_POINTER;
    
    const char origin[] = "file:///";
    PRInt32 originLen = (PRInt32) (sizeof(origin) - 1);
    if (len <= originLen) {
        return NS_ERROR_NULL_POINTER;
    }
    // Copy origin
    memcpy(buf, origin, originLen);
    return NS_OK;
}

NS_IMETHODIMP nsCNullSecurityContext::GetCertificateID(char* buf, int len)
{
   return NS_ERROR_NOT_IMPLEMENTED;
}
