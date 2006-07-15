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
 * The Original Code is Mozilla Communicator client code.
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
#ifndef __nsInternetConfigService_h
#define __nsInternetConfigService_h

#include "nsIInternetConfigService.h"
#include "nsInternetConfig.h"

#define NS_INTERNETCONFIGSERVICE_CID \
  {0x9b8b9d81, 0x5f4f, 0x11d4, \
    { 0x96, 0x96, 0x00, 0x60, 0x08, 0x3a, 0x0b, 0xcf }}

class nsInternetConfigService : public nsIInternetConfigService
{
public:

  NS_DECL_ISUPPORTS
  NS_DECL_NSIINTERNETCONFIGSERVICE

  nsresult GetMappingForMIMEType(const char *mimetype, const char *fileextension, ICMapEntry *entry);

  nsInternetConfigService();
  virtual ~nsInternetConfigService();
protected:
  // some private helper methods...
  nsresult FillMIMEInfoForICEntry(ICMapEntry& entry, nsIMIMEInfo ** mimeinfo);
  
  nsresult GetICKeyPascalString(PRUint32 inIndex, const unsigned char*& outICKey);
  nsresult GetICPreference(PRUint32 inKey, void *outData, long *ioSize);
};

#endif
