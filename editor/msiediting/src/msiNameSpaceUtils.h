// Copyright (c) 2005, MacKichan Software, Inc.  All rights reserved.
#ifndef msiNameSpaceUtils_h___
#define msiNameSpaceUtils_h___

#include "nsString.h"
#include "nsINameSpaceManager.h"

class msiNameSpaceUtils
{
public:
  static nsresult Initialize();
  static void Shutdown();

  static nsresult GetNameSpaceURI(PRInt32 nsID, nsAString& nsURI);
  static nsresult GetNameSpaceID(const nsAString& nsURI, PRInt32 &nsID);
  static PRInt32  GetNameSpaceID(nsIDOMNode * node);

private:
  static nsINameSpaceManager * m_namespaceManager;
  static PRBool m_initialized;
                                
};                  


#endif