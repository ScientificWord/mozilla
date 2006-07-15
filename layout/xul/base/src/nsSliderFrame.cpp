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
 * The Original Code is Mozilla Communicator client code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Dean Tessman <dean_tessman@hotmail.com>
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
// Eric Vaughan
// Netscape Communications
//
// See documentation in associated header file
//

#include "nsSliderFrame.h"
#include "nsStyleContext.h"
#include "nsPresContext.h"
#include "nsIContent.h"
#include "nsCOMPtr.h"
#include "nsUnitConversion.h"
#include "nsINameSpaceManager.h"
#include "nsXULAtoms.h"
#include "nsHTMLAtoms.h"
#include "nsHTMLParts.h"
#include "nsIPresShell.h"
#include "nsStyleChangeList.h"
#include "nsCSSRendering.h"
#include "nsHTMLAtoms.h"
#include "nsIDOMEventReceiver.h"
#include "nsIViewManager.h"
#include "nsIWidget.h"
#include "nsIDOMMouseEvent.h"
#include "nsIDocument.h"
#include "nsScrollbarButtonFrame.h"
#include "nsIScrollbarListener.h"
#include "nsIScrollbarMediator.h"
#include "nsIScrollbarFrame.h"
#include "nsISupportsArray.h"
#include "nsXULAtoms.h"
#include "nsHTMLAtoms.h"
#include "nsIScrollableView.h"
#include "nsRepeatService.h"
#include "nsBoxLayoutState.h"
#include "nsSprocketLayout.h"
#include "nsIServiceManager.h"
#include "nsGUIEvent.h"
#include "nsContentUtils.h"
#include "nsLayoutUtils.h"
#include "nsDisplayList.h"

PRBool nsSliderFrame::gMiddlePref = PR_FALSE;
PRInt32 nsSliderFrame::gSnapMultiplier = 6;

// Turn this on if you want to debug slider frames.
#undef DEBUG_SLIDER

static already_AddRefed<nsIContent>
GetContentOfBox(nsIBox *aBox)
{
  nsIContent* content = aBox->GetContent();
  NS_IF_ADDREF(content);
  return content;
}

nsIFrame*
NS_NewSliderFrame (nsIPresShell* aPresShell, nsStyleContext* aContext)
{
  return new (aPresShell) nsSliderFrame(aPresShell, aContext);
} // NS_NewSliderFrame

nsSliderFrame::nsSliderFrame(nsIPresShell* aPresShell, nsStyleContext* aContext):
  nsBoxFrame(aPresShell, aContext),
  mCurPos(0),
  mScrollbarListener(nsnull),
  mChange(0),
  mMediator(nsnull)
{
}

// stop timer
nsSliderFrame::~nsSliderFrame()
{
   mRedrawImmediate = PR_FALSE;
}

NS_IMETHODIMP
nsSliderFrame::Init(nsIContent*      aContent,
                    nsIFrame*        aParent,
                    nsIFrame*        aPrevInFlow)
{
  nsresult rv = nsBoxFrame::Init(aContent, aParent, aPrevInFlow);

  static PRBool gotPrefs = PR_FALSE;
  if (!gotPrefs) {
    gotPrefs = PR_TRUE;

    gMiddlePref = nsContentUtils::GetBoolPref("middlemouse.scrollbarPosition");
    gSnapMultiplier = nsContentUtils::GetIntPref("slider.snapMultiplier");
  }

  CreateViewForFrame(GetPresContext(), this, GetStyleContext(), PR_TRUE);
  nsIView* view = GetView();
  view->GetViewManager()->SetViewContentTransparency(view, PR_TRUE);
  return rv;
}

NS_IMETHODIMP
nsSliderFrame::RemoveFrame(nsIAtom*        aListName,
                           nsIFrame*       aOldFrame)
{
  nsresult rv = nsBoxFrame::RemoveFrame(aListName, aOldFrame);
  PRInt32 start = GetChildCount();
  if (start == 0)
    RemoveListener();

  return rv;
}

NS_IMETHODIMP
nsSliderFrame::InsertFrames(nsIAtom*        aListName,
                            nsIFrame*       aPrevFrame,
                            nsIFrame*       aFrameList)
{
  PRInt32 start = GetChildCount();
  nsresult rv = nsBoxFrame::InsertFrames(aListName, aPrevFrame, aFrameList);
  if (start == 0)
    AddListener();

  return rv;
}

NS_IMETHODIMP
nsSliderFrame::AppendFrames(nsIAtom*        aListName,
                            nsIFrame*       aFrameList)
{
  // if we have no children and on was added then make sure we add the
  // listener
  PRInt32 start = GetChildCount();
  nsresult rv = nsBoxFrame::AppendFrames(aListName, aFrameList);
  if (start == 0)
    AddListener();

  return rv;
}

PRInt32
nsSliderFrame::GetCurrentPosition(nsIContent* content)
{
  return GetIntegerAttribute(content, nsXULAtoms::curpos, 0);
}

PRInt32
nsSliderFrame::GetMinPosition(nsIContent* content)
{
  return GetIntegerAttribute(content, nsXULAtoms::minpos, 0);
}

PRInt32
nsSliderFrame::GetMaxPosition(nsIContent* content)
{
  return GetIntegerAttribute(content, nsXULAtoms::maxpos, 100);
}

PRInt32
nsSliderFrame::GetIncrement(nsIContent* content)
{
  return GetIntegerAttribute(content, nsXULAtoms::increment, 1);
}


PRInt32
nsSliderFrame::GetPageIncrement(nsIContent* content)
{
  return GetIntegerAttribute(content, nsXULAtoms::pageincrement, 10);
}

PRInt32
nsSliderFrame::GetIntegerAttribute(nsIContent* content, nsIAtom* atom, PRInt32 defaultValue)
{
    nsAutoString value;
    content->GetAttr(kNameSpaceID_None, atom, value);
    if (!value.IsEmpty()) {
      PRInt32 error;

      // convert it to an integer
      defaultValue = value.ToInteger(&error);
    }

    return defaultValue;
}


NS_IMETHODIMP
nsSliderFrame::AttributeChanged(PRInt32 aNameSpaceID,
                                nsIAtom* aAttribute,
                                PRInt32 aModType)
{
  nsresult rv = nsBoxFrame::AttributeChanged(aNameSpaceID, aAttribute,
                                             aModType);
  // if the current position changes
  if (aAttribute == nsXULAtoms::curpos) {
     rv = CurrentPositionChanged(GetPresContext());
     NS_ASSERTION(NS_SUCCEEDED(rv), "failed to change position");
     if (NS_FAILED(rv))
        return rv;
  } else if (aAttribute == nsXULAtoms::minpos ||
             aAttribute == nsXULAtoms::maxpos) {
      // bounds check it.

      nsIBox* scrollbarBox = GetScrollbar();
      nsCOMPtr<nsIContent> scrollbar;
      scrollbar = GetContentOfBox(scrollbarBox);
      PRInt32 current = GetCurrentPosition(scrollbar);
      PRInt32 min = GetMinPosition(scrollbar);
      PRInt32 max = GetMaxPosition(scrollbar);
      if (current < min || current > max)
      {
        if (current < min || max < min)
            current = min;
        else if (current > max)
            current = max;

        // set the new position and notify observers
        nsCOMPtr<nsIScrollbarFrame> scrollbarFrame(do_QueryInterface(scrollbarBox));
        if (scrollbarFrame) {
          nsCOMPtr<nsIScrollbarMediator> mediator;
          scrollbarFrame->GetScrollbarMediator(getter_AddRefs(mediator));
          if (mediator) {
            mediator->PositionChanged(scrollbarFrame, GetCurrentPosition(scrollbar), current);
          }
        }

        nsAutoString currentStr;
        currentStr.AppendInt(current);
        scrollbar->SetAttr(kNameSpaceID_None, nsXULAtoms::curpos, currentStr, PR_TRUE);
      }
  }

  if (aAttribute == nsXULAtoms::minpos ||
      aAttribute == nsXULAtoms::maxpos ||
      aAttribute == nsXULAtoms::pageincrement ||
      aAttribute == nsXULAtoms::increment) {

      nsBoxLayoutState state(GetPresContext());
      MarkDirtyChildren(state);
  }

  return rv;
}

NS_IMETHODIMP
nsSliderFrame::BuildDisplayList(nsDisplayListBuilder*   aBuilder,
                                const nsRect&           aDirtyRect,
                                const nsDisplayListSet& aLists)
{
  if (aBuilder->IsForEventDelivery() && isDraggingThumb()) {
    // This is EVIL, we shouldn't be messing with event delivery just to get
    // thumb mouse drag events to arrive at the slider!
    return aLists.Outlines()->AppendNewToTop(new (aBuilder)
        nsDisplayEventReceiver(this));
  }
  
  return nsBoxFrame::BuildDisplayList(aBuilder, aDirtyRect, aLists);
}

NS_IMETHODIMP
nsSliderFrame::BuildDisplayListForChildren(nsDisplayListBuilder*   aBuilder,
                                           const nsRect&           aDirtyRect,
                                           const nsDisplayListSet& aLists)
{
  // if we are too small to have a thumb don't paint it.
  nsIBox* thumb;
  GetChildBox(&thumb);

  if (thumb) {
    nsRect thumbRect(thumb->GetRect());
    nsMargin m;
    thumb->GetMargin(m);
    thumbRect.Inflate(m);

    nsRect crect;
    GetClientRect(crect);

    if (crect.width < thumbRect.width || crect.height < thumbRect.height)
      return NS_OK;
  }
  
  return nsBoxFrame::BuildDisplayListForChildren(aBuilder, aDirtyRect, aLists);
}

NS_IMETHODIMP
nsSliderFrame::DoLayout(nsBoxLayoutState& aState)
{
  // get the thumb should be our only child
  nsIBox* thumbBox = nsnull;
  GetChildBox(&thumbBox);

  if (!thumbBox) {
    SyncLayout(aState);
    return NS_OK;
  }

  EnsureOrient();

#ifdef DEBUG_LAYOUT
  if (mState & NS_STATE_DEBUG_WAS_SET) {
      if (mState & NS_STATE_SET_TO_DEBUG)
          SetDebug(aState, PR_TRUE);
      else
          SetDebug(aState, PR_FALSE);
  }
#endif

  // get the content area inside our borders
  nsRect clientRect(0,0,0,0);
  GetClientRect(clientRect);

  // get the scrollbar
  nsIBox* scrollbarBox = GetScrollbar();
  nsCOMPtr<nsIContent> scrollbar;
  scrollbar = GetContentOfBox(scrollbarBox);
  PRBool isHorizontal = IsHorizontal();

  // get the thumb's pref size
  nsSize thumbSize(0,0);
  thumbBox->GetPrefSize(aState, thumbSize);

  if (isHorizontal)
    thumbSize.height = clientRect.height;
  else
    thumbSize.width = clientRect.width;

  // get our current position and max position from our content node
  PRInt32 curpospx = GetCurrentPosition(scrollbar);
  PRInt32 minpospx = GetMinPosition(scrollbar);
  PRInt32 maxpospx = GetMaxPosition(scrollbar);
  PRInt32 pageIncrement = GetPageIncrement(scrollbar);

  if (maxpospx < minpospx)
    maxpospx = minpospx;

  if (curpospx < minpospx)
     curpospx = minpospx;
  else if (curpospx > maxpospx)
     curpospx = maxpospx;

  nscoord onePixel = aState.PresContext()->IntScaledPixelsToTwips(1);

  // get max pos in twips
  nscoord maxpos = (maxpospx - minpospx) * onePixel;

  // get our maxpos in twips. This is the space we have left over in the scrollbar
  // after the height of the thumb has been removed
  nscoord& desiredcoord = isHorizontal ? clientRect.width : clientRect.height;
  nscoord& thumbcoord = isHorizontal ? thumbSize.width : thumbSize.height;

  nscoord ourmaxpos = desiredcoord;

  mRatio = 1;

  if ((pageIncrement + maxpospx - minpospx) > 0)
  {
    // if the thumb is flexible make the thumb bigger.
    nscoord flex = 0;
    thumbBox->GetFlex(aState, flex);

    if (flex > 0)
    {
      mRatio = float(pageIncrement) / float(maxpospx - minpospx + pageIncrement);
      nscoord thumbsize = NSToCoordRound(ourmaxpos * mRatio);

      // if there is more room than the thumb needs stretch the thumb
      if (thumbsize > thumbcoord)
        thumbcoord = thumbsize;
    }
  }

  ourmaxpos -= thumbcoord;
  if (float(maxpos) != 0)
    mRatio = float(ourmaxpos) / float(maxpos);

  nscoord curpos = (curpospx - minpospx) * onePixel;

  // set the thumbs y coord to be the current pos * the ratio.
  nscoord pos = nscoord(float(curpos)*mRatio);
  nsRect thumbRect(clientRect.x, clientRect.y, thumbSize.width, thumbSize.height);

  if (isHorizontal)
    thumbRect.x += pos;
  else
    thumbRect.y += pos;

  nsRect oldThumbRect(thumbBox->GetRect());
  LayoutChildAt(aState, thumbBox, thumbRect);


  SyncLayout(aState);

#ifdef DEBUG_SLIDER
  PRInt32 c = GetCurrentPosition(scrollbar);
  PRInt32 min = GetMinPosition(scrollbar);
  PRInt32 max = GetMaxPosition(scrollbar);
  printf("Current=%d, min=%d, max=%d\n", c, min, max);
#endif

  // redraw only if thumb changed size.
  if (oldThumbRect != thumbRect)
    Redraw(aState);

  return NS_OK;
}


NS_IMETHODIMP
nsSliderFrame::HandleEvent(nsPresContext* aPresContext,
                                      nsGUIEvent* aEvent,
                                      nsEventStatus* aEventStatus)
{
  nsIBox* scrollbarBox = GetScrollbar();
  nsCOMPtr<nsIContent> scrollbar;
  scrollbar = GetContentOfBox(scrollbarBox);
  PRBool isHorizontal = IsHorizontal();

  if (isDraggingThumb())
  {
      // we want to draw immediately if the user doing it directly with the
      // mouse that makes redrawing much faster.
      mRedrawImmediate = PR_TRUE;

    switch (aEvent->message) {
    case NS_MOUSE_MOVE: {
      nsPoint eventPoint = nsLayoutUtils::GetEventCoordinatesRelativeTo(aEvent,
                                                                         this);
      if (mChange) {
        // We're in the process of moving the thumb to the mouse,
        // but the mouse just moved.  Make sure to update our
        // destination point.
        mDestinationPoint = eventPoint;
        nsRepeatService::GetInstance()->Stop();
        nsRepeatService::GetInstance()->Start(mMediator);
        break;
      }

       nscoord pos = isHorizontal ? eventPoint.x : eventPoint.y;

       nscoord onePixel = aPresContext->IntScaledPixelsToTwips(1);

       nsIFrame* thumbFrame = mFrames.FirstChild();

       // take our current position and subtract the start location
       pos -= mDragStart;
       PRBool isMouseOutsideThumb = PR_FALSE;
       if (gSnapMultiplier) {
         nsSize thumbSize = thumbFrame->GetSize();
         if (isHorizontal) {
           // horizontal scrollbar - check if mouse is above or below thumb
           // XXXbz what about looking at the .y of the thumb's rect?  Is that
           // always zero here?
           if (eventPoint.y < -gSnapMultiplier * thumbSize.height ||
               eventPoint.y > thumbSize.height +
                                gSnapMultiplier * thumbSize.height)
             isMouseOutsideThumb = PR_TRUE;
         }
         else {
           // vertical scrollbar - check if mouse is left or right of thumb
           if (eventPoint.x < -gSnapMultiplier * thumbSize.width ||
               eventPoint.x > thumbSize.width +
                                gSnapMultiplier * thumbSize.width)
             isMouseOutsideThumb = PR_TRUE;
         }
       }
       if (isMouseOutsideThumb)
       {
         // XXX see bug 81586
         SetCurrentPosition(scrollbar, thumbFrame, (int) (mThumbStart / onePixel / mRatio), PR_FALSE);
         return NS_OK;
       }


       // convert to pixels
       nscoord pospx = pos/onePixel;

       // convert to our internal coordinate system
       pospx = nscoord(pospx/mRatio);

       // if snap="true", then the slider may only be set to min + (increment * x).
       // Otherwise, the slider may be set to any positive integer.
       if (mContent->AttrValueIs(kNameSpaceID_None, nsXULAtoms::snap,
                                 nsXULAtoms::_true, eCaseMatters)) {
         PRInt32 increment = GetIncrement(scrollbar);
         pospx = NSToCoordRound(pospx / (float)increment) * increment;
       }

       // set it
       SetCurrentPosition(scrollbar, thumbFrame, pospx, PR_FALSE);

    }
    break;

    case NS_MOUSE_MIDDLE_BUTTON_UP:
    if(!gMiddlePref)
        break;

    case NS_MOUSE_LEFT_BUTTON_UP:
       // stop capturing
      //printf("stop capturing\n");
      AddListener();
      DragThumb(PR_FALSE);
      if (mChange) {
        nsRepeatService::GetInstance()->Stop();
        mChange = 0;
      }
      mRedrawImmediate = PR_FALSE;//we MUST call nsFrame HandleEvent for mouse ups to maintain the selection state and capture state.
      return nsFrame::HandleEvent(aPresContext, aEvent, aEventStatus);
    }

    // we want to draw immediately if the user doing it directly with the
    // mouse that makes redrawing much faster. Switch it back now.
    mRedrawImmediate = PR_FALSE;

    //return nsFrame::HandleEvent(aPresContext, aEvent, aEventStatus);
    return NS_OK;
  }
  else if ((aEvent->message == NS_MOUSE_LEFT_BUTTON_DOWN && ((nsMouseEvent *)aEvent)->isShift)
      || (gMiddlePref && aEvent->message == NS_MOUSE_MIDDLE_BUTTON_DOWN)) {
    // convert coord from twips to pixels
    nsPoint eventPoint = nsLayoutUtils::GetEventCoordinatesRelativeTo(aEvent,
                                                                      this);
    nscoord pos = isHorizontal ? eventPoint.x : eventPoint.y;

    nscoord onePixel = aPresContext->IntScaledPixelsToTwips(1);
    nscoord pospx = pos/onePixel;

   // adjust so that the middle of the thumb is placed under the click
    nsIFrame* thumbFrame = mFrames.FirstChild();
    nsSize thumbSize = thumbFrame->GetSize();
    nscoord thumbLength = isHorizontal ? thumbSize.width : thumbSize.height;
    thumbLength /= onePixel;
    pospx -= (thumbLength/2);


    // convert to our internal coordinate system
    pospx = nscoord(pospx/mRatio);

    // set it
    SetCurrentPosition(scrollbar, thumbFrame, pospx, PR_FALSE);

    DragThumb(PR_TRUE);

    if (isHorizontal)
      mThumbStart = thumbFrame->GetPosition().x;
    else
      mThumbStart = thumbFrame->GetPosition().y;

    mDragStart = pos - mThumbStart;
  }

  // XXX hack until handle release is actually called in nsframe.
//  if (aEvent->message == NS_MOUSE_EXIT_SYNTH || aEvent->message == NS_MOUSE_RIGHT_BUTTON_UP || aEvent->message == NS_MOUSE_LEFT_BUTTON_UP)
  //   HandleRelease(aPresContext, aEvent, aEventStatus);

  if (aEvent->message == NS_MOUSE_EXIT_SYNTH && mChange)
     HandleRelease(aPresContext, aEvent, aEventStatus);

  return nsFrame::HandleEvent(aPresContext, aEvent, aEventStatus);
}



nsIBox*
nsSliderFrame::GetScrollbar()
{
  // if we are in a scrollbar then return the scrollbar's content node
  // if we are not then return ours.
   nsIFrame* scrollbar;
   nsScrollbarButtonFrame::GetParentWithTag(nsXULAtoms::scrollbar, this, scrollbar);

   if (scrollbar == nsnull)
       return this;

   return scrollbar->IsBoxFrame() ? scrollbar : this;
}

void
nsSliderFrame::PageUpDown(nsIFrame* aThumbFrame, nscoord change)
{
  // on a page up or down get our page increment. We get this by getting the scrollbar we are in and
  // asking it for the current position and the page increment. If we are not in a scrollbar we will
  // get the values from our own node.
  nsIBox* scrollbarBox = GetScrollbar();
  nsCOMPtr<nsIContent> scrollbar;
  scrollbar = GetContentOfBox(scrollbarBox);

  if (mScrollbarListener)
    mScrollbarListener->PagedUpDown(); // Let the listener decide our increment.

  nscoord pageIncrement = GetPageIncrement(scrollbar);
  PRInt32 curpos = GetCurrentPosition(scrollbar);
  PRInt32 minpos = GetMinPosition(scrollbar);
  SetCurrentPosition(scrollbar, aThumbFrame, curpos - minpos + change*pageIncrement, PR_TRUE);
}

// called when the current position changed and we need to update the thumb's location
nsresult
nsSliderFrame::CurrentPositionChanged(nsPresContext* aPresContext)
{
  nsIBox* scrollbarBox = GetScrollbar();
  nsCOMPtr<nsIContent> scrollbar;
  scrollbar = GetContentOfBox(scrollbarBox);

  PRBool isHorizontal = IsHorizontal();

    // get the current position
    PRInt32 curpos = GetCurrentPosition(scrollbar);

    // do nothing if the position did not change
    if (mCurPos == curpos)
        return NS_OK;

    // get our current min and max position from our content node
    PRInt32 minpos = GetMinPosition(scrollbar);
    PRInt32 maxpos = GetMaxPosition(scrollbar);

    if (curpos < minpos || maxpos < minpos)
      curpos = minpos;
    else if (curpos > maxpos)
      curpos = maxpos;

    // convert to pixels
    nscoord onePixel = aPresContext->IntScaledPixelsToTwips(1);
    nscoord curpospx = (curpos - minpos) * onePixel;

    // get the thumb's rect
    nsIFrame* thumbFrame = mFrames.FirstChild();
    if (!thumbFrame)
      return NS_OK; // The thumb may stream in asynchronously via XBL.

    nsRect thumbRect = thumbFrame->GetRect();

    nsRect clientRect;
    GetClientRect(clientRect);

    // figure out the new rect
    nsRect newThumbRect(thumbRect);

    if (isHorizontal)
       newThumbRect.x = clientRect.x + nscoord(float(curpospx)*mRatio);
    else
       newThumbRect.y = clientRect.y + nscoord(float(curpospx)*mRatio);

    // set the rect
    thumbFrame->SetRect(newThumbRect);

    // Figure out the union of the rect so we know what to redraw.
    // Combine the old and new thumb overflow areas.
    nsRect changeRect;
    changeRect.UnionRect(thumbFrame->GetOverflowRect() + thumbRect.TopLeft(),
                         thumbFrame->GetOverflowRect() + newThumbRect.TopLeft());

    // redraw just the change
    Invalidate(changeRect, mRedrawImmediate);
    
    if (mScrollbarListener)
      mScrollbarListener->PositionChanged(aPresContext, mCurPos, curpos);

    mCurPos = curpos;

    return NS_OK;
}

static void UpdateAttribute(nsIContent* aScrollbar, nscoord aNewPos, PRBool aNotify, PRBool aIsSmooth) {
  nsAutoString str;
  str.AppendInt(aNewPos);
  
  if (aIsSmooth) {
    aScrollbar->SetAttr(kNameSpaceID_None, nsXULAtoms::smooth, NS_LITERAL_STRING("true"), PR_FALSE);
  }
  aScrollbar->SetAttr(kNameSpaceID_None, nsXULAtoms::curpos, str, aNotify);
  if (aIsSmooth) {
    aScrollbar->UnsetAttr(kNameSpaceID_None, nsXULAtoms::smooth, PR_FALSE);
  }
}

// newpos should be passed to this function as a position as if the minpos is 0.
// That is, the minpos will be added to the position by this function.
void
nsSliderFrame::SetCurrentPosition(nsIContent* scrollbar, nsIFrame* aThumbFrame, nscoord newpos, PRBool aIsSmooth)
{

   // get our current, min and max position from our content node
  PRInt32 minpos = GetMinPosition(scrollbar);
  PRInt32 maxpos = GetMaxPosition(scrollbar);

  newpos += minpos;

  // get the new position and make sure it is in bounds
  if (newpos < minpos || maxpos < minpos)
      newpos = minpos;
  else if (newpos > maxpos)
      newpos = maxpos;

  nsIBox* scrollbarBox = GetScrollbar();
  nsCOMPtr<nsIScrollbarFrame> scrollbarFrame(do_QueryInterface(scrollbarBox));

  if (scrollbarFrame) {
    // See if we have a mediator.
    nsCOMPtr<nsIScrollbarMediator> mediator;
    scrollbarFrame->GetScrollbarMediator(getter_AddRefs(mediator));
    if (mediator) {
      mediator->PositionChanged(scrollbarFrame, GetCurrentPosition(scrollbar), newpos);
      UpdateAttribute(scrollbar, newpos, PR_FALSE, aIsSmooth);
      CurrentPositionChanged(GetPresContext());
      return;
    }
  }

  UpdateAttribute(scrollbar, newpos, PR_TRUE, aIsSmooth);

#ifdef DEBUG_SLIDER
  printf("Current Pos=%d\n",newpos);
#endif

}

NS_IMETHODIMP
nsSliderFrame::SetInitialChildList(nsIAtom*        aListName,
                                   nsIFrame*       aChildList)
{
  nsresult r = nsBoxFrame::SetInitialChildList(aListName, aChildList);

  AddListener();

  return r;
}

nsresult
nsSliderMediator::MouseDown(nsIDOMEvent* aMouseEvent)
{
  // Only process the event if the thumb is not being dragged.
  if (mSlider && !mSlider->isDraggingThumb())
    return mSlider->MouseDown(aMouseEvent);

  return NS_OK;
}

nsresult
nsSliderMediator::MouseUp(nsIDOMEvent* aMouseEvent)
{
  // Only process the event if the thumb is not being dragged.
  if (mSlider && !mSlider->isDraggingThumb())
    return mSlider->MouseUp(aMouseEvent);

  return NS_OK;
}

nsresult
nsSliderFrame::MouseDown(nsIDOMEvent* aMouseEvent)
{
  //printf("Begin dragging\n");

  if (mContent->AttrValueIs(kNameSpaceID_None, nsHTMLAtoms::disabled,
                            nsXULAtoms::_true, eCaseMatters))
    return NS_OK;

  PRBool isHorizontal = IsHorizontal();

  nsCOMPtr<nsIDOMMouseEvent> mouseEvent(do_QueryInterface(aMouseEvent));

  PRUint16 button = 0;
  PRBool scrollToClick = PR_FALSE;
  mouseEvent->GetShiftKey(&scrollToClick);
  mouseEvent->GetButton(&button);
  if (button != 0) {
    if (button != 1 || !gMiddlePref)
      return NS_OK;
    scrollToClick = PR_TRUE;
  }

  PRInt32 clientPosPx;
  nsIntRect screenRect = GetScreenRect();
  if (isHorizontal) {
    mouseEvent->GetScreenX(&clientPosPx);
    clientPosPx -= screenRect.x;
  } else {
    mouseEvent->GetScreenY(&clientPosPx);
    clientPosPx -= screenRect.y;
  }

  nsPresContext* presContext = GetPresContext();
  nscoord pos = presContext->IntScaledPixelsToTwips(clientPosPx);

  // If shift click or middle button, first
  // place the middle of the slider thumb under the click
  if (scrollToClick) {
    nscoord onePixel = GetPresContext()->IntScaledPixelsToTwips(1);
    nscoord pospx = pos/onePixel;

    // adjust so that the middle of the thumb is placed under the click
    nsIFrame* thumbFrame = mFrames.FirstChild();
    nsSize thumbSize = thumbFrame->GetSize();
    nscoord thumbLength = isHorizontal ? thumbSize.width : thumbSize.height;
    thumbLength /= onePixel;
    pospx -= (thumbLength/2);

    // finally, convert to scrollbar's internal coordinate system
    pospx = nscoord(pospx/mRatio);

    nsIBox* scrollbarBox = GetScrollbar();
    nsCOMPtr<nsIContent> scrollbar;
    scrollbar = GetContentOfBox(scrollbarBox);

    // set it
    SetCurrentPosition(scrollbar, thumbFrame, pospx, PR_FALSE);
  }

  DragThumb(PR_TRUE);

  nsIFrame* thumbFrame = mFrames.FirstChild();

  if (isHorizontal)
     mThumbStart = thumbFrame->GetPosition().x;
  else
     mThumbStart = thumbFrame->GetPosition().y;

  mDragStart = pos - mThumbStart;
  //printf("Pressed mDragStart=%d\n",mDragStart);

  return NS_OK;
}

nsresult
nsSliderFrame::MouseUp(nsIDOMEvent* aMouseEvent)
{
 // printf("Finish dragging\n");

  return NS_OK;
}

void
nsSliderFrame::DragThumb(PRBool aGrabMouseEvents)
{
    // get its view
  nsIView* view = GetView();

  if (view) {
    nsIViewManager* viewMan = view->GetViewManager();

    if (viewMan) {
      PRBool result;

      if (aGrabMouseEvents) {
        viewMan->GrabMouseEvents(view,result);
      } else {
        viewMan->GrabMouseEvents(nsnull,result);
      }
    }
  }
}

PRBool
nsSliderFrame::isDraggingThumb()
{
    // get its view
  nsIView* view = GetView();

  if (view) {
    nsIViewManager* viewMan = view->GetViewManager();

    if (viewMan) {
        nsIView* grabbingView;
        viewMan->GetMouseEventGrabber(grabbingView);
        if (grabbingView == view)
          return PR_TRUE;
    }
  }

  return PR_FALSE;
}

void
nsSliderFrame::AddListener()
{
  if (!mMediator) {
    mMediator = new nsSliderMediator(this);
    NS_ADDREF(mMediator);
  }

  nsIFrame* thumbFrame = mFrames.FirstChild();
  if (thumbFrame) {
    nsCOMPtr<nsIDOMEventReceiver>
      receiver(do_QueryInterface(thumbFrame->GetContent()));

    receiver->AddEventListenerByIID(mMediator, NS_GET_IID(nsIDOMMouseListener));
  }
}

void
nsSliderFrame::RemoveListener()
{
  NS_ASSERTION(mMediator, "No listener was ever added!!");

  nsIFrame* thumbFrame = mFrames.FirstChild();
  if (!thumbFrame)
    return;

  nsCOMPtr<nsIDOMEventReceiver>
    receiver(do_QueryInterface(thumbFrame->GetContent()));

  receiver->RemoveEventListenerByIID(mMediator, NS_GET_IID(nsIDOMMouseListener));
}

NS_IMETHODIMP
nsSliderFrame::HandlePress(nsPresContext* aPresContext,
                           nsGUIEvent*     aEvent,
                           nsEventStatus*  aEventStatus)
{
  if (((nsMouseEvent *)aEvent)->isShift)
    return NS_OK;

  nsIFrame* thumbFrame = mFrames.FirstChild();
  if (!thumbFrame) // display:none?
    return NS_OK;

  if (mContent->AttrValueIs(kNameSpaceID_None, nsHTMLAtoms::disabled,
                            nsXULAtoms::_true, eCaseMatters))
    return NS_OK;
  
  nsRect thumbRect = thumbFrame->GetRect();
  
  nscoord change = 1;
  nsPoint eventPoint = nsLayoutUtils::GetEventCoordinatesRelativeTo(aEvent,
                                                                    this);
  if (IsHorizontal() ? eventPoint.x < thumbRect.x 
                     : eventPoint.y < thumbRect.y)
    change = -1;

  mChange = change;
  DragThumb(PR_TRUE);
  mDestinationPoint = eventPoint;
  PageUpDown(thumbFrame, change);
  
  nsRepeatService::GetInstance()->Start(mMediator);
  
  return NS_OK;
}

NS_IMETHODIMP
nsSliderFrame::HandleRelease(nsPresContext* aPresContext,
                                 nsGUIEvent*     aEvent,
                                 nsEventStatus*  aEventStatus)
{
  nsRepeatService::GetInstance()->Stop();

  return NS_OK;
}

void
nsSliderFrame::Destroy()
{
  // tell our mediator if we have one we are gone.
  if (mMediator) {
    mMediator->SetSlider(nsnull);
    NS_RELEASE(mMediator);
    mMediator = nsnull;
  }

  // call base class Destroy()
  nsBoxFrame::Destroy();
}

NS_IMETHODIMP
nsSliderFrame::GetPrefSize(nsBoxLayoutState& aState, nsSize& aSize)
{
  EnsureOrient();
  return nsBoxFrame::GetPrefSize(aState, aSize);
}

NS_IMETHODIMP
nsSliderFrame::GetMinSize(nsBoxLayoutState& aState, nsSize& aSize)
{
  EnsureOrient();

  // our min size is just our borders and padding
  return nsBox::GetMinSize(aState, aSize);
}

NS_IMETHODIMP
nsSliderFrame::GetMaxSize(nsBoxLayoutState& aState, nsSize& aSize)
{
  EnsureOrient();
  return nsBoxFrame::GetMaxSize(aState, aSize);
}

void
nsSliderFrame::EnsureOrient()
{
  nsIBox* scrollbarBox = GetScrollbar();

  PRBool isHorizontal = (scrollbarBox->GetStateBits() & NS_STATE_IS_HORIZONTAL) != 0;
  if (isHorizontal)
      mState |= NS_STATE_IS_HORIZONTAL;
  else
      mState &= ~NS_STATE_IS_HORIZONTAL;
}


void
nsSliderFrame::SetScrollbarListener(nsIScrollbarListener* aListener)
{
  // Don't addref/release this, since it's actually a frame.
  mScrollbarListener = aListener;
}

NS_IMETHODIMP nsSliderMediator::Notify(nsITimer *timer)
{
  if (mSlider)
    mSlider->Notify(timer);
  return NS_OK;
}

NS_IMETHODIMP_(void) nsSliderFrame::Notify(nsITimer *timer)
{
    PRBool stop = PR_FALSE;

    nsIFrame* thumbFrame = mFrames.FirstChild();
    nsRect thumbRect = thumbFrame->GetRect();

    PRBool isHorizontal = IsHorizontal();

    // See if the thumb has moved past our destination point.
    // if it has we want to stop.
    if (isHorizontal) {
        if (mChange < 0) {
            if (thumbRect.x < mDestinationPoint.x)
                stop = PR_TRUE;
        } else {
            if (thumbRect.x + thumbRect.width > mDestinationPoint.x)
                stop = PR_TRUE;
        }
    } else {
         if (mChange < 0) {
            if (thumbRect.y < mDestinationPoint.y)
                stop = PR_TRUE;
        } else {
            if (thumbRect.y + thumbRect.height > mDestinationPoint.y)
                stop = PR_TRUE;
        }
    }


    if (stop) {
       nsRepeatService::GetInstance()->Stop();
    } else {
      PageUpDown(thumbFrame, mChange);
    }
}

NS_INTERFACE_MAP_BEGIN(nsSliderMediator)
  NS_INTERFACE_MAP_ENTRY(nsIDOMMouseListener)
  NS_INTERFACE_MAP_ENTRY(nsITimerCallback)
  NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(nsISupports, nsITimerCallback)
NS_INTERFACE_MAP_END

NS_IMPL_ADDREF(nsSliderMediator)
NS_IMPL_RELEASE(nsSliderMediator)

