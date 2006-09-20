// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMn_h___
#define msiMn_h___

#include "nsIDOMNode.h"
#include "msiMleaf.h"

class msiMn : public msiMleaf
{
public:
  msiMn(nsIDOMNode* mathmlNode, PRUint32 offset);
};

#endif // msiMn_h___
