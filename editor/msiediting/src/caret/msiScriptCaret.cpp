// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiScriptCaret.h"
#include "msiUtils.h"
#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "nsIEditor.h"
#include "nsISelection.h"
#include "msiRequiredArgument.h"
#include "nsIFrame.h"
#include "nsIView.h"
#include "nsPresContext.h"
#include "nsITransaction.h"
#include "msiIMathMLEditor.h"
#include "nsIMutableArray.h"
#include "nsComponentManagerUtils.h"

msiScriptCaret::msiScriptCaret(nsIDOMNode* mathmlNode, PRUint32 offset, PRUint32 mathmlType)
:  msiMCaretBase(mathmlNode, offset, mathmlType)
{
}


NS_IMETHODIMP
msiScriptCaret::PrepareForCaret(nsIEditor* editor)
{
  nsresult res(NS_ERROR_FAILURE);
  if (m_mathmlNode)
  {
    res = NS_OK;
    nsCOMPtr<nsIDOMNode> child;
    res = m_mathmlNode->GetLastChild(getter_AddRefs(child));
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
    if (NS_SUCCEEDED(res) && m_numKids == 3)
    {
      child = nsnull;
      nsCOMPtr<nsIDOMNode> first;
      res = m_mathmlNode->GetFirstChild(getter_AddRefs(first));
      if (NS_SUCCEEDED(res) && first)
        res = first->GetNextSibling(getter_AddRefs(first));
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
msiScriptCaret::Inquiry(nsIEditor* editor, PRUint32 inquiryID, PRBool *result)
{
  nsresult res(NS_OK);
  *result = PR_FALSE;
  if (inquiryID == AT_RIGHT_EQUIV_TO_0)
    *result = PR_TRUE;
  else if (inquiryID == AT_LEFT_EQUIV_TO_RIGHT_MOST) 
    *result = PR_FALSE;
  else if (inquiryID == CAN_SELECT_CHILD_LEAF)
    *result = (m_numKids == 2 && m_offset >= 1) ? PR_FALSE : PR_TRUE;
  return res;
}

NS_IMETHODIMP
msiScriptCaret::AdjustNodeAndOffsetFromMouseEvent(nsIEditor *editor, nsIPresShell *presShell,
                                                       PRUint32 flags, 
                                                       nsIDOMMouseEvent *mouseEvent, 
                                                       nsIDOMNode **node, 
                                                       PRUint32 *offset)
{
  return msiMCaretBase::AdjustNodeAndOffsetFromMouseEvent(editor, presShell, flags, 
                                                          mouseEvent, node, offset);
}                                                       


NS_IMETHODIMP
msiScriptCaret::Accept(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{

  if (!node || !offset || !m_mathmlNode || !editor)
    return NS_ERROR_FAILURE;
  NS_ASSERTION(flags & FROM_LEFT|FROM_RIGHT, "Accept called without From left or from right");
  nsresult res(NS_OK);
  nsCOMPtr<msiIMathMLCaret> mathmlEditing;
  nsCOMPtr<nsIDOMNode> child;
  if ((flags & FROM_RIGHT))
  {
    if (m_offset == 0)
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
    else if (m_offset == m_numKids)
    {
      msiUtils::GetChildNode(m_mathmlNode, m_offset-1, child);
      msiUtils::GetMathMLCaretInterface(editor, child, RIGHT_MOST, mathmlEditing);
      if (mathmlEditing)
        res = mathmlEditing->Accept(editor, FROM_PARENT|FROM_RIGHT, node, offset);
    }
    else if (!(flags & FROM_CHILD))
    {
      msiUtils::GetChildNode(m_mathmlNode, 0, child);
      if (child)
        msiUtils::GetMathMLCaretInterface(editor, child, RIGHT_MOST, mathmlEditing);
      if (mathmlEditing && msiUtils::IsInputbox(mathmlEditing))
          res = mathmlEditing->Accept(editor, FROM_PARENT, node, offset);
      else
      {
        *node = m_mathmlNode;
        NS_ADDREF(*node);
        *offset = 1;
        res = NS_OK;
      }  
    }
  }  
  else if (flags & FROM_LEFT)
  {
    if (m_offset == 0)
    {
      msiUtils::GetChildNode(m_mathmlNode, 0, child);
      if (child)
      {
        msiUtils::GetMathMLCaretInterface(editor, child, 0, mathmlEditing);
        if (mathmlEditing)
          res = mathmlEditing->Accept(editor, FROM_PARENT|FROM_LEFT, node, offset); 
        else
          res = NS_ERROR_FAILURE; 
      }
    }
    else if (m_offset > 1 && !(flags & FROM_CHILD))
    {
      nsCOMPtr<msiIMathMLCaret> parent;
      msiUtils::SetupPassOffCaretToParent(editor, m_mathmlNode, PR_TRUE, parent);
      if (parent)
        res = parent->Accept(editor, FROM_CHILD|FROM_LEFT, node, offset);
      else
        res = NS_ERROR_FAILURE; 
    }
    else if (m_offset == 1 && (flags & FROM_CHILD))
    {
      *node = m_mathmlNode;
      NS_ADDREF(*node);
      *offset = m_offset;
      res = NS_OK;
    }
    else if (!(flags & FROM_CHILD))
    {
      *node = m_mathmlNode;
      NS_ADDREF(*node);
      *offset = m_offset;
      res = NS_OK;
    }
  }
  return res;
}

NS_IMETHODIMP 
msiScriptCaret::GetSelectableMathFragment(nsIEditor  *editor, 
                                         nsIDOMNode *start,      PRUint32 startOffset, 
                                         nsIDOMNode *end,        PRUint32 endOffset, 
                                         nsIDOMNode **fragStart, PRUint32 *fragStartOffset,
                                         nsIDOMNode **fragEnd,   PRUint32 *fragEndOffset)
{
  //ljh -- code below assumes the script is the "common ancestor" of start and end.
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
      if (m_numKids == 2)
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
        else // (startCompare2 == -1 && endCompare2 == 1)  // select sup or both scripts 
        {
          *fragStart = m_mathmlNode;
          *fragEnd = m_mathmlNode;
          NS_ADDREF(*fragStart);
          NS_ADDREF(*fragEnd);
          *fragStartOffset = startCompare1 == 0 ? 2 : 1;
          *fragEndOffset = 3;
          res = NS_OK;
        }
      }
    }
    else // (startCompare1 == -1 && endCompare1 == 1) have parent select the whole script object
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
msiScriptCaret::AdjustSelectionPoint(nsIEditor *editor, PRBool leftSelPoint, 
                                      nsIDOMNode** selectionNode, PRUint32 * selectionOffset,
                                      msiIMathMLCaret** parentCaret)
{
  nsCOMPtr<msiIMathMLCaret> pCaret;
  PRBool incOffset(!leftSelPoint);
  if (!leftSelPoint && m_offset == 0)
     incOffset = PR_FALSE;
//  else if (leftSelPoint && m_offset == m_numKids)
//    incOffset = PR_TRUE;
  PRBool selDone(PR_FALSE);
  if (m_offset == 1 || (leftSelPoint && m_numKids == m_offset) ||
                       (leftSelPoint && m_numKids == 3 && m_offset == 2))
  {
    *selectionNode = m_mathmlNode;
    NS_ADDREF(*selectionNode);
    *selectionOffset = 1;
    selDone = PR_TRUE;
  }
  nsresult res = msiUtils::SetupPassOffCaretToParent(editor, m_mathmlNode, incOffset, pCaret);
  if (NS_SUCCEEDED(res) && pCaret)  
  {
    if (!selDone)
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
msiScriptCaret::SplitAtDecendents(nsIEditor* editor, 
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
msiScriptCaret::Split(nsIEditor *editor, 
                      nsIDOMNode *appendLeft, 
                      nsIDOMNode *prependRight, 
                      nsIDOMNode **left, 
                      nsIDOMNode **right)
{
  if (!m_mathmlNode || !left || !right)
    return NS_ERROR_NULL_POINTER;
  *left = nsnull;
  *right = nsnull;  
  nsCOMPtr<nsIDOMNode> lf, rt, clone;
  nsresult res = msiUtils::CloneNode(m_mathmlNode, clone);
  if (m_offset == 0)
  {
    if (prependRight)
    {
      nsCOMPtr<nsIDOMNode> dontcare;
      res = msiUtils::ReplaceChildNode(clone, 0, prependRight, dontcare);
    }
    else if (appendLeft)
    {
      nsCOMPtr<nsIDOMElement> inputbox;
      PRUint32 flags(msiIMathMLInsertion::FLAGS_NONE);
      res = msiUtils::CreateInputbox(editor, PR_FALSE, PR_FALSE, flags, inputbox);
      nsCOMPtr<nsIDOMNode> inputboxNode(do_QueryInterface(inputbox));
      if (NS_SUCCEEDED(res) && inputboxNode) 
      {
        nsCOMPtr<nsIDOMNode> dontcare;
        res = msiUtils::ReplaceChildNode(clone, 0, inputboxNode, dontcare);
      }  
    }
    if (NS_SUCCEEDED(res))
    {
      if (appendLeft)
      {
        *left = appendLeft;
        NS_ADDREF(*left);
      }
      *right = clone;
      NS_ADDREF(*right);
    }  
  }
  else if (m_offset == 1)
  {
    nsCOMPtr<nsIDOMNode> first;
    res = m_mathmlNode->GetFirstChild(getter_AddRefs(first));
    res = msiUtils::CloneNode(first, lf);
    nsCOMPtr<nsIDOMElement> inputbox;
    PRUint32 flags(msiIMathMLInsertion::FLAGS_NONE);
    res = msiUtils::CreateInputbox(editor, PR_FALSE, PR_FALSE, flags, inputbox);
    nsCOMPtr<nsIDOMNode> inputboxNode(do_QueryInterface(inputbox));
    if (NS_SUCCEEDED(res) && inputboxNode) 
    {
      nsCOMPtr<nsIDOMNode> dontcare;
      res = msiUtils::ReplaceChildNode(clone, 0, inputboxNode, dontcare);
    }  
    *left = lf;
    NS_ADDREF(*left);
    *right = clone;
    NS_ADDREF(*right);
  }
  else 
  {
    *left = clone;
    NS_ADDREF(*left);
  }
  return res;
}                                     

NS_IMETHODIMP
msiScriptCaret::SetupDeletionTransactions(nsIEditor * editor,
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
  nsresult res(NS_OK);
  if (startOffset == 0 && endOffset == 1 && start == nsnull && end == nsnull)
  {
    nsCOMPtr<nsIDOMNode> clone;
    res = msiUtils::CloneNode(m_mathmlNode, clone);
    if (NS_FAILED(res) || !clone)
      res = NS_ERROR_FAILURE;
    nsCOMPtr<nsIDOMElement> inboxElement;
    PRUint32 flags(msiIMathMLInsertion::FLAGS_NONE);
    res = msiUtils::CreateInputbox(editor, PR_FALSE, PR_FALSE, flags, inboxElement);
    nsCOMPtr<nsIDOMNode> inputbox(do_QueryInterface(inboxElement));
    if (NS_SUCCEEDED(res) && inputbox) 
    {
      nsCOMPtr<nsIDOMNode> dontcare;
      res = msiUtils::ReplaceChildNode(clone, startOffset, inputbox, dontcare);
    }  
    else
      res =NS_ERROR_FAILURE;
    nsCOMPtr<msiIMathMLCaret> parentCaret;
    if (NS_SUCCEEDED(res)) 
      res = msiUtils::SetupPassOffCaretToParent(editor, m_mathmlNode, PR_FALSE, parentCaret);
    if (NS_SUCCEEDED(res) && parentCaret)
    {
      PRUint32 offset(INVALID);
      res = msiUtils::GetOffsetFromCaretInterface(parentCaret, offset);
      if (NS_SUCCEEDED(res) && offset != INVALID) 
        res = parentCaret->SetupDeletionTransactions(editor, offset, offset, 
                                                     nsnull, clone, transactionList);
      else
        res = NS_ERROR_FAILURE;
    }
    else
      res = NS_ERROR_FAILURE;
  }
  else if ((endOffset - startOffset == 1 && end == nsnull) ||
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
  
  else if (startOffset == 1 && endOffset == 3 && start == nsnull && end == nsnull)
  {
    nsCOMPtr<nsIDOMNode> first, clone;
    res = msiUtils::GetChildNode(m_mathmlNode, 0, first);
    if (NS_FAILED(res) || !first)
      res = NS_ERROR_FAILURE;
    if (NS_SUCCEEDED(res))
    {
      res = msiUtils::CloneNode(first, clone);
      if (NS_FAILED(res) || !clone)
        res = NS_ERROR_FAILURE;
    }    
    nsCOMPtr<msiIMathMLCaret> parentCaret;
    if (NS_SUCCEEDED(res)) 
      res = msiUtils::SetupPassOffCaretToParent(editor, m_mathmlNode, PR_FALSE, parentCaret);
    if (NS_SUCCEEDED(res) && parentCaret)
    {
      PRUint32 offset(INVALID);
      res = msiUtils::GetOffsetFromCaretInterface(parentCaret, offset);
      if (NS_SUCCEEDED(res) && offset != INVALID) 
        res = parentCaret->SetupDeletionTransactions(editor, offset, offset+1, 
                                                     clone, nsnull, transactionList);
      else
        res = NS_ERROR_FAILURE;
    }
    else
      res = NS_ERROR_FAILURE;
  }
  else if (endOffset - startOffset > 1) // this should not happen
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
  else
    res = msiMCaretBase::SetupDeletionTransactions(editor, startOffset, endOffset,
                                                   start, end, transactionList);
  return res;
}


NS_IMETHODIMP
msiScriptCaret::CaretLeft(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  if (!node || !offset || !m_mathmlNode || !editor)
    return NS_ERROR_FAILURE;
  nsresult res(NS_OK);
  if (m_offset == 0)
  {
    nsCOMPtr<msiIMathMLCaret> mathmlEditing;
    msiUtils::SetupPassOffCaretToParent(editor, m_mathmlNode, PR_FALSE, mathmlEditing);
    if (mathmlEditing)
      res = mathmlEditing->CaretLeft(editor, FROM_CHILD, node, offset); 
    else 
      res = NS_ERROR_FAILURE;
  }
  else if ((flags & FROM_CHILD) && (m_offset == 1 || (m_numKids == 3 && m_offset == 2)))
  {
    m_offset = 1;  // set caret between base and script
    res = Accept(editor, FROM_RIGHT, node, offset);
  }
  else
    res = msiMCaretBase::CaretLeft(editor, flags, node, offset);
  return res;
}

NS_IMETHODIMP
msiScriptCaret::CaretRight(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  if (!node || !offset || !m_mathmlNode || !editor)
    return NS_ERROR_FAILURE;
  nsresult res(NS_OK);
  nsCOMPtr<nsIDOMNode> child;
  nsCOMPtr<msiIMathMLCaret> mathmlEditing;
  if (m_offset == 2 || m_offset == m_numKids)
  {
    msiUtils::SetupPassOffCaretToParent(editor, m_mathmlNode, PR_TRUE, mathmlEditing);
    if (mathmlEditing)
      res = mathmlEditing->Accept(editor, FROM_CHILD|FROM_LEFT, node, offset); 
    else 
      res = NS_ERROR_FAILURE;
  }
  else if (m_offset == 1)
  {
    msiUtils::GetChildNode(m_mathmlNode, m_numKids-1, child);
    NS_ASSERTION(child, "MathML child node is null.");
    if (child)
    {
      msiUtils::GetMathMLCaretInterface(editor, child, 0, mathmlEditing);
      if (mathmlEditing)
        res = mathmlEditing->Accept(editor, FROM_PARENT|FROM_LEFT, node, offset); 
      else 
        res = NS_ERROR_FAILURE;
    }
    else 
      res = NS_ERROR_FAILURE;
  }
  else
    res = msiMCaretBase::CaretRight(editor, flags, node, offset);
  return res;
}

NS_IMETHODIMP
msiScriptCaret::CaretUp(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return CaretLeft(editor, flags, node, offset);
}

NS_IMETHODIMP
msiScriptCaret::CaretDown(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return CaretRight(editor, flags, node, offset);
}

NS_IMETHODIMP
msiScriptCaret::CaretObjectLeft(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  if (!node || !offset || !m_mathmlNode || !editor)
    return NS_ERROR_FAILURE;
  nsresult res(NS_OK);
  if (m_offset == 0)
  {
    nsCOMPtr<msiIMathMLCaret> mathmlEditing;
    msiUtils::SetupPassOffCaretToParent(editor, m_mathmlNode, PR_FALSE, mathmlEditing);
    if (mathmlEditing)
      res = mathmlEditing->CaretObjectLeft(editor, FROM_CHILD, node, offset); 
    else 
      res = NS_ERROR_FAILURE;
  }
  else if (((flags & FROM_CHILD) && m_offset == 1) || m_offset > 1)
  {
    m_offset = 1;  // set caret between base and script
    res = Accept(editor, FROM_RIGHT, node, offset);
  }
  else
  {
    m_offset = 0; 
    res = Accept(editor, FROM_RIGHT, node, offset);
  }
  return res;
}

NS_IMETHODIMP
msiScriptCaret::CaretObjectRight(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  if (!node || !offset || !m_mathmlNode || !editor)
    return NS_ERROR_FAILURE;
  nsresult res(NS_OK);
  if (m_offset == 0)
  {
    m_offset = 1;
    res = Accept(editor, FROM_LEFT, node, offset); 
  }
  else
  {
    nsCOMPtr<msiIMathMLCaret> mathmlEditing;
    msiUtils::SetupPassOffCaretToParent(editor, m_mathmlNode, PR_TRUE, mathmlEditing);
    if (mathmlEditing)
      res = mathmlEditing->Accept(editor, FROM_CHILD|FROM_LEFT, node, offset); 
    else 
      res = NS_ERROR_FAILURE;
  }
  return res;
}

NS_IMETHODIMP
msiScriptCaret::CaretObjectUp(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return CaretLeft(editor, flags, node, offset);
}

NS_IMETHODIMP
msiScriptCaret::CaretObjectDown(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return CaretRight(editor, flags, node, offset);
}

//private
nsresult
msiScriptCaret::GetFramesAndRects(const nsIFrame * script, 
                                  nsIFrame ** base, nsIFrame ** script1, nsIFrame ** script2,
                                  nsRect & sRect, nsRect &bRect, nsRect& s1Rect, nsRect& s2Rect)
{ // relative to scritp's view
  nsresult res(NS_ERROR_FAILURE);
  *script2 = nsnull;
  s2Rect= nsRect(0,0,0,0);
  if (script)
  {
    *base = script->GetFirstChild(nsnull);
    if (*base)
      *script1 = (*base)->GetNextSibling();
    res = *base && *script1 ? NS_OK : NS_ERROR_FAILURE;  
  }
  if (NS_SUCCEEDED(res))
  {
    sRect = script->GetScreenRectExternal();
    bRect = (*base)->GetScreenRectExternal();
    s1Rect = (*script1)->GetScreenRectExternal();
    if (*script2)
      s2Rect = (*script2)->GetScreenRectExternal();
  }
  return res;
}  
      
#define MIN_THRESHOLD 2 //TODO -- how should this be determined.      

void msiScriptCaret::GetThresholds(const nsRect &sRect, 
                                 const nsRect &bRect, const nsRect& s1Rect, nsRect& s2Rect,
                                 PRInt32 &left, PRInt32& midLf, PRInt32& midRt, PRInt32 & right)
{
  PRInt32 minGap(MIN_THRESHOLD), tmp(0);
  
  left = bRect.x;
  if (left < sRect.x + minGap)
    left = sRect.x + minGap;
  
  right = s1Rect.x + s1Rect.width;
  if (!s2Rect.IsEmpty() && s2Rect.x+s2Rect.width > right)
    right = s2Rect.x+s2Rect.width;
  if ( right > sRect.x + sRect.width - minGap)  
    right = sRect.x + sRect.width - minGap;
    
  midLf = bRect.x + bRect.width;
  midRt = s1Rect.x;
  if (!s2Rect.IsEmpty() && s2Rect.x < s1Rect.x)
    midRt = s2Rect.x;
  if (midRt < midLf)
  {
    tmp = midLf;
    midLf = midRt;
    midRt = tmp;
  }  
  tmp = MIN_THRESHOLD;
  if (midRt - midLf < tmp)
  {
    tmp -= (midRt - midLf);
    midLf -= tmp/2;
    midRt += tmp/2;
  }
}                                      
