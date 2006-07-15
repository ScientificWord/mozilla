/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
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
 *   Seth Spitzer <sspitzer@netscape.com>
 *   Alec Flett <alecf@netscape.com>
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

#include "nsIComponentManager.h"
#include "nsIServiceManager.h"
#include "nsISupportsArray.h"
#include "nsSupportsPrimitives.h"
#include "nsNetUtil.h"

#include "prmem.h"
#include "plstr.h"
#include "prprf.h"
#include "nsString.h"
#include "nsXPIDLString.h"
#include "nscore.h"

#include "nsCRT.h"  // for nsCRT::strtok
#include "nsFileStream.h"
#include "nsIDirectoryService.h"
#include "nsDirectoryServiceDefs.h"
#include "nsAppDirectoryServiceDefs.h"
#include "nsIFileSpec.h" 
#include "nsILocalFile.h"
#include "nsIURL.h"
#include "nsNetCID.h"
#include "nsIStringBundle.h"

#include "nsMessengerMigrator.h"
#include "nsMsgBaseCID.h"
#include "nsMsgCompCID.h"

#include "nsIMsgFolderCache.h"
#include "nsMsgUtils.h"
#include "nsISmtpService.h"
#include "nsIObserverService.h"

#include "nsIMsgAccount.h"
#include "nsIMsgAccountManager.h"

#include "nsIPop3IncomingServer.h"
#include "nsIImapIncomingServer.h"
#include "nsINntpIncomingServer.h"
#include "nsINoIncomingServer.h"
#include "nsIMsgProtocolInfo.h"
#include "nsEscape.h"

#include "nsIUserInfo.h"
#include "nsIAddressBook.h"
#include "nsAbBaseCID.h"

#if defined(MOZ_LDAP_XPCOM)
#include "nsILDAPPrefsService.h"
#endif
#include "nsIMsgFilterService.h"
#include "nsIMsgFilterList.h"

#define BUF_STR_LEN 1024

#if defined(DEBUG_sspitzer) || defined(DEBUG_seth)
#define DEBUG_MIGRATOR 1
#endif

#define IMAP_SCHEMA "imap:/"
#define IMAP_SCHEMA_LENGTH 6
#define MAILBOX_SCHEMA "mailbox:/"
#define MAILBOX_SCHEMA_LENGTH 9

#define POP_4X_MAIL_TYPE 0
#define IMAP_4X_MAIL_TYPE 1
#ifdef HAVE_MOVEMAIL
#define MOVEMAIL_4X_MAIL_TYPE 2
#endif /* HAVE_MOVEMAIL */

/* TODO:  do we want to clear these after migration? */
#define PREF_NEWS_DIRECTORY "news.directory"
#define PREF_MAIL_DIRECTORY "mail.directory"
#define PREF_PREMIGRATION_MAIL_DIRECTORY "premigration.mail.directory"
#define PREF_PREMIGRATION_NEWS_DIRECTORY "premigration.news.directory"
#define PREF_IMAP_DIRECTORY "mail.imap.root_dir"
#define PREF_MAIL_DEFAULT_SENDLATER_URI "mail.default_sendlater_uri"

#define LOCAL_MAIL_FAKE_USER_NAME "nobody"


#ifdef HAVE_MOVEMAIL
#define MOVEMAIL_FAKE_HOST_NAME "movemail"
#endif /* HAVE_MOVEMAIL */
#define DEFAULT_4X_DRAFTS_FOLDER_NAME "Drafts"
#define DEFAULT_4X_SENT_FOLDER_NAME "Sent"
#define DEFAULT_4X_TEMPLATES_FOLDER_NAME "Templates"
#define UNSENT_MESSAGES_FOLDER_NAME "Unsent%20Messages"

#define FILTER_FILE_NAME	"msgFilterRules.dat"		/* this is XP in 5.x */

/* we are going to clear these after migration */
#define PREF_4X_MAIL_IDENTITY_USEREMAIL "mail.identity.useremail"
#define PREF_4X_MAIL_IDENTITY_USERNAME "mail.identity.username"
#define PREF_4X_MAIL_IDENTITY_REPLY_TO "mail.identity.reply_to"    
#define PREF_4X_MAIL_IDENTITY_ORGANIZATION "mail.identity.organization"
#define PREF_4X_MAIL_SIGNATURE_FILE "mail.signature_file"
#define PREF_4X_MAIL_SIGNATURE_DATE "mail.signature_date"
#define PREF_4X_MAIL_COMPOSE_HTML "mail.html_compose"
#define PREF_4X_MAIL_POP_NAME "mail.pop_name"
#define PREF_4X_MAIL_REMEMBER_PASSWORD "mail.remember_password"
#define PREF_4X_MAIL_POP_PASSWORD "mail.pop_password"
#define PREF_4X_NETWORK_HOSTS_POP_SERVER "network.hosts.pop_server"
#define PREF_4X_MAIL_CHECK_NEW_MAIL "mail.check_new_mail"
#define PREF_4X_MAIL_POP3_GETS_NEW_MAIL	"mail.pop3_gets_new_mail"
#define PREF_4X_MAIL_CHECK_TIME "mail.check_time"
#define PREF_4X_MAIL_LEAVE_ON_SERVER "mail.leave_on_server"
#define PREF_4X_MAIL_DELETE_MAIL_LEFT_ON_SERVER "mail.delete_mail_left_on_server"
#define PREF_4X_NETWORK_HOSTS_SMTP_SERVER "network.hosts.smtp_server"
#define PREF_4X_MAIL_SMTP_NAME "mail.smtp_name"
#define PREF_4X_MAIL_SMTP_SSL "mail.smtp.ssl"
#define PREF_4X_MAIL_SERVER_TYPE "mail.server_type"
#define PREF_4X_NETWORK_HOSTS_IMAP_SERVER "network.hosts.imap_servers"
#define PREF_4X_MAIL_USE_IMAP_SENTMAIL "mail.use_imap_sentmail"
#define PREF_4X_NEWS_USE_IMAP_SENTMAIL "news.use_imap_sentmail"
#define PREF_4X_MAIL_IMAP_SENTMAIL_PATH "mail.imap_sentmail_path"
#define PREF_4X_NEWS_IMAP_SENTMAIL_PATH "news.imap_sentmail_path"
#define PREF_4X_MAIL_DEFAULT_CC "mail.default_cc"
#define PREF_4X_NEWS_DEFAULT_CC "news.default_cc"
#define PREF_4X_MAIL_DEFAULT_FCC "mail.default_fcc"
#define PREF_4X_NEWS_DEFAULT_FCC "news.default_fcc"
#define PREF_4X_MAIL_USE_DEFAULT_CC "mail.use_default_cc"
#define PREF_4X_NEWS_USE_DEFAULT_CC "news.use_default_cc"
#define PREF_4X_MAIL_DEFAULT_DRAFTS "mail.default_drafts"
#define PREF_4X_MAIL_DEFAULT_TEMPLATES "mail.default_templates"
#define PREF_4X_MAIL_CC_SELF "mail.cc_self"
#define PREF_4X_NEWS_CC_SELF "news.cc_self"
#define PREF_4X_MAIL_USE_FCC "mail.use_fcc"
#define PREF_4X_NEWS_USE_FCC "news.use_fcc"
#define PREF_4X_NEWS_MAX_ARTICLES "news.max_articles"
#define PREF_4X_NEWS_NOTIFY_ON "news.notify.on"
#define PREF_4X_NEWS_MARK_OLD_READ "news.mark_old_read"
#define PREF_4X_MAIL_ATTACH_VCARD "mail.attach_vcard"
#define PREF_4X_MAIL_IDENTITY_VCARD_ROOT "mail.identity.vcard"

#define PREF_4X_AUTOCOMPLETE_ON_LOCAL_AB "ldap_2.autoComplete.useAddressBooks"
#define PREF_MOZILLA_AUTOCOMPLETE_ON_LOCAL_AB "mail.enable_autocomplete"

#define DEFAULT_FCC_FOLDER_PREF_NAME "mail.identity.default.fcc_folder"
#define DEFAULT_DRAFT_FOLDER_PREF_NAME  "mail.identity.default.draft_folder"
#define DEFAULT_STATIONERY_FOLDER_PREF_NAME "mail.identity.default.stationery_folder"

#define DEFAULT_PAB_FILENAME_PREF_NAME "ldap_2.servers.pab.filename"

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

#define ESCAPE_USER_NAME(outName,inName) \
    *((char **)getter_Copies(outName)) = nsEscape((const char *)inName, url_XAlphas);

#define ESCAPE_FOLDER_NAME(outName,inName) \
    *((char **)getter_Copies(outName)) = nsEscape((const char *)inName, url_Path);


#define CONVERT_4X_URI(IDENTITY,FOR_NEWS,USERNAME,HOSTNAME,DEFAULT_FOLDER_NAME,MACRO_GETTER,MACRO_SETTER,DEFAULT_PREF) \
{ \
  nsXPIDLCString macro_oldStr; \
  nsresult macro_rv; \
  macro_rv = IDENTITY->MACRO_GETTER(getter_Copies(macro_oldStr));	\
  if (NS_FAILED(macro_rv) || !macro_oldStr) { \
    IDENTITY->MACRO_SETTER("");	\
  }\
  else {	\
    char *converted_uri = nsnull; \
    macro_rv = Convert4XUri((const char *)macro_oldStr, FOR_NEWS, USERNAME, HOSTNAME, DEFAULT_FOLDER_NAME, DEFAULT_PREF, &converted_uri); \
    if (NS_FAILED(macro_rv)) { \
      IDENTITY->MACRO_SETTER("");	\
    } \
    else { \
      IDENTITY->MACRO_SETTER(converted_uri); \
    } \
    PR_FREEIF(converted_uri); \
  }	\
}


#define MIGRATE_SIMPLE_FILE_PREF_TO_BOOL_PREF(PREFNAME,MACRO_OBJECT,MACRO_METHOD) \
  { \
    nsresult macro_rv; \
    nsCOMPtr <nsIFileSpec>macro_spec;	\
    macro_rv = m_prefs->GetComplexValue(PREFNAME, NS_GET_IID(nsIFileSpec), getter_AddRefs(macro_spec)); \
    if (NS_SUCCEEDED(macro_rv)) { \
	char *macro_oldStr = nsnull; \
	macro_rv = macro_spec->GetUnixStyleFilePath(&macro_oldStr);	\
    if (NS_SUCCEEDED(macro_rv) && macro_oldStr && (PL_strlen(macro_oldStr))) { \
		MACRO_OBJECT->MACRO_METHOD(PR_TRUE); \
	}	\
	else {	\
		MACRO_OBJECT->MACRO_METHOD(PR_FALSE); \
	}	\
	PR_FREEIF(macro_oldStr); \
    } \
  }


#define MIGRATE_SIMPLE_FILE_PREF_TO_CHAR_PREF(PREFNAME,MACRO_OBJECT,MACRO_METHOD) \
  { \
    nsresult macro_rv; \
    nsCOMPtr <nsIFileSpec>macro_spec;	\
	char *macro_val = nsnull; \
	macro_rv = m_prefs->GetCharPref(PREFNAME, &macro_val); \
	if (NS_SUCCEEDED(macro_rv) && macro_val && PL_strlen(macro_val)) { \
		macro_rv = m_prefs->GetComplexValue(PREFNAME, NS_GET_IID(nsIFileSpec), getter_AddRefs(macro_spec)); \
		if (NS_SUCCEEDED(macro_rv)) { \
			char *macro_oldStr = nsnull; \
			macro_rv = macro_spec->GetUnixStyleFilePath(&macro_oldStr);	\
    		if (NS_SUCCEEDED(macro_rv)) { \
				MACRO_OBJECT->MACRO_METHOD(macro_oldStr); \
            } \
            PR_FREEIF(macro_oldStr); \
    	} \
	} \
	else { \
		MACRO_OBJECT->MACRO_METHOD(""); \
	}	\
    PR_FREEIF(macro_val); \
  }

#define MIGRATE_SIMPLE_FILE_PREF_TO_FILE_PREF(PREFNAME,MACRO_OBJECT,MACRO_METHOD) \
  { \
    nsresult macro_rv; \
    nsCOMPtr <nsILocalFile> macro_file; \
    char *macro_oldStr = nsnull; \
    macro_rv = m_prefs->GetCharPref(PREFNAME, &macro_oldStr); \
    if (NS_SUCCEEDED(macro_rv) && macro_oldStr && PL_strlen(macro_oldStr)) { \
      macro_rv = m_prefs->GetComplexValue(PREFNAME, NS_GET_IID(nsILocalFile), getter_AddRefs(macro_file)); \
      if (NS_SUCCEEDED(macro_rv)) { \
        MACRO_OBJECT->MACRO_METHOD(macro_file); \
      } \
    } \
    PR_FREEIF(macro_oldStr); \
  }

#define MIGRATE_SIMPLE_STR_PREF(PREFNAME,MACRO_OBJECT,MACRO_METHOD) \
  { \
    nsresult macro_rv; \
    char *macro_oldStr = nsnull; \
    macro_rv = m_prefs->GetCharPref(PREFNAME, &macro_oldStr); \
    if (NS_SUCCEEDED(macro_rv)) { \
      MACRO_OBJECT->MACRO_METHOD(macro_oldStr); \
    } \
    PR_FREEIF(macro_oldStr); \
  }

#define MIGRATE_SIMPLE_WSTR_PREF(PREFNAME,MACRO_OBJECT,MACRO_METHOD) \
  { \
    nsresult macro_rv; \
    PRUnichar *macro_oldStr = nsnull; \
    nsCOMPtr<nsISupportsString> macro_tmpstr; \
    macro_rv = m_prefs->GetComplexValue(PREFNAME, NS_GET_IID(nsISupportsString), getter_AddRefs(macro_tmpstr)); \
    if (NS_SUCCEEDED(macro_rv)) { \
      macro_tmpstr->ToString(&macro_oldStr); \
      MACRO_OBJECT->MACRO_METHOD(macro_oldStr); \
    } \
    PR_FREEIF(macro_oldStr); \
  }

#define MIGRATE_SIMPLE_INT_PREF(PREFNAME,MACRO_OBJECT,MACRO_METHOD) \
  { \
    nsresult macro_rv; \
    PRInt32 oldInt; \
    macro_rv = m_prefs->GetIntPref(PREFNAME, &oldInt); \
    if (NS_SUCCEEDED(macro_rv)) { \
      MACRO_OBJECT->MACRO_METHOD(oldInt); \
    } \
  }

#define MIGRATE_SIMPLE_BOOL_PREF(PREFNAME,MACRO_OBJECT,MACRO_METHOD) \
  { \
    nsresult macro_rv; \
    PRBool macro_oldBool; \
    macro_rv = m_prefs->GetBoolPref(PREFNAME, &macro_oldBool); \
    if (NS_SUCCEEDED(macro_rv)) { \
      MACRO_OBJECT->MACRO_METHOD(macro_oldBool); \
    } \
  }

#define MIGRATE_STR_PREF(PREFFORMATSTR,PREFFORMATVALUE,INCOMINGSERVERPTR,INCOMINGSERVERMETHOD) \
  { \
    nsresult macro_rv; \
    char prefName[BUF_STR_LEN]; \
    char *macro_oldStr = nsnull; \
    PR_snprintf(prefName, BUF_STR_LEN, PREFFORMATSTR, PREFFORMATVALUE); \
    macro_rv = m_prefs->GetCharPref(prefName, &macro_oldStr); \
    if (NS_SUCCEEDED(macro_rv)) { \
      INCOMINGSERVERPTR->INCOMINGSERVERMETHOD(macro_oldStr); \
    } \
    PR_FREEIF(macro_oldStr); \
  }

#define MIGRATE_INT_PREF(PREFFORMATSTR,PREFFORMATVALUE,INCOMINGSERVERPTR,INCOMINGSERVERMETHOD) \
  { \
    nsresult macro_rv; \
    PRInt32 oldInt; \
    char prefName[BUF_STR_LEN]; \
    PR_snprintf(prefName, BUF_STR_LEN, PREFFORMATSTR, PREFFORMATVALUE); \
    macro_rv = m_prefs->GetIntPref(prefName, &oldInt); \
    if (NS_SUCCEEDED(macro_rv)) { \
      INCOMINGSERVERPTR->INCOMINGSERVERMETHOD(oldInt); \
    } \
  }

#define MIGRATE_BOOL_PREF(PREFFORMATSTR,PREFFORMATVALUE,INCOMINGSERVERPTR,INCOMINGSERVERMETHOD) \
  { \
    nsresult macro_rv; \
    PRBool macro_oldBool; \
    char prefName[BUF_STR_LEN]; \
    PR_snprintf(prefName, BUF_STR_LEN, PREFFORMATSTR, PREFFORMATVALUE); \
    macro_rv = m_prefs->GetBoolPref(prefName, &macro_oldBool); \
    if (NS_SUCCEEDED(macro_rv)) { \
      INCOMINGSERVERPTR->INCOMINGSERVERMETHOD(macro_oldBool); \
    } \
  }

NS_IMPL_ISUPPORTS2(nsMessengerMigrator, nsIMessengerMigrator, nsIObserver)

nsMessengerMigrator::nsMessengerMigrator() :
  m_haveShutdown(PR_FALSE)
{
}

nsMessengerMigrator::~nsMessengerMigrator()
{
  nsresult rv;

  if(!m_haveShutdown)
  {
    Shutdown();
    //Don't remove from Observer service in Shutdown because Shutdown also gets called
    //from xpcom shutdown observer.  And we don't want to remove from the service in that case.
    nsCOMPtr<nsIObserverService> observerService = 
             do_GetService("@mozilla.org/observer-service;1", &rv);
    if (NS_SUCCEEDED(rv))
    {
      observerService->RemoveObserver(this, NS_XPCOM_SHUTDOWN_OBSERVER_ID);
    }
  }     
}

nsresult nsMessengerMigrator::Init()
{
  nsresult rv;

  nsCOMPtr<nsIObserverService> observerService = 
           do_GetService("@mozilla.org/observer-service;1", &rv);
  if (NS_SUCCEEDED(rv))
  {
    observerService->AddObserver(this, NS_XPCOM_SHUTDOWN_OBSERVER_ID, PR_FALSE);
  }    

  initializeStrings();
  
  rv = getPrefService();
  if (NS_FAILED(rv)) return rv;   

  rv = ResetState();
  return rv;
}

nsresult 
nsMessengerMigrator::Shutdown()
{
  m_prefs = nsnull;

  m_haveShutdown = PR_TRUE;
  return NS_OK;
}       

nsresult
nsMessengerMigrator::getPrefService()
{
  nsresult rv = NS_OK;

  if (!m_prefs) {
    m_prefs = do_GetService(NS_PREFSERVICE_CONTRACTID, &rv);
  }

  if (NS_FAILED(rv)) return rv;

  if (!m_prefs) return NS_ERROR_FAILURE;

  return NS_OK;
} 

nsresult
nsMessengerMigrator::initializeStrings()
{
  nsresult rv;
  nsCOMPtr<nsIStringBundleService> bundleService =
    do_GetService(NS_STRINGBUNDLE_CONTRACTID, &rv);
  NS_ENSURE_SUCCESS(rv, rv);
  
  nsCOMPtr<nsIStringBundle> bundle;
  rv = bundleService->CreateBundle("chrome://messenger/locale/messenger.properties",
                                   getter_AddRefs(bundle));
  NS_ENSURE_SUCCESS(rv, rv);
  
  // now retrieve strings
  nsXPIDLString localFolders;
  rv = bundle->GetStringFromName(NS_LITERAL_STRING("localFolders").get(),
                                 getter_Copies(localFolders));
  NS_ENSURE_SUCCESS(rv, rv);
  // convert to unicode and ASCII

  mLocalFoldersName.Assign(localFolders);
  mLocalFoldersHostname = "Local Folders";

  return NS_OK;
}



NS_IMETHODIMP nsMessengerMigrator::Observe(nsISupports *aSubject, const char *aTopic, const PRUnichar *someData)
{
  if(!nsCRT::strcmp(aTopic, NS_XPCOM_SHUTDOWN_OBSERVER_ID))
  {
    Shutdown();
  }

  return NS_OK;
}           

nsresult
nsMessengerMigrator::ProceedWithMigration()
{
  char *prefvalue = nsnull;
  nsresult rv = NS_OK;
  
  if ((m_oldMailType == POP_4X_MAIL_TYPE) 
#ifdef HAVE_MOVEMAIL
	|| (m_oldMailType == MOVEMAIL_4X_MAIL_TYPE) 
#endif /* HAVE_MOVEMAIL */
	) {
    // if they were using pop or movemail, "mail.pop_name" must have been set
    // otherwise, they don't really have anything to migrate
    rv = m_prefs->GetCharPref(PREF_4X_MAIL_POP_NAME, &prefvalue);
    if (NS_SUCCEEDED(rv)) {
	    if (!prefvalue || !*prefvalue) {
	      rv = NS_ERROR_FAILURE;
	    }
    }
  }
  else if (m_oldMailType == IMAP_4X_MAIL_TYPE) {
    // if they were using imap, "network.hosts.imap_servers" must have been set
    // otherwise, they don't really have anything to migrate
    rv = m_prefs->GetCharPref(PREF_4X_NETWORK_HOSTS_IMAP_SERVER, &prefvalue);
    if (NS_SUCCEEDED(rv)) {
	    if (!prefvalue || !*prefvalue) {
	      rv = NS_ERROR_FAILURE;
	    }
    }
  }
  else {
#ifdef DEBUG_MIGRATOR
    printf("Unrecognized server type %d\n", m_oldMailType);
#endif
    rv = NS_ERROR_UNEXPECTED;
  }

  PR_FREEIF(prefvalue);
  return rv;
}

NS_IMETHODIMP
nsMessengerMigrator::CreateLocalMailAccount(PRBool migrating)
{
  nsresult rv;
  nsCOMPtr<nsIMsgAccountManager> accountManager = 
           do_GetService(NS_MSGACCOUNTMANAGER_CONTRACTID, &rv);
  if (NS_FAILED(rv)) return rv;

  // create the server
  nsCOMPtr<nsIMsgIncomingServer> server;
  rv = accountManager->CreateIncomingServer(LOCAL_MAIL_FAKE_USER_NAME,
                                            mLocalFoldersHostname.get(),
                            "none", getter_AddRefs(server));
  NS_ENSURE_SUCCESS(rv,rv);

  // we don't want "nobody at Local Folders" to show up in the
  // folder pane, so we set the pretty name to "Local Folders"
  server->SetPrettyName(mLocalFoldersName.get());

  nsCOMPtr<nsINoIncomingServer> noServer;
  noServer = do_QueryInterface(server, &rv);
  if (NS_FAILED(rv)) return rv;
   
  // create the directory structure for old 4.x "Local Mail"
  // under <profile dir>/Mail/Local Folders or
  // <"mail.directory" pref>/Local Folders 
  nsCOMPtr <nsIFile> mailDir;
  nsFileSpec dir;
  PRBool dirExists;

  // if the "mail.directory" pref is set, use that.
  // if they used -installer, this pref will point to where their files got copied
  // this only makes sense when we are migrating
  // for a new profile, that pref won't be set.
  if (migrating) {
    nsCOMPtr<nsILocalFile> localFile;
    rv = m_prefs->GetComplexValue(PREF_MAIL_DIRECTORY, NS_GET_IID(nsILocalFile), getter_AddRefs(localFile));
    if (NS_SUCCEEDED(rv))
        mailDir = localFile;
  }

  if (!mailDir) {
    // we want <profile>/Mail
    rv = NS_GetSpecialDirectory(NS_APP_MAIL_50_DIR, getter_AddRefs(mailDir));
    if (NS_FAILED(rv)) return rv;
  }
  
  rv = mailDir->Exists(&dirExists);
  if (NS_SUCCEEDED(rv) && !dirExists)
    rv = mailDir->Create(nsIFile::DIRECTORY_TYPE, 0775);
  if (NS_FAILED(rv)) return rv; 
   
  nsXPIDLCString descString;
  nsCOMPtr<nsIFileSpec> mailDirSpec;
  
  // Convert the nsILocalFile into an nsIFileSpec
  // TODO: convert users os nsIFileSpec to nsILocalFile
  // and avoid this step.
  rv = NS_NewFileSpecFromIFile(mailDir, getter_AddRefs(mailDirSpec));
  if (NS_FAILED(rv)) return rv;

  // set the default local path for "none"
  rv = server->SetDefaultLocalPath(mailDirSpec);
  if (NS_FAILED(rv)) return rv;
 
  if (migrating) {
  	// set the local path for this "none" server
	//
	// we need to set this to <profile>/Mail/Local Folders, because that's where
	// the 4.x "Local Mail" (when using imap) got copied.
	// it would be great to use the server key, but we don't know it
	// when we are copying of the mail.
	rv = mailDirSpec->AppendRelativeUnixPath(mLocalFoldersHostname.get());
	if (NS_FAILED(rv)) return rv; 
	rv = server->SetLocalPath(mailDirSpec);
	if (NS_FAILED(rv)) return rv;
    
	rv = mailDirSpec->Exists(&dirExists);
	if (!dirExists) {
	    mailDirSpec->CreateDir();
	}
  }

  // Create an account when valid server values are established.
  // This will keep the status of accounts sane by avoiding the addition of incomplete accounts. 
  nsCOMPtr<nsIMsgAccount> account;
  rv = accountManager->CreateAccount(getter_AddRefs(account));
  if (NS_FAILED(rv)) return rv;

  // notice, no identity for local mail
  // hook the server to the account 
  // after we set the server's local path
  // (see bug #66018)
  account->SetIncomingServer(server);

  // remember this as the local folders server
  rv = accountManager->SetLocalFoldersServer(server);
  if (NS_FAILED(rv)) return rv;

  return NS_OK;
}

nsresult
nsMessengerMigrator::ResetState()
{
  m_alreadySetNntpDefaultLocalPath = PR_FALSE;
  m_alreadySetImapDefaultLocalPath = PR_FALSE;

  // Reset 'm_oldMailType' in case the prefs file has changed. This is possible in quick launch
  // mode where the profile to be migrated is IMAP type but the current working profile is POP.
  nsresult rv = m_prefs->GetIntPref(PREF_4X_MAIL_SERVER_TYPE, &m_oldMailType);
  if (NS_FAILED(rv))
    m_oldMailType = -1;
  return rv;
}

NS_IMETHODIMP
nsMessengerMigrator::UpgradePrefs()
{
    nsresult rv;

    rv = getPrefService();
    if (NS_FAILED(rv)) return rv;

    // Reset some control vars, necessary in turbo mode.
    ResetState();

    // because mail.server_type defaults to 0 (pop) it will look the user
    // has something to migrate, even with an empty prefs.js file
    // ProceedWithMigration will check if there is something to migrate
    // if not, NS_FAILED(rv) will be true, and we'll return.
    // this plays nicely with msgMail3PaneWindow.js, which will launch the
    // Account Wizard if UpgradePrefs() fails.
    rv = ProceedWithMigration();
    if (NS_FAILED(rv)) {
#ifdef DEBUG_MIGRATOR
      printf("FAIL:  don't proceed with migration.\n");
#endif
      return rv;
    }
#ifdef DEBUG_MIGRATOR
    else {
      printf("PASS:  proceed with migration.\n");
    }
#endif 

    nsCOMPtr<nsIMsgAccountManager> accountManager = 
             do_GetService(NS_MSGACCOUNTMANAGER_CONTRACTID, &rv);
    if (NS_FAILED(rv)) return rv;

    // create a dummy identity, for migration only
    nsCOMPtr<nsIMsgIdentity> identity;

    rv = accountManager->CreateIdentity(getter_AddRefs(identity));
    if (NS_FAILED(rv)) return rv;

    rv = MigrateIdentity(identity);
    if (NS_FAILED(rv)) return rv;    
    
    nsCOMPtr<nsISmtpService> smtpService = 
             do_GetService(NS_SMTPSERVICE_CONTRACTID, &rv);
    if (NS_FAILED(rv)) return rv;    

    nsCOMPtr<nsISmtpServer> smtpServer;
    rv = smtpService->CreateSmtpServer(getter_AddRefs(smtpServer));
    if (NS_FAILED(rv)) return rv;    

    rv = MigrateSmtpServer(smtpServer);
    if (NS_FAILED(rv)) return rv;    

    // set the newly created smtp server as the default
    smtpService->SetDefaultServer(smtpServer); // ignore the error code....continue even if this call fails...


    if ( m_oldMailType == POP_4X_MAIL_TYPE) {
      // in 4.x, you could only have one pop account
      rv = MigratePopAccount(identity);
      if (NS_FAILED(rv)) return rv;

      // everyone gets a local mail account in 5.0
      rv = CreateLocalMailAccount(PR_TRUE);
      if (NS_FAILED(rv)) return rv;
    }
    else if (m_oldMailType == IMAP_4X_MAIL_TYPE) {
      rv = MigrateImapAccounts(identity);
      if (NS_FAILED(rv)) return rv;
      
      // if they had IMAP in 4.x, they also had "Local Mail"
      // we'll migrate that to "Local Folders"
      rv = MigrateLocalMailAccount();
      if (NS_FAILED(rv)) return rv;
    }
#ifdef HAVE_MOVEMAIL
    else if (m_oldMailType == MOVEMAIL_4X_MAIL_TYPE) {
	// if 4.x, you could only have one movemail account
	rv = MigrateMovemailAccount(identity);
	if (NS_FAILED(rv)) return rv;

        // everyone gets a local mail account in 5.0
        rv = CreateLocalMailAccount(PR_TRUE);
        if (NS_FAILED(rv)) return rv;
    }
#endif /* HAVE_MOVEMAIL */
    else {
#ifdef DEBUG_MIGRATOR
      printf("Unrecognized server type %d\n", m_oldMailType);
#endif
      return NS_ERROR_UNEXPECTED;
    }

    rv = MigrateNewsAccounts(identity);
    if (NS_FAILED(rv)) return rv;

    // this will migrate the ldap prefs
#if defined(MOZ_LDAP_XPCOM)
    nsCOMPtr <nsILDAPPrefsService> ldapPrefsService = do_GetService("@mozilla.org/ldapprefs-service;1", &rv);

    rv = ldapPrefsService->MigratePrefsIfNeeded();
    NS_ENSURE_SUCCESS(rv, rv);
#endif    
    rv = MigrateAddressBookPrefs();
    NS_ENSURE_SUCCESS(rv,rv);

    // XXX: No address book migration for 4.x see bug 35509

    m_prefs->ClearUserPref(PREF_4X_MAIL_POP_PASSWORD); // intentionally ignore the return value

    // we're done migrating, let's save the prefs
    nsCOMPtr<nsIPrefService> prefService(do_QueryInterface(m_prefs, &rv));
    if (NS_FAILED(rv)) return rv;

    rv = prefService->SavePrefFile(nsnull);
    if (NS_FAILED(rv)) return rv;

	// remove the temporary identity we used for migration purposes
    identity->ClearAllValues();
    rv = accountManager->RemoveIdentity(identity);

    return rv;
}

nsresult 
nsMessengerMigrator::SetUsernameIfNecessary()
{
    nsresult rv;
    nsXPIDLCString usernameIn4x;

    rv = m_prefs->GetCharPref(PREF_4X_MAIL_IDENTITY_USERNAME, getter_Copies(usernameIn4x));
    if (NS_SUCCEEDED(rv) && !usernameIn4x.IsEmpty()) {
        return NS_OK;
    }

    nsXPIDLString fullnameFromSystem;
    
    nsCOMPtr<nsIUserInfo> userInfo = do_GetService(NS_USERINFO_CONTRACTID, &rv);
    if (NS_FAILED(rv)) return rv;

    if (!userInfo) return NS_ERROR_FAILURE;

    rv = userInfo->GetFullname(getter_Copies(fullnameFromSystem));
    if (NS_FAILED(rv) || !((const PRUnichar *)fullnameFromSystem)) {
        // it is ok not to have this from the system
        return NS_OK;
    }

    nsCOMPtr<nsISupportsString> str(do_CreateInstance(NS_SUPPORTS_STRING_CONTRACTID, &rv));
    if (NS_SUCCEEDED(rv)) {
        str->SetData(fullnameFromSystem);
        rv = m_prefs->SetComplexValue(PREF_4X_MAIL_IDENTITY_USERNAME, NS_GET_IID(nsISupportsString), str);
    }

    return rv;
}

nsresult
nsMessengerMigrator::MigrateIdentity(nsIMsgIdentity *identity)
{
  nsresult rv;
  
  rv = SetUsernameIfNecessary();
  /* SetUsernameIfNecessary() can fail. */
  //if (NS_FAILED(rv)) return rv;

  /* NOTE:  if you add prefs here, make sure you update nsMsgIdentity::Copy() */
  MIGRATE_SIMPLE_STR_PREF(PREF_4X_MAIL_IDENTITY_USEREMAIL,identity,SetEmail)
  MIGRATE_SIMPLE_WSTR_PREF(PREF_4X_MAIL_IDENTITY_USERNAME,identity,SetFullName)
  MIGRATE_SIMPLE_STR_PREF(PREF_4X_MAIL_IDENTITY_REPLY_TO,identity,SetReplyTo)
  MIGRATE_SIMPLE_WSTR_PREF(PREF_4X_MAIL_IDENTITY_ORGANIZATION,identity,SetOrganization)
  MIGRATE_SIMPLE_BOOL_PREF(PREF_4X_MAIL_COMPOSE_HTML,identity,SetComposeHtml)
  MIGRATE_SIMPLE_FILE_PREF_TO_FILE_PREF(PREF_4X_MAIL_SIGNATURE_FILE,identity,SetSignature)
  MIGRATE_SIMPLE_FILE_PREF_TO_BOOL_PREF(PREF_4X_MAIL_SIGNATURE_FILE,identity,SetAttachSignature)
  MIGRATE_SIMPLE_INT_PREF(PREF_4X_MAIL_SIGNATURE_DATE,identity,SetSignatureDate)

  MIGRATE_SIMPLE_BOOL_PREF(PREF_4X_MAIL_ATTACH_VCARD, identity, SetAttachVCard)
  nsCOMPtr <nsIAddressBook> ab = do_CreateInstance(NS_ADDRESSBOOK_CONTRACTID);
  if (ab) 
  {
      nsXPIDLCString escapedVCardStr;
      rv = ab->Convert4xVCardPrefs(PREF_4X_MAIL_IDENTITY_VCARD_ROOT, getter_Copies(escapedVCardStr));
      if (NS_SUCCEEDED(rv) && !escapedVCardStr.IsEmpty()) 
      {
          rv = identity->SetEscapedVCard(escapedVCardStr.get());
          NS_ASSERTION(NS_SUCCEEDED(rv), "failed to set escaped vCard string");
      }
  }    

  /* NOTE:  if you add prefs here, make sure you update nsMsgIdentity::Copy() */
  return NS_OK;
}

nsresult
nsMessengerMigrator::MigrateSmtpServer(nsISmtpServer *server)
{
  MIGRATE_SIMPLE_STR_PREF(PREF_4X_NETWORK_HOSTS_SMTP_SERVER,server,SetHostname)
  MIGRATE_SIMPLE_STR_PREF(PREF_4X_MAIL_SMTP_NAME,server,SetUsername)
  MIGRATE_SIMPLE_INT_PREF(PREF_4X_MAIL_SMTP_SSL,server,SetTrySSL)

  return NS_OK;
}

nsresult
nsMessengerMigrator::SetNewsCopiesAndFolders(nsIMsgIdentity *identity)
{
  nsresult rv;
  
  MIGRATE_SIMPLE_BOOL_PREF(PREF_4X_NEWS_CC_SELF,identity,SetBccSelf)
  MIGRATE_SIMPLE_BOOL_PREF(PREF_4X_NEWS_USE_DEFAULT_CC,identity,SetBccOthers)
  MIGRATE_SIMPLE_STR_PREF(PREF_4X_NEWS_DEFAULT_CC,identity,SetBccList)
  MIGRATE_SIMPLE_BOOL_PREF(PREF_4X_NEWS_USE_FCC,identity,SetDoFcc)
  MIGRATE_SIMPLE_STR_PREF(PREF_4X_MAIL_DEFAULT_DRAFTS,identity,SetDraftFolder)
  MIGRATE_SIMPLE_STR_PREF(PREF_4X_MAIL_DEFAULT_TEMPLATES,identity,SetStationeryFolder)
      
  PRBool news_used_uri_for_sent_in_4x;
  rv = m_prefs->GetBoolPref(PREF_4X_NEWS_USE_IMAP_SENTMAIL, &news_used_uri_for_sent_in_4x);
  if (NS_FAILED(rv)) {
	  MIGRATE_SIMPLE_FILE_PREF_TO_CHAR_PREF(PREF_4X_NEWS_DEFAULT_FCC,identity,SetFccFolder)
  }
  else {
	  if (news_used_uri_for_sent_in_4x) {
	    MIGRATE_SIMPLE_STR_PREF(PREF_4X_NEWS_IMAP_SENTMAIL_PATH,identity,SetFccFolder)
	  }
	  else {
	    MIGRATE_SIMPLE_FILE_PREF_TO_CHAR_PREF(PREF_4X_NEWS_DEFAULT_FCC,identity,SetFccFolder)
	  }
  }

  if (m_oldMailType == IMAP_4X_MAIL_TYPE) {
	CONVERT_4X_URI(identity, PR_TRUE /* for news */, LOCAL_MAIL_FAKE_USER_NAME, mLocalFoldersHostname.get(), DEFAULT_4X_SENT_FOLDER_NAME,GetFccFolder,SetFccFolder,DEFAULT_FCC_FOLDER_PREF_NAME)
	CONVERT_4X_URI(identity, PR_TRUE /* for news */, LOCAL_MAIL_FAKE_USER_NAME, mLocalFoldersHostname.get(), DEFAULT_4X_TEMPLATES_FOLDER_NAME,GetStationeryFolder,SetStationeryFolder,DEFAULT_STATIONERY_FOLDER_PREF_NAME)
	CONVERT_4X_URI(identity, PR_TRUE /* for news */, LOCAL_MAIL_FAKE_USER_NAME, mLocalFoldersHostname.get(), DEFAULT_4X_DRAFTS_FOLDER_NAME,GetDraftFolder,SetDraftFolder,DEFAULT_DRAFT_FOLDER_PREF_NAME)
  }
  else if (m_oldMailType == POP_4X_MAIL_TYPE) {
    nsXPIDLCString pop_username;
    nsXPIDLCString pop_hostname;

    rv = m_prefs->GetCharPref(PREF_4X_MAIL_POP_NAME,getter_Copies(pop_username));
    if (NS_FAILED(rv)) return rv;

    rv = m_prefs->GetCharPref(PREF_4X_NETWORK_HOSTS_POP_SERVER, getter_Copies(pop_hostname));
    if (NS_FAILED(rv)) return rv;

	CONVERT_4X_URI(identity, PR_TRUE /* for news */, pop_username.get(), pop_hostname.get(), DEFAULT_4X_SENT_FOLDER_NAME,GetFccFolder,SetFccFolder,DEFAULT_FCC_FOLDER_PREF_NAME)
	CONVERT_4X_URI(identity, PR_TRUE /* for news */, pop_username.get(), pop_hostname.get(), DEFAULT_4X_TEMPLATES_FOLDER_NAME,GetStationeryFolder,SetStationeryFolder,DEFAULT_STATIONERY_FOLDER_PREF_NAME)
	CONVERT_4X_URI(identity, PR_TRUE /* for news */, pop_username.get(), pop_hostname.get(), DEFAULT_4X_DRAFTS_FOLDER_NAME,GetDraftFolder,SetDraftFolder,DEFAULT_DRAFT_FOLDER_PREF_NAME)
  }
#ifdef HAVE_MOVEMAIL
  else if (m_oldMailType == MOVEMAIL_4X_MAIL_TYPE) {
    nsXPIDLCString pop_username;

	rv = m_prefs->GetCharPref(PREF_4X_MAIL_POP_NAME, getter_Copies(pop_username));
    if (NS_FAILED(rv)) return rv;

	CONVERT_4X_URI(identity, PR_TRUE /* for news */, pop_username.get(), MOVEMAIL_FAKE_HOST_NAME, DEFAULT_4X_SENT_FOLDER_NAME,GetFccFolder,SetFccFolder,DEFAULT_FCC_FOLDER_PREF_NAME)
	CONVERT_4X_URI(identity, PR_TRUE /* for news */, pop_username.get(), MOVEMAIL_FAKE_HOST_NAME, DEFAULT_4X_TEMPLATES_FOLDER_NAME,GetStationeryFolder,SetStationeryFolder,DEFAULT_STATIONERY_FOLDER_PREF_NAME)
	CONVERT_4X_URI(identity, PR_TRUE /* for news */, pop_username.get(), MOVEMAIL_FAKE_HOST_NAME, DEFAULT_4X_DRAFTS_FOLDER_NAME,GetDraftFolder,SetDraftFolder,DEFAULT_DRAFT_FOLDER_PREF_NAME)
  }
#endif /* HAVE_MOVEMAIL */
  else {
	return NS_ERROR_UNEXPECTED;
  }

  return NS_OK;
}

nsresult
nsMessengerMigrator::SetMailCopiesAndFolders(nsIMsgIdentity *identity, const char *username, const char *hostname)
{
  nsresult rv;
  
  MIGRATE_SIMPLE_BOOL_PREF(PREF_4X_MAIL_CC_SELF,identity,SetBccSelf)
  MIGRATE_SIMPLE_BOOL_PREF(PREF_4X_MAIL_USE_DEFAULT_CC,identity,SetBccOthers)
  MIGRATE_SIMPLE_STR_PREF(PREF_4X_MAIL_DEFAULT_CC,identity,SetBccList)
  MIGRATE_SIMPLE_BOOL_PREF(PREF_4X_MAIL_USE_FCC,identity,SetDoFcc)
  MIGRATE_SIMPLE_STR_PREF(PREF_4X_MAIL_DEFAULT_DRAFTS,identity,SetDraftFolder)
  MIGRATE_SIMPLE_STR_PREF(PREF_4X_MAIL_DEFAULT_TEMPLATES,identity,SetStationeryFolder)

  PRBool imap_used_uri_for_sent_in_4x;
  rv = m_prefs->GetBoolPref(PREF_4X_MAIL_USE_IMAP_SENTMAIL, &imap_used_uri_for_sent_in_4x);


  if (NS_FAILED(rv)) {
	MIGRATE_SIMPLE_FILE_PREF_TO_CHAR_PREF(PREF_4X_MAIL_DEFAULT_FCC,identity,SetFccFolder)
  }
  else {
	if (imap_used_uri_for_sent_in_4x) {
		MIGRATE_SIMPLE_STR_PREF(PREF_4X_MAIL_IMAP_SENTMAIL_PATH,identity,SetFccFolder)
	}
	else {
		MIGRATE_SIMPLE_FILE_PREF_TO_CHAR_PREF(PREF_4X_MAIL_DEFAULT_FCC,identity,SetFccFolder)
	}
  }
  CONVERT_4X_URI(identity, PR_FALSE /* for news */, username, hostname, DEFAULT_4X_SENT_FOLDER_NAME,GetFccFolder,SetFccFolder,DEFAULT_FCC_FOLDER_PREF_NAME)
  CONVERT_4X_URI(identity, PR_FALSE /* for news */, username, hostname, DEFAULT_4X_TEMPLATES_FOLDER_NAME,GetStationeryFolder,SetStationeryFolder,DEFAULT_STATIONERY_FOLDER_PREF_NAME)
  CONVERT_4X_URI(identity, PR_FALSE /* for news */, username, hostname, DEFAULT_4X_DRAFTS_FOLDER_NAME,GetDraftFolder,SetDraftFolder,DEFAULT_DRAFT_FOLDER_PREF_NAME)
    
  return NS_OK;
}

// caller will free the memory
nsresult
nsMessengerMigrator::Convert4XUri(const char *old_uri, PRBool for_news, const char *aUsername, const char *aHostname, const char *default_folder_name, const char *default_pref_name, char **new_uri)
{
  nsresult rv;
  *new_uri = nsnull;

  if (!old_uri) {
    return NS_ERROR_NULL_POINTER;
  }

  nsXPIDLCString default_pref_value;
  rv = m_prefs->GetCharPref(default_pref_name, getter_Copies(default_pref_value));
  NS_ENSURE_SUCCESS(rv,rv);

  // if the old pref value was "", old_uri will be default value (mail.identity.default.fcc_folder)
  // so if old_uri is the default, do some default conversion
  if (!nsCRT::strcmp(old_uri, default_pref_value.get())) {
	if (!aUsername || !aHostname) {
		// if the old uri was "", and we don't know the username or the hostname
		// leave it blank.  either someone will be back to fix it,
		// SetNewsCopiesAndFolders() and SetMailCopiesAndFolders()
		// or we are out of luck.
		*new_uri = PR_smprintf("");
		return NS_OK;
	}

	if ((m_oldMailType == IMAP_4X_MAIL_TYPE) && !for_news) {
        nsXPIDLCString escaped_aUsername;
        ESCAPE_USER_NAME(escaped_aUsername,aUsername);
		*new_uri = PR_smprintf("%s/%s@%s/%s",IMAP_SCHEMA,(const char *)escaped_aUsername,aHostname,default_folder_name);
	}
	else if ((m_oldMailType == POP_4X_MAIL_TYPE) 
#ifdef HAVE_MOVEMAIL
		|| (m_oldMailType == MOVEMAIL_4X_MAIL_TYPE)
#endif /* HAVE_MOVEMAIL */
		|| (m_oldMailType == IMAP_4X_MAIL_TYPE)) {
        nsXPIDLCString escaped_aUsername;
        ESCAPE_USER_NAME(escaped_aUsername,aUsername);
		*new_uri = PR_smprintf("%s/%s@%s/%s",MAILBOX_SCHEMA,(const char *)escaped_aUsername,aHostname,default_folder_name);
	}
	else {
		*new_uri = PR_smprintf("");
		return NS_ERROR_UNEXPECTED;
	}
    return NS_OK;
  }

#ifdef DEBUG_MIGRATOR
  printf("old 4.x folder uri = >%s<\n", old_uri);
#endif /* DEBUG_MIGRATOR */

  if (PL_strncasecmp(IMAP_SCHEMA,old_uri,IMAP_SCHEMA_LENGTH) == 0) {
	nsXPIDLCString hostname;
	nsXPIDLCString username;

    nsCOMPtr<nsIURI> uri;
    rv = NS_NewURI(getter_AddRefs(uri), nsDependentCString(old_uri));
    if (NS_FAILED(rv)) return rv;

    rv = uri->GetHost(hostname);
    if (NS_FAILED(rv)) return rv;

    rv = uri->GetUsername(username);
    if (NS_FAILED(rv)) return rv;

    // in 4.x, mac and windows stored the URI as IMAP://<hostname> 
	// if the URI was the default folder on the server.
	// If it wasn't the default folder, they would have stored it as
	if (username.IsEmpty()) {
		char *imap_username = nsnull;
		char *prefname = nsnull;
		prefname = PR_smprintf("mail.imap.server.%s.userName", hostname.get());
		if (!prefname) return NS_ERROR_FAILURE;

		rv = m_prefs->GetCharPref(prefname, &imap_username);
		PR_FREEIF(prefname);
		if (NS_FAILED(rv) || !imap_username || !*imap_username) {
			*new_uri = PR_smprintf("");
			return NS_ERROR_FAILURE;
		}
		else {
			// new_uri = imap://<username>@<hostname>/<folder name>
#ifdef DEBUG_MIGRATOR
			printf("new_uri = %s/%s@%s/%s\n",IMAP_SCHEMA, imap_username, hostname.get(), default_folder_name);
#endif /* DEBUG_MIGRATOR */
			*new_uri = PR_smprintf("%s/%s@%s/%s",IMAP_SCHEMA, imap_username, hostname.get(), default_folder_name);
			return NS_OK;      
		}
    }
    else {
		// IMAP uri's began with "IMAP:/".  we need that to be "imap:/"
    // 4.x escapes imap uri in case folder names contain spaces. For example:
    //   "imap://qatest20@nsmail-2/Folders%2FJan%20Sent"
    // and it needs to be converted to:
    //   "imap://qatest20@nsmail-2/Folders/Jan Sent"
    // ie, we need to unescape the folder names.
#ifdef DEBUG_MIGRATOR
		printf("new_uri = %s%s\n",IMAP_SCHEMA,old_uri+IMAP_SCHEMA_LENGTH);
#endif /* DEBUG_MIGRATOR */

    char *unescaped_uri = nsCRT::strdup(old_uri);
    nsUnescape(unescaped_uri);
    *new_uri = PR_smprintf("%s%s",IMAP_SCHEMA,unescaped_uri+IMAP_SCHEMA_LENGTH);
    nsCRT::free(unescaped_uri);
		return NS_OK;
	}
  }

  char *usernameAtHostname = nsnull;
  nsCOMPtr <nsIFileSpec> mail_dir;
  char *mail_directory_value = nsnull;
  rv = m_prefs->GetComplexValue(PREF_PREMIGRATION_MAIL_DIRECTORY, NS_GET_IID(nsIFileSpec), getter_AddRefs(mail_dir));
  if (NS_SUCCEEDED(rv)) {
	rv = mail_dir->GetUnixStyleFilePath(&mail_directory_value);
  }
  if (NS_FAILED(rv) || !mail_directory_value || !*mail_directory_value) {
#ifdef DEBUG_MIGRATOR
    printf("%s was not set, attempting to use %s instead.\n",PREF_PREMIGRATION_MAIL_DIRECTORY,PREF_MAIL_DIRECTORY);
#endif
    PR_FREEIF(mail_directory_value);

    rv = m_prefs->GetComplexValue(PREF_MAIL_DIRECTORY, NS_GET_IID(nsIFileSpec), getter_AddRefs(mail_dir));
    if (NS_SUCCEEDED(rv)) {
	rv = mail_dir->GetUnixStyleFilePath(&mail_directory_value);
    } 

    if (NS_FAILED(rv) || !mail_directory_value || !*mail_directory_value) {
      NS_ASSERTION(0,"failed to get a base value for the mail.directory");
      return NS_ERROR_UNEXPECTED;
    }
  }

  if (m_oldMailType == POP_4X_MAIL_TYPE) {
    nsXPIDLCString pop_username;
    nsXPIDLCString pop_hostname;

    rv = m_prefs->GetCharPref(PREF_4X_MAIL_POP_NAME, getter_Copies(pop_username));
    if (NS_FAILED(rv)) return rv;

    rv = m_prefs->GetCharPref(PREF_4X_NETWORK_HOSTS_POP_SERVER, getter_Copies(pop_hostname));
    if (NS_FAILED(rv)) return rv;

    // Need to escape hostname in case it contains spaces.
    nsXPIDLCString escaped_pop_username, escaped_pop_hostname;
    ESCAPE_USER_NAME(escaped_pop_hostname, pop_hostname);
    ESCAPE_USER_NAME(escaped_pop_username,pop_username);

    usernameAtHostname = PR_smprintf("%s@%s", escaped_pop_username.get(), escaped_pop_hostname.get());
  }
  else if (m_oldMailType == IMAP_4X_MAIL_TYPE) {
    // Need to escape hostname in case it contains spaces.
    nsXPIDLCString escaped_localfolder_hostname;
    ESCAPE_USER_NAME(escaped_localfolder_hostname,mLocalFoldersHostname.get());
    usernameAtHostname = PR_smprintf("%s@%s",LOCAL_MAIL_FAKE_USER_NAME, escaped_localfolder_hostname.get());
  }
#ifdef HAVE_MOVEMAIL
  else if (m_oldMailType == MOVEMAIL_4X_MAIL_TYPE) {
	nsXPIDLCString movemail_username;

	rv = m_prefs->GetCharPref(PREF_4X_MAIL_POP_NAME, getter_Copies(movemail_username));
	if (NS_FAILED(rv)) return rv; 
	
    nsXPIDLCString escaped_movemail_username;

    ESCAPE_USER_NAME(escaped_movemail_username,movemail_username);

	usernameAtHostname = PR_smprintf("%s@%s",(const char *)escaped_movemail_username,MOVEMAIL_FAKE_HOST_NAME);
  }
#endif /* HAVE_MOVEMAIL */
  else {
#ifdef DEBUG_MIGRATOR
    printf("Unrecognized server type %d\n", m_oldMailType);
#endif
    return NS_ERROR_UNEXPECTED;
  }

  const char *folderPath;

  // mail_directory_value is already in UNIX style at this point...
  if (PL_strncasecmp(MAILBOX_SCHEMA,old_uri,MAILBOX_SCHEMA_LENGTH) == 0) {
#ifdef DEBUG_MIGRATOR
	printf("turn %s into %s/%s/(%s - %s)\n",old_uri,MAILBOX_SCHEMA,usernameAtHostname,old_uri + MAILBOX_SCHEMA_LENGTH,mail_directory_value);
#endif
	// the extra -1 is because in 4.x, we had this:
	// mailbox:<PATH> instead of mailbox:/<PATH> 
	folderPath = old_uri + MAILBOX_SCHEMA_LENGTH + PL_strlen(mail_directory_value) -1;
  }
  else {
#ifdef DEBUG_MIGRATOR
    printf("turn %s into %s/%s/(%s - %s)\n",old_uri,MAILBOX_SCHEMA,usernameAtHostname,old_uri,mail_directory_value);
#endif
	folderPath = old_uri + PL_strlen(mail_directory_value);
  }
  

  // if folder path is "", then the URI was mailbox:/<foobar>
  // and the directory pref was <foobar>
  // this meant it was reall <foobar>/<default folder name>
  // this insanity only happened on mac and windows.
  // Need to escape spaces in folder name (ie, "A Folder" --> "A%20Folder").
  if (!folderPath || !*folderPath) {
    nsXPIDLCString escaped_default_folder;
    ESCAPE_FOLDER_NAME(escaped_default_folder, default_folder_name);
    *new_uri = PR_smprintf("%s/%s/%s",MAILBOX_SCHEMA,usernameAtHostname, escaped_default_folder.get());
  }
  else {
	// if the folder path starts with a /, we don't need to add one.
	// the reason for this is on UNIX, we store mail.directory as "/home/sspitzer/nsmail"
	// but on windows, its "C:\foo\bar\mail"
    // In 4.x if the local folder names have hierarchy in it then the uri is like:
    //   "/My Folders.sbd/New Drafts#1"
    // and it needs to be converted to:
    //   "/My%20Folders/New%20Drafts%231"
    // ie, we need to get rid of all ".sbd" and then escape the URIs (ie, #->%23).
    nsCAutoString clean_folderPath(folderPath);
    clean_folderPath.ReplaceSubstring(".sbd/", "/");
    nsXPIDLCString escaped_folderPath;
    ESCAPE_FOLDER_NAME(escaped_folderPath,clean_folderPath.get());
    *new_uri = PR_smprintf("%s/%s%s%s",MAILBOX_SCHEMA,usernameAtHostname,(folderPath[0] == '/')?"":"/",escaped_folderPath.get());
  }

  if (!*new_uri) {
#ifdef DEBUG_MIGRATOR
    printf("failed to convert 4.x uri: %s\n", old_uri);
#endif
    NS_ASSERTION(0,"uri conversion code not complete");
    return NS_ERROR_FAILURE;
  }

  PR_FREEIF(usernameAtHostname);
  PR_FREEIF(mail_directory_value);

  return NS_OK;
}

nsresult
nsMessengerMigrator::MigrateLocalMailAccount() 
{
  nsresult rv;
  nsCOMPtr<nsIMsgAccountManager> accountManager = 
           do_GetService(NS_MSGACCOUNTMANAGER_CONTRACTID, &rv);
  if (NS_FAILED(rv)) return rv;

  // create the server
  // "none" is the type we use for migrating 4.x "Local Mail"
  nsCOMPtr<nsIMsgIncomingServer> server;
  rv = accountManager->CreateIncomingServer(LOCAL_MAIL_FAKE_USER_NAME,
                            mLocalFoldersHostname.get(),
                            "none", getter_AddRefs(server));
  NS_ENSURE_SUCCESS(rv,rv);

  // now upgrade all the prefs
  // some of this ought to be moved out into the NONE implementation
  nsCOMPtr<nsINoIncomingServer> noServer;
  noServer = do_QueryInterface(server, &rv);
  if (NS_FAILED(rv)) return rv;

  // create the directory structure for old 4.x "Local Mail"
  // under <profile dir>/Mail/Local Folders or
  // <"mail.directory" pref>/Local Folders 
  nsCOMPtr<nsIFile> mailDir;
  nsFileSpec dir;
  PRBool dirExists;

  // if the "mail.directory" pref is set, use that.
  // if they used -installer, this pref will point to where their files got copied
  nsCOMPtr<nsILocalFile> localFile;
  rv = m_prefs->GetComplexValue(PREF_MAIL_DIRECTORY, NS_GET_IID(nsILocalFile), getter_AddRefs(localFile));
  if (NS_SUCCEEDED(rv))
    mailDir = localFile;
  
  if (!mailDir) {
    // we want <profile>/Mail
    rv = NS_GetSpecialDirectory(NS_APP_MAIL_50_DIR, getter_AddRefs(mailDir));
    if (NS_FAILED(rv)) return rv;
  }

  rv = mailDir->Exists(&dirExists);
  if (NS_SUCCEEDED(rv) && !dirExists)
    rv = mailDir->Create(nsIFile::DIRECTORY_TYPE, 0775);
  if (NS_FAILED(rv)) return rv;  
    
  nsCOMPtr<nsIFileSpec> mailDirSpec;
  
  // Convert the nsILocalFile into an nsIFileSpec
  // TODO: convert users of nsIFileSpec to nsILocalFile
  // and avoid this step.
  rv = NS_NewFileSpecFromIFile(mailDir, getter_AddRefs(mailDirSpec));
  if (NS_FAILED(rv)) return rv;

  // set the default local path for "none"
  rv = server->SetDefaultLocalPath(mailDirSpec);
  if (NS_FAILED(rv)) return rv;

  // set the local path for this "none" server

  // get the migration mode for mail
  PRBool copyMailFileInMigration = PR_TRUE;
  rv = m_prefs->GetBoolPref(PREF_MIGRATION_MODE_FOR_MAIL, &copyMailFileInMigration);
  if (NS_FAILED(rv))
    return rv;
  if (copyMailFileInMigration)
  {
    // we need to set this to <profile>/Mail/Local Folders, because that's where
    // the 4.x "Local Mail" (when using imap) got copied.
    // it would be great to use the server key, but we don't know it
    // when we are copying of the mail.
    rv = mailDirSpec->AppendRelativeUnixPath(mLocalFoldersHostname.get());
    if (NS_FAILED(rv)) return rv; 
  }
  rv = server->SetLocalPath(mailDirSpec);
  if (NS_FAILED(rv)) return rv;
    
  rv = mailDirSpec->Exists(&dirExists);
  if (!dirExists) {
    mailDirSpec->CreateDir();
  }
  
  // we don't want "nobody at Local Folders" to show up in the
  // folder pane, so we set the pretty name to "Local Folders"
  server->SetPrettyName(mLocalFoldersName.get());
  
  // pass the "Local Folders" server so the send later uri pref 
  // will be "mailbox://nobody@Local Folders/Unsent Messages"
  rv = SetSendLaterUriPref(server);
  if (NS_FAILED(rv)) return rv;

  // copy the default templates into the Templates folder
  nsCOMPtr <nsINoIncomingServer> noneServer;
  noneServer = do_QueryInterface(server, &rv);
  if (NS_FAILED(rv)) return rv;
  if (!noneServer) return NS_ERROR_FAILURE;
  rv = noneServer->CopyDefaultMessages("Templates",mailDirSpec);
  if (NS_FAILED(rv)) return rv;
 
  // Create an account when valid server values are established.
  // This will keep the status of accounts sane by avoiding the addition of incomplete accounts. 
  nsCOMPtr<nsIMsgAccount> account;
  rv = accountManager->CreateAccount(getter_AddRefs(account));
  if (NS_FAILED(rv)) return rv;

  // notice, no identity for local mail
  // hook the server to the account 
  // after we set the server's local path
  // (see bug #66018)
  rv = account->SetIncomingServer(server);
  if (NS_FAILED(rv)) return rv;

  // remember this as the local folders server
  rv = accountManager->SetLocalFoldersServer(server);
  if (NS_FAILED(rv)) return rv;

  return NS_OK;
}

#ifdef HAVE_MOVEMAIL
nsresult
nsMessengerMigrator::MigrateMovemailAccount(nsIMsgIdentity *identity)
{
  nsresult rv;
  
  nsCOMPtr<nsIMsgIncomingServer> server;
  
  nsCOMPtr<nsIMsgAccountManager> accountManager = 
           do_GetService(NS_MSGACCOUNTMANAGER_CONTRACTID, &rv);
  if (NS_FAILED(rv)) return rv;

  // get the pop username
  // movemail used the pop username in 4.x
  nsXPIDLCString username;
  rv = m_prefs->GetCharPref(PREF_4X_MAIL_POP_NAME, getter_Copies(username));
  if (NS_FAILED(rv)) return rv;

  //
  // create the server
  //
  rv = accountManager->CreateIncomingServer(username, MOVEMAIL_FAKE_HOST_NAME,
                            "movemail", getter_AddRefs(server));
  NS_ENSURE_SUCCESS(rv,rv);

  // create the identity
  nsCOMPtr<nsIMsgIdentity> copied_identity;
  rv = accountManager->CreateIdentity(getter_AddRefs(copied_identity));
  if (NS_FAILED(rv)) return rv;

  // now upgrade all the prefs
  nsCOMPtr <nsIFileSpec> mailDir;
  nsFileSpec dir;
  PRBool dirExists;

  rv = MigrateOldMailPrefs(server);
  if (NS_FAILED(rv)) return rv;

  // if they used -installer, this pref will point to where their files got copied
  rv = m_prefs->GetComplexValue(PREF_MAIL_DIRECTORY, NS_GET_IID(nsIFileSpec), getter_AddRefs(mailDir));
  // create the directory structure for old 4.x pop mail
  // under <profile dir>/Mail/movemail or
  // <"mail.directory" pref>/movemail
  //
  // if the "mail.directory" pref is set, use that.
  if (NS_FAILED(rv)) {
    // we wan't <profile>/Mail
    nsCOMPtr<nsIFile> aFile;
    
    rv = NS_GetSpecialDirectory(NS_APP_MAIL_50_DIR, getter_AddRefs(aFile));
    if (NS_FAILED(rv)) return rv;

    NS_NewFileSpecFromIFile(aFile, getter_AddRefs(mailDir));
    if (NS_FAILED(rv)) return rv;
  }

  // set the default local path for "none" (eventually, "movemail")
  rv = server->SetDefaultLocalPath(mailDir);
  if (NS_FAILED(rv)) return rv;

  rv = mailDir->Exists(&dirExists);
  if (!dirExists) {
    mailDir->CreateDir();
  }

  // we want .../Mail/movemail, not .../Mail
  rv = mailDir->AppendRelativeUnixPath(MOVEMAIL_FAKE_HOST_NAME);
  if (NS_FAILED(rv)) return rv;
  
  // set the local path for this "none" (eventually, "movemail" server)
  rv = server->SetLocalPath(mailDir);
  if (NS_FAILED(rv)) return rv;
    
  rv = mailDir->Exists(&dirExists);
  if (!dirExists) {
    mailDir->CreateDir();
  }

  // Create an account when valid server and identity values are established.
  // This will keep the status of accounts sane by avoiding the addition of incomplete accounts. 
  nsCOMPtr<nsIMsgAccount> account;
  rv = accountManager->CreateAccount(getter_AddRefs(account));
  if (NS_FAILED(rv)) return rv;
    
  // hook the server to the account 
  // before setting the copies and folder prefs
  // (see bug #31904)
  // but after we set the server's local path
  // (see bug #66018)
  account->SetIncomingServer(server);
  account->AddIdentity(copied_identity);

  // make this new identity a copy of the identity
  // that we created out of the 4.x prefs
  rv = copied_identity->Copy(identity);
  if (NS_FAILED(rv)) return rv;

  // XXX: this probably won't work yet...
  // set the cc and fcc values
  rv = SetMailCopiesAndFolders(copied_identity, (const char *)username, MOVEMAIL_FAKE_HOST_NAME);
  if (NS_FAILED(rv)) return rv;
  
  // pass the server so the send later uri pref 
  // will be something like "mailbox://sspitzer@movemail/Unsent Messages"
  rv = SetSendLaterUriPref(server);
  if (NS_FAILED(rv)) return rv;

  // we could only have one movemail account in 4.x, so we make it the default in 5.0
  rv = accountManager->SetDefaultAccount(account);
  return rv;
}
#endif /* HAVE_MOVEMAIL */

nsresult
nsMessengerMigrator::MigratePopAccount(nsIMsgIdentity *identity)
{
  nsresult rv;
  nsCOMPtr<nsIMsgAccountManager> accountManager = 
           do_GetService(NS_MSGACCOUNTMANAGER_CONTRACTID, &rv);
  if (NS_FAILED(rv)) return rv;

  nsCOMPtr<nsIMsgIncomingServer> server;

  // get the pop username
  nsXPIDLCString username;
  rv = m_prefs->GetCharPref(PREF_4X_MAIL_POP_NAME, getter_Copies(username));
  if (NS_FAILED(rv)) return rv;

  // get the hostname and port
  nsXPIDLCString hostAndPort;
  rv = m_prefs->GetCharPref(PREF_4X_NETWORK_HOSTS_POP_SERVER,
                             getter_Copies(hostAndPort));
  if (NS_FAILED(rv)) return rv;

  PRInt32 port = -1;
  nsCAutoString hostname(hostAndPort);
  PRInt32 colonPos = hostname.FindChar(':');
  if (colonPos != -1) {
        hostname.Truncate(colonPos);
        
        // migrate the port from hostAndPort
        nsCAutoString portStr(hostAndPort + colonPos);
        PRInt32 err;
        port = portStr.ToInteger(&err);
        NS_ASSERTION(err == 0, "failed to get the port\n");
        if (err != 0) port=-1;
  }

  //
  // create the server
  //
  rv = accountManager->CreateIncomingServer(username.get(), hostname.get(), "pop3",
                            getter_AddRefs(server));
  NS_ENSURE_SUCCESS(rv,rv);

  // if we got a valid port from above, set it here
  if (port > 0) {
    server->SetPort(port);
  }
  
  // now upgrade all the prefs
  nsCOMPtr <nsIFile> mailDir;
  nsFileSpec dir;
  PRBool dirExists;

#ifdef DEBUG_MIGRATOR
  PRInt32 portValue;
  rv = server->GetPort(&portValue);
  printf("HOSTNAME = %s\n", hostname.get());
  printf("PORT = %d\n", portValue);
#endif /* DEBUG_MIGRATOR */

  rv = MigrateOldMailPrefs(server);
  if (NS_FAILED(rv)) return rv;

  // create the directory structure for old 4.x pop mail
  // under <profile dir>/Mail/<pop host name> or
  // <"mail.directory" pref>/<pop host name>
  //
  // if the "mail.directory" pref is set, use that.
  // if they used -installer, this pref will point to where their files got copied
  nsCOMPtr<nsILocalFile> localFile;
  rv = m_prefs->GetComplexValue(PREF_MAIL_DIRECTORY, NS_GET_IID(nsILocalFile), getter_AddRefs(localFile));
  if (NS_SUCCEEDED(rv))
    mailDir = localFile;
  
  if (!mailDir) {
    // we wan't <profile>/Mail
    rv = NS_GetSpecialDirectory(NS_APP_MAIL_50_DIR, getter_AddRefs(mailDir));
    if (NS_FAILED(rv)) return rv;
  }

  rv = mailDir->Exists(&dirExists);
  if (NS_SUCCEEDED(rv) && !dirExists)
    rv = mailDir->Create(nsIFile::DIRECTORY_TYPE, 0775);
  if (NS_FAILED(rv)) return rv;
  
  nsCOMPtr<nsIFileSpec> mailDirSpec;
  
  // Convert the nsILocalFile into an nsIFileSpec
  // TODO: convert users os nsIFileSpec to nsILocalFile
  // and avoid this step.
  rv = NS_NewFileSpecFromIFile(mailDir, getter_AddRefs(mailDirSpec));
  if (NS_FAILED(rv)) return rv;

  // set the default local path for "pop3"
  rv = server->SetDefaultLocalPath(mailDirSpec);
  if (NS_FAILED(rv)) return rv;

  rv = mailDirSpec->Exists(&dirExists);
  if (!dirExists) {
    mailDirSpec->CreateDir();
  }

  // we want .../Mail/<hostname>, not .../Mail
  // Only host name is required 
  rv = mailDirSpec->AppendRelativeUnixPath(hostname.get());
  if (NS_FAILED(rv)) return rv;
  
  // set the local path for this "pop3" server
  rv = server->SetLocalPath(mailDirSpec);
  if (NS_FAILED(rv)) return rv;
    
  rv = mailDirSpec->Exists(&dirExists);
  if (!dirExists) {
    mailDirSpec->CreateDir();
  }
    
  // pass the pop server so the send later uri pref 
  // will be something like "mailbox://sspitzer@tintin/Unsent Messages"
  rv = SetSendLaterUriPref(server);
  if (NS_FAILED(rv)) return rv;

  // Set check for new mail option for default account to TRUE 
  rv = server->SetLoginAtStartUp(PR_TRUE);

  // create the identity
  nsCOMPtr<nsIMsgIdentity> copied_identity;
  rv = accountManager->CreateIdentity(getter_AddRefs(copied_identity));
  if (NS_FAILED(rv)) return rv;

  // Create an account when valid server and identity values are established.
  // This will keep the status of accounts sane by avoiding the addition of incomplete accounts. 
  nsCOMPtr<nsIMsgAccount> account;
  rv = accountManager->CreateAccount(getter_AddRefs(account));
  if (NS_FAILED(rv)) return rv;

  // hook the server to the account 
  // before setting the copies and folder prefs
  // (see bug #31904)
  // but after we set the server's local path
  // (see bug #66018)
  account->SetIncomingServer(server);
  account->AddIdentity(copied_identity);

  // we could only have one pop account in 4.x, so we make it the default in 5.0
  rv = accountManager->SetDefaultAccount(account);
  NS_ENSURE_SUCCESS(rv, rv);

  // make this new identity a copy of the identity
  // that we created out of the 4.x prefs
  rv = copied_identity->Copy(identity);
  if (NS_FAILED(rv)) return rv;

  rv = SetMailCopiesAndFolders(copied_identity, username.get(), hostname.get());
  if (NS_FAILED(rv)) return rv;
  
  return rv;
}
nsresult 
nsMessengerMigrator::SetSendLaterUriPref(nsIMsgIncomingServer *server)
{
	nsresult rv;

	// set "mail.default_sendlater_uri" to something like
	// mailbox://nobody@Local Folders/Unsent Messages"
	// mailbox://sspitzer@tintin/Unsent Messages"
	//
	// note, the schema is mailbox:/ 
	// Unsent is an off-line thing, and that needs to be
	// on the disk, not on an imap server.
	nsXPIDLCString username;
	rv = server->GetUsername(getter_Copies(username));
	if (NS_FAILED(rv)) return rv;

	nsXPIDLCString hostname;
	rv = server->GetHostName(getter_Copies(hostname));
	if (NS_FAILED(rv)) return rv;

  nsXPIDLCString escaped_username, escaped_hostname;
  ESCAPE_USER_NAME(escaped_hostname, hostname);
  ESCAPE_USER_NAME(escaped_username,username);

	char *sendLaterUriStr = nsnull;
	sendLaterUriStr = PR_smprintf("%s/%s@%s/%s", MAILBOX_SCHEMA, escaped_username.get(), escaped_hostname.get(), UNSENT_MESSAGES_FOLDER_NAME);
	m_prefs->SetCharPref(PREF_MAIL_DEFAULT_SENDLATER_URI, sendLaterUriStr);
	PR_FREEIF(sendLaterUriStr);

	return NS_OK;
}

nsresult
nsMessengerMigrator::MigrateOldMailPrefs(nsIMsgIncomingServer * server)
{
  nsresult rv;

  // don't migrate the remember password pref.  see bug #42216 
  //MIGRATE_SIMPLE_BOOL_PREF(PREF_4X_MAIL_REMEMBER_PASSWORD,server,SetRememberPassword)
  rv = server->SetRememberPassword(PR_FALSE);
  if (NS_FAILED(rv)) return rv;
  
  rv = server->SetPassword(nsnull);
  if (NS_FAILED(rv)) return rv;

  MIGRATE_SIMPLE_BOOL_PREF(PREF_4X_MAIL_CHECK_NEW_MAIL,server,SetDoBiff)
  MIGRATE_SIMPLE_INT_PREF(PREF_4X_MAIL_CHECK_TIME,server,SetBiffMinutes)
  MIGRATE_SIMPLE_BOOL_PREF(PREF_4X_MAIL_POP3_GETS_NEW_MAIL,server,SetDownloadOnBiff)

  nsCOMPtr<nsIPop3IncomingServer> popServer;
  popServer = do_QueryInterface(server, &rv);
  if (NS_SUCCEEDED(rv) && popServer) {
	  MIGRATE_SIMPLE_BOOL_PREF(PREF_4X_MAIL_LEAVE_ON_SERVER,popServer,SetLeaveMessagesOnServer)
	  MIGRATE_SIMPLE_BOOL_PREF(PREF_4X_MAIL_DELETE_MAIL_LEFT_ON_SERVER,popServer,SetDeleteMailLeftOnServer) 
  }
  else {
	// could be a movemail server, in that case do nothing.
  }

  return NS_OK;
}

nsresult
nsMessengerMigrator::MigrateImapAccounts(nsIMsgIdentity *identity)
{
  nsresult rv;
  char *hostList=nsnull;
  rv = getPrefService();
  if (NS_FAILED(rv)) return rv;

  rv = m_prefs->GetCharPref(PREF_4X_NETWORK_HOSTS_IMAP_SERVER, &hostList);
  if (NS_FAILED(rv)) return rv;
  
  if (!hostList || !*hostList) return NS_OK;  // NS_ERROR_FAILURE?
  
  char *token = nsnull;
  char *rest = hostList;
  nsCAutoString str;

  PRBool isDefaultAccount = PR_TRUE;
      
  token = nsCRT::strtok(rest, ",", &rest);
  while (token && *token) {
    str = token;
    str.StripWhitespace();
    
    if (!str.IsEmpty()) {
      // str is the hostname
      rv = MigrateImapAccount(identity,str.get(),isDefaultAccount);
      if  (NS_FAILED(rv)) {
        // failed to migrate.  bail.
        return rv;
      }
      str = "";
      // the default imap server in 4.x was the first one in the list.
      // so after we've seen the first one, the rest are not the default
      isDefaultAccount = PR_FALSE;
    }
    token = nsCRT::strtok(rest, ",", &rest);
  }
  PR_FREEIF(hostList);
  return NS_OK;
}

nsresult
nsMessengerMigrator::MigrateImapAccount(nsIMsgIdentity *identity, const char *hostAndPort, PRBool isDefaultAccount)
{
  nsresult rv;  

  nsCOMPtr<nsIMsgAccountManager> accountManager = 
           do_GetService(NS_MSGACCOUNTMANAGER_CONTRACTID, &rv);
  if (NS_FAILED(rv)) return rv;

  if (!hostAndPort) return NS_ERROR_NULL_POINTER;
 
  // get the old username
  nsXPIDLCString username;
  char *imapUsernamePref =
    PR_smprintf("mail.imap.server.%s.userName", hostAndPort);
  rv = m_prefs->GetCharPref(imapUsernamePref, getter_Copies(username));
  PR_FREEIF(imapUsernamePref);
  NS_ENSURE_SUCCESS(rv,rv);

  PRBool isSecure = PR_FALSE;
  char *imapIsSecurePref = PR_smprintf("mail.imap.server.%s.isSecure", hostAndPort);
  rv = m_prefs->GetBoolPref(imapIsSecurePref, &isSecure);
  PR_FREEIF(imapIsSecurePref);
  NS_ENSURE_SUCCESS(rv,rv);

  // get the old host (and possibly port)
  PRInt32 port = -1;
  nsCAutoString hostname(hostAndPort);
  PRInt32 colonPos = hostname.FindChar(':');
  if (colonPos != -1) {
	nsCAutoString portStr(hostAndPort + colonPos);
	hostname.Truncate(colonPos);
	PRInt32 err;
    port = portStr.ToInteger(&err);
	NS_ASSERTION(err == 0, "failed to get the port\n");
    if (err != 0)
      port = -1;
  }


  //
  // create the server
  //
  nsCOMPtr<nsIMsgIncomingServer> server;
  rv = accountManager->CreateIncomingServer(username.get(), hostname.get(), "imap",
                            getter_AddRefs(server));
  NS_ENSURE_SUCCESS(rv,rv);

  if (port > 0) {
    rv = server->SetPort(port);
    NS_ENSURE_SUCCESS(rv,rv);
  }
  else {
    if (isSecure) {
      nsCOMPtr <nsIMsgProtocolInfo> protocolInfo = do_GetService("@mozilla.org/messenger/protocol/info;1?type=imap", &rv);
      NS_ENSURE_SUCCESS(rv,rv);

      rv = protocolInfo->GetDefaultServerPort(PR_TRUE, &port);
      NS_ENSURE_SUCCESS(rv,rv);

      rv = server->SetPort(port);
      NS_ENSURE_SUCCESS(rv,rv);
    }
  }

  rv = server->SetIsSecure(isSecure);
  NS_ENSURE_SUCCESS(rv,rv);

  // Generate unique pretty name for the account. It is important that this function
  // is called here after all port settings are taken care of.
  // Port values, if not default, will be used as a part of pretty name.
  nsXPIDLString prettyName;
  rv = server->GeneratePrettyNameForMigration(getter_Copies(prettyName));  
  NS_ENSURE_SUCCESS(rv,rv);

  // Set pretty name for the account with this server
  if (prettyName.get())
  {
    rv = server->SetPrettyName(prettyName);
    NS_ENSURE_SUCCESS(rv,rv);
  }

#ifdef DEBUG_MIGRATOR
  PRInt32 portValue;
  rv = server->GetPort(&portValue);
  printf("HOSTNAME = %s\n", hostname.get());
  printf("PORT = %d\n", portValue);
#endif /* DEBUG_MIGRATOR */

  // now upgrade all the prefs

  rv = MigrateOldImapPrefs(server, hostAndPort);
  if (NS_FAILED(rv)) return rv;

  nsCOMPtr <nsIFile> imapMailDir;
  nsFileSpec dir;
  PRBool dirExists;

  // if they used -installer, this pref will point to where their files got copied
  // if the "mail.imap.root_dir" pref is set, use that.
  nsCOMPtr<nsILocalFile> localFile;
  rv = m_prefs->GetComplexValue(PREF_IMAP_DIRECTORY, NS_GET_IID(nsILocalFile), getter_AddRefs(localFile));
  if (NS_SUCCEEDED(rv))
    imapMailDir = localFile;
  
  if (!imapMailDir) {
    // we want <profile>/ImapMail
    rv = NS_GetSpecialDirectory(NS_APP_IMAP_MAIL_50_DIR, getter_AddRefs(imapMailDir));
    if (NS_FAILED(rv)) return rv;
  }

  rv = imapMailDir->Exists(&dirExists);
  if (NS_SUCCEEDED(rv) && !dirExists)
    rv = imapMailDir->Create(nsIFile::DIRECTORY_TYPE, 0775);
  if (NS_FAILED(rv)) return rv;
  
  nsCOMPtr<nsIFileSpec> imapMailDirSpec;

  // Convert the nsILocalFile into an nsIFileSpec
  // TODO: convert users os nsIFileSpec to nsILocalFile
  // and avoid this step.
  rv = NS_NewFileSpecFromIFile(imapMailDir, getter_AddRefs(imapMailDirSpec));
  if (NS_FAILED(rv)) return rv;

  // we only need to do this once
  if (!m_alreadySetImapDefaultLocalPath) {
    // set the default local path for "imap"
    rv = server->SetDefaultLocalPath(imapMailDirSpec);
    if (NS_FAILED(rv)) return rv;

    m_alreadySetImapDefaultLocalPath = PR_TRUE;
  }
  
  // we want .../ImapMail/<hostname>, not .../ImapMail
  rv = imapMailDirSpec->AppendRelativeUnixPath(hostname.get());
  if (NS_FAILED(rv)) return rv;

  // set the local path for this "imap" server
  rv = server->SetLocalPath(imapMailDirSpec);
  if (NS_FAILED(rv)) return rv;
   
  rv = imapMailDirSpec->Exists(&dirExists);
  if (!dirExists) {
    imapMailDirSpec->CreateDir();
  }
  
  // create the identity
  nsCOMPtr<nsIMsgIdentity> copied_identity;
  rv = accountManager->CreateIdentity(getter_AddRefs(copied_identity));
  if (NS_FAILED(rv)) return rv;

  // Create an account when valid server and identity values are established.
  // This will keep the status of accounts sane by avoiding the addition of incomplete accounts. 
  nsCOMPtr<nsIMsgAccount> account;
  rv = accountManager->CreateAccount(getter_AddRefs(account));
  if (NS_FAILED(rv)) return rv;

  // hook the server to the account 
  // before setting the copies and folder prefs
  // (see bug #31904)
  // but after we set the server's local path
  // (see bug #66018)
  account->SetIncomingServer(server);
  account->AddIdentity(copied_identity);

  // make this new identity a copy of the identity
  // that we created out of the 4.x prefs
  rv = copied_identity->Copy(identity);
  if (NS_FAILED(rv)) return rv;
  
  rv = SetMailCopiesAndFolders(copied_identity, username.get(), hostname.get());
  if (NS_FAILED(rv)) return rv;  
  
 
  if (isDefaultAccount) {
    rv = accountManager->SetDefaultAccount(account);
    if (NS_FAILED(rv)) return rv;

    // Set check for new mail option for default account to TRUE 
    rv = server->SetLoginAtStartUp(PR_TRUE);
  }

  return NS_OK;
}

nsresult
nsMessengerMigrator::MigrateOldImapPrefs(nsIMsgIncomingServer *server, const char *hostAndPort)
{
  nsresult rv;

  // some of this ought to be moved out into the IMAP implementation
  nsCOMPtr<nsIImapIncomingServer> imapServer;
  imapServer = do_QueryInterface(server, &rv);
  if (NS_FAILED(rv)) return rv;

  // upgrade the msg incoming server prefs
  // don't migrate the remember password pref.  see bug #42216 
  //MIGRATE_BOOL_PREF("mail.imap.server.%s.remember_password",hostAndPort,server,SetRememberPassword)
  rv = server->SetRememberPassword(PR_FALSE);
  if (NS_FAILED(rv)) return rv;

  rv = server->SetPassword(nsnull);
  if (NS_FAILED(rv)) return rv;

  // upgrade the imap incoming server specific prefs
  MIGRATE_BOOL_PREF("mail.imap.server.%s.check_new_mail",hostAndPort,server,SetDoBiff)
  MIGRATE_INT_PREF("mail.imap.server.%s.check_time",hostAndPort,server,SetBiffMinutes)
  // "mail.imap.new_mail_get_headers" was a global pref across all imap servers in 4.x
  // in 5.0, it's per server
  MIGRATE_BOOL_PREF("%s","mail.imap.new_mail_get_headers",server,SetDownloadOnBiff)
  MIGRATE_STR_PREF("mail.imap.server.%s.admin_url",hostAndPort,imapServer,SetAdminUrl)
  MIGRATE_STR_PREF("mail.imap.server.%s.server_sub_directory",hostAndPort,imapServer,SetServerDirectory);
  MIGRATE_INT_PREF("mail.imap.server.%s.capability",hostAndPort,imapServer,SetCapabilityPref)
  MIGRATE_BOOL_PREF("mail.imap.server.%s.cleanup_inbox_on_exit",hostAndPort,imapServer,SetCleanupInboxOnExit)
  MIGRATE_INT_PREF("mail.imap.server.%s.delete_model",hostAndPort,imapServer,SetDeleteModel)
  MIGRATE_BOOL_PREF("mail.imap.server.%s.dual_use_folders",hostAndPort,imapServer,SetDualUseFolders)
  MIGRATE_BOOL_PREF("mail.imap.server.%s.empty_trash_on_exit",hostAndPort,server,SetEmptyTrashOnExit)
  MIGRATE_INT_PREF("mail.imap.server.%s.empty_trash_threshhold",hostAndPort,imapServer,SetEmptyTrashThreshhold)
  MIGRATE_STR_PREF("mail.imap.server.%s.namespace.other_users",hostAndPort,imapServer,SetOtherUsersNamespace)
  MIGRATE_STR_PREF("mail.imap.server.%s.namespace.personal",hostAndPort,imapServer,SetPersonalNamespace)
  MIGRATE_STR_PREF("mail.imap.server.%s.namespace.public",hostAndPort,imapServer,SetPublicNamespace)
  MIGRATE_BOOL_PREF("mail.imap.server.%s.offline_download",hostAndPort,imapServer,SetOfflineDownload)
  MIGRATE_BOOL_PREF("mail.imap.server.%s.override_namespaces",hostAndPort,imapServer,SetOverrideNamespaces)
  MIGRATE_BOOL_PREF("mail.imap.server.%s.using_subscription",hostAndPort,imapServer,SetUsingSubscription)

  return NS_OK;
}

nsresult 
nsMessengerMigrator::MigrateAddressBookPrefs()
{
  nsresult rv;
  PRBool autoCompleteAgainstLocalAddressbooks;

  rv = m_prefs->GetBoolPref(PREF_4X_AUTOCOMPLETE_ON_LOCAL_AB, &autoCompleteAgainstLocalAddressbooks);

  if (NS_SUCCEEDED(rv)) {
    rv = m_prefs->SetBoolPref(PREF_MOZILLA_AUTOCOMPLETE_ON_LOCAL_AB, autoCompleteAgainstLocalAddressbooks);
    NS_ENSURE_SUCCESS(rv,rv);
  }
  return NS_OK;
}

#ifdef USE_NEWSRC_MAP_FILE
#define NEWSRC_MAP_FILE_COOKIE "netscape-newsrc-map-file"
#endif /* USE_NEWSRC_MAP_FILE */

nsresult
nsMessengerMigrator::MigrateNewsAccounts(nsIMsgIdentity *identity)
{
    nsresult rv;
    nsCOMPtr <nsIFile> newsDir;
    nsFileSpec newsrcDir; // the directory that holds the newsrc files (and the fat file, if we are using one)
    nsFileSpec newsHostsDir; // the directory that holds the host directory, and the summary files.
    // the host directories will be under <profile dir>/News or
    // <"news.directory" pref>/News.
    //
    // the newsrc file don't necessarily live under there.
    //
    // if we didn't use the map file (UNIX) then we want to force the newsHostsDir to be
    // <profile>/News.  in 4.x, on UNIX, the summary files lived in ~/.netscape/xover-cache/<hostname>
    // in 5.0, they will live under <profile>/News/<hostname>, like the other platforms.
    // in 4.x, on UNIX, the "news.directory" pref pointed to the directory to where
    // the newsrc files lived.  we don't want that for the newsHostsDir.
#ifdef USE_NEWSRC_MAP_FILE
    // if they used -installer, this pref will point to where their files got copied
    nsCOMPtr<nsILocalFile> localFile;
    rv = m_prefs->GetComplexValue(PREF_NEWS_DIRECTORY, NS_GET_IID(nsILocalFile), getter_AddRefs(localFile));
    if (NS_SUCCEEDED(rv))
        newsDir = localFile;
#else
    rv = NS_ERROR_FAILURE;
#endif /* USE_NEWSRC_MAP_FILE */

    if (!newsDir) {	    
	    rv = NS_GetSpecialDirectory(NS_APP_NEWS_50_DIR, getter_AddRefs(newsDir));
	    if (NS_FAILED(rv)) return rv;
    }
 
    PRBool dirExists;
    rv = newsDir->Exists(&dirExists);
    if (NS_SUCCEEDED(rv) && !dirExists)
	    newsDir->Create(nsIFile::DIRECTORY_TYPE, 0775);
    if (NS_FAILED(rv)) return rv;
    
    // TODO: convert users os nsIFileSpec to nsILocalFile
    // and avoid this step.
    nsCAutoString pathBuf;
    rv = newsDir->GetNativePath(pathBuf);
    if (NS_FAILED(rv)) return rv;
    newsHostsDir = pathBuf.get();    
    
#ifdef USE_NEWSRC_MAP_FILE	
    // if we are using the fat file, it lives in the newsHostsDir.
    newsrcDir = newsHostsDir;

    // the fat file is really <newsrcDir>/<fat file name>
    // example:  News\fat on Windows, News\NewsFAT on the Mac
    // don't worry, after we migrate, the fat file is going away
    nsFileSpec fatFile(newsrcDir);
	fatFile += NEWS_FAT_FILE_NAME;

    // for each news server in the fat file call MigrateNewsAccount();
	char buffer[512];
	char pseudo_name[512];
	char filename[512];
	char is_newsgroup[512];
	PRBool ok;

	// check if the fat file exists
	// if not, return and handle it gracefully
	if (!fatFile.Exists()) {
		return NS_OK;
	}

	nsInputFileStream inputStream(fatFile);
	
	// if it exists, but it is empty, just return and handle it gracefully
	if (inputStream.eof()) {
		inputStream.close();
		return NS_OK;
	}
	
    /* we expect the first line to be NEWSRC_MAP_FILE_COOKIE */
	ok = inputStream.readline(buffer, sizeof(buffer));
	
    if ((!ok) || (PL_strncmp(buffer, NEWSRC_MAP_FILE_COOKIE, PL_strlen(NEWSRC_MAP_FILE_COOKIE)))) {
		inputStream.close();
		return NS_ERROR_FAILURE;
	}   
	
	while (!inputStream.eof()) {
		char * p;
		PRInt32 i;
		
		ok = inputStream.readline(buffer, sizeof(buffer));
		if (!ok) {
			inputStream.close();
			return NS_ERROR_FAILURE;
		}  
		
		/* TODO: replace this with nsString code? */

		/*
		This used to be scanf() call which would incorrectly
		parse long filenames with spaces in them.  - JRE
		*/
		
		filename[0] = '\0';
		is_newsgroup[0]='\0';
		
		for (i = 0, p = buffer; *p && *p != '\t' && i < 500; p++, i++)
			pseudo_name[i] = *p;
		pseudo_name[i] = '\0';
		if (*p) 
		{
			for (i = 0, p++; *p && *p != '\t' && i < 500; p++, i++)
				filename[i] = *p;
			filename[i]='\0';
			if (*p) 
			{
				for (i = 0, p++; *p && *p != '\r' && i < 500; p++, i++)
					is_newsgroup[i] = *p;
				is_newsgroup[i]='\0';
			}
		}
		
		if(PL_strncmp(is_newsgroup, "TRUE", 4)) {
#ifdef NEWS_FAT_STORES_ABSOLUTE_NEWSRC_FILE_PATHS
			// most likely, the fat file has been copied (or moved ) from
			// its old location.  So the absolute file paths will be wrong.
			// all we care about is the leaf, so use that.
			nsFileSpec oldRcFile(filename);
			char *leaf = oldRcFile.GetLeafName();

			nsFileSpec rcFile(newsrcDir);
			rcFile += leaf;
			nsCRT::free(leaf);
			leaf = nsnull;
#else
			nsFileSpec rcFile(newsrcDir);
			rcFile += filename;
#endif /* NEWS_FAT_STORES_ABSOLUTE_NEWSRC_FILE_PATHS */

			// pseudo-name is of the form newsrc-<host> or snewsrc-<host>.  
			if (PL_strncmp(PSEUDO_NAME_PREFIX,pseudo_name,PL_strlen(PSEUDO_NAME_PREFIX)) == 0) {
                // check that there is a hostname to get after the "newsrc-" part
                NS_ASSERTION(PL_strlen(pseudo_name) > PL_strlen(PSEUDO_NAME_PREFIX), "pseudo_name is too short");
                if (PL_strlen(pseudo_name) <= PL_strlen(PSEUDO_NAME_PREFIX)) {
                    return NS_ERROR_FAILURE;
                }

                char *hostname = pseudo_name + PL_strlen(PSEUDO_NAME_PREFIX);
                rv = MigrateNewsAccount(identity, hostname, rcFile, newsHostsDir, PR_FALSE /* isSecure */);
                if (NS_FAILED(rv)) {
                    // failed to migrate.  bail out
                    return rv;
                }
			}
            else if (PL_strncmp(PSEUDO_SECURE_NAME_PREFIX,pseudo_name,PL_strlen(PSEUDO_SECURE_NAME_PREFIX)) == 0) {
                // check that there is a hostname to get after the "snewsrc-" part
                NS_ASSERTION(PL_strlen(pseudo_name) > PL_strlen(PSEUDO_SECURE_NAME_PREFIX), "pseudo_name is too short");
                if (PL_strlen(pseudo_name) <= PL_strlen(PSEUDO_SECURE_NAME_PREFIX)) {
                    return NS_ERROR_FAILURE;
                }

                char *hostname = pseudo_name + PL_strlen(PSEUDO_SECURE_NAME_PREFIX);
                rv = MigrateNewsAccount(identity, hostname, rcFile, newsHostsDir, PR_TRUE /* isSecure */);
                if (NS_FAILED(rv)) {
                    // failed to migrate.  bail out
                    return rv;
                }
            }
            else {
                continue;
            }
		}
	}
	
	inputStream.close();
#else /* USE_NEWSRC_MAP_FILE */
    nsCOMPtr<nsILocalFile> prefLocal;
    rv = m_prefs->GetComplexValue(PREF_NEWS_DIRECTORY, NS_GET_IID(nsILocalFile), getter_AddRefs(prefLocal));
    if (NS_FAILED(rv)) return rv;
    newsDir = prefLocal;
    
    {
        nsCAutoString pathBuf;
        newsDir->GetNativePath(pathBuf);
        if (NS_FAILED(rv)) return rv;
        newsrcDir = pathBuf.get();
    }

    for (nsDirectoryIterator i(newsrcDir, PR_FALSE); i.Exists(); i++) {
      nsFileSpec possibleRcFile = i.Spec();

      char *filename = possibleRcFile.GetLeafName();
      
      if ((PL_strncmp(NEWSRC_FILE_PREFIX_IN_5x, filename, PL_strlen(NEWSRC_FILE_PREFIX_IN_5x)) == 0) && (PL_strlen(filename) > PL_strlen(NEWSRC_FILE_PREFIX_IN_5x))) {
#ifdef DEBUG_MIGRATOR
        printf("found a newsrc file: %s\n", filename);
#endif
        char *hostname = filename + PL_strlen(NEWSRC_FILE_PREFIX_IN_5x);
        rv = MigrateNewsAccount(identity, hostname, possibleRcFile, newsHostsDir, PR_FALSE /* isSecure */);
        if (NS_FAILED(rv)) {
          // failed to migrate.  bail out
          nsCRT::free(filename);
          return rv;
        }
      }
      else if ((PL_strncmp(SNEWSRC_FILE_PREFIX_IN_5x, filename, PL_strlen(SNEWSRC_FILE_PREFIX_IN_5x)) == 0) && (PL_strlen(filename) > PL_strlen(SNEWSRC_FILE_PREFIX_IN_5x))) {
#ifdef DEBUG_MIGRATOR
        printf("found a secure newsrc file: %s\n", filename);
#endif
        char *hostname = filename + PL_strlen(SNEWSRC_FILE_PREFIX_IN_5x);
        rv = MigrateNewsAccount(identity, hostname, possibleRcFile, newsHostsDir, PR_TRUE /* isSecure */);
        if (NS_FAILED(rv)) {
          // failed to migrate.  bail out
          nsCRT::free(filename);
          return rv;
        }
      }
      nsCRT::free(filename);
      filename = nsnull;
    }
#endif /* USE_NEWSRC_MAP_FILE */

	return NS_OK;
}

nsresult
nsMessengerMigrator::MigrateNewsAccount(nsIMsgIdentity *identity, const char *hostAndPort, nsFileSpec & newsrcfile, nsFileSpec & newsHostsDir, PRBool isSecure)
{  
	nsresult rv;
    nsCOMPtr<nsIMsgAccountManager> accountManager = 
             do_GetService(NS_MSGACCOUNTMANAGER_CONTRACTID, &rv);
    if (NS_FAILED(rv)) return rv;

	nsFileSpec thisNewsHostsDir = newsHostsDir;
    if (!identity) return NS_ERROR_NULL_POINTER;
	if (!hostAndPort) return NS_ERROR_NULL_POINTER;

    PRInt32 port=-1;
	nsCAutoString hostname(hostAndPort);
	PRInt32 colonPos = hostname.FindChar(':');
	if (colonPos != -1) {
		nsCAutoString portStr(hostAndPort + colonPos);
		hostname.Truncate(colonPos);
		PRInt32 err;
		port = portStr.ToInteger(&err);
		NS_ASSERTION(err == 0, "failed to get the port\n");
        if (err != 0)
          port=-1;
	}

    // create the server
	nsCOMPtr<nsIMsgIncomingServer> server;
    // for news, username is always null
    rv = accountManager->CreateIncomingServer(nsnull /* username */, hostname.get(), "nntp",
                              getter_AddRefs(server));
    NS_ENSURE_SUCCESS(rv,rv);
 
    if (port > 0) {
        rv = server->SetPort(port);
        NS_ENSURE_SUCCESS(rv,rv);
    }
    else {
        if (isSecure) {
            nsCOMPtr <nsIMsgProtocolInfo> protocolInfo = do_GetService("@mozilla.org/messenger/protocol/info;1?type=nntp", &rv);
            NS_ENSURE_SUCCESS(rv,rv);
    
            rv = protocolInfo->GetDefaultServerPort(PR_TRUE, &port);
            NS_ENSURE_SUCCESS(rv,rv);

            rv = server->SetPort(port);
            NS_ENSURE_SUCCESS(rv,rv);
        }
    }

    rv = server->SetIsSecure(isSecure);
    NS_ENSURE_SUCCESS(rv,rv);

#ifdef DEBUG_MIGRATOR
	PRInt32 portValue;
	rv = server->GetPort(&portValue);
	printf("HOSTNAME = %s\n", hostname.get());
	printf("PORT = %d\n", portValue);
#endif /* DEBUG_MIGRATOR */

    // we only need to do this once
    if (!m_alreadySetNntpDefaultLocalPath) {
      nsCOMPtr <nsIFileSpec>nntpRootDir;
      rv = NS_NewFileSpecWithSpec(newsHostsDir, getter_AddRefs(nntpRootDir));
      if (NS_FAILED(rv)) return rv;
      
      // set the default local path for "nntp"
      rv = server->SetDefaultLocalPath(nntpRootDir);
      if (NS_FAILED(rv)) return rv;

      // set the newsrc root for "nntp"
      // we really want <profile>/News or /home/sspitzer/
      // not <profile>/News/news.rc or /home/sspitzer/.newsrc-news
      nsFileSpec newsrcFileDir;
      newsrcfile.GetParent(newsrcFileDir);
      if (NS_FAILED(rv)) return rv;
      
      nsCOMPtr <nsIFileSpec>newsrcRootDir;
      rv = NS_NewFileSpecWithSpec(newsrcFileDir, getter_AddRefs(newsrcRootDir));
      if (NS_FAILED(rv)) return rv;
            
      nsCOMPtr<nsINntpIncomingServer> nntpServer;
      nntpServer = do_QueryInterface(server, &rv);
      if (NS_FAILED(rv)) return rv;
      rv = nntpServer->SetNewsrcRootPath(newsrcRootDir);
      if (NS_FAILED(rv)) return rv;

      m_alreadySetNntpDefaultLocalPath = PR_TRUE;
    }
    
#ifdef DEBUG_MIGRATOR
    printf("migrate old nntp prefs\n");
#endif /* DEBUG_MIGRATOR */

    rv = MigrateOldNntpPrefs(server, hostAndPort, newsrcfile);
    if (NS_FAILED(rv)) return rv;
		
	// can't do dir += "host-"; dir += hostname; 
	// because += on a nsFileSpec inserts a separator
	// so we'd end up with host-/<hostname> and not host-<hostname>
	nsCAutoString alteredHost;
    if (isSecure) {
        alteredHost = "shost-";
    }
    else {
        alteredHost = "host-";
    }

    // XXX hostname is assumed to be ASCII only here. To support IDN,
    // we have to use PRUnichar-version of NS_MsgHashIfNecessary
    // (ref. bug 264071)
	alteredHost += hostAndPort;
	NS_MsgHashIfNecessary(alteredHost);	
	thisNewsHostsDir += alteredHost.get();

    nsCOMPtr <nsIFileSpec> newsDir;
    PRBool dirExists;
	rv = NS_NewFileSpecWithSpec(thisNewsHostsDir, getter_AddRefs(newsDir));
	if (NS_FAILED(rv)) return rv;

#ifdef DEBUG_MIGRATOR
    nsXPIDLCString nativePathStr;
    rv = newsDir->GetUnixStyleFilePath(getter_Copies(nativePathStr));
    if (NS_FAILED(rv)) return rv;

    printf("set the local path for this nntp server to: %s\n",(const char *)nativePathStr);
#endif 
    rv = server->SetLocalPath(newsDir);
    if (NS_FAILED(rv)) return rv;
    
	rv = newsDir->Exists(&dirExists);
	if (!dirExists) {
		newsDir->CreateDir();
	}

  // create the identity
  nsCOMPtr<nsIMsgIdentity> copied_identity;
  rv = accountManager->CreateIdentity(getter_AddRefs(copied_identity));
  if (NS_FAILED(rv)) return rv;

  // Create an account when valid server and identity values are established.
  // This will keep the status of accounts sane by avoiding the addition of incomplete accounts. 
  nsCOMPtr<nsIMsgAccount> account;
  rv = accountManager->CreateAccount(getter_AddRefs(account));
  if (NS_FAILED(rv)) return rv;

  // hook the server to the account 
  // before setting the copies and folder prefs
  // (see bug #31904)
  // but after we set the server's local path
  // (see bug #66018)
  account->SetIncomingServer(server);
  account->AddIdentity(copied_identity);

  // make this new identity a copy of the identity
  // that we created out of the 4.x prefs
  rv = copied_identity->Copy(identity);
  if (NS_FAILED(rv)) return rv;

  rv = SetNewsCopiesAndFolders(copied_identity);
  if (NS_FAILED(rv)) return rv;

	return NS_OK;
}

nsresult
nsMessengerMigrator::MigrateOldNntpPrefs(nsIMsgIncomingServer *server, const char *hostAndPort, nsFileSpec & newsrcfile)
{
  nsresult rv;
  
  // some of this ought to be moved out into the NNTP implementation
  nsCOMPtr<nsINntpIncomingServer> nntpServer;
  nntpServer = do_QueryInterface(server, &rv);
  if (NS_FAILED(rv)) return rv;

  MIGRATE_SIMPLE_BOOL_PREF(PREF_4X_NEWS_NOTIFY_ON,nntpServer,SetNotifyOn)
  MIGRATE_SIMPLE_BOOL_PREF(PREF_4X_NEWS_MARK_OLD_READ,nntpServer,SetMarkOldRead)
  MIGRATE_SIMPLE_INT_PREF(PREF_4X_NEWS_MAX_ARTICLES,nntpServer,SetMaxArticles)
 
  /* in 4.x, news username and passwords did not persist beyond the session
   * so we don't need to call server->SetRememberPassword(PR_FALSE);
   * doing so is also bad since it will call nsNntpIncomingServer::ForgetPassword()
   * which fail since don't have any subfolders (newgroups) yet. 
   */

	
  nsCOMPtr <nsIFileSpec> path;
  rv = NS_NewFileSpecWithSpec(newsrcfile, getter_AddRefs(path));
  if (NS_FAILED(rv)) return rv;

  nntpServer->SetNewsrcFilePath(path);
    
  return NS_OK;
}
