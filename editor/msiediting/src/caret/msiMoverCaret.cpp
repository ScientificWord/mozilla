// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiMoverCaret.h"
#include "nsIDOMNode.h"

msiMoverCaret::msiMoverCaret(nsIDOMNode* mathmlNode, PRUint32 offset)
:  msiUnderAndOrOverCaret(mathmlNode, offset, MATHML_MOVER)
{
}
  
