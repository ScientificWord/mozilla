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
 * The Original Code is the mozilla.org LDAP XPCOM SDK.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2002
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Dan Mosedale <dmose@netscape.com>
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

#ifndef _nsLDAPBERValue_h_
#define _nsLDAPBERValue_h_

#include "ldap.h"
#include "nsILDAPBERValue.h"

// 7c9fa10e-1dd2-11b2-a097-ac379e6803b2
//
#define NS_LDAPBERVALUE_CID \
{ 0x7c9fa10e, 0x1dd2, 0x11b2, \
  {0xa0, 0x97, 0xac, 0x37, 0x9e, 0x68, 0x03, 0xb2 }}

class nsLDAPBERValue : public nsILDAPBERValue
{
public:
    NS_DECL_ISUPPORTS
    NS_DECL_NSILDAPBERVALUE

    nsLDAPBERValue();
    virtual ~nsLDAPBERValue();
    
protected:

    /** 
     * nsLDAPControl needs to be able to grovel through this without an
     * an extra copy
     */
    friend class nsLDAPControl;

    PRUint8 *mValue;    // pointer to an array
    PRUint32 mSize;	    // size of the value, in bytes
};

#endif // _nsLDAPBERValue_h_
