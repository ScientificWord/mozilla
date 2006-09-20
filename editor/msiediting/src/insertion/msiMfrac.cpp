// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiMfrac.h"
#include "nsIDOMNode.h"

msiMfrac::msiMfrac(nsIDOMNode* mathmlNode, PRUint32 offset)
:  msiFracRoot(mathmlNode, offset, MATHML_MFRAC)
{
}

