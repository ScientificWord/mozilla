// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMalignmark_h___
#define msiMalignmark_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiMInsertionBase.h"

class msiMalignmark : public msiMInsertionBase
{
public:
  msiMalignmark(nsIDOMNode* mathmlNode, PRUint32 offset);
};
#endif // msiMalignmark_h___
