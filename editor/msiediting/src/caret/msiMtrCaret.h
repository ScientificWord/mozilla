// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMtrCaret_h___
#define msiMtCaretr_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiMCaretBase.h"

class msiMtrCaret : public msiMCaretBase
{
public:
  msiMtrCaret(nsIDOMNode* mathmlNode, PRUint32 offset);
};

#endif // msiMtrCaret_h___
