// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiMsqrtCaret.h"
#include "msiUtils.h"
#include "nsIDOMNode.h"

msiMsqrtCaret::msiMsqrtCaret(nsIDOMNode* mathmlNode, PRUint32 offset) 
:  msiMencloseCaret(mathmlNode, offset, MATHML_MSQRT)
{
}

NS_IMETHODIMP
msiMsqrtCaret::TabLeft(nsIEditor *editor, 
                       nsIDOMNode **node, 
                       PRUint32 *offset)
{
  return TabRight(editor,node,offset);
}


//TODO undo
NS_IMETHODIMP
msiMsqrtCaret::TabRight(nsIEditor *editor, 
                        nsIDOMNode **node, 
                        PRUint32 *offset)
{
  if (!editor || !node || !offset)
    return NS_ERROR_FAILURE;

  nsresult res;
  PRUint32 flags(msiIMathMLInsertion::FLAGS_NONE);
  nsCOMPtr<nsIDOMNode> child;
  msiUtils::GetChildNode(m_mathmlNode, 0, child);
  nsCOMPtr<nsIDOMNode> clone;
  res = msiUtils::CloneNode(child, clone);
  if (NS_FAILED(res))
    return res;
  nsCOMPtr<nsIDOMElement> mrootElement;
  res = msiUtils::CreateMroot(editor, clone, nsnull, PR_TRUE, flags, mrootElement);
  if (NS_FAILED(res))
    return res;
  nsCOMPtr<nsIDOMNode> parent;
  m_mathmlNode->GetParentNode(getter_AddRefs(parent));
  nsCOMPtr<nsIDOMNode> mrootNode(do_QueryInterface(mrootElement));
  if (mrootNode && parent)
  {
    res = editor->ReplaceNode(mrootNode, m_mathmlNode, parent);
  } 
  else
  {
    res = NS_ERROR_FAILURE;  //maybe NS_ERROR_DOM_NOT_FOUND_ERR?
  }
  if (NS_FAILED(res))
    return res;
  //give the caret to the mroot
  nsCOMPtr<msiIMathMLCaret> mathmlCaret;
  msiUtils::GetMathMLCaretInterface(editor, mrootNode, 0, mathmlCaret);
  if (mathmlCaret)
    res = mathmlCaret->Accept(editor, FROM_PARENT|FROM_LEFT, node, offset); 
  else 
    res = NS_ERROR_FAILURE;
  return res;
}
