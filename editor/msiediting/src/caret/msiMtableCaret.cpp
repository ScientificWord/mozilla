// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiMtableCaret.h"
#include "msiUtils.h"
#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "nsIEditor.h"

msiMtableCaret::msiMtableCaret(nsIDOMNode* mathmlNode, PRUint32 offset)
: msiMCaretBase(mathmlNode, offset, MATHML_MTABLE)
{
}

NS_IMETHODIMP
msiMtableCaret::PrepareForCaret(nsIEditor* editor)
{
  return msiMCaretBase::PrepareForCaret(editor);
}

NS_IMETHODIMP
msiMtableCaret::Inquiry(nsIEditor* editor, PRUint32 inquiryID, PRBool *result)
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
msiMtableCaret::AdjustNodeAndOffsetFromMouseEvent(nsIEditor *editor, nsIPresShell *presShell,
                                                  PRUint32 flags, 
                                                  nsIDOMMouseEvent *mouseEvent, 
                                                  nsIDOMNode **node, 
                                                  PRUint32 *offset)
{
  return msiMCaretBase::AdjustNodeAndOffsetFromMouseEvent(editor, presShell, flags, 
                                                          mouseEvent, node, offset);
}                                                       

//TODO
NS_IMETHODIMP 
msiMtableCaret::GetSelectableMathFragment(nsIEditor  *editor, 
                                         nsIDOMNode *start,      PRUint32 startOffset, 
                                         nsIDOMNode *end,        PRUint32 endOffset, 
                                         nsIDOMNode **fragStart, PRUint32 *fragStartOffset,
                                         nsIDOMNode **fragEnd,   PRUint32 *fragEndOffset)
{
  return NS_ERROR_FAILURE;
}

NS_IMETHODIMP
msiMtableCaret::Accept(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  if (!node || !offset || !m_mathmlNode || !editor)
    return NS_ERROR_FAILURE;
  nsresult res(NS_OK);  
  nsCOMPtr<msiIMathMLCaret> mathmlEditing;
  nsCOMPtr<nsIDOMNode> child;
  NS_ASSERTION(flags & FROM_LEFT|FROM_RIGHT, "Accept called without From left or from right");
  if (flags & FROM_RIGHT)
  {
    if (m_offset == m_numKids)
    {
      res = msiUtils::GetChildNode(m_mathmlNode, 0, child);
      NS_ASSERTION(child, "Yuck - child is null");
      if (NS_SUCCEEDED(res) && child)
      {
        msiUtils::GetMathMLCaretInterface(editor, child, RIGHT_MOST, mathmlEditing);
        NS_ASSERTION(mathmlEditing, "Yuck - mathml caret interface is null");
        if (mathmlEditing)
          res = mathmlEditing->Accept(editor, FROM_RIGHT|FROM_PARENT, node, offset);
        else
          res = NS_ERROR_FAILURE;
      }
      else
        res = NS_ERROR_FAILURE;
    }
    else if (!(flags & FROM_CHILD))
    {
      msiUtils::SetupPassOffCaretToParent(editor, m_mathmlNode, PR_FALSE, mathmlEditing);
      if (mathmlEditing)
        res = mathmlEditing->Accept(editor, FROM_CHILD|FROM_RIGHT, node, offset); 
      else 
        res = NS_ERROR_FAILURE;
    }    
  }
  else if (flags & FROM_LEFT)
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
    else if (!(flags & FROM_CHILD))
    {
      msiUtils::SetupPassOffCaretToParent(editor, m_mathmlNode, PR_TRUE, mathmlEditing);
      if (mathmlEditing)
        res = mathmlEditing->Accept(editor, FROM_CHILD|FROM_LEFT, node, offset); 
      else 
        res = NS_ERROR_FAILURE;
    }    
  }
  else if (flags & FROM_ABOVE)
  {
    res = msiUtils::GetChildNode(m_mathmlNode, m_offset, child);
    NS_ASSERTION(child, "Yuck - child is null");
    if (NS_SUCCEEDED(res) && child)
    {
      msiUtils::GetMathMLCaretInterface(editor, child, 0, mathmlEditing);
      NS_ASSERTION(mathmlEditing, "Yuck - mathml caret interface is null");
      if (mathmlEditing)
        res = mathmlEditing->Accept(editor, FROM_ABOVE|FROM_PARENT, node, offset);
      else
        res = NS_ERROR_FAILURE;
    }
    else
      res = NS_ERROR_FAILURE;
  }
  else if (flags & FROM_BELOW)
  {
    res = msiUtils::GetChildNode(m_mathmlNode, m_offset, child);
    NS_ASSERTION(child, "Yuck - child is null");
    if (NS_SUCCEEDED(res) && child)
    {
      msiUtils::GetMathMLCaretInterface(editor, child, 0, mathmlEditing);
      NS_ASSERTION(mathmlEditing, "Yuck - mathml caret interface is null");
      if (mathmlEditing)
        res = mathmlEditing->Accept(editor, FROM_BELOW|FROM_PARENT, node, offset);
      else
        res = NS_ERROR_FAILURE;
    }
    else
      res = NS_ERROR_FAILURE;
  }
  if (*node == nsnull && !(flags & FROM_CHILD))
  {
    nsCOMPtr<nsIDOMNode> child;
    nsCOMPtr<msiIMathMLCaret> mathmlEditing;
    msiUtils::GetChildNode(m_mathmlNode, 0, child);
    if (child)
    {
      PRUint32 pos = m_offset == 0 ? 0 : RIGHT_MOST;
      flags = FROM_PARENT;
      flags |= m_offset == 0 ? FROM_LEFT : FROM_RIGHT;
      msiUtils::GetMathMLCaretInterface(editor, child, pos, mathmlEditing);
      NS_ASSERTION(mathmlEditing, "Yuck - mathml caret interface is null");
      if (mathmlEditing)
        res = mathmlEditing->Accept(editor, flags, node, offset);
    }
  }
  return res;  
}

NS_IMETHODIMP
msiMtableCaret::AdjustSelectionPoint(nsIEditor *editor, PRBool leftSelPoint, 
                                    nsIDOMNode** selectionNode, PRUint32 * selectionOffset,
                                    msiIMathMLCaret** parentCaret)
{
  return msiMCaretBase::AdjustSelectionPoint(editor, leftSelPoint, selectionNode, selectionOffset, parentCaret);
}

NS_IMETHODIMP 
msiMtableCaret::SplitAtDecendents(nsIEditor* editor, 
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
msiMtableCaret::Split(nsIEditor *editor, 
                     nsIDOMNode *appendLeft, 
                     nsIDOMNode *appendRight, 
                     nsIDOMNode **left, 
                     nsIDOMNode **right)
{
  return msiMCaretBase::Split(editor, appendLeft, appendRight, left, right);
} 

NS_IMETHODIMP
msiMtableCaret::SetDeletionTransaction(nsIEditor * editor,
                                      PRBool deletingToTheRight, 
                                      nsITransaction ** txn,
                                      PRBool * toRightInParent)
{
  return msiMCaretBase::SetDeletionTransaction(editor, deletingToTheRight, txn, toRightInParent);
}                                            

//TODO
NS_IMETHODIMP
msiMtableCaret::SetupDeletionTransactions(nsIEditor * editor,
                                         nsIDOMNode * start,
                                         PRUint32 startOffset,
                                         nsIDOMNode * end,
                                         PRUint32 endOffset,
                                         nsIArray ** transactionList,
                                         nsIDOMNode ** coalesceNode,
                                         PRUint32 * coalesceOffset)
{
  return NS_ERROR_FAILURE;  
}

NS_IMETHODIMP
msiMtableCaret::SetupCoalesceTransactions(nsIEditor * editor,
                                         nsIArray ** coalesceTransactions)
{
  return msiMCaretBase::SetupCoalesceTransactions(editor, coalesceTransactions);
}                                               

NS_IMETHODIMP
msiMtableCaret::SetCoalTransactionsAndNode(nsIEditor * editor,
                                         PRBool onLeft,
                                         nsIArray ** transactionList,
                                         nsIDOMNode **coalesceNode)
{
  return msiMCaretBase::SetCoalTransactionsAndNode(editor, onLeft, transactionList, coalesceNode);
}
                                         
NS_IMETHODIMP
msiMtableCaret::CaretLeft(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  if (!node || !offset || !m_mathmlNode || !editor)
    return NS_ERROR_FAILURE;
  nsresult res(NS_OK);  
  nsCOMPtr<msiIMathMLCaret> mathmlEditing;
  if (m_offset < m_numKids)
    res = Accept(editor, FROM_RIGHT, node, offset);
  else
  {
    nsCOMPtr<nsIDOMNode> child;
    msiUtils::GetChildNode(m_mathmlNode, 0, child);  // set caret in child
    NS_ASSERTION(child, "Yuck - child is null");
    if (child)
    {
      PRUint32 pos(RIGHT_MOST);
      msiUtils::GetMathMLCaretInterface(editor, child, pos, mathmlEditing);
      if (mathmlEditing)
      {
        flags = FROM_PARENT | FROM_RIGHT;
        res = mathmlEditing->Accept(editor, flags, node, offset); 
      }
      else
        res = NS_ERROR_FAILURE;
    }  
  }  
  return res;
}

NS_IMETHODIMP
msiMtableCaret::CaretRight(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  if (!node || !offset || !m_mathmlNode || !editor)
    return NS_ERROR_FAILURE;
  nsresult res(NS_OK);  
  nsCOMPtr<msiIMathMLCaret> mathmlEditing;
  if (m_offset > 0)
    res = Accept(editor, FROM_LEFT, node, offset);
  else  //m_offset ==0
  {
    nsCOMPtr<nsIDOMNode> child;
    msiUtils::GetChildNode(m_mathmlNode, 0, child);
    NS_ASSERTION(child, "Yuck - child is null");
    if (child)
    {
      msiUtils::GetMathMLCaretInterface(editor, child, 0, mathmlEditing);
      NS_ASSERTION(mathmlEditing, "Yuck - mathml caret interface is null");
      if (mathmlEditing)
      {
        flags = FROM_PARENT | FROM_LEFT;
        res = mathmlEditing->Accept(editor, flags, node, offset); 
      }
      else
        res = NS_ERROR_FAILURE;
    } 
    else 
      res = NS_ERROR_FAILURE;
  }
  return res;
}

NS_IMETHODIMP
msiMtableCaret::CaretUp(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  if (m_offset > 0)
    --m_offset;
  else
    return msiMCaretBase::CaretUp(editor,flags,node,offset);
  return Accept(editor, FROM_BELOW, node, offset);
}

NS_IMETHODIMP
msiMtableCaret::CaretDown(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  if (m_offset < m_numKids)
    ++m_offset;
  else
    return msiMCaretBase::CaretUp(editor,flags,node,offset);
  return Accept(editor, FROM_ABOVE, node, offset);
}

//TODO
NS_IMETHODIMP
msiMtableCaret::CaretObjectLeft(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  m_offset = 0;
  return Accept(editor, FROM_RIGHT, node, offset);
}

//TODO
NS_IMETHODIMP
msiMtableCaret::CaretObjectRight(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  m_offset = 2;
  return Accept(editor, FROM_LEFT, node, offset);
}

NS_IMETHODIMP
msiMtableCaret::CaretObjectUp(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return CaretLeft(editor, flags, node, offset);
}

NS_IMETHODIMP
msiMtableCaret::CaretObjectDown(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return CaretRight(editor, flags, node, offset);
}
