/* Copyright 2008, MacKichan Software, Inc. */

#include "nsCom.h"
#include "nsFrameSelection.h"
#include "nsMathCursorUtils.h"
#include "nsMathMLCursorMover.h"
#include "nsIDOMNodeList.h"
#include "nsGkAtoms.h"
#include "nsFrame.h"
#include "msiITagListManager.h"
#include "nsIEditor.h"
#include "nsIDOMText.h"
#include "../../editor/libeditor/base/nsEditor.h"
#include "../../editor/libeditor/base/nsEditorUtils.h"


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
  nsCOMPtr<nsIContent> pDisplayParentContent;
  nsCOMPtr<nsIDOMDocument> doc;
  pParent = GetSignificantParent(pFrame);

  // BBM provisional code
  // Cursor doesn't show up if it is inside an mo tag
  // if (pFrame->GetContent()->Tag() == nsGkAtoms::mo_) {
  //   fInside = PR_FALSE;
  // }
  // else if ( GetSignificantParent(pFrame)->GetContent()->Tag() == nsGkAtoms::mo_)
  // {
  //   fInside = PR_FALSE;
  //   pFrame = GetSignificantParent(pFrame);
  // }

  // nsCOMPtr<nsIMathMLCursorMover> pMCM;
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
    else
    {
      PRBool fSelectable;
      count = 0;
      // special case. We are leaving math, so we need to see if  we are in a math display.
      // If so, leave that too.
      pContent = pParent->GetContent();
      nsCOMPtr<nsIDOMElement> pElem = do_QueryInterface(pContent);
      if (pElem) {
        nsAutoString tag;
        pElem->GetTagName(tag);
        if (tag.EqualsLiteral("msidisplay")) {
          pFrame = pParent;
        }
      }

      pSiblingFrame = pFrame->GetNextSibling();
      // Whoa! We must check that the cursor can go into this tag -- it sometimes is a <br/>
      if (pSiblingFrame) {
        pSiblingFrame->IsSelectable( &fSelectable, nsnull);
      }
      if (pSiblingFrame && fSelectable) {
        *aOutFrame = pParent;
        pChild = pParent->GetFirstChild(nsnull);
        *aOutOffset = 1;
        while (pChild && (pChild != pFrame))
        {
          pChild = pChild->GetNextSibling();
          (*aOutOffset)++;
        }
      } else if (pSiblingFrame) {
        *aOutFrame = pSiblingFrame;
        *aOutOffset = 0;
      }
      else
      {
        *aOutFrame = GetFirstTextFramePastFrame(pFrame);
        *aOutOffset = count;
        if (*aOutFrame == nsnull) {
          pFrame->MoveRightAtDocEnd( nsnull );
        }
        // nsCOMPtr<nsIDOMNode> frameNode = do_QueryInterface(pFrame->GetContent());
        // nsCOMPtr<nsIDOMNode> dummy;
        // parentNode->GetOwnerDocument(getter_AddRefs(doc));
        // nsCOMPtr<nsIDOMText>text;
        // doc->CreateTextNode(NS_LITERAL_STRING(" "), getter_AddRefs(text));
        // nsCOMPtr<nsIContent> textContent = do_QueryInterface(text);
        // nsCOMPtr<nsIDOMNode> textNode = do_QueryInterface(text);
        // parentNode->InsertAfter( textNode, frameNode, getter_AddRefs(dummy));
        // *aOutFrame = pFrame->GeNextSibling();
        // *aOutOffset = 0;
      }
    }
  }
  return PR_TRUE;
}

PRBool PlaceCursorBefore( nsIFrame * pFrame, PRBool fInside, nsIFrame** aOutFrame, PRInt32* aOutOffset, PRInt32& count)
{
  nsIFrame * pChild = nsnull;
  nsIFrame * pParent = nsnull;
  nsIFrame * pSiblingFrame = nsnull;
  pParent = GetSignificantParent(pFrame);
  nsCOMPtr<nsIDOMDocument> doc;
  nsCOMPtr<nsIContent> pContent;
  // nsCOMPtr<nsIMathMLCursorMover> pMCM;

    // BBM provisional code
  // Cursor doesn't show up if it is inside an mo tag, so we arrange to have it outside
  // if (pFrame->GetContent()->Tag() == nsGkAtoms::mo_) {
  //   fInside = PR_FALSE;
  // }
  // else if ( GetSignificantParent(pFrame)->GetContent()->Tag() == nsGkAtoms::mo_)
  // {
  //   fInside = PR_FALSE;
  //   pFrame = GetSignificantParent(pFrame);
  // }

  if (fInside)
  {
    // BBM: modified 2013-09-27
    // pChild = GetFirstTextFrame(pFrame);
    // if (pChild)
    // {
    //   count = 0;
    //   *aOutOffset = count;
    //   *aOutFrame = pChild;
    // }
    *aOutOffset = 0;
    *aOutFrame = pFrame;
  }
  else // don't put the cursor inside the tag
  {
    *aOutFrame = pFrame->GetParent();
    *aOutOffset = mmlFrameGetIndexInParent(pFrame, *aOutFrame);
    if (IsMathFrame(pParent))
    {
      pContent = pParent->GetContent();
      *aOutOffset = pContent->IndexOf(pFrame->GetContent());
      *aOutFrame = pParent;
     //check to see if previous frame is a temp input
      pSiblingFrame = pFrame->GetPrevSibling();
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
    else
    {
      count = 0;

      // special case. We are leaving math, so we need to see if  we are in a math display.
      // If so, leave that too.
      pContent = pParent->GetContent();
      nsCOMPtr<nsIDOMElement> pElem = do_QueryInterface(pContent);
      if (pElem) {
        nsAutoString tag;
        pElem->GetTagName(tag);
        if (tag.EqualsLiteral("msidisplay")) {
          pFrame = pParent;
        }
      }
      pSiblingFrame = pFrame->GetPrevSibling();
      if (pSiblingFrame) {
        PRUint32 textlength;
        *aOutFrame = pSiblingFrame;
        nsIAtom * frameType = (pSiblingFrame)->GetType();
        if (nsGkAtoms::textFrame == frameType) {
          textlength = (pSiblingFrame)->GetContent()->TextLength();
          *aOutOffset = textlength;
        } else
          *aOutOffset = (PRUint32(-1));
      }
      else
      {
        // We don't want to go to a text frame. Suppose the math is the
        // first item in a paragraph. We want the cursor before the math, but
        // in the paragraph.
        // pChild = GetLastTextFrameBeforeFrame(pFrame);
        // if (pChild)
        // {
        //   *aOutFrame = pChild;
        //   nsIAtom*  frameType = pChild->GetType();
        //   if (nsGkAtoms::textFrame == frameType)
        //     *aOutOffset = (pChild->GetContent())->TextLength() - count;
        //   else
        //     *aOutOffset = 0;
        // }
        // else
        //   pFrame->MoveLeftAtDocStart( nsnull);
        nsCOMPtr<nsIDOMNode> parentNode = do_QueryInterface(pParent->GetContent());
        nsCOMPtr<nsIDOMNode> frameNode = do_QueryInterface(pFrame->GetContent());
        nsCOMPtr<nsIDOMNode> dummy;
        parentNode->GetOwnerDocument(getter_AddRefs(doc));
        nsCOMPtr<nsIDOMText>text;
        doc->CreateTextNode(NS_LITERAL_STRING(" "), getter_AddRefs(text));
        nsCOMPtr<nsIContent> textContent = do_QueryInterface(text);
        nsCOMPtr<nsIDOMNode> textNode = do_QueryInterface(text);
        parentNode->InsertBefore( textNode, frameNode, getter_AddRefs(dummy));
        *aOutFrame = pFrame->GetPrevSibling();
        *aOutOffset = 0;
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
  if (nsFrame::IsMSIPlotOrGraphicContainer( pFrame->GetContent() ))
    return pFrame;
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
  if (nsFrame::IsMSIPlotOrGraphicContainer( pFrame->GetContent() ))
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
  nsIFrame *pNextTemp = pFrame; //initialize with this to start the loop
  nsIFrame *pTextFrame = nsnull;
  nsCOMPtr<nsIDOMNode> pNode;
  nsAutoString tagName;
  pNode = do_QueryInterface(pTemp->GetContent());
  pNode->GetNodeName(tagName);
  while (!pTextFrame && !tagName.EqualsLiteral("body") && pTemp)
  {
    pTemp = pNextTemp;
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
// Finds the first ancestor that has different content from pFrame
{
  nsCOMPtr<nsIContent> pContent = pFrame->GetContent();
  nsCOMPtr<nsIContent> pContentNew;
  nsIFrame * rval = pFrame->GetParent();
  if (rval) {
    pContentNew = rval->GetContent();
  }
  while (rval && (pContentNew == pContent)) {
    rval = rval->GetParent();
    if (rval) {
      pContentNew = rval->GetContent();
    }
  }
  return rval;
}

nsIFrame * GetTopFrameForContent(nsIFrame * pFrame)
{
  if (!pFrame) return nsnull;
  nsCOMPtr<nsIContent> pContent = pFrame->GetContent();
  nsIFrame * rval = pFrame;
  nsIFrame * pParent;
  pParent = pFrame->GetParent();
  while (pParent->GetContent() == pContent) {
  	rval = pParent;
    pParent = pParent->GetParent();
  }
  return rval;
}

PRUint32
mmlFrameGetIndexInParent( nsIFrame * pF, nsIFrame * pParent)
{
  nsIFrame * pF2 = pParent->GetFirstChild(nsnull);
  PRUint32 index = 0;
  while (pF2 && pF != pF2)
  {
    index++;
    pF2 = pF2->GetNextSibling();
  }
  if (!pF2) return -1;
  return index;
}

PRUint32 GetFrameChildCount(nsIFrame * pParent)
{
  nsIFrame * pF = pParent->GetFirstChild(nsnull);
  PRUint32 index = 0;
  while (pF) {
    index++;
    pF = pF->GetNextSibling();
  }
  return index;
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
