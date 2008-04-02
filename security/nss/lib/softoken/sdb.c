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
 * The Original Code is Red Hat, Inc.
 *
 * The Initial Developer of the Original Code is
 * Red Hat, Inc.
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Robert Relyea (rrelyea@redhat.com)
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
 * This file implements PKCS 11 on top of our existing security modules
 *
 * For more information about PKCS 11 See PKCS 11 Token Inteface Standard.
 *   This implementation has two slots:
 *	slot 1 is our generic crypto support. It does not require login.
 *   It supports Public Key ops, and all they bulk ciphers and hashes. 
 *   It can also support Private Key ops for imported Private keys. It does 
 *   not have any token storage.
 *	slot 2 is our private key support. It requires a login before use. It
 *   can store Private Keys and Certs as token objects. Currently only private
 *   keys and their associated Certificates are saved on the token.
 *
 *   In this implementation, session objects are only visible to the session
 *   that created or generated them.
 */

#include "sdb.h"
#include "pkcs11t.h"
#include "seccomon.h"
#include <sqlite3.h>
#include "prthread.h"
#include "prio.h"
#include "stdio.h"
#include "secport.h"
#include "prmon.h"
#include "prenv.h"

#ifdef SQLITE_UNSAFE_THREADS
#include "prlock.h"
/*
 * SQLite can be compiled to be thread safe or not.
 * turn on SQLITE_UNSAFE_THREADS if the OS does not support
 * a thread safe version of sqlite.
 */
static PRLock *sqlite_lock = NULL;

#define LOCK_SQLITE()  PR_Lock(sqlite_lock);
#define UNLOCK_SQLITE()  PR_Unlock(sqlite_lock);
#else
#define LOCK_SQLITE()  
#define UNLOCK_SQLITE()  
#endif

typedef enum {
	SDB_CERT = 1,
	SDB_KEY = 2
} sdbDataType;

/*
 * defines controlling how long we wait to acquire locks.
 *
 * SDB_SQLITE_BUSY_TIMEOUT specifies how long (in milliseconds)
 *  sqlite will wait on lock. If that timeout expires, sqlite will
 *  return SQLITE_BUSY.
 * SDB_BUSY_RETRY_TIME specifies how many seconds the sdb_ code waits
 *  after receiving a busy before retrying.
 * SDB_MAX_BUSY_RETRIES specifies how many times the sdb_ will retry on
 *  a busy condition.
 *
 * SDB_SQLITE_BUSY_TIMEOUT affects all opertions, both manual 
 *   (prepare/step/reset/finalize) and automatic (sqlite3_exec()).
 * SDB_BUSY_RETRY_TIME and SDB_MAX_BUSY_RETRIES only affect manual operations
 * 
 * total wait time for automatic operations: 
 *   1 second (SDB_SQLITE_BUSY_TIMEOUT/1000).
 * total wait time for manual operations: 
 *   (1 second + 5 seconds) * 10 = 60 seconds.
 * (SDB_SQLITE_BUSY_TIMEOUT/1000 + SDB_BUSY_RETRY_TIME)*SDB_MAX_BUSY_RETRIES
 */
#define SDB_SQLITE_BUSY_TIMEOUT 1000 /* milliseconds */
#define SDB_BUSY_RETRY_TIME        5 /* seconds */
#define SDB_MAX_BUSY_RETRIES      10

/*
 * Note on use of sqlReadDB: Only one thread at a time may have an actual
 * operation going on given sqlite3 * database. An operation is defined as
 * the time from a sqlite3_prepare() until the sqlite3_finalize().
 * Multiple sqlite3 * databases can be open and have simultaneous operations
 * going. We use the sqlXactDB for all write operations. This database
 * is only opened when we first create a transaction and closed when the
 * transaction is complete. sqlReadDB is open when we first opened the database
 * and is used for all read operation. It's use is protected by a monitor. This
 * is because an operation can span the use of FindObjectsInit() through the
 * call to FindObjectsFinal(). In the intermediate time it is possible to call
 * other operations like NSC_GetAttributeValue */

struct SDBPrivateStr {
    char *sqlDBName;		/* invariant, path to this database */
    sqlite3 *sqlXactDB;		/* access protected by dbMon, use protected
                                 * by the transaction. Current transaction db*/
    PRThread *sqlXactThread;	/* protected by dbMon,
			         * current transaction thread */
    sqlite3 *sqlReadDB;		/* use protected by dbMon, value invariant */
    PRIntervalTime lastUpdateTime;  /* last time the cache was updated */
    PRIntervalTime updateInterval;  /* how long the cache can go before it 
                                     * must be updated again */
    sdbDataType type;		/* invariant, database type */
    char *table;	        /* invariant, SQL table which contains the db */
    char *cacheTable;	        /* invariant, SQL table cache of db */
    PRMonitor *dbMon;		/* invariant, monitor to protect 
				 * sqlXact* fields, and use of the sqlReadDB */
};

typedef struct SDBPrivateStr SDBPrivate;

/*
 * known attributes
 */
static const CK_ATTRIBUTE_TYPE known_attributes[] = {
    CKA_CLASS, CKA_TOKEN, CKA_PRIVATE, CKA_LABEL, CKA_APPLICATION,
    CKA_VALUE, CKA_OBJECT_ID, CKA_CERTIFICATE_TYPE, CKA_ISSUER,
    CKA_SERIAL_NUMBER, CKA_AC_ISSUER, CKA_OWNER, CKA_ATTR_TYPES, CKA_TRUSTED,
    CKA_CERTIFICATE_CATEGORY, CKA_JAVA_MIDP_SECURITY_DOMAIN, CKA_URL,
    CKA_HASH_OF_SUBJECT_PUBLIC_KEY, CKA_HASH_OF_ISSUER_PUBLIC_KEY,
    CKA_CHECK_VALUE, CKA_KEY_TYPE, CKA_SUBJECT, CKA_ID, CKA_SENSITIVE,
    CKA_ENCRYPT, CKA_DECRYPT, CKA_WRAP, CKA_UNWRAP, CKA_SIGN, CKA_SIGN_RECOVER,
    CKA_VERIFY, CKA_VERIFY_RECOVER, CKA_DERIVE, CKA_START_DATE, CKA_END_DATE,
    CKA_MODULUS, CKA_MODULUS_BITS, CKA_PUBLIC_EXPONENT, CKA_PRIVATE_EXPONENT,
    CKA_PRIME_1, CKA_PRIME_2, CKA_EXPONENT_1, CKA_EXPONENT_2, CKA_COEFFICIENT,
    CKA_PRIME, CKA_SUBPRIME, CKA_BASE, CKA_PRIME_BITS, 
    CKA_SUB_PRIME_BITS, CKA_VALUE_BITS, CKA_VALUE_LEN, CKA_EXTRACTABLE,
    CKA_LOCAL, CKA_NEVER_EXTRACTABLE, CKA_ALWAYS_SENSITIVE,
    CKA_KEY_GEN_MECHANISM, CKA_MODIFIABLE, CKA_EC_PARAMS,
    CKA_EC_POINT, CKA_SECONDARY_AUTH, CKA_AUTH_PIN_FLAGS,
    CKA_ALWAYS_AUTHENTICATE, CKA_WRAP_WITH_TRUSTED, CKA_WRAP_TEMPLATE,
    CKA_UNWRAP_TEMPLATE, CKA_HW_FEATURE_TYPE, CKA_RESET_ON_INIT,
    CKA_HAS_RESET, CKA_PIXEL_X, CKA_PIXEL_Y, CKA_RESOLUTION, CKA_CHAR_ROWS,
    CKA_CHAR_COLUMNS, CKA_COLOR, CKA_BITS_PER_PIXEL, CKA_CHAR_SETS,
    CKA_ENCODING_METHODS, CKA_MIME_TYPES, CKA_MECHANISM_TYPE,
    CKA_REQUIRED_CMS_ATTRIBUTES, CKA_DEFAULT_CMS_ATTRIBUTES,
    CKA_SUPPORTED_CMS_ATTRIBUTES, CKA_NETSCAPE_URL, CKA_NETSCAPE_EMAIL,
    CKA_NETSCAPE_SMIME_INFO, CKA_NETSCAPE_SMIME_TIMESTAMP,
    CKA_NETSCAPE_PKCS8_SALT, CKA_NETSCAPE_PASSWORD_CHECK, CKA_NETSCAPE_EXPIRES,
    CKA_NETSCAPE_KRL, CKA_NETSCAPE_PQG_COUNTER, CKA_NETSCAPE_PQG_SEED,
    CKA_NETSCAPE_PQG_H, CKA_NETSCAPE_PQG_SEED_BITS, CKA_NETSCAPE_MODULE_SPEC,
    CKA_TRUST_DIGITAL_SIGNATURE, CKA_TRUST_NON_REPUDIATION,
    CKA_TRUST_KEY_ENCIPHERMENT, CKA_TRUST_DATA_ENCIPHERMENT,
    CKA_TRUST_KEY_AGREEMENT, CKA_TRUST_KEY_CERT_SIGN, CKA_TRUST_CRL_SIGN,
    CKA_TRUST_SERVER_AUTH, CKA_TRUST_CLIENT_AUTH, CKA_TRUST_CODE_SIGNING,
    CKA_TRUST_EMAIL_PROTECTION, CKA_TRUST_IPSEC_END_SYSTEM,
    CKA_TRUST_IPSEC_TUNNEL, CKA_TRUST_IPSEC_USER, CKA_TRUST_TIME_STAMPING,
    CKA_TRUST_STEP_UP_APPROVED, CKA_CERT_SHA1_HASH, CKA_CERT_MD5_HASH,
    CKA_NETSCAPE_DB, CKA_NETSCAPE_TRUST, CKA_NSS_OVERRIDE_EXTENSIONS
};

static int known_attributes_size= sizeof(known_attributes)/
			   sizeof(known_attributes[0]);

/* Magic for an explicit NULL. NOTE: ideally this should be
 * out of band data. Since it's not completely out of band, pick
 * a value that has no meaning to any existing PKCS #11 attributes.
 * This value is 1) not a valid string (imbedded '\0'). 2) not a U_LONG
 * or a normal key (too short). 3) not a bool (too long). 4) not an RSA
 * public exponent (too many bits).
 */
const unsigned char SQLITE_EXPLICIT_NULL[] = { 0xa5, 0x0, 0x5a };
#define SQLITE_EXPLICIT_NULL_LEN 3

/*
 * determine when we've completed our tasks
 */
static int 
sdb_done(int err, int *count)
{
    /* allow as many rows as the database wants to give */
    if (err == SQLITE_ROW) {
	*count = 0;
	return 0;
    }
    if (err != SQLITE_BUSY) {
	return 1;
    }
    /* err == SQLITE_BUSY, Dont' retry forever in this case */
    if (++(*count) >= SDB_MAX_BUSY_RETRIES) {
	return 1;
    }
    return 0;
}

/*
 * 
 * strdup limited to 'n' bytes. (Note: len of file is assumed to be >= len)
 *
 * We don't have a PORT_ version of this function,
 * I suspect it's only normally available in glib,
 */
static char *
sdb_strndup(const char *file, int len)
{
   char *result = PORT_Alloc(len+1);

   if (result == NULL) {
	return result;
   }

   PORT_Memcpy(result, file, len);
   result[len] = 0;
   return result;
}

/*
 * call back from  sqlite3_exec("Pragma database_list"). Looks for the
 * temp directory, then return the file the temp directory is stored
 * at. */
static int 
sdb_getTempDirCallback(void *arg, int columnCount, char **cval, char **cname)
{
    int i;
    int found = 0;
    char *file = NULL;
    char *end, *dir;

    /* we've already found the temp directory, don't look at any more records*/
    if (*(char **)arg) {
	return SQLITE_OK;
    }

    /* look at the columns to see if this record is the temp database,
     * and does it say where it is stored */
    for (i=0; i < columnCount; i++) {
	if (PORT_Strcmp(cname[i],"name") == 0) {
	    if (PORT_Strcmp(cval[i], "temp") == 0) {
		found++;
		continue;
	    }
	}
	if (PORT_Strcmp(cname[i],"file") == 0) {
	    if (cval[i] && (*cval[i] != 0)) {
		file = cval[i];
	    }
	}
    }

    /* if we couldn't find it, ask for the next record */
    if (!found || !file) {
	return SQLITE_OK;
    }

    /* drop of the database file name and just return the directory */
    end = PORT_Strrchr(file, '/');
    if (!end) {
	return SQLITE_OK;
    }
    dir = sdb_strndup(file, end-file);

    *(char **)arg = dir;
    return SQLITE_OK;
}

/*
 * find out where sqlite stores the temp tables. We do this by creating
 * a temp table, then looking for the database name that sqlite3 creates.
 */
static char *
sdb_getTempDir(sqlite3 *sqlDB)
{
    char *tempDir = NULL;
    int sqlerr;

    /* create a temporary table */
    sqlerr = sqlite3_exec(sqlDB, "CREATE TEMPORARY TABLE myTemp (id)",
			  NULL, 0, NULL);
    if (sqlerr != SQLITE_OK) {
	return NULL;
    }
    /* look for through the database list for the temp directory */
    sqlerr = sqlite3_exec(sqlDB, "PRAGMA database_list", 
		sdb_getTempDirCallback, &tempDir, NULL);

    /* drop the temp table we created */
    sqlite3_exec(sqlDB, "DROP TABLE myTemp", NULL, 0, NULL);

    if (sqlerr != SQLITE_OK) {
	return NULL;
    }
    return tempDir;
}


/*
 * Map SQL_LITE errors to PKCS #11 errors as best we can.
 */
static int 
sdb_mapSQLError(sdbDataType type, int sqlerr)
{
    switch (sqlerr) {
    /* good matches */
    case SQLITE_OK:
    case SQLITE_DONE:
	return CKR_OK;
    case SQLITE_NOMEM:
	return CKR_HOST_MEMORY;
    case SQLITE_READONLY:
	return CKR_TOKEN_WRITE_PROTECTED;
    /* close matches */
    case SQLITE_AUTH:
    case SQLITE_PERM:
	/*return CKR_USER_NOT_LOGGED_IN; */
    case SQLITE_CANTOPEN:
    case SQLITE_NOTFOUND:
	/* NSS distiguishes between failure to open the cert and the key db */
	return type == SDB_CERT ? 
		CKR_NETSCAPE_CERTDB_FAILED : CKR_NETSCAPE_KEYDB_FAILED;
    case SQLITE_IOERR:
	return CKR_DEVICE_ERROR;
    default:
	break;
    }
    return CKR_GENERAL_ERROR;
}


/*
 * build up database name from a directory, prefix, name, version and flags.
 */
static char *sdb_BuildFileName(const char * directory, 
			const char *prefix, const char *type, 
			int version, int flags)
{
    char *dbname = NULL;
    /* build the full dbname */
    dbname = sqlite3_mprintf("%s/%s%s%d.db",directory, prefix, type, version);
    return dbname;
}


/*
 * find out how expensive the access system call is for non-existant files
 * in the given directory.
 */
PRIntervalTime
sdb_measureAccess(const char *directory)
{
    char *temp;
    PRIntervalTime time;
    PRIntervalTime delta;
    PRIntervalTime next;
    int i;

    /* no directory, just return one */
    if (directory == NULL) {
	return 1;
    }

    /* measure 200 iterations so we have some resolution in the timer
     * to work with. This code tries to open 200 unique files so that
     * any caching code is defeated and we can get a reasonable idea about
     * how well the underlying file system works */
    time =  PR_IntervalNow();
    for (i=0; i < 200; i++) { 
        temp  = sdb_BuildFileName(directory,"","._dOeSnotExist_", time+i, 0);
	PR_Access(temp,PR_ACCESS_EXISTS);
        sqlite3_free(temp);
    } 
    next = PR_IntervalNow();
    delta = next - time;

    /* always return 1 or greater */
    if (delta == 0) delta = 1;
    return delta;
}

/*
 * some file sytems are very slow to run sqlite3 on, particularly if the
 * access count is pretty high. On these filesystems is faster to create
 * a temporary database on the local filesystem and access that. This
 * code uses a temporary table to create that cache. Temp tables are
 * automatically cleared when the database handle it was created on
 * Is freed.
 */
static const char DROP_CACHE_CMD[] = "DROP TABLE %s";
static const char CREATE_CACHE_CMD[] =
  "CREATE TEMPORARY TABLE %s AS SELECT * FROM %s";
static const char CREATE_ISSUER_INDEX_CMD[] = 
  "CREATE INDEX issuer ON %s (a81)";
static const char CREATE_SUBJECT_INDEX_CMD[] = 
  "CREATE INDEX subject ON %s (a101)";
static const char CREATE_LABEL_INDEX_CMD[] = "CREATE INDEX label ON %s (a3)";
static const char CREATE_ID_INDEX_CMD[] = "CREATE INDEX ckaid ON %s (a102)";

static CK_RV
sdb_buildCache(sqlite3 *sqlDB, sdbDataType type, 
		const char *cacheTable, const char *table)
{
    char *newStr;
    int sqlerr = SQLITE_OK;

    newStr = sqlite3_mprintf(CREATE_CACHE_CMD, cacheTable, table);
    if (newStr == NULL) {
	return CKR_HOST_MEMORY;
    }
    sqlerr = sqlite3_exec(sqlDB, newStr, NULL, 0, NULL);
    sqlite3_free(newStr);
    if (sqlerr != SQLITE_OK) {
	return sdb_mapSQLError(type, sqlerr); 
    }
    /* failure to create the indexes is not an issue */
    newStr = sqlite3_mprintf(CREATE_ISSUER_INDEX_CMD, cacheTable);
    if (newStr == NULL) {
	return CKR_OK;
    }
    sqlerr = sqlite3_exec(sqlDB, newStr, NULL, 0, NULL);
    sqlite3_free(newStr);
    newStr = sqlite3_mprintf(CREATE_SUBJECT_INDEX_CMD, cacheTable);
    if (newStr == NULL) {
	return CKR_OK;
    }
    sqlerr = sqlite3_exec(sqlDB, newStr, NULL, 0, NULL);
    sqlite3_free(newStr);
    newStr = sqlite3_mprintf(CREATE_LABEL_INDEX_CMD, cacheTable);
    if (newStr == NULL) {
	return CKR_OK;
    }
    sqlerr = sqlite3_exec(sqlDB, newStr, NULL, 0, NULL);
    sqlite3_free(newStr);
    newStr = sqlite3_mprintf(CREATE_ID_INDEX_CMD, cacheTable);
    if (newStr == NULL) {
	return CKR_OK;
    }
    sqlerr = sqlite3_exec(sqlDB, newStr, NULL, 0, NULL);
    sqlite3_free(newStr);
    return CKR_OK;
}

/*
 * update the cache and the data records describing it.
 *  The cache is updated by dropping the temp database and recreating it.
 */
static CK_RV
sdb_updateCache(SDBPrivate *sdb_p)
{
    int sqlerr = SQLITE_OK;
    CK_RV error = CKR_OK;
    char *newStr;

    /* drop the old table */
    newStr = sqlite3_mprintf(DROP_CACHE_CMD, sdb_p->cacheTable);
    if (newStr == NULL) {
	return CKR_HOST_MEMORY;
    }
    sqlerr = sqlite3_exec(sdb_p->sqlReadDB, newStr, NULL, 0, NULL);
    sqlite3_free(newStr);
    if ((sqlerr != SQLITE_OK) && (sqlerr != SQLITE_ERROR )) {
        /* something went wrong with the drop, don't try to refresh...
         * NOTE: SQLITE_ERROR is returned if the table doesn't exist. In
         * that case, we just continue on and try to reload it */
	return sdb_mapSQLError(sdb_p->type, sqlerr); 
    }
	

    /* set up the new table */
    error = sdb_buildCache(sdb_p->sqlReadDB,sdb_p->type,
				sdb_p->cacheTable,sdb_p->table );
    if (error == CKR_OK) {
	/* we have a new cache! */
	sdb_p->lastUpdateTime = PR_IntervalNow();
    }
    return error;
}

/*
 *  The sharing of sqlite3 handles across threads is tricky. Older versions
 *  couldn't at all, but newer ones can under strict conditions. Basically
 *  no 2 threads can use the same handle while another thread has an open
 *  stmt running. Once the sqlite3_stmt is finalized, another thread can then
 *  use the database handle.
 *
 *  We use monitors to protect against trying to use a database before
 *  it's sqlite3_stmt is finalized. This is preferable to the opening and
 *  closing the database each operation because there is significant overhead
 *  in the open and close. Also continually opening and closing the database
 *  defeats the cache code as the cache table is lost on close (thus
 *  requiring us to have to reinitialize the cache every operation).
 * 
 *  An execption to the shared handle is transations. All writes happen
 *  through a transaction. When we are in  a transaction, we must use the 
 *  same database pointer for that entire transation. In this case we save 
 *  the transaction database and use it for all accesses on the transaction 
 *  thread. Other threads use the common database.  
 *
 *  There can only be once active transaction on the database at a time.
 *
 *  sdb_openDBLocal() provides us with a valid database handle for whatever
 *  state we are in (reading or in a transaction), and acquires any locks
 *  appropriate to that state. It also decides when it's time to refresh
 *  the cache before we start an operation. Any database handle returned
 *  just eventually be closed with sdb_closeDBLocal().
 *
 *  The table returned either points to the database's physical table, or
 *  to the cached shadow. Tranactions always return the physical table
 *  and read operations return either the physical table or the cache
 *  depending on whether or not the cache exists.
 */
static CK_RV 
sdb_openDBLocal(SDBPrivate *sdb_p, sqlite3 **sqlDB, const char **table)
{
    *sqlDB = NULL;

    PR_EnterMonitor(sdb_p->dbMon);

    if (table) {
	*table = sdb_p->table;
    }

    /* We're in a transaction, use the transaction DB */
    if ((sdb_p->sqlXactDB) && (sdb_p->sqlXactThread == PR_GetCurrentThread())) {
	*sqlDB =sdb_p->sqlXactDB;
	/* only one thread can get here, safe to unlock */
        PR_ExitMonitor(sdb_p->dbMon);
	return CKR_OK;
    }

    /*
     * if we are just reading from the table, we may have the table
     * cached in a temporary table (especially if it's on a shared FS).
     * In that case we want to see updates to the table, the the granularity
     * is on order of human scale, not computer scale.
     */
    if (table && sdb_p->cacheTable) {
	PRIntervalTime now = PR_IntervalNow();
	if ((now - sdb_p->lastUpdateTime) > sdb_p->updateInterval) {
	       sdb_updateCache(sdb_p);
        }
	*table = sdb_p->cacheTable;
    }

    *sqlDB = sdb_p->sqlReadDB;

    /* leave holding the lock. only one thread can actually use a given
     * database connection at once */
	
    return CKR_OK;
}

/* closing the local database currenly means unlocking the monitor */
static CK_RV 
sdb_closeDBLocal(SDBPrivate *sdb_p, sqlite3 *sqlDB) 
{
   if (sdb_p->sqlXactDB != sqlDB) {
	/* if we weren't in a transaction, we got a lock */
        PR_ExitMonitor(sdb_p->dbMon);
   }
   return CKR_OK;
}


/*
 * wrapper to sqlite3_open which also sets the busy_timeout
 */
static int
sdb_openDB(const char *name, sqlite3 **sqlDB, int flags)
{
    int sqlerr;
    /*
     * in sqlite3 3.5.0, there is a new open call that allows us
     * to specify read only. Most new OS's are still on 3.3.x (including
     * NSS's internal version and the version shipped with Firefox).
     */
    *sqlDB = NULL;
    sqlerr = sqlite3_open(name, sqlDB);
    if (sqlerr != SQLITE_OK) {
	return sqlerr;
    }

    sqlerr = sqlite3_busy_timeout(*sqlDB, SDB_SQLITE_BUSY_TIMEOUT);
    if (sqlerr != SQLITE_OK) {
	sqlite3_close(*sqlDB);
	*sqlDB = NULL;
	return sqlerr;
    }
    return SQLITE_OK;
}

/* Sigh, if we created a new table since we opened the database,
 * the database handle will not see the new table, we need to close this
 * database and reopen it. Caller must be in a transaction or holding
 * the dbMon. sqlDB is changed on success. */
static int 
sdb_reopenDBLocal(SDBPrivate *sdb_p, sqlite3 **sqlDB) {
    sqlite3 *newDB;
    int sqlerr;

    /* open a new database */
    sqlerr = sdb_openDB(sdb_p->sqlDBName, &newDB, SDB_RDONLY);
    if (sqlerr != SQLITE_OK) {
	return sqlerr;
    }

    /* if we are in a transaction, we may not be holding the monitor.
     * grab it before we update the transaction database. This is
     * safe since are using monitors. */
    PR_EnterMonitor(sdb_p->dbMon);
    /* update our view of the database */
    if (sdb_p->sqlReadDB == *sqlDB) {
	sdb_p->sqlReadDB = newDB;
    } else if (sdb_p->sqlXactDB == *sqlDB) {
	sdb_p->sqlXactDB = newDB;
    }
    PR_ExitMonitor(sdb_p->dbMon);

    /* close the old one */
    sqlite3_close(*sqlDB);

    *sqlDB = newDB;
    return SQLITE_OK;
}

struct SDBFindStr {
    sqlite3 *sqlDB;
    sqlite3_stmt *findstmt;
};


static const char FIND_OBJECTS_CMD[] =  "SELECT ALL * FROM %s WHERE %s;";
static const char FIND_OBJECTS_ALL_CMD[] = "SELECT ALL * FROM %s;";
CK_RV
sdb_FindObjectsInit(SDB *sdb, const CK_ATTRIBUTE *template, CK_ULONG count, 
				SDBFind **find)
{
    SDBPrivate *sdb_p = sdb->private;
    sqlite3  *sqlDB = NULL;
    const char *table;
    char *newStr, *findStr = NULL;
    sqlite3_stmt *findstmt = NULL;
    char *join="";
    int sqlerr = SQLITE_OK;
    CK_RV error = CKR_OK;
    int i;

    LOCK_SQLITE()
    *find = NULL;
    error = sdb_openDBLocal(sdb_p, &sqlDB, &table);
    if (error != CKR_OK) {
	goto loser;
    }

    findStr = sqlite3_mprintf("");
    for (i=0; findStr && i < count; i++) {
	newStr = sqlite3_mprintf("%s%sa%x=$DATA%d", findStr, join,
				template[i].type, i);
        join=" AND ";
	sqlite3_free(findStr);
	findStr = newStr;
    }

    if (findStr == NULL) {
	error = CKR_HOST_MEMORY;
	goto loser;
    }

    if (count == 0) {
	newStr = sqlite3_mprintf(FIND_OBJECTS_ALL_CMD, table);
    } else {
	newStr = sqlite3_mprintf(FIND_OBJECTS_CMD, table, findStr);
    }
    sqlite3_free(findStr);
    if (newStr == NULL) {
	error = CKR_HOST_MEMORY;
	goto loser;
    }
    sqlerr = sqlite3_prepare_v2(sqlDB, newStr, -1, &findstmt, NULL);
    sqlite3_free(newStr);
    for (i=0; sqlerr == SQLITE_OK && i < count; i++) {
	sqlerr = sqlite3_bind_blob(findstmt, i+1, template[i].pValue, 
				template[i].ulValueLen, SQLITE_TRANSIENT);
    }
    if (sqlerr == SQLITE_OK) {
	*find = PORT_New(SDBFind);
	if (*find == NULL) {
	    error = CKR_HOST_MEMORY;
	    goto loser;
	}
	(*find)->findstmt = findstmt;
	(*find)->sqlDB = sqlDB;
	UNLOCK_SQLITE()  
	return CKR_OK;
    } 
    error = sdb_mapSQLError(sdb_p->type, sqlerr);

loser: 
    if (findstmt) {
	sqlite3_finalize(findstmt);
    }
    if (sqlDB) {
	sdb_closeDBLocal(sdb_p, sqlDB) ;
    }
    UNLOCK_SQLITE()  
    return error;
}


CK_RV
sdb_FindObjects(SDB *sdb, SDBFind *sdbFind, CK_OBJECT_HANDLE *object, 
		CK_ULONG arraySize, CK_ULONG *count)
{
    SDBPrivate *sdb_p = sdb->private;
    sqlite3_stmt *stmt = sdbFind->findstmt;
    int sqlerr = SQLITE_OK;
    int retry = 0;

    *count = 0;

    if (arraySize == 0) {
	return CKR_OK;
    }
    LOCK_SQLITE()  

    do {
	sqlerr = sqlite3_step(stmt);
	if (sqlerr == SQLITE_BUSY) {
	    PR_Sleep(SDB_BUSY_RETRY_TIME);
	}
	if (sqlerr == SQLITE_ROW) {
	    /* only care about the id */
	    *object++= sqlite3_column_int(stmt, 0);
	    arraySize--;
	    (*count)++;
	}
    } while (!sdb_done(sqlerr,&retry) && (arraySize > 0));

    /* we only have some of the objects, there is probably more,
     * set the sqlerr to an OK value so we return CKR_OK */
    if (sqlerr == SQLITE_ROW && arraySize == 0) {
	sqlerr = SQLITE_DONE;
    }
    UNLOCK_SQLITE()  

    return sdb_mapSQLError(sdb_p->type, sqlerr);
}

CK_RV
sdb_FindObjectsFinal(SDB *sdb, SDBFind *sdbFind)
{
    SDBPrivate *sdb_p = sdb->private;
    sqlite3_stmt *stmt = sdbFind->findstmt;
    sqlite3 *sqlDB = sdbFind->sqlDB;
    int sqlerr = SQLITE_OK;

    LOCK_SQLITE()  
    if (stmt) {
	sqlite3_reset(stmt);
	sqlerr = sqlite3_finalize(stmt);
    }
    if (sqlDB) {
	sdb_closeDBLocal(sdb_p, sqlDB) ;
    }
    PORT_Free(sdbFind);

    UNLOCK_SQLITE()  
    return sdb_mapSQLError(sdb_p->type, sqlerr);
}

static const char GET_ATTRIBUTE_CMD[] = "SELECT ALL %s FROM %s WHERE id=$ID;";
CK_RV
sdb_GetAttributeValueNoLock(SDB *sdb, CK_OBJECT_HANDLE object_id, 
				CK_ATTRIBUTE *template, CK_ULONG count)
{
    SDBPrivate *sdb_p = sdb->private;
    sqlite3  *sqlDB = NULL;
    sqlite3_stmt *stmt = NULL;
    char *getStr = NULL;
    char *newStr = NULL;
    const char *table = NULL;
    int sqlerr = SQLITE_OK;
    CK_RV error = CKR_OK;
    int found = 0;
    int retry = 0;
    int i;


    /* open a new db if necessary */
    error = sdb_openDBLocal(sdb_p, &sqlDB, &table);
    if (error != CKR_OK) {
	goto loser;
    }

    getStr = sqlite3_mprintf("");
    for (i=0; getStr && i < count; i++) {
	if (i==0) {
	    newStr = sqlite3_mprintf("a%x", template[i].type);
	} else {
	    newStr = sqlite3_mprintf("%s, a%x", getStr, template[i].type);
	}
	sqlite3_free(getStr);
	getStr = newStr;
    }

    if (getStr == NULL) {
	error = CKR_HOST_MEMORY;
	goto loser;
    }

    newStr = sqlite3_mprintf(GET_ATTRIBUTE_CMD, getStr, table);
    sqlite3_free(getStr);
    getStr = NULL;
    if (newStr == NULL) {
	error = CKR_HOST_MEMORY;
	goto loser;
    }

    sqlerr = sqlite3_prepare_v2(sqlDB, newStr, -1, &stmt, NULL);
    if (sqlerr != SQLITE_OK) { goto loser; }
    sqlerr = sqlite3_bind_int(stmt, 1, object_id);
    if (sqlerr != SQLITE_OK) { goto loser; }
    do {
	sqlerr = sqlite3_step(stmt);
	if (sqlerr == SQLITE_BUSY) {
	    PR_Sleep(SDB_BUSY_RETRY_TIME);
	}
	if (sqlerr == SQLITE_ROW) {
	    for (i=0; i < count; i++) {
		int column = i;
	    	int blobSize;
	    	const char *blobData;

	    	blobSize = sqlite3_column_bytes(stmt, column);
		blobData = sqlite3_column_blob(stmt, column);
		if (blobData == NULL) {
		    template[i].ulValueLen = -1;
		    error = CKR_ATTRIBUTE_TYPE_INVALID; 
		    continue;
		}
		/* If the blob equals our explicit NULL value, then the 
		 * attribute is a NULL. */
		if ((blobSize == SQLITE_EXPLICIT_NULL_LEN) &&
		   	(PORT_Memcmp(blobData, SQLITE_EXPLICIT_NULL, 
			      SQLITE_EXPLICIT_NULL_LEN) == 0)) {
		    blobSize = 0;
		}
		if (template[i].pValue) {
		    if (template[i].ulValueLen < blobSize) {
			template[i].ulValueLen = -1;
		    	error = CKR_BUFFER_TOO_SMALL;
			continue;
		    }
	    	    PORT_Memcpy(template[i].pValue, blobData, blobSize);
		}
		template[i].ulValueLen = blobSize;
	    }
	    found = 1;
	}
    } while (!sdb_done(sqlerr,&retry));

loser:
    /* fix up the error if necessary */
    if (error == CKR_OK) {
	error = sdb_mapSQLError(sdb_p->type, sqlerr);
	if (!found && error == CKR_OK) {
	    error = CKR_OBJECT_HANDLE_INVALID;
	}
    }
    if (newStr) {
	sqlite3_free(newStr);
    }

    if (stmt) {
	sqlite3_reset(stmt);
	sqlite3_finalize(stmt);
    }

    /* if we had to open a new database, free it now */
    if (sqlDB) {
	sdb_closeDBLocal(sdb_p, sqlDB) ;
    }
    return error;
}

CK_RV
sdb_GetAttributeValue(SDB *sdb, CK_OBJECT_HANDLE object_id, 
				CK_ATTRIBUTE *template, CK_ULONG count)
{
    CK_RV crv;

    if (count == 0) {
	return CKR_OK;
    }

    LOCK_SQLITE()  
    crv = sdb_GetAttributeValueNoLock(sdb, object_id, template, count);
    UNLOCK_SQLITE()  
    return crv;
}
   
static const char SET_ATTRIBUTE_CMD[] = "UPDATE %s SET %s WHERE id=$ID;";
CK_RV
sdb_SetAttributeValue(SDB *sdb, CK_OBJECT_HANDLE object_id, 
			const CK_ATTRIBUTE *template, CK_ULONG count)
{
    SDBPrivate *sdb_p = sdb->private;
    sqlite3  *sqlDB = NULL;
    sqlite3_stmt *stmt = NULL;
    char *setStr = NULL;
    char *newStr = NULL;
    int sqlerr = SQLITE_OK;
    int retry = 0;
    CK_RV error = CKR_OK;
    int i;

    if ((sdb->sdb_flags & SDB_RDONLY) != 0) {
	return CKR_TOKEN_WRITE_PROTECTED;
    }

    if (count == 0) {
	return CKR_OK;
    }

    LOCK_SQLITE()  
    setStr = sqlite3_mprintf("");
    for (i=0; setStr && i < count; i++) {
	if (i==0) {
	    sqlite3_free(setStr);
   	    setStr = sqlite3_mprintf("a%x=$VALUE%d", 
				template[i].type, i);
	    continue;
	}
	newStr = sqlite3_mprintf("%s,a%x=$VALUE%d", setStr, 
				template[i].type, i);
	sqlite3_free(setStr);
	setStr = newStr;
    }
    newStr = NULL;

    if (setStr == NULL) {
	return CKR_HOST_MEMORY;
    }
    newStr =  sqlite3_mprintf(SET_ATTRIBUTE_CMD, sdb_p->table, setStr);
    sqlite3_free(setStr);
    if (newStr == NULL) {
	UNLOCK_SQLITE()  
	return CKR_HOST_MEMORY;
    }
    error = sdb_openDBLocal(sdb_p, &sqlDB, NULL);
    if (error != CKR_OK) {
	goto loser;
    }
    sqlerr = sqlite3_prepare_v2(sqlDB, newStr, -1, &stmt, NULL);
    if (sqlerr != SQLITE_OK) goto loser;
    for (i=0; i < count; i++) {
	if (template[i].ulValueLen != 0) {
	    sqlerr = sqlite3_bind_blob(stmt, i+1, template[i].pValue, 
				template[i].ulValueLen, SQLITE_STATIC);
	} else {
	    sqlerr = sqlite3_bind_blob(stmt, i+2, SQLITE_EXPLICIT_NULL, 
			SQLITE_EXPLICIT_NULL_LEN, SQLITE_STATIC);
	}
        if (sqlerr != SQLITE_OK) goto loser;
    }
    sqlerr = sqlite3_bind_int(stmt, i+1, object_id);
    if (sqlerr != SQLITE_OK) goto loser;

    do {
	sqlerr = sqlite3_step(stmt);
	if (sqlerr == SQLITE_BUSY) {
	    PR_Sleep(SDB_BUSY_RETRY_TIME);
	}
    } while (!sdb_done(sqlerr,&retry));

loser:
    if (newStr) {
	sqlite3_free(newStr);
    }
    if (error == CKR_OK) {
	error = sdb_mapSQLError(sdb_p->type, sqlerr);
    }

    if (stmt) {
	sqlite3_reset(stmt);
	sqlite3_finalize(stmt);
    }

    if (sqlDB) {
	sdb_closeDBLocal(sdb_p, sqlDB) ;
    }

    UNLOCK_SQLITE()  
    return error;
}

/*
 * check to see if a candidate object handle already exists.
 */
static PRBool
sdb_objectExists(SDB *sdb, CK_OBJECT_HANDLE candidate)
{
    CK_RV crv;
    CK_ATTRIBUTE template = { CKA_LABEL, NULL, 0 };

    crv = sdb_GetAttributeValueNoLock(sdb,candidate,&template, 1);
    if (crv == CKR_OBJECT_HANDLE_INVALID) {
	return PR_FALSE;
    }
    return PR_TRUE;
}

/*
 * if we're here, we are in a transaction, so it's safe
 * to examine the current state of the database
 */
static CK_OBJECT_HANDLE
sdb_getObjectId(SDB *sdb)
{
    CK_OBJECT_HANDLE candidate;
    static CK_OBJECT_HANDLE next_obj = CK_INVALID_HANDLE;
    int count;
    /*
     * get an initial object handle to use
     */
    if (next_obj == CK_INVALID_HANDLE) {
        PRTime time;
	time = PR_Now();

	next_obj = (CK_OBJECT_HANDLE)(time & 0x3fffffffL);
    }
    candidate = next_obj++;
    /* detect that we've looped through all the handles... */
    for (count = 0; count < 0x40000000; count++, candidate = next_obj++) {
	/* mask off excess bits */
	candidate &= 0x3fffffff;
	/* if we hit zero, go to the next entry */
	if (candidate == CK_INVALID_HANDLE) {
	    continue;
	}
	/* make sure we aren't already using */
	if (!sdb_objectExists(sdb, candidate)) {
	    /* this one is free */
	    return candidate;
	}
    }

    /* no handle is free, fail */
    return CK_INVALID_HANDLE;
}

static const char CREATE_CMD[] = "INSERT INTO %s (id%s) VALUES($ID%s);";
CK_RV
sdb_CreateObject(SDB *sdb, CK_OBJECT_HANDLE *object_id, 
		 const CK_ATTRIBUTE *template, CK_ULONG count)
{
    SDBPrivate *sdb_p = sdb->private;
    sqlite3  *sqlDB = NULL;
    sqlite3_stmt *stmt = NULL;
    char *columnStr = NULL;
    char *valueStr = NULL;
    char *newStr = NULL;
    int sqlerr = SQLITE_OK;
    CK_RV error = CKR_OK;
    CK_OBJECT_HANDLE this_object;
    int retry = 0;
    int i;

    if ((sdb->sdb_flags & SDB_RDONLY) != 0) {
	return CKR_TOKEN_WRITE_PROTECTED;
    }

    LOCK_SQLITE()  
    if ((*object_id != CK_INVALID_HANDLE) && 
		!sdb_objectExists(sdb, *object_id)) {
	this_object = *object_id;
    } else {
	this_object = sdb_getObjectId(sdb);
    }
    if (this_object == CK_INVALID_HANDLE) {
	UNLOCK_SQLITE();
	return CKR_HOST_MEMORY;
    }
    columnStr = sqlite3_mprintf("");
    valueStr = sqlite3_mprintf("");
    *object_id = this_object;
    for (i=0; columnStr && valueStr && i < count; i++) {
   	newStr = sqlite3_mprintf("%s,a%x", columnStr, template[i].type);
	sqlite3_free(columnStr);
	columnStr = newStr;
   	newStr = sqlite3_mprintf("%s,$VALUE%d", valueStr, i);
	sqlite3_free(valueStr);
	valueStr = newStr;
    }
    newStr = NULL;
    if ((columnStr == NULL) || (valueStr == NULL)) {
	if (columnStr) {
	    sqlite3_free(columnStr);
	}
	if (valueStr) {
	    sqlite3_free(valueStr);
	}
	UNLOCK_SQLITE()  
	return CKR_HOST_MEMORY;
    }
    newStr =  sqlite3_mprintf(CREATE_CMD, sdb_p->table, columnStr, valueStr);
    sqlite3_free(columnStr);
    sqlite3_free(valueStr);
    error = sdb_openDBLocal(sdb_p, &sqlDB, NULL);
    if (error != CKR_OK) {
	goto loser;
    }
    sqlerr = sqlite3_prepare_v2(sqlDB, newStr, -1, &stmt, NULL);
    if (sqlerr != SQLITE_OK) goto loser;
    sqlerr = sqlite3_bind_int(stmt, 1, *object_id);
    if (sqlerr != SQLITE_OK) goto loser;
    for (i=0; i < count; i++) {
	if (template[i].ulValueLen) {
	    sqlerr = sqlite3_bind_blob(stmt, i+2, template[i].pValue, 
			template[i].ulValueLen, SQLITE_STATIC);
	} else {
	    sqlerr = sqlite3_bind_blob(stmt, i+2, SQLITE_EXPLICIT_NULL, 
			SQLITE_EXPLICIT_NULL_LEN, SQLITE_STATIC);
	}
        if (sqlerr != SQLITE_OK) goto loser;
    }

    do {
	sqlerr = sqlite3_step(stmt);
	if (sqlerr == SQLITE_BUSY) {
	    PR_Sleep(SDB_BUSY_RETRY_TIME);
	}
    } while (!sdb_done(sqlerr,&retry));

loser:
    if (newStr) {
	sqlite3_free(newStr);
    }
    if (error == CKR_OK) {
	error = sdb_mapSQLError(sdb_p->type, sqlerr);
    }

    if (stmt) {
	sqlite3_reset(stmt);
	sqlite3_finalize(stmt);
    }

    if (sqlDB) {
	sdb_closeDBLocal(sdb_p, sqlDB) ;
    }
    UNLOCK_SQLITE()  

    return error;
}

static const char DESTROY_CMD[] = "DELETE FROM %s WHERE (id=$ID);";
CK_RV
sdb_DestroyObject(SDB *sdb, CK_OBJECT_HANDLE object_id)
{
    SDBPrivate *sdb_p = sdb->private;
    sqlite3  *sqlDB = NULL;
    sqlite3_stmt *stmt = NULL;
    char *newStr = NULL;
    int sqlerr = SQLITE_OK;
    CK_RV error = CKR_OK;
    int retry = 0;

    if ((sdb->sdb_flags & SDB_RDONLY) != 0) {
	return CKR_TOKEN_WRITE_PROTECTED;
    }

    LOCK_SQLITE()  
    error = sdb_openDBLocal(sdb_p, &sqlDB, NULL);
    if (error != CKR_OK) {
	goto loser;
    }
    newStr =  sqlite3_mprintf(DESTROY_CMD, sdb_p->table);
    if (newStr == NULL) {
	error = CKR_HOST_MEMORY;
	goto loser;
    }
    sqlerr =sqlite3_prepare_v2(sqlDB, newStr, -1, &stmt, NULL);
    sqlite3_free(newStr);
    if (sqlerr != SQLITE_OK) goto loser;
    sqlerr =sqlite3_bind_int(stmt, 1, object_id);
    if (sqlerr != SQLITE_OK) goto loser;

    do {
	sqlerr = sqlite3_step(stmt);
	if (sqlerr == SQLITE_BUSY) {
	    PR_Sleep(SDB_BUSY_RETRY_TIME);
	}
    } while (!sdb_done(sqlerr,&retry));

loser:
    if (error == CKR_OK) {
	error = sdb_mapSQLError(sdb_p->type, sqlerr);
    }

    if (stmt) {
	sqlite3_reset(stmt);
	sqlite3_finalize(stmt);
    }

    if (sqlDB) {
	sdb_closeDBLocal(sdb_p, sqlDB) ;
    }

    UNLOCK_SQLITE()  
    return error;
}
   
static const char BEGIN_CMD[] = "BEGIN IMMEDIATE TRANSACTION;";
/*
 * start a transaction.
 *
 * We need to open a new database, then store that new database into
 * the private data structure. We open the database first, then use locks
 * to protect storing the data to prevent deadlocks.
 */
CK_RV
sdb_Begin(SDB *sdb)
{
    SDBPrivate *sdb_p = sdb->private;
    sqlite3  *sqlDB = NULL;
    sqlite3_stmt *stmt = NULL;
    int sqlerr = SQLITE_OK;
    CK_RV error = CKR_OK;
    int retry = 0;


    if ((sdb->sdb_flags & SDB_RDONLY) != 0) {
	return CKR_TOKEN_WRITE_PROTECTED;
    }


    LOCK_SQLITE()  

    /* get a new version that we will use for the entire transaction */
    sqlerr = sdb_openDB(sdb_p->sqlDBName, &sqlDB, SDB_RDWR);
    if (sqlerr != SQLITE_OK) {
	goto loser;
    }

    sqlerr =sqlite3_prepare_v2(sqlDB, BEGIN_CMD, -1, &stmt, NULL);

    do {
	sqlerr = sqlite3_step(stmt);
	if (sqlerr == SQLITE_BUSY) {
	    PR_Sleep(SDB_BUSY_RETRY_TIME);
	}
    } while (!sdb_done(sqlerr,&retry));

    if (stmt) {
	sqlite3_reset(stmt);
	sqlite3_finalize(stmt);
    }

loser:
    error = sdb_mapSQLError(sdb_p->type, sqlerr);

    /* we are starting a new transaction, 
     * and if we succeeded, then save this database for the rest of
     * our transaction */
    if (error == CKR_OK) {
	/* we hold a 'BEGIN TRANSACTION' and a sdb_p->lock. At this point
	 * sdb_p->sqlXactDB MUST be null */
	PR_EnterMonitor(sdb_p->dbMon);
	PORT_Assert(sdb_p->sqlXactDB == NULL);
	sdb_p->sqlXactDB = sqlDB;
	sdb_p->sqlXactThread = PR_GetCurrentThread();
        PR_ExitMonitor(sdb_p->dbMon);
    } else {
	/* we failed to start our transaction,
	 * free any databases we opened. */
	if (sqlDB) {
	    sqlite3_close(sqlDB);
	}
    }

    UNLOCK_SQLITE()  
    return error;
}

/*
 * Complete a transaction. Basically undo everything we did in begin.
 * There are 2 flavors Abort and Commit. Basically the only differerence between
 * these 2 are what the database will show. (no change in to former, change in
 * the latter).
 */
static CK_RV 
sdb_complete(SDB *sdb, const char *cmd)
{
    SDBPrivate *sdb_p = sdb->private;
    sqlite3  *sqlDB = NULL;
    sqlite3_stmt *stmt = NULL;
    int sqlerr = SQLITE_OK;
    CK_RV error = CKR_OK;
    int retry = 0;


    if ((sdb->sdb_flags & SDB_RDONLY) != 0) {
	return CKR_TOKEN_WRITE_PROTECTED;
    }

    /* We must have a transation database, or we shouldn't have arrived here */
    PR_EnterMonitor(sdb_p->dbMon);
    PORT_Assert(sdb_p->sqlXactDB);
    if (sdb_p->sqlXactDB == NULL) {
        PR_ExitMonitor(sdb_p->dbMon);
	return CKR_GENERAL_ERROR; /* shouldn't happen */
    }
    PORT_Assert( sdb_p->sqlXactThread == PR_GetCurrentThread());
    if ( sdb_p->sqlXactThread != PR_GetCurrentThread()) {
        PR_ExitMonitor(sdb_p->dbMon);
	return CKR_GENERAL_ERROR; /* shouldn't happen */
    }
    sqlDB = sdb_p->sqlXactDB;
    sdb_p->sqlXactDB = NULL; /* no one else can get to this DB, 
			      * safe to unlock */
    sdb_p->sqlXactThread = NULL; 
    PR_ExitMonitor(sdb_p->dbMon);

    sqlerr =sqlite3_prepare_v2(sqlDB, cmd, -1, &stmt, NULL);

    do {
	sqlerr = sqlite3_step(stmt);
	if (sqlerr == SQLITE_BUSY) {
	    PR_Sleep(SDB_BUSY_RETRY_TIME);
	}
    } while (!sdb_done(sqlerr,&retry));

    /* Pending BEGIN TRANSACTIONS Can move forward at this point. */

    if (stmt) {
	sqlite3_reset(stmt);
	sqlite3_finalize(stmt);
    }

    /* we we have a cached DB image, update it as well */
    if (sdb_p->cacheTable) {
	PR_EnterMonitor(sdb_p->dbMon);
	sdb_updateCache(sdb_p);
	PR_ExitMonitor(sdb_p->dbMon);
    }

    error = sdb_mapSQLError(sdb_p->type, sqlerr);

    /* We just finished a transaction.
     * Free the database, and remove it from the list */
    sqlite3_close(sqlDB);

    return error;
}

static const char COMMIT_CMD[] = "COMMIT TRANSACTION;";
CK_RV
sdb_Commit(SDB *sdb)
{
    CK_RV crv;
    LOCK_SQLITE()  
    crv = sdb_complete(sdb,COMMIT_CMD);
    UNLOCK_SQLITE()  
    return crv;
}

static const char ROLLBACK_CMD[] = "ROLLBACK TRANSACTION;";
CK_RV
sdb_Abort(SDB *sdb)
{
    CK_RV crv;
    LOCK_SQLITE()  
    crv = sdb_complete(sdb,ROLLBACK_CMD);
    UNLOCK_SQLITE()  
    return crv;
}

static int tableExists(sqlite3 *sqlDB, const char *tableName);

static const char GET_PW_CMD[] = "SELECT ALL * FROM metaData WHERE id=$ID;";
CK_RV
sdb_GetMetaData(SDB *sdb, const char *id, SECItem *item1, SECItem *item2)
{
    SDBPrivate *sdb_p = sdb->private;
    sqlite3  *sqlDB = sdb_p->sqlXactDB;
    sqlite3_stmt *stmt = NULL;
    int sqlerr = SQLITE_OK;
    CK_RV error = CKR_OK;
    int found = 0;
    int retry = 0;

    /* currently only Key databases have meta data entries */
    if (sdb_p->type != SDB_KEY) {
	return CKR_OBJECT_HANDLE_INVALID;
    }

    LOCK_SQLITE()  
    error = sdb_openDBLocal(sdb_p, &sqlDB, NULL);
    if (error != CKR_OK) {
	goto loser;
    }

    /* handle 'test' versions of the sqlite db */
    sqlerr = sqlite3_prepare_v2(sqlDB, GET_PW_CMD, -1, &stmt, NULL);
    /* Sigh, if we created a new table since we opened the database,
     * the database handle will not see the new table, we need to close this
     * database and reopen it. This is safe because we are holding the lock
     * still. */
    if (sqlerr == SQLITE_SCHEMA) {
	sqlerr = sdb_reopenDBLocal(sdb_p, &sqlDB);
	if (sqlerr != SQLITE_OK) {
	    goto loser;
	}
	sqlerr = sqlite3_prepare_v2(sqlDB, GET_PW_CMD, -1, &stmt, NULL);
    }
    if (sqlerr != SQLITE_OK) goto loser;
    sqlerr = sqlite3_bind_text(stmt, 1, id, PORT_Strlen(id), SQLITE_STATIC);
    do {
	sqlerr = sqlite3_step(stmt);
	if (sqlerr == SQLITE_BUSY) {
	    PR_Sleep(SDB_BUSY_RETRY_TIME);
	}
	if (sqlerr == SQLITE_ROW) {
	    const char *blobData;
	    item1->len = sqlite3_column_bytes(stmt, 1);
	    if (item1->len > SDB_MAX_META_DATA_LEN) {
		error = CKR_BUFFER_TOO_SMALL;
		continue;
	    }
	    blobData = sqlite3_column_blob(stmt, 1);
	    PORT_Memcpy(item1->data,blobData, item1->len);
	    if (item2) {
		item2->len = sqlite3_column_bytes(stmt, 2);
		if (item2->len > SDB_MAX_META_DATA_LEN) {
		    error = CKR_BUFFER_TOO_SMALL;
		    continue;
		}
		blobData = sqlite3_column_blob(stmt, 2);
		PORT_Memcpy(item2->data,blobData, item2->len);
	    }
	    found = 1;
	}
    } while (!sdb_done(sqlerr,&retry));

loser:
    /* fix up the error if necessary */
    if (error == CKR_OK) {
	error = sdb_mapSQLError(sdb_p->type, sqlerr);
	if (!found && error == CKR_OK) {
	    error = CKR_OBJECT_HANDLE_INVALID;
	}
    }

    if (stmt) {
	sqlite3_reset(stmt);
	sqlite3_finalize(stmt);
    }

    if (sqlDB) {
	sdb_closeDBLocal(sdb_p, sqlDB) ;
    }
    UNLOCK_SQLITE()  

    return error;
}

static const char PW_CREATE_TABLE_CMD[] =
 "CREATE TABLE metaData (id PRIMARY KEY UNIQUE ON CONFLICT REPLACE, item1, item2);";
static const char PW_CREATE_CMD[] =
 "INSERT INTO metaData (id,item1,item2) VALUES($ID,$ITEM1,$ITEM2);";
static const char MD_CREATE_CMD[]  =
 "INSERT INTO metaData (id,item1) VALUES($ID,$ITEM1);";
CK_RV
sdb_PutMetaData(SDB *sdb, const char *id, const SECItem *item1, 
					   const SECItem *item2)
{
    SDBPrivate *sdb_p = sdb->private;
    sqlite3  *sqlDB = sdb_p->sqlXactDB;
    sqlite3_stmt *stmt = NULL;
    int sqlerr = SQLITE_OK;
    CK_RV error = CKR_OK;
    int retry = 0;
    const char *cmd = PW_CREATE_CMD;

    /* only Key databases have password entries */
    if (sdb_p->type != SDB_KEY) {
	return CKR_OBJECT_HANDLE_INVALID;
    }

    LOCK_SQLITE()  
    error = sdb_openDBLocal(sdb_p, &sqlDB, NULL);
    if (error != CKR_OK) {
	goto loser;
    }

    if (!tableExists(sqlDB, "metaData")) {
    	sqlerr = sqlite3_exec(sqlDB, PW_CREATE_TABLE_CMD, NULL, 0, NULL);
        if (sqlerr != SQLITE_OK) goto loser;
    }
    if (item2 == NULL) {
	cmd = MD_CREATE_CMD;
    }
    sqlerr = sqlite3_prepare_v2(sqlDB, cmd, -1, &stmt, NULL);
    if (sqlerr != SQLITE_OK) goto loser;
    sqlerr = sqlite3_bind_text(stmt, 1, id, PORT_Strlen(id), SQLITE_STATIC);
    if (sqlerr != SQLITE_OK) goto loser;
    sqlerr = sqlite3_bind_blob(stmt, 2, item1->data, item1->len, SQLITE_STATIC);
    if (sqlerr != SQLITE_OK) goto loser;
    if (item2) {
    	sqlerr = sqlite3_bind_blob(stmt, 3, item2->data, 
				   item2->len, SQLITE_STATIC);
        if (sqlerr != SQLITE_OK) goto loser;
    }

    do {
	sqlerr = sqlite3_step(stmt);
	if (sqlerr == SQLITE_BUSY) {
	    PR_Sleep(SDB_BUSY_RETRY_TIME);
	}
    } while (!sdb_done(sqlerr,&retry));

loser:
    /* fix up the error if necessary */
    if (error == CKR_OK) {
	error = sdb_mapSQLError(sdb_p->type, sqlerr);
    }

    if (stmt) {
	sqlite3_reset(stmt);
	sqlite3_finalize(stmt);
    }

    if (sqlDB) {
	sdb_closeDBLocal(sdb_p, sqlDB) ;
    }
    UNLOCK_SQLITE()  

    return error;
}

static const char RESET_CMD[] = "DROP TABLE IF EXISTS %s;";
CK_RV
sdb_Reset(SDB *sdb)
{
    SDBPrivate *sdb_p = sdb->private;
    sqlite3  *sqlDB = NULL;
    char *newStr;
    int sqlerr = SQLITE_OK;
    CK_RV error = CKR_OK;

    /* only Key databases can be reset */
    if (sdb_p->type != SDB_KEY) {
	return CKR_OBJECT_HANDLE_INVALID;
    }

    LOCK_SQLITE()  
    error = sdb_openDBLocal(sdb_p, &sqlDB, NULL);
    if (error != CKR_OK) {
	goto loser;
    }

    /* delete the key table */
    newStr =  sqlite3_mprintf(RESET_CMD, sdb_p->table);
    if (newStr == NULL) {
	error = CKR_HOST_MEMORY;
	goto loser;
    }
    sqlerr = sqlite3_exec(sqlDB, newStr, NULL, 0, NULL);
    sqlite3_free(newStr);

    if (sqlerr != SQLITE_OK) goto loser;

    /* delete the password entry table */
    sqlerr = sqlite3_exec(sqlDB, "DROP TABLE IF EXISTS metaData;", 
                          NULL, 0, NULL);

loser:
    /* fix up the error if necessary */
    if (error == CKR_OK) {
	error = sdb_mapSQLError(sdb_p->type, sqlerr);
    }

    if (sqlDB) {
	sdb_closeDBLocal(sdb_p, sqlDB) ;
    }

    UNLOCK_SQLITE()  
    return error;
}


CK_RV 
sdb_Close(SDB *sdb) 
{
    SDBPrivate *sdb_p = sdb->private;
    int sqlerr = SQLITE_OK;
    sdbDataType type = sdb_p->type;

    sqlerr = sqlite3_close(sdb_p->sqlReadDB);
    PORT_Free(sdb_p->sqlDBName);
    if (sdb_p->cacheTable) {
	sqlite3_free(sdb_p->cacheTable);
    }
    if (sdb_p->dbMon) {
	PR_DestroyMonitor(sdb_p->dbMon);
    }
    free(sdb_p);
    free(sdb);
    return sdb_mapSQLError(type, sqlerr);
}


/*
 * functions to support open
 */

static const char CHECK_TABLE_CMD[] = "SELECT ALL * FROM %s LIMIT 0;";
/* return 1 if sqlDB contains table 'tableName */
static int tableExists(sqlite3 *sqlDB, const char *tableName)
{
    char * cmd = sqlite3_mprintf(CHECK_TABLE_CMD, tableName);
    int sqlerr = SQLITE_OK;

    if (cmd == NULL) {
	return 0;
    }

    sqlerr = sqlite3_exec(sqlDB, cmd, NULL, 0, 0);
    sqlite3_free(cmd);

    return (sqlerr == SQLITE_OK) ? 1 : 0;
}

/*
 * initialize a single database
 */
static const char INIT_CMD[] =
 "CREATE TABLE %s (id PRIMARY KEY UNIQUE ON CONFLICT ABORT%s)";
static const char ALTER_CMD[] = 
 "ALTER TABLE %s ADD COLUMN a%x";

CK_RV 
sdb_init(char *dbname, char *table, sdbDataType type, int *inUpdate,
	 int *newInit, int flags, PRIntervalTime accessTime, SDB **pSdb)
{
    int i;
    char *initStr = NULL;
    char *newStr;
    int inTransaction = 0;
    SDB *sdb = NULL;
    SDBPrivate *sdb_p = NULL;
    sqlite3 *sqlDB = NULL;
    int sqlerr = SQLITE_OK;
    CK_RV error = CKR_OK;
    char *cacheTable = NULL;
    PRIntervalTime now = 0;
    PRIntervalTime tempAccess = 0;
    char *tempDir = NULL;
    char *env;
    PRBool enableCache = PR_FALSE;

    *pSdb = NULL;
    *inUpdate = 0;

    /* sqlite3 doesn't have a flag to specify that we want to 
     * open the database read only. If the db doesn't exist,
     * sqlite3 will always create it.
     */
    LOCK_SQLITE();
    if ((flags == SDB_RDONLY) && PR_Access(dbname, PR_ACCESS_EXISTS)) {
	error = sdb_mapSQLError(type, SQLITE_CANTOPEN);
	goto loser;
    }
    sqlerr = sdb_openDB(dbname, &sqlDB, flags);
    if (sqlerr != SQLITE_OK) {
	error = sdb_mapSQLError(type, sqlerr); 
	goto loser;
    }

    if (flags != SDB_RDONLY) {
	sqlerr = sqlite3_exec(sqlDB, BEGIN_CMD, NULL, 0, NULL);
	if (sqlerr != SQLITE_OK) {
	    error = sdb_mapSQLError(type, sqlerr);
	    goto loser;
	}
	inTransaction = 1;
    }
    if (!tableExists(sqlDB,table)) {
	*newInit = 1;
	if (flags != SDB_CREATE) {
	    error = sdb_mapSQLError(type, SQLITE_CANTOPEN);
	    goto loser;
	}
	initStr = sqlite3_mprintf("");
	for (i=0; initStr && i < known_attributes_size; i++) {
	    newStr = sqlite3_mprintf("%s, a%x",initStr, known_attributes[i]);
	    sqlite3_free(initStr);
	    initStr = newStr;
	}
	if (initStr == NULL) {
	    error = CKR_HOST_MEMORY;
	    goto loser;
	}

	newStr = sqlite3_mprintf(INIT_CMD, table, initStr);
	sqlite3_free(initStr);
	if (newStr == NULL) {
            error = CKR_HOST_MEMORY;
	    goto loser;
	}
	sqlerr = sqlite3_exec(sqlDB, newStr, NULL, 0, NULL);
	sqlite3_free(newStr);
	if (sqlerr != SQLITE_OK) {
            error = sdb_mapSQLError(type, sqlerr); 
	    goto loser;
	}

	newStr = sqlite3_mprintf(CREATE_ISSUER_INDEX_CMD, table);
	if (newStr == NULL) {
            error = CKR_HOST_MEMORY;
	    goto loser;
	}
	sqlerr = sqlite3_exec(sqlDB, newStr, NULL, 0, NULL);
	sqlite3_free(newStr);
	if (sqlerr != SQLITE_OK) {
            error = sdb_mapSQLError(type, sqlerr); 
	    goto loser;
	}

	newStr = sqlite3_mprintf(CREATE_SUBJECT_INDEX_CMD, table);
	if (newStr == NULL) {
            error = CKR_HOST_MEMORY;
	    goto loser;
	}
	sqlerr = sqlite3_exec(sqlDB, newStr, NULL, 0, NULL);
	sqlite3_free(newStr);
	if (sqlerr != SQLITE_OK) {
            error = sdb_mapSQLError(type, sqlerr); 
	    goto loser;
	}

	newStr = sqlite3_mprintf(CREATE_LABEL_INDEX_CMD, table);
	if (newStr == NULL) {
            error = CKR_HOST_MEMORY;
	    goto loser;
	}
	sqlerr = sqlite3_exec(sqlDB, newStr, NULL, 0, NULL);
	sqlite3_free(newStr);
	if (sqlerr != SQLITE_OK) {
            error = sdb_mapSQLError(type, sqlerr); 
	    goto loser;
	}

	newStr = sqlite3_mprintf(CREATE_ID_INDEX_CMD, table);
	if (newStr == NULL) {
            error = CKR_HOST_MEMORY;
	    goto loser;
	}
	sqlerr = sqlite3_exec(sqlDB, newStr, NULL, 0, NULL);
	sqlite3_free(newStr);
	if (sqlerr != SQLITE_OK) {
            error = sdb_mapSQLError(type, sqlerr); 
	    goto loser;
	}
    }
    /*
     * detect the case where we have created the database, but have
     * not yet updated it.
     *
     * We only check the Key database because only the key database has
     * a metaData table. The metaData table is created when a password
     * is set, or in the case of update, when a password is supplied.
     * If no key database exists, then the update would have happened immediately
     * on noticing that the cert database didn't exist (see newInit set above).
     */
    if (type == SDB_KEY && !tableExists(sqlDB, "metaData")) {
	*newInit = 1;
    }
    
    /* access to network filesystems are significantly slower than local ones
     * for database operations. In those cases we need to create a cached copy
     * of the database in a temporary location on the local disk. SQLITE
     * already provides a way to create a temporary table and initialize it,
     * so we use it for the cache (see sdb_buildCache for how it's done).*/

     /* 
      * we decide whether or not to use the cache based on the following input.
      *
      * NSS_SDB_USE_CACHE environment variable is non-existant or set to 
      *   anything other than "no" or "yes" ("auto", for instance).
      *   This is the normal case. NSS will measure the performance of access
      *   to the temp database versus the access to the users passed in 
      *   database location. If the temp database location is "significantly"
      *   faster we will use the cache.
      *
      * NSS_SDB_USE_CACHE environment variable is set to "no": cache will not
      *   be used.
      *
      * NSS_SDB_USE_CACHE environment variable is set to "yes": cache will
      *   always be used.
      *
      * It is expected that most applications would use the "auto" selection,
      * the environment variable is primarily to simplify testing, and to 
      * correct potential corner cases where  */

     env = PR_GetEnv("NSS_SDB_USE_CACHE");

     if (env && PORT_Strcasecmp(env,"no") == 0) {
	enableCache = PR_FALSE;
     } else if (env && PORT_Strcasecmp(env,"yes") == 0) {
	enableCache = PR_TRUE;
     } else {
	/*
	 *  Use PR_Access to determine how expensive it
	 * is to check for the existance of a local file compared to the same
	 * check in the temp directory. If the temp directory is faster, cache
	 * the database there. */
	tempDir = sdb_getTempDir(sqlDB);
	tempAccess = sdb_measureAccess(tempDir);
	PORT_Free(tempDir);
	tempDir = NULL;

	/* there is a cost to continually copying the database, account for
	 * that cost in the temp access time with the arbitrary factor of 4 */
	tempAccess = tempAccess*4;
	enableCache = (tempAccess < accessTime) ? PR_TRUE : PR_FALSE;
    }

    if (enableCache) {
	/* try to set the temp store to memory.*/
	sqlite3_exec(sqlDB, "PRAGMA temp_store=MEMORY", NULL, 0, NULL);
	/* Failure to set the temp store to memory is not fatal,
         * ignore the error */

	cacheTable = sqlite3_mprintf("%sCache",table);
	if (cacheTable == NULL) {
	    error = CKR_HOST_MEMORY;
	    goto loser;
	}
	/* build the cache table */
	error = sdb_buildCache(sqlDB, type, cacheTable, table);
	if (error != CKR_OK) {
	    goto loser;
	}
	/* initialize the last cache build time */
	now = PR_IntervalNow();
    }

    sdb = (SDB *) malloc(sizeof(SDB));
    sdb_p = (SDBPrivate *) malloc(sizeof(SDBPrivate));

    /* invariant fields */
    sdb_p->sqlDBName = PORT_Strdup(dbname);
    sdb_p->type = type;
    sdb_p->table = table;
    sdb_p->cacheTable = cacheTable;
    sdb_p->lastUpdateTime = now;
    /* set the cache delay time. This is how long we will wait before we
     * decide the existing cache is stale. Currently set to 10 sec */
    sdb_p->updateInterval = PR_SecondsToInterval(10); 
    sdb_p->dbMon = PR_NewMonitor();
    /* these fields are protected by the lock */
    sdb_p->sqlXactDB = NULL;
    sdb_p->sqlXactThread = NULL;
    sdb->private = sdb_p;
    sdb->sdb_type = SDB_SQL;
    sdb->sdb_flags = flags | SDB_HAS_META;
    sdb->sdb_FindObjectsInit = sdb_FindObjectsInit;
    sdb->sdb_FindObjects = sdb_FindObjects;
    sdb->sdb_FindObjectsFinal = sdb_FindObjectsFinal;
    sdb->sdb_GetAttributeValue = sdb_GetAttributeValue;
    sdb->sdb_SetAttributeValue = sdb_SetAttributeValue;
    sdb->sdb_CreateObject = sdb_CreateObject;
    sdb->sdb_DestroyObject = sdb_DestroyObject;
    sdb->sdb_GetMetaData = sdb_GetMetaData;
    sdb->sdb_PutMetaData = sdb_PutMetaData;
    sdb->sdb_Begin = sdb_Begin;
    sdb->sdb_Commit = sdb_Commit;
    sdb->sdb_Abort = sdb_Abort;
    sdb->sdb_Close = sdb_Close;

    if (inTransaction) {
	sqlerr = sqlite3_exec(sqlDB, COMMIT_CMD, NULL, 0, NULL);
	if (sqlerr != SQLITE_OK) {
	    error = sdb_mapSQLError(sdb_p->type, sqlerr);
	    goto loser;
	}
	inTransaction = 0;
    }

    sdb_p->sqlReadDB = sqlDB;

    *pSdb = sdb;
    UNLOCK_SQLITE();
    return CKR_OK;

loser:
    /* lots of stuff to do */
    if (inTransaction) {
	sqlite3_exec(sqlDB, ROLLBACK_CMD, NULL, 0, NULL);
    }
    if (sdb) {
	free(sdb);
    }
    if (sdb_p) {
	free(sdb_p);
    }
    if (sqlDB) {
	sqlite3_close(sqlDB);
    }
    UNLOCK_SQLITE();
    return error;

}


/* sdbopen */
CK_RV
s_open(const char *directory, const char *certPrefix, const char *keyPrefix,
	int cert_version, int key_version, int flags, 
	SDB **certdb, SDB **keydb, int *newInit)
{
    char *cert = sdb_BuildFileName(directory, certPrefix,
				   "cert", cert_version, flags);
    char *key = sdb_BuildFileName(directory, keyPrefix,
				   "key", key_version, flags);
    CK_RV error = CKR_OK;
    int inUpdate;
    PRIntervalTime accessTime;

    *certdb = NULL;
    *keydb = NULL;
    *newInit = 0;

#ifdef SQLITE_UNSAFE_THREADS
    if (sqlite_lock == NULL) {
	sqlite_lock = PR_NewLock();
	if (sqlite_lock == NULL) {
	    error = CKR_HOST_MEMORY;
	    goto loser;
	}
    }
#endif

    /* how long does it take to test for a non-existant file in our working
     * directory? Allows us to test if we may be on a network file system */
    accessTime = sdb_measureAccess(directory);

    /*
     * open the cert data base
     */
    if (certdb) {
	/* initialize Certificate database */
	error = sdb_init(cert, "nssPublic", SDB_CERT, &inUpdate,
			 newInit, flags, accessTime, certdb);
	if (error != CKR_OK) {
	    goto loser;
	}
    }

    /*
     * open the key data base: 
     *  NOTE:is we want to implement a single database, we open
     *  the same database file as the certificate here.
     *
     *  cert an key db's have different tables, so they will not
     *  conflict.
     */
    if (keydb) {
	/* initialize the Key database */
	error = sdb_init(key, "nssPrivate", SDB_KEY, &inUpdate, 
			newInit, flags, accessTime, keydb);
	if (error != CKR_OK) {
	    goto loser;
	} 
    }


loser:
    if (cert) {
	sqlite3_free(cert);
    }
    if (key) {
	sqlite3_free(key);
    }

    if (error != CKR_OK) {
	/* currently redundant, but could be necessary if more code is added
	 * just before loser */
	if (keydb && *keydb) {
	    sdb_Close(*keydb);
	}
	if (certdb && *certdb) {
	    sdb_Close(*certdb);
	}
    }

    return error;
}

CK_RV
s_shutdown()
{
#ifdef SQLITE_UNSAFE_THREADS
    if (sqlite_lock) {
	PR_DestroyLock(sqlite_lock);
	sqlite_lock = NULL;
    }
#endif
    return CKR_OK;
}
