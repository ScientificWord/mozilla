// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiMathCaret.h"
#include "msiUtils.h"
#include "msiEditingAtoms.h"
#include "msiMContainerCaret.h"
#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "nsIDOMElement.h"
#include "nsIDOMAttr.h"
#include "nsIDOMNodeList.h"
#include "nsIHTMLEditor.h"
#include "nsString.h"
#include "nsISelectionPrivate.h"
#include "msiIMathMLEditor.h"

//TODO many absorb issues to understand and deal with

msiMathCaret::msiMathCaret(nsIDOMNode* mathmlNode, PRUint32 offset) : 
  msiMContainerCaret(mathmlNode, offset, MATHML_MATH), m_isDisplay(PR_FALSE)
{
  nsCOMPtr<nsIDOMElement> mathElement(do_QueryInterface(mathmlNode));
  if (mathElement)
  {
    nsAutoString displayValue, modeValue, display, mode;
    msiEditingAtoms::display->ToString(display);
    msiEditingAtoms::mode->ToString(mode);
    
    mathElement->GetAttribute(display, displayValue);
    mathElement->GetAttribute(mode, modeValue);
    if (msiEditingAtoms::block->Equals(displayValue))
      m_isDisplay = PR_TRUE;
    if (!m_isDisplay && !(msiEditingAtoms::msiinline->Equals(displayValue)) && 
        msiEditingAtoms::display->Equals(modeValue))
        m_isDisplay = PR_TRUE;
  }      
}

NS_IMETHODIMP
msiMathCaret::PrepareForCaret(nsIEditor* editor)
{
  return NS_OK;
}

NS_IMETHODIMP
msiMathCaret::Inquiry(nsIEditor* editor, PRUint32 inquiryID, PRBool *result)
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
msiMathCaret::AdjustNodeAndOffsetFromMouseEvent(nsIEditor *editor, nsIPresShell *presShell,
                                                       PRUint32 flags, 
                                                       nsIDOMMouseEvent *mouseEvent, 
                                                       nsIDOMNode **node, 
                                                       PRUint32 *offset)
{
  if (!editor || !node || !offset || !presShell || !m_mathmlNode || !mouseEvent)
    return NS_ERROR_FAILURE;
  flags = m_offset == 0 ? FROM_RIGHT : FROM_LEFT;
  return Accept(editor, flags, node, offset);
}                                                       
                            

NS_IMETHODIMP
msiMathCaret::Accept(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return msiMContainerCaret::Accept(editor, flags, node, offset);
}

NS_IMETHODIMP 
msiMathCaret::GetSelectableMathFragment(nsIEditor  *editor, 
                                              nsIDOMNode *start,      PRUint32 startOffset, 
                                              nsIDOMNode *end,        PRUint32 endOffset, 
                                              nsIDOMNode **fragStart, PRUint32 *fragStartOffset,
                                              nsIDOMNode **fragEnd,   PRUint32 *fragEndOffset)
{
  if (!editor)
    return NS_ERROR_FAILURE;
  if (!(start && startOffset <= LAST_VALID) && !(end && endOffset <= LAST_VALID))
    return NS_ERROR_FAILURE;
  if (!fragStart || !fragEnd || !fragStartOffset || !fragEndOffset)
    return NS_ERROR_FAILURE;
  *fragStart = nsnull;
  *fragEnd = nsnull;
  *fragStartOffset = INVALID;
  *fragEndOffset = INVALID;
  nsresult res(NS_OK);
  if (start && startOffset <= LAST_VALID && end && endOffset <= LAST_VALID)
    return msiMContainerCaret::GetSelectableMathFragment(editor, start, startOffset, 
                                                         end, endOffset, fragStart, fragStartOffset,
                                                         fragEnd, fragEndOffset);
  else
  {
    nsCOMPtr<nsIDOMNode> node;
    PRUint32 offset(INVALID);
    if (start && startOffset <= LAST_VALID)
    {
      res = msiMCaretBase::GetSelectionPoint(editor, PR_TRUE, start, startOffset, node, offset);
      if (NS_SUCCEEDED(res) && node && offset <= LAST_VALID)
      {
        *fragStart = node;
        NS_ADDREF(*fragStart);
        *fragStartOffset = offset;
      }                                       
    }
    else // end && endOffset <= LAST_VALID
    {
      res = msiMCaretBase::GetSelectionPoint(editor, PR_FALSE, end, endOffset, node, offset);
      if (NS_SUCCEEDED(res) && node && offset <= LAST_VALID)
      {
        *fragEnd = node;
        NS_ADDREF(*fragEnd);
        *fragEndOffset = offset;
      }                                       
    }
  }
  return res;                                                       
}

NS_IMETHODIMP
msiMathCaret::AdjustSelectionPoint(nsIEditor *editor, PRBool leftSelPoint, 
                                   nsIDOMNode** selectionNode, PRUint32 * selectionOffset,
                                   msiIMathMLCaret ** parent)
{
  *parent = nsnull;
  return NS_OK;
}

NS_IMETHODIMP 
msiMathCaret::SplitAtDecendents(nsIEditor* editor, 
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
msiMathCaret::Split(nsIEditor *editor, 
                    nsIDOMNode *appendLeft, 
                    nsIDOMNode *prependRight, 
                    nsIDOMNode **left, 
                    nsIDOMNode **right)
{
  return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP
msiMathCaret::SetDeletionTransaction(nsIEditor * editor,
                                     PRBool deletingToTheRight, 
                                     nsITransaction ** txn,
                                     PRBool * toRightInParent)
{
  NS_ASSERTION(PR_FALSE, "Should not be here since the math element is always top level.\n");
  if (!editor || !m_mathmlNode || !txn || !toRightInParent)
    return NS_ERROR_NULL_POINTER;
  nsCOMPtr<msiIMathMLEditor> msiEditor(do_QueryInterface(editor));
  if (!msiEditor)
    return NS_ERROR_FAILURE;  
  nsresult res(NS_OK);  
  *txn = nsnull;
  *toRightInParent = deletingToTheRight;
  if (deletingToTheRight && (m_offset < m_numKids))
    res = msiEditor->CreateDeleteChildrenTransaction(m_mathmlNode, m_offset, m_numKids-m_offset, txn);
  else if (!deletingToTheRight && (0 < m_offset))
    res = msiEditor->CreateDeleteChildrenTransaction(m_mathmlNode, 0, m_offset, txn);
  return res;
}                                      

NS_IMETHODIMP
msiMathCaret::SetupDeletionTransactions(nsIEditor * editor,
                                        nsIDOMNode * start,
                                        PRUint32 startOffset,
                                        nsIDOMNode * end,
                                        PRUint32 endOffset,
                                        nsIArray ** transactionList,
                                        nsIDOMNode ** coalesceNode,
                                        PRUint32 * coalesceOffset)
{
  if (!start)
  {
    start = m_mathmlNode;
    startOffset = 0;
  }  
  if (!end)
  {
    end = m_mathmlNode;
    endOffset = m_numKids;
  }  
  return msiMCaretBase::InputboxSetupDelTxns(editor, m_mathmlNode, m_numKids, start, startOffset,
                                             end, endOffset, transactionList, coalesceNode, coalesceOffset);
}

NS_IMETHODIMP
msiMathCaret::SetupCoalesceTransactions(nsIEditor * editor,
                                        nsIArray ** coalesceTransactions)
{
  return msiMCaretBase::SetupCoalesceTransactions(editor, coalesceTransactions);
}                                               


NS_IMETHODIMP
msiMathCaret::SetCoalTransactionsAndNode(nsIEditor * editor,
                                        PRBool onLeft,
                                        nsIArray ** transactionList,
                                        nsIDOMNode **coalesceNode)
{
  NS_ASSERTION(PR_FALSE, "This should not be called.");
  return NS_ERROR_FAILURE;
}      

                                   
NS_IMETHODIMP
msiMathCaret::CaretLeft(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  if (!node || !offset || !m_mathmlNode || !editor)
    return NS_ERROR_FAILURE;
  nsresult res(NS_OK);
  if (m_offset > 0)
    res = msiMContainerCaret::CaretLeft(editor, flags, node, offset); 
  else
  {
    PRUint32 newOffset(0);
    nsCOMPtr<nsIDOMNode> carrotNode;
    msiUtils::GetIndexOfChildInParent(m_mathmlNode, newOffset);
    m_mathmlNode->GetParentNode(getter_AddRefs(carrotNode));
    NS_ASSERTION(carrotNode, "Parent node is null");
    if (carrotNode && newOffset <= LAST_VALID)
    {
      *node = carrotNode;
      NS_ADDREF(*node);
      *offset = newOffset;
      res = NS_OK;
    }
  }
  return res;  
}

NS_IMETHODIMP
msiMathCaret::CaretRight(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  if (!node || !offset || !m_mathmlNode || !editor)
    return NS_ERROR_FAILURE;
  nsresult res(NS_ERROR_FAILURE); 
  if (m_offset < m_numKids)
    res = msiMContainerCaret::CaretRight(editor, flags, node, offset); 
  else // leave math
  { 
    PRUint32 index(INVALID);
    nsCOMPtr<nsIDOMNode> parent;
    msiUtils::GetIndexOfChildInParent(m_mathmlNode, index);
    m_mathmlNode->GetParentNode(getter_AddRefs(parent));
    NS_ASSERTION(parent, "Parent node is null");
    if (parent && IS_VALID_NODE_OFFSET(index))
    {
      *node = parent;
      NS_ADDREF(*node);
      *offset = index+1;
      res = NS_OK;
    }
  }
  return res;  
}

NS_IMETHODIMP
msiMathCaret::CaretUp(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return NS_ERROR_FAILURE;
}

NS_IMETHODIMP
msiMathCaret::CaretDown(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return NS_ERROR_FAILURE;
}

NS_IMETHODIMP
msiMathCaret::CaretObjectLeft(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  if (!node || !offset || !m_mathmlNode || !editor)
    return NS_ERROR_FAILURE;
  nsresult res(NS_OK);
  if (m_offset > 0)
    res = msiMContainerCaret::CaretObjectLeft(editor, flags, node, offset); 
  else
  {
    PRUint32 newOffset(0);
    nsCOMPtr<nsIDOMNode> carrotNode;
    msiUtils::GetIndexOfChildInParent(m_mathmlNode, newOffset);
    m_mathmlNode->GetParentNode(getter_AddRefs(carrotNode));
    NS_ASSERTION(carrotNode, "Parent node is null");
    if (carrotNode && newOffset <= LAST_VALID)
    {
      *node = carrotNode;
      NS_ADDREF(*node);
      *offset = newOffset;
      res = NS_OK;
    }
  }
  return res;  
}

NS_IMETHODIMP
msiMathCaret::CaretObjectRight(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  if (!node || !offset || !m_mathmlNode || !editor)
    return NS_ERROR_FAILURE;
  nsresult res(NS_ERROR_FAILURE); 
  if (m_offset < m_numKids)
    res = msiMContainerCaret::CaretObjectRight(editor, flags, node, offset); 
  else // leave math
  { 
    PRUint32 index(INVALID);
    nsCOMPtr<nsIDOMNode> parent;
    msiUtils::GetIndexOfChildInParent(m_mathmlNode, index);
    m_mathmlNode->GetParentNode(getter_AddRefs(parent));
    NS_ASSERTION(parent, "Parent node is null");
    if (parent && IS_VALID_NODE_OFFSET(index))
    {
      *node = parent;
      NS_ADDREF(*node);
      *offset = index+1;
      res = NS_OK;
    }
  }
  return res;  
}

NS_IMETHODIMP
msiMathCaret::CaretObjectUp(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return CaretLeft(editor, flags, node, offset);
}

NS_IMETHODIMP
msiMathCaret::CaretObjectDown(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return CaretRight(editor, flags, node, offset);
}

NS_IMETHODIMP
msiMathCaret::TabLeft(nsIEditor *editor, nsIDOMNode **node, PRUint32 *offset)
{
  return NS_OK;  // nowhere to go
}

NS_IMETHODIMP
msiMathCaret::TabRight(nsIEditor *editor, nsIDOMNode **node, PRUint32 *offset)
{
  return NS_OK;  // nowhere to go
}

