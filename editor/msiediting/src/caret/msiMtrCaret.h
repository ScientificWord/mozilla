// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMtrCaret_h___
#define msiMtCaretr_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiMContainerCaret.h"

class msiMtrCaret : public msiMContainerCaret
{
public:
  msiMtrCaret(nsIDOMNode* mathmlNode, PRUint32 offset);

  //msiIMathMLCaret
  NS_DECL_MSIIMATHMLCARET
};

#endif // msiMtrCaret_h___
