// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMerrorCaret_h___
#define msiMerrorCaret_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiMCaretBase.h"

class msiMerrorCaret : public msiMCaretBase
{
public:
  msiMerrorCaret(nsIDOMNode* mathmlNode, PRUint32 offset);
};

#endif // msiMerror_h___
