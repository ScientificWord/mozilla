// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMenclose_h___
#define msiMenclose_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiMContainer.h"

class msiMenclose : public msiMContainer
{
public:
  msiMenclose(nsIDOMNode* mathmlNode, PRUint32 offset);
  
  /* ------------ msiIMathMLInsertion overrides -------------- */
  NS_IMETHOD Inquiry(nsIEditor* editor,
                     PRUint32 inquiryID, 
                     PRBool * result);
  
};

#endif // msiMenclose_h___
