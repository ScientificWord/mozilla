// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiMerrorCaret.h"
#include "nsIDOMNode.h"

msiMerrorCaret::msiMerrorCaret(nsIDOMNode* mathmlNode, PRUint32 offset)
:  msiMCaretBase(mathmlNode, offset, MATHML_MERROR)
{
}
