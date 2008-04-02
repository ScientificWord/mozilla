/*
 * softoken.h - private data structures and prototypes for the softoken lib
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
/* $Id$ */

#ifndef _SOFTOKEN_H_
#define _SOFTOKEN_H_

#include "blapi.h"
#include "lowkeyti.h"
#include "softoknt.h"
#include "secoidt.h"

#include "pkcs11t.h"     /* CK_RV Required for sftk_fipsPowerUpSelfTest(). */

SEC_BEGIN_PROTOS

/*
** RSA encryption/decryption. When encrypting/decrypting the output
** buffer must be at least the size of the public key modulus.
*/

/*
** Format some data into a PKCS#1 encryption block, preparing the
** data for RSA encryption.
**	"result" where the formatted block is stored (memory is allocated)
**	"modulusLen" the size of the formatted block
**	"blockType" what block type to use (SEC_RSABlock*)
**	"data" the data to format
*/
extern SECStatus RSA_FormatBlock(SECItem *result,
				 unsigned int modulusLen,
				 RSA_BlockType blockType,
				 SECItem *data);
/*
** Similar, but just returns a pointer to the allocated memory, *and*
** will *only* format one block, even if we (in the future) modify
** RSA_FormatBlock() to loop over multiples of modulusLen.
*/
extern unsigned char *RSA_FormatOneBlock(unsigned int modulusLen,
					 RSA_BlockType blockType,
					 SECItem *data);



/*
 * convenience wrappers for doing single RSA operations. They create the
 * RSA context internally and take care of the formatting
 * requirements. Blinding happens automagically within RSA_Sign and
 * RSA_DecryptBlock.
 */
extern
SECStatus RSA_Sign(NSSLOWKEYPrivateKey *key, unsigned char *output,
		       unsigned int *outputLen, unsigned int maxOutputLen,
		       unsigned char *input, unsigned int inputLen);
extern
SECStatus RSA_HashSign(SECOidTag hashOid,
			NSSLOWKEYPrivateKey *key, unsigned char *sig,
			unsigned int *sigLen, unsigned int maxLen,
			unsigned char *hash, unsigned int hashLen);
extern
SECStatus RSA_CheckSign(NSSLOWKEYPublicKey *key, unsigned char *sign,
			    unsigned int signLength, unsigned char *hash,
			    unsigned int hashLength);
extern
SECStatus RSA_HashCheckSign(SECOidTag hashOid,
			    NSSLOWKEYPublicKey *key, unsigned char *sig,
			    unsigned int sigLen, unsigned char *digest,
			    unsigned int digestLen);
extern
SECStatus RSA_CheckSignRecover(NSSLOWKEYPublicKey *key, unsigned char *data,
    			    unsigned int *data_len,unsigned int max_output_len, 
			    unsigned char *sign, unsigned int sign_len);
extern
SECStatus RSA_EncryptBlock(NSSLOWKEYPublicKey *key, unsigned char *output,
			   unsigned int *outputLen, unsigned int maxOutputLen,
			   unsigned char *input, unsigned int inputLen);
extern
SECStatus RSA_DecryptBlock(NSSLOWKEYPrivateKey *key, unsigned char *output,
			   unsigned int *outputLen, unsigned int maxOutputLen,
			   unsigned char *input, unsigned int inputLen);

/*
 * added to make pkcs #11 happy
 *   RAW is RSA_X_509
 */
extern
SECStatus RSA_SignRaw( NSSLOWKEYPrivateKey *key, unsigned char *output,
			 unsigned int *output_len, unsigned int maxOutputLen,
			 unsigned char *input, unsigned int input_len);
extern
SECStatus RSA_CheckSignRaw( NSSLOWKEYPublicKey *key, unsigned char *sign, 
			    unsigned int sign_len, unsigned char *hash, 
			    unsigned int hash_len);
extern
SECStatus RSA_CheckSignRecoverRaw( NSSLOWKEYPublicKey *key, unsigned char *data,
			    unsigned int *data_len, unsigned int max_output_len,
			    unsigned char *sign, unsigned int sign_len);
extern
SECStatus RSA_EncryptRaw( NSSLOWKEYPublicKey *key, unsigned char *output,
			    unsigned int *output_len,
			    unsigned int max_output_len, 
			    unsigned char *input, unsigned int input_len);
extern
SECStatus RSA_DecryptRaw(NSSLOWKEYPrivateKey *key, unsigned char *output,
			     unsigned int *output_len,
    			     unsigned int max_output_len,
			     unsigned char *input, unsigned int input_len);
#ifdef NSS_ENABLE_ECC
/*
** pepare an ECParam structure from DEREncoded params
 */
extern SECStatus EC_FillParams(PRArenaPool *arena,
                               const SECItem *encodedParams, ECParams *params);
extern SECStatus EC_DecodeParams(const SECItem *encodedParams, 
				ECParams **ecparams);
extern SECStatus EC_CopyParams(PRArenaPool *arena, ECParams *dstParams,
              			const ECParams *srcParams);
#endif


/*
** Prepare a buffer for padded CBC encryption, growing to the appropriate 
** boundary, filling with the appropriate padding.
**
** blockSize must be a power of 2.
**
** We add from 1 to blockSize bytes -- we *always* grow.
** The extra bytes contain the value of the length of the padding:
** if we have 2 bytes of padding, then the padding is "0x02, 0x02".
**
** NOTE: If arena is non-NULL, we re-allocate from there, otherwise
** we assume (and use) PR memory (re)allocation.
*/
extern unsigned char * CBC_PadBuffer(PRArenaPool *arena, unsigned char *inbuf, 
                                     unsigned int inlen, unsigned int *outlen,
				     int blockSize);


/****************************************/
/*
** Power-Up selftests required for FIPS and invoked only
** under PKCS #11 FIPS mode.
*/
extern CK_RV sftk_fipsPowerUpSelfTest( void ); 

/*
** make known fixed PKCS #11 key types to their sizes in bytes
*/	
unsigned long sftk_MapKeySize(CK_KEY_TYPE keyType);

/*
** FIPS 140-2 auditing
*/
extern PRBool sftk_audit_enabled;

extern void sftk_LogAuditMessage(NSSAuditSeverity severity, const char *msg);

extern void sftk_AuditCreateObject(CK_SESSION_HANDLE hSession,
			CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulCount,
			CK_OBJECT_HANDLE_PTR phObject, CK_RV rv);

extern void sftk_AuditCopyObject(CK_SESSION_HANDLE hSession,
			CK_OBJECT_HANDLE hObject,
			CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulCount,
			CK_OBJECT_HANDLE_PTR phNewObject, CK_RV rv);

extern void sftk_AuditDestroyObject(CK_SESSION_HANDLE hSession,
			CK_OBJECT_HANDLE hObject, CK_RV rv);

extern void sftk_AuditGetObjectSize(CK_SESSION_HANDLE hSession,
			CK_OBJECT_HANDLE hObject, CK_ULONG_PTR pulSize,
			CK_RV rv);

extern void sftk_AuditGetAttributeValue(CK_SESSION_HANDLE hSession,
			CK_OBJECT_HANDLE hObject, CK_ATTRIBUTE_PTR pTemplate,
			CK_ULONG ulCount, CK_RV rv);

extern void sftk_AuditSetAttributeValue(CK_SESSION_HANDLE hSession,
			CK_OBJECT_HANDLE hObject, CK_ATTRIBUTE_PTR pTemplate,
			CK_ULONG ulCount, CK_RV rv);

extern void sftk_AuditCryptInit(const char *opName,
			CK_SESSION_HANDLE hSession,
			CK_MECHANISM_PTR pMechanism,
			CK_OBJECT_HANDLE hKey, CK_RV rv);

extern void sftk_AuditGenerateKey(CK_SESSION_HANDLE hSession,
			CK_MECHANISM_PTR pMechanism,
			CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulCount,
			CK_OBJECT_HANDLE_PTR phKey, CK_RV rv);

extern void sftk_AuditGenerateKeyPair(CK_SESSION_HANDLE hSession,
			CK_MECHANISM_PTR pMechanism,
			CK_ATTRIBUTE_PTR pPublicKeyTemplate,
			CK_ULONG ulPublicKeyAttributeCount,
			CK_ATTRIBUTE_PTR pPrivateKeyTemplate,
			CK_ULONG ulPrivateKeyAttributeCount,
			CK_OBJECT_HANDLE_PTR phPublicKey,
			CK_OBJECT_HANDLE_PTR phPrivateKey, CK_RV rv);

extern void sftk_AuditWrapKey(CK_SESSION_HANDLE hSession,
			CK_MECHANISM_PTR pMechanism,
			CK_OBJECT_HANDLE hWrappingKey, CK_OBJECT_HANDLE hKey,
			CK_BYTE_PTR pWrappedKey,
			CK_ULONG_PTR pulWrappedKeyLen, CK_RV rv);

extern void sftk_AuditUnwrapKey(CK_SESSION_HANDLE hSession,
			CK_MECHANISM_PTR pMechanism,
			CK_OBJECT_HANDLE hUnwrappingKey,
			CK_BYTE_PTR pWrappedKey, CK_ULONG ulWrappedKeyLen,
			CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulAttributeCount,
			CK_OBJECT_HANDLE_PTR phKey, CK_RV rv);

extern void sftk_AuditDeriveKey(CK_SESSION_HANDLE hSession,
			CK_MECHANISM_PTR pMechanism,
			CK_OBJECT_HANDLE hBaseKey,
			CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulAttributeCount,
			CK_OBJECT_HANDLE_PTR phKey, CK_RV rv);

extern void sftk_AuditDigestKey(CK_SESSION_HANDLE hSession,
			CK_OBJECT_HANDLE hKey, CK_RV rv);

/*
** FIPS 140-2 Error state
*/
extern PRBool sftk_fatalError;

/*
** macros to check for forked child after C_Initialize
*/
#if defined(XP_UNIX) && !defined(NO_PTHREADS)

extern PRBool forked;

extern void ForkedChild(void);

#define CHECK_FORK() \
    do { if (forked) return CKR_DEVICE_ERROR; } while (0)

#else

#define CHECK_FORK()

#endif

SEC_END_PROTOS

#endif /* _SOFTOKEN_H_ */
