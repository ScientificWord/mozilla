// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.
#ifndef msiMactionCaret_h___
#define msiMactionCaret_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiMCaretBase.h"

class msiMactionCaret : public msiMCaretBase
{
public:
  msiMactionCaret(nsIDOMNode* mathmlNode, PRUint32 offset);
};

#endif // msiMactionCaret_h___
