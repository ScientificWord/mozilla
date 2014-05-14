#include "msiISelection.h"
#include "nsIPrivateDOMEvent.h"
#include "msiIMathMLEditor.h"
#include "../../../editor/libeditor/base/jcsDumpNode.h"

class msiTypedSelection : public nsTypedSelection,
                          public msiISelection
{
public:
  msiTypedSelection();
  msiTypedSelection(nsFrameSelection *aList);
  virtual ~msiTypedSelection();
  
  NS_DECL_ISUPPORTS_INHERITED
  NS_DECL_MSIISELECTION
  // override nsISelection
  // certain selection operations require that the msi data be sync'ed with ns data.
  // TODO -- mtables and msi selection ????
  NS_IMETHOD Collapse(nsIDOMNode *parentNode, PRInt32 offset);
  NS_IMETHOD Extend(nsIDOMNode *parentNode, PRInt32 offset);
  NS_IMETHOD CollapseToStart(void);
  NS_IMETHOD CollapseToEnd(void);
  NS_IMETHOD AddRange(nsIDOMRange *range);
  NS_IMETHOD RemoveRange(nsIDOMRange *range);
  NS_IMETHOD RemoveAllRanges(void);
  // end override nsISelection
  NS_IMETHOD Adjust();

protected:
  PRBool IsVoidOfContent(nsIDOMNode * start, PRUint32 startOff, nsIDOMNode* end, PRUint32 endOff);
  PRBool IsMouseEventActive();
  
protected:
  //nsCOMPtr<nsIDOMNode>  m_msiFocusNode;
  //nsCOMPtr<nsIDOMNode>  m_msiAnchorNode;
  //PRUint32              m_msiFocusOffset;
  //PRUint32              m_msiAnchorOffset;
  msiAdjustCaretCB      m_adjustCaretCB; 
  msiSetSelectionCB     m_setSelectionCB;
  void *                m_msiEditor;
  nsCOMPtr<nsIDOMEvent> m_mouseEvent;
};


msiTypedSelection::msiTypedSelection() 
: nsTypedSelection(), // m_msiFocusOffset(INVALID_OFFSET), m_msiAnchorOffset(INVALID_OFFSET),
m_adjustCaretCB(nsnull), m_setSelectionCB(nsnull), m_msiEditor(nsnull)
{
}

msiTypedSelection::msiTypedSelection(nsFrameSelection* aList)
: nsTypedSelection(aList), //m_msiFocusOffset(INVALID_OFFSET), m_msiAnchorOffset(INVALID_OFFSET),
m_adjustCaretCB(nsnull), m_setSelectionCB(nsnull), m_msiEditor(nsnull)
{
}

msiTypedSelection::~msiTypedSelection()
{
}

// ISupports
NS_IMPL_ISUPPORTS_INHERITED1(msiTypedSelection, nsTypedSelection, msiISelection)


// msiISelection interface
NS_IMETHODIMP 
msiTypedSelection::GetMsiAnchorNode(nsIDOMNode* *anAnchorNode)
{
  if (!anAnchorNode)
    return NS_ERROR_NULL_POINTER;
  *anAnchorNode = nsnull;

  nsIDOMNode* myAnchorNode = FetchAnchorNode();
  if (myAnchorNode)
  {
    *anAnchorNode = myAnchorNode;
    NS_ADDREF(*anAnchorNode);
  }
  return NS_OK;
}

NS_IMETHODIMP 
msiTypedSelection::GetMsiAnchorOffset(PRUint32 *msiAnchorOffset)
{
  if (!msiAnchorOffset)
    return NS_ERROR_NULL_POINTER;
  *msiAnchorOffset = FetchAnchorOffset();
  return NS_OK;  
}

NS_IMETHODIMP 
msiTypedSelection::GetMsiFocusNode(nsIDOMNode* *aFocusNode)
{
  if (!aFocusNode)
    return NS_ERROR_NULL_POINTER;
  *aFocusNode = nsnull;

  nsIDOMNode* myFocusNode = FetchFocusNode();
  if (myFocusNode)
  {
    *aFocusNode = myFocusNode;
    NS_ADDREF(*aFocusNode);
  }
  return NS_OK;  
}

NS_IMETHODIMP 
msiTypedSelection::GetMsiFocusOffset(PRUint32 *msiFocusOffset)
{
  if (!msiFocusOffset)
    return NS_ERROR_NULL_POINTER;
  *msiFocusOffset = FetchFocusOffset();

  return NS_OK;  
}

NS_IMETHODIMP 
msiTypedSelection::Set(nsIDOMNode *startNode, PRUint32 startOffset, 
                       nsIDOMNode *endNode, PRUint32 endOffset, 
                       nsIDOMNode *focusNode, PRUint32 focusOffset, 
                       nsIDOMNode *anchorNode, PRUint32 anchorOffset)
{
  if (!startNode || !endNode || !focusNode || !anchorNode)
    return NS_ERROR_FAILURE;
  if (startOffset == INVALID_OFFSET || endOffset == INVALID_OFFSET ||
      focusOffset == INVALID_OFFSET || anchorOffset == INVALID_OFFSET)
    return NS_ERROR_FAILURE;
  nsresult res(NS_OK);
  if (startNode == endNode && startOffset == endOffset)
    return Collapse(anchorNode, anchorOffset);
  if (IsVoidOfContent(startNode, startOffset, endNode, endOffset))
    return Collapse(anchorNode, anchorOffset);
  nsIDOMNode * newNSAnchor = nsnull;  
  nsIDOMNode * newNSFocus = nsnull;
  PRUint32 newNSAnchorOffset(INVALID_OFFSET), newNSFocusOffset(INVALID_OFFSET);
  nsCOMPtr<nsIRangeUtils> ru;
  NS_NewRangeUtils(getter_AddRefs(ru));
  PRBool focusBeforeAnchor = ru->ComparePoints(focusNode, focusOffset, anchorNode, anchorOffset) < 0;
  if (focusBeforeAnchor)
  { 
    newNSAnchor = endNode;
    newNSFocus = startNode;
    newNSAnchorOffset = endOffset;
    newNSFocusOffset = startOffset;
  }
  else
  {
    newNSAnchor = startNode;
    newNSFocus = endNode;
    newNSAnchorOffset = startOffset;
    newNSFocusOffset = endOffset;
  }
  if (anchorNode && anchorOffset != INVALID_OFFSET && focusNode && focusOffset != INVALID_OFFSET)
  {
    if (newNSAnchor == FetchFocusNode() && newNSAnchorOffset == FetchFocusOffset())
    {
      nsDirection direction = GetDirection();
      direction = eDirNext == direction ? eDirPrevious : eDirNext;
      SetDirection(direction); //this will cause newNSAnchor to be viewed as the anchor.
    }
    else if (newNSAnchor != FetchAnchorNode() || newNSAnchorOffset != FetchAnchorOffset())
    {
      res =  nsTypedSelection::Collapse(newNSAnchor, newNSAnchorOffset);
      if (NS_SUCCEEDED(res))
      {
        nsDirection direction = GetDirection();
        direction = eDirNext == direction ? eDirPrevious : eDirNext;
        SetDirection(direction); //this will cause newNSAnchor to be viewed as the anchor.
      }  
    }
     if (NS_SUCCEEDED(res))
     {
        if (FetchFocusNode() ==  newNSFocus && FetchFocusOffset() == newNSFocusOffset) // nothing to do
          res = NS_OK; //ljh 6/06: Extend returns an error in this case which seems wrong -- I don't want to change it's behavior because of other clients.
        else  
          res =  nsTypedSelection::Extend(newNSFocus, newNSFocusOffset);
     } 
  }
  else 
    res = NS_ERROR_FAILURE;
  if (NS_SUCCEEDED(res))
  {
    if (anchorNode && anchorOffset != INVALID_OFFSET && focusNode && focusOffset != INVALID_OFFSET)
    {
      //m_msiFocusNode = focusNode;
      //m_msiAnchorNode = anchorNode;
      //m_msiFocusOffset = focusOffset;
      //m_msiAnchorOffset = anchorOffset;
    }
    else
      SyncMSIwithNS();
  }
  return res;  
}

NS_IMETHODIMP
msiTypedSelection::InitalizeCallbackFunctions(msiAdjustCaretCB adjustCaretCB, 
                                              msiSetSelectionCB setSelectionCB, 
                                              void * msiEditor)
{
  m_msiEditor = msiEditor;
  m_adjustCaretCB = adjustCaretCB;
  m_setSelectionCB = setSelectionCB;
  return NS_OK;
}  

NS_IMETHODIMP
msiTypedSelection::SetDOMEvent(nsIDOMEvent * mouseEvent)
{
  m_mouseEvent = mouseEvent;
  return NS_OK;
}




// end msiISelection interface

// overwrite of nsISelection
NS_IMETHODIMP 
msiTypedSelection::Collapse(nsIDOMNode *parentNode, PRInt32 offset)
{ 
  nsresult res(NS_OK);
  nsCOMPtr<nsIDOMNode> adjustNode(parentNode);
  PRInt32 adjustOffset(offset);
  if (m_adjustCaretCB && IsMouseEventActive() && m_msiEditor)
    res = m_adjustCaretCB(m_msiEditor, m_mouseEvent, adjustNode, adjustOffset);
  if (NS_SUCCEEDED(res))  
    res =  nsTypedSelection::Collapse(adjustNode, adjustOffset);
  SyncMSIwithNS();
  SetDOMEvent(nsnull); // only use a mouse event once
  return res;
}

NS_IMETHODIMP 
msiTypedSelection::Extend(nsIDOMNode *parentNode, PRInt32 offset)
{
  //printf("\njcs --- msiTypedSelection::Extend\n");
  //DumpSelection(this);
  //printf("\nParent\n");
  //DumpNode(parentNode, 0, true);

  nsresult res(NS_OK);
  nsCOMPtr<nsIDOMNode> adjustNode(parentNode);
  PRInt32 adjustOffset(offset);
  PRBool preventDefault(PR_FALSE);
  if (m_adjustCaretCB && IsMouseEventActive() && m_msiEditor)
    res = m_adjustCaretCB(m_msiEditor, m_mouseEvent, adjustNode, adjustOffset);
  if (NS_SUCCEEDED(res) && m_setSelectionCB && m_msiEditor)
    res = m_setSelectionCB(m_msiEditor, adjustNode, adjustOffset, PR_TRUE, preventDefault);
  if (!preventDefault)
  {    
    res =  nsTypedSelection::Extend(adjustNode, adjustOffset);
    SyncMSIwithNS();
  }
  SetDOMEvent(nsnull); // only use a mouse event once
  //printf("\njcs --- leave extend\n");
  //DumpSelection(this);
  return res;
}

NS_IMETHODIMP 
msiTypedSelection::CollapseToStart(void)
{ 
  nsresult res =  nsTypedSelection::CollapseToStart();
  SyncMSIwithNS();
  return res;
}

NS_IMETHODIMP 
msiTypedSelection::CollapseToEnd(void)
{ 
  nsresult res =  nsTypedSelection::CollapseToEnd();
  SyncMSIwithNS();
  return res;
}

NS_IMETHODIMP 
msiTypedSelection::AddRange(nsIDOMRange *range)
{ 
  nsresult res =  nsTypedSelection::AddRange(range);
  SyncMSIwithNS();
  return res;
}

NS_IMETHODIMP 
msiTypedSelection::RemoveRange(nsIDOMRange *range)
{ 
  nsresult res =  nsTypedSelection::RemoveRange(range);
  SyncMSIwithNS();
  return res;
}

NS_IMETHODIMP 
msiTypedSelection::RemoveAllRanges(void)
{
  nsresult res =  nsTypedSelection::RemoveAllRanges();
  SyncMSIwithNS();
  return res;
}

// end overwrite of nsISelection

NS_IMETHODIMP
msiTypedSelection::SyncMSIwithNS()
{
  //m_msiFocusNode = FetchFocusNode();
  //m_msiAnchorNode = FetchAnchorNode();
  //m_msiFocusOffset = FetchFocusOffset();
  //m_msiAnchorOffset = FetchAnchorOffset();
  return NS_OK;
}

PRBool msiTypedSelection::IsVoidOfContent(nsIDOMNode* start, PRUint32 startOff, nsIDOMNode * end, PRUint32 endOff)
{
  PRBool rv(PR_FALSE);
  //return rv; //quick check to see if this is the problem BBM
  if (start == end && startOff == endOff)
    rv = PR_TRUE;
  else
  {
    nsCOMPtr<nsIDOMNode> commonAncestor;
    PRUint32 startLen(0);
    nsCOMPtr<nsIDOMCharacterData> textNode = do_QueryInterface(start);
    if (textNode)
      textNode->GetLength(&startLen);
    else
    {
      nsCOMPtr<nsIDOMNodeList> children;
      start->GetChildNodes(getter_AddRefs(children));
      if (children)
        children->GetLength(&startLen);
    }
    nsContentUtils::GetCommonAncestor(start, end, getter_AddRefs(commonAncestor));
    if (startOff == startLen || endOff == 0)
    {
      PRUint32 startRelativeOff(0), endRelativeOff(0);
      rv = PR_TRUE;
      if (startOff != startLen && start != commonAncestor)
        rv = PR_FALSE;
      if (rv && endOff != 0 && end != commonAncestor)
        rv = PR_FALSE;
      if (rv)
      {
        if (start == commonAncestor)
          startRelativeOff = startOff;  
        else 
        {  
          nsCOMPtr<nsIDOMNode> currTop, currNode;
          start->GetParentNode(getter_AddRefs(currTop));
          currNode = start;
          while (rv && currTop && currTop != commonAncestor && currNode)
          {
            nsCOMPtr<nsIDOMNode> lastKid;
            currTop->GetLastChild(getter_AddRefs(lastKid));
            if (lastKid == currNode)
            {
               nsCOMPtr<nsIDOMNode> newTop;
               currTop->GetParentNode(getter_AddRefs(newTop));
               currNode = currTop;
               currTop = newTop;
            }
            else
              rv = PR_FALSE;
          }
          if (rv && currTop == commonAncestor)
          {
            nsCOMPtr<nsIDOMNode> kid;
            currTop->GetFirstChild(getter_AddRefs(kid));
            while (kid && kid != currNode)
            {
              startRelativeOff += 1;
              nsCOMPtr<nsIDOMNode> nextKid;
              kid->GetNextSibling(getter_AddRefs(nextKid));
              kid = nextKid;
            }
          }
          else
            rv = PR_FALSE;
        }
      }
      if (rv)
      {
        if (end == commonAncestor)
          endRelativeOff = startOff;  
        else 
        {  
          nsCOMPtr<nsIDOMNode> currTop, currNode;
          end->GetParentNode(getter_AddRefs(currTop));
          currNode = end;
          while (rv && currTop && currTop != commonAncestor && currNode)
          {
            nsCOMPtr<nsIDOMNode> firstKid;
            currTop->GetFirstChild(getter_AddRefs(firstKid));
            if (firstKid == currNode)
            {
               nsCOMPtr<nsIDOMNode> newTop;
               currTop->GetParentNode(getter_AddRefs(newTop));
               currNode = currTop;
               currTop = newTop;
            }
            else
              rv = PR_FALSE;
          }
          if (rv && currTop == commonAncestor)
          {
            nsCOMPtr<nsIDOMNode> kid;
            currTop->GetFirstChild(getter_AddRefs(kid));
            while (kid && kid != currNode)
            {
              endRelativeOff += 1;
              nsCOMPtr<nsIDOMNode> nextKid;
              kid->GetNextSibling(getter_AddRefs(nextKid));
              kid = nextKid;
            }
          }
          else
            rv = PR_FALSE;
        }
      }
      if (rv)
      { 
        if (start == commonAncestor && startRelativeOff < endRelativeOff)
          rv = PR_FALSE;
        else if (start != commonAncestor && startRelativeOff+1 < endRelativeOff)
          rv = PR_FALSE;
      }    
    }
  }  
  return rv;
}

PRBool  msiTypedSelection::IsMouseEventActive()
{
  PRBool rv(PR_FALSE);
  if (m_mouseEvent)
  {
    nsCOMPtr<nsIPrivateDOMEvent> privateEvent(do_QueryInterface(m_mouseEvent));
    if (privateEvent)
    {
        nsEvent* innerEvent;
        privateEvent->GetInternalNSEvent(&innerEvent);
        if (innerEvent) 
          rv = !(innerEvent->flags & NS_EVENT_FLAG_STOP_DISPATCH_IMMEDIATELY);
    }
  }
  return rv;
}


NS_IMETHODIMP 
msiTypedSelection::Adjust()
{
  nsresult res = NS_OK;
  PRInt32 rangeCount;
  PRUint32 i;
  nsCOMPtr<nsIDOMRange> range;
  nsCOMPtr<nsIDOMRange> modrange;
  nsCOMPtr<nsIDOMNode>nodeContainerStart;
  nsCOMPtr<nsIDOMNode>nodeContainerEnd;
  PRInt32 offsetStart;
  PRInt32 offsetEnd;
  res = GetRangeCount(&rangeCount);
  rangeCount = 1;
  for (i = 0; i < rangeCount; i++)
  {
    GetRangeAt(i, getter_AddRefs(range));
    range->CloneRange(getter_AddRefs(modrange));
    res = ((msiIMathMLEditor *)m_msiEditor)->AdjustRange(modrange, PR_FALSE, 0);
    modrange->GetStartContainer(getter_AddRefs(nodeContainerStart));
    modrange->GetStartOffset(&offsetStart);
    modrange->GetEndContainer(getter_AddRefs(nodeContainerEnd));
    modrange->GetEndOffset(&offsetEnd);
    Collapse(nodeContainerStart, offsetStart);
    nsTypedSelection::Extend(nodeContainerEnd, offsetEnd);
  }
  return res;
//  return ((msiIMathMLEditor *)m_msiEditor)->AdjustSelection();
}