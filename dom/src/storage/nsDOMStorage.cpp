/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
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
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is
 * Mozilla Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2006
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Neil Deakin <enndeakin@sympatico.ca>
 *   Johnny Stenback <jst@mozilla.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either of the GNU General Public License Version 2 or later (the "GPL"),
 * or the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
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

#include "nsCOMPtr.h"
#include "nsDOMError.h"
#include "nsDOMClassInfo.h"
#include "nsUnicharUtils.h"
#include "nsIDocument.h"
#include "nsDOMStorage.h"
#include "nsContentUtils.h"
#include "nsIScriptSecurityManager.h"
#include "nsIPrincipal.h"
#include "nsIURI.h"
#include "nsReadableUtils.h"
#include "nsIObserverService.h"
#include "nsNetUtil.h"

//
// Helper that tells us whether the caller is secure or not.
//

static PRBool
IsCallerSecure()
{
  nsCOMPtr<nsIPrincipal> subjectPrincipal;
  nsContentUtils::GetSecurityManager()->
    GetSubjectPrincipal(getter_AddRefs(subjectPrincipal));

  if (!subjectPrincipal) {
    // No subject principal means no code is running. Default to not
    // being secure in that case.

    return PR_FALSE;
  }

  nsCOMPtr<nsIURI> codebase;
  subjectPrincipal->GetURI(getter_AddRefs(codebase));

  if (!codebase) {
    return PR_FALSE;
  }

  nsCOMPtr<nsIURI> innerUri = NS_GetInnermostURI(codebase);

  if (!innerUri) {
    return PR_FALSE;
  }

  PRBool isHttps = PR_FALSE;
  nsresult rv = innerUri->SchemeIs("https", &isHttps);

  return NS_SUCCEEDED(rv) && isHttps;
}

nsSessionStorageEntry::nsSessionStorageEntry(KeyTypePointer aStr)
  : nsStringHashKey(aStr), mItem(nsnull)
{
}

nsSessionStorageEntry::nsSessionStorageEntry(const nsSessionStorageEntry& aToCopy)
  : nsStringHashKey(aToCopy), mItem(nsnull)
{
  NS_ERROR("We're horked.");
}

nsSessionStorageEntry::~nsSessionStorageEntry()
{
}

//
// nsDOMStorage
//

#ifdef MOZ_STORAGE
nsDOMStorageDB* nsDOMStorage::gStorageDB = nsnull;
#endif

NS_INTERFACE_MAP_BEGIN(nsDOMStorage)
  NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(nsISupports, nsIDOMStorage)
  NS_INTERFACE_MAP_ENTRY(nsIDOMStorage)
  NS_INTERFACE_MAP_ENTRY(nsPIDOMStorage)
  NS_DOM_INTERFACE_MAP_ENTRY_CLASSINFO(Storage)
NS_INTERFACE_MAP_END

NS_IMPL_ADDREF(nsDOMStorage)
NS_IMPL_RELEASE(nsDOMStorage)

NS_IMETHODIMP
NS_NewDOMStorage(nsISupports* aOuter, REFNSIID aIID, void** aResult)
{
  nsDOMStorage* storage = new nsDOMStorage();
  if (!storage)
    return NS_ERROR_OUT_OF_MEMORY;

  NS_ADDREF(storage);
  *aResult = storage;

  return NS_OK;
}

nsDOMStorage::nsDOMStorage()
  : mUseDB(PR_FALSE), mItemsCached(PR_FALSE)
{
  mItems.Init(8);
}

nsDOMStorage::nsDOMStorage(const nsAString& aDomain, PRBool aUseDB)
  : mUseDB(aUseDB), mItemsCached(PR_FALSE), mDomain(aDomain)
{
#ifndef MOZ_STORAGE
  mUseDB = PR_FALSE;
#endif

  mItems.Init(8);
}

nsDOMStorage::~nsDOMStorage()
{
}

void
nsDOMStorage::Init(const nsAString& aDomain, PRBool aUseDB)
{
  mDomain.Assign(aDomain);
#ifdef MOZ_STORAGE
  mUseDB = aUseDB;
#else
  mUseDB = PR_FALSE;
#endif
}

class ItemCounterState
{
public:
  ItemCounterState(PRBool aIsCallerSecure)
    : mIsCallerSecure(aIsCallerSecure), mCount(0)
  {
  }

  PRBool mIsCallerSecure;
  PRBool mCount;
private:
  ItemCounterState(); // Not to be implemented
};

PR_STATIC_CALLBACK(PLDHashOperator)
ItemCounter(nsSessionStorageEntry* aEntry, void* userArg)
{
  ItemCounterState *state = (ItemCounterState *)userArg;

  if (state->mIsCallerSecure || !aEntry->mItem->IsSecure()) {
    ++state->mCount;
  }

  return PL_DHASH_NEXT;
}

NS_IMETHODIMP
nsDOMStorage::GetLength(PRUint32 *aLength)
{
  if (mUseDB)
    CacheKeysFromDB();

  ItemCounterState state(IsCallerSecure());

  mItems.EnumerateEntries(ItemCounter, &state);

  *aLength = state.mCount;

  return NS_OK;
}

class IndexFinderData
{
public:
  IndexFinderData(PRBool aIsCallerSecure, PRUint32 aWantedIndex)
    : mIsCallerSecure(aIsCallerSecure), mIndex(0), mWantedIndex(aWantedIndex),
      mItem(nsnull)
  {
  }

  PRBool mIsCallerSecure;
  PRUint32 mIndex;
  PRUint32 mWantedIndex;
  nsSessionStorageEntry *mItem;

private:
  IndexFinderData(); // Not to be implemented
};

PR_STATIC_CALLBACK(PLDHashOperator)
IndexFinder(nsSessionStorageEntry* aEntry, void* userArg)
{
  IndexFinderData *data = (IndexFinderData *)userArg;

  if (data->mIndex == data->mWantedIndex &&
      (data->mIsCallerSecure || !aEntry->mItem->IsSecure())) {
    data->mItem = aEntry;

    return PL_DHASH_STOP;
  }

  ++data->mIndex;

  return PL_DHASH_NEXT;
}

NS_IMETHODIMP
nsDOMStorage::Key(PRUint32 aIndex, nsAString& aKey)
{
  // XXXjst: This is as retarded as the DOM spec is, takes an unsigned
  // int, but the spec talks about what to do if a negative value is
  // passed in.

  // XXX: This does a linear search for the key at index, which would
  // suck if there's a large numer of indexes. Do we care? If so,
  // maybe we need to have a lazily populated key array here or
  // something?

  if (mUseDB)
    CacheKeysFromDB();

  IndexFinderData data(IsCallerSecure(), aIndex);
  mItems.EnumerateEntries(IndexFinder, &data);

  if (!data.mItem) {
    // aIndex was larger than the number of accessible keys. Throw.
    return NS_ERROR_DOM_INDEX_SIZE_ERR;
  }

  aKey = data.mItem->GetKey();

  return NS_OK;
}

NS_IMETHODIMP
nsDOMStorage::GetItem(const nsAString& aKey, nsIDOMStorageItem **aItem)
{
  *aItem = nsnull;

  if (aKey.IsEmpty())
    return NS_OK;

  nsSessionStorageEntry *entry = mItems.GetEntry(aKey);
 
  if (entry) {
    if (!IsCallerSecure() && entry->mItem->IsSecure()) {
      return NS_OK;
    }
    NS_ADDREF(*aItem = entry->mItem);
  }
  else if (mUseDB) {
    PRBool secure;
    nsAutoString value;
    nsresult rv = GetDBValue(aKey, value, &secure);
    // return null if access isn't allowed or the key wasn't found
    if (rv == NS_ERROR_DOM_SECURITY_ERR || rv == NS_ERROR_DOM_NOT_FOUND_ERR)
      return NS_OK;
    NS_ENSURE_SUCCESS(rv, rv);

    nsRefPtr<nsDOMStorageItem> newitem =
      new nsDOMStorageItem(this, aKey, secure);
    if (!newitem)
      return NS_ERROR_OUT_OF_MEMORY;

    entry = mItems.PutEntry(aKey);
    NS_ENSURE_TRUE(entry, NS_ERROR_OUT_OF_MEMORY);

    entry->mItem = newitem;
    NS_ADDREF(*aItem = newitem);
  }

  return NS_OK;
}

NS_IMETHODIMP
nsDOMStorage::SetItem(const nsAString& aKey, const nsAString& aData)
{
  if (aKey.IsEmpty())
    return NS_OK;

  nsresult rv;
  nsRefPtr<nsDOMStorageItem> newitem = nsnull;
  nsSessionStorageEntry *entry = mItems.GetEntry(aKey);
  if (entry) {
    if (entry->mItem->IsSecure() && !IsCallerSecure()) {
      return NS_ERROR_DOM_SECURITY_ERR;
    }
    if (!mUseDB) {
      rv = entry->mItem->SetValue(aData);
      NS_ENSURE_SUCCESS(rv, rv);
    }
  }
  else {
    if (mUseDB)
      newitem = new nsDOMStorageItem(this, aKey, PR_FALSE);
    else 
      newitem = new nsDOMStorageItem(nsnull, aData, PR_FALSE);
    if (!newitem)
      return NS_ERROR_OUT_OF_MEMORY;
  }

  if (mUseDB) {
    rv = SetDBValue(aKey, aData, IsCallerSecure());
    NS_ENSURE_SUCCESS(rv, rv);
  }

  if (newitem) {
    entry = mItems.PutEntry(aKey);
    NS_ENSURE_TRUE(entry, NS_ERROR_OUT_OF_MEMORY);
    entry->mItem = newitem;
  }

  // SetDBValue already calls BroadcastChangeNotification so don't do it again
  if (!mUseDB)
    BroadcastChangeNotification();

  return NS_OK;
}

NS_IMETHODIMP nsDOMStorage::RemoveItem(const nsAString& aKey)
{
  if (aKey.IsEmpty())
    return NS_OK;

  nsSessionStorageEntry *entry = mItems.GetEntry(aKey);

  if (entry && entry->mItem->IsSecure() && !IsCallerSecure()) {
    return NS_ERROR_DOM_SECURITY_ERR;
  }

  if (mUseDB) {
#ifdef MOZ_STORAGE
    nsresult rv = InitDB();
    NS_ENSURE_SUCCESS(rv, rv);

    nsAutoString unused;
    PRBool secureItem;
    rv = GetDBValue(aKey, unused, &secureItem);
    if (rv == NS_ERROR_DOM_NOT_FOUND_ERR)
      return NS_OK;
    NS_ENSURE_SUCCESS(rv, rv);

    rv = gStorageDB->RemoveKey(mDomain, aKey);
    NS_ENSURE_SUCCESS(rv, rv);

    mItemsCached = PR_FALSE;

    BroadcastChangeNotification();
#endif
  }
  else if (entry) {
    // clear string as StorageItems may be referencing this item
    entry->mItem->ClearValue();

    BroadcastChangeNotification();
  }

  if (entry) {
    mItems.RawRemoveEntry(entry);
  }

  return NS_OK;
}

nsresult
nsDOMStorage::InitDB()
{
#ifdef MOZ_STORAGE
  if (!gStorageDB) {
    gStorageDB = new nsDOMStorageDB();
    if (!gStorageDB)
      return NS_ERROR_OUT_OF_MEMORY;

    nsresult rv = gStorageDB->Init();
    if (NS_FAILED(rv)) {
      // Failed to initialize the DB, delete it and null out the
      // pointer so we don't end up attempting to use an
      // un-initialized DB later on.

      delete gStorageDB;
      gStorageDB = nsnull;

      return rv;
    }
  }
#endif

  return NS_OK;
}

nsresult
nsDOMStorage::CacheKeysFromDB()
{
#ifdef MOZ_STORAGE
  // cache all the keys in the hash. This is used by the Length and Key methods
  // use this cache for better performance. The disadvantage is that the
  // order may break if someone changes the keys in the database directly.
  if (!mItemsCached) {
    nsresult rv = InitDB();
    NS_ENSURE_SUCCESS(rv, rv);

    rv = gStorageDB->GetAllKeys(mDomain, this, &mItems);
    NS_ENSURE_SUCCESS(rv, rv);

    mItemsCached = PR_TRUE;
  }
#endif

  return NS_OK;
}

nsresult
nsDOMStorage::GetDBValue(const nsAString& aKey, nsAString& aValue,
                         PRBool* aSecure)
{
  aValue.Truncate();

#ifdef MOZ_STORAGE
  NS_ASSERTION(mUseDB,
               "Uh, we should only get here if we're using the database!");

  nsresult rv = InitDB();
  NS_ENSURE_SUCCESS(rv, rv);

  nsAutoString value;
  rv = gStorageDB->GetKeyValue(mDomain, aKey, value, aSecure);
  if (NS_FAILED(rv))
    return rv;

  if (!IsCallerSecure() && *aSecure) {
    return NS_ERROR_DOM_SECURITY_ERR;
  }

  aValue.Assign(value);
#endif

  return NS_OK;
}

nsresult
nsDOMStorage::SetDBValue(const nsAString& aKey,
                         const nsAString& aValue,
                         PRBool aSecure)
{
#ifdef MOZ_STORAGE
  NS_ASSERTION(mUseDB,
               "Uh, we should only get here if we're using the database!");

  nsresult rv = InitDB();
  NS_ENSURE_SUCCESS(rv, rv);

  nsAutoString value;
  rv = gStorageDB->SetKey(mDomain, aKey, aValue, aSecure);
  NS_ENSURE_SUCCESS(rv, rv);

  mItemsCached = PR_FALSE;

  BroadcastChangeNotification();
#endif

  return NS_OK;
}

nsresult
nsDOMStorage::SetSecure(const nsAString& aKey, PRBool aSecure)
{
#ifdef MOZ_STORAGE
  if (mUseDB) {
    nsresult rv = InitDB();
    NS_ENSURE_SUCCESS(rv, rv);

    return gStorageDB->SetSecure(mDomain, aKey, aSecure);
  }
#else
  return NS_ERROR_NOT_IMPLEMENTED;
#endif

  nsSessionStorageEntry *entry = mItems.GetEntry(aKey);
  NS_ASSERTION(entry, "Don't use SetSecure() with non-existing keys!");

  if (entry) {
    entry->mItem->SetSecureInternal(aSecure);
  }  

  return NS_OK;
}

PR_STATIC_CALLBACK(PLDHashOperator)
CopyStorageItems(nsSessionStorageEntry* aEntry, void* userArg)
{
  nsDOMStorage* newstorage = NS_STATIC_CAST(nsDOMStorage*, userArg);

  newstorage->SetItem(aEntry->GetKey(), aEntry->mItem->GetValueInternal());

  if (aEntry->mItem->IsSecure()) {
    newstorage->SetSecure(aEntry->GetKey(), PR_TRUE);
  }

  return PL_DHASH_NEXT;
}

already_AddRefed<nsIDOMStorage>
nsDOMStorage::Clone()
{
  if (mUseDB) {
    NS_ERROR("Uh, don't clone a global storage object.");

    return nsnull;
  }

  nsDOMStorage* storage = new nsDOMStorage(mDomain, PR_FALSE);
  if (!storage)
    return nsnull;

  mItems.EnumerateEntries(CopyStorageItems, storage);

  NS_ADDREF(storage);

  return storage;
}

struct KeysArrayBuilderStruct
{
  PRBool callerIsSecure;
  nsTArray<nsString> *keys;
};

PR_STATIC_CALLBACK(PLDHashOperator)
KeysArrayBuilder(nsSessionStorageEntry* aEntry, void* userArg)
{
  KeysArrayBuilderStruct *keystruct = (KeysArrayBuilderStruct *)userArg;
  
  if (keystruct->callerIsSecure || !aEntry->mItem->IsSecure())
    keystruct->keys->AppendElement(aEntry->GetKey());

  return PL_DHASH_NEXT;
}

nsTArray<nsString> *
nsDOMStorage::GetKeys()
{
  if (mUseDB)
    CacheKeysFromDB();

  KeysArrayBuilderStruct keystruct;
  keystruct.callerIsSecure = IsCallerSecure();
  keystruct.keys = new nsTArray<nsString>();
  if (keystruct.keys)
    mItems.EnumerateEntries(KeysArrayBuilder, &keystruct);
 
  return keystruct.keys;
}

void
nsDOMStorage::BroadcastChangeNotification()
{
  nsresult rv;
  nsCOMPtr<nsIObserverService> observerService =
    do_GetService("@mozilla.org/observer-service;1", &rv);
  if (NS_FAILED(rv)) {
    return;
  }

  // Fire off a notification that a storage object changed. If the
  // storage object is a session storage object, we don't pass a
  // domain, but if it's a global storage object we do.
  observerService->NotifyObservers((nsIDOMStorage *)this,
                                   "dom-storage-changed",
                                   mUseDB ? mDomain.get() : nsnull);
}

//
// nsDOMStorageList
//

NS_INTERFACE_MAP_BEGIN(nsDOMStorageList)
  NS_INTERFACE_MAP_ENTRY(nsISupports)
  NS_INTERFACE_MAP_ENTRY(nsIDOMStorageList)
  NS_DOM_INTERFACE_MAP_ENTRY_CLASSINFO(StorageList)
NS_INTERFACE_MAP_END

NS_IMPL_ADDREF(nsDOMStorageList)
NS_IMPL_RELEASE(nsDOMStorageList)

nsresult
nsDOMStorageList::NamedItem(const nsAString& aDomain,
                            nsIDOMStorage** aStorage)
{
  nsIScriptSecurityManager* ssm = nsContentUtils::GetSecurityManager();
  if (!ssm)
    return NS_ERROR_FAILURE;

  nsCOMPtr<nsIPrincipal> subjectPrincipal;
  nsresult rv = ssm->GetSubjectPrincipal(getter_AddRefs(subjectPrincipal));
  NS_ENSURE_SUCCESS(rv, rv);

  nsCOMPtr<nsIPrincipal> systemPrincipal;
  rv = ssm->GetSystemPrincipal(getter_AddRefs(systemPrincipal));
  NS_ENSURE_SUCCESS(rv, rv);
    
  nsCAutoString currentDomain;
  if (subjectPrincipal) {
    nsCOMPtr<nsIURI> uri;
    rv = subjectPrincipal->GetURI(getter_AddRefs(uri));
    if (NS_SUCCEEDED(rv) && uri) {
      rv = uri->GetAsciiHost(currentDomain);
      NS_ENSURE_SUCCESS(rv, NS_ERROR_DOM_SECURITY_ERR);
    }
  }
    
  if (subjectPrincipal == systemPrincipal || !currentDomain.IsEmpty()) {
    return GetStorageForDomain(aDomain, NS_ConvertUTF8toUTF16(currentDomain),
                               subjectPrincipal == systemPrincipal, aStorage);
  }

  return NS_ERROR_DOM_SECURITY_ERR;
}

// static
PRBool
nsDOMStorageList::CanAccessDomain(const nsAString& aRequestedDomain,
                                  const nsAString& aCurrentDomain)
{
  nsStringArray requestedDomainArray, currentDomainArray;
  PRBool ok = ConvertDomainToArray(aRequestedDomain, &requestedDomainArray);
  if (!ok)
    return PR_FALSE;

  ok = ConvertDomainToArray(aCurrentDomain, &currentDomainArray);
  if (!ok)
    return PR_FALSE;

  if (currentDomainArray.Count() == 1)
    currentDomainArray.AppendString(NS_LITERAL_STRING("localdomain"));

  // need to use the shorter of the two arrays
  PRInt32 currentPos = 0;
  PRInt32 requestedPos = 0;
  PRInt32 length = requestedDomainArray.Count();
  if (currentDomainArray.Count() > length)
    currentPos = currentDomainArray.Count() - length;
  else if (currentDomainArray.Count() < length)
    requestedPos = length - currentDomainArray.Count();

  // If the current domain is different in any of the parts from the
  // requested domain, a security exception is raised
  for (; requestedPos < length; requestedPos++, currentPos++) {
    if (*requestedDomainArray[requestedPos] != *currentDomainArray[currentPos])
      return PR_FALSE;
  }

  return PR_TRUE;
}

nsresult
nsDOMStorageList::GetStorageForDomain(const nsAString& aRequestedDomain,
                                      const nsAString& aCurrentDomain,
                                      PRBool aNoCurrentDomainCheck,
                                      nsIDOMStorage** aStorage)
{
  if (!aNoCurrentDomainCheck && !CanAccessDomain(aRequestedDomain,
                                                 aCurrentDomain)) {
    return NS_ERROR_DOM_SECURITY_ERR;
  }

  nsStringArray requestedDomainArray;
  PRBool ok = ConvertDomainToArray(aRequestedDomain, &requestedDomainArray);
  if (!ok)
    return NS_ERROR_DOM_SECURITY_ERR;
  
  // now rebuild a string for the domain.
  nsAutoString usedDomain;
  PRInt32 requestedPos = 0;
  for (requestedPos = 0; requestedPos < requestedDomainArray.Count();
       requestedPos++) {
    if (!usedDomain.IsEmpty())
      usedDomain.AppendLiteral(".");
    usedDomain.Append(*requestedDomainArray[requestedPos]);
  }

  // now have a valid domain, so look it up in the storage table
  if (!mStorages.Get(usedDomain, aStorage)) {
    nsCOMPtr<nsIDOMStorage> newstorage = new nsDOMStorage(usedDomain, PR_TRUE);
    if (!newstorage)
      return NS_ERROR_OUT_OF_MEMORY;

    if (!mStorages.Put(usedDomain, newstorage))
      return NS_ERROR_OUT_OF_MEMORY;

    newstorage.swap(*aStorage);
  }

  return NS_OK;
}

// static
PRBool
nsDOMStorageList::ConvertDomainToArray(const nsAString& aDomain,
                                       nsStringArray* aArray)
{
  PRInt32 length = aDomain.Length();
  PRInt32 n = 0;
  while (n < length) {
    PRInt32 dotpos = aDomain.FindChar('.', n);
    nsAutoString domain;

    if (dotpos == -1) // no more dots
      domain.Assign(Substring(aDomain, n));
    else if (dotpos - n == 0) // no point continuing in this case
      return false;
    else if (dotpos >= 0)
      domain.Assign(Substring(aDomain, n, dotpos - n));

    ToLowerCase(domain);
    aArray->AppendString(domain);

    if (dotpos == -1)
      break;

    n = dotpos + 1;
  }

  // if n equals the length, there is a dot at the end, so treat it as invalid
  return (n != length);
}

nsresult
NS_NewDOMStorageList(nsIDOMStorageList** aResult)
{
  *aResult = new nsDOMStorageList();
  if (!*aResult)
    return NS_ERROR_OUT_OF_MEMORY;

  NS_ADDREF(*aResult);
  return NS_OK;
}

//
// nsDOMStorageItem
//

NS_INTERFACE_MAP_BEGIN(nsDOMStorageItem)
  NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(nsISupports, nsIDOMStorageItem)
  NS_INTERFACE_MAP_ENTRY(nsIDOMStorageItem)
  NS_INTERFACE_MAP_ENTRY(nsIDOMToString)
  NS_DOM_INTERFACE_MAP_ENTRY_CLASSINFO(StorageItem)
NS_INTERFACE_MAP_END

NS_IMPL_ADDREF(nsDOMStorageItem)
NS_IMPL_RELEASE(nsDOMStorageItem)

nsDOMStorageItem::nsDOMStorageItem(nsDOMStorage* aStorage,
                                   const nsAString& aKey,
                                   PRBool aSecure)
  : mSecure(aSecure),
    mKeyOrValue(aKey),
    mStorage(aStorage)
{
}

nsDOMStorageItem::~nsDOMStorageItem()
{
}

NS_IMETHODIMP
nsDOMStorageItem::GetSecure(PRBool* aSecure)
{
  if (!IsCallerSecure()) {
    return NS_ERROR_DOM_INVALID_ACCESS_ERR;
  }

  if (mStorage) {
    nsAutoString value;
    return mStorage->GetDBValue(mKeyOrValue, value, aSecure);
  }

  *aSecure = IsSecure();
  return NS_OK;
}

NS_IMETHODIMP
nsDOMStorageItem::SetSecure(PRBool aSecure)
{
  if (!IsCallerSecure()) {
    return NS_ERROR_DOM_INVALID_ACCESS_ERR;
  }

  if (mStorage) {
    nsresult rv = mStorage->SetSecure(mKeyOrValue, aSecure);
    NS_ENSURE_SUCCESS(rv, rv);
  }

  mSecure = aSecure;
  return NS_OK;
}

NS_IMETHODIMP
nsDOMStorageItem::GetValue(nsAString& aValue)
{
  if (mStorage) {
    // GetDBValue checks the secure state so no need to do it here
    PRBool secure;
    nsresult rv = mStorage->GetDBValue(mKeyOrValue, aValue, &secure);
    return (rv == NS_ERROR_DOM_NOT_FOUND_ERR) ? NS_OK : rv;
  }

  if (IsSecure() && !IsCallerSecure()) {
    return NS_ERROR_DOM_SECURITY_ERR;
  }

  aValue = mKeyOrValue;
  return NS_OK;
}

NS_IMETHODIMP
nsDOMStorageItem::SetValue(const nsAString& aValue)
{
  PRBool secureCaller = IsCallerSecure();

  if (mStorage) {
    // SetDBValue() does the security checks for us.
    return mStorage->SetDBValue(mKeyOrValue, aValue, secureCaller);
  }

  PRBool secureItem = IsSecure();

  if (!secureCaller && secureItem) {
    // The item is secure, but the caller isn't. Throw.

    return NS_ERROR_DOM_SECURITY_ERR;
  }

  mKeyOrValue = aValue;
  mSecure = secureCaller;
  return NS_OK;
}

NS_IMETHODIMP
nsDOMStorageItem::ToString(nsAString& aStr)
{
  return GetValue(aStr);
}

// QueryInterface implementation for nsDOMStorageEvent
NS_INTERFACE_MAP_BEGIN(nsDOMStorageEvent)
  NS_INTERFACE_MAP_ENTRY(nsIDOMStorageEvent)
  NS_DOM_INTERFACE_MAP_ENTRY_CLASSINFO(StorageEvent)
NS_INTERFACE_MAP_END_INHERITING(nsDOMEvent)

NS_IMPL_ADDREF_INHERITED(nsDOMStorageEvent, nsDOMEvent)
NS_IMPL_RELEASE_INHERITED(nsDOMStorageEvent, nsDOMEvent)


NS_IMETHODIMP
nsDOMStorageEvent::GetDomain(nsAString& aDomain)
{
  // mDomain will be #session for session storage for events that fire
  // due to a change in a session storage object.
  aDomain = mDomain;

  return NS_OK;
}

nsresult
nsDOMStorageEvent::Init()
{
  nsresult rv = InitEvent(NS_LITERAL_STRING("storage"), PR_TRUE, PR_FALSE);
  NS_ENSURE_SUCCESS(rv, rv);

  // This init method is only called by native code, so set the
  // trusted flag here.
  SetTrusted(PR_TRUE);

  return NS_OK;
}

NS_IMETHODIMP
nsDOMStorageEvent::InitStorageEvent(const nsAString& aTypeArg,
                                    PRBool aCanBubbleArg,
                                    PRBool aCancelableArg,
                                    const nsAString& aDomainArg)
{
  nsresult rv = InitEvent(aTypeArg, aCanBubbleArg, aCancelableArg);
  NS_ENSURE_SUCCESS(rv, rv);

  mDomain = aDomainArg;

  return NS_OK;
}

NS_IMETHODIMP
nsDOMStorageEvent::InitStorageEventNS(const nsAString& aNamespaceURIArg,
                                      const nsAString& aTypeArg,
                                      PRBool aCanBubbleArg,
                                      PRBool aCancelableArg,
                                      const nsAString& aDomainArg)
{
  // XXXjst: Figure out what to do with aNamespaceURIArg here!
  nsresult rv = InitEvent(aTypeArg, aCanBubbleArg, aCancelableArg);
  NS_ENSURE_SUCCESS(rv, rv);

  mDomain = aDomainArg;

  return NS_OK;
}
