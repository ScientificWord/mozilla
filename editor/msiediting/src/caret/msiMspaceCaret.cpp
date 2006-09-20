// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiMspaceCaret.h"
#include "nsIDOMNode.h"

msiMspaceCaret::msiMspaceCaret(nsIDOMNode* mathmlNode, PRUint32 offset)
: msiMCaretBase(mathmlNode, offset, MATHML_MACTION)
{
}
