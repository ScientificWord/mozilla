// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMstyle_h___
#define msiMstyle_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiMInsertionBase.h"
#include "msiMContainer.h"

class msiMstyle : public msiMContainer
{
public:
  msiMstyle(nsIDOMNode* mathmlNode, PRUint32 offset);
};

#endif // msiMstyle_h___
