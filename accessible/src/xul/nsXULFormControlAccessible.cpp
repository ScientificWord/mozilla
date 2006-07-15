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
#include "nsIDOMHTMLInputElement.h"
#include "nsIDOMXULButtonElement.h"
#include "nsIDOMXULCheckboxElement.h"
#include "nsIDOMXULMenuListElement.h"
#include "nsIDOMXULSelectCntrlItemEl.h"
#include "nsIDOMXULTextboxElement.h"
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
  *_retval = eSingle_Action;
  return NS_OK;
}

/**
  * Return the name of our only action
  */
NS_IMETHODIMP nsXULButtonAccessible::GetActionName(PRUint8 index, nsAString& _retval)
{
  if (index == eAction_Click) {
    nsAccessible::GetTranslatedString(NS_LITERAL_STRING("press"), _retval); 
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
  *_retval = ROLE_PUSHBUTTON;
  return NS_OK;
}

/**
  * Possible states: focused, focusable, unavailable(disabled)
  */
NS_IMETHODIMP nsXULButtonAccessible::GetState(PRUint32 *aState)
{
  // get focus and disable status from base class
  nsAccessible::GetState(aState);

  PRBool disabled = PR_FALSE;
  nsCOMPtr<nsIDOMXULControlElement> xulFormElement(do_QueryInterface(mDOMNode));  
  if (xulFormElement) {
    xulFormElement->GetDisabled(&disabled);
    if (disabled)
      *aState |= STATE_UNAVAILABLE;
    else 
      *aState |= STATE_FOCUSABLE;
  }

  // Buttons can be checked -- they simply appear pressed in rather than checked
  nsCOMPtr<nsIDOMXULButtonElement> xulButtonElement(do_QueryInterface(mDOMNode));
  if (xulButtonElement) {
    PRBool checked = PR_FALSE;
    PRInt32 checkState = 0;
    xulButtonElement->GetChecked(&checked);
    if (checked) {
      *aState |= STATE_PRESSED;
      xulButtonElement->GetCheckState(&checkState);
      if (checkState == nsIDOMXULButtonElement::CHECKSTATE_MIXED)  
        *aState |= STATE_MIXED;
    }
  }

  nsCOMPtr<nsIDOMElement> element(do_QueryInterface(mDOMNode));
  if (element) {
    PRBool isDefault = PR_FALSE;
    element->HasAttribute(NS_LITERAL_STRING("default"), &isDefault) ;
    if (isDefault)
      *aState |= STATE_DEFAULT;

    nsAutoString type;
    element->GetAttribute(NS_LITERAL_STRING("type"), type);
    if (type.EqualsLiteral("menu") || type.EqualsLiteral("menu-button")) {
      *aState |= STATE_HASPOPUP;
    }
  }

  return NS_OK;
}

void nsXULButtonAccessible::CacheChildren(PRBool aWalkAnonContent)
{
  // An XUL button accessible may have 1 child dropmarker accessible
  if (!mWeakShell) {
    mAccChildCount = eChildCountUninitialized;
    return;   // This outer doc node has been shut down
  }
  if (mAccChildCount == eChildCountUninitialized) {
    mAccChildCount = 0;
    SetFirstChild(nsnull);
    nsAccessibleTreeWalker walker(mWeakShell, mDOMNode, PR_TRUE);
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
      if (NS_SUCCEEDED(dropMarkerAccessible->GetRole(&role)) && role == ROLE_PUSHBUTTON) {
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
  * Only one actions available
  */
NS_IMETHODIMP nsXULDropmarkerAccessible::GetNumActions(PRUint8 *aResult)
{
  *aResult = eSingle_Action;
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
NS_IMETHODIMP nsXULDropmarkerAccessible::GetActionName(PRUint8 index, nsAString& aResult)
{
  if (index == eAction_Click) {
    if (DropmarkerOpen(PR_FALSE))
      aResult.AssignLiteral("close");
    else
      aResult.AssignLiteral("open");
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
  *aResult = ROLE_PUSHBUTTON;
  return NS_OK;
}

NS_IMETHODIMP nsXULDropmarkerAccessible::GetState(PRUint32 *aResult)
{
  *aResult = 0;
  
  if (DropmarkerOpen(PR_FALSE))
    *aResult = STATE_PRESSED;

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
  *_retval = ROLE_CHECKBUTTON;
  return NS_OK;
}

/**
  * Only one action available
  */
NS_IMETHODIMP nsXULCheckboxAccessible::GetNumActions(PRUint8 *_retval)
{
  *_retval = eSingle_Action;
  return NS_OK;
}

/**
  * Return the name of our only action
  */
NS_IMETHODIMP nsXULCheckboxAccessible::GetActionName(PRUint8 index, nsAString& _retval)
{
  if (index == eAction_Click) {
    // check or uncheck
    PRUint32 state;
    GetState(&state);

    if (state & STATE_CHECKED)
      _retval.AssignLiteral("uncheck");
    else
      _retval.AssignLiteral("check");

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
NS_IMETHODIMP nsXULCheckboxAccessible::GetState(PRUint32 *_retval)
{
  // Get focus and disable status from base class
  nsFormControlAccessible::GetState(_retval);

  // Determine Checked state
  nsCOMPtr<nsIDOMXULCheckboxElement> xulCheckboxElement(do_QueryInterface(mDOMNode));
  if (xulCheckboxElement) {
    PRBool checked = PR_FALSE;
    xulCheckboxElement->GetChecked(&checked);
    if (checked) {
      *_retval |= STATE_CHECKED;
      PRInt32 checkState = 0;
      xulCheckboxElement->GetCheckState(&checkState);
      if (checkState == nsIDOMXULCheckboxElement::CHECKSTATE_MIXED)   
        *_retval |= STATE_MIXED;
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

NS_IMETHODIMP nsXULGroupboxAccessible::GetRole(PRUint32 *_retval)
{
  *_retval = ROLE_GROUPING;
  return NS_OK;
}

NS_IMETHODIMP nsXULGroupboxAccessible::GetState(PRUint32 *_retval)
{
  // Groupbox doesn't support focusable state!
  nsAccessible::GetState(_retval);
  *_retval &= ~STATE_FOCUSABLE;

  return NS_OK;
}

NS_IMETHODIMP nsXULGroupboxAccessible::GetName(nsAString& aName)
{
  aName.Truncate();  // Default name is blank 

  if (mRoleMapEntry) {
    nsAccessible::GetName(aName);
    if (!aName.IsEmpty()) {
      return NS_OK;
    }
  }
  nsCOMPtr<nsIDOMElement> element(do_QueryInterface(mDOMNode));
  if (element) {
    nsCOMPtr<nsIDOMNodeList> captions;
    nsAutoString nameSpaceURI;
    element->GetNamespaceURI(nameSpaceURI);
    element->GetElementsByTagNameNS(nameSpaceURI, NS_LITERAL_STRING("caption"), 
                                    getter_AddRefs(captions));
    if (captions) {
      nsCOMPtr<nsIDOMNode> captionNode;
      captions->Item(0, getter_AddRefs(captionNode));
      if (captionNode) {
        element = do_QueryInterface(captionNode);
        NS_ASSERTION(element, "No nsIDOMElement for caption node!");
        element->GetAttribute(NS_LITERAL_STRING("label"), aName) ;
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
  *_retval = ROLE_PROGRESSBAR;
  return NS_OK;
}

NS_IMETHODIMP nsXULProgressMeterAccessible::GetState(PRUint32 *aState)
{
  nsresult rv = nsAccessible::GetState(aState);
  *aState &= ~STATE_FOCUSABLE; // Progress meters are not focusable
  return rv;
}

NS_IMETHODIMP nsXULProgressMeterAccessible::GetValue(nsAString& aValue)
{
  aValue.Truncate();
  nsAccessible::GetValue(aValue);
  if (!aValue.IsEmpty()) {
    return NS_OK;
  }
  nsCOMPtr<nsIDOMElement> element(do_QueryInterface(mDOMNode));
  NS_ASSERTION(element, "No element for DOM node!");
  element->GetAttribute(NS_LITERAL_STRING("value"), aValue);
  if (!aValue.IsEmpty() && aValue.Last() != '%')
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
  nsAutoString currentValue;
  GetValue(currentValue);
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

/** Our only action is to click */
NS_IMETHODIMP nsXULRadioButtonAccessible::DoAction(PRUint8 index)
{
  if (index == eAction_Click) {
    return DoCommand();
  }
  return NS_ERROR_INVALID_ARG;
}

/** We are Focusable and can be Checked and focused */
NS_IMETHODIMP nsXULRadioButtonAccessible::GetState(PRUint32 *_retval)
{
  nsFormControlAccessible::GetState(_retval);
  PRBool selected = PR_FALSE;   // Radio buttons can be selected

  nsCOMPtr<nsIDOMXULSelectControlItemElement> radioButton(do_QueryInterface(mDOMNode));
  if (radioButton) {
    radioButton->GetSelected(&selected);
    if (selected) {
      *_retval |= STATE_CHECKED;
    }
  }

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
  *_retval = ROLE_GROUPING;
  return NS_OK;
}

NS_IMETHODIMP nsXULRadioGroupAccessible::GetState(PRUint32 *_retval)
{
  // The radio group is not focusable.
  // Sometimes the focus controller will report that it is focused.
  // That means that the actual selected radio button should be considered focused
  nsAccessible::GetState(_retval);
  *_retval &= ~(STATE_FOCUSABLE | STATE_FOCUSED);
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
  *_retval = ROLE_STATUSBAR;
  return NS_OK;
}

NS_IMETHODIMP nsXULStatusBarAccessible::GetState(PRUint32 *_retval)
{
  return nsAccessible::GetState(_retval);
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
  *_retval = ROLE_TOOLBAR;
  return NS_OK;
}

NS_IMETHODIMP nsXULToolbarAccessible::GetState(PRUint32 *_retval)
{
  nsAccessible::GetState(_retval);
  *_retval &= ~STATE_FOCUSABLE;  // toolbar is not focusable
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
  *_retval = ROLE_SEPARATOR;
  return NS_OK;
}

NS_IMETHODIMP nsXULToolbarSeparatorAccessible::GetState(PRUint32 *_retval)
{
  *_retval = 0;  // no special state flags for toolbar separator
  return NS_OK;
}

/**
  * XUL Textfield
  */

nsXULTextFieldAccessible::nsXULTextFieldAccessible(nsIDOMNode* aNode, nsIWeakReference* aShell) :
 nsHyperTextAccessible(aNode, aShell)
{
}

NS_IMPL_ISUPPORTS_INHERITED2(nsXULTextFieldAccessible, nsAccessible, nsIAccessibleText, nsIAccessibleEditableText)

NS_IMETHODIMP nsXULTextFieldAccessible::Init()
{
  CheckForEditor();
  return nsHyperTextAccessible::Init();
}

NS_IMETHODIMP nsXULTextFieldAccessible::Shutdown()
{
  if (mEditor) {
    mEditor->RemoveEditActionListener(this);
    mEditor = nsnull;
  }
  return nsHyperTextAccessible::Shutdown();
}

NS_IMETHODIMP nsXULTextFieldAccessible::GetValue(nsAString& aValue)
{
  PRUint32 state;
  GetState(&state);
  if (state & STATE_PROTECTED)    // Don't return password text!
    return NS_ERROR_FAILURE;

  nsCOMPtr<nsIDOMXULTextBoxElement> textBox(do_QueryInterface(mDOMNode));
  if (textBox) {
    return textBox->GetValue(aValue);
  }
  return NS_ERROR_FAILURE;
}

NS_IMETHODIMP nsXULTextFieldAccessible::GetExtState(PRUint32 *aExtState)
{
  nsresult rv = nsHyperTextAccessible::GetExtState(aExtState);
  if (NS_FAILED(rv)) {
    return rv;
  }
  nsCOMPtr<nsIContent> content(do_QueryInterface(mDOMNode));
  NS_ASSERTION(content, "Should not have gotten past nsAccessible::GetExtState() without node");

  PRBool isMultiLine = content->HasAttr(kNameSpaceID_None, nsAccessibilityAtoms::multiline);
  *aExtState |= (isMultiLine ? EXT_STATE_MULTI_LINE : EXT_STATE_SINGLE_LINE);

  PRUint32 state;
  GetState(&state);
  const PRUint32 kNonEditableStates = STATE_READONLY | STATE_UNAVAILABLE;
  if (0 == (state & kNonEditableStates)) {
    *aExtState |= EXT_STATE_EDITABLE;
  }

  return NS_OK;
}

NS_IMETHODIMP nsXULTextFieldAccessible::GetState(PRUint32 *aState)
{
  nsCOMPtr<nsIDOMXULTextBoxElement> textBox(do_QueryInterface(mDOMNode));
  if (!textBox) {
    return NS_ERROR_FAILURE;
  }
  nsHyperTextAccessible::GetState(aState);

  nsCOMPtr<nsIDOMNode> inputField;
  textBox->GetInputField(getter_AddRefs(inputField));
  if (!inputField) {
    return NS_ERROR_FAILURE;
  }
  // Create a temporary accessible from the HTML text field
  // to get the accessible state from. Doesn't add to cache
  // because Init() is not called.
  nsHTMLTextFieldAccessible tempAccessible(inputField, mWeakShell);
  nsresult rv = tempAccessible.GetState(aState);
  if (gLastFocusedNode == mDOMNode) {
    *aState |= STATE_FOCUSED;
  }

  nsCOMPtr<nsIContent> content(do_QueryInterface(mDOMNode));
  NS_ASSERTION(content, "Not possible since we are a nsIDOMXULTextBoxElement");
  if (content->AttrValueIs(kNameSpaceID_None, nsAccessibilityAtoms::type,
                           nsAccessibilityAtoms::password, eIgnoreCase)) {
    *aState |= STATE_PROTECTED;
  }

  if (content->AttrValueIs(kNameSpaceID_None, nsAccessibilityAtoms::readonly,
                           nsAccessibilityAtoms::_true, eIgnoreCase)) {
    *aState |= STATE_READONLY;
  }

  return rv;
}

NS_IMETHODIMP nsXULTextFieldAccessible::GetRole(PRUint32 *aRole)
{
  *aRole = ROLE_ENTRY;
  nsCOMPtr<nsIContent> content(do_QueryInterface(mDOMNode));
  if (content &&
      content->AttrValueIs(kNameSpaceID_None, nsAccessibilityAtoms::type,
                           nsAccessibilityAtoms::password, eIgnoreCase)) {
    *aRole = ROLE_PASSWORD_TEXT;
  }
  return NS_OK;
}


/**
  * Only one actions available
  */
NS_IMETHODIMP nsXULTextFieldAccessible::GetNumActions(PRUint8 *_retval)
{
  *_retval = eSingle_Action;
  return NS_OK;
}

/**
  * Return the name of our only action
  */
NS_IMETHODIMP nsXULTextFieldAccessible::GetActionName(PRUint8 index, nsAString& _retval)
{
  if (index == eAction_Click) {
    nsAccessible::GetTranslatedString(NS_LITERAL_STRING("activate"), _retval); 
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

void nsXULTextFieldAccessible::SetEditor(nsIEditor* aEditor)
{
  mEditor = aEditor;
  if (mEditor)
    mEditor->AddEditActionListener(this);
}

void nsXULTextFieldAccessible::CheckForEditor()
{
  nsCOMPtr<nsIDOMXULTextBoxElement> textBox(do_QueryInterface(mDOMNode));
  if (!textBox) {
    return;
  }

  nsCOMPtr<nsIDOMNode> inputField;
  textBox->GetInputField(getter_AddRefs(inputField));
  nsCOMPtr<nsIContent> inputContent = do_QueryInterface(inputField);
  if (!inputContent) {
    return;
  }
  
  nsCOMPtr<nsIPresShell> presShell = GetPresShell();
  nsIFrame *inputFrame = presShell->GetPrimaryFrameFor(inputContent);
  if (!inputFrame) {
    return;
  }
  
  nsITextControlFrame *textFrame;
  inputFrame->QueryInterface(NS_GET_IID(nsITextControlFrame), (void**)&textFrame);
  if (textFrame) {
    nsCOMPtr<nsIEditor> editor;
    textFrame->GetEditor(getter_AddRefs(editor));
    SetEditor(editor);
  }
}
