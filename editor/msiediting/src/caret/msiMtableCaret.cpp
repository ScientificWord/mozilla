// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiMtableCaret.h"
#include "nsIDOMNode.h"

msiMtableCaret::msiMtableCaret(nsIDOMNode* mathmlNode, PRUint32 offset)
: msiMCaretBase(mathmlNode, offset, MATHML_MTABLE)
{
}

NS_IMETHODIMP
msiMtableCaret::GetNodeAndOffsetFromPoint(nsIEditor *editor, nsIPresShell *presShell, 
                                         PRUint32 flags, PRInt32 x, PRInt32 y, 
                                         nsIDOMNode **node, PRUint32 *offset)
{
  return NS_OK;
}  

NS_IMETHODIMP 
msiMtableCaret::GetSelectableMathFragment(nsIEditor  *editor, 
                                         nsIDOMNode *start,      PRUint32 startOffset, 
                                         nsIDOMNode *end,        PRUint32 endOffset, 
                                         nsIDOMNode **fragStart, PRUint32 *fragStartOffset,
                                         nsIDOMNode **fragEnd,   PRUint32 *fragEndOffset)
{
  return NS_ERROR_FAILURE;
}

