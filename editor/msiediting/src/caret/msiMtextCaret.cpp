// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiMtextCaret.h"
#include "nsIDOMNode.h"

msiMtextCaret::msiMtextCaret(nsIDOMNode* mathmlNode, PRUint32 offset)
:  msiMleafCaret(mathmlNode, offset, MATHML_MTEXT)
{
}

