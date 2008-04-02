/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
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
 * The Initial Developer of the Original Code is Google Inc.
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *  Darin Fisher <darin@meer.net>
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

#include "nsIIncrementalDownload.h"
#include "nsIRequestObserver.h"
#include "nsIProgressEventSink.h"
#include "nsIChannelEventSink.h"
#include "nsIInterfaceRequestor.h"
#include "nsIObserverService.h"
#include "nsIObserver.h"
#include "nsIPropertyBag2.h"
#include "nsIServiceManager.h"
#include "nsILocalFile.h"
#include "nsITimer.h"
#include "nsInt64.h"
#include "nsNetUtil.h"
#include "nsAutoPtr.h"
#include "nsWeakReference.h"
#include "nsChannelProperties.h"
#include "prio.h"
#include "prprf.h"

// Error code used internally by the incremental downloader to cancel the
// network channel when the download is already complete.
#define NS_ERROR_DOWNLOAD_COMPLETE \
    NS_ERROR_GENERATE_FAILURE(NS_ERROR_MODULE_GENERAL, 1)

// Error code used internally by the incremental downloader to cancel the
// network channel when the response to a range request is 200 instead of 206.
#define NS_ERROR_DOWNLOAD_NOT_PARTIAL \
    NS_ERROR_GENERATE_FAILURE(NS_ERROR_MODULE_GENERAL, 2)

// Default values used to initialize a nsIncrementalDownload object.
#define DEFAULT_CHUNK_SIZE (4096 * 16)  // bytes
#define DEFAULT_INTERVAL    60          // seconds

// Number of times to retry a failed byte-range request.
#define MAX_RETRY_COUNT 20

//-----------------------------------------------------------------------------

static nsresult
WriteToFile(nsILocalFile *lf, const char *data, PRUint32 len, PRInt32 flags)
{
  PRFileDesc *fd;
  nsresult rv = lf->OpenNSPRFileDesc(flags, 0600, &fd);
  if (NS_FAILED(rv))
    return rv;

  if (len)
    rv = PR_Write(fd, data, len) == PRInt32(len) ? NS_OK : NS_ERROR_FAILURE;

  PR_Close(fd);
  return rv;
}

static nsresult
AppendToFile(nsILocalFile *lf, const char *data, PRUint32 len)
{
  PRInt32 flags = PR_WRONLY | PR_CREATE_FILE | PR_APPEND;
  return WriteToFile(lf, data, len, flags);
}

// maxSize may be -1 if unknown
static void
MakeRangeSpec(const nsInt64 &size, const nsInt64 &maxSize, PRInt32 chunkSize,
              PRBool fetchRemaining, nsCString &rangeSpec)
{
  rangeSpec.AssignLiteral("bytes=");
  rangeSpec.AppendInt(PRInt64(size));
  rangeSpec.Append('-');

  if (fetchRemaining)
    return;

  nsInt64 end = size + nsInt64(chunkSize);
  if (maxSize != nsInt64(-1) && end > maxSize)
    end = maxSize;
  end -= 1;

  rangeSpec.AppendInt(PRInt64(end));
}

//-----------------------------------------------------------------------------

class nsIncrementalDownload : public nsIIncrementalDownload
                            , public nsIStreamListener
                            , public nsIObserver
                            , public nsIInterfaceRequestor
                            , public nsIChannelEventSink
                            , public nsSupportsWeakReference
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIREQUEST
  NS_DECL_NSIINCREMENTALDOWNLOAD
  NS_DECL_NSIREQUESTOBSERVER
  NS_DECL_NSISTREAMLISTENER
  NS_DECL_NSIOBSERVER
  NS_DECL_NSIINTERFACEREQUESTOR
  NS_DECL_NSICHANNELEVENTSINK

  nsIncrementalDownload();

private:
  ~nsIncrementalDownload() {}
  nsresult FlushChunk();
  nsresult CallOnStartRequest();
  void     CallOnStopRequest();
  nsresult StartTimer(PRInt32 interval);
  nsresult ProcessTimeout();
  nsresult ReadCurrentSize();
  nsresult ClearRequestHeader(nsIHttpChannel *channel);

  nsCOMPtr<nsIRequestObserver>   mObserver;
  nsCOMPtr<nsISupports>          mObserverContext;
  nsCOMPtr<nsIProgressEventSink> mProgressSink;
  nsCOMPtr<nsIURI>               mURI;
  nsCOMPtr<nsIURI>               mFinalURI;
  nsCOMPtr<nsILocalFile>         mDest;
  nsCOMPtr<nsIChannel>           mChannel;
  nsCOMPtr<nsITimer>             mTimer;
  nsAutoArrayPtr<char>           mChunk;
  PRInt32                        mChunkLen;
  PRInt32                        mChunkSize;
  PRInt32                        mInterval;
  nsInt64                        mTotalSize;
  nsInt64                        mCurrentSize;
  PRUint32                       mLoadFlags;
  PRInt32                        mNonPartialCount;
  nsresult                       mStatus;
  PRPackedBool                   mIsPending;
  PRPackedBool                   mDidOnStartRequest;
};

nsIncrementalDownload::nsIncrementalDownload()
  : mChunkLen(0)
  , mChunkSize(DEFAULT_CHUNK_SIZE)
  , mInterval(DEFAULT_INTERVAL)
  , mTotalSize(-1)
  , mCurrentSize(-1)
  , mLoadFlags(LOAD_NORMAL)
  , mNonPartialCount(0)
  , mStatus(NS_OK)
  , mIsPending(PR_FALSE)
  , mDidOnStartRequest(PR_FALSE)
{
}

nsresult
nsIncrementalDownload::FlushChunk()
{
  NS_ASSERTION(mTotalSize != nsInt64(-1), "total size should be known");

  if (mChunkLen == 0)
    return NS_OK;

  nsresult rv = AppendToFile(mDest, mChunk, mChunkLen);
  if (NS_FAILED(rv))
    return rv;

  mCurrentSize += nsInt64(mChunkLen);
  mChunkLen = 0;

  if (mProgressSink)
    mProgressSink->OnProgress(this, mObserverContext,
                              PRUint64(PRInt64(mCurrentSize)),
                              PRUint64(PRInt64(mTotalSize)));
  return NS_OK;
}

nsresult
nsIncrementalDownload::CallOnStartRequest()
{
  if (!mObserver || mDidOnStartRequest)
    return NS_OK;

  mDidOnStartRequest = PR_TRUE;
  return mObserver->OnStartRequest(this, mObserverContext);
}

void
nsIncrementalDownload::CallOnStopRequest()
{
  if (!mObserver)
    return;

  // Ensure that OnStartRequest is always called once before OnStopRequest.
  nsresult rv = CallOnStartRequest();
  if (NS_SUCCEEDED(mStatus))
    mStatus = rv;

  mIsPending = PR_FALSE;

  mObserver->OnStopRequest(this, mObserverContext, mStatus);
  mObserver = nsnull;
  mObserverContext = nsnull;
}

nsresult
nsIncrementalDownload::StartTimer(PRInt32 interval)
{
  nsresult rv;
  mTimer = do_CreateInstance(NS_TIMER_CONTRACTID, &rv);
  if (NS_FAILED(rv))
    return rv;

  return mTimer->Init(this, interval * 1000, nsITimer::TYPE_ONE_SHOT);
}

nsresult
nsIncrementalDownload::ProcessTimeout()
{
  NS_ASSERTION(!mChannel, "how can we have a channel?");

  // Handle existing error conditions
  if (NS_FAILED(mStatus)) {
    CallOnStopRequest();
    return NS_OK;
  }

  // Fetch next chunk
  
  nsCOMPtr<nsIChannel> channel;
  nsresult rv = NS_NewChannel(getter_AddRefs(channel), mFinalURI, nsnull,
                              nsnull, this, mLoadFlags);
  if (NS_FAILED(rv))
    return rv;

  nsCOMPtr<nsIHttpChannel> http = do_QueryInterface(channel, &rv);
  if (NS_FAILED(rv))
    return rv;

  NS_ASSERTION(mCurrentSize != nsInt64(-1),
      "we should know the current file size by now");

  rv = ClearRequestHeader(http);
  if (NS_FAILED(rv))
    return rv;

  // Don't bother making a range request if we are just going to fetch the
  // entire document.
  if (mInterval || mCurrentSize != nsInt64(0)) {
    nsCAutoString range;
    MakeRangeSpec(mCurrentSize, mTotalSize, mChunkSize, mInterval == 0, range);

    rv = http->SetRequestHeader(NS_LITERAL_CSTRING("Range"), range, PR_FALSE);
    if (NS_FAILED(rv))
      return rv;
  }

  rv = channel->AsyncOpen(this, nsnull);
  if (NS_FAILED(rv))
    return rv;

  // Wait to assign mChannel when we know we are going to succeed.  This is
  // important because we don't want to introduce a reference cycle between
  // mChannel and this until we know for a fact that AsyncOpen has succeeded,
  // thus ensuring that our stream listener methods will be invoked.
  mChannel = channel;
  return NS_OK;
}

// Reads the current file size and validates it.
nsresult
nsIncrementalDownload::ReadCurrentSize()
{
  nsInt64 size;
  nsresult rv = mDest->GetFileSize((PRInt64 *) &size);
  if (rv == NS_ERROR_FILE_NOT_FOUND ||
      rv == NS_ERROR_FILE_TARGET_DOES_NOT_EXIST) {
    mCurrentSize = 0;
    return NS_OK;
  }
  if (NS_FAILED(rv))
    return rv;

  mCurrentSize = size; 
  return NS_OK;
}

// nsISupports

NS_IMPL_ISUPPORTS8(nsIncrementalDownload,
                   nsIIncrementalDownload,
                   nsIRequest,
                   nsIStreamListener,
                   nsIRequestObserver,
                   nsIObserver,
                   nsIInterfaceRequestor,
                   nsIChannelEventSink,
                   nsISupportsWeakReference)

// nsIRequest

NS_IMETHODIMP
nsIncrementalDownload::GetName(nsACString &name)
{
  NS_ENSURE_TRUE(mURI, NS_ERROR_NOT_INITIALIZED);

  return mURI->GetSpec(name);
}

NS_IMETHODIMP
nsIncrementalDownload::IsPending(PRBool *isPending)
{
  *isPending = mIsPending;
  return NS_OK;
}

NS_IMETHODIMP
nsIncrementalDownload::GetStatus(nsresult *status)
{
  *status = mStatus;
  return NS_OK;
}

NS_IMETHODIMP
nsIncrementalDownload::Cancel(nsresult status)
{
  NS_ENSURE_ARG(NS_FAILED(status));

  // Ignore this cancelation if we're already canceled.
  if (NS_FAILED(mStatus))
    return NS_OK;

  mStatus = status;

  // Nothing more to do if callbacks aren't pending.
  if (!mIsPending)
    return NS_OK;

  if (mChannel) {
    mChannel->Cancel(mStatus);
    NS_ASSERTION(!mTimer, "what is this timer object doing here?");
  }
  else {
    // dispatch a timer callback event to drive invoking our listener's
    // OnStopRequest.
    if (mTimer)
      mTimer->Cancel();
    StartTimer(0);
  }

  return NS_OK;
}

NS_IMETHODIMP
nsIncrementalDownload::Suspend()
{
  return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP
nsIncrementalDownload::Resume()
{
  return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP
nsIncrementalDownload::GetLoadFlags(nsLoadFlags *loadFlags)
{
  *loadFlags = mLoadFlags;
  return NS_OK;
}

NS_IMETHODIMP
nsIncrementalDownload::SetLoadFlags(nsLoadFlags loadFlags)
{
  mLoadFlags = loadFlags;
  return NS_OK;
}

NS_IMETHODIMP
nsIncrementalDownload::GetLoadGroup(nsILoadGroup **loadGroup)
{
  return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP
nsIncrementalDownload::SetLoadGroup(nsILoadGroup *loadGroup)
{
  return NS_ERROR_NOT_IMPLEMENTED;
}

// nsIIncrementalDownload

NS_IMETHODIMP
nsIncrementalDownload::Init(nsIURI *uri, nsIFile *dest,
                            PRInt32 chunkSize, PRInt32 interval)
{
  // Keep it simple: only allow initialization once
  NS_ENSURE_FALSE(mURI, NS_ERROR_ALREADY_INITIALIZED);

  mDest = do_QueryInterface(dest);
  NS_ENSURE_ARG(mDest);

  mURI = uri;
  mFinalURI = uri;

  if (chunkSize > 0)
    mChunkSize = chunkSize;
  if (interval >= 0)
    mInterval = interval;
  return NS_OK;
}

NS_IMETHODIMP
nsIncrementalDownload::GetURI(nsIURI **result)
{
  NS_IF_ADDREF(*result = mURI);
  return NS_OK;
}

NS_IMETHODIMP
nsIncrementalDownload::GetFinalURI(nsIURI **result)
{
  NS_IF_ADDREF(*result = mFinalURI);
  return NS_OK;
}

NS_IMETHODIMP
nsIncrementalDownload::GetDestination(nsIFile **result)
{
  if (!mDest) {
    *result = nsnull;
    return NS_OK;
  }
  // Return a clone of mDest so that callers may modify the resulting nsIFile
  // without corrupting our internal object.  This also works around the fact
  // that some nsIFile impls may cache the result of stat'ing the filesystem.
  return mDest->Clone(result);
}

NS_IMETHODIMP
nsIncrementalDownload::GetTotalSize(PRInt64 *result)
{
  *result = mTotalSize;
  return NS_OK;
}

NS_IMETHODIMP
nsIncrementalDownload::GetCurrentSize(PRInt64 *result)
{
  *result = mCurrentSize;
  return NS_OK;
}

NS_IMETHODIMP
nsIncrementalDownload::Start(nsIRequestObserver *observer,
                             nsISupports *context)
{
  NS_ENSURE_ARG(observer);
  NS_ENSURE_FALSE(mIsPending, NS_ERROR_IN_PROGRESS);

  // Observe system shutdown so we can be sure to release any reference held
  // between ourselves and the timer.  We have the observer service hold a weak
  // reference to us, so that we don't have to worry about calling
  // RemoveObserver.  XXX(darin): The timer code should do this for us.
  nsCOMPtr<nsIObserverService> obs =
      do_GetService("@mozilla.org/observer-service;1");
  if (obs)
    obs->AddObserver(this, NS_XPCOM_SHUTDOWN_OBSERVER_ID, PR_TRUE);

  nsresult rv = ReadCurrentSize();
  if (NS_FAILED(rv))
    return rv;

  rv = StartTimer(0);
  if (NS_FAILED(rv))
    return rv;

  mObserver = observer;
  mObserverContext = context;
  mProgressSink = do_QueryInterface(observer);  // ok if null

  mIsPending = PR_TRUE;
  return NS_OK;
}

// nsIRequestObserver

NS_IMETHODIMP
nsIncrementalDownload::OnStartRequest(nsIRequest *request,
                                      nsISupports *context)
{
  nsresult rv;

  nsCOMPtr<nsIHttpChannel> http = do_QueryInterface(request, &rv);
  if (NS_FAILED(rv))
    return rv;

  // Ensure that we are receiving a 206 response.
  PRUint32 code;
  rv = http->GetResponseStatus(&code);
  if (NS_FAILED(rv))
    return rv;
  if (code != 206) {
    // We may already have the entire file downloaded, in which case
    // our request for a range beyond the end of the file would have
    // been met with an error response code.
    if (code == 416 && mTotalSize == nsInt64(-1)) {
      mTotalSize = mCurrentSize;
      // Return an error code here to suppress OnDataAvailable.
      return NS_ERROR_DOWNLOAD_COMPLETE;
    }
    // The server may have decided to give us all of the data in one chunk.  If
    // we requested a partial range, then we don't want to download all of the
    // data at once.  So, we'll just try again, but if this keeps happening then
    // we'll eventually give up.
    if (code == 200) {
      if (mInterval) {
        mChannel = nsnull;
        if (++mNonPartialCount > MAX_RETRY_COUNT) {
          NS_WARNING("unable to fetch a byte range; giving up");
          return NS_ERROR_FAILURE;
        }
        // Increase delay with each failure.
        StartTimer(mInterval * mNonPartialCount);
        return NS_ERROR_DOWNLOAD_NOT_PARTIAL;
      }
      // Since we have been asked to download the rest of the file, we can deal
      // with a 200 response.  This may result in downloading the beginning of
      // the file again, but that can't really be helped.
    } else {
      NS_WARNING("server response was unexpected");
      return NS_ERROR_UNEXPECTED;
    }
  } else {
    // We got a partial response, so clear this counter in case the next chunk
    // results in a 200 response.
    mNonPartialCount = 0;
  }

  // Do special processing after the first response.
  if (mTotalSize == nsInt64(-1)) {
    // Update knowledge of mFinalURI
    rv = http->GetURI(getter_AddRefs(mFinalURI));
    if (NS_FAILED(rv))
      return rv;

    if (code == 206) {
      // OK, read the Content-Range header to determine the total size of this
      // download file.
      nsCAutoString buf;
      rv = http->GetResponseHeader(NS_LITERAL_CSTRING("Content-Range"), buf);
      if (NS_FAILED(rv))
        return rv;
      PRInt32 slash = buf.FindChar('/');
      if (slash == kNotFound) {
        NS_WARNING("server returned invalid Content-Range header!");
        return NS_ERROR_UNEXPECTED;
      }
      if (PR_sscanf(buf.get() + slash + 1, "%lld", (PRInt64 *) &mTotalSize) != 1)
        return NS_ERROR_UNEXPECTED;
    } else {
      // Use nsIPropertyBag2 to fetch the content length as it exposes the
      // value as a 64-bit number.
      nsCOMPtr<nsIPropertyBag2> props = do_QueryInterface(request, &rv);
      if (NS_FAILED(rv))
        return rv;
      rv = props->GetPropertyAsInt64(NS_CHANNEL_PROP_CONTENT_LENGTH,
                                     &mTotalSize.mValue);
      // We need to know the total size of the thing we're trying to download.
      if (mTotalSize == nsInt64(-1)) {
        NS_WARNING("server returned no content-length header!");
        return NS_ERROR_UNEXPECTED;
      }
      // Need to truncate (or create, if it doesn't exist) the file since we
      // are downloading the whole thing.
      WriteToFile(mDest, nsnull, 0, PR_WRONLY | PR_CREATE_FILE | PR_TRUNCATE);
      mCurrentSize = 0;
    }

    // Notify observer that we are starting...
    rv = CallOnStartRequest();
    if (NS_FAILED(rv))
      return rv;
  }

  // Adjust mChunkSize accordingly if mCurrentSize is close to mTotalSize.
  nsInt64 diff = mTotalSize - mCurrentSize;
  if (diff <= nsInt64(0)) {
    NS_WARNING("about to set a bogus chunk size; giving up");
    return NS_ERROR_UNEXPECTED;
  }

  if (diff < nsInt64(mChunkSize))
    mChunkSize = PRUint32(diff);

  mChunk = new char[mChunkSize];
  if (!mChunk)
    rv = NS_ERROR_OUT_OF_MEMORY;

  return rv;
}

NS_IMETHODIMP
nsIncrementalDownload::OnStopRequest(nsIRequest *request,
                                     nsISupports *context,
                                     nsresult status)
{
  // Not a real error; just a trick to kill off the channel without our
  // listener having to care.
  if (status == NS_ERROR_DOWNLOAD_NOT_PARTIAL)
    return NS_OK;

  // Not a real error; just a trick used to suppress OnDataAvailable calls.
  if (status == NS_ERROR_DOWNLOAD_COMPLETE)
    status = NS_OK;

  if (NS_SUCCEEDED(mStatus))
    mStatus = status;

  if (mChunk) {
    if (NS_SUCCEEDED(mStatus))
      mStatus = FlushChunk();

    mChunk = nsnull;  // deletes memory
    mChunkLen = 0;
  }

  mChannel = nsnull;

  // Notify listener if we hit an error or finished
  if (NS_FAILED(mStatus) || mCurrentSize == mTotalSize) {
    CallOnStopRequest();
    return NS_OK;
  }

  return StartTimer(mInterval);  // Do next chunk
}

// nsIStreamListener

NS_IMETHODIMP
nsIncrementalDownload::OnDataAvailable(nsIRequest *request,
                                       nsISupports *context,
                                       nsIInputStream *input,
                                       PRUint32 offset,
                                       PRUint32 count)
{
  while (count) {
    PRUint32 space = mChunkSize - mChunkLen;
    PRUint32 n, len = PR_MIN(space, count);

    nsresult rv = input->Read(mChunk + mChunkLen, len, &n);
    if (NS_FAILED(rv))
      return rv;
    if (n != len)
      return NS_ERROR_UNEXPECTED;

    count -= n;
    mChunkLen += n;

    if (mChunkLen == mChunkSize)
      FlushChunk();
  }

  return NS_OK;
}

// nsIObserver

NS_IMETHODIMP
nsIncrementalDownload::Observe(nsISupports *subject, const char *topic,
                               const PRUnichar *data)
{
  if (strcmp(topic, NS_XPCOM_SHUTDOWN_OBSERVER_ID) == 0) {
    Cancel(NS_ERROR_ABORT);

    // Since the app is shutting down, we need to go ahead and notify our
    // observer here.  Otherwise, we would notify them after XPCOM has been
    // shutdown or not at all.
    CallOnStopRequest();
  }
  else if (strcmp(topic, NS_TIMER_CALLBACK_TOPIC) == 0) {
    mTimer = nsnull;
    nsresult rv = ProcessTimeout();
    if (NS_FAILED(rv))
      Cancel(rv);
  }
  return NS_OK;
}

// nsIInterfaceRequestor

NS_IMETHODIMP
nsIncrementalDownload::GetInterface(const nsIID &iid, void **result)
{
  if (iid.Equals(NS_GET_IID(nsIChannelEventSink))) {
    NS_ADDREF_THIS();
    *result = static_cast<nsIChannelEventSink *>(this);
    return NS_OK;
  }

  nsCOMPtr<nsIInterfaceRequestor> ir = do_QueryInterface(mObserver);
  if (ir)
    return ir->GetInterface(iid, result);

  return NS_ERROR_NO_INTERFACE;
}

nsresult 
nsIncrementalDownload::ClearRequestHeader(nsIHttpChannel *channel)
{
  NS_ENSURE_ARG(channel);
  
  // We don't support encodings -- they make the Content-Length not equal
  // to the actual size of the data. 
  return channel->SetRequestHeader(NS_LITERAL_CSTRING("Accept-Encoding"),
                                   NS_LITERAL_CSTRING(""), PR_FALSE);
}

// nsIChannelEventSink

NS_IMETHODIMP
nsIncrementalDownload::OnChannelRedirect(nsIChannel *oldChannel,
                                         nsIChannel *newChannel,
                                         PRUint32 flags)
{
  // In response to a redirect, we need to propagate the Range header.  See bug
  // 311595.  Any failure code returned from this function aborts the redirect.
 
  nsCOMPtr<nsIHttpChannel> http = do_QueryInterface(oldChannel);
  NS_ENSURE_STATE(http);

  nsCOMPtr<nsIHttpChannel> newHttpChannel = do_QueryInterface(newChannel);
  NS_ENSURE_STATE(newHttpChannel);

  NS_NAMED_LITERAL_CSTRING(rangeHdr, "Range");

  nsresult rv = ClearRequestHeader(newHttpChannel);
  if (NS_FAILED(rv))
    return rv;

  // If we didn't have a Range header, then we must be doing a full download.
  nsCAutoString rangeVal;
  http->GetRequestHeader(rangeHdr, rangeVal);
  if (!rangeVal.IsEmpty()) {
    rv = newHttpChannel->SetRequestHeader(rangeHdr, rangeVal, PR_FALSE);
    NS_ENSURE_SUCCESS(rv, rv);
  }

  // Give the observer a chance to see this redirect notification.
  nsCOMPtr<nsIChannelEventSink> sink = do_GetInterface(mObserver);
  if (sink)
    rv = sink->OnChannelRedirect(oldChannel, newChannel, flags);

  // Update mChannel, so we can Cancel the new channel.
  if (NS_SUCCEEDED(rv))
    mChannel = newChannel;

  return rv;
}

extern NS_METHOD
net_NewIncrementalDownload(nsISupports *outer, const nsIID &iid, void **result)
{
  if (outer)
    return NS_ERROR_NO_AGGREGATION;

  nsIncrementalDownload *d = new nsIncrementalDownload();
  if (!d)
    return NS_ERROR_OUT_OF_MEMORY;
  
  NS_ADDREF(d);
  nsresult rv = d->QueryInterface(iid, result);
  NS_RELEASE(d);
  return rv;
}
