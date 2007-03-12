// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.
#include "msiWhitespaceCaret.h"
#include "msiUtils.h"
#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "nsIDOMText.h"
#include "nsIDOMNamedNodeMap.h"
#include "nsIDOMAttr.h"
#include "nsIEditor.h"

msiWhitespaceCaret::msiWhitespaceCaret(nsIDOMNode* mathmlNode, PRUint32 offset)
:  msiMCaretBase(mathmlNode, offset, MSI_WHITESPACE)
{
}

NS_IMETHODIMP
msiWhitespaceCaret::PrepareForCaret(nsIEditor* editor)
{
  return NS_OK;
}

NS_IMETHODIMP
msiWhitespaceCaret::Inquiry(nsIEditor* editor, PRUint32 inquiryID, PRBool *result)
{
  if (inquiryID == AT_RIGHT_EQUIV_TO_0 || inquiryID == AT_LEFT_EQUIV_TO_RIGHT_MOST) 
    *result = PR_TRUE;
  else
    *result = PR_FALSE;
  return NS_OK;
}

NS_IMETHODIMP
msiWhitespaceCaret::AdjustNodeAndOffsetFromMouseEvent(nsIEditor *editor, nsIPresShell *presShell,
                                                       PRUint32 flags, 
                                                       nsIDOMMouseEvent *mouseEvent, 
                                                       nsIDOMNode **node, 
                                                       PRUint32 *offset)
{
  nsresult res(NS_OK);
  return res;
}                                                       
                                      

NS_IMETHODIMP
msiWhitespaceCaret::Accept(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  nsresult res(NS_OK);
  return res;
}

NS_IMETHODIMP 
msiWhitespaceCaret::GetSelectableMathFragment(nsIEditor  *editor, 
                                              nsIDOMNode *start,      PRUint32 startOffset, 
                                              nsIDOMNode *end,        PRUint32 endOffset, 
                                              nsIDOMNode **fragStart, PRUint32 *fragStartOffset,
                                              nsIDOMNode **fragEnd,   PRUint32 *fragEndOffset)
{
  return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP
msiWhitespaceCaret::AdjustSelectionPoint(nsIEditor *editor, PRBool leftSelPoint, 
                                      nsIDOMNode** selectionNode, PRUint32 * selectionOffset,
                                      msiIMathMLCaret** parentCaret)
{                                      
  
  return NS_OK;
}

NS_IMETHODIMP 
msiWhitespaceCaret::SplitAtDecendents(nsIEditor* editor, 
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
msiWhitespaceCaret::Split(nsIEditor *editor, 
                              nsIDOMNode *appendLeft, 
                              nsIDOMNode *appendRight, 
                              nsIDOMNode **left, 
                              nsIDOMNode **right)
{
  return msiMCaretBase::Split(editor, appendLeft, appendRight, left, right);
} 

NS_IMETHODIMP
msiWhitespaceCaret::SetDeletionTransaction(nsIEditor * editor,
                                               PRBool deletingToTheRight, 
                                               nsITransaction ** txn,
                                               PRBool * toRightInParent)
{
  return msiMCaretBase::SetDeletionTransaction(editor, deletingToTheRight, txn, toRightInParent);
}                                            

NS_IMETHODIMP
msiWhitespaceCaret::SetupDeletionTransactions(nsIEditor * editor,
                                              nsIDOMNode * start,
                                              PRUint32 startOffset,
                                              nsIDOMNode * end,
                                              PRUint32 endOffset,
                                              nsIArray ** transactionList,
                                              nsIDOMNode ** coalesceNode,
                                              PRUint32 * coalesceOffset)
{
  return msiMCaretBase::SetupDeletionTransactions(editor, start, startOffset, 
                                                  end, endOffset, transactionList,
                                                  coalesceNode, coalesceOffset);
}
                                  
NS_IMETHODIMP
msiWhitespaceCaret::SetupCoalesceTransactions(nsIEditor * editor,
                                              nsIArray ** coalesceTransactions)
{
  return msiMCaretBase::SetupCoalesceTransactions(editor, coalesceTransactions);
}  

NS_IMETHODIMP
msiWhitespaceCaret::SetCoalTransactionsAndNode(nsIEditor * editor,
                                              PRBool onLeft,
                                              nsIArray ** transactionList,
                                              nsIDOMNode **coalesceNode)
{
  return msiMCaretBase::SetCoalTransactionsAndNode(editor, onLeft, transactionList, coalesceNode);
}                                         


NS_IMETHODIMP
msiWhitespaceCaret::CaretLeft(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  if (!node || !offset || !m_mathmlNode || !editor)
    return NS_ERROR_FAILURE;
  nsresult res(NS_OK);
  nsCOMPtr<msiIMathMLCaret> mathmlEditing;
  msiUtils::SetupPassOffCaretToParent(editor, m_mathmlNode, PR_FALSE, mathmlEditing);
  if (mathmlEditing)
    res = mathmlEditing->CaretLeft(editor, flags, node, offset); 
  else 
    res = NS_ERROR_FAILURE;
  return res;  
}

NS_IMETHODIMP
msiWhitespaceCaret::CaretRight(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  if (!node || !offset || !m_mathmlNode || !editor)
    return NS_ERROR_FAILURE;
  nsresult res(NS_OK);
  nsCOMPtr<msiIMathMLCaret> mathmlEditing;
  msiUtils::SetupPassOffCaretToParent(editor, m_mathmlNode, PR_FALSE, mathmlEditing);
  if (mathmlEditing)
    res = mathmlEditing->CaretRight(editor, flags, node, offset); 
  else 
    res = NS_ERROR_FAILURE;
  return res;  
}

NS_IMETHODIMP
msiWhitespaceCaret::CaretUp(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return msiMCaretBase::CaretUp(editor, flags, node, offset);
}

NS_IMETHODIMP
msiWhitespaceCaret::CaretDown(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return msiMCaretBase::CaretDown(editor, flags, node, offset);
}

NS_IMETHODIMP
msiWhitespaceCaret::CaretObjectLeft(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  if (!node || !offset || !m_mathmlNode || !editor)
    return NS_ERROR_FAILURE;
  nsresult res(NS_OK);
  nsCOMPtr<msiIMathMLCaret> mathmlEditing;
  msiUtils::SetupPassOffCaretToParent(editor, m_mathmlNode, PR_FALSE, mathmlEditing);
  if (mathmlEditing)
    res = mathmlEditing->CaretObjectLeft(editor, flags, node, offset); 
  else 
    res = NS_ERROR_FAILURE;
  return res;  
}

NS_IMETHODIMP
msiWhitespaceCaret::CaretObjectRight(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  if (!node || !offset || !m_mathmlNode || !editor)
    return NS_ERROR_FAILURE;
  nsresult res(NS_OK);
  nsCOMPtr<msiIMathMLCaret> mathmlEditing;
  msiUtils::SetupPassOffCaretToParent(editor, m_mathmlNode, PR_FALSE, mathmlEditing);
  if (mathmlEditing)
    res = mathmlEditing->CaretObjectRight(editor, flags, node, offset); 
  else 
    res = NS_ERROR_FAILURE;
  return res;  
}

NS_IMETHODIMP
msiWhitespaceCaret::CaretObjectUp(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return CaretLeft(editor, flags, node, offset);
}

NS_IMETHODIMP
msiWhitespaceCaret::CaretObjectDown(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return CaretRight(editor, flags, node, offset);
}

NS_IMETHODIMP
msiWhitespaceCaret::TabLeft(nsIEditor *editor, nsIDOMNode ** node, PRUint32 *offset)
{
  return msiMCaretBase::TabLeft(editor, node, offset);
}

NS_IMETHODIMP
msiWhitespaceCaret::TabRight(nsIEditor *editor, nsIDOMNode ** node, PRUint32 *offset)
{
  return msiMCaretBase::TabRight(editor, node, offset);
}
