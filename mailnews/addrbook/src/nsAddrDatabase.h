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
 *   Seth Spitzer <sspitzer@netscape.com>
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

#ifndef _nsAddrDatabase_H_
#define _nsAddrDatabase_H_

#include "nsIAddrDatabase.h"
#include "mdb.h"
#include "nsVoidArray.h"
#include "nsString.h"
#include "nsIAddrDBListener.h"
#include "nsISupportsArray.h"
#include "nsCOMPtr.h"

typedef enum 
{
	AB_NotifyInserted,
	AB_NotifyDeleted,
	AB_NotifyPropertyChanged
} AB_NOTIFY_CODE;

class nsAddrDatabase : public nsIAddrDatabase 
{
public:
	NS_DECL_ISUPPORTS
    NS_DECL_NSIADDRDBANNOUNCER
	//////////////////////////////////////////////////////////////////////////////
	// nsIAddrDatabase methods:

  NS_IMETHOD GetDbPath(nsIFile * *aDbPath);
  NS_IMETHOD SetDbPath(nsIFile * aDbPath);
	NS_IMETHOD Open(nsIFile *aMabFile, PRBool aCreate, PRBool upgrading, nsIAddrDatabase **pCardDB);
	NS_IMETHOD Close(PRBool forceCommit);
  NS_IMETHOD OpenMDB(nsIFile *dbName, PRBool create);
	NS_IMETHOD CloseMDB(PRBool commit);
	NS_IMETHOD Commit(PRUint32 commitType);
	NS_IMETHOD ForceClosed();

	NS_IMETHOD CreateNewCardAndAddToDB(nsIAbCard *newCard, PRBool notify);
	NS_IMETHOD CreateNewListCardAndAddToDB(nsIAbDirectory *list, PRUint32 listRowID, nsIAbCard *newCard, PRBool notify);
	NS_IMETHOD CreateMailListAndAddToDB(nsIAbDirectory *newList, PRBool notify);
	NS_IMETHOD EnumerateCards(nsIAbDirectory *directory, nsISimpleEnumerator **result);
	NS_IMETHOD GetMailingListsFromDB(nsIAbDirectory *parentDir);
	NS_IMETHOD EnumerateListAddresses(nsIAbDirectory *directory, nsISimpleEnumerator **result);
	NS_IMETHOD DeleteCard(nsIAbCard *newCard, PRBool notify);
	NS_IMETHOD EditCard(nsIAbCard *card, PRBool notify);
	NS_IMETHOD ContainsCard(nsIAbCard *card, PRBool *hasCard);
	NS_IMETHOD DeleteMailList(nsIAbDirectory *mailList, PRBool notify);
	NS_IMETHOD EditMailList(nsIAbDirectory *mailList, nsIAbCard *listCard, PRBool notify);
	NS_IMETHOD ContainsMailList(nsIAbDirectory *mailList, PRBool *hasCard);
	NS_IMETHOD DeleteCardFromMailList(nsIAbDirectory *mailList, nsIAbCard *card, PRBool aNotify);
  NS_IMETHOD GetCardFromAttribute(nsIAbDirectory *directory, const char *aName, const char *aValue, PRBool aCaseInsensitive, nsIAbCard **card);	
	NS_IMETHOD GetNewRow(nsIMdbRow * *newRow); 
	NS_IMETHOD GetNewListRow(nsIMdbRow * *newRow); 
	NS_IMETHOD AddCardRowToDB(nsIMdbRow *newRow);
	NS_IMETHOD AddLdifListMember(nsIMdbRow* row, const char * value);

  NS_IMETHOD GetDeletedCardList(PRUint32 *aCount, nsISupportsArray **aDeletedList);
  NS_IMETHOD GetDeletedCardCount(PRUint32 *count);
  NS_IMETHOD PurgeDeletedCardTable();

	NS_IMETHOD AddFirstName(nsIMdbRow * row, const char * value)
	{ return AddCharStringColumn(row, m_FirstNameColumnToken, value); }

	NS_IMETHOD AddLastName(nsIMdbRow * row, const char * value)
	{ return AddCharStringColumn(row, m_LastNameColumnToken, value); }

	NS_IMETHOD AddPhoneticFirstName(nsIMdbRow * row, const char * value)
	{ return AddCharStringColumn(row, m_PhoneticFirstNameColumnToken, value); }

	NS_IMETHOD AddPhoneticLastName(nsIMdbRow * row, const char * value)
	{ return AddCharStringColumn(row, m_PhoneticLastNameColumnToken, value); }

	NS_IMETHOD AddDisplayName(nsIMdbRow * row, const char * value)
	{ return AddCharStringColumn(row, m_DisplayNameColumnToken, value); }

	NS_IMETHOD AddNickName(nsIMdbRow * row, const char * value)
	{ return AddCharStringColumn(row, m_NickNameColumnToken, value); }

	NS_IMETHOD AddPrimaryEmail(nsIMdbRow * row, const char * value);

	NS_IMETHOD Add2ndEmail(nsIMdbRow * row, const char * value)
	{ return AddCharStringColumn(row, m_2ndEmailColumnToken, value); }

	NS_IMETHOD AddPreferMailFormat(nsIMdbRow * row, PRUint32 value)
	{ return AddIntColumn(row, m_MailFormatColumnToken, value); }

	NS_IMETHOD AddPopularityIndex(nsIMdbRow * row, PRUint32 value)
	{ return AddIntColumn(row, m_PopularityIndexColumnToken, value); }

	NS_IMETHOD AddWorkPhone(nsIMdbRow * row, const char * value)
	{ return AddCharStringColumn(row, m_WorkPhoneColumnToken, value); }

	NS_IMETHOD AddHomePhone(nsIMdbRow * row, const char * value)
	{ return AddCharStringColumn(row, m_HomePhoneColumnToken, value); }

	NS_IMETHOD AddFaxNumber(nsIMdbRow * row, const char * value)
	{ return AddCharStringColumn(row, m_FaxColumnToken, value); }

	NS_IMETHOD AddPagerNumber(nsIMdbRow * row, const char * value)
	{ return AddCharStringColumn(row, m_PagerColumnToken, value); }

	NS_IMETHOD AddCellularNumber(nsIMdbRow * row, const char * value)
	{ return AddCharStringColumn(row, m_CellularColumnToken, value); }

  NS_IMETHOD AddWorkPhoneType(nsIMdbRow * row, const char * value)
  { return AddCharStringColumn(row, m_WorkPhoneTypeColumnToken, value); }

  NS_IMETHOD AddHomePhoneType(nsIMdbRow * row, const char * value)
  { return AddCharStringColumn(row, m_HomePhoneTypeColumnToken, value); }

  NS_IMETHOD AddFaxNumberType(nsIMdbRow * row, const char * value)
  { return AddCharStringColumn(row, m_FaxTypeColumnToken, value); }

  NS_IMETHOD AddPagerNumberType(nsIMdbRow * row, const char * value)
  { return AddCharStringColumn(row, m_PagerTypeColumnToken, value); }

  NS_IMETHOD AddCellularNumberType(nsIMdbRow * row, const char * value)
  { return AddCharStringColumn(row, m_CellularTypeColumnToken, value); }

	NS_IMETHOD AddHomeAddress(nsIMdbRow * row, const char * value)
	{ return AddCharStringColumn(row, m_HomeAddressColumnToken, value); }

	NS_IMETHOD AddHomeAddress2(nsIMdbRow * row, const char * value)
	{ return AddCharStringColumn(row, m_HomeAddress2ColumnToken, value); }

	NS_IMETHOD AddHomeCity(nsIMdbRow * row, const char * value)
	{ return AddCharStringColumn(row, m_HomeCityColumnToken, value); }

	NS_IMETHOD AddHomeState(nsIMdbRow * row, const char * value)
	{ return AddCharStringColumn(row, m_HomeStateColumnToken, value); }

	NS_IMETHOD AddHomeZipCode(nsIMdbRow * row, const char * value)
	{ return AddCharStringColumn(row, m_HomeZipCodeColumnToken, value); }

	NS_IMETHOD AddHomeCountry(nsIMdbRow * row, const char * value)
	{ return AddCharStringColumn(row, m_HomeCountryColumnToken, value); }

	NS_IMETHOD AddWorkAddress(nsIMdbRow * row, const char * value)
	{ return AddCharStringColumn(row, m_WorkAddressColumnToken, value); }

	NS_IMETHOD AddWorkAddress2(nsIMdbRow * row, const char * value)
	{ return AddCharStringColumn(row, m_WorkAddress2ColumnToken, value); }

	NS_IMETHOD AddWorkCity(nsIMdbRow * row, const char * value) 
	{ return AddCharStringColumn(row, m_WorkCityColumnToken, value); }

	NS_IMETHOD AddWorkState(nsIMdbRow * row, const char * value)
	{ return AddCharStringColumn(row, m_WorkStateColumnToken, value); }

	NS_IMETHOD AddWorkZipCode(nsIMdbRow * row, const char * value)
	{ return AddCharStringColumn(row, m_WorkZipCodeColumnToken, value); }

	NS_IMETHOD AddWorkCountry(nsIMdbRow * row, const char * value)
	{ return AddCharStringColumn(row, m_WorkCountryColumnToken, value); }

	NS_IMETHOD AddJobTitle(nsIMdbRow * row, const char * value)
	{ return AddCharStringColumn(row, m_JobTitleColumnToken, value); }

	NS_IMETHOD AddDepartment(nsIMdbRow * row, const char * value)
	{ return AddCharStringColumn(row, m_DepartmentColumnToken, value); }

	NS_IMETHOD AddCompany(nsIMdbRow * row, const char * value)
	{ return AddCharStringColumn(row, m_CompanyColumnToken, value); }

  NS_IMETHOD AddAimScreenName(nsIMdbRow * row, const char * value)
  { return AddCharStringColumn(row, m_AimScreenNameColumnToken, value); }

  NS_IMETHOD AddAnniversaryYear(nsIMdbRow * row, const char * value)
  { return AddCharStringColumn(row, m_AnniversaryYearColumnToken, value); }

  NS_IMETHOD AddAnniversaryMonth(nsIMdbRow * row, const char * value)
  { return AddCharStringColumn(row, m_AnniversaryMonthColumnToken, value); }

  NS_IMETHOD AddAnniversaryDay(nsIMdbRow * row, const char * value)
  { return AddCharStringColumn(row, m_AnniversaryDayColumnToken, value); }

  NS_IMETHOD AddSpouseName(nsIMdbRow * row, const char * value)
  { return AddCharStringColumn(row, m_SpouseNameColumnToken, value); }

  NS_IMETHOD AddFamilyName(nsIMdbRow * row, const char * value)
  { return AddCharStringColumn(row, m_FamilyNameColumnToken, value); }

  NS_IMETHOD AddDefaultAddress(nsIMdbRow * row, const char * value)
  { return AddCharStringColumn(row, m_DefaultAddressColumnToken, value); }

  NS_IMETHOD AddCategory(nsIMdbRow * row, const char * value)
  { return AddCharStringColumn(row, m_CategoryColumnToken, value); }

	NS_IMETHOD AddWebPage1(nsIMdbRow * row, const char * value)
	{ return AddCharStringColumn(row, m_WebPage1ColumnToken, value); }

	NS_IMETHOD AddWebPage2(nsIMdbRow * row, const char * value)
	{ return AddCharStringColumn(row, m_WebPage2ColumnToken, value); }

	NS_IMETHOD AddBirthYear(nsIMdbRow * row, const char * value)
	{ return AddCharStringColumn(row, m_BirthYearColumnToken, value); }

	NS_IMETHOD AddBirthMonth(nsIMdbRow * row, const char * value)
	{ return AddCharStringColumn(row, m_BirthMonthColumnToken, value); }

	NS_IMETHOD AddBirthDay(nsIMdbRow * row, const char * value)
	{ return AddCharStringColumn(row, m_BirthDayColumnToken, value); }

	NS_IMETHOD AddCustom1(nsIMdbRow * row, const char * value)
	{ return AddCharStringColumn(row, m_Custom1ColumnToken, value); }

	NS_IMETHOD AddCustom2(nsIMdbRow * row, const char * value)
	{ return AddCharStringColumn(row, m_Custom2ColumnToken, value); }

	NS_IMETHOD AddCustom3(nsIMdbRow * row, const char * value)
	{ return AddCharStringColumn(row, m_Custom3ColumnToken, value); }

	NS_IMETHOD AddCustom4(nsIMdbRow * row, const char * value)
	{ return AddCharStringColumn(row, m_Custom4ColumnToken, value); }

	NS_IMETHOD AddNotes(nsIMdbRow * row, const char * value)
	{ return AddCharStringColumn(row, m_NotesColumnToken, value); }

	NS_IMETHOD AddListName(nsIMdbRow * row, const char * value);

	NS_IMETHOD AddListNickName(nsIMdbRow * row, const char * value)
	{ return AddCharStringColumn(row, m_ListNickNameColumnToken, value); }

	NS_IMETHOD AddListDescription(nsIMdbRow * row, const char * value)
	{ return AddCharStringColumn(row, m_ListDescriptionColumnToken, value); }


	NS_IMETHOD AddListDirNode(nsIMdbRow * listRow);
	NS_IMETHOD GetDirectoryName(PRUnichar **name);

	NS_IMETHOD FindMailListbyUnicodeName(const PRUnichar *listName, PRBool *exist);

	NS_IMETHOD GetCardCount(PRUint32 *count);

  NS_IMETHOD SetCardValue(nsIAbCard *card, const char *name, const PRUnichar *value, PRBool notify);
  NS_IMETHOD GetCardValue(nsIAbCard *card, const char *name, PRUnichar **value);
	// nsAddrDatabase methods:

	nsAddrDatabase();
	virtual ~nsAddrDatabase();

	nsIMdbFactory	*GetMDBFactory();
	nsIMdbEnv		*GetEnv() {return m_mdbEnv;}
	PRUint32		GetCurVersion();
	nsIMdbTableRowCursor *GetTableRowCursor();
	nsIMdbTable		*GetPabTable() {return m_mdbPabTable;}

  static nsAddrDatabase* FindInCache(nsIFile *dbName);

	static void		CleanupCache();

	nsresult CreateABCard(nsIMdbRow* cardRow, mdb_id listRowID, nsIAbCard **result);
	nsresult CreateABListCard(nsIMdbRow* listRow, nsIAbCard **result);
	nsresult CreateABList(nsIMdbRow* listRow, nsIAbDirectory **result);

	PRBool IsListRowScopeToken(mdb_scope scope) { return (scope == m_ListRowScopeToken) ? PR_TRUE: PR_FALSE; }
	PRBool IsCardRowScopeToken(mdb_scope scope) { return (scope == m_CardRowScopeToken) ? PR_TRUE: PR_FALSE;  }
	PRBool IsDataRowScopeToken(mdb_scope scope) { return (scope == m_DataRowScopeToken) ? PR_TRUE: PR_FALSE; }
	nsresult GetCardRowByRowID(mdb_id rowID, nsIMdbRow **dbRow);
	nsresult GetListRowByRowID(mdb_id rowID, nsIMdbRow **dbRow);

	PRUint32 GetListAddressTotal(nsIMdbRow* listRow);
	nsresult GetAddressRowByPos(nsIMdbRow* listRow, PRUint16 pos, nsIMdbRow** cardRow);

    NS_IMETHOD AddListCardColumnsToRow(nsIAbCard *aPCard, nsIMdbRow *aPListRow, PRUint32 aPos, nsIAbCard** aPNewCard, PRBool aInMailingList);
    NS_IMETHOD InitCardFromRow(nsIAbCard *aNewCard, nsIMdbRow* aCardRow);
    NS_IMETHOD SetListAddressTotal(nsIMdbRow* aListRow, PRUint32 aTotal);
    NS_IMETHOD FindRowByCard(nsIAbCard * card,nsIMdbRow **aRow);

protected:
  
  static void		AddToCache(nsAddrDatabase* pAddrDB) {GetDBCache()->AppendElement(pAddrDB);}
	static void		RemoveFromCache(nsAddrDatabase* pAddrDB);
	static PRInt32	FindInCache(nsAddrDatabase* pAddrDB);
  PRBool MatchDbName(nsIFile *dbName); // returns TRUE if they match

	void YarnToUInt32(struct mdbYarn *yarn, PRUint32 *pResult);
	void GetCharStringYarn(char* str, struct mdbYarn* strYarn);
	void GetStringYarn(const nsAString & aStr, struct mdbYarn* strYarn);
	void GetIntYarn(PRUint32 nValue, struct mdbYarn* intYarn);
	nsresult AddCharStringColumn(nsIMdbRow* cardRow, mdb_column inColumn, const char* str);
	nsresult AddStringColumn(nsIMdbRow* aCardRow, mdb_column aInColumn, const nsAString & aStr);
	nsresult AddIntColumn(nsIMdbRow* cardRow, mdb_column inColumn, PRUint32 nValue);
	nsresult AddBoolColumn(nsIMdbRow* cardRow, mdb_column inColumn, PRBool bValue);
	nsresult GetStringColumn(nsIMdbRow *cardRow, mdb_token outToken, nsString& str);
	nsresult GetIntColumn(nsIMdbRow *cardRow, mdb_token outToken, 
							PRUint32* pValue, PRUint32 defaultValue);
	nsresult GetBoolColumn(nsIMdbRow *cardRow, mdb_token outToken, PRBool* pValue);
	nsresult GetListCardFromDB(nsIAbCard *listCard, nsIMdbRow* listRow);
	nsresult GetListFromDB(nsIAbDirectory *newCard, nsIMdbRow* listRow);
	nsresult AddRecordKeyColumnToRow(nsIMdbRow *pRow);
	nsresult AddAttributeColumnsToRow(nsIAbCard *card, nsIMdbRow *cardRow);
	nsresult AddListAttributeColumnsToRow(nsIAbDirectory *list, nsIMdbRow *listRow);
	nsresult CreateCard(nsIMdbRow* cardRow, mdb_id listRowID, nsIAbCard **result);
	nsresult CreateCardFromDeletedCardsTable(nsIMdbRow* cardRow, mdb_id listRowID, nsIAbCard **result);
	nsresult DeleteCardFromListRow(nsIMdbRow* pListRow, mdb_id cardRowID);
	void DeleteCardFromAllMailLists(mdb_id cardRowID);
	nsresult NotifyListEntryChange(PRUint32 abCode, nsIAbDirectory *dir);

	nsresult AddLowercaseColumn(nsIMdbRow * row, mdb_token columnToken, const char* utf8String);
  nsresult GetRowFromAttribute(const char *aName, const char *aUTF8Value, PRBool aCaseInsensitive, nsIMdbRow	**aCardRow);

	static nsVoidArray/*<nsAddrDatabase>*/ * GetDBCache();
	static nsVoidArray/*<nsAddrDatabase>*/ * m_dbCache;

	// mdb bookkeeping stuff
	nsresult			InitExistingDB();
	nsresult			InitNewDB();
	nsresult			InitMDBInfo();
	nsresult			InitPabTable();
  nsresult      InitDeletedCardsTable(PRBool aCreate);
  nsresult			AddRowToDeletedCardsTable(nsIAbCard *card, nsIMdbRow **pCardRow);
  nsresult			DeleteRowFromDeletedCardsTable(nsIMdbRow *pCardRow);

	nsresult			InitLastRecorKey();
	nsresult			GetDataRow(nsIMdbRow **pDataRow);
	nsresult			GetLastRecordKey();
	nsresult			UpdateLastRecordKey();
	nsresult			CheckAndUpdateRecordKey();
	nsresult			UpdateLowercaseEmailListName();
	nsresult			ConvertAndAddLowercaseColumn(nsIMdbRow * row, mdb_token fromCol, mdb_token toCol);
	nsresult			AddUnicodeToColumn(nsIMdbRow * row, mdb_token colToken, mdb_token lowerCaseColToken, const PRUnichar* pUnicodeStr);
	nsresult			CreateCardsForMailList(nsIMdbRow *pListRow, nsIEnumerator **result);

	nsresult			DeleteRow(nsIMdbTable* dbTable, nsIMdbRow* dbRow);

  nsIMdbEnv   *m_mdbEnv;	// to be used in all the db calls.
  nsIMdbStore *m_mdbStore;
  nsIMdbTable *m_mdbPabTable;
  nsIMdbTable *m_mdbDeletedCardsTable;
  nsCOMPtr<nsIFile> m_dbName;
	PRBool				m_mdbTokensInitialized;
    nsVoidArray /*<nsIAddrDBListener>*/ *m_ChangeListeners;

	mdb_kind			m_PabTableKind;
	mdb_kind			m_MailListTableKind;
	mdb_kind			m_DeletedCardsTableKind;

	mdb_scope			m_CardRowScopeToken;
	mdb_scope			m_ListRowScopeToken;
	mdb_scope			m_DataRowScopeToken;

	mdb_token			m_FirstNameColumnToken;
	mdb_token			m_LastNameColumnToken;
	mdb_token			m_PhoneticFirstNameColumnToken;
	mdb_token			m_PhoneticLastNameColumnToken;
	mdb_token			m_DisplayNameColumnToken;
	mdb_token			m_NickNameColumnToken;
	mdb_token			m_PriEmailColumnToken;
	mdb_token			m_2ndEmailColumnToken;
  mdb_token			m_DefaultEmailColumnToken;
  mdb_token			m_CardTypeColumnToken;
	mdb_token			m_WorkPhoneColumnToken;
	mdb_token			m_HomePhoneColumnToken;
	mdb_token			m_FaxColumnToken;
	mdb_token			m_PagerColumnToken;
	mdb_token			m_CellularColumnToken;
  mdb_token			m_WorkPhoneTypeColumnToken;
	mdb_token			m_HomePhoneTypeColumnToken;
	mdb_token			m_FaxTypeColumnToken;
	mdb_token			m_PagerTypeColumnToken;
	mdb_token			m_CellularTypeColumnToken;
	mdb_token			m_HomeAddressColumnToken;
	mdb_token			m_HomeAddress2ColumnToken;
	mdb_token			m_HomeCityColumnToken;
	mdb_token			m_HomeStateColumnToken;
	mdb_token			m_HomeZipCodeColumnToken;
	mdb_token			m_HomeCountryColumnToken;
	mdb_token			m_WorkAddressColumnToken;
	mdb_token			m_WorkAddress2ColumnToken;
	mdb_token			m_WorkCityColumnToken;
	mdb_token			m_WorkStateColumnToken;
	mdb_token			m_WorkZipCodeColumnToken;
	mdb_token			m_WorkCountryColumnToken;
	mdb_token			m_JobTitleColumnToken;
	mdb_token			m_DepartmentColumnToken;
	mdb_token			m_CompanyColumnToken;
  mdb_token			m_AimScreenNameColumnToken;
  mdb_token			m_AnniversaryYearColumnToken;
  mdb_token			m_AnniversaryMonthColumnToken;
  mdb_token			m_AnniversaryDayColumnToken;
  mdb_token			m_SpouseNameColumnToken;
  mdb_token			m_FamilyNameColumnToken;
  mdb_token			m_DefaultAddressColumnToken;
  mdb_token			m_CategoryColumnToken;
	mdb_token			m_WebPage1ColumnToken;
	mdb_token			m_WebPage2ColumnToken;
	mdb_token			m_BirthYearColumnToken;
	mdb_token			m_BirthMonthColumnToken;
	mdb_token			m_BirthDayColumnToken;
	mdb_token			m_Custom1ColumnToken;
	mdb_token			m_Custom2ColumnToken;
	mdb_token			m_Custom3ColumnToken;
	mdb_token			m_Custom4ColumnToken;
	mdb_token			m_NotesColumnToken;
	mdb_token			m_LastModDateColumnToken;
	mdb_token			m_RecordKeyColumnToken;
	mdb_token			m_LowerPriEmailColumnToken;

	mdb_token			m_MailFormatColumnToken;
	mdb_token     m_PopularityIndexColumnToken;
						
	mdb_token			m_AddressCharSetColumnToken;
	mdb_token			m_LastRecordKeyColumnToken;

	mdb_token			m_ListNameColumnToken;
	mdb_token			m_ListNickNameColumnToken;
	mdb_token			m_ListDescriptionColumnToken;
	mdb_token			m_ListTotalColumnToken;
	mdb_token			m_LowerListNameColumnToken;

	PRUint32			m_LastRecordKey;
	nsIAbDirectory*		m_dbDirectory;

private:
  nsresult GetRowForCharColumn(const PRUnichar *unicodeStr, mdb_column findColumn, PRBool bIsCard, nsIMdbRow **findRow);
  PRBool HasRowButDeletedForCharColumn(const PRUnichar *unicodeStr, mdb_column findColumn, PRBool aIsCard, nsIMdbRow **aFindRow);
  nsresult OpenInternal(nsIFile *aMabFile, PRBool aCreate, nsIAddrDatabase **pCardDB);
  nsresult AlertAboutCorruptMabFile(const PRUnichar *aOldFileName, const PRUnichar *aNewFileName);
  nsresult AlertAboutLockedMabFile(const PRUnichar *aFileName);
  nsresult DisplayAlert(const PRUnichar *titleName, const PRUnichar *alertStringName, 
                        const PRUnichar **formatStrings, PRInt32 numFormatStrings);
};

#endif
