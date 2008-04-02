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
 * Portions created by the Initial Developer are Copyright (C) 1998-1999
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Pierre Phaneuf <pp@ludusdesign.com>
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

#include "IMETextTxn.h"
#include "nsIDOMCharacterData.h"
#include "nsIDOMRange.h"
#include "nsIPrivateTextRange.h"
#include "nsISelection.h"
#include "nsISelectionPrivate.h"
#include "nsISelectionController.h"
#include "nsComponentManagerUtils.h"

// #define DEBUG_IMETXN

IMETextTxn::IMETextTxn()
  : EditTxn()
{
}

NS_IMETHODIMP IMETextTxn::Init(nsIDOMCharacterData     *aElement,
                               PRUint32                 aOffset,
                               PRUint32                 aReplaceLength,
                               nsIPrivateTextRangeList *aTextRangeList,
                               const nsAString         &aStringToInsert,
                               nsWeakPtr                aSelConWeak)
{
  NS_ASSERTION(aElement, "illegal value- null ptr- aElement");
  NS_ASSERTION(aTextRangeList, "illegal value- null ptr - aTextRangeList");
  if (!aElement || !aTextRangeList)
     return NS_ERROR_NULL_POINTER;
  mElement = do_QueryInterface(aElement);
  mOffset = aOffset;
  mReplaceLength = aReplaceLength;
  mStringToInsert = aStringToInsert;
  mSelConWeak = aSelConWeak;
  mRangeList = do_QueryInterface(aTextRangeList);
  mFixed = PR_FALSE;
  return NS_OK;
}

NS_IMETHODIMP IMETextTxn::DoTransaction(void)
{

#ifdef DEBUG_IMETXN
  printf("Do IME Text element = %p replace = %d len = %d\n", mElement.get(), mReplaceLength, mStringToInsert.Length());
#endif

  nsCOMPtr<nsISelectionController> selCon = do_QueryReferent(mSelConWeak);
  if (!selCon) return NS_ERROR_NOT_INITIALIZED;

  // advance caret: This requires the presentation shell to get the selection.
  nsresult result;
  if (mReplaceLength == 0) {
    result = mElement->InsertData(mOffset, mStringToInsert);
  } else {
    result = mElement->ReplaceData(mOffset, mReplaceLength, mStringToInsert);
  }
  if (NS_SUCCEEDED(result)) {
    result = CollapseTextSelection();
  }

  return result;
}

NS_IMETHODIMP IMETextTxn::UndoTransaction(void)
{
#ifdef DEBUG_IMETXN
  printf("Undo IME Text element = %p\n", mElement.get());
#endif

  nsCOMPtr<nsISelectionController> selCon = do_QueryReferent(mSelConWeak);
  if (!selCon) return NS_ERROR_NOT_INITIALIZED;

  nsresult result = mElement->DeleteData(mOffset, mStringToInsert.Length());
  if (NS_SUCCEEDED(result))
  { // set the selection to the insertion point where the string was removed
    nsCOMPtr<nsISelection> selection;
    result = selCon->GetSelection(nsISelectionController::SELECTION_NORMAL, getter_AddRefs(selection));
    if (NS_SUCCEEDED(result) && selection) {
      result = selection->Collapse(mElement, mOffset);
      NS_ASSERTION((NS_SUCCEEDED(result)), "selection could not be collapsed after undo of IME insert.");
    }
  }
  return result;
}

NS_IMETHODIMP IMETextTxn::Merge(nsITransaction *aTransaction, PRBool *aDidMerge)
{
  NS_ASSERTION(aDidMerge, "illegal vaule- null ptr- aDidMerge");
  NS_ASSERTION(aTransaction, "illegal vaule- null ptr- aTransaction");
  if (!aDidMerge || !aTransaction)
    return NS_ERROR_NULL_POINTER;
    
#ifdef DEBUG_IMETXN
  printf("Merge IME Text element = %p\n", mElement.get());
#endif

  // 
  // check to make sure we aren't fixed, if we are then nothing get's absorbed
  //
  if (mFixed) {
    *aDidMerge = PR_FALSE;
    return NS_OK;
  }

  //
  // if aTransaction is another IMETextTxn then absorb it
  //
  IMETextTxn*  otherTxn = nsnull;
  nsresult result = aTransaction->QueryInterface(IMETextTxn::GetCID(),(void**)&otherTxn);
  if (otherTxn && NS_SUCCEEDED(result))
  {
    //
    //  we absorb the next IME transaction by adopting its insert string as our own
    //
    nsIPrivateTextRangeList* newTextRangeList;
    otherTxn->GetData(mStringToInsert,&newTextRangeList);
    mRangeList = do_QueryInterface(newTextRangeList);
    *aDidMerge = PR_TRUE;
#ifdef DEBUG_IMETXN
    printf("IMETextTxn assimilated IMETextTxn:%p\n", aTransaction);
#endif
    NS_RELEASE(otherTxn);
    return NS_OK;
  }

  *aDidMerge = PR_FALSE;
  return NS_OK;
}

NS_IMETHODIMP IMETextTxn::MarkFixed(void)
{
  mFixed = PR_TRUE;
  return NS_OK;
}

NS_IMETHODIMP IMETextTxn::GetTxnDescription(nsAString& aString)
{
  aString.AssignLiteral("IMETextTxn: ");
  aString += mStringToInsert;
  return NS_OK;
}

/* ============= nsISupports implementation ====================== */

NS_IMETHODIMP
IMETextTxn::QueryInterface(REFNSIID aIID, void** aInstancePtr)
{
  if (nsnull == aInstancePtr) {
    return NS_ERROR_NULL_POINTER;
  }
  if (aIID.Equals(IMETextTxn::GetCID())) {
    *aInstancePtr = (void*)(IMETextTxn*)this;
    NS_ADDREF_THIS();
    return NS_OK;
  }
  return (EditTxn::QueryInterface(aIID, aInstancePtr));
}

/* ============ protected methods ================== */
static SelectionType TextRangeToSelection(int aTextRangeType)
{
   switch(aTextRangeType)
   {
      case nsIPrivateTextRange::TEXTRANGE_RAWINPUT:
           return nsISelectionController::SELECTION_IME_RAWINPUT;
      case nsIPrivateTextRange::TEXTRANGE_SELECTEDRAWTEXT:
           return nsISelectionController::SELECTION_IME_SELECTEDRAWTEXT;
      case nsIPrivateTextRange::TEXTRANGE_CONVERTEDTEXT:
           return nsISelectionController::SELECTION_IME_CONVERTEDTEXT;
      case nsIPrivateTextRange::TEXTRANGE_SELECTEDCONVERTEDTEXT:
           return nsISelectionController::SELECTION_IME_SELECTEDCONVERTEDTEXT;
      case nsIPrivateTextRange::TEXTRANGE_CARETPOSITION:
      default:
           return nsISelectionController::SELECTION_NORMAL;
   };
}

NS_IMETHODIMP IMETextTxn::GetData(nsString& aResult,nsIPrivateTextRangeList** aTextRangeList)
{
  NS_ASSERTION(aTextRangeList, "illegal value- null ptr- aTextRangeList");
  if (!aTextRangeList)
    return NS_ERROR_NULL_POINTER;
  aResult = mStringToInsert;
  *aTextRangeList = mRangeList;
  return NS_OK;
}

static SelectionType sel[4]=
{
  nsISelectionController::SELECTION_IME_RAWINPUT,
  nsISelectionController::SELECTION_IME_SELECTEDRAWTEXT,
  nsISelectionController::SELECTION_IME_CONVERTEDTEXT,
  nsISelectionController::SELECTION_IME_SELECTEDCONVERTEDTEXT
};

NS_IMETHODIMP IMETextTxn::CollapseTextSelection(void)
{
    nsresult      result;
    PRUint16      i;

#ifdef DEBUG_IMETXN
    PRUint16 listlen,start,stop,type;
    result = mRangeList->GetLength(&listlen);
    printf("nsIPrivateTextRangeList[%p]\n",mRangeList);
    nsIPrivateTextRange* rangePtr;
    for (i=0;i<listlen;i++) {
      (void)mRangeList->Item(i,&rangePtr);
      rangePtr->GetRangeStart(&start);
      rangePtr->GetRangeEnd(&stop);
      rangePtr->GetRangeType(&type);
      printf("range[%d] start=%d end=%d type=",i,start,stop,type);
      if (type==nsIPrivateTextRange::TEXTRANGE_RAWINPUT)
                             printf("TEXTRANGE_RAWINPUT\n");
      else if (type==nsIPrivateTextRange::TEXTRANGE_SELECTEDRAWTEXT)
                                  printf("TEXTRANGE_SELECTEDRAWTEXT\n");
      else if (type==nsIPrivateTextRange::TEXTRANGE_CONVERTEDTEXT)
                                  printf("TEXTRANGE_CONVERTEDTEXT\n");
      else if (type==nsIPrivateTextRange::TEXTRANGE_SELECTEDCONVERTEDTEXT)
                                  printf("TEXTRANGE_SELECTEDCONVERTEDTEXT\n");
      else if (type==nsIPrivateTextRange::TEXTRANGE_CARETPOSITION)
                                  printf("TEXTRANGE_CARETPOSITION\n");
      else printf("unknown constant\n");
    }
#endif
        
    //
    // run through the text range list, if any
    //
    nsCOMPtr<nsISelectionController> selCon = do_QueryReferent(mSelConWeak);
    if (!selCon) return NS_ERROR_NOT_INITIALIZED;

    PRUint16      textRangeListLength,selectionStart,selectionEnd,
                  textRangeType;
    
    result = mRangeList->GetLength(&textRangeListLength);
    if(NS_FAILED(result))
        return result;
    nsCOMPtr<nsISelection> selection;
    result = selCon->GetSelection(nsISelectionController::SELECTION_NORMAL, getter_AddRefs(selection));
    if(NS_SUCCEEDED(result))
    {
      nsCOMPtr<nsISelectionPrivate> selPriv(do_QueryInterface(selection));
      result = selPriv->StartBatchChanges();
      if (NS_SUCCEEDED(result))
      {
        nsCOMPtr<nsISelection> imeSel;
        for(PRInt8 selIdx = 0; selIdx < 4;selIdx++)
        {
          result = selCon->GetSelection(sel[selIdx], getter_AddRefs(imeSel));
          if (NS_SUCCEEDED(result))
          {
            result = imeSel->RemoveAllRanges();
            NS_ASSERTION(NS_SUCCEEDED(result), "Cannot ClearSelection");
            // we just ignore the result and clean up the next one here
          }
        }

        nsCOMPtr<nsIPrivateTextRange> textRange;
        PRBool setCaret=PR_FALSE;
        for(i=0;i<textRangeListLength;i++)
        {
          result = mRangeList->Item(i, getter_AddRefs(textRange));
          NS_ASSERTION(NS_SUCCEEDED(result), "cannot get item");
          if(NS_FAILED(result))
               break;

          result = textRange->GetRangeType(&textRangeType);
          NS_ASSERTION(NS_SUCCEEDED(result), "cannot get range type");
          if(NS_FAILED(result))
               break;

          result = textRange->GetRangeStart(&selectionStart);
          NS_ASSERTION(NS_SUCCEEDED(result), "cannot get range start");
          if(NS_FAILED(result))
               break;
          result = textRange->GetRangeEnd(&selectionEnd);
          NS_ASSERTION(NS_SUCCEEDED(result), "cannot get range end");
          if(NS_FAILED(result))
               break;

          if(nsIPrivateTextRange::TEXTRANGE_CARETPOSITION == textRangeType)
          {
             // Set the caret....
             result = selection->Collapse(mElement,
                      mOffset+selectionStart);
             NS_ASSERTION(NS_SUCCEEDED(result), "Cannot Collapse");
             if(NS_SUCCEEDED(result))
             setCaret = PR_TRUE;
          } else {
             // NS_ASSERTION(selectionStart != selectionEnd, "end == start");
             if(selectionStart == selectionEnd)
                continue;

             result= selCon->GetSelection(TextRangeToSelection(textRangeType),
                     getter_AddRefs(imeSel));
             NS_ASSERTION(NS_SUCCEEDED(result), "Cannot get selction");
             if(NS_FAILED(result))
                break;

             nsCOMPtr<nsIDOMRange> newRange = do_CreateInstance(
                                      "@mozilla.org/content/range;1", &result);
             NS_ASSERTION(NS_SUCCEEDED(result), "Cannot create new nsIDOMRange");
             if(NS_FAILED(result))
                break;

             newRange->SetStart(mElement,mOffset+selectionStart);
             NS_ASSERTION(NS_SUCCEEDED(result), "Cannot SetStart");
             if(NS_FAILED(result))
                break;

             newRange->SetEnd(mElement,mOffset+selectionEnd);
             NS_ASSERTION(NS_SUCCEEDED(result), "Cannot SetEnd");
             if(NS_FAILED(result))
                break;

             imeSel->AddRange(newRange);
             NS_ASSERTION(NS_SUCCEEDED(result), "Cannot AddRange");
             if(NS_FAILED(result))
                break;

          } // if GetRangeEnd
        } // for textRangeListLength
        if(! setCaret) {
          // set cursor
          result = selection->Collapse(mElement,mOffset+mStringToInsert.Length());
          NS_ASSERTION(NS_SUCCEEDED(result), "Cannot Collapse");
        }
        result = selPriv->EndBatchChanges();
        NS_ASSERTION(NS_SUCCEEDED(result), "Cannot EndBatchChanges");
      } // if StartBatchChanges
    } // if GetSelection

    return result;
}

