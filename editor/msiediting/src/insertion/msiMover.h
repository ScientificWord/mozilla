// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMover_h___
#define msiMover_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiUnderAndOrOver.h"

class msiMover : public msiUnderAndOrOver
{
public:
  msiMover(nsIDOMNode* mathmlNode, PRUint32 offset);
};

#endif // msiMover_h___
