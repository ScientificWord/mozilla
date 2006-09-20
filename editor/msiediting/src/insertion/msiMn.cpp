// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "nsIDOMText.h"
#include "nsIDOMCharacterData.h"
#include "nsIEditor.h"
#include "nsISelection.h"

#include "msiMn.h"
#include "msiUtils.h"

msiMn::msiMn(nsIDOMNode* mathmlNode, PRUint32 offset) :
 msiMleaf(mathmlNode, offset, MATHML_MN)
{
}

