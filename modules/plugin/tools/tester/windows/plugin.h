/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef __PLUGIN_H__
#define __PLUGIN_H__

#include "plugbase.h"

typedef enum
{
  sg_manual = 1,
  sg_auto
} ShowGUI;

typedef enum
{
  gp_mode = 1,
  gp_logfile,
  gp_scriptfile,
  gp_tofile,
  gp_toframe,
  gp_flush,
  gp_rememberlast,
  gp_standalone,
  gp_autostartscript
}GUIPrefs;

class CPlugin : public CPluginBase
{
// public interface
public:
  CPlugin(NPP pNPInstance, WORD wMode);
  ~CPlugin();

  BOOL initEmbed(DWORD dwInitData);
  BOOL initFull(DWORD dwInitData);
  void shutEmbed();
  void shutFull();
  BOOL isInitialized();
  void getModulePath(LPSTR szPath, int iSize);
  int messageBox(LPSTR szMessage, LPSTR szTitle, UINT uStyle);
  void getLogFileName(LPSTR szLogFileName, int iSize);
  BOOL initStandAlone(); // create separate native window
  void shutStandAlone(); // destroy separate native window
  BOOL isStandAlone(); // is our GUI is in a separate native window
  void outputToNativeWindow(LPSTR szString); // used to output log in StandAlone mode

  void autoStartScriptIfNeeded();

  DWORD makeNPNCall(NPAPI_Action = action_invalid, 
                    DWORD dw1 = 0L, DWORD dw2 = 0L, 
                    DWORD dw3 = 0L, DWORD dw4 = 0L, 
                    DWORD dw5 = 0L, DWORD dw6 = 0L, 
                    DWORD dw7 = 0L);

  //
  // Windows specific methods
  //
  const HINSTANCE getInstance();
  const HWND getWindow();
  const HWND getParent();

  // tester interface (GUI stuff)
  BOOL createTester();
  void destroyTester();
  void showGUI(ShowGUI sg);

  // utilities
  void restorePreferences();
  void savePreferences();
  void updatePrefs(GUIPrefs prefs, int iValue, char * szValue = NULL);

  // Window message handlers
  void onInit(HWND hWnd, HWND hWndManual, HWND hWndAuto);
  void onDestroy();
  void onLogToFile(BOOL bLofToFile);

private:
  HINSTANCE m_hInst;
  HWND m_hWnd;
  HWND m_hWndParent;
  HWND m_hWndStandAloneLogWindow;
  HWND m_hWndManual;
  HWND m_hWndAuto;
  BOOL m_bPluginReady;

// some GUI data
public:
  HWND m_hWndLastEditFocus;
  int m_iWidth;
  int m_iHeight;

// GUI preferences
  ShowGUI m_Pref_ShowGUI;
  char m_Pref_szLogFile[256];
  char m_Pref_szScriptFile[256];
  BOOL m_Pref_bToFile;
  BOOL m_Pref_bToFrame;
  BOOL m_Pref_bFlushNow;
  BOOL m_Pref_bRememberLastCall;
  BOOL m_Pref_bStandAlone;
  BOOL m_Pref_bAutoStartScript;
};

#endif // __PLUGIN_H__
