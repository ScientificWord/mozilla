// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMrowBoundFence_h___
#define msiMrowBoundFence_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiMInsertionBase.h"

class msiMrowBoundFence : public msiMInsertionBase
{
public:
  msiMrowBoundFence(nsIDOMNode* mathmlNode, PRUint32 offset);
  //msiIMathMLInsertion
  NS_DECL_MSIIMATHMLINSERTION
protected:  
  PRUint32 DetermineInsertPosition(PRUint32 flags, PRBool deleteExisting);
  nsresult SetupPassToParent(nsIEditor * editor,
                             PRUint32 insertPos,
                             nsCOMPtr<msiIMathMLInsertion> & mathmlEditing,
                             PRUint32 & flags);
};

#endif // msiMrowBoundFence_h___
