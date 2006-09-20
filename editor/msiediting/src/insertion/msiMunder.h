// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMunder_h___
#define msiMunder_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiUnderAndOrOver.h"

class msiMunder : public msiUnderAndOrOver
{
public:
  msiMunder(nsIDOMNode* mathmlNode, PRUint32 offset);
};

#endif // msiMunder_h___
