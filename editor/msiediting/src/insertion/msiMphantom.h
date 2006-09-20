// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMphantom_h___
#define msiMphantom_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiMInsertionBase.h"

class msiMphantom : public msiMInsertionBase
{
public:
  msiMphantom(nsIDOMNode* mathmlNode, PRUint32 offset);
};

#endif // msiMphantom_h___
