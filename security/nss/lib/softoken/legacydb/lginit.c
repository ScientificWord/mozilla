/*
 * NSS utility functions
 *
 * ***** BEGIN LICENSE BLOCK *****
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
/* $Id$ */

#include "lowkeyi.h"
#include "pcert.h"
#include "keydbi.h"
#include "lgdb.h"
#include "secoid.h"
#include "prenv.h"

typedef struct LGPrivateStr {
    NSSLOWCERTCertDBHandle *certDB;
    NSSLOWKEYDBHandle *keyDB;
    PRLock *dbLock;
    PLHashTable *hashTable;
} LGPrivate;

static char *
lg_certdb_name_cb(void *arg, int dbVersion)
{
    const char *configdir = (const char *)arg;
    const char *dbver;
    char *smpname = NULL;
    char *dbname = NULL;

    switch (dbVersion) {
      case 8:
	dbver = "8";
	break;
      case 7:
	dbver = "7";
	break;
      case 6:
	dbver = "6";
	break;
      case 5:
	dbver = "5";
	break;
      case 4:
      default:
	dbver = "";
	break;
    }

    /* make sure we return something allocated with PORT_ so we have properly
     * matched frees at the end */
    smpname = PR_smprintf(CERT_DB_FMT, configdir, dbver);
    if (smpname) {
	dbname = PORT_Strdup(smpname);
	PR_smprintf_free(smpname);
    }
    return dbname;
}
    
static char *
lg_keydb_name_cb(void *arg, int dbVersion)
{
    const char *configdir = (const char *)arg;
    const char *dbver;
    char *smpname = NULL;
    char *dbname = NULL;
    
    switch (dbVersion) {
      case 4:
	dbver = "4";
	break;
      case 3:
	dbver = "3";
	break;
      case 1:
	dbver = "1";
	break;
      case 2:
      default:
	dbver = "";
	break;
    }

    smpname = PR_smprintf(KEY_DB_FMT, configdir, dbver);
    if (smpname) {
	dbname = PORT_Strdup(smpname);
	PR_smprintf_free(smpname);
    }
    return dbname;
}

const char *
lg_EvaluateConfigDir(const char *configdir,char **appName)
{
    if (PORT_Strncmp(configdir, MULTIACCESS, sizeof(MULTIACCESS)-1) == 0) {
	char *cdir;

	*appName = PORT_Strdup(configdir+sizeof(MULTIACCESS)-1);
	if (*appName == NULL) {
	    return configdir;
	}
	cdir = *appName;
	while (*cdir && *cdir != ':') {
	    cdir++;
	}
	if (*cdir == ':') {
	   *cdir = 0;
	   cdir++;
	}
	configdir = cdir;
    }
    return configdir;
}

static int rdbmapflags(int flags);
static rdbfunc lg_rdbfunc = NULL;
static rdbstatusfunc lg_rdbstatusfunc = NULL;

/* NOTE: SHLIB_SUFFIX is defined on the command line */
#define RDBLIB SHLIB_PREFIX"rdb."SHLIB_SUFFIX

DB * rdbopen(const char *appName, const char *prefix, 
			const char *type, int flags, int *status)
{
    PRLibrary *lib;
    DB *db;
    char *disableUnload = NULL;

    if (lg_rdbfunc) {
	db = (*lg_rdbfunc)(appName,prefix,type,rdbmapflags(flags));
	if (!db && status && lg_rdbstatusfunc) {
	    *status = (*lg_rdbstatusfunc)();
	}
	return db;
    }

    /*
     * try to open the library.
     */
    lib = PR_LoadLibrary(RDBLIB);

    if (!lib) {
	return NULL;
    }

    /* get the entry points */
    lg_rdbstatusfunc = (rdbstatusfunc) PR_FindSymbol(lib,"rdbstatus");
    lg_rdbfunc = (rdbfunc) PR_FindSymbol(lib,"rdbopen");
    if (lg_rdbfunc) {
	db = (*lg_rdbfunc)(appName,prefix,type,rdbmapflags(flags));
	if (!db && status && lg_rdbstatusfunc) {
	    *status = (*lg_rdbstatusfunc)();
	}
	return db;
    }

    /* couldn't find the entry point, unload the library and fail */
    disableUnload = PR_GetEnv("NSS_DISABLE_UNLOAD");
    if (!disableUnload) {
        PR_UnloadLibrary(lib);
    }
    return NULL;
}

/*
 * the following data structures are from rdb.h.
 */
struct RDBStr {
    DB	db;
    int (*xactstart)(DB *db);
    int (*xactdone)(DB *db, PRBool abort);
    int version;
    int (*dbinitcomplete)(DB *db);
};

#define DB_RDB ((DBTYPE) 0xff)
#define RDB_RDONLY	1
#define RDB_RDWR 	2
#define RDB_CREATE      4

static int
rdbmapflags(int flags) {
   switch (flags) {
   case NO_RDONLY:
	return RDB_RDONLY;
   case NO_RDWR:
	return RDB_RDWR;
   case NO_CREATE:
	return RDB_CREATE;
   default:
	break;
   }
   return 0;
}

PRBool
db_IsRDB(DB *db)
{
    return (PRBool) db->type == DB_RDB;
}

int
db_BeginTransaction(DB *db)
{
    struct RDBStr *rdb = (struct RDBStr *)db;
    if (db->type != DB_RDB) {
	return 0;
    }

    return rdb->xactstart(db);
}

int
db_FinishTransaction(DB *db, PRBool abort)
{
    struct RDBStr *rdb = (struct RDBStr *)db;
    if (db->type != DB_RDB) {
	return 0;
    }

    return rdb->xactdone(db, abort);
}

static DB *
lg_getRawDB(SDB *sdb)
{
    NSSLOWCERTCertDBHandle *certDB;
    NSSLOWKEYDBHandle *keyDB;

    certDB = lg_getCertDB(sdb);
    if (certDB) {
	return certDB->permCertDB;
    }
    keyDB = lg_getKeyDB(sdb);
    if (keyDB) {
	return keyDB->db;
    }
    return NULL;
}

CK_RV
lg_Begin(SDB *sdb)
{
    DB *db = lg_getRawDB(sdb);
    int ret;

    if (db == NULL) {
	return CKR_GENERAL_ERROR; /* shouldn't happen */
    }
    ret = db_BeginTransaction(db);
    if (ret != 0) {
	return CKR_GENERAL_ERROR; /* could happen */
    }
    return CKR_OK;
}

CK_RV
lg_Commit(SDB *sdb)
{
    DB *db = lg_getRawDB(sdb);
    int ret;

    if (db == NULL) {
	return CKR_GENERAL_ERROR; /* shouldn't happen */
    }
    ret = db_FinishTransaction(db, PR_FALSE);
    if (ret != 0) {
	return CKR_GENERAL_ERROR; /* could happen */
    }
    return CKR_OK;
}

CK_RV
lg_Abort(SDB *sdb)
{
    DB *db = lg_getRawDB(sdb);
    int ret;

    if (db == NULL) {
	return CKR_GENERAL_ERROR; /* shouldn't happen */
    }
    ret = db_FinishTransaction(db, PR_TRUE);
    if (ret != 0) {
	return CKR_GENERAL_ERROR; /* could happen */
    }
    return CKR_OK;
}

int
db_InitComplete(DB *db)
{
    struct RDBStr *rdb = (struct RDBStr *)db;
    if (db->type != DB_RDB) {
	return 0;
    }
    /* we should have added a version number to the RDBS structure. Since we
     * didn't, we detect that we have and 'extended' structure if the rdbstatus
     * func exists */
    if (!lg_rdbstatusfunc) {
	return 0;
    }

    return rdb->dbinitcomplete(db);
}



SECStatus
db_Copy(DB *dest,DB *src)
{
    int ret;
    DBT key,data;
    ret = (*src->seq)(src, &key, &data, R_FIRST);
    if (ret)  {
	return SECSuccess;
    }

    do {
	(void)(*dest->put)(dest,&key,&data, R_NOOVERWRITE);
    } while ( (*src->seq)(src, &key, &data, R_NEXT) == 0);
    (void)(*dest->sync)(dest,0);

    return SECSuccess;
}


static CK_RV
lg_OpenCertDB(const char * configdir, const char *prefix, PRBool readOnly,
    					    NSSLOWCERTCertDBHandle **certdbPtr)
{
    NSSLOWCERTCertDBHandle *certdb = NULL;
    CK_RV        crv = CKR_NETSCAPE_CERTDB_FAILED;
    SECStatus    rv;
    char * name = NULL;
    char * appName = NULL;

    if (prefix == NULL) {
	prefix = "";
    }

    configdir = lg_EvaluateConfigDir(configdir, &appName);

    name = PR_smprintf("%s" PATH_SEPARATOR "%s",configdir,prefix);
    if (name == NULL) goto loser;

    certdb = (NSSLOWCERTCertDBHandle*)PORT_ZAlloc(sizeof(NSSLOWCERTCertDBHandle));
    if (certdb == NULL) 
    	goto loser;

    certdb->ref = 1;
/* fix when we get the DB in */
    rv = nsslowcert_OpenCertDB(certdb, readOnly, appName, prefix,
				lg_certdb_name_cb, (void *)name, PR_FALSE);
    if (rv == SECSuccess) {
	crv = CKR_OK;
	*certdbPtr = certdb;
	certdb = NULL;
    }
loser: 
    if (certdb) PR_Free(certdb);
    if (name) PR_smprintf_free(name);
    if (appName) PORT_Free(appName);
    return crv;
}

static CK_RV
lg_OpenKeyDB(const char * configdir, const char *prefix, PRBool readOnly,
    						NSSLOWKEYDBHandle **keydbPtr)
{
    NSSLOWKEYDBHandle *keydb;
    char * name = NULL;
    char * appName = NULL;

    if (prefix == NULL) {
	prefix = "";
    }
    configdir = lg_EvaluateConfigDir(configdir, &appName);

    name = PR_smprintf("%s" PATH_SEPARATOR "%s",configdir,prefix);	
    if (name == NULL) 
	return CKR_HOST_MEMORY;
    keydb = nsslowkey_OpenKeyDB(readOnly, appName, prefix, 
					lg_keydb_name_cb, (void *)name);
    PR_smprintf_free(name);
    if (appName) PORT_Free(appName);
    if (keydb == NULL)
	return CKR_NETSCAPE_KEYDB_FAILED;
    *keydbPtr = keydb;

    return CKR_OK;
}

/*
 * Accessors for the private parts of the sdb structure.
 */
void
lg_DBLock(SDB *sdb) 
{
    LGPrivate *lgdb_p = (LGPrivate *)sdb->private;
    PR_Lock(lgdb_p->dbLock);
}

void
lg_DBUnlock(SDB *sdb) 
{
    LGPrivate *lgdb_p = (LGPrivate *)sdb->private;
    PR_Unlock(lgdb_p->dbLock);
}

PLHashTable *
lg_GetHashTable(SDB *sdb) 
{
    LGPrivate *lgdb_p = (LGPrivate *)sdb->private;
    return lgdb_p->hashTable;
}

NSSLOWCERTCertDBHandle *
lg_getCertDB(SDB *sdb)
{
    LGPrivate *lgdb_p = (LGPrivate *)sdb->private;

    return lgdb_p->certDB;
}

NSSLOWKEYDBHandle *
lg_getKeyDB(SDB *sdb)
{
    LGPrivate *lgdb_p = (LGPrivate *)sdb->private;

    return lgdb_p->keyDB;
}

CK_RV
lg_Close(SDB *sdb)
{
    LGPrivate *lgdb_p = (LGPrivate *)sdb->private;
    lg_ClearTokenKeyHashTable(sdb);
    if (lgdb_p) {
    	if (lgdb_p->certDB) {
	    nsslowcert_ClosePermCertDB(lgdb_p->certDB);
	} else if (lgdb_p->keyDB) {
	    nsslowkey_CloseKeyDB(lgdb_p->keyDB);
	}
	if (lgdb_p->dbLock) {
	    PR_DestroyLock(lgdb_p->dbLock);
	}
	if (lgdb_p->hashTable) {
	    PL_HashTableDestroy(lgdb_p->hashTable);
	}
	PORT_Free(lgdb_p);
    }
    PORT_Free(sdb);
    return CKR_OK;
}

static PLHashNumber
lg_HashNumber(const void *key)
{
    return (PLHashNumber) key;
}

PRIntn
lg_CompareValues(const void *v1, const void *v2)
{
    PLHashNumber value1 = (PLHashNumber) v1;
    PLHashNumber value2 = (PLHashNumber) v2;
    return (value1 == value2);
}


/*
 * helper function to wrap a NSSLOWCERTCertDBHandle or a NSSLOWKEYDBHandle
 * with and sdb structure.
 */
CK_RV 
lg_init(SDB **pSdb, int flags, NSSLOWCERTCertDBHandle *certdbPtr,
	 NSSLOWKEYDBHandle *keydbPtr)
{
    SDB *sdb = NULL;
    LGPrivate *lgdb_p = NULL;
    CK_RV error = CKR_HOST_MEMORY;

    *pSdb = NULL;
    sdb = (SDB *) PORT_Alloc(sizeof(SDB));
    if (sdb == NULL) {
	goto loser;
    }
    lgdb_p = (LGPrivate *) PORT_Alloc(sizeof(LGPrivate));
    if (lgdb_p == NULL) {
	goto loser;
    }
    /* invariant fields */
    lgdb_p->certDB = certdbPtr;
    lgdb_p->keyDB = keydbPtr;
    lgdb_p->dbLock = PR_NewLock();
    if (lgdb_p->dbLock == NULL) {
	goto loser;
    }
    lgdb_p->hashTable = PL_NewHashTable(64, lg_HashNumber, lg_CompareValues,
			SECITEM_HashCompare, NULL, 0);
    if (lgdb_p->hashTable == NULL) {
	goto loser;
    }

    sdb->sdb_type = SDB_LEGACY;
    sdb->sdb_flags = flags;
    sdb->private = lgdb_p;
    sdb->sdb_FindObjectsInit = lg_FindObjectsInit;
    sdb->sdb_FindObjects = lg_FindObjects;
    sdb->sdb_FindObjectsFinal = lg_FindObjectsFinal;
    sdb->sdb_GetAttributeValue = lg_GetAttributeValue;
    sdb->sdb_SetAttributeValue = lg_SetAttributeValue;
    sdb->sdb_CreateObject = lg_CreateObject;
    sdb->sdb_DestroyObject = lg_DestroyObject;
    sdb->sdb_GetMetaData = lg_GetMetaData;
    sdb->sdb_PutMetaData = lg_PutMetaData;
    sdb->sdb_Begin = lg_Begin;
    sdb->sdb_Commit = lg_Commit;
    sdb->sdb_Abort = lg_Abort;
    sdb->sdb_Reset = lg_Reset;
    sdb->sdb_Close = lg_Close;


    *pSdb = sdb;
    return CKR_OK;

loser:
    if (sdb) {
	PORT_Free(sdb);
    }
    if (lgdb_p) {
	if (lgdb_p->dbLock) {
	    PR_DestroyLock(lgdb_p->dbLock);
	}
	if (lgdb_p->hashTable) {
	    PL_HashTableDestroy(lgdb_p->hashTable);
	}
	PORT_Free(lgdb_p);
    }
    return error;

}

/*
 * OK there are now lots of options here, lets go through them all:
 *
 * configdir - base directory where all the cert, key, and module datbases live.
 * certPrefix - prefix added to the beginning of the cert database example: "
 * 			"https-server1-"
 * keyPrefix - prefix added to the beginning of the key database example: "
 * 			"https-server1-"
 * secmodName - name of the security module database (usually "secmod.db").
 * readOnly - Boolean: true if the databases are to be openned read only.
 * nocertdb - Don't open the cert DB and key DB's, just initialize the 
 *			Volatile certdb.
 * nomoddb - Don't open the security module DB, just initialize the 
 *			PKCS #11 module.
 * forceOpen - Continue to force initializations even if the databases cannot
 * 			be opened.
 */
CK_RV
legacy_Open(const char *configdir, const char *certPrefix, 
	    const char *keyPrefix, int certVersion, int keyVersion,
	    int flags, SDB **certDB, SDB **keyDB)
{
    CK_RV crv = CKR_OK;
    SECStatus rv;
    PRBool readOnly = (flags == SDB_RDONLY)? PR_TRUE: PR_FALSE;

    rv = SECOID_Init();
    if (SECSuccess != rv) {
        return CKR_DEVICE_ERROR;
    }
    nsslowcert_InitLocks();

    if (keyDB) *keyDB = NULL;
    if (certDB) *certDB = NULL;

    if (certDB) {
	NSSLOWCERTCertDBHandle *certdbPtr;

	crv = lg_OpenCertDB(configdir, certPrefix, readOnly, &certdbPtr);
	if (crv != CKR_OK) {
	    goto loser;
	}
	crv = lg_init(certDB, flags, certdbPtr, NULL);
	if (crv != CKR_OK) {
	    nsslowcert_ClosePermCertDB(certdbPtr);
	    goto loser;
	}
    }
    if (keyDB) {
	NSSLOWKEYDBHandle *keydbPtr;

	crv = lg_OpenKeyDB(configdir, keyPrefix, readOnly, &keydbPtr);
	if (crv != CKR_OK) {
	    goto loser;
	}
	crv = lg_init(keyDB, flags, NULL, keydbPtr);
	if (crv != CKR_OK) {
	    nsslowkey_CloseKeyDB(keydbPtr);
	    goto loser;
	}
	if (certDB && *certDB) {
	    LGPrivate *lgdb_p = (LGPrivate *)(*certDB)->private;
	    lgdb_p->keyDB = keydbPtr;
	}
    }

loser:
    if (crv != CKR_OK) {
	if (keyDB && *keyDB) {
	    lg_Close(*keyDB);
	    *keyDB = NULL;
	}
	if (certDB && *certDB) {
	    lg_Close(*certDB);
	    *certDB = NULL;
	}
    }
    return crv;
}

CK_RV
legacy_Shutdown(void)
{
    nsslowcert_DestroyFreeLists();
    nsslowcert_DestroyGlobalLocks();
    SECOID_Shutdown();
    return CKR_OK;
}
