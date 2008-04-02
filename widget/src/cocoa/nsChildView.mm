/* -*- Mode: objc; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
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
 *   Håkan Waara <hwaara@gmail.com>
 *   Stuart Morgan <stuart.morgan@alumni.case.edu>
 *   Mats Palmgren <mats.palmgren@bredband.net>
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
#include "nsCocoaWindow.h"

#include "nsObjCExceptions.h"
#include "nsCOMPtr.h"
#include "nsToolkit.h"
#include "nsCRT.h"
#include "nsplugindefs.h"
#include "nsIPrefService.h"
#include "nsIPrefBranch.h"

#include "nsIFontMetrics.h"
#include "nsIDeviceContext.h"
#include "nsIRegion.h"
#include "nsIRollupListener.h"
#include "nsIScrollableView.h"
#include "nsIViewManager.h"
#include "nsIInterfaceRequestor.h"
#include "nsIServiceManager.h"
#include "nsILocalFile.h"
#include "nsILocalFileMac.h"
#include "nsGfxCIID.h"
#include "nsIMenuRollup.h"

#include "nsDragService.h"
#include "nsCursorManager.h"
#include "nsWindowMap.h"
#include "nsCocoaUtils.h"
#include "nsMenuBarX.h"

#include "gfxContext.h"
#include "gfxQuartzSurface.h"

#undef DEBUG_IME
#undef DEBUG_UPDATE
#undef INVALIDATE_DEBUGGING  // flash areas as they are invalidated

#ifdef MOZ_LOGGING
#define FORCE_PR_LOG
#endif
#include "prlog.h"

#ifdef PR_LOGGING
PRLogModuleInfo* sCocoaLog = nsnull;
#endif

// npapi.h defines NPEventType_AdjustCursorEvent but we don't want to include npapi.h here.
// We need to send this in the "what" field for certain native plugin events. WebKit does
// this as well.
#define adjustCursorEvent 33

extern "C" {
  CG_EXTERN void CGContextResetCTM(CGContextRef);
  CG_EXTERN void CGContextSetCTM(CGContextRef, CGAffineTransform);
  CG_EXTERN void CGContextResetClip(CGContextRef);
}

extern PRBool gCocoaWindowMethodsSwizzled; // Defined in nsCocoaWindow.mm

extern nsISupportsArray *gDraggedTransferables;

PRBool nsTSMManager::sIsIMEEnabled = PR_TRUE;
PRBool nsTSMManager::sIsRomanKeyboardsOnly = PR_FALSE;
PRBool nsTSMManager::sIgnoreCommit = PR_FALSE;
NSView<mozView>* nsTSMManager::sComposingView = nsnull;
TSMDocumentID nsTSMManager::sDocumentID = nsnull;
NSString* nsTSMManager::sComposingString = nsnull;

static NS_DEFINE_CID(kRegionCID, NS_REGION_CID);
static NSView* sLastViewEntered = nil;
#ifdef INVALIDATE_DEBUGGING
static void blinkRect(Rect* r);
static void blinkRgn(RgnHandle rgn);
#endif

nsIRollupListener * gRollupListener = nsnull;
nsIWidget         * gRollupWidget   = nsnull;


@interface ChildView(Private)

// sets up our view, attaching it to its owning gecko view
- (id)initWithFrame:(NSRect)inFrame geckoChild:(nsChildView*)inChild;

// sends gecko an ime composition event
- (nsRect) sendCompositionEvent:(PRInt32)aEventType;

// sends gecko an ime text event
- (void) sendTextEvent:(PRUnichar*) aBuffer 
                       attributedString:(NSAttributedString*) aString
                       selectedRange:(NSRange)selRange
                       markedRange:(NSRange)markRange
                       doCommit:(BOOL)doCommit;

// do generic gecko event setup with a generic cocoa event. accepts nil inEvent.
- (void) convertGenericCocoaEvent:(NSEvent*)inEvent toGeckoEvent:(nsInputEvent*)outGeckoEvent;

// set up a gecko mouse event based on a cocoa mouse event
- (void) convertCocoaMouseEvent:(NSEvent*)aMouseEvent toGeckoEvent:(nsInputEvent*)outGeckoEvent;

// set up a gecko key event based on a cocoa key event
- (void) convertCocoaKeyEvent:(NSEvent*)aKeyEvent toGeckoEvent:(nsKeyEvent*)outGeckoEvent;

- (NSMenu*)contextMenu;
- (TopLevelWindowData*)ensureWindowData;

- (void)setIsPluginView:(BOOL)aIsPlugin;
- (BOOL)isPluginView;

- (BOOL)childViewHasPlugin;

- (BOOL)isRectObscuredBySubview:(NSRect)inRect;

- (void)processPendingRedraws;

- (BOOL)ensureCorrectMouseEventTarget:(NSEvent *)anEvent;

- (void)maybeInitContextMenuTracking;

#if USE_CLICK_HOLD_CONTEXTMENU
 // called on a timer two seconds after a mouse down to see if we should display
 // a context menu (click-hold)
- (void)clickHoldCallback:(id)inEvent;
#endif

#ifdef ACCESSIBILITY
- (id<mozAccessible>)accessible;
#endif

@end


// Used to retain an NSView for the remainder of a method's execution.
class nsAutoRetainView {
public:
  nsAutoRetainView(NSView *aView)
  {
    mView = NS_OBJC_TRY_EXPR_ABORT([aView retain]);
  }
  ~nsAutoRetainView()
  {
    NS_OBJC_TRY_ABORT([mView release]);
  }

private:
  NSView *mView;  // [STRONG]
};


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
GeckoRectToNSRect(const nsRect & inGeckoRect, NSRect & outCocoaRect)
{
  outCocoaRect.origin.x = inGeckoRect.x;
  outCocoaRect.origin.y = inGeckoRect.y;
  outCocoaRect.size.width = inGeckoRect.width;
  outCocoaRect.size.height = inGeckoRect.height;
}

static inline void
NSRectToGeckoRect(const NSRect & inCocoaRect, nsRect & outGeckoRect)
{
  outGeckoRect.x = static_cast<nscoord>(inCocoaRect.origin.x);
  outGeckoRect.y = static_cast<nscoord>(inCocoaRect.origin.y);
  outGeckoRect.width = static_cast<nscoord>(inCocoaRect.size.width);
  outGeckoRect.height = static_cast<nscoord>(inCocoaRect.size.height);
}


static inline void 
ConvertGeckoRectToMacRect(const nsRect& aRect, Rect& outMacRect)
{
  outMacRect.left = aRect.x;
  outMacRect.top = aRect.y;
  outMacRect.right = aRect.x + aRect.width;
  outMacRect.bottom = aRect.y + aRect.height;
}

// Flips a screen coordinate from a point in the cocoa coordinate system (bottom-left rect) to a point
// that is a "flipped" cocoa coordinate system (starts in the top-left).
static inline void
FlipCocoaScreenCoordinate (NSPoint &inPoint)
{  
  inPoint.y = nsCocoaUtils::FlippedScreenY(inPoint.y);
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
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK_RETURN;

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

  NS_OBJC_END_TRY_ABORT_BLOCK_RETURN(0);
}


static void
ConvertAttributeToGeckoRange(NSAttributedString *aString, NSRange markRange, NSRange selRange, PRUint32 inCount, nsTextRange* aRanges)
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK;

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

  NS_OBJC_END_TRY_ABORT_BLOCK;
}


static void
FillTextRangeInTextEvent(nsTextEvent *aTextEvent, NSAttributedString* aString, NSRange markRange, NSRange selRange)
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK;

  // Count the number of segments in the attributed string and add one more count for sending current caret position to Gecko.
  // Allocate the right size of nsTextRange and draw caret at right position.
  // Convert the attributed string into an array of nsTextRange and get current caret position by calling above functions.
  PRUint32 count = CountRanges(aString) + 1;
  aTextEvent->rangeArray = new nsTextRange[count];
  if (aTextEvent->rangeArray) {
    aTextEvent->rangeCount = count;
    ConvertAttributeToGeckoRange(aString, markRange, selRange, aTextEvent->rangeCount,  aTextEvent->rangeArray);
  }

  NS_OBJC_END_TRY_ABORT_BLOCK;
}

#pragma mark -


nsChildView::nsChildView() : nsBaseWidget()
, mView(nsnull)
, mParentView(nsnull)
, mParentWidget(nsnull)
, mVisible(PR_FALSE)
, mDrawing(PR_FALSE)
, mLiveResizeInProgress(PR_FALSE)
, mIsPluginView(PR_FALSE)
, mPluginDrawing(PR_FALSE)
, mPluginIsCG(PR_FALSE)
, mInSetFocus(PR_FALSE)
{
#ifdef PR_LOGGING
  if (!sCocoaLog)
    sCocoaLog = PR_NewLogModule("nsCocoaWidgets");
#endif

  SetBackgroundColor(NS_RGB(255, 255, 255));
  SetForegroundColor(NS_RGB(0, 0, 0));
}


nsChildView::~nsChildView()
{
  // notify the children that we're gone
  for (nsIWidget* kid = mFirstChild; kid; kid = kid->GetNextSibling()) {
    nsChildView* childView = static_cast<nsChildView*>(kid);
    childView->mParentWidget = nsnull;
  }

  TearDownView(); // should have already been done from Destroy
}


NS_IMPL_ISUPPORTS_INHERITED2(nsChildView, nsBaseWidget, nsIPluginWidget, nsIKBStateControl)


// Utility method for implementing both Create(nsIWidget ...)
// and Create(nsNativeWidget...)
nsresult nsChildView::StandardCreate(nsIWidget *aParent,
                      const nsRect &aRect,
                      EVENT_CALLBACK aHandleEventFunction,
                      nsIDeviceContext *aContext,
                      nsIAppShell *aAppShell,
                      nsIToolkit *aToolkit,
                      nsWidgetInitData *aInitData,
                      nsNativeWidget aNativeParent)
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK_NSRESULT;

  // See NSWindow (MethodSwizzling) in nsCocoaWindow.mm.
  if (!gCocoaWindowMethodsSwizzled) {
    nsToolkit::SwizzleMethods([NSWindow class], @selector(sendEvent:),
                              @selector(nsCocoaWindow_NSWindow_sendEvent:));
    gCocoaWindowMethodsSwizzled = PR_TRUE;
  }

  mBounds = aRect;

  BaseCreate(aParent, aRect, aHandleEventFunction, 
              aContext, aAppShell, aToolkit, aInitData);

  // inherit things from the parent view and create our parallel 
  // NSView in the Cocoa display system
  mParentView = nil;
  if (aParent) {
    SetBackgroundColor(aParent->GetBackgroundColor());
    SetForegroundColor(aParent->GetForegroundColor());

    // inherit the top-level window. NS_NATIVE_WIDGET is always a NSView
    // regardless of if we're asking a window or a view (for compatibility
    // with windows).
    mParentView = (NSView*)aParent->GetNativeData(NS_NATIVE_WIDGET); 
    mParentWidget = aParent;   
  }
  else
    mParentView = reinterpret_cast<NSView*>(aNativeParent);
  
  // create our parallel NSView and hook it up to our parent. Recall
  // that NS_NATIVE_WIDGET is the NSView.
  NSRect r;
  GeckoRectToNSRect(mBounds, r);
  mView = [CreateCocoaView(r) retain];
  if (!mView) return NS_ERROR_FAILURE;
  
#if DEBUG
  // if our parent is a popup window, we're most certainly coming from a <select> list dropdown which
  // we handle in a different way than other platforms. It's ok that we don't have a parent
  // view because we bailed before even creating the cocoa widgetry and as a result, we
  // don't need to assert. However, if that's not the case, we definitely want to assert
  // to show views aren't getting correctly parented.
  if (aParent) {
    nsWindowType windowType;
    aParent->GetWindowType(windowType);
    if (windowType != eWindowType_popup)
      NS_ASSERTION(mParentView && mView, "couldn't hook up new NSView in hierarchy");
  }
  else {
    NS_ASSERTION(mParentView && mView, "couldn't hook up new NSView in hierarchy");
  }
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
        [mParentView respondsToSelector:@selector(nativeWindow)])
      window = [mParentView nativeWindow];

    [mView setNativeWindow:window];

    [mParentView addSubview:mView];
  }

  // if this is a ChildView, make sure that our per-window data
  // is set up
  if ([mView isKindOfClass:[ChildView class]])
    [(ChildView*)mView ensureWindowData];

  return NS_OK;

  NS_OBJC_END_TRY_ABORT_BLOCK_NSRESULT;
}


// Creates the appropriate child view. Override to create something other than
// our |ChildView| object. Autoreleases, so caller must retain.
NSView*
nsChildView::CreateCocoaView(NSRect inFrame)
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK_NIL;

  return [[[ChildView alloc] initWithFrame:inFrame geckoChild:this] autorelease];

  NS_OBJC_END_TRY_ABORT_BLOCK_NIL;
}


void nsChildView::TearDownView()
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK;

  if (!mView)
    return;

  NSWindow* win = [mView window];
  NSResponder* responder = [win firstResponder];
  
  // We're being unhooked from the view hierarchy, don't leave our view
  // or a child view as the window first responder.
  if (responder && [responder isKindOfClass:[NSView class]] &&
      [(NSView*)responder isDescendantOf:mView]) {
    [win makeFirstResponder:[mView superview]];
  }

  // If mView is win's contentView, win (mView's NSWindow) "owns" mView --
  // win has retained mView, and will detach it from the view hierarchy and
  // release it when necessary (when win is itself destroyed (in a call to
  // [win dealloc])).  So all we need to do here is call [mView release] (to
  // match the call to [mView retain] in nsChildView::StandardCreate()).
  // Also calling [mView removeFromSuperviewWithoutNeedingDisplay] causes
  // mView to be released again and dealloced, while remaining win's
  // contentView.  So if we do that here, win will (for a short while) have
  // an invalid contentView (for the consequences see bmo bugs 381087 and
  // 374260).
  if ([mView isEqual:[win contentView]]) {
    [mView release];
  } else {
    // Stop NSView hierarchy being changed during [ChildView drawRect:]
    [mView performSelectorOnMainThread:@selector(delayedTearDown) withObject:nil waitUntilDone:false];
  }
  mView = nil;

  NS_OBJC_END_TRY_ABORT_BLOCK;
}


// create a nsChildView
NS_IMETHODIMP nsChildView::Create(nsIWidget *aParent,
                      const nsRect &aRect,
                      EVENT_CALLBACK aHandleEventFunction,
                      nsIDeviceContext *aContext,
                      nsIAppShell *aAppShell,
                      nsIToolkit *aToolkit,
                      nsWidgetInitData *aInitData)
{  
  return(StandardCreate(aParent, aRect, aHandleEventFunction, aContext,
                        aAppShell, aToolkit, aInitData, nsnull));
}


// Creates a main nsChildView using a native widget (an NSView)
NS_IMETHODIMP nsChildView::Create(nsNativeWidget aNativeParent,
                      const nsRect &aRect,
                      EVENT_CALLBACK aHandleEventFunction,
                      nsIDeviceContext *aContext,
                      nsIAppShell *aAppShell,
                      nsIToolkit *aToolkit,
                      nsWidgetInitData *aInitData)
{
  // what we're passed in |aNativeParent| is an NSView. 
  return(StandardCreate(nsnull, aRect, aHandleEventFunction, aContext,
                        aAppShell, aToolkit, aInitData, aNativeParent));
}


NS_IMETHODIMP nsChildView::Destroy()
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK_NSRESULT;

  if (mOnDestroyCalled)
    return NS_OK;
  mOnDestroyCalled = PR_TRUE;

  [mView widgetDestroyed];

  nsBaseWidget::OnDestroy();
  nsBaseWidget::Destroy();

  ReportDestroyEvent(); 
  mParentWidget = nil;

  TearDownView();

  return NS_OK;

  NS_OBJC_END_TRY_ABORT_BLOCK_NSRESULT;
}


#pragma mark -


#if 0
static void PrintViewHierarchy(NSView *view)
{
  while (view) {
    NSLog(@"  view is %x, frame %@", view, NSStringFromRect([view frame]));
    view = [view superview];
  }
}
#endif



// Return native data according to aDataType
void* nsChildView::GetNativeData(PRUint32 aDataType)
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK_NSNULL;

  void* retVal = nsnull;

  switch (aDataType) 
  {
    case NS_NATIVE_WIDGET:
    case NS_NATIVE_DISPLAY:
      retVal = (void*)mView;
      break;

    case NS_NATIVE_WINDOW:
      retVal = [mView nativeWindow];
      break;

    case NS_NATIVE_GRAPHIC:
      NS_ASSERTION(0, "Requesting NS_NATIVE_GRAPHIC on a Mac OS X child view!");
      retVal = nsnull;
      break;

    case NS_NATIVE_OFFSETX:
      retVal = 0;
      break;

    case NS_NATIVE_OFFSETY:
      retVal = 0;
      break;

    case NS_NATIVE_PLUGIN_PORT:
#ifndef NP_NO_QUICKDRAW
    case NS_NATIVE_PLUGIN_PORT_QD:
    {
      mPluginIsCG = PR_FALSE;
      mIsPluginView = PR_TRUE;
      if ([mView isKindOfClass:[ChildView class]])
        [(ChildView*)mView setIsPluginView:YES];

      NSWindow* window = [mView nativeWindow];
      if (window) {
        WindowRef topLevelWindow = (WindowRef)[window windowRef];
        if (topLevelWindow) {
          mPluginPort.qdPort.port = ::GetWindowPort(topLevelWindow);

          NSPoint viewOrigin = [mView convertPoint:NSZeroPoint toView:nil];
          NSRect frame = [[window contentView] frame];
          viewOrigin.y = frame.size.height - viewOrigin.y;
          
          // need to convert view's origin to window coordinates.
          // then, encode as "SetOrigin" ready values.
          mPluginPort.qdPort.portx = (PRInt32)-viewOrigin.x;
          mPluginPort.qdPort.porty = (PRInt32)-viewOrigin.y;
        }
      }

      retVal = (void*)&mPluginPort;
      break;
    }
#endif

    case NS_NATIVE_PLUGIN_PORT_CG:
    {
      mPluginIsCG = PR_TRUE;
      mIsPluginView = PR_TRUE;
      if ([mView isKindOfClass:[ChildView class]])
        [(ChildView*)mView setIsPluginView:YES];

      mPluginPort.cgPort.context = (CGContextRef)[[NSGraphicsContext currentContext] graphicsPort];

      NSWindow* window = [mView nativeWindow];
      if (window) {
        WindowRef topLevelWindow = (WindowRef)[window windowRef];
        mPluginPort.cgPort.window = topLevelWindow;
      }

      retVal = (void*)&mPluginPort;
      break;
    }
  }

  return retVal;

  NS_OBJC_END_TRY_ABORT_BLOCK_NSNULL;
}

#pragma mark -

NS_IMETHODIMP nsChildView::GetHasTransparentBackground(PRBool& aTransparent)
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK_NSRESULT;

  aTransparent = ![mView isOpaque];
  return NS_OK;

  NS_OBJC_END_TRY_ABORT_BLOCK_NSRESULT;
}


// This is called by nsContainerFrame on the root widget for all window types
// except popup windows (when nsCocoaWindow::SetHasTransparentBackground is used instead).
NS_IMETHODIMP nsChildView::SetHasTransparentBackground(PRBool aTransparent)
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK_NSRESULT;

  BOOL currentTransparency = ![[mView nativeWindow] isOpaque];
  if (aTransparent != currentTransparency) {
    // Find out if this is a window we created by seeing if the delegate is WindowDelegate. If it is,
    // tell the nsCocoaWindow to set its background to transparent.
    id windowDelegate = [[mView nativeWindow] delegate];
    if (windowDelegate && [windowDelegate isKindOfClass:[WindowDelegate class]]) {
      nsCocoaWindow *widget = [(WindowDelegate *)windowDelegate geckoWidget];
      if (widget) {
        widget->MakeBackgroundTransparent(aTransparent);
        [(ChildView*)mView setTransparent:aTransparent];
      }
    }
  }
  return NS_OK;

  NS_OBJC_END_TRY_ABORT_BLOCK_NSRESULT;
}


NS_IMETHODIMP nsChildView::IsVisible(PRBool& outState)
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK_NSRESULT;

  if (!mVisible) {
    outState = mVisible;
  }
  else {
    // mVisible does not accurately reflect the state of a hidden tabbed view
    // so verify that the view has a window as well
    outState = ([mView window] != nil);
    // now check native widget hierarchy visibility
    if (outState && NSIsEmptyRect([mView visibleRect])) {
      outState = PR_FALSE;
    }
  }

  return NS_OK;

  NS_OBJC_END_TRY_ABORT_BLOCK_NSRESULT;
}


// Hide or show this component
NS_IMETHODIMP nsChildView::Show(PRBool aState)
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK_NSRESULT;

  if (aState != mVisible) {
    [mView setHidden:!aState];
    mVisible = aState;
  }
  return NS_OK;

  NS_OBJC_END_TRY_ABORT_BLOCK_NSRESULT;
}


nsIWidget*
nsChildView::GetParent(void)
{
  return mParentWidget;
}

nsIWidget*
nsChildView::GetTopLevelWidget()
{
  nsIWidget* current = this;
  for (nsIWidget* parent = GetParent(); parent ; parent = parent->GetParent())
    current = parent;
  return current;
}

NS_IMETHODIMP nsChildView::ModalEventFilter(PRBool aRealEvent, void *aEvent,
                                            PRBool *aForWindow)
{
  if (aForWindow)
    *aForWindow = PR_FALSE;
  return NS_ERROR_NOT_IMPLEMENTED;
}


NS_IMETHODIMP nsChildView::Enable(PRBool aState)
{
  return NS_OK;
}


NS_IMETHODIMP nsChildView::IsEnabled(PRBool *aState)
{
  // unimplemented
  if (aState)
   *aState = PR_TRUE;
  return NS_OK;
}


NS_IMETHODIMP nsChildView::SetFocus(PRBool aRaise)
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK_NSRESULT;

  // Don't do anything if SetFocus() has been called reentrantly on the same
  // object.  Sometimes calls to nsChildView::DispatchEvent() can get
  // temporarily stuck, causing calls to [ChildView sendFocusEvent:] and
  // SetFocus() to be reentered.  These reentrant calls are probably the
  // result of one or more bugs, and doing things on a reentrant call can
  // cause problems:  For example if mView is already the first responder and
  // we send it an NS_GOTFOCUS event (see below), this causes the Mochitests
  // to get stuck in the toolkit/content/tests/widgets/test_popup_button.xul
  // test.
  if (mInSetFocus)
    return NS_OK;
  mInSetFocus = PR_TRUE;
  NSWindow* window = [mView window];
  if (window) {
    nsAutoRetainView kungFuDeathGrip(mView);
    // For reasons that aren't yet clear, focus changes within a window (as
    // opposed to those between windows or between apps) should only trigger
    // NS_LOSTFOCUS and NS_GOTFOCUS events (sent to Gecko) in the context of
    // a call to nsChildView::SetFocus() (or nsCocoaWindow::SetFocus(), which
    // in any case re-routes to nsChildView::SetFocus()).  If we send these
    // events on every intra-window focus change (on every call to
    // [ChildView becomeFirstResponder:] or [ChildView resignFirstResponder:]),
    // the result will be strange focus bugs (like bmo bugs 399471, 403232,
    // 404433 and 408266).
    NSResponder* firstResponder = [window firstResponder];
    if ([mView isEqual:firstResponder]) {
      // Sometimes SetFocus() is called on an nsChildView object that's
      // already focused.  In principle this shouldn't happen, and in any
      // case we shouldn't have to dispatch any events.  But if we don't, we
      // sometimes get text-input cursors blinking in more than one text
      // field, or still blinking when the browser is no longer active.  For
      // reasons that aren't at all clear, this problem can be avoided by
      // always sending an NS_GOTFOCUS message here.
      if ([mView isKindOfClass:[ChildView class]])
        [(ChildView *)mView sendFocusEvent:NS_GOTFOCUS];
    } else {
      // Retain and release firstResponder around the call to
      // makeFirstResponder.
      [firstResponder retain];
      if ([window makeFirstResponder:mView]) {
        if ([firstResponder isKindOfClass:[ChildView class]])
          [(ChildView *)firstResponder sendFocusEvent:NS_LOSTFOCUS];
        if ([mView isKindOfClass:[ChildView class]])
          [(ChildView *)mView sendFocusEvent:NS_GOTFOCUS];
      }
      [firstResponder release];
    }
  }
  mInSetFocus = PR_FALSE;
  return NS_OK;

  NS_OBJC_END_TRY_ABORT_BLOCK_NSRESULT;
}


// Set the colormap of the window
NS_IMETHODIMP nsChildView::SetColorMap(nsColorMap *aColorMap)
{
  return NS_OK;
}


NS_IMETHODIMP nsChildView::SetMenuBar(nsIMenuBar * aMenuBar)
{
  return NS_ERROR_FAILURE; // subviews don't have menu bars
}


NS_IMETHODIMP nsChildView::ShowMenuBar(PRBool aShow)
{
  return NS_ERROR_FAILURE; // subviews don't have menu bars
}


// Override to set the cursor on the mac
NS_IMETHODIMP nsChildView::SetCursor(nsCursor aCursor)
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK_NSRESULT;

  nsBaseWidget::SetCursor(aCursor);
  [[nsCursorManager sharedInstance] setCursor: aCursor];
  return NS_OK;

  NS_OBJC_END_TRY_ABORT_BLOCK_NSRESULT;
}


// implement to fix "hidden virtual function" warning
NS_IMETHODIMP nsChildView::SetCursor(imgIContainer* aCursor,
                                      PRUint32 aHotspotX, PRUint32 aHotspotY)
{
  return nsBaseWidget::SetCursor(aCursor, aHotspotX, aHotspotY);
}


#pragma mark -


// Get this component dimension
NS_IMETHODIMP nsChildView::GetBounds(nsRect &aRect)
{
  aRect = mBounds;
  return NS_OK;
}


NS_IMETHODIMP nsChildView::SetBounds(const nsRect &aRect)
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK_NSRESULT;

  nsresult rv = Inherited::SetBounds(aRect);
  if (NS_SUCCEEDED(rv)) {
    NSRect r;
    GeckoRectToNSRect(aRect, r);
    [mView setFrame:r];
  }

  return rv;

  NS_OBJC_END_TRY_ABORT_BLOCK_NSRESULT;
}


NS_IMETHODIMP nsChildView::ConstrainPosition(PRBool aAllowSlop,
                                             PRInt32 *aX, PRInt32 *aY)
{
  return NS_OK;
}


// Move this component, aX and aY are in the parent widget coordinate system
NS_IMETHODIMP nsChildView::Move(PRInt32 aX, PRInt32 aY)
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK_NSRESULT;

  if (!mView || (mBounds.x == aX && mBounds.y == aY))
    return NS_OK;

  mBounds.x = aX;
  mBounds.y = aY;

  NSRect r;
  GeckoRectToNSRect(mBounds, r);
  [mView setFrame:r];

  if (mVisible)
    [mView setNeedsDisplay:YES];

  ReportMoveEvent();

  return NS_OK;

  NS_OBJC_END_TRY_ABORT_BLOCK_NSRESULT;
}


NS_IMETHODIMP nsChildView::Resize(PRInt32 aWidth, PRInt32 aHeight, PRBool aRepaint)
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK_NSRESULT;

  if (!mView || (mBounds.width == aWidth && mBounds.height == aHeight))
    return NS_OK;

  mBounds.width  = aWidth;
  mBounds.height = aHeight;

  NSRect r;
  GeckoRectToNSRect(mBounds, r);
  [mView setFrame:r];

  if (mVisible && aRepaint)
    [mView setNeedsDisplay:YES];

  ReportSizeEvent();

  return NS_OK;

  NS_OBJC_END_TRY_ABORT_BLOCK_NSRESULT;
}


NS_IMETHODIMP nsChildView::Resize(PRInt32 aX, PRInt32 aY, PRInt32 aWidth, PRInt32 aHeight, PRBool aRepaint)
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK_NSRESULT;

  BOOL isMoving = (mBounds.x != aX || mBounds.y != aY);
  BOOL isResizing = (mBounds.width != aWidth || mBounds.height != aHeight);
  if (!mView || (!isMoving && !isResizing))
    return NS_OK;

  if (isMoving) {
    mBounds.x = aX;
    mBounds.y = aY;
  }
  if (isResizing) {
    mBounds.width  = aWidth;
    mBounds.height = aHeight;
  }

  NSRect r;
  GeckoRectToNSRect(mBounds, r);
  [mView setFrame:r];

  if (mVisible && aRepaint)
    [mView setNeedsDisplay:YES];

  if (isMoving) {
    ReportMoveEvent();
    if (mOnDestroyCalled)
      return NS_OK;
  }
  if (isResizing)
    ReportSizeEvent();

  return NS_OK;

  NS_OBJC_END_TRY_ABORT_BLOCK_NSRESULT;
}


NS_METHOD nsChildView::GetPreferredSize(PRInt32& aWidth, PRInt32& aHeight)
{
  return NS_ERROR_FAILURE; // nobody call this anywhere in the code
}


NS_METHOD nsChildView::SetPreferredSize(PRInt32 aWidth, PRInt32 aHeight)
{
  return NS_ERROR_FAILURE; // nobody call this anywhere in the code
}


NS_IMETHODIMP nsChildView::BeginResizingChildren(void)
{
  return NS_OK;
}


NS_IMETHODIMP nsChildView::EndResizingChildren(void)
{
  return NS_OK;
}


NS_IMETHODIMP nsChildView::GetPluginClipRect(nsRect& outClipRect, nsPoint& outOrigin, PRBool& outWidgetVisible)
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK_NSRESULT;

  NS_ASSERTION(mIsPluginView, "GetPluginClipRect must only be called on a plugin widget");
  if (!mIsPluginView) return NS_ERROR_FAILURE;
  
  NSWindow* window = [mView nativeWindow];
  if (!window) return NS_ERROR_FAILURE;
  
  NSPoint viewOrigin = [mView convertPoint:NSZeroPoint toView:nil];
  NSRect frame = [[window contentView] frame];
  viewOrigin.y = frame.size.height - viewOrigin.y;
  
  // set up the clipping region for plugins.
  NSRect visibleBounds = [mView visibleRect];
  NSPoint clipOrigin   = [mView convertPoint:visibleBounds.origin toView:nil];
  
  // Convert from cocoa to QuickDraw coordinates
  clipOrigin.y = frame.size.height - clipOrigin.y;
  
  outClipRect.x = (nscoord)clipOrigin.x;
  outClipRect.y = (nscoord)clipOrigin.y;
  
  
  PRBool isVisible;
  IsVisible(isVisible);
  if (isVisible && [mView window] != nil) {
    outClipRect.width  = (nscoord)visibleBounds.size.width;
    outClipRect.height = (nscoord)visibleBounds.size.height;
    outWidgetVisible = PR_TRUE;
  }
  else {
    outClipRect.width = 0;
    outClipRect.height = 0;
    outWidgetVisible = PR_FALSE;
  }

  // need to convert view's origin to window coordinates.
  // then, encode as "SetOrigin" ready values.
  outOrigin.x = (nscoord)-viewOrigin.x;
  outOrigin.y = (nscoord)-viewOrigin.y;
  
  return NS_OK;

  NS_OBJC_END_TRY_ABORT_BLOCK_NSRESULT;
}


NS_IMETHODIMP nsChildView::StartDrawPlugin()
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK_NSRESULT;

  NS_ASSERTION(mIsPluginView, "StartDrawPlugin must only be called on a plugin widget");
  if (!mIsPluginView) return NS_ERROR_FAILURE;

  // nothing to do if this is a CoreGraphics plugin
  if (mPluginIsCG)
    return NS_OK;

  // prevent reentrant drawing
  if (mPluginDrawing)
    return NS_ERROR_FAILURE;
  
  NSWindow* window = [mView nativeWindow];
  if (!window)
    return NS_ERROR_FAILURE;
  
  // It appears that the WindowRef from which we get the plugin port undergoes the
  // traditional BeginUpdate/EndUpdate cycle, which, if you recall, sets the visible
  // region to the intersection of the visible region and the update region. Since
  // we don't know here if we're being drawn inside a BeginUpdate/EndUpdate pair
  // (which seem to occur in [NSWindow display]), and we don't want to have the burden
  // of correctly doing Carbon invalidates of the plugin rect, we manually set the
  // visible region to be the entire port every time.
  RgnHandle pluginRegion = ::NewRgn();
  if (pluginRegion) {
    PRBool portChanged = (mPluginPort.qdPort.port != CGrafPtr(GetQDGlobalsThePort()));
    CGrafPtr oldPort;
    GDHandle oldDevice;

    if (portChanged) {
      ::GetGWorld(&oldPort, &oldDevice);
      ::SetGWorld(mPluginPort.qdPort.port, ::IsPortOffscreen(mPluginPort.qdPort.port) ? nsnull : ::GetMainDevice());
    }

    ::SetOrigin(0, 0);
    
    nsRect clipRect; // this is in native window coordinates
    nsPoint origin;
    PRBool visible;
    GetPluginClipRect(clipRect, origin, visible);
    
    // XXX if we're not visible, set an empty clip region?
    Rect pluginRect;
    ConvertGeckoRectToMacRect(clipRect, pluginRect);
    
    ::RectRgn(pluginRegion, &pluginRect);
    ::SetPortVisibleRegion(mPluginPort.qdPort.port, pluginRegion);
    ::SetPortClipRegion(mPluginPort.qdPort.port, pluginRegion);
    
    // now set up the origin for the plugin
    ::SetOrigin(origin.x, origin.y);
    
    ::DisposeRgn(pluginRegion);

    if (portChanged)
      ::SetGWorld(oldPort, oldDevice);
  }

  mPluginDrawing = PR_TRUE;
  return NS_OK;

  NS_OBJC_END_TRY_ABORT_BLOCK_NSRESULT;
}


NS_IMETHODIMP nsChildView::EndDrawPlugin()
{
  NS_ASSERTION(mIsPluginView, "EndDrawPlugin must only be called on a plugin widget");
  if (!mIsPluginView) return NS_ERROR_FAILURE;

  mPluginDrawing = PR_FALSE;
  return NS_OK;
}


void nsChildView::LiveResizeStarted()
{
  // XXX todo. Use this to disable Java async redraw during resize
  mLiveResizeInProgress = PR_TRUE;
}


void nsChildView::LiveResizeEnded()
{
  mLiveResizeInProgress = PR_FALSE;
}


#pragma mark -


#ifdef INVALIDATE_DEBUGGING

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


// Invalidate this component's visible area
NS_IMETHODIMP nsChildView::Invalidate(PRBool aIsSynchronous)
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK_NSRESULT;

  if (!mView || !mVisible)
    return NS_OK;

  if (aIsSynchronous) {
    [mView display];
  }
  else if ([NSView focusView]) {
    // if a view is focussed (i.e. being drawn), then postpone the invalidate so that we
    // don't lose it.
    [mView setNeedsPendingDisplay];
  }
  else {
    [mView setNeedsDisplay:YES];
  }

  return NS_OK;

  NS_OBJC_END_TRY_ABORT_BLOCK_NSRESULT;
}


// Invalidate this component's visible area
NS_IMETHODIMP nsChildView::Invalidate(const nsRect &aRect, PRBool aIsSynchronous)
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK_NSRESULT;

  if (!mView || !mVisible)
    return NS_OK;

  NSRect r;
  GeckoRectToNSRect(aRect, r);
  
  if (aIsSynchronous) {
    [mView displayRect:r];
  }
  else if ([NSView focusView]) {
    // if a view is focussed (i.e. being drawn), then postpone the invalidate so that we
    // don't lose it.
    [mView setNeedsPendingDisplayInRect:r];
  }
  else {
    [mView setNeedsDisplayInRect:r];
  }

  return NS_OK;

  NS_OBJC_END_TRY_ABORT_BLOCK_NSRESULT;
}


// Validate the widget
NS_IMETHODIMP nsChildView::Validate()
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK_NSRESULT;

  [mView setNeedsDisplay:NO];
  return NS_OK;

  NS_OBJC_END_TRY_ABORT_BLOCK_NSRESULT;
}


// Invalidate this component's visible area
NS_IMETHODIMP nsChildView::InvalidateRegion(const nsIRegion *aRegion, PRBool aIsSynchronous)
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK_NSRESULT;

  if (!mView || !mVisible)
    return NS_OK;

  // FIXME rewrite to use a Cocoa region when nsIRegion isn't a QD Region
  NSRect r;
  nsRect bounds;
  nsIRegion* region = const_cast<nsIRegion*>(aRegion);     // ugh. this method should be const
  region->GetBoundingBox(&bounds.x, &bounds.y, &bounds.width, &bounds.height);
  GeckoRectToNSRect(bounds, r);
  
  if (aIsSynchronous)
    [mView displayRect:r];
  else
    [mView setNeedsDisplayInRect:r];

  return NS_OK;

  NS_OBJC_END_TRY_ABORT_BLOCK_NSRESULT;
}


inline PRUint16 COLOR8TOCOLOR16(PRUint8 color8)
{
  // return (color8 == 0xFF ? 0xFFFF : (color8 << 8));
  return (color8 << 8) | color8;  /* (color8 * 257) == (color8 * 0x0101) */
}


// Dummy impl, meant to be overridden
PRBool
nsChildView::OnPaint(nsPaintEvent &event)
{
  return PR_TRUE;
}


// this is handled for us by UpdateWidget
NS_IMETHODIMP nsChildView::Update()
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK_NSRESULT;

  // Update means "Flush any pending changes right now."  It does *not* mean
  // repaint the world. :) -- dwh
  [mView displayIfNeeded];
  return NS_OK;

  NS_OBJC_END_TRY_ABORT_BLOCK_NSRESULT;
}


#pragma mark -


// Scroll the bits of a view and its children
// FIXME: I'm sure the invalidating can be optimized, just no time now.
NS_IMETHODIMP nsChildView::Scroll(PRInt32 aDx, PRInt32 aDy, nsRect *aClipRect)
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK_NSRESULT;

  BOOL viewWasDirty = NO;
  if (mVisible) {
    viewWasDirty = [mView needsDisplay];

    NSSize scrollVector = {aDx,aDy};
    [mView scrollRect: [mView visibleRect] by:scrollVector];
  }
  
  // Scroll the children (even if the widget is not visible)
  for (nsIWidget* kid = mFirstChild; kid; kid = kid->GetNextSibling()) {
    // We use resize rather than move since it gives us control
    // over repainting.  We can scroll like a bat out of hell
    // by not wasting time invalidating the widgets, since it's
    // completely unnecessary to do so.
    nsRect bounds;
    kid->GetBounds(bounds);
    kid->Resize(bounds.x + aDx, bounds.y + aDy, bounds.width, bounds.height, PR_FALSE);
  }

  if (mOnDestroyCalled)
    return NS_OK;

  if (mVisible) {
    if (viewWasDirty) {
      [mView setNeedsDisplay:YES];
    }
    else {
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

  NS_OBJC_END_TRY_ABORT_BLOCK_NSRESULT;
}


// Invokes callback and ProcessEvent methods on Event Listener object
NS_IMETHODIMP nsChildView::DispatchEvent(nsGUIEvent* event, nsEventStatus& aStatus)
{
  aStatus = nsEventStatus_eIgnore;

  nsCOMPtr<nsIWidget> kungFuDeathGrip(mParentWidget ? mParentWidget : this);
  if (mParentWidget) {
    nsWindowType type;
    mParentWidget->GetWindowType(type);
    if (type == eWindowType_popup) {
      // use the parent popup's widget if there is no view
      void* clientData = nsnull;
      if (event->widget)
        event->widget->GetClientData(clientData);
      if (!clientData)
        event->widget = mParentWidget;
    }
  }

  if (mEventCallback)
    aStatus = (*mEventCallback)(event);

  // dispatch to event listener if event was not consumed
  if (mEventListener && aStatus != nsEventStatus_eConsumeNoDefault)
    aStatus = mEventListener->ProcessEvent(*event);

  return NS_OK;
}


PRBool nsChildView::DispatchWindowEvent(nsGUIEvent &event)
{
  nsEventStatus status;
  DispatchEvent(&event, status);
  return ConvertStatus(status);
}


// Deal with all sort of mouse event
PRBool nsChildView::DispatchMouseEvent(nsMouseEvent &aEvent)
{
  PRBool result = PR_FALSE;

  // call the event callback 
  if (mEventCallback)
    return DispatchWindowEvent(aEvent);

  if (mMouseListener) {
    nsCOMPtr<nsIWidget> kungFuDeathGrip(this);
    switch (aEvent.message) {
      case NS_MOUSE_MOVE:
        result = ConvertStatus(mMouseListener->MouseMoved(aEvent));
        break;

      case NS_MOUSE_BUTTON_DOWN:
        result = ConvertStatus(mMouseListener->MousePressed(aEvent));
        break;

      case NS_MOUSE_BUTTON_UP: {
        result = ConvertStatus(mMouseListener->MouseReleased(aEvent));
        if (mMouseListener)
          result = ConvertStatus(mMouseListener->MouseClicked(aEvent));
        break;
      }
    } // switch
  }

  return result;
}


#pragma mark -


PRBool nsChildView::ReportDestroyEvent()
{
  nsGUIEvent event(PR_TRUE, NS_DESTROY, this);
  event.time = PR_IntervalNow();
  return DispatchWindowEvent(event);
}


PRBool nsChildView::ReportMoveEvent()
{
  nsGUIEvent moveEvent(PR_TRUE, NS_MOVE, this);
  moveEvent.refPoint.x = mBounds.x;
  moveEvent.refPoint.y = mBounds.y;
  moveEvent.time       = PR_IntervalNow();
  return DispatchWindowEvent(moveEvent);
}


PRBool nsChildView::ReportSizeEvent()
{
  nsSizeEvent sizeEvent(PR_TRUE, NS_SIZE, this);
  sizeEvent.time        = PR_IntervalNow();
  sizeEvent.windowSize  = &mBounds;
  sizeEvent.mWinWidth   = mBounds.width;
  sizeEvent.mWinHeight  = mBounds.height;
  return DispatchWindowEvent(sizeEvent);
}


#pragma mark -


/*  Calculate the x and y offsets for this particular widget
 *  @update  ps 09/22/98
 *  @param   aX -- x offset amount
 *  @param   aY -- y offset amount 
 *  @return  NOTHING
 */
NS_IMETHODIMP nsChildView::CalcOffset(PRInt32 &aX,PRInt32 &aY)
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK_NSRESULT;

  aX = aY = 0;
  NSRect bounds = {{0, 0}, {0, 0}};
  bounds = [mView convertRect:bounds toView:nil];
  aX += static_cast<PRInt32>(bounds.origin.x);
  aY += static_cast<PRInt32>(bounds.origin.y);

  return NS_OK;

  NS_OBJC_END_TRY_ABORT_BLOCK_NSRESULT;
}


// Find if a point in local coordinates is inside this object
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


//    Convert the given rect to global coordinates.
//    @param aLocalRect  -- rect in local coordinates of this widget
//    @param aGlobalRect -- |aLocalRect| in global coordinates
NS_IMETHODIMP nsChildView::WidgetToScreen(const nsRect& aLocalRect, nsRect& aGlobalRect)
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK_NSRESULT;

  NSRect temp;
  GeckoRectToNSRect(aLocalRect, temp);
  
  // 1. First translate this rect into window coords. The returned rect is always in
  //    bottom-left coordinates.
  //
  //    NOTE: convertRect:toView:nil doesn't care if |mView| is a flipped view (with
  //          top-left coords) and so assumes that our passed-in rect's origin is in
  //          bottom-left coordinates. We adjust this further down, by subtracting
  //          the final screen rect's origin by the rect's height, to get the origo
  //          where we want it.
  temp = [mView convertRect:temp toView:nil];  
  
  // 2. We turn the window-coord rect's origin into screen (still bottom-left) coords.
  temp.origin = [[mView nativeWindow] convertBaseToScreen:temp.origin];
  
  // 3. Since we're dealing in bottom-left coords, we need to make it top-left coords
  //    before we pass it back to Gecko.
  FlipCocoaScreenCoordinate(temp.origin);
  
  // 4. If this is rect has a size (and is not simply a point), it is important to account 
  //    for the fact that convertRect:toView:nil thought our passed-in point was in bottom-left 
  //    coords in step #1. Thus, we subtract the rect's height, to get the top-left rect's origin 
  //     where we want it.
  temp.origin.y -= temp.size.height;
  
  NSRectToGeckoRect(temp, aGlobalRect);
  return NS_OK;

  NS_OBJC_END_TRY_ABORT_BLOCK_NSRESULT;
}


//    Convert the given rect to local coordinates.
//    @param aGlobalRect  -- rect in screen coordinates 
//    @param aLocalRect -- |aGlobalRect| in coordinates of this widget
NS_IMETHODIMP nsChildView::ScreenToWidget(const nsRect& aGlobalRect, nsRect& aLocalRect)
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK_NSRESULT;

  NSRect temp;
  GeckoRectToNSRect(aGlobalRect, temp);
  FlipCocoaScreenCoordinate(temp.origin);

  temp.origin = [[mView nativeWindow] convertScreenToBase:temp.origin];   // convert to screen coords
  temp = [mView convertRect:temp fromView:nil];                     // convert to window coords

  NSRectToGeckoRect(temp, aLocalRect);
  
  return NS_OK;

  NS_OBJC_END_TRY_ABORT_BLOCK_NSRESULT;
} 


// Convert the coordinates to some device coordinates so GFX can draw.
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
  // child views don't have titles
  return NS_OK;
}


NS_IMETHODIMP nsChildView::GetAttention(PRInt32 aCycleCount)
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK_NSRESULT;

  [NSApp requestUserAttention:NSInformationalRequest];
  return NS_OK;

  NS_OBJC_END_TRY_ABORT_BLOCK_NSRESULT;
}


#pragma mark -


// Force Input Method Editor to commit the uncommited input
// Note that this and other nsIKBStateControl methods don't necessarily
// get called on the same ChildView that input is going through.
NS_IMETHODIMP nsChildView::ResetInputState()
{
#ifdef DEBUG_IME
  NSLog(@"**** ResetInputState");
#endif

  nsTSMManager::CommitIME();
  return NS_OK;
}


// 'open' means that it can take non-ASCII chars
NS_IMETHODIMP nsChildView::SetIMEOpenState(PRBool aState)
{
#ifdef DEBUG_IME
  NSLog(@"**** SetIMEOpenState aState = %d", aState);
#endif

  nsTSMManager::SetIMEOpenState(aState);
  return NS_OK;
}


// 'open' means that it can take non-ASCII chars
NS_IMETHODIMP nsChildView::GetIMEOpenState(PRBool* aState)
{
#ifdef DEBUG_IME
  NSLog(@"**** GetIMEOpenState");
#endif

  *aState = nsTSMManager::GetIMEOpenState();
  return NS_OK;
}


NS_IMETHODIMP nsChildView::SetIMEEnabled(PRUint32 aState)
{
#ifdef DEBUG_IME
  NSLog(@"**** SetIMEEnabled aState = %d", aState);
#endif

  switch (aState) {
    case nsIKBStateControl::IME_STATUS_ENABLED:
      nsTSMManager::SetRomanKeyboardsOnly(PR_FALSE);
      nsTSMManager::EnableIME(PR_TRUE);
      break;
    case nsIKBStateControl::IME_STATUS_DISABLED:
      nsTSMManager::SetRomanKeyboardsOnly(PR_FALSE);
      nsTSMManager::EnableIME(PR_FALSE);
      break;
    case nsIKBStateControl::IME_STATUS_PASSWORD:
      nsTSMManager::SetRomanKeyboardsOnly(PR_TRUE);
      nsTSMManager::EnableIME(PR_FALSE);
      break;
    default:
      NS_ERROR("not implemented!");
  }
  return NS_OK;
}


NS_IMETHODIMP nsChildView::GetIMEEnabled(PRUint32* aState)
{
#ifdef DEBUG_IME
  NSLog(@"**** GetIMEEnabled");
#endif

  if (nsTSMManager::IsIMEEnabled())
    *aState = nsIKBStateControl::IME_STATUS_ENABLED;
  else if (nsTSMManager::IsRomanKeyboardsOnly())
    *aState = nsIKBStateControl::IME_STATUS_PASSWORD;
  else
    *aState = nsIKBStateControl::IME_STATUS_DISABLED;
  return NS_OK;
}


// Destruct and don't commit the IME composition string.
NS_IMETHODIMP nsChildView::CancelIMEComposition()
{
#ifdef DEBUG_IME
  NSLog(@"**** CancelIMEComposition");
#endif

  nsTSMManager::CancelIME();
  return NS_OK;
}


NS_IMETHODIMP nsChildView::GetToggledKeyState(PRUint32 aKeyCode,
                                              PRBool* aLEDState)
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK_NSRESULT;

#ifdef DEBUG_IME
  NSLog(@"**** GetToggledKeyState");
#endif
  NS_ENSURE_ARG_POINTER(aLEDState);
  PRUint32 key;
  switch (aKeyCode) {
    case NS_VK_CAPS_LOCK:
      key = alphaLock;
      break;
    case NS_VK_NUM_LOCK:
      key = kEventKeyModifierNumLockMask;
      break;
    // Mac doesn't support SCROLL_LOCK state.
    default:
      return NS_ERROR_NOT_IMPLEMENTED;
  }
  PRUint32 modifierFlags = ::GetCurrentKeyModifiers();
  *aLEDState = (modifierFlags & key) != 0;
  return NS_OK;

  NS_OBJC_END_TRY_ABORT_BLOCK_NSRESULT;
}


#pragma mark -


gfxASurface*
nsChildView::GetThebesSurface()
{
  if (!mTempThebesSurface) {
    mTempThebesSurface = new gfxQuartzSurface(gfxSize(1, 1), gfxASurface::ImageFormatARGB32);
  }

  return mTempThebesSurface;
}


NS_IMETHODIMP
nsChildView::BeginSecureKeyboardInput()
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK_NSRESULT;

  nsresult rv = nsBaseWidget::BeginSecureKeyboardInput();
  if (NS_SUCCEEDED(rv))
    ::EnableSecureEventInput();
  return rv;

  NS_OBJC_END_TRY_ABORT_BLOCK_NSRESULT;
}


NS_IMETHODIMP
nsChildView::EndSecureKeyboardInput()
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK_NSRESULT;

  nsresult rv = nsBaseWidget::EndSecureKeyboardInput();
  if (NS_SUCCEEDED(rv))
    ::DisableSecureEventInput();
  return rv;

  NS_OBJC_END_TRY_ABORT_BLOCK_NSRESULT;
}


#ifdef ACCESSIBILITY
void
nsChildView::GetDocumentAccessible(nsIAccessible** aAccessible)
{
  *aAccessible = nsnull;
  
  nsCOMPtr<nsIAccessible> accessible = do_QueryReferent(mAccessible);
  if (!mAccessible) {
    // need to fetch the accessible anew, because it has gone away.
    nsEventStatus status;
    nsAccessibleEvent event(PR_TRUE, NS_GETACCESSIBLE, this);
    DispatchEvent(&event, status);
  
    // cache the accessible in our weak ptr
    mAccessible = do_GetWeakReference(event.accessible);
    
    // now try again
    accessible = do_QueryReferent(mAccessible);
  }
  
  NS_IF_ADDREF(*aAccessible = accessible.get());

  return;
}
#endif


#pragma mark -


@implementation ChildView


// globalDragPboard is non-null during native drag sessions that did not originate
// in our native NSView (it is set in |draggingEntered:|). It is unset when the
// drag session ends for this view, either with the mouse exiting or when a drop
// occurs in this view.
NSPasteboard* globalDragPboard = nil;


// gLastDragView and gLastDragEvent are only non-null during calls to |mouseDragged:|
// in our native NSView. They are used to communicate information to the drag service
// during drag invocation (starting a drag in from the view). All drag service drag
// invocations happen only while these two global variables are non-null, while |mouseDragged:|
// is on the stack.
NSView* gLastDragView = nil;
NSEvent* gLastDragEvent = nil;


// initWithFrame:geckoChild:
- (id)initWithFrame:(NSRect)inFrame geckoChild:(nsChildView*)inChild
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK_NIL;

  if ((self = [super initWithFrame:inFrame])) {
    mWindow = nil;
    mGeckoChild = inChild;
    mIsPluginView = NO;

    mCurKeyEvent = nil;
    mKeyDownHandled = PR_FALSE;
    mIgnoreDoCommand = NO;
    mKeyPressSent = NO;

    // initialization for NSTextInput
    mMarkedRange.location = NSNotFound;
    mMarkedRange.length = 0;

    mLastMenuForEventEvent = nil;
    mDragService = nsnull;
  }
  
  // register for things we'll take from other applications
  PR_LOG(sCocoaLog, PR_LOG_ALWAYS, ("ChildView initWithFrame: registering drag types\n"));
  [self registerForDraggedTypes:[NSArray arrayWithObjects:NSFilenamesPboardType,
                                                          NSStringPboardType,
                                                          NSURLPboardType,
                                                          NSFilesPromisePboardType,
                                                          kWildcardPboardType,
                                                          kCorePboardType_url,
                                                          kCorePboardType_urld,
                                                          kCorePboardType_urln,
                                                          nil]];

  return self;

  NS_OBJC_END_TRY_ABORT_BLOCK_NIL;
}


- (void)dealloc
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK;

  [mPendingDirtyRects release];
  [mLastMenuForEventEvent release];
  
  if (sLastViewEntered == self)
    sLastViewEntered = nil;

  [super dealloc];    

  // This sets the current port to _savePort.
  // todo: Only do if a Quickdraw plugin is present in the hierarchy!
  ::SetPort(NULL);

  NS_OBJC_END_TRY_ABORT_BLOCK;
}


- (void)widgetDestroyed
{
  mGeckoChild = nsnull;
  // Just in case we're destroyed abruptly and missed the draggingExited
  // or performDragOperation message.
  NS_IF_RELEASE(mDragService);
}


// mozView method, return our gecko child view widget. Note this does not AddRef.
- (nsIWidget*) widget
{
  return static_cast<nsIWidget*>(mGeckoChild);
}


// mozView method, get the window that this view is associated with
- (NSWindow*)nativeWindow
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK_NIL;

  NSWindow* currWin = [self window];
  if (currWin)
     return currWin;
  else
     return mWindow;

  NS_OBJC_END_TRY_ABORT_BLOCK_NIL;
}


// mozView method, set the NSWindow that this view is associated with (even when
// not in the view hierarchy).
- (void)setNativeWindow:(NSWindow*)aWindow
{
  mWindow = aWindow;
}


- (void)setNeedsPendingDisplay
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK;

  mPendingFullDisplay = YES;
  [self performSelector:@selector(processPendingRedraws) withObject:nil afterDelay:0];

  NS_OBJC_END_TRY_ABORT_BLOCK;
}


- (void)setNeedsPendingDisplayInRect:(NSRect)invalidRect
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK;

  if (!mPendingDirtyRects)
    mPendingDirtyRects = [[NSMutableArray alloc] initWithCapacity:1];
  [mPendingDirtyRects addObject:[NSValue valueWithRect:invalidRect]];
  [self performSelector:@selector(processPendingRedraws) withObject:nil afterDelay:0];

  NS_OBJC_END_TRY_ABORT_BLOCK;
}


// Clears the queue of any pending invalides
- (void)processPendingRedraws
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK;

  if (mPendingFullDisplay) {
    [self setNeedsDisplay:YES];
  }
  else {
    unsigned int count = [mPendingDirtyRects count];
    for (unsigned int i = 0; i < count; ++i) {
      [self setNeedsDisplayInRect:[[mPendingDirtyRects objectAtIndex:i] rectValue]];
    }
  }
  mPendingFullDisplay = NO;
  [mPendingDirtyRects release];
  mPendingDirtyRects = nil;

  NS_OBJC_END_TRY_ABORT_BLOCK;
}


- (NSString*)description
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK_NIL;

  return [NSString stringWithFormat:@"ChildView %p, gecko child %p, frame %@", self, mGeckoChild, NSStringFromRect([self frame])];

  NS_OBJC_END_TRY_ABORT_BLOCK_NIL;
}


// Find the nearest scrollable view for this ChildView
// (recall that views are not refcounted)
- (nsIScrollableView*) getScrollableView
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK_NSNULL;

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
    if (req) {
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

  NS_OBJC_END_TRY_ABORT_BLOCK_NSNULL;
}


// set the closed hand cursor and record the starting scroll positions
- (void) startHandScroll:(NSEvent*)theEvent
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK;

  if (!mGeckoChild)
    return;

  mHandScrollStartMouseLoc = [[self window] convertBaseToScreen:[theEvent locationInWindow]];

  nsIScrollableView* aScrollableView = [self getScrollableView]; 

  // if we succeeded in getting aScrollableView
  if (aScrollableView) {
    aScrollableView->GetScrollPosition(mHandScrollStartScrollX, mHandScrollStartScrollY);
    mGeckoChild->SetCursor(eCursor_grabbing);
    mInHandScroll = TRUE;
  }

  NS_OBJC_END_TRY_ABORT_BLOCK;
}


// update the scroll position based on the new mouse coordinates
- (void) updateHandScroll:(NSEvent*)theEvent
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK;

  if (!mGeckoChild)
    return;

  nsIScrollableView* aScrollableView = [self getScrollableView];
  if (!aScrollableView)
    return;

  NSPoint newMouseLoc = [[self window] convertBaseToScreen:[theEvent locationInWindow]];

  PRInt32 deltaX = (PRInt32)(mHandScrollStartMouseLoc.x - newMouseLoc.x);
  PRInt32 deltaY = (PRInt32)(newMouseLoc.y - mHandScrollStartMouseLoc.y);

  // convert to the nsIView coordinates
  PRInt32 p2a = mGeckoChild->GetDeviceContext()->AppUnitsPerDevPixel();
  nscoord newX = mHandScrollStartScrollX + NSIntPixelsToAppUnits(deltaX, p2a);
  nscoord newY = mHandScrollStartScrollY + NSIntPixelsToAppUnits(deltaY, p2a);
  aScrollableView->ScrollTo(newX, newY, NS_VMREFRESH_IMMEDIATE);

  NS_OBJC_END_TRY_ABORT_BLOCK;
}


// Return true if the correct modifiers are pressed to perform hand scrolling.
+ (BOOL) areHandScrollModifiers:(unsigned int)modifiers
{
  // The command and option key should be held down.  Ignore capsLock by
  // setting it explicitly to match.
  modifiers |= NSAlphaShiftKeyMask;
  return (modifiers & NSDeviceIndependentModifierFlagsMask) ==
      (NSAlphaShiftKeyMask | NSCommandKeyMask | NSAlternateKeyMask);
}


// If the user is pressing the hand scroll modifiers, then set
// the hand scroll cursor.
- (void) setHandScrollCursor:(NSEvent*)theEvent
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK;

  if (!mGeckoChild)
    return;

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
    }
  }

  NS_OBJC_END_TRY_ABORT_BLOCK;
}


// reset the scroll flag and cursor
- (void) stopHandScroll:(NSEvent*)theEvent
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK;

  mInHandScroll = FALSE;
  [self setHandScrollCursor:theEvent];

  NS_OBJC_END_TRY_ABORT_BLOCK;
}


// When smooth scrolling is turned on on panther, the parent of a scrollbar (which
// I guess they assume is a NSScrollView) gets called with this method. I have no
// idea what the correct return value is, but we have to have this otherwise the scrollbar
// will not continuously respond when the mouse is held down in the pageup/down area.
-(float)_destinationFloatValueForScroller:(id)scroller
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK_RETURN;

  return [scroller floatValue];

  NS_OBJC_END_TRY_ABORT_BLOCK_RETURN(0.0);
}


// Override in order to keep our mouse enter/exit tracking rect in sync with
// the frame of the view
- (void)setFrame:(NSRect)frameRect
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK;

  [super setFrame:frameRect];
  if (mMouseEnterExitTag)
    [self removeTrackingRect:mMouseEnterExitTag];

  if ([self window])
    mMouseEnterExitTag = [self addTrackingRect:[self bounds]
                                         owner:self
                                      userData:nil
                                  assumeInside:[[self window] acceptsMouseMovedEvents]];

  NS_OBJC_END_TRY_ABORT_BLOCK;
}


// Make the origin of this view the topLeft corner (gecko origin) rather
// than the bottomLeft corner (standard cocoa origin).
- (BOOL)isFlipped
{
  return YES;
}


- (void)setTransparent:(BOOL)transparent
{
  mIsTransparent = transparent;
}


- (BOOL)isOpaque
{
  return !mIsTransparent;
}


-(void)setIsPluginView:(BOOL)aIsPlugin
{
  mIsPluginView = aIsPlugin;
}


-(BOOL)isPluginView
{
  return mIsPluginView;
}


- (BOOL)childViewHasPlugin
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK_RETURN;

  NSArray* subviews = [self subviews];
  for (unsigned int i = 0; i < [subviews count]; i ++) {
    id subview = [subviews objectAtIndex:i];
    if ([subview respondsToSelector:@selector(isPluginView)] && [subview isPluginView])
      return YES;
  }
  
  return NO;

  NS_OBJC_END_TRY_ABORT_BLOCK_RETURN(NO);
}


- (void)sendFocusEvent:(PRUint32)eventType
{
  if (!mGeckoChild)
    return;

  nsEventStatus status = nsEventStatus_eIgnore;
  nsGUIEvent focusGuiEvent(PR_TRUE, eventType, mGeckoChild);
  focusGuiEvent.time = PR_IntervalNow();
  mGeckoChild->DispatchEvent(&focusGuiEvent, status);
}


// We accept key and mouse events, so don't keep passing them up the chain. Allow
// this to be a 'focussed' widget for event dispatch
- (BOOL)acceptsFirstResponder
{
  return YES;
}


- (void)viewWillMoveToWindow:(NSWindow *)newWindow
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK;

  if (mMouseEnterExitTag)
    [self removeTrackingRect:mMouseEnterExitTag];

  [super viewWillMoveToWindow:newWindow];

  NS_OBJC_END_TRY_ABORT_BLOCK;
}


- (void)viewDidMoveToWindow
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK;

  if ([self window])
    mMouseEnterExitTag = [self addTrackingRect:[self bounds] owner:self
                                      userData:nil assumeInside: [[self window]
                                      acceptsMouseMovedEvents]];

  [super viewDidMoveToWindow];

  NS_OBJC_END_TRY_ABORT_BLOCK;
}


- (void)viewWillStartLiveResize
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK;

  if (mGeckoChild && mIsPluginView)
    mGeckoChild->LiveResizeStarted();
  
  [super viewWillStartLiveResize];

  NS_OBJC_END_TRY_ABORT_BLOCK;
}


- (void)viewDidEndLiveResize
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK;

  if (mGeckoChild && mIsPluginView)
    mGeckoChild->LiveResizeEnded();

  [super viewDidEndLiveResize];

  NS_OBJC_END_TRY_ABORT_BLOCK;
}


- (void)scrollRect:(NSRect)aRect by:(NSSize)offset
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK;

  // Update any pending dirty rects to reflect the new scroll position
  if (mPendingDirtyRects) {
    unsigned int count = [mPendingDirtyRects count];
    for (unsigned int i = 0; i < count; ++i) {
      NSRect oldRect = [[mPendingDirtyRects objectAtIndex:i] rectValue];
      NSRect newRect = NSOffsetRect(oldRect, offset.width, offset.height);
      [mPendingDirtyRects replaceObjectAtIndex:i
                                    withObject:[NSValue valueWithRect:newRect]];
    }
  }
  [super scrollRect:aRect by:offset];

  NS_OBJC_END_TRY_ABORT_BLOCK;
}


- (BOOL)mouseDownCanMoveWindow
{
  return NO;
}


- (void)lockFocus
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK;

  // Set the current GrafPort to a "safe" port before calling [NSQuickDrawView lockFocus],
  // so that the NSQuickDrawView stashes a pointer to this known-good port internally.
  // It will set the port back to this port on destruction.
  ::SetPort(NULL);  // todo: only do if a Quickdraw plugin is present in the hierarchy!
  [super lockFocus];

  NS_OBJC_END_TRY_ABORT_BLOCK;
}


// The display system has told us that a portion of our view is dirty. Tell
// gecko to paint it
- (void)drawRect:(NSRect)aRect
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK;

  PRBool isVisible;
  if (!mGeckoChild || NS_FAILED(mGeckoChild->IsVisible(isVisible)) || !isVisible)
    return;

  CGContextRef cgContext = (CGContextRef)[[NSGraphicsContext currentContext] graphicsPort];

  nsRect geckoBounds;
  mGeckoChild->GetBounds(geckoBounds);

  NSRect bounds = [self bounds];
  nsRefPtr<gfxQuartzSurface> targetSurface =
    new gfxQuartzSurface(cgContext, gfxSize(bounds.size.width, bounds.size.height));

#ifdef DEBUG_UPDATE
  fprintf (stderr, "---- Update[%p][%p] [%f %f %f %f] cgc: %p\n  gecko bounds: [%d %d %d %d]\n",
           self, mGeckoChild,
           aRect.origin.x, aRect.origin.y, aRect.size.width, aRect.size.height, cgContext,
           geckoBounds.x, geckoBounds.y, geckoBounds.width, geckoBounds.height);

  CGAffineTransform xform = CGContextGetCTM(cgContext);
  fprintf (stderr, "  xform in: [%f %f %f %f %f %f]\n", xform.a, xform.b, xform.c, xform.d, xform.tx, xform.ty);
#endif

  nsRefPtr<gfxContext> targetContext = new gfxContext(targetSurface);

  nsCOMPtr<nsIRenderingContext> rc;
  mGeckoChild->GetDeviceContext()->CreateRenderingContextInstance(*getter_AddRefs(rc));
  rc->Init(mGeckoChild->GetDeviceContext(), targetContext);

  /* clip and build a region */
  nsCOMPtr<nsIRegion> rgn(do_CreateInstance(kRegionCID));
  if (rgn)
    rgn->Init();

  const NSRect *rects;
  int count, i;
  [self getRectsBeingDrawn:&rects count:&count];
  for (i = 0; i < count; ++i) {
    const NSRect& r = rects[i];

    // add to the region
    if (rgn)
      rgn->Union((PRInt32)r.origin.x, (PRInt32)r.origin.y, (PRInt32)r.size.width, (PRInt32)r.size.height);

    // to the context for clipping
    targetContext->Rectangle(gfxRect(r.origin.x, r.origin.y, r.size.width, r.size.height));
  }
  targetContext->Clip();
  
  // bounding box of the dirty area
  nsRect fullRect;
  NSRectToGeckoRect(aRect, fullRect);

  nsPaintEvent paintEvent(PR_TRUE, NS_PAINT, mGeckoChild);
  paintEvent.renderingContext = rc;
  paintEvent.rect = &fullRect;
  paintEvent.region = rgn;

  nsAutoRetainView kungFuDeathGrip(self);
  mGeckoChild->DispatchWindowEvent(paintEvent);
  if (!mGeckoChild)
    return;

  paintEvent.renderingContext = nsnull;
  paintEvent.region = nsnull;

  targetContext = nsnull;
  targetSurface = nsnull;

  // note that the cairo surface *MUST* be destroyed at this point,
  // or bad things will happen (since we can't keep the cgContext around
  // beyond this drawRect message handler)

#ifdef DEBUG_UPDATE
  fprintf (stderr, "  window coords: [%d %d %d %d]\n", fullRect.x, fullRect.y, fullRect.width, fullRect.height);
  fprintf (stderr, "---- update done ----\n");

#if 0
  CGContextSetRGBStrokeColor (cgContext,
                            ((((unsigned long)self) & 0xff)) / 255.0,
                            ((((unsigned long)self) & 0xff00) >> 8) / 255.0,
                            ((((unsigned long)self) & 0xff0000) >> 16) / 255.0,
                            0.5);
#endif 
  CGContextSetRGBStrokeColor (cgContext, 1, 0, 0, 0.8);
  CGContextSetLineWidth (cgContext, 4.0);
  CGContextStrokeRect (cgContext,
                       CGRectMake(aRect.origin.x, aRect.origin.y, aRect.size.width, aRect.size.height));
#endif

  NS_OBJC_END_TRY_ABORT_BLOCK;
}


// Allows us to turn off setting up the clip region
// before each drawRect. We already clip within gecko.
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
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK;

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

  NS_OBJC_END_TRY_ABORT_BLOCK;
}
#endif


// We sometimes need to reroute events when there is a rollup widget and the
// event isn't targeted at it.
//
// Rerouting may be needed when the user tries to navigate a context menu while
// keeping the mouse-button down (left or right mouse button) -- the OS thinks this
// is a dragging operation, so it sends events (mouseMoved and mouseUp) to the
// window where the dragging operation started (the parent of the context
// menu window).  It also works around a bizarre Apple bug - if (while a context
// menu is open) you move the mouse over another app's window and then back over
// the context menu, mouseMoved events will be sent to the window underneath the
// context menu.
- (BOOL)ensureCorrectMouseEventTarget:(NSEvent*)anEvent
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK_RETURN;

  // If there is no rollup widget we assume the OS routed the event correctly.
  if (!gRollupWidget)
    return YES;

  // If this is the rollup widget and the event is not a mouse move then trust the OS routing.  
  // The reason for this trust is complicated.
  //
  // There are three types of mouse events that can legitimately need to be targeted at a window
  // that they are not over. Mouse moves, mouse drags, and mouse ups. Anything else our app wouldn't
  // handle (if the mouse was not over any window) or it would go to the appropriate window.
  //
  // We need to do manual event rerouting for mouse moves because we know that in some cases, like
  // when there is a submenu opened from a popup window, the OS will route mouse move events to the
  // submenu even if the mouse is over the parent. Mouse move events are never tied to a particular
  // window because of some originating action like the starting point of a drag for drag events or
  // a mouse down event for mouse up events, so it is always safe to do our own routing on them here.
  //
  // As for mouse drags and mouse ups, they have originating actions that tie them to windows they
  // may no longer be over. If there is a rollup window present when one of these events is getting
  // processed but we are not it, we are probably the window where the action originated, and that
  // action must have caused the rollup window to come into existence. In that case, we might need
  // to reroute the event if it is over the rollup window. That is why if we're not the rollup window
  // we don't return YES here.
  NSWindow* rollupWindow = (NSWindow*)gRollupWidget->GetNativeData(NS_NATIVE_WINDOW);
  if (mWindow == rollupWindow && [anEvent type] != NSMouseMoved)
    return YES;

  // Find the window that the event is over.
  NSWindow* targetWindow = nsCocoaUtils::FindWindowUnderPoint(nsCocoaUtils::ScreenLocationForEvent(anEvent));

  // If the event was not over any window, send it to the rollup window.
  if (!targetWindow)
    targetWindow = rollupWindow;

  // At this point we've resolved a target window, if we are it then just return
  // yes so we handle it. No need to redirect.
  if (targetWindow == mWindow)
    return YES;

  // Send the event to its new destination.
  NSPoint newWindowLocation = nsCocoaUtils::EventLocationForWindow(anEvent, targetWindow);
  NSEvent *newEvent = [NSEvent mouseEventWithType:[anEvent type]
                                         location:newWindowLocation
                                    modifierFlags:[anEvent modifierFlags]
                                        timestamp:GetCurrentEventTime()
                                     windowNumber:[targetWindow windowNumber]
                                          context:nil
                                      eventNumber:0
                                       clickCount:1
                                         pressure:0.0];
  [targetWindow sendEvent:newEvent];

  // Return NO because we just sent the event somewhere else.
  return NO;

  NS_OBJC_END_TRY_ABORT_BLOCK_RETURN(NO);
}


// If we've just created a non-native context menu, we need to mark it as
// such and let the OS (and other programs) know when it opens and closes
// (this is how the OS knows to close other programs' context menus when
// ours open).  We send the initial notification here, but others are sent
// in nsCocoaWindow::Show().
- (void)maybeInitContextMenuTracking
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK;

  if (!gRollupWidget)
    return;
  NSWindow *popupWindow = (NSWindow*)gRollupWidget->GetNativeData(NS_NATIVE_WINDOW);
  if (!popupWindow || ![popupWindow isKindOfClass:[PopupWindow class]])
    return;

  [[NSDistributedNotificationCenter defaultCenter]
    postNotificationName:@"com.apple.HIToolbox.beginMenuTrackingNotification"
                  object:@"org.mozilla.gecko.PopupWindow"];
  [(PopupWindow*)popupWindow setIsContextMenu:YES];

  NS_OBJC_END_TRY_ABORT_BLOCK;
}


- (BOOL)maybeRollup:(NSEvent*)theEvent
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK_RETURN;

  if (mLastMenuForEventEvent == theEvent)
    return PR_FALSE;

  PRBool retVal = PR_FALSE;
  if (gRollupWidget && gRollupListener) {
    NSWindow* currentPopup = static_cast<NSWindow*>(gRollupWidget->GetNativeData(NS_NATIVE_WINDOW));
    if (!nsCocoaUtils::IsEventOverWindow(theEvent, currentPopup)) {
      PRBool rollup = PR_TRUE;
      if ([theEvent type] == NSScrollWheel) {
        gRollupListener->ShouldRollupOnMouseWheelEvent(&rollup);
        // We don't want the event passed on for scrollwheel events if we're
        // not supposed to close the popup.  Otherwise the background window
        // will scroll when a custom context menu or the autoscroll popup is
        // open (and the mouse isn't over the popup) -- which doesn't seem right.
        // This change resolves bmo bug 344367.
        retVal = PR_TRUE;
      }
      // if we're dealing with menus, we probably have submenus and
      // we don't want to rollup if the clickis in a parent menu of
      // the current submenu
      nsCOMPtr<nsIMenuRollup> menuRollup;
      menuRollup = (do_QueryInterface(gRollupListener));
      if (menuRollup) {
        nsAutoTArray<nsIWidget*, 5> widgetChain;
        menuRollup->GetSubmenuWidgetChain(&widgetChain);
        for (PRUint32 i = 0; i < widgetChain.Length(); i++) {
          nsIWidget* widget = widgetChain[i];
          NSWindow* currWindow = (NSWindow*)widget->GetNativeData(NS_NATIVE_WINDOW);
          if (nsCocoaUtils::IsEventOverWindow(theEvent, currWindow)) {
            rollup = PR_FALSE;
            break;
          }
        } // foreach parent menu widget
      } // if rollup listener knows about menus

      // if we've determined that we should still rollup, do it.
      if (rollup) {
        gRollupListener->Rollup(nsnull);
        retVal = PR_TRUE;
      }
    }
  }

  return retVal;

  NS_OBJC_END_TRY_ABORT_BLOCK_RETURN(NO);
}


- (void)mouseDown:(NSEvent*)theEvent
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK;

  if (![self ensureCorrectMouseEventTarget:theEvent])
    return;

  nsAutoRetainView kungFuDeathGrip(self);

  if ([self maybeRollup:theEvent])
    return;

  unsigned int modifierFlags = [theEvent modifierFlags];

  // if the command and alt keys are held down, initiate hand scrolling
  if ([ChildView areHandScrollModifiers:modifierFlags]) {
    [self startHandScroll:theEvent];
    // needed to change the focus, among other things, since we don't
    // get to do that below.
    [super mouseDown:theEvent];
    return; // do not pass this mousedown event to gecko
  }

#if USE_CLICK_HOLD_CONTEXTMENU
  // fire off timer to check for click-hold after two seconds. retains |theEvent|
  [self performSelector:@selector(clickHoldCallback:) withObject:theEvent afterDelay:2.0];
#endif

  // in order to send gecko events we'll need a gecko widget
  if (!mGeckoChild)
    return;

  nsMouseEvent geckoEvent(PR_TRUE, NS_MOUSE_BUTTON_DOWN, nsnull, nsMouseEvent::eReal);
  [self convertCocoaMouseEvent:theEvent toGeckoEvent:&geckoEvent];
  geckoEvent.clickCount = [theEvent clickCount];
  if (modifierFlags & NSControlKeyMask)
    geckoEvent.button = nsMouseEvent::eRightButton;
  else
    geckoEvent.button = nsMouseEvent::eLeftButton;

  // create native EventRecord for use by plugins
  EventRecord macEvent;
  macEvent.what = mouseDown;
  macEvent.message = 0;
  macEvent.when = ::TickCount();
  ::GetGlobalMouse(&macEvent.where);
  macEvent.modifiers = GetCurrentKeyModifiers();
  geckoEvent.nativeMsg = &macEvent;

  mGeckoChild->DispatchMouseEvent(geckoEvent);

  // XXX maybe call markedTextSelectionChanged:client: here?

  NS_OBJC_END_TRY_ABORT_BLOCK;
}


- (void)mouseUp:(NSEvent *)theEvent
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK;

  if (mInHandScroll) {
    [self updateHandScroll:theEvent];
    [self stopHandScroll:theEvent];
    return;
  }

  if (![self ensureCorrectMouseEventTarget:theEvent])
    return;

  if (!mGeckoChild)
    return;

  nsMouseEvent geckoEvent(PR_TRUE, NS_MOUSE_BUTTON_UP, nsnull, nsMouseEvent::eReal);
  [self convertCocoaMouseEvent:theEvent toGeckoEvent:&geckoEvent];

  // create native EventRecord for use by plugins
  EventRecord macEvent;
  macEvent.what = mouseUp;
  macEvent.message = 0;
  macEvent.when = ::TickCount();
  ::GetGlobalMouse(&macEvent.where);
  macEvent.modifiers = GetCurrentKeyModifiers();
  geckoEvent.nativeMsg = &macEvent;

  mGeckoChild->DispatchMouseEvent(geckoEvent);

  NS_OBJC_END_TRY_ABORT_BLOCK;
}


// sends a mouse enter or exit event into gecko
static nsEventStatus SendGeckoMouseEnterOrExitEvent(PRBool isTrusted,
                                                    PRUint32 msg,
                                                    nsIWidget *widget,
                                                    nsMouseEvent::reasonType aReason,
                                                    NSPoint* localEventLocation)
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK_RETURN;

  if (!widget || !localEventLocation)
    return nsEventStatus_eIgnore;

  nsMouseEvent event(isTrusted, msg, widget, aReason);
  event.refPoint.x = nscoord((PRInt32)localEventLocation->x);
  event.refPoint.y = nscoord((PRInt32)localEventLocation->y);

  EventRecord macEvent;
  macEvent.what = adjustCursorEvent;
  macEvent.message = 0;
  macEvent.when = ::TickCount();
  ::GetGlobalMouse(&macEvent.where);
  macEvent.modifiers = ::GetCurrentKeyModifiers();
  event.nativeMsg = &macEvent;

  nsEventStatus status;
  widget->DispatchEvent(&event, status);
  return status;

  NS_OBJC_END_TRY_ABORT_BLOCK_RETURN(nsEventStatus_eIgnore);
}


- (void)mouseMoved:(NSEvent*)theEvent
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK;

  // Work around an Apple bug that causes the OS to continue sending
  // mouseMoved events to a window for a while after it's been miniaturized.
  // This may be related to a similar problem with popup windows (bmo bug
  // 378645, popup windows continue to receive mouseMoved events after having
  // been "ordered out"), which is worked around in nsCocoaWindow::Show()
  // (search on 378645 in nsCocoaWindow.mm).  This problem is bmo bug 410219,
  // and exists in both OS X 10.4 and 10.5.
  if ([[self window] isMiniaturized])
    return;

  NSPoint windowEventLocation = nsCocoaUtils::EventLocationForWindow(theEvent, mWindow);
  NSPoint viewEventLocation = [self convertPoint:windowEventLocation fromView:nil];

  // Installing a mouseMoved handler on the EventMonitor target (in
  // nsToolkit::RegisterForAllProcessMouseEvents()) means that some of the
  // events received here come from other processes.  For this reason we need
  // to avoid processing them unless they're over a context menu -- otherwise
  // tooltips and other mouse-hover effects will "work" even when our app
  // doesn't have the focus.
  BOOL mouseEventIsOverRollupWidget = NO;
  if (gRollupWidget) {
    NSWindow *popupWindow = (NSWindow*)gRollupWidget->GetNativeData(NS_NATIVE_WINDOW);
    mouseEventIsOverRollupWidget = nsCocoaUtils::IsEventOverWindow(theEvent, popupWindow);
  }

  if (![NSApp isActive] && !mouseEventIsOverRollupWidget) {
    if (sLastViewEntered) {
      nsIWidget* lastViewEnteredWidget = [(NSView<mozView>*)sLastViewEntered widget];
      NSPoint exitEventLocation = [sLastViewEntered convertPoint:windowEventLocation fromView:nil];
      SendGeckoMouseEnterOrExitEvent(PR_TRUE, NS_MOUSE_EXIT, lastViewEnteredWidget, nsMouseEvent::eReal, &exitEventLocation);
      sLastViewEntered = nil;
    }
    return;
  }

  if (![self ensureCorrectMouseEventTarget:theEvent])
    return;

  NSView* view = [[mWindow contentView] hitTest:windowEventLocation];
  if (view) {
    // we shouldn't handle this if the hit view is not us
    if (view != (NSView*)self) {
      [view mouseMoved:theEvent];
      return;
    }
  }
  else {
    // If the hit test returned nil then the mouse isn't over the window. If thse mouse
    // exited the window then send mouse exit to the last view in the window it was over.
    if (sLastViewEntered) {
      NSPoint exitEventLocation = [sLastViewEntered convertPoint:windowEventLocation fromView:nil];
      // NSLog(@"sending NS_MOUSE_EXIT event with point %f,%f\n", exitEventLocation.x, exitEventLocation.y);
      nsIWidget* lastViewEnteredWidget = [(NSView<mozView>*)sLastViewEntered widget];
      SendGeckoMouseEnterOrExitEvent(PR_TRUE, NS_MOUSE_EXIT, lastViewEnteredWidget, nsMouseEvent::eReal, &exitEventLocation);
      sLastViewEntered = nil;
    }
    return;
  }

  // At this point we are supposed to handle this event. If we were not the last view entered, then
  // we should send an exit event to the last view entered and an enter event to ourselves.  
  if (!mGeckoChild)
    return;

  nsAutoRetainView kungFuDeathGrip(self);
  if (sLastViewEntered != self) {
    if (sLastViewEntered) {
      NSPoint exitEventLocation = [sLastViewEntered convertPoint:windowEventLocation fromView:nil];
      // NSLog(@"sending NS_MOUSE_EXIT event with point %f,%f\n", exitEventLocation.x, exitEventLocation.y);
      nsIWidget* lastViewEnteredWidget = [(NSView<mozView>*)sLastViewEntered widget];
      SendGeckoMouseEnterOrExitEvent(PR_TRUE, NS_MOUSE_EXIT, lastViewEnteredWidget, nsMouseEvent::eReal, &exitEventLocation);

      // The mouse exit event we just sent may have destroyed this widget, bail if that happened.
      if (!mGeckoChild)
        return;
    }

    // NSLog(@"sending NS_MOUSE_ENTER event with point %f,%f\n", viewEventLocation.x, viewEventLocation.y);
    SendGeckoMouseEnterOrExitEvent(PR_TRUE, NS_MOUSE_ENTER, mGeckoChild, nsMouseEvent::eReal, &viewEventLocation);

    // The mouse enter event we just sent may have destroyed this widget, bail if that happened.
    if (!mGeckoChild)
      return;

    // mark this view as the last view entered
    sLastViewEntered = (NSView*)self;

    // checks to see if we should change to the hand cursor
    [self setHandScrollCursor:theEvent];
  }

  // check if we are in a hand scroll or if the user
  // has command and alt held down; if so,  we do not want
  // gecko messing with the cursor.
  if ([ChildView areHandScrollModifiers:[theEvent modifierFlags]])
    return;

  nsMouseEvent geckoEvent(PR_TRUE, NS_MOUSE_MOVE, nsnull, nsMouseEvent::eReal);
  [self convertCocoaMouseEvent:theEvent toGeckoEvent:&geckoEvent];

  // create native EventRecord for use by plugins
  EventRecord macEvent;
  macEvent.what = adjustCursorEvent;
  macEvent.message = 0;
  macEvent.when = ::TickCount();
  ::GetGlobalMouse(&macEvent.where);
  macEvent.modifiers = GetCurrentKeyModifiers();
  geckoEvent.nativeMsg = &macEvent;

  mGeckoChild->DispatchMouseEvent(geckoEvent);

  NS_OBJC_END_TRY_ABORT_BLOCK;
}


- (void)mouseDragged:(NSEvent*)theEvent
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK;

  if (![self ensureCorrectMouseEventTarget:theEvent])
    return;

  if (!mGeckoChild)
    return;

  // if the handscroll flag is set, steal this event
  if (mInHandScroll) {
    [self updateHandScroll:theEvent];
    return;
  }

  gLastDragView = self;
  gLastDragEvent = theEvent;

  nsMouseEvent geckoEvent(PR_TRUE, NS_MOUSE_MOVE, nsnull, nsMouseEvent::eReal);
  [self convertCocoaMouseEvent:theEvent toGeckoEvent:&geckoEvent];

  // create native EventRecord for use by plugins
  EventRecord macEvent;
  macEvent.what = nullEvent;
  macEvent.message = 0;
  macEvent.when = ::TickCount();
  ::GetGlobalMouse(&macEvent.where);
  macEvent.modifiers = btnState | ::GetCurrentKeyModifiers();
  geckoEvent.nativeMsg = &macEvent;

  mGeckoChild->DispatchMouseEvent(geckoEvent);

  // Note, sending the above event might have destroyed our widget since we didn't retain.
  // Fine so long as we don't access any local variables from here on.

  gLastDragView = nil;
  gLastDragEvent = nil;
  // XXX maybe call markedTextSelectionChanged:client: here?

  NS_OBJC_END_TRY_ABORT_BLOCK;
}


- (void)rightMouseDown:(NSEvent *)theEvent
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK;

  if (![self ensureCorrectMouseEventTarget:theEvent])
    return;

  nsAutoRetainView kungFuDeathGrip(self);

  [self maybeRollup:theEvent];
  if (!mGeckoChild)
    return;

  // The right mouse went down, fire off a right mouse down event to gecko
  nsMouseEvent geckoEvent(PR_TRUE, NS_MOUSE_BUTTON_DOWN, nsnull, nsMouseEvent::eReal);
  [self convertCocoaMouseEvent:theEvent toGeckoEvent:&geckoEvent];
  geckoEvent.button = nsMouseEvent::eRightButton;
  geckoEvent.clickCount = [theEvent clickCount];

  // create native EventRecord for use by plugins
  EventRecord macEvent;
  macEvent.what = mouseDown;
  macEvent.message = 0;
  macEvent.when = ::TickCount();
  ::GetGlobalMouse(&macEvent.where);
  macEvent.modifiers = controlKey;  // fake a context menu click
  geckoEvent.nativeMsg = &macEvent;

  PRBool handled = mGeckoChild->DispatchMouseEvent(geckoEvent);
  if (!mGeckoChild)
    return;

  if (!handled)
    [super rightMouseDown:theEvent]; // let the superview do context menu stuff

  NS_OBJC_END_TRY_ABORT_BLOCK;
}


- (void)rightMouseUp:(NSEvent *)theEvent
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK;

  if (![self ensureCorrectMouseEventTarget:theEvent])
    return;

  if (!mGeckoChild)
    return;

  nsMouseEvent geckoEvent(PR_TRUE, NS_MOUSE_BUTTON_UP, nsnull, nsMouseEvent::eReal);
  [self convertCocoaMouseEvent:theEvent toGeckoEvent:&geckoEvent];
  geckoEvent.button = nsMouseEvent::eRightButton;
  geckoEvent.clickCount = [theEvent clickCount];

  // create native EventRecord for use by plugins
  EventRecord macEvent;
  macEvent.what = mouseUp;
  macEvent.message = 0;
  macEvent.when = ::TickCount();
  ::GetGlobalMouse(&macEvent.where);
  macEvent.modifiers = controlKey;  // fake a context menu click
  geckoEvent.nativeMsg = &macEvent;

  nsAutoRetainView kungFuDeathGrip(self);
  PRBool handled = mGeckoChild->DispatchMouseEvent(geckoEvent);
  if (!mGeckoChild)
    return;

  if (!handled)
    [super rightMouseUp:theEvent];

  NS_OBJC_END_TRY_ABORT_BLOCK;
}


- (void)rightMouseDragged:(NSEvent*)theEvent
{
  if (![self ensureCorrectMouseEventTarget:theEvent])
    return;

  if (!mGeckoChild)
    return;

  nsMouseEvent geckoEvent(PR_TRUE, NS_MOUSE_MOVE, nsnull, nsMouseEvent::eReal);
  [self convertCocoaMouseEvent:theEvent toGeckoEvent:&geckoEvent];
  geckoEvent.button = nsMouseEvent::eRightButton;

  // send event into Gecko by going directly to the
  // the widget.
  mGeckoChild->DispatchMouseEvent(geckoEvent);
}


- (void)otherMouseDown:(NSEvent *)theEvent
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK;

  if (![self ensureCorrectMouseEventTarget:theEvent])
    return;

  nsAutoRetainView kungFuDeathGrip(self);

  if ([self maybeRollup:theEvent])
    return;

  if (!mGeckoChild)
    return;

  nsMouseEvent geckoEvent(PR_TRUE, NS_MOUSE_BUTTON_DOWN, nsnull, nsMouseEvent::eReal);
  [self convertCocoaMouseEvent:theEvent toGeckoEvent:&geckoEvent];
  geckoEvent.button = nsMouseEvent::eMiddleButton;
  geckoEvent.clickCount = [theEvent clickCount];

  mGeckoChild->DispatchMouseEvent(geckoEvent);

  NS_OBJC_END_TRY_ABORT_BLOCK;
}


- (void)otherMouseUp:(NSEvent *)theEvent
{
  if (!mGeckoChild)
    return;

  nsMouseEvent geckoEvent(PR_TRUE, NS_MOUSE_BUTTON_UP, nsnull, nsMouseEvent::eReal);
  [self convertCocoaMouseEvent:theEvent toGeckoEvent:&geckoEvent];
  geckoEvent.button = nsMouseEvent::eMiddleButton;

  mGeckoChild->DispatchMouseEvent(geckoEvent);
}


- (void)otherMouseDragged:(NSEvent*)theEvent
{
  if (!mGeckoChild)
    return;

  nsMouseEvent geckoEvent(PR_TRUE, NS_MOUSE_MOVE, nsnull, nsMouseEvent::eReal);
  [self convertCocoaMouseEvent:theEvent toGeckoEvent:&geckoEvent];
  geckoEvent.button = nsMouseEvent::eMiddleButton;

  // send event into Gecko by going directly to the
  // the widget.
  mGeckoChild->DispatchMouseEvent(geckoEvent);
}


// Handle an NSScrollWheel event for a single axis only.
-(void)scrollWheel:(NSEvent*)theEvent forAxis:(enum nsMouseScrollEvent::nsMouseScrollFlags)inAxis
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK;

  if (!mGeckoChild)
    return;

  float scrollDelta;

  if (inAxis & nsMouseScrollEvent::kIsVertical)
    scrollDelta = -[theEvent deltaY];
  else if (inAxis & nsMouseScrollEvent::kIsHorizontal)
    scrollDelta = -[theEvent deltaX];
  else
    return; // caller screwed up

  if (scrollDelta == 0)
    // No sense in firing off a Gecko event.  Note that as of 10.4 Tiger,
    // a single NSScrollWheel event might result in deltaX = deltaY = 0.
    return;
  
  nsMouseScrollEvent geckoEvent(PR_TRUE, NS_MOUSE_SCROLL, nsnull);
  [self convertCocoaMouseEvent:theEvent toGeckoEvent:&geckoEvent];
  geckoEvent.scrollFlags |= inAxis;

  // Gecko only understands how to scroll by an integer value.  Using floor
  // and ceil is better than truncating the fraction, especially when
  // |delta| < 1.
  if (scrollDelta < 0)
    geckoEvent.delta = (PRInt32)floorf(scrollDelta);
  else
    geckoEvent.delta = (PRInt32)ceilf(scrollDelta);

  nsAutoRetainView kungFuDeathGrip(self);
  mGeckoChild->DispatchWindowEvent(geckoEvent);
  if (!mGeckoChild)
    return;

  // dispatch scroll wheel carbon event for plugins
  {
    EventRef theEvent;
    OSStatus err = ::MacCreateEvent(NULL,
                          kEventClassMouse,
                          kEventMouseWheelMoved,
                          TicksToEventTime(TickCount()),
                          kEventAttributeUserEvent,
                          &theEvent);
    if (err == noErr) {
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
      
      ::SendEventToEventTarget(theEvent, GetWindowEventTarget((WindowRef)[[self window] windowRef]));
      ReleaseEvent(theEvent);
    }
  }

  NS_OBJC_END_TRY_ABORT_BLOCK;
}


-(void)scrollWheel:(NSEvent*)theEvent
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK;

  nsAutoRetainView kungFuDeathGrip(self);

  if ([self maybeRollup:theEvent])
    return;

  if (!mGeckoChild)
    return;

  // It's possible for a single NSScrollWheel event to carry both useful
  // deltaX and deltaY, for example, when the "wheel" is a trackpad.
  // NSMouseScrollEvent can only carry one axis at a time, so the system
  // event will be split into two Gecko events if necessary.
  [self scrollWheel:theEvent forAxis:nsMouseScrollEvent::kIsVertical];
  if (!mGeckoChild)
    return;
  [self scrollWheel:theEvent forAxis:nsMouseScrollEvent::kIsHorizontal];

  NS_OBJC_END_TRY_ABORT_BLOCK;
}


-(NSMenu*)menuForEvent:(NSEvent*)theEvent
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK_NIL;

  if (!mGeckoChild || [self isPluginView])
    return nil;

  nsAutoRetainView kungFuDeathGrip(self);

  [self maybeRollup:theEvent];
  if (!mGeckoChild)
    return nil;

  [mLastMenuForEventEvent release];
  mLastMenuForEventEvent = [theEvent retain];

  nsMouseEvent geckoEvent(PR_TRUE, NS_CONTEXTMENU, nsnull, nsMouseEvent::eReal);
  [self convertCocoaMouseEvent:theEvent toGeckoEvent:&geckoEvent];
  geckoEvent.button = nsMouseEvent::eRightButton;
  mGeckoChild->DispatchMouseEvent(geckoEvent);
  if (!mGeckoChild)
    return nil;

  // If we're running in a browser that (unlike Camino) uses non-native
  // context menus, we must call maybeInitContextMenuTracking.  This call was
  // dropped with the patch for bug 396186, which caused at least one
  // regression (bug 416455).
  nsCOMPtr<nsIPrefBranch> prefs = do_GetService(NS_PREFSERVICE_CONTRACTID);
  if (prefs) {
    PRBool useNativeContextMenus;
    nsresult rv = prefs->GetBoolPref("ui.use_native_popup_windows", &useNativeContextMenus);
    if (!NS_SUCCEEDED(rv) || !useNativeContextMenus)
      [self maybeInitContextMenuTracking];
  }

  // Go up our view chain to fetch the correct menu to return.
  return [self contextMenu];

  NS_OBJC_END_TRY_ABORT_BLOCK_NIL;
}


- (NSMenu*)contextMenu
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK_NIL;

  NSView* superView = [self superview];
  if ([superView respondsToSelector:@selector(contextMenu)])
    return [(NSView<mozView>*)superView contextMenu];

  return nil;

  NS_OBJC_END_TRY_ABORT_BLOCK_NIL;
}


- (TopLevelWindowData*)ensureWindowData
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK_NIL;

  WindowDataMap* windowMap = [WindowDataMap sharedWindowDataMap];

  TopLevelWindowData* windowData = [windowMap dataForWindow:mWindow];
  if (mWindow && !windowData)
  {
    windowData = [[TopLevelWindowData alloc] initWithWindow:mWindow];
    [windowMap setData:windowData forWindow:mWindow]; // takes ownership
    [windowData release];
  }
  return windowData;

  NS_OBJC_END_TRY_ABORT_BLOCK_NIL;
}


static PRBool ConvertUnicodeToCharCode(PRUnichar inUniChar, unsigned char* outChar)
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK_RETURN;

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

  NS_OBJC_END_TRY_ABORT_BLOCK_RETURN(PR_FALSE);
}


static void ConvertCocoaKeyEventToMacEvent(NSEvent* cocoaEvent, EventRecord& macEvent, PRUint32 keyType = 0)
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK;

    UInt32 charCode = 0;
    if ([cocoaEvent type] == NSFlagsChanged) {
      macEvent.what = keyType == NS_KEY_DOWN ? keyDown : keyUp;
    } else {
      charCode = [[cocoaEvent characters] characterAtIndex:0];
      if ([cocoaEvent type] == NSKeyDown)
        macEvent.what = [cocoaEvent isARepeat] ? autoKey : keyDown;
      else
        macEvent.what = keyUp;
    }

    if (charCode >= 0x0080) {
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

  NS_OBJC_END_TRY_ABORT_BLOCK;
}

// Key code constants
enum
{
  kEscapeKeyCode      = 0x35,
  kRCommandKeyCode    = 0x36, // right command key
  kCommandKeyCode     = 0x37,
  kShiftKeyCode       = 0x38,
  kCapsLockKeyCode    = 0x39,
  kOptionkeyCode      = 0x3A,
  kControlKeyCode     = 0x3B,
  kRShiftKeyCode      = 0x3C, // right shift key
  kROptionKeyCode     = 0x3D, // right option key
  kRControlKeyCode    = 0x3E, // right control key
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

// The following key codes are not defined until Mac OS X 10.5
#if MAC_OS_X_VERSION_MAX_ALLOWED <= MAC_OS_X_VERSION_10_4
  kVK_ANSI_1          = 0x12,
  kVK_ANSI_2          = 0x13,
  kVK_ANSI_3          = 0x14,
  kVK_ANSI_4          = 0x15,
  kVK_ANSI_5          = 0x17,
  kVK_ANSI_6          = 0x16,
  kVK_ANSI_7          = 0x1A,
  kVK_ANSI_8          = 0x1C,
  kVK_ANSI_9          = 0x19,
  kVK_ANSI_0          = 0x1D,
#endif

  kKeypadMultiplyKeyCode  = 0x43,
  kKeypadAddKeyCode       = 0x45,
  kKeypadSubtractKeyCode  = 0x4E,
  kKeypadDecimalKeyCode   = 0x41,
  kKeypadDivideKeyCode    = 0x4B,
  kKeypadEqualsKeyCode    = 0x51, // no correpsonding gecko key code
  kEnterKeyCode           = 0x4C,
  kReturnKeyCode          = 0x24,
  kPowerbookEnterKeyCode  = 0x34, // Enter on Powerbook's keyboard is different
  
  kInsertKeyCode          = 0x72, // also help key
  kDeleteKeyCode          = 0x75, // also forward delete key
  kTabKeyCode             = 0x30,
  kTildeKeyCode           = 0x32,
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


static PRBool IsPrintableChar(PRUnichar aChar)
{
  return (aChar >= 0x20 && aChar <= 0x7E) || aChar >= 0xA0;
}

static PRUint32 GetGeckoKeyCodeFromChar(PRUnichar aChar)
{
  // We don't support the key code for non-ASCII characters
  if (aChar > 0x7E)
    return 0;

  if (aChar >= 'a' && aChar <= 'z') // lowercase
    return PRUint32(toupper(aChar));
  else if (aChar >= 'A' && aChar <= 'Z') // uppercase
    return PRUint32(aChar);
  else if (aChar >= '0' && aChar <= '9')
    return PRUint32(aChar - '0' + NS_VK_0);

  switch (aChar)
  {
    case kReturnCharCode:
    case kEnterCharCode:
    case '\n':
      return NS_VK_RETURN;
    case '{':
    case '[':
      return NS_VK_OPEN_BRACKET;
    case '}':
    case ']':
      return NS_VK_CLOSE_BRACKET;
    case '\'':
    case '"':
      return NS_VK_QUOTE;

    case '\\':                  return NS_VK_BACK_SLASH;
    case ' ':                   return NS_VK_SPACE;
    case ';':                   return NS_VK_SEMICOLON;
    case '=':                   return NS_VK_EQUALS;
    case ',':                   return NS_VK_COMMA;
    case '.':                   return NS_VK_PERIOD;
    case '/':                   return NS_VK_SLASH;
    case '`':                   return NS_VK_BACK_QUOTE;
    case '\t':                  return NS_VK_TAB;
    case '-':                   return NS_VK_SUBTRACT;
    case '+':                   return NS_VK_ADD;

    default:
      if (!IsPrintableChar(aChar))
        NS_WARNING("GetGeckoKeyCodeFromChar has failed.");
      return 0;
    }
}


static PRUint32 ConvertMacToGeckoKeyCode(UInt32 keyCode, nsKeyEvent* aKeyEvent, NSString* characters)
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK_RETURN;

  PRUint32 geckoKeyCode = 0;
  PRUnichar charCode = 0;
  if ([characters length])
    charCode = [characters characterAtIndex:0];

  switch (keyCode)
  {
    // modifiers. We don't get separate events for these
    case kEscapeKeyCode:        geckoKeyCode = NS_VK_ESCAPE;         break;
    case kRCommandKeyCode:
    case kCommandKeyCode:       geckoKeyCode = NS_VK_META;           break;
    case kRShiftKeyCode:
    case kShiftKeyCode:         geckoKeyCode = NS_VK_SHIFT;          break;
    case kCapsLockKeyCode:      geckoKeyCode = NS_VK_CAPS_LOCK;      break;
    case kRControlKeyCode:
    case kControlKeyCode:       geckoKeyCode = NS_VK_CONTROL;        break;
    case kROptionKeyCode:
    case kOptionkeyCode:        geckoKeyCode = NS_VK_ALT;            break;
    case kClearKeyCode:         geckoKeyCode = NS_VK_CLEAR;          break;

    // function keys
    case kF1KeyCode:            geckoKeyCode = NS_VK_F1;             break;
    case kF2KeyCode:            geckoKeyCode = NS_VK_F2;             break;
    case kF3KeyCode:            geckoKeyCode = NS_VK_F3;             break;
    case kF4KeyCode:            geckoKeyCode = NS_VK_F4;             break;
    case kF5KeyCode:            geckoKeyCode = NS_VK_F5;             break;
    case kF6KeyCode:            geckoKeyCode = NS_VK_F6;             break;
    case kF7KeyCode:            geckoKeyCode = NS_VK_F7;             break;
    case kF8KeyCode:            geckoKeyCode = NS_VK_F8;             break;
    case kF9KeyCode:            geckoKeyCode = NS_VK_F9;             break;
    case kF10KeyCode:           geckoKeyCode = NS_VK_F10;            break;
    case kF11KeyCode:           geckoKeyCode = NS_VK_F11;            break;
    case kF12KeyCode:           geckoKeyCode = NS_VK_F12;            break;
    // case kF13KeyCode:           geckoKeyCode = NS_VK_F13;            break;    // clash with the 3 below
    // case kF14KeyCode:           geckoKeyCode = NS_VK_F14;            break;
    // case kF15KeyCode:           geckoKeyCode = NS_VK_F15;            break;
    case kPauseKeyCode:         geckoKeyCode = NS_VK_PAUSE;          break;
    case kScrollLockKeyCode:    geckoKeyCode = NS_VK_SCROLL_LOCK;    break;
    case kPrintScreenKeyCode:   geckoKeyCode = NS_VK_PRINTSCREEN;    break;

    // keypad
    case kKeypad0KeyCode:       geckoKeyCode = NS_VK_NUMPAD0;        break;
    case kKeypad1KeyCode:       geckoKeyCode = NS_VK_NUMPAD1;        break;
    case kKeypad2KeyCode:       geckoKeyCode = NS_VK_NUMPAD2;        break;
    case kKeypad3KeyCode:       geckoKeyCode = NS_VK_NUMPAD3;        break;
    case kKeypad4KeyCode:       geckoKeyCode = NS_VK_NUMPAD4;        break;
    case kKeypad5KeyCode:       geckoKeyCode = NS_VK_NUMPAD5;        break;
    case kKeypad6KeyCode:       geckoKeyCode = NS_VK_NUMPAD6;        break;
    case kKeypad7KeyCode:       geckoKeyCode = NS_VK_NUMPAD7;        break;
    case kKeypad8KeyCode:       geckoKeyCode = NS_VK_NUMPAD8;        break;
    case kKeypad9KeyCode:       geckoKeyCode = NS_VK_NUMPAD9;        break;

    case kKeypadMultiplyKeyCode:  geckoKeyCode = NS_VK_MULTIPLY;     break;
    case kKeypadAddKeyCode:       geckoKeyCode = NS_VK_ADD;          break;
    case kKeypadSubtractKeyCode:  geckoKeyCode = NS_VK_SUBTRACT;     break;
    case kKeypadDecimalKeyCode:   geckoKeyCode = NS_VK_DECIMAL;      break;
    case kKeypadDivideKeyCode:    geckoKeyCode = NS_VK_DIVIDE;       break;

    // these may clash with forward delete and help
    case kInsertKeyCode:        geckoKeyCode = NS_VK_INSERT;         break;
    case kDeleteKeyCode:        geckoKeyCode = NS_VK_DELETE;         break;

    case kBackspaceKeyCode:     geckoKeyCode = NS_VK_BACK;           break;
    case kTabKeyCode:           geckoKeyCode = NS_VK_TAB;            break;
    case kHomeKeyCode:          geckoKeyCode = NS_VK_HOME;           break;
    case kEndKeyCode:           geckoKeyCode = NS_VK_END;            break;
    case kPageUpKeyCode:        geckoKeyCode = NS_VK_PAGE_UP;        break;
    case kPageDownKeyCode:      geckoKeyCode = NS_VK_PAGE_DOWN;      break;
    case kLeftArrowKeyCode:     geckoKeyCode = NS_VK_LEFT;           break;
    case kRightArrowKeyCode:    geckoKeyCode = NS_VK_RIGHT;          break;
    case kUpArrowKeyCode:       geckoKeyCode = NS_VK_UP;             break;
    case kDownArrowKeyCode:     geckoKeyCode = NS_VK_DOWN;           break;
    case kVK_ANSI_1:            geckoKeyCode = NS_VK_1;              break;
    case kVK_ANSI_2:            geckoKeyCode = NS_VK_2;              break;
    case kVK_ANSI_3:            geckoKeyCode = NS_VK_3;              break;
    case kVK_ANSI_4:            geckoKeyCode = NS_VK_4;              break;
    case kVK_ANSI_5:            geckoKeyCode = NS_VK_5;              break;
    case kVK_ANSI_6:            geckoKeyCode = NS_VK_6;              break;
    case kVK_ANSI_7:            geckoKeyCode = NS_VK_7;              break;
    case kVK_ANSI_8:            geckoKeyCode = NS_VK_8;              break;
    case kVK_ANSI_9:            geckoKeyCode = NS_VK_9;              break;
    case kVK_ANSI_0:            geckoKeyCode = NS_VK_0;              break;

    default:
      // if we haven't gotten the key code already, look at the char code
      geckoKeyCode = GetGeckoKeyCodeFromChar(charCode);
  }

  return geckoKeyCode;

  NS_OBJC_END_TRY_ABORT_BLOCK_RETURN(0);
}


static PRBool IsSpecialGeckoKey(UInt32 macKeyCode)
{
  PRBool  isSpecial;
  
  // this table is used to determine which keys are special and should not generate a charCode
  switch (macKeyCode)
  {
    // modifiers - we don't get separate events for these yet
    case kEscapeKeyCode:
    case kShiftKeyCode:
    case kRShiftKeyCode:
    case kCommandKeyCode:
    case kRCommandKeyCode:
    case kCapsLockKeyCode:
    case kControlKeyCode:
    case kRControlKeyCode:
    case kOptionkeyCode:
    case kROptionKeyCode:
    case kClearKeyCode:
      
      // function keys
    case kF1KeyCode:
    case kF2KeyCode:
    case kF3KeyCode:
    case kF4KeyCode:
    case kF5KeyCode:
    case kF6KeyCode:
    case kF7KeyCode:
    case kF8KeyCode:
    case kF9KeyCode:
    case kF10KeyCode:
    case kF11KeyCode:
    case kF12KeyCode:
    case kPauseKeyCode:
    case kScrollLockKeyCode:
    case kPrintScreenKeyCode:
      
    case kInsertKeyCode:
    case kDeleteKeyCode:
    case kTabKeyCode:
    case kBackspaceKeyCode:
      
    case kHomeKeyCode:
    case kEndKeyCode:
    case kPageUpKeyCode:
    case kPageDownKeyCode:
    case kLeftArrowKeyCode:
    case kRightArrowKeyCode:
    case kUpArrowKeyCode:
    case kDownArrowKeyCode:
    case kReturnKeyCode:
    case kEnterKeyCode:
    case kPowerbookEnterKeyCode:
      isSpecial = PR_TRUE;
      break;
      
    default:
      isSpecial = PR_FALSE;
      break;
  }
  
  return isSpecial;
}


// Basic conversion for cocoa to gecko events, common to all conversions.
// Note that it is OK for inEvent to be nil.
- (void) convertGenericCocoaEvent:(NSEvent*)inEvent toGeckoEvent:(nsInputEvent*)outGeckoEvent
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK;

  NS_ASSERTION(outGeckoEvent, "convertGenericCocoaEvent:toGeckoEvent: requires non-null outGeckoEvent");
  if (!outGeckoEvent)
    return;

  outGeckoEvent->widget = [self widget];
  outGeckoEvent->time = PR_IntervalNow();
  outGeckoEvent->nativeMsg = inEvent;

  if (inEvent) {
    unsigned int modifiers = [inEvent modifierFlags];
    outGeckoEvent->isShift   = ((modifiers & NSShiftKeyMask) != 0);
    outGeckoEvent->isControl = ((modifiers & NSControlKeyMask) != 0);
    outGeckoEvent->isAlt     = ((modifiers & NSAlternateKeyMask) != 0);
    outGeckoEvent->isMeta    = ((modifiers & NSCommandKeyMask) != 0);
  }

  NS_OBJC_END_TRY_ABORT_BLOCK;
}


- (void) convertCocoaMouseEvent:(NSEvent*)aMouseEvent toGeckoEvent:(nsInputEvent*)outGeckoEvent
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK;

  NS_ASSERTION(aMouseEvent && outGeckoEvent, "convertCocoaMouseEvent:toGeckoEvent: requires non-null arguments");
  if (!aMouseEvent || !outGeckoEvent)
    return;

  [self convertGenericCocoaEvent:aMouseEvent toGeckoEvent:outGeckoEvent];

  // convert point to view coordinate system
  NSPoint localPoint = [self convertPoint:[aMouseEvent locationInWindow] fromView:nil];
  outGeckoEvent->refPoint.x = static_cast<nscoord>(localPoint.x);
  outGeckoEvent->refPoint.y = static_cast<nscoord>(localPoint.y);

  NS_OBJC_END_TRY_ABORT_BLOCK;
}


- (void) convertCocoaKeyEvent:(NSEvent*)aKeyEvent toGeckoEvent:(nsKeyEvent*)outGeckoEvent
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK;

  NS_ASSERTION(aKeyEvent && outGeckoEvent, "convertCocoaKeyEvent:toGeckoEvent: requires non-null arguments");
  if (!aKeyEvent || !outGeckoEvent)
    return;

  [self convertGenericCocoaEvent:aKeyEvent toGeckoEvent:outGeckoEvent];

  // coords for key events are always 0,0
  outGeckoEvent->refPoint.x = outGeckoEvent->refPoint.y = 0;

  // Initialize whether or not we are using charCodes to false.
  outGeckoEvent->isChar = PR_FALSE;

  // Check to see if the message is a key press that does not involve
  // one of our special key codes.
  if (outGeckoEvent->message == NS_KEY_PRESS && !IsSpecialGeckoKey([aKeyEvent keyCode])) {
    outGeckoEvent->isChar = PR_TRUE; // this is not a special key
    
    outGeckoEvent->charCode = 0;
    outGeckoEvent->keyCode  = 0; // not set for key press events
    
    NSString* unmodifiedChars = [aKeyEvent charactersIgnoringModifiers];
    if ([unmodifiedChars length] > 0)
      outGeckoEvent->charCode = [unmodifiedChars characterAtIndex:0];
    
    // convert control-modified charCode to raw charCode (with appropriate case)
    if (outGeckoEvent->isControl && outGeckoEvent->charCode <= 26)
      outGeckoEvent->charCode += (outGeckoEvent->isShift) ? ('A' - 1) : ('a' - 1);
    
    // gecko also wants charCode to be in the appropriate case
    if (outGeckoEvent->isShift && (outGeckoEvent->charCode >= 'a' && outGeckoEvent->charCode <= 'z'))
      outGeckoEvent->charCode -= 32; // convert to uppercase
  }
  else {
    NSString* characters = nil;
    if ([aKeyEvent type] != NSFlagsChanged)
      characters = [aKeyEvent charactersIgnoringModifiers];
    
    outGeckoEvent->keyCode = ConvertMacToGeckoKeyCode([aKeyEvent keyCode], outGeckoEvent, characters);
    outGeckoEvent->charCode = 0;
  } 

  if (outGeckoEvent->message == NS_KEY_PRESS && !outGeckoEvent->isMeta)
    [NSCursor setHiddenUntilMouseMoves:YES];

  NS_OBJC_END_TRY_ABORT_BLOCK;
}


- (nsRect)sendCompositionEvent:(PRInt32) aEventType
{
#ifdef DEBUG_IME
  NSLog(@"****in sendCompositionEvent; type = %d", aEventType);
#endif

  if (!mGeckoChild)
    return nsRect(0, 0, 0, 0);

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

  if (!mGeckoChild)
    return;

  nsTextEvent textEvent(PR_TRUE, NS_TEXT_TEXT, mGeckoChild);
  textEvent.time = PR_IntervalNow();
  textEvent.theText = aBuffer;
  if (!doCommit)
    FillTextRangeInTextEvent(&textEvent, aString, markRange, selRange);

  mGeckoChild->DispatchWindowEvent(textEvent);
  if (textEvent.rangeArray)
    delete [] textEvent.rangeArray;
}


#pragma mark -
// NSTextInput implementation

#define MAX_BUFFER_SIZE 32


- (void)insertText:(id)insertString
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK;

#if DEBUG_IME
  NSLog(@"****in insertText: '%@'", insertString);
  NSLog(@" markRange = %d, %d", mMarkedRange.location, mMarkedRange.length);
#endif
  if (!mGeckoChild)
    return;

  nsAutoRetainView kungFuDeathGrip(self);
  id arp = [[NSAutoreleasePool alloc] init];

  if (![insertString isKindOfClass:[NSAttributedString class]])
    insertString = [[[NSAttributedString alloc] initWithString:insertString] autorelease];

  NSString *tmpStr = [insertString string];
  unsigned int len = [tmpStr length];
  if (!nsTSMManager::IsComposing() && len == 0) {
    [arp release];
    return; // nothing to do
  }
  PRUnichar buffer[MAX_BUFFER_SIZE];
  PRUnichar *bufPtr = (len >= MAX_BUFFER_SIZE) ? new PRUnichar[len + 1] : buffer;
  [tmpStr getCharacters:bufPtr];
  bufPtr[len] = PRUnichar('\0');

  if (len == 1 && !nsTSMManager::IsComposing()) {
    // dispatch keypress event with char instead of textEvent
    nsKeyEvent geckoEvent(PR_TRUE, NS_KEY_PRESS, mGeckoChild);
    geckoEvent.time      = PR_IntervalNow();
    geckoEvent.charCode  = bufPtr[0]; // gecko expects OS-translated unicode
    geckoEvent.keyCode   = 0;
    geckoEvent.isChar    = PR_TRUE;
    if (mKeyDownHandled)
      geckoEvent.flags |= NS_EVENT_FLAG_NO_DEFAULT;
    // don't set other modifiers from the current event, because here in
    // -insertText: they've already been taken into account in creating
    // the input string.
        
    // create native EventRecord for use by plugins
    EventRecord macEvent;
    if (mCurKeyEvent) {
      ConvertCocoaKeyEventToMacEvent(mCurKeyEvent, macEvent);
      geckoEvent.nativeMsg = &macEvent;
      geckoEvent.isShift   = ([mCurKeyEvent modifierFlags] & NSShiftKeyMask) != 0;
      if (!IsPrintableChar(geckoEvent.charCode)) {
        geckoEvent.keyCode = 
          ConvertMacToGeckoKeyCode([mCurKeyEvent keyCode], &geckoEvent,
                                   [mCurKeyEvent charactersIgnoringModifiers]);
        geckoEvent.charCode = 0;
      }
    } else {
      // Note that insertText is not called only at key pressing.
      if (!IsPrintableChar(geckoEvent.charCode)) {
        geckoEvent.keyCode = GetGeckoKeyCodeFromChar(geckoEvent.charCode);
        geckoEvent.charCode = 0;
      }
    }

    mGeckoChild->DispatchWindowEvent(geckoEvent);
    mKeyPressSent = YES;
  }
  else {
    if (!nsTSMManager::IsComposing()) {
      [self sendCompositionEvent:NS_COMPOSITION_START];
      // Note: mGeckoChild might have become null here. Don't count on it from here on.
      nsTSMManager::StartComposing(self);
      // Note: mGeckoChild might have become null here. Don't count on it from here on.
    }

    if (nsTSMManager::IgnoreCommit()) {
      tmpStr = [tmpStr init];
      len = 0;
      bufPtr[0] = PRUnichar('\0');
      insertString =
        [[[NSAttributedString alloc] initWithString:tmpStr] autorelease];
    }
    [self sendTextEvent:bufPtr attributedString:insertString
                               selectedRange:NSMakeRange(0, len)
                               markedRange:mMarkedRange
                               doCommit:YES];
    // Note: mGeckoChild might have become null here. Don't count on it from here on.

    [self sendCompositionEvent:NS_COMPOSITION_END];
    // Note: mGeckoChild might have become null here. Don't count on it from here on.
    nsTSMManager::EndComposing();
    mMarkedRange = NSMakeRange(NSNotFound, 0);
  }

  if (bufPtr != buffer)
    delete[] bufPtr;

  [arp release];

  NS_OBJC_END_TRY_ABORT_BLOCK;
}


- (void)insertNewline:(id)sender
{
  // dummy impl, does nothing other than stop the beeping when hitting return
}


- (void) doCommandBySelector:(SEL)aSelector
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK;
#if DEBUG_IME 
  NSLog(@"**** in doCommandBySelector %s (ignore %d)", aSelector, mIgnoreDoCommand);
#endif
  if (mIgnoreDoCommand)
    return;

  if (aSelector == @selector(insertNewline:)) {
    [self insertText:@"\n"];
    return;
  }

  [super doCommandBySelector:aSelector];

  NS_OBJC_END_TRY_ABORT_BLOCK;
}


- (void) setMarkedText:(id)aString selectedRange:(NSRange)selRange
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK;

#if DEBUG_IME 
  NSLog(@"****in setMarkedText location: %d, length: %d", selRange.location, selRange.length);
  NSLog(@" markRange = %d, %d", mMarkedRange.location, mMarkedRange.length);
  NSLog(@" aString = '%@'", aString);
#endif

  nsAutoRetainView kungFuDeathGrip(self);
  id arp = [[NSAutoreleasePool alloc] init];

  if (![aString isKindOfClass:[NSAttributedString class]])
    aString = [[[NSAttributedString alloc] initWithString:aString] autorelease];

  NSMutableAttributedString *mutableAttribStr = aString;
  NSString *tmpStr = [mutableAttribStr string];
  unsigned int len = [tmpStr length];
  PRUnichar buffer[MAX_BUFFER_SIZE];
  PRUnichar *bufPtr = (len >= MAX_BUFFER_SIZE) ? new PRUnichar[len + 1] : buffer;
  [tmpStr getCharacters:bufPtr];
  bufPtr[len] = PRUnichar('\0');

#if DEBUG_IME 
  printf("****in setMarkedText, len = %d, text = ", len);
  PRUint32 n = 0;
  PRUint32 maxlen = len > 12 ? 12 : len;
  for (PRUnichar *a = bufPtr; (*a != PRUnichar('\0')) && n<maxlen; a++, n++)
    printf((*a&0xff80) ? "\\u%4X" : "%c", *a); 
  printf("\n");
#endif

  mMarkedRange.length = len;

  if (!nsTSMManager::IsComposing()) {
    nsQueryContentEvent selection(PR_TRUE, NS_QUERY_SELECTED_TEXT, mGeckoChild);
    mGeckoChild->DispatchWindowEvent(selection);
    mMarkedRange.location = selection.mSucceeded ? selection.mReply.mOffset : 0;
    [self sendCompositionEvent:NS_COMPOSITION_START];
    // Note: mGeckoChild might have become null here. Don't count on it from here on.
    nsTSMManager::StartComposing(self);
    // Note: mGeckoChild might have become null here. Don't count on it from here on.
  }

  nsTSMManager::UpdateComposing(tmpStr);

  [self sendTextEvent:bufPtr attributedString:aString
                             selectedRange:selRange
                             markedRange:mMarkedRange
                             doCommit:NO];
  // Note: mGeckoChild might have become null here. Don't count on it from here on.

  if (nsTSMManager::IsComposing() && len == 0) {
    nsTSMManager::CommitIME();    
    // Note: mGeckoChild might have become null here. Don't count on it from here on.
  }
  
  if (bufPtr != buffer)
    delete[] bufPtr;

  [arp release];

  NS_OBJC_END_TRY_ABORT_BLOCK;
}


- (void) unmarkText
{
#if DEBUG_IME
  NSLog(@"****in unmarkText");
  NSLog(@" markedRange   = %d, %d", mMarkedRange.location, mMarkedRange.length);
#endif
  nsTSMManager::CommitIME();
}


- (BOOL) hasMarkedText
{
#if DEBUG_IME
  NSLog(@"****in hasMarkText");
  NSLog(@" markedRange   = %d, %d", mMarkedRange.location, mMarkedRange.length);
#endif
  return (mMarkedRange.location != NSNotFound) && (mMarkedRange.length != 0);
}


- (long) conversationIdentifier
{
#if DEBUG_IME
  NSLog(@"****in conversationIdentifier");
#endif
  if (!mGeckoChild)
    return (long)self;
  nsQueryContentEvent textContent(PR_TRUE, NS_QUERY_TEXT_CONTENT, mGeckoChild);
  textContent.InitForQueryTextContent(0, 0);
  mGeckoChild->DispatchWindowEvent(textContent);
  if (!textContent.mSucceeded)
    return (long)self;
#if DEBUG_IME
  NSLog(@" the ID = %ld", (long)textContent.mReply.mContentsRoot);
#endif
  return (long)textContent.mReply.mContentsRoot;
}


- (NSAttributedString *) attributedSubstringFromRange:(NSRange)theRange
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK_NIL;

#if DEBUG_IME
  NSLog(@"****in attributedSubstringFromRange");
  NSLog(@" theRange      = %d, %d", theRange.location, theRange.length);
  NSLog(@" markedRange   = %d, %d", mMarkedRange.location, mMarkedRange.length);
#endif
  if (!mGeckoChild || theRange.length == 0)
    return nil;

  nsAutoString str;
  nsQueryContentEvent textContent(PR_TRUE, NS_QUERY_TEXT_CONTENT, mGeckoChild);
  textContent.InitForQueryTextContent(theRange.location, theRange.length);
  mGeckoChild->DispatchWindowEvent(textContent);

  if (!textContent.mSucceeded || textContent.mReply.mString.IsEmpty())
    return nil;

  NSString* nsstr =
    [NSString stringWithCharacters:textContent.mReply.mString.get()
                            length:textContent.mReply.mString.Length()];
  NSAttributedString* result =
    [[[NSAttributedString alloc] initWithString:nsstr
                                     attributes:nil] autorelease];
  return result;

  NS_OBJC_END_TRY_ABORT_BLOCK_NIL;
}


- (NSRange) markedRange
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK_RETURN;

#if DEBUG_IME
  NSLog(@"****in markedRange");
  NSLog(@" markedRange   = %d, %d", mMarkedRange.location, mMarkedRange.length);
#endif

  if (![self hasMarkedText]) {
    return NSMakeRange(NSNotFound, 0);
  }

  return mMarkedRange;

  NS_OBJC_END_TRY_ABORT_BLOCK_RETURN(NSMakeRange(0, 0));
}


- (NSRange) selectedRange
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK_RETURN;

#if DEBUG_IME
  NSLog(@"****in selectedRange");
  NSLog(@" markedRange   = %d, %d", mMarkedRange.location, mMarkedRange.length);
#endif
  if (!mGeckoChild)
    return NSMakeRange(NSNotFound, 0);
  nsQueryContentEvent selection(PR_TRUE, NS_QUERY_SELECTED_TEXT, mGeckoChild);
  mGeckoChild->DispatchWindowEvent(selection);
  if (!selection.mSucceeded)
    return NSMakeRange(NSNotFound, 0);

#if DEBUG_IME
  NSLog(@" result of selectedRange = %d, %d",
        selection.mReply.mOffset, selection.mReply.mString.Length());
#endif
  return NSMakeRange(selection.mReply.mOffset,
                     selection.mReply.mString.Length());

  NS_OBJC_END_TRY_ABORT_BLOCK_RETURN(NSMakeRange(0, 0));
}


- (NSRect) firstRectForCharacterRange:(NSRange)theRange
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK_RETURN;

#if DEBUG_IME
  NSLog(@"****in firstRectForCharacterRange");
  NSLog(@" theRange      = %d, %d", theRange.location, theRange.length);
  NSLog(@" markedRange   = %d, %d", mMarkedRange.location, mMarkedRange.length);
#endif
  // XXX this returns first character rect or caret rect, it is limitation of
  // now. We need more work for returns first line rect. But current
  // implementation is enough for IMEs.

  NSRect rect;
  if (!mGeckoChild || theRange.location == NSNotFound)
    return rect;

  nsRect r;
  PRBool useCaretRect = theRange.length == 0;
  if (!useCaretRect) {
    nsQueryContentEvent charRect(PR_TRUE, NS_QUERY_CHARACTER_RECT, mGeckoChild);
    charRect.InitForQueryCharacterRect(theRange.location);
    mGeckoChild->DispatchWindowEvent(charRect);
    if (charRect.mSucceeded)
      r = charRect.mReply.mRect;
    else
      useCaretRect = PR_TRUE;
  }

  if (useCaretRect) {
    nsQueryContentEvent caretRect(PR_TRUE, NS_QUERY_CARET_RECT, mGeckoChild);
    caretRect.InitForQueryCaretRect(theRange.location);
    mGeckoChild->DispatchWindowEvent(caretRect);
    if (!caretRect.mSucceeded)
      return rect;
    r = caretRect.mReply.mRect;
    r.width = 0;
  }

  nsIWidget* rootWidget = mGeckoChild->GetTopLevelWidget();
  NSWindow* rootWindow =
    static_cast<NSWindow*>(rootWidget->GetNativeData(NS_NATIVE_WINDOW));
  NSView* rootView =
    static_cast<NSView*>(rootWidget->GetNativeData(NS_NATIVE_WIDGET));
  if (!rootWindow || !rootView)
    return rect;
  GeckoRectToNSRect(r, rect);
  rect = [rootView convertRect:rect toView:nil];
  rect.origin = [rootWindow convertBaseToScreen:rect.origin];
#if DEBUG_IME
  NSLog(@" result rect (x,y,w,h) = %f, %f, %f, %f",
        rect.origin.x, rect.origin.y, rect.size.width, rect.size.height);
#endif
  return rect;

  NS_OBJC_END_TRY_ABORT_BLOCK_RETURN(NSMakeRect(0.0, 0.0, 0.0, 0.0));
}


- (unsigned int)characterIndexForPoint:(NSPoint)thePoint
{
#if DEBUG_IME
  NSLog(@"****in characterIndexForPoint");
  NSLog(@" markRange = %d, %d", mMarkedRange.location, mMarkedRange.length);
#endif

  // To implement this, we'd have to grovel in text frames looking at text offsets.
  return 0;
}


- (NSArray*) validAttributesForMarkedText
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK_NIL;

#if DEBUG_IME
  NSLog(@"****in validAttributesForMarkedText");
  NSLog(@" markRange = %d, %d", mMarkedRange.location, mMarkedRange.length);
#endif

  //return [NSArray arrayWithObjects:NSUnderlineStyleAttributeName, NSMarkedClauseSegmentAttributeName, NSTextInputReplacementRangeAttributeName, nil];
  return [NSArray array]; // empty array; we don't support any attributes right now

  NS_OBJC_END_TRY_ABORT_BLOCK_NIL;
}


#pragma mark -


+ (NSEvent*)makeNewCocoaEventWithType:(NSEventType)type fromEvent:(NSEvent*)theEvent
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK_NIL;

  NSEvent* newEvent = [NSEvent keyEventWithType:type
                                       location:[theEvent locationInWindow] 
                                  modifierFlags:[theEvent modifierFlags]
                                      timestamp:[theEvent timestamp]
                                   windowNumber:[theEvent windowNumber]
                                        context:[theEvent context]
                                     characters:[theEvent characters]
                    charactersIgnoringModifiers:[theEvent charactersIgnoringModifiers]
                                      isARepeat:[theEvent isARepeat]
                                        keyCode:[theEvent keyCode]];
  return newEvent;

  NS_OBJC_END_TRY_ABORT_BLOCK_NIL;
}


- (void)processKeyDownEvent:(NSEvent*)theEvent keyEquiv:(BOOL)isKeyEquiv
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK;

  if (!mGeckoChild)
    return;

  nsAutoRetainView kungFuDeathGrip(self);
  mCurKeyEvent = theEvent;

  BOOL nonDeadKeyPress = [[theEvent characters] length] > 0;
  if (nonDeadKeyPress) {
    if (![theEvent isARepeat]) {
      NSResponder* firstResponder = [[self window] firstResponder];

      nsKeyEvent geckoEvent(PR_TRUE, NS_KEY_DOWN, nsnull);
      [self convertCocoaKeyEvent:theEvent toGeckoEvent:&geckoEvent];

      // create native EventRecord for use by plugins
      EventRecord macEvent;
      ConvertCocoaKeyEventToMacEvent(theEvent, macEvent);
      geckoEvent.nativeMsg = &macEvent;

      mKeyDownHandled = mGeckoChild->DispatchWindowEvent(geckoEvent);
      if (!mGeckoChild)
        return;

      // The key down event may have shifted the focus, in which
      // case we should not fire the key press.
      if (firstResponder != [[self window] firstResponder]) {
        mCurKeyEvent = nil;
        mKeyDownHandled = PR_FALSE;
        return;
      }
    }

    nsKeyEvent geckoEvent(PR_TRUE, NS_KEY_PRESS, nsnull);
    [self convertCocoaKeyEvent:theEvent toGeckoEvent:&geckoEvent];

    // if this is a non-letter keypress, or the control key is down,
    // dispatch the keydown to gecko, so that we trap delete,
    // control-letter combinations etc before Cocoa tries to use
    // them for keybindings.
    if ((!geckoEvent.isChar || geckoEvent.isControl) &&
        !nsTSMManager::IsComposing()) {
      if (mKeyDownHandled)
        geckoEvent.flags |= NS_EVENT_FLAG_NO_DEFAULT;

      // create native EventRecord for use by plugins
      EventRecord macEvent;
      ConvertCocoaKeyEventToMacEvent(theEvent, macEvent);
      geckoEvent.nativeMsg = &macEvent;

      mIgnoreDoCommand = mGeckoChild->DispatchWindowEvent(geckoEvent);
      if (!mGeckoChild)
        return;
      mKeyPressSent = YES;
    }
  }

  // Let Cocoa interpret the key events, caching IsComposing first.
  // We don't do it if this came from performKeyEquivalent because
  // interpretKeyEvents isn't set up to handle those key combinations.
  PRBool wasComposing = nsTSMManager::IsComposing();
  if (!isKeyEquiv)
    [super interpretKeyEvents:[NSArray arrayWithObject:theEvent]];

  if (!mGeckoChild)
    return;

  if (!mKeyPressSent && nonDeadKeyPress && !wasComposing && !nsTSMManager::IsComposing()) {
    nsKeyEvent geckoEvent(PR_TRUE, NS_KEY_PRESS, nsnull);
    [self convertCocoaKeyEvent:theEvent toGeckoEvent:&geckoEvent];
    if (mKeyDownHandled)
      geckoEvent.flags |= NS_EVENT_FLAG_NO_DEFAULT;

    // create native EventRecord for use by plugins
    EventRecord macEvent;
    ConvertCocoaKeyEventToMacEvent(theEvent, macEvent);
    geckoEvent.nativeMsg = &macEvent;

    mGeckoChild->DispatchWindowEvent(geckoEvent);    
  }

  // Note: mGeckoChild might have become null here. Don't count on it from here on.

  // See note about nested event loops where these variables are declared in header.
  mIgnoreDoCommand = NO;
  mKeyPressSent = NO;
  mCurKeyEvent = nil;
  mKeyDownHandled = PR_FALSE;

  NS_OBJC_END_TRY_ABORT_BLOCK;
}


- (void)keyDown:(NSEvent*)theEvent
{
  [self processKeyDownEvent:theEvent keyEquiv:NO];
}


static BOOL keyUpAlreadySentKeyDown = NO;

- (void)keyUp:(NSEvent*)theEvent
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK;

  // if we don't have any characters we can't generate a keyUp event
  if (!mGeckoChild || [[theEvent characters] length] == 0)
    return;

  nsAutoRetainView kungFuDeathGrip(self);

  // Cocoa doesn't send an NSKeyDown event for control-tab on 10.4, so if this
  // is an NSKeyUp event for control-tab, send a down event to gecko first.
  if (!nsToolkit::OnLeopardOrLater() && !keyUpAlreadySentKeyDown &&
      [theEvent modifierFlags] & NSControlKeyMask && [theEvent keyCode] == kTabKeyCode) {
    // We'll need an NSKeyDown copy of our native event so we convert to a gecko event correctly.
    NSEvent* nativeKeyDownEvent = [ChildView makeNewCocoaEventWithType:NSKeyDown fromEvent:theEvent];

    // send a key down event if we should
    PRBool keyDownHandled = PR_FALSE;
    if (![nativeKeyDownEvent isARepeat]) {
      nsKeyEvent geckoEvent(PR_TRUE, NS_KEY_DOWN, nsnull);
      [self convertCocoaKeyEvent:nativeKeyDownEvent toGeckoEvent:&geckoEvent];

      // create native EventRecord for use by plugins
      EventRecord macEvent;
      ConvertCocoaKeyEventToMacEvent(nativeKeyDownEvent, macEvent);
      geckoEvent.nativeMsg = &macEvent;

      keyDownHandled = mGeckoChild->DispatchWindowEvent(geckoEvent);
      if (!mGeckoChild)
        return;
    }

    // Check to see if we are still the first responder.
    // The key down event may have shifted the focus, in which
    // case we should not fire the key press.
    NSResponder* resp = [[self window] firstResponder];
    if (resp != (NSResponder*)self) {
      keyUpAlreadySentKeyDown = YES;
      [resp keyUp:theEvent];      
      keyUpAlreadySentKeyDown = NO;
      return;
    }

    // now send a key press event if we should
    if (!nsTSMManager::IsComposing()) {
      nsKeyEvent geckoEvent(PR_TRUE, NS_KEY_PRESS, nsnull);
      [self convertCocoaKeyEvent:nativeKeyDownEvent toGeckoEvent:&geckoEvent];

      if (keyDownHandled)
        geckoEvent.flags |= NS_EVENT_FLAG_NO_DEFAULT;

      // create native EventRecord for use by plugins
      EventRecord macEvent;
      ConvertCocoaKeyEventToMacEvent(nativeKeyDownEvent, macEvent);
      geckoEvent.nativeMsg = &macEvent;

      mGeckoChild->DispatchWindowEvent(geckoEvent);
      if (!mGeckoChild)
        return;
    }
  }

  nsKeyEvent geckoEvent(PR_TRUE, NS_KEY_UP, nsnull);
  [self convertCocoaKeyEvent:theEvent toGeckoEvent:&geckoEvent];

  // create native EventRecord for use by plugins
  EventRecord macEvent;
  ConvertCocoaKeyEventToMacEvent(theEvent, macEvent);
  geckoEvent.nativeMsg = &macEvent;

  mGeckoChild->DispatchWindowEvent(geckoEvent);

  NS_OBJC_END_TRY_ABORT_BLOCK;
}


- (BOOL)performKeyEquivalent:(NSEvent*)theEvent
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK_RETURN;

  // First we need to ignore certain system commands. If we don't we'll have
  // to duplicate their functionality in Gecko, which as of this time we haven't.
  // The only thing we ignore now is command-tilde, because NSApp handles that for us
  // and we need the event to propagate to there.
  unsigned int modifierFlags = [theEvent modifierFlags] & NSDeviceIndependentModifierFlagsMask;
  if (modifierFlags & NSCommandKeyMask && [theEvent keyCode] == kTildeKeyCode)
    return NO;

  // don't do anything if we don't have a gecko widget
  if (!mGeckoChild)
    return NO;

  // if we aren't the first responder, pass the event on
  if ([[self window] firstResponder] != self)
    return [super performKeyEquivalent:theEvent];

  // don't process if we're composing, but don't consume the event
  if (nsTSMManager::IsComposing())
    return NO;

  // see if the menu system will handle the event
  if ([[NSApp mainMenu] performKeyEquivalent:theEvent]) {
    return YES;
  }
  else {
    // On Mac OS X 10.5 NSMenu's performKeyEquivalent: method returns NO for disabled menu
    // items that have a matching key equiv. We need to know if that was the case so we can
    // stop here like we would on 10.4 (it returns YES in that case). Since we want to eat
    // the event if that happens the system won't give the disabled command beep, do it here
    // manually.
    if (nsToolkit::OnLeopardOrLater()) {
      id delegate = [[self window] delegate];
      if (delegate && [delegate isKindOfClass:[WindowDelegate class]]) {
        nsCocoaWindow* toplevelWindow = [delegate geckoWidget];
        if (toplevelWindow) {
          nsMenuBarX* menuBar = static_cast<nsMenuBarX*>(toplevelWindow->GetMenuBar());
          if (menuBar && menuBar->ContainsKeyEquiv(modifierFlags, [theEvent characters])) {
            NSBeep();
            return YES;
          }
        }
      }
    }
  }

  // don't handle this if certain modifiers are down - those should
  // be sent as normal key up/down events and cocoa will do so automatically
  // if we reject here
  if ((modifierFlags & NSFunctionKeyMask) || (modifierFlags & NSNumericPadKeyMask))
    return NO;

  if ([theEvent type] == NSKeyDown)
    [self processKeyDownEvent:theEvent keyEquiv:YES];

  return YES;

  NS_OBJC_END_TRY_ABORT_BLOCK_RETURN(NO);
}


- (void)flagsChanged:(NSEvent*)theEvent
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK;

  if (!mGeckoChild)
    return;

  nsAutoRetainView kungFuDeathGrip(self);

  // Fire key up/down events for the modifier keys (shift, alt, ctrl, command).
  if ([theEvent type] == NSFlagsChanged) {
    unsigned int modifiers =
      [theEvent modifierFlags] & NSDeviceIndependentModifierFlagsMask;
    const PRUint32 kModifierMaskTable[] =
      { NSShiftKeyMask, NSControlKeyMask, NSAlternateKeyMask, NSCommandKeyMask };
    const PRUint32 kModifierCount = sizeof(kModifierMaskTable) /
                                    sizeof(kModifierMaskTable[0]);

    for (PRUint32 i = 0; i < kModifierCount; i++) {
      PRUint32 modifierBit = kModifierMaskTable[i];
      if ((modifiers & modifierBit) != (mLastModifierState & modifierBit)) {
        PRUint32 message = ((modifiers & modifierBit) != 0 ? NS_KEY_DOWN :
                                                             NS_KEY_UP);

        // Fire a key event.
        nsKeyEvent geckoEvent(PR_TRUE, message, nsnull);
        [self convertCocoaKeyEvent:theEvent toGeckoEvent:&geckoEvent];

        // create native EventRecord for use by plugins
        EventRecord macEvent;
        ConvertCocoaKeyEventToMacEvent(theEvent, macEvent, message);
        geckoEvent.nativeMsg = &macEvent;

        mGeckoChild->DispatchWindowEvent(geckoEvent);
        if (!mGeckoChild)
          return;

        // Stop if focus has changed.
        // Check to see if we are still the first responder.
        NSResponder* resp = [[self window] firstResponder];
        if (resp != (NSResponder*)self)
          break;
      }
    }

    mLastModifierState = modifiers;
  }

  // check if the hand scroll cursor needs to be set/unset
  [self setHandScrollCursor:theEvent];

  NS_OBJC_END_TRY_ABORT_BLOCK;
}


// This method is called when are are about to lose focus.
// We must always call through to our superclass, even when mGeckoChild is
// nil -- otherwise the keyboard focus can end up in the wrong NSView.
- (BOOL)resignFirstResponder
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK_RETURN;

  return [super resignFirstResponder];

  NS_OBJC_END_TRY_ABORT_BLOCK_RETURN(NO);
}


- (void)viewsWindowDidBecomeKey
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK;

  if (!mGeckoChild)
    return;

  nsAutoRetainView kungFuDeathGrip(self);

  // check to see if the window implements the mozWindow protocol. This
  // allows embedders to avoid re-entrant calls to -makeKeyAndOrderFront,
  // which can happen because these activate/focus calls propagate out
  // to the embedder via nsIEmbeddingSiteWindow::SetFocus().
  BOOL isMozWindow = [[self window] respondsToSelector:@selector(setSuppressMakeKeyFront:)];
  if (isMozWindow)
    [[self window] setSuppressMakeKeyFront:YES];

  [self sendFocusEvent:NS_GOTFOCUS];
  [self sendFocusEvent:NS_ACTIVATE];

  if (isMozWindow)
    [[self window] setSuppressMakeKeyFront:NO];

  NS_OBJC_END_TRY_ABORT_BLOCK;
}


- (void)viewsWindowDidResignKey
{
  if (!mGeckoChild)
    return;

  nsAutoRetainView kungFuDeathGrip(self);

  [self sendFocusEvent:NS_DEACTIVATE];
  [self sendFocusEvent:NS_LOSTFOCUS];
}


// If the call to removeFromSuperview isn't delayed from nsChildView::
// TearDownView(), the NSView hierarchy might get changed during calls to
// [ChildView drawRect:], which leads to "beyond bounds" exceptions in
// NSCFArray.  For more info see bmo bug 373122.  Apple's docs claim that
// removeFromSuperviewWithoutNeedingDisplay "can be safely invoked during
// display" (whatever "display" means).  But it's _not_ true that it can be
// safely invoked during calls to [NSView drawRect:].  We use
// removeFromSuperview here because there's no longer any danger of being
// "invoked during display", and because doing do clears up bmo bug 384343.
- (void)delayedTearDown
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK;

  [self removeFromSuperview];
  [self release];

  NS_OBJC_END_TRY_ABORT_BLOCK;
}


#pragma mark -


// drag'n'drop stuff
#define kDragServiceContractID "@mozilla.org/widget/dragservice;1"


// This is a utility function used by NSView drag event methods
// to send events. It contains all of the logic needed for Gecko
// dragging to work. Returns YES if the event was handled, NO
// if it wasn't.
- (BOOL)doDragAction:(PRUint32)aMessage sender:(id)aSender
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK_RETURN;

  if (!mGeckoChild)
    return NO;

  PR_LOG(sCocoaLog, PR_LOG_ALWAYS, ("ChildView doDragAction: entered\n"));

  if (!mDragService) {
    CallGetService(kDragServiceContractID, &mDragService);
    NS_ASSERTION(mDragService, "Couldn't get a drag service - big problem!");
    if (!mDragService)
      return NO;
  }

  if (aMessage == NS_DRAGDROP_ENTER)
    mDragService->StartDragSession();

  nsCOMPtr<nsIDragSession> dragSession;
  mDragService->GetCurrentSession(getter_AddRefs(dragSession));
  if (dragSession) {
    if (aMessage == NS_DRAGDROP_OVER) {
      // fire the drag event at the source. Just ignore whether it was
      // cancelled or not as there isn't actually a means to stop the drag
      mDragService->FireDragEventAtSource(NS_DRAGDROP_DRAG);
      dragSession->SetCanDrop(PR_FALSE);
    }
    else if (aMessage == NS_DRAGDROP_DROP) {
      // We make the assuption that the dragOver handlers have correctly set
      // the |canDrop| property of the Drag Session.
      PRBool canDrop = PR_FALSE;
      if (!NS_SUCCEEDED(dragSession->GetCanDrop(&canDrop)) || !canDrop)
        return NO;
    }
    
    unsigned int modifierFlags = [[NSApp currentEvent] modifierFlags];
    PRUint32 action = nsIDragService::DRAGDROP_ACTION_MOVE;
    // force copy = option, alias = cmd-option, default is move
    if (modifierFlags & NSAlternateKeyMask) {
      if (modifierFlags & NSCommandKeyMask)
        action = nsIDragService::DRAGDROP_ACTION_LINK;
      else
        action = nsIDragService::DRAGDROP_ACTION_COPY;
    }
    dragSession->SetDragAction(action);
  }

  // set up gecko event
  nsMouseEvent geckoEvent(PR_TRUE, aMessage, nsnull, nsMouseEvent::eReal);
  [self convertGenericCocoaEvent:nil toGeckoEvent:&geckoEvent];

  // Use our own coordinates in the gecko event.
  // Convert event from gecko global coords to gecko view coords.
  NSPoint localPoint = [self convertPoint:[aSender draggingLocation] fromView:nil];
  geckoEvent.refPoint.x = static_cast<nscoord>(localPoint.x);
  geckoEvent.refPoint.y = static_cast<nscoord>(localPoint.y);

  nsAutoRetainView kungFuDeathGrip(self);
  mGeckoChild->DispatchWindowEvent(geckoEvent);
  if (!mGeckoChild)
    return YES;

  if (aMessage == NS_DRAGDROP_EXIT && dragSession) {
    nsCOMPtr<nsIDOMNode> sourceNode;
    dragSession->GetSourceNode(getter_AddRefs(sourceNode));
    if (!sourceNode) {
      // We're leaving a window while doing a drag that was
      // initiated in a different app. End the drag session,
      // since we're done with it for now (until the user
      // drags back into mozilla).
      mDragService->EndDragSession(PR_FALSE);
    }
  }

  return YES;

  NS_OBJC_END_TRY_ABORT_BLOCK_RETURN(NO);
}


- (NSDragOperation)draggingEntered:(id <NSDraggingInfo>)sender
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK_RETURN;

  PR_LOG(sCocoaLog, PR_LOG_ALWAYS, ("ChildView draggingEntered: entered\n"));
  
  // there should never be a globalDragPboard when "draggingEntered:" is
  // called, but just in case we'll take care of it here.
  [globalDragPboard release];

  // Set the global drag pasteboard that will be used for this drag session.
  // This will be set back to nil when the drag session ends (mouse exits
  // the view or a drop happens within the view).
  globalDragPboard = [[sender draggingPasteboard] retain];

  BOOL handled = [self doDragAction:NS_DRAGDROP_ENTER sender:sender];

  return handled ? NSDragOperationGeneric : NSDragOperationNone;

  NS_OBJC_END_TRY_ABORT_BLOCK_RETURN(NSDragOperationNone);
}


- (NSDragOperation)draggingUpdated:(id <NSDraggingInfo>)sender
{
  PR_LOG(sCocoaLog, PR_LOG_ALWAYS, ("ChildView draggingUpdated: entered\n"));

  BOOL handled = [self doDragAction:NS_DRAGDROP_OVER sender:sender];
  return handled ? NSDragOperationGeneric : NSDragOperationNone;
}


- (void)draggingExited:(id <NSDraggingInfo>)sender
{
  PR_LOG(sCocoaLog, PR_LOG_ALWAYS, ("ChildView draggingExited: entered\n"));

  nsAutoRetainView kungFuDeathGrip(self);
  [self doDragAction:NS_DRAGDROP_EXIT sender:sender];
  NS_IF_RELEASE(mDragService);
}


- (BOOL)performDragOperation:(id <NSDraggingInfo>)sender
{
  nsAutoRetainView kungFuDeathGrip(self);
  BOOL handled = [self doDragAction:NS_DRAGDROP_DROP sender:sender];
  NS_IF_RELEASE(mDragService);
  return handled;
}


// NSDraggingSource
- (void)draggedImage:(NSImage *)anImage endedAt:(NSPoint)aPoint operation:(NSDragOperation)operation
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK;

  gDraggedTransferables = nsnull;

  if (!mDragService) {
    CallGetService(kDragServiceContractID, &mDragService);
    NS_ASSERTION(mDragService, "Couldn't get a drag service - big problem!");
  }

  if (mDragService) {
    mDragService->EndDragSession(PR_TRUE);
    NS_RELEASE(mDragService);
  }

  [globalDragPboard release];
  globalDragPboard = nil;

  NS_OBJC_END_TRY_ABORT_BLOCK;
}


// NSDraggingSource
// this is just implemented so we comply with the NSDraggingSource informal protocol
- (unsigned int)draggingSourceOperationMaskForLocal:(BOOL)isLocal
{
  return UINT_MAX;
}

// This method is a callback typically invoked in response to a drag ending on the desktop
// or a Findow folder window; the argument passed is a path to the drop location, to be used
// in constructing a complete pathname for the file(s) we want to create as a result of
// the drag.
- (NSArray *)namesOfPromisedFilesDroppedAtDestination:(id <NSDraggingInfo>)dropDestination
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK_NIL;

  nsresult rv;

  PR_LOG(sCocoaLog, PR_LOG_ALWAYS, ("ChildView namesOfPromisedFilesDroppedAtDestination: entering callback for promised files\n"));

  nsCOMPtr<nsILocalFile> targFile;
  NS_NewLocalFile(EmptyString(), PR_TRUE, getter_AddRefs(targFile));
  nsCOMPtr<nsILocalFileMac> macLocalFile = do_QueryInterface(targFile);
  if (!macLocalFile) {
    NS_ERROR("No Mac local file");
    return nil;
  }

  if (!NS_SUCCEEDED(macLocalFile->InitWithCFURL((CFURLRef)dropDestination))) {
    NS_ERROR("failed InitWithCFURL");
    return nil;
  }

  if (!gDraggedTransferables)
    return nil;

  PRUint32 transferableCount;
  rv = gDraggedTransferables->Count(&transferableCount);
  if (NS_FAILED(rv))
    return nil;

  for (PRUint32 i = 0; i < transferableCount; i++) {
    nsCOMPtr<nsISupports> genericItem;
    gDraggedTransferables->GetElementAt(i, getter_AddRefs(genericItem));
    nsCOMPtr<nsITransferable> item(do_QueryInterface(genericItem));
    if (!item) {
      NS_ERROR("no transferable");
      return nil;
    }

    item->SetTransferData(kFilePromiseDirectoryMime, macLocalFile, sizeof(nsILocalFile*));
    
    // now request the kFilePromiseMime data, which will invoke the data provider
    // If successful, the returned data is a reference to the resulting file.
    nsCOMPtr<nsISupports> fileDataPrimitive;
    PRUint32 dataSize = 0;
    item->GetTransferData(kFilePromiseMime, getter_AddRefs(fileDataPrimitive), &dataSize);
  }
  
  NSPasteboard* generalPboard = [NSPasteboard pasteboardWithName:NSDragPboard];
  NSData* data = [generalPboard dataForType:@"application/x-moz-file-promise-dest-filename"];
  NSString* name = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
  NSArray* rslt = [NSArray arrayWithObject:name];

  [name release];

  return rslt;

  NS_OBJC_END_TRY_ABORT_BLOCK_NIL;
}

#pragma mark -


#ifdef ACCESSIBILITY

/* Every ChildView has a corresponding mozDocAccessible object that is doing all
   the heavy lifting. The topmost ChildView corresponds to a mozRootAccessible
   object.

   All ChildView needs to do is to route all accessibility calls (from the NSAccessibility APIs)
   down to its object, pretending that they are the same.
*/
- (id<mozAccessible>)accessible
{
  if (!mGeckoChild)
    return nil;

  id<mozAccessible> nativeAccessible = nil;

  nsAutoRetainView kungFuDeathGrip(self);
  nsCOMPtr<nsIWidget> kungFuDeathGrip2(mGeckoChild);
  nsCOMPtr<nsIAccessible> accessible;
  mGeckoChild->GetDocumentAccessible(getter_AddRefs(accessible));
  if (!mGeckoChild)
    return nil;

  if (accessible)
    accessible->GetNativeInterface((void**)&nativeAccessible);

#ifdef DEBUG_hakan
  NSAssert(![nativeAccessible isExpired], @"native acc is expired!!!");
#endif
  
  return nativeAccessible;
}

/* Implementation of formal mozAccessible formal protocol (enabling mozViews
   to talk to mozAccessible objects in the accessibility module). */

- (BOOL)hasRepresentedView
{
  return YES;
}

- (id)representedView
{
  return self;
}

- (BOOL)isRoot
{
  return [[self accessible] isRoot];
}

#ifdef DEBUG
- (void)printHierarchy
{
  [[self accessible] printHierarchy];
}
#endif

#pragma mark -

// general

- (BOOL)accessibilityIsIgnored
{
  return [[self accessible] accessibilityIsIgnored];
}

- (id)accessibilityHitTest:(NSPoint)point
{
  return [[self accessible] accessibilityHitTest:point];
}

- (id)accessibilityFocusedUIElement
{
  return [[self accessible] accessibilityFocusedUIElement];
}

// actions

- (NSArray*)accessibilityActionNames
{
  return [[self accessible] accessibilityActionNames];
}

- (NSString*)accessibilityActionDescription:(NSString*)action
{
  return [[self accessible] accessibilityActionDescription:action];
}

- (void)accessibilityPerformAction:(NSString*)action
{
  return [[self accessible] accessibilityPerformAction:action];
}

// attributes

- (NSArray*)accessibilityAttributeNames
{
  return [[self accessible] accessibilityAttributeNames];
}

- (BOOL)accessibilityIsAttributeSettable:(NSString*)attribute
{
  return [[self accessible] accessibilityIsAttributeSettable:attribute];
}

- (id)accessibilityAttributeValue:(NSString*)attribute
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK_NIL;

  id<mozAccessible> accessible = [self accessible];
  
  // if we're the root (topmost) accessible, we need to return our native AXParent as we
  // traverse outside to the hierarchy of whoever embeds us. thus, fall back on NSView's
  // default implementation for this attribute.
  if ([attribute isEqualToString:NSAccessibilityParentAttribute] && [accessible isRoot]) {
    id parentAccessible = [super accessibilityAttributeValue:attribute];
    return parentAccessible;
  }

  return [accessible accessibilityAttributeValue:attribute];

  NS_OBJC_END_TRY_ABORT_BLOCK_NIL;
}

#endif /* ACCESSIBILITY */

@end


#pragma mark -


PRBool
nsTSMManager::GetIMEOpenState()
{
  return GetScriptManagerVariable(smKeyScript) != smRoman ? PR_TRUE : PR_FALSE;
}


void
nsTSMManager::StartComposing(NSView<mozView>* aComposingView)
{
  if (sComposingView && sComposingView != sComposingView)
    CommitIME();
  sComposingView = aComposingView;
  sDocumentID = ::TSMGetActiveDocument();
}


void
nsTSMManager::UpdateComposing(NSString* aComposingString)
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK;

  sComposingString = aComposingString;
  [sComposingString retain];

  NS_OBJC_END_TRY_ABORT_BLOCK;
}


void
nsTSMManager::EndComposing()
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK;

  sComposingView = nsnull;
  if (sComposingString) {
    [sComposingString release];
    sComposingString = nsnull;
  }
  sDocumentID = nsnull;

  NS_OBJC_END_TRY_ABORT_BLOCK;
}


void
nsTSMManager::EnableIME(PRBool aEnable)
{
  if (aEnable == sIsIMEEnabled)
    return;
  CommitIME();
  sIsIMEEnabled = aEnable;
}


void
nsTSMManager::SetIMEOpenState(PRBool aOpen)
{
  if (aOpen == GetIMEOpenState())
    return;
  CommitIME();
  KeyScript(aOpen ? smKeySwapScript : smKeyRoman);
}


#define ENABLE_ROMAN_KYBDS_ONLY -23
void
nsTSMManager::SetRomanKeyboardsOnly(PRBool aRomanOnly)
{
  if (aRomanOnly == sIsRomanKeyboardsOnly)
    return;
  CommitIME();
  KeyScript(aRomanOnly ? ENABLE_ROMAN_KYBDS_ONLY : smKeyEnableKybds);
  sIsRomanKeyboardsOnly = aRomanOnly;
}


void
nsTSMManager::KillComposing()
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK;

  // Force commit the current composition
  // XXX Don't use NSInputManager. Because it cannot control the non-forcused
  // input manager, therefore, on deactivating a window, it does not work fine.
  NS_ASSERTION(sDocumentID, "The TSMDocumentID is null");
  ::FixTSMDocument(sDocumentID);

  NS_OBJC_END_TRY_ABORT_BLOCK;
}


void
nsTSMManager::CommitIME()
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK;

  if (!IsComposing())
    return;
  KillComposing();
  if (!IsComposing())
    return;
  // If the composing transaction is still there, KillComposing only kills the
  // composing in TSM. We also need to kill the our composing transaction too.
  NSAttributedString* str =
    [[NSAttributedString alloc] initWithString:sComposingString];
  [sComposingView insertText:str];
  [str release];

  NS_OBJC_END_TRY_ABORT_BLOCK;
}


void
nsTSMManager::CancelIME()
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK;

  if (!IsComposing())
    return;
  // For canceling the current composing, we need to ignore the param of
  // insertText. But this code is ugly...
  sIgnoreCommit = PR_TRUE;
  KillComposing();
  sIgnoreCommit = PR_FALSE;
  if (!IsComposing())
    return;
  // If the composing transaction is still there, KillComposing only kills the
  // composing in TSM. We also need to kill the our composing transaction too.
  NSAttributedString* str = [[NSAttributedString alloc] initWithString:@""];
  [sComposingView insertText:str];
  [str release];

  NS_OBJC_END_TRY_ABORT_BLOCK;
}
