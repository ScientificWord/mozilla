// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMsupCoalesce_h___
#define msiMsupCoalesce_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiScriptCoalesce.h"

class msiMsupCoalesce : public msiScriptCoalesce
{
public:
  msiMsupCoalesce(nsIDOMNode* mathmlNode, PRUint32 offset);
};

#endif // msiMsupCoalesce_h___
