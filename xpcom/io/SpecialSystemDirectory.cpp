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
 * The Original Code is Mozilla Communicator client code, released
 * March 31, 1998.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Doug Turner <dougt@netscape.com>
 *   IBM Corp.
 *   Fredrik Holmqvist <thesuckiestemail@yahoo.se>
 *   Jungshik Shin <jshin@i18nl10n.com>
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

#include "SpecialSystemDirectory.h"
#include "nsString.h"
#include "nsDependentString.h"

#if defined(XP_WIN)

#include "nsNativeCharsetUtils.h"
#include "nsWinAPIs.h"
#include <windows.h>
#include <shlobj.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <direct.h>

// These are not defined by VC6. 
#ifndef CSIDL_LOCAL_APPDATA
#define CSIDL_LOCAL_APPDATA             0x001C
#endif
#ifndef CSIDL_PROGRAM_FILES
#define CSIDL_PROGRAM_FILES             0x0026
#endif

#elif defined(XP_OS2)

#define MAX_PATH _MAX_PATH
#define INCL_WINWORKPLACE
#define INCL_DOSMISC
#define INCL_DOSMODULEMGR
#define INCL_DOSPROCESS
#define INCL_WINSHELLDATA
#include <os2.h>
#include <stdlib.h>
#include <stdio.h>
#include "prenv.h"

#elif defined(XP_UNIX)

#include <unistd.h>
#include <stdlib.h>
#include <sys/param.h>
#include "prenv.h"

#elif defined(XP_BEOS)

#include <FindDirectory.h>
#include <fs_info.h>
#include <Path.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/param.h>
#include <OS.h>
#include <image.h>
#include "prenv.h"

#endif

#if defined(VMS)
#include <unixlib.h>
#endif

#ifndef MAXPATHLEN
#ifdef MAX_PATH
#define MAXPATHLEN MAX_PATH
#elif defined(_MAX_PATH)
#define MAXPATHLEN _MAX_PATH
#elif defined(CCHMAXPATH)
#define MAXPATHLEN CCHMAXPATH
#else
#define MAXPATHLEN 1024
#endif
#endif

#if defined (XP_WIN)
typedef BOOL (WINAPI * GetSpecialPathProc) (HWND hwndOwner, LPSTR lpszPath,
                                            int nFolder, BOOL fCreate);
typedef BOOL (WINAPI * nsGetSpecialFolderPathW) (HWND hwndOwner, 
                                                 LPWSTR lpszPath,
                                                 int nFolder, BOOL fCreate);
typedef BOOL (WINAPI * nsGetSpecialFolderPathA) (HWND hwndOwner,
                                                 LPSTR lpszPath, int nFolder,
                                                 BOOL fCreate);


static GetSpecialPathProc gGetSpecialPathProc = NULL;
static nsGetSpecialFolderPathA gGetSpecialFolderPathA = NULL;
static nsGetSpecialFolderPathW gGetSpecialFolderPath  = NULL;

static HINSTANCE gShell32DLLInst = NULL;

static BOOL WINAPI NS_GetSpecialFolderPath(HWND hwndOwner, LPWSTR aPath,
                                           int aFolder, BOOL fCreate);
#endif
NS_COM void StartupSpecialSystemDirectory()
{
#if defined (XP_WIN) && !defined (WINCE)
    /* On windows, the old method to get file locations is incredibly slow.
       As of this writing, 3 calls to GetWindowsFolder accounts for 3% of mozilla
       startup. Replacing these older calls with a single call to SHGetSpecialFolderPath
       effectively removes these calls from the performace radar.  We need to 
       support the older way of file location lookup on systems that do not have
       IE4. 
    */ 
    gShell32DLLInst = LoadLibrary("Shell32.dll");
    if(gShell32DLLInst)
    {
        if (NS_UseUnicode())
        {
            gGetSpecialFolderPath = (nsGetSpecialFolderPathW) 
                GetProcAddress(gShell32DLLInst, "SHGetSpecialFolderPathW");
        }
        else 
        {
            gGetSpecialFolderPathA = (nsGetSpecialFolderPathA)
                GetProcAddress(gShell32DLLInst, "SHGetSpecialFolderPathA");
            // need to check because it's not available on Win95 without IE.
            if (gGetSpecialFolderPathA)
                gGetSpecialFolderPath = NS_GetSpecialFolderPath;
        }
    }
#endif
}

NS_COM void ShutdownSpecialSystemDirectory()
{
#if defined (XP_WIN)
    if (gShell32DLLInst)
    {
        FreeLibrary(gShell32DLLInst);
        gShell32DLLInst = NULL;
        gGetSpecialFolderPath = NULL;
    }
#endif
}

#if defined (XP_WIN)

//----------------------------------------------------------------------------------------
static nsresult GetWindowsFolder(int folder, nsILocalFile** aFile)
//----------------------------------------------------------------------------------------
{
    if (gGetSpecialFolderPath) { // With MS IE 4.0 or higher
        WCHAR path[MAX_PATH + 2];
        HRESULT result = gGetSpecialFolderPath(NULL, path, folder, true);
        
        if (!SUCCEEDED(result)) 
            return NS_ERROR_FAILURE;

        // Append the trailing slash
        int len = wcslen(path);
        if (len > 1 && path[len - 1] != L'\\') 
        {
            path[len]   = L'\\';
            path[++len] = L'\0';
        }

        return NS_NewLocalFile(nsDependentString(path, len), PR_TRUE, aFile);
    }

    nsresult rv = NS_ERROR_FAILURE;
    LPMALLOC pMalloc = NULL;
    LPWSTR pBuffer = NULL;
    LPITEMIDLIST pItemIDList = NULL;
    int len;
 
    // Get the shell's allocator. 
    if (!SUCCEEDED(SHGetMalloc(&pMalloc))) 
        return NS_ERROR_FAILURE;

    // Allocate a buffer
    if ((pBuffer = (LPWSTR) pMalloc->Alloc(MAX_PATH + 2)) == NULL)
        return NS_ERROR_OUT_OF_MEMORY;
 
    // Get the PIDL for the folder. 
    if (!SUCCEEDED(SHGetSpecialFolderLocation(NULL, folder, &pItemIDList)))
        goto Clean;
 
    if (!SUCCEEDED(nsWinAPIs::mSHGetPathFromIDList(pItemIDList, pBuffer)))
        goto Clean;

    // Append the trailing slash
    len = wcslen(pBuffer);
    pBuffer[len] = L'\\';
    pBuffer[++len] = L'\0';

    // Assign the directory
    rv = NS_NewLocalFile(nsDependentString(pBuffer, len), 
                         PR_TRUE, 
                         aFile);

Clean:
    // Clean up. 
    if (pItemIDList)
        pMalloc->Free(pItemIDList); 
    if (pBuffer)
        pMalloc->Free(pBuffer); 

	pMalloc->Release();
    
    return rv;
} 

// Assume that this function is always invoked with aPath with the capacity
// no smaller than MAX_PATH. It's possible because it's only referred to in 
// this file and we have made sure that they're indeed.

static BOOL WINAPI NS_GetSpecialFolderPath(HWND hwndOwner,
                                           LPWSTR aPath, 
                                           int aFolder, BOOL fCreate)
{
    char path[MAX_PATH];
    if (gGetSpecialFolderPathA(hwndOwner, path, aFolder, fCreate))
    {
        if (NS_ConvertAtoW(path, 0, aPath) > MAX_PATH)
        {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return FALSE;
        }
        return NS_ConvertAtoW(path, MAX_PATH, aPath) ? TRUE : FALSE;
    }
    return FALSE;
}

#endif // XP_WIN

#if defined (XP_BEOS)                                            
static nsresult
GetBeOSFolder( directory_which which, dev_t volume, nsILocalFile** aFile)
{
    char path[MAXPATHLEN];
    if (volume < 0)
        return NS_ERROR_FAILURE;
        
    status_t result = find_directory(which, volume, false, path, MAXPATHLEN - 2);
    if (result != B_OK)
        return NS_ERROR_FAILURE;
        
    int len = strlen(path);
    if (len == 0)
        return NS_ERROR_FAILURE;

    if (path[len-1] != '/') 
    {
        path[len]   = '/';
        path[len+1] = '\0';            
    }
    return NS_NewNativeLocalFile(nsDependentCString(path), PR_TRUE, aFile);
}
#endif // XP_BEOS

#if defined(XP_UNIX)
static nsresult
GetUnixHomeDir(nsILocalFile** aFile)
{
#ifdef VMS
    char *pHome;
    pHome = getenv("HOME");
    if (*pHome == '/') {
        return NS_NewNativeLocalFile(nsDependentCString(pHome), 
                                     PR_TRUE, 
                                     aFile);
    } else {
        return NS_NewNativeLocalFile(nsDependentCString(decc$translate_vms(pHome)), 
                                     PR_TRUE, 
                                     aFile);
    }
#else
    return NS_NewNativeLocalFile(nsDependentCString(PR_GetEnv("HOME")), 
                                 PR_TRUE, aFile);
#endif
}
#endif

nsresult
GetSpecialSystemDirectory(SystemDirectories aSystemSystemDirectory,
                          nsILocalFile** aFile)
{
#if defined(XP_WIN)
    WCHAR path[MAX_PATH];
#else
    char path[MAXPATHLEN];
#endif

    switch (aSystemSystemDirectory)
    {
        case OS_CurrentWorkingDirectory:
#if defined(XP_WIN)
            if (!nsWinAPIs::mGetCwd(path, MAX_PATH))
                return NS_ERROR_FAILURE;
            return NS_NewLocalFile(nsDependentString(path), 
                                   PR_TRUE, 
                                   aFile);
#elif defined(XP_OS2)
            if (DosQueryPathInfo( ".", FIL_QUERYFULLNAME, path, MAXPATHLEN))
                return NS_ERROR_FAILURE;
#else
            if(!getcwd(path, MAXPATHLEN))
                return NS_ERROR_FAILURE;
#endif

#if !defined(XP_WIN)
            return NS_NewNativeLocalFile(nsDependentCString(path), 
                                         PR_TRUE, 
                                         aFile);
#endif

        case OS_DriveDirectory:
#if defined (XP_WIN)
        {
            PRInt32 len = nsWinAPIs::mGetWindowsDirectory(path, MAX_PATH);
            if (len == 0)
                break;
            if (path[1] == PRUnichar(':') && path[2] == PRUnichar('\\'))
                path[3] = 0;

            return NS_NewLocalFile(nsDependentString(path),
                                   PR_TRUE, 
                                   aFile);
        }
#elif defined(XP_OS2)
        {
            ULONG ulBootDrive = 0;
            char  buffer[] = " :\\OS2\\";
            DosQuerySysInfo( QSV_BOOT_DRIVE, QSV_BOOT_DRIVE,
                             &ulBootDrive, sizeof ulBootDrive);
            buffer[0] = 'A' - 1 + ulBootDrive; // duh, 1-based index...

            return NS_NewNativeLocalFile(nsDependentCString(buffer),
                                         PR_TRUE, 
                                         aFile);
        }
#else
        return NS_NewNativeLocalFile(nsDependentCString("/"), 
                                     PR_TRUE, 
                                     aFile);

#endif
            
        case OS_TemporaryDirectory:
#if defined (XP_WIN) && !defined (WINCE)
        {
            DWORD len = nsWinAPIs::mGetTempPath(MAX_PATH, path);
            if (len == 0)
                break;
            return NS_NewLocalFile(nsDependentString(path, len), 
                                   PR_TRUE, 
                                   aFile);
        }
#elif defined (WINCE)
        {
            return NS_NewNativeLocalFile(NS_LITERAL_CSTRING("\\Temp"), 
                                         PR_TRUE, 
                                         aFile);
        }
#elif defined(XP_OS2)
        {
            char buffer[CCHMAXPATH] = "";
            char *c = getenv( "TMP");
            if( c) strcpy( buffer, c);
            else
            {
                c = getenv( "TEMP");
                if( c) strcpy( buffer, c);
            }

            return NS_NewNativeLocalFile(nsDependentCString(buffer), 
                                         PR_TRUE, 
                                         aFile);
        } 
#elif defined(XP_MACOSX)
        {
            return GetOSXFolderType(kUserDomain, kTemporaryFolderType, aFile);
        }

#elif defined(XP_UNIX) || defined(XP_BEOS)
        {
            static const char *tPath = nsnull;
            if (!tPath) {
                tPath = PR_GetEnv("TMPDIR");
                if (!tPath || !*tPath) {
                    tPath = PR_GetEnv("TMP");
                    if (!tPath || !*tPath) {
                        tPath = PR_GetEnv("TEMP");
                        if (!tPath || !*tPath) {
                            tPath = "/tmp/";
                        }
                    }
                }
            }
            return NS_NewNativeLocalFile(nsDependentCString(tPath), 
                                         PR_TRUE, 
                                         aFile);
        }
#else
        break;
#endif            
#if defined (XP_WIN)
        case Win_SystemDirectory:
        {    
            PRInt32 len = nsWinAPIs::mGetSystemDirectory(path, MAX_PATH);
        
            // Need enough space to add the trailing backslash
            if (!len || len > MAX_PATH - 2)
                break;
            path[len]   = L'\\';
            path[++len] = L'\0';

            return NS_NewLocalFile(nsDependentString(path, len), 
                                   PR_TRUE, 
                                   aFile);
        }

        case Win_WindowsDirectory:
        {    
            PRInt32 len = nsWinAPIs::mGetWindowsDirectory(path, MAX_PATH);
            
            // Need enough space to add the trailing backslash
            if (!len || len > MAX_PATH - 2)
                break;
            
            path[len]   = L'\\';
            path[++len] = L'\0';

            return NS_NewLocalFile(nsDependentString(path, len), 
                                   PR_TRUE, 
                                   aFile);
        }

        case Win_ProgramFiles:
        {
            return GetWindowsFolder(CSIDL_PROGRAM_FILES, aFile);
        }

        case Win_HomeDirectory:
        {    
            PRInt32 len;
            if ((len = 
                 nsWinAPIs::mGetEnvironmentVariable(L"HOME", path, 
                                                    MAX_PATH)) > 0)
            {
                // Need enough space to add the trailing backslash
                if (len > MAX_PATH - 2)
                    break;
               
                path[len]   = L'\\';
                path[++len] = L'\0';

                return NS_NewLocalFile(nsDependentString(path, len), 
                                       PR_TRUE, 
                                       aFile);
            }

            len = nsWinAPIs::mGetEnvironmentVariable(L"HOMEDRIVE", 
                                                     path, MAX_PATH);
            if (0 < len && len < MAX_PATH)
            {
                WCHAR temp[MAX_PATH];
                DWORD len2 = nsWinAPIs::mGetEnvironmentVariable(L"HOMEPATH", 
                                                                temp,
                                                                MAX_PATH);
                if (0 < len2 && len + len2 < MAX_PATH)
                    wcsncat(path, temp, len2);
        
                len = wcslen(path);

                // Need enough space to add the trailing backslash
                if (len > MAX_PATH - 2)
                    break;
            
                path[len]   = L'\\';
                path[++len] = L'\0';
                
                return NS_NewLocalFile(nsDependentString(path, len), 
                                       PR_TRUE, 
                                       aFile);
            }
        }
        case Win_Desktop:
        {
            return GetWindowsFolder(CSIDL_DESKTOP, aFile);
        }
        case Win_Programs:
        {
            return GetWindowsFolder(CSIDL_PROGRAMS, aFile);
        }
        case Win_Controls:
        {
            return GetWindowsFolder(CSIDL_CONTROLS, aFile);
        }
        case Win_Printers:
        {
            return GetWindowsFolder(CSIDL_PRINTERS, aFile);
        }
        case Win_Personal:
        {
            return GetWindowsFolder(CSIDL_PERSONAL, aFile);
        }
        case Win_Favorites:
        {
            return GetWindowsFolder(CSIDL_FAVORITES, aFile);
        }
        case Win_Startup:
        {
            return GetWindowsFolder(CSIDL_STARTUP, aFile);
        }
        case Win_Recent:
        {
            return GetWindowsFolder(CSIDL_RECENT, aFile);
        }
        case Win_Sendto:
        {
            return GetWindowsFolder(CSIDL_SENDTO, aFile);
        }
        case Win_Bitbucket:
        {
            return GetWindowsFolder(CSIDL_BITBUCKET, aFile);
        }
        case Win_Startmenu:
        {
            return GetWindowsFolder(CSIDL_STARTMENU, aFile);
        }
        case Win_Desktopdirectory:
        {
            return GetWindowsFolder(CSIDL_DESKTOPDIRECTORY, aFile);
        }
        case Win_Drives:
        {
            return GetWindowsFolder(CSIDL_DRIVES, aFile);
        }
        case Win_Network:
        {
            return GetWindowsFolder(CSIDL_NETWORK, aFile);
        }
        case Win_Nethood:
        {
            return GetWindowsFolder(CSIDL_NETHOOD, aFile);
        }
        case Win_Fonts:
        {
            return GetWindowsFolder(CSIDL_FONTS, aFile);
        }
        case Win_Templates:
        {
            return GetWindowsFolder(CSIDL_TEMPLATES, aFile);
        }
#ifndef WINCE
        case Win_Common_Startmenu:
        {
            return GetWindowsFolder(CSIDL_COMMON_STARTMENU, aFile);
        }
        case Win_Common_Programs:
        {
            return GetWindowsFolder(CSIDL_COMMON_PROGRAMS, aFile);
        }
        case Win_Common_Startup:
        {
            return GetWindowsFolder(CSIDL_COMMON_STARTUP, aFile);
        }
        case Win_Common_Desktopdirectory:
        {
            return GetWindowsFolder(CSIDL_COMMON_DESKTOPDIRECTORY, aFile);
        }
        case Win_Printhood:
        {
            return GetWindowsFolder(CSIDL_PRINTHOOD, aFile);
        }
        case Win_Cookies:
        {
            return GetWindowsFolder(CSIDL_COOKIES, aFile);
        }
#endif
        case Win_Appdata:
        {
            return GetWindowsFolder(CSIDL_APPDATA, aFile);
        }

        case Win_LocalAppdata:
        {
            return GetWindowsFolder(CSIDL_LOCAL_APPDATA, aFile);
        }
#endif  // XP_WIN

#if defined(XP_UNIX)
        case Unix_LocalDirectory:
            return NS_NewNativeLocalFile(nsDependentCString("/usr/local/netscape/"), 
                                         PR_TRUE, 
                                         aFile);
        case Unix_LibDirectory:
            return NS_NewNativeLocalFile(nsDependentCString("/usr/local/lib/netscape/"), 
                                         PR_TRUE, 
                                         aFile);

        case Unix_HomeDirectory:
            return GetUnixHomeDir(aFile);

        case Unix_DesktopDirectory:
        {
            nsCOMPtr<nsILocalFile> home;
            nsresult rv = GetUnixHomeDir(getter_AddRefs(home));
            if (NS_FAILED(rv))
                return rv;
            rv = home->AppendNative(NS_LITERAL_CSTRING("Desktop"));
            if (NS_FAILED(rv))
                return rv;
            PRBool exists;
            rv = home->Exists(&exists);
            if (NS_FAILED(rv))
                return rv;
            if (!exists)
                return GetUnixHomeDir(aFile);
              
            NS_ADDREF(*aFile = home);
            return NS_OK;
        }
#endif

#ifdef XP_BEOS
        case BeOS_SettingsDirectory:
        {
            return GetBeOSFolder(B_USER_SETTINGS_DIRECTORY,0, aFile);
        }

        case BeOS_HomeDirectory:
        {
            return GetBeOSFolder(B_USER_DIRECTORY,0, aFile);
        }

        case BeOS_DesktopDirectory:
        {
            /* Get the user's desktop folder, which in the future may differ from the boot desktop */
            char path[MAXPATHLEN];
            if (find_directory(B_USER_DIRECTORY, 0, false, path, MAXPATHLEN) != B_OK )
                break;
            return GetBeOSFolder(B_DESKTOP_DIRECTORY, dev_for_path(path), aFile);
        }

        case BeOS_SystemDirectory:
        {
            return GetBeOSFolder(B_BEOS_DIRECTORY,0, aFile);
        }
#endif        
#ifdef XP_OS2
        case OS2_SystemDirectory:
        {
            ULONG ulBootDrive = 0;
            char  buffer[] = " :\\OS2\\System\\";
            DosQuerySysInfo( QSV_BOOT_DRIVE, QSV_BOOT_DRIVE,
                             &ulBootDrive, sizeof ulBootDrive);
            buffer[0] = 'A' - 1 + ulBootDrive; // duh, 1-based index...

            return NS_NewNativeLocalFile(nsDependentCString(buffer), 
                                         PR_TRUE, 
                                         aFile);
        }

     case OS2_OS2Directory:
        {
            ULONG ulBootDrive = 0;
            char  buffer[] = " :\\OS2\\";
            DosQuerySysInfo( QSV_BOOT_DRIVE, QSV_BOOT_DRIVE,
                             &ulBootDrive, sizeof ulBootDrive);
            buffer[0] = 'A' - 1 + ulBootDrive; // duh, 1-based index...

            return NS_NewNativeLocalFile(nsDependentCString(buffer), 
                                         PR_TRUE, 
                                         aFile);
        }

     case OS2_HomeDirectory:
        {
            nsresult rv;
            char *tPath = PR_GetEnv("MOZILLA_HOME");
            char buffer[CCHMAXPATH];
            /* If MOZILLA_HOME is not set, use GetCurrentProcessDirectory */
            /* To ensure we get a long filename system */
            if (!tPath || !*tPath) {
                PPIB ppib;
                PTIB ptib;
                DosGetInfoBlocks( &ptib, &ppib);
                DosQueryModuleName( ppib->pib_hmte, CCHMAXPATH, buffer);
                *strrchr( buffer, '\\') = '\0'; // XXX DBCS misery
                tPath = buffer;
            }
            rv = NS_NewNativeLocalFile(nsDependentCString(tPath),
                                       PR_TRUE, 
                                       aFile);

            PrfWriteProfileString(HINI_USERPROFILE, "Mozilla", "Home", tPath);
            return rv;
        }

        case OS2_DesktopDirectory:
        {
            char szPath[CCHMAXPATH + 1];        
            BOOL fSuccess;
            fSuccess = WinQueryActiveDesktopPathname (szPath, sizeof(szPath));
            int len = strlen (szPath);   
            if (len > CCHMAXPATH -1)
                break;
            szPath[len] = '\\';     
            szPath[len + 1] = '\0';

            return NS_NewNativeLocalFile(nsDependentCString(szPath),
                                         PR_TRUE, 
                                         aFile);
        }
#endif
        default:
            break;
    }
    return NS_ERROR_NOT_AVAILABLE;
}

#if defined (XP_MACOSX)
nsresult
GetOSXFolderType(short aDomain, OSType aFolderType, nsILocalFile **localFile)
{
    OSErr err;
    FSRef fsRef;
    nsresult rv = NS_ERROR_FAILURE;

    err = ::FSFindFolder(aDomain, aFolderType, kCreateFolder, &fsRef);
    if (err == noErr)
    {
        NS_NewLocalFile(EmptyString(), PR_TRUE, localFile);
        nsCOMPtr<nsILocalFileMac> localMacFile(do_QueryInterface(*localFile));
        if (localMacFile)
            rv = localMacFile->InitWithFSRef(&fsRef);
    }
    return rv;
}                                                                      
#endif

