// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMALIGNGROUP_h___
#define msiMALIGNGROUP_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiMInsertionBase.h"

class msiMaligngroup : public msiMInsertionBase
{
public:
  msiMaligngroup(nsIDOMNode* mathmlNode, PRUint32 offset);
};

#endif // msiMALIGNGROUP_h___
