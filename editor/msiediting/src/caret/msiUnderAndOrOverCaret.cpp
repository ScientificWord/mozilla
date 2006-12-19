// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.
#include "msiUnderAndOrOverCaret.h"
#include "msiUtils.h"
#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "nsIEditor.h"
#include "nsISelection.h"
#include "nsIFrame.h"
#include "nsIView.h"
#include "msiRequiredArgument.h"
#include "msiIMathMLEditor.h"
#include "nsITransaction.h"
#include "nsIMutableArray.h"
#include "nsComponentManagerUtils.h"
#include "nsIDOMNodeList.h"

msiUnderAndOrOverCaret::msiUnderAndOrOverCaret(nsIDOMNode* mathmlNode, PRUint32 offset, PRUint32 mathmlType)
:msiMCaretBase(mathmlNode, offset, mathmlType)
{
}

NS_IMETHODIMP
msiUnderAndOrOverCaret::PrepareForCaret(nsIEditor* editor)
{
  if (!editor || !m_mathmlNode)
    return NS_ERROR_FAILURE;
  nsresult res(NS_OK);
  nsCOMPtr<nsIDOMNode> child;
  res = m_mathmlNode->GetFirstChild(getter_AddRefs(child));
  PRUint32 index(1);
  while (NS_SUCCEEDED(res) && child)
  {
    if (msiRequiredArgument::NestRequiredArgumentInMrow(child))
    {
      nsCOMPtr<nsIDOMElement> mrow;
      res = msiUtils::CreateMRow(editor, (nsIDOMNode*)nsnull, mrow);
      if (NS_SUCCEEDED(res) && mrow)
      {
        nsCOMPtr<nsIDOMNode> kid, dontcare;
        nsCOMPtr<nsIDOMNode> mrowNode = do_QueryInterface(mrow);
        if (mrowNode)
          res = m_mathmlNode->ReplaceChild(mrowNode,  child, getter_AddRefs(kid));
        else 
          res = NS_ERROR_FAILURE;  
        if (NS_SUCCEEDED(res))
          res = mrowNode->AppendChild(kid, getter_AddRefs(dontcare));
      }  
    }
    if (NS_SUCCEEDED(res))
    {
      if (index < m_numKids)
      {
        index +=1;
        nsCOMPtr<nsIDOMNode> sibling;
        child->GetNextSibling(getter_AddRefs(sibling));
        if (NS_SUCCEEDED(res) && sibling)
          child = sibling;
        else
        {
          child = nsnull;  
          res = NS_ERROR_FAILURE;
        }
      }
      else
        child = nsnull;  
    }
  }  
  return res;
}

NS_IMETHODIMP
msiUnderAndOrOverCaret::Inquiry(nsIEditor* editor, PRUint32 inquiryID, PRBool *result)
{
  if (inquiryID == AT_RIGHT_EQUIV_TO_0 || inquiryID == AT_LEFT_EQUIV_TO_RIGHT_MOST) 
    *result = PR_FALSE;
  else if (inquiryID == CAN_SELECT_CHILD_LEAF) 
    *result = PR_TRUE;
  else
    *result = PR_FALSE;
  return NS_OK;
}

NS_IMETHODIMP
msiUnderAndOrOverCaret::AdjustNodeAndOffsetFromMouseEvent(nsIEditor *editor, nsIPresShell *presShell,
                                                       PRUint32 flags, 
                                                       nsIDOMMouseEvent *mouseEvent, 
                                                       nsIDOMNode **node, 
                                                       PRUint32 *offset)
{
  return msiMCaretBase::AdjustNodeAndOffsetFromMouseEvent(editor, presShell, flags, 
                                                          mouseEvent, node, offset);
}                                                       

       
NS_IMETHODIMP 
msiUnderAndOrOverCaret::GetSelectableMathFragment(nsIEditor  *editor, 
                                               nsIDOMNode *start,      PRUint32 startOffset, 
                                               nsIDOMNode *end,        PRUint32 endOffset, 
                                               nsIDOMNode **fragStart, PRUint32 *fragStartOffset,
                                               nsIDOMNode **fragEnd,   PRUint32 *fragEndOffset)
{
  //ljh -- code below assumes the over/under is the "common ancestor" of start and end.
  if (!editor || !m_mathmlNode)
    return NS_ERROR_FAILURE;
  if (!(start && startOffset <= LAST_VALID) || !(end && endOffset <= LAST_VALID))
    return NS_ERROR_FAILURE;
  if (!fragStart || !fragEnd || !fragStartOffset || !fragEndOffset)
    return NS_ERROR_FAILURE;
  *fragStart = nsnull;
  *fragEnd = nsnull;
  *fragStartOffset = INVALID;
  *fragEndOffset = INVALID;
  nsresult res(NS_OK);
  PRInt32 startEndCompare(0);
  msiUtils::ComparePoints(editor, start, startOffset, end, endOffset, startEndCompare);
  if (startEndCompare ==  1)
  {
    NS_ASSERTION(PR_FALSE, "start after end");
    res = NS_ERROR_FAILURE;
  }
  else if (startEndCompare == 0)
  { 
    // collapse the selection
    *fragStart = start;
    *fragEnd = start;
    NS_ADDREF(*fragStart);
    NS_ADDREF(*fragEnd);
    *fragStartOffset = startOffset;
    *fragEndOffset = startOffset;
    res = NS_OK;
  }
  else
  {
    PRBool toParent(PR_FALSE);
    PRInt32 startCompare1(0), endCompare1(0), startCompare2(0), endCompare2(0);
    res = msiUtils::ComparePoints(editor, start, startOffset, m_mathmlNode, 1, startCompare1);
    if (NS_SUCCEEDED(res))
      res = msiUtils::ComparePoints(editor, end, endOffset, m_mathmlNode, 1, endCompare1);
    if (NS_SUCCEEDED(res))
      res = msiUtils::ComparePoints(editor, start, startOffset, m_mathmlNode, 2, startCompare2);
    if (NS_SUCCEEDED(res))
      res = msiUtils::ComparePoints(editor, end, endOffset, m_mathmlNode, 2, endCompare2);
    if (NS_FAILED(res))
      return res;
    if (startCompare1 == -1 && endCompare1 <= 0) // select base
    {
      *fragStart = m_mathmlNode;
      *fragEnd = m_mathmlNode;
      NS_ADDREF(*fragStart);
      NS_ADDREF(*fragEnd);
      *fragStartOffset = 0;
      *fragEndOffset = 1;
      res = NS_OK;
    }
    else if (startCompare1 >= 0 && endCompare1 == 1)
    {
      if (m_numKids == 2) // select script1
      {
        *fragStart = m_mathmlNode;
        *fragEnd = m_mathmlNode;
        NS_ADDREF(*fragStart);
        NS_ADDREF(*fragEnd);
        *fragStartOffset = 1;
        *fragEndOffset = 2;
        res = NS_OK;
      }
      else
      {
        if (startCompare2 == -1 && endCompare2 <= 0) // select script1
        {
          *fragStart = m_mathmlNode;
          *fragEnd = m_mathmlNode;
          NS_ADDREF(*fragStart);
          NS_ADDREF(*fragEnd);
          *fragStartOffset = 1;
          *fragEndOffset = 2;
          res = NS_OK;
        }
        else if (startCompare2 >= 0 && endCompare2 == 1) // select script2
        {
          *fragStart = m_mathmlNode;
          *fragEnd = m_mathmlNode;
          NS_ADDREF(*fragStart);
          NS_ADDREF(*fragEnd);
          *fragStartOffset = 2;
          *fragEndOffset = 3;
          res = NS_OK;
        }
        else // (startCompare2 == -1 && endCompare2 == 1)
          toParent = PR_TRUE;
      }
    }
    else
      toParent = PR_TRUE;
    if (toParent)
    {  nsCOMPtr<msiIMathMLCaret> parent;
      res = msiUtils::SetupPassOffCaretToParent(editor, m_mathmlNode, PR_FALSE, parent);
      if (NS_SUCCEEDED(res) && parent)
      {
        nsCOMPtr<nsIDOMNode> parentNode;
        PRUint32 offset(INVALID);
        res = msiUtils::GetMathmlNodeFromCaretInterface(parent, parentNode);
        if (NS_SUCCEEDED(res))
          res = msiUtils::GetOffsetFromCaretInterface(parent, offset);
        if (NS_SUCCEEDED(res) && parentNode && offset <= LAST_VALID)
          res = parent->GetSelectableMathFragment(editor, parentNode, offset, parentNode, offset+1, 
                                                  fragStart, fragStartOffset, fragEnd, fragEndOffset);
      }
    }
  }  
  return res;
}

NS_IMETHODIMP
msiUnderAndOrOverCaret::AdjustSelectionPoint(nsIEditor *editor, PRBool leftSelPoint, 
                                             nsIDOMNode** selectionNode, PRUint32 * selectionOffset,
                                             msiIMathMLCaret** parentCaret)
{
  nsCOMPtr<msiIMathMLCaret> pCaret;
  PRBool incOffset(!leftSelPoint);
  if (!leftSelPoint && m_offset == 0)
     incOffset = PR_FALSE;
  else if (leftSelPoint && m_numKids == 1 && m_offset == 1)
    incOffset = PR_FALSE;
  nsresult res = msiUtils::SetupPassOffCaretToParent(editor, m_mathmlNode, incOffset, pCaret);
  if (NS_SUCCEEDED(res) && pCaret)  
  {
    nsCOMPtr<msiIMathMLEditingBC> parentBC(do_QueryInterface(pCaret));
    if (parentBC)
    {
      parentBC->GetMathmlNode(selectionNode);
      parentBC->GetOffset(selectionOffset);
      *parentCaret = pCaret;
      NS_ADDREF(*parentCaret);
      res = (*selectionNode && (*selectionOffset != INVALID)) ? NS_OK : NS_ERROR_FAILURE;
    }
    else 
      res = NS_ERROR_FAILURE;
    if (NS_SUCCEEDED(res))
    {
      *parentCaret = pCaret;
      NS_ADDREF(*parentCaret);
    }
  }
  return res;
}

                                

NS_IMETHODIMP
msiUnderAndOrOverCaret::Accept(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  if (!node || !offset || !m_mathmlNode || !editor)
    return NS_ERROR_FAILURE;
  nsresult res(NS_OK);
  nsCOMPtr<nsIDOMNode> child;
  nsCOMPtr<msiIMathMLCaret> mathmlEditing;
  if (flags & FROM_LEFT)
  {
    if (m_offset == 0)
    {
      res = msiUtils::GetChildNode(m_mathmlNode, 0, child);
      NS_ASSERTION(child, "Yuck - child is null");
      if (NS_SUCCEEDED(res) && child)
      {
        msiUtils::GetMathMLCaretInterface(editor, child, 0, mathmlEditing);
        NS_ASSERTION(mathmlEditing, "Yuck - mathml caret interface is null");
        if (mathmlEditing)
          res = mathmlEditing->Accept(editor, FROM_LEFT|FROM_PARENT, node, offset);
        else
          res = NS_ERROR_FAILURE;
      }
      else
        res = NS_ERROR_FAILURE;
    }
    else if (!(flags& FROM_CHILD))
    {
      nsCOMPtr<msiIMathMLCaret> parent;
      msiUtils::SetupPassOffCaretToParent(editor, m_mathmlNode, PR_TRUE, parent);
      if (parent)
        res = parent->Accept(editor, FROM_CHILD|FROM_LEFT, node, offset);
      if (*node == nsnull)
      {
        *node = m_mathmlNode;
        NS_ADDREF(*node);
        *offset = 0;
        res = NS_OK;
      }  
    }
    else
      *node = nsnull; // don't accept from child
  }        
  else if (flags & FROM_RIGHT)
  {
    if (m_offset == m_numKids)
    {
      msiUtils::GetChildNode(m_mathmlNode, 0, child);
      msiUtils::GetMathMLCaretInterface(editor, child, RIGHT_MOST, mathmlEditing);
      if (mathmlEditing)
        res = mathmlEditing->Accept(editor, FROM_PARENT|FROM_RIGHT, node, offset);
    }
    else if (!(flags& FROM_CHILD))
    { 
      nsCOMPtr<msiIMathMLCaret> parent;
      msiUtils::SetupPassOffCaretToParent(editor, m_mathmlNode, PR_FALSE, parent);
      if (parent)
        res = parent->Accept(editor, FROM_CHILD|FROM_RIGHT, node, offset);
      if (*node == nsnull)
      {
        *node = m_mathmlNode;
        NS_ADDREF(*node);
        *offset = 0;
        res = NS_OK;
      }  
    }
    else
      *node = nsnull; // don't accept from child
  }
  return res;
}

NS_IMETHODIMP 
msiUnderAndOrOverCaret::SplitAtDecendents(nsIEditor* editor, 
                                          nsIDOMNode *leftDecendent, PRUint32 leftOffset, 
                                          nsIDOMNode *rightDecendent, PRUint32 rightOffset, 
                                          PRUint32 *mmlNodeLeftOffset, PRUint32 *mmlNodeRightOffset, 
                                          nsIDOMNode **left_leftPart, nsIDOMNode **left_rightPart, 
                                          nsIDOMNode **right_leftPart, nsIDOMNode **right_rightPart)
{
  return msiMCaretBase::SplitAtDecendents(editor, leftDecendent, leftOffset, rightDecendent, rightOffset, 
                                          mmlNodeLeftOffset, mmlNodeRightOffset, 
                                          left_leftPart, left_rightPart, 
                                          right_leftPart, right_rightPart);
}


NS_IMETHODIMP
msiUnderAndOrOverCaret::Split(nsIEditor *editor, 
                              nsIDOMNode *appendLeft, 
                              nsIDOMNode *prependRight, 
                              nsIDOMNode **left, 
                              nsIDOMNode **right)
{
  return msiMCaretBase::Split(editor, appendLeft, prependRight, left, right);
}

NS_IMETHODIMP
msiUnderAndOrOverCaret::SetDeletionTransaction(nsIEditor * editor,
                                               PRBool deletingToTheRight, 
                                               nsITransaction ** txn,
                                               PRBool * toRightInParent)
{
  return msiMCaretBase::SetDeletionTransaction(editor, deletingToTheRight, txn, toRightInParent);
}                                            


NS_IMETHODIMP
msiUnderAndOrOverCaret::SetupDeletionTransactions(nsIEditor * editor,
                                                  nsIDOMNode * start,
                                                  PRUint32 startOffset,
                                                  nsIDOMNode * end,
                                                  PRUint32 endOffset,
                                                  nsIArray ** transactionList,
                                                  nsIDOMNode ** coalesceNode,
                                                  PRUint32 * coalesceOffset)
{

  if (m_mathmlNode || !editor || !transactionList || !coalesceNode || !coalesceOffset )
    return NS_ERROR_FAILURE;
  if (!start || !end || !(IS_VALID_NODE_OFFSET(startOffset)) || !(IS_VALID_NODE_OFFSET(endOffset)))
    return NS_ERROR_FAILURE;
  nsCOMPtr<msiIMathMLEditor> msiEditor(do_QueryInterface(editor));
  if (!msiEditor)
    return NS_ERROR_FAILURE;
  
  nsCOMPtr<nsIDOMNodeList> children;
  m_mathmlNode->GetChildNodes(getter_AddRefs(children));
  if (!children)
    return NS_ERROR_FAILURE;  
      
  *coalesceNode = nsnull;
  *coalesceOffset = INVALID;
  *transactionList = nsnull;
  nsresult res(NS_OK);
  nsCOMPtr<nsIMutableArray> mutableTxnArray = do_CreateInstance(NS_ARRAY_CONTRACTID, &res);
  nsCOMPtr<nsIMutableArray> leftTxnList = do_CreateInstance(NS_ARRAY_CONTRACTID, &res);
  nsCOMPtr<nsIMutableArray> rightTxnList = do_CreateInstance(NS_ARRAY_CONTRACTID, &res);
  if (!leftTxnList || !rightTxnList)
    return NS_ERROR_FAILURE;
  PRUint32 leftOffsetInTop(INVALID), rightOffsetInTop(INVALID);
  res = msiMCaretBase::SetUpDeleteTxnsFromDescendent(editor, m_mathmlNode, m_numKids, start, 
                                                     startOffset, PR_TRUE, leftTxnList, leftOffsetInTop);
  if (NS_SUCCEEDED(res))                                                   
    res = msiMCaretBase::SetUpDeleteTxnsFromDescendent(editor, m_mathmlNode, m_numKids, end, 
                                                       endOffset, PR_FALSE, rightTxnList, rightOffsetInTop);
  if (NS_FAILED(res))
    return res;
  if (rightOffsetInTop - leftOffsetInTop > 1)
  {
    nsCOMPtr<msiIMathMLCaret> parentCaret;
    res = msiUtils::SetupPassOffCaretToParent(editor, m_mathmlNode, PR_FALSE, parentCaret);
    if (NS_SUCCEEDED(res) && parentCaret)
    {
      PRUint32 offset(msiIMathMLEditingBC::INVALID);
      nsCOMPtr<nsIDOMNode> parentMMLNode;
      msiUtils::GetOffsetFromCaretInterface(parentCaret, offset);
      msiUtils::GetMathmlNodeFromCaretInterface(parentCaret, parentMMLNode);
      if (NS_SUCCEEDED(res) && offset != msiIMathMLEditingBC::INVALID)
        res = parentCaret->SetupDeletionTransactions(editor, parentMMLNode, offset, 
                                                     parentMMLNode, offset+1, transactionList,
                                                     coalesceNode, coalesceOffset);
    }                                                 
  }
  else if (rightOffsetInTop - leftOffsetInTop == 1)
  {
    nsCOMPtr<nsIDOMElement> inputboxElement;
    nsCOMPtr<nsIDOMNode> newKid;
    PRUint32 flags(msiIMathMLInsertion::FLAGS_NONE);
    res = msiUtils::CreateInputbox(editor, PR_FALSE, PR_FALSE, flags, inputboxElement);
    if (NS_SUCCEEDED(res) && inputboxElement)
      newKid = do_QueryInterface(inputboxElement);
    if (!newKid)
      res = NS_ERROR_FAILURE;  
    nsCOMPtr<nsITransaction> transaction;
    nsCOMPtr<nsIDOMNode> oldKid;
    res = msiUtils::GetChildNode(m_mathmlNode, leftOffsetInTop, oldKid);
    if (NS_SUCCEEDED(res) && oldKid)
      res = msiEditor->CreateReplaceTransaction(newKid, oldKid, m_mathmlNode, getter_AddRefs(transaction));
    if (NS_SUCCEEDED(res) && transaction)
      res = mutableTxnArray->AppendElement(transaction, PR_FALSE);
    else
      res = NS_ERROR_FAILURE;
  }
  else // rightOffsetInTop == leftoffsetInTop
  {
    nsCOMPtr<nsIArray> left(do_QueryInterface(leftTxnList));
    nsCOMPtr<nsIArray> right(do_QueryInterface(rightTxnList));
    PRUint32 leftLen(0), rightLen(0);
    if (left)
      left->GetLength(&leftLen);
    if (right)
      right->GetLength( &rightLen);
      
    if (rightLen > 0)
      res = msiUtils::AppendToMutableList(mutableTxnArray, right);
    if (leftLen > 0)
      res = msiUtils::AppendToMutableList(mutableTxnArray, left);
  }
  nsCOMPtr<nsIArray> txnList(do_QueryInterface(mutableTxnArray));
  PRUint32 len(0);
  if (txnList)
    txnList->GetLength(&len);
  if (NS_SUCCEEDED(res) && len > 0)
  {
    *transactionList = mutableTxnArray;
    NS_ADDREF(*transactionList);
  }  
  return res;
}

NS_IMETHODIMP
msiUnderAndOrOverCaret::SetupCoalesceTransactions(nsIEditor * editor,
                                                  nsIArray ** coalesceTransactions)
{
  return msiMCaretBase::SetupCoalesceTransactions(editor, coalesceTransactions);
}         

NS_IMETHODIMP
msiUnderAndOrOverCaret::SetCoalTransactionsAndNode(nsIEditor * editor,
                                                   PRBool onLeft,
                                                   nsIArray ** transactionList,
                                                   nsIDOMNode **coalesceNode)
{
  return msiMCaretBase::SetCoalTransactionsAndNode(editor, onLeft, transactionList, coalesceNode);
}

NS_IMETHODIMP
msiUnderAndOrOverCaret::CaretLeft(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  if (!node || !offset || !m_mathmlNode || !editor)
    return NS_ERROR_FAILURE;
  m_offset = 0;
  return Accept(editor, FROM_RIGHT, node, offset);
}

NS_IMETHODIMP
msiUnderAndOrOverCaret::CaretRight(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  if (!node || !offset || !m_mathmlNode || !editor)
    return NS_ERROR_FAILURE;
  m_offset = m_numKids;
  return Accept(editor, FROM_LEFT, node, offset);
}

NS_IMETHODIMP
msiUnderAndOrOverCaret::CaretUp(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return CaretLeft(editor, flags, node, offset);
}

NS_IMETHODIMP
msiUnderAndOrOverCaret::CaretDown(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return CaretRight(editor, flags, node, offset);
}

NS_IMETHODIMP
msiUnderAndOrOverCaret::CaretObjectLeft(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  if (!node || !offset || !m_mathmlNode || !editor)
    return NS_ERROR_FAILURE;
  nsresult res(NS_OK);
  nsCOMPtr<msiIMathMLCaret> mathmlEditing;
  if (m_offset == 0)
  {
    msiUtils::SetupPassOffCaretToParent(editor, m_mathmlNode, PR_FALSE, mathmlEditing);
    if (mathmlEditing)
      mathmlEditing->CaretObjectLeft(editor, flags, node, offset); 
    else 
      res = NS_ERROR_FAILURE;
  }
  else
  {
    m_offset = 0;
    res = Accept(editor, FROM_RIGHT, node, offset);
  }
  return res;  
}

NS_IMETHODIMP
msiUnderAndOrOverCaret::CaretObjectRight(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  if (!node || !offset || !m_mathmlNode || !editor)
    return NS_ERROR_FAILURE;
  nsresult res(NS_OK);
  nsCOMPtr<msiIMathMLCaret> mathmlEditing;
  if (m_offset == m_numKids)
  {
    msiUtils::SetupPassOffCaretToParent(editor, m_mathmlNode, PR_TRUE, mathmlEditing);
    if (mathmlEditing)
      mathmlEditing->CaretObjectRight(editor, flags, node, offset); 
    else 
      res = NS_ERROR_FAILURE;
  }
  else
  {
    m_offset = m_numKids;
    res = Accept(editor, FROM_LEFT, node, offset);
  }
  return res;
}

NS_IMETHODIMP
msiUnderAndOrOverCaret::CaretObjectUp(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return CaretLeft(editor, flags, node, offset);
}

NS_IMETHODIMP
msiUnderAndOrOverCaret::CaretObjectDown(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return CaretRight(editor, flags, node, offset);
}

//private
nsresult
msiUnderAndOrOverCaret::GetFramesAndRects(const nsIFrame * underOver, 
                                 nsIFrame ** base, nsIFrame ** script1, nsIFrame ** script2,
                                 nsRect & uoRect, nsRect &bRect, nsRect& s1Rect, nsRect& s2Rect)
{ // relative to scritp's view
  nsresult res(NS_ERROR_FAILURE);
  *script2 = nsnull;
  s2Rect= nsRect(0,0,0,0);
  if (underOver)
  {
    *base = underOver->GetFirstChild(nsnull);
    if (*base)
      *script1 = (*base)->GetNextSibling();
    res = *base && *script1 ? NS_OK : NS_ERROR_FAILURE;  
  }
  if (NS_SUCCEEDED(res))
  {
    uoRect = underOver->GetScreenRectExternal();
    bRect = (*base)->GetScreenRectExternal();
    s1Rect = (*script1)->GetScreenRectExternal();
    if (*script2)
      s2Rect = (*script2)->GetScreenRectExternal();
  }
  return res;
}  

void
msiUnderAndOrOverCaret::GetAboveBelowThresholds(const nsRect& uoRect, const nsRect& bRect, const nsRect& s1Rect, nsRect& s2Rect,
                                             PRInt32& above, PRBool& aboveSet, PRInt32& below, PRBool& belowSet)
{
  aboveSet = PR_FALSE;
  belowSet = PR_FALSE;
  if (s2Rect.IsEmpty())
  {
    if (s1Rect.y < bRect.y)
    {
      above = bRect.y > s1Rect.y+s1Rect.height ? bRect.y : s1Rect.y+s1Rect.height;
      aboveSet = PR_TRUE;
    }
    else
    {
      below = bRect.y+bRect.height < s1Rect.y ? bRect.y+bRect.height : s1Rect.y;
      belowSet = PR_TRUE;
    }
  }
  else
  {
    above = bRect.y > s2Rect.y + s2Rect.height ? bRect.y : s2Rect.y + s2Rect.height;
    below = bRect.y + bRect.height < s1Rect.y ? bRect.y + bRect.height : s1Rect.y;
    aboveSet = PR_TRUE;
    belowSet = PR_TRUE;
  }

}                                             
