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

#ifndef nsIMenu_h__
#define nsIMenu_h__

#include "nsISupports.h"
#include "nsStringFwd.h"
#include "nsEvent.h"

class nsIMenuBar;
class nsIMenu;
class nsIMenuItem;
class nsIContent;
class nsIWidget;
class nsMenuBarX;


// 9225136B-3F56-4CA3-92E0-623D5FB8356B
#define NS_IMENU_IID \
{ 0x9225136B, 0x3F56, 0x4CA3, \
  { 0x92, 0xE0, 0x62, 0x3D, 0x5F, 0xB8, 0x35, 0x6B } }

/**
 * Menu widget
 */
class nsIMenu : public nsISupports {

  public:
    NS_DECLARE_STATIC_IID_ACCESSOR(NS_IMENU_IID)

  /**
    * Creates the Menu
    *
    */
    NS_IMETHOD Create(nsISupports * aParent, const nsAString &aLabel, const nsAString &aAccessKey, 
                      nsMenuBarX* aMenuBar, nsIContent* aNode) = 0;

  /**
   * Get the Menu's Parent.  This addrefs.
   *
   */
    NS_IMETHOD GetParent(nsISupports *&aParent) = 0;

  /**
   * Get the Menu label
   *
   */
    NS_IMETHOD GetLabel(nsString &aText) = 0;

  /**
   * Set the Menu label
   *
   */
    NS_IMETHOD SetLabel(const nsAString &aText) = 0;

  /**
    * Get the Menu Access Key
    *
    */
  NS_IMETHOD GetAccessKey(nsString &aText) = 0;
   
  /**
    * Set the Menu Access Key
    *
    */
  NS_IMETHOD SetAccessKey(const nsAString &aText) = 0;

  /**
    * Set the Menu enabled state
    *
    */
  NS_IMETHOD SetEnabled(PRBool aIsEnabled) = 0;

  /**
    * Get the Menu enabled state
    *
    */
  NS_IMETHOD GetEnabled(PRBool* aIsEnabled) = 0;
  
  /**
    * Adds a Menu Item. Do not use outside of widget menu implementations.
    * Add and modify menu items via DOM content.
    *
    */
    NS_IMETHOD AddItem(nsISupports* aItem) = 0;

   /**
    * Returns the number of visible menu items
    * This includes separators. It does not include hidden items.
    *
    */
    NS_IMETHOD GetVisibleItemCount(PRUint32 &aCount) = 0;

   /**
    * Returns a Menu or Menu Item at a specified Index.
    * This includes separators. It does not include hidden items.
    *
    */
    NS_IMETHOD GetVisibleItemAt(const PRUint32 aPos, nsISupports *& aMenuItem) = 0;

   /**
    * Returns the number of menu items
    * This includes separators. It -does- include hidden items.
    *
    */
    NS_IMETHOD GetItemCount(PRUint32 &aCount) = 0;

   /**
    * Returns a Menu or Menu Item at a specified Index.
    * This includes separators. It -does- include hidden items.
    *
    */
    NS_IMETHOD GetItemAt(const PRUint32 aPos, nsISupports *& aMenuItem) = 0;

   /**
    * Inserts a Menu Item at a specified Index
    *
    */
    NS_IMETHOD InsertItemAt(const PRUint32 aPos, nsISupports * aMenuItem) = 0;

   /**
    * Removes an Menu Item from a specified Index
    *
    */
    NS_IMETHOD RemoveItem(const PRUint32 aPos) = 0;

   /**
    * Removes all the Menu Items
    *
    */
    NS_IMETHOD RemoveAll() = 0;

   /**
    * Gets Native MenuHandle
    *
    */
    NS_IMETHOD  GetNativeData(void** aData) = 0;

   /**
    * Sets Native MenuHandle
    *
    */
    NS_IMETHOD  SetNativeData(void* aData) = 0;

   /**
    * Get menu content
    *
    */
    NS_IMETHOD GetMenuContent(nsIContent ** aMenuContent) = 0;
    
   /**
    * Enable/disable native widget for a particular nsIMenuItem
    *
    */
    NS_IMETHOD ChangeNativeEnabledStatusForMenuItem(nsIMenuItem* aMenuItem,
                                                    PRBool aEnabled) = 0;

   /**
    * Retrieve the native menu and the index of the item within that menu.
    *
    */
    NS_IMETHOD GetMenuRefAndItemIndexForMenuItem(nsISupports* aMenuItem,
                                                 void**       aMenuRef,
                                                 PRUint16*    aMenuItemIndex) = 0;

   /**
    * Sets an appropriate icon for the menu.
    *
    */
    NS_IMETHOD SetupIcon() = 0;
    
   /**
    * Menu has been selected
    *
    */
    virtual nsEventStatus MenuSelected(const nsMenuEvent & aMenuEvent) = 0;
    
   /**
    * Menu has been deselected
    *
    */
    virtual void MenuDeselected(const nsMenuEvent & aMenuEvent) = 0;

   /**
    * Construct menu
    *
    */
    virtual void MenuConstruct(const nsMenuEvent & aMenuEvent, nsIWidget * aParentWindow, void * aMenuNode) = 0;

   /**
    * Destruct menu
    *
    */
    virtual void MenuDestruct(const nsMenuEvent & aMenuEvent) = 0;
    
   /**
    * Set rebuild
    *
    */
    virtual void SetRebuild(PRBool aMenuEvent) = 0;
};

NS_DEFINE_STATIC_IID_ACCESSOR(nsIMenu, NS_IMENU_IID)

#endif
