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
msiMencloseCaret::GetNodeAndOffsetFromMouseEvent(nsIEditor *editor, nsIPresShell *presShell, 
                                                 PRUint32 flags, nsIDOMMouseEvent * mouseEvent,
                                                 nsIDOMNode **node, PRUint32 *offset)
{
  if (!editor || !node || !offset || !presShell || !m_mathmlNode || !mouseEvent)
    return NS_ERROR_FAILURE;
  nsresult res(NS_OK);
  nsIFrame * encloseFrame = nsnull; // no smart pointers for frames.
  nsIFrame * firstKid = nsnull;
  nsIFrame * lastKid = nsnull;
  nsRect encloseRect, firstKidRect, lastKidRect;
  nsPoint offsetPoint(0,0), eventPoint(0,0);
  *node = nsnull;
  *offset = INVALID;
  res = msiMCaretBase::GetPrimaryFrameForNode(presShell, m_mathmlNode, &encloseFrame);
  if (NS_SUCCEEDED(res) && encloseFrame)
  {  
    encloseRect = encloseFrame->GetRect();
    firstKid = encloseFrame->GetFirstChild(nsnull);
    if (firstKid)
    {
      firstKidRect = firstKid->GetRect();
      lastKid = firstKid;
      while (lastKid->GetNextSibling())
        lastKid = lastKid->GetNextSibling();
      lastKidRect = lastKid->GetRect();
    }
    else
      res = NS_ERROR_FAILURE;
  }
  if (NS_SUCCEEDED(res))
    res = msiMCaretBase::GetFrameOffsetFromView(encloseFrame, offsetPoint);
  if (NS_SUCCEEDED(res))
    res = msiUtils::GetPointFromMouseEvent(mouseEvent, eventPoint);                                     
  if (NS_SUCCEEDED(res))
  {
    PRInt32 eventX = eventPoint.x - offsetPoint.x; //relative to sqrtFrame's rect
    PRInt32 lfThres(0), rtThres(0);
    GetThresholds(presShell, encloseRect, firstKidRect, lastKidRect, lfThres, rtThres);
    if (!(flags&FROM_PARENT) && ((eventX <= lfThres) || (encloseRect.width - eventX <= rtThres))) 
    { 
      //ask parent to accept caret.
      PRBool incOffset = eventX <= lfThres ? PR_FALSE : PR_TRUE;
      flags = FROM_CHILD;
      flags |= (eventX <= lfThres) ? FROM_RIGHT : FROM_LEFT;
      nsCOMPtr<msiIMathMLCaret> parent;
      msiUtils::SetupPassOffCaretToParent(editor, m_mathmlNode, incOffset, parent);
      if (parent)
        res = parent->Accept(editor, flags, node, offset);
    }
    if (*node == nsnull)
      res = msiMContainerCaret::GetNodeAndOffsetFromMouseEvent(editor, presShell, flags, 
                                                               mouseEvent, node, offset);
    if (*node == nsnull)
    {
      NS_ASSERTION(PR_FALSE, "Container refused to set node and offset.");
      *node = m_mathmlNode;
      NS_ADDREF(*node);
      *offset = 0;
      res = NS_OK;
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
msiMencloseCaret::SetupDeletionTransactions(nsIEditor * editor,
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
  { // replace contents of enclosed with input box.
    nsCOMPtr<nsIDOMElement> inputbox;
    PRUint32 flags(msiIMathMLInsertion::FLAGS_NONE);
    res = msiUtils::CreateInputbox(editor, PR_FALSE, PR_FALSE, flags, inputbox);
    nsCOMPtr<nsIDOMNode> inputboxNode(do_QueryInterface(inputbox));
    if (NS_SUCCEEDED(res) && inputboxNode) 
    {
      res = msiMCaretBase::SetupDeletionTransactions(editor, startOffset, endOffset,
                                                     inputboxNode, nsnull, transactionList);
    }  
    else
      res = NS_ERROR_FAILURE;
  }
  else
    res = msiMCaretBase::SetupDeletionTransactions(editor, startOffset, endOffset,
                                                   start, end, transactionList);
  return res;
}                                                     
    
#define MIN_THRESHOLD 2 //TODO -- how should this be determined.      
void msiMencloseCaret::GetThresholds(nsIPresShell* shell, const nsRect &encloseRect, 
                                     const nsRect &firstKidRect, const nsRect &lastKidRect, 
                                     PRInt32 &left, PRInt32 & right)
{
  float p2t = 15.0;
  PRInt32 min(15);
  if (shell)
  {
    nsPresContext * context = shell->GetPresContext();
    if (context)
      p2t = context->PixelsToTwips();
  }
  min = NS_STATIC_CAST(PRInt32, MIN_THRESHOLD * p2t);
  left = firstKidRect.x/2;
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
