// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiMrowCoalesce.h"
#include "msiMrowEditingImp.h"   
#include "msiUtils.h"
#include "msiCoalesceUtils.h"
#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "nsIEditor.h"
#include "nsIMutableArray.h"
#include "nsComponentManagerUtils.h"

msiMrowCoalesce::msiMrowCoalesce(nsIDOMNode* mathmlNode, 
                                 PRUint32 offset) 
: msiMCoalesceBase(mathmlNode, offset, MATHML_MROW)
{
  MSI_NewMrowEditingImp(mathmlNode, getter_AddRefs(m_mrowEditingImp));
}
  
msiMrowCoalesce::~msiMrowCoalesce()
{
}

// ISupports
NS_IMPL_ISUPPORTS_INHERITED1(msiMrowCoalesce, msiMCoalesceBase, msiIMrowEditing)


NS_IMETHODIMP
msiMrowCoalesce::Coalesce(nsIEditor * editor,
                          nsIDOMNode * node,
                          nsIArray ** coalesced)                
{
  //TODO -- Need to consider attributes on the mrow!!!
  nsresult res(NS_ERROR_FAILURE);
  *coalesced = nsnull;
  if (node && editor && m_mathmlNode)
  {
    PRUint32 nodetype(MATHML_UNKNOWN);
    res = msiUtils::GetMathmlNodeType(editor, node, nodetype);
    if (NS_SUCCEEDED(res))
    {
      nsCOMPtr<nsIMutableArray> mutableArray;
      nsCOMPtr<nsIArray> nodeArray;
      PRUint32 numExistingKids(0);
      msiUtils::GetNumberofChildren(m_mathmlNode, numExistingKids);
      nsCOMPtr<nsIDOMNode> leftNode, rightNode;
      if (m_offset < numExistingKids)
        res = msiUtils::RemoveChildNode(m_mathmlNode, m_offset, rightNode);
      if (NS_SUCCEEDED(res) && m_offset > 0)  
      {
        res = msiUtils::RemoveChildNode(m_mathmlNode, m_offset-1, leftNode);
        m_offset -= 1;
      }  
      if (NS_SUCCEEDED(res))
      {
        msiUtils::GetNumberofChildren(m_mathmlNode, numExistingKids);
        mutableArray = do_CreateInstance(NS_ARRAY_CONTRACTID, &res);
      }
      if (NS_SUCCEEDED(res) && mutableArray)
      {
        nsCOMPtr<nsIArray> leftArray, rightArray, coalesced;
        PRUint32 pfcFlags(PFCflags_removeRedundantMrows);
        if (leftNode)
        {
          res = msiCoalesceUtils::PrepareForCoalesceFromRight(editor, leftNode, pfcFlags, leftArray);
          if (NS_SUCCEEDED(res))
            res = msiUtils::AppendToMutableList(mutableArray, leftArray);
        }  
        res = mutableArray->AppendElement(node, PR_FALSE);
        if (NS_SUCCEEDED(res) && rightNode)  
        {
           res = msiCoalesceUtils::PrepareForCoalesceFromLeft(editor, rightNode, pfcFlags, rightArray);
          if (NS_SUCCEEDED(res))
            res = msiUtils::AppendToMutableList(mutableArray, rightArray);
        }    
        if (NS_SUCCEEDED(res))  
           nodeArray = do_QueryInterface(mutableArray);
        if (nodeArray)
          res = msiCoalesceUtils::CoalesceArray(editor, nodeArray, coalesced);
        if (NS_SUCCEEDED(res) && coalesced)
          res = msiUtils::InsertChildren(m_mathmlNode, m_offset, coalesced);
      }  
    }  
    if (NS_SUCCEEDED(res))
    {
      nsCOMPtr<nsIMutableArray> mutableArray = do_CreateInstance(NS_ARRAY_CONTRACTID, &res);
      if (NS_SUCCEEDED(res))
      {
        mutableArray->AppendElement(m_mathmlNode, PR_FALSE);
        *coalesced = mutableArray;
        NS_ADDREF(*coalesced);
      }
    }    
  }        
  return res;
}

NS_IMETHODIMP
msiMrowCoalesce::CoalesceTxn(nsIEditor * editor,
                             nsIDOMNode * node,
                             nsITransaction ** txn)                
{
  *txn = nsnull;
  return NS_OK;
}



NS_IMETHODIMP
msiMrowCoalesce::PrepareForCoalesce(nsIEditor * editor,
                                    PRUint32    pfcFlags,
                                    nsIArray ** beforeOffset,                
                                    nsIArray ** afterOffset)                
{
  *beforeOffset = nsnull;
  *afterOffset = nsnull;
  nsresult res(NS_ERROR_FAILURE);
  nsCOMPtr<nsIMutableArray> bMutableArray, aMutableArray;
  PRUint32 numKids(0);
  if (m_mathmlNode)
    msiUtils::GetNumberofChildren(m_mathmlNode, numKids);
  PRBool redundant(pfcFlags & PFCflags_removeRedundantMrows);
  PRBool recurse((pfcFlags & PFCflags_RecursiveFully) || (pfcFlags & PFCflags_Recursive1));
  PRBool offsetOnBoundary = (m_offset== 0 || m_offset == numKids);
  if (redundant)
    IsRedundant(editor, offsetOnBoundary, &redundant);
  if (numKids > 0 && redundant)
  {
    bMutableArray = do_CreateInstance(NS_ARRAY_CONTRACTID, &res);
    if (NS_SUCCEEDED(res))
      aMutableArray = do_CreateInstance(NS_ARRAY_CONTRACTID, &res);
    if (NS_SUCCEEDED(res))
    {
      if (m_offset > numKids)
        m_offset = numKids;
      PRUint32 i(0);
      PRUint32 loc_pfcFlags(pfcFlags & PFCflags_removeRedundantMrows);
      if (pfcFlags & PFCflags_RecursiveFully)
        loc_pfcFlags |= PFCflags_RecursiveFully;
      for (i = 0; i < m_offset && NS_SUCCEEDED(res); i++)
      {
        nsCOMPtr<nsIDOMNode> kid;
        res = msiUtils::GetChildNode(m_mathmlNode, i, kid);
        if (NS_SUCCEEDED(res) && kid)
        {
          if (recurse || i+1 == m_offset) // 
          {
            nsCOMPtr<nsIArray> lfArray;
            res = msiCoalesceUtils::PrepareForCoalesceFromRight(editor, kid, loc_pfcFlags, lfArray);
            if (NS_SUCCEEDED(res))
              res = msiUtils::AppendToMutableList(bMutableArray, lfArray);
          }
          else
            res = bMutableArray->AppendElement(kid, PR_FALSE);
        }
      }
      for (i = m_offset; i < numKids && NS_SUCCEEDED(res); i++)
      {
        nsCOMPtr<nsIDOMNode> kid;
        res = msiUtils::GetChildNode(m_mathmlNode, i, kid);
        if (NS_SUCCEEDED(res) && kid)
        {
          if (recurse || i == m_offset) // 
          {
            nsCOMPtr<nsIArray> rtArray;
            res = msiCoalesceUtils::PrepareForCoalesceFromLeft(editor, kid, loc_pfcFlags, rtArray);
            if (NS_SUCCEEDED(res))
              res = msiUtils::AppendToMutableList(aMutableArray, rtArray);
          }
          else
            res = aMutableArray->AppendElement(kid, PR_FALSE);
        }
      }
      if (NS_SUCCEEDED(res))
      {
        PRUint32 numNodes(0);  
        bMutableArray->GetLength(&numNodes);
        if (numNodes > 0)
        {
          *beforeOffset = bMutableArray;
          NS_ADDREF(*beforeOffset);
        }
        numNodes = 0;
        aMutableArray->GetLength(&numNodes);
        if (numNodes > 0)
        {
          *afterOffset = aMutableArray;
          NS_ADDREF(*afterOffset);
        }
      }
    }
  }
  else if (numKids == 0)
  { 
    // this should cause empty mrow to be discarded 
    bMutableArray = do_CreateInstance(NS_ARRAY_CONTRACTID, &res);
    if (NS_SUCCEEDED(res))
    {
      *beforeOffset = bMutableArray;
      NS_ADDREF(*beforeOffset);
    } 
  }
  else // !redundant
    res = msiMCoalesceBase::PrepareForCoalesce(editor, pfcFlags, beforeOffset, afterOffset);
  return res;    
}
