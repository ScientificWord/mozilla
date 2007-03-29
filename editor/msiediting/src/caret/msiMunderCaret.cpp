// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiMunderCaret.h"
#include "msiUtils.h"
#include "nsIDOMNode.h"
#include "nsIEditor.h"
#include "msiIMathMLEditor.h"
#include "nsComponentManagerUtils.h"

msiMunderCaret::msiMunderCaret(nsIDOMNode* mathmlNode, PRUint32 offset)
:  msiUnderAndOrOverCaret(mathmlNode, offset, MATHML_MUNDER)
{
}
  
NS_IMETHODIMP
msiMunderCaret::PrepareForCaret(nsIEditor* editor)
{
  return msiUnderAndOrOverCaret::PrepareForCaret(editor);
}

NS_IMETHODIMP
msiMunderCaret::Inquiry(nsIEditor* editor, PRUint32 inquiryID, PRBool *result)
{
  return msiUnderAndOrOverCaret::Inquiry(editor, inquiryID, result);
}

NS_IMETHODIMP
msiMunderCaret::AdjustNodeAndOffsetFromMouseEvent(nsIEditor *editor, nsIPresShell *presShell,
                                                PRUint32 flags, 
                                                nsIDOMMouseEvent *mouseEvent, 
                                                nsIDOMNode **node, 
                                                PRUint32 *offset)
{
  return msiUnderAndOrOverCaret::AdjustNodeAndOffsetFromMouseEvent(editor, presShell, flags, 
                                                           mouseEvent, node, offset);
}                                                       

NS_IMETHODIMP
msiMunderCaret::Accept(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return msiUnderAndOrOverCaret::Accept(editor, flags, node, offset);
}

NS_IMETHODIMP 
msiMunderCaret::GetSelectableMathFragment(nsIEditor  *editor, 
                                         nsIDOMNode *start,      PRUint32 startOffset, 
                                         nsIDOMNode *end,        PRUint32 endOffset, 
                                         nsIDOMNode **fragStart, PRUint32 *fragStartOffset,
                                         nsIDOMNode **fragEnd,   PRUint32 *fragEndOffset)
{
  return msiUnderAndOrOverCaret::GetSelectableMathFragment(editor, 
                                                   start,     startOffset, 
                                                   end,       endOffset, 
                                                   fragStart, fragStartOffset,
                                                   fragEnd,   fragEndOffset);
}

NS_IMETHODIMP
msiMunderCaret::AdjustSelectionPoint(nsIEditor *editor, PRBool leftSelPoint, 
                                      nsIDOMNode** selectionNode, PRUint32 * selectionOffset,
                                      msiIMathMLCaret** parentCaret)
{
  return msiUnderAndOrOverCaret::AdjustSelectionPoint(editor, leftSelPoint, 
                                              selectionNode, selectionOffset,
                                              parentCaret);
}

NS_IMETHODIMP 
msiMunderCaret::SplitAtDecendents(nsIEditor* editor, 
                                  nsIDOMNode *leftDecendent, PRUint32 leftOffset, 
                                  nsIDOMNode *rightDecendent, PRUint32 rightOffset, 
                                  PRUint32 *mmlNodeLeftOffset, PRUint32 *mmlNodeRightOffset, 
                                  nsIDOMNode **left_leftPart, nsIDOMNode **left_rightPart, 
                                  nsIDOMNode **right_leftPart, nsIDOMNode **right_rightPart)
{
  return msiUnderAndOrOverCaret::SplitAtDecendents(editor, leftDecendent, leftOffset, rightDecendent, rightOffset, 
                                           mmlNodeLeftOffset, mmlNodeRightOffset, 
                                           left_leftPart, left_rightPart, 
                                           right_leftPart, right_rightPart);
}

NS_IMETHODIMP
msiMunderCaret::Split(nsIEditor *editor, 
                      nsIDOMNode *appendLeft, 
                      nsIDOMNode *prependRight, 
                      nsIDOMNode **left, 
                      nsIDOMNode **right)
{
  return msiUnderAndOrOverCaret::Split(editor, appendLeft, prependRight, left, right);
}  

NS_IMETHODIMP
msiMunderCaret::SetDeletionTransaction(nsIEditor * editor,
                                       PRBool deletingToTheRight, 
                                       nsITransaction ** txn,
                                       PRBool * toRightInParent)
{             
  return msiUnderAndOrOverCaret::SetDeletionTransaction(editor, deletingToTheRight, txn, toRightInParent);
}

NS_IMETHODIMP
msiMunderCaret::SetupDeletionTransactions(nsIEditor * editor,
                                          nsIDOMNode * start,
                                          PRUint32 startOffset,
                                          nsIDOMNode * end,
                                          PRUint32 endOffset,
                                          nsIArray ** transactionList,
                                          nsIDOMNode ** coalesceNode,
                                          PRUint32 * coalesceOffset)
{
  return msiUnderAndOrOverCaret::SetupDeletionTransactions(editor,
                                                   start,
                                                   startOffset,
                                                   end,
                                                   endOffset,
                                                   transactionList,
                                                   coalesceNode,
                                                   coalesceOffset);
}

NS_IMETHODIMP
msiMunderCaret::SetupCoalesceTransactions(nsIEditor * editor,
                                     nsIArray ** coalesceTransactions)
{
  return msiUnderAndOrOverCaret::SetupCoalesceTransactions(editor, coalesceTransactions);
}         

NS_IMETHODIMP
msiMunderCaret::SetCoalTransactionsAndNode(nsIEditor * editor,
                                           PRBool onLeft,
                                           nsIArray ** transactionList,
                                           nsIDOMNode **coalesceNode)
{
  return msiUnderAndOrOverCaret::SetCoalTransactionsAndNode(editor, onLeft, transactionList, coalesceNode);
}

NS_IMETHODIMP
msiMunderCaret::CaretLeft(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return msiUnderAndOrOverCaret::CaretLeft(editor, flags, node, offset);
}

NS_IMETHODIMP
msiMunderCaret::CaretRight(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return msiUnderAndOrOverCaret::CaretRight(editor, flags, node, offset);
}

NS_IMETHODIMP
msiMunderCaret::CaretUp(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return msiUnderAndOrOverCaret::CaretUp(editor,flags,node,offset);
}

NS_IMETHODIMP
msiMunderCaret::CaretDown(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return msiUnderAndOrOverCaret::CaretDown(editor,flags,node,offset);
}

NS_IMETHODIMP
msiMunderCaret::CaretObjectLeft(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return msiUnderAndOrOverCaret::CaretObjectLeft(editor, flags, node, offset);
}

NS_IMETHODIMP
msiMunderCaret::CaretObjectRight(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return msiUnderAndOrOverCaret::CaretObjectRight(editor, flags, node, offset);
}

NS_IMETHODIMP
msiMunderCaret::CaretObjectUp(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return msiUnderAndOrOverCaret::CaretObjectUp(editor, flags, node, offset);
}

NS_IMETHODIMP
msiMunderCaret::CaretObjectDown(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return msiUnderAndOrOverCaret::CaretObjectDown(editor, flags, node, offset);
}

NS_IMETHODIMP
msiMunderCaret::TabLeft(nsIEditor *editor, nsIDOMNode ** node, PRUint32 *offset)
{
  return TabRight(editor, node, offset);
}

NS_IMETHODIMP
msiMunderCaret::TabRight(nsIEditor *editor, nsIDOMNode ** node, PRUint32 *offset)
{
  if (m_numKids != 2 || m_offset == 0)
    return msiMCaretBase::TabRight(editor, node, offset);

  nsresult res;
  PRUint32 flags(msiIMathMLInsertion::FLAGS_NONE);
  nsCOMPtr<nsIDOMNode> base, subscript;
  msiUtils::GetChildNode(m_mathmlNode, 0, base);
  msiUtils::GetChildNode(m_mathmlNode, 1, subscript);
  nsCOMPtr<nsIDOMNode> cloneBase, cloneSub;
  res = msiUtils::CloneNode(base, cloneBase);
  if (NS_FAILED(res))
    return res;
  res = msiUtils::CloneNode(subscript, cloneSub);
  if (NS_FAILED(res))
    return res;
  nsAutoString emptyString;
  nsCOMPtr<nsIDOMElement> msubsupElement;
  res = msiUtils::CreateMunderover(editor, cloneBase, cloneSub, nsnull,
                                   PR_FALSE, PR_TRUE, PR_TRUE,
                                   flags, emptyString, emptyString, msubsupElement);
  if (NS_FAILED(res))
    return res;
  nsCOMPtr<nsIDOMNode> parent;
  m_mathmlNode->GetParentNode(getter_AddRefs(parent));
  nsCOMPtr<nsIDOMNode> msubsupNode(do_QueryInterface(msubsupElement));
  if (msubsupNode && parent)
  {
    nsCOMPtr<nsIDOMNode> dontcare;
    res = parent->ReplaceChild(msubsupNode, m_mathmlNode, getter_AddRefs(dontcare));
  } 
  else
  {
    res = NS_ERROR_FAILURE;  //maybe NS_ERROR_DOM_NOT_FOUND_ERR?
  }
  if (NS_FAILED(res))
    return res;
  //give the caret to the subscript
  nsCOMPtr<nsIDOMNode> superscript;
  msiUtils::GetChildNode(msubsupNode, 2, superscript);
  nsCOMPtr<msiIMathMLCaret> mathmlCaret;
  msiUtils::GetMathMLCaretInterface(editor, superscript, 0, mathmlCaret);
  if (mathmlCaret)
    res = mathmlCaret->Accept(editor, FROM_PARENT|FROM_LEFT, node, offset); 
  else 
    res = NS_ERROR_FAILURE;
  return res;
}
