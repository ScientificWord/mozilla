// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMnCoalesce_h___
#define msiMnCoalesce_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiMCoalesceBase.h"


class msiMnCoalesce : public msiMCoalesceBase
{
public:
  msiMnCoalesce(nsIDOMNode* mathmlNode, PRUint32 offset);
  virtual ~msiMnCoalesce();
  
  //msiIMathMLCoalesce
  NS_DECL_MSIIMATHMLCOALESCE
private:
  PRUint32 m_length;
};

#endif // msiMnCoalesce_h___
