// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.
#ifndef msiMrowCaret_h___
#define msiMrowCaret_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiMContainerCaret.h"
#include "msiIMrowEditing.h"

class msiMrowCaret : public msiMContainerCaret, public msiIMrowEditing
{
public:
  msiMrowCaret(nsIDOMNode * mathmlNode, PRUint32 offset);
  
  // nsISupports
  NS_DECL_ISUPPORTS_INHERITED

  // msiIMrowEditing 
  NS_FORWARD_SAFE_MSIIMROWEDITING(m_mrowEditingImp)
                         
  //override msiIMathMLCaret
  NS_DECL_MSIIMATHMLCARET

protected:
nsCOMPtr<msiIMrowEditing> m_mrowEditingImp;                         

};

#endif // msiMrow_h___
