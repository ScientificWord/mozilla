/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
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
 * Portions created by the Initial Developer are Copyright (C) 2002
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or 
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the NPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */


#include <ControlDefinitions.h>

#include "nsNativeScrollbar.h"
#include "nsIDeviceContext.h"

#include "nsWidgetAtoms.h"
#include "nsINameSpaceManager.h"
#include "nsIDOMElement.h"
#include "nsIScrollbarMediator.h"


inline void BoundsCheck(PRInt32 low, PRUint32& value, PRUint32 high)
{
  if ((PRInt32) value < low)
    value = low;
  if (value > high)
    value = high;
}

//
// StControlActionProcOwner
//
// A class that wraps a control action proc so that it is disposed of
// correctly when the shared library shuts down
//
class StNativeControlActionProcOwner {
public:
  
  StNativeControlActionProcOwner ( )
  {
    sControlActionProc = NewControlActionUPP(nsNativeScrollbar::ScrollActionProc);
    NS_ASSERTION(sControlActionProc, "Couldn't create live scrolling action proc");
  }
  ~StNativeControlActionProcOwner ( )
  {
    if ( sControlActionProc )
      DisposeControlActionUPP(sControlActionProc);
  }

  ControlActionUPP ActionProc() { return sControlActionProc; }
  
private:
  ControlActionUPP sControlActionProc;  
};


static ControlActionUPP 
ScrollbarActionProc( )
{
  static StNativeControlActionProcOwner sActionProcOwner;
  return sActionProcOwner.ActionProc();
}


NS_IMPL_ISUPPORTS_INHERITED1(nsNativeScrollbar, nsWindow, nsINativeScrollbar)

nsNativeScrollbar::nsNativeScrollbar()
  : nsMacControl()
  , mContent(nsnull)
  , mMediator(nsnull)
  , mScrollbar(nsnull)
  , mMaxValue(0)
  , mVisibleImageSize(0)
  , mLineIncrement(0)
  , mMouseDownInScroll(PR_FALSE)
  , mClickedPartCode(0)
{
  mMax = 0;   // override the base class default

  WIDGET_SET_CLASSNAME("nsNativeScrollbar");
  SetControlType(kControlScrollBarLiveProc);
}


nsNativeScrollbar::~nsNativeScrollbar()
{
}



//
// Destroy
//
// Now you're gone, gone, gone, whoa-oh...
//
NS_IMETHODIMP
nsNativeScrollbar::Destroy()
{
  if (mMouseDownInScroll)
  {
    ::PostEvent(mouseUp, 0);
  }
  return nsMacControl::Destroy();
}


//
// ScrollActionProc
//
// Called from the OS toolbox while the scrollbar is being tracked.
//
pascal void
nsNativeScrollbar::ScrollActionProc(ControlHandle ctrl, ControlPartCode part)
{
  nsNativeScrollbar* self = (nsNativeScrollbar*)(::GetControlReference(ctrl));
  NS_ASSERTION(self, "NULL nsNativeScrollbar");
  if ( self )
    self->DoScrollAction(part);
}


//
// DoScrollAction
//
// Called from the action proc of the scrollbar, adjust the control's
// value as well as the value in the content node which communicates
// to gecko that the document is scrolling.
// 
void
nsNativeScrollbar::DoScrollAction(ControlPartCode part)
{
  PRUint32 oldPos, newPos;
  PRUint32 incr;
  PRUint32 visibleImageSize;

  if (mOnDestroyCalled)
    return;

  nsCOMPtr<nsIWidget> parent ( GetParent() );
  if (!parent)
  {
    // parent disappeared while scrolling was in progress.  Handling Destroy
    // should have prevented this.  Bail out.
    NS_ASSERTION(parent, "no parent in DoScrollAction");
    return;
  }

  if (!IsQDStateOK()) {
    // Something on a PLEvent messed with the QD state.  When the Control
    // Manager tried to figure out where the mouse was relative to the
    // control, it will have come up with some wacky results.  The received
    // |part| code and the value returned by |GetControl32BitValue| will not
    // be correct.  There's nothing that can be done about it this time
    // through the action proc, so drop the bad data on the floor.  The
    // port state is reset to what's appropriate for the control, and a fake
    // mouse-down event is posted, which will force the Control Manager to
    // look at the scrollbar again, hopefully while the corrected QD state
    // is still in effect.
    //
    // This works in concert with |nsMacControl::HandleControlEvent|.
    //
    // This relies on the Control Manager responding to mouse-down events
    // while the mouse is already down in a tracking loop by reexamining
    // the position of the scrollbar.
    EndDraw();
    StartDraw();
    ::PostEvent(mouseDown, 0);
    return;
  }

  GetPosition(&oldPos);
  GetLineIncrement(&incr);
  GetViewSize(&visibleImageSize);

  PRBool buttonPress = PR_FALSE;

  switch (part)
  {
    case kControlUpButtonPart:
      newPos = oldPos - (mLineIncrement ? mLineIncrement : 1);
      buttonPress = PR_TRUE;
      break;
    case kControlDownButtonPart:
      newPos = oldPos + (mLineIncrement ? mLineIncrement : 1);
      buttonPress = PR_TRUE;
      break;
    
    case kControlPageUpPart:
      newPos = oldPos - visibleImageSize;
      break;
    case kControlPageDownPart:
      newPos = oldPos + visibleImageSize;
      break;

    case kControlIndicatorPart:
      newPos = ::GetControl32BitValue(GetControl());
      break;

    default:
      // Huh?
      return;
  }

  if (buttonPress) {
    //
    // For the up/down buttons, scroll up or down by the line height and 
    // update the attributes on the content node (the scroll frame listens
    // for these attributes and will scroll accordingly). However,
    // if we have a mediator, we're in an outliner and we have to scroll by
    // lines. Outliner ignores the indexes in ScrollbarButtonPressed() except
    // to check if one is greater than the other to indicate direction.
    //
    if (mMediator) {
      BoundsCheck(0, newPos, mMaxValue);
      mMediator->ScrollbarButtonPressed(mScrollbar, oldPos, newPos);
    }
    else {
      UpdateContentPosition(newPos);
    }
  }
  else {
    //
    // For page up/down and dragging the thumb, scroll by the page height
    // (or directly report the value of the scrollbar) and update the attributes
    // on the content node (as above). If we have a mediator, we're in an
    // outliner so tell it directly that the position has changed. Note that
    // outliner takes the new position as a signed reference, so we have to
    // convert our unsigned to signed first.
    //
    UpdateContentPosition(newPos);
    if (mMediator) {
      PRInt32 np = newPos;
      if (np < 0) {
        np = 0;
      }
      mMediator->PositionChanged(mScrollbar, oldPos, np);
    }
  }

  EndDraw();
    
  // update the area of the parent uncovered by the scrolling. Since
  // we may be in a tight loop, we need to manually validate the area
  // we just updated so the update rect doesn't continue to get bigger
  // and bigger the more we scroll.
  parent->Update();
  parent->Validate();

  StartDraw();
}


//
// UpdateContentPosition
//
// Tell the content node that the scrollbar has changed value and
// then update the scrollbar's position
//
void
nsNativeScrollbar::UpdateContentPosition(PRUint32 inNewPos)
{
  if ( (PRInt32)inNewPos == mValue || !mContent )   // break any possible recursion
    return;

  // guarantee |inNewPos| is in the range of [0, mMaxValue] so it's correctly unsigned
  BoundsCheck(0, inNewPos, mMaxValue);

  // convert the int to a string
  nsAutoString buffer;
  buffer.AppendInt(inNewPos);
  
  mContent->SetAttr(kNameSpaceID_None, nsWidgetAtoms::curpos, buffer, PR_TRUE);
  SetPosition(inNewPos);
}

//-------------------------------------------------------------------------
//
// Get the current hilite state of the control (disables the scrollbar
// if there is nowhere to scroll)
// 
//-------------------------------------------------------------------------
ControlPartCode
nsNativeScrollbar::GetControlHiliteState()
{
  if (mMaxValue == 0)
    return kControlInactivePart;
  
  return Inherited::GetControlHiliteState();
}

/**-------------------------------------------------------------------------------
 * DispatchMouseEvent handle an event for this scrollbar
 * @update  dc 08/31/98
 * @Param aEvent -- The mouse event to respond to for this button
 * @return -- True if the event was handled, PR_FALSE if we did not handle it.
 */ 
PRBool
nsNativeScrollbar::DispatchMouseEvent(nsMouseEvent &aEvent)
{
  PRBool eatEvent = PR_FALSE;
  switch (aEvent.message)
  {
    case NS_MOUSE_LEFT_DOUBLECLICK:
    case NS_MOUSE_LEFT_BUTTON_DOWN:
      mMouseDownInScroll = PR_TRUE;
      NS_ASSERTION(this != 0, "NULL nsNativeScrollbar2");
      ::SetControlReference(mControl, (UInt32) this);
      StartDraw();
      {
        Point thePoint;
        thePoint.h = aEvent.refPoint.x;
        thePoint.v = aEvent.refPoint.y;
        mClickedPartCode = ::TestControl(mControl, thePoint);
        if (mClickedPartCode > 0)
          ::HiliteControl(mControl, mClickedPartCode);

        switch (mClickedPartCode)
        {
          case kControlUpButtonPart:
          case kControlDownButtonPart:
          case kControlPageUpPart:
          case kControlPageDownPart:
          case kControlIndicatorPart:
            // We are assuming Appearance 1.1 or later, so we
            // have the "live scroll" variant of the scrollbar,
            // which lets you pass the action proc to TrackControl
            // for the thumb (this was illegal in previous
            // versions of the defproc).
            ::TrackControl(mControl, thePoint, ScrollbarActionProc());
            ::HiliteControl(mControl, 0);
            // We don't dispatch the mouseDown event because mouseUp is eaten
            // by TrackControl anyway and the only messages the app really
            // cares about are the NS_SCROLLBAR_xxx messages.
            eatEvent = PR_TRUE;
            break;
        }
        SetPosition(mValue);
      }
      EndDraw();
      break;


    case NS_MOUSE_LEFT_BUTTON_UP:
      mMouseDownInScroll = PR_FALSE;
      mClickedPartCode = 0;
      break;

    case NS_MOUSE_EXIT:
      if (mWidgetArmed)
      {
        StartDraw();
        ::HiliteControl(mControl, 0);
        EndDraw();
      }
      break;

    case NS_MOUSE_ENTER:
      if (mWidgetArmed)
      {
        StartDraw();
        ::HiliteControl(mControl, mClickedPartCode);
        EndDraw();
      }
      break;
  }

  if (eatEvent)
    return PR_TRUE;
  return (Inherited::DispatchMouseEvent(aEvent));

}


//
// SetMaxRange
//
// Set the maximum range of a scroll bar. This should be set to the
// full scrollable area minus the visible area.
//
NS_IMETHODIMP
nsNativeScrollbar::SetMaxRange(PRUint32 aEndRange)
{
  if ((PRInt32)aEndRange < 0)
    aEndRange = 0;

  mMaxValue = aEndRange;

  if ( GetControl() ) {
    StartDraw();
    ::SetControl32BitMaximum(GetControl(), mMaxValue);
    EndDraw();
  }
  return NS_OK;
}


//
// GetMaxRange
//
// Get the maximum range of a scroll bar
//
NS_IMETHODIMP
nsNativeScrollbar::GetMaxRange(PRUint32* aMaxRange)
{
  *aMaxRange = mMaxValue;
  return NS_OK;
}


//
// SetPosition
//
// Set the current position of the slider and redraw
//
NS_IMETHODIMP
nsNativeScrollbar::SetPosition(PRUint32 aPos)
{
  if ((PRInt32)aPos < 0)
    aPos = 0;

  PRInt32 oldValue = mValue;
  
  // while we _should_ be ensuring that we don't set our value higher
  // than our max value, the gfx scrollview code plays fast and loose
  // with the rules while going back/forward and adjusts the value to the
  // previous value long before it sets the max. As a result, we would
  // lose the given value (since max would most likely be 0). The only
  // way around that is to relax our restrictions a little bit. (bug 135191)
  //   mValue = ((PRInt32)aPos) > mMaxValue ? mMaxValue : ((int)aPos);
  mValue = aPos;
  
  // redraw the scrollbar. It needs to be synchronous otherwise we end
  // up drawing at 0,0, probably because of the associated view.
  if ( mValue != oldValue )
    Invalidate(PR_TRUE);
  
  return NS_OK;
}


//
// GetPosition
//
// Get the current position of the slider
//
NS_IMETHODIMP
nsNativeScrollbar::GetPosition(PRUint32* aPos)
{
  *aPos = mValue;
  return NS_OK;
}


//
// SetViewSize
//
// According to the toolbox docs, we pass the height of the
// visible view area to SetControlViewSize(). Assuming we've set
// the max to the total area - view height, this will give us a correct
// proportional scrollbar.
//
NS_IMETHODIMP
nsNativeScrollbar::SetViewSize(PRUint32 aSize)
{
  if ((PRInt32)aSize < 0)
    aSize = 0;

  mVisibleImageSize = aSize;
    
  if ( GetControl() )  {
    StartDraw();
    ::SetControlViewSize(GetControl(), mVisibleImageSize);
    EndDraw();
  }
  return NS_OK;
}


//
// GetViewSize
//
// Get the height of the visible view area.
//
NS_IMETHODIMP
nsNativeScrollbar::GetViewSize(PRUint32* aSize)
{
  *aSize = mVisibleImageSize;
  return NS_OK;
}


//
// SetLineIncrement
//
// Set the line increment of the scroll bar
//
NS_IMETHODIMP
nsNativeScrollbar::SetLineIncrement(PRUint32 aLineIncrement)
{
  mLineIncrement  = (((int)aLineIncrement) > 0 ? aLineIncrement : 1);
  return NS_OK;
}


//
// GetLineIncrement
//
// Get the line increment of the scroll bar
//
NS_IMETHODIMP
nsNativeScrollbar::GetLineIncrement(PRUint32* aLineIncrement)
{
  *aLineIncrement = mLineIncrement;
  return NS_OK;
}


//
// GetNarrowSize
//
// Ask the appearance manager for the dimensions of the narrow axis
// of the scrollbar. We cheat and assume the width of a vertical scrollbar
// is the same as the height of a horizontal scrollbar. *shrug*. Shoot me.
//
NS_IMETHODIMP
nsNativeScrollbar::GetNarrowSize(PRInt32* outSize)
{
  if ( *outSize )
    return NS_ERROR_FAILURE;
  SInt32 width = 0;
  ::GetThemeMetric(kThemeMetricScrollBarWidth, &width);
  *outSize = width;
  return NS_OK;
}


//
// SetContent
//
// Hook up this native scrollbar to the rest of gecko. We care about
// the content so we can set attributes on it to affect the scrollview. We
// care about the mediator for <outliner> so we can do row-based scrolling.
//
NS_IMETHODIMP
nsNativeScrollbar::SetContent(nsIContent* inContent, nsISupports* inScrollbar, 
                              nsIScrollbarMediator* inMediator)
{
  mContent = inContent;
  mMediator = inMediator;
  mScrollbar = inScrollbar;
  return NS_OK;
}
