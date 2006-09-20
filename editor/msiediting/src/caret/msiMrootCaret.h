// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMrootCaret_h___
#define msiMrootCaret_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiMCaretBase.h"

class msiMrootCaret : public msiMCaretBase
{
public:
  msiMrootCaret(nsIDOMNode* mathmlNode, PRUint32 offset);
  
  //msiIMathMLCaret
  NS_DECL_MSIIMATHMLCARET
};

#endif // msiMrootCaret_h___
