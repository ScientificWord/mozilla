// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiMtdCaret.h"
#include "nsIDOMNode.h"

msiMtdCaret::msiMtdCaret(nsIDOMNode* mathmlNode, PRUint32 offset)
: msiMCaretBase(mathmlNode, offset, MATHML_MTD)
{
}
