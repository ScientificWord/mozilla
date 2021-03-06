/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* Copyright 2008, MacKichan Software, Inc.*/

#ifndef nsMathMLContainerCursorMover_h__
#define nsMathMLContainerCursorMover_h__

#include "nsCOMPtr.h"
#include "nsIAtom.h"
#include "nsIFrame.h"
#include "nsMathMLCursorMover.h"
#include "nsIMathMLCursorMover.h"

class nsMathMLContainerCursorMover : public nsIMathMLCursorMover {
public:
  NS_DECL_ISUPPORTS
  
  NS_IMETHOD 
  MoveOutToRight(nsIFrame *leavingFrame, nsIFrame **aOutFrame, PRInt32* aOutOffset, PRInt32 count, PRBool* fBailingOut, PRInt32 *_retval);

  NS_IMETHOD 
  MoveOutToLeft(nsIFrame *leavingFrame, nsIFrame **aOutFrame, PRInt32* aOutOffset, PRInt32 count, PRBool* fBailingOut, PRInt32 *_retval);

  NS_IMETHOD 
  EnterFromLeft(nsIFrame *leavingFrame, nsIFrame **aOutFrame, PRInt32* aOutOffset, PRInt32 count, PRBool* fBailingOut, PRInt32 *_retval);

  NS_IMETHOD 
  EnterFromRight(nsIFrame *leavingFrame, nsIFrame **aOutFrame, PRInt32* aOutOffset, PRInt32 count, PRBool* fBailingOut, PRInt32 *_retval);

  nsMathMLContainerCursorMover(nsIFrame * pFrame, PRBool visDistinct = PR_FALSE) : m_pMyFrame(pFrame), m_fVisDistinct(visDistinct) {};

  static nsMathMLContainerCursorMover* 
  Create(nsIFrame * pFrame, PRBool visDistinct = PR_FALSE) {
    nsMathMLContainerCursorMover * pcm = new nsMathMLContainerCursorMover(pFrame, visDistinct);
  //  if (pcm) pcm->AddRef();
    return pcm;
  }

private:
  nsIFrame * m_pMyFrame;
  PRBool m_fVisDistinct;  // If true, the container is visually distinct so that the count of movements should decrement when the cursor leaves the container.
  // Examples: false for mrow and mi and mo, true for fraction numerators, table cells, table rows, etc.
};



#endif /* nsMathMLContainerCursorMover_h__ */
