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
 *   Ervin Yan <Ervin.Yan@Sun.Com>
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
#ifndef nsISO2022CNToUnicode_h__
#define nsISO2022CNToUnicode_h__
#include "nsCOMPtr.h"
#include "nsISupports.h"
#include "nsUCSupport.h"

#define MBYTE       0x8e
#undef PMASK
#define PMASK       0xa0

#define SI          0x0f 
#define SO          0x0e
#define ESC         0x1b
#define SS2         0x4e
#define SS3         0x4f

class nsISO2022CNToUnicode : public nsBasicDecoderSupport
{
public:
  nsISO2022CNToUnicode() : 
        mState(eState_ASCII), 
        mPlaneID(0),
        mRunLength(0) { }

  virtual ~nsISO2022CNToUnicode() {}

  NS_IMETHOD Convert(const char *aSrc, PRInt32 * aSrcLength,
     PRUnichar * aDest, PRInt32 * aDestLength) ;

  NS_IMETHOD GetMaxLength(const char * aSrc, PRInt32 aSrcLength,
     PRInt32 * aDestLength)
  {
    *aDestLength = aSrcLength;
    return NS_OK;
  }

  NS_IMETHOD Reset()
  {
    mState = eState_ASCII;
    mPlaneID = 0;
    mRunLength = 0;

    return NS_OK;
  }

private:
  // State Machine ID
  enum {
    eState_ASCII,
    eState_ESC,                           // ESC
    eState_ESC_24,                        // ESC $

    eState_ESC_24_29,                     // ESC $ )
    eState_ESC_24_29_A,                   // ESC $ ) A
    eState_GB2312_1980,                   // ESC $ ) A SO
    eState_GB2312_1980_2ndbyte,           // ESC $ ) A SO
    eState_ESC_24_29_A_SO_SI,             // ESC $ ) A SO SI
    eState_ESC_24_29_G,                   // ESC $ ) G or H
    eState_CNS11643_1,                    // ESC $ ) G SO
    eState_CNS11643_1_2ndbyte,            // ESC $ ) G SO
    eState_ESC_24_29_G_SO_SI,             // ESC $ ) G SO SI

    eState_ESC_24_2A,                     // ESC $ *
    eState_ESC_24_2A_H,                   // ESC $ * H
    eState_ESC_24_2A_H_ESC,               // ESC $ * H ESC
    eState_CNS11643_2,                    // ESC $ * H ESC SS2
    eState_CNS11643_2_2ndbyte,            // ESC $ * H ESC SS2
    eState_ESC_24_2A_H_ESC_SS2_SI,        // ESC $ * H ESC SS2 SI
    eState_ESC_24_2A_H_ESC_SS2_SI_ESC,    // ESC $ * H ESC SS2 SI ESC

    eState_ESC_24_2B,                     // ESC $ +
    eState_ESC_24_2B_I,                   // ESC $ + I
    eState_ESC_24_2B_I_ESC,               // ESC $ + I ESC
    eState_CNS11643_3,                    // ESC $ + I ESC SS3
    eState_CNS11643_3_2ndbyte,            // ESC $ + I ESC SS3
    eState_ESC_24_2B_I_ESC_SS3_SI,        // ESC $ + I ESC SI
    eState_ESC_24_2B_I_ESC_SS3_SI_ESC,    // ESC $ + I ESC SI ESC
    eState_ERROR
  } mState;

  char mData;

  // Plane number for CNS11643 code
  int mPlaneID;

  // Length of non-ASCII run
  PRUint32 mRunLength;

  // Decoder handler
  nsCOMPtr<nsIUnicodeDecoder> mGB2312_Decoder;
  nsCOMPtr<nsIUnicodeDecoder> mEUCTW_Decoder;

  NS_IMETHOD GB2312_To_Unicode(unsigned char *aSrc, PRInt32 aSrcLength,
     PRUnichar * aDest, PRInt32 * aDestLength) ;

  NS_IMETHOD EUCTW_To_Unicode(unsigned char *aSrc, PRInt32 aSrcLength,
     PRUnichar * aDest, PRInt32 * aDestLength) ;
};
#endif // nsISO2022CNToUnicode_h__
