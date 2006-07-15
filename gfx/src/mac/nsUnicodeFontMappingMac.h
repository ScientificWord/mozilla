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

#ifndef nsUnicodeFontMappingMac_h__
#define nsUnicodeFontMappingMac_h__

#include "nsUnicodeBlock.h"
#include "nsIDeviceContext.h"
#include "nsFont.h"
#include "nsVoidArray.h"
 
#define BAD_FONT_NUM	-1
#define IGNORABLE_FONT_NUM -2
#define BAD_SCRIPT 0x7f

class nsUnicodeMappingUtil;
class nsUnicodeFontMappingCache;

class nsUnicodeFontMappingMac {
public:
   nsUnicodeFontMappingMac(nsFont* aFont, nsIDeviceContext *aDeviceContext, 
   		const nsString& aLangGroup, const nsString& aLANG);
   ~nsUnicodeFontMappingMac();
   		
   short GetFontID(PRUnichar aChar);
   inline const short *GetScriptFallbackFonts() {
   		return mScriptFallbackFontIDs;
   }
   PRBool Equals(const nsUnicodeFontMappingMac& anther);
   
   PRBool ConvertUnicodeToGlyphs(short aFontNum, const PRUnichar* aString,
       ByteCount aStringLength, char *aBuffer, ByteCount aBufferLength, 
       ByteCount& oActualLength, ByteCount& oBytesRead, OptionBits opts);

   static PRBool FontEnumCallback(const nsString& aFamily, PRBool aGeneric, void *aData);

protected:
   PRBool ScriptMapInitComplete();
   void InitByFontFamily(nsFont* aFont, nsIDeviceContext *aDeviceContext);
   void InitByLANG(const nsString& aLANG);
   void InitByLangGroup(const nsString& aLangGroup);
   void InitDefaultScriptFonts();
   void processOneLangRegion(const char* aLanguage, const char* aRegion );
   nsUnicodeBlock GetBlock(PRUnichar aChar);
private:
   
   PRInt8 mPrivBlockToScript [kUnicodeBlockVarScriptMax] ;
   short  mScriptFallbackFontIDs [smPseudoTotalScripts] ;
   nsAutoVoidArray mFontList;

   static nsUnicodeMappingUtil* gUtil;
};

#endif /* nsUnicodeFontMappingMac_h__ */
