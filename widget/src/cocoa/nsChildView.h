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

#ifndef nsChildView_h_
#define nsChildView_h_

// formal protocols
#include "mozView.h"
#ifdef ACCESSIBILITY
#include "nsIAccessible.h"
#include "mozAccessibleProtocol.h"
#endif

#include "nsAutoPtr.h"
#include "nsISupports.h"
#include "nsBaseWidget.h"
#include "nsIPluginWidget.h"
#include "nsIScrollableView.h"
#include "nsWeakPtr.h"

#include "nsIWidget.h"
#include "nsIKBStateControl.h"
#include "nsIAppShell.h"

#include "nsIMouseListener.h"
#include "nsIEventListener.h"
#include "nsString.h"
#include "nsIDragService.h"
#include "nsIMenuBar.h"

#include "nsplugindefs.h"

#undef DARWIN
#import <Carbon/Carbon.h>
#import <Cocoa/Cocoa.h>

class gfxASurface;
class nsChildView;
union nsPluginPort;

@interface ChildView : NSView<
#ifdef ACCESSIBILITY
                              mozAccessible,
#endif
                              mozView, NSTextInput>
{
@private
  NSWindow* mWindow; // shortcut to the top window, [WEAK]
  
  // the nsChildView that created the view. It retains this NSView, so
  // the link back to it must be weak.
  nsChildView* mGeckoChild;
    
  // tag for our mouse enter/exit tracking rect
  NSTrackingRectTag mMouseEnterExitTag;

  // Whether we're a plugin view.
  BOOL mIsPluginView;

  // The following variables are only valid during key down event processing.
  // Their current usage needs to be fixed to avoid problems with nested event
  // loops that can confuse them. Once a variable is set during key down event
  // processing, if an event spawns a nested event loop the previously set value
  // will be wiped out.
  NSEvent* mCurKeyEvent;
  PRBool mKeyDownHandled;
  BOOL mIgnoreDoCommand;
  // While we process key down events we need to keep track of whether or not
  // we sent a key press event. This helps us make sure we do send one
  // eventually.
  BOOL mKeyPressSent;

  // needed for NSTextInput implementation
  NSRange mMarkedRange;

  BOOL mInHandScroll; // true for as long as we are hand scrolling
  // hand scroll locations
  NSPoint mHandScrollStartMouseLoc;
  nscoord mHandScrollStartScrollX, mHandScrollStartScrollY;
  
  // when menuForEvent: is called, we store its event here (strong)
  NSEvent* mLastMenuForEventEvent;
  
  // rects that were invalidated during a draw, so have pending drawing
  NSMutableArray* mPendingDirtyRects;
  BOOL mPendingFullDisplay;

  // All views are always opaque (non-transparent). The only exception is when we're
  // the content view in a transparent XUL window.
  BOOL mIsTransparent;

  // Holds our drag service across multiple drag calls. The reference to the
  // service is obtained when the mouse enters the view and is released when
  // the mouse exits or there is a drop. This prevents us from having to
  // re-establish the connection to the service manager many times per second
  // when handling |draggingUpdated:| messages.
  nsIDragService* mDragService;
  
  PRUint32 mLastModifierState;
}

// these are sent to the first responder when the window key status changes
- (void)viewsWindowDidBecomeKey;
- (void)viewsWindowDidResignKey;

// Stop NSView hierarchy being changed during [ChildView drawRect:]
- (void)delayedTearDown;

- (void)setTransparent:(BOOL)transparent;

- (void)sendFocusEvent:(PRUint32)eventType;
@end



//-------------------------------------------------------------------------
//
// nsTSMManager
//
//-------------------------------------------------------------------------

class nsTSMManager {
public:
  static PRBool IsComposing() { return sComposingView ? PR_TRUE : PR_FALSE; }
  static PRBool IsIMEEnabled() { return sIsIMEEnabled; }
  static PRBool IgnoreCommit() { return sIgnoreCommit; }

  // Note that we cannot get the actual state in TSM. But we can trust this
  // value. Because nsIMEStateManager reset this at every focus changing.
  static PRBool IsRomanKeyboardsOnly() { return sIsRomanKeyboardsOnly; }

  static PRBool GetIMEOpenState();

  static void StartComposing(NSView<mozView>* aComposingView);
  static void UpdateComposing(NSString* aComposingString);
  static void EndComposing();
  static void EnableIME(PRBool aEnable);
  static void SetIMEOpenState(PRBool aOpen);
  static void SetRomanKeyboardsOnly(PRBool aRomanOnly);

  static void CommitIME();
  static void CancelIME();
private:
  static PRBool sIsIMEEnabled;
  static PRBool sIsRomanKeyboardsOnly;
  static PRBool sIgnoreCommit;
  static NSView<mozView>* sComposingView;
  static TSMDocumentID sDocumentID;
  static NSString* sComposingString;

  static void KillComposing();
};

//-------------------------------------------------------------------------
//
// nsChildView
//
//-------------------------------------------------------------------------

class nsChildView : public nsBaseWidget,
                    public nsIPluginWidget,
                    public nsIKBStateControl
{
private:
  typedef nsBaseWidget Inherited;

public:
                          nsChildView();
  virtual                 ~nsChildView();
  
  NS_DECL_ISUPPORTS_INHERITED

  // nsIKBStateControl interface
  NS_IMETHOD              ResetInputState();
  NS_IMETHOD              SetIMEOpenState(PRBool aState);
  NS_IMETHOD              GetIMEOpenState(PRBool* aState);
  NS_IMETHOD              SetIMEEnabled(PRUint32 aState);
  NS_IMETHOD              GetIMEEnabled(PRUint32* aState);
  NS_IMETHOD              CancelIMEComposition();
  NS_IMETHOD              GetToggledKeyState(PRUint32 aKeyCode,
                                             PRBool* aLEDState);

  // nsIWidget interface
  NS_IMETHOD              Create(nsIWidget *aParent,
                                 const nsRect &aRect,
                                 EVENT_CALLBACK aHandleEventFunction,
                                 nsIDeviceContext *aContext,
                                 nsIAppShell *aAppShell = nsnull,
                                 nsIToolkit *aToolkit = nsnull,
                                 nsWidgetInitData *aInitData = nsnull);
  NS_IMETHOD              Create(nsNativeWidget aNativeParent,
                                 const nsRect &aRect,
                                 EVENT_CALLBACK aHandleEventFunction,
                                 nsIDeviceContext *aContext,
                                 nsIAppShell *aAppShell = nsnull,
                                 nsIToolkit *aToolkit = nsnull,
                                 nsWidgetInitData *aInitData = nsnull);

   // Utility method for implementing both Create(nsIWidget ...) and
   // Create(nsNativeWidget...)

  virtual nsresult        StandardCreate(nsIWidget *aParent,
                              const nsRect &aRect,
                              EVENT_CALLBACK aHandleEventFunction,
                              nsIDeviceContext *aContext,
                              nsIAppShell *aAppShell,
                              nsIToolkit *aToolkit,
                              nsWidgetInitData *aInitData,
                              nsNativeWidget aNativeParent = nsnull);

  NS_IMETHOD              Destroy();

  NS_IMETHOD              Show(PRBool aState);
  NS_IMETHOD              IsVisible(PRBool& outState);

  virtual nsIWidget*      GetParent(void);
  nsIWidget*              GetTopLevelWidget();

  NS_IMETHOD              ModalEventFilter(PRBool aRealEvent, void *aEvent,
                                           PRBool *aForWindow);

  NS_IMETHOD              ConstrainPosition(PRBool aAllowSlop,
                                            PRInt32 *aX, PRInt32 *aY);
  NS_IMETHOD              Move(PRInt32 aX, PRInt32 aY);
  NS_IMETHOD              Resize(PRInt32 aWidth,PRInt32 aHeight, PRBool aRepaint);
  NS_IMETHOD              Resize(PRInt32 aX, PRInt32 aY,PRInt32 aWidth,PRInt32 aHeight, PRBool aRepaint);

  NS_IMETHOD              Enable(PRBool aState);
  NS_IMETHOD              IsEnabled(PRBool *aState);
  NS_IMETHOD              SetFocus(PRBool aRaise);
  NS_IMETHOD              SetBounds(const nsRect &aRect);
  NS_IMETHOD              GetBounds(nsRect &aRect);

  NS_IMETHOD              Invalidate(PRBool aIsSynchronous);
  NS_IMETHOD              Invalidate(const nsRect &aRect,PRBool aIsSynchronous);
  NS_IMETHOD              InvalidateRegion(const nsIRegion *aRegion, PRBool aIsSynchronous);
  NS_IMETHOD              Validate();

  virtual void*           GetNativeData(PRUint32 aDataType);
  NS_IMETHOD              SetColorMap(nsColorMap *aColorMap);
  NS_IMETHOD              Scroll(PRInt32 aDx, PRInt32 aDy, nsRect *aClipRect);
  NS_IMETHOD              WidgetToScreen(const nsRect& aOldRect, nsRect& aNewRect);
  NS_IMETHOD              ScreenToWidget(const nsRect& aOldRect, nsRect& aNewRect);
  NS_IMETHOD              BeginResizingChildren(void);
  NS_IMETHOD              EndResizingChildren(void);

  static  PRBool          ConvertStatus(nsEventStatus aStatus)
                          { return aStatus == nsEventStatus_eConsumeNoDefault; }
  NS_IMETHOD              DispatchEvent(nsGUIEvent* event, nsEventStatus & aStatus);
  virtual PRBool          DispatchMouseEvent(nsMouseEvent &aEvent);

  NS_IMETHOD              Update();

  virtual void      ConvertToDeviceCoordinates(nscoord &aX, nscoord &aY);
  void              LocalToWindowCoordinate(nsPoint& aPoint)            { ConvertToDeviceCoordinates(aPoint.x, aPoint.y); }
  void              LocalToWindowCoordinate(nscoord& aX, nscoord& aY)   { ConvertToDeviceCoordinates(aX, aY); }
  void              LocalToWindowCoordinate(nsRect& aRect)              { ConvertToDeviceCoordinates(aRect.x, aRect.y); }

  NS_IMETHOD        SetMenuBar(nsIMenuBar * aMenuBar);
  NS_IMETHOD        ShowMenuBar(PRBool aShow);

  NS_IMETHOD        GetPreferredSize(PRInt32& aWidth, PRInt32& aHeight);
  NS_IMETHOD        SetPreferredSize(PRInt32 aWidth, PRInt32 aHeight);
  
  NS_IMETHOD        SetCursor(nsCursor aCursor);
  NS_IMETHOD        SetCursor(imgIContainer* aCursor, PRUint32 aHotspotX, PRUint32 aHotspotY);
  
  NS_IMETHOD        CaptureRollupEvents(nsIRollupListener * aListener, PRBool aDoCapture, PRBool aConsumeRollupEvent);
  NS_IMETHOD        SetTitle(const nsAString& title);

  NS_IMETHOD        GetAttention(PRInt32 aCycleCount);

  // nsIPluginWidget
  NS_IMETHOD        GetPluginClipRect(nsRect& outClipRect, nsPoint& outOrigin, PRBool& outWidgetVisible);
  NS_IMETHOD        StartDrawPlugin();
  NS_IMETHOD        EndDrawPlugin();
  
  NS_IMETHOD        GetHasTransparentBackground(PRBool& aTransparent);
  NS_IMETHOD        SetHasTransparentBackground(PRBool aTransparent);
  
  // Mac specific methods
  virtual PRBool    PointInWidget(Point aThePoint);
  
  virtual PRBool    DispatchWindowEvent(nsGUIEvent& event);
  
  void              LiveResizeStarted();
  void              LiveResizeEnded();
  
#ifdef ACCESSIBILITY
  void              GetDocumentAccessible(nsIAccessible** aAccessible);
#endif

  virtual gfxASurface* GetThebesSurface();

  NS_IMETHOD BeginSecureKeyboardInput();
  NS_IMETHOD EndSecureKeyboardInput();

protected:

  PRBool            ReportDestroyEvent();
  PRBool            ReportMoveEvent();
  PRBool            ReportSizeEvent();

  NS_IMETHOD        CalcOffset(PRInt32 &aX,PRInt32 &aY);

  virtual PRBool    OnPaint(nsPaintEvent & aEvent);

  // override to create different kinds of child views. Autoreleases, so
  // caller must retain.
  virtual NSView*   CreateCocoaView(NSRect inFrame);
  void              TearDownView();

protected:

  NSView<mozView>*      mView;      // my parallel cocoa view (ChildView or NativeScrollbarView), [STRONG]

  NSView<mozView>*      mParentView;
  nsIWidget*            mParentWidget;

#ifdef ACCESSIBILITY
  // weak ref to this childview's associated mozAccessible for speed reasons 
  // (we get queried for it *a lot* but don't want to own it)
  nsWeakPtr             mAccessible;
#endif

  nsRefPtr<gfxASurface> mTempThebesSurface;

  PRPackedBool          mVisible;
  PRPackedBool          mDrawing;
  PRPackedBool          mLiveResizeInProgress;
  PRPackedBool          mIsPluginView; // true if this is a plugin view
  PRPackedBool          mPluginDrawing;
  PRPackedBool          mPluginIsCG; // true if this is a CoreGraphics plugin

  PRPackedBool          mInSetFocus;

  nsPluginPort          mPluginPort;
};


#endif // nsChildView_h_
