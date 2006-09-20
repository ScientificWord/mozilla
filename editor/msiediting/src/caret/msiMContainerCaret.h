// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.
#ifndef msiMContainerCaret_h___
#define msiMContainerCaret_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiMCaretBase.h"

class msiMContainerCaret : public msiMCaretBase
{
public:
  msiMContainerCaret(nsIDOMNode* mathmlNode, PRUint32 offset, PRUint32 mathmlType);
  
  // msiIMathmlCaret overrides
  NS_IMETHOD AdjustSelectionPoint(nsIEditor *editor, PRBool fromLeft, 
                                  nsIDOMNode** selectionNode, PRUint32 * selectionOffset,
                                  msiIMathMLCaret** parentCaret);
  NS_IMETHOD Inquiry(nsIEditor* editor, PRUint32 inquiryID, PRBool *result);
  // end msiIMathmlCaret overrides

};

#endif // msiMContainerCaret_h___
