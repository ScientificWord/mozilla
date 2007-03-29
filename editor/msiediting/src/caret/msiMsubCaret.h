// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMsubCaret_h___
#define msiMsubCaret_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiScriptCaret.h"

class msiMsubCaret : public msiScriptCaret
{
public:
  msiMsubCaret(nsIDOMNode* mathmlNode, PRUint32 offset);

  // override msiIMathMLCaret
  NS_DECL_MSIIMATHMLCARET
};

#endif // msiMsubCaret_h___
