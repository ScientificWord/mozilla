// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiMo.h"
#include "msiUtils.h"
#include "nsIDOMNode.h"

msiMo::msiMo(nsIDOMNode* mathmlNode, PRUint32 offset) :
 msiMleaf(mathmlNode, offset, MATHML_MO)
{
  //02/06 --  mo are considered to be "atomic"
  if (m_length > 1)
  {
    PRUint32 mid = (m_length+1)/2;
    if (m_offset < mid)
      m_offset = 0;
    else
      m_offset = m_length;  
  }
}
