// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMALIGNGROUPCARET_h___
#define msiMALIGNGROUPCARET_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiMCaretBase.h"

class msiMaligngroupCaret : public msiMCaretBase
{
public:
  msiMaligngroupCaret(nsIDOMNode* mathmlNode, PRUint32 offset);
};

#endif // msiMALIGNGROUPCARET_h___
