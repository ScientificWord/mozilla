// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMmultiscripts_h___
#define msiMmultiscripts_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiMInsertionBase.h"

class msiMmultiscripts : public msiMInsertionBase
{
public:
  msiMmultiscripts(nsIDOMNode* mathmlNode, PRUint32 offset);
};

#endif // msiMmultiscripts_h___
