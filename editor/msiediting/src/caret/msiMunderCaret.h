// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMunderCaret_h___
#define msiMunderCaret_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiUnderAndOrOverCaret.h"

class msiMunderCaret : public msiUnderAndOrOverCaret
{
public:
  msiMunderCaret(nsIDOMNode* mathmlNode, PRUint32 offset);

  // override msiIMathMLCaret
  NS_DECL_MSIIMATHMLCARET
};

#endif // msiMunderCaret_h___
