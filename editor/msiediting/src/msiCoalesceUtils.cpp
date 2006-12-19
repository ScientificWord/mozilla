// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "nsIEditor.h"
#include "nsIDOMNode.h"
#include "msiIMathMLEditor.h"
#include "nsIMutableArray.h"
#include "nsIArray.h"
#include "nsISimpleEnumerator.h"
#include "nsComponentManagerUtils.h"

#include "msiUtils.h"
#include "msiCoalesceUtils.h"
#include "msiMMLEditDefines.h"
#include "msiIMathMLCoalesce.h"
#include "msiEditingAtoms.h"


nsresult msiCoalesceUtils::GetMathMLCoalesceInterface(nsIEditor *editor,
                                                      nsIDOMNode * node,
                                                      PRUint32   offset,
                                                      nsCOMPtr<msiIMathMLCoalesce> & msiEditing)
{
  nsresult res(NS_ERROR_FAILURE);
  if (editor && node)
  {
    nsCOMPtr<msiIMathMLEditor> msiEditor(do_QueryInterface(editor));
    if (msiEditor)
      res = msiEditor->GetMathMLCoalesceInterface(node, offset, getter_AddRefs(msiEditing));
  }
  return res;
}                  

nsresult msiCoalesceUtils::PrepareForCoalesce(nsIEditor * editor,
                                               nsIDOMNode * node,
                                               PRUint32 offset,
                                               PRUint32 pfcFlags,
                                               nsCOMPtr<nsIArray> & beforeOffset,                              
                                               nsCOMPtr<nsIArray> & afterOffset)
{
  nsresult res(NS_ERROR_FAILURE);
  nsCOMPtr<msiIMathMLCoalesce> msiEditing;
  res = GetMathMLCoalesceInterface(editor, node, offset, msiEditing);
  if (NS_SUCCEEDED(res) && msiEditing)
    res = msiEditing->PrepareForCoalesce(editor, pfcFlags, getter_AddRefs(beforeOffset),
                                          getter_AddRefs(afterOffset));
  return res;
}                                               

nsresult msiCoalesceUtils:: PrepareForCoalesceFromRight(nsIEditor * editor,
                                                        nsIDOMNode * node,
                                                        PRUint32 pfcFlags,
                                                        nsCOMPtr<nsIArray> & preparedArray)
{
  nsresult res(NS_ERROR_FAILURE);
  nsCOMPtr<msiIMathMLCoalesce> msiEditing;
  nsCOMPtr<nsIArray> dummy, before, after;
  PRUint32 offset(0);
  if (editor && node)
  {  
    msiUtils::GetRightMostCaretPosition(editor, node, offset);
    res = GetMathMLCoalesceInterface(editor, node, offset, msiEditing);
    if (NS_SUCCEEDED(res) && msiEditing)
      res = msiEditing->PrepareForCoalesce(editor, pfcFlags, getter_AddRefs(before),
                                            getter_AddRefs(after));
    if (NS_SUCCEEDED(res))
      res  = msiUtils::AddToNodeList(dummy, before, after, preparedArray);
  }
  return res;
}   

nsresult msiCoalesceUtils:: PrepareForCoalesceFromLeft(nsIEditor * editor,
                                                       nsIDOMNode * node,
                                                       PRUint32 pfcFlags,
                                                       nsCOMPtr<nsIArray> & preparedArray)
{
  nsresult res(NS_ERROR_FAILURE);
  nsCOMPtr<msiIMathMLCoalesce> msiEditing;
  nsCOMPtr<nsIArray> dummy, before, after;
  PRUint32 offset(0);
  if (editor && node)
  {  
    res = GetMathMLCoalesceInterface(editor, node, offset, msiEditing);
    if (NS_SUCCEEDED(res) && msiEditing)
      res = msiEditing->PrepareForCoalesce(editor, pfcFlags, getter_AddRefs(before),
                                            getter_AddRefs(after));
    if (NS_SUCCEEDED(res))
      res  = msiUtils::AddToNodeList(dummy, before, after, preparedArray);
  }
  return res;
}                                               
                                            


PRBool msiCoalesceUtils::Coalesce(nsIEditor * editor,
                                  nsIDOMNode * node1, 
                                  nsIDOMNode * node2, 
                                  nsCOMPtr<nsIArray> & coalesced)
{
  PRBool ret(PR_FALSE);
  nsresult res(NS_ERROR_FAILURE);
  coalesced = nsnull;
  if (editor && node1 && node2)
  {
    nsCOMPtr<msiIMathMLCoalesce> msiCoal;
    PRUint32 offset(0);
    
    msiUtils::GetRightMostCaretPosition(editor, node1, offset);
    GetMathMLCoalesceInterface(editor, node1, offset, msiCoal);
    if (msiCoal)
      msiCoal->Coalesce(editor, node2, getter_AddRefs(coalesced));
    if (!coalesced)
    {
      msiCoal = nsnull;
      GetMathMLCoalesceInterface(editor, node2, 0, msiCoal);
      if (msiCoal)
        msiCoal->Coalesce(editor, node1, getter_AddRefs(coalesced));
    }
    if (coalesced)
      ret = PR_TRUE;
  }
  return ret;
}

void msiCoalesceUtils::SetCoalesceSwitch(nsIArray * inArray, PRUint32 index, PRBool start)
{
  nsCOMPtr<nsIDOMNode> node;
  if (inArray)
    inArray->QueryElementAt(index, NS_GET_IID(nsIDOMNode), getter_AddRefs(node));
  return SetCoalesceSwitch(node, start);
}
 
 
void msiCoalesceUtils::SetCoalesceSwitch(nsCOMPtr<nsIDOMNode> & node, PRBool start)
{
  nsCOMPtr<nsIDOMElement> element(do_QueryInterface(node));
  nsAutoString cswitch, cstart, cstop;
  msiEditingAtoms::coalesceswitch->ToString(cswitch);
  msiEditingAtoms::coalescestart->ToString(cstart);  
  msiEditingAtoms::coalescestop->ToString(cstop);  
  if (element && start)
    element->SetAttribute(cswitch, cstart);
  else if (element)    
    element->SetAttribute(cswitch, cstop);
  return;  
} 


void _set_coalesing_stopped(nsCOMPtr<nsIDOMNode> node, PRBool & coalescingStopped)
{
  PRBool hasIt(PR_FALSE);
  nsCOMPtr<nsIDOMElement> element(do_QueryInterface(node));
  nsAutoString coalesceswitch;
  msiEditingAtoms::coalesceswitch->ToString(coalesceswitch);
  if (element)
    element->HasAttribute(coalesceswitch, &hasIt);
  if (hasIt)
  {
    nsAutoString value;
    element->GetAttribute(coalesceswitch, value);
    if (msiEditingAtoms::coalescestop->Equals(value))
      coalescingStopped = PR_TRUE;
    else if (msiEditingAtoms::coalescestart->Equals(value))
      coalescingStopped = PR_FALSE;
    element->RemoveAttribute(coalesceswitch);
  }
  return;
}

nsresult msiCoalesceUtils::CoalesceArray(nsIEditor * editor,
                                         nsIArray  * inputArray,
                                         nsCOMPtr<nsIArray>  & coalescedArray)
{
  nsresult res(NS_ERROR_FAILURE);
  coalescedArray = nsnull;
  nsCOMPtr<nsIMutableArray> mutableArray = do_CreateInstance(NS_ARRAY_CONTRACTID, &res);
  NS_ENSURE_SUCCESS(res, res);
  PRUint32 inputLength(0);
  if (inputArray)
    inputArray->GetLength(&inputLength);
  if (NS_SUCCEEDED(res) && editor && inputLength > 0 && mutableArray)
  {
    PRUint32 index(0);
    PRBool coalescing(PR_TRUE);
    PRBool coalescingStopped(PR_FALSE);
    while (index < inputLength && NS_SUCCEEDED(res))
    {
      nsCOMPtr<nsIDOMNode> node, coalescedNode;
      res = inputArray->QueryElementAt(index, NS_GET_IID(nsIDOMNode), getter_AddRefs(node));
      index += 1;
      if (NS_SUCCEEDED(res) && node)
      {
        _set_coalesing_stopped(node, coalescingStopped);
        coalescing = !coalescingStopped;
        while (index < inputLength && coalescing && NS_SUCCEEDED(res))
        {
          nsCOMPtr<nsIDOMNode> node1;
          res = inputArray->QueryElementAt(index, NS_GET_IID(nsIDOMNode), getter_AddRefs(node1));
          if (NS_SUCCEEDED(res) && node1)
          {
             _set_coalesing_stopped(node1, coalescingStopped);
             nsCOMPtr<nsIArray> coalesced;
             PRUint32 len(0);
             if (Coalesce(editor, node, node1, coalesced) && coalesced)
               coalesced->GetLength(&len);
             if (len > 0)
             {
               for(PRUint32 i=0; i+1 < len; i++)
               {
                 nsCOMPtr<nsIDOMNode> currNode;
                 res = coalesced->QueryElementAt(i, NS_GET_IID(nsIDOMNode), getter_AddRefs(currNode));
                 if (NS_SUCCEEDED(res) && currNode)
                   res = mutableArray->AppendElement(currNode, PR_FALSE);
               }
               nsCOMPtr<nsIDOMNode> currNode;
               res = coalesced->QueryElementAt(len-1, NS_GET_IID(nsIDOMNode), getter_AddRefs(currNode));
               node = currNode;
               index += 1;
             }
             else
               coalescing = PR_FALSE;
          }
        }
        if (NS_SUCCEEDED(res) && node)
           res = mutableArray->AppendElement(node, PR_FALSE);
      }
    }
  }
  if (NS_SUCCEEDED(res))
    coalescedArray = do_QueryInterface(mutableArray);
  if (!coalescedArray)
    res = NS_ERROR_FAILURE;
  return res;
}    


//Transaction based coalescing
PRBool msiCoalesceUtils::Coalesce(nsIEditor * editor, nsIDOMNode * left,  
                                  nsIDOMNode * right, nsCOMPtr<nsITransaction> & txn)
{
  PRBool ret(PR_FALSE);
  nsresult res(NS_ERROR_FAILURE);
  txn = nsnull;
  if (editor && left && right)
  {
    nsCOMPtr<msiIMathMLCoalesce> msiCoal;
    PRUint32 offset(msiIMathMLEditingBC::RIGHT_MOST);
    GetMathMLCoalesceInterface(editor, left, offset, msiCoal);
    if (msiCoal)
      msiCoal->CoalesceTxn(editor, right, getter_AddRefs(txn));
    if (!txn)
    {
      msiCoal = nsnull;
      GetMathMLCoalesceInterface(editor, right, 0, msiCoal);
      if (msiCoal)
        msiCoal->CoalesceTxn(editor, left, getter_AddRefs(txn));
    }
    if (txn)
      ret = PR_TRUE;
  }
  return ret;
}                                    
                                     

// Should only be called when caretpos is known to be associated to node.
nsresult msiCoalesceUtils::ForceCaretPositionMark(nsIDOMNode * node, 
                                                  PRUint32 pos, 
                                                  PRBool caretOnText)
{
  nsresult res(NS_ERROR_FAILURE);
  if (node && (pos <= msiIMathMLEditingBC::LAST_VALID || 
               pos == msiIMathMLEditingBC::TO_LEFT    ||
               pos == msiIMathMLEditingBC::TO_RIGHT))
  {
    nsCOMPtr<nsIDOMElement> element(do_QueryInterface(node));
    if (element)
    {
      nsAutoString msicaretpos, msicaretpostext;
      msiEditingAtoms::msicaretpos->ToString(msicaretpos);
      msiEditingAtoms::msicaretpostext->ToString(msicaretpostext);
      element->RemoveAttribute(msicaretpos);
      element->RemoveAttribute(msicaretpostext);
      nsAutoString value, attribute;
      value.AppendInt(pos);
      if (caretOnText)
        attribute = msicaretpostext;
      else  
        attribute = msicaretpos;
      res = element->SetAttribute(attribute, value);
    }
  }  
  return res;
}

nsresult msiCoalesceUtils::ForceRemovalOfCaretMark(nsIDOMNode * node)
{
  nsresult res(NS_ERROR_FAILURE);
  if (node)
  {
    nsCOMPtr<nsIDOMElement> element(do_QueryInterface(node));
    if (element)
    {
      res = NS_OK;
      nsAutoString msicaretpos, msicaretpostext;
      msiEditingAtoms::msicaretpos->ToString(msicaretpos);
      msiEditingAtoms::msicaretpostext->ToString(msicaretpostext);
      element->RemoveAttribute(msicaretpos);
      element->RemoveAttribute(msicaretpostext);
    }
  }
  return res;
}    

