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
 * Portions created by the Initial Developer are Copyright (C) 2002
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *  Brian Ryner <bryner@brianryner.com>
 *  Benjamin Smedberg <bsmedberg@covad.net>
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

#ifndef _nsXULAppAPI_h__
#define _nsXULAppAPI_h__

#include "prtypes.h"
#include "nsID.h"
#include "xrecore.h"
#include "nsXPCOM.h"
#include "nsISupports.h"

/**
 * Application-specific data needed to start the apprunner.
 *
 * @status UNDER_REVIEW - This API is under review to be frozen, but isn't
 *                        frozen yet. Use with caution.
 */
struct nsXREAppData
{
  /**
   * This should be set to sizeof(nsXREAppData). This structure may be
   * extended in future releases, and this ensures that binary compatibility
   * is maintained.
   */
  PRUint32 size;

  /**
   * The directory of the application to be run. May be null if the
   * xulrunner and the app are installed into the same directory.
   */
  nsILocalFile* directory;

  /**
   * The name of the application vendor. This must be ASCII, and is normally
   * mixed-case, e.g. "Mozilla". Optional (may be null), but highly
   * recommended. Must not be the empty string.
   */
  const char *vendor;

  /**
   * The name of the application. This must be ASCII, and is normally
   * mixed-case, e.g. "Firefox". Required (must not be null or an empty
   * string).
   */
  const char *name;

  /**
   * The major version, e.g. "0.8.0+". Optional (may be null), but
   * required for advanced application features such as the extension
   * manager and update service. Must not be the empty string.
   */
  const char *version;

  /** 
   * The application's build identifier, e.g. "2004051604"
   */
  const char *buildID;

  /**
   * The application's UUID. Used by the extension manager to determine
   * compatible extensions. Optional, but required for advanced application
   * features such as the extension manager and update service.
   *
   * This has traditionally been in the form
   * "{AAAAAAAA-BBBB-CCCC-DDDD-EEEEEEEEEEEE}" but for new applications
   * a more readable form is encouraged: "appname@vendor.tld". Only
   * the following characters are allowed: a-z A-Z 0-9 - . @ _ { } *
   */
  const char *ID;

  /**
   * The copyright information to print for the -h commandline flag,
   * e.g. "Copyright (c) 2003 mozilla.org".
   */
  const char *copyright;

  /**
   * Combination of NS_XRE_ prefixed flags (defined below).
   */
  PRUint32 flags;
};

/**
 * Indicates whether or not the profile migrator service may be
 * invoked at startup when creating a profile.
 */
#define NS_XRE_ENABLE_PROFILE_MIGRATOR (1 << 1)

/**
 * Indicates whether or not the extension manager service should be
 * initialized at startup.
 */
#define NS_XRE_ENABLE_EXTENSION_MANAGER (1 << 2)

/**
 * The contract id for the nsIXULAppInfo service.
 */
#define XULAPPINFO_SERVICE_CONTRACTID \
  "@mozilla.org/xre/app-info;1"

/**
 * A directory service key which provides the platform-correct
 * "application data" directory.
 * Windows: Documents and Settings\<User>\Application Data\<Vendor>\<Application>
 * Unix: ~/.<vendor>/<application>
 * Mac: ~/Library/Application Supports/<Application>
 */
#define XRE_USER_APP_DATA_DIR "UAppData"

/**
 * A directory service key which provides a list of all enabled extension
 * directories. The list includes compatible platform-specific extension
 * subdirectories.
 *
 * @note The directory list will have no members when the application is
 *       launched in safe mode.
 */
#define XRE_EXTENSIONS_DIR_LIST "XREExtDL"

/**
 * A directory service key which provides the executable file used to
 * launch the current process.  This is the same value returned by the
 * XRE_GetBinaryPath function defined below.
 */
#define XRE_EXECUTABLE_FILE "XREExeF"

/**
 * A directory service key which specifies the profile
 * directory. Unlike the NS_APP_USER_PROFILE_50_DIR key, this key may
 * be available when the profile hasn't been "started", or after is
 * has been shut down. If the application is running without a
 * profile, such as when showing the profile manager UI, this key will
 * not be available. This key is provided by the XUL apprunner or by
 * the aAppDirProvider object passed to XRE_InitEmbedding.
 */
#define NS_APP_PROFILE_DIR_STARTUP "ProfDS"

/**
 * A directory service key which specifies the profile
 * directory. Unlike the NS_APP_USER_PROFILE_LOCAL_50_DIR key, this key may
 * be available when the profile hasn't been "started", or after is
 * has been shut down. If the application is running without a
 * profile, such as when showing the profile manager UI, this key will
 * not be available. This key is provided by the XUL apprunner or by
 * the aAppDirProvider object passed to XRE_InitEmbedding.
 */
#define NS_APP_PROFILE_LOCAL_DIR_STARTUP "ProfLDS"

/**
 * Begin an XUL application. Does not return until the user exits the
 * application.
 *
 * @param argc/argv Command-line parameters to pass to the application. These
 *                  are in the "native" character set.
 *
 * @param aAppData  Information about the application to be run.
 *
 * @return         A native result code suitable for returning from main().
 *
 * @note           If the binary is linked against the  standalone XPCOM glue,
 *                 XPCOMGlueStartup() should be called before this method.
 *
 * @note           XXXbsmedberg Nobody uses the glue yet, but there is a
 *                 potential problem: on windows, the standalone glue calls
 *                 SetCurrentDirectory, and relative paths on the command line
 *                 won't be correct.
 */
XRE_API(int,
        XRE_main, (int argc, char* argv[], const nsXREAppData* sAppData))

/**
 * Given a path relative to the current working directory (or an absolute
 * path), return an appropriate nsILocalFile object.
 */
XRE_API(nsresult,
        XRE_GetFileFromPath, (const char *aPath, nsILocalFile* *aResult))

/**
 * Get the path of the running application binary and store it in aResult.
 * @param argv0   The value passed as argv[0] of main(). This value is only
 *                used on *nix, and only when other methods of determining
 *                the binary path have failed.
 */
XRE_API(nsresult,
        XRE_GetBinaryPath, (const char *argv0, nsILocalFile* *aResult))

/**
 * Get the static components built in to libxul.
 */
XRE_API(void,
        XRE_GetStaticComponents, (nsStaticModuleInfo const **aStaticComponents,
                                  PRUint32 *aComponentCount))

/**
 * Lock a profile directory using platform-specific semantics.
 *
 * @param aDirectory  The profile directory to lock.
 * @param aLockObject An opaque lock object. The directory will remain locked
 *                    as long as the XPCOM reference is held.
 */
XRE_API(nsresult,
        XRE_LockProfileDirectory, (nsILocalFile* aDirectory,
                                   nsISupports* *aLockObject))

/**
 * Initialize libXUL for embedding purposes.
 *
 * @param aLibXULDirectory   The directory in which the libXUL shared library
 *                           was found.
 * @param aAppDirectory      The directory in which the application components
 *                           and resources can be found. This will map to
 *                           the NS_OS_CURRENT_PROCESS_DIR directory service
 *                           key.
 * @param aAppDirProvider    A directory provider for the application. This
 *                           provider will be aggregated by a libxul provider
 *                           which will provide the base required GRE keys.
 * @param aStaticComponents  Static components provided by the embedding
 *                           application. This should *not* include the
 *                           components from XRE_GetStaticComponents. May be
 *                           null if there are no static components.
 * @param aStaticComponentCount the number of static components in
 *                           aStaticComponents
 *
 * @note This function must be called from the "main" thread.
 *
 * @note At the present time, this function may only be called once in
 * a given process. Use XRE_TermEmbedding to clean up and free
 * resources allocated by XRE_InitEmbedding.
 */

XRE_API(nsresult,
        XRE_InitEmbedding, (nsILocalFile *aLibXULDirectory,
                            nsILocalFile *aAppDirectory,
                            nsIDirectoryServiceProvider *aAppDirProvider,
                            nsStaticModuleInfo const *aStaticComponents,
                            PRUint32 aStaticComponentCount))

/**
 * Fire notifications to inform the toolkit about a new profile. This
 * method should be called after XRE_InitEmbedding if the embedder
 * wishes to run with a profile. Normally the embedder should call
 * XRE_LockProfileDirectory to lock the directory before calling this
 * method.
 *
 * @note There are two possibilities for selecting a profile:
 *
 * 1) Select the profile before calling XRE_InitEmbedding. The aAppDirProvider
 *    object passed to XRE_InitEmbedding should provide the
 *    NS_APP_USER_PROFILE_50_DIR key, and may also provide the following keys:
 *    - NS_APP_USER_PROFILE_LOCAL_50_DIR
 *    - NS_APP_PROFILE_DIR_STARTUP
 *    - NS_APP_PROFILE_LOCAL_DIR_STARTUP
 *    In this scenario XRE_NotifyProfile should be called immediately after
 *    XRE_InitEmbedding. Component registration information will be stored in
 *    the profile and JS components may be stored in the fastload cache.
 *
 * 2) Select a profile some time after calling XRE_InitEmbedding. In this case
 *    the embedder must install a directory service provider which provides
 *    NS_APP_USER_PROFILE_50_DIR and optionally
 *    NS_APP_USER_PROFILE_LOCAL_50_DIR. Component registration information
 *    will be stored in the application directory and JS components will not
 *    fastload.
 */
XRE_API(void,
        XRE_NotifyProfile, ())

/**
 * Terminate embedding started with XRE_InitEmbedding or XRE_InitEmbedding2
 */
XRE_API(void,
        XRE_TermEmbedding, ())

#endif // _nsXULAppAPI_h__
