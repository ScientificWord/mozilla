// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiMsub.h"
#include "msiScript.h"
#include "nsIDOMNode.h"

msiMsub::msiMsub(nsIDOMNode* mathmlNode, PRUint32 offset) 
: msiScript(mathmlNode, offset, MATHML_MSUB)
{
  
}
  
