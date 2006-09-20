// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiMfenced.h"
#include "msiMContainer.h"
#include "nsIDOMNode.h"

msiMfenced::msiMfenced(nsIDOMNode* mathmlNode, PRUint32 offset)
:  msiMContainer(mathmlNode, offset, MATHML_MFENCED)
{
  
}
