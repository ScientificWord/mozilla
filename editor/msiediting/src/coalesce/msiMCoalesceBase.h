// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMCoalesceBase_h___
#define msiMCoalesceBase_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiIMathMLCoalesce.h"
#include "msiMEditingBase.h"


class msiMCoalesceBase : public msiMEditingBase, 
                         public msiIMathMLCoalesce
{
public:
  msiMCoalesceBase(nsIDOMNode* mathmlNode, PRUint32 offset, PRUint32 mathmlType);
  virtual ~msiMCoalesceBase();
  // nsISupports
  NS_DECL_ISUPPORTS_INHERITED
  
  //msiIMathMLCoalesce
  NS_DECL_MSIIMATHMLCOALESCE
};

#endif // msiMCoalesceBase_h___
