// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMleafCaret_h___
#define msiMleafCaret_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiMCaretBase.h"

class msiMleafCaret : public msiMCaretBase
{
public:
  msiMleafCaret(nsIDOMNode* mathmlNode, PRUint32 offset, PRUint32 mathmlType);
  
  //override msiIMathMLCarert
  NS_DECL_MSIIMATHMLCARET
protected:
  nsresult doSetCaretPosition(nsIEditor *editor, nsISelection *selection, PRUint32 offset);  

protected:
  PRUint32 m_length; 
  PRBool   m_isDipole;
  nsCOMPtr<nsIDOMNode> m_textNode;                                  
};

#endif // msiMleafCaret_h___
