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

#ifndef _EXTERN_H_
#define _EXTERN_H_

#include "setup.h"

/* external global variables */
extern HINSTANCE        hInst;
extern HINSTANCE        hSetupRscInst;
extern HINSTANCE        hSDInst;
extern HINSTANCE        hXPIStubInst;

extern HBITMAP          hbmpBoxChecked;
extern HBITMAP          hbmpBoxCheckedDisabled;
extern HBITMAP          hbmpBoxUnChecked;

extern HANDLE           hAccelTable;

extern HWND             hDlgCurrent;
extern HWND             hDlgMessage;
extern HWND             hWndMain;
extern HWND             hWizardWnd;

extern LPSTR            szEGlobalAlloc;
extern LPSTR            szEStringLoad;
extern LPSTR            szEDllLoad;
extern LPSTR            szEStringNull;
extern LPSTR            szEOutOfMemory;
extern LPSTR            szTempSetupPath;

extern LPSTR            szSetupDir;
extern LPSTR            szTempDir;
extern LPSTR            szOSTempDir;
extern LPSTR            szFileIniInstall;

extern LPSTR            szSiteSelectorDescription;

extern DWORD            dwWizardState;
extern DWORD            dwSetupType;

extern DWORD            dwTempSetupType;
extern DWORD            gdwUpgradeValue;
extern DWORD            gdwSiteSelectorStatus;

extern BOOL             bSDUserCanceled;
extern BOOL             bIdiArchivesExists;
extern BOOL             bCreateDestinationDir;
extern BOOL             bReboot;
extern BOOL             gbILUseTemp;
extern BOOL             gbPreviousUnfinishedDownload;
extern BOOL             gbIgnoreRunAppX;
extern BOOL             gbIgnoreProgramFolderX;
extern BOOL             gbRestrictedAccess;
extern BOOL             gbDownloadTriggered;
extern BOOL             gbAllowMultipleInstalls;
extern BOOL             gbForceInstall;
extern BOOL             gbForceInstallGre;
extern BOOL             gShowBannerImage;

extern setupGen         sgProduct;
extern diS              diSetup;
extern diW              diWelcome;
extern diQL             diQuickLaunch;
extern diL              diLicense;
extern diST             diSetupType;
extern diSC             diSelectComponents;
extern diSIP            diSelectInstallPath;
extern diU              diUpgrade;
extern diSC             diSelectAdditionalComponents;
extern diWI             diWindowsIntegration;
extern diPF             diProgramFolder;
extern diDO             diAdditionalOptions;
extern diAS             diAdvancedSettings;
extern diSI             diStartInstall;
extern diDL             diDownloading;
extern diI              diInstalling;
extern diIS             diInstallSuccessful;
extern diD              diDownload;
extern diR              diReboot;
extern siSD             siSDObject;
extern siCF             siCFXpcomFile;
extern siC              *siComponents;
extern ssi              *ssiSiteSelector;
extern char             *SetupFileList[];
extern installGui       sgInstallGui;
extern sems             gErrorMessageStream;
extern sysinfo          gSystemInfo;
extern dsN              *gdsnComponentDSRequirement;

#endif /* _EXTERN_H */

