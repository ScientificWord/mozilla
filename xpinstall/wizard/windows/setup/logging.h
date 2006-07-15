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
 * The Original Code is Mozilla Navigator.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corp.
 * Portions created by the Initial Developer are Copyright (C) 2001
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

#ifndef _LOGGING_H_
#define _LOGGING_H_

int               AppendToGlobalMessageStream(char *szInfo);
void              LogISTime(int iType);
void              LogISProductInfo(void);
void              LogISDestinationPath(void);
void              LogISSetupType(void);
void              LogISComponentsSelected(void);
void              LogISComponentsToDownload(void);
void              LogISComponentsFailedCRC(char *szList, int iWhen);
void              LogISDownloadStatus(char *szStatus, char *szFailedFile);
void              LogISDownloadProtocol(DWORD dwProtocolType);
void              LogISXPInstall(int iWhen);
void              LogISXPInstallComponent(char *szComponentName);
void              LogISXPInstallComponentResult(DWORD dwErrorNumber);
void              LogISLaunchApps(int iWhen);
void              LogISLaunchAppsComponent(char *szComponentName);
void              LogISLaunchAppsComponentUncompress(char *szComponentName,
                                                     DWORD dwErr);
void              LogISProcessXpcomFile(int iStatus, int iResult);
void              LogISShared(void);
void              LogISDiskSpace(dsN *dsnComponentDSRequirement);
void              LogISTurboMode(BOOL bTurboMode);
void              LogMSProductInfo(void);
void              LogMSDownloadFileStatus(void);
void              LogMSDownloadStatus(int iDownloadStatus);
void              LogMSDownloadProtocol(DWORD dwProtocolType);
void              LogMSXPInstallStatus(char *szFile, int iErr);
void              LogMSTurboMode(BOOL bTurboMode);
void              DeleteExitStatusFile(void);
void              LogExitStatus(LPSTR status);
void              GetGreSetupExitStatus(LPSTR aStatus, DWORD aStatusBufSize);

#endif /* _LOGGING_H_ */

