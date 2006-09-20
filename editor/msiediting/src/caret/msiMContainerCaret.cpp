// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.
#include "msiMContainerCaret.h"
#include "nsIDOMNode.h"
#include "msiUtils.h"

msiMContainerCaret::msiMContainerCaret(nsIDOMNode* mathmlNode, PRUint32 offset, PRUint32 mathmlType)
:  msiMCaretBase(mathmlNode, offset, mathmlType)
{
  
}

NS_IMETHODIMP
msiMContainerCaret::Inquiry(nsIEditor* editor, PRUint32 inquiryID, PRBool *result)
{
  if (inquiryID == AT_RIGHT_EQUIV_TO_0 || inquiryID == AT_LEFT_EQUIV_TO_RIGHT_MOST) 
    *result = PR_FALSE;
  else if (inquiryID == CAN_SELECT_CHILD_LEAF) 
    *result = PR_TRUE;
  else
    *result = PR_FALSE;
  return NS_OK;
}


NS_IMETHODIMP
msiMContainerCaret::AdjustSelectionPoint(nsIEditor *editor, PRBool leftSelPoint, 
                                         nsIDOMNode** selectionNode, PRUint32 * selectionOffset,
                                         msiIMathMLCaret** parentCaret)
{
  nsCOMPtr<msiIMathMLCaret> pCaret;
  PRBool incOffset(!leftSelPoint);
  nsresult res = msiUtils::SetupPassOffCaretToParent(editor, m_mathmlNode, incOffset, pCaret);
  if (NS_SUCCEEDED(res) && pCaret)  
  {
    *parentCaret = pCaret;
    NS_ADDREF(*parentCaret);
  }
  else 
    res = NS_ERROR_FAILURE;
  return res;
}
