// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiMfracCaret.h"
#include "msiUtils.h"
#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "nsIEditor.h"
#include "nsISelection.h"
#include "nsIFrame.h"
#include "nsIView.h"
#include "nsPresContext.h"
#include "msiRequiredArgument.h"
#include "msiIMathMLEditor.h"
#include "nsITransaction.h"
#include "nsIArray.h"
#include "nsIMutableArray.h"
#include "nsComponentManagerUtils.h"

msiMfracCaret::msiMfracCaret(nsIDOMNode* mathmlNode, PRUint32 offset)
:  msiMCaretBase(mathmlNode, offset, MATHML_MFRAC)
{
}

NS_IMETHODIMP
msiMfracCaret::PrepareForCaret(nsIEditor* editor)
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
msiMfracCaret::Inquiry(nsIEditor* editor, PRUint32 inquiryID, PRBool *result)
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
msiMfracCaret::AdjustNodeAndOffsetFromMouseEvent(nsIEditor *editor, nsIPresShell *presShell,
                                                       PRUint32 flags, 
                                                       nsIDOMMouseEvent *mouseEvent, 
                                                       nsIDOMNode **node, 
                                                       PRUint32 *offset)
{
  return msiMCaretBase::AdjustNodeAndOffsetFromMouseEvent(editor, presShell, flags, 
                                                          mouseEvent, node, offset);
}                                                       


NS_IMETHODIMP 
msiMfracCaret::GetSelectableMathFragment(nsIEditor  *editor, 
                                         nsIDOMNode *start,      PRUint32 startOffset, 
                                         nsIDOMNode *end,        PRUint32 endOffset, 
                                         nsIDOMNode **fragStart, PRUint32 *fragStartOffset,
                                         nsIDOMNode **fragEnd,   PRUint32 *fragEndOffset)
{
  //ljh -- code below assumes the fraction is the "common ancestor" of start and end.
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
    if (startCompare == -1 && endCompare <= 0) // select numerator
    {
      *fragStart = m_mathmlNode;
      *fragEnd = m_mathmlNode;
      NS_ADDREF(*fragStart);
      NS_ADDREF(*fragEnd);
      *fragStartOffset = 0;
      *fragEndOffset = 1;
      res = NS_OK;
    }
    else if (startCompare >= 0 && endCompare == 1) // select denominator
    {
      *fragStart = m_mathmlNode;
      *fragEnd = m_mathmlNode;
      NS_ADDREF(*fragStart);
      NS_ADDREF(*fragEnd);
      *fragStartOffset = 1;
      *fragEndOffset = 2;
      res = NS_OK;
    }
    else // (startCompare == -1 && endCompare == 1) have parent select the fraction
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
msiMfracCaret::Accept(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
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
    if (m_offset == 1)
    {
      res = msiUtils::GetChildNode(m_mathmlNode, 1, child);
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
    else
      res = NS_ERROR_FAILURE;
  }
  else if (flags & FROM_BELOW)
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
          res = mathmlEditing->Accept(editor, FROM_ABOVE|FROM_PARENT, node, offset);
        else
          res = NS_ERROR_FAILURE;
      }
      else
        res = NS_ERROR_FAILURE;
    }    
    else
      res = NS_ERROR_FAILURE;
  }
  if (*node == nsnull && !(flags & FROM_CHILD) && NS_SUCCEEDED(res))
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
msiMfracCaret::AdjustSelectionPoint(nsIEditor *editor, PRBool leftSelPoint, 
                                    nsIDOMNode** selectionNode, PRUint32 * selectionOffset,
                                    msiIMathMLCaret** parentCaret)
{
  return msiMCaretBase::AdjustSelectionPoint(editor, leftSelPoint, selectionNode, selectionOffset, parentCaret);
}

NS_IMETHODIMP 
msiMfracCaret::SplitAtDecendents(nsIEditor* editor, 
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
msiMfracCaret::Split(nsIEditor *editor, 
                     nsIDOMNode *appendLeft, 
                     nsIDOMNode *appendRight, 
                     nsIDOMNode **left, 
                     nsIDOMNode **right)
{
  return msiMCaretBase::Split(editor, appendLeft, appendRight, left, right);
} 

NS_IMETHODIMP
msiMfracCaret::SetDeletionTransaction(nsIEditor * editor,
                                      PRBool deletingToTheRight, 
                                      nsITransaction ** txn,
                                      PRBool * toRightInParent)
{
  return msiMCaretBase::SetDeletionTransaction(editor, deletingToTheRight, txn, toRightInParent);
}                                            

NS_IMETHODIMP
msiMfracCaret::SetupDeletionTransactions(nsIEditor * editor,
                                         nsIDOMNode * start,
                                         PRUint32 startOffset,
                                         nsIDOMNode * end,
                                         PRUint32 endOffset,
                                         nsIArray ** transactionList,
                                         nsIDOMNode ** coalesceNode,
                                         PRUint32 * coalesceOffset)
{
  return msiMCaretBase::FracRootSetupDelTxns(editor, m_mathmlNode, start, startOffset, 
                                             end,  endOffset, transactionList,
                                             coalesceNode, coalesceOffset);  
}


NS_IMETHODIMP
msiMfracCaret::SetupCoalesceTransactions(nsIEditor * editor,
                                         nsIArray ** coalesceTransactions)
{
  return msiMCaretBase::SetupCoalesceTransactions(editor, coalesceTransactions);
}                                               


NS_IMETHODIMP
msiMfracCaret::SetCoalTransactionsAndNode(nsIEditor * editor,
                                         PRBool onLeft,
                                         nsIArray ** transactionList,
                                         nsIDOMNode **coalesceNode)
{
  return msiMCaretBase::SetCoalTransactionsAndNode(editor, onLeft, transactionList, coalesceNode);
}
                                         
NS_IMETHODIMP
msiMfracCaret::CaretLeft(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  if (!node || !offset || !m_mathmlNode || !editor)
    return NS_ERROR_FAILURE;
  nsresult res(NS_OK);  
  nsCOMPtr<msiIMathMLCaret> mathmlEditing;
  if (m_offset == 0 || m_offset == 1)
    res = Accept(editor, FROM_RIGHT, node, offset);
  else // m_offset == 2
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
msiMfracCaret::CaretRight(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  if (!node || !offset || !m_mathmlNode || !editor)
    return NS_ERROR_FAILURE;
  nsresult res(NS_OK);  
  nsCOMPtr<msiIMathMLCaret> mathmlEditing;
  if (m_offset == 1  || m_offset == 2)
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
msiMfracCaret::CaretUp(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  if (m_offset > 0)
    m_offset = 0;
  else
    return msiMCaretBase::CaretUp(editor,flags,node,offset);
  return Accept(editor, FROM_BELOW, node, offset);
}

NS_IMETHODIMP
msiMfracCaret::CaretDown(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  if (m_offset == 0)
    m_offset = 1;
  else
    return msiMCaretBase::CaretUp(editor,flags,node,offset);
  return Accept(editor, FROM_ABOVE, node, offset);
}

NS_IMETHODIMP
msiMfracCaret::CaretObjectLeft(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  m_offset = 0;
  return Accept(editor, FROM_RIGHT, node, offset);
}

NS_IMETHODIMP
msiMfracCaret::CaretObjectRight(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  m_offset = 2;
  return Accept(editor, FROM_LEFT, node, offset);
}

NS_IMETHODIMP
msiMfracCaret::CaretObjectUp(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return CaretLeft(editor, flags, node, offset);
}

NS_IMETHODIMP
msiMfracCaret::CaretObjectDown(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return CaretRight(editor, flags, node, offset);
}

NS_IMETHODIMP
msiMfracCaret::TabLeft(nsIEditor *editor, nsIDOMNode ** node, PRUint32 *offset)
{
  return TabRight(editor, node, offset);  // only two tab positions, so direction irrelevant
}

NS_IMETHODIMP
msiMfracCaret::TabRight(nsIEditor *editor, nsIDOMNode ** node, PRUint32 *offset)
{
  PRUint32 newflags;
  if (m_offset == 1)
  {
    newflags = FROM_ABOVE;
  }
  else
  {
    m_offset = 0;
    newflags = FROM_BELOW;
  }
  return Accept(editor, newflags, node, offset);
}

//protected
nsresult
msiMfracCaret::GetFramesAndRects(const nsIFrame * frac, 
                                 nsIFrame ** numer, nsIFrame ** denom,
                                 nsRect & fRect, nsRect &nRect, nsRect& dRect)
{ // relative to frac's view
  nsresult res(NS_ERROR_FAILURE);
  if (frac)
  {
    *numer = frac->GetFirstChild(nsnull);
    if (*numer)
      *denom = (*numer)->GetNextSibling();
    res = *numer && *denom ? NS_OK : NS_ERROR_FAILURE;  
  }
  if (NS_SUCCEEDED(res))
  {
    fRect = frac->GetScreenRectExternal();
    nRect = (*numer)->GetScreenRectExternal();
    dRect = (*denom)->GetScreenRectExternal();
  }
  return res;
}  
      
#define MIN_THRESHOLD 2 //TODO -- how should this be determined.      
void msiMfracCaret::GetThresholds(const nsRect &fRect, 
                                 const nsRect &nRect, const nsRect& dRect,
                                 PRInt32 &left, PRInt32 & right)
{
  PRInt32 minGap(MIN_THRESHOLD);
  minGap = minGap > fRect.width/50 ? minGap : fRect.width/50;
  left = nRect.x < dRect.x ? nRect.x : dRect.x;
  if (left < fRect.x + minGap)
    left = fRect.x + minGap;
  right = nRect.x+nRect.width;
  if (dRect.x + dRect.width > right)
    right = dRect.x + dRect.width;
  if ( right > fRect.x + fRect.width - minGap)  
    right = fRect.x + fRect.width - minGap;
}
