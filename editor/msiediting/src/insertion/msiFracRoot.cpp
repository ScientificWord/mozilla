// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiFracRoot.h"
#include "msiUtils.h"
#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "nsIEditor.h"
#include "nsISelection.h"
#include "nsISimpleEnumerator.h"
#include "msiRequiredArgument.h"
#include "nsIMutableArray.h"
#include "nsIArray.h"
#include "nsComponentManagerUtils.h"

msiFracRoot::msiFracRoot(nsIDOMNode* mathmlNode, PRUint32 offset, PRUint32 mathmlType)
:  msiMInsertionBase(mathmlNode, offset, mathmlType)
{
  
}

NS_IMETHODIMP
msiFracRoot::InsertNode(nsIEditor * editor,
                  nsISelection * selection, 
                  nsIDOMNode * node, 
                  PRUint32 flags)
{
  nsresult res(NS_ERROR_FAILURE);
  nsCOMPtr<nsIArray> nodeArray;
  nsCOMPtr<nsIMutableArray> mutableArray = do_CreateInstance(NS_ARRAY_CONTRACTID, &res);
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
msiFracRoot::InsertMath(nsIEditor * editor,
                     nsISelection  * selection, 
                     PRBool isDisplay,
                     nsIDOMNode * left,    
                     nsIDOMNode * right,    
                     PRUint32 flags)
{
  return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP 
msiFracRoot::Inquiry(nsIEditor * editor,
                     PRUint32 inquiryID, 
                     PRBool * result)
{
  nsresult res(NS_OK);
  if (inquiryID == INSERT_DISPLAY || 
      inquiryID == INSERT_INLINE_MATH || 
      inquiryID == IS_MROW_REDUNDANT)
    *result = PR_FALSE;
  else
    *result = PR_TRUE;
  return res;
}

NS_IMETHODIMP
msiFracRoot::InsertNodes(nsIEditor * editor,
                       nsISelection * selection, 
                       nsIArray * nodeList,
                       PRBool  deleteExisting, 
                       PRUint32 flags)
{
  nsresult res(NS_ERROR_FAILURE);
  PRUint32 numKids(0), numNewNodes(0);
  
  if (nodeList)
    nodeList->GetLength(&numNewNodes);
  if (editor && selection && nodeList && m_mathmlNode && numNewNodes > 0)
  {
    nsCOMPtr<nsIDOMNode> child;
    PRUint32 insertPos = DetermineInsertPosition(flags);
    if (insertPos == IP_FirstChildLeft || insertPos == IP_FirstChildRight)
      msiUtils::GetChildNode(m_mathmlNode, 0, child);
    else  
      msiUtils::GetChildNode(m_mathmlNode, 1, child);
    PRBool atRight = (insertPos == IP_FirstChildRight) || (insertPos == IP_SecondChildRight);
    
    res = msiRequiredArgument::doInsertNodes(editor, selection, m_mathmlNode, child,
                                             atRight, nodeList, deleteExisting, flags);
  }
  return res;  
}

//private functions

PRUint32 msiFracRoot::DetermineInsertPosition(PRUint32 flags)
{
  PRUint32 position(IP_FirstChildLeft);
  if ((m_offset == 0 && (flags & FLAGS_NODE_LEFT_RIGHT) == FLAGS_NONE) || 
      (m_offset == 0 && (flags & FLAGS_NODE_LEFT) == FLAGS_NODE_LEFT))
    position = IP_FirstChildLeft;
  else if ((m_offset == 0 && (flags & FLAGS_NODE_RIGHT) == FLAGS_NODE_RIGHT) ||
           (m_offset == 1 && (flags & FLAGS_NODE_LEFT_RIGHT) == FLAGS_NONE))
    position = IP_FirstChildRight;
  else if (m_offset == 1 && ((flags & FLAGS_NODE_LEFT) == FLAGS_NODE_LEFT))
    position = IP_SecondChildLeft;
  else  //((m_offset == 1 && (flags & FLAGS_NODE_RIGHT) == FLAGS_NODE_RIGHT) || m_offset == 2)
    position = IP_SecondChildRight;
  return position;
}

