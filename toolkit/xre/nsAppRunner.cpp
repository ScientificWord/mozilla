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
 * The Original Code is Mozilla Communicator client code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Roland Mainz <roland.mainz@informatik.med.uni-giessen.de>
 *   Benjamin Smedberg <benjamin@smedbergs.us>
 *   Ben Goodger <ben@mozilla.org>
 *   Fredrik Holmqvist <thesuckiestemail@yahoo.se>
 *   Ben Turner <mozilla@songbirdnest.com>
 *   Sergei Dolgov <sergei_d@fi.tartu.ee>
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

#if defined(XP_OS2) && defined(MOZ_OS2_HIGH_MEMORY)
// os2safe.h has to be included before os2.h, needed for high mem
#include <os2safe.h>
#endif

#define XPCOM_TRANSLATE_NSGM_ENTRY_POINT 1

#include "nsAppRunner.h"
#include "nsUpdateDriver.h"

#ifdef XP_MACOSX
#include "MacLaunchHelper.h"
#include "MacApplicationDelegate.h"
#endif

#ifdef XP_OS2
#include "private/pprthred.h"
#endif
#include "prmem.h"
#include "prnetdb.h"
#include "prprf.h"
#include "prproces.h"
#include "prenv.h"

#include "nsIAppShellService.h"
#include "nsIAppStartup.h"
#include "nsIAppStartupNotifier.h"
#include "nsIMutableArray.h"
#include "nsICategoryManager.h"
#include "nsIChromeRegistry.h"
#include "nsICommandLineRunner.h"
#include "nsIComponentManager.h"
#include "nsIComponentRegistrar.h"
#include "nsIContentHandler.h"
#include "nsIDialogParamBlock.h"
#include "nsIDOMWindow.h"
#include "nsIExtensionManager.h"
#include "nsIFastLoadService.h" // for PLATFORM_FASL_SUFFIX
#include "nsIGenericFactory.h"
#include "nsIIOService2.h"
#include "nsIObserverService.h"
#include "nsINativeAppSupport.h"
#include "nsIProcess.h"
#include "nsIProfileUnlocker.h"
#include "nsIPromptService.h"
#include "nsIServiceManager.h"
#include "nsIStringBundle.h"
#include "nsISupportsPrimitives.h"
#include "nsITimelineService.h"
#include "nsIToolkitChromeRegistry.h"
#include "nsIToolkitProfile.h"
#include "nsIToolkitProfileService.h"
#include "nsIURI.h"
#include "nsIWindowCreator.h"
#include "nsIWindowMediator.h"
#include "nsIWindowWatcher.h"
#include "nsIXULAppInfo.h"
#include "nsIXULRuntime.h"
#include "nsPIDOMWindow.h"
#include "nsIBaseWindow.h"
#include "nsIWidget.h"
#include "nsIDocShell.h"
#include "nsAppShellCID.h"

#ifdef XP_WIN
#include "nsIWinAppHelper.h"
#endif

#include "nsCRT.h"
#include "nsCOMPtr.h"
#include "nsDirectoryServiceDefs.h"
#include "nsDirectoryServiceUtils.h"
#include "nsEmbedCID.h"
#include "nsNetUtil.h"
#include "nsReadableUtils.h"
#include "nsStaticComponents.h"
#include "nsXPCOM.h"
#include "nsXPIDLString.h"
#include "nsXPFEComponentsCID.h"
#include "nsVersionComparator.h"

#include "nsAppDirectoryServiceDefs.h"
#include "nsXULAppAPI.h"
#include "nsXREDirProvider.h"
#include "nsToolkitCompsCID.h"

#include "nsINIParser.h"

#include <stdlib.h>

#ifdef XP_UNIX
#include <sys/stat.h>
#include <unistd.h>
#endif

#ifdef XP_BEOS
// execv() behaves bit differently in R5 and Zeta, looks unreliable in such situation
//#include <unistd.h>
#include <AppKit.h>
#include <AppFileInfo.h>
#endif //XP_BEOS

#ifdef XP_WIN
#include <process.h>
#include <shlobj.h>
#include "nsThreadUtils.h"
#endif

#ifdef XP_MACOSX
#include "nsILocalFileMac.h"
#include "nsCommandLineServiceMac.h"
#endif

// for X remote support
#ifdef MOZ_ENABLE_XREMOTE
#ifdef MOZ_WIDGET_PHOTON
#include "PhRemoteClient.h"
#else
#include "XRemoteClient.h"
#endif
#include "nsIRemoteService.h"
#endif

#ifdef NS_TRACE_MALLOC
#include "nsTraceMalloc.h"
#endif

#if defined(DEBUG) && defined(XP_WIN32)
#include <malloc.h>
#endif

#if defined (XP_MACOSX)
#include <Processes.h>
#include <Events.h>
#endif

extern "C" void ShowOSAlert(const char* aMessage);

#ifdef DEBUG
#include "prlog.h"
#endif

#ifdef MOZ_JPROF
#include "jprof.h"
#endif

#ifdef MOZ_CRASHREPORTER
#include "nsExceptionHandler.h"
#include "nsICrashReporter.h"
#define NS_CRASHREPORTER_CONTRACTID "@mozilla.org/toolkit/crash-reporter;1"
#endif

// on x86 linux, the current builds of some popular plugins (notably
// flashplayer and real) expect a few builtin symbols from libgcc
// which were available in some older versions of gcc.  However,
// they're _NOT_ available in newer versions of gcc (eg 3.1), so if
// we want those plugin to work with a gcc-3.1 built binary, we need
// to provide these symbols.  MOZ_ENABLE_OLD_ABI_COMPAT_WRAPPERS defaults
// to true on x86 linux, and false everywhere else.
//
// The fact that the new and free operators are mismatched 
// mirrors the way the original functions in egcs 1.1.2 worked.

#ifdef MOZ_ENABLE_OLD_ABI_COMPAT_WRAPPERS

extern "C" {

# ifndef HAVE___BUILTIN_VEC_NEW
  void *__builtin_vec_new(size_t aSize, const std::nothrow_t &aNoThrow) throw()
  {
    return ::operator new(aSize, aNoThrow);
  }
# endif

# ifndef HAVE___BUILTIN_VEC_DELETE
  void __builtin_vec_delete(void *aPtr, const std::nothrow_t &) throw ()
  {
    if (aPtr) {
      free(aPtr);
    }
  }
# endif

# ifndef HAVE___BUILTIN_NEW
	void *__builtin_new(int aSize)
  {
    return malloc(aSize);
  }
# endif

# ifndef HAVE___BUILTIN_DELETE
	void __builtin_delete(void *aPtr)
  {
    free(aPtr);
  }
# endif

# ifndef HAVE___PURE_VIRTUAL
  void __pure_virtual(void) {
#ifdef WRAP_SYSTEM_INCLUDES
#pragma GCC visibility push(default)
#endif
    extern void __cxa_pure_virtual(void);
#ifdef WRAP_SYSTEM_INCLUDES
#pragma GCC visibility pop
#endif

    __cxa_pure_virtual();
  }
# endif
}
#endif

#if defined(XP_UNIX) || defined(XP_BEOS)
  extern void InstallUnixSignalHandlers(const char *ProgramName);
#endif

int    gArgc;
char **gArgv;

static char gToolkitVersion[20];
static char gToolkitBuildID[40];

static int    gRestartArgc;
static char **gRestartArgv;

#if defined(MOZ_WIDGET_GTK2)
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include "nsGTKToolkit.h"
#endif

// Save the given word to the specified environment variable.
static void
SaveWordToEnv(const char *name, const nsACString & word)
{
  char *expr = PR_smprintf("%s=%s", name, PromiseFlatCString(word).get());
  if (expr)
    PR_SetEnv(expr);
  // We intentionally leak |expr| here since it is required by PR_SetEnv.
}

// Save the path of the given file to the specified environment variable.
static void
SaveFileToEnv(const char *name, nsIFile *file)
{
#ifdef XP_WIN
  nsAutoString path;
  file->GetPath(path);
  SetEnvironmentVariableW(NS_ConvertASCIItoUTF16(name).get(), path.get());
#else
  nsCAutoString path;
  file->GetNativePath(path);
  SaveWordToEnv(name, path);
#endif
}

// Load the path of a file saved with SaveFileToEnv
static already_AddRefed<nsILocalFile>
GetFileFromEnv(const char *name)
{
  nsresult rv;
  nsILocalFile *file = nsnull;

#ifdef XP_WIN
  WCHAR path[_MAX_PATH];
  if (!GetEnvironmentVariableW(NS_ConvertASCIItoUTF16(name).get(),
                               path, _MAX_PATH))
    return nsnull;

  rv = NS_NewLocalFile(nsDependentString(path), PR_TRUE, &file);
  if (NS_FAILED(rv))
    return nsnull;

  return file;
#else
  const char *arg = PR_GetEnv(name);
  if (!arg || !*arg)
    return nsnull;

  rv = NS_NewNativeLocalFile(nsDependentCString(arg), PR_TRUE, &file);
  if (NS_FAILED(rv))
    return nsnull;

  return file;
#endif
}

// Save the path of the given word to the specified environment variable
// provided the environment variable does not have a value.
static void
SaveWordToEnvIfUnset(const char *name, const nsACString & word)
{
  const char *val = PR_GetEnv(name);
  if (!(val && *val))
    SaveWordToEnv(name, word);
}

// Save the path of the given file to the specified environment variable
// provided the environment variable does not have a value.
static void
SaveFileToEnvIfUnset(const char *name, nsIFile *file)
{
  const char *val = PR_GetEnv(name);
  if (!(val && *val))
    SaveFileToEnv(name, file);
}

static PRBool
strimatch(const char* lowerstr, const char* mixedstr)
{
  while(*lowerstr) {
    if (!*mixedstr) return PR_FALSE; // mixedstr is shorter
    if (tolower(*mixedstr) != *lowerstr) return PR_FALSE; // no match

    ++lowerstr;
    ++mixedstr;
  }

  if (*mixedstr) return PR_FALSE; // lowerstr is shorter

  return PR_TRUE;
}

/**
 * Output a string to the user.  This method is really only meant to be used to
 * output last-ditch error messages designed for developers NOT END USERS.
 *
 * @param isError
 *        Pass true to indicate severe errors.
 * @param fmt
 *        printf-style format string followed by arguments.
 */
static void Output(PRBool isError, const char *fmt, ... )
{
  va_list ap;
  va_start(ap, fmt);

#if defined(XP_WIN) && !MOZ_WINCONSOLE
  char *msg = PR_vsmprintf(fmt, ap);
  if (msg)
  {
    UINT flags = MB_OK;
    if (isError)
      flags |= MB_ICONERROR;
    else 
      flags |= MB_ICONINFORMATION;
    MessageBoxA(NULL, msg, "XULRunner", flags);
    PR_smprintf_free(msg);
  }
#else
  vfprintf(stderr, fmt, ap);
#endif

  va_end(ap);
}

enum RemoteResult {
  REMOTE_NOT_FOUND  = 0,
  REMOTE_FOUND      = 1,
  REMOTE_ARG_BAD    = 2
};

enum ArgResult {
  ARG_NONE  = 0,
  ARG_FOUND = 1,
  ARG_BAD   = 2 // you wanted a param, but there isn't one
};

static void RemoveArg(char **argv)
{
  do {
    *argv = *(argv + 1);
    ++argv;
  } while (*argv);

  --gArgc;
}

/**
 * Check for a commandline flag. If the flag takes a parameter, the
 * parameter is returned in aParam. Flags may be in the form -arg or
 * --arg (or /arg on win32/OS2).
 *
 * @param aArg the parameter to check. Must be lowercase.
 * @param aCheckOSInt if true returns ARG_BAD if the osint argument is present
 *        when aArg is also present.
 * @param if non-null, the -arg <data> will be stored in this pointer. This is *not*
 *        allocated, but rather a pointer to the argv data.
 */
static ArgResult
CheckArg(const char* aArg, PRBool aCheckOSInt = PR_FALSE, const char **aParam = nsnull)
{
  char **curarg = gArgv + 1; // skip argv[0]
  ArgResult ar = ARG_NONE;

  while (*curarg) {
    char *arg = curarg[0];

    if (arg[0] == '-'
#if defined(XP_WIN) || defined(XP_OS2)
        || *arg == '/'
#endif
        ) {
      ++arg;
      if (*arg == '-')
        ++arg;

      if (strimatch(aArg, arg)) {
        RemoveArg(curarg);
        if (!aParam) {
          ar = ARG_FOUND;
          break;
        }

        if (*curarg) {
          if (**curarg == '-'
#if defined(XP_WIN) || defined(XP_OS2)
              || **curarg == '/'
#endif
              )
            return ARG_BAD;

          *aParam = *curarg;
          RemoveArg(curarg);
          ar = ARG_FOUND;
          break;
        }
        return ARG_BAD;
      }
    }

    ++curarg;
  }

  if (aCheckOSInt && ar == ARG_FOUND) {
    ArgResult arOSInt = CheckArg("osint");
    if (arOSInt == ARG_FOUND) {
      ar = ARG_BAD;
      PR_fprintf(PR_STDERR, "Error: argument -osint is invalid\n");
    }
  }

  return ar;
}

#if defined(XP_WIN)
/**
 * Check for a commandline flag from the windows shell and remove it from the
 * argv used when restarting. Flags MUST be in the form -arg.
 *
 * @param aArg the parameter to check. Must be lowercase.
 */
static ArgResult
CheckArgShell(const char* aArg)
{
  char **curarg = gRestartArgv + 1; // skip argv[0]

  while (*curarg) {
    char *arg = curarg[0];

    if (arg[0] == '-') {
      ++arg;

      if (strimatch(aArg, arg)) {
        do {
          *curarg = *(curarg + 1);
          ++curarg;
        } while (*curarg);

        --gRestartArgc;

        return ARG_FOUND;
      }
    }

    ++curarg;
  }

  return ARG_NONE;
}

/**
 * Spins up Windows DDE when the app needs to restart or the profile manager
 * will be displayed during startup and the app has been launched by the Windows
 * shell to open an url. This prevents Windows from displaying an error message
 * due to the DDE message not being acknowledged.
 */
static void
ProcessDDE(nsINativeAppSupport* aNative)
{
  // When the app is launched by the windows shell the windows shell
  // expects the app to be available for DDE messages and if it isn't
  // windows displays an error dialog. To prevent the error the DDE server
  // is enabled and pending events are processed when the app needs to
  // restart after it was launched by the shell with the requestpending
  // argument. The requestpending pending argument is removed to
  // differentiate it from being launched when an app restart is not
  // required.
  ArgResult ar;
  ar = CheckArgShell("requestpending");
  if (ar == ARG_FOUND) {
    aNative->Enable(); // enable win32 DDE responses
    nsIThread *thread = NS_GetCurrentThread();
    // This is just a guesstimate based on testing different values.
    // If count is 8 or less windows will display an error dialog.
    PRInt32 count = 20;
    while(--count >= 0) {
      NS_ProcessNextEvent(thread);
      PR_Sleep(PR_MillisecondsToInterval(1));
    }
  }
}
#endif

PRBool gSafeMode = PR_FALSE;

/**
 * The nsXULAppInfo object implements nsIFactory so that it can be its own
 * singleton.
 */
class nsXULAppInfo : public nsIXULAppInfo,
#ifdef XP_WIN
                     public nsIWinAppHelper,
#endif
#ifdef MOZ_CRASHREPORTER
                     public nsICrashReporter,
#endif
                     public nsIXULRuntime
                     
{
public:
  NS_DECL_ISUPPORTS_INHERITED
  NS_DECL_NSIXULAPPINFO
  NS_DECL_NSIXULRUNTIME
#ifdef MOZ_CRASHREPORTER
  NS_DECL_NSICRASHREPORTER
#endif
#ifdef XP_WIN
  NS_DECL_NSIWINAPPHELPER
private:
  nsresult LaunchAppHelperWithArgs(int aArgc, char **aArgv);
#endif
};

NS_INTERFACE_MAP_BEGIN(nsXULAppInfo)
  NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(nsISupports, nsIXULRuntime)
  NS_INTERFACE_MAP_ENTRY(nsIXULRuntime)
#ifdef XP_WIN
  NS_INTERFACE_MAP_ENTRY(nsIWinAppHelper)
#endif
#ifdef MOZ_CRASHREPORTER
  NS_INTERFACE_MAP_ENTRY(nsICrashReporter)
#endif
  NS_INTERFACE_MAP_ENTRY_CONDITIONAL(nsIXULAppInfo, gAppData)
NS_INTERFACE_MAP_END

NS_IMETHODIMP_(nsrefcnt)
nsXULAppInfo::AddRef()
{
  return 1;
}

NS_IMETHODIMP_(nsrefcnt)
nsXULAppInfo::Release()
{
  return 1;
}

NS_IMETHODIMP
nsXULAppInfo::GetVendor(nsACString& aResult)
{
  aResult.Assign(gAppData->vendor);

  return NS_OK;
}

NS_IMETHODIMP
nsXULAppInfo::GetName(nsACString& aResult)
{
  aResult.Assign(gAppData->name);

  return NS_OK;
}

NS_IMETHODIMP
nsXULAppInfo::GetID(nsACString& aResult)
{
  aResult.Assign(gAppData->ID);

  return NS_OK;
}

NS_IMETHODIMP
nsXULAppInfo::GetVersion(nsACString& aResult)
{
  aResult.Assign(gAppData->version);

  return NS_OK;
}

NS_IMETHODIMP
nsXULAppInfo::GetPlatformVersion(nsACString& aResult)
{
  aResult.Assign(gToolkitVersion);

  return NS_OK;
}

NS_IMETHODIMP
nsXULAppInfo::GetAppBuildID(nsACString& aResult)
{
  aResult.Assign(gAppData->buildID);

  return NS_OK;
}

NS_IMETHODIMP
nsXULAppInfo::GetPlatformBuildID(nsACString& aResult)
{
  aResult.Assign(gToolkitBuildID);

  return NS_OK;
}

NS_IMETHODIMP
nsXULAppInfo::GetLogConsoleErrors(PRBool *aResult)
{
  *aResult = gLogConsoleErrors;
  return NS_OK;
}

NS_IMETHODIMP
nsXULAppInfo::SetLogConsoleErrors(PRBool aValue)
{
  gLogConsoleErrors = aValue;
  return NS_OK;
}

NS_IMETHODIMP
nsXULAppInfo::GetInSafeMode(PRBool *aResult)
{
  *aResult = gSafeMode;
  return NS_OK;
}

NS_IMETHODIMP
nsXULAppInfo::GetOS(nsACString& aResult)
{
  aResult.AssignLiteral(OS_TARGET);
  return NS_OK;
}

NS_IMETHODIMP
nsXULAppInfo::GetXPCOMABI(nsACString& aResult)
{
#ifdef TARGET_XPCOM_ABI
  aResult.AssignLiteral(TARGET_XPCOM_ABI);
  return NS_OK;
#else
  return NS_ERROR_NOT_AVAILABLE;
#endif
}

#ifdef XP_WIN
nsresult 
nsXULAppInfo::LaunchAppHelperWithArgs(int aArgc, char **aArgv)
{
  nsresult rv;
  nsCOMPtr<nsIProperties> directoryService = 
    do_GetService(NS_DIRECTORY_SERVICE_CONTRACTID, &rv);
  NS_ENSURE_SUCCESS(rv, rv);

  nsCOMPtr<nsILocalFile> appHelper;
  rv = directoryService->Get(NS_XPCOM_CURRENT_PROCESS_DIR, NS_GET_IID(nsILocalFile), getter_AddRefs(appHelper));
  NS_ENSURE_SUCCESS(rv, rv);

  rv = appHelper->AppendNative(NS_LITERAL_CSTRING("uninstall"));
  NS_ENSURE_SUCCESS(rv, rv);

  rv = appHelper->AppendNative(NS_LITERAL_CSTRING("helper.exe"));
  NS_ENSURE_SUCCESS(rv, rv);

  nsAutoString appHelperPath;
  rv = appHelper->GetPath(appHelperPath);
  NS_ENSURE_SUCCESS(rv, rv);

  if (!WinLaunchChild(appHelperPath.get(), aArgc, aArgv, 1))
    return NS_ERROR_FAILURE;
  else
    return NS_OK;
}

NS_IMETHODIMP
nsXULAppInfo::PostUpdate(nsILocalFile *aLogFile)
{
  nsresult rv;
  int upgradeArgc = aLogFile ? 3 : 2;
  char **upgradeArgv = (char**) malloc(sizeof(char*) * (upgradeArgc + 1));

  if (!upgradeArgv)
    return NS_ERROR_OUT_OF_MEMORY;

  upgradeArgv[0] = "argv0ignoredbywinlaunchchild";
  upgradeArgv[1] = "/postupdate";

  char *pathArg = nsnull;

  if (aLogFile) {
    nsCAutoString logFilePath;
    rv = aLogFile->GetNativePath(logFilePath);
    NS_ENSURE_SUCCESS(rv, rv);

    pathArg = PR_smprintf("/uninstalllog=%s", logFilePath.get());
    if (!pathArg)
      return NS_ERROR_OUT_OF_MEMORY;

    upgradeArgv[2] = pathArg;
    upgradeArgv[3] = nsnull;
  }
  else {
    upgradeArgv[2] = nsnull;
  }

  rv = LaunchAppHelperWithArgs(upgradeArgc, upgradeArgv);
  
  if (pathArg)
    PR_smprintf_free(pathArg);

  free(upgradeArgv);
  return rv;
}

// Matches the enum in WinNT.h for the Vista SDK but renamed so that we can
// safely build with the Vista SDK and without it.
typedef enum 
{
  VistaTokenElevationTypeDefault = 1,
  VistaTokenElevationTypeFull,
  VistaTokenElevationTypeLimited
} VISTA_TOKEN_ELEVATION_TYPE;

// avoid collision with TokeElevationType enum in WinNT.h
// of the Vista SDK
#define VistaTokenElevationType static_cast< TOKEN_INFORMATION_CLASS >( 18 )

NS_IMETHODIMP
nsXULAppInfo::GetUserCanElevate(PRBool *aUserCanElevate)
{
  HANDLE hToken;

  VISTA_TOKEN_ELEVATION_TYPE elevationType;
  DWORD dwSize; 

  if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken) ||
      !GetTokenInformation(hToken, VistaTokenElevationType, &elevationType,
                           sizeof(elevationType), &dwSize)) {
    *aUserCanElevate = PR_FALSE;
  } 
  else {
    // The possible values returned for elevationType and their meanings are:
    //   TokenElevationTypeDefault: The token does not have a linked token 
    //     (e.g. UAC disabled or a standard user, so they can't be elevated)
    //   TokenElevationTypeFull: The token is linked to an elevated token 
    //     (e.g. UAC is enabled and the user is already elevated so they can't
    //      be elevated again)
    //   TokenElevationTypeLimited: The token is linked to a limited token 
    //     (e.g. UAC is enabled and the user is not elevated, so they can be
    //	    elevated)
    *aUserCanElevate = (elevationType == VistaTokenElevationTypeLimited);
  }

  if (hToken)
    CloseHandle(hToken);

  return NS_OK;
}
#endif

#ifdef MOZ_CRASHREPORTER
NS_IMETHODIMP
nsXULAppInfo::AnnotateCrashReport(const nsACString& key,
                                  const nsACString& data)
{
  return CrashReporter::AnnotateCrashReport(key, data);
}

NS_IMETHODIMP
nsXULAppInfo::WriteMinidumpForException(void* aExceptionInfo)
{
#ifdef XP_WIN32
  return CrashReporter::WriteMinidumpForException(static_cast<EXCEPTION_POINTERS*>(aExceptionInfo));
#else
  return NS_ERROR_NOT_IMPLEMENTED;
#endif
}
#endif

static const nsXULAppInfo kAppInfo;
static NS_METHOD AppInfoConstructor(nsISupports* aOuter,
                                    REFNSIID aIID, void **aResult)
{
  NS_ENSURE_NO_AGGREGATION(aOuter);

  return const_cast<nsXULAppInfo*>(&kAppInfo)->
    QueryInterface(aIID, aResult);
}

PRBool gLogConsoleErrors
#ifdef DEBUG
         = PR_TRUE;
#else
         = PR_FALSE;
#endif

#define NS_ENSURE_TRUE_LOG(x, ret)               \
  PR_BEGIN_MACRO                                 \
  if (NS_UNLIKELY(!(x))) {                       \
    NS_WARNING("NS_ENSURE_TRUE(" #x ") failed"); \
    gLogConsoleErrors = PR_TRUE;                 \
    return ret;                                  \
  }                                              \
  PR_END_MACRO

#define NS_ENSURE_SUCCESS_LOG(res, ret)          \
  NS_ENSURE_TRUE_LOG(NS_SUCCEEDED(res), ret)

/**
 * Because we're starting/stopping XPCOM several times in different scenarios,
 * this class is a stack-based critter that makes sure that XPCOM is shut down
 * during early returns.
 */

class ScopedXPCOMStartup
{
public:
  ScopedXPCOMStartup() :
    mServiceManager(nsnull) { }
  ~ScopedXPCOMStartup();

  nsresult Initialize();
  nsresult DoAutoreg();
  nsresult RegisterProfileService();
  nsresult SetWindowCreator(nsINativeAppSupport* native);

private:
  nsIServiceManager* mServiceManager;
};

ScopedXPCOMStartup::~ScopedXPCOMStartup()
{
  if (mServiceManager) {
    nsCOMPtr<nsIAppStartup> appStartup (do_GetService(NS_APPSTARTUP_CONTRACTID));
    if (appStartup)
      appStartup->DestroyHiddenWindow();

    gDirServiceProvider->DoShutdown();

    WriteConsoleLog();

    NS_ShutdownXPCOM(mServiceManager);
    mServiceManager = nsnull;
  }
}

// {95d89e3e-a169-41a3-8e56-719978e15b12}
#define APPINFO_CID \
  { 0x95d89e3e, 0xa169, 0x41a3, { 0x8e, 0x56, 0x71, 0x99, 0x78, 0xe1, 0x5b, 0x12 } }

static nsModuleComponentInfo kComponents[] =
{
  {
    "nsXULAppInfo",
    APPINFO_CID,
    XULAPPINFO_SERVICE_CONTRACTID,
    AppInfoConstructor
  }
#ifdef MOZ_CRASHREPORTER
,
  {
    "nsXULAppInfo",
    APPINFO_CID,
    NS_CRASHREPORTER_CONTRACTID,
    AppInfoConstructor
  }
#endif
};

NS_IMPL_NSGETMODULE(Apprunner, kComponents)

#if !defined(_BUILD_STATIC_BIN) && !defined(MOZ_ENABLE_LIBXUL)
static nsStaticModuleInfo const kXREStaticModules[] =
{
  {
    "Apprunner",
    Apprunner_NSGetModule
  }
};

nsStaticModuleInfo const *const kPStaticModules = kXREStaticModules;
PRUint32 const kStaticModuleCount = NS_ARRAY_LENGTH(kXREStaticModules);
#endif

nsresult
ScopedXPCOMStartup::Initialize()
{
  NS_ASSERTION(gDirServiceProvider, "Should not get here!");

  nsresult rv;
  rv = NS_InitXPCOM3(&mServiceManager, gDirServiceProvider->GetAppDir(),
                     gDirServiceProvider,
                     kPStaticModules, kStaticModuleCount);
  if (NS_FAILED(rv)) {
    NS_ERROR("Couldn't start xpcom!");
    mServiceManager = nsnull;
  }
  else {
    nsCOMPtr<nsIComponentRegistrar> reg =
      do_QueryInterface(mServiceManager);
    NS_ASSERTION(reg, "Service Manager doesn't QI to Registrar.");
  }

  return rv;
}

// {0C4A446C-EE82-41f2-8D04-D366D2C7A7D4}
static const nsCID kNativeAppSupportCID =
  { 0xc4a446c, 0xee82, 0x41f2, { 0x8d, 0x4, 0xd3, 0x66, 0xd2, 0xc7, 0xa7, 0xd4 } };

// {5F5E59CE-27BC-47eb-9D1F-B09CA9049836}
static const nsCID kProfileServiceCID =
  { 0x5f5e59ce, 0x27bc, 0x47eb, { 0x9d, 0x1f, 0xb0, 0x9c, 0xa9, 0x4, 0x98, 0x36 } };

nsresult
ScopedXPCOMStartup::RegisterProfileService()
{
  NS_ASSERTION(mServiceManager, "Not initialized!");

  nsCOMPtr<nsIFactory> factory;
  NS_NewToolkitProfileFactory(getter_AddRefs(factory));
  if (!factory) return NS_ERROR_OUT_OF_MEMORY;

  nsCOMPtr<nsIComponentRegistrar> reg (do_QueryInterface(mServiceManager));
  if (!reg) return NS_ERROR_NO_INTERFACE;

  return reg->RegisterFactory(kProfileServiceCID,
                              "Toolkit Profile Service",
                              NS_PROFILESERVICE_CONTRACTID,
                              factory);
}

nsresult
ScopedXPCOMStartup::DoAutoreg()
{
#ifdef DEBUG
  // _Always_ autoreg if we're in a debug build, under the assumption
  // that people are busily modifying components and will be angry if
  // their changes aren't noticed.
  nsCOMPtr<nsIComponentRegistrar> registrar
    (do_QueryInterface(mServiceManager));
  NS_ASSERTION(registrar, "Where's the component registrar?");

  registrar->AutoRegister(nsnull);
#endif

  return NS_OK;
}

/**
 * This is a little factory class that serves as a singleton-service-factory
 * for the nativeappsupport object.
 */
class nsSingletonFactory : public nsIFactory
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIFACTORY

  nsSingletonFactory(nsISupports* aSingleton);
  ~nsSingletonFactory() { }

private:
  nsCOMPtr<nsISupports> mSingleton;
};

nsSingletonFactory::nsSingletonFactory(nsISupports* aSingleton)
  : mSingleton(aSingleton)
{
  NS_ASSERTION(mSingleton, "Singleton was null!");
}

NS_IMPL_ISUPPORTS1(nsSingletonFactory, nsIFactory)

NS_IMETHODIMP
nsSingletonFactory::CreateInstance(nsISupports* aOuter,
                                   const nsIID& aIID,
                                   void* *aResult)
{
  NS_ENSURE_NO_AGGREGATION(aOuter);

  return mSingleton->QueryInterface(aIID, aResult);
}

NS_IMETHODIMP
nsSingletonFactory::LockFactory(PRBool)
{
  return NS_OK;
}

/**
 * Set our windowcreator on the WindowWatcher service.
 */
nsresult
ScopedXPCOMStartup::SetWindowCreator(nsINativeAppSupport* native)
{
  nsresult rv;

  nsCOMPtr<nsIComponentRegistrar> registrar
    (do_QueryInterface(mServiceManager));
  NS_ASSERTION(registrar, "Where's the component registrar?");

  nsCOMPtr<nsIFactory> nativeFactory = new nsSingletonFactory(native);
  NS_ENSURE_TRUE(nativeFactory, NS_ERROR_OUT_OF_MEMORY);

  rv = registrar->RegisterFactory(kNativeAppSupportCID,
                                  "Native App Support",
                                  NS_NATIVEAPPSUPPORT_CONTRACTID,
                                  nativeFactory);
  NS_ENSURE_SUCCESS(rv, rv);

  // Inform the chrome registry about OS accessibility
  nsCOMPtr<nsIToolkitChromeRegistry> cr (do_GetService(NS_CHROMEREGISTRY_CONTRACTID));
  if (cr)
    cr->CheckForOSAccessibility();

  nsCOMPtr<nsIWindowCreator> creator (do_GetService(NS_APPSTARTUP_CONTRACTID));
  if (!creator) return NS_ERROR_UNEXPECTED;

  nsCOMPtr<nsIWindowWatcher> wwatch
    (do_GetService(NS_WINDOWWATCHER_CONTRACTID, &rv));
  NS_ENSURE_SUCCESS(rv, rv);

  return wwatch->SetWindowCreator(creator);
}

/**
 * A helper class which calls NS_LogInit/NS_LogTerm in its scope.
 */
class ScopedLogging
{
public:
  ScopedLogging() { NS_LogInit(); }
  ~ScopedLogging() { NS_LogTerm(); }
};

static void DumpArbitraryHelp()
{
  nsresult rv;

  ScopedLogging log;

  {
    nsXREDirProvider dirProvider;
    dirProvider.Initialize(nsnull, gAppData->xreDirectory);

    ScopedXPCOMStartup xpcom;
    xpcom.Initialize();
    xpcom.DoAutoreg();

    nsCOMPtr<nsICommandLineRunner> cmdline
      (do_CreateInstance("@mozilla.org/toolkit/command-line;1"));
    if (!cmdline)
      return;

    nsCString text;
    rv = cmdline->GetHelpText(text);
    if (NS_SUCCEEDED(rv))
      printf("%s", text.get());
  }
}

// English text needs to go into a dtd file.
// But when this is called we have no components etc. These strings must either be
// here, or in a native resource file.
static void
DumpHelp()
{
  printf("Usage: %s [ options ... ] [URL]\n"
         "       where options include:\n\n", gArgv[0]);

#ifdef MOZ_X11
  printf("X11 options\n"
         "\t--display=DISPLAY\t\tX display to use\n"
         "\t--sync\t\tMake X calls synchronous\n"
         "\t--no-xshm\t\tDon't use X shared memory extension\n"
         "\t--xim-preedit=STYLE\n"
         "\t--xim-status=STYLE\n");
#endif
#ifdef XP_UNIX
  printf("\t--g-fatal-warnings\t\tMake all warnings fatal\n"
         "\nMozilla options\n");
#endif

  printf("\t-height <value>\t\tSet height of startup window to <value>.\n"
         "\t-h or -help\t\tPrint this message.\n"
         "\t-width <value>\t\tSet width of startup window to <value>.\n"
         "\t-v or -version\t\tPrint %s version.\n"
         "\t-P <profile>\t\tStart with <profile>.\n"
         "\t-ProfileManager\t\tStart with ProfileManager.\n"
         "\t-no-remote\t\tOpen new instance, not a new window in running instance.\n"
         "\t-UILocale <locale>\t\tStart with <locale> resources as UI Locale.\n"
         "\t-safe-mode\t\tDisables extensions and themes for this session.\n", gAppData->name);

#if defined(XP_WIN) || defined(XP_OS2)
  printf("\t-console\t\tStart %s with a debugging console.\n", gAppData->name);
#endif

  // this works, but only after the components have registered.  so if you drop in a new command line handler, -help
  // won't not until the second run.
  // out of the bug, because we ship a component.reg file, it works correctly.
  DumpArbitraryHelp();
}

#ifdef DEBUG_warren
#ifdef XP_WIN
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#endif

#if defined(FREEBSD)
// pick up fpsetmask prototype.
#include <ieeefp.h>
#endif

static inline void
DumpVersion()
{
  printf("%s %s %s", 
         gAppData->vendor ? gAppData->vendor : "", gAppData->name, gAppData->version);
  if (gAppData->copyright)
      printf(", %s", gAppData->copyright);
  printf("\n");
}

#ifdef MOZ_ENABLE_XREMOTE
// use int here instead of a PR type since it will be returned
// from main - just to keep types consistent
static int
HandleRemoteArgument(const char* remote, const char* aDesktopStartupID)
{
  nsresult rv;
  ArgResult ar;

  const char *profile = 0;
  nsCAutoString program(gAppData->name);
  ToLowerCase(program);
  const char *username = getenv("LOGNAME");

  ar = CheckArg("p", PR_FALSE, &profile);
  if (ar == ARG_BAD) {
    PR_fprintf(PR_STDERR, "Error: argument -p requires a profile name\n");
    return 1;
  }

  const char *temp = nsnull;
  ar = CheckArg("a", PR_FALSE, &temp);
  if (ar == ARG_BAD) {
    PR_fprintf(PR_STDERR, "Error: argument -a requires an application name\n");
    return 1;
  } else if (ar == ARG_FOUND) {
    program.Assign(temp);
  }

  ar = CheckArg("u", PR_FALSE, &username);
  if (ar == ARG_BAD) {
    PR_fprintf(PR_STDERR, "Error: argument -u requires a username\n");
    return 1;
  }

  XRemoteClient client;
  rv = client.Init();
  if (NS_FAILED(rv)) {
    PR_fprintf(PR_STDERR, "Error: Failed to connect to X server.\n");
    return 1;
  }

  nsXPIDLCString response;
  PRBool success = PR_FALSE;
  rv = client.SendCommand(program.get(), username, profile, remote,
                          aDesktopStartupID, getter_Copies(response), &success);
  // did the command fail?
  if (NS_FAILED(rv)) {
    PR_fprintf(PR_STDERR, "Error: Failed to send command: %s\n",
               response ? response.get() : "No response included");
    return 1;
  }

  if (!success) {
    PR_fprintf(PR_STDERR, "Error: No running window found\n");
    return 2;
  }

  return 0;
}

static RemoteResult
RemoteCommandLine(const char* aDesktopStartupID)
{
  nsresult rv;
  ArgResult ar;

  nsCAutoString program(gAppData->name);
  ToLowerCase(program);
  const char *username = getenv("LOGNAME");

  const char *temp = nsnull;
  ar = CheckArg("a", PR_TRUE, &temp);
  if (ar == ARG_BAD) {
    PR_fprintf(PR_STDERR, "Error: argument -a requires an application name\n");
    return REMOTE_ARG_BAD;
  } else if (ar == ARG_FOUND) {
    program.Assign(temp);
  }

  ar = CheckArg("u", PR_TRUE, &username);
  if (ar == ARG_BAD) {
    PR_fprintf(PR_STDERR, "Error: argument -u requires a username\n");
    return REMOTE_ARG_BAD;
  }

  XRemoteClient client;
  rv = client.Init();
  if (NS_FAILED(rv))
    return REMOTE_NOT_FOUND;
 
  nsXPIDLCString response;
  PRBool success = PR_FALSE;
  rv = client.SendCommandLine(program.get(), username, nsnull,
                              gArgc, gArgv, aDesktopStartupID,
                              getter_Copies(response), &success);
  // did the command fail?
  if (NS_FAILED(rv) || !success)
    return REMOTE_NOT_FOUND;

  return REMOTE_FOUND;
}
#endif // MOZ_ENABLE_XREMOTE

#ifdef XP_MACOSX
static char const *gBinaryPath;
#endif

nsresult
XRE_GetBinaryPath(const char* argv0, nsILocalFile* *aResult)
{
  nsresult rv;
  nsCOMPtr<nsILocalFile> lf;

  // We need to use platform-specific hackery to find the
  // path of this executable. This is copied, with some modifications, from
  // nsGREDirServiceProvider.cpp

#ifdef XP_WIN
  PRUnichar exePath[MAXPATHLEN];

  if (!::GetModuleFileNameW(0, exePath, MAXPATHLEN))
    return NS_ERROR_FAILURE;

  rv = NS_NewLocalFile(nsDependentString(exePath), PR_TRUE,
                       getter_AddRefs(lf));
  if (NS_FAILED(rv))
    return rv;

#elif defined(XP_MACOSX)
  if (gBinaryPath)
    return NS_NewNativeLocalFile(nsDependentCString(gBinaryPath), PR_FALSE,
                                 aResult);

  NS_NewNativeLocalFile(EmptyCString(), PR_TRUE, getter_AddRefs(lf));
  nsCOMPtr<nsILocalFileMac> lfm (do_QueryInterface(lf));
  if (!lfm)
    return NS_ERROR_FAILURE;

  // Works even if we're not bundled.
  CFBundleRef appBundle = CFBundleGetMainBundle();
  if (!appBundle)
    return NS_ERROR_FAILURE;

  CFURLRef bundleURL = CFBundleCopyExecutableURL(appBundle);
  if (!bundleURL)
    return NS_ERROR_FAILURE;

  FSRef fileRef;
  if (!CFURLGetFSRef(bundleURL, &fileRef)) {
    CFRelease(bundleURL);
    return NS_ERROR_FAILURE;
  }

  rv = lfm->InitWithFSRef(&fileRef);
  CFRelease(bundleURL);

  if (NS_FAILED(rv))
    return rv;

#elif defined(XP_UNIX)
  struct stat fileStat;
  char exePath[MAXPATHLEN];
  char tmpPath[MAXPATHLEN];

  rv = NS_ERROR_FAILURE;

  // on unix, there is no official way to get the path of the current binary.
  // instead of using the MOZILLA_FIVE_HOME hack, which doesn't scale to
  // multiple applications, we will try a series of techniques:
  //
  // 1) look for /proc/<pid>/exe which is a symlink to the executable on newer
  //    Linux kernels
  // 2) use realpath() on argv[0], which works unless we're loaded from the
  //    PATH
  // 3) manually walk through the PATH and look for ourself
  // 4) give up

// #ifdef __linux__
#if 0
  int r = readlink("/proc/self/exe", exePath, MAXPATHLEN);

  // apparently, /proc/self/exe can sometimes return weird data... check it
  if (r > 0 && r < MAXPATHLEN && stat(exePath, &fileStat) == 0) {
    rv = NS_OK;
  }

#endif
  if (NS_FAILED(rv) &&
      realpath(argv0, exePath) && stat(exePath, &fileStat) == 0) {
    rv = NS_OK;
  }

  if (NS_FAILED(rv)) {
    const char *path = getenv("PATH");
    if (!path)
      return NS_ERROR_FAILURE;

    char *pathdup = strdup(path);
    if (!pathdup)
      return NS_ERROR_OUT_OF_MEMORY;

    PRBool found = PR_FALSE;
    char *newStr = pathdup;
    char *token;
    while ( (token = nsCRT::strtok(newStr, ":", &newStr)) ) {
      sprintf(tmpPath, "%s/%s", token, argv0);
      if (realpath(tmpPath, exePath) && stat(exePath, &fileStat) == 0) {
        found = PR_TRUE;
        break;
      }
    }
    free(pathdup);
    if (!found)
      return NS_ERROR_FAILURE;
  }

  rv = NS_NewNativeLocalFile(nsDependentCString(exePath), PR_TRUE,
                             getter_AddRefs(lf));
  if (NS_FAILED(rv))
    return rv;

#elif defined(XP_OS2)
  PPIB ppib;
  PTIB ptib;
  char exePath[MAXPATHLEN];

  DosGetInfoBlocks( &ptib, &ppib);
  DosQueryModuleName( ppib->pib_hmte, MAXPATHLEN, exePath);
  rv = NS_NewNativeLocalFile(nsDependentCString(exePath), PR_TRUE,
                             getter_AddRefs(lf));
  if (NS_FAILED(rv))
    return rv;

#elif defined(XP_BEOS)
  int32 cookie = 0;
  image_info info;

  if(get_next_image_info(0, &cookie, &info) != B_OK)
    return NS_ERROR_FAILURE;

  rv = NS_NewNativeLocalFile(nsDependentCString(info.name), PR_TRUE,
                             getter_AddRefs(lf));
  if (NS_FAILED(rv))
    return rv;

#elif
#error Oops, you need platform-specific code here
#endif

  NS_ADDREF(*aResult = lf);
  return NS_OK;
}

#define NS_ERROR_LAUNCHED_CHILD_PROCESS NS_ERROR_GENERATE_FAILURE(NS_ERROR_MODULE_PROFILE, 200)

#ifdef XP_WIN
#include "nsWindowsRestart.cpp"
#endif

#if defined(XP_OS2) && (__GNUC__ == 3 && __GNUC_MINOR__ == 3) // broken OS/2 GCC
// Copy the environment maintained by the C library into an ASCIIZ array
// that can be used to pass it on to the OS/2 Dos* APIs (which otherwise
// don't know anything about the stuff set by PR_SetEnv() or setenv()).
char *createEnv()
{
  // just allocate the maximum amount (24 kB = 0x60000 bytes), to be able to
  // copy the existing environment
  char *env = (char *)calloc(0x6000, sizeof(char));
  if (!env) {
    return NULL;
  }

  // walk along the environ string array of the C library and copy
  // everything (that fits) into the output environment array, leaving
  // null bytes between the entries
  char *penv = env; // movable pointer to result environment ASCIIZ array
  int i = 0, space = 0x6000;
  while (environ[i] && environ[i][0]) {
    int len = strlen(environ[i]);
    if (space - len <= 0) {
      break;
    }
    strcpy(penv, environ[i]);
    i++; // next environment variable
    penv += len + 1; // jump to after next null byte
    space -= len - 1; // subtract consumed length from usable space
  }

  return env;
}

// OS2LaunchChild() is there to replace _execv() which is broken in the C
// runtime library that comes with GCC 3.3.5 on OS/2. It uses createEnv()
// to copy the process environment and add necessary variables
//
// returns -1 on failure and 0 on success
int OS2LaunchChild(const char *aExePath, int aArgc, char **aArgv)
{
  // find total length of aArgv
  int len = 0;
  for (int i = 0; i < aArgc; i++) {
    len += strlen(aArgv[i]) + 1; // plus space in between
  }
  len++; // leave space for null byte at end
  // allocate enough space for all strings and nulls,
  // calloc helpfully initializes to null
  char *args = (char *)calloc(len, sizeof(char));
  if (!args) {
    return -1;
  }
  char *pargs = args; // extra pointer to after the last argument
  // build argument list in the format the DosStartSession() wants,
  // adding spaces between the arguments
  for (int i = 0; i < aArgc; i++, *pargs++ = ' ') {
    strcpy(pargs, aArgv[i]);
    pargs += strlen(aArgv[i]);
  }
  if (aArgc > 1) {
    *(pargs-1) = '\0'; // replace last space
  }
  *pargs = '\0';
  // make sure that the program is separated by null byte
  pargs = strchr(args, ' ');
  if (pargs) {
    *pargs = '\0';
  }

  char *env = createEnv();

  char error[CCHMAXPATH] = { 0 };
  RESULTCODES crc = { 0 };
  ULONG rc = DosExecPgm(error, sizeof(error), EXEC_ASYNC, args, env,
                        &crc, (PSZ)aExePath);
  free(args); // done with the arguments
  if (env) {
    free(env);
  }
  if (rc != NO_ERROR) {
    return -1;
  }

  return 0;
}
#endif

// If aBlankCommandLine is true, then the application will be launched with a
// blank command line instead of being launched with the same command line that
// it was initially started with.
static nsresult LaunchChild(nsINativeAppSupport* aNative,
                            PRBool aBlankCommandLine = PR_FALSE,
                            int needElevation = 0)
{
  aNative->Quit(); // release DDE mutex, if we're holding it

  // Restart this process by exec'ing it into the current process
  // if supported by the platform.  Otherwise, use NSPR.
 
  if (aBlankCommandLine) {
    gRestartArgc = 1;
    gRestartArgv[gRestartArgc] = nsnull;
  }

  PR_SetEnv("MOZ_LAUNCHED_CHILD=1");

#if defined(XP_MACOSX)
  LaunchChildMac(gRestartArgc, gRestartArgv);
#else
  nsCOMPtr<nsILocalFile> lf;
  nsresult rv = XRE_GetBinaryPath(gArgv[0], getter_AddRefs(lf));
  if (NS_FAILED(rv))
    return rv;

#if defined(XP_WIN)
  nsAutoString exePath;
  rv = lf->GetPath(exePath);
  if (NS_FAILED(rv))
    return rv;

  if (!WinLaunchChild(exePath.get(), gRestartArgc, gRestartArgv, needElevation))
    return NS_ERROR_FAILURE;

#else
  nsCAutoString exePath;
  rv = lf->GetNativePath(exePath);
  if (NS_FAILED(rv))
    return rv;

#if defined(XP_OS2) && (__GNUC__ == 3 && __GNUC_MINOR__ == 3)
  // implementation of _execv() is broken with GCC 3.3.x on OS/2
  if (OS2LaunchChild(exePath.get(), gRestartArgc, gRestartArgv) == -1)
    return NS_ERROR_FAILURE;
#elif defined(XP_OS2)
  if (_execv(exePath.get(), gRestartArgv) == -1)
    return NS_ERROR_FAILURE;
#elif defined(XP_UNIX)
  if (execv(exePath.get(), gRestartArgv) == -1)
    return NS_ERROR_FAILURE;
#elif defined(XP_BEOS)
  extern char **environ;
  status_t res;
  res = resume_thread(load_image(gRestartArgc,(const char **)gRestartArgv,(const char **)environ));
  if (res != B_OK)
    return NS_ERROR_FAILURE;
#else
  PRProcess* process = PR_CreateProcess(exePath.get(), gRestartArgv,
                                        nsnull, nsnull);
  if (!process) return NS_ERROR_FAILURE;

  PRInt32 exitCode;
  PRStatus failed = PR_WaitProcess(process, &exitCode);
  if (failed || exitCode)
    return NS_ERROR_FAILURE;
#endif // XP_OS2 series
#endif // WP_WIN
#endif // WP_MACOSX

  return NS_ERROR_LAUNCHED_CHILD_PROCESS;
}

static const char kProfileProperties[] =
  "chrome://mozapps/locale/profile/profileSelection.properties";

static nsresult
ProfileLockedDialog(nsILocalFile* aProfileDir, nsILocalFile* aProfileLocalDir,
                    nsIProfileUnlocker* aUnlocker,
                    nsINativeAppSupport* aNative, nsIProfileLock* *aResult)
{
  nsresult rv;

  ScopedXPCOMStartup xpcom;
  rv = xpcom.Initialize();
  NS_ENSURE_SUCCESS(rv, rv);

  rv = xpcom.DoAutoreg();
  rv |= xpcom.SetWindowCreator(aNative);
  NS_ENSURE_SUCCESS(rv, NS_ERROR_FAILURE);

  { //extra scoping is needed so we release these components before xpcom shutdown
    nsCOMPtr<nsIStringBundleService> sbs
      (do_GetService(NS_STRINGBUNDLE_CONTRACTID));
    NS_ENSURE_TRUE(sbs, NS_ERROR_FAILURE);

    nsCOMPtr<nsIStringBundle> sb;
    sbs->CreateBundle(kProfileProperties, getter_AddRefs(sb));
    NS_ENSURE_TRUE_LOG(sbs, NS_ERROR_FAILURE);

    NS_ConvertUTF8toUTF16 appName(gAppData->name);
    const PRUnichar* params[] = {appName.get(), appName.get()};

    nsXPIDLString killMessage;
#ifndef XP_MACOSX
    static const PRUnichar kRestartNoUnlocker[] = {'r','e','s','t','a','r','t','M','e','s','s','a','g','e','N','o','U','n','l','o','c','k','e','r','\0'}; // "restartMessageNoUnlocker"
    static const PRUnichar kRestartUnlocker[] = {'r','e','s','t','a','r','t','M','e','s','s','a','g','e','U','n','l','o','c','k','e','r','\0'}; // "restartMessageUnlocker"
#else
    static const PRUnichar kRestartNoUnlocker[] = {'r','e','s','t','a','r','t','M','e','s','s','a','g','e','N','o','U','n','l','o','c','k','e','r','M','a','c','\0'}; // "restartMessageNoUnlockerMac"
    static const PRUnichar kRestartUnlocker[] = {'r','e','s','t','a','r','t','M','e','s','s','a','g','e','U','n','l','o','c','k','e','r','M','a','c','\0'}; // "restartMessageUnlockerMac"
#endif

    sb->FormatStringFromName(aUnlocker ? kRestartUnlocker : kRestartNoUnlocker,
                             params, 2, getter_Copies(killMessage));

    nsXPIDLString killTitle;
    sb->FormatStringFromName(NS_LITERAL_STRING("restartTitle").get(),
                             params, 1, getter_Copies(killTitle));

    if (!killMessage || !killTitle)
      return NS_ERROR_FAILURE;

    nsCOMPtr<nsIPromptService> ps
      (do_GetService(NS_PROMPTSERVICE_CONTRACTID));
    NS_ENSURE_TRUE(ps, NS_ERROR_FAILURE);

    PRUint32 flags = nsIPromptService::BUTTON_TITLE_OK * nsIPromptService::BUTTON_POS_0;

    if (aUnlocker) {
      flags =
        nsIPromptService::BUTTON_TITLE_CANCEL * nsIPromptService::BUTTON_POS_0 +
        nsIPromptService::BUTTON_TITLE_IS_STRING * nsIPromptService::BUTTON_POS_1 +
        nsIPromptService::BUTTON_POS_1_DEFAULT;
    }

    PRInt32 button;
    rv = ps->ConfirmEx(nsnull, killTitle, killMessage, flags,
                       killTitle, nsnull, nsnull, nsnull, nsnull, &button);
    NS_ENSURE_SUCCESS_LOG(rv, rv);

    if (button == 1 && aUnlocker) {
      rv = aUnlocker->Unlock(nsIProfileUnlocker::FORCE_QUIT);
      if (NS_FAILED(rv)) return rv;

      return NS_LockProfilePath(aProfileDir, aProfileLocalDir, nsnull, aResult);
    }

    return NS_ERROR_ABORT;
  }
}

static const char kProfileManagerURL[] =
  "chrome://mozapps/content/profile/profileSelection.xul";

static nsresult
ShowProfileManager(nsIToolkitProfileService* aProfileSvc,
                   nsINativeAppSupport* aNative)
{
  nsresult rv;

  nsCOMPtr<nsILocalFile> profD, profLD;
  PRUnichar* profileNamePtr;
  nsCAutoString profileName;

  {
    ScopedXPCOMStartup xpcom;
    rv = xpcom.Initialize();
    NS_ENSURE_SUCCESS(rv, rv);

    rv = xpcom.DoAutoreg();
    rv |= xpcom.RegisterProfileService();
    rv |= xpcom.SetWindowCreator(aNative);
    NS_ENSURE_SUCCESS(rv, NS_ERROR_FAILURE);

#ifdef XP_MACOSX
    SetupMacCommandLine(gRestartArgc, gRestartArgv);
#endif

#ifdef XP_WIN
    ProcessDDE(aNative);
#endif

    { //extra scoping is needed so we release these components before xpcom shutdown
      nsCOMPtr<nsIWindowWatcher> windowWatcher
        (do_GetService(NS_WINDOWWATCHER_CONTRACTID));
      nsCOMPtr<nsIDialogParamBlock> ioParamBlock
        (do_CreateInstance(NS_DIALOGPARAMBLOCK_CONTRACTID));
      nsCOMPtr<nsIMutableArray> dlgArray (do_CreateInstance(NS_ARRAY_CONTRACTID));
      NS_ENSURE_TRUE(windowWatcher && ioParamBlock && dlgArray, NS_ERROR_FAILURE);

      ioParamBlock->SetObjects(dlgArray);

      nsCOMPtr<nsIAppStartup> appStartup
        (do_GetService(NS_APPSTARTUP_CONTRACTID));
      NS_ENSURE_TRUE(appStartup, NS_ERROR_FAILURE);

      nsCOMPtr<nsIDOMWindow> newWindow;
      rv = windowWatcher->OpenWindow(nsnull,
                                     kProfileManagerURL,
                                     "_blank",
                                     "centerscreen,chrome,modal,titlebar",
                                     ioParamBlock,
                                     getter_AddRefs(newWindow));

      NS_ENSURE_SUCCESS_LOG(rv, rv);

      aProfileSvc->Flush();

      PRInt32 dialogConfirmed;
      rv = ioParamBlock->GetInt(0, &dialogConfirmed);
      if (NS_FAILED(rv) || dialogConfirmed == 0) return NS_ERROR_ABORT;

      nsCOMPtr<nsIProfileLock> lock;
      rv = dlgArray->QueryElementAt(0, NS_GET_IID(nsIProfileLock),
                                    getter_AddRefs(lock));
      NS_ENSURE_SUCCESS_LOG(rv, rv);

      rv = lock->GetDirectory(getter_AddRefs(profD));
      NS_ENSURE_SUCCESS(rv, rv);

      rv = lock->GetLocalDirectory(getter_AddRefs(profLD));
      NS_ENSURE_SUCCESS(rv, rv);

      rv = ioParamBlock->GetString(0, &profileNamePtr);
      NS_ENSURE_SUCCESS(rv, rv);

      CopyUTF16toUTF8(profileNamePtr, profileName);
      NS_Free(profileNamePtr);

      lock->Unlock();
    }
  }

  SaveFileToEnv("XRE_PROFILE_PATH", profD);
  SaveFileToEnv("XRE_PROFILE_LOCAL_PATH", profLD);
  SaveWordToEnv("XRE_PROFILE_NAME", profileName);

  PRBool offline = PR_FALSE;
  aProfileSvc->GetStartOffline(&offline);
  if (offline) {
    PR_SetEnv("XRE_START_OFFLINE=1");
  }

  return LaunchChild(aNative);
}

static nsresult
ImportProfiles(nsIToolkitProfileService* aPService,
               nsINativeAppSupport* aNative)
{
  nsresult rv;

  PR_SetEnv("XRE_IMPORT_PROFILES=1");

  // try to import old-style profiles
  { // scope XPCOM
    ScopedXPCOMStartup xpcom;
    rv = xpcom.Initialize();
    if (NS_SUCCEEDED(rv)) {
      xpcom.DoAutoreg();
      xpcom.RegisterProfileService();

#ifdef XP_MACOSX
      SetupMacCommandLine(gRestartArgc, gRestartArgv);
#endif

      nsCOMPtr<nsIProfileMigrator> migrator
        (do_GetService(NS_PROFILEMIGRATOR_CONTRACTID));
      if (migrator) {
        migrator->Import();
      }
    }
  }

  aPService->Flush();
  return LaunchChild(aNative);
}

// Pick a profile. We need to end up with a profile lock.
//
// 1) check for -profile <path>
// 2) check for -P <name>
// 3) check for -ProfileManager
// 4) use the default profile, if there is one
// 5) if there are *no* profiles, set up profile-migration
// 6) display the profile-manager UI

static PRBool gDoMigration = PR_FALSE;

static nsresult
SelectProfile(nsIProfileLock* *aResult, nsINativeAppSupport* aNative,
              PRBool* aStartOffline, nsACString* aProfileName)
{
  nsresult rv;
  ArgResult ar;
  const char* arg;
  *aResult = nsnull;
  *aStartOffline = PR_FALSE;

  ar = CheckArg("offline", PR_TRUE);
  if (ar == ARG_BAD) {
    PR_fprintf(PR_STDERR, "Error: argument -offline is invalid when argument -osint is specified\n");
    return NS_ERROR_FAILURE;
  }

  arg = PR_GetEnv("XRE_START_OFFLINE");
  if ((arg && *arg) || ar)
    *aStartOffline = PR_TRUE;


  nsCOMPtr<nsILocalFile> lf = GetFileFromEnv("XRE_PROFILE_PATH");
  if (lf) {
    nsCOMPtr<nsILocalFile> localDir =
      GetFileFromEnv("XRE_PROFILE_LOCAL_PATH");
    if (!localDir) {
      localDir = lf;
    }

    arg = PR_GetEnv("XRE_PROFILE_NAME");
    if (arg && *arg && aProfileName)
      aProfileName->Assign(nsDependentCString(arg));

    // Clear out flags that we handled (or should have handled!) last startup.
    const char *dummy;
    CheckArg("p", PR_FALSE, &dummy);
    CheckArg("profile", PR_FALSE, &dummy);
    CheckArg("profilemanager");

    return NS_LockProfilePath(lf, localDir, nsnull, aResult);
  }

  ar = CheckArg("migration", PR_TRUE);
  if (ar == ARG_BAD) {
    PR_fprintf(PR_STDERR, "Error: argument -migration is invalid when argument -osint is specified\n");
    return NS_ERROR_FAILURE;
  } else if (ar == ARG_FOUND) {
    gDoMigration = PR_TRUE;
  }

  ar = CheckArg("profile", PR_TRUE, &arg);
  if (ar == ARG_BAD) {
    PR_fprintf(PR_STDERR, "Error: argument -profile requires a path\n");
    return NS_ERROR_FAILURE;
  }
  if (ar) {
    nsCOMPtr<nsILocalFile> lf;
    rv = XRE_GetFileFromPath(arg, getter_AddRefs(lf));
    NS_ENSURE_SUCCESS(rv, rv);

    nsCOMPtr<nsIProfileUnlocker> unlocker;

    // If a profile path is specified directory on the command line, then
    // assume that the temp directory is the same as the given directory.
    rv = NS_LockProfilePath(lf, lf, getter_AddRefs(unlocker), aResult);
    if (NS_SUCCEEDED(rv))
      return rv;

    return ProfileLockedDialog(lf, lf, unlocker, aNative, aResult);
  }

  nsCOMPtr<nsIToolkitProfileService> profileSvc;
  rv = NS_NewToolkitProfileService(getter_AddRefs(profileSvc));
  NS_ENSURE_SUCCESS(rv, rv);

  ar = CheckArg("createprofile", PR_TRUE, &arg);
  if (ar == ARG_BAD) {
    PR_fprintf(PR_STDERR, "Error: argument -createprofile requires a profile name\n");
    return NS_ERROR_FAILURE;
  }
  if (ar) {
    nsCOMPtr<nsIToolkitProfile> profile;

    const char* delim = strchr(arg, ' ');
    if (delim) {
      nsCOMPtr<nsILocalFile> lf;
      rv = NS_NewNativeLocalFile(nsDependentCString(delim + 1),
                                   PR_TRUE, getter_AddRefs(lf));
      if (NS_FAILED(rv)) {
        PR_fprintf(PR_STDERR, "Error: profile path not valid.\n");
        return rv;
      }
      
      // As with -profile, assume that the given path will be used for both the
      // main profile directory and the temp profile directory.
      rv = profileSvc->CreateProfile(lf, lf, nsDependentCSubstring(arg, delim),
                                     getter_AddRefs(profile));
    } else {
      rv = profileSvc->CreateProfile(nsnull, nsnull, nsDependentCString(arg),
                                     getter_AddRefs(profile));
    }
    // Some pathological arguments can make it this far
    if (NS_FAILED(rv)) {
      PR_fprintf(PR_STDERR, "Error creating profile.\n");
      return rv; 
    }
    rv = NS_ERROR_ABORT;  
    profileSvc->Flush();

    // XXXben need to ensure prefs.js exists here so the tinderboxes will
    //        not go orange.
    nsCOMPtr<nsILocalFile> prefsJSFile;
    profile->GetRootDir(getter_AddRefs(prefsJSFile));
    prefsJSFile->AppendNative(NS_LITERAL_CSTRING("prefs.js"));
    nsCAutoString pathStr;
    prefsJSFile->GetNativePath(pathStr);
    PR_fprintf(PR_STDERR, "Success: created profile '%s' at '%s'\n", arg, pathStr.get());
    PRBool exists;
    prefsJSFile->Exists(&exists);
    if (!exists)
      prefsJSFile->Create(nsIFile::NORMAL_FILE_TYPE, 0644);
    // XXXdarin perhaps 0600 would be better?

    return rv;
  }

  PRUint32 count;
  rv = profileSvc->GetProfileCount(&count);
  NS_ENSURE_SUCCESS(rv, rv);

  if (gAppData->flags & NS_XRE_ENABLE_PROFILE_MIGRATOR) {
    arg = PR_GetEnv("XRE_IMPORT_PROFILES");
    if (!count && (!arg || !*arg)) {
      return ImportProfiles(profileSvc, aNative);
    }
  }

  ar = CheckArg("p", PR_FALSE, &arg);
  if (ar == ARG_BAD) {
    ar = CheckArg("osint");
    if (ar == ARG_FOUND) {
      PR_fprintf(PR_STDERR, "Error: argument -p is invalid when argument -osint is specified\n");
      return NS_ERROR_FAILURE;
    }
    return ShowProfileManager(profileSvc, aNative);
  }
  if (ar) {
    ar = CheckArg("osint");
    if (ar == ARG_FOUND) {
      PR_fprintf(PR_STDERR, "Error: argument -p is invalid when argument -osint is specified\n");
      return NS_ERROR_FAILURE;
    }
    nsCOMPtr<nsIToolkitProfile> profile;
    rv = profileSvc->GetProfileByName(nsDependentCString(arg),
                                      getter_AddRefs(profile));
    if (NS_SUCCEEDED(rv)) {
      nsCOMPtr<nsIProfileUnlocker> unlocker;
      rv = profile->Lock(nsnull, aResult);
      if (NS_SUCCEEDED(rv)) {
        if (aProfileName)
          aProfileName->Assign(nsDependentCString(arg));
        return NS_OK;
      }

      nsCOMPtr<nsILocalFile> profileDir;
      rv = profile->GetRootDir(getter_AddRefs(profileDir));
      NS_ENSURE_SUCCESS(rv, rv);

      nsCOMPtr<nsILocalFile> profileLocalDir;
      rv = profile->GetLocalDir(getter_AddRefs(profileLocalDir));
      NS_ENSURE_SUCCESS(rv, rv);

      return ProfileLockedDialog(profileDir, profileLocalDir, unlocker,
                                 aNative, aResult);
    }

    return ShowProfileManager(profileSvc, aNative);
  }

  ar = CheckArg("profilemanager", PR_TRUE);
  if (ar == ARG_BAD) {
    PR_fprintf(PR_STDERR, "Error: argument -profilemanager is invalid when argument -osint is specified\n");
    return NS_ERROR_FAILURE;
  } else if (ar == ARG_FOUND) {
    return ShowProfileManager(profileSvc, aNative);
  }

  if (!count) {
    gDoMigration = PR_TRUE;

    // create a default profile
    nsCOMPtr<nsIToolkitProfile> profile;
    nsresult rv = profileSvc->CreateProfile(nsnull, // choose a default dir for us
                                            nsnull, // choose a default dir for us
                                            NS_LITERAL_CSTRING("default"),
                                            getter_AddRefs(profile));
    if (NS_SUCCEEDED(rv)) {
      profileSvc->Flush();
      rv = profile->Lock(nsnull, aResult);
      if (NS_SUCCEEDED(rv)) {
        if (aProfileName)
          aProfileName->Assign(NS_LITERAL_CSTRING("default"));
        return NS_OK;
      }
    }
  }

  PRBool useDefault = PR_TRUE;
  if (count > 1)
    profileSvc->GetStartWithLastProfile(&useDefault);

  if (useDefault) {
    nsCOMPtr<nsIToolkitProfile> profile;
    // GetSelectedProfile will auto-select the only profile if there's just one
    profileSvc->GetSelectedProfile(getter_AddRefs(profile));
    if (profile) {
      nsCOMPtr<nsIProfileUnlocker> unlocker;
      rv = profile->Lock(getter_AddRefs(unlocker), aResult);
      if (NS_SUCCEEDED(rv)) {
        // Try to grab the profile name.
        if (aProfileName) {
          rv = profile->GetName(*aProfileName);
          if (NS_FAILED(rv))
            aProfileName->Truncate(0);
        }
        return NS_OK;
      }

      nsCOMPtr<nsILocalFile> profileDir;
      rv = profile->GetRootDir(getter_AddRefs(profileDir));
      NS_ENSURE_SUCCESS(rv, rv);

      nsCOMPtr<nsILocalFile> profileLocalDir;
      rv = profile->GetRootDir(getter_AddRefs(profileLocalDir));
      NS_ENSURE_SUCCESS(rv, rv);

      return ProfileLockedDialog(profileDir, profileLocalDir, unlocker,
                                 aNative, aResult);
    }
  }

  return ShowProfileManager(profileSvc, aNative);
}

#define FILE_COMPATIBILITY_INFO NS_LITERAL_CSTRING("compatibility.ini")

static PRBool
CheckCompatibility(nsIFile* aProfileDir, const nsCString& aVersion,
                   const nsCString& aOSABI, nsIFile* aXULRunnerDir,
                   nsIFile* aAppDir)
{
  nsCOMPtr<nsIFile> file;
  aProfileDir->Clone(getter_AddRefs(file));
  if (!file)
    return PR_FALSE;
  file->AppendNative(FILE_COMPATIBILITY_INFO);

  nsINIParser parser;
  nsCOMPtr<nsILocalFile> localFile(do_QueryInterface(file));
  nsresult rv = parser.Init(localFile);
  if (NS_FAILED(rv))
    return PR_FALSE;

  nsCAutoString buf;
  rv = parser.GetString("Compatibility", "LastVersion", buf);
  if (NS_FAILED(rv) || !aVersion.Equals(buf))
    return PR_FALSE;

  rv = parser.GetString("Compatibility", "LastOSABI", buf);
  if (NS_FAILED(rv) || !aOSABI.Equals(buf))
    return PR_FALSE;

  rv = parser.GetString("Compatibility", "LastPlatformDir", buf);
  if (NS_FAILED(rv))
    return PR_FALSE;

  nsCOMPtr<nsILocalFile> lf;
  rv = NS_NewNativeLocalFile(buf, PR_FALSE,
                             getter_AddRefs(lf));
  if (NS_FAILED(rv))
    return PR_FALSE;

  PRBool eq;
  rv = lf->Equals(aXULRunnerDir, &eq);
  if (NS_FAILED(rv) || !eq)
    return PR_FALSE;

  if (aAppDir) {
    rv = parser.GetString("Compatibility", "LastAppDir", buf);
    if (NS_FAILED(rv))
      return PR_FALSE;

    rv = NS_NewNativeLocalFile(buf, PR_FALSE,
                               getter_AddRefs(lf));
    if (NS_FAILED(rv))
      return PR_FALSE;

    rv = lf->Equals(aAppDir, &eq);
    if (NS_FAILED(rv) || !eq)
      return PR_FALSE;
  }

  return PR_TRUE;
}

static void BuildVersion(nsCString &aBuf)
{
  aBuf.Assign(gAppData->version);
  aBuf.Append('_');
  aBuf.Append(gAppData->buildID);
  aBuf.Append('/');
  aBuf.Append(gToolkitBuildID);
}

static void
WriteVersion(nsIFile* aProfileDir, const nsCString& aVersion,
             const nsCString& aOSABI, nsIFile* aXULRunnerDir,
             nsIFile* aAppDir)
{
  nsCOMPtr<nsIFile> file;
  aProfileDir->Clone(getter_AddRefs(file));
  if (!file)
    return;
  file->AppendNative(FILE_COMPATIBILITY_INFO);

  nsCOMPtr<nsILocalFile> lf = do_QueryInterface(file);

  nsCAutoString platformDir;
  aXULRunnerDir->GetNativePath(platformDir);

  nsCAutoString appDir;
  if (aAppDir)
    aAppDir->GetNativePath(appDir);

  PRFileDesc *fd = nsnull;
  lf->OpenNSPRFileDesc(PR_WRONLY | PR_CREATE_FILE | PR_TRUNCATE, 0600, &fd);
  if (!fd) {
    NS_ERROR("could not create output stream");
    return;
  }

  static const char kHeader[] = "[Compatibility]" NS_LINEBREAK
                                "LastVersion=";

  PR_Write(fd, kHeader, sizeof(kHeader) - 1);
  PR_Write(fd, aVersion.get(), aVersion.Length());

  static const char kOSABIHeader[] = NS_LINEBREAK "LastOSABI=";
  PR_Write(fd, kOSABIHeader, sizeof(kOSABIHeader) - 1);
  PR_Write(fd, aOSABI.get(), aOSABI.Length());

  static const char kPlatformDirHeader[] = NS_LINEBREAK "LastPlatformDir=";

  PR_Write(fd, kPlatformDirHeader, sizeof(kPlatformDirHeader) - 1);
  PR_Write(fd, platformDir.get(), platformDir.Length());

  static const char kAppDirHeader[] = NS_LINEBREAK "LastAppDir=";
  if (aAppDir) {
    PR_Write(fd, kAppDirHeader, sizeof(kAppDirHeader) - 1);
    PR_Write(fd, appDir.get(), appDir.Length());
  }

  static const char kNL[] = NS_LINEBREAK;
  PR_Write(fd, kNL, sizeof(kNL) - 1);

  PR_Close(fd);
}

static PRBool ComponentsListChanged(nsIFile* aProfileDir)
{
  nsCOMPtr<nsIFile> file;
  aProfileDir->Clone(getter_AddRefs(file));
  if (!file)
    return PR_TRUE;
  file->AppendNative(NS_LITERAL_CSTRING(".autoreg"));

  PRBool exists = PR_FALSE;
  file->Exists(&exists);
  return exists;
}

static void RemoveComponentRegistries(nsIFile* aProfileDir, nsIFile* aLocalProfileDir,
                                      PRBool aRemoveEMFiles)
{
  nsCOMPtr<nsIFile> file;
  aProfileDir->Clone(getter_AddRefs(file));
  if (!file)
    return;

  file->AppendNative(NS_LITERAL_CSTRING("compreg.dat"));
  file->Remove(PR_FALSE);

  file->SetNativeLeafName(NS_LITERAL_CSTRING("xpti.dat"));
  file->Remove(PR_FALSE);

  file->SetNativeLeafName(NS_LITERAL_CSTRING(".autoreg"));
  file->Remove(PR_FALSE);

  if (aRemoveEMFiles) {
    file->SetNativeLeafName(NS_LITERAL_CSTRING("extensions.ini"));
    file->Remove(PR_FALSE);
  }

  aLocalProfileDir->Clone(getter_AddRefs(file));
  if (!file)
    return;

  file->AppendNative(NS_LITERAL_CSTRING("XUL" PLATFORM_FASL_SUFFIX));
  file->Remove(PR_FALSE);
}

// To support application initiated restart via nsIAppStartup.quit, we
// need to save various environment variables, and then restore them
// before re-launching the application.

static struct {
  const char *name;
  char *value;
} gSavedVars[] = {
  {"XUL_APP_FILE", nsnull}
};

static void SaveStateForAppInitiatedRestart()
{
  for (size_t i = 0; i < NS_ARRAY_LENGTH(gSavedVars); ++i) {
    const char *s = PR_GetEnv(gSavedVars[i].name);
    if (s)
      gSavedVars[i].value = PR_smprintf("%s=%s", gSavedVars[i].name, s);
  }
}

static void RestoreStateForAppInitiatedRestart()
{
  for (size_t i = 0; i < NS_ARRAY_LENGTH(gSavedVars); ++i) {
    if (gSavedVars[i].value)
      PR_SetEnv(gSavedVars[i].value);
  }
}

#ifdef MOZ_CRASHREPORTER
// When we first initialize the crash reporter we don't have a profile,
// so we set the minidump path to $TEMP.  Once we have a profile,
// we set it to $PROFILE/minidumps, creating the directory
// if needed.
static void MakeOrSetMinidumpPath(nsIFile* profD)
{
  nsCOMPtr<nsIFile> dumpD;
  nsresult rv = profD->Clone(getter_AddRefs(dumpD));
  
  if(dumpD) {
    PRBool fileExists;
    //XXX: do some more error checking here
    dumpD->Append(NS_LITERAL_STRING("minidumps"));
    rv = dumpD->Exists(&fileExists);
    if(!fileExists) {
      dumpD->Create(nsIFile::DIRECTORY_TYPE, 0700);
    }

    nsAutoString pathStr;
    if(NS_SUCCEEDED(dumpD->GetPath(pathStr)))
      CrashReporter::SetMinidumpPath(pathStr);
  }
}
#endif

const nsXREAppData* gAppData = nsnull;

#if defined(XP_OS2)
// because we use early returns, we use a stack-based helper to un-set the OS2 FP handler
class ScopedFPHandler {
private:
  EXCEPTIONREGISTRATIONRECORD excpreg;

public:
  ScopedFPHandler() { PR_OS2_SetFloatExcpHandler(&excpreg); }
  ~ScopedFPHandler() { PR_OS2_UnsetFloatExcpHandler(&excpreg); }
};
#endif

#ifdef MOZ_WIDGET_GTK2
#include "prlink.h"
typedef void (*_g_set_application_name_fn)(const gchar *application_name);
typedef void (*_gtk_window_set_auto_startup_notification_fn)(gboolean setting);

static PRFuncPtr FindFunction(const char* aName)
{
  PRLibrary *lib = nsnull;
  PRFuncPtr result = PR_FindFunctionSymbolAndLibrary(aName, &lib);
  // Since the library was already loaded, we can safely unload it here.
  if (lib) {
    PR_UnloadLibrary(lib);
  }
  return result;
}

static nsIWidget* GetMainWidget(nsIDOMWindow* aWindow)
{
  // get the native window for this instance
  nsCOMPtr<nsPIDOMWindow> window(do_QueryInterface(aWindow));
  NS_ENSURE_TRUE(window, nsnull);

  nsCOMPtr<nsIBaseWindow> baseWindow
    (do_QueryInterface(window->GetDocShell()));
  NS_ENSURE_TRUE(baseWindow, nsnull);

  nsCOMPtr<nsIWidget> mainWidget;
  baseWindow->GetMainWidget(getter_AddRefs(mainWidget));
  return mainWidget;
}

static nsGTKToolkit* GetGTKToolkit()
{
  nsCOMPtr<nsIAppShellService> svc = do_GetService(NS_APPSHELLSERVICE_CONTRACTID);
  if (!svc)
    return nsnull;
  nsCOMPtr<nsIDOMWindowInternal> window;
  svc->GetHiddenDOMWindow(getter_AddRefs(window));
  if (!window)
    return nsnull;
  nsIWidget* widget = GetMainWidget(window);
  if (!widget)
    return nsnull;
  nsIToolkit* toolkit = widget->GetToolkit();
  if (!toolkit)
    return nsnull;
  return static_cast<nsGTKToolkit*>(toolkit);
}

static void MOZ_gdk_display_close(GdkDisplay *display)
{
  // gdk_display_close was broken prior to gtk+-2.10.0.
  // (http://bugzilla.gnome.org/show_bug.cgi?id=85715)
  // gdk_display_manager_set_default_display (gdk_display_manager_get(), NULL)
  // was also broken.
  if(gtk_check_version(2,10,0) != NULL) {
    // Version check failed - broken gdk_display_close.
    //
    // Let the gdk structures leak but at least close the Display,
    // assuming that gdk will not use it again.
    Display* dpy = GDK_DISPLAY_XDISPLAY(display);
    XCloseDisplay(dpy);
  }
  else {
    gdk_display_close(display);
#if GTK_CHECK_VERSION(2,8,0) && \
  (defined(DEBUG) || defined(NS_BUILD_REFCNT_LOGGING) || defined(NS_TRACE_MALLOC))
    cairo_debug_reset_static_data();
#endif
  }
}
#endif // MOZ_WIDGET_GTK2

/** 
 * NSPR will search for the "nspr_use_zone_allocator" symbol throughout
 * the process and use it to determine whether the application defines its own
 * memory allocator or not.
 *
 * Since most applications (e.g. Firefox and Thunderbird) don't use any special
 * allocators and therefore don't define this symbol, NSPR must search the
 * entire process, which reduces startup performance.
 *
 * By defining the symbol here, we can avoid the wasted lookup and hopefully
 * improve startup performance.
 */
PRBool nspr_use_zone_allocator = PR_FALSE;

int
XRE_main(int argc, char* argv[], const nsXREAppData* aAppData)
{
  nsresult rv;
  ArgResult ar;
  NS_TIMELINE_MARK("enter main");

#ifdef DEBUG
  if (PR_GetEnv("XRE_MAIN_BREAK"))
    NS_BREAK();
#endif

#ifdef XP_WIN32
  // Suppress the "DLL Foo could not be found" dialog, such that if dependent
  // libraries (such as GDI+) are not preset, we gracefully fail to load those
  // XPCOM components, instead of being ungraceful.
  UINT realMode = SetErrorMode(0);
  realMode |= SEM_FAILCRITICALERRORS;
  // If XRE_NO_WINDOWS_CRASH_DIALOG is set, suppress displaying the "This
  // application has crashed" dialog box.  This is mainly useful for
  // automated testing environments, e.g. tinderbox, where there's no need
  // for a dozen of the dialog boxes to litter the console
  if (getenv("XRE_NO_WINDOWS_CRASH_DIALOG"))
    realMode |= SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX;

  SetErrorMode(realMode);

#ifdef DEBUG
  // Disable small heap allocator to get heapwalk() giving us
  // accurate heap numbers. Win2k non-debug does not use small heap allocator.
  // Win2k debug seems to be still using it.
  // http://msdn.microsoft.com/library/default.asp?url=/library/en-us/vclib/html/_crt__set_sbh_threshold.asp
  _set_sbh_threshold(0);
#endif
#endif

#if defined(XP_UNIX) || defined(XP_BEOS)
  InstallUnixSignalHandlers(argv[0]);
#endif

#ifdef MOZ_ACCESSIBILITY_ATK
  // Reset GTK_MODULES, strip atk-bridge if exists
  // Mozilla will load libatk-bridge.so later if necessary
  const char* gtkModules = PR_GetEnv("GTK_MODULES");
  if (gtkModules && *gtkModules) {
    nsCString gtkModulesStr(gtkModules);
    gtkModulesStr.ReplaceSubstring("atk-bridge", "");
    char* expr = PR_smprintf("GTK_MODULES=%s", gtkModulesStr.get());
    if (expr)
      PR_SetEnv(expr);
    // We intentionally leak |expr| here since it is required by PR_SetEnv.
  }
#endif

  // Unbuffer stdout, needed for tinderbox tests.
  setbuf(stdout, 0);

#if defined(FREEBSD)
  // Disable all SIGFPE's on FreeBSD, as it has non-IEEE-conformant fp
  // trap behavior that trips up on floating-point tests performed by
  // the JS engine.  See bugzilla bug 9967 details.
  fpsetmask(0);
#endif

  gArgc = argc;
  gArgv = argv;

  NS_ENSURE_TRUE(aAppData, 2);

#ifdef XP_MACOSX
  // The xulrunner stub executable tricks CFBundleGetMainBundle on
  // purpose into lying about the main bundle path. It will set
  // XRE_BINARY_PATH to inform us of our real location.
  gBinaryPath = getenv("XRE_BINARY_PATH");

  if (gBinaryPath && !*gBinaryPath)
    gBinaryPath = nsnull;
#endif

  // Check for application.ini overrides
  const char* override = nsnull;
  ar = CheckArg("override", PR_TRUE, &override);
  if (ar == ARG_BAD) {
    Output(PR_TRUE, "Incorrect number of arguments passed to -override");
    return 1;
  }
  else if (ar == ARG_FOUND) {
    nsCOMPtr<nsILocalFile> overrideLF;
    rv = XRE_GetFileFromPath(override, getter_AddRefs(overrideLF));
    if (NS_FAILED(rv)) {
      Output(PR_TRUE, "Error: unrecognized override.ini path.\n");
      return 1;
    }

    nsXREAppData* overrideAppData = const_cast<nsXREAppData*>(aAppData);
    rv = XRE_ParseAppData(overrideLF, overrideAppData);
    if (NS_FAILED(rv)) {
      Output(PR_TRUE, "Couldn't read override.ini");
      return 1;
    }
  }

  ScopedAppData appData(aAppData);
  gAppData = &appData;

  // Check sanity and correctness of app data.

  if (!appData.name) {
    Output(PR_TRUE, "Error: App:Name not specified in application.ini\n");
    return 1;
  }
  if (!appData.buildID) {
    Output(PR_TRUE, "Error: App:BuildID not specified in application.ini\n");
    return 1;
  }

  ScopedLogging log;

  if (!appData.xreDirectory) {
    nsCOMPtr<nsILocalFile> lf;
    rv = XRE_GetBinaryPath(gArgv[0], getter_AddRefs(lf));
    if (NS_FAILED(rv))
      return 2;

    nsCOMPtr<nsIFile> greDir;
    rv = lf->GetParent(getter_AddRefs(greDir));
    if (NS_FAILED(rv))
      return 2;
    
    rv = CallQueryInterface(greDir, &appData.xreDirectory);
    if (NS_FAILED(rv))
      return 2;
  }

  nsCOMPtr<nsIFile> iniFile;
  rv = appData.xreDirectory->Clone(getter_AddRefs(iniFile));
  if (NS_FAILED(rv))
    return 2;

  iniFile->AppendNative(NS_LITERAL_CSTRING("platform.ini"));

  nsCOMPtr<nsILocalFile> localIniFile = do_QueryInterface(iniFile);
  if (!localIniFile)
    return 2;

  nsINIParser parser;
  rv = parser.Init(localIniFile);
  if (NS_SUCCEEDED(rv)) {
    rv = parser.GetString("Build", "Milestone",
                          gToolkitVersion, sizeof(gToolkitVersion));
    NS_ASSERTION(NS_SUCCEEDED(rv), "Failed to get toolkit version");

    rv = parser.GetString("Build", "BuildID",
                          gToolkitBuildID, sizeof(gToolkitBuildID));
    NS_ASSERTION(NS_SUCCEEDED(rv), "Failed to get toolkit buildid");
  }
  else {
    NS_ERROR("Couldn't parse platform.ini!");
  }

  if (appData.size > offsetof(nsXREAppData, minVersion)) {
    if (!appData.minVersion) {
      Output(PR_TRUE, "Error: Gecko:MinVersion not specified in application.ini\n");
      return 1;
    }

    if (!appData.maxVersion) {
      // If no maxVersion is specified, we assume the app is only compatible
      // with the initial preview release. Do not increment this number ever!
      SetAllocatedString(appData.maxVersion, "1.*");
    }

    if (NS_CompareVersions(appData.minVersion, gToolkitVersion) > 0 ||
        NS_CompareVersions(appData.maxVersion, gToolkitVersion) < 0) {
      Output(PR_TRUE, "Error: Platform version '%s' is not compatible with\n"
             "minVersion >= %s\nmaxVersion <= %s\n",
             gToolkitVersion,
             appData.minVersion, appData.maxVersion);
      return 1;
    }
  }

#ifdef MOZ_CRASHREPORTER
  const char* crashreporterEnv = PR_GetEnv("MOZ_CRASHREPORTER");
  if (crashreporterEnv && *crashreporterEnv) {
    appData.flags |= NS_XRE_ENABLE_CRASH_REPORTER;
  }

  if ((appData.flags & NS_XRE_ENABLE_CRASH_REPORTER) &&
      NS_SUCCEEDED(
         CrashReporter::SetExceptionHandler(appData.xreDirectory,
                                            appData.crashReporterURL))) {
    // pass some basic info from the app data
    if (appData.vendor)
      CrashReporter::AnnotateCrashReport(NS_LITERAL_CSTRING("Vendor"),
                                         nsDependentCString(appData.vendor));
    if (appData.name)
      CrashReporter::AnnotateCrashReport(NS_LITERAL_CSTRING("ProductName"),
                                         nsDependentCString(appData.name));
    if (appData.version)
      CrashReporter::AnnotateCrashReport(NS_LITERAL_CSTRING("Version"),
                                         nsDependentCString(appData.version));
    if (appData.buildID)
      CrashReporter::AnnotateCrashReport(NS_LITERAL_CSTRING("BuildID"),
                                         nsDependentCString(appData.buildID));
    CrashReporter::SetRestartArgs(argc, argv);

    // annotate other data (user id etc)
    nsXREDirProvider dirProvider;
    nsCOMPtr<nsILocalFile> userAppDataDir;
    rv = dirProvider.Initialize(gAppData->directory, gAppData->xreDirectory);
    if (NS_SUCCEEDED(rv) &&
        NS_SUCCEEDED(dirProvider.GetUserAppDataDirectory(
                                                         getter_AddRefs(userAppDataDir)))) {
      CrashReporter::SetupExtraData(userAppDataDir,
                                    nsDependentCString(appData.buildID));

      // see if we have a crashreporter-override.ini in the application directory
      nsCOMPtr<nsIFile> overrideini;
      PRBool exists;
      static char overrideEnv[MAXPATHLEN];
      if (NS_SUCCEEDED(dirProvider.GetAppDir()->Clone(getter_AddRefs(overrideini))) &&
          NS_SUCCEEDED(overrideini->AppendNative(NS_LITERAL_CSTRING("crashreporter-override.ini"))) &&
          NS_SUCCEEDED(overrideini->Exists(&exists)) &&
          exists) {
#ifdef XP_WIN
        nsAutoString overridePathW;
        overrideini->GetPath(overridePathW);
        NS_ConvertUTF16toUTF8 overridePath(overridePathW);
#else
        nsCAutoString overridePath;
        overrideini->GetNativePath(overridePath);
#endif

        sprintf(overrideEnv, "MOZ_CRASHREPORTER_STRINGS_OVERRIDE=%s",
                overridePath.get());
        PR_SetEnv(overrideEnv);
      }
    }
  }
#endif

#ifdef XP_MACOSX
  if (PR_GetEnv("MOZ_LAUNCHED_CHILD")) {
    // This is needed, on relaunch, to force the OS to use the "Cocoa Dock
    // API".  Otherwise the call to ReceiveNextEvent() below will make it
    // use the "Carbon Dock API".  For more info see bmo bug 377166.
    EnsureUseCocoaDockAPI();

    // When the app relaunches, the original process exits.  This causes
    // the dock tile to stop bouncing, lose the "running" triangle, and
    // if the tile does not permanently reside in the Dock, even disappear.
    // This can be confusing to the user, who is expecting the app to launch.
    // Calling ReceiveNextEvent without requesting any event is enough to
    // cause a dock tile for the child process to appear.
    const EventTypeSpec kFakeEventList[] = { { INT_MAX, INT_MAX } };
    EventRef event;
    ::ReceiveNextEvent(GetEventTypeCount(kFakeEventList), kFakeEventList,
                       kEventDurationNoWait, PR_FALSE, &event);
  }

  if (CheckArg("foreground")) {
    // The original process communicates that it was in the foreground by
    // adding this argument.  This new process, which is taking over for
    // the old one, should make itself the active application.
    ProcessSerialNumber psn;
    if (::GetCurrentProcess(&psn) == noErr)
      ::SetFrontProcess(&psn);
  }
#endif

  PR_SetEnv("MOZ_LAUNCHED_CHILD=");

  gRestartArgc = gArgc;
  gRestartArgv = (char**) malloc(sizeof(char*) * (gArgc + 1));
  if (!gRestartArgv) return 1;

  int i;
  for (i = 0; i < gArgc; ++i) {
    gRestartArgv[i] = gArgv[i];
  }
  gRestartArgv[gArgc] = nsnull;

#if defined(XP_OS2)
  PRBool StartOS2App(int aArgc, char **aArgv);
  if (!StartOS2App(gArgc, gArgv))
    return 1;
  ScopedFPHandler handler;
#endif /* XP_OS2 */

  ar = CheckArg("safe-mode", PR_TRUE);
  if (ar == ARG_BAD) {
    PR_fprintf(PR_STDERR, "Error: argument -safe-mode is invalid when argument -osint is specified\n");
    return 1;
  } else if (ar == ARG_FOUND) {
    gSafeMode = PR_TRUE;
  }

#ifdef XP_MACOSX
  if (GetCurrentKeyModifiers() & optionKey)
    gSafeMode = PR_TRUE;
#endif

  // Handle -no-remote command line argument. Setup the environment to
  // better accommodate other components and various restart scenarios.
  ar = CheckArg("no-remote", PR_TRUE);
  if (ar == ARG_BAD) {
    PR_fprintf(PR_STDERR, "Error: argument -a requires an application name\n");
    return 1;
  } else if (ar == ARG_FOUND) {
    PR_SetEnv("MOZ_NO_REMOTE=1");
  }

  // Handle -help and -version command line arguments.
  // They should return quickly, so we deal with them here.
  if (CheckArg("h") || CheckArg("help") || CheckArg("?")) {
    DumpHelp();
    return 0;
  }

  if (CheckArg("v") || CheckArg("version")) {
    DumpVersion();
    return 0;
  }
    
#ifdef NS_TRACE_MALLOC
  gArgc = argc = NS_TraceMallocStartupArgs(gArgc, gArgv);
#endif

  {
    nsXREDirProvider dirProvider;
    rv = dirProvider.Initialize(gAppData->directory, gAppData->xreDirectory);
    if (NS_FAILED(rv))
      return 1;

    // Check for -register, which registers chrome and then exits immediately.
    ar = CheckArg("register", PR_TRUE);
    if (ar == ARG_BAD) {
      PR_fprintf(PR_STDERR, "Error: argument -register is invalid when argument -osint is specified\n");
      return 1;
    } else if (ar == ARG_FOUND) {
      ScopedXPCOMStartup xpcom;
      rv = xpcom.Initialize();
      NS_ENSURE_SUCCESS(rv, 1);

      {
        nsCOMPtr<nsIChromeRegistry> chromeReg
          (do_GetService("@mozilla.org/chrome/chrome-registry;1"));
        NS_ENSURE_TRUE(chromeReg, 1);

        chromeReg->CheckForNewChrome();
      }
      return 0;
    }

#if defined(MOZ_WIDGET_GTK2) || defined(MOZ_ENABLE_XREMOTE)
    // Stash DESKTOP_STARTUP_ID in malloc'ed memory because gtk_init will clear it.
#define HAVE_DESKTOP_STARTUP_ID
    const char* desktopStartupIDEnv = PR_GetEnv("DESKTOP_STARTUP_ID");
    nsCAutoString desktopStartupID;
    if (desktopStartupIDEnv) {
      desktopStartupID.Assign(desktopStartupIDEnv);
    }
#endif

#if defined(MOZ_WIDGET_GTK2)
    g_thread_init(NULL);
    // setup for private colormap.  Ideally we'd like to do this
    // in nsAppShell::Create, but we need to get in before gtk
    // has been initialized to make sure everything is running
    // consistently.
    if (CheckArg("install"))
      gdk_rgb_set_install(TRUE);

    // Initialize GTK here for splash.

    // Open the display ourselves instead of using gtk_init, so that we can
    // close it without fear that one day gtk might clean up the display it
    // opens.
    if (!gtk_parse_args(&gArgc, &gArgv))
      return 1;

    GdkDisplay* display = nsnull;
    {
      // display_name is owned by gdk.
      const char *display_name = gdk_get_display_arg_name();
      if (!display_name) {
        display_name = PR_GetEnv("DISPLAY");
        if (!display_name) {
          PR_fprintf(PR_STDERR, "Error: no display specified\n");
          return 1;
        }
      }
      display = gdk_display_open(display_name);
      if (!display) {
        PR_fprintf(PR_STDERR, "Error: cannot open display: %s\n", display_name);
        return 1;
      }
    }
    gdk_display_manager_set_default_display (gdk_display_manager_get(),
                                             display);
    
    // g_set_application_name () is only defined in glib2.2 and higher.
    _g_set_application_name_fn _g_set_application_name =
      (_g_set_application_name_fn)FindFunction("g_set_application_name");
    if (_g_set_application_name) {
      _g_set_application_name(gAppData->name);
    }
    _gtk_window_set_auto_startup_notification_fn _gtk_window_set_auto_startup_notification =
      (_gtk_window_set_auto_startup_notification_fn)FindFunction("gtk_window_set_auto_startup_notification");
    if (_gtk_window_set_auto_startup_notification) {
      _gtk_window_set_auto_startup_notification(PR_FALSE);
    }

    gtk_widget_set_default_visual(gdk_rgb_get_visual());
    gtk_widget_set_default_colormap(gdk_rgb_get_cmap());
#endif /* MOZ_WIDGET_GTK2 */

    // Call the code to install our handler
#ifdef MOZ_JPROF
    setupProfilingStuff();
#endif

    // Try to allocate "native app support."
    nsCOMPtr<nsINativeAppSupport> nativeApp;
    rv = NS_CreateNativeAppSupport(getter_AddRefs(nativeApp));
    if (NS_FAILED(rv))
      return 1;

    PRBool canRun = PR_FALSE;
    rv = nativeApp->Start(&canRun);
    if (NS_FAILED(rv) || !canRun) {
      return 1;
    }

#ifdef MOZ_ENABLE_XREMOTE
    // handle -remote now that xpcom is fired up

    const char* xremotearg;
    ar = CheckArg("remote", PR_TRUE, &xremotearg);
    if (ar == ARG_BAD) {
      PR_fprintf(PR_STDERR, "Error: -remote requires an argument\n");
      return 1;
    }
    const char* desktopStartupIDPtr =
      desktopStartupID.IsEmpty() ? nsnull : desktopStartupID.get();
    if (ar) {
      return HandleRemoteArgument(xremotearg, desktopStartupIDPtr);
    }

    if (!PR_GetEnv("MOZ_NO_REMOTE")) {
      // Try to remote the entire command line. If this fails, start up normally.
      RemoteResult rr = RemoteCommandLine(desktopStartupIDPtr);
      if (rr == REMOTE_FOUND)
        return 0;
      else if (rr == REMOTE_ARG_BAD)
        return 1;
    }
#endif

#if defined(MOZ_UPDATER)
  // Check for and process any available updates
  nsCOMPtr<nsIFile> updRoot;
  PRBool persistent;
  rv = dirProvider.GetFile(XRE_UPDATE_ROOT_DIR, &persistent,
                           getter_AddRefs(updRoot));
  // XRE_UPDATE_ROOT_DIR may fail. Fallback to appDir if failed
  if (NS_FAILED(rv))
    updRoot = dirProvider.GetAppDir();

  ProcessUpdates(dirProvider.GetGREDir(),
                 dirProvider.GetAppDir(),
                 updRoot,
                 gRestartArgc,
                 gRestartArgv);
#endif

    nsCOMPtr<nsIProfileLock> profileLock;
    PRBool startOffline = PR_FALSE;
    nsCAutoString profileName;

    rv = SelectProfile(getter_AddRefs(profileLock), nativeApp, &startOffline,
                       &profileName);
    if (rv == NS_ERROR_LAUNCHED_CHILD_PROCESS ||
        rv == NS_ERROR_ABORT) return 0;
    if (NS_FAILED(rv)) return 1;

    nsCOMPtr<nsILocalFile> profD;
    rv = profileLock->GetDirectory(getter_AddRefs(profD));
    NS_ENSURE_SUCCESS(rv, 1);

    nsCOMPtr<nsILocalFile> profLD;
    rv = profileLock->GetLocalDirectory(getter_AddRefs(profLD));
    NS_ENSURE_SUCCESS(rv, 1);

    rv = dirProvider.SetProfile(profD, profLD);
    NS_ENSURE_SUCCESS(rv, 1);

    //////////////////////// NOW WE HAVE A PROFILE ////////////////////////

#ifdef MOZ_CRASHREPORTER
    if (appData.flags & NS_XRE_ENABLE_CRASH_REPORTER)
        MakeOrSetMinidumpPath(profD);
#endif

    PRBool upgraded = PR_FALSE;

    nsCAutoString version;
    BuildVersion(version);

#ifdef TARGET_OS_ABI
    NS_NAMED_LITERAL_CSTRING(osABI, TARGET_OS_ABI);
#else
    // No TARGET_XPCOM_ABI, but at least the OS is known
    NS_NAMED_LITERAL_CSTRING(osABI, OS_TARGET "_UNKNOWN");
#endif

    // Check for version compatibility with the last version of the app this 
    // profile was started with.  The format of the version stamp is defined
    // by the BuildVersion function.
    PRBool versionOK = CheckCompatibility(profD, version, osABI,
                                          dirProvider.GetGREDir(),
                                          gAppData->directory);

    // Every time a profile is loaded by a build with a different version,
    // it updates the compatibility.ini file saying what version last wrote
    // the compreg.dat.  On subsequent launches if the version matches, 
    // there is no need for re-registration.  If the user loads the same
    // profile in different builds the component registry must be
    // re-generated to prevent mysterious component loading failures.
    //
    if (gSafeMode) {
      RemoveComponentRegistries(profD, profLD, PR_FALSE);
      WriteVersion(profD, NS_LITERAL_CSTRING("Safe Mode"), osABI,
                   dirProvider.GetGREDir(), gAppData->directory);
    }
    else if (versionOK) {
      if (ComponentsListChanged(profD)) {
        // Remove compreg.dat and xpti.dat, forcing component re-registration.
        // The new list of additional components directories is derived from
        // information in "extensions.ini".
        RemoveComponentRegistries(profD, profLD, PR_FALSE);
      }
      // Nothing need be done for the normal startup case.
    }
    else {
      // Remove compreg.dat and xpti.dat, forcing component re-registration
      // with the default set of components (this disables any potentially
      // troublesome incompatible XPCOM components). 
      RemoveComponentRegistries(profD, profLD, PR_TRUE);

      // Tell the Extension Manager it should check for incompatible 
      // Extensions and re-write the "extensions.ini" file with a list of 
      // directories for compatible extensions
      upgraded = PR_TRUE;

      // Write out version
      WriteVersion(profD, version, osABI,
                   dirProvider.GetGREDir(), gAppData->directory);
    }

    PRBool needsRestart = PR_FALSE;
    PRBool appInitiatedRestart = PR_FALSE;

    // Allows the user to forcefully bypass the restart process at their
    // own risk. Useful for debugging or for tinderboxes where child 
    // processes can be problematic.
    {
      // Start the real application
      ScopedXPCOMStartup xpcom;
      rv = xpcom.Initialize();
      NS_ENSURE_SUCCESS(rv, 1); 
      rv = xpcom.DoAutoreg();
      rv |= xpcom.RegisterProfileService();
      rv |= xpcom.SetWindowCreator(nativeApp);
      NS_ENSURE_SUCCESS(rv, 1);

      {
        if (startOffline) {
          nsCOMPtr<nsIIOService2> io (do_GetService("@mozilla.org/network/io-service;1"));
          NS_ENSURE_TRUE(io, 1);
          io->SetManageOfflineStatus(PR_FALSE);
          io->SetOffline(PR_TRUE);
        }

        {
          NS_TIMELINE_ENTER("startupNotifier");
          nsCOMPtr<nsIObserver> startupNotifier
            (do_CreateInstance(NS_APPSTARTUPNOTIFIER_CONTRACTID, &rv));
          NS_ENSURE_SUCCESS(rv, 1);

          startupNotifier->Observe(nsnull, APPSTARTUP_TOPIC, nsnull);
          NS_TIMELINE_LEAVE("startupNotifier");
        }

        nsCOMPtr<nsIAppStartup> appStartup
          (do_GetService(NS_APPSTARTUP_CONTRACTID));
        NS_ENSURE_TRUE(appStartup, 1);

        if (gDoMigration) {
          nsCOMPtr<nsIFile> file;
          dirProvider.GetAppDir()->Clone(getter_AddRefs(file));
          file->AppendNative(NS_LITERAL_CSTRING("override.ini"));
          nsINIParser parser;
          nsCOMPtr<nsILocalFile> localFile(do_QueryInterface(file));
          nsresult rv = parser.Init(localFile);
          if (NS_SUCCEEDED(rv)) {
            nsCAutoString buf;
            rv = parser.GetString("XRE", "EnableProfileMigrator", buf);
            if (NS_SUCCEEDED(rv)) {
              if (buf[0] == '0' || buf[0] == 'f' || buf[0] == 'F') {
                gDoMigration = PR_FALSE;
              }
            }
          }
        }

        // Profile Migration
        if (gAppData->flags & NS_XRE_ENABLE_PROFILE_MIGRATOR && gDoMigration) {
          gDoMigration = PR_FALSE;
          nsCOMPtr<nsIProfileMigrator> pm
            (do_CreateInstance(NS_PROFILEMIGRATOR_CONTRACTID));
          if (pm)
            pm->Migrate(&dirProvider);
        }
        dirProvider.DoStartup();

        nsCOMPtr<nsICommandLineRunner> cmdLine
          (do_CreateInstance("@mozilla.org/toolkit/command-line;1"));
        NS_ENSURE_TRUE(cmdLine, 1);

        nsCOMPtr<nsIFile> workingDir;
        rv = NS_GetSpecialDirectory(NS_OS_CURRENT_WORKING_DIR, getter_AddRefs(workingDir));
        NS_ENSURE_SUCCESS(rv, 1);

        rv = cmdLine->Init(gArgc, gArgv,
                           workingDir, nsICommandLine::STATE_INITIAL_LAUNCH);
        NS_ENSURE_SUCCESS(rv, 1);

        /* Special-case services that need early access to the command
           line. */
        nsCOMPtr<nsIObserver> chromeObserver
          (do_GetService("@mozilla.org/chrome/chrome-registry;1"));
        if (chromeObserver) {
          chromeObserver->Observe(cmdLine, "command-line-startup", nsnull);
        }

        NS_TIMELINE_ENTER("appStartup->CreateHiddenWindow");
        rv = appStartup->CreateHiddenWindow();
        NS_TIMELINE_LEAVE("appStartup->CreateHiddenWindow");
        NS_ENSURE_SUCCESS(rv, 1);

#if defined(HAVE_DESKTOP_STARTUP_ID) && defined(MOZ_WIDGET_GTK2)
        nsRefPtr<nsGTKToolkit> toolkit = GetGTKToolkit();
        if (toolkit && !desktopStartupID.IsEmpty()) {
          toolkit->SetDesktopStartupID(desktopStartupID);
        }
#endif

        // Extension Compatibility Checking and Startup
        if (gAppData->flags & NS_XRE_ENABLE_EXTENSION_MANAGER) {
          nsCOMPtr<nsIExtensionManager> em(do_GetService("@mozilla.org/extensions/manager;1"));
          NS_ENSURE_TRUE(em, 1);

          ar = CheckArg("install-global-extension", PR_TRUE);
          if (ar == ARG_BAD) {
            PR_fprintf(PR_STDERR, "Error: argument -install-global-extension is invalid when argument -osint is specified\n");
            return 1;
          } else if (ar == ARG_FOUND) {
            // Do the required processing and then shut down.
            em->HandleCommandLineArgs(cmdLine);
            return 0;
          }

          ar = CheckArg("install-global-theme", PR_TRUE);
          if (ar == ARG_BAD) {
            PR_fprintf(PR_STDERR, "Error: argument -install-global-theme is invalid when argument -osint is specified\n");
            return 1;
          } else if (ar == ARG_FOUND) {
            // Do the required processing and then shut down.
            em->HandleCommandLineArgs(cmdLine);
            return 0;
          }

          if (upgraded) {
            rv = em->CheckForMismatches(&needsRestart);
            if (NS_FAILED(rv)) {
              needsRestart = PR_FALSE;
              upgraded = PR_FALSE;
            }
          }

          if (!upgraded || !needsRestart)
            em->Start(cmdLine, &needsRestart);
        }

        // We want to restart no more than 2 times. The first restart,
        // NO_EM_RESTART == "0" , and the second time, "1".
        char* noEMRestart = PR_GetEnv("NO_EM_RESTART");
        if (noEMRestart && *noEMRestart && *noEMRestart == '1') {
          if (upgraded || needsRestart) {
            NS_WARNING("EM tried to force us to restart twice! Forcefully preventing that.");
          }
          needsRestart = upgraded = PR_FALSE;
        }

        if (!upgraded && !needsRestart) {
          SaveStateForAppInitiatedRestart();

          // clear out any environment variables which may have been set 
          // during the relaunch process now that we know we won't be relaunching.
          PR_SetEnv("XRE_PROFILE_PATH=");
          PR_SetEnv("XRE_PROFILE_LOCAL_PATH=");
          PR_SetEnv("XRE_PROFILE_NAME=");
          PR_SetEnv("XRE_START_OFFLINE=");
          PR_SetEnv("XRE_IMPORT_PROFILES=");
          PR_SetEnv("NO_EM_RESTART=");
          PR_SetEnv("XUL_APP_FILE=");
          PR_SetEnv("XRE_BINARY_PATH=");

#ifdef XP_MACOSX
          // we re-initialize the command-line service and do appleevents munging
          // after we are sure that we're not restarting
          cmdLine = do_CreateInstance("@mozilla.org/toolkit/command-line;1");
          NS_ENSURE_TRUE(cmdLine, 1);

          SetupMacCommandLine(gArgc, gArgv);

          rv = cmdLine->Init(gArgc, gArgv,
                             workingDir, nsICommandLine::STATE_INITIAL_LAUNCH);
          NS_ENSURE_SUCCESS(rv, 1);
#endif
#ifdef MOZ_WIDGET_COCOA
          // Prepare Cocoa's form of Apple Event handling.
          SetupMacApplicationDelegate();
#endif
          nsCOMPtr<nsIObserverService> obsService
            (do_GetService("@mozilla.org/observer-service;1"));
          if (obsService)
            obsService->NotifyObservers(nsnull, "final-ui-startup", nsnull);

          rv = cmdLine->Run();
          NS_ENSURE_SUCCESS_LOG(rv, 1);

#ifdef MOZ_ENABLE_XREMOTE
          // if we have X remote support, start listening for requests on the
          // proxy window.
          nsCOMPtr<nsIRemoteService> remoteService;
          remoteService = do_GetService("@mozilla.org/toolkit/remote-service;1");
          if (remoteService)
            remoteService->Startup(gAppData->name,
                                   PromiseFlatCString(profileName).get());
#endif /* MOZ_ENABLE_XREMOTE */

          // enable win32 DDE responses and Mac appleevents responses
          nativeApp->Enable();

          NS_TIMELINE_ENTER("appStartup->Run");
          rv = appStartup->Run();
          NS_TIMELINE_LEAVE("appStartup->Run");
          if (NS_FAILED(rv)) {
            NS_ERROR("failed to run appstartup");
            gLogConsoleErrors = PR_TRUE;
          }

          // Check for an application initiated restart.  This is one that
          // corresponds to nsIAppStartup.quit(eRestart)
          if (rv == NS_SUCCESS_RESTART_APP) {
            needsRestart = PR_TRUE;
            appInitiatedRestart = PR_TRUE;
          }

#ifdef MOZ_ENABLE_XREMOTE
          // shut down the x remote proxy window
          if (remoteService)
            remoteService->Shutdown();
#endif /* MOZ_ENABLE_XREMOTE */

#ifdef MOZ_TIMELINE
          // Make sure we print this out even if timeline is runtime disabled
          if (NS_FAILED(NS_TIMELINE_LEAVE("main1")))
            NS_TimelineForceMark("...main1");
#endif
        }
        else {
          // Upgrade condition (build id changes), but the restart hint was 
          // not set by the Extension Manager. This is because the compatibility
          // resolution for Extensions is different than for the component 
          // registry - major milestone vs. build id. 
          needsRestart = PR_TRUE;

#ifdef XP_WIN
          ProcessDDE(nativeApp);
#endif

#ifdef XP_MACOSX
          SetupMacCommandLine(gRestartArgc, gRestartArgv);
#endif
        }
      }
    }

    // unlock the profile after ScopedXPCOMStartup object (xpcom) 
    // has gone out of scope.  see bug #386739 for more details
    profileLock->Unlock();

    // Restart the app after XPCOM has been shut down cleanly. 
    if (needsRestart) {
      if (appInitiatedRestart) {
        RestoreStateForAppInitiatedRestart();
      }
      else {
        char* noEMRestart = PR_GetEnv("NO_EM_RESTART");
        if (noEMRestart && *noEMRestart) {
          PR_SetEnv("NO_EM_RESTART=1");
        }
        else {
          PR_SetEnv("NO_EM_RESTART=0");
        }
      }

      // Ensure that these environment variables are set:
      SaveFileToEnvIfUnset("XRE_PROFILE_PATH", profD);
      SaveFileToEnvIfUnset("XRE_PROFILE_LOCAL_PATH", profLD);
      SaveWordToEnvIfUnset("XRE_PROFILE_NAME", profileName);

#ifdef XP_MACOSX
      if (gBinaryPath) {
        static char kEnvVar[MAXPATHLEN];
        sprintf(kEnvVar, "XRE_BINARY_PATH=%s", gBinaryPath);
        PR_SetEnv(kEnvVar);
      }
#endif

// XXXkt s/MOZ_TOOLKIT_GTK2/MOZ_WIDGET_GTK2/?
// but the hidden window has been destroyed so toolkit is NULL anyway.
#if defined(HAVE_DESKTOP_STARTUP_ID) && defined(MOZ_TOOLKIT_GTK2)
      nsGTKToolkit* toolkit = GetGTKToolkit();
      if (toolkit) {
        nsCAutoString currentDesktopStartupID;
        toolkit->GetDesktopStartupID(&currentDesktopStartupID);
        if (!currentDesktopStartupID.IsEmpty()) {
          nsCAutoString desktopStartupEnv;
          desktopStartupEnv.AssignLiteral("DESKTOP_STARTUP_ID=");
          desktopStartupEnv.Append(currentDesktopStartupID);
          // Leak it with extreme prejudice!
          PR_SetEnv(ToNewCString(desktopStartupEnv));
        }
      }
#endif

#ifdef MOZ_WIDGET_GTK2
      MOZ_gdk_display_close(display);
#endif

      rv = LaunchChild(nativeApp, appInitiatedRestart, upgraded ? -1 : 0);

#ifdef MOZ_CRASHREPORTER
      if (appData.flags & NS_XRE_ENABLE_CRASH_REPORTER)
        CrashReporter::UnsetExceptionHandler();
#endif

      return rv == NS_ERROR_LAUNCHED_CHILD_PROCESS ? 0 : 1;
    }

#ifdef MOZ_WIDGET_GTK2
    // gdk_display_close also calls gdk_display_manager_set_default_display
    // appropriately when necessary.
    MOZ_gdk_display_close(display);
#endif
  }

#ifdef MOZ_CRASHREPORTER
  if (appData.flags & NS_XRE_ENABLE_CRASH_REPORTER)
      CrashReporter::UnsetExceptionHandler();
#endif

  return NS_FAILED(rv) ? 1 : 0;
}
