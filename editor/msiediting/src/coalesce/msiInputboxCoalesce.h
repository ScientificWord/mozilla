// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiInputboxCoalesce_h___
#define msiInputboxCoalesce_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiMCoalesceBase.h"


class msiInputboxCoalesce : public msiMCoalesceBase
{
public:
  msiInputboxCoalesce(nsIDOMNode* mathmlNode, PRUint32 offset);
  virtual ~msiInputboxCoalesce();

  //msiIMathMLCoalesce
  NS_DECL_MSIIMATHMLCOALESCE
};

#endif // msiInputboxCoalesce_h___
