// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMtextCaret_h___
#define msiMtextCaret_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiMleafCaret.h"

class msiMtextCaret : public msiMleafCaret
{
public:
  msiMtextCaret(nsIDOMNode* mathmlNode, PRUint32 offset);
};

#endif // msiMtextCaret_h___
