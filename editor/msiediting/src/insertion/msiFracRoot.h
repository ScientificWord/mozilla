// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiFracRoot_h___
#define msiFracRoot_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiMInsertionBase.h"

class msiFracRoot : public msiMInsertionBase
{
public:
  msiFracRoot(nsIDOMNode* mathmlNode, PRUint32 offset, PRUint32 mathmlType);
  // overwrite msiIMathMLInsertion
  NS_DECL_MSIIMATHMLINSERTION
  
protected:  
  enum {IP_FirstChildLeft, IP_FirstChildRight, IP_SecondChildLeft, IP_SecondChildRight};                      
  virtual PRUint32 DetermineInsertPosition(PRUint32 flags);
  
};

#endif // msiFracRoot_h___
