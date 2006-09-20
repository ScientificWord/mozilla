// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMroot_h___
#define msiMroot_h___

#include "nsIDOMNode.h"
#include "msiFracRoot.h"

class msiMroot : public msiFracRoot
{
public:
  msiMroot(nsIDOMNode* mathmlNode, PRUint32 offset);
};

#endif // msiMroot_h___
