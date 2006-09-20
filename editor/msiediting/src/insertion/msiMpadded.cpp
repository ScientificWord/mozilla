// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiMpadded.h"
#include "nsIDOMNode.h"

msiMpadded::msiMpadded(nsIDOMNode* mathmlNode, PRUint32 offset)
: msiMInsertionBase(mathmlNode, offset, MATHML_MPADDED)
{
}
