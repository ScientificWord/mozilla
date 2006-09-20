// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiMsCaret.h"
#include "nsIDOMNode.h"

msiMsCaret::msiMsCaret(nsIDOMNode* mathmlNode, PRUint32 offset)
: msiMCaretBase(mathmlNode, offset, MATHML_MS)
{
}
 