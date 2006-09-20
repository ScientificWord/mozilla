// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMs_h___
#define msiMs_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiMInsertionBase.h"


class msiMs : public msiMInsertionBase
{
public:
  msiMs(nsIDOMNode* mathmlNode, PRUint32 offset);
};

#endif // msiMs_h___
