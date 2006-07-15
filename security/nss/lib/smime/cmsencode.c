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
 * CMS encoding.
 *
 * $Id$
 */

#include "cmslocal.h"

#include "cert.h"
#include "key.h"
#include "secasn1.h"
#include "secoid.h"
#include "secitem.h"
#include "pk11func.h"
#include "secerr.h"

struct nss_cms_encoder_output {
    NSSCMSContentCallback outputfn;
    void *outputarg;
    PLArenaPool *destpoolp;
    SECItem *dest;
};

struct NSSCMSEncoderContextStr {
    SEC_ASN1EncoderContext *	ecx;		/* ASN.1 encoder context */
    PRBool			ecxupdated;	/* true if data was handed in */
    NSSCMSMessage *		cmsg;		/* pointer to the root message */
    SECOidTag			type;		/* type tag of the current content */
    NSSCMSContent		content;	/* pointer to current content */
    struct nss_cms_encoder_output output;	/* output function */
    int				error;		/* error code */
    NSSCMSEncoderContext *	childp7ecx;	/* link to child encoder context */
};

static SECStatus nss_cms_before_data(NSSCMSEncoderContext *p7ecx);
static SECStatus nss_cms_after_data(NSSCMSEncoderContext *p7ecx);
static SECStatus nss_cms_encoder_update(NSSCMSEncoderContext *p7ecx, const char *data, unsigned long len);
static SECStatus nss_cms_encoder_work_data(NSSCMSEncoderContext *p7ecx, SECItem *dest,
			     const unsigned char *data, unsigned long len,
			     PRBool final, PRBool innermost);

extern const SEC_ASN1Template NSSCMSMessageTemplate[];

/*
 * The little output function that the ASN.1 encoder calls to hand
 * us bytes which we in turn hand back to our caller (via the callback
 * they gave us).
 */
static void
nss_cms_encoder_out(void *arg, const char *buf, unsigned long len,
		      int depth, SEC_ASN1EncodingPart data_kind)
{
    struct nss_cms_encoder_output *output = (struct nss_cms_encoder_output *)arg;
    unsigned char *dest;
    unsigned long offset;

#ifdef CMSDEBUG
    int i;

    fprintf(stderr, "kind = %d, depth = %d, len = %d\n", data_kind, depth, len);
    for (i=0; i < len; i++) {
	fprintf(stderr, " %02x%s", (unsigned int)buf[i] & 0xff, ((i % 16) == 15) ? "\n" : "");
    }
    if ((i % 16) != 0)
	fprintf(stderr, "\n");
#endif

    if (output->outputfn != NULL)
	/* call output callback with DER data */
	output->outputfn(output->outputarg, buf, len);

    if (output->dest != NULL) {
	/* store DER data in SECItem */
	offset = output->dest->len;
	if (offset == 0) {
	    dest = (unsigned char *)PORT_ArenaAlloc(output->destpoolp, len);
	} else {
	    dest = (unsigned char *)PORT_ArenaGrow(output->destpoolp, 
				  output->dest->data,
				  output->dest->len,
				  output->dest->len + len);
	}
	if (dest == NULL)
	    /* oops */
	    return;

	output->dest->data = dest;
	output->dest->len += len;

	/* copy it in */
	PORT_Memcpy(output->dest->data + offset, buf, len);
    }
}

/*
 * nss_cms_encoder_notify - ASN.1 encoder callback
 *
 * this function is called by the ASN.1 encoder before and after the encoding of
 * every object. here, it is used to keep track of data structures, set up
 * encryption and/or digesting and possibly set up child encoders.
 */
static void
nss_cms_encoder_notify(void *arg, PRBool before, void *dest, int depth)
{
    NSSCMSEncoderContext *p7ecx;
    NSSCMSContentInfo *rootcinfo, *cinfo;
    PRBool after = !before;
    PLArenaPool *poolp;
    SECOidTag childtype;
    SECItem *item;

    p7ecx = (NSSCMSEncoderContext *)arg;
    PORT_Assert(p7ecx != NULL);

    rootcinfo = &(p7ecx->cmsg->contentInfo);
    poolp = p7ecx->cmsg->poolp;

#ifdef CMSDEBUG
    fprintf(stderr, "%6.6s, dest = 0x%08x, depth = %d\n", before ? "before" : "after", dest, depth);
#endif

    /*
     * Watch for the content field, at which point we want to instruct
     * the ASN.1 encoder to start taking bytes from the buffer.
     */
    switch (p7ecx->type) {
    default:
    case SEC_OID_UNKNOWN:
	/* we're still in the root message */
	if (after && dest == &(rootcinfo->contentType)) {
	    /* got the content type OID now - so find out the type tag */
	    p7ecx->type = NSS_CMSContentInfo_GetContentTypeTag(rootcinfo);
	    /* set up a pointer to our current content */
	    p7ecx->content = rootcinfo->content;
	}
	break;

    case SEC_OID_PKCS7_DATA:
	if (before && dest == &(rootcinfo->rawContent)) {
	    /* just set up encoder to grab from user - no encryption or digesting */
	    if ((item = rootcinfo->content.data) != NULL)
		(void)nss_cms_encoder_work_data(p7ecx, NULL, item->data, item->len, PR_TRUE, PR_TRUE);
	    else
		SEC_ASN1EncoderSetTakeFromBuf(p7ecx->ecx);
	    SEC_ASN1EncoderClearNotifyProc(p7ecx->ecx);	/* no need to get notified anymore */
	}
	break;

    case SEC_OID_PKCS7_SIGNED_DATA:
    case SEC_OID_PKCS7_ENVELOPED_DATA:
    case SEC_OID_PKCS7_DIGESTED_DATA:
    case SEC_OID_PKCS7_ENCRYPTED_DATA:

	/* when we know what the content is, we encode happily until we reach the inner content */
	cinfo = NSS_CMSContent_GetContentInfo(p7ecx->content.pointer, p7ecx->type);
	childtype = NSS_CMSContentInfo_GetContentTypeTag(cinfo);

	if (after && dest == &(cinfo->contentType)) {
	    /* we're right before encoding the data (if we have some or not) */
	    /* (for encrypted data, we're right before the contentEncAlg which may change */
	    /*  in nss_cms_before_data because of IV calculation when setting up encryption) */
	    if (nss_cms_before_data(p7ecx) != SECSuccess)
		p7ecx->error = PORT_GetError();
	}
	if (before && dest == &(cinfo->rawContent)) {
	    if (childtype == SEC_OID_PKCS7_DATA && (item = cinfo->content.data) != NULL)
		/* we have data - feed it in */
		(void)nss_cms_encoder_work_data(p7ecx, NULL, item->data, item->len, PR_TRUE, PR_TRUE);
	    else
		/* else try to get it from user */
		SEC_ASN1EncoderSetTakeFromBuf(p7ecx->ecx);
	}
	if (after && dest == &(cinfo->rawContent)) {
	    if (nss_cms_after_data(p7ecx) != SECSuccess)
		p7ecx->error = PORT_GetError();
	    SEC_ASN1EncoderClearNotifyProc(p7ecx->ecx);	/* no need to get notified anymore */
	}
	break;
    }
}

/*
 * nss_cms_before_data - setup the current encoder to receive data
 */
static SECStatus
nss_cms_before_data(NSSCMSEncoderContext *p7ecx)
{
    SECStatus rv;
    SECOidTag childtype;
    NSSCMSContentInfo *cinfo;
    PLArenaPool *poolp;
    NSSCMSEncoderContext *childp7ecx;
    const SEC_ASN1Template *template;

    poolp = p7ecx->cmsg->poolp;

    /* call _Encode_BeforeData handlers */
    switch (p7ecx->type) {
    case SEC_OID_PKCS7_SIGNED_DATA:
	/* we're encoding a signedData, so set up the digests */
	rv = NSS_CMSSignedData_Encode_BeforeData(p7ecx->content.signedData);
	break;
    case SEC_OID_PKCS7_DIGESTED_DATA:
	/* we're encoding a digestedData, so set up the digest */
	rv = NSS_CMSDigestedData_Encode_BeforeData(p7ecx->content.digestedData);
	break;
    case SEC_OID_PKCS7_ENVELOPED_DATA:
	rv = NSS_CMSEnvelopedData_Encode_BeforeData(p7ecx->content.envelopedData);
	break;
    case SEC_OID_PKCS7_ENCRYPTED_DATA:
	rv = NSS_CMSEncryptedData_Encode_BeforeData(p7ecx->content.encryptedData);
	break;
    default:
	rv = SECFailure;
    }
    if (rv != SECSuccess)
	return SECFailure;

    /* ok, now we have a pointer to cinfo */
    /* find out what kind of data is encapsulated */
    
    cinfo = NSS_CMSContent_GetContentInfo(p7ecx->content.pointer, p7ecx->type);
    childtype = NSS_CMSContentInfo_GetContentTypeTag(cinfo);

    switch (childtype) {
    case SEC_OID_PKCS7_SIGNED_DATA:
    case SEC_OID_PKCS7_ENVELOPED_DATA:
    case SEC_OID_PKCS7_ENCRYPTED_DATA:
    case SEC_OID_PKCS7_DIGESTED_DATA:
#if 0
    case SEC_OID_PKCS7_DATA:		/* XXX here also??? maybe yes! */
#endif
	/* in these cases, we need to set up a child encoder! */
	/* create new encoder context */
	childp7ecx = PORT_ZAlloc(sizeof(NSSCMSEncoderContext));
	if (childp7ecx == NULL)
	    return SECFailure;

	/* the CHILD encoder needs to hand its encoded data to the CURRENT encoder
	 * (which will encrypt and/or digest it)
	 * this needs to route back into our update function
	 * which finds the lowest encoding context & encrypts and computes digests */
	childp7ecx->type = childtype;
	childp7ecx->content = cinfo->content;
	/* use the non-recursive update function here, of course */
	childp7ecx->output.outputfn = (NSSCMSContentCallback)nss_cms_encoder_update;
	childp7ecx->output.outputarg = p7ecx;
	childp7ecx->output.destpoolp = NULL;
	childp7ecx->output.dest = NULL;
	childp7ecx->cmsg = p7ecx->cmsg;

	template = NSS_CMSUtil_GetTemplateByTypeTag(childtype);
	if (template == NULL)
	    goto loser;		/* cannot happen */

	/* now initialize the data for encoding the first third */
	switch (childp7ecx->type) {
	case SEC_OID_PKCS7_SIGNED_DATA:
	    rv = NSS_CMSSignedData_Encode_BeforeStart(cinfo->content.signedData);
	    break;
	case SEC_OID_PKCS7_ENVELOPED_DATA:
	    rv = NSS_CMSEnvelopedData_Encode_BeforeStart(cinfo->content.envelopedData);
	    break;
	case SEC_OID_PKCS7_DIGESTED_DATA:
	    rv = NSS_CMSDigestedData_Encode_BeforeStart(cinfo->content.digestedData);
	    break;
	case SEC_OID_PKCS7_ENCRYPTED_DATA:
	    rv = NSS_CMSEncryptedData_Encode_BeforeStart(cinfo->content.encryptedData);
	    break;
	case SEC_OID_PKCS7_DATA:
	    rv = SECSuccess;
	    break;
	default:
	    PORT_Assert(0);
	    break;
	}
	if (rv != SECSuccess)
	    goto loser;

	/*
	 * Initialize the BER encoder.
	 */
	childp7ecx->ecx = SEC_ASN1EncoderStart(cinfo->content.pointer, template,
					   nss_cms_encoder_out, &(childp7ecx->output));
	if (childp7ecx->ecx == NULL)
	    goto loser;

	childp7ecx->ecxupdated = PR_FALSE;

	/*
	 * Indicate that we are streaming.  We will be streaming until we
	 * get past the contents bytes.
	 */
	SEC_ASN1EncoderSetStreaming(childp7ecx->ecx);

	/*
	 * The notify function will watch for the contents field.
	 */
	SEC_ASN1EncoderSetNotifyProc(childp7ecx->ecx, nss_cms_encoder_notify, childp7ecx);

	/* please note that we are NOT calling SEC_ASN1EncoderUpdate here to kick off the */
	/* encoding process - we'll do that from the update function instead */
	/* otherwise we'd be encoding data from a call of the notify function of the */
	/* parent encoder (which would not work) */

	/* this will kick off the encoding process & encode everything up to the content bytes,
	 * at which point the notify function sets streaming mode (and possibly creates
	 * another child encoder). */
	if (SEC_ASN1EncoderUpdate(childp7ecx->ecx, NULL, 0) != SECSuccess)
	    goto loser;

	p7ecx->childp7ecx = childp7ecx;
	break;

    case SEC_OID_PKCS7_DATA:
	p7ecx->childp7ecx = NULL;
	break;
    default:
	/* we do not know this type */
	p7ecx->error = SEC_ERROR_BAD_DER;
	break;
    }

    return SECSuccess;

loser:
    if (childp7ecx) {
	if (childp7ecx->ecx)
	    SEC_ASN1EncoderFinish(childp7ecx->ecx);
	PORT_Free(childp7ecx);
    }
    return SECFailure;
}

static SECStatus
nss_cms_after_data(NSSCMSEncoderContext *p7ecx)
{
    SECStatus rv = SECFailure;

    switch (p7ecx->type) {
    case SEC_OID_PKCS7_SIGNED_DATA:
	/* this will finish the digests and sign */
	rv = NSS_CMSSignedData_Encode_AfterData(p7ecx->content.signedData);
	break;
    case SEC_OID_PKCS7_ENVELOPED_DATA:
	rv = NSS_CMSEnvelopedData_Encode_AfterData(p7ecx->content.envelopedData);
	break;
    case SEC_OID_PKCS7_DIGESTED_DATA:
	rv = NSS_CMSDigestedData_Encode_AfterData(p7ecx->content.digestedData);
	break;
    case SEC_OID_PKCS7_ENCRYPTED_DATA:
	rv = NSS_CMSEncryptedData_Encode_AfterData(p7ecx->content.encryptedData);
	break;
    case SEC_OID_PKCS7_DATA:
	/* do nothing */
	break;
    default:
	rv = SECFailure;
	break;
    }
    return rv;
}

/*
 * nss_cms_encoder_work_data - process incoming data
 *
 * (from the user or the next encoding layer)
 * Here, we need to digest and/or encrypt, then pass it on
 */
static SECStatus
nss_cms_encoder_work_data(NSSCMSEncoderContext *p7ecx, SECItem *dest,
			     const unsigned char *data, unsigned long len,
			     PRBool final, PRBool innermost)
{
    unsigned char *buf = NULL;
    SECStatus rv;
    NSSCMSContentInfo *cinfo;

    rv = SECSuccess;		/* may as well be optimistic */

    /*
     * We should really have data to process, or we should be trying
     * to finish/flush the last block.  (This is an overly paranoid
     * check since all callers are in this file and simple inspection
     * proves they do it right.  But it could find a bug in future
     * modifications/development, that is why it is here.)
     */
    PORT_Assert ((data != NULL && len) || final);

    /* we got data (either from the caller, or from a lower level encoder) */
    cinfo = NSS_CMSContent_GetContentInfo(p7ecx->content.pointer, p7ecx->type);

    /* Update the running digest. */
    if (len && cinfo->digcx != NULL)
	NSS_CMSDigestContext_Update(cinfo->digcx, data, len);

    /* Encrypt this chunk. */
    if (cinfo->ciphcx != NULL) {
	unsigned int inlen;	/* length of data being encrypted */
	unsigned int outlen;	/* length of encrypted data */
	unsigned int buflen;	/* length available for encrypted data */

	inlen = len;
	buflen = NSS_CMSCipherContext_EncryptLength(cinfo->ciphcx, inlen, final);
	if (buflen == 0) {
	    /*
	     * No output is expected, but the input data may be buffered
	     * so we still have to call Encrypt.
	     */
	    rv = NSS_CMSCipherContext_Encrypt(cinfo->ciphcx, NULL, NULL, 0,
				   data, inlen, final);
	    if (final) {
		len = 0;
		goto done;
	    }
	    return rv;
	}

	if (dest != NULL)
	    buf = (unsigned char*)PORT_ArenaAlloc(p7ecx->cmsg->poolp, buflen);
	else
	    buf = (unsigned char*)PORT_Alloc(buflen);

	if (buf == NULL) {
	    rv = SECFailure;
	} else {
	    rv = NSS_CMSCipherContext_Encrypt(cinfo->ciphcx, buf, &outlen, buflen,
				   data, inlen, final);
	    data = buf;
	    len = outlen;
	}
	if (rv != SECSuccess)
	    /* encryption or malloc failed? */
	    return rv;
    }


    /*
     * at this point (data,len) has everything we'd like to give to the CURRENT encoder
     * (which will encode it, then hand it back to the user or the parent encoder)
     * We don't encode the data if we're innermost and we're told not to include the data
     */
    if (p7ecx->ecx != NULL && len && (!innermost || cinfo->rawContent != NULL))
	rv = SEC_ASN1EncoderUpdate(p7ecx->ecx, (const char *)data, len);

done:

    if (cinfo->ciphcx != NULL) {
	if (dest != NULL) {
	    dest->data = buf;
	    dest->len = len;
	} else if (buf != NULL) {
	    PORT_Free (buf);
	}
    }
    return rv;
}

/*
 * nss_cms_encoder_update - deliver encoded data to the next higher level
 *
 * no recursion here because we REALLY want to end up at the next higher encoder!
 */
static SECStatus
nss_cms_encoder_update(NSSCMSEncoderContext *p7ecx, const char *data, unsigned long len)
{
    /* XXX Error handling needs help.  Return what?  Do "Finish" on failure? */
    return nss_cms_encoder_work_data (p7ecx, NULL, (const unsigned char *)data, len, PR_FALSE, PR_FALSE);
}

/*
 * NSS_CMSEncoder_Start - set up encoding of a CMS message
 *
 * "cmsg" - message to encode
 * "outputfn", "outputarg" - callback function for delivery of DER-encoded output
 *                           will not be called if NULL.
 * "dest" - if non-NULL, pointer to SECItem that will hold the DER-encoded output
 * "destpoolp" - pool to allocate DER-encoded output in
 * "pwfn", pwfn_arg" - callback function for getting token password
 * "decrypt_key_cb", "decrypt_key_cb_arg" - callback function for getting bulk key for encryptedData
 * "detached_digestalgs", "detached_digests" - digests from detached content
 */
NSSCMSEncoderContext *
NSS_CMSEncoder_Start(NSSCMSMessage *cmsg,
			NSSCMSContentCallback outputfn, void *outputarg,
			SECItem *dest, PLArenaPool *destpoolp,
			PK11PasswordFunc pwfn, void *pwfn_arg,
			NSSCMSGetDecryptKeyCallback decrypt_key_cb, void *decrypt_key_cb_arg,
			SECAlgorithmID **detached_digestalgs, SECItem **detached_digests)
{
    NSSCMSEncoderContext *p7ecx;
    SECStatus rv;
    NSSCMSContentInfo *cinfo;

    NSS_CMSMessage_SetEncodingParams(cmsg, pwfn, pwfn_arg, decrypt_key_cb, decrypt_key_cb_arg,
					detached_digestalgs, detached_digests);

    p7ecx = (NSSCMSEncoderContext *)PORT_ZAlloc(sizeof(NSSCMSEncoderContext));
    if (p7ecx == NULL) {
	PORT_SetError(SEC_ERROR_NO_MEMORY);
	return NULL;
    }

    p7ecx->cmsg = cmsg;
    p7ecx->output.outputfn = outputfn;
    p7ecx->output.outputarg = outputarg;
    p7ecx->output.dest = dest;
    p7ecx->output.destpoolp = destpoolp;
    p7ecx->type = SEC_OID_UNKNOWN;

    cinfo = NSS_CMSMessage_GetContentInfo(cmsg);

    switch (NSS_CMSContentInfo_GetContentTypeTag(cinfo)) {
    case SEC_OID_PKCS7_SIGNED_DATA:
	rv = NSS_CMSSignedData_Encode_BeforeStart(cinfo->content.signedData);
	break;
    case SEC_OID_PKCS7_ENVELOPED_DATA:
	rv = NSS_CMSEnvelopedData_Encode_BeforeStart(cinfo->content.envelopedData);
	break;
    case SEC_OID_PKCS7_DIGESTED_DATA:
	rv = NSS_CMSDigestedData_Encode_BeforeStart(cinfo->content.digestedData);
	break;
    case SEC_OID_PKCS7_ENCRYPTED_DATA:
	rv = NSS_CMSEncryptedData_Encode_BeforeStart(cinfo->content.encryptedData);
	break;
    default:
	rv = SECFailure;
	break;
    }
    if (rv != SECSuccess) {
	PORT_Free(p7ecx);
	return NULL;
    }

    /* Initialize the BER encoder.
     * Note that this will not encode anything until the first call to SEC_ASN1EncoderUpdate */
    p7ecx->ecx = SEC_ASN1EncoderStart(cmsg, NSSCMSMessageTemplate,
				       nss_cms_encoder_out, &(p7ecx->output));
    if (p7ecx->ecx == NULL) {
	PORT_Free (p7ecx);
	return NULL;
    }
    p7ecx->ecxupdated = PR_FALSE;

    /*
     * Indicate that we are streaming.  We will be streaming until we
     * get past the contents bytes.
     */
    SEC_ASN1EncoderSetStreaming(p7ecx->ecx);

    /*
     * The notify function will watch for the contents field.
     */
    SEC_ASN1EncoderSetNotifyProc(p7ecx->ecx, nss_cms_encoder_notify, p7ecx);

    /* this will kick off the encoding process & encode everything up to the content bytes,
     * at which point the notify function sets streaming mode (and possibly creates
     * a child encoder). */
    if (SEC_ASN1EncoderUpdate(p7ecx->ecx, NULL, 0) != SECSuccess) {
	PORT_Free (p7ecx);
	return NULL;
    }

    return p7ecx;
}

/*
 * NSS_CMSEncoder_Update - take content data delivery from the user
 *
 * "p7ecx" - encoder context
 * "data" - content data
 * "len" - length of content data
 *
 * need to find the lowest level (and call SEC_ASN1EncoderUpdate on the way down),
 * then hand the data to the work_data fn
 */
SECStatus
NSS_CMSEncoder_Update(NSSCMSEncoderContext *p7ecx, const char *data, unsigned long len)
{
    SECStatus rv;
    NSSCMSContentInfo *cinfo;
    SECOidTag childtype;

    if (p7ecx->error)
	return SECFailure;

    /* hand data to the innermost decoder */
    if (p7ecx->childp7ecx) {
	/* recursion here */
	rv = NSS_CMSEncoder_Update(p7ecx->childp7ecx, data, len);
    } else {
	/* we are at innermost decoder */
	/* find out about our inner content type - must be data */
	cinfo = NSS_CMSContent_GetContentInfo(p7ecx->content.pointer, p7ecx->type);
	childtype = NSS_CMSContentInfo_GetContentTypeTag(cinfo);
	if (childtype != SEC_OID_PKCS7_DATA)
	    return SECFailure;
	/* and we must not have preset data */
	if (cinfo->content.data != NULL)
	    return SECFailure;

	/*  hand it the data so it can encode it (let DER trickle up the chain) */
	rv = nss_cms_encoder_work_data(p7ecx, NULL, (const unsigned char *)data, len, PR_FALSE, PR_TRUE);
    }
    return rv;
}

/*
 * NSS_CMSEncoder_Cancel - stop all encoding
 *
 * we need to walk down the chain of encoders and the finish them from the innermost out
 */
SECStatus
NSS_CMSEncoder_Cancel(NSSCMSEncoderContext *p7ecx)
{
    SECStatus rv = SECFailure;

    /* XXX do this right! */

    /*
     * Finish any inner decoders before us so that all the encoded data is flushed
     * This basically finishes all the decoders from the innermost to the outermost.
     * Finishing an inner decoder may result in data being updated to the outer decoder
     * while we are already in NSS_CMSEncoder_Finish, but that's allright.
     */
    if (p7ecx->childp7ecx) {
	rv = NSS_CMSEncoder_Cancel(p7ecx->childp7ecx); /* frees p7ecx->childp7ecx */
	/* remember rv for now */
    }

    /*
     * On the way back up, there will be no more data (if we had an
     * inner encoder, it is done now!)
     * Flush out any remaining data and/or finish digests.
     */
    rv = nss_cms_encoder_work_data(p7ecx, NULL, NULL, 0, PR_TRUE, (p7ecx->childp7ecx == NULL));
    if (rv != SECSuccess)
	goto loser;

    p7ecx->childp7ecx = NULL;

    /* kick the encoder back into working mode again.
     * We turn off streaming stuff (which will cause the encoder to continue
     * encoding happily, now that we have all the data (like digests) ready for it).
     */
    SEC_ASN1EncoderClearTakeFromBuf(p7ecx->ecx);
    SEC_ASN1EncoderClearStreaming(p7ecx->ecx);

    /* now that TakeFromBuf is off, this will kick this encoder to finish encoding */
    rv = SEC_ASN1EncoderUpdate(p7ecx->ecx, NULL, 0);

loser:
    SEC_ASN1EncoderFinish(p7ecx->ecx);
    PORT_Free (p7ecx);
    return rv;
}

/*
 * NSS_CMSEncoder_Finish - signal the end of data
 *
 * we need to walk down the chain of encoders and the finish them from the innermost out
 */
SECStatus
NSS_CMSEncoder_Finish(NSSCMSEncoderContext *p7ecx)
{
    SECStatus rv = SECFailure;
    NSSCMSContentInfo *cinfo;
    SECOidTag childtype;

    /*
     * Finish any inner decoders before us so that all the encoded data is flushed
     * This basically finishes all the decoders from the innermost to the outermost.
     * Finishing an inner decoder may result in data being updated to the outer decoder
     * while we are already in NSS_CMSEncoder_Finish, but that's allright.
     */
    if (p7ecx->childp7ecx) {
	rv = NSS_CMSEncoder_Finish(p7ecx->childp7ecx); /* frees p7ecx->childp7ecx */
	if (rv != SECSuccess)
	    goto loser;
    }

    /*
     * On the way back up, there will be no more data (if we had an
     * inner encoder, it is done now!)
     * Flush out any remaining data and/or finish digests.
     */
    rv = nss_cms_encoder_work_data(p7ecx, NULL, NULL, 0, PR_TRUE, (p7ecx->childp7ecx == NULL));
    if (rv != SECSuccess)
	goto loser;

    p7ecx->childp7ecx = NULL;

    /* find out about our inner content type - must be data */
    cinfo = NSS_CMSContent_GetContentInfo(p7ecx->content.pointer, p7ecx->type);
    childtype = NSS_CMSContentInfo_GetContentTypeTag(cinfo);
    if (childtype == SEC_OID_PKCS7_DATA && cinfo->content.data == NULL) {
	SEC_ASN1EncoderClearTakeFromBuf(p7ecx->ecx);
	/* now that TakeFromBuf is off, this will kick this encoder to finish encoding */
	rv = SEC_ASN1EncoderUpdate(p7ecx->ecx, NULL, 0);
    }

    SEC_ASN1EncoderClearStreaming(p7ecx->ecx);

    if (p7ecx->error)
	rv = SECFailure;

loser:
    SEC_ASN1EncoderFinish(p7ecx->ecx);
    PORT_Free (p7ecx);
    return rv;
}
