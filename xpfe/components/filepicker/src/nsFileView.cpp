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
 * Portions created by the Initial Developer are Copyright (C) 2001
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *  Brian Ryner <bryner@brianryner.com>
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

#include "nsIFileView.h"
#include "nsITreeView.h"
#include "nsIGenericFactory.h"
#include "nsITreeSelection.h"
#include "nsITreeColumns.h"
#include "nsITreeBoxObject.h"
#include "nsILocalFile.h"
#include "nsString.h"
#include "nsReadableUtils.h"
#include "nsCRT.h"
#include "prmem.h"
#include "nsPrintfCString.h"
#include "nsVoidArray.h"
#include "nsIDateTimeFormat.h"
#include "nsDateTimeFormatCID.h"
#include "nsQuickSort.h"
#include "nsIAtom.h"
#include "nsIAutoCompleteResult.h"
#include "nsIAutoCompleteSearch.h"
#include "nsISimpleEnumerator.h"
#include "nsAutoPtr.h"
#include "nsIMutableArray.h"

#include "nsWildCard.h"

#define NS_FILECOMPLETE_CID { 0xcb60980e, 0x18a5, 0x4a77, \
                            { 0x91, 0x10, 0x81, 0x46, 0x61, 0x4c, 0xa7, 0xf0 } }
#define NS_FILECOMPLETE_CONTRACTID "@mozilla.org/autocomplete/search;1?name=file"

class nsFileResult : public nsIAutoCompleteResult
{
public:
  // aSearchString is the text typed into the autocomplete widget
  // aSearchParam is the picker's currently displayed directory
  nsFileResult(const nsAString& aSearchString, const nsAString& aSearchParam);

  NS_DECL_ISUPPORTS
  NS_DECL_NSIAUTOCOMPLETERESULT

  nsStringArray mValues;
  nsAutoString mSearchString;
  PRUint16 mSearchResult;
};

NS_IMPL_ISUPPORTS1(nsFileResult, nsIAutoCompleteResult)

nsFileResult::nsFileResult(const nsAString& aSearchString,
                           const nsAString& aSearchParam):
  mSearchString(aSearchString)
{
  if (aSearchString.IsEmpty())
    mSearchResult = RESULT_IGNORED;
  else {
    PRInt32 slashPos = mSearchString.RFindChar('/');
    mSearchResult = RESULT_FAILURE;
    nsCOMPtr<nsILocalFile> directory;
    nsDependentSubstring parent(Substring(mSearchString, 0, slashPos + 1));
    if (!parent.IsEmpty() && parent.First() == '/')
      NS_NewLocalFile(parent, PR_TRUE, getter_AddRefs(directory));
    if (!directory) {
      if (NS_FAILED(NS_NewLocalFile(aSearchParam, PR_TRUE, getter_AddRefs(directory))))
        return;
      if (slashPos > 0)
        directory->AppendRelativePath(Substring(mSearchString, 0, slashPos));
    }
    nsCOMPtr<nsISimpleEnumerator> dirEntries;
    if (NS_FAILED(directory->GetDirectoryEntries(getter_AddRefs(dirEntries))))
      return;
    mSearchResult = RESULT_NOMATCH;
    PRBool hasMore = PR_FALSE;
    nsDependentSubstring prefix(Substring(mSearchString, slashPos + 1));
    while (NS_SUCCEEDED(dirEntries->HasMoreElements(&hasMore)) && hasMore) {
      nsCOMPtr<nsISupports> nextItem;
      dirEntries->GetNext(getter_AddRefs(nextItem));
      nsCOMPtr<nsILocalFile> nextFile(do_QueryInterface(nextItem));
      nsAutoString fileName;
      nextFile->GetLeafName(fileName);
      if (StringBeginsWith(fileName, prefix)) {
        fileName.Insert(parent, 0);
        mValues.AppendString(fileName);
        if (mSearchResult == RESULT_NOMATCH && fileName.Equals(mSearchString))
          mSearchResult = RESULT_IGNORED;
        else
          mSearchResult = RESULT_SUCCESS;
      }
    }
    mValues.Sort();
  }
}

NS_IMETHODIMP nsFileResult::GetSearchString(nsAString & aSearchString)
{
  aSearchString.Assign(mSearchString);
  return NS_OK;
}

NS_IMETHODIMP nsFileResult::GetSearchResult(PRUint16 *aSearchResult)
{
  NS_ENSURE_ARG_POINTER(aSearchResult);
  *aSearchResult = mSearchResult;
  return NS_OK;
}

NS_IMETHODIMP nsFileResult::GetDefaultIndex(PRInt32 *aDefaultIndex)
{
  NS_ENSURE_ARG_POINTER(aDefaultIndex);
  *aDefaultIndex = -1;
  return NS_OK;
}

NS_IMETHODIMP nsFileResult::GetErrorDescription(nsAString & aErrorDescription)
{
  aErrorDescription.Truncate();
  return NS_OK;
}

NS_IMETHODIMP nsFileResult::GetMatchCount(PRUint32 *aMatchCount)
{
  NS_ENSURE_ARG_POINTER(aMatchCount);
  *aMatchCount = mValues.Count();
  return NS_OK;
}

NS_IMETHODIMP nsFileResult::GetValueAt(PRInt32 index, nsAString & aValue)
{
  mValues.StringAt(index, aValue);
  return NS_OK;
}

NS_IMETHODIMP nsFileResult::GetCommentAt(PRInt32 index, nsAString & aComment)
{
  aComment.Truncate();
  return NS_OK;
}

NS_IMETHODIMP nsFileResult::GetStyleAt(PRInt32 index, nsAString & aStyle)
{
  aStyle.Truncate();
  return NS_OK;
}

NS_IMETHODIMP nsFileResult::GetImageAt(PRInt32 index, nsAString & aImage)
{
  aImage.Truncate();
  return NS_OK;
}

NS_IMETHODIMP nsFileResult::RemoveValueAt(PRInt32 rowIndex, PRBool removeFromDb)
{
  return NS_OK;
}

class nsFileComplete : public nsIAutoCompleteSearch
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIAUTOCOMPLETESEARCH
};

NS_IMPL_ISUPPORTS1(nsFileComplete, nsIAutoCompleteSearch)

NS_IMETHODIMP
nsFileComplete::StartSearch(const nsAString& aSearchString,
                            const nsAString& aSearchParam,
                            nsIAutoCompleteResult *aPreviousResult,
                            nsIAutoCompleteObserver *aListener)
{
  NS_ENSURE_ARG_POINTER(aListener);
  nsRefPtr<nsFileResult> result = new nsFileResult(aSearchString, aSearchParam);
  NS_ENSURE_TRUE(result, NS_ERROR_OUT_OF_MEMORY);
  return aListener->OnSearchResult(this, result);
}

NS_IMETHODIMP
nsFileComplete::StopSearch()
{
  return NS_OK;
}

#define NS_FILEVIEW_CID { 0xa5570462, 0x1dd1, 0x11b2, \
                         { 0x9d, 0x19, 0xdf, 0x30, 0xa2, 0x7f, 0xbd, 0xc4 } }

class nsFileView : public nsIFileView,
                   public nsITreeView
{
public:
  nsFileView();
  nsresult Init();

  NS_DECL_ISUPPORTS
  NS_DECL_NSIFILEVIEW
  NS_DECL_NSITREEVIEW
  
protected:
  virtual ~nsFileView();
  
  void FilterFiles();
  void ReverseArray(nsISupportsArray* aArray);
  void SortArray(nsISupportsArray* aArray);
  void SortInternal();

  nsCOMPtr<nsISupportsArray> mFileList;
  nsCOMPtr<nsISupportsArray> mDirList;
  nsCOMPtr<nsISupportsArray> mFilteredFiles;

  nsCOMPtr<nsIFile> mDirectoryPath;
  nsCOMPtr<nsITreeBoxObject> mTree;
  nsCOMPtr<nsITreeSelection> mSelection;
  nsCOMPtr<nsIAtom> mDirectoryAtom;
  nsCOMPtr<nsIAtom> mFileAtom;
  nsCOMPtr<nsIDateTimeFormat> mDateFormatter;

  PRInt16 mSortType;
  PRInt32 mTotalRows;

  nsVoidArray mCurrentFilters;

  PRPackedBool mShowHiddenFiles;
  PRPackedBool mDirectoryFilter;
  PRPackedBool mReverseSort;
};

// Factory constructor
NS_GENERIC_FACTORY_CONSTRUCTOR(nsFileComplete)
NS_GENERIC_FACTORY_CONSTRUCTOR_INIT(nsFileView, Init)

static const nsModuleComponentInfo components[] =
{
  { "nsFileComplete", NS_FILECOMPLETE_CID,
    NS_FILECOMPLETE_CONTRACTID, nsFileCompleteConstructor },
  { "nsFileView", NS_FILEVIEW_CID,
    NS_FILEVIEW_CONTRACTID, nsFileViewConstructor }
};

NS_IMPL_NSGETMODULE(nsFileViewModule, components)


nsFileView::nsFileView() :
  mSortType(-1),
  mTotalRows(0),
  mShowHiddenFiles(PR_FALSE),
  mDirectoryFilter(PR_FALSE),
  mReverseSort(PR_FALSE)
{
}

nsFileView::~nsFileView()
{
  PRInt32 count = mCurrentFilters.Count();
  for (PRInt32 i = 0; i < count; ++i)
    NS_Free(mCurrentFilters[i]);
}

nsresult
nsFileView::Init()
{
  mDirectoryAtom = do_GetAtom("directory");
  mFileAtom = do_GetAtom("file");
  NS_NewISupportsArray(getter_AddRefs(mFileList));
  NS_NewISupportsArray(getter_AddRefs(mDirList));
  NS_NewISupportsArray(getter_AddRefs(mFilteredFiles));
  mDateFormatter = do_CreateInstance(NS_DATETIMEFORMAT_CONTRACTID);

  return NS_OK;
}

// nsISupports implementation

NS_IMPL_ISUPPORTS2(nsFileView, nsITreeView, nsIFileView)

// nsIFileView implementation

NS_IMETHODIMP
nsFileView::SetShowHiddenFiles(PRBool aShowHidden)
{
  if (aShowHidden != mShowHiddenFiles) {
    mShowHiddenFiles = aShowHidden;

    // This could be better optimized, but since the hidden
    // file functionality is not currently used, this will be fine.
    SetDirectory(mDirectoryPath);
  }
    
  return NS_OK;
}

NS_IMETHODIMP
nsFileView::GetShowHiddenFiles(PRBool* aShowHidden)
{
  *aShowHidden = mShowHiddenFiles;
  return NS_OK;
}

NS_IMETHODIMP
nsFileView::SetShowOnlyDirectories(PRBool aOnlyDirs)
{
  if (aOnlyDirs == mDirectoryFilter)
    return NS_OK;

  mDirectoryFilter = aOnlyDirs;
  PRUint32 dirCount;
  mDirList->Count(&dirCount);
  if (mDirectoryFilter) {
    PRInt32 rowDiff = mTotalRows - dirCount;

    mFilteredFiles->Clear();
    mTotalRows = dirCount;
    if (mTree)
      mTree->RowCountChanged(mTotalRows, -rowDiff);
  } else {
    // Run the filter again to get the file list back
    FilterFiles();

    SortArray(mFilteredFiles);
    if (mReverseSort)
      ReverseArray(mFilteredFiles);

    if (mTree)
      mTree->RowCountChanged(dirCount, mTotalRows - dirCount);
  }

  return NS_OK;
}

NS_IMETHODIMP
nsFileView::GetShowOnlyDirectories(PRBool* aOnlyDirs)
{
  *aOnlyDirs = mDirectoryFilter;
  return NS_OK;
}

NS_IMETHODIMP
nsFileView::GetSortType(PRInt16* aSortType)
{
  *aSortType = mSortType;
  return NS_OK;
}

NS_IMETHODIMP
nsFileView::GetReverseSort(PRBool* aReverseSort)
{
  *aReverseSort = mReverseSort;
  return NS_OK;
}

NS_IMETHODIMP
nsFileView::Sort(PRInt16 aSortType, PRBool aReverseSort)
{
  if (aSortType == mSortType) {
    if (aReverseSort != mReverseSort) {
      mReverseSort = aReverseSort;
      ReverseArray(mDirList);
      ReverseArray(mFilteredFiles);
    } else
      return NS_OK;
  } else {
    mSortType = aSortType;
    mReverseSort = aReverseSort;
    SortInternal();
  }

  if (mTree)
    mTree->Invalidate();

  return NS_OK;
}

NS_IMETHODIMP
nsFileView::SetDirectory(nsIFile* aDirectory)
{
  nsCOMPtr<nsISimpleEnumerator> dirEntries;
  aDirectory->GetDirectoryEntries(getter_AddRefs(dirEntries));

  if (!dirEntries) {
    // Couldn't read in the directory, this can happen if the user does not
    // have permission to list it.
    return NS_ERROR_FAILURE;
  }

  mDirectoryPath = aDirectory;
  mFileList->Clear();
  mDirList->Clear();

  PRBool hasMore = PR_FALSE;

  while (NS_SUCCEEDED(dirEntries->HasMoreElements(&hasMore)) && hasMore) {
    nsCOMPtr<nsISupports> nextItem;
    dirEntries->GetNext(getter_AddRefs(nextItem));
    nsCOMPtr<nsIFile> theFile = do_QueryInterface(nextItem);

    PRBool isDirectory;
    theFile->IsDirectory(&isDirectory);

    if (isDirectory) {
      PRBool isHidden;
      theFile->IsHidden(&isHidden);
      if (mShowHiddenFiles || !isHidden) {
        mDirList->AppendElement(theFile);
      }
    }
    else {
      mFileList->AppendElement(theFile);
    }
  }

  if (mTree) {
    mTree->BeginUpdateBatch();
    mTree->RowCountChanged(0, -mTotalRows);
  }

  FilterFiles();
  SortInternal();

  if (mTree) {
    mTree->EndUpdateBatch();
    mTree->ScrollToRow(0);
  }

  return NS_OK;
}

NS_IMETHODIMP
nsFileView::SetFilter(const nsAString& aFilterString)
{
  PRInt32 filterCount = mCurrentFilters.Count();
  for (PRInt32 i = 0; i < filterCount; ++i)
    NS_Free(mCurrentFilters[i]);
  mCurrentFilters.Clear();

  nsAString::const_iterator start, iter, end;
  aFilterString.BeginReading(iter);
  aFilterString.EndReading(end);

  while (PR_TRUE) {
    // skip over delimiters
    while (iter != end && (*iter == ';' || *iter == ' '))
      ++iter;

    if (iter == end)
      break;

    start = iter; // start of a filter

    // we know this is neither ';' nor ' ', skip to next char
    ++iter;

    // find next delimiter or end of string
    while (iter != end && (*iter != ';' && *iter != ' '))
      ++iter;

    PRUnichar* filter = ToNewUnicode(Substring(start, iter));
    if (!filter)
      return NS_ERROR_OUT_OF_MEMORY;

    if (!mCurrentFilters.AppendElement(filter)) {
      NS_Free(filter);
      return NS_ERROR_OUT_OF_MEMORY;
    }

    if (iter == end)
      break;

    ++iter; // we know this is either ';' or ' ', skip to next char
  }

  if (mTree) {
    mTree->BeginUpdateBatch();
    PRUint32 count;
    mDirList->Count(&count);
    mTree->RowCountChanged(count, count - mTotalRows);
  }

  mFilteredFiles->Clear();

  FilterFiles();

  SortArray(mFilteredFiles);
  if (mReverseSort)
    ReverseArray(mFilteredFiles);

  if (mTree)
    mTree->EndUpdateBatch();

  return NS_OK;
}

NS_IMETHODIMP
nsFileView::GetSelectedFiles(nsIArray** aFiles)
{
  *aFiles = nsnull;

  PRInt32 numRanges;
  mSelection->GetRangeCount(&numRanges);

  PRUint32 dirCount;
  mDirList->Count(&dirCount);

  nsCOMPtr<nsIMutableArray> fileArray =
    do_CreateInstance(NS_ARRAY_CONTRACTID);
  NS_ENSURE_STATE(fileArray);

  for (PRInt32 range = 0; range < numRanges; ++range) {
    PRInt32 rangeBegin, rangeEnd;
    mSelection->GetRangeAt(range, &rangeBegin, &rangeEnd);

    for (PRInt32 itemIndex = rangeBegin; itemIndex <= rangeEnd; ++itemIndex) {
      nsCOMPtr<nsIFile> curFile;

      if (itemIndex < (PRInt32) dirCount)
        curFile = do_QueryElementAt(mDirList, itemIndex);
      else {
        if (itemIndex < mTotalRows)
          curFile = do_QueryElementAt(mFilteredFiles, itemIndex - dirCount);
      }

      if (curFile)
        fileArray->AppendElement(curFile, PR_FALSE);
    }
  }

  NS_ADDREF(*aFiles = fileArray);
  return NS_OK;
}


// nsITreeView implementation

NS_IMETHODIMP
nsFileView::GetRowCount(PRInt32* aRowCount)
{
  *aRowCount = mTotalRows;
  return NS_OK;
}

NS_IMETHODIMP
nsFileView::GetSelection(nsITreeSelection** aSelection)
{
  *aSelection = mSelection;
  NS_IF_ADDREF(*aSelection);
  return NS_OK;
}

NS_IMETHODIMP
nsFileView::SetSelection(nsITreeSelection* aSelection)
{
  mSelection = aSelection;
  return NS_OK;
}

NS_IMETHODIMP
nsFileView::GetRowProperties(PRInt32 aIndex,
                             nsISupportsArray* aProperties)
{
  return NS_OK;
}

NS_IMETHODIMP
nsFileView::GetCellProperties(PRInt32 aRow, nsITreeColumn* aCol,
                              nsISupportsArray* aProperties)
{
  PRUint32 dirCount;
  mDirList->Count(&dirCount);

  if (aRow < (PRInt32) dirCount)
    aProperties->AppendElement(mDirectoryAtom);
  else if (aRow < mTotalRows)
    aProperties->AppendElement(mFileAtom);

  return NS_OK;
}

NS_IMETHODIMP
nsFileView::GetColumnProperties(nsITreeColumn* aCol,
                                nsISupportsArray* aProperties)
{
  return NS_OK;
}

NS_IMETHODIMP
nsFileView::IsContainer(PRInt32 aIndex, PRBool* aIsContainer)
{
  *aIsContainer = PR_FALSE;
  return NS_OK;
}

NS_IMETHODIMP
nsFileView::IsContainerOpen(PRInt32 aIndex, PRBool* aIsOpen)
{
  *aIsOpen = PR_FALSE;
  return NS_OK;
}

NS_IMETHODIMP
nsFileView::IsContainerEmpty(PRInt32 aIndex, PRBool* aIsEmpty)
{
  *aIsEmpty = PR_FALSE;
  return NS_OK;
}

NS_IMETHODIMP
nsFileView::IsSeparator(PRInt32 aIndex, PRBool* aIsSeparator)
{
  *aIsSeparator = PR_FALSE;
  return NS_OK;
}

NS_IMETHODIMP
nsFileView::IsSorted(PRBool* aIsSorted)
{
  *aIsSorted = (mSortType >= 0);
  return NS_OK;
}

NS_IMETHODIMP
nsFileView::CanDrop(PRInt32 aIndex, PRInt32 aOrientation, PRBool* aCanDrop)
{
  *aCanDrop = PR_FALSE;
  return NS_OK;
}

NS_IMETHODIMP
nsFileView::Drop(PRInt32 aRow, PRInt32 aOrientation)
{
  return NS_OK;
}

NS_IMETHODIMP
nsFileView::GetParentIndex(PRInt32 aRowIndex, PRInt32* aParentIndex)
{
  *aParentIndex = -1;
  return NS_OK;
}

NS_IMETHODIMP
nsFileView::HasNextSibling(PRInt32 aRowIndex, PRInt32 aAfterIndex, 
                           PRBool* aHasSibling)
{
  *aHasSibling = (aAfterIndex < (mTotalRows - 1));
  return NS_OK;
}

NS_IMETHODIMP
nsFileView::GetLevel(PRInt32 aIndex, PRInt32* aLevel)
{
  *aLevel = 0;
  return NS_OK;
}

NS_IMETHODIMP
nsFileView::GetImageSrc(PRInt32 aRow, nsITreeColumn* aCol,
                        nsAString& aImageSrc)
{
  return NS_OK;
}

NS_IMETHODIMP
nsFileView::GetProgressMode(PRInt32 aRow, nsITreeColumn* aCol,
                            PRInt32* aProgressMode)
{
  return NS_OK;
}

NS_IMETHODIMP
nsFileView::GetCellValue(PRInt32 aRow, nsITreeColumn* aCol,
                         nsAString& aCellValue)
{
  return NS_OK;
}

NS_IMETHODIMP
nsFileView::GetCellText(PRInt32 aRow, nsITreeColumn* aCol,
                        nsAString& aCellText)
{
  PRUint32 dirCount, fileCount;
  mDirList->Count(&dirCount);
  mFilteredFiles->Count(&fileCount);

  PRBool isDirectory;
  nsCOMPtr<nsIFile> curFile;

  if (aRow < (PRInt32) dirCount) {
    isDirectory = PR_TRUE;
    curFile = do_QueryElementAt(mDirList, aRow);
  } else if (aRow < mTotalRows) {
    isDirectory = PR_FALSE;
    curFile = do_QueryElementAt(mFilteredFiles, aRow - dirCount);
  } else {
    // invalid row
    aCellText.SetCapacity(0);
    return NS_OK;
  }

  const PRUnichar* colID;
  aCol->GetIdConst(&colID);
  if (NS_LITERAL_STRING("FilenameColumn").Equals(colID)) {
    curFile->GetLeafName(aCellText);
  } else if (NS_LITERAL_STRING("LastModifiedColumn").Equals(colID)) {
    PRInt64 lastModTime;
    curFile->GetLastModifiedTime(&lastModTime);
    // XXX FormatPRTime could take an nsAString&
    nsAutoString temp;
    mDateFormatter->FormatPRTime(nsnull, kDateFormatShort, kTimeFormatSeconds,
                                 lastModTime * 1000, temp);
    aCellText = temp;
  } else {
    // file size
    if (isDirectory)
      aCellText.SetCapacity(0);
    else {
      PRInt64 fileSize;
      curFile->GetFileSize(&fileSize);
      CopyUTF8toUTF16(nsPrintfCString("%lld", fileSize), aCellText);
    }
  }

  return NS_OK;
}

NS_IMETHODIMP
nsFileView::SetTree(nsITreeBoxObject* aTree)
{
  mTree = aTree;
  return NS_OK;
}

NS_IMETHODIMP
nsFileView::ToggleOpenState(PRInt32 aIndex)
{
  return NS_OK;
}

NS_IMETHODIMP
nsFileView::CycleHeader(nsITreeColumn* aCol)
{
  return NS_OK;
}

NS_IMETHODIMP
nsFileView::SelectionChanged()
{
  return NS_OK;
}

NS_IMETHODIMP
nsFileView::CycleCell(PRInt32 aRow, nsITreeColumn* aCol)
{
  return NS_OK;
}

NS_IMETHODIMP
nsFileView::IsEditable(PRInt32 aRow, nsITreeColumn* aCol,
                       PRBool* aIsEditable)
{
  *aIsEditable = PR_FALSE;
  return NS_OK;
}

NS_IMETHODIMP
nsFileView::IsSelectable(PRInt32 aRow, nsITreeColumn* aCol,
                         PRBool* aIsSelectable)
{
  *aIsSelectable = PR_FALSE;
  return NS_OK;
}

NS_IMETHODIMP
nsFileView::SetCellValue(PRInt32 aRow, nsITreeColumn* aCol,
                         const nsAString& aValue)
{
  return NS_OK;
}

NS_IMETHODIMP
nsFileView::SetCellText(PRInt32 aRow, nsITreeColumn* aCol,
                        const nsAString& aValue)
{
  return NS_OK;
}

NS_IMETHODIMP
nsFileView::PerformAction(const PRUnichar* aAction)
{
  return NS_OK;
}

NS_IMETHODIMP
nsFileView::PerformActionOnRow(const PRUnichar* aAction, PRInt32 aRow)
{
  return NS_OK;
}

NS_IMETHODIMP
nsFileView::PerformActionOnCell(const PRUnichar* aAction, PRInt32 aRow,
                                nsITreeColumn* aCol)
{
  return NS_OK;
}

// Private methods

void
nsFileView::FilterFiles()
{
  PRUint32 count = 0;
  mDirList->Count(&count);
  mTotalRows = count;
  mFileList->Count(&count);
  mFilteredFiles->Clear();
  PRInt32 filterCount = mCurrentFilters.Count();

  nsCOMPtr<nsIFile> file;
  for (PRUint32 i = 0; i < count; ++i) {
    file = do_QueryElementAt(mFileList, i);
    PRBool isHidden = PR_FALSE;
    if (!mShowHiddenFiles)
      file->IsHidden(&isHidden);
    
    nsAutoString ucsLeafName;
    if(NS_FAILED(file->GetLeafName(ucsLeafName))) {
      // need to check return value for GetLeafName()
      continue;
    }
    
    if (!isHidden) {
      for (PRInt32 j = 0; j < filterCount; ++j) {
        PRBool matched = PR_FALSE;
        if (!nsCRT::strcmp((const PRUnichar*) mCurrentFilters.ElementAt(j),
                           NS_LITERAL_STRING("..apps").get()))
        {
          file->IsExecutable(&matched);
        } else
          matched = (NS_WildCardMatch(ucsLeafName.get(),
                                      (const PRUnichar*) mCurrentFilters.ElementAt(j),
                                      PR_TRUE) == MATCH);

        if (matched) {
          mFilteredFiles->AppendElement(file);
          ++mTotalRows;
          break;
        }
      }
    }
  }
}

void
nsFileView::ReverseArray(nsISupportsArray* aArray)
{
  PRUint32 count;
  aArray->Count(&count);
  for (PRUint32 i = 0; i < count/2; ++i) {
    nsCOMPtr<nsISupports> element = dont_AddRef(aArray->ElementAt(i));
    nsCOMPtr<nsISupports> element2 = dont_AddRef(aArray->ElementAt(count-i-1));
    aArray->ReplaceElementAt(element2, i);
    aArray->ReplaceElementAt(element, count-i-1);
  }
}

PR_STATIC_CALLBACK(int)
SortNameCallback(const void* aElement1, const void* aElement2, void* aContext)
{
  nsIFile* file1 = *static_cast<nsIFile* const *>(aElement1);
  nsIFile* file2 = *static_cast<nsIFile* const *>(aElement2);
  
  nsAutoString leafName1, leafName2;
  file1->GetLeafName(leafName1);
  file2->GetLeafName(leafName2);

  return Compare(leafName1, leafName2);
}

PR_STATIC_CALLBACK(int)
SortSizeCallback(const void* aElement1, const void* aElement2, void* aContext)
{
  nsIFile* file1 = *static_cast<nsIFile* const *>(aElement1);
  nsIFile* file2 = *static_cast<nsIFile* const *>(aElement2);

  PRInt64 size1, size2;
  file1->GetFileSize(&size1);
  file2->GetFileSize(&size2);

  if (LL_EQ(size1, size2))
    return 0;

  return (LL_CMP(size1, <, size2) ? -1 : 1);
}

PR_STATIC_CALLBACK(int)
SortDateCallback(const void* aElement1, const void* aElement2, void* aContext)
{
  nsIFile* file1 = *static_cast<nsIFile* const *>(aElement1);
  nsIFile* file2 = *static_cast<nsIFile* const *>(aElement2);

  PRInt64 time1, time2;
  file1->GetLastModifiedTime(&time1);
  file2->GetLastModifiedTime(&time2);

  if (LL_EQ(time1, time2))
    return 0;

  return (LL_CMP(time1, <, time2) ? -1 : 1);
}

void
nsFileView::SortArray(nsISupportsArray* aArray)
{
  // We assume the array to be in filesystem order, which
  // for our purposes, is completely unordered.

  int (*compareFunc)(const void*, const void*, void*);

  switch (mSortType) {
  case sortName:
    compareFunc = SortNameCallback;
    break;
  case sortSize:
    compareFunc = SortSizeCallback;
    break;
  case sortDate:
    compareFunc = SortDateCallback;
    break;
  default:
    return;
  }

  PRUint32 count;
  aArray->Count(&count);

  // each item will have an additional refcount while
  // the array is alive.
  nsIFile** array = new nsIFile*[count];
  PRUint32 i;
  for (i = 0; i < count; ++i)
    aArray->QueryElementAt(i, NS_GET_IID(nsIFile), (void**)&(array[i]));

  NS_QuickSort(array, count, sizeof(nsIFile*), compareFunc, nsnull);

  for (i = 0; i < count; ++i) {
    aArray->ReplaceElementAt(array[i], i);
    NS_RELEASE(array[i]);
  }

  delete[] array;
}

void
nsFileView::SortInternal()
{
  SortArray(mDirList);
  SortArray(mFilteredFiles);

  if (mReverseSort) {
    ReverseArray(mDirList);
    ReverseArray(mFilteredFiles);
  }
}
