// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiMunderoverCaret.h"
#include "nsIDOMNode.h"

msiMunderoverCaret::msiMunderoverCaret(nsIDOMNode* mathmlNode, PRUint32 offset)
:  msiUnderAndOrOverCaret(mathmlNode, offset, MATHML_MUNDEROVER)
{
}
