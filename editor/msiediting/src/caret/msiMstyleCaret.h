// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMstyleCaret_h___
#define msiMstyleCaret_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiMContainerCaret.h"

class msiMstyleCaret : public msiMContainerCaret
{
public:
  msiMstyleCaret(nsIDOMNode* mathmlNode, PRUint32 offset);
};

#endif // msiMstyleCaret_h___
