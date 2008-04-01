/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
// vim: ft=cpp tw=78 sw=2 et ts=2
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
 * Boris Zbarsky <bzbarsky@mit.edu>.
 * Portions created by the Initial Developer are Copyright (C) 2003
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

/*
 * A base class which implements nsIImageLoadingContent and can be
 * subclassed by various content nodes that want to provide image
 * loading functionality (eg <img>, <object>, etc).
 */

#ifndef nsImageLoadingContent_h__
#define nsImageLoadingContent_h__

#include "nsIImageLoadingContent.h"
#include "imgIRequest.h"
#include "prtypes.h"
#include "nsCOMPtr.h"
#include "nsString.h"

class nsIURI;
class nsIDocument;
class imgILoader;
class nsIIOService;

class nsImageLoadingContent : public nsIImageLoadingContent
{
  /* METHODS */
public:
  nsImageLoadingContent();
  virtual ~nsImageLoadingContent();

  NS_DECL_IMGICONTAINEROBSERVER
  NS_DECL_IMGIDECODEROBSERVER
  NS_DECL_NSIIMAGELOADINGCONTENT

protected:
  /**
   * LoadImage is called by subclasses when the appropriate
   * attributes (eg 'src' for <img> tags) change.  The string passed
   * in is the new uri string; this consolidates the code for getting
   * the charset, constructing URI objects, and any other incidentals
   * into this superclass.   
   *
   * @param aNewURI the URI spec to be loaded (may be a relative URI)
   * @param aForce If true, make sure to load the URI.  If false, only
   *        load if the URI is different from the currently loaded URI.
   * @param aNotify If true, nsIDocumentObserver state change notifications
   *                will be sent as needed.
   */
  nsresult LoadImage(const nsAString& aNewURI, PRBool aForce,
                     PRBool aNotify);

  /**
   * ImageState is called by subclasses that are computing their content state.
   * The return value will have the NS_EVENT_STATE_BROKEN,
   * NS_EVENT_STATE_USERDISABLED, and NS_EVENT_STATE_SUPPRESSED bits set as
   * needed.  Note that this state assumes that this node is "trying" to be an
   * image (so for example complete lack of attempt to load an image will lead
   * to NS_EVENT_STATE_BROKEN being set).  Subclasses that are not "trying" to
   * be an image (eg an HTML <input> of type other than "image") should just
   * not call this method when computing their intrinsic state.
   */
  PRInt32 ImageState() const;

  /**
   * LoadImage is called by subclasses when the appropriate
   * attributes (eg 'src' for <img> tags) change. If callers have an
   * URI object already available, they should use this method.
   *
   * @param aNewURI the URI to be loaded
   * @param aForce If true, make sure to load the URI.  If false, only
   *        load if the URI is different from the currently loaded URI.
   * @param aNotify If true, nsIDocumentObserver state change notifications
   *                will be sent as needed.
   * @param aDocument Optional parameter giving the document this node is in.
   *        This is purely a performance optimization.
   * @param aLoadFlags Optional parameter specifying load flags to use for
   *        the image load
   */
  nsresult LoadImage(nsIURI* aNewURI, PRBool aForce, PRBool aNotify,
                     nsIDocument* aDocument = nsnull,
                     nsLoadFlags aLoadFlags = nsIRequest::LOAD_NORMAL);

  /**
   * helper to get the document for this content (from the nodeinfo
   * and such).  Not named GetDocument to prevent ambiguous method
   * names in subclasses
   *
   * @return the document we belong to
   */
  nsIDocument* GetOurDocument();

  /**
   * CancelImageRequests is called by subclasses when they want to
   * cancel all image requests (for example when the subclass is
   * somehow not an image anymore).
   */
  void CancelImageRequests(PRBool aNotify);

  /**
   * UseAsPrimaryRequest is called by subclasses when they have an existing
   * imgIRequest that they want this nsImageLoadingContent to use.  This may
   * effectively be called instead of LoadImage or LoadImageWithChannel.
   * If aNotify is true, this method will notify on state changes.
   */
  nsresult UseAsPrimaryRequest(imgIRequest* aRequest, PRBool aNotify);

  /**
   * Derived classes of nsImageLoadingContent MUST call
   * DestroyImageLoadingContent from their destructor, or earlier.  It
   * does things that cannot be done in ~nsImageLoadingContent because
   * they rely on being able to QueryInterface to other derived classes,
   * which cannot happen once the derived class destructor has started
   * calling the base class destructors.
   */
  void DestroyImageLoadingContent();

private:
  /**
   * Struct used to manage the image observers.
   */
  struct ImageObserver {
    ImageObserver(imgIDecoderObserver* aObserver) :
      mObserver(aObserver),
      mNext(nsnull)
    {
      MOZ_COUNT_CTOR(ImageObserver);
    }
    ~ImageObserver()
    {
      MOZ_COUNT_DTOR(ImageObserver);
      delete mNext;
    }

    nsCOMPtr<imgIDecoderObserver> mObserver;
    ImageObserver* mNext;
  };

  /**
   * Struct to report state changes
   */
  struct AutoStateChanger {
    AutoStateChanger(nsImageLoadingContent* aImageContent,
                     PRBool aNotify) :
      mImageContent(aImageContent),
      mNotify(aNotify)
    {
      NS_ASSERTION(!mImageContent->mStartingLoad,
                   "Nested AutoStateChangers somehow?");
      mImageContent->mStartingLoad = PR_TRUE;
    }
    ~AutoStateChanger()
    {
      mImageContent->mStartingLoad = PR_FALSE;
      mImageContent->UpdateImageState(mNotify);
    }

    nsImageLoadingContent* mImageContent;
    PRBool mNotify;
  };

  friend struct AutoStateChanger;

  /**
   * UpdateImageState recomputes the current state of this image loading
   * content and updates what ImageState() returns accordingly.  It will also
   * fire a ContentStatesChanged() notification as needed if aNotify is true.
   */
  void UpdateImageState(PRBool aNotify);

  /**
   * CancelImageRequests can be called when we want to cancel the
   * image requests, generally due to our src changing and us wanting
   * to start a new load.  The "current" request will be canceled only
   * if it has not progressed far enough to know the image size yet
   * unless aEvenIfSizeAvailable is true.
   *
   * @param aReason the reason the requests are being canceled
   * @param aEvenIfSizeAvailable cancels the current load even if its size is
   *                             available
   * @param aNewImageStatus the nsIContentPolicy status of the new image load
   */
  void CancelImageRequests(nsresult aReason, PRBool aEvenIfSizeAvailable,
                           PRInt16 aNewImageStatus);

  /**
   * Method to create an nsIURI object from the given string (will
   * handle getting the right charset, base, etc).  You MUST pass in a
   * non-null document to this function.
   *
   * @param aSpec the string spec (from an HTML attribute, eg)
   * @param aDocument the document we belong to
   * @return the URI we want to be loading
   */
  nsresult StringToURI(const nsAString& aSpec, nsIDocument* aDocument,
                       nsIURI** aURI);

  /**
   * Method to fire an event once we know what's going on with the image load.
   *
   * @param aEventType "load" or "error" depending on how things went
   */
  nsresult FireEvent(const nsAString& aEventType);
  class Event;
  friend class Event;

  /* MEMBERS */
protected:
  nsCOMPtr<imgIRequest> mCurrentRequest;
  nsCOMPtr<imgIRequest> mPendingRequest;
  nsCOMPtr<nsIURI>      mCurrentURI;

private:
  /**
   * Typically we will have only one observer (our frame in the screen
   * prescontext), so we want to only make space for one and to
   * heap-allocate anything past that (saves memory and malloc churn
   * in the common case).  The storage is a linked list, we just
   * happen to actually hold the first observer instead of a pointer
   * to it.
   */
  ImageObserver mObserverList;

  PRInt16 mImageBlockingStatus;
  PRPackedBool mLoadingEnabled : 1;
  PRPackedBool mStartingLoad : 1;

  /**
   * The state we had the last time we checked whether we needed to notify the
   * document of a state change.  These are maintained by UpdateImageState.
   */
  PRPackedBool mLoading : 1;
  PRPackedBool mBroken : 1;
  PRPackedBool mUserDisabled : 1;
  PRPackedBool mSuppressed : 1;
};

#endif // nsImageLoadingContent_h__
