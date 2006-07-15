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

#include "nssrenam.h"
#include "cert.h"
#include "secpkcs7.h"
#include "base64.h"
#include "secitem.h"
#include "secder.h"
#include "secasn1.h"
#include "secoid.h"
#include "secerr.h"

SEC_ASN1_MKSUB(SEC_AnyTemplate)

SECStatus
SEC_ReadPKCS7Certs(SECItem *pkcs7Item, CERTImportCertificateFunc f, void *arg)
{
    SEC_PKCS7ContentInfo *contentInfo = NULL;
    SECStatus rv;
    SECItem **certs;
    int count;

    contentInfo = SEC_PKCS7DecodeItem(pkcs7Item, NULL, NULL, NULL, NULL, NULL, 
				      NULL, NULL);
    if ( contentInfo == NULL ) {
	goto loser;
    }

    if ( SEC_PKCS7ContentType (contentInfo) != SEC_OID_PKCS7_SIGNED_DATA ) {
	goto loser;
    }

    certs = contentInfo->content.signedData->rawCerts;
    if ( certs ) {
	count = 0;
	
	while ( *certs ) {
	    count++;
	    certs++;
	}
	rv = (* f)(arg, contentInfo->content.signedData->rawCerts, count);
    }
    
    rv = SECSuccess;
    
    goto done;
loser:
    rv = SECFailure;
    
done:
    if ( contentInfo ) {
	SEC_PKCS7DestroyContentInfo(contentInfo);
    }

    return(rv);
}

const SEC_ASN1Template SEC_CertSequenceTemplate[] = {
    { SEC_ASN1_SEQUENCE_OF | SEC_ASN1_XTRN, 0, SEC_ASN1_SUB(SEC_AnyTemplate) }
};

SECStatus
SEC_ReadCertSequence(SECItem *certsItem, CERTImportCertificateFunc f, void *arg)
{
    SECStatus rv;
    SECItem **certs;
    int count;
    SECItem **rawCerts = NULL;
    PRArenaPool *arena;
    SEC_PKCS7ContentInfo *contentInfo = NULL;

    arena = PORT_NewArena(DER_DEFAULT_CHUNKSIZE);
    if (arena == NULL) {
	return SECFailure;
    }

    contentInfo = SEC_PKCS7DecodeItem(certsItem, NULL, NULL, NULL, NULL, NULL, 
				      NULL, NULL);
    if ( contentInfo == NULL ) {
	goto loser;
    }

    if ( SEC_PKCS7ContentType (contentInfo) != SEC_OID_NS_TYPE_CERT_SEQUENCE ) {
	goto loser;
    }


    rv = SEC_QuickDERDecodeItem(arena, &rawCerts, SEC_CertSequenceTemplate,
		    contentInfo->content.data);

    if (rv != SECSuccess) {
	goto loser;
    }

    certs = rawCerts;
    if ( certs ) {
	count = 0;
	
	while ( *certs ) {
	    count++;
	    certs++;
	}
	rv = (* f)(arg, rawCerts, count);
    }
    
    rv = SECSuccess;
    
    goto done;
loser:
    rv = SECFailure;
    
done:
    if ( contentInfo ) {
	SEC_PKCS7DestroyContentInfo(contentInfo);
    }

    if ( arena ) {
	PORT_FreeArena(arena, PR_FALSE);
    }
    
    return(rv);
}

CERTCertificate *
CERT_ConvertAndDecodeCertificate(char *certstr)
{
    CERTCertificate *cert;
    SECStatus rv;
    SECItem der;

    rv = ATOB_ConvertAsciiToItem(&der, certstr);
    if (rv != SECSuccess)
	return NULL;

    cert = CERT_NewTempCertificate(CERT_GetDefaultCertDB(), 
                                   &der, NULL, PR_FALSE, PR_TRUE);

    PORT_Free(der.data);
    return cert;
}

#define NS_CERT_HEADER "-----BEGIN CERTIFICATE-----"
#define NS_CERT_TRAILER "-----END CERTIFICATE-----"

#define CERTIFICATE_TYPE_STRING "certificate"
#define CERTIFICATE_TYPE_LEN (sizeof(CERTIFICATE_TYPE_STRING)-1)

CERTPackageType
CERT_CertPackageType(SECItem *package, SECItem *certitem)
{
    unsigned char *cp;
    unsigned int seqLen, seqLenLen;
    SECItem oiditem;
    SECOidData *oiddata;
    CERTPackageType type = certPackageNone;
    
    cp = package->data;

    /* is a DER encoded certificate of some type? */
    if ( ( *cp  & 0x1f ) == SEC_ASN1_SEQUENCE ) {
	cp++;
	
	if ( *cp & 0x80) {
	    /* Multibyte length */
	    seqLenLen = cp[0] & 0x7f;
	    
	    switch (seqLenLen) {
	      case 4:
		seqLen = ((unsigned long)cp[1]<<24) |
		    ((unsigned long)cp[2]<<16) | (cp[3]<<8) | cp[4];
		break;
	      case 3:
		seqLen = ((unsigned long)cp[1]<<16) | (cp[2]<<8) | cp[3];
		break;
	      case 2:
		seqLen = (cp[1]<<8) | cp[2];
		break;
	      case 1:
		seqLen = cp[1];
		break;
	      default:
		/* indefinite length */
		seqLen = 0;
	    }
	    cp += ( seqLenLen + 1 );

	} else {
	    seqLenLen = 0;
	    seqLen = *cp;
	    cp++;
	}

	/* check entire length if definite length */
	if ( seqLen || seqLenLen ) {
	    if ( package->len != ( seqLen + seqLenLen + 2 ) ) {
		/* not a DER package */
		return(type);
	    }
	}
	
	/* check the type string */
	/* netscape wrapped DER cert */
	if ( ( cp[0] == SEC_ASN1_OCTET_STRING ) &&
	    ( cp[1] == CERTIFICATE_TYPE_LEN ) &&
	    ( PORT_Strcmp((char *)&cp[2], CERTIFICATE_TYPE_STRING) ) ) {
	    
	    cp += ( CERTIFICATE_TYPE_LEN + 2 );

	    /* it had better be a certificate by now!! */
	    if ( certitem ) {
		certitem->data = cp;
		certitem->len = package->len -
		    ( cp - (unsigned char *)package->data );
	    }
	    type = certPackageNSCertWrap;
	    
	} else if ( cp[0] == SEC_ASN1_OBJECT_ID ) {
	    /* XXX - assume DER encoding of OID len!! */
	    oiditem.len = cp[1];
	    oiditem.data = (unsigned char *)&cp[2];
	    oiddata = SECOID_FindOID(&oiditem);
	    if ( oiddata == NULL ) {
		/* failure */
		return(type);
	    }

	    if ( certitem ) {
		certitem->data = package->data;
		certitem->len = package->len;
	    }
	    
	    switch ( oiddata->offset ) {
	      case SEC_OID_PKCS7_SIGNED_DATA:
		type = certPackagePKCS7;
		break;
	      case SEC_OID_NS_TYPE_CERT_SEQUENCE:
		type = certPackageNSCertSeq;
		break;
	      default:
		break;
	    }
	    
	} else {
	    /* it had better be a certificate by now!! */
	    if ( certitem ) {
		certitem->data = package->data;
		certitem->len = package->len;
	    }
	    
	    type = certPackageCert;
	}
    }

    return(type);
}

/*
 * read an old style ascii or binary certificate chain
 */
SECStatus
CERT_DecodeCertPackage(char *certbuf,
		       int certlen,
		       CERTImportCertificateFunc f,
		       void *arg)
{
    unsigned char *cp;
    unsigned char *bincert = NULL;
    char *         ascCert = NULL;
    SECStatus      rv;
    
    if ( certbuf == NULL ) {
	return(SECFailure);
    }
    
    cp = (unsigned char *)certbuf;

    /* is a DER encoded certificate of some type? */
    if ( ( *cp  & 0x1f ) == SEC_ASN1_SEQUENCE ) {
	SECItem certitem;
	SECItem *pcertitem = &certitem;
	int seqLen, seqLenLen;

	cp++;
	
	if ( *cp & 0x80) {
	    /* Multibyte length */
	    seqLenLen = cp[0] & 0x7f;
	    
	    switch (seqLenLen) {
	      case 4:
		seqLen = ((unsigned long)cp[1]<<24) |
		    ((unsigned long)cp[2]<<16) | (cp[3]<<8) | cp[4];
		break;
	      case 3:
		seqLen = ((unsigned long)cp[1]<<16) | (cp[2]<<8) | cp[3];
		break;
	      case 2:
		seqLen = (cp[1]<<8) | cp[2];
		break;
	      case 1:
		seqLen = cp[1];
		break;
	      default:
		/* indefinite length */
		seqLen = 0;
	    }
	    cp += ( seqLenLen + 1 );

	} else {
	    seqLenLen = 0;
	    seqLen = *cp;
	    cp++;
	}

	/* check entire length if definite length */
	if ( seqLen || seqLenLen ) {
	    if ( certlen != ( seqLen + seqLenLen + 2 ) ) {
		if (certlen > ( seqLen + seqLenLen + 2 ))
		    PORT_SetError(SEC_ERROR_EXTRA_INPUT);
		else 
		    PORT_SetError(SEC_ERROR_INPUT_LEN);
		goto notder;
	    }
	}
	
	/* check the type string */
	/* netscape wrapped DER cert */
	if ( ( cp[0] == SEC_ASN1_OCTET_STRING ) &&
	    ( cp[1] == CERTIFICATE_TYPE_LEN ) &&
	    ( PORT_Strcmp((char *)&cp[2], CERTIFICATE_TYPE_STRING) ) ) {
	    
	    cp += ( CERTIFICATE_TYPE_LEN + 2 );

	    /* it had better be a certificate by now!! */
	    certitem.data = cp;
	    certitem.len = certlen - ( cp - (unsigned char *)certbuf );
	    
	    rv = (* f)(arg, &pcertitem, 1);
	    
	    return(rv);
	} else if ( cp[0] == SEC_ASN1_OBJECT_ID ) {
	    SECOidData *oiddata;
	    SECItem oiditem;
	    /* XXX - assume DER encoding of OID len!! */
	    oiditem.len = cp[1];
	    oiditem.data = (unsigned char *)&cp[2];
	    oiddata = SECOID_FindOID(&oiditem);
	    if ( oiddata == NULL ) {
		return(SECFailure);
	    }

	    certitem.data = (unsigned char*)certbuf;
	    certitem.len = certlen;
	    
	    switch ( oiddata->offset ) {
	      case SEC_OID_PKCS7_SIGNED_DATA:
		return(SEC_ReadPKCS7Certs(&certitem, f, arg));
		break;
	      case SEC_OID_NS_TYPE_CERT_SEQUENCE:
		return(SEC_ReadCertSequence(&certitem, f, arg));
		break;
	      default:
		break;
	    }
	    
	} else {
	    /* it had better be a certificate by now!! */
	    certitem.data = (unsigned char*)certbuf;
	    certitem.len = certlen;
	    
	    rv = (* f)(arg, &pcertitem, 1);
	    return(rv);
	}
    }

    /* now look for a netscape base64 ascii encoded cert */
notder:
  {
    unsigned char *certbegin = NULL; 
    unsigned char *certend   = NULL;
    char          *pc;
    int cl;

    /* Convert the ASCII data into a nul-terminated string */
    ascCert = (char *)PORT_Alloc(certlen + 1);
    if (!ascCert) {
        rv = SECFailure;
	goto loser;
    }

    PORT_Memcpy(ascCert, certbuf, certlen);
    ascCert[certlen] = '\0';

    pc = PORT_Strchr(ascCert, '\n');  /* find an EOL */
    if (!pc) { /* maybe this is a MAC file */
	pc = ascCert;
	while (*pc && NULL != (pc = PORT_Strchr(pc, '\r'))) {
	    *pc++ = '\n';
	}
    }

    cp = (unsigned char *)ascCert;
    cl = certlen;

    /* find the beginning marker */
    while ( cl > sizeof(NS_CERT_HEADER) ) {
	if ( !PORT_Strncasecmp((char *)cp, NS_CERT_HEADER,
			     sizeof(NS_CERT_HEADER)-1) ) {
	    cp = cp + sizeof(NS_CERT_HEADER);
	    certbegin = cp;
	    break;
	}
	
	/* skip to next eol */
	do {
	    cp++;
	    cl--;
	} while ( ( *cp != '\n') && cl );

	/* skip all blank lines */
	while ( ( *cp == '\n') && cl ) {
	    cp++;
	    cl--;
	}
    }

    if ( certbegin ) {
	/* find the ending marker */
	while ( cl > sizeof(NS_CERT_TRAILER) ) {
	    if ( !PORT_Strncasecmp((char *)cp, NS_CERT_TRAILER,
				 sizeof(NS_CERT_TRAILER)-1) ) {
		certend = (unsigned char *)cp;
		break;
	    }

	    /* skip to next eol */
	    do {
		cp++;
		cl--;
	    } while ( ( *cp != '\n') && cl );

	    /* skip all blank lines */
	    while ( ( *cp == '\n') && cl ) {
		cp++;
		cl--;
	    }
	}
    }

    if ( certbegin && certend ) {
	unsigned int binLen;

	*certend = 0;
	/* convert to binary */
	bincert = ATOB_AsciiToData(certbegin, &binLen);
	if (!bincert) {
	    rv = SECFailure;
	    goto loser;
	}

	/* now recurse to decode the binary */
	rv = CERT_DecodeCertPackage((char *)bincert, binLen, f, arg);
	
    } else {
	rv = SECFailure;
    }
  }

loser:

    if ( bincert ) {
	PORT_Free(bincert);
    }

    if ( ascCert ) {
	PORT_Free(ascCert);
    }

    return(rv);
}

typedef struct {
    PRArenaPool *arena;
    SECItem cert;
} collect_args;

static SECStatus
collect_certs(void *arg, SECItem **certs, int numcerts)
{
    SECStatus rv;
    collect_args *collectArgs;
    
    collectArgs = (collect_args *)arg;
    
    rv = SECITEM_CopyItem(collectArgs->arena, &collectArgs->cert, *certs);

    return(rv);
}


/*
 * read an old style ascii or binary certificate
 */
CERTCertificate *
CERT_DecodeCertFromPackage(char *certbuf, int certlen)
{
    collect_args collectArgs;
    SECStatus rv;
    CERTCertificate *cert = NULL;
    
    collectArgs.arena = PORT_NewArena(DER_DEFAULT_CHUNKSIZE);
    
    rv = CERT_DecodeCertPackage(certbuf, certlen, collect_certs,
				(void *)&collectArgs);
    if ( rv == SECSuccess ) {
	cert = CERT_NewTempCertificate(CERT_GetDefaultCertDB(),
	                               &collectArgs.cert, NULL, 
	                               PR_FALSE, PR_TRUE);
    }
    
    PORT_FreeArena(collectArgs.arena, PR_FALSE);
    
    return(cert);
}
