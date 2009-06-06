// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.
#include "msiMContainer.h"
#include "msiUtils.h"
#include "msiCoalesceUtils.h"
#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "nsIEditor.h"
#include "nsISelection.h"
#include "nsISimpleEnumerator.h"
#include "nsIMutableArray.h"
#include "nsComponentManagerUtils.h"

msiMContainer::msiMContainer(nsIDOMNode* mathmlNode, PRUint32 offset, PRUint32 mathmlType)
:  msiMInsertionBase(mathmlNode, offset, mathmlType)
{
  
}

NS_IMETHODIMP msiMContainer::Inquiry(nsIEditor* editor,
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
  else if (inquiryID == IS_MROW_REDUNDANT)
    *result = PR_TRUE;
  else
    *result = PR_TRUE;
  return res;
}                                                 

NS_IMETHODIMP
msiMContainer::InsertNode(nsIEditor *editor,
                          nsISelection *selection, 
                          nsIDOMNode *node, 
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
msiMContainer::InsertMath(nsIEditor *editor,
                         nsISelection   *selection, 
                         PRBool isDisplay,
                         nsIDOMNode * inLeft,
                         nsIDOMNode * inRight,
                         PRUint32        flags)
{
  return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP
msiMContainer::InsertNodes(nsIEditor * editor,
                           nsISelection * selection, 
                           nsIArray * nodeList,
                           PRBool  deleteExisting, 
                           PRUint32 flags)
{

  nsresult res(NS_ERROR_FAILURE);
  PRUint32 insertPos(0), numKids(0), numNewNodes(0);
  if (nodeList)
    nodeList->GetLength(&numNewNodes);
  if (editor && selection && m_mathmlNode && numNewNodes > 0)
  {
    nsCOMPtr<nsIDOMNode> tobeRemoved;
    insertPos = DetermineInsertPosition(flags, deleteExisting);
    res = msiUtils::GetNumberofChildren(m_mathmlNode, numKids);
    if (NS_SUCCEEDED(res) && !deleteExisting) // check to see if there is an inputbox in the neighborhood, is so wack it!
    {
      if (insertPos > 0 )
      {
        nsCOMPtr<nsIDOMNode> node;
        res = msiUtils::GetChildNode(m_mathmlNode, insertPos-1, node);
        if (NS_SUCCEEDED(res) && node && msiUtils::IsInputbox(editor, node))
        {
          insertPos -= 1;
          deleteExisting = PR_TRUE;
        } 
      }
      if (!deleteExisting && insertPos < numKids)
      {
        nsCOMPtr<nsIDOMNode> node;
        res = msiUtils::GetChildNode(m_mathmlNode, insertPos, node);
        if (NS_SUCCEEDED(res) && node && msiUtils::IsInputbox(editor, node))
          deleteExisting = PR_TRUE;
      }
    }
    if (NS_SUCCEEDED(res) && deleteExisting)
      res = msiUtils::GetChildNode(m_mathmlNode, insertPos, tobeRemoved);
    
    nsCOMPtr<nsIDOMNode> lfSide, rtSide, lfClone, rtClone;
    if (insertPos > 0 )  // Added left side node to beginning of node list for coalescing
    {
      msiUtils::GetChildNode(m_mathmlNode, insertPos-1, lfSide);
      msiUtils::CloneChildNode(m_mathmlNode, insertPos-1, lfClone);
    }
    // Added right side node to end of node list for coalescing    
    if (!deleteExisting && (insertPos < numKids))
    {
      msiUtils::GetChildNode(m_mathmlNode, insertPos, rtSide);
      msiUtils::CloneChildNode(m_mathmlNode, insertPos, rtClone);
    }
    else if (deleteExisting && (insertPos+1 < numKids))
    {
      msiUtils::GetChildNode(m_mathmlNode, insertPos+1, rtSide);
      msiUtils::CloneChildNode(m_mathmlNode, insertPos+1, rtClone);
    }
    
    PRUint32 pfcFlags(msiIMathMLCoalesce::PFCflags_removeRedundantMrows);
    nsCOMPtr<nsIArray> addToFront, addToEnd, nodeArray, coalesced;
    nsCOMPtr<nsIArray> inList(nodeList);
    nsCOMPtr<nsIMutableArray> mutableArray = do_CreateInstance(NS_ARRAY_CONTRACTID, &res);
    if (NS_SUCCEEDED(res) && mutableArray && inList)
    {
      if (lfClone)
      {
        res = msiCoalesceUtils::PrepareForCoalesceFromRight(editor, lfClone, pfcFlags, addToFront);
        if (NS_SUCCEEDED(res) && addToFront)
           res = msiUtils::AppendToMutableList(mutableArray, addToFront);
      }
      if (NS_SUCCEEDED(res))
         res = msiUtils::AppendToMutableList(mutableArray, inList);
      if (NS_SUCCEEDED(res) && rtClone)
      {
        res = msiCoalesceUtils::PrepareForCoalesceFromLeft(editor, rtClone, pfcFlags, addToEnd);
        if (NS_SUCCEEDED(res) && addToEnd)
          res = msiUtils::AppendToMutableList(mutableArray, addToEnd);
      }  
      if (NS_SUCCEEDED(res))  
        nodeArray = do_QueryInterface(mutableArray);
      if (NS_SUCCEEDED(res) && nodeArray)
        res = msiCoalesceUtils::CoalesceArray(editor, nodeArray, coalesced);
    }
    if (NS_SUCCEEDED(res) && coalesced)
    {
      nsCOMPtr<nsISimpleEnumerator> enumerator;
      coalesced->Enumerate(getter_AddRefs(enumerator));
      if (enumerator)
      {
        if (lfSide)
        {
          editor->SimpleDeleteNode(lfSide); //We don't call DeleteNode since we don't want to trigger DidDelete and WillDelete
          insertPos -= 1;
        }
        if (tobeRemoved)
        {
          editor->SimpleDeleteNode(tobeRemoved);
        }
        if  (rtSide)
        {
          editor->SimpleDeleteNode(rtSide);
        }
          
        PRBool someMore(PR_FALSE);
        while (NS_SUCCEEDED(res) && NS_SUCCEEDED(enumerator->HasMoreElements(&someMore)) && someMore) 
        {
          nsCOMPtr<nsISupports> isupp;
          if (NS_FAILED(enumerator->GetNext(getter_AddRefs(isupp))))
          {
            res = NS_ERROR_FAILURE; 
            break;
          }
          nsCOMPtr<nsIDOMNode> node(do_QueryInterface(isupp));
          if (node) 
          {
            res = editor->InsertNode(node, m_mathmlNode, insertPos);
            insertPos += 1;
          }   
        }
      }  
      if (NS_SUCCEEDED(res))
      {
        msiUtils::MarkCaretPosition(editor, m_mathmlNode, insertPos, flags, PR_FALSE, PR_FALSE);
        msiUtils::doSetCaretPosition(editor, selection, m_mathmlNode);
      }
    }  
  }    
  return res;
}

//protected
nsresult msiMContainer::Split(nsCOMPtr<nsIDOMNode> & left,
                              nsCOMPtr<nsIDOMNode> & right)
{ 
  nsresult res(NS_ERROR_FAILURE);
  if (m_mathmlNode)
  { 
    PRUint32 count;
    msiUtils::GetNumberofChildren(m_mathmlNode, count);
    if (m_offset == 0)
    {
      left = nsnull;
      msiUtils::CloneNode(m_mathmlNode, right);
    }
    else if (m_offset >= count)
    {
      right = nsnull;
      msiUtils::CloneNode(m_mathmlNode, left);
    }
    else //if (0 < m_offset && m_offset < count)
    { 
      msiUtils::CloneNode(m_mathmlNode, left);
      msiUtils::CloneNode(m_mathmlNode, right);
      if (left && right)
      {
        
        PRUint32 num_to_delete = count - m_offset;
        PRUint32 i(0);
        for (i = 1; i <= num_to_delete; i++)
        {
          nsCOMPtr<nsIDOMNode> child, dontcare;
          left->GetLastChild(getter_AddRefs(child));
          if (child)
            left->RemoveChild(child, getter_AddRefs(dontcare));
        }
        num_to_delete = m_offset;
        for (i = 1; i <= num_to_delete; i++)
        {
          nsCOMPtr<nsIDOMNode> child, dontcare;
          right->GetFirstChild(getter_AddRefs(child));
          if (child)
            right->RemoveChild(child, getter_AddRefs(dontcare));
        }
      }  
    } 
  }
  return res;  
}


PRUint32 msiMContainer::DetermineInsertPosition(PRUint32 flags, PRBool deleteExisting)
{
  PRUint32 insertPos(m_offset);
  PRUint32 numKids(0);
  msiUtils::GetNumberofChildren(m_mathmlNode, numKids);
  if (!deleteExisting && ((flags & FLAGS_NODE_RIGHT) == FLAGS_NODE_RIGHT))
    insertPos += 1;
  if (insertPos > numKids)
    insertPos = numKids;  
  return insertPos;
}
