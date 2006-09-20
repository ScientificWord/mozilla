// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMoCaret_h___
#define msiMoCaret_h___

#include "nsIDOMNode.h"
#include "msiMleafCaret.h"

class msiMoCaret : public msiMleafCaret
{
public:
  msiMoCaret(nsIDOMNode* mathmlNode, PRUint32 offset);
};
#endif // msiMoCaret_h___
