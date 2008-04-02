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
 *   IBM Corp.
 *   Fredrik Holmqvist <thesuckiestemail@yahoo.se>
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

#include "nsCOMPtr.h"
#include "nsAutoPtr.h"
#include "nsDirectoryService.h"
#include "nsDirectoryServiceDefs.h"
#include "nsLocalFile.h"
#include "nsDebug.h"
#include "nsStaticAtom.h"
#include "nsEnumeratorUtils.h"

#include "nsICategoryManager.h"
#include "nsISimpleEnumerator.h"
#include "nsIStringEnumerator.h"

#if defined(XP_WIN)
#include <windows.h>
#include <shlobj.h>
#include <stdlib.h>
#include <stdio.h>
#elif defined(XP_UNIX)
#include <unistd.h>
#include <stdlib.h>
#include <sys/param.h>
#include "prenv.h"
#ifdef XP_MACOSX
#include <CoreServices/CoreServices.h>
#include <Folders.h>
#include <Files.h>
#include <Memory.h>
#include <Processes.h>
#include <Gestalt.h>
#include <CFURL.h>
#include <InternetConfig.h>
#endif
#elif defined(XP_OS2)
#define MAX_PATH _MAX_PATH
#elif defined(XP_BEOS)
#include <FindDirectory.h>
#include <Path.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/param.h>
#include <OS.h>
#include <image.h>
#include "prenv.h"
#endif

#include "SpecialSystemDirectory.h"
#include "nsAppFileLocationProvider.h"

#define COMPONENT_REGISTRY_NAME NS_LITERAL_CSTRING("compreg.dat")
#define COMPONENT_DIRECTORY     NS_LITERAL_CSTRING("components")

#define XPTI_REGISTRY_NAME      NS_LITERAL_CSTRING("xpti.dat")

// define home directory
// For Windows platform, We are choosing Appdata folder as HOME
#if defined (XP_WIN)
#define HOME_DIR NS_WIN_APPDATA_DIR
#elif defined (XP_MACOSX)
#define HOME_DIR NS_OSX_HOME_DIR
#elif defined (XP_UNIX)
#define HOME_DIR NS_UNIX_HOME_DIR
#elif defined (XP_OS2)
#define HOME_DIR NS_OS2_HOME_DIR
#elif defined (XP_BEOS)
#define HOME_DIR NS_BEOS_HOME_DIR
#endif

//----------------------------------------------------------------------------------------
nsresult 
nsDirectoryService::GetCurrentProcessDirectory(nsILocalFile** aFile)
//----------------------------------------------------------------------------------------
{
    NS_ENSURE_ARG_POINTER(aFile);
    *aFile = nsnull;
    
   //  Set the component registry location:
    if (!gService)
        return NS_ERROR_FAILURE;

    nsresult rv; 
 
    nsCOMPtr<nsIProperties> dirService;
    rv = nsDirectoryService::Create(nsnull, 
                                    NS_GET_IID(nsIProperties), 
                                    getter_AddRefs(dirService));  // needs to be around for life of product

    if (dirService)
    {
      nsCOMPtr <nsILocalFile> aLocalFile;
      dirService->Get(NS_XPCOM_INIT_CURRENT_PROCESS_DIR, NS_GET_IID(nsILocalFile), getter_AddRefs(aLocalFile));
      if (aLocalFile)
      {
        *aFile = aLocalFile;
        NS_ADDREF(*aFile);
        return NS_OK;
      }
    }

    nsLocalFile* localFile = new nsLocalFile;

    if (localFile == nsnull)
        return NS_ERROR_OUT_OF_MEMORY;
    NS_ADDREF(localFile);



#ifdef XP_WIN
    PRUnichar buf[MAX_PATH];
    if ( ::GetModuleFileNameW(0, buf, sizeof(buf)) )
    {
        // chop off the executable name by finding the rightmost backslash
        PRUnichar* lastSlash = wcsrchr(buf, L'\\');
        if (lastSlash)
            *(lastSlash + 1) = L'\0';

        localFile->InitWithPath(nsDependentString(buf));
        *aFile = localFile;
        return NS_OK;
    }

#elif defined(XP_MACOSX)
    // Works even if we're not bundled.
    CFBundleRef appBundle = CFBundleGetMainBundle();
    if (appBundle != nsnull)
    {
        CFURLRef bundleURL = CFBundleCopyExecutableURL(appBundle);
        if (bundleURL != nsnull)
        {
            CFURLRef parentURL = CFURLCreateCopyDeletingLastPathComponent(kCFAllocatorDefault, bundleURL);
            if (parentURL)
            {
                // Pass PR_TRUE for the "resolveAgainstBase" arg to CFURLGetFileSystemRepresentation.
                // This will resolve the relative portion of the CFURL against it base, giving a full
                // path, which CFURLCopyFileSystemPath doesn't do.
                char buffer[PATH_MAX];
                if (CFURLGetFileSystemRepresentation(parentURL, PR_TRUE, (UInt8 *)buffer, sizeof(buffer)))
                {
#ifdef DEBUG_conrad
                    printf("nsDirectoryService - CurrentProcessDir is: %s\n", buffer);
#endif
                    rv = localFile->InitWithNativePath(nsDependentCString(buffer));
                    if (NS_SUCCEEDED(rv))
                        *aFile = localFile;
                }
                CFRelease(parentURL);
            }
            CFRelease(bundleURL);
        }
    }
    
    NS_ASSERTION(*aFile, "nsDirectoryService - Could not determine CurrentProcessDir.\n");
    if (*aFile)
        return NS_OK;

#elif defined(XP_UNIX)

    // In the absence of a good way to get the executable directory let
    // us try this for unix:
    //    - if MOZILLA_FIVE_HOME is defined, that is it
    //    - else give the current directory
    char buf[MAXPATHLEN];

    // The MOZ_DEFAULT_MOZILLA_FIVE_HOME variable can be set at configure time with
    // a --with-default-mozilla-five-home=foo autoconf flag.
    // 
    // The idea here is to allow for builds that have a default MOZILLA_FIVE_HOME
    // regardless of the environment.  This makes it easier to write apps that
    // embed mozilla without having to worry about setting up the environment 
    //
    // We do this by putenv()ing the default value into the environment.  Note that
    // we only do this if it is not already set.
#ifdef MOZ_DEFAULT_MOZILLA_FIVE_HOME
    const char *home = PR_GetEnv("MOZILLA_FIVE_HOME");
    if (!home || !*home)
    {
        putenv("MOZILLA_FIVE_HOME=" MOZ_DEFAULT_MOZILLA_FIVE_HOME);
    }
#endif

    char *moz5 = PR_GetEnv("MOZILLA_FIVE_HOME");
    if (moz5 && *moz5)
    {
        if (realpath(moz5, buf)) {
            localFile->InitWithNativePath(nsDependentCString(buf));
            *aFile = localFile;
            return NS_OK;
        }
    }
#if defined(DEBUG)
    static PRBool firstWarning = PR_TRUE;

    if((!moz5 || !*moz5) && firstWarning) {
        // Warn that MOZILLA_FIVE_HOME not set, once.
        printf("Warning: MOZILLA_FIVE_HOME not set.\n");
        firstWarning = PR_FALSE;
    }
#endif /* DEBUG */

    // Fall back to current directory.
    if (getcwd(buf, sizeof(buf)))
    {
        localFile->InitWithNativePath(nsDependentCString(buf));
        *aFile = localFile;
        return NS_OK;
    }

#elif defined(XP_OS2)
    PPIB ppib;
    PTIB ptib;
    char buffer[CCHMAXPATH];
    DosGetInfoBlocks( &ptib, &ppib);
    DosQueryModuleName( ppib->pib_hmte, CCHMAXPATH, buffer);
    *strrchr( buffer, '\\') = '\0'; // XXX DBCS misery
    localFile->InitWithNativePath(nsDependentCString(buffer));
    *aFile = localFile;
    return NS_OK;

#elif defined(XP_BEOS)
    char buf[MAXPATHLEN];
    int32 cookie = 0;
    image_info info;
    char *p;
    *buf = 0;
    if(get_next_image_info(0, &cookie, &info) == B_OK)
    {
        strcpy(buf, info.name);
        if((p = strrchr(buf, '/')) != 0)
        {
            if( (p-2 >= buf) && *(p-2)=='/' && *(p-1)=='.')
                p -=2; 
            *p = 0;
            localFile->InitWithNativePath(nsDependentCString(buf));
            *aFile = localFile;
            return NS_OK;
        }
    }
#endif
    
    NS_RELEASE(localFile);

    NS_ERROR("unable to get current process directory");
    return NS_ERROR_FAILURE;
} // GetCurrentProcessDirectory()


nsIAtom*  nsDirectoryService::sCurrentProcess = nsnull;
nsIAtom*  nsDirectoryService::sComponentRegistry = nsnull;
nsIAtom*  nsDirectoryService::sXPTIRegistry = nsnull;
nsIAtom*  nsDirectoryService::sComponentDirectory = nsnull;
nsIAtom*  nsDirectoryService::sGRE_Directory = nsnull;
nsIAtom*  nsDirectoryService::sGRE_ComponentDirectory = nsnull;
nsIAtom*  nsDirectoryService::sOS_DriveDirectory = nsnull;
nsIAtom*  nsDirectoryService::sOS_TemporaryDirectory = nsnull;
nsIAtom*  nsDirectoryService::sOS_CurrentProcessDirectory = nsnull;
nsIAtom*  nsDirectoryService::sOS_CurrentWorkingDirectory = nsnull;
nsIAtom*  nsDirectoryService::sOS_DesktopDirectory = nsnull;
nsIAtom*  nsDirectoryService::sOS_HomeDirectory = nsnull;
#if defined (XP_MACOSX)
nsIAtom*  nsDirectoryService::sDirectory = nsnull;
nsIAtom*  nsDirectoryService::sTrashDirectory = nsnull;
nsIAtom*  nsDirectoryService::sStartupDirectory = nsnull;
nsIAtom*  nsDirectoryService::sShutdownDirectory = nsnull;
nsIAtom*  nsDirectoryService::sAppleMenuDirectory = nsnull;
nsIAtom*  nsDirectoryService::sControlPanelDirectory = nsnull;
nsIAtom*  nsDirectoryService::sExtensionDirectory = nsnull;
nsIAtom*  nsDirectoryService::sFontsDirectory = nsnull;
nsIAtom*  nsDirectoryService::sPreferencesDirectory = nsnull;
nsIAtom*  nsDirectoryService::sDocumentsDirectory = nsnull;
nsIAtom*  nsDirectoryService::sInternetSearchDirectory = nsnull;
nsIAtom*  nsDirectoryService::sUserLibDirectory = nsnull;
nsIAtom*  nsDirectoryService::sDefaultDownloadDirectory = nsnull;
nsIAtom*  nsDirectoryService::sUserDesktopDirectory = nsnull;
nsIAtom*  nsDirectoryService::sLocalDesktopDirectory = nsnull;
nsIAtom*  nsDirectoryService::sUserApplicationsDirectory = nsnull;
nsIAtom*  nsDirectoryService::sLocalApplicationsDirectory = nsnull;
nsIAtom*  nsDirectoryService::sUserDocumentsDirectory = nsnull;
nsIAtom*  nsDirectoryService::sLocalDocumentsDirectory = nsnull;
nsIAtom*  nsDirectoryService::sUserInternetPlugInDirectory = nsnull;
nsIAtom*  nsDirectoryService::sLocalInternetPlugInDirectory = nsnull;
nsIAtom*  nsDirectoryService::sUserFrameworksDirectory = nsnull;
nsIAtom*  nsDirectoryService::sLocalFrameworksDirectory = nsnull;
nsIAtom*  nsDirectoryService::sUserPreferencesDirectory = nsnull;
nsIAtom*  nsDirectoryService::sLocalPreferencesDirectory = nsnull;
nsIAtom*  nsDirectoryService::sPictureDocumentsDirectory = nsnull;
nsIAtom*  nsDirectoryService::sMovieDocumentsDirectory = nsnull;
nsIAtom*  nsDirectoryService::sMusicDocumentsDirectory = nsnull;
nsIAtom*  nsDirectoryService::sInternetSitesDirectory = nsnull;
#elif defined (XP_WIN) 
nsIAtom*  nsDirectoryService::sSystemDirectory = nsnull;
nsIAtom*  nsDirectoryService::sWindowsDirectory = nsnull;
nsIAtom*  nsDirectoryService::sWindowsProgramFiles = nsnull;
nsIAtom*  nsDirectoryService::sDesktop = nsnull;
nsIAtom*  nsDirectoryService::sPrograms = nsnull;
nsIAtom*  nsDirectoryService::sControls = nsnull;
nsIAtom*  nsDirectoryService::sPrinters = nsnull;
nsIAtom*  nsDirectoryService::sPersonal = nsnull;
nsIAtom*  nsDirectoryService::sFavorites = nsnull;
nsIAtom*  nsDirectoryService::sStartup = nsnull;
nsIAtom*  nsDirectoryService::sRecent = nsnull;
nsIAtom*  nsDirectoryService::sSendto = nsnull;
nsIAtom*  nsDirectoryService::sBitbucket = nsnull;
nsIAtom*  nsDirectoryService::sStartmenu = nsnull;
nsIAtom*  nsDirectoryService::sDesktopdirectory = nsnull;
nsIAtom*  nsDirectoryService::sDrives = nsnull;
nsIAtom*  nsDirectoryService::sNetwork = nsnull;
nsIAtom*  nsDirectoryService::sNethood = nsnull;
nsIAtom*  nsDirectoryService::sFonts = nsnull;
nsIAtom*  nsDirectoryService::sTemplates = nsnull;
nsIAtom*  nsDirectoryService::sCommon_Startmenu = nsnull;
nsIAtom*  nsDirectoryService::sCommon_Programs = nsnull;
nsIAtom*  nsDirectoryService::sCommon_Startup = nsnull;
nsIAtom*  nsDirectoryService::sCommon_Desktopdirectory = nsnull;
nsIAtom*  nsDirectoryService::sAppdata = nsnull;
nsIAtom*  nsDirectoryService::sLocalAppdata = nsnull;
nsIAtom*  nsDirectoryService::sPrinthood = nsnull;
nsIAtom*  nsDirectoryService::sWinCookiesDirectory = nsnull;
nsIAtom*  nsDirectoryService::sDefaultDownloadDirectory = nsnull;
#elif defined (XP_UNIX)
nsIAtom*  nsDirectoryService::sLocalDirectory = nsnull;
nsIAtom*  nsDirectoryService::sLibDirectory = nsnull;
#elif defined (XP_OS2)
nsIAtom*  nsDirectoryService::sSystemDirectory = nsnull;
nsIAtom*  nsDirectoryService::sOS2Directory = nsnull;
#elif defined (XP_BEOS)
nsIAtom*  nsDirectoryService::sSettingsDirectory = nsnull;
nsIAtom*  nsDirectoryService::sSystemDirectory = nsnull;
#endif


nsDirectoryService* nsDirectoryService::gService = nsnull;

nsDirectoryService::nsDirectoryService() :
    mHashtable(256, PR_TRUE)
{
}

NS_METHOD
nsDirectoryService::Create(nsISupports *outer, REFNSIID aIID, void **aResult)
{
    NS_ENSURE_ARG_POINTER(aResult);
    NS_ENSURE_NO_AGGREGATION(outer);

    if (!gService)
    {
        return NS_ERROR_NOT_INITIALIZED;
    }

    return gService->QueryInterface(aIID, aResult);
}

static const nsStaticAtom directory_atoms[] = {
    { NS_XPCOM_CURRENT_PROCESS_DIR,     &nsDirectoryService::sCurrentProcess },
    { NS_XPCOM_COMPONENT_REGISTRY_FILE, &nsDirectoryService::sComponentRegistry },
    { NS_XPCOM_COMPONENT_DIR,         &nsDirectoryService::sComponentDirectory },
    { NS_XPCOM_XPTI_REGISTRY_FILE, &nsDirectoryService::sXPTIRegistry },
    { NS_GRE_DIR,                  &nsDirectoryService::sGRE_Directory },
    { NS_GRE_COMPONENT_DIR,        &nsDirectoryService::sGRE_ComponentDirectory },
    { NS_OS_DRIVE_DIR,             &nsDirectoryService::sOS_DriveDirectory },
    { NS_OS_TEMP_DIR,              &nsDirectoryService::sOS_TemporaryDirectory },
    { NS_OS_CURRENT_PROCESS_DIR,   &nsDirectoryService::sOS_CurrentProcessDirectory },
    { NS_OS_CURRENT_WORKING_DIR,   &nsDirectoryService::sOS_CurrentWorkingDirectory },
    { NS_OS_HOME_DIR,              &nsDirectoryService::sOS_HomeDirectory },
    { NS_OS_DESKTOP_DIR,           &nsDirectoryService::sOS_DesktopDirectory },
    { NS_XPCOM_INIT_CURRENT_PROCESS_DIR, nsnull },
#if defined (XP_MACOSX)
    { NS_OS_SYSTEM_DIR,                   &nsDirectoryService::sDirectory },
    { NS_MAC_TRASH_DIR,                   &nsDirectoryService::sTrashDirectory },
    { NS_MAC_STARTUP_DIR,                 &nsDirectoryService::sStartupDirectory },
    { NS_MAC_SHUTDOWN_DIR,                &nsDirectoryService::sShutdownDirectory },
    { NS_MAC_APPLE_MENU_DIR,              &nsDirectoryService::sAppleMenuDirectory },
    { NS_MAC_CONTROL_PANELS_DIR,          &nsDirectoryService::sControlPanelDirectory },
    { NS_MAC_EXTENSIONS_DIR,              &nsDirectoryService::sExtensionDirectory },
    { NS_MAC_FONTS_DIR,                   &nsDirectoryService::sFontsDirectory },
    { NS_MAC_PREFS_DIR,                   &nsDirectoryService::sPreferencesDirectory },
    { NS_MAC_DOCUMENTS_DIR,               &nsDirectoryService::sDocumentsDirectory },
    { NS_MAC_INTERNET_SEARCH_DIR,         &nsDirectoryService::sInternetSearchDirectory },
    { NS_MAC_USER_LIB_DIR,                &nsDirectoryService::sUserLibDirectory },
    { NS_OSX_DEFAULT_DOWNLOAD_DIR,        &nsDirectoryService::sDefaultDownloadDirectory },
    { NS_OSX_USER_DESKTOP_DIR,            &nsDirectoryService::sUserDesktopDirectory },
    { NS_OSX_LOCAL_DESKTOP_DIR,           &nsDirectoryService::sLocalDesktopDirectory },
    { NS_OSX_USER_APPLICATIONS_DIR,       &nsDirectoryService::sUserApplicationsDirectory },
    { NS_OSX_LOCAL_APPLICATIONS_DIR,      &nsDirectoryService::sLocalApplicationsDirectory },
    { NS_OSX_USER_DOCUMENTS_DIR,          &nsDirectoryService::sUserDocumentsDirectory },
    { NS_OSX_LOCAL_DOCUMENTS_DIR,         &nsDirectoryService::sLocalDocumentsDirectory },
    { NS_OSX_USER_INTERNET_PLUGIN_DIR,    &nsDirectoryService::sUserInternetPlugInDirectory },
    { NS_OSX_LOCAL_INTERNET_PLUGIN_DIR,   &nsDirectoryService::sLocalInternetPlugInDirectory },
    { NS_OSX_USER_FRAMEWORKS_DIR,         &nsDirectoryService::sUserFrameworksDirectory },
    { NS_OSX_LOCAL_FRAMEWORKS_DIR,        &nsDirectoryService::sLocalFrameworksDirectory },
    { NS_OSX_USER_PREFERENCES_DIR,        &nsDirectoryService::sUserPreferencesDirectory },
    { NS_OSX_LOCAL_PREFERENCES_DIR,       &nsDirectoryService::sLocalPreferencesDirectory },
    { NS_OSX_PICTURE_DOCUMENTS_DIR,       &nsDirectoryService::sPictureDocumentsDirectory },
    { NS_OSX_MOVIE_DOCUMENTS_DIR,         &nsDirectoryService::sMovieDocumentsDirectory },
    { NS_OSX_MUSIC_DOCUMENTS_DIR,         &nsDirectoryService::sMusicDocumentsDirectory },
    { NS_OSX_INTERNET_SITES_DIR,          &nsDirectoryService::sInternetSitesDirectory },
#elif defined (XP_WIN) 
    { NS_OS_SYSTEM_DIR,            &nsDirectoryService::sSystemDirectory },
    { NS_WIN_WINDOWS_DIR,          &nsDirectoryService::sWindowsDirectory },
    { NS_WIN_PROGRAM_FILES_DIR,    &nsDirectoryService::sWindowsProgramFiles },
    { NS_WIN_DESKTOP_DIR,          &nsDirectoryService::sDesktop },
    { NS_WIN_PROGRAMS_DIR,         &nsDirectoryService::sPrograms },
    { NS_WIN_CONTROLS_DIR,         &nsDirectoryService::sControls },
    { NS_WIN_PRINTERS_DIR,         &nsDirectoryService::sPrinters },
    { NS_WIN_PERSONAL_DIR,         &nsDirectoryService::sPersonal },
    { NS_WIN_FAVORITES_DIR,        &nsDirectoryService::sFavorites },
    { NS_WIN_STARTUP_DIR,          &nsDirectoryService::sStartup },
    { NS_WIN_RECENT_DIR,           &nsDirectoryService::sRecent },
    { NS_WIN_SEND_TO_DIR,          &nsDirectoryService::sSendto },
    { NS_WIN_BITBUCKET_DIR,        &nsDirectoryService::sBitbucket },
    { NS_WIN_STARTMENU_DIR,        &nsDirectoryService::sStartmenu },
    { NS_WIN_DESKTOP_DIRECTORY,    &nsDirectoryService::sDesktopdirectory },
    { NS_WIN_DRIVES_DIR,           &nsDirectoryService::sDrives },
    { NS_WIN_NETWORK_DIR,          &nsDirectoryService::sNetwork },
    { NS_WIN_NETHOOD_DIR,          &nsDirectoryService::sNethood },
    { NS_WIN_FONTS_DIR,            &nsDirectoryService::sFonts },
    { NS_WIN_TEMPLATES_DIR,        &nsDirectoryService::sTemplates },
    { NS_WIN_COMMON_STARTMENU_DIR, &nsDirectoryService::sCommon_Startmenu },
    { NS_WIN_COMMON_PROGRAMS_DIR,  &nsDirectoryService::sCommon_Programs },
    { NS_WIN_COMMON_STARTUP_DIR,   &nsDirectoryService::sCommon_Startup },
    { NS_WIN_COMMON_DESKTOP_DIRECTORY, &nsDirectoryService::sCommon_Desktopdirectory },
    { NS_WIN_APPDATA_DIR,          &nsDirectoryService::sAppdata },
    { NS_WIN_LOCAL_APPDATA_DIR,    &nsDirectoryService::sLocalAppdata },
    { NS_WIN_PRINTHOOD,            &nsDirectoryService::sPrinthood },
    { NS_WIN_COOKIES_DIR,          &nsDirectoryService::sWinCookiesDirectory },
    { NS_WIN_DEFAULT_DOWNLOAD_DIR, &nsDirectoryService::sDefaultDownloadDirectory },
#elif defined (XP_UNIX)
    { NS_UNIX_LOCAL_DIR,           &nsDirectoryService::sLocalDirectory },
    { NS_UNIX_LIB_DIR,             &nsDirectoryService::sLibDirectory },
#elif defined (XP_OS2)
    { NS_OS_SYSTEM_DIR,            &nsDirectoryService::sSystemDirectory },
    { NS_OS2_DIR,                  &nsDirectoryService::sOS2Directory },
#elif defined (XP_BEOS)
    { NS_OS_SYSTEM_DIR,            &nsDirectoryService::sSystemDirectory },
    { NS_BEOS_SETTINGS_DIR,        &nsDirectoryService::sSettingsDirectory },
#endif
};    

NS_IMETHODIMP
nsDirectoryService::Init()
{
    NS_NOTREACHED("nsDirectoryService::Init() for internal use only!");
    return NS_OK;
}

nsresult
nsDirectoryService::RealInit()
{
    NS_ASSERTION(!gService, 
                 "nsDirectoryService::RealInit Mustn't initialize twice!");

    nsresult rv;

    nsRefPtr<nsDirectoryService> self = new nsDirectoryService();
    if (!self)
        return NS_ERROR_OUT_OF_MEMORY;

    rv = NS_NewISupportsArray(getter_AddRefs(((nsDirectoryService*) self)->mProviders));
    if (NS_FAILED(rv))
        return rv;

    NS_RegisterStaticAtoms(directory_atoms, NS_ARRAY_LENGTH(directory_atoms));
    
    // Let the list hold the only reference to the provider.
    nsAppFileLocationProvider *defaultProvider = new nsAppFileLocationProvider;
    if (!defaultProvider)
        return NS_ERROR_OUT_OF_MEMORY;
    // AppendElement returns PR_TRUE for success.
    rv = ((nsDirectoryService*) self)->mProviders->AppendElement(defaultProvider) ? NS_OK : NS_ERROR_FAILURE;
    if (NS_FAILED(rv))
        return rv;

    self.swap(gService);
    return NS_OK;
}

PRBool
nsDirectoryService::ReleaseValues(nsHashKey* key, void* data, void* closure)
{
    nsISupports* value = (nsISupports*)data;
    NS_IF_RELEASE(value);
    return PR_TRUE;
}

nsDirectoryService::~nsDirectoryService()
{
}

NS_IMPL_THREADSAFE_ISUPPORTS4(nsDirectoryService, nsIProperties, nsIDirectoryService, nsIDirectoryServiceProvider, nsIDirectoryServiceProvider2)


NS_IMETHODIMP
nsDirectoryService::Undefine(const char* prop)
{
    NS_ENSURE_ARG(prop);

    nsCStringKey key(prop);
    if (!mHashtable.Exists(&key))
        return NS_ERROR_FAILURE;

    mHashtable.Remove (&key);
    return NS_OK;
 }

NS_IMETHODIMP
nsDirectoryService::GetKeys(PRUint32 *count, char ***keys)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

struct FileData
{
  FileData(const char* aProperty,
           const nsIID& aUUID) :
    property(aProperty),
    data(nsnull),
    persistent(PR_TRUE),
    uuid(aUUID) {}
    
  const char*   property;
  nsISupports*  data;
  PRBool        persistent;
  const nsIID&  uuid;
};

static PRBool FindProviderFile(nsISupports* aElement, void *aData)
{
  nsresult rv;
  FileData* fileData = (FileData*)aData;
  if (fileData->uuid.Equals(NS_GET_IID(nsISimpleEnumerator)))
  {
      // Not all providers implement this iface
      nsCOMPtr<nsIDirectoryServiceProvider2> prov2 = do_QueryInterface(aElement);
      if (prov2)
      {
          nsCOMPtr<nsISimpleEnumerator> newFiles;
          rv = prov2->GetFiles(fileData->property, getter_AddRefs(newFiles));
          if (NS_SUCCEEDED(rv) && newFiles) {
              if (fileData->data) {
                  nsCOMPtr<nsISimpleEnumerator> unionFiles;

                  NS_NewUnionEnumerator(getter_AddRefs(unionFiles),
                                        (nsISimpleEnumerator*) fileData->data, newFiles);

                  if (unionFiles)
                      unionFiles.swap(* (nsISimpleEnumerator**) &fileData->data);
              }
              else
              {
                  NS_ADDREF(fileData->data = newFiles);
              }
                  
              fileData->persistent = PR_FALSE; // Enumerators can never be persistent
              return rv == NS_SUCCESS_AGGREGATE_RESULT;
          }
      }
  }
  else
  {
      nsCOMPtr<nsIDirectoryServiceProvider> prov = do_QueryInterface(aElement);
      if (prov)
      {
          rv = prov->GetFile(fileData->property, &fileData->persistent, (nsIFile **)&fileData->data);
          if (NS_SUCCEEDED(rv) && fileData->data)
              return PR_FALSE;
      }
  }

  return PR_TRUE;
}

NS_IMETHODIMP
nsDirectoryService::Get(const char* prop, const nsIID & uuid, void* *result)
{
    NS_ENSURE_ARG(prop);

    nsCStringKey key(prop);
    
    nsCOMPtr<nsISupports> value = dont_AddRef(mHashtable.Get(&key));
    
    if (value)
    {
        nsCOMPtr<nsIFile> cloneFile;
        nsCOMPtr<nsIFile> cachedFile = do_QueryInterface(value);
        NS_ASSERTION(cachedFile, 
                     "nsDirectoryService::Get nsIFile expected");

        cachedFile->Clone(getter_AddRefs(cloneFile));
        return cloneFile->QueryInterface(uuid, result);
    }

    // it is not one of our defaults, lets check any providers
    FileData fileData(prop, uuid);

    mProviders->EnumerateBackwards(FindProviderFile, &fileData);
    if (fileData.data)
    {
        if (fileData.persistent)
        {
            Set(prop, static_cast<nsIFile*>(fileData.data));
        }
        nsresult rv = (fileData.data)->QueryInterface(uuid, result);
        NS_RELEASE(fileData.data);  // addref occurs in FindProviderFile()
        return rv;
    }

    FindProviderFile(static_cast<nsIDirectoryServiceProvider*>(this), &fileData);
    if (fileData.data)
    {
        if (fileData.persistent)
        {
            Set(prop, static_cast<nsIFile*>(fileData.data));
        }
        nsresult rv = (fileData.data)->QueryInterface(uuid, result);
        NS_RELEASE(fileData.data);  // addref occurs in FindProviderFile()
        return rv;
    }

    return NS_ERROR_FAILURE;
}

NS_IMETHODIMP
nsDirectoryService::Set(const char* prop, nsISupports* value)
{
    NS_ENSURE_ARG(prop);

    nsCStringKey key(prop);
    if (mHashtable.Exists(&key) || value == nsnull)
        return NS_ERROR_FAILURE;

    nsCOMPtr<nsIFile> ourFile;
    value->QueryInterface(NS_GET_IID(nsIFile), getter_AddRefs(ourFile));
    if (ourFile)
    {
      nsCOMPtr<nsIFile> cloneFile;
      ourFile->Clone (getter_AddRefs (cloneFile));
      mHashtable.Put(&key, cloneFile);

      return NS_OK;
    }

    return NS_ERROR_FAILURE;   
}

NS_IMETHODIMP
nsDirectoryService::Has(const char *prop, PRBool *_retval)
{
    NS_ENSURE_ARG(prop);

    *_retval = PR_FALSE;
    nsCOMPtr<nsIFile> value;
    nsresult rv = Get(prop, NS_GET_IID(nsIFile), getter_AddRefs(value));
    if (NS_FAILED(rv))
        return NS_OK;
    
    if (value)
    {
        *_retval = PR_TRUE;
    }
    
    return rv;
}

NS_IMETHODIMP
nsDirectoryService::RegisterProvider(nsIDirectoryServiceProvider *prov)
{
    nsresult rv;
    if (!prov)
        return NS_ERROR_FAILURE;
    if (!mProviders)
        return NS_ERROR_NOT_INITIALIZED;

    nsCOMPtr<nsISupports> supports = do_QueryInterface(prov, &rv);
    if (NS_FAILED(rv)) return rv;

    // AppendElement returns PR_TRUE for success.
    return mProviders->AppendElement(supports) ? NS_OK : NS_ERROR_FAILURE;
}

void
nsDirectoryService::RegisterCategoryProviders()
{
    nsCOMPtr<nsICategoryManager> catman
        (do_GetService(NS_CATEGORYMANAGER_CONTRACTID));
    if (!catman)
        return;

    nsCOMPtr<nsISimpleEnumerator> entries;
    catman->EnumerateCategory(XPCOM_DIRECTORY_PROVIDER_CATEGORY,
                              getter_AddRefs(entries));

    nsCOMPtr<nsIUTF8StringEnumerator> strings(do_QueryInterface(entries));
    if (!strings)
        return;

    PRBool more;
    while (NS_SUCCEEDED(strings->HasMore(&more)) && more) {
        nsCAutoString entry;
        strings->GetNext(entry);

        nsXPIDLCString contractID;
        catman->GetCategoryEntry(XPCOM_DIRECTORY_PROVIDER_CATEGORY, entry.get(), getter_Copies(contractID));

        if (contractID) {
            nsCOMPtr<nsIDirectoryServiceProvider> provider = do_GetService(contractID.get());
            if (provider)
                RegisterProvider(provider);
        }
    }
}

NS_IMETHODIMP
nsDirectoryService::UnregisterProvider(nsIDirectoryServiceProvider *prov)
{
    nsresult rv;
    if (!prov)
        return NS_ERROR_FAILURE;
    if (!mProviders)
        return NS_ERROR_NOT_INITIALIZED;

    nsCOMPtr<nsISupports> supports = do_QueryInterface(prov, &rv);
    if (NS_FAILED(rv)) return rv;

    // RemoveElement returns PR_TRUE for success.
    return mProviders->RemoveElement(supports) ? NS_OK : NS_ERROR_FAILURE;
}

// DO NOT ADD ANY LOCATIONS TO THIS FUNCTION UNTIL YOU TALK TO: dougt@netscape.com.
// This is meant to be a place of xpcom or system specific file locations, not
// application specific locations.  If you need the later, register a callback for
// your application.  

NS_IMETHODIMP
nsDirectoryService::GetFile(const char *prop, PRBool *persistent, nsIFile **_retval)
{
    nsCOMPtr<nsILocalFile> localFile;
    nsresult rv = NS_ERROR_FAILURE;

    *_retval = nsnull;
    *persistent = PR_TRUE;

    nsIAtom* inAtom = NS_NewAtom(prop);

    // check to see if it is one of our defaults
        
    if (inAtom == nsDirectoryService::sCurrentProcess || 
        inAtom == nsDirectoryService::sOS_CurrentProcessDirectory )
    {
        rv = GetCurrentProcessDirectory(getter_AddRefs(localFile));
    }
    else if (inAtom == nsDirectoryService::sComponentRegistry)
    {
        rv = GetCurrentProcessDirectory(getter_AddRefs(localFile));
        if (!localFile)
            return NS_ERROR_FAILURE;

        localFile->AppendNative(COMPONENT_DIRECTORY);           
        localFile->AppendNative(COMPONENT_REGISTRY_NAME);           
    }
    else if (inAtom == nsDirectoryService::sXPTIRegistry)
    {
        rv = GetCurrentProcessDirectory(getter_AddRefs(localFile));
        if (!localFile)
            return NS_ERROR_FAILURE;

        localFile->AppendNative(COMPONENT_DIRECTORY);           
        localFile->AppendNative(XPTI_REGISTRY_NAME);           
    }
    
    // Unless otherwise set, the core pieces of the GRE exist
    // in the current process directory.
    else if (inAtom == nsDirectoryService::sGRE_Directory)
    {
        rv = GetCurrentProcessDirectory(getter_AddRefs(localFile));
    }
    // the GRE components directory is relative to the GRE directory
    // by default; applications may override this behavior in special
    // cases
    else if (inAtom == nsDirectoryService::sGRE_ComponentDirectory)
    {
        rv = Get(NS_GRE_DIR, NS_GET_IID(nsILocalFile), getter_AddRefs(localFile));
        if (localFile)
             localFile->AppendNative(COMPONENT_DIRECTORY);
    }
    else if (inAtom == nsDirectoryService::sComponentDirectory)
    {
        rv = GetCurrentProcessDirectory(getter_AddRefs(localFile));
        if (localFile)
            localFile->AppendNative(COMPONENT_DIRECTORY);           
    }
    else if (inAtom == nsDirectoryService::sOS_DriveDirectory)
    {
        rv = GetSpecialSystemDirectory(OS_DriveDirectory, getter_AddRefs(localFile));
    }
    else if (inAtom == nsDirectoryService::sOS_TemporaryDirectory)
    {
        rv = GetSpecialSystemDirectory(OS_TemporaryDirectory, getter_AddRefs(localFile));
    }
    else if (inAtom == nsDirectoryService::sOS_CurrentProcessDirectory)
    {
        rv = GetSpecialSystemDirectory(OS_CurrentProcessDirectory, getter_AddRefs(localFile)); 
    }
    else if (inAtom == nsDirectoryService::sOS_CurrentWorkingDirectory)
    {
        rv = GetSpecialSystemDirectory(OS_CurrentWorkingDirectory, getter_AddRefs(localFile)); 
    }
       
#if defined(XP_MACOSX)
    else if (inAtom == nsDirectoryService::sDirectory)
    {
        rv = GetOSXFolderType(kClassicDomain, kSystemFolderType, getter_AddRefs(localFile));
    }
    else if (inAtom == nsDirectoryService::sTrashDirectory)
    {
        rv = GetOSXFolderType(kClassicDomain, kTrashFolderType, getter_AddRefs(localFile));
    }
    else if (inAtom == nsDirectoryService::sStartupDirectory)
    {
        rv = GetOSXFolderType(kClassicDomain, kStartupFolderType, getter_AddRefs(localFile));
    }
    else if (inAtom == nsDirectoryService::sShutdownDirectory)
    {
        rv = GetOSXFolderType(kClassicDomain, kShutdownFolderType, getter_AddRefs(localFile));
    }
    else if (inAtom == nsDirectoryService::sAppleMenuDirectory)
    {
        rv = GetOSXFolderType(kClassicDomain, kAppleMenuFolderType, getter_AddRefs(localFile));
    }
    else if (inAtom == nsDirectoryService::sControlPanelDirectory)
    {
        rv = GetOSXFolderType(kClassicDomain, kControlPanelFolderType, getter_AddRefs(localFile));
    }
    else if (inAtom == nsDirectoryService::sExtensionDirectory)
    {
        rv = GetOSXFolderType(kClassicDomain, kExtensionFolderType, getter_AddRefs(localFile));
    }
    else if (inAtom == nsDirectoryService::sFontsDirectory)
    {
        rv = GetOSXFolderType(kClassicDomain, kFontsFolderType, getter_AddRefs(localFile));
    }
    else if (inAtom == nsDirectoryService::sPreferencesDirectory)
    {
        rv = GetOSXFolderType(kClassicDomain, kPreferencesFolderType, getter_AddRefs(localFile));
    }
    else if (inAtom == nsDirectoryService::sDocumentsDirectory)
    {
        rv = GetOSXFolderType(kClassicDomain, kDocumentsFolderType, getter_AddRefs(localFile));
    }
    else if (inAtom == nsDirectoryService::sInternetSearchDirectory)
    {
        rv = GetOSXFolderType(kClassicDomain, kInternetSearchSitesFolderType, getter_AddRefs(localFile));
    }   
    else if (inAtom == nsDirectoryService::sUserLibDirectory)
    {
        rv = GetOSXFolderType(kUserDomain, kDomainLibraryFolderType, getter_AddRefs(localFile));
    }
    else if (inAtom == nsDirectoryService::sOS_HomeDirectory)
    {
        rv = GetOSXFolderType(kUserDomain, kDomainTopLevelFolderType, getter_AddRefs(localFile));
    }
    else if (inAtom == nsDirectoryService::sDefaultDownloadDirectory)
    {
        NS_NewLocalFile(EmptyString(), PR_TRUE, getter_AddRefs(localFile));
        nsCOMPtr<nsILocalFileMac> localMacFile(do_QueryInterface(localFile));

        if (localMacFile)
        {
            OSErr err;
            ICInstance icInstance;

            err = ::ICStart(&icInstance, 'XPCM');
            if (err == noErr)
            {
                // ICGetPref() crashes when getting the download directory if the download
                // directory has never been specified (e.g. a new user account), bug 265903.
                // To work around this we enumerate through the IC prefs to see if the
                // download directory has been specified before trying to obtain it.
                long numPrefs = 0;
                err = ::ICCountPref(icInstance, &numPrefs);
                if (err == noErr)
                {
                    for ( long i = 0; i < numPrefs; ++i )
                    {
                        Str255 key;
                        err = ::ICGetIndPref(icInstance, i, key);
                        if ( err == noErr && ( PLstrcmp( key, kICDownloadFolder ) == 0 ) )
                        {
                            ICAttr attrs;
                            ICFileSpec icFileSpec;
                            long size = kICFileSpecHeaderSize;
                            err = ::ICGetPref(icInstance, kICDownloadFolder, &attrs, &icFileSpec, &size);
                            if (err == noErr || (err == icTruncatedErr && size >= kICFileSpecHeaderSize))
                            {
                                rv = localMacFile->InitWithFSSpec(&icFileSpec.fss);
                            }
                            break;
                        }
                    }
                }
                ::ICStop(icInstance);
            }
            
            if (NS_FAILED(rv))
            { 
                // We got an error getting the DL folder from IC so try finding the user's Desktop folder
                rv = GetOSXFolderType(kUserDomain, kDesktopFolderType, getter_AddRefs(localFile));
            }
        }
        
        // Don't cache the DL directory as the user may change it while we're running.
        // Negligible perf hit as this directory is only requested for downloads
        *persistent = PR_FALSE;
    }
    else if (inAtom == nsDirectoryService::sUserDesktopDirectory ||
             inAtom == nsDirectoryService::sOS_DesktopDirectory)
    {
        rv = GetOSXFolderType(kUserDomain, kDesktopFolderType, getter_AddRefs(localFile));
    }
    else if (inAtom == nsDirectoryService::sLocalDesktopDirectory)
    {
        rv = GetOSXFolderType(kLocalDomain, kDesktopFolderType, getter_AddRefs(localFile));
    }
    else if (inAtom == nsDirectoryService::sUserApplicationsDirectory)
    {
        rv = GetOSXFolderType(kUserDomain, kApplicationsFolderType, getter_AddRefs(localFile));
    }
    else if (inAtom == nsDirectoryService::sLocalApplicationsDirectory)
    {
        rv = GetOSXFolderType(kLocalDomain, kApplicationsFolderType, getter_AddRefs(localFile));
    }
    else if (inAtom == nsDirectoryService::sUserDocumentsDirectory)
    {
        rv = GetOSXFolderType(kUserDomain, kDocumentsFolderType, getter_AddRefs(localFile));
    }
    else if (inAtom == nsDirectoryService::sLocalDocumentsDirectory)
    {
        rv = GetOSXFolderType(kLocalDomain, kDocumentsFolderType, getter_AddRefs(localFile));
    }
    else if (inAtom == nsDirectoryService::sUserInternetPlugInDirectory)
    {
        rv = GetOSXFolderType(kUserDomain, kInternetPlugInFolderType, getter_AddRefs(localFile));
    }
    else if (inAtom == nsDirectoryService::sLocalInternetPlugInDirectory)
    {
        rv = GetOSXFolderType(kLocalDomain, kInternetPlugInFolderType, getter_AddRefs(localFile));
    }
    else if (inAtom == nsDirectoryService::sUserFrameworksDirectory)
    {
        rv = GetOSXFolderType(kUserDomain, kFrameworksFolderType, getter_AddRefs(localFile));
    }
    else if (inAtom == nsDirectoryService::sLocalFrameworksDirectory)
    {
        rv = GetOSXFolderType(kLocalDomain, kFrameworksFolderType, getter_AddRefs(localFile));
    }
    else if (inAtom == nsDirectoryService::sUserPreferencesDirectory)
    {
        rv = GetOSXFolderType(kUserDomain, kPreferencesFolderType, getter_AddRefs(localFile));
    }
    else if (inAtom == nsDirectoryService::sLocalPreferencesDirectory)
    {
        rv = GetOSXFolderType(kLocalDomain, kPreferencesFolderType, getter_AddRefs(localFile));
    }
    else if (inAtom == nsDirectoryService::sPictureDocumentsDirectory)
    {
        rv = GetOSXFolderType(kUserDomain, kPictureDocumentsFolderType, getter_AddRefs(localFile));
    }
    else if (inAtom == nsDirectoryService::sMovieDocumentsDirectory)
    {
        rv = GetOSXFolderType(kUserDomain, kMovieDocumentsFolderType, getter_AddRefs(localFile));
    }
    else if (inAtom == nsDirectoryService::sMusicDocumentsDirectory)
    {
        rv = GetOSXFolderType(kUserDomain, kMusicDocumentsFolderType, getter_AddRefs(localFile));
    }
    else if (inAtom == nsDirectoryService::sInternetSitesDirectory)
    {
        rv = GetOSXFolderType(kUserDomain, kInternetSitesFolderType, getter_AddRefs(localFile));
    }
#elif defined (XP_WIN)
    else if (inAtom == nsDirectoryService::sSystemDirectory)
    {
        rv = GetSpecialSystemDirectory(Win_SystemDirectory, getter_AddRefs(localFile)); 
    }
    else if (inAtom == nsDirectoryService::sWindowsDirectory)
    {
        rv = GetSpecialSystemDirectory(Win_WindowsDirectory, getter_AddRefs(localFile)); 
    }
    else if (inAtom == nsDirectoryService::sWindowsProgramFiles)
    {
        rv = GetSpecialSystemDirectory(Win_ProgramFiles, getter_AddRefs(localFile));
    }
    else if (inAtom == nsDirectoryService::sOS_HomeDirectory)
    {
        rv = GetSpecialSystemDirectory(Win_HomeDirectory, getter_AddRefs(localFile)); 
    }
    else if (inAtom == nsDirectoryService::sDesktop)
    {
        rv = GetSpecialSystemDirectory(Win_Desktop, getter_AddRefs(localFile)); 
    }
    else if (inAtom == nsDirectoryService::sPrograms)
    {
        rv = GetSpecialSystemDirectory(Win_Programs, getter_AddRefs(localFile)); 
    }
    else if (inAtom == nsDirectoryService::sControls)
    {
        rv = GetSpecialSystemDirectory(Win_Controls, getter_AddRefs(localFile)); 
    }
    else if (inAtom == nsDirectoryService::sPrinters)
    {
        rv = GetSpecialSystemDirectory(Win_Printers, getter_AddRefs(localFile)); 
    }
    else if (inAtom == nsDirectoryService::sPersonal)
    {
        rv = GetSpecialSystemDirectory(Win_Personal, getter_AddRefs(localFile)); 
    }
    else if (inAtom == nsDirectoryService::sFavorites)
    {
        rv = GetSpecialSystemDirectory(Win_Favorites, getter_AddRefs(localFile)); 
    }
    else if (inAtom == nsDirectoryService::sStartup)
    {
        rv = GetSpecialSystemDirectory(Win_Startup, getter_AddRefs(localFile));
    }
    else if (inAtom == nsDirectoryService::sRecent)
    {
        rv = GetSpecialSystemDirectory(Win_Recent, getter_AddRefs(localFile)); 
    }
    else if (inAtom == nsDirectoryService::sSendto)
    {
        rv = GetSpecialSystemDirectory(Win_Sendto, getter_AddRefs(localFile)); 
    }
    else if (inAtom == nsDirectoryService::sBitbucket)
    {
        rv = GetSpecialSystemDirectory(Win_Bitbucket, getter_AddRefs(localFile)); 
    }
    else if (inAtom == nsDirectoryService::sStartmenu)
    {
        rv = GetSpecialSystemDirectory(Win_Startmenu, getter_AddRefs(localFile)); 
    }
    else if (inAtom == nsDirectoryService::sDesktopdirectory ||
             inAtom == nsDirectoryService::sOS_DesktopDirectory)
    {
        rv = GetSpecialSystemDirectory(Win_Desktopdirectory, getter_AddRefs(localFile)); 
    }
    else if (inAtom == nsDirectoryService::sDrives)
    {
        rv = GetSpecialSystemDirectory(Win_Drives, getter_AddRefs(localFile));
    }
    else if (inAtom == nsDirectoryService::sNetwork)
    {
        rv = GetSpecialSystemDirectory(Win_Network, getter_AddRefs(localFile)); 
    }
    else if (inAtom == nsDirectoryService::sNethood)
    {
        rv = GetSpecialSystemDirectory(Win_Nethood, getter_AddRefs(localFile)); 
    }
    else if (inAtom == nsDirectoryService::sFonts)
    {
        rv = GetSpecialSystemDirectory(Win_Fonts, getter_AddRefs(localFile));
    }
    else if (inAtom == nsDirectoryService::sTemplates)
    {
        rv = GetSpecialSystemDirectory(Win_Templates, getter_AddRefs(localFile)); 
    }
    else if (inAtom == nsDirectoryService::sCommon_Startmenu)
    {
        rv = GetSpecialSystemDirectory(Win_Common_Startmenu, getter_AddRefs(localFile)); 
    }
    else if (inAtom == nsDirectoryService::sCommon_Programs)
    {
        rv = GetSpecialSystemDirectory(Win_Common_Programs, getter_AddRefs(localFile)); 
    }
    else if (inAtom == nsDirectoryService::sCommon_Startup)
    {
        rv = GetSpecialSystemDirectory(Win_Common_Startup, getter_AddRefs(localFile)); 
    }
    else if (inAtom == nsDirectoryService::sCommon_Desktopdirectory)
    {
        rv = GetSpecialSystemDirectory(Win_Common_Desktopdirectory, getter_AddRefs(localFile)); 
    }
    else if (inAtom == nsDirectoryService::sAppdata)
    {
        rv = GetSpecialSystemDirectory(Win_Appdata, getter_AddRefs(localFile)); 
    }
    else if (inAtom == nsDirectoryService::sLocalAppdata)
    {
        rv = GetSpecialSystemDirectory(Win_LocalAppdata, getter_AddRefs(localFile)); 
    }
    else if (inAtom == nsDirectoryService::sPrinthood)
    {
        rv = GetSpecialSystemDirectory(Win_Printhood, getter_AddRefs(localFile)); 
    }
    else if (inAtom == nsDirectoryService::sWinCookiesDirectory)
    {
        rv = GetSpecialSystemDirectory(Win_Cookies, getter_AddRefs(localFile)); 
    }
    else if (inAtom == nsDirectoryService::sDefaultDownloadDirectory)
    {
        rv = GetSpecialSystemDirectory(Win_Downloads, getter_AddRefs(localFile));
    }
#elif defined (XP_UNIX)

    else if (inAtom == nsDirectoryService::sLocalDirectory)
    {
        rv = GetSpecialSystemDirectory(Unix_LocalDirectory, getter_AddRefs(localFile)); 
    }
    else if (inAtom == nsDirectoryService::sLibDirectory)
    {
        rv = GetSpecialSystemDirectory(Unix_LibDirectory, getter_AddRefs(localFile));
    }
    else if (inAtom == nsDirectoryService::sOS_HomeDirectory)
    {
        rv = GetSpecialSystemDirectory(Unix_HomeDirectory, getter_AddRefs(localFile)); 
    }
    else if (inAtom == nsDirectoryService::sOS_DesktopDirectory)
    {
        rv = GetSpecialSystemDirectory(Unix_DesktopDirectory, getter_AddRefs(localFile)); 
    }
#elif defined (XP_OS2)
    else if (inAtom == nsDirectoryService::sSystemDirectory)
    {
        rv = GetSpecialSystemDirectory(OS2_SystemDirectory, getter_AddRefs(localFile)); 
    }
    else if (inAtom == nsDirectoryService::sOS2Directory)
    {
        rv = GetSpecialSystemDirectory(OS2_OS2Directory, getter_AddRefs(localFile)); 
    }
    else if (inAtom == nsDirectoryService::sOS_HomeDirectory)
    {
        rv = GetSpecialSystemDirectory(OS2_HomeDirectory, getter_AddRefs(localFile)); 
    }
    else if (inAtom == nsDirectoryService::sOS_DesktopDirectory)
    {
        rv = GetSpecialSystemDirectory(OS2_DesktopDirectory, getter_AddRefs(localFile)); 
    }
#elif defined (XP_BEOS)
    else if (inAtom == nsDirectoryService::sSettingsDirectory)
    {
        rv = GetSpecialSystemDirectory(BeOS_SettingsDirectory, getter_AddRefs(localFile)); 
    }
    else if (inAtom == nsDirectoryService::sOS_HomeDirectory)
    {
        rv = GetSpecialSystemDirectory(BeOS_HomeDirectory, getter_AddRefs(localFile));
    }
    else if (inAtom == nsDirectoryService::sOS_DesktopDirectory)
    {
        rv = GetSpecialSystemDirectory(BeOS_DesktopDirectory, getter_AddRefs(localFile)); 
    }
    else if (inAtom == nsDirectoryService::sSystemDirectory)
    {
        rv = GetSpecialSystemDirectory(BeOS_SystemDirectory, getter_AddRefs(localFile)); 
    }
#endif


    NS_RELEASE(inAtom);

    if (localFile && NS_SUCCEEDED(rv))
        return localFile->QueryInterface(NS_GET_IID(nsIFile), (void**)_retval);
#ifdef DEBUG_dougt
    printf("Failed to find directory for key: %s\n", prop);
#endif
    return rv;
}

NS_IMETHODIMP
nsDirectoryService::GetFiles(const char *prop, nsISimpleEnumerator **_retval)
{
    NS_ENSURE_ARG_POINTER(_retval);
    *_retval = nsnull;
        
    return NS_ERROR_FAILURE;
}
