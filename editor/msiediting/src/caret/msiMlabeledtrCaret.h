// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMlabeledtrCaret_h___
#define msiMlabeledtrCaret_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiMCaretBase.h"

class msiMlabeledtrCaret : public msiMCaretBase
{
public:
  msiMlabeledtrCaret(nsIDOMNode* mathmlNode, PRUint32 offset);
};

#endif // msiMlabeledtCaretr_h___
