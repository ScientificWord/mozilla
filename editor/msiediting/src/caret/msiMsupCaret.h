// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMsupCaret_h___
#define msiMsupCaret_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiScriptCaret.h"

class msiMsupCaret : public msiScriptCaret
{
public:
  msiMsupCaret(nsIDOMNode* mathmlNode, PRUint32 offset);
  
  // override msiIMathMLCaret
  NS_DECL_MSIIMATHMLCARET
};

#endif // msiMsupCaret_h___
