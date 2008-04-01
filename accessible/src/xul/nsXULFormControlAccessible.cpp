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
 *   Aaron Leventhal (aaronl@netscape.com)
 *   Kyle Yuan (kyle.yuan@sun.com)
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

// NOTE: alphabetically ordered
#include "nsXULFormControlAccessible.h"
#include "nsHTMLFormControlAccessible.h"
#include "nsAccessibilityAtoms.h"
#include "nsAccessibleTreeWalker.h"
#include "nsXULMenuAccessible.h"
#include "nsIDOMHTMLInputElement.h"
#include "nsIDOMNSEditableElement.h"
#include "nsIDOMXULButtonElement.h"
#include "nsIDOMXULCheckboxElement.h"
#include "nsIDOMXULMenuListElement.h"
#include "nsIDOMXULSelectCntrlItemEl.h"
#include "nsIDOMXULTextboxElement.h"
#include "nsIEditor.h"
#include "nsIFrame.h"
#include "nsINameSpaceManager.h"
#include "nsITextControlFrame.h"
#include "nsIPresShell.h"

/**
  * XUL Button: can contain arbitrary HTML content
  */

/**
  * Default Constructor
  */

// Don't inherit from nsFormControlAccessible - it doesn't allow children and a button can have a dropmarker child
nsXULButtonAccessible::nsXULButtonAccessible(nsIDOMNode* aNode, nsIWeakReference* aShell):
nsAccessibleWrap(aNode, aShell)
{ 
}

/**
  * Only one actions available
  */
NS_IMETHODIMP nsXULButtonAccessible::GetNumActions(PRUint8 *_retval)
{
  *_retval = 1;
  return NS_OK;
}

/**
  * Return the name of our only action
  */
NS_IMETHODIMP nsXULButtonAccessible::GetActionName(PRUint8 aIndex, nsAString& aName)
{
  if (aIndex == eAction_Click) {
    aName.AssignLiteral("press"); 
    return NS_OK;
  }
  return NS_ERROR_INVALID_ARG;
}

/**
  * Tell the button to do it's action
  */
NS_IMETHODIMP nsXULButtonAccessible::DoAction(PRUint8 index)
{
  if (index == 0) {
    return DoCommand();
  }
  return NS_ERROR_INVALID_ARG;
}

/**
  * We are a pushbutton
  */
NS_IMETHODIMP nsXULButtonAccessible::GetRole(PRUint32 *_retval)
{
  *_retval = nsIAccessibleRole::ROLE_PUSHBUTTON;
  return NS_OK;
}

/**
  * Possible states: focused, focusable, unavailable(disabled)
  */
NS_IMETHODIMP
nsXULButtonAccessible::GetState(PRUint32 *aState, PRUint32 *aExtraState)
{
  // get focus and disable status from base class
  nsresult rv = nsAccessible::GetState(aState, aExtraState);
  NS_ENSURE_SUCCESS(rv, rv);
  if (!mDOMNode)
    return NS_OK;

  PRBool disabled = PR_FALSE;
  nsCOMPtr<nsIDOMXULControlElement> xulFormElement(do_QueryInterface(mDOMNode));
  if (xulFormElement) {
    xulFormElement->GetDisabled(&disabled);
    if (disabled)
      *aState |= nsIAccessibleStates::STATE_UNAVAILABLE;
    else 
      *aState |= nsIAccessibleStates::STATE_FOCUSABLE;
  }

  // Buttons can be checked -- they simply appear pressed in rather than checked
  nsCOMPtr<nsIDOMXULButtonElement> xulButtonElement(do_QueryInterface(mDOMNode));
  if (xulButtonElement) {
    nsAutoString type;
    xulButtonElement->GetType(type);
    if (type.EqualsLiteral("checkbox") || type.EqualsLiteral("radio")) {
      *aState |= nsIAccessibleStates::STATE_CHECKABLE;
      PRBool checked = PR_FALSE;
      PRInt32 checkState = 0;
      xulButtonElement->GetChecked(&checked);
      if (checked) {
        *aState |= nsIAccessibleStates::STATE_PRESSED;
        xulButtonElement->GetCheckState(&checkState);
        if (checkState == nsIDOMXULButtonElement::CHECKSTATE_MIXED) { 
          *aState |= nsIAccessibleStates::STATE_MIXED;
        }
      }
    }
  }

  nsCOMPtr<nsIDOMElement> element(do_QueryInterface(mDOMNode));
  if (element) {
    PRBool isDefault = PR_FALSE;
    element->HasAttribute(NS_LITERAL_STRING("default"), &isDefault) ;
    if (isDefault)
      *aState |= nsIAccessibleStates::STATE_DEFAULT;

    nsAutoString type;
    element->GetAttribute(NS_LITERAL_STRING("type"), type);
    if (type.EqualsLiteral("menu") || type.EqualsLiteral("menu-button")) {
      *aState |= nsIAccessibleStates::STATE_HASPOPUP;
    }
  }

  return NS_OK;
}

void nsXULButtonAccessible::CacheChildren()
{
  // An XUL button accessible may have 1 child dropmarker accessible
  if (!mWeakShell) {
    mAccChildCount = eChildCountUninitialized;
    return;   // This outer doc node has been shut down
  }
  if (mAccChildCount == eChildCountUninitialized) {
    mAccChildCount = 0;  // Avoid reentry
    SetFirstChild(nsnull);
    PRBool allowsAnonChildren = PR_FALSE;
    GetAllowsAnonChildAccessibles(&allowsAnonChildren);
    nsAccessibleTreeWalker walker(mWeakShell, mDOMNode, allowsAnonChildren);
    walker.GetFirstChild();
    nsCOMPtr<nsIAccessible> dropMarkerAccessible;
    while (walker.mState.accessible) {
      dropMarkerAccessible = walker.mState.accessible;
      walker.GetNextSibling();
    }

    // If the anonymous tree walker can find accessible children, 
    // and the last one is a push button, then use it as the only accessible 
    // child -- because this is the scenario where we have a dropmarker child

    if (dropMarkerAccessible) {    
      PRUint32 role;
      if (NS_SUCCEEDED(dropMarkerAccessible->GetRole(&role)) &&
          role == nsIAccessibleRole::ROLE_PUSHBUTTON) {
        SetFirstChild(dropMarkerAccessible);
        nsCOMPtr<nsPIAccessible> privChildAcc = do_QueryInterface(dropMarkerAccessible);
        privChildAcc->SetNextSibling(nsnull);
        privChildAcc->SetParent(this);
        mAccChildCount = 1;
      }
    }
  }
}

/**
  * XUL Dropmarker: can contain arbitrary HTML content
  */

/**
  * Default Constructor
  */
nsXULDropmarkerAccessible::nsXULDropmarkerAccessible(nsIDOMNode* aNode, nsIWeakReference* aShell):
nsFormControlAccessible(aNode, aShell)
{ 
}

/**
  * Only one action available
  */
NS_IMETHODIMP nsXULDropmarkerAccessible::GetNumActions(PRUint8 *aResult)
{
  *aResult = 1;
  return NS_OK;
}

PRBool nsXULDropmarkerAccessible::DropmarkerOpen(PRBool aToggleOpen)
{
  PRBool isOpen = PR_FALSE;

  nsCOMPtr<nsIDOMNode> parentButtonNode;
  mDOMNode->GetParentNode(getter_AddRefs(parentButtonNode));
  nsCOMPtr<nsIDOMXULButtonElement> parentButtonElement(do_QueryInterface(parentButtonNode));

  if (parentButtonElement) {
    parentButtonElement->GetOpen(&isOpen);
    if (aToggleOpen)
      parentButtonElement->SetOpen(!isOpen);
  }
  else {
    nsCOMPtr<nsIDOMXULMenuListElement> parentMenuListElement(do_QueryInterface(parentButtonNode));
    if (parentMenuListElement) {
      parentMenuListElement->GetOpen(&isOpen);
      if (aToggleOpen)
        parentMenuListElement->SetOpen(!isOpen);
    }
  }

  return isOpen;
}

/**
  * Return the name of our only action
  */
NS_IMETHODIMP nsXULDropmarkerAccessible::GetActionName(PRUint8 aIndex, nsAString& aName)
{
  if (aIndex == eAction_Click) {
    if (DropmarkerOpen(PR_FALSE))
      aName.AssignLiteral("close");
    else
      aName.AssignLiteral("open");
    return NS_OK;
  }

  return NS_ERROR_INVALID_ARG;
}

/**
  * Tell the Dropmarker to do it's action
  */
NS_IMETHODIMP nsXULDropmarkerAccessible::DoAction(PRUint8 index)
{
  if (index == eAction_Click) {
    DropmarkerOpen(PR_TRUE); // Reverse the open attribute
    return NS_OK;
  }
  return NS_ERROR_INVALID_ARG;
}

/**
  * We are a pushbutton
  */
NS_IMETHODIMP nsXULDropmarkerAccessible::GetRole(PRUint32 *aResult)
{
  *aResult = nsIAccessibleRole::ROLE_PUSHBUTTON;
  return NS_OK;
}

NS_IMETHODIMP
nsXULDropmarkerAccessible::GetState(PRUint32 *aState, PRUint32 *aExtraState)
{
  *aState = 0;
  if (!mDOMNode) {
    if (aExtraState) {
      *aExtraState = nsIAccessibleStates::EXT_STATE_DEFUNCT;
    }
    return NS_OK;
  }
  if (aExtraState)
    *aExtraState = 0;

  if (DropmarkerOpen(PR_FALSE))
    *aState = nsIAccessibleStates::STATE_PRESSED;

  return NS_OK;
}

/**
  * XUL checkbox
  */

/**
  * Default Constructor
  */
nsXULCheckboxAccessible::nsXULCheckboxAccessible(nsIDOMNode* aNode, nsIWeakReference* aShell):
nsFormControlAccessible(aNode, aShell)
{ 
}

/**
  * We are a CheckButton
  */
NS_IMETHODIMP nsXULCheckboxAccessible::GetRole(PRUint32 *_retval)
{
  *_retval = nsIAccessibleRole::ROLE_CHECKBUTTON;
  return NS_OK;
}

/**
  * Only one action available
  */
NS_IMETHODIMP nsXULCheckboxAccessible::GetNumActions(PRUint8 *_retval)
{
  *_retval = 1;
  return NS_OK;
}

/**
  * Return the name of our only action
  */
NS_IMETHODIMP nsXULCheckboxAccessible::GetActionName(PRUint8 aIndex, nsAString& aName)
{
  if (aIndex == eAction_Click) {
    // check or uncheck
    PRUint32 state;
    GetState(&state, nsnull);

    if (state & nsIAccessibleStates::STATE_CHECKED)
      aName.AssignLiteral("uncheck");
    else
      aName.AssignLiteral("check");

    return NS_OK;
  }
  return NS_ERROR_INVALID_ARG;
}

/**
  * Tell the checkbox to do its only action -- check( or uncheck) itself
  */
NS_IMETHODIMP nsXULCheckboxAccessible::DoAction(PRUint8 index)
{
  if (index == eAction_Click) {
   return DoCommand();
  }
  return NS_ERROR_INVALID_ARG;
}

/**
  * Possible states: focused, focusable, unavailable(disabled), checked
  */
NS_IMETHODIMP
nsXULCheckboxAccessible::GetState(PRUint32 *aState, PRUint32 *aExtraState)
{
  // Get focus and disable status from base class
  nsresult rv = nsFormControlAccessible::GetState(aState, aExtraState);
  NS_ENSURE_SUCCESS(rv, rv);
  if (!mDOMNode)
    return NS_OK;
  
  *aState |= nsIAccessibleStates::STATE_CHECKABLE;
  
  // Determine Checked state
  nsCOMPtr<nsIDOMXULCheckboxElement> xulCheckboxElement(do_QueryInterface(mDOMNode));
  if (xulCheckboxElement) {
    PRBool checked = PR_FALSE;
    xulCheckboxElement->GetChecked(&checked);
    if (checked) {
      *aState |= nsIAccessibleStates::STATE_CHECKED;
      PRInt32 checkState = 0;
      xulCheckboxElement->GetCheckState(&checkState);
      if (checkState == nsIDOMXULCheckboxElement::CHECKSTATE_MIXED)
        *aState |= nsIAccessibleStates::STATE_MIXED;
    }
  }

  return NS_OK;
}

/**
  * XUL groupbox
  */

nsXULGroupboxAccessible::nsXULGroupboxAccessible(nsIDOMNode* aNode, nsIWeakReference* aShell):
nsAccessibleWrap(aNode, aShell)
{ 
}

NS_IMETHODIMP nsXULGroupboxAccessible::GetRole(PRUint32 *aRole)
{
  *aRole = nsIAccessibleRole::ROLE_GROUPING;
  return NS_OK;
}

NS_IMETHODIMP
nsXULGroupboxAccessible::GetName(nsAString& aName)
{
  aName.Truncate();

  nsCOMPtr<nsIAccessible> label;
  GetAccessibleRelated(nsIAccessibleRelation::RELATION_LABELLED_BY,
                       getter_AddRefs(label));
  if (label) {
    return label->GetName(aName);
  }

  return NS_OK;
}

NS_IMETHODIMP
nsXULGroupboxAccessible::GetAccessibleRelated(PRUint32 aRelationType,
                                              nsIAccessible **aRelated)
{
  *aRelated = nsnull;

  nsresult rv = nsAccessibleWrap::GetAccessibleRelated(aRelationType, aRelated);
  if (NS_FAILED(rv) || *aRelated) {
    // Either the node is shut down, or another relation mechanism has been used
    return rv;
  }

  if (aRelationType == nsIAccessibleRelation::RELATION_LABELLED_BY) {
    // The label for xul:groupbox is generated from xul:label that is
    // inside the anonymous content of the xul:caption.
    // The xul:label has an accessible object but the xul:caption does not
    nsCOMPtr<nsIAccessible> testLabelAccessible;
    while (NextChild(testLabelAccessible)) {
      if (Role(testLabelAccessible) == nsIAccessibleRole::ROLE_LABEL) {
        // Ensure that it's our label
        nsCOMPtr<nsIAccessible> testGroupboxAccessible;
        testLabelAccessible->GetAccessibleRelated(nsIAccessibleRelation::RELATION_LABEL_FOR,
                                                  getter_AddRefs(testGroupboxAccessible));
        if (testGroupboxAccessible == this) {
          // The <label> points back to this groupbox
          NS_ADDREF(*aRelated = testLabelAccessible);
          return NS_OK;
        }
      }
    }
  }

  return NS_OK;
}

/**
  * progressmeter
  */
NS_IMPL_ISUPPORTS_INHERITED1(nsXULProgressMeterAccessible, nsFormControlAccessible, nsIAccessibleValue)

nsXULProgressMeterAccessible::nsXULProgressMeterAccessible(nsIDOMNode* aNode, nsIWeakReference* aShell):
nsFormControlAccessible(aNode, aShell)
{ 
}

NS_IMETHODIMP nsXULProgressMeterAccessible::GetRole(PRUint32 *_retval)
{
  *_retval = nsIAccessibleRole::ROLE_PROGRESSBAR;
  return NS_OK;
}

NS_IMETHODIMP nsXULProgressMeterAccessible::GetValue(nsAString& aValue)
{
  aValue.Truncate();
  nsAccessible::GetValue(aValue);
  if (!aValue.IsEmpty()) {
    return NS_OK;
  }
  nsCOMPtr<nsIContent> content(do_QueryInterface(mDOMNode));
  if (!content) {
    return NS_ERROR_FAILURE;
  }
  content->GetAttr(kNameSpaceID_None, nsAccessibilityAtoms::value, aValue);
  if (aValue.IsEmpty()) {
    aValue.AppendLiteral("0");  // Empty value for progress meter = 0%
  }
  aValue.AppendLiteral("%");
  return NS_OK;
}

NS_IMETHODIMP nsXULProgressMeterAccessible::GetMaximumValue(double *aMaximumValue)
{
  *aMaximumValue = 1; // 100% = 1;
  return NS_OK;
}

NS_IMETHODIMP nsXULProgressMeterAccessible::GetMinimumValue(double *aMinimumValue)
{
  *aMinimumValue = 0;
  return NS_OK;
}

NS_IMETHODIMP nsXULProgressMeterAccessible::GetMinimumIncrement(double *aMinimumIncrement)
{
  *aMinimumIncrement = 0;
  return NS_OK;
}

NS_IMETHODIMP nsXULProgressMeterAccessible::GetCurrentValue(double *aCurrentValue)
{
  *aCurrentValue = 0;
  nsAutoString currentValue;
  nsCOMPtr<nsIContent> content(do_QueryInterface(mDOMNode));
  if (!content) {
    return NS_ERROR_FAILURE;
  }
  content->GetAttr(kNameSpaceID_None, nsAccessibilityAtoms::value, currentValue);

  PRInt32 error;
  *aCurrentValue = currentValue.ToFloat(&error) / 100;
  return NS_OK;
}

NS_IMETHODIMP nsXULProgressMeterAccessible::SetCurrentValue(double aValue)
{
  return NS_ERROR_FAILURE; // Progress meters are readonly!
}


/**
  * XUL Radio Button
  */

/** Constructor */
nsXULRadioButtonAccessible::nsXULRadioButtonAccessible(nsIDOMNode* aNode, nsIWeakReference* aShell):
nsRadioButtonAccessible(aNode, aShell)
{ 
}

/** We are Focusable and can be Checked and focused */
NS_IMETHODIMP
nsXULRadioButtonAccessible::GetState(PRUint32 *aState, PRUint32 *aExtraState)
{
  nsresult rv = nsFormControlAccessible::GetState(aState, aExtraState);
  NS_ENSURE_SUCCESS(rv, rv);
  if (!mDOMNode)
    return NS_OK;

  *aState |= nsIAccessibleStates::STATE_CHECKABLE;
  
  PRBool selected = PR_FALSE;   // Radio buttons can be selected

  nsCOMPtr<nsIDOMXULSelectControlItemElement> radioButton(do_QueryInterface(mDOMNode));
  if (radioButton) {
    radioButton->GetSelected(&selected);
    if (selected) {
      *aState |= nsIAccessibleStates::STATE_CHECKED;
    }
  }

  return NS_OK;
}

nsresult
nsXULRadioButtonAccessible::GetAttributesInternal(nsIPersistentProperties *aAttributes)
{
  NS_ENSURE_ARG_POINTER(aAttributes);
  NS_ENSURE_TRUE(mDOMNode, NS_ERROR_FAILURE);

  nsresult rv = nsFormControlAccessible::GetAttributesInternal(aAttributes);
  NS_ENSURE_SUCCESS(rv, rv);

  nsAccUtils::SetAccAttrsForXULSelectControlItem(mDOMNode, aAttributes);

  return NS_OK;
}

/**
  * XUL Radio Group
  *   The Radio Group proxies for the Radio Buttons themselves. The Group gets
  *   focus whereas the Buttons do not. So we only have an accessible object for
  *   this for the purpose of getting the proper RadioButton. Need this here to 
  *   avoid circular reference problems when navigating the accessible tree and
  *   for getting to the radiobuttons.
  */

/** Constructor */
nsXULRadioGroupAccessible::nsXULRadioGroupAccessible(nsIDOMNode* aNode, nsIWeakReference* aShell):
nsXULSelectableAccessible(aNode, aShell)
{ 
}

NS_IMETHODIMP nsXULRadioGroupAccessible::GetRole(PRUint32 *_retval)
{
  *_retval = nsIAccessibleRole::ROLE_GROUPING;
  return NS_OK;
}

NS_IMETHODIMP
nsXULRadioGroupAccessible::GetState(PRUint32 *aState, PRUint32 *aExtraState)
{
  // The radio group is not focusable.
  // Sometimes the focus controller will report that it is focused.
  // That means that the actual selected radio button should be considered focused
  nsresult rv = nsAccessible::GetState(aState, aExtraState);
  NS_ENSURE_SUCCESS(rv, rv);
  if (mDOMNode) {
    *aState &= ~(nsIAccessibleStates::STATE_FOCUSABLE |
                 nsIAccessibleStates::STATE_FOCUSED);
  }

  return NS_OK;
}
/**
  * XUL StatusBar: can contain arbitrary HTML content
  */

/**
  * Default Constructor
  */
nsXULStatusBarAccessible::nsXULStatusBarAccessible(nsIDOMNode* aNode, nsIWeakReference* aShell):
nsAccessibleWrap(aNode, aShell)
{ 
}

/**
  * We are a statusbar
  */
NS_IMETHODIMP nsXULStatusBarAccessible::GetRole(PRUint32 *_retval)
{
  *_retval = nsIAccessibleRole::ROLE_STATUSBAR;
  return NS_OK;
}

/**
  * XUL Toolbar Button
  */
nsXULToolbarButtonAccessible::nsXULToolbarButtonAccessible(nsIDOMNode* aNode, nsIWeakReference* aShell):
nsXULButtonAccessible(aNode, aShell)
{
}

nsresult
nsXULToolbarButtonAccessible::GetAttributesInternal(nsIPersistentProperties *aAttributes)
{
  NS_ENSURE_ARG_POINTER(aAttributes);
  NS_ENSURE_TRUE(mDOMNode, NS_ERROR_FAILURE);

  nsresult rv = nsXULButtonAccessible::GetAttributesInternal(aAttributes);
  NS_ENSURE_SUCCESS(rv, rv);

  nsCOMPtr<nsIAccessible> parent(GetParent());
  PRInt32 setSize = 0;
  PRInt32 posInSet = 0;

  if (parent) {
    nsCOMPtr<nsIAccessible> sibling;
    nsCOMPtr<nsIAccessible> tempSibling;
    parent->GetFirstChild(getter_AddRefs(sibling));
    while (sibling) {
      if (IsSeparator(sibling)) { // end of a group of buttons
        if (posInSet)
          break; // we've found our group, so we're done
        setSize = 0; // not our group, so start a new group
      } else {
        setSize++; // another button in the group
        if (sibling == this)
          posInSet = setSize; // we've found our button
      }
      sibling->GetNextSibling(getter_AddRefs(tempSibling));
      sibling.swap(tempSibling);
    }
  }
  
  nsAccUtils::SetAccGroupAttrs(aAttributes, 0, posInSet, setSize);

  return NS_OK;
}

PRBool
nsXULToolbarButtonAccessible::IsSeparator(nsIAccessible *aAccessible)
{
  nsCOMPtr<nsIDOMNode> domNode;
  nsCOMPtr<nsIAccessNode> accessNode(do_QueryInterface(aAccessible));
  accessNode->GetDOMNode(getter_AddRefs(domNode));
  nsCOMPtr<nsIContent> contentDomNode(do_QueryInterface(domNode));

  if (!contentDomNode)
    return PR_FALSE;

  return (contentDomNode->Tag() == nsAccessibilityAtoms::toolbarseparator) ||
         (contentDomNode->Tag() == nsAccessibilityAtoms::toolbarspacer) ||
         (contentDomNode->Tag() == nsAccessibilityAtoms::toolbarspring);
}

/**
  * XUL ToolBar
  */

nsXULToolbarAccessible::nsXULToolbarAccessible(nsIDOMNode* aNode, nsIWeakReference* aShell):
nsAccessibleWrap(aNode, aShell)
{ 
}

NS_IMETHODIMP nsXULToolbarAccessible::GetRole(PRUint32 *_retval)
{
  *_retval = nsIAccessibleRole::ROLE_TOOLBAR;
  return NS_OK;
}

/**
  * XUL Toolbar Separator
  */

nsXULToolbarSeparatorAccessible::nsXULToolbarSeparatorAccessible(nsIDOMNode* aNode, nsIWeakReference* aShell):
nsLeafAccessible(aNode, aShell)
{ 
}

NS_IMETHODIMP nsXULToolbarSeparatorAccessible::GetRole(PRUint32 *_retval)
{
  *_retval = nsIAccessibleRole::ROLE_SEPARATOR;
  return NS_OK;
}

NS_IMETHODIMP
nsXULToolbarSeparatorAccessible::GetState(PRUint32 *aState,
                                          PRUint32 *aExtraState)
{
  *aState = 0;  // no special state flags for toolbar separator
  if (aExtraState) {
    *aExtraState = mDOMNode ? 0 : nsIAccessibleStates::EXT_STATE_DEFUNCT;
  }
  return NS_OK;
}

/**
  * XUL Textfield
  */

nsXULTextFieldAccessible::nsXULTextFieldAccessible(nsIDOMNode* aNode, nsIWeakReference* aShell) :
 nsHyperTextAccessibleWrap(aNode, aShell)
{
}

NS_IMPL_ISUPPORTS_INHERITED3(nsXULTextFieldAccessible, nsAccessible, nsHyperTextAccessible, nsIAccessibleText, nsIAccessibleEditableText)

NS_IMETHODIMP nsXULTextFieldAccessible::GetValue(nsAString& aValue)
{
  PRUint32 state;
  GetState(&state, nsnull);
  if (state & nsIAccessibleStates::STATE_PROTECTED)    // Don't return password text!
    return NS_ERROR_FAILURE;

  nsCOMPtr<nsIDOMXULTextBoxElement> textBox(do_QueryInterface(mDOMNode));
  if (textBox) {
    return textBox->GetValue(aValue);
  }
  nsCOMPtr<nsIDOMXULMenuListElement> menuList(do_QueryInterface(mDOMNode));
  if (menuList) {
    return menuList->GetLabel(aValue);
  }
  return NS_ERROR_FAILURE;
}

already_AddRefed<nsIDOMNode> nsXULTextFieldAccessible::GetInputField()
{
  nsIDOMNode *inputField = nsnull;
  nsCOMPtr<nsIDOMXULTextBoxElement> textBox = do_QueryInterface(mDOMNode);
  if (textBox) {
    textBox->GetInputField(&inputField);
    return inputField;
  }
  nsCOMPtr<nsIDOMXULMenuListElement> menuList = do_QueryInterface(mDOMNode);
  if (menuList) {   // <xul:menulist droppable="false">
    menuList->GetInputField(&inputField);
  }
  NS_ASSERTION(inputField, "No input field for nsXULTextFieldAccessible");
  return inputField;
}

NS_IMETHODIMP
nsXULTextFieldAccessible::GetState(PRUint32 *aState, PRUint32 *aExtraState)
{
  nsresult rv = nsHyperTextAccessibleWrap::GetState(aState, aExtraState);
  NS_ENSURE_SUCCESS(rv, rv);
  if (!mDOMNode)
    return NS_OK;

  nsCOMPtr<nsIDOMNode> inputField = GetInputField();
  NS_ENSURE_TRUE(inputField, NS_ERROR_FAILURE);

  // Create a temporary accessible from the HTML text field
  // to get the accessible state from. Doesn't add to cache
  // because Init() is not called.
  nsHTMLTextFieldAccessible* tempAccessible =
    new nsHTMLTextFieldAccessible(inputField, mWeakShell);
  if (!tempAccessible)
    return NS_ERROR_OUT_OF_MEMORY;
  nsCOMPtr<nsIAccessible> kungFuDeathGrip = tempAccessible;
  rv = tempAccessible->GetState(aState, nsnull);
  NS_ENSURE_SUCCESS(rv, rv);

  if (gLastFocusedNode == mDOMNode) {
    *aState |= nsIAccessibleStates::STATE_FOCUSED;
  }

  nsCOMPtr<nsIContent> content(do_QueryInterface(mDOMNode));
  NS_ASSERTION(content, "Not possible since we have an mDOMNode");

  nsCOMPtr<nsIDOMXULMenuListElement> menuList(do_QueryInterface(mDOMNode));
  if (menuList) {
    // <xul:menulist droppable="false">
    if (!content->AttrValueIs(kNameSpaceID_None, nsAccessibilityAtoms::editable,
                              nsAccessibilityAtoms::_true, eIgnoreCase)) {
      *aState |= nsIAccessibleStates::STATE_READONLY;
    }
  }
  else {
    // <xul:textbox>
    if (content->AttrValueIs(kNameSpaceID_None, nsAccessibilityAtoms::type,
                             nsAccessibilityAtoms::password, eIgnoreCase)) {
      *aState |= nsIAccessibleStates::STATE_PROTECTED;
    }
    if (content->AttrValueIs(kNameSpaceID_None, nsAccessibilityAtoms::readonly,
                             nsAccessibilityAtoms::_true, eIgnoreCase)) {
      *aState |= nsIAccessibleStates::STATE_READONLY;
    }
  }

  if (!aExtraState)
    return NS_OK;

  PRBool isMultiLine = content->HasAttr(kNameSpaceID_None,
                                        nsAccessibilityAtoms::multiline);

  if (isMultiLine) {
    *aExtraState |= nsIAccessibleStates::EXT_STATE_MULTI_LINE;
  }
  else {
    *aExtraState |= nsIAccessibleStates::EXT_STATE_SINGLE_LINE;
  }

  return NS_OK;
}

NS_IMETHODIMP nsXULTextFieldAccessible::GetRole(PRUint32 *aRole)
{
  *aRole = nsIAccessibleRole::ROLE_ENTRY;
  nsCOMPtr<nsIContent> content(do_QueryInterface(mDOMNode));
  if (content &&
      content->AttrValueIs(kNameSpaceID_None, nsAccessibilityAtoms::type,
                           nsAccessibilityAtoms::password, eIgnoreCase)) {
    *aRole = nsIAccessibleRole::ROLE_PASSWORD_TEXT;
  }
  return NS_OK;
}


/**
  * Only one actions available
  */
NS_IMETHODIMP nsXULTextFieldAccessible::GetNumActions(PRUint8 *_retval)
{
  *_retval = 1;
  return NS_OK;
}

/**
  * Return the name of our only action
  */
NS_IMETHODIMP nsXULTextFieldAccessible::GetActionName(PRUint8 aIndex, nsAString& aName)
{
  if (aIndex == eAction_Click) {
    aName.AssignLiteral("activate"); 
    return NS_OK;
  }
  return NS_ERROR_INVALID_ARG;
}

/**
  * Tell the button to do it's action
  */
NS_IMETHODIMP nsXULTextFieldAccessible::DoAction(PRUint8 index)
{
  if (index == 0) {
    nsCOMPtr<nsIDOMXULElement> element(do_QueryInterface(mDOMNode));
    if (element)
    {
      element->Focus();
      return NS_OK;
    }
    return NS_ERROR_FAILURE;
  }
  return NS_ERROR_INVALID_ARG;
}

NS_IMETHODIMP
nsXULTextFieldAccessible::GetAllowsAnonChildAccessibles(PRBool *aAllowsAnonChildren)
{
  *aAllowsAnonChildren = PR_FALSE;
  return NS_OK;
}

NS_IMETHODIMP nsXULTextFieldAccessible::GetAssociatedEditor(nsIEditor **aEditor)
{
  *aEditor = nsnull;
  nsCOMPtr<nsIDOMNode> inputField = GetInputField();
  nsCOMPtr<nsIDOMNSEditableElement> editableElt(do_QueryInterface(inputField));
  NS_ENSURE_TRUE(editableElt, NS_ERROR_FAILURE);
  return editableElt->GetEditor(aEditor);
}
