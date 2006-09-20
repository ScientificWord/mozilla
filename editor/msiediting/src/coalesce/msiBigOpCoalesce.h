// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiBigOpCoalesce_h___
#define msiBigOpCoalesce_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiMCoalesceBase.h"
#include "msiIBigOpInfo.h"


class msiBigOpCoalesce : public msiMCoalesceBase
{
public:
  msiBigOpCoalesce(nsIDOMNode* mathmlNode, PRUint32 offset, nsCOMPtr<msiIBigOpInfo> & bigOpInfo);
  virtual ~msiBigOpCoalesce();

  //msiIMathMLCoalesce
  NS_DECL_MSIIMATHMLCOALESCE
protected:
  PRUint32 m_maxOffset;
  nsCOMPtr<msiIBigOpInfo> m_bigOpInfo;                             
  
  void DetermineScriptShiftAttributes(nsIDOMNode * node, nsAString & subShift, nsAString & supShift);
  
};

#endif // msiBigOpCoalesce_h___
