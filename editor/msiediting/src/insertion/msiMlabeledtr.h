// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMlabeledtr_h___
#define msiMlabeledtr_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiMInsertionBase.h"

class msiMlabeledtr : public msiMInsertionBase
{
public:
  msiMlabeledtr(nsIDOMNode* mathmlNode, PRUint32 offset);
  //msiIMathMLInsertion
  NS_DECL_MSIIMATHMLINSERTION
};

#endif // msiMlabeledtr_h___
