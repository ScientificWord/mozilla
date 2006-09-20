// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiMalignmarkCaret.h"
#include "msiUtils.h"
#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "nsIEditor.h"
#include "nsISelection.h"

msiMalignmarkCaret::msiMalignmarkCaret(nsIDOMNode* mathmlNode, PRUint32 offset) 
:  msiMCaretBase(mathmlNode, offset, MATHML_MALIGNMARK)
{
}
