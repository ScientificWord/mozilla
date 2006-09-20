// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMtable_h___
#define msiMtable_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiMInsertionBase.h"

class msiMtable : public msiMInsertionBase
{
public:
  msiMtable(nsIDOMNode* mathmlNode, PRUint32 offset);
  //msiIMathMLInsertion
  NS_DECL_MSIIMATHMLINSERTION
};

#endif // msiMtable_h___
