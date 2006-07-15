/*
 * ldvector.c - platform dependent DSO containing freebl implementation.
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

#include "loader.h"
#include "alghmac.h"

static const struct FREEBLVectorStr vector = 
{

    sizeof vector,
    FREEBL_VERSION,

    RSA_NewKey,
    RSA_PublicKeyOp,
    RSA_PrivateKeyOp,
    DSA_NewKey,
    DSA_SignDigest,
    DSA_VerifyDigest,
    DSA_NewKeyFromSeed,
    DSA_SignDigestWithSeed,
    DH_GenParam,
    DH_NewKey,
    DH_Derive,
    KEA_Derive,
    KEA_Verify,
    RC4_CreateContext,
    RC4_DestroyContext,
    RC4_Encrypt,
    RC4_Decrypt,
    RC2_CreateContext,
    RC2_DestroyContext,
    RC2_Encrypt,
    RC2_Decrypt,
    RC5_CreateContext,
    RC5_DestroyContext,
    RC5_Encrypt,
    RC5_Decrypt,
    DES_CreateContext,
    DES_DestroyContext,
    DES_Encrypt,
    DES_Decrypt,
    AES_CreateContext,
    AES_DestroyContext,
    AES_Encrypt,
    AES_Decrypt,
    MD5_Hash,
    MD5_HashBuf,
    MD5_NewContext,
    MD5_DestroyContext,
    MD5_Begin,
    MD5_Update,
    MD5_End,
    MD5_FlattenSize,
    MD5_Flatten,
    MD5_Resurrect,
    MD5_TraceState,
    MD2_Hash,
    MD2_NewContext,
    MD2_DestroyContext,
    MD2_Begin,
    MD2_Update,
    MD2_End,
    MD2_FlattenSize,
    MD2_Flatten,
    MD2_Resurrect,
    SHA1_Hash,
    SHA1_HashBuf,
    SHA1_NewContext,
    SHA1_DestroyContext,
    SHA1_Begin,
    SHA1_Update,
    SHA1_End,
    SHA1_TraceState,
    SHA1_FlattenSize,
    SHA1_Flatten,
    SHA1_Resurrect,
    RNG_RNGInit,
    RNG_RandomUpdate,
    RNG_GenerateGlobalRandomBytes,
    RNG_RNGShutdown,
    PQG_ParamGen,
    PQG_ParamGenSeedLen,
    PQG_VerifyParams,

    /* End of Version 3.001. */

    RSA_PrivateKeyOpDoubleChecked,
    RSA_PrivateKeyCheck,
    BL_Cleanup,

    /* End of Version 3.002. */

    SHA256_NewContext,
    SHA256_DestroyContext,
    SHA256_Begin,
    SHA256_Update,
    SHA256_End,
    SHA256_HashBuf,
    SHA256_Hash,
    SHA256_TraceState,
    SHA256_FlattenSize,
    SHA256_Flatten,
    SHA256_Resurrect,

    SHA512_NewContext,
    SHA512_DestroyContext,
    SHA512_Begin,
    SHA512_Update,
    SHA512_End,
    SHA512_HashBuf,
    SHA512_Hash,
    SHA512_TraceState,
    SHA512_FlattenSize,
    SHA512_Flatten,
    SHA512_Resurrect,

    SHA384_NewContext,
    SHA384_DestroyContext,
    SHA384_Begin,
    SHA384_Update,
    SHA384_End,
    SHA384_HashBuf,
    SHA384_Hash,
    SHA384_TraceState,
    SHA384_FlattenSize,
    SHA384_Flatten,
    SHA384_Resurrect,

    /* End of Version 3.003. */

    AESKeyWrap_CreateContext,
    AESKeyWrap_DestroyContext,
    AESKeyWrap_Encrypt,
    AESKeyWrap_Decrypt,

    /* End of Version 3.004. */

    BLAPI_SHVerify,
    BLAPI_VerifySelf,

    /* End of Version 3.005. */

    EC_NewKey,
    EC_NewKeyFromSeed,
    EC_ValidatePublicKey,
    ECDH_Derive,
    ECDSA_SignDigest,
    ECDSA_VerifyDigest,
    ECDSA_SignDigestWithSeed,

    /* End of Version 3.006. */
    /* End of Version 3.007. */

    AES_InitContext,
    AESKeyWrap_InitContext,
    DES_InitContext,
    RC2_InitContext,
    RC4_InitContext,

    AES_AllocateContext,
    AESKeyWrap_AllocateContext,
    DES_AllocateContext,
    RC2_AllocateContext,
    RC4_AllocateContext,

    MD2_Clone,
    MD5_Clone,
    SHA1_Clone,
    SHA256_Clone,
    SHA384_Clone,
    SHA512_Clone,

    TLS_PRF,
    HASH_GetRawHashObject,

    HMAC_Create,
    HMAC_Init,
    HMAC_Begin,
    HMAC_Update,
    HMAC_Clone,
    HMAC_Finish,
    HMAC_Destroy,

    RNG_SystemInfoForRNG,

    /* End of Version 3.008. */

    FIPS186Change_GenerateX,
    FIPS186Change_ReduceModQForDSA,

    /* End of Version 3.009. */
};

const FREEBLVector * 
FREEBL_GetVector(void)
{
    extern const char __nss_freebl_rcsid[];
    extern const char __nss_freebl_sccsid[];

    /* force a reference that won't get optimized away */
    volatile char c = __nss_freebl_rcsid[0] + __nss_freebl_sccsid[0]; 

    return &vector;
}

