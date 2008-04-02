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
 *  The following code handles the storage of PKCS 11 modules used by the
 * NSS. This file is written to abstract away how the modules are
 * stored so we can deside that later.
 */
#include "sftkdb.h"
#include "sftkdbti.h"
#include "sdb.h"
#include "prsystem.h"
#include "prprf.h"
#include "prenv.h"
#include "lgglue.h"
#include "secerr.h"

static LGOpenFunc legacy_glue_open = NULL;
static LGReadSecmodFunc legacy_glue_readSecmod = NULL;
static LGReleaseSecmodFunc legacy_glue_releaseSecmod = NULL;
static LGDeleteSecmodFunc legacy_glue_deleteSecmod = NULL;
static LGAddSecmodFunc legacy_glue_addSecmod = NULL;
static LGShutdownFunc legacy_glue_shutdown = NULL;

/*
 * The following 3 functions duplicate the work done by bl_LoadLibrary.
 * We should make bl_LoadLibrary a global and replace the call to
 * sftkdb_LoadLibrary(const char *libname) with it.
 */
#ifdef XP_UNIX
#include <unistd.h>
#define LG_MAX_LINKS 20
static char *
sftkdb_resolvePath(const char *orig)
{
    int count = 0;
    int len =0;
    int ret = -1;
    char *resolved = NULL;
    char *source = NULL;

    len = 1025; /* MAX PATH +1*/
    if (strlen(orig)+1 > len) {
	/* PATH TOO LONG */
	return NULL;
    }
    resolved = PORT_Alloc(len);
    if (!resolved) {
	return NULL;
    }
    source = PORT_Alloc(len);
    if (!source) {
	goto loser;
    }
    PORT_Strcpy(source, orig);
    /* Walk down all the links */
    while ( count++ < LG_MAX_LINKS) {
	char *tmp;
	/* swap our previous sorce out with resolved */
	/* read it */
	ret = readlink(source, resolved, len-1);
	if (ret  < 0) {
	    break;
 	}
	resolved[ret] = 0;
	tmp = source; source = resolved; resolved = tmp;
    }
    if (count > 1) {
	ret = 0;
    }
loser:
    if (resolved) {
	PORT_Free(resolved);
    }
    if (ret < 0) {
	if (source) {
	    PORT_Free(source);
	    source = NULL;
	}
    }
    return source;
}

#endif

static PRLibrary *
sftkdb_LoadFromPath(const char *path, const char *libname)
{
    char *c;
    int pathLen, nameLen, fullPathLen;
    char *fullPathName = NULL;
    PRLibSpec libSpec;
    PRLibrary *lib = NULL;


    /* strip of our parent's library name */ 
    c = strrchr(path, PR_GetDirectorySeparator());
    if (!c) {
	return NULL; /* invalid path */
    }
    pathLen = (c-path)+1;
    nameLen = strlen(libname);
    fullPathLen = pathLen + nameLen +1;
    fullPathName = (char *)PORT_Alloc(fullPathLen);
    if (fullPathName == NULL) {
	return NULL; /* memory allocation error */
    }
    PORT_Memcpy(fullPathName, path, pathLen);
    PORT_Memcpy(fullPathName+pathLen, libname, nameLen);
    fullPathName[fullPathLen-1] = 0;

    libSpec.type = PR_LibSpec_Pathname;
    libSpec.value.pathname = fullPathName;
    lib = PR_LoadLibraryWithFlags(libSpec, PR_LD_NOW | PR_LD_LOCAL);
    PORT_Free(fullPathName);
    return lib;
}

static PRLibrary *
sftkdb_LoadLibrary(const char *libname)
{
    PRLibrary *lib = NULL;
    PRFuncPtr fn_addr;
    char *parentLibPath = NULL;

    fn_addr  = (PRFuncPtr) &sftkdb_LoadLibrary;
    parentLibPath = PR_GetLibraryFilePathname(SOFTOKEN_LIB_NAME, fn_addr);

    if (!parentLibPath) {
	goto done;
    }

    lib = sftkdb_LoadFromPath(parentLibPath, libname);
#ifdef XP_UNIX
    /* handle symbolic link case */
    if (!lib) {
	char *trueParentLibPath = sftkdb_resolvePath(parentLibPath);
	if (!trueParentLibPath) {
	    goto done;
	}
    	lib = sftkdb_LoadFromPath(trueParentLibPath, libname);
	PORT_Free(trueParentLibPath);
    }
#endif

done:
    if (parentLibPath) {
	PORT_Free(parentLibPath);
    }

    /* still couldn't load it, try the generic path */
    if (!lib) {
	PRLibSpec libSpec;
	libSpec.type = PR_LibSpec_Pathname;
	libSpec.value.pathname = libname;
	lib = PR_LoadLibraryWithFlags(libSpec, PR_LD_NOW | PR_LD_LOCAL);
    }
    return lib;
}

/*
 * stub files for legacy db's to be able to encrypt and decrypt
 * various keys and attributes.
 */
static SECStatus
sftkdb_encrypt_stub(PRArenaPool *arena, SDB *sdb, SECItem *plainText,
		    SECItem **cipherText)
{
    SFTKDBHandle *handle = sdb->app_private;
    SECStatus rv;

    if (handle == NULL) {
	return SECFailure;
    }

    /* if we aren't th handle, try the other handle */
    if (handle->type != SFTK_KEYDB_TYPE) {
	handle = handle->peerDB;
    }

    /* not a key handle */
    if (handle == NULL || handle->passwordLock == NULL) {
	return SECFailure;
    }

    PZ_Lock(handle->passwordLock);
    if (handle->passwordKey.data == NULL) {
	PZ_Unlock(handle->passwordLock);
	/* PORT_SetError */
	return SECFailure;
    }

    rv = sftkdb_EncryptAttribute(arena, 
	handle->newKey?handle->newKey:&handle->passwordKey, 
	plainText, cipherText);
    PZ_Unlock(handle->passwordLock);

    return rv;
}

/*
 * stub files for legacy db's to be able to encrypt and decrypt
 * various keys and attributes.
 */
static SECStatus
sftkdb_decrypt_stub(SDB *sdb, SECItem *cipherText, SECItem **plainText) 
{
    SFTKDBHandle *handle = sdb->app_private;
    SECStatus rv;

    if (handle == NULL) {
	return SECFailure;
    }

    /* if we aren't th handle, try the other handle */
    if (handle->type != SFTK_KEYDB_TYPE) {
	handle = handle->peerDB;
    }

    /* not a key handle */
    if (handle == NULL || handle->passwordLock == NULL) {
	return SECFailure;
    }

    PZ_Lock(handle->passwordLock);
    if (handle->passwordKey.data == NULL) {
	PZ_Unlock(handle->passwordLock);
	/* PORT_SetError */
	return SECFailure;
    }
    rv = sftkdb_DecryptAttribute(&handle->passwordKey, cipherText, plainText);
    PZ_Unlock(handle->passwordLock);

    return rv;
}

static PRLibrary *legacy_glue_lib = NULL;
static SECStatus 
sftkdbLoad_Legacy()
{
    PRLibrary *lib = NULL;
    LGSetCryptFunc setCryptFunction = NULL;

    if (legacy_glue_lib) {
	return SECSuccess;
    }

    lib = sftkdb_LoadLibrary(SHLIB_PREFIX"nssdbm"SHLIB_VERSION"."SHLIB_SUFFIX);
    if (lib == NULL) {
	return SECFailure;
    }
    
    legacy_glue_open = (LGOpenFunc)PR_FindFunctionSymbol(lib, "legacy_Open");
    legacy_glue_readSecmod = (LGReadSecmodFunc) PR_FindFunctionSymbol(lib,
						 "legacy_ReadSecmodDB");
    legacy_glue_releaseSecmod = (LGReleaseSecmodFunc) PR_FindFunctionSymbol(lib,
					 	 "legacy_ReleaseSecmodDBData");
    legacy_glue_deleteSecmod = (LGDeleteSecmodFunc) PR_FindFunctionSymbol(lib,
						 "legacy_DeleteSecmodDB");
    legacy_glue_addSecmod = (LGAddSecmodFunc)PR_FindFunctionSymbol(lib, 
						 "legacy_AddSecmodDB");
    legacy_glue_shutdown = (LGShutdownFunc) PR_FindFunctionSymbol(lib, 
						"legacy_Shutdown");
    setCryptFunction = (LGSetCryptFunc) PR_FindFunctionSymbol(lib, 
						"legacy_SetCryptFunctions");

    if (!legacy_glue_open || !legacy_glue_readSecmod || 
	    !legacy_glue_releaseSecmod || !legacy_glue_deleteSecmod || 
	    !legacy_glue_addSecmod || !setCryptFunction) {
	PR_UnloadLibrary(lib);
	return SECFailure;
    }
    setCryptFunction(sftkdb_encrypt_stub,sftkdb_decrypt_stub);
    legacy_glue_lib = lib;
    return SECSuccess;
}

CK_RV
sftkdbCall_open(const char *dir, const char *certPrefix, const char *keyPrefix, 
		int certVersion, int keyVersion, int flags, 
		SDB **certDB, SDB **keyDB)
{
    SECStatus rv;

    rv = sftkdbLoad_Legacy();
    if (rv != SECSuccess) {
	return CKR_GENERAL_ERROR;
    }
    if (!legacy_glue_open) {
	PORT_SetError(SEC_ERROR_LIBRARY_FAILURE);
	return SECFailure;
    }
    return (*legacy_glue_open)(dir, certPrefix, keyPrefix, 
				certVersion, keyVersion,
				flags, certDB, keyDB);
}

char **
sftkdbCall_ReadSecmodDB(const char *appName, const char *filename, 
			const char *dbname, char *params, PRBool rw)
{
    SECStatus rv;

    rv = sftkdbLoad_Legacy();
    if (rv != SECSuccess) {
	return NULL;
    }
    if (!legacy_glue_readSecmod) {
	PORT_SetError(SEC_ERROR_LIBRARY_FAILURE);
	return NULL;
    }
    return (*legacy_glue_readSecmod)(appName, filename, dbname, params, rw);
}

SECStatus
sftkdbCall_ReleaseSecmodDBData(const char *appName, 
			const char *filename, const char *dbname, 
			char **moduleSpecList, PRBool rw)
{
    SECStatus rv;

    rv = sftkdbLoad_Legacy();
    if (rv != SECSuccess) {
	return rv;
    }
    if (!legacy_glue_releaseSecmod) {
	PORT_SetError(SEC_ERROR_LIBRARY_FAILURE);
	return SECFailure;
    }
    return (*legacy_glue_releaseSecmod)(appName, filename, dbname, 
					  moduleSpecList, rw);
}

SECStatus
sftkdbCall_DeleteSecmodDB(const char *appName, 
		      const char *filename, const char *dbname, 
		      char *args, PRBool rw)
{
    SECStatus rv;

    rv = sftkdbLoad_Legacy();
    if (rv != SECSuccess) {
	return rv;
    }
    if (!legacy_glue_deleteSecmod) {
	PORT_SetError(SEC_ERROR_LIBRARY_FAILURE);
	return SECFailure;
    }
    return (*legacy_glue_deleteSecmod)(appName, filename, dbname, args, rw);
}

SECStatus
sftkdbCall_AddSecmodDB(const char *appName, 
		   const char *filename, const char *dbname, 
		   char *module, PRBool rw)
{
    SECStatus rv;

    rv = sftkdbLoad_Legacy();
    if (rv != SECSuccess) {
	return rv;
    }
    if (!legacy_glue_addSecmod) {
	PORT_SetError(SEC_ERROR_LIBRARY_FAILURE);
	return SECFailure;
    }
    return (*legacy_glue_addSecmod)(appName, filename, dbname, module, rw);
}

CK_RV
sftkdbCall_Shutdown(void)
{
    CK_RV crv = CKR_OK;
    char *disableUnload = NULL;
    if (!legacy_glue_lib) {
	return CKR_OK;
    }
    if (legacy_glue_shutdown) {
	crv = (*legacy_glue_shutdown)();
    }
    disableUnload = PR_GetEnv("NSS_DISABLE_UNLOAD");
    if (!disableUnload) {
        PR_UnloadLibrary(legacy_glue_lib);
    }
    legacy_glue_lib = NULL;
    legacy_glue_open = NULL;
    legacy_glue_readSecmod = NULL;
    legacy_glue_releaseSecmod = NULL;
    legacy_glue_deleteSecmod = NULL;
    legacy_glue_addSecmod = NULL;
    return crv;
}
    

