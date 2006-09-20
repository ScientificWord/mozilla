// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMsqrt_h___
#define msiMsqrt_h___


#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiMContainer.h"

class msiMsqrt : public msiMContainer
{
public:
  msiMsqrt(nsIDOMNode* mathmlNode, PRUint32 offset);
  
  /* ------------ msiIMathMLInsertion overrides -------------- */
  NS_IMETHOD Inquiry(nsIEditor* editor,
                     PRUint32 inquiryID, 
                     PRBool * result);
  
};

#endif // msiMsqrt_h___
