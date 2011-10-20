/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* Copyright MacKichan Software, Inc., 2008*/

#include "nsGkAtoms.h"
#include "nsMathMLContainerCursorMover.h"
#include "nsMathCursorUtils.h"

NS_IMPL_ISUPPORTS1(nsMathMLContainerCursorMover, nsIMathMLCursorMover)

NS_IMETHODIMP nsMathMLContainerCursorMover::MoveOutToRight(
    nsIFrame *leavingFrame, 
    nsIFrame **aOutFrame, 
    PRInt32* aOutOffset, 
    PRInt32 count,
    PRBool* fBailingOut, 
    PRInt32 *_retval)
{
#ifdef debug_barry
  printf("nsMathMLContainerCursorMover MoveOutToRight, count = %d\n", count);
#endif
  // get the frame we are part of  
  nsIFrame* pFrame;
  pFrame = m_pMyFrame;
  nsIFrame* pTempFrame;
  nsCOMPtr<nsIMathMLCursorMover> pMCM;
  if (leavingFrame)
  {
    NS_ASSERTION(m_pMyFrame == leavingFrame->GetParent(), "In MoveOutToRight, leavingFrame must be a child!");
    pTempFrame = leavingFrame->GetNextSibling();
    if (pTempFrame)
    {
      pMCM = do_QueryInterface(pTempFrame);  
      if (pMCM) pMCM->EnterFromLeft(nsnull, aOutFrame, aOutOffset, count, fBailingOut, _retval);
      else  // probably pTempFrame is a text frame
      {
        *aOutFrame = pTempFrame;
        *aOutOffset = count;  
        *_retval = 0;
      }
      return NS_OK;
    }
  } 
  // if we get here, leavingFrame is null or there is no child after leavingFrame. Leave this frame.
  pTempFrame = pFrame->GetParent();
  pMCM = do_QueryInterface(pTempFrame);
  if (pMCM) pMCM->MoveOutToRight(pFrame, aOutFrame, aOutOffset, count, fBailingOut, _retval);
  else // we have gone out of math.  Put the cursor at the end of the math if count == 0
       // and after the math if count == 1 
  {                                                                        
    if (count == 0)
    {
      PlaceCursorAfter(pFrame, PR_TRUE, aOutFrame, aOutOffset, count);
    }
    else  //bail out so that the default Mozilla code takes over
    {
      PlaceCursorAfter(pFrame, PR_FALSE, aOutFrame, aOutOffset, count);
//      *fBailingOut = PR_TRUE;
    }
    *_retval = 0;
  }
  return NS_OK;  
}


NS_IMETHODIMP 
nsMathMLContainerCursorMover::MoveOutToLeft(nsIFrame *leavingFrame, nsIFrame **aOutFrame, PRInt32* aOutOffset, PRInt32 count,
    PRBool* fBailingOut, PRInt32 *_retval)
{                
#ifdef debug_barry
  printf("nsMathMLContainerCursorMover MoveOutToLeft, count = %d\n", count);
#endif
  // get the frame we are part of  
  nsIFrame* pFrame;
  pFrame = m_pMyFrame;
  nsIFrame* pTempFrame;
  nsCOMPtr<nsIMathMLCursorMover> pMCM;
  if (leavingFrame)
  {
    NS_ASSERTION(m_pMyFrame == leavingFrame->GetParent(), "In MoveOutToLeft, leavingFrame must be a child!");
    // awkward getprevioussibling
    pTempFrame = pFrame->GetFirstChild(nsnull);
    if (pTempFrame == leavingFrame) pTempFrame = nsnull; //there is no predecessor to leavingFrame
    while (pTempFrame && (pTempFrame->GetNextSibling() != leavingFrame)) pTempFrame = pTempFrame->GetNextSibling();
    if (pTempFrame)
    {
      pMCM = do_QueryInterface(pTempFrame);  
      if (pMCM) pMCM->EnterFromRight(nsnull, aOutFrame, aOutOffset, count, fBailingOut, _retval);
      else  // probably pTempFrame is a text frame
      {
        *aOutFrame = pTempFrame;
        *aOutOffset = count; 
        *_retval = 0;
      }
      return NS_OK;
    }
  } 
  // if we get here, leavingFrame is null or there is no child preceding leavingFrame. Leave this frame.
  pTempFrame = pFrame->GetParent();
  pMCM = do_QueryInterface(pTempFrame);
  if (pMCM) pMCM->MoveOutToLeft(pFrame, aOutFrame, aOutOffset, count, fBailingOut, _retval);
  else // we have gone out of math.  Put the cursor at the beginning of the math if count == 0
       // and before the math if count == 1 
  {                                                                        
    if (count == 0)
    {
      PlaceCursorBefore(pFrame, PR_TRUE, aOutFrame, aOutOffset, count);
    }
    else  //bail out so that the default Mozilla code takes over
    {
      PlaceCursorBefore(pFrame, PR_FALSE, aOutFrame, aOutOffset, count);
//      *fBailingOut = PR_TRUE;
    }
    *_retval = 0;
  }
  return NS_OK;  
}  

NS_IMETHODIMP 
nsMathMLContainerCursorMover::EnterFromLeft(nsIFrame *leavingFrame, nsIFrame **aOutFrame, PRInt32* aOutOffset, PRInt32 count,
    PRBool* fBailingOut, PRInt32 *_retval)
{
#ifdef debug_barry
  printf("nsMathMLContainerCursorMover EnterFromLeft, count = %d\n", count);
#endif
	NS_ASSERTION(leavingFrame==nsnull, "Non-null leavingFrame passed to nsMathMLContainerCursorMover::EnterFromLeft!");
	nsIFrame* pFrame;
	nsCOMPtr<nsIContent> pContent;
	pFrame = m_pMyFrame;
	nsIFrame* pTempFrame;
	nsIAtom * frametype;
	nsCOMPtr<nsIMathMLCursorMover> pMCM;
	pTempFrame = pFrame->GetFirstChild(nsnull);
	if (pTempFrame) frametype = pTempFrame->GetType();
	while (pTempFrame && (!(pMCM = do_QueryInterface(pTempFrame))) && (nsGkAtoms::textFrame != frametype))
	{
		pTempFrame = pTempFrame->GetFirstChild(nsnull);
		if (pTempFrame) frametype = pTempFrame->GetType();
	}
	if (pTempFrame)
	{ // either pMCM is not null, of frametype == textframe
		if (pMCM) pMCM->EnterFromLeft(nsnull, aOutFrame, aOutOffset, count, fBailingOut, _retval);
		else 
		{
			if (nsGkAtoms::textFrame == frametype) 
			{
			*aOutOffset = count;
			*aOutFrame = pTempFrame; 
			*_retval = count;
			}
		}
		return NS_OK;
	}
  else // this frame has no children
  {
    pMCM = do_QueryInterface(pFrame->GetParent());
    if (pMCM) {
      *aOutOffset = 0;
      pMCM->MoveOutToRight(pFrame, aOutFrame, aOutOffset, count, fBailingOut, _retval);
    }
    else // we have gone out of math 
    {
      if (!count)
      {
        pContent = m_pMyFrame->GetContent();
        *aOutOffset = pContent->GetChildCount();
        *aOutFrame = m_pMyFrame; 
        *_retval = 0;
      }
      // if count == 1, we fall through??
    }  
  }
  return NS_OK;  
}

NS_IMETHODIMP 
nsMathMLContainerCursorMover::EnterFromRight(nsIFrame *leavingFrame, nsIFrame **aOutFrame, PRInt32* aOutOffset, PRInt32 count,
    PRBool* fBailingOut, PRInt32 *_retval)
{
#ifdef debug_barry
  printf("nsMathMLContainerCursorMover EnterFromRight, count = %d\n", count);
#endif
  NS_ASSERTION(leavingFrame==nsnull, "Non-null leavingFrame passed to nsMathMLContainerCursorMover::EnterFromRight!");
  nsIFrame* pFrame;
  nsCOMPtr<nsIContent> pContent;
  pFrame = m_pMyFrame;
  nsIFrame* pTempFrame;
	nsIAtom * frametype;
  nsCOMPtr<nsIMathMLCursorMover> pMCM;
  // get last child
  pTempFrame = pFrame->GetFirstChild(nsnull);
  while (pTempFrame && (pTempFrame->GetNextSibling())) pTempFrame = pTempFrame->GetNextSibling();
	while (pTempFrame && (!(pMCM = do_QueryInterface(pTempFrame))) && (nsGkAtoms::textFrame != frametype))
	{
		pTempFrame = pTempFrame->GetFirstChild(nsnull);
	  while (pTempFrame && (pTempFrame->GetNextSibling())) pTempFrame = pTempFrame->GetNextSibling();
		if (pTempFrame) frametype = pTempFrame->GetType();
	}
  if (pTempFrame)
  {
    if (pMCM) pMCM->EnterFromRight(nsnull, aOutFrame, aOutOffset, count, fBailingOut, _retval);
    else 
		{
			if (nsGkAtoms::textFrame == frametype) 
			{
				*aOutFrame = pTempFrame; 
	      PRInt32 start, end;
	      pTempFrame->GetOffsets(start,end);
	      (*aOutOffset) = (end - start - count);
	      *_retval = 0;
			}
    }
    return NS_OK;
  }
  else // this frame has no children
  {
    pMCM = do_QueryInterface(pFrame->GetParent());
    if (pMCM) pMCM->MoveOutToLeft(pFrame, aOutFrame, aOutOffset, count, fBailingOut, _retval);
    else // we have gone out of math 
    {
      if (!count)
      {
        pContent = m_pMyFrame->GetContent();
        *aOutOffset = pContent->GetChildCount();
        *aOutFrame = m_pMyFrame; 
        *_retval = 0;
      }
      else *fBailingOut = PR_TRUE;
    }  
  }
  return NS_OK;  
}
