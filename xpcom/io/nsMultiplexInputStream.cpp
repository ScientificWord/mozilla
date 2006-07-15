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
 * The Original Code is frightening to behold.
 *
 * The Initial Developer of the Original Code is
 * Jonas Sicking.
 * Portions created by the Initial Developer are Copyright (C) 2001
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Jonas Sicking <sicking@bigfoot.com>
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

/**
 * The multiplex stream concatenates a list of input streams into a single
 * stream.
 */

#include "nsMultiplexInputStream.h"
#include "nsIMultiplexInputStream.h"
#include "nsISeekableStream.h"
#include "nsCOMPtr.h"
#include "nsCOMArray.h"
#include "nsInt64.h"

class nsMultiplexInputStream : public nsIMultiplexInputStream,
                               public nsISeekableStream
{
public:
    nsMultiplexInputStream();

    NS_DECL_ISUPPORTS
    NS_DECL_NSIINPUTSTREAM
    NS_DECL_NSIMULTIPLEXINPUTSTREAM
    NS_DECL_NSISEEKABLESTREAM

private:
    ~nsMultiplexInputStream() {}

    struct ReadSegmentsState {
        nsIInputStream* mThisStream;
        PRUint32 mOffset;
        nsWriteSegmentFun mWriter;
        void* mClosure;
        PRBool mDone;
    };

    static NS_METHOD ReadSegCb(nsIInputStream* aIn, void* aClosure,
                               const char* aFromRawSegment, PRUint32 aToOffset,
                               PRUint32 aCount, PRUint32 *aWriteCount);
    
    nsCOMArray<nsIInputStream> mStreams;
    PRUint32 mCurrentStream;
    PRBool mStartedReadingCurrent;
    nsresult mStatus;
};


NS_IMPL_THREADSAFE_ISUPPORTS3(nsMultiplexInputStream,
                              nsIMultiplexInputStream,
                              nsIInputStream,
                              nsISeekableStream)

nsMultiplexInputStream::nsMultiplexInputStream()
    : mCurrentStream(0),
      mStartedReadingCurrent(PR_FALSE),
      mStatus(NS_OK)
{
}

/* readonly attribute unsigned long count; */
NS_IMETHODIMP
nsMultiplexInputStream::GetCount(PRUint32 *aCount)
{
    *aCount = mStreams.Count();
    return NS_OK;
}

/* void appendStream (in nsIInputStream stream); */
NS_IMETHODIMP
nsMultiplexInputStream::AppendStream(nsIInputStream *aStream)
{
    return mStreams.AppendObject(aStream) ? NS_OK : NS_ERROR_OUT_OF_MEMORY;
}

/* void insertStream (in nsIInputStream stream, in unsigned long index); */
NS_IMETHODIMP
nsMultiplexInputStream::InsertStream(nsIInputStream *aStream, PRUint32 aIndex)
{
    PRBool result = mStreams.InsertObjectAt(aStream, aIndex);
    NS_ENSURE_TRUE(result, NS_ERROR_OUT_OF_MEMORY);
    if (mCurrentStream > aIndex ||
        (mCurrentStream == aIndex && mStartedReadingCurrent))
        ++mCurrentStream;
    return NS_OK;
}

/* void removeStream (in unsigned long index); */
NS_IMETHODIMP
nsMultiplexInputStream::RemoveStream(PRUint32 aIndex)
{
    PRBool result = mStreams.RemoveObjectAt(aIndex);
    NS_ENSURE_TRUE(result, NS_ERROR_NOT_AVAILABLE);
    if (mCurrentStream > aIndex)
        --mCurrentStream;
    else if (mCurrentStream == aIndex)
        mStartedReadingCurrent = PR_FALSE;

    return NS_OK;
}

/* nsIInputStream getStream (in unsigned long index); */
NS_IMETHODIMP
nsMultiplexInputStream::GetStream(PRUint32 aIndex, nsIInputStream **_retval)
{
    *_retval = mStreams.SafeObjectAt(aIndex);
    NS_ENSURE_TRUE(*_retval, NS_ERROR_NOT_AVAILABLE);

    NS_ADDREF(*_retval);
    return NS_OK;
}

/* void close (); */
NS_IMETHODIMP
nsMultiplexInputStream::Close()
{
    mStatus = NS_BASE_STREAM_CLOSED;

    nsresult rv = NS_OK;

    PRUint32 len = mStreams.Count();
    for (PRUint32 i = 0; i < len; ++i) {
        nsresult rv2 = mStreams[i]->Close();
        // We still want to close all streams, but we should return an error
        if (NS_FAILED(rv2))
            rv = rv2;
    }
    return rv;
}

/* unsigned long available (); */
NS_IMETHODIMP
nsMultiplexInputStream::Available(PRUint32 *_retval)
{
    if (NS_FAILED(mStatus))
        return mStatus;

    nsresult rv;
    PRUint32 avail = 0;

    PRUint32 len = mStreams.Count();
    for (PRUint32 i = mCurrentStream; i < len; i++) {
        PRUint32 streamAvail;
        rv = mStreams[i]->Available(&streamAvail);
        NS_ENSURE_SUCCESS(rv, rv);
        avail += streamAvail;
    }
    *_retval = avail;
    return NS_OK;
}

/* [noscript] unsigned long read (in charPtr buf, in unsigned long count); */
NS_IMETHODIMP
nsMultiplexInputStream::Read(char * aBuf, PRUint32 aCount, PRUint32 *_retval)
{
    // It is tempting to implement this method in terms of ReadSegments, but
    // that would prevent this class from being used with streams that only
    // implement Read (e.g., file streams).
 
    *_retval = 0;

    if (mStatus == NS_BASE_STREAM_CLOSED)
        return NS_OK;
    if (NS_FAILED(mStatus))
        return mStatus;
 
    nsresult rv = NS_OK;

    PRUint32 len = mStreams.Count();
    while (mCurrentStream < len && aCount) {
        PRUint32 read;
        rv = mStreams[mCurrentStream]->Read(aBuf, aCount, &read);

        // XXX some streams return NS_BASE_STREAM_CLOSED to indicate EOF.
        // (This is a bug in those stream implementations)
        if (rv == NS_BASE_STREAM_CLOSED) {
            NS_NOTREACHED("Input stream's Read method returned NS_BASE_STREAM_CLOSED");
            rv = NS_OK;
            read = 0;
        }
        else if (NS_FAILED(rv))
            break;

        if (read == 0) {
            ++mCurrentStream;
            mStartedReadingCurrent = PR_FALSE;
        }
        else {
            NS_ASSERTION(aCount >= read, "Read more than requested");
            *_retval += read;
            aCount -= read;
            aBuf += read;
            mStartedReadingCurrent = PR_TRUE;
        }
    }
    return *_retval ? NS_OK : rv;
}

/* [noscript] unsigned long readSegments (in nsWriteSegmentFun writer,
 *                                        in voidPtr closure,
 *                                        in unsigned long count); */
NS_IMETHODIMP
nsMultiplexInputStream::ReadSegments(nsWriteSegmentFun aWriter, void *aClosure,
                                     PRUint32 aCount, PRUint32 *_retval)
{
    if (mStatus == NS_BASE_STREAM_CLOSED) {
        *_retval = 0;
        return NS_OK;
    }
    if (NS_FAILED(mStatus))
        return mStatus;

    NS_ASSERTION(aWriter, "missing aWriter");

    nsresult rv = NS_OK;
    ReadSegmentsState state;
    state.mThisStream = this;
    state.mOffset = 0;
    state.mWriter = aWriter;
    state.mClosure = aClosure;
    state.mDone = PR_FALSE;
    
    PRUint32 len = mStreams.Count();
    while (mCurrentStream < len && aCount) {
        PRUint32 read;
        rv = mStreams[mCurrentStream]->ReadSegments(ReadSegCb, &state, aCount, &read);

        // XXX some streams return NS_BASE_STREAM_CLOSED to indicate EOF.
        // (This is a bug in those stream implementations)
        if (rv == NS_BASE_STREAM_CLOSED) {
            NS_NOTREACHED("Input stream's Read method returned NS_BASE_STREAM_CLOSED");
            rv = NS_OK;
            read = 0;
        }

        // if |aWriter| decided to stop reading segments...
        if (state.mDone || NS_FAILED(rv))
            break;

        // if stream is empty, then advance to the next stream.
        if (read == 0) {
            ++mCurrentStream;
            mStartedReadingCurrent = PR_FALSE;
        }
        else {
            NS_ASSERTION(aCount >= read, "Read more than requested");
            state.mOffset += read;
            aCount -= read;
            mStartedReadingCurrent = PR_TRUE;
        }
    }

    // if we successfully read some data, then this call succeeded.
    *_retval = state.mOffset;
    return state.mOffset ? NS_OK : rv;
}

NS_METHOD
nsMultiplexInputStream::ReadSegCb(nsIInputStream* aIn, void* aClosure,
                                  const char* aFromRawSegment,
                                  PRUint32 aToOffset, PRUint32 aCount,
                                  PRUint32 *aWriteCount)
{
    nsresult rv;
    ReadSegmentsState* state = (ReadSegmentsState*)aClosure;
    rv = (state->mWriter)(state->mThisStream,
                          state->mClosure,
                          aFromRawSegment,
                          aToOffset + state->mOffset,
                          aCount,
                          aWriteCount);
    if (NS_FAILED(rv))
        state->mDone = PR_TRUE;
    return rv;
}

/* readonly attribute boolean nonBlocking; */
NS_IMETHODIMP
nsMultiplexInputStream::IsNonBlocking(PRBool *aNonBlocking)
{
    PRUint32 len = mStreams.Count();
    for (PRUint32 i = 0; i < len; ++i) {
        nsresult rv = mStreams[i]->IsNonBlocking(aNonBlocking);
        NS_ENSURE_SUCCESS(rv, rv);
        // If one is non-blocking the entire stream becomes non-blocking
        // (except that we don't implement nsIAsyncInputStream, so there's
        //  not much for the caller to do if Read returns "would block")
        if (*aNonBlocking)
            return NS_OK;
    }
    return NS_OK;
}

/* void seek (in PRInt32 whence, in PRInt32 offset); */
NS_IMETHODIMP
nsMultiplexInputStream::Seek(PRInt32 aWhence, PRInt64 aOffset)
{
    if (NS_FAILED(mStatus))
        return mStatus;

    nsresult rv;

    // rewinding to start is easy, and should be the most common case
    if (aWhence == NS_SEEK_SET && aOffset == 0)
    {
        PRUint32 i, last;
        last = mStartedReadingCurrent ? mCurrentStream+1 : mCurrentStream;
        for (i = 0; i < last; ++i) {
            nsCOMPtr<nsISeekableStream> stream = do_QueryInterface(mStreams[i]);
            NS_ENSURE_TRUE(stream, NS_ERROR_NO_INTERFACE);

            rv = stream->Seek(NS_SEEK_SET, 0);
            NS_ENSURE_SUCCESS(rv, rv);
        }
        mCurrentStream = 0;
        mStartedReadingCurrent = PR_FALSE;
        return NS_OK;
    }

    // other Seeks not implemented yet
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* PRUint32 tell (); */
NS_IMETHODIMP
nsMultiplexInputStream::Tell(PRInt64 *_retval)
{
    if (NS_FAILED(mStatus))
        return mStatus;

    nsresult rv;
    nsInt64 ret64 = 0;
    PRUint32 i, last;
    last = mStartedReadingCurrent ? mCurrentStream+1 : mCurrentStream;
    for (i = 0; i < last; ++i) {
        nsCOMPtr<nsISeekableStream> stream = do_QueryInterface(mStreams[i]);
        NS_ENSURE_TRUE(stream, NS_ERROR_NO_INTERFACE);

        PRInt64 pos;
        rv = stream->Tell(&pos);
        NS_ENSURE_SUCCESS(rv, rv);
        ret64 += pos;
    }
    *_retval =  ret64;

    return NS_OK;
}

/* void setEOF (); */
NS_IMETHODIMP
nsMultiplexInputStream::SetEOF()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

NS_METHOD
nsMultiplexInputStreamConstructor(nsISupports *outer,
                                  REFNSIID iid,
                                  void **result)
{
    *result = nsnull;

    if (outer)
        return NS_ERROR_NO_AGGREGATION;

    nsMultiplexInputStream *inst;
    NS_NEWXPCOM(inst, nsMultiplexInputStream);
    if (!inst)
        return NS_ERROR_OUT_OF_MEMORY;

    NS_ADDREF(inst);
    nsresult rv = inst->QueryInterface(iid, result);
    NS_RELEASE(inst);

    return rv;
}
