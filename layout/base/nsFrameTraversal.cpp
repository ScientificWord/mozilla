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
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
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
#include "nsCOMPtr.h"
#include "nsLayoutAtoms.h"

#include "nsFrameTraversal.h"
#include "nsFrameList.h"
#include "nsPlaceholderFrame.h"


class nsFrameIterator: public nsIBidirectionalEnumerator
{
public:
  NS_DECL_ISUPPORTS

  NS_IMETHOD First();

  NS_IMETHOD Last();
  
  NS_IMETHOD Next()=0;

  NS_IMETHOD Prev()=0;

  NS_IMETHOD CurrentItem(nsISupports **aItem);

  NS_IMETHOD IsDone();//what does this mean??off edge? yes

  nsFrameIterator();

protected:
  void      setCurrent(nsIFrame *aFrame){mCurrent = aFrame;}
  nsIFrame *getCurrent(){return mCurrent;}
  void      setStart(nsIFrame *aFrame){mStart = aFrame;}
  nsIFrame *getStart(){return mStart;}
  nsIFrame *getLast(){return mLast;}
  void      setLast(nsIFrame *aFrame){mLast = aFrame;}
  PRInt8    getOffEdge(){return mOffEdge;}
  void      setOffEdge(PRInt8 aOffEdge){mOffEdge = aOffEdge;}
private:
  nsIFrame *mStart;
  nsIFrame *mCurrent;
  nsIFrame *mLast; //the last one that was in current;
  PRInt8    mOffEdge; //0= no -1 to far prev, 1 to far next;
};


/*
class nsFastFrameIterator: public nsFrameIterator
{
  nsFastFrameIterator(nsIFrame *start);
private :
  
  virtual nsresult Next();

  virtual nsresult Prev();

}
*/

class nsLeafIterator: public nsFrameIterator
{
public:
  nsLeafIterator(nsPresContext* aPresContext, nsIFrame *start);
  void SetExtensive(PRBool aExtensive) {mExtensive = aExtensive;}
  PRBool GetExtensive(){return mExtensive;}
  void   SetLockInScrollView(PRBool aLockScroll){mLockScroll = aLockScroll;}
private :
  
  NS_IMETHOD Next();

  NS_IMETHOD Prev();

  nsPresContext* mPresContext;
  PRPackedBool mExtensive;
  PRBool mLockScroll;
};

class nsFocusIterator : public nsFrameIterator
{
public:
  nsFocusIterator(nsPresContext* aPresContext, nsIFrame* aStart);
private:
  NS_IMETHOD Next();
  NS_IMETHOD Prev();
  NS_IMETHOD Last();

  /*
    Our own versions of the standard frame tree navigation
    methods, which apply the following rules for placeholder
    frames:

     - If a frame HAS a placeholder frame, getting its parent
       gets the placeholder's parent.

     - If a frame's first child or next/prev sibling IS a
       placeholder frame, then we instead return the real frame.

     - If a frame HAS a placeholder frame, getting its next/prev
       sibling gets the placeholder frame's next/prev sibling.

    These are all applied recursively to support multiple levels of
    placeholders.
  */

  nsIFrame* GetParentFrame(nsIFrame* aFrame);
  nsIFrame* GetFirstChild(nsIFrame* aFrame);
  nsIFrame* GetNextSibling(nsIFrame* aFrame);
  nsIFrame* GetPrevSibling(nsIFrame* aFrame);

  nsIFrame* GetPlaceholderFrame(nsIFrame* aFrame);
  PRBool    IsPopupFrame(nsIFrame* aFrame);

  nsPresContext* mPresContext;
};

#ifdef IBMBIDI // Simon

class nsVisualIterator: public nsFrameIterator
{
  public:
    nsVisualIterator(nsPresContext* aPresContext, nsIFrame *start);
    void   SetLockInScrollView(PRBool aLockScroll){mLockScroll = aLockScroll;}
  private :

    NS_IMETHOD Next();

    NS_IMETHOD Prev();

    nsPresContext* mPresContext;
    PRBool mLockScroll;
};

#endif
/************IMPLEMENTATIONS**************/

nsresult NS_CreateFrameTraversal(nsIFrameTraversal** aResult)
{
  NS_ENSURE_ARG_POINTER(aResult);
  *aResult = nsnull;

  nsCOMPtr<nsIFrameTraversal> t(new nsFrameTraversal());
  if (!t)
    return NS_ERROR_OUT_OF_MEMORY;

  *aResult = t;
  NS_ADDREF(*aResult);

  return NS_OK;
}

nsresult
NS_NewFrameTraversal(nsIBidirectionalEnumerator **aEnumerator,
                     nsTraversalType aType,
                     nsPresContext* aPresContext,
                     nsIFrame *aStart,
                     PRBool aLockInScrollView)
{
  if (!aEnumerator || !aStart)
    return NS_ERROR_NULL_POINTER;
  switch(aType)
  {
  case LEAF: {
    nsLeafIterator *trav = new nsLeafIterator(aPresContext, aStart);
    if (!trav)
      return NS_ERROR_OUT_OF_MEMORY;
    trav->SetLockInScrollView(aLockInScrollView);
    *aEnumerator = NS_STATIC_CAST(nsIBidirectionalEnumerator*, trav);
    NS_ADDREF(trav);
    trav->SetExtensive(PR_FALSE);
  }
  break;
  case EXTENSIVE:{
    nsLeafIterator *trav = new nsLeafIterator(aPresContext, aStart);
    if (!trav)
      return NS_ERROR_OUT_OF_MEMORY;
    *aEnumerator = NS_STATIC_CAST(nsIBidirectionalEnumerator*, trav);
    NS_ADDREF(trav);
    trav->SetExtensive(PR_TRUE);
  }
  break;
  case FOCUS: {
    nsFocusIterator *trav = new nsFocusIterator(aPresContext, aStart);
    if (!trav)
      return NS_ERROR_OUT_OF_MEMORY;
    *aEnumerator = NS_STATIC_CAST(nsIBidirectionalEnumerator*, trav);
    NS_ADDREF(trav);
  }
  break;
#ifdef IBMBIDI
  case VISUAL:{
    nsVisualIterator *trav = new nsVisualIterator(aPresContext, aStart);
    if (!trav)
      return NS_ERROR_OUT_OF_MEMORY;
    trav->SetLockInScrollView(aLockInScrollView);
    *aEnumerator = NS_STATIC_CAST(nsIBidirectionalEnumerator*, trav);
    NS_ADDREF(trav);
              }
    break;
#endif
#if 0
  case FASTEST:{
    nsFastestTraversal *trav = new nsFastestTraversal(aStart);
    if (!trav)
      return NS_ERROR_NOMEMORY;
    *aEnumerator = NS_STATIC_CAST(nsIBidirectionalEnumerator*, trav);
    NS_ADDREF(trav);
               }
#endif
  default:
    return NS_ERROR_NOT_IMPLEMENTED;
    break;
  }
  return NS_OK;
}


nsFrameTraversal::nsFrameTraversal()
{
}

nsFrameTraversal::~nsFrameTraversal()
{
}

NS_IMPL_ISUPPORTS1(nsFrameTraversal,nsIFrameTraversal)

NS_IMETHODIMP 
nsFrameTraversal::NewFrameTraversal(nsIBidirectionalEnumerator **aEnumerator,
                              PRUint32 aType,
                              nsPresContext* aPresContext,
                              nsIFrame *aStart)
{
  return NS_NewFrameTraversal(aEnumerator, NS_STATIC_CAST(nsTraversalType,
                                                          aType),
                              aPresContext, aStart,PR_FALSE);  
}

/*********nsFrameIterator************/
NS_IMPL_ISUPPORTS2(nsFrameIterator, nsIEnumerator, nsIBidirectionalEnumerator)

nsFrameIterator::nsFrameIterator()
{
  mOffEdge = 0;
  mLast = nsnull;
  mCurrent = nsnull;
  mStart = nsnull;
}



NS_IMETHODIMP
nsFrameIterator::CurrentItem(nsISupports **aItem)
{
  if (!aItem)
    return NS_ERROR_NULL_POINTER;
  *aItem = mCurrent;
  if (mOffEdge)
    return NS_ENUMERATOR_FALSE;
  return NS_OK;
}



NS_IMETHODIMP
nsFrameIterator::IsDone()//what does this mean??off edge? yes
{
  if (mOffEdge != 0)
    return NS_OK;
  return NS_ENUMERATOR_FALSE;
}



NS_IMETHODIMP
nsFrameIterator::First()
{
  mCurrent = mStart;
  return NS_OK;
}



NS_IMETHODIMP
nsFrameIterator::Last()
{
  return NS_ERROR_FAILURE;
}



/*********LEAFITERATOR**********/


nsLeafIterator::nsLeafIterator(nsPresContext* aPresContext, nsIFrame *aStart)
  : mPresContext(aPresContext)
{
  setStart(aStart);
  setCurrent(aStart);
  setLast(aStart);
  SetLockInScrollView(PR_FALSE);
}

static PRBool
IsRootFrame(nsIFrame* aFrame)
{
  nsIAtom* atom = aFrame->GetType();
  return (atom == nsLayoutAtoms::canvasFrame) ||
         (atom == nsLayoutAtoms::rootFrame);
}

NS_IMETHODIMP
nsLeafIterator::Next()
{
//recursive-oid method to get next frame
  nsIFrame *result = nsnull;
  nsIFrame *parent = getCurrent();
  if (!parent)
    parent = getLast();
  if (!mExtensive)
  {
    while (nsnull != (result = parent->GetFirstChild(nsnull)))
    {
      parent = result;
    }
  }
  if (parent != getCurrent())
  {
    result = parent;
  }
  else {
    while(parent) {
      result = parent->GetNextSibling();
      if (result) {
        parent = result;
        while (nsnull != (result = parent->GetFirstChild(nsnull)))
          {
            parent = result;
          }
        result = parent;
        break;
      }
      else
      {
        result = parent->GetParent();
        if (!result || IsRootFrame(result)) {
          result = nsnull;
          break;
        }
        else 
        {
          parent = result;
          // check if  FrameType of result is TextInputFrame
          if (mLockScroll) //lock the traversal when we hit a scroll frame
          {
            if ( result->GetType() == nsLayoutAtoms::scrollFrame )
              return NS_ERROR_FAILURE;
          }
          if (mExtensive)
            break;
        }  
      }
    }
  }
  setCurrent(result);
  if (!result)
    setOffEdge(1);
  return NS_OK;
}




NS_IMETHODIMP
nsLeafIterator::Prev()
{
//recursive-oid method to get prev frame
  nsIFrame *result = nsnull;
  nsIFrame *parent = getCurrent();
  if (!parent)
    parent = getLast();

  while (parent){
    nsIFrame *grandParent = parent->GetParent();
    if (grandParent) 
    {
      // check if  FrameType of grandParent is TextInputFrame
      if (mLockScroll) //lock the traversal when we hit a scroll frame
      {
        nsIAtom* atom = grandParent->GetType();
  #ifdef DEBUG_skamio
        if ( atom ) 
        {
          nsAutoString aString;
          res = atom->ToString(aString);
          if ( NS_SUCCEEDED(res) ) {
            printf("%s:%d\n", __FILE__, __LINE__);
            printf("FrameType: %s\n", NS_ConvertUTF16toUTF8(aString).get());
          }
        }
  #endif
        if ( atom == nsLayoutAtoms::scrollFrame ) 
          return NS_ERROR_FAILURE;
      }
      result = grandParent->GetFirstChild(nsnull);
      nsFrameList list(result);
      result = list.GetPrevSiblingFor(parent);
      if (result)
      {
        parent = result;
        while (nsnull != (result = parent->GetFirstChild(nsnull)))
        {
          parent = result;
          while ((result = parent->GetNextSibling()) != nsnull)
          {
            parent = result;
          }
        }
        result = parent;
        break;
      }
      else if (!(result = parent->GetParent()))
      {
        result = nsnull;
        break;
      }
      else 
      {
        parent = result;
        if (mExtensive)
          break;
      }
    }
    else
    {
      setLast(parent);
      result = nsnull;
      break;
    }
  }

  setCurrent(result);
  if (!result)
    setOffEdge(-1);
  return NS_OK;
}

nsFocusIterator::nsFocusIterator(nsPresContext* aPresContext, nsIFrame* aStart)
  : mPresContext(aPresContext)
{
  nsIFrame* start = aStart;
  if (aStart)
    start = nsPlaceholderFrame::GetRealFrameFor(aStart);

  setStart(start);
  setCurrent(start);
  setLast(start);
}

nsIFrame*
nsFocusIterator::GetPlaceholderFrame(nsIFrame* aFrame)
{
  nsIFrame* result = aFrame;
  nsIPresShell *presShell = mPresContext->GetPresShell();
  if (presShell) {
    nsIFrame* placeholder = 0;
    presShell->GetPlaceholderFrameFor(aFrame, &placeholder);
    if (placeholder)
      result = placeholder;
  }

  if (result != aFrame)
    result = GetPlaceholderFrame(result);

  return result;
}

PRBool
nsFocusIterator::IsPopupFrame(nsIFrame* aFrame)
{
  return (aFrame->GetStyleDisplay()->mDisplay == NS_STYLE_DISPLAY_POPUP);
}

nsIFrame*
nsFocusIterator::GetParentFrame(nsIFrame* aFrame)
{
  nsIFrame* placeholder = GetPlaceholderFrame(aFrame);
  if (placeholder)
    return placeholder->GetParent();

  return nsnull;
}

nsIFrame*
nsFocusIterator::GetFirstChild(nsIFrame* aFrame)
{
  nsIFrame* result = aFrame->GetFirstChild(nsnull);
  if (result)
    result = nsPlaceholderFrame::GetRealFrameFor(result);

  if (result && IsPopupFrame(result))
    result = GetNextSibling(result);

  return result;
}

nsIFrame*
nsFocusIterator::GetNextSibling(nsIFrame* aFrame)
{
  nsIFrame* result = nsnull;
  nsIFrame* placeholder = GetPlaceholderFrame(aFrame);
  if (placeholder) {
    result = placeholder->GetNextSibling();
    if (result)
      result = nsPlaceholderFrame::GetRealFrameFor(result);
  }

  if (result && IsPopupFrame(result))
    result = GetNextSibling(result);

  return result;
}

nsIFrame*
nsFocusIterator::GetPrevSibling(nsIFrame* aFrame)
{
  nsIFrame* result = 0;
  nsIFrame* placeholder = GetPlaceholderFrame(aFrame);
  if (placeholder) {
    nsIFrame* parent = GetParentFrame(placeholder);
    if (parent) {
      nsFrameList list(parent->GetFirstChild(nsnull));
      result = list.GetPrevSiblingFor(placeholder);
      if (result)
        result = nsPlaceholderFrame::GetRealFrameFor(result);
    }
  }

  if (result && IsPopupFrame(result))
    result = GetPrevSibling(result);

  return result;
}

NS_IMETHODIMP
nsFocusIterator::Next()
{
  nsIFrame* result = 0;
  nsIFrame* parent = getCurrent();
  if (!parent)
    parent = getLast();

  if ((result = GetFirstChild(parent)))
    parent = result;

  result = parent;
  if (result == getCurrent()) {
    while (result) {
      if ((parent = GetNextSibling(result))) {
        result = parent;
        break;
       } else {
        parent = result;
        result = GetParentFrame(parent);
        if (!result || IsRootFrame(result)) {
          result = nsnull;
          break;
        }
      }
    }

    if (!result) {
      setLast(parent);
    }
  }

  setCurrent(result);
  if (!result)
    setOffEdge(1);
  return NS_OK;
}

NS_IMETHODIMP
nsFocusIterator::Prev()
{
  nsIFrame *result = nsnull;
  nsIFrame *parent = getCurrent();
  if (!parent)
    parent = getLast();
  if (parent) {
    if ((result = GetPrevSibling(parent))) {
      parent = result;
      while ((result = GetFirstChild(parent))) {
        parent = result;
        while ((result = GetNextSibling(parent)))
          parent = result;
      }
      result = parent;
    } else if (!(result = GetParentFrame(parent))) {
      result = 0;
      setLast(parent);
    }
  }

  setCurrent(result);
  if (!result)
    setOffEdge(-1);
  return NS_OK;
}

NS_IMETHODIMP
nsFocusIterator::Last()
{
  nsIFrame* result;
  nsIFrame* parent = getCurrent();
  while (!IsRootFrame(parent) && (result = GetParentFrame(parent)))
    parent = result;

  while ((result = GetFirstChild(parent))) {
    parent = result;
    while ((result = GetNextSibling(parent)))
      parent = result;
  }

  setCurrent(parent);
  if (!parent)
    setOffEdge(1);
  return NS_OK;
}

#ifdef IBMBIDI

/*********VISUALITERATOR**********/

nsVisualIterator::nsVisualIterator(nsPresContext* aPresContext, nsIFrame *aStart)
: mPresContext(aPresContext)
{
  setStart(aStart);
  setCurrent(aStart);
  setLast(aStart);
  SetLockInScrollView(PR_FALSE);
}

NS_IMETHODIMP
   nsVisualIterator::Next()
{
  //recursive-oid method to get next frame
  nsIFrame *result = nsnull;
  nsIFrame *parent = getCurrent();
  if (!parent)
    parent = getLast();
  while (nsnull != (result = parent->GetFirstChild(nsnull)))
  {
    parent = result;
  }
  if (parent != getCurrent())
  {
    result = parent;
  }
  else {
    while(parent) {
      nsIFrame *grandParent = parent->GetParent();
      if (grandParent) {
        nsFrameList list(grandParent->GetFirstChild(nsnull));
        result = list.GetNextVisualFor(parent);
        if (result) {
          while (result) {
            parent = result;
            nsFrameList list(parent->GetFirstChild(nsnull));
            result = list.GetNextVisualFor(nsnull);
          }
          result = parent;
          break;
        }
        else if (!(result = parent->GetParent()) || IsRootFrame(result)) {
          result = nsnull;
          break;
        }
        else 
        {
          parent = result;
          if (mLockScroll) //lock the traversal when we hit a scroll frame
          {
            if ( result->GetType() == nsLayoutAtoms::scrollFrame )
              return NS_ERROR_FAILURE;
          }
        }
      } 
      else{
        setLast(parent);
        result = nsnull;
        break;
      }
    }
  }

  setCurrent(result);
  if (!result)
    setOffEdge(-1);
  return NS_OK;
}

NS_IMETHODIMP
   nsVisualIterator::Prev()
{
  //recursive-oid method to get prev frame
  nsIFrame *result = nsnull;
  nsIFrame *parent = getCurrent();
  if (!parent)
    parent = getLast();
  while(parent){
    nsIFrame *grandParent = parent->GetParent();
    if (grandParent) {
      if (mLockScroll) //lock the traversal when we hit a scroll frame
      {
        if (grandParent->GetType() == nsLayoutAtoms::scrollFrame)
          return NS_ERROR_FAILURE;
      }
      nsFrameList list(grandParent->GetFirstChild(nsnull));
      result = list.GetPrevVisualFor(parent);
      if (result){
        while (result) {
          parent = result;
          nsFrameList list(parent->GetFirstChild(nsnull));
          result = list.GetPrevVisualFor(nsnull);
        }
        result = parent;
        break;
      }
      else if (!(result = parent->GetParent())) {
        break;
      }
      else 
      {
        parent = result;
      }
    }
    else{
      setLast(parent);
      result = nsnull;
      break;
    }
  }

  setCurrent(result);
  if (!result)
    setOffEdge(-1);
  return NS_OK;
}
#endif
