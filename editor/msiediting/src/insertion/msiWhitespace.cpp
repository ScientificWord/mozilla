// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.
#include "msiWhitespace.h"
#include "msiUtils.h"
#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "nsIDOMText.h"
#include "nsIDOMNamedNodeMap.h"
#include "nsIDOMAttr.h"
#include "nsIEditor.h"


//ljh in most cases Whitespace pass off to their parent because it doesn't know if the node need to be
// nested in an mrow.
msiWhitespace::msiWhitespace(nsIDOMNode* mathmlNode, PRUint32 offset) 
:  msiMInsertionBase(mathmlNode, offset, MSI_WHITESPACE)
{
}

NS_IMETHODIMP
msiWhitespace::InsertNode(nsIEditor      *editor,
                        nsISelection   *selection, 
                        nsIDOMNode     *node, 
                        PRUint32        flags)
{
  nsresult res(NS_ERROR_FAILURE);
  nsCOMPtr<msiIMathMLInsertion> mathmlEditing;
  res = SetupPassToParent(editor, mathmlEditing, flags);
  if (NS_SUCCEEDED(res) && mathmlEditing)
    res = mathmlEditing->InsertNode(editor, selection, node, flags);
  return res; 
}   

NS_IMETHODIMP
msiWhitespace::InsertMath(nsIEditor *editor,
                         nsISelection   *selection, 
                         PRBool isDisplay,
                         nsIDOMNode * left,
                         nsIDOMNode * right,
                         PRUint32        flags)
{
  nsresult res(NS_ERROR_FAILURE);
  if (m_mathmlNode)
  {
    nsCOMPtr<msiIMathMLInsertion> mathmlEditing;
  res = SetupPassToParent(editor, mathmlEditing, flags);
    if (NS_SUCCEEDED(res) && mathmlEditing)
      res = mathmlEditing->InsertMath(editor, selection, isDisplay, nsnull, nsnull, flags);
  }
  return res;    
}

NS_IMETHODIMP 
msiWhitespace::Inquiry(nsIEditor * editor,
                       PRUint32 inquiryID, 
                       PRBool * result)
{
  nsresult res(NS_OK);
  if (inquiryID == INSERT_DISPLAY || inquiryID == INSERT_INLINE_MATH)
  {
    if (m_mathmlNode)
    {
      nsCOMPtr<msiIMathMLInsertion> mathmlEditing;
      res = msiUtils::SetupPassOffInsertToParent(editor, m_mathmlNode, PR_FALSE, mathmlEditing);
      if (NS_SUCCEEDED(res) && mathmlEditing)
        res = mathmlEditing->Inquiry(editor, inquiryID, result);
      else
        *result = PR_FALSE;
    }
    else
      *result = PR_FALSE;
  }
  else
    *result = PR_TRUE;
  return res;
}

NS_IMETHODIMP
msiWhitespace::InsertNodes(nsIEditor * editor,
                               nsISelection * selection, 
                               nsIArray * nodeList,
                               PRBool  deleteExisting, 
                               PRUint32 flags)
{
  nsresult res(NS_ERROR_FAILURE);
  if (m_mathmlNode)
  {
    nsCOMPtr<msiIMathMLInsertion> mathmlEditing;
    res = SetupPassToParent(editor, mathmlEditing, flags);
    if (NS_SUCCEEDED(res) && mathmlEditing)
      res = mathmlEditing->InsertNodes(editor, selection, nodeList, deleteExisting, flags);
  }
  return res;    
}




//protected
nsresult msiWhitespace::SetupPassToParent(nsIEditor * editor,
                                        nsCOMPtr<msiIMathMLInsertion> & mathmlEditing,
                                        PRUint32 & flags)
{                                     
  nsresult res(NS_ERROR_FAILURE);
  if (m_mathmlNode) 
  {
    res = msiUtils::SetupPassOffInsertToParent(editor, m_mathmlNode, PR_FALSE, mathmlEditing);
    flags &= ~FLAGS_RIGHTSIDE;
    flags |= FLAGS_FROM_NODE | FLAGS_LEFTSIDE;
  }
  return res;
}                                  
