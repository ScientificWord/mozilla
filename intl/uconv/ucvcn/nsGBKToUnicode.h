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
#ifndef nsGBK2312ToUnicode_h___
#define nsGBK2312ToUnicode_h___

#include "nsCOMPtr.h"
#include "nsIUnicodeDecoder.h"
#include "nsUCSupport.h"
#include "gbku.h"

//----------------------------------------------------------------------
// Class nsGBKToUnicode [declaration]

/**
 * A character set converter from GBK to Unicode.
 * 
 *
 * @created         07/Sept/1999
 * @author  Yueheng Xu, Yueheng.Xu@intel.com
 */
class nsGBKToUnicode : public nsBufferDecoderSupport
{
public:
		  
  /**
   * Class constructor.
   */
  nsGBKToUnicode() : nsBufferDecoderSupport(1)
  {
    mExtensionDecoder = nsnull;
    m4BytesDecoder = nsnull;
  };

protected:

  //--------------------------------------------------------------------
  // Subclassing of nsDecoderSupport class [declaration]
  NS_IMETHOD ConvertNoBuff(const char* aSrc, PRInt32 * aSrcLength, PRUnichar *aDest, PRInt32 * aDestLength);

protected:
  nsGBKConvUtil mUtil;
  nsCOMPtr<nsIUnicodeDecoder> mExtensionDecoder;
  nsCOMPtr<nsIUnicodeDecoder> m4BytesDecoder;

  virtual void CreateExtensionDecoder();
  virtual void Create4BytesDecoder();
  PRBool TryExtensionDecoder(const char* aSrc, PRUnichar* aDest);
  PRBool Try4BytesDecoder(const char* aSrc, PRUnichar* aDest);
  virtual PRBool DecodeToSurrogate(const char* aSrc, PRUnichar* aDest);

};


class nsGB18030ToUnicode : public nsGBKToUnicode
{
public:
  nsGB18030ToUnicode() {};
  virtual ~nsGB18030ToUnicode() {};
protected:
  virtual void CreateExtensionDecoder();
  virtual void Create4BytesDecoder();
  virtual PRBool DecodeToSurrogate(const char* aSrc, PRUnichar* aDest);
};

#endif /* nsGBK2312ToUnicode_h___ */

