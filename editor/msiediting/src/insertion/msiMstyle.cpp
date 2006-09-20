// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiMstyle.h"
#include "nsIDOMNode.h"

msiMstyle::msiMstyle(nsIDOMNode* mathmlNode, PRUint32 offset)
: msiMContainer(mathmlNode, offset, MATHML_MSTYLE)
{
}
 