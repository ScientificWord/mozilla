/* -*- Mode: C; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
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
 * The Original Code is Mozilla Communicator client code, released
 * March 31, 1998.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Sean Su <ssu@netscape.com>
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

#ifndef _EXTRA_H_
#define _EXTRA_H_

#define INCL_PM
#define INCL_DOS
#define INCL_DOSERRORS
#include <os2.h>

typedef long HRESULT;

typedef struct diskSpaceNode dsN;
struct diskSpaceNode
{
  unsigned long long       ullSpaceRequired;
  PSZ             szPath;
  dsN             *Next;
  dsN             *Prev;
};

typedef struct structVer
{
  unsigned long long ullMajor;
  unsigned long long ullMinor;
  unsigned long long ullRelease;
  unsigned long long ullBuild;
} verBlock;

BOOL              InitApplication(HMODULE hInstance);
BOOL              InitInstance(HMODULE hInstance, ULONG dwCmdShow);
void              PrintError(PSZ   szMsg, ULONG dwErrorCodeSH);
void              FreeMemory(void **vPointer);
void              *NS_GlobalAlloc(ULONG dwMaxBuf);
HRESULT           Initialize(HMODULE hInstance, PSZ szAppName);
HRESULT           NS_LoadStringAlloc(HMODULE hInstance, ULONG dwID, PSZ   *szStringBuf, ULONG dwStringBuf);
HRESULT           NS_LoadString(HMODULE hInstance, ULONG dwID, PSZ   szStringBuf, ULONG dwStringBuf);
HRESULT           WinSpawn(PSZ   szClientName, PSZ   szParameters, PSZ   szCurrentDir, BOOL bWait);
HRESULT           ParseConfigIni(PSZ   lpszCmdLine);
HRESULT           DecryptString(PSZ   szOutputStr, PSZ   szInputStr);
HRESULT           DecryptVariable(PSZ   szVariable, ULONG dwVariableSize);
//void              GetWinReg(HKEY hkRootKey, PSZ   szKey, PSZ   szName, PSZ   szReturnValue, ULONG dwSize);
//void              SetWinReg(HKEY hkRootKey, PSZ   szKey, PSZ   szName, ULONG dwType, PSZ   szData, ULONG dwSize);
//void              SetWinRegNumValue(HKEY hkRootKey, PSZ   szKey, PSZ   szName, ULONG dwData);
HRESULT           InitUninstallGeneral(void);
HRESULT           InitDlgUninstall(diU *diDialog);
sil               *CreateSilNode();
void              SilNodeInsert(sil *silHead, sil *silTemp);
void              SilNodeDelete(sil *silTemp);
void              DeInitialize(void);
void              DeInitDlgUninstall(diU *diDialog);
void              DetermineOSVersion(void);
void              DeInitILomponents(void);
void              DeInitUninstallGeneral(void);
HRESULT           ParseUninstallIni(int argc, char *argv[]);
void              ParsePath(PSZ   szInput, PSZ   szOutput, ULONG dwLength, ULONG dwType);
void              RemoveBackSlash(PSZ   szInput);
void              AppendBackSlash(PSZ   szInput, ULONG dwInputSize);
void              RemoveSlash(PSZ   szInput);
void              AppendSlash(PSZ   szInput, ULONG dwInputSize);
HRESULT           FileExists(PSZ   szFile);
BOOL              isFAT(char* szPath);
HRESULT           CheckInstances();
void              RemoveQuotes(PSZ   lpszSrc, PSZ   lpszDest, int iDestSize);
PSZ               GetFirstNonSpace(PSZ   lpszString);
ULONG             ParseCommandLine(int argc, char *argv[]);
void              SetUninstallRunMode(PSZ   szMode);
void              Delay(ULONG dwSeconds);
HRESULT           GetAppPath();
ULONG             CleanupAppList();
//BOOL              ProcessAppItem(HKEY hkRootKey, PSZ   szAppItem);
HRESULT           GetUninstallLogPath();

HWND              FindWindow(PCSZ pszAtomString);

#endif

