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
 * The Original Code is Mozilla Communicator client code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Joe Hewitt <hewitt@netscape.com> (Original Author)
 *   Dean Tessman <dean_tessman@hotmail.com>
 *   Johnny Stenback <jst@mozilla.jstenback.com>
 *   Masayuki Nakano <masayuki@d-toybox.com>
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

#include "nsAutoCompleteController.h"
#ifdef MOZ_MORK
#include "nsAutoCompleteMdbResult.h"
#endif
#include "nsAutoCompleteSimpleResult.h"
#include "nsVoidArray.h"
#include "nsAutoCompleteStringArray.h"

#include "nsNetCID.h"
#include "nsIIOService.h"
#include "nsToolkitCompsCID.h"
#include "nsIServiceManager.h"
#include "nsIAtomService.h"
#include "nsReadableUtils.h"
#include "nsUnicharUtils.h"
#include "nsITreeColumns.h"
#include "nsIGenericFactory.h"
#include "nsIObserverService.h"
#include "nsIDOMKeyEvent.h"

static const char *kAutoCompleteSearchCID = "@mozilla.org/autocomplete/search;1?name=";

NS_IMPL_ISUPPORTS4(nsAutoCompleteController, nsIAutoCompleteController,
                                             nsIAutoCompleteObserver,
                                             nsITimerCallback,
                                             nsITreeView)

nsAutoCompleteController::nsAutoCompleteController() :
  mEnterAfterSearch(0),
  mDefaultIndexCompleted(PR_FALSE),
  mBackspaced(PR_FALSE),
  mPopupClosedByCompositionStart(PR_FALSE),
  mIsIMEComposing(PR_FALSE),
  mIgnoreHandleText(PR_FALSE),
  mIsOpen(PR_FALSE),
  mSearchStatus(0),
  mRowCount(0),
  mSearchesOngoing(0),
  mFirstSearchResult(PR_FALSE)
{
  mSearches = do_CreateInstance("@mozilla.org/supports-array;1");
  mResults = do_CreateInstance("@mozilla.org/supports-array;1");
}

nsAutoCompleteController::~nsAutoCompleteController()
{
  SetInput(nsnull);
}

////////////////////////////////////////////////////////////////////////
//// nsIAutoCompleteController

NS_IMETHODIMP
nsAutoCompleteController::GetSearchStatus(PRUint16 *aSearchStatus)
{
  *aSearchStatus = mSearchStatus;
  return NS_OK;
}

NS_IMETHODIMP
nsAutoCompleteController::GetMatchCount(PRUint32 *aMatchCount)
{
  *aMatchCount = mRowCount;
  return NS_OK;
}

NS_IMETHODIMP
nsAutoCompleteController::GetInput(nsIAutoCompleteInput **aInput)
{
  *aInput = mInput;
  NS_IF_ADDREF(*aInput);
  return NS_OK;
}

NS_IMETHODIMP
nsAutoCompleteController::SetInput(nsIAutoCompleteInput *aInput)
{
  // Don't do anything if the input isn't changing.
  if (mInput == aInput)
    return NS_OK;

  // Clear out the current search context
  if (mInput) {
    // Stop all searches in case they are async.
    StopSearch();
    ClearResults();
    if (mIsOpen)
      ClosePopup();
    mSearches->Clear();
  }
    
  mInput = aInput;

  // Nothing more to do if the input was just being set to null.
  if (!aInput)
    return NS_OK;

  nsAutoString newValue;
  mInput->GetTextValue(newValue);
  
  // Reset all search state members to default values
  mSearchString = newValue;
  mEnterAfterSearch = 0;
  mDefaultIndexCompleted = PR_FALSE;
  mBackspaced = PR_FALSE;
  mSearchStatus = nsIAutoCompleteController::STATUS_NONE;
  mRowCount = 0;
  mSearchesOngoing = 0;
  
  // Initialize our list of search objects
  PRUint32 searchCount;
  mInput->GetSearchCount(&searchCount);
  mResults->SizeTo(searchCount);
  mSearches->SizeTo(searchCount);
  mMatchCounts.SetLength(searchCount);

  const char *searchCID = kAutoCompleteSearchCID;

  for (PRUint32 i = 0; i < searchCount; ++i) {
    // Use the search name to create the contract id string for the search service
    nsCAutoString searchName;
    mInput->GetSearchAt(i, searchName);
    nsCAutoString cid(searchCID);
    cid.Append(searchName);
    
    // Use the created cid to get a pointer to the search service and store it for later
    nsCOMPtr<nsIAutoCompleteSearch> search = do_GetService(cid.get());
    if (search)
      mSearches->AppendElement(search);
  }

  return NS_OK;
}

NS_IMETHODIMP
nsAutoCompleteController::StartSearch(const nsAString &aSearchString)
{ 
  mSearchString = aSearchString;
  StartSearchTimer();
  return NS_OK;
}

NS_IMETHODIMP
nsAutoCompleteController::HandleText(PRBool aIgnoreSelection)
{
  if (!mInput) {
    // Stop all searches in case they are async.
    StopSearch();
    // Note: if now is after blur and IME end composition,
    // check mInput before calling.
    // See https://bugzilla.mozilla.org/show_bug.cgi?id=193544#c31
    NS_ERROR("Called before attaching to the control or after detaching from the control");
    return NS_OK;
  }

  nsAutoString newValue;
  mInput->GetTextValue(newValue);

  // Note: the events occur in the following order when IME is used.
  // 1. composition start event(HandleStartComposition)
  // 2. composition end event(HandleEndComposition)
  // 3. input event(HandleText)
  // Note that the input event occurs if IME composition is cancelled, as well.
  // In HandleEndComposition, we are processing the popup properly.
  // Therefore, the input event after composition end event should do nothing.
  // (E.g., calling StopSearch() and ClosePopup().)
  // If it is not, popup is always closed after composition end.
  if (mIgnoreHandleText) {
    mIgnoreHandleText = PR_FALSE;
    if (newValue.Equals(mSearchString))
      return NS_OK;
    NS_ERROR("Now is after composition end event. But the value was changed.");
  }

  // Stop all searches in case they are async.
  StopSearch();

  if (!mInput) {
    // StopSearch() can call PostSearchCleanup() which might result
    // in a blur event, which could null out mInput, so we need to check it
    // again.  See bug #395344 for more details
    return NS_OK;
  }

  PRBool disabled;
  mInput->GetDisableAutoComplete(&disabled);
  NS_ENSURE_TRUE(!disabled, NS_OK);

  // Don't search again if the new string is the same as the last search
  if (newValue.Length() > 0 && newValue.Equals(mSearchString))
    return NS_OK;

  // Determine if the user has removed text from the end (probably by backspacing)
  if (newValue.Length() < mSearchString.Length() &&
      Substring(mSearchString, 0, newValue.Length()).Equals(newValue))
  {
    // We need to throw away previous results so we don't try to search through them again
    ClearResults();
    mBackspaced = PR_TRUE;
  } else
    mBackspaced = PR_FALSE;

  if (mRowCount == 0)
    // XXX Handle the case where we have no results because of an ignored prefix. 
    // This is just a hack. I have no idea what I'm doing. Hewitt, fix this the right
    // way when you get a chance. -dwh
    ClearResults();

  mSearchString = newValue;

  // Don't search if the value is empty
  if (newValue.Length() == 0) {
    ClosePopup();
    return NS_OK;
  }

  if (aIgnoreSelection) {
    StartSearchTimer();
  } else {
    // Kick off the search only if the cursor is at the end of the textbox
  PRInt32 selectionStart;
  mInput->GetSelectionStart(&selectionStart);
  PRInt32 selectionEnd;
  mInput->GetSelectionEnd(&selectionEnd);

  if (selectionStart == selectionEnd && selectionStart == (PRInt32) mSearchString.Length())
    StartSearchTimer();
  }

  return NS_OK;
}

NS_IMETHODIMP
nsAutoCompleteController::HandleEnter(PRBool aIsPopupSelection, PRBool *_retval)
{
  *_retval = PR_FALSE;
  if (!mInput)
    return NS_OK;

  // allow the event through unless there is something selected in the popup
  mInput->GetPopupOpen(_retval);
  if (*_retval) {
    nsCOMPtr<nsIAutoCompletePopup> popup;
    mInput->GetPopup(getter_AddRefs(popup));

    if (popup) {
      PRInt32 selectedIndex;
      popup->GetSelectedIndex(&selectedIndex);
      *_retval = selectedIndex >= 0;
    }
  }
  
  // clear the search timer only if we are not searching.
  // if we are searching, EnterMatch() will not handle the enter
  // immediately.  instead, we will handle it on the next result we process
  // but we need the search timer to fire to kick of that search 
  if (mSearchStatus != nsIAutoCompleteController::STATUS_SEARCHING)
    ClearSearchTimer();
  EnterMatch(aIsPopupSelection);
  
  return NS_OK;
}

NS_IMETHODIMP
nsAutoCompleteController::HandleEscape(PRBool *_retval)
{
  *_retval = PR_FALSE;
  if (!mInput)
    return NS_OK;

  // allow the event through if the popup is closed
  mInput->GetPopupOpen(_retval);
  
  // Stop all searches in case they are async.
  StopSearch();
  ClearResults();
  RevertTextValue();
  ClosePopup();

  return NS_OK;
}

NS_IMETHODIMP
nsAutoCompleteController::HandleStartComposition()
{
  NS_ENSURE_TRUE(!mIsIMEComposing, NS_OK);

  mPopupClosedByCompositionStart = PR_FALSE;
  mIsIMEComposing = PR_TRUE;

  if (!mInput)
    return NS_OK;

  PRBool disabled;
  mInput->GetDisableAutoComplete(&disabled);
  if (disabled)
    return NS_OK;

  // Stop all searches in case they are async.
  StopSearch();

  if (!mInput) {
    // StopSearch() can call PostSearchCleanup() which might result
    // in a blur event, which could null out mInput, so we need to check it
    // again.  See bug #408463 for more details
    return NS_OK;
  }

  PRBool isOpen;
  mInput->GetPopupOpen(&isOpen);
  if (isOpen)
    ClosePopup();
  mPopupClosedByCompositionStart = isOpen;
  return NS_OK;
}

NS_IMETHODIMP
nsAutoCompleteController::HandleEndComposition()
{
  NS_ENSURE_TRUE(mIsIMEComposing, NS_OK);

  mIsIMEComposing = PR_FALSE;
  PRBool forceOpenPopup = mPopupClosedByCompositionStart;
  mPopupClosedByCompositionStart = PR_FALSE;

  if (!mInput)
    return NS_OK;

  nsAutoString value;
  mInput->GetTextValue(value);
  SetSearchString(EmptyString());
  if (!value.IsEmpty()) {
    // Show the popup with a filtered result set
    HandleText(PR_TRUE);
  } else if (forceOpenPopup) {
    PRBool cancel;
    HandleKeyNavigation(nsIDOMKeyEvent::DOM_VK_DOWN, &cancel);
  }
  // On here, |value| and |mSearchString| are same. Therefore, next HandleText should be
  // ignored. Because there are no reason to research.
  mIgnoreHandleText = PR_TRUE;

  return NS_OK;
}

NS_IMETHODIMP
nsAutoCompleteController::HandleTab()
{
  PRBool cancel;
  return HandleEnter(PR_FALSE, &cancel);
}

NS_IMETHODIMP
nsAutoCompleteController::HandleKeyNavigation(PRUint32 aKey, PRBool *_retval)
{
  // By default, don't cancel the event
  *_retval = PR_FALSE;

  if (!mInput) {
    // Note: if now is after blur and IME end composition,
    // check mInput before calling.
    // See https://bugzilla.mozilla.org/show_bug.cgi?id=193544#c31
    NS_ERROR("Called before attaching to the control or after detaching from the control");
    return NS_OK;
  }

  nsCOMPtr<nsIAutoCompletePopup> popup;
  mInput->GetPopup(getter_AddRefs(popup));
  NS_ENSURE_TRUE(popup != nsnull, NS_ERROR_FAILURE);

  PRBool disabled;
  mInput->GetDisableAutoComplete(&disabled);
  NS_ENSURE_TRUE(!disabled, NS_OK);

  if (aKey == nsIDOMKeyEvent::DOM_VK_UP ||
      aKey == nsIDOMKeyEvent::DOM_VK_DOWN || 
      aKey == nsIDOMKeyEvent::DOM_VK_PAGE_UP || 
      aKey == nsIDOMKeyEvent::DOM_VK_PAGE_DOWN)
  {
    // Prevent the input from handling up/down events, as it may move
    // the cursor to home/end on some systems
    *_retval = PR_TRUE;
    
    PRBool isOpen;
    mInput->GetPopupOpen(&isOpen);
    if (isOpen) {
      PRBool reverse = aKey == nsIDOMKeyEvent::DOM_VK_UP ||
                      aKey == nsIDOMKeyEvent::DOM_VK_PAGE_UP ? PR_TRUE : PR_FALSE;
      PRBool page = aKey == nsIDOMKeyEvent::DOM_VK_PAGE_UP ||
                    aKey == nsIDOMKeyEvent::DOM_VK_PAGE_DOWN ? PR_TRUE : PR_FALSE;
      
      // Fill in the value of the textbox with whatever is selected in the popup
      // if the completeSelectedIndex attribute is set.  We check this before
      // calling SelectBy since that can null out mInput in some cases.
      PRBool completeSelection;
      mInput->GetCompleteSelectedIndex(&completeSelection);

      // Instruct the result view to scroll by the given amount and direction
      popup->SelectBy(reverse, page);

      if (completeSelection)
      {
        PRInt32 selectedIndex;
        popup->GetSelectedIndex(&selectedIndex);
        if (selectedIndex >= 0) {
          //  A result is selected, so fill in its value
          nsAutoString value;
          if (NS_SUCCEEDED(GetResultValueAt(selectedIndex, PR_TRUE, value))) {
            mInput->SetTextValue(value);
            mInput->SelectTextRange(value.Length(), value.Length());
          }
        } else {
          // Nothing is selected, so fill in the last typed value
          mInput->SetTextValue(mSearchString);
          mInput->SelectTextRange(mSearchString.Length(), mSearchString.Length());
        }
      }
    } else {
      // Open the popup if there has been a previous search, or else kick off a new search
      PRUint32 resultCount;
      mResults->Count(&resultCount);
      if (resultCount) {
        if (mRowCount) {
          OpenPopup();
        }
      } else
        StartSearchTimer();
    }    
  } else if (   aKey == nsIDOMKeyEvent::DOM_VK_LEFT 
             || aKey == nsIDOMKeyEvent::DOM_VK_RIGHT 
#ifndef XP_MACOSX
             || aKey == nsIDOMKeyEvent::DOM_VK_HOME
#endif
            )
  {
    // The user hit a text-navigation key.
    PRBool isOpen;
    mInput->GetPopupOpen(&isOpen);
    if (isOpen) {
      PRInt32 selectedIndex;
      popup->GetSelectedIndex(&selectedIndex);
      if (selectedIndex >= 0) {
        // The pop-up is open and has a selection, take its value
        nsAutoString value;
        if (NS_SUCCEEDED(GetResultValueAt(selectedIndex, PR_TRUE, value))) {
          mInput->SetTextValue(value);
          mInput->SelectTextRange(value.Length(), value.Length());
        }
      }
      // Close the pop-up even if nothing was selected
      ClearSearchTimer();
      ClosePopup();
    }

    // mInput may be null, see bug #408463 and bug #409902
    if (mInput) {
      // Update last-searched string to the current input, since the input may
      // have changed.  Without this, subsequent backspaces look like text
      // additions, not text deletions.
      nsAutoString value;

      mInput->GetTextValue(value);
      mSearchString = value;
    }
    else
      NS_WARNING("unexpected, please comment in bug #409902");
  }
  
  return NS_OK;
}

NS_IMETHODIMP
nsAutoCompleteController::HandleDelete(PRBool *_retval)
{
  *_retval = PR_FALSE;
  if (!mInput)
    return NS_OK;

  PRBool isOpen = PR_FALSE;
  mInput->GetPopupOpen(&isOpen);
  if (!isOpen || mRowCount <= 0) {
    // Nothing left to delete, proceed as normal
    HandleText(PR_FALSE);
    return NS_OK;
  }
  
  nsCOMPtr<nsIAutoCompletePopup> popup;
  mInput->GetPopup(getter_AddRefs(popup));

  PRInt32 index, searchIndex, rowIndex;
  popup->GetSelectedIndex(&index);
  RowIndexToSearch(index, &searchIndex, &rowIndex);
  NS_ENSURE_TRUE(searchIndex >= 0 && rowIndex >= 0, NS_ERROR_FAILURE);

  nsCOMPtr<nsIAutoCompleteResult> result;
  mResults->GetElementAt(searchIndex, getter_AddRefs(result));
  NS_ENSURE_TRUE(result, NS_ERROR_FAILURE);

  nsAutoString search;
  mInput->GetSearchParam(search);

  // Clear the row in our result and in the DB.
  result->RemoveValueAt(rowIndex, PR_TRUE);
  --mRowCount;

  // Unselect the current item.
  popup->SetSelectedIndex(-1);

  // Tell the tree that the row count changed. 
  if (mTree)
    mTree->RowCountChanged(mRowCount, -1);

  // Adjust index, if needed.
  if (index >= (PRInt32)mRowCount)
    index = mRowCount - 1;

  if (mRowCount > 0) {
    // There are still rows in the popup, select the current index again.
    popup->SetSelectedIndex(index);

    // Complete to the new current value.
    nsAutoString value;
    if (NS_SUCCEEDED(GetResultValueAt(index, PR_TRUE, value))) {
      CompleteValue(value, PR_FALSE);

      // Make sure we cancel the event that triggerd this call.
      *_retval = PR_TRUE;
    }

    // Invalidate the popup.
    popup->Invalidate();
  } else {
    // Nothing left in the popup, clear any pending search timers and
    // close the popup.
    ClearSearchTimer();
    ClosePopup();
  }

  return NS_OK;
}

NS_IMETHODIMP
nsAutoCompleteController::GetValueAt(PRInt32 aIndex, nsAString & _retval)
{
  GetResultValueAt(aIndex, PR_FALSE, _retval);
  
  return NS_OK;
}

NS_IMETHODIMP
nsAutoCompleteController::GetCommentAt(PRInt32 aIndex, nsAString & _retval)
{
  PRInt32 searchIndex;
  PRInt32 rowIndex;
  RowIndexToSearch(aIndex, &searchIndex, &rowIndex);
  NS_ENSURE_TRUE(searchIndex >= 0 && rowIndex >= 0, NS_ERROR_FAILURE);
  
  nsCOMPtr<nsIAutoCompleteResult> result;
  mResults->GetElementAt(searchIndex, getter_AddRefs(result));
  NS_ENSURE_TRUE(result, NS_ERROR_FAILURE);

  return result->GetCommentAt(rowIndex, _retval);
}

NS_IMETHODIMP
nsAutoCompleteController::GetStyleAt(PRInt32 aIndex, nsAString & _retval)
{
  PRInt32 searchIndex;
  PRInt32 rowIndex;
  RowIndexToSearch(aIndex, &searchIndex, &rowIndex);
  NS_ENSURE_TRUE(searchIndex >= 0 && rowIndex >= 0, NS_ERROR_FAILURE);
  
  nsCOMPtr<nsIAutoCompleteResult> result;
  mResults->GetElementAt(searchIndex, getter_AddRefs(result));
  NS_ENSURE_TRUE(result, NS_ERROR_FAILURE);

  return result->GetStyleAt(rowIndex, _retval);
}

NS_IMETHODIMP
nsAutoCompleteController::GetImageAt(PRInt32 aIndex, nsAString & _retval)
{
  PRInt32 searchIndex;
  PRInt32 rowIndex;
  RowIndexToSearch(aIndex, &searchIndex, &rowIndex);
  NS_ENSURE_TRUE(searchIndex >= 0 && rowIndex >= 0, NS_ERROR_FAILURE);
  
  nsCOMPtr<nsIAutoCompleteResult> result;
  mResults->GetElementAt(searchIndex, getter_AddRefs(result));
  NS_ENSURE_TRUE(result, NS_ERROR_FAILURE);

  return result->GetImageAt(rowIndex, _retval);
}

NS_IMETHODIMP
nsAutoCompleteController::SetSearchString(const nsAString &aSearchString)
{ 
  mSearchString = aSearchString;
  return NS_OK;
}

NS_IMETHODIMP
nsAutoCompleteController::GetSearchString(nsAString &aSearchString)
{ 
  aSearchString = mSearchString;
  return NS_OK;
}


////////////////////////////////////////////////////////////////////////
//// nsIAutoCompleteObserver

NS_IMETHODIMP
nsAutoCompleteController::OnSearchResult(nsIAutoCompleteSearch *aSearch, nsIAutoCompleteResult* aResult)
{
  // look up the index of the search which is returning
  PRUint32 count;
  mSearches->Count(&count);
  for (PRUint32 i = 0; i < count; ++i) {
    nsCOMPtr<nsIAutoCompleteSearch> search;
    mSearches->GetElementAt(i, getter_AddRefs(search));
    if (search == aSearch) {
      ProcessResult(i, aResult);
    }
  }
  
  return NS_OK;
}

////////////////////////////////////////////////////////////////////////
//// nsITimerCallback

NS_IMETHODIMP
nsAutoCompleteController::Notify(nsITimer *timer)
{
  mTimer = nsnull;
  StartSearch();
  return NS_OK;
}

////////////////////////////////////////////////////////////////////////
// nsITreeView

NS_IMETHODIMP
nsAutoCompleteController::GetRowCount(PRInt32 *aRowCount)
{
  *aRowCount = mRowCount;
  return NS_OK;
}

NS_IMETHODIMP
nsAutoCompleteController::GetRowProperties(PRInt32 index, nsISupportsArray *properties)
{
  // XXX This is a hack because the tree doesn't seem to be painting the selected row
  //     the normal way.  Please remove this ASAP.
  PRInt32 currentIndex;
  mSelection->GetCurrentIndex(&currentIndex);
  
  /*
  if (index == currentIndex) {
    nsCOMPtr<nsIAtomService> atomSvc = do_GetService("@mozilla.org/atom-service;1");
    nsCOMPtr<nsIAtom> atom;
    atomSvc->GetAtom(NS_LITERAL_STRING("menuactive").get(), getter_AddRefs(atom));
    properties->AppendElement(atom);
  }
  */

  return NS_OK;
}

NS_IMETHODIMP
nsAutoCompleteController::GetCellProperties(PRInt32 row, nsITreeColumn* col, nsISupportsArray* properties)
{
  GetRowProperties(row, properties);
  
  if (row >= 0) {
    nsAutoString className;
    GetStyleAt(row, className);
    if (!className.IsEmpty()) {
      nsCOMPtr<nsIAtomService> atomSvc = do_GetService("@mozilla.org/atom-service;1");
      nsCOMPtr<nsIAtom> atom;
      atomSvc->GetAtom(className.get(), getter_AddRefs(atom));
      properties->AppendElement(atom);
    }
  }
  
  return NS_OK;
}

NS_IMETHODIMP
nsAutoCompleteController::GetColumnProperties(nsITreeColumn* col, nsISupportsArray* properties)
{
  return NS_OK;
}

NS_IMETHODIMP
nsAutoCompleteController::GetImageSrc(PRInt32 row, nsITreeColumn* col, nsAString& _retval)
{
  const PRUnichar* colID;
  col->GetIdConst(&colID);

  if (NS_LITERAL_STRING("treecolAutoCompleteValue").Equals(colID))
    return GetImageAt(row, _retval);

  return NS_OK;
}

NS_IMETHODIMP
nsAutoCompleteController::GetProgressMode(PRInt32 row, nsITreeColumn* col, PRInt32* _retval)
{
  NS_NOTREACHED("tree has no progress cells");
  return NS_OK;
}

NS_IMETHODIMP
nsAutoCompleteController::GetCellValue(PRInt32 row, nsITreeColumn* col, nsAString& _retval)
{  
  NS_NOTREACHED("all of our cells are text");
  return NS_OK;
}

NS_IMETHODIMP
nsAutoCompleteController::GetCellText(PRInt32 row, nsITreeColumn* col, nsAString& _retval)
{
  const PRUnichar* colID;
  col->GetIdConst(&colID);
  
  if (NS_LITERAL_STRING("treecolAutoCompleteValue").Equals(colID))
    GetValueAt(row, _retval);
  else if (NS_LITERAL_STRING("treecolAutoCompleteComment").Equals(colID))
    GetCommentAt(row, _retval);
     
  return NS_OK;
}

NS_IMETHODIMP
nsAutoCompleteController::IsContainer(PRInt32 index, PRBool *_retval)
{
  *_retval = PR_FALSE;
  return NS_OK;
}

NS_IMETHODIMP
nsAutoCompleteController::IsContainerOpen(PRInt32 index, PRBool *_retval)
{
  NS_NOTREACHED("no container cells");
  return NS_OK;
}

NS_IMETHODIMP
nsAutoCompleteController::IsContainerEmpty(PRInt32 index, PRBool *_retval)
{
  NS_NOTREACHED("no container cells");
  return NS_OK;
}

NS_IMETHODIMP
nsAutoCompleteController::GetLevel(PRInt32 index, PRInt32 *_retval)
{
  *_retval = 0;
  return NS_OK;
}

NS_IMETHODIMP
nsAutoCompleteController::GetParentIndex(PRInt32 rowIndex, PRInt32 *_retval)
{
  *_retval = 0;
  return NS_OK;
}

NS_IMETHODIMP
nsAutoCompleteController::HasNextSibling(PRInt32 rowIndex, PRInt32 afterIndex, PRBool *_retval)
{
  *_retval = PR_FALSE;
  return NS_OK;
}

NS_IMETHODIMP
nsAutoCompleteController::ToggleOpenState(PRInt32 index)
{
  return NS_OK;
}

NS_IMETHODIMP
nsAutoCompleteController::SetTree(nsITreeBoxObject *tree)
{
  mTree = tree;
  return NS_OK;
}

NS_IMETHODIMP
nsAutoCompleteController::GetSelection(nsITreeSelection * *aSelection)
{
  *aSelection = mSelection;
  NS_IF_ADDREF(*aSelection);
  return NS_OK;
}

NS_IMETHODIMP nsAutoCompleteController::SetSelection(nsITreeSelection * aSelection)
{
  mSelection = aSelection;
  return NS_OK;
}

NS_IMETHODIMP
nsAutoCompleteController::SelectionChanged()
{
  return NS_OK;
}

NS_IMETHODIMP
nsAutoCompleteController::SetCellValue(PRInt32 row, nsITreeColumn* col, const nsAString& value)
{
  return NS_OK;
}

NS_IMETHODIMP
nsAutoCompleteController::SetCellText(PRInt32 row, nsITreeColumn* col, const nsAString& value)
{
  return NS_OK;
}

NS_IMETHODIMP
nsAutoCompleteController::CycleHeader(nsITreeColumn* col)
{
  return NS_OK;
}

NS_IMETHODIMP
nsAutoCompleteController::CycleCell(PRInt32 row, nsITreeColumn* col)
{
  return NS_OK;
}

NS_IMETHODIMP
nsAutoCompleteController::IsEditable(PRInt32 row, nsITreeColumn* col, PRBool *_retval)
{
  *_retval = PR_FALSE;
  return NS_OK;
}

NS_IMETHODIMP
nsAutoCompleteController::IsSelectable(PRInt32 row, nsITreeColumn* col, PRBool *_retval)
{
  *_retval = PR_FALSE;
  return NS_OK;
}

NS_IMETHODIMP
nsAutoCompleteController::IsSeparator(PRInt32 index, PRBool *_retval)
{
  *_retval = PR_FALSE;
  return NS_OK;
}

NS_IMETHODIMP
nsAutoCompleteController::IsSorted(PRBool *_retval)
{
  *_retval = PR_FALSE;
  return NS_OK;
}

NS_IMETHODIMP
nsAutoCompleteController::CanDrop(PRInt32 index, PRInt32 orientation, PRBool *_retval)
{
  return NS_OK;
}

NS_IMETHODIMP
nsAutoCompleteController::Drop(PRInt32 row, PRInt32 orientation)
{
  return NS_OK;
}

NS_IMETHODIMP
nsAutoCompleteController::PerformAction(const PRUnichar *action)
{
  return NS_OK;
}

NS_IMETHODIMP
nsAutoCompleteController::PerformActionOnRow(const PRUnichar *action, PRInt32 row)
{
  return NS_OK;
}

NS_IMETHODIMP
nsAutoCompleteController::PerformActionOnCell(const PRUnichar* action, PRInt32 row, nsITreeColumn* col)
{
  return NS_OK;
}

////////////////////////////////////////////////////////////////////////
//// nsAutoCompleteController

nsresult
nsAutoCompleteController::OpenPopup()
{
  PRUint32 minResults;
  mInput->GetMinResultsForPopup(&minResults);

  if (mRowCount >= minResults) {
    mIsOpen = PR_TRUE;
    return mInput->SetPopupOpen(PR_TRUE);
  }
  
  return NS_OK;
}

nsresult
nsAutoCompleteController::ClosePopup()
{
  if (!mInput) {
    return NS_OK;
  }

  PRBool isOpen;
  mInput->GetPopupOpen(&isOpen);
  if (!isOpen)
    return NS_OK;

  nsCOMPtr<nsIAutoCompletePopup> popup;
  mInput->GetPopup(getter_AddRefs(popup));
  NS_ENSURE_TRUE(popup != nsnull, NS_ERROR_FAILURE);
  popup->SetSelectedIndex(-1);
  mIsOpen = PR_FALSE;
  return mInput->SetPopupOpen(PR_FALSE);
}

nsresult
nsAutoCompleteController::StartSearch()
{
  NS_ENSURE_STATE(mInput);
  mSearchStatus = nsIAutoCompleteController::STATUS_SEARCHING;
  mDefaultIndexCompleted = PR_FALSE;
  
  PRUint32 count;
  mSearches->Count(&count);
  mSearchesOngoing = count;
  mFirstSearchResult = PR_TRUE;

  // notify the input that the search is beginning
  mInput->OnSearchBegin();
  
  PRUint32 searchesFailed = 0;
  for (PRUint32 i = 0; i < count; ++i) {
    nsCOMPtr<nsIAutoCompleteSearch> search;
    mSearches->GetElementAt(i, getter_AddRefs(search));
    nsCOMPtr<nsIAutoCompleteResult> result;
    mResults->GetElementAt(i, getter_AddRefs(result));
    
    if (result) {
      PRUint16 searchResult;
      result->GetSearchResult(&searchResult);
      if (searchResult != nsIAutoCompleteResult::RESULT_SUCCESS &&
          searchResult != nsIAutoCompleteResult::RESULT_SUCCESS_ONGOING)
        result = nsnull;
    }
    
    nsAutoString searchParam;
    // XXXben - can yank this when we discover what's causing this to 
    // fail & crash. 
    nsresult rv = mInput->GetSearchParam(searchParam);
    if (NS_FAILED(rv))
        return rv;
    
    rv = search->StartSearch(mSearchString, searchParam, result, static_cast<nsIAutoCompleteObserver *>(this));
    if (NS_FAILED(rv)) {
      ++searchesFailed;
      --mSearchesOngoing;
    }
  }
  
  if (searchesFailed == count)
    PostSearchCleanup();

  return NS_OK;
}

NS_IMETHODIMP
nsAutoCompleteController::StopSearch()
{
  // Stop the timer if there is one
  ClearSearchTimer();

  // Stop any ongoing asynchronous searches
  if (mSearchStatus == nsIAutoCompleteController::STATUS_SEARCHING) {
    PRUint32 count;
    mSearches->Count(&count);
  
    for (PRUint32 i = 0; i < count; ++i) {
      nsCOMPtr<nsIAutoCompleteSearch> search;
      mSearches->GetElementAt(i, getter_AddRefs(search));
      search->StopSearch();
    }
    mSearchesOngoing = 0;
    // since we were searching, but now we've stopped,
    // we need to call PostSearchCleanup()
    PostSearchCleanup();
  } 
  return NS_OK;
}

nsresult
nsAutoCompleteController::StartSearchTimer()
{
  // Don't create a new search timer if we're already waiting for one to fire.
  // If we don't check for this, we won't be able to cancel the original timer
  // and may crash when it fires (bug 236659).
  if (mTimer || !mInput)
    return NS_OK;

  PRUint32 timeout;
  mInput->GetTimeout(&timeout);

  mTimer = do_CreateInstance("@mozilla.org/timer;1");
  mTimer->InitWithCallback(this, timeout, nsITimer::TYPE_ONE_SHOT);
  return NS_OK;
}

nsresult
nsAutoCompleteController::ClearSearchTimer()
{
  if (mTimer) {
    mTimer->Cancel();
    mTimer = nsnull;
  }
  return NS_OK;
}

nsresult
nsAutoCompleteController::EnterMatch(PRBool aIsPopupSelection)
{
  // If a search is still ongoing, bail out of this function
  // and let the search finish, and tell it to come back here when it's done
  if (mSearchStatus == nsIAutoCompleteController::STATUS_SEARCHING) {
    mEnterAfterSearch = aIsPopupSelection ? 2 : 1;
    return NS_OK;
  }
  mEnterAfterSearch = 0;
  
  nsCOMPtr<nsIAutoCompletePopup> popup;
  mInput->GetPopup(getter_AddRefs(popup));
  NS_ENSURE_TRUE(popup != nsnull, NS_ERROR_FAILURE);
  
  PRBool forceComplete;
  mInput->GetForceComplete(&forceComplete);
  
  // Ask the popup if it wants to enter a special value into the textbox
  nsAutoString value;
  popup->GetOverrideValue(value);
  if (value.IsEmpty()) {
    PRBool completeSelection;
    mInput->GetCompleteSelectedIndex(&completeSelection);

    // If completeselectedindex is false or a row was selected from the popup,
    // enter it into the textbox. If completeselectedindex is true, or
    // EnterMatch was called via other means, for instance pressing Enter,
    // don't fill in the value as it will have already been filled in as needed.
    PRInt32 selectedIndex;
    popup->GetSelectedIndex(&selectedIndex);
    if (selectedIndex >= 0 && (!completeSelection || aIsPopupSelection))
      GetResultValueAt(selectedIndex, PR_TRUE, value);
    
    if (forceComplete && value.IsEmpty()) {
      // Since nothing was selected, and forceComplete is specified, that means
      // we have to find the first default match and enter it instead
      PRUint32 count;
      mResults->Count(&count);
      for (PRUint32 i = 0; i < count; ++i) {
        nsCOMPtr<nsIAutoCompleteResult> result;
        mResults->GetElementAt(i, getter_AddRefs(result));

        if (result) {
          PRInt32 defaultIndex;
          result->GetDefaultIndex(&defaultIndex);
          if (defaultIndex >= 0) {
            result->GetValueAt(defaultIndex, value);
            break;
          }
        }
      }
    }
  }
  
  nsCOMPtr<nsIObserverService> obsSvc =
    do_GetService("@mozilla.org/observer-service;1");
  NS_ENSURE_STATE(obsSvc);
  obsSvc->NotifyObservers(mInput, "autocomplete-will-enter-text", nsnull);

  if (!value.IsEmpty()) {
    mInput->SetTextValue(value);
    mInput->SelectTextRange(value.Length(), value.Length());
    mSearchString = value;
  }
  
  obsSvc->NotifyObservers(mInput, "autocomplete-did-enter-text", nsnull);
  ClosePopup();
  
  PRBool cancel;
  mInput->OnTextEntered(&cancel);
  
  return NS_OK;
}

nsresult
nsAutoCompleteController::RevertTextValue()
{
  // StopSearch() can call PostSearchCleanup() which might result
  // in a blur event, which could null out mInput, so we need to check it
  // again.  See bug #408463 for more details
  if (!mInput)
    return NS_OK;

  nsAutoString oldValue(mSearchString);
  
  PRBool cancel = PR_FALSE;
  mInput->OnTextReverted(&cancel);  

  if (!cancel) {
    nsCOMPtr<nsIObserverService> obsSvc =
      do_GetService("@mozilla.org/observer-service;1");
    NS_ENSURE_STATE(obsSvc);
    obsSvc->NotifyObservers(mInput, "autocomplete-will-revert-text", nsnull);

    mInput->SetTextValue(oldValue);

    obsSvc->NotifyObservers(mInput, "autocomplete-did-revert-text", nsnull);
  }

  return NS_OK;
}

nsresult
nsAutoCompleteController::ProcessResult(PRInt32 aSearchIndex, nsIAutoCompleteResult *aResult)
{
  NS_ENSURE_STATE(mInput);

  // If this is the first search result we are processing
  // we should clear out the previously cached results
  if (mFirstSearchResult) {
    ClearResults();
    mFirstSearchResult = PR_FALSE;
  }

  PRUint16 result = 0;
  if (aResult)
    aResult->GetSearchResult(&result);

  // if our results are incremental, the search is still ongoing
  if (result != nsIAutoCompleteResult::RESULT_SUCCESS_ONGOING &&
      result != nsIAutoCompleteResult::RESULT_NOMATCH_ONGOING) {
    --mSearchesOngoing;
  }

  PRUint32 oldMatchCount = 0;
  PRUint32 matchCount = 0;
  if (aResult)
    aResult->GetMatchCount(&matchCount);

  PRInt32 oldIndex = mResults->IndexOf(aResult);
  if (oldIndex == -1) {
    // cache the result
    mResults->AppendElement(aResult);
    mMatchCounts.AppendElement(matchCount);
  }
  else {
    // replace the cached result
    mResults->ReplaceElementAt(aResult, oldIndex);
    oldMatchCount = mMatchCounts[aSearchIndex];
    mMatchCounts[oldIndex] = matchCount;
  }

  PRUint32 oldRowCount = mRowCount;
  // If the search failed, increase the match count 
  // to include the error description
  if (result == nsIAutoCompleteResult::RESULT_FAILURE) {
    nsAutoString error;
    aResult->GetErrorDescription(error);
    if (!error.IsEmpty()) {
      ++mRowCount;
      if (mTree)
        mTree->RowCountChanged(oldRowCount, 1);
    }
  } else if (result == nsIAutoCompleteResult::RESULT_SUCCESS ||
             result == nsIAutoCompleteResult::RESULT_SUCCESS_ONGOING) {
    // Increase the match count for all matches in this result
    mRowCount += matchCount - oldMatchCount;

    if (mTree)
      mTree->RowCountChanged(oldRowCount, matchCount - oldMatchCount);

    // Try to autocomplete the default index for this search
    CompleteDefaultIndex(aSearchIndex);
  }

  // Refresh the popup view to display the new search results
  nsCOMPtr<nsIAutoCompletePopup> popup;
  mInput->GetPopup(getter_AddRefs(popup));
  NS_ENSURE_TRUE(popup != nsnull, NS_ERROR_FAILURE);
  popup->Invalidate();
  
  // Make sure the popup is open, if necessary, since we now have at least one
  // search result ready to display. Don't force the popup closed if we might
  // get results in the future to avoid unnecessarily canceling searches.
  if (mRowCount)
    OpenPopup();
  else if (result != nsIAutoCompleteResult::RESULT_NOMATCH_ONGOING)
    ClosePopup();

  if (mSearchesOngoing == 0) {
    // If this is the last search to return, cleanup
    PostSearchCleanup();
  }
  else if (mEnterAfterSearch) {
    // since we still have searches ongoing (mSearchesOngoing != 0)
    // and the user has hit enter, stop the searches
    StopSearch();
  }

  return NS_OK;
}

nsresult
nsAutoCompleteController::PostSearchCleanup()
{  
  NS_ENSURE_STATE(mInput);
  if (mRowCount) {
    OpenPopup();
    mSearchStatus = nsIAutoCompleteController::STATUS_COMPLETE_MATCH;
  } else {
    mSearchStatus = nsIAutoCompleteController::STATUS_COMPLETE_NO_MATCH;
    ClosePopup();
  }
  
  // notify the input that the search is complete
  mInput->OnSearchComplete();
  
  // if mEnterAfterSearch was set, then the user hit enter while the 
  // search was ongoing, so we need to enter a match now that the search is done
  if (mEnterAfterSearch)
    EnterMatch(mEnterAfterSearch == 2);

  return NS_OK;
}

nsresult
nsAutoCompleteController::ClearResults()
{
  PRInt32 oldRowCount = mRowCount;
  mRowCount = 0;
  mResults->Clear();
  mMatchCounts.Clear();
  if (oldRowCount != 0) {
    if (mTree)
      mTree->RowCountChanged(0, -oldRowCount);
    else if (mInput) {
      nsCOMPtr<nsIAutoCompletePopup> popup;
      mInput->GetPopup(getter_AddRefs(popup));
      NS_ENSURE_TRUE(popup != nsnull, NS_ERROR_FAILURE);
      // if we had a tree, RowCountChanged() would have cleared the selection
      // when the selected row was removed.  But since we don't have a tree,
      // we need to clear the selection manually.
      popup->SetSelectedIndex(-1);
    }
  }
  return NS_OK;
}

nsresult
nsAutoCompleteController::CompleteDefaultIndex(PRInt32 aSearchIndex)
{
  if (mDefaultIndexCompleted || mEnterAfterSearch || mBackspaced || mRowCount == 0 || mSearchString.Length() == 0)
    return NS_OK;

  PRBool shouldComplete;
  mInput->GetCompleteDefaultIndex(&shouldComplete);
  if (!shouldComplete)
    return NS_OK;

  nsCOMPtr<nsIAutoCompleteSearch> search;
  mSearches->GetElementAt(aSearchIndex, getter_AddRefs(search));
  nsCOMPtr<nsIAutoCompleteResult> result;
  mResults->GetElementAt(aSearchIndex, getter_AddRefs(result));
  NS_ENSURE_TRUE(result != nsnull, NS_ERROR_FAILURE);
  
  // The search must explicitly provide a default index in order
  // for us to be able to complete 
  PRInt32 defaultIndex;
  result->GetDefaultIndex(&defaultIndex);
  NS_ENSURE_TRUE(defaultIndex >= 0, NS_OK);

  nsAutoString resultValue;
  result->GetValueAt(defaultIndex, resultValue);
  CompleteValue(resultValue, PR_TRUE);
  
  mDefaultIndexCompleted = PR_TRUE;

  return NS_OK;
}

nsresult
nsAutoCompleteController::CompleteValue(nsString &aValue, 
                                        PRBool selectDifference)
/* mInput contains mSearchString, which we want to autocomplete to aValue.  If
 * selectDifference is true, select the remaining portion of aValue not 
 * contained in mSearchString. */
{
  const PRInt32 mSearchStringLength = mSearchString.Length();
  PRInt32 endSelect = aValue.Length();  // By default, select all of aValue.

  if (aValue.IsEmpty() || 
      StringBeginsWith(aValue, mSearchString,
                       nsCaseInsensitiveStringComparator())) {
    // aValue is empty (we were asked to clear mInput), or mSearchString 
    // matches the beginning of aValue.  In either case we can simply 
    // autocomplete to aValue.
    mInput->SetTextValue(aValue);
  } else {
    PRInt32 findIndex;  // Offset of mSearchString within aValue.

    nsresult rv;
    nsCOMPtr<nsIIOService> ios = do_GetService(NS_IOSERVICE_CONTRACTID, &rv);
    NS_ENSURE_SUCCESS(rv, rv);
    nsCAutoString scheme;
    if (NS_SUCCEEDED(ios->ExtractScheme(NS_ConvertUTF16toUTF8(aValue), scheme))) {
      // Trying to autocomplete a URI from somewhere other than the beginning.
      // Only succeed if the missing portion is "http://"; otherwise do not
      // autocomplete.  This prevents us from "helpfully" autocompleting to a
      // URI that isn't equivalent to what the user expected.
      findIndex = 7; // length of "http://"

      if ((endSelect < findIndex + mSearchStringLength) ||
          !scheme.LowerCaseEqualsLiteral("http") ||
          !Substring(aValue, findIndex, mSearchStringLength).Equals(
            mSearchString, nsCaseInsensitiveStringComparator())) {
        return NS_OK;
      }
    } else {
      // Autocompleting something other than a URI from the middle.  Assume we
      // can just go ahead and autocomplete the missing final portion; this
      // seems like a problematic assumption...
      nsString::const_iterator iter, end;
      aValue.BeginReading(iter);
      aValue.EndReading(end);
      const nsString::const_iterator::pointer start = iter.get();
      ++iter;  // Skip past beginning since we know that doesn't match

      FindInReadable(mSearchString, iter, end, 
                     nsCaseInsensitiveStringComparator());

      findIndex = iter.get() - start;
    }

    mInput->SetTextValue(mSearchString + 
                         Substring(aValue, mSearchStringLength + findIndex,
                                   endSelect));

    endSelect -= findIndex; // We're skipping this many characters of aValue.
  }

  mInput->SelectTextRange(selectDifference ? 
                          mSearchStringLength : endSelect, endSelect);

  return NS_OK;
}

nsresult
nsAutoCompleteController::GetResultValueAt(PRInt32 aIndex, PRBool aValueOnly, nsAString & _retval)
{
  NS_ENSURE_TRUE(aIndex >= 0 && (PRUint32) aIndex < mRowCount, NS_ERROR_ILLEGAL_VALUE);

  PRInt32 searchIndex;
  PRInt32 rowIndex;
  RowIndexToSearch(aIndex, &searchIndex, &rowIndex);
  NS_ENSURE_TRUE(searchIndex >= 0 && rowIndex >= 0, NS_ERROR_FAILURE);
  
  nsCOMPtr<nsIAutoCompleteResult> result;
  mResults->GetElementAt(searchIndex, getter_AddRefs(result));
  NS_ENSURE_TRUE(result != nsnull, NS_ERROR_FAILURE);
  
  PRUint16 searchResult;
  result->GetSearchResult(&searchResult);
  
  if (searchResult == nsIAutoCompleteResult::RESULT_FAILURE) {
    if (aValueOnly)
      return NS_ERROR_FAILURE;
    result->GetErrorDescription(_retval);
  } else if (searchResult == nsIAutoCompleteResult::RESULT_SUCCESS ||
             searchResult == nsIAutoCompleteResult::RESULT_SUCCESS_ONGOING) {
    result->GetValueAt(rowIndex, _retval);
  }
  
  return NS_OK;
}

/**
 * Given the index of a row in the autocomplete popup, find the
 * corresponding nsIAutoCompleteSearch index, and sub-index into 
 * the search's results list.
 */
nsresult
nsAutoCompleteController::RowIndexToSearch(PRInt32 aRowIndex, PRInt32 *aSearchIndex, PRInt32 *aItemIndex)
{
  *aSearchIndex = -1;
  *aItemIndex = -1;
  
  PRUint32 count;
  mSearches->Count(&count);
  PRUint32 index = 0;

  // Move index through the results of each registered nsIAutoCompleteSearch 
  // until we find the given row
  for (PRUint32 i = 0; i < count; ++i) {
    nsCOMPtr<nsIAutoCompleteResult> result;
    mResults->GetElementAt(i, getter_AddRefs(result));
    if (!result)
      continue;
      
    PRUint16 searchResult;
    result->GetSearchResult(&searchResult);
    
    // Find out how many results were provided by the 
    // current nsIAutoCompleteSearch
    PRUint32 rowCount = 0;
    if (searchResult == nsIAutoCompleteResult::RESULT_SUCCESS ||
        searchResult == nsIAutoCompleteResult::RESULT_SUCCESS_ONGOING) {
      result->GetMatchCount(&rowCount);
    }
    
    // If the given row index is within the results range 
    // of the current nsIAutoCompleteSearch then return the
    // search index and sub-index into the results array
    if ((rowCount != 0) && (index + rowCount-1 >= (PRUint32) aRowIndex)) {
      *aSearchIndex = i;
      *aItemIndex = aRowIndex - index;
      return NS_OK;
    }

    // Advance the popup table index cursor past the
    // results of the current search.
    index += rowCount;
  }

  return NS_OK;
}

NS_GENERIC_FACTORY_CONSTRUCTOR(nsAutoCompleteController)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsAutoCompleteSimpleResult)
NS_GENERIC_FACTORY_SINGLETON_CONSTRUCTOR(nsAutoCompleteSearchStringArray, nsAutoCompleteSearchStringArray::GetInstance)
#ifdef MOZ_MORK
NS_GENERIC_FACTORY_CONSTRUCTOR(nsAutoCompleteMdbResult)
#endif

static const nsModuleComponentInfo components[] =
{
  { "AutoComplete Controller",
    NS_AUTOCOMPLETECONTROLLER_CID, 
    NS_AUTOCOMPLETECONTROLLER_CONTRACTID,
    nsAutoCompleteControllerConstructor },

  { "AutoComplete Simple Result",
    NS_AUTOCOMPLETESIMPLERESULT_CID, 
    NS_AUTOCOMPLETESIMPLERESULT_CONTRACTID,
    nsAutoCompleteSimpleResultConstructor },

  { "String Array Autocomplete",
    NS_STRINGARRAYAUTOCOMPLETE_CID,
    NS_STRINGARRAYAUTOCOMPLETE_CONTRACTID,
    nsAutoCompleteSearchStringArrayConstructor },

#ifdef MOZ_MORK
  { "AutoComplete Mdb Result",
    NS_AUTOCOMPLETEMDBRESULT_CID, 
    NS_AUTOCOMPLETEMDBRESULT_CONTRACTID,
    nsAutoCompleteMdbResultConstructor },
#endif
};

NS_IMPL_NSGETMODULE(tkAutoCompleteModule, components)
