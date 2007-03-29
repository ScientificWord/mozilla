// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMoverCaret_h___
#define msiMoverCaret_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiUnderAndOrOverCaret.h"

class msiMoverCaret : public msiUnderAndOrOverCaret
{
public:
  msiMoverCaret(nsIDOMNode* mathmlNode, PRUint32 offset);

  // override msiIMathMLCaret
  NS_DECL_MSIIMATHMLCARET
};

#endif // msiMoverCaret_h___
