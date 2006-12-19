// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.
#include "msiMrowBoundFenceCaret.h"
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

msiMrowBoundFenceCaret::msiMrowBoundFenceCaret(nsIDOMNode* mathmlNode, PRUint32 offset)
:  msiMCaretBase(mathmlNode, offset, MATHML_MROWBOUNDFENCE)
{
}

NS_IMETHODIMP
msiMrowBoundFenceCaret::PrepareForCaret(nsIEditor* editor)
{
  return NS_OK;
}

NS_IMETHODIMP
msiMrowBoundFenceCaret::Inquiry(nsIEditor* editor, PRUint32 inquiryID, PRBool *result)
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
msiMrowBoundFenceCaret::AdjustNodeAndOffsetFromMouseEvent(nsIEditor *editor, nsIPresShell *presShell,
                                                       PRUint32 flags, 
                                                       nsIDOMMouseEvent *mouseEvent, 
                                                       nsIDOMNode **node, 
                                                       PRUint32 *offset)
{
  if (!editor || !node || !offset || !presShell || !m_mathmlNode || !mouseEvent)
    return NS_ERROR_FAILURE;
  *node = nsnull;
  *offset = INVALID;
  nsIFrame * fenceFrame = nsnull; // no smart pointers for frames.
  nsIFrame * openFrame = nsnull;
  nsIFrame * closeFrame = nsnull;
  nsRect fenceRect(0,0,0,0), openRect(0,0,0,0), closeRect(0,0,0,0);
  nsPoint eventPoint(0,0);
  
  nsresult res = msiMCaretBase::GetPrimaryFrameForNode(presShell, m_mathmlNode, &fenceFrame);
  if (NS_SUCCEEDED(res) && fenceFrame)
  {  
    fenceRect = fenceFrame->GetScreenRectExternal();
    openFrame = fenceFrame->GetFirstChild(nsnull);
    if (openFrame)
    {
      openRect = openFrame->GetScreenRectExternal();
      closeFrame = openFrame;
      while (closeFrame->GetNextSibling())
        closeFrame = closeFrame->GetNextSibling();
      closeRect = closeFrame->GetScreenRectExternal();
    }
    else
      res = NS_ERROR_FAILURE;
        
  }
  if (NS_SUCCEEDED(res))
    res = msiUtils::GetScreenPointFromMouseEvent(mouseEvent, eventPoint);                                     
  if (NS_SUCCEEDED(res))
  {
    PRInt32 lfThres(0), rtThres(0);
    GetThresholds(fenceRect, openRect, closeRect, lfThres, rtThres);
    if (!(flags&FROM_PARENT) && ((eventPoint.x <= fenceRect.x + lfThres) || 
                                 (fenceRect.x + fenceRect.width - rtThres <= eventPoint.x))) 
    { 
      m_offset = eventPoint.x <= fenceRect.x + lfThres ? 0 : m_numKids;
      flags = eventPoint.x <= fenceRect.x + lfThres ? FROM_RIGHT : FROM_LEFT;
      res = Accept(editor, flags, node, offset);
    }
    else
    {
      if (eventPoint.x <= fenceRect.x + (fenceRect.width/2))
      {
        m_offset = 1;
        res =Accept(editor, FROM_LEFT, node, offset);
      }
      else
      {
        m_offset = m_numKids-1;
        res = Accept(editor, FROM_RIGHT, node, offset);
      }  
    }
  }
  return res;   
}                                                       


NS_IMETHODIMP
msiMrowBoundFenceCaret::Accept(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  if (!node || !offset || !m_mathmlNode || !editor)
    return NS_ERROR_FAILURE;
  NS_ASSERTION(flags & FROM_LEFT|FROM_RIGHT, "Accept called without From left or from right");
  nsresult res(NS_OK);
  nsCOMPtr<msiIMathMLCaret> mathmlEditing;
  nsCOMPtr<nsIDOMNode> child;
  msiUtils::GetChildNode(m_mathmlNode, 1, child);
  if ((flags & FROM_RIGHT) && m_offset > 1 && m_offset < m_numKids)
  {  
    msiUtils::GetChildNode(m_mathmlNode, m_offset-1, child);
    NS_ASSERTION(child, "Null child node");
    if (child)
    {
      msiUtils::GetMathMLCaretInterface(editor, child, RIGHT_MOST, mathmlEditing);
      NS_ASSERTION(mathmlEditing, "Null mathmlEditing interface");
      if (NS_SUCCEEDED(res) && mathmlEditing)
        res = mathmlEditing->Accept(editor, FROM_PARENT|FROM_RIGHT, node, offset);
      else
        res = NS_ERROR_FAILURE;  
    }
    else
      res = NS_ERROR_FAILURE;  
  }
  else if ((flags & FROM_LEFT) && m_offset > 0 && m_offset < m_numKids - 1)
  {
    msiUtils::GetChildNode(m_mathmlNode, m_offset, child);
    NS_ASSERTION(child, "Null child node");
    if (child)
    {
      msiUtils::GetMathMLCaretInterface(editor, child, 0, mathmlEditing);
      NS_ASSERTION(mathmlEditing, "Null mathmlEditing interface");
      if (NS_SUCCEEDED(res) && mathmlEditing)
        res = mathmlEditing->Accept(editor, FROM_PARENT|FROM_LEFT, node, offset);
      else
        res = NS_ERROR_FAILURE;  
    }
    else
      res = NS_ERROR_FAILURE;  
  }
  else if (m_offset <= 1 || m_offset+1 >= m_numKids)
  { 
    nsCOMPtr<msiIMathMLCaret> parent;
    PRBool incOffset(m_offset+1 >= m_numKids);
    flags = FROM_CHILD;
    flags |= incOffset ? FROM_LEFT : FROM_RIGHT;
    msiUtils::SetupPassOffCaretToParent(editor, m_mathmlNode, incOffset, parent);
    if (!(flags&FROM_PARENT) && parent)
      res = parent->Accept(editor, flags, node, offset);
    if (*node == nsnull)
    {
      if (m_offset == 1)
        m_offset = 0;
      else if (m_offset+1 == m_numKids)
        m_offset = m_numKids;
      *node = m_mathmlNode;
      NS_ADDREF(*node);
      *offset = m_offset; 
      res = NS_OK;   
    }
  }
  else
  {
    *node = m_mathmlNode;
    NS_ADDREF(*node);
    *offset = m_offset; 
    res = NS_OK;   
  }
  return res;
}

NS_IMETHODIMP 
msiMrowBoundFenceCaret::GetSelectableMathFragment(nsIEditor  *editor, 
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
  nsCOMPtr<msiIMathMLCaret> parent;
  nsresult res = msiUtils::SetupPassOffCaretToParent(editor, m_mathmlNode, PR_FALSE, parent);
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
  return res;                                            
}


NS_IMETHODIMP
msiMrowBoundFenceCaret::AdjustSelectionPoint(nsIEditor *editor, PRBool leftSelPoint, 
                                             nsIDOMNode** selectionNode, PRUint32 * selectionOffset,
                                             msiIMathMLCaret** parentCaret)
{
  return msiMCaretBase::AdjustSelectionPoint(editor, leftSelPoint, selectionNode, selectionOffset, parentCaret);
}

NS_IMETHODIMP 
msiMrowBoundFenceCaret::SplitAtDecendents(nsIEditor* editor, 
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
msiMrowBoundFenceCaret::Split(nsIEditor *editor, 
                              nsIDOMNode *appendLeft, 
                              nsIDOMNode *appendRight, 
                              nsIDOMNode **left, 
                              nsIDOMNode **right)
{
  return msiMCaretBase::Split(editor, appendLeft, appendRight, left, right);
} 

NS_IMETHODIMP
msiMrowBoundFenceCaret::SetDeletionTransaction(nsIEditor * editor,
                                               PRBool deletingToTheRight, 
                                               nsITransaction ** txn,
                                               PRBool * toRightInParent)
{
  return msiMCaretBase::SetDeletionTransaction(editor, deletingToTheRight, txn, toRightInParent);
}                                            
                                    

NS_IMETHODIMP
msiMrowBoundFenceCaret::SetupDeletionTransactions(nsIEditor * editor,
                                                  nsIDOMNode * start,
                                                  PRUint32 startOffset,
                                                  nsIDOMNode * end,
                                                  PRUint32 endOffset,
                                                  nsIArray ** transactionList,
                                                  nsIDOMNode ** coalesceNode,
                                                  PRUint32 * coalesceOffset)
{
  if (!m_mathmlNode || !editor || !transactionList || !coalesceNode || !coalesceOffset)
    return NS_ERROR_FAILURE;
  if (!(IS_VALID_NODE_OFFSET(startOffset)) || !(IS_VALID_NODE_OFFSET(endOffset)))
    return NS_ERROR_FAILURE;
  nsCOMPtr<msiIMathMLCaret> parentCaret;
  nsresult  res = msiUtils::SetupPassOffCaretToParent(editor, m_mathmlNode, PR_FALSE, parentCaret);
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
  else
    res = NS_ERROR_FAILURE;
  return res;
}   

NS_IMETHODIMP
msiMrowBoundFenceCaret::SetupCoalesceTransactions(nsIEditor * editor,
                                                  nsIArray ** coalesceTransactions)
{
  return msiMCaretBase::SetupCoalesceTransactions(editor, coalesceTransactions);
}         

NS_IMETHODIMP
msiMrowBoundFenceCaret::SetCoalTransactionsAndNode(nsIEditor * editor,
                                                   PRBool onLeft,
                                                   nsIArray ** transactionList,
                                                   nsIDOMNode **coalesceNode)
{
  return msiMCaretBase::SetCoalTransactionsAndNode(editor, onLeft, transactionList, coalesceNode);
}
  
NS_IMETHODIMP
msiMrowBoundFenceCaret::CaretLeft(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
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
        res = mathmlEditing->CaretLeft(editor, FROM_CHILD, node, offset); 
    }
    else 
      res = NS_ERROR_FAILURE;
  }
  else if (m_offset == 1)
  {
    m_offset = 0;
    res = Accept(editor, FROM_RIGHT, node, offset);
  }
  else if (m_offset == m_numKids || m_offset+1 == m_numKids)
  {
    NS_ASSERTION(m_offset == m_numKids, "m_offset == m_numKids-1 should not occur.");
    m_offset = m_numKids-1;
    res = Accept(editor, FROM_RIGHT, node, offset);
  }
  else
    res = msiMCaretBase::CaretLeft(editor, flags, node, offset);
  return res;
}

NS_IMETHODIMP
msiMrowBoundFenceCaret::CaretRight(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  if (!node || !offset || !m_mathmlNode || !editor)
    return NS_ERROR_FAILURE;
  nsresult res(NS_OK);
  nsCOMPtr<msiIMathMLCaret> mathmlEditing;
  if (m_offset == m_numKids)
  {
    msiUtils::SetupPassOffCaretToParent(editor, m_mathmlNode, PR_TRUE, mathmlEditing);
    if (mathmlEditing)
    {
      flags = FROM_CHILD;
        res = mathmlEditing->CaretRight(editor, flags, node, offset); 
    }
    else 
      res = NS_ERROR_FAILURE;
  }
  else if (m_offset+1 == m_numKids)
  {
    m_offset = m_numKids;
    res = Accept(editor, FROM_LEFT, node, offset); 
  }
  else if (m_offset == 0 || m_offset == 1)
  {
    NS_ASSERTION(m_offset == 0, "m_offset == 1 should not occur.");
    m_offset = 1;
    res =Accept(editor, FROM_LEFT, node, offset);
  }
  else
    res = msiMCaretBase::CaretRight(editor, flags, node, offset);
  return res;
}

NS_IMETHODIMP
msiMrowBoundFenceCaret::CaretUp(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return CaretLeft(editor, flags, node, offset);
}

NS_IMETHODIMP
msiMrowBoundFenceCaret::CaretDown(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return CaretRight(editor, flags, node, offset);
}

NS_IMETHODIMP
msiMrowBoundFenceCaret::CaretObjectLeft(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  m_offset = 0;
  return Accept(editor, FROM_RIGHT, node, offset);
}

NS_IMETHODIMP
msiMrowBoundFenceCaret::CaretObjectRight(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  m_offset = m_numKids;
  return Accept(editor, FROM_LEFT, node, offset);
}

NS_IMETHODIMP
msiMrowBoundFenceCaret::CaretObjectUp(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return CaretLeft(editor, flags, node, offset);
}

NS_IMETHODIMP
msiMrowBoundFenceCaret::CaretObjectDown(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return CaretRight(editor, flags, node, offset);
}

//private
#define MIN_THRESHOLD 2 //TODO -- how should this be determined.      
void msiMrowBoundFenceCaret::GetThresholds(const nsRect &frameRect, 
                                           const nsRect &openRect, const nsRect &closeRect, 
                                           PRInt32 &left, PRInt32 & right)
{
  PRInt32 min(MIN_THRESHOLD);
  left = openRect.width/2;
  PRInt32 gap = openRect.x - frameRect.x;
  if (gap > 0)
    left += gap;
  right = closeRect.width/2;
  gap = frameRect.x + frameRect.width - closeRect.x - closeRect.width;
  if (gap > 0)  
    right += gap; 
  if (left < min)
    left = min;
  if (right < min)
    right = min;
}                                      
