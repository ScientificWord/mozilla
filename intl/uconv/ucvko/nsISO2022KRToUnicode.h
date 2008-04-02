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
#ifndef nsISO2022KRToUnicode_h__
#define nsISO2022KRToUnicode_h__
#include "nsISupports.h"
#include "nsUCSupport.h"


 
class nsISO2022KRToUnicode : public nsBasicDecoderSupport
{
public:
  nsISO2022KRToUnicode()
  { 
    mState = mState_ASCII;
    mLastLegalState = mState_ASCII;
    mData = 0;
    mEUCKRDecoder = nsnull;
    mRunLength = 0;
  }

  virtual ~nsISO2022KRToUnicode()
  {
    NS_IF_RELEASE(mEUCKRDecoder);
  }

  NS_IMETHOD Convert(const char * aSrc, PRInt32 * aSrcLength,
     PRUnichar * aDest, PRInt32 * aDestLength) ;
  
  NS_IMETHOD GetMaxLength(const char * aSrc, PRInt32 aSrcLength,
     PRInt32 * aDestLength) 
  {
    *aDestLength = aSrcLength;
    return NS_OK;
  }

  NS_IMETHOD Reset()
  {
    mState = mState_ASCII;
    mLastLegalState = mState_ASCII;
    mRunLength = 0;
    return NS_OK;
  }

private:
  enum {
    mState_ASCII,
    mState_ESC,
    mState_ESC_24,
    mState_ESC_24_29,
    mState_KSX1001_1992,
    mState_KSX1001_1992_2ndbyte,
    mState_ERROR
  } mState, mLastLegalState;

  PRUint8 mData;

  // Length of non-ASCII run
  PRUint32 mRunLength;

  nsIUnicodeDecoder *mEUCKRDecoder;
};
#endif // nsISO2022KRToUnicode_h__
