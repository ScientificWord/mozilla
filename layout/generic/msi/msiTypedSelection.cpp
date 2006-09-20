#include "msiISelection.h"

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

protected:
  void SyncMSIwithNS();
  PRBool IsVoidOfContent(nsIDOMNode * start, PRUint32 startOff, nsIDOMNode* end, PRUint32 endOff);
  
protected:
  nsCOMPtr<nsIDOMNode>  m_msiFocusNode;
  nsCOMPtr<nsIDOMNode>  m_msiAnchorNode;
  PRUint32              m_msiFocusOffset;
  PRUint32              m_msiAnchorOffset;
};


msiTypedSelection::msiTypedSelection() 
: nsTypedSelection(), m_msiFocusOffset(INVALID_OFFSET),
m_msiAnchorOffset(INVALID_OFFSET)
{
}

msiTypedSelection::msiTypedSelection(nsFrameSelection* aList)
: nsTypedSelection(aList),
m_msiFocusOffset(INVALID_OFFSET), m_msiAnchorOffset(INVALID_OFFSET)
{
}

msiTypedSelection::~msiTypedSelection()
{
}

// ISupports
NS_IMPL_ISUPPORTS_INHERITED1(msiTypedSelection, nsTypedSelection, msiISelection)


// msiISelection interface
NS_IMETHODIMP 
msiTypedSelection::GetMsiAnchorNode(nsIDOMNode * *msiAnchorNode)
{
  if (!msiAnchorNode)
    return NS_ERROR_NULL_POINTER;
  *msiAnchorNode = nsnull;
  if (m_msiAnchorNode)
  {
    *msiAnchorNode = m_msiAnchorNode;
    NS_ADDREF(*msiAnchorNode);
  }
  return NS_OK;  
}

NS_IMETHODIMP 
msiTypedSelection::GetMsiAnchorOffset(PRUint32 *msiAnchorOffset)
{
  if (!msiAnchorOffset)
    return NS_ERROR_NULL_POINTER;
  *msiAnchorOffset = m_msiAnchorOffset;
  return NS_OK;  
}

NS_IMETHODIMP 
msiTypedSelection::GetMsiFocusNode(nsIDOMNode * *msiFocusNode)
{
  if (!msiFocusNode)
    return NS_ERROR_NULL_POINTER;
  *msiFocusNode = nsnull;
  if (m_msiFocusNode)
  {
    *msiFocusNode = m_msiFocusNode;
    NS_ADDREF(*msiFocusNode);
  }
  return NS_OK;  
}

NS_IMETHODIMP 
msiTypedSelection::GetMsiFocusOffset(PRUint32 *msiFocusOffset)
{
  if (!msiFocusOffset)
    return NS_ERROR_NULL_POINTER;
  *msiFocusOffset = m_msiFocusOffset;
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
  PRBool focusBeforeAnchor = nsRange::ComparePoints(focusNode, focusOffset, anchorNode, anchorOffset) < 0;
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
      res =  nsTypedSelection::Extend(newNSAnchor, newNSAnchorOffset);
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
      m_msiFocusNode = focusNode;
      m_msiAnchorNode = anchorNode;
      m_msiFocusOffset = focusOffset;
      m_msiAnchorOffset = anchorOffset;
    }
    else
      SyncMSIwithNS();
  }
  return res;  
}

NS_IMETHODIMP
msiTypedSelection::GetMouseDown(PRBool * isdown)
{
  if (!isdown || !mFrameSelection)
    return NS_ERROR_FAILURE;
  *isdown = mFrameSelection->GetMouseDownState();
  return NS_OK;
}

NS_IMETHODIMP
msiTypedSelection::SetMouseDown(PRBool isdown)
{
  if (!isdown || !mFrameSelection)
    return NS_ERROR_FAILURE;
  mFrameSelection->SetMouseDownState(isdown);
  return NS_OK;
}



// end msiISelection interface

// overwrite of nsISelection
NS_IMETHODIMP 
msiTypedSelection::Collapse(nsIDOMNode *parentNode, PRInt32 offset)
{ 
  nsresult res =  nsTypedSelection::Collapse(parentNode, offset);
  SyncMSIwithNS();
  return res;
}
NS_IMETHODIMP 
msiTypedSelection::Extend(nsIDOMNode *parentNode, PRInt32 offset)
{ 
  nsresult res =  nsTypedSelection::Extend(parentNode, offset);
  SyncMSIwithNS();
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

void msiTypedSelection::SyncMSIwithNS()
{
  m_msiFocusNode = FetchFocusNode();
  m_msiAnchorNode = FetchAnchorNode();
  m_msiFocusOffset = FetchFocusOffset();
  m_msiAnchorOffset = FetchAnchorOffset();
}

PRBool msiTypedSelection::IsVoidOfContent(nsIDOMNode* start, PRUint32 startOff, nsIDOMNode * end, PRUint32 endOff)
{
  PRBool rv(PR_FALSE);
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
