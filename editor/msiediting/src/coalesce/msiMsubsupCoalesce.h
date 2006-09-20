// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMsubsupCoalesce_h___
#define msiMsubsupCoalesce_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiScriptCoalesce.h"

class msiMsubsupCoalesce : public msiScriptCoalesce
{
public:
  msiMsubsupCoalesce(nsIDOMNode* mathmlNode, PRUint32 offset);
};

#endif // msiMsubsupCoalesce_h___
