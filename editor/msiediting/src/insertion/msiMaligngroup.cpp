// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiMaligngroup.h"
#include "nsIDOMNode.h"

msiMaligngroup::msiMaligngroup(nsIDOMNode* mathmlNode, PRUint32 offset)
:  msiMInsertionBase(mathmlNode, offset, MATHML_MALIGNGROUP)
{
}
