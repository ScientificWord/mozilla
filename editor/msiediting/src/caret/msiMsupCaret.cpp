// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiMsupCaret.h"
#include "msiScriptCaret.h"
#include "nsIDOMNode.h"

msiMsupCaret::msiMsupCaret(nsIDOMNode* mathmlNode, PRUint32 offset) 
: msiScriptCaret(mathmlNode, offset, MATHML_MSUP)
{
  
}
  
