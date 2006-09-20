// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiMoCaret.h"
#include "nsIDOMNode.h"

msiMoCaret::msiMoCaret(nsIDOMNode* mathmlNode, PRUint32 offset) :
 msiMleafCaret(mathmlNode, offset, MATHML_MO)
{
  //TODO 02/06: MOs are atomic
  m_isDipole = PR_TRUE;
}
