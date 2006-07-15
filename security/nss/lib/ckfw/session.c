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
 * session.c
 *
 * This file implements the NSSCKFWSession type and methods.
 */

#ifndef CK_T
#include "ck.h"
#endif /* CK_T */

/*
 * NSSCKFWSession
 *
 *  -- create/destroy --
 *  nssCKFWSession_Create
 *  nssCKFWSession_Destroy
 *
 *  -- public accessors --
 *  NSSCKFWSession_GetMDSession
 *  NSSCKFWSession_GetArena
 *  NSSCKFWSession_CallNotification
 *  NSSCKFWSession_IsRWSession
 *  NSSCKFWSession_IsSO
 *
 *  -- implement public accessors --
 *  nssCKFWSession_GetMDSession
 *  nssCKFWSession_GetArena
 *  nssCKFWSession_CallNotification
 *  nssCKFWSession_IsRWSession
 *  nssCKFWSession_IsSO
 *
 *  -- private accessors --
 *  nssCKFWSession_GetSlot
 *  nssCKFWSession_GetSessionState
 *  nssCKFWSession_SetFWFindObjects
 *  nssCKFWSession_GetFWFindObjects
 *  nssCKFWSession_SetMDSession
 *  nssCKFWSession_SetHandle
 *  nssCKFWSession_GetHandle
 *  nssCKFWSession_RegisterSessionObject
 *  nssCKFWSession_DeegisterSessionObject
 *
 *  -- module fronts --
 *  nssCKFWSession_GetDeviceError
 *  nssCKFWSession_Login
 *  nssCKFWSession_Logout
 *  nssCKFWSession_InitPIN
 *  nssCKFWSession_SetPIN
 *  nssCKFWSession_GetOperationStateLen
 *  nssCKFWSession_GetOperationState
 *  nssCKFWSession_SetOperationState
 *  nssCKFWSession_CreateObject
 *  nssCKFWSession_CopyObject
 *  nssCKFWSession_FindObjectsInit
 *  nssCKFWSession_SeedRandom
 *  nssCKFWSession_GetRandom
 */

struct NSSCKFWSessionStr {
  NSSArena *arena;
  NSSCKMDSession *mdSession;
  NSSCKFWToken *fwToken;
  NSSCKMDToken *mdToken;
  NSSCKFWInstance *fwInstance;
  NSSCKMDInstance *mdInstance;
  CK_VOID_PTR pApplication;
  CK_NOTIFY Notify;

  /*
   * Everything above is set at creation time, and then not modified.
   * The items below are atomic.  No locking required.  If we fear
   * about pointer-copies being nonatomic, we'll lock fwFindObjects.
   */

  CK_BBOOL rw;
  NSSCKFWFindObjects *fwFindObjects;
  nssCKFWHash *sessionObjectHash;
  CK_SESSION_HANDLE hSession;
};

#ifdef DEBUG
/*
 * But first, the pointer-tracking stuff.
 *
 * NOTE: the pointer-tracking support in NSS/base currently relies
 * upon NSPR's CallOnce support.  That, however, relies upon NSPR's
 * locking, which is tied into the runtime.  We need a pointer-tracker
 * implementation that uses the locks supplied through C_Initialize.
 * That support, however, can be filled in later.  So for now, I'll
 * just do this routines as no-ops.
 */

static CK_RV
session_add_pointer
(
  const NSSCKFWSession *fwSession
)
{
  return CKR_OK;
}

static CK_RV
session_remove_pointer
(
  const NSSCKFWSession *fwSession
)
{
  return CKR_OK;
}

NSS_IMPLEMENT CK_RV
nssCKFWSession_verifyPointer
(
  const NSSCKFWSession *fwSession
)
{
  return CKR_OK;
}

#endif /* DEBUG */

/*
 * nssCKFWSession_Create
 *
 */
NSS_IMPLEMENT NSSCKFWSession *
nssCKFWSession_Create
(
  NSSCKFWToken *fwToken,
  CK_BBOOL rw,
  CK_VOID_PTR pApplication,
  CK_NOTIFY Notify,
  CK_RV *pError
)
{
  NSSArena *arena = (NSSArena *)NULL;
  NSSCKFWSession *fwSession;
  NSSCKFWSlot *fwSlot;

#ifdef NSSDEBUG
  if( (CK_RV *)NULL == pError ) {
    return (NSSCKFWSession *)NULL;
  }

  *pError = nssCKFWToken_verifyPointer(fwToken);
  if( CKR_OK != *pError ) {
    return (NSSCKFWSession *)NULL;
  }
#endif /* NSSDEBUG */

  arena = NSSArena_Create();
  if( (NSSArena *)NULL == arena ) {
    *pError = CKR_HOST_MEMORY;
    return (NSSCKFWSession *)NULL;
  }

  fwSession = nss_ZNEW(arena, NSSCKFWSession);
  if( (NSSCKFWSession *)NULL == fwSession ) {
    *pError = CKR_HOST_MEMORY;
    goto loser;
  }

  fwSession->arena = arena;
  fwSession->mdSession = (NSSCKMDSession *)NULL; /* set later */
  fwSession->fwToken = fwToken;
  fwSession->mdToken = nssCKFWToken_GetMDToken(fwToken);

  fwSlot = nssCKFWToken_GetFWSlot(fwToken);
  fwSession->fwInstance = nssCKFWSlot_GetFWInstance(fwSlot);
  fwSession->mdInstance = nssCKFWSlot_GetMDInstance(fwSlot);

  fwSession->rw = rw;
  fwSession->pApplication = pApplication;
  fwSession->Notify = Notify;

  fwSession->fwFindObjects = (NSSCKFWFindObjects *)NULL;

  fwSession->sessionObjectHash = nssCKFWHash_Create(fwSession->fwInstance, arena, pError);
  if( (nssCKFWHash *)NULL == fwSession->sessionObjectHash ) {
    if( CKR_OK == *pError ) {
      *pError = CKR_GENERAL_ERROR;
    }
    goto loser;
  }

#ifdef DEBUG
  *pError = session_add_pointer(fwSession);
  if( CKR_OK != *pError ) {
    goto loser;
  }
#endif /* DEBUG */

  return fwSession;

 loser:
  if( (NSSArena *)NULL != arena ) {
    if( fwSession && (nssCKFWHash *)NULL != fwSession->sessionObjectHash ) {
      (void)nssCKFWHash_Destroy(fwSession->sessionObjectHash);
    }
    NSSArena_Destroy(arena);
  }

  return (NSSCKFWSession *)NULL;
}

static void
nss_ckfw_session_object_destroy_iterator
(
  const void *key,
  void *value,
  void *closure
)
{
  NSSCKFWObject *fwObject = (NSSCKFWObject *)value;
  nssCKFWObject_Finalize(fwObject);
}

/*
 * nssCKFWSession_Destroy
 *
 */
NSS_IMPLEMENT CK_RV
nssCKFWSession_Destroy
(
  NSSCKFWSession *fwSession,
  CK_BBOOL removeFromTokenHash
)
{
  CK_RV error = CKR_OK;
  nssCKFWHash *sessionObjectHash;

#ifdef NSSDEBUG
  error = nssCKFWSession_verifyPointer(fwSession);
  if( CKR_OK != error ) {
    return error;
  }
#endif /* NSSDEBUG */

  if( removeFromTokenHash ) {
    error = nssCKFWToken_RemoveSession(fwSession->fwToken, fwSession);
  }

  /*
   * Invalidate session objects
   */

  sessionObjectHash = fwSession->sessionObjectHash;
  fwSession->sessionObjectHash = (nssCKFWHash *)NULL;

  nssCKFWHash_Iterate(sessionObjectHash, 
                      nss_ckfw_session_object_destroy_iterator, 
                      (void *)NULL);

#ifdef DEBUG
  (void)session_remove_pointer(fwSession);
#endif /* DEBUG */
  (void)nssCKFWHash_Destroy(sessionObjectHash);
  NSSArena_Destroy(fwSession->arena);

  return error;
}

/*
 * nssCKFWSession_GetMDSession
 *
 */
NSS_IMPLEMENT NSSCKMDSession *
nssCKFWSession_GetMDSession
(
  NSSCKFWSession *fwSession
)
{
#ifdef NSSDEBUG
  if( CKR_OK != nssCKFWSession_verifyPointer(fwSession) ) {
    return (NSSCKMDSession *)NULL;
  }
#endif /* NSSDEBUG */

  return fwSession->mdSession;
}

/*
 * nssCKFWSession_GetArena
 *
 */
NSS_IMPLEMENT NSSArena *
nssCKFWSession_GetArena
(
  NSSCKFWSession *fwSession,
  CK_RV *pError
)
{
#ifdef NSSDEBUG
  if( (CK_RV *)NULL == pError ) {
    return (NSSArena *)NULL;
  }

  *pError = nssCKFWSession_verifyPointer(fwSession);
  if( CKR_OK != *pError ) {
    return (NSSArena *)NULL;
  }
#endif /* NSSDEBUG */

  return fwSession->arena;
}

/*
 * nssCKFWSession_CallNotification
 *
 */
NSS_IMPLEMENT CK_RV
nssCKFWSession_CallNotification
(
  NSSCKFWSession *fwSession,
  CK_NOTIFICATION event
)
{
  CK_RV error = CKR_OK;
  CK_SESSION_HANDLE handle;

#ifdef NSSDEBUG
  error = nssCKFWSession_verifyPointer(fwSession);
  if( CKR_OK != error ) {
    return error;
  }
#endif /* NSSDEBUG */

  if( (CK_NOTIFY)NULL == fwSession->Notify ) {
    return CKR_OK;
  }

  handle = nssCKFWInstance_FindSessionHandle(fwSession->fwInstance, fwSession);
  if( (CK_SESSION_HANDLE)0 == handle ) {
    return CKR_GENERAL_ERROR;
  }

  error = fwSession->Notify(handle, event, fwSession->pApplication);

  return error;
}

/*
 * nssCKFWSession_IsRWSession
 *
 */
NSS_IMPLEMENT CK_BBOOL
nssCKFWSession_IsRWSession
(
  NSSCKFWSession *fwSession
)
{
#ifdef NSSDEBUG
  if( CKR_OK != nssCKFWSession_verifyPointer(fwSession) ) {
    return CK_FALSE;
  }
#endif /* NSSDEBUG */

  return fwSession->rw;
}

/*
 * nssCKFWSession_IsSO
 *
 */
NSS_IMPLEMENT CK_BBOOL
nssCKFWSession_IsSO
(
  NSSCKFWSession *fwSession
)
{
  CK_STATE state;

#ifdef NSSDEBUG
  if( CKR_OK != nssCKFWSession_verifyPointer(fwSession) ) {
    return CK_FALSE;
  }
#endif /* NSSDEBUG */

  state = nssCKFWToken_GetSessionState(fwSession->fwToken);
  switch( state ) {
  case CKS_RO_PUBLIC_SESSION:
  case CKS_RO_USER_FUNCTIONS:
  case CKS_RW_PUBLIC_SESSION:
  case CKS_RW_USER_FUNCTIONS:
    return CK_FALSE;
  case CKS_RW_SO_FUNCTIONS:
    return CK_TRUE;
  default:
    return CK_FALSE;
  }
}

/*
 * nssCKFWSession_GetFWSlot
 *
 */
NSS_IMPLEMENT NSSCKFWSlot *
nssCKFWSession_GetFWSlot
(
  NSSCKFWSession *fwSession
)
{
#ifdef NSSDEBUG
  if( CKR_OK != nssCKFWSession_verifyPointer(fwSession) ) {
    return (NSSCKFWSlot *)NULL;
  }
#endif /* NSSDEBUG */

  return nssCKFWToken_GetFWSlot(fwSession->fwToken);
}

/*
 * nssCFKWSession_GetSessionState
 *
 */
NSS_IMPLEMENT CK_STATE
nssCKFWSession_GetSessionState
(
  NSSCKFWSession *fwSession
)
{
#ifdef NSSDEBUG
  if( CKR_OK != nssCKFWSession_verifyPointer(fwSession) ) {
    return CKS_RO_PUBLIC_SESSION; /* whatever */
  }
#endif /* NSSDEBUG */

  return nssCKFWToken_GetSessionState(fwSession->fwToken);
}

/*
 * nssCKFWSession_SetFWFindObjects
 *
 */
NSS_IMPLEMENT CK_RV
nssCKFWSession_SetFWFindObjects
(
  NSSCKFWSession *fwSession,
  NSSCKFWFindObjects *fwFindObjects
)
{
#ifdef NSSDEBUG
  CK_RV error = CKR_OK;
#endif /* NSSDEBUG */

#ifdef NSSDEBUG
  error = nssCKFWSession_verifyPointer(fwSession);
  if( CKR_OK != error ) {
    return error;
  }

  /* fwFindObjects may be null */
#endif /* NSSDEBUG */

  if( ((NSSCKFWFindObjects *)NULL != fwSession->fwFindObjects) &&
      ((NSSCKFWFindObjects *)NULL != fwFindObjects) ) {
    return CKR_OPERATION_ACTIVE;
  }

  fwSession->fwFindObjects = fwFindObjects;

  return CKR_OK;
}

/*
 * nssCKFWSession_GetFWFindObjects
 *
 */
NSS_IMPLEMENT NSSCKFWFindObjects *
nssCKFWSession_GetFWFindObjects
(
  NSSCKFWSession *fwSession,
  CK_RV *pError
)
{
#ifdef NSSDEBUG
  if( (CK_RV *)NULL == pError ) {
    return (NSSCKFWFindObjects *)NULL;
  }

  *pError = nssCKFWSession_verifyPointer(fwSession);
  if( CKR_OK != *pError ) {
    return (NSSCKFWFindObjects *)NULL;
  }
#endif /* NSSDEBUG */

  if( (NSSCKFWFindObjects *)NULL == fwSession->fwFindObjects ) {
    *pError = CKR_OPERATION_NOT_INITIALIZED;
    return (NSSCKFWFindObjects *)NULL;
  }

  return fwSession->fwFindObjects;
}

/*
 * nssCKFWSession_SetMDSession
 *
 */
NSS_IMPLEMENT CK_RV
nssCKFWSession_SetMDSession
(
  NSSCKFWSession *fwSession,
  NSSCKMDSession *mdSession
)
{
#ifdef NSSDEBUG
  CK_RV error = CKR_OK;
#endif /* NSSDEBUG */

#ifdef NSSDEBUG
  error = nssCKFWSession_verifyPointer(fwSession);
  if( CKR_OK != error ) {
    return error;
  }

  if( (NSSCKMDSession *)NULL == mdSession ) {
    return CKR_ARGUMENTS_BAD;
  }
#endif /* NSSDEBUG */

  if( (NSSCKMDSession *)NULL != fwSession->mdSession ) {
    return CKR_GENERAL_ERROR;
  }

  fwSession->mdSession = mdSession;

  return CKR_OK;
}

/*
 * nssCKFWSession_SetHandle
 *
 */
NSS_IMPLEMENT CK_RV
nssCKFWSession_SetHandle
(
  NSSCKFWSession *fwSession,
  CK_SESSION_HANDLE hSession
)
{
#ifdef NSSDEBUG
  CK_RV error = CKR_OK;
#endif /* NSSDEBUG */

#ifdef NSSDEBUG
  error = nssCKFWSession_verifyPointer(fwSession);
  if( CKR_OK != error ) {
    return error;
  }
#endif /* NSSDEBUG */

  if( (CK_SESSION_HANDLE)0 != fwSession->hSession ) {
    return CKR_GENERAL_ERROR;
  }

  fwSession->hSession = hSession;

  return CKR_OK;
}

/*
 * nssCKFWSession_GetHandle
 *
 */
NSS_IMPLEMENT CK_SESSION_HANDLE
nssCKFWSession_GetHandle
(
  NSSCKFWSession *fwSession
)
{
#ifdef NSSDEBUG
  if( CKR_OK != nssCKFWSession_verifyPointer(fwSession) ) {
    return NULL;
  }
#endif /* NSSDEBUG */

  return fwSession->hSession;
}

/*
 * nssCKFWSession_RegisterSessionObject
 *
 */
NSS_IMPLEMENT CK_RV
nssCKFWSession_RegisterSessionObject
(
  NSSCKFWSession *fwSession,
  NSSCKFWObject *fwObject
)
{
  CK_RV rv = CKR_OK;

#ifdef NSSDEBUG
  if( CKR_OK != nssCKFWSession_verifyPointer(fwSession) ) {
    return CKR_GENERAL_ERROR;
  }
#endif /* NSSDEBUG */

  if( (nssCKFWHash *)NULL != fwSession->sessionObjectHash ) {
    rv = nssCKFWHash_Add(fwSession->sessionObjectHash, fwObject, fwObject);
  }

  return rv;
}

/*
 * nssCKFWSession_DeregisterSessionObject
 *
 */
NSS_IMPLEMENT CK_RV
nssCKFWSession_DeregisterSessionObject
(
  NSSCKFWSession *fwSession,
  NSSCKFWObject *fwObject
)
{
#ifdef NSSDEBUG
  if( CKR_OK != nssCKFWSession_verifyPointer(fwSession) ) {
    return CKR_GENERAL_ERROR;
  }
#endif /* NSSDEBUG */

  if( (nssCKFWHash *)NULL != fwSession->sessionObjectHash ) {
    nssCKFWHash_Remove(fwSession->sessionObjectHash, fwObject);
  }

  return CKR_OK;
}

/*
 * nssCKFWSession_GetDeviceError
 *
 */
NSS_IMPLEMENT CK_ULONG
nssCKFWSession_GetDeviceError
(
  NSSCKFWSession *fwSession
)
{
#ifdef NSSDEBUG
  if( CKR_OK != nssCKFWSession_verifyPointer(fwSession) ) {
    return (CK_ULONG)0;
  }

  if( (NSSCKMDSession *)NULL == fwSession->mdSession ) {
    return (CK_ULONG)0;
  }
#endif /* NSSDEBUG */

  if( (void *)NULL == (void *)fwSession->mdSession->GetDeviceError ) {
    return (CK_ULONG)0;
  }

  return fwSession->mdSession->GetDeviceError(fwSession->mdSession, 
    fwSession, fwSession->mdToken, fwSession->fwToken, 
    fwSession->mdInstance, fwSession->fwInstance);
}

/*
 * nssCKFWSession_Login
 *
 */
NSS_IMPLEMENT CK_RV
nssCKFWSession_Login
(
  NSSCKFWSession *fwSession,
  CK_USER_TYPE userType,
  NSSItem *pin
)
{
  CK_RV error = CKR_OK;
  CK_STATE oldState;
  CK_STATE newState;

#ifdef NSSDEBUG
  error = nssCKFWSession_verifyPointer(fwSession);
  if( CKR_OK != error ) {
    return error;
  }

  switch( userType ) {
  case CKU_SO:
  case CKU_USER:
    break;
  default:
    return CKR_USER_TYPE_INVALID;
  }

  if( (NSSItem *)NULL == pin ) {
    if( CK_TRUE != nssCKFWToken_GetHasProtectedAuthenticationPath(fwSession->fwToken) ) {
      return CKR_ARGUMENTS_BAD;
    }
  }

  if( (NSSCKMDSession *)NULL == fwSession->mdSession ) {
    return CKR_GENERAL_ERROR;
  }
#endif /* NSSDEBUG */

  oldState = nssCKFWToken_GetSessionState(fwSession->fwToken);

  /*
   * It's not clear what happens when you're already logged in.
   * I'll just fail; but if we decide to change, the logic is
   * all right here.
   */

  if( CKU_SO == userType ) {
    switch( oldState ) {
    case CKS_RO_PUBLIC_SESSION:      
      /*
       * There's no such thing as a read-only security officer
       * session, so fail.  The error should be CKR_SESSION_READ_ONLY,
       * except that C_Login isn't defined to return that.  So we'll
       * do CKR_SESSION_READ_ONLY_EXISTS, which is what is documented.
       */
      return CKR_SESSION_READ_ONLY_EXISTS;
    case CKS_RO_USER_FUNCTIONS:
      return CKR_USER_ANOTHER_ALREADY_LOGGED_IN;
    case CKS_RW_PUBLIC_SESSION:
      newState = CKS_RW_SO_FUNCTIONS;
      break;
    case CKS_RW_USER_FUNCTIONS:
      return CKR_USER_ANOTHER_ALREADY_LOGGED_IN;
    case CKS_RW_SO_FUNCTIONS:
      return CKR_USER_ALREADY_LOGGED_IN;
    default:
      return CKR_GENERAL_ERROR;
    }
  } else /* CKU_USER == userType */ {
    switch( oldState ) {
    case CKS_RO_PUBLIC_SESSION:      
      newState = CKS_RO_USER_FUNCTIONS;
      break;
    case CKS_RO_USER_FUNCTIONS:
      return CKR_USER_ALREADY_LOGGED_IN;
    case CKS_RW_PUBLIC_SESSION:
      newState = CKS_RW_USER_FUNCTIONS;
      break;
    case CKS_RW_USER_FUNCTIONS:
      return CKR_USER_ALREADY_LOGGED_IN;
    case CKS_RW_SO_FUNCTIONS:
      return CKR_USER_ANOTHER_ALREADY_LOGGED_IN;
    default:
      return CKR_GENERAL_ERROR;
    }
  }

  /*
   * So now we're in one of three cases:
   *
   * Old == CKS_RW_PUBLIC_SESSION, New == CKS_RW_SO_FUNCTIONS;
   * Old == CKS_RW_PUBLIC_SESSION, New == CKS_RW_USER_FUNCTIONS;
   * Old == CKS_RO_PUBLIC_SESSION, New == CKS_RO_USER_FUNCTIONS;
   */

  if( (void *)NULL == (void *)fwSession->mdSession->Login ) {
    /*
     * The Module doesn't want to be informed (or check the pin)
     * it'll just rely on the Framework as needed.
     */
    ;
  } else {
    error = fwSession->mdSession->Login(fwSession->mdSession, fwSession,
      fwSession->mdToken, fwSession->fwToken, fwSession->mdInstance,
      fwSession->fwInstance, userType, pin, oldState, newState);
    if( CKR_OK != error ) {
      return error;
    }
  }

  (void)nssCKFWToken_SetSessionState(fwSession->fwToken, newState);
  return CKR_OK;
}

/*
 * nssCKFWSession_Logout
 *
 */
NSS_IMPLEMENT CK_RV
nssCKFWSession_Logout
(
  NSSCKFWSession *fwSession
)
{
  CK_RV error = CKR_OK;
  CK_STATE oldState;
  CK_STATE newState;

#ifdef NSSDEBUG
  error = nssCKFWSession_verifyPointer(fwSession);
  if( CKR_OK != error ) {
    return error;
  }

  if( (NSSCKMDSession *)NULL == fwSession->mdSession ) {
    return CKR_GENERAL_ERROR;
  }
#endif /* NSSDEBUG */

  oldState = nssCKFWToken_GetSessionState(fwSession->fwToken);

  switch( oldState ) {
  case CKS_RO_PUBLIC_SESSION:
    return CKR_USER_NOT_LOGGED_IN;
  case CKS_RO_USER_FUNCTIONS:
    newState = CKS_RO_PUBLIC_SESSION;
    break;
  case CKS_RW_PUBLIC_SESSION:
    return CKR_USER_NOT_LOGGED_IN;
  case CKS_RW_USER_FUNCTIONS:
    newState = CKS_RW_PUBLIC_SESSION;
    break;
  case CKS_RW_SO_FUNCTIONS:
    newState = CKS_RW_PUBLIC_SESSION;
    break;
  default:
    return CKR_GENERAL_ERROR;
  }

  /*
   * So now we're in one of three cases:
   *
   * Old == CKS_RW_SO_FUNCTIONS,   New == CKS_RW_PUBLIC_SESSION;
   * Old == CKS_RW_USER_FUNCTIONS, New == CKS_RW_PUBLIC_SESSION;
   * Old == CKS_RO_USER_FUNCTIONS, New == CKS_RO_PUBLIC_SESSION;
   */

  if( (void *)NULL == (void *)fwSession->mdSession->Logout ) {
    /*
     * The Module doesn't want to be informed.  Okay.
     */
    ;
  } else {
    error = fwSession->mdSession->Logout(fwSession->mdSession, fwSession,
      fwSession->mdToken, fwSession->fwToken, fwSession->mdInstance,
      fwSession->fwInstance, oldState, newState);
    if( CKR_OK != error ) {
      /*
       * Now what?!  A failure really should end up with the Framework
       * considering it logged out, right?
       */
      ;
    }
  }

  (void)nssCKFWToken_SetSessionState(fwSession->fwToken, newState);
  return error;
}

/*
 * nssCKFWSession_InitPIN
 *
 */
NSS_IMPLEMENT CK_RV
nssCKFWSession_InitPIN
(
  NSSCKFWSession *fwSession,
  NSSItem *pin
)
{
  CK_RV error = CKR_OK;
  CK_STATE state;

#ifdef NSSDEBUG
  error = nssCKFWSession_verifyPointer(fwSession);
  if( CKR_OK != error ) {
    return error;
  }

  if( (NSSCKMDSession *)NULL == fwSession->mdSession ) {
    return CKR_GENERAL_ERROR;
  }
#endif /* NSSDEBUG */

  state = nssCKFWToken_GetSessionState(fwSession->fwToken);
  if( CKS_RW_SO_FUNCTIONS != state ) {
    return CKR_USER_NOT_LOGGED_IN;
  }

  if( (NSSItem *)NULL == pin ) {
    CK_BBOOL has = nssCKFWToken_GetHasProtectedAuthenticationPath(fwSession->fwToken);
    if( CK_TRUE != has ) {
      return CKR_ARGUMENTS_BAD;
    }
  }

  if( (void *)NULL == (void *)fwSession->mdSession->InitPIN ) {
    return CKR_TOKEN_WRITE_PROTECTED;
  }

  error = fwSession->mdSession->InitPIN(fwSession->mdSession, fwSession,
    fwSession->mdToken, fwSession->fwToken, fwSession->mdInstance,
    fwSession->fwInstance, pin);

  return error;
}

/*
 * nssCKFWSession_SetPIN
 *
 */
NSS_IMPLEMENT CK_RV
nssCKFWSession_SetPIN
(
  NSSCKFWSession *fwSession,
  NSSItem *newPin,
  NSSItem *oldPin
)
{
  CK_RV error = CKR_OK;
  CK_STATE state;

#ifdef NSSDEBUG
  error = nssCKFWSession_verifyPointer(fwSession);
  if( CKR_OK != error ) {
    return error;
  }

  if( (NSSCKMDSession *)NULL == fwSession->mdSession ) {
    return CKR_GENERAL_ERROR;
  }
#endif /* NSSDEBUG */

  state = nssCKFWToken_GetSessionState(fwSession->fwToken);
  if( (CKS_RW_SO_FUNCTIONS != state) &&
      (CKS_RW_USER_FUNCTIONS != state) ) {
    return CKR_USER_NOT_LOGGED_IN;
  }

  if( (NSSItem *)NULL == newPin ) {
    CK_BBOOL has = nssCKFWToken_GetHasProtectedAuthenticationPath(fwSession->fwToken);
    if( CK_TRUE != has ) {
      return CKR_ARGUMENTS_BAD;
    }
  }

  if( (NSSItem *)NULL == oldPin ) {
    CK_BBOOL has = nssCKFWToken_GetHasProtectedAuthenticationPath(fwSession->fwToken);
    if( CK_TRUE != has ) {
      return CKR_ARGUMENTS_BAD;
    }
  }

  if( (void *)NULL == (void *)fwSession->mdSession->SetPIN ) {
    return CKR_TOKEN_WRITE_PROTECTED;
  }

  error = fwSession->mdSession->SetPIN(fwSession->mdSession, fwSession,
    fwSession->mdToken, fwSession->fwToken, fwSession->mdInstance,
    fwSession->fwInstance, newPin, oldPin);

  return error;
}

/*
 * nssCKFWSession_GetOperationStateLen
 *
 */
NSS_IMPLEMENT CK_ULONG
nssCKFWSession_GetOperationStateLen
(
  NSSCKFWSession *fwSession,
  CK_RV *pError
)
{
  CK_ULONG mdAmt;
  CK_ULONG fwAmt;

#ifdef NSSDEBUG
  if( (CK_RV *)NULL == pError ) {
    return (CK_ULONG)0;
  }

  *pError = nssCKFWSession_verifyPointer(fwSession);
  if( CKR_OK != *pError ) {
    return (CK_ULONG)0;
  }

  if( (NSSCKMDSession *)NULL == fwSession->mdSession ) {
    *pError = CKR_GENERAL_ERROR;
    return (CK_ULONG)0;
  }
#endif /* NSSDEBUG */

  if( (void *)NULL == (void *)fwSession->mdSession->GetOperationStateLen ) {
    *pError = CKR_STATE_UNSAVEABLE;
  }

  /*
   * We could check that the session is actually in some state..
   */

  mdAmt = fwSession->mdSession->GetOperationStateLen(fwSession->mdSession,
    fwSession, fwSession->mdToken, fwSession->fwToken, fwSession->mdInstance,
    fwSession->fwInstance, pError);

  if( ((CK_ULONG)0 == mdAmt) && (CKR_OK != *pError) ) {
    return (CK_ULONG)0;
  }

  /*
   * Add a bit of sanity-checking
   */
  fwAmt = mdAmt + 2*sizeof(CK_ULONG);

  return fwAmt;
}

/*
 * nssCKFWSession_GetOperationState
 *
 */
NSS_IMPLEMENT CK_RV
nssCKFWSession_GetOperationState
(
  NSSCKFWSession *fwSession,
  NSSItem *buffer
)
{
  CK_RV error = CKR_OK;
  CK_ULONG fwAmt;
  CK_ULONG *ulBuffer;
  NSSItem i2;
  CK_ULONG n, i;

#ifdef NSSDEBUG
  error = nssCKFWSession_verifyPointer(fwSession);
  if( CKR_OK != error ) {
    return error;
  }

  if( (NSSItem *)NULL == buffer ) {
    return CKR_ARGUMENTS_BAD;
  }

  if( (void *)NULL == buffer->data ) {
    return CKR_ARGUMENTS_BAD;
  }

  if( (NSSCKMDSession *)NULL == fwSession->mdSession ) {
    return CKR_GENERAL_ERROR;
  }
#endif /* NSSDEBUG */

  if( (void *)NULL == (void *)fwSession->mdSession->GetOperationState ) {
    return CKR_STATE_UNSAVEABLE;
  }

  /*
   * Sanity-check the caller's buffer.
   */

  error = CKR_OK;
  fwAmt = nssCKFWSession_GetOperationStateLen(fwSession, &error);
  if( ((CK_ULONG)0 == fwAmt) && (CKR_OK != error) ) {
    return error;
  }

  if( buffer->size < fwAmt ) {
    return CKR_BUFFER_TOO_SMALL;
  }

  ulBuffer = (CK_ULONG *)buffer->data;

  i2.size = buffer->size - 2*sizeof(CK_ULONG);
  i2.data = (void *)&ulBuffer[2];

  error = fwSession->mdSession->GetOperationState(fwSession->mdSession,
    fwSession, fwSession->mdToken, fwSession->fwToken, 
    fwSession->mdInstance, fwSession->fwInstance, &i2);

  if( CKR_OK != error ) {
    return error;
  }

  /*
   * Add a little integrety/identity check.  
   * NOTE: right now, it's pretty stupid.  
   * A CRC or something would be better.
   */

  ulBuffer[0] = 0x434b4657; /* CKFW */
  ulBuffer[1] = 0;
  n = i2.size/sizeof(CK_ULONG);
  for( i = 0; i < n; i++ ) {
    ulBuffer[1] ^= ulBuffer[2+i];
  }

  return CKR_OK;
}

/*
 * nssCKFWSession_SetOperationState
 *
 */
NSS_IMPLEMENT CK_RV
nssCKFWSession_SetOperationState
(
  NSSCKFWSession *fwSession,
  NSSItem *state,
  NSSCKFWObject *encryptionKey,
  NSSCKFWObject *authenticationKey
)
{
  CK_RV error = CKR_OK;
  CK_ULONG *ulBuffer;
  CK_ULONG n, i;
  CK_ULONG x;
  NSSItem s;
  NSSCKMDObject *mdek;
  NSSCKMDObject *mdak;

#ifdef NSSDEBUG
  error = nssCKFWSession_verifyPointer(fwSession);
  if( CKR_OK != error ) {
    return error;
  }

  if( (NSSItem *)NULL == state ) {
    return CKR_ARGUMENTS_BAD;
  }

  if( (void *)NULL == state->data ) {
    return CKR_ARGUMENTS_BAD;
  }

  if( (NSSCKFWObject *)NULL != encryptionKey ) {
    error = nssCKFWObject_verifyPointer(encryptionKey);
    if( CKR_OK != error ) {
      return error;
    }
  }

  if( (NSSCKFWObject *)NULL != authenticationKey ) {
    error = nssCKFWObject_verifyPointer(authenticationKey);
    if( CKR_OK != error ) {
      return error;
    }
  }

  if( (NSSCKMDSession *)NULL == fwSession->mdSession ) {
    return CKR_GENERAL_ERROR;
  }
#endif /* NSSDEBUG */

  ulBuffer = (CK_ULONG *)state->data;
  if( 0x43b4657 != ulBuffer[0] ) {
    return CKR_SAVED_STATE_INVALID;
  }
  n = (state->size / sizeof(CK_ULONG)) - 2;
  x = (CK_ULONG)0;
  for( i = 0; i < n; i++ ) {
    x ^= ulBuffer[2+i];
  }

  if( x != ulBuffer[1] ) {
    return CKR_SAVED_STATE_INVALID;
  }

  if( (void *)NULL == (void *)fwSession->mdSession->SetOperationState ) {
    return CKR_GENERAL_ERROR;
  }

  s.size = state->size - 2*sizeof(CK_ULONG);
  s.data = (void *)&ulBuffer[2];

  if( (NSSCKFWObject *)NULL != encryptionKey ) {
    mdek = nssCKFWObject_GetMDObject(encryptionKey);
  } else {
    mdek = (NSSCKMDObject *)NULL;
  }

  if( (NSSCKFWObject *)NULL != authenticationKey ) {
    mdak = nssCKFWObject_GetMDObject(authenticationKey);
  } else {
    mdak = (NSSCKMDObject *)NULL;
  }

  error = fwSession->mdSession->SetOperationState(fwSession->mdSession, 
    fwSession, fwSession->mdToken, fwSession->fwToken, fwSession->mdInstance,
    fwSession->fwInstance, &s, mdek, encryptionKey, mdak, authenticationKey);

  if( CKR_OK != error ) {
    return error;
  }

  /*
   * Here'd we restore any session data
   */
  
  return CKR_OK;
}

static CK_BBOOL
nss_attributes_form_token_object
(
  CK_ATTRIBUTE_PTR pTemplate,
  CK_ULONG ulAttributeCount
)
{
  CK_ULONG i;
  CK_BBOOL rv;

  for( i = 0; i < ulAttributeCount; i++ ) {
    if( CKA_TOKEN == pTemplate[i].type ) {
      /* If we sanity-check, we can remove this sizeof check */
      if( sizeof(CK_BBOOL) == pTemplate[i].ulValueLen ) {
        (void)nsslibc_memcpy(&rv, pTemplate[i].pValue, sizeof(CK_BBOOL));
        return rv;
      } else {
        return CK_FALSE;
      }
    }
  }

  return CK_FALSE;
}

/*
 * nssCKFWSession_CreateObject
 *
 */
NSS_IMPLEMENT NSSCKFWObject *
nssCKFWSession_CreateObject
(
  NSSCKFWSession *fwSession,
  CK_ATTRIBUTE_PTR pTemplate,
  CK_ULONG ulAttributeCount,
  CK_RV *pError
)
{
  NSSArena *arena;
  NSSCKMDObject *mdObject;
  NSSCKFWObject *fwObject;
  CK_BBOOL isTokenObject;

#ifdef NSSDEBUG
  if( (CK_RV *)NULL == pError ) {
    return (NSSCKFWObject *)NULL;
  }

  *pError = nssCKFWSession_verifyPointer(fwSession);
  if( CKR_OK != pError ) {
    return (NSSCKFWObject *)NULL;
  }

  if( (CK_ATTRIBUTE_PTR)NULL == pTemplate ) {
    *pError = CKR_ARGUMENTS_BAD;
    return (NSSCKFWObject *)NULL;
  }

  if( (NSSCKMDSession *)NULL == fwSession->mdSession ) {
    *pError = CKR_GENERAL_ERROR;
    return (NSSCKFWObject *)NULL;
  }
#endif /* NSSDEBUG */

  /*
   * Here would be an excellent place to sanity-check the object.
   */

  isTokenObject = nss_attributes_form_token_object(pTemplate, ulAttributeCount);
  if( CK_TRUE == isTokenObject ) {
    /* === TOKEN OBJECT === */

    if( (void *)NULL == (void *)fwSession->mdSession->CreateObject ) {
      *pError = CKR_TOKEN_WRITE_PROTECTED;
      return (NSSCKFWObject *)NULL;
    }

    arena = nssCKFWToken_GetArena(fwSession->fwToken, pError);
    if( (NSSArena *)NULL == arena ) {
      if( CKR_OK == *pError ) {
        *pError = CKR_GENERAL_ERROR;
      }
      return (NSSCKFWObject *)NULL;
    }

    goto callmdcreateobject;
  } else {
    /* === SESSION OBJECT === */

    arena = nssCKFWSession_GetArena(fwSession, pError);
    if( (NSSArena *)NULL == arena ) {
      if( CKR_OK == *pError ) {
        *pError = CKR_GENERAL_ERROR;
      }
      return (NSSCKFWObject *)NULL;
    }

    if( CK_TRUE == nssCKFWInstance_GetModuleHandlesSessionObjects(
                     fwSession->fwInstance) ) {
      /* --- module handles the session object -- */

      if( (void *)NULL == (void *)fwSession->mdSession->CreateObject ) {
        *pError = CKR_GENERAL_ERROR;
        return (NSSCKFWObject *)NULL;
      }
      
      goto callmdcreateobject;
    } else {
      /* --- framework handles the session object -- */
      mdObject = nssCKMDSessionObject_Create(fwSession->fwToken, 
        arena, pTemplate, ulAttributeCount, pError);
      goto gotmdobject;
    }
  }

 callmdcreateobject:
  mdObject = fwSession->mdSession->CreateObject(fwSession->mdSession,
    fwSession, fwSession->mdToken, fwSession->fwToken,
    fwSession->mdInstance, fwSession->fwInstance, arena, pTemplate,
    ulAttributeCount, pError);

 gotmdobject:
  if( (NSSCKMDObject *)NULL == mdObject ) {
    if( CKR_OK == *pError ) {
      *pError = CKR_GENERAL_ERROR;
    }
    return (NSSCKFWObject *)NULL;
  }

  fwObject = nssCKFWObject_Create(arena, mdObject, fwSession, 
    fwSession->fwToken, fwSession->fwInstance, pError);
  if( (NSSCKFWObject *)NULL == fwObject ) {
    if( CKR_OK == *pError ) {
      *pError = CKR_GENERAL_ERROR;
    }
    
    if( (void *)NULL != (void *)mdObject->Destroy ) {
      (void)mdObject->Destroy(mdObject, (NSSCKFWObject *)NULL,
        fwSession->mdSession, fwSession, fwSession->mdToken,
        fwSession->fwToken, fwSession->mdInstance, fwSession->fwInstance);
    }
    
    return (NSSCKFWObject *)NULL;
  }

  if( CK_FALSE == isTokenObject ) {
    if( CK_FALSE == nssCKFWHash_Exists(fwSession->sessionObjectHash, fwObject) ) {
      *pError = nssCKFWHash_Add(fwSession->sessionObjectHash, fwObject, fwObject);
      if( CKR_OK != *pError ) {
        nssCKFWObject_Finalize(fwObject);
        return (NSSCKFWObject *)NULL;
      }
    }
  }
  
  return fwObject;
}

/*
 * nssCKFWSession_CopyObject
 *
 */
NSS_IMPLEMENT NSSCKFWObject *
nssCKFWSession_CopyObject
(
  NSSCKFWSession *fwSession,
  NSSCKFWObject *fwObject,
  CK_ATTRIBUTE_PTR pTemplate,
  CK_ULONG ulAttributeCount,
  CK_RV *pError
)
{
  CK_BBOOL oldIsToken;
  CK_BBOOL newIsToken;
  CK_ULONG i;
  NSSCKFWObject *rv;

#ifdef NSSDEBUG
  if( (CK_RV *)NULL == pError ) {
    return (NSSCKFWObject *)NULL;
  }

  *pError = nssCKFWSession_verifyPointer(fwSession);
  if( CKR_OK != *pError ) {
    return (NSSCKFWObject *)NULL;
  }

  *pError = nssCKFWObject_verifyPointer(fwObject);
  if( CKR_OK != *pError ) {
    return (NSSCKFWObject *)NULL;
  }

  if( (NSSCKMDSession *)NULL == fwSession->mdSession ) {
    *pError = CKR_GENERAL_ERROR;
    return (NSSCKFWObject *)NULL;
  }
#endif /* NSSDEBUG */

  /*
   * Sanity-check object
   */

  oldIsToken = nssCKFWObject_IsTokenObject(fwObject);

  newIsToken = oldIsToken;
  for( i = 0; i < ulAttributeCount; i++ ) {
    if( CKA_TOKEN == pTemplate[i].type ) {
      /* Since we sanity-checked the object, we know this is the right size. */
      (void)nsslibc_memcpy(&newIsToken, pTemplate[i].pValue, sizeof(CK_BBOOL));
      break;
    }
  }

  /*
   * If the Module handles its session objects, or if both the new
   * and old object are token objects, use CopyObject if it exists.
   */

  if( ((void *)NULL != (void *)fwSession->mdSession->CopyObject) &&
      (((CK_TRUE == oldIsToken) && (CK_TRUE == newIsToken)) ||
       (CK_TRUE == nssCKFWInstance_GetModuleHandlesSessionObjects(
                     fwSession->fwInstance))) ) {
    /* use copy object */
    NSSArena *arena;
    NSSCKMDObject *mdOldObject;
    NSSCKMDObject *mdObject;

    mdOldObject = nssCKFWObject_GetMDObject(fwObject);

    if( CK_TRUE == newIsToken ) {
      arena = nssCKFWToken_GetArena(fwSession->fwToken, pError);
    } else {
      arena = nssCKFWSession_GetArena(fwSession, pError);
    }
    if( (NSSArena *)NULL == arena ) {
      if( CKR_OK == *pError ) {
        *pError = CKR_GENERAL_ERROR;
      }
      return (NSSCKFWObject *)NULL;
    }

    mdObject = fwSession->mdSession->CopyObject(fwSession->mdSession,
      fwSession, fwSession->mdToken, fwSession->fwToken,
      fwSession->mdInstance, fwSession->fwInstance, mdOldObject,
      fwObject, arena, pTemplate, ulAttributeCount, pError);
    if( (NSSCKMDObject *)NULL == mdObject ) {
      if( CKR_OK == *pError ) {
        *pError = CKR_GENERAL_ERROR;
      }
      return (NSSCKFWObject *)NULL;
    }

    rv = nssCKFWObject_Create(arena, mdObject, fwSession,
      fwSession->fwToken, fwSession->fwInstance, pError);
    if( (NSSCKFWObject *)NULL == fwObject ) {
      if( CKR_OK == *pError ) {
        *pError = CKR_GENERAL_ERROR;
      }

      if( (void *)NULL != (void *)mdObject->Destroy ) {
        (void)mdObject->Destroy(mdObject, (NSSCKFWObject *)NULL,
          fwSession->mdSession, fwSession, fwSession->mdToken,
          fwSession->fwToken, fwSession->mdInstance, fwSession->fwInstance);
      }
    
      return (NSSCKFWObject *)NULL;
    }

    if( CK_FALSE == newIsToken ) {
      if( CK_FALSE == nssCKFWHash_Exists(fwSession->sessionObjectHash, rv) ) {
        *pError = nssCKFWHash_Add(fwSession->sessionObjectHash, rv, rv);
        if( CKR_OK != *pError ) {
          nssCKFWObject_Finalize(rv);
          return (NSSCKFWObject *)NULL;
        }
      }
    }

    return rv;
  } else {
    /* use create object */
    NSSArena *tmpArena;
    CK_ATTRIBUTE_PTR newTemplate;
    CK_ULONG i, j, n, newLength, k;
    CK_ATTRIBUTE_TYPE_PTR oldTypes;
    NSSCKFWObject *rv;
    
    tmpArena = NSSArena_Create();
    if( (NSSArena *)NULL == tmpArena ) {
      *pError = CKR_HOST_MEMORY;
      return (NSSCKFWObject *)NULL;
    }

    n = nssCKFWObject_GetAttributeCount(fwObject, pError);
    if( (0 == n) && (CKR_OK != *pError) ) {
      return (NSSCKFWObject *)NULL;
    }

    oldTypes = nss_ZNEWARRAY(tmpArena, CK_ATTRIBUTE_TYPE, n);
    if( (CK_ATTRIBUTE_TYPE_PTR)NULL == oldTypes ) {
      NSSArena_Destroy(tmpArena);
      *pError = CKR_HOST_MEMORY;
      return (NSSCKFWObject *)NULL;
    }

    *pError = nssCKFWObject_GetAttributeTypes(fwObject, oldTypes, n);
    if( CKR_OK != *pError ) {
      NSSArena_Destroy(tmpArena);
      return (NSSCKFWObject *)NULL;
    }

    newLength = n;
    for( i = 0; i < ulAttributeCount; i++ ) {
      for( j = 0; j < n; j++ ) {
        if( oldTypes[j] == pTemplate[i].type ) {
          if( (CK_VOID_PTR)NULL == pTemplate[i].pValue ) {
            /* Removing the attribute */
            newLength--;
          }
          break;
        }
      }
      if( j == n ) {
        /* Not found */
        newLength++;
      }
    }

    newTemplate = nss_ZNEWARRAY(tmpArena, CK_ATTRIBUTE, newLength);
    if( (CK_ATTRIBUTE_PTR)NULL == newTemplate ) {
      NSSArena_Destroy(tmpArena);
      *pError = CKR_HOST_MEMORY;
      return (NSSCKFWObject *)NULL;
    }

    k = 0;
    for( j = 0; j < n; j++ ) {
      for( i = 0; i < ulAttributeCount; i++ ) {
        if( oldTypes[j] == pTemplate[i].type ) {
          if( (CK_VOID_PTR)NULL == pTemplate[i].pValue ) {
            /* This attribute is being deleted */
            ;
          } else {
            /* This attribute is being replaced */
            newTemplate[k].type = pTemplate[i].type;
            newTemplate[k].pValue = pTemplate[i].pValue;
            newTemplate[k].ulValueLen = pTemplate[i].ulValueLen;
            k++;
          }
          break;
        }
      }
      if( i == ulAttributeCount ) {
        /* This attribute is being copied over from the old object */
        NSSItem item, *it;
        item.size = 0;
        item.data = (void *)NULL;
        it = nssCKFWObject_GetAttribute(fwObject, oldTypes[j],
          &item, tmpArena, pError);
        if( (NSSItem *)NULL == it ) {
          if( CKR_OK == *pError ) {
            *pError = CKR_GENERAL_ERROR;
          }
          NSSArena_Destroy(tmpArena);
          return (NSSCKFWObject *)NULL;
        }
        newTemplate[k].type = oldTypes[j];
        newTemplate[k].pValue = it->data;
        newTemplate[k].ulValueLen = it->size;
        k++;
      }
    }
    /* assert that k == newLength */

    rv = nssCKFWSession_CreateObject(fwSession, newTemplate, newLength, pError);
    if( (NSSCKFWObject *)NULL == rv ) {
      if( CKR_OK == *pError ) {
        *pError = CKR_GENERAL_ERROR;
      }
      NSSArena_Destroy(tmpArena);
      return (NSSCKFWObject *)NULL;
    }

    NSSArena_Destroy(tmpArena);
    return rv;
  }
}

/*
 * nssCKFWSession_FindObjectsInit
 *
 */
NSS_IMPLEMENT NSSCKFWFindObjects *
nssCKFWSession_FindObjectsInit
(
  NSSCKFWSession *fwSession,
  CK_ATTRIBUTE_PTR pTemplate,
  CK_ULONG ulAttributeCount,
  CK_RV *pError
)
{
  NSSCKMDFindObjects *mdfo1 = (NSSCKMDFindObjects *)NULL;
  NSSCKMDFindObjects *mdfo2 = (NSSCKMDFindObjects *)NULL;

#ifdef NSSDEBUG
  if( (CK_RV *)NULL == pError ) {
    return (NSSCKFWFindObjects *)NULL;
  }

  *pError = nssCKFWSession_verifyPointer(fwSession);
  if( CKR_OK != *pError ) {
    return (NSSCKFWFindObjects *)NULL;
  }

  if( ((CK_ATTRIBUTE_PTR)NULL == pTemplate) && (ulAttributeCount != 0) ) {
    *pError = CKR_ARGUMENTS_BAD;
    return (NSSCKFWFindObjects *)NULL;
  }

  if( (NSSCKMDSession *)NULL == fwSession->mdSession ) {
    *pError = CKR_GENERAL_ERROR;
    return (NSSCKFWFindObjects *)NULL;
  }
#endif /* NSSDEBUG */

  if( CK_TRUE != nssCKFWInstance_GetModuleHandlesSessionObjects(
                   fwSession->fwInstance) ) {
    CK_ULONG i;

    /*
     * Does the search criteria restrict us to token or session
     * objects?
     */

    for( i = 0; i < ulAttributeCount; i++ ) {
      if( CKA_TOKEN == pTemplate[i].type ) {
        /* Yes, it does. */
        CK_BBOOL isToken;
        if( sizeof(CK_BBOOL) != pTemplate[i].ulValueLen ) {
          *pError = CKR_ATTRIBUTE_VALUE_INVALID;
          return (NSSCKFWFindObjects *)NULL;
        }
        (void)nsslibc_memcpy(&isToken, pTemplate[i].pValue, sizeof(CK_BBOOL));

        if( CK_TRUE == isToken ) {
          /* Pass it on to the module's search routine */
          if( (void *)NULL == (void *)fwSession->mdSession->FindObjectsInit ) {
            goto wrap;
          }

          mdfo1 = fwSession->mdSession->FindObjectsInit(fwSession->mdSession,
                    fwSession, fwSession->mdToken, fwSession->fwToken,
                    fwSession->mdInstance, fwSession->fwInstance, 
                    pTemplate, ulAttributeCount, pError);
        } else {
          /* Do the search ourselves */
          mdfo1 = nssCKMDFindSessionObjects_Create(fwSession->fwToken, 
                    pTemplate, ulAttributeCount, pError);
        }

        if( (NSSCKMDFindObjects *)NULL == mdfo1 ) {
          if( CKR_OK == *pError ) {
            *pError = CKR_GENERAL_ERROR;
          }
          return (NSSCKFWFindObjects *)NULL;
        }
        
        goto wrap;
      }
    }

    if( i == ulAttributeCount ) {
      /* No, it doesn't.  Do a hybrid search. */
      mdfo1 = fwSession->mdSession->FindObjectsInit(fwSession->mdSession,
                fwSession, fwSession->mdToken, fwSession->fwToken,
                fwSession->mdInstance, fwSession->fwInstance, 
                pTemplate, ulAttributeCount, pError);

      if( (NSSCKMDFindObjects *)NULL == mdfo1 ) {
        if( CKR_OK == *pError ) {
          *pError = CKR_GENERAL_ERROR;
        }
        return (NSSCKFWFindObjects *)NULL;
      }

      mdfo2 = nssCKMDFindSessionObjects_Create(fwSession->fwToken,
                pTemplate, ulAttributeCount, pError);
      if( (NSSCKMDFindObjects *)NULL == mdfo2 ) {
        if( CKR_OK == *pError ) {
          *pError = CKR_GENERAL_ERROR;
        }
        if( (void *)NULL != (void *)mdfo1->Final ) {
          mdfo1->Final(mdfo1, (NSSCKFWFindObjects *)NULL, fwSession->mdSession,
            fwSession, fwSession->mdToken, fwSession->fwToken, 
            fwSession->mdInstance, fwSession->fwInstance);
        }
        return (NSSCKFWFindObjects *)NULL;
      }

      goto wrap;
    }
    /*NOTREACHED*/
  } else {
    /* Module handles all its own objects.  Pass on to module's search */
    mdfo1 = fwSession->mdSession->FindObjectsInit(fwSession->mdSession,
              fwSession, fwSession->mdToken, fwSession->fwToken,
              fwSession->mdInstance, fwSession->fwInstance, 
              pTemplate, ulAttributeCount, pError);

    if( (NSSCKMDFindObjects *)NULL == mdfo1 ) {
      if( CKR_OK == *pError ) {
        *pError = CKR_GENERAL_ERROR;
      }
      return (NSSCKFWFindObjects *)NULL;
    }

    goto wrap;
  }

 wrap:
  return nssCKFWFindObjects_Create(fwSession, fwSession->fwToken,
           fwSession->fwInstance, mdfo1, mdfo2, pError);
}

/*
 * nssCKFWSession_SeedRandom
 *
 */
NSS_IMPLEMENT CK_RV
nssCKFWSession_SeedRandom
(
  NSSCKFWSession *fwSession,
  NSSItem *seed
)
{
  CK_RV error = CKR_OK;

#ifdef NSSDEBUG
  error = nssCKFWSession_verifyPointer(fwSession);
  if( CKR_OK != error ) {
    return error;
  }

  if( (NSSItem *)NULL == seed ) {
    return CKR_ARGUMENTS_BAD;
  }

  if( (void *)NULL == seed->data ) {
    return CKR_ARGUMENTS_BAD;
  }

  if( 0 == seed->size ) {
    return CKR_ARGUMENTS_BAD;
  }

  if( (NSSCKMDSession *)NULL == fwSession->mdSession ) {
    return CKR_GENERAL_ERROR;
  }
#endif /* NSSDEBUG */

  if( (void *)NULL == (void *)fwSession->mdSession->SeedRandom ) {
    return CKR_RANDOM_SEED_NOT_SUPPORTED;
  }

  error = fwSession->mdSession->SeedRandom(fwSession->mdSession, fwSession,
    fwSession->mdToken, fwSession->fwToken, fwSession->mdInstance,
    fwSession->fwInstance, seed);

  return error;
}

/*
 * nssCKFWSession_GetRandom
 *
 */
NSS_IMPLEMENT CK_RV
nssCKFWSession_GetRandom
(
  NSSCKFWSession *fwSession,
  NSSItem *buffer
)
{
  CK_RV error = CKR_OK;

#ifdef NSSDEBUG
  error = nssCKFWSession_verifyPointer(fwSession);
  if( CKR_OK != error ) {
    return error;
  }

  if( (NSSItem *)NULL == buffer ) {
    return CKR_ARGUMENTS_BAD;
  }

  if( (void *)NULL == buffer->data ) {
    return CKR_ARGUMENTS_BAD;
  }

  if( (NSSCKMDSession *)NULL == fwSession->mdSession ) {
    return CKR_GENERAL_ERROR;
  }
#endif /* NSSDEBUG */

  if( (void *)NULL == (void *)fwSession->mdSession->GetRandom ) {
    if( CK_TRUE == nssCKFWToken_GetHasRNG(fwSession->fwToken) ) {
      return CKR_GENERAL_ERROR;
    } else {
      return CKR_RANDOM_NO_RNG;
    }
  }

  if( 0 == buffer->size ) {
    return CKR_OK;
  }

  error = fwSession->mdSession->GetRandom(fwSession->mdSession, fwSession,
    fwSession->mdToken, fwSession->fwToken, fwSession->mdInstance,
    fwSession->fwInstance, buffer);

  return error;
}

/*
 * NSSCKFWSession_GetMDSession
 *
 */

NSS_IMPLEMENT NSSCKMDSession *
NSSCKFWSession_GetMDSession
(
  NSSCKFWSession *fwSession
)
{
#ifdef DEBUG
  if( CKR_OK != nssCKFWSession_verifyPointer(fwSession) ) {
    return (NSSCKMDSession *)NULL;
  }
#endif /* DEBUG */

  return nssCKFWSession_GetMDSession(fwSession);
}

/*
 * NSSCKFWSession_GetArena
 *
 */

NSS_IMPLEMENT NSSArena *
NSSCKFWSession_GetArena
(
  NSSCKFWSession *fwSession,
  CK_RV *pError
)
{
#ifdef DEBUG
  if( (CK_RV *)NULL == pError ) {
    return (NSSArena *)NULL;
  }

  *pError = nssCKFWSession_verifyPointer(fwSession);
  if( CKR_OK != *pError ) {
    return (NSSArena *)NULL;
  }
#endif /* DEBUG */

  return nssCKFWSession_GetArena(fwSession, pError);
}

/*
 * NSSCKFWSession_CallNotification
 *
 */

NSS_IMPLEMENT CK_RV
NSSCKFWSession_CallNotification
(
  NSSCKFWSession *fwSession,
  CK_NOTIFICATION event
)
{
#ifdef DEBUG
  CK_RV error = CKR_OK;

  error = nssCKFWSession_verifyPointer(fwSession);
  if( CKR_OK != error ) {
    return error;
  }
#endif /* DEBUG */

  return nssCKFWSession_CallNotification(fwSession, event);
}

/*
 * NSSCKFWSession_IsRWSession
 *
 */

NSS_IMPLEMENT CK_BBOOL
NSSCKFWSession_IsRWSession
(
  NSSCKFWSession *fwSession
)
{
#ifdef DEBUG
  if( CKR_OK != nssCKFWSession_verifyPointer(fwSession) ) {
    return CK_FALSE;
  }
#endif /* DEBUG */

  return nssCKFWSession_IsRWSession(fwSession);
}

/*
 * NSSCKFWSession_IsSO
 *
 */

NSS_IMPLEMENT CK_BBOOL
NSSCKFWSession_IsSO
(
  NSSCKFWSession *fwSession
)
{
#ifdef DEBUG
  if( CKR_OK != nssCKFWSession_verifyPointer(fwSession) ) {
    return CK_FALSE;
  }
#endif /* DEBUG */

  return nssCKFWSession_IsSO(fwSession);
}
