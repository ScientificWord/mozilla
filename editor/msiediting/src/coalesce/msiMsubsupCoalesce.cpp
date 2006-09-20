// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiMsubsupCoalesce.h"
#include "nsIDOMNode.h"

msiMsubsupCoalesce::msiMsubsupCoalesce(nsIDOMNode* mathmlNode, PRUint32 offset) 
: msiScriptCoalesce(mathmlNode, offset, MATHML_MSUBSUP)
{
  
}
  
