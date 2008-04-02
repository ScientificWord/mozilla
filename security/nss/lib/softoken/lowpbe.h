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

#ifndef _SECPKCS5_H_
#define _SECPKCS5_H_

#include "plarena.h"
#include "secitem.h"
#include "seccomon.h"
#include "secoidt.h"
#include "hasht.h"

typedef SECItem * (* SEC_PKCS5GetPBEPassword)(void *arg);

/* used for V2 PKCS 12 Draft Spec */ 
typedef enum {
    pbeBitGenIDNull = 0,
    pbeBitGenCipherKey = 0x01,
    pbeBitGenCipherIV = 0x02,
    pbeBitGenIntegrityKey = 0x03
} PBEBitGenID;

typedef enum {
    NSSPKCS5_PBKDF1 = 0,
    NSSPKCS5_PBKDF2 = 1,
    NSSPKCS5_PKCS12_V2 = 2
} NSSPKCS5PBEType;

typedef struct NSSPKCS5PBEParameterStr NSSPKCS5PBEParameter;

struct NSSPKCS5PBEParameterStr {
    PRArenaPool *poolp;
    SECItem	salt;		/* octet string */
    SECItem	iteration;	/* integer */
    SECItem	keyLength;	/* integer */

    /* used locally */
    int		iter;
    int 	keyLen;
    int		ivLen;
    unsigned char *ivData;
    HASH_HashType hashType;
    NSSPKCS5PBEType pbeType;
    SECAlgorithmID  prfAlg;	
    PBEBitGenID	keyID;
    SECOidTag	encAlg;
    PRBool	is2KeyDES;
};


SEC_BEGIN_PROTOS
/* Create a PKCS5 Algorithm ID
 * The algorithm ID is set up using the PKCS #5 parameter structure
 *  algorithm is the PBE algorithm ID for the desired algorithm
 *  pbe is a pbe param block with all the info needed to create the 
 *   algorithm id.
 * If an error occurs or the algorithm specified is not supported 
 * or is not a password based encryption algorithm, NULL is returned.
 * Otherwise, a pointer to the algorithm id is returned.
 */
extern SECAlgorithmID *
nsspkcs5_CreateAlgorithmID(PRArenaPool *arena, SECOidTag algorithm, 
						NSSPKCS5PBEParameter *pbe);

/*
 * Convert an Algorithm ID to a PBE Param.
 * NOTE: this does not suppport PKCS 5 v2 because it's only used for the
 * keyDB which only support PKCS 5 v1, PFX, and PKCS 12.
 */
NSSPKCS5PBEParameter *
nsspkcs5_AlgidToParam(SECAlgorithmID *algid);

/*
 * Convert an Algorithm ID to a PBE Param.
 * NOTE: this does not suppport PKCS 5 v2 because it's only used for the
 * keyDB which only support PKCS 5 v1, PFX, and PKCS 12.
 */
NSSPKCS5PBEParameter *
nsspkcs5_NewParam(SECOidTag alg, SECItem *salt, int iterator);


/* Encrypt/Decrypt data using password based encryption.  
 *  algid is the PBE algorithm identifier,
 *  pwitem is the password,
 *  src is the source for encryption/decryption,
 *  encrypt is PR_TRUE for encryption, PR_FALSE for decryption.
 * The key and iv are generated based upon PKCS #5 then the src
 * is either encrypted or decrypted.  If an error occurs, NULL
 * is returned, otherwise the ciphered contents is returned.
 */
extern SECItem *
nsspkcs5_CipherData(NSSPKCS5PBEParameter *, SECItem *pwitem,
		    SECItem *src, PRBool encrypt, PRBool *update);

extern SECItem *
nsspkcs5_ComputeKeyAndIV(NSSPKCS5PBEParameter *, SECItem *pwitem,
		    			SECItem *iv, PRBool faulty3DES);

/* Destroys PBE parameter */
extern void
nsspkcs5_DestroyPBEParameter(NSSPKCS5PBEParameter *param);

HASH_HashType HASH_FromHMACOid(SECOidTag oid);

SEC_END_PROTOS

#endif
