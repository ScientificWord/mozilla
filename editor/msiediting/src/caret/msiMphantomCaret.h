// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMphantomCaret_h___
#define msiMphantomCaret_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiMContainerCaret.h"

class msiMphantomCaret : public msiMContainerCaret
{
public:
  msiMphantomCaret(nsIDOMNode* mathmlNode, PRUint32 offset);
};

#endif // msiMphantomCaret_h___
