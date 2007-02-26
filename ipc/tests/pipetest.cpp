/*
 * The contents of this file are subject to the Mozilla Public
 * License Version 1.1 (the "MPL"); you may not use this file
 * except in compliance with the MPL. You may obtain a copy of
 * the MPL at http://www.mozilla.org/MPL/
 *
 * Software distributed under the MPL is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the MPL for the specific language governing
 * rights and limitations under the MPL.
 *
 * The Original Code is protoZilla.
 *
 * The Initial Developer of the Original Code is Ramalingam Saravanan.
 * Portions created by Ramalingam Saravanan <svn@xmlterm.org> are
 * Copyright (C) 2000 Ramalingam Saravanan. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the
 * terms of the GNU General Public License (the "GPL"), in which case
 * the provisions of the GPL are applicable instead of
 * those above. If you wish to allow use of your version of this
 * file only under the terms of the GPL and not to allow
 * others to use your version of this file under the MPL, indicate
 * your decision by deleting the provisions above and replace them
 * with the notice and other provisions required by the GPL.
 * If you do not delete the provisions above, a recipient
 * may use your version of this file under either the MPL or the
 * GPL.
 */


//#define DEBUG_pipet 1

/**
 * A Test application that exercises the PipeTransport module.
 */


#include "../src/ipc.h"
#include "nspr.h"
#include "nsIRequestObserver.h"
#include "nsIStreamListener.h"
#include "nsIPipeTransport.h"
#include "nsIServiceManager.h"
#include "nsXPIDLString.h"
#include "nsIEventQueueService.h"
#include "nsIThread.h"
#include "nsIRunnable.h"
#include "nsIInputStream.h"
#include "nsIOutputStream.h"
#include "netCore.h"

PRUint32 gExited = 0;

// Helper class to handle polling of STDOUT pipe
class nsListenerImpl : public nsIStreamListener
{
public:
    NS_DECL_ISUPPORTS
    NS_DECL_NSIREQUESTOBSERVER
    NS_DECL_NSISTREAMLISTENER

    nsListenerImpl();
    virtual ~nsListenerImpl() {};

protected:

};

// nsListenerImpl implementation
nsListenerImpl::nsListenerImpl(void)
{
    NS_INIT_ISUPPORTS();
}


// nsISupports implementation
NS_IMPL_THREADSAFE_ISUPPORTS1 (nsListenerImpl, nsIStreamListener);

// nsIStreamListener implementation
NS_IMETHODIMP
nsListenerImpl::OnStartRequest(nsIRequest *aRequest, nsISupports *aContext) {
    printf("nsListenerImpl::OnStartRequest\n");
    return NS_OK;
}


NS_IMETHODIMP
nsListenerImpl::OnStopRequest(nsIRequest* aRequest, nsISupports* aContext,
                               nsresult aStatus)
{
    printf("nsListenerImpl::OnStopRequest\n");
    return NS_OK;
}


// nsIStreamListener method
NS_IMETHODIMP
nsListenerImpl::OnDataAvailable(nsIRequest* aRequest, nsISupports* aContext,
                               nsIInputStream *aInputStream, PRUint32 aSourceOffset,
                               PRUint32 aLength) {
  nsresult rv;

#ifdef DEBUG_pipet
  printf("nsListenerImpl::OnDataAvailable, offset=%d, length=%d\n",
         aSourceOffset, aLength);
#endif

  char buf[81];
  PRUint32 readCount = 0;

  while (aLength > 0) {

    rv = aInputStream->Read((char*) buf, 80, &readCount);

    if (NS_FAILED(rv) || (readCount <= 0))
      return NS_ERROR_FAILURE;

    buf[readCount] = '\0';
#ifdef DEBUG_pipet
    printf("READ(%d): ", readCount);
#endif
    printf("%s", buf);

    aLength -= readCount;
  }

  return NS_OK;
}


// Helper class to handle polling of STDOUT pipe
class nsConsolePoll : public nsIRunnable
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIRUNNABLE

  nsConsolePoll(nsIPipeTransport* aPipeT);
  virtual ~nsConsolePoll();

protected:
  nsCOMPtr<nsIPipeTransport> mPipeT;
};

// nsConsolePoll implementation

// nsISupports implementation
NS_IMPL_THREADSAFE_ISUPPORTS1 (nsConsolePoll, nsIRunnable);

// nsConsolePoll implementation
nsConsolePoll::nsConsolePoll(nsIPipeTransport* aPipeT)
{
    NS_INIT_ISUPPORTS();
    mPipeT = aPipeT;
}

nsConsolePoll::~nsConsolePoll()
{
  mPipeT = nsnull;
}

// nsIRunnable implementation
NS_IMETHODIMP
nsConsolePoll::Run()
{
  nsresult rv = NS_OK;

#ifdef DEBUG_pipet
  nsCOMPtr<nsIThread> myThread;
  rv = nsIThread::GetCurrent(getter_AddRefs(myThread));
  printf("nsConsolePoll::Run: myThread=%x\n", (int) myThread.get());
#endif

  nsCOMPtr<nsIOutputStream> stdinWrite;
  rv = mPipeT->OpenOutputStream(0, PRUint32(-1), 0,
                                getter_AddRefs(stdinWrite));
  if (NS_FAILED(rv)) return rv;

  printf("***Accepting console line input\n");

  char line[81];
  PRUint32 writeCount;

  for (;;) {
    fgets(line, 81, stdin);
    if (strstr(line, "quit") == line)
      break;
#ifdef DEBUG_pipet
    printf("CONSOLE: %s", line);
#endif
    rv = stdinWrite->Write(line, strlen(line), &writeCount);
    if (NS_FAILED(rv)) return rv;
  }

  gExited = 11;

  return rv;
}

main()
{
    nsresult rv;

    // Initialize XPCOM
    rv = NS_InitXPCOM2(nsnull, nsnull, nsnull);
    if (NS_FAILED(rv))
    {
        printf("ERROR: XPCOM intialization error [%x].\n", rv);
        return -1;
    }

    printf("pipetest: Creating event Q\n");

#ifdef _IPC_MOZILLA_1_8
    nsCOMPtr<nsIEventQueueService> service = do_GetService(NS_EVENTQUEUESERVICE_CONTRACTID, &rv);
    if (NS_FAILED(rv)) return rv;

    rv = service->CreateThreadEventQueue();
    if (NS_FAILED(rv)) return rv;

    nsCOMPtr<nsIEventQueue> currentThreadQ;
#else

#endif
    rv = service->GetThreadEventQueue(NS_CURRENT_THREAD,
                                  getter_AddRefs(currentThreadQ));
    if (NS_FAILED(rv)) return rv;

    (void) nsComponentManager::AutoRegister(nsIComponentManagerObsolete::NS_Startup, nsnull);

    // Create an instance of our component
    nsCOMPtr<nsIPipeTransport> mypipet = do_CreateInstance(NS_PIPETRANSPORT_CONTRACTID, &rv);
    if (NS_FAILED(rv)) {
        printf("pipetest: ERROR: creating PipeTransport [%x]\n", rv);
        return -2;
    }

    // Call methods on our component to test it out.
    printf("pipetest: Testing PipeTransport interface\n");

    PRUint32 nArgs = 0;
#ifdef XP_WIN
    const char *executable = "bash";
    char **args = NULL;

#else
    const char *executable = "/bin/sh";
    char **args = NULL;
#endif

    rv = mypipet->Init(executable,
                       (const char **)args, nArgs,
                       NULL, 0,
                       0, "",
                       false, false, nsnull);
    if (NS_FAILED(rv)) {
        printf("pipetest: ERROR: Calling Init [%x]\n", rv);
        return -3;
    }

    // Call ReadStdout
    nsCOMPtr<nsIRequest> pipeRequest;
    nsIStreamListener* listener = new nsListenerImpl();
    rv = mypipet->AsyncRead( listener, (nsISupports*) nsnull,
                             0, PRUint32(-1), 0,
                             getter_AddRefs(pipeRequest) );
    if (NS_FAILED(rv)) {
        printf("pipetest: ERROR: Calling ReadStdout [%x]\n", rv);
        return -3;
    }

    // Console input helper class
    nsConsolePoll* consolePoll = new nsConsolePoll(mypipet);
    nsCOMPtr<nsIThread> consoleThread;

    // Spin up a new thread to handle STDOUT polling
    rv = NS_NewThread(getter_AddRefs(consoleThread),
                      NS_STATIC_CAST(nsIRunnable*, consolePoll));

    // Process events until we're finished.
    PLEvent *event;
    PRBool eventAvailable;
    PRIntervalTime sleepInterval = PR_MillisecondsToInterval(20);

    for (;;) {
      if (gExited) {
        // Kill process
        rv = pipeRequest->Cancel(NS_BINDING_ABORTED);
        if (gExited == 1) break;
        gExited--;
      }

      rv = currentThreadQ->EventAvailable(eventAvailable);
      if (NS_FAILED(rv)) return rv;

      if (eventAvailable) {
        rv = currentThreadQ->WaitForEvent(&event);
        if (NS_FAILED(rv)) return rv;

        rv = currentThreadQ->HandleEvent(event);
        if (NS_FAILED(rv)) return rv;

      } else {
        PR_Sleep(sleepInterval);
      }
    }

  if (NS_FAILED(rv)) {
      printf("pipetest: ERROR: Calling Kill [%x]\n", rv);
      return NS_ERROR_FAILURE;
  }

  printf("pipetest: Finished testing ....\n");
  mypipet = nsnull;

  printf("pipetest: Destroying event Q\n");
  rv = service->DestroyThreadEventQueue();
  if (NS_FAILED(rv)) return rv;

  // Shutdown XPCOM
  NS_ShutdownXPCOM(nsnull);
  return NS_OK;
}
