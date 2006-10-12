// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.
#include "msiMrowCaret.h"
#include "msiMContainerCaret.h"
#include "nsIDOMNode.h"
#include "nsIDOMNodeList.h"
#include "nsCOMPtr.h"
#include "msiUtils.h"   
#include "msiMrowEditingImp.h"   

msiMrowCaret::msiMrowCaret(nsIDOMNode* mathmlNode, PRUint32 offset) 
:  msiMContainerCaret(mathmlNode, offset, MATHML_MROW)
{
  MSI_NewMrowEditingImp(mathmlNode, getter_AddRefs(m_mrowEditingImp));
}

// ISupports
NS_IMPL_ISUPPORTS_INHERITED1(msiMrowCaret, msiMContainerCaret, msiIMrowEditing)

NS_IMETHODIMP
msiMrowCaret::PrepareForCaret(nsIEditor* editor)
{
  return NS_OK;
}


NS_IMETHODIMP
msiMrowCaret::Inquiry(nsIEditor* editor, PRUint32 inquiryID, PRBool *result)
{
  if (inquiryID == AT_RIGHT_EQUIV_TO_0 || inquiryID == AT_LEFT_EQUIV_TO_RIGHT_MOST) 
    *result = PR_TRUE;
  else if (inquiryID == CAN_SELECT_CHILD_LEAF) 
    *result = PR_TRUE;
  else
    *result = PR_FALSE;
  return NS_OK;
}


NS_IMETHODIMP
msiMrowCaret::GetNodeAndOffsetFromMouseEvent(nsIEditor *editor, nsIPresShell *presShell, 
                                             PRUint32 flags, nsIDOMMouseEvent * mouseEvent,
                                             nsIDOMNode **node, PRUint32 *offset)
{
   return msiMContainerCaret::GetNodeAndOffsetFromMouseEvent(editor, presShell, flags,
                                                             mouseEvent, node, offset);
} 

NS_IMETHODIMP
msiMrowCaret::AdjustNodeAndOffsetFromMouseEvent(nsIEditor *editor, nsIPresShell *presShell,
                                                       PRUint32 flags, 
                                                       nsIDOMMouseEvent *mouseEvent, 
                                                       nsIDOMNode **node, 
                                                       PRUint32 *offset)
{
  return msiMContainerCaret::AdjustNodeAndOffsetFromMouseEvent(editor, presShell, flags, 
                                                               mouseEvent, node, offset);
}                                                       
     
                                   
NS_IMETHODIMP
msiMrowCaret::Accept(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return msiMContainerCaret::Accept(editor, flags, node, offset);
}

NS_IMETHODIMP 
msiMrowCaret::GetSelectableMathFragment(nsIEditor  *editor, 
                                         nsIDOMNode *start,      PRUint32 startOffset, 
                                         nsIDOMNode *end,        PRUint32 endOffset, 
                                         nsIDOMNode **fragStart, PRUint32 *fragStartOffset,
                                         nsIDOMNode **fragEnd,   PRUint32 *fragEndOffset)
{
  return msiMContainerCaret::GetSelectableMathFragment(editor, start, startOffset, 
                                                       end, endOffset, fragStart, fragStartOffset,
                                                       fragEnd, fragEndOffset);
}

NS_IMETHODIMP
msiMrowCaret::AdjustSelectionPoint(nsIEditor *editor, PRBool leftSelPoint, 
                                      nsIDOMNode** selectionNode, PRUint32 * selectionOffset,
                                      msiIMathMLCaret** parentCaret)
{
  return msiMContainerCaret::AdjustSelectionPoint(editor, leftSelPoint, selectionNode, selectionOffset, parentCaret);
}


NS_IMETHODIMP 
msiMrowCaret::SplitAtDecendents(nsIEditor* editor, 
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
msiMrowCaret::Split(nsIEditor * editor, 
                    nsIDOMNode *leftPart,  // leftPart and rightPart are the
                    nsIDOMNode *rightPart, // partition of the child node at m_offset
                    nsIDOMNode **left, 
                    nsIDOMNode **right)
{
  if (!m_mathmlNode || !left || !right)
    return NS_ERROR_NULL_POINTER;
  *left = nsnull;
  *right = nsnull;  
  nsCOMPtr<nsIDOMNode> lf, rt;
  nsresult res = msiUtils::SplitNode(m_mathmlNode, m_offset, PR_TRUE, lf, rt);
  if (NS_SUCCEEDED(res) && lf && rt)
  {
    if (leftPart)
    {
      nsCOMPtr<nsIDOMNode> dontcare;
      res = lf->AppendChild(leftPart, getter_AddRefs(dontcare));
    } 
    if (leftPart || rightPart)
    {
      nsCOMPtr<nsIDOMNode> first;
      rt->GetFirstChild(getter_AddRefs(first));
      nsCOMPtr<nsIDOMNode> dontcare;
      if (first && rightPart)
        res = rt->ReplaceChild(rightPart, first, getter_AddRefs(dontcare));
      else if (!first && rightPart)  
        res = rt->AppendChild(rightPart, getter_AddRefs(dontcare));
      else if (first && leftPart) /* && !rightPart */
        res = rt->RemoveChild(first, getter_AddRefs(dontcare));
    } 
    if (NS_SUCCEEDED(res))
    {
      PRBool hasChild(PR_FALSE);
      lf->HasChildNodes(&hasChild);
      if (hasChild)
      { 
        *left = lf;
        NS_ADDREF(*left);
      }
      hasChild = PR_FALSE;  
      rt->HasChildNodes(&hasChild);
      if (hasChild)
      { 
        *right = rt;
        NS_ADDREF(*right);
      }
    }
  }
  else
    res = NS_ERROR_FAILURE;
  return res;    
}                                     

NS_IMETHODIMP
msiMrowCaret::SetupDeletionTransactions(nsIEditor * editor,
                                        PRUint32 startOffset,
                                        PRUint32 endOffset,
                                        nsIDOMNode * start,
                                        nsIDOMNode * end,
                                        nsIArray ** transactionList)
{
  return msiMCaretBase::SetupDeletionTransactions(editor, startOffset, endOffset,
                                                  start, end, transactionList);
}



NS_IMETHODIMP
msiMrowCaret::CaretLeft(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  if (!node || !offset || !m_mathmlNode || !editor)
    return NS_ERROR_FAILURE;
  nsresult res(NS_OK);
  PRBool structural(PR_TRUE);
  IsStructural(&structural);
  if (!structural && (m_offset == 0))
  {
    nsCOMPtr<msiIMathMLCaret> mathmlEditing;
    msiUtils::SetupPassOffCaretToParent(editor, m_mathmlNode, PR_FALSE, mathmlEditing);
    flags = FROM_CHILD;
    if (mathmlEditing)
      res = mathmlEditing->CaretLeft(editor, flags, node, offset);
    else
      res = NS_ERROR_FAILURE;  
  }
  else 
    res = msiMContainerCaret::CaretLeft(editor, flags, node, offset);
  return res;
}

NS_IMETHODIMP
msiMrowCaret::CaretRight(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  if (!node || !offset || !m_mathmlNode || !editor)
    return NS_ERROR_FAILURE;
  nsresult res(NS_OK);
  PRBool structural(PR_TRUE);
  IsStructural(&structural);
  if (!structural && (m_offset == m_numKids))
  {
    nsCOMPtr<msiIMathMLCaret> mathmlEditing;
    msiUtils::SetupPassOffCaretToParent(editor, m_mathmlNode, PR_TRUE, mathmlEditing);
    flags = FROM_CHILD;
    if (mathmlEditing)
      res = mathmlEditing->CaretRight(editor, flags, node, offset);
    else
      res = NS_ERROR_FAILURE;  
  }
  else 
    res = msiMContainerCaret::CaretRight(editor, flags, node, offset);
  return res;
}

NS_IMETHODIMP
msiMrowCaret::CaretUp(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return CaretLeft(editor, flags, node, offset);
}

NS_IMETHODIMP
msiMrowCaret::CaretDown(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return CaretRight(editor, flags, node, offset);
}

NS_IMETHODIMP
msiMrowCaret::CaretObjectLeft(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  if (!node || !offset || !m_mathmlNode || !editor)
    return NS_ERROR_FAILURE;
  nsresult res(NS_OK);
  PRBool structural(PR_TRUE);
  IsStructural(&structural);
  if (!structural && (m_offset == 0))
  {
    nsCOMPtr<msiIMathMLCaret> mathmlEditing;
    msiUtils::SetupPassOffCaretToParent(editor, m_mathmlNode, PR_FALSE, mathmlEditing);
    flags = FROM_CHILD;
    if (mathmlEditing)
      res = mathmlEditing->CaretObjectLeft(editor, flags, node, offset);
    else
      res = NS_ERROR_FAILURE;  
  }
  else 
    res = msiMContainerCaret::CaretObjectLeft(editor, flags, node, offset);
  return res;
}

NS_IMETHODIMP
msiMrowCaret::CaretObjectRight(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  if (!node || !offset || !m_mathmlNode || !editor)
    return NS_ERROR_FAILURE;
  nsresult res(NS_OK);
  PRBool structural(PR_TRUE);
  IsStructural(&structural);
  if (!structural && (m_offset == m_numKids))
  {
    nsCOMPtr<msiIMathMLCaret> mathmlEditing;
    msiUtils::SetupPassOffCaretToParent(editor, m_mathmlNode, PR_TRUE, mathmlEditing);
    flags = FROM_CHILD;
    if (mathmlEditing)
      res = mathmlEditing->CaretObjectRight(editor, flags, node, offset);
    else
      res = NS_ERROR_FAILURE;  
  }
  else 
    res = msiMContainerCaret::CaretObjectRight(editor, flags, node, offset);
  return res;
}

NS_IMETHODIMP
msiMrowCaret::CaretObjectUp(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return CaretLeft(editor, flags, node, offset);
}

NS_IMETHODIMP
msiMrowCaret::CaretObjectDown(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return CaretRight(editor, flags, node, offset);
}
