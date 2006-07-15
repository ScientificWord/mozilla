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
 * The Original Code is the Netscape security libraries.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1994-2000
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


#ifndef _P12LOCAL_H_
#define _P12LOCAL_H_

#include "plarena.h"
#include "secoidt.h"
#include "secasn1.h"
#include "secder.h"
#include "certt.h"
#include "secpkcs7.h"
#include "pkcs12.h"
#include "p12.h"

/* helper functions */
extern const SEC_ASN1Template *
sec_pkcs12_choose_bag_type(void *src_or_dest, PRBool encoding);
extern const SEC_ASN1Template *
sec_pkcs12_choose_cert_crl_type(void *src_or_dest, PRBool encoding);
extern const SEC_ASN1Template *
sec_pkcs12_choose_shroud_type(void *src_or_dest, PRBool encoding);
extern SECItem *sec_pkcs12_generate_salt(void);
extern SECItem *sec_pkcs12_generate_key_from_password(SECOidTag algorithm, 
	SECItem *salt, SECItem *password);
extern SECItem *sec_pkcs12_generate_mac(SECItem *key, SECItem *msg, 
					PRBool old_method);
extern SGNDigestInfo *sec_pkcs12_compute_thumbprint(SECItem *der_cert);
extern SECItem *sec_pkcs12_create_virtual_password(SECItem *password, 
			SECItem *salt, PRBool swapUnicodeBytes);
extern SECStatus sec_pkcs12_append_shrouded_key(SEC_PKCS12BaggageItem *bag,
						 SEC_PKCS12ESPVKItem *espvk);
extern void *sec_pkcs12_find_object(SEC_PKCS12SafeContents *safe,
				SEC_PKCS12Baggage *baggage, SECOidTag objType,
				SECItem *nickname, SGNDigestInfo *thumbprint);
extern PRBool sec_pkcs12_convert_item_to_unicode(PRArenaPool *arena, SECItem *dest,
						 SECItem *src, PRBool zeroTerm,
						 PRBool asciiConvert, PRBool toUnicode);
extern CK_MECHANISM_TYPE sec_pkcs12_algtag_to_mech(SECOidTag algtag);

/* create functions */
extern SEC_PKCS12PFXItem *sec_pkcs12_new_pfx(void);
extern SEC_PKCS12SafeContents *sec_pkcs12_create_safe_contents(
	PRArenaPool *poolp);
extern SEC_PKCS12Baggage *sec_pkcs12_create_baggage(PRArenaPool *poolp);
extern SEC_PKCS12BaggageItem *sec_pkcs12_create_external_bag(SEC_PKCS12Baggage *luggage);
extern void SEC_PKCS12DestroyPFX(SEC_PKCS12PFXItem *pfx);
extern SEC_PKCS12AuthenticatedSafe *sec_pkcs12_new_asafe(PRArenaPool *poolp);

/* conversion from old to new */
extern SEC_PKCS12DecoderContext *
sec_PKCS12ConvertOldSafeToNew(PRArenaPool *arena, PK11SlotInfo *slot,
			      PRBool swapUnicode, SECItem *pwitem,
			      void *wincx, SEC_PKCS12SafeContents *safe,
			      SEC_PKCS12Baggage *baggage);
	
#endif
