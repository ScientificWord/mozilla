// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiMtrCaret.h"
#include "msiMContainerCaret.h"
#include "nsIDOMNode.h"
#include "nsCOMPtr.h"
#include "msiUtils.h"   

msiMtrCaret::msiMtrCaret(nsIDOMNode* mathmlNode, PRUint32 offset)
:  msiMContainerCaret(mathmlNode, offset, MATHML_MTR)
{
}

NS_IMETHODIMP
msiMtrCaret::PrepareForCaret(nsIEditor* editor)
{
  return NS_OK;
}

NS_IMETHODIMP
msiMtrCaret::Inquiry(nsIEditor* editor, PRUint32 inquiryID, PRBool *result)
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
msiMtrCaret::AdjustNodeAndOffsetFromMouseEvent(nsIEditor *editor, nsIPresShell *presShell,
                                                       PRUint32 flags, 
                                                       nsIDOMMouseEvent *mouseEvent, 
                                                       nsIDOMNode **node, 
                                                       PRUint32 *offset)
{
  return msiMContainerCaret::AdjustNodeAndOffsetFromMouseEvent(editor, presShell, flags, 
                                                               mouseEvent, node, offset);
}                                                       
                                   
NS_IMETHODIMP
msiMtrCaret::Accept(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  if (!node || !offset || !m_mathmlNode || !editor)
    return NS_ERROR_FAILURE;
  nsCOMPtr<nsIDOMNode> child;
  nsresult res(NS_ERROR_FAILURE);  
  NS_ASSERTION(flags & (FROM_LEFT|FROM_RIGHT|FROM_ABOVE|FROM_BELOW), "Accept called without From left or from right");
  if ((flags & (FROM_ABOVE|FROM_BELOW))
      || ((flags & FROM_RIGHT) && m_offset > 0)
      || ((flags & FROM_LEFT) && m_offset < m_numKids))
  {
    PRUint32 index = flags & FROM_RIGHT ? m_offset-1 : m_offset;
    msiUtils::GetChildNode(m_mathmlNode, index, child);
    NS_ASSERTION(child, "Null child node");
    if (child)
    {
      PRUint32 newPos = 0;
      if (flags & FROM_RIGHT)
        if (!(flags & FROM_ABOVE))  // special case for SHIFT-TAB
          flags = RIGHT_MOST;
      nsCOMPtr<msiIMathMLCaret> mathmlEditing;
      msiUtils::GetMathMLCaretInterface(editor, child, newPos, mathmlEditing);
      NS_ASSERTION(mathmlEditing, "Null mathmlEditing interface");
      if (mathmlEditing)
      {
        PRUint32 currFlags = (flags&FROM_RIGHT) ? FROM_RIGHT : FROM_LEFT; 
        currFlags |= FROM_PARENT;
        res = mathmlEditing->Accept(editor, currFlags, node, offset);
      } 
    }
  }
  return res;
}

NS_IMETHODIMP 
msiMtrCaret::GetSelectableMathFragment(nsIEditor  *editor, 
                                         nsIDOMNode *start,      PRUint32 startOffset, 
                                         nsIDOMNode *end,        PRUint32 endOffset, 
                                         nsIDOMNode **fragStart, PRUint32 *fragStartOffset,
                                         nsIDOMNode **fragEnd,   PRUint32 *fragEndOffset)
{
  return msiMContainerCaret::GetSelectableMathFragment(editor, start, startOffset, 
                                                       end, endOffset, fragStart, fragStartOffset,
                                                       fragEnd, fragEndOffset);
}

NS_IMETHODIMP
msiMtrCaret::AdjustSelectionPoint(nsIEditor *editor, PRBool leftSelPoint, 
                                      nsIDOMNode** selectionNode, PRUint32 * selectionOffset,
                                      msiIMathMLCaret** parentCaret)
{
  return msiMContainerCaret::AdjustSelectionPoint(editor, leftSelPoint, selectionNode, selectionOffset, parentCaret);
}

NS_IMETHODIMP 
msiMtrCaret::SplitAtDecendents(nsIEditor* editor, 
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
msiMtrCaret::Split(nsIEditor * editor, 
                    nsIDOMNode *leftPart,  // leftPart and rightPart are the
                    nsIDOMNode *rightPart, // partition of the child node at m_offset
                    nsIDOMNode **left, 
                    nsIDOMNode **right)
{
  //SLS why would an mtr split?
  return NS_ERROR_FAILURE;    
} 

NS_IMETHODIMP
msiMtrCaret::SetDeletionTransaction(nsIEditor * editor,
                                     PRBool deletingToTheRight, 
                                     nsITransaction ** txn,
                                     PRBool * toRightInParent)
{
  return msiMCaretBase::SetDeletionTransaction(editor,
                                               deletingToTheRight, 
                                               txn,
                                               toRightInParent);
}  

NS_IMETHODIMP
msiMtrCaret::SetupDeletionTransactions(nsIEditor * editor,
                                        nsIDOMNode * start,
                                        PRUint32 startOffset,
                                        nsIDOMNode * end,
                                        PRUint32 endOffset,
                                        nsIArray ** transactionList,
                                        nsIDOMNode ** coalesceNode,
                                        PRUint32 * coalesceOffset)
{
  return msiMCaretBase::SetupDeletionTransactions(editor, start, startOffset, 
                                                  end, endOffset, transactionList, 
                                                  coalesceNode, coalesceOffset);
}

NS_IMETHODIMP
msiMtrCaret::SetupCoalesceTransactions(nsIEditor * editor,
                                        nsIArray ** coalesceTransactions)
{
  return msiMCaretBase::SetupCoalesceTransactions(editor, coalesceTransactions);
}

//TODO
NS_IMETHODIMP
msiMtrCaret::SetCoalTransactionsAndNode(nsIEditor * editor,
                                         PRBool onLeft,
                                         nsIArray ** transactionList,
                                         nsIDOMNode **coalesceNode)
{
  return NS_ERROR_FAILURE;
}                                         

NS_IMETHODIMP
msiMtrCaret::CaretLeft(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  if (!node || !offset || !m_mathmlNode || !editor)
    return NS_ERROR_FAILURE;
  nsresult res(NS_OK);
  if (m_offset == 0)
  {
    nsCOMPtr<msiIMathMLCaret> mathmlEditing;
    msiUtils::SetupPassOffCaretToParent(editor, m_mathmlNode, PR_FALSE, mathmlEditing);
    flags = FROM_CHILD;
    if (mathmlEditing)
      res = mathmlEditing->CaretLeft(editor, flags, node, offset);
    else
      res = NS_ERROR_FAILURE;  
  }
  else 
    res = msiMContainerCaret::CaretLeft(editor, flags, node, offset);
  return res;
}

NS_IMETHODIMP
msiMtrCaret::CaretRight(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  if (!node || !offset || !m_mathmlNode || !editor)
    return NS_ERROR_FAILURE;
  nsresult res(NS_OK);
  if (m_offset == m_numKids)
  {
    nsCOMPtr<msiIMathMLCaret> mathmlEditing;
    msiUtils::SetupPassOffCaretToParent(editor, m_mathmlNode, PR_TRUE, mathmlEditing);
    flags = FROM_CHILD;
    if (mathmlEditing)
      res = mathmlEditing->CaretRight(editor, flags, node, offset);
    else
      res = NS_ERROR_FAILURE;  
  }
  else 
    res = msiMContainerCaret::CaretRight(editor, flags, node, offset);
  return res;
}

NS_IMETHODIMP
msiMtrCaret::CaretUp(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return msiMCaretBase::CaretUp(editor, flags, node, offset);
}

NS_IMETHODIMP
msiMtrCaret::CaretDown(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return msiMCaretBase::CaretDown(editor, flags, node, offset);
}

//TODO
NS_IMETHODIMP
msiMtrCaret::CaretObjectLeft(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  if (!node || !offset || !m_mathmlNode || !editor)
    return NS_ERROR_FAILURE;
  nsresult res(NS_OK);
  if (m_offset == 0)
  {
    nsCOMPtr<msiIMathMLCaret> mathmlEditing;
    msiUtils::SetupPassOffCaretToParent(editor, m_mathmlNode, PR_FALSE, mathmlEditing);
    flags = FROM_CHILD;
    if (mathmlEditing)
      res = mathmlEditing->CaretObjectLeft(editor, flags, node, offset);
    else
      res = NS_ERROR_FAILURE;  
  }
  else 
    res = msiMContainerCaret::CaretObjectLeft(editor, flags, node, offset);
  return res;
}

//TODO
NS_IMETHODIMP
msiMtrCaret::CaretObjectRight(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  if (!node || !offset || !m_mathmlNode || !editor)
    return NS_ERROR_FAILURE;
  nsresult res(NS_OK);
  if (m_offset == m_numKids)
  {
    nsCOMPtr<msiIMathMLCaret> mathmlEditing;
    msiUtils::SetupPassOffCaretToParent(editor, m_mathmlNode, PR_TRUE, mathmlEditing);
    flags = FROM_CHILD;
    if (mathmlEditing)
      res = mathmlEditing->CaretObjectRight(editor, flags, node, offset);
    else
      res = NS_ERROR_FAILURE;  
  }
  else 
    res = msiMContainerCaret::CaretObjectRight(editor, flags, node, offset);
  return res;
}

NS_IMETHODIMP
msiMtrCaret::CaretObjectUp(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return CaretLeft(editor, flags, node, offset);
}

NS_IMETHODIMP
msiMtrCaret::CaretObjectDown(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return CaretRight(editor, flags, node, offset);
}

NS_IMETHODIMP
msiMtrCaret::TabLeft(nsIEditor *editor, nsIDOMNode ** node, PRUint32 *offset)
{
  return msiMCaretBase::TabLeft(editor, node, offset);
}

NS_IMETHODIMP
msiMtrCaret::TabRight(nsIEditor *editor, nsIDOMNode ** node, PRUint32 *offset)
{
  return msiMCaretBase::TabRight(editor, node, offset);
}


