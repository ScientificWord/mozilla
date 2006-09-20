// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMEditingBase_h___
#define msiMEditingBase_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiIMathMLEditingBC.h"


class msiMEditingBase : public msiIMathMLEditingBC
{
public:
  msiMEditingBase(nsIDOMNode* mathmlNode, PRUint32 offset, PRUint32 mathmlType);
  virtual ~msiMEditingBase();

  // nsISupports
  NS_DECL_ISUPPORTS
  
  //msiIMathMLEditing Base Class
  NS_DECL_MSIIMATHMLEDITINGBC
  
protected:
  PRUint32             m_mathmlType;
  nsCOMPtr<nsIDOMNode> m_mathmlNode;
  PRUint32             m_offset;
  PRUint32             m_numKids;
};

#endif // msiMEditingBase_h___
