// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiMleafCoalesce.h"
#include "msiUtils.h"
#include "msiCoalesceUtils.h"
#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "nsIEditor.h"
#include "nsIMutableArray.h"
#include "nsIDOMText.h"
#include "nsIDOMCharacterData.h"
#include "nsComponentManagerUtils.h"

msiMleafCoalesce::msiMleafCoalesce(nsIDOMNode* mathmlNode, PRUint32 offset, PRUint32 mathmlType) 
: msiMCoalesceBase(mathmlNode, offset, mathmlType)
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
  
msiMleafCoalesce::~msiMleafCoalesce()
{
}

NS_IMETHODIMP
msiMleafCoalesce::Coalesce(nsIEditor * editor,
                           nsIDOMNode * node,
                           nsIArray ** coalesced)                
{
  return msiMCoalesceBase::Coalesce(editor, node, coalesced);
}

NS_IMETHODIMP
msiMleafCoalesce::PrepareForCoalesce(nsIEditor * editor,
                                     PRUint32    pfcFlags,
                                     nsIArray ** beforeOffset,                
                                     nsIArray ** afterOffset)                
{
  nsresult res(NS_ERROR_FAILURE);
  if (m_length == 0)
  {
    nsCOMPtr<nsIMutableArray> mutableArray = do_CreateInstance(NS_ARRAY_CONTRACTID, &res);
    if (NS_SUCCEEDED(res))
    {
      *beforeOffset = mutableArray;  //return valid but empty array
      NS_ADDREF(*beforeOffset);
    }  
  }
  else
    res = msiMCoalesceBase::PrepareForCoalesce(editor, pfcFlags, beforeOffset, afterOffset);
  return res;  
}
