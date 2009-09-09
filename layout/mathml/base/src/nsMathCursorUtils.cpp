/* Copyright 2008, MacKichan Software, Inc. */

#include "nsCom.h"
#include "nsFrameSelection.h"
#include "nsMathCursorUtils.h"
#include "nsMathMLCursorMover.h"

PRBool IsMathFrame( nsIFrame * aFrame );

PRBool PlaceCursorAfter( nsIFrame * pFrame, PRBool fInside, nsIFrame** aOutFrame, PRInt32* aOutOffset, PRInt32& count)
{
  nsIFrame * pChild;
  nsIFrame * pParent;
  nsCOMPtr<nsIContent> pContent;
  pParent = pFrame->GetParent();

  nsCOMPtr<nsIMathMLCursorMover> pMCM;
  if (fInside) // we put the cursor at the end of the contents of pFrame; we do not recurse.
  {
    // find the last child
    pChild = GetLastTextFrame(pFrame);
    if (pChild) *aOutFrame = pChild;
    else return PR_FALSE;
    nsAutoString value;
    *aOutOffset = (pChild->GetContent())->TextLength();
  }
  else // don't put the cursor inside the tag
  {
    if (IsMathFrame(pParent))
    {
      pContent = pParent->GetContent();
      *aOutOffset = 1+pContent->IndexOf(pFrame->GetContent());
      *aOutFrame = pParent;
    }
    else
    {
      *aOutFrame = GetFirstTextFramePastFrame(pFrame);
      *aOutOffset = count;
    } 
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
    pChild = GetFirstTextFrame(pFrame);
    if (pChild)
    {
      *aOutOffset = count;
      count = 0;
      *aOutFrame = pChild; 
    }
  }
  else // don't put the cursor inside the tag
  {
    if (count == 0)
    {
      pParent = pFrame->GetParent();
      pContent = pParent->GetContent();
      *aOutOffset = pContent->IndexOf(pFrame->GetContent());
      *aOutFrame = pParent;
    }
    else
    { 
      pChild = GetLastTextFrameBeforeFrame(pFrame);
      *aOutFrame = pChild;
      *aOutOffset = (pChild->GetContent())->TextLength() - count;
     }
  }
  return PR_TRUE;
}


nsIFrame * GetFirstTextFrame( nsIFrame * pFrame )
{
  if (!pFrame) return nsnull;
  nsIAtom* type = pFrame->GetType();
  nsIFrame * pRet = nsnull;
  nsIFrame * pChild = nsnull; 
  if (type == nsGkAtoms::textFrame )
    return pFrame;
  else
  {
    pChild = pFrame->GetFirstChild(nsnull);
    while (pChild && !(pRet = GetFirstTextFrame(pChild)))
    {
      pChild = pChild->GetNextSibling();
      pRet = GetFirstTextFrame( pChild);
    }
    return pRet;
  }
}

nsIFrame * GetPrevSib(nsIFrame * pFrame)
{
  nsIFrame * pTemp = pFrame->GetParent()->GetFirstChild(nsnull);
  while (pTemp && (pTemp->GetNextSibling() != pFrame))
    pTemp = pTemp->GetNextSibling();
  return pTemp;
}

nsIFrame * GetLastChild(nsIFrame * pFrame)
{
  if (!pFrame) return nsnull;
  nsIFrame * pTemp = pFrame->GetFirstChild(nsnull);
  while (pTemp && pTemp->GetNextSibling()) pTemp = pTemp->GetNextSibling();
  return pTemp;
} 

nsIFrame * GetFirstTextFramePastFrame( nsIFrame * pFrame )
{
  nsIFrame *pTemp = pFrame;
  nsIFrame *pTextFrame = nsnull;
  while (!pTextFrame && pTemp)
  {
    while (pTemp && !pTemp->GetNextSibling())
    {
      pTemp = pTemp->GetParent();
    }
    pTemp = pTemp = pTemp->GetNextSibling();
    pTextFrame = GetFirstTextFrame(pTemp);
  }
  return pTextFrame;
}


nsIFrame * GetLastTextFrame( nsIFrame * pFrame )
{
  if (!pFrame) return nsnull;
  nsAutoString textContents;
  nsIAtom* type = pFrame->GetType();
  nsIFrame * pRet = nsnull;
  nsIFrame * pChild = nsnull; 
  nsIContent * pContent = nsnull;
  if (type == nsGkAtoms::textFrame) 
    return pFrame;
  else
  {
    pChild = GetLastChild(pFrame);
    while (PR_TRUE)
    {
      while (pChild && !(pRet = GetLastTextFrame(pChild)))
      {
        pChild = GetPrevSib(pChild);
        pRet = GetLastTextFrame( pChild);
      }
      if (pRet)
      {
        pContent = pRet->GetContent();
        if (!pContent->TextIsOnlyWhitespace()) return pRet;
      }
      else return pRet;
    }
  }
}											   


nsIFrame * GetLastTextFrameBeforeFrame( nsIFrame * pFrame )
{
  nsIFrame *pTemp = pFrame;
  nsIFrame *pTextFrame = nsnull;
  while (!pTextFrame && pTemp)
  {
    while (pTemp && !GetPrevSib(pTemp))
    {
      pTemp = pTemp->GetParent();
    }
    pTemp = GetPrevSib(pTemp);
    pTextFrame = GetLastTextFrame(pTemp);
  }
  return pTextFrame;
}

											   
// DOM tree navigation routines that pass over ignorable white space.
// See the "Whitespace in the DOM" article on the MDC
/*

PRBool IsAllWS( nsIDOMNode * node)
{
  nsCOMPtr<nsIContent> pContent = do_QueryInterface(node);
  if (pContent)
    return pContent->TextIsOnlyWhitespace();
  return PR_FALSE;
}


PRBool IsIgnorable( nsIDOMNode * node)
{
  PRUint16 aType;
  node->GetNodeType(&aType);
  if (aType == (PRUint16)nsIDOMNode::TEXT_NODE)
    return IsAllWS(node);
  else if (aType == (PRUint16)nsIDOMNode::COMMENT_NODE)
    return PR_TRUE;
  return PR_FALSE;
}


nsIDOMNode * NodeBefore( nsIDOMNode * node)
{
  nsCOMPtr<nsIDOMNode> sibling(node);
  do 
  {
     sibling->GetPreviousSibling(getter_AddRefs(sibling));
  } while (sibling && IsIgnorable(sibling));
  return sibling;
}

nsIDOMNode * NodeAfter( nsIDOMNode * node)
{
  nsCOMPtr<nsIDOMNode> sibling(node);
  do 
  {
     sibling->GetNextSibling(getter_AddRefs(sibling));
  } while (sibling && IsIgnorable(sibling));
  return sibling;
}

nsIDOMNode * FirstChild( nsIDOMNode * parent)
{
  nsCOMPtr<nsIDOMNode> sibling;
  parent->GetFirstChild(getter_AddRefs(sibling));
  while (sibling && IsIgnorable(sibling))
  {
     sibling->GetNextSibling(getter_AddRefs(sibling));
  }
  return sibling;
}

nsIDOMNode * LastChild( nsIDOMNode * parent)
{
  nsCOMPtr<nsIDOMNode> sibling;
  parent->GetLastChild(getter_AddRefs(sibling));
  while (sibling && IsIgnorable(sibling))
  {
     sibling->GetPreviousSibling(getter_AddRefs(sibling));
  }
  return sibling;
}


*/