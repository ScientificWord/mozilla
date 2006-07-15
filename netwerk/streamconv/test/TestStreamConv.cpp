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

#include "nsIServiceManager.h"
#include "nsIComponentManager.h"
#include "nsIComponentRegistrar.h"
#include "nsIStreamConverterService.h"
#include "nsIStreamConverter.h"
#include "nsICategoryManager.h"
#include "nsIFactory.h"
#include "nsIStringStream.h"
#include "nsCOMPtr.h"
#include "nsNetUtil.h"
#include "nsThreadUtils.h"

#include "nspr.h"

#define ASYNC_TEST // undefine this if you want to test sycnronous conversion.

/////////////////////////////////
// Event pump setup
/////////////////////////////////
#ifdef XP_WIN
#include <windows.h>
#endif
#ifdef XP_OS2
#include <os2.h>
#endif

static int gKeepRunning = 0;
/////////////////////////////////
// Event pump END
/////////////////////////////////


/////////////////////////////////
// Test converters include
/////////////////////////////////
#include "Converters.h"

// CID setup
static NS_DEFINE_CID(kStreamConverterServiceCID, NS_STREAMCONVERTERSERVICE_CID);

////////////////////////////////////////////////////////////////////////
// EndListener - This listener is the final one in the chain. It
//   receives the fully converted data, although it doesn't do anything with
//   the data.
////////////////////////////////////////////////////////////////////////
class EndListener : public nsIStreamListener {
public:
    // nsISupports declaration
    NS_DECL_ISUPPORTS

    EndListener() {};

    // nsIStreamListener method
    NS_IMETHOD OnDataAvailable(nsIRequest* request, nsISupports *ctxt, nsIInputStream *inStr, 
                               PRUint32 sourceOffset, PRUint32 count)
    {
        nsresult rv;
        PRUint32 read, len;
        rv = inStr->Available(&len);
        if (NS_FAILED(rv)) return rv;

        char *buffer = (char*)nsMemory::Alloc(len + 1);
        if (!buffer) return NS_ERROR_OUT_OF_MEMORY;

        rv = inStr->Read(buffer, len, &read);
        buffer[len] = '\0';
        if (NS_SUCCEEDED(rv)) {
            printf("CONTEXT %p: Received %u bytes and the following data: \n %s\n\n", ctxt, read, buffer);
        }
        nsMemory::Free(buffer);

        return NS_OK;
    }

    // nsIRequestObserver methods
    NS_IMETHOD OnStartRequest(nsIRequest* request, nsISupports *ctxt) { return NS_OK; }

    NS_IMETHOD OnStopRequest(nsIRequest* request, nsISupports *ctxt, 
                             nsresult aStatus) { return NS_OK; }
};

NS_IMPL_ISUPPORTS1(EndListener, nsIStreamListener)
////////////////////////////////////////////////////////////////////////
// EndListener END
////////////////////////////////////////////////////////////////////////


nsresult SendData(const char * aData, nsIStreamListener* aListener, nsIRequest* request) {
    nsresult rv;

    nsCOMPtr<nsIStringInputStream> dataStream
      (do_CreateInstance("@mozilla.org/io/string-input-stream;1", &rv));
    NS_ENSURE_SUCCESS(rv, rv);

    rv = dataStream->SetData(aData, strlen(aData));
    NS_ENSURE_SUCCESS(rv, rv);

    PRUint32 avail;
    dataStream->Available(&avail);

    return aListener->OnDataAvailable(request, nsnull, dataStream, 0, avail);
}
#define SEND_DATA(x) SendData(x, converterListener, request)

int
main(int argc, char* argv[])
{
    nsresult rv;
    {
        nsCOMPtr<nsIServiceManager> servMan;
        NS_InitXPCOM2(getter_AddRefs(servMan), nsnull, nsnull);
        nsCOMPtr<nsIComponentRegistrar> registrar = do_QueryInterface(servMan);
        NS_ASSERTION(registrar, "Null nsIComponentRegistrar");
        if (registrar)
            registrar->AutoRegister(nsnull);
    
        nsCOMPtr<nsIThread> thread = do_GetCurrentThread();

        nsCOMPtr<nsICategoryManager> catman =
            do_GetService(NS_CATEGORYMANAGER_CONTRACTID, &rv);
        if (NS_FAILED(rv)) return rv;
        nsCString previous;

        ///////////////////////////////////////////
        // BEGIN - Stream converter registration
        //   All stream converters must register with the ComponentManager
        ///////////////////////////////////////////

        // these stream converters are just for testing. running this harness
        // from the dist/bin dir will also pickup converters registered
        // in other modules (necko converters for example).

        PRUint32 converterListSize = 7;
        const char *const converterList[] = {
            "?from=a/foo&to=b/foo",
            "?from=b/foo&to=c/foo",
            "?from=b/foo&to=d/foo",
            "?from=c/foo&to=d/foo",
            "?from=d/foo&to=e/foo",
            "?from=d/foo&to=f/foo",
            "?from=t/foo&to=k/foo",
        };

        TestConverterFactory *convFactory = new TestConverterFactory(kTestConverterCID, "TestConverter", NS_ISTREAMCONVERTER_KEY);
        nsCOMPtr<nsIFactory> convFactSup(do_QueryInterface(convFactory, &rv));
        if (NS_FAILED(rv)) return rv;

        for (PRUint32 count = 0; count < converterListSize; ++count) {
            // register the TestConverter with the component manager. One contractid registration
            // per conversion pair (from - to pair).
            nsCString contractID(NS_ISTREAMCONVERTER_KEY);
            contractID.Append(converterList[count]);
            rv = registrar->RegisterFactory(kTestConverterCID,
                                            "TestConverter",
                                            contractID.get(),
                                            convFactSup);
            if (NS_FAILED(rv)) return rv;
            rv = catman->AddCategoryEntry(NS_ISTREAMCONVERTER_KEY, converterList[count], "x",
                                            PR_TRUE, PR_TRUE, getter_Copies(previous));
            if (NS_FAILED(rv)) return rv;
        }

        nsCOMPtr<nsIStreamConverterService> StreamConvService =
                 do_GetService(kStreamConverterServiceCID, &rv);
        if (NS_FAILED(rv)) return rv;

        // Define the *from* content type and *to* content-type for conversion.
        static const char fromStr[] = "a/foo";
        static const char toStr[] = "c/foo";
    
#ifdef ASYNC_TEST
        // ASYNCHRONOUS conversion

        // Build up a channel that represents the content we're
        // starting the transaction with.
        //
        // sample multipart mixed content-type string:
        // "multipart/x-mixed-replacE;boundary=thisrandomstring"
#if 0
        nsCOMPtr<nsIChannel> channel;
        nsCOMPtr<nsIURI> dummyURI;
        rv = NS_NewURI(getter_AddRefs(dummyURI), "http://meaningless");
        if (NS_FAILED(rv)) return rv;

        rv = NS_NewInputStreamChannel(getter_AddRefs(channel),
                                      dummyURI,
                                      nsnull,   // inStr
                                      "text/plain", // content-type
                                      -1);      // XXX fix contentLength
        if (NS_FAILED(rv)) return rv;

        nsCOMPtr<nsIRequest> request(do_QueryInterface(channel));
#endif

        nsCOMPtr<nsIRequest> request;

        // setup a listener to receive the converted data. This guy is the end
        // listener in the chain, he wants the fully converted (toType) data.
        // An example of this listener in mozilla would be the DocLoader.
        nsIStreamListener *dataReceiver = new EndListener();
        NS_ADDREF(dataReceiver);

        // setup a listener to push the data into. This listener sits inbetween the
        // unconverted data of fromType, and the final listener in the chain (in this case
        // the dataReceiver.
        nsIStreamListener *converterListener = nsnull;
        rv = StreamConvService->AsyncConvertData(fromStr, toStr,
                                                 dataReceiver, nsnull, &converterListener);
        if (NS_FAILED(rv)) return rv;
        NS_RELEASE(dataReceiver);

        // at this point we have a stream listener to push data to, and the one
        // that will receive the converted data. Let's mimic On*() calls and get the conversion
        // going. Typically these On*() calls would be made inside their respective wrappers On*()
        // methods.
        rv = converterListener->OnStartRequest(request, nsnull);
        if (NS_FAILED(rv)) return rv;

        rv = SEND_DATA("aaa");
        if (NS_FAILED(rv)) return rv;

        rv = SEND_DATA("aaa");
        if (NS_FAILED(rv)) return rv;

        // Finish the request.
        rv = converterListener->OnStopRequest(request, nsnull, rv);
        if (NS_FAILED(rv)) return rv;

        NS_RELEASE(converterListener);
#else
        // SYNCHRONOUS conversion
        nsCOMPtr<nsIInputStream> convertedData;
        rv = StreamConvService->Convert(inputData, fromStr, toStr,
                                        nsnull, getter_AddRefs(convertedData));
        if (NS_FAILED(rv)) return rv;
#endif

        // Enter the message pump to allow the URL load to proceed.
        while ( gKeepRunning ) {
            if (!NS_ProcessNextEvent(thread))
                break;
        }
    } // this scopes the nsCOMPtrs
    // no nsCOMPtrs are allowed to be alive when you call NS_ShutdownXPCOM
    NS_ShutdownXPCOM(nsnull);
    return rv;
}
