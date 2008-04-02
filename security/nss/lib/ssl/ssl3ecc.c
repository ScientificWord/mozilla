/*
 * SSL3 Protocol
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
 *   Dr Vipul Gupta <vipul.gupta@sun.com> and
 *   Douglas Stebila <douglas@stebila.ca>, Sun Microsystems Laboratories
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

/* ECC code moved here from ssl3con.c */
/* $Id$ */

#include "nssrenam.h"
#include "nss.h"
#include "cert.h"
#include "ssl.h"
#include "cryptohi.h"	/* for DSAU_ stuff */
#include "keyhi.h"
#include "secder.h"
#include "secitem.h"

#include "sslimpl.h"
#include "sslproto.h"
#include "sslerr.h"
#include "prtime.h"
#include "prinrval.h"
#include "prerror.h"
#include "pratom.h"
#include "prthread.h"
#include "prinit.h"

#include "pk11func.h"
#include "secmod.h"
#include "ec.h"
#include "blapi.h"

#include <stdio.h>

#ifdef NSS_ENABLE_ECC

#ifndef PK11_SETATTRS
#define PK11_SETATTRS(x,id,v,l) (x)->type = (id); \
		(x)->pValue=(v); (x)->ulValueLen = (l);
#endif

#define SSL_GET_SERVER_PUBLIC_KEY(sock, type) \
    (ss->serverCerts[type].serverKeyPair ? \
    ss->serverCerts[type].serverKeyPair->pubKey : NULL)

#define SSL_IS_CURVE_NEGOTIATED(curvemsk, curveName) \
    ((curveName > ec_noName) && \
     (curveName < ec_pastLastName) && \
     ((1UL << curveName) & curvemsk) != 0)



static SECStatus ssl3_CreateECDHEphemeralKeys(sslSocket *ss, ECName ec_curve);

#define supportedCurve(x) (((x) > ec_noName) && ((x) < ec_pastLastName))

/* Table containing OID tags for elliptic curves named in the
 * ECC-TLS IETF draft.
 */
static const SECOidTag ecName2OIDTag[] = {
	0,  
	SEC_OID_SECG_EC_SECT163K1,  /*  1 */
	SEC_OID_SECG_EC_SECT163R1,  /*  2 */
	SEC_OID_SECG_EC_SECT163R2,  /*  3 */
	SEC_OID_SECG_EC_SECT193R1,  /*  4 */
	SEC_OID_SECG_EC_SECT193R2,  /*  5 */
	SEC_OID_SECG_EC_SECT233K1,  /*  6 */
	SEC_OID_SECG_EC_SECT233R1,  /*  7 */
	SEC_OID_SECG_EC_SECT239K1,  /*  8 */
	SEC_OID_SECG_EC_SECT283K1,  /*  9 */
	SEC_OID_SECG_EC_SECT283R1,  /* 10 */
	SEC_OID_SECG_EC_SECT409K1,  /* 11 */
	SEC_OID_SECG_EC_SECT409R1,  /* 12 */
	SEC_OID_SECG_EC_SECT571K1,  /* 13 */
	SEC_OID_SECG_EC_SECT571R1,  /* 14 */
	SEC_OID_SECG_EC_SECP160K1,  /* 15 */
	SEC_OID_SECG_EC_SECP160R1,  /* 16 */
	SEC_OID_SECG_EC_SECP160R2,  /* 17 */
	SEC_OID_SECG_EC_SECP192K1,  /* 18 */
	SEC_OID_SECG_EC_SECP192R1,  /* 19 */
	SEC_OID_SECG_EC_SECP224K1,  /* 20 */
	SEC_OID_SECG_EC_SECP224R1,  /* 21 */
	SEC_OID_SECG_EC_SECP256K1,  /* 22 */
	SEC_OID_SECG_EC_SECP256R1,  /* 23 */
	SEC_OID_SECG_EC_SECP384R1,  /* 24 */
	SEC_OID_SECG_EC_SECP521R1,  /* 25 */
};

static const PRUint16 curve2bits[] = {
	  0, /*  ec_noName     = 0,   */
	163, /*  ec_sect163k1  = 1,   */
	163, /*  ec_sect163r1  = 2,   */
	163, /*  ec_sect163r2  = 3,   */
	193, /*  ec_sect193r1  = 4,   */
	193, /*  ec_sect193r2  = 5,   */
	233, /*  ec_sect233k1  = 6,   */
	233, /*  ec_sect233r1  = 7,   */
	239, /*  ec_sect239k1  = 8,   */
	283, /*  ec_sect283k1  = 9,   */
	283, /*  ec_sect283r1  = 10,  */
	409, /*  ec_sect409k1  = 11,  */
	409, /*  ec_sect409r1  = 12,  */
	571, /*  ec_sect571k1  = 13,  */
	571, /*  ec_sect571r1  = 14,  */
	160, /*  ec_secp160k1  = 15,  */
	160, /*  ec_secp160r1  = 16,  */
	160, /*  ec_secp160r2  = 17,  */
	192, /*  ec_secp192k1  = 18,  */
	192, /*  ec_secp192r1  = 19,  */
	224, /*  ec_secp224k1  = 20,  */
	224, /*  ec_secp224r1  = 21,  */
	256, /*  ec_secp256k1  = 22,  */
	256, /*  ec_secp256r1  = 23,  */
	384, /*  ec_secp384r1  = 24,  */
	521, /*  ec_secp521r1  = 25,  */
      65535  /*  ec_pastLastName      */
};

typedef struct Bits2CurveStr {
    PRUint16    bits;
    ECName      curve;
} Bits2Curve;

static const Bits2Curve bits2curve [] = {
   {	192,     ec_secp192r1    /*  = 19,  fast */  },
   {	160,     ec_secp160r2    /*  = 17,  fast */  },
   {	160,     ec_secp160k1    /*  = 15,  */       },
   {	160,     ec_secp160r1    /*  = 16,  */       },
   {	163,     ec_sect163k1    /*  = 1,   */       },
   {	163,     ec_sect163r1    /*  = 2,   */       },
   {	163,     ec_sect163r2    /*  = 3,   */       },
   {	192,     ec_secp192k1    /*  = 18,  */       },
   {	193,     ec_sect193r1    /*  = 4,   */       },
   {	193,     ec_sect193r2    /*  = 5,   */       },
   {	224,     ec_secp224r1    /*  = 21,  fast */  },
   {	224,     ec_secp224k1    /*  = 20,  */       },
   {	233,     ec_sect233k1    /*  = 6,   */       },
   {	233,     ec_sect233r1    /*  = 7,   */       },
   {	239,     ec_sect239k1    /*  = 8,   */       },
   {	256,     ec_secp256r1    /*  = 23,  fast */  },
   {	256,     ec_secp256k1    /*  = 22,  */       },
   {	283,     ec_sect283k1    /*  = 9,   */       },
   {	283,     ec_sect283r1    /*  = 10,  */       },
   {	384,     ec_secp384r1    /*  = 24,  fast */  },
   {	409,     ec_sect409k1    /*  = 11,  */       },
   {	409,     ec_sect409r1    /*  = 12,  */       },
   {	521,     ec_secp521r1    /*  = 25,  fast */  },
   {	571,     ec_sect571k1    /*  = 13,  */       },
   {	571,     ec_sect571r1    /*  = 14,  */       },
   {  65535,     ec_noName    }
};

typedef struct ECDHEKeyPairStr {
    ssl3KeyPair *  pair;
    int            error;  /* error code of the call-once function */
    PRCallOnceType once;
} ECDHEKeyPair;

/* arrays of ECDHE KeyPairs */
static ECDHEKeyPair gECDHEKeyPairs[ec_pastLastName];

SECStatus 
ssl3_ECName2Params(PRArenaPool * arena, ECName curve, SECKEYECParams * params)
{
    SECOidData *oidData = NULL;

    if ((curve <= ec_noName) || (curve >= ec_pastLastName) ||
	((oidData = SECOID_FindOIDByTag(ecName2OIDTag[curve])) == NULL)) {
        PORT_SetError(SEC_ERROR_UNSUPPORTED_ELLIPTIC_CURVE);
	return SECFailure;
    }

    SECITEM_AllocItem(arena, params, (2 + oidData->oid.len));
    /* 
     * params->data needs to contain the ASN encoding of an object ID (OID)
     * representing the named curve. The actual OID is in 
     * oidData->oid.data so we simply prepend 0x06 and OID length
     */
    params->data[0] = SEC_ASN1_OBJECT_ID;
    params->data[1] = oidData->oid.len;
    memcpy(params->data + 2, oidData->oid.data, oidData->oid.len);

    return SECSuccess;
}

static ECName 
params2ecName(SECKEYECParams * params)
{
    SECItem oid = { siBuffer, NULL, 0};
    SECOidData *oidData = NULL;
    ECName i;

    /* 
     * params->data needs to contain the ASN encoding of an object ID (OID)
     * representing a named curve. Here, we strip away everything
     * before the actual OID and use the OID to look up a named curve.
     */
    if (params->data[0] != SEC_ASN1_OBJECT_ID) return ec_noName;
    oid.len = params->len - 2;
    oid.data = params->data + 2;
    if ((oidData = SECOID_FindOID(&oid)) == NULL) return ec_noName;
    for (i = ec_noName + 1; i < ec_pastLastName; i++) {
	if (ecName2OIDTag[i] == oidData->offset)
	    return i;
    }

    return ec_noName;
}

/* Caller must set hiLevel error code. */
static SECStatus
ssl3_ComputeECDHKeyHash(SECItem ec_params, SECItem server_ecpoint,
			     SSL3Random *client_rand, SSL3Random *server_rand,
			     SSL3Hashes *hashes, PRBool bypassPKCS11)
{
    PRUint8     * hashBuf;
    PRUint8     * pBuf;
    SECStatus     rv 		= SECSuccess;
    unsigned int  bufLen;
    /*
     * XXX For now, we only support named curves (the appropriate
     * checks are made before this method is called) so ec_params
     * takes up only two bytes. ECPoint needs to fit in 256 bytes
     * (because the spec says the length must fit in one byte)
     */
    PRUint8       buf[2*SSL3_RANDOM_LENGTH + 2 + 1 + 256];

    bufLen = 2*SSL3_RANDOM_LENGTH + ec_params.len + 1 + server_ecpoint.len;
    if (bufLen <= sizeof buf) {
    	hashBuf = buf;
    } else {
    	hashBuf = PORT_Alloc(bufLen);
	if (!hashBuf) {
	    return SECFailure;
	}
    }

    memcpy(hashBuf, client_rand, SSL3_RANDOM_LENGTH); 
    	pBuf = hashBuf + SSL3_RANDOM_LENGTH;
    memcpy(pBuf, server_rand, SSL3_RANDOM_LENGTH);
    	pBuf += SSL3_RANDOM_LENGTH;
    memcpy(pBuf, ec_params.data, ec_params.len);
    	pBuf += ec_params.len;
    pBuf[0] = (PRUint8)(server_ecpoint.len);
    pBuf += 1;
    memcpy(pBuf, server_ecpoint.data, server_ecpoint.len);
    	pBuf += server_ecpoint.len;
    PORT_Assert((unsigned int)(pBuf - hashBuf) == bufLen);

    rv = ssl3_ComputeCommonKeyHash(hashBuf, bufLen, hashes, bypassPKCS11);

    PRINT_BUF(95, (NULL, "ECDHkey hash: ", hashBuf, bufLen));
    PRINT_BUF(95, (NULL, "ECDHkey hash: MD5 result", hashes->md5, MD5_LENGTH));
    PRINT_BUF(95, (NULL, "ECDHkey hash: SHA1 result", hashes->sha, SHA1_LENGTH));

    if (hashBuf != buf && hashBuf != NULL)
    	PORT_Free(hashBuf);
    return rv;
}


/* Called from ssl3_SendClientKeyExchange(). */
SECStatus
ssl3_SendECDHClientKeyExchange(sslSocket * ss, SECKEYPublicKey * svrPubKey)
{
    PK11SymKey *	pms 		= NULL;
    SECStatus           rv    		= SECFailure;
    PRBool              isTLS;
    CK_MECHANISM_TYPE	target;
    SECKEYPublicKey	*pubKey = NULL;		/* Ephemeral ECDH key */
    SECKEYPrivateKey	*privKey = NULL;	/* Ephemeral ECDH key */

    PORT_Assert( ss->opt.noLocks || ssl_HaveSSL3HandshakeLock(ss) );
    PORT_Assert( ss->opt.noLocks || ssl_HaveXmitBufLock(ss));

    isTLS = (PRBool)(ss->ssl3.pwSpec->version > SSL_LIBRARY_VERSION_3_0);

    /* Generate ephemeral EC keypair */
    if (svrPubKey->keyType != ecKey) {
	PORT_SetError(SEC_ERROR_BAD_KEY);
	goto loser;
    }
    /* XXX SHOULD CALL ssl3_CreateECDHEphemeralKeys here, instead! */
    privKey = SECKEY_CreateECPrivateKey(&svrPubKey->u.ec.DEREncodedParams, 
	                                &pubKey, NULL);
    if (!privKey || !pubKey) {
	    ssl_MapLowLevelError(SEC_ERROR_KEYGEN_FAIL);
	    rv = SECFailure;
	    goto loser;
    }
    PRINT_BUF(50, (ss, "ECDH public value:",
					pubKey->u.ec.publicValue.data,
					pubKey->u.ec.publicValue.len));

    if (isTLS) target = CKM_TLS_MASTER_KEY_DERIVE_DH;
    else target = CKM_SSL3_MASTER_KEY_DERIVE_DH;

    /*  Determine the PMS */
    pms = PK11_PubDeriveWithKDF(privKey, svrPubKey, PR_FALSE, NULL, NULL,
			    CKM_ECDH1_DERIVE, target, CKA_DERIVE, 0,
			    CKD_NULL, NULL, NULL);

    if (pms == NULL) {
	SSL3AlertDescription desc  = illegal_parameter;
	(void)SSL3_SendAlert(ss, alert_fatal, desc);
	ssl_MapLowLevelError(SSL_ERROR_CLIENT_KEY_EXCHANGE_FAILURE);
	goto loser;
    }

    SECKEY_DestroyPrivateKey(privKey);
    privKey = NULL;

    rv = ssl3_InitPendingCipherSpec(ss,  pms);
    PK11_FreeSymKey(pms); pms = NULL;

    if (rv != SECSuccess) {
	ssl_MapLowLevelError(SSL_ERROR_CLIENT_KEY_EXCHANGE_FAILURE);
	goto loser;
    }

    rv = ssl3_AppendHandshakeHeader(ss, client_key_exchange, 
					pubKey->u.ec.publicValue.len + 1);
    if (rv != SECSuccess) {
        goto loser;	/* err set by ssl3_AppendHandshake* */
    }

    rv = ssl3_AppendHandshakeVariable(ss, 
					pubKey->u.ec.publicValue.data,
					pubKey->u.ec.publicValue.len, 1);
    SECKEY_DestroyPublicKey(pubKey);
    pubKey = NULL;

    if (rv != SECSuccess) {
        goto loser;	/* err set by ssl3_AppendHandshake* */
    }

    rv = SECSuccess;

loser:
    if(pms) PK11_FreeSymKey(pms);
    if(privKey) SECKEY_DestroyPrivateKey(privKey);
    if(pubKey) SECKEY_DestroyPublicKey(pubKey);
    return rv;
}


/*
** Called from ssl3_HandleClientKeyExchange()
*/
SECStatus
ssl3_HandleECDHClientKeyExchange(sslSocket *ss, SSL3Opaque *b,
				     PRUint32 length,
                                     SECKEYPublicKey *srvrPubKey,
                                     SECKEYPrivateKey *srvrPrivKey)
{
    PK11SymKey *      pms;
    SECStatus         rv;
    SECKEYPublicKey   clntPubKey;
    CK_MECHANISM_TYPE	target;
    PRBool isTLS;

    PORT_Assert( ss->opt.noLocks || ssl_HaveRecvBufLock(ss) );
    PORT_Assert( ss->opt.noLocks || ssl_HaveSSL3HandshakeLock(ss) );

    clntPubKey.keyType = ecKey;
    clntPubKey.u.ec.DEREncodedParams.len = 
	srvrPubKey->u.ec.DEREncodedParams.len;
    clntPubKey.u.ec.DEREncodedParams.data = 
	srvrPubKey->u.ec.DEREncodedParams.data;

    rv = ssl3_ConsumeHandshakeVariable(ss, &clntPubKey.u.ec.publicValue, 
	                               1, &b, &length);
    if (rv != SECSuccess) {
	SEND_ALERT
	return SECFailure;	/* XXX Who sets the error code?? */
    }

    isTLS = (PRBool)(ss->ssl3.prSpec->version > SSL_LIBRARY_VERSION_3_0);

    if (isTLS) target = CKM_TLS_MASTER_KEY_DERIVE_DH;
    else target = CKM_SSL3_MASTER_KEY_DERIVE_DH;

    /*  Determine the PMS */
    pms = PK11_PubDeriveWithKDF(srvrPrivKey, &clntPubKey, PR_FALSE, NULL, NULL,
			    CKM_ECDH1_DERIVE, target, CKA_DERIVE, 0,
			    CKD_NULL, NULL, NULL);

    if (pms == NULL) {
	/* last gasp.  */
	ssl_MapLowLevelError(SSL_ERROR_CLIENT_KEY_EXCHANGE_FAILURE);
	return SECFailure;
    }

    rv = ssl3_InitPendingCipherSpec(ss,  pms);
    PK11_FreeSymKey(pms);
    if (rv != SECSuccess) {
	SEND_ALERT
	return SECFailure; /* error code set by ssl3_InitPendingCipherSpec */
    }
    return SECSuccess;
}

ECName
ssl3_GetCurveWithECKeyStrength(PRUint32 curvemsk, int requiredECCbits)
{
    int    i;
    
    for ( i = 0; bits2curve[i].curve != ec_noName; i++) {
	if (bits2curve[i].bits < requiredECCbits)
	    continue;
    	if (SSL_IS_CURVE_NEGOTIATED(curvemsk, bits2curve[i].curve)) {
	    return bits2curve[i].curve;
	}
    }
    PORT_SetError(SSL_ERROR_NO_CYPHER_OVERLAP);
    return ec_noName;
}

/* find the "weakest link".  Get strength of signature key and of sym key.
 * choose curve for the weakest of those two.
 */
ECName
ssl3_GetCurveNameForServerSocket(sslSocket *ss)
{
    SECKEYPublicKey * svrPublicKey = NULL;
    ECName ec_curve = ec_noName;
    int    signatureKeyStrength = 521;
    int    requiredECCbits = ss->sec.secretKeyBits * 2;
    int    i;

    if (ss->ssl3.hs.kea_def->kea == kea_ecdhe_ecdsa) {
	svrPublicKey = SSL_GET_SERVER_PUBLIC_KEY(ss, kt_ecdh);
	if (svrPublicKey)
	    ec_curve = params2ecName(&svrPublicKey->u.ec.DEREncodedParams);
	if (!SSL_IS_CURVE_NEGOTIATED(ss->ssl3.hs.negotiatedECCurves, ec_curve)) {
	    PORT_SetError(SSL_ERROR_NO_CYPHER_OVERLAP);
	    return ec_noName;
	}
	signatureKeyStrength = curve2bits[ ec_curve ];
    } else {
        /* RSA is our signing cert */
        int serverKeyStrengthInBits;
 
        svrPublicKey = SSL_GET_SERVER_PUBLIC_KEY(ss, kt_rsa);
        if (!svrPublicKey) {
            PORT_SetError(SSL_ERROR_NO_CYPHER_OVERLAP);
            return ec_noName;
        }
 
        /* currently strength in bytes */
        serverKeyStrengthInBits = svrPublicKey->u.rsa.modulus.len;
        if (svrPublicKey->u.rsa.modulus.data[0] == 0) {
            serverKeyStrengthInBits--;
        }
        /* convert to strength in bits */
        serverKeyStrengthInBits *= BPB;
 
        signatureKeyStrength =
	    SSL_RSASTRENGTH_TO_ECSTRENGTH(serverKeyStrengthInBits);
    }
    if ( requiredECCbits > signatureKeyStrength ) 
         requiredECCbits = signatureKeyStrength;

    return ssl3_GetCurveWithECKeyStrength(ss->ssl3.hs.negotiatedECCurves,
					  requiredECCbits);
}

/* function to clear out the lists */
static SECStatus 
ssl3_ShutdownECDHECurves(void *appData, void *nssData)
{
    int i;
    ECDHEKeyPair *keyPair = &gECDHEKeyPairs[0];

    for (i=0; i < ec_pastLastName; i++, keyPair++) {
	if (keyPair->pair) {
	    ssl3_FreeKeyPair(keyPair->pair);
	}
    }
    memset(gECDHEKeyPairs, 0, sizeof gECDHEKeyPairs);
    return SECSuccess;
}

static PRStatus
ssl3_ECRegister(void)
{
    SECStatus rv;
    rv = NSS_RegisterShutdown(ssl3_ShutdownECDHECurves, gECDHEKeyPairs);
    if (rv != SECSuccess) {
	gECDHEKeyPairs[ec_noName].error = PORT_GetError();
    }
    return (PRStatus)rv;
}

/* CallOnce function, called once for each named curve. */
static PRStatus 
ssl3_CreateECDHEphemeralKeyPair(void * arg)
{
    SECKEYPrivateKey *    privKey  = NULL;
    SECKEYPublicKey *     pubKey   = NULL;
    ssl3KeyPair *	  keyPair  = NULL;
    ECName                ec_curve = (ECName)arg;
    SECKEYECParams        ecParams = { siBuffer, NULL, 0 };

    PORT_Assert(gECDHEKeyPairs[ec_curve].pair == NULL);

    /* ok, no one has generated a global key for this curve yet, do so */
    if (ssl3_ECName2Params(NULL, ec_curve, &ecParams) != SECSuccess) {
	gECDHEKeyPairs[ec_curve].error = PORT_GetError();
	return PR_FAILURE;
    }

    privKey = SECKEY_CreateECPrivateKey(&ecParams, &pubKey, NULL);    
    SECITEM_FreeItem(&ecParams, PR_FALSE);

    if (!privKey || !pubKey || !(keyPair = ssl3_NewKeyPair(privKey, pubKey))) {
	if (privKey) {
	    SECKEY_DestroyPrivateKey(privKey);
	}
	if (pubKey) {
	    SECKEY_DestroyPublicKey(pubKey);
	}
	ssl_MapLowLevelError(SEC_ERROR_KEYGEN_FAIL);
	gECDHEKeyPairs[ec_curve].error = PORT_GetError();
	return PR_FAILURE;
    }

    gECDHEKeyPairs[ec_curve].pair = keyPair;
    return PR_SUCCESS;
}

/*
 * Creates the ephemeral public and private ECDH keys used by
 * server in ECDHE_RSA and ECDHE_ECDSA handshakes.
 * For now, the elliptic curve is chosen to be the same
 * strength as the signing certificate (ECC or RSA).
 * We need an API to specify the curve. This won't be a real
 * issue until we further develop server-side support for ECC
 * cipher suites.
 */
static SECStatus
ssl3_CreateECDHEphemeralKeys(sslSocket *ss, ECName ec_curve)
{
    ssl3KeyPair *	  keyPair        = NULL;

    /* if there's no global key for this curve, make one. */
    if (gECDHEKeyPairs[ec_curve].pair == NULL) {
	PRStatus status;

	status = PR_CallOnce(&gECDHEKeyPairs[ec_noName].once, ssl3_ECRegister);
        if (status != PR_SUCCESS) {
	    PORT_SetError(gECDHEKeyPairs[ec_noName].error);
	    return SECFailure;
    	}
	status = PR_CallOnceWithArg(&gECDHEKeyPairs[ec_curve].once,
	                            ssl3_CreateECDHEphemeralKeyPair,
				    (void *)ec_curve);
        if (status != PR_SUCCESS) {
	    PORT_SetError(gECDHEKeyPairs[ec_curve].error);
	    return SECFailure;
    	}
    }

    keyPair = gECDHEKeyPairs[ec_curve].pair;
    PORT_Assert(keyPair != NULL);
    if (!keyPair) 
    	return SECFailure;
    ss->ephemeralECDHKeyPair = ssl3_GetKeyPairRef(keyPair);

    return SECSuccess;
}

SECStatus
ssl3_HandleECDHServerKeyExchange(sslSocket *ss, SSL3Opaque *b, PRUint32 length)
{
    PRArenaPool *    arena     = NULL;
    SECKEYPublicKey *peerKey   = NULL;
    PRBool           isTLS;
    SECStatus        rv;
    int              errCode   = SSL_ERROR_RX_MALFORMED_SERVER_KEY_EXCH;
    SSL3AlertDescription desc  = illegal_parameter;
    SSL3Hashes       hashes;
    SECItem          signature = {siBuffer, NULL, 0};

    SECItem          ec_params = {siBuffer, NULL, 0};
    SECItem          ec_point  = {siBuffer, NULL, 0};
    unsigned char    paramBuf[3]; /* only for curve_type == named_curve */

    isTLS = (PRBool)(ss->ssl3.prSpec->version > SSL_LIBRARY_VERSION_3_0);

    /* XXX This works only for named curves, revisit this when
     * we support generic curves.
     */
    ec_params.len  = sizeof paramBuf;
    ec_params.data = paramBuf;
    rv = ssl3_ConsumeHandshake(ss, ec_params.data, ec_params.len, &b, &length);
    if (rv != SECSuccess) {
	goto loser;		/* malformed. */
    }

    /* Fail if the curve is not a named curve */
    if ((ec_params.data[0] != ec_type_named) || 
	(ec_params.data[1] != 0) ||
	!supportedCurve(ec_params.data[2])) {
	    errCode = SEC_ERROR_UNSUPPORTED_ELLIPTIC_CURVE;
	    desc = handshake_failure;
	    goto alert_loser;
    }

    rv = ssl3_ConsumeHandshakeVariable(ss, &ec_point, 1, &b, &length);
    if (rv != SECSuccess) {
	goto loser;		/* malformed. */
    }
    /* Fail if the ec point uses compressed representation */
    if (ec_point.data[0] != EC_POINT_FORM_UNCOMPRESSED) {
	    errCode = SEC_ERROR_UNSUPPORTED_EC_POINT_FORM;
	    desc = handshake_failure;
	    goto alert_loser;
    }

    rv = ssl3_ConsumeHandshakeVariable(ss, &signature, 2, &b, &length);
    if (rv != SECSuccess) {
	goto loser;		/* malformed. */
    }

    if (length != 0) {
	if (isTLS)
	    desc = decode_error;
	goto alert_loser;		/* malformed. */
    }

    PRINT_BUF(60, (NULL, "Server EC params", ec_params.data, 
	ec_params.len));
    PRINT_BUF(60, (NULL, "Server EC point", ec_point.data, ec_point.len));

    /* failures after this point are not malformed handshakes. */
    /* TLS: send decrypt_error if signature failed. */
    desc = isTLS ? decrypt_error : handshake_failure;

    /*
     *  check to make sure the hash is signed by right guy
     */
    rv = ssl3_ComputeECDHKeyHash(ec_params, ec_point,
				      &ss->ssl3.hs.client_random,
				      &ss->ssl3.hs.server_random, 
				      &hashes, ss->opt.bypassPKCS11);

    if (rv != SECSuccess) {
	errCode =
	    ssl_MapLowLevelError(SSL_ERROR_SERVER_KEY_EXCHANGE_FAILURE);
	goto alert_loser;
    }
    rv = ssl3_VerifySignedHashes(&hashes, ss->sec.peerCert, &signature,
				isTLS, ss->pkcs11PinArg);
    if (rv != SECSuccess)  {
	errCode =
	    ssl_MapLowLevelError(SSL_ERROR_SERVER_KEY_EXCHANGE_FAILURE);
	goto alert_loser;
    }

    arena = PORT_NewArena(DER_DEFAULT_CHUNKSIZE);
    if (arena == NULL) {
	goto no_memory;
    }

    ss->sec.peerKey = peerKey = PORT_ArenaZNew(arena, SECKEYPublicKey);
    if (peerKey == NULL) {
	goto no_memory;
    }

    peerKey->arena                 = arena;
    peerKey->keyType               = ecKey;

    /* set up EC parameters in peerKey */
    if (ssl3_ECName2Params(arena, ec_params.data[2], 
	    &peerKey->u.ec.DEREncodedParams) != SECSuccess) {
	/* we should never get here since we already 
	 * checked that we are dealing with a supported curve
	 */
	errCode = SEC_ERROR_UNSUPPORTED_ELLIPTIC_CURVE;
	goto alert_loser;
    }

    /* copy publicValue in peerKey */
    if (SECITEM_CopyItem(arena, &peerKey->u.ec.publicValue,  &ec_point))
    {
	PORT_FreeArena(arena, PR_FALSE);
	goto no_memory;
    }
    peerKey->pkcs11Slot         = NULL;
    peerKey->pkcs11ID           = CK_INVALID_HANDLE;

    ss->sec.peerKey = peerKey;
    ss->ssl3.hs.ws = wait_cert_request;

    return SECSuccess;

alert_loser:
    (void)SSL3_SendAlert(ss, alert_fatal, desc);
loser:
    PORT_SetError( errCode );
    return SECFailure;

no_memory:	/* no-memory error has already been set. */
    ssl_MapLowLevelError(SSL_ERROR_SERVER_KEY_EXCHANGE_FAILURE);
    return SECFailure;
}

SECStatus
ssl3_SendECDHServerKeyExchange(sslSocket *ss)
{
const ssl3KEADef *     kea_def     = ss->ssl3.hs.kea_def;
    SECStatus          rv          = SECFailure;
    int                length;
    PRBool             isTLS;
    SECItem            signed_hash = {siBuffer, NULL, 0};
    SSL3Hashes         hashes;

    SECKEYPublicKey *  ecdhePub;
    SECItem            ec_params = {siBuffer, NULL, 0};
    unsigned char      paramBuf[3];
    ECName             curve;
    SSL3KEAType        certIndex;


    /* Generate ephemeral ECDH key pair and send the public key */
    curve = ssl3_GetCurveNameForServerSocket(ss);
    if (curve == ec_noName) {
    	goto loser;
    }
    rv = ssl3_CreateECDHEphemeralKeys(ss, curve);
    if (rv != SECSuccess) {
	goto loser; 	/* err set by AppendHandshake. */
    }	    
    ecdhePub = ss->ephemeralECDHKeyPair->pubKey;
    PORT_Assert(ecdhePub != NULL);
    if (!ecdhePub) {
	PORT_SetError(SSL_ERROR_SERVER_KEY_EXCHANGE_FAILURE);
	return SECFailure;
    }	
    
    ec_params.len  = sizeof paramBuf;
    ec_params.data = paramBuf;
    curve = params2ecName(&ecdhePub->u.ec.DEREncodedParams);
    if (curve != ec_noName) {
	ec_params.data[0] = ec_type_named;
	ec_params.data[1] = 0x00;
	ec_params.data[2] = curve;
    } else {
	PORT_SetError(SEC_ERROR_UNSUPPORTED_ELLIPTIC_CURVE);
	goto loser;
    }		

    rv = ssl3_ComputeECDHKeyHash(ec_params, ecdhePub->u.ec.publicValue,
				      &ss->ssl3.hs.client_random,
				      &ss->ssl3.hs.server_random,
				      &hashes, ss->opt.bypassPKCS11);
    if (rv != SECSuccess) {
	ssl_MapLowLevelError(SSL_ERROR_SERVER_KEY_EXCHANGE_FAILURE);
	goto loser;
    }

    isTLS = (PRBool)(ss->ssl3.pwSpec->version > SSL_LIBRARY_VERSION_3_0);

    /* XXX SSLKEAType isn't really a good choice for 
     * indexing certificates but that's all we have
     * for now.
     */
    if (kea_def->kea == kea_ecdhe_rsa)
	certIndex = kt_rsa;
    else /* kea_def->kea == kea_ecdhe_ecdsa */
	certIndex = kt_ecdh;

    rv = ssl3_SignHashes(&hashes, ss->serverCerts[certIndex].SERVERKEY, 
			 &signed_hash, isTLS);
    if (rv != SECSuccess) {
	goto loser;		/* ssl3_SignHashes has set err. */
    }
    if (signed_hash.data == NULL) {
	/* how can this happen and rv == SECSuccess ?? */
	PORT_SetError(SSL_ERROR_SERVER_KEY_EXCHANGE_FAILURE);
	goto loser;
    }

    length = ec_params.len + 
	     1 + ecdhePub->u.ec.publicValue.len + 
	     2 + signed_hash.len;

    rv = ssl3_AppendHandshakeHeader(ss, server_key_exchange, length);
    if (rv != SECSuccess) {
	goto loser; 	/* err set by AppendHandshake. */
    }

    rv = ssl3_AppendHandshake(ss, ec_params.data, ec_params.len);
    if (rv != SECSuccess) {
	goto loser; 	/* err set by AppendHandshake. */
    }

    rv = ssl3_AppendHandshakeVariable(ss, ecdhePub->u.ec.publicValue.data,
				      ecdhePub->u.ec.publicValue.len, 1);
    if (rv != SECSuccess) {
	goto loser; 	/* err set by AppendHandshake. */
    }

    rv = ssl3_AppendHandshakeVariable(ss, signed_hash.data,
				      signed_hash.len, 2);
    if (rv != SECSuccess) {
	goto loser; 	/* err set by AppendHandshake. */
    }

    PORT_Free(signed_hash.data);
    return SECSuccess;

loser:
    if (signed_hash.data != NULL) 
    	PORT_Free(signed_hash.data);
    return SECFailure;
}

/* Lists of ECC cipher suites for searching and disabling. */

static const ssl3CipherSuite ecdh_suites[] = {
    TLS_ECDH_ECDSA_WITH_3DES_EDE_CBC_SHA,
    TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA,
    TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA,
    TLS_ECDH_ECDSA_WITH_NULL_SHA,
    TLS_ECDH_ECDSA_WITH_RC4_128_SHA,
    TLS_ECDH_RSA_WITH_3DES_EDE_CBC_SHA,
    TLS_ECDH_RSA_WITH_AES_128_CBC_SHA,
    TLS_ECDH_RSA_WITH_AES_256_CBC_SHA,
    TLS_ECDH_RSA_WITH_NULL_SHA,
    TLS_ECDH_RSA_WITH_RC4_128_SHA,
    0 /* end of list marker */
};

static const ssl3CipherSuite ecdh_ecdsa_suites[] = {
    TLS_ECDH_ECDSA_WITH_3DES_EDE_CBC_SHA,
    TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA,
    TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA,
    TLS_ECDH_ECDSA_WITH_NULL_SHA,
    TLS_ECDH_ECDSA_WITH_RC4_128_SHA,
    0 /* end of list marker */
};

static const ssl3CipherSuite ecdh_rsa_suites[] = {
    TLS_ECDH_RSA_WITH_3DES_EDE_CBC_SHA,
    TLS_ECDH_RSA_WITH_AES_128_CBC_SHA,
    TLS_ECDH_RSA_WITH_AES_256_CBC_SHA,
    TLS_ECDH_RSA_WITH_NULL_SHA,
    TLS_ECDH_RSA_WITH_RC4_128_SHA,
    0 /* end of list marker */
};

static const ssl3CipherSuite ecdhe_ecdsa_suites[] = {
    TLS_ECDHE_ECDSA_WITH_3DES_EDE_CBC_SHA,
    TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA,
    TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA,
    TLS_ECDHE_ECDSA_WITH_NULL_SHA,
    TLS_ECDHE_ECDSA_WITH_RC4_128_SHA,
    0 /* end of list marker */
};

static const ssl3CipherSuite ecdhe_rsa_suites[] = {
    TLS_ECDHE_RSA_WITH_3DES_EDE_CBC_SHA,
    TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA,
    TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA,
    TLS_ECDHE_RSA_WITH_NULL_SHA,
    TLS_ECDHE_RSA_WITH_RC4_128_SHA,
    0 /* end of list marker */
};

/* List of all ECC cipher suites */
static const ssl3CipherSuite ecSuites[] = {
    TLS_ECDHE_ECDSA_WITH_3DES_EDE_CBC_SHA,
    TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA,
    TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA,
    TLS_ECDHE_ECDSA_WITH_NULL_SHA,
    TLS_ECDHE_ECDSA_WITH_RC4_128_SHA,
    TLS_ECDHE_RSA_WITH_3DES_EDE_CBC_SHA,
    TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA,
    TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA,
    TLS_ECDHE_RSA_WITH_NULL_SHA,
    TLS_ECDHE_RSA_WITH_RC4_128_SHA,
    TLS_ECDH_ECDSA_WITH_3DES_EDE_CBC_SHA,
    TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA,
    TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA,
    TLS_ECDH_ECDSA_WITH_NULL_SHA,
    TLS_ECDH_ECDSA_WITH_RC4_128_SHA,
    TLS_ECDH_RSA_WITH_3DES_EDE_CBC_SHA,
    TLS_ECDH_RSA_WITH_AES_128_CBC_SHA,
    TLS_ECDH_RSA_WITH_AES_256_CBC_SHA,
    TLS_ECDH_RSA_WITH_NULL_SHA,
    TLS_ECDH_RSA_WITH_RC4_128_SHA,
    0 /* end of list marker */
};

/* On this socket, Disable the ECC cipher suites in the argument's list */
SECStatus
ssl3_DisableECCSuites(sslSocket * ss, const ssl3CipherSuite * suite)
{
    if (!suite)
    	suite = ecSuites;
    for (; *suite; ++suite) {
	SECStatus rv      = ssl3_CipherPrefSet(ss, *suite, PR_FALSE);

	PORT_Assert(rv == SECSuccess); /* else is coding error */
    }
    return SECSuccess;
}

/* Look at the server certs configured on this socket, and disable any
 * ECC cipher suites that are not supported by those certs.
 */
void
ssl3_FilterECCipherSuitesByServerCerts(sslSocket * ss)
{
    CERTCertificate * svrCert;

    svrCert = ss->serverCerts[kt_rsa].serverCert;
    if (!svrCert) {
	ssl3_DisableECCSuites(ss, ecdhe_rsa_suites);
    }

    svrCert = ss->serverCerts[kt_ecdh].serverCert;
    if (!svrCert) {
	ssl3_DisableECCSuites(ss, ecdh_suites);
	ssl3_DisableECCSuites(ss, ecdhe_ecdsa_suites);
    } else {
	SECOidTag sigTag = SECOID_GetAlgorithmTag(&svrCert->signature);

	switch (sigTag) {
	case SEC_OID_PKCS1_RSA_ENCRYPTION:
	case SEC_OID_PKCS1_MD2_WITH_RSA_ENCRYPTION:
	case SEC_OID_PKCS1_MD4_WITH_RSA_ENCRYPTION:
	case SEC_OID_PKCS1_MD5_WITH_RSA_ENCRYPTION:
	case SEC_OID_PKCS1_SHA1_WITH_RSA_ENCRYPTION:
	case SEC_OID_PKCS1_SHA256_WITH_RSA_ENCRYPTION:
	case SEC_OID_PKCS1_SHA384_WITH_RSA_ENCRYPTION:
	case SEC_OID_PKCS1_SHA512_WITH_RSA_ENCRYPTION:
	    ssl3_DisableECCSuites(ss, ecdh_ecdsa_suites);
	    break;
	case SEC_OID_ANSIX962_ECDSA_SHA1_SIGNATURE:
	case SEC_OID_ANSIX962_ECDSA_SHA224_SIGNATURE:
	case SEC_OID_ANSIX962_ECDSA_SHA256_SIGNATURE:
	case SEC_OID_ANSIX962_ECDSA_SHA384_SIGNATURE:
	case SEC_OID_ANSIX962_ECDSA_SHA512_SIGNATURE:
	case SEC_OID_ANSIX962_ECDSA_SIGNATURE_RECOMMENDED_DIGEST:
	case SEC_OID_ANSIX962_ECDSA_SIGNATURE_SPECIFIED_DIGEST:
	    ssl3_DisableECCSuites(ss, ecdh_rsa_suites);
	    break;
	default:
	    ssl3_DisableECCSuites(ss, ecdh_suites);
	    break;
	}
    }
}

/* Ask: is ANY ECC cipher suite enabled on this socket? */
/* Order(N^2).  Yuk.  Also, this ignores export policy. */
PRBool
ssl3_IsECCEnabled(sslSocket * ss)
{
    const ssl3CipherSuite * suite;

    for (suite = ecSuites; *suite; ++suite) {
	PRBool    enabled = PR_FALSE;
	SECStatus rv      = ssl3_CipherPrefGet(ss, *suite, &enabled);

	PORT_Assert(rv == SECSuccess); /* else is coding error */
	if (rv == SECSuccess && enabled)
	    return PR_TRUE;
    }
    return PR_FALSE;
}

#define BE(n) 0, n

#ifndef NSS_ECC_MORE_THAN_SUITE_B
/* Prefabricated TLS client hello extension, Elliptic Curves List,
 * offers only 3 curves, the Suite B curves, 23-25 
 */
static const PRUint8 EClist[12] = {
    BE(10),         /* Extension type */
    BE( 8),         /* octets that follow ( 3 pairs + 1 length pair) */
    BE( 6),         /* octets that follow ( 3 pairs) */
    BE(23), BE(24), BE(25)
};
#else
/* Prefabricated TLS client hello extension, Elliptic Curves List,
 * offers curves 1-25.
 */
static const PRUint8 EClist[56] = {
    BE(10),         /* Extension type */
    BE(52),         /* octets that follow (25 pairs + 1 length pair) */
    BE(50),         /* octets that follow (25 pairs) */
            BE( 1), BE( 2), BE( 3), BE( 4), BE( 5), BE( 6), BE( 7), 
    BE( 8), BE( 9), BE(10), BE(11), BE(12), BE(13), BE(14), BE(15), 
    BE(16), BE(17), BE(18), BE(19), BE(20), BE(21), BE(22), BE(23), 
    BE(24), BE(25)
};
#endif

static const PRUint8 ECPtFmt[6] = {
    BE(11),         /* Extension type */
    BE( 2),         /* octets that follow */
             1,     /* octets that follow */
                 0  /* uncompressed type only */
};

/* Send our "canned" (precompiled) Supported Elliptic Curves extension,
 * which says that we support all TLS-defined named curves.
 */
PRInt32
ssl3_SendSupportedEllipticCurvesExtension(
			sslSocket * ss,
			PRBool      append,
			PRUint32    maxBytes)
{
    if (!ss || !ssl3_IsECCEnabled(ss))
    	return 0;
    if (append && maxBytes >= (sizeof EClist)) {
	SECStatus rv = ssl3_AppendHandshake(ss, EClist, (sizeof EClist));
    }
    return (sizeof EClist);
}

/* Send our "canned" (precompiled) Supported Point Formats extension,
 * which says that we only support uncompressed points.
 */
PRInt32
ssl3_SendSupportedPointFormatsExtension(
			sslSocket * ss,
			PRBool      append,
			PRUint32    maxBytes)
{
    if (!ss || !ssl3_IsECCEnabled(ss))
    	return 0;
    if (append && maxBytes >= (sizeof ECPtFmt)) {
	SECStatus rv = ssl3_AppendHandshake(ss, ECPtFmt, (sizeof ECPtFmt));
    }
    return (sizeof ECPtFmt);
}

/* Just make sure that the remote client supports uncompressed points,
 * Since that is all we support.  Disable ECC cipher suites if it doesn't.
 */
static SECStatus
ssl3_HandleSupportedPointFormatsExtension(sslSocket * ss, PRUint16 ex_type, 
                                          SECItem *data)
{
    int i;

    if (data->len < 2 || data->len > 255 || !data->data ||
        data->len != (unsigned int)data->data[0] + 1) {
    	/* malformed */
	goto loser;
    }
    for (i = data->len; --i > 0; ) {
    	if (data->data[i] == 0) {
	    /* indicate that we should send a reply */
	    SECStatus rv;
	    rv = ssl3_RegisterServerHelloExtensionSender(ss, ex_type,
			      &ssl3_SendSupportedPointFormatsExtension);
	    return rv;
	}
    }
loser:
    /* evil client doesn't support uncompressed */
    ssl3_DisableECCSuites(ss, ecSuites);
    return SECFailure;
}


#define SSL3_GET_SERVER_PUBLICKEY(sock, type) \
    (ss->serverCerts[type].serverKeyPair ? \
    ss->serverCerts[type].serverKeyPair->pubKey : NULL)

/* Extract the TLS curve name for the public key in our EC server cert. */
ECName ssl3_GetSvrCertCurveName(sslSocket *ss) 
{
    SECKEYPublicKey       *srvPublicKey; 
    ECName		  ec_curve       = ec_noName;

    srvPublicKey = SSL3_GET_SERVER_PUBLICKEY(ss, kt_ecdh);
    if (srvPublicKey) {
	ec_curve = params2ecName(&srvPublicKey->u.ec.DEREncodedParams);
    }
    return ec_curve;
}

/* Ensure that the curve in our server cert is one of the ones suppored
 * by the remote client, and disable all ECC cipher suites if not.
 */
static SECStatus
ssl3_HandleSupportedEllipticCurvesExtension(sslSocket * ss, PRUint16 ex_type, 
                                            SECItem *data)
{
    PRInt32  list_len;
    PRUint32 peerCurves   = 0;
    PRUint32 mutualCurves = 0;
    PRUint16 svrCertCurveName;

    if (!data->data || data->len < 4 || data->len > 65535)
    	goto loser;
    /* get the length of elliptic_curve_list */
    list_len = ssl3_ConsumeHandshakeNumber(ss, 2, &data->data, &data->len);
    if (list_len < 0 || data->len != list_len || (data->len % 2) != 0) {
    	/* malformed */
	goto loser;
    }
    /* build bit vector of peer's supported curve names */
    while (data->len) {
	PRInt32  curve_name = 
		 ssl3_ConsumeHandshakeNumber(ss, 2, &data->data, &data->len);
	if (curve_name > ec_noName && curve_name < ec_pastLastName) {
	    peerCurves |= (1U << curve_name);
	}
    }
    /* What curves do we support in common? */
    mutualCurves = ss->ssl3.hs.negotiatedECCurves &= peerCurves;
    if (!mutualCurves) { /* no mutually supported EC Curves */
    	goto loser;
    }

    /* if our ECC cert doesn't use one of these supported curves, 
     * disable ECC cipher suites that require an ECC cert. 
     */
    svrCertCurveName = ssl3_GetSvrCertCurveName(ss);
    if (svrCertCurveName != ec_noName &&
        (mutualCurves & (1U << svrCertCurveName)) != 0) {
	return SECSuccess;
    }
    /* Our EC cert doesn't contain a mutually supported curve.
     * Disable all ECC cipher suites that require an EC cert 
     */
    ssl3_DisableECCSuites(ss, ecdh_ecdsa_suites);
    ssl3_DisableECCSuites(ss, ecdhe_ecdsa_suites);
    return SECFailure;

loser:
    /* no common curve supported */
    ssl3_DisableECCSuites(ss, ecSuites);
    return SECFailure;
}

#endif /* NSS_ENABLE_ECC */

/* Format an SNI extension, using the name from the socket's URL,
 * unless that name is a dotted decimal string.
 */
PRInt32 
ssl3_SendServerNameIndicationExtension(
			sslSocket * ss,
			PRBool      append,
			PRUint32    maxBytes)
{
    PRUint32 len, span;
    /* must have a hostname */
    if (!ss || !ss->url || !ss->url[0])
    	return 0;
    /* must have at lest one character other than [0-9\.] */
    len  = PORT_Strlen(ss->url);
    span = strspn(ss->url, "0123456789.");
    if (len == span) {
    	/* is a dotted decimal IP address */
	return 0;
    }
    if (append && maxBytes >= len + 9) {
	SECStatus rv;
	/* extension_type */
	rv = ssl3_AppendHandshakeNumber(ss,       0, 2); 
	if (rv != SECSuccess) return 0;
	/* length of extension_data */
	rv = ssl3_AppendHandshakeNumber(ss, len + 5, 2); 
	if (rv != SECSuccess) return 0;
	/* length of server_name_list */
	rv = ssl3_AppendHandshakeNumber(ss, len + 3, 2);
	if (rv != SECSuccess) return 0;
	/* Name Type (host_name) */
	rv = ssl3_AppendHandshake(ss,       "\0",    1);
	if (rv != SECSuccess) return 0;
	/* HostName (length and value) */
	rv = ssl3_AppendHandshakeVariable(ss, ss->url, len, 2);
	if (rv != SECSuccess) return 0;
    }
    return len + 9;
}

/* handle an incoming SNI extension, by ignoring it. */
SECStatus
ssl3_HandleServerNameIndicationExtension(sslSocket * ss, PRUint16 ex_type, 
                                         SECItem *data)
{
    /* For now, we ignore this, as if we didn't understand it. :-)  */
    return SECSuccess;
}

/* Table of handlers for received TLS hello extensions, one per extension.
 * In the second generation, this table will be dynamic, and functions
 * will be registered here.
 */
static const ssl3HelloExtensionHandler handlers[] = {
    {  0, &ssl3_HandleServerNameIndicationExtension    },
#ifdef NSS_ENABLE_ECC
    { 10, &ssl3_HandleSupportedEllipticCurvesExtension },
    { 11, &ssl3_HandleSupportedPointFormatsExtension   },
#endif
    { -1, NULL }
};

/* Table of functions to format TLS hello extensions, one per extension.
 * This static table is for the formatting of client hello extensions.
 * The server's table of hello senders is dynamic, in the socket struct,
 * and sender functions are registered there.
 */
static const 
ssl3HelloExtensionSender clientHelloSenders[MAX_EXTENSION_SENDERS] = {
    {  0, &ssl3_SendServerNameIndicationExtension    },
#ifdef NSS_ENABLE_ECC
    { 10, &ssl3_SendSupportedEllipticCurvesExtension },
    { 11, &ssl3_SendSupportedPointFormatsExtension   },
#else
    { -1, NULL }
#endif
};

/* go through hello extensions in buffer "b".
 * For each one, find the extension handler in the table above, and 
 * if present, invoke that handler.  
 * ignore any extensions with unknown extension types.
 */
SECStatus 
ssl3_HandleClientHelloExtensions(sslSocket *ss, 
                                 SSL3Opaque **b, 
				 PRUint32 *length)
{
    while (*length) {
	const ssl3HelloExtensionHandler * handler;
	SECStatus rv;
	PRInt32   extension_type;
	SECItem   extension_data;

	/* Get the extension's type field */
	extension_type = ssl3_ConsumeHandshakeNumber(ss, 2, b, length);
	if (extension_type < 0)  /* failure to decode extension_type */
	    return SECFailure;   /* alert already sent */

	/* get the data for this extension, so we can pass it or skip it. */
	rv = ssl3_ConsumeHandshakeVariable(ss, &extension_data, 2, b, length);
	if (rv != SECSuccess)
	    return rv;

	/* find extension_type in table of Client Hello Extension Handlers */
	for (handler = handlers; handler->ex_type >= 0; handler++) {
	    if (handler->ex_type == extension_type)
	        break;
	}

	/* if found,  Call this handler */
	if (handler->ex_type == extension_type) {
	    rv = (*handler->ex_handler)(ss, (PRUint16)extension_type, 
	                                             &extension_data);
	    /* Ignore this result */
	    /* Essentially, treat all bad extensions as unrecognized types. */
	}
    }
    return SECSuccess;
}

/* Add a callback function to the table of senders of server hello extensions.
 */
SECStatus 
ssl3_RegisterServerHelloExtensionSender(sslSocket *ss, PRUint16 ex_type,
				        ssl3HelloExtensionSenderFunc cb)
{
    int i;
    ssl3HelloExtensionSender *sender = &ss->serverExtensionSenders[0];

    for (i = 0; i < MAX_EXTENSION_SENDERS; ++i, ++sender) {
        if (!sender->ex_sender) {
	    sender->ex_type   = ex_type;
	    sender->ex_sender = cb;
	    return SECSuccess;
	}
	/* detect duplicate senders */
	PORT_Assert(sender->ex_type != ex_type);
	if (sender->ex_type == ex_type) {
	    /* duplicate */
	    break;
	}
    }
    PORT_Assert(i < MAX_EXTENSION_SENDERS); /* table needs to grow */
    PORT_SetError(SEC_ERROR_LIBRARY_FAILURE);
    return SECFailure;
}

/* call each of the extension senders and return the accumulated length */
PRInt32
ssl3_CallHelloExtensionSenders(sslSocket *ss, PRBool append, PRUint32 maxBytes,
                               const ssl3HelloExtensionSender *sender)
{
    PRInt32 total_exten_len = 0;
    int i;

    if (!sender)
    	sender = &clientHelloSenders[0];

    for (i = 0; i < MAX_EXTENSION_SENDERS; ++i, ++sender) {
	if (sender->ex_sender) {
	    PRInt32 extLen = (*sender->ex_sender)(ss, append, maxBytes);
	    if (extLen < 0)
	    	return -1;
	    maxBytes        -= extLen;
	    total_exten_len += extLen;
	}
    }
    return total_exten_len;
}

