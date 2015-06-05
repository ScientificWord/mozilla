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

#include "msiDeleteRangeTxn.h"
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

#include "msiSelectionManager.h"

#ifdef NS_DEBUG
static PRBool gNoisy = PR_FALSE;
#endif

// note that editor, rangeUpdater and msiSelectionManager are not refcounted
msiDeleteRangeTxn::msiDeleteRangeTxn()
: EditAggregateTxn(), m_msiSelMan(nsnull), m_rangeIndex(-1),
m_editor(nsnull), m_rangeUpdater(nsnull)
{
}

nsresult msiDeleteRangeTxn::Init(nsIEditor *editor,
                              msiSelectionManager * msiSelMan,
                              PRInt32 rangeIndex, 
                              nsRangeUpdater *rangeUpdater)
{
  NS_ASSERTION(editor && msiSelMan && (rangeIndex >= 0), "bad state");
  if (!editor || !msiSelMan || rangeIndex < 0) 
    return NS_ERROR_NOT_INITIALIZED;

  m_editor = editor;
  m_msiSelMan = msiSelMan;
  m_rangeIndex = rangeIndex;
  m_rangeUpdater = rangeUpdater;
  nsresult result(NS_OK);

#ifdef NS_DEBUG
    nsCOMPtr<nsIDOMRange> range;
    result = msiSelMan->GetRange(rangeIndex, range);
    NS_ASSERTION((NS_SUCCEEDED(result)), "GetRange failed.");
    if (NS_FAILED(result))
      return result;
    nsCOMPtr<nsIDOMNode> startParent, endParent, commonParent;
    PRInt32 startOffset(msiIMathMLEditingBC::INVALID), endOffset(msiIMathMLEditingBC::INVALID);
    result = range->GetStartContainer(getter_AddRefs(startParent));
    NS_ASSERTION((NS_SUCCEEDED(result)), "GetStartParent failed.");
    result = range->GetEndContainer(getter_AddRefs(endParent));
    NS_ASSERTION((NS_SUCCEEDED(result)), "GetEndParent failed.");
    result = range->GetStartOffset(&startOffset);
    NS_ASSERTION((NS_SUCCEEDED(result)), "GetStartOffset failed.");
    result = range->GetEndOffset(&endOffset);
    NS_ASSERTION((NS_SUCCEEDED(result)), "GetEndOffset failed.");
    result = range->GetCommonAncestorContainer(getter_AddRefs(commonParent));
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
    if (gNoisy)
    {
      printf ("DeleteRange: %d of %p to %d of %p\n", 
               startOffset, (void *)startParent, endOffset, (void *)endParent);
   }         
#endif
  return result;

}

msiDeleteRangeTxn::~msiDeleteRangeTxn()
{
}

NS_IMETHODIMP msiDeleteRangeTxn::DoTransaction(void)
{
#ifdef NS_DEBUG
  if (gNoisy) { printf("Do  MSI Delete Range\n"); }
#endif
  nsresult res(NS_OK);
  if (!m_editor || !m_msiSelMan || m_rangeIndex < 0) 
    res = NS_ERROR_NOT_INITIALIZED;
  nsCOMPtr<nsIDOMRange> range;
  res = m_msiSelMan->GetRange(m_rangeIndex, range);
  NS_ASSERTION((NS_SUCCEEDED(res)), "GetRange failed.");
  if (NS_FAILED(res))
    return res;
  PRBool okToCollapseAfterDelete(PR_TRUE);
  nsCOMPtr<nsIDOMNode> startParent, endParent, commonParent;
  PRInt32 startOffset(msiIMathMLEditingBC::INVALID), endOffset(msiIMathMLEditingBC::INVALID);
  
  res = range->GetStartContainer(getter_AddRefs(startParent));
  NS_ASSERTION((NS_SUCCEEDED(res)), "GetStartParent failed.");
  res = range->GetEndContainer(getter_AddRefs(endParent));
  NS_ASSERTION((NS_SUCCEEDED(res)), "GetEndParent failed.");
  res = range->GetStartOffset(&startOffset);
  NS_ASSERTION((NS_SUCCEEDED(res)), "GetStartOffset failed.");
  res = range->GetEndOffset(&endOffset);
  NS_ASSERTION((NS_SUCCEEDED(res)), "GetEndOffset failed.");
  res = range->GetCommonAncestorContainer(getter_AddRefs(commonParent));
  NS_ASSERTION((NS_SUCCEEDED(res)), "GetCommonParent failed.");

  if (!startParent || !endParent || !commonParent ||
      !IS_VALID_NODE_OFFSET(startOffset) || !IS_VALID_NODE_OFFSET(endOffset)) 
    return NS_ERROR_FAILURE;
  nsCOMPtr<msiIMathMLCaret> mathCaret;
  nsCOMPtr<msiIMathMLEditor> msiEditor(do_QueryInterface(m_editor));
  if (msiEditor)
    res = msiEditor->GetMathMLCaretInterface(commonParent, msiIMathMLEditingBC::INVALID, getter_AddRefs(mathCaret));
  
  if (NS_SUCCEEDED(res) && mathCaret) // both start and end in the same math run
  { 
    okToCollapseAfterDelete = PR_FALSE;  //TODO -- temp
  
    DeleteMathmlRangeTxn *txn;
    res = TransactionFactory::GetNewTransaction(DeleteMathmlRangeTxn::GetCID(), (EditTxn **)&txn);
    txn->Init(m_editor, mathCaret, startParent, startOffset, endParent, endOffset);
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
        m_editor->GetDocument(getter_AddRefs(doc));
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
          okToCollapseAfterDelete = PR_FALSE;  //TODO -- temp
          DeleteMathmlRangeTxn *txn;
          nsCOMPtr<nsIDOMNode> dummy;
          res = TransactionFactory::GetNewTransaction(DeleteMathmlRangeTxn::GetCID(), (EditTxn **)&txn);
          txn->Init(m_editor, startCaret, startParent, startOffset, dummy, msiIMathMLEditingBC::INVALID);
          AppendChild(txn);
          NS_RELEASE(txn);
        }
        if (endCaret)  
        {
          okToCollapseAfterDelete = PR_FALSE;  //TODO -- temp
          DeleteMathmlRangeTxn *txn;
          nsCOMPtr<nsIDOMNode> dummy;
          res = TransactionFactory::GetNewTransaction(DeleteMathmlRangeTxn::GetCID(), (EditTxn **)&txn);
          txn->Init(m_editor, endCaret, dummy, msiIMathMLEditingBC::INVALID, endParent, endOffset);
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
        range = newRange;  
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
        res = CreateTxnsToDeleteNodesBetween(range);
        if (NS_SUCCEEDED(res))
        {
          // delete the relevant content in the end node
          res = CreateTxnsToDeleteContent(endParent, endOffset, nsIEditor::ePrevious);
        }
      }
    }
  }
  // if we've successfully built this aggregate transaction, then do it.
  if (NS_SUCCEEDED(res)) 
  {
    res = EditAggregateTxn::DoTransaction();
  }
  if (NS_FAILED(res)) 
    return res;
  
  // only set selection to deletion point if editor gives permission
  PRBool bAdjustSelection;
  m_editor->ShouldTxnSetSelection(&bAdjustSelection);
  bAdjustSelection = PR_FALSE; // BBM: testing
  if (bAdjustSelection) // && okToCollapseAfterDelete) //TODO -- ok to collapse is temp
  {
    nsCOMPtr<nsISelection> selection;
    res = m_editor->GetSelection(getter_AddRefs(selection));
    if (NS_FAILED(res) || !selection) 
      return NS_ERROR_FAILURE;
    res = selection->Collapse(startParent, startOffset);
  }
  else
  {
    // do nothing - dom range gravity will adjust selection
  }
  return res;
}

NS_IMETHODIMP msiDeleteRangeTxn::UndoTransaction(void)
{
#ifdef NS_DEBUG
  if (gNoisy) { printf("Undo Delete Range\n"); }
#endif
  return EditAggregateTxn::UndoTransaction();
}

NS_IMETHODIMP msiDeleteRangeTxn::RedoTransaction(void)
{
#ifdef NS_DEBUG
  if (gNoisy) { printf("Redo Delete Range\n"); }
#endif

  return EditAggregateTxn::RedoTransaction();
}

NS_IMETHODIMP msiDeleteRangeTxn::Merge(nsITransaction *aTransaction, PRBool *aDidMerge)
{
  if (aDidMerge)
    *aDidMerge = PR_FALSE;
  return NS_OK;
}

NS_IMETHODIMP msiDeleteRangeTxn::GetTxnDescription(nsAString& aString)
{
  aString.AssignLiteral("msiDeleteRangeTxn");
  return NS_OK;
}

nsresult 
msiDeleteRangeTxn::CreateTxnsToDeleteBetween(nsIDOMNode *aStartParent, 
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
    txn->Init(m_editor, textNode, aStartOffset, numToDel, m_rangeUpdater);
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

      txn->Init(m_editor, child, m_rangeUpdater);
      AppendChild(txn);
      NS_RELEASE(txn);
    }
  }
  return result;
}

nsresult 
msiDeleteRangeTxn::CreateTxnsToDeleteContent(nsIDOMNode *aParent, 
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

      txn->Init(m_editor, textNode, start, numToDelete, m_rangeUpdater);
      AppendChild(txn);
      NS_RELEASE(txn);
    }
  }

  return result;
}

nsresult
msiDeleteRangeTxn::CreateTxnsToDeleteNodesBetween(nsCOMPtr <nsIDOMRange> & range)
{
  nsCOMPtr<nsIContentIterator> iter = do_CreateInstance("@mozilla.org/content/subtree-content-iterator;1");
  if (!iter) return NS_ERROR_NULL_POINTER;

  nsresult result = iter->Init(range);
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

    txn->Init(m_editor, node, m_rangeUpdater);
    AppendChild(txn);
    NS_RELEASE(txn);
    iter->Next();
  }
  return result;
}


nsresult 
msiDeleteRangeTxn::GetIndexOfChildInParent(nsIDOMNode * child, PRUint32 &index)
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


nsresult 
msiDeleteRangeTxn::GetMathParent(nsIDOMNode * node,
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
