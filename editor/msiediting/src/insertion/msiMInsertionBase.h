// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMInsertionBase_h___
#define msiMInsertionBase_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiIMathMLInsertion.h"
#include "msiMEditingBase.h"


class msiMInsertionBase : public msiMEditingBase, 
                          public msiIMathMLInsertion
{
public:
  msiMInsertionBase(nsIDOMNode* mathmlNode, PRUint32 offset, PRUint32 mathmlType);
  virtual ~msiMInsertionBase();
  // nsISupports
  NS_DECL_ISUPPORTS_INHERITED
  
  //msiIMathMLInsertion
  NS_DECL_MSIIMATHMLINSERTION
protected:
  enum {insertModeNone, passOffToParent, replaceInputbox, doMrow, absorbIntoMrow };  
};

#endif // msiMInsertionBase_h___
