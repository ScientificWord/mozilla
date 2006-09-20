// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMglyphCaret_h___
#define msiMglyphCaret_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiMCaretBase.h"

class msiMglyphCaret : public msiMCaretBase
{
public:
  msiMglyphCaret(nsIDOMNode* mathmlNode, PRUint32 offset);
};

#endif // msiMglyphCaret_h___
