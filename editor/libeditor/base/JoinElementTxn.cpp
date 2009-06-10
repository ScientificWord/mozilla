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

#include "JoinElementTxn.h"
#include "nsEditor.h"
#include "nsIDOMNodeList.h"
#include "nsIDOMCharacterData.h"

#ifdef NS_DEBUG
static PRBool gNoisy = PR_FALSE;
#endif

JoinElementTxn::JoinElementTxn()
  : EditTxn()
{
}

NS_IMETHODIMP JoinElementTxn::Init(nsEditor   *aEditor,
                                   nsIDOMNode *aLeftNode,
                                   nsIDOMNode *aRightNode)
{
  NS_PRECONDITION((aEditor && aLeftNode && aRightNode), "null arg");
  if (!aEditor || !aLeftNode || !aRightNode) { return NS_ERROR_NULL_POINTER; }
  mEditor = aEditor;
  mLeftNode = do_QueryInterface(aLeftNode);
  nsCOMPtr<nsIDOMNode>leftParent;
  nsresult result = mLeftNode->GetParentNode(getter_AddRefs(leftParent));
  if (NS_FAILED(result)) return result;
  if (!mEditor->IsModifiableNode(leftParent)) {
    return NS_ERROR_FAILURE;
  }
  mRightNode = do_QueryInterface(aRightNode);
  mOffset=0;
  return NS_OK;
}

// After DoTransaction() and RedoTransaction(), the left node is removed from the content tree and right node remains.
NS_IMETHODIMP JoinElementTxn::DoTransaction(void)
{
#ifdef NS_DEBUG
  if (gNoisy) { printf("%p Do Join of %p and %p\n", this, mLeftNode.get(), mRightNode.get()); }
#endif

  NS_PRECONDITION((mEditor && mLeftNode && mRightNode), "null arg");
  if (!mEditor || !mLeftNode || !mRightNode) { return NS_ERROR_NOT_INITIALIZED; }

  // get the parent node
  nsCOMPtr<nsIDOMNode>leftParent;
  nsresult result = mLeftNode->GetParentNode(getter_AddRefs(leftParent));
  if (NS_FAILED(result)) return result;
  if (!leftParent) return NS_ERROR_NULL_POINTER;

  // verify that mLeftNode and mRightNode have the same parent
  nsCOMPtr<nsIDOMNode>rightParent;
  result = mRightNode->GetParentNode(getter_AddRefs(rightParent));
  if (NS_FAILED(result)) return result;
  if (!rightParent) return NS_ERROR_NULL_POINTER;

  if (leftParent==rightParent)
  {
    mParent= do_QueryInterface(leftParent); // set this instance mParent. 
                                            // Other methods will see a non-null mParent and know all is well
    nsCOMPtr<nsIDOMCharacterData> leftNodeAsText = do_QueryInterface(mLeftNode);
    if (leftNodeAsText) 
    {
      leftNodeAsText->GetLength(&mOffset);
    }
    else 
    {
      nsCOMPtr<nsIDOMNodeList> childNodes;
      result = mLeftNode->GetChildNodes(getter_AddRefs(childNodes));
      if (NS_FAILED(result)) return result;
      if (childNodes) 
      {
        childNodes->GetLength(&mOffset);
      }
    }
    result = mEditor->JoinNodesImpl(mLeftNode, mRightNode, mParent, PR_TRUE);
#ifdef NS_DEBUG
    if (NS_SUCCEEDED(result))
    {
      if (gNoisy) { printf("  left node = %p removed\n", mLeftNode.get()); }
    }
#endif
  }
  else 
  {
    NS_ASSERTION(PR_FALSE, "2 nodes do not have same parent");
    return NS_ERROR_INVALID_ARG;
  }
  return result;
}

//XXX: what if instead of split, we just deleted the unneeded children of mRight
//     and re-inserted mLeft?
NS_IMETHODIMP JoinElementTxn::UndoTransaction(void)
{
#ifdef NS_DEBUG
  if (gNoisy) { printf("%p Undo Join, leftt node = %p\n", this, mLeftNode.get()); }
#endif

  NS_ASSERTION(mRightNode && mLeftNode && mParent, "bad state");
  if (!mRightNode || !mLeftNode || !mParent) { return NS_ERROR_NOT_INITIALIZED; }
  nsresult result;
  nsCOMPtr<nsIDOMNode>resultNode;
  // first, massage the existing node so it is in its post-split state
  nsCOMPtr<nsIDOMCharacterData>leftNodeAsText = do_QueryInterface(mLeftNode);
  if (leftNodeAsText)
  {
    result = leftNodeAsText->DeleteData(mOffset, -1);
  }
  else
  {
    nsCOMPtr<nsIDOMNode>child;
    nsCOMPtr<nsIDOMNode>destChild;
    result = mRightNode->GetFirstChild(getter_AddRefs(destChild));
    result = mLeftNode->GetFirstChild(getter_AddRefs(child));
    nsCOMPtr<nsIDOMNode>nextSibling;
    PRUint32 i;
    for (i=0; i<mOffset; i++)
    {
      child->GetNextSibling(getter_AddRefs(nextSibling));
      child = do_QueryInterface(nextSibling);
    };

    for (i=mOffset; child; i++)
    {
      if (NS_FAILED(result)) {return result;}
      if (!child) {return NS_ERROR_NULL_POINTER;}
      child->GetNextSibling(getter_AddRefs(nextSibling));
      result = mRightNode->InsertBefore(child, destChild, getter_AddRefs(destChild));
      child = do_QueryInterface(nextSibling);
    }
  }
  // second, re-insert the right node into the tree 
  nsCOMPtr<nsIDOMNode> insertBeforeMe;
  result = mLeftNode->GetNextSibling(getter_AddRefs(insertBeforeMe));
  result = mParent->InsertBefore(mRightNode, insertBeforeMe, getter_AddRefs(resultNode));
  return result;

}

NS_IMETHODIMP JoinElementTxn::GetTxnDescription(nsAString& aString)
{
  aString.AssignLiteral("JoinElementTxn");
  return NS_OK;
}
