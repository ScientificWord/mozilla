/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Mozilla MathML Project.
 *
 * The Initial Developer of the Original Code is
 * The University Of Queensland.
 * Portions created by the Initial Developer are Copyright (C) 1999
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Roger B. Sidje <rbs@maths.uq.edu.au>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either of the GNU General Public License Version 2 or later (the "GPL"),
 * or the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

//
// a helper frame class to wrap non-MathML frames so that foreign elements 
// (e.g., html:img) can mix better with other surrounding MathML markups
//

#include "nsCOMPtr.h"
#include "nsHTMLParts.h"
#include "nsFrame.h"
#include "nsAreaFrame.h"
#include "nsLineLayout.h"
#include "nsPresContext.h"
#include "nsStyleContext.h"
#include "nsStyleConsts.h"
#include "nsIRenderingContext.h"
#include "nsIFontMetrics.h"
#include "nsMathCursorUtils.h"

#include "nsMathMLForeignFrameWrapper.h"

NS_IMPL_ADDREF_INHERITED(nsMathMLForeignFrameWrapper, nsMathMLFrame)
NS_IMPL_RELEASE_INHERITED(nsMathMLForeignFrameWrapper, nsMathMLFrame)
NS_IMPL_QUERY_INTERFACE_INHERITED1(nsMathMLForeignFrameWrapper, nsBlockFrame, nsIMathMLFrame)

nsIFrame*
NS_NewMathMLForeignFrameWrapper(nsIPresShell* aPresShell, nsStyleContext* aContext)
{
  return new (aPresShell) nsMathMLForeignFrameWrapper(aContext);
}

NS_IMETHODIMP
nsMathMLForeignFrameWrapper::Reflow(nsPresContext*          aPresContext,
                                    nsHTMLReflowMetrics&     aDesiredSize,
                                    const nsHTMLReflowState& aReflowState,
                                    nsReflowStatus&          aStatus)
{
  // Let the base class do the reflow
  nsresult rv = nsBlockFrame::Reflow(aPresContext, aDesiredSize, aReflowState, aStatus);

  mReference.x = 0;
  mReference.y = aDesiredSize.ascent;

  // just make-up a bounding metrics
  mBoundingMetrics.Clear();
  mBoundingMetrics.ascent = aDesiredSize.ascent;
  mBoundingMetrics.descent = aDesiredSize.height - aDesiredSize.ascent;
  mBoundingMetrics.width = aDesiredSize.width;
  mBoundingMetrics.leftBearing = 0;
  mBoundingMetrics.rightBearing = aDesiredSize.width;
  aDesiredSize.mBoundingMetrics = mBoundingMetrics;

  NS_FRAME_SET_TRUNCATION(aStatus, aReflowState, aDesiredSize);
  return rv;
}

 
nsresult
nsMathMLForeignFrameWrapper::MoveOutToRight(nsIFrame * leavingFrame, nsIFrame** aOutFrame, PRInt32* aOutOffset, PRUint32& count)
{
  printf("containerframe MoveOutToRight, count = %d\n", count);
  // default behavior is to pass it on to the children
  nsIFrame* pFrame = GetFirstChild(nsnull);
  nsCOMPtr<nsIMathMLFrame> pMathMLFrame;
  if (leavingFrame)
  {
    while (pFrame && pFrame != leavingFrame) pFrame = pFrame->GetNextSibling();
    // assert pFrame == null or pFrame == leavingFrame)         
    if (pFrame) pFrame = pFrame->GetNextSibling();
  }  
  if (pFrame)
  {
    pMathMLFrame = do_QueryInterface(pFrame);  
    if (pMathMLFrame) pMathMLFrame->EnterFromLeft(aOutFrame, aOutOffset, count);
    else 
      printf("nsMathMLContainerFrame has non-MathML child\n");  //could be a text node BBM: fix this
  }
  else     
  {
    pFrame = GetParent();
    pMathMLFrame = do_QueryInterface(pFrame);
    if (pMathMLFrame) pMathMLFrame->MoveOutToRight(this, aOutFrame, aOutOffset, count);
    else // we have gone out of math.  Put the cursor at the end of the math if count == 0
         // and after the math if count == 1                                                                         
      PlaceCursorAfter(this, (count == 0), aOutFrame, aOutOffset, count);
  }
  return NS_OK;  
}

nsresult
nsMathMLForeignFrameWrapper::MoveOutToLeft(nsIFrame * leavingFrame, nsIFrame** aOutFrame, PRInt32* aOutOffset, PRUint32& count)
{                
  printf("containerframe MoveOutToLeft, count = %d\n", count);
  // same as above, but backwards in a singly-linked list
  nsCOMPtr<nsMathMLFrame> pMathMLFrame;
  nsIFrame * pFrame = GetFirstChild(nsnull);
  nsIContent * pContent;
  nsIFrame * pParent = nsnull;
  nsIFrame * pPrevious = nsnull;
  while (pFrame && pFrame != leavingFrame)
  {
    pPrevious = pFrame;
    pFrame = pFrame->GetNextSibling();
  }
  if (pPrevious) 
  {
    pMathMLFrame = do_QueryInterface(pPrevious);
    if (pMathMLFrame) pMathMLFrame->EnterFromRight(aOutFrame, aOutOffset, count);
    else  // next frame backward is not math -- probably text
      PlaceCursorAfter(pPrevious, PR_TRUE, aOutFrame, aOutOffset, count);
    
  }
  else // we are falling out of the mathml frame
  {
    pParent = GetParent();
    pMathMLFrame = do_QueryInterface(pParent);
    if (pMathMLFrame) pMathMLFrame->MoveOutToLeft(this, aOutFrame, aOutOffset, count);
    else // we have gone out of math; proceed only if count > 0. 
         // If count==0, leave the cursor at the beginning of math  
    {
//      if (count) 
//        (*paPos)->mMath = PR_FALSE;
      if (!count) //      else
      {
        PRUint32 nodeCount = 0;
        nsIFrame * pNode;
        pNode = pFrame->GetFirstChild(nsnull); 
        while (pNode && pNode != this)
        {
          nodeCount ++;
          pNode = pNode->GetNextSibling();
        }  // on 
        pContent = this->GetContent();
        *aOutOffset = ++nodeCount;
        *aOutFrame = this; 
//        (*paPos)->mMath = PR_TRUE;
      }
    }  
  }
  return NS_OK;  
}  

nsresult
nsMathMLForeignFrameWrapper::EnterFromLeft(nsIFrame** aOutFrame, PRInt32* aOutOffset, PRUint32& count)
{
  printf("containerframe EnterFromLeft, count = %d\n", count);
  nsIFrame * pFrame = GetFirstChild(nsnull);
  nsIContent * pContent;
  nsCOMPtr<nsMathMLFrame> pMathMLFrame;
  if (pFrame)
  {
    pMathMLFrame = do_QueryInterface(pFrame);
    if (pMathMLFrame) pMathMLFrame->EnterFromLeft(aOutFrame, aOutOffset, count);
    else // child frame is not a math frame. Probably a text frame. We'll assume this for not
    // BBM come back and fix this!
    {
      pContent = pFrame->GetContent();
      *aOutOffset = count;
      count = 0;
      *aOutFrame = pFrame; 
//      (*paPos)->mMath = PR_TRUE;
    }
  }
  else // this frame has no children
  {
    pMathMLFrame = do_QueryInterface(GetParent());
    if (pMathMLFrame) pMathMLFrame->MoveOutToRight(this, aOutFrame, aOutOffset, count);
    else // we have gone out of math or count == 0; proceed if count > 0. 
         // If count==0, leave the cursor at the end of this object  
    {
//      if (count) 
//        (*paPos)->mMath = PR_FALSE;
//      else
        if (!count)      {
        pContent = this->GetContent();
        *aOutOffset = pContent->GetChildCount();
        *aOutFrame = this; 
//        (*paPos)->mMath = PR_TRUE;
      }
    }  
  }
  return NS_OK;  
}

nsresult
nsMathMLForeignFrameWrapper::EnterFromRight(nsIFrame** aOutFrame, PRInt32* aOutOffset, PRUint32& count)
{
  printf("containerframe EnterFromRight, count = %d\n", count);
  nsIFrame * pFrame = GetFirstChild(nsnull);
  nsCOMPtr<nsMathMLFrame> pMathMLFrame;
  if (!pFrame)
  {
   pMathMLFrame = do_QueryInterface(GetParent());
   if (pMathMLFrame) pMathMLFrame->EnterFromRight(aOutFrame, aOutOffset, count);
  }
  while (pFrame->GetNextSibling() != nsnull)
    pFrame = pFrame->GetNextSibling();
  if (!pFrame) printf("Error in nsMathMLContainerFrame::EnterFromRight()\n");   
  else
  {
    pMathMLFrame = do_QueryInterface(pFrame);
    if (pMathMLFrame) pMathMLFrame->EnterFromRight(aOutFrame, aOutOffset, count);
  }
  return NS_OK;  
}

