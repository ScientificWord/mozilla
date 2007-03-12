// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiMnCaret.h"
#include "nsIDOMNode.h"

msiMnCaret::msiMnCaret(nsIDOMNode* mathmlNode, PRUint32 offset) :
 msiMleafCaret(mathmlNode, offset, MATHML_MN)
{
}
