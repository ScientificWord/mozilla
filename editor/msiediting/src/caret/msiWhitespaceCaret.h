// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiWhitespaceCaret_h___
#define msiWhitespaceCaret_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "nsIEditor.h"
#include "msiMCaretBase.h"


class msiWhitespaceCaret : public msiMCaretBase
{
public:
  msiWhitespaceCaret(nsIDOMNode* mathmlNode, PRUint32 offset);

  //override msiIMathMLCaret
  NS_DECL_MSIIMATHMLCARET
};

#endif // msiWhitespaceCaret_h___
