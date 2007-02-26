// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMtableCaret_h___
#define msiMtableCaret_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiMCaretBase.h"

class msiMtableCaret : public msiMCaretBase
{
public:
  msiMtableCaret(nsIDOMNode* mathmlNode, PRUint32 offset);
  
  // msiIMathmlCaret overrides
  NS_IMETHOD  GetSelectableMathFragment(nsIEditor  *editor, 
                                        nsIDOMNode *start,      PRUint32 startOffset, 
                                        nsIDOMNode *end,        PRUint32 endOffset, 
                                        nsIDOMNode **fragStart, PRUint32 *fragStartOffset,
                                        nsIDOMNode **fragEnd,   PRUint32 *fragEndOffset);
                                       
  // end msiIMathmlCaret overrides
};

#endif // msiMtableCaret_h___
