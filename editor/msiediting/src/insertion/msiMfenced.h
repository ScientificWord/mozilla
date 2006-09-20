// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMfenced_h___
#define msiMfenced_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiMContainer.h"

class msiMfenced : public msiMContainer
{
public:
  msiMfenced(nsIDOMNode* mathmlNode, PRUint32 offset);
};

#endif // msiMfenced_h___
