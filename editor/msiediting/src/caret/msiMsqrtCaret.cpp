// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiMsqrtCaret.h"
#include "nsIDOMNode.h"

msiMsqrtCaret::msiMsqrtCaret(nsIDOMNode* mathmlNode, PRUint32 offset) 
:  msiMencloseCaret(mathmlNode, offset, MATHML_MSQRT)
{
}

