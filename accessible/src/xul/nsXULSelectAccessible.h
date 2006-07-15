/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
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
#ifndef __nsXULSelectAccessible_h__
#define __nsXULSelectAccessible_h__

#include "nsCOMPtr.h"
#include "nsIAccessibleSelectable.h"
#include "nsXULMenuAccessible.h"

class nsIWeakReference;

/**
  * Selects, Listboxes and Comboboxes, are made up of a number of different
  *  widgets, some of which are shared between the two. This file contains 
  *  all of the widgets for both of the Selects, for XUL only.
  *
  *  Listbox:
  *     - nsXULListboxAccessible
  *        - nsXULSelectListAccessible
  *           - nsXULSelectOptionAccessible
  *
  *  Comboboxes:
  *     - nsXULComboboxAccessible      <menulist />
  *        - nsHTMLTextFieldAccessible
  *        - nsXULComboboxButtonAccessible    
  *        - nsXULSelectListAccessible      <menupopup />
  *           - nsXULSelectOptionAccessible(s)   <menuitem />
  */

/** ------------------------------------------------------ */
/**  First, the common widgets                             */
/** ------------------------------------------------------ */

/*
 * The basic implemetation of nsIAccessibleSelectable.
 */
class nsXULSelectableAccessible : public nsAccessibleWrap
{
public:
  NS_DECL_ISUPPORTS_INHERITED
  NS_DECL_NSIACCESSIBLESELECTABLE

  nsXULSelectableAccessible(nsIDOMNode* aDOMNode, nsIWeakReference* aShell);
  virtual ~nsXULSelectableAccessible() {}

  NS_IMETHOD GetName(nsAString& _retval);

protected:
  NS_IMETHOD ChangeSelection(PRInt32 aIndex, PRUint8 aMethod, PRBool *aSelState);
  nsresult AppendFlatStringFromSubtree(nsIContent *aContent, nsAString *aFlatString)
    { return NS_ERROR_FAILURE; }  // Overrides base impl in nsAccessible
};

/*
 * The list that contains all the options in the select.
 */
class nsXULSelectListAccessible : public nsAccessibleWrap
{
public:
  
  nsXULSelectListAccessible(nsIDOMNode* aDOMNode, nsIWeakReference* aShell);
  virtual ~nsXULSelectListAccessible() {}

  /* ----- nsIAccessible ----- */
  NS_IMETHOD GetRole(PRUint32 *_retval);
  NS_IMETHOD GetState(PRUint32 *_retval);
};

/*
 * Options inside the select, contained within the list
 */
class nsXULSelectOptionAccessible : public nsXULMenuitemAccessible
{
public:
  
  nsXULSelectOptionAccessible(nsIDOMNode* aDOMNode, nsIWeakReference* aShell);
  virtual ~nsXULSelectOptionAccessible() {}

  /* ----- nsIAccessible ----- */
  NS_IMETHOD GetRole(PRUint32 *_retval);
  NS_IMETHOD GetState(PRUint32 *_retval);
  nsIFrame*  GetBoundsFrame();
};

/** ------------------------------------------------------ */
/**  Secondly, the Listbox widget                          */
/** ------------------------------------------------------ */

/*
 * A class the represents the XUL Listbox widget.
 */
class nsXULListboxAccessible : public nsXULSelectableAccessible
{
public:

  nsXULListboxAccessible(nsIDOMNode* aDOMNode, nsIWeakReference* aShell);
  virtual ~nsXULListboxAccessible() {}

  /* ----- nsIAccessible ----- */
  NS_IMETHOD GetRole(PRUint32 *_retval);
  NS_IMETHOD GetState(PRUint32 *_retval);
  NS_IMETHOD GetValue(nsAString& _retval);

};

/**
  * Listitems -- used in listboxes 
  */
class nsXULListitemAccessible : public nsXULMenuitemAccessible
{
public:
  NS_DECL_ISUPPORTS_INHERITED
  
  nsXULListitemAccessible(nsIDOMNode* aDOMNode, nsIWeakReference* aShell);
  virtual ~nsXULListitemAccessible() {}

  /* ----- nsIAccessible ----- */
  NS_IMETHOD GetName(nsAString& _retval);
  NS_IMETHOD GetRole(PRUint32 *_retval);
  NS_IMETHOD GetState(PRUint32 *_retval);
  NS_IMETHOD GetActionName(PRUint8 index, nsAString& _retval);
  // Don't use XUL menu's special child aggregator, this can be a rich list item
  NS_IMETHOD GetChildCount(PRInt32 *aAccChildCount) { return nsAccessibleWrap::GetChildCount(aAccChildCount); }

private:
  PRBool mIsCheckbox;
};

/** ------------------------------------------------------ */
/**  Finally, the Combobox widgets                         */
/** ------------------------------------------------------ */

/*
 * A class the represents the XUL Combobox widget.
 */
class nsXULComboboxAccessible : public nsXULSelectableAccessible
{
public:

  nsXULComboboxAccessible(nsIDOMNode* aDOMNode, nsIWeakReference* aShell);
  virtual ~nsXULComboboxAccessible() {}

  /* ----- nsIAccessible ----- */
  NS_IMETHOD GetChildCount(PRInt32 *_retval);
  NS_IMETHOD GetRole(PRUint32 *_retval);
  NS_IMETHOD GetState(PRUint32 *_retval);

  NS_IMETHOD GetValue(nsAString& _retval);
  NS_IMETHOD GetDescription(nsAString& aDescription);
};

#endif
