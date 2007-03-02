// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.
#include "msiInputboxCaret.h"
#include "msiUtils.h"
#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "nsIDOMText.h"
#include "nsIDOMNamedNodeMap.h"
#include "nsIDOMAttr.h"
#include "nsIEditor.h"
#include "nsIDOMEvent.h"
#include "nsRect.h"
#include "nsIFrame.h"
#include "nsPoint.h"

msiInputboxCaret::msiInputboxCaret(nsIDOMNode* mathmlNode, PRUint32 offset)
:  msiMCaretBase(mathmlNode, offset, MSI_INPUTBOX)
{
  
}

NS_IMETHODIMP
msiInputboxCaret::PrepareForCaret(nsIEditor* editor)
{
  return NS_OK;
}

NS_IMETHODIMP
msiInputboxCaret::Inquiry(nsIEditor* editor, PRUint32 inquiryID, PRBool *result)
{
  if (inquiryID == AT_RIGHT_EQUIV_TO_0 || inquiryID == AT_LEFT_EQUIV_TO_RIGHT_MOST) 
    *result = PR_FALSE; //TODO -- not sure about this
  else
    *result = PR_FALSE;
  return NS_OK;
}

NS_IMETHODIMP
msiInputboxCaret::AdjustNodeAndOffsetFromMouseEvent(nsIEditor *editor, nsIPresShell *presShell,
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
    if (baseRect.x <= eventPoint.x && eventPoint.x <= baseRect.x + baseRect.width)
      res = Accept(editor, FLAGS_NONE, node, offset);
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
msiInputboxCaret::Accept(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  nsresult res(NS_ERROR_FAILURE);
  NS_ASSERTION(flags & FROM_LEFT|FROM_RIGHT, "Accept called without From left or from right");
  if (node && m_mathmlNode)
  {
    nsCOMPtr<nsIDOMNode> child;
    msiUtils::GetChildNode(m_mathmlNode, 0, child);  // get text node child.
    *node = child;
    *offset = 1;
    NS_ADDREF(*node);
    res = NS_OK;
  }  
  return res;
}

NS_IMETHODIMP 
msiInputboxCaret::GetSelectableMathFragment(nsIEditor  *editor, 
                                            nsIDOMNode *start,      PRUint32 startOffset, 
                                            nsIDOMNode *end,        PRUint32 endOffset, 
                                            nsIDOMNode **fragStart, PRUint32 *fragStartOffset,
                                            nsIDOMNode **fragEnd,   PRUint32 *fragEndOffset)
{
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
  nsCOMPtr<msiIMathMLCaret> parent;
  nsresult res = msiUtils::SetupPassOffCaretToParent(editor, m_mathmlNode, PR_FALSE, parent);
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
  return res;                                            
}

NS_IMETHODIMP
msiInputboxCaret::AdjustSelectionPoint(nsIEditor *editor, PRBool leftSelPoint, 
                                          nsIDOMNode** selectionNode, PRUint32 * selectionOffset,
                                          msiIMathMLCaret ** parent)
{
  return msiMCaretBase::AdjustSelectionPoint(editor, leftSelPoint, selectionNode, selectionOffset, parent);
}

NS_IMETHODIMP
msiInputboxCaret::SplitAtDecendents(nsIEditor* editor, 
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
msiInputboxCaret::Split(nsIEditor *editor, 
                        nsIDOMNode *appendLeft, 
                        nsIDOMNode *appendRight, 
                        nsIDOMNode **left, 
                        nsIDOMNode **right)
{
  return msiMCaretBase::Split(editor, appendLeft, appendRight, left, right);
}

NS_IMETHODIMP
msiInputboxCaret::SetDeletionTransaction(nsIEditor * editor,
                                      PRBool deletingToTheRight, 
                                      nsITransaction ** txn,
                                      PRBool * toRightInParent)
{

  if (!editor || !m_mathmlNode || !txn || !toRightInParent)
    return NS_ERROR_NULL_POINTER;
  *txn = nsnull;
  *toRightInParent = deletingToTheRight;
  return NS_OK;
}   

NS_IMETHODIMP
msiInputboxCaret::SetupDeletionTransactions(nsIEditor * editor,
                                            nsIDOMNode * start,
                                            PRUint32 startOffset,
                                            nsIDOMNode * end,
                                            PRUint32 endOffset,
                                            nsIArray ** transactionList,
                                            nsIDOMNode ** coalesceNode,
                                            PRUint32 * coalesceOffset)
{
  NS_ASSERTION(PR_FALSE, "Yuck\n");
  return msiMCaretBase::InputboxSetupDelTxns(editor, m_mathmlNode, m_numKids, start, startOffset,
                                             end, endOffset, transactionList, coalesceNode, coalesceOffset);
}


NS_IMETHODIMP
msiInputboxCaret::SetupCoalesceTransactions(nsIEditor * editor,
                                               nsIArray ** coalesceTransactions)
{
  return msiMCaretBase:: SetupCoalesceTransactions(editor, coalesceTransactions);
}                 

NS_IMETHODIMP
msiInputboxCaret::SetCoalTransactionsAndNode(nsIEditor * editor,
                                            PRBool onLeft,
                                            nsIArray ** transactionList,
                                            nsIDOMNode **coalesceNode)
{
  return msiMCaretBase::SetCoalTransactionsAndNode(editor, onLeft, transactionList, coalesceNode);
}                                         

NS_IMETHODIMP
msiInputboxCaret::CaretLeft(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  nsresult res(NS_ERROR_FAILURE);
  NS_ASSERTION(m_mathmlNode && editor, "m_mathmlNode or editor is null");
  if (m_mathmlNode && editor )
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
msiInputboxCaret::CaretRight(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  nsresult res(NS_ERROR_FAILURE);
  NS_ASSERTION(m_mathmlNode && editor, "m_mathmlNode or editor is null");
  if (m_mathmlNode && editor)
  {
    flags = FROM_CHILD;
    nsCOMPtr<msiIMathMLCaret> mathmlEditing;
    msiUtils::SetupPassOffCaretToParent(editor, m_mathmlNode, PR_TRUE, mathmlEditing);
    NS_ASSERTION(mathmlEditing, "Parent's mathmlEditing interface is null");
    if (mathmlEditing)
      res = mathmlEditing->CaretRight(editor, flags, node, offset);
  }
  return res;
}

NS_IMETHODIMP
msiInputboxCaret::CaretUp(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return msiMCaretBase::CaretUp(editor, flags, node, offset);
}

NS_IMETHODIMP
msiInputboxCaret::CaretDown(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return msiMCaretBase::CaretDown(editor, flags, node, offset);
}

NS_IMETHODIMP
msiInputboxCaret::CaretObjectLeft(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  nsresult res(NS_ERROR_FAILURE);
  NS_ASSERTION(m_mathmlNode && editor, "m_mathmlNode or editor is null");
  if (m_mathmlNode && editor )
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
msiInputboxCaret::CaretObjectRight(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  nsresult res(NS_ERROR_FAILURE);
  NS_ASSERTION(m_mathmlNode && editor, "m_mathmlNode or editor is null");
  if (m_mathmlNode && editor )
  {
    nsCOMPtr<msiIMathMLCaret> mathmlEditing;
    flags = FROM_CHILD;
    msiUtils::SetupPassOffCaretToParent(editor, m_mathmlNode, PR_FALSE, mathmlEditing);
    NS_ASSERTION(mathmlEditing, "Parent's mathmlEditing interface is null");
    if (mathmlEditing)
      res = mathmlEditing->CaretObjectRight(editor, flags, node, offset);
  }
  return res;   
}

NS_IMETHODIMP
msiInputboxCaret::CaretObjectUp(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return CaretLeft(editor, flags, node, offset);
}

NS_IMETHODIMP
msiInputboxCaret::CaretObjectDown(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return CaretRight(editor, flags, node, offset);
}

