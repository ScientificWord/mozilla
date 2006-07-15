/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include "TestCommon.h"
#ifdef WIN32 
#include <windows.h>
#endif

#include "nsIComponentRegistrar.h"
#include "nsIIOService.h"
#include "nsIServiceManager.h"
#include "nsNetUtil.h"

#include "nsIUploadChannel.h"

static NS_DEFINE_CID(kIOServiceCID, NS_IOSERVICE_CID);

#include "prlog.h"
#if defined(PR_LOGGING)
//
// set NSPR_LOG_MODULES=Test:5
//
static PRLogModuleInfo *gTestLog = nsnull;
#endif
#define LOG(args) PR_LOG(gTestLog, PR_LOG_DEBUG, args)

//-----------------------------------------------------------------------------
// InputTestConsumer
//-----------------------------------------------------------------------------

class InputTestConsumer : public nsIStreamListener
{
public:

  InputTestConsumer();
  virtual ~InputTestConsumer();

  NS_DECL_ISUPPORTS
  NS_DECL_NSIREQUESTOBSERVER
  NS_DECL_NSISTREAMLISTENER
};

InputTestConsumer::InputTestConsumer()
{
}

InputTestConsumer::~InputTestConsumer()
{
}

NS_IMPL_ISUPPORTS1(InputTestConsumer, nsIStreamListener)

NS_IMETHODIMP
InputTestConsumer::OnStartRequest(nsIRequest *request, nsISupports* context)
{
  LOG(("InputTestConsumer::OnStartRequest\n"));
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
  PRUint32 amt, size;
  nsresult rv;

  while (aLength) {
    size = PR_MIN(aLength, sizeof(buf));
    rv = aIStream->Read(buf, size, &amt);
    if (NS_FAILED(rv)) {
      NS_ASSERTION((NS_BASE_STREAM_WOULD_BLOCK != rv), 
                   "The stream should never block.");
      return rv;
    }
    aLength -= amt;
  }
  return NS_OK;
}


NS_IMETHODIMP
InputTestConsumer::OnStopRequest(nsIRequest *request, nsISupports* context,
                                 nsresult aStatus)
{
    LOG(("InputTestConsumer::OnStopRequest [status=%x]\n", aStatus));
    QuitPumpingEvents();
    return NS_OK;
}


int
main(int argc, char* argv[])
{
    if (test_common_init(&argc, &argv) != 0)
        return -1;

    nsresult rv;

    if (argc < 2) {
        printf("usage: %s <url> <file-to-upload>\n", argv[0]);
        return -1;
    }
    char* uriSpec  = argv[1];
    char* fileName = argv[2];

#if defined(PR_LOGGING) 
    gTestLog = PR_NewLogModule("Test");
#endif

    {
        nsCOMPtr<nsIServiceManager> servMan;
        NS_InitXPCOM2(getter_AddRefs(servMan), nsnull, nsnull);
        nsCOMPtr<nsIComponentRegistrar> registrar = do_QueryInterface(servMan);
        NS_ASSERTION(registrar, "Null nsIComponentRegistrar");
        if (registrar)
            registrar->AutoRegister(nsnull);

        nsCOMPtr<nsIIOService> ioService(do_GetService(kIOServiceCID, &rv));
        // first thing to do is create ourselves a stream that
        // is to be uploaded.
        nsCOMPtr<nsIInputStream> uploadStream;
        rv = NS_NewPostDataStream(getter_AddRefs(uploadStream),
                                  PR_TRUE,
                                  nsDependentCString(fileName), // XXX UTF-8
                                  0, ioService);
        if (NS_FAILED(rv)) return rv;

        // create our url.
        nsCOMPtr<nsIURI> uri;
        rv = NS_NewURI(getter_AddRefs(uri), uriSpec);
        if (NS_FAILED(rv)) return rv;

        nsCOMPtr<nsIChannel> channel;
        rv = ioService->NewChannelFromURI(uri, getter_AddRefs(channel));
        if (NS_FAILED(rv)) return rv;
	
        // QI and set the upload stream
        nsCOMPtr<nsIUploadChannel> uploadChannel(do_QueryInterface(channel));
        uploadChannel->SetUploadStream(uploadStream, EmptyCString(), -1);

        // create a dummy listener
        InputTestConsumer* listener;

        listener = new InputTestConsumer;
        if (!listener) {
            NS_ERROR("Failed to create a new stream listener!");
            return -1;
        }
        NS_ADDREF(listener);

        channel->AsyncOpen(listener, nsnull);

        PumpEvents();
    } // this scopes the nsCOMPtrs
    // no nsCOMPtrs are allowed to be alive when you call NS_ShutdownXPCOM
    rv = NS_ShutdownXPCOM(nsnull);
    NS_ASSERTION(NS_SUCCEEDED(rv), "NS_ShutdownXPCOM failed");

    return 0;
}

