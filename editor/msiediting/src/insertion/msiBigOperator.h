// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiBigOperator_h___
#define msiBigOperator_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiMInsertionBase.h"
#include "msiIBigOpInfo.h"

class msiBigOperator : public msiMInsertionBase
{
public:
  msiBigOperator(nsIDOMNode* mathmlNode, PRUint32 offset, nsCOMPtr<msiIBigOpInfo> & bigOpInfo);
  
  // override msiIMathMLInsertion
  NS_DECL_MSIIMATHMLINSERTION
protected:

  enum {IP_Invalid, IP_BaseLeft, IP_BaseRight, IP_Script1Left, IP_Script1Right, IP_Script2Left, IP_Script2Right};                      
  virtual PRUint32 DetermineInsertPosition(PRUint32 flags);
  nsresult SetupPassToParent(nsIEditor * editor, nsCOMPtr<msiIMathMLInsertion> & mathmlEditing,
                             PRUint32 & flags);
  nsCOMPtr<msiIBigOpInfo> m_bigOpInfo;                             
                       
};

#endif // msiBigOperator_h___
