// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.
#include "msiMi.h"
#include "nsIDOMNode.h"

msiMi::msiMi(nsIDOMNode* mathmlNode, PRUint32 offset) :
 msiMleaf(mathmlNode, offset, MATHML_MI)
{
  //02/06 --  mi are considered to be "atomic"
  if (m_length > 1)
  {
    PRUint32 mid = (m_length+1)/2;
    if (m_offset < mid)
      m_offset = 0;
    else
      m_offset = m_length;  
  }
}
