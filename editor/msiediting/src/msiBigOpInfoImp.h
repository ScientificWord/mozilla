// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiBigOpInfoImp_h__
#define msiBigOpInfoImp_h__

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiIBigOpInfo.h"


class msiBigOpInfoImp  : public msiIBigOpInfo
{
public:
  msiBigOpInfoImp(nsIDOMNode* mathmlNode, PRUint32 offset);
  virtual ~msiBigOpInfoImp();
  
  // nsISupports
  NS_DECL_ISUPPORTS
  
  // msiIBigOpInfo
  NS_DECL_MSIIBIGOPINFO

protected:
  nsCOMPtr<nsIDOMNode> m_rawMathmlNode;
  PRUint32             m_rawOffset;
  nsCOMPtr<nsIDOMNode> m_mo;
  nsCOMPtr<nsIDOMNode> m_mstyle;
  nsCOMPtr<nsIDOMNode> m_script;
  PRUint32             m_offset;
  PRUint32             m_flags;
  PRUint32             m_scriptType;
};

nsresult
MSI_NewBigOpInfoImp(nsIDOMNode* mathmlNode, PRUint32 offset, msiIBigOpInfo** aResult);


#endif // msiBigOpInfoImp_h__
