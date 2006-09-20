// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMsup_h___
#define msiMsup_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiScript.h"

class msiMsup : public msiScript
{
public:
  msiMsup(nsIDOMNode* mathmlNode, PRUint32 offset);
};

#endif // msiMsup_h___
