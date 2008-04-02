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
 * pkix_pl_infoaccess.c
 *
 * InfoAccess Object Definitions
 *
 */

#include "pkix_pl_infoaccess.h"

/* XXX Following SEC_OID_PKIX defines should be merged in NSS */
#define SEC_OID_PKIX_CA_REPOSITORY     1003
#define SEC_OID_PKIX_TIMESTAMPING      1005
/* XXX Following OID defines hould be moved to NSS */
static const unsigned char siaTimeStampingOID[] = {0x2b, 0x06, 0x01, 0x05,
                                0x05, 0x07, 0x030, 0x03};
static const unsigned char siaCaRepositoryOID[] = {0x2b, 0x06, 0x01, 0x05,
                                0x05, 0x07, 0x030, 0x05};


/* --Private-InfoAccess-Functions----------------------------------*/

/*
 * FUNCTION: pkix_pl_InfoAccess_Create
 * DESCRIPTION:
 *
 *  This function creates an InfoAccess from the method provided in "method" and
 *  the GeneralName provided in "generalName" and stores the result at
 *  "pInfoAccess".
 *
 * PARAMETERS
 *  "method"
 *      The UInt32 value to be stored as the method field of the InfoAccess.
 *  "generalName"
 *      The GeneralName to be stored as the generalName field of the InfoAccess.
 *      Must be non-NULL.
 *  "pInfoAccess"
 *      Address where the result is stored. Must be non-NULL.
 *  "plContext"
 *      Platform-specific context pointer.
 * THREAD SAFETY:
 *  Thread Safe (see Thread Safety Definitions in Programmer's Guide)
 * RETURNS:
 *  Returns NULL if the function succeeds.
 *  Returns a Fatal Error if the function fails in an unrecoverable way.
 */
static PKIX_Error *
pkix_pl_InfoAccess_Create(
        PKIX_UInt32 method,
        PKIX_PL_GeneralName *generalName,
        PKIX_PL_InfoAccess **pInfoAccess,
        void *plContext)
{

        PKIX_PL_InfoAccess *infoAccess = NULL;

        PKIX_ENTER(INFOACCESS, "pkix_pl_InfoAccess_Create");
        PKIX_NULLCHECK_TWO(generalName, pInfoAccess);

        PKIX_CHECK(PKIX_PL_Object_Alloc
                (PKIX_INFOACCESS_TYPE,
                sizeof (PKIX_PL_InfoAccess),
                (PKIX_PL_Object **)&infoAccess,
                plContext),
                PKIX_COULDNOTCREATEINFOACCESSOBJECT);

        infoAccess->method = method;

        PKIX_INCREF(generalName);
        infoAccess->location = generalName;

        *pInfoAccess = infoAccess;
        infoAccess = NULL;

cleanup:
        PKIX_DECREF(infoAccess);

        PKIX_RETURN(INFOACCESS);
}

/*
 * FUNCTION: pkix_pl_InfoAccess_Destroy
 * (see comments for PKIX_PL_DestructorCallback in pkix_pl_pki.h)
 */
static PKIX_Error *
pkix_pl_InfoAccess_Destroy(
        PKIX_PL_Object *object,
        void *plContext)
{
        PKIX_PL_InfoAccess *infoAccess = NULL;

        PKIX_ENTER(INFOACCESS, "pkix_pl_InfoAccess_Destroy");
        PKIX_NULLCHECK_ONE(object);

        PKIX_CHECK(pkix_CheckType(object, PKIX_INFOACCESS_TYPE, plContext),
                PKIX_OBJECTNOTANINFOACCESS);

        infoAccess = (PKIX_PL_InfoAccess *)object;

        PKIX_DECREF(infoAccess->location);

cleanup:

        PKIX_RETURN(INFOACCESS);
}

/*
 * FUNCTION: pkix_pl_InfoAccess_ToString
 * (see comments for PKIX_PL_ToStringCallback in pkix_pl_pki.h)
 */
static PKIX_Error *
pkix_pl_InfoAccess_ToString(
        PKIX_PL_Object *object,
        PKIX_PL_String **pString,
        void *plContext)
{
        PKIX_PL_InfoAccess *infoAccess;
        PKIX_PL_String *infoAccessString = NULL;
        char *asciiFormat = NULL;
        char *asciiMethod = NULL;
        PKIX_PL_String *formatString = NULL;
        PKIX_PL_String *methodString = NULL;
        PKIX_PL_String *locationString = NULL;

        PKIX_ENTER(INFOACCESS, "pkix_pl_InfoAccess_ToString");
        PKIX_NULLCHECK_TWO(object, pString);

        PKIX_CHECK(pkix_CheckType
                    (object, PKIX_INFOACCESS_TYPE, plContext),
                    PKIX_OBJECTNOTINFOACCESS);

        infoAccess = (PKIX_PL_InfoAccess *)object;

        asciiFormat =
                "["
                "method:%s, "
                "location:%s"
                "]";

        PKIX_CHECK(PKIX_PL_String_Create
                    (PKIX_ESCASCII,
                    asciiFormat,
                    0,
                    &formatString,
                    plContext),
                    PKIX_STRINGCREATEFAILED);

        switch(infoAccess->method) {
            case PKIX_INFOACCESS_CA_ISSUERS:
                    asciiMethod = "caIssuers";
                    break;
            case PKIX_INFOACCESS_OCSP:
                    asciiMethod = "ocsp";
                    break;
            case PKIX_INFOACCESS_TIMESTAMPING:
                    asciiMethod = "timestamping";
                    break;
            case PKIX_INFOACCESS_CA_REPOSITORY:
                    asciiMethod = "caRepository";
                    break;
            default:
                    asciiMethod = "unknown";
        }

        PKIX_CHECK(PKIX_PL_String_Create
                    (PKIX_ESCASCII,
                    asciiMethod,
                    0,
                    &methodString,
                    plContext),
                    PKIX_STRINGCREATEFAILED);

        PKIX_TOSTRING(infoAccess->location, &locationString, plContext,
                    PKIX_GENERALNAMETOSTRINGFAILED);

        PKIX_CHECK(PKIX_PL_Sprintf
                    (&infoAccessString,
                    plContext,
                    formatString,
                    methodString,
                    locationString),
                    PKIX_SPRINTFFAILED);

        *pString = infoAccessString;

cleanup:

        PKIX_DECREF(formatString);
        PKIX_DECREF(methodString);
        PKIX_DECREF(locationString);

        PKIX_RETURN(INFOACCESS);
}

/*
 * FUNCTION: pkix_pl_InfoAccess_Hashcode
 * (see comments for PKIX_PL_HashcodeCallback in pkix_pl_pki.h)
 */
static PKIX_Error *
pkix_pl_InfoAccess_Hashcode(
        PKIX_PL_Object *object,
        PKIX_UInt32 *pHashcode,
        void *plContext)
{
        PKIX_PL_InfoAccess *infoAccess = NULL;
        PKIX_UInt32 infoAccessHash;

        PKIX_ENTER(INFOACCESS, "pkix_pl_InfoAccess_Hashcode");
        PKIX_NULLCHECK_TWO(object, pHashcode);

        PKIX_CHECK(pkix_CheckType
                    (object, PKIX_INFOACCESS_TYPE, plContext),
                    PKIX_OBJECTNOTINFOACCESS);

        infoAccess = (PKIX_PL_InfoAccess *)object;

        PKIX_HASHCODE(infoAccess->location, &infoAccessHash, plContext,
                    PKIX_OBJECTHASHCODEFAILED);

        infoAccessHash += (infoAccess->method << 7);

        *pHashcode = infoAccessHash;

cleanup:

        PKIX_RETURN(INFOACCESS);

}

/*
 * FUNCTION: pkix_pl_InfoAccess_Equals
 * (see comments for PKIX_PL_Equals_Callback in pkix_pl_pki.h)
 */
static PKIX_Error *
pkix_pl_InfoAccess_Equals(
        PKIX_PL_Object *firstObject,
        PKIX_PL_Object *secondObject,
        PKIX_Boolean *pResult,
        void *plContext)
{
        PKIX_PL_InfoAccess *firstInfoAccess = NULL;
        PKIX_PL_InfoAccess *secondInfoAccess = NULL;
        PKIX_UInt32 secondType;
        PKIX_Boolean cmpResult;

        PKIX_ENTER(INFOACCESS, "pkix_pl_InfoAccess_Equals");
        PKIX_NULLCHECK_THREE(firstObject, secondObject, pResult);

        /* test that firstObject is a InfoAccess */
        PKIX_CHECK(pkix_CheckType
                (firstObject, PKIX_INFOACCESS_TYPE, plContext),
                PKIX_FIRSTOBJECTNOTINFOACCESS);

        /*
         * Since we know firstObject is a InfoAccess, if both references are
         * identical, they must be equal
         */
        if (firstObject == secondObject){
                *pResult = PKIX_TRUE;
                goto cleanup;
        }

        /*
         * If secondObject isn't a InfoAccess, we don't throw an error.
         * We simply return a Boolean result of FALSE
         */
        *pResult = PKIX_FALSE;
        PKIX_CHECK(PKIX_PL_Object_GetType
                    (secondObject, &secondType, plContext),
                    PKIX_COULDNOTGETTYPEOFSECONDARGUMENT);
        if (secondType != PKIX_INFOACCESS_TYPE) goto cleanup;

        firstInfoAccess = (PKIX_PL_InfoAccess *)firstObject;
        secondInfoAccess = (PKIX_PL_InfoAccess *)secondObject;

        *pResult = PKIX_FALSE;

        if (firstInfoAccess->method != secondInfoAccess->method) {
                goto cleanup;
        }

        PKIX_EQUALS(firstInfoAccess, secondInfoAccess, &cmpResult, plContext,
                PKIX_OBJECTEQUALSFAILED);

        *pResult = cmpResult;

cleanup:

        PKIX_RETURN(INFOACCESS);
}

/*
 * FUNCTION: pkix_pl_InfoAccess_RegisterSelf
 * DESCRIPTION:
 *  Registers PKIX_INFOACCESS_TYPE and its related functions with systemClasses[]
 * THREAD SAFETY:
 *  Not Thread Safe - for performance and complexity reasons
 *
 *  Since this function is only called by PKIX_PL_Initialize, which should
 *  only be called once, it is acceptable that this function is not
 *  thread-safe.
 */
PKIX_Error *
pkix_pl_InfoAccess_RegisterSelf(void *plContext)
{
        extern pkix_ClassTable_Entry systemClasses[PKIX_NUMTYPES];
        pkix_ClassTable_Entry entry;

        PKIX_ENTER(INFOACCESS,
                "pkix_pl_InfoAccess_RegisterSelf");

        entry.description = "InfoAccess";
        entry.objCounter = 0;
        entry.typeObjectSize = sizeof(PKIX_PL_InfoAccess);
        entry.destructor = pkix_pl_InfoAccess_Destroy;
        entry.equalsFunction = pkix_pl_InfoAccess_Equals;
        entry.hashcodeFunction = pkix_pl_InfoAccess_Hashcode;
        entry.toStringFunction = pkix_pl_InfoAccess_ToString;
        entry.comparator = NULL;
        entry.duplicateFunction = pkix_duplicateImmutable;

        systemClasses[PKIX_INFOACCESS_TYPE] = entry;

        PKIX_RETURN(INFOACCESS);
}

/*
 * FUNCTION: pkix_pl_InfoAccess_CreateList
 * DESCRIPTION:
 *
 *  Based on data in CERTAuthInfoAccess array "nssInfoAccess", this function
 *  creates and returns a PKIX_List of PKIX_PL_InfoAccess at "pInfoAccessList".
 *
 * PARAMETERS
 *  "nssInfoAccess"
 *      The pointer array of CERTAuthInfoAccess that contains access data.
 *      May be NULL.
 *  "pInfoAccessList"
 *      Address where a list of PKIX_PL_InfoAccess is returned.
 *      Must be non-NULL.
 *  "plContext"
 *      Platform-specific context pointer.
 * THREAD SAFETY:
 *  Thread Safe (see Thread Safety Definitions in Programmer's Guide)
 * RETURNS:
 *  Returns NULL if the function succeeds.
 *  Returns a Fatal Error if the function fails in an unrecoverable way.
 */
PKIX_Error *
pkix_pl_InfoAccess_CreateList(
        CERTAuthInfoAccess **nssInfoAccess,
        PKIX_List **pInfoAccessList, /* of PKIX_PL_InfoAccess */
        void *plContext)
{
        PKIX_List *infoAccessList = NULL;
        PKIX_PL_InfoAccess *infoAccess = NULL;
        PKIX_PL_GeneralName *location = NULL;
        PKIX_UInt32 method;
        int i;

        PKIX_ENTER(INFOACCESS, "PKIX_PL_InfoAccess_CreateList");
        PKIX_NULLCHECK_ONE(pInfoAccessList);

        PKIX_CHECK(PKIX_List_Create(&infoAccessList, plContext),
                PKIX_LISTCREATEFAILED);

        if (nssInfoAccess == NULL) {
                goto cleanup;
        }

        for (i = 0; nssInfoAccess[i] != NULL; i++) {

                if (nssInfoAccess[i]->location == NULL) {
                    continue;
                }

                PKIX_CHECK(pkix_pl_GeneralName_Create
                        (nssInfoAccess[i]->location, &location, plContext),
                        PKIX_GENERALNAMECREATEFAILED);

                PKIX_CERT_DEBUG("\t\tCalling SECOID_FindOIDTag).\n");
                method = SECOID_FindOIDTag(&nssInfoAccess[i]->method);

                if (method == 0) {

                /* XXX
                 * This part of code is definitely hacking, need NSS decode
                 * support. We can reuse the CERT_DecodeAuthInfoAccessExtension
                 * since SIA and AIA are all the same type. However NSS need
                 * to add SIA, CaRepository, TimeStamping OID definitions and
                 * the numerical method, timeStamping and caRepository values.
                 *
                 * We assume now, since method is 0, implies the method for SIA
                 * was not decoded by CERT_DecodeAuthInfoAccessExtension()
                 * so we compare and put value in. This part should be taken
                 * out eventually if CERT_DecodeInfoAccessExtension (*renamed*)
                 * is doing the job.
                 */

                        PKIX_CERT_DEBUG("\t\tCalling PORT_Strncmp).\n");
                        if (PORT_Strncmp
                                ((char *)nssInfoAccess[i]->method.data,
                                (char *)siaTimeStampingOID,
                                nssInfoAccess[i]->method.len) == 0) {
                                method = SEC_OID_PKIX_TIMESTAMPING;
                        } else if (PORT_Strncmp
                                ((char *)nssInfoAccess[i]->method.data,
                                (char *)siaCaRepositoryOID,
                                nssInfoAccess[i]->method.len) == 0) {
                                method = SEC_OID_PKIX_CA_REPOSITORY;
                        }
                }

                /* Map NSS access method value into PKIX constant */
                switch(method) {
                        case SEC_OID_PKIX_CA_ISSUERS:
                                method = PKIX_INFOACCESS_CA_ISSUERS;
                                break;
                        case SEC_OID_PKIX_OCSP:
                                method = PKIX_INFOACCESS_OCSP;
                                break;
                        case SEC_OID_PKIX_TIMESTAMPING:
                                method = PKIX_INFOACCESS_TIMESTAMPING;
                                break;
                        case SEC_OID_PKIX_CA_REPOSITORY:
                                method = PKIX_INFOACCESS_CA_REPOSITORY;
                                break;
                        default:
                                break;
                }

                PKIX_CHECK(pkix_pl_InfoAccess_Create
                        (method, location, &infoAccess, plContext),
                        PKIX_INFOACCESSCREATEFAILED);

                PKIX_CHECK(PKIX_List_AppendItem
                            (infoAccessList,
                            (PKIX_PL_Object *)infoAccess,
                            plContext),
                            PKIX_LISTAPPENDITEMFAILED);
                PKIX_DECREF(infoAccess);
                PKIX_DECREF(location);
        }

        *pInfoAccessList = infoAccessList;
        infoAccessList = NULL;

cleanup:

        PKIX_DECREF(infoAccessList);
        PKIX_DECREF(infoAccess);
        PKIX_DECREF(location);

        PKIX_RETURN(INFOACCESS);
}

/* --Public-Functions------------------------------------------------------- */

/*
 * FUNCTION: PKIX_PL_InfoAccess_GetMethod (see comments in pkix_pl_pki.h)
 */
PKIX_Error *
PKIX_PL_InfoAccess_GetMethod(
        PKIX_PL_InfoAccess *infoAccess,
        PKIX_UInt32 *pMethod,
        void *plContext)
{
        PKIX_ENTER(INFOACCESS, "PKIX_PL_InfoAccess_GetMethod");
        PKIX_NULLCHECK_TWO(infoAccess, pMethod);

        *pMethod = infoAccess->method;

        PKIX_RETURN(INFOACCESS);
}

/*
 * FUNCTION: PKIX_PL_InfoAccess_GetLocation (see comments in pkix_pl_pki.h)
 */
PKIX_Error *
PKIX_PL_InfoAccess_GetLocation(
        PKIX_PL_InfoAccess *infoAccess,
        PKIX_PL_GeneralName **pLocation,
        void *plContext)
{
        PKIX_ENTER(INFOACCESS, "PKIX_PL_InfoAccess_GetLocation");
        PKIX_NULLCHECK_TWO(infoAccess, pLocation);

        PKIX_INCREF(infoAccess->location);

        *pLocation = infoAccess->location;

cleanup:
        PKIX_RETURN(INFOACCESS);
}

/*
 * FUNCTION: PKIX_PL_InfoAccess_GetLocationType (see comments in pkix_pl_pki.h)
 */
PKIX_Error *
PKIX_PL_InfoAccess_GetLocationType(
        PKIX_PL_InfoAccess *infoAccess,
        PKIX_UInt32 *pType,
        void *plContext)
{
        PKIX_PL_String *locationString = NULL;
        PKIX_UInt32 type = PKIX_INFOACCESS_LOCATION_UNKNOWN;
        PKIX_UInt32 len = 0;
        void *location = NULL;

        PKIX_ENTER(INFOACCESS, "PKIX_PL_InfoAccess_GetLocationType");
        PKIX_NULLCHECK_TWO(infoAccess, pType);

        if (infoAccess->location != NULL) {

                PKIX_TOSTRING(infoAccess->location, &locationString, plContext,
                    PKIX_GENERALNAMETOSTRINGFAILED);

                PKIX_CHECK(PKIX_PL_String_GetEncoded
                    (locationString, PKIX_ESCASCII, &location, &len, plContext),
                    PKIX_STRINGGETENCODEDFAILED);

                PKIX_OID_DEBUG("\tCalling PORT_Strcmp).\n");
                if (PORT_Strncmp(location, "ldap:", 5) == 0){
                        type = PKIX_INFOACCESS_LOCATION_LDAP;
                } else
                if (PORT_Strncmp(location, "http:", 5) == 0){
                        type = PKIX_INFOACCESS_LOCATION_HTTP;
                }
        }

        *pType = type;

cleanup:

        PKIX_PL_Free(location, plContext);
        PKIX_DECREF(locationString);

        PKIX_RETURN(INFOACCESS);
}

/*
 * FUNCTION: pkix_pl_InfoAccess_ParseTokens
 * DESCRIPTION:
 *
 *  This function parses the string beginning at "startPos" into tokens using
 *  the separator contained in "separator" and the terminator contained in
 *  "terminator", copying the tokens into space allocated from the arena
 *  pointed to by "arena". It stores in "tokens" a null-terminated array of
 *  pointers to those tokens.
 *
 * PARAMETERS
 *  "arena"
 *      Address of a PRArenaPool to be used in populating the LDAPLocation.
 *      Must be non-NULL.
 *  "startPos"
 *      The address of char string that contains a subset of ldap location.
 *  "tokens"
 *      The address of an array of char string for storing returned tokens.
 *      Must be non-NULL.
 *  "separator"
 *      The character that is taken as token separator. Must be non-NULL.
 *  "terminator"
 *      The character that is taken as parsing terminator. Must be non-NULL.
 *  "plContext"
 *      Platform-specific context pointer.
 * THREAD SAFETY:
 *  Thread Safe (see Thread Safety Definitions in Programmer's Guide)
 * RETURNS:
 *  Returns NULL if the function succeeds.
 *  Returns an InfoAccess Error if the function fails in a non-fatal way.
 *  Returns a Fatal Error if the function fails in an unrecoverable way.
 */
static PKIX_Error *
pkix_pl_InfoAccess_ParseTokens(
        PRArenaPool *arena,
        char **startPos, /* return update */
        char ***tokens,
        char separator,
        char terminator,
        void *plContext)
{
        PKIX_UInt32 len = 0;
        PKIX_UInt32 numFilters = 0;
        PKIX_Int32 cmpResult = -1;
        char *endPos = NULL;
        char *p = NULL;
        char **filterP = NULL;

        PKIX_ENTER(INFOACCESS, "pkix_pl_InfoAccess_ParseTokens");
        PKIX_NULLCHECK_THREE(arena, startPos, tokens);

        endPos = *startPos;

        /* First pass: parse to <terminator> to count number of components */
        numFilters = 0;
        while (*endPos != terminator && *endPos != '\0') {
                endPos++;
                if (*endPos == separator) {
                        numFilters++;
                }
        }

        if (*endPos != terminator) {
                PKIX_ERROR(PKIX_LOCATIONSTRINGNOTPROPERLYTERMINATED);
        }

        /* Last one doesn't have a "," as separator, although we allow it */
        if (*(endPos-1) != ',') {
                numFilters++;
        }

        /*
         * If string is a=xx, b=yy, c=zz, etc., use a=xx for filter,
         * and everything else for the base
         */
        if (numFilters > 2) numFilters = 2;

        PKIX_PL_NSSCALLRV
                (INFOACCESS, *tokens, PORT_ArenaZAlloc,
                (arena, (numFilters+1)*sizeof(void *)));

        /* Second pass: parse to fill in components in token array */
        filterP = *tokens;
        endPos = *startPos;

        while (numFilters) {
            if (*endPos == separator || *endPos == terminator) {
                    len = endPos - *startPos;
                    PKIX_PL_NSSCALLRV(INFOACCESS, p, PORT_ArenaZAlloc,
                            (arena, (len+1)));

                    *filterP = p;

                    while (len) {
                            if (**startPos == '%') {
                            /* replace %20 by blank */
                                PKIX_PL_NSSCALLRV(INFOACCESS, cmpResult,
                                    strncmp, ((void *)*startPos, "%20", 3));
                                if (cmpResult == 0) {
                                    *p = ' ';
                                    *startPos += 3;
                                    len -= 3;
                                }
                            } else {
                                *p = **startPos;
                                (*startPos)++;
                                len--;
                            }
                            p++;
                    }

                    *p = '\0';
                    filterP++;
                    numFilters--;

                    separator = terminator;

                    if (endPos == '\0') {
                        break;
                    } else {
                        endPos++;
                        *startPos = endPos;
                        continue;
                    }
            }
            endPos++;
        }

        *filterP = NULL;

cleanup:

        PKIX_RETURN(INFOACCESS);
}

/*
 * FUNCTION: pkix_pl_InfoAccess_ParseLocation
 * DESCRIPTION:
 *
 *  This function parses the GeneralName pointed to by "generalName" into the
 *  fields of the LDAPRequestParams pointed to by "request" and a domainName
 *  pointed to by "pDomainName", using the PRArenaPool pointed to by "arena" to
 *  allocate storage for the request components and for the domainName string.
 *
 *  The expected GeneralName string should be in the format described by the
 *  following BNF:
 *
 *  ldap://<ldap-server-site>/[cn=<cname>][,o=<org>][,c=<country>]?
 *  [caCertificate|crossCertificatPair|certificateRevocationList];
 *  [binary|<other-type>]
 *  [[,caCertificate|crossCertificatPair|certificateRevocationList]
 *   [binary|<other-type>]]*
 *
 * PARAMETERS
 *  "generalName"
 *      Address of the GeneralName whose LDAPLocation is to be parsed. Must be
 *      non-NULL.
 *  "arena"
 *      Address of PRArenaPool to be used for the domainName and for components
 *      of the LDAPRequest. Must be non-NULL.
 *  "request"
 *      Address of the LDAPRequestParams into which request components are
 *      stored. Must be non-NULL.
 *  *pDomainName"
 *      Address at which the domainName is stored. Must be non-NULL.
 *  "plContext"
 *      Platform-specific context pointer.
 * THREAD SAFETY:
 *  Thread Safe (see Thread Safety Definitions in Programmer's Guide)
 * RETURNS:
 *  Returns NULL if the function succeeds.
 *  Returns an InfoAccess Error if the function fails in a non-fatal way.
 *  Returns a Fatal Error if the function fails in an unrecoverable way.
 */
PKIX_Error *
pkix_pl_InfoAccess_ParseLocation(
        PKIX_PL_GeneralName *generalName,
        PRArenaPool *arena,
        LDAPRequestParams *request,
        char **pDomainName,
        void *plContext)
{
        PKIX_PL_String *locationString = NULL;
        PKIX_UInt32 len = 0;
        PKIX_UInt32 ncIndex = 0;
        char *domainName = NULL;
        char **avaArray = NULL;
        char **attrArray = NULL;
        char *attr = NULL;
        char *locationAscii = NULL;
        char *startPos = NULL;
        char *endPos = NULL;
        char *avaPtr = NULL;
        LdapAttrMask attrBit = 0;
        LDAPNameComponent **setOfNameComponent = NULL;
        LDAPNameComponent *nameComponent = NULL;
        void *v = NULL;

        PKIX_ENTER(INFOACCESS, "pkix_pl_InfoAccess_ParseLocation");
        PKIX_NULLCHECK_FOUR(generalName, arena, request, pDomainName);

        PKIX_TOSTRING(generalName, &locationString, plContext,
                PKIX_GENERALNAMETOSTRINGFAILED);

        PKIX_CHECK(PKIX_PL_String_GetEncoded
                (locationString,
                PKIX_ESCASCII,
                (void **)&locationAscii,
                &len,
                plContext),
                PKIX_STRINGGETENCODEDFAILED);

#if 0
        /* For testing inside the firewall... */
        locationAscii = "ldap://nss.red.iplanet.com:1389/cn=Good%20CA,o="
                "Test%20Certificates,c=US?caCertificate;binary";
#endif

        /* Skip "ldap:" */
        endPos = locationAscii;
        while (*endPos != ':' && *endPos != '\0') {
                endPos++;
        }
        if (*endPos == '\0') {
                PKIX_ERROR(PKIX_GENERALNAMESTRINGMISSINGLOCATIONTYPE);
        }

        /* Skip "//" */
        endPos++;
        if (*endPos != '\0' && *(endPos+1) != '0' &&
            *endPos == '/' && *(endPos+1) == '/') {
                endPos += 2;
        } else {
                PKIX_ERROR(PKIX_GENERALNAMESTRINGMISSINGDOUBLESLASH);
        }

        /* Get the server-site */
        startPos = endPos;
        while(*endPos != '/' && *(endPos) != '\0') {
                endPos++;
        }
        if (*endPos == '\0') {
                PKIX_ERROR(PKIX_GENERALNAMESTRINGMISSINGSERVERSITE);
        }

        len = endPos - startPos;
        endPos++;

        PKIX_PL_NSSCALLRV(INFOACCESS, domainName, PORT_ArenaZAlloc,
                (arena, len + 1));

        PKIX_PL_NSSCALL(INFOACCESS, PORT_Memcpy, (domainName, startPos, len));

        domainName[len] = '\0';

        *pDomainName = domainName;

        /*
         * Get a list of AttrValueAssertions (such as 
         * "cn=CommonName, o=Organization, c=US" into a null-terminated array
         */
        startPos = endPos;
        PKIX_CHECK(pkix_pl_InfoAccess_ParseTokens
                (arena,
                &startPos,
                (char ***) &avaArray,
                ',',
                '?',
                plContext),
                PKIX_INFOACCESSPARSETOKENSFAILED);

        /* Count how many AVAs we have */
        for (len = 0; avaArray[len] != NULL; len++) {}

        if (len < 2) {
                PKIX_ERROR(PKIX_NOTENOUGHNAMECOMPONENTSINGENERALNAME);
        }

        /* Use last name component for baseObject */
        request->baseObject = avaArray[len - 1];

        /* Use only one component for filter. LDAP servers aren't too smart. */
        len = 2;   /* Eliminate this when servers get smarter. */

        avaArray[len - 1] = NULL;

        /* Get room for null-terminated array of (LdapNameComponent *) */
        PKIX_PL_NSSCALLRV
                (INFOACCESS, v, PORT_ArenaZAlloc,
                (arena, len*sizeof(LDAPNameComponent *)));

        setOfNameComponent = (LDAPNameComponent **)v;

        /* Get room for the remaining LdapNameComponents */
        PKIX_PL_NSSCALLRV
                (INFOACCESS, v, PORT_ArenaZNewArray,
                (arena, LDAPNameComponent, --len));

        nameComponent = (LDAPNameComponent *)v;

        /* Convert remaining AVAs to LDAPNameComponents */
        for (ncIndex = 0; ncIndex < len; ncIndex ++) {
                setOfNameComponent[ncIndex] = nameComponent;
                avaPtr = avaArray[ncIndex];
                nameComponent->attrType = (unsigned char *)avaPtr;
                while ((*avaPtr != '=') && (*avaPtr != '\0')) {
                        avaPtr++;
                        if (avaPtr == '\0') {
                                PKIX_ERROR(PKIX_NAMECOMPONENTWITHNOEQ);
                        }
                }
                *(avaPtr++) = '\0';
                nameComponent->attrValue = (unsigned char *)avaPtr;
                nameComponent++;
        }

        setOfNameComponent[len] = NULL;
        request->nc = setOfNameComponent;
                
        /*
         * Get a list of AttrTypes (such as 
         * "caCertificate;binary, crossCertificatePair;binary") into
         * a null-terminated array
         */

        PKIX_CHECK(pkix_pl_InfoAccess_ParseTokens
                (arena,
                (char **) &startPos,
                (char ***) &attrArray,
                ',',
                '\0',
                plContext),
                PKIX_INFOACCESSPARSETOKENSFAILED);

        /* Convert array of Attr Types into a bit mask */
        request->attributes = 0;
        attr = attrArray[0];
        while (attr != NULL) {
                PKIX_CHECK(pkix_pl_LdapRequest_AttrStringToBit
                        (attr, &attrBit, plContext),
                        PKIX_LDAPREQUESTATTRSTRINGTOBITFAILED);
                request->attributes |= attrBit;
                attr = *(++attrArray);
        }

cleanup:

        PKIX_PL_Free(locationAscii, plContext);
        PKIX_DECREF(locationString);

        PKIX_RETURN(INFOACCESS);
}
