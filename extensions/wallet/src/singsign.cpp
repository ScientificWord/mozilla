/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
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
 *   Pierre Phaneuf <pp@ludusdesign.com>
 *   Mike Calmus
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

#define alphabetize 1

#include "singsign.h"
#include "wallet.h"
#include "nsNetUtil.h"

#ifdef XP_MAC
#include "prpriv.h"             /* for NewNamedMonitor */
#include "prinrval.h"           /* for PR_IntervalNow */
#ifdef APPLE_KEYCHAIN                   /* APPLE */
#include "Keychain.h"                   /* APPLE */
#define kNetscapeProtocolType   'form'  /* APPLE */
#endif                                  /* APPLE */
#else
#include "private/prpriv.h"     /* for NewNamedMonitor */
#endif

#include "nsIPref.h"
#include "nsIServiceManager.h"
#include "nsIIOService.h"
#include "nsIURL.h"
#include "nsIDOMHTMLDocument.h"
#include "prmem.h"
#include "prprf.h"
#include "nsXPIDLString.h"
#include "nsReadableUtils.h"
#include "nsIObserverService.h"
#include "nsIObserver.h"
#include "nsIPromptService2.h"
#include "nsIWindowWatcher.h"
#include "nsIAuthInformation.h"
#include "nsIProxiedChannel.h"
#include "nsIProxyInfo.h"
#include "nsIIDNService.h"
#include "nsNetCID.h"
#include "nsCRT.h"
#include "nsPromptUtils.h"

//#define SINGSIGN_LOGGING
#ifdef SINGSIGN_LOGGING
#define LOG(args) printf args
#else
#define LOG(args)
#endif

// Currently the default is on, so we don't need the code to prompt the
// user if the default is off.
#undef WALLET_PASSWORDMANAGER_DEFAULT_IS_OFF

/********************
 * Global Variables *
 ********************/

/* locks for signon cache */

static PRMonitor * signon_lock_monitor = NULL;
static PRThread  * signon_lock_owner = NULL;
static int signon_lock_count = 0;

/* load states */

static PRBool si_PartiallyLoaded = PR_FALSE;
static PRInt32 si_LastFormForWhichUserHasBeenSelected = -1;

/* apple keychain stuff */

#ifdef APPLE_KEYCHAIN
static PRBool     si_list_invalid = PR_FALSE;
static KCCallbackUPP si_kcUPP = NULL;
static int
si_SaveSignonDataInKeychain();
#endif

#define USERNAMEFIELD "\\=username=\\"
#define PASSWORDFIELD "\\=password=\\"

/******************
 * Key Management *
 ******************/

char* signonFileName = nsnull;
static PRBool gLoadedUserData = PR_FALSE;
static PRUint32 gSelectUserDialogCount = 0;


/***************************
 * Locking the Signon List *
 ***************************/

static void
si_lock_signon_list(void) {
  if(!signon_lock_monitor) {
    signon_lock_monitor = PR_NewNamedMonitor("signon-lock");
  }
  PR_EnterMonitor(signon_lock_monitor);
  while(PR_TRUE) {

    /* no current owner or owned by this thread */
    PRThread * t = PR_GetCurrentThread();
    if(signon_lock_owner == NULL || signon_lock_owner == t) {
      signon_lock_owner = t;
      signon_lock_count++;
      PR_ExitMonitor(signon_lock_monitor);
      return;
    }

    /* owned by someone else -- wait till we can get it */
    PR_Wait(signon_lock_monitor, PR_INTERVAL_NO_TIMEOUT);
  }
}

static void
si_unlock_signon_list(void) {
    PR_EnterMonitor(signon_lock_monitor);

#ifdef DEBUG
    /* make sure someone doesn't try to free a lock they don't own */
    PR_ASSERT(signon_lock_owner == PR_GetCurrentThread());
#endif

    signon_lock_count--;
    if(signon_lock_count == 0) {
        signon_lock_owner = NULL;
        PR_Notify(signon_lock_monitor);
    }
    PR_ExitMonitor(signon_lock_monitor);
}


/********************************
 * Preference Utility Functions *
 ********************************/

void
SI_RegisterCallback(const char* domain, PrefChangedFunc callback, void* instance_data) {
  nsresult ret;
  nsCOMPtr<nsIPref> pPrefService = do_GetService(NS_PREF_CONTRACTID, &ret);
  if (NS_SUCCEEDED(ret)) {
    ret = pPrefService->RegisterCallback(domain, callback, instance_data);
  }
}

void
SI_UnregisterCallback(const char* domain, PrefChangedFunc callback, void* instance_data) {
  nsresult ret;
  nsCOMPtr<nsIPref> pPrefService = do_GetService(NS_PREF_CONTRACTID, &ret);
  if (NS_SUCCEEDED(ret)) {
    ret = pPrefService->UnregisterCallback(domain, callback, instance_data);
  }
}

void
SI_SetBoolPref(const char * prefname, PRBool prefvalue) {
  nsresult ret;
  nsCOMPtr<nsIPref> pPrefService = do_GetService(NS_PREF_CONTRACTID, &ret);
  if (NS_SUCCEEDED(ret)) {
    ret = pPrefService->SetBoolPref(prefname, prefvalue);
    if (NS_SUCCEEDED(ret)) {
      ret = pPrefService->SavePrefFile(nsnull); 
    }
  }
}

PRBool
SI_GetBoolPref(const char * prefname, PRBool defaultvalue) {
  nsresult ret;
  PRBool prefvalue = defaultvalue;
  nsCOMPtr<nsIPref> pPrefService = do_GetService(NS_PREF_CONTRACTID, &ret);
  if (NS_SUCCEEDED(ret)) {
    ret = pPrefService->GetBoolPref(prefname, &prefvalue);
  }
  return prefvalue;
}

void
SI_SetCharPref(const char * prefname, const char * prefvalue) {
  if (!prefvalue) {
    return; /* otherwise the SetCharPref routine called below will crash */
  }
  nsresult ret;
  nsCOMPtr<nsIPref> pPrefService = do_GetService(NS_PREF_CONTRACTID, &ret);
  if (NS_SUCCEEDED(ret)) {
    ret = pPrefService->SetCharPref(prefname, prefvalue);
    if (NS_SUCCEEDED(ret)) {
      ret = pPrefService->SavePrefFile(nsnull); 
    }
  }
}

void
SI_GetCharPref(const char * prefname, char** aPrefvalue) {
  nsresult ret;
  nsCOMPtr<nsIPref> pPrefService = do_GetService(NS_PREF_CONTRACTID, &ret);
  if (NS_SUCCEEDED(ret)) {
    ret = pPrefService->CopyCharPref(prefname, aPrefvalue);
    if (NS_FAILED(ret)) {
      *aPrefvalue = nsnull;
    }
  } else {
    *aPrefvalue = nsnull;
  }
}

void
SI_GetLocalizedUnicharPref(const char * prefname, PRUnichar** aPrefvalue) {
  nsresult ret;
  nsCOMPtr<nsIPref> pPrefService = do_GetService(NS_PREF_CONTRACTID, &ret);
  if (NS_SUCCEEDED(ret)) {
    ret = pPrefService->GetLocalizedUnicharPref(prefname, aPrefvalue);
    if (NS_FAILED(ret)) {
      *aPrefvalue = nsnull;
    }
  } else {
      *aPrefvalue = nsnull;
  }
}


/*********************************
 * Preferences for Single Signon *
 *********************************/

static const char *pref_rememberSignons = "signon.rememberSignons";
#ifdef WALLET_PASSWORDMANAGER_DEFAULT_IS_OFF
static const char *pref_Notified = "signon.Notified";
#endif
static const char *pref_SignonFileName = "signon.SignonFileName";

static PRBool si_RememberSignons = PR_FALSE;
#ifdef WALLET_PASSWORDMANAGER_DEFAULT_IS_OFF
static PRBool si_Notified = PR_FALSE;
#endif

static int
si_SaveSignonDataLocked(char * state, PRBool notify);

int
SI_LoadSignonData();

void
SI_RemoveAllSignonData();

#ifdef WALLET_PASSWORDMANAGER_DEFAULT_IS_OFF
static PRBool
si_GetNotificationPref(void) {
  return si_Notified;
}

static void
si_SetNotificationPref(PRBool x) {
  SI_SetBoolPref(pref_Notified, x);
  si_Notified = x;
}
#endif

static void
si_SetSignonRememberingPref(PRBool x) {
#ifdef APPLE_KEYCHAIN
  if (x == 0) {
    /* We no longer need the Keychain callback installed */
    KCRemoveCallback( si_kcUPP );
    DisposeRoutineDescriptor( si_kcUPP );
    si_kcUPP = NULL;
  }
#endif
  si_RememberSignons = x;
}

int PR_CALLBACK
si_SignonRememberingPrefChanged(const char * newpref, void * data) {
    PRBool x;
    x = SI_GetBoolPref(pref_rememberSignons, PR_TRUE);
    si_SetSignonRememberingPref(x);
    return 0; /* this is PREF_NOERROR but we no longer include prefapi.h */
}

static void
si_RegisterSignonPrefCallbacks(void) {
  PRBool x;
  static PRBool first_time = PR_TRUE;
  if(first_time) {
    first_time = PR_FALSE;
    SI_RegisterCallback(pref_rememberSignons, si_SignonRememberingPrefChanged, NULL);
  }
  
  if (!gLoadedUserData) {
    gLoadedUserData = PR_TRUE;
    SI_LoadSignonData();
#ifdef WALLET_PASSWORDMANAGER_DEFAULT_IS_OFF
    x = SI_GetBoolPref(pref_Notified, PR_FALSE);
    si_SetNotificationPref(x);        
#endif
    x = SI_GetBoolPref(pref_rememberSignons, PR_FALSE);
    si_SetSignonRememberingPref(x);
  }
}

static PRBool
si_GetSignonRememberingPref(void) {
#ifdef APPLE_KEYCHAIN
  /* If the Keychain has been locked or an item deleted or updated,
   * we need to reload the signon data
   */
  if (si_list_invalid) {
    /*
     * set si_list_invalid to PR_FALSE first because SI_RemoveAllSignonData
     * calls si_GetSignonRememberingPref
     */
    si_list_invalid = PR_FALSE;
    SI_LoadSignonData();
  }
#endif

  si_RegisterSignonPrefCallbacks();

#ifdef WALLET_PASSWORDMANAGER_DEFAULT_IS_OFF
  /*
   * We initially want the rememberSignons pref to be PR_FALSE.  But this will
   * prevent the notification message from ever occurring.  To get around
   * this problem, if the signon pref is PR_FALSE and no notification has
   * ever been given, we will treat this as if the signon pref were PR_TRUE.
   */
  if (!si_RememberSignons && !si_GetNotificationPref()) {
    return PR_TRUE;
  } else {
    return si_RememberSignons;
  }
#else
  return si_RememberSignons;
#endif
}

void
SI_InitSignonFileName() {
  SI_GetCharPref(pref_SignonFileName, &signonFileName);
  if (!signonFileName) {
    signonFileName = Wallet_RandomName("s");
    SI_SetCharPref(pref_SignonFileName, signonFileName);
  }
}


/***********
 * Dialogs *
 ***********/

#ifdef WALLET_PASSWORDMANAGER_DEFAULT_IS_OFF
static PRBool
si_ConfirmYN(PRUnichar * szMessage, nsIDOMWindowInternal* window) {
  return Wallet_ConfirmYN(szMessage, window);
}
#endif

static PRInt32
si_3ButtonConfirm(PRUnichar * szMessage, nsIDOMWindowInternal* window) {
  return Wallet_3ButtonConfirm(szMessage, window);
}

static PRBool
si_SelectDialog(const PRUnichar* szMessage, nsIPrompt* dialog, PRUnichar** pList, PRInt32* pCount, PRUint32 formNumber) {
  if (si_LastFormForWhichUserHasBeenSelected == (PRInt32)formNumber) {
    /* a user was already selected for this form, use same one again */
    *pCount = 0; /* last user selected is now at head of list */
    return PR_TRUE;
  }
  nsresult rv;
  PRInt32 selectedIndex;
  PRBool rtnValue;
  PRUnichar * title_string = Wallet_Localize("SelectUserTitleLine");

  /* Notify signon manager dialog to update its display */
  nsCOMPtr<nsIObserverService> os(do_GetService("@mozilla.org/observer-service;1"));
  gSelectUserDialogCount++;
  if (os) {
    os->NotifyObservers(nsnull, "signonSelectUser", NS_LITERAL_STRING("suspend").get());
  }

  rv = dialog->Select( title_string, szMessage, *pCount, const_cast<const PRUnichar**>(pList), &selectedIndex, &rtnValue );

  gSelectUserDialogCount--;
  if (os) {
    os->NotifyObservers(nsnull, "signonSelectUser", NS_LITERAL_STRING("resume").get());
  }

  Recycle(title_string);
  if (selectedIndex >= *pCount) {
    return PR_FALSE; // out-of-range selection
  }
  *pCount = selectedIndex;
  if (rtnValue) {
    si_LastFormForWhichUserHasBeenSelected = formNumber;
  }
  return rtnValue;  
}

static nsresult
si_CheckGetPassword
  (PRUnichar ** password,
   const PRUnichar* dialogTitle,
   const PRUnichar * szMessage,
   nsIPrompt* dialog,
   PRUint32 savePassword,
   PRBool* checkValue)
{
  nsresult res;

  PRUnichar * prompt_string = (PRUnichar*)dialogTitle;
  if (dialogTitle == nsnull || dialogTitle[0] == 0)
    prompt_string = Wallet_Localize("PromptForPassword");
  PRUnichar * check_string;
  
  // According to nsIPrompt spec, the checkbox is shown or not
  // depending on whether or not checkValue == nsnull, not checkMsg.
  PRBool * check_value = checkValue;
  if (savePassword != SINGSIGN_SAVE_PASSWORD_PERMANENTLY) {
    check_string = nsnull;
    check_value = nsnull;
  } else if (SI_GetBoolPref(pref_Crypto, PR_FALSE)) {
    check_string = Wallet_Localize("SaveThisPasswordEncrypted");
  } else {
    check_string = Wallet_Localize("SaveThisPasswordObscured");
  }

  PRBool confirmed = PR_FALSE;  
  res = dialog->PromptPassword(prompt_string,
                               szMessage,
                               password,
                               check_string,
                               check_value,
                               &confirmed);

  if (dialogTitle == nsnull)
    Recycle(prompt_string);
  if (check_string)
    Recycle(check_string);

  if (NS_FAILED(res)) {
    return res;
  }
  if (confirmed) {
    return NS_OK;
  } else {
    return NS_ERROR_FAILURE; /* user pressed cancel */
  }
}

static nsresult
si_CheckGetData
  (PRUnichar ** data,
   const PRUnichar* dialogTitle,
   const PRUnichar * szMessage,
   nsIPrompt* dialog,
   PRUint32 savePassword,
   PRBool* checkValue)
{
  nsresult res;  

  PRUnichar * prompt_string = (PRUnichar*)dialogTitle;
  if (dialogTitle == nsnull || dialogTitle[0] == 0)
    prompt_string = Wallet_Localize("PromptForData");
  PRUnichar * check_string;

  // According to nsIPrompt spec, the checkbox is shown or not
  // depending on whether or not checkValue == nsnull, not checkMsg.
  PRBool * check_value = checkValue;
  if (savePassword != SINGSIGN_SAVE_PASSWORD_PERMANENTLY) {
    check_string = nsnull;
    check_value = nsnull;
  } else if (SI_GetBoolPref(pref_Crypto, PR_FALSE)) {
    check_string = Wallet_Localize("SaveThisValueEncrypted");
  } else {
    check_string = Wallet_Localize("SaveThisValueObscured");
  }

  PRBool confirmed = PR_FALSE;  
  res = dialog->Prompt(prompt_string,
                       szMessage,
                       data,
                       check_string,
                       check_value,
                       &confirmed);

  if (dialogTitle == nsnull || dialogTitle[0] == 0)
    Recycle(prompt_string);
  if (check_string)
    Recycle(check_string);

  if (NS_FAILED(res)) {
    return res;
  }
  if (confirmed) {
    return NS_OK;
  } else {
    return NS_ERROR_FAILURE; /* user pressed cancel */
  }
}

static nsresult
si_CheckGetUsernamePassword
  (PRUnichar ** username,
   PRUnichar ** password,
   const PRUnichar* dialogTitle,
   const PRUnichar * szMessage,
   nsIPrompt* dialog,
   PRUint32 savePassword,
   PRBool* checkValue)
{
  nsresult res;  
  PRUnichar * check_string;
  PRUnichar * prompt_string = (PRUnichar*)dialogTitle;
  if (dialogTitle == nsnull || dialogTitle[0] == 0)
    prompt_string = Wallet_Localize("PromptForPassword");
  
  // According to nsIPrompt spec, the checkbox is shown or not
  // depending on whether or not checkValue == nsnull, not checkMsg.
  PRBool * check_value = checkValue;
  if (savePassword != SINGSIGN_SAVE_PASSWORD_PERMANENTLY) {
    check_string = nsnull;
    check_value = nsnull;
  } else if (SI_GetBoolPref(pref_Crypto, PR_FALSE)) {
    check_string = Wallet_Localize("SaveTheseValuesEncrypted");
  } else {
    check_string = Wallet_Localize("SaveTheseValuesObscured");
  }

  PRBool confirmed = PR_FALSE;  
  res = dialog->PromptUsernameAndPassword(prompt_string,
                                          szMessage,
                                          username, password,
                                          check_string,
                                          check_value,
                                          &confirmed);

  if (dialogTitle == nsnull || dialogTitle[0] == 0)
    Recycle(prompt_string);
  if (check_string)
    Recycle(check_string);
  
  if (NS_FAILED(res)) {
    return res;
  }
  if (confirmed) {
    return NS_OK;
  } else {
    return NS_ERROR_FAILURE; /* user pressed cancel */
  }
}

static nsresult
si_CheckPromptAuth
  (nsIPromptService2* aService, nsIDOMWindow* aParent, nsIChannel* aChannel,
   PRUint32 aLevel, nsIAuthInformation* aAuthInfo, PRBool* remembered)
{
  PRUnichar* check_string;
  if (SI_GetBoolPref(pref_Crypto, PR_FALSE)) {
    check_string = Wallet_Localize("SaveTheseValuesEncrypted");
  } else {
    check_string = Wallet_Localize("SaveTheseValuesObscured");
  }

  PRBool confirmed = PR_FALSE;  
  nsresult rv = aService->PromptAuth(aParent, aChannel, aLevel,
                                     aAuthInfo, check_string,
                                     remembered, &confirmed);
  if (check_string)
    Recycle(check_string);
  
  if (NS_FAILED(rv))
    return rv;

  if (confirmed) {
    return NS_OK;
  } else {
    return NS_ERROR_FAILURE; /* user pressed cancel */
  }
}



/********************
 * Utility Routines *
 ********************/

/* StrAllocCopy should really be defined elsewhere */
#include "plstr.h"
#include "prmem.h"

#undef StrAllocCopy
#define StrAllocCopy(dest, src) Local_SACopy (&(dest), src)
static char *
Local_SACopy(char **destination, const char *source) {
  if(*destination) {
    PL_strfree(*destination);
  }
  *destination = PL_strdup(source);
  return *destination;
}

#ifdef WALLET_PASSWORDMANAGER_DEFAULT_IS_OFF
/* If user-entered password is "********", then generate a random password */
static void
si_Randomize(nsString& password) {
  PRIntervalTime randomNumber;
  int i;
  const char * hexDigits = "0123456789AbCdEf";
  if (password.EqualsLiteral("********")) {
    randomNumber = PR_IntervalNow();
    for (i=0; i<8; i++) {
      password.SetCharAt(hexDigits[randomNumber%16], i);
      randomNumber = randomNumber/16;
    }
  }
}
#endif


/***********************
 * Encryption Routines *
 ***********************/

static PRBool
si_CompareEncryptedToCleartext(const nsString& crypt, const nsString& text) {
  nsAutoString decrypted;
  if (NS_FAILED(Wallet_Decrypt(crypt, decrypted))) {
    return PR_FALSE;
  }
  return (decrypted == text);
}

static PRBool
si_CompareEncryptedToEncrypted(const nsString& crypt1, const nsString& crypt2) {
  nsAutoString decrypted1;
  nsAutoString decrypted2;
  if (NS_FAILED(Wallet_Decrypt(crypt1, decrypted1))) {
    return PR_FALSE;
  }
  if (NS_FAILED(Wallet_Decrypt(crypt2, decrypted2))) {
    return PR_FALSE;
  }
  return (decrypted1 == decrypted2);
}


/************************
 * Managing Signon List *
 ************************/

static PRUint32
SecondsFromPRTime(PRTime prTime) {
  PRInt64 microSecondsPerSecond, intermediateResult;
  PRUint32 seconds;
  
  LL_I2L(microSecondsPerSecond, PR_USEC_PER_SEC);
  LL_DIV(intermediateResult, prTime, microSecondsPerSecond);
  LL_L2UI(seconds, intermediateResult);
  return seconds;
}

si_SignonDataStruct::si_SignonDataStruct()
  : isPassword(PR_FALSE)
{
  MOZ_COUNT_CTOR(si_SignonDataStruct);
}
si_SignonDataStruct::~si_SignonDataStruct()
{
  MOZ_COUNT_DTOR(si_SignonDataStruct);
}

class si_SignonUserStruct {
public:
  si_SignonUserStruct()
  {
    MOZ_COUNT_CTOR(si_SignonUserStruct);
  }
  ~si_SignonUserStruct()
  {
    for (PRInt32 i = signonData_list.Count() - 1; i >= 0; i--) {
      delete static_cast<si_SignonDataStruct*>(signonData_list.ElementAt(i));
    }
    MOZ_COUNT_DTOR(si_SignonUserStruct);
  }
  PRUint32 time;
  nsVoidArray signonData_list; // elements are si_SignonDataStruct
};

class si_SignonURLStruct {
public:
  si_SignonURLStruct() : passwordRealm(NULL), chosen_user(NULL)
  {
    MOZ_COUNT_CTOR(si_SignonURLStruct);
  }
  ~si_SignonURLStruct()
  {
    MOZ_COUNT_DTOR(si_SignonURLStruct);
  }
  char * passwordRealm;
  si_SignonUserStruct* chosen_user; /* this is a state variable */
  nsVoidArray signonUser_list;
};

class si_Reject {
public:
  si_Reject() : passwordRealm(NULL)
  {
    MOZ_COUNT_CTOR(si_Reject);
  }
  ~si_Reject()
  {
    MOZ_COUNT_DTOR(si_Reject);
  }
  char * passwordRealm;
  nsString userName;
};

static nsVoidArray * si_signon_list=0;
static nsVoidArray * si_reject_list=0;
#define LIST_COUNT(list) (list ? list->Count() : 0)
static PRBool si_signon_list_changed = PR_FALSE;

/*
 * Get the URL node for a given URL name
 *
 * This routine is called only when holding the signon lock!!!
 */
static si_SignonURLStruct *
si_GetURL(const char * passwordRealm) {
  si_SignonURLStruct * url;
  if (!passwordRealm) {
    /* no passwordRealm specified, return first URL (returns NULL if not URLs) */
    if (LIST_COUNT(si_signon_list)==0) {
      return NULL;
    }
    /* XXX how can this be right -- seems wrong to give back a random password */
    LOG(("  returning first element in the signon list\n"));
    return (si_SignonURLStruct *) (si_signon_list->ElementAt(0));
  }

  PRInt32 urlCount = LIST_COUNT(si_signon_list);
  if (urlCount) {
    // If the last char of passwordRealm is '/' then strip it before making comparison.
    nsCAutoString realmWithoutTrailingSlash(passwordRealm);
    if (!realmWithoutTrailingSlash.IsEmpty() && realmWithoutTrailingSlash.Last() == '/')
      realmWithoutTrailingSlash.Truncate(realmWithoutTrailingSlash.Length()-1);

    for (PRInt32 i=0; i<urlCount; i++) {
      url = static_cast<si_SignonURLStruct*>(si_signon_list->ElementAt(i));
      if(url->passwordRealm && !PL_strcmp(realmWithoutTrailingSlash.get(), url->passwordRealm)) {
        return url;
      }
    }
  }
  return (NULL);
}

/**
 * composite URL struct for handling the migration of legacy password entries.
 */

class si_SignonCompositeURLStruct : public si_SignonURLStruct {
public:
  si_SignonURLStruct *primaryUrl;
  si_SignonURLStruct *legacyUrl;
};

static si_SignonCompositeURLStruct * si_composite_url=0;

#if defined(SINGSIGN_LOGGING)
static void
si_DumpUserList(nsVoidArray &list)
{
  LOG(("dumping user list:\n"));
  PRInt32 i, j, user_count = list.Count(), data_count;
  for (i=0; i<user_count; ++i) {
    si_SignonUserStruct *user = (si_SignonUserStruct *) list[i];
    LOG((" user[%d]\n", i));
    data_count = user->signonData_list.Count();
    for (j=0; j<data_count; ++j) {
      si_SignonDataStruct *data = (si_SignonDataStruct *) user->signonData_list[j];
      LOG(("  (%s,%s)\n",
          NS_ConvertUTF16toUTF8(data->name).get(),
          NS_ConvertUTF16toUTF8(data->value).get()));
    }
  }
}
#endif

static si_SignonURLStruct *
si_GetCompositeURL(const char *primaryRealm, const char *legacyRealm)
{
  si_SignonURLStruct *primaryUrl, *legacyUrl;

  primaryUrl = si_GetURL(primaryRealm);

  if (legacyRealm)
    legacyUrl = si_GetURL(legacyRealm);
  else
    legacyUrl = nsnull;

  if (primaryUrl && legacyUrl) {
    LOG((">>> building composite URL struct\n"));
    if (si_composite_url) {
      NS_ERROR("si_composite_url already in use");
      return NULL;
    }
    si_composite_url = new si_SignonCompositeURLStruct;
    if (!si_composite_url)
      return NULL;

    si_composite_url->primaryUrl = primaryUrl;
    si_composite_url->legacyUrl = legacyUrl;

    si_composite_url->signonUser_list.AppendElements(primaryUrl->signonUser_list);
    si_composite_url->signonUser_list.AppendElements(legacyUrl->signonUser_list);

#if defined(SINGSIGN_LOGGING)
    si_DumpUserList(si_composite_url->signonUser_list);
#endif

    /* need to transfer the chosen_user state variable */
    if (primaryUrl->chosen_user)
      si_composite_url->chosen_user = primaryUrl->chosen_user;
    else if (legacyUrl->chosen_user) {
      si_SignonUserStruct *chosen_user = legacyUrl->chosen_user;
      PRInt32 index;
      /* XXX fixup chosen_user -- THIS SHOULD NOT BE NECESSARY */
      index = legacyUrl->signonUser_list.IndexOf(chosen_user);
      if (index < 0) {
        index = primaryUrl->signonUser_list.IndexOf(chosen_user);
        if (index >= 0)
          primaryUrl->chosen_user = chosen_user;
        legacyUrl->chosen_user = NULL;
      }
      /* move first element of legacy user list to front */
      index = si_composite_url->signonUser_list.IndexOf(chosen_user);
      if (index > 0)
        si_composite_url->signonUser_list.MoveElement(index, 0);
      si_composite_url->chosen_user = chosen_user;
    }
    else
      si_composite_url->chosen_user = NULL;

#if defined(SINGSIGN_LOGGING)
    LOG(("after chosen_user fixup [chosen_user=%x]:\n", si_composite_url->chosen_user));
    si_DumpUserList(si_composite_url->signonUser_list);
#endif

    return si_composite_url;
  }

  if (primaryUrl)
    return primaryUrl;

  return legacyUrl;
}

static PRInt32
si_SetChosenUser(si_SignonURLStruct *url, si_SignonUserStruct *chosen_user)
{
  PRInt32 index;

  index = url->signonUser_list.IndexOf(chosen_user);
  if (index < 0) {
      url->chosen_user = NULL;
      return -1;
  }

  url->chosen_user = chosen_user;
  return index; 
}

static void
si_ReleaseCompositeURL(si_SignonURLStruct *url)
{
  if (url == si_composite_url) {
    si_SignonUserStruct *chosen_user = url->chosen_user;
    /* need to transfer the chosen_user state variable */
    if (chosen_user) {
      PRInt32 index;

      /* store chosen_user */
      index = si_SetChosenUser(url = si_composite_url->primaryUrl, chosen_user);
      if (index >= 0)
        si_composite_url->legacyUrl->chosen_user = NULL;
      else
        index = si_SetChosenUser(url = si_composite_url->legacyUrl, chosen_user);
      NS_ASSERTION(index >= 0, "chosen_user not found");

      /* need to move chosen_user to front of list */
      url->signonUser_list.MoveElement(index, 0);
    }
    else {
      si_composite_url->primaryUrl->chosen_user = NULL;
      si_composite_url->legacyUrl->chosen_user = NULL;
    }
    si_composite_url->primaryUrl = NULL;
    si_composite_url->legacyUrl = NULL;
    si_composite_url->chosen_user = NULL;
    si_composite_url->signonUser_list.Clear();

    delete si_composite_url;
    si_composite_url = NULL;
  }
}

/* Remove a user node from a given URL node */
static PRBool
si_RemoveUser(const char *passwordRealm, const nsString& userName, PRBool save, PRBool loginFailure, PRBool notify, PRBool first = PR_FALSE) {
  si_SignonURLStruct * url;
  si_SignonUserStruct * user;
  si_SignonDataStruct * data;

  si_lock_signon_list();

  /* get URL corresponding to host */
  url = si_GetURL(passwordRealm);
  if (!url) {
    /* URL not found */
    si_unlock_signon_list();
    return PR_FALSE;
  }

  /* free the data in each node of the specified user node for this URL */
  if (first) {

    /* remove the first user */
    user = static_cast<si_SignonUserStruct *>
                      (url->signonUser_list.ElementAt(0));

  } else {

    /* find the specified user */
    PRInt32 userCount = url->signonUser_list.Count();
    for (PRInt32 i=0; i<userCount; i++) {
      user = static_cast<si_SignonUserStruct*>(url->signonUser_list.ElementAt(i));
      PRInt32 dataCount = user->signonData_list.Count();
      for (PRInt32 ii=0; ii<dataCount; ii++) {
        data = static_cast<si_SignonDataStruct*>(user->signonData_list.ElementAt(ii));
        if (si_CompareEncryptedToCleartext(data->value, userName)) {
          goto foundUser;
        }
      }
    }
    si_unlock_signon_list();
    return PR_FALSE; /* user not found so nothing to remove */
    foundUser: ;
  }

  /* free the user node */
  url->signonUser_list.RemoveElement(user);
  delete user;

  /* remove this URL if it contains no more users */
  if (url->signonUser_list.Count() == 0) {
    PR_Free(url->passwordRealm);
    si_signon_list->RemoveElement(url);
    delete url;
  }

  /* write out the change to disk */
  if (save) {
    si_signon_list_changed = PR_TRUE;
    si_SaveSignonDataLocked("signons", notify);
  }

  si_unlock_signon_list();
  return PR_TRUE;
}

nsresult
SINGSIGN_RemoveUser(const char *host, const PRUnichar *user, PRBool notify) {
  PRBool rv = si_RemoveUser(host, nsDependentString(user), PR_TRUE, PR_FALSE, notify);
  return rv ? NS_OK : NS_ERROR_FAILURE;
}

nsresult
SINGSIGN_RemoveUserAfterLoginFailure(const char *host, const PRUnichar *user, PRBool notify) {
  PRBool rv = si_RemoveUser(host, nsDependentString(user), PR_TRUE, PR_TRUE, notify);
  return rv ? NS_OK : NS_ERROR_FAILURE;
}

static void
si_FreeReject(si_Reject * reject);

nsresult
SINGSIGN_RemoveReject(const char *host) {
  si_Reject* reject;
  nsresult rv = NS_ERROR_FAILURE;

  /* step backwards through all rejects */
  si_lock_signon_list();
  PRInt32 rejectCount = LIST_COUNT(si_reject_list);
  while (rejectCount>0) {
    rejectCount--;
    reject = static_cast<si_Reject*>(si_reject_list->ElementAt(rejectCount));
    if (reject && !PL_strcmp(reject->passwordRealm, host)) {
      si_FreeReject(reject);
      si_signon_list_changed = PR_TRUE;
      rv = NS_OK;
    }
  }
  si_SaveSignonDataLocked("rejects", PR_FALSE);
  si_unlock_signon_list();
  return rv;
}

static void
si_PutReject(const char * passwordRealm, const nsString& userName, PRBool save);

nsresult
SINGSIGN_AddReject(const char *host /*, const char *userName*/) {
  si_PutReject(host, nsString(/*thisParameter_isObsolete*/), PR_TRUE);
// @see http://bonsai.mozilla.org/cvsblame.cgi?file=mozilla/extensions/wallet/src/singsign.cpp&rev=1.212&mark=1693#1650
  return NS_OK;
}

/* Determine if a specified url/user exists */
static PRBool
si_CheckForUser(const char *passwordRealm, const nsString& userName) {
  si_SignonURLStruct * url;
  si_SignonUserStruct * user;
  si_SignonDataStruct * data;

  /* do nothing if signon preference is not enabled */
  if (!si_GetSignonRememberingPref()) {
    return PR_FALSE;
  }

  si_lock_signon_list();

  /* get URL corresponding to passwordRealm */
  url = si_GetURL(passwordRealm);
  if (!url) {
    /* URL not found */
    si_unlock_signon_list();
    return PR_FALSE;
  }

  /* find the specified user */
  PRInt32 userCount = url->signonUser_list.Count();
  for (PRInt32 i=0; i<userCount; i++) {
    user = static_cast<si_SignonUserStruct*>(url->signonUser_list.ElementAt(i));
    PRInt32 dataCount = user->signonData_list.Count();
    for (PRInt32 ii=0; ii<dataCount; ii++) {
      data = static_cast<si_SignonDataStruct*>(user->signonData_list.ElementAt(ii));
      if (si_CompareEncryptedToCleartext(data->value, userName)) {
        si_unlock_signon_list();
        return PR_TRUE;
      }
    }
  }
  si_unlock_signon_list();
  return PR_FALSE; /* user not found */
}

/*
 * Get first data node that is not a password
 */

static si_SignonDataStruct *
si_GetFirstNonPasswordData(si_SignonUserStruct* user) {
  PRInt32 dataCount = user->signonData_list.Count();
  for (PRInt32 j=0; j<dataCount; j++) {
    si_SignonDataStruct * data =
      static_cast<si_SignonDataStruct *>(user->signonData_list.ElementAt(j));
    if (!data->isPassword) {
      return data;
    }
  }
  return nsnull;
}

/*
 * Get the user node for a given URL
 *
 * This routine is called only when holding the signon lock!!!
 *
 * This routine is called only if signon pref is enabled!!!
 */
static si_SignonUserStruct*
si_GetUser(nsIPrompt* dialog, const char* passwordRealm, const char *legacyRealm,
           PRBool pickFirstUser, const nsString& userText, PRUint32 formNumber) {
  si_SignonURLStruct* url;
  si_SignonUserStruct* user = nsnull;
  si_SignonDataStruct* data;

  /* get to node for this URL */
  url = si_GetCompositeURL(passwordRealm, legacyRealm);

  if (url != NULL) {

    /* node for this URL was found */
    PRInt32 user_count;
    if ((user_count = url->signonUser_list.Count()) == 1) {

      /* only one set of data exists for this URL so select it */
      user = static_cast<si_SignonUserStruct *>
                        (url->signonUser_list.ElementAt(0));
      url->chosen_user = user;

    } else if (pickFirstUser) {
      PRInt32 userCount = url->signonUser_list.Count();
      for (PRInt32 i=0; i<userCount; i++) {
        user = static_cast<si_SignonUserStruct*>(url->signonUser_list.ElementAt(i));
        /* consider first data node to be the identifying item */
        data = static_cast<si_SignonDataStruct *>
                          (user->signonData_list.ElementAt(0));
        if (data->name != userText) {
          /* name of current data item does not match name in data node */
          continue;
        }
        break;
      }
      url->chosen_user = user;

    } else {
      /* multiple users for this URL so a choice needs to be made */
      PRUnichar ** list;
      PRUnichar ** list2;
      si_SignonUserStruct** users;
      si_SignonUserStruct** users2;
      list = (PRUnichar**)PR_Malloc(user_count*sizeof(PRUnichar*));
      users = (si_SignonUserStruct **) PR_Malloc(user_count*sizeof(si_SignonUserStruct*));
      list2 = list;
      users2 = users;

      /* step through set of user nodes for this URL and create list of
       * first data node of each (presumably that is the user name).
       * Note that the user nodes are already ordered by
       * most-recently-used so the first one in the list is the most
       * likely one to be chosen.
       */
      user_count = 0;
      PRInt32 userCount = url->signonUser_list.Count();
      for (PRInt32 i=0; i<userCount; i++) {
        user = static_cast<si_SignonUserStruct*>(url->signonUser_list.ElementAt(i));
        /* consider first data node to be the identifying item */
        data = static_cast<si_SignonDataStruct *>
                          (user->signonData_list.ElementAt(0));
        if (data->name != userText) {
          /* name of current data item does not match name in data node */
          continue;
        }
        nsAutoString userName;
        data = si_GetFirstNonPasswordData(user);
        if (NS_SUCCEEDED(Wallet_Decrypt (data->value, userName))) {
          *(list2++) = ToNewUnicode(userName);
          *(users2++) = user;
          user_count++;
        } else {
          break;
        }
      }

      /* have user select a username from the list */
      PRUnichar * selectUser = Wallet_Localize("SelectUser");
      if (user_count == 0) {
        /* not first data node for any saved user, so simply pick first user */
        if (url->chosen_user) {
          user = url->chosen_user;
        } else {
          /* no user selection had been made for first data node */
          user = NULL;
        } 
      } else if (user_count == 1) {
        /* only one user for this form at this url, so select it */
        user = users[0];
      } else if ((user_count > 1) && si_SelectDialog(selectUser, dialog, list, &user_count, formNumber)) {
        /* user pressed OK */
        if (user_count == -1) {
          user_count = 0; /* user didn't select, so use first one */
        }
        user = users[user_count]; /* this is the selected item */
        /* item selected is now most-recently used, put at head of list */
        url->signonUser_list.RemoveElement(user);
        url->signonUser_list.InsertElementAt(user, 0);
      } else {
        user = NULL;
      }
      Recycle(selectUser);
      url->chosen_user = user;
      while (--list2 > list) {
        Recycle(*list2);
      }
      PR_Free(list);
      PR_Free(users);

      /* if we don't remove the URL from the cache at this point, the
       * cached copy will be brought containing the last-used username
       * rather than the username that was just selected
       */

#ifdef junk
      NET_RemoveURLFromCache(NET_CreateURLStruct((char *)passwordRealm, NET_DONT_RELOAD));
#endif

    }
    si_ReleaseCompositeURL(url);
  } else {
    user = NULL;
  }
  return user;
}

/*
 * Get a specific user node for a given URL
 *
 * This routine is called only when holding the signon lock!!!
 *
 * This routine is called only if signon pref is enabled!!!
 */
static si_SignonUserStruct*
si_GetSpecificUser(const char* passwordRealm, const nsString& userName, const nsString& userText) {
  si_SignonURLStruct* url;
  si_SignonUserStruct* user;
  si_SignonDataStruct* data;

  /* get to node for this URL */
  url = si_GetURL(passwordRealm);
  if (url != NULL) {

    /* step through set of user nodes for this URL looking for specified username */
    PRInt32 userCount2 = url->signonUser_list.Count();
    for (PRInt32 i2=0; i2<userCount2; i2++) {
      user = static_cast<si_SignonUserStruct*>(url->signonUser_list.ElementAt(i2));
      /* consider first data node to be the identifying item */
      data = static_cast<si_SignonDataStruct *>
                        (user->signonData_list.ElementAt(0));
      if (data->name != userText) {
        /* desired username text does not match name in data node */
        continue;
      }
      if (!si_CompareEncryptedToCleartext(data->value, userName)) {
        /* desired username value does not match value in data node */
        continue;
      }
      return user;
    }

    /* if we don't remove the URL from the cache at this point, the
     * cached copy will be brought containing the last-used username
     * rather than the username that was just selected
     */

#ifdef junk
    NET_RemoveURLFromCache(NET_CreateURLStruct((char *)passwordRealm, NET_DONT_RELOAD));
#endif

  }
  return NULL;
}

/*
 * Get the url and user for which a change-of-password is to be applied
 *
 * This routine is called only when holding the signon lock!!!
 *
 * This routine is called only if signon pref is enabled!!!
 */
static si_SignonUserStruct*
si_GetURLAndUserForChangeForm(nsIPrompt* dialog, const nsString& password)
{
  si_SignonURLStruct* url;
  si_SignonUserStruct* user;
  si_SignonDataStruct * data;
  PRInt32 user_count;

  PRUnichar ** list;
  PRUnichar ** list2;
  si_SignonUserStruct** users;
  si_SignonUserStruct** users2;
  si_SignonURLStruct** urls;
  si_SignonURLStruct** urls2;

  /* get count of total number of user nodes at all url nodes */
  user_count = 0;
  PRInt32 urlCount = LIST_COUNT(si_signon_list);
  for (PRInt32 i=0; i<urlCount; i++) {
    url = static_cast<si_SignonURLStruct*>(si_signon_list->ElementAt(i));
    PRInt32 userCount = url->signonUser_list.Count();
    for (PRInt32 ii=0; ii<userCount; ii++) {
      user = static_cast<si_SignonUserStruct*>(url->signonUser_list.ElementAt(ii));
      user_count++;
    }
  }

  /* avoid malloc of zero */
  if( user_count == 0 )
  {
    return NULL;
  }
  
  /* allocate lists for maximumum possible url and user names */
  list = (PRUnichar**)PR_Malloc(user_count*sizeof(PRUnichar*));
  users = (si_SignonUserStruct **) PR_Malloc(user_count*sizeof(si_SignonUserStruct*));
  urls = (si_SignonURLStruct **)PR_Malloc(user_count*sizeof(si_SignonUserStruct*));
  list2 = list;
  users2 = users;
  urls2 = urls;
    
  /* step through set of URLs and users and create list of each */
  user_count = 0;
  PRInt32 urlCount2 = LIST_COUNT(si_signon_list);
  for (PRInt32 i2=0; i2<urlCount2; i2++) {
    url = static_cast<si_SignonURLStruct*>(si_signon_list->ElementAt(i2));
    PRInt32 userCount = url->signonUser_list.Count();
    for (PRInt32 i3=0; i3<userCount; i3++) {
      user = static_cast<si_SignonUserStruct*>(url->signonUser_list.ElementAt(i3));
      /* find saved password and see if it matches password user just entered */
      PRInt32 dataCount = user->signonData_list.Count();
      for (PRInt32 i4=0; i4<dataCount; i4++) {
        data = static_cast<si_SignonDataStruct*>(user->signonData_list.ElementAt(i4));
        if (data->isPassword && si_CompareEncryptedToCleartext(data->value, password)) {
          /* passwords match so add entry to list */
          /* consider first data node to be the identifying item */
          data = static_cast<si_SignonDataStruct *>
                            (user->signonData_list.ElementAt(0));

          nsAutoString userName;
          if (NS_SUCCEEDED(Wallet_Decrypt (data->value, userName))) {
            nsAutoString temp; temp.AssignASCII(url->passwordRealm); // XXX non-ascii realms?
            temp.AppendLiteral(":");
            temp.Append(userName);

            *list2 = ToNewUnicode(temp);
            list2++;
            *(users2++) = user;
            *(urls2++) = url;
            user_count++;
          }
          break;
        }
      }
    }
  }

  /* query user */
  PRUnichar * msg = Wallet_Localize("SelectUserWhosePasswordIsBeingChanged");
//@@@@ is 0 correct?
  if (user_count && si_SelectDialog(msg, dialog, list, &user_count, 0)) {
    user = users[user_count];
    url = urls[user_count];
    /*
     * since this user node is now the most-recently-used one, move it
     * to the head of the user list so that it can be favored for
     * re-use the next time this form is encountered
     */
    url->signonUser_list.RemoveElement(user);
    url->signonUser_list.InsertElementAt(user, 0);
    si_signon_list_changed = PR_TRUE;
    si_SaveSignonDataLocked("signons", PR_TRUE);
  } else {
    user = NULL;
  }
  Recycle(msg);

  /* free allocated strings */
  while (--list2 > list) {
    Recycle(*list2);
  }
  PR_Free(list);
  PR_Free(users);
  PR_Free(urls);
  return user;
}

/*
 * Remove all the signons and free everything
 */

void
SI_RemoveAllSignonData() {
  if (si_PartiallyLoaded) {
    /* repeatedly remove first user node of first URL node */
    while (si_RemoveUser(NULL, EmptyString(), PR_FALSE, PR_FALSE, PR_FALSE, PR_TRUE)) {
    }
  }
  si_PartiallyLoaded = PR_FALSE;

  if (si_reject_list) {
    si_Reject * reject;
    while (LIST_COUNT(si_reject_list)>0) {
      reject = static_cast<si_Reject*>(si_reject_list->ElementAt(0));
      if (reject) {
        si_FreeReject(reject);
        si_signon_list_changed = PR_TRUE;
      }
    }
    delete si_reject_list;
    si_reject_list = nsnull;
  }
  delete si_signon_list;
  si_signon_list = nsnull;
}

void
SI_DeleteAll() {
  if (si_PartiallyLoaded) {
    /* repeatedly remove first user node of first URL node */
    while (si_RemoveUser(NULL, EmptyString(), PR_FALSE, PR_FALSE, PR_TRUE, PR_TRUE)) {
    }
  }
  si_PartiallyLoaded = PR_FALSE;
  si_signon_list_changed = PR_TRUE;
  si_SaveSignonDataLocked("signons", PR_TRUE);
}

void
SI_ClearUserData() {
  SI_RemoveAllSignonData();
  gLoadedUserData = PR_FALSE;
}

void
SI_DeletePersistentUserData() {

  if (signonFileName && signonFileName[0]) {
    nsCOMPtr<nsIFile> file;
    nsresult rv = Wallet_ProfileDirectory(getter_AddRefs(file));
    if (NS_SUCCEEDED(rv)) {
      rv = file->AppendNative(nsDependentCString(signonFileName));
      if (NS_SUCCEEDED(rv))
        file->Remove(PR_FALSE);
    }
  }
}

/****************************
 * Managing the Reject List *
 ****************************/

static void
si_FreeReject(si_Reject * reject) {

  /*
   * This routine should only be called while holding the
   * signon list lock
   */

  if(!reject) {
      return;
  }
  si_reject_list->RemoveElement(reject);
  PR_FREEIF(reject->passwordRealm);
  delete reject;
}

static PRBool
si_CheckForReject(const char * passwordRealm, const nsString& userName) {
  si_Reject * reject;

  si_lock_signon_list();
  if (si_reject_list) {
    PRInt32 rejectCount = LIST_COUNT(si_reject_list);
    for (PRInt32 i=0; i<rejectCount; i++) {
      reject = static_cast<si_Reject*>(si_reject_list->ElementAt(i));
      if(!PL_strcmp(passwordRealm, reject->passwordRealm)) {
// No need for username check on a rejectlist entry.  URL check is sufficient
//    if(!PL_strcmp(userName, reject->userName) && !PL_strcmp(passwordRealm, reject->passwordRealm)) {
        si_unlock_signon_list();
        return PR_TRUE;
      }
    }
  }
  si_unlock_signon_list();
  return PR_FALSE;
}

static void
si_PutReject(const char * passwordRealm, const nsString& userName, PRBool save) {
  char * passwordRealm2=NULL;
  nsAutoString userName2;
  si_Reject * reject = new si_Reject;

  if (reject) {
    if(!si_reject_list) {
      si_reject_list = new nsVoidArray();
      if(!si_reject_list) {
        delete reject;
        return;
      }
    }

    /*
     * lock the signon list
     *  Note that, for efficiency, SI_LoadSignonData already sets the lock
     *  before calling this routine whereas none of the other callers do.
     *  So we need to determine whether or not we were called from
     *  SI_LoadSignonData before setting or clearing the lock.  We can
     *  determine this by testing "save" since only SI_LoadSignonData
     *  passes in a value of PR_FALSE for "save".
     */
    if (save) {
      si_lock_signon_list();
    }

    StrAllocCopy(passwordRealm2, passwordRealm);
    userName2 = userName;
    reject->passwordRealm = passwordRealm2;
    reject->userName = userName2;

#ifdef alphabetize
    /* add it to the list in alphabetical order */
    si_Reject * tmp_reject;
    PRBool rejectAdded = PR_FALSE;
    PRInt32 rejectCount = LIST_COUNT(si_reject_list);
    for (PRInt32 i = 0; i<rejectCount; ++i) {
      tmp_reject = static_cast<si_Reject *>(si_reject_list->ElementAt(i));
      if (tmp_reject) {
        if (PL_strcasecmp(reject->passwordRealm, tmp_reject->passwordRealm)<0) {
          si_reject_list->InsertElementAt(reject, i);
          rejectAdded = PR_TRUE;
          break;
        }
      }
    }
    if (!rejectAdded) {
      si_reject_list->AppendElement(reject);
    }
#else
    /* add it to the end of the list */
    si_reject_list->AppendElement(reject);
#endif

    if (save) {
      si_signon_list_changed = PR_TRUE;
      si_lock_signon_list();
      si_SaveSignonDataLocked("rejects", PR_TRUE);
      si_unlock_signon_list();
    }
  }
}

/*
 * Put data obtained from a submit form into the data structure for
 * the specified URL
 *
 * See comments below about state of signon lock when routine is called!!!
 *
 * This routine is called only if signon pref is enabled!!!
 */
static void
si_PutData(const char *passwordRealm, nsVoidArray *signonData, PRBool save) {
  PRBool added_to_list = PR_FALSE;
  si_SignonURLStruct * url;
  si_SignonUserStruct * user;
  si_SignonDataStruct * data;
  si_SignonDataStruct * data2;
  PRBool mismatch = PR_FALSE;

  /* discard this if the password is empty */
  PRInt32 count = signonData->Count();
  for (PRInt32 i=0; i<count; i++) {
    data2 = static_cast<si_SignonDataStruct*>(signonData->ElementAt(i));
    if (data2->isPassword && data2->value.IsEmpty()) {
      return;
    }
  }

  /* make sure the signon list exists */
  if (!si_signon_list) {
    si_signon_list = new nsVoidArray();
    if (!si_signon_list) {
      return;
    }
  }

  /*
   * lock the signon list
   *   Note that, for efficiency, SI_LoadSignonData already sets the lock
   *   before calling this routine whereas none of the other callers do.
   *   So we need to determine whether or not we were called from
   *   SI_LoadSignonData before setting or clearing the lock.  We can
   *   determine this by testing "save" since only SI_LoadSignonData passes
   *   in a value of PR_FALSE for "save".
   */
  if (save) {
    si_lock_signon_list();
  }

  /* find node in signon list having the same URL */
  if ((url = si_GetURL(passwordRealm)) == NULL) {

    /* doesn't exist so allocate new node to be put into signon list */
    url = new si_SignonURLStruct;
    if (!url) {
      if (save) {
        si_unlock_signon_list();
      }
      return;
    }

    /* fill in fields of new node */
	url->passwordRealm = nsnull;
	if (passwordRealm) {
    	url->passwordRealm = PL_strdup(passwordRealm);
	}

    if (!url->passwordRealm) {
      if (save) {
        si_unlock_signon_list();
      }
      return;
    }

    /* put new node into signon list */

#ifdef alphabetize
    /* add it to the list in alphabetical order */
    si_SignonURLStruct * tmp_URL;
    PRInt32 urlCount = LIST_COUNT(si_signon_list);
    for (PRInt32 ii = 0; ii<urlCount; ++ii) {
      tmp_URL = static_cast<si_SignonURLStruct *>(si_signon_list->ElementAt(ii));
      if (tmp_URL) {
        if (PL_strcasecmp(url->passwordRealm, tmp_URL->passwordRealm)<0) {
          si_signon_list->InsertElementAt(url, ii);
          added_to_list = PR_TRUE;
          break;
        }
      }
    }
    if (!added_to_list) {
      si_signon_list->AppendElement(url);
    }
#else
    /* add it to the end of the list */
    si_signon_list->AppendElement(url);
#endif
  }

  /* initialize state variables in URL node */
  url->chosen_user = NULL;

  /*
   * see if a user node with data list matching new data already exists
   * (password fields will not be checked for in this matching)
   */
  PRInt32 userCount = url->signonUser_list.Count();
  for (PRInt32 i2=0; i2<userCount; i2++) {
    if (!save) {
      break; /* otherwise we could be asked for master password when loading signon data */
    }
    user = static_cast<si_SignonUserStruct*>(url->signonUser_list.ElementAt(i2));
    PRInt32 j = 0;
    PRInt32 dataCount = user->signonData_list.Count();
    for (PRInt32 i3=0; i3<dataCount; i3++) {
      data = static_cast<si_SignonDataStruct*>(user->signonData_list.ElementAt(i3));
      mismatch = PR_FALSE;

      /* check for match on name field and type field */
      if (j < signonData->Count()) {
        data2 = static_cast<si_SignonDataStruct*>(signonData->ElementAt(j));
        if ((data->isPassword == data2->isPassword) &&
            (data->name == data2->name)) {

          /* success, now check for match on value field if not password */
          if (si_CompareEncryptedToEncrypted(data->value, data2->value) || data->isPassword) {
            j++; /* success */
          } else {
            mismatch = PR_TRUE;
            break; /* value mismatch, try next user */
          }
        } else {
          mismatch = PR_TRUE;
          break; /* name or type mismatch, try next user */
        }
      }
    }
    if (!mismatch) {

      /* all names and types matched and all non-password values matched */

      /*
       * note: it is ok for password values not to match; it means
       * that the user has either changed his password behind our
       * back or that he previously mis-entered the password
       */

      /* update the saved password values */
      j = 0;
      PRInt32 dataCount2 = user->signonData_list.Count();
      for (PRInt32 i4=0; i4<dataCount2; i4++) {
        data = static_cast<si_SignonDataStruct*>(user->signonData_list.ElementAt(i4));

        /* update saved password */
        if ((j < signonData->Count()) && data->isPassword) {
          data2 = static_cast<si_SignonDataStruct*>(signonData->ElementAt(j));
          if (!si_CompareEncryptedToEncrypted(data->value, data2->value)) {
            si_signon_list_changed = PR_TRUE;
            data->value = data2->value;
            user->time = SecondsFromPRTime(PR_Now()); 
/* commenting out because I don't see how such randomizing could ever have worked. */
//          si_Randomize(data->value);
          }
        }
        j++;
      }

      /*
       * since this user node is now the most-recently-used one, move it
       * to the head of the user list so that it can be favored for
       * re-use the next time this form is encountered
       */
      url->signonUser_list.RemoveElement(user);
      url->signonUser_list.InsertElementAt(user, 0);

      /* return */
      if (save) {
        si_signon_list_changed = PR_TRUE;
        si_SaveSignonDataLocked("signons", PR_TRUE);
        si_unlock_signon_list();
      }
      return; /* nothing more to do since data already exists */
    }
  }

  /* user node with current data not found so create one */
  user = new si_SignonUserStruct;
  if (!user) {
    if (save) {
      si_unlock_signon_list();
    }
    return;
  }

  /* create and fill in data nodes for new user node */
  for (PRInt32 k=0; k<signonData->Count(); k++) {

    /* create signon data node */
    data = new si_SignonDataStruct;
    if (!data) {
      delete user;
      if (save) {
        si_unlock_signon_list();
      }
      return;
    }
    data2 = static_cast<si_SignonDataStruct*>(signonData->ElementAt(k));
    data->isPassword = data2->isPassword;
    data->name = data2->name;
    data->value = data2->value;
/* commenting out because I don't see how such randomizing could ever have worked. */
//  if (data->isPassword) {
//    si_Randomize(data->value);
//  }
    /* append new data node to end of data list */
    user->signonData_list.AppendElement(data);
  }

  /* append new user node to front of user list for matching URL */
    /*
     * Note that by appending to the front, we assure that if there are
     * several users, the most recently used one will be favored for
     * reuse the next time this form is encountered.  But don't do this
     * when reading in the saved signons (i.e., when save is PR_FALSE), otherwise
     * we will be reversing the order when reading in.
     */
  if (save) {
    user->time = SecondsFromPRTime(PR_Now());
    url->signonUser_list.InsertElementAt(user, 0);
    si_signon_list_changed = PR_TRUE;
    si_SaveSignonDataLocked("signons", PR_TRUE);
    si_unlock_signon_list();
  } else {
    user->time = 0;
    url->signonUser_list.AppendElement(user);
  }
}


/*****************************
 * Managing the Signon Files *
 *****************************/
 
////////////////////////////////////////////////////////////////////////////////
// nsSingleSignOnProfileObserver
// This observer is a global object and is registered the first time any consumer
// touches signon profile data. That is, when SI_LoadSignonData() is called.

class nsSingleSignOnProfileObserver : public nsIObserver
{
public:
    nsSingleSignOnProfileObserver() { }
    virtual ~nsSingleSignOnProfileObserver() {}
    
    NS_DECL_ISUPPORTS
    
    NS_IMETHODIMP Observe(nsISupports*, const char *aTopic, const PRUnichar *someData) 
    {
        if (!strcmp(aTopic, "profile-before-change")) {
            SI_ClearUserData();
        if (!nsCRT::strcmp(someData, NS_LITERAL_STRING("shutdown-cleanse").get()))
            SI_DeletePersistentUserData();
        }
        return NS_OK;
    }
};
NS_IMPL_THREADSAFE_ISUPPORTS1(nsSingleSignOnProfileObserver, nsIObserver)

static nsresult EnsureSingleSignOnProfileObserver()
{
  static nsSingleSignOnProfileObserver *gSignOnProfileObserver;
  
  if (!gSignOnProfileObserver) {      
    nsCOMPtr<nsIObserverService> observerService(do_GetService("@mozilla.org/observer-service;1"));
    if (!observerService)
      return NS_ERROR_FAILURE;
      
    gSignOnProfileObserver = new nsSingleSignOnProfileObserver;
    if (!gSignOnProfileObserver)
      return NS_ERROR_OUT_OF_MEMORY;

    // The observer service holds the only ref to the observer
    // It thus has the lifespan of the observer service
    nsresult rv = observerService->AddObserver(gSignOnProfileObserver, "profile-before-change", PR_FALSE);
    if (NS_FAILED(rv)) {
      delete gSignOnProfileObserver;
      gSignOnProfileObserver = nsnull; 
      return rv;
    }
  }
  return NS_OK;
}

#define BUFFER_SIZE 4096

/*
 * get a line from a file
 * return -1 if end of file reached
 * strip carriage returns and line feeds from end of line
 */
static PRInt32
si_ReadLine(nsIInputStream* strm, nsString& lineBuffer)
{
  nsCAutoString line;
  nsresult rv = wallet_GetLine(strm, line);
  if (NS_FAILED(rv))
    return -1;
  
  CopyUTF8toUTF16(line, lineBuffer);
  return NS_OK;
}

/*
 * Load signon data from disk file
 * Return value is:
 *   -1: fatal error
 *    0: successfully load
 *   +1: user aborted the load (by failing to open the database)
 */
int
SI_LoadSignonData() {
  char * passwordRealm;
  nsAutoString buffer;
  PRBool badInput = PR_FALSE;

#ifdef APPLE_KEYCHAIN
  if (KeychainManagerAvailable()) {
    SI_RemoveAllSignonData();
    return si_LoadSignonDataFromKeychain();
  }
#endif
  
  /* open the signon file */
  nsCOMPtr<nsIFile> file;
  nsresult rv = Wallet_ProfileDirectory(getter_AddRefs(file));
  if (NS_FAILED(rv)) {
    return -1;
  }


  rv = EnsureSingleSignOnProfileObserver();
  NS_ASSERTION(NS_SUCCEEDED(rv), "Failed to register profile change observer");

  SI_InitSignonFileName();
  file->AppendNative(nsDependentCString(signonFileName));

  nsCOMPtr<nsIInputStream> strm;
  rv = NS_NewLocalFileInputStream(getter_AddRefs(strm), file);
  if (NS_FAILED(rv)) {
    si_PartiallyLoaded = PR_TRUE;
    return 0;
  }

  SI_RemoveAllSignonData();

  /* read the format information */
  nsAutoString format;
  if (NS_FAILED(si_ReadLine(strm, format))) {
    return -1;
  }
  if (!format.EqualsLiteral(HEADER_VERSION)) {
    /* something's wrong */
    return -1;
  }

  /* read the reject list */
  si_lock_signon_list();
  while (NS_SUCCEEDED(si_ReadLine(strm, buffer))) {
    /* a blank line is perfectly valid here -- corresponds to a local file */
    if (!buffer.IsEmpty() && buffer.CharAt(0) == '.') {
      break; /* end of reject list */
    }
    passwordRealm = ToNewCString(buffer);
    si_PutReject(passwordRealm, buffer, PR_FALSE); /* middle parameter is obsolete */
    Recycle (passwordRealm);
  }

  /* read the URL line */
  while (NS_SUCCEEDED(si_ReadLine(strm, buffer))) {
    /* a blank line is perfectly valid here -- corresponds to a local file */
    passwordRealm = ToNewCString(buffer);
    if (!passwordRealm) {
      si_unlock_signon_list();
      return -1;
    }

    /* prepare to read the name/value pairs */
    badInput = PR_FALSE;

    nsVoidArray signonData;
    si_SignonDataStruct * data;
    while(NS_SUCCEEDED(si_ReadLine(strm, buffer))) {

      /* line starting with . terminates the pairs for this URL entry */
      if (buffer.CharAt(0) == '.') {
        break; /* end of URL entry */
      }

      /* line just read is the name part */

      /* save the name part and determine if it is a password */
      PRBool ret;
      nsAutoString name;
      nsAutoString value;
      PRBool isPassword;
      if (buffer.CharAt(0) == '*') {
        isPassword = PR_TRUE;
        buffer.Mid(name, 1, buffer.Length()-1);
        ret = si_ReadLine(strm, buffer);
      } else {
        isPassword = PR_FALSE;
        name = buffer;
        ret = si_ReadLine(strm, buffer);
      }

      /* read in and save the value part */
      if(NS_FAILED(ret)) {
        /* error in input file so give up */
        badInput = PR_TRUE;
        break;
      }
      value = buffer;

      data = new si_SignonDataStruct;
      data->name = name;
      data->value = value;
      data->isPassword = isPassword;
      signonData.AppendElement(data);
    }

    /* store the info for this URL into memory-resident data structure */
    PRInt32 count = signonData.Count();
    if (count) {
      si_PutData(passwordRealm, &signonData, PR_FALSE);
    }

    /* free up all the allocations done for processing this URL */
    Recycle(passwordRealm);
    for (PRInt32 i=count-1; i>=0; i--) {
      data = static_cast<si_SignonDataStruct*>(signonData.ElementAt(i));
      delete data;
    }
  }

  si_unlock_signon_list();
  si_PartiallyLoaded = PR_TRUE;
  return 0;
}

/*
 * Save signon data to disk file
 * The parameter passed in on entry is ignored
 *
 * This routine is called only when holding the signon lock!!!
 *
 * This routine is called only if signon pref is enabled!!!
 */

static int
si_SaveSignonDataLocked(char * state, PRBool notify) {
  si_SignonURLStruct * url;
  si_SignonUserStruct * user;
  si_SignonDataStruct * data;
  si_Reject * reject;

  /* do nothing if signon list has not changed */
  if(!si_signon_list_changed) {
    return(-1);
  }

#ifdef APPLE_KEYCHAIN
  if (KeychainManagerAvailable()) {
    return si_SaveSignonDataInKeychain();
  }
#endif

  /* do nothing if we are unable to open file that contains signon list */
  nsCOMPtr<nsIFile> file;
  nsresult rv = Wallet_ProfileDirectory(getter_AddRefs(file));
  if (NS_FAILED(rv)) {
    return 0;
  }

  file->AppendNative(nsDependentCString(signonFileName));

  nsCOMPtr<nsIOutputStream> fileOutputStream;
  rv = NS_NewSafeLocalFileOutputStream(getter_AddRefs(fileOutputStream),
                                       file,
                                       -1,
                                       0600);
  if (NS_FAILED(rv))
    return 0;

  nsCOMPtr<nsIOutputStream> strm;
  rv = NS_NewBufferedOutputStream(getter_AddRefs(strm), fileOutputStream, 4096);
  if (NS_FAILED(rv))
    return 0;

  /* write out the format revision number */

  wallet_PutLine(strm, HEADER_VERSION);

  /* format for next part of file shall be:
   * passwordRealm -- first url/username on reject list
   * userName
   * passwordRealm -- second url/username on reject list
   * userName
   * ...     -- etc.
   * .       -- end of list
   */

  /* write out reject list */
  if (si_reject_list) {
    PRInt32 rejectCount = LIST_COUNT(si_reject_list);
    for (PRInt32 i=0; i<rejectCount; i++) {
      reject = static_cast<si_Reject*>(si_reject_list->ElementAt(i));
      wallet_PutLine(strm, reject->passwordRealm);
    }
  }
  wallet_PutLine(strm, ".");

  /* format for cached logins shall be:
   * url LINEBREAK {name LINEBREAK value LINEBREAK}*  . LINEBREAK
   * if type is password, name is preceded by an asterisk (*)
   */

  /* write out each URL node */
  if((si_signon_list)) {
    PRInt32 urlCount = LIST_COUNT(si_signon_list);
    for (PRInt32 i2=0; i2<urlCount; i2++) {
      url = static_cast<si_SignonURLStruct*>(si_signon_list->ElementAt(i2));

      /* write out each user node of the URL node */
      PRInt32 userCount = url->signonUser_list.Count();
      for (PRInt32 i3=0; i3<userCount; i3++) {
        user = static_cast<si_SignonUserStruct*>(url->signonUser_list.ElementAt(i3));
        wallet_PutLine(strm, url->passwordRealm);

        /* write out each data node of the user node */
        PRInt32 dataCount = user->signonData_list.Count();
        for (PRInt32 i4=0; i4<dataCount; i4++) {
          data = static_cast<si_SignonDataStruct*>(user->signonData_list.ElementAt(i4));
          if (data->isPassword) {
            static const char asterisk = '*';
            PRUint32 dummy;
            strm->Write(&asterisk, 1, &dummy);
          }
          wallet_PutLine(strm, NS_ConvertUTF16toUTF8(data->name).get());
          wallet_PutLine(strm, NS_ConvertUTF16toUTF8(data->value).get());
        }
        wallet_PutLine(strm, ".");
      }
    }
  }
  si_signon_list_changed = PR_FALSE;

  // All went ok. Maybe except for problems in Write(), but the stream detects
  // that for us
  nsCOMPtr<nsISafeOutputStream> safeStream = do_QueryInterface(strm);
  NS_ASSERTION(safeStream, "expected a safe output stream!");
  if (safeStream) {
    rv = safeStream->Finish();
    if (NS_FAILED(rv)) {
      NS_WARNING("failed to save wallet file! possible dataloss");
      return 0;
    }
  }
  strm = nsnull;
  fileOutputStream = nsnull;


  /* Notify signon manager dialog to update its display */
  if (notify) {
    nsCOMPtr<nsIObserverService> os(do_GetService("@mozilla.org/observer-service;1"));
    if (os) {
      os->NotifyObservers(nsnull, "signonChanged", NS_ConvertASCIItoUTF16(state).get());
    }
  }

  return 0;
}


/***************************
 * Processing Signon Forms *
 ***************************/

static PRBool
si_ExtractRealm(nsIURI *uri, nsCString &realm)
{
  nsCAutoString hostPort;

  /* Security check: if URI is of a scheme that doesn't support hostnames,
   * we have no host to get the signon data from, so we must not attempt to
   * build a valid realm from the URI (bug 159484) */
  nsresult rv = uri->GetHostPort(hostPort);
  if (NS_FAILED(rv) || hostPort.IsEmpty())
    return PR_FALSE;

  nsCAutoString scheme;
  rv = uri->GetScheme(scheme);
  if (NS_FAILED(rv) || scheme.IsEmpty())
    return PR_FALSE;

  realm = scheme + NS_LITERAL_CSTRING("://") + hostPort;
  return PR_TRUE;
}

/* Ask user if it is ok to save the signon data */
static PRBool
si_OkToSave(const char *passwordRealm, const char *legacyRealm,
            const nsString& userName, nsIDOMWindowInternal* window) {

  /* if url/user already exists, then it is safe to save it again */
  if (si_CheckForUser(passwordRealm, userName)) {
    return PR_TRUE;
  }
  if (legacyRealm && si_CheckForUser(legacyRealm, userName)) {
    return PR_TRUE;
  }

#ifdef WALLET_PASSWORDMANAGER_DEFAULT_IS_OFF
  if (!si_RememberSignons && !si_GetNotificationPref()) {
    PRUnichar * notification = Wallet_Localize("PasswordNotification");
    si_SetNotificationPref(PR_TRUE);
    if (!si_ConfirmYN(notification, window)) {
      Recycle(notification);
      SI_SetBoolPref(pref_rememberSignons, PR_FALSE);
      return PR_FALSE;
    }
    Recycle(notification);
    SI_SetBoolPref(pref_rememberSignons, PR_TRUE);
  }
#endif

  if (si_CheckForReject(passwordRealm, userName)) {
    return PR_FALSE;
  }
  if (legacyRealm && si_CheckForReject(legacyRealm, userName)) {
    return PR_FALSE;
  }

  PRUnichar * message;
  if (SI_GetBoolPref(pref_Crypto, PR_FALSE)) {
    message = Wallet_Localize("WantToSavePasswordEncrypted?");
  } else {
    message = Wallet_Localize("WantToSavePasswordObscured?");
  }

  PRInt32 button = si_3ButtonConfirm(message, window);
  if (button == NEVER_BUTTON) {
    si_PutReject(passwordRealm, userName, PR_TRUE);
  }
  Recycle(message);
  return (button == YES_BUTTON);
}

/*
 * Check for a signon submission and remember the data if so
 */
static void
si_RememberSignonData
    (nsIPrompt* dialog, const char* passwordRealm, const char* legacyRealm,
     nsVoidArray * signonData, nsIDOMWindowInternal* window)
{
  int passwordCount = 0;
  int pswd[3];
  si_SignonDataStruct * data = nsnull;  // initialization only to avoid warning message
  si_SignonDataStruct * data0;
  si_SignonDataStruct * data1;
  si_SignonDataStruct * data2;

  /* do nothing if signon preference is not enabled */
  if (!si_GetSignonRememberingPref()){
    return;
  }

  /* determine how many passwords are in the form and where they are */
  for (PRInt32 i=0; i<signonData->Count(); i++) {
    data = static_cast<si_SignonDataStruct*>(signonData->ElementAt(i));
    if (data->isPassword) {
      if (passwordCount < 3 ) {
        pswd[passwordCount] = i;
      }
      passwordCount++;
    }
  }

  /* process the form according to how many passwords it has */
  if (passwordCount == 1) {
    /* one-password form is a log-in so remember it */

    /* obtain the index of the first input field (that is the username) */
    PRInt32 j;
    for (j=0; j<signonData->Count(); j++) {
      data = static_cast<si_SignonDataStruct*>(signonData->ElementAt(j));
      if (!data->isPassword) {
        break;
      }
    }

    if (j<signonData->Count()) {
      if (si_OkToSave(passwordRealm, legacyRealm, data->value, window)) {
        // remove legacy password entry if found
        if (legacyRealm && si_CheckForUser(legacyRealm, data->value)) {
          si_RemoveUser(legacyRealm, data->value, PR_TRUE, PR_FALSE, PR_TRUE);
        }
        Wallet_GiveCaveat(window, nsnull);
        for (j=0; j<signonData->Count(); j++) {
          data2 = static_cast<si_SignonDataStruct*>(signonData->ElementAt(j));
          nsAutoString value(data2->value);
          if (NS_FAILED(Wallet_Encrypt(value, data2->value))) {
            return;
          }
        }
        si_PutData(passwordRealm, signonData, PR_TRUE);
      }
    }
  } else if (passwordCount == 2) {
    /* two-password form is a registration */

  } else if (passwordCount == 3) {
    /* three-password form is a change-of-password request */

    si_SignonUserStruct* user;

    /* make sure all passwords are non-null and 2nd and 3rd are identical */
    data0 = static_cast<si_SignonDataStruct*>(signonData->ElementAt(pswd[0]));
    data1 = static_cast<si_SignonDataStruct*>(signonData->ElementAt(pswd[1]));
    data2 = static_cast<si_SignonDataStruct*>(signonData->ElementAt(pswd[2]));
    if (data0->value.IsEmpty() || data1->value.IsEmpty() ||
        data2->value.IsEmpty() || data1->value != data2->value) {
      return;
    }

    /* ask user if this is a password change */
    si_lock_signon_list();
    user = si_GetURLAndUserForChangeForm(dialog, data0->value);

    /* return if user said no */
    if (!user) {
      si_unlock_signon_list();
      return;
    }

    /* get to password being saved */
    PRInt32 dataCount = user->signonData_list.Count();
    for (PRInt32 k=0; k<dataCount; k++) {
      data = static_cast<si_SignonDataStruct*>(user->signonData_list.ElementAt(k));
      if (data->isPassword) {
        break;
      }
    }

    /*
     * if second password is "********" then generate a random
     * password for it and use same random value for third password
     * as well (Note: this all works because we already know that
     * second and third passwords are identical so third password
     * must also be "********".  Furthermore si_Randomize() will
     * create a random password of exactly eight characters -- the
     * same length as "********".)
     */

/* commenting out because I don't see how such randomizing could ever have worked. */
//    si_Randomize(data1->value);
//    data2->value = data1->value;

    if (NS_SUCCEEDED(Wallet_Encrypt(data1->value, data->value))) {
      user->time = SecondsFromPRTime(PR_Now()); 
      si_signon_list_changed = PR_TRUE;
      si_SaveSignonDataLocked("signons", PR_TRUE);
      si_unlock_signon_list();
    }
  }
}

void
SI_ShutdownModule()
{
  if(signon_lock_monitor) {
    PR_DestroyMonitor(signon_lock_monitor);
    signon_lock_monitor = nsnull;
  }
}

void
SINGSIGN_RememberSignonData 
    (nsIPrompt* dialog, nsIURI* passwordRealm, nsVoidArray * signonData,
     nsIDOMWindowInternal* window)
{
  if (!passwordRealm)
    return;      

  nsCAutoString realm, legacyRealm;
  if (!si_ExtractRealm(passwordRealm, realm))
    return;

  if (NS_FAILED(passwordRealm->GetHost(legacyRealm)))
    return;

  if (!realm.IsEmpty()) {
    si_RememberSignonData(dialog, realm.get(), legacyRealm.get(), signonData, window);
  }
}

static void
si_RestoreSignonData(nsIPrompt* dialog,
                     const char* passwordRealm, const char* legacyRealm,
                     const PRUnichar* name, PRUnichar** value,
                     PRUint32 formNumber, PRUint32 elementNumber) {
  si_SignonUserStruct* user;
  si_SignonDataStruct* data;
  nsAutoString correctedName;

  /* do nothing if signon preference is not enabled */
  if (!si_GetSignonRememberingPref()){
    return;
  }

  si_lock_signon_list();
  if (elementNumber == 0) {
    si_LastFormForWhichUserHasBeenSelected = -1;
  }

  /* Correct the field name to avoid mistaking for fields in browser-generated form
   *
   *   Note that data saved for browser-generated logins (e.g. http authentication)
   *   use artificial field names starting with * \= (see USERNAMEFIELD and PASSWORDFIELD.
   *   To avoid mistakes whereby saved logins for http authentication is then prefilled
   *   into a field on the html form at the same URL, we will prevent html field names
   *   from starting with \=.  We do that by doubling up a backslash if it appears in the
   *   first character position
   */
  if (*name == '\\') {
    correctedName = NS_LITERAL_STRING("\\") + nsDependentString(name);
  } else {
    correctedName = name;
  }

  /* determine if name has been saved (avoids unlocking the database if not) */
  PRBool nameFound = PR_FALSE;
  user = si_GetUser(dialog, passwordRealm, legacyRealm, PR_FALSE, correctedName, formNumber);
  if (user) {
    PRInt32 dataCount = user->signonData_list.Count();
    for (PRInt32 i=0; i<dataCount; i++) {
      data = static_cast<si_SignonDataStruct*>(user->signonData_list.ElementAt(i));
      LOG(("  got [name=%s value=%s]\n",
              NS_LossyConvertUTF16toASCII(data->name).get(),
              NS_LossyConvertUTF16toASCII(data->value).get()));
      if(!correctedName.IsEmpty() && (data->name == correctedName)) {
        nameFound = PR_TRUE;
      }
    }
  }
  if (!nameFound) {
    si_unlock_signon_list();
    return;
  }

#ifdef xxx
  /*
   * determine if it is a change-of-password field
   *    the heuristic that we will use is that if this is the first
   *    item on the form and it is a password, this is probably a
   *    change-of-password form
   */
  /* see if this is first item in form and is a password */
  /* get first saved user just so we can see the name of the first item on the form */
  user = si_GetUser(passwordRealm, PR_TRUE, NULL, formNumber); /* this is the first saved user */
  if (user) {
    data = static_cast<si_SignonDataStruct *>
                      (user->signonData_list.ElementAt(0)); /* 1st item on form */
    if(data->isPassword && !correctedName.IsEmpty() && (data->name == correctedName)) {
      /* current item is first item on form and is a password */
      user = (passwordRealm, MK_SIGNON_PASSWORDS_FETCH);
      if (user) {
        /* user has confirmed it's a change-of-password form */
        PRInt32 dataCount = user->signonData_list.Count();
        for (PRInt32 i=1; i<dataCount; i++) {
          data = static_cast<si_SignonDataStruct*>(user->signonData_list.ElementAt(i));
          if (data->isPassword) {
            nsAutoString password;
            if (NS_SUCCEEDED(Wallet_Decrypt(data->value, password))) {
              *value = ToNewUnicode(password);
            }
            si_unlock_signon_list();
            return;
          }
        }
      }
    }
  }
#endif

  /* restore the data from previous time this URL was visited */

  user = si_GetUser(dialog, passwordRealm, legacyRealm, PR_FALSE, correctedName, formNumber);
  if (user) {
    PRInt32 dataCount = user->signonData_list.Count();
    for (PRInt32 i=0; i<dataCount; i++) {
      data = static_cast<si_SignonDataStruct*>(user->signonData_list.ElementAt(i));
      LOG(("  got [name=%s value=%s]\n",
              NS_LossyConvertUTF16toASCII(data->name).get(),
              NS_LossyConvertUTF16toASCII(data->value).get()));
      if(!correctedName.IsEmpty() && (data->name == correctedName)) {
        nsAutoString password;
        if (NS_SUCCEEDED(Wallet_Decrypt(data->value, password))) {
          *value = ToNewUnicode(password);
        }
        si_unlock_signon_list();
        return;
      }
    }
  }
  si_unlock_signon_list();
}

void
SINGSIGN_RestoreSignonData(nsIPrompt* dialog, nsIURI* passwordRealm, const PRUnichar* name, PRUnichar** value, PRUint32 formNumber, PRUint32 elementNumber) {
  LOG(("enter SINGSIGN_RestoreSignonData\n"));

  if (!passwordRealm)
    return;  

  nsCAutoString realm;
  if (!si_ExtractRealm(passwordRealm, realm))
    return;

  nsCAutoString legacyRealm;
  if (NS_FAILED(passwordRealm->GetHost(legacyRealm)))
    return;

  si_RestoreSignonData(dialog, realm.get(), legacyRealm.get(), name, value, formNumber, elementNumber);

  LOG(("exit SINGSIGN_RestoreSignonData [value=%s]\n",
      *value ? NS_LossyConvertUTF16toASCII(*value).get() : "(null)"));
}

/*
 * Remember signon data from a browser-generated password dialog
 */
static void
si_RememberSignonDataFromBrowser(const char* passwordRealm, const nsString& username, const nsString& password) {
  /* do nothing if signon preference is not enabled */
  if (!si_GetSignonRememberingPref()){
    return;
  }

  nsVoidArray signonData;
  si_SignonDataStruct data1;
  data1.name.AssignLiteral(USERNAMEFIELD);
  if (NS_FAILED(Wallet_Encrypt(username, data1.value))) {
    return;
  }
  data1.isPassword = PR_FALSE;
  signonData.AppendElement(&data1);
  si_SignonDataStruct data2;
  data2.name.AssignLiteral(PASSWORDFIELD);
  if (NS_FAILED(Wallet_Encrypt(password, data2.value))) {
    return;
  }
  data2.isPassword = PR_TRUE;
  signonData.AppendElement(&data2);

  /* Save the signon data */
  si_PutData(passwordRealm, &signonData, PR_TRUE);
}

/*
 * Check for remembered data from a previous browser-generated password dialog
 * restore it if so
 */
static PRBool
si_RestoreOldSignonDataFromBrowser
    (nsIPrompt* dialog, const char* passwordRealm, PRBool pickFirstUser, nsString& username, nsString& password) {
  si_SignonUserStruct* user;
  si_SignonDataStruct* data;

  /* get the data from previous time this URL was visited */
  si_lock_signon_list();
  if (!username.IsEmpty()) {
    user = si_GetSpecificUser(passwordRealm, username, NS_ConvertASCIItoUTF16(USERNAMEFIELD));
  } else {
    si_LastFormForWhichUserHasBeenSelected = -1;
    user = si_GetUser(dialog, passwordRealm, nsnull, pickFirstUser, NS_ConvertASCIItoUTF16(USERNAMEFIELD), 0);
  }
  if (!user) {
    /* leave original username and password from caller unchanged */
    /* username = 0; */
    /* *password = 0; */
    si_unlock_signon_list();
    return PR_FALSE;
  }

  /* restore the data from previous time this URL was visited */
  PRInt32 dataCount = user->signonData_list.Count();
  for (PRInt32 i=0; i<dataCount; i++) {
    data = static_cast<si_SignonDataStruct*>(user->signonData_list.ElementAt(i));
    nsAutoString decrypted;
    if (NS_SUCCEEDED(Wallet_Decrypt(data->value, decrypted))) {
      if(data->name.EqualsLiteral(USERNAMEFIELD)) {
        username = decrypted;
      } else if(data->name.EqualsLiteral(PASSWORDFIELD)) {
        password = decrypted;
      }
    }
  }
  si_unlock_signon_list();
  return PR_TRUE;
}

PRBool
SINGSIGN_StorePassword(const char *passwordRealm, const PRUnichar *user, const PRUnichar *password)
{
//  Wallet_GiveCaveat(nsnull, dialog); ??? what value to use for dialog?
  si_RememberSignonDataFromBrowser(passwordRealm, nsDependentString(user), nsDependentString(password));
  return PR_TRUE;
}

enum DialogType {promptUsernameAndPassword, promptPassword, prompt};

static nsresult
si_DoDialogIfPrefIsOff(
    const PRUnichar *dialogTitle,
    const PRUnichar *text,
    PRUnichar **user,
    PRUnichar **pwd,
    const PRUnichar *defaultText,
    PRUnichar **resultText,
    const char *passwordRealm,
    nsIPrompt* dialog,
    PRBool *pressedOK, 
    PRUint32 savePassword,
    DialogType dlg) {

  nsresult res = NS_ERROR_FAILURE;
  const PRUnichar * prompt_string = dialogTitle;
  if (dialogTitle == nsnull || !dialogTitle[0]) {
    prompt_string = Wallet_Localize("PromptForData");
  }

  nsAutoString data(defaultText);
  switch (dlg) {
    case promptUsernameAndPassword:
      res = dialog->PromptUsernameAndPassword(prompt_string,
                                              text,
                                              user,
                                              pwd,
                                              nsnull, nsnull,
                                              pressedOK);
      break;
    case promptPassword:
      res = dialog->PromptPassword(prompt_string,
                                   text,
                                   pwd,
                                   nsnull, nsnull,
                                   pressedOK);
      break;
    case prompt:
      *resultText = ToNewUnicode(data);
      res = dialog->Prompt(prompt_string,
                           text,
                           resultText,
                           nsnull, nsnull,
                           pressedOK);
#ifdef DEBUG
      break;
    default:
      NS_ERROR("Undefined DialogType in si_DoDialogIfPrefIsOff");
#endif
  }

  if (dialogTitle != prompt_string) {
    Recycle(const_cast<PRUnichar*>(prompt_string));
  }
  return res;
}

/* The following comments apply to the three prompt routines that follow
 *
 * If a password was successfully obtain (either from the single-signon
 * database or from a dialog with the user), we return NS_OK for the
 * function value and PR_TRUE for the boolean argument "pressedOK".
 *
 * If the user presses cancel from the dialog, we return NS_OK for the
 * function value and PR_FALSE for the boolean argument "pressedOK".
 *
 * If a password is not collected for any other reason, we return the
 * failure code for the function value and the boolean argument
 * "pressedOK" is undefined.
 */

nsresult
SINGSIGN_PromptUsernameAndPassword
    (const PRUnichar *dialogTitle, const PRUnichar *text, PRUnichar **user, PRUnichar **pwd,
     const char *passwordRealm, nsIPrompt* dialog, PRBool *pressedOK, PRUint32 savePassword) {

  nsresult res;

  /* do only the dialog if signon preference is not enabled */
  if (!si_GetSignonRememberingPref()){
    return si_DoDialogIfPrefIsOff(dialogTitle,
                                  text,
                                  user,
                                  pwd,
                                  nsnull,
                                  nsnull,
                                  passwordRealm,
                                  dialog,
                                  pressedOK, 
                                  savePassword,
                                  promptUsernameAndPassword);
  }

  /* prefill with previous username/password if any */
  nsAutoString username, password;
  si_RestoreOldSignonDataFromBrowser(dialog, passwordRealm, PR_FALSE, username, password);

  /* get new username/password from user */
  if (!(*user = ToNewUnicode(username))) {
    return NS_ERROR_OUT_OF_MEMORY;
  }
  if (!(*pwd = ToNewUnicode(password))) {
    PR_Free(*user);
    return NS_ERROR_OUT_OF_MEMORY;
  }

  PRBool checked = (**user != 0);
  PRBool remembered = checked;
  res = si_CheckGetUsernamePassword(user, pwd, dialogTitle, text, dialog, savePassword, &checked);
  if (NS_FAILED(res)) {
    /* user pressed Cancel */
    PR_FREEIF(*user);
    PR_FREEIF(*pwd);
    *pressedOK = PR_FALSE;
    return NS_OK;
  }
  if (checked) {
    Wallet_GiveCaveat(nsnull, dialog);
    si_RememberSignonDataFromBrowser (passwordRealm, nsDependentString(*user), nsDependentString(*pwd));
  } else if (remembered) {
    /* a login was remembered but user unchecked the box; we forget the remembered login */
    si_RemoveUser(passwordRealm, username, PR_TRUE, PR_FALSE, PR_TRUE);  
  }

  /* cleanup and return */
  *pressedOK = PR_TRUE;
  return NS_OK;
}

nsresult
SINGSIGN_PromptPassword
    (const PRUnichar *dialogTitle, const PRUnichar *text, PRUnichar **pwd, const char *passwordRealm,
     nsIPrompt* dialog, PRBool *pressedOK, PRUint32 savePassword) 
{

  nsresult res;
  nsAutoString password, username;

  /* do only the dialog if signon preference is not enabled */
  if (!si_GetSignonRememberingPref()){
    return si_DoDialogIfPrefIsOff(dialogTitle,
                                  text,
                                  nsnull,
                                  pwd,
                                  nsnull,
                                  nsnull,
                                  passwordRealm,
                                  dialog,
                                  pressedOK, 
                                  savePassword,
                                  promptPassword);
  }

  /* get previous password used with this username, pick first user if no username found */
  si_RestoreOldSignonDataFromBrowser(dialog, passwordRealm, username.IsEmpty(), username, password);

  /* return if a password was found */
  if (!password.IsEmpty()) {
    *pwd = ToNewUnicode(password);
    *pressedOK = PR_TRUE;
    return NS_OK;
  }

  /* no password found, get new password from user */
  PRBool checked = PR_FALSE;
  res = si_CheckGetPassword(pwd, dialogTitle, text, dialog, savePassword, &checked);
  if (NS_FAILED(res)) {
    /* user pressed Cancel */
    PR_FREEIF(*pwd);
    *pressedOK = PR_FALSE;
    return NS_OK;
  }
  if (checked) {
    Wallet_GiveCaveat(nsnull, dialog);
    si_RememberSignonDataFromBrowser(passwordRealm, username, nsDependentString(*pwd));
  }

  /* cleanup and return */
  *pressedOK = PR_TRUE;
  return NS_OK;
}

nsresult
SINGSIGN_Prompt
    (const PRUnichar *dialogTitle, const PRUnichar *text, const PRUnichar *defaultText, PRUnichar **resultText,
     const char *passwordRealm, nsIPrompt* dialog, PRBool *pressedOK, PRUint32 savePassword) 
{
  nsresult res;
  nsAutoString data, emptyUsername;

  /* do only the dialog if signon preference is not enabled */
  if (!si_GetSignonRememberingPref()){
    return si_DoDialogIfPrefIsOff(dialogTitle,
                                  text,
                                  nsnull,
                                  nsnull,
                                  defaultText,
                                  resultText,
                                  passwordRealm,
                                  dialog,
                                  pressedOK, 
                                  savePassword,
                                  prompt);
  }

  /* get previous data used with this hostname */
  si_RestoreOldSignonDataFromBrowser(dialog, passwordRealm, PR_TRUE, emptyUsername, data);

  /* return if data was found */
  if (!data.IsEmpty()) {
    *resultText = ToNewUnicode(data);
    *pressedOK = PR_TRUE;
    return NS_OK;
  }

  /* no data found, get new data from user */
  data = defaultText;
  *resultText = ToNewUnicode(data);
  PRBool checked = PR_FALSE;
  res = si_CheckGetData(resultText, dialogTitle, text, dialog, savePassword, &checked);
  if (NS_FAILED(res)) {
    /* user pressed Cancel */
    PR_FREEIF(*resultText);
    *pressedOK = PR_FALSE;
    return NS_OK;
  }
  if (checked) {
    Wallet_GiveCaveat(nsnull, dialog);
    si_RememberSignonDataFromBrowser(passwordRealm, emptyUsername, nsDependentString(*resultText));
  }

  /* cleanup and return */
  *pressedOK = PR_TRUE;
  return NS_OK;
}

nsresult
SINGSIGN_PromptAuth
    (nsIPromptService2* aService, nsIDOMWindow* aParent, nsIChannel* aChannel,
     PRUint32 aLevel, nsIAuthInformation* aAuthInfo, PRBool* retval) {

  nsCAutoString key;
  NS_GetAuthKey(aChannel, aAuthInfo, key);

  /* do only the dialog if signon preference is not enabled */
  if (!si_GetSignonRememberingPref()){
    return aService->PromptAuth(aParent, aChannel, aLevel,
                                aAuthInfo, nsnull, nsnull,
                                retval);
  }

  /* prefill with previous username/password if any */
  /* this needs a dialog to choose between multiple usernames */
  nsCOMPtr<nsIPrompt> prompt;
  nsresult rv;
  nsCOMPtr<nsIWindowWatcher> wwatcher =
    do_GetService(NS_WINDOWWATCHER_CONTRACTID, &rv);
  wwatcher->GetNewPrompter(aParent, getter_AddRefs(prompt));

  // If we have a domain, insert a "domain\" in front of the username
  // for the lookup
  nsAutoString domain, username, password;
  aAuthInfo->GetDomain(domain);
  aAuthInfo->GetUsername(username);
  if (!domain.IsEmpty()) {
    domain.Append(PRUnichar('\\'));
    username.Insert(domain, 0);
  }
  // Offer saving the data iff we had previously stored information
  PRBool checked = si_RestoreOldSignonDataFromBrowser(prompt,
                                                      key.get(),
                                                      PR_FALSE,
                                                      username,
                                                      password);

  PRUint32 flags = 0;
  aAuthInfo->GetFlags(&flags);

  if (checked) {
    NS_SetAuthInfo(aAuthInfo, username, password);
    // If we were only asked for a password, return immediately
    // (to match SINGSIGN_PromptPassword)
    if (flags & nsIAuthInformation::ONLY_PASSWORD)
      return NS_OK;
  }

  PRBool remembered = checked;
  rv = si_CheckPromptAuth(aService, aParent, aChannel, aLevel,
                          aAuthInfo, &checked);
  if (NS_FAILED(rv)) {
    /* user pressed Cancel */
    *retval = PR_FALSE;
    return NS_OK;
  }

  /* Get the newly entered data back */
  aAuthInfo->GetDomain(domain);
  aAuthInfo->GetUsername(username);
  aAuthInfo->GetPassword(password);
  if (!domain.IsEmpty()) {
    domain.Append(PRUnichar('\\'));
    username.Insert(domain, 0);
  }

  if (checked) {
    Wallet_GiveCaveat(nsnull, prompt);
    si_RememberSignonDataFromBrowser (key.get(), username, password);
  } else if (remembered) {
    /* a login was remembered but user unchecked the box; we forget the remembered login */
    si_RemoveUser(key.get(), username, PR_TRUE, PR_FALSE, PR_TRUE);  
  }

  /* cleanup and return */
  *retval = PR_TRUE;
  return NS_OK;
}


/*****************
 * Signon Viewer *
 *****************/

/* return PR_TRUE if "number" is in sequence of comma-separated numbers */
PRBool
SI_InSequence(const nsString& sequence, PRInt32 number)
{
  nsAutoString tail( sequence );
  nsAutoString head, temp;
  PRInt32 separator;

  for (;;) {
    /* get next item in list */
    separator = tail.FindChar(',');
    if (-1 == separator) {
      return PR_FALSE;
    }
    tail.Left(head, separator);
    tail.Mid(temp, separator+1, tail.Length() - (separator+1));
    tail = temp;

    /* test item to see if it equals our number */
    PRInt32 error;
    PRInt32 numberInList = head.ToInteger(&error);
    if (!error && numberInList == number) {
      return PR_TRUE;
    }
  }
  /* NOTREACHED */
  return PR_FALSE;
}

void
SI_FindValueInArgs(const nsAString& results, const nsAString& name, nsAString& value)
{
  /* note: name must start and end with a vertical bar */
  nsReadingIterator<PRUnichar> start, end, barPos;
  results.BeginReading(start);
  results.EndReading(end);

  FindInReadable(name, start, end);
  if (start == end) {
    return;
  }
  start.advance(name.Length()); /* get past the |name| part */
  barPos = start;
  results.EndReading(end);
  FindCharInReadable(PRUnichar('|'), barPos, end);
  value = Substring(start, barPos);
}

PRBool
SINGSIGN_ReencryptAll()
{
  /* force loading of the signons file */
  si_RegisterSignonPrefCallbacks();

  nsAutoString buffer;
  si_SignonURLStruct *url;
  si_SignonUserStruct * user;
  si_SignonDataStruct* data = nsnull;

  si_lock_signon_list();
  PRInt32 urlCount = LIST_COUNT(si_signon_list);
  for (PRInt32 i=0; i<urlCount; i++) {
    url = static_cast<si_SignonURLStruct*>(si_signon_list->ElementAt(i));
    PRInt32 userCount = url->signonUser_list.Count();
    for (PRInt32 j=0; j<userCount; j++) {
      user = static_cast<si_SignonUserStruct*>(url->signonUser_list.ElementAt(j));

      PRInt32 dataCount = user->signonData_list.Count();
      for (PRInt32 k=0; k<dataCount; k++) {
        data = static_cast<si_SignonDataStruct *>
                          (user->signonData_list.ElementAt(k));
        nsAutoString userName;
        if (NS_FAILED(Wallet_Decrypt(data->value, userName))) {
          //Don't try to re-encrypt. Just go to the next one.
          continue;
        }
        if (NS_FAILED(Wallet_Encrypt(userName, data->value))) {
          return PR_FALSE;
        }
      }
    }
  }
  si_signon_list_changed = PR_TRUE;
  si_SaveSignonDataLocked("signons", PR_TRUE);
  si_unlock_signon_list();
  return PR_TRUE;
}

nsresult
SINGSIGN_HaveData(nsIPrompt* dialog, const char *passwordRealm, const PRUnichar *userName, PRBool *retval)
{
  nsAutoString data, usernameForLookup;

  *retval = PR_FALSE;

  if (!si_GetSignonRememberingPref()) {
    return NS_OK;
  }

  /* get previous data used with this username, pick first user if no username found */
  si_RestoreOldSignonDataFromBrowser(dialog, passwordRealm, usernameForLookup.IsEmpty(), usernameForLookup, data);

  if (!data.IsEmpty()) {
    *retval = PR_TRUE;
  }

  return NS_OK;
}

PRInt32
SINGSIGN_HostCount() {
  /* force loading of the signons file */
  si_RegisterSignonPrefCallbacks();

  if (!si_signon_list) {
    return 0;
  }
  return si_signon_list->Count();
}

PRInt32
SINGSIGN_UserCount(PRInt32 host) {
  if (!si_signon_list) {
    return 0;
  }

  si_SignonURLStruct *hostStruct;
  hostStruct = static_cast<si_SignonURLStruct*>(si_signon_list->ElementAt(host));
  return hostStruct->signonUser_list.Count();
}

nsresult
SINGSIGN_Enumerate
    (PRInt32 hostNumber, PRInt32 userNumber, PRBool decrypt, char **host,
     PRUnichar ** user, PRUnichar ** pswd) {

  if (gSelectUserDialogCount>0 && hostNumber==0 && userNumber==0) {
    // starting to enumerate over all saved logins
    // notify recipients if login list is in use by SelectUserDialog
    nsCOMPtr<nsIObserverService> os(do_GetService("@mozilla.org/observer-service;1"));
    if (os) {
      os->NotifyObservers(nsnull, "signonSelectUser", NS_LITERAL_STRING("inUse").get());
    }
  }

  if (hostNumber > SINGSIGN_HostCount() || userNumber > SINGSIGN_UserCount(hostNumber)) {
    return NS_ERROR_FAILURE;
  }
  si_SignonURLStruct *hostStruct;
  si_SignonUserStruct * userStruct;
  si_SignonDataStruct* data = nsnull;

  hostStruct = static_cast<si_SignonURLStruct*>(si_signon_list->ElementAt(hostNumber));
  NS_ASSERTION(hostStruct, "corrupt singlesignon list");
  *host = (char *) nsMemory::Clone
    (hostStruct->passwordRealm, strlen(hostStruct->passwordRealm) + 1);
  NS_ENSURE_ARG_POINTER(host);
  userStruct =
    static_cast<si_SignonUserStruct*>(hostStruct->signonUser_list.ElementAt(userNumber));

  /* first non-password data item for user is the username */
  PRInt32 dataCount = userStruct->signonData_list.Count();
  PRInt32 k;
  for (k=0; k<dataCount; k++) {
    data = static_cast<si_SignonDataStruct *>(userStruct->signonData_list.ElementAt(k));
    if (!(data->isPassword)) {
      break;
    }
  }

  nsresult rv;
  nsAutoString userName;
  if (decrypt) {
    rv = Wallet_Decrypt(data->value, userName);
    if (NS_FAILED(rv)) {
      /* don't display saved signons if user couldn't unlock the database */
    return rv;
    }
  } else {
    userName = data->value;
  }
  if (!(*user = ToNewUnicode(userName))) {
    return NS_ERROR_OUT_OF_MEMORY;
  }

  /* first password data item for user is the password */
  for (k=0; k<dataCount; k++) {
    data = static_cast<si_SignonDataStruct *>(userStruct->signonData_list.ElementAt(k));
    if ((data->isPassword)) {
      break;
    }
  }

  nsAutoString passWord;
  if (decrypt) {
    rv = Wallet_Decrypt(data->value, passWord);
    if (NS_FAILED(rv)) {
      /* don't display saved signons if user couldn't unlock the database */
      Recycle(*user);
      return rv;
    }
  } else {
    passWord = data->value;
  }  
  if (!(*pswd = ToNewUnicode(passWord))) {
    Recycle(*user);
    return NS_ERROR_OUT_OF_MEMORY;
  }
  return NS_OK;
}

PRInt32
SINGSIGN_RejectCount() {
  if (!si_reject_list) {
    return 0;
  }
  return si_reject_list->Count();
}

nsresult
SINGSIGN_RejectEnumerate
    (PRInt32 rejectNumber, char **host) {

  si_Reject *reject;
  reject = static_cast<si_Reject*>(si_reject_list->ElementAt(rejectNumber));
  NS_ASSERTION(reject, "corrupt reject list");

  *host = (char *) nsMemory::Clone
    (reject->passwordRealm, strlen(reject->passwordRealm) + 1);
  NS_ENSURE_ARG_POINTER(host);
  return NS_OK;
}

#ifdef APPLE_KEYCHAIN
/************************************
 * Apple Keychain Specific Routines *
 ************************************/

/*
 * APPLE
 * The Keychain callback.  This routine will be called whenever a lock,
 * delete, or update event occurs in the Keychain.  The only action taken
 * is to make the signon list invalid, so it will be read in again the
 * next time it is accessed.
 */
OSStatus PR_CALLBACK
si_KeychainCallback( KCEvent keychainEvent, KCCallbackInfo *info, void *userContext) {
  PRBool    *listInvalid = (PRBool*)userContext;
  *listInvalid = PR_TRUE;
}

/*
 * APPLE
 * Get the signon data from the keychain
 *
 * This routine is called only if signon pref is enabled!!!
 */
static int
si_LoadSignonDataFromKeychain() {
  char * passwordRealm;
  si_FormSubmitData submit;
  nsAutoString name_array[MAX_ARRAY_SIZE];
  nsAutoString value_array[MAX_ARRAY_SIZE];
  uint8 type_array[MAX_ARRAY_SIZE];
  char buffer[BUFFER_SIZE];
  PRBool badInput = PR_FALSE;
  int i;
  KCItemRef   itemRef;
  KCAttributeList attrList;
  KCAttribute attr[2];
  KCItemClass itemClass = kInternetPasswordKCItemClass;
  KCProtocolType protocol = kNetscapeProtocolType;
  OSStatus status = noErr;
  KCSearchRef searchRef = NULL;
  /* initialize the submit structure */
  submit.name_array = name_array;
  submit.value_array = value_array;
  submit.type_array = (PRUnichar *)type_array;

  /* set up the attribute list */
  attrList.count = 2;
  attrList.attr = attr;
  attr[0].tag = kClassKCItemAttr;
  attr[0].data = &itemClass;
  attr[0].length = sizeof(itemClass);

  attr[1].tag = kProtocolKCItemAttr;
  attr[1].data = &protocol;
  attr[1].length = sizeof(protocol);

  status = KCFindFirstItem( &attrList, &searchRef, &itemRef );

#if 0
  if (status == noErr) {
    /* if we found a Netscape item, let's assume notice has been given */
    si_SetNotificationPref(PR_TRUE);
  } else {
    si_SetNotificationPref(PR_FALSE);
  }
#endif

  si_lock_signon_list();
  while(status == noErr) {
    char *value;
    uint16 i = 0;
    uint32 actualSize;
    KCItemFlags flags;
    PRBool reject = PR_FALSE;
    submit.value_cnt = 0;

    /* first find out if it is a reject entry */
    attr[0].tag = kFlagsKCItemAttr;
    attr[0].length = sizeof(KCItemFlags);
    attr[0].data = &flags;
    status = KCGetAttribute( itemRef, attr, nil );
    if (status != noErr) {
      break;
    }
    if (flags & kNegativeKCItemFlag) {
      reject = PR_TRUE;
    }

    /* get the server name */
    attr[0].tag = kServerKCItemAttr;
    attr[0].length = BUFFER_SIZE;
    attr[0].data = buffer;
    status = KCGetAttribute( itemRef, attr, &actualSize );
    if (status != noErr) {
      break;
    }

    /* null terminate */
    buffer[actualSize] = 0;
    passwordRealm = NULL;
    StrAllocCopy(passwordRealm, buffer);
    if (!reject) {
      /* get the password data */
      status = KCGetData(itemRef, BUFFER_SIZE, buffer, &actualSize);
      if (status != noErr) {
        break;
      }

      /* null terminate */
      buffer[actualSize] = 0;

      /* parse for '=' which separates the name and value */
      uint16 bufferlen = PL_strlen(buffer);
      for (i = 0; i < bufferlen; i++) {
        if (buffer[i] == '=') {
          value = &buffer[i+1];
          buffer[i] = 0;
          break;
        }
      }
      name_array[submit.value_cnt] = NULL;
      value_array[submit.value_cnt] = NULL;
      type_array[submit.value_cnt] = FORM_TYPE_PASSWORD;
      StrAllocCopy(name_array[submit.value_cnt], buffer);
      StrAllocCopy(value_array[submit.value_cnt], value);
    }

    /* get the account attribute */
    attr[0].tag = kAccountKCItemAttr;
    attr[0].length = BUFFER_SIZE;
    attr[0].data = buffer;
    status = KCGetAttribute( itemRef, attr, &actualSize );
    if (status != noErr) {
      break;
    }

    /* null terminate */
    buffer[actualSize] = 0;
    if (!reject) {
      /* parse for '=' which separates the name and value */
      uint16 bufferlen = PL_strlen(buffer);
      for (i = 0; i < bufferlen; i++) {
        if (buffer[i] == '=') {
          value = &buffer[i+1];
          buffer[i] = 0;
          break;
        }
      }
      submit.value_cnt++;
      name_array[submit.value_cnt] = NULL;
      value_array[submit.value_cnt] = NULL;
      type_array[submit.value_cnt] = FORM_TYPE_TEXT;
      StrAllocCopy(name_array[submit.value_cnt], buffer);
      StrAllocCopy(value_array[submit.value_cnt], value);

      /* check for overruning of the arrays */
      if (submit.value_cnt >= MAX_ARRAY_SIZE) {
        break;
      }
      submit.value_cnt++;
      /* store the info for this URL into memory-resident data structure */
      if (!passwordRealm || passwordRealm[0] == 0) {
        badInput = PR_TRUE;
      }
      if (!badInput) {
        si_PutData(passwordRealm, &submit, PR_FALSE);
      }

    } else {
      /* reject */
      si_PutReject(passwordRealm, nsDependentString(buffer), PR_FALSE);
    }
    reject = PR_FALSE; /* reset reject flag */
    PR_Free(passwordRealm);
    KCReleaseItemRef( &itemRef );
    status = KCFindNextItem( searchRef, &itemRef);
  }
  si_unlock_signon_list();

  if (searchRef) {
    KCReleaseSearchRef( &searchRef );
  }

  /* Register a callback with the Keychain if we haven't already done so. */

  if (si_kcUPP == NULL) {
    si_kcUPP = NewKCCallbackProc( si_KeychainCallback );
    if (!si_kcUPP) {
      return memFullErr;
    }

    KCAddCallback( si_kcUPP, kLockKCEventMask + kDeleteKCEventMask + kUpdateKCEventMask, &si_list_invalid );
    /*
     * Note that the callback is not necessarily removed.  We take advantage
     * of the fact that the Keychain will clean up the callback when the app
     * goes away. It is explicitly removed when the signon preference is turned off.
     */
  }
  if (status == errKCItemNotFound) {
    status = 0;
  }
  return (status);
}

/*
 * APPLE
 * Save signon data to Apple Keychain
 *
 * This routine is called only if signon pref is enabled!!!
 */
static int
si_SaveSignonDataInKeychain() {
  char* account = nil;
  char* password = nil;
  si_SignonURLStruct * URL;
  si_SignonUserStruct * user;
  si_SignonDataStruct * data;
  si_Reject * reject;
  OSStatus status;
  KCItemRef itemRef;
  KCAttribute attr;
  KCItemFlags flags = kInvisibleKCItemFlag + kNegativeKCItemFlag;
  uint32 actualLength;

  /* save off the reject list */
  if (si_reject_list) {
    PRInt32 rejectCount = LIST_COUNT(si_reject_list);
    for (PRInt32 i=0; i<rejectCount; i++) {
      reject = static_cast<si_Reject*>(si_reject_list->ElementAt(i));
      status = kcaddinternetpassword
        (reject->passwordRealm, nil,
         reject->userName,
         kAnyPort,
         kNetscapeProtocolType,
         kAnyAuthType,
         0,
         nil,
         &itemRef);
      if (status != noErr && status != errKCDuplicateItem) {
        return(status);
      }
      if (status == noErr) {
        /*
         * make the item invisible so the user doesn't see it and
         * negative so we know that it is a reject entry
         */
        attr.tag = kFlagsKCItemAttr;
        attr.data = &flags;
        attr.length = sizeof( flags );

        status = KCSetAttribute( itemRef, &attr );
        if (status != noErr) {
          return(status);
        }
        status = KCUpdateItem(itemRef);
        if (status != noErr) {
          return(status);
        }
        KCReleaseItemRef(&itemRef);
      }
    }
  }

  /* save off the passwords */
  if((si_signon_list)) {
    PRInt32 urlCount = LIST_COUNT(si_signon_list);
    for (PRInt32 i=0; i<urlCount; i++) {
      URL = static_cast<si_SignonURLStruct*>(si_signon_list->ElementAt(i));

      /* add each user node of the URL node */
      PRInt32 userCount = URL->signonUser_list.Count();
      for (PRInt32 i=0; i<userCount; i++) {
        user = static_cast<si_SignonUserStruct*>(URL->signonUser_list.ElementAt(i));

        /* write out each data node of the user node */
        PRInt32 count = user->signonData_list.Count();
        for (PRInt32 i=0; i<count; i++) {
          data = static_cast<si_SignonDataStruct*>(user->signonData_list.ElementAt(i));
          char* attribute = nil;
          if (data->isPassword) {
            password = PR_Malloc(PL_strlen(data->value) + PL_strlen(data->name) + 2);
            if (!password) {
              return (-1);
            }
            attribute = password;
          } else {
            account = PR_Malloc( PL_strlen(data->value) + PL_strlen(data->name) + 2);
            if (!account) {
              PR_Free(password);
              return (-1);
            }
            attribute = account;
          }
          PL_strcpy(attribute, data->name);
          PL_strcat(attribute, "=");
          PL_strcat(attribute, data->value);
        }
        /* if it's already there, we just want to change the password */
        status = kcfindinternetpassword
            (URL->passwordRealm,
             nil,
             account,
             kAnyPort,
             kNetscapeProtocolType,
             kAnyAuthType,
             0,
             nil,
             &actualLength,
             &itemRef);
        if (status == noErr) {
          status = KCSetData(itemRef, PL_strlen(password), password);
          if (status != noErr) {
            return(status);
          }
          status = KCUpdateItem(itemRef);
          KCReleaseItemRef(&itemRef);
        } else {
          /* wasn't there, let's add it */
          status = kcaddinternetpassword
            (URL->passwordRealm,
             nil,
             account,
             kAnyPort,
             kNetscapeProtocolType,
             kAnyAuthType,
             PL_strlen(password),
             password,
             nil);
        }
        if (account) {
          PR_Free(account);
        }
        if (password) {
          PR_Free(password);
        } 
        account = password = nil;
        if (status != noErr) {
          return(status);
        }
      }
    }
  }
  si_signon_list_changed = PR_FALSE;
  return (0);
}

#endif
