// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiUnderAndOrOver.h"
#include "msiUtils.h"
//#include "msiCoalesceUtils.h"
#include "msiRequiredArgument.h"
#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "nsIEditor.h"
#include "nsISelection.h"
#include "msiIMathMLInsertion.h"
#include "msiMrow.h"
#include "nsIMutableArray.h"
#include "nsIArray.h"
#include "nsComponentManagerUtils.h"

msiUnderAndOrOver::msiUnderAndOrOver(nsIDOMNode* mathmlNode, PRUint32 offset, PRUint32 mathmlType)
: msiMInsertionBase(mathmlNode, offset, mathmlType)
{
  
}

NS_IMETHODIMP
msiUnderAndOrOver::InsertNode(nsIEditor * editor,
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
msiUnderAndOrOver::InsertMath(nsIEditor *editor,
                      nsISelection   *selection, 
                      PRBool isDisplay,
                      nsIDOMNode * left,
                      nsIDOMNode * right,
                      PRUint32 flags)
{
  nsresult res(NS_ERROR_FAILURE);
  if (m_offset == 0 && !left)
  {
    nsCOMPtr<msiIMathMLInsertion> msiEditing;
    res = msiUtils::SetupPassOffInsertToParent(editor, m_mathmlNode, PR_FALSE, msiEditing);
    if (NS_SUCCEEDED(res) && msiEditing)
      res = msiEditing->InsertMath(editor, selection, isDisplay, nsnull, nsnull, flags);
  }
  else
    res = NS_ERROR_NOT_IMPLEMENTED;
  return res;
}

NS_IMETHODIMP
msiUnderAndOrOver::Inquiry(nsIEditor * editor,
                   PRUint32 inquiryID, 
                   PRBool * result)
{
  nsresult res(NS_OK);
  if (inquiryID == INSERT_DISPLAY || inquiryID == INSERT_INLINE_MATH)
  {
    if (m_offset == 0 && m_mathmlNode)
    { //TODO -- consider nested scripts further
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
  else if (inquiryID == IS_MROW_REDUNDANT)
    *result = PR_FALSE;
  else
    *result = PR_TRUE;
  return res;
}

NS_IMETHODIMP
msiUnderAndOrOver::InsertNodes(nsIEditor * editor,
                       nsISelection * selection, 
                       nsIArray * nodeList,
                       PRBool  deleteExisting, 
                       PRUint32 flags)
{
  nsresult res(NS_ERROR_FAILURE);
  PRUint32 insertPos(0), numKids(0), numNodes(0); //, caretPos(0);
  if (nodeList)
    nodeList->GetLength(&numNodes);
  if (editor && selection && nodeList && m_mathmlNode && numNodes > 0)
  {
    insertPos = DetermineInsertPosition(flags);
    nsCOMPtr<nsIDOMNode> child;
    if (insertPos == IP_BaseLeft || insertPos == IP_BaseRight)
      msiUtils::GetChildNode(m_mathmlNode, 0, child);
    else if (insertPos == IP_Script1Left || insertPos == IP_Script1Right) 
      msiUtils::GetChildNode(m_mathmlNode, 1, child);
    else // if (insertPos == IP_Script2Left || insertPos == IP_Script2Right) 
      msiUtils::GetChildNode(m_mathmlNode, 2, child);

    PRBool atRight = (insertPos == IP_BaseRight) || (insertPos == IP_Script1Right) || 
                     (insertPos == IP_Script2Right);
    res = msiRequiredArgument::doInsertNodes(editor, selection, m_mathmlNode, child,
                                             atRight, nodeList, deleteExisting, flags);
  }
  return res;  
}


//private



PRUint32 msiUnderAndOrOver::DetermineInsertPosition(PRUint32 flags)
{
  PRUint32 position(IP_BaseLeft);
  if ((m_offset == 0 && (flags & FLAGS_NODE_LEFT_RIGHT) == FLAGS_NONE) || 
      (m_offset == 0 && (flags & FLAGS_NODE_LEFT) == FLAGS_NODE_LEFT))
    position = IP_BaseLeft;
  else if ((m_offset == 0 && (flags & FLAGS_NODE_RIGHT) == FLAGS_NODE_RIGHT) ||
           (m_offset == 1 && (flags & FLAGS_NODE_LEFT_RIGHT) == FLAGS_NONE))
    position = IP_BaseRight;
  else if (m_offset == 1 && ((flags & FLAGS_NODE_LEFT) == FLAGS_NODE_LEFT))
    position = IP_Script1Left;
  else  //((m_offset == 1 && (flags & FLAGS_NODE_RIGHT) == FLAGS_NODE_RIGHT) || m_offset == 2)
    position = IP_Script1Right;
  return position;
}

                            
