/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
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
 *   Jungshik Shin <jshin@mailaps.org>
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

#include "nsCP949ToUnicode.h"
#include "nsUCvKODll.h"
#include "nsUCConstructors.h"

//----------------------------------------------------------------------
// Global functions and data [declaration]

static const uScanClassID g_CP949ScanClassIDs[] = {
  u1ByteCharset,
// This is necessary to decode 8byte sequence representation of Hangul
// syllables. This representation is uniq to EUC-KR and is not used
// in CP949. However, this converter is for both EUC-KR and CP949
// so that this class ID is put here. See bug 131388. 
  uDecomposedHangulCharset,  
  u2BytesGRCharset,       // EUC_KR
  u2BytesGR128Charset,    // CP949 High
  u2BytesCharset          // CP949 Low
};

// CP949(non-EUC-KR portion) to Unicode
static const PRUint16 g_utCP949NoKSCHangulMapping[] = {
#include "u20cp949hangul.ut"
};

static const uRange g_CP949Ranges[] = {
  { 0x00, 0x7E },
  { 0xA4, 0xA4 },   // 8byte seq. for Hangul syllables not available
                    // in pre-composed form in KS X 1001
  { 0xA1, 0xFE },
  { 0xA1, 0xC6 },   // CP949 extension B. ends at 0xC6.
  { 0x80, 0xA0 }
};

static const PRUint16 *g_CP949MappingTableSet [] ={
  g_ucvko_AsciiMapping,
  g_HangulNullMapping,
  g_utKSC5601Mapping,
  g_utCP949NoKSCHangulMapping,
  g_utCP949NoKSCHangulMapping
//g_CP949HighMapping,
//g_CP949LowMapping
};


NS_METHOD
nsCP949ToUnicodeConstructor(nsISupports *aOuter, REFNSIID aIID,
                            void **aResult)
{
  return CreateMultiTableDecoder(sizeof(g_CP949Ranges) / sizeof(g_CP949Ranges[0]),
                                 (const uRange*) &g_CP949Ranges,
                                 (uScanClassID*) &g_CP949ScanClassIDs,
                                 (uMappingTable**) &g_CP949MappingTableSet, 1,
                                 aOuter, aIID, aResult);
}

