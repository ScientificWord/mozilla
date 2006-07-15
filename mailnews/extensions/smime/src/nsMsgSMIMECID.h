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
 *   Scott MacGregor <mscott@netscape.com>
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

#ifndef nsMsgSMIMECID_h__
#define nsMsgSMIMECID_h__

#include "nsISupports.h"
#include "nsIFactory.h"
#include "nsIComponentManager.h"

#define NS_MSGSMIMECOMPFIELDS_CONTRACTID \
  "@mozilla.org/messenger-smime/composefields;1"

#define NS_MSGSMIMECOMPFIELDS_CID						     \
{ /* 122C919C-96B7-49a0-BBC8-0ABC67EEFFE0 */     \
 0x122c919c, 0x96b7, 0x49a0,                     \
 { 0xbb, 0xc8, 0xa, 0xbc, 0x67, 0xee, 0xff, 0xe0 }}

#define NS_MSGCOMPOSESECURE_CID						       \
{ /* dd753201-9a23-4e08-957f-b3616bf7e012 */     \
 0xdd753201, 0x9a23, 0x4e08,                     \
 {0x95, 0x7f, 0xb3, 0x61, 0x6b, 0xf7, 0xe0, 0x12 }}

#define NS_SMIMEJSHELPER_CONTRACTID \
  "@mozilla.org/messenger-smime/smimejshelper;1"

#define NS_SMIMEJSJELPER_CID                     \
{ /* d57d928c-60e4-4f81-999d-5c762e611205 */     \
 0xd57d928c, 0x60e4, 0x4f81,                     \
 {0x99, 0x9d, 0x5c, 0x76, 0x2e, 0x61, 0x12, 0x05 }}

#define NS_SMIMEENCRYPTURISERVICE_CONTRACTID     \
  "@mozilla.org/messenger-smime/smime-encrypted-uris-service;1"

#define NS_SMIMEENCRYPTURISERVICE_CID            \
{ /* a0134d58-018f-4d40-a099-fa079e5024a6 */     \
 0xa0134d58, 0x018f, 0x4d40,                     \
 {0xa0, 0x99, 0xfa, 0x07, 0x9e, 0x50, 0x24, 0xa6 }}

#endif // nsMsgSMIMECID_h__
