/* Copyright 2008, MacKichan Software, Inc. */

#include "nsMathCursorUtils.h"
#include "nsCOM.h"
#include "nsMathMLFrame.h"
#include "nsFrameSelection.h"


PRBool PlaceCursorAfter( nsIFrame * pFrame, PRBool fInside, nsPeekOffsetStruct** paPos, PRUint32& count)
{
  nsIFrame * pChild;
  nsIFrame * pParent;
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
      (*paPos)->mResultContent = pFrame->GetContent();
      (*paPos)->mContentOffset = 1+(*paPos)->mResultContent->IndexOf(pChild->GetContent());
      if (count>0) count--;
      (*paPos)->mResultFrame = pFrame; 
      (*paPos)->mMath = PR_TRUE;
    }
    else // child is not math, assumed to be text
    {
      (*paPos)->mResultContent = pChild->GetContent();
      (*paPos)->mContentOffset = -1;           // largest possible value gets converted eventually to the end.
      if (count>0) count--;
      (*paPos)->mResultFrame = pChild; 
      (*paPos)->mMath = PR_TRUE;
    }
  }
  else // don't put the cursor inside the tag
  {
    pParent = pFrame->GetParent();
    (*paPos)->mResultContent = pParent->GetContent();
    (*paPos)->mContentOffset = 1+(*paPos)->mResultContent->IndexOf(pFrame->GetContent());
    (*paPos)->mResultFrame = pParent; 
    (*paPos)->mMath = PR_TRUE;
  }
  return PR_TRUE;
}

PRBool PlaceCursorBefore( nsIFrame * pFrame, PRBool fInside, nsPeekOffsetStruct** paPos, PRUint32& count)
{
  nsIFrame * pChild;
  nsIFrame * pParent;
  nsCOMPtr<nsMathMLFrame> pMathMLFrame;
  if (fInside)
  {
    pChild = pFrame->GetFirstChild(nsnull);
    pMathMLFrame = do_QueryInterface(pChild);
    if (pMathMLFrame) // child is a math ml frame. Recurse down the tree
    {
      pMathMLFrame->ContinueFromBefore(paPos, count);
    }
    else // child is not math, assumed to be text
    {
      (*paPos)->mResultContent = pChild->GetContent();
      (*paPos)->mContentOffset = count;
      if (count>0) count--;
      (*paPos)->mResultFrame = pChild; 
      (*paPos)->mMath = PR_TRUE;
    }
  }
  else // don't put the cursor inside the tag
  {
    pParent = pFrame->GetParent();
    (*paPos)->mResultContent = pParent->GetContent();
    (*paPos)->mContentOffset = (*paPos)->mResultContent->IndexOf(pFrame->GetContent());
    (*paPos)->mResultFrame = pParent; 
    (*paPos)->mMath = PR_TRUE;
  }
  return PR_TRUE;
}




											   
