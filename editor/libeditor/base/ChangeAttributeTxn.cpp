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

#include "ChangeAttributeTxn.h"
#include "nsIDOMElement.h"

ChangeAttributeTxn::ChangeAttributeTxn()
  : EditTxn()
{
}

ChangeAttributeTxn::~ChangeAttributeTxn()
{
}

NS_IMETHODIMP ChangeAttributeTxn::Init(nsIEditor      *aEditor,
                                       nsIDOMElement  *aElement,
                                       const nsAString& aAttribute,
                                       const nsAString& aValue,
                                       PRBool aRemoveAttribute)
{
  NS_ASSERTION(aEditor && aElement, "bad arg");
  if (!aEditor || !aElement) { return NS_ERROR_NULL_POINTER; }

  mEditor = aEditor;
  mElement = do_QueryInterface(aElement);
  mAttribute = aAttribute;
  mValue = aValue;
  mRemoveAttribute = aRemoveAttribute;
  mAttributeWasSet=PR_FALSE;
  mUndoValue.Truncate();
  return NS_OK;
}

NS_IMETHODIMP ChangeAttributeTxn::DoTransaction(void)
{
  NS_ASSERTION(mEditor && mElement, "bad state");
  if (!mEditor || !mElement) { return NS_ERROR_NOT_INITIALIZED; }

  // need to get the current value of the attribute and save it, and set mAttributeWasSet
  nsresult result = mEditor->GetAttributeValue(mElement, mAttribute, mUndoValue, &mAttributeWasSet);
  // XXX: hack until attribute-was-set code is implemented
  if (!mUndoValue.IsEmpty())
    mAttributeWasSet = PR_TRUE;
  // XXX: end hack
  
  // now set the attribute to the new value
  if (!mRemoveAttribute)
    result = mElement->SetAttribute(mAttribute, mValue);
  else
    result = mElement->RemoveAttribute(mAttribute);

  return result;
}

NS_IMETHODIMP ChangeAttributeTxn::UndoTransaction(void)
{
  NS_ASSERTION(mEditor && mElement, "bad state");
  if (!mEditor || !mElement) { return NS_ERROR_NOT_INITIALIZED; }

  nsresult result;
  if (mAttributeWasSet)
    result = mElement->SetAttribute(mAttribute, mUndoValue);
  else
    result = mElement->RemoveAttribute(mAttribute);

  return result;
}

NS_IMETHODIMP ChangeAttributeTxn::RedoTransaction(void)
{
  NS_ASSERTION(mEditor && mElement, "bad state");
  if (!mEditor || !mElement) { return NS_ERROR_NOT_INITIALIZED; }

  nsresult result;
  if (!mRemoveAttribute)
    result = mElement->SetAttribute(mAttribute, mValue);
  else
    result = mElement->RemoveAttribute(mAttribute);

  return result;
}

NS_IMETHODIMP ChangeAttributeTxn::Merge(nsITransaction *aTransaction, PRBool *aDidMerge)
{
  if (aDidMerge)
    *aDidMerge=PR_FALSE;
  return NS_OK;
}

NS_IMETHODIMP ChangeAttributeTxn::GetTxnDescription(nsAString& aString)
{
  aString.AssignLiteral("ChangeAttributeTxn: [mRemoveAttribute == ");

  if (!mRemoveAttribute)
    aString.AppendLiteral("false] ");
  else
    aString.AppendLiteral("true] ");
  aString += mAttribute;
  return NS_OK;
}
