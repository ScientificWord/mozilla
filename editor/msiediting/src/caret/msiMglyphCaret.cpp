// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiMglyphCaret.h"
#include "nsIDOMNode.h"

msiMglyphCaret::msiMglyphCaret(nsIDOMNode* mathmlNode, PRUint32 offset)
:  msiMCaretBase(mathmlNode, offset, MATHML_MGLYPH)
{
  
}
