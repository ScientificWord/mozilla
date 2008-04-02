//* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
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
 * The Original Code is Url Classifier code
 *
 * The Initial Developer of the Original Code is
 * Google Inc.
 * Portions created by the Initial Developer are Copyright (C) 2006
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Tony Chang <tony@ponderer.org> (original author)
 *   Brett Wilson <brettw@gmail.com>
 *   Dave Camp <dcamp@mozilla.com>
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

#include "nsAutoPtr.h"
#include "nsCOMPtr.h"
#include "mozIStorageService.h"
#include "mozIStorageConnection.h"
#include "mozIStorageStatement.h"
#include "mozStorageHelper.h"
#include "mozStorageCID.h"
#include "nsAppDirectoryServiceDefs.h"
#include "nsAutoLock.h"
#include "nsCRT.h"
#include "nsICryptoHash.h"
#include "nsICryptoHMAC.h"
#include "nsIDirectoryService.h"
#include "nsIObserverService.h"
#include "nsIPrefBranch.h"
#include "nsIPrefBranch2.h"
#include "nsIPrefService.h"
#include "nsIProperties.h"
#include "nsIProxyObjectManager.h"
#include "nsToolkitCompsCID.h"
#include "nsIUrlClassifierUtils.h"
#include "nsUrlClassifierDBService.h"
#include "nsUrlClassifierUtils.h"
#include "nsURILoader.h"
#include "nsString.h"
#include "nsTArray.h"
#include "nsVoidArray.h"
#include "nsNetUtil.h"
#include "nsNetCID.h"
#include "nsThreadUtils.h"
#include "nsXPCOMStrings.h"
#include "prlog.h"
#include "prlock.h"
#include "prprf.h"
#include "prnetdb.h"
#include "zlib.h"

/**
 * The DBServices stores a set of Fragments.  A fragment is one URL
 * fragment containing two or more domain components and some number
 * of path components.
 *
 * Fragment examples:
 *   example.com/
 *   www.example.com/foo/bar
 *   www.mail.example.com/mail
 *
 * Fragments are described in "Simplified Regular Expression Lookup"
 * section of the protocol document at
 * http://code.google.com/p/google-safe-browsing/wiki/Protocolv2Spec
 *
 * A fragment is associated with a domain.  The domain for a given
 * fragment is the three-host-component domain of the fragment (two
 * host components for URLs with only two components) with a trailing
 * slash.  So for the fragments listed above, the domains are
 * example.com/, www.example.com/ and mail.example.com/.
 *
 * Fragments and domains are hashed in the database.  The hash is described
 * in the protocol document, but it's basically a truncated SHA256 hash.
 *
 * A (table, chunk id, domain key, fragment) tuple is referred to as
 * an Entry.
 */

// NSPR_LOG_MODULES=UrlClassifierDbService:5
#if defined(PR_LOGGING)
static const PRLogModuleInfo *gUrlClassifierDbServiceLog = nsnull;
#define LOG(args) PR_LOG(gUrlClassifierDbServiceLog, PR_LOG_DEBUG, args)
#define LOG_ENABLED() PR_LOG_TEST(gUrlClassifierDbServiceLog, 4)
#else
#define LOG(args)
#define LOG_ENABLED() (PR_FALSE)
#endif

// Schema versioning:  note that we don't bother to migrate between different
// versions of the schema, we just start fetching the data freshly with each
// migration.

// The database filename is updated when there is an incompatible
// schema change and we expect both implementations to continue
// accessing the same database (such as between stable versions of the
// platform).
#define DATABASE_FILENAME "urlclassifier3.sqlite"

// The implementation version is updated during development when we
// want to change schema, or to recover from updating bugs.  When an
// implementation version change is detected, the database is scrapped
// and we start over.
#define IMPLEMENTATION_VERSION 3

#define MAX_HOST_COMPONENTS 5
#define MAX_PATH_COMPONENTS 4

// Updates will fail if fed chunks larger than this
#define MAX_CHUNK_SIZE (1024 * 1024)

// Prefs for implementing nsIURIClassifier to block page loads
#define CHECK_MALWARE_PREF      "browser.safebrowsing.malware.enabled"
#define CHECK_MALWARE_DEFAULT   PR_FALSE

#define CHECK_PHISHING_PREF     "browser.safebrowsing.enabled"
#define CHECK_PHISHING_DEFAULT  PR_FALSE

#define GETHASH_NOISE_PREF      "urlclassifier.gethashnoise"
#define GETHASH_NOISE_DEFAULT   4

class nsUrlClassifierDBServiceWorker;

// Singleton instance.
static nsUrlClassifierDBService* sUrlClassifierDBService;

// Thread that we do the updates on.
static nsIThread* gDbBackgroundThread = nsnull;

// Once we've committed to shutting down, don't do work in the background
// thread.
static PRBool gShuttingDownThread = PR_FALSE;

// -------------------------------------------------------------------------
// Hash class implementation

// A convenience wrapper around the potentially-truncated hash for a
// domain or fragment.

template <PRUint32 S>
struct nsUrlClassifierHash
{
  static const PRUint32 sHashSize = S;
  typedef nsUrlClassifierHash<S> self_type;
  PRUint8 buf[S];

  nsresult FromPlaintext(const nsACString& plainText, nsICryptoHash *hash) {
    // From the protocol doc:
    // Each entry in the chunk is composed of the 128 most significant bits
    // of the SHA 256 hash of a suffix/prefix expression.

    nsresult rv = hash->Init(nsICryptoHash::SHA256);
    NS_ENSURE_SUCCESS(rv, rv);

    rv = hash->Update
      (reinterpret_cast<const PRUint8*>(plainText.BeginReading()),
       plainText.Length());
    NS_ENSURE_SUCCESS(rv, rv);

    nsCAutoString hashed;
    rv = hash->Finish(PR_FALSE, hashed);
    NS_ENSURE_SUCCESS(rv, rv);

    NS_ASSERTION(hashed.Length() >= sHashSize,
                 "not enough characters in the hash");

    memcpy(buf, hashed.BeginReading(), sHashSize);

    return NS_OK;
  }

  void Assign(const nsACString& str) {
    NS_ASSERTION(str.Length() >= sHashSize,
                 "string must be at least sHashSize characters long");
    memcpy(buf, str.BeginReading(), sHashSize);
  }

  const PRBool operator==(const self_type& hash) const {
    return (memcmp(buf, hash.buf, sizeof(buf)) == 0);
  }
  const PRBool operator!=(const self_type& hash) const {
    return !(*this == hash);
  }
  const PRBool operator<(const self_type& hash) const {
    return memcmp(buf, hash.buf, sizeof(self_type)) < 0;
  }
  const PRBool StartsWith(const nsUrlClassifierHash<PARTIAL_LENGTH>& hash) const {
    NS_ASSERTION(sHashSize >= PARTIAL_LENGTH, "nsUrlClassifierHash must be at least PARTIAL_LENGTH bytes long");
    return memcmp(buf, hash.buf, PARTIAL_LENGTH) == 0;
  }
};

typedef nsUrlClassifierHash<DOMAIN_LENGTH> nsUrlClassifierDomainHash;
typedef nsUrlClassifierHash<PARTIAL_LENGTH> nsUrlClassifierPartialHash;
typedef nsUrlClassifierHash<COMPLETE_LENGTH> nsUrlClassifierCompleteHash;


// -------------------------------------------------------------------------
// Entry class implementation

// This class represents one entry in the classifier database.  It consists
// of a table id, a chunk id, a domain hash, and a partial or complete hash.
class nsUrlClassifierEntry
{
public:
  nsUrlClassifierEntry()
    : mId(-1)
    , mHavePartial(PR_FALSE)
    , mHaveComplete(PR_FALSE)
    , mTableId(0)
    , mChunkId(0)
    , mAddChunkId(0)
    {}
  ~nsUrlClassifierEntry() {}

  // Check that this entry could potentially match the complete hash.
  PRBool Match(const nsUrlClassifierCompleteHash &hash);

  // Check that the sub entry should apply to this entry.
  PRBool SubMatch(const nsUrlClassifierEntry& sub);

  // Clear out the entry structure
  void Clear();

  // Set the partial hash for this domain.
  void SetHash(const nsUrlClassifierPartialHash &partialHash) {
    mPartialHash = partialHash;
    mHavePartial = PR_TRUE;
  }

  // Set the complete hash for this domain.
  void SetHash(const nsUrlClassifierCompleteHash &completeHash) {
    mCompleteHash = completeHash;
    mHaveComplete = PR_TRUE;
  }

  PRBool operator== (const nsUrlClassifierEntry& entry) const {
    return ! (mTableId != entry.mTableId ||
              mChunkId != entry.mChunkId ||
              mHavePartial != entry.mHavePartial ||
              (mHavePartial && mPartialHash != entry.mPartialHash) ||
              mHaveComplete != entry.mHaveComplete ||
              (mHaveComplete && mCompleteHash != entry.mCompleteHash));
  }

  PRBool operator< (const nsUrlClassifierEntry& entry) const {
    return (mTableId < entry.mTableId ||
            mChunkId < entry.mChunkId ||
            mHavePartial && !entry.mHavePartial ||
            (mHavePartial && mPartialHash < entry.mPartialHash) ||
            mHaveComplete && !entry.mHaveComplete ||
            (mHaveComplete && mCompleteHash < entry.mCompleteHash));
  }

  PRInt64 mId;

  nsUrlClassifierDomainHash mKey;

  PRBool mHavePartial;
  nsUrlClassifierPartialHash mPartialHash;

  PRBool mHaveComplete;
  nsUrlClassifierCompleteHash mCompleteHash;

  PRUint32 mTableId;
  PRUint32 mChunkId;
  PRUint32 mAddChunkId;
};

PRBool
nsUrlClassifierEntry::Match(const nsUrlClassifierCompleteHash &hash)
{
  if (mHaveComplete)
    return mCompleteHash == hash;

  if (mHavePartial)
    return hash.StartsWith(mPartialHash);

  return PR_FALSE;
}

PRBool
nsUrlClassifierEntry::SubMatch(const nsUrlClassifierEntry &subEntry)
{
  if ((mTableId != subEntry.mTableId) || (mChunkId != subEntry.mAddChunkId))
    return PR_FALSE;

  if (subEntry.mHaveComplete)
    return mHaveComplete && mCompleteHash == subEntry.mCompleteHash;

  if (subEntry.mHavePartial)
    return mHavePartial && mPartialHash == subEntry.mPartialHash;

  return PR_FALSE;
}

void
nsUrlClassifierEntry::Clear()
{
  mId = -1;
  mHavePartial = PR_FALSE;
  mHaveComplete = PR_FALSE;
}

// -------------------------------------------------------------------------
// Lookup result class implementation

// This helper class wraps a nsUrlClassifierEntry found during a lookup.
class nsUrlClassifierLookupResult
{
public:
  nsUrlClassifierLookupResult() : mConfirmed(PR_FALSE), mNoise(PR_FALSE) {}
  ~nsUrlClassifierLookupResult() {}

  PRBool operator==(const nsUrlClassifierLookupResult &result) const {
    // Don't need to compare table name, it's contained by id in the entry.
    return (mLookupFragment == result.mLookupFragment &&
            mConfirmed == result.mConfirmed &&
            mEntry == result.mEntry);
  }

  PRBool operator<(const nsUrlClassifierLookupResult &result) const {
    // Don't need to compare table name, it's contained by id in the entry.
    return (mLookupFragment < result.mLookupFragment ||
            mConfirmed < result.mConfirmed ||
            mEntry < result.mEntry);
  }

  // The hash that matched this entry.
  nsUrlClassifierCompleteHash mLookupFragment;

  // The entry that was found during the lookup.
  nsUrlClassifierEntry mEntry;

  // TRUE if the lookup matched a complete hash (not just a partial
  // one).
  PRPackedBool mConfirmed;

  // TRUE if this lookup is gethash noise.  Does not represent an actual
  // result.
  PRPackedBool mNoise;

  // The table name associated with mEntry.mTableId.
  nsCString mTableName;
};

// -------------------------------------------------------------------------
// Store class implementation

// This class mediates access to the classifier and chunk entry tables.
class nsUrlClassifierStore
{
public:
  nsUrlClassifierStore() {}
  virtual ~nsUrlClassifierStore() {}

  // Initialize the statements for the store.
  nsresult Init(nsUrlClassifierDBServiceWorker *worker,
                mozIStorageConnection *connection,
                const nsACString& entriesTableName);
  // Shut down the store.
  void Close();

  // Read an entry from a database statement
  virtual PRBool ReadStatement(mozIStorageStatement* statement,
                               nsUrlClassifierEntry& entry);

  // Prepare a statement to write this entry to the database
  virtual nsresult BindStatement(const nsUrlClassifierEntry& entry,
                                 mozIStorageStatement* statement);

  // Read the entries for a given key/table from the database
  nsresult ReadEntries(const nsUrlClassifierDomainHash& key,
                       PRUint32 tableId,
                       nsTArray<nsUrlClassifierEntry>& entry);

  // Read the entries for a given key/table/chunk from the database
  nsresult ReadEntries(const nsUrlClassifierDomainHash& key,
                       PRUint32 tableId,
                       PRUint32 chunkId,
                       nsTArray<nsUrlClassifierEntry>& entry);

  // Read the entry with a given ID from the database
  nsresult ReadEntry(PRInt64 id, nsUrlClassifierEntry& entry, PRBool *exists);

  // Remove an entry from the database
  nsresult DeleteEntry(nsUrlClassifierEntry& entry);

  // Write an entry to the database
  nsresult WriteEntry(nsUrlClassifierEntry& entry);

  // Update an entry in the database.  The entry must already exist in the
  // database or this method will fail.
  nsresult UpdateEntry(nsUrlClassifierEntry& entry);

  // Remove all entries for a given table/chunk pair from the database.
  nsresult Expire(PRUint32 tableId,
                  PRUint32 chunkNum);

  // Read a certain number of rows adjacent to the requested rowid that
  // don't have complete hash data.
  nsresult ReadNoiseEntries(PRInt64 rowID,
                            PRUint32 numRequested,
                            PRBool before,
                            nsTArray<nsUrlClassifierEntry> &entries);

  // Ask the db for a random number.  This is temporary, and should be
  // replaced with nsIRandomGenerator when 419739 is fixed.
  nsresult RandomNumber(PRInt64 *randomNum);

  // Retrieve the lookup statement for this table.
  mozIStorageStatement *LookupStatement() { return mLookupStatement; }

protected:
  nsresult ReadEntries(mozIStorageStatement *statement,
                       nsTArray<nsUrlClassifierEntry>& entries);
  nsUrlClassifierDBServiceWorker *mWorker;
  nsCOMPtr<mozIStorageConnection> mConnection;

  nsCOMPtr<mozIStorageStatement> mLookupStatement;
  nsCOMPtr<mozIStorageStatement> mLookupWithTableStatement;
  nsCOMPtr<mozIStorageStatement> mLookupWithChunkStatement;
  nsCOMPtr<mozIStorageStatement> mLookupWithIDStatement;

  nsCOMPtr<mozIStorageStatement> mInsertStatement;
  nsCOMPtr<mozIStorageStatement> mUpdateStatement;
  nsCOMPtr<mozIStorageStatement> mDeleteStatement;
  nsCOMPtr<mozIStorageStatement> mExpireStatement;

  nsCOMPtr<mozIStorageStatement> mPartialEntriesStatement;
  nsCOMPtr<mozIStorageStatement> mPartialEntriesAfterStatement;
  nsCOMPtr<mozIStorageStatement> mLastPartialEntriesStatement;
  nsCOMPtr<mozIStorageStatement> mPartialEntriesBeforeStatement;

  nsCOMPtr<mozIStorageStatement> mRandomStatement;
};

nsresult
nsUrlClassifierStore::Init(nsUrlClassifierDBServiceWorker *worker,
                           mozIStorageConnection *connection,
                           const nsACString& entriesName)
{
  mWorker = worker;
  mConnection = connection;

  nsresult rv = mConnection->CreateStatement
    (NS_LITERAL_CSTRING("SELECT * FROM ") + entriesName +
     NS_LITERAL_CSTRING(" WHERE domain=?1"),
     getter_AddRefs(mLookupStatement));
  NS_ENSURE_SUCCESS(rv, rv);

  rv = mConnection->CreateStatement
    (NS_LITERAL_CSTRING("SELECT * FROM ") + entriesName +
     NS_LITERAL_CSTRING(" WHERE domain=?1 AND table_id=?2"),
     getter_AddRefs(mLookupWithTableStatement));
  NS_ENSURE_SUCCESS(rv, rv);

  rv = mConnection->CreateStatement
    (NS_LITERAL_CSTRING("SELECT * FROM ") + entriesName +
     NS_LITERAL_CSTRING(" WHERE id=?1"),
     getter_AddRefs(mLookupWithIDStatement));
  NS_ENSURE_SUCCESS(rv, rv);

  rv = mConnection->CreateStatement
    (NS_LITERAL_CSTRING("SELECT * FROM ") + entriesName +
     NS_LITERAL_CSTRING(" WHERE domain=?1 AND table_id=?2 AND chunk_id=?3"),
     getter_AddRefs(mLookupWithChunkStatement));
  NS_ENSURE_SUCCESS(rv, rv);

  rv = mConnection->CreateStatement
    (NS_LITERAL_CSTRING("DELETE FROM ") + entriesName +
     NS_LITERAL_CSTRING(" WHERE id=?1"),
     getter_AddRefs(mDeleteStatement));
  NS_ENSURE_SUCCESS(rv, rv);

  rv = mConnection->CreateStatement
    (NS_LITERAL_CSTRING("DELETE FROM ") + entriesName +
     NS_LITERAL_CSTRING(" WHERE table_id=?1 AND chunk_id=?2"),
     getter_AddRefs(mExpireStatement));
  NS_ENSURE_SUCCESS(rv, rv);

  rv = mConnection->CreateStatement
    (NS_LITERAL_CSTRING("SELECT * FROM ") + entriesName +
     NS_LITERAL_CSTRING(" WHERE complete_data ISNULL"
                        " LIMIT ?1"),
     getter_AddRefs(mPartialEntriesStatement));
  NS_ENSURE_SUCCESS(rv, rv);

  rv = mConnection->CreateStatement
    (NS_LITERAL_CSTRING("SELECT * FROM ") + entriesName +
     NS_LITERAL_CSTRING(" WHERE id > ?1 AND complete_data ISNULL"
                        " LIMIT ?2"),
     getter_AddRefs(mPartialEntriesAfterStatement));
  NS_ENSURE_SUCCESS(rv, rv);

  rv = mConnection->CreateStatement
    (NS_LITERAL_CSTRING("SELECT * FROM ") + entriesName +
     NS_LITERAL_CSTRING(" WHERE complete_data ISNULL"
                        " ORDER BY id DESC LIMIT ?1"),
     getter_AddRefs(mLastPartialEntriesStatement));
  NS_ENSURE_SUCCESS(rv, rv);

  rv = mConnection->CreateStatement
    (NS_LITERAL_CSTRING("SELECT * FROM ") + entriesName +
     NS_LITERAL_CSTRING(" WHERE id < ?1 AND complete_data ISNULL"
                        " ORDER BY id DESC LIMIT ?2"),
     getter_AddRefs(mPartialEntriesBeforeStatement));
  NS_ENSURE_SUCCESS(rv, rv);

  rv = mConnection->CreateStatement
    (NS_LITERAL_CSTRING("SELECT abs(random())"),
     getter_AddRefs(mRandomStatement));
  NS_ENSURE_SUCCESS(rv, rv);

  return NS_OK;
}

void
nsUrlClassifierStore::Close()
{
  mLookupStatement = nsnull;
  mLookupWithTableStatement = nsnull;
  mLookupWithIDStatement = nsnull;
  mLookupWithChunkStatement = nsnull;

  mInsertStatement = nsnull;
  mUpdateStatement = nsnull;
  mDeleteStatement = nsnull;
  mExpireStatement = nsnull;

  mPartialEntriesStatement = nsnull;
  mPartialEntriesAfterStatement = nsnull;
  mPartialEntriesBeforeStatement = nsnull;
  mLastPartialEntriesStatement = nsnull;

  mRandomStatement = nsnull;

  mConnection = nsnull;
}


PRBool
nsUrlClassifierStore::ReadStatement(mozIStorageStatement* statement,
                                    nsUrlClassifierEntry& entry)
{
  entry.mId = statement->AsInt64(0);

  PRUint32 size;
  const PRUint8* blob = statement->AsSharedBlob(1, &size);
  if (!blob || (size != DOMAIN_LENGTH))
    return PR_FALSE;
  memcpy(entry.mKey.buf, blob, DOMAIN_LENGTH);

  blob = statement->AsSharedBlob(2, &size);
  if (!blob || size == 0) {
    entry.mHavePartial = PR_FALSE;
  } else {
    if (size != PARTIAL_LENGTH)
      return PR_FALSE;
    entry.mHavePartial = PR_TRUE;
    memcpy(entry.mPartialHash.buf, blob, PARTIAL_LENGTH);
  }

  blob = statement->AsSharedBlob(3, &size);
  if (!blob || size == 0) {
    entry.mHaveComplete = PR_FALSE;
  } else {
    if (size != COMPLETE_LENGTH)
      return PR_FALSE;
    entry.mHaveComplete = PR_TRUE;
    memcpy(entry.mCompleteHash.buf, blob, COMPLETE_LENGTH);
  }

  // If we only have a partial entry, and that partial entry matches the
  // domain, we don't save the extra copy to the database.
  if (!(entry.mHavePartial || entry.mHaveComplete)) {
    entry.SetHash(entry.mKey);
  }

  entry.mChunkId = statement->AsInt32(4);
  entry.mTableId = statement->AsInt32(5);

  return PR_TRUE;
}

nsresult
nsUrlClassifierStore::BindStatement(const nsUrlClassifierEntry &entry,
                                    mozIStorageStatement* statement)
{
  nsresult rv;

  if (entry.mId == -1)
    rv = statement->BindNullParameter(0);
  else
    rv = statement->BindInt64Parameter(0, entry.mId);
  NS_ENSURE_SUCCESS(rv, rv);

  rv = statement->BindBlobParameter(1, entry.mKey.buf, DOMAIN_LENGTH);
  NS_ENSURE_SUCCESS(rv, rv);

  if (entry.mHavePartial) {
    // If we only have a partial entry and that entry matches the domain,
    // we'll save some space by only storing the domain hash.
    if (!entry.mHaveComplete && entry.mKey == entry.mPartialHash) {
      rv = statement->BindNullParameter(2);
    } else {
      rv = statement->BindBlobParameter(2, entry.mPartialHash.buf,
                                        PARTIAL_LENGTH);
    }
  } else {
    rv = statement->BindNullParameter(2);
  }
  NS_ENSURE_SUCCESS(rv, rv);

  if (entry.mHaveComplete) {
    rv = statement->BindBlobParameter(3, entry.mCompleteHash.buf, COMPLETE_LENGTH);
  } else {
    rv = statement->BindNullParameter(3);
  }
  NS_ENSURE_SUCCESS(rv, rv);

  rv = statement->BindInt32Parameter(4, entry.mChunkId);
  NS_ENSURE_SUCCESS(rv, rv);

  rv = statement->BindInt32Parameter(5, entry.mTableId);
  NS_ENSURE_SUCCESS(rv, rv);

  return PR_TRUE;
}

nsresult
nsUrlClassifierStore::ReadEntries(mozIStorageStatement *statement,
                                  nsTArray<nsUrlClassifierEntry>& entries)
{
  PRBool exists;
  nsresult rv = statement->ExecuteStep(&exists);
  NS_ENSURE_SUCCESS(rv, rv);

  while (exists) {
    nsUrlClassifierEntry *entry = entries.AppendElement();
    if (!entry) {
      return NS_ERROR_OUT_OF_MEMORY;
    }

    if (!ReadStatement(statement, *entry))
      return NS_ERROR_FAILURE;

    statement->ExecuteStep(&exists);
  }

  return NS_OK;
}

nsresult
nsUrlClassifierStore::ReadEntries(const nsUrlClassifierDomainHash& hash,
                                  PRUint32 tableId,
                                  nsTArray<nsUrlClassifierEntry>& entries)
{
  mozStorageStatementScoper scoper(mLookupWithTableStatement);

  nsresult rv = mLookupWithTableStatement->BindBlobParameter
                  (0, hash.buf, DOMAIN_LENGTH);
  NS_ENSURE_SUCCESS(rv, rv);
  rv = mLookupWithTableStatement->BindInt32Parameter(1, tableId);
  NS_ENSURE_SUCCESS(rv, rv);

  return ReadEntries(mLookupWithTableStatement, entries);
}

nsresult
nsUrlClassifierStore::ReadEntries(const nsUrlClassifierDomainHash& hash,
                                  PRUint32 tableId,
                                  PRUint32 chunkId,
                                  nsTArray<nsUrlClassifierEntry>& entries)
{
  mozStorageStatementScoper scoper(mLookupWithChunkStatement);

  nsresult rv = mLookupWithChunkStatement->BindBlobParameter
                  (0, hash.buf, DOMAIN_LENGTH);
  NS_ENSURE_SUCCESS(rv, rv);

  rv = mLookupWithChunkStatement->BindInt32Parameter(1, tableId);
  NS_ENSURE_SUCCESS(rv, rv);
  rv = mLookupWithChunkStatement->BindInt32Parameter(2, chunkId);
  NS_ENSURE_SUCCESS(rv, rv);

  return ReadEntries(mLookupWithChunkStatement, entries);
}

nsresult
nsUrlClassifierStore::ReadEntry(PRInt64 id,
                                nsUrlClassifierEntry& entry,
                                PRBool *exists)
{
  entry.Clear();

  mozStorageStatementScoper scoper(mLookupWithIDStatement);

  nsresult rv = mLookupWithIDStatement->BindInt64Parameter(0, id);
  NS_ENSURE_SUCCESS(rv, rv);

  rv = mLookupWithIDStatement->ExecuteStep(exists);
  NS_ENSURE_SUCCESS(rv, rv);

  if (*exists) {
    if (ReadStatement(mLookupWithIDStatement, entry))
      return NS_ERROR_FAILURE;
  }

  return NS_OK;
}

nsresult
nsUrlClassifierStore::ReadNoiseEntries(PRInt64 rowID,
                                       PRUint32 numRequested,
                                       PRBool before,
                                       nsTArray<nsUrlClassifierEntry> &entries)
{
  if (numRequested == 0) {
    return NS_OK;
  }

  mozIStorageStatement *statement =
    before ? mPartialEntriesBeforeStatement : mPartialEntriesAfterStatement;
  mozStorageStatementScoper scoper(statement);

  nsresult rv = statement->BindInt64Parameter(0, rowID);
  NS_ENSURE_SUCCESS(rv, rv);

  statement->BindInt32Parameter(1, numRequested);
  NS_ENSURE_SUCCESS(rv, rv);

  PRUint32 length = entries.Length();
  rv = ReadEntries(statement, entries);
  NS_ENSURE_SUCCESS(rv, rv);

  PRUint32 numRead = entries.Length() - length;

  if (numRead >= numRequested)
    return NS_OK;

  // If we didn't get enough entries, we need the search to wrap around from
  // beginning to end (or vice-versa)

  mozIStorageStatement *wraparoundStatement =
    before ? mPartialEntriesStatement : mLastPartialEntriesStatement;
  mozStorageStatementScoper wraparoundScoper(wraparoundStatement);

  rv = wraparoundStatement->BindInt32Parameter(0, numRequested - numRead);
  NS_ENSURE_SUCCESS(rv, rv);

  return ReadEntries(wraparoundStatement, entries);
}

nsresult
nsUrlClassifierStore::RandomNumber(PRInt64 *randomNum)
{
  mozStorageStatementScoper randScoper(mRandomStatement);
  PRBool exists;
  nsresult rv = mRandomStatement->ExecuteStep(&exists);
  NS_ENSURE_SUCCESS(rv, rv);
  if (!exists)
    return NS_ERROR_NOT_AVAILABLE;

  *randomNum = mRandomStatement->AsInt64(0);

  return NS_OK;
}

// -------------------------------------------------------------------------
// nsUrlClassifierAddStore class implementation

// This class accesses the moz_classifier table.
class nsUrlClassifierAddStore: public nsUrlClassifierStore
{
public:
  nsUrlClassifierAddStore() {};
  virtual ~nsUrlClassifierAddStore() {};

  nsresult Init(nsUrlClassifierDBServiceWorker *worker,
                mozIStorageConnection *connection,
                const nsACString& entriesTableName);
};

nsresult
nsUrlClassifierAddStore::Init(nsUrlClassifierDBServiceWorker *worker,
                              mozIStorageConnection *connection,
                              const nsACString &entriesTableName)
{
  nsresult rv = nsUrlClassifierStore::Init(worker, connection,
                                           entriesTableName);
  NS_ENSURE_SUCCESS(rv, rv);

  rv = mConnection->CreateStatement
    (NS_LITERAL_CSTRING("INSERT OR REPLACE INTO ") + entriesTableName +
     NS_LITERAL_CSTRING(" VALUES (?1, ?2, ?3, ?4, ?5, ?6)"),
     getter_AddRefs(mInsertStatement));
  NS_ENSURE_SUCCESS(rv, rv);

  rv = mConnection->CreateStatement
    (NS_LITERAL_CSTRING("UPDATE ") + entriesTableName +
     NS_LITERAL_CSTRING(" SET domain=?2, partial_data=?3, "
                        " complete_data=?4, chunk_id=?5, table_id=?6"
                        " WHERE id=?1"),
     getter_AddRefs(mUpdateStatement));
  NS_ENSURE_SUCCESS(rv, rv);

  return NS_OK;
}

// -------------------------------------------------------------------------
// nsUrlClassifierSubStore class implementation

// This class accesses the moz_subs table.
class nsUrlClassifierSubStore : public nsUrlClassifierStore
{
public:
  nsUrlClassifierSubStore() {};
  virtual ~nsUrlClassifierSubStore() {};

  nsresult Init(nsUrlClassifierDBServiceWorker *worker,
                mozIStorageConnection *connection,
                const nsACString& entriesTableName);

  void Close();

  // Read an entry from a database statement
  virtual PRBool ReadStatement(mozIStorageStatement* statement,
                               nsUrlClassifierEntry& entry);

  // Prepare a statement to write this entry to the database
  virtual nsresult BindStatement(const nsUrlClassifierEntry& entry,
                                 mozIStorageStatement* statement);

  // Read a sub entry that would apply to the given add
  nsresult ReadSubEntries(const nsUrlClassifierEntry &addEntry,
                          nsTArray<nsUrlClassifierEntry> &subEntry);

protected:
  nsCOMPtr<mozIStorageStatement> mLookupWithAddChunkStatement;
};

nsresult
nsUrlClassifierSubStore::Init(nsUrlClassifierDBServiceWorker *worker,
                              mozIStorageConnection *connection,
                              const nsACString &entriesTableName)
{
  nsresult rv = nsUrlClassifierStore::Init(worker, connection,
                                           entriesTableName);
  NS_ENSURE_SUCCESS(rv, rv);

  rv = mConnection->CreateStatement
    (NS_LITERAL_CSTRING("INSERT OR REPLACE INTO ") + entriesTableName +
     NS_LITERAL_CSTRING(" VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7)"),
     getter_AddRefs(mInsertStatement));
  NS_ENSURE_SUCCESS(rv, rv);

  rv = mConnection->CreateStatement
    (NS_LITERAL_CSTRING("UPDATE ") + entriesTableName +
     NS_LITERAL_CSTRING(" SET domain=?2, partial_data=?3, complete_data=?4,"
                        " chunk_id=?5, table_id=?6, add_chunk_id=?7"
                        " WHERE id=?1"),
     getter_AddRefs(mUpdateStatement));
  NS_ENSURE_SUCCESS(rv, rv);

  rv = mConnection->CreateStatement
    (NS_LITERAL_CSTRING("SELECT * FROM ") + entriesTableName +
     NS_LITERAL_CSTRING(" WHERE domain=?1 AND table_id=?2 AND add_chunk_id=?3"),
     getter_AddRefs(mLookupWithAddChunkStatement));
  NS_ENSURE_SUCCESS(rv, rv);

  return NS_OK;
}

PRBool
nsUrlClassifierSubStore::ReadStatement(mozIStorageStatement* statement,
                                       nsUrlClassifierEntry& entry)
{
  if (!nsUrlClassifierStore::ReadStatement(statement, entry))
    return PR_FALSE;

  entry.mAddChunkId = statement->AsInt32(6);
  return PR_TRUE;
}

nsresult
nsUrlClassifierSubStore::BindStatement(const nsUrlClassifierEntry& entry,
                                       mozIStorageStatement* statement)
{
  nsresult rv = nsUrlClassifierStore::BindStatement(entry, statement);
  NS_ENSURE_SUCCESS(rv, rv);

  return statement->BindInt32Parameter(6, entry.mAddChunkId);
}

nsresult
nsUrlClassifierSubStore::ReadSubEntries(const nsUrlClassifierEntry &addEntry,
                                        nsTArray<nsUrlClassifierEntry>& entries)
{
  mozStorageStatementScoper scoper(mLookupWithAddChunkStatement);

  nsresult rv = mLookupWithAddChunkStatement->BindBlobParameter
                  (0, addEntry.mKey.buf, DOMAIN_LENGTH);
  NS_ENSURE_SUCCESS(rv, rv);
  rv = mLookupWithAddChunkStatement->BindInt32Parameter(1, addEntry.mTableId);
  NS_ENSURE_SUCCESS(rv, rv);
  rv = mLookupWithAddChunkStatement->BindInt32Parameter(2, addEntry.mChunkId);
  NS_ENSURE_SUCCESS(rv, rv);

  return ReadEntries(mLookupWithAddChunkStatement, entries);
}

void
nsUrlClassifierSubStore::Close()
{
  nsUrlClassifierStore::Close();
  mLookupWithAddChunkStatement = nsnull;
}

// -------------------------------------------------------------------------
// Actual worker implemenatation
class nsUrlClassifierDBServiceWorker : public nsIUrlClassifierDBServiceWorker
{
public:
  nsUrlClassifierDBServiceWorker();

  NS_DECL_ISUPPORTS
  NS_DECL_NSIURLCLASSIFIERDBSERVICE
  NS_DECL_NSIURLCLASSIFIERDBSERVICEWORKER

  // Initialize, called in the main thread
  nsresult Init(PRInt32 gethashNoise);

  // Queue a lookup for the worker to perform, called in the main thread.
  nsresult QueueLookup(const nsACString& lookupKey,
                       nsIUrlClassifierLookupCallback* callback);

  // Handle any queued-up lookups.  We call this function during long-running
  // update operations to prevent lookups from blocking for too long.
  nsresult HandlePendingLookups();

private:
  // No subclassing
  ~nsUrlClassifierDBServiceWorker();

  // Disallow copy constructor
  nsUrlClassifierDBServiceWorker(nsUrlClassifierDBServiceWorker&);

  // Try to open the db, DATABASE_FILENAME.
  nsresult OpenDb();

  // Create table in the db if they don't exist.
  nsresult MaybeCreateTables(mozIStorageConnection* connection);

  nsresult GetTableName(PRUint32 tableId, nsACString& table);
  nsresult GetTableId(const nsACString& table, PRUint32* tableId);

  // Decompress a zlib'ed chunk (used for -exp tables)
  nsresult InflateChunk(nsACString& chunk);

  // Expand shavar chunk into its individual entries
  nsresult GetShaEntries(PRUint32 tableId,
                         PRUint32 chunkType,
                         PRUint32 chunkNum,
                         PRUint32 domainSize,
                         PRUint32 hashSize,
                         nsACString& chunk,
                         nsTArray<nsUrlClassifierEntry>& entries);

  // Expand a chunk into its individual entries
  nsresult GetChunkEntries(const nsACString& table,
                           PRUint32 tableId,
                           PRUint32 chunkType,
                           PRUint32 chunkNum,
                           PRUint32 hashSize,
                           nsACString& chunk,
                           nsTArray<nsUrlClassifierEntry>& entries);

  // Parse one stringified range of chunks of the form "n" or "n-m" from a
  // comma-separated list of chunks.  Upon return, 'begin' will point to the
  // next range of chunks in the list of chunks.
  PRBool ParseChunkRange(nsACString::const_iterator &begin,
                         const nsACString::const_iterator &end,
                         PRUint32 *first, PRUint32 *last);

  // Expand a stringified chunk list into an array of ints.
  nsresult ParseChunkList(const nsACString& chunkStr,
                          nsTArray<PRUint32>& chunks);

  // Join an array of ints into a stringified chunk list.
  nsresult JoinChunkList(nsTArray<PRUint32>& chunks, nsCString& chunkStr);

  // List the add/subtract chunks that have been applied to a table
  nsresult GetChunkLists(PRUint32 tableId,
                         nsACString& addChunks,
                         nsACString& subChunks);

  // Set the list of add/subtract chunks that have been applied to a table
  nsresult SetChunkLists(PRUint32 tableId,
                         const nsACString& addChunks,
                         const nsACString& subChunks);

  // Cache the list of add/subtract chunks applied to the table, optionally
  // parsing the add or sub lists.  These lists are cached while updating
  // tables to avoid excessive database reads/writes and parsing.
  nsresult CacheChunkLists(PRUint32 tableId,
                           PRBool parseAdds,
                           PRBool parseSubs);

  // Clear the cached list of add/subtract chunks.
  void ClearCachedChunkLists();

  // Flush the cached add/subtract lists to the database.
  nsresult FlushChunkLists();

  // Add a list of entries to the database, merging with
  // existing entries as necessary
  nsresult AddChunk(PRUint32 tableId, PRUint32 chunkNum,
                    nsTArray<nsUrlClassifierEntry>& entries);

  // Expire an add chunk
  nsresult ExpireAdd(PRUint32 tableId, PRUint32 chunkNum);

  // Subtract a list of entries from the database
  nsresult SubChunk(PRUint32 tableId, PRUint32 chunkNum,
                    nsTArray<nsUrlClassifierEntry>& entries);

  // Expire a subtract chunk
  nsresult ExpireSub(PRUint32 tableId, PRUint32 chunkNum);

  // Handle line-oriented control information from a stream update
  nsresult ProcessResponseLines(PRBool* done);
  // Handle chunk data from a stream update
  nsresult ProcessChunk(PRBool* done);

  // Reset the in-progress update stream
  void ResetStream();

  // Reset the in-progress update
  void ResetUpdate();

  // take a lookup string (www.hostname.com/path/to/resource.html) and
  // expand it into the set of fragments that should be searched for in an
  // entry
  nsresult GetLookupFragments(const nsCSubstring& spec,
                              nsTArray<nsUrlClassifierCompleteHash>& fragments);

  // Check for a canonicalized IP address.
  PRBool IsCanonicalizedIP(const nsACString& host);

  // Get the database key for a given URI.  This is the top three
  // domain components if they exist, otherwise the top two.
  //  hostname.com/foo/bar -> hostname.com
  //  mail.hostname.com/foo/bar -> mail.hostname.com
  //  www.mail.hostname.com/foo/bar -> mail.hostname.com
  nsresult GetKey(const nsACString& spec, nsUrlClassifierDomainHash& hash);

  // Look for a given lookup string (www.hostname.com/path/to/resource.html)
  // in the entries at the given key.  Returns a list of entries that match.
  nsresult CheckKey(const nsCSubstring& spec,
                    const nsUrlClassifierDomainHash& key,
                    nsTArray<nsUrlClassifierLookupResult>& results);

  // Perform a classifier lookup for a given url.
  nsresult DoLookup(const nsACString& spec, nsIUrlClassifierLookupCallback* c);

  // Add entries to the results.
  nsresult AddNoise(PRInt64 nearID,
                    PRInt32 count,
                    nsTArray<nsUrlClassifierLookupResult>& results);

  nsCOMPtr<nsIFile> mDBFile;

  nsCOMPtr<nsICryptoHash> mCryptoHash;

  // Holds a connection to the Db.  We lazily initialize this because it has
  // to be created in the background thread (currently mozStorageConnection
  // isn't thread safe).
  nsCOMPtr<mozIStorageConnection> mConnection;

  // The main collection of entries.  This is the store that will be checked
  // when classifying a URL.
  nsUrlClassifierAddStore mMainStore;

  // The collection of subs waiting for their accompanying add.
  nsUrlClassifierSubStore mPendingSubStore;

  nsCOMPtr<mozIStorageStatement> mGetChunkListsStatement;
  nsCOMPtr<mozIStorageStatement> mSetChunkListsStatement;

  nsCOMPtr<mozIStorageStatement> mGetTablesStatement;
  nsCOMPtr<mozIStorageStatement> mGetTableIdStatement;
  nsCOMPtr<mozIStorageStatement> mGetTableNameStatement;
  nsCOMPtr<mozIStorageStatement> mInsertTableIdStatement;

  // We receive data in small chunks that may be broken in the middle of
  // a line.  So we save the last partial line here.
  nsCString mPendingStreamUpdate;

  PRInt32 mUpdateWait;

  PRBool mResetRequested;

  enum {
    STATE_LINE,
    STATE_CHUNK
  } mState;

  enum {
    CHUNK_ADD,
    CHUNK_SUB
  } mChunkType;

  PRUint32 mChunkNum;
  PRUint32 mHashSize;
  PRUint32 mChunkLen;

  nsCString mUpdateTable;
  PRUint32 mUpdateTableId;

  nsresult mUpdateStatus;

  nsCOMPtr<nsIUrlClassifierUpdateObserver> mUpdateObserver;
  PRBool mInStream;
  PRBool mPrimaryStream;

  PRBool mHaveCachedLists;
  PRUint32 mCachedListsTable;
  nsCAutoString mCachedSubsStr;
  nsCAutoString mCachedAddsStr;

  PRBool mHaveCachedAddChunks;
  nsTArray<PRUint32> mCachedAddChunks;

  PRBool mHaveCachedSubChunks;
  nsTArray<PRUint32> mCachedSubChunks;

  // The client key with which the data from the server will be MAC'ed.
  nsCString mUpdateClientKey;

  // The MAC stated by the server.
  nsCString mServerMAC;

  nsCOMPtr<nsICryptoHMAC> mHMAC;
  // The number of noise entries to add to the set of lookup results.
  PRInt32 mGethashNoise;

  // Pending lookups are stored in a queue for processing.  The queue
  // is protected by mPendingLookupLock.
  PRLock* mPendingLookupLock;

  class PendingLookup {
  public:
    nsCString mKey;
    nsCOMPtr<nsIUrlClassifierLookupCallback> mCallback;
  };

  // list of pending lookups
  nsTArray<PendingLookup> mPendingLookups;
};

NS_IMPL_THREADSAFE_ISUPPORTS1(nsUrlClassifierDBServiceWorker,
                              nsIUrlClassifierDBServiceWorker)

nsUrlClassifierDBServiceWorker::nsUrlClassifierDBServiceWorker()
  : mUpdateWait(0)
  , mResetRequested(PR_FALSE)
  , mState(STATE_LINE)
  , mChunkType(CHUNK_ADD)
  , mChunkNum(0)
  , mHashSize(0)
  , mChunkLen(0)
  , mUpdateTableId(0)
  , mUpdateStatus(NS_OK)
  , mInStream(PR_FALSE)
  , mPrimaryStream(PR_FALSE)
  , mHaveCachedLists(PR_FALSE)
  , mCachedListsTable(PR_UINT32_MAX)
  , mHaveCachedAddChunks(PR_FALSE)
  , mHaveCachedSubChunks(PR_FALSE)
  , mGethashNoise(0)
  , mPendingLookupLock(nsnull)
{
}

nsUrlClassifierDBServiceWorker::~nsUrlClassifierDBServiceWorker()
{
  NS_ASSERTION(!mConnection,
               "Db connection not closed, leaking memory!  Call CloseDb "
               "to close the connection.");
  if (mPendingLookupLock)
    PR_DestroyLock(mPendingLookupLock);
}

nsresult
nsUrlClassifierDBServiceWorker::Init(PRInt32 gethashNoise)
{
  mGethashNoise = gethashNoise;

  // Compute database filename

  // Because we dump raw integers into the database, this database isn't
  // portable between machine types, so store it in the local profile dir.
  nsresult rv = NS_GetSpecialDirectory(NS_APP_USER_PROFILE_LOCAL_50_DIR,
                                       getter_AddRefs(mDBFile));

  if (NS_FAILED(rv)) {
    rv = NS_GetSpecialDirectory(NS_APP_USER_PROFILE_50_DIR,
                                getter_AddRefs(mDBFile));
  }

  if (NS_FAILED(rv)) return NS_ERROR_NOT_AVAILABLE;

  rv = mDBFile->Append(NS_LITERAL_STRING(DATABASE_FILENAME));
  NS_ENSURE_SUCCESS(rv, rv);

  mPendingLookupLock = PR_NewLock();
  if (!mPendingLookupLock)
    return NS_ERROR_OUT_OF_MEMORY;

  ResetUpdate();

  return NS_OK;
}

nsresult
nsUrlClassifierDBServiceWorker::QueueLookup(const nsACString& spec,
                                            nsIUrlClassifierLookupCallback* callback)
{
  nsAutoLock lock(mPendingLookupLock);

  PendingLookup* lookup = mPendingLookups.AppendElement();
  if (!lookup) return NS_ERROR_OUT_OF_MEMORY;

  lookup->mKey = spec;
  lookup->mCallback = callback;

  return NS_OK;
}

nsresult
nsUrlClassifierDBServiceWorker::GetLookupFragments(const nsACString& spec,
                                                   nsTArray<nsUrlClassifierCompleteHash>& fragments)
{
  fragments.Clear();

  nsACString::const_iterator begin, end, iter;
  spec.BeginReading(begin);
  spec.EndReading(end);

  iter = begin;
  if (!FindCharInReadable('/', iter, end)) {
    return NS_OK;
  }

  const nsCSubstring& host = Substring(begin, iter++);
  nsCAutoString path;
  path.Assign(Substring(iter, end));

  /**
   * From the protocol doc:
   * For the hostname, the client will try at most 5 different strings.  They
   * are:
   * a) The exact hostname of the url
   * b) The 4 hostnames formed by starting with the last 5 components and
   *    successivly removing the leading component.  The top-level component
   *    can be skipped.
   */
  nsCStringArray hosts;
  hosts.AppendCString(host);

  host.BeginReading(begin);
  host.EndReading(end);
  int numComponents = 0;
  while (RFindInReadable(NS_LITERAL_CSTRING("."), begin, end) &&
         numComponents < MAX_HOST_COMPONENTS) {
    // don't bother checking toplevel domains
    if (++numComponents >= 2) {
      host.EndReading(iter);
      hosts.AppendCString(Substring(end, iter));
    }
    end = begin;
    host.BeginReading(begin);
  }

  /**
   * From the protocol doc:
   * For the path, the client will also try at most 6 different strings.
   * They are:
   * a) the exact path of the url, including query parameters
   * b) the exact path of the url, without query parameters
   * c) the 4 paths formed by starting at the root (/) and
   *    successively appending path components, including a trailing
   *    slash.  This behavior should only extend up to the next-to-last
   *    path component, that is, a trailing slash should never be
   *    appended that was not present in the original url.
   */
  nsCStringArray paths;
  paths.AppendCString(path);

  path.BeginReading(iter);
  path.EndReading(end);
  if (FindCharInReadable('?', iter, end)) {
    path.BeginReading(begin);
    path = Substring(begin, iter);
    paths.AppendCString(path);
  }

  // Check an empty path (for whole-domain blacklist entries)
  paths.AppendCString(EmptyCString());

  numComponents = 1;
  path.BeginReading(begin);
  path.EndReading(end);
  iter = begin;
  while (FindCharInReadable('/', iter, end) &&
         numComponents < MAX_PATH_COMPONENTS) {
    iter++;
    paths.AppendCString(Substring(begin, iter));
    numComponents++;
  }

  for (int hostIndex = 0; hostIndex < hosts.Count(); hostIndex++) {
    for (int pathIndex = 0; pathIndex < paths.Count(); pathIndex++) {
      nsCAutoString key;
      key.Assign(*hosts[hostIndex]);
      key.Append('/');
      key.Append(*paths[pathIndex]);
      LOG(("Chking %s", key.get()));

      nsUrlClassifierCompleteHash* hash = fragments.AppendElement();
      if (!hash) return NS_ERROR_OUT_OF_MEMORY;
      hash->FromPlaintext(key, mCryptoHash);
    }
  }

  return NS_OK;
}

nsresult
nsUrlClassifierDBServiceWorker::CheckKey(const nsACString& spec,
                                         const nsUrlClassifierDomainHash& hash,
                                         nsTArray<nsUrlClassifierLookupResult>& results)
{
  mozStorageStatementScoper lookupScoper(mMainStore.LookupStatement());

  nsresult rv = mMainStore.LookupStatement()->BindBlobParameter
    (0, hash.buf, DOMAIN_LENGTH);
  NS_ENSURE_SUCCESS(rv, rv);

  nsTArray<nsUrlClassifierCompleteHash> fragments;
  PRBool haveFragments = PR_FALSE;

  PRBool exists;
  rv = mMainStore.LookupStatement()->ExecuteStep(&exists);
  NS_ENSURE_SUCCESS(rv, rv);
  while (exists) {
    if (!haveFragments) {
      rv = GetLookupFragments(spec, fragments);
      NS_ENSURE_SUCCESS(rv, rv);
      haveFragments = PR_TRUE;
    }

    nsUrlClassifierEntry entry;
    if (!mMainStore.ReadStatement(mMainStore.LookupStatement(), entry))
      return NS_ERROR_FAILURE;

    for (PRUint32 i = 0; i < fragments.Length(); i++) {
      if (entry.Match(fragments[i])) {
        // If the entry doesn't contain a complete hash, we need to
        // save it here so that it can be compared against the
        // complete hash.  However, we don't set entry.mHaveComplete
        // because it isn't a verified part of the entry yet.
        nsUrlClassifierLookupResult *result = results.AppendElement();
        if (!result)
          return NS_ERROR_OUT_OF_MEMORY;

        result->mLookupFragment = fragments[i];
        result->mEntry = entry;
        // This is a confirmed result if we matched a complete
        // fragment.
        result->mConfirmed = entry.mHaveComplete;

        // Fill in the table name.
        GetTableName(entry.mTableId, result->mTableName);
        break;
      }
    }

    rv = mMainStore.LookupStatement()->ExecuteStep(&exists);
    NS_ENSURE_SUCCESS(rv, rv);
  }

  return NS_OK;
}

/**
 * Lookup up a key in the database is a two step process:
 *
 * a) First we look for any Entries in the database that might apply to this
 *    url.  For each URL there are one or two possible domain names to check:
 *    the two-part domain name (example.com) and the three-part name
 *    (www.example.com).  We check the database for both of these.
 * b) If we find any entries, we check the list of fragments for that entry
 *    against the possible subfragments of the URL as described in the
 *    "Simplified Regular Expression Lookup" section of the protocol doc.
 */
nsresult
nsUrlClassifierDBServiceWorker::DoLookup(const nsACString& spec,
                                         nsIUrlClassifierLookupCallback* c)
{
  if (gShuttingDownThread) {
    c->LookupComplete(nsnull);
    return NS_ERROR_NOT_INITIALIZED;
  }

  nsresult rv = OpenDb();
  if (NS_FAILED(rv)) {
    c->LookupComplete(nsnull);
    return NS_ERROR_FAILURE;
  }

#if defined(PR_LOGGING)
  PRIntervalTime clockStart = 0;
  if (LOG_ENABLED()) {
    clockStart = PR_IntervalNow();
  }
#endif

  nsACString::const_iterator begin, end, iter;
  spec.BeginReading(begin);
  spec.EndReading(end);

  iter = begin;
  if (!FindCharInReadable('/', iter, end)) {
    return NS_OK;
  }

  const nsCSubstring& host = Substring(begin, iter++);

  nsAutoPtr<nsTArray<nsUrlClassifierLookupResult> > results;
  results = new nsTArray<nsUrlClassifierLookupResult>();
  if (!results) {
    c->LookupComplete(nsnull);
    return NS_ERROR_OUT_OF_MEMORY;
  }

  nsUrlClassifierDomainHash hash;

  if (IsCanonicalizedIP(host)) {
    // Don't break up the host into components
    nsCAutoString lookupHost;
    lookupHost.Assign(host);
    lookupHost.Append("/");
    hash.FromPlaintext(lookupHost, mCryptoHash);
    CheckKey(spec, hash, *results);
  } else {
    nsCStringArray hostComponents;
    hostComponents.ParseString(PromiseFlatCString(host).get(), ".");

    if (hostComponents.Count() < 2) {
      // no host or toplevel host, this won't match anything in the db
      c->LookupComplete(nsnull);
      return NS_OK;
    }

    // First check with two domain components
    PRInt32 last = hostComponents.Count() - 1;
    nsCAutoString lookupHost;
    lookupHost.Assign(*hostComponents[last - 1]);
    lookupHost.Append(".");
    lookupHost.Append(*hostComponents[last]);
    lookupHost.Append("/");
    hash.FromPlaintext(lookupHost, mCryptoHash);

    // we ignore failures from CheckKey because we'd rather try to find
    // more results than fail.
    CheckKey(spec, hash, *results);

    // Now check with three domain components
    if (hostComponents.Count() > 2) {
      nsCAutoString lookupHost2;
      lookupHost2.Assign(*hostComponents[last - 2]);
      lookupHost2.Append(".");
      lookupHost2.Append(lookupHost);
      hash.FromPlaintext(lookupHost2, mCryptoHash);

      CheckKey(spec, hash, *results);
    }
  }

#if defined(PR_LOGGING)
  if (LOG_ENABLED()) {
    PRIntervalTime clockEnd = PR_IntervalNow();
    LOG(("query took %dms\n",
         PR_IntervalToMilliseconds(clockEnd - clockStart)));
  }
#endif

  for (PRUint32 i = 0; i < results->Length(); i++) {
    if (!results->ElementAt(i).mConfirmed) {
      // We're going to be doing a gethash request, add some extra entries.
      AddNoise(results->ElementAt(i).mEntry.mId, mGethashNoise, *results);
      break;
    }
  }

  // At this point ownership of 'results' is handed to the callback.
  c->LookupComplete(results.forget());

  return NS_OK;
}

nsresult
nsUrlClassifierDBServiceWorker::HandlePendingLookups()
{
  nsAutoLock lock(mPendingLookupLock);
  while (mPendingLookups.Length() > 0) {
    PendingLookup lookup = mPendingLookups[0];
    mPendingLookups.RemoveElementAt(0);
    lock.unlock();

    DoLookup(lookup.mKey, lookup.mCallback);

    lock.lock();
  }

  return NS_OK;
}

nsresult
nsUrlClassifierDBServiceWorker::AddNoise(PRInt64 nearID,
                                         PRInt32 count,
                                         nsTArray<nsUrlClassifierLookupResult>& results)
{
  if (count < 1) {
    return NS_OK;
  }

  PRInt64 randomNum;
  nsresult rv = mMainStore.RandomNumber(&randomNum);
  NS_ENSURE_SUCCESS(rv, rv);

  PRInt32 numBefore = randomNum % count;

  nsTArray<nsUrlClassifierEntry> noiseEntries;
  rv = mMainStore.ReadNoiseEntries(nearID, numBefore, PR_TRUE, noiseEntries);
  NS_ENSURE_SUCCESS(rv, rv);

  rv = mMainStore.ReadNoiseEntries(nearID, count - numBefore, PR_FALSE, noiseEntries);
  NS_ENSURE_SUCCESS(rv, rv);

  for (PRUint32 i = 0; i < noiseEntries.Length(); i++) {
    nsUrlClassifierLookupResult *result = results.AppendElement();
    if (!result)
      return NS_ERROR_OUT_OF_MEMORY;

    result->mEntry = noiseEntries[i];
    result->mConfirmed = PR_FALSE;
    result->mNoise = PR_TRUE;

    // Fill in the table name.
    GetTableName(noiseEntries[i].mTableId, result->mTableName);
  }

  return NS_OK;
}


// Lookup a key in the db.
NS_IMETHODIMP
nsUrlClassifierDBServiceWorker::Lookup(const nsACString& spec,
                                       nsIUrlClassifierCallback* c)
{
  return HandlePendingLookups();
}

NS_IMETHODIMP
nsUrlClassifierDBServiceWorker::GetTables(nsIUrlClassifierCallback* c)
{
  if (gShuttingDownThread)
    return NS_ERROR_NOT_INITIALIZED;

  nsresult rv = OpenDb();
  if (NS_FAILED(rv)) {
    NS_ERROR("Unable to open database");
    return NS_ERROR_FAILURE;
  }

  mozStorageStatementScoper scoper(mGetTablesStatement);

  nsCAutoString response;
  PRBool hasMore;
  while (NS_SUCCEEDED(rv = mGetTablesStatement->ExecuteStep(&hasMore)) &&
         hasMore) {
    nsCAutoString val;
    mGetTablesStatement->GetUTF8String(0, val);

    if (val.IsEmpty()) {
      continue;
    }

    response.Append(val);
    response.Append(';');

    mGetTablesStatement->GetUTF8String(1, val);

    PRBool haveAdds = PR_FALSE;
    if (!val.IsEmpty()) {
      response.Append("a:");
      response.Append(val);
      haveAdds = PR_TRUE;
    }

    mGetTablesStatement->GetUTF8String(2, val);
    if (!val.IsEmpty()) {
      if (haveAdds)
        response.Append(":");

      response.Append("s:");
      response.Append(val);
    }

    response.Append('\n');
  }

  if (NS_FAILED(rv)) {
    response.Truncate();
  }

  c->HandleEvent(response);

  return rv;
}

nsresult
nsUrlClassifierDBServiceWorker::GetTableId(const nsACString& table,
                                           PRUint32* tableId)
{
  mozStorageStatementScoper findScoper(mGetTableIdStatement);

  nsresult rv = mGetTableIdStatement->BindUTF8StringParameter(0, table);
  NS_ENSURE_SUCCESS(rv, rv);

  PRBool exists;
  rv = mGetTableIdStatement->ExecuteStep(&exists);
  NS_ENSURE_SUCCESS(rv, rv);
  if (exists) {
    *tableId = mGetTableIdStatement->AsInt32(0);
    return NS_OK;
  }

  mozStorageStatementScoper insertScoper(mInsertTableIdStatement);
  rv = mInsertTableIdStatement->BindUTF8StringParameter(0, table);
  NS_ENSURE_SUCCESS(rv, rv);

  rv = mInsertTableIdStatement->Execute();
  NS_ENSURE_SUCCESS(rv, rv);

  PRInt64 rowId;
  rv = mConnection->GetLastInsertRowID(&rowId);
  NS_ENSURE_SUCCESS(rv, rv);

  if (rowId > PR_UINT32_MAX)
    return NS_ERROR_FAILURE;

  *tableId = rowId;

  return NS_OK;
}

nsresult
nsUrlClassifierDBServiceWorker::GetTableName(PRUint32 tableId,
                                             nsACString& tableName)
{
  mozStorageStatementScoper findScoper(mGetTableNameStatement);
  nsresult rv = mGetTableNameStatement->BindInt32Parameter(0, tableId);
  NS_ENSURE_SUCCESS(rv, rv);

  PRBool exists;
  rv = mGetTableNameStatement->ExecuteStep(&exists);
  NS_ENSURE_SUCCESS(rv, rv);
  if (!exists) return NS_ERROR_FAILURE;

  return mGetTableNameStatement->GetUTF8String(0, tableName);
}

nsresult
nsUrlClassifierDBServiceWorker::InflateChunk(nsACString& chunk)
{
  nsCAutoString inflated;
  char buf[4096];

  const nsPromiseFlatCString& flat = PromiseFlatCString(chunk);

  z_stream stream;
  memset(&stream, 0, sizeof(stream));
  stream.next_in = (Bytef*)flat.get();
  stream.avail_in = flat.Length();

  if (inflateInit(&stream) != Z_OK) {
    return NS_ERROR_FAILURE;
  }

  int code;
  do {
    stream.next_out = (Bytef*)buf;
    stream.avail_out = sizeof(buf);

    code = inflate(&stream, Z_NO_FLUSH);
    PRUint32 numRead = sizeof(buf) - stream.avail_out;

    if (code == Z_OK || code == Z_STREAM_END) {
      inflated.Append(buf, numRead);
    }
  } while (code == Z_OK);

  inflateEnd(&stream);

  if (code != Z_STREAM_END) {
    return NS_ERROR_FAILURE;
  }

  chunk = inflated;

  return NS_OK;
}

nsresult
nsUrlClassifierStore::DeleteEntry(nsUrlClassifierEntry& entry)
{
  if (entry.mId == -1) {
    return NS_OK;
  }

  mozStorageStatementScoper scoper(mDeleteStatement);
  mDeleteStatement->BindInt64Parameter(0, entry.mId);
  nsresult rv = mDeleteStatement->Execute();
  NS_ENSURE_SUCCESS(rv, rv);

  entry.mId = -1;

  return NS_OK;
}

nsresult
nsUrlClassifierStore::WriteEntry(nsUrlClassifierEntry& entry)
{
  PRBool newEntry = (entry.mId == -1);

  if (newEntry) {
    // The insert statement chooses a random ID for the entry, which
    // might collide.  This should be exceedingly rare, but we'll try
    // a few times, otherwise assume a real error.
    nsresult rv;
    for (PRUint32 i = 0; i < 10; i++) {
      mozStorageStatementScoper scoper(mInsertStatement);

      rv = BindStatement(entry, mInsertStatement);
      NS_ENSURE_SUCCESS(rv, rv);

      rv = mInsertStatement->Execute();
      if (NS_SUCCEEDED(rv)) {
        break;
      }
    }

    NS_ENSURE_SUCCESS(rv, rv);

    PRInt64 rowId;
    rv = mConnection->GetLastInsertRowID(&rowId);
    NS_ENSURE_SUCCESS(rv, rv);

    if (rowId > PR_UINT32_MAX) {
      return NS_ERROR_FAILURE;
    }

    entry.mId = rowId;
  }

  return NS_OK;
}

nsresult
nsUrlClassifierStore::UpdateEntry(nsUrlClassifierEntry& entry)
{
  mozStorageStatementScoper scoper(mUpdateStatement);

  NS_ENSURE_ARG(entry.mId != -1);

  nsresult rv = BindStatement(entry, mUpdateStatement);
  NS_ENSURE_SUCCESS(rv, rv);

  rv = mUpdateStatement->Execute();
  NS_ENSURE_SUCCESS(rv, rv);

  return NS_OK;
}

PRBool
nsUrlClassifierDBServiceWorker::IsCanonicalizedIP(const nsACString& host)
{
  // The canonicalization process will have left IP addresses in dotted
  // decimal with no surprises.
  PRUint32 i1, i2, i3, i4;
  char c;
  if (PR_sscanf(PromiseFlatCString(host).get(), "%u.%u.%u.%u%c",
                &i1, &i2, &i3, &i4, &c) == 4) {
    return (i1 <= 0xFF && i1 <= 0xFF && i1 <= 0xFF && i1 <= 0xFF);
  }

  return PR_FALSE;
}

nsresult
nsUrlClassifierDBServiceWorker::GetKey(const nsACString& spec,
                                       nsUrlClassifierDomainHash& hash)
{
  nsACString::const_iterator begin, end, iter;
  spec.BeginReading(begin);
  spec.EndReading(end);

  iter = begin;
  if (!FindCharInReadable('/', iter, end)) {
    return NS_OK;
  }

  const nsCSubstring& host = Substring(begin, iter);

  if (IsCanonicalizedIP(host)) {
    nsCAutoString key;
    key.Assign(host);
    key.Append("/");
    return hash.FromPlaintext(key, mCryptoHash);
  }

  nsCStringArray hostComponents;
  hostComponents.ParseString(PromiseFlatCString(host).get(), ".");

  if (hostComponents.Count() < 2)
    return NS_ERROR_FAILURE;

  PRInt32 last = hostComponents.Count() - 1;
  nsCAutoString lookupHost;

  if (hostComponents.Count() > 2) {
    lookupHost.Append(*hostComponents[last - 2]);
    lookupHost.Append(".");
  }

  lookupHost.Append(*hostComponents[last - 1]);
  lookupHost.Append(".");
  lookupHost.Append(*hostComponents[last]);
  lookupHost.Append("/");

  return hash.FromPlaintext(lookupHost, mCryptoHash);
}

nsresult
nsUrlClassifierDBServiceWorker::GetShaEntries(PRUint32 tableId,
                                              PRUint32 chunkType,
                                              PRUint32 chunkNum,
                                              PRUint32 domainSize,
                                              PRUint32 fragmentSize,
                                              nsACString& chunk,
                                              nsTArray<nsUrlClassifierEntry>& entries)
{
  PRUint32 start = 0;
  while (start + domainSize + 1 <= chunk.Length()) {
    nsUrlClassifierDomainHash domain;
    domain.Assign(Substring(chunk, start, DOMAIN_LENGTH));
    start += domainSize;

    // then there is a one-byte count of fragments
    PRUint8 numEntries = static_cast<PRUint8>(chunk[start]);
    start++;

    if (numEntries == 0) {
      // if there are no fragments, the domain itself is treated as a
      // fragment.  This will only work if domainHashSize == hashSize
      if (domainSize != fragmentSize) {
        NS_WARNING("Received 0-fragment entry where domainSize != fragmentSize");
        return NS_ERROR_FAILURE;
      }

      nsUrlClassifierEntry* entry = entries.AppendElement();
      if (!entry) return NS_ERROR_OUT_OF_MEMORY;

      entry->mKey = domain;
      entry->mTableId = tableId;
      entry->mChunkId = chunkNum;
      entry->SetHash(domain);

      if (chunkType == CHUNK_SUB) {
        if (start + 4 > chunk.Length()) {
          // there isn't as much data as there should be.
          NS_WARNING("Received a zero-entry sub chunk without an associated add.");
          return NS_ERROR_FAILURE;
        }
        const nsCSubstring& str = Substring(chunk, start, 4);
        const PRUint32 *p = reinterpret_cast<const PRUint32*>(str.BeginReading());
        entry->mAddChunkId = PR_ntohl(*p);
        if (entry->mAddChunkId == 0) {
          NS_WARNING("Received invalid chunk number.");
          return NS_ERROR_FAILURE;
        }
        start += 4;
      }
    } else {
      PRUint32 entrySize = fragmentSize;
      if (chunkType == CHUNK_SUB) {
        entrySize += 4;
      }
      if (start + (numEntries * entrySize) > chunk.Length()) {
        // there isn't as much data as they said there would be.
        NS_WARNING("Received a chunk without enough data");
        return NS_ERROR_FAILURE;
      }

      for (PRUint8 i = 0; i < numEntries; i++) {
        nsUrlClassifierEntry* entry = entries.AppendElement();
        if (!entry) return NS_ERROR_OUT_OF_MEMORY;

        entry->mKey = domain;
        entry->mTableId = tableId;
        entry->mChunkId = chunkNum;

        if (chunkType == CHUNK_SUB) {
          const nsCSubstring& str = Substring(chunk, start, 4);
          const PRUint32 *p = reinterpret_cast<const PRUint32*>(str.BeginReading());
          entry->mAddChunkId = PR_ntohl(*p);
          if (entry->mAddChunkId == 0) {
            NS_WARNING("Received invalid chunk number.");
            return NS_ERROR_FAILURE;
          }
          start += 4;
        }

        if (fragmentSize == PARTIAL_LENGTH) {
          nsUrlClassifierPartialHash hash;
          hash.Assign(Substring(chunk, start, PARTIAL_LENGTH));
          entry->SetHash(hash);
        } else if (fragmentSize == COMPLETE_LENGTH) {
          nsUrlClassifierCompleteHash hash;
          hash.Assign(Substring(chunk, start, COMPLETE_LENGTH));
          entry->SetHash(hash);
        } else {
          NS_ASSERTION(PR_FALSE, "Invalid fragment size!");
          return NS_ERROR_FAILURE;
        }

        start += fragmentSize;
      }
    }
  }

  return NS_OK;
}

nsresult
nsUrlClassifierDBServiceWorker::GetChunkEntries(const nsACString& table,
                                                PRUint32 tableId,
                                                PRUint32 chunkType,
                                                PRUint32 chunkNum,
                                                PRUint32 hashSize,
                                                nsACString& chunk,
                                                nsTArray<nsUrlClassifierEntry>& entries)
{
  nsresult rv;
  if (StringEndsWith(table, NS_LITERAL_CSTRING("-exp"))) {
    // regexp tables need to be ungzipped
    rv = InflateChunk(chunk);
    NS_ENSURE_SUCCESS(rv, rv);
  }

  if (StringEndsWith(table, NS_LITERAL_CSTRING("-shavar"))) {
    rv = GetShaEntries(tableId, chunkType, chunkNum, DOMAIN_LENGTH, hashSize,
                       chunk, entries);
    NS_ENSURE_SUCCESS(rv, rv);
  } else {
    nsCStringArray lines;
    lines.ParseString(PromiseFlatCString(chunk).get(), "\n");

    // non-hashed tables need to be hashed
    for (PRInt32 i = 0; i < lines.Count(); i++) {
      nsUrlClassifierEntry *entry = entries.AppendElement();
      if (!entry)
        return NS_ERROR_OUT_OF_MEMORY;

      nsCAutoString entryStr;
      if (chunkType == CHUNK_SUB) {
        nsCString::const_iterator begin, iter, end;
        lines[i]->BeginReading(begin);
        lines[i]->EndReading(end);
        iter = begin;
        if (!FindCharInReadable(':', iter, end) ||
            PR_sscanf(lines[i]->get(), "%d:", &entry->mAddChunkId) != 1) {
          NS_WARNING("Received sub chunk without associated add chunk.");
          return NS_ERROR_FAILURE;
        }
        iter++;
        entryStr = Substring(iter, end);
      } else {
        entryStr = *lines[i];
      }

      rv = GetKey(entryStr, entry->mKey);
      NS_ENSURE_SUCCESS(rv, rv);

      entry->mTableId = tableId;
      entry->mChunkId = chunkNum;
      if (hashSize == PARTIAL_LENGTH) {
        nsUrlClassifierPartialHash hash;
        hash.FromPlaintext(entryStr, mCryptoHash);
        entry->SetHash(hash);
      } else if (hashSize == COMPLETE_LENGTH) {
        nsUrlClassifierCompleteHash hash;
        hash.FromPlaintext(entryStr, mCryptoHash);
        entry->SetHash(hash);
      } else {
        NS_ASSERTION(PR_FALSE, "Invalid fragment size!");
        return NS_ERROR_FAILURE;
      }
    }
  }

  return NS_OK;
}

PRBool
nsUrlClassifierDBServiceWorker::ParseChunkRange(nsACString::const_iterator &begin,
                                                const nsACString::const_iterator &end,
                                                PRUint32 *first,
                                                PRUint32 *last)
{
  nsACString::const_iterator iter = begin;
  FindCharInReadable(',', iter, end);

  nsCAutoString element(Substring(begin, iter));
  begin = iter;
  if (begin != end)
    begin++;

  PRUint32 numRead = PR_sscanf(element.get(), "%u-%u", first, last);
  if (numRead == 2) {
    if (*first > *last) {
      PRUint32 tmp = *first;
      *first = *last;
      *last = tmp;
    }
    return PR_TRUE;
  }

  if (numRead == 1) {
    *last = *first;
    return PR_TRUE;
  }

  return PR_FALSE;
}

nsresult
nsUrlClassifierDBServiceWorker::ParseChunkList(const nsACString& chunkStr,
                                               nsTArray<PRUint32>& chunks)
{
  LOG(("Parsing %s", PromiseFlatCString(chunkStr).get()));

  nsACString::const_iterator begin, end;
  chunkStr.BeginReading(begin);
  chunkStr.EndReading(end);
  while (begin != end) {
    PRUint32 first, last;
    if (ParseChunkRange(begin, end, &first, &last)) {
      for (PRUint32 num = first; num <= last; num++) {
        chunks.AppendElement(num);
      }
    }
  }

  LOG(("Got %d elements.", chunks.Length()));

  return NS_OK;
}

nsresult
nsUrlClassifierDBServiceWorker::JoinChunkList(nsTArray<PRUint32>& chunks,
                                              nsCString& chunkStr)
{
  chunkStr.Truncate();
  chunks.Sort();

  PRUint32 i = 0;
  while (i < chunks.Length()) {
    if (i != 0) {
      chunkStr.Append(',');
    }
    chunkStr.AppendInt(chunks[i]);

    PRUint32 first = i;
    PRUint32 last = first;
    i++;
    while (i < chunks.Length() && chunks[i] == chunks[i - 1] + 1) {
      last = chunks[i++];
    }

    if (last != first) {
      chunkStr.Append('-');
      chunkStr.AppendInt(last);
    }
  }

  return NS_OK;
}


nsresult
nsUrlClassifierDBServiceWorker::GetChunkLists(PRUint32 tableId,
                                              nsACString& addChunks,
                                              nsACString& subChunks)
{
  addChunks.Truncate();
  subChunks.Truncate();

  mozStorageStatementScoper scoper(mGetChunkListsStatement);

  nsresult rv = mGetChunkListsStatement->BindInt32Parameter(0, tableId);
  NS_ENSURE_SUCCESS(rv, rv);

  PRBool hasMore = PR_FALSE;
  rv = mGetChunkListsStatement->ExecuteStep(&hasMore);
  NS_ENSURE_SUCCESS(rv, rv);

  if (!hasMore) {
    LOG(("Getting chunks for %d, found nothing", tableId));
    return NS_OK;
  }

  rv = mGetChunkListsStatement->GetUTF8String(0, addChunks);
  NS_ENSURE_SUCCESS(rv, rv);

  rv = mGetChunkListsStatement->GetUTF8String(1, subChunks);
  NS_ENSURE_SUCCESS(rv, rv);

  LOG(("Getting chunks for %d, got %s %s",
       tableId,
       PromiseFlatCString(addChunks).get(),
       PromiseFlatCString(subChunks).get()));

  return NS_OK;
}

nsresult
nsUrlClassifierDBServiceWorker::SetChunkLists(PRUint32 tableId,
                                              const nsACString& addChunks,
                                              const nsACString& subChunks)
{
  mozStorageStatementScoper scoper(mSetChunkListsStatement);

  mSetChunkListsStatement->BindUTF8StringParameter(0, addChunks);
  mSetChunkListsStatement->BindUTF8StringParameter(1, subChunks);
  mSetChunkListsStatement->BindInt32Parameter(2, tableId);
  nsresult rv = mSetChunkListsStatement->Execute();
  NS_ENSURE_SUCCESS(rv, rv);

  return NS_OK;
}

nsresult
nsUrlClassifierDBServiceWorker::CacheChunkLists(PRUint32 tableId,
                                                PRBool parseAdds,
                                                PRBool parseSubs)
{
  nsresult rv;

  if (mHaveCachedLists && mCachedListsTable != tableId) {
    rv = FlushChunkLists();
    NS_ENSURE_SUCCESS(rv, rv);
  }

  if (!mHaveCachedLists) {
    rv = GetChunkLists(tableId, mCachedAddsStr, mCachedSubsStr);
    NS_ENSURE_SUCCESS(rv, rv);

    mHaveCachedLists = PR_TRUE;
    mCachedListsTable = tableId;
  }

  if (parseAdds && !mHaveCachedAddChunks) {
    ParseChunkList(mCachedAddsStr, mCachedAddChunks);
    mHaveCachedAddChunks = PR_TRUE;
  }

  if (parseSubs && !mHaveCachedSubChunks) {
    ParseChunkList(mCachedSubsStr, mCachedSubChunks);
    mHaveCachedSubChunks = PR_TRUE;
  }

  return NS_OK;
}

nsresult
nsUrlClassifierDBServiceWorker::FlushChunkLists()
{
  if (!mHaveCachedLists) {
    return NS_OK;
  }

  if (mHaveCachedAddChunks) {
    JoinChunkList(mCachedAddChunks, mCachedAddsStr);
  }

  if (mHaveCachedSubChunks) {
    JoinChunkList(mCachedSubChunks, mCachedSubsStr);
  }

  nsresult rv = SetChunkLists(mCachedListsTable,
                              mCachedAddsStr, mCachedSubsStr);

  // clear out the cache before checking/returning the error here.
  ClearCachedChunkLists();

  return rv;
}

void
nsUrlClassifierDBServiceWorker::ClearCachedChunkLists()
{
  mCachedAddsStr.Truncate();
  mCachedSubsStr.Truncate();
  mCachedListsTable = PR_UINT32_MAX;
  mHaveCachedLists = PR_FALSE;

  mCachedAddChunks.Clear();
  mHaveCachedAddChunks = PR_FALSE;

  mCachedSubChunks.Clear();
  mHaveCachedSubChunks = PR_FALSE;
}

nsresult
nsUrlClassifierDBServiceWorker::AddChunk(PRUint32 tableId,
                                         PRUint32 chunkNum,
                                         nsTArray<nsUrlClassifierEntry>& entries)
{
#if defined(PR_LOGGING)
  PRIntervalTime clockStart = 0;
  if (LOG_ENABLED()) {
    clockStart = PR_IntervalNow();
  }
#endif

  LOG(("Adding %d entries to chunk %d", entries.Length(), chunkNum));

  nsresult rv = CacheChunkLists(tableId, PR_TRUE, PR_FALSE);
  NS_ENSURE_SUCCESS(rv, rv);
  mCachedAddChunks.AppendElement(chunkNum);

  nsTArray<PRUint32> entryIDs;

  nsAutoTArray<nsUrlClassifierEntry, 5> subEntries;
  nsUrlClassifierDomainHash lastKey;

  for (PRUint32 i = 0; i < entries.Length(); i++) {
    nsUrlClassifierEntry& thisEntry = entries[i];

    HandlePendingLookups();

    if (i == 0 || lastKey != thisEntry.mKey) {
      subEntries.Clear();
      rv = mPendingSubStore.ReadSubEntries(thisEntry, subEntries);
      NS_ENSURE_SUCCESS(rv, rv);
      lastKey = thisEntry.mKey;
    }

    PRBool writeEntry = PR_TRUE;
    for (PRUint32 j = 0; j < subEntries.Length(); j++) {
      if (thisEntry.SubMatch(subEntries[j])) {
        rv = mPendingSubStore.DeleteEntry(subEntries[j]);
        NS_ENSURE_SUCCESS(rv, rv);
        subEntries.RemoveElementAt(j);

        writeEntry = PR_FALSE;
        break;
      }
    }

    HandlePendingLookups();

    if (writeEntry) {
      rv = mMainStore.WriteEntry(thisEntry);
      NS_ENSURE_SUCCESS(rv, rv);
    }
  }

#if defined(PR_LOGGING)
  if (LOG_ENABLED()) {
    PRIntervalTime clockEnd = PR_IntervalNow();
    LOG(("adding chunk %d took %dms\n", chunkNum,
         PR_IntervalToMilliseconds(clockEnd - clockStart)));
  }
#endif

  return rv;
}

nsresult
nsUrlClassifierStore::Expire(PRUint32 tableId, PRUint32 chunkNum)
{
  LOG(("Expiring chunk %d\n", chunkNum));

  mozStorageStatementScoper expireScoper(mExpireStatement);

  nsresult rv = mExpireStatement->BindInt32Parameter(0, tableId);
  NS_ENSURE_SUCCESS(rv, rv);
  rv = mExpireStatement->BindInt32Parameter(1, chunkNum);
  NS_ENSURE_SUCCESS(rv, rv);

  mWorker->HandlePendingLookups();

  rv = mExpireStatement->Execute();
  NS_ENSURE_SUCCESS(rv, rv);

  return NS_OK;
}

nsresult
nsUrlClassifierDBServiceWorker::ExpireAdd(PRUint32 tableId,
                                          PRUint32 chunkNum)
{
  nsresult rv = CacheChunkLists(tableId, PR_TRUE, PR_FALSE);
  NS_ENSURE_SUCCESS(rv, rv);
  mCachedAddChunks.RemoveElement(chunkNum);

  return mMainStore.Expire(tableId, chunkNum);
}

nsresult
nsUrlClassifierDBServiceWorker::SubChunk(PRUint32 tableId,
                                         PRUint32 chunkNum,
                                         nsTArray<nsUrlClassifierEntry>& entries)
{
  nsresult rv = CacheChunkLists(tableId, PR_FALSE, PR_TRUE);
  mCachedSubChunks.AppendElement(chunkNum);

  nsAutoTArray<nsUrlClassifierEntry, 5> existingEntries;
  nsUrlClassifierDomainHash lastKey;

  for (PRUint32 i = 0; i < entries.Length(); i++) {
    nsUrlClassifierEntry& thisEntry = entries[i];

    HandlePendingLookups();

    if (i == 0 || lastKey != thisEntry.mKey) {
      existingEntries.Clear();
      rv = mMainStore.ReadEntries(thisEntry.mKey, thisEntry.mTableId,
                                  thisEntry.mAddChunkId, existingEntries);
      NS_ENSURE_SUCCESS(rv, rv);
      lastKey = thisEntry.mKey;
    }

    PRUint32 writeEntry = PR_TRUE;
    for (PRUint32 j = 0; j < existingEntries.Length(); j++) {
      if (existingEntries[j].SubMatch(thisEntry)) {
        rv = mMainStore.DeleteEntry(existingEntries[j]);
        NS_ENSURE_SUCCESS(rv, rv);
        existingEntries.RemoveElementAt(j);
        writeEntry = PR_FALSE;
        break;
      }
    }

    if (writeEntry) {
      // Save this entry in the pending subtraction store.
      rv = mPendingSubStore.WriteEntry(thisEntry);
      NS_ENSURE_SUCCESS(rv, rv);
    }
  }

  return NS_OK;
}

nsresult
nsUrlClassifierDBServiceWorker::ExpireSub(PRUint32 tableId, PRUint32 chunkNum)
{
  nsresult rv = CacheChunkLists(tableId, PR_FALSE, PR_TRUE);
  NS_ENSURE_SUCCESS(rv, rv);
  mCachedSubChunks.RemoveElement(chunkNum);

  return mPendingSubStore.Expire(tableId, chunkNum);
}

nsresult
nsUrlClassifierDBServiceWorker::ProcessChunk(PRBool* done)
{
  // wait until the chunk has been read
  if (mPendingStreamUpdate.Length() < static_cast<PRUint32>(mChunkLen)) {
    *done = PR_TRUE;
    return NS_OK;
  }

  nsCAutoString chunk;
  chunk.Assign(Substring(mPendingStreamUpdate, 0, mChunkLen));
  mPendingStreamUpdate = Substring(mPendingStreamUpdate, mChunkLen);

  LOG(("Handling a chunk sized %d", chunk.Length()));

  nsTArray<nsUrlClassifierEntry> entries;
  nsresult rv = GetChunkEntries(mUpdateTable, mUpdateTableId, mChunkType,
                                mChunkNum, mHashSize, chunk, entries);
  NS_ENSURE_SUCCESS(rv, rv);

  if (mChunkType == CHUNK_ADD) {
    rv = AddChunk(mUpdateTableId, mChunkNum, entries);
  } else {
    rv = SubChunk(mUpdateTableId, mChunkNum, entries);
  }

  mState = STATE_LINE;
  *done = PR_FALSE;

  return rv;
}

nsresult
nsUrlClassifierDBServiceWorker::ProcessResponseLines(PRBool* done)
{
  PRUint32 cur = 0;
  PRInt32 next;

  nsresult rv;
  // We will run to completion unless we find a chunk line
  *done = PR_TRUE;

  nsACString& updateString = mPendingStreamUpdate;

  while(cur < updateString.Length() &&
        (next = updateString.FindChar('\n', cur)) != kNotFound) {
    const nsCSubstring& line = Substring(updateString, cur, next - cur);
    cur = next + 1;

    LOG(("Processing %s\n", PromiseFlatCString(line).get()));

    if (mHMAC && mServerMAC.IsEmpty()) {
      // If we did not receive a server MAC during BeginStream(), we
      // require the first line of the update to be either a MAC or
      // a request to rekey.
      if (StringBeginsWith(line, NS_LITERAL_CSTRING("m:"))) {
        mServerMAC = Substring(line, 2);
        nsUrlClassifierUtils::UnUrlsafeBase64(mServerMAC);

        // The remainder of the pending update needs to be digested.
        const nsCSubstring &toDigest = Substring(updateString, cur);
        rv = mHMAC->Update(reinterpret_cast<const PRUint8*>(toDigest.BeginReading()),
                           toDigest.Length());
        NS_ENSURE_SUCCESS(rv, rv);
      } else if (line.EqualsLiteral("e:pleaserekey")) {
        mUpdateObserver->RekeyRequested();
      } else {
        LOG(("No MAC specified!"));
        return NS_ERROR_FAILURE;
      }
    } else if (StringBeginsWith(line, NS_LITERAL_CSTRING("n:"))) {
      if (PR_sscanf(PromiseFlatCString(line).get(), "n:%d",
                    &mUpdateWait) != 1) {
        LOG(("Error parsing n: field: %s", PromiseFlatCString(line).get()));
        mUpdateWait = 0;
      }
    } else if (line.EqualsLiteral("r:pleasereset")) {
      mResetRequested = PR_TRUE;
    } else if (line.EqualsLiteral("e:pleaserekey")) {
      mUpdateObserver->RekeyRequested();
    } else if (StringBeginsWith(line, NS_LITERAL_CSTRING("i:"))) {
      mUpdateTable.Assign(Substring(line, 2));
      GetTableId(mUpdateTable, &mUpdateTableId);
      LOG(("update table: '%s' (%d)", mUpdateTable.get(), mUpdateTableId));
    } else if (StringBeginsWith(line, NS_LITERAL_CSTRING("u:"))) {
      if (!mPrimaryStream) {
        LOG(("Forwarded update tried to add its own forwarded update."));
        return NS_ERROR_FAILURE;
      }

      const nsCSubstring& data = Substring(line, 2);
      if (mHMAC) {
        // We're expecting MACs alongside any url forwards.
        nsCSubstring::const_iterator begin, end, sepBegin, sepEnd;
        data.BeginReading(begin);
        sepBegin = begin;

        data.EndReading(end);
        sepEnd = end;

        if (!RFindInReadable(NS_LITERAL_CSTRING(","), sepBegin, sepEnd)) {
          NS_WARNING("No MAC specified for a redirect in a request that expects a MAC");
          return NS_ERROR_FAILURE;
        }

        nsCString serverMAC(Substring(sepEnd, end));
        nsUrlClassifierUtils::UnUrlsafeBase64(serverMAC);
        mUpdateObserver->UpdateUrlRequested(Substring(begin, sepBegin),
                                            mUpdateTable,
                                            serverMAC);
      } else {
        // We didn't ask for a MAC, none should have been specified.
        mUpdateObserver->UpdateUrlRequested(data, mUpdateTable,
                                            NS_LITERAL_CSTRING(""));
      }
    } else if (StringBeginsWith(line, NS_LITERAL_CSTRING("a:")) ||
               StringBeginsWith(line, NS_LITERAL_CSTRING("s:"))) {
      mState = STATE_CHUNK;
      char command;
      if (PR_sscanf(PromiseFlatCString(line).get(),
                    "%c:%d:%d:%d", &command, &mChunkNum, &mHashSize, &mChunkLen) != 4) {
        return NS_ERROR_FAILURE;
      }

      if (mChunkLen > MAX_CHUNK_SIZE) {
        return NS_ERROR_FAILURE;
      }

      if (!(mHashSize == PARTIAL_LENGTH || mHashSize == COMPLETE_LENGTH)) {
        NS_WARNING("Invalid hash size specified in update.");
        return NS_ERROR_FAILURE;
      }

      mChunkType = (command == 'a') ? CHUNK_ADD : CHUNK_SUB;

      // Done parsing lines, move to chunk state now
      *done = PR_FALSE;
      break;
    } else if (StringBeginsWith(line, NS_LITERAL_CSTRING("ad:"))) {
      const nsCSubstring &list = Substring(line, 3);
      nsACString::const_iterator begin, end;
      list.BeginReading(begin);
      list.EndReading(end);
      while (begin != end) {
        PRUint32 first, last;
        if (ParseChunkRange(begin, end, &first, &last)) {
          for (PRUint32 num = first; num <= last; num++) {
            rv = ExpireAdd(mUpdateTableId, num);
            NS_ENSURE_SUCCESS(rv, rv);
          }
        } else {
          return NS_ERROR_FAILURE;
        }
      }
    } else if (StringBeginsWith(line, NS_LITERAL_CSTRING("sd:"))) {
      const nsCSubstring &list = Substring(line, 3);
      nsACString::const_iterator begin, end;
      list.BeginReading(begin);
      list.EndReading(end);
      while (begin != end) {
        PRUint32 first, last;
        if (ParseChunkRange(begin, end, &first, &last)) {
          for (PRUint32 num = first; num <= last; num++) {
            rv = ExpireSub(mUpdateTableId, num);
            NS_ENSURE_SUCCESS(rv, rv);
          }
        } else {
          return NS_ERROR_FAILURE;
        }
      }
    } else {
      LOG(("ignoring unknown line: '%s'", PromiseFlatCString(line).get()));
    }
  }

  mPendingStreamUpdate = Substring(updateString, cur);

  return NS_OK;
}

void
nsUrlClassifierDBServiceWorker::ResetStream()
{
  mState = STATE_LINE;
  mChunkNum = 0;
  mHashSize = 0;
  mChunkLen = 0;
  mInStream = PR_FALSE;
  mPrimaryStream = PR_FALSE;
  mUpdateTable.Truncate();
  mPendingStreamUpdate.Truncate();
  mServerMAC.Truncate();
  mHMAC = nsnull;
}

void
nsUrlClassifierDBServiceWorker::ResetUpdate()
{
  mUpdateWait = 0;
  mUpdateStatus = NS_OK;
  mUpdateObserver = nsnull;
  mUpdateClientKey.Truncate();
  mResetRequested = PR_FALSE;
}

NS_IMETHODIMP
nsUrlClassifierDBServiceWorker::SetHashCompleter(const nsACString &tableName,
                                                 nsIUrlClassifierHashCompleter *completer)
{
  return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP
nsUrlClassifierDBServiceWorker::BeginUpdate(nsIUrlClassifierUpdateObserver *observer,
                                            const nsACString &clientKey)
{
  if (gShuttingDownThread)
    return NS_ERROR_NOT_INITIALIZED;

  NS_ENSURE_STATE(!mUpdateObserver);

  nsresult rv = OpenDb();
  if (NS_FAILED(rv)) {
    NS_ERROR("Unable to open database");
    return NS_ERROR_FAILURE;
  }

  PRBool transaction;
  rv = mConnection->GetTransactionInProgress(&transaction);
  if (NS_FAILED(rv)) {
    mUpdateStatus = rv;
    return rv;
  }

  if (transaction) {
    NS_WARNING("Transaction already in progress in nsUrlClassifierDBServiceWorker::BeginUpdate.  Cancelling update.");
    mUpdateStatus = NS_ERROR_FAILURE;
    return rv;
  }

  rv = mConnection->BeginTransaction();
  if (NS_FAILED(rv)) {
    mUpdateStatus = rv;
    return rv;
  }

  mUpdateObserver = observer;

  if (!clientKey.IsEmpty()) {
    rv = nsUrlClassifierUtils::DecodeClientKey(clientKey, mUpdateClientKey);
    NS_ENSURE_SUCCESS(rv, rv);
  }

  // The first stream in an update is the only stream that may request
  // forwarded updates.
  mPrimaryStream = PR_TRUE;

  return NS_OK;
}

NS_IMETHODIMP
nsUrlClassifierDBServiceWorker::BeginStream(const nsACString &table,
                                            const nsACString &serverMAC)
{
  if (gShuttingDownThread)
    return NS_ERROR_NOT_INITIALIZED;

  NS_ENSURE_STATE(mUpdateObserver);
  NS_ENSURE_STATE(!mInStream);

  mInStream = PR_TRUE;

  nsresult rv;

  // If we're expecting a MAC, create the nsICryptoHMAC component now.
  if (!mUpdateClientKey.IsEmpty()) {
    mHMAC = do_CreateInstance(NS_CRYPTO_HMAC_CONTRACTID, &rv);
    if (NS_FAILED(rv)) {
      NS_WARNING("Failed to create nsICryptoHMAC instance");
      mUpdateStatus = rv;
      return mUpdateStatus;
    }
    NS_ENSURE_SUCCESS(rv, rv);

    rv = mHMAC->Init(nsICryptoHMAC::SHA1,
                     reinterpret_cast<const PRUint8*>(mUpdateClientKey.BeginReading()),
                     mUpdateClientKey.Length());
    if (NS_FAILED(rv)) {
      NS_WARNING("Failed to initialize nsICryptoHMAC instance");
      mUpdateStatus = rv;
      return mUpdateStatus;
    }
  }

  mServerMAC = serverMAC;

  if (!table.IsEmpty()) {
    mUpdateTable = table;
    GetTableId(mUpdateTable, &mUpdateTableId);
    LOG(("update table: '%s' (%d)", mUpdateTable.get(), mUpdateTableId));
  }

  return NS_OK;
}

/**
 * Updating the database:
 *
 * The Update() method takes a series of chunks seperated with control data,
 * as described in
 * http://code.google.com/p/google-safe-browsing/wiki/Protocolv2Spec
 *
 * It will iterate through the control data until it reaches a chunk.  By
 * the time it reaches a chunk, it should have received
 * a) the table to which this chunk applies
 * b) the type of chunk (add, delete, expire add, expire delete).
 * c) the chunk ID
 * d) the length of the chunk.
 *
 * For add and subtract chunks, it needs to read the chunk data (expires
 * don't have any data).  Chunk data is a list of URI fragments whose
 * encoding depends on the type of table (which is indicated by the end
 * of the table name):
 * a) tables ending with -exp are a zlib-compressed list of URI fragments
 *    separated by newlines.
 * b) tables ending with -sha128 have the form
 *    [domain][N][frag0]...[fragN]
 *       16    1   16        16
 *    If N is 0, the domain is reused as a fragment.
 * c) any other tables are assumed to be a plaintext list of URI fragments
 *    separated by newlines.
 *
 * Update() can be fed partial data;  It will accumulate data until there is
 * enough to act on.  Finish() should be called when there will be no more
 * data.
 */
NS_IMETHODIMP
nsUrlClassifierDBServiceWorker::UpdateStream(const nsACString& chunk)
{
  if (gShuttingDownThread)
    return NS_ERROR_NOT_INITIALIZED;

  NS_ENSURE_STATE(mInStream);

  HandlePendingLookups();

  LOG(("Update from Stream."));
  nsresult rv = OpenDb();
  if (NS_FAILED(rv)) {
    NS_ERROR("Unable to open database");
    return NS_ERROR_FAILURE;
  }

  // if something has gone wrong during this update, just throw it away
  if (NS_FAILED(mUpdateStatus)) {
    return mUpdateStatus;
  }

  if (mHMAC && !mServerMAC.IsEmpty()) {
    rv = mHMAC->Update(reinterpret_cast<const PRUint8*>(chunk.BeginReading()),
                       chunk.Length());
    if (NS_FAILED(rv)) {
      mUpdateStatus = rv;
      return mUpdateStatus;
    }
  }

  LOG(("Got %s\n", PromiseFlatCString(chunk).get()));

  mPendingStreamUpdate.Append(chunk);

  PRBool done = PR_FALSE;
  while (!done) {
    if (mState == STATE_CHUNK) {
      rv = ProcessChunk(&done);
    } else {
      rv = ProcessResponseLines(&done);
    }
    if (NS_FAILED(rv)) {
      mUpdateStatus = rv;
      return rv;
    }
  }

  return NS_OK;
}

NS_IMETHODIMP
nsUrlClassifierDBServiceWorker::FinishStream()
{
  if (gShuttingDownThread)
    return NS_ERROR_NOT_INITIALIZED;

  NS_ENSURE_STATE(mInStream);
  NS_ENSURE_STATE(mUpdateObserver);

  if (NS_SUCCEEDED(mUpdateStatus) && mHMAC) {
    nsCAutoString clientMAC;
    mHMAC->Finish(PR_TRUE, clientMAC);

    if (clientMAC != mServerMAC) {
      NS_WARNING("Invalid update MAC!");
      LOG(("Invalid update MAC: expected %s, got %s",
           mServerMAC.get(), clientMAC.get()));
      mUpdateStatus = NS_ERROR_FAILURE;
    }
  }

  mUpdateObserver->StreamFinished(mUpdateStatus);

  ResetStream();

  return NS_OK;
}

NS_IMETHODIMP
nsUrlClassifierDBServiceWorker::FinishUpdate()
{
  if (gShuttingDownThread)
    return NS_ERROR_NOT_INITIALIZED;

  NS_ENSURE_STATE(!mInStream);
  NS_ENSURE_STATE(mUpdateObserver);

  if (NS_SUCCEEDED(mUpdateStatus)) {
    mUpdateStatus = FlushChunkLists();
  }

  nsCAutoString arg;
  if (NS_SUCCEEDED(mUpdateStatus)) {
    mUpdateStatus = mConnection->CommitTransaction();
  } else {
    mConnection->RollbackTransaction();
  }

  if (NS_SUCCEEDED(mUpdateStatus)) {
    mUpdateObserver->UpdateSuccess(mUpdateWait);
  } else {
    mUpdateObserver->UpdateError(mUpdateStatus);
  }

  // ResetUpdate() clears mResetRequested...
  PRBool resetRequested = mResetRequested;

  ResetUpdate();

  // It's important that we only reset the database if the update was
  // successful, otherwise unauthenticated updates could cause a
  // database reset.
  if (NS_SUCCEEDED(mUpdateStatus) && resetRequested) {
    ResetDatabase();
  }

  return NS_OK;
}

NS_IMETHODIMP
nsUrlClassifierDBServiceWorker::ResetDatabase()
{
  LOG(("nsUrlClassifierDBServiceWorker::ResetDatabase [%p]", this));
  ClearCachedChunkLists();

  nsresult rv = CloseDb();
  NS_ENSURE_SUCCESS(rv, rv);

  mDBFile->Remove(PR_FALSE);

  return NS_OK;
}

NS_IMETHODIMP
nsUrlClassifierDBServiceWorker::CancelUpdate()
{
  LOG(("CancelUpdate"));

  if (mUpdateObserver) {
    mUpdateStatus = NS_BINDING_ABORTED;

    ClearCachedChunkLists();
    mConnection->RollbackTransaction();
    mUpdateObserver->UpdateError(mUpdateStatus);

    ResetStream();
    ResetUpdate();
  }

  return NS_OK;
}

// Allows the main thread to delete the connection which may be in
// a background thread.
// XXX This could be turned into a single shutdown event so the logic
// is simpler in nsUrlClassifierDBService::Shutdown.
NS_IMETHODIMP
nsUrlClassifierDBServiceWorker::CloseDb()
{
  if (mConnection) {
    CancelUpdate();

    mMainStore.Close();
    mPendingSubStore.Close();

    mGetChunkListsStatement = nsnull;
    mSetChunkListsStatement = nsnull;

    mGetTablesStatement = nsnull;
    mGetTableIdStatement = nsnull;
    mGetTableNameStatement = nsnull;
    mInsertTableIdStatement = nsnull;

    mConnection = nsnull;
    LOG(("urlclassifier db closed\n"));
  }

  mCryptoHash = nsnull;

  return NS_OK;
}

NS_IMETHODIMP
nsUrlClassifierDBServiceWorker::CacheCompletions(nsTArray<nsUrlClassifierLookupResult> *results)
{
  LOG(("nsUrlClassifierDBServiceWorker::CacheCompletions [%p]", this));

  nsAutoPtr<nsTArray<nsUrlClassifierLookupResult> > resultsPtr(results);

  // Start a new transaction.  If a transaction is open for an update
  // this will be a noop, and this cache will be included in the
  // update's transaction.
  mozStorageTransaction trans(mConnection, PR_TRUE);

  for (PRUint32 i = 0; i < results->Length(); i++) {
    nsUrlClassifierLookupResult& result = results->ElementAt(i);
    // Failing to update here shouldn't be fatal (and might be common,
    // if we're updating entries that were removed since they were
    // returned after a lookup).
    mMainStore.UpdateEntry(result.mEntry);
  }

  return NS_OK;
}

nsresult
nsUrlClassifierDBServiceWorker::OpenDb()
{
  // Connection already open, don't do anything.
  if (mConnection)
    return NS_OK;

  LOG(("Opening db\n"));

  nsresult rv;
  // open the connection
  nsCOMPtr<mozIStorageService> storageService =
    do_GetService(MOZ_STORAGE_SERVICE_CONTRACTID, &rv);
  NS_ENSURE_SUCCESS(rv, rv);

  PRBool exists;
  rv = mDBFile->Exists(&exists);
  NS_ENSURE_SUCCESS(rv, rv);
  PRBool newDB = !exists;

  nsCOMPtr<mozIStorageConnection> connection;
  rv = storageService->OpenDatabase(mDBFile, getter_AddRefs(connection));
  NS_ENSURE_SUCCESS(rv, rv);

  PRBool ready;
  connection->GetConnectionReady(&ready);
  if (!ready) {
    // delete the db and try opening again
    rv = mDBFile->Remove(PR_FALSE);
    NS_ENSURE_SUCCESS(rv, rv);

    newDB = PR_TRUE;

    rv = storageService->OpenDatabase(mDBFile, getter_AddRefs(connection));
    NS_ENSURE_SUCCESS(rv, rv);
  }

  if (!newDB) {
    PRInt32 databaseVersion;
    rv = connection->GetSchemaVersion(&databaseVersion);
    NS_ENSURE_SUCCESS(rv, rv);

    if (databaseVersion != IMPLEMENTATION_VERSION) {
      LOG(("Incompatible database, removing."));

      rv = connection->Close();
      NS_ENSURE_SUCCESS(rv, rv);

      rv = mDBFile->Remove(PR_FALSE);
      NS_ENSURE_SUCCESS(rv, rv);

      newDB = PR_TRUE;

      rv = storageService->OpenDatabase(mDBFile, getter_AddRefs(connection));
      NS_ENSURE_SUCCESS(rv, rv);
    }
  }

  if (newDB) {
    rv = connection->SetSchemaVersion(IMPLEMENTATION_VERSION);
    NS_ENSURE_SUCCESS(rv, rv);
  }

  rv = connection->ExecuteSimpleSQL(NS_LITERAL_CSTRING("PRAGMA synchronous=OFF"));
  NS_ENSURE_SUCCESS(rv, rv);

  rv = connection->ExecuteSimpleSQL(NS_LITERAL_CSTRING("PRAGMA page_size=4096"));
  NS_ENSURE_SUCCESS(rv, rv);

  rv = connection->ExecuteSimpleSQL(NS_LITERAL_CSTRING("PRAGMA default_page_size=4096"));
  NS_ENSURE_SUCCESS(rv, rv);

  // Create the table
  rv = MaybeCreateTables(connection);
  NS_ENSURE_SUCCESS(rv, rv);

  rv = mMainStore.Init(this, connection,
                       NS_LITERAL_CSTRING("moz_classifier"));
  NS_ENSURE_SUCCESS(rv, rv);

  rv = mPendingSubStore.Init(this, connection,
                             NS_LITERAL_CSTRING("moz_subs"));
  NS_ENSURE_SUCCESS(rv, rv);

  rv = connection->CreateStatement
         (NS_LITERAL_CSTRING("SELECT add_chunks, sub_chunks FROM moz_tables"
                             " WHERE id=?1"),
          getter_AddRefs(mGetChunkListsStatement));
  NS_ENSURE_SUCCESS(rv, rv);

  rv = connection->CreateStatement
         (NS_LITERAL_CSTRING("UPDATE moz_tables"
                             " SET add_chunks=?1, sub_chunks=?2"
                             " WHERE id=?3"),
          getter_AddRefs(mSetChunkListsStatement));
  NS_ENSURE_SUCCESS(rv, rv);

  rv = connection->CreateStatement
         (NS_LITERAL_CSTRING("SELECT name, add_chunks, sub_chunks"
                             " FROM moz_tables"),
          getter_AddRefs(mGetTablesStatement));
  NS_ENSURE_SUCCESS(rv, rv);

  rv = connection->CreateStatement
    (NS_LITERAL_CSTRING("SELECT id FROM moz_tables"
                        " WHERE name = ?1"),
     getter_AddRefs(mGetTableIdStatement));
  NS_ENSURE_SUCCESS(rv, rv);

  rv = connection->CreateStatement
    (NS_LITERAL_CSTRING("SELECT name FROM moz_tables"
                        " WHERE id = ?1"),
     getter_AddRefs(mGetTableNameStatement));
  NS_ENSURE_SUCCESS(rv, rv);

  rv = connection->CreateStatement
    (NS_LITERAL_CSTRING("INSERT INTO moz_tables(id, name, add_chunks, sub_chunks)"
                        " VALUES (null, ?1, null, null)"),
     getter_AddRefs(mInsertTableIdStatement));
  NS_ENSURE_SUCCESS(rv, rv);

  mConnection = connection;

  mCryptoHash = do_CreateInstance(NS_CRYPTO_HASH_CONTRACTID, &rv);
  NS_ENSURE_SUCCESS(rv, rv);

  return NS_OK;
}

nsresult
nsUrlClassifierDBServiceWorker::MaybeCreateTables(mozIStorageConnection* connection)
{
  LOG(("MaybeCreateTables\n"));

  nsresult rv = connection->ExecuteSimpleSQL(
    NS_LITERAL_CSTRING("CREATE TABLE IF NOT EXISTS moz_classifier"
                       " (id INTEGER PRIMARY KEY,"
                       "  domain BLOB,"
                       "  partial_data BLOB,"
                       "  complete_data BLOB,"
                       "  chunk_id INTEGER,"
                       "  table_id INTEGER)"));
  NS_ENSURE_SUCCESS(rv, rv);

  rv = connection->ExecuteSimpleSQL(
    NS_LITERAL_CSTRING("CREATE INDEX IF NOT EXISTS"
                       " moz_classifier_domain_index"
                       " ON moz_classifier(domain)"));
  NS_ENSURE_SUCCESS(rv, rv);

  rv = connection->ExecuteSimpleSQL(
    NS_LITERAL_CSTRING("CREATE INDEX IF NOT EXISTS"
                       " moz_classifier_chunk_index"
                       " ON moz_classifier(chunk_id)"));
  NS_ENSURE_SUCCESS(rv, rv);

  rv = connection->ExecuteSimpleSQL(
    NS_LITERAL_CSTRING("CREATE TABLE IF NOT EXISTS moz_subs"
                       " (id INTEGER PRIMARY KEY,"
                       "  domain BLOB,"
                       "  partial_data BLOB,"
                       "  complete_data BLOB,"
                       "  chunk_id INTEGER,"
                       "  table_id INTEGER,"
                       "  add_chunk_id INTEGER)"));
  NS_ENSURE_SUCCESS(rv, rv);

  rv = connection->ExecuteSimpleSQL(
    NS_LITERAL_CSTRING("CREATE INDEX IF NOT EXISTS"
                       " moz_subs_domain_index"
                       " ON moz_subs(domain)"));
  NS_ENSURE_SUCCESS(rv, rv);

  rv = connection->ExecuteSimpleSQL(
    NS_LITERAL_CSTRING("CREATE INDEX IF NOT EXISTS"
                       " moz_subs_chunk_index"
                       " ON moz_subs(chunk_id)"));
  NS_ENSURE_SUCCESS(rv, rv);

  rv = connection->ExecuteSimpleSQL(
    NS_LITERAL_CSTRING("CREATE TABLE IF NOT EXISTS moz_tables"
                       " (id INTEGER PRIMARY KEY,"
                       "  name TEXT,"
                       "  add_chunks TEXT,"
                       "  sub_chunks TEXT);"));
  NS_ENSURE_SUCCESS(rv, rv);

  return rv;
}

// -------------------------------------------------------------------------
// nsUrlClassifierLookupCallback
//
// This class takes the results of a lookup found on the worker thread
// and handles any necessary partial hash expansions before calling
// the client callback.

class nsUrlClassifierLookupCallback : public nsIUrlClassifierLookupCallback
                                    , public nsIUrlClassifierHashCompleterCallback
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIURLCLASSIFIERLOOKUPCALLBACK
  NS_DECL_NSIURLCLASSIFIERHASHCOMPLETERCALLBACK

  nsUrlClassifierLookupCallback(nsUrlClassifierDBService *dbservice,
                                nsIUrlClassifierCallback *c)
    : mDBService(dbservice)
    , mResults(nsnull)
    , mPendingCompletions(0)
    , mCallback(c)
    {}

private:
  nsresult HandleResults();

  nsRefPtr<nsUrlClassifierDBService> mDBService;
  nsAutoPtr<nsTArray<nsUrlClassifierLookupResult> > mResults;

  // Completed results to send back to the worker for caching.
  nsAutoPtr<nsTArray<nsUrlClassifierLookupResult> > mCacheResults;

  PRUint32 mPendingCompletions;
  nsCOMPtr<nsIUrlClassifierCallback> mCallback;
};

NS_IMPL_THREADSAFE_ISUPPORTS2(nsUrlClassifierLookupCallback,
                              nsIUrlClassifierLookupCallback,
                              nsIUrlClassifierHashCompleterCallback)

NS_IMETHODIMP
nsUrlClassifierLookupCallback::LookupComplete(nsTArray<nsUrlClassifierLookupResult>* results)
{
  NS_ASSERTION(mResults == nsnull,
               "Should only get one set of results per nsUrlClassifierLookupCallback!");

  if (!results) {
    HandleResults();
    return NS_OK;
  }

  mResults = results;
  mResults->Sort();

  // Check the results for partial matches.  Partial matches will need to be
  // completed.
  for (PRUint32 i = 0; i < results->Length(); i++) {
    nsUrlClassifierLookupResult& result = results->ElementAt(i);
    if (!result.mConfirmed) {
      nsCOMPtr<nsIUrlClassifierHashCompleter> completer;
      if (mDBService->GetCompleter(result.mTableName,
                                   getter_AddRefs(completer))) {
        nsCAutoString partialHash;
        partialHash.Assign(reinterpret_cast<char*>(result.mEntry.mPartialHash.buf),
                           PARTIAL_LENGTH);

        nsresult rv = completer->Complete(partialHash, this);
        if (NS_SUCCEEDED(rv)) {
          mPendingCompletions++;
        }
      } else {
        NS_WARNING("Partial match in a table without a valid completer, ignoring partial match.");
      }
    }
  }

  if (mPendingCompletions == 0) {
    // All results were complete, we're ready!
    HandleResults();
  }

  return NS_OK;
}

NS_IMETHODIMP
nsUrlClassifierLookupCallback::CompletionFinished(nsresult status)
{
  LOG(("nsUrlClassifierLookupCallback::CompletionFinished [%p, %08x]",
       this, status));
  if (NS_FAILED(status)) {
    NS_WARNING("gethash response failed.");
  }

  mPendingCompletions--;
  if (mPendingCompletions == 0) {
    HandleResults();

    if (mCacheResults) {
      // This hands ownership of the cache results array back to the worker
      // thread.
      mDBService->CacheCompletions(mCacheResults.forget());
    }
  }

  return NS_OK;
}

NS_IMETHODIMP
nsUrlClassifierLookupCallback::Completion(const nsACString& completeHash,
                                          const nsACString& tableName,
                                          PRUint32 chunkId,
                                          PRBool verified)
{
  LOG(("nsUrlClassifierLookupCallback::Completion [%p, %d]", this, verified));
  nsUrlClassifierCompleteHash hash;
  hash.Assign(completeHash);

  for (PRUint32 i = 0; i < mResults->Length(); i++) {
    nsUrlClassifierLookupResult& result = mResults->ElementAt(i);

    if (!result.mEntry.mHaveComplete &&
        hash.StartsWith(result.mEntry.mPartialHash) &&
        // XXX: We really want to be comparing the table name to make
        // sure it matches.  Due to a short-lived server bug, they
        // won't just yet.  This should be fixed as soon as the server is.
#if 0
        result.mTableName == tableName &&
#endif
        result.mEntry.mChunkId == chunkId) {
      // We have a completion for this entry.  Fill it in...
      result.mEntry.SetHash(hash);

      // ... and make sure that it was the entry we were looking for.
      if (result.mLookupFragment == hash)
        result.mConfirmed = PR_TRUE;

      // If this result is guaranteed to come from our list provider,
      // we can cache the results.
      if (verified) {
        if (!mCacheResults) {
          mCacheResults = new nsTArray<nsUrlClassifierLookupResult>();
          if (!mCacheResults)
            return NS_ERROR_OUT_OF_MEMORY;
        }

        mCacheResults->AppendElement(result);
      }
    }
  }

  return NS_OK;
}

nsresult
nsUrlClassifierLookupCallback::HandleResults()
{
  if (!mResults) {
    // No results, this URI is clean.
    return mCallback->HandleEvent(NS_LITERAL_CSTRING(""));
  }

  // Build a stringified list of result tables.
  mResults->Sort();
  PRUint32 lastTableId = 0;
  nsCAutoString tables;
  for (PRUint32 i = 0; i < mResults->Length(); i++) {
    nsUrlClassifierLookupResult& result = mResults->ElementAt(i);
    // Leave out results that weren't confirmed, as their existence on
    // the list can't be verified.  Also leave out randomly-generated
    // noise.
    if (!result.mConfirmed || result.mNoise)
      continue;

    if (tables.Length() > 0) {
      if (lastTableId == result.mEntry.mTableId)
        continue;
      tables.Append(",");
    }

    tables.Append(result.mTableName);
    lastTableId = result.mEntry.mTableId;
  }

  return mCallback->HandleEvent(tables);
}


// -------------------------------------------------------------------------
// Helper class for nsIURIClassifier implementation, translates table names
// to nsIURIClassifier enums.

class nsUrlClassifierClassifyCallback : public nsIUrlClassifierCallback
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIURLCLASSIFIERCALLBACK

  nsUrlClassifierClassifyCallback(nsIURIClassifierCallback *c,
                                  PRBool checkMalware,
                                  PRBool checkPhishing)
    : mCallback(c)
    , mCheckMalware(checkMalware)
    , mCheckPhishing(checkPhishing)
    {}

private:
  nsCOMPtr<nsIURIClassifierCallback> mCallback;
  PRPackedBool mCheckMalware;
  PRPackedBool mCheckPhishing;
};

NS_IMPL_THREADSAFE_ISUPPORTS1(nsUrlClassifierClassifyCallback,
                              nsIUrlClassifierCallback)

NS_IMETHODIMP
nsUrlClassifierClassifyCallback::HandleEvent(const nsACString& tables)
{
  // XXX: we should probably have the wardens tell the service which table
  // names match with which classification.  For now the table names give
  // enough information.
  nsresult response = NS_OK;

  nsACString::const_iterator begin, end;

  tables.BeginReading(begin);
  tables.EndReading(end);
  if (mCheckMalware &&
      FindInReadable(NS_LITERAL_CSTRING("-malware-"), begin, end)) {
    response = NS_ERROR_MALWARE_URI;
  } else {
    // Reset begin before checking phishing table
    tables.BeginReading(begin);

    if (mCheckPhishing &&
        FindInReadable(NS_LITERAL_CSTRING("-phish-"), begin, end)) {
      response = NS_ERROR_PHISHING_URI;
    }
  }

  mCallback->OnClassifyComplete(response);

  return NS_OK;
}


// -------------------------------------------------------------------------
// Proxy class implementation

NS_IMPL_THREADSAFE_ISUPPORTS3(nsUrlClassifierDBService,
                              nsIUrlClassifierDBService,
                              nsIURIClassifier,
                              nsIObserver)

/* static */ nsUrlClassifierDBService*
nsUrlClassifierDBService::GetInstance(nsresult *result)
{
  *result = NS_OK;
  if (!sUrlClassifierDBService) {
    sUrlClassifierDBService = new nsUrlClassifierDBService();
    if (!sUrlClassifierDBService) {
      *result = NS_ERROR_OUT_OF_MEMORY;
      return nsnull;
    }

    NS_ADDREF(sUrlClassifierDBService);   // addref the global

    *result = sUrlClassifierDBService->Init();
    if (NS_FAILED(*result)) {
      NS_RELEASE(sUrlClassifierDBService);
      return nsnull;
    }
  } else {
    // Already exists, just add a ref
    NS_ADDREF(sUrlClassifierDBService);   // addref the return result
  }
  return sUrlClassifierDBService;
}


nsUrlClassifierDBService::nsUrlClassifierDBService()
 : mCheckMalware(CHECK_MALWARE_DEFAULT)
 , mCheckPhishing(CHECK_PHISHING_DEFAULT)
 , mInUpdate(PR_FALSE)
{
}

nsUrlClassifierDBService::~nsUrlClassifierDBService()
{
  sUrlClassifierDBService = nsnull;
}

nsresult
nsUrlClassifierDBService::Init()
{
#if defined(PR_LOGGING)
  if (!gUrlClassifierDbServiceLog)
    gUrlClassifierDbServiceLog = PR_NewLogModule("UrlClassifierDbService");
#endif

  // Force the storage service to be created on the main thread.
  nsresult rv;
  nsCOMPtr<mozIStorageService> storageService =
    do_GetService(MOZ_STORAGE_SERVICE_CONTRACTID, &rv);
  NS_ENSURE_SUCCESS(rv, rv);

  // Force PSM to be loaded on the main thread.
  nsCOMPtr<nsICryptoHash> hash =
    do_CreateInstance(NS_CRYPTO_HASH_CONTRACTID, &rv);
  NS_ENSURE_SUCCESS(rv, rv);

  // Should we check document loads for malware URIs?
  nsCOMPtr<nsIPrefBranch2> prefs = do_GetService(NS_PREFSERVICE_CONTRACTID);

  PRInt32 gethashNoise = 0;
  if (prefs) {
    PRBool tmpbool;
    rv = prefs->GetBoolPref(CHECK_MALWARE_PREF, &tmpbool);
    mCheckMalware = NS_SUCCEEDED(rv) ? tmpbool : CHECK_MALWARE_DEFAULT;

    prefs->AddObserver(CHECK_MALWARE_PREF, this, PR_FALSE);

    rv = prefs->GetBoolPref(CHECK_PHISHING_PREF, &tmpbool);
    mCheckPhishing = NS_SUCCEEDED(rv) ? tmpbool : CHECK_PHISHING_DEFAULT;

    prefs->AddObserver(CHECK_PHISHING_PREF, this, PR_FALSE);

    if (NS_FAILED(prefs->GetIntPref(GETHASH_NOISE_PREF, &gethashNoise))) {
      gethashNoise = GETHASH_NOISE_DEFAULT;
    }
  }

  // Start the background thread.
  rv = NS_NewThread(&gDbBackgroundThread);
  if (NS_FAILED(rv))
    return rv;

  mWorker = new nsUrlClassifierDBServiceWorker();
  if (!mWorker)
    return NS_ERROR_OUT_OF_MEMORY;

  rv = mWorker->Init(gethashNoise);
  if (NS_FAILED(rv)) {
    mWorker = nsnull;
    return rv;
  }

  // Proxy for calling the worker on the background thread
  rv = NS_GetProxyForObject(gDbBackgroundThread,
                            NS_GET_IID(nsIUrlClassifierDBServiceWorker),
                            mWorker,
                            NS_PROXY_ASYNC,
                            getter_AddRefs(mWorkerProxy));
  NS_ENSURE_SUCCESS(rv, rv);

  mCompleters.Init();

  // Add an observer for shutdown
  nsCOMPtr<nsIObserverService> observerService =
      do_GetService("@mozilla.org/observer-service;1");
  if (!observerService)
    return NS_ERROR_FAILURE;

  observerService->AddObserver(this, "profile-before-change", PR_FALSE);
  observerService->AddObserver(this, "xpcom-shutdown-threads", PR_FALSE);

  return NS_OK;
}

NS_IMETHODIMP
nsUrlClassifierDBService::Classify(nsIURI *uri,
                                   nsIURIClassifierCallback* c,
                                   PRBool* result)
{
  NS_ENSURE_TRUE(gDbBackgroundThread, NS_ERROR_NOT_INITIALIZED);

  if (!(mCheckMalware || mCheckPhishing)) {
    *result = PR_FALSE;
    return NS_OK;
  }

  nsRefPtr<nsUrlClassifierClassifyCallback> callback =
    new nsUrlClassifierClassifyCallback(c, mCheckMalware, mCheckPhishing);
  if (!callback) return NS_ERROR_OUT_OF_MEMORY;

  nsresult rv = LookupURI(uri, callback);
  if (rv == NS_ERROR_MALFORMED_URI) {
    // The URI had no hostname, don't try to classify it.
    *result = PR_FALSE;
    return NS_OK;
  }
  NS_ENSURE_SUCCESS(rv, rv);

  *result = PR_TRUE;
  return NS_OK;
}

NS_IMETHODIMP
nsUrlClassifierDBService::Lookup(const nsACString& spec,
                                 nsIUrlClassifierCallback* c)
{
  NS_ENSURE_TRUE(gDbBackgroundThread, NS_ERROR_NOT_INITIALIZED);

  nsCOMPtr<nsIURI> uri;

  nsresult rv = NS_NewURI(getter_AddRefs(uri), spec);
  NS_ENSURE_SUCCESS(rv, rv);

  uri = NS_GetInnermostURI(uri);
  if (!uri) {
    return NS_ERROR_FAILURE;
  }

  return LookupURI(uri, c);
}

nsresult
nsUrlClassifierDBService::LookupURI(nsIURI* uri,
                                    nsIUrlClassifierCallback* c)
{
  NS_ENSURE_TRUE(gDbBackgroundThread, NS_ERROR_NOT_INITIALIZED);

  nsCAutoString key;
  // Canonicalize the url
  nsCOMPtr<nsIUrlClassifierUtils> utilsService =
    do_GetService(NS_URLCLASSIFIERUTILS_CONTRACTID);
  nsresult rv = utilsService->GetKeyForURI(uri, key);
  if (NS_FAILED(rv))
    return rv;

  // Create an nsUrlClassifierLookupCallback object.  This object will
  // take care of confirming partial hash matches if necessary before
  // calling the client's callback.
  nsCOMPtr<nsIUrlClassifierLookupCallback> callback =
    new nsUrlClassifierLookupCallback(this, c);
  if (!callback)
    return NS_ERROR_OUT_OF_MEMORY;

  nsCOMPtr<nsIUrlClassifierLookupCallback> proxyCallback;
  // The proxy callback uses the current thread.
  rv = NS_GetProxyForObject(NS_PROXY_TO_CURRENT_THREAD,
                            NS_GET_IID(nsIUrlClassifierLookupCallback),
                            callback,
                            NS_PROXY_ASYNC,
                            getter_AddRefs(proxyCallback));
  NS_ENSURE_SUCCESS(rv, rv);

  // Queue this lookup and call the lookup function to flush the queue if
  // necessary.
  rv = mWorker->QueueLookup(key, proxyCallback);
  NS_ENSURE_SUCCESS(rv, rv);

  return mWorkerProxy->Lookup(EmptyCString(), nsnull);
}

NS_IMETHODIMP
nsUrlClassifierDBService::GetTables(nsIUrlClassifierCallback* c)
{
  NS_ENSURE_TRUE(gDbBackgroundThread, NS_ERROR_NOT_INITIALIZED);

  nsresult rv;
  // The proxy callback uses the current thread.
  nsCOMPtr<nsIUrlClassifierCallback> proxyCallback;
  rv = NS_GetProxyForObject(NS_PROXY_TO_CURRENT_THREAD,
                            NS_GET_IID(nsIUrlClassifierCallback),
                            c,
                            NS_PROXY_ASYNC,
                            getter_AddRefs(proxyCallback));
  NS_ENSURE_SUCCESS(rv, rv);

  return mWorkerProxy->GetTables(proxyCallback);
}

NS_IMETHODIMP
nsUrlClassifierDBService::SetHashCompleter(const nsACString &tableName,
                                           nsIUrlClassifierHashCompleter *completer)
{
  if (completer) {
    if (!mCompleters.Put(tableName, completer)) {
      return NS_ERROR_OUT_OF_MEMORY;
    }
  } else {
    mCompleters.Remove(tableName);
  }

  return NS_OK;
}

NS_IMETHODIMP
nsUrlClassifierDBService::BeginUpdate(nsIUrlClassifierUpdateObserver *observer,
                                      const nsACString &clientKey)
{
  NS_ENSURE_TRUE(gDbBackgroundThread, NS_ERROR_NOT_INITIALIZED);

  if (mInUpdate)
    return NS_ERROR_NOT_AVAILABLE;

  mInUpdate = PR_TRUE;

  nsresult rv;

  // The proxy observer uses the current thread
  nsCOMPtr<nsIUrlClassifierUpdateObserver> proxyObserver;
  rv = NS_GetProxyForObject(NS_PROXY_TO_CURRENT_THREAD,
                            NS_GET_IID(nsIUrlClassifierUpdateObserver),
                            observer,
                            NS_PROXY_ASYNC,
                            getter_AddRefs(proxyObserver));
  NS_ENSURE_SUCCESS(rv, rv);

  return mWorkerProxy->BeginUpdate(proxyObserver, clientKey);
}

NS_IMETHODIMP
nsUrlClassifierDBService::BeginStream(const nsACString &table,
                                      const nsACString &serverMAC)
{
  NS_ENSURE_TRUE(gDbBackgroundThread, NS_ERROR_NOT_INITIALIZED);

  return mWorkerProxy->BeginStream(table, serverMAC);
}

NS_IMETHODIMP
nsUrlClassifierDBService::UpdateStream(const nsACString& aUpdateChunk)
{
  NS_ENSURE_TRUE(gDbBackgroundThread, NS_ERROR_NOT_INITIALIZED);

  return mWorkerProxy->UpdateStream(aUpdateChunk);
}

NS_IMETHODIMP
nsUrlClassifierDBService::FinishStream()
{
  NS_ENSURE_TRUE(gDbBackgroundThread, NS_ERROR_NOT_INITIALIZED);

  return mWorkerProxy->FinishStream();
}

NS_IMETHODIMP
nsUrlClassifierDBService::FinishUpdate()
{
  NS_ENSURE_TRUE(gDbBackgroundThread, NS_ERROR_NOT_INITIALIZED);

  mInUpdate = PR_FALSE;

  return mWorkerProxy->FinishUpdate();
}


NS_IMETHODIMP
nsUrlClassifierDBService::CancelUpdate()
{
  NS_ENSURE_TRUE(gDbBackgroundThread, NS_ERROR_NOT_INITIALIZED);

  mInUpdate = PR_FALSE;

  return mWorkerProxy->CancelUpdate();
}

NS_IMETHODIMP
nsUrlClassifierDBService::ResetDatabase()
{
  NS_ENSURE_TRUE(gDbBackgroundThread, NS_ERROR_NOT_INITIALIZED);

  return mWorkerProxy->ResetDatabase();
}

nsresult
nsUrlClassifierDBService::CacheCompletions(nsTArray<nsUrlClassifierLookupResult> *results)
{
  NS_ENSURE_TRUE(gDbBackgroundThread, NS_ERROR_NOT_INITIALIZED);

  return mWorkerProxy->CacheCompletions(results);
}

NS_IMETHODIMP
nsUrlClassifierDBService::Observe(nsISupports *aSubject, const char *aTopic,
                                  const PRUnichar *aData)
{
  if (!strcmp(aTopic, NS_PREFBRANCH_PREFCHANGE_TOPIC_ID)) {
    nsresult rv;
    nsCOMPtr<nsIPrefBranch> prefs(do_QueryInterface(aSubject, &rv));
    NS_ENSURE_SUCCESS(rv, rv);
    if (NS_LITERAL_STRING(CHECK_MALWARE_PREF).Equals(aData)) {
      PRBool tmpbool;
      rv = prefs->GetBoolPref(CHECK_MALWARE_PREF, &tmpbool);
      mCheckMalware = NS_SUCCEEDED(rv) ? tmpbool : CHECK_MALWARE_DEFAULT;
    } else if (NS_LITERAL_STRING(CHECK_PHISHING_PREF).Equals(aData)) {
      PRBool tmpbool;
      rv = prefs->GetBoolPref(CHECK_PHISHING_PREF, &tmpbool);
      mCheckPhishing = NS_SUCCEEDED(rv) ? tmpbool : CHECK_PHISHING_DEFAULT;
    }
  } else if (!strcmp(aTopic, "profile-before-change") ||
             !strcmp(aTopic, "xpcom-shutdown-threads")) {
    Shutdown();
  } else {
    return NS_ERROR_UNEXPECTED;
  }

  return NS_OK;
}

// Join the background thread if it exists.
nsresult
nsUrlClassifierDBService::Shutdown()
{
  LOG(("shutting down db service\n"));

  if (!gDbBackgroundThread)
    return NS_OK;

  mCompleters.Clear();

  nsCOMPtr<nsIPrefBranch2> prefs = do_GetService(NS_PREFSERVICE_CONTRACTID);
  if (prefs) {
    prefs->RemoveObserver(CHECK_MALWARE_PREF, this);
    prefs->RemoveObserver(CHECK_PHISHING_PREF, this);
  }

  nsresult rv;
  // First close the db connection.
  if (mWorker) {
    rv = mWorkerProxy->CloseDb();
    NS_ASSERTION(NS_SUCCEEDED(rv), "failed to post close db event");
  }

  mWorkerProxy = nsnull;

  LOG(("joining background thread"));

  gShuttingDownThread = PR_TRUE;

  nsIThread *backgroundThread = gDbBackgroundThread;
  gDbBackgroundThread = nsnull;
  backgroundThread->Shutdown();
  NS_RELEASE(backgroundThread);

  return NS_OK;
}
