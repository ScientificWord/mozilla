// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMathCaret_h___
#define msiMathCaret_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiMContainerCaret.h"

class msiMathCaret : public msiMContainerCaret
{
public:
  msiMathCaret(nsIDOMNode* mathmlNode, PRUint32 offset);
                         
  //override msiIMathMLCaret
  NS_DECL_MSIIMATHMLCARET
                         
protected:
  PRBool m_isDisplay; 
};


#endif // msiMathCaret_h___
