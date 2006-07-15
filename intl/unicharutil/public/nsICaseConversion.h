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
#ifndef nsICaseConversion_h__
#define nsICaseConversion_h__


#include "nsISupports.h"
#include "nscore.h"

// {07D3D8E0-9614-11d2-B3AD-00805F8A6670}
#define NS_ICASECONVERSION_IID \
{ 0x7d3d8e0, 0x9614, 0x11d2, \
    { 0xb3, 0xad, 0x0, 0x80, 0x5f, 0x8a, 0x66, 0x70 } }

class nsICaseConversion : public nsISupports {

public: 

  NS_DECLARE_STATIC_IID_ACCESSOR(NS_ICASECONVERSION_IID)

  // Convert one Unicode character into upper case
  NS_IMETHOD ToUpper( PRUnichar aChar, PRUnichar* aReturn) = 0;

  // Convert one Unicode character into lower case
  NS_IMETHOD ToLower( PRUnichar aChar, PRUnichar* aReturn) = 0;

  // Convert one Unicode character into title case
  NS_IMETHOD ToTitle( PRUnichar aChar, PRUnichar* aReturn) = 0;

  // Convert an array of Unicode characters into upper case
  NS_IMETHOD ToUpper( const PRUnichar* anArray, PRUnichar* aReturn, PRUint32 aLen) = 0;

  // Convert an array of Unicode characters into lower case
  NS_IMETHOD ToLower( const PRUnichar* anArray, PRUnichar* aReturn, PRUint32 aLen) = 0;

  // Convert an array of Unicode characters into title case
  NS_IMETHOD ToTitle( const PRUnichar* anArray, PRUnichar* aReturn, 
                      PRUint32 aLen, PRBool aStartInWordBundary=PR_TRUE) = 0;

  // case-insensitive PRUnichar* comparison - aResult returns similar
  // to strcasecmp
  NS_IMETHOD CaseInsensitiveCompare(const PRUnichar* aLeft, const PRUnichar* aRight, PRUint32 aLength, PRInt32* aResult) = 0;
};

NS_DEFINE_STATIC_IID_ACCESSOR(nsICaseConversion, NS_ICASECONVERSION_IID)

#endif  /* nsICaseConversion_h__ */
