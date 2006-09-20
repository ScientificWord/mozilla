// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiMmultiscriptsCaret.h"
#include "nsIDOMNode.h"

msiMmultiscriptsCaret::msiMmultiscriptsCaret(nsIDOMNode* mathmlNode, PRUint32 offset)
:  msiMCaretBase(mathmlNode, offset, MATHML_MMULTISCRIPTS)
{
  
}
