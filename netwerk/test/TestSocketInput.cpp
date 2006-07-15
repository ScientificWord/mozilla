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
#include <stdio.h>

#ifdef WIN32
#include <windows.h>
#endif
#ifdef XP_OS2
#include <os2.h>
#endif

#include "nscore.h"
#include "nsCOMPtr.h"
#include "nsISocketTransportService.h"
#include "nsIEventQueueService.h"
#include "nsIServiceManager.h"
#include "nsIComponentRegistrar.h"
#include "nsITransport.h"
#include "nsIRequest.h"
#include "nsIStreamListener.h"
#include "nsIInputStream.h"

static NS_DEFINE_CID(kSocketTransportServiceCID, NS_SOCKETTRANSPORTSERVICE_CID);
static NS_DEFINE_CID(kEventQueueServiceCID,      NS_EVENTQUEUESERVICE_CID);

static int gKeepRunning = 1;

class InputTestConsumer : public nsIStreamListener
{
public:

  InputTestConsumer();
  virtual ~InputTestConsumer();

  // ISupports interface...
  NS_DECL_ISUPPORTS

  // IStreamListener interface...
  NS_IMETHOD OnStartRequest(nsIRequest *request, nsISupports* context);

  NS_IMETHOD OnDataAvailable(nsIRequest *request, nsISupports* context,
                             nsIInputStream *aIStream, 
                             PRUint32 aSourceOffset,
                             PRUint32 aLength);

  NS_IMETHOD OnStopRequest(nsIRequest *request, nsISupports* context,
                           nsresult aStatus);

};


InputTestConsumer::InputTestConsumer()
{
}

InputTestConsumer::~InputTestConsumer()
{
}


NS_IMPL_ISUPPORTS2(InputTestConsumer, nsIRequestObserver, nsIStreamListener)


NS_IMETHODIMP
InputTestConsumer::OnStartRequest(nsIRequest *request, nsISupports* context)
{
  printf("+++ OnStartRequest +++\n");
  return NS_OK;
}


NS_IMETHODIMP
InputTestConsumer::OnDataAvailable(nsIRequest *request, 
                                   nsISupports* context,
                                   nsIInputStream *aIStream, 
                                   PRUint32 aSourceOffset,
                                   PRUint32 aLength)
{
  char buf[1025];
  while (aLength > 0) {
    PRUint32 amt;
    aIStream->Read(buf, 1024, &amt);
    if (amt == 0) break;
    buf[amt] = '\0';
    printf(buf);
    aLength -= amt;
  }

  return NS_OK;
}


NS_IMETHODIMP
InputTestConsumer::OnStopRequest(nsIRequest *request, nsISupports* context,
                                 nsresult aStatus)
{
  gKeepRunning = 0;
  printf("+++ OnStopRequest status %x +++\n", aStatus);
  return NS_OK;
}


int
main(int argc, char* argv[])
{
  nsresult rv;

  if (argc < 2) {
      printf("usage: %s <host>\n", argv[0]);
      return -1;
  }

  int port;
  char* hostName = argv[1];
//nsString portString(argv[2]);

//port = portString.ToInteger(&rv);
  port = 13;
  {
    nsCOMPtr<nsIServiceManager> servMan;
    NS_InitXPCOM2(getter_AddRefs(servMan), nsnull, nsnull);
    nsCOMPtr<nsIComponentRegistrar> registrar = do_QueryInterface(servMan);
    NS_ASSERTION(registrar, "Null nsIComponentRegistrar");
    if (registrar)
      registrar->AutoRegister(nsnull);

    // Create the Event Queue for this thread...
    nsCOMPtr<nsIEventQueueService> eventQService =
             do_GetService(kEventQueueServiceCID, &rv);
    if (NS_FAILED(rv)) return rv;

    nsCOMPtr<nsIEventQueue> eventQ;
    rv = eventQService->GetThreadEventQueue(NS_CURRENT_THREAD, getter_AddRefs(eventQ));
    if (NS_FAILED(rv)) return rv;

    nsCOMPtr<nsISocketTransportService> sts =
             do_GetService(kSocketTransportServiceCID, &rv);
    if (NS_FAILED(rv)) return rv;

    nsITransport* transport;

    rv = sts->CreateTransport(hostName, port, nsnull, 0, 0, &transport);
    if (NS_SUCCEEDED(rv)) {
      nsCOMPtr<nsIRequest> request;
      transport->AsyncRead(new InputTestConsumer, nsnull, 0, -1, 0, getter_AddRefs(request));

      NS_RELEASE(transport);
    }

    // Enter the message pump to allow the URL load to proceed.
    while ( gKeepRunning ) {
      PLEvent *gEvent;
      eventQ->WaitForEvent(&gEvent);
      eventQ->HandleEvent(gEvent);
    }

  } // this scopes the nsCOMPtrs
  // no nsCOMPtrs are allowed to be alive when you call NS_ShutdownXPCOM
  rv = NS_ShutdownXPCOM(nsnull);
  NS_ASSERTION(NS_SUCCEEDED(rv), "NS_ShutdownXPCOM failed");
  return 0;
}

