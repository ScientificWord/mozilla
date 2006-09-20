// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiMspace.h"
#include "nsIDOMNode.h"

msiMspace::msiMspace(nsIDOMNode* mathmlNode, PRUint32 offset)
: msiMInsertionBase(mathmlNode, offset, MATHML_MACTION)
{
}
