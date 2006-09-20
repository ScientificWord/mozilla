// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiMroot.h"
#include "nsIDOMNode.h"

msiMroot::msiMroot(nsIDOMNode* mathmlNode, PRUint32 offset)
:  msiFracRoot(mathmlNode, offset, MATHML_MROOT)
{
}


 