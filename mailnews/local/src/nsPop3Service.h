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

#ifndef nsPop3Service_h___
#define nsPop3Service_h___

#include "nscore.h"

#include "nsIPop3Service.h"
#include "nsIPop3URL.h"
#include "nsIUrlListener.h"
#include "nsIStreamListener.h"
#include "nsFileSpec.h"
#include "nsIProtocolHandler.h"
#include "nsIMsgProtocolInfo.h"


class nsPop3Service : public nsIPop3Service,
                      public nsIProtocolHandler,
                      public nsIMsgProtocolInfo
{
public:

	nsPop3Service();
	virtual ~nsPop3Service();
	
	NS_DECL_ISUPPORTS
    NS_DECL_NSIPOP3SERVICE
    NS_DECL_NSIPROTOCOLHANDLER
    NS_DECL_NSIMSGPROTOCOLINFO

protected:
        nsresult GetMail(PRBool downloadNewMail,
                         nsIMsgWindow* aMsgWindow, 
                         nsIUrlListener * aUrlListener,
                         nsIMsgFolder *inbox, 
                         nsIPop3IncomingServer *popServer,
                         nsIURI ** aURL);
	// convience function to make constructing of the pop3 url easier...
	nsresult BuildPop3Url(const char * urlSpec, nsIMsgFolder *inbox,
                          nsIPop3IncomingServer *, nsIUrlListener * aUrlListener,
                          nsIURI ** aUrl, nsIMsgWindow *aMsgWindow);

	nsresult RunPopUrl(nsIMsgIncomingServer * aServer, nsIURI * aUrlToRun);
};

#endif /* nsPop3Service_h___ */
