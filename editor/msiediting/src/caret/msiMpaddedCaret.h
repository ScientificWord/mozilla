// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMpaddedCaret_h___
#define msiMpaddedCaret_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiMCaretBase.h"

class msiMpaddedCaret : public msiMCaretBase
{
public:
  msiMpaddedCaret(nsIDOMNode* mathmlNode, PRUint32 offset);
};

#endif // msiMpaddedCaret_h___
