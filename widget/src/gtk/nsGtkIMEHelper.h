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
 *   Frank Tang <ftang@netsape.com>
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
#ifndef nsGtkIMEHelper_h__
#define nsGtkIMEHelper_h__
#include "nsIUnicodeDecoder.h"
#include "nsString.h"
#include <gtk/gtk.h>

/*
 * We are gradually moving IME related function into this class
 */
class nsGtkIMEHelper {
public:
  ~nsGtkIMEHelper();
  nsresult ConvertToUnicode(const char* aSrc, PRInt32* aSrcLen,
                            PRUnichar*  aDes, PRInt32* aDesLen);
  static nsGtkIMEHelper *GetSingleton();
  void ResetDecoder();
  static void Shutdown();
  PRInt32 MultiByteToUnicode(const char*, const PRInt32,
                             PRUnichar**, PRInt32*);
#if defined(USE_XIM) && defined(_AIX)
  PRUnichar* GetUnichars() const { return mUnichars; }
  PRInt32 GetUnicharsSize() const { return mUnicharsSize; }

  // It is the caller's responsibility to free any data in
  // mUnichars prior to calling SetUnichars()
  void SetUnichars(PRUnichar *aUnichars) { mUnichars = aUnichars; }
  void SetUnicharsSize(PRInt32 aUnicharsSize)
    { mUnicharsSize = aUnicharsSize; }
#endif

private:
  nsGtkIMEHelper();
  nsIUnicodeDecoder* mDecoder;
  void SetupUnicodeDecoder();
  static nsGtkIMEHelper *gSingleton;
#if defined(USE_XIM) && defined(_AIX)
  PRUnichar* mUnichars;
  PRInt32 mUnicharsSize;
#endif
};

#ifdef USE_XIM
class nsIMEPreedit {
 private:
  PRInt32       mCaretPosition;
  nsString*     mIMECompUnicode;
  nsCString*    mIMECompAttr;
  PRUnichar*    mCompositionUniString;
  PRInt32       mCompositionUniStringSize;
 public:
  nsIMEPreedit();
  ~nsIMEPreedit();
  void Reset();
  const PRUnichar* GetPreeditString() const;
  const char* GetPreeditFeedback() const;
  int GetPreeditLength() const;
  void SetPreeditString(const XIMText* aText,
                        const PRInt32 aChangeFirst,
                        const PRInt32 aChangeLength);
  static void IMSetTextRange(const PRInt32 aLen,
                             const char *aFeedback,
                             PRUint32 *,
                             nsTextRangeArray*);
};

class nsWindow;

class nsIMEStatus {
 private:
  Window mIMStatusWindow;
  Window mIMStatusLabel;  
  XFontSet mFontset;
  int mWidth;
  int mHeight;
  GC mGC;
  void resize(const char *);
  void remove_decoration();
  void getAtoms();
  static Bool client_filter(Display *d, Window w, XEvent *ev,
                               XPointer client_data);
  static Bool repaint_filter(Display *d, Window w, XEvent *ev,
                             XPointer client_data);
  static Bool clientmessage_filter(Display *d, Window w, XEvent *ev,
                                   XPointer client_data);
  void CreateNative();
  void DestroyNative();
 public:
  nsIMEStatus();
  nsIMEStatus(GdkFont*);
  void SetFont(GdkFont*);
  ~nsIMEStatus();
  void UnregisterClientFilter(Window);
  void RegisterClientFilter(Window);
  void setText(const char*);
  void setParentWindow(nsWindow*);
  void resetParentWindow(nsWindow*);
  void show();
  void hide();

  nsWindow *mAttachedWindow;
  GdkWindow *mParent;
};

/* for XIM callback */
typedef int (*XIMProc1)(XIC, XPointer, XPointer);
typedef struct {
  XPointer client_data;
  XIMProc1 callback;
} XIMCallback1;

class nsIMEGtkIC {
 private:
  static int preedit_start_cbproc(XIC, XPointer, XPointer);
  static int preedit_draw_cbproc(XIC, XPointer, XPointer);
  static int preedit_done_cbproc(XIC, XPointer, XPointer);
  static int preedit_caret_cbproc(XIC, XPointer, XPointer) { return 0; };
  static int status_draw_cbproc(XIC, XPointer, XPointer);
  static int status_start_cbproc(XIC, XPointer, XPointer) { return 0; };
  static int status_done_cbproc(XIC, XPointer, XPointer) { return 0; };
  static nsIMEStatus *gStatus;
  nsWindow *mClientWindow;
  nsWindow *mFocusWindow;
  static nsWindow *gGlobalFocusWindow;
  nsIMEGtkIC(nsWindow*, GdkFont*, GdkFont*);
  nsIMEGtkIC(nsWindow*, GdkFont*);
  GdkICPrivate *mIC;
  GdkICPrivate *mIC_backup;
  nsIMEPreedit *mPreedit;
  GdkFont      *mStatusFontset;

 public:
  nsIMEPreedit *GetPreedit() {return mPreedit;}
  ~nsIMEGtkIC();
  static nsIMEGtkIC *GetXIC(nsWindow*, GdkFont*, GdkFont*);
  static nsIMEGtkIC *GetXIC(nsWindow*, GdkFont*);
  void SetFocusWindow(nsWindow * aFocusWindow);
  nsWindow* GetFocusWindow();
  nsWindow* GetGlobalFocusWindow();
  static void UnsetFocusWindow();
  static GdkIMStyle GetInputStyle();

  GdkIMStyle mInputStyle;
  char *mStatusText;
  void SetStatusText(const char*);
  void SetPreeditFont(GdkFont*);
  void SetStatusFont(GdkFont*);
  void SetPreeditSpotLocation(unsigned long, unsigned long);
  void SetPreeditArea(int, int, int, int);
  void ResetStatusWindow(nsWindow * aWindow);
  PRBool IsPreeditComposing();
  PRInt32 ResetIC(PRUnichar **aUnichar, PRInt32 *aUnisize);
};
#endif // USE_XIM 
#endif // nsGtkIMEHelper_h__
