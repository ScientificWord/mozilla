// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiMlabeledtrCaret.h"
#include "nsIDOMNode.h"

msiMlabeledtrCaret::msiMlabeledtrCaret(nsIDOMNode* mathmlNode, PRUint32 offset)
:  msiMCaretBase(mathmlNode, offset, MATHML_MLABELEDTR)
{
}
