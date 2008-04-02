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
 * Portions created by the Initial Developer are Copyright (C) 2006
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Tony Chang <tc@google.com>
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

#ifndef _NS_STREAMCIPHER_H_
#define _NS_STREAMCIPHER_H_

#include "nsIStreamCipher.h"
#include "nsString.h"
#include "pk11func.h"

#define NS_STREAMCIPHER_CLASSNAME  "Stream Cipher Component"
/* dbfcbe4a-10f7-4d6f-a481-68e6d6b71d21 */
#define NS_STREAMCIPHER_CID   \
{ 0xdbfcbe4a, 0x10f7, 0x4d6f, {0xa4, 0x81, 0x68, 0xe6, 0xd6, 0xb7, 0x1d, 0x21}}
#define NS_STREAMCIPHER_CONTRACTID "@mozilla.org/security/streamcipher;1"

class nsStreamCipher : public nsIStreamCipher
{
public:
  nsStreamCipher();

  NS_DECL_ISUPPORTS
  NS_DECL_NSISTREAMCIPHER

private:
  ~nsStreamCipher();

  // Helper method for initializing this object.
  // aIV may be null.
  nsresult InitWithIV_(nsIKeyObject *aKey, SECItem* aIV);
  
  // Disallow copy constructor
  nsStreamCipher(nsStreamCipher&);

  // Holds our stream cipher context.
  PK11Context* mContext;

  // Holds the amount we've computed so far.
  nsCString mValue;
};

#endif // _NS_STREAMCIPHER_H_
