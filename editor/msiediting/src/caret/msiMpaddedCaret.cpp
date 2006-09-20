// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiMpaddedCaret.h"
#include "nsIDOMNode.h"

msiMpaddedCaret::msiMpaddedCaret(nsIDOMNode* mathmlNode, PRUint32 offset)
: msiMCaretBase(mathmlNode, offset, MATHML_MPADDED)
{
}
