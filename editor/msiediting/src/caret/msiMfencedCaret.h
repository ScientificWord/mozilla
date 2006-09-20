// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMfencedCaret_h___
#define msiMfencedCaret_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiMContainerCaret.h"

class msiMfencedCaret : public msiMContainerCaret
{
public:
  msiMfencedCaret(nsIDOMNode* mathmlNode, PRUint32 offset);

  // msiIMathmlCaret overrides
  NS_IMETHOD AdjustSelectionPoint(nsIEditor *editor, PRBool leftSelPoint, 
                                  nsIDOMNode** selectionNode, PRUint32 * selectionOffset,
                                  msiIMathMLCaret** parentCaret);
  // end msiIMathmlCaret overrides
};

#endif // msiMfencedCaret_h___
