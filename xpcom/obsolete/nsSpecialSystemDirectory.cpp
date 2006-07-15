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

#include "nsSpecialSystemDirectory.h"
#include "nsDebug.h"

#ifdef XP_MAC
#include <Folders.h>
#include <Files.h>
#include <Memory.h>
#include <Processes.h>
#include <Gestalt.h>
#include "nsIInternetConfigService.h"
#ifdef DEBUG
#include "prenv.h" // For PR_Getenv
#endif
#elif defined(XP_WIN)
#include <windows.h>
#include <shlobj.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "nsNativeCharsetUtils.h"
#elif defined(XP_OS2)
#define MAX_PATH _MAX_PATH
#define INCL_WINWORKPLACE
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

#include "plstr.h"

#include "nsHashtable.h"
#include "prlog.h"

class SystemDirectoriesKey : public nsHashKey {
public:

    SystemDirectoriesKey(nsSpecialSystemDirectory::SystemDirectories newKey) : sdKey(newKey) {}

    virtual PRUint32 HashCode(void) const
    {
        return PRUint32(sdKey);
    }
    
    virtual PRBool Equals(const nsHashKey *aKey) const
    {
        nsSpecialSystemDirectory::SystemDirectories other = 
            ((SystemDirectoriesKey*)aKey)->sdKey;
        return other == sdKey;
    }
    
    virtual nsHashKey *Clone(void) const
    {
        return new SystemDirectoriesKey(sdKey);
    }

private:
    nsSpecialSystemDirectory::SystemDirectories sdKey; // sd for SystemDirectories
};

PR_STATIC_CALLBACK(PRBool) DeleteSystemDirKeys(nsHashKey *aKey, void *aData, void* closure)
{
    delete ((nsFileSpec *)aData);
    return PR_TRUE;
}

#define NS_SYSTEMDIR_HASH_NUM (10)
static nsHashtable *systemDirectoriesLocations = NULL;
#if defined (XP_WIN)
typedef BOOL (WINAPI * GetSpecialPathProc) (HWND hwndOwner, LPSTR lpszPath, int nFolder, BOOL fCreate);
GetSpecialPathProc gGetSpecialPathProc = NULL;
static HINSTANCE gShell32DLLInst = NULL;
#endif

#if defined (XP_WIN)

//----------------------------------------------------------------------------------------
static char* MakeUpperCase(char* aPath)
//----------------------------------------------------------------------------------------
{
  // windows does not care about case.  push to uppercase:
  nsAutoString widePath;
  nsDependentCString path(aPath);
  nsresult rv = NS_CopyNativeToUnicode(path, widePath);
  if (NS_FAILED(rv)) {
      NS_ERROR("failed to convert a path to Unicode");
      return aPath;
  }

  PRUnichar *start = widePath.BeginWriting();
  PRUnichar *end = widePath.EndWriting();

  while (start != end) {
      // XXX this doesn't change any non-ASCII character 
      *start = towupper(*start);
      ++start;
  }

  nsCAutoString newCPath;
  NS_CopyUnicodeToNative(widePath, newCPath); 
  NS_ASSERTION(path.Length() >= newCPath.Length(), 
               "uppercased string is longer than original");
  ::strcpy(aPath, newCPath.get());

  return aPath;
}

//----------------------------------------------------------------------------------------
static void GetWindowsFolder(int folder, nsFileSpec& outDirectory)
//----------------------------------------------------------------------------------------
{

    if (gGetSpecialPathProc) {
        TCHAR path[MAX_PATH];
        HRESULT result = gGetSpecialPathProc(NULL, path, folder, true);
        
        if (!SUCCEEDED(result)) 
            return;

        // Append the trailing slash
        int len = PL_strlen(path);
        if (len>1 && path[len-1] != '\\') 
        {
            path[len]   = '\\';
            path[len + 1] = '\0';
        }
        outDirectory = path;
        return;
    }

    LPMALLOC pMalloc = NULL;
    LPSTR pBuffer = NULL;
    LPITEMIDLIST pItemIDList = NULL;
    int len;
 
    // Get the shell's allocator. 
    if (!SUCCEEDED(SHGetMalloc(&pMalloc))) 
        return;

    // Allocate a buffer
    if ((pBuffer = (LPSTR) pMalloc->Alloc(MAX_PATH + 2)) == NULL) 
        return; 
 
    // Get the PIDL for the folder. 
    if (!SUCCEEDED(SHGetSpecialFolderLocation( 
            NULL, folder, &pItemIDList)))
        goto Clean;
 
    if (!SUCCEEDED(SHGetPathFromIDList(pItemIDList, pBuffer)))
        goto Clean;

    // Append the trailing slash
    len = PL_strlen(pBuffer);
    pBuffer[len]   = '\\';
    pBuffer[len + 1] = '\0';

    // Assign the directory
    outDirectory = pBuffer;

Clean:
    // Clean up. 
    if (pItemIDList)
        pMalloc->Free(pItemIDList); 
    if (pBuffer)
        pMalloc->Free(pBuffer); 

	pMalloc->Release();
} // GetWindowsFolder
#endif // XP_WIN

//----------------------------------------------------------------------------------------
static void GetCurrentWorkingDirectory(nsFileSpec& aFileSpec)
//----------------------------------------------------------------------------------------
{
    aFileSpec = ".";
    return;
} // GetCurrentWorkingDirectory

//----------------------------------------------------------------------------------------
static void GetCurrentProcessDirectory(nsFileSpec& aFileSpec)
//----------------------------------------------------------------------------------------
{
#if defined (XP_WIN)
    char buf[MAX_PATH];
    if ( ::GetModuleFileName(0, buf, sizeof(buf)) ) {
        // chop of the executable name by finding the rightmost backslash
        char* lastSlash = PL_strrchr(buf, '\\');
        if (lastSlash)
            *(lastSlash + 1) = '\0';

        aFileSpec = buf;
        return;
    }

#elif defined(XP_OS2)
    PPIB ppib;
    PTIB ptib;
    char buffer[CCHMAXPATH];
    DosGetInfoBlocks( &ptib, &ppib);
    DosQueryModuleName( ppib->pib_hmte, CCHMAXPATH, buffer);
    *strrchr( buffer, '\\') = '\0'; // XXX DBCS misery
    aFileSpec = buffer;
    return;

#elif defined(XP_UNIX)

    // In the absence of a good way to get the executable directory let
    // us try this for unix:
    //	- if MOZILLA_FIVE_HOME is defined, that is it
    //	- else give the current directory
    char buf[MAXPATHLEN];
    char *moz5 = PR_GetEnv("MOZILLA_FIVE_HOME");
    if (moz5)
    {
        aFileSpec = moz5;
        return;
    }
    else
    {
#if defined(DEBUG)
        static PRBool firstWarning = PR_TRUE;

        if(firstWarning) {
            // Warn that MOZILLA_FIVE_HOME not set, once.
            printf("Warning: MOZILLA_FIVE_HOME not set.\n");
            firstWarning = PR_FALSE;
        }
#endif /* DEBUG */

        // Fall back to current directory.
        if (getcwd(buf, sizeof(buf)))
        {
            aFileSpec = buf;
            return;
        }
    }

#elif defined(XP_BEOS)

    char *moz5 = getenv("MOZILLA_FIVE_HOME");
    if (moz5)
    {
        aFileSpec = moz5;
        return;
    }
    else
    {
      static char buf[MAXPATHLEN];
      int32 cookie = 0;
      image_info info;
      char *p;
      *buf = 0;
      if(get_next_image_info(0, &cookie, &info) == B_OK)
      {
        strcpy(buf, info.name);
        if((p = strrchr(buf, '/')) != 0)
        {
          *p = 0;
          aFileSpec = buf;
          return;
        }
      }
    }

#endif

    NS_ERROR("unable to get current process directory");
} // GetCurrentProcessDirectory()

//nsSpecialSystemDirectory::nsSpecialSystemDirectory()
//:    nsFileSpec(nsnull)
//{
//}

//----------------------------------------------------------------------------------------
nsSpecialSystemDirectory::nsSpecialSystemDirectory(SystemDirectories aSystemSystemDirectory)
//----------------------------------------------------------------------------------------
:    nsFileSpec(nsnull)
{
    *this = aSystemSystemDirectory;
}

//----------------------------------------------------------------------------------------
nsSpecialSystemDirectory::~nsSpecialSystemDirectory()
//----------------------------------------------------------------------------------------
{
}

//----------------------------------------------------------------------------------------
void nsSpecialSystemDirectory::operator = (SystemDirectories aSystemSystemDirectory)
//----------------------------------------------------------------------------------------
{
    SystemDirectoriesKey dirKey(aSystemSystemDirectory);
    SystemDirectoriesKey mozBinDirKey(Moz_BinDirectory);

    // This flag is used to tell whether or not we need to append something
    // onto the *this.  Search for needToAppend to how it's used.
    // IT's VERY IMPORTANT that needToAppend is initialized to PR_TRUE.
    PRBool needToAppend = PR_TRUE;

    *this = (const char*)nsnull;
    switch (aSystemSystemDirectory)
    {
        
        case OS_DriveDirectory:
#if defined (XP_WIN)
        {
            char path[_MAX_PATH];
            PRInt32 len = GetWindowsDirectory( path, _MAX_PATH );
            if (len)
            {
                if ( path[1] == ':' && path[2] == '\\' )
                    path[3] = 0;
            }
            *this = MakeUpperCase(path);
        }
#elif defined(XP_OS2)
        {
            // printf( "*** Warning warning OS_DriveDirectory called for");
            
            ULONG ulBootDrive = 0;
            char  buffer[] = " :\\OS2\\";
            DosQuerySysInfo( QSV_BOOT_DRIVE, QSV_BOOT_DRIVE,
                             &ulBootDrive, sizeof ulBootDrive);
            buffer[0] = 'A' - 1 + ulBootDrive; // duh, 1-based index...
            *this = buffer;
#ifdef DEBUG
            printf( "Got OS_DriveDirectory: %s\n", buffer);
#endif
        }
#else
        *this = "/";
#endif
        break;

            
        case OS_TemporaryDirectory:
#if defined (WINCE)
            {
                *this = "\\TEMP";
            }
#elif defined (XP_WIN)
        {
            char path[_MAX_PATH];
            DWORD len = GetTempPath(_MAX_PATH, path);
            *this = MakeUpperCase(path);
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
             if( c) *this = buffer;
             // use exe's directory if not set
             else GetCurrentProcessDirectory(*this);
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
			
			*this = tPath;
		}
#endif
        break;

        case OS_CurrentProcessDirectory:
            GetCurrentProcessDirectory(*this);
            break;

        case OS_CurrentWorkingDirectory:
            GetCurrentWorkingDirectory(*this);
            break;

        case XPCOM_CurrentProcessComponentRegistry:
            {
                nsFileSpec *dirSpec = NULL;

                // if someone has called nsSpecialSystemDirectory::Set()
                if (systemDirectoriesLocations) {
                    // look for the value for the argument key
                    if (!(dirSpec = (nsFileSpec *)systemDirectoriesLocations->Get(&dirKey))) {
                        // if not found, try Moz_BinDirectory
                        dirSpec = (nsFileSpec *)
                            systemDirectoriesLocations->Get(&mozBinDirKey);
                    }
                    else {
                        // if the value is found for the argument key,
                        // we don't need to append.
                        needToAppend = PR_FALSE;
                    }
                }
                
                if (dirSpec)
                {
                    *this = *dirSpec;
                }
                else
                {
                    GetCurrentProcessDirectory(*this);
                }

                if (needToAppend) {
                    // XXX We need to unify these names across all platforms
                    *this += "component.reg";
                }
            }
            break;

        case XPCOM_CurrentProcessComponentDirectory:
            {
                nsFileSpec *dirSpec = NULL;
                // if someone has called nsSpecialSystemDirectory::Set()
                if (systemDirectoriesLocations) {
                    // look for the value for the argument key
                    if (!(dirSpec = (nsFileSpec *)systemDirectoriesLocations->Get(&dirKey))) {
                        // if not found, try Moz_BinDirectory
                        dirSpec = (nsFileSpec *)
                            systemDirectoriesLocations->Get(&mozBinDirKey);
                    }
                    else {
                        // if the value is found for the argument key,
                        // we don't need to append.
                        needToAppend = PR_FALSE;
                    }
                }
                if (dirSpec)
                {
                    *this = *dirSpec;
                }
                else
                {
                    // <exedir>/Components
                    GetCurrentProcessDirectory(*this);
                }

                if (needToAppend) {
                    // XXX We need to unify these names across all platforms
                    *this += "components";
                }
            }
            break;

        case Moz_BinDirectory:
            {
                nsFileSpec *dirSpec = NULL;
                // if someone has called nsSpecialSystemDirectory::Set()
                if (systemDirectoriesLocations) {
                    // look for the value for the argument key
                    dirSpec = (nsFileSpec *)
                        systemDirectoriesLocations->Get(&dirKey);
                }
                if (dirSpec) {
                    *this = *dirSpec;
                }
                else {
                    GetCurrentProcessDirectory(*this);
                }
            }
            break;
            
#if defined (XP_WIN)
        case Win_SystemDirectory:
        {    
            char path[_MAX_PATH];
            PRInt32 len = GetSystemDirectory( path, _MAX_PATH );
        
            // Need enough space to add the trailing backslash
            if (len > _MAX_PATH-2)
                break;
            path[len]   = '\\';
            path[len+1] = '\0';

            *this = MakeUpperCase(path);

            break;
        }

        case Win_WindowsDirectory:
        {    
            char path[_MAX_PATH];
            PRInt32 len = GetWindowsDirectory( path, _MAX_PATH );
            
            // Need enough space to add the trailing backslash
            if (len > _MAX_PATH-2)
                break;
            
            path[len]   = '\\';
            path[len+1] = '\0';

            *this = MakeUpperCase(path);
            break;
        }

        case Win_HomeDirectory:
        {    
            char path[_MAX_PATH];
            if (GetEnvironmentVariable(TEXT("HOME"), path, _MAX_PATH) > 0)
            {
                PRInt32 len = PL_strlen(path);
                // Need enough space to add the trailing backslash
                if (len > _MAX_PATH - 2)
                    break;
               
                path[len]   = '\\';
                path[len+1] = '\0';
                
                *this = MakeUpperCase(path);
                break;
            }

            if (GetEnvironmentVariable(TEXT("HOMEDRIVE"), path, _MAX_PATH) > 0)
            {
                char temp[_MAX_PATH];
                if (GetEnvironmentVariable(TEXT("HOMEPATH"), temp, _MAX_PATH) > 0)
                   PL_strcatn(path, _MAX_PATH, temp);
        
                PRInt32 len = PL_strlen(path);

                // Need enough space to add the trailing backslash
                if (len > _MAX_PATH - 2)
                    break;
            
                path[len]   = '\\';
                path[len+1] = '\0';
                
                *this = MakeUpperCase(path);
                break;
            }
        }
        case Win_Desktop:
        {
            GetWindowsFolder(CSIDL_DESKTOP, *this);
            break;
        }
        case Win_Programs:
        {
            GetWindowsFolder(CSIDL_PROGRAMS, *this);
            break;
        }
        case Win_Controls:
        {
            GetWindowsFolder(CSIDL_CONTROLS, *this);
            break;
        }
        case Win_Printers:
        {
            GetWindowsFolder(CSIDL_PRINTERS, *this);
            break;
        }
        case Win_Personal:
        {
            GetWindowsFolder(CSIDL_PERSONAL, *this);
            break;
        }
        case Win_Favorites:
        {
            GetWindowsFolder(CSIDL_FAVORITES, *this);
            break;
        }
        case Win_Startup:
        {
            GetWindowsFolder(CSIDL_STARTUP, *this);
            break;
        }
        case Win_Recent:
        {
            GetWindowsFolder(CSIDL_RECENT, *this);
            break;
        }
        case Win_Sendto:
        {
            GetWindowsFolder(CSIDL_SENDTO, *this);
            break;
        }
        case Win_Bitbucket:
        {
            GetWindowsFolder(CSIDL_BITBUCKET, *this);
            break;
        }
        case Win_Startmenu:
        {
            GetWindowsFolder(CSIDL_STARTMENU, *this);
            break;
        }
        case Win_Desktopdirectory:
        {
            GetWindowsFolder(CSIDL_DESKTOPDIRECTORY, *this);
            break;
        }
        case Win_Drives:
        {
            GetWindowsFolder(CSIDL_DRIVES, *this);
            break;
        }
        case Win_Network:
        {
            GetWindowsFolder(CSIDL_NETWORK, *this);
            break;
        }
        case Win_Nethood:
        {
            GetWindowsFolder(CSIDL_NETHOOD, *this);
            break;
        }
        case Win_Fonts:
        {
            GetWindowsFolder(CSIDL_FONTS, *this);
            break;
        }
        case Win_Templates:
        {
            GetWindowsFolder(CSIDL_TEMPLATES, *this);
            break;
        }
#ifndef WINCE
        case Win_Common_Startmenu:
        {
            GetWindowsFolder(CSIDL_COMMON_STARTMENU, *this);
            break;
        }
        case Win_Common_Programs:
        {
            GetWindowsFolder(CSIDL_COMMON_PROGRAMS, *this);
            break;
        }
        case Win_Common_Startup:
        {
            GetWindowsFolder(CSIDL_COMMON_STARTUP, *this);
            break;
        }
        case Win_Common_Desktopdirectory:
        {
            GetWindowsFolder(CSIDL_COMMON_DESKTOPDIRECTORY, *this);
            break;
        }
        case Win_Printhood:
        {
            GetWindowsFolder(CSIDL_PRINTHOOD, *this);
            break;
        }
        case Win_Cookies:
        {
            GetWindowsFolder(CSIDL_COOKIES, *this);
            break;
        }
#endif // WINCE

        case Win_Appdata:
        {
            GetWindowsFolder(CSIDL_APPDATA, *this);
            break;
        }
#endif  // XP_WIN

#if defined(XP_UNIX)
        case Unix_LocalDirectory:
            *this = "/usr/local/netscape/";
            break;

        case Unix_LibDirectory:
            *this = "/usr/local/lib/netscape/";
            break;

        case Unix_HomeDirectory:
#ifdef VMS
	    {
	        char *pHome;
	        pHome = getenv("HOME");
		if (*pHome == '/')
        	    *this = pHome;
		else
        	    *this = decc$translate_vms(pHome);
	    }
#else
            *this = PR_GetEnv("HOME");
#endif
            break;

#endif        

#ifdef XP_BEOS
        case BeOS_SettingsDirectory:
		{
            char path[MAXPATHLEN];
			find_directory(B_USER_SETTINGS_DIRECTORY, 0, 0, path, MAXPATHLEN);
            // Need enough space to add the trailing backslash
			int len = strlen(path);
            if (len > MAXPATHLEN-2)
                break;
            path[len]   = '/';
            path[len+1] = '\0';
			*this = path;
            break;
		}

        case BeOS_HomeDirectory:
		{
            char path[MAXPATHLEN];
			find_directory(B_USER_DIRECTORY, 0, 0, path, MAXPATHLEN);
            // Need enough space to add the trailing backslash
			int len = strlen(path);
            if (len > MAXPATHLEN-2)
                break;
            path[len]   = '/';
            path[len+1] = '\0';
			*this = path;
            break;
		}

        case BeOS_DesktopDirectory:
		{
            char path[MAXPATHLEN];
			find_directory(B_DESKTOP_DIRECTORY, 0, 0, path, MAXPATHLEN);
            // Need enough space to add the trailing backslash
			int len = strlen(path);
            if (len > MAXPATHLEN-2)
                break;
            path[len]   = '/';
            path[len+1] = '\0';
			*this = path;
            break;
		}

        case BeOS_SystemDirectory:
		{
            char path[MAXPATHLEN];
			find_directory(B_BEOS_DIRECTORY, 0, 0, path, MAXPATHLEN);
            // Need enough space to add the trailing backslash
			int len = strlen(path);
            if (len > MAXPATHLEN-2)
                break;
            path[len]   = '/';
            path[len+1] = '\0';
			*this = path;
            break;
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
            *this = buffer;
#ifdef DEBUG
            printf( "Got OS2_SystemDirectory: %s\n", buffer);
#endif
            break;
        }

     case OS2_OS2Directory:
        {
            ULONG ulBootDrive = 0;
            char  buffer[] = " :\\OS2\\";
            DosQuerySysInfo( QSV_BOOT_DRIVE, QSV_BOOT_DRIVE,
                             &ulBootDrive, sizeof ulBootDrive);
            buffer[0] = 'A' - 1 + ulBootDrive; // duh, 1-based index...
            *this = buffer;
#ifdef DEBUG
            printf( "Got OS2_OS2Directory: %s\n", buffer);
#endif
            break;
        }

     case OS2_HomeDirectory:
        {
            char *tPath = PR_GetEnv("MOZILLA_HOME");
            /* If MOZILLA_HOME is not set, use GetCurrentProcessDirectory */
            /* To ensure we get a long filename system */
            if (!tPath || !*tPath)
              GetCurrentProcessDirectory(*this);
            else
              *this = tPath;
            PrfWriteProfileString(HINI_USERPROFILE, "Mozilla", "Home", *this);
            break;
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
#ifdef DEBUG
            if (fSuccess) {
               printf ("Got OS2_DesktopDirectory: %s\n", szPath);
            } else {
               printf ("Failed getting OS2_DesktopDirectory: %s\n", szPath);
            }
#endif
            break;           
        }

#endif
        default:
            break;    
    }
}

void
nsSpecialSystemDirectory::Set(SystemDirectories dirToSet, nsFileSpec *dirSpec)
{
    SystemDirectoriesKey dirKey(dirToSet);
    
    PR_ASSERT(NULL != dirSpec);
    
    if (NULL == systemDirectoriesLocations) {
        systemDirectoriesLocations = new nsHashtable(NS_SYSTEMDIR_HASH_NUM);
    }
    PR_ASSERT(NULL != systemDirectoriesLocations);
    
    nsFileSpec *newSpec = new nsFileSpec(*dirSpec);
    if (NULL != newSpec) {
        systemDirectoriesLocations->Put(&dirKey, newSpec);
    }
    
    return;
}
