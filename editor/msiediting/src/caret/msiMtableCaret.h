// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMtableCaret_h___
#define msiMtableCaret_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiMCaretBase.h"

class msiMtableCaret : public msiMCaretBase
{
public:
  msiMtableCaret(nsIDOMNode* mathmlNode, PRUint32 offset);
  //msiIMathMLCaret
  NS_DECL_MSIIMATHMLCARET
};

#endif // msiMtableCaret_h___
