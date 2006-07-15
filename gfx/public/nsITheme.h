/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
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
 * The Original Code is the Mozilla browser.
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

#ifndef nsITheme_h_
#define nsITheme_h_

#include "nsISupports.h"
#include "nsCOMPtr.h"
#include "nsColor.h"

struct nsRect;
struct nsSize;
struct nsFont;
struct nsMargin;
class nsPresContext;
class nsIRenderingContext;
class nsIDeviceContext;
class nsIFrame;
class nsIContent;
class nsIAtom;

// IID for the nsITheme interface
// {75220e36-b77a-464c-bd82-988cf86391cc}
#define NS_ITHEME_IID     \
{ 0x75220e36, 0xb77a, 0x464c, { 0xbd, 0x82, 0x98, 0x8c, 0xf8, 0x63, 0x91, 0xcc } }

// {D930E29B-6909-44e5-AB4B-AF10D6923705}
#define NS_THEMERENDERER_CID \
{ 0xd930e29b, 0x6909, 0x44e5, { 0xab, 0x4b, 0xaf, 0x10, 0xd6, 0x92, 0x37, 0x5 } }

class nsITheme: public nsISupports {
public:
  NS_DECLARE_STATIC_IID_ACCESSOR(NS_ITHEME_IID)
  
  NS_IMETHOD DrawWidgetBackground(nsIRenderingContext* aContext,
                                  nsIFrame* aFrame,
                                  PRUint8 aWidgetType,
                                  const nsRect& aRect,
                                  const nsRect& aClipRect)=0;

  NS_IMETHOD GetWidgetBorder(nsIDeviceContext* aContext, 
                             nsIFrame* aFrame,
                             PRUint8 aWidgetType,
                             nsMargin* aResult)=0;

  // This method can return PR_FALSE to indicate that the CSS padding value
  // should be used.  Otherwise, it will fill in aResult with the desired
  // padding and return PR_TRUE.
  virtual PRBool GetWidgetPadding(nsIDeviceContext* aContext,
                                  nsIFrame* aFrame,
                                  PRUint8 aWidgetType,
                                  nsMargin* aResult) = 0;

  // This method can return PR_FALSE to indicate that no special overflow
  // area is required by the native widget. Otherwise it will fill in
  // aResult with the desired overflow area and return PR_TRUE.
  virtual PRBool GetWidgetOverflow(nsIDeviceContext* aContext,
                                   nsIFrame* aFrame,
                                   PRUint8 aWidgetType,
                                   nsRect* aResult)
  { return PR_FALSE; }

  NS_IMETHOD GetMinimumWidgetSize(nsIRenderingContext* aContext, nsIFrame* aFrame,
                                  PRUint8 aWidgetType,
                                  nsSize* aResult,
                                  PRBool* aIsOverridable)=0;

  NS_IMETHOD WidgetStateChanged(nsIFrame* aFrame, PRUint8 aWidgetType, 
                                nsIAtom* aAttribute, PRBool* aShouldRepaint)=0;

  NS_IMETHOD ThemeChanged()=0;

  virtual PRBool ThemeSupportsWidget(nsPresContext* aPresContext,
                                     nsIFrame* aFrame,
                                     PRUint8 aWidgetType)=0;

  virtual PRBool WidgetIsContainer(PRUint8 aWidgetType)=0;
};

NS_DEFINE_STATIC_IID_ACCESSOR(nsITheme, NS_ITHEME_IID)

// Creator function
extern NS_METHOD NS_NewNativeTheme(nsISupports *aOuter, REFNSIID aIID, void **aResult);

#endif
