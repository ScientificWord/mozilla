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

#include "plarena.h"

#include "seccomon.h"
#include "secitem.h"
#include "secport.h"
#include "hasht.h"
#include "pkcs11t.h"
#include "blapi.h"
#include "hasht.h"
#include "secasn1.h"
#include "secder.h"
#include "lowpbe.h"
#include "secoid.h"
#include "alghmac.h"
#include "softoken.h"
#include "secerr.h"

/* template for PKCS 5 PBE Parameter.  This template has been expanded
 * based upon the additions in PKCS 12.  This should eventually be moved
 * if RSA updates PKCS 5.
 */
static const SEC_ASN1Template NSSPKCS5PBEParameterTemplate[] =
{
    { SEC_ASN1_SEQUENCE, 
	0, NULL, sizeof(NSSPKCS5PBEParameter) },
    { SEC_ASN1_OCTET_STRING, 
	offsetof(NSSPKCS5PBEParameter, salt) },
    { SEC_ASN1_INTEGER,
	offsetof(NSSPKCS5PBEParameter, iteration) },
    { 0 }
};

static const SEC_ASN1Template NSSPKCS5PKCS12V2PBEParameterTemplate[] =
{   
    { SEC_ASN1_SEQUENCE, 0, NULL, sizeof(NSSPKCS5PBEParameter) },
    { SEC_ASN1_OCTET_STRING, offsetof(NSSPKCS5PBEParameter, salt) },
    { SEC_ASN1_INTEGER, offsetof(NSSPKCS5PBEParameter, iteration) },
    { 0 }
};

SECStatus
nsspkcs5_HashBuf(const SECHashObject *hashObj, unsigned char *dest,
					 unsigned char *src, int len)
{
    void *ctx;
    unsigned int retLen;

    ctx = hashObj->create();
    if(ctx == NULL) {
	return SECFailure;
    }
    hashObj->begin(ctx);
    hashObj->update(ctx, src, len);
    hashObj->end(ctx, dest, &retLen, hashObj->length);
    hashObj->destroy(ctx, PR_TRUE);
    return SECSuccess;
}

/* generate bits using any hash
 */
static SECItem *
nsspkcs5_PBKDF1(const SECHashObject *hashObj, SECItem *salt, SECItem *pwd, 
						int iter, PRBool faulty3DES) 
{
    SECItem *hash = NULL, *pre_hash = NULL;
    SECStatus rv = SECFailure;

    if((salt == NULL) || (pwd == NULL) || (iter < 0)) {
	return NULL;
    }
	
    hash = (SECItem *)PORT_ZAlloc(sizeof(SECItem));
    pre_hash = (SECItem *)PORT_ZAlloc(sizeof(SECItem));

    if((hash != NULL) && (pre_hash != NULL)) {
	int i, ph_len;

	ph_len = hashObj->length;
	if((salt->len + pwd->len) > hashObj->length) {
	    ph_len = salt->len + pwd->len;
	}

	rv = SECFailure;

	/* allocate buffers */
	hash->len = hashObj->length;
	hash->data = (unsigned char *)PORT_ZAlloc(hash->len);
	pre_hash->data = (unsigned char *)PORT_ZAlloc(ph_len);

	/* in pbeSHA1TripleDESCBC there was an allocation error that made
	 * it into the caller.  We do not want to propagate those errors
	 * further, so we are doing it correctly, but reading the old method.
	 */
	if (faulty3DES) {
	    pre_hash->len = ph_len;
	} else {
	    pre_hash->len = salt->len + pwd->len;
	}

	/* preform hash */
	if ((hash->data != NULL) && (pre_hash->data != NULL)) {
	    rv = SECSuccess;
	    /* check for 0 length password */
	    if(pwd->len > 0) {
		PORT_Memcpy(pre_hash->data, pwd->data, pwd->len);
	    }
	    if(salt->len > 0) {
		PORT_Memcpy((pre_hash->data+pwd->len), salt->data, salt->len);
	    }
	    for(i = 0; ((i < iter) && (rv == SECSuccess)); i++) {
		rv = nsspkcs5_HashBuf(hashObj, hash->data, 
					pre_hash->data, pre_hash->len);
		if(rv != SECFailure) {
		    pre_hash->len = hashObj->length;
		    PORT_Memcpy(pre_hash->data, hash->data, hashObj->length);
		}
	    }
	}
    }

    if(pre_hash != NULL) {
	SECITEM_FreeItem(pre_hash, PR_TRUE);
    }

    if((rv != SECSuccess) && (hash != NULL)) {
	SECITEM_FreeItem(hash, PR_TRUE);
	hash = NULL;
    }

    return hash;
}

/* this bit generation routine is described in PKCS 12 and the proposed
 * extensions to PKCS 5.  an initial hash is generated following the
 * instructions laid out in PKCS 5.  If the number of bits generated is
 * insufficient, then the method discussed in the proposed extensions to
 * PKCS 5 in PKCS 12 are used.  This extension makes use of the HMAC
 * function.  And the P_Hash function from the TLS standard.
 */
static SECItem *
nsspkcs5_PFXPBE(const SECHashObject *hashObj, NSSPKCS5PBEParameter *pbe_param,
				SECItem *init_hash, unsigned int bytes_needed)
{
    SECItem *ret_bits = NULL;
    int hash_size = 0;
    unsigned int i;
    unsigned int hash_iter;
    unsigned int dig_len;
    SECStatus rv = SECFailure;
    unsigned char *state = NULL;
    unsigned int state_len;
    HMACContext *cx = NULL;

    hash_size = hashObj->length;
    hash_iter = (bytes_needed + (unsigned int)hash_size - 1) / hash_size;

    /* allocate return buffer */
    ret_bits = (SECItem  *)PORT_ZAlloc(sizeof(SECItem));
    if(ret_bits == NULL)
	return NULL;
    ret_bits->data = (unsigned char *)PORT_ZAlloc((hash_iter * hash_size) + 1);
    ret_bits->len = (hash_iter * hash_size);
    if(ret_bits->data == NULL) {
	PORT_Free(ret_bits);
	return NULL;
    }

    /* allocate intermediate hash buffer.  8 is for the 8 bytes of
     * data which are added based on iteration number 
     */

    if ((unsigned int)hash_size > pbe_param->salt.len) {
	state_len = hash_size;
    } else {
	state_len = pbe_param->salt.len;
    }
    state = (unsigned char *)PORT_ZAlloc(state_len);
    if(state == NULL) {
	rv = SECFailure;
	goto loser;
    }
    if(pbe_param->salt.len > 0) {
	PORT_Memcpy(state, pbe_param->salt.data, pbe_param->salt.len);
    }

    cx = HMAC_Create(hashObj, init_hash->data, init_hash->len, PR_TRUE);
    if (cx == NULL) {
	rv = SECFailure;
	goto loser;
    }

    for(i = 0; i < hash_iter; i++) { 

	/* generate output bits */
	HMAC_Begin(cx);
	HMAC_Update(cx, state, state_len);
	HMAC_Update(cx, pbe_param->salt.data, pbe_param->salt.len);
	rv = HMAC_Finish(cx, ret_bits->data + (i * hash_size),
			 &dig_len, hash_size);
	if (rv != SECSuccess)
	    goto loser;
	PORT_Assert((unsigned int)hash_size == dig_len);

	/* generate new state */
	HMAC_Begin(cx);
	HMAC_Update(cx, state, state_len);
	rv = HMAC_Finish(cx, state, &state_len, state_len);
	if (rv != SECSuccess)
	    goto loser;
	PORT_Assert(state_len == dig_len);
    }

loser:
    if (state != NULL)
	PORT_ZFree(state, state_len);
    HMAC_Destroy(cx, PR_TRUE);

    if(rv != SECSuccess) {
	SECITEM_ZfreeItem(ret_bits, PR_TRUE);
	ret_bits = NULL;
    }

    return ret_bits;
}

/* generate bits for the key and iv determination.  if enough bits
 * are not generated using PKCS 5, then we need to generate more bits
 * based on the extension proposed in PKCS 12
 */
static SECItem *
nsspkcs5_PBKDF1Extended(const SECHashObject *hashObj,
	 NSSPKCS5PBEParameter *pbe_param, SECItem *pwitem, PRBool faulty3DES)
{
    SECItem * hash 		= NULL;
    SECItem * newHash 		= NULL;
    int       bytes_needed;
    int       bytes_available;
    
    bytes_needed = pbe_param->ivLen + pbe_param->keyLen;
    bytes_available = hashObj->length;
    
    hash = nsspkcs5_PBKDF1(hashObj, &pbe_param->salt, pwitem, 
						pbe_param->iter, faulty3DES);

    if(hash == NULL) {
	return NULL;
    }

    if(bytes_needed <= bytes_available) {
	return hash;
    } 

    newHash = nsspkcs5_PFXPBE(hashObj, pbe_param, hash, bytes_needed);
    if (hash != newHash)
	SECITEM_FreeItem(hash, PR_TRUE);
    return newHash;
}

#ifdef PBKDF2

/*
 * PBDKDF2 is PKCS #5 v2.0 it's currently not used by NSS
 */
/*
 * We This is safe because hLen for all our
 * HMAC algorithms are multiples of 4.
 */
static void
xorbytes(unsigned char *dest, unsigned char *src, int len)
{
#ifdef PARANOIA
    while (len--) {
    	*dest = *dest ^ *src;
	dest++;
	src++;
    }
#else
    PRUInt32 dest32 = (PRUInt32 *)dest;
    PRUInt32 src32 = (PRUInt32 *)dest;
    while (len -= sizeof(PRUInt32)) {
    	*dest32 = *dest32 ^ *src32;
	dest++;
	src++;
    }
#endif
}

static SECStatus
nsspkcs5_PBKFD2_F(const SECHashObject *hashobj, SECItem *pwitem, SECItem *salt,
			int iterations, unsigned int i, unsigned char *T)
{
    int j;
    HMACContext *cx = NULL;
    unsigned int hLen = hashObject->length
    SECStatus rv = SECFailure;
    unsigned char *last = NULL;
    int lastLength = salt->len + 4;

    cx=HMAC_Create(hashobj,pwitem->data,pwitem->len,PR_TRUE);
    if (cx == NULL) {
	goto loser;
    }
    PORT_Memset(T,0,hLen);
    realLastLength= MAX(lastLength,hLen);
    last = PORT_Alloc(realLastLength);
    if (last == NULL) {
	goto loser;
    }
    PORT_Memcpy(last,salt.data,salt.len);
    last[salt->len  ] = (i >> 24) & 0xff;
    last[salt->len+1] = (i >> 16) & 0xff;
    last[salt->len+2] = (i >> 8) & 0xff;
    last[salt->len+3] =    i  & 0xff;

    /* NOTE: we need at least one iteration to return success! */
    for (j=0; j < interations; j++) {
	rv =HMAC_Begin(cx);
	if (rv !=SECSuccess) {
	   break;
	}
	HMAC_Update(cx,last,lastLength);
	rv =HMAC_Finish(cx,last,&lastLength,hLen);
	if (rv !=SECSuccess) {
	   break;
	}
	do_xor(T,last,hLen);
    }
loser:
    if (cx) {
	HMAC_DestroyContext(cx, PR_TRUE);
    }
    if (last) {
	PORT_ZFree(last,reaLastLength);
    }
    return rv;
}

static SECItem *
nsspkcs5_PBKFD2(const SECHashObject *hashObj, NSSPKCS5PBEParameter *pbe_param, 
							SECItem *pwitem)
{
    unsigned int dkLen = bytesNeeded;
    unsigned int hLen = hashObject->length
    unsigned int l = (dkLen+hLen-1) / hLen;
    unsigned char *rp;
    SECItem *result;
    SECItem *salt = pbe_param->salt;
    int interations = pbe_param->iter;
    int bytesNeeded = pbe_param->keyLen;

    result = SECITEM_AllocItem(NULL,NULL,l*hLen);
    if (result == NULL) {
	return NULL;
    }

    T = PORT_Alloc(hLen);
    if (T == NULL) {
	goto loser;
    }

    for (i=0,rp=results->data; i < l ; i++, rp +=hLen) {
	rv = nsspkcs5_PBKFD2_F(hashobj,pwitem,salt,iterations,i,T);
	if (rv != SECSuccess) {
	    break;
	}
	PORT_Memcpy(rp,T,hLen);
    }

loser:
    if (T) {
	PORT_ZFree(T);
    }
    if (rv != SECSuccess) {
	SECITEM_FreeITEM(result,PR_TRUE);
	result = NULL;
    } else {
	result->len = dkLen;
    }
	
    return result;
}
#endif

#define HMAC_BUFFER 64
#define NSSPBE_ROUNDUP(x,y) ((((x)+((y)-1))/(y))*(y))
#define NSSPBE_MIN(x,y) ((x) < (y) ? (x) : (y))
/*
 * This is the extended PBE function defined by the final PKCS #12 spec.
 */
static SECItem *
nsspkcs5_PKCS12PBE(const SECHashObject *hashObject, 
		   NSSPKCS5PBEParameter *pbe_param, SECItem *pwitem, 
		   PBEBitGenID bitGenPurpose, unsigned int bytesNeeded)
{
    PRArenaPool *arena = NULL;
    unsigned int SLen,PLen;
    unsigned int hashLength = hashObject->length;
    unsigned char *S, *P;
    SECItem *A = NULL, B, D, I;
    SECItem *salt = &pbe_param->salt;
    unsigned int c,i = 0;
    unsigned int hashLen;
    int iter;
    unsigned char *iterBuf;
    void *hash = NULL;

    arena = PORT_NewArena(DER_DEFAULT_CHUNKSIZE);
    if(!arena) {
	return NULL;
    }

    /* how many hash object lengths are needed */
    c = (bytesNeeded + (hashLength-1))/hashLength;

    /* initialize our buffers */
    D.len = HMAC_BUFFER;
    /* B and D are the same length, use one alloc go get both */
    D.data = (unsigned char*)PORT_ArenaZAlloc(arena, D.len*2);
    B.len = D.len;
    B.data = D.data + D.len;

    /* if all goes well, A will be returned, so don't use our temp arena */
    A = SECITEM_AllocItem(NULL,NULL,c*hashLength);
    if (A == NULL) {
	goto loser;
    }
    
    SLen = NSSPBE_ROUNDUP(salt->len,HMAC_BUFFER);
    PLen = NSSPBE_ROUNDUP(pwitem->len,HMAC_BUFFER);
    I.len = SLen+PLen;
    I.data = (unsigned char*)PORT_ArenaZAlloc(arena, I.len);
    if (I.data == NULL) {
	goto loser;
    }

    /* S & P are only used to initialize I */
    S = I.data;
    P = S + SLen;

    PORT_Memset(D.data, (char)bitGenPurpose, D.len);
    if (SLen) {
	for (i=0; i < SLen; i += salt->len) {
	    PORT_Memcpy(S+i, salt->data, NSSPBE_MIN(SLen-i,salt->len));
	}
    } 
    if (PLen) {
	for (i=0; i < PLen; i += pwitem->len) {
	    PORT_Memcpy(P+i, pwitem->data, NSSPBE_MIN(PLen-i,pwitem->len));
	}
    } 

    iterBuf = (unsigned char*)PORT_ArenaZAlloc(arena,hashLength);
    if (iterBuf == NULL) {
	goto loser;
    }

    hash = hashObject->create();
    if(!hash) {
	goto loser;
    }
    /* calculate the PBE now */
    for(i = 0; i < c; i++) {
	int Bidx;	/* must be signed or the for loop won't terminate */
	unsigned int k, j;
	unsigned char *Ai = A->data+i*hashLength;


	for(iter = 0; iter < pbe_param->iter; iter++) {
	    hashObject->begin(hash);

	    if (iter) {
		hashObject->update(hash, iterBuf, hashLen);
	    } else {
		hashObject->update(hash, D.data, D.len);
		hashObject->update(hash, I.data, I.len); 
	    }

	    hashObject->end(hash, iterBuf, &hashLen, hashObject->length);
	    if(hashLen != hashObject->length) {
		break;
	    }
	}

	PORT_Memcpy(Ai, iterBuf, hashLength);
	for (Bidx = 0; Bidx < B.len; Bidx += hashLength) {
	    PORT_Memcpy(B.data+Bidx,iterBuf,NSSPBE_MIN(B.len-Bidx,hashLength));
	}

	k = I.len/B.len;
	for(j = 0; j < k; j++) {
	    unsigned int q, carryBit;
	    unsigned char *Ij = I.data + j*B.len;

	    /* (Ij = Ij+B+1) */
	    for (Bidx = (B.len-1), q=1, carryBit=0; Bidx >= 0; Bidx--,q=0) {
		q += (unsigned int)Ij[Bidx];
		q += (unsigned int)B.data[Bidx];
		q += carryBit;

		carryBit = (q > 0xff);
		Ij[Bidx] = (unsigned char)(q & 0xff);
	    }
	}
    }
loser:
    if (hash) {
    	hashObject->destroy(hash, PR_TRUE);
    }
    if(arena) {
	PORT_FreeArena(arena, PR_TRUE);
    }

    if (A) {
        /* if i != c, then we didn't complete the loop above and must of failed
         * somwhere along the way */
        if (i != c) {
	    SECITEM_ZfreeItem(A,PR_TRUE);
	    A = NULL;
        } else {
    	    A->len = bytesNeeded;
        }
    }
    
    return A;
}

/*
 * generate key as per PKCS 5
 */
SECItem *
nsspkcs5_ComputeKeyAndIV(NSSPKCS5PBEParameter *pbe_param, SECItem *pwitem,
		      SECItem *iv, PRBool faulty3DES)
{
    SECItem *hash = NULL, *key = NULL;
    const SECHashObject *hashObj;
    PRBool getIV = PR_FALSE;

    if((pbe_param == NULL) || (pwitem == NULL)) {
	return NULL;
    }

    key = SECITEM_AllocItem(NULL,NULL,pbe_param->keyLen);
    if (key == NULL) {
	return NULL;
    }

    if ((pbe_param->ivLen) && (iv->data == NULL)) {
	getIV = PR_TRUE;
    	iv->data = (unsigned char *)PORT_Alloc(pbe_param->ivLen);
    	if (iv->data == NULL) {
	    goto loser;
	}
	iv->len = pbe_param->ivLen;
    }

    hashObj = HASH_GetRawHashObject(pbe_param->hashType);
    switch (pbe_param->pbeType) {
    case NSSPKCS5_PBKDF1:
	hash = nsspkcs5_PBKDF1Extended(hashObj,pbe_param,pwitem,faulty3DES);
	if (hash == NULL) {
	    goto loser;
	}
	PORT_Assert(hash->len >= key->len+iv->len);
	if (getIV) {
	    PORT_Memcpy(iv->data, hash->data+(hash->len - iv->len),iv->len);
	}
    	break;
#ifdef PBKDF2
    case NSSPKCS5_PBKDF2:
	hash = nsspkcs5_PBKDF2(hashObj,pbe_param,pwitem);
	PORT_Assert(!getIV);
    	break;
#endif
    case NSSPKCS5_PKCS12_V2:
	if (getIV) {
	    hash = nsspkcs5_PKCS12PBE(hashObj,pbe_param,pwitem,
						pbeBitGenCipherIV,iv->len);
	    if (hash == NULL) {
		goto loser;
	    }
	    PORT_Memcpy(iv->data,hash->data,iv->len);
	    SECITEM_ZfreeItem(hash,PR_TRUE);
	    hash = NULL;
	}
	hash = nsspkcs5_PKCS12PBE(hashObj,pbe_param,pwitem,
						pbe_param->keyID,key->len);
    default:
	break;
    }

    if (hash == NULL) {
	goto loser;
    }

    if (pbe_param->is2KeyDES) {
	PORT_Memcpy(key->data, hash->data, (key->len * 2) / 3);
	PORT_Memcpy(&(key->data[(key->len  * 2) / 3]), key->data,
		    key->len / 3);
    } else {
	PORT_Memcpy(key->data, hash->data, key->len);
    }

    SECITEM_ZfreeItem(hash, PR_TRUE);
    return key;

loser:
    if (getIV && iv->data) {
	PORT_ZFree(iv->data,iv->len);
	iv->data = NULL;
    }

    SECITEM_ZfreeItem(key, PR_TRUE);
    return NULL;
}

static SECStatus
nsspkcs5_FillInParam(SECOidTag algorithm, NSSPKCS5PBEParameter *pbe_param)
{
    PRBool skipType = PR_FALSE;

    pbe_param->keyLen = 5;
    pbe_param->ivLen = 8;
    pbe_param->hashType = HASH_AlgSHA1;
    pbe_param->pbeType = NSSPKCS5_PBKDF1;
    pbe_param->encAlg = SEC_OID_RC2_CBC;
    pbe_param->is2KeyDES = PR_FALSE;
    switch(algorithm) {
    /* DES3 Algorithms */
    case SEC_OID_PKCS12_V2_PBE_WITH_SHA1_AND_2KEY_TRIPLE_DES_CBC:
	pbe_param->is2KeyDES = PR_TRUE;
	/* fall through */
    case SEC_OID_PKCS12_V2_PBE_WITH_SHA1_AND_3KEY_TRIPLE_DES_CBC:
	pbe_param->pbeType = NSSPKCS5_PKCS12_V2;
	/* fall through */
    case SEC_OID_PKCS12_PBE_WITH_SHA1_AND_TRIPLE_DES_CBC:
	pbe_param->keyLen = 24;
	pbe_param->encAlg = SEC_OID_DES_EDE3_CBC;
	break;

    /* DES Algorithms */
    case SEC_OID_PKCS5_PBE_WITH_MD2_AND_DES_CBC:
    	pbe_param->hashType = HASH_AlgMD2;
	goto finish_des;
    case SEC_OID_PKCS5_PBE_WITH_MD5_AND_DES_CBC:
    	pbe_param->hashType = HASH_AlgMD5;
	/* fall through */
    case SEC_OID_PKCS5_PBE_WITH_SHA1_AND_DES_CBC:
finish_des:
	pbe_param->keyLen = 8;
	pbe_param->encAlg =  SEC_OID_DES_CBC;
	break;

    /* RC2 Algorithms */
    case SEC_OID_PKCS12_V2_PBE_WITH_SHA1_AND_128_BIT_RC2_CBC:
	pbe_param->keyLen = 16;
	/* fall through */
    case SEC_OID_PKCS12_V2_PBE_WITH_SHA1_AND_40_BIT_RC2_CBC:
	pbe_param->pbeType = NSSPKCS5_PKCS12_V2;
	break;
    case SEC_OID_PKCS12_PBE_WITH_SHA1_AND_128_BIT_RC2_CBC:
	pbe_param->keyLen = 16;
	/* fall through */
    case SEC_OID_PKCS12_PBE_WITH_SHA1_AND_40_BIT_RC2_CBC:
	break;

    /* RC4 algorithms */
    case SEC_OID_PKCS12_PBE_WITH_SHA1_AND_128_BIT_RC4:
	skipType = PR_TRUE;
	/* fall through */
    case SEC_OID_PKCS12_V2_PBE_WITH_SHA1_AND_128_BIT_RC4:
	pbe_param->keyLen = 16;
	/* fall through */
    case SEC_OID_PKCS12_V2_PBE_WITH_SHA1_AND_40_BIT_RC4:
	if (!skipType) {
    	    pbe_param->pbeType = NSSPKCS5_PKCS12_V2;
	}
	/* fall through */
    case SEC_OID_PKCS12_PBE_WITH_SHA1_AND_40_BIT_RC4:
        pbe_param->ivLen = 0;
        pbe_param->encAlg =  SEC_OID_RC4;
        break;
    default:
        return SECFailure;
    }

    return SECSuccess;
}

/* decode the algid and generate a PKCS 5 parameter from it
 */
NSSPKCS5PBEParameter *
nsspkcs5_NewParam(SECOidTag alg, SECItem *salt, int iterator)
{
    PRArenaPool *arena = NULL;
    NSSPKCS5PBEParameter *pbe_param = NULL;
    SECStatus rv = SECFailure;

    arena = PORT_NewArena(SEC_ASN1_DEFAULT_ARENA_SIZE);
    if (arena == NULL)
	return NULL;

    /* allocate memory for the parameter */
    pbe_param = (NSSPKCS5PBEParameter *)PORT_ArenaZAlloc(arena, 
	sizeof(NSSPKCS5PBEParameter));

    if (pbe_param == NULL) {
	goto loser;
    }

    pbe_param->poolp = arena;

    rv = nsspkcs5_FillInParam(alg, pbe_param);
    if (rv != SECSuccess) {
	goto loser;
    }

    pbe_param->iter = iterator;
    if (salt) {
	rv = SECITEM_CopyItem(arena,&pbe_param->salt,salt);
    }

    /* default key gen */
    pbe_param->keyID = pbeBitGenCipherKey;

loser:
    if (rv != SECSuccess) {
	PORT_FreeArena(arena, PR_TRUE);
	pbe_param = NULL;
    }

    return pbe_param;
}

/* decode the algid and generate a PKCS 5 parameter from it
 */
NSSPKCS5PBEParameter *
nsspkcs5_AlgidToParam(SECAlgorithmID *algid)
{
    NSSPKCS5PBEParameter *pbe_param = NULL;
    SECOidTag algorithm;
    SECStatus rv = SECFailure;

    if (algid == NULL) {
	return NULL;
    }

    algorithm = SECOID_GetAlgorithmTag(algid);
    if (algorithm == SEC_OID_UNKNOWN) {
	goto loser;
    }

    pbe_param = nsspkcs5_NewParam(algorithm, NULL, 1);
    if (pbe_param == NULL) {
	goto loser;
    }

    /* decode parameter */
    rv = SECFailure;
    switch (pbe_param->pbeType) {
    case NSSPKCS5_PBKDF1:
	rv = SEC_ASN1DecodeItem(pbe_param->poolp, pbe_param, 
	    NSSPKCS5PBEParameterTemplate, &algid->parameters);
	break;
    case NSSPKCS5_PKCS12_V2:
	rv = SEC_ASN1DecodeItem(pbe_param->poolp, pbe_param, 
		NSSPKCS5PKCS12V2PBEParameterTemplate, &algid->parameters);
	break;
    case NSSPKCS5_PBKDF2:
	break;
    }

loser:
    if (rv == SECSuccess) {
    	pbe_param->iter = DER_GetInteger(&pbe_param->iteration);
    } else {
	nsspkcs5_DestroyPBEParameter(pbe_param);
	pbe_param = NULL;
    }

    return pbe_param;
}

/* destroy a pbe parameter.  it assumes that the parameter was 
 * generated using the appropriate create function and therefor
 * contains an arena pool.
 */
void 
nsspkcs5_DestroyPBEParameter(NSSPKCS5PBEParameter *pbe_param)
{
    if (pbe_param != NULL) {
	PORT_FreeArena(pbe_param->poolp, PR_TRUE);
    }
}


/* crypto routines */
/* perform DES encryption and decryption.  these routines are called
 * by nsspkcs5_CipherData.  In the case of an error, NULL is returned.
 */
static SECItem *
sec_pkcs5_des(SECItem *key, SECItem *iv, SECItem *src, PRBool triple_des, 
								PRBool encrypt)
{
    SECItem *dest;
    SECItem *dup_src;
    SECStatus rv = SECFailure;
    int pad;

    if((src == NULL) || (key == NULL) || (iv == NULL))
	return NULL;

    dup_src = SECITEM_DupItem(src);
    if(dup_src == NULL) {
	return NULL;
    }

    if(encrypt != PR_FALSE) {
	void *dummy;

	dummy = DES_PadBuffer(NULL, dup_src->data, 
	    dup_src->len, &dup_src->len);
	if(dummy == NULL) {
	    SECITEM_FreeItem(dup_src, PR_TRUE);
	    return NULL;
	}
	dup_src->data = (unsigned char*)dummy;
    }

    dest = (SECItem *)PORT_ZAlloc(sizeof(SECItem));
    if(dest != NULL) {
	/* allocate with over flow */
	dest->data = (unsigned char *)PORT_ZAlloc(dup_src->len + 64);
	if(dest->data != NULL) {
	    DESContext *ctxt;
	    ctxt = DES_CreateContext(key->data, iv->data, 
			(triple_des ? NSS_DES_EDE3_CBC : NSS_DES_CBC), 
			encrypt);
	
	    if(ctxt != NULL) {
		rv = (encrypt ? DES_Encrypt : DES_Decrypt)(
			ctxt, dest->data, &dest->len,
			dup_src->len + 64, dup_src->data, dup_src->len);

		/* remove padding -- assumes 64 bit blocks */
		if((encrypt == PR_FALSE) && (rv == SECSuccess)) {
		    pad = dest->data[dest->len-1];
		    if((pad > 0) && (pad <= 8)) {
			if(dest->data[dest->len-pad] != pad) {
			    rv = SECFailure;
			    PORT_SetError(SEC_ERROR_BAD_PASSWORD);
			} else {
			    dest->len -= pad;
			}
		    } else {
			rv = SECFailure;
			PORT_SetError(SEC_ERROR_BAD_PASSWORD);
		    }
		}
		DES_DestroyContext(ctxt, PR_TRUE);
	    }
	}
    }

    if(rv == SECFailure) {
	if(dest != NULL) {
	    SECITEM_FreeItem(dest, PR_TRUE);
	}
	dest = NULL;
    }

    if(dup_src != NULL) {
	SECITEM_FreeItem(dup_src, PR_TRUE);
    }

    return dest;
}

/* perform rc2 encryption/decryption if an error occurs, NULL is returned
 */
static SECItem *
sec_pkcs5_rc2(SECItem *key, SECItem *iv, SECItem *src, PRBool dummy, 
								PRBool encrypt)
{
    SECItem *dest;
    SECItem *dup_src;
    SECStatus rv = SECFailure;
    int pad;

    if((src == NULL) || (key == NULL) || (iv == NULL)) {
	return NULL;
    }

    dup_src = SECITEM_DupItem(src);
    if(dup_src == NULL) {
	return NULL;
    }

    if(encrypt != PR_FALSE) {
	void *dummy;

	dummy = DES_PadBuffer(NULL, dup_src->data, 
			      dup_src->len, &dup_src->len);
	if(dummy == NULL) {
	    SECITEM_FreeItem(dup_src, PR_TRUE);
	    return NULL;
	}
	dup_src->data = (unsigned char*)dummy;
    }

    dest = (SECItem *)PORT_ZAlloc(sizeof(SECItem));
    if(dest != NULL) {
	dest->data = (unsigned char *)PORT_ZAlloc(dup_src->len + 64);
	if(dest->data != NULL) {
	    RC2Context *ctxt;

	    ctxt = RC2_CreateContext(key->data, key->len, iv->data,
					 	NSS_RC2_CBC, key->len);

	    if(ctxt != NULL) {
		rv = (encrypt ? RC2_Encrypt: RC2_Decrypt)(
			ctxt, dest->data, &dest->len,
			dup_src->len + 64, dup_src->data, dup_src->len);

		/* assumes 8 byte blocks  -- remove padding */	
		if((rv == SECSuccess) && (encrypt != PR_TRUE)) {
		    pad = dest->data[dest->len-1];
		    if((pad > 0) && (pad <= 8)) {
			if(dest->data[dest->len-pad] != pad) {
			    PORT_SetError(SEC_ERROR_BAD_PASSWORD);
			    rv = SECFailure;
			} else {
			    dest->len -= pad;
			}
		    } else {
			PORT_SetError(SEC_ERROR_BAD_PASSWORD);
			rv = SECFailure;
		    }
		}

	    }
	}
    }

    if((rv != SECSuccess) && (dest != NULL)) {
	SECITEM_FreeItem(dest, PR_TRUE);
	dest = NULL;
    }

    if(dup_src != NULL) {
	SECITEM_FreeItem(dup_src, PR_TRUE);
    }

    return dest;
}

/* perform rc4 encryption and decryption */
static SECItem *
sec_pkcs5_rc4(SECItem *key, SECItem *iv, SECItem *src, PRBool dummy_op,
								 PRBool encrypt)
{
    SECItem *dest;
    SECStatus rv = SECFailure;

    if((src == NULL) || (key == NULL) || (iv == NULL)) {
	return NULL;
    }

    dest = (SECItem *)PORT_ZAlloc(sizeof(SECItem));
    if(dest != NULL) {
	dest->data = (unsigned char *)PORT_ZAlloc(sizeof(unsigned char) *
	    (src->len + 64));
	if(dest->data != NULL) {
	    RC4Context *ctxt;

	    ctxt = RC4_CreateContext(key->data, key->len);
	    if(ctxt) { 
		rv = (encrypt ? RC4_Encrypt : RC4_Decrypt)(
				ctxt, dest->data, &dest->len,
				src->len + 64, src->data, src->len);
		RC4_DestroyContext(ctxt, PR_TRUE);
	    }
	}
    }

    if((rv != SECSuccess) && (dest)) {
	SECITEM_FreeItem(dest, PR_TRUE);
	dest = NULL;
    }

    return dest;
}
/* function pointer template for crypto functions */
typedef SECItem *(* pkcs5_crypto_func)(SECItem *key, SECItem *iv,
                                         SECItem *src, PRBool op1, PRBool op2);

/* performs the cipher operation on the src and returns the result.
 * if an error occurs, NULL is returned. 
 *
 * a null length password is allowed.  this corresponds to encrypting
 * the data with ust the salt.
 */
/* change this to use PKCS 11? */
SECItem *
nsspkcs5_CipherData(NSSPKCS5PBEParameter *pbe_param, SECItem *pwitem, 
		    SECItem *src, PRBool encrypt, PRBool *update)
{
    SECItem *key = NULL, iv;
    SECItem *dest = NULL;
    PRBool tripleDES = PR_TRUE;
    pkcs5_crypto_func cryptof;

    iv.data = NULL;

    if (update) { 
        *update = PR_FALSE;
    }

    if ((pwitem == NULL) || (src == NULL)) {
	return NULL;
    }

    /* get key, and iv */
    key = nsspkcs5_ComputeKeyAndIV(pbe_param, pwitem, &iv, PR_FALSE);
    if(key == NULL) {
	return NULL;
    }

    switch(pbe_param->encAlg) {
    case SEC_OID_DES_EDE3_CBC:
	cryptof = sec_pkcs5_des;
	tripleDES = PR_TRUE;
	break;
    case SEC_OID_DES_CBC:
	cryptof = sec_pkcs5_des;
	tripleDES = PR_FALSE;
	break;
    case SEC_OID_RC2_CBC:
	cryptof = sec_pkcs5_rc2;
	break;
    case SEC_OID_RC4:
	cryptof = sec_pkcs5_rc4;
	break;
    default:
	cryptof = NULL;
	break;
    }

    if (cryptof == NULL) {
	goto loser;
    }

    dest = (*cryptof)(key, &iv, src, tripleDES, encrypt);
    /* 
     * it's possible for some keys and keydb's to claim to
     * be triple des when they're really des. In this case
     * we simply try des. If des works we set the update flag
     * so the key db knows it needs to update all it's entries.
     *  The case can only happen on decrypted of a 
     *  SEC_OID_DES_EDE3_CBD.
     */
    if ((dest == NULL) && (encrypt == PR_FALSE) && 
				(pbe_param->encAlg == SEC_OID_DES_EDE3_CBC)) {
	dest = (*cryptof)(key, &iv, src, PR_FALSE, encrypt);
	if (update && (dest != NULL)) *update = PR_TRUE;
    }

loser:
    if (key != NULL) {
	SECITEM_ZfreeItem(key, PR_TRUE);
    }
    if (iv.data != NULL) {
	SECITEM_ZfreeItem(&iv, PR_FALSE);
    }

    return dest;
}

/* creates a algorithm ID containing the PBE algorithm and appropriate
 * parameters.  the required parameter is the algorithm.  if salt is
 * not specified, it is generated randomly.  if IV is specified, it overrides
 * the PKCS 5 generation of the IV.  
 *
 * the returned SECAlgorithmID should be destroyed using 
 * SECOID_DestroyAlgorithmID
 */
SECAlgorithmID *
nsspkcs5_CreateAlgorithmID(PRArenaPool *arena, SECOidTag algorithm, 
					NSSPKCS5PBEParameter *pbe_param)
{
    SECAlgorithmID *algid, *ret_algid = NULL;
    SECItem der_param;
    SECStatus rv = SECFailure;
    void *dummy = NULL;

    if (arena == NULL) {
	return NULL;
    }

    der_param.data = NULL;
    der_param.len = 0;

    /* generate the algorithm id */
    algid = (SECAlgorithmID *)PORT_ArenaZAlloc(arena, sizeof(SECAlgorithmID));
    if (algid == NULL) {
	goto loser;
    }

    if (pbe_param->iteration.data == NULL) {
	dummy = SEC_ASN1EncodeInteger(pbe_param->poolp,&pbe_param->iteration,
								pbe_param->iter);
	if (dummy == NULL) {
	    goto loser;
	}
    }
    switch (pbe_param->pbeType) {
    case NSSPKCS5_PBKDF1:
	dummy = SEC_ASN1EncodeItem(arena, &der_param, pbe_param,
					NSSPKCS5PBEParameterTemplate);
	break;
    case NSSPKCS5_PKCS12_V2:
	dummy = SEC_ASN1EncodeItem(arena, &der_param, pbe_param,
	    				NSSPKCS5PKCS12V2PBEParameterTemplate);
	break;
    default:
	break;
    }

    if (dummy == NULL) {
	goto loser;
    }
	
    rv = SECOID_SetAlgorithmID(arena, algid, algorithm, &der_param);
    if (rv != SECSuccess) {
	goto loser;
    }

    ret_algid = (SECAlgorithmID *)PORT_ZAlloc(sizeof(SECAlgorithmID));
    if (ret_algid == NULL) {
	goto loser;
    }

    rv = SECOID_CopyAlgorithmID(NULL, ret_algid, algid);
    if (rv != SECSuccess) {
	SECOID_DestroyAlgorithmID(ret_algid, PR_TRUE);
	ret_algid = NULL;
    }

loser:	

    return ret_algid;
}
