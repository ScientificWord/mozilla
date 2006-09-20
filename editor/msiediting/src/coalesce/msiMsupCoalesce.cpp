// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiMsupCoalesce.h"
#include "nsIDOMNode.h"

msiMsupCoalesce::msiMsupCoalesce(nsIDOMNode* mathmlNode, PRUint32 offset) 
: msiScriptCoalesce(mathmlNode, offset, MATHML_MSUP)
{
  
}
  
