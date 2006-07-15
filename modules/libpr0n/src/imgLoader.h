/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * ***** BEGIN LICENSE BLOCK *****
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
 * Portions created by the Initial Developer are Copyright (C) 2001
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Stuart Parmenter <pavlov@netscape.com>
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

#include "imgILoader.h"
#include "nsIContentSniffer.h"

#ifdef LOADER_THREADSAFE
#include "prlock.h"
#endif

class imgRequest;
class imgRequestProxy;
class imgIRequest;
class imgIDecoderObserver;
class nsILoadGroup;

#define NS_IMGLOADER_CID \
{ /* 9f6a0d2e-1dd1-11b2-a5b8-951f13c846f7 */         \
     0x9f6a0d2e,                                     \
     0x1dd1,                                         \
     0x11b2,                                         \
    {0xa5, 0xb8, 0x95, 0x1f, 0x13, 0xc8, 0x46, 0xf7} \
}

class imgLoader : public imgILoader, public nsIContentSniffer
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_IMGILOADER
  NS_DECL_NSICONTENTSNIFFER

  imgLoader();
  virtual ~imgLoader();

  static nsresult GetMimeTypeFromContent(const char* aContents, PRUint32 aLength, nsACString& aContentType);

private:
  nsresult CreateNewProxyForRequest(imgRequest *aRequest, nsILoadGroup *aLoadGroup,
                                    imgIDecoderObserver *aObserver,
                                    nsLoadFlags aLoadFlags, imgIRequest *aRequestProxy,
                                    imgIRequest **_retval);
};



/**
 * proxy stream listener class used to handle multipart/x-mixed-replace
 */

#include "nsCOMPtr.h"
#include "nsIStreamListener.h"

class ProxyListener : public nsIStreamListener
{
public:
  ProxyListener(nsIStreamListener *dest);
  virtual ~ProxyListener();

  /* additional members */
  NS_DECL_ISUPPORTS
  NS_DECL_NSISTREAMLISTENER
  NS_DECL_NSIREQUESTOBSERVER

private:
  nsCOMPtr<nsIStreamListener> mDestListener;
};


/**
 * validate checker
 */

#include "nsCOMArray.h"

class imgCacheValidator : public nsIStreamListener
{
public:
  imgCacheValidator(imgRequest *request, void *aContext);
  virtual ~imgCacheValidator();

  void AddProxy(imgRequestProxy *aProxy);

  /* additional members */
  NS_DECL_ISUPPORTS
  NS_DECL_NSISTREAMLISTENER
  NS_DECL_NSIREQUESTOBSERVER

private:
  nsCOMPtr<nsIStreamListener> mDestListener;

  imgRequest *mRequest;
  nsCOMArray<imgIRequest> mProxies;

  void *mContext;
};
