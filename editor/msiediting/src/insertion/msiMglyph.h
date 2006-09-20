// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMglyph_h___
#define msiMglyph_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiMInsertionBase.h"

class msiMglyph : public msiMInsertionBase
{
public:
  msiMglyph(nsIDOMNode* mathmlNode, PRUint32 offset);
};

#endif // msiMglyph_h___
