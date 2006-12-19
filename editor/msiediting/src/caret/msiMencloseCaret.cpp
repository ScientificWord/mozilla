// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiMencloseCaret.h"
#include "nsIDOMNode.h"
#include "msiUtils.h"
#include "nsCOMPtr.h"
#include "nsIEditor.h"
#include "nsISelection.h"
#include "nsIFrame.h"
#include "nsIView.h"
#include "nsPresContext.h"
#include "nsIDOMMouseEvent.h"
#include "nsIPrivateDOMEvent.h"

msiMencloseCaret::msiMencloseCaret(nsIDOMNode* mathmlNode, PRUint32 offset)
:  msiMContainerCaret(mathmlNode, offset, MATHML_MENCLOSE)
{
}

msiMencloseCaret::msiMencloseCaret(nsIDOMNode* mathmlNode, PRUint32 offset, PRUint32 mathmlType)
:  msiMContainerCaret(mathmlNode, offset, mathmlType)
{
}

NS_IMETHODIMP
msiMencloseCaret::AdjustNodeAndOffsetFromMouseEvent(nsIEditor *editor, nsIPresShell *presShell,
                                                       PRUint32 flags, 
                                                       nsIDOMMouseEvent *mouseEvent, 
                                                       nsIDOMNode **node, 
                                                       PRUint32 *offset)
{
  if (!editor || !node || !offset || !presShell || !m_mathmlNode || !mouseEvent)
    return NS_ERROR_FAILURE;
  nsIFrame * encloseFrame = nsnull; // no smart pointers for frames.
  nsIFrame * firstKid = nsnull;
  nsIFrame * lastKid = nsnull;
  nsRect encloseRect, firstKidRect, lastKidRect;
  nsPoint eventPoint(0,0);
  *node = nsnull;
  *offset = INVALID;
  nsresult res = msiMCaretBase::GetPrimaryFrameForNode(presShell, m_mathmlNode, &encloseFrame);
  if (NS_SUCCEEDED(res) && encloseFrame)
  {  
    encloseRect = encloseFrame->GetScreenRectExternal();
    firstKid = encloseFrame->GetFirstChild(nsnull);
    if (firstKid)
    {
      firstKidRect = firstKid->GetScreenRectExternal();
      lastKid = firstKid;
      while (lastKid->GetNextSibling())
        lastKid = lastKid->GetNextSibling();
      lastKidRect = lastKid->GetScreenRectExternal();
    }
    else
      res = NS_ERROR_FAILURE;
  }
  if (NS_SUCCEEDED(res))
    res = msiUtils::GetScreenPointFromMouseEvent(mouseEvent, eventPoint);
  if (NS_SUCCEEDED(res))
  {
    PRInt32 lfThres(0), rtThres(0);
    GetThresholds(encloseRect, firstKidRect, lastKidRect, lfThres, rtThres);
    if (encloseRect.x + lfThres <= eventPoint.x && eventPoint.x <= eventPoint.x + encloseRect.width - rtThres)
      res = msiMContainerCaret::AdjustNodeAndOffsetFromMouseEvent(editor, presShell, flags, 
                                                                   mouseEvent, node, offset);
    else
    {
      nsCOMPtr<msiIMathMLCaret> mathmlEditing;
      PRBool incOffset = encloseRect.x + lfThres <= eventPoint.x;
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
msiMencloseCaret::AdjustSelectionPoint(nsIEditor *editor, PRBool leftSelPoint, 
                                       nsIDOMNode** selectionNode, PRUint32 * selectionOffset,
                                       msiIMathMLCaret** parentCaret)
{
  return msiMCaretBase::AdjustSelectionPoint(editor, leftSelPoint, selectionNode, selectionOffset, parentCaret);
}


NS_IMETHODIMP
msiMencloseCaret::SetDeletionTransaction(nsIEditor * editor,
                                         PRBool deletingToTheRight, 
                                         nsITransaction ** txn,
                                         PRBool * toRightInParent)
{

  NS_ASSERTION(PR_FALSE, "Should not be here since 0th and last offsets are inside the enclose element\n");
  return msiMCaretBase::SetDeletionTransaction(editor, deletingToTheRight, txn, toRightInParent);
}                                      

NS_IMETHODIMP
msiMencloseCaret::SetupDeletionTransactions(nsIEditor * editor,
                                            nsIDOMNode * start,
                                            PRUint32 startOffset,
                                            nsIDOMNode * end,
                                            PRUint32 endOffset,
                                            nsIArray ** transactionList,
                                            nsIDOMNode ** coalesceNode,
                                            PRUint32 * coalesceOffset)
{
  return msiMCaretBase::InputboxSetupDelTxns(editor, m_mathmlNode, m_numKids, start, startOffset,
                                             end, endOffset, transactionList, coalesceNode, coalesceOffset);
}                                                     


#define MIN_THRESHOLD 2 //TODO -- how should this be determined.      
void msiMencloseCaret::GetThresholds(const nsRect &encloseRect, 
                                     const nsRect &firstKidRect, const nsRect &lastKidRect, 
                                     PRInt32 &left, PRInt32 & right)
{
  PRInt32 min(MIN_THRESHOLD);
  left = (firstKidRect.x- encloseRect.x)/2;
  right = encloseRect.x + encloseRect.width - lastKidRect.x - lastKidRect.width;
  if (left < min)
    left = min;
  if (right < min)
    right = min;
}                                      

NS_IMETHODIMP
msiMencloseCaret::CaretObjectLeft(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  if (!node || !offset || !m_mathmlNode || !editor)
    return NS_ERROR_FAILURE;
  nsresult res(NS_OK);
  if (m_offset == 0)
  {
    nsCOMPtr<msiIMathMLCaret> mathmlEditing;
    msiUtils::SetupPassOffCaretToParent(editor, m_mathmlNode, PR_FALSE, mathmlEditing);
    if (mathmlEditing)
    {
      flags = FROM_CHILD;
      res = mathmlEditing->CaretObjectLeft(editor, flags, node, offset); 
    }
    else 
      res = NS_ERROR_FAILURE;
  }
  else if (m_offset == m_numKids)
  { 
    nsCOMPtr<msiIMathMLCaret> mathmlEditing;
    msiUtils::SetupPassOffCaretToParent(editor, m_mathmlNode, PR_FALSE, mathmlEditing);
    if (mathmlEditing)
      res = mathmlEditing->Accept(editor, FROM_CHILD|FROM_RIGHT, node, offset); 
    else 
      res = NS_ERROR_FAILURE;
  }
  else
    res = msiMContainerCaret::CaretObjectLeft(editor, flags, node, offset);
  return res;
}

NS_IMETHODIMP
msiMencloseCaret::CaretObjectRight(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  if (!node || !offset || !m_mathmlNode || !editor)
    return NS_ERROR_FAILURE;
  nsresult res(NS_OK);
  if (m_offset == m_numKids)
  {
    nsCOMPtr<msiIMathMLCaret> mathmlEditing;
    msiUtils::SetupPassOffCaretToParent(editor, m_mathmlNode, PR_TRUE, mathmlEditing);
    if (mathmlEditing)
    {
      flags = FROM_CHILD;
      res = mathmlEditing->CaretObjectRight(editor, flags, node, offset); 
    }
    else 
      res = NS_ERROR_FAILURE;
  }
  else if (m_offset == 0)
  { 
    nsCOMPtr<msiIMathMLCaret> mathmlEditing;
    msiUtils::SetupPassOffCaretToParent(editor, m_mathmlNode, PR_TRUE, mathmlEditing);
    if (mathmlEditing)
      res = mathmlEditing->Accept(editor, FROM_CHILD|FROM_LEFT, node, offset); 
    else 
      res = NS_ERROR_FAILURE;
  }
  else
    res = msiMContainerCaret::CaretObjectRight(editor, flags, node, offset);
  return res;
}
