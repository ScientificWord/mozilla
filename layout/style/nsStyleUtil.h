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
#ifndef nsStyleUtil_h___
#define nsStyleUtil_h___

#include "nsCoord.h"
#include "nsPresContext.h"
#include "nsILinkHandler.h" // for nsLinkState

struct nsStyleBackground;

enum nsFontSizeType {
  eFontSize_HTML  	= 1,
  eFontSize_CSS			= 2
};


// Style utility functions
class nsStyleUtil {
public:
  
  static float GetScalingFactor(PRInt32 aScaler);

  static nscoord CalcFontPointSize(PRInt32 aHTMLSize, PRInt32 aBasePointSize, 
                                   float aScalingFactor, nsPresContext* aPresContext,
                                   nsFontSizeType aFontSizeType = eFontSize_HTML);

  static nscoord FindNextSmallerFontSize(nscoord aFontSize, PRInt32 aBasePointSize, 
                                         float aScalingFactor, nsPresContext* aPresContext,
                                         nsFontSizeType aFontSizeType = eFontSize_HTML);

  static nscoord FindNextLargerFontSize(nscoord aFontSize, PRInt32 aBasePointSize, 
                                        float aScalingFactor, nsPresContext* aPresContext,
                                        nsFontSizeType aFontSizeType = eFontSize_HTML);

  static PRInt32 ConstrainFontWeight(PRInt32 aWeight);

  static PRBool IsHTMLLink(nsIContent *aContent, nsIAtom *aTag, nsPresContext *aPresContext, nsLinkState *aState);
  static PRBool IsLink(nsIContent *aContent, nsPresContext *aPresContext, nsLinkState *aState);

 static PRBool DashMatchCompare(const nsAString& aAttributeValue,
                                const nsAString& aSelectorValue,
                                const nsStringComparator& aComparator);
                                
  static void EscapeCSSString(const nsString& aString, nsAString& aReturn);

  /*
   * Convert an author-provided floating point number to an integer (0
   * ... 255) appropriate for use in the alpha component of a color.
   */
  static PRUint8 FloatToColorComponent(float aAlpha)
  {
    NS_ASSERTION(0.0 <= aAlpha && aAlpha <= 1.0, "out of range");
    return NSToIntRound(aAlpha * 255);
  }

  /*
   * Convert the alpha component of an nscolor (0 ... 255) to the
   * floating point number with the least accurate *decimal*
   * representation that is converted to that color.
   *
   * Should be used only by serialization code.
   */
  static float ColorComponentToFloat(PRUint8 aAlpha);

  /*
   * Does this child count as significant for selector matching?
   */
  static PRBool IsSignificantChild(nsIContent* aChild,
                                   PRBool aTextIsSignificant,
                                   PRBool aWhitespaceIsSignificant);
};


#endif /* nsStyleUtil_h___ */
