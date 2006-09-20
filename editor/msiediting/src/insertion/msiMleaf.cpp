// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.
#include "msiMleaf.h"
#include "msiUtils.h"
#include "msiCoalesceUtils.h"
#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "nsIDOMText.h"
#include "nsIDOMCharacterData.h"
#include "nsIEditor.h"
#include "nsISelection.h"
#include "nsIDOMNodeList.h"
#include "msiIMathMLCoalesce.h"
#include "nsIMutableArray.h"
#include "nsIArray.h"
#include "nsComponentManagerUtils.h"

msiMleaf::msiMleaf(nsIDOMNode* mathmlNode, PRUint32 offset, PRUint32 mathmlType)
:  msiMInsertionBase(mathmlNode, offset, mathmlType), m_length(0)
{
  if (mathmlNode)
  {
    nsCOMPtr<nsIDOMNode> child;
    mathmlNode->GetFirstChild(getter_AddRefs(child));
    if (child)
    {
      nsCOMPtr<nsIDOMText> text(do_QueryInterface(child));
      if (text)
      {
         nsCOMPtr<nsIDOMCharacterData> characterdata(do_QueryInterface(text));
         if (characterdata)
           characterdata->GetLength(&m_length);
      }
    }
  }
}

NS_IMETHODIMP
msiMleaf::InsertNode(nsIEditor * editor,
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
msiMleaf::InsertMath(nsIEditor *editor,
                         nsISelection   *selection, 
                         PRBool isDisplay,
                         nsIDOMNode * inLeft,
                         nsIDOMNode * inRight,
                         PRUint32        flags)
{
  nsresult res(NS_ERROR_FAILURE);
  nsCOMPtr<msiIMathMLInsertion> mathmlEditing;
  nsCOMPtr<nsIDOMNode> left, right;
  res = SetupPassToParent(editor, mathmlEditing, left, right, flags);
  if (NS_SUCCEEDED(res) && mathmlEditing)
      res = mathmlEditing->InsertMath(editor, selection, isDisplay, left, right, flags);
  return res;
}

NS_IMETHODIMP 
msiMleaf::Inquiry(nsIEditor* editor,
                  PRUint32 inquiryID, 
                  PRBool * result)
{
  nsresult res(NS_OK);
  nsCOMPtr<msiIMathMLInsertion> mathmlEditing;
  PRUint32 dontcare(0);
  res = SetupPassToParent(editor, mathmlEditing, dontcare);
  if (NS_SUCCEEDED(res) && mathmlEditing)
      res = mathmlEditing->Inquiry(editor, inquiryID, result);
  return res;
}

NS_IMETHODIMP
msiMleaf::InsertNodes(nsIEditor * editor,
                               nsISelection * selection, 
                               nsIArray * nodeList,
                               PRBool  deleteExisting, 
                               PRUint32 flags)
{
  nsresult res(NS_ERROR_FAILURE);
  PRBool localDelExisting(PR_FALSE);
  nsCOMPtr<nsIArray> nodeArray;
  PRUint32 numNodes(0);
  if (nodeList)
    nodeList->GetLength(&numNodes);
  if (m_mathmlNode && numNodes > 0)
  {
    if (m_length == 0 || (m_offset > 0 && m_offset < m_length))
    {
      PRUint32 pfcFlags(msiIMathMLCoalesce::PFCflags_none);
      nsCOMPtr<nsIArray> addToFront, addToEnd;
      nsCOMPtr<nsIDOMNode> clone;
      res = msiUtils::CloneNode(m_mathmlNode, clone);
      if (NS_SUCCEEDED(res) && clone)
        res = msiCoalesceUtils::PrepareForCoalesce(editor, clone, m_offset, pfcFlags, addToFront, addToEnd);
      if (NS_SUCCEEDED(res))
      {
        res = msiUtils::AddToNodeList(nodeList, addToFront, addToEnd, nodeArray);
        localDelExisting = PR_TRUE;
      }  
    }
    else
    {
      res = NS_OK;
      nodeArray = nodeList;
    }
    if (NS_SUCCEEDED(res) && nodeArray)
    {
      nsCOMPtr<msiIMathMLInsertion> mathmlEditing;
      res = SetupPassToParent(editor, mathmlEditing, flags);
      if (NS_SUCCEEDED(res) && mathmlEditing && nodeArray)
        res = mathmlEditing->InsertNodes(editor, selection, nodeArray, localDelExisting, flags);
    }    
  }
  return res;
}



nsresult msiMleaf::SetupPassToParent(nsIEditor * editor,
                                     nsCOMPtr<msiIMathMLInsertion> & mathmlEditing,
                                     PRUint32 & flags)
{                                     
  nsresult res(NS_ERROR_FAILURE);
  if (m_mathmlNode) 
  {
    res = msiUtils::SetupPassOffInsertToParent(editor, m_mathmlNode, PR_FALSE, mathmlEditing);
    if (NS_SUCCEEDED(res))
    {
      flags &= ~(FLAGS_RIGHTSIDE|FLAGS_LEFTSIDE);
      flags |= FLAGS_FROM_NODE;
      flags |= (m_offset > 0) ? FLAGS_RIGHTSIDE : FLAGS_LEFTSIDE;
    }  
  }
  return res;
}                                  

nsresult msiMleaf::SetupPassToParent(nsIEditor * editor,
                                     nsCOMPtr<msiIMathMLInsertion> & mathmlEditing,
                                     nsCOMPtr<nsIDOMNode> & left,
                                     nsCOMPtr<nsIDOMNode> & right,
                                     PRUint32 & flags)
{                                     
  nsresult res(NS_ERROR_FAILURE);
  left = nsnull;
  right = nsnull;
  if (m_mathmlNode) 
  {
    res = msiUtils::SetupPassOffInsertToParent(editor, m_mathmlNode, PR_FALSE, mathmlEditing);
    if (NS_SUCCEEDED(res) && mathmlEditing)
    {
      flags &= ~(FLAGS_RIGHTSIDE|FLAGS_LEFTSIDE);
      if (m_offset > 0)
      {
        flags |= FLAGS_FROM_NODE | FLAGS_RIGHTSIDE;
        msiUtils::CloneNode(m_mathmlNode, left);
      }
      else
      {
        flags |= FLAGS_FROM_NODE | FLAGS_LEFTSIDE;
        msiUtils::CloneNode(m_mathmlNode, right);
      }  
    }
  }
  return res;
}                                  

//nsresult msiMleaf::Split(nsCOMPtr<nsIDOMNode> & left, nsCOMPtr<nsIDOMNode> & right)
//{
//
//  nsresult res(NS_ERROR_FAILURE);
//  if (m_offset == 0)
//    res = msiUtils::CloneNode(m_mathmlNode, right);
//  else if (m_offset >= m_length)  
//    res = msiUtils::CloneNode(m_mathmlNode, left);
//  else // 0 < m_offset < m_length)
//  {
//    res = msiUtils::CloneNode(m_mathmlNode, left);
//    if (NS_SUCCEEDED(res))
//      res = msiUtils::CloneNode(m_mathmlNode, right);
//    if (NS_SUCCEEDED(res) && left)
//    {
//      nsCOMPtr<nsIDOMNode> child;
//      res = left->GetFirstChild(getter_AddRefs(child));
//      if (NS_SUCCEEDED(res) && child)
//      {
//        nsCOMPtr<nsIDOMCharacterData> charData(do_QueryInterface(child));
//        if (charData)
//          res = charData->DeleteData(m_offset, m_length-m_offset);
//      }
//    }  
//    if (NS_SUCCEEDED(res) && right)
//    {
//      nsCOMPtr<nsIDOMNode> child;
//      res = right->GetFirstChild(getter_AddRefs(child));
//      if (NS_SUCCEEDED(res) && child)
//      {
//        nsCOMPtr<nsIDOMCharacterData> charData(do_QueryInterface(child));
//        if (charData)
//          res = charData->DeleteData(0, m_offset);
//      }  
//    }
//  }
//  return res;
//}  
