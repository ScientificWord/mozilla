// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMerror_h___
#define msiMerror_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiMInsertionBase.h"

class msiMerror : public msiMInsertionBase
{
public:
  msiMerror(nsIDOMNode* mathmlNode, PRUint32 offset);
};

#endif // msiMerror_h___
