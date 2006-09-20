// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiInputbox_h___
#define msiInputbox_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "nsIEditor.h"
#include "msiMInsertionBase.h"


class msiInputbox : public msiMInsertionBase
{
public:
  msiInputbox(nsIDOMNode* mathmlNode, PRUint32 offset);

  //override msiIMathMLInsertion
  NS_DECL_MSIIMATHMLINSERTION
protected:  
// Setup the pass off of an insertion to the parent node. The mathmlEditing parameter
// will contain the msiIMathMLInsertion interface for the parent. Typically the parent
// will replace the inputbox with the node to be inserted.
virtual nsresult SetupPassToParent(nsIEditor * editor,
                                   nsCOMPtr<msiIMathMLInsertion> & mathmlEditing,
                                   PRUint32 & flags);
  
};

#endif // msiInputbox_h___
