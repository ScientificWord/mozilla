/* -*- Mode: C; tab-width: 8 -*-*/
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


#ifndef _CRMFI_H_
#define _CRMFI_H_
/* This file will contain all declarations common to both 
 * encoding and decoding of CRMF Cert Requests.  This header 
 * file should only be included internally by CRMF implementation
 * files.
 */
#include "secasn1.h"
#include "crmfit.h"
#include "secerr.h"

#define CRMF_DEFAULT_ARENA_SIZE   1024
#define MAX_WRAPPED_KEY_LEN       2048


#define CRMF_BITS_TO_BYTES(bits) (((bits)+7)/8)
#define CRMF_BYTES_TO_BITS(bytes) ((bytes)*8)

struct crmfEncoderArg {
    SECItem *buffer;
    long     allocatedLen;
};

struct crmfEncoderOutput {
    CRMFEncoderOutputCallback fn;
    void *outputArg;
};

/*
 * This funciton is used by the API for encoding functions that are 
 * exposed through the API, ie all of the CMMF_Encode* and CRMF_Encode*
 * functions.
 */
extern void
       crmf_encoder_out(void *arg, const char *buf, unsigned long len,
                        int depth, SEC_ASN1EncodingPart data_kind);

/*
 * This function is used when we want to encode something locally within
 * the library, ie the CertRequest so that we can produce its signature.
 */
extern SECStatus 
       crmf_init_encoder_callback_arg (struct crmfEncoderArg *encoderArg,
				       SECItem               *derDest);

/*
 * This is the callback function we feed to the ASN1 encoder when doing
 * internal DER-encodings.  ie, encoding the cert request so we can 
 * produce a signature.
 */
extern void
crmf_generic_encoder_callback(void *arg, const char* buf, unsigned long len,
			      int depth, SEC_ASN1EncodingPart data_kind);

/* The ASN1 templates that need to be seen by internal files
 * in order to implement CRMF.
 */
extern const SEC_ASN1Template CRMFCertReqMsgTemplate[];
extern const SEC_ASN1Template CRMFRAVerifiedTemplate[];
extern const SEC_ASN1Template CRMFPOPOSigningKeyTemplate[];
extern const SEC_ASN1Template CRMFPOPOKeyEnciphermentTemplate[];
extern const SEC_ASN1Template CRMFPOPOKeyAgreementTemplate[];
extern const SEC_ASN1Template CRMFThisMessageTemplate[];
extern const SEC_ASN1Template CRMFSubsequentMessageTemplate[];
extern const SEC_ASN1Template CRMFDHMACTemplate[];
extern const SEC_ASN1Template CRMFEncryptedKeyWithEncryptedValueTemplate[];
extern const SEC_ASN1Template CRMFEncryptedValueTemplate[];

/*
 * Use these two values for encoding Boolean values.
 */
extern const unsigned char hexTrue;
extern const unsigned char hexFalse;
/*
 * Prototypes for helper routines used internally by multiple files.
 */
extern SECStatus crmf_encode_integer(PRArenaPool *poolp, SECItem *dest, 
				     long value);
extern SECStatus crmf_make_bitstring_copy(PRArenaPool *arena, SECItem *dest, 
					  SECItem *src);

extern SECStatus crmf_copy_pkiarchiveoptions(PRArenaPool           *poolp, 
					     CRMFPKIArchiveOptions *destOpt,
					     CRMFPKIArchiveOptions *srcOpt);
extern SECStatus  
       crmf_destroy_pkiarchiveoptions(CRMFPKIArchiveOptions *inArchOptions,
				      PRBool                 freeit);
extern const SEC_ASN1Template*
       crmf_get_pkiarchiveoptions_subtemplate(CRMFControl *inControl);

extern SECStatus crmf_copy_encryptedkey(PRArenaPool       *poolp,
					CRMFEncryptedKey  *srcEncrKey,
					CRMFEncryptedKey  *destEncrKey);
extern SECStatus
crmf_copy_encryptedvalue(PRArenaPool        *poolp,
			 CRMFEncryptedValue *srcValue,
			 CRMFEncryptedValue *destValue);

extern SECStatus
crmf_copy_encryptedvalue_secalg(PRArenaPool     *poolp,
				SECAlgorithmID  *srcAlgId,
				SECAlgorithmID **destAlgId);

extern SECStatus crmf_template_copy_secalg(PRArenaPool *poolp, 
					   SECAlgorithmID **dest,
					   SECAlgorithmID *src);

extern SECStatus crmf_copy_cert_name(PRArenaPool *poolp, CERTName **dest, 
				     CERTName *src);

extern SECStatus crmf_template_add_public_key(PRArenaPool               *poolp,
					      CERTSubjectPublicKeyInfo **dest,
					      CERTSubjectPublicKeyInfo  *pubKey);

extern CRMFCertExtension* crmf_create_cert_extension(PRArenaPool *poolp, 
						     SECOidTag    tag, 
						     PRBool       isCritical,
						     SECItem     *data);
extern CRMFCertRequest*
crmf_copy_cert_request(PRArenaPool *poolp, CRMFCertRequest *srcReq);

extern SECStatus crmf_destroy_encrypted_value(CRMFEncryptedValue *inEncrValue, 
					      PRBool freeit);

extern CRMFEncryptedValue *
crmf_create_encrypted_value_wrapped_privkey(SECKEYPrivateKey   *inPrivKey,
					    SECKEYPublicKey    *inPubKey,
					    CRMFEncryptedValue *destValue);

extern CK_MECHANISM_TYPE 
       crmf_get_mechanism_from_public_key(SECKEYPublicKey *inPubKey);

extern SECStatus
crmf_encrypted_value_unwrap_priv_key(PRArenaPool        *poolp,
				     CRMFEncryptedValue *encValue,
				     SECKEYPrivateKey   *privKey,
				     SECKEYPublicKey    *newPubKey,
				     SECItem            *nickname,
				     PK11SlotInfo       *slot,
				     unsigned char       keyUsage,
				     SECKEYPrivateKey  **unWrappedKey,
				     void               *wincx);

extern SECItem*
crmf_get_public_value(SECKEYPublicKey *pubKey, SECItem *dest);

extern CRMFCertExtension*
crmf_copy_cert_extension(PRArenaPool *poolp, CRMFCertExtension *inExtension);

extern SECStatus
crmf_create_prtime(SECItem *src, PRTime **dest);
#endif /*_CRMFI_H_*/
