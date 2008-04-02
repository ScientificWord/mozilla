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

/*
 * mechanism.c
 *
 * This file implements the NSSCKFWMechanism type and methods.
 */

#ifndef CK_T
#include "ck.h"
#endif /* CK_T */

/*
 * NSSCKFWMechanism
 *
 *  -- create/destroy --
 *  nssCKFWMechanism_Create
 *  nssCKFWMechanism_Destroy
 *
 *  -- implement public accessors --
 *  nssCKFWMechanism_GetMDMechanism
 *  nssCKFWMechanism_GetParameter
 *
 *  -- private accessors --
 *
 *  -- module fronts --
 *  nssCKFWMechanism_GetMinKeySize
 *  nssCKFWMechanism_GetMaxKeySize
 *  nssCKFWMechanism_GetInHardware
 *  nssCKFWMechanism_GetCanEncrypt
 *  nssCKFWMechanism_GetCanDecrypt
 *  nssCKFWMechanism_GetCanDigest
 *  nssCKFWMechanism_GetCanSign
 *  nssCKFWMechanism_GetCanSignRecover
 *  nssCKFWMechanism_GetCanVerify
 *  nssCKFWMechanism_GetCanGenerate
 *  nssCKFWMechanism_GetCanGenerateKeyPair
 *  nssCKFWMechanism_GetCanUnwrap
 *  nssCKFWMechanism_GetCanWrap
 *  nssCKFWMechanism_GetCanDerive
 *  nssCKFWMechanism_EncryptInit
 *  nssCKFWMechanism_DecryptInit
 *  nssCKFWMechanism_DigestInit
 *  nssCKFWMechanism_SignInit
 *  nssCKFWMechanism_VerifyInit
 *  nssCKFWMechanism_SignRecoverInit
 *  nssCKFWMechanism_VerifyRecoverInit
 *  nssCKFWMechanism_GenerateKey
 *  nssCKFWMechanism_GenerateKeyPair
 *  nssCKFWMechanism_GetWrapKeyLength
 *  nssCKFWMechanism_WrapKey
 *  nssCKFWMechanism_UnwrapKey
 *  nssCKFWMechanism_DeriveKey
 */


struct NSSCKFWMechanismStr {
   NSSCKMDMechanism *mdMechanism;
   NSSCKMDToken *mdToken;
   NSSCKFWToken *fwToken;
   NSSCKMDInstance *mdInstance;
   NSSCKFWInstance *fwInstance;
};

/*
 * nssCKFWMechanism_Create
 *
 */
NSS_IMPLEMENT NSSCKFWMechanism *
nssCKFWMechanism_Create
(
  NSSCKMDMechanism *mdMechanism,
  NSSCKMDToken *mdToken,
  NSSCKFWToken *fwToken,
  NSSCKMDInstance *mdInstance,
  NSSCKFWInstance *fwInstance
)
{
  NSSCKFWMechanism *fwMechanism;


  fwMechanism = nss_ZNEW(NULL, NSSCKFWMechanism);
  if ((NSSCKFWMechanism *)NULL == fwMechanism) {
    return (NSSCKFWMechanism *)NULL;
  }
  fwMechanism->mdMechanism = mdMechanism;
  fwMechanism->mdToken = mdToken;
  fwMechanism->fwToken = fwToken;
  fwMechanism->mdInstance = mdInstance;
  fwMechanism->fwInstance = fwInstance;
  return fwMechanism;
}

/*
 * nssCKFWMechanism_Destroy
 *
 */
NSS_IMPLEMENT void
nssCKFWMechanism_Destroy
(
  NSSCKFWMechanism *fwMechanism
)
{
  /* destroy any fw resources held by nssCKFWMechanism (currently none) */

  if ((void *)NULL == (void *)fwMechanism->mdMechanism->Destroy) {
    /* destroys it's parent as well */
    fwMechanism->mdMechanism->Destroy(
        fwMechanism->mdMechanism, 
        fwMechanism,
        fwMechanism->mdInstance,
        fwMechanism->fwInstance);
  }
  /* if the Destroy function wasn't supplied, then the mechanism is 'static',
   * and there is nothing to destroy */
  return;
}

/*
 * nssCKFWMechanism_GetMDMechanism
 *
 */
NSS_IMPLEMENT NSSCKMDMechanism *
nssCKFWMechanism_GetMDMechanism
(
  NSSCKFWMechanism *fwMechanism
)
{
  return fwMechanism->mdMechanism;
}

/*
 * nssCKFWMechanism_GetMinKeySize
 *
 */
NSS_IMPLEMENT CK_ULONG
nssCKFWMechanism_GetMinKeySize
(
  NSSCKFWMechanism *fwMechanism,
  CK_RV *pError
)
{
  if ( (void *)NULL == (void *)fwMechanism->mdMechanism->GetMinKeySize) {
    return 0;
  }

  return fwMechanism->mdMechanism->GetMinKeySize(fwMechanism->mdMechanism,
    fwMechanism, fwMechanism->mdToken, fwMechanism->fwToken, 
    fwMechanism->mdInstance, fwMechanism->fwInstance, pError);
}

/*
 * nssCKFWMechanism_GetMaxKeySize
 *
 */
NSS_IMPLEMENT CK_ULONG
nssCKFWMechanism_GetMaxKeySize
(
  NSSCKFWMechanism *fwMechanism,
  CK_RV *pError
)
{
  if ( (void *)NULL == (void *)fwMechanism->mdMechanism->GetMaxKeySize) {
    return 0;
  }

  return fwMechanism->mdMechanism->GetMaxKeySize(fwMechanism->mdMechanism,
    fwMechanism, fwMechanism->mdToken, fwMechanism->fwToken, 
    fwMechanism->mdInstance, fwMechanism->fwInstance, pError);
}

/*
 * nssCKFWMechanism_GetInHardware
 *
 */
NSS_IMPLEMENT CK_BBOOL
nssCKFWMechanism_GetInHardware
(
  NSSCKFWMechanism *fwMechanism,
  CK_RV *pError
)
{
  if ( (void *)NULL == (void *)fwMechanism->mdMechanism->GetInHardware) {
    return CK_FALSE;
  }

  return fwMechanism->mdMechanism->GetInHardware(fwMechanism->mdMechanism,
    fwMechanism, fwMechanism->mdToken, fwMechanism->fwToken, 
    fwMechanism->mdInstance, fwMechanism->fwInstance, pError);
}


/*
 * the following are determined automatically by which of the cryptographic
 * functions are defined for this mechanism.
 */
/*
 * nssCKFWMechanism_GetCanEncrypt
 *
 */
NSS_EXTERN CK_BBOOL
nssCKFWMechanism_GetCanEncrypt
(
  NSSCKFWMechanism *fwMechanism,
  CK_RV *pError
)
{
  if ( (void *)NULL == (void *)fwMechanism->mdMechanism->EncryptInit) {
    return CK_FALSE;
  }
  return CK_TRUE;
}

/*
 * nssCKFWMechanism_GetCanDecrypt
 *
 */
NSS_EXTERN CK_BBOOL
nssCKFWMechanism_GetCanDecrypt
(
  NSSCKFWMechanism *fwMechanism,
  CK_RV *pError
)
{
  if ( (void *)NULL == (void *)fwMechanism->mdMechanism->DecryptInit) {
    return CK_FALSE;
  }
  return CK_TRUE;
}

/*
 * nssCKFWMechanism_GetCanDigest
 *
 */
NSS_EXTERN CK_BBOOL
nssCKFWMechanism_GetCanDigest
(
  NSSCKFWMechanism *fwMechanism,
  CK_RV *pError
)
{
  if ( (void *)NULL == (void *)fwMechanism->mdMechanism->DigestInit) {
    return CK_FALSE;
  }
  return CK_TRUE;
}

/*
 * nssCKFWMechanism_GetCanSign
 *
 */
NSS_EXTERN CK_BBOOL
nssCKFWMechanism_GetCanSign
(
  NSSCKFWMechanism *fwMechanism,
  CK_RV *pError
)
{
  if ( (void *)NULL == (void *)fwMechanism->mdMechanism->SignInit) {
    return CK_FALSE;
  }
  return CK_TRUE;
}

/*
 * nssCKFWMechanism_GetCanSignRecover
 *
 */
NSS_EXTERN CK_BBOOL
nssCKFWMechanism_GetCanSignRecover
(
  NSSCKFWMechanism *fwMechanism,
  CK_RV *pError
)
{
  if ( (void *)NULL == (void *)fwMechanism->mdMechanism->SignRecoverInit) {
    return CK_FALSE;
  }
  return CK_TRUE;
}

/*
 * nssCKFWMechanism_GetCanVerify
 *
 */
NSS_EXTERN CK_BBOOL
nssCKFWMechanism_GetCanVerify
(
  NSSCKFWMechanism *fwMechanism,
  CK_RV *pError
)
{
  if ( (void *)NULL == (void *)fwMechanism->mdMechanism->VerifyInit) {
    return CK_FALSE;
  }
  return CK_TRUE;
}

/*
 * nssCKFWMechanism_GetCanVerifyRecover
 *
 */
NSS_EXTERN CK_BBOOL
nssCKFWMechanism_GetCanVerifyRecover
(
  NSSCKFWMechanism *fwMechanism,
  CK_RV *pError
)
{
  if ( (void *)NULL == (void *)fwMechanism->mdMechanism->VerifyRecoverInit) {
    return CK_FALSE;
  }
  return CK_TRUE;
}

/*
 * nssCKFWMechanism_GetCanGenerate
 *
 */
NSS_EXTERN CK_BBOOL
nssCKFWMechanism_GetCanGenerate
(
  NSSCKFWMechanism *fwMechanism,
  CK_RV *pError
)
{
  if ( (void *)NULL == (void *)fwMechanism->mdMechanism->GenerateKey) {
    return CK_FALSE;
  }
  return CK_TRUE;
}

/*
 * nssCKFWMechanism_GetCanGenerateKeyPair
 *
 */
NSS_EXTERN CK_BBOOL
nssCKFWMechanism_GetCanGenerateKeyPair
(
  NSSCKFWMechanism *fwMechanism,
  CK_RV *pError
)
{
  if ( (void *)NULL == (void *)fwMechanism->mdMechanism->GenerateKeyPair) {
    return CK_FALSE;
  }
  return CK_TRUE;
}

/*
 * nssCKFWMechanism_GetCanUnwrap
 *
 */
NSS_EXTERN CK_BBOOL
nssCKFWMechanism_GetCanUnwrap
(
  NSSCKFWMechanism *fwMechanism,
  CK_RV *pError
)
{
  if ( (void *)NULL == (void *)fwMechanism->mdMechanism->UnwrapKey) {
    return CK_FALSE;
  }
  return CK_TRUE;
}

/*
 * nssCKFWMechanism_GetCanWrap
 *
 */
NSS_EXTERN CK_BBOOL
nssCKFWMechanism_GetCanWrap
(
  NSSCKFWMechanism *fwMechanism,
  CK_RV *pError
)
{
  if ( (void *)NULL == (void *)fwMechanism->mdMechanism->WrapKey) {
    return CK_FALSE;
  }
  return CK_TRUE;
}

/*
 * nssCKFWMechanism_GetCanDerive
 *
 */
NSS_EXTERN CK_BBOOL
nssCKFWMechanism_GetCanDerive
(
  NSSCKFWMechanism *fwMechanism,
  CK_RV *pError
)
{
  if ( (void *)NULL == (void *)fwMechanism->mdMechanism->DeriveKey) {
    return CK_FALSE;
  }
  return CK_TRUE;
}

/*
 * These are the actual crypto operations
 */

/* 
 * nssCKFWMechanism_EncryptInit
 *  Start an encryption session.
 */
NSS_EXTERN CK_RV
nssCKFWMechanism_EncryptInit
(
  NSSCKFWMechanism *fwMechanism,
  CK_MECHANISM     *pMechanism,
  NSSCKFWSession   *fwSession,
  NSSCKFWObject    *fwObject
)
{
  NSSCKFWCryptoOperation *fwOperation;
  NSSCKMDCryptoOperation *mdOperation;
  NSSCKMDSession *mdSession;
  NSSCKMDObject  *mdObject;
  CK_RV  error = CKR_OK;


  fwOperation = nssCKFWSession_GetCurrentCryptoOperation(fwSession, 
                        NSSCKFWCryptoOperationState_EncryptDecrypt);
  if ((NSSCKFWCryptoOperation *)NULL != fwOperation) {
    return CKR_OPERATION_ACTIVE;
  }

  if ( (void *)NULL == (void *)fwMechanism->mdMechanism->EncryptInit) {
    return CKR_FUNCTION_FAILED;
  }

  mdSession = nssCKFWSession_GetMDSession(fwSession);
  mdObject = nssCKFWObject_GetMDObject(fwObject);
  mdOperation = fwMechanism->mdMechanism->EncryptInit(
        fwMechanism->mdMechanism,
        fwMechanism,
        pMechanism,
        mdSession,
        fwSession,
        fwMechanism->mdToken,
        fwMechanism->fwToken,
        fwMechanism->mdInstance,
        fwMechanism->fwInstance,
        mdObject,
        fwObject,
        &error
  );
  if ((NSSCKMDCryptoOperation *)NULL == mdOperation) {
    goto loser;
  }

  fwOperation = nssCKFWCryptoOperation_Create(mdOperation, 
        mdSession, fwSession, fwMechanism->mdToken, fwMechanism->fwToken,
        fwMechanism->mdInstance, fwMechanism->fwInstance,
        NSSCKFWCryptoOperationType_Encrypt, &error);
  if ((NSSCKFWCryptoOperation *)NULL != fwOperation) {
    nssCKFWSession_SetCurrentCryptoOperation(fwSession, fwOperation,
                NSSCKFWCryptoOperationState_EncryptDecrypt);
  }

loser:
  return error;
}

/* 
 * nssCKFWMechanism_DecryptInit
 *  Start an encryption session.
 */
NSS_EXTERN CK_RV
nssCKFWMechanism_DecryptInit
(
  NSSCKFWMechanism *fwMechanism,
  CK_MECHANISM     *pMechanism,
  NSSCKFWSession   *fwSession,
  NSSCKFWObject    *fwObject
)
{
  NSSCKFWCryptoOperation *fwOperation;
  NSSCKMDCryptoOperation *mdOperation;
  NSSCKMDSession *mdSession;
  NSSCKMDObject  *mdObject;
  CK_RV  error = CKR_OK;


  fwOperation = nssCKFWSession_GetCurrentCryptoOperation(fwSession, 
                        NSSCKFWCryptoOperationState_EncryptDecrypt);
  if ((NSSCKFWCryptoOperation *)NULL != fwOperation) {
    return CKR_OPERATION_ACTIVE;
  }

  if ( (void *)NULL == (void *)fwMechanism->mdMechanism->DecryptInit) {
    return CKR_FUNCTION_FAILED;
  }

  mdSession = nssCKFWSession_GetMDSession(fwSession);
  mdObject = nssCKFWObject_GetMDObject(fwObject);
  mdOperation = fwMechanism->mdMechanism->DecryptInit(
        fwMechanism->mdMechanism,
        fwMechanism,
        pMechanism,
        mdSession,
        fwSession,
        fwMechanism->mdToken,
        fwMechanism->fwToken,
        fwMechanism->mdInstance,
        fwMechanism->fwInstance,
        mdObject,
        fwObject,
        &error
  );
  if ((NSSCKMDCryptoOperation *)NULL == mdOperation) {
    goto loser;
  }

  fwOperation = nssCKFWCryptoOperation_Create(mdOperation, 
        mdSession, fwSession, fwMechanism->mdToken, fwMechanism->fwToken,
        fwMechanism->mdInstance, fwMechanism->fwInstance,
        NSSCKFWCryptoOperationType_Decrypt, &error);
  if ((NSSCKFWCryptoOperation *)NULL != fwOperation) {
    nssCKFWSession_SetCurrentCryptoOperation(fwSession, fwOperation,
                NSSCKFWCryptoOperationState_EncryptDecrypt);
  }

loser:
  return error;
}

/* 
 * nssCKFWMechanism_DigestInit
 *  Start an encryption session.
 */
NSS_EXTERN CK_RV
nssCKFWMechanism_DigestInit
(
  NSSCKFWMechanism *fwMechanism,
  CK_MECHANISM     *pMechanism,
  NSSCKFWSession   *fwSession
)
{
  NSSCKFWCryptoOperation *fwOperation;
  NSSCKMDCryptoOperation *mdOperation;
  NSSCKMDSession *mdSession;
  CK_RV  error = CKR_OK;


  fwOperation = nssCKFWSession_GetCurrentCryptoOperation(fwSession, 
                        NSSCKFWCryptoOperationState_Digest);
  if ((NSSCKFWCryptoOperation *)NULL != fwOperation) {
    return CKR_OPERATION_ACTIVE;
  }

  if ( (void *)NULL == (void *)fwMechanism->mdMechanism->DigestInit) {
    return CKR_FUNCTION_FAILED;
  }

  mdSession = nssCKFWSession_GetMDSession(fwSession);
  mdOperation = fwMechanism->mdMechanism->DigestInit(
        fwMechanism->mdMechanism,
        fwMechanism,
        pMechanism,
        mdSession,
        fwSession,
        fwMechanism->mdToken,
        fwMechanism->fwToken,
        fwMechanism->mdInstance,
        fwMechanism->fwInstance,
        &error
  );
  if ((NSSCKMDCryptoOperation *)NULL == mdOperation) {
    goto loser;
  }

  fwOperation = nssCKFWCryptoOperation_Create(mdOperation, 
        mdSession, fwSession, fwMechanism->mdToken, fwMechanism->fwToken,
        fwMechanism->mdInstance, fwMechanism->fwInstance,
        NSSCKFWCryptoOperationType_Digest, &error);
  if ((NSSCKFWCryptoOperation *)NULL != fwOperation) {
    nssCKFWSession_SetCurrentCryptoOperation(fwSession, fwOperation,
                NSSCKFWCryptoOperationState_Digest);
  }

loser:
  return error;
}

/* 
 * nssCKFWMechanism_SignInit
 *  Start an encryption session.
 */
NSS_EXTERN CK_RV
nssCKFWMechanism_SignInit
(
  NSSCKFWMechanism *fwMechanism,
  CK_MECHANISM     *pMechanism,
  NSSCKFWSession   *fwSession,
  NSSCKFWObject    *fwObject
)
{
  NSSCKFWCryptoOperation *fwOperation;
  NSSCKMDCryptoOperation *mdOperation;
  NSSCKMDSession *mdSession;
  NSSCKMDObject  *mdObject;
  CK_RV  error = CKR_OK;


  fwOperation = nssCKFWSession_GetCurrentCryptoOperation(fwSession, 
                        NSSCKFWCryptoOperationState_SignVerify);
  if ((NSSCKFWCryptoOperation *)NULL != fwOperation) {
    return CKR_OPERATION_ACTIVE;
  }

  if ( (void *)NULL == (void *)fwMechanism->mdMechanism->SignInit) {
    return CKR_FUNCTION_FAILED;
  }

  mdSession = nssCKFWSession_GetMDSession(fwSession);
  mdObject = nssCKFWObject_GetMDObject(fwObject);
  mdOperation = fwMechanism->mdMechanism->SignInit(
        fwMechanism->mdMechanism,
        fwMechanism,
        pMechanism,
        mdSession,
        fwSession,
        fwMechanism->mdToken,
        fwMechanism->fwToken,
        fwMechanism->mdInstance,
        fwMechanism->fwInstance,
        mdObject,
        fwObject,
        &error
  );
  if ((NSSCKMDCryptoOperation *)NULL == mdOperation) {
    goto loser;
  }

  fwOperation = nssCKFWCryptoOperation_Create(mdOperation, 
        mdSession, fwSession, fwMechanism->mdToken, fwMechanism->fwToken,
        fwMechanism->mdInstance, fwMechanism->fwInstance,
        NSSCKFWCryptoOperationType_Sign, &error);
  if ((NSSCKFWCryptoOperation *)NULL != fwOperation) {
    nssCKFWSession_SetCurrentCryptoOperation(fwSession, fwOperation,
                NSSCKFWCryptoOperationState_SignVerify);
  }

loser:
  return error;
}

/* 
 * nssCKFWMechanism_VerifyInit
 *  Start an encryption session.
 */
NSS_EXTERN CK_RV
nssCKFWMechanism_VerifyInit
(
  NSSCKFWMechanism *fwMechanism,
  CK_MECHANISM     *pMechanism,
  NSSCKFWSession   *fwSession,
  NSSCKFWObject    *fwObject
)
{
  NSSCKFWCryptoOperation *fwOperation;
  NSSCKMDCryptoOperation *mdOperation;
  NSSCKMDSession *mdSession;
  NSSCKMDObject  *mdObject;
  CK_RV  error = CKR_OK;


  fwOperation = nssCKFWSession_GetCurrentCryptoOperation(fwSession, 
                        NSSCKFWCryptoOperationState_SignVerify);
  if ((NSSCKFWCryptoOperation *)NULL != fwOperation) {
    return CKR_OPERATION_ACTIVE;
  }

  if ( (void *)NULL == (void *)fwMechanism->mdMechanism->VerifyInit) {
    return CKR_FUNCTION_FAILED;
  }

  mdSession = nssCKFWSession_GetMDSession(fwSession);
  mdObject = nssCKFWObject_GetMDObject(fwObject);
  mdOperation = fwMechanism->mdMechanism->VerifyInit(
        fwMechanism->mdMechanism,
        fwMechanism,
        pMechanism,
        mdSession,
        fwSession,
        fwMechanism->mdToken,
        fwMechanism->fwToken,
        fwMechanism->mdInstance,
        fwMechanism->fwInstance,
        mdObject,
        fwObject,
        &error
  );
  if ((NSSCKMDCryptoOperation *)NULL == mdOperation) {
    goto loser;
  }

  fwOperation = nssCKFWCryptoOperation_Create(mdOperation, 
        mdSession, fwSession, fwMechanism->mdToken, fwMechanism->fwToken,
        fwMechanism->mdInstance, fwMechanism->fwInstance,
        NSSCKFWCryptoOperationType_Verify, &error);
  if ((NSSCKFWCryptoOperation *)NULL != fwOperation) {
    nssCKFWSession_SetCurrentCryptoOperation(fwSession, fwOperation,
                NSSCKFWCryptoOperationState_SignVerify);
  }

loser:
  return error;
}

/* 
 * nssCKFWMechanism_SignRecoverInit
 *  Start an encryption session.
 */
NSS_EXTERN CK_RV
nssCKFWMechanism_SignRecoverInit
(
  NSSCKFWMechanism *fwMechanism,
  CK_MECHANISM     *pMechanism,
  NSSCKFWSession   *fwSession,
  NSSCKFWObject    *fwObject
)
{
  NSSCKFWCryptoOperation *fwOperation;
  NSSCKMDCryptoOperation *mdOperation;
  NSSCKMDSession *mdSession;
  NSSCKMDObject  *mdObject;
  CK_RV  error = CKR_OK;


  fwOperation = nssCKFWSession_GetCurrentCryptoOperation(fwSession, 
                        NSSCKFWCryptoOperationState_SignVerify);
  if ((NSSCKFWCryptoOperation *)NULL != fwOperation) {
    return CKR_OPERATION_ACTIVE;
  }

  if ( (void *)NULL == (void *)fwMechanism->mdMechanism->SignRecoverInit) {
    return CKR_FUNCTION_FAILED;
  }

  mdSession = nssCKFWSession_GetMDSession(fwSession);
  mdObject = nssCKFWObject_GetMDObject(fwObject);
  mdOperation = fwMechanism->mdMechanism->SignRecoverInit(
        fwMechanism->mdMechanism,
        fwMechanism,
        pMechanism,
        mdSession,
        fwSession,
        fwMechanism->mdToken,
        fwMechanism->fwToken,
        fwMechanism->mdInstance,
        fwMechanism->fwInstance,
        mdObject,
        fwObject,
        &error
  );
  if ((NSSCKMDCryptoOperation *)NULL == mdOperation) {
    goto loser;
  }

  fwOperation = nssCKFWCryptoOperation_Create(mdOperation, 
        mdSession, fwSession, fwMechanism->mdToken, fwMechanism->fwToken,
        fwMechanism->mdInstance, fwMechanism->fwInstance,
        NSSCKFWCryptoOperationType_SignRecover, &error);
  if ((NSSCKFWCryptoOperation *)NULL != fwOperation) {
    nssCKFWSession_SetCurrentCryptoOperation(fwSession, fwOperation,
                NSSCKFWCryptoOperationState_SignVerify);
  }

loser:
  return error;
}

/* 
 * nssCKFWMechanism_VerifyRecoverInit
 *  Start an encryption session.
 */
NSS_EXTERN CK_RV
nssCKFWMechanism_VerifyRecoverInit
(
  NSSCKFWMechanism *fwMechanism,
  CK_MECHANISM     *pMechanism,
  NSSCKFWSession   *fwSession,
  NSSCKFWObject    *fwObject
)
{
  NSSCKFWCryptoOperation *fwOperation;
  NSSCKMDCryptoOperation *mdOperation;
  NSSCKMDSession *mdSession;
  NSSCKMDObject  *mdObject;
  CK_RV  error = CKR_OK;


  fwOperation = nssCKFWSession_GetCurrentCryptoOperation(fwSession, 
                        NSSCKFWCryptoOperationState_SignVerify);
  if ((NSSCKFWCryptoOperation *)NULL != fwOperation) {
    return CKR_OPERATION_ACTIVE;
  }

  if ( (void *)NULL == (void *)fwMechanism->mdMechanism->VerifyRecoverInit) {
    return CKR_FUNCTION_FAILED;
  }

  mdSession = nssCKFWSession_GetMDSession(fwSession);
  mdObject = nssCKFWObject_GetMDObject(fwObject);
  mdOperation = fwMechanism->mdMechanism->VerifyRecoverInit(
        fwMechanism->mdMechanism,
        fwMechanism,
        pMechanism,
        mdSession,
        fwSession,
        fwMechanism->mdToken,
        fwMechanism->fwToken,
        fwMechanism->mdInstance,
        fwMechanism->fwInstance,
        mdObject,
        fwObject,
        &error
  );
  if ((NSSCKMDCryptoOperation *)NULL == mdOperation) {
    goto loser;
  }

  fwOperation = nssCKFWCryptoOperation_Create(mdOperation, 
        mdSession, fwSession, fwMechanism->mdToken, fwMechanism->fwToken,
        fwMechanism->mdInstance, fwMechanism->fwInstance,
        NSSCKFWCryptoOperationType_VerifyRecover, &error);
  if ((NSSCKFWCryptoOperation *)NULL != fwOperation) {
    nssCKFWSession_SetCurrentCryptoOperation(fwSession, fwOperation,
                NSSCKFWCryptoOperationState_SignVerify);
  }

loser:
  return error;
}

/*
 * nssCKFWMechanism_GenerateKey
 */
NSS_EXTERN NSSCKFWObject *
nssCKFWMechanism_GenerateKey
(
  NSSCKFWMechanism *fwMechanism,
  CK_MECHANISM_PTR pMechanism,
  NSSCKFWSession   *fwSession,
  CK_ATTRIBUTE_PTR pTemplate,
  CK_ULONG         ulAttributeCount,
  CK_RV            *pError
)
{
  NSSCKMDSession *mdSession;
  NSSCKMDObject  *mdObject;
  NSSCKFWObject  *fwObject = NULL;
  NSSArena       *arena;

  if ( (void *)NULL == (void *)fwMechanism->mdMechanism->GenerateKey) {
    *pError = CKR_FUNCTION_FAILED;
    return (NSSCKFWObject *)NULL;
  }

  arena = nssCKFWToken_GetArena(fwMechanism->fwToken, pError);
  if ((NSSArena *)NULL == arena) {
    if (CKR_OK == *pError) {
      *pError = CKR_GENERAL_ERROR;
    }
    return (NSSCKFWObject *)NULL;
  }

  mdSession = nssCKFWSession_GetMDSession(fwSession);
  mdObject = fwMechanism->mdMechanism->GenerateKey(
        fwMechanism->mdMechanism,
        fwMechanism,
        pMechanism,
        mdSession,
        fwSession,
        fwMechanism->mdToken,
        fwMechanism->fwToken,
        fwMechanism->mdInstance,
        fwMechanism->fwInstance,
        pTemplate,
        ulAttributeCount,
        pError);

  if ((NSSCKMDObject *)NULL == mdObject) {
    return (NSSCKFWObject *)NULL;
  }

  fwObject = nssCKFWObject_Create(arena, mdObject, 
        fwSession, fwMechanism->fwToken, fwMechanism->fwInstance, pError);

  return fwObject;
}

/*
 * nssCKFWMechanism_GenerateKeyPair
 */
NSS_EXTERN CK_RV
nssCKFWMechanism_GenerateKeyPair
(
  NSSCKFWMechanism *fwMechanism,
  CK_MECHANISM_PTR pMechanism,
  NSSCKFWSession   *fwSession,
  CK_ATTRIBUTE_PTR pPublicKeyTemplate,
  CK_ULONG         ulPublicKeyAttributeCount,
  CK_ATTRIBUTE_PTR pPrivateKeyTemplate,
  CK_ULONG         ulPrivateKeyAttributeCount,
  NSSCKFWObject    **fwPublicKeyObject,
  NSSCKFWObject    **fwPrivateKeyObject
)
{
  NSSCKMDSession *mdSession;
  NSSCKMDObject  *mdPublicKeyObject;
  NSSCKMDObject  *mdPrivateKeyObject;
  NSSArena       *arena;
  CK_RV         error = CKR_OK;

  if ( (void *)NULL == (void *)fwMechanism->mdMechanism->GenerateKey) {
    return CKR_FUNCTION_FAILED;
  }

  arena = nssCKFWToken_GetArena(fwMechanism->fwToken, &error);
  if ((NSSArena *)NULL == arena) {
    if (CKR_OK == error) {
      error = CKR_GENERAL_ERROR;
    }
    return error;
  }

  mdSession = nssCKFWSession_GetMDSession(fwSession);
  error = fwMechanism->mdMechanism->GenerateKeyPair(
        fwMechanism->mdMechanism,
        fwMechanism,
        pMechanism,
        mdSession,
        fwSession,
        fwMechanism->mdToken,
        fwMechanism->fwToken,
        fwMechanism->mdInstance,
        fwMechanism->fwInstance,
        pPublicKeyTemplate,
        ulPublicKeyAttributeCount,
        pPrivateKeyTemplate,
        ulPrivateKeyAttributeCount,
        &mdPublicKeyObject,
        &mdPrivateKeyObject);

  if (CKR_OK != error) {
    return error;
  }

  *fwPublicKeyObject = nssCKFWObject_Create(arena, mdPublicKeyObject, 
        fwSession, fwMechanism->fwToken, fwMechanism->fwInstance, &error);
  if ((NSSCKFWObject *)NULL == *fwPublicKeyObject) {
    return error;
  }
  *fwPrivateKeyObject = nssCKFWObject_Create(arena, mdPrivateKeyObject, 
        fwSession, fwMechanism->fwToken, fwMechanism->fwInstance, &error);

  return error;
}

/*
 * nssCKFWMechanism_GetWrapKeyLength
 */
NSS_EXTERN CK_ULONG
nssCKFWMechanism_GetWrapKeyLength
(
  NSSCKFWMechanism *fwMechanism,
  CK_MECHANISM_PTR pMechanism,
  NSSCKFWSession   *fwSession,
  NSSCKFWObject    *fwWrappingKeyObject,
  NSSCKFWObject    *fwKeyObject,
  CK_RV                   *pError
)
{
  NSSCKMDSession *mdSession;
  NSSCKMDObject  *mdWrappingKeyObject;
  NSSCKMDObject  *mdKeyObject;

  if ( (void *)NULL == (void *)fwMechanism->mdMechanism->WrapKey) {
    *pError = CKR_FUNCTION_FAILED;
    return (CK_ULONG) 0;
  }

  mdSession = nssCKFWSession_GetMDSession(fwSession);
  mdWrappingKeyObject = nssCKFWObject_GetMDObject(fwWrappingKeyObject);
  mdKeyObject = nssCKFWObject_GetMDObject(fwKeyObject);
  return fwMechanism->mdMechanism->GetWrapKeyLength(
        fwMechanism->mdMechanism,
        fwMechanism,
        pMechanism,
        mdSession,
        fwSession,
        fwMechanism->mdToken,
        fwMechanism->fwToken,
        fwMechanism->mdInstance,
        fwMechanism->fwInstance,
        mdWrappingKeyObject,
        fwWrappingKeyObject,
        mdKeyObject,
        fwKeyObject,
        pError);
}

/*
 * nssCKFWMechanism_WrapKey
 */
NSS_EXTERN CK_RV
nssCKFWMechanism_WrapKey
(
  NSSCKFWMechanism *fwMechanism,
  CK_MECHANISM_PTR pMechanism,
  NSSCKFWSession   *fwSession,
  NSSCKFWObject    *fwWrappingKeyObject,
  NSSCKFWObject    *fwKeyObject,
  NSSItem          *wrappedKey
)
{
  NSSCKMDSession *mdSession;
  NSSCKMDObject  *mdWrappingKeyObject;
  NSSCKMDObject  *mdKeyObject;

  if ( (void *)NULL == (void *)fwMechanism->mdMechanism->WrapKey) {
    return CKR_FUNCTION_FAILED;
  }

  mdSession = nssCKFWSession_GetMDSession(fwSession);
  mdWrappingKeyObject = nssCKFWObject_GetMDObject(fwWrappingKeyObject);
  mdKeyObject = nssCKFWObject_GetMDObject(fwKeyObject);
  return fwMechanism->mdMechanism->WrapKey(
        fwMechanism->mdMechanism,
        fwMechanism,
        pMechanism,
        mdSession,
        fwSession,
        fwMechanism->mdToken,
        fwMechanism->fwToken,
        fwMechanism->mdInstance,
        fwMechanism->fwInstance,
        mdWrappingKeyObject,
        fwWrappingKeyObject,
        mdKeyObject,
        fwKeyObject,
        wrappedKey);
}

/*
 * nssCKFWMechanism_UnwrapKey
 */
NSS_EXTERN NSSCKFWObject *
nssCKFWMechanism_UnwrapKey
(
  NSSCKFWMechanism *fwMechanism,
  CK_MECHANISM_PTR pMechanism,
  NSSCKFWSession   *fwSession,
  NSSCKFWObject    *fwWrappingKeyObject,
  NSSItem          *wrappedKey,
  CK_ATTRIBUTE_PTR pTemplate,
  CK_ULONG         ulAttributeCount,
  CK_RV            *pError
)
{
  NSSCKMDSession *mdSession;
  NSSCKMDObject  *mdObject;
  NSSCKMDObject  *mdWrappingKeyObject;
  NSSCKFWObject  *fwObject = NULL;
  NSSArena       *arena;

  if ( (void *)NULL == (void *)fwMechanism->mdMechanism->UnwrapKey) {
    /* we could simulate UnwrapKey using Decrypt and Create object, but
     * 1) it's not clear that would work well, and 2) the low level token
     * may want to restrict unwrap key for a reason, so just fail it it
     * can't be done */
    *pError = CKR_FUNCTION_FAILED;
    return (NSSCKFWObject *)NULL;
  }

  arena = nssCKFWToken_GetArena(fwMechanism->fwToken, pError);
  if ((NSSArena *)NULL == arena) {
    if (CKR_OK == *pError) {
      *pError = CKR_GENERAL_ERROR;
    }
    return (NSSCKFWObject *)NULL;
  }

  mdSession = nssCKFWSession_GetMDSession(fwSession);
  mdWrappingKeyObject = nssCKFWObject_GetMDObject(fwWrappingKeyObject);
  mdObject = fwMechanism->mdMechanism->UnwrapKey(
        fwMechanism->mdMechanism,
        fwMechanism,
        pMechanism,
        mdSession,
        fwSession,
        fwMechanism->mdToken,
        fwMechanism->fwToken,
        fwMechanism->mdInstance,
        fwMechanism->fwInstance,
        mdWrappingKeyObject,
        fwWrappingKeyObject,
        wrappedKey,
        pTemplate,
        ulAttributeCount,
        pError);

  if ((NSSCKMDObject *)NULL == mdObject) {
    return (NSSCKFWObject *)NULL;
  }

  fwObject = nssCKFWObject_Create(arena, mdObject, 
        fwSession, fwMechanism->fwToken, fwMechanism->fwInstance, pError);

  return fwObject;
}

/* 
 * nssCKFWMechanism_DeriveKey
 */
NSS_EXTERN NSSCKFWObject *
nssCKFWMechanism_DeriveKey
(
  NSSCKFWMechanism *fwMechanism,
  CK_MECHANISM_PTR pMechanism,
  NSSCKFWSession   *fwSession,
  NSSCKFWObject    *fwBaseKeyObject,
  CK_ATTRIBUTE_PTR pTemplate,
  CK_ULONG         ulAttributeCount,
  CK_RV            *pError
)
{
  NSSCKMDSession *mdSession;
  NSSCKMDObject  *mdObject;
  NSSCKMDObject  *mdBaseKeyObject;
  NSSCKFWObject  *fwObject = NULL;
  NSSArena       *arena;

  if ( (void *)NULL == (void *)fwMechanism->mdMechanism->DeriveKey) {
    *pError = CKR_FUNCTION_FAILED;
    return (NSSCKFWObject *)NULL;
  }

  arena = nssCKFWToken_GetArena(fwMechanism->fwToken, pError);
  if ((NSSArena *)NULL == arena) {
    if (CKR_OK == *pError) {
      *pError = CKR_GENERAL_ERROR;
    }
    return (NSSCKFWObject *)NULL;
  }

  mdSession = nssCKFWSession_GetMDSession(fwSession);
  mdBaseKeyObject = nssCKFWObject_GetMDObject(fwBaseKeyObject);
  mdObject = fwMechanism->mdMechanism->DeriveKey(
        fwMechanism->mdMechanism,
        fwMechanism,
        pMechanism,
        mdSession,
        fwSession,
        fwMechanism->mdToken,
        fwMechanism->fwToken,
        fwMechanism->mdInstance,
        fwMechanism->fwInstance,
        mdBaseKeyObject,
        fwBaseKeyObject,
        pTemplate,
        ulAttributeCount,
        pError);

  if ((NSSCKMDObject *)NULL == mdObject) {
    return (NSSCKFWObject *)NULL;
  }

  fwObject = nssCKFWObject_Create(arena, mdObject, 
        fwSession, fwMechanism->fwToken, fwMechanism->fwInstance, pError);

  return fwObject;
}

