// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiMunderCaret.h"
#include "nsIDOMNode.h"

msiMunderCaret::msiMunderCaret(nsIDOMNode* mathmlNode, PRUint32 offset)
:  msiUnderAndOrOverCaret(mathmlNode, offset, MATHML_MUNDER)
{
}
