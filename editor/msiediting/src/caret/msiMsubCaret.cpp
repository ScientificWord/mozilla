// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiMsubCaret.h"
#include "msiScriptCaret.h"
#include "nsIDOMNode.h"

msiMsubCaret::msiMsubCaret(nsIDOMNode* mathmlNode, PRUint32 offset) 
: msiScriptCaret(mathmlNode, offset, MATHML_MSUB)
{
  
}
  
