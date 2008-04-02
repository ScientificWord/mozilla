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
 * The Original Code is Mozilla Communicator client code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Pierre Phaneuf <pp@ludusdesign.com>
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

#include "nsCOMPtr.h"
#include "nsWalletService.h"
#include "nsIServiceManager.h"
#include "wallet.h"
#include "singsign.h"
#include "nsPassword.h"
#include "nsIObserverService.h"
#include "nsIDOMHTMLDocument.h"
#include "nsIDOMHTMLCollection.h"
#include "nsIDOMHTMLFormElement.h"
#include "nsIDocument.h"
#include "nsCURILoader.h"
#include "nsIDOMHTMLInputElement.h"
#include "nsIFormControl.h"
#include "nsIDocShell.h"
#include "nsPIDOMWindow.h"
#include "nsIPrompt.h"
#include "nsIChannel.h"
#include "nsIWindowWatcher.h"
#include "nsIWebProgress.h"
#include "nsXPIDLString.h"
#include "nsUnicharUtils.h"
#include "nsReadableUtils.h"
#include "nsICategoryManager.h"
#include "nsNetUtil.h"
#include "nsEmbedCID.h"

// for making the leap from nsIDOMWindowInternal -> nsIPresShell

static NS_DEFINE_IID(kDocLoaderServiceCID, NS_DOCUMENTLOADER_SERVICE_CID);


////////////////////////////////////////////////////////////////////////////////
// nsWalletlibService

nsWalletlibService::nsWalletlibService()
{
}

nsWalletlibService::~nsWalletlibService()
{
#ifdef DEBUG_dp
  printf("Wallet Service destroyed successfully.\n");
#endif /* DEBUG_dp */
  Wallet_ReleaseAllLists();
  SI_ClearUserData();
}

NS_IMPL_THREADSAFE_ISUPPORTS6(nsWalletlibService,
                              nsIWalletService,
                              nsIObserver,
                              nsIFormSubmitObserver,
                              nsIWebProgressListener,
                              nsIPromptFactory,
                              nsISupportsWeakReference)

NS_IMETHODIMP nsWalletlibService::WALLET_PreEdit(nsAString& walletList) {
  ::WLLT_PreEdit(walletList);
  return NS_OK;
}

NS_IMETHODIMP nsWalletlibService::WALLET_PostEdit(const nsAString & walletList) {
  ::WLLT_PostEdit(walletList);
  return NS_OK;
}

NS_IMETHODIMP nsWalletlibService::WALLET_ChangePassword(PRBool* status) {
  ::WLLT_ChangePassword(status);
  return NS_OK;
}

NS_IMETHODIMP nsWalletlibService::WALLET_DeleteAll() {
  ::WLLT_DeleteAll();
  return NS_OK;
}

NS_IMETHODIMP
nsWalletlibService::WALLET_RequestToCapture(nsIDOMWindowInternal* aWin,
                                            PRUint32* status)
{

  nsCOMPtr<nsPIDOMWindow> window(do_QueryInterface(aWin));
  nsIDocShell *docShell = window->GetDocShell();

  nsCOMPtr<nsIPresShell> presShell;
  if(docShell)
   docShell->GetPresShell(getter_AddRefs(presShell));

  ::WLLT_RequestToCapture(presShell, aWin, status);
  return NS_OK;
}

NS_IMETHODIMP
nsWalletlibService::WALLET_PrefillOneElement
    (nsIDOMWindowInternal* aWin, nsIDOMNode* elementNode, PRUnichar **value)
{
  nsAutoString compositeValue;
  nsresult rv = ::WLLT_PrefillOneElement(aWin, elementNode, compositeValue);
  *value = ToNewUnicode(compositeValue);
  return rv;
}

NS_IMETHODIMP
nsWalletlibService::WALLET_Prefill(PRBool quick,
                                   nsIDOMWindowInternal* aWin,
                                   PRBool* status)
{
  nsCOMPtr<nsPIDOMWindow> window(do_QueryInterface(aWin));
  nsIDocShell *docShell = window->GetDocShell();

  nsCOMPtr<nsIPresShell> presShell;
  if(docShell)
    docShell->GetPresShell(getter_AddRefs(presShell));

  return ::WLLT_Prefill(presShell, quick, aWin);
}

NS_IMETHODIMP nsWalletlibService::WALLET_PrefillReturn(const nsAString & results){
  ::WLLT_PrefillReturn(results);
  return NS_OK;
}

NS_IMETHODIMP nsWalletlibService::WALLET_ExpirePassword(PRBool* status){
  ::WLLT_ExpirePassword(status);
  return NS_OK;
}

NS_IMETHODIMP nsWalletlibService::WALLET_InitReencryptCallback(nsIDOMWindowInternal* window){
  /* register callback to be used when encryption pref changes */
  ::WLLT_InitReencryptCallback(window);
  return NS_OK;
}

NS_IMETHODIMP nsWalletlibService::WALLET_GetNopreviewListForViewer(nsAString& aNopreviewList){
  ::WLLT_GetNopreviewListForViewer(aNopreviewList);
  return NS_OK;
}

NS_IMETHODIMP nsWalletlibService::WALLET_GetNocaptureListForViewer(nsAString& aNocaptureList){
  ::WLLT_GetNocaptureListForViewer(aNocaptureList);
  return NS_OK;
}

NS_IMETHODIMP nsWalletlibService::WALLET_GetPrefillListForViewer(nsAString& aPrefillList){
  ::WLLT_GetPrefillListForViewer(aPrefillList);
  return NS_OK;
}

NS_IMETHODIMP nsWalletlibService::SI_SignonViewerReturn(const nsAString& results){
  ::Wallet_SignonViewerReturn(results);
  return NS_OK;
}

NS_IMETHODIMP nsWalletlibService::Observe(nsISupports *aSubject, const char *aTopic, const PRUnichar *someData)
{
  if (!nsCRT::strcmp(aTopic, "profile-before-change")) {
    PRBool status;
    WLLT_ExpirePassword(&status);
    WLLT_ClearUserData();
    if (!nsCRT::strcmp(someData, NS_LITERAL_STRING("shutdown-cleanse").get())) {
      WLLT_DeletePersistentUserData();
    }
  }
  else if (!nsCRT::strcmp(aTopic, "login-succeeded")) {
    // A login succeeded; store the password.
    nsCOMPtr<nsIURI> uri = do_QueryInterface(aSubject);
    if (uri) {
      nsCAutoString spec;
      if (NS_SUCCEEDED(uri->GetSpec(spec)))
        SINGSIGN_StorePassword(spec.get(), EmptyString().get(), someData);
    }
  }
  else if (!nsCRT::strcmp(aTopic, "login-failed")) {
    // A login failed; clean out any information we've stored about
    // the URL where the failure occurred.
    nsCOMPtr<nsIURI> uri = do_QueryInterface(aSubject);
    if (uri) {
      nsCAutoString spec;
      if (NS_SUCCEEDED(uri->GetSpec(spec)))
        SINGSIGN_RemoveUserAfterLoginFailure(spec.get(), EmptyString().get(), PR_TRUE);
    }
  }
  return NS_OK;
}

#define CRLF "\015\012"
NS_IMETHODIMP nsWalletlibService::Notify(nsIDOMHTMLFormElement* formNode, nsIDOMWindowInternal* window, nsIURI* actionURL, PRBool* cancelSubmit)
{
  if (!formNode) {
    return NS_ERROR_FAILURE;
  }

  NS_ENSURE_TRUE(window, NS_OK);

  ::WLLT_OnSubmit(formNode, window);
  return NS_OK;
}

NS_IMETHODIMP
nsWalletlibService::RegisterProc(nsIComponentManager *aCompMgr,
                                 nsIFile *aPath,
                                 const char *registryLocation,
                                 const char *componentType,
                                 const nsModuleComponentInfo *info)
{
  // Register ourselves into the NS_CATEGORY_HTTP_STARTUP
  nsresult rv;
  nsCOMPtr<nsICategoryManager> catman = do_GetService(NS_CATEGORYMANAGER_CONTRACTID, &rv);
  if (NS_FAILED(rv)) return rv;

  nsXPIDLCString prevEntry;
  catman->AddCategoryEntry(NS_FIRST_FORMSUBMIT_CATEGORY, "Form Manager", NS_WALLETSERVICE_CONTRACTID,
                           PR_TRUE, PR_TRUE, getter_Copies(prevEntry));
  
  catman->AddCategoryEntry(NS_PASSWORDMANAGER_CATEGORY, "Password Manager", NS_WALLETSERVICE_CONTRACTID,
                           PR_TRUE, PR_TRUE, getter_Copies(prevEntry));
  return NS_OK;
}

NS_IMETHODIMP
nsWalletlibService::UnregisterProc(nsIComponentManager *aCompMgr,
                                   nsIFile *aPath,
                                   const char *registryLocation,
                                   const nsModuleComponentInfo *info)
{
  nsresult rv;
  nsCOMPtr<nsICategoryManager> catman = do_GetService(NS_CATEGORYMANAGER_CONTRACTID, &rv);
  if (NS_FAILED(rv)) return rv;

  catman->DeleteCategoryEntry(NS_FIRST_FORMSUBMIT_CATEGORY,
                              NS_WALLETSERVICE_CONTRACTID, PR_TRUE);

  catman->DeleteCategoryEntry(NS_PASSWORDMANAGER_CATEGORY,
                              NS_WALLETSERVICE_CONTRACTID, PR_TRUE);

  // Return value is not used from this function.
  return NS_OK;
}

PRBool expireMasterPassword = PR_FALSE;
#define expireMasterPasswordPref "signon.expireMasterPassword"

int PR_CALLBACK
ExpireMasterPasswordPrefChanged(const char * newpref, void * data) {
  nsresult rv;
  nsCOMPtr<nsIPref> prefs(do_GetService(NS_PREF_CONTRACTID, &rv));
  if (NS_FAILED(prefs->GetBoolPref(expireMasterPasswordPref, &expireMasterPassword))) {
    expireMasterPassword = PR_FALSE;
  }
  if (expireMasterPassword) {
      PRBool status;
      WLLT_ExpirePasswordOnly(&status);
  }
  return 0;
}

nsresult nsWalletlibService::Init()
{
  nsresult rv;

  nsCOMPtr<nsIObserverService> svc =
           do_GetService("@mozilla.org/observer-service;1", &rv);
  if (NS_SUCCEEDED(rv) && svc) {
    // Register as an observer of form submission
    svc->AddObserver(this, NS_EARLYFORMSUBMIT_SUBJECT, PR_TRUE);
    // Register as an observer of profile changes
    svc->AddObserver(this, "profile-before-change", PR_TRUE);
    // Register as an observer for login
    svc->AddObserver(this, "login-succeeded", PR_TRUE);
    svc->AddObserver(this, "login-failed", PR_TRUE);
  }
  else
    NS_ERROR("Could not get nsIObserverService");

  // Get the global document loader service...
  nsCOMPtr<nsIWebProgress> progress =
           do_GetService(kDocLoaderServiceCID, &rv);
  if (NS_SUCCEEDED(rv)) {
    (void) progress->AddProgressListener((nsIWebProgressListener*)this,
                                    nsIWebProgress::NOTIFY_STATE_DOCUMENT);
  }
  else
    NS_ERROR("Could not get nsIWebProgress");

  /* initialize the expire-master-password feature */
  nsCOMPtr<nsIPref> prefs(do_GetService(NS_PREF_CONTRACTID, &rv));
  if (NS_SUCCEEDED(rv)) {
    prefs->RegisterCallback(expireMasterPasswordPref, ExpireMasterPasswordPrefChanged, NULL);
    prefs->GetBoolPref(expireMasterPasswordPref, &expireMasterPassword);
  }

  return NS_OK;
}

// nsIWebProgressListener implementation
NS_IMETHODIMP
nsWalletlibService::OnStateChange(nsIWebProgress* aWebProgress,
                                  nsIRequest *aRequest,
                                  PRUint32 progressStateFlags,
                                  nsresult aStatus)
{
    nsresult rv = NS_OK;

    // If the load failed, do not try to prefill...
    if (NS_FAILED(aStatus)) {
       return NS_OK;
    }

    if (progressStateFlags & nsIWebProgressListener::STATE_IS_DOCUMENT) {
        if (progressStateFlags & nsIWebProgressListener::STATE_STOP) {

          nsCOMPtr<nsIDOMWindow> domWin;
          rv = aWebProgress->GetDOMWindow(getter_AddRefs(domWin));
          if (NS_FAILED(rv)) return rv;

          nsCOMPtr<nsIDOMDocument> domDoc;
          rv = domWin->GetDocument(getter_AddRefs(domDoc));
          if (NS_FAILED(rv)) return rv;

          // we only want to handle HTML documents as they're the
          // only one's that can have forms which we might want to
          // pre-fill.
          nsCOMPtr<nsIDOMHTMLDocument> htmldoc(do_QueryInterface(domDoc, &rv));
          if (NS_FAILED(rv)) return NS_OK;

          nsCOMPtr<nsIDocument> doc(do_QueryInterface(htmldoc, &rv));
          if (NS_FAILED(rv)) {
            NS_ASSERTION(0, "no document available");
            return NS_OK;
          }

          nsIURI *uri = doc->GetDocumentURI();
          if (!uri) {
            NS_ASSERTION(0, "no URI available");
            return NS_OK;
          }

          nsCOMPtr<nsIDOMHTMLCollection> forms;
          rv = htmldoc->GetForms(getter_AddRefs(forms));
          if (NS_FAILED(rv) || (forms == nsnull)) return rv;

          PRUint32 elementNumber = 0;
          PRUint32 numForms;
          forms->GetLength(&numForms);
          for (PRUint32 formX = 0; formX < numForms; formX++) {
            nsCOMPtr<nsIDOMNode> formNode;
            forms->Item(formX, getter_AddRefs(formNode));
            if (nsnull != formNode) {
              nsCOMPtr<nsIDOMHTMLFormElement> formElement(do_QueryInterface(formNode));
              if ((nsnull != formElement)) {
                nsCOMPtr<nsIDOMHTMLCollection> elements;
                rv = formElement->GetElements(getter_AddRefs(elements));
                if ((NS_SUCCEEDED(rv)) && (nsnull != elements)) {
                  /* got to the form elements at long last */
                  PRUint32 numElements;
                  elements->GetLength(&numElements);
                  /* get number of passwords on form */
                  PRInt32 passwordCount = 0;
                  for (PRUint32 elementXX = 0; elementXX < numElements; elementXX++) {
                    nsCOMPtr<nsIDOMNode> elementNode;
                    elements->Item(elementXX, getter_AddRefs(elementNode));
                    if (nsnull != elementNode) {
                      nsCOMPtr<nsIDOMHTMLInputElement> inputElement(do_QueryInterface(elementNode));
                      if ((NS_SUCCEEDED(rv)) && (nsnull != inputElement)) {
                        nsAutoString type;
                        rv = inputElement->GetType(type);
                        if (NS_SUCCEEDED(rv)) {
                          if (type.LowerCaseEqualsLiteral("password")) {
                            passwordCount++;
                          }
                        }
                      }
                    }
                  }
                  /* don't prefill if there were no passwords on the form */
                  if (passwordCount == 0) {
                    continue;
                  }
                  for (PRUint32 elementX = 0; elementX < numElements; elementX++) {
                    nsCOMPtr<nsIDOMNode> elementNode;
                    elements->Item(elementX, getter_AddRefs(elementNode));
                    if (nsnull != elementNode) {
                      nsCOMPtr<nsIDOMHTMLInputElement> inputElement(do_QueryInterface(elementNode));
                      if ((NS_SUCCEEDED(rv)) && (nsnull != inputElement)) {
                        nsAutoString type;
                        rv = inputElement->GetType(type);
                        if (NS_SUCCEEDED(rv)) {
                          if (type.IsEmpty() ||
                              type.LowerCaseEqualsLiteral("text") ||
                              type.LowerCaseEqualsLiteral("password")) {
                            nsAutoString field;
                            rv = inputElement->GetName(field);
                            if (NS_SUCCEEDED(rv)) {
                              PRUnichar* nameString = ToNewUnicode(field);
                              if (nameString) {
                                nsAutoString value;
                                PRUnichar* valueString = NULL;
                                nsCOMPtr<nsIPrompt> prompter;

                                nsCOMPtr<nsIChannel> channel = do_QueryInterface(aRequest);
                                if (channel)
                                  NS_QueryNotificationCallbacks(channel, prompter);
                                if (!prompter) {
                                  nsCOMPtr<nsIWindowWatcher> wwatch(do_GetService(NS_WINDOWWATCHER_CONTRACTID));
                                  if (wwatch)
                                    wwatch->GetNewPrompter(0, getter_AddRefs(prompter));
                                }
                                if (prompter) {
                                   SINGSIGN_RestoreSignonData(prompter, uri, nameString, &valueString, formX, elementNumber++);
                                }
                                if (valueString) {
                                  value = valueString;
                                  rv = inputElement->SetValue(value);
                                  // warning! don't delete valueString
                                }
                                Recycle(nameString);
                              }
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
          if (expireMasterPassword) {
            PRBool status;
            WLLT_ExpirePasswordOnly(&status);
          }
        }
    }
    return rv;
}

NS_IMETHODIMP
nsWalletlibService::OnProgressChange(nsIWebProgress *aWebProgress,
                                     nsIRequest *aRequest,
                                     PRInt32 aCurSelfProgress,
                                     PRInt32 aMaxSelfProgress,
                                     PRInt32 aCurTotalProgress,
                                     PRInt32 aMaxTotalProgress)
{
    NS_NOTREACHED("notification excluded in AddProgressListener(...)");
    return NS_OK;
}

NS_IMETHODIMP
nsWalletlibService::OnLocationChange(nsIWebProgress* aWebProgress,
                                     nsIRequest* aRequest,
                                     nsIURI *location)
{
    NS_NOTREACHED("notification excluded in AddProgressListener(...)");
    return NS_OK;
}

NS_IMETHODIMP
nsWalletlibService::OnStatusChange(nsIWebProgress* aWebProgress,
                                   nsIRequest* aRequest,
                                   nsresult aStatus,
                                   const PRUnichar* aMessage)
{
    NS_NOTREACHED("notification excluded in AddProgressListener(...)");
    return NS_OK;
}

NS_IMETHODIMP
nsWalletlibService::OnSecurityChange(nsIWebProgress *aWebProgress,
                                     nsIRequest *aRequest,
                                     PRUint32 state)
{
    NS_NOTREACHED("notification excluded in AddProgressListener(...)");
    return NS_OK;
}

NS_IMETHODIMP
nsWalletlibService::HaveData(nsIPrompt* dialog, const char *key, const PRUnichar *userName, PRBool *_retval)
{
  return ::SINGSIGN_HaveData(dialog, key, userName, _retval);
}

NS_IMETHODIMP
nsWalletlibService::WALLET_Encrypt (const PRUnichar *text, char **crypt) {
  nsAutoString textAutoString( text );
  nsAutoString cryptAutoString;
  PRBool rv = ::Wallet_Encrypt(textAutoString, cryptAutoString);
  *crypt = ToNewCString(cryptAutoString);
  return rv;
}

NS_IMETHODIMP
nsWalletlibService::WALLET_Decrypt (const char *crypt, PRUnichar **text) {
  nsAutoString cryptAutoString; cryptAutoString.AssignASCII(crypt);
  nsAutoString textAutoString;
  PRBool rv = ::Wallet_Decrypt(cryptAutoString, textAutoString);
  *text = ToNewUnicode(textAutoString);
  return rv;
}

NS_IMETHODIMP
nsWalletlibService::GetPrompt(nsIDOMWindow* aParent,
                              const nsIID& aIID,
                              void** _retval) {
  if (!aIID.Equals(NS_GET_IID(nsIAuthPrompt2))) {
    NS_WARNING("Wallet asked for unexpected interface");
    return NS_NOINTERFACE;
  }

  // NOTE: It is important to return the specific return value here. The
  // caller cares.
  nsresult rv;
  nsCOMPtr<nsIPromptService2> service =
    do_GetService(NS_PROMPTSERVICE_CONTRACTID, &rv);
  if (NS_FAILED(rv))
    return rv;

  nsWalletAuthPromptWrapper* wrapper =
    new nsWalletAuthPromptWrapper(service, aParent);
  if (!wrapper)
    return NS_ERROR_OUT_OF_MEMORY;

  NS_ADDREF(wrapper);
  *_retval = static_cast<nsIAuthPrompt2*>(wrapper);
  return NS_OK;
}

////////////////////////////////////////////////////////////////////////////////
// nsWalletAuthPromptWrapper

NS_IMPL_ISUPPORTS1(nsWalletAuthPromptWrapper, nsIAuthPrompt2)

NS_IMETHODIMP
nsWalletAuthPromptWrapper::PromptAuth(nsIChannel* aChannel,
                                      PRUint32 aLevel,
                                      nsIAuthInformation* aAuthInfo,
                                      PRBool* retval)
{
  return SINGSIGN_PromptAuth(mService, mParent, aChannel,
                             aLevel, aAuthInfo, retval);
}

NS_IMETHODIMP
nsWalletAuthPromptWrapper::AsyncPromptAuth(nsIChannel*,
                                           nsIAuthPromptCallback*,
                                           nsISupports*,
                                           PRUint32,
                                           nsIAuthInformation*,
                                           nsICancelable**)
{
  return NS_ERROR_NOT_IMPLEMENTED;
}



////////////////////////////////////////////////////////////////////////////////
// nsSingleSignOnPrompt

NS_IMPL_THREADSAFE_ISUPPORTS2(nsSingleSignOnPrompt,
                              nsIAuthPromptWrapper,
                              nsIAuthPrompt)

nsresult
nsSingleSignOnPrompt::Init()
{
  return NS_OK;
}

NS_IMETHODIMP
nsSingleSignOnPrompt::Prompt(const PRUnichar *dialogTitle, const PRUnichar *text,
                             const PRUnichar *passwordRealm, PRUint32 savePassword,
                             const PRUnichar *defaultText, PRUnichar **result, PRBool *_retval)
{
  nsresult rv;
  rv = SINGSIGN_Prompt(
    dialogTitle, text, defaultText, result,
    NS_ConvertUTF16toUTF8(passwordRealm).get(), mPrompt, _retval, savePassword);
  return rv;
}

NS_IMETHODIMP
nsSingleSignOnPrompt::PromptUsernameAndPassword(const PRUnichar *dialogTitle, const PRUnichar *text,
                                                const PRUnichar *passwordRealm, PRUint32 savePassword,
                                                PRUnichar **user, PRUnichar **pwd, PRBool *_retval)
{
  nsresult rv;
  rv = SINGSIGN_PromptUsernameAndPassword(
    dialogTitle, text, user, pwd,
    NS_ConvertUTF16toUTF8(passwordRealm).get(), mPrompt, _retval, savePassword);
  return rv;
}

NS_IMETHODIMP
nsSingleSignOnPrompt::PromptPassword(const PRUnichar *dialogTitle, const PRUnichar *text,
                                     const PRUnichar *passwordRealm, PRUint32 savePassword,
                                     PRUnichar **pwd, PRBool *_retval)
{
  nsresult rv;
  rv = SINGSIGN_PromptPassword(
    dialogTitle, text, pwd,
    NS_ConvertUTF16toUTF8(passwordRealm).get(), mPrompt, _retval, savePassword);
  return rv;
}

// nsISingleSignOnPrompt methods:

NS_IMETHODIMP
nsSingleSignOnPrompt::SetPromptDialogs(nsIPrompt* dialogs)
{
  mPrompt = dialogs;
  return NS_OK;
}

////////////////////////////////////////////////////////////////////////////////
