// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiMrowEditingImp.h"
#include "msiUtils.h"
#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "nsIEditor.h"
#include "nsIDOMNamedNodeMap.h"

msiMrowEditingImp::msiMrowEditingImp(nsIDOMNode* mathmlNode) 
: m_mathmlNode(mathmlNode)
{
}

msiMrowEditingImp::~msiMrowEditingImp()
{
  m_mathmlNode = nsnull;
}


NS_IMPL_ISUPPORTS1(msiMrowEditingImp, msiIMrowEditing)


// msiIMrowEditing methods
NS_IMETHODIMP
msiMrowEditingImp::IsStructural(PRBool *isStructural)
{
  *isStructural = PR_FALSE;
  return NS_OK;
} 

NS_IMETHODIMP
msiMrowEditingImp::IsRedundant(nsIEditor * editor, PRBool offsetOnBoundary, PRBool *isRedundant)
{
  nsresult res(NS_ERROR_FAILURE);
  *isRedundant = PR_FALSE;
  if (m_mathmlNode)
  {
    res = NS_OK;
    PRUint32 purgeMode = msiUtils::GetMrowPurgeMode();
    if (purgeMode == msiUtils::MROW_PURGE_ALL)
      *isRedundant = PR_TRUE;
    else if (purgeMode == msiUtils::MROW_PURGE_BOUNDARY && offsetOnBoundary)
      *isRedundant = PR_TRUE;
    if (*isRedundant)
    { 
      PRBool hasAttributes(PR_FALSE);
      m_mathmlNode->HasAttributes(&hasAttributes); // TODO -- may need more refinement
      if (hasAttributes)
      {
        nsCOMPtr<nsIDOMNamedNodeMap> attributes;
        m_mathmlNode->GetAttributes(getter_AddRefs(attributes));
        if (attributes)
        {
          PRUint32 length(0);
          attributes->GetLength(&length);
          if (length == 1)
          {
            nsCOMPtr<nsIDOMNode> mozDirty;
            attributes->GetNamedItem(NS_LITERAL_STRING("_moz_dirty"), getter_AddRefs(mozDirty));
            if (mozDirty)
              *isRedundant= PR_TRUE;
          }
          else if (length == 0)
            *isRedundant= PR_TRUE;
        }
        else
          *isRedundant = PR_TRUE;
        
      }
      else
        *isRedundant = PR_TRUE;
    }  
    if (*isRedundant)
    {
      nsCOMPtr<nsIDOMNode> parent;
      nsCOMPtr<nsIDOMNode> child;
      nsCOMPtr<nsIDOMElement> childElement;
      m_mathmlNode->GetParentNode(getter_AddRefs(parent));
      nsString localName;
      nsAutoString fenced;
      if (parent)
        parent->GetLocalName(localName);
      if (localName.EqualsLiteral("mfenced"))
        *isRedundant = PR_FALSE;
      if (localName.EqualsLiteral("mrow")) {
        parent->GetFirstChild(getter_AddRefs(child));
        if (child) {
          childElement = do_QueryInterface(child);
          if (childElement) childElement->GetAttribute(NS_LITERAL_STRING("fence"), fenced);
          if (fenced.EqualsLiteral("true")) *isRedundant = PR_FALSE;
        }
      }
    } 
  }
  return NS_OK;
} 

// end msiIMrowEditing methods


nsresult
MSI_NewMrowEditingImp(nsIDOMNode* mathmlNode, msiIMrowEditing** aResult)
{
  msiMrowEditingImp * imp = new msiMrowEditingImp(mathmlNode);
  if (!imp) 
    return NS_ERROR_OUT_OF_MEMORY;

  *aResult = static_cast<msiIMrowEditing*>(imp);
  NS_ADDREF(*aResult);
  
  return NS_OK;
}
