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
 * The Original Code is the Netscape security libraries.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1994-2000
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

#ifndef PK11INSTALL_H
#define PK11INSTALL_H

#include <prio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*Pk11Install_ErrorHandler)(char *);

typedef enum {
	PK11_INSTALL_NO_ERROR=0,
	PK11_INSTALL_DIR_DOESNT_EXIST,
	PK11_INSTALL_FILE_DOESNT_EXIST,
	PK11_INSTALL_FILE_NOT_READABLE,
	PK11_INSTALL_ERROR_STRING,
	PK11_INSTALL_JAR_ERROR,
	PK11_INSTALL_NO_INSTALLER_SCRIPT,
	PK11_INSTALL_DELETE_TEMP_FILE,
	PK11_INSTALL_OPEN_SCRIPT_FILE,
	PK11_INSTALL_SCRIPT_PARSE,
	PK11_INSTALL_SEMANTIC,
	PK11_INSTALL_SYSINFO,
	PK11_INSTALL_NO_PLATFORM,
	PK11_INSTALL_BOGUS_REL_DIR,
	PK11_INSTALL_NO_MOD_FILE,
	PK11_INSTALL_ADD_MODULE,
	PK11_INSTALL_JAR_EXTRACT,
	PK11_INSTALL_DIR_NOT_WRITEABLE,
	PK11_INSTALL_CREATE_DIR,
	PK11_INSTALL_REMOVE_DIR,
	PK11_INSTALL_EXEC_FILE,
	PK11_INSTALL_WAIT_PROCESS,
	PK11_INSTALL_PROC_ERROR,
	PK11_INSTALL_USER_ABORT,
	PK11_INSTALL_UNSPECIFIED
} Pk11Install_Error;
#define PK11_INSTALL_SUCCESS PK11_INSTALL_NO_ERROR

/**************************************************************************
 *
 * P k 1 1 I n s t a l l _ I n i t
 *
 * Does initialization that otherwise would be done on the fly.  Only
 * needs to be called by multithreaded apps, before they make any calls
 * to this library.
 */
void 
Pk11Install_Init();

/**************************************************************************
 *
 * P k 1 1 I n s t a l l _ S e t E r r o r H a n d l e r
 *
 * Sets the error handler to be used by the library.  Returns the current
 * error handler function.
 */
Pk11Install_ErrorHandler
Pk11Install_SetErrorHandler(Pk11Install_ErrorHandler handler);


/**************************************************************************
 *
 * P k 1 1 I n s t a l l _ R e l e a s e
 *
 * Releases static data structures used by the library.  Don't use the
 * library after calling this, unless you call Pk11Install_Init()
 * first.  This function doesn't have to be called at all unless you're
 * really anal about freeing memory before your program exits.
 */
void 
Pk11Install_Release();

/*************************************************************************
 *
 * P k 1 1 I n s t a l l _ D o I n s t a l l
 *
 * jarFile is the path of a JAR in the PKCS #11 module JAR format.
 * installDir is the directory relative to which files will be
 *   installed.
 * feedback is a file descriptor to which to write informative (not error)
 * status messages: what files are being installed, what modules are being
 * installed.  If feedback==NULL, no messages will be displayed.
 * If force != 0, interactive prompts will be suppressed.
 * If noverify == PR_TRUE, signatures won't be checked on the JAR file.
 */
Pk11Install_Error
Pk11Install_DoInstall(char *jarFile, const char *installDir,
	const char *tempDir, PRFileDesc *feedback, short force,
	PRBool noverify);

#ifdef __cplusplus
}
#endif

#endif /*PK11INSTALL_H*/
