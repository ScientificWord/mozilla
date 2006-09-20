// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMenclose_h___
#define msiMenclose_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiMInsertionBase.h"

class msiMenclose : public msiMInsertionBase
{
public:
  msiMenclose(nsIDOMNode* mathmlNode, PRUint32 offset);
};

#endif // msiMenclose_h___
