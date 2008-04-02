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

#include "nsUnicodeToEUCJP.h"
#include "nsUCVJADll.h"
#include "nsUCConstructors.h"

//----------------------------------------------------------------------
// Global functions and data [declaration]

// Shift Table
static const PRInt16 g0201ShiftOutTable[] =  {
        2,
        ShiftOutCell(u1ByteChar,         1, 0x00, 0x00, 0x00, 0x7F),
        ShiftOutCell(u1BytePrefix8EChar, 2, 0x00, 0xA1, 0x00, 0xDF)
};

#define SIZE_OF_TABLES 4
static const uScanClassID gScanClassIDs[SIZE_OF_TABLES] = {
  u2BytesGRCharset,
  u2BytesGRCharset,
  uMultibytesCharset,
  u2BytesGRPrefix8FCharset
};

static const PRInt16 *gShiftTables[SIZE_OF_TABLES] =  {
    0,
    0,
    g0201ShiftOutTable,
    0
};

static const PRUint16 *gMappingTables[SIZE_OF_TABLES] = {
    g_uf0208Mapping,
    g_uf0208extMapping,
    g_uf0201Mapping,
    g_uf0212Mapping
};

NS_METHOD
nsUnicodeToEUCJPConstructor(nsISupports *aOuter, REFNSIID aIID,
                            void **aResult)
{
    return CreateMultiTableEncoder(SIZE_OF_TABLES,
                                   (uScanClassID*) gScanClassIDs,
                                   (uShiftOutTable**) gShiftTables, 
                                   (uMappingTable**) gMappingTables,
                                   3 /* max length = src * 3 */,
                                   aOuter, aIID, aResult);
}

