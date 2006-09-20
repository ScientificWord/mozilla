// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiMphantom.h"
#include "nsIDOMNode.h"

msiMphantom::msiMphantom(nsIDOMNode* mathmlNode, PRUint32 offset)
:  msiMInsertionBase(mathmlNode, offset, MATHML_MPHANTOM)
{
}
