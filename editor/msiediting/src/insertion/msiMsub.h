// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMsub_h___
#define msiMsub_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiScript.h"

class msiMsub : public msiScript
{
public:
  msiMsub(nsIDOMNode* mathmlNode, PRUint32 offset);
};

#endif // msiMsub_h___
