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
 * Portions created by the Initial Developer are Copyright (C) 1998-1999
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

#include "nsReadableUtils.h"
#include "nsCRT.h"

#include "DeleteElementTxn.h"
#include "nsSelectionState.h"
#ifdef NS_DEBUG
#include "nsIDOMElement.h"
#endif

#ifdef NS_DEBUG
static PRBool gNoisy = PR_FALSE;
#endif


DeleteElementTxn::DeleteElementTxn()
: EditTxn()
,mElement()
,mParent()
,mRefNode()
,mRangeUpdater(nsnull)
{
}

NS_IMETHODIMP DeleteElementTxn::Init(nsIEditor *aEditor,
                                     nsIDOMNode *aElement,
                                     nsRangeUpdater *aRangeUpdater)
{
  if (!aEditor || !aElement) return NS_ERROR_NULL_POINTER;
  mEditor = aEditor;
  mElement = do_QueryInterface(aElement);
  nsresult result = mElement->GetParentNode(getter_AddRefs(mParent));
  if (NS_FAILED(result)) { return result; }

  // do nothing if the parent is read-only
  if (mParent && !mEditor->IsModifiableNode(mParent)) {
    return NS_ERROR_FAILURE;
  }

  mRangeUpdater = aRangeUpdater;
  return NS_OK;
}


NS_IMETHODIMP DeleteElementTxn::DoTransaction(void)
{
#ifdef NS_DEBUG
  if (gNoisy) { printf("%p Do Delete Element element = %p\n", this, mElement.get()); }
#endif

  if (!mElement) return NS_ERROR_NOT_INITIALIZED;

  if (!mParent) { return NS_OK; }  // this is a no-op, there's no parent to delete mElement from

#ifdef NS_DEBUG
  // begin debug output
  nsCOMPtr<nsIDOMElement> element = do_QueryInterface(mElement);
  nsAutoString elementTag(NS_LITERAL_STRING("text node"));
  if (element)
    element->GetTagName(elementTag);
  nsCOMPtr<nsIDOMElement> parentElement = do_QueryInterface(mParent);
  nsAutoString parentElementTag(NS_LITERAL_STRING("text node"));
  if (parentElement)
    parentElement->GetTagName(parentElementTag);
  char *c, *p;
  c = ToNewCString(elementTag);
  p = ToNewCString(parentElementTag);
  if (c&&p)
  {
    if (gNoisy)
      printf("  DeleteElementTxn:  deleting child %s from parent %s\n", c, p); 

    NS_Free(c);
    NS_Free(p);
  }
  // end debug output
#endif

  // remember which child mElement was (by remembering which child was next)
  nsresult result = mElement->GetNextSibling(getter_AddRefs(mRefNode));  // can return null mRefNode

  // give range updater a chance.  SelAdjDeleteNode() needs to be called *before*
  // we do the action, unlike some of the other nsRangeStore update methods.
  if (mRangeUpdater) 
    mRangeUpdater->SelAdjDeleteNode(mElement, PR_TRUE);

  nsCOMPtr<nsIDOMNode> resultNode;
  return mParent->RemoveChild(mElement, getter_AddRefs(resultNode));
}

NS_IMETHODIMP DeleteElementTxn::UndoTransaction(void)
{
#ifdef NS_DEBUG
  if (gNoisy) { printf("%p Undo Delete Element element = %p, parent = %p\n", this, mElement.get(), mParent.get()); }
#endif

  if (!mParent) { return NS_OK; } // this is a legal state, the txn is a no-op
  if (!mElement) { return NS_ERROR_NULL_POINTER; }

#ifdef Never
  // begin debug output
  nsCOMPtr<nsIDOMElement> element = do_QueryInterface(mElement);
  nsAutoString elementTag(NS_LITERAL_STRING("text node"));
  if (element)
    element->GetTagName(elementTag);
  nsCOMPtr<nsIDOMElement> parentElement = do_QueryInterface(mParent);
  nsAutoString parentElementTag(NS_LITERAL_STRING("text node"));
  if (parentElement)
    parentElement->GetTagName(parentElementTag);
  char *c, *p;
  c = ToNewCString(elementTag);
  p = ToNewCString(parentElementTag);
  if (c&&p)
  {
    if (gNoisy)
      printf("  DeleteElementTxn:  inserting child %s back into parent %s\n", c, p); 

    NS_Free(c);
    NS_Free(p);
  }
  // end debug output
#endif
  nsresult res;
  nsCOMPtr<nsIDOMNode> resultNode;
  res = mParent->InsertBefore(mElement, mRefNode, getter_AddRefs(resultNode));
//  mEditor->markNodeDirty(mElement);
  return res;
}

NS_IMETHODIMP DeleteElementTxn::RedoTransaction(void)
{
#ifdef NS_DEBUG
  if (gNoisy) { printf("%p Redo Delete Element element = %p, parent = %p\n", this, mElement.get(), mParent.get()); }
#endif

  if (!mParent) { return NS_OK; } // this is a legal state, the txn is a no-op
  if (!mElement) { return NS_ERROR_NULL_POINTER; }

  if (mRangeUpdater) 
    mRangeUpdater->SelAdjDeleteNode(mElement, PR_TRUE);

  nsCOMPtr<nsIDOMNode> resultNode;
  return mParent->RemoveChild(mElement, getter_AddRefs(resultNode));
}

NS_IMETHODIMP DeleteElementTxn::GetTxnDescription(nsAString& aString)
{
  nsAutoString elementName;
  nsAutoString parentName;
  nsAutoString refNodeName;

  if (mElement) {
    mElement->GetNodeName(elementName);
    aString.AssignLiteral("DeleteElementTxn: element ");
    aString.Append(elementName);
  }
  if (mParent) {
    mParent->GetNodeName(parentName);
    aString.AppendLiteral(", parent ");
    aString.Append(parentName);
  }
  if (mRefNode) {
    mRefNode->GetNodeName(refNodeName);
    aString.AppendLiteral(", refNode ");
    aString.Append(refNodeName);
  }

  return NS_OK;
}
