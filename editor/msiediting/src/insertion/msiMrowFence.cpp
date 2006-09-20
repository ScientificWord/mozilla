// Copyright (c) 2005, MacKichan Software, Inc.  All rights reserved.

#include "msiMrowFence.h"
#include "msiUtils.h"
#include "msiCoalesceUtils.h"
#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "nsIEditor.h"
#include "nsISelection.h"
#include "nsISimpleEnumerator.h"
#include "nsIMutableArray.h"
#include "nsComponentManagerUtils.h"

msiMrowFence::msiMrowFence(nsIDOMNode* mathmlNode, PRUint32 offset)
:  msiMInsertionBase(mathmlNode, offset, MATHML_MROWFENCE)
{
  
}

NS_IMETHODIMP
msiMrowFence::InsertNode(nsIEditor * editor,
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
msiMrowFence::InsertMath(nsIEditor * editor,
                     nsISelection  * selection, 
                     PRBool isDisplay,
                     nsIDOMNode * left,    
                     nsIDOMNode * right,    
                     PRUint32 flags)
{
  return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP 
msiMrowFence::Inquiry(nsIEditor * editor,
                      PRUint32 inquiryID, 
                      PRBool * result)
{
  nsresult res(NS_OK);
  if (inquiryID == INSERT_DISPLAY || inquiryID == INSERT_INLINE_MATH)
    *result = PR_FALSE;
  else if (inquiryID == IS_MROW_REDUNDANT)
    *result = PR_TRUE; //TODO -- is this really the case
  else
    *result = PR_TRUE;
  return res;
}

NS_IMETHODIMP
msiMrowFence::InsertNodes(nsIEditor * editor,
                          nsISelection * selection, 
                          nsIArray * nodeList,
                          PRBool  deleteExisting, 
                          PRUint32 flags)
{
  nsresult res(NS_ERROR_FAILURE);
  PRUint32 insertPos(0), numKids(0), numNewNodes(0), caretPos(0);
  PRBool doMRow(PR_FALSE);
  nsCOMPtr<nsIDOMNode> tobeRemoved;
  nsCOMPtr<msiIMathMLInsertion> mrowEditing;
  nsCOMPtr<nsIArray> nodeArray;
  
  if (nodeList)
    nodeList->GetLength(&numNewNodes);
  if (editor && selection && nodeList && m_mathmlNode && numNewNodes > 0)
  {
    insertPos = DetermineInsertPosition(flags, deleteExisting);
    msiUtils::GetNumberofChildren(m_mathmlNode, numKids);
    if (insertPos == 0 || insertPos == numKids)
    {
      nsCOMPtr<msiIMathMLInsertion> msiEditing;
      PRUint32 localFlags(flags);
      res = SetupPassToParent(editor, insertPos, msiEditing, localFlags);
      if (NS_SUCCEEDED(res) && msiEditing)
        res = msiEditing->InsertNodes(editor, selection, nodeList, PR_FALSE, localFlags);
    }
    else
    {  
      if (!deleteExisting) // check to see if there is an inputbox in the neighborhood, is so wack-it!
      {
        if (insertPos > 1)
        {
          nsCOMPtr<nsIDOMNode> node;
          msiUtils::GetChildNode(m_mathmlNode, insertPos-1, node);
          if (node && msiUtils::IsInputbox(editor, node))
          {
            insertPos -= 1;
            deleteExisting = PR_TRUE;
          } 
        }
        if (!deleteExisting && insertPos+1 < numKids)
        {
          nsCOMPtr<nsIDOMNode> node;
          msiUtils::GetChildNode(m_mathmlNode, insertPos, node);
          if (node && msiUtils::IsInputbox(editor, node))
            deleteExisting = PR_TRUE;
        }
      }
      if (deleteExisting)
        msiUtils::GetChildNode(m_mathmlNode, insertPos, tobeRemoved);
      
      //Determine if nodes should be inserted into neighoring mrow
      if (insertPos > 1 && ((flags & FLAGS_RIGHTSIDE) == FLAGS_NONE))
      {
        nsCOMPtr<nsIDOMNode> child;
        msiUtils::GetChildNode(m_mathmlNode, insertPos-1, child);
        if (child && msiUtils::IsMrow(editor, child))
        {
          PRUint32 offset(0);
          msiUtils::GetNumberofChildren(child, offset);
          res = msiUtils::GetMathMLInsertionInterface(editor, child, offset, mrowEditing);
        }  
      }
      else if (!deleteExisting && insertPos+1 < numKids &&  ((flags & FLAGS_LEFTSIDE) == FLAGS_NONE) )
      {
        nsCOMPtr<nsIDOMNode> child;
        msiUtils::GetChildNode(m_mathmlNode, insertPos, child);
        if (child && msiUtils::IsMrow(editor, child))
          res = msiUtils::GetMathMLInsertionInterface(editor, child, 0, mrowEditing);
      }
      else if (deleteExisting && insertPos+2 < numKids) 
      {
        nsCOMPtr<nsIDOMNode> child;
        msiUtils::GetChildNode(m_mathmlNode, insertPos+1, child);
        if (child && msiUtils::IsMrow(editor, child))
          res = msiUtils::GetMathMLInsertionInterface(editor, child, 0, mrowEditing);
      }
      if (mrowEditing && NS_SUCCEEDED(res))
      {
        if (tobeRemoved)
          editor->DeleteNode(tobeRemoved);
        flags &= ~(FLAGS_RIGHTSIDE|FLAGS_LEFTSIDE|FLAGS_FROM_NODE);
        res = mrowEditing->InsertNodes(editor, selection, nodeList, PR_FALSE, flags);
      }
      else
      {
        nsCOMPtr<nsIArray> addToFront, addToEnd, nodeArray, coalescedArray;
        nsCOMPtr<nsIArray> inList(nodeList);
        nsCOMPtr<nsIMutableArray> mutableArray = do_CreateInstance(NS_ARRAY_CONTRACTID, &res);
        if (NS_SUCCEEDED(res) && mutableArray && inList)
        {
          PRUint32 pfcFlags(msiIMathMLCoalesce::PFCflags_removeRedundantMrows);
          nsCOMPtr<nsIDOMNode> lfSide, rtSide, lfClone, rtClone;
          if (insertPos > 1 )  // Added left side node to beginning of node list for coalescing
          {
            msiUtils::GetChildNode(m_mathmlNode, insertPos-1, lfSide);
            if (lfSide)
            {
              msiUtils::CloneNode(lfSide, lfClone);
              if (lfClone)
              {
                res = msiCoalesceUtils::PrepareForCoalesceFromRight(editor, lfClone, pfcFlags, addToFront);
                if (NS_SUCCEEDED(res))
                   res = msiUtils::AppendToMutableList(mutableArray, addToFront);
              }
            } 
          }
          if (NS_SUCCEEDED(res))
             res = msiUtils::AppendToMutableList(mutableArray, inList);
          // Added right side node to end of node list for coalescing    
          if (!deleteExisting && (insertPos+1 < numKids))
          {
            msiUtils::GetChildNode(m_mathmlNode, insertPos, rtSide);
            if (rtSide)
              msiUtils::CloneNode(rtSide, rtClone);
          }
          else if (deleteExisting && (insertPos+2 < numKids))
          {
            msiUtils::GetChildNode(m_mathmlNode, insertPos+1, rtSide);
            if (rtSide)
              msiUtils::CloneNode(rtSide, rtClone);
          }
          if (rtClone)
          {
            res = msiCoalesceUtils::PrepareForCoalesceFromLeft(editor, rtClone, pfcFlags, addToEnd);
            if (NS_SUCCEEDED(res))
              res = msiUtils::AppendToMutableList(mutableArray, addToEnd);
          }  
          if (NS_SUCCEEDED(res))
            nodeArray = do_QueryInterface(mutableArray);
          if (nodeArray)
            res = msiCoalesceUtils::CoalesceArray(editor, nodeArray, coalescedArray);
          if (NS_SUCCEEDED(res) && coalescedArray)
          {
          
            nsCOMPtr<nsIDOMElement> mrow;
            nsCOMPtr<nsIDOMNode> newNode;
            coalescedArray->GetLength(&numNewNodes);
            if (numNewNodes > 1 )
            {
              res = msiUtils::CreateMRow(editor,coalescedArray, mrow);
              if (NS_SUCCEEDED(res) && mrow)
                newNode = do_QueryInterface(mrow);
            }
            else if  (numNewNodes == 1)
              res = coalescedArray->QueryElementAt(0, NS_GET_IID(nsIDOMNode), getter_AddRefs(newNode));
            if (NS_SUCCEEDED(res) && newNode)
            {
              if (lfSide)
              {
                editor->DeleteNode(lfSide);
                insertPos -= 1;
              }
              if (tobeRemoved)
                editor->DeleteNode(tobeRemoved);
              if  (rtSide)
                editor->DeleteNode(rtSide);
              res = editor->InsertNode(newNode, m_mathmlNode, insertPos);
              if (NS_SUCCEEDED(res))
              {  
                msiUtils::MarkCaretPosition(editor, m_mathmlNode, insertPos+1, flags, PR_FALSE, PR_FALSE);
                msiUtils::doSetCaretPosition(editor, selection, m_mathmlNode);
              }  
            }
          }
        }
      }
    }
  }
  return res;
}         

//private functions


PRUint32 msiMrowFence::DetermineInsertPosition(PRUint32 flags, PRBool deleteExisting)
{
  PRUint32 insertPos(m_offset), numKids(0);
  msiUtils::GetNumberofChildren(m_mathmlNode, numKids);
  if (!deleteExisting && ((flags & FLAGS_NODE_RIGHT) == FLAGS_NODE_RIGHT))
    insertPos += 1;
  if (insertPos > numKids)
    insertPos = numKids;  
  return insertPos;
}

nsresult msiMrowFence::SetupPassToParent(nsIEditor * editor,
                                         PRUint32 insertPos,
                                         nsCOMPtr<msiIMathMLInsertion> & mathmlEditing,
                                         PRUint32 & flags)
{                                     
  nsresult res(NS_ERROR_FAILURE);
  if (editor && m_mathmlNode) 
  {
    res = msiUtils::SetupPassOffInsertToParent(editor, m_mathmlNode, PR_FALSE, mathmlEditing);
    if (NS_SUCCEEDED(res))
    {
      flags &= ~(FLAGS_RIGHTSIDE|FLAGS_LEFTSIDE);
      flags |= FLAGS_FROM_NODE;
      flags |= (insertPos > 0) ? FLAGS_RIGHTSIDE : FLAGS_LEFTSIDE;
    }  
  }
  return res;
}                                  
