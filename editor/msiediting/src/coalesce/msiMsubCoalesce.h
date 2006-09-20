// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMsubCoalesce_h___
#define msiMsubCoalesce_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiScriptCoalesce.h"

class msiMsubCoalesce : public msiScriptCoalesce
{
public:
  msiMsubCoalesce(nsIDOMNode* mathmlNode, PRUint32 offset);
};

#endif // msiMsubCoalesce_h___
