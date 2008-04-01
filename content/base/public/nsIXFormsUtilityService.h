/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
 * The Original Code is Mozilla XForms support.
 *
 * The Initial Developer of the Original Code is
 * IBM Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2006
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *  Aaron Reed <aaronr@us.ibm.com>
 *  Alexander Surkov <surkov.alexander@gmail.com>
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

#ifndef nsIXFormsUtilityService_h
#define nsIXFormsUtilityService_h

#include "nsISupports.h"

class nsIDOMNode;
class nsIDOMNodeList;
class nsIEditor;

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* nsIXFormsUtilityService */
#define NS_IXFORMSUTILITYSERVICE_IID_STR "cd3457b6-cb6a-496c-bdfa-6cfecb2bd5fb"
#define NS_IXFORMSUTILITYSERVICE_IID \
{ 0xcd3457b6, 0xcb6a, 0x496c, \
  { 0xbd, 0xfa, 0x6c, 0xfe, 0xcb, 0x2b, 0xd5, 0xfb } }


/**
 * Private interface implemented by the nsXFormsUtilityService in XForms
 * extension.
 */
class NS_NO_VTABLE nsIXFormsUtilityService : public nsISupports {
public:

  NS_DECLARE_STATIC_IID_ACCESSOR(NS_IXFORMSUTILITYSERVICE_IID)

  enum {
    STATE_OUT_OF_RANGE,
    STATE_IN_RANGE,
    STATE_NOT_A_RANGE
  };

  /**
   * Returns the name of the builtin type of the instance node that aElement is
   * bound to. Fails if aElement doesn't have a bound node.
   */
  NS_IMETHOD GetBuiltinTypeName(nsIDOMNode *aElement, nsAString& aName) = 0;

  /**
   * Return true if instance node that element is bound to is readonly.
   */
  NS_IMETHOD IsReadonly(nsIDOMNode *aElement, PRBool *aState) = 0;

  /**
   * Return true if instance node that element is bound to is relevant.
   */
  NS_IMETHOD IsRelevant(nsIDOMNode *aElement, PRBool *aState) = 0;

  /**
   * Return true if instance node that element is bound to is required.
   */
  NS_IMETHOD IsRequired(nsIDOMNode *aElement, PRBool *aState) = 0;

  /**
   * Return true if instance node that element is bound to is valid.
   */
  NS_IMETHOD IsValid(nsIDOMNode *aElement, PRBool *aState) = 0;

  /**
   * Return constant declared above that indicates whether instance node that
   * element is bound to is out of range, is in range or neither. The last value
   * is used if element can't have in-range or out-of-range state, for exmple,
   * xforms:input.
   */
  NS_IMETHOD IsInRange(nsIDOMNode *aElement, PRUint32 *aState) = 0;

  /**
   * Return value of instance node that given node is bound to. If given node is
   * xforms:item element then the method returns item value.
   */
  NS_IMETHOD GetValue(nsIDOMNode *aElement, nsAString& aValue) = 0;

  /**
   * Set the focus to xforms element.
   */
  NS_IMETHOD Focus(nsIDOMNode *aElement) = 0;

  /**
   * Return @start attribute value of xforms:range element. Failure if
   * given element is not xforms:range.
   */
  NS_IMETHOD GetRangeStart(nsIDOMNode *aElement, nsAString& aValue) = 0;

  /**
   * Return @end attribute value of xforms:range element. Failure if
   * given element is not xforms:range.
   */
  NS_IMETHOD GetRangeEnd(nsIDOMNode *aElement, nsAString& aValue) = 0;

  /**
   * Return @step attribute value of xforms:range element. Failure if
   * given element is not xforms:range.
   */
  NS_IMETHOD GetRangeStep(nsIDOMNode *aElement, nsAString& aValue) = 0;

  /**
   * Return nsIEditor for xforms element if element is editable, null if it is
   * not editable. Failure if given element doesn't support editing.
   */
  NS_IMETHOD GetEditor(nsIDOMNode *aElement, nsIEditor **aEditor) = 0;

  /**
   * Return true if dropmarker is in open state (combobox popup is open),
   * otherwise false. Failure if given 'aElement' node is not direct child of
   * combobox element or is not combobox itself.
   */
  NS_IMETHOD IsDropmarkerOpen(nsIDOMNode *aElement, PRBool* aIsOpen) = 0;

  /**
   * Toggles dropmarker state (close/open combobox popup). Failure if given
   * 'aElement' node is not direct child of combobox element or is not combobox
   * itself.
   */
  NS_IMETHOD ToggleDropmarkerState(nsIDOMNode *aElement) = 0;

  /**
   * Get selected xforms:item element for xforms:select1. Failure if
   * given 'aElement' node is not xforms:select1.
   */
  NS_IMETHOD GetSelectedItemForSelect1(nsIDOMNode *aElement,
                                       nsIDOMNode ** aItem) = 0;

  /**
   * Set selected xforms:item element for xforms:select1. Failure if
   * given 'aElement' node is not xforms:select1.
   */
  NS_IMETHOD SetSelectedItemForSelect1(nsIDOMNode *aElement,
                                       nsIDOMNode *aItem) = 0;

  /**
   * Get the list of selected xforms:item elements from the xforms:select.
   * Failure if given 'aElement' node is not xforms:select.
   */
  NS_IMETHOD GetSelectedItemsForSelect(nsIDOMNode *aElement,
                                       nsIDOMNodeList **aItems) = 0;

  /**
   * Add xforms:item element to selection of xforms:select. Failure if
   * given 'aElement' node is not xforms:select.
   */
  NS_IMETHOD AddItemToSelectionForSelect(nsIDOMNode *aElement,
                                         nsIDOMNode *Item) = 0;

  /**
   * Remove xforms:item element from selection of xforms:select. Failure if
   * given 'aElement' node is not xforms:select.
   */
  NS_IMETHOD RemoveItemFromSelectionForSelect(nsIDOMNode *aElement,
                                              nsIDOMNode *Item) = 0;

  /**
   * Deslect all xforms:item elements contained in the given xforms:select.
   * Failure if given 'aElement' node is not xforms:select.
   */
  NS_IMETHOD ClearSelectionForSelect(nsIDOMNode *aElement) = 0;

  /**
   * Select all xforms:item elements of xforms:select. Failure if
   * given 'aElement' node is not xforms:select.
   */
  NS_IMETHOD SelectAllItemsForSelect(nsIDOMNode *aElement) = 0;

  /**
   * Return true if given xforms:item element of xforms:select is selected,
   * otherwise return false. Failure if given 'aElement' node is not
   * xforms:select.
   */
  NS_IMETHOD IsSelectItemSelected(nsIDOMNode *aElement, nsIDOMNode *aItem,
                                  PRBool *aIsSelected) = 0;

  /**
   * Return the list of xforms:item or xforms:choices elements that are children
   * of the given 'aElement' node. 'aElement' node may be a xforms:select,
   * xforms:select1, xforms:choices or xforms:itemset element. Otherwise
   * failure.
   */
  NS_IMETHOD GetSelectChildrenFor(nsIDOMNode *aElement,
                                  nsIDOMNodeList **aNodeList) = 0;
};

NS_DEFINE_STATIC_IID_ACCESSOR(nsIXFormsUtilityService,
                              NS_IXFORMSUTILITYSERVICE_IID)

#endif /* nsIXFormsUtilityService_h */
