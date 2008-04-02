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
 * The Original Code is the Mozilla OS/2 libraries.
 *
 * The Initial Developer of the Original Code is
 * John Fairhurst, <john_fairhurst@iname.com>.
 * Portions created by the Initial Developer are Copyright (C) 1999
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Rich Walsh <dragtext@e-vertise.com>
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
 * ***** END LICENSE BLOCK *****
 * This Original Code has been modified by IBM Corporation.
 * Modifications made by IBM described herein are
 * Copyright (c) International Business Machines
 * Corporation, 2000
 *
 * Modifications to Mozilla code or documentation
 * identified per MPL Section 3.3
 *
 * Date             Modified by     Description of modification
 * 03/23/2000       IBM Corp.      Added InvalidateRegion method.
 * 04/12/2000       IBM Corp.      Changed params on DispatchMouseEvent to match Windows..
 * 04/14/2000       IBM Corp.      Declared EventIsInsideWindow for CaptureRollupEvents
 * 06/15/2000       IBM Corp.      Added NS2PM for rectangles
 * 06/21/2000       IBM Corp.      Added CaptureMouse
 *
 */

#ifndef _nswindow_h
#define _nswindow_h

#include "nsWidgetDefs.h"
#include "nsBaseWidget.h"
#include "nsToolkit.h"
#include "nsSwitchToUIThread.h"
#include "gfxOS2Surface.h"
#include "gfxContext.h"

class nsIMenuBar;
class imgIContainer;

//#define DEBUG_FOCUS

#ifdef DEBUG_FOCUS
  #define DEBUGFOCUS(what) printf("[%x] "#what" (%d)\n", (int)this, mWindowIdentifier)
#else
  #define DEBUGFOCUS(what)
#endif

// Base widget class.
// This is abstract.  Controls (labels, radio buttons, listboxen) derive
// from here.  A thing called a child window derives from here, and the
// frame window class derives from the child.
// nsFrameWindow is separate because work needs to be done there to decide
// whether methods apply to frame or client.

/* Possible states of the window, used to emulate windows better... */
   // default state; Create() not called 
   #define   nsWindowState_ePrecreate      0x00000001
   // processing Create() method          
   #define   nsWindowState_eInCreate       0x00000002
   // active, existing window             
   #define      nsWindowState_eLive        0x00000004
   //processing Close() method            
   #define      nsWindowState_eClosing     0x00000008
   // object destructor running 
   #define      nsWindowState_eDoingDelete 0x00000010
   // window destroyed 
   #define      nsWindowState_eDead        0x00000100         

MRESULT EXPENTRY fnwpNSWindow( HWND, ULONG, MPARAM, MPARAM);
MRESULT EXPENTRY fnwpFrame( HWND, ULONG, MPARAM, MPARAM);

class nsWindow : public nsBaseWidget,
                 public nsSwitchToUIThread
{
 public:
   // Scaffolding
   nsWindow();
   virtual ~nsWindow();

  static void ReleaseGlobals();

   // nsIWidget

   // Creation from native (eh?) or widget parent, destroy
   NS_IMETHOD Create( nsIWidget *aParent,
                      const nsRect &aRect,
                      EVENT_CALLBACK aHandleEventFunction,
                      nsIDeviceContext *aContext,
                      nsIAppShell *aAppShell = nsnull,
                      nsIToolkit *aToolkit = nsnull,
                      nsWidgetInitData *aInitData = nsnull);
   NS_IMETHOD Create( nsNativeWidget aParent,
                      const nsRect &aRect,
                      EVENT_CALLBACK aHandleEventFunction,
                      nsIDeviceContext *aContext,
                      nsIAppShell *aAppShell = nsnull,
                      nsIToolkit *aToolkit = nsnull,
                      nsWidgetInitData *aInitData = nsnull);
   gfxASurface* GetThebesSurface();
   NS_IMETHOD Destroy(); // call before releasing

   // Hierarchy: only interested in widget children (it seems)
   virtual nsIWidget *GetParent();

    NS_IMETHOD              SetSizeMode(PRInt32 aMode);

   // Physical properties
   NS_IMETHOD Show( PRBool bState);
   NS_IMETHOD ConstrainPosition(PRBool aAllowSlop, PRInt32 *aX, PRInt32 *aY);
   NS_IMETHOD Move( PRInt32 aX, PRInt32 aY);
   NS_IMETHOD Resize( PRInt32 aWidth,
                      PRInt32 aHeight,
                      PRBool   aRepaint);
   NS_IMETHOD Resize( PRInt32 aX,
                      PRInt32 aY,
                      PRInt32 aWidth,
                      PRInt32 aHeight,
                      PRBool   aRepaint);
   NS_IMETHOD GetClientBounds( nsRect &aRect);
   NS_IMETHOD Enable( PRBool aState);
   NS_IMETHOD IsEnabled(PRBool *aState);
   NS_IMETHOD SetFocus(PRBool aRaise);
   NS_IMETHOD GetBounds(nsRect &aRect);
   NS_IMETHOD IsVisible( PRBool &aState);
   NS_IMETHOD PlaceBehind(nsTopLevelWidgetZPlacement aPlacement,
                          nsIWidget *aWidget, PRBool aActivate);

   NS_IMETHOD CaptureMouse(PRBool aCapture);

   NS_IMETHOD ModalEventFilter( PRBool aRealEvent, void *aEvent,
                                PRBool *aForWindow );

   NS_IMETHOD GetPreferredSize( PRInt32 &aWidth, PRInt32 &aHeight);
   NS_IMETHOD SetPreferredSize( PRInt32 aWidth, PRInt32 aHeight);

   NS_IMETHOD BeginResizingChildren();
   NS_IMETHOD EndResizingChildren();
   NS_IMETHOD WidgetToScreen( const nsRect &aOldRect, nsRect &aNewRect);
   NS_IMETHOD ScreenToWidget( const nsRect &aOldRect, nsRect &aNewRect);
   NS_IMETHOD DispatchEvent( struct nsGUIEvent *event, nsEventStatus &aStatus);
   NS_IMETHOD CaptureRollupEvents(nsIRollupListener * aListener, PRBool aDoCapture, PRBool aConsumeRollupEvent);

   NS_IMETHOD              GetLastInputEventTime(PRUint32& aTime);

   // Widget appearance
   virtual nsIFontMetrics *GetFont();
   NS_IMETHOD              SetFont( const nsFont &aFont);
   NS_IMETHOD              SetColorMap( nsColorMap *aColorMap);
   NS_IMETHOD              SetCursor( nsCursor aCursor);
   NS_IMETHOD              SetCursor(imgIContainer* aCursor,
                                     PRUint32 aHotspotX, PRUint32 aHotspotY);
   NS_IMETHOD              HideWindowChrome(PRBool aShouldHide);
   NS_IMETHOD              SetTitle( const nsAString& aTitle); 
   NS_IMETHOD              SetIcon(const nsAString& aIconSpec); 
   NS_IMETHOD              SetMenuBar(nsIMenuBar * aMenuBar) { return NS_ERROR_FAILURE; } 
   NS_IMETHOD              ShowMenuBar(PRBool aShow)         { return NS_ERROR_FAILURE; } 
   NS_IMETHOD              Invalidate( PRBool aIsSynchronous);
   NS_IMETHOD              Invalidate( const nsRect & aRect, PRBool aIsSynchronous);
   NS_IMETHOD              InvalidateRegion(const nsIRegion *aRegion, PRBool aIsSynchronous);
   NS_IMETHOD              Update();
   NS_IMETHOD              Scroll( PRInt32 aDx, PRInt32 aDy, nsRect *aClipRect);
   NS_IMETHOD              ScrollWidgets(PRInt32 aDx, PRInt32 aDy);
   NS_IMETHOD              ScrollRect(nsRect &aRect, PRInt32 aDx, PRInt32 aDy);

   // Get a HWND or a HPS.
   virtual void  *GetNativeData( PRUint32 aDataType);
   virtual void   FreeNativeData( void *aDatum, PRUint32 aDataType);
   virtual HWND   GetMainWindow() const           { return mWnd; }

   // nsSwitchToPMThread interface
    virtual BOOL            CallMethod(MethodInfo *info);

   // PM methods which need to be public (menus, etc)
   ULONG  GetNextID()    { return mNextID++; }
   void   NS2PM_PARENT( POINTL &ptl);
   void   NS2PM( POINTL &ptl);
   void   NS2PM( RECTL &rcl);

protected:
    static  BOOL            DealWithPopups ( ULONG inMsg, MRESULT* outResult ) ;

    static  PRBool          EventIsInsideWindow(nsWindow* aWindow); 

    static  nsWindow *      GetNSWindowPtr(HWND aWnd);
    static  BOOL            SetNSWindowPtr(HWND aWnd, nsWindow * ptr);

   static  nsWindow*   gCurrentWindow;
   // nsWindow methods subclasses must provide for creation to work
   virtual PCSZ  WindowClass();
   virtual ULONG WindowStyle();

   // hooks subclasses may wish to override!
   virtual void     PostCreateWidget()            {}
   virtual PRInt32  GetClientHeight()             { return mBounds.height; }
   virtual ULONG    GetSWPFlags( ULONG flags)     { return flags; }
   virtual void     SetupForPrint( HWND /*hwnd*/) {}

   // Useful functions for subclasses to use, threaded as necessary.
   virtual nsresult GetWindowText( nsString &str, PRUint32 *rc);
   virtual void     AddToStyle( ULONG style);
   virtual void     RemoveFromStyle( ULONG style);
   // return true if deferred
   virtual BOOL     SetWindowPos( HWND hwndInsertBehind, long x, long y,
                                  long cx, long cy, unsigned long flags);

   // Message handlers - may wish to override.  Default implementation for
   // control, paint & scroll is to do nothing.

   // Return whether message has been processed.
   virtual PRBool ProcessMessage( ULONG m, MPARAM p1, MPARAM p2, MRESULT &r);
   virtual PRBool OnPaint();
   virtual void   OnDestroy();
   virtual PRBool OnReposition( PSWP pNewSwp);
   virtual PRBool OnResize( PRInt32 aX, PRInt32 aY);
   virtual PRBool OnMove( PRInt32 aX, PRInt32 aY);
   virtual PRBool OnKey( MPARAM mp1, MPARAM mp2);
   virtual PRBool DispatchFocus( PRUint32 aEventType, PRBool isMozWindowTakingFocus);
   virtual PRBool OnScroll( ULONG msgid, MPARAM mp1, MPARAM mp2);
   virtual PRBool OnVScroll( MPARAM mp1, MPARAM mp2);
   virtual PRBool OnHScroll( MPARAM mp1, MPARAM mp2);
   virtual PRBool OnControl( MPARAM mp1, MPARAM mp2);
   // called after param has been set...
   virtual PRBool OnPresParamChanged( MPARAM mp1, MPARAM mp2);
   virtual PRBool OnDragDropMsg(ULONG msg, MPARAM mp1, MPARAM mp2, MRESULT &mr);

   static BOOL sIsRegistered;

   // PM data members
   HWND      mWnd;            // window handle
   HWND      mFrameWnd;       // frame window handle
   PFNWP     mPrevWndProc;    // previous window procedure
   nsWindow *mParent;         // parent widget
   ULONG     mNextID;         // next child window id
   PSWP      mSWPs;           // SWPs for deferred window positioning
   ULONG     mlHave, mlUsed;  // description of mSWPs array
   HPOINTER  mFrameIcon;      // current frame icon
   VDKEY     mDeadKey;        // dead key from previous keyevent
   BOOL      mHaveDeadKey;    // is mDeadKey valid [0 may be a valid dead key, for all I know]
   QMSG      mQmsg;
   PRBool    mIsTopWidgetWindow;
   BOOL      mIsScrollBar;
   BOOL      mInSetFocus;
   BOOL      mChromeHidden;
   nsContentType mContentType;
   HPS       mDragHps;        // retrieved by DrgGetPS() during a drag
   PRUint32  mDragStatus;     // set while this object is being dragged over
   HPOINTER  mCssCursorHPtr;  // created by SetCursor(imgIContainer*)
   nsCOMPtr<imgIContainer> mCssCursorImg;  // saved by SetCursor(imgIContainer*)

   HWND      GetParentHWND() const;
   HWND      GetHWND() const   { return mWnd; }
   PFNWP     GetPrevWP() const { return mPrevWndProc; }

   // nglayout data members
   PRInt32        mPreferredHeight;
   PRInt32        mPreferredWidth;
   nsToolkit     *mOS2Toolkit;
   nsFont        *mFont;
   nsIMenuBar    *mMenuBar;
   PRInt32        mWindowState;
   nsRefPtr<gfxOS2Surface> mThebesSurface;

   // Implementation ------------------------------
   void DoCreate( HWND hwndP, nsWindow *wndP, const nsRect &rect,
                  EVENT_CALLBACK aHandleEventFunction,
                  nsIDeviceContext *aContext, nsIAppShell *aAppShell,
                  nsIToolkit *aToolkit, nsWidgetInitData *aInitData);

   virtual void RealDoCreate( HWND hwndP, nsWindow *aParent,
                              const nsRect &aRect,
                              EVENT_CALLBACK aHandleEventFunction,
                              nsIDeviceContext *aContext,
                              nsIAppShell *aAppShell,
                              nsWidgetInitData *aInitData,
                              HWND hwndOwner = 0);

   // hook so dialog can be created looking like a dialog
   virtual ULONG GetFCFlags();

   virtual void SubclassWindow(BOOL bState);

   PRBool  ConvertStatus( nsEventStatus aStatus)
                        { return aStatus == nsEventStatus_eConsumeNoDefault; }
   void    InitEvent( nsGUIEvent &event, nsPoint *pt = 0);
   virtual PRBool DispatchWindowEvent(nsGUIEvent* event);
   virtual PRBool DispatchWindowEvent(nsGUIEvent*event, nsEventStatus &aStatus);
   PRBool  DispatchStandardEvent( PRUint32 aMsg);
   PRBool  DispatchCommandEvent(PRUint32 aEventCommand);
   PRBool  DispatchDragDropEvent( PRUint32 aMsg);
   virtual PRBool DispatchMouseEvent(PRUint32 aEventType, MPARAM mp1, MPARAM mp2, 
                                     PRBool aIsContextMenuKey = PR_FALSE,
                                     PRInt16 aButton = nsMouseEvent::eLeftButton);
   virtual PRBool DispatchResizeEvent( PRInt32 aClientX, PRInt32 aClientY);
   void GetNonClientBounds(nsRect &aRect);
   void    DeferPosition( HWND, HWND, long, long, long, long, ULONG);
   void ConstrainZLevel(HWND *aAfter);

   PRBool   CheckDragStatus(PRUint32 aAction, HPS * oHps);
   PRBool   ReleaseIfDragHPS(HPS aHps);

   HBITMAP DataToBitmap(PRUint8* aImageData, PRUint32 aWidth,
                        PRUint32 aHeight, PRUint32 aDepth);
   HBITMAP CreateBitmapRGB(PRUint8* aImageData, PRUint32 aWidth, PRUint32 aHeight);
   // 'format' should be 'gfx_format' which is a PRInt32
   HBITMAP CreateTransparencyMask(PRInt32  format, PRUint8* aImageData,
                                  PRUint32 aWidth, PRUint32 aHeight);

   // Enumeration of the methods which are accessible on the PM thread
   enum {
      CREATE,
      DESTROY,
      SET_FOCUS,
      UPDATE_WINDOW,
      SET_TITLE,
      GET_TITLE
   };
   friend MRESULT EXPENTRY fnwpNSWindow( HWND, ULONG, MPARAM, MPARAM);
   friend MRESULT EXPENTRY fnwpFrame( HWND, ULONG, MPARAM, MPARAM);
#ifdef DEBUG_FOCUS
   int mWindowIdentifier;
#endif
};

#define PM2NS_PARENT NS2PM_PARENT
#define PM2NS NS2PM

#define PMSCAN_PADMULT      0x37
#define PMSCAN_PAD7         0x47
#define PMSCAN_PAD8         0x48
#define PMSCAN_PAD9         0x49
#define PMSCAN_PADMINUS     0x4A
#define PMSCAN_PAD4         0x4B
#define PMSCAN_PAD5         0x4C
#define PMSCAN_PAD6         0x4D
#define PMSCAN_PADPLUS      0x4E
#define PMSCAN_PAD1         0x4F
#define PMSCAN_PAD2         0x50
#define PMSCAN_PAD3         0x51
#define PMSCAN_PAD0         0x52
#define PMSCAN_PADPERIOD    0x53
#define PMSCAN_PADDIV       0x5c

#define isNumPadScanCode(scanCode) !( (scanCode < PMSCAN_PAD7) ||      \
                                      (scanCode > PMSCAN_PADPERIOD) || \
                                      (scanCode == PMSCAN_PADMULT) ||  \
                                      (scanCode == PMSCAN_PADDIV) ||   \
                                      (scanCode == PMSCAN_PADMINUS) || \
                                      (scanCode == PMSCAN_PADPLUS) )
#define isNumlockOn (BOOL)WinGetKeyState(HWND_DESKTOP, VK_NUMLOCK) & 0x0001

extern PRUint32 WMChar2KeyCode( MPARAM mp1, MPARAM mp2);

extern nsWindow *NS_HWNDToWindow( HWND hwnd);

#endif
