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
 *   Don Bragg <dbragg@netscape.com>
 *   Seth Spitzer <sspitzer@netscape.com>
 *   Pierre Phaneuf <pp@ludusdesign.com>
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

#include "pratom.h"
#include "nsIComponentManager.h"
#include "nsIComponentManager.h"
#include "nsIPromptService.h"
#include "nsIServiceManager.h"
#include "nsPIDOMWindow.h"
#include "nsIScriptContext.h"
#include "nsILocalFile.h"
#include "nsDirectoryServiceUtils.h"
#include "nsDirectoryServiceDefs.h"
#include "nsDependentString.h"
#include "nsFileStream.h"
#include "nsIFileSpec.h"
#include "nsCOMPtr.h"
#include "prio.h"
#include "prerror.h"
#include "prmem.h"
#include "nsIPrefService.h"
#include "plstr.h"
#include "prprf.h"
#include "nsXPIDLString.h"
#include "nsReadableUtils.h"
#include "nsIStringBundle.h"
#include "nsProxiedService.h"

#include "nsNetUtil.h"
#include "nsCRT.h"

#include "nsVoidArray.h"

#include "nsIBaseWindow.h"
#include "nsIDocShell.h"
#include "nsIDocShellTreeItem.h"
#include "nsIDocShellTreeOwner.h"
#include "nsIWebBrowserChrome.h"
#include "nsIWindowWatcher.h"
#include "nsEmbedCID.h"

#ifdef DEBUG_seth
#define DEBUG_UTF8_CONVERSION 1
#endif 

#include "nsICharsetConverterManager.h"
#include "nsIPlatformCharset.h"

#define CHROME_STYLE nsIWebBrowserChrome::CHROME_ALL | nsIWebBrowserChrome::CHROME_CENTER_SCREEN
#define MIGRATION_PROPERTIES_URL "chrome://communicator/locale/profile/migration.properties"

/* Network */

#include "nsPrefMigration.h"
#include "nsPrefMigrationFactory.h"

#define PREF_FILE_HEADER_STRING "# Mozilla User Preferences    " 

#define MAX_PREF_LEN 1024

#if defined(XP_UNIX) && !defined(XP_MACOSX)
#define IMAP_MAIL_FILTER_FILE_NAME_IN_4x "mailrule"
#define POP_MAIL_FILTER_FILE_NAME_IN_4x "mailrule"
#define MAIL_SUMMARY_SUFFIX_IN_4x ".summary"
#define NEWS_SUMMARY_SUFFIX_IN_4x ".snm"
#define COOKIES_FILE_NAME_IN_4x "cookies"
#define BOOKMARKS_FILE_NAME_IN_4x "bookmarks.html"
#define NEWSRC_PREFIX_IN_4x ".newsrc-"
#define SNEWSRC_PREFIX_IN_4x ".snewsrc-"
#define POPSTATE_FILE_IN_4x "popstate"
#define PSM_CERT7_DB "cert7.db"
#define PSM_KEY3_DB "key3.db"
#define PSM_SECMODULE_DB "secmodule.db"
#elif defined(XP_MACOSX)
#define MAC_RULES_FILE_ENDING_STRING_IN_4X " Rules"
#define IMAP_MAIL_FILTER_FILE_NAME_IN_4x "<hostname> Rules"
#define POP_MAIL_FILTER_FILE_NAME_IN_4x "Filter Rules"
#define MAIL_SUMMARY_SUFFIX_IN_4x ".snm"
#define NEWS_SUMMARY_SUFFIX_IN_4x ".snm"
#define COOKIES_FILE_NAME_IN_4x "MagicCookie"
#define BOOKMARKS_FILE_NAME_IN_4x "Bookmarks.html"
#define POPSTATE_FILE_IN_4x "Pop State"
#define SECURITY_PATH "Security"
#define PSM_CERT7_DB "Certificates7"
#define PSM_KEY3_DB "Key Database3"
#define PSM_SECMODULE_DB "Security Modules"
#else /* XP_WIN || XP_OS2 */
#define IMAP_MAIL_FILTER_FILE_NAME_IN_4x "rules.dat"
#define POP_MAIL_FILTER_FILE_NAME_IN_4x "rules.dat"
#define MAIL_SUMMARY_SUFFIX_IN_4x ".snm"
#define NEWS_SUMMARY_SUFFIX_IN_4x ".snm"
#define COOKIES_FILE_NAME_IN_4x "cookies.txt"
#define BOOKMARKS_FILE_NAME_IN_4x "bookmark.htm"
// purposely not defined, since it was in the right place
// and with the right name in 4.x
//#define POPSTATE_FILE_IN_4x "popstate.dat"
#define PSM_CERT7_DB "cert7.db"
#define PSM_KEY3_DB "key3.db"
#define PSM_SECMODULE_DB "secmod.db"
#endif /* XP_UNIX */

#define SUMMARY_SUFFIX_IN_5x ".msf"
#define COOKIES_FILE_NAME_IN_5x "cookies.txt"
#define IMAP_MAIL_FILTER_FILE_NAME_IN_5x "rules.dat"
#define POP_MAIL_FILTER_FILE_NAME_IN_5x "rules.dat"
#define POPSTATE_FILE_IN_5x	"popstate.dat"
#define BOOKMARKS_FILE_NAME_IN_5x "bookmarks.html"
#define HISTORY_FILE_NAME_IN_5x "history.dat"

// only UNIX had movemail in 4.x
#if defined(XP_UNIX) && !defined(XP_MACOSX)
#define HAVE_MOVEMAIL 1
#endif /* XP_UNIX */

#define PREMIGRATION_PREFIX "premigration."

// this is for the hidden preference setting in mozilla/modules/libpref/src/init/mailnews.js
// pref("mail.migration.copyMailFiles", true);
//
// see bugzilla bug 80035 (http://bugzilla.mozilla.org/show_bug.cgi?id=80035)
//
// the default value for this setting is true which means when migrating from
// Netscape 4.x, mozilla will copy all the contents of Local Folders and Imap
// Folder to the newly created subfolders of migrated mozilla profile
// when this value is set to false, mozilla will not copy these contents and
// still share them with Netscape 4.x
//
// Advantages of forbidding copy operation:
//     reduce the disk usage
//     quick migration
// Disadvantage of forbidding copy operation:
//     without perfect lock mechamism, there is possibility of data corruption
//     when Netscape 4.x and mozilla run at the same time and access the same
//     mail file at the same time
#define PREF_MIGRATION_MODE_FOR_MAIL "mail.migration.copyMailFiles"

#define PREF_MAIL_DIRECTORY "mail.directory"
#define PREF_NEWS_DIRECTORY "news.directory"
#define PREF_MAIL_IMAP_ROOT_DIR "mail.imap.root_dir"
#define PREF_NETWORK_HOSTS_POP_SERVER "network.hosts.pop_server"
#define PREF_4X_NETWORK_HOSTS_IMAP_SERVER "network.hosts.imap_servers"  
#define PREF_MAIL_SERVER_TYPE	"mail.server_type"
#define PREF_BROWSER_CACHE_DIRECTORY "browser.cache.directory"
#define POP_4X_MAIL_TYPE 0
#define IMAP_4X_MAIL_TYPE 1
#ifdef HAVE_MOVEMAIL
#define MOVEMAIL_4X_MAIL_TYPE 2
#define NEW_MOVEMAIL_DIR_NAME "movemail"
#endif /* HAVE_MOVEMAIL */

#if defined(XP_UNIX) && !defined(XP_MACOSX)
/* a 4.x profile on UNIX is rooted at something like
 * "/u/sspitzer/.netscape"
 * profile + OLD_MAIL_DIR_NAME = "/u/sspitzer/.netscape/../nsmail" = "/u/sspitzer/nsmail"
 * profile + OLD_NEWS_DIR_NAME = "/u/sspitzer/.netscape/xover-cache"
 * profile + OLD_IMAPMAIL_DIR_NAME = "/u/sspitzer/.netscape/../ns_imap" = "/u/sspitzer/ns_imap"
 * which is as good as we're going to get for defaults on UNIX.
 */
#define OLD_MAIL_DIR_NAME "/../nsmail"
#define OLD_NEWS_DIR_NAME "/xover-cache"
#define OLD_IMAPMAIL_DIR_NAME "/../ns_imap"
#else
#define OLD_MAIL_DIR_NAME "Mail"
#define OLD_NEWS_DIR_NAME "News"
#define OLD_IMAPMAIL_DIR_NAME "ImapMail"
#endif /* XP_UNIX */

#define NEW_DIR_SUFFIX "5"

#define PREF_FILE_NAME_IN_5x "prefs.js"

#define PREF_MIGRATION_PROGRESS_URL "chrome://communicator/content/profile/profileMigrationProgress.xul"

typedef struct
{
  const char* oldFile;
  const char* newFile;

} MigrateProfileItem;

/* 
 * In 4.x the mac cookie file used expiration times starting from
 * 1900 whereas all the other platforms started from
 * 1970.  In 5.0 it was made cross platform so that all platforms use
 * expiration times starting from 1970.  That means that mac cookies 
 * generated in 4.x cannot be migrated to 5.0 as is -- instead the 
 * expiration time must first be decreased by
 * the number of seconds between 1-1-1900 and 1-1-1970
 * 
 *  70 years * 365 days/year * 86,400 secs/day      = 2,207,520,000 seconds
 * + 17 leap years * 86,400 additional sec/leapyear =     1,468,800 seconds
 *                                                  = 2,208,988,800 seconds
 */
#if defined(XP_MACOSX)
#define NEED_TO_FIX_4X_COOKIES 1
#define SECONDS_BETWEEN_1900_AND_1970 2208988800UL
#endif /* XP_MACOSX */

/*-----------------------------------------------------------------
 * Globals
 *-----------------------------------------------------------------*/
nsPrefMigration* nsPrefMigration::mInstance = nsnull;

nsPrefMigration *
nsPrefMigration::GetInstance()
{
    if (mInstance == nsnull) 
    {
        mInstance = new nsPrefMigration();
    }
    return mInstance;
}



nsPrefMigration::nsPrefMigration()
{
  mErrorCode = NS_OK;
}



PRBool ProfilesToMigrateCleanup(void* aElement, void *aData)
{
  if (aElement)
    delete (MigrateProfileItem*)aElement;

  return PR_TRUE;
}

nsPrefMigration::~nsPrefMigration()
{
  mProfilesToMigrate.EnumerateForwards((nsVoidArrayEnumFunc)ProfilesToMigrateCleanup, nsnull);
  mInstance = nsnull; 
}



nsresult
nsPrefMigration::getPrefService()
{
  // get the prefs service
  nsresult rv = NS_OK;

  nsCOMPtr<nsIPrefBranch> pIMyService(do_GetService(NS_PREFSERVICE_CONTRACTID, &rv));
  if(NS_FAILED(rv)) return rv;

  return NS_GetProxyForObject(NS_PROXY_TO_MAIN_THREAD, NS_GET_IID(nsIPrefBranch),
                              pIMyService, NS_PROXY_SYNC,
                              getter_AddRefs(m_prefBranch));

}

NS_IMPL_THREADSAFE_ISUPPORTS1(nsPrefMigration, nsIPrefMigration)

NS_IMETHODIMP
nsPrefMigration::AddProfilePaths(const char * oldProfilePathStr, const char * newProfilePathStr)
{
  MigrateProfileItem* item = new MigrateProfileItem();
  if (!item)
    return NS_ERROR_OUT_OF_MEMORY;

  item->oldFile = oldProfilePathStr;
  item->newFile = newProfilePathStr;
  
  if (mProfilesToMigrate.AppendElement((void*)item))
    return NS_OK;

  return NS_ERROR_FAILURE;
}


NS_IMETHODIMP
nsPrefMigration::ProcessPrefs(PRBool showProgressAsModalWindow)
{
  nsresult rv;
  
  nsCOMPtr<nsIWindowWatcher> windowWatcher(do_GetService(NS_WINDOWWATCHER_CONTRACTID, &rv));
  if (NS_FAILED(rv)) return rv;

  // WindowWatcher can work with or without parent window
  rv = windowWatcher->OpenWindow(nsnull,
                                 PREF_MIGRATION_PROGRESS_URL,
                                 "_blank",
                                 "centerscreen,modal,titlebar",
                                 nsnull,
                                 getter_AddRefs(mPMProgressWindow));
  if (NS_FAILED(rv)) return rv;

  return NS_OK;
}


static PRThread* gMigrationThread = nsnull;


extern "C" void ProfileMigrationController(void *data)
{
  if (!data) return;

  nsPrefMigration* migrator = (nsPrefMigration*)data;
  nsIPrefMigration* interfaceM = (nsIPrefMigration*)data;
  PRInt32 index = 0;
  PRInt32 choice = 0;
  nsresult rv = NS_OK;

  nsCOMPtr<nsIPrefMigration> prefProxy;

  do {
    
    choice = 0;
    migrator->mErrorCode = 0;
    MigrateProfileItem* item = nsnull;

    if (migrator->mProfilesToMigrate.Count() != 0)
      item = (MigrateProfileItem*)migrator->mProfilesToMigrate.ElementAt(index);
    if (item)
    {
        rv = migrator->ProcessPrefsCallback(item->oldFile, item->newFile);
        if (NS_FAILED(rv))
        {
          migrator->mErrorCode = rv;
#ifdef DEBUG
          printf("failed to migrate properly.  err=%d\n",rv);
#endif
        }
    }
    else
    {
      migrator->mErrorCode = NS_ERROR_FAILURE;
      return;
    }

    nsCOMPtr<nsIPrefMigration> migratorInterface = do_QueryInterface(interfaceM, &rv);
    if (NS_FAILED(rv))
    {
      migrator->mErrorCode = rv;
      return;
    }

    if (!prefProxy)
    {
        rv = NS_GetProxyForObject(NS_PROXY_TO_MAIN_THREAD,
                                  NS_GET_IID(nsIPrefMigration),
                                  migratorInterface, NS_PROXY_SYNC,
                                  getter_AddRefs(prefProxy));
        if (NS_FAILED(rv))
        {
          migrator->mErrorCode = rv;
          return;
        }
    }


    if (migrator->mErrorCode != 0)
    {
      if (migrator->mErrorCode == MIGRATION_RETRY)
      {
        rv = prefProxy->ShowSpaceDialog(&choice);
        if (NS_FAILED(rv))
        {
          migrator->mErrorCode = rv;
          return;
        }
        choice++;// Increment choice to match the RETRY=1, CANCEL=2 and CREATE_NEW=3 format
      }
    }

  } while (choice == MIGRATION_RETRY);

  prefProxy->WindowCloseCallback();
  migrator->mErrorCode = choice;

}

NS_IMETHODIMP
nsPrefMigration::WindowCloseCallback()
{
  nsCOMPtr<nsPIDOMWindow> window(do_QueryInterface(mPMProgressWindow));
  if (!window) return NS_ERROR_FAILURE;

  nsCOMPtr<nsIDocShellTreeItem> treeItem =
    do_QueryInterface(window->GetDocShell());
  if (!treeItem) return NS_ERROR_FAILURE;
  nsCOMPtr<nsIDocShellTreeOwner> treeOwner;
  treeItem->GetTreeOwner(getter_AddRefs(treeOwner));
  if (!treeOwner) return NS_ERROR_FAILURE;
  nsCOMPtr<nsIBaseWindow> baseWindow(do_QueryInterface(treeOwner));
  if (baseWindow)
    baseWindow->Destroy();
   
#ifdef DEBUG
   printf("end of pref migration\n");
#endif
   return NS_OK;
}


NS_IMETHODIMP
nsPrefMigration::ShowSpaceDialog(PRInt32 *choice)
{
  nsresult rv;
  nsCOMPtr<nsIStringBundleService> bundleService = do_GetService(NS_STRINGBUNDLE_CONTRACTID, &rv);
  if (NS_FAILED(rv)) return rv;

  nsCOMPtr<nsIStringBundle> bundle;
  rv = bundleService->CreateBundle(MIGRATION_PROPERTIES_URL, getter_AddRefs(bundle));
  if (NS_FAILED(rv)) return rv;

  nsXPIDLString noSpaceTitle, noSpaceText, retryLabel, createNewLabel;
  rv = bundle->GetStringFromName(NS_LITERAL_STRING("noSpace.title").get(), getter_Copies(noSpaceTitle));
  if (NS_FAILED(rv)) return rv;
  rv = bundle->GetStringFromName(NS_LITERAL_STRING("noSpace.text").get(), getter_Copies(noSpaceText));
  if (NS_FAILED(rv)) return rv;
  rv = bundle->GetStringFromName(NS_LITERAL_STRING("retry.label").get(), getter_Copies(retryLabel));
  if (NS_FAILED(rv)) return rv;
  rv = bundle->GetStringFromName(NS_LITERAL_STRING("createNew.label").get(), getter_Copies(createNewLabel));
  if (NS_FAILED(rv)) return rv;

  nsCOMPtr<nsIPromptService> promptService = do_GetService(NS_PROMPTSERVICE_CONTRACTID, &rv);
  if (NS_FAILED(rv)) return rv;

  const PRUint32 buttons =
    (nsIPromptService::BUTTON_TITLE_IS_STRING * nsIPromptService::BUTTON_POS_0)+
    (nsIPromptService::BUTTON_TITLE_CANCEL * nsIPromptService::BUTTON_POS_1)+
    (nsIPromptService::BUTTON_TITLE_IS_STRING * nsIPromptService::BUTTON_POS_2);
  return promptService->ConfirmEx(mPMProgressWindow, noSpaceTitle, noSpaceText,
                                  buttons, retryLabel, nsnull, createNewLabel,
                                  nsnull, nsnull, choice);
}


NS_IMETHODIMP
nsPrefMigration::ProcessPrefsFromJS()  // called via js so that we can have progress bar that show up.
{
  gMigrationThread = PR_CreateThread(PR_USER_THREAD,
                                     ProfileMigrationController,
                                     this, 
                                     PR_PRIORITY_NORMAL, 
                                     PR_GLOBAL_THREAD, 
                                     PR_UNJOINABLE_THREAD,
                                     0);  
  return NS_OK;
}  
    

NS_IMETHODIMP
nsPrefMigration::GetError()
{
  return mErrorCode;
}

nsresult
nsPrefMigration::ConvertPersistentStringToFileSpec(const char *str, nsIFileSpec *path)
{
	nsresult rv;
	if (!str || !path) return NS_ERROR_NULL_POINTER;
	
	rv = path->SetPersistentDescriptorString(str);
	return rv;
}
     
/*--------------------------------------------------------------------------
 * ProcessPrefsCallback is the primary funtion for the class nsPrefMigration.
 *
 * Called by: The Profile Manager (nsProfile.cpp)
 * INPUT: The specific profile path (prefPath) and the 5.0 installed path
 * OUTPUT: The modified 5.0 prefs files
 * RETURN: Success or a failure code
 *
 *-------------------------------------------------------------------------*/
nsresult
nsPrefMigration::ProcessPrefsCallback(const char* oldProfilePathStr, const char * newProfilePathStr)
{ 
  nsresult rv;
  
  nsCOMPtr<nsIFileSpec> oldProfilePath;
  nsCOMPtr<nsIFileSpec> newProfilePath; 
  nsCOMPtr<nsIFileSpec> oldPOPMailPath;
  nsCOMPtr<nsIFileSpec> newPOPMailPath;
  nsCOMPtr<nsIFileSpec> oldIMAPMailPath;
  nsCOMPtr<nsIFileSpec> newIMAPMailPath;
  nsCOMPtr<nsIFileSpec> oldIMAPLocalMailPath;
  nsCOMPtr<nsIFileSpec> newIMAPLocalMailPath;
  nsCOMPtr<nsIFileSpec> oldNewsPath;
  nsCOMPtr<nsIFileSpec> newNewsPath;
  nsCOMPtr<nsILocalFile> newPrefsFile;
#ifdef HAVE_MOVEMAIL
  nsCOMPtr<nsIFileSpec> oldMOVEMAILMailPath;
  nsCOMPtr<nsIFileSpec> newMOVEMAILMailPath;
#endif /* HAVE_MOVEMAIL */
  PRBool exists                  = PR_FALSE, 
         enoughSpace             = PR_TRUE,
         localMailDriveDefault   = PR_FALSE,
         summaryMailDriveDefault = PR_FALSE,
         newsDriveDefault        = PR_FALSE,
         copyMailFileInMigration = PR_TRUE;

  nsFileSpec localMailSpec,
             summaryMailSpec,
             newsSpec, 
             oldProfileSpec, newProfileSpec;

  PRInt32 serverType = POP_4X_MAIL_TYPE; 
  char *popServerName = nsnull;

  PRUint32 totalLocalMailSize = 0,
           totalSummaryFileSize = 0,
           totalNewsSize = 0, 
           totalProfileSize = 0,
           totalRequired = 0;


  PRInt64  localMailDrive   = LL_Zero(),
           summaryMailDrive = LL_Zero(),
           newsDrive        = LL_Zero(),
           profileDrive     = LL_Zero();

  PRInt64  DriveID[MAX_DRIVES];
  PRUint32 SpaceRequired[MAX_DRIVES];
  
#if defined(NS_DEBUG)
  printf("*Entered Actual Migration routine*\n");
#endif

  for (int i=0; i < MAX_DRIVES; i++)
  {
    DriveID[i] = LL_Zero();
    SpaceRequired[i] = 0;
  }
  
  rv = getPrefService();
  if (NS_FAILED(rv)) return rv;

  rv = NS_NewFileSpec(getter_AddRefs(oldProfilePath));
  if (NS_FAILED(rv)) return rv;
  rv = NS_NewFileSpec(getter_AddRefs(newProfilePath));
  if (NS_FAILED(rv)) return rv;
      
  rv = ConvertPersistentStringToFileSpec(oldProfilePathStr, oldProfilePath);
  if (NS_FAILED(rv)) return rv;
  rv = ConvertPersistentStringToFileSpec(newProfilePathStr, newProfilePath);
  if (NS_FAILED(rv)) return rv;

  oldProfilePath->GetFileSpec(&oldProfileSpec);
  newProfilePath->GetFileSpec(&newProfileSpec);
  

  /* initialize prefs with the old prefs.js file (which is a copy of the 4.x preferences file) */
  nsCOMPtr<nsIFileSpec> PrefsFile4x;

  //Get the location of the 4.x prefs file
  rv = NS_NewFileSpec(getter_AddRefs(PrefsFile4x));
  if (NS_FAILED(rv)) return rv;
  
  rv = PrefsFile4x->FromFileSpec(oldProfilePath);
  if (NS_FAILED(rv)) return rv;

  rv = PrefsFile4x->AppendRelativeUnixPath(PREF_FILE_NAME_IN_4x);
  if (NS_FAILED(rv)) return rv;

  //Need to convert PrefsFile4x to an IFile in order to copy it to a 
  //unique name in the system temp directory.
  nsFileSpec PrefsFile4xAsFileSpec;
  rv = PrefsFile4x->GetFileSpec(&PrefsFile4xAsFileSpec);
  if (NS_FAILED(rv)) return rv;
  
  nsCOMPtr<nsILocalFile> PrefsFile4xAsIFile;
  rv = NS_FileSpecToIFile(&PrefsFile4xAsFileSpec,
                     getter_AddRefs(PrefsFile4xAsIFile));
  if (NS_FAILED(rv)) return rv;

  nsCOMPtr<nsIFile> systemTempDir;
  rv = NS_GetSpecialDirectory(NS_OS_TEMP_DIR, getter_AddRefs(systemTempDir));
  if (NS_FAILED(rv)) return rv;

  systemTempDir->AppendNative(NS_LITERAL_CSTRING("migrate"));
  
  //Create a unique directory in the system temp dir based on the name of the 4.x prefs file
  rv = systemTempDir->CreateUnique(nsIFile::DIRECTORY_TYPE, 0700); 
  if (NS_FAILED(rv)) return rv;

  rv = PrefsFile4xAsIFile->CopyToNative(systemTempDir, NS_LITERAL_CSTRING(PREF_FILE_NAME_IN_4x));
  if (NS_FAILED(rv)) return rv;
  
  nsCOMPtr<nsIFile> cloneFile;
  rv = systemTempDir->Clone(getter_AddRefs(cloneFile));
  if (NS_FAILED(rv)) return rv;

  m_prefsFile = do_QueryInterface(cloneFile, &rv);
  if (NS_FAILED(rv)) return rv;

  rv = m_prefsFile->AppendNative(NS_LITERAL_CSTRING(PREF_FILE_NAME_IN_4x));
  if (NS_FAILED(rv)) return rv;

  nsCOMPtr<nsIPrefService> psvc(do_QueryInterface(m_prefBranch));

  //Clear the prefs in case a previous set was read in.
  psvc->ResetPrefs();

  //Now read the prefs from the prefs file in the system directory
  psvc->ReadUserPrefs(m_prefsFile);

  //
  // Start computing the sizes required for migration
  //
  rv = GetSizes(oldProfileSpec, PR_FALSE, &totalProfileSize);
  profileDrive = newProfileSpec.GetDiskSpaceAvailable();

  rv = m_prefBranch->GetIntPref(PREF_MAIL_SERVER_TYPE, &serverType);
  if (NS_FAILED(rv)) return rv;

  // get the migration mode for mail
  rv = m_prefBranch->GetBoolPref(PREF_MIGRATION_MODE_FOR_MAIL,
                                 &copyMailFileInMigration);
  if (NS_FAILED(rv))
    return rv;

  if (serverType == POP_4X_MAIL_TYPE) {
    summaryMailDriveDefault = PR_TRUE; //summary files are only used in IMAP so just set it to true here.
    summaryMailDrive = profileDrive;   //just set the drive for summary files to be the same as the new profile

    rv = NS_NewFileSpec(getter_AddRefs(newPOPMailPath));
    if (NS_FAILED(rv)) return rv;

    rv = NS_NewFileSpec(getter_AddRefs(oldPOPMailPath));
    if (NS_FAILED(rv)) return rv;
    
    rv = GetDirFromPref(oldProfilePath,newProfilePath,NEW_MAIL_DIR_NAME, PREF_MAIL_DIRECTORY, newPOPMailPath, oldPOPMailPath);
    if (NS_FAILED(rv)) {
      rv = DetermineOldPath(oldProfilePath, OLD_MAIL_DIR_NAME, "mailDirName", oldPOPMailPath);
      if (NS_FAILED(rv)) return rv;

      rv = SetPremigratedFilePref(PREF_MAIL_DIRECTORY, oldPOPMailPath);
      if (NS_FAILED(rv)) return rv;

      rv = newPOPMailPath->FromFileSpec(newProfilePath);
      if (NS_FAILED(rv)) return rv;

      localMailDriveDefault = PR_TRUE;
    }
    oldPOPMailPath->GetFileSpec(&localMailSpec);
    rv = GetSizes(localMailSpec, PR_TRUE, &totalLocalMailSize);
    localMailDrive = localMailSpec.GetDiskSpaceAvailable();
  }
  else if(serverType == IMAP_4X_MAIL_TYPE) {
    rv = NS_NewFileSpec(getter_AddRefs(newIMAPLocalMailPath));
    if (NS_FAILED(rv)) return rv;
      
    rv = NS_NewFileSpec(getter_AddRefs(oldIMAPLocalMailPath));
    if (NS_FAILED(rv)) return rv;
        
    /* First get the actual 4.x "Local Mail" files location */
    rv = GetDirFromPref(oldProfilePath,newProfilePath, NEW_MAIL_DIR_NAME, PREF_MAIL_DIRECTORY, newIMAPLocalMailPath, oldIMAPLocalMailPath);
    if (NS_FAILED(rv)) {
      rv = DetermineOldPath(oldProfilePath, OLD_MAIL_DIR_NAME, "mailDirName", oldIMAPLocalMailPath);
      if (NS_FAILED(rv)) return rv;

      rv = SetPremigratedFilePref(PREF_MAIL_DIRECTORY, oldIMAPLocalMailPath);
      if (NS_FAILED(rv)) return rv;

      rv = newIMAPLocalMailPath->FromFileSpec(newProfilePath);
      if (NS_FAILED(rv)) return rv;
      
      localMailDriveDefault = PR_TRUE;
    }

    oldIMAPLocalMailPath->GetFileSpec(&localMailSpec);
    rv = GetSizes(localMailSpec, PR_TRUE, &totalLocalMailSize);
    localMailDrive = localMailSpec.GetDiskSpaceAvailable();

    /* Next get IMAP mail summary files location */
    rv = NS_NewFileSpec(getter_AddRefs(newIMAPMailPath));
    if (NS_FAILED(rv)) return rv;
    
    rv = NS_NewFileSpec(getter_AddRefs(oldIMAPMailPath));
    if (NS_FAILED(rv)) return rv;

    rv = GetDirFromPref(oldProfilePath,newProfilePath, NEW_IMAPMAIL_DIR_NAME, PREF_MAIL_IMAP_ROOT_DIR,newIMAPMailPath,oldIMAPMailPath);
    if (NS_FAILED(rv)) {
      rv = oldIMAPMailPath->FromFileSpec(oldProfilePath);
      if (NS_FAILED(rv)) return rv;
        
      /* we didn't over localize "ImapMail" in 4.x, so this is all we have to do */
      rv = oldIMAPMailPath->AppendRelativeUnixPath(OLD_IMAPMAIL_DIR_NAME);
      if (NS_FAILED(rv)) return rv;

      rv = SetPremigratedFilePref(PREF_MAIL_IMAP_ROOT_DIR, oldIMAPMailPath);
      if (NS_FAILED(rv)) return rv;   
      
      rv = newIMAPMailPath->FromFileSpec(newProfilePath);
      if (NS_FAILED(rv)) return rv;

      summaryMailDriveDefault = PR_TRUE;
    }

    oldIMAPMailPath->GetFileSpec(&summaryMailSpec);
    rv = GetSizes(summaryMailSpec, PR_TRUE, &totalSummaryFileSize);
    summaryMailDrive = summaryMailSpec.GetDiskSpaceAvailable();
  }   

#ifdef HAVE_MOVEMAIL
  else if (serverType == MOVEMAIL_4X_MAIL_TYPE) {
    
    summaryMailDriveDefault = PR_TRUE;
    summaryMailDrive = profileDrive;

    rv = NS_NewFileSpec(getter_AddRefs(newMOVEMAILMailPath));
    if (NS_FAILED(rv)) return rv;

    rv = NS_NewFileSpec(getter_AddRefs(oldMOVEMAILMailPath));
    if (NS_FAILED(rv)) return rv;
    
    rv = GetDirFromPref(oldProfilePath,newProfilePath,NEW_MAIL_DIR_NAME, PREF_MAIL_DIRECTORY, newMOVEMAILMailPath, oldMOVEMAILMailPath);
    if (NS_FAILED(rv)) {
      rv = oldMOVEMAILMailPath->FromFileSpec(oldProfilePath);
      if (NS_FAILED(rv)) return rv;

      /* we didn't over localize this in 4.x, so this is all we have to do */
      rv = oldMOVEMAILMailPath->AppendRelativeUnixPath(OLD_MAIL_DIR_NAME);
      if (NS_FAILED(rv)) return rv;
      
      rv = SetPremigratedFilePref(PREF_MAIL_DIRECTORY, oldMOVEMAILMailPath);
      if (NS_FAILED(rv)) return rv;

      rv = newMOVEMAILMailPath->FromFileSpec(newProfilePath);
      if (NS_FAILED(rv)) return rv;

      localMailDriveDefault = PR_TRUE;
    }
    oldMOVEMAILMailPath->GetFileSpec(&localMailSpec);
    rv = GetSizes(localMailSpec, PR_TRUE, &totalLocalMailSize);

    localMailDrive = localMailSpec.GetDiskSpaceAvailable();
   
  }    
#endif //HAVE_MOVEMAIL

    ////////////////////////////////////////////////////////////////////////////
    // Now get the NEWS disk space requirements for migration.
    ////////////////////////////////////////////////////////////////////////////
    rv = NS_NewFileSpec(getter_AddRefs(newNewsPath));
    if (NS_FAILED(rv)) return rv;
    
    rv = NS_NewFileSpec(getter_AddRefs(oldNewsPath));
    if (NS_FAILED(rv)) return rv;
    
    rv = GetDirFromPref(oldProfilePath,newProfilePath, NEW_NEWS_DIR_NAME, PREF_NEWS_DIRECTORY, newNewsPath,oldNewsPath);
    if (NS_FAILED(rv)) {
      rv = DetermineOldPath(oldProfilePath, OLD_NEWS_DIR_NAME, "newsDirName", oldNewsPath);
      if (NS_FAILED(rv)) return rv;

      rv = SetPremigratedFilePref(PREF_NEWS_DIRECTORY, oldNewsPath);
      if (NS_FAILED(rv)) return rv; 

      rv = newNewsPath->FromFileSpec(newProfilePath);
      if (NS_FAILED(rv)) return rv;

      newsDriveDefault = PR_TRUE;
    }
    oldNewsPath->GetFileSpec(&newsSpec);
    rv = GetSizes(newsSpec, PR_TRUE, &totalNewsSize);
    newsDrive = newsSpec.GetDiskSpaceAvailable();

    // 
    // Compute the space needed to migrate the profile
    //
    if(newsDriveDefault && localMailDriveDefault && summaryMailDriveDefault) // DEFAULT: All on the same drive
    {
      totalRequired = totalNewsSize + totalLocalMailSize + totalSummaryFileSize + totalProfileSize;
      rv = ComputeSpaceRequirements(DriveID, SpaceRequired, profileDrive, totalRequired);
      if (NS_FAILED(rv))
        enoughSpace = PR_FALSE;
    }
    else
    {
      rv = ComputeSpaceRequirements(DriveID, SpaceRequired, profileDrive, totalProfileSize);
      if (NS_FAILED(rv))
        enoughSpace = PR_FALSE;
      rv = ComputeSpaceRequirements(DriveID, SpaceRequired, localMailDrive, totalLocalMailSize);
      if (NS_FAILED(rv))
        enoughSpace = PR_FALSE;
      rv = ComputeSpaceRequirements(DriveID, SpaceRequired, summaryMailDrive, totalSummaryFileSize);
      if (NS_FAILED(rv))
        enoughSpace = PR_FALSE;
      rv = ComputeSpaceRequirements(DriveID, SpaceRequired, newsDrive, totalNewsSize);
      if (NS_FAILED(rv))
        enoughSpace = PR_FALSE;
    }

    if (!enoughSpace)
    {
      mErrorCode = MIGRATION_RETRY; 
      return NS_OK;
    }

  ////////////////////////////////////////////////////////////////////////////
  // If we reached this point, there is enough room to do a migration.
  // Start creating directories and setting new pref values.
  ////////////////////////////////////////////////////////////////////////////

  /* Create the new profile tree for 5.x */
  rv = CreateNewUser5Tree(oldProfilePath, newProfilePath);
  if (NS_FAILED(rv)) return rv;

  
  if (serverType == POP_4X_MAIL_TYPE) {

    rv = newPOPMailPath->Exists(&exists);
    if (NS_FAILED(rv)) return rv;
    if (!exists)  {
      rv = newPOPMailPath->CreateDir();
      if (NS_FAILED(rv)) return rv;
    }

    rv = newPOPMailPath->AppendRelativeUnixPath(NEW_MAIL_DIR_NAME);
    if (NS_FAILED(rv)) return rv;
 
    rv = newPOPMailPath->Exists(&exists);
    if (NS_FAILED(rv)) return rv;
    if (!exists)  {
      rv = newPOPMailPath->CreateDir();
      if (NS_FAILED(rv)) return rv;
    }

    {
      // temporarily go through nsFileSpec
      nsFileSpec newPOPMailPathSpec;
      newPOPMailPath->GetFileSpec(&newPOPMailPathSpec);
      
      nsCOMPtr<nsILocalFile> newPOPMailPathFile;
      NS_FileSpecToIFile(&newPOPMailPathSpec,
                         getter_AddRefs(newPOPMailPathFile));
      
      rv = m_prefBranch->SetComplexValue(PREF_MAIL_DIRECTORY,
                                         NS_GET_IID(nsILocalFile),
                                         newPOPMailPathFile);
      if (NS_FAILED(rv)) return rv;
    }

    m_prefBranch->GetCharPref(PREF_NETWORK_HOSTS_POP_SERVER, &popServerName);

    nsCAutoString popServerNamewithoutPort(popServerName);
    PRInt32 colonPos = popServerNamewithoutPort.FindChar(':');

    if (colonPos != -1 ) {
	popServerNamewithoutPort.Truncate(colonPos);
	rv = newPOPMailPath->AppendRelativeUnixPath(popServerNamewithoutPort.get());
    }
    else {
	rv = newPOPMailPath->AppendRelativeUnixPath(popServerName);
    }

    if (NS_FAILED(rv)) return rv;				  
    
    rv = newPOPMailPath->Exists(&exists);
    if (NS_FAILED(rv)) return rv;
    if (!exists)  {
      rv = newPOPMailPath->CreateDir();
      if (NS_FAILED(rv)) return rv;
    }
  }
  else if (serverType == IMAP_4X_MAIL_TYPE) {
   if( copyMailFileInMigration )  // copy mail files in migration 
   {
    rv = newIMAPLocalMailPath->Exists(&exists);
    if (NS_FAILED(rv)) return rv;
    if (!exists)  {
      rv = newIMAPLocalMailPath->CreateDir();
      if (NS_FAILED(rv)) return rv;
    }
      
    rv = newIMAPLocalMailPath->AppendRelativeUnixPath(NEW_MAIL_DIR_NAME);
    if (NS_FAILED(rv)) return rv;

    /* Now create the new "Mail/Local Folders" directory */
    rv = newIMAPLocalMailPath->Exists(&exists);
    if (NS_FAILED(rv)) return rv;
    if (!exists)  {
      newIMAPLocalMailPath->CreateDir();
    }

    {
      // temporarily go through nsFileSpec
      nsFileSpec newIMAPLocalMailPathSpec;
      newIMAPLocalMailPath->GetFileSpec(&newIMAPLocalMailPathSpec);
      
      nsCOMPtr<nsILocalFile> newIMAPLocalMailPathFile;
      NS_FileSpecToIFile(&newIMAPLocalMailPathSpec,
                         getter_AddRefs(newIMAPLocalMailPathFile));
      
      rv = m_prefBranch->SetComplexValue(PREF_MAIL_DIRECTORY,
                                         NS_GET_IID(nsILocalFile),
                                         newIMAPLocalMailPathFile);
      if (NS_FAILED(rv)) return rv;
    }

    rv = newIMAPLocalMailPath->AppendRelativeUnixPath(NEW_LOCAL_MAIL_DIR_NAME);
    if (NS_FAILED(rv)) return rv;
    rv = newIMAPLocalMailPath->Exists(&exists);
    if (NS_FAILED(rv)) return rv;
    if (!exists)  {
      rv = newIMAPLocalMailPath->CreateDir();
      if (NS_FAILED(rv)) return rv;
    }

    /* Now deal with the IMAP mail summary file location */
    rv = newIMAPMailPath->Exists(&exists);
    if (NS_FAILED(rv)) return rv;
    if (!exists)  {
      rv = newIMAPMailPath->CreateDir();
      if (NS_FAILED(rv)) return rv;
    }

    rv = newIMAPMailPath->AppendRelativeUnixPath(NEW_IMAPMAIL_DIR_NAME);
    if (NS_FAILED(rv)) return rv;

    rv = newIMAPMailPath->Exists(&exists);
    if (NS_FAILED(rv)) return rv;
    if (!exists)  {
      rv = newIMAPMailPath->CreateDir();
      if (NS_FAILED(rv)) return rv;
    }

    {
      // temporarily go through nsFileSpec
      nsFileSpec newIMAPMailPathSpec;
      newIMAPMailPath->GetFileSpec(&newIMAPMailPathSpec);
      
      nsCOMPtr<nsILocalFile> newIMAPMailPathFile;
      NS_FileSpecToIFile(&newIMAPMailPathSpec,
                         getter_AddRefs(newIMAPMailPathFile));
      
      rv = m_prefBranch->SetComplexValue(PREF_MAIL_IMAP_ROOT_DIR,
                                         NS_GET_IID(nsILocalFile),
                                         newIMAPMailPathFile);
      if (NS_FAILED(rv)) return rv;
    }
   }
   else
   {
    {
      // temporarily go through nsFileSpec
      nsFileSpec oldIMAPLocalMailPathSpec;
      oldIMAPLocalMailPath->GetFileSpec(&oldIMAPLocalMailPathSpec);

      nsCOMPtr<nsILocalFile> oldIMAPLocalMailPathFile;
      NS_FileSpecToIFile(&oldIMAPLocalMailPathSpec,
                         getter_AddRefs(oldIMAPLocalMailPathFile));

      rv = m_prefBranch->SetComplexValue(PREF_MAIL_DIRECTORY,
                                         NS_GET_IID(nsILocalFile),
                                         oldIMAPLocalMailPathFile);
      if (NS_FAILED(rv)) return rv;
    }
    {
      // temporarily go through nsFileSpec
      nsFileSpec oldIMAPMailPathSpec;
      oldIMAPMailPath->GetFileSpec(&oldIMAPMailPathSpec);

      nsCOMPtr<nsILocalFile> oldIMAPMailPathFile;
      NS_FileSpecToIFile(&oldIMAPMailPathSpec,
                         getter_AddRefs(oldIMAPMailPathFile));

      rv = m_prefBranch->SetComplexValue(PREF_MAIL_IMAP_ROOT_DIR,
                                         NS_GET_IID(nsILocalFile),
                                         oldIMAPMailPathFile);
      if (NS_FAILED(rv)) return rv;
    }
   }
  }

#ifdef HAVE_MOVEMAIL
  else if (serverType == MOVEMAIL_4X_MAIL_TYPE) {

    rv = newMOVEMAILMailPath->Exists(&exists);
    if (NS_FAILED(rv)) return rv;
    if (!exists)  {
      rv = newMOVEMAILMailPath->CreateDir();
      if (NS_FAILED(rv)) return rv;
    }

    rv = newMOVEMAILMailPath->AppendRelativeUnixPath(NEW_MAIL_DIR_NAME);
    if (NS_FAILED(rv)) return rv;

    rv = newMOVEMAILMailPath->Exists(&exists);
    if (NS_FAILED(rv)) return rv;
    if (!exists)  {
      rv = newMOVEMAILMailPath->CreateDir();
      if (NS_FAILED(rv)) return rv;
    }

    {
      // temporarily go through nsFileSpec
      nsFileSpec newMOVEMAILPathSpec;
      newMOVEMAILMailPath->GetFileSpec(&newMOVEMAILPathSpec);
      
      nsCOMPtr<nsILocalFile> newMOVEMAILPathFile;
      NS_FileSpecToIFile(&newMOVEMAILPathSpec,
                         getter_AddRefs(newMOVEMAILPathFile));
      
      rv = m_prefBranch->SetComplexValue(PREF_MAIL_DIRECTORY,
                                         NS_GET_IID(nsILocalFile),
                                         newMOVEMAILPathFile); 
      if (NS_FAILED(rv)) return rv;
    }

    rv = newMOVEMAILMailPath->AppendRelativeUnixPath(NEW_MOVEMAIL_DIR_NAME);
    if (NS_FAILED(rv)) return rv;

    rv = newMOVEMAILMailPath->Exists(&exists);
    if (NS_FAILED(rv)) return rv;
    if (!exists)  {
      rv = newMOVEMAILMailPath->CreateDir();
      if (NS_FAILED(rv)) return rv;
    }
    rv = NS_OK;
  }
#endif /* HAVE_MOVEMAIL */
  else {
    NS_ASSERTION(0,"failure, didn't recognize your mail server type.\n");
    return NS_ERROR_UNEXPECTED;
  }
  
  ////////////////////////////////////////////////////////////////////////////
  // Set all the appropriate NEWS prefs.
  ////////////////////////////////////////////////////////////////////////////

  rv = newNewsPath->Exists(&exists);
  if (NS_FAILED(rv)) return rv;
  if (!exists)  {
    rv = newNewsPath->CreateDir();
    if (NS_FAILED(rv)) return rv;
  }

  rv = newNewsPath->AppendRelativeUnixPath(NEW_NEWS_DIR_NAME);
  if (NS_FAILED(rv)) return rv;

  rv = newNewsPath->Exists(&exists);
  if (NS_FAILED(rv)) return rv;
  if (!exists)  {
    rv = newNewsPath->CreateDir();
    if (NS_FAILED(rv)) return rv;
  }

  {
    // temporarily go through nsFileSpec
    nsFileSpec newNewsPathSpec;
    newNewsPath->GetFileSpec(&newNewsPathSpec);
    
    nsCOMPtr<nsILocalFile> newNewsPathFile;
    NS_FileSpecToIFile(&newNewsPathSpec,
                       getter_AddRefs(newNewsPathFile));
    
    rv = m_prefBranch->SetComplexValue(PREF_NEWS_DIRECTORY,
                                       NS_GET_IID(nsILocalFile),
                                       newNewsPathFile); 
    if (NS_FAILED(rv)) return rv;
  }

  PRBool needToRenameFilterFiles;
  if (PL_strcmp(IMAP_MAIL_FILTER_FILE_NAME_IN_4x,IMAP_MAIL_FILTER_FILE_NAME_IN_5x)) {
#ifdef IMAP_MAIL_FILTER_FILE_NAME_FORMAT_IN_4x
    // if we defined a format, the filter files don't live in the host directories
    // (mac does this.)  we'll take care of those filter files later, in DoSpecialUpdates()
    needToRenameFilterFiles = PR_FALSE;
#else
    needToRenameFilterFiles = PR_TRUE;
#endif /* IMAP_MAIL_FILTER_FILE_NAME_FORMAT_IN_4x */
  }
  else {
    // if the name was the same in 4x as in 5x, no need to rename it
    needToRenameFilterFiles = PR_FALSE;
  }
  
  // just copy what we need
  rv = DoTheCopy(oldProfilePath, newProfilePath, COOKIES_FILE_NAME_IN_4x);
  if (NS_FAILED(rv)) return rv;
  rv = DoTheCopy(oldProfilePath, newProfilePath, BOOKMARKS_FILE_NAME_IN_4x);
  if (NS_FAILED(rv)) return rv;
#if defined(XP_MACOSX)
  rv = DoTheCopy(oldProfilePath, newProfilePath, SECURITY_PATH, PR_TRUE);
  if (NS_FAILED(rv)) return rv;
#else
  rv = DoTheCopy(oldProfilePath, newProfilePath, PSM_CERT7_DB);
  if (NS_FAILED(rv)) return rv;
  rv = DoTheCopy(oldProfilePath, newProfilePath, PSM_KEY3_DB);
  if (NS_FAILED(rv)) return rv;
  rv = DoTheCopy(oldProfilePath, newProfilePath, PSM_SECMODULE_DB);
  if (NS_FAILED(rv)) return rv;
#endif /* XP_MACOSX */

#if defined(XP_MACOSX)
  // Copy the Mac filter rule files which sits at the top level dir of a 4.x profile.
  if(serverType == IMAP_4X_MAIL_TYPE) {
    rv = CopyFilesByPattern(oldProfilePath, newProfilePath, MAC_RULES_FILE_ENDING_STRING_IN_4X);
    NS_ENSURE_SUCCESS(rv,rv);
  }
#endif

  rv = DoTheCopy(oldNewsPath, newNewsPath, PR_TRUE);
  if (NS_FAILED(rv)) return rv;

#ifdef NEED_TO_COPY_AND_RENAME_NEWSRC_FILES
  /* in 4.x, the newsrc files were in $HOME.  Now that we can have multiple
   * profiles in 5.x, with the same user, this won't fly.
   * when they migrate, we need to copy from $HOME/.newsrc-<host> to
   * ~/.mozilla/<profile>/News/newsrc-<host>
   */
  rv = CopyAndRenameNewsrcFiles(newNewsPath);
  if (NS_FAILED(rv)) return rv;
#endif /* NEED_TO_COPY_AND_RENAME_NEWSRC_FILES */

  if (serverType == IMAP_4X_MAIL_TYPE) {
    if( copyMailFileInMigration )  // copy mail files in migration
    {
    rv = DoTheCopyAndRename(oldIMAPMailPath, newIMAPMailPath, PR_TRUE, needToRenameFilterFiles, IMAP_MAIL_FILTER_FILE_NAME_IN_4x, IMAP_MAIL_FILTER_FILE_NAME_IN_5x);
    if (NS_FAILED(rv)) return rv;
    rv = DoTheCopyAndRename(oldIMAPLocalMailPath, newIMAPLocalMailPath, PR_TRUE, needToRenameFilterFiles,IMAP_MAIL_FILTER_FILE_NAME_IN_4x,IMAP_MAIL_FILTER_FILE_NAME_IN_5x);
    if (NS_FAILED(rv)) return rv;
    }
    else  // Copy & Rename filter files
    {
      // IMAP path
      // don't care if this fails
      (void)DoTheCopyAndRename(oldIMAPMailPath, PR_TRUE, IMAP_MAIL_FILTER_FILE_NAME_IN_4x, IMAP_MAIL_FILTER_FILE_NAME_IN_5x);
      
      // Local Folders path
      // don't care if this fails
      (void)DoTheCopyAndRename(oldIMAPLocalMailPath, PR_TRUE, IMAP_MAIL_FILTER_FILE_NAME_IN_4x, IMAP_MAIL_FILTER_FILE_NAME_IN_5x);
    }
  }
  else if (serverType == POP_4X_MAIL_TYPE) {
    // fix for bug #202010
    // copy over the pop filter and popstate files now
    // and later, in DoSpecialUpdates()
    // we'll move and rename them
#ifdef POP_MAIL_FILTER_FILE_NAME_IN_4x
    rv = DoTheCopy(oldProfilePath, newProfilePath, POP_MAIL_FILTER_FILE_NAME_IN_4x);
    if (NS_FAILED(rv)) return rv;
#endif
    
#ifdef POPSTATE_FILE_IN_4x 
    rv = DoTheCopy(oldProfilePath, newProfilePath, POPSTATE_FILE_IN_4x);
    if (NS_FAILED(rv)) return rv;
#endif
    
    rv = DoTheCopy(oldPOPMailPath, newPOPMailPath, PR_TRUE);
    if (NS_FAILED(rv)) return rv;
  }
#ifdef HAVE_MOVEMAIL
  else if (serverType == MOVEMAIL_4X_MAIL_TYPE) {
    // in 4.x, the movemail filter name was the same as the pop filter name
    // copy over the filter file now
    // and later, in DoSpecialUpdates()
    // we'll move and rename them
    rv = DoTheCopy(oldProfilePath, newProfilePath, POP_MAIL_FILTER_FILE_NAME_IN_4x);
    if (NS_FAILED(rv)) return rv;
    
    rv = DoTheCopy(oldMOVEMAILMailPath, newMOVEMAILMailPath, PR_TRUE);
  }
#endif /* HAVE_MOVEMAIL */
  else {
    NS_ASSERTION(0, "unknown mail server type!");
    return NS_ERROR_FAILURE;
  }
  
  // Don't inherit the 4.x cache file location for mozilla!
  // The cache pref later gets set with a default in nsAppRunner::InitCachePrefs().
  m_prefBranch->ClearUserPref(PREF_BROWSER_CACHE_DIRECTORY);

  rv = DoSpecialUpdates(newProfilePath);
  if (NS_FAILED(rv)) return rv;
  PR_FREEIF(popServerName);

  nsXPIDLCString path;

  newProfilePath->GetNativePath(getter_Copies(path));
  NS_NewNativeLocalFile(path, PR_TRUE, getter_AddRefs(newPrefsFile));

  rv = newPrefsFile->AppendNative(NS_LITERAL_CSTRING(PREF_FILE_NAME_IN_5x));
  if (NS_FAILED(rv)) return rv;

  rv=psvc->SavePrefFile(newPrefsFile);
  if (NS_FAILED(rv)) return rv;
  rv=psvc->ResetPrefs();
  if (NS_FAILED(rv)) return rv;

  PRBool flagExists = PR_FALSE;
  m_prefsFile->Exists(&flagExists); //Delete the prefs.js file in the temp directory.
  if (flagExists)
    m_prefsFile->Remove(PR_FALSE);
  
  systemTempDir->Exists(&flagExists); //Delete the unique dir in the system temp dir.
  if (flagExists)
    systemTempDir->Remove(PR_FALSE);

  return rv;
}


/*----------------------------------------------------------------------------
 * CreateNewUsers5Tree creates the directory called users5 (parent of the
 * of the profile directories) and the profile directory itself
 *---------------------------------------------------------------------------*/

nsresult
nsPrefMigration::CreateNewUser5Tree(nsIFileSpec * oldProfilePath, nsIFileSpec * newProfilePath)
{
  nsresult rv;
  PRBool exists;
  
  NS_ASSERTION(*PREF_FILE_NAME_IN_4x, "don't know how to migrate your platform");
  if (!*PREF_FILE_NAME_IN_4x) {
    return NS_ERROR_UNEXPECTED;
  }
      
  /* Copy the old prefs file to the new profile directory for modification and reading.  
     after copying it, rename it to pref.js, the 5.x pref file name on all platforms */
  nsCOMPtr<nsIFileSpec> oldPrefsFile;
  rv = NS_NewFileSpec(getter_AddRefs(oldPrefsFile)); 
  if (NS_FAILED(rv)) return rv;
  
  rv = oldPrefsFile->FromFileSpec(oldProfilePath);
  if (NS_FAILED(rv)) return rv;
  
  rv = oldPrefsFile->AppendRelativeUnixPath(PREF_FILE_NAME_IN_4x);
  if (NS_FAILED(rv)) return rv;


  /* the new prefs file */
  nsCOMPtr<nsIFileSpec> newPrefsFile;
  rv = NS_NewFileSpec(getter_AddRefs(newPrefsFile)); 
  if (NS_FAILED(rv)) return rv;
  
  rv = newPrefsFile->FromFileSpec(newProfilePath);
  if (NS_FAILED(rv)) return rv;
  
  rv = newPrefsFile->Exists(&exists);
  if (!exists)
  {
	  rv = newPrefsFile->CreateDir();
  }

  rv = oldPrefsFile->CopyToDir(newPrefsFile);
  NS_ASSERTION(NS_SUCCEEDED(rv),"failed to copy prefs file");

  rv = newPrefsFile->AppendRelativeUnixPath(PREF_FILE_NAME_IN_4x);
  rv = newPrefsFile->Rename(PREF_FILE_NAME_IN_5x);
 
  rv = getPrefService();
  if (NS_FAILED(rv)) return rv;

  return NS_OK;
}

/*---------------------------------------------------------------------------------
 * GetDirFromPref gets a directory based on a preference set in the 4.x
 * preferences file, adds a 5 and resets the preference.
 *
 * INPUT: 
 *         oldProfilePath - the path to the old 4.x profile directory.  
 *                          currently only used by UNIX
 *
 *         newProfilePath - the path to the 5.0 profile directory
 *                          currently only used by UNIX
 *
 *         newDirName     - the leaf name of the directory in the 5.0 world that corresponds to
 *                          this pref.  Examples:  "Mail", "ImapMail", "News".
 *                          only used on UNIX.
 *
 *         pref - the pref in the "dot" format (e.g. mail.directory)
 *
 * OUTPUT: newPath - The old path with a 5 added (on mac and windows)
 *                   the newProfilePath + "/" + newDirName (on UNIX)
 *         oldPath - The old path from the pref (if any)
 *
 *
 * RETURNS: NS_OK if the pref was successfully pulled from the prefs file
 *
 *--------------------------------------------------------------------------------*/
nsresult
nsPrefMigration::GetDirFromPref(nsIFileSpec * oldProfilePath, nsIFileSpec * newProfilePath, const char *newDirName, const char* pref, nsIFileSpec* newPath, nsIFileSpec* oldPath)
{
  nsresult rv;
  
  if (!oldProfilePath || !newProfilePath || !newDirName || !pref || !newPath || !oldPath) return NS_ERROR_NULL_POINTER;
  
  rv = getPrefService();
  if (NS_FAILED(rv)) return rv;  
  
  nsCOMPtr <nsIFileSpec> oldPrefPath;
  nsXPIDLCString oldPrefPathStr;
  rv = m_prefBranch->GetCharPref(pref, getter_Copies(oldPrefPathStr));
  if (NS_FAILED(rv)) return rv;
  
  // the default on the mac was "".  doing GetFileXPref on that would return
  // the current working directory, like viewer_debug.  yikes!
  if (oldPrefPathStr.IsEmpty()) {
  	rv = NS_ERROR_FAILURE;
  }
  if (NS_FAILED(rv)) return rv;
  
  nsCOMPtr <nsILocalFile> oldPrefPathFile;
  rv = m_prefBranch->GetComplexValue(pref, NS_GET_IID(nsILocalFile),
                                     getter_AddRefs(oldPrefPathFile));
  if (NS_FAILED(rv)) return rv;
  
  // convert nsILocalFile to nsIFileSpec
  rv = oldPrefPathFile->GetNativePath(oldPrefPathStr);
  if (NS_FAILED(rv)) return rv;

  rv = NS_NewFileSpec(getter_AddRefs(oldPrefPath));
  if (NS_FAILED(rv)) return rv;
  
  rv = oldPrefPath->SetNativePath(oldPrefPathStr);
  if (NS_FAILED(rv)) return rv;

  // oldPath will also needs the conversion from nsILocalFile
  // this is nasty, eventually we'll switch entirely over to nsILocalFile
  rv = oldPath->SetNativePath(oldPrefPathStr);
  if (NS_FAILED(rv)) return rv;

  
#ifdef XP_UNIX
	// what if they don't want to go to <profile>/<newDirName>?
	// what if unix users want "mail.directory" + "5" (like "~/ns_imap5")
	// or "mail.imap.root_dir" + "5" (like "~/nsmail5")?
	// should we let them?  no.  let's migrate them to
	// <profile>/Mail and <profile>/ImapMail
	// let's make all three platforms the same.
	if (PR_TRUE) {
#else
	nsCOMPtr <nsIFileSpec> oldPrefPathParent;
	rv = oldPrefPath->GetParent(getter_AddRefs(oldPrefPathParent));
	if (NS_FAILED(rv)) return rv;

	// if the pref pointed to the default directory
	// treat it as if the pref wasn't set
	// this way it will get migrated as the user expects
	PRBool pathsMatch;
	rv = oldProfilePath->Equals(oldPrefPathParent, &pathsMatch);
	if (NS_SUCCEEDED(rv) && pathsMatch) {
#endif /* XP_UNIX */
		rv = newPath->FromFileSpec(newProfilePath);
		if (NS_FAILED(rv)) return rv;
	}
	else {
		nsXPIDLCString leafname;
		rv = newPath->FromFileSpec(oldPath);
		if (NS_FAILED(rv)) return rv;
		rv = newPath->GetLeafName(getter_Copies(leafname));
		if (NS_FAILED(rv)) return rv;
		nsCString newleafname((const char *)leafname);
		newleafname += NEW_DIR_SUFFIX;
		rv = newPath->SetLeafName(newleafname.get());
		if (NS_FAILED(rv)) return rv;
	}

  rv = SetPremigratedFilePref(pref, oldPath);
  if (NS_FAILED(rv)) return rv;
  
#ifdef XP_UNIX
  /* on UNIX, we kept the newsrc files in "news.directory", (which was usually ~)
   * and the summary files in ~/.netscape/xover-cache
   * oldPath should point to ~/.netscape/xover-cache, not "news.directory"
   * but we want to save the old "news.directory" in "premigration.news.directory"
   * later, again for UNIX only, 
   * we will copy the .newsrc files (from "news.directory") into the new <profile>/News directory.
   * isn't this fun?  
   */
  if (PL_strcmp(PREF_NEWS_DIRECTORY, pref) == 0) {
    rv = oldPath->FromFileSpec(oldProfilePath);
    if (NS_FAILED(rv)) return rv;
    rv = oldPath->AppendRelativeUnixPath(OLD_NEWS_DIR_NAME);
    if (NS_FAILED(rv)) return rv;
  }
#endif /* XP_UNIX */
  return rv;
}

static PRBool
nsCStringEndsWith(nsCString& name, const char *ending)
{
  if (!ending) return PR_FALSE;

  PRInt32 len = name.Length();
  if (len == 0) return PR_FALSE;

  PRInt32 endingLen = PL_strlen(ending);
  if (len > endingLen && name.RFind(ending, PR_TRUE) == len - endingLen) {
        return PR_TRUE;
  }
  else {
        return PR_FALSE;
  }
}

#ifdef NEED_TO_COPY_AND_RENAME_NEWSRC_FILES
static PRBool
nsCStringStartsWith(nsCString& name, const char *starting)
{
	if (!starting) return PR_FALSE;
	PRInt32	len = name.Length();
	if (len == 0) return PR_FALSE;
	
	PRInt32 startingLen = PL_strlen(starting);
	if (len > startingLen && name.RFind(starting, PR_TRUE) == 0) {
		return PR_TRUE;
	}
	else {
		return PR_FALSE;
	}
}
#endif
 
/*---------------------------------------------------------------------------------
 * GetSizes reads the 4.x files in the profile tree and accumulates their sizes
 *
 * INPUT:
 *
 * OUPUT:
 *
 * RETURNS:
 *
 *--------------------------------------------------------------------------------*/
nsresult
nsPrefMigration::GetSizes(nsFileSpec inputPath, PRBool readSubdirs, PRUint32 *sizeTotal)
{
  nsCAutoString fileOrDirNameStr;

  for (nsDirectoryIterator dir(inputPath, PR_FALSE); dir.Exists(); dir++)
  {
    nsFileSpec fileOrDirName = dir.Spec();
    char* folderName = fileOrDirName.GetLeafName();
    fileOrDirNameStr.Assign(folderName);
    if (!nsCStringEndsWith(fileOrDirNameStr, MAIL_SUMMARY_SUFFIX_IN_4x) && !nsCStringEndsWith(fileOrDirNameStr, NEWS_SUMMARY_SUFFIX_IN_4x) && !nsCStringEndsWith(fileOrDirNameStr, SUMMARY_SUFFIX_IN_5x)) /* Don't copy the summary files */
    {
      if (fileOrDirName.IsDirectory())
      {
        if(readSubdirs)
        {
          GetSizes(fileOrDirName, PR_TRUE, sizeTotal); /* re-enter the GetSizes function */
        }
      }
      else
        *sizeTotal += fileOrDirName.GetFileSize();
    }
    nsCRT::free(folderName);
  }

  return NS_OK;
}

/*---------------------------------------------------------------------------*
 * ComputeSpaceRequirments
 *
 *---------------------------------------------------------------------------*/
nsresult
nsPrefMigration::ComputeSpaceRequirements(PRInt64 DriveArray[MAX_DRIVES], 
                                          PRUint32 SpaceReqArray[MAX_DRIVES], 
                                          PRInt64 Drive, 
                                          PRUint32 SpaceNeeded)
{
  int i=0;
  PRFloat64 temp;

  while(LL_NE(DriveArray[i],LL_Zero()) && LL_NE(DriveArray[i], Drive) && i < MAX_DRIVES)
    i++;

  if (LL_EQ(DriveArray[i], LL_Zero()))
  {
    DriveArray[i] = Drive;
    SpaceReqArray[i] += SpaceNeeded;
  }
  else if (LL_EQ(DriveArray[i], Drive))
    SpaceReqArray[i] += SpaceNeeded;
  else
    return NS_ERROR_FAILURE;
  
  LL_L2F(temp, DriveArray[i]);
  if (SpaceReqArray[i] > temp)
    return NS_ERROR_FAILURE;
  
  return NS_OK;
}

#ifdef NEED_TO_COPY_AND_RENAME_NEWSRC_FILES
nsresult 
nsPrefMigration::CopyAndRenameNewsrcFiles(nsIFileSpec * newPathSpec)
{
  nsresult rv;
  nsCOMPtr <nsIFileSpec>oldPathSpec;
  nsFileSpec oldPath;
  nsFileSpec newPath;
  nsCAutoString fileOrDirNameStr;

  rv = GetPremigratedFilePref(PREF_NEWS_DIRECTORY, getter_AddRefs(oldPathSpec));
  if (NS_FAILED(rv)) return rv;
  rv = oldPathSpec->GetFileSpec(&oldPath);
  if (NS_FAILED(rv)) return rv;
  rv = newPathSpec->GetFileSpec(&newPath);
  if (NS_FAILED(rv)) return rv;

  for (nsDirectoryIterator dir(oldPath, PR_FALSE); dir.Exists(); dir++)
  {
    nsFileSpec fileOrDirName = dir.Spec(); //set first file or dir to a nsFileSpec
    //get the filename without the full path
    char* folderName = fileOrDirName.GetLeafName();
    fileOrDirNameStr.Assign(folderName);

    if (nsCStringStartsWith(fileOrDirNameStr, NEWSRC_PREFIX_IN_4x) || nsCStringStartsWith(fileOrDirNameStr, SNEWSRC_PREFIX_IN_4x)) {
#ifdef DEBUG_seth
	    printf("newsrc file == %s\n",folderName);
#endif /* DEBUG_seth */

	    rv = fileOrDirName.CopyToDir(newPath);
        NS_ASSERTION(NS_SUCCEEDED(rv),"failed to copy news file");

        nsFileSpec newFile = newPath;
        newFile += fileOrDirNameStr.get();
        newFile.Rename(folderName + 1); /* rename .newsrc-news to newsrc-news, no need to keep it hidden anymore */
    }
    nsCRT::free(folderName);
  }

  return NS_OK;
}
#endif /* NEED_TO_COPY_AND_RENAME_NEWSRC_FILES */

/*-------------------------------------------------------------------------
 * DoTheCopyAndRename copies the files listed in oldPath to newPath
 *                    and renames files, if necessary
 *
 * INPUT: oldPath - The old profile path plus the specific data type 
 *                  (e.g. mail or news)
 *        newPath - The new profile path plus the specific data type
 *
 *        readSubdirs
 *
 *        needToRenameFiles - do we need to search for files named oldFile
 *                            and rename them to newFile
 *
 *        oldFile           - old file name (used for renaming)
 *
 *        newFile           - new file name (used for renaming)
 *
 * RETURNS: NS_OK if successful
 *          NS_ERROR_FAILURE if failed
 *
 *--------------------------------------------------------------------------*/
nsresult
nsPrefMigration::DoTheCopyAndRename(nsIFileSpec * oldPathSpec, nsIFileSpec *newPathSpec, PRBool readSubdirs, PRBool needToRenameFiles, const char *oldName, const char *newName)
{
  nsresult rv;
  nsCAutoString fileOrDirNameStr;
  nsFileSpec oldPath;
  nsFileSpec newPath;
  
  rv = oldPathSpec->GetFileSpec(&oldPath);
  if (NS_FAILED(rv)) return rv;
  rv = newPathSpec->GetFileSpec(&newPath);
  if (NS_FAILED(rv)) return rv;
  
  for (nsDirectoryIterator dir(oldPath, PR_FALSE); dir.Exists(); dir++)
  {
    nsFileSpec fileOrDirName = dir.Spec(); //set first file or dir to a nsFileSpec
    //get the filename without the full path
    char* folderName = fileOrDirName.GetLeafName();
    fileOrDirNameStr.Assign(folderName);

    if (!nsCStringEndsWith(fileOrDirNameStr, MAIL_SUMMARY_SUFFIX_IN_4x) && !nsCStringEndsWith(fileOrDirNameStr, NEWS_SUMMARY_SUFFIX_IN_4x) && !nsCStringEndsWith(fileOrDirNameStr, SUMMARY_SUFFIX_IN_5x)) /* Don't copy the summary files */
    {
      if (fileOrDirName.IsDirectory())
      {
        if(readSubdirs)
        {
          nsCOMPtr<nsIFileSpec> newPathExtended;
          rv = NS_NewFileSpecWithSpec(newPath, getter_AddRefs(newPathExtended));
          rv = newPathExtended->AppendRelativeUnixPath(folderName);
          rv = newPathExtended->CreateDir();
          
          nsCOMPtr<nsIFileSpec>fileOrDirNameSpec;
          rv = NS_NewFileSpecWithSpec(fileOrDirName, getter_AddRefs(fileOrDirNameSpec));
          DoTheCopyAndRename(fileOrDirNameSpec, newPathExtended, PR_TRUE, needToRenameFiles, oldName, newName); /* re-enter the DoTheCopyAndRename function */
        }
      }
      else {
        // copy the file
        rv = fileOrDirName.CopyToDir(newPath);
        NS_ASSERTION(NS_SUCCEEDED(rv),"failed to copy file");

        if (needToRenameFiles) {
          // rename the file, if it matches
          if (fileOrDirNameStr.Equals(oldName)) {
            nsFileSpec newFile = newPath;
            newFile += fileOrDirNameStr.get();
            newFile.Rename(newName);
          }
        }
      }
    }
    nsCRT::free(folderName);
  }  
  
  return NS_OK;
}

/*-------------------------------------------------------------------------
 * DoTheCopyAndRename copies and renames files
 *
 * INPUT: aPath - the path
 *
 *        aReadSubdirs - if sub directories should be handled
 *
 *        aOldFile - old file name (used for renaming)
 *
 *        aNewFile - new file name (used for renaming)
 *
 * RETURNS: NS_OK if successful
 *          NS_ERROR_FAILURE if failed
 *
 *--------------------------------------------------------------------------*/
nsresult
nsPrefMigration::DoTheCopyAndRename(nsIFileSpec * aPathSpec, PRBool aReadSubdirs, const char *aOldName, const char *aNewName)
{
  if( !aOldName || !aNewName || !strcmp(aOldName, aNewName) )
    return NS_ERROR_FAILURE;

  nsresult rv;
  nsFileSpec path, file;
  
  rv = aPathSpec->GetFileSpec(&path);
  if (NS_FAILED(rv))
    return rv;
  rv = aPathSpec->GetFileSpec(&file);
  if (NS_FAILED(rv))
    return rv;
  file += aOldName;
  
  // Handle sub folders
  for (nsDirectoryIterator dir(path, PR_FALSE); dir.Exists(); dir++)
  {
    nsFileSpec fileOrDirName = dir.Spec(); //set first file or dir to a nsFileSpec
    if (fileOrDirName.IsDirectory())
    {
      if( aReadSubdirs )
      {
        nsCOMPtr<nsIFileSpec>fileOrDirNameSpec;
        rv = NS_NewFileSpecWithSpec(fileOrDirName, getter_AddRefs(fileOrDirNameSpec));
        DoTheCopyAndRename(fileOrDirNameSpec, aReadSubdirs, aOldName, aNewName); /* re-enter the DoTheCopyAndRename function */
      }
      else
        continue;
    }
  }

  nsCOMPtr<nsILocalFile> localFileOld, localFileDirectory;
  rv = NS_FileSpecToIFile(&file, getter_AddRefs(localFileOld));
  if (NS_FAILED(rv))
    return rv;
  rv = NS_FileSpecToIFile(&path, getter_AddRefs(localFileDirectory));
  if (NS_FAILED(rv))
    return rv;
  NS_ConvertUTF8toUTF16 newName(aNewName);
  localFileOld->CopyTo(localFileDirectory, newName);

  return NS_OK;
}

nsresult
nsPrefMigration::CopyFilesByPattern(nsIFileSpec * oldPathSpec, nsIFileSpec * newPathSpec, const char *pattern)
{
  nsFileSpec oldPath;
  nsFileSpec newPath;
  
  nsresult rv = oldPathSpec->GetFileSpec(&oldPath);
  NS_ENSURE_SUCCESS(rv,rv);
  rv = newPathSpec->GetFileSpec(&newPath);
  NS_ENSURE_SUCCESS(rv,rv);
  
  for (nsDirectoryIterator dir(oldPath, PR_FALSE); dir.Exists(); dir++)
  {
    nsFileSpec fileOrDirName = dir.Spec();    //set first file or dir to a nsFileSpec

    if (fileOrDirName.IsDirectory())
      continue;

    nsCAutoString fileOrDirNameStr(fileOrDirName.GetLeafName());
    if (!nsCStringEndsWith(fileOrDirNameStr, pattern))
      continue;

    // copy the file
    rv = fileOrDirName.CopyToDir(newPath);
    NS_ENSURE_SUCCESS(rv,rv);
  }  
  
  return NS_OK;
}

nsresult
nsPrefMigration::DoTheCopy(nsIFileSpec * oldPath, nsIFileSpec * newPath, PRBool readSubdirs)
{
  return DoTheCopyAndRename(oldPath, newPath, readSubdirs, PR_FALSE, "", "");
}

nsresult
nsPrefMigration::DoTheCopy(nsIFileSpec * oldPath, nsIFileSpec * newPath, const char *fileOrDirName, PRBool isDirectory)
{
  nsresult rv;

  if (isDirectory)
  {
    nsCOMPtr<nsIFileSpec> oldSubPath;

    NS_NewFileSpec(getter_AddRefs(oldSubPath));
    oldSubPath->FromFileSpec(oldPath);
    rv = oldSubPath->AppendRelativeUnixPath(fileOrDirName);
    if (NS_FAILED(rv)) return rv;
    PRBool exist;
    rv = oldSubPath->Exists(&exist);
    if (NS_FAILED(rv)) return rv;
    if (!exist)
    {
      rv = oldSubPath->CreateDir();
      if (NS_FAILED(rv)) return rv;
    }

    nsCOMPtr<nsIFileSpec> newSubPath;

    NS_NewFileSpec(getter_AddRefs(newSubPath));
    newSubPath->FromFileSpec(newPath);
    rv = newSubPath->AppendRelativeUnixPath(fileOrDirName);
    if (NS_FAILED(rv)) return rv;
    rv = newSubPath->Exists(&exist);
    if (NS_FAILED(rv)) return rv;
    if (!exist)
    {
      rv = newSubPath->CreateDir();
      if (NS_FAILED(rv)) return rv;
    }

    DoTheCopy(oldSubPath, newSubPath, PR_TRUE);
  }
  else
  {
    nsCOMPtr<nsIFileSpec> file;
    NS_NewFileSpec(getter_AddRefs(file));
    file->FromFileSpec(oldPath);
    rv = file->AppendRelativeUnixPath(fileOrDirName);
    if( NS_FAILED(rv) ) return rv;
    PRBool exist;
    rv = file->Exists(&exist);
    if( NS_FAILED(rv) ) return rv;
    if( exist) {
      file->CopyToDir(newPath);
    }
  }

  return rv;
}

#if defined(NEED_TO_FIX_4X_COOKIES)
/* this code only works on the mac.  in 4.x, the line endings where '\r' on the mac.
   this code will fix the expire times and the line endings, so the code is nsCookie.cpp
   can read the migrate cookies. */
static PRInt32
GetCookieLine(nsInputFileStream &strm, nsAutoString& aLine) 
{
  /* read the line */
  aLine.Truncate();
  char c;
  for (;;) {
    c = strm.get();
    
    /* note that eof is not set until we read past the end of the file */
    if (strm.eof()) {
      return -1;
    }

    /* stop at the '\r' */
    if (c != '\r') {
      aLine.Append(PRUnichar(c));
    }
    else {
      break;
    }
  }
  return 0;
}

static nsresult
PutCookieLine(nsOutputFileStream &strm, const nsString& aLine)
{
  /* allocate a buffer from the heap */
  char * cp = ToNewCString(aLine);
  if (! cp) {
    return NS_ERROR_FAILURE;
  }

  /* output each character */
  char* p = cp;
  while (*p) {
    strm.put(*(p++));
  }
  NS_Free(cp);
  // the lines in a 5.x cookie file call end with '\n', on all platforms
  strm.put('\n');
  return NS_OK;
}

static nsresult
Fix4xCookies(nsIFileSpec * profilePath) {
  nsAutoString inBuffer, outBuffer;
  nsFileSpec profileDirectory;
  nsresult rv = profilePath->GetFileSpec(&profileDirectory);
  if (NS_FAILED(rv)) {
    return rv;
  }

  /* open input file */
  nsFileSpec oldCookies(profileDirectory);
  oldCookies += COOKIES_FILE_NAME_IN_4x;
  
  /* it is possible that the 4.x cookies file does not exist.  just return normally.  see #55444 */
  if (!oldCookies.Exists()) return NS_OK;
  
  nsInputFileStream inStream(oldCookies);
  if (!inStream.is_open()) {
    return NS_ERROR_FAILURE;
  }

  /* open output file */
  nsFileSpec newCookies(profileDirectory);
  newCookies += COOKIES_FILE_NAME_IN_5x;
  
  nsOutputFileStream outStream(newCookies);
  if (!outStream.is_open()) {
    return NS_ERROR_FAILURE;
  }

  while (GetCookieLine(inStream,inBuffer) != -1){

    /* skip line if it is a comment or null line */
    if (inBuffer.IsEmpty() || inBuffer.CharAt(0) == '#' ||
        inBuffer.CharAt(0) == nsCRT::CR || inBuffer.CharAt(0) == nsCRT::LF) {
      PutCookieLine(outStream, inBuffer);
      continue;
    }

    /* locate expire field, skip line if it does not contain all its fields */
    int hostIndex, isDomainIndex, pathIndex, xxxIndex, expiresIndex, nameIndex, cookieIndex;
    hostIndex = 0;
    if ((isDomainIndex=inBuffer.FindChar('\t', hostIndex)+1) == 0 ||
        (pathIndex=inBuffer.FindChar('\t', isDomainIndex)+1) == 0 ||
        (xxxIndex=inBuffer.FindChar('\t', pathIndex)+1) == 0 ||
        (expiresIndex=inBuffer.FindChar('\t', xxxIndex)+1) == 0 ||
        (nameIndex=inBuffer.FindChar('\t', expiresIndex)+1) == 0 ||
        (cookieIndex=inBuffer.FindChar('\t', nameIndex)+1) == 0 ) {
      continue;
    }

    /* separate the expires field from the rest of the cookie line */
    nsAutoString prefix, expiresString, suffix;
    inBuffer.Mid(prefix, hostIndex, expiresIndex-hostIndex-1);
    inBuffer.Mid(expiresString, expiresIndex, nameIndex-expiresIndex-1);
    inBuffer.Mid(suffix, nameIndex, inBuffer.Length()-nameIndex);

    /* correct the expires field */
    char * expiresCString = ToNewCString(expiresString);
    unsigned long expires = strtoul(expiresCString, nsnull, 10);
    NS_Free(expiresCString);

    /* if the cookie is supposed to expire at the end of the session
     * expires == 0.  don't adjust those cookies.
     */
    if (expires) {
    	expires -= SECONDS_BETWEEN_1900_AND_1970;
    }
    char dateString[36];
    PR_snprintf(dateString, sizeof(dateString), "%lu", expires);

    /* generate the output buffer and write it to file */
    outBuffer = prefix;
    outBuffer.Append(PRUnichar('\t'));
    outBuffer.AppendWithConversion(dateString);
    outBuffer.Append(PRUnichar('\t'));
    outBuffer.Append(suffix);
    PutCookieLine(outStream, outBuffer);
  }

  inStream.close();
  outStream.close();
  return NS_OK;
}

#endif /* NEED_TO_FIX_4X_COOKIES */

/*----------------------------------------------------------------------------
 * DoSpecialUpdates updates is a routine that does some miscellaneous updates 
 * like renaming certain files, etc.
 *--------------------------------------------------------------------------*/
nsresult
nsPrefMigration::DoSpecialUpdates(nsIFileSpec  * profilePath)
{
  nsresult rv;
  PRInt32 serverType;
  nsFileSpec fs;

  rv = profilePath->GetFileSpec(&fs);
  if (NS_FAILED(rv)) return rv;
  
  fs += PREF_FILE_NAME_IN_5x;
  
  nsOutputFileStream fsStream(fs, (PR_WRONLY | PR_CREATE_FILE | PR_APPEND));
  
  if (!fsStream.is_open())
  {
    return NS_ERROR_FAILURE;
  }

  /* Need to add a string to the top of the prefs.js file to prevent it
   * from being loaded as a standard javascript file which would be a
   * security hole.
   */
  fsStream << PREF_FILE_HEADER_STRING << nsEndl ;
  fsStream.close();

  // rename the cookies file, but only if we need to.
#if defined(NEED_TO_FIX_4X_COOKIES)
  rv = Fix4xCookies(profilePath);  
  if (NS_FAILED(rv)) {
    return rv;
  }
#else
  rv = Rename4xFileAfterMigration(profilePath,COOKIES_FILE_NAME_IN_4x,COOKIES_FILE_NAME_IN_5x);
  if (NS_FAILED(rv)) return rv;
#endif /* NEED_TO_FIX_4X_COOKIES */

  // rename the bookmarks file, but only if we need to.
  rv = Rename4xFileAfterMigration(profilePath,BOOKMARKS_FILE_NAME_IN_4x,BOOKMARKS_FILE_NAME_IN_5x);
  if (NS_FAILED(rv)) return rv;
    
  /* Create the new mail directory from the setting in prefs.js or a default */
  rv = m_prefBranch->GetIntPref(PREF_MAIL_SERVER_TYPE, &serverType);
  if (NS_FAILED(rv)) return rv; 
  if (serverType == POP_4X_MAIL_TYPE) {
	rv = RenameAndMove4xPopFilterFile(profilePath);
  	if (NS_FAILED(rv)) return rv; 

	rv = RenameAndMove4xPopStateFile(profilePath);
  	if (NS_FAILED(rv)) return rv; 
  }
#ifdef IMAP_MAIL_FILTER_FILE_NAME_FORMAT_IN_4x 
  else if (serverType == IMAP_4X_MAIL_TYPE) {
  	rv = RenameAndMove4xImapFilterFiles(profilePath);
	if (NS_FAILED(rv)) return rv;
  }
#endif /* IMAP_MAIL_FILTER_FILE_NAME_FORMAT_IN_4x */

  return rv;
}

nsresult
nsPrefMigration::RenameAndMove4xPopFilterFile(nsIFileSpec * profilePath)
{
  return RenameAndMove4xPopFile(profilePath, POP_MAIL_FILTER_FILE_NAME_IN_4x, POP_MAIL_FILTER_FILE_NAME_IN_5x);
}

nsresult
nsPrefMigration::RenameAndMove4xPopStateFile(nsIFileSpec * profilePath)
{
#ifdef POPSTATE_FILE_IN_4x
  return RenameAndMove4xPopFile(profilePath, POPSTATE_FILE_IN_4x, POPSTATE_FILE_IN_5x);
#else 
  // on windows, popstate.dat was in Users\<profile>\MAIL\popstate.dat
  // which is the right place, unlike linux and mac.
  // so, when we migrate Users\<profile>\Mail to Users50\<profile>\Mail\<hostname>
  // it just works
  return NS_OK;
#endif /* POPSTATE_FILE_IN_4x */
}

nsresult
nsPrefMigration::RenameAndMove4xPopFile(nsIFileSpec * profilePath, const char *fileNameIn4x, const char *fileNameIn5x)
{
  nsFileSpec file;
  nsresult rv = profilePath->GetFileSpec(&file);
  if (NS_FAILED(rv)) return rv;
  
  // we assume the 4.x pop files live at <profile>/<fileNameIn4x>
  file += fileNameIn4x;

  // figure out where the 4.x pop mail directory got copied to
  char *popServerName = nsnull;
  nsFileSpec migratedPopDirectory;
  rv = profilePath->GetFileSpec(&migratedPopDirectory);
  migratedPopDirectory += NEW_MAIL_DIR_NAME;
  m_prefBranch->GetCharPref(PREF_NETWORK_HOSTS_POP_SERVER, &popServerName);
  migratedPopDirectory += popServerName;
  PR_FREEIF(popServerName);

  // copy the 4.x file from <profile>/<fileNameIn4x> to the <profile>/Mail/<hostname>/<fileNameIn4x>
  rv = file.CopyToDir(migratedPopDirectory);
  NS_ASSERTION(NS_SUCCEEDED(rv),"failed to copy pop file");
  
  // XXX todo, delete the old file
  // we are leaving it behind
  
  // make migratedPopDirectory point the the copied filter file,
  // <profile>/Mail/<hostname>/<fileNameIn4x>
  migratedPopDirectory += fileNameIn4x;

  // rename <profile>/Mail/<hostname>/<fileNameIn4x>to <profile>/Mail/<hostname>/<fileNameIn5x>, if necessary
  if (PL_strcmp(fileNameIn4x,fileNameIn5x)) {
	  migratedPopDirectory.Rename(fileNameIn5x);
  }

  return NS_OK;
}


#ifdef IMAP_MAIL_FILTER_FILE_NAME_FORMAT_IN_4x
#define BUFFER_LEN	128
nsresult
nsPrefMigration::RenameAndMove4xImapFilterFile(nsIFileSpec * profilePath, const char *hostname)
{
  nsresult rv = NS_OK;
  char imapFilterFileName[BUFFER_LEN];

  // the 4.x imap filter file lives in "<profile>/<hostname> Rules"
  nsFileSpec file;
  rv = profilePath->GetFileSpec(&file);
  if (NS_FAILED(rv)) return rv;
  
  PR_snprintf(imapFilterFileName, BUFFER_LEN, IMAP_MAIL_FILTER_FILE_NAME_FORMAT_IN_4x, hostname);
  file += imapFilterFileName;

  // if that file didn't exist, because they didn't use filters for that server, return now
  if (!file.Exists()) return NS_OK;

  // figure out where the 4.x pop mail directory got copied to
  nsFileSpec migratedImapDirectory;
  rv = profilePath->GetFileSpec(&migratedImapDirectory);
  migratedImapDirectory += NEW_IMAPMAIL_DIR_NAME;
  migratedImapDirectory += hostname;

  // copy the 4.x file from "<profile>/<hostname> Rules" to <profile>/ImapMail/<hostname>/
  rv = file.CopyToDir(migratedImapDirectory);
  NS_ASSERTION(NS_SUCCEEDED(rv),"failed to copy imap file");

  // make migratedPopDirectory point the the copied filter file,
  // "<profile>/ImapMail/<hostname>/<hostname> Rules"
  migratedImapDirectory += imapFilterFileName;

  // rename "<profile>/ImapMail/<hostname>/<hostname> Rules" to  "<profile>/ImapMail/<hostname>/rules.dat"
  migratedImapDirectory.Rename(IMAP_MAIL_FILTER_FILE_NAME_IN_5x);

  return NS_OK;         
}

nsresult
nsPrefMigration::RenameAndMove4xImapFilterFiles(nsIFileSpec * profilePath)
{
  nsresult rv;
  char *hostList=nsnull;

  rv = m_prefBranch->GetCharPref(PREF_4X_NETWORK_HOSTS_IMAP_SERVER, &hostList);
  if (NS_FAILED(rv)) return rv;

  if (!hostList || !*hostList) return NS_OK; 

  char *token = nsnull;
  char *rest = hostList;
  nsCAutoString str;

  token = nsCRT::strtok(rest, ",", &rest);
  while (token && *token) {
    str = token;
    str.StripWhitespace();

    if (!str.IsEmpty()) {
      // str is the hostname
      rv = RenameAndMove4xImapFilterFile(profilePath,str.get());
      if  (NS_FAILED(rv)) {
        // failed to migrate.  bail.
        return rv;
      }
      str = "";
    }
    token = nsCRT::strtok(rest, ",", &rest);
  }
  PR_FREEIF(hostList);
  return NS_OK;    
}
#endif /* IMAP_MAIL_FILTER_FILE_NAME_FORMAT_IN_4x */

nsresult
nsPrefMigration::Rename4xFileAfterMigration(nsIFileSpec * profilePath, const char *oldFileName, const char *newFileName)
{
  nsresult rv = NS_OK;
  // if they are the same, don't bother to rename the file.
  if (PL_strcmp(oldFileName, newFileName) == 0) {
    return rv;
  }
               
  nsFileSpec file;
  rv = profilePath->GetFileSpec(&file);
  if (NS_FAILED(rv)) return rv;
  
  file += oldFileName;
  
  // make sure it exists before you try to rename it
  if (file.Exists()) {
    file.Rename(newFileName);
  }
  return rv;
}

#ifdef NEED_TO_COPY_AND_RENAME_NEWSRC_FILES
nsresult
nsPrefMigration::GetPremigratedFilePref(const char *pref_name, nsIFileSpec **path)
{
        nsresult rv;

        if (!pref_name) return NS_ERROR_FAILURE;

        char premigration_pref[MAX_PREF_LEN];
        PR_snprintf(premigration_pref,MAX_PREF_LEN,"%s%s",PREMIGRATION_PREFIX,pref_name);
#ifdef DEBUG_seth
        printf("getting %s (into a nsFileSpec)\n", premigration_pref);
#endif
        rv = m_prefBranch->GetComplexValue((const char *)premigration_pref,
                                           NS_GET_IID(nsIFileSpec),
                                           (void **)path);
        return rv;
}

#endif /* NEED_TO_COPY_AND_RENAME_NEWSRC_FILES */

nsresult 
nsPrefMigration::SetPremigratedFilePref(const char *pref_name, nsIFileSpec *path)
{
	nsresult rv;

	if (!pref_name) return NS_ERROR_FAILURE;

	// save off the old pref, prefixed with "premigration"
	// for example, we need the old "mail.directory" pref when
	// migrating the copies and folder prefs in nsMsgAccountManager.cpp
	//
	// note we do this for all platforms.
	char premigration_pref[MAX_PREF_LEN];
	PR_snprintf(premigration_pref,MAX_PREF_LEN,"%s%s",PREMIGRATION_PREFIX,pref_name);
#ifdef DEBUG_seth
	printf("setting %s (from a nsFileSpec) for later...\n", premigration_pref);
#endif

  // need to convert nsIFileSpec->nsILocalFile
  nsFileSpec pathSpec;
  path->GetFileSpec(&pathSpec);
  
  nsCOMPtr<nsILocalFile> pathFile;
  rv = NS_FileSpecToIFile(&pathSpec, getter_AddRefs(pathFile));
  if (NS_FAILED(rv)) return rv;
  
  PRBool exists = PR_FALSE;
  pathFile->Exists(&exists);
  
  NS_ASSERTION(exists, "the path does not exist.  see bug #55444");
  if (!exists) return NS_OK;
  
  rv = m_prefBranch->SetComplexValue((const char *)premigration_pref,
                                     NS_GET_IID(nsILocalFile), pathFile);
  return rv;
}

nsresult 
nsPrefMigration::DetermineOldPath(nsIFileSpec *profilePath, const char *oldPathName, const char *oldPathEntityName, nsIFileSpec *oldPath)
{
	nsresult rv;

  	/* set oldLocalFile to profilePath.  need to convert nsIFileSpec->nsILocalFile */
	nsCOMPtr<nsILocalFile> oldLocalFile;
	nsFileSpec pathSpec;
	profilePath->GetFileSpec(&pathSpec);
	rv = NS_FileSpecToIFile(&pathSpec, getter_AddRefs(oldLocalFile));
	if (NS_FAILED(rv)) return rv;
	
	/* get the string bundle, and get the appropriate localized string out of it */
	nsCOMPtr<nsIStringBundleService> bundleService = do_GetService(NS_STRINGBUNDLE_CONTRACTID, &rv);
	if (NS_FAILED(rv)) return rv;

    nsCOMPtr<nsIStringBundle> bundle;
    rv = bundleService->CreateBundle(MIGRATION_PROPERTIES_URL, getter_AddRefs(bundle));
	if (NS_FAILED(rv)) return rv;

	nsXPIDLString localizedDirName;
	nsAutoString entityName;
	entityName.AssignWithConversion(oldPathEntityName);
	rv = bundle->GetStringFromName(entityName.get(), getter_Copies(localizedDirName));
	if (NS_FAILED(rv)) return rv;

	rv = oldLocalFile->AppendRelativePath(localizedDirName);
	if (NS_FAILED(rv)) return rv;

	PRBool exists = PR_FALSE;
	rv = oldLocalFile->Exists(&exists);
	if (!exists) {
		/* if the localized name doesn't exist, use the english name */
		rv = oldPath->FromFileSpec(profilePath);
		if (NS_FAILED(rv)) return rv;
		
		rv = oldPath->AppendRelativeUnixPath(oldPathName);
		if (NS_FAILED(rv)) return rv;
		
		return NS_OK;
	}

	/* at this point, the folder with the localized name exists, so use it */
	nsCAutoString persistentDescriptor;
	rv = oldLocalFile->GetPersistentDescriptor(persistentDescriptor);
	if (NS_FAILED(rv)) return rv;
	rv = oldPath->SetPersistentDescriptorString(persistentDescriptor.get());
	if (NS_FAILED(rv)) return rv;

	return NS_OK;
}

////////////////////////////////////////////////////////////////////////
// nsPrefConverter
////////////////////////////////////////////////////////////////////////

/* 
  these are the prefs we know we need to convert to utf8.
  we'll also be converting:

  Please make sure that any pref that contains native characters
  in it's value is not included in this list as we do not want to 
  convert them into UTF-8 format. Prefs are being get and set in a 
  unicode format (FileXPref) now and there is no need for 
  conversion of those prefs. 

 "ldap_2.server.*.description"
 "intl.font*.fixed_font"
 "intl.font*.prop_font"
 "mail.identity.vcard.*"
 */

static const char *prefsToConvert[] = {
      "custtoolbar.personal_toolbar_folder",
      "editor.author",
      "li.server.ldap.userbase",
      "mail.identity.organization",
      "mail.identity.username",
      nsnull
};

nsPrefConverter::~nsPrefConverter()
{
}


nsPrefConverter::nsPrefConverter()
{
}

NS_IMPL_ISUPPORTS1(nsPrefConverter, nsIPrefConverter)

// Apply a charset conversion from the given charset to UTF-8 for the input C string.
static
nsresult 
ConvertStringToUTF8(const char* aCharset, const char* inString, char** outString)
{
  if (nsnull == outString)
    return NS_ERROR_NULL_POINTER;

  nsresult rv;
  // convert result to unicode
  nsCOMPtr<nsICharsetConverterManager> ccm = 
           do_GetService(NS_CHARSETCONVERTERMANAGER_CONTRACTID, &rv);

  if(NS_SUCCEEDED(rv)) {
    nsCOMPtr <nsIUnicodeDecoder> decoder; // this may be cached

    rv = ccm->GetUnicodeDecoderRaw(aCharset, getter_AddRefs(decoder));
    if(NS_SUCCEEDED(rv) && decoder) {
      PRInt32 uniLength = 0;
      PRInt32 srcLength = strlen(inString);
      rv = decoder->GetMaxLength(inString, srcLength, &uniLength);
      if (NS_SUCCEEDED(rv)) {
        PRUnichar *unichars = new PRUnichar [uniLength];

        if (nsnull != unichars) {
          // convert to unicode
          rv = decoder->Convert(inString, &srcLength, unichars, &uniLength);
          if (NS_SUCCEEDED(rv)) {
            nsAutoString aString;
            aString.Assign(unichars, uniLength);
            // convert to UTF-8
            *outString = ToNewUTF8String(aString);
          }
          delete [] unichars;
        }
        else {
          rv = NS_ERROR_OUT_OF_MEMORY;
        }
      }
    }    
  }

  return rv;
}

nsresult
ConvertPrefToUTF8(const char *prefname, nsIPrefBranch *prefs, const char* charSet)
{
    nsresult rv;

    if (!prefname || !prefs) return NS_ERROR_FAILURE;
#ifdef DEBUG_UTF8_CONVERSION 
    printf("converting %s to UTF8\n", prefname);
#endif /* DEBUG_UTF8_CONVERSION */
    
    nsXPIDLCString prefval;

    rv = prefs->GetCharPref(prefname, getter_Copies(prefval));
    if (NS_FAILED(rv)) return rv;

    if (prefval.IsEmpty()) {
        // no need to convert ""
        return NS_OK;
    }

    nsXPIDLCString outval;
    rv = ConvertStringToUTF8(charSet, (const char *)prefval, getter_Copies(outval));
    // only set the pref if the conversion worked, and it convert to something non null
    if (NS_SUCCEEDED(rv) && (const char *)outval && PL_strlen((const char *)outval)) {
#ifdef DEBUG_UTF8_CONVERSION
        printf("converting %s to %s\n",(const char *)prefval, (const char *)outval);
#endif /* DEBUG_UTF8_CONVERSION */
        rv = prefs->SetCharPref(prefname, (const char *)outval);
    }

    return NS_OK;
}


static PRBool charEndsWith(const char *str, const char *endStr)
{
    PRUint32 endStrLen = PL_strlen(endStr);
    PRUint32 strLen = PL_strlen(str);
    
    if (strLen < endStrLen) return PR_FALSE;

    PRUint32 pos = strLen - endStrLen;
    if (PL_strncmp(str + pos, endStr, endStrLen) == 0) {
        return PR_TRUE;
    }
    else {
        return PR_FALSE;
    }
}

static
void fontPrefEnumerationFunction(const char *name, nsCStringArray *arr)
{
#ifdef DEBUG_UTF8_CONVERSION
  printf("fontPrefEnumerationFunction: %s\n", name);
#endif 

  if (charEndsWith(name,".fixed_font") || charEndsWith(name,".prop_font")) {
    nsCString str(name);
    arr->AppendCString(str);
  }
}

static
void ldapPrefEnumerationFunction(const char *name, nsCStringArray *arr)
{
#ifdef DEBUG_UTF8_CONVERSION
  printf("ldapPrefEnumerationFunction: %s\n", name);
#endif 

  // we only want to convert "ldap_2.servers.*.description"
  if (charEndsWith(name,".description")) {
    nsCString str(name);
    arr->AppendCString(str);
  }
}

static
void vCardPrefEnumerationFunction(const char *name, nsCStringArray *arr)
{
#ifdef DEBUG_UTF8_CONVERSION
  printf("vCardPrefEnumerationFunction: %s\n", name);
#endif 

  // the 4.x vCard prefs might need converting
  nsCString str(name);
  arr->AppendCString(str);
}


typedef struct {
    nsIPrefBranch *prefs;
    const char* charSet;
} PrefEnumerationClosure;

PRBool
convertPref(nsCString &aElement, void *aData)
{
  PrefEnumerationClosure *closure;
  closure = (PrefEnumerationClosure *)aData;

  ConvertPrefToUTF8(aElement.get(), closure->prefs, closure->charSet);
  return PR_TRUE;
}

NS_IMETHODIMP
nsPrefConverter::ConvertPrefsToUTF8()
{
  nsresult rv;

  nsCStringArray prefsToMigrate;

  nsCOMPtr<nsIPrefBranch> prefs(do_GetService(NS_PREFSERVICE_CONTRACTID, &rv));
  if(NS_FAILED(rv)) return rv;
  if (!prefs) return NS_ERROR_FAILURE;

  nsCAutoString charSet;
  rv = GetPlatformCharset(charSet);
  
  if (NS_FAILED(rv)) return rv;

  PRUint32 i;

  for (i = 0; prefsToConvert[i]; i++) {
    nsCString prefnameStr( prefsToConvert[i] );
    prefsToMigrate.AppendCString(prefnameStr);
  }

  PRUint32 count;
  char **childArray;

  if (NS_SUCCEEDED(prefs->GetChildList("intl.font.", &count, &childArray))) {
    for (i = 0; i < count; i++)
      fontPrefEnumerationFunction(childArray[i], &prefsToMigrate);

    NS_FREE_XPCOM_ALLOCATED_POINTER_ARRAY(count, childArray);
  }

  if (NS_SUCCEEDED(prefs->GetChildList("ldap_2.servers.", &count, &childArray))) {
    for (i = 0; i < count; i++)
      ldapPrefEnumerationFunction(childArray[i], &prefsToMigrate);

    NS_FREE_XPCOM_ALLOCATED_POINTER_ARRAY(count, childArray);
  }

  if (NS_SUCCEEDED(prefs->GetChildList("mail.identity.vcard.", &count, &childArray))) {
    for (i = 0; i < count; i++)
      vCardPrefEnumerationFunction(childArray[i], &prefsToMigrate);

    NS_FREE_XPCOM_ALLOCATED_POINTER_ARRAY(count, childArray);
  }

  PrefEnumerationClosure closure;

  closure.prefs = prefs;
  closure.charSet = charSet.get();

  prefsToMigrate.EnumerateForwards((nsCStringArrayEnumFunc)convertPref, (void *)(&closure));

  rv = prefs->SetBoolPref("prefs.converted-to-utf8", PR_TRUE);
  return NS_OK;
}

// A wrapper function to call the interface to get a platform file charset.
nsresult 
nsPrefConverter::GetPlatformCharset(nsCString& aCharset)
{
  nsresult rv;

  // we may cache it since the platform charset will not change through application life
  nsCOMPtr <nsIPlatformCharset> platformCharset = do_GetService(NS_PLATFORMCHARSET_CONTRACTID, &rv);
  if (NS_SUCCEEDED(rv) && platformCharset) {
   rv = platformCharset->GetCharset(kPlatformCharsetSel_4xPrefsJS, aCharset);
  }
  if (NS_FAILED(rv)) {
   aCharset.AssignLiteral("ISO-8859-1");  // use ISO-8859-1 in case of any error
  }
 
  return rv;
}


