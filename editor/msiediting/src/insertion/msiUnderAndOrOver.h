// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiUnderAndOrOver_h___
#define msiUnderAndOrOver_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiMInsertionBase.h"

class msiUnderAndOrOver : public msiMInsertionBase
{
public:
  msiUnderAndOrOver(nsIDOMNode* mathmlNode, PRUint32 offset, PRUint32 mathmlType);
  
  // override msiIMathMLInsertion
  NS_DECL_MSIIMATHMLINSERTION
protected:
  enum {IP_BaseLeft, IP_BaseRight, IP_Script1Left, IP_Script1Right, IP_Script2Left, IP_Script2Right};                      
  virtual PRUint32 DetermineInsertPosition(PRUint32 flags);
};

#endif // msiUnderAndOrOver_h___
