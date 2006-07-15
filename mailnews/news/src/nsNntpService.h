/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
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
 * either of the GNU General Public License Version 2 or later (the "GPL"),
 * or the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
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

#ifndef nsNntpService_h___
#define nsNntpService_h___

#include "nsINntpService.h"
#include "nsIProtocolHandler.h"
#include "nsIMsgMessageService.h"
#include "nsINntpIncomingServer.h"
#include "nsIMsgIncomingServer.h"
#include "nsIFileSpec.h"
#include "MailNewsTypes.h"
#include "nsIMsgProtocolInfo.h"
#include "nsIMsgWindow.h"
#include "nsINntpUrl.h"
#include "nsCOMPtr.h"
#include "nsIContentHandler.h"
#include "nsICacheSession.h"

#ifdef MOZ_XUL_APP
#include "nsICommandLineHandler.h"
#define ICOMMANDLINEHANDLER nsICommandLineHandler
#else
#include "nsICmdLineHandler.h"
#define ICOMMANDLINEHANDLER nsICmdLineHandler
#endif

class nsIURI;
class nsIUrlListener;

class nsNntpService : public nsINntpService,
                      public nsIMsgMessageService,
                      public nsIMsgMessageFetchPartService,
                      public nsIProtocolHandler,
                      public nsIMsgProtocolInfo,
                      public ICOMMANDLINEHANDLER,
                      public nsIContentHandler
{
public:

  NS_DECL_ISUPPORTS
  NS_DECL_NSINNTPSERVICE
  NS_DECL_NSIMSGMESSAGESERVICE
  NS_DECL_NSIPROTOCOLHANDLER
  NS_DECL_NSIMSGPROTOCOLINFO
  NS_DECL_NSICONTENTHANDLER
  NS_DECL_NSIMSGMESSAGEFETCHPARTSERVICE
  
#ifdef MOZ_XUL_APP
  NS_DECL_NSICOMMANDLINEHANDLER
#else
  NS_DECL_NSICMDLINEHANDLER
  CMDLINEHANDLER_REGISTERPROC_DECLS
#endif

  // nsNntpService
  nsNntpService();
  virtual ~nsNntpService();

protected:
  PRBool WeAreOffline();

  nsresult GetNntpServerByAccount(const char *aAccountKey, nsIMsgIncomingServer **aNntpServer);
  nsresult SetUpNntpUrlForPosting(const char *aAccountKey, char **newsUrlSpec);
  nsresult FindHostFromGroup(nsCString &host, nsCString &groupName);
  nsresult FindServerWithNewsgroup(nsCString &host, nsCString &groupName);
  
  nsresult CreateMessageIDURL(nsIMsgFolder *folder, nsMsgKey key, char **url);
  // a convience routine used to put together news urls
  nsresult ConstructNntpUrl(const char * urlString, nsIUrlListener *aUrlListener,  nsIMsgWindow * aMsgWindow, const char *originalMessageUri, PRInt32 action, nsIURI ** aUrl);
  nsresult CreateNewsAccount(const char *aHostname, PRBool aIsSecure, PRInt32 aPort, nsIMsgIncomingServer **aServer);
  nsresult GetProtocolForUri(nsIURI *aUri, nsIMsgWindow *aMsgWindow, nsINNTPProtocol **aProtocol);
  // a convience routine to run news urls
  nsresult RunNewsUrl (nsIURI * aUrl, nsIMsgWindow *aMsgWindow, nsISupports * aConsumer);
  // a convience routine to go from folder uri to msg folder
  nsresult GetFolderFromUri(const char *uri, nsIMsgFolder **folder);
  static PRBool findNewsServerWithGroup(nsISupports *aElement, void *data);
  nsresult DecomposeNewsMessageURI(const char * aMessageURI, nsIMsgFolder ** aFolder, nsMsgKey *aMsgKey);

  PRBool            mPrintingOperation; // Flag for printing operations
  PRBool			mOpenAttachmentOperation; // Flag for opening attachments

  nsCOMPtr<nsICacheSession> mCacheSession; // the cache session used by news
};

#endif /* nsNntpService_h___ */
