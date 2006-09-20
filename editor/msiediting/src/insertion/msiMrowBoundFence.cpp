// Copyright (c) 2005, MacKichan Software, Inc.  All rights reserved.

#include "msiMrowBoundFence.h"
#include "msiUtils.h"
#include "msiCoalesceUtils.h"
#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "nsIEditor.h"
#include "nsISelection.h"
#include "nsISimpleEnumerator.h"
#include "nsIMutableArray.h"
#include "nsIArray.h"
#include "nsComponentManagerUtils.h"

msiMrowBoundFence::msiMrowBoundFence(nsIDOMNode* mathmlNode, PRUint32 offset)
:  msiMInsertionBase(mathmlNode, offset, MATHML_MROWBOUNDFENCE)
{
  
}

NS_IMETHODIMP
msiMrowBoundFence::InsertNode(nsIEditor * editor,
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
msiMrowBoundFence::InsertMath(nsIEditor * editor,
                     nsISelection  * selection, 
                     PRBool isDisplay,
                     nsIDOMNode * left,    
                     nsIDOMNode * right,    
                     PRUint32 flags)
{
  return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP 
msiMrowBoundFence::Inquiry(nsIEditor * editor,
                      PRUint32 inquiryID, 
                      PRBool * result)
{
  nsresult res(NS_OK);
  if (inquiryID == INSERT_DISPLAY || inquiryID == INSERT_INLINE_MATH)
    *result = PR_FALSE;
  else if (inquiryID == IS_MROW_REDUNDANT)
    *result = PR_FALSE; //TODO -- is this really the case
  else
    *result = PR_TRUE;
  return res;
}

NS_IMETHODIMP
msiMrowBoundFence::InsertNodes(nsIEditor * editor,
                          nsISelection * selection, 
                          nsIArray * nodeList,
                          PRBool  deleteExisting, 
                          PRUint32 flags)
{
  nsresult res(NS_ERROR_FAILURE);
  PRUint32 insertPos(0), numKids(0), numNewNodes(0), caretPos(0);
  PRBool doMRow(PR_FALSE);
  nsCOMPtr<nsIDOMNode> tobeRemoved;
  nsCOMPtr<msiIMathMLInsertion> mchildEditing;
  nsCOMPtr<nsIArray> nodeArray;
  
  if (nodeList)
    nodeList->GetLength(&numNewNodes);

  if (editor && selection && nodeList && m_mathmlNode && numNewNodes > 0)
  {
    msiUtils::GetNumberofChildren(m_mathmlNode, numKids);
    NS_ASSERTION(numKids<=3, "MrowBoundFence has too many children!");

    //RWA: Here's the deal. If there are only two children (the fences), insertion is allowed at position 1.
    //If there are more than three children, we're already (presumably) invalid and should reject any insertion (?).
    //If there are exactly three children and the middle one is an input box, the situation is the same as with two children.
    //In any other case, the insertion request should not come here! If it does, we should send it to our parent.

    //In the case where we have one input box child as our second child and m_offset is 1 or 2, or the case where 
    //"deleteExisting" is true and m_offset is 1, we're good to go. But if "deleteExisting" is true and the node to
    //be inserted isn't of the same type as the existing node, should we object?
    //What should we do with the flags?

    insertPos = DetermineInsertPosition(flags, deleteExisting);
    if (insertPos == 1)  //if insertPos is anything else, we won't even try - it just goes out to the parent
    {
      if (!deleteExisting) // check to see if there is an inputbox in the neighborhood, is so wack-it!
      {
        nsCOMPtr<nsIDOMNode> node;
        msiUtils::GetChildNode(m_mathmlNode, 1, node);
        if (node && msiUtils::IsInputbox(editor, node))
          deleteExisting = PR_TRUE;
      }
      if (deleteExisting)
        msiUtils::GetChildNode(m_mathmlNode, insertPos, tobeRemoved);
      
    }

    if (deleteExisting && insertPos==1)
    {
      nsCOMPtr<nsIArray> coalescedArray;
      nsCOMPtr<nsIArray> inList(nodeList);
      if (NS_SUCCEEDED(res) && inList)
      {
        if (inList)
          res = msiCoalesceUtils::CoalesceArray(editor, inList, coalescedArray);
        if (NS_SUCCEEDED(res) && coalescedArray)
        {
          nsCOMPtr<nsIDOMElement> mrow;
          nsCOMPtr<nsIDOMNode> newNode;
          coalescedArray->GetLength(&numNewNodes);
          if  (numNewNodes == 1)
          {
            res = coalescedArray->QueryElementAt(0, NS_GET_IID(nsIDOMNode), getter_AddRefs(newNode));
            if (NS_SUCCEEDED(res) && newNode)
            {
              if (tobeRemoved)
                editor->DeleteNode(tobeRemoved);
              res = editor->InsertNode(newNode, m_mathmlNode, 1);
              if (NS_SUCCEEDED(res))
              {  
                msiUtils::MarkCaretPosition(editor, m_mathmlNode, 1, flags, PR_FALSE, PR_FALSE);
                msiUtils::doSetCaretPosition(editor, selection, m_mathmlNode);
              }  
            }
          }
          else
            res = NS_ERROR_FAILURE;
        }
      }
    }
    if (!NS_SUCCEEDED(res))
    {
      nsCOMPtr<msiIMathMLInsertion> msiEditing;
      PRUint32 localFlags(flags);
      res = SetupPassToParent(editor, insertPos, msiEditing, localFlags);
      if (NS_SUCCEEDED(res) && msiEditing)
        res = msiEditing->InsertNodes(editor, selection, nodeList, PR_FALSE, localFlags);
    }
  }

  return res;
}

//private functions


PRUint32 msiMrowBoundFence::DetermineInsertPosition(PRUint32 flags, PRBool deleteExisting)
{
  PRUint32 insertPos(m_offset), numKids(0);
  msiUtils::GetNumberofChildren(m_mathmlNode, numKids);  //should be 3, in fact
  if (insertPos > numKids)
    insertPos = numKids;  
  return insertPos;
}

nsresult msiMrowBoundFence::SetupPassToParent(nsIEditor * editor,
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
      flags |= (insertPos > 1) ? FLAGS_RIGHTSIDE : FLAGS_LEFTSIDE;
    }  
  }
  return res;
}                                  
