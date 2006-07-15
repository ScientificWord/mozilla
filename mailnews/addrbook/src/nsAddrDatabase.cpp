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
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1999
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Pierre Phaneuf <pp@ludusdesign.com>
 *   Mark Banner <mark@standard8.demon.co.uk>
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

// this file implements the nsAddrDatabase interface using the MDB Interface.

#include "nsAddrDatabase.h"
#include "nsIEnumerator.h"
#include "nsString.h"
#include "nsReadableUtils.h"
#include "nsAutoPtr.h"
#include "nsRDFCID.h"
#include "nsUnicharUtils.h"
#include "nsMsgUtils.h"
#include "nsAbBaseCID.h"
#include "nsIAbCard.h"
#include "nsIAbMDBCard.h"
#include "nsIAbDirectory.h"
#include "nsIAbMDBDirectory.h"
#include "nsIAddrBookSession.h"

#include "nsIServiceManager.h"
#include "nsRDFCID.h"

#include "nsMorkCID.h"
#include "nsIMdbFactoryFactory.h"
#include "nsXPIDLString.h"
#include "nsIRDFService.h"
#include "nsIProxyObjectManager.h"
#include "nsProxiedService.h"
#include "prprf.h"

#include "nsIPromptService.h"
#include "nsIStringBundle.h"
#include "nsIFile.h"

#include "nsAddressBook.h" // for the map

#include "nsEmbedCID.h"

#define ID_PAB_TABLE            1
#define ID_DELETEDCARDS_TABLE           2

const PRInt32 kAddressBookDBVersion = 1;

static const char kPabTableKind[] = "ns:addrbk:db:table:kind:pab";
static const char kDeletedCardsTableKind[] = "ns:addrbk:db:table:kind:deleted"; // this table is used to keep the deleted cards

static const char kCardRowScope[] = "ns:addrbk:db:row:scope:card:all";
static const char kListRowScope[] = "ns:addrbk:db:row:scope:list:all";
static const char kDataRowScope[] = "ns:addrbk:db:row:scope:data:all";

#define DATAROW_ROWID 1

#define COLUMN_STR_MAX 16

#define PURGE_CUTOFF_COUNT 50

static const char kRecordKeyColumn[] = "RecordKey";
static const char kLastRecordKeyColumn[] = "LastRecordKey";

static const char kMailListTotalLists[] = "ListTotalLists";    // total number of mail list in a mailing list
static const char kLowerListNameColumn[] = "LowercaseListName";

struct mdbOid gAddressBookTableOID;

static const char kMailListAddressFormat[] = "Address%d";

nsAddrDatabase::nsAddrDatabase()
    : m_mdbEnv(nsnull), m_mdbStore(nsnull),
      m_mdbPabTable(nsnull), 
      m_mdbDeletedCardsTable(nsnull),
      m_mdbTokensInitialized(PR_FALSE), 
      m_ChangeListeners(nsnull),
      m_PabTableKind(0),
      m_MailListTableKind(0),
      m_DeletedCardsTableKind(0),
      m_CardRowScopeToken(0),
      m_FirstNameColumnToken(0),
      m_LastNameColumnToken(0),
      m_PhoneticFirstNameColumnToken(0),
      m_PhoneticLastNameColumnToken(0),
      m_DisplayNameColumnToken(0),
      m_NickNameColumnToken(0),
      m_PriEmailColumnToken(0),
      m_2ndEmailColumnToken(0),
      m_WorkPhoneColumnToken(0),
      m_HomePhoneColumnToken(0),
      m_FaxColumnToken(0),
      m_PagerColumnToken(0),
      m_CellularColumnToken(0),
      m_WorkPhoneTypeColumnToken(0),
      m_HomePhoneTypeColumnToken(0),
      m_FaxTypeColumnToken(0),
      m_PagerTypeColumnToken(0),
      m_CellularTypeColumnToken(0),
      m_HomeAddressColumnToken(0),
      m_HomeAddress2ColumnToken(0),
      m_HomeCityColumnToken(0),
      m_HomeStateColumnToken(0),
      m_HomeZipCodeColumnToken(0),
      m_HomeCountryColumnToken(0),
      m_WorkAddressColumnToken(0),
      m_WorkAddress2ColumnToken(0),
      m_WorkCityColumnToken(0),
      m_WorkStateColumnToken(0),
      m_WorkZipCodeColumnToken(0),
      m_WorkCountryColumnToken(0),
      m_CompanyColumnToken(0),
      m_AimScreenNameColumnToken(0),
      m_AnniversaryYearColumnToken(0),
      m_AnniversaryMonthColumnToken(0),
      m_AnniversaryDayColumnToken(0),
      m_SpouseNameColumnToken(0),
      m_FamilyNameColumnToken(0),
      m_DefaultAddressColumnToken(0),
      m_CategoryColumnToken(0),
      m_WebPage1ColumnToken(0),
      m_WebPage2ColumnToken(0),
      m_BirthYearColumnToken(0),
      m_BirthMonthColumnToken(0),
      m_BirthDayColumnToken(0),
      m_Custom1ColumnToken(0),
      m_Custom2ColumnToken(0),
      m_Custom3ColumnToken(0),
      m_Custom4ColumnToken(0),
      m_NotesColumnToken(0),
      m_LastModDateColumnToken(0),
      m_MailFormatColumnToken(0),
      m_PopularityIndexColumnToken(0),
      m_AddressCharSetColumnToken(0),
      m_LastRecordKey(0),
      m_dbDirectory(nsnull)
{
}

nsAddrDatabase::~nsAddrDatabase()
{
    Close(PR_FALSE);    // better have already been closed.
    if (m_ChangeListeners)
    {
        // better not be any listeners, because we're going away.
        NS_ASSERTION(m_ChangeListeners->Count() == 0, "shouldn't have any listeners");
        delete m_ChangeListeners;
    }

    RemoveFromCache(this);
}

NS_IMPL_THREADSAFE_ADDREF(nsAddrDatabase)

NS_IMETHODIMP_(nsrefcnt) nsAddrDatabase::Release(void)                    
{                                                      
  // XXX FIX THIS
  NS_PRECONDITION(0 != mRefCnt, "dup release");        
  nsrefcnt count = PR_AtomicDecrement((PRInt32 *)&mRefCnt);
  NS_LOG_RELEASE(this, count,"nsAddrDatabase"); 
  if (count == 0)    // OK, the cache is no longer holding onto this, so we really want to delete it, 
  {                // after removing it from the cache.
    mRefCnt = 1; /* stabilize */
    RemoveFromCache(this);
    // clean up after ourself!
    if (m_mdbPabTable)
      m_mdbPabTable->Release();
    if (m_mdbDeletedCardsTable)
      m_mdbDeletedCardsTable->Release();
    NS_IF_RELEASE(m_mdbStore);
    NS_IF_RELEASE(m_mdbEnv);
    NS_DELETEXPCOM(this);                              
    return 0;                                          
  }
  return count;                                      
}

NS_IMETHODIMP nsAddrDatabase::QueryInterface(REFNSIID aIID, void** aResult)
{   
    if (aResult == NULL)  
        return NS_ERROR_NULL_POINTER;  

    if (aIID.Equals(NS_GET_IID(nsIAddrDatabase)) ||
        aIID.Equals(NS_GET_IID(nsIAddrDBAnnouncer)) ||
        aIID.Equals(NS_GET_IID(nsISupports))) {
        *aResult = NS_STATIC_CAST(nsIAddrDatabase*, this);   
        NS_ADDREF_THIS();
        return NS_OK;
    }
    return NS_NOINTERFACE;
}   

NS_IMETHODIMP nsAddrDatabase::AddListener(nsIAddrDBListener *listener)
{
  if (!listener)
    return NS_ERROR_NULL_POINTER;
  if (!m_ChangeListeners) 
    {
    m_ChangeListeners = new nsVoidArray();
    if (!m_ChangeListeners) 
            return NS_ERROR_OUT_OF_MEMORY;
  }
    PRInt32 count = m_ChangeListeners->Count();
    PRInt32 i;
    for (i = 0; i < count; i++)
    {
        nsIAddrDBListener *dbListener = (nsIAddrDBListener *)m_ChangeListeners->ElementAt(i);
        if (dbListener == listener)
            return NS_OK;
    }
    return m_ChangeListeners->AppendElement(listener);
}

NS_IMETHODIMP nsAddrDatabase::RemoveListener(nsIAddrDBListener *listener)
{
    if (!m_ChangeListeners) 
        return NS_OK;

    PRInt32 count = m_ChangeListeners->Count();
    PRInt32 i;
    for (i = 0; i < count; i++)
    {
        nsIAddrDBListener *dbListener = (nsIAddrDBListener *)m_ChangeListeners->ElementAt(i);
        if (dbListener == listener)
        {
            m_ChangeListeners->RemoveElementAt(i);
            return NS_OK;
        }
    }
    return NS_ERROR_FAILURE;
}

NS_IMETHODIMP nsAddrDatabase::NotifyCardAttribChange(PRUint32 abCode)
{
  if (!m_ChangeListeners)
      return NS_OK;
    PRInt32 i;
    for (i = 0; i < m_ChangeListeners->Count(); i++)
    {
        nsIAddrDBListener *changeListener =
            (nsIAddrDBListener *) m_ChangeListeners->ElementAt(i);

        nsresult rv = changeListener->OnCardAttribChange(abCode);
    NS_ENSURE_SUCCESS(rv, rv);
    }
    return NS_OK;
}

NS_IMETHODIMP nsAddrDatabase::NotifyCardEntryChange(PRUint32 abCode, nsIAbCard *card)
{
  if (!m_ChangeListeners)
      return NS_OK;
  PRInt32 i;
  PRInt32 count = m_ChangeListeners->Count();
  for (i = count - 1; i >= 0; i--)
  {
    nsIAddrDBListener *changeListener = 
            (nsIAddrDBListener *) m_ChangeListeners->ElementAt(i);

    if (changeListener)
    {
      nsresult rv = changeListener->OnCardEntryChange(abCode, card);
      NS_ENSURE_SUCCESS(rv, rv);
    }
    else
      m_ChangeListeners->RemoveElementAt(i); //remove null ptr in the list
    // since we're looping down, this is ok
  }
  return NS_OK;
}

nsresult nsAddrDatabase::NotifyListEntryChange(PRUint32 abCode, nsIAbDirectory *dir)
{
  if (!m_ChangeListeners)
        return NS_OK;

    PRInt32 i;
    PRInt32 count = m_ChangeListeners->Count();
    for (i = 0; i < count; i++)
    {
        nsIAddrDBListener *changeListener = 
            (nsIAddrDBListener *) m_ChangeListeners->ElementAt(i);

        nsresult rv = changeListener->OnListEntryChange(abCode, dir); 
    NS_ENSURE_SUCCESS(rv, rv);
    }
    return NS_OK;
}


NS_IMETHODIMP nsAddrDatabase::NotifyAnnouncerGoingAway(void)
{
  if (!m_ChangeListeners)
    return NS_OK;
    // run loop backwards because listeners remove themselves from the list 
    // on this notification
    PRInt32 i;
    for (i = m_ChangeListeners->Count() - 1; i >= 0 ; i--)
    {
        nsIAddrDBListener *changeListener =
            (nsIAddrDBListener *) m_ChangeListeners->ElementAt(i);

        nsresult rv = changeListener->OnAnnouncerGoingAway(); 
    NS_ENSURE_SUCCESS(rv, rv);
    }
  return NS_OK;
}



nsVoidArray *nsAddrDatabase::m_dbCache = NULL;

//----------------------------------------------------------------------
// GetDBCache
//----------------------------------------------------------------------

nsVoidArray/*<nsAddrDatabase>*/ *
nsAddrDatabase::GetDBCache()
{
    if (!m_dbCache)
        m_dbCache = new nsVoidArray();

    return m_dbCache;
    
}

void
nsAddrDatabase::CleanupCache()
{
    if (m_dbCache) // clean up memory leak
    {
        PRInt32 i;
        for (i = 0; i < GetDBCache()->Count(); i++)
        {
            nsAddrDatabase* pAddrDB = NS_STATIC_CAST(nsAddrDatabase*, GetDBCache()->ElementAt(i));
            if (pAddrDB)
            {
                pAddrDB->ForceClosed();
                i--;    // back up array index, since closing removes db from cache.
            }
        }
//        NS_ASSERTION(GetNumInCache() == 0, "some msg dbs left open");    // better not be any open db's.
        delete m_dbCache;
    }
    m_dbCache = nsnull; // Need to reset to NULL since it's a
              // static global ptr and maybe referenced 
              // again in other places.
}

//----------------------------------------------------------------------
// FindInCache - this addrefs the db it finds.
//----------------------------------------------------------------------
nsAddrDatabase* nsAddrDatabase::FindInCache(nsIFile *dbName)
{
    PRInt32 i;
    for (i = 0; i < GetDBCache()->Count(); i++)
    {
        nsAddrDatabase* pAddrDB = NS_STATIC_CAST(nsAddrDatabase*, GetDBCache()->ElementAt(i));
        if (pAddrDB->MatchDbName(dbName))
        {
            NS_ADDREF(pAddrDB);
            return pAddrDB;
        }
    }
    return nsnull;
}

//----------------------------------------------------------------------
// FindInCache
//----------------------------------------------------------------------
PRInt32 nsAddrDatabase::FindInCache(nsAddrDatabase* pAddrDB)
{
    PRInt32 i;
    for (i = 0; i < GetDBCache()->Count(); i++)
    {
        if (GetDBCache()->ElementAt(i) == pAddrDB)
        {
            return(i);
        }
    }
    return(-1);
}

PRBool nsAddrDatabase::MatchDbName(nsIFile* dbName)    // returns PR_TRUE if they match
{
    PRBool dbMatches = PR_FALSE;

    nsresult rv = m_dbName->Equals(dbName, &dbMatches);
    if (NS_FAILED(rv))
      return PR_FALSE;

    return dbMatches;
}

//----------------------------------------------------------------------
// RemoveFromCache
//----------------------------------------------------------------------
void nsAddrDatabase::RemoveFromCache(nsAddrDatabase* pAddrDB)
{
    PRInt32 i = FindInCache(pAddrDB);
    if (i != -1)
    {
        GetDBCache()->RemoveElementAt(i);
    }
}

nsIMdbFactory *nsAddrDatabase::GetMDBFactory()
{
    static nsIMdbFactory *gMDBFactory = nsnull;
    if (!gMDBFactory)
    {
        nsresult rv;
    nsCOMPtr <nsIMdbFactoryFactory> factoryfactory = do_CreateInstance(NS_MORK_CONTRACTID, &rv);

        if (NS_SUCCEEDED(rv) && factoryfactory)
          rv = factoryfactory->GetMdbFactory(&gMDBFactory);
    }
    return gMDBFactory;
}

/* caller need to delete *aDbPath */
NS_IMETHODIMP nsAddrDatabase::GetDbPath(nsIFile* *aDbPath)
{
    if (!aDbPath)
        return NS_ERROR_NULL_POINTER;

    return m_dbName->Clone(aDbPath);
}

NS_IMETHODIMP nsAddrDatabase::SetDbPath(nsIFile* aDbPath)
{
    return aDbPath->Clone(getter_AddRefs(m_dbName));
}

NS_IMETHODIMP nsAddrDatabase::Open
(nsIFile *aMabFile, PRBool aCreate, PRBool upgrading /* unused */, nsIAddrDatabase** pAddrDB)
{ 
  *pAddrDB = nsnull;
  
  nsAddrDatabase *pAddressBookDB = FindInCache(aMabFile);

  if (pAddressBookDB) {
    *pAddrDB = pAddressBookDB;
    return NS_OK;
  }
  
  nsresult rv = OpenInternal(aMabFile, aCreate, pAddrDB);
  if (NS_SUCCEEDED(rv))
    return NS_OK;
  
  if (rv == NS_ERROR_FILE_ACCESS_DENIED)
  {
    static PRBool gAlreadyAlerted;
     // only do this once per session to avoid annoying the user
    if (!gAlreadyAlerted)
    {
      gAlreadyAlerted = PR_TRUE;
      nsAutoString mabFileName;
      rv = aMabFile->GetLeafName(mabFileName);
      NS_ENSURE_SUCCESS(rv, rv);
      AlertAboutLockedMabFile(mabFileName.get());
    }
  }
  // try one more time
  // but first rename corrupt mab file
  // and prompt the user
  else if (aCreate) 
  {
    nsCOMPtr<nsIFile> dummyBackupMabFile;
    nsCOMPtr<nsIFile> actualBackupMabFile;

    // First create a clone of the corrupt mab file that we'll
    // use to generate the name for the backup file that we are
    // going to move it to.
    rv = aMabFile->Clone(getter_AddRefs(dummyBackupMabFile));
    NS_ENSURE_SUCCESS(rv, rv);

    // Now create a second clone that we'll use to do the move
    // (this allows us to leave the original name intact)
    rv = aMabFile->Clone(getter_AddRefs(actualBackupMabFile));
    NS_ENSURE_SUCCESS(rv, rv);

    // Now we try and generate a new name for the corrupt mab
    // file using the dummy backup mab file

    // First append .bak - we have to do this the long way as
    // AppendNative is to the path, not the LeafName.
    nsCAutoString dummyBackupMabFileName;
    rv = dummyBackupMabFile->GetNativeLeafName(dummyBackupMabFileName);
    NS_ENSURE_SUCCESS(rv, rv);

    dummyBackupMabFileName.Append(NS_LITERAL_CSTRING(".bak"));

    rv = dummyBackupMabFile->SetNativeLeafName(dummyBackupMabFileName);
    NS_ENSURE_SUCCESS(rv, rv);

    // Now see if we can create it unique
    rv = dummyBackupMabFile->CreateUnique(nsIFile::NORMAL_FILE_TYPE, 0600);
    NS_ENSURE_SUCCESS(rv, rv);

    // Now get the new name
    nsCAutoString backupMabFileName;
    rv = dummyBackupMabFile->GetNativeLeafName(backupMabFileName);
    NS_ENSURE_SUCCESS(rv, rv);

    // And the parent directory
    nsCOMPtr<nsIFile> parentDir;
    rv = dummyBackupMabFile->GetParent(getter_AddRefs(parentDir));
    NS_ENSURE_SUCCESS(rv, rv);

    // Now move the corrupt file to its backup location
    rv = actualBackupMabFile->MoveToNative(parentDir, backupMabFileName);
    NS_ASSERTION(NS_SUCCEEDED(rv), "failed to rename corrupt mab file");

    if (NS_SUCCEEDED(rv)) {
      // now we can try to recreate the original mab file
      rv = OpenInternal(aMabFile, aCreate, pAddrDB);
      NS_ASSERTION(NS_SUCCEEDED(rv), "failed to create .mab file, after rename");

      if (NS_SUCCEEDED(rv)) {
        nsAutoString originalMabFileName;
        rv = aMabFile->GetLeafName(originalMabFileName);
        NS_ENSURE_SUCCESS(rv, rv);

        // if this fails, we don't care
        (void)AlertAboutCorruptMabFile(originalMabFileName.get(), 
          NS_ConvertASCIItoUTF16(backupMabFileName).get());
      }
    }
  }
  return rv;
}

nsresult nsAddrDatabase::DisplayAlert(const PRUnichar *titleName, const PRUnichar *alertStringName, const PRUnichar **formatStrings, PRInt32 numFormatStrings)
{
  nsresult rv;
  nsCOMPtr<nsIStringBundleService> bundleService = do_GetService(NS_STRINGBUNDLE_CONTRACTID, &rv);
  NS_ENSURE_SUCCESS(rv, rv);

  nsCOMPtr<nsIStringBundle> bundle;
  rv = bundleService->CreateBundle("chrome://messenger/locale/addressbook/addressBook.properties", getter_AddRefs(bundle));
  NS_ENSURE_SUCCESS(rv, rv);
  
  nsXPIDLString alertMessage;
  rv = bundle->FormatStringFromName(alertStringName, formatStrings, numFormatStrings,
    getter_Copies(alertMessage));
  NS_ENSURE_SUCCESS(rv, rv);

  nsXPIDLString alertTitle;
  rv = bundle->GetStringFromName(titleName, getter_Copies(alertTitle));
  NS_ENSURE_SUCCESS(rv, rv);

  nsCOMPtr<nsIPromptService> prompter =
      do_GetService(NS_PROMPTSERVICE_CONTRACTID, &rv);
  NS_ENSURE_SUCCESS(rv, rv);

  return prompter->Alert(nsnull /* we don't know the parent window */, alertTitle.get(), alertMessage.get());
}

nsresult nsAddrDatabase::AlertAboutCorruptMabFile(const PRUnichar *aOldFileName, const PRUnichar *aNewFileName)
{
  const PRUnichar *formatStrings[] = { aOldFileName, aOldFileName, aNewFileName };
  return DisplayAlert(NS_LITERAL_STRING("corruptMabFileTitle").get(),
    NS_LITERAL_STRING("corruptMabFileAlert").get(), formatStrings, 3);
}

nsresult nsAddrDatabase::AlertAboutLockedMabFile(const PRUnichar *aFileName)
{
  const PRUnichar *formatStrings[] = { aFileName };
  return DisplayAlert(NS_LITERAL_STRING("lockedMabFileTitle").get(),
    NS_LITERAL_STRING("lockedMabFileAlert").get(), formatStrings, 1);
}

nsresult
nsAddrDatabase::OpenInternal(nsIFile *aMabFile, PRBool aCreate, nsIAddrDatabase** pAddrDB)
{    
  nsAddrDatabase *pAddressBookDB = new nsAddrDatabase();
  if (!pAddressBookDB) {
    return NS_ERROR_OUT_OF_MEMORY;
  }
  
  NS_ADDREF(pAddressBookDB);
  
  nsresult rv = pAddressBookDB->OpenMDB(aMabFile, aCreate);
  if (NS_SUCCEEDED(rv)) 
  {
    pAddressBookDB->SetDbPath(aMabFile);
    GetDBCache()->AppendElement(pAddressBookDB);
    *pAddrDB = pAddressBookDB;
  }
  else 
  {
    *pAddrDB = nsnull;
    pAddressBookDB->ForceClosed();
    NS_IF_RELEASE(pAddressBookDB);
    pAddressBookDB = nsnull;
  }
  return rv;
}

// Open the MDB database synchronously. If successful, this routine
// will set up the m_mdbStore and m_mdbEnv of the database object 
// so other database calls can work.
NS_IMETHODIMP nsAddrDatabase::OpenMDB(nsIFile *dbName, PRBool create)
{
  nsresult ret = NS_OK;
  nsIMdbFactory *myMDBFactory = GetMDBFactory();
  if (myMDBFactory)
  {
    ret = myMDBFactory->MakeEnv(NULL, &m_mdbEnv);
    if (NS_SUCCEEDED(ret))
    {
      nsIMdbThumb *thumb = nsnull;
      nsCAutoString filePath;

      ret = dbName->GetNativePath(filePath);
      NS_ENSURE_SUCCESS(ret, ret);

      nsIMdbHeap* dbHeap = 0;
      mdb_bool dbFrozen = mdbBool_kFalse; // not readonly, we want modifiable
      
      if (m_mdbEnv)
        m_mdbEnv->SetAutoClear(PR_TRUE);
      
      PRBool dbNameExists = PR_FALSE;
      ret = dbName->Exists(&dbNameExists);
      NS_ENSURE_SUCCESS(ret, ret);

      if (!dbNameExists) 
        ret = NS_ERROR_FILE_NOT_FOUND;
      else
      {
        mdbOpenPolicy inOpenPolicy;
        mdb_bool    canOpen;
        mdbYarn        outFormatVersion;
        nsIMdbFile* oldFile = 0;
        PRInt64 fileSize;
        ret = dbName->GetFileSize(&fileSize);
        NS_ENSURE_SUCCESS(ret, ret);
        
        ret = myMDBFactory->OpenOldFile(m_mdbEnv, dbHeap, filePath.get(),
          dbFrozen, &oldFile);
        if ( oldFile )
        {
          if ( ret == NS_OK )
          {
            ret = myMDBFactory->CanOpenFilePort(m_mdbEnv, oldFile, // the file to investigate
              &canOpen, &outFormatVersion);
            if (ret == 0 && canOpen)
            {
              inOpenPolicy.mOpenPolicy_ScopePlan.mScopeStringSet_Count = 0;
              inOpenPolicy.mOpenPolicy_MinMemory = 0;
              inOpenPolicy.mOpenPolicy_MaxLazy = 0;
              
              ret = myMDBFactory->OpenFileStore(m_mdbEnv, dbHeap,
                oldFile, &inOpenPolicy, &thumb); 
            }
            else if (fileSize != 0)
              ret = NS_ERROR_FILE_ACCESS_DENIED;
          }
          NS_RELEASE(oldFile); // always release our file ref, store has own
        }
        if (NS_FAILED(ret))
          ret = NS_ERROR_FILE_ACCESS_DENIED;
      }
      
      if (NS_SUCCEEDED(ret) && thumb)
      {
        mdb_count outTotal;    // total somethings to do in operation
        mdb_count outCurrent;  // subportion of total completed so far
        mdb_bool outDone = PR_FALSE;      // is operation finished?
        mdb_bool outBroken;     // is operation irreparably dead and broken?
        do
        {
          ret = thumb->DoMore(m_mdbEnv, &outTotal, &outCurrent, &outDone, &outBroken);
          if (ret != 0)
          { 
            outDone = PR_TRUE;
            break;
          }
        }
        while (NS_SUCCEEDED(ret) && !outBroken && !outDone);
        if (NS_SUCCEEDED(ret) && outDone)
        {
          ret = myMDBFactory->ThumbToOpenStore(m_mdbEnv, thumb, &m_mdbStore);
          if (ret == NS_OK && m_mdbStore)
          {
            ret = InitExistingDB();
            create = PR_FALSE;
          }
        }
      }
      else if (create && ret != NS_ERROR_FILE_ACCESS_DENIED)
      {
        nsIMdbFile* newFile = 0;
        ret = myMDBFactory->CreateNewFile(m_mdbEnv, dbHeap, filePath.get(), &newFile);
        if ( newFile )
        {
          if (ret == NS_OK)
          {
            mdbOpenPolicy inOpenPolicy;
            
            inOpenPolicy.mOpenPolicy_ScopePlan.mScopeStringSet_Count = 0;
            inOpenPolicy.mOpenPolicy_MinMemory = 0;
            inOpenPolicy.mOpenPolicy_MaxLazy = 0;
            
            ret = myMDBFactory->CreateNewFileStore(m_mdbEnv, dbHeap,
                                                   newFile, &inOpenPolicy,
                                                   &m_mdbStore);
            if (ret == NS_OK)
              ret = InitNewDB();
          }
          NS_RELEASE(newFile); // always release our file ref, store has own
        }
      }
      NS_IF_RELEASE(thumb);
    }
  }
  //Convert the DB error to a valid nsresult error.
  if (ret == 1)
    ret = NS_ERROR_FAILURE;
  return ret;
}

NS_IMETHODIMP nsAddrDatabase::CloseMDB(PRBool commit)
{
    if (commit)
        Commit(nsAddrDBCommitType::kSessionCommit);
//???    RemoveFromCache(this);  // if we've closed it, better not leave it in the cache.
    return NS_OK;
}

// force the database to close - this'll flush out anybody holding onto
// a database without having a listener!
// This is evil in the com world, but there are times we need to delete the file.
NS_IMETHODIMP nsAddrDatabase::ForceClosed()
{
    nsresult    err = NS_OK;
    nsCOMPtr<nsIAddrDatabase> aDb(do_QueryInterface(this, &err));

    // make sure someone has a reference so object won't get deleted out from under us.
    AddRef();    
    NotifyAnnouncerGoingAway();
    // OK, remove from cache first and close the store.
    RemoveFromCache(this);

    err = CloseMDB(PR_FALSE);    // since we're about to delete it, no need to commit.
    NS_IF_RELEASE(m_mdbStore);
    Release();
    return err;
}

NS_IMETHODIMP nsAddrDatabase::Commit(PRUint32 commitType)
{
  nsresult err = NS_OK;
  nsIMdbThumb *commitThumb = nsnull;

  if (commitType == nsAddrDBCommitType::kLargeCommit ||
      commitType == nsAddrDBCommitType::kSessionCommit)
  {
    mdb_percent outActualWaste = 0;
    mdb_bool outShould;
    if (m_mdbStore && m_mdbEnv) 
    {
      // check how much space would be saved by doing a compress commit.
      // If it's more than 30%, go for it.
      // N.B. - I'm not sure this calls works in Mork for all cases.
      err = m_mdbStore->ShouldCompress(m_mdbEnv, 30, &outActualWaste, &outShould);
      if (NS_SUCCEEDED(err) && outShould)
      {
        commitType = nsAddrDBCommitType::kCompressCommit;
      }
    }
  }

  if (m_mdbStore && m_mdbEnv)
  {
    switch (commitType)
    {
      case nsAddrDBCommitType::kSmallCommit:
        err = m_mdbStore->SmallCommit(m_mdbEnv);
        break;
      case nsAddrDBCommitType::kLargeCommit:
        err = m_mdbStore->LargeCommit(m_mdbEnv, &commitThumb);
        break;
      case nsAddrDBCommitType::kSessionCommit:
        // comment out until persistence works.
        err = m_mdbStore->SessionCommit(m_mdbEnv, &commitThumb);
        break;
      case nsAddrDBCommitType::kCompressCommit:
        err = m_mdbStore->CompressCommit(m_mdbEnv, &commitThumb);
        break;
      }
  }
  if (commitThumb && m_mdbEnv)
  {
    mdb_count outTotal = 0;    // total somethings to do in operation
    mdb_count outCurrent = 0;  // subportion of total completed so far
    mdb_bool outDone = PR_FALSE;      // is operation finished?
    mdb_bool outBroken = PR_FALSE;     // is operation irreparably dead and broken?
    while (!outDone && !outBroken && err == NS_OK)
    {
      err = commitThumb->DoMore(m_mdbEnv, &outTotal, &outCurrent, &outDone, &outBroken);
    }
    NS_RELEASE(commitThumb);
  }
  // ### do something with error, but clear it now because mork errors out on commits.
  if (m_mdbEnv)
    m_mdbEnv->ClearErrors();
  return err;
}

NS_IMETHODIMP nsAddrDatabase::Close(PRBool forceCommit /* = TRUE */)
{
    return CloseMDB(forceCommit);
}

// set up empty tablesetc.
nsresult nsAddrDatabase::InitNewDB()
{
  nsresult err = InitMDBInfo();
  if (NS_SUCCEEDED(err))
  {
    err = InitPabTable();
    err = InitLastRecorKey();
    Commit(nsAddrDBCommitType::kLargeCommit);
  }
  return err;
}

nsresult nsAddrDatabase::AddRowToDeletedCardsTable(nsIAbCard *card, nsIMdbRow **pCardRow)
{
  if (!m_mdbEnv)
    return NS_ERROR_NULL_POINTER;

  nsresult rv = NS_OK;
  if (!m_mdbDeletedCardsTable)
    rv = InitDeletedCardsTable(PR_TRUE);
  if (NS_SUCCEEDED(rv)) {
    // lets first purge old records if there are more than PURGE_CUTOFF_COUNT records
    PurgeDeletedCardTable();
    nsCOMPtr<nsIMdbRow> cardRow;
    rv = GetNewRow(getter_AddRefs(cardRow));
    if (NS_SUCCEEDED(rv) && cardRow) {
      mdb_err merror = m_mdbDeletedCardsTable->AddRow(m_mdbEnv, cardRow);
      if (merror != NS_OK) return NS_ERROR_FAILURE;
      nsXPIDLString unicodeStr;
      card->GetFirstName(getter_Copies(unicodeStr));
      AddFirstName(cardRow, NS_ConvertUTF16toUTF8(unicodeStr).get());
    
      card->GetLastName(getter_Copies(unicodeStr));
      AddLastName(cardRow, NS_ConvertUTF16toUTF8(unicodeStr).get());
    
      card->GetDisplayName(getter_Copies(unicodeStr));
      AddDisplayName(cardRow, NS_ConvertUTF16toUTF8(unicodeStr).get());

      card->GetPrimaryEmail(getter_Copies(unicodeStr));
      if (unicodeStr)
        AddUnicodeToColumn(cardRow, m_PriEmailColumnToken, m_LowerPriEmailColumnToken, unicodeStr);

      PRUint32 nowInSeconds;
      PRTime now = PR_Now();
      PRTime2Seconds(now, &nowInSeconds);
      AddIntColumn(cardRow, m_LastModDateColumnToken, nowInSeconds);

      nsXPIDLString value;
      GetCardValue(card, CARD_ATTRIB_PALMID, getter_Copies(value));
      if (value)
      {
        nsCOMPtr<nsIAbCard> addedCard;
        rv = CreateCardFromDeletedCardsTable(cardRow, 0, getter_AddRefs(addedCard));
        if (NS_SUCCEEDED(rv))
          SetCardValue(addedCard, CARD_ATTRIB_PALMID, value, PR_FALSE);
      }
      NS_IF_ADDREF(*pCardRow = cardRow);
    }
    Commit(nsAddrDBCommitType::kLargeCommit);
  }
  return rv;
}

nsresult nsAddrDatabase::DeleteRowFromDeletedCardsTable(nsIMdbRow *pCardRow)
{
  if (!m_mdbEnv)
    return NS_ERROR_NULL_POINTER;

  mdb_err merror = NS_OK;
  if (m_mdbDeletedCardsTable) {
    pCardRow->CutAllColumns(m_mdbEnv);
    merror = m_mdbDeletedCardsTable->CutRow(m_mdbEnv, pCardRow);
  }
  return merror;
}


nsresult nsAddrDatabase::InitDeletedCardsTable(PRBool aCreate)
{
  nsresult mdberr = NS_OK;
  if (!m_mdbDeletedCardsTable)
  {
    struct mdbOid deletedCardsTableOID;
    deletedCardsTableOID.mOid_Scope = m_CardRowScopeToken;
    deletedCardsTableOID.mOid_Id = ID_DELETEDCARDS_TABLE;
    if (m_mdbStore && m_mdbEnv)
    {
      m_mdbStore->GetTable(m_mdbEnv, &deletedCardsTableOID, &m_mdbDeletedCardsTable);
      // if deletedCardsTable does not exist and bCreate is set, create a new one
      if (!m_mdbDeletedCardsTable && aCreate) 
      {
        mdberr = (nsresult) m_mdbStore->NewTableWithOid(m_mdbEnv, &deletedCardsTableOID, 
                                                        m_DeletedCardsTableKind, 
                                                        PR_TRUE, (const mdbOid*)nsnull, 
                                                        &m_mdbDeletedCardsTable);
      }
    }
  }
  return mdberr;
}

nsresult nsAddrDatabase::InitPabTable()
{
  return m_mdbStore && m_mdbEnv ? m_mdbStore->NewTableWithOid(m_mdbEnv,
                                                  &gAddressBookTableOID, 
                                                  m_PabTableKind,
                                                  PR_FALSE,
                                                  (const mdbOid*)nsnull,
                                                  &m_mdbPabTable)
      : NS_ERROR_NULL_POINTER;
}

//save the last record number, store in m_DataRowScopeToken, row 1
nsresult nsAddrDatabase::InitLastRecorKey()
{
  if (!m_mdbPabTable || !m_mdbStore || !m_mdbEnv)
    return NS_ERROR_NULL_POINTER;

  nsIMdbRow *pDataRow = nsnull;
  mdbOid dataRowOid;
  dataRowOid.mOid_Scope = m_DataRowScopeToken;
  dataRowOid.mOid_Id = DATAROW_ROWID;
  nsresult err = m_mdbStore->NewRowWithOid(m_mdbEnv, &dataRowOid, &pDataRow);

  if (NS_SUCCEEDED(err) && pDataRow)
  {
    m_LastRecordKey = 0;
    err = AddIntColumn(pDataRow, m_LastRecordKeyColumnToken, 0);
    err = m_mdbPabTable->AddRow(m_mdbEnv, pDataRow);
    NS_RELEASE(pDataRow);
  }
  return err;
}

nsresult nsAddrDatabase::GetDataRow(nsIMdbRow **pDataRow)
{
  if (!m_mdbStore || !m_mdbEnv)
    return NS_ERROR_NULL_POINTER;

  nsIMdbRow *pRow = nsnull;
  mdbOid dataRowOid;
  dataRowOid.mOid_Scope = m_DataRowScopeToken;
  dataRowOid.mOid_Id = DATAROW_ROWID;
  m_mdbStore->GetRow(m_mdbEnv, &dataRowOid, &pRow);
  *pDataRow = pRow;

  return pRow ? NS_OK : NS_ERROR_FAILURE;
}

nsresult nsAddrDatabase::GetLastRecordKey()
{
    if (!m_mdbPabTable)
        return NS_ERROR_NULL_POINTER;

    nsCOMPtr <nsIMdbRow> pDataRow;
    nsresult err = GetDataRow(getter_AddRefs(pDataRow));

    if (NS_SUCCEEDED(err) && pDataRow)
    {
        m_LastRecordKey = 0;
        err = GetIntColumn(pDataRow, m_LastRecordKeyColumnToken, &m_LastRecordKey, 0);
        if (NS_FAILED(err))
            err = NS_ERROR_NOT_AVAILABLE;
        return NS_OK;
    }

    return NS_ERROR_NOT_AVAILABLE;
}

nsresult nsAddrDatabase::UpdateLastRecordKey()
{
  if (!m_mdbPabTable || !m_mdbEnv)
    return NS_ERROR_NULL_POINTER;

  nsCOMPtr <nsIMdbRow> pDataRow;
  nsresult err = GetDataRow(getter_AddRefs(pDataRow));

  if (NS_SUCCEEDED(err) && pDataRow)
  {
    err = AddIntColumn(pDataRow, m_LastRecordKeyColumnToken, m_LastRecordKey);
    err = m_mdbPabTable->AddRow(m_mdbEnv, pDataRow);
    return NS_OK;
  }
  else if (!pDataRow)
    err = InitLastRecorKey();
  else
    return NS_ERROR_NOT_AVAILABLE;
  return err;
}

nsresult nsAddrDatabase::InitExistingDB()
{
  nsresult err = InitMDBInfo();
  if (err == NS_OK)
  {
    if (!m_mdbStore || !m_mdbEnv)
      return NS_ERROR_NULL_POINTER;

    err = m_mdbStore->GetTable(m_mdbEnv, &gAddressBookTableOID, &m_mdbPabTable);
    if (NS_SUCCEEDED(err) && m_mdbPabTable)
    {
      err = GetLastRecordKey();
      if (err == NS_ERROR_NOT_AVAILABLE)
        CheckAndUpdateRecordKey();
      UpdateLowercaseEmailListName();
    }
  }
  return err;
}

nsresult nsAddrDatabase::CheckAndUpdateRecordKey()
{
  if (!m_mdbEnv)
    return NS_ERROR_NULL_POINTER;

  nsresult err = NS_OK;
  nsIMdbTableRowCursor* rowCursor = nsnull;
  nsIMdbRow* findRow = nsnull;
  mdb_pos    rowPos = 0;

  mdb_err merror = m_mdbPabTable->GetTableRowCursor(m_mdbEnv, -1, &rowCursor);

  if (!(merror == NS_OK && rowCursor))
    return NS_ERROR_FAILURE;

  nsCOMPtr <nsIMdbRow> pDataRow;
  err = GetDataRow(getter_AddRefs(pDataRow));
  if (NS_FAILED(err))
    InitLastRecorKey();

  do
  {  //add key to each card and mailing list row
    merror = rowCursor->NextRow(m_mdbEnv, &findRow, &rowPos);
    if (merror == NS_OK && findRow)
    {
      mdbOid rowOid;

      if (findRow->GetOid(GetEnv(), &rowOid) == NS_OK)
      {
        if (!IsDataRowScopeToken(rowOid.mOid_Scope))
        {
          m_LastRecordKey++;
          err = AddIntColumn(findRow, m_RecordKeyColumnToken, m_LastRecordKey);
        }
      }
    }
  } while (findRow);

  UpdateLastRecordKey();
  Commit(nsAddrDBCommitType::kLargeCommit);
  return NS_OK;
}

nsresult nsAddrDatabase::UpdateLowercaseEmailListName()
{
  if (!m_mdbEnv)
    return NS_ERROR_NULL_POINTER;

  nsresult err = NS_OK;
  nsIMdbTableRowCursor* rowCursor = nsnull;
  nsIMdbRow* findRow = nsnull;
  mdb_pos    rowPos = 0;
  PRBool commitRequired = PR_FALSE;
  
  mdb_err merror = m_mdbPabTable->GetTableRowCursor(m_mdbEnv, -1, &rowCursor);
  
  if (!(merror == NS_OK && rowCursor))
    return NS_ERROR_FAILURE;
  
  do
  {   //add lowercase primary email to each card and mailing list row
    merror = rowCursor->NextRow(m_mdbEnv, &findRow, &rowPos);
    if (merror == NS_OK && findRow)
    {
      mdbOid rowOid;
      
      if (findRow->GetOid(GetEnv(), &rowOid) == NS_OK)
      {
        nsAutoString tempString;
        if (IsCardRowScopeToken(rowOid.mOid_Scope))
        {
          err = GetStringColumn(findRow, m_LowerPriEmailColumnToken, tempString);
          if (NS_SUCCEEDED(err))
            break;
          
          err = ConvertAndAddLowercaseColumn(findRow, m_PriEmailColumnToken, 
            m_LowerPriEmailColumnToken);
          commitRequired = PR_TRUE;
        }
        else if (IsListRowScopeToken(rowOid.mOid_Scope))
        {
          err = GetStringColumn(findRow, m_LowerListNameColumnToken, tempString);
          if (NS_SUCCEEDED(err))
            break;
          
          err = ConvertAndAddLowercaseColumn(findRow, m_ListNameColumnToken, 
            m_LowerListNameColumnToken);
          commitRequired = PR_TRUE;
        }
      }
      findRow->Release();
    }
  } while (findRow);

  if (findRow)
    findRow->Release();
  rowCursor->Release();
  if (commitRequired)
    Commit(nsAddrDBCommitType::kLargeCommit);
  return NS_OK;
}

/*  
We store UTF8 strings in the database.  We need to convert the UTF8 
string into unicode string, then convert to lower case.  Before storing 
back into the database,  we need to convert the lowercase unicode string 
into UTF8 string.
*/
nsresult nsAddrDatabase::ConvertAndAddLowercaseColumn
(nsIMdbRow * row, mdb_token fromCol, mdb_token toCol)
{
    nsAutoString colString;

    nsresult rv = GetStringColumn(row, fromCol, colString);
    if (!colString.IsEmpty())
    {
        rv = AddLowercaseColumn(row, toCol, NS_ConvertUTF16toUTF8(colString).get());
    }
    return rv;
}
 
// Change the unicode string to lowercase, then convert to UTF8 string to store in db
nsresult nsAddrDatabase::AddUnicodeToColumn(nsIMdbRow * row, mdb_token aColToken, mdb_token aLowerCaseColToken, const PRUnichar* aUnicodeStr)
{
  nsresult rv = AddCharStringColumn(row, aColToken, NS_ConvertUTF16toUTF8(aUnicodeStr).get());
  NS_ENSURE_SUCCESS(rv,rv);

  rv = AddLowercaseColumn(row, aLowerCaseColToken, NS_ConvertUTF16toUTF8(aUnicodeStr).get());
  NS_ENSURE_SUCCESS(rv,rv);
  return rv;
}

// initialize the various tokens and tables in our db's env
nsresult nsAddrDatabase::InitMDBInfo()
{
  nsresult err = NS_OK;

  if (!m_mdbTokensInitialized && m_mdbStore && m_mdbEnv)
  {
    m_mdbTokensInitialized = PR_TRUE;
    err = m_mdbStore->StringToToken(m_mdbEnv, kCardRowScope, &m_CardRowScopeToken); 
    err = m_mdbStore->StringToToken(m_mdbEnv, kListRowScope, &m_ListRowScopeToken); 
    err = m_mdbStore->StringToToken(m_mdbEnv, kDataRowScope, &m_DataRowScopeToken); 
    gAddressBookTableOID.mOid_Scope = m_CardRowScopeToken;
    gAddressBookTableOID.mOid_Id = ID_PAB_TABLE;
    if (NS_SUCCEEDED(err))
    { 
      m_mdbStore->StringToToken(m_mdbEnv,  kFirstNameColumn, &m_FirstNameColumnToken);
      m_mdbStore->StringToToken(m_mdbEnv,  kLastNameColumn, &m_LastNameColumnToken);
      m_mdbStore->StringToToken(m_mdbEnv,  kPhoneticFirstNameColumn, &m_PhoneticFirstNameColumnToken);
      m_mdbStore->StringToToken(m_mdbEnv,  kPhoneticLastNameColumn, &m_PhoneticLastNameColumnToken);
      m_mdbStore->StringToToken(m_mdbEnv,  kDisplayNameColumn, &m_DisplayNameColumnToken);
      m_mdbStore->StringToToken(m_mdbEnv,  kNicknameColumn, &m_NickNameColumnToken);
      m_mdbStore->StringToToken(m_mdbEnv,  kPriEmailColumn, &m_PriEmailColumnToken);
      m_mdbStore->StringToToken(m_mdbEnv,  kLowerPriEmailColumn, &m_LowerPriEmailColumnToken);
      m_mdbStore->StringToToken(m_mdbEnv,  k2ndEmailColumn, &m_2ndEmailColumnToken);
      m_mdbStore->StringToToken(m_mdbEnv,  kPreferMailFormatColumn, &m_MailFormatColumnToken);
      m_mdbStore->StringToToken(m_mdbEnv,  kPopularityIndexColumn, &m_PopularityIndexColumnToken);
      m_mdbStore->StringToToken(m_mdbEnv,  kWorkPhoneColumn, &m_WorkPhoneColumnToken);
      m_mdbStore->StringToToken(m_mdbEnv,  kHomePhoneColumn, &m_HomePhoneColumnToken);
      m_mdbStore->StringToToken(m_mdbEnv,  kFaxColumn, &m_FaxColumnToken);
      m_mdbStore->StringToToken(m_mdbEnv,  kPagerColumn, &m_PagerColumnToken);
      m_mdbStore->StringToToken(m_mdbEnv,  kCellularColumn, &m_CellularColumnToken);
      m_mdbStore->StringToToken(m_mdbEnv,  kWorkPhoneTypeColumn, &m_WorkPhoneTypeColumnToken);
      m_mdbStore->StringToToken(m_mdbEnv,  kHomePhoneTypeColumn, &m_HomePhoneTypeColumnToken);
      m_mdbStore->StringToToken(m_mdbEnv,  kFaxTypeColumn, &m_FaxTypeColumnToken);
      m_mdbStore->StringToToken(m_mdbEnv,  kPagerTypeColumn, &m_PagerTypeColumnToken);
      m_mdbStore->StringToToken(m_mdbEnv,  kCellularTypeColumn, &m_CellularTypeColumnToken);
      m_mdbStore->StringToToken(m_mdbEnv,  kHomeAddressColumn, &m_HomeAddressColumnToken);
      m_mdbStore->StringToToken(m_mdbEnv,  kHomeAddress2Column, &m_HomeAddress2ColumnToken);
      m_mdbStore->StringToToken(m_mdbEnv,  kHomeCityColumn, &m_HomeCityColumnToken);
      m_mdbStore->StringToToken(m_mdbEnv,  kHomeStateColumn, &m_HomeStateColumnToken);
      m_mdbStore->StringToToken(m_mdbEnv,  kHomeZipCodeColumn, &m_HomeZipCodeColumnToken);
      m_mdbStore->StringToToken(m_mdbEnv,  kHomeCountryColumn, &m_HomeCountryColumnToken);
      m_mdbStore->StringToToken(m_mdbEnv,  kWorkAddressColumn, &m_WorkAddressColumnToken);
      m_mdbStore->StringToToken(m_mdbEnv,  kWorkAddress2Column, &m_WorkAddress2ColumnToken);
      m_mdbStore->StringToToken(m_mdbEnv,  kWorkCityColumn, &m_WorkCityColumnToken);
      m_mdbStore->StringToToken(m_mdbEnv,  kWorkStateColumn, &m_WorkStateColumnToken);
      m_mdbStore->StringToToken(m_mdbEnv,  kWorkZipCodeColumn, &m_WorkZipCodeColumnToken);
      m_mdbStore->StringToToken(m_mdbEnv,  kWorkCountryColumn, &m_WorkCountryColumnToken);
      m_mdbStore->StringToToken(m_mdbEnv,  kJobTitleColumn, &m_JobTitleColumnToken);
      m_mdbStore->StringToToken(m_mdbEnv,  kDepartmentColumn, &m_DepartmentColumnToken);
      m_mdbStore->StringToToken(m_mdbEnv,  kCompanyColumn, &m_CompanyColumnToken);
      m_mdbStore->StringToToken(m_mdbEnv,  kAimScreenNameColumn, &m_AimScreenNameColumnToken);
      m_mdbStore->StringToToken(m_mdbEnv,  kAnniversaryYearColumn, &m_AnniversaryYearColumnToken);
      m_mdbStore->StringToToken(m_mdbEnv,  kAnniversaryMonthColumn, &m_AnniversaryMonthColumnToken);
      m_mdbStore->StringToToken(m_mdbEnv,  kAnniversaryDayColumn, &m_AnniversaryDayColumnToken);
      m_mdbStore->StringToToken(m_mdbEnv,  kSpouseNameColumn, &m_SpouseNameColumnToken);
      m_mdbStore->StringToToken(m_mdbEnv,  kFamilyNameColumn, &m_FamilyNameColumnToken);
      m_mdbStore->StringToToken(m_mdbEnv,  kDefaultAddressColumn, &m_DefaultAddressColumnToken);
      m_mdbStore->StringToToken(m_mdbEnv,  kCategoryColumn, &m_CategoryColumnToken);
      m_mdbStore->StringToToken(m_mdbEnv,  kWebPage1Column, &m_WebPage1ColumnToken);
      m_mdbStore->StringToToken(m_mdbEnv,  kWebPage2Column, &m_WebPage2ColumnToken);
      m_mdbStore->StringToToken(m_mdbEnv,  kBirthYearColumn, &m_BirthYearColumnToken);
      m_mdbStore->StringToToken(m_mdbEnv,  kBirthMonthColumn, &m_BirthMonthColumnToken);
      m_mdbStore->StringToToken(m_mdbEnv,  kBirthDayColumn, &m_BirthDayColumnToken);
      m_mdbStore->StringToToken(m_mdbEnv,  kCustom1Column, &m_Custom1ColumnToken);
      m_mdbStore->StringToToken(m_mdbEnv,  kCustom2Column, &m_Custom2ColumnToken);
      m_mdbStore->StringToToken(m_mdbEnv,  kCustom3Column, &m_Custom3ColumnToken);
      m_mdbStore->StringToToken(m_mdbEnv,  kCustom4Column, &m_Custom4ColumnToken);
      m_mdbStore->StringToToken(m_mdbEnv,  kNotesColumn, &m_NotesColumnToken);
      m_mdbStore->StringToToken(m_mdbEnv,  kLastModifiedDateColumn, &m_LastModDateColumnToken);
      m_mdbStore->StringToToken(m_mdbEnv,  kRecordKeyColumn, &m_RecordKeyColumnToken);
      m_mdbStore->StringToToken(m_mdbEnv,  kAddressCharSetColumn, &m_AddressCharSetColumnToken);
      m_mdbStore->StringToToken(m_mdbEnv,  kLastRecordKeyColumn, &m_LastRecordKeyColumnToken);

      err = m_mdbStore->StringToToken(m_mdbEnv, kPabTableKind, &m_PabTableKind); 

      m_mdbStore->StringToToken(m_mdbEnv,  kMailListName, &m_ListNameColumnToken);
      m_mdbStore->StringToToken(m_mdbEnv,  kMailListNickName, &m_ListNickNameColumnToken);
      m_mdbStore->StringToToken(m_mdbEnv,  kMailListDescription, &m_ListDescriptionColumnToken);
      m_mdbStore->StringToToken(m_mdbEnv,  kMailListTotalAddresses, &m_ListTotalColumnToken);
      m_mdbStore->StringToToken(m_mdbEnv,  kLowerListNameColumn, &m_LowerListNameColumnToken);
      m_mdbStore->StringToToken(m_mdbEnv,  kDeletedCardsTableKind, &m_DeletedCardsTableKind);
    }
  }
  return err;
}

////////////////////////////////////////////////////////////////////////////////

nsresult nsAddrDatabase::AddRecordKeyColumnToRow(nsIMdbRow *pRow)
{
  if (pRow && m_mdbEnv)
  {
    m_LastRecordKey++;
    nsresult err = AddIntColumn(pRow, m_RecordKeyColumnToken, m_LastRecordKey);
    NS_ENSURE_SUCCESS(err, err);

    err = m_mdbPabTable->AddRow(m_mdbEnv, pRow);
    UpdateLastRecordKey();
    return err;
  }
  return NS_ERROR_NULL_POINTER;
}

nsresult nsAddrDatabase::AddAttributeColumnsToRow(nsIAbCard *card, nsIMdbRow *cardRow)
{
  nsresult err = NS_OK;
  
  if ((!card && !cardRow) || !m_mdbEnv)
    return NS_ERROR_NULL_POINTER;
  
  mdbOid rowOid, tableOid;
  m_mdbPabTable->GetOid(m_mdbEnv, &tableOid);
  cardRow->GetOid(m_mdbEnv, &rowOid);
  
  nsCOMPtr<nsIAbMDBCard> dbcard(do_QueryInterface(card, &err));
  if(NS_SUCCEEDED(err) && dbcard)
  {
    dbcard->SetDbTableID(tableOid.mOid_Id);
    dbcard->SetDbRowID(rowOid.mOid_Id);
  }
  // add the row to the singleton table.
  if (card && cardRow)
  {
    nsXPIDLString unicodeStr;
    card->GetFirstName(getter_Copies(unicodeStr));
    AddFirstName(cardRow, NS_ConvertUTF16toUTF8(unicodeStr).get());
    
    card->GetLastName(getter_Copies(unicodeStr));
    AddLastName(cardRow, NS_ConvertUTF16toUTF8(unicodeStr).get());
    
    card->GetPhoneticFirstName(getter_Copies(unicodeStr));
    AddPhoneticFirstName(cardRow, NS_ConvertUTF16toUTF8(unicodeStr).get());
    
    card->GetPhoneticLastName(getter_Copies(unicodeStr));
    AddPhoneticLastName(cardRow, NS_ConvertUTF16toUTF8(unicodeStr).get());
    
    card->GetDisplayName(getter_Copies(unicodeStr));
    AddDisplayName(cardRow, NS_ConvertUTF16toUTF8(unicodeStr).get());
    
    card->GetNickName(getter_Copies(unicodeStr));
    AddNickName(cardRow, NS_ConvertUTF16toUTF8(unicodeStr).get());
    
    card->GetPrimaryEmail(getter_Copies(unicodeStr));
    if (unicodeStr)
      AddUnicodeToColumn(cardRow, m_PriEmailColumnToken, m_LowerPriEmailColumnToken, unicodeStr);

    card->GetSecondEmail(getter_Copies(unicodeStr));
    Add2ndEmail(cardRow, NS_ConvertUTF16toUTF8(unicodeStr).get());
    
    PRUint32 format = nsIAbPreferMailFormat::unknown;
    card->GetPreferMailFormat(&format);
    AddPreferMailFormat(cardRow, format);

    PRUint32 popularityIndex = 0;
    card->GetPopularityIndex(&popularityIndex);
    AddPopularityIndex(cardRow, popularityIndex);
    
    card->GetWorkPhone(getter_Copies(unicodeStr));
    AddWorkPhone(cardRow, NS_ConvertUTF16toUTF8(unicodeStr).get());
    
    card->GetHomePhone(getter_Copies(unicodeStr));
    AddHomePhone(cardRow, NS_ConvertUTF16toUTF8(unicodeStr).get());
    
    card->GetFaxNumber(getter_Copies(unicodeStr));
    AddFaxNumber(cardRow, NS_ConvertUTF16toUTF8(unicodeStr).get());
    
    card->GetPagerNumber(getter_Copies(unicodeStr));
    AddPagerNumber(cardRow,NS_ConvertUTF16toUTF8(unicodeStr).get());
    
    card->GetCellularNumber(getter_Copies(unicodeStr));
    AddCellularNumber(cardRow,NS_ConvertUTF16toUTF8(unicodeStr).get());
    
    card->GetWorkPhoneType(getter_Copies(unicodeStr));
    AddWorkPhoneType(cardRow, NS_ConvertUTF16toUTF8(unicodeStr).get());
    
    card->GetHomePhoneType(getter_Copies(unicodeStr));
    AddHomePhoneType(cardRow, NS_ConvertUTF16toUTF8(unicodeStr).get());
    
    card->GetFaxNumberType(getter_Copies(unicodeStr));
    AddFaxNumberType(cardRow, NS_ConvertUTF16toUTF8(unicodeStr).get());
    
    card->GetPagerNumberType(getter_Copies(unicodeStr));
    AddPagerNumberType(cardRow,NS_ConvertUTF16toUTF8(unicodeStr).get());
    
    card->GetCellularNumberType(getter_Copies(unicodeStr));
    AddCellularNumberType(cardRow,NS_ConvertUTF16toUTF8(unicodeStr).get());
    
    card->GetHomeAddress(getter_Copies(unicodeStr));
    AddHomeAddress(cardRow, NS_ConvertUTF16toUTF8(unicodeStr).get());
    
    card->GetHomeAddress2(getter_Copies(unicodeStr)); 
    AddHomeAddress2(cardRow, NS_ConvertUTF16toUTF8(unicodeStr).get());
    
    card->GetHomeCity(getter_Copies(unicodeStr)); 
    AddHomeCity(cardRow, NS_ConvertUTF16toUTF8(unicodeStr).get());
    
    card->GetHomeState(getter_Copies(unicodeStr)); 
    AddHomeState(cardRow, NS_ConvertUTF16toUTF8(unicodeStr).get());
    
    card->GetHomeZipCode(getter_Copies(unicodeStr)); 
    AddHomeZipCode(cardRow, NS_ConvertUTF16toUTF8(unicodeStr).get());
    
    card->GetHomeCountry(getter_Copies(unicodeStr)); 
    AddHomeCountry(cardRow, NS_ConvertUTF16toUTF8(unicodeStr).get());
    
    card->GetWorkAddress(getter_Copies(unicodeStr));  
    AddWorkAddress(cardRow, NS_ConvertUTF16toUTF8(unicodeStr).get());
      
    card->GetWorkAddress2(getter_Copies(unicodeStr)); 
    AddWorkAddress2(cardRow, NS_ConvertUTF16toUTF8(unicodeStr).get());
      
    card->GetWorkCity(getter_Copies(unicodeStr)); 
    AddWorkCity(cardRow, NS_ConvertUTF16toUTF8(unicodeStr).get());
      
    card->GetWorkState(getter_Copies(unicodeStr)); 
    AddWorkState(cardRow, NS_ConvertUTF16toUTF8(unicodeStr).get());
     
    card->GetWorkZipCode(getter_Copies(unicodeStr)); 
    AddWorkZipCode(cardRow, NS_ConvertUTF16toUTF8(unicodeStr).get());
     
    card->GetWorkCountry(getter_Copies(unicodeStr)); 
    AddWorkCountry(cardRow, NS_ConvertUTF16toUTF8(unicodeStr).get());
      
    card->GetJobTitle(getter_Copies(unicodeStr)); 
    AddJobTitle(cardRow, NS_ConvertUTF16toUTF8(unicodeStr).get());
      
    card->GetDepartment(getter_Copies(unicodeStr)); 
    AddDepartment(cardRow, NS_ConvertUTF16toUTF8(unicodeStr).get());
      
    card->GetCompany(getter_Copies(unicodeStr)); 
    AddCompany(cardRow, NS_ConvertUTF16toUTF8(unicodeStr).get());
     
    // AimScreenName
    card->GetAimScreenName(getter_Copies(unicodeStr)); 
    AddAimScreenName(cardRow, NS_ConvertUTF16toUTF8(unicodeStr).get());
     
    card->GetAnniversaryYear(getter_Copies(unicodeStr)); 
    AddAnniversaryYear(cardRow, NS_ConvertUTF16toUTF8(unicodeStr).get());
  
    card->GetAnniversaryMonth(getter_Copies(unicodeStr)); 
    AddAnniversaryMonth(cardRow, NS_ConvertUTF16toUTF8(unicodeStr).get());
  
    card->GetAnniversaryDay(getter_Copies(unicodeStr)); 
    AddAnniversaryDay(cardRow, NS_ConvertUTF16toUTF8(unicodeStr).get());

    card->GetSpouseName(getter_Copies(unicodeStr)); 
    AddSpouseName(cardRow, NS_ConvertUTF16toUTF8(unicodeStr).get());

    card->GetFamilyName(getter_Copies(unicodeStr)); 
    AddFamilyName(cardRow, NS_ConvertUTF16toUTF8(unicodeStr).get());

    card->GetDefaultAddress(getter_Copies(unicodeStr)); 
    AddDefaultAddress(cardRow, NS_ConvertUTF16toUTF8(unicodeStr).get());

    card->GetCategory(getter_Copies(unicodeStr)); 
    AddCategory(cardRow, NS_ConvertUTF16toUTF8(unicodeStr).get());

    card->GetWebPage1(getter_Copies(unicodeStr)); 
    AddWebPage1(cardRow,NS_ConvertUTF16toUTF8(unicodeStr).get());
     
    card->GetWebPage2(getter_Copies(unicodeStr)); 
    AddWebPage2(cardRow, NS_ConvertUTF16toUTF8(unicodeStr).get());
     
    card->GetBirthYear(getter_Copies(unicodeStr)); 
    AddBirthYear(cardRow, NS_ConvertUTF16toUTF8(unicodeStr).get());
      
    card->GetBirthMonth(getter_Copies(unicodeStr)); 
    AddBirthMonth(cardRow, NS_ConvertUTF16toUTF8(unicodeStr).get());
      
    card->GetBirthDay(getter_Copies(unicodeStr)); 
    AddBirthDay(cardRow, NS_ConvertUTF16toUTF8(unicodeStr).get());
      
    card->GetCustom1(getter_Copies(unicodeStr)); 
    AddCustom1(cardRow,NS_ConvertUTF16toUTF8(unicodeStr).get());
     
    card->GetCustom2(getter_Copies(unicodeStr)); 
    AddCustom2(cardRow, NS_ConvertUTF16toUTF8(unicodeStr).get());
      
    card->GetCustom3(getter_Copies(unicodeStr)); 
    AddCustom3(cardRow, NS_ConvertUTF16toUTF8(unicodeStr).get());
      
    card->GetCustom4(getter_Copies(unicodeStr)); 
    AddCustom4(cardRow, NS_ConvertUTF16toUTF8(unicodeStr).get());
    
    card->GetNotes(getter_Copies(unicodeStr)); 
    AddNotes(cardRow, NS_ConvertUTF16toUTF8(unicodeStr).get());
      
    PRUint32 lastModDate = 0;
    card->GetLastModifiedDate(&lastModDate);
    AddIntColumn(cardRow, m_LastModDateColumnToken, lastModDate);
      
  }
  return NS_OK;
}

NS_IMETHODIMP nsAddrDatabase::CreateNewCardAndAddToDB(nsIAbCard *newCard, PRBool notify /* = FALSE */)
{
  nsCOMPtr <nsIMdbRow> cardRow;
  
  if (!newCard || !m_mdbPabTable || !m_mdbEnv)
    return NS_ERROR_NULL_POINTER;
  
  nsresult rv = GetNewRow(getter_AddRefs(cardRow));
  if (NS_SUCCEEDED(rv) && cardRow)
  {
    AddAttributeColumnsToRow(newCard, cardRow);
    AddRecordKeyColumnToRow(cardRow);
    
    // we need to do this for dnd
    PRUint32 key = 0;
    rv = GetIntColumn(cardRow, m_RecordKeyColumnToken, &key, 0);
    if (NS_SUCCEEDED(rv)) {
      nsCOMPtr<nsIAbMDBCard> dbnewCard = do_QueryInterface(newCard);
      if (dbnewCard)
        dbnewCard->SetKey(key);
    }
    
    mdb_err merror = m_mdbPabTable->AddRow(m_mdbEnv, cardRow);
    if (merror != NS_OK) return NS_ERROR_FAILURE;
    
  }
  else
    return rv;
  
  //  do notification
  if (notify)
  {
    NotifyCardEntryChange(AB_NotifyInserted, newCard);
  }
  return rv;
}

NS_IMETHODIMP nsAddrDatabase::CreateNewListCardAndAddToDB(nsIAbDirectory *aList, PRUint32 listRowID, nsIAbCard *newCard, PRBool notify /* = FALSE */)
{
  if (!newCard || !m_mdbPabTable || !m_mdbStore || !m_mdbEnv)
        return NS_ERROR_NULL_POINTER;

  nsIMdbRow* pListRow = nsnull;
  mdbOid listRowOid;
  listRowOid.mOid_Scope = m_ListRowScopeToken;
  listRowOid.mOid_Id = listRowID;
  nsresult rv = m_mdbStore->GetRow(m_mdbEnv, &listRowOid, &pListRow);
  NS_ENSURE_SUCCESS(rv,rv);

  if (!pListRow)
    return NS_OK;
  
  nsCOMPtr <nsISupportsArray> addressList;
  rv = aList->GetAddressLists(getter_AddRefs(addressList));
  NS_ENSURE_SUCCESS(rv,rv);

  PRUint32 count;
    addressList->Count(&count);

  nsXPIDLString newEmail;
  rv = newCard->GetPrimaryEmail(getter_Copies(newEmail));
  NS_ENSURE_SUCCESS(rv,rv);

    PRUint32 i;
  for (i = 0; i < count; i++) {
    nsCOMPtr<nsIAbCard> currentCard = do_QueryElementAt(addressList, i, &rv);
    NS_ENSURE_SUCCESS(rv,rv);

    PRBool equals;
    rv = newCard->Equals(currentCard, &equals);
    NS_ENSURE_SUCCESS(rv,rv);

    if (equals) {
      // card is already in list, bail out.
      // this can happen when dropping a card on a mailing list from the directory that contains the mailing list
      return NS_OK;
    }

    nsXPIDLString currentEmail;
    rv = currentCard->GetPrimaryEmail(getter_Copies(currentEmail));
    NS_ENSURE_SUCCESS(rv,rv);

    if (!nsCRT::strcmp(newEmail.get(), currentEmail.get())) {
      // card is already in list, bail out
      // this can happen when dropping a card on a mailing list from another directory (not the one that contains the mailing list
      // or if you have multiple cards on a directory, with the same primary email address.
      return NS_OK;
    }
  }  
    
  // start from 1
  PRUint32 totalAddress = GetListAddressTotal(pListRow) + 1;
  SetListAddressTotal(pListRow, totalAddress);
  nsCOMPtr<nsIAbCard> pNewCard;
  rv = AddListCardColumnsToRow(newCard, pListRow, totalAddress, getter_AddRefs(pNewCard), PR_TRUE /* aInMailingList */);
  NS_ENSURE_SUCCESS(rv,rv);

  addressList->AppendElement(newCard);

  if (notify)
    NotifyCardEntryChange(AB_NotifyInserted, newCard); 

    return rv;
}

NS_IMETHODIMP nsAddrDatabase::AddListCardColumnsToRow
(nsIAbCard *pCard, nsIMdbRow *pListRow, PRUint32 pos, nsIAbCard** pNewCard, PRBool aInMailingList)
{
  if (!pCard || !pListRow || !m_mdbStore || !m_mdbEnv)
    return NS_ERROR_NULL_POINTER;
  
  nsresult    err = NS_OK;
  nsXPIDLString email;
  pCard->GetPrimaryEmail(getter_Copies(email));
  if (email)
  {
    nsIMdbRow    *pCardRow = nsnull;
    // Please DO NOT change the 3rd param of GetRowFromAttribute() call to 
    // PR_TRUE (ie, case insensitive) without reading bugs #128535 and #121478.
    err = GetRowFromAttribute(kPriEmailColumn, NS_ConvertUTF16toUTF8(email).get(), PR_FALSE /* retain case */, &pCardRow);
    PRBool cardWasAdded = PR_FALSE;
    if (NS_FAILED(err) || !pCardRow)
    {
      //New Email, then add a new row with this email
      err  = GetNewRow(&pCardRow);
      
      if (NS_SUCCEEDED(err) && pCardRow)
      {
        AddPrimaryEmail(pCardRow, NS_ConvertUTF16toUTF8(email).get());
        err = m_mdbPabTable->AddRow(m_mdbEnv, pCardRow);
        // Create a key for this row as well.
        if (NS_SUCCEEDED(err))
          AddRecordKeyColumnToRow(pCardRow);
      }
      
      cardWasAdded = PR_TRUE;
    }
    
    NS_ENSURE_TRUE(pCardRow, NS_ERROR_NULL_POINTER);
    
    nsXPIDLString name;
    pCard->GetDisplayName(getter_Copies(name));
    if (!name.IsEmpty()) {
      AddDisplayName(pCardRow, NS_ConvertUTF16toUTF8(name).get());
      err = m_mdbPabTable->AddRow(m_mdbEnv, pCardRow);
    }

    nsCOMPtr<nsIAbCard> newCard;
    CreateABCard(pCardRow, 0, getter_AddRefs(newCard));
    NS_IF_ADDREF(*pNewCard = newCard);
    
    if (cardWasAdded) {
      NotifyCardEntryChange(AB_NotifyInserted, newCard);
    }
    else if (!aInMailingList) {
      NotifyCardEntryChange(AB_NotifyInserted, pCard);
    }
    else {
      NotifyCardEntryChange(AB_NotifyPropertyChanged, pCard);
    }
    
    //add a column with address row id to the list row
    mdb_token listAddressColumnToken;
    
    char columnStr[COLUMN_STR_MAX];
    PR_snprintf(columnStr, COLUMN_STR_MAX, kMailListAddressFormat, pos);
    m_mdbStore->StringToToken(m_mdbEnv,  columnStr, &listAddressColumnToken);
    
    mdbOid outOid;
    
    if (pCardRow->GetOid(m_mdbEnv, &outOid) == NS_OK)
    {
      //save address row ID to the list row
      err = AddIntColumn(pListRow, listAddressColumnToken, outOid.mOid_Id);
    }
    NS_RELEASE(pCardRow);

  }
  
  return NS_OK;
}

nsresult nsAddrDatabase::AddListAttributeColumnsToRow(nsIAbDirectory *list, nsIMdbRow *listRow)
{
    nsresult    err = NS_OK;

    if ((!list && !listRow) || !m_mdbEnv)
        return NS_ERROR_NULL_POINTER;

    mdbOid rowOid, tableOid;
    m_mdbPabTable->GetOid(m_mdbEnv, &tableOid);
    listRow->GetOid(m_mdbEnv, &rowOid);

    nsCOMPtr<nsIAbMDBDirectory> dblist(do_QueryInterface(list,&err));
    if (NS_SUCCEEDED(err))
        dblist->SetDbRowID(rowOid.mOid_Id);

    // add the row to the singleton table.
    if (NS_SUCCEEDED(err) && listRow)
    {
        nsXPIDLString unicodeStr;

        list->GetDirName(getter_Copies(unicodeStr));
        if (unicodeStr)
            AddUnicodeToColumn(listRow, m_ListNameColumnToken, m_LowerListNameColumnToken, unicodeStr);

        list->GetListNickName(getter_Copies(unicodeStr));
        AddListNickName(listRow, NS_ConvertUTF16toUTF8(unicodeStr).get());
        
        list->GetDescription(getter_Copies(unicodeStr));
        AddListDescription(listRow, NS_ConvertUTF16toUTF8(unicodeStr).get());
            
    // XXX todo, this code has problems if you manually enter duplicate emails.
        nsCOMPtr <nsISupportsArray> pAddressLists;
        list->GetAddressLists(getter_AddRefs(pAddressLists));
        PRUint32 count;
        pAddressLists->Count(&count);

      nsXPIDLString email;
        PRUint32 i, total;
        total = 0;
        for (i = 0; i < count; i++)
        {
            nsCOMPtr<nsIAbCard> pCard(do_QueryElementAt(pAddressLists, i, &err));
            
            if (NS_FAILED(err))
                continue;

            pCard->GetPrimaryEmail(getter_Copies(email));
            PRInt32 emailLength = nsCRT::strlen(email);
            if (email && emailLength)
                total++;
        }
        SetListAddressTotal(listRow, total);

        PRUint32 pos;
        for (i = 0; i < count; i++)
        {
            nsCOMPtr<nsIAbCard> pCard(do_QueryElementAt(pAddressLists, i, &err));
            
            if (NS_FAILED(err))
                continue;

            PRBool listHasCard = PR_FALSE;
            err = list->HasCard(pCard, &listHasCard);

            // start from 1
            pos = i + 1;
            pCard->GetPrimaryEmail(getter_Copies(email));
            PRInt32 emailLength = nsCRT::strlen(email);
            if (email && emailLength)
            {
                nsCOMPtr<nsIAbCard> pNewCard;
                err = AddListCardColumnsToRow(pCard, listRow, pos, getter_AddRefs(pNewCard), listHasCard);
                if (pNewCard)
                    pAddressLists->ReplaceElementAt(pNewCard, i);
            }
        }
    }
    return NS_OK;
}

PRUint32 nsAddrDatabase::GetListAddressTotal(nsIMdbRow* listRow)
{
    PRUint32 count = 0;
    GetIntColumn(listRow, m_ListTotalColumnToken, &count, 0);
    return count;
}

NS_IMETHODIMP nsAddrDatabase::SetListAddressTotal(nsIMdbRow* aListRow, PRUint32 aTotal)
{
    return AddIntColumn(aListRow, m_ListTotalColumnToken, aTotal);
}

NS_IMETHODIMP nsAddrDatabase::FindRowByCard(nsIAbCard * aCard,nsIMdbRow **aRow)
{
    nsXPIDLString primaryEmail;
    aCard->GetPrimaryEmail(getter_Copies(primaryEmail));
    return GetRowForCharColumn(primaryEmail, m_PriEmailColumnToken, PR_TRUE, aRow);
}

nsresult nsAddrDatabase::GetAddressRowByPos(nsIMdbRow* listRow, PRUint16 pos, nsIMdbRow** cardRow)
{
  if (!m_mdbStore || !listRow || !cardRow || !m_mdbEnv)
    return NS_ERROR_NULL_POINTER;

  mdb_token listAddressColumnToken;

  char columnStr[COLUMN_STR_MAX];
  PR_snprintf(columnStr, COLUMN_STR_MAX, kMailListAddressFormat, pos);
  m_mdbStore->StringToToken(m_mdbEnv, columnStr, &listAddressColumnToken);

  nsAutoString tempString;
  mdb_id rowID;
  nsresult err = GetIntColumn(listRow, listAddressColumnToken, (PRUint32*)&rowID, 0);
  NS_ENSURE_SUCCESS(err, err);

  return GetCardRowByRowID(rowID, cardRow);
}

NS_IMETHODIMP nsAddrDatabase::CreateMailListAndAddToDB(nsIAbDirectory *newList, PRBool notify /* = FALSE */)
{
    nsresult    err = NS_OK;
    nsIMdbRow    *listRow;

    if (!newList || !m_mdbPabTable || !m_mdbEnv)
        return NS_ERROR_NULL_POINTER;
    
    err  = GetNewListRow(&listRow);

    if (NS_SUCCEEDED(err) && listRow)
    {
        AddListAttributeColumnsToRow(newList, listRow);
        AddRecordKeyColumnToRow(listRow);
        mdb_err merror = m_mdbPabTable->AddRow(m_mdbEnv, listRow);
        if (merror != NS_OK) return NS_ERROR_FAILURE;

        nsCOMPtr<nsIAbCard> listCard;
        CreateABListCard(listRow, getter_AddRefs(listCard));
        NotifyCardEntryChange(AB_NotifyInserted, listCard);

        NS_RELEASE(listRow);
        return NS_OK;
    }
    else 
        return NS_ERROR_FAILURE;

}

void nsAddrDatabase::DeleteCardFromAllMailLists(mdb_id cardRowID)
{
  if (!m_mdbEnv)
    return;

    nsCOMPtr <nsIMdbTableRowCursor> rowCursor;
    m_mdbPabTable->GetTableRowCursor(m_mdbEnv, -1, getter_AddRefs(rowCursor));

    if (rowCursor)
    {
        nsCOMPtr <nsIMdbRow> pListRow;
        mdb_pos rowPos;
        do 
        {
            mdb_err err = rowCursor->NextRow(m_mdbEnv, getter_AddRefs(pListRow), &rowPos);

            if (err == NS_OK && pListRow)
            {
                mdbOid rowOid;

                if (pListRow->GetOid(m_mdbEnv, &rowOid) == NS_OK)
                {
                    if (IsListRowScopeToken(rowOid.mOid_Scope))
                        DeleteCardFromListRow(pListRow, cardRowID);
                }
            }
        } while (pListRow);
    }
}

NS_IMETHODIMP nsAddrDatabase::DeleteCard(nsIAbCard *card, PRBool notify)
{
  if (!card || !m_mdbPabTable || !m_mdbStore || !m_mdbEnv)
    return NS_ERROR_NULL_POINTER;

  nsresult err = NS_OK;
  PRBool bIsMailList = PR_FALSE;
  card->GetIsMailList(&bIsMailList);

  // get the right row
  nsIMdbRow* pCardRow = nsnull;
  mdbOid rowOid;

  rowOid.mOid_Scope = bIsMailList ? m_ListRowScopeToken : m_CardRowScopeToken;

  nsCOMPtr<nsIAbMDBCard> dbcard(do_QueryInterface(card, &err));
  NS_ENSURE_SUCCESS(err, err);

  dbcard->GetDbRowID((PRUint32*)&rowOid.mOid_Id);

  err = m_mdbStore->GetRow(m_mdbEnv, &rowOid, &pCardRow);
  NS_ENSURE_SUCCESS(err,err);
  if (!pCardRow)
    return NS_OK;

  // Add the deleted card to the deletedcards table
  nsCOMPtr <nsIMdbRow> cardRow;
  AddRowToDeletedCardsTable(card, getter_AddRefs(cardRow));
  err = DeleteRow(m_mdbPabTable, pCardRow);
  
  //delete the person card from all mailing list
  if (!bIsMailList)
    DeleteCardFromAllMailLists(rowOid.mOid_Id);
    
  if (NS_SUCCEEDED(err)) {
    if (notify) 
      NotifyCardEntryChange(AB_NotifyDeleted, card);
  }
  else
    DeleteRowFromDeletedCardsTable(cardRow);

  NS_RELEASE(pCardRow);
  return NS_OK;
}

nsresult nsAddrDatabase::DeleteCardFromListRow(nsIMdbRow* pListRow, mdb_id cardRowID)
{
  NS_ENSURE_ARG_POINTER(pListRow);
  if (!m_mdbStore || !m_mdbEnv)
    return NS_ERROR_NULL_POINTER;

  nsresult err = NS_OK;

  PRUint32 totalAddress = GetListAddressTotal(pListRow);
   
  PRUint32 pos;
  for (pos = 1; pos <= totalAddress; pos++)
  {
    mdb_token listAddressColumnToken;
    mdb_id rowID;

    char columnStr[COLUMN_STR_MAX];
    PR_snprintf(columnStr, COLUMN_STR_MAX, kMailListAddressFormat, pos);
    m_mdbStore->StringToToken(m_mdbEnv, columnStr, &listAddressColumnToken);

    err = GetIntColumn(pListRow, listAddressColumnToken, (PRUint32*)&rowID, 0);

    if (cardRowID == rowID)
    {
      if (pos == totalAddress)
        err = pListRow->CutColumn(m_mdbEnv, listAddressColumnToken);
      else
      {
        //replace the deleted one with the last one and delete the last one
        mdb_id lastRowID;
        mdb_token lastAddressColumnToken;
        PR_snprintf(columnStr, COLUMN_STR_MAX, kMailListAddressFormat, totalAddress);
        m_mdbStore->StringToToken(m_mdbEnv, columnStr, &lastAddressColumnToken);

        err = GetIntColumn(pListRow, lastAddressColumnToken, (PRUint32*)&lastRowID, 0);
        NS_ENSURE_SUCCESS(err, err);

        err = AddIntColumn(pListRow, listAddressColumnToken, lastRowID);
        NS_ENSURE_SUCCESS(err, err);

        err = pListRow->CutColumn(m_mdbEnv, lastAddressColumnToken);
        NS_ENSURE_SUCCESS(err, err);
      }

      // Reset total count after the card has been deleted.
      SetListAddressTotal(pListRow, totalAddress-1);
      break;
    }
  }
  return NS_OK;
}

NS_IMETHODIMP nsAddrDatabase::DeleteCardFromMailList(nsIAbDirectory *mailList, nsIAbCard *card, PRBool aNotify)
{
  if (!card || !m_mdbPabTable || !m_mdbStore || !m_mdbEnv)
    return NS_ERROR_NULL_POINTER;

  nsresult err = NS_OK;

  // get the right row
  nsIMdbRow* pListRow = nsnull;
  mdbOid listRowOid;
  listRowOid.mOid_Scope = m_ListRowScopeToken;

  nsCOMPtr<nsIAbMDBDirectory> dbmailList(do_QueryInterface(mailList,&err));
  NS_ENSURE_SUCCESS(err, err);

  dbmailList->GetDbRowID((PRUint32*)&listRowOid.mOid_Id);

  err = m_mdbStore->GetRow(m_mdbEnv, &listRowOid, &pListRow);
  NS_ENSURE_SUCCESS(err,err);
  if (!pListRow)
    return NS_OK;

  PRUint32 cardRowID;
    
  nsCOMPtr<nsIAbMDBCard> dbcard(do_QueryInterface(card, &err));
  if(NS_FAILED(err) || !dbcard)
    return NS_ERROR_NULL_POINTER;
  dbcard->GetDbRowID(&cardRowID);
    
  err = DeleteCardFromListRow(pListRow, cardRowID);
  if (NS_SUCCEEDED(err) && aNotify) {            
    NotifyCardEntryChange(AB_NotifyDeleted, card);
  }
  NS_RELEASE(pListRow);
  return NS_OK;
}

NS_IMETHODIMP nsAddrDatabase::SetCardValue(nsIAbCard *card, const char *name, const PRUnichar *value, PRBool notify)
{
  NS_ENSURE_ARG_POINTER(card);
  NS_ENSURE_ARG_POINTER(name);
  NS_ENSURE_ARG_POINTER(value);
  if (!m_mdbStore || !m_mdbEnv)
    return NS_ERROR_NULL_POINTER;

  nsresult rv = NS_OK;
  
  nsCOMPtr <nsIMdbRow> cardRow;
  mdbOid rowOid;
  rowOid.mOid_Scope = m_CardRowScopeToken;
  
  // XXX todo
  // it might be that the caller always has a nsIAbMDBCard
  nsCOMPtr<nsIAbMDBCard> dbcard = do_QueryInterface(card, &rv);
  NS_ENSURE_SUCCESS(rv, rv);
  dbcard->GetDbRowID((PRUint32*)&rowOid.mOid_Id);
  
  rv = m_mdbStore->GetRow(m_mdbEnv, &rowOid, getter_AddRefs(cardRow));
  NS_ENSURE_SUCCESS(rv, rv);
  
  if (!cardRow)
    return NS_OK;
    
  mdb_token token;
  rv = m_mdbStore->StringToToken(m_mdbEnv, name, &token);
  NS_ENSURE_SUCCESS(rv, rv);
  
  rv = AddCharStringColumn(cardRow, token, NS_ConvertUTF16toUTF8(value).get());
  return rv;
}

NS_IMETHODIMP nsAddrDatabase::GetCardValue(nsIAbCard *card, const char *name, PRUnichar **value)
{
  if (!m_mdbStore || !card || !name || !value || !m_mdbEnv)
    return NS_ERROR_NULL_POINTER;

  nsresult rv = NS_OK;
  
  nsCOMPtr <nsIMdbRow> cardRow;
  mdbOid rowOid;
  rowOid.mOid_Scope = m_CardRowScopeToken;
  
  // XXX todo
  // it might be that the caller always has a nsIAbMDBCard
  nsCOMPtr<nsIAbMDBCard> dbcard = do_QueryInterface(card, &rv);
  NS_ENSURE_SUCCESS(rv, rv);
  dbcard->GetDbRowID((PRUint32*)&rowOid.mOid_Id);
  
  rv = m_mdbStore->GetRow(m_mdbEnv, &rowOid, getter_AddRefs(cardRow));
  NS_ENSURE_SUCCESS(rv, rv);
  
  if (!cardRow) {
    *value = nsnull;
    // this can happen when adding cards when editing a mailing list
    return NS_OK;
  }
  
  mdb_token token;
  m_mdbStore->StringToToken(m_mdbEnv, name, &token);
  
  // XXX fix me
  // avoid extra copying and allocations (did dmb already do this on the trunk?)
  nsAutoString tempString;
  rv = GetStringColumn(cardRow, token, tempString);
  if (NS_FAILED(rv)) {
    // not all cards are going this column
    *value = nsnull;
    return NS_OK;
  }
  
  *value = nsCRT::strdup(tempString.get());
  if (!*value) 
    return NS_ERROR_OUT_OF_MEMORY;
  return NS_OK;
}

NS_IMETHODIMP nsAddrDatabase::GetDeletedCardList(PRUint32 *aCount, nsISupportsArray **aDeletedList)
{
  if (!m_mdbEnv)
    return NS_ERROR_NULL_POINTER;

  nsCOMPtr<nsISupportsArray> resultCardArray;
  nsresult rv = NS_NewISupportsArray(getter_AddRefs(resultCardArray));
  if (NS_FAILED(rv)) return rv;
  *aCount = 0;
  // make sure the member is set properly
  InitDeletedCardsTable(PR_FALSE);
  if (m_mdbDeletedCardsTable)
  {
    nsCOMPtr<nsIMdbTableRowCursor>      rowCursor;
    mdb_pos                             rowPos;
    PRBool                              done = PR_FALSE;
    nsCOMPtr<nsIMdbRow>                 currentRow;

    m_mdbDeletedCardsTable->GetTableRowCursor(m_mdbEnv, -1, getter_AddRefs(rowCursor));
    if (!rowCursor)
        return NS_ERROR_FAILURE;
    while (!done)
    {
      nsresult rv = rowCursor->NextRow(m_mdbEnv, getter_AddRefs(currentRow), &rowPos);
      if (currentRow && NS_SUCCEEDED(rv))
      {
        mdbOid rowOid;
        if (currentRow->GetOid(m_mdbEnv, &rowOid) == NS_OK)
        {
          nsCOMPtr<nsIAbCard>    card;
          rv = CreateCardFromDeletedCardsTable(currentRow, 0, getter_AddRefs(card));
          if (NS_SUCCEEDED(rv)) {
            (*aCount) += 1;
            resultCardArray->AppendElement(card);
          }
        }
      }
      else
          done = PR_TRUE;
    }
    if (*aCount > 0)
      NS_IF_ADDREF(*aDeletedList = resultCardArray);
  }
  return NS_OK;
}

NS_IMETHODIMP nsAddrDatabase::GetDeletedCardCount(PRUint32 *aCount)
{
    // initialize count first
    *aCount = 0;
    InitDeletedCardsTable(PR_FALSE);
    if (m_mdbDeletedCardsTable)
      return m_mdbDeletedCardsTable->GetCount(m_mdbEnv, aCount);
    return NS_OK;
}

NS_IMETHODIMP nsAddrDatabase::PurgeDeletedCardTable()
{
  if (!m_mdbEnv)
    return NS_ERROR_NULL_POINTER;

    if (m_mdbDeletedCardsTable) {
        mdb_count cardCount=0;
        // if not too many cards let it be
        m_mdbDeletedCardsTable->GetCount(m_mdbEnv, &cardCount);
        if(cardCount < PURGE_CUTOFF_COUNT)
            return NS_OK;
        PRUint32 purgeTimeInSec;
        PRTime2Seconds(PR_Now(), &purgeTimeInSec);
        purgeTimeInSec -= (182*24*60*60);  // six months in seconds
        nsCOMPtr<nsIMdbTableRowCursor> rowCursor;
        nsresult rv = m_mdbDeletedCardsTable->GetTableRowCursor(m_mdbEnv, -1, getter_AddRefs(rowCursor));
        while(NS_SUCCEEDED(rv)) {
            nsCOMPtr<nsIMdbRow> currentRow;
            mdb_pos rowPos;
            rv = rowCursor->NextRow(m_mdbEnv, getter_AddRefs(currentRow), &rowPos);
            if(currentRow) {
                PRUint32 deletedTimeStamp = 0;
                GetIntColumn(currentRow, m_LastModDateColumnToken, &deletedTimeStamp, 0);
                // if record was deleted more than six months earlier, purge it
                if(deletedTimeStamp && (deletedTimeStamp < purgeTimeInSec)) {
                    if(NS_SUCCEEDED(currentRow->CutAllColumns(m_mdbEnv)))
                        m_mdbDeletedCardsTable->CutRow(m_mdbEnv, currentRow);
                }
                else
                    // since the ordering in Mork is maintained and thus
                    // the cards added later appear on the top when retrieved
                    break;
            }
            else
                break; // no more row
        }
    }

    return NS_OK;
}

NS_IMETHODIMP nsAddrDatabase::EditCard(nsIAbCard *card, PRBool notify)
{
  // XXX make sure this isn't getting called when we're just editing one or two well known fields
  if (!card || !m_mdbPabTable || !m_mdbStore || !m_mdbEnv)
    return NS_ERROR_NULL_POINTER;
  
  nsresult err = NS_OK;
  
  nsCOMPtr <nsIMdbRow> cardRow;
  mdbOid rowOid;
  rowOid.mOid_Scope = m_CardRowScopeToken;
  
  PRUint32 nowInSeconds;
  PRTime now = PR_Now();
  PRTime2Seconds(now, &nowInSeconds);
  card->SetLastModifiedDate(nowInSeconds);
  nsCOMPtr<nsIAbMDBCard> dbcard(do_QueryInterface(card, &err));
  NS_ENSURE_SUCCESS(err, err);
  dbcard->GetDbRowID((PRUint32*)&rowOid.mOid_Id);
  
  err = m_mdbStore->GetRow(m_mdbEnv, &rowOid, getter_AddRefs(cardRow));
  NS_ENSURE_SUCCESS(err, err);
  
  if (!cardRow)
    return NS_OK;
  
  err = AddAttributeColumnsToRow(card, cardRow);
  NS_ENSURE_SUCCESS(err, err);
  
  if (notify) 
    NotifyCardEntryChange(AB_NotifyPropertyChanged, card);
  
  return NS_OK;
}

NS_IMETHODIMP nsAddrDatabase::ContainsCard(nsIAbCard *card, PRBool *hasCard)
{
    if (!card || !m_mdbPabTable || !m_mdbEnv)
        return NS_ERROR_NULL_POINTER;

    nsresult err = NS_OK;
    mdb_bool hasOid;
    mdbOid rowOid;
    PRBool bIsMailList;

    card->GetIsMailList(&bIsMailList);
    
    if (bIsMailList)
        rowOid.mOid_Scope = m_ListRowScopeToken;
    else
        rowOid.mOid_Scope = m_CardRowScopeToken;

    nsCOMPtr<nsIAbMDBCard> dbcard(do_QueryInterface(card, &err));
    NS_ENSURE_SUCCESS(err, err);
    dbcard->GetDbRowID((PRUint32*)&rowOid.mOid_Id);

    err = m_mdbPabTable->HasOid(m_mdbEnv, &rowOid, &hasOid);
    if (NS_SUCCEEDED(err))
    {
        *hasCard = hasOid;
    }

    return err;
}

NS_IMETHODIMP nsAddrDatabase::DeleteMailList(nsIAbDirectory *mailList, PRBool notify)
{
  if (!mailList || !m_mdbPabTable || !m_mdbStore || !m_mdbEnv)
    return NS_ERROR_NULL_POINTER;

  nsresult err = NS_OK;

  // get the row
  nsIMdbRow* pListRow = nsnull;
  mdbOid rowOid;
  rowOid.mOid_Scope = m_ListRowScopeToken;

  nsCOMPtr<nsIAbMDBDirectory> dbmailList(do_QueryInterface(mailList,&err));
  NS_ENSURE_SUCCESS(err, err);
  dbmailList->GetDbRowID((PRUint32*)&rowOid.mOid_Id);

  err = m_mdbStore->GetRow(m_mdbEnv, &rowOid, &pListRow);
  NS_ENSURE_SUCCESS(err,err);

  if (!pListRow)
    return NS_OK;

  err = DeleteRow(m_mdbPabTable, pListRow);
  NS_RELEASE(pListRow);
  return err;
}

NS_IMETHODIMP nsAddrDatabase::EditMailList(nsIAbDirectory *mailList, nsIAbCard *listCard, PRBool notify)
{
  if (!mailList || !m_mdbPabTable || !m_mdbStore || !m_mdbEnv)
    return NS_ERROR_NULL_POINTER;

  nsresult err = NS_OK;

  nsIMdbRow* pListRow = nsnull;
  mdbOid rowOid;
  rowOid.mOid_Scope = m_ListRowScopeToken;

  nsCOMPtr<nsIAbMDBDirectory> dbmailList(do_QueryInterface(mailList,&err));
  NS_ENSURE_SUCCESS(err, err);
  dbmailList->GetDbRowID((PRUint32*)&rowOid.mOid_Id);

  err = m_mdbStore->GetRow(m_mdbEnv, &rowOid, &pListRow);
  NS_ENSURE_SUCCESS(err, err);

  if (!pListRow)
    return NS_OK;

  err = AddListAttributeColumnsToRow(mailList, pListRow);
  NS_ENSURE_SUCCESS(err, err);

  if (notify)
  {
    NotifyListEntryChange(AB_NotifyPropertyChanged, mailList);

    if (listCard)
    {
      NotifyCardEntryChange(AB_NotifyPropertyChanged, listCard);
    }
  }

  NS_RELEASE(pListRow);
  return NS_OK;
}

NS_IMETHODIMP nsAddrDatabase::ContainsMailList(nsIAbDirectory *mailList, PRBool *hasList)
{
    if (!mailList || !m_mdbPabTable || !m_mdbEnv)
        return NS_ERROR_NULL_POINTER;

    mdb_err err = NS_OK;
    mdb_bool hasOid;
    mdbOid rowOid;

    rowOid.mOid_Scope = m_ListRowScopeToken;

    nsCOMPtr<nsIAbMDBDirectory> dbmailList(do_QueryInterface(mailList,&err));
    NS_ENSURE_SUCCESS(err, err);
    dbmailList->GetDbRowID((PRUint32*)&rowOid.mOid_Id);

    err = m_mdbPabTable->HasOid(m_mdbEnv, &rowOid, &hasOid);
    if (err == NS_OK)
        *hasList = hasOid;

    return (err == NS_OK) ? NS_OK : NS_ERROR_FAILURE;
}

NS_IMETHODIMP nsAddrDatabase::GetNewRow(nsIMdbRow * *newRow)
{
  if (!m_mdbStore || !newRow || !m_mdbEnv)
    return NS_ERROR_NULL_POINTER;

  return m_mdbStore->NewRow(m_mdbEnv, m_CardRowScopeToken, newRow);
}

NS_IMETHODIMP nsAddrDatabase::GetNewListRow(nsIMdbRow * *newRow)
{
  if (!m_mdbStore || !newRow || !m_mdbEnv)
    return NS_ERROR_NULL_POINTER;

  return m_mdbStore->NewRow(m_mdbEnv, m_ListRowScopeToken, newRow);
}

NS_IMETHODIMP nsAddrDatabase::AddCardRowToDB(nsIMdbRow *newRow)
{
  if (m_mdbPabTable && m_mdbEnv)
  {
    if (m_mdbPabTable->AddRow(m_mdbEnv, newRow) == NS_OK)
    {
      AddRecordKeyColumnToRow(newRow);
      return NS_OK;
    }
    return NS_ERROR_FAILURE;
  }

  return NS_ERROR_FAILURE;
}
 
NS_IMETHODIMP nsAddrDatabase::AddLdifListMember(nsIMdbRow* listRow, const char* value)
{
  if (!m_mdbStore || !listRow || !value || !m_mdbEnv)
    return NS_ERROR_NULL_POINTER;

  PRUint32 total = GetListAddressTotal(listRow);
  //add member
  nsCAutoString valueString(value);
  nsCAutoString email;
  PRInt32 emailPos = valueString.Find("mail=");
  emailPos += strlen("mail=");
  valueString.Right(email, valueString.Length() - emailPos);
  nsCOMPtr <nsIMdbRow> cardRow;
  // Please DO NOT change the 3rd param of GetRowFromAttribute() call to 
  // PR_TRUE (ie, case insensitive) without reading bugs #128535 and #121478.
  nsresult rv = GetRowFromAttribute(kPriEmailColumn, email.get(), PR_FALSE /* retain case */, getter_AddRefs(cardRow));
  if (NS_SUCCEEDED(rv) && cardRow)
  {
    mdbOid outOid;
    mdb_id rowID = 0;
    if (cardRow->GetOid(m_mdbEnv, &outOid) == NS_OK)
      rowID = outOid.mOid_Id;

    // start from 1
    total += 1;
    mdb_token listAddressColumnToken;
    char columnStr[COLUMN_STR_MAX];
    PR_snprintf(columnStr, COLUMN_STR_MAX, kMailListAddressFormat, total); 
    m_mdbStore->StringToToken(m_mdbEnv, columnStr, &listAddressColumnToken);

    rv = AddIntColumn(listRow, listAddressColumnToken, rowID);
    NS_ENSURE_SUCCESS(rv, rv);

    SetListAddressTotal(listRow, total);
  }
  return NS_OK;
}
 

void nsAddrDatabase::GetCharStringYarn(char* str, struct mdbYarn* strYarn)
{
    strYarn->mYarn_Grow = nsnull;
    strYarn->mYarn_Buf = str;
    strYarn->mYarn_Size = PL_strlen((const char *) strYarn->mYarn_Buf) + 1;
    strYarn->mYarn_Fill = strYarn->mYarn_Size - 1;
    strYarn->mYarn_Form = 0;
}

void nsAddrDatabase::GetStringYarn(const nsAString & aStr, struct mdbYarn* strYarn)
{
    strYarn->mYarn_Buf = ToNewUTF8String(aStr);
    strYarn->mYarn_Size = PL_strlen((const char *) strYarn->mYarn_Buf) + 1;
    strYarn->mYarn_Fill = strYarn->mYarn_Size - 1;
    strYarn->mYarn_Form = 0;     
}

void nsAddrDatabase::GetIntYarn(PRUint32 nValue, struct mdbYarn* intYarn)
{
    intYarn->mYarn_Fill = intYarn->mYarn_Size;
    intYarn->mYarn_Form = 0;
    intYarn->mYarn_Grow = nsnull;

    PR_snprintf((char*)intYarn->mYarn_Buf, intYarn->mYarn_Size, "%lx", nValue);
    intYarn->mYarn_Fill = PL_strlen((const char *) intYarn->mYarn_Buf);
}

nsresult nsAddrDatabase::AddCharStringColumn(nsIMdbRow* cardRow, mdb_column inColumn, const char* str)
{
  if (!m_mdbEnv)
    return NS_ERROR_NULL_POINTER;

  struct mdbYarn yarn;

  GetCharStringYarn((char *) str, &yarn);
  mdb_err err = cardRow->AddColumn(m_mdbEnv,  inColumn, &yarn);

  return (err == NS_OK) ? NS_OK : NS_ERROR_FAILURE;
}

nsresult nsAddrDatabase::AddStringColumn(nsIMdbRow* aCardRow, mdb_column aInColumn, const nsAString & aStr)
{
  if (!m_mdbEnv)
    return NS_ERROR_NULL_POINTER;

  struct mdbYarn yarn;

  GetStringYarn(aStr, &yarn);
  mdb_err err = aCardRow->AddColumn(m_mdbEnv, aInColumn, &yarn);

  return (err == NS_OK) ? NS_OK : NS_ERROR_FAILURE;
}

nsresult nsAddrDatabase::AddIntColumn(nsIMdbRow* cardRow, mdb_column inColumn, PRUint32 nValue)
{
  if (!m_mdbEnv)
    return NS_ERROR_NULL_POINTER;

  struct mdbYarn yarn;
  char    yarnBuf[100];

  yarn.mYarn_Buf = (void *) yarnBuf;
  yarn.mYarn_Size = sizeof(yarnBuf);
  GetIntYarn(nValue, &yarn);
  mdb_err err = cardRow->AddColumn(m_mdbEnv,  inColumn, &yarn);

  return (err == NS_OK) ? NS_OK : NS_ERROR_FAILURE;
}

nsresult nsAddrDatabase::AddBoolColumn(nsIMdbRow* cardRow, mdb_column inColumn, PRBool bValue)
{
  if (!m_mdbEnv)
    return NS_ERROR_NULL_POINTER;

  struct mdbYarn yarn;
  char    yarnBuf[100];

  yarn.mYarn_Buf = (void *) yarnBuf;
  yarn.mYarn_Size = sizeof(yarnBuf);

  GetIntYarn(bValue ? 1 : 0, &yarn);

  mdb_err err = cardRow->AddColumn(m_mdbEnv, inColumn, &yarn);

  return (err == NS_OK) ? NS_OK : NS_ERROR_FAILURE;
}

nsresult nsAddrDatabase::GetStringColumn(nsIMdbRow *cardRow, mdb_token outToken, nsString& str)
{
  nsresult    err = NS_ERROR_NULL_POINTER;
  nsIMdbCell    *cardCell;
  
  if (cardRow && m_mdbEnv)
  {
    err = cardRow->GetCell(m_mdbEnv, outToken, &cardCell);
    if (err == NS_OK && cardCell)
    {
      struct mdbYarn yarn;
      cardCell->AliasYarn(m_mdbEnv, &yarn);
      NS_ConvertUTF8toUTF16 uniStr((const char*) yarn.mYarn_Buf, yarn.mYarn_Fill);
      if (!uniStr.IsEmpty())
        str.Assign(uniStr);
      else
        err = NS_ERROR_FAILURE;
      cardCell->Release(); // always release ref
    }
    else
      err = NS_ERROR_FAILURE;
  }
  return err;
}

void nsAddrDatabase::YarnToUInt32(struct mdbYarn *yarn, PRUint32 *pResult)
{
    PRUint32 i, result, numChars;
    char *p = (char *) yarn->mYarn_Buf;
    if (yarn->mYarn_Fill > 8)
        numChars = 8;
    else
        numChars = yarn->mYarn_Fill;
    for (i=0, result = 0; i < numChars; i++, p++)
    {
        char C = *p;

        PRInt8 unhex = ((C >= '0' && C <= '9') ? C - '0' :
            ((C >= 'A' && C <= 'F') ? C - 'A' + 10 :
             ((C >= 'a' && C <= 'f') ? C - 'a' + 10 : -1)));
        if (unhex < 0)
            break;
        result = (result << 4) | unhex;
    }
    
    *pResult = result;
}

nsresult nsAddrDatabase::GetIntColumn
(nsIMdbRow *cardRow, mdb_token outToken, PRUint32* pValue, PRUint32 defaultValue)
{
    nsresult    err = NS_ERROR_NULL_POINTER;
    nsIMdbCell    *cardCell;

    if (pValue)
        *pValue = defaultValue;
    if (cardRow && m_mdbEnv)
    {
        err = cardRow->GetCell(m_mdbEnv, outToken, &cardCell);
        if (err == NS_OK && cardCell)
        {
            struct mdbYarn yarn;
            cardCell->AliasYarn(m_mdbEnv, &yarn);
            YarnToUInt32(&yarn, pValue);
            cardCell->Release();
        }
        else
            err = NS_ERROR_FAILURE;
    }
    return err;
}

nsresult nsAddrDatabase::GetBoolColumn(nsIMdbRow *cardRow, mdb_token outToken, PRBool* pValue)
{
  NS_ENSURE_ARG_POINTER(pValue);

    nsresult    err = NS_ERROR_NULL_POINTER;
    nsIMdbCell    *cardCell;
    PRUint32 nValue = 0;

    if (cardRow && m_mdbEnv)
    {
        err = cardRow->GetCell(m_mdbEnv, outToken, &cardCell);
        if (err == NS_OK && cardCell)
        {
            struct mdbYarn yarn;
            cardCell->AliasYarn(m_mdbEnv, &yarn);
            YarnToUInt32(&yarn, &nValue);
            cardCell->Release();
        }
        else
            err = NS_ERROR_FAILURE;
    }

    *pValue = nValue ? PR_TRUE : PR_FALSE;
    return err;
}

/*  value is UTF8 string */
NS_IMETHODIMP nsAddrDatabase::AddPrimaryEmail(nsIMdbRow *aRow, const char *aValue)
{
  NS_ENSURE_ARG_POINTER(aValue);
  
  nsresult rv = AddCharStringColumn(aRow, m_PriEmailColumnToken, aValue);
  NS_ENSURE_SUCCESS(rv,rv);

  rv = AddLowercaseColumn(aRow, m_LowerPriEmailColumnToken, aValue);
  NS_ENSURE_SUCCESS(rv,rv);
  return rv;
}

/*  value is UTF8 string */
NS_IMETHODIMP nsAddrDatabase::AddListName(nsIMdbRow *aRow, const char *aValue)
{
  NS_ENSURE_ARG_POINTER(aValue);
  
  nsresult rv = AddCharStringColumn(aRow, m_ListNameColumnToken, aValue);
  NS_ENSURE_SUCCESS(rv,rv);

  rv = AddLowercaseColumn(aRow, m_LowerListNameColumnToken, aValue);
  NS_ENSURE_SUCCESS(rv,rv);
  return rv;
}

/* 
columnValue is UTF8 string, need to convert back to lowercase unicode then 
back to UTF8 string
*/
nsresult nsAddrDatabase::AddLowercaseColumn
(nsIMdbRow * row, mdb_token columnToken, const char* columnValue)
{
  nsresult rv = NS_OK;
  if (columnValue)
  {
    NS_ConvertUTF8toUTF16 newUnicodeString(columnValue);
    ToLowerCase(newUnicodeString);
    rv = AddCharStringColumn(row, columnToken, NS_ConvertUTF16toUTF8(newUnicodeString).get());   
  }
  return rv;
}

NS_IMETHODIMP nsAddrDatabase::InitCardFromRow(nsIAbCard *newCard, nsIMdbRow* cardRow)
{
    nsresult    err = NS_OK;
    if (!newCard || !cardRow)
        return NS_ERROR_NULL_POINTER;

  nsAutoString tempString;

  // FIX ME
  // there is no reason to set / copy all these attributes on the card, when we'll never even
  // ask for them.
    err = GetStringColumn(cardRow, m_FirstNameColumnToken, tempString);
    if (NS_SUCCEEDED(err) && !tempString.IsEmpty())
    {
        newCard->SetFirstName(tempString.get());
    }

    err = GetStringColumn(cardRow, m_LastNameColumnToken, tempString);
    if (NS_SUCCEEDED(err) && !tempString.IsEmpty())
    {
        newCard->SetLastName(tempString.get());
    }

    err = GetStringColumn(cardRow, m_PhoneticFirstNameColumnToken, tempString);
    if (NS_SUCCEEDED(err) && !tempString.IsEmpty())
    {
        newCard->SetPhoneticFirstName(tempString.get());
    }

    err = GetStringColumn(cardRow, m_PhoneticLastNameColumnToken, tempString);
    if (NS_SUCCEEDED(err) && !tempString.IsEmpty())
    {
        newCard->SetPhoneticLastName(tempString.get());
    }

    err = GetStringColumn(cardRow, m_DisplayNameColumnToken, tempString);
    if (NS_SUCCEEDED(err) && !tempString.IsEmpty())
    {
        newCard->SetDisplayName(tempString.get());
    }

    err = GetStringColumn(cardRow, m_NickNameColumnToken, tempString);
    if (NS_SUCCEEDED(err) && !tempString.IsEmpty())
    {
        newCard->SetNickName(tempString.get());
    }

    err = GetStringColumn(cardRow, m_PriEmailColumnToken, tempString);
    if (NS_SUCCEEDED(err) && !tempString.IsEmpty())
    {
        newCard->SetPrimaryEmail(tempString.get());
    }

    err = GetStringColumn(cardRow, m_2ndEmailColumnToken, tempString);
    if (NS_SUCCEEDED(err) && !tempString.IsEmpty())
    {
        newCard->SetSecondEmail(tempString.get());
    }

    PRUint32 format = nsIAbPreferMailFormat::unknown;
    err = GetIntColumn(cardRow, m_MailFormatColumnToken, &format, 0);
    if (NS_SUCCEEDED(err))
        newCard->SetPreferMailFormat(format);

    PRUint32 popularityIndex = 0;
    err = GetIntColumn(cardRow, m_PopularityIndexColumnToken, &popularityIndex, 0);
    if (NS_SUCCEEDED(err))
        newCard->SetPopularityIndex(popularityIndex);

    err = GetStringColumn(cardRow, m_WorkPhoneColumnToken, tempString);
    if (NS_SUCCEEDED(err) && !tempString.IsEmpty())
    {
        newCard->SetWorkPhone(tempString.get());
    }

    err = GetStringColumn(cardRow, m_HomePhoneColumnToken, tempString);
    if (NS_SUCCEEDED(err) && !tempString.IsEmpty())
    {
        newCard->SetHomePhone(tempString.get());
    }

    err = GetStringColumn(cardRow, m_FaxColumnToken, tempString);
    if (NS_SUCCEEDED(err) && !tempString.IsEmpty())
    {
        newCard->SetFaxNumber(tempString.get());
    }

    err = GetStringColumn(cardRow, m_PagerColumnToken, tempString);
    if (NS_SUCCEEDED(err) && !tempString.IsEmpty())
    {
        newCard->SetPagerNumber(tempString.get());
    }

    err = GetStringColumn(cardRow, m_CellularColumnToken, tempString);
    if (NS_SUCCEEDED(err) && !tempString.IsEmpty())
    {
        newCard->SetCellularNumber(tempString.get());
    }

    err = GetStringColumn(cardRow, m_WorkPhoneTypeColumnToken, tempString);
    if (NS_SUCCEEDED(err) && !tempString.IsEmpty())
        newCard->SetWorkPhoneType(tempString.get());

    err = GetStringColumn(cardRow, m_HomePhoneTypeColumnToken, tempString);
    if (NS_SUCCEEDED(err) && !tempString.IsEmpty())
        newCard->SetHomePhoneType(tempString.get());

    err = GetStringColumn(cardRow, m_FaxTypeColumnToken, tempString);
    if (NS_SUCCEEDED(err) && !tempString.IsEmpty())
        newCard->SetFaxNumberType(tempString.get());

    err = GetStringColumn(cardRow, m_PagerTypeColumnToken, tempString);
    if (NS_SUCCEEDED(err) && !tempString.IsEmpty())
        newCard->SetPagerNumberType(tempString.get());

    err = GetStringColumn(cardRow, m_CellularTypeColumnToken, tempString);
    if (NS_SUCCEEDED(err) && !tempString.IsEmpty())
        newCard->SetCellularNumberType(tempString.get());

    err = GetStringColumn(cardRow, m_HomeAddressColumnToken, tempString);
    if (NS_SUCCEEDED(err) && !tempString.IsEmpty())
    {
        newCard->SetHomeAddress(tempString.get());
    }

    err = GetStringColumn(cardRow, m_HomeAddress2ColumnToken, tempString);
    if (NS_SUCCEEDED(err) && !tempString.IsEmpty())
    {
        newCard->SetHomeAddress2(tempString.get());
    }

    err = GetStringColumn(cardRow, m_HomeCityColumnToken, tempString);
    if (NS_SUCCEEDED(err) && !tempString.IsEmpty())
    {
        newCard->SetHomeCity(tempString.get());
    }

    err = GetStringColumn(cardRow, m_HomeStateColumnToken, tempString);
    if (NS_SUCCEEDED(err) && !tempString.IsEmpty())
    {
        newCard->SetHomeState(tempString.get());
    }

    err = GetStringColumn(cardRow, m_HomeZipCodeColumnToken, tempString);
    if (NS_SUCCEEDED(err) && !tempString.IsEmpty())
    {
        newCard->SetHomeZipCode(tempString.get());
    }

    err = GetStringColumn(cardRow, m_HomeCountryColumnToken, tempString);
    if (NS_SUCCEEDED(err) && !tempString.IsEmpty())
    {
        newCard->SetHomeCountry(tempString.get());
    }

    err = GetStringColumn(cardRow, m_WorkAddressColumnToken, tempString);
    if (NS_SUCCEEDED(err) && !tempString.IsEmpty())
    {
        newCard->SetWorkAddress(tempString.get());
    }

    err = GetStringColumn(cardRow, m_WorkAddress2ColumnToken, tempString);
    if (NS_SUCCEEDED(err) && !tempString.IsEmpty())
    {
        newCard->SetWorkAddress2(tempString.get());
    }

    err = GetStringColumn(cardRow, m_WorkCityColumnToken, tempString);
    if (NS_SUCCEEDED(err) && !tempString.IsEmpty())
    {
        newCard->SetWorkCity(tempString.get());
    }

    err = GetStringColumn(cardRow, m_WorkStateColumnToken, tempString);
    if (NS_SUCCEEDED(err) && !tempString.IsEmpty())
    {
        newCard->SetWorkState(tempString.get());
    }

    err = GetStringColumn(cardRow, m_WorkZipCodeColumnToken, tempString);
    if (NS_SUCCEEDED(err) && !tempString.IsEmpty())
    {
        newCard->SetWorkZipCode(tempString.get());
    }

    err = GetStringColumn(cardRow, m_WorkCountryColumnToken, tempString);
    if (NS_SUCCEEDED(err) && !tempString.IsEmpty())
    {
        newCard->SetWorkCountry(tempString.get());
    }

    err = GetStringColumn(cardRow, m_JobTitleColumnToken, tempString);
    if (NS_SUCCEEDED(err) && !tempString.IsEmpty())
    {
        newCard->SetJobTitle(tempString.get());
    }

    err = GetStringColumn(cardRow, m_DepartmentColumnToken, tempString);
    if (NS_SUCCEEDED(err) && !tempString.IsEmpty())
    {
        newCard->SetDepartment(tempString.get());
    }

    err = GetStringColumn(cardRow, m_CompanyColumnToken, tempString);
    if (NS_SUCCEEDED(err) && !tempString.IsEmpty())
    {
        newCard->SetCompany(tempString.get());
    }

    // AimScreenName
    err = GetStringColumn(cardRow, m_AimScreenNameColumnToken, tempString);
    if (NS_SUCCEEDED(err) && !tempString.IsEmpty())
        newCard->SetAimScreenName(tempString.get());

    err = GetStringColumn(cardRow, m_AnniversaryYearColumnToken, tempString);
    if (NS_SUCCEEDED(err) && !tempString.IsEmpty())
        newCard->SetAnniversaryYear(tempString.get());

    err = GetStringColumn(cardRow, m_AnniversaryMonthColumnToken, tempString);
    if (NS_SUCCEEDED(err) && !tempString.IsEmpty())
        newCard->SetAnniversaryMonth(tempString.get());

    err = GetStringColumn(cardRow, m_AnniversaryDayColumnToken, tempString);
    if (NS_SUCCEEDED(err) && !tempString.IsEmpty())
        newCard->SetAnniversaryDay(tempString.get());

    err = GetStringColumn(cardRow, m_SpouseNameColumnToken, tempString);
    if (NS_SUCCEEDED(err) && !tempString.IsEmpty())
        newCard->SetSpouseName(tempString.get());

    err = GetStringColumn(cardRow, m_FamilyNameColumnToken, tempString);
    if (NS_SUCCEEDED(err) && !tempString.IsEmpty())
        newCard->SetFamilyName(tempString.get());

    err = GetStringColumn(cardRow, m_DefaultAddressColumnToken, tempString);
    if (NS_SUCCEEDED(err) && !tempString.IsEmpty())
        newCard->SetDefaultAddress(tempString.get());

    err = GetStringColumn(cardRow, m_CategoryColumnToken, tempString);
    if (NS_SUCCEEDED(err) && !tempString.IsEmpty())
        newCard->SetCategory(tempString.get());

    err = GetStringColumn(cardRow, m_WebPage1ColumnToken, tempString);
    if (NS_SUCCEEDED(err) && !tempString.IsEmpty())
    {
        newCard->SetWebPage1(tempString.get());
    }

    err = GetStringColumn(cardRow, m_WebPage2ColumnToken, tempString);
    if (NS_SUCCEEDED(err) && !tempString.IsEmpty())
    {
        newCard->SetWebPage2(tempString.get());
    }

    err = GetStringColumn(cardRow, m_BirthYearColumnToken, tempString);
    if (NS_SUCCEEDED(err) && !tempString.IsEmpty())
    {
        newCard->SetBirthYear(tempString.get());
    }

    err = GetStringColumn(cardRow, m_BirthMonthColumnToken, tempString);
    if (NS_SUCCEEDED(err) && !tempString.IsEmpty())
    {
        newCard->SetBirthMonth(tempString.get());
    }

    err = GetStringColumn(cardRow, m_BirthDayColumnToken, tempString);
    if (NS_SUCCEEDED(err) && !tempString.IsEmpty())
    {
        newCard->SetBirthDay(tempString.get());
    }

    err = GetStringColumn(cardRow, m_Custom1ColumnToken, tempString);
    if (NS_SUCCEEDED(err) && !tempString.IsEmpty())
    {
        newCard->SetCustom1(tempString.get());
    }

    err = GetStringColumn(cardRow, m_Custom2ColumnToken, tempString);
    if (NS_SUCCEEDED(err) && !tempString.IsEmpty())
    {
        newCard->SetCustom2(tempString.get());
    }

    err = GetStringColumn(cardRow, m_Custom3ColumnToken, tempString);
    if (NS_SUCCEEDED(err) && !tempString.IsEmpty())
    {
        newCard->SetCustom3(tempString.get());
    }

    err = GetStringColumn(cardRow, m_Custom4ColumnToken, tempString);
    if (NS_SUCCEEDED(err) && !tempString.IsEmpty())
    {
        newCard->SetCustom4(tempString.get());
    }

    err = GetStringColumn(cardRow, m_NotesColumnToken, tempString);
    if (NS_SUCCEEDED(err) && !tempString.IsEmpty())
    {
        newCard->SetNotes(tempString.get());
    }
    PRUint32 lastModDate = 0;
    err = GetIntColumn(cardRow, m_LastModDateColumnToken, &lastModDate, 0);
    if (NS_SUCCEEDED(err))
      newCard->SetLastModifiedDate(lastModDate);

    PRUint32 key = 0;
    err = GetIntColumn(cardRow, m_RecordKeyColumnToken, &key, 0);
    if (NS_SUCCEEDED(err))
    {
        nsCOMPtr<nsIAbMDBCard> dbnewCard(do_QueryInterface(newCard, &err));
        if (NS_SUCCEEDED(err) && dbnewCard)
            dbnewCard->SetKey(key);
    }

    return err;
}

nsresult nsAddrDatabase::GetListCardFromDB(nsIAbCard *listCard, nsIMdbRow* listRow)
{
    nsresult    err = NS_OK;
    if (!listCard || !listRow)
        return NS_ERROR_NULL_POINTER;

    nsAutoString tempString;

    err = GetStringColumn(listRow, m_ListNameColumnToken, tempString);
    if (NS_SUCCEEDED(err) && !tempString.IsEmpty())
    {
        listCard->SetDisplayName(tempString.get());
        listCard->SetLastName(tempString.get());
    }
    err = GetStringColumn(listRow, m_ListNickNameColumnToken, tempString);
    if (NS_SUCCEEDED(err) && !tempString.IsEmpty())
    {
        listCard->SetNickName(tempString.get());
    }
    err = GetStringColumn(listRow, m_ListDescriptionColumnToken, tempString);
    if (NS_SUCCEEDED(err) && !tempString.IsEmpty())
    {
        listCard->SetNotes(tempString.get());
    }
    PRUint32 key = 0;
    err = GetIntColumn(listRow, m_RecordKeyColumnToken, &key, 0);
    if (NS_SUCCEEDED(err))
    {
        nsCOMPtr<nsIAbMDBCard> dblistCard(do_QueryInterface(listCard, &err));
        if (NS_SUCCEEDED(err) && dblistCard)
            dblistCard->SetKey(key);
    }
    return err;
}

nsresult nsAddrDatabase::GetListFromDB(nsIAbDirectory *newList, nsIMdbRow* listRow)
{
  nsresult    err = NS_OK;
  if (!newList || !listRow || !m_mdbStore || !m_mdbEnv)
    return NS_ERROR_NULL_POINTER;

  nsAutoString tempString;

  err = GetStringColumn(listRow, m_ListNameColumnToken, tempString);
  if (NS_SUCCEEDED(err) && !tempString.IsEmpty())
  {
    newList->SetDirName(tempString.get());
  }
  err = GetStringColumn(listRow, m_ListNickNameColumnToken, tempString);
  if (NS_SUCCEEDED(err) && !tempString.IsEmpty())
  {
    newList->SetListNickName(tempString.get());
  }
  err = GetStringColumn(listRow, m_ListDescriptionColumnToken, tempString);
  if (NS_SUCCEEDED(err) && !tempString.IsEmpty())
  {
    newList->SetDescription(tempString.get());
  }

  PRUint32 totalAddress = GetListAddressTotal(listRow);
  PRUint32 pos;
  for (pos = 1; pos <= totalAddress; pos++)
  {
    mdb_token listAddressColumnToken;
    mdb_id rowID;

    char columnStr[COLUMN_STR_MAX];
    PR_snprintf(columnStr, COLUMN_STR_MAX, kMailListAddressFormat, pos);
    m_mdbStore->StringToToken(m_mdbEnv, columnStr, &listAddressColumnToken);

    nsCOMPtr <nsIMdbRow> cardRow;
    err = GetIntColumn(listRow, listAddressColumnToken, (PRUint32*)&rowID, 0);
    NS_ENSURE_SUCCESS(err, err);
    err = GetCardRowByRowID(rowID, getter_AddRefs(cardRow));
    NS_ENSURE_SUCCESS(err, err);
        
    if (cardRow)
    {
      nsCOMPtr<nsIAbCard> card;
      err = CreateABCard(cardRow, 0, getter_AddRefs(card));

      nsCOMPtr<nsIAbMDBDirectory> dbnewList(do_QueryInterface(newList, &err));
      if(NS_SUCCEEDED(err))
        dbnewList->AddAddressToList(card);
    }
//        NS_IF_ADDREF(card);
  }

  return err;
}

class nsAddrDBEnumerator : public nsISimpleEnumerator 
{
public:
    NS_DECL_ISUPPORTS

    // nsISimpleEnumerator methods:
    NS_DECL_NSISIMPLEENUMERATOR

    // nsAddrDBEnumerator methods:

    nsAddrDBEnumerator(nsAddrDatabase* aDb);

protected:
    nsRefPtr<nsAddrDatabase> mDb;
    nsIMdbTable *mDbTable;
    nsCOMPtr<nsIMdbTableRowCursor> mRowCursor;
    nsCOMPtr<nsIMdbRow> mCurrentRow;
    mdb_pos mRowPos;
};

nsAddrDBEnumerator::nsAddrDBEnumerator(nsAddrDatabase* aDb)
    : mDb(aDb),
      mDbTable(aDb->GetPabTable()),
      mRowPos(-1)
{
}

NS_IMPL_ISUPPORTS1(nsAddrDBEnumerator, nsISimpleEnumerator)

NS_IMETHODIMP
nsAddrDBEnumerator::HasMoreElements(PRBool *aResult)
{
  NS_ENSURE_ARG_POINTER(aResult);
    *aResult = PR_FALSE;

    if (!mDbTable || !mDb->GetEnv())
    {
        return NS_ERROR_NULL_POINTER;
    }

    nsCOMPtr<nsIMdbTableRowCursor> rowCursor;
    mDbTable->GetTableRowCursor(mDb->GetEnv(), mRowPos,
                                getter_AddRefs(rowCursor));
    NS_ENSURE_TRUE(rowCursor, NS_ERROR_FAILURE);

    mdbOid rowOid;
    rowCursor->NextRowOid(mDb->GetEnv(), &rowOid, nsnull);
    while (rowOid.mOid_Id != (mdb_id)-1)
    {
        if (mDb->IsListRowScopeToken(rowOid.mOid_Scope) ||
            mDb->IsCardRowScopeToken(rowOid.mOid_Scope))
        {
            *aResult = PR_TRUE;

            return NS_OK;
        }

        if (!mDb->IsDataRowScopeToken(rowOid.mOid_Scope))
        {
            return NS_ERROR_FAILURE;
        }

        rowCursor->NextRowOid(mDb->GetEnv(), &rowOid, nsnull);
    }

    return NS_OK;
}

NS_IMETHODIMP
nsAddrDBEnumerator::GetNext(nsISupports **aResult)
{
  NS_ENSURE_ARG_POINTER(aResult);

    *aResult = nsnull;

    if (!mDbTable || !mDb->GetEnv())
    {
        return NS_ERROR_NULL_POINTER;
    }

    if (!mRowCursor)
    {
         mDbTable->GetTableRowCursor(mDb->GetEnv(), -1,
                                     getter_AddRefs(mRowCursor));
         NS_ENSURE_TRUE(mRowCursor, NS_ERROR_FAILURE);
    }

    nsCOMPtr<nsIAbCard> resultCard;
    mRowCursor->NextRow(mDb->GetEnv(), getter_AddRefs(mCurrentRow), &mRowPos);
    while (mCurrentRow)
    {
        mdbOid rowOid;
        if (NS_SUCCEEDED(mCurrentRow->GetOid(mDb->GetEnv(), &rowOid)))
        {
            nsresult rv;
            if (mDb->IsListRowScopeToken(rowOid.mOid_Scope))
            {
                rv = mDb->CreateABListCard(mCurrentRow,
                                           getter_AddRefs(resultCard));
                NS_ENSURE_SUCCESS(rv, rv);
            }
            else if (mDb->IsCardRowScopeToken(rowOid.mOid_Scope))
            {
                rv = mDb->CreateABCard(mCurrentRow, 0,
                                       getter_AddRefs(resultCard));
                NS_ENSURE_SUCCESS(rv, rv);
            }
            else if (!mDb->IsDataRowScopeToken(rowOid.mOid_Scope))
            {
                return NS_ERROR_FAILURE;
            }

            if (resultCard)
            {
                return CallQueryInterface(resultCard, aResult);
            }
        }

        mRowCursor->NextRow(mDb->GetEnv(), getter_AddRefs(mCurrentRow),
                            &mRowPos);
    }

    return NS_ERROR_FAILURE;
}

class nsListAddressEnumerator : public nsISimpleEnumerator 
{
public:
    NS_DECL_ISUPPORTS

    // nsIEnumerator methods:
    NS_DECL_NSISIMPLEENUMERATOR

    // nsListAddressEnumerator methods:

    nsListAddressEnumerator(nsAddrDatabase* aDb, mdb_id aRowID);

protected:
    nsRefPtr<nsAddrDatabase> mDb;
    nsIMdbTable *mDbTable;
    nsCOMPtr<nsIMdbRow> mListRow;
    mdb_id mListRowID;
    PRUint32 mAddressTotal;
    PRUint16 mAddressPos;
};

nsListAddressEnumerator::nsListAddressEnumerator(nsAddrDatabase* aDb,
                                                 mdb_id aRowID)
    : mDb(aDb),
      mDbTable(aDb->GetPabTable()),
      mListRowID(aRowID),
      mAddressPos(0)
{
    mDb->GetListRowByRowID(mListRowID, getter_AddRefs(mListRow));
    mAddressTotal = aDb->GetListAddressTotal(mListRow);
}

NS_IMPL_ISUPPORTS1(nsListAddressEnumerator, nsISimpleEnumerator)

NS_IMETHODIMP
nsListAddressEnumerator::HasMoreElements(PRBool *aResult)
{
  NS_ENSURE_ARG_POINTER(aResult);

  *aResult = PR_FALSE;

  if (!mDbTable || !mDb->GetEnv())
  {
    return NS_ERROR_NULL_POINTER;
  }

  // In some cases it is possible that GetAddressRowByPos returns success,
  // but currentRow is null. This is typically due to the fact that a card
  // has been deleted from the parent and not the list. Whilst we have fixed
  // that there are still a few dbs around there that we need to support
  // correctly. Therefore, whilst processing lists ensure that we don't return
  // false if the only thing stopping us is a blank row, just skip it and try
  // the next one.
  while (mAddressPos < mAddressTotal)
  {
    nsCOMPtr<nsIMdbRow> currentRow;
    nsresult rv = mDb->GetAddressRowByPos(mListRow, mAddressPos + 1,
                                          getter_AddRefs(currentRow));
    NS_ENSURE_SUCCESS(rv, rv);

    if (currentRow)
    {
      *aResult = PR_TRUE;
      break;
    }

    ++mAddressPos;
  }

  return NS_OK;
}

NS_IMETHODIMP
nsListAddressEnumerator::GetNext(nsISupports **aResult)
{
  NS_ENSURE_ARG_POINTER(aResult);

    *aResult = nsnull;

    if (!mDbTable || !mDb->GetEnv())
    {
        return NS_ERROR_NULL_POINTER;
    }

    if (++mAddressPos <= mAddressTotal)
    {
        nsCOMPtr<nsIMdbRow> currentRow;
        nsresult rv = mDb->GetAddressRowByPos(mListRow, mAddressPos,
                                              getter_AddRefs(currentRow));
        NS_ENSURE_SUCCESS(rv, rv);

        nsCOMPtr<nsIAbCard> resultCard;
        rv = mDb->CreateABCard(currentRow, mListRowID,
                               getter_AddRefs(resultCard));
        NS_ENSURE_SUCCESS(rv, rv);

        return CallQueryInterface(resultCard, aResult);
    }

    return NS_ERROR_FAILURE;
}

////////////////////////////////////////////////////////////////////////////////

NS_IMETHODIMP nsAddrDatabase::EnumerateCards(nsIAbDirectory *directory, nsISimpleEnumerator **result)
{
    nsAddrDBEnumerator* e = new nsAddrDBEnumerator(this);
    m_dbDirectory = directory;
    if (!e)
        return NS_ERROR_OUT_OF_MEMORY;
    NS_ADDREF(e);
    *result = e;
    return NS_OK;
}

NS_IMETHODIMP nsAddrDatabase::GetMailingListsFromDB(nsIAbDirectory *parentDir)
{
    nsCOMPtr<nsIAbDirectory>    resultList;
    nsIMdbTableRowCursor*       rowCursor = nsnull;
    nsCOMPtr<nsIMdbRow> currentRow;
     mdb_pos                        rowPos;
    PRBool                      done = PR_FALSE;

  if (!m_mdbEnv)
    return NS_ERROR_NULL_POINTER;

    m_dbDirectory = parentDir;

    nsIMdbTable*                dbTable = GetPabTable();

    if (!dbTable)
        return NS_ERROR_FAILURE;

    dbTable->GetTableRowCursor(m_mdbEnv, -1, &rowCursor);
    if (!rowCursor)
        return NS_ERROR_FAILURE;

    while (!done)
    {
                nsresult rv = rowCursor->NextRow(m_mdbEnv, getter_AddRefs(currentRow), &rowPos);
        if (currentRow && NS_SUCCEEDED(rv))
        {
            mdbOid rowOid;

            if (currentRow->GetOid(m_mdbEnv, &rowOid) == NS_OK)
            {
                if (IsListRowScopeToken(rowOid.mOid_Scope))
                    rv = CreateABList(currentRow, getter_AddRefs(resultList));
            }
        }
        else
            done = PR_TRUE;
    }
        NS_IF_RELEASE(rowCursor);
    return NS_OK;
}

NS_IMETHODIMP nsAddrDatabase::EnumerateListAddresses(nsIAbDirectory *directory, nsISimpleEnumerator **result)
{
    nsresult rv = NS_OK; 
    mdb_id rowID;

    nsCOMPtr<nsIAbMDBDirectory> dbdirectory(do_QueryInterface(directory,&rv));
    
    if(NS_SUCCEEDED(rv))
    {
    dbdirectory->GetDbRowID((PRUint32*)&rowID);

    nsListAddressEnumerator* e = new nsListAddressEnumerator(this, rowID);
    m_dbDirectory = directory;
    if (!e)
        return NS_ERROR_OUT_OF_MEMORY;
    NS_ADDREF(e);
    *result = e;
    }
    return rv;
}

nsresult nsAddrDatabase::CreateCardFromDeletedCardsTable(nsIMdbRow* cardRow, mdb_id listRowID, nsIAbCard **result)
{
  if (!cardRow || !m_mdbEnv || !result)
    return NS_ERROR_NULL_POINTER;

    nsresult rv = NS_OK; 

    mdbOid outOid;
    mdb_id rowID=0;

    if (cardRow->GetOid(m_mdbEnv, &outOid) == NS_OK)
        rowID = outOid.mOid_Id;

    if(NS_SUCCEEDED(rv))
    {
        nsCOMPtr<nsIAbCard> personCard;
        personCard = do_CreateInstance(NS_ABMDBCARD_CONTRACTID, &rv);
        NS_ENSURE_SUCCESS(rv,rv);

        nsCOMPtr<nsIAbMDBCard> dbpersonCard (do_QueryInterface(personCard, &rv));

        if (NS_SUCCEEDED(rv) && dbpersonCard)
        {
            InitCardFromRow(personCard, cardRow);
            mdbOid tableOid;
            m_mdbDeletedCardsTable->GetOid(m_mdbEnv, &tableOid);

            dbpersonCard->SetDbTableID(tableOid.mOid_Id);
            dbpersonCard->SetDbRowID(rowID);
            dbpersonCard->SetAbDatabase(this);
        }

        NS_IF_ADDREF(*result = personCard);
    }

    return rv;
}

nsresult nsAddrDatabase::CreateCard(nsIMdbRow* cardRow, mdb_id listRowID, nsIAbCard **result)
{
  if (!cardRow || !m_mdbEnv || !result)
    return NS_ERROR_NULL_POINTER;

    nsresult rv = NS_OK; 

    mdbOid outOid;
    mdb_id rowID=0;

    if (cardRow->GetOid(m_mdbEnv, &outOid) == NS_OK)
        rowID = outOid.mOid_Id;

    if(NS_SUCCEEDED(rv))
    {
        nsCOMPtr<nsIAbCard> personCard;
      personCard = do_CreateInstance(NS_ABMDBCARD_CONTRACTID, &rv);
      NS_ENSURE_SUCCESS(rv,rv);

        nsCOMPtr<nsIAbMDBCard> dbpersonCard (do_QueryInterface(personCard, &rv));

        if (NS_SUCCEEDED(rv) && dbpersonCard)
        {
            InitCardFromRow(personCard, cardRow);
            mdbOid tableOid;
            m_mdbPabTable->GetOid(m_mdbEnv, &tableOid);

            dbpersonCard->SetDbTableID(tableOid.mOid_Id);
            dbpersonCard->SetDbRowID(rowID);
            dbpersonCard->SetAbDatabase(this);
        }

        NS_IF_ADDREF(*result = personCard);
    }

    return rv;
}

nsresult nsAddrDatabase::CreateABCard(nsIMdbRow* cardRow, mdb_id listRowID, nsIAbCard **result)
{
    return CreateCard(cardRow, listRowID, result);
}

/* create a card for mailing list in the address book */
nsresult nsAddrDatabase::CreateABListCard(nsIMdbRow* listRow, nsIAbCard **result)
{
  if (!listRow || !m_mdbEnv || !result)
    return NS_ERROR_NULL_POINTER;

    nsresult rv = NS_OK; 

    mdbOid outOid;
    mdb_id rowID=0;

    if (listRow->GetOid(m_mdbEnv, &outOid) == NS_OK)
        rowID = outOid.mOid_Id;

    char* listURI = nsnull;

    nsAutoString fileName;
    rv = m_dbName->GetLeafName(fileName);
    NS_ENSURE_SUCCESS(rv, rv);
    listURI = PR_smprintf("%s%s/MailList%ld", kMDBDirectoryRoot, NS_ConvertUTF16toUTF8(fileName).get(), rowID);

    nsCOMPtr<nsIAbCard> personCard;
    nsCOMPtr<nsIAbMDBDirectory> dbm_dbDirectory(do_QueryInterface(m_dbDirectory, &rv));
    if(NS_SUCCEEDED(rv) && dbm_dbDirectory)
    {
    personCard = do_CreateInstance(NS_ABMDBCARD_CONTRACTID, &rv);
        NS_ENSURE_SUCCESS(rv,rv);

        if (personCard)
        {
            GetListCardFromDB(personCard, listRow);
            mdbOid tableOid;
            m_mdbPabTable->GetOid(m_mdbEnv, &tableOid);
    
            nsCOMPtr<nsIAbMDBCard> dbpersonCard(do_QueryInterface(personCard, &rv));
      if (NS_SUCCEEDED(rv) && dbpersonCard)
      {
              dbpersonCard->SetDbTableID(tableOid.mOid_Id);
              dbpersonCard->SetDbRowID(rowID);
              dbpersonCard->SetAbDatabase(this);
      }
            personCard->SetIsMailList(PR_TRUE);
            personCard->SetMailListURI(listURI);
        }
        
        NS_IF_ADDREF(*result = personCard);
    }
    if (listURI)
        PR_smprintf_free(listURI);

    return rv;
}

/* create a sub directory for mailing list in the address book left pane */
nsresult nsAddrDatabase::CreateABList(nsIMdbRow* listRow, nsIAbDirectory **result)
{
    nsresult rv = NS_OK; 

    if (!listRow || !m_mdbEnv || !result)
        return NS_ERROR_NULL_POINTER;

    mdbOid outOid;
    mdb_id rowID=0;

    if (listRow->GetOid(m_mdbEnv, &outOid) == NS_OK)
        rowID = outOid.mOid_Id;

    char* listURI = nsnull;

    nsAutoString fileName;
    m_dbName->GetLeafName(fileName);
    NS_ENSURE_SUCCESS(rv, rv);

    listURI = PR_smprintf("%s%s/MailList%ld", kMDBDirectoryRoot, NS_ConvertUTF16toUTF8(fileName).get(), rowID);

    nsCOMPtr<nsIAbDirectory> mailList;
    nsCOMPtr<nsIAbMDBDirectory> dbm_dbDirectory(do_QueryInterface(m_dbDirectory, &rv));
    if(NS_SUCCEEDED(rv) && dbm_dbDirectory)
    {
        rv = dbm_dbDirectory->AddDirectory(listURI, getter_AddRefs(mailList));

        nsCOMPtr<nsIAbMDBDirectory> dbmailList (do_QueryInterface(mailList, &rv));

        if (mailList) 
        {
            // if we are using turbo, and we "exit" and restart with the same profile
            // the current mailing list will still be in memory, so when we do
            // GetResource() and QI, we'll get it again.
            // in that scenario, the mailList that we pass in will already be
            // be a mailing list, with a valid row and all the entries
            // in that scenario, we can skip GetListFromDB(), which would have
            // have added all the cards to the list again.
            // see bug #134743
            mdb_id existingID;
            dbmailList->GetDbRowID(&existingID);
            if (existingID != rowID) {
              GetListFromDB(mailList, listRow);
              dbmailList->SetDbRowID(rowID);
              mailList->SetIsMailList(PR_TRUE);
            }

            dbm_dbDirectory->AddMailListToDirectory(mailList);
            NS_IF_ADDREF(*result = mailList);
        }
    }

    if (listURI)
        PR_smprintf_free(listURI);

    return rv;
}

nsresult nsAddrDatabase::GetCardRowByRowID(mdb_id rowID, nsIMdbRow **dbRow)
{
  if (!m_mdbStore || !m_mdbEnv)
    return NS_ERROR_NULL_POINTER;

  mdbOid rowOid;
  rowOid.mOid_Scope = m_CardRowScopeToken;
  rowOid.mOid_Id = rowID;

  return m_mdbStore->GetRow(m_mdbEnv, &rowOid, dbRow);
}

nsresult nsAddrDatabase::GetListRowByRowID(mdb_id rowID, nsIMdbRow **dbRow)
{
  if (!m_mdbStore || !m_mdbEnv)
    return NS_ERROR_NULL_POINTER;

  mdbOid rowOid;
  rowOid.mOid_Scope = m_ListRowScopeToken;
  rowOid.mOid_Id = rowID;

  return m_mdbStore->GetRow(m_mdbEnv, &rowOid, dbRow);
}

nsresult nsAddrDatabase::GetRowFromAttribute(const char *aName, const char *aUTF8Value, PRBool aCaseInsensitive, nsIMdbRow **aCardRow)
{
  NS_ENSURE_ARG_POINTER(aName);
  NS_ENSURE_ARG_POINTER(aUTF8Value);
  NS_ENSURE_ARG_POINTER(aCardRow);
  if (!m_mdbStore || !m_mdbEnv)
    return NS_ERROR_NULL_POINTER;
  
  mdb_token token;
  m_mdbStore->StringToToken(m_mdbEnv, aName, &token);
  
    NS_ConvertUTF8toUTF16 newUnicodeString(aUTF8Value);

  if (aCaseInsensitive)
    ToLowerCase(newUnicodeString);
  
  return GetRowForCharColumn(newUnicodeString.get(), token, PR_TRUE, aCardRow);
}

NS_IMETHODIMP nsAddrDatabase::GetCardFromAttribute(nsIAbDirectory *aDirectory, const char *aName, const char *aUTF8Value, PRBool aCaseInsensitive, nsIAbCard **aCardResult)
{
  NS_ENSURE_ARG_POINTER(aCardResult);
  
  m_dbDirectory = aDirectory;
  nsCOMPtr <nsIMdbRow> cardRow;
  nsresult rv = GetRowFromAttribute(aName, aUTF8Value, aCaseInsensitive, getter_AddRefs(cardRow));
  if (NS_SUCCEEDED(rv) && cardRow)
    rv = CreateABCard(cardRow, 0, aCardResult);
  else 
  {
    *aCardResult = nsnull;
    rv = NS_OK;
  }
  return rv;
}

NS_IMETHODIMP nsAddrDatabase::GetDirectoryName(PRUnichar **name)
{
    if (m_dbDirectory && name)
    {
        return m_dbDirectory->GetDirName(name);
    }
    else
        return NS_ERROR_FAILURE;
}

NS_IMETHODIMP nsAddrDatabase::AddListDirNode(nsIMdbRow * listRow)
{
    nsresult rv = NS_OK;

    static NS_DEFINE_CID(kRDFServiceCID, NS_RDFSERVICE_CID);
    NS_WITH_PROXIED_SERVICE(nsIRDFService, rdfService, kRDFServiceCID,
                            NS_PROXY_TO_MAIN_THREAD, &rv);
    if (NS_SUCCEEDED(rv)) 
    {
        nsCOMPtr<nsIRDFResource>    parentResource;

        nsAutoString parentURI;
        rv = m_dbName->GetLeafName(parentURI);
        NS_ENSURE_SUCCESS(rv, rv);

        parentURI = NS_LITERAL_STRING(kMDBDirectoryRoot) + parentURI;

        rv = rdfService->GetResource(NS_ConvertUTF16toUTF8(parentURI), getter_AddRefs(parentResource));
        nsCOMPtr<nsIAbDirectory> parentDir;
        rv = NS_GetProxyForObject( NS_PROXY_TO_MAIN_THREAD,
                                   NS_GET_IID( nsIAbDirectory),
                                   parentResource,
                                   NS_PROXY_SYNC | NS_PROXY_ALWAYS,
                                   getter_AddRefs( parentDir));
        if (parentDir)
        {
            m_dbDirectory = parentDir;
            nsCOMPtr<nsIAbDirectory> mailList;
            rv = CreateABList(listRow, getter_AddRefs(mailList));
            if (mailList)
            {
                nsCOMPtr<nsIAbMDBDirectory> dbparentDir(do_QueryInterface(parentDir, &rv));
                if(NS_SUCCEEDED(rv))
                    dbparentDir->NotifyDirItemAdded(mailList);
            }
        }
    }
    return rv;
}

NS_IMETHODIMP nsAddrDatabase::FindMailListbyUnicodeName(const PRUnichar *listName, PRBool *exist)
{
  nsAutoString unicodeString(listName);
  ToLowerCase(unicodeString);
  
  nsCOMPtr <nsIMdbRow> listRow;
  nsresult rv = GetRowForCharColumn(unicodeString.get(), m_LowerListNameColumnToken, PR_FALSE, getter_AddRefs(listRow));
  *exist = (NS_SUCCEEDED(rv) && listRow);
  return rv;
}

NS_IMETHODIMP nsAddrDatabase::GetCardCount(PRUint32 *count)
{    
    mdb_err rv;
    mdb_count c;
    rv = m_mdbPabTable->GetCount(m_mdbEnv, &c);
    if (rv == NS_OK)
        *count = c - 1;  // Don't count LastRecordKey 

    return rv;
}

PRBool 
nsAddrDatabase::HasRowButDeletedForCharColumn(const PRUnichar *unicodeStr, mdb_column findColumn, PRBool aIsCard, nsIMdbRow **aFindRow)
{
  if (!m_mdbStore || !aFindRow || !m_mdbEnv)
    return NS_ERROR_NULL_POINTER;

  mdbYarn    sourceYarn;
  
  NS_ConvertUTF16toUTF8 UTF8String(unicodeStr);
  sourceYarn.mYarn_Buf = (void *) UTF8String.get();
  sourceYarn.mYarn_Fill = UTF8String.Length();
  sourceYarn.mYarn_Form = 0;
  sourceYarn.mYarn_Size = sourceYarn.mYarn_Fill;
  
  mdbOid        outRowId;
  nsresult rv;
  
  if (aIsCard)
  {
    rv = m_mdbStore->FindRow(m_mdbEnv, m_CardRowScopeToken,
      findColumn, &sourceYarn,  &outRowId, aFindRow);
    
    // no such card, so bail out early
    if (NS_SUCCEEDED(rv) && !*aFindRow)
      return PR_FALSE;
    
    // we might not have loaded the "delete cards" table yet
    // so do that (but don't create it, if we don't have one), 
    // so we can see if the row is really a delete card.
    if (!m_mdbDeletedCardsTable)
      rv = InitDeletedCardsTable(PR_FALSE);
    
    // if still no deleted cards table, there are no deleted cards
    if (!m_mdbDeletedCardsTable)
      return PR_FALSE;
    
    mdb_bool hasRow = PR_FALSE;
    rv = m_mdbDeletedCardsTable->HasRow(m_mdbEnv, *aFindRow, &hasRow);
    return (NS_SUCCEEDED(rv) && hasRow);
  }
  
  rv = m_mdbStore->FindRow(m_mdbEnv, m_ListRowScopeToken,
    findColumn, &sourceYarn,  &outRowId, aFindRow);
  return (NS_SUCCEEDED(rv) && *aFindRow);
}

nsresult 
nsAddrDatabase::GetRowForCharColumn(const PRUnichar *unicodeStr, mdb_column findColumn, PRBool aIsCard, nsIMdbRow **aFindRow)
{
  NS_ENSURE_ARG_POINTER(unicodeStr);
  NS_ENSURE_ARG_POINTER(aFindRow);
  if (!m_mdbEnv)
    return NS_ERROR_NULL_POINTER;

  *aFindRow = nsnull;

  // see bug #198303
  // the addition of the m_mdbDeletedCardsTable table has complicated life in the addressbook
  // (it was added for palm sync).  until we fix the underlying problem, we have to jump through hoops
  // in order to know if we have a row (think card) for a given column value (think email=foo@bar.com)
  // there are 4 scenarios:
  //   1) no cards with a match
  //   2) at least one deleted card with a match, but no non-deleted cards
  //   3) at least one non-deleted card with a match, but no deleted cards
  //   4) at least one deleted card, and one non-deleted card with a match.
  // 
  // if we have no cards that match (FindRow() returns nothing), we can bail early
  // but if FindRow() returns something, we have to check if it is in the deleted table
  // if not in the deleted table we can return the row (we found a non-deleted card)
  // but if so, we have to search through the table of non-deleted cards
  // for a match.  If we find one, we return it.  but if not, we report that there are no
  // non-deleted cards.  This is the expensive part.  The worse case scenario is to have
  // deleted lots of cards, and then have a lot of non-deleted cards.
  // we'd have to call FindRow(), HasRow(), and then search the list of non-deleted cards
  // each time we call GetRowForCharColumn().
  if (!HasRowButDeletedForCharColumn(unicodeStr, findColumn, aIsCard, aFindRow))
  {
    // if we have a row, it's the row for the non-delete card, so return NS_OK.
    // otherwise, there is no such card (deleted or not), so return NS_ERROR_FAILURE
    return *aFindRow ? NS_OK : NS_ERROR_FAILURE;
  }

  // check if there is a non-deleted card
  nsCOMPtr<nsIMdbTableRowCursor> rowCursor;
  mdb_pos rowPos;
  PRBool done = PR_FALSE;
  nsCOMPtr<nsIMdbRow> currentRow;
  nsAutoString columnValue;
  
  mdb_scope targetScope = aIsCard ? m_CardRowScopeToken : m_ListRowScopeToken;
  
  m_mdbPabTable->GetTableRowCursor(m_mdbEnv, -1, getter_AddRefs(rowCursor));
  if (!rowCursor)
    return NS_ERROR_FAILURE;
  
  while (!done)
  {
    nsresult rv = rowCursor->NextRow(m_mdbEnv, getter_AddRefs(currentRow), &rowPos);
    if (currentRow && NS_SUCCEEDED(rv))
    {
      mdbOid rowOid;
      if ((currentRow->GetOid(m_mdbEnv, &rowOid) == NS_OK) && (rowOid.mOid_Scope == targetScope))
      {
        rv = GetStringColumn(currentRow, findColumn, columnValue);
        if (NS_SUCCEEDED(rv) && columnValue.Equals(unicodeStr))
        {
          NS_IF_ADDREF(*aFindRow = currentRow);
          return NS_OK;
      }
    }
  }
  else
      done = PR_TRUE;
}
  return NS_ERROR_FAILURE;
}

nsresult nsAddrDatabase::DeleteRow(nsIMdbTable* dbTable, nsIMdbRow* dbRow)
{
  if (!m_mdbEnv)
    return NS_ERROR_NULL_POINTER;

  mdb_err err = dbRow->CutAllColumns(m_mdbEnv);
  err = dbTable->CutRow(m_mdbEnv, dbRow);

  return (err == NS_OK) ? NS_OK : NS_ERROR_FAILURE;
}
