// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMnCaret_h___
#define msiMnCaret_h___

#include "nsIDOMNode.h"
#include "msiMleafCaret.h"

class msiMnCaret : public msiMleafCaret
{
public:
  msiMnCaret(nsIDOMNode* mathmlNode, PRUint32 offset);
};

#endif // msiMnCaret_h___
