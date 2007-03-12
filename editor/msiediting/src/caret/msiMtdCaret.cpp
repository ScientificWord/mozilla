// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiMtdCaret.h"
#include "msiMContainerCaret.h"
#include "nsIDOMNode.h"
#include "nsCOMPtr.h"
#include "msiUtils.h"   

msiMtdCaret::msiMtdCaret(nsIDOMNode* mathmlNode, PRUint32 offset)
: msiMContainerCaret(mathmlNode, offset, MATHML_MTD)
{
}

NS_IMETHODIMP
msiMtdCaret::PrepareForCaret(nsIEditor* editor)
{
  return NS_OK;
}

NS_IMETHODIMP
msiMtdCaret::Inquiry(nsIEditor* editor, PRUint32 inquiryID, PRBool *result)
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
msiMtdCaret::AdjustNodeAndOffsetFromMouseEvent(nsIEditor *editor, nsIPresShell *presShell,
                                                       PRUint32 flags, 
                                                       nsIDOMMouseEvent *mouseEvent, 
                                                       nsIDOMNode **node, 
                                                       PRUint32 *offset)
{
  return msiMContainerCaret::AdjustNodeAndOffsetFromMouseEvent(editor, presShell, flags, 
                                                               mouseEvent, node, offset);
}                                                       
                                   
NS_IMETHODIMP
msiMtdCaret::Accept(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return msiMContainerCaret::Accept(editor, flags, node, offset);
}

NS_IMETHODIMP 
msiMtdCaret::GetSelectableMathFragment(nsIEditor  *editor, 
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
msiMtdCaret::AdjustSelectionPoint(nsIEditor *editor, PRBool leftSelPoint, 
                                      nsIDOMNode** selectionNode, PRUint32 * selectionOffset,
                                      msiIMathMLCaret** parentCaret)
{
  return msiMContainerCaret::AdjustSelectionPoint(editor, leftSelPoint, selectionNode, selectionOffset, parentCaret);
}

NS_IMETHODIMP 
msiMtdCaret::SplitAtDecendents(nsIEditor* editor, 
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
msiMtdCaret::Split(nsIEditor * editor, 
                    nsIDOMNode *leftPart,  // leftPart and rightPart are the
                    nsIDOMNode *rightPart, // partition of the child node at m_offset
                    nsIDOMNode **left, 
                    nsIDOMNode **right)
{
  //SLS why would an mtd split?
  return NS_ERROR_FAILURE;    
} 

NS_IMETHODIMP
msiMtdCaret::SetDeletionTransaction(nsIEditor * editor,
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
msiMtdCaret::SetupDeletionTransactions(nsIEditor * editor,
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
msiMtdCaret::SetupCoalesceTransactions(nsIEditor * editor,
                                        nsIArray ** coalesceTransactions)
{
  return msiMCaretBase::SetupCoalesceTransactions(editor, coalesceTransactions);
}

//TODO
NS_IMETHODIMP
msiMtdCaret::SetCoalTransactionsAndNode(nsIEditor * editor,
                                         PRBool onLeft,
                                         nsIArray ** transactionList,
                                         nsIDOMNode **coalesceNode)
{
  return NS_ERROR_FAILURE;
}                                         

NS_IMETHODIMP
msiMtdCaret::CaretLeft(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
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
msiMtdCaret::CaretRight(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
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
msiMtdCaret::CaretUp(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return msiMCaretBase::CaretUp(editor, flags, node, offset);
}

NS_IMETHODIMP
msiMtdCaret::CaretDown(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return msiMCaretBase::CaretDown(editor, flags, node, offset);
}

//TODO
NS_IMETHODIMP
msiMtdCaret::CaretObjectLeft(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
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
msiMtdCaret::CaretObjectRight(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
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
msiMtdCaret::CaretObjectUp(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return CaretLeft(editor, flags, node, offset);
}

NS_IMETHODIMP
msiMtdCaret::CaretObjectDown(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return CaretRight(editor, flags, node, offset);
}

NS_IMETHODIMP
msiMtdCaret::TabLeft(nsIEditor *editor, nsIDOMNode ** node, PRUint32 *offset)
{
  return msiMCaretBase::TabLeft(editor, node, offset);
}

NS_IMETHODIMP
msiMtdCaret::TabRight(nsIEditor *editor, nsIDOMNode ** node, PRUint32 *offset)
{
  return msiMCaretBase::TabRight(editor, node, offset);
}

