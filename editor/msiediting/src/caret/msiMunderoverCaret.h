// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMunderoverCaret_h___
#define msiMunderoverCaret_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiUnderAndOrOverCaret.h"

class msiMunderoverCaret : public msiUnderAndOrOverCaret
{
public:
  msiMunderoverCaret(nsIDOMNode* mathmlNode, PRUint32 offset);
};

#endif // msiMunderoverCaret_h___
