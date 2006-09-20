// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiMlabeledtr.h"
#include "nsIDOMNode.h"
#include "msiUtils.h"

msiMlabeledtr::msiMlabeledtr(nsIDOMNode* mathmlNode, PRUint32 offset)
:  msiMInsertionBase(mathmlNode, offset, MATHML_MLABELEDTR)
{
}

NS_IMETHODIMP
msiMlabeledtr::InsertNode(nsIEditor * editor,
                          nsISelection * selection, 
                          nsIDOMNode * node, 
                          PRUint32 flags)
{
  nsresult res(NS_ERROR_FAILURE);
  if (editor && m_mathmlNode && node)
  {
    PRUint32 numKids(0);
    msiUtils::GetNumberofChildren(m_mathmlNode, numKids);
    PRUint32 childIndex(0), newOffset(0);
    flags &= ~(FLAGS_LEFTSIDE | FLAGS_RIGHTSIDE);
    if (m_offset > 0 && m_offset == numKids)
    {
      childIndex = m_offset - 1;
      flags |= FLAGS_FROM_NODE | FLAGS_RIGHTSIDE;
    }
    else
    {
      childIndex = m_offset;
      flags |= FLAGS_FROM_NODE | FLAGS_LEFTSIDE;
    }
    nsCOMPtr<nsIDOMNode> child;
    msiUtils::GetChildNode(m_mathmlNode, childIndex, child);
    NS_ASSERTION(child, "NULL child node");
    if (child)
    {
      if (flags & FLAGS_RIGHTSIDE)
        msiUtils::GetRightMostCaretPosition(editor, child, newOffset);
      else
        newOffset = 0;
      
      nsCOMPtr<msiIMathMLInsertion> mathmlEditing;
      res = msiUtils::GetMathMLInsertionInterface(editor, child, newOffset, mathmlEditing);
      NS_ASSERTION(mathmlEditing, "Get Insertion interface failed for child of mtr");
      if (NS_SUCCEEDED(res) && mathmlEditing)
      {
#ifdef DEBUG
        PRUint32 mathmlType = msiUtils::GetMathmlNodeType(mathmlEditing);
        NS_ASSERTION(mathmlType == MATHML_MTD, "Child of MTR is not MTD");
#endif        
        res = mathmlEditing->InsertNode(editor, selection, node, flags);
      }
      else  
        res = NS_ERROR_FAILURE;
    }
  }
  return res;
}

NS_IMETHODIMP
msiMlabeledtr::InsertMath(nsIEditor * editor,
                          nsISelection  * selection, 
                          PRBool isDisplay,
                          nsIDOMNode * left,    
                          nsIDOMNode * right,    
                          PRUint32 flags)
{
  return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP 
msiMlabeledtr::Inquiry(nsIEditor * editor,
                       PRUint32 inquiryID, 
                       PRBool * result)
{
  nsresult res(NS_OK);
  if (inquiryID == INSERT_DISPLAY || inquiryID == INSERT_INLINE_MATH)
    *result = PR_FALSE;
  else
    *result = PR_TRUE;
  return res;
}

NS_IMETHODIMP
msiMlabeledtr::InsertNodes(nsIEditor * editor,
                               nsISelection * selection, 
                               nsIArray * nodeList,
                               PRBool  deleteExisting, 
                               PRUint32 flags)
{
  return NS_ERROR_NOT_IMPLEMENTED;
}


