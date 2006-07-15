/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
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
 * The Original Code is Mozilla Communicator client code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
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

#ifndef nsISO88591ToUnicode_h___
#define nsISO88591ToUnicode_h___

#include "nsISupports.h"

// Class ID for our ISO88591ToUnicode charset converter
// {A3254CB0-8E20-11d2-8A98-00600811A836}
#define NS_ISO88591TOUNICODE_CID \
  { 0xa3254cb0, 0x8e20, 0x11d2, {0x8a, 0x98, 0x0, 0x60, 0x8, 0x11, 0xa8, 0x36}}

#define NS_ISO88591TOUNICODE_CONTRACTID "@mozilla.org/intl/unicode/decoder;1?charset=ISO-8859-1"

/**
 * A character set converter from ISO88591 to Unicode.
 *
 * @created         23/Nov/1998
 * @author  Catalin Rotaru [CATA]
 */
NS_METHOD
nsISO88591ToUnicodeConstructor(nsISupports *aOuter, REFNSIID aIID,
                               void **aResult);

#endif /* nsISO88591ToUnicode_h___ */
