// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiMCaretBase.h"
#include "msiIMrowEditing.h"
#include "msiUtils.h"
#include "msiCoalesceUtils.h"
#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "nsIDOMNodeList.h"
#include "nsITextContent.h"
#include "nsIEditor.h"
#include "nsISelection.h"
#include "nsIFrame.h"
#include "nsIContent.h"
#include "nsIView.h"
#include "nsPresContext.h"
#include "nsIDOMMouseEvent.h"
#include "nsIPrivateDOMEvent.h"
#include "msiIMathMLEditor.h"
#include "msiIMathMLCoalesce.h"
#include "nsITransaction.h"
#include "nsISimpleEnumerator.h"
#include "nsIArray.h"
#include "nsIMutableArray.h"
#include "nsComponentManagerUtils.h"


msiMCaretBase::msiMCaretBase(nsIDOMNode* mathmlNode, 
                             PRUint32 offset,
                             PRUint32 mathmlType) 
: msiMEditingBase(mathmlNode, offset, mathmlType)
{
}
  
msiMCaretBase::~msiMCaretBase()
{
}

NS_IMPL_ISUPPORTS_INHERITED1(msiMCaretBase, msiMEditingBase, msiIMathMLCaret)


NS_IMETHODIMP
msiMCaretBase::PrepareForCaret(nsIEditor* editor)
{
  return NS_OK;
}

NS_IMETHODIMP
msiMCaretBase::Inquiry(nsIEditor* editor, PRUint32 inquiryID, PRBool *result)
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
msiMCaretBase::AdjustNodeAndOffsetFromMouseEvent(nsIEditor *editor, nsIPresShell *presShell,
                                                       PRUint32 flags, 
                                                       nsIDOMMouseEvent *mouseEvent, 
                                                       nsIDOMNode **node, 
                                                       PRUint32 *offset)
{
  if (!editor || !node || !offset || !presShell || !m_mathmlNode || !mouseEvent)
    return NS_ERROR_FAILURE;
  nsresult res(NS_OK);
  nsIFrame * baseFrame = nsnull; // no smart pointers for frames.
  nsRect baseRect;
  nsPoint eventPoint(0,0);
  *node = nsnull;
  *offset = INVALID;
  res = msiMCaretBase::GetPrimaryFrameForNode(presShell, m_mathmlNode, &baseFrame);
  if (NS_SUCCEEDED(res) && baseFrame)
  {
     baseRect = baseFrame->GetScreenRectExternal();
    res = msiUtils::GetScreenPointFromMouseEvent(mouseEvent, eventPoint);                                     
  }
  else
    res = NS_ERROR_FAILURE;
  if (NS_SUCCEEDED(res))
  {
    if ( baseRect.x <= eventPoint.x && eventPoint.x <= baseRect.x + baseRect.width)
    {
      PRUint32 acceptFlags = flags & FROM_RIGHT ? FROM_RIGHT : FROM_LEFT;
      res = Accept(editor, acceptFlags, node, offset);
    }
    else
    {
      nsCOMPtr<msiIMathMLCaret> mathmlEditing;
      PRBool incOffset = (baseRect.x <= eventPoint.x);
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
msiMCaretBase::GetSelectableMathFragment(nsIEditor  *editor, 
                                         nsIDOMNode *start,      PRUint32 startOffset, 
                                         nsIDOMNode *end,        PRUint32 endOffset, 
                                         nsIDOMNode **fragStart, PRUint32 *fragStartOffset,
                                         nsIDOMNode **fragEnd,   PRUint32 *fragEndOffset)
{
  nsCOMPtr<nsIDOMNode> selStart, selEnd;
  PRUint32 selStartOffset(INVALID), selEndOffset(INVALID);
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
  
  nsresult res = msiMCaretBase::GetSelectionPoint(editor, PR_TRUE, start, startOffset, selStart, selStartOffset);
  if (NS_SUCCEEDED(res))
    res = msiMCaretBase::GetSelectionPoint(editor, PR_FALSE, end, endOffset, selEnd, selEndOffset);
  if (NS_SUCCEEDED(res) && selStart && selEnd && 
       selStartOffset <= LAST_VALID && selEndOffset <= LAST_VALID)
  {
    *fragStart = selStart;
    *fragEnd = selEnd;
    NS_ADDREF(*fragStart);
    NS_ADDREF(*fragEnd);
    *fragStartOffset = selStartOffset;
    *fragEndOffset = selEndOffset;
  }
  return res;
}


NS_IMETHODIMP
msiMCaretBase::Accept(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  if (!node || !offset || !m_mathmlNode || !editor)
    return NS_ERROR_FAILURE;
  nsCOMPtr<nsIDOMNode> child;
  nsresult res(NS_OK);  
  NS_ASSERTION(flags & (FROM_LEFT|FROM_RIGHT), "Accept called without From left or from right");
  if (m_numKids == 1)
  {
    msiUtils::GetChildNode(m_mathmlNode, 0, child);
    NS_ASSERTION(child, "Null child node");
    if (child && msiUtils::IsInputbox(editor, child))
    {
       nsCOMPtr<msiIMathMLCaret> mathmlEditing;
       res = msiUtils::GetMathMLCaretInterface(editor, child, 1, mathmlEditing);
       NS_ASSERTION(mathmlEditing, "Null mathml caret interface");
       if (NS_SUCCEEDED(res) && mathmlEditing)
         res = mathmlEditing->Accept(editor, FROM_PARENT|FROM_LEFT, node, offset);
       else
         res = NS_ERROR_FAILURE;  
    }
  }
  if (*node == nsnull)
  {
    if (((flags & FROM_RIGHT) && m_offset > 0) ||  ((flags & FROM_LEFT) && m_offset < m_numKids))
    {
      PRUint32 index = flags & FROM_RIGHT ? m_offset-1 : m_offset;
      msiUtils::GetChildNode(m_mathmlNode, index, child);
      NS_ASSERTION(child, "Null child node");
      if (child)
      {
        PRUint32 newPos = flags & FROM_RIGHT ? RIGHT_MOST : 0;
        nsCOMPtr<msiIMathMLCaret> mathmlEditing;
        msiUtils::GetMathMLCaretInterface(editor, child, newPos, mathmlEditing);
        NS_ASSERTION(mathmlEditing, "Null mathmlEditing interface");
        if (mathmlEditing && msiUtils::IsMrow(mathmlEditing))
        {
          nsCOMPtr<msiIMrowEditing> mrowEditing(do_QueryInterface(mathmlEditing));
          NS_ASSERTION(mrowEditing, "Null mrowEditing interface");
          if (mrowEditing)
          {
            PRBool structural(PR_TRUE);
            mrowEditing->IsStructural(&structural);
            if (!structural)
            {
              PRUint32 currFlags = (flags&FROM_RIGHT) ? FROM_RIGHT : FROM_LEFT; 
              currFlags |= FROM_PARENT;
              res = mathmlEditing->Accept(editor, currFlags, node, offset);
            } 
          } 
        } 
      }
    }
  }
  if (*node == nsnull)
  {
    *node = m_mathmlNode;
    NS_ADDREF(*node);
    *offset = m_offset;
    res = NS_OK;
  }  
  return res;
}

NS_IMETHODIMP
msiMCaretBase::AdjustSelectionPoint(nsIEditor *editor, PRBool leftSelPoint, 
                                    nsIDOMNode** selectionNode, PRUint32 * selectionOffset,
                                    msiIMathMLCaret** parentCaret)
{
  nsCOMPtr<msiIMathMLCaret> pCaret;
  PRBool incOffset(!leftSelPoint);
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
      if (NS_SUCCEEDED(res))
      {
        *parentCaret = pCaret;
        NS_ADDREF(*parentCaret);
      }
    }
    else 
      res = NS_ERROR_FAILURE;
  }
  return res;
}

NS_IMETHODIMP 
msiMCaretBase::SplitAtDecendents(nsIEditor * editor, nsIDOMNode *leftDecendent, PRUint32 leftOffset, 
                                 nsIDOMNode *rightDecendent, PRUint32 rightOffset, 
                                 PRUint32 *mmlNodeLeftOffset, PRUint32 *mmlNodeRightOffset, 
                                 nsIDOMNode **left_leftPart, nsIDOMNode **left_rightPart, 
                                 nsIDOMNode **right_leftPart, nsIDOMNode **right_rightPart)
{
  if (!editor || !mmlNodeLeftOffset || !mmlNodeRightOffset)
    return NS_ERROR_NULL_POINTER;
  if (!left_leftPart || !left_rightPart || !right_leftPart || !right_rightPart)
    return NS_ERROR_NULL_POINTER;
 
  nsresult res(NS_OK);
  if (leftDecendent && leftOffset != INVALID)
  {
    nsCOMPtr<nsIDOMNode> currDecendent;
    PRUint32 currOffset(INVALID);
    nsCOMPtr<msiIMathMLCaret> currCaret;
    res = msiUtils::GetMathMLCaretInterface(editor, leftDecendent, leftOffset, currCaret);
    if (NS_SUCCEEDED(res) && currCaret)
    {
      res = msiUtils::GetMathmlNodeFromCaretInterface(currCaret, currDecendent);
      if (NS_SUCCEEDED(res))
        res = msiUtils::GetOffsetFromCaretInterface(currCaret, currOffset);
    }
    nsCOMPtr<nsIDOMNode> currLeft, currRight;
    while (NS_SUCCEEDED(res) && currDecendent != m_mathmlNode && currCaret)
    {
      if (NS_SUCCEEDED(res) && currCaret)
      {
        nsCOMPtr<nsIDOMNode> left, right;
        res = currCaret->Split(editor, currLeft, currRight, getter_AddRefs(left), getter_AddRefs(right));
        if (NS_SUCCEEDED(res))
        {
          nsCOMPtr<msiIMathMLCaret> parentCaret;
          res = msiUtils::SetupPassOffCaretToParent(editor, currDecendent, PR_FALSE, parentCaret);
          currLeft = left;
          currRight = right;
          if (NS_SUCCEEDED(res) && parentCaret)
          {
            nsCOMPtr<nsIDOMNode> newNode;
            PRUint32 newOffset(INVALID);
            res = msiUtils::GetMathmlNodeFromCaretInterface(parentCaret, newNode);
            if (NS_SUCCEEDED(res))
              res = msiUtils::GetOffsetFromCaretInterface(parentCaret, newOffset);
            if (NS_SUCCEEDED(res) && newNode && newOffset != INVALID)
            {
              currDecendent = newNode;
              currOffset = newOffset;
              currCaret = parentCaret;
            
            }
            else
              res = NS_ERROR_FAILURE;
          }
          else
            res = NS_ERROR_FAILURE;
        }
      }
      else 
        res = NS_ERROR_FAILURE;
    }
    if (NS_SUCCEEDED(res))
    {
      if (currLeft)
      {
        *left_leftPart = currLeft;
        NS_ADDREF(*left_leftPart);
      }
      if (currRight)
      {
        *left_rightPart = currRight;
        NS_ADDREF(*left_rightPart);
      }
      *mmlNodeLeftOffset = currOffset;
    }
  }
  if (NS_SUCCEEDED(res) && rightDecendent && rightOffset != INVALID)
  {
    nsCOMPtr<nsIDOMNode> currDecendent;
    PRUint32 currOffset(INVALID);
    nsCOMPtr<msiIMathMLCaret> currCaret;
    res = msiUtils::GetMathMLCaretInterface(editor, rightDecendent, rightOffset, currCaret);
    if (NS_SUCCEEDED(res) && currCaret)
    {
      res = msiUtils::GetMathmlNodeFromCaretInterface(currCaret, currDecendent);
      if (NS_SUCCEEDED(res))
        res = msiUtils::GetOffsetFromCaretInterface(currCaret, currOffset);
    }
    nsCOMPtr<nsIDOMNode> currLeft, currRight;
    while (NS_SUCCEEDED(res) && currDecendent != m_mathmlNode && currCaret)
    {
      if (NS_SUCCEEDED(res) && currCaret)
      {
        nsCOMPtr<nsIDOMNode> left, right;
        res = currCaret->Split(editor, currLeft, currRight, getter_AddRefs(left), getter_AddRefs(right));
        if (NS_SUCCEEDED(res))
        {
          nsCOMPtr<msiIMathMLCaret> parentCaret;
          res = msiUtils::SetupPassOffCaretToParent(editor, currDecendent, PR_FALSE, parentCaret);
          currLeft = left;
          currRight = right;
          if (NS_SUCCEEDED(res) && parentCaret)
          {
            nsCOMPtr<nsIDOMNode> newNode;
            PRUint32 newOffset(INVALID);
            res = msiUtils::GetMathmlNodeFromCaretInterface(parentCaret, newNode);
            if (NS_SUCCEEDED(res))
              res = msiUtils::GetOffsetFromCaretInterface(parentCaret, newOffset);
            if (NS_SUCCEEDED(res) && newNode && newOffset != INVALID)
            {
              currDecendent = newNode;
              currOffset = newOffset;
              currCaret = parentCaret;
            
            }
            else
              res = NS_ERROR_FAILURE;
          }
          else
            res = NS_ERROR_FAILURE;
        }
      }
      else 
        res = NS_ERROR_FAILURE;
    }
    if (NS_SUCCEEDED(res))
    {
      if (currLeft)
      {
        *right_leftPart = currLeft;
        NS_ADDREF(*right_leftPart);
      }
      if (currRight)
      {
        *right_rightPart = currRight;
        NS_ADDREF(*right_rightPart);
      }
      *mmlNodeRightOffset = currOffset;
    }
  }
  return res;  
}




NS_IMETHODIMP
msiMCaretBase::Split(nsIEditor *editor, 
                     nsIDOMNode *appendLeft, 
                     nsIDOMNode *appendRight, 
                     nsIDOMNode **left, 
                     nsIDOMNode **right)
{
  if (!m_mathmlNode || !left || !right)
    return NS_ERROR_NULL_POINTER;
  *left = nsnull;
  *right = nsnull;  
  nsCOMPtr<nsIDOMNode> clone;
  nsresult res = msiUtils::CloneNode(m_mathmlNode, clone);
  if (NS_SUCCEEDED(res) && clone)
  {
    if (m_offset <= m_numKids/2)
    {
      *left = clone;
      NS_ADDREF(*left);
    }
    else
    {
      *right = clone;
      NS_ADDREF(*right);
    }  
  }
  else
    res = NS_ERROR_FAILURE;
  return res;    
}  


NS_IMETHODIMP
msiMCaretBase::SetupDeletionTransactions(nsIEditor * editor,
                                         PRUint32 startOffset,
                                         PRUint32 endOffset,
                                         nsIDOMNode * start,
                                         nsIDOMNode * end,
                                         nsIArray ** transactionList)
{
  if (!m_mathmlNode || !editor || !transactionList)
    return NS_ERROR_FAILURE;
  nsresult res(NS_OK);
  if (startOffset == 0 && endOffset == m_numKids && start == nsnull && end == nsnull)
  {
    nsCOMPtr<msiIMathMLCaret> parentCaret;
    res = msiUtils::SetupPassOffCaretToParent(editor, m_mathmlNode, PR_FALSE, parentCaret);
    if (NS_SUCCEEDED(res) && parentCaret)
    {
      PRUint32 offset(INVALID);
      res = msiUtils::GetOffsetFromCaretInterface(parentCaret, offset);
      res = parentCaret->SetupDeletionTransactions(editor, offset, offset+1, 
                                                   nsnull, nsnull, transactionList);
    }
    else
      res = NS_ERROR_FAILURE;
  }
  else
  {
    nsCOMPtr<nsIDOMNode> tobeCoalLeft, tobeCoalRight;
    nsCOMPtr<msiIMathMLEditor> msiEditor(do_QueryInterface(editor));
    nsCOMPtr<nsIDOMNodeList> children;
    nsCOMPtr<nsIMutableArray> mutableTxnArray, tobeCoalesced;
    nsCOMPtr<nsIArray> prepareForCoalLeft, prepareForCoalRight, coalescedArray;
    mutableTxnArray = do_CreateInstance(NS_ARRAY_CONTRACTID, &res);
    m_mathmlNode->GetChildNodes(getter_AddRefs(children));
    if (!msiEditor || !children || !mutableTxnArray)
      return NS_ERROR_FAILURE;
    
    PRUint32 startingIndex(startOffset), endingIndex(endOffset), count(0);
    children->GetLength(&count);
    if (start)
      tobeCoalLeft = start;
    else if (startOffset > 0)  
    {
      nsCOMPtr<nsIDOMNode> child, clone;
      res = children->Item(startOffset-1, getter_AddRefs(child));
      if (NS_SUCCEEDED(res) && child)
      {
        
        msiUtils::CloneNode(child, clone);
        if (clone)
        {
          tobeCoalLeft = clone;
          startingIndex -= 1;
        }
      }
      else
        res = NS_ERROR_FAILURE;
    }
    
    if (end)
    {
      tobeCoalRight = end;
      endingIndex += 1;
    }
    else if (endOffset < count)  
    {
      nsCOMPtr<nsIDOMNode> child, clone;
      res = children->Item(endOffset, getter_AddRefs(child));
      if (NS_SUCCEEDED(res) && child)
      {
        
        msiUtils::CloneNode(child, clone);
        if (clone)
        {
          tobeCoalRight = clone;
          endingIndex += 1;
        }
      }
      else
        res = NS_ERROR_FAILURE;
    }
    if (NS_SUCCEEDED(res))
      tobeCoalesced = do_CreateInstance(NS_ARRAY_CONTRACTID, &res);
      
    if (NS_SUCCEEDED(res) && tobeCoalLeft)
    {
      PRUint32 pfcFlags(msiIMathMLCoalesce::PFCflags_removeRedundantMrows);
      res = msiCoalesceUtils::PrepareForCoalesceFromRight(editor, tobeCoalLeft, pfcFlags, prepareForCoalLeft);
      if (NS_SUCCEEDED(res))
        res = msiUtils::AppendToMutableList(tobeCoalesced, prepareForCoalLeft);
    }  
    if (NS_SUCCEEDED(res) && tobeCoalRight)
    {
      PRUint32 pfcFlags(msiIMathMLCoalesce::PFCflags_removeRedundantMrows);
      res = msiCoalesceUtils::PrepareForCoalesceFromLeft(editor, tobeCoalRight, pfcFlags, prepareForCoalRight);
      if (NS_SUCCEEDED(res))
        res = msiUtils::AppendToMutableList(tobeCoalesced, prepareForCoalRight);
    }
    if (NS_SUCCEEDED(res) && tobeCoalesced)
    {
      nsCOMPtr<nsIArray> tmpArray(do_QueryInterface(tobeCoalesced));
      if (tmpArray)
        res = msiCoalesceUtils::CoalesceArray(editor, tmpArray, coalescedArray);
    }  
    for (PRUint32 i=startingIndex; NS_SUCCEEDED(res) && i < endingIndex; i++)
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
    
    nsCOMPtr<nsISimpleEnumerator> enumerator;
    if (coalescedArray)
      coalescedArray->Enumerate(getter_AddRefs(enumerator));
    if (enumerator)
    {
      PRBool someMore(PR_FALSE);
      PRUint32 offset(startingIndex);
      while (NS_SUCCEEDED(res) && NS_SUCCEEDED(enumerator->HasMoreElements(&someMore)) && someMore) 
      {
        nsCOMPtr<nsISupports> isupp;
        if (NS_FAILED(enumerator->GetNext(getter_AddRefs(isupp))))
        {
          res = NS_ERROR_FAILURE; 
          break;
        }
        nsCOMPtr<nsIDOMNode> child(do_QueryInterface(isupp));
        if (child) 
        {
          nsCOMPtr<nsITransaction> transaction;
          res = msiEditor->CreateInsertTransaction(child, m_mathmlNode, offset, getter_AddRefs(transaction));
          if (NS_SUCCEEDED(res) && transaction)
          {
            res = mutableTxnArray->AppendElement(transaction, PR_FALSE);
            offset += 1;
          }
          else
            res = NS_ERROR_FAILURE;
        }
        else
          res = NS_ERROR_FAILURE;
      }  
    }
    if (NS_SUCCEEDED(res))
    {
      *transactionList = mutableTxnArray;
      NS_ADDREF(*transactionList);
    }  
  }
  return res;
  
}                                         
                                   

NS_IMETHODIMP
msiMCaretBase::CaretLeft(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  if (!node || !offset || !m_mathmlNode || !editor)
    return NS_ERROR_FAILURE;
  nsresult res(NS_OK);
  nsCOMPtr<msiIMathMLCaret> mathmlEditing;
  if (m_offset == 0)
  {
    flags = FROM_CHILD | FROM_RIGHT;
    msiUtils::SetupPassOffCaretToParent(editor, m_mathmlNode, PR_FALSE, mathmlEditing);
    if (mathmlEditing)
      mathmlEditing->Accept(editor, flags, node, offset); 
    else 
      res = NS_ERROR_FAILURE;
  }
  else
  {
    nsCOMPtr<nsIDOMNode> child;
    msiUtils::GetChildNode(m_mathmlNode, m_offset-1, child);
    if (child)
    {
      msiUtils::GetMathMLCaretInterface(editor, child, RIGHT_MOST, mathmlEditing);
      if (mathmlEditing)
      {
        flags = FROM_PARENT;
        PRBool atLeftEquiv(PR_FALSE);
        mathmlEditing->Inquiry(editor, AT_LEFT_EQUIV_TO_RIGHT_MOST, &atLeftEquiv);
        if (atLeftEquiv)
          res = mathmlEditing->CaretLeft(editor, flags, node, offset);
        else
        {
          flags |= FROM_RIGHT;
          res = mathmlEditing->Accept(editor, flags, node, offset); 
        }
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
msiMCaretBase::CaretRight(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  if (!node || !offset || !m_mathmlNode || !editor)
    return NS_ERROR_FAILURE;
  nsresult res(NS_OK);
  nsCOMPtr<msiIMathMLCaret> mathmlEditing;
  if (m_offset == m_numKids)
  {
    msiUtils::SetupPassOffCaretToParent(editor, m_mathmlNode, PR_TRUE, mathmlEditing);
    if (mathmlEditing)
      mathmlEditing->Accept(editor, FROM_CHILD|FROM_LEFT, node, offset); 
    else 
      res = NS_ERROR_FAILURE;
  }
  else
  {
    nsCOMPtr<nsIDOMNode> child;
    msiUtils::GetChildNode(m_mathmlNode, m_offset, child);
    NS_ASSERTION(child, "Child node is null");
    if (child)
    {
      msiUtils::GetMathMLCaretInterface(editor, child, 0, mathmlEditing);
      if (mathmlEditing)
      {
        flags = FROM_PARENT;
        PRBool atRightEquiv(PR_FALSE);
        mathmlEditing->Inquiry(editor, AT_RIGHT_EQUIV_TO_0, &atRightEquiv);
        if (atRightEquiv)
          res = mathmlEditing->CaretRight(editor, flags, node, offset);
        else
        {
          flags |= FROM_LEFT;
          res = mathmlEditing->Accept(editor, flags, node, offset); 
        }
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
msiMCaretBase::CaretUp(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return CaretLeft(editor, flags, node, offset);
}

NS_IMETHODIMP
msiMCaretBase::CaretDown(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return CaretRight(editor, flags, node, offset);
}

NS_IMETHODIMP
msiMCaretBase::CaretObjectLeft(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
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
    nsCOMPtr<nsIDOMNode> child;
    msiUtils::GetChildNode(m_mathmlNode, m_offset-1, child);
    if (child)
    {
      msiUtils::GetMathMLCaretInterface(editor, child, RIGHT_MOST, mathmlEditing);
      if (mathmlEditing)
        res = mathmlEditing->CaretObjectLeft(editor, flags, node, offset);
      else 
        res = NS_ERROR_FAILURE;
    }
    else 
      res = NS_ERROR_FAILURE;
  }
  return res;
}

NS_IMETHODIMP
msiMCaretBase::CaretObjectRight(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
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
    nsCOMPtr<nsIDOMNode> child;
    msiUtils::GetChildNode(m_mathmlNode, m_offset, child);
    NS_ASSERTION(child, "Child node is null");
    if (child)
    {
      msiUtils::GetMathMLCaretInterface(editor, child, 0, mathmlEditing);
      if (mathmlEditing)
          res = mathmlEditing->CaretObjectRight(editor, flags, node, offset);
      else 
        res = NS_ERROR_FAILURE;
    }
    else 
      res = NS_ERROR_FAILURE;
  }
  return res;
}

NS_IMETHODIMP
msiMCaretBase::CaretObjectUp(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return CaretLeft(editor, flags, node, offset);
}

NS_IMETHODIMP
msiMCaretBase::CaretObjectDown(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return CaretRight(editor, flags, node, offset);
}

// Mouse Util functions

nsresult msiMCaretBase::GetNodeFromFrame(nsIFrame* frame, nsCOMPtr<nsIDOMNode> & node)
{
  nsresult res(NS_ERROR_FAILURE);
  if (frame)
  {
    nsIContent * content = frame->GetContent();
    if (content)
      node = do_QueryInterface(content);
    if (node)
      res = NS_OK;  
  }
  return res;  
}

nsresult msiMCaretBase::GetPrimaryFrameForNode(nsIPresShell * presShell, nsIDOMNode * node, nsIFrame ** frame)
{
  nsresult res(NS_ERROR_FAILURE);
  if (node && presShell && frame)
  {
    *frame = nsnull;
    nsCOMPtr<nsIContent> content(do_QueryInterface(node));
    if (content)
      *frame = presShell->GetPrimaryFrameFor(content);
    if (*frame)
      res = NS_OK;  
  }
  return res;
}
// End Mouse Util functions
// Selection Util functions
nsresult msiMCaretBase::GetSelectionPoint(nsIEditor * editor, PRBool leftSelPoint,
                                          nsIDOMNode * startNode, PRUint32 startOffset,
                                          nsCOMPtr<nsIDOMNode> & selectionNode, PRUint32 & selectionOffset)
{
  selectionNode = nsnull;
  selectionOffset = INVALID;
  nsCOMPtr<msiIMathMLCaret> currCaret;
  nsresult res = msiUtils::GetMathMLCaretInterface(editor, startNode, startOffset, currCaret);
  PRUint32 currOffset(INVALID);
  nsCOMPtr<nsIDOMNode> currMathNode;
  if (NS_SUCCEEDED(res) && currCaret)
  {
    nsCOMPtr<msiIMathMLEditingBC> bc(do_QueryInterface(currCaret));
    if (bc)
    {
      bc->GetMathmlNode(getter_AddRefs(currMathNode));
      bc->GetOffset(&currOffset);
    }
    else
      res = NS_ERROR_FAILURE;
  }
  if (NS_SUCCEEDED(res) && currMathNode && currOffset != INVALID)
  {
    selectionNode = currMathNode;
    selectionOffset = currOffset;
    while (NS_SUCCEEDED(res) && currCaret && currMathNode && currMathNode != m_mathmlNode)
    {
      nsCOMPtr <nsIDOMNode> currSelNode;
      PRUint32 currSelOffset(INVALID);
      nsCOMPtr<msiIMathMLCaret> parentCaret;
      res = currCaret->AdjustSelectionPoint(editor, leftSelPoint, getter_AddRefs(currSelNode), 
                                            &currSelOffset, getter_AddRefs(parentCaret));
      if (NS_SUCCEEDED(res) && currSelNode && currSelOffset != INVALID)
      {
        selectionNode = currSelNode;
        selectionOffset = currSelOffset;
      }                                      
      currMathNode = nsnull;
      currOffset = INVALID;
      currCaret = nsnull;
      if (NS_SUCCEEDED(res) && parentCaret)  
      {
        nsCOMPtr<msiIMathMLEditingBC> parentBC(do_QueryInterface(parentCaret));
        if (parentBC)
        {
          parentBC->GetMathmlNode(getter_AddRefs(currMathNode));
          parentBC->GetOffset(&currOffset);
          currCaret = parentCaret;
          res = (currCaret && currMathNode && currOffset != INVALID) ? NS_OK : NS_ERROR_FAILURE;
        }
        else 
          res = NS_ERROR_FAILURE;
      }    
    }
  }
  return res;
}                                              
// End selection Util functions


