// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiMath.h"
#include "msiUtils.h"
#include "msiEditingAtoms.h"
#include "msiMContainer.h"
#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "nsIDOMElement.h"
#include "nsIDOMAttr.h"
#include "nsIDOMNodeList.h"
#include "nsString.h"

//TODO many absorb issues to understand and deal with

msiMath::msiMath(nsIDOMNode* mathmlNode, PRUint32 offset) : 
  msiMContainer(mathmlNode, offset, MATHML_MATH), m_isDisplay(PR_FALSE)
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

NS_IMETHODIMP msiMath::Inquiry(nsIEditor* editor,
                               PRUint32 inquiryID, 
                               PRBool * result)
{
  if (inquiryID == INSERT_DISPLAY || 
      inquiryID == INSERT_INLINE_MATH)
    *result = PR_TRUE;
  else if (inquiryID == IS_MROW_REDUNDANT)
    *result = msiUtils::MROW_PURGE_NONE == msiUtils::GetMrowPurgeMode() ? PR_FALSE : PR_TRUE; 
  else
    *result = PR_TRUE;
  return NS_OK;
}                                                 


NS_IMETHODIMP
msiMath::InsertMath(nsIEditor *editor,
                    nsISelection   *selection, 
                    PRBool isDisplay,
                    nsIDOMNode * inleft,
                    nsIDOMNode * inright,
                    PRUint32 flags)
{
  nsresult res(NS_ERROR_FAILURE);
  if (editor && m_mathmlNode)
  {
    PRUint32 numKids(0);
    msiUtils::GetNumberofChildren(m_mathmlNode, numKids);
    nsCOMPtr<nsIDOMNode> firstKid;
    m_mathmlNode->GetFirstChild(getter_AddRefs(firstKid));
    if (numKids == 1 && firstKid && msiUtils::IsInputbox(editor, firstKid))
      res = SetDisplayMode(editor, isDisplay);
    else
    {
      if (inleft || inright) 
      {// the node at m_offset has been split into left and right. Replace it with left and right and set m_offset to point between them.
        nsCOMPtr<nsIDOMNodeList> children;
        nsCOMPtr<nsIDOMNode> childToReplace;
        m_mathmlNode->GetChildNodes(getter_AddRefs(children));
        if (children) 
          children->Item(m_offset, getter_AddRefs(childToReplace));
        if (childToReplace)
        {
          if (inleft && inright)
          {
            editor->ReplaceNode(inleft, childToReplace, m_mathmlNode);
            m_offset += 1; 
            editor->InsertNode(inright, m_mathmlNode, m_offset);
          }
          else if (inleft)
          {
            m_offset += 1; 
          }
        }  
      }
    
      nsCOMPtr<nsIDOMNode> parent, left, right, displayNode;
      nsCOMPtr<nsIDOMElement> displayElement;
      PRUint32 index;
      Split(left, right);
      m_mathmlNode->GetParentNode(getter_AddRefs(parent));
      msiUtils::CreateMathElement(editor, PR_TRUE, PR_TRUE, flags, displayElement);
      displayNode = do_QueryInterface(displayElement);
      msiUtils::GetIndexOfChildInParent(m_mathmlNode, index);
      if ((left || right) && displayNode && parent)
      {
        res = NS_OK;
        if (left)
          res = editor->InsertNode(left, parent, index);
        if (NS_SUCCEEDED(res))
          res = editor->ReplaceNode(displayNode, m_mathmlNode, parent);
        if (NS_SUCCEEDED(res) && right)
          res = editor->InsertNode(right, parent, index+2); // right of the display
        if (NS_SUCCEEDED(res))
          msiUtils::doSetCaretPosition(editor, selection, displayNode);
      }  
    }
  }
  return res;
}

nsresult msiMath::SetDisplayMode(nsIEditor * editor, PRBool isDisplay)
{
  nsresult res(NS_ERROR_FAILURE);
  nsCOMPtr<nsIDOMElement> mathElement(do_QueryInterface(m_mathmlNode));
  PRBool suppressTrans(PR_FALSE);
  if (mathElement && editor)
  {
    res = NS_OK;
    nsAutoString display, mode, block;
    msiEditingAtoms::display->ToString(display);
    msiEditingAtoms::mode->ToString(mode);
    msiEditingAtoms::block->ToString(block);
    if (m_isDisplay && !isDisplay)
    {
      editor->RemoveAttributeOrEquivalent(mathElement, mode, suppressTrans);
      editor->RemoveAttributeOrEquivalent(mathElement, display, suppressTrans);
      m_isDisplay = PR_FALSE;
    }
    else if (!m_isDisplay && isDisplay)
    {
      editor->RemoveAttributeOrEquivalent(mathElement, mode, suppressTrans);
      editor->SetAttributeOrEquivalent(mathElement, display, block, suppressTrans);
      m_isDisplay = PR_TRUE;
    }
  }
  return res;    
}


