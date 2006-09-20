// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiMaligngroupCaret.h"
#include "nsIDOMNode.h"

msiMaligngroupCaret::msiMaligngroupCaret(nsIDOMNode* mathmlNode, PRUint32 offset)
:  msiMCaretBase(mathmlNode, offset, MATHML_MALIGNGROUP)
{
}
