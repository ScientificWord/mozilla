// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiMsubsupCaret.h"
#include "nsIDOMNode.h"

msiMsubsupCaret::msiMsubsupCaret(nsIDOMNode* mathmlNode, PRUint32 offset)
:  msiScriptCaret(mathmlNode, offset, MATHML_MSUBSUP)
{
}
  