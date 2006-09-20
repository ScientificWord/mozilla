// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiMover.h"
#include "nsIDOMNode.h"

msiMover::msiMover(nsIDOMNode* mathmlNode, PRUint32 offset)
:  msiUnderAndOrOver(mathmlNode, offset, MATHML_MOVER)
{
}
  
