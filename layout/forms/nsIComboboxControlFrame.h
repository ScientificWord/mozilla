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

#ifndef nsIComboboxControlFrame_h___
#define nsIComboboxControlFrame_h___

#include "nsISupports.h"
#include "nsFont.h"

class nsPresContext;
class nsString;
class nsIContent;
class nsVoidArray;
class nsCSSFrameConstructor;


// IID for the nsIComboboxControlFrame class
#define NS_ICOMBOBOXCONTROLFRAME_IID    \
  { 0x23f75e9c, 0x6850, 0x11da, \
      { 0x95, 0x2c, 0x0, 0xe0, 0x81, 0x61, 0x16, 0x5f } }

/** 
  * nsIComboboxControlFrame is the common interface for frames of form controls. It
  * provides a uniform way of creating widgets, resizing, and painting.
  * @see nsLeafFrame and its base classes for more info
  */
class nsIComboboxControlFrame : public nsISupports {

public:
  NS_DECLARE_STATIC_IID_ACCESSOR(NS_ICOMBOBOXCONTROLFRAME_IID)

  /**
   * Indicates whether the list is dropped down
   */
  virtual PRBool IsDroppedDown() = 0;

  /**
   * Shows or hides the drop down
   */
  virtual void ShowDropDown(PRBool aDoDropDown) = 0;

  /**
   * Gets the Drop Down List
   */
  virtual nsIFrame* GetDropDown() = 0;

  /**
   * Sets the Drop Down List
   */
  virtual void SetDropDown(nsIFrame* aDropDownFrame) = 0;

  /**
   * Tells the combobox to roll up
   */
  virtual void RollupFromList() = 0;

  /**
   * Redisplay the selected text (will do nothing if text has not changed)
   */
  NS_IMETHOD RedisplaySelectedText() = 0;

  /**
   * Method for the listbox to set and get the recent index
   */
  virtual PRInt32 UpdateRecentIndex(PRInt32 aIndex) = 0;

  /**
   *
   */
  virtual void AbsolutelyPositionDropDown() = 0;

  /**
   * Notification that the content has been reset
   */
  virtual void OnContentReset() = 0;
  
  /**
   * This returns the index of the item that is currently being displayed
   * in the display area. It may differ from what the currently Selected index
   * is in in the dropdown.
   *
   * Detailed explanation: 
   * When the dropdown is dropped down via a mouse click and the user moves the mouse 
   * up and down without clicking, the currently selected item is being tracking inside 
   * the dropdown, but the combobox is not being updated. When the user selects items
   * with the arrow keys, the combobox is being updated. So when the user clicks outside
   * the dropdown and it needs to roll up it has to decide whether to keep the current 
   * selection or not. This method is used to get the current index in the combobox to
   * compare it to the current index in the dropdown to see if the combox has been updated
   * and that way it knows whether to "cancel" the current selection residing in the 
   * dropdown. Or whether to leave the selection alone.
   */
  virtual PRInt32 GetIndexOfDisplayArea() = 0;
};

NS_DEFINE_STATIC_IID_ACCESSOR(nsIComboboxControlFrame,
                              NS_ICOMBOBOXCONTROLFRAME_IID)

#endif

