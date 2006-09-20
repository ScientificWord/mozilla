// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiMmultiscripts.h"
#include "nsIDOMNode.h"

msiMmultiscripts::msiMmultiscripts(nsIDOMNode* mathmlNode, PRUint32 offset)
:  msiMInsertionBase(mathmlNode, offset, MATHML_MMULTISCRIPTS)
{
  
}
