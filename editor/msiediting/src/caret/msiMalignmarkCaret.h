// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMalignmarkCaret_h___
#define msiMalignmarkCaret_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiMCaretBase.h"

class msiMalignmarkCaret : public msiMCaretBase
{
public:
  msiMalignmarkCaret(nsIDOMNode* mathmlNode, PRUint32 offset);
};
#endif // msiMalignmark_h___
