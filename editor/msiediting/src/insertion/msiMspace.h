// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMspace_h___
#define msiMspace_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiMInsertionBase.h"

class msiMspace : public msiMInsertionBase
{
public:
  msiMspace(nsIDOMNode* mathmlNode, PRUint32 offset);
};

#endif // msiMspace_h___
