// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiMEditingBase.h"
#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "nsIEditor.h"
#include "msiUtils.h"

msiMEditingBase::msiMEditingBase(nsIDOMNode* mathmlNode, 
                                 PRUint32 offset,
                                 PRUint32 mathmlType) 
: m_mathmlNode(mathmlNode), m_offset(offset), m_mathmlType(mathmlType), m_numKids(INVALID)
{
  if(m_mathmlNode)
  {
    nsCOMPtr<nsIDOMNode> currChild;
    nsresult res = m_mathmlNode->GetFirstChild(getter_AddRefs(currChild));
    PRUint32 index(0);
    while (NS_SUCCEEDED(res) && currChild)
    {
      nsCOMPtr<nsIDOMNode> nextChild;
      res = currChild->GetNextSibling(getter_AddRefs(nextChild));
      if (msiUtils::IsWhitespace(currChild))
      {
        nsCOMPtr<nsIDOMNode> dontcare;
        res = m_mathmlNode->RemoveChild(currChild, getter_AddRefs(dontcare));
        if (m_offset <= LAST_VALID && m_offset > index)
          m_offset -= 1;
      }
      else
        index += 1;
      currChild = nextChild;    
    }
    m_numKids = index;
    if (m_offset == RIGHT_MOST)
      m_offset = index; 
  }
}
  
msiMEditingBase::~msiMEditingBase()
{
  m_mathmlNode = nsnull;
}

NS_IMPL_ISUPPORTS1(msiMEditingBase, msiIMathMLEditingBC)

NS_IMETHODIMP 
msiMEditingBase::GetMathmlType(PRUint32 * mathmlType)
{
    *mathmlType = m_mathmlType;
    return NS_OK;
}

NS_IMETHODIMP 
msiMEditingBase::GetMathmlNode(nsIDOMNode * * mathmlNode)
{
  *mathmlNode = m_mathmlNode;
  NS_ADDREF(*mathmlNode); 
  return NS_OK; 
}

NS_IMETHODIMP 
msiMEditingBase::GetNumKids(PRUint32 *numKids)
{
   *numKids = m_numKids;
   return NS_OK;
}

NS_IMETHODIMP 
msiMEditingBase::GetOffset(PRUint32 *offset)
{
   *offset = m_offset;
   return NS_OK;
}

NS_IMETHODIMP 
msiMEditingBase::SetOffset(PRUint32 offset)
{
    m_offset = offset;
    return NS_OK;
}
