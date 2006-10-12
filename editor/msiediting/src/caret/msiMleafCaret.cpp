// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.
#include "msiMleafCaret.h"
#include "msiUtils.h"
#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "nsIDOMText.h"
#include "nsIEditor.h"
#include "nsISelection.h"
#include "nsIFrame.h"
#include "nsIView.h"
#include "nsPresContext.h"
#include "nsIDOMMouseEvent.h"
#include "nsIPrivateDOMEvent.h"
#include "nsHTMLReflowState.h"
#include "nsLayoutUtils.h"
#include "nsITransaction.h"
#include "msiIMathMLEditor.h"
#include "nsIArray.h"
#include "nsIMutableArray.h"
#include "nsComponentManagerUtils.h"
#include "msiEditingAtoms.h"

msiMleafCaret::msiMleafCaret(nsIDOMNode* mathmlNode, PRUint32 offset, PRUint32 mathmlType)
:  msiMCaretBase(mathmlNode, offset, mathmlType), m_isDipole(PR_FALSE), m_length(0)
{
  if (mathmlNode)
  {
    nsCOMPtr<nsIDOMNode> child;
    mathmlNode->GetFirstChild(getter_AddRefs(child));
    if (child)
    {
      nsCOMPtr<nsIDOMText> text(do_QueryInterface(child));
      if (text)
      {
         m_textNode = child;
         nsCOMPtr<nsIDOMCharacterData> characterdata(do_QueryInterface(text));
         if (characterdata)
           characterdata->GetLength(&m_length);
      }
    }
    if (offset == RIGHT_MOST)
      m_offset = m_length;
  }
}

NS_IMETHODIMP
msiMleafCaret::PrepareForCaret(nsIEditor* editor)
{
  return NS_OK;
}

NS_IMETHODIMP
msiMleafCaret::Inquiry(nsIEditor* editor, PRUint32 inquiryID, PRBool *result)
{
  if (inquiryID == AT_RIGHT_EQUIV_TO_0 || inquiryID == AT_LEFT_EQUIV_TO_RIGHT_MOST) 
    *result = PR_TRUE;
  else
    *result = PR_FALSE;
  return NS_OK;
}


NS_IMETHODIMP
msiMleafCaret::GetNodeAndOffsetFromMouseEvent(nsIEditor *editor, nsIPresShell *presShell, 
                                              PRUint32 flags, nsIDOMMouseEvent *mouseEvent,
                                              nsIDOMNode **node, PRUint32 *offset)
{
  if (!editor || !node || !offset || !presShell || !m_mathmlNode || !mouseEvent)
    return NS_ERROR_FAILURE;
  nsresult res(NS_OK);
  nsIFrame * leafFrame = nsnull; // no smart pointers for frames.
  nsIFrame * textFrame = nsnull; // no smart pointers for frames.
  nsRect leafRect, textRect;
  nsPoint eventPoint(0, 0);
  PRInt32 lfGap(0), rtGap(0);
  *node = nsnull;
  *offset = INVALID;
  res = msiMCaretBase::GetPrimaryFrameForNode(presShell, m_mathmlNode, &leafFrame);
  if (NS_SUCCEEDED(res) && leafFrame)
  {
    leafRect = leafFrame->GetScreenRectExternal();
    textFrame = leafFrame->GetFirstChild(nsnull);
    NS_ASSERTION(textFrame && textFrame->GetType() == msiEditingAtoms::textFrame, "child of leaf is not textframe");
    if (textFrame && textFrame->GetType() == msiEditingAtoms::textFrame)
    {
      textRect = textFrame->GetScreenRectExternal();
      lfGap = textRect.x > 0 ? textRect.x : 0;
      rtGap = leafRect.width - textRect.x - textRect.width;
      rtGap = rtGap < 0 ? 0 : rtGap;
    }
    else
      res = NS_ERROR_FAILURE;  
  }
  if (NS_SUCCEEDED(res))
    res = msiUtils::GetScreenPointFromMouseEvent(mouseEvent, eventPoint);                                     
  if (NS_SUCCEEDED(res))
  {
    m_offset = m_length;
    if (m_isDipole)
      m_offset =  (eventPoint.x <= lfGap + textRect.width/2) ? 0 : m_length;
    else
    {
      PRUint32 loc_offset(NS_MAXSIZE);
      res = msiUtils::GetOffsetIntoTextFromEvent(textFrame, mouseEvent, &loc_offset);
      if (NS_SUCCEEDED(res))
        m_offset = loc_offset;
    }
    res = Accept(editor, FLAGS_NONE, node, offset);
  }
  return res;   
}   

NS_IMETHODIMP
msiMleafCaret::AdjustNodeAndOffsetFromMouseEvent(nsIEditor *editor, nsIPresShell *presShell, 
                                                       PRUint32 flags, 
                                                       nsIDOMMouseEvent *mouseEvent, 
                                                       nsIDOMNode **node, 
                                                       PRUint32 *offset)
{
  if (!editor || !node || !offset || !presShell || !m_mathmlNode || !mouseEvent)
    return NS_ERROR_FAILURE;
  nsresult res(NS_OK);
  nsIFrame * leafFrame = nsnull; // no smart pointers for frames.
  nsIFrame * textFrame = nsnull; // no smart pointers for frames.
  nsRect leafRect, textRect;
  nsPoint eventPoint(0, 0);
  PRInt32 lfGap(0), rtGap(0);
  *node = nsnull;
  *offset = INVALID;
  res = msiMCaretBase::GetPrimaryFrameForNode(presShell, m_mathmlNode, &leafFrame);
  if (NS_SUCCEEDED(res) && leafFrame)
  {
    leafRect = leafFrame->GetScreenRectExternal();
    textFrame = leafFrame->GetFirstChild(nsnull);
    NS_ASSERTION(textFrame && textFrame->GetType() == msiEditingAtoms::textFrame, "child of leaf is not textframe");
    if (textFrame && textFrame->GetType() == msiEditingAtoms::textFrame)
    {
      textRect = textFrame->GetScreenRectExternal();
      lfGap = textRect.x - leafRect.x;
      if (lfGap < 0)
      {
        leafRect.x = textRect.x;
        leafRect.width -= lfGap;
        lfGap = 0;
      }
      rtGap = leafRect.x + leafRect.width - textRect.x - textRect.width;
      if (rtGap < 0)
      {
        leafRect.width -= rtGap;
        rtGap = 0;
      }
    }
    else
      res = NS_ERROR_FAILURE;  
  }
  if (NS_SUCCEEDED(res))
    res = msiUtils::GetScreenPointFromMouseEvent(mouseEvent, eventPoint);                                     
  if (NS_SUCCEEDED(res))
  {
    if ( leafRect.x <= eventPoint.x && eventPoint.x <= leafRect.x + leafRect.width)
    {
      if (m_isDipole)
        m_offset =  (eventPoint.x <= lfGap + textRect.x + textRect.width/2) ? 0 : m_length;
      // else  accept system placement
      res = Accept(editor, FLAGS_NONE, node, offset);
    }
    else
    {
      nsCOMPtr<msiIMathMLCaret> mathmlEditing;
      PRBool incOffset = (leafRect.x <= eventPoint.x);
      msiUtils::SetupPassOffCaretToParent(editor, m_mathmlNode, incOffset, mathmlEditing);
      if (mathmlEditing)
      {
        flags = FROM_CHILD;
        flags |= incOffset ? FROM_LEFT : FROM_RIGHT;
        res = mathmlEditing->AdjustNodeAndOffsetFromMouseEvent(editor, presShell, flags, 
                                                               mouseEvent, node, offset);
      } 
    }  
  }
  return res;   
}                                                       


NS_IMETHODIMP
msiMleafCaret::Accept(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  if (!node || !offset || !m_mathmlNode || !editor)
    return NS_ERROR_FAILURE;
  NS_ASSERTION(flags & FROM_LEFT|FROM_RIGHT, "Accept called without From left or from right");
  nsresult res(NS_OK);  
  PRUint32 pos(0);
  if (m_isDipole && (m_offset != 0 || m_offset != m_length))
    pos =  (m_offset <= m_length/2 ) ? 0 : m_length;
  else
    pos = m_offset;
  if (!(flags&FROM_PARENT) && (m_offset == 0 || m_offset == m_length))
  {
    nsCOMPtr<msiIMathMLCaret> mathmlEditing;
    PRBool incOffset = m_offset == m_length;
    msiUtils::SetupPassOffCaretToParent(editor, m_mathmlNode, incOffset, mathmlEditing);
    if (mathmlEditing)
    {
      flags = FROM_CHILD;
      flags |= incOffset ? FROM_LEFT : FROM_RIGHT;
      mathmlEditing->Accept(editor, flags, node, offset); 
    }  
  }
  if (*node == nsnull)
  {
    if (m_textNode)
      *node = m_textNode;
    else
      *node  = m_mathmlNode;
    NS_ADDREF(*node);
    *offset = pos;
  }      
  return res;
}

NS_IMETHODIMP 
msiMleafCaret::GetSelectableMathFragment(nsIEditor  *editor, 
                                         nsIDOMNode *start,      PRUint32 startOffset, 
                                         nsIDOMNode *end,        PRUint32 endOffset, 
                                         nsIDOMNode **fragStart, PRUint32 *fragStartOffset,
                                         nsIDOMNode **fragEnd,   PRUint32 *fragEndOffset)
{
  if (!editor)
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
    if (m_isDipole)
    {  
      startOffset = 0;
      endOffset = m_length;
    }
    endOffset = endOffset > m_length ? m_length : endOffset;
    if (startOffset == 0 && endOffset  == m_length)
    {
      // have parent select the leaf node
      nsCOMPtr<msiIMathMLCaret> parent;
      res = msiUtils::SetupPassOffCaretToParent(editor, m_mathmlNode, PR_FALSE, parent);
      if (NS_SUCCEEDED(res) && parent)
      {
        PRBool parentCanSelectKid(PR_FALSE);
        parent->Inquiry(editor, CAN_SELECT_CHILD_LEAF, &parentCanSelectKid);
        if (parentCanSelectKid)
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
        else
        {
          *fragEnd = m_mathmlNode;
          *fragStart = m_mathmlNode;
          NS_ADDREF(*fragEnd);
          NS_ADDREF(*fragStart);
          *fragStartOffset = 0;
          *fragEndOffset = 1;
        }                                            
      }
    }  
    else if (m_textNode)
    {
      *fragEnd = m_textNode;
      *fragStart = m_textNode;
      NS_ADDREF(*fragEnd);
      NS_ADDREF(*fragStart);
      *fragStartOffset = startOffset;
      *fragEndOffset = endOffset;
    }
    else
      res = NS_ERROR_FAILURE;
  }
  return res;
}

NS_IMETHODIMP
msiMleafCaret::AdjustSelectionPoint(nsIEditor *editor, PRBool leftSelPoint, 
                                       nsIDOMNode** selectionNode, PRUint32 * selectionOffset,
                                       msiIMathMLCaret** parentCaret)
{
  nsresult res(NS_OK);  
  PRUint32 pos(0);
  if (m_isDipole && (m_offset != 0 || m_offset != m_length))
    pos =  (m_offset <= m_length/2 ) ? 0 : m_length;
  else
    pos = m_offset;
  PRBool doAdjust = (pos == 0 || pos == m_length);
  PRBool incOffset(!leftSelPoint);
  if (!leftSelPoint && m_offset == 0)
    incOffset = PR_FALSE;
  else if (leftSelPoint && m_offset == m_length)
    incOffset = PR_TRUE;  
  nsCOMPtr<msiIMathMLCaret> pCaret;
  res = msiUtils::SetupPassOffCaretToParent(editor, m_mathmlNode, incOffset, pCaret);
  if (NS_SUCCEEDED(res) && pCaret)  
  {
    if (doAdjust)
    {
      nsCOMPtr<msiIMathMLEditingBC> parentBC(do_QueryInterface(pCaret));
      if (parentBC)
      {
        parentBC->GetMathmlNode(selectionNode);
        parentBC->GetOffset(selectionOffset);
        res = (*selectionNode && (*selectionOffset != INVALID)) ? NS_OK : NS_ERROR_FAILURE;
      }
      else 
        res = NS_ERROR_FAILURE;
    }
    else if (m_textNode)
    {
      *selectionNode = m_textNode;
      NS_ADDREF(*selectionNode);
      *selectionOffset = m_offset;
    }
    if (NS_SUCCEEDED(res))
    {
      *parentCaret = pCaret;
      NS_ADDREF(*parentCaret);
    }
  }
  return res;
}

NS_IMETHODIMP 
msiMleafCaret::SplitAtDecendents(nsIEditor* editor, 
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
msiMleafCaret::Split(nsIEditor *editor, 
                     nsIDOMNode *appendLeft, 
                     nsIDOMNode *appendRight, 
                     nsIDOMNode **left, 
                     nsIDOMNode **right)
{
  if (!m_mathmlNode || !left || !right)
    return NS_ERROR_NULL_POINTER;
  *left = nsnull;
  *right = nsnull;  
  nsresult res(NS_OK);
  if (m_offset == 0 || (m_isDipole && m_offset <= m_length/2))
    res = m_mathmlNode->CloneNode(PR_TRUE, right);
  else if (m_offset >= m_length || (m_isDipole && m_offset > m_length/2))  
    res = m_mathmlNode->CloneNode(PR_TRUE, left);
  else // 0 < m_offset < m_length && !m_isDipole)
  {
    res = m_mathmlNode->CloneNode(PR_TRUE, left);
    if (NS_SUCCEEDED(res))
      res = m_mathmlNode->CloneNode(PR_TRUE, right);
    if (NS_SUCCEEDED(res) && *left)
    {
      nsCOMPtr<nsIDOMNode> child;
      res = (*left)->GetFirstChild(getter_AddRefs(child));
      if (NS_SUCCEEDED(res) && child)
      {
        nsCOMPtr<nsIDOMCharacterData> charData(do_QueryInterface(child));
        if (charData)
          res = charData->DeleteData(m_offset, m_length-m_offset);
      }
    }  
    if (NS_SUCCEEDED(res) && *right)
    {
      nsCOMPtr<nsIDOMNode> child;
      res = (*right)->GetFirstChild(getter_AddRefs(child));
      if (NS_SUCCEEDED(res) && child)
      {
        nsCOMPtr<nsIDOMCharacterData> charData(do_QueryInterface(child));
        if (charData)
          res = charData->DeleteData(0, m_offset);
      }  
    }
  }
  return res;
}                                     

NS_IMETHODIMP
msiMleafCaret::SetupDeletionTransactions(nsIEditor * editor,
                                         PRUint32 startOffset,
                                         PRUint32 endOffset,
                                         nsIDOMNode * start,
                                         nsIDOMNode * end,
                                         nsIArray ** transactionList)
{

  if (!m_mathmlNode || !editor || !transactionList)
    return NS_ERROR_FAILURE;
      
  nsresult res(NS_OK);
  if (m_isDipole || (startOffset == 0 && endOffset == m_length))
  {
    nsCOMPtr<msiIMathMLCaret> parentCaret;
    res = msiUtils::SetupPassOffCaretToParent(editor, m_mathmlNode, PR_FALSE, parentCaret);
    if (NS_SUCCEEDED(res) && parentCaret)
    {
      PRUint32 offset(INVALID);
      res = msiUtils::GetOffsetFromCaretInterface(parentCaret, offset);
      if (NS_SUCCEEDED(res) && offset  != INVALID)
        res = parentCaret->SetupDeletionTransactions(editor, offset, offset+1, 
                                                     nsnull, nsnull, transactionList);
      else                                               
        res = NS_ERROR_FAILURE;
    }
    else
      res = NS_ERROR_FAILURE;
  }
  else if (startOffset < endOffset)
  {
    nsCOMPtr<nsIDOMNode> first, clone;
    res = m_mathmlNode->GetFirstChild(getter_AddRefs(first));
    if (NS_FAILED(res) || !first)
      res = NS_ERROR_FAILURE;
    if (NS_SUCCEEDED(res))
      res = msiUtils::CloneNode(first, clone);
    if (NS_FAILED(res) || !clone)
      res = NS_ERROR_FAILURE;
    if (NS_SUCCEEDED(res))
    {
      nsCOMPtr<nsIDOMCharacterData> charData(do_QueryInterface(clone));
      if (charData)
        res = charData->DeleteData(startOffset, endOffset-startOffset);
      else
        res = NS_ERROR_FAILURE;
      if (NS_SUCCEEDED(res))
      {
        nsCOMPtr<nsIMutableArray> mutableTxnArray = do_CreateInstance(NS_ARRAY_CONTRACTID, &res);
        nsCOMPtr<msiIMathMLEditor> msiEditor(do_QueryInterface(editor));
        if (!msiEditor || !mutableTxnArray)
          return NS_ERROR_FAILURE;
        nsCOMPtr<nsITransaction> transaction;
        res = msiEditor->CreateReplaceTransaction(clone, first, m_mathmlNode, getter_AddRefs(transaction));
        if (NS_SUCCEEDED(res) && transaction)
          res = mutableTxnArray->AppendElement(transaction, PR_FALSE);
        else
          res = NS_ERROR_FAILURE;
        if (NS_SUCCEEDED(res))
        {
          *transactionList = mutableTxnArray;
          NS_ADDREF(*transactionList);
        }  
      }   
    }
  }  
  return res;
}


NS_IMETHODIMP
msiMleafCaret::CaretLeft(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  if (!node || !offset || !m_mathmlNode || !editor)
    return NS_ERROR_FAILURE;
  nsresult res(NS_OK);
  if (m_offset > 0) 
  {
    m_offset = m_isDipole ? 0 : m_offset-1;
    Accept(editor, FROM_RIGHT, node, offset);
  }
  else //m_offset == 0
  {
    nsCOMPtr<msiIMathMLCaret> mathmlEditing;
    flags = FROM_CHILD;
    msiUtils::SetupPassOffCaretToParent(editor, m_mathmlNode, PR_FALSE, mathmlEditing);
    NS_ASSERTION(mathmlEditing, "Parent's mathmlEditing interface is null");
    if (mathmlEditing)
      res = mathmlEditing->CaretLeft(editor, flags, node, offset);
  }
  return res;   
}

NS_IMETHODIMP
msiMleafCaret::CaretRight(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  if (!node || !offset || !m_mathmlNode || !editor)
    return NS_ERROR_FAILURE;
  nsresult res(NS_OK);
  if (m_offset < m_length) 
  {
    m_offset = m_isDipole ? m_length : m_offset+1;
    Accept(editor, FROM_LEFT, node, offset);
  }
  else  // m_offset == m_length
  {
    nsCOMPtr<msiIMathMLCaret> mathmlEditing;
    flags = FROM_CHILD;
    msiUtils::SetupPassOffCaretToParent(editor, m_mathmlNode, PR_TRUE, mathmlEditing);
    NS_ASSERTION(mathmlEditing, "Parent's mathmlEditing interface is null");
    if (mathmlEditing)
      res = mathmlEditing->CaretRight(editor, flags, node, offset);
  }
  return res;
}

NS_IMETHODIMP
msiMleafCaret::CaretUp(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return CaretLeft(editor, flags, node, offset);
}

NS_IMETHODIMP
msiMleafCaret::CaretDown(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return CaretRight(editor, flags, node, offset);
}

NS_IMETHODIMP
msiMleafCaret::CaretObjectLeft(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  if (!node || !offset || !m_mathmlNode || !editor)
    return NS_ERROR_FAILURE;
  nsresult res(NS_OK);
  if (m_offset > 0) 
  {
    m_offset = 0;
    Accept(editor, FROM_RIGHT, node, offset);
  }
  else //m_offset == 0
  {
    nsCOMPtr<msiIMathMLCaret> mathmlEditing;
    flags = FROM_CHILD;
    msiUtils::SetupPassOffCaretToParent(editor, m_mathmlNode, PR_FALSE, mathmlEditing);
    NS_ASSERTION(mathmlEditing, "Parent's mathmlEditing interface is null");
    if (mathmlEditing)
      res = mathmlEditing->CaretObjectLeft(editor, flags, node, offset);
  }
  return res;   
}

NS_IMETHODIMP
msiMleafCaret::CaretObjectRight(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  if (!node || !offset || !m_mathmlNode || !editor)
    return NS_ERROR_FAILURE;
  nsresult res(NS_OK);
  if (m_offset < m_length) 
  {
    m_offset = m_length;
    Accept(editor, FROM_LEFT, node, offset);
  }
  else  // m_offset == m_length
  {
    nsCOMPtr<msiIMathMLCaret> mathmlEditing;
    flags = FROM_CHILD;
    msiUtils::SetupPassOffCaretToParent(editor, m_mathmlNode, PR_TRUE, mathmlEditing);
    NS_ASSERTION(mathmlEditing, "Parent's mathmlEditing interface is null");
    if (mathmlEditing)
      res = mathmlEditing->CaretObjectRight(editor, flags, node, offset);
  }
  return res;
}

NS_IMETHODIMP
msiMleafCaret::CaretObjectUp(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return CaretLeft(editor, flags, node, offset);
}

NS_IMETHODIMP
msiMleafCaret::CaretObjectDown(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return CaretRight(editor, flags, node, offset);
}

//private
nsresult
msiMleafCaret::doSetCaretPosition(nsIEditor * editor,
                                  nsISelection * selection,
                                  PRUint32 position)
{
  nsresult res(NS_ERROR_FAILURE);
  if (selection && m_mathmlNode)
  {
     nsCOMPtr<nsIDOMNode> child;
     msiUtils::GetChildNode(m_mathmlNode, 0, child);
     if (child)
     {
       nsCOMPtr<nsIDOMText> text(do_QueryInterface(child));
       NS_ASSERTION(text, "Child node of math leaf is not a text node");
       if (text)
       {
         PRUint32 rightPos(0);
         res = msiUtils::GetRightMostCaretPosition(editor, child, rightPos);
         if (NS_SUCCEEDED(res) && position > rightPos)
            position = rightPos;
         else
           res = NS_ERROR_FAILURE;   
       }
       res = msiUtils::doSetCaretPosition(selection, child, position);
     } 
  }
  return res;
}                                  
