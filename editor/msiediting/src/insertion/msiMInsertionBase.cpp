// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiMInsertionBase.h"
#include "msiMrow.h"
#include "msiUtils.h"
#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "nsIDOMNodeList.h"
#include "nsITextContent.h"
#include "nsIEditor.h"
#include "nsISelection.h"

msiMInsertionBase::msiMInsertionBase(nsIDOMNode* mathmlNode, 
                                 PRUint32 offset,
                                 PRUint32 mathmlType) 
: msiMEditingBase(mathmlNode, offset, mathmlType)
{
}
  
msiMInsertionBase::~msiMInsertionBase()
{
}

NS_IMPL_ISUPPORTS_INHERITED1(msiMInsertionBase, msiMEditingBase, msiIMathMLInsertion)

NS_IMETHODIMP
msiMInsertionBase::InsertNode(nsIEditor * editor,
                            nsISelection * selection, 
                            nsIDOMNode * node, 
                            PRUint32 flags)
{
  return NS_ERROR_NOT_IMPLEMENTED;
}


NS_IMETHODIMP
msiMInsertionBase::InsertNodes(nsIEditor * editor,
                               nsISelection * selection, 
                               nsIArray * nodeList,
                               PRBool  deleteExisting, 
                               PRUint32 flags)
{
  return NS_ERROR_NOT_IMPLEMENTED;
}


NS_IMETHODIMP
msiMInsertionBase::InsertMath(nsIEditor * editor,
                            nsISelection * selection, 
                            PRBool isDisplay,
                            nsIDOMNode * inLeft,
                            nsIDOMNode * inRight,
                            PRUint32 flags)
{
  return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP
msiMInsertionBase::Inquiry(nsIEditor* editor,
                           PRUint32 inquiryID, 
                           PRBool * result)
{
  nsresult res(NS_OK);
  if (inquiryID == INSERT_DISPLAY || 
      inquiryID == INSERT_INLINE_MATH || 
      inquiryID == IS_MROW_REDUNDANT)
    *result = PR_FALSE;
  else
    *result = PR_TRUE;
  return res;
}


