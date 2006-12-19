// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.
#include "msiBigOpCaret.h"
#include "msiIMrowEditing.h"
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
#include "nsIArray.h"
#include "nsIMutableArray.h"
#include "nsComponentManagerUtils.h"

msiBigOperatorCaret::msiBigOperatorCaret(nsIDOMNode* mathmlNode, PRUint32 offset,
                                         nsCOMPtr<msiIBigOpInfo> & bigOpInfo)
:msiMCaretBase(mathmlNode, offset, MSI_BIGOPERATOR), m_bigOpInfo(bigOpInfo)
{
}

NS_IMETHODIMP
msiBigOperatorCaret::PrepareForCaret(nsIEditor* editor)
{
  if (!m_bigOpInfo || !m_mathmlNode)
    return NS_ERROR_FAILURE;
  PRUint32 scriptType(MATHML_UNKNOWN);
  m_bigOpInfo->GetScriptType(&scriptType);
  if (scriptType == MATHML_UNKNOWN) 
    return NS_OK;
  // has scripts
  nsresult res(NS_OK);
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
        res = mrowNode->AppendChild(kid, getter_AddRefs(dontcare));
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
  return res;
}

NS_IMETHODIMP
msiBigOperatorCaret::Inquiry(nsIEditor* editor, PRUint32 inquiryID, PRBool *result)
{
  PRUint32 scriptType(MATHML_UNKNOWN);
  if (m_bigOpInfo)
    m_bigOpInfo->GetScriptType(&scriptType);
  if (inquiryID == AT_RIGHT_EQUIV_TO_0)
    *result = PR_TRUE;
  else if (inquiryID == AT_LEFT_EQUIV_TO_RIGHT_MOST && scriptType == MATHML_UNKNOWN) 
    *result = PR_TRUE;
  else if (inquiryID == CAN_SELECT_CHILD_LEAF) 
    *result = PR_TRUE;
  else
    *result = PR_FALSE;
  return NS_OK;
}

NS_IMETHODIMP
msiBigOperatorCaret::AdjustNodeAndOffsetFromMouseEvent(nsIEditor *editor, nsIPresShell *presShell, 
                                                       PRUint32 flags, 
                                                       nsIDOMMouseEvent *mouseEvent, 
                                                       nsIDOMNode **node, 
                                                       PRUint32 *offset)
{
  if (!editor || !node || !offset || !presShell || !m_mathmlNode || !mouseEvent)
    return NS_ERROR_FAILURE;
    nsIFrame *bigOpFrame = nsnull;
  nsresult  res = msiMCaretBase::GetPrimaryFrameForNode(presShell, m_mathmlNode, &bigOpFrame);
  if (NS_SUCCEEDED(res) && bigOpFrame)
  {
    nsPoint eventPoint(0,0);
    nsRect bigOpRect(0,0,0,0);
    bigOpRect = bigOpFrame->GetScreenRectExternal();
    res = msiUtils::GetScreenPointFromMouseEvent(mouseEvent, eventPoint);                                     
    if (NS_SUCCEEDED(res))
    {
      m_offset = eventPoint.x <= (bigOpRect.x + (bigOpRect.width/2)) ? 0 : m_numKids;
      flags = m_offset == 0 ? FROM_RIGHT : FROM_LEFT;
      res = Accept(editor, flags, node, offset);
    }  
  }
  return res;
}                                                       
       
NS_IMETHODIMP 
msiBigOperatorCaret::GetSelectableMathFragment(nsIEditor  *editor, 
                                               nsIDOMNode *start,      PRUint32 startOffset, 
                                               nsIDOMNode *end,        PRUint32 endOffset, 
                                               nsIDOMNode **fragStart, PRUint32 *fragStartOffset,
                                               nsIDOMNode **fragEnd,   PRUint32 *fragEndOffset)
{
  //ljh -- code below assumes the big Op is the "common ancestor" of start and end.
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
    PRUint32 scriptType(MATHML_UNKNOWN);
    if (m_bigOpInfo)
      m_bigOpInfo->GetScriptType(&scriptType);
    if (scriptType == MATHML_UNKNOWN) 
      toParent = PR_TRUE;
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
      if (startCompare1 == -1 && endCompare1 <= 0)
        toParent = PR_TRUE;
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
    }
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
msiBigOperatorCaret::AdjustSelectionPoint(nsIEditor *editor, PRBool leftSelPoint, 
                                          nsIDOMNode** selectionNode, PRUint32 * selectionOffset,
                                          msiIMathMLCaret ** parentCaret)
{
  return msiMCaretBase::AdjustSelectionPoint(editor, leftSelPoint, selectionNode, selectionOffset, parentCaret);
}

NS_IMETHODIMP
msiBigOperatorCaret::Accept(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  if (!node || !offset || !m_mathmlNode || !editor)
    return NS_ERROR_FAILURE;
  nsresult res(NS_OK);
  PRUint32 scriptType(MATHML_UNKNOWN);
  if (m_bigOpInfo)
    m_bigOpInfo->GetScriptType(&scriptType);
  nsCOMPtr<msiIMathMLCaret> mathmlEditing;
  nsCOMPtr<nsIDOMNode> child;
  NS_ASSERTION(flags & FROM_LEFT|FROM_RIGHT, "Accept called without From left or from right");
  if (scriptType == MATHML_UNKNOWN) 
  {
    if (!(flags&FROM_PARENT))
    {
      PRBool incOffset = m_offset != 0;
      msiUtils::SetupPassOffCaretToParent(editor, m_mathmlNode, incOffset, mathmlEditing);
      if (mathmlEditing)
      {
        PRUint32 currFlags(FROM_CHILD);
        if (flags & FROM_RIGHT)
          currFlags |= FROM_RIGHT;
        else
          currFlags |= FROM_LEFT;
        res = mathmlEditing->Accept(editor, currFlags, node, offset); 
      }  
    }
    if (*node == nsnull)
    {
      *node  = m_mathmlNode;
      NS_ADDREF(*node);
      *offset = m_offset;
    }
  }
  else if (flags & FROM_LEFT)
  {
    if (m_offset == 0)
    {
      *node  = m_mathmlNode;
      NS_ADDREF(*node);
      *offset = m_offset;
    }
    else if (m_offset == 1)
    {
      msiUtils::GetChildNode(m_mathmlNode, m_numKids-1, child);
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
    if (m_offset == 0 ) //|| m_offset == 1)
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
  }
  return res;
}

NS_IMETHODIMP 
msiBigOperatorCaret::SplitAtDecendents(nsIEditor* editor, 
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
msiBigOperatorCaret::Split(nsIEditor *editor, 
                           nsIDOMNode *appendLeft, 
                           nsIDOMNode *prependRight, 
                           nsIDOMNode **left, 
                           nsIDOMNode **right)
{
  return msiMCaretBase::Split(editor, appendLeft, prependRight, left, right);
}            

NS_IMETHODIMP
msiBigOperatorCaret::SetDeletionTransaction(nsIEditor * editor,
                                            PRBool deletingToTheRight, 
                                            nsITransaction ** txn,
                                            PRBool * toRightInParent)
{
  return msiMCaretBase::SetDeletionTransaction(editor, deletingToTheRight, txn, toRightInParent);
}                                            
                         

NS_IMETHODIMP
msiBigOperatorCaret::SetupDeletionTransactions(nsIEditor * editor,
                                               nsIDOMNode * start,
                                               PRUint32 startOffset,
                                               nsIDOMNode * end,
                                               PRUint32 endOffset,
                                               nsIArray ** transactionList,
                                               nsIDOMNode ** coalesceNode,
                                               PRUint32 * coalesceOffset)
{
  if (!m_mathmlNode || !editor || !transactionList || 
      !coalesceNode || !coalesceOffset )
    return NS_ERROR_FAILURE;
  if (!(IS_VALID_NODE_OFFSET(startOffset)) || !(IS_VALID_NODE_OFFSET(endOffset)))
    return NS_ERROR_FAILURE;
  *coalesceNode = nsnull;
  *coalesceOffset = INVALID;
  nsCOMPtr<msiIMathMLEditor> msiEditor(do_QueryInterface(editor));
  if (!msiEditor)
    return NS_ERROR_FAILURE;  

  nsresult res(NS_OK);
  PRUint32 scriptType(MATHML_UNKNOWN);
  if (m_bigOpInfo)
    m_bigOpInfo->GetScriptType(&scriptType);
  if (scriptType == MATHML_UNKNOWN) 
    return msiMCaretBase::SetupDeletionTransactions(editor, start, startOffset, 
                                                   end, endOffset, transactionList, 
                                                   coalesceNode, coalesceOffset);
                                                   
  nsCOMPtr<nsIMutableArray> mutableTxnArray = do_CreateInstance(NS_ARRAY_CONTRACTID, &res);
  nsCOMPtr<nsIMutableArray> leftTxnList = do_CreateInstance(NS_ARRAY_CONTRACTID, &res);
  nsCOMPtr<nsIMutableArray> rightTxnList = do_CreateInstance(NS_ARRAY_CONTRACTID, &res);
  PRUint32 leftOffsetInTop(INVALID), rightOffsetInTop(INVALID);
  
  msiMCaretBase::SetUpDeleteTxnsFromDescendent(editor, m_mathmlNode, m_numKids, start, startOffset, PR_TRUE, leftTxnList, leftOffsetInTop);
  msiMCaretBase::SetUpDeleteTxnsFromDescendent(editor, m_mathmlNode, m_numKids, end, endOffset, PR_FALSE, rightTxnList, rightOffsetInTop);
                                                   
  if (leftOffsetInTop >= 1 && rightOffsetInTop - leftOffsetInTop == 1)
  {
    nsCOMPtr<nsIDOMElement> inputboxElement;
    nsCOMPtr<nsIDOMNode> newKid;
    PRUint32 flags(msiIMathMLInsertion::FLAGS_NONE);
    res = msiUtils::CreateInputbox(editor, PR_FALSE, PR_FALSE, flags, inputboxElement);
    if (NS_SUCCEEDED(res) && inputboxElement)
      newKid = do_QueryInterface(inputboxElement);
    if (!newKid)
      res = NS_ERROR_FAILURE;  
    nsCOMPtr<nsITransaction> transaction;
    nsCOMPtr<nsIDOMNode> oldKid;
    res = msiUtils::GetChildNode(m_mathmlNode, leftOffsetInTop, oldKid);
    if (NS_SUCCEEDED(res) && oldKid)
      res = msiEditor->CreateReplaceTransaction(newKid, oldKid, m_mathmlNode, getter_AddRefs(transaction));
    if (NS_SUCCEEDED(res) && transaction)
      res = mutableTxnArray->AppendElement(transaction, PR_FALSE);
    else
      res = NS_ERROR_FAILURE;
  }
  else if (leftOffsetInTop >= 1 && rightOffsetInTop == leftOffsetInTop )
  {
    nsCOMPtr<nsIArray> left(do_QueryInterface(leftTxnList));
    nsCOMPtr<nsIArray> right(do_QueryInterface(rightTxnList));
    PRUint32 leftLen(0), rightLen(0);
    if (left)
      left->GetLength(&leftLen);
    if (right)
      right->GetLength(&rightLen);
      
    if (rightLen > 0)
      res = msiUtils::AppendToMutableList(mutableTxnArray, right);
    if (leftLen > 0)
      res = msiUtils::AppendToMutableList(mutableTxnArray, left);
  }
  else // this should not happen
  {
    NS_ASSERTION(PR_FALSE, "Yucky\n");
    nsCOMPtr<msiIMathMLCaret> parentCaret;
    res = msiUtils::SetupPassOffCaretToParent(editor, m_mathmlNode, PR_FALSE, parentCaret);
    if (NS_SUCCEEDED(res) && parentCaret)
    {
      nsCOMPtr<nsIDOMNode> mmlNode;
      PRUint32 offset(INVALID);
      res = msiUtils::GetOffsetFromCaretInterface(parentCaret, offset);
      if (NS_SUCCEEDED(res)) 
        res = msiUtils::GetMathmlNodeFromCaretInterface(parentCaret, mmlNode);
      
      if (NS_SUCCEEDED(res) && offset != INVALID && mmlNode) 
        res = parentCaret->SetupDeletionTransactions(editor, mmlNode, offset, 
                                                     mmlNode, offset+1, transactionList,
                                                     coalesceNode, coalesceOffset);
      else
        res = NS_ERROR_FAILURE;
    }
    else
      res = NS_ERROR_FAILURE;
  }
  
  nsCOMPtr<nsIArray> txnList(do_QueryInterface(mutableTxnArray));
  PRUint32 len(0);
  if (txnList)
    txnList->GetLength(&len);
  if (NS_SUCCEEDED(res) && len > 0)
  {
    *transactionList = mutableTxnArray;
    NS_ADDREF(*transactionList);
  }  
  return res;
}


NS_IMETHODIMP
msiBigOperatorCaret::SetupCoalesceTransactions(nsIEditor * editor,
                                               nsIArray ** coalesceTransactions)
{
  return msiMCaretBase:: SetupCoalesceTransactions(editor, coalesceTransactions);
}                                               

NS_IMETHODIMP
msiBigOperatorCaret::SetCoalTransactionsAndNode(nsIEditor * editor,
                                                PRBool onLeft,
                                                nsIArray ** transactionList,
                                                nsIDOMNode **coalesceNode)
{
  return msiMCaretBase::SetCoalTransactionsAndNode(editor, onLeft, transactionList, coalesceNode);
}                                         

NS_IMETHODIMP
msiBigOperatorCaret::CaretLeft(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  if (!node || !offset || !m_mathmlNode || !editor)
    return NS_ERROR_FAILURE;
  nsresult res(NS_OK);
  PRUint32 scriptType(MATHML_UNKNOWN);
  if (m_bigOpInfo)
    m_bigOpInfo->GetScriptType(&scriptType);
  nsCOMPtr<nsIDOMNode> child;
  nsCOMPtr<msiIMathMLCaret> mathmlEditing;
  if (m_offset == 0)
  {
    msiUtils::SetupPassOffCaretToParent(editor, m_mathmlNode, PR_FALSE, mathmlEditing);
    NS_ASSERTION(mathmlEditing, "Parent's mathmlEditing interface is null");
    if (mathmlEditing)
      res = mathmlEditing->CaretLeft(editor, FROM_CHILD|FROM_RIGHT, node, offset);
  }
  else if (m_offset == 1 || (m_numKids == 3 && m_offset == 2))
  {
    m_offset = 0;
    res = Accept(editor, FROM_RIGHT, node, offset);
  }
  else 
  {
    msiUtils::GetChildNode(m_mathmlNode, m_numKids-1, child);
    if (child)
    {
      msiUtils::GetMathMLCaretInterface(editor, child, RIGHT_MOST, mathmlEditing);
      if (mathmlEditing)
        res = mathmlEditing->Accept(editor, FROM_PARENT|FROM_RIGHT, node, offset); 
      else 
        res = NS_ERROR_FAILURE;
    }
    else 
      res = NS_ERROR_FAILURE;
  }
  return res;   
}

NS_IMETHODIMP
msiBigOperatorCaret::CaretRight(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  if (!node || !offset || !m_mathmlNode || !editor)
    return NS_ERROR_FAILURE;
  nsresult res(NS_OK);
  PRUint32 scriptType(MATHML_UNKNOWN);
  nsCOMPtr<nsIDOMNode> child;
  nsCOMPtr<msiIMathMLCaret> mathmlEditing;
  if (m_bigOpInfo)
    m_bigOpInfo->GetScriptType(&scriptType);
  if (scriptType == MATHML_UNKNOWN) 
  {
    if (m_offset == 0)
    {
      m_offset = 1;
      res =Accept(editor, FROM_LEFT, node, offset);
    }
    else
    {
      msiUtils::SetupPassOffCaretToParent(editor, m_mathmlNode, PR_TRUE, mathmlEditing);
      if (mathmlEditing)
        res = mathmlEditing->CaretRight(editor, FROM_CHILD, node, offset); 
      else 
        res = NS_ERROR_FAILURE;
    }  
  }
  else // in particular scriptType != MATHML_UNKNOWN
  {
    if (m_offset == 2 || m_offset == m_numKids)
    {
      msiUtils::SetupPassOffCaretToParent(editor, m_mathmlNode, PR_TRUE, mathmlEditing);
      if (mathmlEditing)
        res = mathmlEditing->Accept(editor, FROM_CHILD|FROM_RIGHT, node, offset); 
      else 
        res = NS_ERROR_FAILURE;
    }
    else
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
  }
  return res;
}

NS_IMETHODIMP
msiBigOperatorCaret::CaretUp(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return CaretLeft(editor, flags, node, offset);
}

NS_IMETHODIMP
msiBigOperatorCaret::CaretDown(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return CaretRight(editor, flags, node, offset);
}

NS_IMETHODIMP
msiBigOperatorCaret::CaretObjectLeft(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  m_offset = 0;
  return Accept(editor, FROM_RIGHT, node, offset);
}

NS_IMETHODIMP
msiBigOperatorCaret::CaretObjectRight(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  m_offset = m_numKids;
  return Accept(editor, FROM_LEFT, node, offset);
}

NS_IMETHODIMP
msiBigOperatorCaret::CaretObjectUp(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return CaretLeft(editor, flags, node, offset);
}

NS_IMETHODIMP
msiBigOperatorCaret::CaretObjectDown(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return CaretRight(editor, flags, node, offset);
}

//private
nsresult
msiBigOperatorCaret::GetFramesAndRects(const nsIFrame * script, 
                                 nsIFrame ** base, nsIFrame ** script1, nsIFrame ** script2,
                                 nsRect & sRect, nsRect &bRect, nsRect& s1Rect, nsRect& s2Rect,
                                 PRBool& isAboveBelow)
{ // relative to scritp's view
  nsresult res(NS_ERROR_FAILURE);
  *script2 = nsnull;
  s2Rect= nsRect(0,0,0,0);
  isAboveBelow = PR_TRUE;
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
    if (bRect.x + bRect.width <= s1Rect.x)
      isAboveBelow = PR_FALSE;
  }
  return res;
}  

void
msiBigOperatorCaret::GetAtRightThresholds(const nsRect& sRect, const nsRect& bRect, const nsRect& s1Rect, nsRect& s2Rect,
                                          PRInt32& left, PRInt32& right)
{
  left = bRect.x+bRect.width;
  right = s1Rect.x + s1Rect.width;
  if (!s2Rect.IsEmpty() && s2Rect.x+s2Rect.width > right)
    right = s2Rect.x+s2Rect.width;
  if ( right > sRect.x + sRect.width)  
    right = sRect.x + sRect.width;
}                           



void
msiBigOperatorCaret::GetAboveBelowThresholds(const nsRect& sRect, const nsRect& bRect, const nsRect& s1Rect, nsRect& s2Rect,
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
