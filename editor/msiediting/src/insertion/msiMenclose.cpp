// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiMenclose.h"
#include "nsIDOMNode.h"

msiMenclose::msiMenclose(nsIDOMNode* mathmlNode, PRUint32 offset)
:  msiMContainer(mathmlNode, offset, MATHML_MENCLOSE)
{
}

NS_IMETHODIMP
msiMenclose::Inquiry(nsIEditor* editor,
                     PRUint32 inquiryID, 
                     PRBool * result)
{
  nsresult res(NS_OK);
  if (inquiryID == INSERT_DISPLAY || inquiryID == INSERT_INLINE_MATH)
    *result = PR_FALSE;
  else if (inquiryID == IS_MROW_REDUNDANT)
    *result = PR_FALSE;
  else
    *result = PR_TRUE;
  return res;
}                                                 
