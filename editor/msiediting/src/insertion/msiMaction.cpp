// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.
#include "msiMaction.h"
#include "nsIDOMNode.h"

msiMaction::msiMaction(nsIDOMNode* mathmlNode, PRUint32 offset) 
:  msiMInsertionBase(mathmlNode, offset, MATHML_MACTION)
{
  
}
