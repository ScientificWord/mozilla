// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiMtext.h"
#include "nsIDOMNode.h"

msiMtext::msiMtext(nsIDOMNode* mathmlNode, PRUint32 offset)
:  msiMInsertionBase(mathmlNode, offset, MATHML_MTEXT)
{
}

NS_IMETHODIMP
msiMtext::Inquiry(nsIEditor * editor,
                     PRUint32 inquiryID,
                     PRBool * result)
{
  nsresult res(NS_OK);
  if (inquiryID == INSERT_DISPLAY ||
      inquiryID == IS_MROW_REDUNDANT)
    *result = PR_FALSE;
  else
    *result = PR_TRUE;
  return res;
}
