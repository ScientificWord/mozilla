// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiMsup.h"
#include "msiScript.h"
#include "nsIDOMNode.h"

msiMsup::msiMsup(nsIDOMNode* mathmlNode, PRUint32 offset) 
: msiScript(mathmlNode, offset, MATHML_MSUP)
{
  
}
  
