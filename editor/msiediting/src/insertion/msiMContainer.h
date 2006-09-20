// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.
#ifndef msiMContainer_h___
#define msiMContainer_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiMInsertionBase.h"

class msiMContainer : public msiMInsertionBase
{
public:
  msiMContainer(nsIDOMNode* mathmlNode, PRUint32 offset, PRUint32 mathmlType);

  // override msiIMathMLInsertion
  NS_DECL_MSIIMATHMLINSERTION

protected:  

  nsresult Split(nsCOMPtr<nsIDOMNode> & left,
                 nsCOMPtr<nsIDOMNode> & right);
  
  PRUint32 DetermineInsertPosition(PRUint32 flags, PRBool deleteExisting);
  
};

#endif // msiMContainer_h___
