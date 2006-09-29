// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiLayoutUtils_h___
#define msiLayoutUtils_h___

#include "msiILayoutUtils.h"

class msiLayoutUtils : public msiILayoutUtils
{
public:
  NS_DECL_ISUPPORTS
  
  // msiILayoutUtils methods
  NS_IMETHOD GetOffsetIntoTextFromEvent(nsIFrame *textFrame, nsIDOMEvent *domEvent, PRUint32 *offset);
  // end msiILayoutUtils methods

  msiLayoutUtils();
  virtual ~msiLayoutUtils();
};

  nsresult MSI_NewLayoutUtils(msiILayoutUtils** aInstancePtrResult);

#endif  //msiLayoutUtils_h__