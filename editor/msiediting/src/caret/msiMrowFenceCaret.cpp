// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.
#include "msiMrowFenceCaret.h"
#include "msiIMrowEditing.h"
#include "msiUtils.h"
#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "nsIEditor.h"
#include "nsISelection.h"
#include "nsIFrame.h"
#include "nsIContent.h"
#include "nsIView.h"
#include "nsPresContext.h"
#include "nsIDOMMouseEvent.h"
#include "nsIPrivateDOMEvent.h"
#include "msiIMathMLEditor.h"
#include "nsITransaction.h"
#include "nsIDOMNodeList.h"
#include "nsIMutableArray.h"
#include "nsComponentManagerUtils.h"

msiMrowFenceCaret::msiMrowFenceCaret(nsIDOMNode* mathmlNode, PRUint32 offset)
:  msiMContainerCaret(mathmlNode, offset, MATHML_MROWFENCE)
{
}

NS_IMETHODIMP
msiMrowFenceCaret::PrepareForCaret(nsIEditor* editor)
{
  return NS_OK;
}

NS_IMETHODIMP
msiMrowFenceCaret::Inquiry(nsIEditor* editor, PRUint32 inquiryID, PRBool *result)
{
  if (inquiryID == AT_RIGHT_EQUIV_TO_0 || inquiryID == AT_LEFT_EQUIV_TO_RIGHT_MOST) 
    *result = PR_TRUE;
  else if (inquiryID == CAN_SELECT_CHILD_LEAF) 
    *result = PR_TRUE;
  else
    *result = PR_FALSE;
  return NS_OK;
}

NS_IMETHODIMP
msiMrowFenceCaret::AdjustNodeAndOffsetFromMouseEvent(nsIEditor *editor, nsIPresShell *presShell,
                                                       PRUint32 flags, 
                                                       nsIDOMMouseEvent *mouseEvent, 
                                                       nsIDOMNode **node, 
                                                       PRUint32 *offset)
{
  return msiMContainerCaret::AdjustNodeAndOffsetFromMouseEvent(editor, presShell, flags, 
                                                               mouseEvent, node, offset);
}                                                       
   
                                   
NS_IMETHODIMP
msiMrowFenceCaret::Accept(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  if (!node || !offset || !m_mathmlNode || !editor)
    return NS_ERROR_FAILURE;
  NS_ASSERTION(flags & FROM_LEFT|FROM_RIGHT, "Accept called without From left or from right");
  nsresult res(NS_OK);
  nsCOMPtr<nsIDOMNode> child;
  msiUtils::GetChildNode(m_mathmlNode, 1, child);
  if (m_offset == 0 || m_offset == m_numKids)
  { 
    nsCOMPtr<msiIMathMLCaret> parent;
    PRBool incOffset(m_offset==m_numKids);
    flags = FROM_CHILD;
    flags |= incOffset ? FROM_LEFT : FROM_RIGHT;
    msiUtils::SetupPassOffCaretToParent(editor, m_mathmlNode, incOffset, parent);
    if (!(flags&FROM_PARENT) && parent)
      parent->Accept(editor, flags, node, offset);
      
  }
  else if (m_numKids == 3 && child && msiUtils::IsInputbox(editor, child))
  {
    nsCOMPtr<msiIMathMLCaret> mathmlEditing;
    res = msiUtils::GetMathMLCaretInterface(editor, child, 1, mathmlEditing);
    NS_ASSERTION(mathmlEditing, "Null mathml caret interface");
    if (mathmlEditing)
      res = mathmlEditing->Accept(editor, FROM_PARENT|FROM_LEFT, node, offset);
    else
      res = NS_ERROR_FAILURE;  
  }
  else
    res = msiMContainerCaret::Accept(editor, flags, node, offset);
  if (*node == nsnull)
  {
    *node = m_mathmlNode;
    NS_ADDREF(*node);
    *offset = m_offset;
  }
  return NS_OK;  
}

NS_IMETHODIMP 
msiMrowFenceCaret::GetSelectableMathFragment(nsIEditor  *editor, 
                                             nsIDOMNode *start,      PRUint32 startOffset, 
                                             nsIDOMNode *end,        PRUint32 endOffset, 
                                             nsIDOMNode **fragStart, PRUint32 *fragStartOffset,
                                             nsIDOMNode **fragEnd,   PRUint32 *fragEndOffset)
{
  //ljh -- code below assumes the the fence is the "common ancestor" of start and end.
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
  PRInt32 startCompare(0), endCompare(0);
  nsresult res = msiUtils::ComparePoints(editor, start, startOffset, m_mathmlNode, 1, startCompare);
  if (NS_SUCCEEDED(res))
    res = msiUtils::ComparePoints(editor, end, endOffset, m_mathmlNode, m_numKids-1, endCompare);
  if (startCompare == -1 || endCompare == 1)
  {
    nsCOMPtr<msiIMathMLCaret> parent;
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
  else
    res = msiMContainerCaret::GetSelectableMathFragment(editor, start, startOffset, 
                                                  end, endOffset, fragStart, fragStartOffset,
                                                  fragEnd, fragEndOffset);
  return res;                                                  
}

NS_IMETHODIMP
msiMrowFenceCaret::AdjustSelectionPoint(nsIEditor *editor, PRBool leftSelPoint, 
                                        nsIDOMNode** selectionNode, PRUint32 * selectionOffset,
                                        msiIMathMLCaret** parentCaret)
{
  return msiMCaretBase::AdjustSelectionPoint(editor, leftSelPoint, selectionNode, selectionOffset, parentCaret);
}

NS_IMETHODIMP 
msiMrowFenceCaret::SplitAtDecendents(nsIEditor* editor, 
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
msiMrowFenceCaret::Split(nsIEditor *editor, 
                         nsIDOMNode *appendLeft, 
                         nsIDOMNode *prependRight, 
                         nsIDOMNode **left, 
                         nsIDOMNode **right)
{
  return msiMCaretBase::Split(editor, appendLeft, prependRight, left, right);
}

NS_IMETHODIMP
msiMrowFenceCaret::SetDeletionTransaction(nsIEditor * editor,
                                          PRBool deletingToTheRight, 
                                          nsITransaction ** txn,
                                          PRBool * toRightInParent)
{
  return msiMCaretBase::SetDeletionTransaction(editor, deletingToTheRight, txn, toRightInParent);
}                                            
    

NS_IMETHODIMP
msiMrowFenceCaret::SetupDeletionTransactions(nsIEditor * editor,
                                             nsIDOMNode * start,
                                             PRUint32 startOffset,
                                             nsIDOMNode * end,
                                             PRUint32 endOffset,
                                             nsIArray ** transactionList,
                                             nsIDOMNode ** coalesceNode,
                                             PRUint32 * coalesceOffset)
{
  if (!m_mathmlNode || !editor || !transactionList || !coalesceNode || !coalesceOffset )
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
  
  if (leftOffsetInTop == 0 || rightOffsetInTop == m_numKids)
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
  else if (leftOffsetInTop == 1 || rightOffsetInTop == m_numKids-1)
  {
    // replace contents of enclosed with input box.
    nsCOMPtr<nsIDOMElement> inputbox;
    nsCOMPtr<nsITransaction> inputboxTxn;
    PRUint32 flags(msiIMathMLInsertion::FLAGS_NONE);
    res = msiUtils::CreateInputbox(editor, PR_FALSE, PR_FALSE, flags, inputbox);
    nsCOMPtr<nsIDOMNode> inputboxNode(do_QueryInterface(inputbox));
    if (NS_SUCCEEDED(res) && inputboxNode) 
    {
      res = msiEditor->CreateInsertTransaction(inputboxNode, m_mathmlNode, 1, getter_AddRefs(inputboxTxn));
      if (NS_FAILED(res) || !inputboxTxn)
        res = NS_ERROR_FAILURE;
      
    }  
    else
      res = NS_ERROR_FAILURE;
    for (PRUint32 i=leftOffsetInTop; NS_SUCCEEDED(res) && i < rightOffsetInTop; i++)
    {
      nsCOMPtr<nsIDOMNode> child;
      res = children->Item(i, getter_AddRefs(child));
      if (NS_SUCCEEDED(res) && child)
      {
        nsCOMPtr<nsITransaction> transaction;
        res = msiEditor->CreateDeleteTransaction(child, getter_AddRefs(transaction));
        if (NS_SUCCEEDED(res) && transaction)
          res = mutableTxnArray->AppendElement(transaction, PR_FALSE);
        else
          res = NS_ERROR_FAILURE;
      }
      else
        res = NS_ERROR_FAILURE;
    }
    if (NS_SUCCEEDED(res))
      res = mutableTxnArray->AppendElement(inputboxTxn, PR_FALSE);
    nsCOMPtr<nsIArray> txnList(do_QueryInterface(mutableTxnArray));
    if (NS_SUCCEEDED(res) && txnList)
    {
      *transactionList = txnList;
      NS_ADDREF(*transactionList);
    }
  }
  else
    res = msiMCaretBase::SetupDeletionTransactions(editor, start, startOffset, 
                                                   end, endOffset, transactionList, 
                                                   coalesceNode, coalesceOffset);
  return res;
}
                                 
NS_IMETHODIMP
msiMrowFenceCaret::SetupCoalesceTransactions(nsIEditor * editor,
                                                  nsIArray ** coalesceTransactions)
{
  return msiMCaretBase::SetupCoalesceTransactions(editor, coalesceTransactions);
}         

NS_IMETHODIMP
msiMrowFenceCaret::CaretLeft(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  if (!node || !offset || !m_mathmlNode || !editor)
    return NS_ERROR_FAILURE;
  nsresult res(NS_OK);
  if (m_offset == 0)
  {
    nsCOMPtr<msiIMathMLCaret> mathmlEditing;
    msiUtils::SetupPassOffCaretToParent(editor, m_mathmlNode, PR_FALSE, mathmlEditing);
    if (mathmlEditing)
    {
      flags = FROM_CHILD;
      res = mathmlEditing->CaretLeft(editor, flags, node, offset); 
    }
    else 
      res = NS_ERROR_FAILURE;
  }
  else if (m_offset == 1)
  { 
    m_offset = 0;
    res = Accept(editor, FROM_RIGHT, node, offset); 
  }
  else if (m_offset == m_numKids)
  {
    m_offset = m_numKids-1;
    res = Accept(editor, FROM_RIGHT, node, offset);
    
  }
  else
    res = msiMContainerCaret::CaretLeft(editor, flags, node, offset);
  return res;
}

NS_IMETHODIMP
msiMrowFenceCaret::CaretRight(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  if (!node || !offset || !m_mathmlNode || !editor)
    return NS_ERROR_FAILURE;
  nsresult res(NS_OK);
  if (m_offset == m_numKids)
  {
    nsCOMPtr<msiIMathMLCaret> mathmlEditing;
    msiUtils::SetupPassOffCaretToParent(editor, m_mathmlNode, PR_TRUE, mathmlEditing);
    if (mathmlEditing)
    {
      flags = FROM_CHILD;
      res = mathmlEditing->CaretRight(editor, flags, node, offset); 
    }
    else 
      res = NS_ERROR_FAILURE;
  }
  else if (m_offset+1== m_numKids)
  { 
    m_offset = m_numKids;
    res = Accept(editor, FROM_LEFT, node, offset); 
  }
  else if (m_offset == 0)
  {
    m_offset = 1;
    res = Accept(editor, FROM_LEFT, node, offset);
    
  }
  else
    res = msiMContainerCaret::CaretRight(editor, flags, node, offset);
  return res;
}

NS_IMETHODIMP
msiMrowFenceCaret::CaretUp(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return CaretLeft(editor, flags, node, offset);
}

NS_IMETHODIMP
msiMrowFenceCaret::CaretDown(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return CaretRight(editor, flags, node, offset);
}

NS_IMETHODIMP
msiMrowFenceCaret::CaretObjectLeft(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  if (!node || !offset || !m_mathmlNode || !editor)
    return NS_ERROR_FAILURE;
  nsresult res(NS_OK);
  if (m_offset == 0)
  {
    nsCOMPtr<msiIMathMLCaret> mathmlEditing;
    msiUtils::SetupPassOffCaretToParent(editor, m_mathmlNode, PR_FALSE, mathmlEditing);
    if (mathmlEditing)
    {
      flags = FROM_CHILD;
      res = mathmlEditing->CaretObjectLeft(editor, flags, node, offset); 
    }
    else 
      res = NS_ERROR_FAILURE;
  }
  else if (m_offset == 1 || m_offset == m_numKids)
  { 
    m_offset = 0;
    res = Accept(editor, FROM_RIGHT, node, offset); 
  }
  else
    res = msiMContainerCaret::CaretObjectLeft(editor, flags, node, offset);
  return res;
}

NS_IMETHODIMP
msiMrowFenceCaret::CaretObjectRight(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  if (!node || !offset || !m_mathmlNode || !editor)
    return NS_ERROR_FAILURE;
  nsresult res(NS_OK);
  if (m_offset == m_numKids)
  {
    nsCOMPtr<msiIMathMLCaret> mathmlEditing;
    msiUtils::SetupPassOffCaretToParent(editor, m_mathmlNode, PR_TRUE, mathmlEditing);
    if (mathmlEditing)
    {
      flags = FROM_CHILD;
      res = mathmlEditing->CaretObjectRight(editor, flags, node, offset); 
    }
    else 
      res = NS_ERROR_FAILURE;
  }
  else if (m_offset+1== m_numKids || m_offset == 0)
  { 
    m_offset = m_numKids;
    res = Accept(editor, FROM_LEFT, node, offset); 
  }
  else
    res = msiMContainerCaret::CaretObjectRight(editor, flags, node, offset);
  return res;
}

NS_IMETHODIMP
msiMrowFenceCaret::CaretObjectUp(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return CaretLeft(editor, flags, node, offset);
}

NS_IMETHODIMP
msiMrowFenceCaret::CaretObjectDown(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return CaretRight(editor, flags, node, offset);
}

//private
#define MIN_THRESHOLD 2 //TODO -- how should this be determined.      
void msiMrowFenceCaret::GetThresholds(const nsRect &frameRect, 
                                      const nsRect &openRect, const nsRect &closeRect, 
                                      PRInt32 &left, PRInt32 & right)
{
  PRInt32 min(MIN_THRESHOLD);
  left = openRect.width/2;
  if (openRect.x > 0)
    left += openRect.x;
  right = closeRect.width/2;
  PRInt32 gap = frameRect.width - closeRect.x - closeRect.width;
  if (gap > 0)  
    right += gap; 
  if (left < min)
    left = min;
  if (right < min)
    right = min;
}                                      
