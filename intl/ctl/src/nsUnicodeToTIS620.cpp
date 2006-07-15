/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * ucvth : nsUnicodeToTIS620.h
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
 * The Original Code is mozilla.org code. The Initial Developer of the Original Code is Sun Microsystems, Inc.  Portions created by SUN are Copyright (C) 2000 SUN Microsystems, Inc. All Rights Reserved.
 *
 * The Initial Developer of the Original Code is
 * Sun Microsystems, Inc.
 * Portions created by the Initial Developer are Copyright (C) 2000
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Prabhat Hegde (prabhat.hegde@sun.com)
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
#include "nsCOMPtr.h"
#include "nsIServiceManager.h"
#include "nsICharsetConverterManager.h"
#include "nsILanguageAtomService.h"
#include "nsCtlCIID.h"
#include "nsILE.h"
#include "nsULE.h"
#include "nsUnicodeToTIS620.h"

static NS_DEFINE_CID(kCharSetManagerCID, NS_ICHARSETCONVERTERMANAGER_CID);

NS_IMPL_ISUPPORTS2(nsUnicodeToTIS620, nsIUnicodeEncoder, nsICharRepresentable)

NS_IMETHODIMP nsUnicodeToTIS620::SetOutputErrorBehavior(PRInt32 aBehavior,
                                                        nsIUnicharEncoder * aEncoder, 
                                                        PRUnichar aChar)
{
  return NS_ERROR_NOT_IMPLEMENTED;
}

nsUnicodeToTIS620::nsUnicodeToTIS620()
{
  static   NS_DEFINE_CID(kLECID, NS_ULE_CID);
  nsresult rv;

  mCtlObj = do_CreateInstance(kLECID, &rv);
  if (NS_FAILED(rv)) {
#ifdef DEBUG_prabhath
    // No other error handling needed here since we
    // handle absence of mCtlObj in Convert
    printf("ERROR: Cannot create instance of component " NS_ULE_PROGID " [%x].\n", rv);
#endif
    NS_WARNING("Thai Text Layout Will Not Be Supported\n");
    mCtlObj = nsnull;
  }
}

nsUnicodeToTIS620::~nsUnicodeToTIS620()
{
  // Maybe convert nsILE to a service
  // No NS_IF_RELEASE(mCtlObj) of nsCOMPtr;
}

/*
 * This method converts the unicode to this font index.
 * Note: ConversionBufferFullException is not handled
 *       since this class is only used for character display.
 */
NS_IMETHODIMP nsUnicodeToTIS620::Convert(const PRUnichar* input,
                                         PRInt32*         aSrcLength,
                                         char*            output,
                                         PRInt32*         aDestLength)
{
  PRSize outLen = 0;
#ifdef DEBUG_prabhath_no_shaper
  printf("Debug/Test Case of No thai pango shaper Object\n");
  // Comment out mCtlObj == nsnull for test purposes
#endif

  if (mCtlObj == nsnull) {
#ifdef DEBUG_prabhath
    printf("ERROR: No CTL IMPLEMENTATION - Default Thai Conversion");
    // CP874 is the default converter for thai ; 
    // In case mCtlObj is absent (no CTL support), use it to convert.
#endif

    nsCOMPtr<nsICharsetConverterManager> charsetMgr =
        do_GetService(kCharSetManagerCID);
    if (!charsetMgr)
      return NS_ERROR_FAILURE;
    
    nsCOMPtr<nsIUnicodeEncoder> encoder;
    charsetMgr->GetUnicodeEncoderRaw("TIS-620", getter_AddRefs(encoder));
    if (!encoder) {
      NS_WARNING("cannot get default converter for tis-620");
      return NS_ERROR_FAILURE;
    }

    encoder->Convert(input, aSrcLength, output, aDestLength);
    return NS_OK;
  }

  // CTLized shaping conversion starts here
  // No question of starting the conversion from an offset
  mCharOff = mByteOff = 0;

  // Charset tis620-0, tis620.2533-1, tis620.2529-1 & iso8859-11
  // are equivalent and have the same presentation forms

  // tis620-2 is hard-coded since we only generate presentation forms
  // in Windows-Stye as it is the current defacto-standard for the
  // presentation of thai content
  mCtlObj->GetPresentationForm(input, *aSrcLength, "tis620-2",
                               &output[mByteOff], &outLen);

  *aDestLength = outLen;
  return NS_OK;
}

NS_IMETHODIMP nsUnicodeToTIS620::Finish(char * output, PRInt32 * aDestLength)
{
  // Finish does'nt have to do much as Convert already flushes
  // to output buffer
  mByteOff = mCharOff = 0;
  return NS_OK;
}

//================================================================
NS_IMETHODIMP nsUnicodeToTIS620::Reset()
{
  mByteOff = mCharOff = 0;
  return NS_OK;
}

//================================================================
NS_IMETHODIMP nsUnicodeToTIS620::GetMaxLength(const PRUnichar * aSrc,
                                              PRInt32 aSrcLength,
                                              PRInt32 * aDestLength)
{
  *aDestLength = (aSrcLength + 1) * 2; // Each Thai character can generate
                                        // atmost two presentation forms
  return NS_OK;
}

//================================================================
NS_IMETHODIMP nsUnicodeToTIS620::FillInfo(PRUint32* aInfo)
{
  PRUint16 i;

  // 00-0x7f
  for (i = 0;i <= 0x7f; i++)
    SET_REPRESENTABLE(aInfo, i);

  // 0x0e01-0x0e7f
  for (i = 0x0e01; i <= 0xe3a; i++)
    SET_REPRESENTABLE(aInfo, i);

  // U+0E3B - U+0E3E is undefined
  // U+0E3F - U+0E5B
  for (i = 0x0e3f; i <= 0x0e5b; i++)
    SET_REPRESENTABLE(aInfo, i);

  // U+0E5C - U+0E7F Undefined
  return NS_OK;
}
