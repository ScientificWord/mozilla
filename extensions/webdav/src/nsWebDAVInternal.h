/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
// vim:expandtab:ts=4 sw=4:
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
 * Oracle Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2004
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Mike Shaver <shaver@off.net> (original author)
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

#ifndef nsWebDAVInternal_h__
#define nsWebDAVInternal_h__

#if defined(MOZ_LOGGING)
#define FORCE_PR_LOG
#endif

#include "prlog.h"
#include "nsCOMPtr.h"
#include "nsString.h"

class nsIStreamListener;
class nsIRequestObserver;
class nsIOutputStream;
class nsIDOMNode;
class nsIDOMElement;
class nsIDOMDocument;
class nsIDOMNodeList;

#include "nsIWebDAVListener.h"
#include "nsIWebDAVResource.h"

#define LOG(args) PR_LOG(gDAVLog, 4, args)
#define LOG_ENABLED() PR_LOG_TEST(gDAVLog, 4)

extern PRLogModuleInfo *gDAVLog;

nsresult
NS_WD_NewOperationStreamListener(nsIWebDAVResource *resource,
                                 nsIWebDAVOperationListener *listener,
                                 nsISupports *closure,
                                 PRUint32 operation,
                                 nsIStreamListener **streamListener);

nsIStreamListener *
NS_WD_NewPropfindStreamListener(nsIWebDAVResource *resource,
                                nsIWebDAVOperationListener *listener,
                                nsISupports *closure,
                                PRBool isPropname);

nsIStreamListener *
NS_WD_NewReportStreamListener(nsIWebDAVResource *resource,
                              nsIWebDAVOperationListener *listener,
                              nsISupports *closure);

nsresult
NS_WD_NewGetOperationRequestObserver(nsIWebDAVResource *resource,
                                     nsIWebDAVOperationListener *listener,
                                     nsISupports *closure,
                                     nsIOutputStream *outstream,
                                     nsIRequestObserver **observer);

nsresult
NS_WD_NewGetToStringOperationRequestObserver(nsIWebDAVResource *resource,
                                             nsIWebDAVOperationListener *listener,
                                             nsISupports *closure,
                                             nsIStreamListener **streamListener);

nsresult
NS_WD_GetElementByTagName(nsIDOMElement *parentElt, const nsAString &tagName,
                          nsIDOMElement **elt);

nsresult
NS_WD_ElementTextChildValue(nsIDOMElement *elt, const nsAString &tagName,
                            nsAString &value);

nsresult
NS_WD_AppendElementWithNS(nsIDOMDocument *doc, nsIDOMNode *parent,
                          const nsAString& ns, const nsAString& tagName,
                          nsIDOMElement **child);

nsresult
NS_WD_GetDocAndResponseListFromBuffer(const nsACString &buffer,
                                      nsIDOMDocument **xmldoc,
                                      nsIDOMNodeList **responseList,
                                      PRUint32 *length);
#endif /* nsWebDAVInternal_h__ */
