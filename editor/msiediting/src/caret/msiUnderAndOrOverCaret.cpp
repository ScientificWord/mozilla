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
msiUnderAndOrOverCaret::GetNodeAndOffsetFromMouseEvent(nsIEditor *editor, nsIPresShell *presShell, 
                                                       PRUint32 flags, nsIDOMMouseEvent * mouseEvent,
                                               nsIDOMNode **node, PRUint32 *offset)
{
  if (!editor || !node || !offset || !presShell || !m_mathmlNode || !mouseEvent)
    return NS_ERROR_FAILURE;
  *node = nsnull;
  *offset = INVALID;
  nsIFrame * uoFrame = nsnull; // no smart pointers for frames.
  nsIFrame * baseFrame = nsnull;
  nsIFrame * script1Frame = nsnull;
  nsIFrame * script2Frame = nsnull;
  nsRect uoRect, baseRect, script1Rect, script2Rect;
  nsPoint eventPoint(0,0);;
  *node = nsnull;
  *offset = INVALID;
  nsresult res = msiMCaretBase::GetPrimaryFrameForNode(presShell, m_mathmlNode, &uoFrame);
  if (NS_SUCCEEDED(res))
    res = GetFramesAndRects(presShell, uoFrame, &baseFrame, &script1Frame, &script2Frame, 
                            uoRect, baseRect, script1Rect, script2Rect);
  if (NS_SUCCEEDED(res))
    res = msiUtils::GetPointFromMouseEvent(mouseEvent, eventPoint);                                     
  if (NS_SUCCEEDED(res))
  {
    nsCOMPtr<nsIDOMNode> child;
    PRInt32 above(0), below(0);
    PRBool aboveSet(PR_FALSE), belowSet(PR_FALSE);
    GetAboveBelowThresholds(uoRect, baseRect, script1Rect, script2Rect,
                            above, aboveSet, below, belowSet);
    if (aboveSet && eventPoint.y <= above)
    {
      if (!script2Frame) 
        res =  msiMCaretBase::GetNodeFromFrame(script1Frame, child);
      else
        res =  msiMCaretBase::GetNodeFromFrame(script2Frame, child);
    }
    else if (belowSet && eventPoint.y >= below)
      res =  msiMCaretBase::GetNodeFromFrame(script1Frame, child);
    else
      res =  msiMCaretBase::GetNodeFromFrame(baseFrame, child);
    if (NS_SUCCEEDED(res) && child)
    {
      nsCOMPtr<msiIMathMLCaret> mathCaret;
      PRUint32 pos(0);
      msiUtils::GetMathMLCaretInterface(editor, child, pos, mathCaret);
      NS_ASSERTION(mathCaret, "Yuck - mathml caret interface is null");
      if (mathCaret)
        res = mathCaret->GetNodeAndOffsetFromMouseEvent(editor, presShell, FROM_PARENT,
                                                        mouseEvent, node, offset);
    }
  }  
  if (*node == nsnull)
  {
    NS_ASSERTION(PR_FALSE, "Under/Over failed to set node and offset.");
    m_offset = eventPoint.x <= uoRect.x + (uoRect.width/2) ? 0 : m_numKids;
    flags = m_offset== 0 ? FROM_RIGHT : FROM_LEFT;
    res = Accept(editor, flags, node, offset);
  }  
  
  return res;
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
msiUnderAndOrOverCaret::SetupDeletionTransactions(nsIEditor * editor,
                                                  PRUint32 startOffset,
                                                  PRUint32 endOffset,
                                                  nsIDOMNode * start,
                                                  nsIDOMNode * end,
                                                  nsIArray ** transactionList)
{
  if (!m_mathmlNode || !editor || !transactionList)
    return NS_ERROR_FAILURE;
  nsresult res(NS_OK);
  if ((endOffset - startOffset == 1 && end == nsnull) ||
      (endOffset == startOffset))
  {
    nsCOMPtr<nsIDOMNode> newKid;
    if (startOffset == endOffset)
    {
      if ((startOffset != m_numKids)  && (start || end))
      {
        res = msiRequiredArgument::MakeRequiredArgument(editor, start, end, newKid);
        if (NS_FAILED(res) || !newKid)
          res = NS_ERROR_FAILURE;  
      }    
    }
    else if (start)
    {
      res = msiRequiredArgument::MakeRequiredArgument(editor, start, nsnull, newKid);
      if (NS_FAILED(res) || !newKid)
        res = NS_ERROR_FAILURE;  
    }
    else
    {
      nsCOMPtr<nsIDOMElement> inputboxElement;
      PRUint32 flags(msiIMathMLInsertion::FLAGS_NONE);
      res = msiUtils::CreateInputbox(editor, PR_FALSE, PR_FALSE, flags, inputboxElement);
      if (NS_SUCCEEDED(res) && inputboxElement)
        newKid = do_QueryInterface(inputboxElement);
      if (!newKid)
        res = NS_ERROR_FAILURE;  
    }
    if (NS_SUCCEEDED(res) && newKid)
    {
      nsCOMPtr<msiIMathMLEditor> msiEditor(do_QueryInterface(editor));
      nsCOMPtr<nsIMutableArray> mutableTxnArray = do_CreateInstance(NS_ARRAY_CONTRACTID, &res);
      if (!msiEditor ||!mutableTxnArray)
        res = NS_ERROR_FAILURE;
      if (NS_SUCCEEDED(res))
      {
        nsCOMPtr<nsITransaction> transaction;
        nsCOMPtr<nsIDOMNode> oldKid;
        res = msiUtils::GetChildNode(m_mathmlNode, startOffset, oldKid);
        if (NS_SUCCEEDED(res) && oldKid)
          res = msiEditor->CreateReplaceTransaction(newKid, oldKid, m_mathmlNode, getter_AddRefs(transaction));
        if (NS_SUCCEEDED(res) && transaction)
          res = mutableTxnArray->AppendElement(transaction, PR_FALSE);
        else
          res = NS_ERROR_FAILURE;
      }
      if (NS_SUCCEEDED(res))
      {
        *transactionList = mutableTxnArray;
        NS_ADDREF(*transactionList);
      }  
    }
  }
  else // this should not happen
  {
    NS_ASSERTION(PR_FALSE, "Yucky\n");
    nsCOMPtr<msiIMathMLCaret> parentCaret;
    res = msiUtils::SetupPassOffCaretToParent(editor, m_mathmlNode, PR_FALSE, parentCaret);
    if (NS_SUCCEEDED(res) && parentCaret)
    {
      PRUint32 offset(INVALID);
      res = msiUtils::GetOffsetFromCaretInterface(parentCaret, offset);
      if (NS_SUCCEEDED(res) && offset != INVALID) 
        res = parentCaret->SetupDeletionTransactions(editor, offset, offset+1, 
                                                     nsnull, nsnull, transactionList);
      else
        res = NS_ERROR_FAILURE;
    }
    else
      res = NS_ERROR_FAILURE;
  }
  return res;
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
msiUnderAndOrOverCaret::GetFramesAndRects(nsIPresShell* shell, const nsIFrame * underOver, 
                                 nsIFrame ** base, nsIFrame ** script1, nsIFrame ** script2,
                                 nsRect & uoRect, nsRect &bRect, nsRect& s1Rect, nsRect& s2Rect)
{ // relative to scritp's view
  nsresult res(NS_ERROR_FAILURE);
  nsPresContext * context = nsnull;
  *script2 = nsnull;
  s2Rect= nsRect(0,0,0,0);
  if (underOver && shell)
  {
    *base = underOver->GetFirstChild(nsnull);
    if (*base)
      *script1 = (*base)->GetNextSibling();
    res = *base && *script1 ? NS_OK : NS_ERROR_FAILURE;  
    if (NS_SUCCEEDED(res))
    {
      *script2 = (*script1)->GetNextSibling();
      context = shell->GetPresContext();
      res = context ? NS_OK : NS_ERROR_FAILURE;
    }  
  }
  if (NS_SUCCEEDED(res))
  {
    nsPoint offsetPoint(0,0);
    nsIView * dontcare = nsnull;
    res = underOver->GetOffsetFromView(offsetPoint, &dontcare);
    if (NS_SUCCEEDED(res))
    {
      uoRect = underOver->GetRect();
      uoRect.x = offsetPoint.x;
      uoRect.y = offsetPoint.y;
      bRect = (*base)->GetRect();
      bRect.x += offsetPoint.x;
      bRect.y += offsetPoint.y;
      s1Rect = (*script1)->GetRect();
      s1Rect.x += offsetPoint.x;
      s1Rect.y += offsetPoint.y;
      if (*script2)
      {
        s2Rect = (*script2)->GetRect();
        s2Rect.x += offsetPoint.x;
        s2Rect.y += offsetPoint.y;
      }
    }
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
