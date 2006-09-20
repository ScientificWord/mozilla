// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMtr_h___
#define msiMtr_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiMInsertionBase.h"

class msiMtr : public msiMInsertionBase
{
public:
  msiMtr(nsIDOMNode* mathmlNode, PRUint32 offset);
  //msiIMathMLInsertion
  NS_DECL_MSIIMATHMLINSERTION
};

#endif // msiMtr_h___
