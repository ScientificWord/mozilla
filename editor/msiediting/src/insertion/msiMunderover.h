// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMunderover_h___
#define msiMunderover_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiUnderAndOrOver.h"

class msiMunderover : public msiUnderAndOrOver
{
public:
  msiMunderover(nsIDOMNode* mathmlNode, PRUint32 offset);
protected:
  virtual PRUint32 DetermineInsertPosition(PRUint32 flags);
};

#endif // msiMunderover_h___
