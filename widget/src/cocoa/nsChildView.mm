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
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Josh Aas <josh@mozilla.com>
 *   Mark Mentovai <mark@moxienet.com>
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
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#include <unistd.h>

#include "nsChildView.h"

#include "nsCOMPtr.h"
#include "nsToolkit.h"
#include "prmem.h"
#include "nsCRT.h"
#include "nsplugindefs.h"

#include "nsIFontMetrics.h"
#include "nsIDeviceContext.h"
#include "nsIEnumerator.h"
#include "nsIRegion.h"
#include "nsIRollupListener.h"
#include "nsIEventSink.h"
#include "nsIScrollableView.h"
#include "nsIInterfaceRequestor.h"
#include "nsIServiceManager.h"

#include "nsMacResources.h"

#import "nsCursorManager.h"
#import "nsWindowMap.h"

#ifdef MOZ_CAIRO_GFX
#include "gfxContext.h"
#include "gfxQuartzSurface.h"
#else
#include "nsGfxUtils.h" // for StPortSetter
#endif

static NSView* sLastViewEntered = nil;

#if MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_3
// category of NSView methods to quiet warnings

@interface NSView(ChildViewExtensions)
- (void)getRectsBeingDrawn:(const NSRect **)rects count:(int *)count;
- (BOOL)needsToDrawRect:(NSRect)aRect;
- (BOOL)wantsDefaultClipping;
- (void)setHidden:(BOOL)aHidden;
@end
#endif

//#define DEBUG_IME 1

@interface ChildView(Private)

  // sets up our view, attaching it to its owning gecko view
- (id)initWithFrame:(NSRect)inFrame geckoChild:(nsChildView*)inChild eventSink:(nsIEventSink*)inSink;

// sends gecko an ime composition event
- (nsRect) sendCompositionEvent:(PRInt32)aEventType;

// sends gecko an ime text event
- (void) sendTextEvent:(PRUnichar*) aBuffer 
                       attributedString:(NSAttributedString*) aString
                       selectedRange:(NSRange)selRange
                       markedRange:(NSRange)markRange
                       doCommit:(BOOL)doCommit;

  // convert from one event system to the other for event dispatching
- (void) convertEvent:(NSEvent*)inEvent message:(PRInt32)inMsg toGeckoEvent:(nsInputEvent*)outGeckoEvent;

  // create a gecko key event out of a cocoa event
- (void) convertKeyEvent:(NSEvent*)aKeyEvent message:(PRUint32)aMessage
           toGeckoEvent:(nsKeyEvent*)outGeckoEvent;
- (void) convertLocation:(NSPoint)inPoint message:(PRInt32)inMsg
          modifiers:(unsigned int)inMods toGeckoEvent:(nsInputEvent*)outGeckoEvent;

- (NSMenu*)getContextMenu;
- (TopLevelWindowData*)ensureWindowData;

- (void)setIsPluginView:(BOOL)aIsPlugin;
- (BOOL)getIsPluginView;

- (BOOL)childViewHasPlugin;

- (BOOL)isRectObscuredBySubview:(NSRect)inRect;

#if USE_CLICK_HOLD_CONTEXTMENU
 // called on a timer two seconds after a mouse down to see if we should display
 // a context menu (click-hold)
- (void)clickHoldCallback:(id)inEvent;
#endif

@end

#pragma mark -

////////////////////////////////////////////////////
nsIRollupListener * gRollupListener = nsnull;
nsIWidget         * gRollupWidget   = nsnull;

#pragma mark -

//#define PAINT_DEBUGGING         // flash areas as they are painted
//#define INVALIDATE_DEBUGGING    // flash areas as they are invalidated

#if defined(INVALIDATE_DEBUGGING) || defined(PAINT_DEBUGGING)
static void blinkRect(Rect* r);
static void blinkRgn(RgnHandle rgn);
#endif


#pragma mark -


/* Convenience routines to go from a gecko rect to cocoa NSRects and back
 *
 * Gecko rects (nsRect) contain an origin (x,y) in a coordinate
 * system with (0,0) in the top-left of the screen. Cocoa rects
 * (NSRect) contain an origin (x,y) in a coordinate system with
 * (0,0) in the bottom-left of the screen. Both nsRect and NSRect
 * contain width/height info, with no difference in their use.
 * If a Cocoa rect is from a flipped view, there is no need to
 * convert coordinate systems.
 */


static inline void
ConvertGeckoToFlippedCocoaRect(const nsRect & inGeckoRect, NSRect & outCocoaRect)
{
  outCocoaRect.origin.x = inGeckoRect.x;
  outCocoaRect.origin.y = inGeckoRect.y;
  outCocoaRect.size.width = inGeckoRect.width;
  outCocoaRect.size.height = inGeckoRect.height;
}

static inline void
ConvertFlippedCocoaToGeckoRect(const NSRect & inCocoaRect, nsRect & outGeckoRect)
{
  outGeckoRect.x = NS_STATIC_CAST(nscoord, inCocoaRect.origin.x);
  outGeckoRect.y = NS_STATIC_CAST(nscoord, inCocoaRect.origin.y);
  outGeckoRect.width = NS_STATIC_CAST(nscoord, inCocoaRect.size.width);
  outGeckoRect.height = NS_STATIC_CAST(nscoord, inCocoaRect.size.height);
}


static inline void 
ConvertGeckoRectToMacRect(const nsRect& aRect, Rect& outMacRect)
{
  outMacRect.left = aRect.x;
  outMacRect.top = aRect.y;
  outMacRect.right = aRect.x + aRect.width;
  outMacRect.bottom = aRect.y + aRect.height;
}

static PRUint32
UnderlineAttributeToTextRangeType(PRUint32 aUnderlineStyle, NSRange selRange)
{
#ifdef DEBUG_IME
  NSLog(@"****in underlineAttributeToTextRangeType = %d", aUnderlineStyle);
#endif

  // For more info on the underline attribute, please see: 
  // http://developer.apple.com/techpubs/macosx/Cocoa/TasksAndConcepts/ProgrammingTopics/AttributedStrings/Tasks/AccessingAttrs.html
  // We are not clear where the define for value 2 is right now. 
  // To see this value in japanese ime, type 'aaaaaaaaa' and hit space to make the
  // ime send you some part of text in 1 (NSSingleUnderlineStyle) and some part in 2. 
  // ftang will ask apple for more details
  //
  // it probably means show 1-pixel thickness underline vs 2-pixel thickness
  
  PRUint32 attr;
  if (selRange.length == 0) {
    switch (aUnderlineStyle) {
      case 1:
        attr = NS_TEXTRANGE_RAWINPUT;
        break;
      case 2:
      default:
        attr = NS_TEXTRANGE_SELECTEDRAWTEXT;
        break;
    }
  }
  else {
    switch (aUnderlineStyle) {
      case 1:
        attr = NS_TEXTRANGE_CONVERTEDTEXT;
        break;
      case 2:
      default:
        attr = NS_TEXTRANGE_SELECTEDCONVERTEDTEXT;
        break;
    }
  }
  return attr;
}

static PRUint32
CountRanges(NSAttributedString *aString)
{
  // Iterate through aString for the NSUnderlineStyleAttributeName and count the 
  // different segments adjusting limitRange as we go.
  PRUint32 count = 0;
  NSRange effectiveRange;
  NSRange limitRange = NSMakeRange(0, [aString length]);
  while (limitRange.length > 0) {
    [aString attribute:NSUnderlineStyleAttributeName 
             atIndex:limitRange.location 
             longestEffectiveRange:&effectiveRange
             inRange:limitRange];
    limitRange = NSMakeRange(NSMaxRange(effectiveRange), 
                             NSMaxRange(limitRange) - NSMaxRange(effectiveRange));
    count++;
  }
  return count;
}

static void
ConvertAttributeToGeckoRange(NSAttributedString *aString, NSRange markRange, NSRange selRange, PRUint32 inCount, nsTextRange* aRanges)
{
  // Convert the Cocoa range into the nsTextRange Array used in Gecko.
  // Iterate through the attributed string and map the underline attribute to Gecko IME textrange attributes.
  // We may need to change the code here if we change the implementation of validAttributesForMarkedText.
  PRUint32 i = 0;
  NSRange effectiveRange;
  NSRange limitRange = NSMakeRange(0, [aString length]);
  while ((limitRange.length > 0) && (i < inCount)) {
    id attributeValue = [aString attribute:NSUnderlineStyleAttributeName 
                              atIndex:limitRange.location 
                              longestEffectiveRange:&effectiveRange
                              inRange:limitRange];
    aRanges[i].mStartOffset = effectiveRange.location;                         
    aRanges[i].mEndOffset = NSMaxRange(effectiveRange);                         
    aRanges[i].mRangeType = UnderlineAttributeToTextRangeType([attributeValue intValue], selRange); 
    limitRange = NSMakeRange(NSMaxRange(effectiveRange), 
                             NSMaxRange(limitRange) - NSMaxRange(effectiveRange));
    i++;
  }
  // Get current caret position.
  // Caret is indicator of insertion point, so mEndOffset = 0.
  aRanges[i].mStartOffset = selRange.location + selRange.length;                         
  aRanges[i].mEndOffset = 0;                         
  aRanges[i].mRangeType = NS_TEXTRANGE_CARETPOSITION;
}

static void
FillTextRangeInTextEvent(nsTextEvent *aTextEvent, NSAttributedString* aString, NSRange markRange, NSRange selRange)
{ 
  // Count the number of segments in the attributed string and add one more count for sending current caret position to Gecko.
  // Allocate the right size of nsTextRange and draw caret at right position.
  // Convert the attributed string into an array of nsTextRange and get current caret position by calling above functions.
  PRUint32 count = CountRanges(aString) + 1;
  aTextEvent->rangeArray = new nsTextRange[count];
  if (aTextEvent->rangeArray)
  {
    aTextEvent->rangeCount = count;
    ConvertAttributeToGeckoRange(aString, markRange, selRange, aTextEvent->rangeCount,  aTextEvent->rangeArray);
  } 
}

#pragma mark -

//-------------------------------------------------------------------------
//
// nsChildView constructor
//
//-------------------------------------------------------------------------
nsChildView::nsChildView() : nsBaseWidget()
, mView(nsnull)
, mParentView(nsnull)
, mParentWidget(nsnull)
#ifndef MOZ_CAIRO_GFX
, mTempRenderingContextMadeHere(PR_FALSE)
#endif
, mDestructorCalled(PR_FALSE)
, mVisible(PR_FALSE)
, mDrawing(PR_FALSE)
, mAcceptFocusOnClick(PR_TRUE)
, mLiveResizeInProgress(PR_FALSE)
, mPluginDrawing(PR_FALSE)
, mPluginPort(nsnull)
, mVisRgn(nsnull)
{
  SetBackgroundColor(NS_RGB(255, 255, 255));
  SetForegroundColor(NS_RGB(0, 0, 0));
}


//-------------------------------------------------------------------------
//
// nsChildView destructor
//
//-------------------------------------------------------------------------
nsChildView::~nsChildView()
{
  // notify the children that we're gone
  for (nsIWidget* kid = mFirstChild; kid; kid = kid->GetNextSibling()) {
    nsChildView* childView = NS_STATIC_CAST(nsChildView*, kid);
    childView->mParentWidget = nsnull;
  }

  TearDownView(); // should have already been done from Destroy
  
  delete mPluginPort;

  if (mVisRgn)
  {
    ::DisposeRgn(mVisRgn);
    mVisRgn = nsnull;
  }
}

NS_IMPL_ISUPPORTS_INHERITED3(nsChildView, nsBaseWidget, nsIPluginWidget, nsIKBStateControl, nsIEventSink)

//-------------------------------------------------------------------------
//
// Utility method for implementing both Create(nsIWidget ...) and
// Create(nsNativeWidget...)
//-------------------------------------------------------------------------

nsresult nsChildView::StandardCreate(nsIWidget *aParent,
                      const nsRect &aRect,
                      EVENT_CALLBACK aHandleEventFunction,
                      nsIDeviceContext *aContext,
                      nsIAppShell *aAppShell,
                      nsIToolkit *aToolkit,
                      nsWidgetInitData *aInitData,
                      nsNativeWidget aNativeParent)
{
  mBounds = aRect;

//  CalcWindowRegions();

  BaseCreate(aParent, aRect, aHandleEventFunction, 
              aContext, aAppShell, aToolkit, aInitData);

  // inherit things from the parent view and create our parallel 
  // NSView in the Cocoa display system
  mParentView = nil;
  if ( aParent ) {
    SetBackgroundColor(aParent->GetBackgroundColor());
    SetForegroundColor(aParent->GetForegroundColor());

    // inherit the top-level window. NS_NATIVE_WIDGET is always a NSView
    // regardless of if we're asking a window or a view (for compatibility
    // with windows).
    mParentView = (NSView*)aParent->GetNativeData(NS_NATIVE_WIDGET); 
    mParentWidget = aParent;   
  }
  else
    mParentView = NS_REINTERPRET_CAST(NSView*,aNativeParent);
  
  // create our parallel NSView and hook it up to our parent. Recall
  // that NS_NATIVE_WIDGET is the NSView.
  NSRect r;
  ConvertGeckoToFlippedCocoaRect(mBounds, r);
  mView = [CreateCocoaView(r) retain];
  if (!mView) return NS_ERROR_FAILURE;
  
#if DEBUG
  // if our parent is a popup window, we're most certainly coming from a <select> list dropdown which
  // we handle in a different way than other platforms. It's ok that we don't have a parent
  // view because we bailed before even creating the cocoa widgetry and as a result, we
  // don't need to assert. However, if that's not the case, we definitely want to assert
  // to show views aren't getting correctly parented.
  if ( aParent ) {
    nsWindowType windowType;
    aParent->GetWindowType(windowType);
    if ( windowType != eWindowType_popup )
      NS_ASSERTION(mParentView && mView, "couldn't hook up new NSView in hierarchy");
  }
  else
    NS_ASSERTION(mParentView && mView, "couldn't hook up new NSView in hierarchy");
#endif

  // If this view was created in a Gecko view hierarchy, the initial state
  // is hidden.  If the view is attached only to a native NSView but has
  // no Gecko parent (as in embedding), the initial state is visible.
  if (mParentWidget)
    [mView setHidden:YES];
  else
    mVisible = PR_TRUE;

  // Hook it up in the NSView hierarchy.
  if (mParentView) {
    NSWindow* window = [mParentView window];
    if (!window &&
        [mParentView respondsToSelector:@selector(getNativeWindow)])
      window = [mParentView getNativeWindow];

    [mView setNativeWindow:window];

    [mParentView addSubview:mView];
  }

  // if this is a ChildView, make sure that our per-window data
  // is set up
  if ([mView isKindOfClass:[ChildView class]])
    [(ChildView*)mView ensureWindowData];

  return NS_OK;
}


//
// CreateCocoaView
//
// Creates the appropriate child view. Override to create something other than
// our |ChildView| object. Autoreleases, so caller must retain.
//
NSView*
nsChildView::CreateCocoaView(NSRect inFrame)
{
  return [[[ChildView alloc] initWithFrame:inFrame geckoChild:this eventSink:nsnull] autorelease];
}

//-------------------------------------------------------------------------
//
//
//-------------------------------------------------------------------------
void nsChildView::TearDownView()
{
  if (mView)
  {
    NSWindow* win = [mView window];
    NSResponder* responder = [win firstResponder];

    // We're being unhooked from the view hierarchy, don't leave our view
    // or a child view as the window first responder.
    if (responder && [responder isKindOfClass:[NSView class]] &&
        [(NSView*)responder isDescendantOf:mView])
      [win makeFirstResponder: [mView superview]];

    [mView removeFromSuperviewWithoutNeedingDisplay];
    [mView release];
    mView = nil;
  }
}

//-------------------------------------------------------------------------
//
// create a nsChildView
//
//-------------------------------------------------------------------------
NS_IMETHODIMP nsChildView::Create(nsIWidget *aParent,
                      const nsRect &aRect,
                      EVENT_CALLBACK aHandleEventFunction,
                      nsIDeviceContext *aContext,
                      nsIAppShell *aAppShell,
                      nsIToolkit *aToolkit,
                      nsWidgetInitData *aInitData)
{  
  return(StandardCreate(aParent, aRect, aHandleEventFunction,
                          aContext, aAppShell, aToolkit, aInitData,
                            nsnull));
}

//-------------------------------------------------------------------------
//
// Creates a main nsChildView using a native widget (an NSView)
//
//-------------------------------------------------------------------------
NS_IMETHODIMP nsChildView::Create(nsNativeWidget aNativeParent,
                      const nsRect &aRect,
                      EVENT_CALLBACK aHandleEventFunction,
                      nsIDeviceContext *aContext,
                      nsIAppShell *aAppShell,
                      nsIToolkit *aToolkit,
                      nsWidgetInitData *aInitData)
{
  // what we're passed in |aNativeParent| is an NSView. 
  return(StandardCreate(nsnull, aRect, aHandleEventFunction,
                  aContext, aAppShell, aToolkit, aInitData, aNativeParent));
}

//-------------------------------------------------------------------------
//
// Close this nsChildView
//
//-------------------------------------------------------------------------
NS_IMETHODIMP nsChildView::Destroy()
{
  if (mOnDestroyCalled)
    return NS_OK;
  mOnDestroyCalled = PR_TRUE;

  [mView widgetDestroyed];

  nsBaseWidget::OnDestroy();
  nsBaseWidget::Destroy();

  ReportDestroyEvent(); // beard: this seems to cause the window to be deleted. moved all release code to destructor.
  mParentWidget = nil;

  TearDownView();

  return NS_OK;
}

#pragma mark -

#if 0
static void PrintViewHierarcy(NSView *view)
{
  while (view)
  {
    NSLog(@"  view is %@, frame %@", view, NSStringFromRect([view frame]));
    view = [view superview];
  }
}
#endif

//-------------------------------------------------------------------------
//
// Return some native data according to aDataType
//
//-------------------------------------------------------------------------
void* nsChildView::GetNativeData(PRUint32 aDataType)
{
  void* retVal = nsnull;

  switch (aDataType) 
  {
    case NS_NATIVE_WIDGET:            // the NSView
    case NS_NATIVE_DISPLAY:
      retVal = (void*)mView;
      break;

    case NS_NATIVE_WINDOW:
      retVal = [mView getNativeWindow];
      break;
      
    case NS_NATIVE_GRAPHIC:           // quickdraw port
      // XXX qdPort is invalid if we have not locked focus
      retVal = GetChildViewQuickDrawPort();
      break;
      
    case NS_NATIVE_REGION:
    {
      if (!mVisRgn)
        mVisRgn = ::NewRgn();

      // XXX qdPort is invalid if we have not locked focus
      GrafPtr grafPort = GetChildViewQuickDrawPort();
      if (grafPort && mVisRgn)
        ::GetPortVisibleRegion(grafPort, mVisRgn);
      retVal = (void*)mVisRgn;
      break;
    }
      
    case NS_NATIVE_OFFSETX:
      retVal = 0;
      break;

    case NS_NATIVE_OFFSETY:
      retVal = 0;
      break;
    
#if 0
    case NS_NATIVE_COLORMAP:
      //�TODO
      break;
#endif

    case NS_NATIVE_PLUGIN_PORT:
      // this needs to be a combination of the port and the offsets.
      if (mPluginPort == nsnull)
      {
        mPluginPort = new nsPluginPort;
        if ([mView isKindOfClass:[ChildView class]])
          [(ChildView*)mView setIsPluginView: YES];
      }

      NSWindow* window = [mView getNativeWindow];
      if (window)
      {
        WindowRef topLevelWindow = (WindowRef)[window windowRef];
        if (topLevelWindow)
        {
          mPluginPort->port = ::GetWindowPort(topLevelWindow);

          NSPoint viewOrigin = [mView convertPoint:NSZeroPoint toView:nil];
          NSRect frame = [[window contentView] frame];
          viewOrigin.y = frame.size.height - viewOrigin.y;
          
          // need to convert view's origin to window coordinates.
          // then, encode as "SetOrigin" ready values.
          mPluginPort->portx = (PRInt32)-viewOrigin.x;
          mPluginPort->porty = (PRInt32)-viewOrigin.y;

        }
      }
      else
      {
#ifdef DEBUG
        printf("@@@@ Couldn't get NSWindow for plugin port. @@@@\n");
#endif
      }

      retVal = (void*)mPluginPort;
      break;
  }

  return retVal;
}

#pragma mark -


//-------------------------------------------------------------------------
//
// Return PR_TRUE if the whether the component is visible, PR_FALSE otherwise
//
//-------------------------------------------------------------------------
NS_IMETHODIMP nsChildView::IsVisible(PRBool& outState)
{
  if (!mVisible)
    outState = mVisible;
  else
    // mVisible does not accurately reflect the state of a hidden tabbed view
    // so verify that the view has a window as well
    outState = ([mView window] != nil);

  return NS_OK;
}

//-------------------------------------------------------------------------
//
// Hide or show this component
//
//-------------------------------------------------------------------------
NS_IMETHODIMP nsChildView::Show(PRBool aState)
{
  if (aState != mVisible) {
    [mView setHidden:!aState];
    mVisible = aState;
  }
  return NS_OK;
}

nsIWidget*
nsChildView::GetParent(void)
{
  return mParentWidget;
}
    
NS_IMETHODIMP nsChildView::ModalEventFilter(PRBool aRealEvent, void *aEvent,
                                         PRBool *aForWindow)
{
  if (aForWindow)
    *aForWindow = PR_FALSE;
  return NS_ERROR_NOT_IMPLEMENTED;
}


//
// Enable
//
// Enable/disable this view
//
NS_IMETHODIMP nsChildView::Enable(PRBool aState)
{
  // unimplemented;
  return NS_OK;
}


NS_IMETHODIMP nsChildView::IsEnabled(PRBool *aState)
{
  // unimplemented
  if (aState)
   *aState = PR_TRUE;
  return NS_OK;
}

//-------------------------------------------------------------------------
//
// Set the focus on this component
//
//-------------------------------------------------------------------------
NS_IMETHODIMP nsChildView::SetFocus(PRBool aRaise)
{
  NSWindow* window = [mView window];
  if (window)
    [window makeFirstResponder: mView];
  return NS_OK;
}

//-------------------------------------------------------------------------
//
// Get this component font
//
//-------------------------------------------------------------------------
nsIFontMetrics* nsChildView::GetFont(void)
{
#ifdef MOZ_CAIRO_GFX
  return nsnull;
#else
  return mFontMetrics;
#endif
}

    
//-------------------------------------------------------------------------
//
// Set this component font
//
//-------------------------------------------------------------------------
NS_IMETHODIMP nsChildView::SetFont(const nsFont &aFont)
{
#ifdef MOZ_CAIRO_GFX
  return NS_ERROR_NOT_IMPLEMENTED;
#else
  mFontMetrics = nsnull;
  if (mContext)
    mContext->GetMetricsFor(aFont, *getter_AddRefs(mFontMetrics));
  return NS_OK;
#endif
}


//-------------------------------------------------------------------------
//
// Set the colormap of the window
//
//-------------------------------------------------------------------------
NS_IMETHODIMP nsChildView::SetColorMap(nsColorMap *aColorMap)
{
  return NS_OK;
}


//
// SetMenuBar
// ShowMenuBar
// GetMenuBar
//
// Meaningless in this context. A subview doesn't have a menubar.
//

NS_IMETHODIMP nsChildView::SetMenuBar(nsIMenuBar * aMenuBar)
{
  return NS_ERROR_FAILURE;
}

NS_IMETHODIMP nsChildView::ShowMenuBar(PRBool aShow)
{
  return NS_ERROR_FAILURE;
}

nsIMenuBar* nsChildView::GetMenuBar()
{
  return nsnull;
}


//
// SetCursor
//
// Override to set the cursor on the mac
//
NS_IMETHODIMP nsChildView::SetCursor(nsCursor aCursor)
{
  nsBaseWidget::SetCursor(aCursor);
  [[nsCursorManager sharedInstance] setCursor: aCursor];
  return NS_OK;
}

// implement to fix "hidden virtual function" warning
NS_IMETHODIMP nsChildView::SetCursor(imgIContainer* aCursor,
                                      PRUint32 aHotspotX, PRUint32 aHotspotY)
{
  return nsBaseWidget::SetCursor(aCursor, aHotspotX, aHotspotY);
}


#pragma mark -
//-------------------------------------------------------------------------
//
// Get this component dimension
//
//-------------------------------------------------------------------------
NS_IMETHODIMP nsChildView::GetBounds(nsRect &aRect)
{
  aRect = mBounds;
  return NS_OK;
}


NS_METHOD nsChildView::SetBounds(const nsRect &aRect)
{
  nsresult rv = Inherited::SetBounds(aRect);
  if ( NS_SUCCEEDED(rv) ) {
    //CalcWindowRegions();
    NSRect r;
    ConvertGeckoToFlippedCocoaRect(aRect, r);
    [mView setFrame:r];
  }

  return rv;
}


NS_IMETHODIMP nsChildView::ConstrainPosition(PRBool aAllowSlop,
                                             PRInt32 *aX, PRInt32 *aY)
{
  return NS_OK;
}

//-------------------------------------------------------------------------
//
// Move this component
// aX and aY are in the parent widget coordinate system
//-------------------------------------------------------------------------
NS_IMETHODIMP nsChildView::Move(PRInt32 aX, PRInt32 aY)
{
  return MoveWithRepaintOption(aX, aY, PR_TRUE);
}

NS_IMETHODIMP nsChildView::MoveWithRepaintOption(PRInt32 aX, PRInt32 aY, PRBool aRepaint)
{
  if ((mBounds.x != aX) || (mBounds.y != aY))
  {
    // Invalidate the current location
    if (mVisible && aRepaint)
      [[mView superview] setNeedsDisplayInRect: [mView frame]];    //XXX needed?
        
    // Set the bounds
    mBounds.x = aX;
    mBounds.y = aY;
   
    NSRect r;
    ConvertGeckoToFlippedCocoaRect(mBounds, r);
    [mView setFrame:r];

    if (mVisible && aRepaint)
      [mView setNeedsDisplay:YES];

    
    // Report the event
    ReportMoveEvent();
  }
  return NS_OK;
}

//-------------------------------------------------------------------------
//
// Resize this component
//
//-------------------------------------------------------------------------
NS_IMETHODIMP nsChildView::Resize(PRInt32 aWidth, PRInt32 aHeight, PRBool aRepaint)
{
  if ((mBounds.width != aWidth) || (mBounds.height != aHeight))
  {
    // Set the bounds
    mBounds.width  = aWidth;
    mBounds.height = aHeight;

    if (mVisible && aRepaint)
      [[mView superview] setNeedsDisplayInRect: [mView frame]];    //XXX needed?
    
    NSRect r;
    ConvertGeckoToFlippedCocoaRect(mBounds, r);
    [mView setFrame:r];

    if (mVisible && aRepaint)
      [mView setNeedsDisplay:YES];

    // Report the event
    ReportSizeEvent();
  }
 
  return NS_OK;
}

//-------------------------------------------------------------------------
//
// Resize this component
//
//-------------------------------------------------------------------------
NS_IMETHODIMP nsChildView::Resize(PRInt32 aX, PRInt32 aY, PRInt32 aWidth, PRInt32 aHeight, PRBool aRepaint)
{
  MoveWithRepaintOption(aX, aY, aRepaint);
  Resize(aWidth, aHeight, aRepaint);
  return NS_OK;
}


//
// GetPreferredSize
// SetPreferredSize
//
// Nobody even calls these aywhere in the code
//
NS_METHOD nsChildView::GetPreferredSize(PRInt32& aWidth, PRInt32& aHeight)
{
  return NS_ERROR_FAILURE;
}

NS_METHOD nsChildView::SetPreferredSize(PRInt32 aWidth, PRInt32 aHeight)
{
  return NS_ERROR_FAILURE;
}


//-------------------------------------------------------------------------
// 
//
//-------------------------------------------------------------------------
NS_IMETHODIMP nsChildView::BeginResizingChildren(void)
{
  return NS_OK;
}

//-------------------------------------------------------------------------
// 
//
//-------------------------------------------------------------------------
NS_IMETHODIMP nsChildView::EndResizingChildren(void)
{
  return NS_OK;
}

//-------------------------------------------------------------------------
// 
//
//-------------------------------------------------------------------------
NS_IMETHODIMP nsChildView::GetPluginClipRect(nsRect& outClipRect, nsPoint& outOrigin, PRBool& outWidgetVisible)
{
  NS_ASSERTION(mPluginPort, "GetPluginClipRect must only be called on a plugin widget");
  if (!mPluginPort) return NS_ERROR_FAILURE;
  
  NSWindow* window = [mView getNativeWindow];
  if (!window) return NS_ERROR_FAILURE;
  
  NSPoint viewOrigin = [mView convertPoint:NSZeroPoint toView:nil];
  NSRect frame = [[window contentView] frame];
  viewOrigin.y = frame.size.height - viewOrigin.y;
  
  // set up the clipping region for plugins.
  NSRect visibleBounds = [mView visibleRect];
  NSPoint clipOrigin   = [mView convertPoint:visibleBounds.origin toView:nil];
  
  // Convert from cocoa to QuickDraw coordinates
  clipOrigin.y = frame.size.height - clipOrigin.y;
  
  outClipRect.x      = (nscoord)clipOrigin.x;
  outClipRect.y      = (nscoord)clipOrigin.y;
  
  if ([mView window] != nil)
  {
    outClipRect.width  = (nscoord)visibleBounds.size.width;
    outClipRect.height = (nscoord)visibleBounds.size.height;
    outWidgetVisible = PR_TRUE;
  }
  else
  {
    outClipRect.width = 0;
    outClipRect.height = 0;
    outWidgetVisible = PR_FALSE;
  }

  // need to convert view's origin to window coordinates.
  // then, encode as "SetOrigin" ready values.
  outOrigin.x = (nscoord)-viewOrigin.x;
  outOrigin.y = (nscoord)-viewOrigin.y;
  
  return NS_OK;
}


//-------------------------------------------------------------------------
// 
//
//-------------------------------------------------------------------------
NS_IMETHODIMP nsChildView::StartDrawPlugin()
{
#ifndef MOZ_CAIRO_GFX
  NS_ASSERTION(mPluginPort, "StartDrawPlugin must only be called on a plugin widget");
  if (!mPluginPort)
    return NS_ERROR_FAILURE;
  
  // prevent reentrant drawing
  if (mPluginDrawing)
    return NS_ERROR_FAILURE;

  NSWindow* window = [mView getNativeWindow];
  if (!window) return NS_ERROR_FAILURE;

  if (window /* [mView lockFocusIfCanDraw] */)
  {
    // It appears that the WindowRef from which we get the plugin port undergoes the
    // traditional BeginUpdate/EndUpdate cycle, which, if you recall, sets the visible
    // region to the intersection of the visible region and the update region. Since
    // we don't know here if we're being drawn inside a BeginUpdate/EndUpdate pair
    // (which seem to occur in [NSWindow display]), and we don't want to have the burden
    // of correctly doing Carbon invalidates of the plugin rect, we manually set the
    // visible region to be the entire port every time.
    RgnHandle pluginRegion = ::NewRgn();
    if (pluginRegion)
    {
      StPortSetter setter(mPluginPort->port);
      ::SetOrigin(0, 0);

      nsRect  clipRect;   // this is in native window coordinates
      nsPoint origin;
      PRBool visible;
      GetPluginClipRect(clipRect, origin, visible);
      
      // XXX if we're not visible, set an empty clip region?
      Rect pluginRect;
      ConvertGeckoRectToMacRect(clipRect, pluginRect);
      
      ::RectRgn(pluginRegion, &pluginRect);
      ::SetPortVisibleRegion(mPluginPort->port, pluginRegion);
      ::SetPortClipRegion(mPluginPort->port, pluginRegion);
      
      // now set up the origin for the plugin
      ::SetOrigin(origin.x, origin.y);

      ::DisposeRgn(pluginRegion);
    }

    mPluginDrawing = PR_TRUE;
    return NS_OK;
  }
  
  NS_ASSERTION(0, "lockFocusIfCanDraw returned false\n");
#endif
  return NS_ERROR_FAILURE;
}

//-------------------------------------------------------------------------
// 
//
//-------------------------------------------------------------------------
NS_IMETHODIMP nsChildView::EndDrawPlugin()
{
  NS_ASSERTION(mPluginPort, "EndDrawPlugin must only be called on a plugin widget");
  //[mView unlockFocus];
  mPluginDrawing = PR_FALSE;
  return NS_OK;
}

//-------------------------------------------------------------------------
// 
//
//-------------------------------------------------------------------------
void nsChildView::LiveResizeStarted()
{
  // XXX todo. Use this to disable Java async redraw during resize
  mLiveResizeInProgress = PR_TRUE;
}

//-------------------------------------------------------------------------
// 
//
//-------------------------------------------------------------------------
void nsChildView::LiveResizeEnded()
{
  mLiveResizeInProgress = PR_FALSE;
}


#pragma mark -

#if defined(INVALIDATE_DEBUGGING) || defined(PAINT_DEBUGGING)

static Boolean KeyDown(const UInt8 theKey)
{
  KeyMap map;
  GetKeys(map);
  return ((*((UInt8 *)map + (theKey >> 3)) >> (theKey & 7)) & 1) != 0;
}

static Boolean caps_lock()
{
  return KeyDown(0x39);
}

static void blinkRect(Rect* r)
{
  StRegionFromPool oldClip;
  if (oldClip != NULL)
    ::GetClip(oldClip);

  ::ClipRect(r);
  ::InvertRect(r);
  UInt32 end = ::TickCount() + 5;
  while (::TickCount() < end) ;
  ::InvertRect(r);

  if (oldClip != NULL)
    ::SetClip(oldClip);
}

static void blinkRgn(RgnHandle rgn)
{
  StRegionFromPool oldClip;
  if (oldClip != NULL)
    ::GetClip(oldClip);

  ::SetClip(rgn);
  ::InvertRgn(rgn);
  UInt32 end = ::TickCount() + 5;
  while (::TickCount() < end) ;
  ::InvertRgn(rgn);

  if (oldClip != NULL)
    ::SetClip(oldClip);
}

#endif

//-------------------------------------------------------------------------
//
// Invalidate this component's visible area
//
//-------------------------------------------------------------------------
NS_IMETHODIMP nsChildView::Invalidate(PRBool aIsSynchronous)
{
  if (!mView || !mVisible)
    return NS_OK;

  if (aIsSynchronous)
    [mView display];
  else if ([NSView focusView])
  {
    // if a view is focussed (i.e. being drawn), then postpone the invalidate so that we
    // don't lose it.
    [mView performSelector:@selector(setNeedsDisplayWithValue:) withObject:nil afterDelay:0];
  }
  else
    [mView setNeedsDisplay:YES];

  return NS_OK;
}

//-------------------------------------------------------------------------
//
// Invalidate this component's visible area
//
//-------------------------------------------------------------------------
NS_IMETHODIMP nsChildView::Invalidate(const nsRect &aRect, PRBool aIsSynchronous)
{
  if (!mView || !mVisible)
    return NS_OK;

  NSRect r;
  ConvertGeckoToFlippedCocoaRect ( aRect, r );
  
  if (aIsSynchronous)
    [mView displayRect:r];
  else if ([NSView focusView])
  {
    // if a view is focussed (i.e. being drawn), then postpone the invalidate so that we
    // don't lose it.
    [mView performSelector:@selector(setNeedsDisplayWithValue:) withObject:[NSValue valueWithRect:r] afterDelay:0];
  }
  else
    [mView setNeedsDisplayInRect:r];
  
  return NS_OK;
}


//-------------------------------------------------------------------------
//
// Validate the widget
//
//-------------------------------------------------------------------------
NS_IMETHODIMP nsChildView::Validate()
{
  [mView setNeedsDisplay:NO];
  return NS_OK;
}


//-------------------------------------------------------------------------
//
// Invalidate this component's visible area
//
//-------------------------------------------------------------------------

NS_IMETHODIMP nsChildView::InvalidateRegion(const nsIRegion *aRegion, PRBool aIsSynchronous)
{
  if ( !mView || !mVisible)
    return NS_OK;
    
//FIXME rewrite to use a Cocoa region when nsIRegion isn't a QD Region
  NSRect r;
  nsRect bounds;
  nsIRegion* region = NS_CONST_CAST(nsIRegion*, aRegion);     // ugh. this method should be const
  region->GetBoundingBox ( &bounds.x, &bounds.y, &bounds.width, &bounds.height );
  ConvertGeckoToFlippedCocoaRect(bounds, r);
  
  if ( aIsSynchronous )
    [mView displayRect:r];
  else
    [mView setNeedsDisplayInRect:r];

  return NS_OK;
}

inline PRUint16 COLOR8TOCOLOR16(PRUint8 color8)
{
  // return (color8 == 0xFF ? 0xFFFF : (color8 << 8));
  return (color8 << 8) | color8;  /* (color8 * 257) == (color8 * 0x0101) */
}


#ifndef MOZ_CAIRO_GFX
//-------------------------------------------------------------------------
//  StartDraw
//
//-------------------------------------------------------------------------
void nsChildView::StartDraw(nsIRenderingContext* aRenderingContext)
{
  if (mDrawing)
    return;
  mDrawing = PR_TRUE;

  if (aRenderingContext == nsnull)
  {
    // make sure we have a rendering context
    mTempRenderingContext = getter_AddRefs(GetRenderingContext());
    mTempRenderingContextMadeHere = PR_TRUE;
  }
  else
  {
    // if we already have a rendering context, save its state
    mTempRenderingContext = aRenderingContext;
    mTempRenderingContextMadeHere = PR_FALSE;
    mTempRenderingContext->PushState();

    // set the environment to the current widget
    mTempRenderingContext->Init(mContext, this);
  }

  // set the widget font. nsMacControl implements SetFont, which is where
  // the font should get set.
  if (mFontMetrics)
  {
    mTempRenderingContext->SetFont(mFontMetrics);
  }

  // set the widget background and foreground colors
  nscolor color = GetBackgroundColor();
  RGBColor macColor;
  macColor.red   = COLOR8TOCOLOR16(NS_GET_R(color));
  macColor.green = COLOR8TOCOLOR16(NS_GET_G(color));
  macColor.blue  = COLOR8TOCOLOR16(NS_GET_B(color));
  ::RGBBackColor(&macColor);

  color = GetForegroundColor();
  macColor.red   = COLOR8TOCOLOR16(NS_GET_R(color));
  macColor.green = COLOR8TOCOLOR16(NS_GET_G(color));
  macColor.blue  = COLOR8TOCOLOR16(NS_GET_B(color));
  ::RGBForeColor(&macColor);

  mTempRenderingContext->SetColor(color);       // just in case, set the rendering context color too
}


//-------------------------------------------------------------------------
//  EndDraw
//
//-------------------------------------------------------------------------
void nsChildView::EndDraw()
{
  if (!mDrawing)
    return;
  mDrawing = PR_FALSE;

  if (mTempRenderingContextMadeHere)
    mTempRenderingContext->PopState();

  mTempRenderingContext = nsnull;
}
#endif /* MOZ_CAIRO_GFX */

//-------------------------------------------------------------------------
//
//
//-------------------------------------------------------------------------
void
nsChildView::Flash(nsPaintEvent &aEvent)
{
#if 0
  Rect flashRect;
  if (debug_WantPaintFlashing() && aEvent.rect ) {
    ::SetRect ( &flashRect, aEvent.rect->x, aEvent.rect->y, aEvent.rect->x + aEvent.rect->width,
            aEvent.rect->y + aEvent.rect->height );
    StPortSetter portSetter(GetQuickDrawPort());
    unsigned long endTicks;
    ::InvertRect ( &flashRect );
    ::Delay(10, &endTicks);
    ::InvertRect ( &flashRect );
  }
#endif
}


//
// OnPaint
//
// Dummy impl, meant to be overridden
//
PRBool
nsChildView::OnPaint(nsPaintEvent &event)
{
  return PR_TRUE;
}


//
// Update
//
// this is handled for us by UpdateWidget
// 
NS_IMETHODIMP nsChildView::Update()
{
  // Update means "Flush any pending changes right now."  It does *not* mean
  // repaint the world. :) -- dwh
  [mView displayIfNeeded];
  return NS_OK;
}


#pragma mark -


#ifndef MOZ_CAIRO_GFX
//
// UpdateWidget
//
// Dispatches the Paint event into Gecko. Usually called from our 
// NSView in response to the display system noticing that something
// needs repainting. We don't have to worry about painting our child views
// because the display system will take care of that for us.
//
void 
nsChildView::UpdateWidget(nsRect& aRect, nsIRenderingContext* aContext)
{
  if (! mVisible)
    return;

#ifdef DEBUG_vladimir
  fprintf(stderr, "nsChildView[%p]::UpdateWidget called!\n", this);
#else
  // For updating widgets, we _always_ want to use the NSQuickDrawView's port,
  // since that's the correct port for gecko to use to make rendering contexts.
  // The plugin is the only thing that uses the plugin port.
  GrafPtr curPort = GetChildViewQuickDrawPort();
  if (!curPort) return;

  StPortSetter port(curPort);
  
  // initialize the paint event
  nsPaintEvent paintEvent(PR_TRUE, NS_PAINT, this);
  paintEvent.renderingContext = aContext;       // nsPaintEvent
  paintEvent.rect             = &aRect;

  // offscreen drawing is pointless.
  if (paintEvent.rect->x < 0)
    paintEvent.rect->x = 0;
  if (paintEvent.rect->y < 0)
    paintEvent.rect->y = 0;
    
  // draw the widget
  StartDraw(aContext);
  if ( OnPaint(paintEvent) ) {
    nsEventStatus eventStatus;
    DispatchWindowEvent(paintEvent,eventStatus);
    if(eventStatus != nsEventStatus_eIgnore)
      Flash(paintEvent);
  }
  EndDraw();
#endif
}
#endif


//
// Scroll
//
// Scroll the bits of a view and its children
//
// FIXME: I'm sure the invalidating can be optimized, just no time now.
//
NS_IMETHODIMP nsChildView::Scroll(PRInt32 aDx, PRInt32 aDy, nsRect *aClipRect)
{
  BOOL viewWasDirty = NO;
  if (mVisible)
  {
    viewWasDirty = [mView needsDisplay];

    NSSize scrollVector = {aDx,aDy};
    [mView scrollRect: [mView visibleRect] by:scrollVector];
  }
  
  // Scroll the children (even if the widget is not visible)
  for (nsIWidget* kid = mFirstChild; kid; kid = kid->GetNextSibling()) {
    // We use resize rather than move since it gives us control
    // over repainting.  In the case of blitting, Quickdraw views
    // draw their child widgets on the blit, so we can scroll
    // like a bat out of hell by not wasting time invalidating
    // the widgets, since it's completely unnecessary to do so.
    nsRect bounds;
    kid->GetBounds(bounds);
    kid->Resize(bounds.x + aDx, bounds.y + aDy, bounds.width, bounds.height, PR_FALSE);
  }

  if (mVisible)
  {
    if (viewWasDirty)
    {
      [mView setNeedsDisplay:YES];
    }
    else
    {
      NSRect frame = [mView visibleRect];
      NSRect horizInvalid = frame;
      NSRect vertInvalid = frame;
  
      if (aDx != 0) {
        horizInvalid.size.width = abs(aDx);
        if (aDx < 0)
          horizInvalid.origin.x = frame.origin.x + frame.size.width + aDx;
        [mView setNeedsDisplayInRect: horizInvalid];
      }

      if (aDy != 0) {
        vertInvalid.size.height = abs(aDy);
        if (aDy < 0)
          vertInvalid.origin.y = frame.origin.y + frame.size.height + aDy;
        [mView setNeedsDisplayInRect: vertInvalid];
      }

      // We also need to check for any ChildViews which overlap this widget
      // but are not descendent widgets.  If there are any, we need to
      // invalidate the area of this view that these ChildViews will have been
      // blitted into, since these widgets aren't supposed to scroll with
      // this widget.

      // To do this, start at the root Gecko NSView, and walk down along
      // our ancestor view chain, looking at all the subviews in each level
      // of the hierarchy.  If we find a non-ancestor view that overlaps
      // this view, invalidate the area around it.

      // We need to convert all rects to a common ancestor view to intersect
      // them, since a view's frame is in the coordinate space of its parent.
      // Use mParentView as the frame of reference.
      NSRect selfFrame = [mParentView convertRect:[mView frame] fromView:[mView superview]];
      NSView* view = mParentView;
      BOOL selfLevel = NO;

      while (!selfLevel) {
        NSView* nextAncestorView = nil;
        NSArray* subviews = [view subviews];
        for (unsigned int i = 0; i < [subviews count]; ++i) {
          NSView* subView = [subviews objectAtIndex: i];
          if (subView == mView)
            selfLevel = YES;
          else if ([mView isDescendantOf:subView])
            nextAncestorView = subView;
          else {
            NSRect intersectArea = NSIntersectionRect([mParentView convertRect:[subView frame] fromView:[subView superview]], selfFrame);
            if (!NSIsEmptyRect(intersectArea)) {
              NSPoint origin = [mView convertPoint:intersectArea.origin fromView:mParentView];

              if (aDy != 0) {
                vertInvalid.origin.x = origin.x;
                if (aDy < 0)  // scrolled down, invalidate above
                  vertInvalid.origin.y = origin.y + aDy;
                else          // invalidate below
                  vertInvalid.origin.y = origin.y + intersectArea.size.height;
                vertInvalid.size.width = intersectArea.size.width;
                [mView setNeedsDisplayInRect: vertInvalid];
              }

              if (aDx != 0) {
                horizInvalid.origin.y = origin.y;
                if (aDx < 0)  // scrolled right, invalidate to the left
                  horizInvalid.origin.x = origin.x + aDx;
                else          // invalidate to the right
                  horizInvalid.origin.x = origin.x + intersectArea.size.width;
                horizInvalid.size.height = intersectArea.size.height;
                [mView setNeedsDisplayInRect: horizInvalid];
              }
            }
          }
        }
        view = nextAncestorView;
      }
    }
  }
  
  // This is an evil hack that doesn't always work.
  // 
  // Drawing plugins in a Cocoa environment is tricky, because the
  // plugins are living in a Carbon WindowRef/BeginUpdate/EndUpdate
  // world, and Cocoa has its own notion of dirty rectangles. Throw
  // Quartz Extreme and QuickTime into the mix, and things get bad.
  // 
  // This code is working around a cosmetic issue seen when Quartz Extreme
  // is active, and you're scrolling a page with a QuickTime plugin; areas
  // outside the plugin fail to scroll properly. This [display] ensures that
  // the view is properly drawn before the next Scroll call.
  //
  // The time this doesn't work is when you're scrolling a page containing
  // an iframe which in turn contains a plugin.
  //
  // This is turned off because it makes scrolling pages with plugins slow.
  // 
  //if ([mView childViewHasPlugin])
  //  [mView display];

  return NS_OK;
}


//-------------------------------------------------------------------------
//
// Invokes callback and  ProcessEvent method on Event Listener object
//
//-------------------------------------------------------------------------
NS_IMETHODIMP nsChildView::DispatchEvent(nsGUIEvent* event, nsEventStatus& aStatus)
{
  aStatus = nsEventStatus_eIgnore;
  if (! mDestructorCalled)
  {
    nsIWidget* aWidget = event->widget;
    NS_IF_ADDREF(aWidget);
    
    if (mMenuListener != nsnull) {
      if (NS_MENU_EVENT == event->eventStructType)
        aStatus = mMenuListener->MenuSelected( static_cast<nsMenuEvent&>(*event) );
    }
    if (mEventCallback)
      aStatus = (*mEventCallback)(event);

    // Dispatch to event listener if event was not consumed
    if ((aStatus != nsEventStatus_eConsumeNoDefault) && (mEventListener != nsnull))
      aStatus = mEventListener->ProcessEvent(*event);

    NS_IF_RELEASE(aWidget);
  }
  return NS_OK;
}

//-------------------------------------------------------------------------
PRBool nsChildView::DispatchWindowEvent(nsGUIEvent &event)
{
  nsEventStatus status;
  DispatchEvent(&event, status);
  return ConvertStatus(status);
}

//-------------------------------------------------------------------------
PRBool nsChildView::DispatchWindowEvent(nsGUIEvent &event,nsEventStatus &aStatus)
{
  DispatchEvent(&event, aStatus);
  return ConvertStatus(aStatus);
}

//-------------------------------------------------------------------------
//
// Deal with all sort of mouse event
//
//-------------------------------------------------------------------------
PRBool nsChildView::DispatchMouseEvent(nsMouseEvent &aEvent)
{
  PRBool result = PR_FALSE;
  
  if (mEventCallback == nsnull && mMouseListener == nsnull)
    return result;

  // call the event callback 
  if (mEventCallback != nsnull) {
    result = (DispatchWindowEvent(aEvent));
    return result;
  }

  if (mMouseListener != nsnull) {
    switch (aEvent.message) {
      case NS_MOUSE_MOVE:
        result = ConvertStatus(mMouseListener->MouseMoved(aEvent));
        break;

      case NS_MOUSE_LEFT_BUTTON_DOWN:
      case NS_MOUSE_MIDDLE_BUTTON_DOWN:
      case NS_MOUSE_RIGHT_BUTTON_DOWN:
        result = ConvertStatus(mMouseListener->MousePressed(aEvent));
        break;

      case NS_MOUSE_LEFT_BUTTON_UP:
      case NS_MOUSE_MIDDLE_BUTTON_UP:
      case NS_MOUSE_RIGHT_BUTTON_UP:
        result = ConvertStatus(mMouseListener->MouseReleased(aEvent));
        result = ConvertStatus(mMouseListener->MouseClicked(aEvent));
        break;
    } // switch
  } 
  return result;
}

#pragma mark -

//-------------------------------------------------------------------------
//
//
//-------------------------------------------------------------------------
PRBool nsChildView::ReportDestroyEvent()
{
  // nsEvent
  nsGUIEvent event(PR_TRUE, NS_DESTROY, this);
  event.time = PR_IntervalNow();

  // dispatch event
  return (DispatchWindowEvent(event));
}

//-------------------------------------------------------------------------
//
//
//-------------------------------------------------------------------------
PRBool nsChildView::ReportMoveEvent()
{
  // nsEvent
  nsGUIEvent moveEvent(PR_TRUE, NS_MOVE, this);
  moveEvent.refPoint.x     = mBounds.x;
  moveEvent.refPoint.y     = mBounds.y;
  moveEvent.time        = PR_IntervalNow();

  // dispatch event
  return (DispatchWindowEvent(moveEvent));
}

//-------------------------------------------------------------------------
//
//
//-------------------------------------------------------------------------
PRBool nsChildView::ReportSizeEvent()
{
  // nsEvent
  nsSizeEvent sizeEvent(PR_TRUE, NS_SIZE, this);
  sizeEvent.time        = PR_IntervalNow();

  // nsSizeEvent
  sizeEvent.windowSize  = &mBounds;
  sizeEvent.mWinWidth   = mBounds.width;
  sizeEvent.mWinHeight  = mBounds.height;
  
  // dispatch event
  return(DispatchWindowEvent(sizeEvent));
}



#pragma mark -


//-------------------------------------------------------------------------
//
//
//-------------------------------------------------------------------------
void nsChildView::CalcWindowRegions()
{
  // i don't think this is necessary anymore...
}



//-------------------------------------------------------------------------
/*  Calculate the x and y offsets for this particular widget
 *  @update  ps 09/22/98
 *  @param   aX -- x offset amount
 *  @param   aY -- y offset amount 
 *  @return  NOTHING
 */
 
NS_IMETHODIMP nsChildView::CalcOffset(PRInt32 &aX,PRInt32 &aY)
{
  aX = aY = 0;
  NSRect bounds = {{0, 0}, {0, 0}};
  bounds = [mView convertRect:bounds toView:nil];
  aX += NS_STATIC_CAST(PRInt32, bounds.origin.x);
  aY += NS_STATIC_CAST(PRInt32, bounds.origin.y);

  return NS_OK;
}


//-------------------------------------------------------------------------
// PointInWidget
//    Find if a point in local coordinates is inside this object
//-------------------------------------------------------------------------
PRBool nsChildView::PointInWidget(Point aThePoint)
{
  // get the origin in local coordinates
  nsPoint widgetOrigin(0, 0);
  LocalToWindowCoordinate(widgetOrigin);

  // get rectangle relatively to the parent
  nsRect widgetRect;
  GetBounds(widgetRect);

  // convert the topLeft corner to local coordinates
  widgetRect.MoveBy(widgetOrigin.x, widgetOrigin.y);

  // finally tell whether it's a hit
  return widgetRect.Contains(aThePoint.h, aThePoint.v);
}

#pragma mark -


//-------------------------------------------------------------------------
// WidgetToScreen
//    Convert the given rect to global coordinates.
//    @param aLocalRect  -- rect in local coordinates of this widget
//    @param aGlobalRect -- |aLocalRect| in global coordinates
//-------------------------------------------------------------------------
NS_IMETHODIMP nsChildView::WidgetToScreen(const nsRect& aLocalRect, nsRect& aGlobalRect)
{
  NSRect temp;
  ConvertGeckoToFlippedCocoaRect(aLocalRect, temp);
  temp = [mView convertRect:temp toView:nil];                       // convert to window coords
  temp.origin = [[mView getNativeWindow] convertBaseToScreen:temp.origin];   // convert to screen coords
  
  // need to flip the point relative to the main screen
  if ([[NSScreen screens] count] > 0)   // paranoia
  {
    // "global" coords are relative to the upper left of the main screen,
    // which is the first screen in the array (not [NSScreen mainScreen]).
    NSRect mainScreenFrame = [[[NSScreen screens] objectAtIndex:0] frame];
    temp.origin.y = NSMaxY(mainScreenFrame) - temp.origin.y;
  }
  
  ConvertFlippedCocoaToGeckoRect(temp, aGlobalRect);
  
  return NS_OK;
}



//-------------------------------------------------------------------------
// ScreenToWidget
//    Convert the given rect to local coordinates.
//    @param aGlobalRect  -- rect in screen coordinates 
//    @param aLocalRect -- |aGlobalRect| in coordinates of this widget
//-------------------------------------------------------------------------
NS_IMETHODIMP nsChildView::ScreenToWidget(const nsRect& aGlobalRect, nsRect& aLocalRect)
{
  NSRect temp;
  ConvertGeckoToFlippedCocoaRect(aGlobalRect, temp);

  // need to flip the point relative to the main screen
  if ([[NSScreen screens] count] > 0)   // paranoia
  {
    // "global" coords are relative to the upper left of the main screen,
    // which is the first screen in the array (not [NSScreen mainScreen]).
    NSRect mainScreenFrame = [[[NSScreen screens] objectAtIndex:0] frame];
    temp.origin.y = NSMaxY(mainScreenFrame) - temp.origin.y;
  }

  temp.origin = [[mView getNativeWindow] convertScreenToBase:temp.origin];   // convert to screen coords
  temp = [mView convertRect:temp fromView:nil];                     // convert to window coords

  ConvertFlippedCocoaToGeckoRect(temp, aLocalRect);
  
  return NS_OK;
} 


//=================================================================
/*  Convert the coordinates to some device coordinates so GFX can draw.
 *  @update  dc 09/16/98
 *  @param   nscoord -- X coordinate to convert
 *  @param   nscoord -- Y coordinate to convert
 *  @return  NONE
 */
void nsChildView::ConvertToDeviceCoordinates(nscoord &aX, nscoord &aY)
{
  PRInt32 offX = 0, offY = 0;
  this->CalcOffset(offX,offY);

  aX += offX;
  aY += offY;
}


NS_IMETHODIMP nsChildView::CaptureRollupEvents(nsIRollupListener * aListener, 
                                               PRBool aDoCapture, 
                                               PRBool aConsumeRollupEvent)
{
  // this never gets called, only top-level windows can be rollup widgets
  return NS_OK;
}


NS_IMETHODIMP nsChildView::SetTitle(const nsAString& title)
{
  // nothing to do here
  return NS_OK;
}


NS_IMETHODIMP nsChildView::GetAttention(PRInt32 aCycleCount)
{
  [NSApp requestUserAttention:NSInformationalRequest];
  return NS_OK;
}

#pragma mark -

//
// Force Input Method Editor to commit the uncommited input
// Note that this and other nsIKBStateControl methods don't necessarily
// get called on the same ChildView that input is going through.
//
NS_IMETHODIMP nsChildView::ResetInputState()
{
#ifdef DEBUG_IME
  NSLog(@"**** ResetInputState");
#endif

  NSInputManager *currentIM = [NSInputManager currentInputManager];
  
  // commit the current text
  [currentIM unmarkText];

  // and clear the input manager's string
  [currentIM markedTextAbandoned:mView];
  
  return NS_OK;
}

//
// 'open' means that it can take non-ASCII chars
//
NS_IMETHODIMP nsChildView::SetIMEOpenState(PRBool aState)
{
  return NS_ERROR_NOT_IMPLEMENTED;
}

//
// 'open' means that it can take non-ASCII chars
//
NS_IMETHODIMP nsChildView::GetIMEOpenState(PRBool* aState)
{
  return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP nsChildView::SetIMEEnabled(PRBool aState)
{
  return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP nsChildView::GetIMEEnabled(PRBool* aState)
{
  return NS_ERROR_NOT_IMPLEMENTED;
}

//
// Destruct and don't commit the IME composition string.
//
NS_IMETHODIMP nsChildView::CancelIMEComposition()
{
#ifdef DEBUG_IME
  NSLog(@"**** CancelIMEComposition");
#endif
  NSInputManager *currentIM = [NSInputManager currentInputManager];
  [currentIM markedTextAbandoned:mView];
  
  return NS_OK;
}


//
// GetQuickDrawPort
//
// Find a quickdraw port in which to draw (needed by GFX until it
// is converted to Cocoa). This must be overridden if CreateCocoaView()
// does not create something that inherits from NSQuickDrawView!
//
GrafPtr
nsChildView::GetQuickDrawPort()
{
  if (mPluginPort)
    return mPluginPort->port;

  return GetChildViewQuickDrawPort();
}

GrafPtr
nsChildView::GetChildViewQuickDrawPort()
{
#ifndef MOZ_CAIRO_GFX
  if ([mView isKindOfClass:[ChildView class]])
    return (GrafPtr)[(ChildView*)mView qdPort];
#endif

  return nsnull;
}

#pragma mark -


//
// DispatchEvent
//
// Handle an event coming into us and send it to gecko.
//
NS_IMETHODIMP
nsChildView::DispatchEvent ( void* anEvent, PRBool *_retval )
{
  return NS_OK;
}


//
// DragEvent
//
// The drag manager has let us know that something related to a drag has
// occurred in this window. It could be any number of things, ranging from 
// a drop, to a drag enter/leave, or a drag over event. The actual event
// is passed in |aMessage| and is passed along to our event handler so Gecko
// knows about it.
//
NS_IMETHODIMP
nsChildView::DragEvent(PRUint32 aMessage, PRInt16 aMouseGlobalX, PRInt16 aMouseGlobalY,
                         PRUint16 aKeyModifiers, PRBool *_retval)
{
  // ensure that this is going to a ChildView (not something else like a
  // scrollbar). I think it's safe to just bail at this point if it's not
  // what we expect it to be
  if (![mView isKindOfClass:[ChildView class]]) {
    *_retval = PR_FALSE;
    return NS_OK;
  }
  
  nsMouseEvent geckoEvent(PR_TRUE, 0, nsnull, nsMouseEvent::eReal);
  
  // we're given the point in global coordinates. We need to convert it to
  // window coordinates for convert:message:toGeckoEvent

  NSPoint dragLoc = NSMakePoint(aMouseGlobalX, aMouseGlobalY);

  // need to flip the point relative to the main screen
  if ([[NSScreen screens] count] > 0)   // paranoia
  {
    // "global" coords are relative to the upper left of the main screen,
    // which is the first screen in the array (not [NSScreen mainScreen]).
    NSRect mainScreenFrame = [[[NSScreen screens] objectAtIndex:0] frame];
    dragLoc.y = NSMaxY(mainScreenFrame) - dragLoc.y;
  }

  // convert to window coords
  dragLoc = [[mView window] convertScreenToBase:dragLoc];
  // and fill in the event
  [(ChildView*)mView convertLocation:dragLoc message:aMessage modifiers:0 toGeckoEvent:&geckoEvent];

  DispatchWindowEvent(geckoEvent);
  
  // we handled the event
  *_retval = PR_TRUE;
  return NS_OK;
}


//
// Scroll
//
// The cocoa view calls DispatchWindowEvent() directly, so no need for this
//
NS_IMETHODIMP
nsChildView::Scroll(PRBool aVertical, PRInt16 aNumLines, PRInt16 aMouseLocalX, 
                    PRInt16 aMouseLocalY, PRBool *_retval)
{
  return NS_ERROR_NOT_IMPLEMENTED;
}


NS_IMETHODIMP
nsChildView::Idle()
{
  // do some idle stuff?
  return NS_ERROR_NOT_IMPLEMENTED;
}

#ifdef MOZ_CAIRO_GFX
gfxASurface*
nsChildView::GetThebesSurface()
{
#ifdef DEBUG_vladimir
  fprintf (stderr, "nsChildView[%p]::GetThebesSurface\n", this);
#endif

  return new gfxQuartzSurface(gfxASurface::ImageFormatARGB32, 1, 1);
}
#endif

#pragma mark -


@implementation ChildView

//
// initWithFrame:geckoChild:eventSink:
//
// do init stuff
//
- (id)initWithFrame:(NSRect)inFrame geckoChild:(nsChildView*)inChild eventSink:(nsIEventSink*)inSink
{
  if ((self = [super initWithFrame:inFrame])) {
    mGeckoChild = inChild;
    mIsPluginView = NO;
    mCurKeyEvent = nil;
    
    // initialization for NSTextInput
    mMarkedRange.location = NSNotFound;
    mMarkedRange.length = 0;
    mSelectedRange.location = NSNotFound;
    mSelectedRange.length = 0;
    mInComposition = NO;
    mLastMenuForEventEvent = nil;
  }
  
  return self;
}

- (void)dealloc
{
  [mLastMenuForEventEvent release];
  
  if (sLastViewEntered == self)
    sLastViewEntered = nil;
  
  [super dealloc];    // This sets the current port to _savePort (which should be
                      // a valid port, checked with the assertion above.
  SetPort(NULL);      // Bullet-proof against future changes in NSQDView
}

- (void)widgetDestroyed
{
  mGeckoChild = nsnull;
}

//
// -widget
// mozView method
//
// return our gecko child view widget. Note this does not AddRef.
//
- (nsIWidget*) widget
{
  return NS_STATIC_CAST(nsIWidget*, mGeckoChild);
}

//
// -getNativeWindow
// mozView method
//
// get the window that this view is associated with
//
- (NSWindow*)getNativeWindow
{
  NSWindow* currWin = [self window];
  if (currWin)
     return currWin;
  else
     return mWindow;
}

//
// -setNativeWindow:
// mozView method
//
// set the NSWindow that this view is associated with (even when not in the view
// hierarchy).
//
- (void)setNativeWindow:(NSWindow*)aWindow
{
  mWindow = aWindow;
}

//
// -setNeedsDisplayWithValue:
//
//
- (void)setNeedsDisplayWithValue:(NSValue*)inRectValue
{
  if (inRectValue)
  {
    NSRect theRect = [inRectValue rectValue];
    [self setNeedsDisplayInRect:theRect];
  }
  else
  {
    [self setNeedsDisplay:YES];
  }
}


- (NSString*)description
{
  return [NSString stringWithFormat:@"ChildView %p, gecko child %p, frame %@", self, mGeckoChild, NSStringFromRect([self frame])];
}

// Find the nearest scrollable view for this ChildView
// (recall that views are not refcounted)
- (nsIScrollableView*) getScrollableView
{
  nsIScrollableView* scrollableView = nsnull;

  ChildView* currView = self;
  // we have to loop up through superviews in case the view that received the
  // mouseDown is in fact a plugin view with no scrollbars
  while (currView) {

    // This is a hack I learned in nsView::GetViewFor(nsIWidget* aWidget)
    // that I'm not sure is kosher. If anyone knows a better way to get
    // the view for a widget, I'd love to hear it. --Nathan

    void* clientData;
    [currView widget]->GetClientData(clientData);

    nsISupports* data = (nsISupports*)clientData;
    nsCOMPtr<nsIInterfaceRequestor> req(do_QueryInterface(data));
    if (req)
    {
      req->GetInterface(NS_GET_IID(nsIScrollableView), (void**)&scrollableView);
      if (scrollableView)
        break;
    }

    if ([[currView superview] isMemberOfClass:[ChildView class]])
        currView = (ChildView*)[currView superview];
    else
        currView = nil;
  }

  return scrollableView;
}

// set the closed hand cursor and record the starting scroll positions
- (void) startHandScroll:(NSEvent*)theEvent
{
  mHandScrollStartMouseLoc = [[self window] convertBaseToScreen:[theEvent locationInWindow]];

  nsIScrollableView* aScrollableView = [self getScrollableView]; 

  // if we succeeded in getting aScrollableView
  if (aScrollableView) {
    aScrollableView->GetScrollPosition(mHandScrollStartScrollX, mHandScrollStartScrollY);
    mGeckoChild->SetCursor(eCursor_grabbing);
    mInHandScroll = TRUE;
  }
}

// update the scroll position based on the new mouse coordinates
- (void) updateHandScroll:(NSEvent*)theEvent
{
  nsIScrollableView* aScrollableView = [self getScrollableView];
  if (!aScrollableView)
    return;
  
  NSPoint newMouseLoc = [[self window] convertBaseToScreen:[theEvent locationInWindow]];

  PRInt32 deltaX = (PRInt32)(mHandScrollStartMouseLoc.x - newMouseLoc.x);
  PRInt32 deltaY = (PRInt32)(newMouseLoc.y - mHandScrollStartMouseLoc.y);

  // convert to the nsIView coordinates
  float mPixelsToTwips = 1.0;
  mPixelsToTwips = mGeckoChild->GetDeviceContext()->DevUnitsToAppUnits();
  nscoord newX = mHandScrollStartScrollX +
    NSIntPixelsToTwips(deltaX, mPixelsToTwips);
  nscoord newY = mHandScrollStartScrollY +
    NSIntPixelsToTwips(deltaY, mPixelsToTwips);
  aScrollableView->ScrollTo(newX, newY, NS_VMREFRESH_IMMEDIATE);
}

// reset the scroll flag and cursor
- (void) stopHandScroll:(NSEvent*)theEvent
{
  mInHandScroll = FALSE;

  // calling flagsChanged will set the cursor appropriately
  [self flagsChanged:theEvent];
}

// Return true if the correct modifiers are pressed to perform hand scrolling.
+ (BOOL) areHandScrollModifiers:(unsigned int)modifiers
{
  // The command and option key should be held down; ignore caps lock. We only
  // check the low word because Apple started using it in panther for other purposes
  // (no idea what).
  modifiers |= NSAlphaShiftKeyMask; // ignore capsLock by setting it explicitly to match
  return modifiers >> 16 == (NSAlphaShiftKeyMask | NSCommandKeyMask | NSAlternateKeyMask) >> 16;
}

//
// -setFrame
//
// Override in order to keep our mouse enter/exit tracking rect in sync with
// the frame of the view
//
- (void)setFrame:(NSRect)frameRect
{  
  [super setFrame:frameRect];
  if (mMouseEnterExitTag)
    [self removeTrackingRect:mMouseEnterExitTag];

  if ([self window])
    mMouseEnterExitTag = [self addTrackingRect:[self bounds]
                                         owner:self
                                      userData:nil
                                  assumeInside:[[self window] acceptsMouseMovedEvents]];
}


// 
// -isFlipped
//
// Make the origin of this view the topLeft corner (gecko origin) rather
// than the bottomLeft corner (standard cocoa origin).
//
- (BOOL)isFlipped
{
  return YES;
}

// -isOpaque
//
// NSQuickDrawViews do not correctly update if opaque, because of a known incompatibility
// between the way that NSQuickDrawView is implemented, and the NSWindow update mechanism.
// This is unlikely to change in future.
// 
// It's unfortunate, because it's expensive to redraw every parent view when updating
// a portion of any given NSQDView. However, there is no efficient workaround. See
// bug 166932.
// 
- (BOOL)isOpaque
{
#ifdef MOZ_CAIRO_GFX
  // this will be NO when we can do transparent windows/views
  return YES;
#else
  return mIsPluginView;
#endif
}

-(void)setIsPluginView:(BOOL)aIsPlugin
{
  mIsPluginView = aIsPlugin;
}

-(BOOL)getIsPluginView
{
  return mIsPluginView;
}

- (BOOL)childViewHasPlugin
{
  NSArray* subviews = [self subviews];
  for (unsigned int i = 0; i < [subviews count]; i ++) {
    id subview = [subviews objectAtIndex:i];
    if ([subview respondsToSelector:@selector(getIsPluginView)] && [subview getIsPluginView])
      return YES;
  }
  
  return NO;
}

//
// -acceptsFirstResponder
//
// We accept key and mouse events, so don't keep passing them up the chain. Allow
// this to be a 'focussed' widget for event dispatch
//
- (BOOL)acceptsFirstResponder
{
  return YES;
}

- (void)viewWillMoveToWindow:(NSWindow *)newWindow
{
  if (mMouseEnterExitTag)
    [self removeTrackingRect:mMouseEnterExitTag];

  [super viewWillMoveToWindow:newWindow];
}

- (void)viewDidMoveToWindow
{
  if ([self window])
    mMouseEnterExitTag = [self addTrackingRect:[self bounds] owner:self
                                      userData:nil assumeInside: [[self window]
                                      acceptsMouseMovedEvents]];

  [super viewDidMoveToWindow];
}

- (void)viewWillStartLiveResize
{
  if (mGeckoChild && mIsPluginView)
    mGeckoChild->LiveResizeStarted();
  
  [super viewWillStartLiveResize];
}

- (void)viewDidEndLiveResize
{
  if (mGeckoChild && mIsPluginView)
    mGeckoChild->LiveResizeEnded();

  [super viewDidEndLiveResize];
}

- (BOOL)mouseDownCanMoveWindow
{
  return NO;
}

- (void)lockFocus
{
  // Set the current GrafPort to a "safe" port before calling [NSQuickDrawView lockFocus],
  // so that the NSQuickDrawView stashes a pointer to this known-good port internally.
  // It will set the port back to this port on destruction.
  SetPort(NULL);
  [super lockFocus];
}

//
// -drawRect:
//
// The display system has told us that a portion of our view is dirty. Tell
// gecko to paint it
//
- (void)drawRect:(NSRect)aRect
{
  PRBool isVisible;
  if (!mGeckoChild || NS_FAILED(mGeckoChild->IsVisible(isVisible)) || !isVisible)
    return;
  
  // Workaround for the fact that NSQuickDrawViews can't be opaque; see if the rect
  // being drawn is covered by a subview, and, if so, just bail.
  if ([self isRectObscuredBySubview:aRect])
    return;

#ifdef MOZ_CAIRO_GFX
  CGContextRef cgContext = (CGContextRef)[[NSGraphicsContext currentContext] graphicsPort];
  nsRect geckoBounds;
  mGeckoChild->GetBounds(geckoBounds);
  nsRefPtr<gfxQuartzSurface> targetSurface =
    new gfxQuartzSurface(cgContext, geckoBounds.width, geckoBounds.height, PR_FALSE);

  //fprintf (stderr, "Update[%p] [%f %f %f %f] cgc: %p gecko bounds: [%d %d %d %d]\n", mGeckoChild, aRect.origin.x, aRect.origin.y, aRect.size.width, aRect.size.height, cgContext, geckoBounds.x, geckoBounds.y, geckoBounds.width, geckoBounds.height);

  CGAffineTransform xform = CGContextGetCTM(cgContext);
  //fprintf (stderr, "  context xform: t: %f %f xx: %f xy: %f yx: %f yy: %f\n", xform.tx, xform.ty, xform.a, xform.b, xform.c, xform.d);

  nsRefPtr<gfxContext> targetContext = new gfxContext(targetSurface);

#if 0
  targetContext->Rectangle(gfxRect(aRect.origin.x, aRect.origin.y,
                                   aRect.size.width, aRect.size.height));
  targetContext->Clip();
#else
  const NSRect *rects;
  int count, i;
  [self getRectsBeingDrawn:&rects count:&count];
  for (i = 0; i < count; ++i) {
    //fprintf (stderr, " Clip rect[%d]: %f %f %f %f\n", i, rects[i].origin.x, rects[i].origin.y, rects[i].size.width, rects[i].size.height);
    targetContext->Rectangle(gfxRect(rects[i].origin.x, rects[i].origin.y,
                                     rects[i].size.width, rects[i].size.height));
  }
  targetContext->Clip();
#endif

  nsCOMPtr<nsIRenderingContext> rc;
  mGeckoChild->GetDeviceContext()->CreateRenderingContextInstance(*getter_AddRefs(rc));
  rc->Init (mGeckoChild->GetDeviceContext(), targetContext);
  
  nsRect r, tr;
  ConvertFlippedCocoaToGeckoRect(aRect, r);
  tr = r;

  mGeckoChild->LocalToWindowCoordinate(tr);
  //targetContext->Translate(gfxPoint(tr.x, tr.y));

  //fprintf (stderr, "  window coords: [%d %d %d %d]\n", tr.x, tr.y, tr.width, tr.height);

  nsPaintEvent paintEvent(PR_TRUE, NS_PAINT, mGeckoChild);
  paintEvent.renderingContext = rc;
  paintEvent.rect = &r;

  nsEventStatus eventStatus = nsEventStatus_eIgnore;
  mGeckoChild->DispatchWindowEvent(paintEvent);

  paintEvent.renderingContext = nsnull;

  //fprintf (stderr, "---- update done ----\n");

#else /* MOZ_CAIRO_GFX */
  // tell gecko to paint.
  const NSRect *rects;
  int count, i;
  [self getRectsBeingDrawn:&rects count:&count];
  for (i = 0; i < count; ++i) {
    nsRect r;
    ConvertFlippedCocoaToGeckoRect(rects[i], r);
    nsCOMPtr<nsIRenderingContext> rendContext = getter_AddRefs(mGeckoChild->GetRenderingContext());
    mGeckoChild->UpdateWidget(r, rendContext);
  }
#endif
}

- (BOOL)isRectObscuredBySubview:(NSRect)inRect
{
  unsigned int numSubviews = [[self subviews] count];
  for (unsigned int i = 0; i < numSubviews; i++) {
    NSRect subviewFrame = [[[self subviews] objectAtIndex:i] frame];
    if (NSContainsRect(subviewFrame, inRect))
      return YES;
  }
  
  return NO;
}

//
// -wantsDefaultClipping
//
// A panther-only method, allows us to turn off setting up the clip region
// before each drawRect. We already clip within gecko.
//
- (BOOL)wantsDefaultClipping
{
  return NO;
}

#if USE_CLICK_HOLD_CONTEXTMENU
//
// -clickHoldCallback:
//
// called from a timer two seconds after a mouse down to see if we should display
// a context menu (click-hold). |anEvent| is the original mouseDown event. If we're
// still in that mouseDown by this time, put up the context menu, otherwise just
// fuhgeddaboutit. |anEvent| has been retained by the OS until after this callback
// fires so we're ok there.
//
// This code currently messes in a bunch of edge cases (bugs 234751, 232964, 232314)
// so removing it until we get it straightened out.
//
- (void)clickHoldCallback:(id)theEvent;
{
  if( theEvent == [NSApp currentEvent] ) {
    // we're still in the middle of the same mousedown event here, activate
    // click-hold context menu by triggering the right mouseDown action.
    NSEvent* clickHoldEvent = [NSEvent mouseEventWithType:NSRightMouseDown
                                                  location:[theEvent locationInWindow]
                                             modifierFlags:[theEvent modifierFlags]
                                                 timestamp:[theEvent timestamp]
                                              windowNumber:[theEvent windowNumber]
                                                   context:[theEvent context]
                                               eventNumber:[theEvent eventNumber]
                                                clickCount:[theEvent clickCount]
                                                  pressure:[theEvent pressure]];
    [self rightMouseDown:clickHoldEvent];
  }
}
#endif

- (void)mouseDown:(NSEvent *)theEvent
{
  // Make sure this view is not in the rollup widget. The fastest way to do this
  // is by comparing native window pointers. Also don't roll up if we just put
  // the popup up in an earlier menuForEvent: event.
  if (mLastMenuForEventEvent != theEvent && gRollupWidget != nsnull) {
    NSWindow *ourNativeWindow = [self getNativeWindow];
    NSWindow *rollupNativeWindow = (NSWindow*)gRollupWidget->GetNativeData(NS_NATIVE_WINDOW);
    if (ourNativeWindow != rollupNativeWindow) {
      // roll up any popups
      if (gRollupListener != nsnull) {
        gRollupListener->Rollup();
        // If we rolled up a popup, we don't want to pass the click down to gecko.
        // This happens e.g. when you click a popupmenubutton (the menu opens), then click 
        // on the popupmenubutton a second time, which should hide the menu.
        return;
      }
    }
  }
  
  // if the command and alt keys are held down, initiate hand scrolling
  if ([ChildView areHandScrollModifiers:[theEvent modifierFlags]]) {
    [self startHandScroll: theEvent];
    // needed to change the focus, among other things, since we don't
    // get to do that below.
    [super mouseDown:theEvent];
    return; // do not pass this mousedown event to gecko
  }

#if USE_CLICK_HOLD_CONTEXTMENU
  // fire off timer to check for click-hold after two seconds. retains |theEvent|
  [self performSelector:@selector(clickHoldCallback:) withObject:theEvent afterDelay:2.0];
#endif

  nsMouseEvent geckoEvent(PR_TRUE, 0, nsnull, nsMouseEvent::eReal);
  [self convertEvent:theEvent message:NS_MOUSE_LEFT_BUTTON_DOWN toGeckoEvent:&geckoEvent];
  geckoEvent.clickCount = [theEvent clickCount];
  
  NSPoint mouseLoc = [theEvent locationInWindow];
  NSPoint screenLoc = [[self window] convertBaseToScreen: mouseLoc];

  EventRecord macEvent;
  macEvent.what = mouseDown;
  macEvent.message = 0;
  macEvent.when = ::TickCount();
  ::GetGlobalMouse(&macEvent.where);
  macEvent.modifiers = GetCurrentKeyModifiers();
  geckoEvent.nativeMsg = &macEvent;

  // send event into Gecko by going directly to the
  // the widget.
  if (mGeckoChild)
    mGeckoChild->DispatchMouseEvent(geckoEvent);
  
  // XXX maybe call markedTextSelectionChanged:client: here?
}

- (void)mouseUp:(NSEvent *)theEvent
{
  if (mInHandScroll) {
    [self updateHandScroll:theEvent];
    [self stopHandScroll:theEvent];
    return;
  }
  nsMouseEvent geckoEvent(PR_TRUE, 0, nsnull, nsMouseEvent::eReal);
  [self convertEvent:theEvent message:NS_MOUSE_LEFT_BUTTON_UP toGeckoEvent:&geckoEvent];
  
  NSPoint mouseLoc = [theEvent locationInWindow];
  NSPoint screenLoc = [[self window] convertBaseToScreen: mouseLoc];

  EventRecord macEvent;
  macEvent.what = mouseUp;
  macEvent.message = 0;
  macEvent.when = ::TickCount();
  ::GetGlobalMouse(&macEvent.where);
  macEvent.modifiers = GetCurrentKeyModifiers();
  geckoEvent.nativeMsg = &macEvent;

  // send event into Gecko by going directly to the
  // the widget.
  if (mGeckoChild)
    mGeckoChild->DispatchMouseEvent(geckoEvent);
}

- (void)mouseMoved:(NSEvent*)theEvent
{
  NSView* view = [[[self window] contentView] hitTest: [theEvent locationInWindow]];
  if (view != (NSView*)self) {
    // We shouldn't handle this.  Send it to the right view.
    [view mouseMoved: theEvent];
    return;
  }
  
  // If we're passing handling this mouse moved event, we should be the last
  // view entered. If that isn't the case, the mouse probably started over our
  // view and thus we need to send a mouse entered event.
  if (sLastViewEntered != self)
    [self mouseEntered:nil];
  
  // check if we are in a hand scroll or if the user
  // has command and alt held down; if so,  we do not want
  // gecko messing with the cursor.
  if ([ChildView  areHandScrollModifiers:[theEvent modifierFlags]]) {
    return;
  }
  nsMouseEvent geckoEvent(PR_TRUE, 0, nsnull, nsMouseEvent::eReal);
  [self convertEvent:theEvent message:NS_MOUSE_MOVE toGeckoEvent:&geckoEvent];

  NSPoint mouseLoc = [theEvent locationInWindow];
  NSPoint screenLoc = [[self window] convertBaseToScreen: mouseLoc];

  EventRecord macEvent;
  macEvent.what = nullEvent;
  macEvent.message = 0;
  macEvent.when = ::TickCount();
  ::GetGlobalMouse(&macEvent.where);
  
  macEvent.modifiers = GetCurrentKeyModifiers();
  geckoEvent.nativeMsg = &macEvent;

  // send event into Gecko by going directly to the
  // the widget.
  mGeckoChild->DispatchMouseEvent(geckoEvent);
}

- (void)mouseDragged:(NSEvent*)theEvent
{
  // if the handscroll flag is set, steal this event
  if (mInHandScroll) {
    [self updateHandScroll:theEvent];
    return;
  }
  nsMouseEvent geckoEvent(PR_TRUE, 0, nsnull, nsMouseEvent::eReal);
  [self convertEvent:theEvent message:NS_MOUSE_MOVE toGeckoEvent:&geckoEvent];

  EventRecord macEvent;
  macEvent.what = nullEvent;
  macEvent.message = 0;
  macEvent.when = ::TickCount();
  ::GetGlobalMouse(&macEvent.where);
  macEvent.modifiers = btnState | GetCurrentKeyModifiers();
  geckoEvent.nativeMsg = &macEvent;
  
  // send event into Gecko by going directly to the
  // the widget.
  mGeckoChild->DispatchMouseEvent(geckoEvent);    

  // XXX maybe call markedTextSelectionChanged:client: here?
}

static nsEventStatus SendMouseEvent(PRBool isTrusted, PRUint32 msg, nsIWidget *w,
                                    nsMouseEvent::reasonType aReason,
                                    NSPoint* localEventLocation,
                                    nsChildView* receiver)
{
  if (!receiver || !localEventLocation)
    return nsEventStatus_eIgnore;
  
  nsEventStatus status;
  nsMouseEvent event(isTrusted, msg, w, aReason);
  event.refPoint.x = nscoord((PRInt32)localEventLocation->x);
  event.refPoint.y = nscoord((PRInt32)localEventLocation->y);
  receiver->DispatchEvent(&event, status);
  return status;
}

- (void)mouseEntered:(NSEvent*)theEvent
{
  // Getting nil for theEvent is a special case. That only happens if we called 
  // this from mouseExited: because the mouse exited a view that is a subview of this.
  if (theEvent == nil) {
    NSPoint windowEventLocation = [[self window] convertScreenToBase:[NSEvent mouseLocation]];
    NSPoint localEventLocation = [self convertPoint:windowEventLocation fromView:nil];
    // NSLog(@"sending NS_MOUSE_ENTER event with point %f,%f\n", localEventLocation.x, localEventLocation.y);
    SendMouseEvent(PR_TRUE, NS_MOUSE_ENTER, mGeckoChild, nsMouseEvent::eReal, &localEventLocation, mGeckoChild);
    
    // mark this view as the last view entered
    sLastViewEntered = (NSView*)self;
    
    // don't continue because this isn't a standard NSView mouseEntered: call
    return;
  }
  
  NSView* view = [[[self window] contentView] hitTest:[theEvent locationInWindow]];
  if (view == (NSView*)self) {    
    // if we entered from another gecko view then we need to send NS_MOUSE_EXIT to that view
    if (sLastViewEntered)
      [sLastViewEntered mouseExited:nil];
    
    NSPoint eventLocation = [theEvent locationInWindow];
    NSPoint localEventLocation = [self convertPoint:eventLocation fromView:nil];
    // NSLog(@"sending NS_MOUSE_ENTER event with point %f,%f\n", localEventLocation.x, localEventLocation.y);
    SendMouseEvent(PR_TRUE, NS_MOUSE_ENTER, mGeckoChild, nsMouseEvent::eReal, &localEventLocation, mGeckoChild);
    
    // mark this view as the last view entered
    sLastViewEntered = (NSView*)self;
  }
  
  // checks to see if we should change to the hand cursor
  [self flagsChanged:theEvent];
}

- (void)mouseExited:(NSEvent*)theEvent
{
  // Getting nil for theEvent is a special case. That only happens if we called this
  // from mouseEntered: because the mouse entered a view that is a subview of this.
  if (theEvent == nil) {    
    NSPoint windowEventLocation = [[self window] convertScreenToBase:[NSEvent mouseLocation]];
    NSPoint localEventLocation = [self convertPoint:windowEventLocation fromView:nil];
    // NSLog(@"sending NS_MOUSE_EXIT event with point %f,%f\n", localEventLocation.x, localEventLocation.y);
    SendMouseEvent(PR_TRUE, NS_MOUSE_EXIT, mGeckoChild, nsMouseEvent::eReal, &localEventLocation, mGeckoChild);
    
    // don't continue because this isn't a standard NSView mouseExited: call
    return;
  }
  
  if (sLastViewEntered == (NSView*)self) {
    NSPoint eventLocation = [theEvent locationInWindow];
    NSPoint localEventLocation = [self convertPoint:eventLocation fromView:nil];
    // NSLog(@"sending NS_MOUSE_EXIT event with point %f,%f\n", localEventLocation.x, localEventLocation.y);
    SendMouseEvent(PR_TRUE, NS_MOUSE_EXIT, mGeckoChild, nsMouseEvent::eReal, &localEventLocation, mGeckoChild);
    
    // Now we need to send NS_MOUSE_ENTERED to whatever view we're over now.
    // Otherwise that view won't get get the Cocoa mouseEntered: if it was a
    // superview of this view. Be careful with this logic, remember that the mouse
    // can move onto another window from another app that overlaps this view.
    NSView* view = [[[self window] contentView] hitTest:eventLocation];
    if (view && view != self && [view isKindOfClass:[ChildView class]] && [self isDescendantOf:view])
      [(ChildView*)view mouseEntered:nil];
    else
      sLastViewEntered = nil;
  }
  
  // Gecko may have set the cursor to ibeam or link hand, or handscroll may
  // have set it to the open hand cursor. Cocoa won't call this during a drag.
  mGeckoChild->SetCursor(eCursor_standard);
}

- (void)rightMouseDown:(NSEvent *)theEvent
{  
  // Make sure this view is not in the rollup widget. The fastest way to do this
  // is by comparing native window pointers. Also don't roll up if we just put
  // the popup up in an earlier menuForEvent: event.
  if (mLastMenuForEventEvent != theEvent && gRollupWidget != nsnull) {
    NSWindow *ourNativeWindow = [self getNativeWindow];
    NSWindow *rollupNativeWindow = (NSWindow*)gRollupWidget->GetNativeData(NS_NATIVE_WINDOW);
    if (ourNativeWindow != rollupNativeWindow) {
      // roll up any popups
      if (gRollupListener != nsnull)
        gRollupListener->Rollup();
    }
  }
  
  // The right mouse went down, fire off a right mouse down event to gecko
  nsMouseEvent geckoEvent(PR_TRUE, 0, nsnull, nsMouseEvent::eReal);
  [self convertEvent:theEvent message:NS_MOUSE_RIGHT_BUTTON_DOWN toGeckoEvent:&geckoEvent];

  // plugins need a native event here
  EventRecord macEvent;
  macEvent.what = mouseDown;
  macEvent.message = 0;
  macEvent.when = ::TickCount();
  ::GetGlobalMouse(&macEvent.where);
  macEvent.modifiers = controlKey;  // fake a context menu click
  geckoEvent.nativeMsg = &macEvent;

  geckoEvent.clickCount = [theEvent clickCount];
  PRBool handled = mGeckoChild->DispatchMouseEvent(geckoEvent);
  if (!handled)
    [super rightMouseDown:theEvent];    // let the superview do context menu stuff
}

- (void)rightMouseUp:(NSEvent *)theEvent
{
  nsMouseEvent geckoEvent(PR_TRUE, 0, nsnull, nsMouseEvent::eReal);
  [self convertEvent:theEvent message:NS_MOUSE_RIGHT_BUTTON_UP toGeckoEvent:&geckoEvent];

  // plugins need a native event here
  EventRecord macEvent;
  macEvent.what = mouseUp;
  macEvent.message = 0;
  macEvent.when = ::TickCount();
  ::GetGlobalMouse(&macEvent.where);
  macEvent.modifiers = controlKey;  // fake a context menu click
  geckoEvent.nativeMsg = &macEvent;

  geckoEvent.clickCount = [theEvent clickCount];
  PRBool handled = mGeckoChild->DispatchMouseEvent(geckoEvent);
  if (!handled)
    [super rightMouseUp:theEvent];
}

- (void)otherMouseDown:(NSEvent *)theEvent
{
  nsMouseEvent geckoEvent(PR_TRUE, 0, nsnull, nsMouseEvent::eReal);
  [self convertEvent:theEvent message:NS_MOUSE_MIDDLE_BUTTON_DOWN toGeckoEvent:&geckoEvent];
  geckoEvent.clickCount = [theEvent clickCount];
  
  // send event into Gecko by going directly to the
  // the widget.
  mGeckoChild->DispatchMouseEvent(geckoEvent);
  
} // otherMouseDown


- (void)otherMouseUp:(NSEvent *)theEvent
{
  nsMouseEvent geckoEvent(PR_TRUE, 0, nsnull, nsMouseEvent::eReal);
  [self convertEvent:theEvent message:NS_MOUSE_MIDDLE_BUTTON_UP toGeckoEvent:&geckoEvent];
  
  // send event into Gecko by going directly to the
  // the widget.
  mGeckoChild->DispatchMouseEvent(geckoEvent);
  
} // mouseUp

//
// -scrollWheel:forAxis:
//
// Handle an NSScrollWheel event for a single axis only.
//
-(void)scrollWheel:(NSEvent*)theEvent forAxis:(enum nsMouseScrollEvent::nsMouseScrollFlags)inAxis
{
  float scrollDelta;

  if (inAxis & nsMouseScrollEvent::kIsVertical)
    scrollDelta = -[theEvent deltaY];
  else if (inAxis & nsMouseScrollEvent::kIsHorizontal)
    scrollDelta = -[theEvent deltaX];
  else
    // Caller screwed up
    return;

  if (scrollDelta == 0)
    // No sense in firing off a Gecko event.  Note that as of 10.4 Tiger,
    // a single NSScrollWheel event might result in deltaX = deltaY = 0.
    return;
  
  nsMouseScrollEvent geckoEvent(PR_TRUE, 0, nsnull);
  [self convertEvent:theEvent message:NS_MOUSE_SCROLL toGeckoEvent:&geckoEvent];
  geckoEvent.scrollFlags |= inAxis;

  // Gecko only understands how to scroll by an integer value.  Using floor
  // and ceil is better than truncating the fraction, especially when
  // |delta| < 1.
  if (scrollDelta < 0)
    geckoEvent.delta = (PRInt32)floorf(scrollDelta);
  else
    geckoEvent.delta = (PRInt32)ceilf(scrollDelta);

  mGeckoChild->DispatchWindowEvent(geckoEvent);

  // dispatch scroll wheel carbon event for plugins
  {
    EventRef theEvent;
    OSStatus err = ::MacCreateEvent(NULL,
                          kEventClassMouse,
                          kEventMouseWheelMoved,
                          TicksToEventTime(TickCount()),
                          kEventAttributeUserEvent,
                          &theEvent);
    if (err == noErr)
    {
      EventMouseWheelAxis axis;
      if (inAxis & nsMouseScrollEvent::kIsVertical)
        axis = kEventMouseWheelAxisY;
      else if (inAxis & nsMouseScrollEvent::kIsHorizontal)
        axis = kEventMouseWheelAxisX;
      
      SetEventParameter(theEvent,
                            kEventParamMouseWheelAxis,
                            typeMouseWheelAxis,
                            sizeof(EventMouseWheelAxis),
                            &axis);

      SInt32 delta = (SInt32)-geckoEvent.delta;
      SetEventParameter(theEvent,
                            kEventParamMouseWheelDelta,
                            typeLongInteger,
                            sizeof(SInt32),
                            &delta);

      Point mouseLoc;
      ::GetGlobalMouse(&mouseLoc);
      SetEventParameter(theEvent,
                            kEventParamMouseLocation,
                            typeQDPoint,
                            sizeof(Point),
                            &mouseLoc);
      
      SendEventToWindow(theEvent, (WindowRef)[[self window] windowRef]);
      ReleaseEvent(theEvent);
    }
  }
}

-(void)scrollWheel:(NSEvent*)theEvent
{
  // close popups if we're supposed to...
  if (gRollupListener && gRollupWidget &&
      [self window] != gRollupWidget->GetNativeData(NS_NATIVE_WINDOW)) {
    PRBool rollup = PR_FALSE;
    gRollupListener->ShouldRollupOnMouseWheelEvent(&rollup);
    if (rollup)
      gRollupListener->Rollup();
  }
  
  // It's possible for a single NSScrollWheel event to carry both useful
  // deltaX and deltaY, for example, when the "wheel" is a trackpad.
  // NSMouseScrollEvent can only carry one axis at a time, so the system
  // event will be split into two Gecko events if necessary.
  [self scrollWheel:theEvent forAxis:nsMouseScrollEvent::kIsVertical];
  [self scrollWheel:theEvent forAxis:nsMouseScrollEvent::kIsHorizontal];
}

-(NSMenu*)menuForEvent:(NSEvent*)theEvent
{
  if ([self getIsPluginView])
    return nil;
  
  [mLastMenuForEventEvent release];
  mLastMenuForEventEvent = [theEvent retain];
  
  // Fire the context menu event into Gecko.
  nsMouseEvent geckoEvent(PR_TRUE, 0, nsnull, nsMouseEvent::eReal);
  [self convertEvent:theEvent message:NS_CONTEXTMENU toGeckoEvent:&geckoEvent];
  mGeckoChild->DispatchMouseEvent(geckoEvent);
  
  // Go up our view chain to fetch the correct menu to return.
  return [self getContextMenu];
}

- (NSMenu*)getContextMenu
{
  NSView* superView = [self superview];
  if ([superView respondsToSelector:@selector(getContextMenu)])
    return [(NSView<mozView>*)superView getContextMenu];

  return nil;
}

- (TopLevelWindowData*)ensureWindowData
{
  WindowDataMap* windowMap = [WindowDataMap sharedWindowDataMap];

  TopLevelWindowData* windowData = [windowMap dataForWindow:mWindow];
  if (mWindow && !windowData)
  {
    windowData = [[TopLevelWindowData alloc] initWithWindow:mWindow];
    [windowMap setData:windowData forWindow:mWindow]; // takes ownership
    [windowData release];
  }
  return windowData;
}

//
// -convertEvent:message:toGeckoEvent:
//
// convert from one event system to the other for event dispatching
//
- (void) convertEvent:(NSEvent*)inEvent message:(PRInt32)inMsg toGeckoEvent:(nsInputEvent*)outGeckoEvent
{
  outGeckoEvent->nativeMsg = inEvent;
  [self convertLocation:[inEvent locationInWindow] message:inMsg modifiers:[inEvent modifierFlags]
          toGeckoEvent:outGeckoEvent];
}

- (void) convertLocation:(NSPoint)inPoint message:(PRInt32)inMsg modifiers:(unsigned int)inMods toGeckoEvent:(nsInputEvent*)outGeckoEvent
{
  outGeckoEvent->message = inMsg;
  outGeckoEvent->widget = [self widget];
  outGeckoEvent->time = PR_IntervalNow();
  
  if (outGeckoEvent->eventStructType != NS_KEY_EVENT) {
    NSPoint mouseLoc = inPoint;
    
    // convert point to view coordinate system
    NSPoint localPoint = [self convertPoint:mouseLoc fromView:nil];
    
    outGeckoEvent->refPoint.x = NS_STATIC_CAST(nscoord, localPoint.x);
    outGeckoEvent->refPoint.y = NS_STATIC_CAST(nscoord, localPoint.y);
  }
  
  // set up modifier keys
  outGeckoEvent->isShift    = ((inMods & NSShiftKeyMask) != 0);
  outGeckoEvent->isControl  = ((inMods & NSControlKeyMask) != 0);
  outGeckoEvent->isAlt      = ((inMods & NSAlternateKeyMask) != 0);
  outGeckoEvent->isMeta     = ((inMods & NSCommandKeyMask) != 0);
}

 
static PRBool ConvertUnicodeToCharCode(PRUnichar inUniChar, unsigned char* outChar)
{
  UnicodeToTextInfo converterInfo;
  TextEncoding      systemEncoding;
  Str255            convertedString;
  OSStatus          err;
  
  *outChar = 0;
  
  err = ::UpgradeScriptInfoToTextEncoding(smSystemScript, kTextLanguageDontCare, kTextRegionDontCare, NULL, &systemEncoding);
  if (err != noErr)
    return PR_FALSE;
  
  err = ::CreateUnicodeToTextInfoByEncoding(systemEncoding, &converterInfo);
  if (err != noErr)
    return PR_FALSE;
  
  err = ::ConvertFromUnicodeToPString(converterInfo, sizeof(PRUnichar), &inUniChar, convertedString);
  if (err != noErr)
    return PR_FALSE;

  *outChar = convertedString[1];
  ::DisposeUnicodeToTextInfo(&converterInfo);
  return PR_TRUE;
}

static void ConvertCocoaKeyEventToMacEvent(NSEvent* cocoaEvent, EventRecord& macEvent)
{
    if ([cocoaEvent type] == NSKeyDown)
      macEvent.what = [cocoaEvent isARepeat] ? autoKey : keyDown;
    else
      macEvent.what = keyUp;

    UInt32 charCode = [[cocoaEvent characters] characterAtIndex: 0];
    if (charCode >= 0x0080)
    {
        switch (charCode) {
        case NSUpArrowFunctionKey:
            charCode = kUpArrowCharCode;
            break;
        case NSDownArrowFunctionKey:
            charCode = kDownArrowCharCode;
            break;
        case NSLeftArrowFunctionKey:
            charCode = kLeftArrowCharCode;
            break;
        case NSRightArrowFunctionKey:
            charCode = kRightArrowCharCode;
            break;
        default:
            unsigned char convertedCharCode;
            if (ConvertUnicodeToCharCode(charCode, &convertedCharCode))
              charCode = convertedCharCode;
            //NSLog(@"charcode is %d, converted to %c, char is %@", charCode, convertedCharCode, [cocoaEvent characters]);
            break;
        }
    }
    macEvent.message = (charCode & 0x00FF) | ([cocoaEvent keyCode] << 8);
    macEvent.when = ::TickCount();
    ::GetGlobalMouse(&macEvent.where);
    macEvent.modifiers = ::GetCurrentKeyModifiers();
}

- (nsRect)sendCompositionEvent:(PRInt32) aEventType
{
#ifdef DEBUG_IME
  NSLog(@"****in sendCompositionEvent; type = %d", aEventType);
#endif

  // static void init_composition_event( *aEvent, int aType)
  nsCompositionEvent event(PR_TRUE, aEventType, mGeckoChild);
  event.time = PR_IntervalNow();
  mGeckoChild->DispatchWindowEvent(event);
  return event.theReply.mCursorPosition;
}

- (void)sendTextEvent:(PRUnichar*) aBuffer 
                      attributedString:(NSAttributedString*) aString  
                      selectedRange:(NSRange) selRange 
                      markedRange:(NSRange) markRange
                      doCommit:(BOOL) doCommit
{
#ifdef DEBUG_IME
  NSLog(@"****in sendTextEvent; string = '%@'", aString);
  NSLog(@" markRange = %d, %d;  selRange = %d, %d", markRange.location, markRange.length, selRange.location, selRange.length);
#endif

  nsTextEvent textEvent(PR_TRUE, NS_TEXT_TEXT, mGeckoChild);
  textEvent.time = PR_IntervalNow();
  textEvent.theText = aBuffer;
  if (!doCommit)
    FillTextRangeInTextEvent(&textEvent, aString, markRange, selRange);

  mGeckoChild->DispatchWindowEvent(textEvent);
  if ( textEvent.rangeArray )
    delete [] textEvent.rangeArray;
}

#define MAX_BUFFER_SIZE 32

// NSTextInput implementation

- (void)insertText:(id)insertString
{
#if DEBUG_IME
  NSLog(@"****in insertText: '%@'", insertString);
  NSLog(@" markRange = %d, %d;  selRange = %d, %d", mMarkedRange.location, mMarkedRange.length, mSelectedRange.location, mSelectedRange.length);
#endif

  if (![insertString isKindOfClass:[NSAttributedString class]])
    insertString = [[[NSAttributedString alloc] initWithString:insertString] autorelease];

  NSString *tmpStr = [insertString string];
  unsigned int len = [tmpStr length];
  PRUnichar buffer[MAX_BUFFER_SIZE];
  PRUnichar *bufPtr = (len >= MAX_BUFFER_SIZE) ? new PRUnichar[len + 1] : buffer;
  [tmpStr getCharacters: bufPtr];
  bufPtr[len] = (PRUnichar)'\0';

  if (len == 1 && !mInComposition)
  {
    // dispatch keypress event with char instead of textEvent
    nsKeyEvent geckoEvent(PR_TRUE, NS_KEY_PRESS, mGeckoChild);
    geckoEvent.time      = PR_IntervalNow();
    geckoEvent.charCode  = bufPtr[0]; // gecko expects OS-translated unicode
    geckoEvent.isChar    = PR_TRUE;
    geckoEvent.isShift   = ([mCurKeyEvent modifierFlags] & NSShiftKeyMask) != 0;
    // don't set other modifiers from the current event, because here in
    // -insertText: they've already been taken into account in creating
    // the input string.
        
    // plugins need a native keyDown or autoKey event here
    EventRecord macEvent;
    if (mCurKeyEvent)
    {
      ConvertCocoaKeyEventToMacEvent(mCurKeyEvent, macEvent);
      geckoEvent.nativeMsg = &macEvent;
    }

    mGeckoChild->DispatchWindowEvent(geckoEvent);
  }
  else
  {
    if (!mInComposition)
    {
      // send start composition event to gecko
      [self sendCompositionEvent: NS_COMPOSITION_START];
      mInComposition = YES;
    }

    // dispatch textevent (is this redundant?)
    [self sendTextEvent:bufPtr attributedString:insertString
                               selectedRange:NSMakeRange(0, len)
                               markedRange:mMarkedRange
                               doCommit:YES];

    // send end composition event to gecko
    [self sendCompositionEvent: NS_COMPOSITION_END];
    mInComposition = NO;
    mSelectedRange = mMarkedRange = NSMakeRange(NSNotFound, 0);
  }

  if (bufPtr != buffer)
    delete[] bufPtr;
}

- (void)insertNewline:(id)sender
{
  // dummy impl, does nothing (other than stop the beeping when hitting return)
}

- (void) doCommandBySelector:(SEL)aSelector
{ 
#if DEBUG_IME 
  NSLog(@"**** in doCommandBySelector %s (ignore %d)", aSelector, mIgnoreDoCommand);
#endif
  if (!mIgnoreDoCommand)
    [super doCommandBySelector:aSelector];
}

- (void) setMarkedText:(id)aString selectedRange:(NSRange)selRange
{
#if DEBUG_IME 
  NSLog(@"****in setMarkedText location: %d, length: %d", selRange.location, selRange.length);
  NSLog(@" markRange = %d, %d;  selRange = %d, %d", mMarkedRange.location, mMarkedRange.length, mSelectedRange.location, mSelectedRange.length);
  NSLog(@" aString = '%@'", aString);
#endif

  if ( ![aString isKindOfClass:[NSAttributedString class]] )
    aString = [[[NSAttributedString alloc] initWithString:aString] autorelease];

  mSelectedRange = selRange;

  NSMutableAttributedString *mutableAttribStr = aString;
  NSString *tmpStr = [mutableAttribStr string];
  unsigned int len = [tmpStr length];
  PRUnichar buffer[MAX_BUFFER_SIZE];
  PRUnichar *bufPtr = (len >= MAX_BUFFER_SIZE) ? new PRUnichar[len + 1] : buffer;
  [tmpStr getCharacters: bufPtr];
  bufPtr[len] = (PRUnichar)'\0';

#if DEBUG_IME 
  printf("****in setMarkedText, len = %d, text = ", len);
  PRUint32 n = 0;
  PRUint32 maxlen = len > 12 ? 12 : len;
  for (PRUnichar *a = bufPtr; (*a != (PRUnichar)'\0') && n<maxlen; a++, n++) printf((*a&0xff80) ? "\\u%4X" : "%c", *a); 
  printf("\n");
#endif

  mMarkedRange.location = 0;
  mMarkedRange.length = len;

  if (!mInComposition)
  {
    [self sendCompositionEvent:NS_COMPOSITION_START];
    mInComposition = YES;
  }

  [self sendTextEvent:bufPtr attributedString:aString
                             selectedRange:selRange
                             markedRange:mMarkedRange
                             doCommit:NO];

  if (mInComposition && len == 0)
    [self unmarkText];
  
  if (bufPtr != buffer)
    delete[] bufPtr;
}

- (void) unmarkText
{
#if DEBUG_IME
  NSLog(@"****in unmarkText");
  NSLog(@" markedRange   = %d, %d", mMarkedRange.location, mMarkedRange.length);
  NSLog(@" selectedRange = %d, %d", mSelectedRange.location, mSelectedRange.length);
#endif

  mSelectedRange = mMarkedRange = NSMakeRange(NSNotFound, 0);
  if (mInComposition) {
    [self sendCompositionEvent: NS_COMPOSITION_END];
    mInComposition = NO;  // brade: do we need to send an end composition event?
  }
}

- (BOOL) hasMarkedText
{
  return (mMarkedRange.location != NSNotFound) && (mMarkedRange.length != 0);
}

- (long) conversationIdentifier
{
  return (long)self;
}

- (NSAttributedString *) attributedSubstringFromRange:(NSRange)theRange
{
#if DEBUG_IME
  NSLog(@"****in attributedSubstringFromRange");
  NSLog(@" theRange      = %d, %d", theRange.location, theRange.length);
  NSLog(@" markedRange   = %d, %d", mMarkedRange.location, mMarkedRange.length);
  NSLog(@" selectedRange = %d, %d", mSelectedRange.location, mSelectedRange.length);
#endif

  nsReconversionEvent reconversionEvent(PR_TRUE, NS_RECONVERSION_QUERY,
                                        mGeckoChild);
  reconversionEvent.time = PR_IntervalNow();

  nsresult rv = mGeckoChild->DispatchWindowEvent(reconversionEvent);
  PRUnichar* reconvstr;
  if (NS_SUCCEEDED(rv) && (reconvstr = reconversionEvent.theReply.mReconversionString))
  {
    NSAttributedString* result = [[[NSAttributedString alloc] initWithString:[NSString stringWithCharacters:reconvstr length:nsCRT::strlen(reconvstr)]
                                                                  attributes:nil] autorelease];
    nsMemory::Free(reconvstr);
    return result;
  }

  return nil;
}

- (NSRange) markedRange
{
#if DEBUG_IME
  NSLog(@"****in markedRange");
  NSLog(@" markedRange   = %d, %d", mMarkedRange.location, mMarkedRange.length);
  NSLog(@" selectedRange = %d, %d", mSelectedRange.location, mSelectedRange.length);
#endif

  if (![self hasMarkedText]) {
    return NSMakeRange(NSNotFound, 0);
  }

  return mMarkedRange;
}

- (NSRange) selectedRange
{
#if DEBUG_IME
  NSLog(@"****in selectedRange");
  NSLog(@" markedRange   = %d, %d", mMarkedRange.location, mMarkedRange.length);
  NSLog(@" selectedRange = %d, %d", mSelectedRange.location, mSelectedRange.length);
#endif

  return mSelectedRange;
}


- (NSRect) firstRectForCharacterRange:(NSRange)theRange
{
#if DEBUG_IME
  NSLog(@"****in firstRectForCharacterRange");
  NSLog(@" theRange      = %d, %d", theRange.location, theRange.length);
  NSLog(@" markedRange   = %d, %d", mMarkedRange.location, mMarkedRange.length);
  NSLog(@" selectedRange = %d, %d", mSelectedRange.location, mSelectedRange.length);
#endif

  nsRect compositionRect = [self sendCompositionEvent:NS_COMPOSITION_QUERY];

  NSRect rangeRect;
  ConvertGeckoToFlippedCocoaRect(compositionRect, rangeRect);

  // convert to window coords
  rangeRect = [self convertRect:rangeRect toView:nil];
  // convert to cocoa screen coords
  rangeRect.origin = [[self getNativeWindow] convertBaseToScreen:rangeRect.origin];
  return rangeRect;
}


- (unsigned int)characterIndexForPoint:(NSPoint)thePoint
{
#if DEBUG_IME
  NSLog(@"****in characterIndexForPoint");
  NSLog(@" markRange = %d, %d;  selectRange = %d, %d", mMarkedRange.location, mMarkedRange.length, mSelectedRange.location, mSelectedRange.length);
#endif

  // To implement this, we'd have to grovel in text frames looking at text offsets.
  return 0;
}


// need to declare this because AppKit does not make it available as API or SPI
//static NSString* const NSMarkedClauseSegmentAttributeName = @"NSMarkedClauseSegment"; 
//static NSString* const NSTextInputReplacementRangeAttributeName = @"NSTextInputReplacementRangeAttributeName"; 

- (NSArray*) validAttributesForMarkedText
{
#if DEBUG_IME
  NSLog(@"****in validAttributesForMarkedText");
  NSLog(@" markRange = %d, %d;  selectRange = %d, %d", mMarkedRange.location, mMarkedRange.length, mSelectedRange.location, mSelectedRange.length);
#endif

  //return [NSArray arrayWithObjects:NSUnderlineStyleAttributeName, NSMarkedClauseSegmentAttributeName, NSTextInputReplacementRangeAttributeName, nil];
  return [NSArray array]; // empty array; we don't support any attributes right now
}
// end NSTextInput

//
// keyDown:
//
// Handle matching cocoa IME with gecko key events. Sends a key down and key press
// event to gecko.
//
- (void)keyDown:(NSEvent*)theEvent
{
  PRBool isKeyDownEventHandled = PR_TRUE;
  PRBool isKeyEventHandled = PR_FALSE;
  BOOL  isARepeat = [theEvent isARepeat];

  mCurKeyEvent = theEvent;
  
  // if we have a dead-key event, we won't get a character
  // since we have no character, there isn't any point to generating
  // a gecko event until they have dead key events
  BOOL nonDeadKeyPress = [[theEvent characters] length] > 0;
  if (!isARepeat && nonDeadKeyPress)
  {
    // Fire a key down. We'll fire key presses via -insertText:
    nsKeyEvent geckoEvent(PR_TRUE, 0, nsnull);
    geckoEvent.refPoint.x = geckoEvent.refPoint.y = 0;
    [self convertKeyEvent:theEvent
                  message:NS_KEY_DOWN
             toGeckoEvent:&geckoEvent];

    // XXX Maybe we should only do this when there is a plugin present.
    EventRecord macEvent;
    ConvertCocoaKeyEventToMacEvent(theEvent, macEvent);
    geckoEvent.nativeMsg = &macEvent;
    isKeyDownEventHandled = mGeckoChild->DispatchWindowEvent(geckoEvent);
  }
  
  // Check to see if we are still the first responder.
  // The key down event may have shifted the focus, in which
  // case we should not fire the key press.
  NSResponder* resp = [[self window] firstResponder];
  if (resp != (NSResponder*)self) {
#if DEBUG
    printf("We are no longer the responder. Bailing.\n");
#endif
    mCurKeyEvent = nil;
    return;
  }
    
  if (nonDeadKeyPress)
  {
    nsKeyEvent geckoEvent(PR_TRUE, 0, nsnull);
    geckoEvent.refPoint.x = geckoEvent.refPoint.y = 0;

    [self convertKeyEvent:theEvent
                  message:NS_KEY_PRESS
             toGeckoEvent:&geckoEvent];
    
    // if this is a non-letter keypress, or the control key is down,
    // dispatch the keydown to gecko, so that we trap delete,
    // control-letter combinations etc before Cocoa tries to use
    // them for keybindings.
    if ((!geckoEvent.isChar || geckoEvent.isControl) && !mInComposition)
    {
      // plugins need a native event, it will either be keyDown or autoKey
      EventRecord macEvent;
      ConvertCocoaKeyEventToMacEvent(theEvent, macEvent);
      geckoEvent.nativeMsg = &macEvent;
      
      isKeyEventHandled = mGeckoChild->DispatchWindowEvent(geckoEvent);
      mIgnoreDoCommand = isKeyEventHandled;
    }
  }

  [super interpretKeyEvents:[NSArray arrayWithObject:theEvent]];

  mIgnoreDoCommand = NO;
  mCurKeyEvent = nil;
}

- (void)keyUp:(NSEvent*)theEvent
{
  // if we don't have any characters we can't generate a keyUp event
  if (0 == [[theEvent characters] length])
    return;

  // Fire a key up.
  nsKeyEvent geckoEvent(PR_TRUE, 0, nsnull);
  geckoEvent.refPoint.x = geckoEvent.refPoint.y = 0;

  [self convertKeyEvent:theEvent
                message:NS_KEY_UP
           toGeckoEvent:&geckoEvent];

  // As an optimisation, only do this when there is a plugin present.
  EventRecord macEvent;
  ConvertCocoaKeyEventToMacEvent(theEvent, macEvent);
  geckoEvent.nativeMsg = &macEvent;

  mGeckoChild->DispatchWindowEvent(geckoEvent);
}

// look for the user's pressing of command and alt so that we can display
// the hand scroll cursor
- (void)flagsChanged:(NSEvent*)theEvent
{
  BOOL inMouseView = NO;
  // check to see if the user has hand scroll modifiers held down; if so, 
  // find out if the cursor is in an ChildView
  if ([ChildView areHandScrollModifiers:[theEvent modifierFlags]]) {
    NSPoint pointInWindow = [[self window] mouseLocationOutsideOfEventStream];

    NSView* mouseView = [[[self window] contentView] hitTest:pointInWindow];
    inMouseView = (mouseView != nil && [mouseView isMemberOfClass:[ChildView class]]);   
  }
  if (inMouseView) {
      mGeckoChild->SetCursor(eCursor_grab);
  } else {
    nsCursor cursor = mGeckoChild->GetCursor();
    if (!mInHandScroll) {
      if (cursor == eCursor_grab || cursor == eCursor_grabbing)
        mGeckoChild->SetCursor(eCursor_standard);
      // pass on the event since we are not using it
      [super flagsChanged:theEvent];
    }
  }
}

// This method is called when we are about to be focused.
- (BOOL)becomeFirstResponder
{
  if (!mGeckoChild) return NO;   // we've been destroyed

  nsFocusEvent event(PR_TRUE, NS_GOTFOCUS, mGeckoChild);
  mGeckoChild->DispatchWindowEvent(event);

  return [super becomeFirstResponder];
}

// This method is called when are are about to lose focus.
- (BOOL)resignFirstResponder
{
  if (!mGeckoChild) return NO;   // we've been destroyed

  nsFocusEvent event(PR_TRUE, NS_LOSTFOCUS, mGeckoChild);
  mGeckoChild->DispatchWindowEvent(event);

  return [super resignFirstResponder];
}

- (void)viewsWindowDidBecomeKey
{
  if (!mGeckoChild)
    return;   // we've been destroyed (paranoia)
  
  // check to see if the window implements the mozWindow protocol. This
  // allows embedders to avoid re-entrant calls to -makeKeyAndOrderFront,
  // which can happen because these activate/focus calls propagate out
  // to the embedder via nsIEmbeddingSiteWindow::SetFocus().
  BOOL isMozWindow = [[self window] respondsToSelector:@selector(setSuppressMakeKeyFront:)];
  if (isMozWindow)
    [[self window] setSuppressMakeKeyFront:YES];

  nsFocusEvent focusEvent(PR_TRUE, NS_GOTFOCUS, mGeckoChild);
  mGeckoChild->DispatchWindowEvent(focusEvent);

  nsFocusEvent activateEvent(PR_TRUE, NS_ACTIVATE, mGeckoChild);
  mGeckoChild->DispatchWindowEvent(activateEvent);

  if (isMozWindow)
    [[self window] setSuppressMakeKeyFront:NO];
}

- (void)viewsWindowDidResignKey
{
  if (!mGeckoChild)
    return;   // we've been destroyed
  
  nsFocusEvent deactivateEvent(PR_TRUE, NS_DEACTIVATE, mGeckoChild);
  mGeckoChild->DispatchWindowEvent(deactivateEvent);

  nsFocusEvent unfocusEvent(PR_TRUE, NS_LOSTFOCUS, mGeckoChild);
  mGeckoChild->DispatchWindowEvent(unfocusEvent);
}

//-------------------------------------------------------------------------
//
// ConvertMacToRaptorKeyCode
//
//-------------------------------------------------------------------------


// Key code constants
enum
{
  kEscapeKeyCode      = 0x35,
  kCommandKeyCode     = 0x37,
  kShiftKeyCode       = 0x38,
  kCapsLockKeyCode    = 0x39,
  kControlKeyCode     = 0x3B,
  kOptionkeyCode      = 0x3A,   // left and right option keys
  kClearKeyCode       = 0x47,
  
  // function keys
  kF1KeyCode          = 0x7A,
  kF2KeyCode          = 0x78,
  kF3KeyCode          = 0x63,
  kF4KeyCode          = 0x76,
  kF5KeyCode          = 0x60,
  kF6KeyCode          = 0x61,
  kF7KeyCode          = 0x62,
  kF8KeyCode          = 0x64,
  kF9KeyCode          = 0x65,
  kF10KeyCode         = 0x6D,
  kF11KeyCode         = 0x67,
  kF12KeyCode         = 0x6F,
  kF13KeyCode         = 0x69,
  kF14KeyCode         = 0x6B,
  kF15KeyCode         = 0x71,
  
  kPrintScreenKeyCode = kF13KeyCode,
  kScrollLockKeyCode  = kF14KeyCode,
  kPauseKeyCode       = kF15KeyCode,
  
  // keypad
  kKeypad0KeyCode     = 0x52,
  kKeypad1KeyCode     = 0x53,
  kKeypad2KeyCode     = 0x54,
  kKeypad3KeyCode     = 0x55,
  kKeypad4KeyCode     = 0x56,
  kKeypad5KeyCode     = 0x57,
  kKeypad6KeyCode     = 0x58,
  kKeypad7KeyCode     = 0x59,
  kKeypad8KeyCode     = 0x5B,
  kKeypad9KeyCode     = 0x5C,
  
  kKeypadMultiplyKeyCode  = 0x43,
  kKeypadAddKeyCode       = 0x45,
  kKeypadSubtractKeyCode  = 0x4E,
  kKeypadDecimalKeyCode   = 0x41,
  kKeypadDivideKeyCode    = 0x4B,
  kKeypadEqualsKeyCode    = 0x51,     // no correpsonding raptor key code
  kEnterKeyCode           = 0x4C,
  kReturnKeyCode          = 0x24,
  kPowerbookEnterKeyCode  = 0x34,     // Enter on Powerbook's keyboard is different
  
  kInsertKeyCode          = 0x72,       // also help key
  kDeleteKeyCode          = 0x75,       // also forward delete key
  kTabKeyCode             = 0x30,
  kBackspaceKeyCode       = 0x33,
  kHomeKeyCode            = 0x73, 
  kEndKeyCode             = 0x77,
  kPageUpKeyCode          = 0x74,
  kPageDownKeyCode        = 0x79,
  kLeftArrowKeyCode       = 0x7B,
  kRightArrowKeyCode      = 0x7C,
  kUpArrowKeyCode         = 0x7E,
  kDownArrowKeyCode       = 0x7D
  
};

static PRUint32 ConvertMacToRaptorKeyCode(UInt32 keyCode, nsKeyEvent* aKeyEvent, NSString* characters)
{
  PRUint32 raptorKeyCode = 0;
  PRUint8 charCode;
  if ([characters length])
    charCode = [characters characterAtIndex: 0];
  else
    charCode = 0;

  switch (keyCode)
  {
//  case ??             :       raptorKeyCode = NS_VK_CANCEL;   break;      // don't know what this key means. Nor does joki

// modifiers. We don't get separate events for these
    case kEscapeKeyCode:        raptorKeyCode = NS_VK_ESCAPE;         break;
    case kShiftKeyCode:         raptorKeyCode = NS_VK_SHIFT;          break;
//    case kCommandKeyCode:       raptorKeyCode = NS_VK_META;           break;
    case kCapsLockKeyCode:      raptorKeyCode = NS_VK_CAPS_LOCK;      break;
    case kControlKeyCode:       raptorKeyCode = NS_VK_CONTROL;        break;
    case kOptionkeyCode:        raptorKeyCode = NS_VK_ALT;            break;
    case kClearKeyCode:         raptorKeyCode = NS_VK_CLEAR;          break;

// function keys
    case kF1KeyCode:            raptorKeyCode = NS_VK_F1;             break;
    case kF2KeyCode:            raptorKeyCode = NS_VK_F2;             break;
    case kF3KeyCode:            raptorKeyCode = NS_VK_F3;             break;
    case kF4KeyCode:            raptorKeyCode = NS_VK_F4;             break;
    case kF5KeyCode:            raptorKeyCode = NS_VK_F5;             break;
    case kF6KeyCode:            raptorKeyCode = NS_VK_F6;             break;
    case kF7KeyCode:            raptorKeyCode = NS_VK_F7;             break;
    case kF8KeyCode:            raptorKeyCode = NS_VK_F8;             break;
    case kF9KeyCode:            raptorKeyCode = NS_VK_F9;             break;
    case kF10KeyCode:           raptorKeyCode = NS_VK_F10;            break;
    case kF11KeyCode:           raptorKeyCode = NS_VK_F11;            break;
    case kF12KeyCode:           raptorKeyCode = NS_VK_F12;            break;
//  case kF13KeyCode:           raptorKeyCode = NS_VK_F13;            break;    // clash with the 3 below
//  case kF14KeyCode:           raptorKeyCode = NS_VK_F14;            break;
//  case kF15KeyCode:           raptorKeyCode = NS_VK_F15;            break;
    case kPauseKeyCode:         raptorKeyCode = NS_VK_PAUSE;          break;
    case kScrollLockKeyCode:    raptorKeyCode = NS_VK_SCROLL_LOCK;    break;
    case kPrintScreenKeyCode:   raptorKeyCode = NS_VK_PRINTSCREEN;    break;
  
// keypad
    case kKeypad0KeyCode:       raptorKeyCode = NS_VK_NUMPAD0;        break;
    case kKeypad1KeyCode:       raptorKeyCode = NS_VK_NUMPAD1;        break;
    case kKeypad2KeyCode:       raptorKeyCode = NS_VK_NUMPAD2;        break;
    case kKeypad3KeyCode:       raptorKeyCode = NS_VK_NUMPAD3;        break;
    case kKeypad4KeyCode:       raptorKeyCode = NS_VK_NUMPAD4;        break;
    case kKeypad5KeyCode:       raptorKeyCode = NS_VK_NUMPAD5;        break;
    case kKeypad6KeyCode:       raptorKeyCode = NS_VK_NUMPAD6;        break;
    case kKeypad7KeyCode:       raptorKeyCode = NS_VK_NUMPAD7;        break;
    case kKeypad8KeyCode:       raptorKeyCode = NS_VK_NUMPAD8;        break;
    case kKeypad9KeyCode:       raptorKeyCode = NS_VK_NUMPAD9;        break;

    case kKeypadMultiplyKeyCode:  raptorKeyCode = NS_VK_MULTIPLY;     break;
    case kKeypadAddKeyCode:       raptorKeyCode = NS_VK_ADD;          break;
    case kKeypadSubtractKeyCode:  raptorKeyCode = NS_VK_SUBTRACT;     break;
    case kKeypadDecimalKeyCode:   raptorKeyCode = NS_VK_DECIMAL;      break;
    case kKeypadDivideKeyCode:    raptorKeyCode = NS_VK_DIVIDE;       break;
//  case ??               :       raptorKeyCode = NS_VK_SEPARATOR;    break;


// these may clash with forward delete and help
    case kInsertKeyCode:        raptorKeyCode = NS_VK_INSERT;         break;
    case kDeleteKeyCode:        raptorKeyCode = NS_VK_DELETE;         break;

    case kBackspaceKeyCode:     raptorKeyCode = NS_VK_BACK;           break;
    case kTabKeyCode:           raptorKeyCode = NS_VK_TAB;            break;
    case kHomeKeyCode:          raptorKeyCode = NS_VK_HOME;           break;
    case kEndKeyCode:           raptorKeyCode = NS_VK_END;            break;
    case kPageUpKeyCode:        raptorKeyCode = NS_VK_PAGE_UP;        break;
    case kPageDownKeyCode:      raptorKeyCode = NS_VK_PAGE_DOWN;      break;
    case kLeftArrowKeyCode:     raptorKeyCode = NS_VK_LEFT;           break;
    case kRightArrowKeyCode:    raptorKeyCode = NS_VK_RIGHT;          break;
    case kUpArrowKeyCode:       raptorKeyCode = NS_VK_UP;             break;
    case kDownArrowKeyCode:     raptorKeyCode = NS_VK_DOWN;           break;

    default:
        if (aKeyEvent->isControl)
          charCode += 64;
      
        // if we haven't gotten the key code already, look at the char code
        switch (charCode)
        {
          case kReturnCharCode:       raptorKeyCode = NS_VK_RETURN;       break;
          case kEnterCharCode:        raptorKeyCode = NS_VK_RETURN;       break;      // fix me!
          case ' ':                   raptorKeyCode = NS_VK_SPACE;        break;
          case ';':                   raptorKeyCode = NS_VK_SEMICOLON;    break;
          case '=':                   raptorKeyCode = NS_VK_EQUALS;       break;
          case ',':                   raptorKeyCode = NS_VK_COMMA;        break;
          case '.':                   raptorKeyCode = NS_VK_PERIOD;       break;
          case '/':                   raptorKeyCode = NS_VK_SLASH;        break;
          case '`':                   raptorKeyCode = NS_VK_BACK_QUOTE;   break;
          case '{':
          case '[':                   raptorKeyCode = NS_VK_OPEN_BRACKET; break;
          case '\\':                  raptorKeyCode = NS_VK_BACK_SLASH;   break;
          case '}':
          case ']':                   raptorKeyCode = NS_VK_CLOSE_BRACKET;  break;
          case '\'':
          case '"':                   raptorKeyCode = NS_VK_QUOTE;        break;
          
          default:
            
            if (charCode >= '0' && charCode <= '9')   // numerals
            {
              raptorKeyCode = charCode;
            }
            else if (charCode >= 'a' && charCode <= 'z')    // lowercase
            {
              raptorKeyCode = toupper(charCode);
            }
            else if (charCode >= 'A' && charCode <= 'Z')    // uppercase
            {
              raptorKeyCode = charCode;
            }

            break;
        }
  }

  return raptorKeyCode;
}

static PRBool IsSpecialRaptorKey(UInt32 macKeyCode)
{
  PRBool  isSpecial;

  // 
  // this table is used to determine which keys are special and should not generate a charCode
  //  
  switch (macKeyCode)
  {
// modifiers. We don't get separate events for these
// yet
    case kEscapeKeyCode:        isSpecial = PR_TRUE; break;
    case kShiftKeyCode:         isSpecial = PR_TRUE; break;
    case kCommandKeyCode:       isSpecial = PR_TRUE; break;
    case kCapsLockKeyCode:      isSpecial = PR_TRUE; break;
    case kControlKeyCode:       isSpecial = PR_TRUE; break;
    case kOptionkeyCode:        isSpecial = PR_TRUE; break;
    case kClearKeyCode:         isSpecial = PR_TRUE; break;

// function keys
    case kF1KeyCode:            isSpecial = PR_TRUE; break;
    case kF2KeyCode:            isSpecial = PR_TRUE; break;
    case kF3KeyCode:            isSpecial = PR_TRUE; break;
    case kF4KeyCode:            isSpecial = PR_TRUE; break;
    case kF5KeyCode:            isSpecial = PR_TRUE; break;
    case kF6KeyCode:            isSpecial = PR_TRUE; break;
    case kF7KeyCode:            isSpecial = PR_TRUE; break;
    case kF8KeyCode:            isSpecial = PR_TRUE; break;
    case kF9KeyCode:            isSpecial = PR_TRUE; break;
    case kF10KeyCode:           isSpecial = PR_TRUE; break;
    case kF11KeyCode:           isSpecial = PR_TRUE; break;
    case kF12KeyCode:           isSpecial = PR_TRUE; break;
    case kPauseKeyCode:         isSpecial = PR_TRUE; break;
    case kScrollLockKeyCode:    isSpecial = PR_TRUE; break;
    case kPrintScreenKeyCode:   isSpecial = PR_TRUE; break;

    case kInsertKeyCode:        isSpecial = PR_TRUE; break;
    case kDeleteKeyCode:        isSpecial = PR_TRUE; break;
    case kTabKeyCode:           isSpecial = PR_TRUE; break;
    case kBackspaceKeyCode:     isSpecial = PR_TRUE; break;

    case kHomeKeyCode:          isSpecial = PR_TRUE; break; 
    case kEndKeyCode:           isSpecial = PR_TRUE; break;
    case kPageUpKeyCode:        isSpecial = PR_TRUE; break;
    case kPageDownKeyCode:      isSpecial = PR_TRUE; break;
    case kLeftArrowKeyCode:     isSpecial = PR_TRUE; break;
    case kRightArrowKeyCode:    isSpecial = PR_TRUE; break;
    case kUpArrowKeyCode:       isSpecial = PR_TRUE; break;
    case kDownArrowKeyCode:     isSpecial = PR_TRUE; break;
    case kReturnKeyCode:        isSpecial = PR_TRUE; break;
    case kEnterKeyCode:         isSpecial = PR_TRUE; break;
    case kPowerbookEnterKeyCode: isSpecial = PR_TRUE; break;

    default:                    isSpecial = PR_FALSE; break;
  }
  return isSpecial;
}

- (void) convertKeyEvent:(NSEvent*)aKeyEvent message:(PRUint32)aMessage 
            toGeckoEvent:(nsKeyEvent*)outGeckoEvent
{
  [self convertEvent:aKeyEvent message:aMessage toGeckoEvent:outGeckoEvent];

  // Initialize whether or not we are using charCodes to false.
  outGeckoEvent->isChar = PR_FALSE;
    
  // Check to see if the message is a key press that does not involve
  // one of our special key codes.
  if (aMessage == NS_KEY_PRESS && !IsSpecialRaptorKey([aKeyEvent keyCode])) 
  {
    if (!outGeckoEvent->isControl && !outGeckoEvent->isMeta)
      outGeckoEvent->isControl = outGeckoEvent->isAlt = outGeckoEvent->isMeta = 0;
    
    outGeckoEvent->charCode = 0;
    outGeckoEvent->keyCode  = 0;

    NSString* unmodifiedChars = [aKeyEvent charactersIgnoringModifiers];
    if ([unmodifiedChars length] > 0)
      outGeckoEvent->charCode = [unmodifiedChars characterAtIndex:0];
    
    // We're not a special key.
    outGeckoEvent->isChar = PR_TRUE;

    // convert control-modified charCode to raw charCode (with appropriate case)
    if (outGeckoEvent->isControl && outGeckoEvent->charCode <= 26)
      outGeckoEvent->charCode += (outGeckoEvent->isShift) ? ('A' - 1) : ('a' - 1);

    // gecko also wants charCode to be in the appropriate case
    if (outGeckoEvent->isShift && (outGeckoEvent->charCode >= 'a' && outGeckoEvent->charCode <= 'z'))
      outGeckoEvent->charCode -= 32;    // convert to uppercase
  }
  else
  {
    outGeckoEvent->keyCode = ConvertMacToRaptorKeyCode([aKeyEvent keyCode], outGeckoEvent, [aKeyEvent characters]);
    outGeckoEvent->charCode = 0;
  } 
  
  if (aMessage == NS_KEY_PRESS && !outGeckoEvent->isMeta && outGeckoEvent->keyCode != NS_VK_PAGE_UP && 
      outGeckoEvent->keyCode != NS_VK_PAGE_DOWN)
    ::ObscureCursor();
}

//
// -_destinationFloatValueForScroller
//
// When smooth scrolling is turned on on panther, the parent of a scrollbar (which
// I guess they assume is a NSScrollView) gets called with this method. I have no
// idea what the correct return value is, but we have to have this otherwise the scrollbar
// will not continuously respond when the mouse is held down in the pageup/down area.
//
-(float)_destinationFloatValueForScroller:(id)scroller
{
  return [scroller floatValue];
}

@end
