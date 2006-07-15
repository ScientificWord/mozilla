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
 * The Original Code is Mozilla.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2002
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Darin Fisher <darin@netscape.com>
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
#include "nsIComponentRegistrar.h"
#include "nsIStreamTransportService.h"
#include "nsIAsyncInputStream.h"
#include "nsIProgressEventSink.h"
#include "nsIInterfaceRequestor.h"
#include "nsIInterfaceRequestorUtils.h"
#include "nsIProxyObjectManager.h"
#include "nsIRequest.h"
#include "nsIServiceManager.h"
#include "nsIComponentManager.h"
#include "nsCOMPtr.h"
#include "nsMemory.h"
#include "nsStringAPI.h"
#include "nsIFileStreams.h"
#include "nsIStreamListener.h"
#include "nsILocalFile.h"
#include "nsNetUtil.h"
#include "nsAutoLock.h"
#include "prlog.h"
#include "prenv.h"

////////////////////////////////////////////////////////////////////////////////

#if defined(PR_LOGGING)
//
// set NSPR_LOG_MODULES=Test:5
//
static PRLogModuleInfo *gTestLog = nsnull;
#endif
#define LOG(args) PR_LOG(gTestLog, PR_LOG_DEBUG, args)

////////////////////////////////////////////////////////////////////////////////

static NS_DEFINE_CID(kStreamTransportServiceCID, NS_STREAMTRANSPORTSERVICE_CID);

////////////////////////////////////////////////////////////////////////////////

#define CHUNK_SIZE 500

class MyCopier : public nsIInputStreamCallback
               , public nsIOutputStreamCallback
{
public:
    NS_DECL_ISUPPORTS

    MyCopier()
        : mLock(nsnull)
        , mInputCondition(NS_OK)
    {
    }

    virtual ~MyCopier()
    {
        if (mLock)
            PR_DestroyLock(mLock);
        if (mInput)
            mInput->Close();
        if (mOutput)
            mOutput->Close();
    }

    // called on any thread
    NS_IMETHOD OnInputStreamReady(nsIAsyncInputStream *inStr)
    {
        LOG(("OnInputStreamReady\n"));
        nsAutoLock lock(mLock);
        NS_ASSERTION(inStr == mInput, "unexpected stream");
        Process_Locked();
        return NS_OK;
    }

    // called on any thread
    NS_IMETHOD OnOutputStreamReady(nsIAsyncOutputStream *outStr)
    {
        LOG(("OnOutputStreamReady\n"));
        nsAutoLock lock(mLock);
        NS_ASSERTION(outStr == mOutput, "unexpected stream");
        Process_Locked();
        return NS_OK;
    }

    void Close_Locked()
    {
        LOG(("Close_Locked\n"));

        mOutput->Close();
        mOutput = 0;
        mInput->Close();
        mInput = 0;

        // post done copying event
        QuitPumpingEvents();
    }

    void Process_Locked()
    {
        while (1) {
            mInputCondition = NS_OK; // reset

            PRUint32 n;
            nsresult rv = mOutput->WriteSegments(FillOutputBuffer, this, CHUNK_SIZE, &n);
            if (NS_FAILED(rv) || (n == 0)) {
                if (rv == NS_BASE_STREAM_WOULD_BLOCK)
                    mOutput->AsyncWait(this, 0, 0, nsnull);
                else if (mInputCondition == NS_BASE_STREAM_WOULD_BLOCK)
                    mInput->AsyncWait(this, 0, 0, nsnull);
                else
                    Close_Locked();
                break;
            }
        }
    }

    nsresult AsyncCopy(nsITransport *srcTrans, nsITransport *destTrans)
    {
        mLock = PR_NewLock();
        if (!mLock)
            return NS_ERROR_OUT_OF_MEMORY;

        nsresult rv;

        nsCOMPtr<nsIInputStream> inStr;
        rv = srcTrans->OpenInputStream(0, 0, 0, getter_AddRefs(inStr));
        if (NS_FAILED(rv)) return rv;

        nsCOMPtr<nsIOutputStream> outStr;
        rv = destTrans->OpenOutputStream(0, 0, 0, getter_AddRefs(outStr));
        if (NS_FAILED(rv)) return rv;

        mInput = do_QueryInterface(inStr);
        mOutput = do_QueryInterface(outStr);

        return mInput->AsyncWait(this, 0, 0, nsnull);
    }

    static NS_METHOD FillOutputBuffer(nsIOutputStream *outStr,
                                      void *closure,
                                      char *buffer,
                                      PRUint32 offset,
                                      PRUint32 count,
                                      PRUint32 *countRead)
    {
        MyCopier *self = (MyCopier *) closure;

        nsresult rv = self->mInput->Read(buffer, count, countRead);
        if (NS_FAILED(rv))
            self->mInputCondition = rv;
        else if (*countRead == 0)
            self->mInputCondition = NS_BASE_STREAM_CLOSED;

        return self->mInputCondition;
    }

protected:
    PRLock                        *mLock;
    nsCOMPtr<nsIAsyncInputStream>  mInput;
    nsCOMPtr<nsIAsyncOutputStream> mOutput;
    nsresult                       mInputCondition;
};

NS_IMPL_THREADSAFE_ISUPPORTS2(MyCopier,
                              nsIInputStreamCallback,
                              nsIOutputStreamCallback)

////////////////////////////////////////////////////////////////////////////////

/**
 * asynchronously copy file.
 */
static nsresult
RunTest(nsIFile *srcFile, nsIFile *destFile)
{
    nsresult rv;

    LOG(("RunTest\n"));

    nsCOMPtr<nsIStreamTransportService> sts =
        do_GetService(kStreamTransportServiceCID, &rv);
    if (NS_FAILED(rv)) return rv;

    nsCOMPtr<nsIInputStream> srcStr;
    rv = NS_NewLocalFileInputStream(getter_AddRefs(srcStr), srcFile);
    if (NS_FAILED(rv)) return rv;

    nsCOMPtr<nsIOutputStream> destStr;
    rv = NS_NewLocalFileOutputStream(getter_AddRefs(destStr), destFile);
    if (NS_FAILED(rv)) return rv;

    nsCOMPtr<nsITransport> srcTransport;
    rv = sts->CreateInputTransport(srcStr, nsInt64(-1), nsInt64(-1), PR_TRUE,
                                   getter_AddRefs(srcTransport));
    if (NS_FAILED(rv)) return rv;

    nsCOMPtr<nsITransport> destTransport;
    rv = sts->CreateOutputTransport(destStr, nsInt64(-1), nsInt64(-1), PR_TRUE,
                                    getter_AddRefs(destTransport));
    if (NS_FAILED(rv)) return rv;

    MyCopier *copier = new MyCopier();
    if (copier == nsnull)
        return NS_ERROR_OUT_OF_MEMORY;
    NS_ADDREF(copier);

    rv = copier->AsyncCopy(srcTransport, destTransport);
    if (NS_FAILED(rv)) return rv;

    PumpEvents();

    NS_RELEASE(copier);
    return NS_OK;
}

////////////////////////////////////////////////////////////////////////////////

static nsresult
RunBlockingTest(nsIFile *srcFile, nsIFile *destFile)
{
    nsresult rv;

    LOG(("RunBlockingTest\n"));

    nsCOMPtr<nsIStreamTransportService> sts =
        do_GetService(kStreamTransportServiceCID, &rv);
    if (NS_FAILED(rv)) return rv;

    nsCOMPtr<nsIInputStream> srcIn;
    rv = NS_NewLocalFileInputStream(getter_AddRefs(srcIn), srcFile);
    if (NS_FAILED(rv)) return rv;

    nsCOMPtr<nsIOutputStream> fileOut;
    rv = NS_NewLocalFileOutputStream(getter_AddRefs(fileOut), destFile);
    if (NS_FAILED(rv)) return rv;
    
    nsCOMPtr<nsITransport> destTransport;
    rv = sts->CreateOutputTransport(fileOut, nsInt64(-1), nsInt64(-1),
                                    PR_TRUE, getter_AddRefs(destTransport));
    if (NS_FAILED(rv)) return rv;

    nsCOMPtr<nsIOutputStream> destOut;
    rv = destTransport->OpenOutputStream(nsITransport::OPEN_BLOCKING, 100, 10, getter_AddRefs(destOut));
    if (NS_FAILED(rv)) return rv;

    char buf[120];
    PRUint32 n;
    for (;;) {
        rv = srcIn->Read(buf, sizeof(buf), &n);
        if (NS_FAILED(rv) || (n == 0)) return rv;

        rv = destOut->Write(buf, n, &n);
        if (NS_FAILED(rv)) return rv;
    }

    return NS_OK;
}

////////////////////////////////////////////////////////////////////////////////

int
main(int argc, char* argv[])
{
    if (test_common_init(&argc, &argv) != 0)
        return -1;

    nsresult rv;

    if (argc < 2) {
        printf("usage: %s <file-to-read>\n", argv[0]);
        return -1;
    }
    char* fileName = argv[1];
    {
        nsCOMPtr<nsIServiceManager> servMan;
        NS_InitXPCOM2(getter_AddRefs(servMan), nsnull, nsnull);
        nsCOMPtr<nsIComponentRegistrar> registrar = do_QueryInterface(servMan);
        NS_ASSERTION(registrar, "Null nsIComponentRegistrar");
        if (registrar)
            registrar->AutoRegister(nsnull);

#if defined(PR_LOGGING)
        gTestLog = PR_NewLogModule("Test");
#endif

        nsCOMPtr<nsILocalFile> srcFile;
        rv = NS_NewNativeLocalFile(nsDependentCString(fileName), PR_FALSE, getter_AddRefs(srcFile));
        if (NS_FAILED(rv)) return rv;

        nsCOMPtr<nsIFile> destFile;
        rv = srcFile->Clone(getter_AddRefs(destFile));
        if (NS_FAILED(rv)) return rv;

        nsCAutoString leafName;
        rv = destFile->GetNativeLeafName(leafName);
        if (NS_FAILED(rv)) return rv;

        nsCAutoString newName(leafName);
        newName.Append(NS_LITERAL_CSTRING(".1"));
        rv = destFile->SetNativeLeafName(newName);
        if (NS_FAILED(rv)) return rv;

        rv = RunTest(srcFile, destFile);
        NS_ASSERTION(NS_SUCCEEDED(rv), "RunTest failed");

        newName = leafName;
        newName.Append(NS_LITERAL_CSTRING(".2"));
        rv = destFile->SetNativeLeafName(newName);
        if (NS_FAILED(rv)) return rv;

        rv = RunBlockingTest(srcFile, destFile);
        NS_ASSERTION(NS_SUCCEEDED(rv), "RunBlockingTest failed");

        // give background threads a chance to finish whatever work they may
        // be doing.
        PR_Sleep(PR_SecondsToInterval(1));
    } // this scopes the nsCOMPtrs
    // no nsCOMPtrs are allowed to be alive when you call NS_ShutdownXPCOM
    rv = NS_ShutdownXPCOM(nsnull);
    NS_ASSERTION(NS_SUCCEEDED(rv), "NS_ShutdownXPCOM failed");
    return NS_OK;
}
