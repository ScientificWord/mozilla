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
 * pkix_build.c
 *
 * Top level buildChain function
 *
 */

/* #define PKIX_BUILDDEBUG 1 */
/* #define PKIX_FORWARDBUILDERSTATEDEBUG 1 */

#include "pkix_build.h"

/*
 * List of critical extension OIDs associate with what build chain has
 * checked. Those OIDs need to be removed from the unresolved critical
 * extension OIDs list manually (instead of by checker automatically).
 */
static char *buildCheckedCritExtOIDs[] = {
        PKIX_CERTKEYUSAGE_OID,
        PKIX_CERTSUBJALTNAME_OID,
        PKIX_BASICCONSTRAINTS_OID,
        PKIX_NAMECONSTRAINTS_OID,
        PKIX_EXTENDEDKEYUSAGE_OID,
        PKIX_NSCERTTYPE_OID,
        NULL
};

/* --Private-ForwardBuilderState-Functions---------------------------------- */

/*
 * FUNCTION: pkix_ForwardBuilderState_Destroy
 * (see comments for PKIX_PL_DestructorCallback in pkix_pl_system.h)
 */
static PKIX_Error *
pkix_ForwardBuilderState_Destroy(
        PKIX_PL_Object *object,
        void *plContext)
{
        PKIX_ForwardBuilderState *state = NULL;

        PKIX_ENTER(FORWARDBUILDERSTATE, "pkix_ForwardBuilderState_Destroy");
        PKIX_NULLCHECK_ONE(object);

        PKIX_CHECK(pkix_CheckType
                (object, PKIX_FORWARDBUILDERSTATE_TYPE, plContext),
                PKIX_OBJECTNOTFORWARDBUILDERSTATE);

        state = (PKIX_ForwardBuilderState *)object;

        state->status = BUILD_INITIAL;
        state->traversedCACerts = 0;
        state->certStoreIndex = 0;
        state->numCerts = 0;
        state->numAias = 0;
        state->certIndex = 0;
        state->aiaIndex = 0;
        state->anchorIndex = 0;
        state->certCheckedIndex = 0;
        state->checkerIndex = 0;
        state->hintCertIndex = 0;
        state->numFanout = 0;
        state->numDepth = 0;
        state->reasonCode = 0;
        state->dsaParamsNeeded = PKIX_FALSE;
        state->revCheckDelayed = PKIX_FALSE;
        state->canBeCached = PKIX_FALSE;
        state->useOnlyLocal = PKIX_FALSE;
        state->alreadyTriedAIA = PKIX_FALSE;
        state->revChecking = PKIX_FALSE;
        state->usingHintCerts = PKIX_FALSE;
        state->certLoopingDetected = PKIX_FALSE;
        PKIX_DECREF(state->validityDate);
        PKIX_DECREF(state->prevCert);
        PKIX_DECREF(state->candidateCert);
        PKIX_DECREF(state->traversedSubjNames);
        PKIX_DECREF(state->trustChain);
        PKIX_DECREF(state->aia);
        PKIX_DECREF(state->candidateCerts);
        PKIX_DECREF(state->reversedCertChain);
        PKIX_DECREF(state->checkedCritExtOIDs);
        PKIX_DECREF(state->checkerChain);
        PKIX_DECREF(state->revCheckers);
        PKIX_DECREF(state->certSel);
        PKIX_DECREF(state->verifyNode);
        PKIX_DECREF(state->client);

        /*
         * If we ever add a child link we have to be careful not to have loops
         * in the Destroy process. But with one-way links we should be okay.
         */
        if (state->parentState == NULL) {
                state->buildConstants.numAnchors = 0;
                state->buildConstants.numCertStores = 0;
                state->buildConstants.numHintCerts = 0;
                state->buildConstants.procParams = 0;
                PKIX_DECREF(state->buildConstants.testDate);
                PKIX_DECREF(state->buildConstants.timeLimit);
                PKIX_DECREF(state->buildConstants.targetCert);
                PKIX_DECREF(state->buildConstants.targetPubKey);
                PKIX_DECREF(state->buildConstants.certStores);
                PKIX_DECREF(state->buildConstants.anchors);
                PKIX_DECREF(state->buildConstants.userCheckers);
                PKIX_DECREF(state->buildConstants.hintCerts);
                PKIX_DECREF(state->buildConstants.crlChecker);
                PKIX_DECREF(state->buildConstants.aiaMgr);
        } else {
                PKIX_DECREF(state->parentState);
        }

cleanup:

        PKIX_RETURN(FORWARDBUILDERSTATE);
}

/*
 * FUNCTION: pkix_ForwardBuilderState_Create
 *
 * DESCRIPTION:
 *  Allocate and initialize a ForwardBuilderState.
 *
 * PARAMETERS
 *  "traversedCACerts"
 *      Number of CA certificates traversed.
 *  "numFanout"
 *      Number of Certs that can be considered at this level (0 = no limit)
 *  "numDepth"
 *      Number of additional levels that can be searched (0 = no limit)
 *  "dsaParamsNeeded"
 *      Boolean value indicating whether DSA parameters are needed.
 *  "revCheckDelayed"
 *      Boolean value indicating whether rev check is delayed until after
 *      entire chain is built.
 *  "canBeCached"
 *      Boolean value indicating whether all certs on the chain can be cached.
 *  "validityDate"
 *      Address of Date at which build chain Certs' most restricted validity
 *      time is kept. May be NULL.
 *  "prevCert"
 *      Address of Cert just traversed. Must be non-NULL.
 *  "traversedSubjNames"
 *      Address of List of GeneralNames that have been traversed.
 *      Must be non-NULL.
 *  "trustChain"
 *      Address of List of certificates traversed. Must be non-NULL.
 *  "parentState"
 *      Address of previous ForwardBuilder state
 *  "pState"
 *      Address where ForwardBuilderState will be stored. Must be non-NULL.
 *  "plContext"
 *      Platform-specific context pointer.
 * THREAD SAFETY:
 *  Thread Safe (see Thread Safety Definitions in Programmer's Guide)
 * RETURNS:
 *  Returns NULL if the function succeeds.
 *  Returns a Build Error if the function fails in a non-fatal way.
 *  Returns a Fatal Error if the function fails in an unrecoverable way.
 */
static PKIX_Error *
pkix_ForwardBuilderState_Create(
        PKIX_Int32 traversedCACerts,
        PKIX_UInt32 numFanout,
        PKIX_UInt32 numDepth,
        PKIX_Boolean dsaParamsNeeded,
        PKIX_Boolean revCheckDelayed,
        PKIX_Boolean canBeCached,
        PKIX_PL_Date *validityDate,
        PKIX_PL_Cert *prevCert,
        PKIX_List *traversedSubjNames,
        PKIX_List *trustChain,
        PKIX_ForwardBuilderState *parentState,
        PKIX_ForwardBuilderState **pState,
        void *plContext)
{
        PKIX_ForwardBuilderState *state = NULL;

        PKIX_ENTER(FORWARDBUILDERSTATE, "pkix_ForwardBuilderState_Create");
        PKIX_NULLCHECK_FOUR(prevCert, traversedSubjNames, pState, trustChain);

        PKIX_CHECK(PKIX_PL_Object_Alloc
                (PKIX_FORWARDBUILDERSTATE_TYPE,
                sizeof (PKIX_ForwardBuilderState),
                (PKIX_PL_Object **)&state,
                plContext),
                PKIX_COULDNOTCREATEFORWARDBUILDERSTATEOBJECT);

        state->status = BUILD_INITIAL;
        state->traversedCACerts = traversedCACerts;
        state->certStoreIndex = 0;
        state->numCerts = 0;
        state->numAias = 0;
        state->certIndex = 0;
        state->aiaIndex = 0;
        state->anchorIndex = 0;
        state->certCheckedIndex = 0;
        state->checkerIndex = 0;
        state->hintCertIndex = 0;
        state->numFanout = numFanout;
        state->numDepth = numDepth;
        state->reasonCode = 0;
        state->revChecking = numDepth;
        state->dsaParamsNeeded = dsaParamsNeeded;
        state->revCheckDelayed = revCheckDelayed;
        state->canBeCached = canBeCached;
        state->useOnlyLocal = PKIX_TRUE;
        state->alreadyTriedAIA = PKIX_FALSE;
        state->revChecking = PKIX_FALSE;
        state->usingHintCerts = PKIX_FALSE;
        state->certLoopingDetected = PKIX_FALSE;

        PKIX_INCREF(validityDate);
        state->validityDate = validityDate;

        PKIX_INCREF(prevCert);
        state->prevCert = prevCert;

        state->candidateCert = NULL;

        PKIX_INCREF(traversedSubjNames);
        state->traversedSubjNames = traversedSubjNames;

        PKIX_INCREF(trustChain);
        state->trustChain = trustChain;

        state->aia = NULL;
        state->candidateCerts = NULL;
        state->reversedCertChain = NULL;
        state->checkedCritExtOIDs = NULL;
        state->checkerChain = NULL;
        state->revCheckers = NULL;
        state->certSel = NULL;
        state->verifyNode = NULL;
        state->client = NULL;

        PKIX_INCREF(parentState);
        state->parentState = parentState;

        if (parentState != NULL) {
                state->buildConstants.numAnchors =
                         parentState->buildConstants.numAnchors;
                state->buildConstants.numCertStores = 
                        parentState->buildConstants.numCertStores; 
                state->buildConstants.numHintCerts = 
                        parentState->buildConstants.numHintCerts; 
                state->buildConstants.maxFanout =
                        parentState->buildConstants.maxFanout;
                state->buildConstants.maxDepth =
                        parentState->buildConstants.maxDepth;
                state->buildConstants.maxTime =
                        parentState->buildConstants.maxTime;
                state->buildConstants.procParams = 
                        parentState->buildConstants.procParams; 
                state->buildConstants.testDate =
                        parentState->buildConstants.testDate;
                state->buildConstants.timeLimit =
                        parentState->buildConstants.timeLimit;
                state->buildConstants.targetCert =
                        parentState->buildConstants.targetCert;
                state->buildConstants.targetPubKey =
                        parentState->buildConstants.targetPubKey;
                state->buildConstants.certStores =
                        parentState->buildConstants.certStores;
                state->buildConstants.anchors =
                        parentState->buildConstants.anchors;
                state->buildConstants.userCheckers =
                        parentState->buildConstants.userCheckers;
                state->buildConstants.hintCerts =
                        parentState->buildConstants.hintCerts;
                state->buildConstants.crlChecker =
                        parentState->buildConstants.crlChecker;
                state->buildConstants.aiaMgr =
                        parentState->buildConstants.aiaMgr;
        }

        *pState = state;
        state = NULL;
cleanup:
        
        PKIX_DECREF(state);

        PKIX_RETURN(FORWARDBUILDERSTATE);
}

/*
 * FUNCTION: pkix_Build_GetResourceLimits
 *
 * DESCRIPTION:
 *  Retrieve Resource Limits from ProcessingParams and initialize them in
 *  BuildConstants.
 *
 * PARAMETERS
 *  "buildConstants"
 *      Address of a BuildConstants structure containing objects and values
 *      that remain constant throughout the building of a chain. Must be
 *      non-NULL.
 *  "plContext"
 *      Platform-specific context pointer.
 * THREAD SAFETY:
 *  Thread Safe (see Thread Safety Definitions in Programmer's Guide)
 * RETURNS:
 *  Returns NULL if the function succeeds.
 *  Returns a Build Error if the function fails in a non-fatal way.
 *  Returns a Fatal Error if the function fails in an unrecoverable way.
 */
static PKIX_Error *
pkix_Build_GetResourceLimits(
        BuildConstants *buildConstants,
        void *plContext)
{
        PKIX_ResourceLimits *resourceLimits = NULL;

        PKIX_ENTER(BUILD, "pkix_Build_GetResourceLimits");
        PKIX_NULLCHECK_ONE(buildConstants);

        PKIX_CHECK(PKIX_ProcessingParams_GetResourceLimits
                (buildConstants->procParams, &resourceLimits, plContext),
                PKIX_PROCESSINGPARAMSGETRESOURCELIMITSFAILED);

        buildConstants->maxFanout = 0;
        buildConstants->maxDepth = 0;
        buildConstants->maxTime = 0;

        if (resourceLimits) {

                PKIX_CHECK(PKIX_ResourceLimits_GetMaxFanout
                        (resourceLimits, &buildConstants->maxFanout, plContext),
                        PKIX_RESOURCELIMITSGETMAXFANOUTFAILED);

                PKIX_CHECK(PKIX_ResourceLimits_GetMaxDepth
                        (resourceLimits, &buildConstants->maxDepth, plContext),
                        PKIX_RESOURCELIMITSGETMAXDEPTHFAILED);

                PKIX_CHECK(PKIX_ResourceLimits_GetMaxTime
                        (resourceLimits, &buildConstants->maxTime, plContext),
                        PKIX_RESOURCELIMITSGETMAXTIMEFAILED);
        }

cleanup:

        PKIX_DECREF(resourceLimits);

        PKIX_RETURN(BUILD);
}

/*
 * FUNCTION: pkix_ForwardBuilderState_ToString
 * (see comments for PKIX_PL_ToStringCallback in pkix_pl_system.h)
 */
static PKIX_Error *
pkix_ForwardBuilderState_ToString
        (PKIX_PL_Object *object,
        PKIX_PL_String **pString,
        void *plContext)
{
        PKIX_ForwardBuilderState *state = NULL;
        PKIX_PL_String *formatString = NULL;
        PKIX_PL_String *resultString = NULL;
        PKIX_PL_String *buildStatusString = NULL;
        PKIX_PL_String *validityDateString = NULL;
        PKIX_PL_String *prevCertString = NULL;
        PKIX_PL_String *candidateCertString = NULL;
        PKIX_PL_String *traversedSubjNamesString = NULL;
        PKIX_PL_String *trustChainString = NULL;
        PKIX_PL_String *candidateCertsString = NULL;
        PKIX_PL_String *certSelString = NULL;
        PKIX_PL_String *verifyNodeString = NULL;
        PKIX_PL_String *parentStateString = NULL;
        char *asciiFormat = "\n"
                "\t{buildStatus: \t%s\n"
                "\ttraversedCACerts: \t%d\n"
                "\tcertStoreIndex: \t%d\n"
                "\tnumCerts: \t%d\n"
                "\tnumAias: \t%d\n"
                "\tcertIndex: \t%d\n"
                "\taiaIndex: \t%d\n"
                "\tnumFanout: \t%d\n"
                "\tnumDepth:  \t%d\n"
                "\treasonCode:  \t%d\n"
                "\tdsaParamsNeeded: \t%d\n"
                "\trevCheckDelayed: \t%d\n"
                "\tcanBeCached: \t%d\n"
                "\tuseOnlyLocal: \t%d\n"
                "\talreadyTriedAIA: \t%d\n"
                "\trevChecking: \t%d\n"
                "\tvalidityDate: \t%s\n"
                "\tprevCert: \t%s\n"
                "\tcandidateCert: \t%s\n"
                "\ttraversedSubjNames: \t%s\n"
                "\ttrustChain: \t%s\n"
                "\tcandidateCerts: \t%s\n"
                "\tcertSel: \t%s\n"
                "\tverifyNode: \t%s\n"
                "\tparentState: \t%s}\n";
        char *asciiStatus = NULL;

        PKIX_ENTER(FORWARDBUILDERSTATE, "pkix_ForwardBuilderState_ToString");
        PKIX_NULLCHECK_TWO(object, pString);

        PKIX_CHECK(pkix_CheckType
                (object, PKIX_FORWARDBUILDERSTATE_TYPE, plContext),
                PKIX_OBJECTNOTFORWARDBUILDERSTATE);

        state = (PKIX_ForwardBuilderState *)object;

        PKIX_CHECK(PKIX_PL_String_Create
                (PKIX_ESCASCII, asciiFormat, 0, &formatString, plContext),
                PKIX_STRINGCREATEFAILED);

        switch (state->status) {
            case BUILD_SHORTCUTPENDING: asciiStatus = "BUILD_SHORTCUTPENDING";
                                        break;
            case BUILD_INITIAL:         asciiStatus = "BUILD_INITIAL";
                                        break;
            case BUILD_TRYAIA:          asciiStatus = "BUILD_TRYAIA";
                                        break;
            case BUILD_AIAPENDING:      asciiStatus = "BUILD_AIAPENDING";
                                        break;
            case BUILD_COLLECTINGCERTS: asciiStatus = "BUILD_COLLECTINGCERTS";
                                        break;
            case BUILD_GATHERPENDING:   asciiStatus = "BUILD_GATHERPENDING";
                                        break;
            case BUILD_CERTVALIDATING:  asciiStatus = "BUILD_CERTVALIDATING";
                                        break;
            case BUILD_ABANDONNODE:     asciiStatus = "BUILD_ABANDONNODE";
                                        break;
            case BUILD_CRLPREP:         asciiStatus = "BUILD_CRLPREP";
                                        break;
            case BUILD_CRL1:            asciiStatus = "BUILD_CRL1";
                                        break;
            case BUILD_DATEPREP:        asciiStatus = "BUILD_DATEPREP";
                                        break;
            case BUILD_CHECKTRUSTED:    asciiStatus = "BUILD_CHECKTRUSTED";
                                        break;
            case BUILD_CHECKTRUSTED2:   asciiStatus = "BUILD_CHECKTRUSTED2";
                                        break;
            case BUILD_ADDTOCHAIN:      asciiStatus = "BUILD_ADDTOCHAIN";
                                        break;
            case BUILD_CHECKWITHANCHORS:asciiStatus = "BUILD_CHECKWITHANCHORS";
                                        break;
            case BUILD_CRL2PREP:        asciiStatus = "BUILD_CRL2PREP";
                                        break;
            case BUILD_CRL2:            asciiStatus = "BUILD_CRL2";
                                        break;
            case BUILD_VALCHAIN:        asciiStatus = "BUILD_VALCHAIN";
                                        break;
            case BUILD_VALCHAIN2:       asciiStatus = "BUILD_VALCHAIN2";
                                        break;
            case BUILD_EXTENDCHAIN:     asciiStatus = "BUILD_EXTENDCHAIN";
                                        break;
            case BUILD_GETNEXTCERT:     asciiStatus = "BUILD_GETNEXTCERT";
                                        break;
            default:                    asciiStatus = "INVALID STATUS";
                                        break;
        }

        PKIX_CHECK(PKIX_PL_String_Create
                (PKIX_ESCASCII, asciiStatus, 0, &buildStatusString, plContext),
                PKIX_STRINGCREATEFAILED);

        PKIX_TOSTRING
               (state->validityDate, &validityDateString, plContext,
                PKIX_OBJECTTOSTRINGFAILED);

        PKIX_TOSTRING
               (state->prevCert, &prevCertString, plContext,
                PKIX_OBJECTTOSTRINGFAILED);

        PKIX_TOSTRING
                (state->candidateCert, &candidateCertString, plContext,
                PKIX_OBJECTTOSTRINGFAILED);

        PKIX_TOSTRING
                (state->traversedSubjNames,
                &traversedSubjNamesString,
                plContext,
                PKIX_OBJECTTOSTRINGFAILED);

        PKIX_TOSTRING
                (state->trustChain, &trustChainString, plContext,
                PKIX_OBJECTTOSTRINGFAILED);

        PKIX_TOSTRING
                (state->candidateCerts, &candidateCertsString, plContext,
                PKIX_OBJECTTOSTRINGFAILED);

        PKIX_TOSTRING
                (state->certSel, &certSelString, plContext,
                PKIX_OBJECTTOSTRINGFAILED);

        PKIX_TOSTRING
                (state->verifyNode, &verifyNodeString, plContext,
                PKIX_OBJECTTOSTRINGFAILED);

        PKIX_TOSTRING
                (state->parentState, &parentStateString, plContext,
                PKIX_OBJECTTOSTRINGFAILED);

        PKIX_CHECK(PKIX_PL_Sprintf
                (&resultString,
                plContext,
                formatString,
                buildStatusString,
                (PKIX_Int32)state->traversedCACerts,
                (PKIX_UInt32)state->certStoreIndex,
                (PKIX_UInt32)state->numCerts,
                (PKIX_UInt32)state->numAias,
                (PKIX_UInt32)state->certIndex,
                (PKIX_UInt32)state->aiaIndex,
                (PKIX_UInt32)state->numFanout,
                (PKIX_UInt32)state->numDepth,
                (PKIX_UInt32)state->reasonCode,
                state->dsaParamsNeeded,
                state->revCheckDelayed,
                state->canBeCached,
                state->useOnlyLocal,
                state->alreadyTriedAIA,
                state->revChecking,
                validityDateString,
                prevCertString,
                candidateCertString,
                traversedSubjNamesString,
                trustChainString,
                candidateCertsString,
                certSelString,
                verifyNodeString,
                parentStateString),
                PKIX_SPRINTFFAILED);

        *pString = resultString;

cleanup:
        PKIX_DECREF(formatString);
        PKIX_DECREF(buildStatusString);
        PKIX_DECREF(validityDateString);
        PKIX_DECREF(prevCertString);
        PKIX_DECREF(candidateCertString);
        PKIX_DECREF(traversedSubjNamesString);
        PKIX_DECREF(trustChainString);
        PKIX_DECREF(candidateCertsString);
        PKIX_DECREF(certSelString);
        PKIX_DECREF(verifyNodeString);
        PKIX_DECREF(parentStateString);

        PKIX_RETURN(FORWARDBUILDERSTATE);

}

/*
 * FUNCTION: pkix_ForwardBuilderState_RegisterSelf
 *
 * DESCRIPTION:
 *  Registers PKIX_FORWARDBUILDERSTATE_TYPE and its related functions
 *  with systemClasses[]
 *
 * THREAD SAFETY:
 *  Not Thread Safe (see Thread Safety Definitions in Programmer's Guide)
 *
 *  Since this function is only called by PKIX_PL_Initialize, which should
 *  only be called once, it is acceptable that this function is not
 *  thread-safe.
 */
PKIX_Error *
pkix_ForwardBuilderState_RegisterSelf(void *plContext)
{

        extern pkix_ClassTable_Entry systemClasses[PKIX_NUMTYPES];
        pkix_ClassTable_Entry entry;

        PKIX_ENTER(FORWARDBUILDERSTATE,
                    "pkix_ForwardBuilderState_RegisterSelf");

        entry.description = "ForwardBuilderState";
        entry.objCounter = 0;
        entry.typeObjectSize = sizeof(PKIX_ForwardBuilderState);
        entry.destructor = pkix_ForwardBuilderState_Destroy;
        entry.equalsFunction = NULL;
        entry.hashcodeFunction = NULL;
        entry.toStringFunction = pkix_ForwardBuilderState_ToString;
        entry.comparator = NULL;
        entry.duplicateFunction = NULL;

        systemClasses[PKIX_FORWARDBUILDERSTATE_TYPE] = entry;

        PKIX_RETURN(FORWARDBUILDERSTATE);
}

#if PKIX_FORWARDBUILDERSTATEDEBUG
/*
 * FUNCTION: pkix_ForwardBuilderState_DumpState
 *
 * DESCRIPTION:
 *  This function invokes the ToString function on the argument pointed to
 *  by "state".
 * PARAMETERS:
 *  "state"
 *      The address of the ForwardBuilderState object. Must be non-NULL.
 *
 * THREAD SAFETY:
 *  Not Thread Safe (see Thread Safety Definitions in Programmer's Guide)
 */
PKIX_Error *
pkix_ForwardBuilderState_DumpState(
        PKIX_ForwardBuilderState *state,
        void *plContext)
{
        PKIX_PL_String *stateString = NULL;
        char *stateAscii = NULL;
        PKIX_UInt32 length;

        PKIX_ENTER(FORWARDBUILDERSTATE,"pkix_ForwardBuilderState_DumpState");
        PKIX_NULLCHECK_ONE(state);

        PKIX_CHECK(PKIX_PL_Object_InvalidateCache
                ((PKIX_PL_Object *)state, plContext),
                PKIX_OBJECTINVALIDATECACHEFAILED);

        PKIX_CHECK(PKIX_PL_Object_ToString
                ((PKIX_PL_Object*)state, &stateString, plContext),
                PKIX_OBJECTTOSTRINGFAILED);

        PKIX_CHECK(PKIX_PL_String_GetEncoded
                    (stateString,
                    PKIX_ESCASCII,
                    (void **)&stateAscii,
                    &length,
                    plContext),
                    PKIX_STRINGGETENCODEDFAILED);

        PKIX_DEBUG_ARG("In Phase 1: state = %s\n", stateAscii);

        PKIX_FREE(stateAscii);
        PKIX_DECREF(stateString);

cleanup:
        PKIX_RETURN(FORWARDBUILDERSTATE);
}
#endif

/*
 * FUNCTION: pkix_ForwardBuilderState_IsIOPending
 * DESCRIPTION:
 *
 *  This function determines whether the state of the ForwardBuilderState
 *  pointed to by "state" indicates I/O is in progress, and stores the Boolean
 *  result at "pPending".
 *
 * PARAMETERS:
 *  "state"
 *      The address of the ForwardBuilderState object. Must be non-NULL.
 *  "pPending"
 *      The address at which the result is stored. Must be non-NULL.
 *  "plContext"
 *      Platform-specific context pointer.
 * THREAD SAFETY:
 *  Thread Safe (see Thread Safety Definitions in Programmer's Guide)
 * RETURNS:
 *  Returns NULL if the function succeeds.
 *  Returns a ForwardBuilderState Error if the function fails in a
 *      non-fatal way.
 *  Returns a Fatal Error if the function fails in an unrecoverable way.
 */
static PKIX_Error*
pkix_ForwardBuilderState_IsIOPending(
        PKIX_ForwardBuilderState *state,
        PKIX_Boolean *pPending,
        void *plContext)
{
        PKIX_ENTER(FORWARDBUILDERSTATE, "pkix_ForwardBuilderState_IsIOPending");
        PKIX_NULLCHECK_TWO(state, pPending);

        if ((state->status == BUILD_GATHERPENDING) ||
            (state->status == BUILD_CRL1) ||
            (state->status == BUILD_CRL2) ||
            (state->status == BUILD_CHECKTRUSTED2) ||
            (state->status == BUILD_VALCHAIN2) ||
            (state->status == BUILD_AIAPENDING)) {
                *pPending = PKIX_TRUE;
        } else {
                *pPending = PKIX_FALSE;
        }

        PKIX_RETURN(FORWARDBUILDERSTATE);
}

/* --Private-BuildChain-Functions------------------------------------------- */

/*
 * FUNCTION: pkix_Build_CheckCertAgainstAnchor
 * DESCRIPTION:
 *
 *  Checks whether the Cert pointed to by "candidateCert" successfully chains to
 *  the TrustAnchor pointed to by "anchor". Successful chaining includes
 *  successful subject/issuer name chaining, using the List of traversed subject
 *  names pointed to by "traversedSubjNames" to check for name constraints
 *  violation, and successful signature verification. If the "candidateCert"
 *  successfully chains, PKIX_TRUE is stored at the address pointed to by
 *  "pPassed". Otherwise PKIX_FALSE is stored.
 *
 *  If a non-NULL VerifyNode is supplied, then this function will, in the event
 *  of a failure, set the Error associated with the failure in the VerifyNode.
 *  .
 *
 * PARAMETERS:
 *  "candidateCert"
 *      Address of Cert that is being checked. Must be non-NULL.
 *  "anchor"
 *      Address of TrustAnchor with which the Cert must successfully chain.
 *      Must be non-NULL.
 *  "traversedSubjNames"
 *      Address of List of subject names in certificates previously traversed.
 *      Must be non-NULL.
 *  "pPassed"
 *      Address at which Boolean result is stored. Must be non-NULL.
 *  "verifyNode"
 *      Address of the VerifyNode to receive the Error. May be NULL.
 *  "plContext"
 *      Platform-specific context pointer.
 * THREAD SAFETY:
 *  Thread Safe (see Thread Safety Definitions in Programmer's Guide)
 * RETURNS:
 *  Returns NULL if the function succeeds.
 *  Returns a Build Error if the function fails in a non-fatal way
 *  Returns a Fatal Error if the function fails in an unrecoverable way.
 */
static PKIX_Error *
pkix_Build_CheckCertAgainstAnchor(
        PKIX_PL_Cert *candidateCert,
        PKIX_TrustAnchor *anchor,
        PKIX_List *traversedSubjNames,
        PKIX_Boolean *pPassed,
        PKIX_VerifyNode *verifyNode,
        void *plContext)
{
        PKIX_PL_Cert *trustedCert = NULL;
        PKIX_PL_CertNameConstraints *anchorNC = NULL;
        PKIX_CertSelector *certSel = NULL;
        PKIX_ComCertSelParams *certSelParams = NULL;
        PKIX_PL_X500Name *trustedSubject = NULL;
        PKIX_PL_X500Name *candidateIssuer = NULL;
        PKIX_CertSelector_MatchCallback selectorMatch = NULL;
        PKIX_Boolean certMatch = PKIX_TRUE;
        PKIX_Boolean anchorMatch = PKIX_FALSE;
        PKIX_PL_PublicKey *trustedPubKey = NULL;
        PKIX_VerifyNode *anchorVerifyNode = NULL;
        PKIX_Error *verifyError = NULL;

        PKIX_ENTER(BUILD, "pkix_Build_CheckCertAgainstAnchor");
        PKIX_NULLCHECK_THREE(anchor, candidateCert, pPassed);

        *pPassed = PKIX_FALSE;

        PKIX_CHECK(PKIX_TrustAnchor_GetTrustedCert
                    (anchor, &trustedCert, plContext),
                    PKIX_TRUSTANCHORGETTRUSTEDCERTFAILED);

        PKIX_CHECK(PKIX_PL_Cert_GetSubject
                    (trustedCert, &trustedSubject, plContext),
                    PKIX_CERTGETSUBJECTFAILED);

        PKIX_NULLCHECK_ONE(trustedSubject);

        PKIX_CHECK(PKIX_PL_Cert_GetIssuer
                    (candidateCert, &candidateIssuer, plContext),
                    PKIX_CERTGETISSUERFAILED);

        PKIX_CHECK(PKIX_PL_X500Name_Match
                    (trustedSubject, candidateIssuer, &anchorMatch, plContext),
                    PKIX_X500NAMEMATCHFAILED);

        if (!anchorMatch) {
                goto cleanup;
        }

        PKIX_CHECK(PKIX_TrustAnchor_GetNameConstraints
                    (anchor, &anchorNC, plContext),
                    PKIX_TRUSTANCHORGETNAMECONSTRAINTSFAILED);

        if (anchorNC == NULL) {
                PKIX_CHECK(PKIX_CertSelector_Create
                            (NULL, NULL, &certSel, plContext),
                            PKIX_CERTSELECTORCREATEFAILED);

                PKIX_CHECK(PKIX_ComCertSelParams_Create
                            (&certSelParams, plContext),
                            PKIX_COMCERTSELPARAMSCREATEFAILED);

                PKIX_NULLCHECK_ONE(traversedSubjNames);

                PKIX_CHECK(PKIX_ComCertSelParams_SetPathToNames
                        (certSelParams, traversedSubjNames, plContext),
                        PKIX_COMCERTSELPARAMSSETPATHTONAMESFAILED);

                PKIX_CHECK(PKIX_CertSelector_SetCommonCertSelectorParams
                        (certSel, certSelParams, plContext),
                        PKIX_CERTSELECTORSETCOMMONCERTSELECTORPARAMSFAILED);

                PKIX_CHECK(PKIX_CertSelector_GetMatchCallback
                        (certSel, &selectorMatch, plContext),
                        PKIX_CERTSELECTORGETMATCHCALLBACKFAILED);

                PKIX_CHECK(selectorMatch
                        (certSel, candidateCert, &certMatch, plContext),
                        PKIX_SELECTORMATCHFAILED);

                if (!certMatch) {
                        goto cleanup;
                }

        }

        PKIX_CHECK(PKIX_PL_Cert_GetSubjectPublicKey
                (trustedCert, &trustedPubKey, plContext),
                PKIX_CERTGETSUBJECTPUBLICKEYFAILED);

        PKIX_CHECK(PKIX_PL_Cert_VerifySignature
                   (candidateCert, trustedPubKey, plContext),
                   PKIX_CERTVERIFYSIGNATUREFAILED);

cleanup:

        if (PKIX_ERROR_RECEIVED || !anchorMatch || !certMatch) {
                if (pkixErrorClass == PKIX_FATAL_ERROR) {
                        goto fatal;
                }
                if (verifyNode != NULL) {
                        if (!anchorMatch) {
                            PKIX_ERROR_CREATE
                                (BUILD,
                                PKIX_ANCHORDIDNOTCHAINTOCERT,
                                verifyError);
                        } else if (!certMatch) {
                            PKIX_ERROR_CREATE
                                (BUILD,
                                PKIX_ANCHORDIDNOTPASSCERTSELECTORCRITERIA,
                                verifyError);
                        } else {
                            verifyError = pkixErrorResult;
                            pkixErrorResult = NULL;
                        }
                        PKIX_DECREF(pkixErrorResult);
                }
        } else {
                *pPassed = PKIX_TRUE;
        }

        if (verifyNode != NULL) {
                PKIX_CHECK_FATAL(pkix_VerifyNode_Create
                        (trustedCert,
                        1,
                        verifyError,
                        &anchorVerifyNode,
                        plContext),
                        PKIX_VERIFYNODECREATEFAILED);
                PKIX_CHECK_FATAL(pkix_VerifyNode_AddToTree
                        (verifyNode, anchorVerifyNode, plContext),
                        PKIX_VERIFYNODEADDTOTREEFAILED);
                PKIX_DECREF(verifyError);
        }

fatal:
        PKIX_DECREF(verifyError);
        PKIX_DECREF(anchorVerifyNode);
        PKIX_DECREF(trustedCert);
        PKIX_DECREF(anchorNC);
        PKIX_DECREF(certSel);
        PKIX_DECREF(certSelParams);
        PKIX_DECREF(trustedSubject);
        PKIX_DECREF(trustedPubKey);
        PKIX_DECREF(candidateIssuer);

        PKIX_RETURN(BUILD);
}

/*
 * FUNCTION: pkix_Build_SortCertComparator
 * DESCRIPTION:
 *
 *  This Function takes two Certificates cast in "obj1" and "obj2",
 *  compares their validity NotAfter dates and returns the result at
 *  "pResult". The comparison key(s) can be expanded by using other
 *  data in the Certificate in the future.
 *
 * PARAMETERS:
 *  "obj1"
 *      Address of the PKIX_PL_Object that is a cast of PKIX_PL_Cert.
 *      Must be non-NULL.
 *  "obj2"
 *      Address of the PKIX_PL_Object that is a cast of PKIX_PL_Cert.
 *      Must be non-NULL.
 *  "pResult"
 *      Address where the comparison result is returned. Must be non-NULL.
 *  "plContext"
 *      Platform-specific context pointer.
 * THREAD SAFETY:
 *  Thread Safe (see Thread Safety Definitions in Programmer's Guide)
 * RETURNS:
 *  Returns NULL if the function succeeds.
 *  Returns a Build Error if the function fails in a non-fatal way
 *  Returns a Fatal Error if the function fails in an unrecoverable way.
 */
static PKIX_Error *
pkix_Build_SortCertComparator(
        PKIX_PL_Object *obj1,
        PKIX_PL_Object *obj2,
        PKIX_Int32 *pResult,
        void *plContext)
{
        PKIX_PL_Date *date1 = NULL;
        PKIX_PL_Date *date2 = NULL;
        PKIX_Boolean result = PKIX_FALSE;

        PKIX_ENTER(BUILD, "pkix_Build_SortCertComparator");
        PKIX_NULLCHECK_THREE(obj1, obj2, pResult);

        /*
         * For sorting candidate certificates, we use NotAfter date as the
         * sorted key for now (can be expanded if desired in the future).
         *
         * In PKIX_BuildChain, the List of CertStores was reordered so that
         * trusted CertStores are ahead of untrusted CertStores. That sort, or
         * this one, could be taken out if it is determined that it doesn't help
         * performance, or in some way hinders the solution of choosing desired
         * candidates.
         */

        PKIX_CHECK(pkix_CheckType(obj1, PKIX_CERT_TYPE, plContext),
                    PKIX_OBJECTNOTCERT);
        PKIX_CHECK(pkix_CheckType(obj2, PKIX_CERT_TYPE, plContext),
                    PKIX_OBJECTNOTCERT);

        PKIX_CHECK(PKIX_PL_Cert_GetValidityNotAfter
                ((PKIX_PL_Cert *)obj1, &date1, plContext),
                PKIX_CERTGETVALIDITYNOTAFTERFAILED);

        PKIX_CHECK(PKIX_PL_Cert_GetValidityNotAfter
                ((PKIX_PL_Cert *)obj2, &date2, plContext),
                PKIX_CERTGETVALIDITYNOTAFTERFAILED);
        
        PKIX_CHECK(PKIX_PL_Object_Compare
                ((PKIX_PL_Object *)date1,
                (PKIX_PL_Object *)date2,
                &result,
                plContext),
                PKIX_OBJECTCOMPARATORFAILED);

        *pResult = result;

cleanup:

        PKIX_DECREF(date1);
        PKIX_DECREF(date2);

        PKIX_RETURN(BUILD);
}

/* This local error check macro */
#define ERROR_CHECK(errCode) \
    if (pkixErrorResult) { \
        pkixTempErrorReceived = PKIX_TRUE; \
        pkixErrorClass = pkixErrorResult->errClass; \
        if (pkixErrorClass == PKIX_FATAL_ERROR) { \
            goto cleanup; \
        } \
        if (verifyNode) { \
            PKIX_INCREF(pkixErrorResult); \
            verifyNode->error = pkixErrorResult; \
        } \
        pkixErrorCode = errCode; \
        pkixErrorMsg = PKIX_ErrorText[errCode]; \
        goto cleanup; \
    }

/*
 * FUNCTION: pkix_Build_VerifyCertificate
 * DESCRIPTION:
 *
 *  Checks whether the previous Cert stored in the ForwardBuilderState pointed
 *  to by "state" successfully chains, including signature verification, to the
 *  candidate Cert also stored in "state", using the Boolean value in "trusted"
 *  to determine whether "candidateCert" is trusted. Using the Boolean value in
 *  "revocationChecking" for the existence of revocation checking, it sets
 *  "pNeedsCRLChecking" to PKIX_TRUE if the candidate Cert needs to be checked
 *  against Certificate Revocation Lists.
 *
 *  First it checks whether "candidateCert" has already been traversed by
 *  determining whether it is contained in the List of traversed Certs. It
 *  checks the candidate Cert with user checkers, if any, in the List pointed to
 *  by "userCheckers". It then runs the signature validation. Finally, it
 *  determines the appropriate value for "pNeedsCRLChecking".
 *
 *  If this Certificate fails verification, and state->verifyNode is non-NULL,
 *  this function sets the Error code into the verifyNode.
 *
 * PARAMETERS:
 *  "state"
 *      Address of ForwardBuilderState to be used. Must be non-NULL.
 *  "userCheckers"
 *      Address of a List of CertChainCheckers to be used, if present, to
 *      validate the candidateCert.
 *  "revocationChecking"
 *      Boolean indication of whether revocation checking is available, either
 *      as a CertChainChecker or a List of RevocationCheckers.
 *  "trusted"
 *      Boolean value of trust for the candidate Cert
 *  "pNeedsCRLChecking"
 *      Address where Boolean CRL-checking-needed value is stored.
 *      Must be non-NULL.
 *  "plContext"
 *      Platform-specific context pointer.
 * THREAD SAFETY:
 *  Thread Safe (see Thread Safety Definitions in Programmer's Guide)
 * RETURNS:
 *  Returns NULL if the function succeeds.
 *  Returns a Build Error if the function fails in a non-fatal way
 *  Returns a Fatal Error if the function fails in an unrecoverable way.
 */
static PKIX_Error *
pkix_Build_VerifyCertificate(
        PKIX_ForwardBuilderState *state,
        PKIX_List *userCheckers,
        PKIX_Boolean revocationChecking,
        PKIX_Boolean *pTrusted,
        PKIX_Boolean *pNeedsCRLChecking,
        PKIX_VerifyNode *verifyNode,
        void *plContext)
{
        PKIX_UInt32 numUserCheckers = 0;
        PKIX_UInt32 i = 0;
        PKIX_Boolean loopFound = PKIX_FALSE;
        PKIX_Boolean dsaParamsNeeded = PKIX_FALSE;
        PKIX_Boolean isSelfIssued = PKIX_FALSE;
        PKIX_Boolean supportForwardChecking = PKIX_FALSE;
        PKIX_Boolean trusted = PKIX_FALSE;
        PKIX_PL_Cert *candidateCert = NULL;
        PKIX_PL_PublicKey *candidatePubKey = NULL;
        PKIX_CertChainChecker *userChecker = NULL;
        PKIX_CertChainChecker_CheckCallback checkerCheck = NULL;
        void *nbioContext = NULL;

        PKIX_ENTER(BUILD, "pkix_Build_VerifyCertificate");
        PKIX_NULLCHECK_THREE(state, pTrusted, pNeedsCRLChecking);
        PKIX_NULLCHECK_THREE
                (state->candidateCerts, state->prevCert, state->trustChain);

        *pNeedsCRLChecking = PKIX_FALSE;

        PKIX_INCREF(state->candidateCert);
        candidateCert = state->candidateCert;

        PKIX_CHECK(PKIX_PL_Cert_IsCertTrusted
                (candidateCert, &trusted, plContext),
                PKIX_CERTISCERTTRUSTEDFAILED);

        *pTrusted = trusted;

        /* check for loops */
        PKIX_CHECK(pkix_List_Contains
                (state->trustChain,
                (PKIX_PL_Object *)candidateCert,
                &loopFound,
                plContext),
                PKIX_LISTCONTAINSFAILED);

        if (loopFound) {
                if (verifyNode != NULL) {
                        PKIX_Error *verifyError = NULL;
                        PKIX_ERROR_CREATE
                                (BUILD,
                                PKIX_LOOPDISCOVEREDDUPCERTSNOTALLOWED,
                                verifyError);
                        PKIX_DECREF(verifyNode->error);
                        verifyNode->error = verifyError;
                }
                /* Even if error logged, still need to abort
                 * if cert is not trusted. */
                if (!trusted) {
                        PKIX_ERROR(PKIX_LOOPDISCOVEREDDUPCERTSNOTALLOWED);
                }
                state->certLoopingDetected = PKIX_TRUE;
        }

        if (userCheckers != NULL) {

                PKIX_CHECK(PKIX_List_GetLength
                    (userCheckers, &numUserCheckers, plContext),
                    PKIX_LISTGETLENGTHFAILED);

                for (i = 0; i < numUserCheckers; i++) {

                    PKIX_CHECK(PKIX_List_GetItem
                        (userCheckers,
                        i,
                        (PKIX_PL_Object **) &userChecker,
                        plContext),
                        PKIX_LISTGETITEMFAILED);

                    PKIX_CHECK
                        (PKIX_CertChainChecker_IsForwardCheckingSupported
                        (userChecker, &supportForwardChecking, plContext),
                        PKIX_CERTCHAINCHECKERISFORWARDCHECKINGSUPPORTEDFAILED);

                    if (supportForwardChecking == PKIX_TRUE) {

                        PKIX_CHECK(PKIX_CertChainChecker_GetCheckCallback
                            (userChecker, &checkerCheck, plContext),
                            PKIX_CERTCHAINCHECKERGETCHECKCALLBACKFAILED);

                        pkixErrorResult =
                            checkerCheck(userChecker, candidateCert, NULL,
                                         &nbioContext, plContext);

                        ERROR_CHECK(PKIX_USERCHECKERCHECKFAILED);
                    }

                    PKIX_DECREF(userChecker);
                }
        }

        /* signature check */

        if ((!(state->dsaParamsNeeded)) || trusted) {
                PKIX_CHECK(PKIX_PL_Cert_GetSubjectPublicKey
                            (candidateCert, &candidatePubKey, plContext),
                            PKIX_CERTGETSUBJECTPUBLICKEYFAILED);

                PKIX_CHECK(PKIX_PL_PublicKey_NeedsDSAParameters
                            (candidatePubKey, &dsaParamsNeeded, plContext),
                            PKIX_PUBLICKEYNEEDSDSAPARAMETERSFAILED);

                if (dsaParamsNeeded) {
                        if (trusted) {
                                PKIX_ERROR(PKIX_MISSINGDSAPARAMETERS);
                        } else {
                                state->dsaParamsNeeded = PKIX_TRUE;
                                goto cleanup;
                        }
                }

                pkixErrorResult = PKIX_PL_Cert_VerifyKeyUsage
                        (candidateCert, PKIX_KEY_CERT_SIGN, plContext);

                ERROR_CHECK(PKIX_CERTVERIFYKEYUSAGEFAILED);

                pkixErrorResult = PKIX_PL_Cert_VerifySignature
                        (state->prevCert, candidatePubKey, plContext);

                ERROR_CHECK(PKIX_CERTVERIFYSIGNATUREFAILED);

                if (revocationChecking) {
                        if (!trusted) {
                            if (state->revCheckDelayed) {
                                goto cleanup;
                            } else {
                                PKIX_CHECK(pkix_IsCertSelfIssued
                                        (candidateCert,
                                        &isSelfIssued,
                                        plContext),
                                        PKIX_ISCERTSELFISSUEDFAILED);

                                if (isSelfIssued) {
                                        state->revCheckDelayed = PKIX_TRUE;
                                        goto cleanup;
                                }
                            }
                        }

                        pkixErrorResult = PKIX_PL_Cert_VerifyKeyUsage
                                (candidateCert, PKIX_KEY_CERT_SIGN, plContext);

                        ERROR_CHECK(PKIX_CERTVERIFYKEYUSAGEFAILED);

                        *pNeedsCRLChecking = PKIX_TRUE;
                }
        }

cleanup:

        PKIX_DECREF(candidateCert);
        PKIX_DECREF(candidatePubKey);
        PKIX_DECREF(userChecker);

        PKIX_RETURN(BUILD);
}

/*
 * FUNCTION: pkix_Build_ValidationCheckers
 * DESCRIPTION:
 *
 *  Creates a List of Objects to be used in determining whether the List of
 *  Certs pointed to by "certChain" successfully validates using the
 *  ForwardBuilderState pointed to by "state", and the TrustAnchor pointed to by
 *  "anchor". These objects are a reversed Cert Chain, consisting of the certs
 *  in "certChain" in reversed order, suitable for presenting to the
 *  CertChainCheckers; a List of critical extension OIDS that have already been
 *  processed in forward building; a List of CertChainCheckers to be called, and
 *  a List of RevocationCheckers to be called. These results are stored in
 *  fields of "state".
 *
 * PARAMETERS:
 *  "state"
 *      Address of ForwardBuilderState to be used. Must be non-NULL.
 *  "certChain"
 *      Address of List of Certs to be validated. Must be non-NULL.
 *  "anchor"
 *      Address of TrustAnchor to be used. Must be non-NULL.
 *  "plContext"
 *      Platform-specific context pointer.
 * THREAD SAFETY:
 *  Thread Safe (see Thread Safety Definitions in Programmer's Guide)
 * RETURNS:
 *  Returns NULL if the function succeeds.
 *  Returns a Build Error if the function fails in a non-fatal way
 *  Returns a Fatal Error if the function fails in an unrecoverable way.
 */
static PKIX_Error *
pkix_Build_ValidationCheckers(
        PKIX_ForwardBuilderState *state,
        PKIX_List *certChain,
        PKIX_TrustAnchor *anchor,
        void *plContext)
{
        PKIX_List *checkers = NULL;
        PKIX_List *initialPolicies = NULL;
        PKIX_List *reversedCertChain = NULL;
        PKIX_List *buildCheckedCritExtOIDsList = NULL;
        PKIX_ProcessingParams *procParams = NULL;
        PKIX_PL_Cert *trustedCert = NULL;
        PKIX_PL_PublicKey *trustedPubKey = NULL;
        PKIX_CertChainChecker *sigChecker = NULL;
        PKIX_CertChainChecker *crlChecker = NULL;
        PKIX_CertChainChecker *policyChecker = NULL;
        PKIX_CertChainChecker *userChecker = NULL;
        PKIX_RevocationChecker *revChecker = NULL;
        PKIX_List *userCheckersList = NULL;
        PKIX_List *userCheckerExtOIDs = NULL;
        PKIX_List *revCheckers = NULL;
        PKIX_PL_OID *oid = NULL;
        PKIX_Boolean supportForwardChecking = PKIX_FALSE;
        PKIX_Boolean policyQualifiersRejected = PKIX_FALSE;
        PKIX_Boolean initialPolicyMappingInhibit = PKIX_FALSE;
        PKIX_Boolean initialAnyPolicyInhibit = PKIX_FALSE;
        PKIX_Boolean initialExplicitPolicy = PKIX_FALSE;
        PKIX_UInt32 numChainCerts;
        PKIX_UInt32 numCertCheckers;
        PKIX_UInt32 i;

        PKIX_ENTER(BUILD, "pkix_Build_ValidationCheckers");
        PKIX_NULLCHECK_THREE(state, certChain, anchor);

        PKIX_CHECK(PKIX_List_Create(&checkers, plContext),
                PKIX_LISTCREATEFAILED);

        PKIX_CHECK(PKIX_List_ReverseList
                (certChain, &reversedCertChain, plContext),
                PKIX_LISTREVERSELISTFAILED);

        PKIX_CHECK(PKIX_List_GetLength
                (reversedCertChain, &numChainCerts, plContext),
                PKIX_LISTGETLENGTHFAILED);

        procParams = state->buildConstants.procParams;

        PKIX_CHECK(PKIX_ProcessingParams_GetInitialPolicies
                (procParams, &initialPolicies, plContext),
                PKIX_PROCESSINGPARAMSGETINITIALPOLICIESFAILED);

        PKIX_CHECK(PKIX_ProcessingParams_GetPolicyQualifiersRejected
                (procParams, &policyQualifiersRejected, plContext),
                PKIX_PROCESSINGPARAMSGETPOLICYQUALIFIERSREJECTEDFAILED);

        PKIX_CHECK(PKIX_ProcessingParams_IsPolicyMappingInhibited
                (procParams, &initialPolicyMappingInhibit, plContext),
                PKIX_PROCESSINGPARAMSISPOLICYMAPPINGINHIBITEDFAILED);

        PKIX_CHECK(PKIX_ProcessingParams_IsAnyPolicyInhibited
                (procParams, &initialAnyPolicyInhibit, plContext),
                PKIX_PROCESSINGPARAMSISANYPOLICYINHIBITEDFAILED);

        PKIX_CHECK(PKIX_ProcessingParams_IsExplicitPolicyRequired
                (procParams, &initialExplicitPolicy, plContext),
                PKIX_PROCESSINGPARAMSISEXPLICITPOLICYREQUIREDFAILED);

        PKIX_CHECK(pkix_PolicyChecker_Initialize
                (initialPolicies,
                policyQualifiersRejected,
                initialPolicyMappingInhibit,
                initialExplicitPolicy,
                initialAnyPolicyInhibit,
                numChainCerts,
                &policyChecker,
                plContext),
                PKIX_POLICYCHECKERINITIALIZEFAILED);

        PKIX_CHECK(PKIX_List_AppendItem
                (checkers, (PKIX_PL_Object *)policyChecker, plContext),
                PKIX_LISTAPPENDITEMFAILED);

        /*
         * Create an OID list that contains critical extensions processed
         * by BuildChain. These are specified in a static const array.
         */
        PKIX_CHECK(PKIX_List_Create(&buildCheckedCritExtOIDsList, plContext),
                PKIX_LISTCREATEFAILED);

        for (i = 0; buildCheckedCritExtOIDs[i] != NULL; i++) {
                PKIX_CHECK(PKIX_PL_OID_Create
                        (buildCheckedCritExtOIDs[i], &oid, plContext),
                        PKIX_OIDCREATEFAILED);

                PKIX_CHECK(PKIX_List_AppendItem
                        (buildCheckedCritExtOIDsList,
                        (PKIX_PL_Object *) oid,
                        plContext),
                        PKIX_LISTAPPENDITEMFAILED);

                PKIX_DECREF(oid);
        }

        if (state->buildConstants.userCheckers != NULL) {

                PKIX_CHECK(PKIX_List_GetLength
                        (state->buildConstants.userCheckers,
                        &numCertCheckers,
                        plContext),
                        PKIX_LISTGETLENGTHFAILED);

                for (i = 0; i < numCertCheckers; i++) {

                        PKIX_CHECK(PKIX_List_GetItem
                            (state->buildConstants.userCheckers,
                            i,
                            (PKIX_PL_Object **) &userChecker,
                            plContext),
                            PKIX_LISTGETITEMFAILED);

                        PKIX_CHECK
                            (PKIX_CertChainChecker_IsForwardCheckingSupported
                            (userChecker, &supportForwardChecking, plContext),
                            PKIX_CERTCHAINCHECKERGETSUPPORTEDEXTENSIONSFAILED);

                        /*
                         * If this userChecker supports forwardChecking then it
                         * should have been checked during build chain. Skip
                         * checking but need to add checker's extension OIDs
                         * to buildCheckedCritExtOIDsList.
                         */
                        if (supportForwardChecking == PKIX_TRUE) {

                          PKIX_CHECK
                            (PKIX_CertChainChecker_GetSupportedExtensions
                            (userChecker, &userCheckerExtOIDs, plContext),
                            PKIX_CERTCHAINCHECKERGETSUPPORTEDEXTENSIONSFAILED);

                          if (userCheckerExtOIDs != NULL) {
                            PKIX_CHECK(pkix_List_AppendList
                                (buildCheckedCritExtOIDsList,
                                userCheckerExtOIDs,
                                plContext),
                                PKIX_LISTAPPENDLISTFAILED);
                          }

                        } else {
                            PKIX_CHECK(PKIX_List_AppendItem
                                (checkers,
                                (PKIX_PL_Object *)userChecker,
                                plContext),
                                PKIX_LISTAPPENDITEMFAILED);
                        }

                        PKIX_DECREF(userCheckerExtOIDs);
                        PKIX_DECREF(userChecker);
                }
        }

        if (procParams->revCheckers) {
            PKIX_CHECK(
                PKIX_PL_Object_Duplicate(
                    (PKIX_PL_Object*)procParams->revCheckers,
                    (PKIX_PL_Object **)&revCheckers,
                    plContext),
                PKIX_LISTDUPLICATEFAILED);
        } else {
            PKIX_CHECK(PKIX_List_Create(&revCheckers, plContext),
                       PKIX_LISTCREATEFAILED);
        }

        if ((state->dsaParamsNeeded) || (state->revCheckDelayed)) {

                if ((state->dsaParamsNeeded) ||
                    (state->buildConstants.crlChecker)) {

                        PKIX_CHECK(PKIX_TrustAnchor_GetTrustedCert
                                (anchor, &trustedCert, plContext),
                                PKIX_TRUSTANCHORGETTRUSTEDCERTFAILED);

                        PKIX_CHECK(PKIX_PL_Cert_GetSubjectPublicKey
                                (trustedCert, &trustedPubKey, plContext),
                                PKIX_CERTGETSUBJECTPUBLICKEYFAILED);

                        PKIX_NULLCHECK_ONE(state->buildConstants.certStores);

#if 0
                        PKIX_CHECK(pkix_DefaultCRLChecker_Initialize
                                (state->buildConstants.certStores,
                                state->buildConstants.testDate,
                                trustedPubKey,
                                numChainCerts,
                                &crlChecker,
                                plContext),
                                PKIX_DEFAULTCRLCHECKERINITIALIZEFAILED);

                        PKIX_CHECK(PKIX_List_AppendItem
                                (checkers,
                                (PKIX_PL_Object *)crlChecker,
                                plContext),
                                PKIX_LISTAPPENDITEMFAILED);
#else
                        PKIX_CHECK(pkix_DefaultRevChecker_Initialize
                                (state->buildConstants.certStores,
                                state->buildConstants.testDate,
                                trustedPubKey,
                                numChainCerts,
                                &revChecker,
                                plContext),
                                PKIX_DEFAULTREVCHECKERINITIALIZEFAILED);

                        PKIX_CHECK(PKIX_List_AppendItem
                                (revCheckers,
                                (PKIX_PL_Object *)revChecker,
                                plContext),
                                PKIX_LISTAPPENDITEMFAILED);

#endif

                        if (state->dsaParamsNeeded) {

                                PKIX_CHECK(pkix_SignatureChecker_Initialize
                                        (trustedPubKey,
                                        numChainCerts,
                                        &sigChecker,
                                        plContext),
                                        PKIX_SIGNATURECHECKERINITIALIZEFAILED);

                                PKIX_CHECK(PKIX_List_AppendItem
                                        (checkers,
                                        (PKIX_PL_Object *)sigChecker,
                                        plContext),
                                        PKIX_LISTAPPENDITEMFAILED);
                        }
                }
        }

        PKIX_INCREF(reversedCertChain);
        state->reversedCertChain = reversedCertChain;
        PKIX_INCREF(buildCheckedCritExtOIDsList);
        state->checkedCritExtOIDs = buildCheckedCritExtOIDsList;
        PKIX_INCREF(checkers);
        state->checkerChain = checkers;
        PKIX_INCREF(revCheckers);
        state->revCheckers = revCheckers;
        state->certCheckedIndex = 0;
        state->checkerIndex = 0;
        state->revChecking = PKIX_FALSE;


cleanup:

        PKIX_DECREF(reversedCertChain);
        PKIX_DECREF(buildCheckedCritExtOIDsList);
        PKIX_DECREF(checkers);
        PKIX_DECREF(revCheckers);
        PKIX_DECREF(revChecker);
        PKIX_DECREF(initialPolicies);
        PKIX_DECREF(trustedCert);
        PKIX_DECREF(trustedPubKey);
        PKIX_DECREF(sigChecker);
        PKIX_DECREF(crlChecker);
        PKIX_DECREF(policyChecker);
        PKIX_DECREF(userChecker);
        PKIX_DECREF(userCheckersList);

        PKIX_RETURN(BUILD);
}

/*
 * FUNCTION: pkix_Build_ValidateEntireChain
 * DESCRIPTION:
 *
 *  Checks whether the current List of Certs successfully validates using the 
 *  TrustAnchor pointed to by "anchor" and other parameters contained, as was
 *  the Cert List, in "state".
 *
 *  If a checker using non-blocking I/O returns with a non-NULL non-blocking I/O
 *  context (NBIOContext), an indication that I/O is in progress and the
 *  checking has not been completed, this function stores that context at
 *  "pNBIOContext". Otherwise, it stores NULL at "pNBIOContext".
 *
 *  If not awaiting I/O and if successful, a ValidateResult is created
 *  containing the Public Key of the target certificate (including DSA parameter
 *  inheritance, if any) and the PolicyNode representing the policy tree output
 *  by the validation algorithm.  If not successful, an Error pointer is
 *  returned.
 *
 * PARAMETERS:
 *  "state"
 *      Address of ForwardBuilderState to be used. Must be non-NULL.
 *  "anchor"
 *      Address of TrustAnchor to be used. Must be non-NULL.
 *  "pNBIOContext"
 *      Address at which the NBIOContext is stored indicating whether the
 *      validation is complete. Must be non-NULL.
 *  "pValResult"
 *      Address at which the ValidateResult is stored. Must be non-NULL.
 *  "plContext"
 *      Platform-specific context pointer.
 * THREAD SAFETY:
 *  Thread Safe (see Thread Safety Definitions in Programmer's Guide)
 * RETURNS:
 *  Returns NULL if the function succeeds.
 *  Returns a Build Error if the function fails in a non-fatal way
 *  Returns a Fatal Error if the function fails in an unrecoverable way.
 */
static PKIX_Error *
pkix_Build_ValidateEntireChain(
        PKIX_ForwardBuilderState *state,
        PKIX_TrustAnchor *anchor,
        void **pNBIOContext,
        PKIX_ValidateResult **pValResult,
        PKIX_VerifyNode *verifyNode,
        void *plContext)
{
        PKIX_UInt32 numChainCerts = 0;
        PKIX_PL_PublicKey *subjPubKey = NULL;
        PKIX_PolicyNode *policyTree = NULL;
        PKIX_ValidateResult *valResult = NULL;
        void *nbioContext = NULL;

        PKIX_ENTER(BUILD, "pkix_Build_ValidateEntireChain");
        PKIX_NULLCHECK_FOUR(state, anchor, pNBIOContext, pValResult);

        *pNBIOContext = NULL; /* prepare for case of error exit */

        PKIX_CHECK(PKIX_List_GetLength
                (state->reversedCertChain, &numChainCerts, plContext),
                PKIX_LISTGETLENGTHFAILED);

        pkixErrorResult =
            pkix_CheckChain(state->reversedCertChain, numChainCerts,
                            state->checkerChain, state->revCheckers,
                            state->checkedCritExtOIDs,
                            state->buildConstants.procParams,
                            &state->certCheckedIndex, &state->checkerIndex,
                            &state->revChecking, &state->reasonCode,
                            &nbioContext, &subjPubKey, &policyTree, NULL,
                            plContext);

        if (nbioContext != NULL) {
                *pNBIOContext = nbioContext;
                goto cleanup;
        }

        ERROR_CHECK(PKIX_CHECKCHAINFAILED);

        if (state->reasonCode != 0) {
                PKIX_ERROR(PKIX_CHAINREJECTEDBYREVOCATIONCHECKER);
        }

        if (state->dsaParamsNeeded == PKIX_FALSE) {
                PKIX_INCREF(state->buildConstants.targetPubKey);
                subjPubKey = state->buildConstants.targetPubKey;
        }

        PKIX_CHECK(pkix_ValidateResult_Create
                (subjPubKey, anchor, policyTree, &valResult, plContext),
                PKIX_VALIDATERESULTCREATEFAILED);

        *pValResult = valResult;
        valResult = NULL;

cleanup:
        PKIX_DECREF(subjPubKey);
        PKIX_DECREF(policyTree);
        PKIX_DECREF(valResult);

        PKIX_RETURN(BUILD);
}

/*
 * FUNCTION: pkix_Build_SortCandidateCerts
 * DESCRIPTION:
 *
 *  This function sorts a List of candidate Certs pointed to by "candidates"
 *  using an algorithm that places Certs most likely to produce a successful
 *  chain at the front of the list, storing the resulting sorted List at
 *  "pSortedCandidates".
 *
 *  At present the only sort criterion is that trusted Certs go ahead of
 *  untrusted Certs.
 *
 * PARAMETERS:
 *  "candidates"
 *      Address of List of Candidate Certs to be sorted. Must be non-NULL.
 *  "pSortedCandidates"
 *      Address at which sorted List is stored. Must be non-NULL.
 *  "plContext"
 *      Platform-specific context pointer.
 * THREAD SAFETY:
 *  Thread Safe (see Thread Safety Definitions in Programmer's Guide)
 * RETURNS:
 *  Returns NULL if the function succeeds.
 *  Returns a Build Error if the function fails in a non-fatal way
 *  Returns a Fatal Error if the function fails in an unrecoverable way.
 */
static PKIX_Error *
pkix_Build_SortCandidateCerts(
        PKIX_List *candidates,
        PKIX_List **pSortedCandidates,
        void *plContext)
{
        PKIX_List *sortedList = NULL;

        PKIX_ENTER(BUILD, "pkix_Build_SortCandidateCerts");
        PKIX_NULLCHECK_TWO(candidates, pSortedCandidates);

        /*
         * Both bubble and quick sort algorithms are available.
         * For a list of fewer than around 100 items, the bubble sort is more
         * efficient. (This number was determined by experimenting with both
         * algorithms on a Java List.)
         * If the candidate list is very small, using the sort can drag down
         * the performance a little bit.
         */

        PKIX_CHECK(pkix_List_BubbleSort
                (candidates,
                pkix_Build_SortCertComparator,
                &sortedList,
                plContext),
                PKIX_LISTBUBBLESORTFAILED);

        *pSortedCandidates = sortedList;

cleanup:

        PKIX_RETURN(BUILD);
}

/*
 * FUNCTION: pkix_Build_BuildSelectorAndParams
 * DESCRIPTION:
 *
 *  This function creates a CertSelector, initialized with an appropriate
 *  ComCertSelParams, using the variables provided in the ForwardBuilderState
 *  pointed to by "state". The CertSelector created is stored in the certsel
 *  element of "state".
 *
 * PARAMETERS:
 *  "state"
 *      Address of ForwardBuilderState to be used. Must be non-NULL.
 *  "plContext"
 *      Platform-specific context pointer.
 * THREAD SAFETY:
 *  Thread Safe (see Thread Safety Definitions in Programmer's Guide)
 * RETURNS:
 *  Returns NULL if the function succeeds.
 *  Returns a Build Error if the function fails in a non-fatal way
 *  Returns a Fatal Error if the function fails in an unrecoverable way.
 */
static PKIX_Error *
pkix_Build_BuildSelectorAndParams(
        PKIX_ForwardBuilderState *state,
        void *plContext)
{
        PKIX_ComCertSelParams *certSelParams = NULL;
        PKIX_CertSelector *certSel = NULL;
        PKIX_PL_X500Name *currentIssuer = NULL;
        PKIX_PL_Date *testDate = NULL;

        PKIX_ENTER(BUILD, "pkix_Build_BuildSelectorAndParams");
        PKIX_NULLCHECK_THREE(state, state->prevCert, state->traversedSubjNames);

        PKIX_CHECK(PKIX_PL_Cert_GetIssuer
                (state->prevCert, &currentIssuer, plContext),
                PKIX_CERTGETISSUERFAILED);

        PKIX_CHECK(PKIX_ComCertSelParams_Create(&certSelParams, plContext),
                PKIX_COMCERTSELPARAMSCREATEFAILED);

        PKIX_CHECK(PKIX_ComCertSelParams_SetSubject
                (certSelParams, currentIssuer, plContext),
                PKIX_COMCERTSELPARAMSSETSUBJECTFAILED);

        PKIX_INCREF(state->buildConstants.testDate);
        testDate = state->buildConstants.testDate;

        PKIX_CHECK(PKIX_ComCertSelParams_SetCertificateValid
                (certSelParams, testDate, plContext),
                PKIX_COMCERTSELPARAMSSETCERTIFICATEVALIDFAILED);

        PKIX_CHECK(PKIX_ComCertSelParams_SetBasicConstraints
                (certSelParams, state->traversedCACerts, plContext),
                PKIX_COMCERTSELPARAMSSETBASICCONSTRAINTSFAILED);

        PKIX_CHECK(PKIX_ComCertSelParams_SetPathToNames
                (certSelParams, state->traversedSubjNames, plContext),
                PKIX_COMCERTSELPARAMSSETPATHTONAMESFAILED);

        PKIX_CHECK(PKIX_CertSelector_Create
                (NULL, NULL, &state->certSel, plContext),
                PKIX_CERTSELECTORCREATEFAILED);

        PKIX_CHECK(PKIX_CertSelector_SetCommonCertSelectorParams
                (state->certSel, certSelParams, plContext),
                PKIX_CERTSELECTORSETCOMMONCERTSELECTORPARAMSFAILED);

        PKIX_CHECK(PKIX_List_Create(&state->candidateCerts, plContext),
                PKIX_LISTCREATEFAILED);

        state->certStoreIndex = 0;

cleanup:
        PKIX_DECREF(certSelParams);
        PKIX_DECREF(certSel);
        PKIX_DECREF(currentIssuer);
        PKIX_DECREF(testDate);

        PKIX_RETURN(BUILD);
}

/*
 * FUNCTION: pkix_Build_CombineWithTrust
 * DESCRIPTION:
 *
 *  Adds each Cert in the List pointed to by "fromList" to the List pointed
 *  to by "toList", if it is not already a member of that List. If it is a
 *  member of both Lists, then the two instances are checked to see if either
 *  is trusted, in which case the trusted one is retained. In other words,
 *  "toList" becomes the union of the two sets, with trust preserved.
 *
 *  It is assumed that fromList does not contain duplicates. Therefore as
 *  elements of "fromlist" are added to "tolist", subsequent additions do
 *  not need to be checked for equality against these new members.
 *
 * PARAMETERS:
 *  "fromList"
 *      Address of a List of Certs to be added, if not already present, to
 *      "toList". Must be non-NULL, but may be empty.
 *  "toList"
 *      Address of a List of Certs to be augmented by "fromList". Must be
 *      non-NULL, but may be empty.
 *  "plContext"
 *      Platform-specific context pointer.
 * THREAD SAFETY:
 *  Not Thread Safe - assumes exclusive access to "toList"
 *  (see Thread Safety Definitions in Programmer's Guide)
 * RETURNS:
 *  Returns NULL if the function succeeds
 *  Returns a Build Error if the function fails in a non-fatal way.
 *  Returns a Fatal Error if the function fails in an unrecoverable way
 */
static PKIX_Error *
pkix_Build_CombineWithTrust(
        PKIX_List *toList,
        PKIX_List *fromList,
        void *plContext)
{
        PKIX_Boolean match = PKIX_FALSE;
        PKIX_Boolean trusted = PKIX_FALSE;
        PKIX_UInt32 fromlistLen = 0;
        PKIX_UInt32 originalTolistLen = 0;
        PKIX_UInt32 fromlistIx = 0;
        PKIX_UInt32 tolistIx = 0;
        PKIX_PL_Object *fObject = NULL;
        PKIX_PL_Object *tObject = NULL;

        PKIX_ENTER(BUILD, "pkix_Build_CombineWithTrust");
        PKIX_NULLCHECK_TWO(fromList, toList);

        PKIX_CHECK(PKIX_List_GetLength(fromList, &fromlistLen, plContext),
                PKIX_LISTGETLENGTHFAILED);

        PKIX_CHECK(PKIX_List_GetLength(toList, &originalTolistLen, plContext),
                PKIX_LISTGETLENGTHFAILED);

        for (fromlistIx = 0; fromlistIx < fromlistLen; fromlistIx++) {

                PKIX_CHECK(PKIX_List_GetItem
                        (fromList, fromlistIx, &fObject, plContext),
                        PKIX_LISTGETITEMFAILED);

                PKIX_NULLCHECK_ONE(fObject);

                match = PKIX_FALSE;
                for (tolistIx = 0; tolistIx < originalTolistLen; tolistIx++) {
                        PKIX_CHECK(PKIX_List_GetItem
                                (toList, tolistIx, &tObject, plContext),
                                PKIX_LISTGETITEMFAILED);

                        PKIX_NULLCHECK_ONE(tObject);

                        PKIX_CHECK(PKIX_PL_Object_Equals
                                (fObject, tObject, &match, plContext),
                                PKIX_OBJECTEQUALSFAILED);

                        if (match) {
                                PKIX_CHECK(pkix_CheckType
                                        (tObject, PKIX_CERT_TYPE, plContext),
                                        PKIX_OBJECTNOTCERT);

                                PKIX_CHECK(PKIX_PL_Cert_IsCertTrusted
                                        ((PKIX_PL_Cert *)tObject, &trusted,
                                         plContext),
                                        PKIX_CERTISCERTTRUSTEDFAILED);
        
                                /* If tObject is a trusted cert, keep it. */
                                if (trusted == PKIX_TRUE) {
                                        PKIX_DECREF(tObject);
                                        break;
                                }

                                PKIX_CHECK(pkix_CheckType
                                        (fObject, PKIX_CERT_TYPE, plContext),
                                        PKIX_OBJECTNOTCERT);

                                PKIX_CHECK(PKIX_PL_Cert_IsCertTrusted
                                        ((PKIX_PL_Cert *)fObject, &trusted,
                                         plContext),
                                        PKIX_CERTISCERTTRUSTEDFAILED);

                                /* If fObject is a trusted cert, replace it. */
                                if (trusted == PKIX_TRUE) {
                                        PKIX_CHECK(PKIX_List_SetItem
                                                (toList,
                                                tolistIx,
                                                fObject,
                                                plContext),
                                                PKIX_LISTSETITEMFAILED);
                                        PKIX_DECREF(tObject);
                                        break;
                                }
                        }
                        PKIX_DECREF(tObject);
                }

                if (match == PKIX_FALSE) {
                        PKIX_CHECK(PKIX_List_AppendItem
                                (toList, fObject, plContext),
                                PKIX_LISTAPPENDITEMFAILED);
                }

                PKIX_DECREF(fObject);
        }

cleanup:

        PKIX_DECREF(fObject);
        PKIX_DECREF(tObject);

        PKIX_RETURN(BUILD);
}

/*
 * FUNCTION: pkix_Build_GatherCerts
 * DESCRIPTION:
 *
 *  This function traverses the CertStores in the List of CertStores contained
 *  in "state",  using the certSelector and other parameters contained in
 *  "state", to obtain a List of all available Certs that satisfy the criteria.
 *  If a CertStore has a cache, "certSelParams" is used both to query the cache
 *  and, if an actual CertStore search occurred, to update the cache. (Behavior
 *  is undefined if "certSelParams" is different from the parameters that were
 *  used to initialize the certSelector in "state".)
 * 
 *  If a CertStore using non-blocking I/O returns with an indication that I/O is
 *  in progress and the checking has not been completed, this function stores
 *  platform-dependent information at "pNBIOContext". Otherwise it stores NULL
 *  at "pNBIOContext", and state is updated with the results of the search.
 *
 * PARAMETERS:
 *  "state"
 *      Address of ForwardBuilderState to be used. Must be non-NULL.
 *  "certSelParams"
 *      Address of ComCertSelParams which were used in creating the current
 *      CertSelector, and to be used in querying and updating any caches that
 *      may be associated with with the CertStores.
 *  "pNBIOContext"
 *      Address at which platform-dependent information is returned if request
 *      is suspended for non-blocking I/O. Must be non-NULL.
 *  "plContext"
 *      Platform-specific context pointer.
 * THREAD SAFETY:
 *  Thread Safe (see Thread Safety Definitions in Programmer's Guide)
 * RETURNS:
 *  Returns NULL if the function succeeds.
 *  Returns a Build Error if the function fails in a non-fatal way
 *  Returns a Fatal Error if the function fails in an unrecoverable way.
 */
/* return NULL if wouldblock, empty list if none found, else list of found */
static PKIX_Error *
pkix_Build_GatherCerts(
        PKIX_ForwardBuilderState *state,
        PKIX_ComCertSelParams *certSelParams,
        void **pNBIOContext,
        void *plContext)
{
        PKIX_Boolean certStoreIsCached = PKIX_FALSE;
        PKIX_Boolean certStoreCanBeUsed = PKIX_FALSE;
        PKIX_Boolean foundInCache = PKIX_FALSE;
        PKIX_CertStore *certStore = NULL;
        PKIX_CertStore_CertCallback getCerts = NULL;
        PKIX_List *certsFound = NULL;
        PKIX_List *sorted = NULL;
        void *nbioContext = NULL;

        PKIX_ENTER(BUILD, "pkix_Build_GatherCerts");
        PKIX_NULLCHECK_THREE(state, certSelParams, pNBIOContext);

        nbioContext = *pNBIOContext;
        *pNBIOContext = NULL;

        while (state->certStoreIndex < state->buildConstants.numCertStores) {

                /* Get the current CertStore */
                PKIX_CHECK(PKIX_List_GetItem
                        (state->buildConstants.certStores,
                        state->certStoreIndex,
                        (PKIX_PL_Object **)&certStore,
                        plContext),
                        PKIX_LISTGETITEMFAILED);

                if ((state->useOnlyLocal) == PKIX_FALSE) {
                    certStoreCanBeUsed = PKIX_TRUE;
                } else {
                    PKIX_CHECK(PKIX_CertStore_GetLocalFlag
                            (certStore, &certStoreCanBeUsed, plContext),
                            PKIX_CERTSTOREGETLOCALFLAGFAILED);
                }

                if (certStoreCanBeUsed == PKIX_TRUE) {
                    /* If GATHERPENDING, we've already checked the cache */
                    if (state->status == BUILD_GATHERPENDING) {
                        certStoreIsCached = PKIX_FALSE;
                        foundInCache = PKIX_FALSE;
                    } else {
                        PKIX_CHECK(PKIX_CertStore_GetCertStoreCacheFlag
                                (certStore, &certStoreIsCached, plContext),
                                PKIX_CERTSTOREGETCERTSTORECACHEFLAGFAILED);

                        if (certStoreIsCached) {
                        /*
                         * Look for Certs in the cache, using the SubjectName as
                         * the key. Then the ComCertSelParams are used to filter
                         * for qualified certs. If none are found, then the
                         * certStores are queried. When we eventually add items
                         * to the cache, we will only add items that passed the
                         * ComCertSelParams filter, rather than all Certs which
                         * matched the SubjectName.
                         */

                                PKIX_CHECK(pkix_CacheCert_Lookup
                                        (certStore,
                                        certSelParams,
                                        state->buildConstants.testDate,
                                        &foundInCache,
                                        &certsFound,
                                        plContext),
                                        PKIX_CACHECERTCHAINLOOKUPFAILED);

                        }
                    }

                    /*
                     * XXX need to verify if Cert is trusted, hence may not
                     * be worth it to have the Cert Cached or
                     * If it is trusted, don't cache, but once there is cached
                     * certs, we won't get certs from database any more.
                     * can use flag to force not getting certs from cache
                     */
                    if (!foundInCache) {

                        if (nbioContext == NULL) {
                                PKIX_CHECK(PKIX_CertStore_GetCertCallback
                                        (certStore, &getCerts, plContext),
                                        PKIX_CERTSTOREGETCERTCALLBACKFAILED);

                                PKIX_CHECK(getCerts
                                        (certStore,
                                        state->certSel,
                                        &nbioContext,
                                        &certsFound,
                                        plContext),
                                        PKIX_GETCERTSFAILED);
                        } else {
                                PKIX_CHECK(PKIX_CertStore_CertContinue
                                        (certStore,
                                        state->certSel,
                                        &nbioContext,
                                        &certsFound,
                                        plContext),
                                        PKIX_CERTSTORECERTCONTINUEFAILED);
                        }

                        if (certStoreIsCached && certsFound) {

                                PKIX_CHECK(pkix_CacheCert_Add
                                        (certStore,
                                        certSelParams,
                                        certsFound,
                                        plContext),
                                        PKIX_CACHECERTADDFAILED);
                        }
                    }

                    /*
                     * getCerts returns an empty list for "NONE FOUND",
                     * a NULL list for "would block"
                     */
                    if (certsFound == NULL) {
                        state->status = BUILD_GATHERPENDING;
                        *pNBIOContext = nbioContext;
                        goto cleanup;
                    } else {
                        PKIX_CHECK(pkix_Build_CombineWithTrust
                                (state->candidateCerts, certsFound, plContext),
                                PKIX_BUILDCOMBINEWITHTRUSTFAILED);
                        PKIX_DECREF(certsFound);
                    }
                }

                /* Are there any more certStores to query? */
                PKIX_DECREF(certStore);
                ++(state->certStoreIndex);
        }

        /* No, return the list we have gathered */
        PKIX_CHECK(PKIX_List_GetLength
                (state->candidateCerts, &state->numCerts, plContext),
                PKIX_LISTGETLENGTHFAILED);

        if (state->numCerts > 1) {
                /* sort Certs to try to optimize search */
                PKIX_CHECK(pkix_Build_SortCandidateCerts
                        (state->candidateCerts, &sorted, plContext),
                        PKIX_BUILDSORTCANDIDATECERTSFAILED);

                PKIX_DECREF(state->candidateCerts);
                state->candidateCerts = sorted;
                sorted = NULL;
        }

        state->certIndex = 0;

cleanup:

        PKIX_DECREF(certStore);
        PKIX_DECREF(certsFound);

        PKIX_RETURN(BUILD);
}

/*
 * FUNCTION: pkix_Build_UpdateDate
 * DESCRIPTION:
 *
 *  This function updates the validityDate contained in "state", for the current
 *  CertChain contained in "state", to include the validityDate of the
 *  candidateCert contained in "state". The validityDate of a chain is the
 *  earliest of all the notAfter dates contained in the respective Certificates.
 *
 * PARAMETERS:
 *  "state"
 *      Address of ForwardBuilderState to be used. Must be non-NULL.
 *  "plContext"
 *      Platform-specific context pointer.
 * THREAD SAFETY:
 *  Thread Safe (see Thread Safety Definitions in Programmer's Guide)
 * RETURNS:
 *  Returns NULL if the function succeeds.
 *  Returns a Build Error if the function fails in a non-fatal way
 *  Returns a Fatal Error if the function fails in an unrecoverable way.
 */
static PKIX_Error *
pkix_Build_UpdateDate(
        PKIX_ForwardBuilderState *state,
        void *plContext)
{
        PKIX_Boolean canBeCached = PKIX_FALSE;
        PKIX_Int32 comparison = 0;
        PKIX_PL_Date *notAfter = NULL;

        PKIX_ENTER(BUILD, "pkix_Build_UpdateDate");
        PKIX_NULLCHECK_ONE(state);

        PKIX_CHECK(PKIX_PL_Cert_GetCacheFlag
                (state->candidateCert, &canBeCached, plContext),
                PKIX_CERTGETCACHEFLAGFAILED);

        state->canBeCached = state->canBeCached && canBeCached;
        if (state->canBeCached == PKIX_TRUE) {

                /*
                 * So far, all certs can be cached. Update cert
                 * chain validity time, which is the earliest of
                 * all certs' notAfter times.
                 */
                PKIX_CHECK(PKIX_PL_Cert_GetValidityNotAfter
                        (state->candidateCert, &notAfter, plContext),
                        PKIX_CERTGETVALIDITYNOTAFTERFAILED);

                if (state->validityDate == NULL) {
                        state->validityDate = notAfter;
                        notAfter = NULL;
                } else {
                        PKIX_CHECK(PKIX_PL_Object_Compare
                                ((PKIX_PL_Object *)state->validityDate,
                                (PKIX_PL_Object *)notAfter,
                                &comparison,
                                plContext),
                                PKIX_OBJECTCOMPARATORFAILED);
                        if (comparison > 0) {
                                PKIX_DECREF(state->validityDate);
                                state->validityDate = notAfter;
                                notAfter = NULL;
                        }
                }
        }

cleanup:

        PKIX_DECREF(notAfter);

        PKIX_RETURN(BUILD);
}

/*
 * FUNCTION: pkix_Build_RevCheckPrep
 * DESCRIPTION:
 *
 *  This function prepares the CertChainCheckerState of the crlChecker contained
 *  in "state" for checking the Certificate pointed to by "certToCheck".
 *
 * PARAMETERS:
 *  "state"
 *      Address of ForwardBuilderState to be used. Must be non-NULL.
 *  "certToCheck"
 *      Address of Certificate to be checked. Must be non-NULL.
 *  "plContext"
 *      Platform-specific context pointer.
 * THREAD SAFETY:
 *  Thread Safe (see Thread Safety Definitions in Programmer's Guide)
 * RETURNS:
 *  Returns NULL if the function succeeds.
 *  Returns a Build Error if the function fails in a non-fatal way
 *  Returns a Fatal Error if the function fails in an unrecoverable way.
 */
static PKIX_Error *
pkix_Build_RevCheckPrep(
        PKIX_ForwardBuilderState *state,
        PKIX_PL_Cert *certToCheck,
        void *plContext)
{
        PKIX_PL_Object *crlCheckerState = NULL;

        PKIX_ENTER(BUILD, "pkix_Build_RevCheckPrep");
        PKIX_NULLCHECK_ONE(state);

        PKIX_CHECK(PKIX_CertChainChecker_GetCertChainCheckerState
                (state->buildConstants.crlChecker, &crlCheckerState, plContext),
                PKIX_CERTCHAINCHECKERGETCERTCHAINCHECKERSTATEFAILED);

        PKIX_CHECK(pkix_CheckType
                (crlCheckerState, PKIX_DEFAULTCRLCHECKERSTATE_TYPE, plContext),
                PKIX_OBJECTNOTDEFAULTCRLCHECKERSTATE);

        /* Set up CRLSelector */
        PKIX_CHECK(pkix_DefaultCRLChecker_Check_SetSelector
                (certToCheck,
                (pkix_DefaultCRLCheckerState *) crlCheckerState,
                plContext),
                PKIX_DEFAULTCRLCHECKERCHECKSETSELECTORFAILED);

        PKIX_CHECK
                (PKIX_CertChainChecker_SetCertChainCheckerState
                (state->buildConstants.crlChecker, crlCheckerState, plContext),
                PKIX_CERTCHAINCHECKERSETCERTCHAINCHECKERSTATEFAILED);

cleanup:

        PKIX_DECREF(crlCheckerState);

        PKIX_RETURN(BUILD);
}

/*
 * FUNCTION: pkix_BuildForwardDepthFirstSearch
 * DESCRIPTION:
 *
 *  This function performs a depth first search in the "forward" direction (from
 *  the target Cert to the trust anchor). A non-NULL targetCert must be stored
 *  in the ForwardBuilderState before this function is called. It is not written
 *  recursively since execution may be suspended in in any of several places
 *  pending completion of non-blocking I/O. This iterative structure makes it
 *  much easier to resume where it left off.
 *
 *  Since the nature of the search is recursive, the recursion is handled by
 *  chaining states. That is, each new step involves creating a new
 *  ForwardBuilderState linked to its predecessor. If a step turns out to be
 *  fruitless, the state of the predecessor is restored and the next alternative
 *  is tried. When a search is successful, values needed from the last state
 *  (canBeCached and validityDate) are copied to the state provided by the
 *  caller, so that the caller can retrieve those values.
 *
 *  There are three return arguments, the NBIOContext, the ValidateResult and
 *  the ForwardBuilderState. If NBIOContext is non-NULL, it means the search is
 *  suspended until the results of a non-blocking IO become available. The
 *  caller may wait for the completion using platform-dependent methods and then
 *  call this function again, allowing it to resume the search. If NBIOContext
 *  is NULL and the ValidateResult is non-NULL, it means the search has
 *  concluded successfully. If the NBIOContext is NULL but the ValidateResult is
 *  NULL, it means the search was unsuccessful.
 *
 *  This function performs several steps at each node in the constructed chain:
 *
 *  1) It retrieves Certs from the registered CertStores that match the
 *  criteria established by the ForwardBuilderState pointed to by "state", such
 *  as a subject name matching the issuer name of the previous Cert. If there
 *  are no matching Certs, the function returns to the previous, or "parent",
 *  state and tries to continue the chain building with another of the Certs
 *  obtained from the CertStores as possible issuers for that parent Cert.
 *
 *  2) For each candidate Cert returned by the CertStores, this function checks
 *  whether the Cert is valid. If it is trusted, this function checks whether
 *  this Cert might serve as a TrustAnchor for a complete chain.
 *
 *  3) It determines whether this Cert, in conjunction with any of the
 *  TrustAnchors, might complete a chain. A complete chain, from this or the
 *  preceding step, is checked to see whether it is valid as a complete
 *  chain, including the checks that cannot be done in the forward direction.
 *
 *  4) If this Cert chains successfully, but is not a complete chain, that is,
 *  we have not reached a trusted Cert, a new ForwardBuilderState is created
 *  with this Cert as the immediate predecessor, and we continue in step (1),
 *  attempting to get Certs from the CertStores with this Certs "issuer" as
 *  their subject.
 *
 *  5) If an entire chain validates successfully, then we are done. A
 *  ValidateResult is created containing the Public Key of the target
 *  certificate (including DSA parameter inheritance, if any) and the
 *  PolicyNode representing the policy tree output by the validation algorithm,
 *  and stored at pValResult, and the function exits returning NULL.
 *
 *  5) If the entire chain does not validate successfully, the algorithm
 *  discards the latest Cert and continues in step 2 with the next candidate
 *  Cert, backing up to a parent state when no more possibilities exist at a
 *  given level, and returning failure when we try to back up but discover we
 *  are at the top level.
 *
 * PARAMETERS:
 *  "pNBIOContext"
 *      Address at which platform-dependent information is returned if building
 *      is suspended for non-blocking I/O. Must be non-NULL.
 *  "pState"
 *      Address at which input ForwardBuilderState is found, and at which output
 *      ForwardBuilderState is stored. Must be non-NULL.
 *  "pValResult"
 *      Address at which the ValidateResult is stored. Must be non-NULL.
 *  "plContext"
 *      Platform-specific context pointer.
 * THREAD SAFETY:
 *  Thread Safe (see Thread Safety Definitions in Programmer's Guide)
 * RETURNS:
 *  Returns NULL if the function succeeds.
 *  Returns a Build Error if the function fails in a non-fatal way.
 *  Returns a Fatal Error if the function fails in an unrecoverable way.
 */
static PKIX_Error *
pkix_BuildForwardDepthFirstSearch(
        void **pNBIOContext,
        PKIX_ForwardBuilderState **pState,
        PKIX_ValidateResult **pValResult,
        void *plContext)
{
        PKIX_Boolean outOfOptions = PKIX_FALSE;
        PKIX_Boolean trusted = PKIX_FALSE;
        PKIX_Boolean isSelfIssued = PKIX_FALSE;
        PKIX_Boolean canBeCached = PKIX_FALSE;
        PKIX_Boolean passed = PKIX_FALSE;
        PKIX_Boolean revocationCheckingExists = PKIX_FALSE;
        PKIX_Boolean needsCRLChecking = PKIX_FALSE;
        PKIX_Boolean ioPending = PKIX_FALSE;
        PKIX_PL_Date *validityDate = NULL;
        PKIX_PL_Date *currTime  = NULL;
        PKIX_Int32 childTraversedCACerts = 0;
        PKIX_UInt32 numSubjectNames = 0;
        PKIX_UInt32 numChained = 0;
        PKIX_Int32 cmpTimeResult = 0;
        PKIX_UInt32 i = 0;
        PKIX_UInt32 certsSoFar = 0;
        PKIX_List *childTraversedSubjNames = NULL;
        PKIX_List *subjectNames = NULL;
        PKIX_List *unfilteredCerts = NULL;
        PKIX_List *filteredCerts = NULL;
        PKIX_PL_Object *subjectName = NULL;
        PKIX_ValidateResult *valResult = NULL;
        PKIX_ForwardBuilderState *state = NULL;
        PKIX_ForwardBuilderState *childState = NULL;
        PKIX_ForwardBuilderState *parentState = NULL;
        PKIX_PL_Object *crlCheckerState = NULL;
        PKIX_PL_PublicKey *candidatePubKey = NULL;
        PKIX_PL_PublicKey *trustedPubKey = NULL;
        PKIX_ComCertSelParams *certSelParams = NULL;
        PKIX_TrustAnchor *trustAnchor = NULL;
        PKIX_PL_Cert *trustedCert = NULL;
        PKIX_VerifyNode *verifyNode = NULL;
        PKIX_Error *verifyError = NULL;
        void *nbio = NULL;

        PKIX_ENTER(BUILD, "pkix_BuildForwardDepthFirstSearch");
        PKIX_NULLCHECK_FOUR(pNBIOContext, pState, *pState, pValResult);

        nbio = *pNBIOContext;
        *pNBIOContext = NULL;
        state = *pState;
        *pState = NULL; /* no net change in reference count */
        PKIX_INCREF(state->validityDate);
        validityDate = state->validityDate;
        canBeCached = state->canBeCached;
        PKIX_DECREF(*pValResult);

        /*
         * We return if successful; if we fall off the end
         * of this "while" clause our search has failed.
         */
        while (outOfOptions == PKIX_FALSE) {

            if (state->buildConstants.maxTime != 0) {
                    PKIX_DECREF(currTime);
                    PKIX_CHECK(PKIX_PL_Date_Create_UTCTime
                            (NULL, &currTime, plContext),
                            PKIX_DATECREATEUTCTIMEFAILED);

                    PKIX_CHECK(PKIX_PL_Object_Compare
                             ((PKIX_PL_Object *)state->buildConstants.timeLimit,
                             (PKIX_PL_Object *)currTime,
                             &cmpTimeResult,
                             plContext),
                             PKIX_OBJECTCOMPARATORFAILED);

                    if (cmpTimeResult < 0) {
                        if (state->verifyNode != NULL) {
                                PKIX_ERROR_CREATE
                                    (BUILD,
                                    PKIX_TIMECONSUMEDEXCEEDSRESOURCELIMITS,
                                    verifyError);
                                PKIX_CHECK_FATAL(pkix_VerifyNode_SetError
                                    (state->verifyNode,
                                    verifyError,
                                    plContext),
                                    PKIX_VERIFYNODESETERRORFAILED);
                                PKIX_DECREF(verifyError);
                        }
                        /* Even if we logged error, we still have to abort */
                        PKIX_ERROR(PKIX_TIMECONSUMEDEXCEEDSRESOURCELIMITS);
                    }
            }

            if (state->status == BUILD_INITIAL) {

                PKIX_CHECK(pkix_Build_BuildSelectorAndParams(state, plContext),
                        PKIX_BUILDBUILDSELECTORANDPARAMSFAILED);

                /*
                 * If the caller supplied a partial certChain (hintCerts) try
                 * the next one from that List before we go to the certStores.
                 */
                if (state->buildConstants.numHintCerts > 0) {
                        /* How many Certs does our trust chain have already? */
                        PKIX_CHECK(PKIX_List_GetLength
                                (state->trustChain, &certsSoFar, plContext),
                                PKIX_LISTGETLENGTHFAILED);

                        /* That includes the target Cert. Don't count it. */
                        certsSoFar--;

                        /* Are we still within range of the partial chain? */
                        if (certsSoFar >= state->buildConstants.numHintCerts) {
                                state->status = BUILD_TRYAIA;
                        } else {
                                /*
                                 * If we already have n certs, we want the n+1th
                                 * (i.e., index = n) from the list of hints.
                                 */
                                PKIX_CHECK(PKIX_List_GetItem
                                    (state->buildConstants.hintCerts,
                                    certsSoFar,
                                    (PKIX_PL_Object **)&state->candidateCert,
                                    plContext),
                                    PKIX_LISTGETITEMFAILED);

                                PKIX_CHECK(PKIX_List_AppendItem
                                    (state->candidateCerts,
                                    (PKIX_PL_Object *)state->candidateCert,
                                    plContext),
                                    PKIX_LISTAPPENDITEMFAILED);

                                state->numCerts = 1;
                                state->usingHintCerts = PKIX_TRUE;
                                state->status = BUILD_CERTVALIDATING;
                        }
                } else {
                        state->status = BUILD_TRYAIA;
                }

            }

            if (state->status == BUILD_TRYAIA) {
                if ((state->useOnlyLocal == PKIX_TRUE) ||
                    (state->alreadyTriedAIA == PKIX_TRUE)) {
                        state->status = BUILD_COLLECTINGCERTS;
                } else {
                        state->status = BUILD_AIAPENDING;
                }
            }

            if (state->status == BUILD_AIAPENDING) {
                PKIX_CHECK(PKIX_PL_AIAMgr_GetAIACerts
                        (state->buildConstants.aiaMgr,
                        state->prevCert,
                        &nbio,
                        &unfilteredCerts,
                        plContext),
                        PKIX_AIAMGRGETAIACERTSFAILED);

                if (nbio != NULL) {
                        /* IO still pending, resume later */
                        *pNBIOContext = nbio;
                        goto cleanup;
                }

                state->numCerts = 0;

#ifdef PKIX_BUILDDEBUG
                /* Turn this on to trace the List of Certs, before CertSelect */
                {
                                PKIX_PL_String *unString;
                                char *unAscii;
                                PKIX_UInt32 length;
                                PKIX_TOSTRING
                                        ((PKIX_PL_Object*)unfilteredCerts,
                                        &unString,
                                        plContext,
                                        PKIX_OBJECTTOSTRINGFAILED);

                                PKIX_CHECK(PKIX_PL_String_GetEncoded
                                        (unString,
                                        PKIX_ESCASCII,
                                        (void **)&unAscii,
                                        &length,
                                        plContext),
                                        PKIX_STRINGGETENCODEDFAILED);

                                PKIX_DEBUG_ARG
                                        ("unfilteredCerts = %s\n", unAscii);
                                PKIX_DECREF(unString);
                                PKIX_FREE(unAscii);
                }
#endif

                /* Note: Certs winnowed here don't get into VerifyTree. */
                if (unfilteredCerts) {
                        PKIX_CHECK(pkix_CertSelector_Select
                                (state->certSel,
                                unfilteredCerts,
                                &filteredCerts,
                                plContext),
                                PKIX_CERTSELECTORSELECTFAILED);

                        PKIX_DECREF(unfilteredCerts);

                        PKIX_CHECK(PKIX_List_GetLength
                                (filteredCerts, &(state->numCerts), plContext),
                                PKIX_LISTGETLENGTHFAILED);

#ifdef PKIX_BUILDDEBUG
                /* Turn this on to trace the List of Certs, after CertSelect */
                {
                        PKIX_PL_String *unString;
                        char *unAscii;
                        PKIX_UInt32 length;
                        PKIX_TOSTRING
                                ((PKIX_PL_Object*)filteredCerts,
                                &unString,
                                plContext,
                                PKIX_OBJECTTOSTRINGFAILED);

                        PKIX_CHECK(PKIX_PL_String_GetEncoded
                                    (unString,
                                    PKIX_ESCASCII,
                                    (void **)&unAscii,
                                    &length,
                                    plContext),
                                    PKIX_STRINGGETENCODEDFAILED);

                        PKIX_DEBUG_ARG("filteredCerts = %s\n", unAscii);
                        PKIX_DECREF(unString);
                        PKIX_FREE(unAscii);
                }
#endif

                        PKIX_DECREF(state->candidateCerts);
                        state->candidateCerts = filteredCerts;
                        filteredCerts = NULL;

                }

                /* Are there any Certs to try? */
                if (state->numCerts > 0) {
                        state->status = BUILD_CERTVALIDATING;
                } else {
                        state->status = BUILD_COLLECTINGCERTS;
                }
            }

            PKIX_DECREF(certSelParams);
            PKIX_CHECK(PKIX_CertSelector_GetCommonCertSelectorParams
                (state->certSel, &certSelParams, plContext),
                PKIX_CERTSELECTORGETCOMMONCERTSELECTORPARAMSFAILED);

            /* **** Querying the CertStores ***** */
            if ((state->status == BUILD_COLLECTINGCERTS) ||
                (state->status == BUILD_GATHERPENDING)) {

#if PKIX_FORWARDBUILDERSTATEDEBUG
                PKIX_CHECK(pkix_ForwardBuilderState_DumpState
                        (state, plContext),
                        PKIX_FORWARDBUILDERSTATEDUMPSTATEFAILED);
#endif

                PKIX_CHECK(pkix_Build_GatherCerts
                        (state, certSelParams, &nbio, plContext),
                        PKIX_BUILDGATHERCERTSFAILED);

                if (nbio != NULL) {
                        /* IO still pending, resume later */
                        *pNBIOContext = nbio;
                        goto cleanup;
                }

                /* Are there any Certs to try? */
                if (state->numCerts > 0) {
                        state->status = BUILD_CERTVALIDATING;
                } else {
                        state->status = BUILD_ABANDONNODE;
                }
            }

            /* ****Phase 2 - Chain building***** */

#if PKIX_FORWARDBUILDERSTATEDEBUG
            PKIX_CHECK(pkix_ForwardBuilderState_DumpState(state, plContext),
                    PKIX_FORWARDBUILDERSTATEDUMPSTATEFAILED);
#endif

            if (state->status == BUILD_CERTVALIDATING) {

                    revocationCheckingExists =
                        (state->buildConstants.crlChecker != NULL);

                    if ((revocationCheckingExists == PKIX_FALSE) &&
                        (state->revCheckers != NULL)) {
                            PKIX_CHECK(PKIX_List_GetLength
                                    (state->revCheckers, &i, plContext),
                                    PKIX_LISTGETLENGTHFAILED);
                            if (i > 0) {
                                    revocationCheckingExists = PKIX_TRUE;
                            }
                    }

                    PKIX_DECREF(state->candidateCert);
                    PKIX_CHECK(PKIX_List_GetItem
                            (state->candidateCerts,
                            state->certIndex,
                            (PKIX_PL_Object **)&(state->candidateCert),
                            plContext),
                            PKIX_LISTGETITEMFAILED);

                    if ((state->verifyNode) != NULL) {
                            PKIX_CHECK_FATAL(pkix_VerifyNode_Create
                                    (state->candidateCert,
                                    0,
                                    NULL,
                                    &verifyNode,
                                    plContext),
                                    PKIX_VERIFYNODECREATEFAILED);
                    }

                    /* If failure, this function sets Error in verifyNode */
                    verifyError = pkix_Build_VerifyCertificate
                            (state,
                            state->buildConstants.userCheckers,
                            revocationCheckingExists,
                            &trusted,
                            &needsCRLChecking,
                            verifyNode,
                            plContext);

                    if (verifyError) {
                            pkixTempErrorReceived = PKIX_TRUE;
                            pkixErrorClass = verifyError->errClass;
                            if (pkixErrorClass == PKIX_FATAL_ERROR) {
                                pkixErrorResult = verifyError;
                                verifyError = NULL;
                                goto fatal;
                            }
                    }

                    if (PKIX_ERROR_RECEIVED) {
                            if (state->verifyNode != NULL) {
                                PKIX_CHECK_FATAL(pkix_VerifyNode_SetError
                                    (verifyNode, verifyError, plContext),
                                    PKIX_VERIFYNODESETERRORFAILED);
                                PKIX_CHECK_FATAL(pkix_VerifyNode_AddToTree
                                        (state->verifyNode,
                                        verifyNode,
                                        plContext),
                                        PKIX_VERIFYNODEADDTOTREEFAILED);
                                PKIX_DECREF(verifyNode);
                            }
                            PKIX_DECREF(verifyError);
                            if (state->certLoopingDetected) {
                                PKIX_ERROR
                                    (PKIX_LOOPDISCOVEREDDUPCERTSNOTALLOWED);
                            }
                            state->status = BUILD_GETNEXTCERT;
                    } else if (needsCRLChecking) {
                            state->status = BUILD_CRLPREP;
                    } else {
                            state->status = BUILD_DATEPREP;
                    }
            }

            if (state->status == BUILD_CRLPREP) {

                    PKIX_CHECK(pkix_Build_RevCheckPrep
                            (state, state->prevCert, plContext),
                            PKIX_BUILDREVCHECKPREPFAILED);

                    state->status = BUILD_CRL1;
            }

            if (state->status == BUILD_CRL1) {

                PKIX_CHECK(PKIX_PL_Cert_GetSubjectPublicKey
                        (state->candidateCert,
                        &candidatePubKey,
                        plContext),
                        PKIX_CERTGETSUBJECTPUBLICKEYFAILED);

                PKIX_CHECK(PKIX_CertChainChecker_GetCertChainCheckerState
                        (state->buildConstants.crlChecker,
                        &crlCheckerState,
                        plContext),
                        PKIX_CERTCHAINCHECKERGETCERTCHAINCHECKERSTATEFAILED);

                PKIX_CHECK(pkix_CheckType
                        (crlCheckerState,
                        PKIX_DEFAULTCRLCHECKERSTATE_TYPE,
                        plContext),
                        PKIX_OBJECTNOTDEFAULTCRLCHECKERSTATE);

                verifyError = pkix_DefaultCRLChecker_Check_Helper
                        (state->buildConstants.crlChecker,
                        state->prevCert,
                        candidatePubKey,
                        (pkix_DefaultCRLCheckerState *) crlCheckerState,
                        NULL, /* unresolved crit extensions */
                        state->useOnlyLocal,
                        &nbio,
                        plContext);
                if (verifyError) {
                    pkixTempErrorReceived = PKIX_TRUE;
                    pkixErrorClass = verifyError->errClass;
                    if (pkixErrorClass == PKIX_FATAL_ERROR) {
                        pkixErrorResult = verifyError;
                        verifyError = NULL;
                        goto fatal;
                    }
                }

                PKIX_DECREF(candidatePubKey);
                PKIX_DECREF(crlCheckerState);

                if (nbio != NULL) {
                    /* IO still pending, resume later */
                    goto cleanup;
                } else if (PKIX_ERROR_RECEIVED) {
                    if (state->verifyNode != NULL) {
                            PKIX_CHECK_FATAL(pkix_VerifyNode_SetError
                                    (verifyNode, verifyError, plContext),
                                    PKIX_VERIFYNODESETERRORFAILED);
                            PKIX_CHECK_FATAL(pkix_VerifyNode_AddToTree
                                    (state->verifyNode,
                                    verifyNode,
                                    plContext),
                                    PKIX_VERIFYNODEADDTOTREEFAILED);
                            PKIX_DECREF(verifyNode);
                    }
                    PKIX_DECREF(verifyError);
                    if (state->certLoopingDetected) {
                            PKIX_ERROR
                                (PKIX_LOOPDISCOVEREDDUPCERTSNOTALLOWED);
                    }
                    state->status = BUILD_GETNEXTCERT;
                } else {
                    state->status = BUILD_DATEPREP;
                }
            }

            if (state->status == BUILD_DATEPREP) {
                    /* Keep track of whether this chain can be cached */
                    PKIX_CHECK(pkix_Build_UpdateDate(state, plContext),
                            PKIX_BUILDUPDATEDATEFAILED);
    
                    canBeCached = state->canBeCached;
                    PKIX_DECREF(validityDate);
                    PKIX_INCREF(state->validityDate);
                    validityDate = state->validityDate;
                    if (trusted == PKIX_TRUE) {
                            state->status = BUILD_CHECKTRUSTED;
                    } else {
                            state->status = BUILD_ADDTOCHAIN;
                    }
            }

            if (state->status == BUILD_CHECKTRUSTED) {

                    /*
                     * If this cert is trusted, try to validate the entire
                     * chain using this certificate as trust anchor.
                     */
                    PKIX_CHECK(PKIX_TrustAnchor_CreateWithCert
                      (state->candidateCert,
                      &trustAnchor,
                      plContext),
                      PKIX_TRUSTANCHORCREATEWITHCERTFAILED);

                    PKIX_CHECK(pkix_Build_ValidationCheckers
                      (state,
                      state->trustChain,
                      trustAnchor,
                      plContext),
                      PKIX_BUILDVALIDATIONCHECKERSFAILED);

                    state->status = BUILD_CHECKTRUSTED2;
            }

            if (state->status == BUILD_CHECKTRUSTED2) {
                    PKIX_CHECK_ONLY_FATAL(pkix_Build_ValidateEntireChain
                        (state,
                        trustAnchor,
                        &nbio, &valResult,
                        verifyNode,
                        plContext),
                        PKIX_BUILDVALIDATEENTIRECHAINFAILED);

                    if (nbio != NULL) {
                            /* IO still pending, resume later */
                            goto cleanup;
                    } else {
                            PKIX_DECREF(state->reversedCertChain);
                            PKIX_DECREF(state->checkedCritExtOIDs);
                            PKIX_DECREF(state->checkerChain);
                            PKIX_DECREF(state->revCheckers);
                            if (state->verifyNode != NULL) {
                                PKIX_CHECK_FATAL(pkix_VerifyNode_AddToTree
                                        (state->verifyNode,
                                        verifyNode,
                                        plContext),
                                        PKIX_VERIFYNODEADDTOTREEFAILED);
                                PKIX_DECREF(verifyNode);
                            }

                            if (!PKIX_ERROR_RECEIVED) {
                                *pValResult = valResult;
                                valResult = NULL;
                                /* Change state so IsIOPending is FALSE */
                                state->status = BUILD_CHECKTRUSTED;
                                goto cleanup;
                            }
                            PKIX_DECREF(trustAnchor);
                    }

                    /*
                     * If chain doesn't validate with a trusted Cert,
                     * adding more Certs to it can't help.
                     */
                    if (state->certLoopingDetected) {
                            PKIX_ERROR
                                (PKIX_LOOPDISCOVEREDDUPCERTSNOTALLOWED);
                    }
                    state->status = BUILD_GETNEXTCERT;
            }

            /*
             * This Cert was not trusted. Add it to our chain, and
             * continue building. If we don't reach a trust anchor,
             * we'll take it off later and continue without it.
             */
            if (state->status == BUILD_ADDTOCHAIN) {
                    PKIX_CHECK(PKIX_List_AppendItem
                            (state->trustChain,
                            (PKIX_PL_Object *)state->candidateCert,
                            plContext),
                            PKIX_LISTAPPENDITEMFAILED);

                    state->status = BUILD_CHECKWITHANCHORS;
                    state->anchorIndex = 0;
            }

            while ((state->status == BUILD_CHECKWITHANCHORS) ||
                (state->status == BUILD_CRL2) ||
                (state->status == BUILD_VALCHAIN2)) {
                    if (state->anchorIndex >=
                            state->buildConstants.numAnchors) {
                                   state->status = BUILD_EXTENDCHAIN;
                            break;
                    } else {

                            PKIX_CHECK(PKIX_List_GetItem
                                    (state->buildConstants.anchors,
                                    state->anchorIndex,
                                    (PKIX_PL_Object **)&trustAnchor,
                                    plContext),
                                    PKIX_LISTGETITEMFAILED);

                    }

                    if (state->status == BUILD_CHECKWITHANCHORS) {

                            /*
                             * Does this Trust Anchor chain to this cert?
                             * (If state->verifyNode is non-NULL, this function
                             * chains a verifyNode for each anchor checked.)
                             */
                            PKIX_CHECK(pkix_Build_CheckCertAgainstAnchor
                                    (state->candidateCert,
                                    trustAnchor,
                                    state->traversedSubjNames,
                                    &passed,
                                    verifyNode,
                                    plContext),
                                    PKIX_CHECKCERTAGAINSTANCHORFAILED);

                            if (passed == PKIX_TRUE) {
                                    if (state->buildConstants.crlChecker) {
                                            state->status = BUILD_CRL2PREP;
                                    } else {
                                            state->status = BUILD_VALCHAIN;
                                    }
                            } /* else increment anchorIndex and try next */
                    }

                    if (state->status == BUILD_CRL2PREP) {
                            PKIX_CHECK(pkix_Build_RevCheckPrep
                                    (state, state->candidateCert, plContext),
                                    PKIX_BUILDREVCHECKPREPFAILED);
                            state->status = BUILD_CRL2;
                    }
 
                    if (state->status == BUILD_CRL2) {
                      PKIX_CHECK(PKIX_TrustAnchor_GetTrustedCert
                          (trustAnchor, &trustedCert, plContext),
                          PKIX_TRUSTANCHORGETTRUSTEDCERTFAILED);

                      PKIX_CHECK(PKIX_PL_Cert_GetSubjectPublicKey
                          (trustedCert, &trustedPubKey, plContext),
                          PKIX_CERTGETSUBJECTPUBLICKEYFAILED);

                      PKIX_CHECK
                          (PKIX_CertChainChecker_GetCertChainCheckerState
                          (state->buildConstants.crlChecker,
                          &crlCheckerState,
                          plContext),
                          PKIX_CERTCHAINCHECKERGETCERTCHAINCHECKERSTATEFAILED);

                      PKIX_CHECK(pkix_CheckType
                          (crlCheckerState,
                          PKIX_DEFAULTCRLCHECKERSTATE_TYPE,
                          plContext),
                          PKIX_OBJECTNOTDEFAULTCRLCHECKERSTATE);

                      verifyError = pkix_DefaultCRLChecker_Check_Helper
                          (state->buildConstants.crlChecker,
                          state->candidateCert,
                          trustedPubKey,
                          (pkix_DefaultCRLCheckerState *) crlCheckerState,
                          NULL, /* unresolved crit extensions */
                          state->useOnlyLocal,
                          &nbio,
                          plContext);
                      if (verifyError) {
                          pkixTempErrorReceived = PKIX_TRUE;
                          pkixErrorClass = verifyError->errClass;
                          if (pkixErrorClass == PKIX_FATAL_ERROR) {
                              pkixErrorResult = verifyError;
                              verifyError = NULL;
                              goto fatal;
                          }
                      }

                      PKIX_DECREF(trustedCert);
                      PKIX_DECREF(trustedPubKey);
                      PKIX_DECREF(crlCheckerState);

                      if (nbio != NULL) {
                              /* IO still pending, resume later */
                              goto cleanup;
                      } else if (PKIX_ERROR_RECEIVED) {
                              if (state->verifyNode != NULL) {
                                  PKIX_CHECK_FATAL
                                      (pkix_VerifyNode_SetError
                                      (verifyNode,
                                      verifyError,
                                      plContext),
                                      PKIX_VERIFYNODESETERRORFAILED);
                              }
                              PKIX_DECREF(verifyError);
                              /* try again with the next trust anchor */
                              state->status = BUILD_CHECKWITHANCHORS;
                      } else {
                              state->status = BUILD_VALCHAIN;
                      }
                    }

                    if (state->status == BUILD_VALCHAIN) {
                            /* Does the chain pass all validation tests? */
                            PKIX_CHECK(pkix_Build_ValidationCheckers
                                    (state,
                                    state->trustChain,
                                    trustAnchor,
                                    plContext),
                                    PKIX_BUILDVALIDATIONCHECKERSFAILED);

                            state->status = BUILD_VALCHAIN2;
                    }

                    if (state->status == BUILD_VALCHAIN2) {
                            PKIX_CHECK_ONLY_FATAL
                                    (pkix_Build_ValidateEntireChain
                                    (state,
                                    trustAnchor,
                                    &nbio,
                                    &valResult,
                                    verifyNode,
                                    plContext),
                                    PKIX_BUILDVALIDATEENTIRECHAINFAILED);

                            if (nbio != NULL) {
                                    /* IO still pending, resume later */
                                    goto cleanup;
                            } else {
                                    PKIX_DECREF(state->reversedCertChain);
                                    PKIX_DECREF(state->checkedCritExtOIDs);
                                    PKIX_DECREF(state->checkerChain);
                                    PKIX_DECREF(state->revCheckers);
                                    if (!PKIX_ERROR_RECEIVED) {
                                        *pValResult = valResult;
                                        valResult = NULL;
                                        if (state->verifyNode != NULL) {
                                            PKIX_CHECK_FATAL
                                                (pkix_VerifyNode_AddToTree
                                                        (state->verifyNode,
                                                        verifyNode,
                                                        plContext),
                                                PKIX_VERIFYNODEADDTOTREEFAILED);
                                            PKIX_DECREF(verifyNode);
                                        }
                                        /* Make IsIOPending FALSE */
                                        state->status = BUILD_VALCHAIN;
                                        goto cleanup;
                                    }
                            }

                            state->status = BUILD_CHECKWITHANCHORS;
                    }

                    PKIX_DECREF(trustAnchor);
                    state->anchorIndex++;
            } /* while (anchorIndex < numAnchors) */

            if (state->status == BUILD_EXTENDCHAIN) {

                    /* Check whether we are allowed to extend the chain */
                    if ((state->buildConstants.maxDepth != 0) &&
                        (state->numDepth <= 1)) {

                        if (state->verifyNode != NULL) {
                                PKIX_ERROR_CREATE
                                    (BUILD,
                                    PKIX_DEPTHWOULDEXCEEDRESOURCELIMITS,
                                    verifyError);
                                PKIX_CHECK_FATAL(pkix_VerifyNode_SetError
                                    (verifyNode, verifyError, plContext),
                                    PKIX_VERIFYNODESETERRORFAILED);
                                PKIX_CHECK_FATAL(pkix_VerifyNode_AddToTree
                                    (state->verifyNode, verifyNode, plContext),
                                    PKIX_VERIFYNODEADDTOTREEFAILED);
                                PKIX_DECREF(verifyNode);
                                PKIX_DECREF(verifyError);
                        }
                        /* Even if error logged, still need to abort */
                        PKIX_ERROR(PKIX_DEPTHWOULDEXCEEDRESOURCELIMITS);
                    }

                    PKIX_CHECK(pkix_IsCertSelfIssued
                            (state->candidateCert, &isSelfIssued, plContext),
                            PKIX_ISCERTSELFISSUEDFAILED);
                 
                    PKIX_CHECK(PKIX_PL_Object_Duplicate
                            ((PKIX_PL_Object *)state->traversedSubjNames,
                            (PKIX_PL_Object **)&childTraversedSubjNames,
                            plContext),
                            PKIX_OBJECTDUPLICATEFAILED);
         
                    if (isSelfIssued) {
                            childTraversedCACerts = state->traversedCACerts;
                    } else {
                            childTraversedCACerts = state->traversedCACerts + 1;
                 
                            PKIX_CHECK(PKIX_PL_Cert_GetAllSubjectNames
                                (state->candidateCert,
                                &subjectNames,
                                plContext),
                                PKIX_CERTGETALLSUBJECTNAMESFAILED);
                 
                            if (subjectNames) {
                                PKIX_CHECK(PKIX_List_GetLength
                                    (subjectNames,
                                    &numSubjectNames,
                                    plContext),
                                    PKIX_LISTGETLENGTHFAILED);
         
                            } else {
                                numSubjectNames = 0;
                            }

                            for (i = 0; i < numSubjectNames; i++) {
                                PKIX_CHECK(PKIX_List_GetItem
                                    (subjectNames,
                                    i,
                                    &subjectName,
                                    plContext),
                                    PKIX_LISTGETITEMFAILED);
                                PKIX_NULLCHECK_ONE
                                    (state->traversedSubjNames);
                                PKIX_CHECK(PKIX_List_AppendItem
                                    (state->traversedSubjNames,
                                    subjectName,
                                    plContext),
                                    PKIX_LISTAPPENDITEMFAILED);
                                PKIX_DECREF(subjectName);
                            }
                            PKIX_DECREF(subjectNames);
                        }
            
                        PKIX_CHECK(pkix_ForwardBuilderState_Create
                            (childTraversedCACerts,
                            state->buildConstants.maxFanout,
                            state->numDepth - 1,
                            state->dsaParamsNeeded,
                            state->revCheckDelayed,
                            canBeCached,
                            validityDate,
                            state->candidateCert,
                            childTraversedSubjNames,
                            state->trustChain,
                            state,
                            &childState,
                            plContext),
                            PKIX_FORWARDBUILDSTATECREATEFAILED);

                        PKIX_DECREF(childTraversedSubjNames);
                        PKIX_DECREF(certSelParams);
                        childState->verifyNode = verifyNode;
                        verifyNode = NULL;
                        PKIX_DECREF(state);
                        state = childState; /* state->status == BUILD_INITIAL */
                        childState = NULL;
                        continue; /* with while (!outOfOptions) */
            }

            if (state->status == BUILD_GETNEXTCERT) {
                    pkixTempErrorReceived = PKIX_FALSE;
                    PKIX_DECREF(state->candidateCert);

                    /*
                     * If we were using a Cert from the callier-supplied partial
                     * chain, delete it and go to the certStores.
                     */
                    if (state->usingHintCerts == PKIX_TRUE) {

                            PKIX_DECREF(state->candidateCerts);
                            PKIX_CHECK(PKIX_List_Create
                                (&state->candidateCerts, plContext),
                                PKIX_LISTCREATEFAILED);

                            state->numCerts = 0;
                            state->usingHintCerts = PKIX_FALSE;
                            state->status = BUILD_TRYAIA;
                            continue;

                    } else if (++(state->certIndex) < (state->numCerts)) {

                            if ((state->buildConstants.maxFanout != 0) &&
                                (--(state->numFanout) == 0)) {

                                if (state->verifyNode != NULL) {
                                        PKIX_ERROR_CREATE
                                            (BUILD,
                                            PKIX_FANOUTEXCEEDSRESOURCELIMITS,
                                            verifyError);
                                        PKIX_CHECK_FATAL
                                            (pkix_VerifyNode_SetError
                                            (state->verifyNode,
                                            verifyError,
                                            plContext),
                                            PKIX_VERIFYNODESETERRORFAILED);
                                        PKIX_DECREF(verifyError);
                                }
                                /* Even if error logged, still need to abort */
                                PKIX_ERROR
                                        (PKIX_FANOUTEXCEEDSRESOURCELIMITS);
                            }
                            state->status = BUILD_CERTVALIDATING;
                            continue;
                    }

                    /*
                     * We have no more certs to try. If we got them by
                     * following an AIA, let's go back and try our
                     * certStores for certs.
                     */
                    if (state->alreadyTriedAIA == PKIX_FALSE) {
                            state->alreadyTriedAIA = PKIX_TRUE;
                            state->status = BUILD_INITIAL;
                            PKIX_DECREF(state->candidateCerts);
                            PKIX_DECREF(state->certSel);
                            continue;
                    }
            }

            /*
             * Adding the current cert to the chain didn't help. If our search
             * has been restricted to local certStores, try opening up the
             * search and see whether that helps. Otherwise, back up to the
             * parent cert, and see if there are any more to try.
             */
            if (state->useOnlyLocal == PKIX_TRUE) {
                state->useOnlyLocal = PKIX_FALSE;
                state->certStoreIndex = 0;
                state->numFanout = state->buildConstants.maxFanout;
                state->status = BUILD_TRYAIA;
            } else do {
                if (state->parentState == NULL) {
                        /* We are at the top level, and can't back up! */
                        outOfOptions = PKIX_TRUE;
                } else {

                        /*
                         * Try the next cert, if any, for this parent.
                         * Otherwise keep backing up until we reach a
                         * parent with more certs to try.
                         */
                        PKIX_CHECK(PKIX_List_GetLength
                                (state->trustChain, &numChained, plContext),
                                PKIX_LISTGETLENGTHFAILED);
                        PKIX_CHECK(PKIX_List_DeleteItem
                                (state->trustChain, numChained - 1, plContext),
                                PKIX_LISTDELETEITEMFAILED);
                        PKIX_INCREF(state->parentState);
                        parentState = state->parentState;
                        PKIX_DECREF(verifyNode);
                        verifyNode = state->verifyNode;
                        state->verifyNode = NULL;
                        PKIX_DECREF(state);
                        state = parentState;
                        parentState = NULL;
                        if (state->verifyNode != NULL) {
                                PKIX_CHECK_FATAL(pkix_VerifyNode_AddToTree
                                        (state->verifyNode,
                                        verifyNode,
                                        plContext),
                                        PKIX_VERIFYNODEADDTOTREEFAILED);
                                PKIX_DECREF(verifyNode);
                        }
                        PKIX_DECREF(validityDate);
                        PKIX_INCREF(state->validityDate);
                        validityDate = state->validityDate;
                        canBeCached = state->canBeCached;

                        /* Are there any more Certs to try? */
                        if (++(state->certIndex) < (state->numCerts)) {
                                state->status = BUILD_CERTVALIDATING;
                                PKIX_DECREF(state->candidateCert);
                                break;
                        }
                }
                PKIX_DECREF(state->candidateCert);
            } while (outOfOptions == PKIX_FALSE);

        } /* while (outOfOptions == PKIX_FALSE) */

cleanup:

        if (pkixErrorClass == PKIX_FATAL_ERROR) {
            goto fatal;
        }

        /* verifyNode should be equal to NULL at this point. Assert it.
         * Temporarelly use verifyError to store an error ref to which we
         * have in pkixErrorResult. This is done to prevent error cloberring
         * while using macros below. */
        PORT_Assert(verifyError == NULL);
        verifyError = pkixErrorResult;

        /*
         * We were called with an initialState that had no parent. If we are
         * returning with an error or with a result, we must destroy any state
         * that we created (any state with a parent).
         */

        PKIX_CHECK_FATAL(pkix_ForwardBuilderState_IsIOPending
                         (state, &ioPending, plContext),
                PKIX_FORWARDBUILDERSTATEISIOPENDINGFAILED);

        if (ioPending == PKIX_FALSE) {
                while (state->parentState) {
                        PKIX_INCREF(state->parentState);
                        parentState = state->parentState;
                        PKIX_DECREF(verifyNode);
                        verifyNode = state->verifyNode;
                        state->verifyNode = NULL;
                        PKIX_DECREF(state);
                        state = parentState;
                        parentState = NULL;
                        if (state->verifyNode != NULL) {
                                PKIX_CHECK_FATAL(pkix_VerifyNode_AddToTree
                                        (state->verifyNode,
                                        verifyNode,
                                        plContext),
                                        PKIX_VERIFYNODEADDTOTREEFAILED);
                                PKIX_DECREF(verifyNode);
                        }
                }
                state->canBeCached = canBeCached;
                PKIX_DECREF(state->validityDate);
                state->validityDate = validityDate;
                validityDate = NULL;
        }
        *pState = state;
        state = NULL;
        pkixErrorResult = verifyError;
        verifyError = NULL;

fatal:
        PKIX_DECREF(state);
        PKIX_DECREF(parentState);
        PKIX_DECREF(childState);
        PKIX_DECREF(valResult);
        PKIX_DECREF(verifyError);
        PKIX_DECREF(verifyNode);
        PKIX_DECREF(candidatePubKey);
        PKIX_DECREF(trustedPubKey);
        PKIX_DECREF(childTraversedSubjNames);
        PKIX_DECREF(certSelParams);
        PKIX_DECREF(subjectNames);
        PKIX_DECREF(subjectName);
        PKIX_DECREF(trustAnchor);
        PKIX_DECREF(validityDate);
        PKIX_DECREF(crlCheckerState);
        PKIX_DECREF(currTime);
        PKIX_DECREF(filteredCerts);
        PKIX_DECREF(unfilteredCerts);
        PKIX_DECREF(trustedCert);

        PKIX_RETURN(BUILD);
}

/*
 * FUNCTION: pkix_Build_TryShortcut
 * DESCRIPTION:
 *
 *  This function checks whether the target cert in "state", subject to the name
 *  constraints specified by "targetSubjNames", forms a complete trust chain
 *  with any of the trust anchors.
 * 
 *  If a crlChecker using non-blocking I/O returns with an indication that I/O
 *  is in progress, this function stores the NBIOContext (returned by the
 *  checker) at "pNBIOContext". Otherwise, it stores NULL at "pNBIOContext" and
 *  indicates in "pAnchor" whether a complete trust chain was found. If no
 *  successful trust chain is found, NULL is stored at "pAnchor". If a
 *  successful trust chain is found, the anchor that completed the chain is
 *  stored at "pAnchor".
 *
 * PARAMETERS:
 *  "state"
 *      Address of ForwardBuilderState to be used. Must be non-NULL.
 *  "targetSubjNames"
 *      Address of List of subject names in targetCertificate. Must be non-NULL.
 *  "pNBIOContext"
 *      Address at which the NBIOContext is stored indicating whether the
 *      checking is complete. Must be non-NULL.
 *  "pAnchor"
 *      Address at which successful trustAnchor is stored, if trustAnchor and
 *      Certificate form a complete trust chain. Must be non-NULL.
 *  "plContext"
 *      Platform-specific context pointer.
 * THREAD SAFETY:
 *  Thread Safe (see Thread Safety Definitions in Programmer's Guide)
 * RETURNS:
 *  Returns NULL if the function succeeds.
 *  Returns a Build Error if the function fails in a non-fatal way
 *  Returns a Fatal Error if the function fails in an unrecoverable way.
 */
static PKIX_Error *
pkix_Build_TryShortcut(
        PKIX_ForwardBuilderState *state,
        PKIX_List *targetSubjNames,
        void **pNBIOContext,
        PKIX_TrustAnchor **pAnchor,
        void *plContext)
{
        PKIX_Boolean passed = PKIX_FALSE;
        void *nbioContext = NULL;
        PKIX_TrustAnchor *anchor = NULL;
        PKIX_PL_Cert *trustedCert = NULL;
        PKIX_PL_PublicKey *trustedPubKey = NULL;
        PKIX_PL_Object *crlCheckerState = NULL;
        PKIX_Error *crlCheckerError = NULL;

        PKIX_ENTER(BUILD, "pkix_Build_TryShortcut");
        PKIX_NULLCHECK_THREE(state, pNBIOContext, pAnchor);

        *pNBIOContext = NULL; /* prepare in case of error exit */

        /*
         * Does the target cert, with any of our trust
         * anchors, form a complete trust chain?
         */
        while (state->anchorIndex < state->buildConstants.numAnchors) {
                PKIX_CHECK(PKIX_List_GetItem
                        (state->buildConstants.anchors,
                        state->anchorIndex,
                        (PKIX_PL_Object **)&anchor,
                        plContext),
                        PKIX_LISTGETITEMFAILED);
                PKIX_CHECK(pkix_Build_CheckCertAgainstAnchor
                        (state->prevCert,
                        anchor,
                        targetSubjNames,
                        &passed,
                        state->verifyNode,
                        plContext),
                        PKIX_CHECKCERTAGAINSTANCHORFAILED);

                if (passed == PKIX_TRUE) {
                    if (state->buildConstants.crlChecker != NULL) {

                        PKIX_CHECK(PKIX_TrustAnchor_GetTrustedCert
                                (anchor, &trustedCert, plContext),
                                PKIX_TRUSTANCHORGETTRUSTEDCERTFAILED);

                        PKIX_CHECK(PKIX_PL_Cert_GetSubjectPublicKey
                                (trustedCert, &trustedPubKey, plContext),
                                PKIX_CERTGETSUBJECTPUBLICKEYFAILED);

                        PKIX_CHECK
                          (PKIX_CertChainChecker_GetCertChainCheckerState
                          (state->buildConstants.crlChecker,
                          &crlCheckerState,
                          plContext),
                          PKIX_CERTCHAINCHECKERGETCERTCHAINCHECKERSTATEFAILED);

                        PKIX_CHECK(pkix_CheckType
                                (crlCheckerState,
                                PKIX_DEFAULTCRLCHECKERSTATE_TYPE,
                                plContext),
                                PKIX_OBJECTNOTDEFAULTCRLCHECKERSTATE);

                        /* Set up CRLSelector */
                        PKIX_CHECK(pkix_DefaultCRLChecker_Check_SetSelector
                            (state->prevCert,
                            (pkix_DefaultCRLCheckerState *) crlCheckerState,
                            plContext),
                            PKIX_DEFAULTCRLCHECKERCHECKSETSELECTORFAILED);

                        crlCheckerError =
                                pkix_DefaultCRLChecker_Check_Helper
                                (state->buildConstants.crlChecker,
                                state->prevCert,
                                trustedPubKey,
                                (pkix_DefaultCRLCheckerState *) crlCheckerState,
                                NULL, /* unresolved crit extensions */
                                PKIX_FALSE,
                                &nbioContext,
                                plContext);

                        if (crlCheckerError) {
                                pkixTempErrorReceived = PKIX_TRUE;
                                pkixErrorClass = crlCheckerError->errClass;
                                if (pkixErrorClass == PKIX_FATAL_ERROR) {
                                    pkixErrorResult = crlCheckerError;
                                    crlCheckerError = NULL;
                                    goto cleanup;
                                }
                        }

                        if (nbioContext != NULL) {
                                state->status = BUILD_SHORTCUTPENDING;
                                *pNBIOContext = nbioContext;
                                goto cleanup;
                        }

                        PKIX_DECREF(trustedCert);
                        PKIX_DECREF(trustedPubKey);
                        PKIX_DECREF(crlCheckerState);

                    } /* if (state->buildConstants.crlChecker != NULL) */

                    if ((state->verifyNode) && (crlCheckerError)) {
                            state->verifyNode->error = crlCheckerError;
                            crlCheckerError = NULL;
                    }
                    PKIX_DECREF(crlCheckerError);
                    if (!PKIX_ERROR_RECEIVED) {
                            /* Exit loop with anchor set */
                            break;
                    }

                }   /* if (passed == PKIX_FALSE) ... else ... */
                PKIX_DECREF(trustedPubKey);
                PKIX_DECREF(anchor);
                state->anchorIndex++;
        } /* while (state->anchorIndex < state->buildConstants.numAnchors) */

        *pAnchor = anchor;
        anchor = NULL;

cleanup:

        PKIX_DECREF(trustedCert);
        PKIX_DECREF(trustedPubKey);
        PKIX_DECREF(crlCheckerState);
        PKIX_DECREF(anchor);

        PKIX_RETURN(BUILD);
}

/*
 * FUNCTION: pkix_Build_CheckInCache
 * DESCRIPTION:
 *
 * The function tries to locate a chain for a cert in the cert chain cache.
 * If found, the chain goes through revocation chacking and returned back to
 * caller. Chains that fail revocation check get removed from cache.
 * 
 * PARAMETERS:
 *  "state"
 *      Address of ForwardBuilderState to be used. Must be non-NULL.
 *  "pBuildResult"
 *      Address at which the BuildResult is stored, after a successful build.
 *      Must be non-NULL.
 *  "pNBIOContext"
 *      Address at which the NBIOContext is stored indicating whether the
 *      validation is complete. Must be non-NULL.
 *  "plContext"
 *      Platform-specific context pointer.
 * THREAD SAFETY:
 *  Thread Safe (see Thread Safety Definitions in Programmer's Guide)
 * RETURNS:
 *  Returns NULL if the function succeeds.
 *  Returns a Build Error if the function fails in a non-fatal way
 *  Returns a Fatal Error if the function fails in an unrecoverable way.
 */
static PKIX_Error*
pkix_Build_CheckInCache(
        PKIX_ForwardBuilderState *state,
        PKIX_BuildResult **pBuildResult,
        void **pNBIOContext,
        void *plContext)
{
        PKIX_PL_Cert *targetCert = NULL;
        PKIX_List *anchors = NULL;
        PKIX_PL_Date *testDate = NULL;
        PKIX_BuildResult *buildResult = NULL;
        PKIX_ValidateResult *valResult = NULL;
        PKIX_TrustAnchor *matchingAnchor = NULL;
        PKIX_PL_Cert *trustedCert = NULL;
        PKIX_List *certList = NULL;
        PKIX_Boolean cacheHit = PKIX_FALSE;
        PKIX_Boolean trusted = PKIX_FALSE;
        PKIX_Boolean stillValid = PKIX_FALSE;
        void *nbioContext = NULL;

        PKIX_ENTER(BUILD, "pkix_Build_CheckInCache");

        nbioContext = *pNBIOContext;
        *pNBIOContext = NULL;

        targetCert = state->buildConstants.targetCert;
        anchors = state->buildConstants.anchors;
        testDate = state->buildConstants.testDate;

        /* Check whether this cert verification has been cached. */
        PKIX_CHECK(pkix_CacheCertChain_Lookup
                   (targetCert,
                    anchors,
                    testDate,
                    &cacheHit,
                    &buildResult,
                    plContext),
                   PKIX_CACHECERTCHAINLOOKUPFAILED);
        
        if (!cacheHit) {
            goto cleanup;
        }
        
        /*
         * We found something in cache. Verify that the anchor
         * cert is still trusted,
         */
        PKIX_CHECK(PKIX_BuildResult_GetValidateResult
                   (buildResult, &valResult, plContext),
                   PKIX_BUILDRESULTGETVALIDATERESULTFAILED);
        
        PKIX_CHECK(PKIX_ValidateResult_GetTrustAnchor
                       (valResult, &matchingAnchor, plContext),
                   PKIX_VALIDATERESULTGETTRUSTANCHORFAILED);
        
        PKIX_DECREF(valResult);
        
        PKIX_CHECK(PKIX_TrustAnchor_GetTrustedCert
                   (matchingAnchor, &trustedCert, plContext),
                   PKIX_TRUSTANCHORGETTRUSTEDCERTFAILED);
        
        PKIX_CHECK(PKIX_PL_Cert_IsCertTrusted
                   (trustedCert, &trusted, plContext),
                   PKIX_CERTISCERTTRUSTEDFAILED);
        
        if (!trusted) {
            goto cleanup;
        }
        /*
         * Since the key usage may vary for different
         * applications, we need to verify the chain again.
         * Reverification will be improved with a fix for 397805.
         */
        PKIX_CHECK(PKIX_BuildResult_GetCertChain
                   (buildResult, &certList, plContext),
                   PKIX_BUILDRESULTGETCERTCHAINFAILED);
        
        /* setting this variable will trigger addition rev
         * checker into cert chain checker list */
        state->revCheckDelayed = PKIX_TRUE;
        
        PKIX_CHECK(pkix_Build_ValidationCheckers
                   (state,
                    certList,
                    matchingAnchor,
                    plContext),
                   PKIX_BUILDVALIDATIONCHECKERSFAILED);
        
        state->revCheckDelayed = PKIX_FALSE;

        PKIX_CHECK_ONLY_FATAL(
            pkix_Build_ValidateEntireChain(state, matchingAnchor,
                                           &nbioContext, &valResult,
                                           state->verifyNode, plContext),
            PKIX_BUILDVALIDATEENTIRECHAINFAILED);

        if (nbioContext != NULL) {
            /* IO still pending, resume later */
            *pNBIOContext = nbioContext;
            goto cleanup;
        }
        PKIX_DECREF(state->reversedCertChain);
        PKIX_DECREF(state->checkedCritExtOIDs);
        PKIX_DECREF(state->checkerChain);
        PKIX_DECREF(state->revCheckers);
        
        if (!PKIX_ERROR_RECEIVED) {
            /* The result from cache is still valid. But we replace an old*/
            *pBuildResult = buildResult;
            buildResult = NULL;
            stillValid = PKIX_TRUE;
        }

cleanup:

        if (!nbioContext && cacheHit && !(trusted && stillValid)) {
            /* The anchor of this chain is no longer trusted or
             * chain cert(s) has been revoked.
             * Invalidate this result in the cache */
            PKIX_CHECK_FATAL(pkix_CacheCertChain_Remove
                       (targetCert,
                        anchors,
                        plContext),
                       PKIX_CACHECERTCHAINREMOVEFAILED);
        }

fatal:
       PKIX_DECREF(buildResult);
       PKIX_DECREF(valResult);
       PKIX_DECREF(certList);
       PKIX_DECREF(matchingAnchor);
       PKIX_DECREF(trustedCert);

       
       PKIX_RETURN(BUILD);
}

/*
 * FUNCTION: pkix_Build_InitiateBuildChain
 * DESCRIPTION:
 *
 *  This function initiates the search for a BuildChain, using the parameters
 *  provided in "procParams" and, if continuing a search that was suspended
 *  for I/O, using the ForwardBuilderState pointed to by "state".
 *
 *  If a successful chain is built, this function stores the BuildResult at
 *  "pBuildResult". Alternatively, if an operation using non-blocking I/O
 *  is in progress and the operation has not been completed, this function
 *  stores the platform-dependent non-blocking I/O context (nbioContext) at
 *  "pNBIOContext", the FowardBuilderState at "pState", and NULL at
 *  "pBuildResult". Finally, if chain building was unsuccessful, this function
 *  stores NULL at both "pState" and at "pBuildResult".
 *
 *  Note: This function is re-entered only for the case of non-blocking I/O
 *  in the "short-cut" attempt to build a chain using the target Certificate
 *  directly with one of the trustAnchors. For all other cases, resumption
 *  after non-blocking I/O is via pkix_Build_ResumeBuildChain.
 *
 * PARAMETERS:
 *  "procParams"
 *      Address of the ProcessingParams for the search. Must be non-NULL.
 *  "pNBIOContext"
 *      Address at which the NBIOContext is stored indicating whether the
 *      validation is complete. Must be non-NULL.
 *  "pState"
 *      Address at which the ForwardBuilderState is stored, if the chain
 *      building is suspended for waiting I/O; also, the address at which the
 *      ForwardBuilderState is provided for resumption of the chain building
 *      attempt. Must be non-NULL.
 *  "pBuildResult"
 *      Address at which the BuildResult is stored, after a successful build.
 *      Must be non-NULL.
 *  "pVerifyNode"
 *      Address at which a VerifyNode chain is returned, if non-NULL.
 *  "plContext"
 *      Platform-specific context pointer.
 * THREAD SAFETY:
 *  Thread Safe (see Thread Safety Definitions in Programmer's Guide)
 * RETURNS:
 *  Returns NULL if the function succeeds.
 *  Returns a Build Error if the function fails in a non-fatal way
 *  Returns a Fatal Error if the function fails in an unrecoverable way.
 */
static PKIX_Error *
pkix_Build_InitiateBuildChain(
        PKIX_ProcessingParams *procParams,
        void **pNBIOContext,
        PKIX_ForwardBuilderState **pState,
        PKIX_BuildResult **pBuildResult,
        PKIX_VerifyNode **pVerifyNode,
        void *plContext)
{
        PKIX_UInt32 numAnchors = 0;
        PKIX_UInt32 numCertStores = 0;
        PKIX_UInt32 numHintCerts = 0;
        PKIX_UInt32 i = 0;
        PKIX_Boolean dsaParamsNeeded = PKIX_FALSE;
        PKIX_Boolean isCrlEnabled = PKIX_FALSE;
        PKIX_Boolean nistCRLPolicyEnabled = PKIX_TRUE;
        PKIX_Boolean isDuplicate = PKIX_FALSE;
        PKIX_PL_Cert *trustedCert = NULL;
        PKIX_CertSelector *targetConstraints = NULL;
        PKIX_ComCertSelParams *targetParams = NULL;
        PKIX_List *anchors = NULL;
        PKIX_List *targetSubjNames = NULL;
        PKIX_PL_Cert *targetCert = NULL;
        PKIX_PL_Object *firstHintCert = NULL;
        PKIX_CertChainChecker *crlChecker = NULL;
        PKIX_List *certStores = NULL;
        PKIX_CertStore *certStore = NULL;
        PKIX_List *userCheckers = NULL;
        PKIX_List *hintCerts = NULL;
        PKIX_PL_Date *testDate = NULL;
        PKIX_PL_PublicKey *targetPubKey = NULL;
        void *nbioContext = NULL;
        BuildConstants buildConstants;

        PKIX_List *tentativeChain = NULL;
        PKIX_ValidateResult *valResult = NULL;
        PKIX_BuildResult *buildResult = NULL;
        PKIX_List *certList = NULL;
        PKIX_TrustAnchor *matchingAnchor = NULL;
        PKIX_ForwardBuilderState *state = NULL;
        PKIX_CertStore_CheckTrustCallback trustCallback = NULL;
        PKIX_PL_AIAMgr *aiaMgr = NULL;

        PKIX_ENTER(BUILD, "pkix_Build_InitiateBuildChain");
        PKIX_NULLCHECK_FOUR(procParams, pNBIOContext, pState, pBuildResult);

        nbioContext = *pNBIOContext;
        *pNBIOContext = NULL;

        if (*pState != NULL) {
            state = *pState;
            *pState = NULL; /* no net change in reference count */
            /* attempted shortcut ran into non-blocking I/O */
        } else {

            PKIX_CHECK(PKIX_ProcessingParams_GetDate
                    (procParams, &testDate, plContext),
                    PKIX_PROCESSINGPARAMSGETDATEFAILED);
    
            if (!testDate) {
                    PKIX_CHECK(PKIX_PL_Date_Create_UTCTime
                            (NULL, &testDate, plContext),
                            PKIX_DATECREATEUTCTIMEFAILED);
            }
    
            PKIX_CHECK(PKIX_ProcessingParams_GetTrustAnchors
                    (procParams, &anchors, plContext),
                    PKIX_PROCESSINGPARAMSGETTRUSTANCHORSFAILED);
    
            PKIX_CHECK(PKIX_List_GetLength(anchors, &numAnchors, plContext),
                    PKIX_LISTGETLENGTHFAILED);
    
            /* retrieve stuff from targetCertConstraints */
    
            PKIX_CHECK(PKIX_ProcessingParams_GetTargetCertConstraints
                    (procParams, &targetConstraints, plContext),
                    PKIX_PROCESSINGPARAMSGETTARGETCERTCONSTRAINTSFAILED);
    
            PKIX_CHECK(PKIX_CertSelector_GetCommonCertSelectorParams
                    (targetConstraints, &targetParams, plContext),
                    PKIX_CERTSELECTORGETCOMMONCERTSELECTORPARAMSFAILED);
    
            PKIX_CHECK(PKIX_ComCertSelParams_GetCertificate
                    (targetParams, &targetCert, plContext),
                    PKIX_COMCERTSELPARAMSGETCERTIFICATEFAILED);
    
            PKIX_CHECK(PKIX_ProcessingParams_GetHintCerts
                        (procParams, &hintCerts, plContext),
                        PKIX_PROCESSINGPARAMSGETHINTCERTSFAILED);
    
            if (hintCerts != NULL) {
                    PKIX_CHECK(PKIX_List_GetLength
                            (hintCerts, &numHintCerts, plContext),
                            PKIX_LISTGETLENGTHFAILED);
            }

            /*
             * Caller must provide either a target Cert
             * (in ComCertSelParams->Certificate) or a partial Cert
             * chain (in ProcParams->HintCerts).
             */

            if (targetCert == NULL) {

                    /* Use first cert of hintCerts as the targetCert */
                    if (numHintCerts == 0) {
                            PKIX_ERROR(PKIX_NOTARGETCERTSUPPLIED);
                    }

                    PKIX_CHECK(PKIX_List_GetItem
                            (hintCerts,
                            0,
                            (PKIX_PL_Object **)&targetCert,
                            plContext),
                            PKIX_LISTGETITEMFAILED);

                    PKIX_CHECK(PKIX_List_DeleteItem(hintCerts, 0, plContext),
                            PKIX_LISTGETITEMFAILED);
            } else {

                    /*
                     * If the first hintCert is the same as the targetCert,
                     * delete it from hintCerts.
                     */ 
                    if (numHintCerts != 0) {
                            PKIX_CHECK(PKIX_List_GetItem
                                    (hintCerts, 0, &firstHintCert, plContext),
                                    PKIX_LISTGETITEMFAILED);

                            PKIX_CHECK(PKIX_PL_Object_Equals
                                    ((PKIX_PL_Object *)targetCert,
                                    firstHintCert,
                                    &isDuplicate,
                                    plContext),
                                    PKIX_OBJECTEQUALSFAILED);

                            if (isDuplicate) {
                                    PKIX_CHECK(PKIX_List_DeleteItem
                                    (hintCerts, 0, plContext),
                                    PKIX_LISTGETITEMFAILED);
                            }
                            PKIX_DECREF(firstHintCert);
                    }

            }

            if (targetCert == NULL) {
                PKIX_ERROR(PKIX_NOTARGETCERTSUPPLIED);
            }

            PKIX_CHECK(PKIX_PL_Cert_GetAllSubjectNames
                    (targetCert,
                    &targetSubjNames,
                    plContext),
                    PKIX_CERTGETALLSUBJECTNAMESFAILED);
    
            PKIX_CHECK(PKIX_PL_Cert_GetSubjectPublicKey
                    (targetCert, &targetPubKey, plContext),
                    PKIX_CERTGETSUBJECTPUBLICKEYFAILED);
    
            PKIX_CHECK(PKIX_List_Create(&tentativeChain, plContext),
                    PKIX_LISTCREATEFAILED);
    
            PKIX_CHECK(PKIX_List_AppendItem
                    (tentativeChain, (PKIX_PL_Object *)targetCert, plContext),
                    PKIX_LISTAPPENDITEMFAILED);
    
            PKIX_CHECK(PKIX_PL_PublicKey_NeedsDSAParameters
                    (targetPubKey, &dsaParamsNeeded, plContext),
                    PKIX_PUBLICKEYNEEDSDSAPARAMETERSFAILED);
    
            /* Failure here is reportable */
            pkixErrorResult = PKIX_PL_Cert_CheckValidity
                    (targetCert, testDate, plContext);
            if (pkixErrorResult) {
                    pkixErrorClass = pkixErrorResult->errClass;
                    if (pkixErrorClass == PKIX_FATAL_ERROR) {
                        goto cleanup;
                    }
                    if (pVerifyNode != NULL) {
                            PKIX_Error *tempResult =
                                pkix_VerifyNode_Create(targetCert, 0,
                                                       pkixErrorResult,
                                                       pVerifyNode,
                                                       plContext);
                            if (tempResult) {
                                PKIX_DECREF(pkixErrorResult);
                                pkixErrorResult = tempResult;
                                pkixErrorCode = PKIX_VERIFYNODECREATEFAILED;
                                pkixErrorClass = PKIX_FATAL_ERROR;
                                goto cleanup;
                            }
                    }
                    pkixErrorCode = PKIX_CERTCHECKVALIDITYFAILED;
                    goto cleanup;
            }
    
            PKIX_CHECK(pkix_ProcessingParams_GetRevocationEnabled
                    (procParams, &isCrlEnabled, plContext),
                    PKIX_PROCESSINGPARAMSGETREVOCATIONENABLEDFAILED);
    
            PKIX_CHECK(
                pkix_ProcessingParams_GetNISTRevocationPolicyEnabled
                (procParams, &nistCRLPolicyEnabled, plContext),
                PKIX_PROCESSINGPARAMSGETNISTREVPOLICYENABLEDFAILED);


            PKIX_CHECK(PKIX_ProcessingParams_GetCertStores
                    (procParams, &certStores, plContext),
                    PKIX_PROCESSINGPARAMSGETCERTSTORESFAILED);
    
            PKIX_CHECK(PKIX_List_GetLength
                    (certStores, &numCertStores, plContext),
                    PKIX_LISTGETLENGTHFAILED);
    
            /* Reorder CertStores so trusted are at front of the List */
            if (numCertStores > 1) {
                for (i = numCertStores - 1; i > 0; i--) {
                    PKIX_CHECK_ONLY_FATAL(PKIX_List_GetItem
                        (certStores,
                        i,
                        (PKIX_PL_Object **)&certStore,
                        plContext),
                        PKIX_LISTGETITEMFAILED);
                    PKIX_CHECK_ONLY_FATAL(PKIX_CertStore_GetTrustCallback
                        (certStore, &trustCallback, plContext),
                        PKIX_CERTSTOREGETTRUSTCALLBACKFAILED);
    
                    if (trustCallback != NULL) {
                        /* Is a trusted Cert, move CertStore to front */
                        PKIX_CHECK(PKIX_List_DeleteItem
                            (certStores, i, plContext),
                            PKIX_LISTDELETEITEMFAILED);
                        PKIX_CHECK(PKIX_List_InsertItem
                            (certStores,
                            0,
                            (PKIX_PL_Object *)certStore,
                            plContext),
                        PKIX_LISTINSERTITEMFAILED);
    
                    }
    
                    PKIX_DECREF(certStore);
                }
            }
    
            PKIX_CHECK(PKIX_ProcessingParams_GetCertChainCheckers
                        (procParams, &userCheckers, plContext),
                        PKIX_PROCESSINGPARAMSGETCERTCHAINCHECKERSFAILED);
    
            if (isCrlEnabled) {
                    if (numCertStores > 0) {
                            PKIX_CHECK(pkix_DefaultCRLChecker_Initialize
                                    (certStores,
                                    testDate,
                                    NULL,
                                    0,
                                    nistCRLPolicyEnabled,
                                    &crlChecker,
                                    plContext),
                                    PKIX_DEFAULTCRLCHECKERINITIALIZEFAILED);
                    } else {
                        PKIX_ERROR(PKIX_CANTENABLEREVOCATIONWITHOUTCERTSTORE);
                    }
            }
    
            PKIX_CHECK(PKIX_PL_AIAMgr_Create(&aiaMgr, plContext),
                    PKIX_AIAMGRCREATEFAILED);

            /*
             * We initialize all the fields of buildConstants here, in one place,
             * just to help keep track and ensure that we got everything.
             */
    
            buildConstants.numAnchors = numAnchors;
            buildConstants.numCertStores = numCertStores;
            buildConstants.numHintCerts = numHintCerts;
            buildConstants.procParams = procParams;
            buildConstants.testDate = testDate;
            buildConstants.timeLimit = NULL;
            buildConstants.targetCert = targetCert;
            buildConstants.targetPubKey = targetPubKey;
            buildConstants.certStores = certStores;
            buildConstants.anchors = anchors;
            buildConstants.userCheckers = userCheckers;
            buildConstants.hintCerts = hintCerts;
            buildConstants.crlChecker = crlChecker;
            buildConstants.aiaMgr = aiaMgr;
                
            PKIX_CHECK(pkix_Build_GetResourceLimits(&buildConstants, plContext),
                    PKIX_BUILDGETRESOURCELIMITSFAILED);
    
            PKIX_CHECK(pkix_ForwardBuilderState_Create
                    (0,              /* PKIX_UInt32 traversedCACerts */
                    buildConstants.maxFanout,
                    buildConstants.maxDepth,
                    dsaParamsNeeded, /* PKIX_Boolean dsaParamsNeeded */
                    PKIX_FALSE,      /* PKIX_Boolean revCheckDelayed */
                    PKIX_TRUE,       /* PKIX_Boolean canBeCached */
                    NULL,            /* PKIX_Date *validityDate */
                    targetCert,      /* PKIX_PL_Cert *prevCert */
                    targetSubjNames, /* PKIX_List *traversedSubjNames */
                    tentativeChain,  /* PKIX_List *trustChain */
                    NULL,            /* PKIX_ForwardBuilderState *parent */
                    &state,          /* PKIX_ForwardBuilderState **pState */
                    plContext),
                    PKIX_BUILDSTATECREATEFAILED);
    
            state->buildConstants.numAnchors = buildConstants.numAnchors;
            state->buildConstants.numCertStores = buildConstants.numCertStores; 
            state->buildConstants.numHintCerts = buildConstants.numHintCerts;
            state->buildConstants.maxFanout = buildConstants.maxFanout;
            state->buildConstants.maxDepth = buildConstants.maxDepth;
            state->buildConstants.maxTime = buildConstants.maxTime;
            state->buildConstants.procParams = buildConstants.procParams; 
            PKIX_INCREF(buildConstants.testDate);
            state->buildConstants.testDate = buildConstants.testDate;
            state->buildConstants.timeLimit = buildConstants.timeLimit;
            PKIX_INCREF(buildConstants.targetCert);
            state->buildConstants.targetCert = buildConstants.targetCert;
            PKIX_INCREF(buildConstants.targetPubKey);
            state->buildConstants.targetPubKey =
                    buildConstants.targetPubKey;
            PKIX_INCREF(buildConstants.certStores);
            state->buildConstants.certStores = buildConstants.certStores;
            PKIX_INCREF(buildConstants.anchors);
            state->buildConstants.anchors = buildConstants.anchors;
            PKIX_INCREF(buildConstants.userCheckers);
            state->buildConstants.userCheckers =
                    buildConstants.userCheckers;
            PKIX_INCREF(buildConstants.hintCerts);
            state->buildConstants.hintCerts = buildConstants.hintCerts;
            PKIX_INCREF(buildConstants.crlChecker);
            state->buildConstants.crlChecker = buildConstants.crlChecker;
            state->buildConstants.aiaMgr = buildConstants.aiaMgr;
            aiaMgr = NULL;

            if (buildConstants.maxTime != 0) {
                    PKIX_CHECK(PKIX_PL_Date_Create_CurrentOffBySeconds
                            (buildConstants.maxTime,
                            &state->buildConstants.timeLimit,
                            plContext),
                            PKIX_DATECREATECURRENTOFFBYSECONDSFAILED);
            }

            if (pVerifyNode != NULL) {
                PKIX_Error *tempResult =
                    pkix_VerifyNode_Create(targetCert, 0, NULL,
                                           &(state->verifyNode),
                                           plContext);
                if (tempResult) {
                    pkixErrorResult = tempResult;
                    pkixErrorCode = PKIX_VERIFYNODECREATEFAILED;
                    pkixErrorClass = PKIX_FATAL_ERROR;
                    goto cleanup;
                }
            }

            PKIX_CHECK(
                pkix_Build_CheckInCache(state, &buildResult,
                                        &nbioContext, plContext),
                PKIX_UNABLETOBUILDCHAIN);
            if (nbioContext) {
                *pNBIOContext = nbioContext;
                *pState = state;
                state = NULL;
                goto cleanup;
            }
            if (buildResult) {
                *pBuildResult = buildResult;
                if (pVerifyNode != NULL) {
                    *pVerifyNode = state->verifyNode;
                    state->verifyNode = NULL;
                }
                goto cleanup;
            }
        }

        /* If we're resuming after non-blocking I/O we need to get SubjNames */
        if (targetSubjNames == NULL) {
            PKIX_CHECK(PKIX_PL_Cert_GetAllSubjectNames
                    (state->buildConstants.targetCert,
                    &targetSubjNames,
                    plContext),
                    PKIX_CERTGETALLSUBJECTNAMESFAILED);
        }

        /*
         * We can avoid the search if this cert, with any of our trust
         * anchors, forms a complete trust chain.
         */
        PKIX_CHECK_ONLY_FATAL(pkix_Build_TryShortcut
                (state,
                targetSubjNames,
                &nbioContext,
                &matchingAnchor,
                plContext),
                PKIX_BUILDTRYSHORTCUTFAILED);

        if (nbioContext != NULL) {
                *pNBIOContext = nbioContext;
                PKIX_INCREF(state);
                *pState = state;
                goto cleanup;
        }

        state->status = BUILD_INITIAL;

        if (matchingAnchor) {
                PKIX_CHECK(pkix_ValidateResult_Create
                        (state->buildConstants.targetPubKey,
                        matchingAnchor,
                        NULL,
                        &valResult,
                        plContext),
                        PKIX_VALIDATERESULTCREATEFAILED);
        } else {
                PKIX_CHECK(pkix_BuildForwardDepthFirstSearch
                        (&nbioContext, &state, &valResult, plContext),
                        PKIX_BUILDFORWARDDEPTHFIRSTSEARCHFAILED);
        }

        /* non-null nbioContext means the build would block */
        if (nbioContext != NULL) {

                *pNBIOContext = nbioContext;
                *pBuildResult = NULL;

        /* no valResult means the build has failed */
        } else {
                if (pVerifyNode != NULL) {
                        PKIX_INCREF(state->verifyNode);
                        *pVerifyNode = state->verifyNode;
                }

                if (valResult == NULL) {

                        PKIX_DECREF(state);
                        *pState = NULL;
                        PKIX_ERROR(PKIX_UNABLETOBUILDCHAIN);

                } else {

                        PKIX_CHECK(pkix_BuildResult_Create
                                (valResult,
                                state->trustChain,
                                &buildResult,
                                plContext),
                                PKIX_BUILDRESULTCREATEFAILED);

                        *pBuildResult = buildResult;
                }
        }

        *pState = state;
        state = NULL;

cleanup:

        PKIX_DECREF(targetConstraints);
        PKIX_DECREF(targetParams);
        PKIX_DECREF(anchors);
        PKIX_DECREF(targetSubjNames);
        PKIX_DECREF(targetCert);
        PKIX_DECREF(crlChecker);
        PKIX_DECREF(certStores);
        PKIX_DECREF(certStore);
        PKIX_DECREF(userCheckers);
        PKIX_DECREF(hintCerts);
        PKIX_DECREF(firstHintCert);
        PKIX_DECREF(testDate);
        PKIX_DECREF(targetPubKey);
        PKIX_DECREF(tentativeChain);
        PKIX_DECREF(valResult);
        PKIX_DECREF(certList);
        PKIX_DECREF(matchingAnchor);
        PKIX_DECREF(trustedCert);
        PKIX_DECREF(state);
        PKIX_DECREF(aiaMgr);

        PKIX_RETURN(BUILD);
}

/*
 * FUNCTION: pkix_Build_ResumeBuildChain
 * DESCRIPTION:
 *
 *  This function continues the search for a BuildChain, using the parameters
 *  provided in "procParams" and the ForwardBuilderState pointed to by "state".
 *
 *  If a successful chain is built, this function stores the BuildResult at
 *  "pBuildResult". Alternatively, if an operation using non-blocking I/O
 *  is in progress and the operation has not been completed, this function
 *  stores the FowardBuilderState at "pState" and NULL at "pBuildResult".
 *  Finally, if chain building was unsuccessful, this function stores NULL
 *  at both "pState" and at "pBuildResult".
 *
 * PARAMETERS:
 *  "pNBIOContext"
 *      Address at which the NBIOContext is stored indicating whether the
 *      validation is complete. Must be non-NULL.
 *  "pState"
 *     Address at which the ForwardBuilderState is provided for resumption of
 *     the chain building attempt; also, the address at which the
 *     ForwardBuilderStateis stored, if the chain building is suspended for
 *     waiting I/O. Must be non-NULL.
 *  "pBuildResult"
 *      Address at which the BuildResult is stored, after a successful build.
 *      Must be non-NULL.
 *  "plContext"
 *      Platform-specific context pointer.
 * THREAD SAFETY:
 *  Thread Safe (see Thread Safety Definitions in Programmer's Guide)
 * RETURNS:
 *  Returns NULL if the function succeeds.
 *  Returns a Build Error if the function fails in a non-fatal way
 *  Returns a Fatal Error if the function fails in an unrecoverable way.
 */
static PKIX_Error *
pkix_Build_ResumeBuildChain(
        void **pNBIOContext,
        PKIX_ForwardBuilderState **pState,
        PKIX_BuildResult **pBuildResult,
        PKIX_VerifyNode **pVerifyNode,
        void *plContext)
{
        PKIX_ForwardBuilderState *state = NULL;
        PKIX_ValidateResult *valResult = NULL;
        PKIX_BuildResult *buildResult = NULL;
        void *nbioContext = NULL;

        PKIX_ENTER(BUILD, "pkix_Build_ResumeBuildChain");
        PKIX_NULLCHECK_THREE(pState, *pState, pBuildResult);

        nbioContext = *pNBIOContext;
        *pNBIOContext = NULL;

        state = *pState;

        PKIX_CHECK(pkix_BuildForwardDepthFirstSearch
                (&nbioContext, &state, &valResult, plContext),
                PKIX_BUILDFORWARDDEPTHFIRSTSEARCHFAILED);

        /* non-null nbioContext means the build would block */
        if (nbioContext != NULL) {

                *pNBIOContext = nbioContext;
                *pBuildResult = NULL;

        /* no valResult means the build has failed */
        } else if (valResult == NULL) {

                PKIX_DECREF(state);
                *pState = NULL;
                PKIX_ERROR(PKIX_UNABLETOBUILDCHAIN);

        } else {

                PKIX_CHECK(pkix_BuildResult_Create
                        (valResult,
                        state->trustChain,
                        &buildResult,
                        plContext),
                        PKIX_BUILDRESULTCREATEFAILED);

                *pBuildResult = buildResult;
        }

        *pState = state;

cleanup:

        PKIX_DECREF(valResult);

        PKIX_RETURN(BUILD);
}

/* --Public-Functions--------------------------------------------- */

/*
 * FUNCTION: PKIX_BuildChain (see comments in pkix.h)
 */
PKIX_Error *
PKIX_BuildChain(
        PKIX_ProcessingParams *procParams,
        void **pNBIOContext,
        void **pState,
        PKIX_BuildResult **pBuildResult,
        PKIX_VerifyNode **pVerifyNode,
        void *plContext)
{
        PKIX_ForwardBuilderState *state = NULL;
        PKIX_BuildResult *buildResult = NULL;
        void *nbioContext = NULL;

        PKIX_ENTER(BUILD, "PKIX_BuildChain");
        PKIX_NULLCHECK_FOUR(procParams, pNBIOContext, pState, pBuildResult);

        nbioContext = *pNBIOContext;
        *pNBIOContext = NULL;

        if (*pState == NULL) {
                PKIX_CHECK(pkix_Build_InitiateBuildChain
                        (procParams,
                        &nbioContext,
                        &state,
                        &buildResult,
                        pVerifyNode,
                        plContext),
                        PKIX_BUILDINITIATEBUILDCHAINFAILED);
        } else {
                state = (PKIX_ForwardBuilderState *)(*pState);
                *pState = NULL; /* no net change in reference count */
                if (state->status == BUILD_SHORTCUTPENDING) {
                        PKIX_CHECK(pkix_Build_InitiateBuildChain
                                (procParams,
                                &nbioContext,
                                &state,
                                &buildResult,
                                pVerifyNode,
                                plContext),
                                PKIX_BUILDINITIATEBUILDCHAINFAILED);
                } else {
                        PKIX_CHECK(pkix_Build_ResumeBuildChain
                                (&nbioContext,
                                &state,
                                &buildResult,
                                pVerifyNode,
                                plContext),
                                PKIX_BUILDINITIATEBUILDCHAINFAILED);
                }
        }

        /* non-null nbioContext means the build would block */
        if (nbioContext != NULL) {

                *pNBIOContext = nbioContext;
                *pState = state;
                state = NULL;
                *pBuildResult = NULL;

        /* no buildResult means the build has failed */
        } else if (buildResult == NULL) {
                PKIX_ERROR(PKIX_UNABLETOBUILDCHAIN);
        } else {
                /*
                 * If we made a successful chain by combining the target Cert
                 * with one of the Trust Anchors, we may have never created a
                 * validityDate. We treat this situation as
                 * canBeCached = PKIX_FALSE.
                 */
                if ((state != NULL) &&
                    ((state->validityDate) != NULL) &&
                    (state->canBeCached)) {
                        PKIX_CHECK(pkix_CacheCertChain_Add
                                (state->buildConstants.targetCert,
                                state->buildConstants.anchors,
                                state->validityDate,
                                buildResult,
                                plContext),
                                PKIX_CACHECERTCHAINADDFAILED);
                }

                *pState = NULL;
                *pBuildResult = buildResult;
                buildResult = NULL;
        }

cleanup:
        PKIX_DECREF(buildResult);
        PKIX_DECREF(state);

        PKIX_RETURN(BUILD);
}
