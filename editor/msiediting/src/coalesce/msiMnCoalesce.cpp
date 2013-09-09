// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiMnCoalesce.h"
#include "msiUtils.h"
#include "msiCoalesceUtils.h"
#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "nsIEditor.h"
#include "nsIMutableArray.h"
#include "nsIDOMText.h"
#include "nsIDOMCharacterData.h"
#include "nsIMutableArray.h"
#include "nsComponentManagerUtils.h"

msiMnCoalesce::msiMnCoalesce(nsIDOMNode* mathmlNode, 
                                   PRUint32 offset) 
: msiMCoalesceBase(mathmlNode, offset, MATHML_MN)
{
  m_length = 0;
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
  
msiMnCoalesce::~msiMnCoalesce()
{
}

NS_IMETHODIMP
msiMnCoalesce::Coalesce(nsIEditor * editor,
                        nsIDOMNode * node,
                        nsIArray ** coalesced)                
{
  nsresult res(NS_ERROR_FAILURE);
  *coalesced = nsnull;
  if (node && editor && m_mathmlNode)
  {
    res = NS_OK;
    PRUint32 newTextLen(0);
    PRUint32 nodetype(MATHML_UNKNOWN);
    res = msiUtils::GetMathmlNodeType(editor, node, nodetype);
    nsAutoString newText;
    nsCOMPtr<nsIDOMNode> clone;
    if (NS_SUCCEEDED(res) && nodetype == msiIMathMLEditingBC::MATHML_MN)
    {
      nsCOMPtr<nsIDOMNode> kid;
      nsCOMPtr<nsIDOMCharacterData> cDataNode;
      node->GetFirstChild(getter_AddRefs(kid));
      if (kid)
        cDataNode = do_QueryInterface(kid);
      if (cDataNode)
        cDataNode->GetLength(&newTextLen);
      if (newTextLen > 0)
      {
        cDataNode->GetData(newText);
        newTextLen = newText.Length();
      }
      else 
        newTextLen = 0;
      if (newTextLen > 0)
        msiUtils::CloneNode(m_mathmlNode, clone);
    }
	else if (NS_SUCCEEDED(res) && nodetype == msiIMathMLEditingBC::MATHML_MO){
	  //TODO  coalesce . or , into MN
	  nsCOMPtr<nsIDOMNode> kid;
      nsCOMPtr<nsIDOMCharacterData> cDataNode;
      node->GetFirstChild(getter_AddRefs(kid));
      if (kid)
        cDataNode = do_QueryInterface(kid);
      if (cDataNode)
        cDataNode->GetLength(&newTextLen);
      if (newTextLen > 0)
      {
        cDataNode->GetData(newText);
        newTextLen = newText.Length();
		if (newText[0] != '.')
			newTextLen = 0;
      }
      else 
        newTextLen = 0;

      if (newTextLen > 0)
        msiUtils::CloneNode(m_mathmlNode, clone);


	}
    if (NS_SUCCEEDED(res) && clone && newTextLen > 0)
    {
      nsCOMPtr<nsIDOMNode> child;
      res = clone->GetFirstChild(getter_AddRefs(child));
      if (NS_SUCCEEDED(res) && child && newTextLen > 0)
      {
        nsCOMPtr<nsIDOMCharacterData> charDataNode(do_QueryInterface(child));
        if (charDataNode)
        {
          res = charDataNode->InsertData(m_offset, newText);
          if (NS_SUCCEEDED(res))
          {
            //Adjust caretPos if need be.
            PRUint32 caretPos(msiIMathMLEditingBC::INVALID), newCaretPos(msiIMathMLEditingBC::INVALID);
            PRBool onText(PR_FALSE);
            PRBool nodeHasMark = msiUtils::NodeHasCaretMark(node, caretPos, onText);
            PRUint32 oldRtMostPos(0);
            msiUtils::GetRightMostCaretPosition(editor, m_mathmlNode, oldRtMostPos);
            if (nodeHasMark && caretPos <= msiIMathMLEditingBC::LAST_VALID && caretPos > m_offset)
                newCaretPos += newTextLen;
            else if (nodeHasMark)
            {
              if (caretPos == msiIMathMLEditingBC::TO_LEFT)
                newCaretPos = m_offset;
              else if (caretPos == msiIMathMLEditingBC::TO_RIGHT)
              {
                if (m_offset == oldRtMostPos)
                  newCaretPos = caretPos;
                else
                  newCaretPos = m_offset + newTextLen;
              }
              else
                newCaretPos = m_offset + caretPos;
            }
            else {
              nodeHasMark = msiUtils::NodeHasCaretMark(m_mathmlNode, caretPos, onText);
              newCaretPos = caretPos + newTextLen;
            }
            if (newCaretPos <= msiIMathMLEditingBC::LAST_VALID ||
                newCaretPos <= msiIMathMLEditingBC::TO_LEFT    ||
                newCaretPos <= msiIMathMLEditingBC::TO_RIGHT)
            {
              onText = newCaretPos <= msiIMathMLEditingBC::LAST_VALID ? PR_TRUE : PR_FALSE;
              msiCoalesceUtils::ForceCaretPositionMark(clone, newCaretPos, onText);
            }
            nsCOMPtr<nsIMutableArray> mutableArray = do_CreateInstance(NS_ARRAY_CONTRACTID, &res);
            if (NS_SUCCEEDED(res) && mutableArray)
            {
              mutableArray->AppendElement(clone, PR_FALSE);
              *coalesced = mutableArray;
              NS_ADDREF(*coalesced);
            }  
          }  
        }
      }
    }
  }
  return res;
}

NS_IMETHODIMP
msiMnCoalesce::CoalesceTxn(nsIEditor * editor,
                              nsIDOMNode * node,
                              nsITransaction ** txn)                
{
  *txn = nsnull;
  return NS_OK;
}


NS_IMETHODIMP
msiMnCoalesce::PrepareForCoalesce(nsIEditor * editor,
                                  PRUint32    pfcFlags,
                                  nsIArray ** beforeOffset,                
                                  nsIArray ** afterOffset)                
{
  *beforeOffset = nsnull;
  *afterOffset = nsnull;
  nsresult res(NS_ERROR_FAILURE);
  nsCOMPtr<nsIDOMNode> left, right;
  if (m_length == 0)
  {
    nsCOMPtr<nsIMutableArray> mutableArray = do_CreateInstance(NS_ARRAY_CONTRACTID, &res);
    if (NS_SUCCEEDED(res))
    {
      *beforeOffset = mutableArray;  //return valid but empty array
      NS_ADDREF(*beforeOffset);
    }  
  }
  else if (m_offset == 0)
  {
    right = do_QueryInterface(m_mathmlNode);
    if (right)
      res = NS_OK;
  }
  else if (m_offset == m_length)  
  {
    left = do_QueryInterface(m_mathmlNode);
    if (left)
      res = NS_OK;
  }
  else // 0 < m_offset < m_length)
  {
    left = do_QueryInterface(m_mathmlNode);
    res = msiUtils::CloneNode(m_mathmlNode, right);
    if (NS_SUCCEEDED(res) && left)
    {
      nsCOMPtr<nsIDOMNode> child;
      res = left->GetFirstChild(getter_AddRefs(child));
      if (NS_SUCCEEDED(res) && child)
      {
        nsCOMPtr<nsIDOMCharacterData> charData(do_QueryInterface(child));
        if (charData)
          res = charData->DeleteData(m_offset, m_length-m_offset);
      }
    }  
    if (NS_SUCCEEDED(res) && right)
    {
      nsCOMPtr<nsIDOMNode> child;
      res = right->GetFirstChild(getter_AddRefs(child));
      if (NS_SUCCEEDED(res) && child)
      {
        nsCOMPtr<nsIDOMCharacterData> charData(do_QueryInterface(child));
        if (charData)
          res = charData->DeleteData(0, m_offset);
      }  
    }
  }
  if (NS_SUCCEEDED(res) && left)
  {
    nsCOMPtr<nsIMutableArray> mutableArray = do_CreateInstance(NS_ARRAY_CONTRACTID, &res);
    if (NS_SUCCEEDED(res) && mutableArray)
      res = mutableArray->AppendElement(left, PR_FALSE);
    if (NS_SUCCEEDED(res))
    {
      *beforeOffset = mutableArray;
      NS_ADDREF(*beforeOffset);
    }  
  }
  if (NS_SUCCEEDED(res) && right)
  {
    nsCOMPtr<nsIMutableArray> mutableArray = do_CreateInstance(NS_ARRAY_CONTRACTID, &res);
    if (NS_SUCCEEDED(res) && mutableArray)
      res = mutableArray->AppendElement(right, PR_FALSE);
    if (NS_SUCCEEDED(res))
    {
      *afterOffset = mutableArray;
      NS_ADDREF(*afterOffset);
    }  
  }
  return res;
}
