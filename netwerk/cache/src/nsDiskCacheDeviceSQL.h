/* vim:set ts=2 sw=2 sts=2 et cin: */
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
 * The Original Code is Mozilla.
 *
 * The Initial Developer of the Original Code is IBM Corporation.
 * Portions created by IBM Corporation are Copyright (C) 2004
 * IBM Corporation. All Rights Reserved.
 *
 * Contributor(s):
 *   Darin Fisher <darin@meer.net>
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

#ifndef nsOfflineCacheDevice_h__
#define nsOfflineCacheDevice_h__

#include "nsCacheDevice.h"
#include "nsILocalFile.h"
#include "nsIObserver.h"
#include "mozIStorageConnection.h"
#include "mozIStorageFunction.h"
#include "nsIFile.h"
#include "nsAutoPtr.h"
#include "nsCOMPtr.h"
#include "nsCOMArray.h"
#include "nsVoidArray.h"

class nsOfflineCacheDevice;

class nsOfflineCacheEvictionFunction : public mozIStorageFunction {
public:
  NS_DECL_ISUPPORTS
  NS_DECL_MOZISTORAGEFUNCTION

  nsOfflineCacheEvictionFunction(nsOfflineCacheDevice *device)
    : mDevice(device)
  {}

  void Reset() { mItems.Clear(); }
  void Apply();

private:
  nsOfflineCacheDevice *mDevice;
  nsCOMArray<nsIFile> mItems;

};

class nsOfflineCacheDevice : public nsCacheDevice
{
public:
  nsOfflineCacheDevice();

  /**
   * nsCacheDevice methods
   */

  virtual ~nsOfflineCacheDevice();

  virtual nsresult        Init();
  virtual nsresult        Shutdown();

  virtual const char *    GetDeviceID(void);
  virtual nsCacheEntry *  FindEntry(nsCString * key, PRBool *collision);
  virtual nsresult        DeactivateEntry(nsCacheEntry * entry);
  virtual nsresult        BindEntry(nsCacheEntry * entry);
  virtual void            DoomEntry( nsCacheEntry * entry );

  virtual nsresult OpenInputStreamForEntry(nsCacheEntry *    entry,
                                           nsCacheAccessMode mode,
                                           PRUint32          offset,
                                           nsIInputStream ** result);

  virtual nsresult OpenOutputStreamForEntry(nsCacheEntry *     entry,
                                            nsCacheAccessMode  mode,
                                            PRUint32           offset,
                                            nsIOutputStream ** result);

  virtual nsresult        GetFileForEntry(nsCacheEntry *    entry,
                                          nsIFile **        result);

  virtual nsresult        OnDataSizeChange(nsCacheEntry * entry, PRInt32 deltaSize);
  
  virtual nsresult        Visit(nsICacheVisitor * visitor);

  virtual nsresult        EvictEntries(const char * clientID);


  /* Entry ownership */
  nsresult                GetOwnerDomains(const char *        clientID,
                                          PRUint32 *          count,
                                          char ***            domains);
  nsresult                GetOwnerURIs(const char *           clientID,
                                       const nsACString &     ownerDomain,
                                       PRUint32 *             count,
                                       char ***               uris);
  nsresult                SetOwnedKeys(const char *           clientID,
                                       const nsACString &     ownerDomain,
                                       const nsACString &     ownerUrl,
                                       PRUint32               count,
                                       const char **          keys);
  nsresult                GetOwnedKeys(const char *           clientID,
                                       const nsACString &     ownerDomain,
                                       const nsACString &     ownerUrl,
                                       PRUint32 *             count,
                                       char ***               keys);
  nsresult                AddOwnedKey(const char *            clientID,
                                      const nsACString &      ownerDomain,
                                      const nsACString &      ownerURI,
                                      const nsACString &      key);
  nsresult                RemoveOwnedKey(const char *         clientID,
                                         const nsACString &   ownerDomain,
                                         const nsACString &   ownerURI,
                                         const nsACString &   key);
  nsresult                KeyIsOwned(const char *             clientID,
                                     const nsACString &       ownerDomain,
                                     const nsACString &       ownerURI,
                                     const nsACString &       key,
                                     PRBool *                 isOwned);

  nsresult                ClearKeysOwnedByDomain(const char *clientID,
                                                 const nsACString &ownerDomain);
  nsresult                GetDomainUsage(const char *clientID,
                                         const nsACString &ownerDomain,
                                         PRUint32 *usage);
  nsresult                EvictUnownedEntries(const char *clientID);

  nsresult                CreateTemporaryClientID(nsACString &clientID);
  nsresult                MergeTemporaryClientID(const char *clientID,
                                                 const char *fromClientID);


  /**
   * Preference accessors
   */

  void                    SetCacheParentDirectory(nsILocalFile * parentDir);
  void                    SetCapacity(PRUint32  capacity);

  nsILocalFile *          CacheDirectory() { return mCacheDirectory; }
  PRUint32                CacheCapacity() { return mCacheCapacity; }
  PRUint32                CacheSize();
  PRUint32                EntryCount();
  
private:
  PRBool   Initialized() { return mDB != nsnull; }
  nsresult UpdateEntry(nsCacheEntry *entry);
  nsresult UpdateEntrySize(nsCacheEntry *entry, PRUint32 newSize);
  nsresult DeleteEntry(nsCacheEntry *entry, PRBool deleteData);
  nsresult DeleteData(nsCacheEntry *entry);
  nsresult EnableEvictionObserver();
  nsresult DisableEvictionObserver();
  nsresult RunSimpleQuery(mozIStorageStatement *statment,
                          PRUint32 resultIndex,
                          PRUint32 * count,
                          char *** values);

  nsCOMPtr<mozIStorageConnection>          mDB;
  nsRefPtr<nsOfflineCacheEvictionFunction> mEvictionFunction;

  nsCOMPtr<mozIStorageStatement>  mStatement_CacheSize;
  nsCOMPtr<mozIStorageStatement>  mStatement_DomainSize;
  nsCOMPtr<mozIStorageStatement>  mStatement_EntryCount;
  nsCOMPtr<mozIStorageStatement>  mStatement_UpdateEntry;
  nsCOMPtr<mozIStorageStatement>  mStatement_UpdateEntrySize;
  nsCOMPtr<mozIStorageStatement>  mStatement_UpdateEntryFlags;
  nsCOMPtr<mozIStorageStatement>  mStatement_DeleteEntry;
  nsCOMPtr<mozIStorageStatement>  mStatement_FindEntry;
  nsCOMPtr<mozIStorageStatement>  mStatement_BindEntry;
  nsCOMPtr<mozIStorageStatement>  mStatement_ClearOwnership;
  nsCOMPtr<mozIStorageStatement>  mStatement_RemoveOwnership;
  nsCOMPtr<mozIStorageStatement>  mStatement_ClearDomain;
  nsCOMPtr<mozIStorageStatement>  mStatement_AddOwnership;
  nsCOMPtr<mozIStorageStatement>  mStatement_CheckOwnership;
  nsCOMPtr<mozIStorageStatement>  mStatement_DeleteConflicts;
  nsCOMPtr<mozIStorageStatement>  mStatement_DeleteUnowned;
  nsCOMPtr<mozIStorageStatement>  mStatement_ListOwned;
  nsCOMPtr<mozIStorageStatement>  mStatement_ListOwners;
  nsCOMPtr<mozIStorageStatement>  mStatement_ListOwnerDomains;
  nsCOMPtr<mozIStorageStatement>  mStatement_ListOwnerURIs;
  nsCOMPtr<mozIStorageStatement>  mStatement_SwapClientID;

  nsCOMPtr<nsILocalFile>          mCacheDirectory;
  PRUint32                        mCacheCapacity;
  PRInt32                         mDeltaCounter;
};

#endif // nsOfflineCacheDevice_h__
