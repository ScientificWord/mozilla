// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMsqrtCaret_h___
#define msiMsqrtCaret_h___


#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiMencloseCaret.h"

class msiMsqrtCaret : public msiMencloseCaret
{
public:
  msiMsqrtCaret(nsIDOMNode* mathmlNode, PRUint32 offset);

  // msiIMathmlCaret overrides
  NS_IMETHOD TabLeft(nsIEditor *editor,
                     nsIDOMNode **node,
                     PRUint32 *offset);
                 
  NS_IMETHOD TabRight(nsIEditor *editor,
                      nsIDOMNode **node,
                      PRUint32 *offset);
};

#endif // msiMsqrtCaret_h___
