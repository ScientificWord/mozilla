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
 * Contributor(s):
 *  Dave Townsend <dtownsend@oxymoronical.com>
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

#ifndef _NS_DATASIGNATUREVERIFIER_H_
#define _NS_DATASIGNATUREVERIFIER_H_

#include "nsIDataSignatureVerifier.h"

#include "keythi.h"

// 296d76aa-275b-4f3c-af8a-30a4026c18fc
#define NS_DATASIGNATUREVERIFIER_CID \
    { 0x296d76aa, 0x275b, 0x4f3c, \
    { 0xaf, 0x8a, 0x30, 0xa4, 0x02, 0x6c, 0x18, 0xfc } }
#define NS_DATASIGNATUREVERIFIER_CONTRACTID \
    "@mozilla.org/security/datasignatureverifier;1"

class nsDataSignatureVerifier : public nsIDataSignatureVerifier
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDATASIGNATUREVERIFIER

  nsDataSignatureVerifier()
  {
  }

private:
  ~nsDataSignatureVerifier()
  {
  }
};

#endif // _NS_DATASIGNATUREVERIFIER_H_
