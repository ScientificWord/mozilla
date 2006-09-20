// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.
#include "msiMiCaret.h"
#include "nsIDOMNode.h"

msiMiCaret::msiMiCaret(nsIDOMNode* mathmlNode, PRUint32 offset) :
 msiMleafCaret(mathmlNode, offset, MATHML_MI)
{
  //TODO 02/06: MIs are atomic
  m_isDipole = PR_TRUE;
}
