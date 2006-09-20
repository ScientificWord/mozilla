// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.
#include "msiMrow.h"
#include "msiMContainer.h"
#include "nsIDOMNode.h"
#include "nsIDOMNodeList.h"
#include "nsCOMPtr.h"
#include "msiUtils.h"   
#include "msiCoalesceUtils.h"   
#include "msiMrowEditingImp.h"   

msiMrow::msiMrow(nsIDOMNode* mathmlNode, PRUint32 offset) 
:  msiMContainer(mathmlNode, offset, MATHML_MROW)
{
  MSI_NewMrowEditingImp(mathmlNode, getter_AddRefs(m_mrowEditingImp));
}

// ISupports
NS_IMPL_ISUPPORTS_INHERITED1(msiMrow, msiMContainer, msiIMrowEditing)


NS_IMETHODIMP
msiMrow::InsertMath(nsIEditor * editor,
                    nsISelection * selection, 
                    PRBool isDisplay,
                    nsIDOMNode * inleft,
                    nsIDOMNode * inright,
                    PRUint32 flags)
{
  nsresult res(NS_ERROR_FAILURE);
  if (editor && m_mathmlNode)
  {
    if (inleft || inright) 
    {// the node at m_offset has been split into left and right. Replace it with left and right and set m_offset to point between them.
      nsCOMPtr<nsIDOMNodeList> children;
      nsCOMPtr<nsIDOMNode> childToReplace;
      m_mathmlNode->GetChildNodes(getter_AddRefs(children));
      if (children) 
        children->Item(m_offset, getter_AddRefs(childToReplace));
      if (childToReplace)
      {
        if (inleft && inright)
        {
          editor->ReplaceNode(inleft, childToReplace, m_mathmlNode);
          m_offset += 1; 
          editor->InsertNode(inright, m_mathmlNode, m_offset);
        }
        else if (inleft)
        {
          m_offset += 1; 
        }
      }  
    }
    nsCOMPtr<nsIDOMNode> left, right;
    res = Split(left, right);
    if (NS_SUCCEEDED(res))
    {
      nsCOMPtr<msiIMathMLInsertion> msiEditing;
      res = msiUtils::SetupPassOffInsertToParent(editor, m_mathmlNode, PR_FALSE, msiEditing);
      if (NS_SUCCEEDED(res) && msiEditing)
        res = msiEditing->InsertMath(editor, selection, isDisplay, left, right, flags);
    }
  }
  return res;
}

NS_IMETHODIMP
msiMrow::InsertNodes(nsIEditor * editor,
                     nsISelection * selection, 
                     nsIArray * nodeList,
                     PRBool  deleteExisting, 
                     PRUint32 flags)
{
  nsresult res(NS_ERROR_FAILURE);
  if (m_mathmlNode && editor)
  {
    PRBool isRedundant(PR_FALSE);
    PRUint32 insertPos = DetermineInsertPosition(flags, deleteExisting);
    PRUint32 numKids(0);
    msiUtils::GetNumberofChildren(m_mathmlNode, numKids);
    nsCOMPtr<msiIMathMLInsertion> msiEditing;
    res = msiUtils::SetupPassOffInsertToParent(editor, m_mathmlNode, PR_FALSE, msiEditing);
    if (NS_SUCCEEDED(res) && msiEditing)
    {
      msiEditing->Inquiry(editor, IS_MROW_REDUNDANT, &isRedundant);
      if (isRedundant)
      {
        PRBool offsetOnBoundary = (insertPos == 0 || insertPos == numKids);
        IsRedundant(editor, offsetOnBoundary, &isRedundant);
      }  
    }
    if (isRedundant)
    {
      nsCOMPtr<nsIDOMNode> clone;
      nsCOMPtr<nsIArray> addToFront, addToEnd;
      nsCOMPtr<nsIArray> nodeArray;
      PRUint32 numNewNodes(0);
      res = msiUtils::CloneNode(m_mathmlNode, clone);
      if (nodeList)
        nodeList->GetLength(&numNewNodes);
      if (NS_SUCCEEDED(res) && clone && numNewNodes > 0)
      {
        if (NS_SUCCEEDED(res) && !deleteExisting) // check to see if there is an inputbox in the neighborhood, is so wack it!
        {
          if (insertPos > 0 )
          {
            nsCOMPtr<nsIDOMNode> node;
            res = msiUtils::GetChildNode(clone, insertPos-1, node);
            if (NS_SUCCEEDED(res) && node && msiUtils::IsInputbox(editor, node))
            {
              insertPos -= 1;
              deleteExisting = PR_TRUE;
            } 
          }
          if (!deleteExisting && insertPos < numKids)
          {
            nsCOMPtr<nsIDOMNode> node;
            res = msiUtils::GetChildNode(clone, insertPos, node);
            if (NS_SUCCEEDED(res) && node && msiUtils::IsInputbox(editor, node))
              deleteExisting = PR_TRUE;
          }
        }                                                      
        if (NS_SUCCEEDED(res) && deleteExisting)
        {
          nsCOMPtr<nsIDOMNode> dontcare;
          res = msiUtils::RemoveChildNode(clone, insertPos, dontcare);
        }
        res = msiUtils::GetNumberofChildren(clone, numKids);
        if (NS_SUCCEEDED(res) && numKids)
        {
          PRUint32 pfcFlags(msiIMathMLCoalesce::PFCflags_removeRedundantMrows);
          res = msiCoalesceUtils::PrepareForCoalesce(editor, clone, insertPos, pfcFlags, addToFront, addToEnd);
          if (NS_SUCCEEDED(res))
          { 
            PRUint32 len(0);
            if (addToFront)
              addToFront->GetLength(&len);
            if (len >= 2)
            {
              msiCoalesceUtils::SetCoalesceSwitch(addToFront, 0, PR_FALSE);
              msiCoalesceUtils::SetCoalesceSwitch(addToFront, len-1, PR_TRUE);
            }
            if (numNewNodes >= 2)
              msiCoalesceUtils::SetCoalesceSwitch(nodeList, numNewNodes-1, PR_TRUE);
            len = 0;  
            if (addToEnd)
              addToEnd->GetLength(&len);
            if (len >= 2)
              msiCoalesceUtils::SetCoalesceSwitch(addToEnd, 0, PR_FALSE);
            res = msiUtils::AddToNodeList(nodeList, addToFront, addToEnd, nodeArray);
          }
          if (NS_SUCCEEDED(res) && nodeArray)
          {
            flags &= ~(FLAGS_RIGHTSIDE|FLAGS_LEFTSIDE);
            flags |= FLAGS_FROM_NODE | FLAGS_LEFTSIDE;
            if (msiEditing)
              res = msiEditing->InsertNodes(editor, selection, nodeArray, PR_TRUE, flags);
          }
        }      
      }    
    }
    else
      res = msiMContainer::InsertNodes(editor, selection, nodeList, deleteExisting, flags);
  }
  return res;
}


