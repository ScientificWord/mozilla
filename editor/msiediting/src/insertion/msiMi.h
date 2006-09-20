// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMi_h___
#define msiMi_h___

#include "nsIDOMNode.h"
#include "msiMleaf.h"

class msiMi : public msiMleaf
{
public:
  msiMi(nsIDOMNode* mathmlNode, PRUint32 offset);
};

#endif // msiMI_h___
