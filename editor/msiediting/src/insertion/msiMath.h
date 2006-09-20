// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMath_h___
#define msiMath_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiMContainer.h"

class msiMath : public msiMContainer
{
public:
  msiMath(nsIDOMNode* mathmlNode, PRUint32 offset);
  
  /* ------------ msiIMathMLInsertion overrides -------------- */
  NS_IMETHOD InsertMath(nsIEditor * editor,
                         nsISelection * selection, 
                         PRBool isDisplay,
                         nsIDOMNode * inLeft,
                         nsIDOMNode * inRight,
                         PRUint32 flags);
                         
  NS_IMETHOD  Inquiry(nsIEditor* editor,
                      PRUint32 inquiryID, 
                      PRBool * result);
protected:
  nsresult SetDisplayMode(nsIEditor * editor, PRBool isDisplay);
                          
protected:
  PRBool m_isDisplay; 
};


#endif // msiMath_h___
