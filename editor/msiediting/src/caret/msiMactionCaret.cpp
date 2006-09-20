// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.
#include "msiMactionCaret.h"
#include "nsIDOMNode.h"

msiMactionCaret::msiMactionCaret(nsIDOMNode* mathmlNode, PRUint32 offset) 
:  msiMCaretBase(mathmlNode, offset, MATHML_MACTION)
{
  
}

