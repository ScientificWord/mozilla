// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMo_h___
#define msiMo_h___

#include "nsIDOMNode.h"
#include "msiMleaf.h"

class msiMo : public msiMleaf
{
public:
  msiMo(nsIDOMNode* mathmlNode, PRUint32 offset);
};
#endif // msiMo_h___
