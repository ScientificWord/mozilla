// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMrowCoalesce_h___
#define msiMrowCoalesce_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiMCoalesceBase.h"
#include "msiIMrowEditing.h"


class msiMrowCoalesce : public msiMCoalesceBase, public msiIMrowEditing
{
public:
  msiMrowCoalesce(nsIDOMNode* mathmlNode, PRUint32 offset);
  virtual ~msiMrowCoalesce();

  // nsISupports
  NS_DECL_ISUPPORTS_INHERITED

  //msiIMathMLCoalesce
  NS_DECL_MSIIMATHMLCOALESCE
  
  // msiIMrowEditing 
  NS_FORWARD_SAFE_MSIIMROWEDITING(m_mrowEditingImp)
  
protected:
nsCOMPtr<msiIMrowEditing> m_mrowEditingImp;                         

};

#endif // msiMrowCoalesce_h___
