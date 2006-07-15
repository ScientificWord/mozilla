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
 * Portions created by the Initial Developer are Copyright (C) 1999
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
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

#ifdef MOZ_LOGGING
#define FORCE_PR_LOG /* Allow logging in the release build */
#endif

#include "nsMsgBiffManager.h"
#include "nsCRT.h"
#include "nsIMsgMailSession.h"
#include "nsIMsgAccountManager.h"
#include "nsMsgBaseCID.h"
#include "nsIObserverService.h"
#include "nsStatusBarBiffManager.h"
#include "nsCOMArray.h"
#include "prlog.h"

static NS_DEFINE_CID(kStatusBarBiffManagerCID, NS_STATUSBARBIFFMANAGER_CID);

static PRLogModuleInfo *MsgBiffLogModule = nsnull;

NS_IMPL_ISUPPORTS4(nsMsgBiffManager, nsIMsgBiffManager, nsIIncomingServerListener, nsIObserver, nsISupportsWeakReference)

void OnBiffTimer(nsITimer *timer, void *aBiffManager)
{
  nsMsgBiffManager *biffManager = (nsMsgBiffManager*)aBiffManager;
  biffManager->PerformBiff();		
}

nsMsgBiffManager::nsMsgBiffManager()
{
  mBiffArray = nsnull;
  mHaveShutdown = PR_FALSE;
  mInited = PR_FALSE;
}

nsMsgBiffManager::~nsMsgBiffManager()
{
  
  if (mBiffTimer) {
    mBiffTimer->Cancel();
  }
  
  PRInt32 count = mBiffArray->Count();
  PRInt32 i;
  for(i=0; i < count; i++)
  {
    nsBiffEntry *biffEntry = (nsBiffEntry*)mBiffArray->ElementAt(i);
    delete biffEntry;
  }
  delete mBiffArray;
  
  if(!mHaveShutdown)
  {
    Shutdown();
  }
}

NS_IMETHODIMP nsMsgBiffManager::Init()
{
  if (mInited)
    return NS_OK;

  mInited = PR_TRUE;
  nsresult rv;
  
  nsCOMPtr<nsIMsgAccountManager> accountManager = 
  do_GetService(NS_MSGACCOUNTMANAGER_CONTRACTID, &rv);
  if (NS_SUCCEEDED(rv))
  {
    accountManager->AddIncomingServerListener(this);
  }

  if(mHaveShutdown) //in turbo mode on profile change we don't need to do anything below this
  {
    mHaveShutdown = PR_FALSE;
    return NS_OK;
  }

  mBiffArray = new nsVoidArray();
  if(!mBiffArray)
    return NS_ERROR_OUT_OF_MEMORY;
  
  nsCOMPtr<nsIObserverService> observerService = 
    do_GetService("@mozilla.org/observer-service;1", &rv);
  if (NS_SUCCEEDED(rv))
  {    
    observerService->AddObserver(this, NS_XPCOM_SHUTDOWN_OBSERVER_ID, PR_TRUE);
  }
  
  
  //Ensure status bar biff service has started
  nsCOMPtr<nsIFolderListener> statusBarBiffService = 
    do_GetService(kStatusBarBiffManagerCID, &rv);
  
  if (!MsgBiffLogModule)
    MsgBiffLogModule = PR_NewLogModule("MsgBiff");

  return NS_OK;
}

NS_IMETHODIMP nsMsgBiffManager::Shutdown()
{
  if (mBiffTimer) 
  {
    mBiffTimer->Cancel();
    mBiffTimer = nsnull;
  }
  nsresult rv;
  nsCOMPtr<nsIMsgAccountManager> accountManager = do_GetService(NS_MSGACCOUNTMANAGER_CONTRACTID, &rv);
  if (NS_SUCCEEDED(rv))
  {
    accountManager->RemoveIncomingServerListener(this);
  }
  
  mHaveShutdown = PR_TRUE;
  mInited = PR_FALSE;
  return NS_OK;
}


NS_IMETHODIMP nsMsgBiffManager::AddServerBiff(nsIMsgIncomingServer *server)
{
  nsresult rv;
  PRInt32 biffMinutes;
  
  rv = server->GetBiffMinutes(&biffMinutes);
  if(NS_FAILED(rv))
    return rv;
  
  //Don't add if biffMinutes isn't > 0
  if(biffMinutes > 0)
  {
    PRInt32 serverIndex = FindServer(server);
    //Only add it if it hasn't been added already.
    if(serverIndex == -1)
    {
      nsBiffEntry *biffEntry = new nsBiffEntry;
      if(!biffEntry)
        return NS_ERROR_OUT_OF_MEMORY;
      biffEntry->server = server;
      nsTime currentTime;
      rv = SetNextBiffTime(biffEntry, currentTime);
      if(NS_FAILED(rv))
        return rv;
      
      AddBiffEntry(biffEntry);
      SetupNextBiff();
    }
  }
  return NS_OK;
}

NS_IMETHODIMP nsMsgBiffManager::RemoveServerBiff(nsIMsgIncomingServer *server)
{
  PRInt32 pos = FindServer(server);
  if(pos != -1)
  {
    nsBiffEntry *biffEntry = (nsBiffEntry*)mBiffArray->ElementAt(pos);
    mBiffArray->RemoveElementAt(pos);
    delete biffEntry;
  }
  
  //Should probably reset biff time if this was the server that gets biffed next.
	return NS_OK;
}


NS_IMETHODIMP nsMsgBiffManager::ForceBiff(nsIMsgIncomingServer *server)
{
  return NS_OK;
}

NS_IMETHODIMP nsMsgBiffManager::ForceBiffAll()
{
  return NS_OK;
}

NS_IMETHODIMP nsMsgBiffManager::OnServerLoaded(nsIMsgIncomingServer *server)
{
  nsresult rv;
  PRBool doBiff = PR_FALSE;
  
  rv = server->GetDoBiff(&doBiff);
  
  if(NS_SUCCEEDED(rv) && doBiff)
  {
    rv = AddServerBiff(server);
  }
  
  return rv;
}

NS_IMETHODIMP nsMsgBiffManager::OnServerUnloaded(nsIMsgIncomingServer *server)
{
  return RemoveServerBiff(server);
}

NS_IMETHODIMP nsMsgBiffManager::OnServerChanged(nsIMsgIncomingServer *server)
{
  // nothing required.  If the hostname or username changed
  // the next time biff fires, we'll ping the right server
  return NS_OK;
}

NS_IMETHODIMP nsMsgBiffManager::Observe(nsISupports *aSubject, const char *aTopic, const PRUnichar *someData)
{
  if(!nsCRT::strcmp(aTopic, NS_XPCOM_SHUTDOWN_OBSERVER_ID))
  {
    Shutdown();
  }
  
  return NS_OK;
}

PRInt32 nsMsgBiffManager::FindServer(nsIMsgIncomingServer *server)
{
  PRInt32 count = mBiffArray->Count();
  for(PRInt32 i = 0; i < count; i++)
  {
    nsBiffEntry *biffEntry = (nsBiffEntry*)mBiffArray->ElementAt(i);
    if(server == biffEntry->server.get())
      return i;
  }
  return -1;
}

nsresult nsMsgBiffManager::AddBiffEntry(nsBiffEntry *biffEntry)
{
  PRInt32 i;
  PRInt32 count = mBiffArray->Count();
  for(i = 0; i < count; i++)
  {
    nsBiffEntry *current = (nsBiffEntry*)mBiffArray->ElementAt(i);
    if(biffEntry->nextBiffTime < current->nextBiffTime)
      break;
    
  }
  PR_LOG(MsgBiffLogModule, PR_LOG_ALWAYS, ("inserting biff entry at %d\n", i));
  mBiffArray->InsertElementAt(biffEntry, i);
  return NS_OK;
}

nsresult nsMsgBiffManager::SetNextBiffTime(nsBiffEntry *biffEntry, nsTime startTime)
{
  nsresult rv;
  nsIMsgIncomingServer *server = biffEntry->server;
  
  if(!server)
    return NS_ERROR_FAILURE;
  
  PRInt32 biffInterval;
  rv = server->GetBiffMinutes(&biffInterval);
  if(NS_FAILED(rv))
    return rv;
  //Add 60 secs/minute in microseconds to current time. biffEntry->nextBiffTime's
  //constructor makes it the current time.
  nsInt64 chosenTimeInterval = biffInterval;
  chosenTimeInterval *= 60000000;
  biffEntry->nextBiffTime = startTime;
  biffEntry->nextBiffTime += chosenTimeInterval;
  return NS_OK;
}

nsresult nsMsgBiffManager::SetupNextBiff()
{
  
  if(mBiffArray->Count() > 0)
  {
    
    //Get the next biff entry
    nsBiffEntry *biffEntry = (nsBiffEntry*)mBiffArray->ElementAt(0);
    nsTime currentTime;
    nsInt64 biffDelay;
    nsInt64 ms(1000);
    if(currentTime > biffEntry->nextBiffTime)
    {
      PRInt64 microSecondsPerSecond;
  
      LL_I2L(microSecondsPerSecond, PR_USEC_PER_SEC);
      LL_MUL(biffDelay, 30, microSecondsPerSecond); //let's wait 30 seconds before firing biff again
    }
    else
      biffDelay = biffEntry->nextBiffTime - currentTime;
    //Convert biffDelay into milliseconds
    nsInt64 timeInMS = biffDelay / ms;
    PRUint32 timeInMSUint32 = (PRUint32)timeInMS;
    //Can't currently reset a timer when it's in the process of
    //calling Notify. So, just release the timer here and create a new one.
    if(mBiffTimer)
    {
      mBiffTimer->Cancel();
    }
    PR_LOG(MsgBiffLogModule, PR_LOG_ALWAYS, ("setting %d timer\n", timeInMSUint32));
    mBiffTimer = do_CreateInstance("@mozilla.org/timer;1");
    mBiffTimer->InitWithFuncCallback(OnBiffTimer, (void*)this, timeInMSUint32, 
                                     nsITimer::TYPE_ONE_SHOT);
    
  }
  return NS_OK;
}

//This is the function that does a biff on all of the servers whose time it is to biff.
nsresult nsMsgBiffManager::PerformBiff()
{
  nsTime currentTime;
  nsCOMArray <nsIMsgFolder> targetFolders;
  PR_LOG(MsgBiffLogModule, PR_LOG_ALWAYS, ("performing biffs\n"));

  for(PRInt32 i = 0; i < mBiffArray->Count(); i++)
  {
    nsBiffEntry *current = (nsBiffEntry*)mBiffArray->ElementAt(i);
    if(current->nextBiffTime < currentTime)
    {
      PRBool serverBusy = PR_FALSE;
      PRBool serverRequiresPassword = PR_TRUE;
      PRBool passwordPromptRequired; 

      current->server->GetPasswordPromptRequired(&passwordPromptRequired);
      current->server->GetServerBusy(&serverBusy);
      current->server->GetServerRequiresPasswordForBiff(&serverRequiresPassword);
      // find the dest folder we're actually downloading to...
      nsCOMPtr<nsIMsgFolder> rootMsgFolder;
      current->server->GetRootMsgFolder(getter_AddRefs(rootMsgFolder));
      PRInt32 targetFolderIndex = targetFolders.IndexOfObject(rootMsgFolder);
      if (targetFolderIndex == kNotFound)
        targetFolders.AppendObject(rootMsgFolder);

      // so if we need to be authenticated to biff, check that we are
      // (since we don't want to prompt the user for password UI)
      // and make sure the server isn't already in the middle of downloading new messages
      if(!serverBusy && (!serverRequiresPassword || !passwordPromptRequired) && targetFolderIndex == kNotFound)
      {
        nsXPIDLCString serverKey;
        current->server->GetKey(getter_Copies(serverKey));
        nsresult rv = current->server->PerformBiff(nsnull);
        PR_LOG(MsgBiffLogModule, PR_LOG_ALWAYS, ("biffing server %s rv = %x\n", serverKey.get(), rv));
      }
      else
      {
        PR_LOG(MsgBiffLogModule, PR_LOG_ALWAYS, ("not biffing server serverBusy = %d requirespassword = %d password prompt required = %d targetFolderIndex = %d\n",
          serverBusy, serverRequiresPassword, passwordPromptRequired, targetFolderIndex));
      }
      // if we didn't do this server because the destination server was already being
      // biffed into, leave this server in the biff array so it will fire next.
      if (targetFolderIndex == kNotFound)
      {
        mBiffArray->RemoveElementAt(i);
        i--; //Because we removed it we need to look at the one that just moved up.
        SetNextBiffTime(current, currentTime);
        AddBiffEntry(current);
      }
#ifdef DEBUG_David_Bienvenu
      else
        printf("dest account performing biff\n");
#endif
    }
    else
      //since we're in biff order, there's no reason to keep checking
      break;
  }
  SetupNextBiff();
  return NS_OK;
}

