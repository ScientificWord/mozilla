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

#include "nsUnicodeToISO2022JP.h"
#include "nsIComponentManager.h"
#include "nsUCVJADll.h"
#include "nsUnicodeEncodeHelper.h"

//----------------------------------------------------------------------
// Global functions and data [declaration]

static const PRUint16 g_ufAsciiMapping [] = {
  0x0001, 0x0004, 0x0005, 0x0008, 0x0000, 0x0000, 0x007F, 0x0000
};

#define SIZE_OF_TABLES 5
static const PRUint16 * g_ufMappingTables[SIZE_OF_TABLES] = {
  g_ufAsciiMapping,             // ASCII           ISOREG 6
  g_uf0201GLMapping,            // JIS X 0201-1976 ISOREG 14
  g_uf0208Mapping,              // JIS X 0208-1983 ISOREG 87
  g_uf0208extMapping,           // JIS X 0208 - cp932 ext
  g_uf0208Mapping,              // JIS X 0208-1978 ISOREG 42
};

static const uScanClassID g_ufScanClassIDs[SIZE_OF_TABLES] = {
  u1ByteCharset,                // ASCII           ISOREG 6
  u1ByteCharset,                // JIS X 0201-1976 ISOREG 14
  u2BytesCharset,               // JIS X 0208-1983 ISOREG 87
  u2BytesCharset,               // JIS X 0208- cp932 ext
  u2BytesCharset,               // JIS X 0208-1978 ISOREG 42
};

//----------------------------------------------------------------------
// Class nsUnicodeToISO2022JP [implementation]

// worst case max length: 
//  1  2 3  4  5  6  7 8
// ESC $ B XX XX ESC ( B
nsUnicodeToISO2022JP::nsUnicodeToISO2022JP() 
: nsEncoderSupport(8)
{
  Reset();
}

nsUnicodeToISO2022JP::~nsUnicodeToISO2022JP() 
{
}

nsresult nsUnicodeToISO2022JP::ChangeCharset(PRInt32 aCharset,
                                             char * aDest, 
                                             PRInt32 * aDestLength)
{
  // both 2 and 3 generate the same escape sequence. 2 is for
  // the standard JISx0208 table, and 3 is for theCP932 extensions
  // therefore, we treat them as the same one.
  if(((2 == aCharset) && ( 3 == mCharset)) ||
     ((3 == aCharset) && ( 2 == mCharset)) )
  {
    mCharset = aCharset;
  }

  if(aCharset == mCharset) 
  {
    *aDestLength = 0;
    return NS_OK;
  } 
  
  if (*aDestLength < 3) {
    *aDestLength = 0;
    return NS_OK_UENC_MOREOUTPUT;
  }

  switch (aCharset) {
    case 0: // ASCII ISOREG 6
      aDest[0] = 0x1b;
      aDest[1] = '(';
      aDest[2] = 'B';
      break;
    case 1: // JIS X 0201-1976 ("Roman" set) ISOREG 14
      aDest[0] = 0x1b;
      aDest[1] = '(';
      aDest[2] = 'J';
      break;
    case 2: // JIS X 0208-1983 ISOREG 87
    case 3: // JIS X 0208-1983 
            // we currently use this for CP932 ext
      aDest[0] = 0x1b;
      aDest[1] = '$';
      aDest[2] = 'B';
      break;
    case 4: // JIS X 0201-1978 ISOREG 87- 
            // we currently do not have a diff mapping for it.
      aDest[0] = 0x1b;
      aDest[1] = '$';
      aDest[2] = '@';
      break;
  }

  mCharset = aCharset;
  *aDestLength = 3;
  return NS_OK;
}

//----------------------------------------------------------------------
// Subclassing of nsTableEncoderSupport class [implementation]

NS_IMETHODIMP nsUnicodeToISO2022JP::FillInfo(PRUint32* aInfo)
{
  return nsUnicodeEncodeHelper::FillInfo(aInfo, SIZE_OF_TABLES, 
                                         (uMappingTable **) g_ufMappingTables);

}
NS_IMETHODIMP nsUnicodeToISO2022JP::ConvertNoBuffNoErr(
                                    const PRUnichar * aSrc, 
                                    PRInt32 * aSrcLength, 
                                    char * aDest, 
                                    PRInt32 * aDestLength)
{
  nsresult res = NS_OK;

  const PRUnichar * src = aSrc;
  const PRUnichar * srcEnd = aSrc + *aSrcLength;
  char * dest = aDest;
  char * destEnd = aDest + *aDestLength;
  PRInt32 bcr, bcw;
  PRInt32 i;

  while (src < srcEnd) {
    for (i=0; i< SIZE_OF_TABLES ; i++) {
      bcr = 1;
      bcw = destEnd - dest;
      res = nsUnicodeEncodeHelper::ConvertByTable(src, &bcr, dest, &bcw, 
                                      g_ufScanClassIDs[i], nsnull,
                                      (uMappingTable *) g_ufMappingTables[i]);
      if (res != NS_ERROR_UENC_NOMAPPING) break;
    }

    if ( i>=  SIZE_OF_TABLES) {
      res = NS_ERROR_UENC_NOMAPPING;
      src++;
    }
    if (res != NS_OK) break;

    bcw = destEnd - dest;
    res = ChangeCharset(i, dest, &bcw);
    dest += bcw;
    if (res != NS_OK) break;

    bcr = srcEnd - src;
    bcw = destEnd - dest;
    res = nsUnicodeEncodeHelper::ConvertByTable(src, &bcr, dest, &bcw, 
                                      g_ufScanClassIDs[i], nsnull,
                                      (uMappingTable *) g_ufMappingTables[i]);
    src += bcr;
    dest += bcw;

    if ((res != NS_OK) && (res != NS_ERROR_UENC_NOMAPPING)) break;
    if (res == NS_ERROR_UENC_NOMAPPING) src--;
  }

  *aSrcLength = src - aSrc;
  *aDestLength  = dest - aDest;
  return res;
}

NS_IMETHODIMP nsUnicodeToISO2022JP::FinishNoBuff(char * aDest, 
                                                 PRInt32 * aDestLength)
{
  ChangeCharset(0, aDest, aDestLength);
  return NS_OK;
}

NS_IMETHODIMP nsUnicodeToISO2022JP::Reset()
{
  mCharset = 0;
  return nsEncoderSupport::Reset();
}
