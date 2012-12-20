/* Copyright 2008, MacKichan Software, Inc. */

#include "nsCom.h"
#include "nsFrameSelection.h"
#include "nsMathCursorUtils.h"
#include "nsMathMLCursorMover.h"
#include "nsIDOMNodeList.h"
#include "nsGkAtoms.h"

PRBool IsMathFrame( nsIFrame * aFrame );
nsIFrame * GetLastChild(nsIFrame * pFrame);
PRBool IsDisplayFrame( nsIFrame * aFrame, PRInt32& count )
{
  PRBool retval = PR_FALSE;
  nsCOMPtr<nsIDOMElement> element = do_QueryInterface(aFrame->GetContent());
  if (element)
  {
    nsAutoString stringTag;
    element->GetLocalName(stringTag);
    retval = (stringTag.EqualsLiteral("msidisplay"));
  }
  return retval;
}

    
PRBool PlaceCursorAfter( nsIFrame * pFrame, PRBool fInside, nsIFrame** aOutFrame, PRInt32* aOutOffset, PRInt32& count)
{
  nsIFrame * pParent;
  nsIFrame * pChild;
  nsIFrame * pSiblingFrame;
  nsCOMPtr<nsIContent> pContent;
  pParent = GetSignificantParent(pFrame);

  // BBM provisional code
  // Cursor doesn't show up if it is inside an mo tag
  if (pFrame->GetContent()->Tag() == nsGkAtoms::mo_) {
    fInside = PR_FALSE;
  }
  else if ( GetSignificantParent(pFrame)->GetContent()->Tag() == nsGkAtoms::mo_)
  {
    fInside = PR_FALSE;
    pFrame = GetSignificantParent(pFrame);
  }

  nsCOMPtr<nsIMathMLCursorMover> pMCM;
  if (fInside) // we put the cursor at the end of the contents of pFrame; we do not recurse.
  {
    nsIAtom* frametype = pFrame->GetType();
    if (frametype == nsGkAtoms::textFrame) {
      *aOutFrame = pFrame;
      *aOutOffset = pFrame->GetContent()->TextLength();
      count = 0;
      return NS_OK;
    }    
     // find the last child
    pChild = GetLastChild(pFrame);
    pContent = pFrame->GetContent();
    while (pChild && pContent == pChild->GetContent())
    {
      pChild = GetLastChild(pChild);
    }
    if (pChild && pContent) {
      // nsIAtom* type = pChild->GetType();
      // if (type == nsGkAtoms::textFrame )
      // {
      //   *aOutFrame = pChild;
      //   *aOutOffset = -1;
      // }
      // else if (pChild){
        *aOutOffset = 1+pContent->IndexOf(pChild->GetContent());
        *aOutFrame = pFrame;
 //     }
    }
    else
    {
      // BBM: This is left over from previous iterations and probably never runs
  		*aOutFrame = pFrame;
  	  nsCOMPtr<nsIDOMElement> element = do_QueryInterface(pFrame->GetContent());
      nsCOMPtr<nsIDOMNodeList> nodelist;
  		nsresult res = element->GetChildNodes(getter_AddRefs(nodelist));
  		PRUint32 countofNodes;
  		res = nodelist->GetLength(&countofNodes);
  		*aOutOffset = countofNodes;
    }
  }
  else // don't put the cursor inside the tag
  {
    if (IsMathFrame(pParent))
    {
      pContent = pParent->GetContent();
      *aOutOffset = 1+pContent->IndexOf(pFrame->GetContent());
      *aOutFrame = pParent;
      //check to see if next frame is a temp input
      pSiblingFrame = pFrame->GetNextSibling();
      if (pSiblingFrame) {
        pContent = pSiblingFrame->GetContent();
        nsCOMPtr<nsIDOMElement> pElem = do_QueryInterface(pContent);
        nsAutoString attrVal;
        pElem ->GetAttribute(NS_LITERAL_STRING("tempinput"), attrVal);
        if (attrVal.EqualsLiteral("true"))
        {
          *aOutFrame = pSiblingFrame;
          *aOutOffset = 0;
        }
      } 
    }
//    else if (IsDisplayFrame(pParent, count))
//    {
//      *aOutFrame = GetSignificantParent(pParent);
//      pContent = (*aOutFrame)->GetContent();
//      *aOutOffset = 1+pContent->IndexOf(pParent->GetContent());
//    }
    else
    {
      *aOutFrame = GetFirstTextFramePastFrame(pFrame);
      *aOutOffset = count;
      if (*aOutFrame == nsnull)
      {
        count = 0;
        pFrame->MoveRightAtDocEndFrame( aOutFrame, *aOutOffset);
      }
//	    pParent = GetSignificantParent(pFrame);
//	    *aOutFrame = pParent;
//			(*aOutOffset) = 1;
//			pChild = pParent->GetFirstChild(nsnull);
//			while (pChild && pChild != pFrame)
//			{
//				pChild = pChild->GetNextSibling();
//				(*aOutOffset)++;
//			}
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

    // BBM provisional code
  // Cursor doesn't show up if it is inside an mo tag, so we arrange to have it outside
  if (pFrame->GetContent()->Tag() == nsGkAtoms::mo_) {
    fInside = PR_FALSE;
  }
  else if ( GetSignificantParent(pFrame)->GetContent()->Tag() == nsGkAtoms::mo_)
  {
    fInside = PR_FALSE;
    pFrame = GetSignificantParent(pFrame);
  }

  if (fInside)
  {
    pChild = GetFirstTextFrame(pFrame);
    if (pChild)
    {
      count = 0;
      *aOutOffset = count;
      *aOutFrame = pChild;
    }
  }
  else // don't put the cursor inside the tag
  {
    pParent = GetSignificantParent(pFrame);
    pContent = pParent->GetContent();
    if (count == 0)
    {
      pParent = GetSignificantParent(pFrame);
      pContent = pParent->GetContent();
      *aOutOffset = pContent->IndexOf(pFrame->GetContent());
      *aOutFrame = pParent;
    }
    else
    {
      pChild = GetLastTextFrameBeforeFrame(pFrame);
      *aOutFrame = pChild;
      if (pChild)
      {
       nsIAtom*  frameType = pChild->GetType();
       if (nsGkAtoms::textFrame == frameType)
         *aOutOffset = (pChild->GetContent())->TextLength() - count;
       else
         *aOutOffset = 0;
      }
      else
      {
        count = 0;
        return pFrame->MoveLeftAtDocStartFrame( aOutFrame, *aOutOffset);
      }
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
  {
    if (!(pFrame->GetContent()->TextIsOnlyWhitespace()))
      return pFrame;
  }
  pChild = pFrame->GetFirstChild(nsnull);
  while (pChild && !(pRet = GetFirstTextFrame(pChild)))
  {
    pChild = pChild->GetNextSibling();
    pRet = GetFirstTextFrame( pChild);
  }
  return pRet;
}

nsIFrame * GetPrevSib(nsIFrame * pFrame)
{
  nsIFrame * pTemp = GetSignificantParent(pFrame)->GetFirstChild(nsnull);
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
      pTemp = GetSignificantParent(pTemp);
    }
    if (pTemp) pTemp = pTemp->GetNextSibling();
    if (pTemp) pTextFrame = GetFirstTextFrame(pTemp);
    else return nsnull;  // TODO: BBM can't return null without caller checking. Check this out.
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
    //if (!(pFrame->GetContent()->TextIsOnlyWhitespace()))
      return pFrame;
  PRInt16 n = 0;
  pChild = GetLastChild(pFrame);
  while (n++ < 1000)  // was PR_TRUE
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
//    if (pChild == pRet) return nsnull;
  }
//  return nsnull;
}

nsIFrame * GetLastTextFrameBeforeFrame( nsIFrame * pFrame ) // if there is no previous text frame, this will return the frame the
//cursor should be in.
{
  nsIFrame *pTemp = pFrame;
  nsIFrame *pNextTemp = nsnull;
  nsIFrame *pTextFrame = nsnull;
  nsCOMPtr<nsIDOMNode> pNode;
  nsAutoString tagName;
  pNode = do_QueryInterface(pTemp->GetContent());
  pNode->GetNodeName(tagName);
  while (!pTextFrame && !tagName.EqualsLiteral("body") && pTemp)
  {
    while (pTemp && !tagName.EqualsLiteral("body") && !GetPrevSib(pTemp))
    {
      pNextTemp = GetSignificantParent(pTemp);
      pNode = do_QueryInterface( pNextTemp ->GetContent());
      pNode->GetNodeName(tagName);
      if (!tagName.EqualsLiteral("body"))
          pTemp = pNextTemp;
    }
    pNextTemp = GetPrevSib(pTemp);
    if (pNextTemp)
      pTextFrame = GetLastTextFrame(pNextTemp);
    else
      pTextFrame = pTemp;
  }
  return pTextFrame;
}

nsIFrame * GetSignificantParent(nsIFrame * pFrame)
{
  nsCOMPtr<nsIContent> pContent = pFrame->GetContent();
  nsIFrame * rval = pFrame->GetParent();
  while (rval && rval->GetContent() == pContent)
    rval = rval->GetParent();
  return rval;
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