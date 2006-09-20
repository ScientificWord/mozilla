// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMtdCaret_h___
#define msiMtdCaret_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiMCaretBase.h"

class msiMtdCaret : public msiMCaretBase
{
public:
  msiMtdCaret(nsIDOMNode* mathmlNode, PRUint32 offset);
};

#endif // msiMtdCaret_h___
