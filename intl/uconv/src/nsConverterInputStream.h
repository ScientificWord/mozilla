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

#include "nsIInputStream.h"
#include "nsIConverterInputStream.h"
#include "nsIUnicharLineInputStream.h"
#include "nsString.h"
#include "nsReadLine.h"

#include "nsCOMPtr.h"
#include "nsIUnicodeDecoder.h"
#include "nsIByteBuffer.h"
#include "nsIUnicharBuffer.h"

#define NS_CONVERTERINPUTSTREAM_CONTRACTID "@mozilla.org/intl/converter-input-stream;1"

// {2BC2AD62-AD5D-4b7b-A9DB-F74AE203C527}
#define NS_CONVERTERINPUTSTREAM_CID \
  { 0x2bc2ad62, 0xad5d, 0x4b7b, \
   { 0xa9, 0xdb, 0xf7, 0x4a, 0xe2, 0x3, 0xc5, 0x27 } }



class nsConverterInputStream : public nsIConverterInputStream,
                               public nsIUnicharLineInputStream {

 public:
    NS_DECL_ISUPPORTS
    NS_DECL_NSIUNICHARINPUTSTREAM
    NS_DECL_NSIUNICHARLINEINPUTSTREAM
    NS_DECL_NSICONVERTERINPUTSTREAM

    nsConverterInputStream() :
        mLastErrorCode(NS_OK),
        mLeftOverBytes(0),
        mUnicharDataOffset(0),
        mUnicharDataLength(0),
        mReplacementChar(DEFAULT_REPLACEMENT_CHARACTER),
        mLineBuffer(nsnull) { }
    
    virtual ~nsConverterInputStream() { Close(); }

 private:


    PRUint32 Fill(nsresult *aErrorCode);
    
    nsCOMPtr<nsIUnicodeDecoder> mConverter;
    nsCOMPtr<nsIByteBuffer> mByteData;
    nsCOMPtr<nsIUnicharBuffer> mUnicharData;
    nsCOMPtr<nsIInputStream> mInput;

    nsresult  mLastErrorCode;
    PRUint32  mLeftOverBytes;
    PRUint32  mUnicharDataOffset;
    PRUint32  mUnicharDataLength;
    PRUnichar mReplacementChar;

    nsLineBuffer<PRUnichar>* mLineBuffer;    
};
