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
#include "nspr.h"
#include "secerr.h"
#include "secport.h"
#include "seccomon.h"
#include "secoid.h"
#include "sslerr.h"
#include "genname.h"
#include "keyhi.h"
#include "cert.h"
#include "certdb.h"
#include "cryptohi.h"

#ifndef NSS_3_4_CODE
#define NSS_3_4_CODE
#endif /* NSS_3_4_CODE */
#include "nsspki.h"
#include "pkitm.h"
#include "pkim.h"
#include "pki3hack.h"
#include "base.h"

#define PENDING_SLOP (24L*60L*60L)

/*
 * WARNING - this function is depricated, and will go away in the near future.
 *	It has been superseded by CERT_CheckCertValidTimes().
 *
 * Check the validity times of a certificate
 */
SECStatus
CERT_CertTimesValid(CERTCertificate *c)
{
    int64 now, notBefore, notAfter, pendingSlop;
    SECStatus rv;
    
    /* if cert is already marked OK, then don't bother to check */
    if ( c->timeOK ) {
	return(SECSuccess);
    }
    
    /* get current time */
    now = PR_Now();
    rv = CERT_GetCertTimes(c, &notBefore, &notAfter);
    
    if (rv) {
	return(SECFailure);
    }
    
    LL_I2L(pendingSlop, PENDING_SLOP);
    LL_SUB(notBefore, notBefore, pendingSlop);

    if (LL_CMP(now, <, notBefore) || LL_CMP(now, >, notAfter)) {
	PORT_SetError(SEC_ERROR_EXPIRED_CERTIFICATE);
	return(SECFailure);
    }

    return(SECSuccess);
}

/*
 * verify the signature of a signed data object with the given DER publickey
 */
SECStatus
CERT_VerifySignedDataWithPublicKey(CERTSignedData *sd, 
                                   SECKEYPublicKey *pubKey,
		                   void *wincx)
{
    SECStatus        rv;
    SECItem          sig;

    if ( !pubKey || !sd ) {
	PORT_SetError(PR_INVALID_ARGUMENT_ERROR);
	return SECFailure;
    }

    /* check the signature */
    sig = sd->signature;
    /* convert sig->len from bit counts to byte count. */
    DER_ConvertBitString(&sig);

    rv = VFY_VerifyDataWithAlgorithmID(sd->data.data, sd->data.len, pubKey, 
			&sig, &sd->signatureAlgorithm, NULL, wincx);

    return rv ? SECFailure : SECSuccess;
}

/*
 * verify the signature of a signed data object with the given DER publickey
 */
SECStatus
CERT_VerifySignedDataWithPublicKeyInfo(CERTSignedData *sd, 
                                       CERTSubjectPublicKeyInfo *pubKeyInfo,
		                       void *wincx)
{
    SECKEYPublicKey *pubKey;
    SECStatus        rv		= SECFailure;

    /* get cert's public key */
    pubKey = SECKEY_ExtractPublicKey(pubKeyInfo);
    if (pubKey) {
	rv =  CERT_VerifySignedDataWithPublicKey(sd, pubKey, wincx);
	SECKEY_DestroyPublicKey(pubKey);
    }
    return rv;
}

/*
 * verify the signature of a signed data object with the given certificate
 */
SECStatus
CERT_VerifySignedData(CERTSignedData *sd, CERTCertificate *cert,
		      int64 t, void *wincx)
{
    SECKEYPublicKey *pubKey = 0;
    SECStatus        rv     = SECFailure;
    SECCertTimeValidity validity;

    /* check the certificate's validity */
    validity = CERT_CheckCertValidTimes(cert, t, PR_FALSE);
    if ( validity != secCertTimeValid ) {
	return rv;
    }

    /* get cert's public key */
    pubKey = CERT_ExtractPublicKey(cert);
    if (pubKey) {
	rv =  CERT_VerifySignedDataWithPublicKey(sd, pubKey, wincx);
	SECKEY_DestroyPublicKey(pubKey);
    }
    return rv;
}


/* Software FORTEZZA installation hack. The software fortezza installer does
 * not have access to the krl and cert.db file. Accept FORTEZZA Certs without
 * KRL's in this case. 
 */
static int dont_use_krl = 0;
/* not a public exposed function... */
void sec_SetCheckKRLState(int value) { dont_use_krl = value; }

SECStatus
SEC_CheckKRL(CERTCertDBHandle *handle,SECKEYPublicKey *key,
	     CERTCertificate *rootCert, int64 t, void * wincx)
{
    CERTSignedCrl *crl = NULL;
    SECStatus rv = SECFailure;
    SECStatus rv2;
    CERTCrlEntry **crlEntry;
    SECCertTimeValidity validity;
    CERTCertificate *issuerCert = NULL;

    if (dont_use_krl) return SECSuccess;

    /* first look up the KRL */
    crl = SEC_FindCrlByName(handle,&rootCert->derSubject, SEC_KRL_TYPE);
    if (crl == NULL) {
	PORT_SetError(SEC_ERROR_NO_KRL);
	goto done;
    }

    /* get the issuing certificate */
    issuerCert = CERT_FindCertByName(handle, &crl->crl.derName);
    if (issuerCert == NULL) {
        PORT_SetError(SEC_ERROR_KRL_BAD_SIGNATURE);
        goto done;
    }


    /* now verify the KRL signature */
    rv2 = CERT_VerifySignedData(&crl->signatureWrap, issuerCert, t, wincx);
    if (rv2 != SECSuccess) {
	PORT_SetError(SEC_ERROR_KRL_BAD_SIGNATURE);
    	goto done;
    }

    /* Verify the date validity of the KRL */
    validity = SEC_CheckCrlTimes(&crl->crl, t);
    if (validity == secCertTimeExpired) {
	PORT_SetError(SEC_ERROR_KRL_EXPIRED);
	goto done;
    }

    /* now make sure the key in this cert is still valid */
    if (key->keyType != fortezzaKey) {
	PORT_SetError(SSL_ERROR_BAD_CERT_DOMAIN);
	goto done; /* This should be an assert? */
    }

    /* now make sure the key is not on the revocation list */
    for (crlEntry = crl->crl.entries; crlEntry && *crlEntry; crlEntry++) {
	if (PORT_Memcmp((*crlEntry)->serialNumber.data,
				key->u.fortezza.KMID,
				    (*crlEntry)->serialNumber.len) == 0) {
	    PORT_SetError(SEC_ERROR_REVOKED_KEY);
	    goto done;
	}
    }
    rv = SECSuccess;

done:
    if (issuerCert) CERT_DestroyCertificate(issuerCert);
    if (crl) SEC_DestroyCrl(crl);
    return rv;
}

SECStatus
SEC_CheckCRL(CERTCertDBHandle *handle,CERTCertificate *cert,
	     CERTCertificate *caCert, int64 t, void * wincx)
{
    return CERT_CheckCRL(cert, caCert, NULL, t, wincx);
}

/*
 * Find the issuer of a cert.  Use the authorityKeyID if it exists.
 */
CERTCertificate *
CERT_FindCertIssuer(CERTCertificate *cert, int64 validTime, SECCertUsage usage)
{
#ifdef NSS_CLASSIC
    CERTAuthKeyID *   authorityKeyID = NULL;  
    CERTCertificate * issuerCert     = NULL;
    SECItem *         caName;
    PRArenaPool       *tmpArena = NULL;

    tmpArena = PORT_NewArena(DER_DEFAULT_CHUNKSIZE);
    
    if ( !tmpArena ) {
	goto loser;
    }
    authorityKeyID = CERT_FindAuthKeyIDExten(tmpArena,cert);

    if ( authorityKeyID != NULL ) {
	/* has the authority key ID extension */
	if ( authorityKeyID->keyID.data != NULL ) {
	    /* extension contains a key ID, so lookup based on it */
	    issuerCert = CERT_FindCertByKeyID(cert->dbhandle, &cert->derIssuer,
					      &authorityKeyID->keyID);
	    if ( issuerCert == NULL ) {
		PORT_SetError (SEC_ERROR_UNKNOWN_ISSUER);
		goto loser;
	    }
	    
	} else if ( authorityKeyID->authCertIssuer != NULL ) {
	    /* no key ID, so try issuer and serial number */
	    caName = (SECItem*)CERT_GetGeneralNameByType(authorityKeyID->authCertIssuer,
					       		 certDirectoryName, PR_TRUE);

	    /*
	     * caName is NULL when the authCertIssuer field is not
	     * being used, or other name form is used instead.
	     * If the directoryName format and serialNumber fields are
	     * used, we use them to find the CA cert.
	     * Note:
	     *	By the time it gets here, we known for sure that if the
	     *	authCertIssuer exists, then the authCertSerialNumber
	     *	must also exists (CERT_DecodeAuthKeyID() ensures this).
	     *	We don't need to check again. 
	     */

	    if (caName != NULL) {
		CERTIssuerAndSN issuerSN;

		issuerSN.derIssuer.data = caName->data;
		issuerSN.derIssuer.len = caName->len;
		issuerSN.serialNumber.data = 
			authorityKeyID->authCertSerialNumber.data;
		issuerSN.serialNumber.len = 
			authorityKeyID->authCertSerialNumber.len;
		issuerCert = CERT_FindCertByIssuerAndSN(cert->dbhandle,
								&issuerSN);
		if ( issuerCert == NULL ) {
		    PORT_SetError (SEC_ERROR_UNKNOWN_ISSUER);
		    goto loser;
		}
	    }
	}
    }
    if ( issuerCert == NULL ) {
	/* if there is not authorityKeyID, then try to find the issuer */
	/* find a valid CA cert with correct usage */
	issuerCert = CERT_FindMatchingCert(cert->dbhandle,
					   &cert->derIssuer,
					   certOwnerCA, usage, PR_TRUE,
					   validTime, PR_TRUE);

	/* if that fails, then fall back to grabbing any cert with right name*/
	if ( issuerCert == NULL ) {
	    issuerCert = CERT_FindCertByName(cert->dbhandle, &cert->derIssuer);
	    if ( issuerCert == NULL ) {
		PORT_SetError (SEC_ERROR_UNKNOWN_ISSUER);
	    }
	}
    }

loser:
    if (tmpArena != NULL) {
	PORT_FreeArena(tmpArena, PR_FALSE);
	tmpArena = NULL;
    }

    return(issuerCert);
#else
    NSSCertificate *me;
    NSSTime *nssTime;
    NSSTrustDomain *td;
    NSSCryptoContext *cc;
    NSSCertificate *chain[3];
    NSSUsage nssUsage;
    PRStatus status;

    me = STAN_GetNSSCertificate(cert);
    if (!me) {
        PORT_SetError(SEC_ERROR_NO_MEMORY);
	return NULL;
    }
    nssTime = NSSTime_SetPRTime(NULL, validTime);
    nssUsage.anyUsage = PR_FALSE;
    nssUsage.nss3usage = usage;
    nssUsage.nss3lookingForCA = PR_TRUE;
    memset(chain, 0, 3*sizeof(NSSCertificate *));
    td   = STAN_GetDefaultTrustDomain();
    cc = STAN_GetDefaultCryptoContext();
    (void)NSSCertificate_BuildChain(me, nssTime, &nssUsage, NULL, 
                                    chain, 2, NULL, &status, td, cc);
    nss_ZFreeIf(nssTime);
    if (status == PR_SUCCESS) {
	PORT_Assert(me == chain[0]);
	/* if it's a root, the chain will only have one cert */
	if (!chain[1]) {
	    /* already has a reference from the call to BuildChain */
	    return cert;
	} 
	NSSCertificate_Destroy(chain[0]); /* the first cert in the chain */
	return STAN_GetCERTCertificate(chain[1]); /* return the 2nd */
    } 
    if (chain[0]) {
	PORT_Assert(me == chain[0]);
	NSSCertificate_Destroy(chain[0]); /* the first cert in the chain */
    }
    PORT_SetError (SEC_ERROR_UNKNOWN_ISSUER);
    return NULL;
#endif
}

/*
 * return required trust flags for various cert usages for CAs
 */
SECStatus
CERT_TrustFlagsForCACertUsage(SECCertUsage usage,
			      unsigned int *retFlags,
			      SECTrustType *retTrustType)
{
    unsigned int requiredFlags;
    SECTrustType trustType;

    switch ( usage ) {
      case certUsageSSLClient:
	requiredFlags = CERTDB_TRUSTED_CLIENT_CA;
	trustType = trustSSL;
        break;
      case certUsageSSLServer:
      case certUsageSSLCA:
	requiredFlags = CERTDB_TRUSTED_CA;
	trustType = trustSSL;
        break;
      case certUsageSSLServerWithStepUp:
	requiredFlags = CERTDB_TRUSTED_CA | CERTDB_GOVT_APPROVED_CA;
	trustType = trustSSL;
        break;
      case certUsageEmailSigner:
      case certUsageEmailRecipient:
	requiredFlags = CERTDB_TRUSTED_CA;
	trustType = trustEmail;
	break;
      case certUsageObjectSigner:
	requiredFlags = CERTDB_TRUSTED_CA;
	trustType = trustObjectSigning;
	break;
      case certUsageVerifyCA:
      case certUsageAnyCA:
      case certUsageStatusResponder:
	requiredFlags = CERTDB_TRUSTED_CA;
	trustType = trustTypeNone;
	break;
      default:
	PORT_Assert(0);
	goto loser;
    }
    if ( retFlags != NULL ) {
	*retFlags = requiredFlags;
    }
    if ( retTrustType != NULL ) {
	*retTrustType = trustType;
    }
    
    return(SECSuccess);
loser:
    return(SECFailure);
}





static void
AddToVerifyLog(CERTVerifyLog *log, CERTCertificate *cert, unsigned long error,
	       unsigned int depth, void *arg)
{
    CERTVerifyLogNode *node, *tnode;

    PORT_Assert(log != NULL);
    
    node = (CERTVerifyLogNode *)PORT_ArenaAlloc(log->arena,
						sizeof(CERTVerifyLogNode));
    if ( node != NULL ) {
	node->cert = CERT_DupCertificate(cert);
	node->error = error;
	node->depth = depth;
	node->arg = arg;
	
	if ( log->tail == NULL ) {
	    /* empty list */
	    log->head = log->tail = node;
	    node->prev = NULL;
	    node->next = NULL;
	} else if ( depth >= log->tail->depth ) {
	    /* add to tail */
	    node->prev = log->tail;
	    log->tail->next = node;
	    log->tail = node;
	    node->next = NULL;
	} else if ( depth < log->head->depth ) {
	    /* add at head */
	    node->prev = NULL;
	    node->next = log->head;
	    log->head->prev = node;
	    log->head = node;
	} else {
	    /* add in middle */
	    tnode = log->tail;
	    while ( tnode != NULL ) {
		if ( depth >= tnode->depth ) {
		    /* insert after tnode */
		    node->prev = tnode;
		    node->next = tnode->next;
		    tnode->next->prev = node;
		    tnode->next = node;
		    break;
		}

		tnode = tnode->prev;
	    }
	}

	log->count++;
    }
    return;
}

#define EXIT_IF_NOT_LOGGING(log) \
    if ( log == NULL ) { \
	goto loser; \
    }

#define LOG_ERROR_OR_EXIT(log,cert,depth,arg) \
    if ( log != NULL ) { \
	AddToVerifyLog(log, cert, PORT_GetError(), depth, (void *)arg); \
    } else { \
	goto loser; \
    }

#define LOG_ERROR(log,cert,depth,arg) \
    if ( log != NULL ) { \
	AddToVerifyLog(log, cert, PORT_GetError(), depth, (void *)arg); \
    }


typedef enum { cbd_None, cbd_User, cbd_CA } cbd_FortezzaType;

static SECStatus
cert_VerifyFortezzaV1Cert(CERTCertDBHandle *handle, CERTCertificate *cert,
	cbd_FortezzaType *next_type, cbd_FortezzaType last_type,
	int64 t, void *wincx)
{
    unsigned char priv = 0;
    SECKEYPublicKey *key;
    SECStatus rv;

    *next_type = cbd_CA;

    /* read the key */
    key = CERT_ExtractPublicKey(cert);

    /* Cant' get Key? fail. */
    if (key == NULL) {
    	PORT_SetError(SEC_ERROR_BAD_KEY);
	return SECFailure;
    }


    /* if the issuer is not an old fortezza cert, we bail */
    if (key->keyType != fortezzaKey) {
    	SECKEY_DestroyPublicKey(key);
	/* CA Cert not fortezza */
    	PORT_SetError(SEC_ERROR_NOT_FORTEZZA_ISSUER);
	return SECFailure;
    }

    /* get the privilege mask */
    if (key->u.fortezza.DSSpriviledge.len > 0) {
	priv = key->u.fortezza.DSSpriviledge.data[0];
    }

    /*
     * make sure the CA's keys are OK
     */
            
    rv = SEC_CheckKRL(handle, key, NULL, t, wincx);
    SECKEY_DestroyPublicKey(key);
    if (rv != SECSuccess) {
	return rv;
    }

    switch (last_type) {
      case cbd_User:
	/* first check for subordination */
	/*rv = FortezzaSubordinateCheck(cert,issuerCert);*/
	rv = SECSuccess;

	/* now check for issuer privilege */
	if ((rv != SECSuccess) || ((priv & 0x10) == 0)) {
	    /* bail */
	    PORT_SetError (SEC_ERROR_CA_CERT_INVALID);
	    return SECFailure;
	}
	break;
      case cbd_CA:
	if ((priv & 0x20) == 0) {
	    /* bail */
	    PORT_SetError (SEC_ERROR_CA_CERT_INVALID);
	    return SECFailure;
	}
	break;
      case cbd_None:
	*next_type = (priv & 0x30) ? cbd_CA : cbd_User;
	break;
      default:
	/* bail */ /* shouldn't ever happen */
    	PORT_SetError(SEC_ERROR_UNKNOWN_ISSUER);
	return SECFailure;
    }
    return SECSuccess;
}


static SECStatus
cert_VerifyCertChain(CERTCertDBHandle *handle, CERTCertificate *cert,
		     PRBool checkSig, PRBool* sigerror,
                     SECCertUsage certUsage, int64 t, void *wincx,
                     CERTVerifyLog *log, PRBool* revoked)
{
    SECTrustType trustType;
    CERTBasicConstraints basicConstraint;
    CERTCertificate *issuerCert = NULL;
    CERTCertificate *subjectCert = NULL;
    CERTCertificate *badCert = NULL;
    PRBool isca;
    PRBool isFortezzaV1 = PR_FALSE;
    SECStatus rv;
    SECStatus rvFinal = SECSuccess;
    int count;
    int currentPathLen = 0;
    int pathLengthLimit = CERT_UNLIMITED_PATH_CONSTRAINT;
    int flags;
    unsigned int caCertType;
    unsigned int requiredCAKeyUsage;
    unsigned int requiredFlags;
    PRArenaPool *arena = NULL;
    CERTGeneralName *namesList = NULL;
    CERTCertificate **certsList      = NULL;
    int certsListLen = 16;
    int namesCount = 0;
    PRBool subjectCertIsSelfIssued;

    cbd_FortezzaType last_type = cbd_None;

    if (revoked) {
        *revoked = PR_FALSE;
    }

    if (CERT_KeyUsageAndTypeForCertUsage(certUsage, PR_TRUE,
					 &requiredCAKeyUsage,
					 &caCertType)
	!= SECSuccess ) {
	PORT_Assert(0);
	EXIT_IF_NOT_LOGGING(log);
	requiredCAKeyUsage = 0;
	caCertType = 0;
    }

    switch ( certUsage ) {
      case certUsageSSLClient:
      case certUsageSSLServer:
      case certUsageSSLCA:
      case certUsageSSLServerWithStepUp:
      case certUsageEmailSigner:
      case certUsageEmailRecipient:
      case certUsageObjectSigner:
      case certUsageVerifyCA:
      case certUsageStatusResponder:
	if ( CERT_TrustFlagsForCACertUsage(certUsage, &requiredFlags,
					   &trustType) != SECSuccess ) {
	    PORT_Assert(0);
	    EXIT_IF_NOT_LOGGING(log);
	    requiredFlags = 0;
	    trustType = trustSSL;
	}
	break;
      default:
	PORT_Assert(0);
	EXIT_IF_NOT_LOGGING(log);
	requiredFlags = 0;
	trustType = trustSSL;/* This used to be 0, but we need something
			      * that matches the enumeration type.
			      */
	caCertType = 0;
    }
    
    subjectCert = CERT_DupCertificate(cert);
    if ( subjectCert == NULL ) {
	goto loser;
    }

    /* determine if the cert is fortezza.
     */
    isFortezzaV1 = (PRBool)
	(CERT_GetCertKeyType(&subjectCert->subjectPublicKeyInfo) 
							== fortezzaKey);

    if (isFortezzaV1) {
	rv = cert_VerifyFortezzaV1Cert(handle, subjectCert, &last_type, 
						cbd_None, t, wincx);
	if (rv == SECFailure) {
	    /**** PORT_SetError is already set by cert_VerifyFortezzaV1Cert **/
	    LOG_ERROR_OR_EXIT(log,subjectCert,0,0);
	}
    }

    arena = PORT_NewArena(DER_DEFAULT_CHUNKSIZE);
    if (arena == NULL) {
	goto loser;
    }

    certsList = PORT_ZNewArray(CERTCertificate *, certsListLen);
    if (certsList == NULL)
	goto loser;

    /* RFC 3280 says that the name constraints will apply to the names
    ** in the leaf (EE) cert, whether it is self issued or not, so
    ** we pretend that it is not.
    */
    subjectCertIsSelfIssued = PR_FALSE;
    for ( count = 0; count < CERT_MAX_CERT_CHAIN; count++ ) {
	PRBool validCAOverride = PR_FALSE;

	/* Construct a list of names for the current and all previous 
	 * certifcates (except leaf (EE) certs, root CAs, and self-issued
	 * intermediate CAs) to be verified against the name constraints 
	 * extension of the issuer certificate. 
	 */
	if (subjectCertIsSelfIssued == PR_FALSE) {
	    CERTGeneralName *subjectNameList;
	    int subjectNameListLen;
	    int i;
	    subjectNameList    = CERT_GetCertificateNames(subjectCert, arena);
	    subjectNameListLen = CERT_GetNamesLength(subjectNameList);
	    if (certsListLen <= namesCount + subjectNameListLen) {
		CERTCertificate **tmpCertsList;
		certsListLen = (namesCount + subjectNameListLen) * 2;
		tmpCertsList = 
		    (CERTCertificate **)PORT_Realloc(certsList, 
	                            certsListLen * sizeof(CERTCertificate *));
		if (tmpCertsList == NULL) {
		    goto loser;
		}
		certsList = tmpCertsList;
	    }
	    for (i = 0; i < subjectNameListLen; i++) {
		certsList[namesCount + i] = subjectCert;
	    }
	    namesCount += subjectNameListLen;
	    namesList = cert_CombineNamesLists(namesList, subjectNameList);
	}

        /* check if the cert has an unsupported critical extension */
	if ( subjectCert->options.bits.hasUnsupportedCriticalExt ) {
	    PORT_SetError(SEC_ERROR_UNKNOWN_CRITICAL_EXTENSION);
	    LOG_ERROR_OR_EXIT(log,subjectCert,count,0);
	}

	/* find the certificate of the issuer */
	issuerCert = CERT_FindCertIssuer(subjectCert, t, certUsage);
	if ( ! issuerCert ) {
	    PORT_SetError(SEC_ERROR_UNKNOWN_ISSUER);
	    LOG_ERROR(log,subjectCert,count,0);
	    goto loser;
	}

	/* verify the signature on the cert */
	if ( checkSig ) {
	    rv = CERT_VerifySignedData(&subjectCert->signatureWrap,
				       issuerCert, t, wincx);
    
	    if ( rv != SECSuccess ) {
                if (sigerror) {
                    *sigerror = PR_TRUE;
                }
		if ( PORT_GetError() == SEC_ERROR_EXPIRED_CERTIFICATE ) {
		    PORT_SetError(SEC_ERROR_EXPIRED_ISSUER_CERTIFICATE);
		    LOG_ERROR_OR_EXIT(log,issuerCert,count+1,0);
		} else {
		    PORT_SetError(SEC_ERROR_BAD_SIGNATURE);
		    LOG_ERROR_OR_EXIT(log,subjectCert,count,0);
		}
	    }
	}

	/*
	 * XXX - fortezza may need error logging stuff added
	 */
	if (isFortezzaV1) {
	    rv = cert_VerifyFortezzaV1Cert(handle, issuerCert, &last_type, 
					last_type, t, wincx);
	    if (rv == SECFailure) {
		/**** PORT_SetError is already set by *
		 * cert_VerifyFortezzaV1Cert **/
		LOG_ERROR_OR_EXIT(log,subjectCert,0,0);
	    }
	}

	/* If the basicConstraint extension is included in an immediate CA
	 * certificate, make sure that the isCA flag is on.  If the
	 * pathLenConstraint component exists, it must be greater than the
	 * number of CA certificates we have seen so far.  If the extension
	 * is omitted, we will assume that this is a CA certificate with
	 * an unlimited pathLenConstraint (since it already passes the
	 * netscape-cert-type extension checking).
	 *
	 * In the fortezza (V1) case, we've already checked the CA bits
	 * in the key, so we're presumed to be a CA; however we really don't
	 * want to bypass Basic constraint or netscape extension parsing.
         * 
         * In Fortezza V2, basicConstraint will be set for every CA,PCA,PAA
	 */

	rv = CERT_FindBasicConstraintExten(issuerCert, &basicConstraint);
	if ( rv != SECSuccess ) {
	    if (PORT_GetError() != SEC_ERROR_EXTENSION_NOT_FOUND) {
		LOG_ERROR_OR_EXIT(log,issuerCert,count+1,0);
	    } 
	    pathLengthLimit = CERT_UNLIMITED_PATH_CONSTRAINT;
	    /* no basic constraints found, if we're fortezza, CA bit is already
	     * verified (isca = PR_TRUE). otherwise, we aren't (yet) a ca
	     * isca = PR_FALSE */
	    isca = isFortezzaV1;
	} else  {
	    if ( basicConstraint.isCA == PR_FALSE ) {
		PORT_SetError (SEC_ERROR_CA_CERT_INVALID);
		LOG_ERROR_OR_EXIT(log,issuerCert,count+1,0);
	    }
	    pathLengthLimit = basicConstraint.pathLenConstraint;
	    isca = PR_TRUE;
	}    
	/* make sure that the path len constraint is properly set.*/
	if (pathLengthLimit >= 0 && currentPathLen > pathLengthLimit) {
	    PORT_SetError (SEC_ERROR_PATH_LEN_CONSTRAINT_INVALID);
	    LOG_ERROR_OR_EXIT(log, issuerCert, count+1, pathLengthLimit);
	}
	
	/* XXX - the error logging may need to go down into CRL stuff at some
	 * point
	 */
	/* check revoked list (issuer) */
        rv = SEC_CheckCRL(handle, subjectCert, issuerCert, t, wincx);
        if (rv == SECFailure) {
            if (revoked) {
                *revoked = PR_TRUE;
            }
            LOG_ERROR_OR_EXIT(log,subjectCert,count,0);
        } else if (rv == SECWouldBlock) {
            /* We found something fishy, so we intend to issue an
             * error to the user, but the user may wish to continue
             * processing, in which case we better make sure nothing
             * worse has happened... so keep cranking the loop */
            rvFinal = SECFailure;
            if (revoked) {
                *revoked = PR_TRUE;
            }
            LOG_ERROR(log,subjectCert,count,0);
        }

	if ( issuerCert->trust ) {
	    /* we have some trust info, but this does NOT imply that this
	     * cert is actually trusted for any purpose.  The cert may be
	     * explicitly UNtrusted.  We won't know until we examine the
	     * trust bits.
	     */
	    if (certUsage == certUsageStatusResponder) {
		/* XXX NSS has done this for years, but it seems incorrect. */
		rv = rvFinal;
		goto done;
	    }

	    /*
	     * check the trust parms of the issuer
	     */
	    if ( certUsage == certUsageVerifyCA ) {
		if ( subjectCert->nsCertType & NS_CERT_TYPE_EMAIL_CA ) {
		    trustType = trustEmail;
		} else if ( subjectCert->nsCertType & NS_CERT_TYPE_SSL_CA ) {
		    trustType = trustSSL;
		} else {
		    trustType = trustObjectSigning;
		}
	    }
	    
	    flags = SEC_GET_TRUST_FLAGS(issuerCert->trust, trustType);
	    
	    if (flags & CERTDB_VALID_CA) {
		if ( ( flags & requiredFlags ) == requiredFlags) {
		    /* we found a trusted one, so return */
		    rv = rvFinal; 
		    goto done;
		}
		validCAOverride = PR_TRUE;
	    }
	}

	if (!validCAOverride) {
	    /*
	     * Make sure that if this is an intermediate CA in the chain that
	     * it was given permission by its signer to be a CA.
	     */
	    /*
	     * if basicConstraints says it is a ca, then we check the
	     * nsCertType.  If the nsCertType has any CA bits set, then
	     * it must have the right one.
	     */
	    if (!isca || (issuerCert->nsCertType & NS_CERT_TYPE_CA)) {
		isca = (issuerCert->nsCertType & caCertType) ? PR_TRUE : PR_FALSE;
	    }
	
	    if (  !isca  ) {
		PORT_SetError(SEC_ERROR_CA_CERT_INVALID);
		LOG_ERROR_OR_EXIT(log,issuerCert,count+1,0);
	    }

	    /* make sure key usage allows cert signing */
	    if (CERT_CheckKeyUsage(issuerCert, requiredCAKeyUsage) != SECSuccess) {
		PORT_SetError(SEC_ERROR_INADEQUATE_KEY_USAGE);
		LOG_ERROR_OR_EXIT(log,issuerCert,count+1,requiredCAKeyUsage);
	    }
	}

	/* make sure that the entire chain is within the name space of the 
	** current issuer certificate.
	*/
	rv = CERT_CompareNameSpace(issuerCert, namesList, certsList, 
	                           arena, &badCert);
	if (rv != SECSuccess || badCert != NULL) {
	    PORT_SetError(SEC_ERROR_CERT_NOT_IN_NAME_SPACE);
            LOG_ERROR_OR_EXIT(log, badCert, count + 1, 0);
	    goto loser;
	}
	/* make sure that the issuer is not self signed.  If it is, then
	 * stop here to prevent looping.
	 */
	if (issuerCert->isRoot) {
	    PORT_SetError(SEC_ERROR_UNTRUSTED_ISSUER);
	    LOG_ERROR(log, issuerCert, count+1, 0);
	    goto loser;
	} 
	/* The issuer cert will be the subject cert in the next loop.
	 * A cert is self-issued if its subject and issuer are equal and
	 * both are of non-zero length. 
	 */
	subjectCertIsSelfIssued = (PRBool)
	    SECITEM_ItemsAreEqual(&issuerCert->derIssuer, 
				  &issuerCert->derSubject) &&
	    issuerCert->derSubject.len > 0;
	if (subjectCertIsSelfIssued == PR_FALSE) {
	    /* RFC 3280 says only non-self-issued intermediate CA certs 
	     * count in path length.
	     */
	    ++currentPathLen;
	}

	CERT_DestroyCertificate(subjectCert);
	subjectCert = issuerCert;
	issuerCert = NULL;
    }

    PORT_SetError(SEC_ERROR_UNKNOWN_ISSUER);
    LOG_ERROR(log,subjectCert,count,0);
loser:
    rv = SECFailure;
done:
    if (certsList != NULL) {
	PORT_Free(certsList);
    }
    if ( issuerCert ) {
	CERT_DestroyCertificate(issuerCert);
    }
    
    if ( subjectCert ) {
	CERT_DestroyCertificate(subjectCert);
    }

    if ( arena != NULL ) {
	PORT_FreeArena(arena, PR_FALSE);
    }
    return rv;
}

SECStatus
CERT_VerifyCertChain(CERTCertDBHandle *handle, CERTCertificate *cert,
		     PRBool checkSig, SECCertUsage certUsage, int64 t,
		     void *wincx, CERTVerifyLog *log)
{
    return cert_VerifyCertChain(handle, cert, checkSig, NULL, certUsage, t,
			 wincx, log, NULL);
}

/*
 * verify that a CA can sign a certificate with the requested usage.
 * XXX This function completely ignores cert path length constraints!
 */
SECStatus
CERT_VerifyCACertForUsage(CERTCertDBHandle *handle, CERTCertificate *cert,
		PRBool checkSig, SECCertUsage certUsage, int64 t,
		void *wincx, CERTVerifyLog *log)
{
    SECTrustType trustType;
    CERTBasicConstraints basicConstraint;
    PRBool isca;
    PRBool validCAOverride = PR_FALSE;
    SECStatus rv;
    SECComparison rvCompare;
    SECStatus rvFinal = SECSuccess;
    int flags;
    unsigned int caCertType;
    unsigned int requiredCAKeyUsage;
    unsigned int requiredFlags;
    CERTCertificate *issuerCert;


    if (CERT_KeyUsageAndTypeForCertUsage(certUsage, PR_TRUE,
					 &requiredCAKeyUsage,
					 &caCertType) != SECSuccess ) {
	PORT_Assert(0);
	EXIT_IF_NOT_LOGGING(log);
	requiredCAKeyUsage = 0;
	caCertType = 0;
    }

    switch ( certUsage ) {
      case certUsageSSLClient:
      case certUsageSSLServer:
      case certUsageSSLCA:
      case certUsageSSLServerWithStepUp:
      case certUsageEmailSigner:
      case certUsageEmailRecipient:
      case certUsageObjectSigner:
      case certUsageVerifyCA:
      case certUsageStatusResponder:
	if ( CERT_TrustFlagsForCACertUsage(certUsage, &requiredFlags,
					   &trustType) != SECSuccess ) {
	    PORT_Assert(0);
	    EXIT_IF_NOT_LOGGING(log);
	    requiredFlags = 0;
	    trustType = trustSSL;
	}
	break;
      default:
	PORT_Assert(0);
	EXIT_IF_NOT_LOGGING(log);
	requiredFlags = 0;
	trustType = trustSSL;/* This used to be 0, but we need something
			      * that matches the enumeration type.
			      */
	caCertType = 0;
    }
    
    /* If the basicConstraint extension is included in an intermmediate CA
     * certificate, make sure that the isCA flag is on.  If the
     * pathLenConstraint component exists, it must be greater than the
     * number of CA certificates we have seen so far.  If the extension
     * is omitted, we will assume that this is a CA certificate with
     * an unlimited pathLenConstraint (since it already passes the
     * netscape-cert-type extension checking).
     *
     * In the fortezza (V1) case, we've already checked the CA bits
     * in the key, so we're presumed to be a CA; however we really don't
     * want to bypass Basic constraint or netscape extension parsing.
     * 
     * In Fortezza V2, basicConstraint will be set for every CA,PCA,PAA
     */

    rv = CERT_FindBasicConstraintExten(cert, &basicConstraint);
    if ( rv != SECSuccess ) {
	if (PORT_GetError() != SEC_ERROR_EXTENSION_NOT_FOUND) {
	    LOG_ERROR_OR_EXIT(log,cert,0,0);
	} 
	/* no basic constraints found, if we're fortezza, CA bit is already
	 * verified (isca = PR_TRUE). otherwise, we aren't (yet) a ca
	 * isca = PR_FALSE */
	isca = PR_FALSE;
    } else  {
	if ( basicConstraint.isCA == PR_FALSE ) {
	    PORT_SetError (SEC_ERROR_CA_CERT_INVALID);
	    LOG_ERROR_OR_EXIT(log,cert,0,0);
	}

	/* can't check path length if we don't know the previous path */
	isca = PR_TRUE;
    }
	
    if ( cert->trust ) {
	/* we have some trust info, but this does NOT imply that this
	 * cert is actually trusted for any purpose.  The cert may be
	 * explicitly UNtrusted.  We won't know until we examine the
	 * trust bits.
	 */
        if (certUsage == certUsageStatusResponder) {
	    /* Check the special case of certUsageStatusResponder */
            issuerCert = CERT_FindCertIssuer(cert, t, certUsage);
            if (issuerCert) {
                if (SEC_CheckCRL(handle, cert, issuerCert, t, wincx) 
		    != SECSuccess) {
                    PORT_SetError(SEC_ERROR_REVOKED_CERTIFICATE);
                    CERT_DestroyCertificate(issuerCert);
                    goto loser;
                }
                CERT_DestroyCertificate(issuerCert);
            }
	    /* XXX We have NOT determined that this cert is trusted.
	     * For years, NSS has treated this as trusted, 
	     * but it seems incorrect.
	     */
	    rv = rvFinal; 
	    goto done;
        }

	/*
	 * check the trust parms of the issuer
	 */
	flags = SEC_GET_TRUST_FLAGS(cert->trust, trustType);
	    
	if (flags & CERTDB_VALID_CA) {
	    if ( ( flags & requiredFlags ) == requiredFlags) {
		/* we found a trusted one, so return */
		rv = rvFinal; 
		goto done;
	    }
	    validCAOverride = PR_TRUE;
	}
    }
    if (!validCAOverride) {
	/*
	 * Make sure that if this is an intermediate CA in the chain that
	 * it was given permission by its signer to be a CA.
	 */
	/*
	 * if basicConstraints says it is a ca, then we check the
	 * nsCertType.  If the nsCertType has any CA bits set, then
	 * it must have the right one.
	 */
	if (!isca || (cert->nsCertType & NS_CERT_TYPE_CA)) {
	    isca = (cert->nsCertType & caCertType) ? PR_TRUE : PR_FALSE;
	}
	
	if (!isca) {
	    PORT_SetError(SEC_ERROR_CA_CERT_INVALID);
	    LOG_ERROR_OR_EXIT(log,cert,0,0);
	}
	    
	/* make sure key usage allows cert signing */
	if (CERT_CheckKeyUsage(cert, requiredCAKeyUsage) != SECSuccess) {
	    PORT_SetError(SEC_ERROR_INADEQUATE_KEY_USAGE);
	    LOG_ERROR_OR_EXIT(log,cert,0,requiredCAKeyUsage);
	}
    }
    /* make sure that the issuer is not self signed.  If it is, then
     * stop here to prevent looping.
     */
    rvCompare = SECITEM_CompareItem(&cert->derSubject, &cert->derIssuer);
    if (rvCompare == SECEqual) {
	    PORT_SetError(SEC_ERROR_UNTRUSTED_ISSUER);
	    LOG_ERROR(log, cert, 0, 0);
	    goto loser;
    }

    return CERT_VerifyCertChain(handle, cert, checkSig, certUsage, t, 
		     					wincx, log);
loser:
    rv = SECFailure;
done:
    return rv;
}

#define NEXT_USAGE() { \
    i*=2; \
    certUsage++; \
    continue; \
}

#define VALID_USAGE() { \
    NEXT_USAGE(); \
}

#define INVALID_USAGE() { \
    if (returnedUsages) { \
        *returnedUsages &= (~i); \
    } \
    if (PR_TRUE == requiredUsage) { \
        valid = SECFailure; \
    } \
    NEXT_USAGE(); \
}

/*
 * verify a certificate by checking if it's valid and that we
 * trust the issuer.
 *
 * certificateUsage contains a bitfield of all cert usages that are
 * required for verification to succeed
 *
 * a bitfield of cert usages is returned in *returnedUsages
 * if requiredUsages is non-zero, the returned bitmap is only
 * for those required usages, otherwise it is for all usages
 *
 */
SECStatus
CERT_VerifyCertificate(CERTCertDBHandle *handle, CERTCertificate *cert,
		PRBool checkSig, SECCertificateUsage requiredUsages, int64 t,
		void *wincx, CERTVerifyLog *log, SECCertificateUsage* returnedUsages)
{
    SECStatus rv;
    SECStatus valid;
    unsigned int requiredKeyUsage;
    unsigned int requiredCertType;
    unsigned int flags;
    unsigned int certType;
    PRBool       allowOverride;
    SECCertTimeValidity validity;
    CERTStatusConfig *statusConfig;
    PRInt32 i;
    SECCertUsage certUsage = 0;
    PRBool checkedOCSP = PR_FALSE;
    PRBool checkAllUsages = PR_FALSE;
    PRBool revoked = PR_FALSE;
    PRBool sigerror = PR_FALSE;

    if (!requiredUsages) {
        /* there are no required usages, so the user probably wants to
           get status for all usages */
        checkAllUsages = PR_TRUE;
    }

    if (returnedUsages) {
        *returnedUsages = 0;
    } else {
        /* we don't have a place to return status for all usages,
           so we can skip checks for usages that aren't required */
        checkAllUsages = PR_FALSE;
    }
    valid = SECSuccess ; /* start off assuming cert is valid */
   
#ifdef notdef 
    /* check if this cert is in the Evil list */
    rv = CERT_CheckForEvilCert(cert);
    if ( rv != SECSuccess ) {
	PORT_SetError(SEC_ERROR_REVOKED_CERTIFICATE);
	LOG_ERROR(log,cert,0,0);
	return SECFailure;
    }
#endif
    
    /* make sure that the cert is valid at time t */
    allowOverride = (PRBool)((requiredUsages & certificateUsageSSLServer) ||
                             (requiredUsages & certificateUsageSSLServerWithStepUp));
    validity = CERT_CheckCertValidTimes(cert, t, allowOverride);
    if ( validity != secCertTimeValid ) {
        LOG_ERROR(log,cert,0,validity);
	return SECFailure;
    }

    /* check key usage and netscape cert type */
    cert_GetCertType(cert);
    certType = cert->nsCertType;

    for (i=1;i<=certificateUsageHighest && !(SECFailure == valid && !returnedUsages) ;) {
        PRBool requiredUsage = (i & requiredUsages) ? PR_TRUE : PR_FALSE;
        if (PR_FALSE == requiredUsage && PR_FALSE == checkAllUsages) {
            NEXT_USAGE();
        }
        if (returnedUsages) {
            *returnedUsages |= i; /* start off assuming this usage is valid */
        }
        switch ( certUsage ) {
          case certUsageSSLClient:
          case certUsageSSLServer:
          case certUsageSSLServerWithStepUp:
          case certUsageSSLCA:
          case certUsageEmailSigner:
          case certUsageEmailRecipient:
          case certUsageObjectSigner:
          case certUsageStatusResponder:
            rv = CERT_KeyUsageAndTypeForCertUsage(certUsage, PR_FALSE,
                                                  &requiredKeyUsage,
                                                  &requiredCertType);
            if ( rv != SECSuccess ) {
                PORT_Assert(0);
                /* EXIT_IF_NOT_LOGGING(log); XXX ??? */
                requiredKeyUsage = 0;
                requiredCertType = 0;
                INVALID_USAGE();
            }
            break;

          case certUsageAnyCA:
          case certUsageProtectedObjectSigner:
          case certUsageUserCertImport:
          case certUsageVerifyCA:
              /* these usages cannot be verified */
              NEXT_USAGE();

          default:
            PORT_Assert(0);
            requiredKeyUsage = 0;
            requiredCertType = 0;
            INVALID_USAGE();
        }
        if ( CERT_CheckKeyUsage(cert, requiredKeyUsage) != SECSuccess ) {
            if (PR_TRUE == requiredUsage) {
                PORT_SetError(SEC_ERROR_INADEQUATE_KEY_USAGE);
            }
            LOG_ERROR(log,cert,0,requiredKeyUsage);
            INVALID_USAGE();
        }
        if ( !( certType & requiredCertType ) ) {
            if (PR_TRUE == requiredUsage) {
                PORT_SetError(SEC_ERROR_INADEQUATE_CERT_TYPE);
            }
            LOG_ERROR(log,cert,0,requiredCertType);
            INVALID_USAGE();
        }

        /* check trust flags to see if this cert is directly trusted */
        if ( cert->trust ) { /* the cert is in the DB */
            switch ( certUsage ) {
              case certUsageSSLClient:
              case certUsageSSLServer:
                flags = cert->trust->sslFlags;

                /* is the cert directly trusted or not trusted ? */
                if ( flags & CERTDB_VALID_PEER ) {/*the trust record is valid*/
                    if ( flags & CERTDB_TRUSTED ) {	/* trust this cert */
                        VALID_USAGE();
                    } else { /* don't trust this cert */
                        if (PR_TRUE == requiredUsage) {
                            PORT_SetError(SEC_ERROR_UNTRUSTED_CERT);
                        }
                        LOG_ERROR(log,cert,0,flags);
                        INVALID_USAGE();
                    }
                }
                break;
              case certUsageSSLServerWithStepUp:
                /* XXX - step up certs can't be directly trusted */
                break;
              case certUsageSSLCA:
                break;
              case certUsageEmailSigner:
              case certUsageEmailRecipient:
                flags = cert->trust->emailFlags;

                /* is the cert directly trusted or not trusted ? */
                if ( ( flags & ( CERTDB_VALID_PEER | CERTDB_TRUSTED ) ) ==
                    ( CERTDB_VALID_PEER | CERTDB_TRUSTED ) ) {
                    VALID_USAGE();
                }
                break;
              case certUsageObjectSigner:
                flags = cert->trust->objectSigningFlags;

                /* is the cert directly trusted or not trusted ? */
                if ( flags & CERTDB_VALID_PEER ) {/*the trust record is valid*/
                    if ( flags & CERTDB_TRUSTED ) {	/* trust this cert */
                        VALID_USAGE();
                    } else { /* don't trust this cert */
                        if (PR_TRUE == requiredUsage) {
                            PORT_SetError(SEC_ERROR_UNTRUSTED_CERT);
                        }
                        LOG_ERROR(log,cert,0,flags);
                        INVALID_USAGE();
                    }
                }
                break;
              case certUsageVerifyCA:
              case certUsageStatusResponder:
                flags = cert->trust->sslFlags;
                /* is the cert directly trusted or not trusted ? */
                if ( ( flags & ( CERTDB_VALID_CA | CERTDB_TRUSTED_CA ) ) ==
                    ( CERTDB_VALID_CA | CERTDB_TRUSTED_CA ) ) {
                    VALID_USAGE();
                }
                flags = cert->trust->emailFlags;
                /* is the cert directly trusted or not trusted ? */
                if ( ( flags & ( CERTDB_VALID_CA | CERTDB_TRUSTED_CA ) ) ==
                    ( CERTDB_VALID_CA | CERTDB_TRUSTED_CA ) ) {
                    VALID_USAGE();
                }
                flags = cert->trust->objectSigningFlags;
                /* is the cert directly trusted or not trusted ? */
                if ( ( flags & ( CERTDB_VALID_CA | CERTDB_TRUSTED_CA ) ) ==
                    ( CERTDB_VALID_CA | CERTDB_TRUSTED_CA ) ) {
                    VALID_USAGE();
                }
                break;
              case certUsageAnyCA:
              case certUsageProtectedObjectSigner:
              case certUsageUserCertImport:
                /* XXX to make the compiler happy.  Should these be
                 * explicitly handled?
                 */
                break;
            }
        }

        if (PR_TRUE == revoked || PR_TRUE == sigerror) {
            INVALID_USAGE();
        }

        rv = cert_VerifyCertChain(handle, cert,
            checkSig, &sigerror,
            certUsage, t, wincx, log,
            &revoked);

        if (rv != SECSuccess) {
            /* EXIT_IF_NOT_LOGGING(log); XXX ???? */
            INVALID_USAGE();
        }

        /*
         * Check OCSP revocation status, but only if the cert we are checking
         * is not a status reponder itself.  We only do this in the case
         * where we checked the cert chain (above); explicit trust "wins"
         * (avoids status checking, just as it avoids CRL checking) by
         * bypassing this code.
         */

        if (PR_FALSE == checkedOCSP) {
            checkedOCSP = PR_TRUE; /* only check OCSP once */
            statusConfig = CERT_GetStatusConfig(handle);
            if ( (! (requiredUsages & certificateUsageStatusResponder)) &&
                statusConfig != NULL) {
                if (statusConfig->statusChecker != NULL) {
                    rv = (* statusConfig->statusChecker)(handle, cert,
                                                                 t, wincx);
                    if (rv != SECSuccess) {
                        LOG_ERROR(log,cert,0,0);
                        revoked = PR_TRUE;
                        INVALID_USAGE();
                    }
                }
            }
        }

        NEXT_USAGE();
    }
    
    return(valid);
}
			
/* obsolete, do not use for new code */
SECStatus
CERT_VerifyCert(CERTCertDBHandle *handle, CERTCertificate *cert,
		PRBool checkSig, SECCertUsage certUsage, int64 t,
		void *wincx, CERTVerifyLog *log)
{
    SECStatus rv;
    unsigned int requiredKeyUsage;
    unsigned int requiredCertType;
    unsigned int flags;
    unsigned int certType;
    PRBool       allowOverride;
    SECCertTimeValidity validity;
    CERTStatusConfig *statusConfig;
   
#ifdef notdef 
    /* check if this cert is in the Evil list */
    rv = CERT_CheckForEvilCert(cert);
    if ( rv != SECSuccess ) {
	PORT_SetError(SEC_ERROR_REVOKED_CERTIFICATE);
	LOG_ERROR_OR_EXIT(log,cert,0,0);
    }
#endif
    
    /* make sure that the cert is valid at time t */
    allowOverride = (PRBool)((certUsage == certUsageSSLServer) ||
                             (certUsage == certUsageSSLServerWithStepUp));
    validity = CERT_CheckCertValidTimes(cert, t, allowOverride);
    if ( validity != secCertTimeValid ) {
	LOG_ERROR_OR_EXIT(log,cert,0,validity);
    }

    /* check key usage and netscape cert type */
    cert_GetCertType(cert);
    certType = cert->nsCertType;
    switch ( certUsage ) {
      case certUsageSSLClient:
      case certUsageSSLServer:
      case certUsageSSLServerWithStepUp:
      case certUsageSSLCA:
      case certUsageEmailSigner:
      case certUsageEmailRecipient:
      case certUsageObjectSigner:
      case certUsageStatusResponder:
	rv = CERT_KeyUsageAndTypeForCertUsage(certUsage, PR_FALSE,
					      &requiredKeyUsage,
					      &requiredCertType);
	if ( rv != SECSuccess ) {
	    PORT_Assert(0);
	    EXIT_IF_NOT_LOGGING(log);
	    requiredKeyUsage = 0;
	    requiredCertType = 0;
	}
	break;
      case certUsageVerifyCA:
	requiredKeyUsage = KU_KEY_CERT_SIGN;
	requiredCertType = NS_CERT_TYPE_CA;
	if ( ! ( certType & NS_CERT_TYPE_CA ) ) {
	    certType |= NS_CERT_TYPE_CA;
	}
	break;
      default:
	PORT_Assert(0);
	EXIT_IF_NOT_LOGGING(log);
	requiredKeyUsage = 0;
	requiredCertType = 0;
    }
    if ( CERT_CheckKeyUsage(cert, requiredKeyUsage) != SECSuccess ) {
	PORT_SetError(SEC_ERROR_INADEQUATE_KEY_USAGE);
	LOG_ERROR_OR_EXIT(log,cert,0,requiredKeyUsage);
    }
    if ( !( certType & requiredCertType ) ) {
	PORT_SetError(SEC_ERROR_INADEQUATE_CERT_TYPE);
	LOG_ERROR_OR_EXIT(log,cert,0,requiredCertType);
    }

    /* check trust flags to see if this cert is directly trusted */
    if ( cert->trust ) { /* the cert is in the DB */
	switch ( certUsage ) {
	  case certUsageSSLClient:
	  case certUsageSSLServer:
	    flags = cert->trust->sslFlags;
	    
	    /* is the cert directly trusted or not trusted ? */
	    if ( flags & CERTDB_VALID_PEER ) {/*the trust record is valid*/
		if ( flags & CERTDB_TRUSTED ) {	/* trust this cert */
		    goto winner;
		} else { /* don't trust this cert */
		    PORT_SetError(SEC_ERROR_UNTRUSTED_CERT);
		    LOG_ERROR_OR_EXIT(log,cert,0,flags);
		}
	    }
	    break;
	  case certUsageSSLServerWithStepUp:
	    /* XXX - step up certs can't be directly trusted */
	    break;
	  case certUsageSSLCA:
	    break;
	  case certUsageEmailSigner:
	  case certUsageEmailRecipient:
	    flags = cert->trust->emailFlags;
	    
	    /* is the cert directly trusted or not trusted ? */
	    if ( ( flags & ( CERTDB_VALID_PEER | CERTDB_TRUSTED ) ) ==
		( CERTDB_VALID_PEER | CERTDB_TRUSTED ) ) {
		goto winner;
	    }
	    break;
	  case certUsageObjectSigner:
	    flags = cert->trust->objectSigningFlags;

	    /* is the cert directly trusted or not trusted ? */
	    if ( flags & CERTDB_VALID_PEER ) {/*the trust record is valid*/
		if ( flags & CERTDB_TRUSTED ) {	/* trust this cert */
		    goto winner;
		} else { /* don't trust this cert */
		    PORT_SetError(SEC_ERROR_UNTRUSTED_CERT);
		    LOG_ERROR_OR_EXIT(log,cert,0,flags);
		}
	    }
	    break;
	  case certUsageVerifyCA:
	  case certUsageStatusResponder:
	    flags = cert->trust->sslFlags;
	    /* is the cert directly trusted or not trusted ? */
	    if ( ( flags & ( CERTDB_VALID_CA | CERTDB_TRUSTED_CA ) ) ==
		( CERTDB_VALID_CA | CERTDB_TRUSTED_CA ) ) {
		goto winner;
	    }
	    flags = cert->trust->emailFlags;
	    /* is the cert directly trusted or not trusted ? */
	    if ( ( flags & ( CERTDB_VALID_CA | CERTDB_TRUSTED_CA ) ) ==
		( CERTDB_VALID_CA | CERTDB_TRUSTED_CA ) ) {
		goto winner;
	    }
	    flags = cert->trust->objectSigningFlags;
	    /* is the cert directly trusted or not trusted ? */
	    if ( ( flags & ( CERTDB_VALID_CA | CERTDB_TRUSTED_CA ) ) ==
		( CERTDB_VALID_CA | CERTDB_TRUSTED_CA ) ) {
		goto winner;
	    }
	    break;
	  case certUsageAnyCA:
	  case certUsageProtectedObjectSigner:
	  case certUsageUserCertImport:
	    /* XXX to make the compiler happy.  Should these be
	     * explicitly handled?
	     */
	    break;
	}
    }

    rv = CERT_VerifyCertChain(handle, cert, checkSig, certUsage,
			      t, wincx, log);
    if (rv != SECSuccess) {
	EXIT_IF_NOT_LOGGING(log);
    }

    /*
     * Check revocation status, but only if the cert we are checking
     * is not a status reponder itself.  We only do this in the case
     * where we checked the cert chain (above); explicit trust "wins"
     * (avoids status checking, just as it avoids CRL checking, which
     * is all done inside VerifyCertChain) by bypassing this code.
     */
    statusConfig = CERT_GetStatusConfig(handle);
    if (certUsage != certUsageStatusResponder && statusConfig != NULL) {
	if (statusConfig->statusChecker != NULL) {
	    rv = (* statusConfig->statusChecker)(handle, cert,
							 t, wincx);
	    if (rv != SECSuccess) {
		LOG_ERROR_OR_EXIT(log,cert,0,0);
	    }
	}
    }

winner:
    return(SECSuccess);

loser:
    rv = SECFailure;
    
    return(rv);
}

/*
 * verify a certificate by checking if its valid and that we
 * trust the issuer.  Verify time against now.
 */
SECStatus
CERT_VerifyCertificateNow(CERTCertDBHandle *handle, CERTCertificate *cert,
		   PRBool checkSig, SECCertificateUsage requiredUsages,
                   void *wincx, SECCertificateUsage* returnedUsages)
{
    return(CERT_VerifyCertificate(handle, cert, checkSig, 
		   requiredUsages, PR_Now(), wincx, NULL, returnedUsages));
}

/* obsolete, do not use for new code */
SECStatus
CERT_VerifyCertNow(CERTCertDBHandle *handle, CERTCertificate *cert,
		   PRBool checkSig, SECCertUsage certUsage, void *wincx)
{
    return(CERT_VerifyCert(handle, cert, checkSig, 
		   certUsage, PR_Now(), wincx, NULL));
}


/* [ FROM pcertdb.c ] */
/*
 * Supported usage values and types:
 *	certUsageSSLClient
 *	certUsageSSLServer
 *	certUsageSSLServerWithStepUp
 *	certUsageEmailSigner
 *	certUsageEmailRecipient
 *	certUsageObjectSigner
 */

CERTCertificate *
CERT_FindMatchingCert(CERTCertDBHandle *handle, SECItem *derName,
		      CERTCertOwner owner, SECCertUsage usage,
		      PRBool preferTrusted, int64 validTime, PRBool validOnly)
{
    CERTCertList *certList = NULL;
    CERTCertificate *cert = NULL;
    unsigned int requiredTrustFlags;
    SECTrustType requiredTrustType;
    unsigned int flags;
    
    PRBool lookingForCA = PR_FALSE;
    SECStatus rv;
    CERTCertListNode *node;
    CERTCertificate *saveUntrustedCA = NULL;
    
    /* if preferTrusted is set, must be a CA cert */
    PORT_Assert( ! ( preferTrusted && ( owner != certOwnerCA ) ) );
    
    if ( owner == certOwnerCA ) {
	lookingForCA = PR_TRUE;
	if ( preferTrusted ) {
	    rv = CERT_TrustFlagsForCACertUsage(usage, &requiredTrustFlags,
					       &requiredTrustType);
	    if ( rv != SECSuccess ) {
		goto loser;
	    }
	    requiredTrustFlags |= CERTDB_VALID_CA;
	}
    }

    certList = CERT_CreateSubjectCertList(NULL, handle, derName, validTime,
					  validOnly);
    if ( certList != NULL ) {
	rv = CERT_FilterCertListByUsage(certList, usage, lookingForCA);
	if ( rv != SECSuccess ) {
	    goto loser;
	}
	
	node = CERT_LIST_HEAD(certList);
	
	while ( !CERT_LIST_END(node, certList) ) {
	    cert = node->cert;

	    /* looking for a trusted CA cert */
	    if ( ( owner == certOwnerCA ) && preferTrusted &&
		( requiredTrustType != trustTypeNone ) ) {

		if ( cert->trust == NULL ) {
		    flags = 0;
		} else {
		    flags = SEC_GET_TRUST_FLAGS(cert->trust, requiredTrustType);
		}

		if ( ( flags & requiredTrustFlags ) != requiredTrustFlags ) {
		    /* cert is not trusted */
		    /* if this is the first cert to get this far, then save
		     * it, so we can use it if we can't find a trusted one
		     */
		    if ( saveUntrustedCA == NULL ) {
			saveUntrustedCA = cert;
		    }
		    goto endloop;
		}
	    }
	    /* if we got this far, then this cert meets all criteria */
	    break;
	    
endloop:
	    node = CERT_LIST_NEXT(node);
	    cert = NULL;
	}

	/* use the saved one if we have it */
	if ( cert == NULL ) {
	    cert = saveUntrustedCA;
	}

	/* if we found one then bump the ref count before freeing the list */
	if ( cert != NULL ) {
	    /* bump the ref count */
	    cert = CERT_DupCertificate(cert);
	}
	
	CERT_DestroyCertList(certList);
    }

    return(cert);

loser:
    if ( certList != NULL ) {
	CERT_DestroyCertList(certList);
    }

    return(NULL);
}


/* [ From certdb.c ] */
/*
 * Filter a list of certificates, removing those certs that do not have
 * one of the named CA certs somewhere in their cert chain.
 *
 *	"certList" - the list of certificates to filter
 *	"nCANames" - number of CA names
 *	"caNames" - array of CA names in string(rfc 1485) form
 *	"usage" - what use the certs are for, this is used when
 *		selecting CA certs
 */
SECStatus
CERT_FilterCertListByCANames(CERTCertList *certList, int nCANames,
			     char **caNames, SECCertUsage usage)
{
    CERTCertificate *issuerCert = NULL;
    CERTCertificate *subjectCert;
    CERTCertListNode *node, *freenode;
    CERTCertificate *cert;
    int n;
    char **names;
    PRBool found;
    int64 time;
    
    if ( nCANames <= 0 ) {
	return(SECSuccess);
    }

    time = PR_Now();
    
    node = CERT_LIST_HEAD(certList);
    
    while ( ! CERT_LIST_END(node, certList) ) {
	cert = node->cert;
	
	subjectCert = CERT_DupCertificate(cert);

	/* traverse the CA certs for this cert */
	found = PR_FALSE;
	while ( subjectCert != NULL ) {
	    n = nCANames;
	    names = caNames;
	   
            if (subjectCert->issuerName != NULL) { 
	        while ( n > 0 ) {
		    if ( PORT_Strcmp(*names, subjectCert->issuerName) == 0 ) {
		        found = PR_TRUE;
		        break;
		    }

		    n--;
		    names++;
                }
	    }

	    if ( found ) {
		break;
	    }
	    
	    issuerCert = CERT_FindCertIssuer(subjectCert, time, usage);
	    if ( issuerCert == subjectCert ) {
		CERT_DestroyCertificate(issuerCert);
		issuerCert = NULL;
		break;
	    }
	    CERT_DestroyCertificate(subjectCert);
	    subjectCert = issuerCert;

	}
	CERT_DestroyCertificate(subjectCert);
	if ( !found ) {
	    /* CA was not found, so remove this cert from the list */
	    freenode = node;
	    node = CERT_LIST_NEXT(node);
	    CERT_RemoveCertListNode(freenode);
	} else {
	    /* CA was found, so leave it in the list */
	    node = CERT_LIST_NEXT(node);
	}
    }
    
    return(SECSuccess);
}

/*
 * Given a certificate, return a string containing the nickname, and possibly
 * one of the validity strings, based on the current validity state of the
 * certificate.
 *
 * "arena" - arena to allocate returned string from.  If NULL, then heap
 *	is used.
 * "cert" - the cert to get nickname from
 * "expiredString" - the string to append to the nickname if the cert is
 *		expired.
 * "notYetGoodString" - the string to append to the nickname if the cert is
 *		not yet good.
 */
char *
CERT_GetCertNicknameWithValidity(PRArenaPool *arena, CERTCertificate *cert,
				 char *expiredString, char *notYetGoodString)
{
    SECCertTimeValidity validity;
    char *nickname = NULL, *tmpstr = NULL;
    
    validity = CERT_CheckCertValidTimes(cert, PR_Now(), PR_FALSE);

    /* if the cert is good, then just use the nickname directly */
    if ( validity == secCertTimeValid ) {
	if ( arena == NULL ) {
	    nickname = PORT_Strdup(cert->nickname);
	} else {
	    nickname = PORT_ArenaStrdup(arena, cert->nickname);
	}
	
	if ( nickname == NULL ) {
	    goto loser;
	}
    } else {
	    
	/* if the cert is not valid, then tack one of the strings on the
	 * end
	 */
	if ( validity == secCertTimeExpired ) {
	    tmpstr = PR_smprintf("%s%s", cert->nickname,
				 expiredString);
	} else if ( validity == secCertTimeNotValidYet ) {
	    /* not yet valid */
	    tmpstr = PR_smprintf("%s%s", cert->nickname,
				 notYetGoodString);
        } else {
            /* undetermined */
	    tmpstr = PR_smprintf("%s",
                        "(NULL) (Validity Unknown)");
        }

	if ( tmpstr == NULL ) {
	    goto loser;
	}

	if ( arena ) {
	    /* copy the string into the arena and free the malloc'd one */
	    nickname = PORT_ArenaStrdup(arena, tmpstr);
	    PORT_Free(tmpstr);
	} else {
	    nickname = tmpstr;
	}
	if ( nickname == NULL ) {
	    goto loser;
	}
    }    
    return(nickname);

loser:
    return(NULL);
}

/*
 * Collect the nicknames from all certs in a CertList.  If the cert is not
 * valid, append a string to that nickname.
 *
 * "certList" - the list of certificates
 * "expiredString" - the string to append to the nickname of any expired cert
 * "notYetGoodString" - the string to append to the nickname of any cert
 *		that is not yet valid
 */
CERTCertNicknames *
CERT_NicknameStringsFromCertList(CERTCertList *certList, char *expiredString,
				 char *notYetGoodString)
{
    CERTCertNicknames *names;
    PRArenaPool *arena;
    CERTCertListNode *node;
    char **nn;
    
    /* allocate an arena */
    arena = PORT_NewArena(DER_DEFAULT_CHUNKSIZE);
    if ( arena == NULL ) {
	return(NULL);
    }
    
    /* allocate the structure */
    names = PORT_ArenaAlloc(arena, sizeof(CERTCertNicknames));
    if ( names == NULL ) {
	goto loser;
    }

    /* init the structure */
    names->arena = arena;
    names->head = NULL;
    names->numnicknames = 0;
    names->nicknames = NULL;
    names->totallen = 0;

    /* count the certs in the list */
    node = CERT_LIST_HEAD(certList);
    while ( ! CERT_LIST_END(node, certList) ) {
	names->numnicknames++;
	node = CERT_LIST_NEXT(node);
    }
    
    /* allocate nicknames array */
    names->nicknames = PORT_ArenaAlloc(arena,
				       sizeof(char *) * names->numnicknames);
    if ( names->nicknames == NULL ) {
	goto loser;
    }

    /* just in case printf can't deal with null strings */
    if (expiredString == NULL ) {
	expiredString = "";
    }

    if ( notYetGoodString == NULL ) {
	notYetGoodString = "";
    }
    
    /* traverse the list of certs and collect the nicknames */
    nn = names->nicknames;
    node = CERT_LIST_HEAD(certList);
    while ( ! CERT_LIST_END(node, certList) ) {
	*nn = CERT_GetCertNicknameWithValidity(arena, node->cert,
					       expiredString,
					       notYetGoodString);
	if ( *nn == NULL ) {
	    goto loser;
	}

	names->totallen += PORT_Strlen(*nn);
	
	nn++;
	node = CERT_LIST_NEXT(node);
    }

    return(names);

loser:
    PORT_FreeArena(arena, PR_FALSE);
    return(NULL);
}

/*
 * Extract the nickname from a nickmake string that may have either
 * expiredString or notYetGoodString appended.
 *
 * Args:
 *	"namestring" - the string containing the nickname, and possibly
 *		one of the validity label strings
 *	"expiredString" - the expired validity label string
 *	"notYetGoodString" - the not yet good validity label string
 *
 * Returns the raw nickname
 */
char *
CERT_ExtractNicknameString(char *namestring, char *expiredString,
			   char *notYetGoodString)
{
    int explen, nyglen, namelen;
    int retlen;
    char *retstr;
    
    namelen = PORT_Strlen(namestring);
    explen = PORT_Strlen(expiredString);
    nyglen = PORT_Strlen(notYetGoodString);
    
    if ( namelen > explen ) {
	if ( PORT_Strcmp(expiredString, &namestring[namelen-explen]) == 0 ) {
	    retlen = namelen - explen;
	    retstr = (char *)PORT_Alloc(retlen+1);
	    if ( retstr == NULL ) {
		goto loser;
	    }
	    
	    PORT_Memcpy(retstr, namestring, retlen);
	    retstr[retlen] = '\0';
	    goto done;
	}
    }

    if ( namelen > nyglen ) {
	if ( PORT_Strcmp(notYetGoodString, &namestring[namelen-nyglen]) == 0) {
	    retlen = namelen - nyglen;
	    retstr = (char *)PORT_Alloc(retlen+1);
	    if ( retstr == NULL ) {
		goto loser;
	    }
	    
	    PORT_Memcpy(retstr, namestring, retlen);
	    retstr[retlen] = '\0';
	    goto done;
	}
    }

    /* if name string is shorter than either invalid string, then it must
     * be a raw nickname
     */
    retstr = PORT_Strdup(namestring);
    
done:
    return(retstr);

loser:
    return(NULL);
}

CERTCertList *
CERT_GetCertChainFromCert(CERTCertificate *cert, int64 time, SECCertUsage usage)
{
    CERTCertList *chain = NULL;

    if (NULL == cert) {
        return NULL;
    }
    
    cert = CERT_DupCertificate(cert);
    if (NULL == cert) {
        PORT_SetError(SEC_ERROR_NO_MEMORY);
        return NULL;
    }

    chain = CERT_NewCertList();
    if (NULL == chain) {
        PORT_SetError(SEC_ERROR_NO_MEMORY);
        return NULL;
    }

    while (cert != NULL) {
	if (SECSuccess != CERT_AddCertToListTail(chain, cert)) {
            /* return partial chain */
            PORT_SetError(SEC_ERROR_NO_MEMORY);
            return chain;
        }

	if (SECITEM_CompareItem(&cert->derIssuer, &cert->derSubject)
	    == SECEqual) {
            /* return complete chain */
	    return chain;
	}

	cert = CERT_FindCertIssuer(cert, time, usage);
    }

    /* return partial chain */
    PORT_SetError(SEC_ERROR_UNKNOWN_ISSUER);
    return chain;
}
