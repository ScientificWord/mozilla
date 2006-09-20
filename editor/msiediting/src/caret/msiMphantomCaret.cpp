// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiMphantomCaret.h"
#include "nsIDOMNode.h"

msiMphantomCaret::msiMphantomCaret(nsIDOMNode* mathmlNode, PRUint32 offset)
:  msiMContainerCaret(mathmlNode, offset, MATHML_MPHANTOM)
{
}
