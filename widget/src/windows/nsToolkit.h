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
 *   Masayuki Nakano <masayuki@d-toybox.com>
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

#ifndef TOOLKIT_H      
#define TOOLKIT_H

#include "nsdefs.h"
#include "nsIToolkit.h"

#include "nsWindowAPI.h"
#include "nsITimer.h"
#include "nsCOMPtr.h"

#include <imm.h>

struct MethodInfo;
class nsIEventQueue;
class MouseTrailer;

// we used to use MAX_PATH
// which works great for one file
// but for multiple files, the format is
// dirpath\0\file1\0file2\0...filen\0\0
// and that can quickly be more than MAX_PATH (260)
// see bug #172001 for more details
#define FILE_BUFFER_SIZE 4096 


/**
 * Wrapper around the thread running the message pump.
 * The toolkit abstraction is necessary because the message pump must
 * execute within the same thread that created the widget under Win32.
 */ 

class nsToolkit : public nsIToolkit
{

  public:

            NS_DECL_ISUPPORTS

                            nsToolkit();
            NS_IMETHOD      Init(PRThread *aThread);
            void            CallMethod(MethodInfo *info);
            // Return whether the current thread is the application's Gui thread.  
            PRBool          IsGuiThread(void)      { return (PRBool)(mGuiThread == PR_GetCurrentThread());}
            PRThread*       GetGuiThread(void)       { return mGuiThread;   }
            HWND            GetDispatchWindow(void)  { return mDispatchWnd; }
            void            CreateInternalWindow(PRThread *aThread);
            // Return whether the user is currently moving any application window
            PRBool          UserIsMovingWindow(void);

private:
                            ~nsToolkit();
            void            CreateUIThread(void);

public:
    // Window procedure for the internal window
    static LRESULT CALLBACK WindowProc(HWND hWnd, 
                                        UINT Msg, 
                                        WPARAM WParam, 
                                        LPARAM lParam);

protected:
    // Handle of the window used to receive dispatch messages.
    HWND        mDispatchWnd;
    // Thread Id of the "main" Gui thread.
    PRThread    *mGuiThread;

public:
    static HINSTANCE mDllInstance;
    // OS flag
    static PRBool    mIsWinXP;

    static PRBool InitVersionInfo();
    static void Startup(HINSTANCE hModule);
    static void Shutdown();

    static MouseTrailer *gMouseTrailer;
};

#define WM_CALLMETHOD   (WM_USER+1)

inline void nsToolkit::CallMethod(MethodInfo *info)
{
    NS_PRECONDITION(::IsWindow(mDispatchWnd), "Invalid window handle");
    ::SendMessage(mDispatchWnd, WM_CALLMETHOD, (WPARAM)0, (LPARAM)info);
}

class  nsWindow;

/**
 * Makes sure exit/enter mouse messages are always dispatched.
 * In the case where the mouse has exited the outer most window the
 * only way to tell if it has exited is to set a timer and look at the
 * mouse pointer to see if it is within the outer most window.
 */ 

class MouseTrailer 
{
public:
    HWND                  GetMouseTrailerWindow() { return mMouseTrailerWindow; }
    HWND                  GetCaptureWindow() { return mCaptureWindow; }

    void                  SetMouseTrailerWindow(HWND aWnd);
    void                  SetCaptureWindow(HWND aWnd);
    void                  Disable() { mEnabled = PR_FALSE; DestroyTimer(); }
    void                  Enable() { mEnabled = PR_TRUE; CreateTimer(); }
    void                  DestroyTimer();

                          MouseTrailer();
                          ~MouseTrailer();
private:

    nsresult              CreateTimer();

    static void           TimerProc(nsITimer* aTimer, void* aClosure);

    // Information for mouse enter/exit events
    HWND                  mMouseTrailerWindow;
    HWND                  mCaptureWindow;
    PRBool                mIsInCaptureMode;
    PRBool                mEnabled;
    nsCOMPtr<nsITimer>    mTimer;
};


//-------------------------------------------------------------------------
//
// related WM_IME_REQUEST definition.
// VC++5.0 header doesn't have reconversion structure and message.
//
//-------------------------------------------------------------------------

#ifndef WM_IME_REQUEST
#define WM_IME_REQUEST                  0x0288
#endif    // #ifndef WM_IME_REQUEST

#ifndef IMR_RECONVERTSTRING
#define IMR_RECONVERTSTRING             0x0004
typedef struct tagRECONVERTSTRING {
  DWORD dwSize;
  DWORD dwVersion;
  DWORD dwStrLen;
  DWORD dwStrOffset;
  DWORD dwCompStrLen;
  DWORD dwCompStrOffset;
  DWORD dwTargetStrLen;
  DWORD dwTargetStrOffset;
} RECONVERTSTRING, FAR * LPRECONVERTSTRING;
#endif    // #ifndef IMR_RECONVERTSTRING

#ifndef IMR_QUERYCHARPOSITION
#define IMR_QUERYCHARPOSITION           0x0006
typedef struct tagIMECHARPOSITION {
  DWORD dwSize;
  DWORD dwCharPos;
  POINT pt;
  UINT  cLineHeight;
  RECT  rcDocument;
} IMECHARPOSITION, *PIMECHARPOSITION;
#endif    // #ifndef IMR_QUERYCHARPOSITION

//-------------------------------------------------------------------------
//
// from http://msdn.microsoft.com/library/en-us/dnime/html/msime.asp
//
//-------------------------------------------------------------------------

#define RWM_MOUSE           TEXT("MSIMEMouseOperation")

#define IMEMOUSE_NONE       0x00    // no mouse button was pushed
#define IMEMOUSE_LDOWN      0x01
#define IMEMOUSE_RDOWN      0x02
#define IMEMOUSE_MDOWN      0x04
#define IMEMOUSE_WUP        0x10    // wheel up
#define IMEMOUSE_WDOWN      0x20    // wheel down

#endif  // TOOLKIT_H
