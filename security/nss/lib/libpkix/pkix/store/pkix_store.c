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
 * The Original Code is the PKIX-C library.
 *
 * The Initial Developer of the Original Code is
 * Sun Microsystems, Inc.
 * Portions created by the Initial Developer are
 * Copyright 2004-2007 Sun Microsystems, Inc.  All Rights Reserved.
 *
 * Contributor(s):
 *   Sun Microsystems, Inc.
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
 * pkix_store.c
 *
 * CertStore Function Definitions
 *
 */

#include "pkix_store.h"

/* --CertStore-Private-Functions----------------------------------------- */

/*
 * FUNCTION: pkix_CertStore_Destroy
 * (see comments for PKIX_PL_DestructorCallback in pkix_pl_system.h)
 */
static PKIX_Error *
pkix_CertStore_Destroy(
        PKIX_PL_Object *object,
        void *plContext)
{
        PKIX_CertStore *certStore = NULL;

        PKIX_ENTER(CERTSTORE, "pkix_CertStore_Destroy");
        PKIX_NULLCHECK_ONE(object);

        /* Check that this object is a CertStore object */
        PKIX_CHECK(pkix_CheckType(object, PKIX_CERTSTORE_TYPE, plContext),
                PKIX_OBJECTNOTCERTSTORE);

        certStore = (PKIX_CertStore *)object;

        certStore->certCallback = NULL;
        certStore->crlCallback = NULL;
        certStore->certContinue = NULL;
        certStore->crlContinue = NULL;
        certStore->trustCallback = NULL;

        PKIX_DECREF(certStore->certStoreContext);

cleanup:

        PKIX_RETURN(CERTSTORE);
}

/*
 * FUNCTION: pkix_CertStore_Hashcode
 * (see comments for PKIX_PL_HashcodeCallback in pkix_pl_system.h)
 */
static PKIX_Error *
pkix_CertStore_Hashcode(
        PKIX_PL_Object *object,
        PKIX_UInt32 *pHashcode,
        void *plContext)
{
        PKIX_CertStore *certStore = NULL;
        PKIX_UInt32 tempHash = 0;

        PKIX_ENTER(CERTSTORE, "pkix_CertStore_Hashcode");
        PKIX_NULLCHECK_TWO(object, pHashcode);

        PKIX_CHECK(pkix_CheckType(object, PKIX_CERTSTORE_TYPE, plContext),
                    PKIX_OBJECTNOTCERTSTORE);

        certStore = (PKIX_CertStore *)object;

        if (certStore->certStoreContext) {
                PKIX_CHECK(PKIX_PL_Object_Hashcode
                    ((PKIX_PL_Object *) certStore->certStoreContext,
                    &tempHash,
                    plContext),
                   PKIX_CERTSTOREHASHCODEFAILED);
        }

        *pHashcode = (PKIX_UInt32) certStore->certCallback +
                     (PKIX_UInt32) certStore->crlCallback +
                     (PKIX_UInt32) certStore->certContinue +
                     (PKIX_UInt32) certStore->crlContinue +
                     (PKIX_UInt32) certStore->trustCallback +
                     (tempHash << 7);

cleanup:

        PKIX_RETURN(CERTSTORE);
}

/*
 * FUNCTION: pkix_CertStore_Equals
 * (see comments for PKIX_PL_EqualsCallback in pkix_pl_system.h)
 */
static PKIX_Error *
pkix_CertStore_Equals(
        PKIX_PL_Object *firstObject,
        PKIX_PL_Object *secondObject,
        PKIX_Int32 *pResult,
        void *plContext)
{
        PKIX_CertStore *firstCS = NULL;
        PKIX_CertStore *secondCS = NULL;
        PKIX_Boolean cmpResult = PKIX_FALSE;

        PKIX_ENTER(CERTSTORE, "pkix_CertStore_Equals");
        PKIX_NULLCHECK_THREE(firstObject, secondObject, pResult);

        PKIX_CHECK(pkix_CheckTypes
                    (firstObject, secondObject, PKIX_CERTSTORE_TYPE, plContext),
                    PKIX_ARGUMENTSNOTDATES);

        firstCS = (PKIX_CertStore *)firstObject;
        secondCS = (PKIX_CertStore *)secondObject;

        cmpResult = (firstCS->certCallback == secondCS->certCallback) &&
            (firstCS->crlCallback == secondCS->crlCallback) &&
            (firstCS->certContinue == secondCS->certContinue) &&
            (firstCS->crlContinue == secondCS->crlContinue) &&
            (firstCS->trustCallback == secondCS->trustCallback);

        if (cmpResult &&
            (firstCS->certStoreContext != secondCS->certStoreContext)) {

                PKIX_CHECK(PKIX_PL_Object_Equals
                    ((PKIX_PL_Object *) firstCS->certStoreContext,
                    (PKIX_PL_Object *) secondCS->certStoreContext,
                    &cmpResult,
                    plContext),
                    PKIX_CERTSTOREEQUALSFAILED);
        }

        *pResult = cmpResult;

cleanup:

        PKIX_RETURN(CERTSTORE);
}

/*
 * FUNCTION: pkix_CertStore_RegisterSelf
 * DESCRIPTION:
 *  Registers PKIX_CERTSTORE_TYPE and its related functions with
 *  systemClasses[]
 * THREAD SAFETY:
 *  Not Thread Safe - for performance and complexity reasons
 *
 *  Since this function is only called by PKIX_PL_Initialize, which should
 *  only be called once, it is acceptable that this function is not
 *  thread-safe.
 */
PKIX_Error *
pkix_CertStore_RegisterSelf(void *plContext)
{
        extern pkix_ClassTable_Entry systemClasses[PKIX_NUMTYPES];
        pkix_ClassTable_Entry entry;

        PKIX_ENTER(CERTSTORE, "pkix_CertStore_RegisterSelf");

        entry.description = "CertStore";
        entry.objCounter = 0;
        entry.typeObjectSize = sizeof(PKIX_CertStore);
        entry.destructor = pkix_CertStore_Destroy;
        entry.equalsFunction = pkix_CertStore_Equals;
        entry.hashcodeFunction = pkix_CertStore_Hashcode;
        entry.toStringFunction = NULL;
        entry.comparator = NULL;
        entry.duplicateFunction = pkix_duplicateImmutable;

        systemClasses[PKIX_CERTSTORE_TYPE] = entry;

        PKIX_RETURN(CERTSTORE);
}

/* --CertStore-Public-Functions------------------------------------------ */

/*
 * FUNCTION: PKIX_CertStore_Create (see comments in pkix_certstore.h)
 */
PKIX_Error *
PKIX_CertStore_Create(
        PKIX_CertStore_CertCallback certCallback,
        PKIX_CertStore_CRLCallback crlCallback,
        PKIX_CertStore_CertContinueFunction certContinue,
        PKIX_CertStore_CrlContinueFunction crlContinue,
        PKIX_CertStore_CheckTrustCallback trustCallback,
        PKIX_PL_Object *certStoreContext,
        PKIX_Boolean cacheFlag,
        PKIX_Boolean localFlag,
        PKIX_CertStore **pStore,
        void *plContext)
{
        PKIX_CertStore *certStore = NULL;

        PKIX_ENTER(CERTSTORE, "PKIX_CertStore_Create");
        PKIX_NULLCHECK_THREE(certCallback, crlCallback, pStore);

        PKIX_CHECK(PKIX_PL_Object_Alloc
                    (PKIX_CERTSTORE_TYPE,
                    sizeof (PKIX_CertStore),
                    (PKIX_PL_Object **)&certStore,
                    plContext),
                    PKIX_COULDNOTCREATECERTSTOREOBJECT);

        certStore->certCallback = certCallback;
        certStore->crlCallback = crlCallback;
        certStore->certContinue = certContinue;
        certStore->crlContinue = crlContinue;
        certStore->trustCallback = trustCallback;
        certStore->cacheFlag = cacheFlag;
        certStore->localFlag = localFlag;

        PKIX_INCREF(certStoreContext);
        certStore->certStoreContext = certStoreContext;

        *pStore = certStore;
        certStore = NULL;

cleanup:

        PKIX_DECREF(certStore);

        PKIX_RETURN(CERTSTORE);
}

/*
 * FUNCTION: PKIX_CertStore_GetCertCallback (see comments in pkix_certstore.h)
 */
PKIX_Error *
PKIX_CertStore_GetCertCallback(
        PKIX_CertStore *store,
        PKIX_CertStore_CertCallback *pCallback,
        void *plContext)
{
        PKIX_ENTER(CERTSTORE, "PKIX_CertStore_GetCertCallback");
        PKIX_NULLCHECK_TWO(store, pCallback);

        *pCallback = store->certCallback;

        PKIX_RETURN(CERTSTORE);
}

/*
 * FUNCTION: PKIX_CertStore_GetCRLCallback (see comments in pkix_certstore.h)
 */
PKIX_Error *
PKIX_CertStore_GetCRLCallback(
        PKIX_CertStore *store,
        PKIX_CertStore_CRLCallback *pCallback,
        void *plContext)
{
        PKIX_ENTER(CERTSTORE, "PKIX_CertStore_GetCRLCallback");
        PKIX_NULLCHECK_TWO(store, pCallback);

        *pCallback = store->crlCallback;

        PKIX_RETURN(CERTSTORE);
}

/*
 * FUNCTION: PKIX_CertStore_CertContinue (see comments in pkix_certstore.h)
 */
PKIX_Error *
PKIX_CertStore_CertContinue(
        PKIX_CertStore *store,
        PKIX_CertSelector *selector,
        void **pNBIOContext,
        PKIX_List **pCertList,
        void *plContext)
{
        PKIX_ENTER(CERTSTORE, "PKIX_CertStore_CertContinue");
        PKIX_NULLCHECK_FOUR(store, selector, pNBIOContext, pCertList);

        PKIX_CHECK(store->certContinue
                (store, selector, pNBIOContext, pCertList, plContext),
                PKIX_CERTSTORECERTCONTINUEFUNCTIONFAILED);

cleanup:

        PKIX_RETURN(CERTSTORE);
}

/*
 * FUNCTION: PKIX_CertStore_CrlContinue (see comments in pkix_certstore.h)
 */
PKIX_Error *
PKIX_CertStore_CrlContinue(
        PKIX_CertStore *store,
        PKIX_CRLSelector *selector,
        void **pNBIOContext,
        PKIX_List **pCrlList,
        void *plContext)
{
        PKIX_ENTER(CERTSTORE, "PKIX_CertStore_CrlContinue");
        PKIX_NULLCHECK_FOUR(store, selector, pNBIOContext, pCrlList);

        PKIX_CHECK(store->crlContinue
                (store, selector, pNBIOContext, pCrlList, plContext),
                PKIX_CERTSTORECRLCONTINUEFAILED);

cleanup:

        PKIX_RETURN(CERTSTORE);
}

/*
 * FUNCTION: PKIX_CertStore_GetTrustCallback (see comments in pkix_certstore.h)
 */
PKIX_Error *
PKIX_CertStore_GetTrustCallback(
        PKIX_CertStore *store,
        PKIX_CertStore_CheckTrustCallback *pCallback,
        void *plContext)
{
        PKIX_ENTER(CERTSTORE, "PKIX_CertStore_GetTrustCallback");
        PKIX_NULLCHECK_TWO(store, pCallback);

        *pCallback = store->trustCallback;

        PKIX_RETURN(CERTSTORE);
}

/*
 * FUNCTION: PKIX_CertStore_GetCertStoreContext
 * (see comments in pkix_certstore.h)
 */
PKIX_Error *
PKIX_CertStore_GetCertStoreContext(
        PKIX_CertStore *store,
        PKIX_PL_Object **pCertStoreContext,
        void *plContext)
{
        PKIX_ENTER(CERTSTORE, "PKIX_CertStore_GetCertStoreContext");
        PKIX_NULLCHECK_TWO(store, pCertStoreContext);

        PKIX_INCREF(store->certStoreContext);
        *pCertStoreContext = store->certStoreContext;

cleanup:
        PKIX_RETURN(CERTSTORE);
}

/*
 * FUNCTION: PKIX_CertStore_GetCertStoreCacheFlag
 * (see comments in pkix_certstore.h)
 */
PKIX_Error *
PKIX_CertStore_GetCertStoreCacheFlag(
        PKIX_CertStore *store,
        PKIX_Boolean *pCacheFlag,
        void *plContext)
{
        PKIX_ENTER(CERTSTORE, "PKIX_CertStore_GetCertStoreCacheFlag");
        PKIX_NULLCHECK_TWO(store, pCacheFlag);

        *pCacheFlag = store->cacheFlag;

        PKIX_RETURN(CERTSTORE);
}

/*
 * FUNCTION: PKIX_CertStore_GetLocalFlag
 * (see comments in pkix_certstore.h)
 */
PKIX_Error *
PKIX_CertStore_GetLocalFlag(
        PKIX_CertStore *store,
        PKIX_Boolean *pLocalFlag,
        void *plContext)
{
        PKIX_ENTER(CERTSTORE, "PKIX_CertStore_GetLocalFlag");
        PKIX_NULLCHECK_TWO(store, pLocalFlag);

        *pLocalFlag = store->localFlag;

        PKIX_RETURN(CERTSTORE);
}
