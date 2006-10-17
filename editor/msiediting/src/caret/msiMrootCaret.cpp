// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiMrootCaret.h"
#include "nsIDOMNode.h"
#include "msiUtils.h"
#include "nsCOMPtr.h"
#include "nsIEditor.h"
#include "nsISelection.h"
#include "nsIFrame.h"
#include "nsIView.h"
#include "nsPresContext.h"
#include "nsIDOMMouseEvent.h"
#include "nsIPrivateDOMEvent.h"
#include "msiRequiredArgument.h"
#include "msiMfracCaret.h"

msiMrootCaret::msiMrootCaret(nsIDOMNode* mathmlNode, PRUint32 offset)
:  msiMCaretBase(mathmlNode, offset, MATHML_MROOT)
{
}

NS_IMETHODIMP
msiMrootCaret::PrepareForCaret(nsIEditor* editor)
{
  nsresult res(NS_ERROR_FAILURE);
  if (m_mathmlNode)
  {
    res = NS_OK;
    nsCOMPtr<nsIDOMNode> child;
    res = m_mathmlNode->GetFirstChild(getter_AddRefs(child));
    if (NS_SUCCEEDED(res) &&child && msiRequiredArgument::NestRequiredArgumentInMrow(child))
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
          res =mrowNode->AppendChild(kid, getter_AddRefs(dontcare));
      }  
    }
    if (NS_SUCCEEDED(res))
    {
      child = nsnull;
      m_mathmlNode->GetLastChild(getter_AddRefs(child));
      if (NS_SUCCEEDED(res) &&child && msiRequiredArgument::NestRequiredArgumentInMrow(child))
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
            res =mrowNode->AppendChild(kid, getter_AddRefs(dontcare));
        }  
      }
    }  
  }
  return res;
}

NS_IMETHODIMP
msiMrootCaret::Inquiry(nsIEditor* editor, PRUint32 inquiryID, PRBool *result)
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
msiMrootCaret::AdjustNodeAndOffsetFromMouseEvent(nsIEditor *editor, nsIPresShell *presShell,
                                                       PRUint32 flags, 
                                                       nsIDOMMouseEvent *mouseEvent, 
                                                       nsIDOMNode **node, 
                                                       PRUint32 *offset)
{
  return msiMCaretBase::AdjustNodeAndOffsetFromMouseEvent(editor, presShell, flags, 
                                                          mouseEvent, node, offset);
}                                                       


                                        
NS_IMETHODIMP
msiMrootCaret::Accept(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  if (!node || !offset || !m_mathmlNode || !editor)
    return NS_ERROR_FAILURE;
  nsresult res(NS_OK);  
  nsCOMPtr<msiIMathMLCaret> mathmlEditing;
  nsCOMPtr<nsIDOMNode> child;
  NS_ASSERTION(flags & FROM_LEFT|FROM_RIGHT, "Accept called without From left or from right");
  if (flags & FROM_RIGHT)
  {
    if (m_offset == 2)
    {
      res = msiUtils::GetChildNode(m_mathmlNode, 0, child);
      NS_ASSERTION(child, "Yuck - child is null");
      if (NS_SUCCEEDED(res) && child)
      {
        res = msiUtils::GetMathMLCaretInterface(editor, child, RIGHT_MOST, mathmlEditing);
        if (NS_SUCCEEDED(res) && mathmlEditing)
          res = mathmlEditing->Accept(editor, FROM_RIGHT|FROM_PARENT, node, offset);
        else
          res = NS_ERROR_FAILURE;
      }
      else
        res = NS_ERROR_FAILURE;
    }
    else if (!(flags & FROM_CHILD) && m_offset == 1)
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
      res = msiUtils::GetChildNode(m_mathmlNode, 1, child);
      NS_ASSERTION(child, "Yuck - child is null");
      if (NS_SUCCEEDED(res) && child)
      {
        res = msiUtils::GetMathMLCaretInterface(editor, child, 0, mathmlEditing);
        if (NS_SUCCEEDED(res) && mathmlEditing)
          res = mathmlEditing->Accept(editor, FROM_LEFT|FROM_PARENT, node, offset);
        else
          res = NS_ERROR_FAILURE;
      }
      else
        res = NS_ERROR_FAILURE;
    }
    else if (!(flags & FROM_CHILD) && m_offset == 1)
    {
      msiUtils::SetupPassOffCaretToParent(editor, m_mathmlNode, PR_TRUE, mathmlEditing);
      if (mathmlEditing)
        res = mathmlEditing->Accept(editor, FROM_CHILD|FROM_LEFT, node, offset); 
      else 
        res = NS_ERROR_FAILURE;
    }    
  }
  if (*node == nsnull && !(flags & FROM_CHILD))
  {
    PRUint32 index = m_offset == 2 ? 1 : 0;
    msiUtils::GetChildNode(m_mathmlNode, index, child);
    if (child)
    {
      PRUint32 pos = m_offset == 0 ? 0 : RIGHT_MOST;
      flags = FROM_PARENT;
      flags |= m_offset == 0 ? FROM_LEFT : FROM_RIGHT;
      msiUtils::GetMathMLCaretInterface(editor, child, pos, mathmlEditing);
      if (NS_SUCCEEDED(res) && mathmlEditing)
        res = mathmlEditing->Accept(editor, flags, node, offset);
      else
        res = NS_ERROR_FAILURE;
    }
  }
  return res;  
}

NS_IMETHODIMP 
msiMrootCaret::GetSelectableMathFragment(nsIEditor  *editor, 
                                         nsIDOMNode *start,      PRUint32 startOffset, 
                                         nsIDOMNode *end,        PRUint32 endOffset, 
                                         nsIDOMNode **fragStart, PRUint32 *fragStartOffset,
                                         nsIDOMNode **fragEnd,   PRUint32 *fragEndOffset)
{
  //ljh -- code below assumes the root is the "common ancestor" of start and end.
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
    PRInt32 startCompare(0), endCompare(0);
    res = msiUtils::ComparePoints(editor, start, startOffset, m_mathmlNode, 1, startCompare);
    if (NS_SUCCEEDED(res))
      res = msiUtils::ComparePoints(editor, end, endOffset, m_mathmlNode, 1, endCompare);
    if (startCompare == -1 && endCompare <= 0) // select base
    {
      *fragStart = m_mathmlNode;
      *fragEnd = m_mathmlNode;
      NS_ADDREF(*fragStart);
      NS_ADDREF(*fragEnd);
      *fragStartOffset = 0;
      *fragEndOffset = 1;
      res = NS_OK;
    }
    else if (startCompare >= 0 && endCompare == 1) // select index
    {
      *fragStart = m_mathmlNode;
      *fragEnd = m_mathmlNode;
      NS_ADDREF(*fragStart);
      NS_ADDREF(*fragEnd);
      *fragStartOffset = 1;
      *fragEndOffset = 2;
      res = NS_OK;
    }
    else // (startCompare == -1 && endCompare == 1) have parent select the root
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
  }
  return res;
}

NS_IMETHODIMP 
msiMrootCaret::SplitAtDecendents(nsIEditor* editor, 
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
msiMrootCaret::Split(nsIEditor *editor, 
                     nsIDOMNode *appendLeft, 
                     nsIDOMNode *appendRight, 
                     nsIDOMNode **left, 
                     nsIDOMNode **right)
{
  return msiMCaretBase::Split(editor, appendLeft, appendRight, left, right);
}                                     

NS_IMETHODIMP
msiMrootCaret::SetupDeletionTransactions(nsIEditor * editor,
                                         PRUint32 startOffset,
                                         PRUint32 endOffset,
                                         nsIDOMNode * start,
                                         nsIDOMNode * end,
                                         nsIArray ** transactionList)
{
  if (!m_mathmlNode || !editor || !transactionList)
    return NS_ERROR_FAILURE;
  if (!(IS_VALID_NODE_OFFSET(startOffset)) || !(IS_VALID_NODE_OFFSET(endOffset)))
    return NS_ERROR_FAILURE;
  return msiMfracCaret::SetupDelTxnForFracOrRoot(editor, m_mathmlNode,
                                                 startOffset, endOffset,
                                                 start,  end,
                                                 transactionList);  
}


NS_IMETHODIMP
msiMrootCaret::CaretLeft(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  if (!node || !offset || !m_mathmlNode || !editor)
    return NS_ERROR_FAILURE;
  nsresult res(NS_OK);  
  nsCOMPtr<msiIMathMLCaret> mathmlEditing;
  if ( m_offset == 2 || m_offset == 0)
  {
    nsCOMPtr<nsIDOMNode> child;
    PRUint32 index = m_offset == 2 ? 0 : 1;
    res = msiUtils::GetChildNode(m_mathmlNode, index, child);
    NS_ASSERTION(child, "Yuck - child is null");
    if (NS_SUCCEEDED(res) && child)
    {
      res = msiUtils::GetMathMLCaretInterface(editor, child, RIGHT_MOST, mathmlEditing);
      if (NS_SUCCEEDED(res) && mathmlEditing)
        res = mathmlEditing->Accept(editor, FROM_RIGHT|FROM_PARENT, node, offset);
      else
        res = NS_ERROR_FAILURE;
    }
    else
      res = NS_ERROR_FAILURE;
  }
  else if (m_offset == 1)
    res = Accept(editor, FROM_RIGHT, node, offset);
  return res;
}

NS_IMETHODIMP
msiMrootCaret::CaretRight(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  if (!node || !offset || !m_mathmlNode || !editor)
    return NS_ERROR_FAILURE;
  nsresult res(NS_OK);  
  nsCOMPtr<msiIMathMLCaret> mathmlEditing;
  if ( m_offset == 0 || m_offset == 2)
  {
    nsCOMPtr<nsIDOMNode> child;
    PRUint32 index = m_offset == 2 ? 0 : 1;
    res = msiUtils::GetChildNode(m_mathmlNode, index, child);
    NS_ASSERTION(child, "Yuck - child is null");
    if (NS_SUCCEEDED(res) && child)
    {
      res = msiUtils::GetMathMLCaretInterface(editor, child, 0, mathmlEditing);
      if (NS_SUCCEEDED(res) && mathmlEditing)
        res = mathmlEditing->Accept(editor, FROM_LEFT|FROM_PARENT, node, offset);
      else
        res = NS_ERROR_FAILURE;
    }
    else
      res = NS_ERROR_FAILURE;
  }
  else if (m_offset == 1)
    res = Accept(editor, FROM_LEFT, node, offset);
  return res;  
}

NS_IMETHODIMP
msiMrootCaret::CaretUp(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return CaretLeft(editor, flags, node, offset);
}

NS_IMETHODIMP
msiMrootCaret::CaretDown(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return CaretRight(editor, flags, node, offset);
}

NS_IMETHODIMP
msiMrootCaret::CaretObjectLeft(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  m_offset = 0;
  return Accept(editor, FROM_RIGHT, node, offset);
}

NS_IMETHODIMP
msiMrootCaret::CaretObjectRight(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  m_offset = 2;
  return Accept(editor, FROM_LEFT, node, offset);
}

NS_IMETHODIMP
msiMrootCaret::CaretObjectUp(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return CaretLeft(editor, flags, node, offset);
}

NS_IMETHODIMP
msiMrootCaret::CaretObjectDown(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return CaretRight(editor, flags, node, offset);
}

NS_IMETHODIMP
msiMrootCaret::AdjustSelectionPoint(nsIEditor *editor, PRBool leftSelPoint, 
                                    nsIDOMNode** selectionNode, PRUint32 * selectionOffset,
                                    msiIMathMLCaret** parentCaret)
{
  return msiMCaretBase::AdjustSelectionPoint(editor, leftSelPoint, selectionNode, selectionOffset, parentCaret);
}


 