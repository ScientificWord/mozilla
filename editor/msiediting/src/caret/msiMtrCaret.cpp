// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiMtrCaret.h"
#include "nsIDOMNode.h"

msiMtrCaret::msiMtrCaret(nsIDOMNode* mathmlNode, PRUint32 offset)
:  msiMCaretBase(mathmlNode, offset, MATHML_MTR)
{
}
