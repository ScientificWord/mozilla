// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMsCaret_h___
#define msiMsCaret_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiMCaretBase.h"


class msiMsCaret : public msiMCaretBase
{
public:
  msiMsCaret(nsIDOMNode* mathmlNode, PRUint32 offset);
};

#endif // msiMsCaret_h___
