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
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Kathleen Brade <brade@netscape.com>
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

#include "nsString.h"

#include "nsIController.h"
#include "nsIControllers.h"
#include "nsIObserver.h"

#include "nsIComponentManager.h"

#include "nsServiceManagerUtils.h"
#include "nsIScriptSecurityManager.h"

#include "nsIDOMDocument.h"
#include "nsIDOMWindow.h"
#include "nsPIDOMWindow.h"
#include "nsIDOMWindowInternal.h"
#include "nsIFocusController.h"

#include "nsSupportsArray.h"

#include "nsCommandManager.h"


nsCommandManager::nsCommandManager()
: mWindow(nsnull)
{
  /* member initializers and constructor code */
}

nsCommandManager::~nsCommandManager()
{
  /* destructor code */
}


NS_IMPL_ISUPPORTS3(nsCommandManager, nsICommandManager, nsPICommandUpdater, nsISupportsWeakReference)

#if 0
#pragma mark -
#endif

/* void init (in nsIDOMWindow aWindow); */
NS_IMETHODIMP
nsCommandManager::Init(nsIDOMWindow *aWindow)
{
  NS_ENSURE_ARG_POINTER(aWindow);
  
  NS_ASSERTION(aWindow, "Need non-null window here");
  mWindow = aWindow;      // weak ptr
  return NS_OK;
}

/* void commandStatusChanged (in DOMString aCommandName, in long aChangeFlags); */
NS_IMETHODIMP
nsCommandManager::CommandStatusChanged(const char * aCommandName)
{
	nsCStringKey hashKey(aCommandName);

  
  nsresult rv = NS_OK;
	nsCOMPtr<nsISupports>       commandSupports  = getter_AddRefs(mCommandObserversTable.Get(&hashKey));
	nsCOMPtr<nsISupportsArray>  commandObservers = do_QueryInterface(commandSupports);
	if (commandObservers)
	{
	  PRUint32  numItems;
	  rv = commandObservers->Count(&numItems);
	  if (NS_FAILED(rv)) return rv;
	  
	  for (PRUint32 i = 0; i < numItems; i ++)
	  {
	    nsCOMPtr<nsISupports>   itemSupports;
	    rv = commandObservers->GetElementAt(i, getter_AddRefs(itemSupports));
	    if (NS_FAILED(rv)) break;
      nsCOMPtr<nsIObserver>   itemObserver = do_QueryInterface(itemSupports);
      if (itemObserver)
      {
      	// should we get the command state to pass here? This might be expensive.
        itemObserver->Observe((nsICommandManager *)this, aCommandName,NS_LITERAL_STRING("command_status_changed").get());
      }
	  }
	}

  return NS_OK;
}

#if 0
#pragma mark -
#endif

/* void addCommandObserver (in nsIObserver aCommandObserver, in wstring aCommandToObserve); */
NS_IMETHODIMP
nsCommandManager::AddCommandObserver(nsIObserver *aCommandObserver, const char *aCommandToObserve)
{
	NS_ENSURE_ARG(aCommandObserver);
	
	nsresult rv = NS_OK;

	// XXX todo: handle special cases of aCommandToObserve being null, or empty
	
	// for each command in the table, we make a list of observers for that command
  nsCStringKey hashKey(aCommandToObserve);
	
	nsCOMPtr<nsISupports>       commandSupports  = getter_AddRefs(mCommandObserversTable.Get(&hashKey));
	nsCOMPtr<nsISupportsArray>  commandObservers = do_QueryInterface(commandSupports);
	if (!commandObservers)
	{
	  rv = NS_NewISupportsArray(getter_AddRefs(commandObservers));
  	if (NS_FAILED(rv)) return rv;
  	
  	commandSupports = do_QueryInterface(commandObservers);
  	rv = mCommandObserversTable.Put(&hashKey, commandSupports);
  	if (NS_FAILED(rv)) return rv;
	}
	
	// need to check that this command observer hasn't already been registered
	nsCOMPtr<nsISupports> observerAsSupports = do_QueryInterface(aCommandObserver);
	PRInt32 existingIndex = commandObservers->IndexOf(observerAsSupports);
  if (existingIndex == -1)
    rv = commandObservers->AppendElement(observerAsSupports);
  else
    NS_WARNING("Registering command observer twice on the same command");
  
  return rv;
}

/* void removeCommandObserver (in nsIObserver aCommandObserver, in wstring aCommandObserved); */
NS_IMETHODIMP
nsCommandManager::RemoveCommandObserver(nsIObserver *aCommandObserver, const char *aCommandObserved)
{
	NS_ENSURE_ARG(aCommandObserver);

	// XXX todo: handle special cases of aCommandToObserve being null, or empty
	nsCStringKey hashKey(aCommandObserved);

	nsCOMPtr<nsISupports>       commandSupports  = getter_AddRefs(mCommandObserversTable.Get(&hashKey));
	nsCOMPtr<nsISupportsArray>  commandObservers = do_QueryInterface(commandSupports);
	if (!commandObservers)
	  return NS_ERROR_UNEXPECTED;
	
	nsresult removed = commandObservers->RemoveElement(aCommandObserver);
	return (removed) ? NS_OK : NS_ERROR_FAILURE;
}

/* boolean isCommandSupported(in string aCommandName,
                              in nsIDOMWindow aTargetWindow); */
NS_IMETHODIMP
nsCommandManager::IsCommandSupported(const char *aCommandName,
                                     nsIDOMWindow *aTargetWindow,
                                     PRBool *outCommandSupported)
{
  NS_ENSURE_ARG_POINTER(outCommandSupported);

  nsCOMPtr<nsIController> controller;
  GetControllerForCommand(aCommandName, aTargetWindow, getter_AddRefs(controller)); 
  *outCommandSupported = (controller.get() != nsnull);
  return NS_OK;
}

/* boolean isCommandEnabled(in string aCommandName,
                            in nsIDOMWindow aTargetWindow); */
NS_IMETHODIMP
nsCommandManager::IsCommandEnabled(const char *aCommandName,
                                   nsIDOMWindow *aTargetWindow,
                                   PRBool *outCommandEnabled)
{
  NS_ENSURE_ARG_POINTER(outCommandEnabled);
  
  PRBool  commandEnabled = PR_FALSE;
  
  nsCOMPtr<nsIController> controller;
  GetControllerForCommand(aCommandName, aTargetWindow, getter_AddRefs(controller)); 
  if (controller)
  {
    controller->IsCommandEnabled(aCommandName, &commandEnabled);
  }
  *outCommandEnabled = commandEnabled;
  return NS_OK;
}

/* void getCommandState (in DOMString aCommandName,
                         in nsIDOMWindow aTargetWindow,
                         inout nsICommandParams aCommandParams); */
NS_IMETHODIMP
nsCommandManager::GetCommandState(const char *aCommandName,
                                  nsIDOMWindow *aTargetWindow,
                                  nsICommandParams *aCommandParams)
{
  nsCOMPtr<nsIController> controller;
  nsAutoString tValue;
  nsresult rv = GetControllerForCommand(aCommandName, aTargetWindow, getter_AddRefs(controller)); 
  if (!controller)
    return NS_ERROR_FAILURE;

  nsCOMPtr<nsICommandController>  commandController = do_QueryInterface(controller);
  if (commandController)
    rv = commandController->GetCommandStateWithParams(aCommandName, aCommandParams);
  else
    rv = NS_ERROR_NOT_IMPLEMENTED;
  return rv;
}

/* void doCommand(in string aCommandName,
                  in nsICommandParams aCommandParams,
                  in nsIDOMWindow aTargetWindow); */
NS_IMETHODIMP
nsCommandManager::DoCommand(const char *aCommandName,
                            nsICommandParams *aCommandParams,
                            nsIDOMWindow *aTargetWindow)
{
  nsCOMPtr<nsIController> controller;
  nsresult rv = GetControllerForCommand(aCommandName, aTargetWindow, getter_AddRefs(controller)); 
  if (!controller)
	  return NS_ERROR_FAILURE;

  nsCOMPtr<nsICommandController>  commandController = do_QueryInterface(controller);
  if (commandController && aCommandParams)
    rv = commandController->DoCommandWithParams(aCommandName, aCommandParams);
  else
    rv = controller->DoCommand(aCommandName);
  return rv;
}

#ifdef XP_MAC
#pragma mark -
#endif

nsresult
nsCommandManager::IsCallerChrome(PRBool *is_caller_chrome)
{
  *is_caller_chrome = PR_FALSE;
  nsresult rv = NS_OK;
  nsCOMPtr<nsIScriptSecurityManager> secMan = 
      do_GetService(NS_SCRIPTSECURITYMANAGER_CONTRACTID, &rv);
  if (NS_FAILED(rv))
    return rv;
  if (!secMan)
    return NS_ERROR_FAILURE;

  rv = secMan->SubjectPrincipalIsSystem(is_caller_chrome);
  return rv;
}

nsresult
nsCommandManager::GetControllerForCommand(const char *aCommand, 
                                          nsIDOMWindow *aTargetWindow,
                                          nsIController** outController)
{
  nsresult rv = NS_ERROR_FAILURE;
  *outController = nsnull;

  // check if we're in content or chrome
  // if we're not chrome we must have a target window or we bail
  PRBool isChrome = PR_FALSE;
  rv = IsCallerChrome(&isChrome);
  if (NS_FAILED(rv))
    return rv;

  if (!isChrome) {
    if (!aTargetWindow)
      return rv;

    // if a target window is specified, it must be the window we expect
    if (aTargetWindow != mWindow)
        return NS_ERROR_FAILURE;
  }

  if (aTargetWindow)
  {
    // get the controller for this particular window
    nsCOMPtr<nsIDOMWindowInternal> domWindowInternal = do_QueryInterface(aTargetWindow);
    if (!domWindowInternal)
      return NS_ERROR_FAILURE;

    nsCOMPtr<nsIControllers> controllers;
    rv = domWindowInternal->GetControllers(getter_AddRefs(controllers));
    if (NS_FAILED(rv))
      return rv;
    if (!controllers)
      return NS_ERROR_FAILURE;

    // dispatch the command
    return controllers->GetControllerForCommand(aCommand, outController);
  }


  // else we're not targeted to a particular window so use focus
  nsCOMPtr<nsPIDOMWindow> window = do_QueryInterface(mWindow);
  if (!window)
    return NS_ERROR_FAILURE;

  nsIFocusController *focusController = window->GetRootFocusController();
  if (!focusController)
    return NS_ERROR_FAILURE;

  nsCOMPtr<nsIDOMWindowInternal> focusWindowInternal;
  rv = focusController->GetFocusedWindow(getter_AddRefs(focusWindowInternal));
  if (NS_FAILED(rv))
    return rv;

  // get the destination window so we can check if it's in content or chrome
  nsCOMPtr<nsIDOMWindow> destWindow = do_QueryInterface(focusWindowInternal);
  if (!destWindow)
    return NS_ERROR_FAILURE;

  // no target window; send command to focus controller
  return focusController->GetControllerForCommand(aCommand, outController);
}

