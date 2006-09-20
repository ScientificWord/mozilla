// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiMfencedCaret.h"
#include "msiMContainerCaret.h"
#include "nsIDOMNode.h"

msiMfencedCaret::msiMfencedCaret(nsIDOMNode* mathmlNode, PRUint32 offset)
:  msiMContainerCaret(mathmlNode, offset, MATHML_MFENCED)
{
  
}


NS_IMETHODIMP
msiMfencedCaret::AdjustSelectionPoint(nsIEditor *editor, PRBool leftSelPoint, 
                                      nsIDOMNode** selectionNode, PRUint32 * selectionOffset,
                                      msiIMathMLCaret** parentCaret)
{
  return msiMCaretBase::AdjustSelectionPoint(editor, leftSelPoint, selectionNode, selectionOffset, parentCaret);
}

