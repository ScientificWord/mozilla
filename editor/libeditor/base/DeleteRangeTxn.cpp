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
 *   Pierre Phaneuf <pp@ludusdesign.com>
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

#include "DeleteRangeTxn.h"
#include "ReplaceElementTxn.h"
#include "nsIDOMRange.h"
#include "nsIDOMCharacterData.h"
#include "nsIDOMNodeList.h"
#include "nsISelection.h"
#include "DeleteTextTxn.h"
#include "DeleteElementTxn.h"
#include "TransactionFactory.h"
#include "nsIContentIterator.h"
#include "nsIContent.h"
#include "nsComponentManagerUtils.h"
#include "msiEditor.h"
#include "msiIMathMLCaret.h"
#include "msiIMathMLEditor.h"
#include "DeleteMathmlRangeTxn.h"
#include "nsIDOMDocument.h"
#include "nsIDOMDocumentRange.h"
#ifdef NS_DEBUG
static PRBool gNoisy = PR_FALSE;
#endif

// note that aEditor is not refcounted
DeleteRangeTxn::DeleteRangeTxn()
: EditAggregateTxn()
,mRange()
,mEditor(nsnull)
,mRangeUpdater(nsnull)
{
}

NS_IMETHODIMP DeleteRangeTxn::Init(nsIEditor *aEditor, 
                                   nsIDOMRange *aRange,
                                   nsRangeUpdater *aRangeUpdater)
{
  NS_ASSERTION(aEditor && aRange, "bad state");
  if (!aEditor || !aRange) 
    return NS_ERROR_NOT_INITIALIZED;

  mEditor = aEditor;
  mRange  = do_QueryInterface(aRange);
  mRangeUpdater = aRangeUpdater;
  nsresult result(NS_OK);

#ifdef NS_DEBUG
  {
    nsCOMPtr<nsIDOMNode> startParent, endParent, commonParent;
    PRInt32 startOffset(msiIMathMLEditingBC::INVALID), endOffset(msiIMathMLEditingBC::INVALID);
    result = aRange->GetStartContainer(getter_AddRefs(startParent));
    NS_ASSERTION((NS_SUCCEEDED(result)), "GetStartParent failed.");
    result = aRange->GetEndContainer(getter_AddRefs(endParent));
    NS_ASSERTION((NS_SUCCEEDED(result)), "GetEndParent failed.");
    result = aRange->GetStartOffset(&startOffset);
    NS_ASSERTION((NS_SUCCEEDED(result)), "GetStartOffset failed.");
    result = aRange->GetEndOffset(&endOffset);
    NS_ASSERTION((NS_SUCCEEDED(result)), "GetEndOffset failed.");
    result = aRange->GetCommonAncestorContainer(getter_AddRefs(commonParent));
    NS_ASSERTION((NS_SUCCEEDED(result)), "GetCommonParent failed.");
    PRUint32 count;
    nsCOMPtr<nsIDOMCharacterData> textNode = do_QueryInterface(startParent);
    if (textNode)
      textNode->GetLength(&count);
    else
    {
      nsCOMPtr<nsIDOMNodeList> children;
      result = startParent->GetChildNodes(getter_AddRefs(children));
      NS_ASSERTION(((NS_SUCCEEDED(result)) && children), "bad start child list");
      children->GetLength(&count);
    }
    NS_ASSERTION(startOffset<=(PRInt32)count, "bad start offset");

    textNode = do_QueryInterface(endParent);
    if (textNode)
      textNode->GetLength(&count);
    else
    {
      nsCOMPtr<nsIDOMNodeList> children;
      result = endParent->GetChildNodes(getter_AddRefs(children));
      NS_ASSERTION(((NS_SUCCEEDED(result)) && children), "bad end child list");
      children->GetLength(&count);
    }
    NS_ASSERTION(endOffset<=(PRInt32)count, "bad end offset");

#ifdef NS_DEBUG
    if (gNoisy)
    {
      printf ("DeleteRange: %d of %p to %d of %p\n", 
               startOffset, (void *)startParent, endOffset, (void *)endParent);
    }         
#endif
  }
#endif
  return result;

}

DeleteRangeTxn::~DeleteRangeTxn()
{
}

NS_IMETHODIMP DeleteRangeTxn::DoTransaction(void)
{
#ifdef NS_DEBUG
  if (gNoisy) { printf("Do Delete Range\n"); }
#endif
  if (!mRange || !mEditor) 
    return NS_ERROR_NOT_INITIALIZED;

  nsresult result(NS_OK);
  nsCOMPtr<nsIDOMNode> startParent, endParent, commonParent;
  PRInt32 startOffset(msiIMathMLEditingBC::INVALID), endOffset(msiIMathMLEditingBC::INVALID);
  
  result = mRange->GetStartContainer(getter_AddRefs(startParent));
  NS_ASSERTION((NS_SUCCEEDED(result)), "GetStartParent failed.");
  result = mRange->GetEndContainer(getter_AddRefs(endParent));
  NS_ASSERTION((NS_SUCCEEDED(result)), "GetEndParent failed.");
  result = mRange->GetStartOffset(&startOffset);
  NS_ASSERTION((NS_SUCCEEDED(result)), "GetStartOffset failed.");
  result = mRange->GetEndOffset(&endOffset);
  NS_ASSERTION((NS_SUCCEEDED(result)), "GetEndOffset failed.");
  result = mRange->GetCommonAncestorContainer(getter_AddRefs(commonParent));
  NS_ASSERTION((NS_SUCCEEDED(result)), "GetCommonParent failed.");

  if (!startParent || !endParent || !commonParent ||
      !IS_VALID_NODE_OFFSET(startOffset) || !IS_VALID_NODE_OFFSET(endOffset)) 
    return NS_ERROR_FAILURE;


  nsCOMPtr<msiIMathMLCaret> mathCaret;
  nsCOMPtr<msiIMathMLEditor> msiEditor(do_QueryInterface(mEditor));
  nsresult res(NS_ERROR_FAILURE);
  if (msiEditor)
    res = msiEditor->GetMathMLCaretInterface(commonParent, msiIMathMLEditingBC::INVALID, getter_AddRefs(mathCaret));
  
  if (NS_SUCCEEDED(res) && mathCaret) // both start and end in the same math run
  { 
    DeleteMathmlRangeTxn *txn;
    res = TransactionFactory::GetNewTransaction(DeleteMathmlRangeTxn::GetCID(), (EditTxn **)&txn);
    txn->Init(mEditor, mathCaret, startParent, startOffset, endParent, endOffset);
    AppendChild(txn);
    NS_RELEASE(txn);
  }
  else
  {
    res = NS_OK;
    //check to see if start and/or end is in math. If so, create deletemathmlrangetxn for them and
    // update mRange etc.
    nsCOMPtr<nsIDOMNode> startMath, endMath, newStart, newEnd;
    PRUint32 newStartOffset(msiIMathMLEditingBC::INVALID), newEndOffset(msiIMathMLEditingBC::INVALID);
    nsCOMPtr<msiIMathMLCaret> startCaret, endCaret;
    nsCOMPtr<nsIDOMRange> newRange;
    
    GetMathParent(startParent, startMath);
    if (startMath && msiEditor)
    {
      msiEditor->GetMathMLCaretInterface(startMath, msiIMathMLEditingBC::INVALID, getter_AddRefs(startCaret));
      startMath->GetParentNode(getter_AddRefs(newStart));
      GetIndexOfChildInParent(startMath, newStartOffset);
      newStartOffset += 1;
    }
    GetMathParent(endParent, endMath);
    if (endMath && msiEditor)
    {
      msiEditor->GetMathMLCaretInterface(endMath, msiIMathMLEditingBC::INVALID, getter_AddRefs(endCaret));
      endMath->GetParentNode(getter_AddRefs(newEnd));
      GetIndexOfChildInParent(endMath, newEndOffset);
    }
    if (startMath || endMath)
    {
      if (startMath && (!startCaret || !newStart || newStartOffset == msiIMathMLEditingBC::INVALID))
        res = NS_ERROR_FAILURE;
      if (endMath && (!endCaret || !newEnd || newEndOffset == msiIMathMLEditingBC::INVALID))
        res = NS_ERROR_FAILURE;
      if (NS_SUCCEEDED(res))
      {
        nsCOMPtr<nsIDOMDocument> doc;
        mEditor->GetDocument(getter_AddRefs(doc));
        if (doc)
        {
          nsCOMPtr<nsIDOMDocumentRange> docRange(do_QueryInterface(doc));
          if (docRange)
            docRange->CreateRange(getter_AddRefs(newRange));
        }
      }
      if (NS_FAILED(res) || !newRange)
        res = NS_ERROR_FAILURE;
      if (NS_SUCCEEDED(res))  
      {
        if (startCaret)  
        {
          DeleteMathmlRangeTxn *txn;
          nsCOMPtr<nsIDOMNode> dummy;
          res = TransactionFactory::GetNewTransaction(DeleteMathmlRangeTxn::GetCID(), (EditTxn **)&txn);
          txn->Init(mEditor, startCaret, startParent, startOffset, dummy, msiIMathMLEditingBC::INVALID);
          AppendChild(txn);
          NS_RELEASE(txn);
        }
        if (endCaret)  
        {
          DeleteMathmlRangeTxn *txn;
          nsCOMPtr<nsIDOMNode> dummy;
          res = TransactionFactory::GetNewTransaction(DeleteMathmlRangeTxn::GetCID(), (EditTxn **)&txn);
          txn->Init(mEditor, endCaret, dummy, msiIMathMLEditingBC::INVALID, endParent, endOffset);
          AppendChild(txn);
          NS_RELEASE(txn);
        }
        
        if (newStart)
        {
          newRange->SetStart(newStart, newStartOffset);
          startParent = newStart;
          startOffset = newStartOffset;
        }
        else
          newRange->SetStart(startParent, startOffset);
        if (newEnd)
        {
          newRange->SetEnd(newEnd, newEndOffset);
          endParent = newEnd;
          endOffset = newEndOffset;
        }
        else
          newRange->SetEnd(endParent, endOffset);
        mRange = newRange;  
      }
    }
    // build the child transactions
    if (startParent==endParent)
    { // the selection begins and ends in the same node
      res = CreateTxnsToDeleteBetween(startParent, startOffset, endOffset);
    }
    else
    { // the selection ends in a different node from where it started
      // delete the relevant content in the start node
      res = CreateTxnsToDeleteContent(startParent, startOffset, nsIEditor::eNext);
      if (NS_SUCCEEDED(res))
      {
        // delete the intervening nodes
        res = CreateTxnsToDeleteNodesBetween();
        if (NS_SUCCEEDED(res))
        {
          // delete the relevant content in the end node
          res = CreateTxnsToDeleteContent(endParent, endOffset, nsIEditor::ePrevious);
        }
      }
    }
  }
  // if we've successfully built this aggregate transaction, then do it.
  if (NS_SUCCEEDED(res)) {
    res = EditAggregateTxn::DoTransaction();
  }

  if (NS_FAILED(res)) return res;
  
  // only set selection to deletion point if editor gives permission
  PRBool bAdjustSelection;
  mEditor->ShouldTxnSetSelection(&bAdjustSelection);
  if (bAdjustSelection)
  {
    nsCOMPtr<nsISelection> selection;
    res = mEditor->GetSelection(getter_AddRefs(selection));
    if (NS_FAILED(res)) return res;
    if (!selection) return NS_ERROR_NULL_POINTER;
    res = selection->Collapse(startParent, startOffset);
  }
  else
  {
    // do nothing - dom range gravity will adjust selection
  }

  return res;
}

NS_IMETHODIMP DeleteRangeTxn::UndoTransaction(void)
{
#ifdef NS_DEBUG
  if (gNoisy) { printf("Undo Delete Range\n"); }
#endif

  if (!mRange || !mEditor) 
    return NS_ERROR_NOT_INITIALIZED;

  return EditAggregateTxn::UndoTransaction();
}

NS_IMETHODIMP DeleteRangeTxn::RedoTransaction(void)
{
#ifdef NS_DEBUG
  if (gNoisy) { printf("Redo Delete Range\n"); }
#endif

  if (!mRange || !mEditor) 
    return NS_ERROR_NOT_INITIALIZED;

  return EditAggregateTxn::RedoTransaction();
}

NS_IMETHODIMP DeleteRangeTxn::Merge(nsITransaction *aTransaction, PRBool *aDidMerge)
{
  if (aDidMerge)
    *aDidMerge = PR_FALSE;
  return NS_OK;
}

NS_IMETHODIMP DeleteRangeTxn::GetTxnDescription(nsAString& aString)
{
  aString.AssignLiteral("DeleteRangeTxn");
  return NS_OK;
}

NS_IMETHODIMP 
DeleteRangeTxn::CreateTxnsToDeleteBetween(nsIDOMNode *aStartParent, 
                                          PRUint32    aStartOffset, 
                                          PRUint32    aEndOffset)
{
  nsresult result;
  // see what kind of node we have
  nsCOMPtr<nsIDOMCharacterData> textNode = do_QueryInterface(aStartParent);
  if (textNode)
  { // if the node is a text node, then delete text content
    DeleteTextTxn *txn;
    result = TransactionFactory::GetNewTransaction(DeleteTextTxn::GetCID(), (EditTxn **)&txn);
    if (NS_FAILED(result)) return result;
    if (!txn) return NS_ERROR_NULL_POINTER;

    PRInt32 numToDel;
    if (aStartOffset==aEndOffset)
      numToDel = 1;
    else
      numToDel = aEndOffset-aStartOffset;
    txn->Init(mEditor, textNode, aStartOffset, numToDel, mRangeUpdater);
    AppendChild(txn);
    NS_RELEASE(txn);
  }
  else
  {
    nsCOMPtr<nsIDOMNodeList> children;
    result = aStartParent->GetChildNodes(getter_AddRefs(children));
    if (NS_FAILED(result)) return result;
    if (!children) return NS_ERROR_NULL_POINTER;

#ifdef DEBUG
    PRUint32 childCount;
    children->GetLength(&childCount);
    NS_ASSERTION(aEndOffset<=childCount, "bad aEndOffset");
#endif
    PRUint32 i;
    for (i=aStartOffset; i<aEndOffset; i++)
    {
      nsCOMPtr<nsIDOMNode> child;
      result = children->Item(i, getter_AddRefs(child));
      if (NS_FAILED(result)) return result;
      if (!child) return NS_ERROR_NULL_POINTER;

      DeleteElementTxn *txn;
      result = TransactionFactory::GetNewTransaction(DeleteElementTxn::GetCID(), (EditTxn **)&txn);
      if (NS_FAILED(result)) return result;
      if (!txn) return NS_ERROR_NULL_POINTER;

      txn->Init(child, mRangeUpdater);
      AppendChild(txn);
      NS_RELEASE(txn);
    }
  }
  return result;
}

NS_IMETHODIMP DeleteRangeTxn::CreateTxnsToDeleteContent(nsIDOMNode *aParent, 
                                                        PRUint32    aOffset, 
                                                        nsIEditor::EDirection aAction)
{
  nsresult result = NS_OK;
  // see what kind of node we have
  nsCOMPtr<nsIDOMCharacterData> textNode = do_QueryInterface(aParent);
  if (textNode)
  { // if the node is a text node, then delete text content
    PRUint32 start, numToDelete;
    if (nsIEditor::eNext == aAction)
    {
      start=aOffset;
      textNode->GetLength(&numToDelete);
      numToDelete -= aOffset;
    }
    else
    {
      start=0;
      numToDelete=aOffset;
    }
    
    if (numToDelete)
    {
      DeleteTextTxn *txn;
      result = TransactionFactory::GetNewTransaction(DeleteTextTxn::GetCID(), (EditTxn **)&txn);
      if (NS_FAILED(result)) return result;
      if (!txn) return NS_ERROR_NULL_POINTER;

      txn->Init(mEditor, textNode, start, numToDelete, mRangeUpdater);
      AppendChild(txn);
      NS_RELEASE(txn);
    }
  }

  return result;
}

NS_IMETHODIMP DeleteRangeTxn::CreateTxnsToDeleteNodesBetween()
{
  nsCOMPtr<nsIContentIterator> iter = do_CreateInstance("@mozilla.org/content/subtree-content-iterator;1");
  if (!iter) return NS_ERROR_NULL_POINTER;

  nsresult result = iter->Init(mRange);
  if (NS_FAILED(result)) return result;

  while (!iter->IsDone())
  {
    nsCOMPtr<nsIDOMNode> node = do_QueryInterface(iter->GetCurrentNode());
    if (!node)
      return NS_ERROR_NULL_POINTER;

    DeleteElementTxn *txn;
    result = TransactionFactory::GetNewTransaction(DeleteElementTxn::GetCID(), (EditTxn **)&txn);
    if (NS_FAILED(result)) return result;
    if (!txn) return NS_ERROR_NULL_POINTER;

    txn->Init(node, mRangeUpdater);
    AppendChild(txn);
    NS_RELEASE(txn);
    iter->Next();
  }
  return result;
}


nsresult DeleteRangeTxn::GetIndexOfChildInParent(nsIDOMNode * child, PRUint32 &index)
{
  nsresult res(NS_ERROR_FAILURE);
  if (child)
  {
    nsCOMPtr <nsIDOMNode> parent;
    child->GetParentNode(getter_AddRefs(parent));
    if (parent)
    {
      nsCOMPtr<nsIDOMNodeList> children;
      parent->GetChildNodes(getter_AddRefs(children));
      if (children) 
      {
        PRUint32 i, count;
        children->GetLength(&count);
        for (i=0; i < count; i++) 
        {
          nsCOMPtr<nsIDOMNode> currChild;
          children->Item(i, getter_AddRefs(currChild));
          if (currChild == child)
          {
            index = i;
            res = NS_OK;
            break;
          }
        }
      }  
    }
  }
  return res;
} 


nsresult DeleteRangeTxn::GetMathParent(nsIDOMNode * node,
                                 nsCOMPtr<nsIDOMNode> & mathParent)
{
  nsresult res(NS_ERROR_FAILURE);
  mathParent = nsnull;
  nsCOMPtr<nsIDOMNode> p = node;
  if (!p)
   res = NS_ERROR_NULL_POINTER;
  while (p)
  {
    nsAutoString localName, nsURI;
    p->GetLocalName(localName);
    p->GetNamespaceURI(nsURI);
    PRInt32 nsID(kNameSpaceID_Unknown);
    //msiNameSpaceUtils::GetNameSpaceID(nsURI, nsID);
    
    if (localName.Equals(NS_LITERAL_STRING("math")))
    //if (msiString_math.Equals(localName) && nsID == kNameSpaceID_MathML)
    {
      mathParent = p;
      p = nsnull;
      res = NS_OK;
    }
    else
    {
      nsCOMPtr<nsIDOMNode> temp = p;
      temp->GetParentNode(getter_AddRefs(p));
    }
  }
  return res;
}   
