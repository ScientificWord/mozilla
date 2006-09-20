// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiMglyph.h"
#include "nsIDOMNode.h"

msiMglyph::msiMglyph(nsIDOMNode* mathmlNode, PRUint32 offset)
:  msiMInsertionBase(mathmlNode, offset, MATHML_MGLYPH)
{
  
}
