// Copyright (c) 2005, MacKichan Software, Inc.  All rights reserved.

//TODO  11/05: I may just want to make m_namespaceManager a member variable of msiEditingManager.
// Currently, msiUtits uses NameSpace function but doesn't include msiEditingManager.h
// (See bug 69 comments 7 and 8.)



#include "nsIDOMNode.h"
#include "nsServiceManagerUtils.h"
#include "nsCOMPtr.h"
#include "nsString.h"

#include "msiNameSpaceUtils.h"

PRBool msiNameSpaceUtils::m_initialized = PR_FALSE;
nsINameSpaceManager* msiNameSpaceUtils::m_namespaceManager = nsnull;

nsresult msiNameSpaceUtils::Initialize()
{
  nsresult rv(NS_OK);
  if (!m_initialized)
  {
    rv = CallGetService(NS_NAMESPACEMANAGER_CONTRACTID, &m_namespaceManager);
    m_initialized = PR_TRUE;
    if (NS_FAILED(rv))
      m_namespaceManager = nsnull;
  }   
  return rv;
}

void msiNameSpaceUtils::Shutdown()
{
 // NS_IF_RELEASE(m_namespaceManager);
}



nsresult msiNameSpaceUtils::GetNameSpaceURI(PRInt32 nsID, nsAString& nsURI)
{
  nsresult res(NS_ERROR_FAILURE);
  if (m_namespaceManager)
    res = m_namespaceManager->GetNameSpaceURI(nsID, nsURI);
  return res;  
}


nsresult msiNameSpaceUtils::GetNameSpaceID(const nsAString& nsURI, PRInt32 &nsID)
{
  nsresult res(NS_ERROR_FAILURE);
  if (m_namespaceManager)
  {
    nsID = m_namespaceManager->GetNameSpaceID(nsURI);
    res = NS_OK;
  }
  return res;  
}

PRInt32 msiNameSpaceUtils::GetNameSpaceID(nsIDOMNode * node)
{ 
  PRInt32 nsID(kNameSpaceID_Unknown);
  if (node)
  {
    nsAutoString nsURI;
    node->GetNamespaceURI(nsURI);
    GetNameSpaceID(nsURI, nsID);
  }
  return nsID;
}  
  
  
  
  
  
  
  
  
  
  
  
  
