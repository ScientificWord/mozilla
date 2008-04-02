/*
 * loader.h - load platform dependent DSO containing freebl implementation.
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
 * Portions created by the Initial Developer are Copyright (C) 2000
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Dr Vipul Gupta <vipul.gupta@sun.com>, Sun Microsystems Laboratories
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

#ifndef _LOADER_H_
#define _LOADER_H_ 1

#include "blapi.h"

#define FREEBL_VERSION 0x030A

struct FREEBLVectorStr {

  unsigned short length;  /* of this struct in bytes */
  unsigned short version; /* of this struct. */

  RSAPrivateKey * (* p_RSA_NewKey)(int         keySizeInBits,
				 SECItem *   publicExponent);

  SECStatus (* p_RSA_PublicKeyOp) (RSAPublicKey *   key,
				 unsigned char *  output,
				 const unsigned char *  input);

  SECStatus (* p_RSA_PrivateKeyOp)(RSAPrivateKey *  key,
				  unsigned char *  output,
				  const unsigned char *  input);

  SECStatus (* p_DSA_NewKey)(const PQGParams *    params, 
		            DSAPrivateKey **      privKey);

  SECStatus (* p_DSA_SignDigest)(DSAPrivateKey *   key,
				SECItem *         signature,
				const SECItem *   digest);

  SECStatus (* p_DSA_VerifyDigest)(DSAPublicKey *  key,
				  const SECItem *  signature,
				  const SECItem *  digest);

  SECStatus (* p_DSA_NewKeyFromSeed)(const PQGParams *params, 
				     const unsigned char * seed,
                                     DSAPrivateKey **privKey);

  SECStatus (* p_DSA_SignDigestWithSeed)(DSAPrivateKey * key,
				        SECItem *             signature,
				        const SECItem *       digest,
				        const unsigned char * seed);

 SECStatus (* p_DH_GenParam)(int primeLen, DHParams ** params);

 SECStatus (* p_DH_NewKey)(DHParams *           params, 
                           DHPrivateKey **	privKey);

 SECStatus (* p_DH_Derive)(SECItem *    publicValue, 
		           SECItem *    prime, 
			   SECItem *    privateValue, 
			   SECItem *    derivedSecret,
			   unsigned int maxOutBytes);

 SECStatus (* p_KEA_Derive)(SECItem *prime, 
                            SECItem *public1, 
                            SECItem *public2, 
			    SECItem *private1, 
			    SECItem *private2,
			    SECItem *derivedSecret);

 PRBool (* p_KEA_Verify)(SECItem *Y, SECItem *prime, SECItem *subPrime);

 RC4Context * (* p_RC4_CreateContext)(const unsigned char *key, int len);

 void (* p_RC4_DestroyContext)(RC4Context *cx, PRBool freeit);

 SECStatus (* p_RC4_Encrypt)(RC4Context *cx, unsigned char *output,
			    unsigned int *outputLen, unsigned int maxOutputLen,
			    const unsigned char *input, unsigned int inputLen);

 SECStatus (* p_RC4_Decrypt)(RC4Context *cx, unsigned char *output,
			    unsigned int *outputLen, unsigned int maxOutputLen,
			    const unsigned char *input, unsigned int inputLen);

 RC2Context * (* p_RC2_CreateContext)(const unsigned char *key, 
                     unsigned int len, const unsigned char *iv, 
		     int mode, unsigned effectiveKeyLen);

 void (* p_RC2_DestroyContext)(RC2Context *cx, PRBool freeit);

 SECStatus (* p_RC2_Encrypt)(RC2Context *cx, unsigned char *output,
			    unsigned int *outputLen, unsigned int maxOutputLen,
			    const unsigned char *input, unsigned int inputLen);

 SECStatus (* p_RC2_Decrypt)(RC2Context *cx, unsigned char *output,
			    unsigned int *outputLen, unsigned int maxOutputLen,
			    const unsigned char *input, unsigned int inputLen);

 RC5Context *(* p_RC5_CreateContext)(const SECItem *key, unsigned int rounds,
                     unsigned int wordSize, const unsigned char *iv, int mode);

 void (* p_RC5_DestroyContext)(RC5Context *cx, PRBool freeit);

 SECStatus (* p_RC5_Encrypt)(RC5Context *cx, unsigned char *output,
                            unsigned int *outputLen, unsigned int maxOutputLen,
                            const unsigned char *input, unsigned int inputLen);

 SECStatus (* p_RC5_Decrypt)(RC5Context *cx, unsigned char *output,
                            unsigned int *outputLen, unsigned int maxOutputLen,
                            const unsigned char *input, unsigned int inputLen);

 DESContext *(* p_DES_CreateContext)(const unsigned char *key, 
                                     const unsigned char *iv,
				     int mode, PRBool encrypt);

 void (* p_DES_DestroyContext)(DESContext *cx, PRBool freeit);

 SECStatus (* p_DES_Encrypt)(DESContext *cx, unsigned char *output,
			    unsigned int *outputLen, unsigned int maxOutputLen,
			    const unsigned char *input, unsigned int inputLen);

 SECStatus (* p_DES_Decrypt)(DESContext *cx, unsigned char *output,
			    unsigned int *outputLen, unsigned int maxOutputLen,
			    const unsigned char *input, unsigned int inputLen);

 AESContext * (* p_AES_CreateContext)(const unsigned char *key, 
                            const unsigned char *iv, 
			    int mode, int encrypt, unsigned int keylen, 
			    unsigned int blocklen);

 void (* p_AES_DestroyContext)(AESContext *cx, PRBool freeit);

 SECStatus (* p_AES_Encrypt)(AESContext *cx, unsigned char *output,
			    unsigned int *outputLen, unsigned int maxOutputLen,
			    const unsigned char *input, unsigned int inputLen);

 SECStatus (* p_AES_Decrypt)(AESContext *cx, unsigned char *output,
			    unsigned int *outputLen, unsigned int maxOutputLen,
			    const unsigned char *input, unsigned int inputLen);

 SECStatus (* p_MD5_Hash)(unsigned char *dest, const char *src);

 SECStatus (* p_MD5_HashBuf)(unsigned char *dest, const unsigned char *src,
			     uint32 src_length);

 MD5Context *(* p_MD5_NewContext)(void);

 void (* p_MD5_DestroyContext)(MD5Context *cx, PRBool freeit);

 void (* p_MD5_Begin)(MD5Context *cx);

 void (* p_MD5_Update)(MD5Context *cx,
		       const unsigned char *input, unsigned int inputLen);

 void (* p_MD5_End)(MD5Context *cx, unsigned char *digest,
		    unsigned int *digestLen, unsigned int maxDigestLen);

 unsigned int (* p_MD5_FlattenSize)(MD5Context *cx);

 SECStatus (* p_MD5_Flatten)(MD5Context *cx,unsigned char *space);

 MD5Context * (* p_MD5_Resurrect)(unsigned char *space, void *arg);

 void (* p_MD5_TraceState)(MD5Context *cx);

 SECStatus (* p_MD2_Hash)(unsigned char *dest, const char *src);

 MD2Context *(* p_MD2_NewContext)(void);

 void (* p_MD2_DestroyContext)(MD2Context *cx, PRBool freeit);

 void (* p_MD2_Begin)(MD2Context *cx);

 void (* p_MD2_Update)(MD2Context *cx,
		       const unsigned char *input, unsigned int inputLen);

 void (* p_MD2_End)(MD2Context *cx, unsigned char *digest,
		    unsigned int *digestLen, unsigned int maxDigestLen);

 unsigned int (* p_MD2_FlattenSize)(MD2Context *cx);

 SECStatus (* p_MD2_Flatten)(MD2Context *cx,unsigned char *space);

 MD2Context * (* p_MD2_Resurrect)(unsigned char *space, void *arg);

 SECStatus (* p_SHA1_Hash)(unsigned char *dest, const char *src);

 SECStatus (* p_SHA1_HashBuf)(unsigned char *dest, const unsigned char *src,
			      uint32 src_length);

 SHA1Context *(* p_SHA1_NewContext)(void);

 void (* p_SHA1_DestroyContext)(SHA1Context *cx, PRBool freeit);

 void (* p_SHA1_Begin)(SHA1Context *cx);

 void (* p_SHA1_Update)(SHA1Context *cx, const unsigned char *input,
			unsigned int inputLen);

 void (* p_SHA1_End)(SHA1Context *cx, unsigned char *digest,
		     unsigned int *digestLen, unsigned int maxDigestLen);

 void (* p_SHA1_TraceState)(SHA1Context *cx);

 unsigned int (* p_SHA1_FlattenSize)(SHA1Context *cx);

 SECStatus (* p_SHA1_Flatten)(SHA1Context *cx,unsigned char *space);

 SHA1Context * (* p_SHA1_Resurrect)(unsigned char *space, void *arg);

 SECStatus (* p_RNG_RNGInit)(void);

 SECStatus (* p_RNG_RandomUpdate)(const void *data, size_t bytes);

 SECStatus (* p_RNG_GenerateGlobalRandomBytes)(void *dest, size_t len);

 void  (* p_RNG_RNGShutdown)(void);

 SECStatus (* p_PQG_ParamGen)(unsigned int j, PQGParams **pParams,  
	                      PQGVerify **pVfy);    

 SECStatus (* p_PQG_ParamGenSeedLen)( unsigned int j, unsigned int seedBytes, 
                                     PQGParams **pParams, PQGVerify **pVfy); 

 SECStatus (* p_PQG_VerifyParams)(const PQGParams *params, 
                                  const PQGVerify *vfy, SECStatus *result);

  /* Version 3.001 came to here */

  SECStatus (* p_RSA_PrivateKeyOpDoubleChecked)(RSAPrivateKey *key,
                              unsigned char *output,
                              const unsigned char *input);

  SECStatus (* p_RSA_PrivateKeyCheck)(RSAPrivateKey *key);

  void (* p_BL_Cleanup)(void);

  /* Version 3.002 came to here */

 SHA256Context *(* p_SHA256_NewContext)(void);
 void (* p_SHA256_DestroyContext)(SHA256Context *cx, PRBool freeit);
 void (* p_SHA256_Begin)(SHA256Context *cx);
 void (* p_SHA256_Update)(SHA256Context *cx, const unsigned char *input,
			unsigned int inputLen);
 void (* p_SHA256_End)(SHA256Context *cx, unsigned char *digest,
		     unsigned int *digestLen, unsigned int maxDigestLen);
 SECStatus (* p_SHA256_HashBuf)(unsigned char *dest, const unsigned char *src,
			      uint32 src_length);
 SECStatus (* p_SHA256_Hash)(unsigned char *dest, const char *src);
 void (* p_SHA256_TraceState)(SHA256Context *cx);
 unsigned int (* p_SHA256_FlattenSize)(SHA256Context *cx);
 SECStatus (* p_SHA256_Flatten)(SHA256Context *cx,unsigned char *space);
 SHA256Context * (* p_SHA256_Resurrect)(unsigned char *space, void *arg);

 SHA512Context *(* p_SHA512_NewContext)(void);
 void (* p_SHA512_DestroyContext)(SHA512Context *cx, PRBool freeit);
 void (* p_SHA512_Begin)(SHA512Context *cx);
 void (* p_SHA512_Update)(SHA512Context *cx, const unsigned char *input,
			unsigned int inputLen);
 void (* p_SHA512_End)(SHA512Context *cx, unsigned char *digest,
		     unsigned int *digestLen, unsigned int maxDigestLen);
 SECStatus (* p_SHA512_HashBuf)(unsigned char *dest, const unsigned char *src,
			      uint32 src_length);
 SECStatus (* p_SHA512_Hash)(unsigned char *dest, const char *src);
 void (* p_SHA512_TraceState)(SHA512Context *cx);
 unsigned int (* p_SHA512_FlattenSize)(SHA512Context *cx);
 SECStatus (* p_SHA512_Flatten)(SHA512Context *cx,unsigned char *space);
 SHA512Context * (* p_SHA512_Resurrect)(unsigned char *space, void *arg);

 SHA384Context *(* p_SHA384_NewContext)(void);
 void (* p_SHA384_DestroyContext)(SHA384Context *cx, PRBool freeit);
 void (* p_SHA384_Begin)(SHA384Context *cx);
 void (* p_SHA384_Update)(SHA384Context *cx, const unsigned char *input,
			unsigned int inputLen);
 void (* p_SHA384_End)(SHA384Context *cx, unsigned char *digest,
		     unsigned int *digestLen, unsigned int maxDigestLen);
 SECStatus (* p_SHA384_HashBuf)(unsigned char *dest, const unsigned char *src,
			      uint32 src_length);
 SECStatus (* p_SHA384_Hash)(unsigned char *dest, const char *src);
 void (* p_SHA384_TraceState)(SHA384Context *cx);
 unsigned int (* p_SHA384_FlattenSize)(SHA384Context *cx);
 SECStatus (* p_SHA384_Flatten)(SHA384Context *cx,unsigned char *space);
 SHA384Context * (* p_SHA384_Resurrect)(unsigned char *space, void *arg);

  /* Version 3.003 came to here */

 AESKeyWrapContext * (* p_AESKeyWrap_CreateContext)(const unsigned char *key, 
                   const unsigned char *iv, int encrypt, unsigned int keylen);

 void (* p_AESKeyWrap_DestroyContext)(AESKeyWrapContext *cx, PRBool freeit);

 SECStatus (* p_AESKeyWrap_Encrypt)(AESKeyWrapContext *cx, 
            unsigned char *output,
            unsigned int *outputLen, unsigned int maxOutputLen,
            const unsigned char *input, unsigned int inputLen);

 SECStatus (* p_AESKeyWrap_Decrypt)(AESKeyWrapContext *cx, 
            unsigned char *output,
            unsigned int *outputLen, unsigned int maxOutputLen,
            const unsigned char *input, unsigned int inputLen);

  /* Version 3.004 came to here */

 PRBool (*p_BLAPI_SHVerify)(const char *name, PRFuncPtr addr);
 PRBool (*p_BLAPI_VerifySelf)(const char *name);

  /* Version 3.005 came to here */

 SECStatus (* p_EC_NewKey)(ECParams *           params, 
                           ECPrivateKey **	privKey);

 SECStatus (* p_EC_NewKeyFromSeed)(ECParams *   params, 
                             ECPrivateKey **	privKey,
                             const unsigned char * seed,
                             int                seedlen);

 SECStatus (* p_EC_ValidatePublicKey)(ECParams *   params, 
			     SECItem *	        publicValue);

 SECStatus (* p_ECDH_Derive)(SECItem *          publicValue, 
                             ECParams *         params,
                             SECItem *          privateValue,
                             PRBool             withCofactor,
                             SECItem *          derivedSecret);

 SECStatus (* p_ECDSA_SignDigest)(ECPrivateKey * key,
                             SECItem *          signature,
                             const SECItem *    digest);

 SECStatus (* p_ECDSA_VerifyDigest)(ECPublicKey * key,
                             const SECItem *    signature,
                             const SECItem *    digest);

 SECStatus (* p_ECDSA_SignDigestWithSeed)(ECPrivateKey * key,
                             SECItem *          signature,
                             const SECItem *    digest,
                             const unsigned char * seed,
                             const int          seedlen);

  /* Version 3.006 came to here */

  /* no modification to FREEBLVectorStr itself 
   * but ECParamStr was modified 
   */

  /* Version 3.007 came to here */

 SECStatus (* p_AES_InitContext)(AESContext *cx,
				 const unsigned char *key, 
				 unsigned int keylen, 
				 const unsigned char *iv, 
				 int mode, 
				 unsigned int encrypt,
				 unsigned int blocklen);
 SECStatus (* p_AESKeyWrap_InitContext)(AESKeyWrapContext *cx,
				 const unsigned char *key, 
				 unsigned int keylen, 
				 const unsigned char *iv, 
				 int mode, 
				 unsigned int encrypt,
				 unsigned int blocklen);
 SECStatus (* p_DES_InitContext)(DESContext *cx,
				 const unsigned char *key, 
				 unsigned int keylen,
				 const unsigned char *iv, 
				 int mode,
				 unsigned int encrypt,
				 unsigned int );
 SECStatus (* p_RC2_InitContext)(RC2Context *cx,
				 const unsigned char *key, 
				 unsigned int keylen,
				 const unsigned char *iv, 
				 int mode, 
				 unsigned int effectiveKeyLen,
				 unsigned int );
 SECStatus (* p_RC4_InitContext)(RC4Context *cx, 
				 const unsigned char *key, 
				 unsigned int keylen,
				 const unsigned char *, 
				 int, 
				 unsigned int ,
				 unsigned int );

 AESContext *(*p_AES_AllocateContext)(void);
 AESKeyWrapContext *(*p_AESKeyWrap_AllocateContext)(void);
 DESContext *(*p_DES_AllocateContext)(void);
 RC2Context *(*p_RC2_AllocateContext)(void);
 RC4Context *(*p_RC4_AllocateContext)(void);

 void (* p_MD2_Clone)(MD2Context *dest, MD2Context *src);
 void (* p_MD5_Clone)(MD5Context *dest, MD5Context *src);
 void (* p_SHA1_Clone)(SHA1Context *dest, SHA1Context *src);
 void (* p_SHA256_Clone)(SHA256Context *dest, SHA256Context *src);
 void (* p_SHA384_Clone)(SHA384Context *dest, SHA384Context *src);
 void (* p_SHA512_Clone)(SHA512Context *dest, SHA512Context *src);

 SECStatus (* p_TLS_PRF)(const SECItem *secret, const char *label, 
		         SECItem *seed, SECItem *result, PRBool isFIPS);

 const SECHashObject *(* p_HASH_GetRawHashObject)(HASH_HashType hashType);

 HMACContext * (* p_HMAC_Create)(const SECHashObject *hashObj, 
				 const unsigned char *secret, 
				 unsigned int secret_len, PRBool isFIPS);
 SECStatus (* p_HMAC_Init)(HMACContext *cx, const SECHashObject *hash_obj, 
			   const unsigned char *secret, 
			   unsigned int secret_len, PRBool isFIPS);
 void (* p_HMAC_Begin)(HMACContext *cx);
 void  (* p_HMAC_Update)(HMACContext *cx, const unsigned char *data, 
			 unsigned int data_len);
 HMACContext * (* p_HMAC_Clone)(HMACContext *cx);
 SECStatus (* p_HMAC_Finish)(HMACContext *cx, unsigned char *result, 
			     unsigned int *result_len, 
			     unsigned int max_result_len);
 void (* p_HMAC_Destroy)(HMACContext *cx, PRBool freeit);

 void (* p_RNG_SystemInfoForRNG)(void);

  /* Version 3.008 came to here */

 SECStatus (* p_FIPS186Change_GenerateX)(unsigned char *XKEY,
                                         const unsigned char *XSEEDj,
                                         unsigned char *x_j);
 SECStatus (* p_FIPS186Change_ReduceModQForDSA)(const unsigned char *w,
                                                const unsigned char *q,
                                                unsigned char *xj);

  /* Version 3.009 came to here */

 SECStatus (* p_Camellia_InitContext)(CamelliaContext *cx,
				 const unsigned char *key, 
				 unsigned int keylen, 
				 const unsigned char *iv, 
				 int mode, 
				 unsigned int encrypt,
				 unsigned int unused);

 CamelliaContext *(*p_Camellia_AllocateContext)(void);
 CamelliaContext * (* p_Camellia_CreateContext)(const unsigned char *key, 
						const unsigned char *iv, 
						int mode, int encrypt,
						unsigned int keylen);
 void (* p_Camellia_DestroyContext)(CamelliaContext *cx, PRBool freeit);

 SECStatus (* p_Camellia_Encrypt)(CamelliaContext *cx, unsigned char *output,
				  unsigned int *outputLen,
				  unsigned int maxOutputLen,
				  const unsigned char *input,
				  unsigned int inputLen);

 SECStatus (* p_Camellia_Decrypt)(CamelliaContext *cx, unsigned char *output,
				  unsigned int *outputLen,
				  unsigned int maxOutputLen,
				  const unsigned char *input,
				  unsigned int inputLen);

 void (* p_PQG_DestroyParams)(PQGParams *params);

 void (* p_PQG_DestroyVerify)(PQGVerify *vfy);

  /* Version 3.010 came to here */
};

typedef struct FREEBLVectorStr FREEBLVector;

SEC_BEGIN_PROTOS

typedef const FREEBLVector * FREEBLGetVectorFn(void);

extern FREEBLGetVectorFn FREEBL_GetVector;

SEC_END_PROTOS

#endif
