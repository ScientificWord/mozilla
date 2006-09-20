// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMmultiscriptsCaret_h___
#define msiMmultiscriptsCaret_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiMCaretBase.h"

class msiMmultiscriptsCaret : public msiMCaretBase
{
public:
  msiMmultiscriptsCaret(nsIDOMNode* mathmlNode, PRUint32 offset);
};

#endif // msiMmultiscriptsCaret_h___
