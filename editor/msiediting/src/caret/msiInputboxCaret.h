// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiInputboxCaret_h___
#define msiInputboxCaret_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "nsIEditor.h"
#include "msiMCaretBase.h"


class msiInputboxCaret : public msiMCaretBase
{
public:
  msiInputboxCaret(nsIDOMNode* mathmlNode, PRUint32 offset);

  //override msiIMathMLCaret
  NS_DECL_MSIIMATHMLCARET
};

#endif // msiInputboxCaret_h___
