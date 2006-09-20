// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.
#include "msiInputbox.h"
#include "msiUtils.h"
#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "nsIEditor.h"
#include "nsComponentManagerUtils.h"

msiInputbox::msiInputbox(nsIDOMNode* mathmlNode, PRUint32 offset) 
:  msiMInsertionBase(mathmlNode, offset, MSI_INPUTBOX)
{
}

NS_IMETHODIMP
msiInputbox::InsertNode(nsIEditor      *editor,
                        nsISelection   *selection, 
                        nsIDOMNode     *node, 
                        PRUint32        flags)
{
  nsresult res(NS_ERROR_FAILURE);
  nsCOMPtr<nsIMutableArray> mutableArray = do_CreateInstance(NS_ARRAY_CONTRACTID, &res);
  nsCOMPtr<nsIArray> nodeArray;
  if (NS_SUCCEEDED(res) && mutableArray && node)
  {
    res = mutableArray->AppendElement(node, PR_FALSE);
    if (NS_SUCCEEDED(res))
      nodeArray = do_QueryInterface(mutableArray);
    if (NS_SUCCEEDED(res) && nodeArray)
      res = InsertNodes(editor, selection, nodeArray, PR_FALSE, flags);
  }
  return res;
}

NS_IMETHODIMP
msiInputbox::InsertNodes(nsIEditor * editor,
                          nsISelection * selection, 
                          nsIArray * nodeList,
                          PRBool  deleteExisting, 
                          PRUint32 flags)
{
  nsresult res(NS_ERROR_FAILURE);
  nsCOMPtr<msiIMathMLInsertion> mathmlEditing;
  res = SetupPassToParent(editor, mathmlEditing, flags);
  if (NS_SUCCEEDED(res) && mathmlEditing)
    res = mathmlEditing->InsertNodes(editor, selection, nodeList, PR_TRUE, flags);
  return res;  
}


NS_IMETHODIMP
msiInputbox::InsertMath(nsIEditor *editor,
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
msiInputbox::Inquiry(nsIEditor * editor,
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

//protected
nsresult msiInputbox::SetupPassToParent(nsIEditor * editor,
                                        nsCOMPtr<msiIMathMLInsertion> & mathmlEditing,
                                        PRUint32 & flags)
{                                     
  nsresult res(NS_ERROR_FAILURE);
  if (m_mathmlNode) 
  {
    res = msiUtils::SetupPassOffInsertToParent(editor, m_mathmlNode, PR_FALSE, mathmlEditing);
    flags |= FLAGS_FROM_NODE | FLAGS_LEFTSIDE;
  }
  return res;
}                                  
