// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMtdCaret_h___
#define msiMtdCaret_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiMContainerCaret.h"

class msiMtdCaret : public msiMContainerCaret
{
public:
  msiMtdCaret(nsIDOMNode* mathmlNode, PRUint32 offset);

  //msiIMathMLCaret
  NS_DECL_MSIIMATHMLCARET
};

#endif // msiMtdCaret_h___
