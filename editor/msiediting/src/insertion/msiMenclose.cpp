// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiMenclose.h"
#include "nsIDOMNode.h"

msiMenclose::msiMenclose(nsIDOMNode* mathmlNode, PRUint32 offset)
:  msiMInsertionBase(mathmlNode, offset, MATHML_MENCLOSE)
{
}
