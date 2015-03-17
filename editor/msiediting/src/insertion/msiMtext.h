// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMtext_h___
#define msiMtext_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiMInsertionBase.h"

class msiMtext : public msiMInsertionBase
{
public:
  msiMtext(nsIDOMNode* mathmlNode, PRUint32 offset);
  // NS_IMETHOD Inquiry(nsIEditor * editor, PRUint32 inquiryID, PRBool * result);
};

#endif // msiMtext_h___
