/* Copyright 2008, MacKichan Software, Inc. */

#include "nsMathCursorUtils.h"
#include "nsCOM.h"
#include "nsMathMLFrame.h"
#include "nsFrameSelection.h"


PRBool PlaceCursorAfter( nsIFrame * pFrame, PRBool fInside, nsIFrame** aOutFrame, PRInt32* aOutOffset, PRUint32& count)
{
  nsIFrame * pChild;
  nsIFrame * pParent;
  nsCOMPtr<nsIContent> pContent;
  nsCOMPtr<nsMathMLFrame> pMathMLFrame;
  if (fInside) // we put the cursor at the end of the contents of pFrame; we do not recurse.
  {
    // find the last child
    pChild = pFrame->GetFirstChild(nsnull);
    while (pChild && pChild->GetNextSibling())
      pChild = pChild->GetNextSibling();
    pMathMLFrame = do_QueryInterface(pChild);
    if (pMathMLFrame) // last child is a math ml frame. Put the cursor after it.
    {
      pContent = pFrame->GetContent();
      *aOutOffset = 1+pContent->IndexOf(pChild->GetContent());
      if (count>0) count--;
      *aOutFrame = pFrame; 
//      (*paPos)->mMath = PR_TRUE;
    }
    else // child is not math, assumed to be text
    {
      pContent = pChild->GetContent();
      *aOutOffset = -1;           // largest possible value gets converted eventually to the end.
      if (count>0) count--;
      *aOutFrame = pChild; 
//      (*paPos)->mMath = PR_TRUE;
    }
  }
  else // don't put the cursor inside the tag
  {
    pParent = pFrame->GetParent();
    pContent = pParent->GetContent();
    *aOutOffset = 1+pContent->IndexOf(pFrame->GetContent());
    *aOutFrame = pParent; 
//    (*paPos)->mMath = PR_TRUE;
  }
  return PR_TRUE;
}

PRBool PlaceCursorBefore( nsIFrame * pFrame, PRBool fInside, nsIFrame** aOutFrame, PRInt32* aOutOffset, PRUint32& count)
{
  nsIFrame * pChild;
  nsIFrame * pParent;
  nsCOMPtr<nsIContent> pContent;
  nsCOMPtr<nsMathMLFrame> pMathMLFrame;
  if (fInside)
  {
    pChild = pFrame->GetFirstChild(nsnull);
    pMathMLFrame = do_QueryInterface(pChild);
    if (pMathMLFrame) // child is a math ml frame. Recurse down the tree
    {
      pMathMLFrame->EnterFromLeft(aOutFrame, aOutOffset, count);
    }
    else // child is not math, assumed to be text
    {
      pContent = pChild->GetContent();
      *aOutOffset = count;
      if (count>0) count--;
      *aOutFrame = pChild; 
//      (*paPos)->mMath = PR_TRUE;
    }
  }
  else // don't put the cursor inside the tag
  {
    pParent = pFrame->GetParent();
    pContent = pParent->GetContent();
    *aOutOffset = pContent->IndexOf(pFrame->GetContent());
    *aOutFrame = pParent; 
//    (*paPos)->mMath = PR_TRUE;
  }
  return PR_TRUE;
}




											   
