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
 * Internal header file included only by files in pkcs11 dir, or in
 * pkcs11 specific client and server files.
 */

#ifndef  _SECMODTI_H_
#define  _SECMODTI_H_ 1
#include "prmon.h"
#include "prtypes.h"
#include "nssilckt.h"
#include "pk11init.h"
#include "secmodt.h"
#include "pkcs11t.h"

#ifndef NSS_3_4_CODE
#define NSS_3_4_CODE
#endif /* NSS_3_4_CODE */
#include "nssdevt.h"

/* internal data structures */

/* Traverse slots callback */
typedef struct pk11TraverseSlotStr {
    SECStatus (*callback)(PK11SlotInfo *,CK_OBJECT_HANDLE, void *);
    void *callbackArg;
    CK_ATTRIBUTE *findTemplate;
    int templateCount;
} pk11TraverseSlot;


/* represent a pkcs#11 slot reference counted. */
struct PK11SlotInfoStr {
    /* the PKCS11 function list for this slot */
    void *functionList;
    SECMODModule *module; /* our parent module */
    /* Boolean to indicate the current state of this slot */
    PRBool needTest;	/* Has this slot been tested for Export complience */
    PRBool isPerm;	/* is this slot a permanment device */
    PRBool isHW;	/* is this slot a hardware device */
    PRBool isInternal;  /* is this slot one of our internal PKCS #11 devices */
    PRBool disabled;	/* is this slot disabled... */
    PK11DisableReasons reason; 	/* Why this slot is disabled */
    PRBool readOnly;	/* is the token in this slot read-only */
    PRBool needLogin;	/* does the token of the type that needs 
			 * authentication (still true even if token is logged 
			 * in) */
    PRBool hasRandom;   /* can this token generated random numbers */
    PRBool defRWSession; /* is the default session RW (we open our default 
			  * session rw if the token can only handle one session
			  * at a time. */
    PRBool isThreadSafe; /* copied from the module */
    /* The actual flags (many of which are distilled into the above PRBools) */
    CK_FLAGS flags;      /* flags from PKCS #11 token Info */
    /* a default session handle to do quick and dirty functions */
    CK_SESSION_HANDLE session; 
    PZLock *sessionLock; /* lock for this session */
    /* our ID */
    CK_SLOT_ID slotID;
    /* persistant flags saved from startup to startup */
    unsigned long defaultFlags;
    /* keep track of who is using us so we don't accidently get freed while
     * still in use */
    PRInt32 refCount;    /* to be in/decremented by atomic calls ONLY! */
    PZLock *freeListLock;
    PK11SymKey *freeSymKeysWithSessionHead;
    PK11SymKey *freeSymKeysHead;
    int keyCount;
    int maxKeyCount;
    /* Password control functions for this slot. many of these are only
     * active if the appropriate flag is on in defaultFlags */
    int askpw;		/* what our password options are */
    int timeout;	/* If we're ask_timeout, what is our timeout time is 
			 * seconds */
    int authTransact;   /* allow multiple authentications off one password if
		         * they are all part of the same transaction */
    int64 authTime;     /* when were we last authenticated */
    int minPassword;	/* smallest legal password */
    int maxPassword;	/* largest legal password */
    uint16 series;	/* break up the slot info into various groups of 
			 * inserted tokens so that keys and certs can be
			 * invalidated */
    uint16 flagSeries;	/* record the last series for the last event
                         * returned for this slot */
    PRBool flagState;	/* record the state of the last event returned for this
			 * slot. */
    uint16 wrapKey;	/* current wrapping key for SSL master secrets */
    CK_MECHANISM_TYPE wrapMechanism;
			/* current wrapping mechanism for current wrapKey */
    CK_OBJECT_HANDLE refKeys[1]; /* array of existing wrapping keys for */
    CK_MECHANISM_TYPE *mechanismList; /* list of mechanism supported by this
				       * token */
    int mechanismCount;
    /* cache the certificates stored on the token of this slot */
    CERTCertificate **cert_array;
    int array_size;
    int cert_count;
    char serial[16];
    /* since these are odd sizes, keep them last. They are odd sizes to 
     * allow them to become null terminated strings */
    char slot_name[65];
    char token_name[33];
    PRBool hasRootCerts;
    PRBool hasRootTrust;
    PRBool hasRSAInfo;
    CK_FLAGS RSAInfoFlags;
    PRBool protectedAuthPath;
    PRBool isActiveCard;
    PRIntervalTime lastLoginCheck;
    unsigned int lastState;
    /* for Stan */
    NSSToken *nssToken;
    /* fast mechanism lookup */
    char mechanismBits[256];
};

/* Symetric Key structure. Reference Counted */
struct PK11SymKeyStr {
    CK_MECHANISM_TYPE type;	/* type of operation this key was created for*/
    CK_OBJECT_HANDLE  objectID; /* object id of this key in the slot */
    PK11SlotInfo      *slot;    /* Slot this key is loaded into */
    void	      *cx;	/* window context in case we need to loggin */
    PK11SymKey	      *next;
    PRBool	      owner;
    SECItem	      data;	/* raw key data if available */
    CK_SESSION_HANDLE session;
    PRBool	      sessionOwner;
    PRInt32	      refCount;	/* number of references to this key */
    int		      size;	/* key size in bytes */
    PK11Origin	      origin;	/* where this key came from 
                                 * (see def in secmodt.h) */
    PK11SymKey        *parent;  /* potential owner key of the session */
    uint16 series;		/* break up the slot info into various groups 
				 * of inserted tokens so that keys and certs 
				 * can be invalidated */
    void *userData;		/* random data the application can attach to
                                 * this key */
    PK11FreeDataFunc freeFunc;	/* function to free the user data */
};


/*
 * hold a hash, encryption or signing context for multi-part operations.
 * hold enough information so that multiple contexts can be interleaved
 * if necessary. ... Not RefCounted.
 */
struct PK11ContextStr {
    CK_ATTRIBUTE_TYPE	operation; /* type of operation this context is doing
				    * (CKA_ENCRYPT, CKA_SIGN, CKA_HASH, etc. */
    PK11SymKey  	*key;	   /* symetric key used in this context */
    PK11SlotInfo	*slot;	   /* slot this context is operationing on */
    CK_SESSION_HANDLE	session;   /* session this context is using */
    PZLock		*sessionLock; /* lock before accessing a PKCS #11 
				       * session */
    PRBool		ownSession;/* do we own the session? */
    void 		*cx;	   /* window context in case we need to loggin*/
    void		*savedData;/* save data when we are multiplexing on a
				    * single context */
    unsigned long	savedLength; /* length of the saved context */
    SECItem		*param;	    /* mechanism parameters used to build this
								context */
    PRBool		init;	    /* has this contexted been initialized */
    CK_MECHANISM_TYPE	type;	    /* what is the PKCS #11 this context is
				     * representing (usually what algorithm is
				     * being used (CKM_RSA_PKCS, CKM_DES,
				     * CKM_SHA, etc.*/
    PRBool		fortezzaHack; /*Fortezza SSL has some special
				       * non-standard semantics*/
};

/*
 * structure to hold a pointer to a unique PKCS #11 object 
 * (pointer to the slot and the object id).
 */
struct PK11GenericObjectStr {
    PK11GenericObject *prev;
    PK11GenericObject *next;
    PK11SlotInfo *slot;
    CK_OBJECT_HANDLE objectID;
};


#define MAX_TEMPL_ATTRS 16 /* maximum attributes in template */

/* This mask includes all CK_FLAGs with an equivalent CKA_ attribute. */
#define CKF_KEY_OPERATION_FLAGS 0x000e7b00UL


#endif /* _SECMODTI_H_ */
