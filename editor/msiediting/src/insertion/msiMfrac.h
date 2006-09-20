// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMfrac_h___
#define msiMfrac_h___

#include "nsIDOMNode.h"
#include "msiFracRoot.h"

class msiMfrac : public msiFracRoot
{
public:
  msiMfrac(nsIDOMNode* mathmlNode, PRUint32 offset);
};

#endif // msiMfrac_h___
