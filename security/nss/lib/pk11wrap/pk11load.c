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
 * The following handles the loading, unloading and management of
 * various PCKS #11 modules
 */
#include "seccomon.h"
#include "pkcs11.h"
#include "secmod.h"
#include "prlink.h"
#include "pk11func.h"
#include "secmodi.h"
#include "secmodti.h"
#include "nssilock.h"
#include "secerr.h"

extern void FC_GetFunctionList(void);
extern void NSC_GetFunctionList(void);
extern void NSC_ModuleDBFunc(void);

#ifdef DEBUG
#define DEBUG_MODULE 1
#endif

#ifdef DEBUG_MODULE
static char *modToDBG = NULL;

#include "debug_module.c"
#endif

/* build the PKCS #11 2.01 lock files */
CK_RV PR_CALLBACK secmodCreateMutext(CK_VOID_PTR_PTR pmutex) {
    *pmutex = (CK_VOID_PTR) PZ_NewLock(nssILockOther);
    if ( *pmutex ) return CKR_OK;
    return CKR_HOST_MEMORY;
}

CK_RV PR_CALLBACK secmodDestroyMutext(CK_VOID_PTR mutext) {
    PZ_DestroyLock((PZLock *)mutext);
    return CKR_OK;
}

CK_RV PR_CALLBACK secmodLockMutext(CK_VOID_PTR mutext) {
    PZ_Lock((PZLock *)mutext);
    return CKR_OK;
}

CK_RV PR_CALLBACK secmodUnlockMutext(CK_VOID_PTR mutext) {
    PZ_Unlock((PZLock *)mutext);
    return CKR_OK;
}

static SECMODModuleID  nextModuleID = 1;
static const CK_C_INITIALIZE_ARGS secmodLockFunctions = {
    secmodCreateMutext, secmodDestroyMutext, secmodLockMutext, 
    secmodUnlockMutext, CKF_LIBRARY_CANT_CREATE_OS_THREADS|
	CKF_OS_LOCKING_OK
    ,NULL
};

static PRBool loadSingleThreadedModules = PR_TRUE;
static PRBool enforceAlreadyInitializedError = PR_TRUE;
static PRBool finalizeModules = PR_TRUE;

/* set global options for NSS PKCS#11 module loader */
SECStatus pk11_setGlobalOptions(PRBool noSingleThreadedModules,
                                PRBool allowAlreadyInitializedModules,
                                PRBool dontFinalizeModules)
{
    if (noSingleThreadedModules) {
        loadSingleThreadedModules = PR_FALSE;
    } else {
        loadSingleThreadedModules = PR_TRUE;
    }
    if (allowAlreadyInitializedModules) {
        enforceAlreadyInitializedError = PR_FALSE;
    } else {
        enforceAlreadyInitializedError = PR_TRUE;
    }
    if (dontFinalizeModules) {
        finalizeModules = PR_FALSE;
    } else {
        finalizeModules = PR_TRUE;
    }
    return SECSuccess;
}

PRBool pk11_getFinalizeModulesOption(void)
{
    return finalizeModules;
}

/*
 * collect the steps we need to initialize a module in a single function
 */
SECStatus
secmod_ModuleInit(SECMODModule *mod, PRBool* alreadyLoaded)
{
    CK_C_INITIALIZE_ARGS moduleArgs;
    CK_VOID_PTR pInitArgs;
    CK_RV crv;

    if (!mod || !alreadyLoaded) {
        PORT_SetError(SEC_ERROR_INVALID_ARGS);
        return SECFailure;
    }

    if (mod->isThreadSafe == PR_FALSE) {
	pInitArgs = NULL;
    } else if (mod->libraryParams == NULL) {
	pInitArgs = (void *) &secmodLockFunctions;
    } else {
	moduleArgs = secmodLockFunctions;
	moduleArgs.LibraryParameters = (void *) mod->libraryParams;
	pInitArgs = &moduleArgs;
    }
    crv = PK11_GETTAB(mod)->C_Initialize(pInitArgs);
    if ((CKR_CRYPTOKI_ALREADY_INITIALIZED == crv) &&
        (!enforceAlreadyInitializedError)) {
        *alreadyLoaded = PR_TRUE;
        return SECSuccess;
    }
    if (crv != CKR_OK) {
	if (pInitArgs == NULL ||
		crv == CKR_NETSCAPE_CERTDB_FAILED ||
		crv == CKR_NETSCAPE_KEYDB_FAILED) {
	    PORT_SetError(PK11_MapError(crv));
	    return SECFailure;
	}
	if (!loadSingleThreadedModules) {
	    PORT_SetError(SEC_ERROR_INCOMPATIBLE_PKCS11);
	    return SECFailure;
	}
	mod->isThreadSafe = PR_FALSE;
    	crv = PK11_GETTAB(mod)->C_Initialize(NULL);
	if ((CKR_CRYPTOKI_ALREADY_INITIALIZED == crv) &&
	    (!enforceAlreadyInitializedError)) {
	    *alreadyLoaded = PR_TRUE;
	    return SECSuccess;
	}
    	if (crv != CKR_OK)  {
	    PORT_SetError(PK11_MapError(crv));
	    return SECFailure;
	}
    }
    return SECSuccess;
}

/*
 * set the hasRootCerts flags in the module so it can be stored back
 * into the database.
 */
void
SECMOD_SetRootCerts(PK11SlotInfo *slot, SECMODModule *mod) {
    PK11PreSlotInfo *psi = NULL;
    int i;

    if (slot->hasRootCerts) {
	for (i=0; i < mod->slotInfoCount; i++) {
	    if (slot->slotID == mod->slotInfo[i].slotID) {
		psi = &mod->slotInfo[i];
		break;
	    }
	}
	if (psi == NULL) {
	   /* allocate more slots */
	   PK11PreSlotInfo *psi_list = (PK11PreSlotInfo *)
		PORT_ArenaAlloc(mod->arena,
			(mod->slotInfoCount+1)* sizeof(PK11PreSlotInfo));
	   /* copy the old ones */
	   if (mod->slotInfoCount > 0) {
		PORT_Memcpy(psi_list,mod->slotInfo,
				(mod->slotInfoCount)*sizeof(PK11PreSlotInfo));
	   }
	   /* assign psi to the last new slot */
	   psi = &psi_list[mod->slotInfoCount];
	   psi->slotID = slot->slotID;
	   psi->askpw = 0;
	   psi->timeout = 0;
	   psi ->defaultFlags = 0;

	   /* increment module count & store new list */
	   mod->slotInfo = psi_list;
	   mod->slotInfoCount++;
	   
	}
	psi->hasRootCerts = 1;
    }
}

/*
 * load a new module into our address space and initialize it.
 */
SECStatus
SECMOD_LoadPKCS11Module(SECMODModule *mod) {
    PRLibrary *library = NULL;
    CK_C_GetFunctionList entry = NULL;
    char * full_name;
    CK_INFO info;
    CK_ULONG slotCount = 0;
    SECStatus rv;
    PRBool alreadyLoaded = PR_FALSE;

    if (mod->loaded) return SECSuccess;

    /* intenal modules get loaded from their internal list */
    if (mod->internal) {
	/* internal, statically get the C_GetFunctionList function */
	if (mod->isFIPS) {
	    entry = (CK_C_GetFunctionList) FC_GetFunctionList;
	} else {
	    entry = (CK_C_GetFunctionList) NSC_GetFunctionList;
	}
	if (mod->isModuleDB) {
	    mod->moduleDBFunc = (void *) NSC_ModuleDBFunc;
	}
	if (mod->moduleDBOnly) {
	    mod->loaded = PR_TRUE;
	    return SECSuccess;
	}
    } else {
	/* Not internal, load the DLL and look up C_GetFunctionList */
	if (mod->dllName == NULL) {
	    return SECFailure;
	}

#ifdef notdef
	/* look up the library name */
	full_name = PR_GetLibraryName(PR_GetLibraryPath(),mod->dllName);
	if (full_name == NULL) {
	    return SECFailure;
	}
#else
	full_name = PORT_Strdup(mod->dllName);
#endif

	/* load the library. If this succeeds, then we have to remember to
	 * unload the library if anything goes wrong from here on out...
	 */
	library = PR_LoadLibrary(full_name);
	mod->library = (void *)library;
	PORT_Free(full_name);
	if (library == NULL) {
	    return SECFailure;
	}

	/*
	 * now we need to get the entry point to find the function pointers
	 */
	if (!mod->moduleDBOnly) {
	    entry = (CK_C_GetFunctionList)
			PR_FindSymbol(library, "C_GetFunctionList");
	}
	if (mod->isModuleDB) {
	    mod->moduleDBFunc = (void *)
			PR_FindSymbol(library, "NSS_ReturnModuleSpecData");
	}
	if (mod->moduleDBFunc == NULL) mod->isModuleDB = PR_FALSE;
	if (entry == NULL) {
	    if (mod->isModuleDB) {
		mod->loaded = PR_TRUE;
		mod->moduleDBOnly = PR_TRUE;
		return SECSuccess;
	    }
	    PR_UnloadLibrary(library);
	    return SECFailure;
	}
    }

    /*
     * We need to get the function list
     */
    if ((*entry)((CK_FUNCTION_LIST_PTR *)&mod->functionList) != CKR_OK) 
								goto fail;

#ifdef DEBUG_MODULE
    if (PR_TRUE) {
	modToDBG = PR_GetEnv("NSS_DEBUG_PKCS11_MODULE");
	if (modToDBG && strcmp(mod->commonName, modToDBG) == 0) {
	    mod->functionList = (void *)nss_InsertDeviceLog(
	                           (CK_FUNCTION_LIST_PTR)mod->functionList);
	}
    }
#endif

    mod->isThreadSafe = PR_TRUE;

    /* Now we initialize the module */
    rv = secmod_ModuleInit(mod, &alreadyLoaded);
    if (rv != SECSuccess) {
	goto fail;
    }

    /* check the version number */
    if (PK11_GETTAB(mod)->C_GetInfo(&info) != CKR_OK) goto fail2;
    if (info.cryptokiVersion.major != 2) goto fail2;
    /* all 2.0 are a priori *not* thread safe */
    if (info.cryptokiVersion.minor < 1) {
        if (!loadSingleThreadedModules) {
            PORT_SetError(SEC_ERROR_INCOMPATIBLE_PKCS11);
            goto fail2;
        } else {
            mod->isThreadSafe = PR_FALSE;
        }
    }
    mod->cryptokiVersion = info.cryptokiVersion;

    /* If we don't have a common name, get it from the PKCS 11 module */
    if ((mod->commonName == NULL) || (mod->commonName[0] == 0)) {
	mod->commonName = PK11_MakeString(mod->arena,NULL,
	   (char *)info.libraryDescription, sizeof(info.libraryDescription));
	if (mod->commonName == NULL) goto fail2;
    }
    

    /* initialize the Slots */
    if (PK11_GETTAB(mod)->C_GetSlotList(CK_FALSE, NULL, &slotCount) == CKR_OK) {
	CK_SLOT_ID *slotIDs;
	int i;
	CK_RV crv;

	mod->slots = (PK11SlotInfo **)PORT_ArenaAlloc(mod->arena,
					sizeof(PK11SlotInfo *) * slotCount);
	if (mod->slots == NULL) goto fail2;

	slotIDs = (CK_SLOT_ID *) PORT_Alloc(sizeof(CK_SLOT_ID)*slotCount);
	if (slotIDs == NULL) {
	    goto fail2;
	}  
	crv = PK11_GETTAB(mod)->C_GetSlotList(CK_FALSE, slotIDs, &slotCount);
	if (crv != CKR_OK) {
	    PORT_Free(slotIDs);
	    goto fail2;
	}

	/* Initialize each slot */
	for (i=0; i < (int)slotCount; i++) {
	    mod->slots[i] = PK11_NewSlotInfo(mod);
	    PK11_InitSlot(mod,slotIDs[i],mod->slots[i]);
	    /* look down the slot info table */
	    PK11_LoadSlotList(mod->slots[i],mod->slotInfo,mod->slotInfoCount);
	    SECMOD_SetRootCerts(mod->slots[i],mod);
	}
	mod->slotCount = slotCount;
	mod->slotInfoCount = 0;
	PORT_Free(slotIDs);
    }
    
    mod->loaded = PR_TRUE;
    mod->moduleID = nextModuleID++;
    return SECSuccess;
fail2:
    if (enforceAlreadyInitializedError || (!alreadyLoaded)) {
        PK11_GETTAB(mod)->C_Finalize(NULL);
    }
fail:
    mod->functionList = NULL;
    if (library) PR_UnloadLibrary(library);
    return SECFailure;
}

SECStatus
SECMOD_UnloadModule(SECMODModule *mod) {
    PRLibrary *library;

    if (!mod->loaded) {
	return SECFailure;
    }
    if (finalizeModules) {
        if (!mod->moduleDBOnly) PK11_GETTAB(mod)->C_Finalize(NULL);
    }
    mod->moduleID = 0;
    mod->loaded = PR_FALSE;
    
    /* do we want the semantics to allow unloading the internal library?
     * if not, we should change this to SECFailure and move it above the
     * mod->loaded = PR_FALSE; */
    if (mod->internal) {
	return SECSuccess;
    }

    library = (PRLibrary *)mod->library;
    /* paranoia */
    if (library == NULL) {
	return SECFailure;
    }

    PR_UnloadLibrary(library);
    return SECSuccess;
}

void
nss_DumpModuleLog(void)
{
#ifdef DEBUG_MODULE
    if (modToDBG) {
	print_final_statistics();
    }
#endif
}
