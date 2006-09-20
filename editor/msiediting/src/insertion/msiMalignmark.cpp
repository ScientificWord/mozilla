// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiMalignmark.h"
#include "msiUtils.h"
#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "nsIEditor.h"
#include "nsISelection.h"

msiMalignmark::msiMalignmark(nsIDOMNode* mathmlNode, PRUint32 offset) 
:  msiMInsertionBase(mathmlNode, offset, MATHML_MALIGNMARK)
{
}
