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
#ifndef nsIStyleSheetLinkingElement_h__
#define nsIStyleSheetLinkingElement_h__


#include "nsISupports.h"

class nsIDocument;
class nsICSSLoaderObserver;
class nsIURI;

#define NS_ISTYLESHEETLINKINGELEMENT_IID          \
{ 0xd753c84a, 0x17fd, 0x4d5f, \
 { 0xb2, 0xe9, 0x63, 0x52, 0x8c, 0x87, 0x99, 0x7a } }

class nsIStyleSheet;

class nsIStyleSheetLinkingElement : public nsISupports {
public:
  NS_DECLARE_STATIC_IID_ACCESSOR(NS_ISTYLESHEETLINKINGELEMENT_IID)

  /**
   * Used to make the association between a style sheet and
   * the element that linked it to the document.
   *
   * @param aStyleSheet the style sheet associated with this
   *                    element.
   */
  NS_IMETHOD SetStyleSheet(nsIStyleSheet* aStyleSheet) = 0;

  /**
   * Used to obtain the style sheet linked in by this element.
   *
   * @param aStyleSheet out parameter that returns the style
   *                    sheet associated with this element.
   */
  NS_IMETHOD GetStyleSheet(nsIStyleSheet*& aStyleSheet) = 0;

  /**
   * Initialize the stylesheet linking element. If aDontLoadStyle is
   * true the element will ignore the first modification to the
   * element that would cause a stylesheet to be loaded. Subsequent
   * modifications to the element will not be ignored.
   */
  NS_IMETHOD InitStyleLinkElement(PRBool aDontLoadStyle) = 0;

  /**
   * Tells this element to update the stylesheet.
   *
   * @param aObserver    observer to notify once the stylesheet is loaded.
   *                     This will be passed to the CSSLoader
   * @param [out] aWillNotify whether aObserver will be notified when the sheet
   *                          loads.  If this is false, then either we didn't
   *                          start the sheet load at all, the load failed, or
   *                          this was an inline sheet that completely finished
   *                          loading.  In the case when the load failed the
   *                          failure code will be returned.
   * @param [out] whether the sheet is an alternate sheet.  This value is only
   *              meaningful if aWillNotify is true.
   */
  NS_IMETHOD UpdateStyleSheet(nsICSSLoaderObserver* aObserver,
                              PRBool *aWillNotify,
                              PRBool *aIsAlternate) = 0;

  /**
   * Tells this element whether to update the stylesheet when the
   * element's properties change.
   *
   * @param aEnableUpdates update on changes or not.
   */
  NS_IMETHOD SetEnableUpdates(PRBool aEnableUpdates) = 0;

  /**
   * Gets the charset that the element claims the style sheet is in
   *
   * @param aCharset the charset
   */
  NS_IMETHOD GetCharset(nsAString& aCharset) = 0;

  /**
   * Tells this element to use a different base URI. This is used for
   * proper loading of xml-stylesheet processing instructions in XUL overlays
   * and is only currently used by nsXMLStylesheetPI.
   *
   * @param aNewBaseURI the new base URI, nsnull to use the default base URI.
   */
  virtual void OverrideBaseURI(nsIURI* aNewBaseURI) = 0;

  // This doesn't entirely belong here since they only make sense for
  // some types of linking elements, but it's a better place than
  // anywhere else.
  virtual void SetLineNumber(PRUint32 aLineNumber) = 0;
};

NS_DEFINE_STATIC_IID_ACCESSOR(nsIStyleSheetLinkingElement,
                              NS_ISTYLESHEETLINKINGELEMENT_IID)

#endif // nsILinkingElement_h__
