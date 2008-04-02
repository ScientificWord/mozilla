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
 *   Red Hat, Inc.
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
 * This file defines the types in the libpkix API.
 * XXX Maybe we should specify the API version number in all API header files
 *
 */

#ifndef _PKIXT_H
#define _PKIXT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "secerr.h"

/* Types
 *
 * This header file provides typedefs for the abstract types used by libpkix.
 * It also provides several useful macros.
 *
 * Note that all these abstract types are typedef'd as opaque structures. This
 * is intended to discourage the caller from looking at the contents directly,
 * since the format of the contents may change from one version of the library
 * to the next. Instead, callers should only access these types using the
 * functions defined in the public header files.
 *
 * An instance of an abstract type defined in this file is called an "object"
 * here, although C does not have real support for objects.
 *
 * Because C does not typically have automatic garbage collection, the caller
 * is expected to release the reference to any object that they create or that
 * is returned to them by a libpkix function. The caller should do this by
 * using the PKIX_PL_Object_DecRef function. Note that the caller should not
 * release the reference to an object if the object has been passed to a
 * libpkix function and that function has not returned.
 *
 * Please refer to libpkix Programmer's Guide for more details.
 */

/* Version
 *
 * These macros specify the major and minor version of the libpkix API defined
 * by this header file.
 */

#define PKIX_MAJOR_VERSION              ((PKIX_UInt32) 0)
#define PKIX_MINOR_VERSION              ((PKIX_UInt32) 3)

/* Maximum minor version
 *
 * This macro is used to specify that the caller wants the largest minor
 * version available.
 */

#define PKIX_MAX_MINOR_VERSION          ((PKIX_UInt32) 4000000000)

/* Define Cert Store type for database access */
#define PKIX_STORE_TYPE_NONE            0
#define PKIX_STORE_TYPE_PK11            1

/* Portable Code (PC) data types
 *
 * These types are used to perform the primary operations of this library:
 * building and validating chains of X.509 certificates.
 */

typedef struct PKIX_ErrorStruct PKIX_Error;
typedef struct PKIX_ProcessingParamsStruct PKIX_ProcessingParams;
typedef struct PKIX_ValidateParamsStruct PKIX_ValidateParams;
typedef struct PKIX_ValidateResultStruct PKIX_ValidateResult;
typedef struct PKIX_ResourceLimitsStruct PKIX_ResourceLimits;
typedef struct PKIX_BuildResultStruct PKIX_BuildResult;
typedef struct PKIX_CertStoreStruct PKIX_CertStore;
typedef struct PKIX_CertChainCheckerStruct PKIX_CertChainChecker;
typedef struct PKIX_RevocationCheckerStruct PKIX_RevocationChecker;
typedef struct PKIX_CertSelectorStruct PKIX_CertSelector;
typedef struct PKIX_CRLSelectorStruct PKIX_CRLSelector;
typedef struct PKIX_ComCertSelParamsStruct PKIX_ComCertSelParams;
typedef struct PKIX_ComCRLSelParamsStruct PKIX_ComCRLSelParams;
typedef struct PKIX_TrustAnchorStruct PKIX_TrustAnchor;
typedef struct PKIX_PolicyNodeStruct PKIX_PolicyNode;
typedef struct PKIX_LoggerStruct PKIX_Logger;
typedef struct PKIX_ListStruct PKIX_List;
typedef struct PKIX_ForwardBuilderStateStruct PKIX_ForwardBuilderState;
typedef struct PKIX_DefaultRevocationCheckerStruct
                        PKIX_DefaultRevocationChecker;
typedef struct PKIX_OcspCheckerStruct PKIX_OcspChecker;
typedef struct PKIX_VerifyNodeStruct PKIX_VerifyNode;

/* Portability Layer (PL) data types
 *
 * These types are used are used as portable data types that are defined
 * consistently across platforms
 */

typedef struct PKIX_PL_ObjectStruct PKIX_PL_Object;
typedef struct PKIX_PL_ByteArrayStruct PKIX_PL_ByteArray;
typedef struct PKIX_PL_HashTableStruct PKIX_PL_HashTable;
typedef struct PKIX_PL_MutexStruct PKIX_PL_Mutex;
typedef struct PKIX_PL_RWLockStruct PKIX_PL_RWLock;
typedef struct PKIX_PL_MonitorLockStruct PKIX_PL_MonitorLock;
typedef struct PKIX_PL_BigIntStruct PKIX_PL_BigInt;
typedef struct PKIX_PL_StringStruct PKIX_PL_String;
typedef struct PKIX_PL_OIDStruct PKIX_PL_OID;
typedef struct PKIX_PL_CertStruct PKIX_PL_Cert;
typedef struct PKIX_PL_GeneralNameStruct PKIX_PL_GeneralName;
typedef struct PKIX_PL_X500NameStruct PKIX_PL_X500Name;
typedef struct PKIX_PL_PublicKeyStruct PKIX_PL_PublicKey;
typedef struct PKIX_PL_DateStruct PKIX_PL_Date;
typedef struct PKIX_PL_CertNameConstraintsStruct PKIX_PL_CertNameConstraints;
typedef struct PKIX_PL_CertBasicConstraintsStruct PKIX_PL_CertBasicConstraints;
typedef struct PKIX_PL_CertPoliciesStruct PKIX_PL_CertPolicies;
typedef struct PKIX_PL_CertPolicyInfoStruct PKIX_PL_CertPolicyInfo;
typedef struct PKIX_PL_CertPolicyQualifierStruct PKIX_PL_CertPolicyQualifier;
typedef struct PKIX_PL_CertPolicyMapStruct PKIX_PL_CertPolicyMap;
typedef struct PKIX_PL_CRLStruct PKIX_PL_CRL;
typedef struct PKIX_PL_CRLEntryStruct PKIX_PL_CRLEntry;
typedef struct PKIX_PL_CollectionCertStoreStruct PKIX_PL_CollectionCertStore;
typedef struct PKIX_PL_CollectionCertStoreContext
                        PKIX_PL_CollectionCertStoreContext;
typedef struct PKIX_PL_LdapCertStoreContext PKIX_PL_LdapCertStoreContext;
typedef struct PKIX_PL_LdapRequestStruct PKIX_PL_LdapRequest;
typedef struct PKIX_PL_LdapResponseStruct PKIX_PL_LdapResponse;
typedef struct PKIX_PL_LdapDefaultClientStruct PKIX_PL_LdapDefaultClient;
typedef struct PKIX_PL_SocketStruct PKIX_PL_Socket;
typedef struct PKIX_PL_InfoAccessStruct PKIX_PL_InfoAccess;
typedef struct PKIX_PL_AIAMgrStruct PKIX_PL_AIAMgr;
typedef struct PKIX_PL_OcspCertIDStruct PKIX_PL_OcspCertID;
typedef struct PKIX_PL_OcspRequestStruct PKIX_PL_OcspRequest;
typedef struct PKIX_PL_OcspResponseStruct PKIX_PL_OcspResponse;
typedef struct PKIX_PL_HttpClientStruct PKIX_PL_HttpClient;
typedef struct PKIX_PL_HttpDefaultClientStruct PKIX_PL_HttpDefaultClient;
typedef struct PKIX_PL_HttpCertStoreContextStruct PKIX_PL_HttpCertStoreContext;

/* Primitive types
 *
 * In order to guarantee desired behavior as well as platform-independence, we
 * typedef these types depending on the platform. XXX This needs more work!
 */

/* XXX Try compiling these files (and maybe the whole libpkix-nss) on Win32.
 * We don't know what type is at least 32 bits long. ISO C probably requires
 * at least 32 bits for long. we could default to that and only list platforms
 * where that's not true.
 *
 * #elif
 * #error
 * #endif
 */

/* currently, int is 32 bits on all our supported platforms */

typedef unsigned int PKIX_UInt32;
typedef int PKIX_Int32;

typedef int PKIX_Boolean;

/* Object Types
 *
 * Every reference-counted PKIX_PL_Object is associated with an integer type.
 */
#define PKIX_TYPES \
    TYPEMACRO(AIAMGR), \
    TYPEMACRO(BASICCONSTRAINTSCHECKERSTATE), \
    TYPEMACRO(BIGINT), \
    TYPEMACRO(BUILDRESULT), \
    TYPEMACRO(BYTEARRAY), \
    TYPEMACRO(CERT), \
    TYPEMACRO(CERTBASICCONSTRAINTS), \
    TYPEMACRO(CERTCHAINCHECKER), \
    TYPEMACRO(CERTNAMECONSTRAINTS), \
    TYPEMACRO(CERTNAMECONSTRAINTSCHECKERSTATE), \
    TYPEMACRO(CERTPOLICYCHECKERSTATE), \
    TYPEMACRO(CERTPOLICYINFO), \
    TYPEMACRO(CERTPOLICYMAP), \
    TYPEMACRO(CERTPOLICYNODE), \
    TYPEMACRO(CERTPOLICYQUALIFIER), \
    TYPEMACRO(CERTSELECTOR), \
    TYPEMACRO(CERTSTORE), \
    TYPEMACRO(COLLECTIONCERTSTORECONTEXT), \
    TYPEMACRO(COMCERTSELPARAMS), \
    TYPEMACRO(COMCRLSELPARAMS), \
    TYPEMACRO(CRL), \
    TYPEMACRO(CRLENTRY), \
    TYPEMACRO(CRLSELECTOR), \
    TYPEMACRO(DATE), \
    TYPEMACRO(DEFAULTCRLCHECKERSTATE), \
    TYPEMACRO(DEFAULTREVOCATIONCHECKER), \
    TYPEMACRO(EKUCHECKER), \
    TYPEMACRO(ERROR), \
    TYPEMACRO(FORWARDBUILDERSTATE), \
    TYPEMACRO(GENERALNAME), \
    TYPEMACRO(HASHTABLE), \
    TYPEMACRO(HTTPCERTSTORECONTEXT), \
    TYPEMACRO(HTTPDEFAULTCLIENT), \
    TYPEMACRO(INFOACCESS), \
    TYPEMACRO(LDAPDEFAULTCLIENT), \
    TYPEMACRO(LDAPREQUEST), \
    TYPEMACRO(LDAPRESPONSE), \
    TYPEMACRO(LIST), \
    TYPEMACRO(LOGGER), \
    TYPEMACRO(MONITORLOCK), \
    TYPEMACRO(MUTEX), \
    TYPEMACRO(OBJECT), \
    TYPEMACRO(OCSPCERTID), \
    TYPEMACRO(OCSPCHECKER), \
    TYPEMACRO(OCSPREQUEST), \
    TYPEMACRO(OCSPRESPONSE), \
    TYPEMACRO(OID), \
    TYPEMACRO(PROCESSINGPARAMS), \
    TYPEMACRO(PUBLICKEY), \
    TYPEMACRO(RESOURCELIMITS), \
    TYPEMACRO(REVOCATIONCHECKER), \
    TYPEMACRO(RWLOCK), \
    TYPEMACRO(SIGNATURECHECKERSTATE), \
    TYPEMACRO(SOCKET), \
    TYPEMACRO(STRING), \
    TYPEMACRO(TARGETCERTCHECKERSTATE), \
    TYPEMACRO(TRUSTANCHOR), \
    TYPEMACRO(VALIDATEPARAMS), \
    TYPEMACRO(VALIDATERESULT), \
    TYPEMACRO(VERIFYNODE), \
    TYPEMACRO(X500NAME)

#define TYPEMACRO(type) PKIX_ ## type ## _TYPE

typedef enum {     /* Now invoke all those TYPEMACROs to assign the numbers */
   PKIX_TYPES,
   PKIX_NUMTYPES   /* This gets PKIX_NUMTYPES defined as the total number */
} PKIX_TYPENUM;


#ifdef PKIX_USER_OBJECT_TYPE

/* User Define Object Types
 *
 * User may define their own object types offset from PKIX_USER_OBJECT_TYPE
 */
#define PKIX_USER_OBJECT_TYPEBASE 1000

#endif /* PKIX_USER_OBJECT_TYPE */

/* Error Codes
 *
 * This list is used to define a set of PKIX_Error exception class numbers.
 * ERRMACRO is redefined to produce a corresponding set of
 * strings in the table "const char *PKIX_ERRORCLASSNAMES[PKIX_NUMERRORCLASSES]" in
 * pkix_error.c. For example, since the fifth ERRMACRO entry is MUTEX, then
 * PKIX_MUTEX_ERROR is defined in pkixt.h as 4, and PKIX_ERRORCLASSNAMES[4] is
 * initialized in pkix_error.c with the value "MUTEX".
 */
#define PKIX_ERRORCLASSES \
   ERRMACRO(AIAMGR), \
   ERRMACRO(BASICCONSTRAINTSCHECKERSTATE), \
   ERRMACRO(BIGINT), \
   ERRMACRO(BUILD), \
   ERRMACRO(BUILDRESULT), \
   ERRMACRO(BYTEARRAY), \
   ERRMACRO(CERT), \
   ERRMACRO(CERTBASICCONSTRAINTS), \
   ERRMACRO(CERTCHAINCHECKER), \
   ERRMACRO(CERTNAMECONSTRAINTS), \
   ERRMACRO(CERTNAMECONSTRAINTSCHECKERSTATE), \
   ERRMACRO(CERTPOLICYCHECKERSTATE), \
   ERRMACRO(CERTPOLICYINFO), \
   ERRMACRO(CERTPOLICYMAP), \
   ERRMACRO(CERTPOLICYNODE), \
   ERRMACRO(CERTPOLICYQUALIFIER), \
   ERRMACRO(CERTSELECTOR), \
   ERRMACRO(CERTSTORE), \
   ERRMACRO(CERTVFYPKIX), \
   ERRMACRO(COLLECTIONCERTSTORECONTEXT), \
   ERRMACRO(COMCERTSELPARAMS), \
   ERRMACRO(COMCRLSELPARAMS), \
   ERRMACRO(CONTEXT), \
   ERRMACRO(CRL), \
   ERRMACRO(CRLENTRY), \
   ERRMACRO(CRLSELECTOR), \
   ERRMACRO(DATE), \
   ERRMACRO(DEFAULTCRLCHECKERSTATE), \
   ERRMACRO(DEFAULTREVOCATIONCHECKER), \
   ERRMACRO(EKUCHECKER), \
   ERRMACRO(ERROR), \
   ERRMACRO(FATAL), \
   ERRMACRO(FORWARDBUILDERSTATE), \
   ERRMACRO(GENERALNAME), \
   ERRMACRO(HASHTABLE), \
   ERRMACRO(HTTPCERTSTORECONTEXT), \
   ERRMACRO(HTTPDEFAULTCLIENT), \
   ERRMACRO(INFOACCESS), \
   ERRMACRO(LDAPCLIENT), \
   ERRMACRO(LDAPDEFAULTCLIENT), \
   ERRMACRO(LDAPREQUEST), \
   ERRMACRO(LDAPRESPONSE), \
   ERRMACRO(LIFECYCLE), \
   ERRMACRO(LIST), \
   ERRMACRO(LOGGER), \
   ERRMACRO(MEM), \
   ERRMACRO(MONITORLOCK), \
   ERRMACRO(MUTEX), \
   ERRMACRO(OBJECT), \
   ERRMACRO(OCSPCERTID), \
   ERRMACRO(OCSPCHECKER), \
   ERRMACRO(OCSPREQUEST), \
   ERRMACRO(OCSPRESPONSE), \
   ERRMACRO(OID), \
   ERRMACRO(PROCESSINGPARAMS), \
   ERRMACRO(PUBLICKEY), \
   ERRMACRO(RESOURCELIMITS), \
   ERRMACRO(REVOCATIONCHECKER), \
   ERRMACRO(RWLOCK), \
   ERRMACRO(SIGNATURECHECKERSTATE), \
   ERRMACRO(SOCKET), \
   ERRMACRO(STRING), \
   ERRMACRO(TARGETCERTCHECKERSTATE), \
   ERRMACRO(TRUSTANCHOR), \
   ERRMACRO(USERDEFINEDMODULES), \
   ERRMACRO(VALIDATE), \
   ERRMACRO(VALIDATEPARAMS), \
   ERRMACRO(VALIDATERESULT), \
   ERRMACRO(VERIFYNODE), \
   ERRMACRO(X500NAME)

#define ERRMACRO(type) PKIX_ ## type ## _ERROR

typedef enum {     /* Now invoke all those ERRMACROs to assign the numbers */
   PKIX_ERRORCLASSES,
   PKIX_NUMERRORCLASSES   /* This gets PKIX_NUMERRORCLASSES defined as the total number */
} PKIX_ERRORCLASS;

/* Now define error strings (for internationalization) */

#define PKIX_ERRORENTRY(name,desc) PKIX_ ## name

/* Define all the error numbers */
typedef enum    {
#include "pkix_errorstrings.h"
} PKIX_ERRORCODE;

extern const char * const PKIX_ErrorText[];


/* String Formats
 *
 * These formats specify supported encoding formats for Strings.
 */

#define PKIX_ESCASCII           0
#define PKIX_UTF8               1
#define PKIX_UTF16              2
#define PKIX_UTF8_NULL_TERM     3
#define PKIX_ESCASCII_DEBUG     4

/* Name Types
 *
 * These types specify supported formats for GeneralNames.
 */

#define PKIX_OTHER_NAME         1
#define PKIX_RFC822_NAME        2
#define PKIX_DNS_NAME           3
#define PKIX_X400_ADDRESS       4
#define PKIX_DIRECTORY_NAME     5
#define PKIX_EDIPARTY_NAME      6
#define PKIX_URI_NAME           7
#define PKIX_IP_NAME            8
#define PKIX_OID_NAME           9

/* Key Usages
 *
 * These types specify supported Key Usages
 */

#define PKIX_DIGITAL_SIGNATURE  0x001
#define PKIX_NON_REPUDIATION    0x002
#define PKIX_KEY_ENCIPHERMENT   0x004
#define PKIX_DATA_ENCIPHERMENT  0x008
#define PKIX_KEY_AGREEMENT      0x010
#define PKIX_KEY_CERT_SIGN      0x020
#define PKIX_CRL_SIGN           0x040
#define PKIX_ENCIPHER_ONLY      0x080
#define PKIX_DECIPHER_ONLY      0x100

/* Reason Flags
 *
 * These macros specify supported Reason Flags
 */

#define PKIX_UNUSED                     0x001
#define PKIX_KEY_COMPROMISE             0x002
#define PKIX_CA_COMPROMISE              0x004
#define PKIX_AFFILIATION_CHANGED        0x008
#define PKIX_SUPERSEDED                 0x010
#define PKIX_CESSATION_OF_OPERATION     0x020
#define PKIX_CERTIFICATE_HOLD           0x040
#define PKIX_PRIVILEGE_WITHDRAWN        0x080
#define PKIX_AA_COMPROMISE              0x100

/* Boolean values
 *
 * These macros specify the Boolean values of TRUE and FALSE
 * XXX Is it the case that any non-zero value is actually considered TRUE
 * and this is just a convenient mnemonic macro?
 */

#define PKIX_TRUE                       ((PKIX_Boolean) 1)
#define PKIX_FALSE                      ((PKIX_Boolean) 0)

/*
 * Define constants for basic constraints selector
 *      (see comments in pkix_certsel.h)
 */

#define PKIX_CERTSEL_ENDENTITY_MIN_PATHLENGTH (-2)
#define PKIX_CERTSEL_ALL_MATCH_MIN_PATHLENGTH (-1)

/*
 * PKIX_ALLOC_ERROR is a special error object hard-coded into the pkix_error.o
 * object file. It is thrown if system memory cannot be allocated or may be
 * thrown for other unrecoverable errors. PKIX_ALLOC_ERROR is immutable.
 * IncRef, DecRef and all Settor functions cannot be called.
 * XXX Does anyone actually need to know about this?
 * XXX Why no DecRef? Would be good to handle it the same.
 */

PKIX_Error* PKIX_ALLOC_ERROR(void);

/*
 * In a CertBasicConstraints extension, if the CA flag is set,
 * indicating the certificate refers to a Certification
 * Authority, then the pathLen field indicates how many intermediate
 * certificates (not counting self-signed ones) can exist in a valid
 * chain following this certificate. If the pathLen has the value
 * of this constant, then the length of the chain is unlimited
 */
#define PKIX_UNLIMITED_PATH_CONSTRAINT ((PKIX_Int32) -1)

/*
 * Define Certificate Extension hard-coded OID's
 */
#define PKIX_CERTKEYUSAGE_OID                  "2.5.29.15"
#define PKIX_CERTSUBJALTNAME_OID               "2.5.29.17"
#define PKIX_BASICCONSTRAINTS_OID              "2.5.29.19"
#define PKIX_CRLREASONCODE_OID                 "2.5.29.21"
#define PKIX_NAMECONSTRAINTS_OID               "2.5.29.30"
#define PKIX_CERTIFICATEPOLICIES_OID           "2.5.29.32"
#define PKIX_CERTIFICATEPOLICIES_ANYPOLICY_OID "2.5.29.32.0"
#define PKIX_POLICYMAPPINGS_OID                "2.5.29.33"
#define PKIX_POLICYCONSTRAINTS_OID             "2.5.29.36"
#define PKIX_EXTENDEDKEYUSAGE_OID              "2.5.29.37"
#define PKIX_INHIBITANYPOLICY_OID              "2.5.29.54"
#define PKIX_NSCERTTYPE_OID "2.16.840.1.113730.1.1"

#ifdef __cplusplus
}
#endif

#endif /* _PKIXT_H */
