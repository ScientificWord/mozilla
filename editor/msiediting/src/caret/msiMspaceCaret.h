// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMspaceCaret_h___
#define msiMspaceCaret_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiMCaretBase.h"

class msiMspaceCaret : public msiMCaretBase
{
public:
  msiMspaceCaret(nsIDOMNode* mathmlNode, PRUint32 offset);
};

#endif // msiMspaceCaret_h___
