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
 * Portions created by the Initial Developer are Copyright (C) 1999
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

#ifndef nsUnicodeRenderingToolkit_h__
#define nsUnicodeRenderingToolkit_h__

#include <UnicodeConverter.h>

#include "nsATSUIUtils.h"

#include "nsCOMPtr.h"
#include "nsISaveAsCharset.h"
#include "nsIRenderingContext.h"

class nsUnicodeFallbackCache;
class nsIDeviceContext;
class nsGraphicState;
class nsUnicodeFontMappingMac;

class nsUnicodeRenderingToolkit 
{
public:
  nsUnicodeRenderingToolkit() {};
  virtual ~nsUnicodeRenderingToolkit() {};

  nsresult PrepareToDraw(float aP2T, nsIDeviceContext* aContext, nsGraphicState* aGS, 
                         CGrafPtr aPort, PRBool aRightToLeftText);
  nsresult GetTextDimensions(const PRUnichar *aString, PRUint32 aLength, 
                             nsTextDimensions &aDimension, PRInt32 *aFontID);
  nsresult GetWidth(const PRUnichar *aString, PRUint32 aLength, nscoord &aWidth,
                    PRInt32 *aFontID);

  nsresult DrawString(const PRUnichar *aString, PRUint32 aLength, nscoord aX, nscoord aY,
                      PRInt32 aFontID,
                      const nscoord* aSpacing);
#ifdef MOZ_MATHML
  nsresult GetTextBoundingMetrics(const PRUnichar *aString, PRUint32 aLength,
                                  nsBoundingMetrics &aBoundingMetrics, PRInt32 *aFontID);
#endif // MOZ_MATHML

private:  
  // Unicode text measure/drawing functions
  PRBool TECFallbackGetDimensions(const PRUnichar *pChar, nsTextDimensions& oWidth,
                                  short fontNum, nsUnicodeFontMappingMac& fontMapping);
  PRBool TECFallbackDrawChar(const PRUnichar *pChar, PRInt32 x, PRInt32 y, short& oWidth,
                             short fontNum, nsUnicodeFontMappingMac& fontMapping);
                  
  PRBool ATSUIFallbackGetDimensions(const PRUnichar *pChar, nsTextDimensions& oWidth, short fontNum,  
                                    short aSize, PRBool aBold, PRBool aItalic, nscolor aColor);
  PRBool ATSUIFallbackDrawChar(const PRUnichar *pChar, PRInt32 x, PRInt32 y, short& oWidth, short fontNum, 
                               short aSize, PRBool aBold, PRBool aItalic, nscolor aColor);
  PRBool SurrogateGetDimensions(const PRUnichar *aSurrogatePt, nsTextDimensions& oWidth, short fontNum,  
                                    short aSize, PRBool aBold, PRBool aItalic, nscolor aColor);
  PRBool SurrogateDrawChar(const PRUnichar *aSurrogatePt, PRInt32 x, PRInt32 y, short& oWidth, short fontNum, 
                               short aSize, PRBool aBold, PRBool aItalic, nscolor aColor);
  
  PRBool UPlusFallbackGetWidth(const PRUnichar *pChar, short& oWidth);
  PRBool UPlusFallbackDrawChar(const PRUnichar *pChar, PRInt32 x, PRInt32 y, short& oWidth);

  PRBool QuestionMarkFallbackGetWidth(const PRUnichar *pChar, short& oWidth);
  PRBool QuestionMarkFallbackDrawChar(const PRUnichar *pChar, PRInt32 x, PRInt32 y, short& oWidth);

  PRBool LatinFallbackGetWidth(const PRUnichar *pChar, short& oWidth);
  PRBool LatinFallbackDrawChar(const PRUnichar *pChar, PRInt32 x, PRInt32 y, short& oWidth);
  PRBool PrecomposeHangulFallbackGetWidth(const PRUnichar *pChar, short& oWidth,short koreanFont, short origFont);
  PRBool PrecomposeHangulFallbackDrawChar(const PRUnichar *pChar, PRInt32 x, PRInt32 y, short& oWidth,
                                          short koreanFont, short origFont);
  PRBool TransliterateFallbackGetWidth(const PRUnichar *pChar, short& oWidth);
  PRBool TransliterateFallbackDrawChar(const PRUnichar *pChar, PRInt32 x, PRInt32 y, short& oWidth);
  PRBool LoadTransliterator();
  
  void GetScriptTextWidth(const char* aText, ByteCount aLen, short& aWidth);
  void DrawScriptText(const char* aText, ByteCount aLen, PRInt32 x, PRInt32 y, short& aWidth);
  
  nsresult GetTextSegmentWidth(const PRUnichar *aString, PRUint32 aLength, short fontNum, 
                               nsUnicodeFontMappingMac& fontMapping, PRUint32& oWidth);
  nsresult GetTextSegmentDimensions(const PRUnichar *aString, PRUint32 aLength, short fontNum, 
                                    nsUnicodeFontMappingMac& fontMapping, nsTextDimensions& aDimension);
  nsresult DrawTextSegment(const PRUnichar *aString, PRUint32 aLength, short fontNum, 
                           nsUnicodeFontMappingMac& fontMapping, PRInt32 x, PRInt32 y, PRUint32& oWidth);

#ifdef MOZ_MATHML
  PRBool TECFallbackGetBoundingMetrics(const PRUnichar *pChar, nsBoundingMetrics& oBoundingMetrics,
  										short fontNum, nsUnicodeFontMappingMac& fontMapping);
  PRBool ATSUIFallbackGetBoundingMetrics(const PRUnichar *pChar, nsBoundingMetrics& oBoundingMetrics, short fontNum,
                                         short aSize, PRBool aBold, PRBool aItalic, nscolor aColor);
  PRBool SurrogateGetBoundingMetrics(const PRUnichar *aSurrogatePt, nsBoundingMetrics& oBoundingMetrics, short fontNum,
                                         short aSize, PRBool aBold, PRBool aItalic, nscolor aColor);
  void GetScriptTextBoundingMetrics(const char* aText, ByteCount aLen, ScriptCode aScript, nsBoundingMetrics& oBoundingMetrics);
  nsresult GetTextSegmentBoundingMetrics(const PRUnichar *aString, PRUint32 aLength, short fontNum,
                                         nsUnicodeFontMappingMac& fontMapping, nsBoundingMetrics& oBoundingMetrics);
#endif // MOZ_MATHML

  nsUnicodeFallbackCache* GetTECFallbackCache();    
private:
  float mP2T; // Pixel to Twip conversion factor
  nsIDeviceContext *mContext;
  nsGraphicState *mGS; // current graphic state - shortcut for mCurrentSurface->GetGS()

  CGrafPtr mPort; // current grafPort - shortcut for mCurrentSurface->GetPort()
  nsATSUIToolkit mATSUIToolkit;
  nsCOMPtr<nsISaveAsCharset> mTrans;
  PRBool mRightToLeftText;

};
#endif /* nsUnicodeRenderingToolkit_h__ */
