// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiMCoalesceBase.h"
#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "nsIEditor.h"
#include "nsIMutableArray.h"
#include "nsComponentManagerUtils.h"

msiMCoalesceBase::msiMCoalesceBase(nsIDOMNode* mathmlNode, 
                                   PRUint32 offset,
                                   PRUint32 mathmlType) 
: msiMEditingBase(mathmlNode, offset, mathmlType)
{
}
  
msiMCoalesceBase::~msiMCoalesceBase()
{
}

NS_IMPL_ISUPPORTS_INHERITED1(msiMCoalesceBase, msiMEditingBase, msiIMathMLCoalesce)

NS_IMETHODIMP
msiMCoalesceBase::Coalesce(nsIEditor * editor,
                           nsIDOMNode * node,
                           nsIArray ** coalesced)                
{
  *coalesced = nsnull;
  return NS_OK;
}

NS_IMETHODIMP
msiMCoalesceBase::CoalesceTxn(nsIEditor * editor,
                              nsIDOMNode * node,
                              nsITransaction ** txn)                
{
  *txn = nsnull;
  return NS_OK;
}

NS_IMETHODIMP
msiMCoalesceBase::PrepareForCoalesce(nsIEditor * editor,
                                     PRUint32    pfcFlags,
                                     nsIArray ** beforeOffset,                
                                     nsIArray ** afterOffset)
{
  *beforeOffset = nsnull;
  *afterOffset = nsnull;
  nsresult res(NS_ERROR_FAILURE);
  nsCOMPtr<nsIMutableArray> mutableArray = do_CreateInstance(NS_ARRAY_CONTRACTID, &res);
  if (NS_SUCCEEDED(res) && mutableArray && m_mathmlNode)
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



