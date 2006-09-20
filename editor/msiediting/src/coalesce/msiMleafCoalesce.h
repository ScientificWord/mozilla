// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMleafCoalesce_h___
#define msiMleafCoalesce_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiMCoalesceBase.h"


class msiMleafCoalesce : public msiMCoalesceBase
{
public:
  msiMleafCoalesce(nsIDOMNode* mathmlNode, PRUint32 offset, PRUint32 mathmlType);
  virtual ~msiMleafCoalesce();
  
  //msiIMathMLCoalesce
  NS_DECL_MSIIMATHMLCOALESCE
private:
  PRUint32 m_length;
};

#endif // msiMnCoalesce_h___
