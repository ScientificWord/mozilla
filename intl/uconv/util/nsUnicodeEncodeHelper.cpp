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

#include "pratom.h"
#include "unicpriv.h"
#include "nsIUnicodeEncoder.h"
#include "nsUnicodeEncodeHelper.h"

//----------------------------------------------------------------------
// Class nsUnicodeEncodeHelper [implementation]
nsresult nsUnicodeEncodeHelper::ConvertByTable(
                                     const PRUnichar * aSrc, 
                                     PRInt32 * aSrcLength, 
                                     char * aDest, 
                                     PRInt32 * aDestLength, 
                                     uShiftTable * aShiftTable, 
                                     uMappingTable  * aMappingTable)
{
  const PRUnichar * src = aSrc;
  const PRUnichar * srcEnd = aSrc + *aSrcLength;
  char * dest = aDest;
  PRInt32 destLen = *aDestLength;

  PRUnichar med;
  PRInt32 bcw; // byte count for write;
  nsresult res = NS_OK;

  while (src < srcEnd) {
    if (!uMapCode((uTable*) aMappingTable, NS_STATIC_CAST(PRUnichar, *(src++)), NS_REINTERPRET_CAST(PRUint16*, &med))) {
      res = NS_ERROR_UENC_NOMAPPING;
      break;
    }

    if (!uGenerate(aShiftTable, 0, med, (PRUint8 *)dest, destLen, 
      (PRUint32 *)&bcw)) { 
      src--;
      res = NS_OK_UENC_MOREOUTPUT;
      break;
    }

    dest += bcw;
    destLen -= bcw;
  }

  *aSrcLength = src - aSrc;
  *aDestLength  = dest - aDest;
  return res;
}

nsresult nsUnicodeEncodeHelper::ConvertByMultiTable(
                                     const PRUnichar * aSrc, 
                                     PRInt32 * aSrcLength, 
                                     char * aDest, 
                                     PRInt32 * aDestLength, 
                                     PRInt32 aTableCount, 
                                     uShiftTable ** aShiftTable, 
                                     uMappingTable  ** aMappingTable)
{
  const PRUnichar * src = aSrc;
  const PRUnichar * srcEnd = aSrc + *aSrcLength;
  char * dest = aDest;
  PRInt32 destLen = *aDestLength;

  PRUnichar med;
  PRInt32 bcw; // byte count for write;
  nsresult res = NS_OK;
  PRInt32 i;

  while (src < srcEnd) {
    for (i=0; i<aTableCount; i++) 
      if (uMapCode((uTable*) aMappingTable[i], NS_STATIC_CAST(PRUint16, *src), NS_REINTERPRET_CAST(PRUint16*, &med))) break;

    src++;
    if (i == aTableCount) {
      res = NS_ERROR_UENC_NOMAPPING;
      break;
    }

    if (!uGenerate(aShiftTable[i], 0, med, (PRUint8 *)dest, destLen, 
      (PRUint32 *)&bcw)) { 
      src--;
      res = NS_OK_UENC_MOREOUTPUT;
      break;
    }

    dest += bcw;
    destLen -= bcw;
  }

  *aSrcLength = src - aSrc;
  *aDestLength  = dest - aDest;
  return res;
}

nsresult nsUnicodeEncodeHelper::FillInfo(PRUint32 *aInfo, uMappingTable  * aMappingTable)
{
   uFillInfo((uTable*) aMappingTable, aInfo);
   return NS_OK;
}

nsresult nsUnicodeEncodeHelper::FillInfo(PRUint32 *aInfo, PRInt32 aTableCount, uMappingTable  ** aMappingTable)
{
   for (PRInt32 i=0; i<aTableCount; i++) 
      uFillInfo((uTable*) aMappingTable[i], aInfo);
   return NS_OK;
}
