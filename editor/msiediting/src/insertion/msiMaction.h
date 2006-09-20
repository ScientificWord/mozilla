// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.
#ifndef msiMaction_h___
#define msiMaction_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiMInsertionBase.h"

class msiMaction : public msiMInsertionBase
{
public:
  msiMaction(nsIDOMNode* mathmlNode, PRUint32 offset);
};

#endif // msiMaction_h___
