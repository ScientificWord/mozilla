// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiMstyleCaret.h"
#include "nsIDOMNode.h"

msiMstyleCaret::msiMstyleCaret(nsIDOMNode* mathmlNode, PRUint32 offset)
: msiMContainerCaret(mathmlNode, offset, MATHML_MSTYLE)
{
}
 