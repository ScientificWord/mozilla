/* Copyright 2008, MacKichan Software, Inc. */

#include "nsCom.h"
#include "nsFrameSelection.h"
#include "nsMathCursorUtils.h"
#include "nsMathMLCursorMover.h"


PRBool PlaceCursorAfter( nsIFrame * pFrame, PRBool fInside, nsIFrame** aOutFrame, PRInt32* aOutOffset, PRInt32& count)
{
  nsIFrame * pChild;
  nsIFrame * pParent;
  nsCOMPtr<nsIContent> pContent;
  nsCOMPtr<nsIMathMLCursorMover> pMCM;
  if (fInside) // we put the cursor at the end of the contents of pFrame; we do not recurse.
  {
    // find the last child
    pChild = pFrame->GetFirstChild(nsnull);
    while (pChild && pChild->GetNextSibling())
      pChild = pChild->GetNextSibling();
    pMCM = do_QueryInterface(pChild);
    if (pMCM) // last child is a math ml frame. Put the cursor after it.
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

PRBool PlaceCursorBefore( nsIFrame * pFrame, PRBool fInside, nsIFrame** aOutFrame, PRInt32* aOutOffset, PRInt32& count)
{
  nsIFrame * pChild;
  nsIFrame * pParent;
  nsCOMPtr<nsIContent> pContent;
  nsCOMPtr<nsIMathMLCursorMover> pMCM;
  if (fInside)
  {
    pChild = pFrame->GetFirstChild(nsnull);
    pMCM = do_QueryInterface(pChild);
    if (pMCM) // child is a math ml frame. Recurse down the tree
    {
      pMCM->EnterFromLeft(pFrame, aOutFrame, aOutOffset, count, (PRInt32*)&count);
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




											   
