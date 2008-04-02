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
 *   Josh Aas <josh@mozilla.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
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

#ifndef nsMenuItemX_h_
#define nsMenuItemX_h_

#include "nsIMenuItem.h"
#include "nsString.h"
#include "nsChangeObserver.h"
#include "nsAutoPtr.h"

#import <Cocoa/Cocoa.h>

class nsIMenu;
class nsMenuItemIconX;

/**
 * Native menu item wrapper
 */

class nsMenuItemX : public nsIMenuItem,
                    public nsChangeObserver
{
public:
  nsMenuItemX();
  virtual ~nsMenuItemX();

  // nsISupports
  NS_DECL_ISUPPORTS
  NS_DECL_CHANGEOBSERVER

  // nsIMenuItem Methods
  NS_IMETHOD Create(nsIMenu* aParent, const nsString & aLabel, EMenuItemType aItemType,
                    nsMenuBarX* aMenuBar, nsIContent* aNode);
  NS_IMETHOD GetLabel(nsString &aText);
  NS_IMETHOD GetShortcutChar(nsString &aText);
  NS_IMETHOD GetEnabled(PRBool *aIsEnabled);
  NS_IMETHOD SetChecked(PRBool aIsEnabled);
  NS_IMETHOD GetChecked(PRBool *aIsEnabled);
  NS_IMETHOD GetMenuItemType(EMenuItemType *aIsCheckbox);
  NS_IMETHOD GetNativeData(void*& aData);
  NS_IMETHOD IsSeparator(PRBool & aIsSep);

  NS_IMETHOD DoCommand();
  NS_IMETHOD DispatchDOMEvent(const nsString &eventName, PRBool *preventDefaultCalled);
  NS_IMETHOD SetupIcon();
  NS_IMETHOD GetMenuItemContent(nsIContent ** aMenuItemContent);

protected:

  void UncheckRadioSiblings(nsIContent* inCheckedElement);
  void SetKeyEquiv(PRUint8 aModifiers, const nsString &aText);

  NSMenuItem*               mNativeMenuItem;       // strong ref, we own
  
  nsString                  mLabel;
  nsString                  mKeyEquivalent;

  nsIMenu*                  mMenuParent;          // weak, parent owns us
  nsMenuBarX*               mMenuBar;             // weak
  
  nsCOMPtr<nsIContent>      mContent;
  nsCOMPtr<nsIContent>      mCommandContent;
  nsRefPtr<nsMenuItemIconX> mIcon;
  
  PRUint8           mModifiers;
  PRPackedBool      mEnabled;
  PRPackedBool      mIsChecked;
  EMenuItemType     mType; // regular, checkbox, radio, or separator
};

#endif // nsMenuItemX_h_
