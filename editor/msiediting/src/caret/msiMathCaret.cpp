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
msiMathCaret::SetupDeletionTransactions(nsIEditor * editor,
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
  nsresult res(NS_OK); 
  if (m_offset < m_numKids)
    res = msiMContainerCaret::CaretRight(editor, flags, node, offset); 
  else // leave math
  { 
    //TODO -- fix this hack.
    nsCOMPtr<nsIHTMLEditor> htmlEditor = do_QueryInterface(editor);
    nsCOMPtr<nsISelection> selection;
    editor->GetSelection(getter_AddRefs(selection));
    nsCOMPtr<nsIDOMElement> element = do_QueryInterface(m_mathmlNode);
    if (htmlEditor && element && selection)
    {
      res = htmlEditor->SetCaretAfterElement(element);
      nsCOMPtr<nsISelectionPrivate> selPriv(do_QueryInterface(selection));
      if (selPriv)
        selPriv->SetInterlinePosition(PR_TRUE);
    }
    else
    {
      PRUint32 newOffset(0);
      nsCOMPtr<nsIDOMNode> carrotNode;
      msiUtils::GetIndexOfChildInParent(m_mathmlNode, newOffset);
      m_mathmlNode->GetParentNode(getter_AddRefs(carrotNode));
      NS_ASSERTION(carrotNode, "Parent node is null");
      if (carrotNode && newOffset < LAST_VALID)
      {
        *node = carrotNode;
        NS_ADDREF(*node);
        *offset = newOffset+1;
        res = NS_OK;
      }
    }    
  }
  return res;  
}

NS_IMETHODIMP
msiMathCaret::CaretUp(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return CaretLeft(editor, flags, node, offset);
}

NS_IMETHODIMP
msiMathCaret::CaretDown(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset)
{
  return CaretRight(editor, flags, node, offset);
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
  nsresult res(NS_OK); 
  if (m_offset < m_numKids)
    res = msiMContainerCaret::CaretObjectRight(editor, flags, node, offset); 
  else // leave math
  { 
    //TODO -- fix this hack.
    nsCOMPtr<nsIHTMLEditor> htmlEditor = do_QueryInterface(editor);
    nsCOMPtr<nsISelection> selection;
    editor->GetSelection(getter_AddRefs(selection));
    nsCOMPtr<nsIDOMElement> element = do_QueryInterface(m_mathmlNode);
    if (htmlEditor && element && selection)
    {
      res = htmlEditor->SetCaretAfterElement(element);
      nsCOMPtr<nsISelectionPrivate> selPriv(do_QueryInterface(selection));
      if (selPriv)
        selPriv->SetInterlinePosition(PR_TRUE);
    }
    else
    {
      PRUint32 newOffset(0);
      nsCOMPtr<nsIDOMNode> carrotNode;
      msiUtils::GetIndexOfChildInParent(m_mathmlNode, newOffset);
      m_mathmlNode->GetParentNode(getter_AddRefs(carrotNode));
      NS_ASSERTION(carrotNode, "Parent node is null");
      if (carrotNode && newOffset < LAST_VALID)
      {
        *node = carrotNode;
        NS_ADDREF(*node);
        *offset = newOffset+1;
        res = NS_OK;
      }
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


