// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiMunder.h"
#include "nsIDOMNode.h"

msiMunder::msiMunder(nsIDOMNode* mathmlNode, PRUint32 offset)
:  msiUnderAndOrOver(mathmlNode, offset, MATHML_MUNDER)
{
}
