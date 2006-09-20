// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiMerror.h"
#include "nsIDOMNode.h"

msiMerror::msiMerror(nsIDOMNode* mathmlNode, PRUint32 offset)
:  msiMInsertionBase(mathmlNode, offset, MATHML_MERROR)
{
}
