// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMiCaret_h___
#define msiMiCaret_h___

#include "nsIDOMNode.h"
#include "msiMleafCaret.h"

class msiMiCaret : public msiMleafCaret
{
public:
  msiMiCaret(nsIDOMNode* mathmlNode, PRUint32 offset);
};

#endif // msiMiCaret_h___
