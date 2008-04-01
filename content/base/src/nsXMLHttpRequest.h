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

#ifndef nsXMLHttpRequest_h__
#define nsXMLHttpRequest_h__

#include "nsIXMLHttpRequest.h"
#include "nsISupportsUtils.h"
#include "nsCOMPtr.h"
#include "nsString.h"
#include "nsIDOMLoadListener.h"
#include "nsIDOMEventTarget.h"
#include "nsIDOMDocument.h"
#include "nsIURI.h"
#include "nsIHttpChannel.h"
#include "nsIDocument.h"
#include "nsIStreamListener.h"
#include "nsWeakReference.h"
#include "jsapi.h"
#include "nsIScriptContext.h"
#include "nsIChannelEventSink.h"
#include "nsIInterfaceRequestor.h"
#include "nsIHttpHeaderVisitor.h"
#include "nsIProgressEventSink.h"
#include "nsCOMArray.h"
#include "nsJSUtils.h"
#include "nsTArray.h"
#include "nsCycleCollectionParticipant.h"
#include "nsIJSNativeInitializer.h"
#include "nsPIDOMWindow.h"
#include "nsIDOMLSProgressEvent.h"
#include "nsClassHashtable.h"
#include "nsHashKeys.h"
#include "prclist.h"
#include "prtime.h"

class nsILoadGroup;

class nsAccessControlLRUCache
{
  struct CacheEntry : public PRCList
  {
    CacheEntry(const nsACString& aKey, PRTime aValue)
    : key(aKey), value(aValue)
    {
      MOZ_COUNT_CTOR(nsAccessControlLRUCache::CacheEntry);
    }
    
    ~CacheEntry()
    {
      MOZ_COUNT_DTOR(nsAccessControlLRUCache::CacheEntry);
    }
    
    nsCString key;
    PRTime value;
  };

public:
  nsAccessControlLRUCache()
  {
    MOZ_COUNT_CTOR(nsAccessControlLRUCache);
    PR_INIT_CLIST(&mList);
  }

  ~nsAccessControlLRUCache()
  {
    Clear();
    MOZ_COUNT_DTOR(nsAccessControlLRUCache);
  }

  PRBool Initialize()
  {
    return mTable.Init();
  }

  void GetEntry(nsIURI* aURI, nsIPrincipal* aPrincipal,
                PRTime* _retval);

  void PutEntry(nsIURI* aURI, nsIPrincipal* aPrincipal,
                PRTime aValue);

  void Clear();

private:
  PRBool GetEntryInternal(const nsACString& aKey, CacheEntry** _retval);

  PR_STATIC_CALLBACK(PLDHashOperator)
    RemoveExpiredEntries(const nsACString& aKey, nsAutoPtr<CacheEntry>& aValue,
                         void* aUserData);

  static PRBool GetCacheKey(nsIURI* aURI, nsIPrincipal* aPrincipal,
                            nsACString& _retval);

  nsClassHashtable<nsCStringHashKey, CacheEntry> mTable;
  PRCList mList;
};

class nsXMLHttpRequest : public nsIXMLHttpRequest,
                         public nsIJSXMLHttpRequest,
                         public nsIDOMLoadListener,
                         public nsIDOMEventTarget,
                         public nsIStreamListener,
                         public nsIChannelEventSink,
                         public nsIProgressEventSink,
                         public nsIInterfaceRequestor,
                         public nsSupportsWeakReference,
                         public nsIJSNativeInitializer
{
public:
  nsXMLHttpRequest();
  virtual ~nsXMLHttpRequest();

  NS_DECL_CYCLE_COLLECTING_ISUPPORTS

  // nsIXMLHttpRequest
  NS_DECL_NSIXMLHTTPREQUEST

  // nsIJSXMLHttpRequest
  NS_DECL_NSIJSXMLHTTPREQUEST

  // nsIDOMEventTarget
  NS_DECL_NSIDOMEVENTTARGET

  // nsIDOMEventListener
  NS_DECL_NSIDOMEVENTLISTENER

  // nsIDOMLoadListener
  NS_IMETHOD Load(nsIDOMEvent* aEvent);
  NS_IMETHOD BeforeUnload(nsIDOMEvent* aEvent);
  NS_IMETHOD Unload(nsIDOMEvent* aEvent);
  NS_IMETHOD Abort(nsIDOMEvent* aEvent);
  NS_IMETHOD Error(nsIDOMEvent* aEvent);

  // nsIStreamListener
  NS_DECL_NSISTREAMLISTENER

  // nsIRequestObserver
  NS_DECL_NSIREQUESTOBSERVER

  // nsIChannelEventSink
  NS_DECL_NSICHANNELEVENTSINK

  // nsIProgressEventSink
  NS_DECL_NSIPROGRESSEVENTSINK

  // nsIInterfaceRequestor
  NS_DECL_NSIINTERFACEREQUESTOR

  // nsIJSNativeInitializer
  NS_IMETHOD Initialize(nsISupports* aOwner, JSContext* cx, JSObject* obj,
                       PRUint32 argc, jsval* argv);

  // This is called by the factory constructor.
  nsresult Init();

  NS_DECL_CYCLE_COLLECTION_CLASS_AMBIGUOUS(nsXMLHttpRequest, nsIXMLHttpRequest)

  static PRBool EnsureACCache()
  {
    if (sAccessControlCache)
      return PR_TRUE;

    nsAutoPtr<nsAccessControlLRUCache> newCache(new nsAccessControlLRUCache());
    NS_ENSURE_TRUE(newCache, PR_FALSE);

    if (newCache->Initialize()) {
      sAccessControlCache = newCache.forget();
      return PR_TRUE;
    }

    return PR_FALSE;
  }

  static void ShutdownACCache()
  {
    if (sAccessControlCache) {
      delete sAccessControlCache;
      sAccessControlCache = nsnull;
    }
  }

  static nsAccessControlLRUCache* sAccessControlCache;

protected:

  nsresult DetectCharset(nsACString& aCharset);
  nsresult ConvertBodyToText(nsAString& aOutBuffer);
  static NS_METHOD StreamReaderFunc(nsIInputStream* in,
                void* closure,
                const char* fromRawSegment,
                PRUint32 toOffset,
                PRUint32 count,
                PRUint32 *writeCount);
  // Change the state of the object with this. The broadcast argument
  // determines if the onreadystatechange listener should be called.
  // If aClearEventListeners is true, ChangeState will take refs to
  // any event listeners it needs and call ClearEventListeners before
  // making any HandleEvent() calls that could change the listener
  // values.
  nsresult ChangeState(PRUint32 aState, PRBool aBroadcast = PR_TRUE,
                       PRBool aClearEventListeners = PR_FALSE);
  nsresult RequestCompleted();
  nsresult GetLoadGroup(nsILoadGroup **aLoadGroup);
  nsIURI *GetBaseURI();

  // This creates a trusted event, which is not cancelable and doesn't
  // bubble. Don't call this if we have no event listeners, since this may
  // use our script context, which is not set in that case.
  nsresult CreateEvent(const nsAString& aType, nsIDOMEvent** domevent);

  // Make a copy of a pair of members to be passed to NotifyEventListeners.
  void CopyEventListeners(nsCOMPtr<nsIDOMEventListener>& aListener,
                          const nsCOMArray<nsIDOMEventListener>& aListenerArray,
                          nsCOMArray<nsIDOMEventListener>& aCopy);

  // aListeners must be a "non-live" list (i.e., addEventListener and
  // removeEventListener should not affect it).  It should be built from
  // member variables by calling CopyEventListeners.
  void NotifyEventListeners(const nsCOMArray<nsIDOMEventListener>& aListeners,
                            nsIDOMEvent* aEvent);
  void ClearEventListeners();
  already_AddRefed<nsIHttpChannel> GetCurrentHttpChannel();

  /**
   * Check if mChannel is ok for a cross-site request by making sure no
   * inappropriate headers are set, and no username/password is set.
   *
   * Also updates the XML_HTTP_REQUEST_USE_XSITE_AC bit.
   */
  nsresult CheckChannelForCrossSiteRequest();

  nsresult CheckInnerWindowCorrectness()
  {
    if (mOwner) {
      NS_ASSERTION(mOwner->IsInnerWindow(), "Should have inner window here!\n");
      nsPIDOMWindow* outer = mOwner->GetOuterWindow();
      if (!outer || outer->GetCurrentInnerWindow() != mOwner) {
        return NS_ERROR_FAILURE;
      }
    }
    return NS_OK;
  }

  nsCOMPtr<nsISupports> mContext;
  nsCOMPtr<nsIPrincipal> mPrincipal;
  nsCOMPtr<nsIChannel> mChannel;
  // mReadRequest is different from mChannel for multipart requests
  nsCOMPtr<nsIRequest> mReadRequest;
  nsCOMPtr<nsIDOMDocument> mDocument;
  nsCOMPtr<nsIChannel> mACGetChannel;

  nsCOMArray<nsIDOMEventListener> mLoadEventListeners;
  nsCOMArray<nsIDOMEventListener> mErrorEventListeners;
  nsCOMArray<nsIDOMEventListener> mProgressEventListeners;
  nsCOMArray<nsIDOMEventListener> mUploadProgressEventListeners;
  nsCOMArray<nsIDOMEventListener> mReadystatechangeEventListeners;

  // These may be null (native callers or xpcshell).
  nsCOMPtr<nsIScriptContext> mScriptContext;
  nsCOMPtr<nsPIDOMWindow>    mOwner; // Inner window.

  nsCOMPtr<nsIDOMEventListener> mOnLoadListener;
  nsCOMPtr<nsIDOMEventListener> mOnErrorListener;
  nsCOMPtr<nsIDOMEventListener> mOnProgressListener;
  nsCOMPtr<nsIDOMEventListener> mOnUploadProgressListener;
  nsCOMPtr<nsIDOMEventListener> mOnReadystatechangeListener;

  nsCOMPtr<nsIStreamListener> mXMLParserStreamListener;

  // used to implement getAllResponseHeaders()
  class nsHeaderVisitor : public nsIHttpHeaderVisitor {
  public:
    NS_DECL_ISUPPORTS
    NS_DECL_NSIHTTPHEADERVISITOR
    nsHeaderVisitor() { }
    virtual ~nsHeaderVisitor() {}
    const nsACString &Headers() { return mHeaders; }
  private:
    nsCString mHeaders;
  };

  nsCString mResponseBody;

  nsCString mOverrideMimeType;

  /**
   * The notification callbacks the channel had when Send() was
   * called.  We want to forward things here as needed.
   */
  nsCOMPtr<nsIInterfaceRequestor> mNotificationCallbacks;
  /**
   * Sink interfaces that we implement that mNotificationCallbacks may
   * want to also be notified for.  These are inited lazily if we're
   * asked for the relevant interface.
   */
  nsCOMPtr<nsIChannelEventSink> mChannelEventSink;
  nsCOMPtr<nsIProgressEventSink> mProgressEventSink;

  PRUint32 mState;

  // List of potentially dangerous headers explicitly set using
  // SetRequestHeader.
  nsTArray<nsCString> mExtraRequestHeaders;
};


// helper class to expose a progress DOM Event

class nsXMLHttpProgressEvent : public nsIDOMLSProgressEvent
{
public:
  nsXMLHttpProgressEvent(nsIDOMEvent * aInner, PRUint64 aCurrentProgress, PRUint64 aMaxProgress);
  virtual ~nsXMLHttpProgressEvent();

  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMLSPROGRESSEVENT
  NS_FORWARD_NSIDOMEVENT(mInner->)

protected:
  nsCOMPtr<nsIDOMEvent> mInner;
  PRUint64 mCurProgress;
  PRUint64 mMaxProgress;
};

#endif
