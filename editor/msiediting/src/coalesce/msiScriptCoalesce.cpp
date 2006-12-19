// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiScriptCoalesce.h"
#include "msiScript.h"
#include "msiUtils.h"
#include "msiCoalesceUtils.h"
#include "msiRequiredArgument.h"
#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "nsIEditor.h"
#include "nsIMutableArray.h"
#include "msiEditingAtoms.h"
#include "nsComponentManagerUtils.h"
#include "msiIMathMLEditor.h"

msiScriptCoalesce::msiScriptCoalesce(nsIDOMNode* mathmlNode, 
                                      PRUint32 offset,
                                      PRUint32 mathmlType) 
: msiMCoalesceBase(mathmlNode, offset, mathmlType)
{
  if (mathmlType == MATHML_MSUBSUP)
    m_maxOffset = 3;
  else
    m_maxOffset = 2;  
}
  
msiScriptCoalesce::~msiScriptCoalesce()
{
}

NS_IMETHODIMP
msiScriptCoalesce::Coalesce(nsIEditor * editor,
                        nsIDOMNode * node,
                        nsIArray ** coalesced)                
{
  nsresult res(NS_ERROR_FAILURE);
  *coalesced = nsnull;
  if (node && editor && m_mathmlNode)
  {
    if (m_offset == 0)
      res = CoalesceLeft(editor, node, coalesced);
    else if (m_offset == m_maxOffset)
      res = CoalesceRight(editor, node, coalesced);
  }
  return res;
}

NS_IMETHODIMP
msiScriptCoalesce::PrepareForCoalesce(nsIEditor * editor,
                                      PRUint32    pfcFlags,
                                      nsIArray ** beforeOffset,                
                                      nsIArray ** afterOffset)                
{
  *beforeOffset = nsnull;
  *afterOffset = nsnull;
  nsresult res(NS_OK);
  nsCOMPtr<nsIMutableArray> mutableArray1, mutableArray2;
  if (m_offset == 1)
    mutableArray2 = do_CreateInstance(NS_ARRAY_CONTRACTID, &res);
  if (NS_SUCCEEDED(res))  
    mutableArray1 = do_CreateInstance(NS_ARRAY_CONTRACTID, &res);
  if (NS_SUCCEEDED(res) && m_mathmlNode)
  {
    if (m_offset == 0 || m_offset == 1)
    {
      nsCOMPtr<nsIDOMNode> child, base;
      res = msiUtils::GetChildNode(m_mathmlNode, 0, child);
      if (NS_SUCCEEDED(res) && child)
      {
        if (!msiUtils::IsInputbox(editor, child))
        {
          nsCOMPtr<nsIDOMElement> inputbox;
          nsCOMPtr<nsIDOMNode> inputnode;
          PRUint32 dummyFlags(0);
          msiUtils::CreateInputbox(editor, PR_FALSE, PR_FALSE, dummyFlags, inputbox);
          if (inputbox)
            inputnode = do_QueryInterface(inputbox);
          if (inputnode)
          {
            res = msiUtils::ReplaceChildNode(m_mathmlNode, 0, inputnode, base);
            if (NS_SUCCEEDED(res) && base)
            {
              nsCOMPtr<nsIArray> baseArray;
              nsCOMPtr<nsIMutableArray> appendto;
              PRUint32 offset(0);
              if (m_offset == 1)
                msiUtils::GetRightMostCaretPosition(editor, base, offset);
              res = msiCoalesceUtils::PrepareForCoalesceFromRight(editor, base, pfcFlags, baseArray);
              if (NS_SUCCEEDED(res))
              {
                if (m_offset == 1)
                  appendto =  mutableArray2;
                else
                  appendto = mutableArray1;
              }
              if (NS_SUCCEEDED(res) && appendto && baseArray)
                res = msiUtils::AppendToMutableList(appendto, baseArray);  
            }
          }
          else
            res = NS_ERROR_FAILURE;
        }      
        if (NS_SUCCEEDED(res))
          res = mutableArray1->AppendElement(m_mathmlNode, PR_FALSE);
        if (NS_SUCCEEDED(res))
        {
          if (m_offset == 0)
          {
            *afterOffset = mutableArray1;
            NS_ADDREF(*afterOffset);
          }
          else //m_offset == 1;
          {
            *afterOffset = mutableArray1;
            NS_ADDREF(*afterOffset);
            PRUint32 numNodes(0);  
            if (mutableArray2)
              mutableArray2->GetLength(&numNodes);
            if (numNodes > 0)
            {
              *beforeOffset = mutableArray2;
              NS_ADDREF(*beforeOffset);
            }
          }
        }
      }
      else
        res = NS_ERROR_FAILURE;
    }
    else
    {
      res = mutableArray1->AppendElement(m_mathmlNode, PR_FALSE);
      if (NS_SUCCEEDED(res))
      {
        *beforeOffset = mutableArray1;
        NS_ADDREF(*beforeOffset);
      }   
    }  
  }
  else 
   res = NS_ERROR_FAILURE;
    
  return res;
}

NS_IMETHODIMP
msiScriptCoalesce::CoalesceTxn(nsIEditor * editor,
                               nsIDOMNode * node,
                               nsITransaction ** txn)                
{
  nsresult res(NS_ERROR_FAILURE);
  *txn = nsnull;
  if (node && editor && m_mathmlNode)
  {
    if (m_offset == 0)
      res = CoalesceLeft(editor, node, txn);
    else if (m_offset == m_maxOffset)
    {
      NS_ASSERTION(PR_FALSE, "Currently not implemented");
      //TODO 12/06 ljh -- currently this should not occur since this method is only used for deletion
      //res = CoalesceRight(editor, node, txn);
    }  
  }
  return res;
}


nsresult msiScriptCoalesce::CoalesceLeft(nsIEditor * editor, nsIDOMNode * node, nsIArray** coalesced)
{
  //ljh m_offset == 0
  nsresult res(NS_ERROR_FAILURE);
  nsCOMPtr<nsIDOMNode> child, dontcare;
  msiUtils::GetChildNode(m_mathmlNode, 0, child);
  if (child && msiUtils::IsInputbox(editor, child) && node)
  {
    res = m_mathmlNode->ReplaceChild(node, child, getter_AddRefs(dontcare));
    if (NS_SUCCEEDED(res))
    {
      nsCOMPtr<nsIMutableArray> mutableArray = do_CreateInstance(NS_ARRAY_CONTRACTID, &res);
      if (NS_SUCCEEDED(res))
      {
        res = mutableArray->AppendElement(m_mathmlNode, PR_FALSE);
        if (NS_SUCCEEDED(res))
        {
          *coalesced = mutableArray;
          NS_ADDREF(*coalesced);
        }
      }
    }
  }
  return res;
}    

nsresult msiScriptCoalesce::CoalesceRight(nsIEditor * editor, nsIDOMNode * node, nsIArray** coalesced)
{
  //ljh m_offset == m_maxOffset
  nsresult res(NS_ERROR_FAILURE);
  nsCOMPtr<nsIDOMNode> currSup, currSub, inSup, inSub, newSup, newSub, base;
  PRUint32 mmltype(MATHML_UNKNOWN);
  res = msiUtils::GetMathmlNodeType(editor, node, mmltype);
  if (NS_SUCCEEDED(res) && (mmltype == MATHML_MSUB || mmltype == MATHML_MSUP || mmltype == MATHML_MSUBSUP))
  {
    res = msiUtils::GetChildNode(m_mathmlNode, 0, base);
    if (NS_SUCCEEDED(res) && (mmltype == MATHML_MSUB || mmltype == MATHML_MSUBSUP))
      res = msiUtils::GetChildNode(node, 1, inSub);
    else if (mmltype == MATHML_MSUP)
      res = msiUtils::GetChildNode(node, 1, inSup);
    if (NS_SUCCEEDED(res) && mmltype == MATHML_MSUBSUP)
      res = msiUtils::GetChildNode(node, 2, inSup);
    
    if (m_mathmlType == MATHML_MSUB || m_mathmlType == MATHML_MSUBSUP)
      res = msiUtils::GetChildNode(m_mathmlNode, 1, currSub);
    else if (m_mathmlType == MATHML_MSUP)
      res = msiUtils::GetChildNode(m_mathmlNode, 1, currSup);
    if (NS_SUCCEEDED(res) && m_mathmlType == MATHML_MSUBSUP)
      res = msiUtils::GetChildNode(m_mathmlNode, 2, currSup);
      
    if (NS_SUCCEEDED(res) && (currSup || inSup))
      res = msiRequiredArgument::MakeRequiredArgument(editor, currSup, inSup, newSup);
    if (NS_SUCCEEDED(res) && (currSub || inSub))
      res = msiRequiredArgument::MakeRequiredArgument(editor, currSub, inSub, newSub);
    if (NS_SUCCEEDED(res) && base && (newSub || newSup))
    {
      nsAutoString subShift, supShift;
      msiScriptCoalesce::DetermineScriptShiftAttributes(node, subShift, supShift);
      PRUint32 dummyflags(0);
      nsCOMPtr<nsIDOMElement> newElement;
      if (newSub && newSup)
        res = msiUtils::CreateMSubSup(editor, base, newSub, newSup, PR_FALSE, PR_FALSE, PR_FALSE,
                                      dummyflags, subShift, supShift, newElement); 
      else if (newSub)
        res = msiUtils::CreateMSubOrMSup(editor, PR_FALSE, base, newSub, PR_FALSE, PR_FALSE, 
                                         dummyflags, subShift, newElement);
      else //newSup
        res = msiUtils::CreateMSubOrMSup(editor, PR_TRUE, base, newSup, PR_FALSE, PR_FALSE, 
                                         dummyflags, supShift, newElement);
      if (NS_SUCCEEDED(res) && newElement)
      {
        nsCOMPtr<nsIMutableArray> mutableArray = do_CreateInstance(NS_ARRAY_CONTRACTID, &res);
        if (NS_SUCCEEDED(res))
        {
          res = mutableArray->AppendElement(newElement, PR_FALSE);
          if (NS_SUCCEEDED(res))
          {
            *coalesced = mutableArray;
            NS_ADDREF(*coalesced);
          }
        }
      }  
    }
  }
  return res;
}  

nsresult msiScriptCoalesce::CoalesceLeft(nsIEditor * editor, nsIDOMNode * node, nsITransaction** txn)
{
  //ljh m_offset == 0
  nsCOMPtr<msiIMathMLEditor> msiEditor(do_QueryInterface(editor));
  if (!msiEditor)  
    return NS_ERROR_FAILURE;
  nsresult res(NS_OK);
  nsCOMPtr<nsIDOMNode> currbase;
  msiUtils::GetChildNode(m_mathmlNode, 0, currbase);
  if (!currbase || !m_mathmlNode || !txn || !node)
    return NS_ERROR_FAILURE;
  if (msiUtils::IsInputbox(editor, currbase))
  {
    nsCOMPtr<nsITransaction> replaceBaseTxn;
    res = msiEditor->CreateReplaceScriptBaseTransaction(m_mathmlNode, node, getter_AddRefs(replaceBaseTxn));
    if (NS_SUCCEEDED(res) && replaceBaseTxn)
    {
      *txn = replaceBaseTxn;
      NS_ADDREF(*txn);
    }
  }
  else
  {
    //TODO try to coalese node into currbase but not currbase into node
  }
  return res;
}    

void msiScriptCoalesce::DetermineScriptShiftAttributes(nsIDOMNode * node, nsAString & subShift, nsAString & supShift)
{
  nsCOMPtr<nsIDOMElement> currElement(do_QueryInterface(m_mathmlNode));
  nsCOMPtr<nsIDOMElement> newElement(do_QueryInterface(node));
  subShift.Truncate(0);
  supShift.Truncate(0);
  nsAutoString subscriptshift, superscriptshift;
  msiEditingAtoms::subscriptshift->ToString(subscriptshift);
  msiEditingAtoms::superscriptshift->ToString(superscriptshift);
  if (currElement)
  {
    currElement->GetAttribute(subscriptshift, subShift);
    currElement->GetAttribute(superscriptshift, supShift);
  }
  if (newElement)
  {
    if (subShift.IsEmpty())
      newElement->GetAttribute(subscriptshift, subShift);
    if (supShift.IsEmpty())
      newElement->GetAttribute(superscriptshift, supShift);
  }
  return;
}  

