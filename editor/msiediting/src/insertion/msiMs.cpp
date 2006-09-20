// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiMs.h"
#include "nsIDOMNode.h"

msiMs::msiMs(nsIDOMNode* mathmlNode, PRUint32 offset)
: msiMInsertionBase(mathmlNode, offset, MATHML_MS)
{
}
 