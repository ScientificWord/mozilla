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

/*
 * Support routines for CMS implementation, none of which are exported.
 *
 * Do not export this file!  If something in here is really needed outside
 * of smime code, first try to add a CMS interface which will do it for
 * you.  If that has a problem, then just move out what you need, changing
 * its name as appropriate!
 *
 * $Id$
 */

#ifndef _CMSLOCAL_H_
#define _CMSLOCAL_H_

#include "cms.h"
#include "cmsreclist.h"
#include "secasn1t.h"

extern const SEC_ASN1Template NSSCMSContentInfoTemplate[];

/************************************************************************/
SEC_BEGIN_PROTOS

/***********************************************************************
 * cmscipher.c - en/decryption routines
 ***********************************************************************/

/*
 * NSS_CMSCipherContext_StartDecrypt - create a cipher context to do decryption
 * based on the given bulk * encryption key and algorithm identifier (which may include an iv).
 */
extern NSSCMSCipherContext *
NSS_CMSCipherContext_StartDecrypt(PK11SymKey *key, SECAlgorithmID *algid);

/*
 * NSS_CMSCipherContext_StartEncrypt - create a cipher object to do encryption,
 * based on the given bulk encryption key and algorithm tag.  Fill in the algorithm
 * identifier (which may include an iv) appropriately.
 */
extern NSSCMSCipherContext *
NSS_CMSCipherContext_StartEncrypt(PRArenaPool *poolp, PK11SymKey *key, SECAlgorithmID *algid);

extern void
NSS_CMSCipherContext_Destroy(NSSCMSCipherContext *cc);

/*
 * NSS_CMSCipherContext_DecryptLength - find the output length of the next call to decrypt.
 *
 * cc - the cipher context
 * input_len - number of bytes used as input
 * final - true if this is the final chunk of data
 *
 * Result can be used to perform memory allocations.  Note that the amount
 * is exactly accurate only when not doing a block cipher or when final
 * is false, otherwise it is an upper bound on the amount because until
 * we see the data we do not know how many padding bytes there are
 * (always between 1 and bsize).
 */
extern unsigned int
NSS_CMSCipherContext_DecryptLength(NSSCMSCipherContext *cc, unsigned int input_len, PRBool final);

/*
 * NSS_CMSCipherContext_EncryptLength - find the output length of the next call to encrypt.
 *
 * cc - the cipher context
 * input_len - number of bytes used as input
 * final - true if this is the final chunk of data
 *
 * Result can be used to perform memory allocations.
 */
extern unsigned int
NSS_CMSCipherContext_EncryptLength(NSSCMSCipherContext *cc, unsigned int input_len, PRBool final);

/*
 * NSS_CMSCipherContext_Decrypt - do the decryption
 *
 * cc - the cipher context
 * output - buffer for decrypted result bytes
 * output_len_p - number of bytes in output
 * max_output_len - upper bound on bytes to put into output
 * input - pointer to input bytes
 * input_len - number of input bytes
 * final - true if this is the final chunk of data
 *
 * Decrypts a given length of input buffer (starting at "input" and
 * containing "input_len" bytes), placing the decrypted bytes in
 * "output" and storing the output length in "*output_len_p".
 * "cc" is the return value from NSS_CMSCipher_StartDecrypt.
 * When "final" is true, this is the last of the data to be decrypted.
 */ 
extern SECStatus
NSS_CMSCipherContext_Decrypt(NSSCMSCipherContext *cc, unsigned char *output,
		  unsigned int *output_len_p, unsigned int max_output_len,
		  const unsigned char *input, unsigned int input_len,
		  PRBool final);

/*
 * NSS_CMSCipherContext_Encrypt - do the encryption
 *
 * cc - the cipher context
 * output - buffer for decrypted result bytes
 * output_len_p - number of bytes in output
 * max_output_len - upper bound on bytes to put into output
 * input - pointer to input bytes
 * input_len - number of input bytes
 * final - true if this is the final chunk of data
 *
 * Encrypts a given length of input buffer (starting at "input" and
 * containing "input_len" bytes), placing the encrypted bytes in
 * "output" and storing the output length in "*output_len_p".
 * "cc" is the return value from NSS_CMSCipher_StartEncrypt.
 * When "final" is true, this is the last of the data to be encrypted.
 */ 
extern SECStatus
NSS_CMSCipherContext_Encrypt(NSSCMSCipherContext *cc, unsigned char *output,
		  unsigned int *output_len_p, unsigned int max_output_len,
		  const unsigned char *input, unsigned int input_len,
		  PRBool final);

/************************************************************************
 * cmspubkey.c - public key operations
 ************************************************************************/

/*
 * NSS_CMSUtil_EncryptSymKey_RSA - wrap a symmetric key with RSA
 *
 * this function takes a symmetric key and encrypts it using an RSA public key
 * according to PKCS#1 and RFC2633 (S/MIME)
 */
extern SECStatus
NSS_CMSUtil_EncryptSymKey_RSA(PLArenaPool *poolp, CERTCertificate *cert,
                              PK11SymKey *key,
                              SECItem *encKey);

extern SECStatus
NSS_CMSUtil_EncryptSymKey_RSAPubKey(PLArenaPool *poolp,
                                    SECKEYPublicKey *publickey,
                                    PK11SymKey *bulkkey, SECItem *encKey);

/*
 * NSS_CMSUtil_DecryptSymKey_RSA - unwrap a RSA-wrapped symmetric key
 *
 * this function takes an RSA-wrapped symmetric key and unwraps it, returning a symmetric
 * key handle. Please note that the actual unwrapped key data may not be allowed to leave
 * a hardware token...
 */
extern PK11SymKey *
NSS_CMSUtil_DecryptSymKey_RSA(SECKEYPrivateKey *privkey, SECItem *encKey, SECOidTag bulkalgtag);

extern SECStatus
NSS_CMSUtil_EncryptSymKey_MISSI(PLArenaPool *poolp, CERTCertificate *cert, PK11SymKey *key,
			SECOidTag symalgtag, SECItem *encKey, SECItem **pparams, void *pwfn_arg);

extern PK11SymKey *
NSS_CMSUtil_DecryptSymKey_MISSI(SECKEYPrivateKey *privkey, SECItem *encKey,
			SECAlgorithmID *keyEncAlg, SECOidTag bulkalgtag, void *pwfn_arg);

extern SECStatus
NSS_CMSUtil_EncryptSymKey_ESDH(PLArenaPool *poolp, CERTCertificate *cert, PK11SymKey *key,
			SECItem *encKey, SECItem **ukm, SECAlgorithmID *keyEncAlg,
			SECItem *originatorPubKey);

extern PK11SymKey *
NSS_CMSUtil_DecryptSymKey_ESDH(SECKEYPrivateKey *privkey, SECItem *encKey,
			SECAlgorithmID *keyEncAlg, SECOidTag bulkalgtag, void *pwfn_arg);

/************************************************************************
 * cmsreclist.c - recipient list stuff
 ************************************************************************/
extern NSSCMSRecipient **nss_cms_recipient_list_create(NSSCMSRecipientInfo **recipientinfos);
extern void nss_cms_recipient_list_destroy(NSSCMSRecipient **recipient_list);
extern NSSCMSRecipientEncryptedKey *NSS_CMSRecipientEncryptedKey_Create(PLArenaPool *poolp);

/************************************************************************
 * cmsarray.c - misc array functions
 ************************************************************************/
/*
 * NSS_CMSArray_Alloc - allocate an array in an arena
 */
extern void **
NSS_CMSArray_Alloc(PRArenaPool *poolp, int n);

/*
 * NSS_CMSArray_Add - add an element to the end of an array
 */
extern SECStatus
NSS_CMSArray_Add(PRArenaPool *poolp, void ***array, void *obj);

/*
 * NSS_CMSArray_IsEmpty - check if array is empty
 */
extern PRBool
NSS_CMSArray_IsEmpty(void **array);

/*
 * NSS_CMSArray_Count - count number of elements in array
 */
extern int
NSS_CMSArray_Count(void **array);

/*
 * NSS_CMSArray_Sort - sort an array ascending, in place
 *
 * If "secondary" is not NULL, the same reordering gets applied to it.
 * If "tertiary" is not NULL, the same reordering gets applied to it.
 * "compare" is a function that returns 
 *  < 0 when the first element is less than the second
 *  = 0 when the first element is equal to the second
 *  > 0 when the first element is greater than the second
 */
extern void
NSS_CMSArray_Sort(void **primary, int (*compare)(void *,void *), void **secondary, void **tertiary);

/************************************************************************
 * cmsattr.c - misc attribute functions
 ************************************************************************/
/*
 * NSS_CMSAttribute_Create - create an attribute
 *
 * if value is NULL, the attribute won't have a value. It can be added later
 * with NSS_CMSAttribute_AddValue.
 */
extern NSSCMSAttribute *
NSS_CMSAttribute_Create(PRArenaPool *poolp, SECOidTag oidtag, SECItem *value, PRBool encoded);

/*
 * NSS_CMSAttribute_AddValue - add another value to an attribute
 */
extern SECStatus
NSS_CMSAttribute_AddValue(PLArenaPool *poolp, NSSCMSAttribute *attr, SECItem *value);

/*
 * NSS_CMSAttribute_GetType - return the OID tag
 */
extern SECOidTag
NSS_CMSAttribute_GetType(NSSCMSAttribute *attr);

/*
 * NSS_CMSAttribute_GetValue - return the first attribute value
 *
 * We do some sanity checking first:
 * - Multiple values are *not* expected.
 * - Empty values are *not* expected.
 */
extern SECItem *
NSS_CMSAttribute_GetValue(NSSCMSAttribute *attr);

/*
 * NSS_CMSAttribute_CompareValue - compare the attribute's first value against data
 */
extern PRBool
NSS_CMSAttribute_CompareValue(NSSCMSAttribute *attr, SECItem *av);

/*
 * NSS_CMSAttributeArray_Encode - encode an Attribute array as SET OF Attributes
 *
 * If you are wondering why this routine does not reorder the attributes
 * first, and might be tempted to make it do so, see the comment by the
 * call to ReorderAttributes in cmsencode.c.  (Or, see who else calls this
 * and think long and hard about the implications of making it always
 * do the reordering.)
 */
extern SECItem *
NSS_CMSAttributeArray_Encode(PRArenaPool *poolp, NSSCMSAttribute ***attrs, SECItem *dest);

/*
 * NSS_CMSAttributeArray_Reorder - sort attribute array by attribute's DER encoding
 *
 * make sure that the order of the attributes guarantees valid DER (which must be
 * in lexigraphically ascending order for a SET OF); if reordering is necessary it
 * will be done in place (in attrs).
 */
extern SECStatus
NSS_CMSAttributeArray_Reorder(NSSCMSAttribute **attrs);

/*
 * NSS_CMSAttributeArray_FindAttrByOidTag - look through a set of attributes and
 * find one that matches the specified object ID.
 *
 * If "only" is true, then make sure that there is not more than one attribute
 * of the same type.  Otherwise, just return the first one found. (XXX Does
 * anybody really want that first-found behavior?  It was like that when I found it...)
 */
extern NSSCMSAttribute *
NSS_CMSAttributeArray_FindAttrByOidTag(NSSCMSAttribute **attrs, SECOidTag oidtag, PRBool only);

/*
 * NSS_CMSAttributeArray_AddAttr - add an attribute to an
 * array of attributes. 
 */
extern SECStatus
NSS_CMSAttributeArray_AddAttr(PLArenaPool *poolp, NSSCMSAttribute ***attrs, NSSCMSAttribute *attr);

/*
 * NSS_CMSAttributeArray_SetAttr - set an attribute's value in a set of attributes
 */
extern SECStatus
NSS_CMSAttributeArray_SetAttr(PLArenaPool *poolp, NSSCMSAttribute ***attrs, SECOidTag type, SECItem *value, PRBool encoded);

/*
 * NSS_CMSSignedData_AddTempCertificate - add temporary certificate references.
 * They may be needed for signature verification on the data, for example.
 */
extern SECStatus
NSS_CMSSignedData_AddTempCertificate(NSSCMSSignedData *sigd, CERTCertificate *cert);

/************************************************************************/
SEC_END_PROTOS

#endif /* _CMSLOCAL_H_ */
