// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMsubsupCaret_h___
#define msiMsubsupCaret_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiScriptCaret.h"

class msiMsubsupCaret : public msiScriptCaret
{
public:
  msiMsubsupCaret(nsIDOMNode* mathmlNode, PRUint32 offset);
};

#endif // msiMsubsupCaret_h___
