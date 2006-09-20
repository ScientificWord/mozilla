// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiInputboxCoalesce.h"
#include "msiUtils.h"
#include "msiCoalesceUtils.h"
#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "nsIEditor.h"
#include "nsIMutableArray.h"
#include "nsComponentManagerUtils.h"

msiInputboxCoalesce::msiInputboxCoalesce(nsIDOMNode* mathmlNode, 
                                         PRUint32 offset) 
: msiMCoalesceBase(mathmlNode, offset, MSI_INPUTBOX)
{
}
  
msiInputboxCoalesce::~msiInputboxCoalesce()
{
}

NS_IMETHODIMP
msiInputboxCoalesce::Coalesce(nsIEditor * editor,
                              nsIDOMNode * node,
                              nsIArray ** coalesced)                
{
  nsresult res(NS_ERROR_FAILURE);
  if (editor && m_mathmlNode && node)
  {
    PRUint32 dontcare(0);
    PRBool  dontcare1(PR_FALSE);
    if (msiUtils::NodeHasCaretMark(m_mathmlNode, dontcare, dontcare1))
    {
      PRBool onText = msiUtils::IsMleaf(editor, node, PR_TRUE);
      PRUint32 position(0);
      if (msiUtils::IsInputbox(editor, node))
        position = 1;
      else if (m_offset == 0)
        msiUtils::GetRightMostCaretPosition(editor, node, position);  
      msiCoalesceUtils::ForceCaretPositionMark(node, position, onText);
    }
    nsCOMPtr<nsIMutableArray> mutableArray = do_CreateInstance(NS_ARRAY_CONTRACTID, &res);
    if (NS_SUCCEEDED(res))
    {
      mutableArray->AppendElement(node, PR_FALSE);
      *coalesced = mutableArray;
      NS_ADDREF(*coalesced);
    }  
  }        
  return res;
}


NS_IMETHODIMP
msiInputboxCoalesce::PrepareForCoalesce(nsIEditor * editor,
                                        PRUint32    pfcFlags,
                                        nsIArray ** beforeOffset,                
                                        nsIArray ** afterOffset)                
{
  nsresult res(NS_ERROR_FAILURE);
  *beforeOffset = nsnull;
  *afterOffset = nsnull;
  nsCOMPtr<nsIMutableArray> mutableArray = do_CreateInstance(NS_ARRAY_CONTRACTID, &res);
  if (NS_SUCCEEDED(res))
  {
    res = mutableArray->AppendElement(m_mathmlNode, PR_FALSE);
    if (NS_SUCCEEDED(res))
    {
      if (m_offset == 0)
      {
        *afterOffset = mutableArray;
        NS_ADDREF(*afterOffset);
      }
      else
      {
        *beforeOffset = mutableArray;
        NS_ADDREF(*beforeOffset);
      }
    }  
  }
  return res;    
}
