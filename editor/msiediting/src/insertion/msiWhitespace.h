// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiWhitespace_h___
#define msiWhitespace_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "nsIEditor.h"
#include "msiMInsertionBase.h"


class msiWhitespace : public msiMInsertionBase
{
public:
  msiWhitespace(nsIDOMNode* mathmlNode, PRUint32 offset);

  //override msiIMathMLInsertion
  NS_DECL_MSIIMATHMLINSERTION
protected:  
virtual nsresult SetupPassToParent(nsIEditor * editor,
                                   nsCOMPtr<msiIMathMLInsertion> & mathmlEditing,
                                   PRUint32 & flags);
  
};

#endif // msiWhitespace_h___
