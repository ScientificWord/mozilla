// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMpadded_h___
#define msiMpadded_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiMInsertionBase.h"

class msiMpadded : public msiMInsertionBase
{
public:
  msiMpadded(nsIDOMNode* mathmlNode, PRUint32 offset);
};

#endif // msiMpadded_h___
