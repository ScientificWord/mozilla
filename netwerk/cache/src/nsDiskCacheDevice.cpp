/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
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
 * The Original Code is nsDiskCacheDevice.cpp, released
 * February 22, 2001.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2001
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Gordon Sheridan <gordon@netscape.com>
 *   Patrick C. Beard <beard@netscape.com>
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

#include <limits.h>

// include files for ftruncate (or equivalent)
#if defined(XP_UNIX) || defined(XP_BEOS)
#include <unistd.h>
#elif defined(XP_WIN)
#include <windows.h>
#elif defined(XP_OS2)
#define INCL_DOSERRORS
#include <os2.h>
#else
// XXX add necessary include file for ftruncate (or equivalent)
#endif

#include "prtypes.h"
#include "prthread.h"
#include "prbit.h"

#include "private/pprio.h"

#include "nsDiskCacheDevice.h"
#include "nsDiskCacheEntry.h"
#include "nsDiskCacheMap.h"
#include "nsDiskCacheStreams.h"

#include "nsDiskCache.h"

#include "nsCacheService.h"
#include "nsCache.h"

#include "nsDeleteDir.h"

#include "nsICacheVisitor.h"
#include "nsReadableUtils.h"
#include "nsIInputStream.h"
#include "nsIOutputStream.h"
#include "nsAutoLock.h"
#include "nsCRT.h"
#include "nsCOMArray.h"
#include "nsISimpleEnumerator.h"

static const char DISK_CACHE_DEVICE_ID[] = { "disk" };


/******************************************************************************
 *  nsDiskCacheEvictor
 *
 *  Helper class for nsDiskCacheDevice.
 *
 *****************************************************************************/

class nsDiskCacheEvictor : public nsDiskCacheRecordVisitor
{
public:
    nsDiskCacheEvictor( nsDiskCacheMap *      cacheMap,
                        nsDiskCacheBindery *  cacheBindery,
                        PRUint32              targetSize,
                        const char *          clientID)
        : mCacheMap(cacheMap)
        , mBindery(cacheBindery)
        , mTargetSize(targetSize)
        , mClientID(clientID)
    { 
        mClientIDSize = clientID ? strlen(clientID) : 0;
    }
    
    virtual PRInt32  VisitRecord(nsDiskCacheRecord *  mapRecord);
 
private:
        nsDiskCacheMap *     mCacheMap;
        nsDiskCacheBindery * mBindery;
        PRUint32             mTargetSize;
        const char *         mClientID;
        PRUint32             mClientIDSize;
};


PRInt32
nsDiskCacheEvictor::VisitRecord(nsDiskCacheRecord *  mapRecord)
{
    if (mCacheMap->TotalSize() < mTargetSize)
        return kStopVisitingRecords;
    
    if (mClientID) {
        // we're just evicting records for a specific client
        nsDiskCacheEntry * diskEntry = mCacheMap->ReadDiskCacheEntry(mapRecord);
        if (!diskEntry)
            return kVisitNextRecord;  // XXX or delete record?
    
        // Compare clientID's without malloc
        if ((diskEntry->mKeySize <= mClientIDSize) ||
            (diskEntry->Key()[mClientIDSize] != ':') ||
            (memcmp(diskEntry->Key(), mClientID, mClientIDSize) != 0)) {
            return kVisitNextRecord;  // clientID doesn't match, skip it
        }
    }
    
    nsDiskCacheBinding * binding = mBindery->FindActiveBinding(mapRecord->HashNumber());
    if (binding) {
        // We are currently using this entry, so all we can do is doom it.
        // Since we're enumerating the records, we don't want to call
        // DeleteRecord when nsCacheService::DoomEntry() calls us back.
        binding->mDoomed = PR_TRUE;         // mark binding record as 'deleted'
        nsCacheService::DoomEntry(binding->mCacheEntry);
    } else {
        // entry not in use, just delete storage because we're enumerating the records
        (void) mCacheMap->DeleteStorage(mapRecord);
    }

    return kDeleteRecordAndContinue;  // this will REALLY delete the record
}


/******************************************************************************
 *  nsDiskCacheDeviceInfo
 *****************************************************************************/

class nsDiskCacheDeviceInfo : public nsICacheDeviceInfo {
public:
    NS_DECL_ISUPPORTS
    NS_DECL_NSICACHEDEVICEINFO

    nsDiskCacheDeviceInfo(nsDiskCacheDevice* device)
        :   mDevice(device)
    {
    }

    virtual ~nsDiskCacheDeviceInfo() {}
    
private:
    nsDiskCacheDevice* mDevice;
};

NS_IMPL_ISUPPORTS1(nsDiskCacheDeviceInfo, nsICacheDeviceInfo)

/* readonly attribute string description; */
NS_IMETHODIMP nsDiskCacheDeviceInfo::GetDescription(char ** aDescription)
{
    NS_ENSURE_ARG_POINTER(aDescription);
    *aDescription = NS_strdup("Disk cache device");
    return *aDescription ? NS_OK : NS_ERROR_OUT_OF_MEMORY;
}

/* readonly attribute string usageReport; */
NS_IMETHODIMP nsDiskCacheDeviceInfo::GetUsageReport(char ** usageReport)
{
    NS_ENSURE_ARG_POINTER(usageReport);
    nsCString buffer;
    
    buffer.AssignLiteral("\n<tr>\n<td><b>Cache Directory:</b></td>\n<td><tt> ");
    nsCOMPtr<nsILocalFile> cacheDir;
    nsAutoString           path;
    mDevice->getCacheDirectory(getter_AddRefs(cacheDir)); 
    nsresult rv = cacheDir->GetPath(path);
    if (NS_SUCCEEDED(rv)) {
        AppendUTF16toUTF8(path, buffer);
    } else {
        buffer.AppendLiteral("directory unavailable");
    }
    buffer.AppendLiteral("</tt></td>\n</tr>\n");
    // buffer.Append("<tr><td><b>Files:</b></td><td><tt> XXX</tt></td></tr>");
    *usageReport = ToNewCString(buffer);
    if (!*usageReport) return NS_ERROR_OUT_OF_MEMORY;

    return NS_OK;
}

/* readonly attribute unsigned long entryCount; */
NS_IMETHODIMP nsDiskCacheDeviceInfo::GetEntryCount(PRUint32 *aEntryCount)
{
    NS_ENSURE_ARG_POINTER(aEntryCount);
    *aEntryCount = mDevice->getEntryCount();
    return NS_OK;
}

/* readonly attribute unsigned long totalSize; */
NS_IMETHODIMP nsDiskCacheDeviceInfo::GetTotalSize(PRUint32 *aTotalSize)
{
    NS_ENSURE_ARG_POINTER(aTotalSize);
    // Returned unit's are in bytes
    *aTotalSize = mDevice->getCacheSize() * 1024;
    return NS_OK;
}

/* readonly attribute unsigned long maximumSize; */
NS_IMETHODIMP nsDiskCacheDeviceInfo::GetMaximumSize(PRUint32 *aMaximumSize)
{
    NS_ENSURE_ARG_POINTER(aMaximumSize);
    // Returned unit's are in bytes
    *aMaximumSize = mDevice->getCacheCapacity() * 1024;
    return NS_OK;
}


/******************************************************************************
 *  nsDiskCache
 *****************************************************************************/

/**
 *  nsDiskCache::Hash(const char * key)
 *
 *  This algorithm of this method implies nsDiskCacheRecords will be stored
 *  in a certain order on disk.  If the algorithm changes, existing cache
 *  map files may become invalid, and therefore the kCurrentVersion needs
 *  to be revised.
 */
PLDHashNumber
nsDiskCache::Hash(const char * key)
{
    PLDHashNumber h = 0;
    for (const PRUint8* s = (PRUint8*) key; *s != '\0'; ++s)
        h = PR_ROTATE_LEFT32(h, 4) ^ *s;
    return (h == 0 ? ULONG_MAX : h);
}


nsresult
nsDiskCache::Truncate(PRFileDesc *  fd, PRUint32  newEOF)
{
    // use modified SetEOF from nsFileStreams::SetEOF()

#if defined(XP_UNIX) || defined(XP_BEOS)
    if (ftruncate(PR_FileDesc2NativeHandle(fd), newEOF) != 0) {
        NS_ERROR("ftruncate failed");
        return NS_ERROR_FAILURE;
    }

#elif defined(XP_WIN)
    PRInt32 cnt = PR_Seek(fd, newEOF, PR_SEEK_SET);
    if (cnt == -1)  return NS_ERROR_FAILURE;
    if (!SetEndOfFile((HANDLE) PR_FileDesc2NativeHandle(fd))) {
        NS_ERROR("SetEndOfFile failed");
        return NS_ERROR_FAILURE;
    }

#elif defined(XP_OS2)
    if (DosSetFileSize((HFILE) PR_FileDesc2NativeHandle(fd), newEOF) != NO_ERROR) {
        NS_ERROR("DosSetFileSize failed");
        return NS_ERROR_FAILURE;
    }
#else
    // add implementations for other platforms here
#endif
    return NS_OK;
}


/******************************************************************************
 *  nsDiskCacheDevice
 *****************************************************************************/

nsDiskCacheDevice::nsDiskCacheDevice()
    : mCacheCapacity(0)
    , mInitialized(PR_FALSE)
{
}

nsDiskCacheDevice::~nsDiskCacheDevice()
{
    Shutdown();
}


/**
 *  methods of nsCacheDevice
 */
nsresult
nsDiskCacheDevice::Init()
{
    nsresult rv;

    NS_ENSURE_TRUE(!Initialized(), NS_ERROR_FAILURE);
       
    if (!mCacheDirectory)
        return NS_ERROR_FAILURE;

    rv = mBindery.Init();
    if (NS_FAILED(rv))
        return rv;
    
    // Open Disk Cache
    rv = OpenDiskCache();
    if (NS_FAILED(rv)) {
        (void) mCacheMap.Close(PR_FALSE);
        return rv;
    }

    mInitialized = PR_TRUE;
    return NS_OK;
}


/**
 *  NOTE: called while holding the cache service lock
 */
nsresult
nsDiskCacheDevice::Shutdown()
{
    nsresult rv = Shutdown_Private(PR_TRUE);
    if (NS_FAILED(rv))
        return rv;

    if (mCacheDirectory) {
        // delete any trash files left-over before shutting down.
        nsCOMPtr<nsIFile> trashDir;
        GetTrashDir(mCacheDirectory, &trashDir);
        if (trashDir) {
            PRBool exists;
            if (NS_SUCCEEDED(trashDir->Exists(&exists)) && exists)
                DeleteDir(trashDir, PR_FALSE, PR_TRUE);
        }
    }

    return NS_OK;
}


nsresult
nsDiskCacheDevice::Shutdown_Private(PRBool  flush)
{
    CACHE_LOG_DEBUG(("CACHE: disk Shutdown_Private [%u]\n", flush));

    if (Initialized()) {
        // check cache limits in case we need to evict.
        EvictDiskCacheEntries(mCacheCapacity);

        // write out persistent information about the cache.
        (void) mCacheMap.Close(flush);

        mBindery.Reset();

        mInitialized = PR_FALSE;
    }

    return NS_OK;
}


const char *
nsDiskCacheDevice::GetDeviceID()
{
    return DISK_CACHE_DEVICE_ID;
}


/**
 *  FindEntry -
 *
 *      cases:  key not in disk cache, hash number free
 *              key not in disk cache, hash number used
 *              key in disk cache
 *
 *  NOTE: called while holding the cache service lock
 */
nsCacheEntry *
nsDiskCacheDevice::FindEntry(nsCString * key, PRBool *collision)
{
    if (!Initialized())  return nsnull;  // NS_ERROR_NOT_INITIALIZED
    nsDiskCacheRecord       record;
    nsDiskCacheBinding *    binding = nsnull;
    PLDHashNumber           hashNumber = nsDiskCache::Hash(key->get());

    *collision = PR_FALSE;

#if DEBUG  /* because we shouldn't be called for active entries */
    binding = mBindery.FindActiveBinding(hashNumber);
    NS_ASSERTION(!binding || strcmp(binding->mCacheEntry->Key()->get(), key->get()) != 0,
                 "FindEntry() called for a bound entry.");
    binding = nsnull;
#endif
    
    // lookup hash number in cache map
    nsresult rv = mCacheMap.FindRecord(hashNumber, &record);
    if (NS_FAILED(rv))  return nsnull;  // XXX log error?
    
    nsDiskCacheEntry * diskEntry = mCacheMap.ReadDiskCacheEntry(&record);
    if (!diskEntry) return nsnull;
    
    // compare key to be sure
    if (strcmp(diskEntry->Key(), key->get()) != 0) {
        *collision = PR_TRUE;
        return nsnull;
    }
    
    nsCacheEntry * entry = diskEntry->CreateCacheEntry(this);
    if (!entry)  return nsnull;
    
    binding = mBindery.CreateBinding(entry, &record);
    if (!binding) {
        delete entry;
        return nsnull;
    }
    
    return entry;
}


/**
 *  NOTE: called while holding the cache service lock
 */
nsresult
nsDiskCacheDevice::DeactivateEntry(nsCacheEntry * entry)
{
    nsresult              rv = NS_OK;
    nsDiskCacheBinding * binding = GetCacheEntryBinding(entry);
    NS_ASSERTION(binding, "DeactivateEntry: binding == nsnull");
    if (!binding)  return NS_ERROR_UNEXPECTED;

    CACHE_LOG_DEBUG(("CACHE: disk DeactivateEntry [%p %x]\n",
        entry, binding->mRecord.HashNumber()));

    if (entry->IsDoomed()) {
        // delete data, entry, record from disk for entry
        rv = mCacheMap.DeleteStorage(&binding->mRecord);

    } else {
        // save stuff to disk for entry
        rv = mCacheMap.WriteDiskCacheEntry(binding);
        if (NS_FAILED(rv)) {
            // clean up as best we can
            (void) mCacheMap.DeleteStorage(&binding->mRecord);
            (void) mCacheMap.DeleteRecord(&binding->mRecord);
            binding->mDoomed = PR_TRUE; // record is no longer in cache map
        }
    }

    mBindery.RemoveBinding(binding); // extract binding from collision detection stuff
    delete entry;   // which will release binding
    return rv;
}


/**
 * BindEntry()
 *      no hash number collision -> no problem
 *      collision
 *          record not active -> evict, no problem
 *          record is active
 *              record is already doomed -> record shouldn't have been in map, no problem
 *              record is not doomed -> doom, and replace record in map
 *              
 *              walk matching hashnumber list to find lowest generation number
 *              take generation number from other (data/meta) location,
 *                  or walk active list
 *
 *  NOTE: called while holding the cache service lock
 */
nsresult
nsDiskCacheDevice::BindEntry(nsCacheEntry * entry)
{
    if (!Initialized())  return  NS_ERROR_NOT_INITIALIZED;
    nsresult rv = NS_OK;
    nsDiskCacheRecord record, oldRecord;
    
    // create a new record for this entry
    record.SetHashNumber(nsDiskCache::Hash(entry->Key()->get()));
    record.SetEvictionRank(ULONG_MAX - SecondsFromPRTime(PR_Now()));

    CACHE_LOG_DEBUG(("CACHE: disk BindEntry [%p %x]\n",
        entry, record.HashNumber()));

    if (!entry->IsDoomed()) {
        // if entry isn't doomed, add it to the cache map
        rv = mCacheMap.AddRecord(&record, &oldRecord); // deletes old record, if any
        if (NS_FAILED(rv))  return rv;
        
        PRUint32    oldHashNumber = oldRecord.HashNumber();
        if (oldHashNumber) {
            // gotta evict this one first
            nsDiskCacheBinding * oldBinding = mBindery.FindActiveBinding(oldHashNumber);
            if (oldBinding) {
                // XXX if debug : compare keys for hashNumber collision

                if (!oldBinding->mCacheEntry->IsDoomed()) {
                // we've got a live one!
                    nsCacheService::DoomEntry(oldBinding->mCacheEntry);
                    // storage will be delete when oldBinding->mCacheEntry is Deactivated
                }
            } else {
                // delete storage
                // XXX if debug : compare keys for hashNumber collision
                rv = mCacheMap.DeleteStorage(&oldRecord);
                if (NS_FAILED(rv))  return rv;  // XXX delete record we just added?
            }
        }
    }
    
    // Make sure this entry has its associated nsDiskCacheBinding attached.
    nsDiskCacheBinding *  binding = mBindery.CreateBinding(entry, &record);
    NS_ASSERTION(binding, "nsDiskCacheDevice::BindEntry");
    if (!binding) return NS_ERROR_OUT_OF_MEMORY;
    NS_ASSERTION(binding->mRecord.ValidRecord(), "bad cache map record");

    return NS_OK;
}


/**
 *  NOTE: called while holding the cache service lock
 */
void
nsDiskCacheDevice::DoomEntry(nsCacheEntry * entry)
{
    CACHE_LOG_DEBUG(("CACHE: disk DoomEntry [%p]\n", entry));

    nsDiskCacheBinding * binding = GetCacheEntryBinding(entry);
    NS_ASSERTION(binding, "DoomEntry: binding == nsnull");
    if (!binding)  return;

    if (!binding->mDoomed) {
        // so it can't be seen by FindEntry() ever again.
        nsresult rv = mCacheMap.DeleteRecord(&binding->mRecord);
        NS_ASSERTION(NS_SUCCEEDED(rv),"DeleteRecord failed.");
        binding->mDoomed = PR_TRUE; // record in no longer in cache map
    }
}


/**
 *  NOTE: called while holding the cache service lock
 */
nsresult
nsDiskCacheDevice::OpenInputStreamForEntry(nsCacheEntry *      entry,
                                           nsCacheAccessMode   mode, 
                                           PRUint32            offset,
                                           nsIInputStream **   result)
{
    CACHE_LOG_DEBUG(("CACHE: disk OpenInputStreamForEntry [%p %x %u]\n",
        entry, mode, offset));

    NS_ENSURE_ARG_POINTER(entry);
    NS_ENSURE_ARG_POINTER(result);

    nsresult             rv;
    nsDiskCacheBinding * binding = GetCacheEntryBinding(entry);
    NS_ENSURE_TRUE(binding, NS_ERROR_UNEXPECTED);
    
    NS_ASSERTION(binding->mCacheEntry == entry, "binding & entry don't point to each other");

    rv = binding->EnsureStreamIO();
    if (NS_FAILED(rv)) return rv;

    return binding->mStreamIO->GetInputStream(offset, result);
}


/**
 *  NOTE: called while holding the cache service lock
 */
nsresult
nsDiskCacheDevice::OpenOutputStreamForEntry(nsCacheEntry *      entry,
                                            nsCacheAccessMode   mode, 
                                            PRUint32            offset,
                                            nsIOutputStream **  result)
{
    CACHE_LOG_DEBUG(("CACHE: disk OpenOutputStreamForEntry [%p %x %u]\n",
        entry, mode, offset));
 
    NS_ENSURE_ARG_POINTER(entry);
    NS_ENSURE_ARG_POINTER(result);

    nsresult             rv;
    nsDiskCacheBinding * binding = GetCacheEntryBinding(entry);
    NS_ENSURE_TRUE(binding, NS_ERROR_UNEXPECTED);
    
    NS_ASSERTION(binding->mCacheEntry == entry, "binding & entry don't point to each other");

    rv = binding->EnsureStreamIO();
    if (NS_FAILED(rv)) return rv;

    return binding->mStreamIO->GetOutputStream(offset, result);
}


/**
 *  NOTE: called while holding the cache service lock
 */
nsresult
nsDiskCacheDevice::GetFileForEntry(nsCacheEntry *    entry,
                                   nsIFile **        result)
{
    NS_ENSURE_ARG_POINTER(result);
    *result = nsnull;

    nsresult             rv;
        
    nsDiskCacheBinding * binding = GetCacheEntryBinding(entry);
    if (!binding) {
        NS_WARNING("GetFileForEntry: binding == nsnull");
        return NS_ERROR_UNEXPECTED;
    }
    
    // check/set binding->mRecord for separate file, sync w/mCacheMap
    if (binding->mRecord.DataLocationInitialized()) {
        if (binding->mRecord.DataFile() != 0)
            return NS_ERROR_NOT_AVAILABLE;  // data not stored as separate file

        NS_ASSERTION(binding->mRecord.DataFileGeneration() == binding->mGeneration, "error generations out of sync");
    } else {
        binding->mRecord.SetDataFileGeneration(binding->mGeneration);
        binding->mRecord.SetDataFileSize(0);    // 1k minimum
        if (!binding->mDoomed) {
            // record stored in cache map, so update it
            rv = mCacheMap.UpdateRecord(&binding->mRecord);
            if (NS_FAILED(rv))  return rv;
        }
    }
    
    nsCOMPtr<nsIFile>  file;
    rv = mCacheMap.GetFileForDiskCacheRecord(&binding->mRecord,
                                             nsDiskCache::kData,
                                             getter_AddRefs(file));
    if (NS_FAILED(rv))  return rv;
    
    NS_IF_ADDREF(*result = file);
    return NS_OK;
}


/**
 *  This routine will get called every time an open descriptor is written to.
 *
 *  NOTE: called while holding the cache service lock
 */
nsresult
nsDiskCacheDevice::OnDataSizeChange(nsCacheEntry * entry, PRInt32 deltaSize)
{
    CACHE_LOG_DEBUG(("CACHE: disk OnDataSizeChange [%p %d]\n",
        entry, deltaSize));

    // If passed a negative value, then there's nothing to do.
    if (deltaSize < 0)
        return NS_OK;

    nsDiskCacheBinding * binding = GetCacheEntryBinding(entry);
    NS_ASSERTION(binding, "OnDataSizeChange: binding == nsnull");
    if (!binding)  return NS_ERROR_UNEXPECTED;

    NS_ASSERTION(binding->mRecord.ValidRecord(), "bad record");

    PRUint32  newSize = entry->DataSize() + deltaSize;
    PRUint32  newSizeK =  ((newSize + 0x3FF) >> 10);

    // If the new size is larger than max. file size or larger than
    // half the cache capacity (which is in KiB's), doom the entry and abort
    if ((newSize > kMaxDataFileSize) || (newSizeK > mCacheCapacity/2)) {
        nsresult rv = nsCacheService::DoomEntry(entry);
        NS_ASSERTION(NS_SUCCEEDED(rv),"DoomEntry() failed.");
        return NS_ERROR_ABORT;
    }

    PRUint32  sizeK = ((entry->DataSize() + 0x03FF) >> 10); // round up to next 1k

    NS_ASSERTION(sizeK < USHRT_MAX, "data size out of range");
    NS_ASSERTION(newSizeK < USHRT_MAX, "data size out of range");

    // pre-evict entries to make space for new data
    PRUint32  targetCapacity = mCacheCapacity > (newSizeK - sizeK)
                             ? mCacheCapacity - (newSizeK - sizeK)
                             : 0;
    EvictDiskCacheEntries(targetCapacity);
    
    return NS_OK;
}


/******************************************************************************
 *  EntryInfoVisitor
 *****************************************************************************/
class EntryInfoVisitor : public nsDiskCacheRecordVisitor
{
public:
    EntryInfoVisitor(nsDiskCacheMap *    cacheMap,
                     nsICacheVisitor *   visitor)
        : mCacheMap(cacheMap)
        , mVisitor(visitor)
    {}
    
    virtual PRInt32  VisitRecord(nsDiskCacheRecord *  mapRecord)
    {
        // XXX optimization: do we have this record in memory?
        
        // read in the entry (metadata)
        nsDiskCacheEntry * diskEntry = mCacheMap->ReadDiskCacheEntry(mapRecord);
        if (!diskEntry) {
            return kVisitNextRecord;
        }

        // create nsICacheEntryInfo
        nsDiskCacheEntryInfo * entryInfo = new nsDiskCacheEntryInfo(DISK_CACHE_DEVICE_ID, diskEntry);
        if (!entryInfo) {
            return kStopVisitingRecords;
        }
        nsCOMPtr<nsICacheEntryInfo> ref(entryInfo);
        
        PRBool  keepGoing;
        (void)mVisitor->VisitEntry(DISK_CACHE_DEVICE_ID, entryInfo, &keepGoing);
        return keepGoing ? kVisitNextRecord : kStopVisitingRecords;
    }
 
private:
        nsDiskCacheMap *    mCacheMap;
        nsICacheVisitor *   mVisitor;
};


nsresult
nsDiskCacheDevice::Visit(nsICacheVisitor * visitor)
{
    if (!Initialized())  return NS_ERROR_NOT_INITIALIZED;
    nsDiskCacheDeviceInfo* deviceInfo = new nsDiskCacheDeviceInfo(this);
    nsCOMPtr<nsICacheDeviceInfo> ref(deviceInfo);
    
    PRBool keepGoing;
    nsresult rv = visitor->VisitDevice(DISK_CACHE_DEVICE_ID, deviceInfo, &keepGoing);
    if (NS_FAILED(rv)) return rv;
    
    if (keepGoing) {
        EntryInfoVisitor  infoVisitor(&mCacheMap, visitor);
        return mCacheMap.VisitRecords(&infoVisitor);
    }

    return NS_OK;
}


nsresult
nsDiskCacheDevice::EvictEntries(const char * clientID)
{
    CACHE_LOG_DEBUG(("CACHE: disk EvictEntries [%s]\n", clientID));

    if (!Initialized())  return NS_ERROR_NOT_INITIALIZED;
    nsresult  rv;

    if (clientID == nsnull) {
        // we're clearing the entire disk cache
        rv = ClearDiskCache();
        if (rv != NS_ERROR_CACHE_IN_USE)
            return rv;
    }

    nsDiskCacheEvictor  evictor(&mCacheMap, &mBindery, 0, clientID);
    rv = mCacheMap.VisitRecords(&evictor);
    
    if (clientID == nsnull)     // we tried to clear the entire cache
        rv = mCacheMap.Trim(); // so trim cache block files (if possible)
    return rv;
}


/**
 *  private methods
 */

nsresult
nsDiskCacheDevice::OpenDiskCache()
{
    // if we don't have a cache directory, create one and open it
    PRBool exists;
    nsresult rv = mCacheDirectory->Exists(&exists);
    if (NS_FAILED(rv))
        return rv;

    PRBool trashing = PR_FALSE;
    if (exists) {
        // Try opening cache map file.
        rv = mCacheMap.Open(mCacheDirectory);        
        // move "corrupt" caches to trash
        if (rv == NS_ERROR_FILE_CORRUPTED) {
            rv = DeleteDir(mCacheDirectory, PR_TRUE, PR_FALSE);
            if (NS_FAILED(rv))
                return rv;
            exists = PR_FALSE;
            trashing = PR_TRUE;
        }
        else if (NS_FAILED(rv))
            return rv;
    }

    // if we don't have a cache directory, create one and open it
    if (!exists) {
        rv = mCacheDirectory->Create(nsIFile::DIRECTORY_TYPE, 0777);
        CACHE_LOG_PATH(PR_LOG_ALWAYS, "\ncreate cache directory: %s\n", mCacheDirectory);
        CACHE_LOG_ALWAYS(("mCacheDirectory->Create() = %x\n", rv));
        if (NS_FAILED(rv))
            return rv;
    
        // reopen the cache map     
        rv = mCacheMap.Open(mCacheDirectory);
        if (NS_FAILED(rv))
            return rv;
    }

    if (!trashing) {
        // delete any trash files leftover from a previous run
        nsCOMPtr<nsIFile> trashDir;
        GetTrashDir(mCacheDirectory, &trashDir);
        if (trashDir) {
            PRBool exists;
            if (NS_SUCCEEDED(trashDir->Exists(&exists)) && exists)
                DeleteDir(trashDir, PR_FALSE, PR_FALSE);
        }
    }

    return NS_OK;
}


nsresult
nsDiskCacheDevice::ClearDiskCache()
{
    if (mBindery.ActiveBindings())
        return NS_ERROR_CACHE_IN_USE;

    nsresult rv = Shutdown_Private(PR_FALSE);  // false: don't bother flushing
    if (NS_FAILED(rv))
        return rv;

    // If the disk cache directory is already gone, then it's not an error if
    // we fail to delete it ;-)
    rv = DeleteDir(mCacheDirectory, PR_TRUE, PR_FALSE);
    if (NS_FAILED(rv) && rv != NS_ERROR_FILE_TARGET_DOES_NOT_EXIST)
        return rv;

    return Init();
}


nsresult
nsDiskCacheDevice::EvictDiskCacheEntries(PRUint32  targetCapacity)
{
    CACHE_LOG_DEBUG(("CACHE: disk EvictDiskCacheEntries [%u]\n",
        targetCapacity));

    NS_ASSERTION(targetCapacity > 0, "oops");

    if (mCacheMap.TotalSize() < targetCapacity)
        return NS_OK;

    // targetCapacity is in KiB's
    nsDiskCacheEvictor  evictor(&mCacheMap, &mBindery, targetCapacity, nsnull);
    return mCacheMap.EvictRecords(&evictor);
}


/**
 *  methods for prefs
 */

void
nsDiskCacheDevice::SetCacheParentDirectory(nsILocalFile * parentDir)
{
    nsresult rv;
    PRBool  exists;

    if (Initialized()) {
        NS_ASSERTION(PR_FALSE, "Cannot switch cache directory when initialized");
        return;
    }

    if (!parentDir) {
        mCacheDirectory = nsnull;
        return;
    }

    // ensure parent directory exists
    rv = parentDir->Exists(&exists);
    if (NS_SUCCEEDED(rv) && !exists)
        rv = parentDir->Create(nsIFile::DIRECTORY_TYPE, 0700);
    if (NS_FAILED(rv))  return;

    // ensure cache directory exists
    nsCOMPtr<nsIFile> directory;
    
    rv = parentDir->Clone(getter_AddRefs(directory));
    if (NS_FAILED(rv))  return;
    rv = directory->AppendNative(NS_LITERAL_CSTRING("Cache"));
    if (NS_FAILED(rv))  return;
    
    mCacheDirectory = do_QueryInterface(directory);
}


void
nsDiskCacheDevice::getCacheDirectory(nsILocalFile ** result)
{
    *result = mCacheDirectory;
    NS_IF_ADDREF(*result);
}


/**
 *  NOTE: called while holding the cache service lock
 */
void
nsDiskCacheDevice::SetCapacity(PRUint32  capacity)
{
    // Units are KiB's
    mCacheCapacity = capacity;
    if (Initialized()) {
        // start evicting entries if the new size is smaller!
        EvictDiskCacheEntries(mCacheCapacity);
    }
}


PRUint32 nsDiskCacheDevice::getCacheCapacity()
{
    return mCacheCapacity;
}


PRUint32 nsDiskCacheDevice::getCacheSize()
{
    return mCacheMap.TotalSize();
}


PRUint32 nsDiskCacheDevice::getEntryCount()
{
    return mCacheMap.EntryCount();
}
