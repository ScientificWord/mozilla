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

#ifdef DEBUG
static const char CVS_ID[] = "@(#) $RCSfile$ $Revision$ $Date$";
#endif /* DEBUG */

#ifndef NSS_3_4_CODE
#define NSS_3_4_CODE
#endif /* NSS_3_4_CODE */

#ifndef PKIT_H
#include "pkit.h"
#endif /* PKIT_H */

#ifndef DEVM_H
#include "devm.h"
#endif /* DEVM_H */

#include "pki3hack.h"
#include "dev3hack.h"
#include "pkim.h"

#ifndef BASE_H
#include "base.h"
#endif /* BASE_H */

#include "pk11func.h"
#include "secmodti.h"

NSS_IMPLEMENT nssSession *
nssSession_ImportNSS3Session(NSSArena *arenaOpt,
                             CK_SESSION_HANDLE session, 
                             PZLock *lock, PRBool rw)
{
    nssSession *rvSession;
    rvSession = nss_ZNEW(arenaOpt, nssSession);
    rvSession->handle = session;
    rvSession->lock = lock;
    rvSession->ownLock = PR_FALSE;
    rvSession->isRW = rw;
    return rvSession;
}

NSS_IMPLEMENT nssSession *
nssSlot_CreateSession
(
  NSSSlot *slot,
  NSSArena *arenaOpt,
  PRBool readWrite
)
{
    nssSession *rvSession;
    rvSession = nss_ZNEW(arenaOpt, nssSession);
    if (!rvSession) {
	return (nssSession *)NULL;
    }
    if (readWrite) {
	rvSession->handle = PK11_GetRWSession(slot->pk11slot);
	if (rvSession->handle == CK_INVALID_HANDLE) {
	    nss_ZFreeIf(rvSession);
	    return NULL;
	}
	rvSession->isRW = PR_TRUE;
	rvSession->slot = slot;
        /*
         * The session doesn't need its own lock.  Here's why.
         * 1. If we are reusing the default RW session of the slot,
         *    the slot lock is already locked to protect the session.
         * 2. If the module is not thread safe, the slot (or rather
         *    module) lock is already locked.
         * 3. If the module is thread safe and we are using a new
         *    session, no higher-level lock has been locked and we
         *    would need a lock for the new session.  However, the
         *    NSS_3_4_CODE usage of the session is that it is always
         *    used and destroyed within the same function and never
         *    shared with another thread.
         * So the session is either already protected by another
         * lock or only used by one thread.
         */
        rvSession->lock = NULL;
        rvSession->ownLock = PR_FALSE;
	return rvSession;
    } else {
	return NULL;
    }
}

NSS_IMPLEMENT PRStatus
nssSession_Destroy
(
  nssSession *s
)
{
    CK_RV ckrv = CKR_OK;
    if (s) {
	if (s->isRW) {
	    PK11_RestoreROSession(s->slot->pk11slot, s->handle);
	}
	nss_ZFreeIf(s);
    }
    return (ckrv == CKR_OK) ? PR_SUCCESS : PR_FAILURE;
}

static NSSSlot *
nssSlot_CreateFromPK11SlotInfo(NSSTrustDomain *td, PK11SlotInfo *nss3slot)
{
    NSSSlot *rvSlot;
    NSSArena *arena;
    arena = nssArena_Create();
    if (!arena) {
	return NULL;
    }
    rvSlot = nss_ZNEW(arena, NSSSlot);
    if (!rvSlot) {
	nssArena_Destroy(arena);
	return NULL;
    }
    rvSlot->base.refCount = 1;
    rvSlot->base.lock = PZ_NewLock(nssILockOther);
    rvSlot->base.arena = arena;
    rvSlot->pk11slot = nss3slot;
    rvSlot->epv = nss3slot->functionList;
    rvSlot->slotID = nss3slot->slotID;
    /* Grab the slot name from the PKCS#11 fixed-length buffer */
    rvSlot->base.name = nssUTF8_Duplicate(nss3slot->slot_name,td->arena);
    rvSlot->lock = (nss3slot->isThreadSafe) ? NULL : nss3slot->sessionLock;
    return rvSlot;
}

NSS_IMPLEMENT NSSToken *
nssToken_CreateFromPK11SlotInfo(NSSTrustDomain *td, PK11SlotInfo *nss3slot)
{
    NSSToken *rvToken;
    NSSArena *arena;
    arena = nssArena_Create();
    if (!arena) {
	return NULL;
    }
    rvToken = nss_ZNEW(arena, NSSToken);
    if (!rvToken) {
	nssArena_Destroy(arena);
	return NULL;
    }
    rvToken->base.refCount = 1;
    rvToken->base.lock = PZ_NewLock(nssILockOther);
    rvToken->base.arena = arena;
    rvToken->pk11slot = nss3slot;
    rvToken->epv = nss3slot->functionList;
    rvToken->defaultSession = nssSession_ImportNSS3Session(td->arena,
                                                       nss3slot->session,
                                                       nss3slot->sessionLock,
                                                       nss3slot->defRWSession);
    /* The above test was used in 3.4, for this cache have it always on */
    if (!PK11_IsInternal(nss3slot) && PK11_IsHW(nss3slot)) {
	rvToken->cache = nssTokenObjectCache_Create(rvToken, 
	                                            PR_TRUE, PR_TRUE, PR_TRUE);
	if (!rvToken->cache) {
	    nssArena_Destroy(arena);
	    return (NSSToken *)NULL;
	}
    }
    rvToken->trustDomain = td;
    /* Grab the token name from the PKCS#11 fixed-length buffer */
    rvToken->base.name = nssUTF8_Duplicate(nss3slot->token_name,td->arena);
    rvToken->slot = nssSlot_CreateFromPK11SlotInfo(td, nss3slot);
    rvToken->slot->token = rvToken;
    rvToken->defaultSession->slot = rvToken->slot;
    return rvToken;
}

NSS_IMPLEMENT void
nssToken_UpdateName(NSSToken *token)
{
    if (!token) {
	return;
    }
    token->base.name = nssUTF8_Duplicate(token->pk11slot->token_name,token->base.arena);
}

NSS_IMPLEMENT PRBool
nssSlot_IsPermanent
(
  NSSSlot *slot
)
{
    return slot->pk11slot->isPerm;
}

NSS_IMPLEMENT PRBool
nssSlot_IsFriendly
(
  NSSSlot *slot
)
{
    return PK11_IsFriendly(slot->pk11slot);
}

NSS_IMPLEMENT PRStatus
nssToken_Refresh(NSSToken *token)
{
    PK11SlotInfo *nss3slot;

    if (!token) {
	return PR_SUCCESS;
    }
    nss3slot = token->pk11slot;
    token->defaultSession = nssSession_ImportNSS3Session(token->slot->base.arena,
                                                       nss3slot->session,
                                                       nss3slot->sessionLock,
                                                       nss3slot->defRWSession);
    return PR_SUCCESS;
}

NSS_IMPLEMENT PRStatus
nssSlot_Refresh
(
  NSSSlot *slot
)
{
    PK11SlotInfo *nss3slot = slot->pk11slot;
    PRBool doit = PR_FALSE;
    if (slot->token->base.name[0] == 0) {
	doit = PR_TRUE;
    }
    if (PK11_InitToken(nss3slot, PR_FALSE) != SECSuccess) {
	return PR_FAILURE;
    }
    if (doit) {
	nssTrustDomain_UpdateCachedTokenCerts(slot->token->trustDomain, 
	                                      slot->token);
    }
    return nssToken_Refresh(slot->token);
}

NSS_IMPLEMENT PRStatus
nssToken_GetTrustOrder
(
  NSSToken *tok
)
{
    PK11SlotInfo *slot;
    SECMODModule *module;
    slot = tok->pk11slot;
    module = PK11_GetModule(slot);
    return module->trustOrder;
}

NSS_IMPLEMENT PRBool
nssSlot_IsLoggedIn
(
  NSSSlot *slot
)
{
    if (!slot->pk11slot->needLogin) {
	return PR_TRUE;
    }
    return PK11_IsLoggedIn(slot->pk11slot, NULL);
}


NSSTrustDomain *
nssToken_GetTrustDomain(NSSToken *token)
{
    return token->trustDomain;
}

NSS_EXTERN PRStatus
nssTrustDomain_RemoveTokenCertsFromCache
(
  NSSTrustDomain *td,
  NSSToken *token
);

NSS_IMPLEMENT PRStatus
nssToken_NotifyCertsNotVisible
(
  NSSToken *tok
)
{
    return nssTrustDomain_RemoveTokenCertsFromCache(tok->trustDomain, tok);
}

