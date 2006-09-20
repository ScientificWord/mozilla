// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiMsubCoalesce.h"
#include "nsIDOMNode.h"

msiMsubCoalesce::msiMsubCoalesce(nsIDOMNode* mathmlNode, PRUint32 offset) 
: msiScriptCoalesce(mathmlNode, offset, MATHML_MSUB)
{
  
}
  
