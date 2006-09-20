// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMrowEditingImp_h___
#define msiMrowEditingImp_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiIMrowEditing.h"


class msiMrowEditingImp : public msiIMrowEditing
{
public:
  msiMrowEditingImp(nsIDOMNode* mathmlNode);
  virtual ~msiMrowEditingImp();
  // nsISupports
  NS_DECL_ISUPPORTS
  
  //msiIMrowEditing
  NS_DECL_MSIIMROWEDITING
protected:

nsCOMPtr<nsIDOMNode> m_mathmlNode;

};

nsresult
MSI_NewMrowEditingImp(nsIDOMNode* mathmlNode, msiIMrowEditing** aResult);


#endif // msiMrowEditingImp_h___
