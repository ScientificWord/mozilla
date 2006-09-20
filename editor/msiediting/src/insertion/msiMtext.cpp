// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiMtext.h"
#include "nsIDOMNode.h"

msiMtext::msiMtext(nsIDOMNode* mathmlNode, PRUint32 offset)
:  msiMInsertionBase(mathmlNode, offset, MATHML_MTEXT)
{
}
