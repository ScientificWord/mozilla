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
 * Red Hat, Inc.
 * Portions created by the Initial Developer are
 * Copyright 2008 Red Hat, Inc.  All Rights Reserved.
 *
 * Contributor(s):
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
 * pkix_pl_ocspcertid.h
 *
 * Public Key Object Definitions
 *
 */

#ifndef _PKIX_PL_OCSPCERTID_H
#define _PKIX_PL_OCSPCERTID_H

#include "pkix_pl_common.h"

#ifdef __cplusplus
extern "C" {
#endif

struct PKIX_PL_OcspCertIDStruct {
        CERTOCSPCertID *certID;
        PRBool certIDWasConsumed;
};

/* see source file for function documentation */

PKIX_Error *pkix_pl_OcspCertID_RegisterSelf(void *plContext);

PKIX_Error *
PKIX_PL_OcspCertID_Create(
        PKIX_PL_Cert *cert,
        PKIX_PL_Date *validity,
        PKIX_PL_OcspCertID **object,
        void *plContext);

PKIX_Error *
PKIX_PL_OcspCertID_GetFreshCacheStatus(
        PKIX_PL_OcspCertID *cid, 
        PKIX_PL_Date *validity,
        PKIX_Boolean *hasFreshStatus,
        PKIX_Boolean *statusIsGood,
        SECErrorCodes *missingResponseError,
        void *plContext);

PKIX_Error *
PKIX_PL_OcspCertID_RememberOCSPProcessingFailure(
        PKIX_PL_OcspCertID *cid, 
        void *plContext);

#ifdef __cplusplus
}
#endif

#endif /* _PKIX_PL_OCSPCERTID_H */
