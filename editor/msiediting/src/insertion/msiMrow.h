// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.
#ifndef msiMrow_h___
#define msiMrow_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiMContainer.h"
#include "msiIMrowEditing.h"

class msiMrow : public msiMContainer, public msiIMrowEditing
{
public:
  msiMrow(nsIDOMNode * mathmlNode, PRUint32 offset);
  
  // nsISupports
  NS_DECL_ISUPPORTS_INHERITED
  
  // msiIMrowEditing 
  NS_FORWARD_SAFE_MSIIMROWEDITING(m_mrowEditingImp)
  
  /* ------------ msiIMathMLInsertion overrides -------------- */
  NS_IMETHOD InsertMath(nsIEditor *editor,
                         nsISelection   *selection, 
                         PRBool isDisplay,
                         nsIDOMNode * left,
                         nsIDOMNode * right,
                         PRUint32     flags);
                         
  NS_IMETHOD InsertNodes(nsIEditor *editor, 
                         nsISelection *selection, 
                         nsIArray *nodeArray, 
                         PRBool deleteExisting, 
                         PRUint32 flags);
protected:
nsCOMPtr<msiIMrowEditing> m_mrowEditingImp;                         
                         
};

#endif // msiMrow_h___
