// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMsubsup_h___
#define msiMsubsup_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiScript.h"

class msiMsubsup : public msiScript
{
public:
  msiMsubsup(nsIDOMNode* mathmlNode, PRUint32 offset);
protected:
  virtual PRUint32 DetermineInsertPosition(PRUint32 flags);
};

#endif // msiMsubsup_h___
