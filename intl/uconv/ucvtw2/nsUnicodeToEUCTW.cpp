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

#include "nsUnicodeToEUCTW.h"
#include "nsUCvTW2Dll.h"
#include "nsUCConstructors.h"

//----------------------------------------------------------------------
// Global functions and data [declaration]

static const uScanClassID g_EUCTWScanClassSet [] = {
  u1ByteCharset,
  u2BytesGRCharset,
  u2BytesGRPrefix8EA2Charset,
  u2BytesGRPrefix8EA3Charset,
  u2BytesGRPrefix8EA4Charset,
  u2BytesGRPrefix8EA5Charset,
  u2BytesGRPrefix8EA6Charset,
  u2BytesGRPrefix8EA7Charset
};

static const PRUint16 *g_EUCTWMappingTableSet [] ={
  g_ASCIIMappingTable,
  g_ufCNS1MappingTable,
  g_ufCNS2MappingTable,
  g_ufCNS3MappingTable,
  g_ufCNS4MappingTable,
  g_ufCNS5MappingTable,
  g_ufCNS6MappingTable,
  g_ufCNS7MappingTable
};

//----------------------------------------------------------------------
// Class nsUnicodeToEUCTW [implementation]

NS_METHOD
nsUnicodeToEUCTWConstructor(nsISupports *aOuter, REFNSIID aIID,
                            void **aResult)
{
  return CreateMultiTableEncoder(8,
                                 (uScanClassID*) &g_EUCTWScanClassSet,
                                 (uMappingTable**) &g_EUCTWMappingTableSet,
                                 4 /* max length = src * 4 */,
                                 aOuter, aIID, aResult);
}

