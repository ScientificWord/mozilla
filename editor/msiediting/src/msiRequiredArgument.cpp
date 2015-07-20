// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiRequiredArgument.h"
#include "msiUtils.h"
#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "nsIEditor.h"
#include "nsISelection.h"
#include "msiMrow.h"
#include "msiCoalesceUtils.h"
#include "msiEditingAtoms.h"
#include "nsIMutableArray.h"
#include "nsComponentManagerUtils.h"

nsresult
msiRequiredArgument::doInsertNodes(nsIEditor * editor,
                                   nsISelection * selection,
                                   nsCOMPtr<nsIDOMNode> & parent,
                                   nsCOMPtr<nsIDOMNode> & requiredArg,
                                   PRBool atRight,
                                   nsIArray * nodeList,
                                   PRBool  deleteExisting,
                                   PRUint32 flags)
{
  nsresult res(NS_ERROR_FAILURE);
  PRUint32 numKids(0), numNodes(0), caretPos(0);
  PRBool doMRow(PR_FALSE);
  nsCOMPtr<nsIDOMNode> tobeDeleted;
  nsCOMPtr<msiIMathMLInsertion> mrowEditing;
  nsCOMPtr<nsIArray> nodeArray, listToParent;

  if (nodeList)
    nodeList->GetLength(&numNodes);
  if (editor && selection && nodeList && parent, requiredArg && numNodes > 0)
  {
    nsCOMPtr<nsIArray> coalescedArray;
    if (!deleteExisting && msiUtils::IsInputbox(editor, requiredArg))
      deleteExisting = PR_TRUE;
    if (deleteExisting)
      tobeDeleted = requiredArg;

    if (!deleteExisting && msiUtils::IsMrow(editor, requiredArg))
    {
      PRUint32 offset(0);
      if (atRight)
        msiUtils::GetNumberofChildren(requiredArg, offset);
      res = msiUtils::GetMathMLInsertionInterface(editor, requiredArg, offset, mrowEditing);
    }

    // Act on action determined above
    if (mrowEditing)
      res = mrowEditing->InsertNodes(editor, selection, nodeList, PR_FALSE, flags);
    else
    {
      if (!tobeDeleted)
      {
        PRUint32 pfcFlags(msiIMathMLCoalesce::PFCflags_removeRedundantMrows);
        nsCOMPtr<nsIDOMNode> clone;
        nsCOMPtr<nsIArray> addToList;
        nsCOMPtr<nsIArray> inList(nodeList);
        nsCOMPtr<nsIMutableArray> mutableArray = do_CreateInstance(NS_ARRAY_CONTRACTID, &res);
        if (NS_SUCCEEDED(res) && inList)
        {
          tobeDeleted = requiredArg;
          res = msiUtils::CloneNode(tobeDeleted, clone);
          if (NS_SUCCEEDED(res) && clone)
          {
            if (atRight)
            {
              res = msiCoalesceUtils::PrepareForCoalesceFromRight(editor, clone, pfcFlags, addToList);
              if (NS_SUCCEEDED(res) && addToList)
                 res = msiUtils::AppendToMutableList(mutableArray, addToList);
            }
            if (NS_SUCCEEDED(res))
               res = msiUtils::AppendToMutableList(mutableArray, inList);

            if (!atRight)
            {
              res = msiCoalesceUtils::PrepareForCoalesceFromLeft(editor, clone, pfcFlags, addToList);
              if (NS_SUCCEEDED(res) && addToList)
                res = msiUtils::AppendToMutableList(mutableArray, addToList);
            }
            if (NS_SUCCEEDED(res))
              nodeArray = do_QueryInterface(mutableArray);
          }
        }
      }
      else
        nodeArray = nodeList;

      if (nodeArray)
        res = msiCoalesceUtils::CoalesceArray(editor, nodeArray, coalescedArray);
      if (NS_SUCCEEDED(res) && coalescedArray)
      {
        nsCOMPtr<nsIDOMElement> mrow;
        nsCOMPtr<nsIDOMNode> newNode, caretNode;
        coalescedArray->GetLength(&numNodes);
        if (numNodes > 1 )
        {
          res = msiUtils::CreateMRow(editor, coalescedArray, mrow);
          if (NS_SUCCEEDED(res) && mrow)
          {
            newNode = do_QueryInterface(mrow);
            caretNode = newNode;
          }
        }
        else if  (numNodes == 1)
        {
          res = coalescedArray->QueryElementAt(0, NS_GET_IID(nsIDOMNode), getter_AddRefs(newNode));
          if (NestRequiredArgumentInMrow(newNode))
          {
            res = msiUtils::CreateMRow(editor, newNode, mrow);
            if (NS_SUCCEEDED(res) && mrow)
            {
              newNode = do_QueryInterface(mrow);
              caretNode = newNode;
            }
          }
          else
            caretNode = parent;
        }
        if (NS_SUCCEEDED(res) && newNode)
        {
          res = editor->ReplaceNode(newNode, tobeDeleted, parent);
          // if (NS_SUCCEEDED(res))
          //   msiUtils::MarkCaretPosition(editor, caretNode, numNodes, flags, PR_FALSE, PR_FALSE);
          msiUtils::doSetCaretPosition(editor, selection, caretNode);
        }
      }
    }
  }
  return res;
}

nsresult msiRequiredArgument::MakeRequiredArgument(nsIEditor * editor,
                                                   nsIDOMNode * leftNode,
                                                   nsIDOMNode * rightNode,
                                                   nsCOMPtr<nsIDOMNode> & argument)
{
  nsresult res(NS_ERROR_FAILURE);
  if (leftNode || rightNode)
  {
    res = NS_OK;
    nsCOMPtr<nsIMutableArray> mutableArray = do_CreateInstance(NS_ARRAY_CONTRACTID, &res);
    NS_ENSURE_SUCCESS(res, res);
    nsCOMPtr<nsIArray> leftArray, rightArray, tobe, coalesced;
    PRUint32 pfcFlags(msiIMathMLCoalesce::PFCflags_removeRedundantMrows);
    if (leftNode)
    {
      res = msiCoalesceUtils::PrepareForCoalesceFromRight(editor, leftNode, pfcFlags, leftArray);
      if (NS_SUCCEEDED(res))
         res = msiUtils::AppendToMutableList(mutableArray, leftArray);
    }
    if (NS_SUCCEEDED(res) && rightNode)
    {
      res = msiCoalesceUtils::PrepareForCoalesceFromLeft(editor, rightNode, pfcFlags, rightArray);
      if (NS_SUCCEEDED(res))
         res = msiUtils::AppendToMutableList(mutableArray, rightArray);
    }
    if (NS_SUCCEEDED(res))
      tobe = do_QueryInterface(mutableArray);
    if (tobe)
      res = msiCoalesceUtils::CoalesceArray(editor, tobe, coalesced);
    if (NS_SUCCEEDED(res) && coalesced)
    {
      nsCOMPtr<nsIDOMElement> mrow;
      PRUint32 numNodes(0);
      coalesced->GetLength(&numNodes);
      if (numNodes > 1 )
      {
        res = msiUtils::CreateMRow(editor, coalesced, mrow);
        if (NS_SUCCEEDED(res) && mrow)
          argument = do_QueryInterface(mrow);
      }
      else if  (numNodes == 1)
      {
        res = coalesced->QueryElementAt(0, NS_GET_IID(nsIDOMNode), getter_AddRefs(argument));
        if (NestRequiredArgumentInMrow(argument))
        {
          res = msiUtils::CreateMRow(editor, argument, mrow);
          if (NS_SUCCEEDED(res) && mrow)
            argument = do_QueryInterface(mrow);
        }
      }
      if (!argument)
        res = NS_ERROR_FAILURE;
    }
  }
  return res;
}




PRBool msiRequiredArgument::NestRequiredArgumentInMrow(nsIDOMNode * node)
{
  PRBool rv(PR_TRUE);
  nsAutoString localName;
  if (node)
    node->GetLocalName(localName);
  if (!localName.IsEmpty())
  {
    if (msiEditingAtoms::mrow->Equals(localName)   ||
        msiEditingAtoms::mo->Equals(localName)     ||
        msiEditingAtoms::mn->Equals(localName)     ||
        msiEditingAtoms::mi->Equals(localName)     ||
        msiEditingAtoms::mtext->Equals(localName)  ||
        msiEditingAtoms::mstyle->Equals(localName) ||  //TODO ??
        msiEditingAtoms::ms->Equals(localName)     ||  //TODO ??
        msiEditingAtoms::mspace->Equals(localName)  )   //TODO ??
       rv = PR_FALSE;
  }
  return rv;
}



