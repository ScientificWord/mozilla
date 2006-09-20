// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMtd_h___
#define msiMtd_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiMContainer.h"

class msiMtd : public msiMContainer
{
public:
  msiMtd(nsIDOMNode* mathmlNode, PRUint32 offset);
  
  /* ------------ msiIMathMLInsertion overrides -------------- */
  NS_IMETHOD Inquiry(nsIEditor* editor,
                     PRUint32 inquiryID, 
                     PRBool * result);
};

#endif // msiMtd_h___
