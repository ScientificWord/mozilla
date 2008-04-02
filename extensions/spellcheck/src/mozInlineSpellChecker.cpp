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
 * The Original Code is Real-time Spellchecking
 *
 * The Initial Developer of the Original Code is Mozdev Group, Inc.
 * Portions created by the Initial Developer are Copyright (C) 2004
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s): Neil Deakin (neil@mozdevgroup.com)
 *                 Scott MacGregor (mscott@mozilla.org)
 *                 Brett Wilson <brettw@gmail.com>
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

/**
 * This class is called by the editor to handle spellchecking after various
 * events. The main entrypoint is SpellCheckAfterEditorChange, which is called
 * when the text is changed.
 *
 * It is VERY IMPORTANT that we do NOT do any operations that might cause DOM
 * notifications to be flushed when we are called from the editor. This is
 * because the call might originate from a frame, and flushing the
 * notifications might cause that frame to be deleted.
 *
 * Using the WordUtil class to find words causes DOM notifications to be
 * flushed because it asks for style information. As a result, we post an event
 * and do all of the spellchecking in that event handler, which occurs later.
 * We store all DOM pointers in ranges because they are kept up-to-date with
 * DOM changes that may have happened while the event was on the queue.
 *
 * We also allow the spellcheck to be suspended and resumed later. This makes
 * large pastes or initializations with a lot of text not hang the browser UI.
 *
 * An optimization is the mNeedsCheckAfterNavigation flag. This is set to
 * true when we get any change, and false once there is no possibility
 * something changed that we need to check on navigation. Navigation events
 * tend to be a little tricky because we want to check the current word on
 * exit if something has changed. If we navigate inside the word, we don't want
 * to do anything. As a result, this flag is cleared in FinishNavigationEvent
 * when we know that we are checking as a result of navigation.
 */

#include "mozInlineSpellChecker.h"
#include "mozInlineSpellWordUtil.h"
#include "mozISpellI18NManager.h"
#include "nsCOMPtr.h"
#include "nsCRT.h"
#include "nsIDocument.h"
#include "nsIDOMDocument.h"
#include "nsIDOMDocumentRange.h"
#include "nsIDOMElement.h"
#include "nsIDOMEventTarget.h"
#include "nsPIDOMEventTarget.h"
#include "nsIDOMMouseEvent.h"
#include "nsIDOMKeyEvent.h"
#include "nsIDOMNode.h"
#include "nsIDOMNodeList.h"
#include "nsIDOMNSRange.h"
#include "nsIDOMRange.h"
#include "nsIDOMText.h"
#include "nsIPlaintextEditor.h"
#include "nsIPrefBranch.h"
#include "nsIPrefService.h"
#include "nsIRunnable.h"
#include "nsISelection.h"
#include "nsISelection2.h"
#include "nsISelectionController.h"
#include "nsIServiceManager.h"
#include "nsITextServicesFilter.h"
#include "nsString.h"
#include "nsThreadUtils.h"
#include "nsUnicharUtils.h"
#include "nsIContent.h"
#include "nsIEventStateManager.h"
#include "nsIEventListenerManager.h"
#include "nsGUIEvent.h"

// Set to spew messages to the console about what is happening.
//#define DEBUG_INLINESPELL

// the number of milliseconds that we will take at once to do spellchecking
#define INLINESPELL_CHECK_TIMEOUT 50

// The number of words to check before we look at the time to see if
// INLINESPELL_CHECK_TIMEOUT ms have elapsed. This prevents us from spending
// too much time checking the clock. Note that misspelled words count for
// more than one word in this calculation.
#define INLINESPELL_TIMEOUT_CHECK_FREQUENCY 50

// This number is the number of checked words a misspelled word counts for
// when we're checking the time to see if the alloted time is up for
// spellchecking. Misspelled words take longer to process since we have to
// create a range, so they count more. The exact number isn't very important
// since this just controls how often we check the current time.
#define MISSPELLED_WORD_COUNT_PENALTY 4


static PRBool ContentIsDescendantOf(nsINode* aPossibleDescendant,
                                    nsINode* aPossibleAncestor);

static const char kMaxSpellCheckSelectionSize[] = "extensions.spellcheck.inline.max-misspellings";

mozInlineSpellStatus::mozInlineSpellStatus(mozInlineSpellChecker* aSpellChecker)
    : mSpellChecker(aSpellChecker), mWordCount(0)
{
}

// mozInlineSpellStatus::InitForEditorChange
//
//    This is the most complicated case. For changes, we need to compute the
//    range of stuff that changed based on the old and new caret positions,
//    as well as use a range possibly provided by the editor (start and end,
//    which are usually NULL) to get a range with the union of these.

nsresult
mozInlineSpellStatus::InitForEditorChange(
    PRInt32 aAction,
    nsIDOMNode* aAnchorNode, PRInt32 aAnchorOffset,
    nsIDOMNode* aPreviousNode, PRInt32 aPreviousOffset,
    nsIDOMNode* aStartNode, PRInt32 aStartOffset,
    nsIDOMNode* aEndNode, PRInt32 aEndOffset)
{
  nsresult rv;

  nsCOMPtr<nsIDOMDocumentRange> docRange;
  rv = GetDocumentRange(getter_AddRefs(docRange));
  NS_ENSURE_SUCCESS(rv, rv);

  // save the anchor point as a range so we can find the current word later
  rv = PositionToCollapsedRange(docRange, aAnchorNode, aAnchorOffset,
                                getter_AddRefs(mAnchorRange));
  NS_ENSURE_SUCCESS(rv, rv);

  if (aAction == mozInlineSpellChecker::kOpDeleteSelection) {
    // Deletes are easy, the range is just the current anchor. We set the range
    // to check to be empty, FinishInitOnEvent will fill in the range to be
    // the current word.
    mOp = eOpChangeDelete;
    mRange = nsnull;
    return NS_OK;
  }

  mOp = eOpChange;

  // range to check
  rv = docRange->CreateRange(getter_AddRefs(mRange));
  NS_ENSURE_SUCCESS(rv, rv);

  // ...we need to put the start and end in the correct order
  nsCOMPtr<nsIDOMNSRange> nsrange = do_QueryInterface(mAnchorRange, &rv);
  NS_ENSURE_SUCCESS(rv, rv);
  PRInt16 cmpResult;
  rv = nsrange->ComparePoint(aPreviousNode, aPreviousOffset, &cmpResult);
  NS_ENSURE_SUCCESS(rv, rv);
  if (cmpResult < 0) {
    // previous anchor node is before the current anchor
    rv = mRange->SetStart(aPreviousNode, aPreviousOffset);
    NS_ENSURE_SUCCESS(rv, rv);
    rv = mRange->SetEnd(aAnchorNode, aAnchorOffset);
  } else {
    // previous anchor node is after (or the same as) the current anchor
    rv = mRange->SetStart(aAnchorNode, aAnchorOffset);
    NS_ENSURE_SUCCESS(rv, rv);
    rv = mRange->SetEnd(aPreviousNode, aPreviousOffset);
  }
  NS_ENSURE_SUCCESS(rv, rv);

  // On insert save this range: DoSpellCheck optimizes things in this range.
  // Otherwise, just leave this NULL.
  if (aAction == mozInlineSpellChecker::kOpInsertText)
    mCreatedRange = mRange;

  // if we were given a range, we need to expand our range to encompass it
  if (aStartNode && aEndNode) {
    nsrange = do_QueryInterface(mRange, &rv);
    NS_ENSURE_SUCCESS(rv, rv);

    rv = nsrange->ComparePoint(aStartNode, aStartOffset, &cmpResult);
    NS_ENSURE_SUCCESS(rv, rv);
    if (cmpResult < 0) { // given range starts before
      rv = mRange->SetStart(aStartNode, aStartOffset);
      NS_ENSURE_SUCCESS(rv, rv);
    }

    rv = nsrange->ComparePoint(aEndNode, aEndOffset, &cmpResult);
    NS_ENSURE_SUCCESS(rv, rv);
    if (cmpResult > 0) { // given range ends after
      rv = mRange->SetEnd(aEndNode, aEndOffset);
      NS_ENSURE_SUCCESS(rv, rv);
    }
  }

  return NS_OK;
}

// mozInlineSpellStatis::InitForNavigation
//
//    For navigation events, we just need to store the new and old positions.
//
//    In some cases, we detect that we shouldn't check. If this event should
//    not be processed, *aContinue will be false.

nsresult
mozInlineSpellStatus::InitForNavigation(
    PRBool aForceCheck, PRInt32 aNewPositionOffset,
    nsIDOMNode* aOldAnchorNode, PRInt32 aOldAnchorOffset,
    nsIDOMNode* aNewAnchorNode, PRInt32 aNewAnchorOffset,
    PRBool* aContinue)
{
  nsresult rv;
  mOp = eOpNavigation;

  mForceNavigationWordCheck = aForceCheck;
  mNewNavigationPositionOffset = aNewPositionOffset;

  // get the root node for checking
  nsCOMPtr<nsIEditor> editor = do_QueryReferent(mSpellChecker->mEditor, &rv);
  NS_ENSURE_SUCCESS(rv, rv);
  nsCOMPtr<nsIDOMElement> rootElt;
  rv = editor->GetRootElement(getter_AddRefs(rootElt));
  NS_ENSURE_SUCCESS(rv, rv);

  // the anchor node might not be in the DOM anymore, check
  nsCOMPtr<nsINode> root = do_QueryInterface(rootElt, &rv);
  NS_ENSURE_SUCCESS(rv, rv);
  nsCOMPtr<nsINode> currentAnchor = do_QueryInterface(aOldAnchorNode, &rv);
  NS_ENSURE_SUCCESS(rv, rv);
  if (root && currentAnchor && ! ContentIsDescendantOf(currentAnchor, root)) {
    *aContinue = PR_FALSE;
    return NS_OK;
  }

  nsCOMPtr<nsIDOMDocumentRange> docRange;
  rv = GetDocumentRange(getter_AddRefs(docRange));
  NS_ENSURE_SUCCESS(rv, rv);

  rv = PositionToCollapsedRange(docRange, aOldAnchorNode, aOldAnchorOffset,
                                getter_AddRefs(mOldNavigationAnchorRange));
  NS_ENSURE_SUCCESS(rv, rv);
  rv = PositionToCollapsedRange(docRange, aNewAnchorNode, aNewAnchorOffset,
                                getter_AddRefs(mAnchorRange));
  NS_ENSURE_SUCCESS(rv, rv);

  *aContinue = PR_TRUE;
  return NS_OK;
}

// mozInlineSpellStatus::InitForSelection
//
//    It is easy for selections since we always re-check the spellcheck
//    selection.

nsresult
mozInlineSpellStatus::InitForSelection()
{
  mOp = eOpSelection;
  return NS_OK;
}

// mozInlineSpellStatus::InitForRange
//
//    Called to cause the spellcheck of the given range. This will look like
//    a change operation over the given range.

nsresult
mozInlineSpellStatus::InitForRange(nsIDOMRange* aRange)
{
  mOp = eOpChange;
  mRange = aRange;
  return NS_OK;
}

// mozInlineSpellStatus::FinishInitOnEvent
//
//    Called when the event is triggered to complete initialization that
//    might require the WordUtil. This calls to the operation-specific
//    initializer, and also sets the range to be the entire element if it
//    is NULL.
//
//    Watch out: the range might still be NULL if there is nothing to do,
//    the caller will have to check for this.

nsresult
mozInlineSpellStatus::FinishInitOnEvent(mozInlineSpellWordUtil& aWordUtil)
{
  nsresult rv;
  if (! mRange) {
    rv = mSpellChecker->MakeSpellCheckRange(nsnull, 0, nsnull, 0,
                                            getter_AddRefs(mRange));
    NS_ENSURE_SUCCESS(rv, rv);
  }

  switch (mOp) {
    case eOpChange:
      if (mAnchorRange)
        return FillNoCheckRangeFromAnchor(aWordUtil);
      break;
    case eOpChangeDelete:
      if (mAnchorRange) {
        rv = FillNoCheckRangeFromAnchor(aWordUtil);
        NS_ENSURE_SUCCESS(rv, rv);
      }
      // Delete events will have no range for the changed text (because it was
      // deleted), and InitForEditorChange will set it to NULL. Here, we select
      // the entire word to cause any underlining to be removed.
      mRange = mNoCheckRange;
      break;
    case eOpNavigation:
      return FinishNavigationEvent(aWordUtil);
    case eOpSelection:
      // this gets special handling in ResumeCheck
      break;
    case eOpResume:
      // everything should be initialized already in this case
      break;
    default:
      NS_NOTREACHED("Bad operation");
      return NS_ERROR_NOT_INITIALIZED;
  }
  return NS_OK;
}

// mozInlineSpellStatus::FinishNavigationEvent
//
//    This verifies that we need to check the word at the previous caret
//    position. Now that we have the word util, we can find the word belonging
//    to the previous caret position. If the new position is inside that word,
//    we don't want to do anything. In this case, we'll NULL out mRange so
//    that the caller will know not to continue.
//
//    Notice that we don't set mNoCheckRange. We check here whether the cursor
//    is in the word that needs checking, so it isn't necessary. Plus, the
//    spellchecker isn't guaranteed to only check the given word, and it could
//    remove the underline from the new word under the cursor.

nsresult
mozInlineSpellStatus::FinishNavigationEvent(mozInlineSpellWordUtil& aWordUtil)
{
  NS_ASSERTION(mAnchorRange, "No anchor for navigation!");
  nsCOMPtr<nsIDOMNode> newAnchorNode, oldAnchorNode;
  PRInt32 newAnchorOffset, oldAnchorOffset;

  // get the DOM position of the old caret, the range should be collapsed
  nsresult rv = mOldNavigationAnchorRange->GetStartContainer(
      getter_AddRefs(oldAnchorNode));
  NS_ENSURE_SUCCESS(rv, rv);
  rv = mOldNavigationAnchorRange->GetStartOffset(&oldAnchorOffset);
  NS_ENSURE_SUCCESS(rv, rv);

  // find the word on the old caret position, this is the one that we MAY need
  // to check
  nsCOMPtr<nsIDOMRange> oldWord;
  rv = aWordUtil.GetRangeForWord(oldAnchorNode, oldAnchorOffset,
                                 getter_AddRefs(oldWord));
  NS_ENSURE_SUCCESS(rv, rv);
  nsCOMPtr<nsIDOMNSRange> oldWordNS = do_QueryInterface(oldWord, &rv);
  NS_ENSURE_SUCCESS(rv, rv);

  // get the DOM position of the new caret, the range should be collapsed
  rv = mAnchorRange->GetStartContainer(getter_AddRefs(newAnchorNode));
  NS_ENSURE_SUCCESS(rv, rv);
  rv = mAnchorRange->GetStartOffset(&newAnchorOffset);
  NS_ENSURE_SUCCESS(rv, rv);

  // see if the new cursor position is in the word of the old cursor position
  PRBool isInRange = PR_FALSE;
  if (! mForceNavigationWordCheck) {
    rv = oldWordNS->IsPointInRange(newAnchorNode,
                                   newAnchorOffset + mNewNavigationPositionOffset,
                                   &isInRange);
    NS_ENSURE_SUCCESS(rv, rv);
  }

  if (isInRange) {
    // caller should give up
    mRange = nsnull;
  } else {
    // check the old word
    mRange = oldWord;

    // Once we've spellchecked the current word, we don't need to spellcheck
    // for any more navigation events.
    mSpellChecker->mNeedsCheckAfterNavigation = PR_FALSE;
  }
  return NS_OK;
}

// mozInlineSpellStatus::FillNoCheckRangeFromAnchor
//
//    Given the mAnchorRange object, computes the range of the word it is on
//    (if any) and fills that range into mNoCheckRange. This is used for
//    change and navigation events to know which word we should skip spell
//    checking on

nsresult
mozInlineSpellStatus::FillNoCheckRangeFromAnchor(
    mozInlineSpellWordUtil& aWordUtil)
{
  nsCOMPtr<nsIDOMNode> anchorNode;
  nsresult rv = mAnchorRange->GetStartContainer(getter_AddRefs(anchorNode));
  NS_ENSURE_SUCCESS(rv, rv);

  PRInt32 anchorOffset;
  rv = mAnchorRange->GetStartOffset(&anchorOffset);
  NS_ENSURE_SUCCESS(rv, rv);

  return aWordUtil.GetRangeForWord(anchorNode, anchorOffset,
                                   getter_AddRefs(mNoCheckRange));
}

// mozInlineSpellStatus::GetDocumentRange
//
//    Returns the nsIDOMDocumentRange object for the document for the
//    current spellchecker.

nsresult
mozInlineSpellStatus::GetDocumentRange(nsIDOMDocumentRange** aDocRange)
{
  nsresult rv;
  *aDocRange = nsnull;
  if (! mSpellChecker->mEditor)
    return NS_ERROR_UNEXPECTED;

  nsCOMPtr<nsIEditor> editor = do_QueryReferent(mSpellChecker->mEditor, &rv);
  NS_ENSURE_SUCCESS(rv, rv);

  nsCOMPtr<nsIDOMDocument> domDoc;
  rv = editor->GetDocument(getter_AddRefs(domDoc));
  NS_ENSURE_SUCCESS(rv, rv);

  nsCOMPtr<nsIDOMDocumentRange> docRange = do_QueryInterface(domDoc, &rv);
  NS_ENSURE_SUCCESS(rv, rv);

  docRange.swap(*aDocRange);
  return NS_OK;
}

// mozInlineSpellStatus::PositionToCollapsedRange
//
//    Converts a given DOM position to a collapsed range covering that
//    position. We use ranges to store DOM positions becuase they stay
//    updated as the DOM is changed.

nsresult
mozInlineSpellStatus::PositionToCollapsedRange(nsIDOMDocumentRange* aDocRange,
    nsIDOMNode* aNode, PRInt32 aOffset, nsIDOMRange** aRange)
{
  *aRange = nsnull;
  nsCOMPtr<nsIDOMRange> range;
  nsresult rv = aDocRange->CreateRange(getter_AddRefs(range));
  NS_ENSURE_SUCCESS(rv, rv);

  rv = range->SetStart(aNode, aOffset);
  NS_ENSURE_SUCCESS(rv, rv);
  rv = range->SetEnd(aNode, aOffset);
  NS_ENSURE_SUCCESS(rv, rv);

  range.swap(*aRange);
  return NS_OK;
}

// mozInlineSpellResume

class mozInlineSpellResume : public nsRunnable
{
public:
  mozInlineSpellResume(const mozInlineSpellStatus& aStatus) : mStatus(aStatus) {}
  mozInlineSpellStatus mStatus;
  nsresult Post()
  {
    return NS_DispatchToMainThread(this);
  }

  NS_IMETHOD Run()
  {
    mStatus.mSpellChecker->ResumeCheck(&mStatus);
    return NS_OK;
  }
};


NS_INTERFACE_MAP_BEGIN(mozInlineSpellChecker)
NS_INTERFACE_MAP_ENTRY(nsIInlineSpellChecker)
NS_INTERFACE_MAP_ENTRY(nsIEditActionListener)
NS_INTERFACE_MAP_ENTRY(nsIDOMFocusListener)
NS_INTERFACE_MAP_ENTRY(nsIDOMMouseListener)
NS_INTERFACE_MAP_ENTRY(nsISupportsWeakReference)
NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(nsISupports, nsIDOMKeyListener)
NS_INTERFACE_MAP_ENTRY(nsIDOMKeyListener)
NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(nsIDOMEventListener, nsIDOMKeyListener)
NS_INTERFACE_MAP_END

NS_IMPL_ADDREF(mozInlineSpellChecker)
NS_IMPL_RELEASE(mozInlineSpellChecker)

mozInlineSpellChecker::SpellCheckingState
  mozInlineSpellChecker::gCanEnableSpellChecking =
  mozInlineSpellChecker::SpellCheck_Uninitialized;

mozInlineSpellChecker::mozInlineSpellChecker() :
    mNumWordsInSpellSelection(0),
    mMaxNumWordsInSpellSelection(250),
    mNeedsCheckAfterNavigation(PR_FALSE)
{
  nsCOMPtr<nsIPrefBranch> prefs = do_GetService(NS_PREFSERVICE_CONTRACTID);
  if (prefs)
    prefs->GetIntPref(kMaxSpellCheckSelectionSize, &mMaxNumWordsInSpellSelection); 
  mMaxMisspellingsPerCheck = mMaxNumWordsInSpellSelection * 3 / 4;
}

mozInlineSpellChecker::~mozInlineSpellChecker()
{
}

NS_IMETHODIMP
mozInlineSpellChecker::GetSpellChecker(nsIEditorSpellCheck **aSpellCheck)
{
  *aSpellCheck = mSpellCheck;
  NS_IF_ADDREF(*aSpellCheck);
  return NS_OK;
}

NS_IMETHODIMP
mozInlineSpellChecker::Init(nsIEditor *aEditor)
{
  mEditor = do_GetWeakReference(aEditor);
  return NS_OK;
}

// mozInlineSpellChecker::Cleanup
//
//    Called by the editor when the editor is going away. This is important
//    because we remove listeners. We do NOT clean up anything else in this
//    function, because it can get called while DoSpellCheck is running!
//
//    Getting the style information there can cause DOM notifications to be
//    flushed, which can cause editors to go away which will bring us here.
//    We can not do anything that will cause DoSpellCheck to freak out.

nsresult mozInlineSpellChecker::Cleanup()
{
  mNumWordsInSpellSelection = 0;
  nsCOMPtr<nsISelection> spellCheckSelection;
  nsresult rv = GetSpellCheckSelection(getter_AddRefs(spellCheckSelection));
  if (NS_FAILED(rv)) {
    // Ensure we still unregister event listeners (but return a failure code)
    UnregisterEventListeners();
  } else {
    spellCheckSelection->RemoveAllRanges();

    rv = UnregisterEventListeners();
  }

  return rv;
}

// mozInlineSpellChecker::CanEnableInlineSpellChecking
//
//    This function can be called to see if it seems likely that we can enable
//    spellchecking before actually creating the InlineSpellChecking objects.
//
//    The problem is that we can't get the dictionary list without actually
//    creating a whole bunch of spellchecking objects. This function tries to
//    do that and caches the result so we don't have to keep allocating those
//    objects if there are no dictionaries or spellchecking.
//
//    This caching will prevent adding dictionaries at runtime if we start out
//    with no dictionaries! Installing dictionaries as extensions will require
//    a restart anyway, so it shouldn't be a problem.

PRBool // static
mozInlineSpellChecker::CanEnableInlineSpellChecking()
{
  nsresult rv;
  if (gCanEnableSpellChecking == SpellCheck_Uninitialized) {
    gCanEnableSpellChecking = SpellCheck_NotAvailable;

    nsCOMPtr<nsIEditorSpellCheck> spellchecker =
      do_CreateInstance("@mozilla.org/editor/editorspellchecker;1", &rv);
    NS_ENSURE_SUCCESS(rv, PR_FALSE);

    PRBool canSpellCheck = PR_FALSE;
    rv = spellchecker->CanSpellCheck(&canSpellCheck);
    NS_ENSURE_SUCCESS(rv, PR_FALSE);

    if (canSpellCheck)
      gCanEnableSpellChecking = SpellCheck_Available;
  }
  return (gCanEnableSpellChecking == SpellCheck_Available);
}

// mozInlineSpellChecker::RegisterEventListeners
//
//    The inline spell checker listens to mouse events and keyboard navigation+ //    events.

nsresult
mozInlineSpellChecker::RegisterEventListeners()
{
  nsCOMPtr<nsIEditor> editor (do_QueryReferent(mEditor));
  NS_ENSURE_TRUE(editor, NS_ERROR_NULL_POINTER);

  editor->AddEditActionListener(this);

  nsCOMPtr<nsIDOMDocument> doc;
  nsresult rv = editor->GetDocument(getter_AddRefs(doc));
  NS_ENSURE_SUCCESS(rv, rv);

  nsCOMPtr<nsPIDOMEventTarget> piTarget = do_QueryInterface(doc, &rv);
  NS_ENSURE_SUCCESS(rv, rv);

  nsCOMPtr<nsIEventListenerManager> elmP;
  piTarget->GetListenerManager(PR_TRUE, getter_AddRefs(elmP));
  if (elmP) {
    // Focus event doesn't bubble so adding the listener to capturing phase
    elmP->AddEventListenerByIID(static_cast<nsIDOMFocusListener *>(this),
                                NS_GET_IID(nsIDOMFocusListener),
                                NS_EVENT_FLAG_CAPTURE);
  }

  piTarget->AddEventListenerByIID(static_cast<nsIDOMMouseListener*>(this),
                                  NS_GET_IID(nsIDOMMouseListener));
  piTarget->AddEventListenerByIID(static_cast<nsIDOMKeyListener*>(this),
                                  NS_GET_IID(nsIDOMKeyListener));

  return NS_OK;
}

// mozInlineSpellChecker::UnregisterEventListeners

nsresult
mozInlineSpellChecker::UnregisterEventListeners()
{
  nsCOMPtr<nsIEditor> editor (do_QueryReferent(mEditor));
  NS_ENSURE_TRUE(editor, NS_ERROR_NULL_POINTER);

  editor->RemoveEditActionListener(this);

  nsCOMPtr<nsIDOMDocument> doc;
  editor->GetDocument(getter_AddRefs(doc));
  NS_ENSURE_TRUE(doc, NS_ERROR_NULL_POINTER);
  
  nsCOMPtr<nsPIDOMEventTarget> piTarget = do_QueryInterface(doc);
  NS_ENSURE_TRUE(piTarget, NS_ERROR_NULL_POINTER);

  nsCOMPtr<nsIEventListenerManager> elmP;
  piTarget->GetListenerManager(PR_TRUE, getter_AddRefs(elmP));
  if (elmP) {
    elmP->RemoveEventListenerByIID(static_cast<nsIDOMFocusListener *>(this),
                                   NS_GET_IID(nsIDOMFocusListener),
                                   NS_EVENT_FLAG_CAPTURE);
  }

  piTarget->RemoveEventListenerByIID(static_cast<nsIDOMMouseListener*>(this),
                                     NS_GET_IID(nsIDOMMouseListener));
  piTarget->RemoveEventListenerByIID(static_cast<nsIDOMKeyListener*>(this),
                                     NS_GET_IID(nsIDOMKeyListener));
  
  return NS_OK;
}

// mozInlineSpellChecker::GetEnableRealTimeSpell

NS_IMETHODIMP
mozInlineSpellChecker::GetEnableRealTimeSpell(PRBool* aEnabled)
{
  NS_ENSURE_ARG_POINTER(aEnabled);
  *aEnabled = mSpellCheck != nsnull;
  return NS_OK;
}

// mozInlineSpellChecker::SetEnableRealTimeSpell

NS_IMETHODIMP
mozInlineSpellChecker::SetEnableRealTimeSpell(PRBool aEnabled)
{
  if (!aEnabled) {
    mSpellCheck = nsnull;
    return Cleanup();
  }

  if (!mSpellCheck) {
    nsresult res = NS_OK;
    nsCOMPtr<nsIEditorSpellCheck> spellchecker = do_CreateInstance("@mozilla.org/editor/editorspellchecker;1", &res);
    if (NS_SUCCEEDED(res) && spellchecker)
    {
      nsCOMPtr<nsITextServicesFilter> filter = do_CreateInstance("@mozilla.org/editor/txtsrvfiltermail;1", &res);
      spellchecker->SetFilter(filter);
      nsCOMPtr<nsIEditor> editor (do_QueryReferent(mEditor));
      res = spellchecker->InitSpellChecker(editor, PR_FALSE);
      NS_ENSURE_SUCCESS(res, res);

      nsCOMPtr<nsITextServicesDocument> tsDoc = do_CreateInstance("@mozilla.org/textservices/textservicesdocument;1", &res);
      NS_ENSURE_SUCCESS(res, res);

      res = tsDoc->SetFilter(filter);
      NS_ENSURE_SUCCESS(res, res);

      res = tsDoc->InitWithEditor(editor);
      NS_ENSURE_SUCCESS(res, res);

      mTextServicesDocument = tsDoc;
      mSpellCheck = spellchecker;

      // spell checking is enabled, register our event listeners to track navigation
      RegisterEventListeners();
    }
  }

  // spellcheck the current contents. SpellCheckRange doesn't supply a created
  // range to DoSpellCheck, which in our case is the entire range. But this
  // optimization doesn't matter because there is nothing in the spellcheck
  // selection when starting, which triggers a better optimization.
  return SpellCheckRange(nsnull);
}

// mozInlineSpellChecker::SpellCheckAfterEditorChange
//
//    Called by the editor when nearly anything happens to change the content.
//
//    The start and end positions specify a range for the thing that happened,
//    but these are usually NULL, even when you'd think they would be useful
//    because you want the range (for example, pasting). We ignore them in
//    this case.

NS_IMETHODIMP
mozInlineSpellChecker::SpellCheckAfterEditorChange(
    PRInt32 aAction, nsISelection *aSelection,
    nsIDOMNode *aPreviousSelectedNode, PRInt32 aPreviousSelectedOffset,
    nsIDOMNode *aStartNode, PRInt32 aStartOffset,
    nsIDOMNode *aEndNode, PRInt32 aEndOffset)
{
  nsresult rv;
  NS_ENSURE_ARG_POINTER(aSelection);
  if (!mSpellCheck)
    return NS_OK; // disabling spell checking is not an error

  // this means something has changed, and we never check the current word,
  // therefore, we should spellcheck for subsequent caret navigations
  mNeedsCheckAfterNavigation = PR_TRUE;

  // the anchor node is the position of the caret
  nsCOMPtr<nsIDOMNode> anchorNode;
  rv = aSelection->GetAnchorNode(getter_AddRefs(anchorNode));
  NS_ENSURE_SUCCESS(rv, rv);
  PRInt32 anchorOffset;
  rv = aSelection->GetAnchorOffset(&anchorOffset);
  NS_ENSURE_SUCCESS(rv, rv);

  mozInlineSpellStatus status(this);
  rv = status.InitForEditorChange(aAction,
                                  anchorNode, anchorOffset,
                                  aPreviousSelectedNode, aPreviousSelectedOffset,
                                  aStartNode, aStartOffset,
                                  aEndNode, aEndOffset);
  NS_ENSURE_SUCCESS(rv, rv);
  rv = ScheduleSpellCheck(status);
  NS_ENSURE_SUCCESS(rv, rv);

  // remember the current caret position after every change
  SaveCurrentSelectionPosition();
  return NS_OK;
}

// mozInlineSpellChecker::SpellCheckRange
//
//    Spellchecks all the words in the given range.
//    Supply a NULL range and this will check the entire editor.

nsresult
mozInlineSpellChecker::SpellCheckRange(nsIDOMRange* aRange)
{
  NS_ENSURE_TRUE(mSpellCheck, NS_ERROR_NOT_INITIALIZED);

  mozInlineSpellStatus status(this);
  nsresult rv = status.InitForRange(aRange);
  NS_ENSURE_SUCCESS(rv, rv);
  return ScheduleSpellCheck(status);
}

// mozInlineSpellChecker::GetMispelledWord

NS_IMETHODIMP
mozInlineSpellChecker::GetMispelledWord(nsIDOMNode *aNode, PRInt32 aOffset,
                                        nsIDOMRange **newword)
{
  NS_ENSURE_ARG_POINTER(aNode);
  nsCOMPtr<nsISelection> spellCheckSelection;
  nsresult res = GetSpellCheckSelection(getter_AddRefs(spellCheckSelection));
  NS_ENSURE_SUCCESS(res, res); 

  return IsPointInSelection(spellCheckSelection, aNode, aOffset, newword);
}

// mozInlineSpellChecker::ReplaceWord

NS_IMETHODIMP
mozInlineSpellChecker::ReplaceWord(nsIDOMNode *aNode, PRInt32 aOffset,
                                   const nsAString &newword)
{
  nsCOMPtr<nsIEditor> editor (do_QueryReferent(mEditor));
  NS_ENSURE_TRUE(editor, NS_ERROR_NULL_POINTER);
  NS_ENSURE_TRUE(newword.Length() != 0, NS_ERROR_FAILURE);

  nsCOMPtr<nsIDOMRange> range;
  nsresult res = GetMispelledWord(aNode, aOffset, getter_AddRefs(range));
  NS_ENSURE_SUCCESS(res, res); 

  if (range)
  {
    editor->BeginTransaction();
  
    nsCOMPtr<nsISelection> selection;
    res = editor->GetSelection(getter_AddRefs(selection));
    NS_ENSURE_SUCCESS(res, res);
    selection->RemoveAllRanges();
    selection->AddRange(range);
    editor->DeleteSelection(nsIEditor::eNone);

    nsCOMPtr<nsIPlaintextEditor> textEditor(do_QueryReferent(mEditor));
    textEditor->InsertText(newword);

    editor->EndTransaction();
  }

  return NS_OK;
}

// mozInlineSpellChecker::AddWordToDictionary

NS_IMETHODIMP
mozInlineSpellChecker::AddWordToDictionary(const nsAString &word)
{
  NS_ENSURE_TRUE(mSpellCheck, NS_ERROR_NOT_INITIALIZED);

  nsAutoString wordstr(word);
  nsresult rv = mSpellCheck->AddWordToDictionary(wordstr.get());
  NS_ENSURE_SUCCESS(rv, rv); 

  mozInlineSpellStatus status(this);
  rv = status.InitForSelection();
  NS_ENSURE_SUCCESS(rv, rv);
  return ScheduleSpellCheck(status);
}

// mozInlineSpellChecker::IgnoreWord

NS_IMETHODIMP
mozInlineSpellChecker::IgnoreWord(const nsAString &word)
{
  NS_ENSURE_TRUE(mSpellCheck, NS_ERROR_NOT_INITIALIZED);

  nsAutoString wordstr(word);
  nsresult rv = mSpellCheck->IgnoreWordAllOccurrences(wordstr.get());
  NS_ENSURE_SUCCESS(rv, rv); 

  mozInlineSpellStatus status(this);
  rv = status.InitForSelection();
  NS_ENSURE_SUCCESS(rv, rv);
  return ScheduleSpellCheck(status);
}

// mozInlineSpellChecker::IgnoreWords

NS_IMETHODIMP
mozInlineSpellChecker::IgnoreWords(const PRUnichar **aWordsToIgnore,
                                   PRUint32 aCount)
{
  // add each word to the ignore list and then recheck the document
  for (PRUint32 index = 0; index < aCount; index++)
    mSpellCheck->IgnoreWordAllOccurrences(aWordsToIgnore[index]);

  mozInlineSpellStatus status(this);
  nsresult rv = status.InitForSelection();
  NS_ENSURE_SUCCESS(rv, rv);
  return ScheduleSpellCheck(status);
}

NS_IMETHODIMP mozInlineSpellChecker::WillCreateNode(const nsAString & aTag, nsIDOMNode *aParent, PRInt32 aPosition)
{
  return NS_OK;
}

NS_IMETHODIMP mozInlineSpellChecker::DidCreateNode(const nsAString & aTag, nsIDOMNode *aNode, nsIDOMNode *aParent,
                                                   PRInt32 aPosition, nsresult aResult)
{
  return NS_OK;
}

NS_IMETHODIMP mozInlineSpellChecker::WillInsertNode(nsIDOMNode *aNode, nsIDOMNode *aParent,
                                                    PRInt32 aPosition)
{
  return NS_OK;
}

NS_IMETHODIMP mozInlineSpellChecker::DidInsertNode(nsIDOMNode *aNode, nsIDOMNode *aParent,
                                                   PRInt32 aPosition, nsresult aResult)
{

  return NS_OK;
}

NS_IMETHODIMP mozInlineSpellChecker::WillDeleteNode(nsIDOMNode *aChild)
{
  return NS_OK;
}

NS_IMETHODIMP mozInlineSpellChecker::DidDeleteNode(nsIDOMNode *aChild, nsresult aResult)
{
  return NS_OK;
}

NS_IMETHODIMP mozInlineSpellChecker::WillSplitNode(nsIDOMNode *aExistingRightNode, PRInt32 aOffset)
{
  return NS_OK;
}

NS_IMETHODIMP
mozInlineSpellChecker::DidSplitNode(nsIDOMNode *aExistingRightNode,
                                    PRInt32 aOffset,
                                    nsIDOMNode *aNewLeftNode, nsresult aResult)
{
  return SpellCheckBetweenNodes(aNewLeftNode, 0, aNewLeftNode, 0);
}

NS_IMETHODIMP mozInlineSpellChecker::WillJoinNodes(nsIDOMNode *aLeftNode, nsIDOMNode *aRightNode, nsIDOMNode *aParent)
{
  return NS_OK;
}

NS_IMETHODIMP mozInlineSpellChecker::DidJoinNodes(nsIDOMNode *aLeftNode, nsIDOMNode *aRightNode, 
                                                  nsIDOMNode *aParent, nsresult aResult)
{
  return SpellCheckBetweenNodes(aRightNode, 0, aRightNode, 0);
}

NS_IMETHODIMP mozInlineSpellChecker::WillInsertText(nsIDOMCharacterData *aTextNode, PRInt32 aOffset, const nsAString & aString)
{
  return NS_OK;
}

NS_IMETHODIMP mozInlineSpellChecker::DidInsertText(nsIDOMCharacterData *aTextNode, PRInt32 aOffset,
                                                   const nsAString & aString, nsresult aResult)
{
  return NS_OK;
}

NS_IMETHODIMP mozInlineSpellChecker::WillDeleteText(nsIDOMCharacterData *aTextNode, PRInt32 aOffset, PRInt32 aLength)
{
  return NS_OK;
}

NS_IMETHODIMP mozInlineSpellChecker::DidDeleteText(nsIDOMCharacterData *aTextNode, PRInt32 aOffset, PRInt32 aLength, nsresult aResult)
{
  return NS_OK;
}

NS_IMETHODIMP mozInlineSpellChecker::WillDeleteSelection(nsISelection *aSelection)
{
  return NS_OK;
}

NS_IMETHODIMP mozInlineSpellChecker::DidDeleteSelection(nsISelection *aSelection)
{
  return NS_OK;
}

// mozInlineSpellChecker::MakeSpellCheckRange
//
//    Given begin and end positions, this function constructs a range as
//    required for ScheduleSpellCheck. If the start and end nodes are NULL,
//    then the entire range will be selected, and you can supply -1 as the
//    offset to the end range to select all of that node.
//
//    If the resulting range would be empty, NULL is put into *aRange and the
//    function succeeds.

nsresult
mozInlineSpellChecker::MakeSpellCheckRange(
    nsIDOMNode* aStartNode, PRInt32 aStartOffset,
    nsIDOMNode* aEndNode, PRInt32 aEndOffset,
    nsIDOMRange** aRange)
{
  nsresult rv;
  *aRange = nsnull;

  nsCOMPtr<nsIEditor> editor (do_QueryReferent(mEditor));
  NS_ENSURE_TRUE(editor, NS_ERROR_NULL_POINTER);

  nsCOMPtr<nsIDOMDocument> doc;
  rv = editor->GetDocument(getter_AddRefs(doc));
  NS_ENSURE_SUCCESS(rv, rv);

  nsCOMPtr<nsIDOMDocumentRange> docrange = do_QueryInterface(doc);
  NS_ENSURE_TRUE(docrange, NS_ERROR_FAILURE);

  nsCOMPtr<nsIDOMRange> range;
  rv = docrange->CreateRange(getter_AddRefs(range));
  NS_ENSURE_SUCCESS(rv, rv);

  // possibly use full range of the editor
  nsCOMPtr<nsIDOMElement> rootElem;
  if (! aStartNode || ! aEndNode) {
    rv = editor->GetRootElement(getter_AddRefs(rootElem));
    NS_ENSURE_SUCCESS(rv, rv);

    aStartNode = rootElem;
    aStartOffset = 0;

    aEndNode = rootElem;
    aEndOffset = -1;
  }

  if (aEndOffset == -1) {
    nsCOMPtr<nsIDOMNodeList> childNodes;
    rv = aEndNode->GetChildNodes(getter_AddRefs(childNodes));
    NS_ENSURE_SUCCESS(rv, rv);

    PRUint32 childCount;
    rv = childNodes->GetLength(&childCount);
    NS_ENSURE_SUCCESS(rv, rv);

    aEndOffset = childCount;
  }

  // sometimes we are are requested to check an empty range (possibly an empty
  // document). This will result in assertions later.
  if (aStartNode == aEndNode && aStartOffset == aEndOffset)
    return NS_OK;

  rv = range->SetStart(aStartNode, aStartOffset);
  NS_ENSURE_SUCCESS(rv, rv);
  if (aEndOffset)
    rv = range->SetEnd(aEndNode, aEndOffset);
  else
    rv = range->SetEndAfter(aEndNode);
  NS_ENSURE_SUCCESS(rv, rv);

  range.swap(*aRange);
  return NS_OK;
}

nsresult
mozInlineSpellChecker::SpellCheckBetweenNodes(nsIDOMNode *aStartNode,
                                              PRInt32 aStartOffset,
                                              nsIDOMNode *aEndNode,
                                              PRInt32 aEndOffset)
{
  nsCOMPtr<nsIDOMRange> range;
  nsresult rv = MakeSpellCheckRange(aStartNode, aStartOffset,
                                    aEndNode, aEndOffset,
                                    getter_AddRefs(range));
  NS_ENSURE_SUCCESS(rv, rv);

  if (! range)
    return NS_OK; // range is empty: nothing to do

  mozInlineSpellStatus status(this);
  rv = status.InitForRange(range);
  NS_ENSURE_SUCCESS(rv, rv);
  return ScheduleSpellCheck(status);
}

// mozInlineSpellChecker::SkipSpellCheckForNode
//
//    There are certain conditions when we don't want to spell check a node. In
//    particular quotations, moz signatures, etc. This routine returns false
//    for these cases.

nsresult
mozInlineSpellChecker::SkipSpellCheckForNode(nsIEditor* aEditor,
                                             nsIDOMNode *aNode,
                                             PRBool *checkSpelling)
{
  *checkSpelling = PR_TRUE;
  NS_ENSURE_ARG_POINTER(aNode);

  PRUint32 flags;
  aEditor->GetFlags(&flags);
  if (flags & nsIPlaintextEditor::eEditorMailMask)
  {
    nsCOMPtr<nsIDOMNode> parent;
    aNode->GetParentNode(getter_AddRefs(parent));

    while (parent)
    {
      nsCOMPtr<nsIDOMElement> parentElement = do_QueryInterface(parent);
      if (!parentElement)
        break;

      nsAutoString parentTagName;
      parentElement->GetTagName(parentTagName);

      if (parentTagName.Equals(NS_LITERAL_STRING("blockquote"), nsCaseInsensitiveStringComparator()))
      {
        *checkSpelling = PR_FALSE;
        break;
      }
      else if (parentTagName.Equals(NS_LITERAL_STRING("pre"), nsCaseInsensitiveStringComparator()))
      {
        nsAutoString classname;
        parentElement->GetAttribute(NS_LITERAL_STRING("class"),classname);
        if (classname.Equals(NS_LITERAL_STRING("moz-signature")))
          *checkSpelling = PR_FALSE;
      }

      nsCOMPtr<nsIDOMNode> nextParent;
      parent->GetParentNode(getter_AddRefs(nextParent));
      parent = nextParent;
    }
  }
  else {
    // XXX Do we really want this for all read-write content?
    nsCOMPtr<nsIContent> content = do_QueryInterface(aNode);
    *checkSpelling = !!(content->IntrinsicState() & NS_EVENT_STATE_MOZ_READWRITE);
  }

  return NS_OK;
}

// mozInlineSpellChecker::ScheduleSpellCheck
//
//    This is called by code to do the actual spellchecking. We will set up
//    the proper structures for calls to DoSpellCheck.

nsresult
mozInlineSpellChecker::ScheduleSpellCheck(const mozInlineSpellStatus& aStatus)
{
  mozInlineSpellResume* resume = new mozInlineSpellResume(aStatus);
  NS_ENSURE_TRUE(resume, NS_ERROR_OUT_OF_MEMORY);

  nsresult rv = resume->Post();
  if (NS_FAILED(rv))
    delete resume;
  return rv;
}

// mozInlineSpellChecker::DoSpellCheckSelection
//
//    Called to re-check all misspelled words. We iterate over all ranges in
//    the selection and call DoSpellCheck on them. This is used when a word
//    is ignored or added to the dictionary: all instances of that word should
//    be removed from the selection.
//
//    FIXME-PERFORMANCE: This takes as long as it takes and is not resumable.
//    Typically, checking this small amount of text is relatively fast, but
//    for large numbers of words, a lag may be noticable.

nsresult
mozInlineSpellChecker::DoSpellCheckSelection(mozInlineSpellWordUtil& aWordUtil,
                                             nsISelection* aSpellCheckSelection,
                                             mozInlineSpellStatus* aStatus)
{
  nsresult rv;

  // clear out mNumWordsInSpellSelection since we'll be rebuilding the ranges.
  mNumWordsInSpellSelection = 0;

  // Since we could be modifying the ranges for the spellCheckSelection while
  // looping on the spell check selection, keep a separate array of range
  // elements inside the selection
  nsCOMArray<nsIDOMRange> ranges;

  PRInt32 count;
  aSpellCheckSelection->GetRangeCount(&count);

  PRInt32 idx;
  nsCOMPtr<nsIDOMRange> checkRange;
  for (idx = 0; idx < count; idx ++) {
    aSpellCheckSelection->GetRangeAt(idx, getter_AddRefs(checkRange));
    if (checkRange) {
      if (! ranges.AppendObject(checkRange))
        return NS_ERROR_OUT_OF_MEMORY;
    }
  }

  // We have saved the ranges above. Clearing the spellcheck selection here
  // isn't necessary (rechecking each word will modify it as necessary) but
  // provides better performance. By ensuring that no ranges need to be
  // removed in DoSpellCheck, we can save checking range inclusion which is
  // slow.
  aSpellCheckSelection->RemoveAllRanges();

  // We use this state object for all calls, and just update its range. Note
  // that we don't need to call FinishInit since we will be filling in the
  // necessary information.
  mozInlineSpellStatus status(this);
  rv = status.InitForRange(nsnull);
  NS_ENSURE_SUCCESS(rv, rv);

  PRBool doneChecking;
  for (idx = 0; idx < count; idx ++) {
    checkRange = ranges[idx];
    if (checkRange) {
      // We can consider this word as "added" since we know it has no spell
      // check range over it that needs to be deleted. All the old ranges
      // were cleared above. We also need to clear the word count so that we
      // check all words instead of stopping early.
      status.mRange = checkRange;
      rv = DoSpellCheck(aWordUtil, aSpellCheckSelection, &status,
                        &doneChecking);
      NS_ENSURE_SUCCESS(rv, rv);
      NS_ASSERTION(doneChecking, "We gave the spellchecker one word, but it didn't finish checking?!?!");

      status.mWordCount = 0;
    }
  }

  return NS_OK;
}

// mozInlineSpellChecker::DoSpellCheck
//
//    This function checks words intersecting the given range, excluding those
//    inside mStatus->mNoCheckRange (can be NULL). Words inside aNoCheckRange
//    will have any spell selection removed (this is used to hide the
//    underlining for the word that the caret is in). aNoCheckRange should be
//    on word boundaries.
//
//    mResume->mCreatedRange is a possibly NULL range of new text that was
//    inserted.  Inside this range, we don't bother to check whether things are
//    inside the spellcheck selection, which speeds up large paste operations
//    considerably.
//
//    Normal case when editing text by typing
//       h e l l o   w o r k d   h o w   a r e   y o u
//                            ^ caret
//                   [-------] mRange
//                   [-------] mNoCheckRange
//      -> does nothing (range is the same as the no check range)
//
//    Case when pasting:
//             [---------- pasted text ----------]
//       h e l l o   w o r k d   h o w   a r e   y o u
//                                                ^ caret
//                                               [---] aNoCheckRange
//      -> recheck all words in range except those in aNoCheckRange
//
//    If checking is complete, *aDoneChecking will be set. If there is more
//    but we ran out of time, this will be false and the range will be
//    updated with the stuff that still needs checking.

nsresult mozInlineSpellChecker::DoSpellCheck(mozInlineSpellWordUtil& aWordUtil,
                                             nsISelection *aSpellCheckSelection,
                                             mozInlineSpellStatus* aStatus,
                                             PRBool* aDoneChecking)
{
  nsCOMPtr<nsIDOMNode> beginNode, endNode;
  PRInt32 beginOffset, endOffset;
  *aDoneChecking = PR_TRUE;

  // get the editor for SkipSpellCheckForNode, this may fail in reasonable
  // circumstances since the editor could have gone away
  nsCOMPtr<nsIEditor> editor (do_QueryReferent(mEditor));
  if (! editor)
    return NS_ERROR_FAILURE;

  PRBool iscollapsed;
  nsresult rv = aStatus->mRange->GetCollapsed(&iscollapsed);
  NS_ENSURE_SUCCESS(rv, rv);
  if (iscollapsed)
    return NS_OK;

  nsCOMPtr<nsISelection2> sel2 = do_QueryInterface(aSpellCheckSelection, &rv);
  NS_ENSURE_SUCCESS(rv, rv);

  // see if the selection has any ranges, if not, then we can optimize checking
  // range inclusion later (we have no ranges when we are initially checking or
  // when there are no misspelled words yet).
  PRInt32 originalRangeCount;
  rv = aSpellCheckSelection->GetRangeCount(&originalRangeCount);
  NS_ENSURE_SUCCESS(rv, rv);

  // set the starting DOM position to be the beginning of our range
  NS_ENSURE_SUCCESS(rv, rv);
  aStatus->mRange->GetStartContainer(getter_AddRefs(beginNode));
  aStatus->mRange->GetStartOffset(&beginOffset);
  aStatus->mRange->GetEndContainer(getter_AddRefs(endNode));
  aStatus->mRange->GetEndOffset(&endOffset);
  aWordUtil.SetEnd(endNode, endOffset);
  aWordUtil.SetPosition(beginNode, beginOffset);

  // we need to use IsPointInRange which is on a more specific interface
  nsCOMPtr<nsIDOMNSRange> noCheckRange, createdRange;
  if (aStatus->mNoCheckRange)
    noCheckRange = do_QueryInterface(aStatus->mNoCheckRange);
  if (aStatus->mCreatedRange)
    createdRange = do_QueryInterface(aStatus->mCreatedRange);

  PRInt32 wordsSinceTimeCheck = 0;
  PRTime beginTime = PR_Now();

  nsAutoString wordText;
  nsCOMPtr<nsIDOMRange> wordRange;
  PRBool dontCheckWord;
  while (NS_SUCCEEDED(aWordUtil.GetNextWord(wordText,
                                            getter_AddRefs(wordRange),
                                            &dontCheckWord)) &&
         wordRange) {
    wordsSinceTimeCheck ++;

    // get the range for the current word
    wordRange->GetStartContainer(getter_AddRefs(beginNode));
    wordRange->GetEndContainer(getter_AddRefs(endNode));
    wordRange->GetStartOffset(&beginOffset);
    wordRange->GetEndOffset(&endOffset);

#ifdef DEBUG_INLINESPELL
    printf("->Got word \"%s\"", NS_ConvertUTF16toUTF8(wordText).get());
    if (dontCheckWord)
      printf(" (not checking)");
    printf("\n");
#endif

    // see if there is a spellcheck range that already intersects the word
    // and remove it. We only need to remove old ranges, so don't bother if
    // there were no ranges when we started out.
    if (originalRangeCount > 0) {
      // likewise, if this word is inside new text, we won't bother testing
      PRBool inCreatedRange = PR_FALSE;
      if (createdRange)
        createdRange->IsPointInRange(beginNode, beginOffset, &inCreatedRange);
      if (! inCreatedRange) {
        nsCOMArray<nsIDOMRange> ranges;
        rv = sel2->GetRangesForIntervalCOMArray(beginNode, beginOffset,
                                                endNode, endOffset,
                                                PR_TRUE, &ranges);
        NS_ENSURE_SUCCESS(rv, rv);
        for (PRInt32 i = 0; i < ranges.Count(); i ++)
          RemoveRange(aSpellCheckSelection, ranges[i]);
      }
    }

    // some words are special and don't need checking
    if (dontCheckWord)
      continue;

    // some nodes we don't spellcheck
    PRBool checkSpelling;
    rv = SkipSpellCheckForNode(editor, beginNode, &checkSpelling);
    NS_ENSURE_SUCCESS(rv, rv);
    if (!checkSpelling)
      continue;

    // Don't check spelling if we're inside the noCheckRange. This needs to
    // be done after we clear any old selection because the excluded word
    // might have been previously marked.
    //
    // We do a simple check to see if the beginning of our word is in the
    // exclusion range. Because the exclusion range is a multiple of a word,
    // this is sufficient.
    if (noCheckRange) {
      PRBool inExclusion = PR_FALSE;
      noCheckRange->IsPointInRange(beginNode, beginOffset, &inExclusion);
      if (inExclusion)
        continue;
    }

    // check spelling and add to selection if misspelled
    PRBool isMisspelled;
    aWordUtil.NormalizeWord(wordText);
    rv = mSpellCheck->CheckCurrentWordNoSuggest(wordText.get(), &isMisspelled);
    if (isMisspelled) {
      // misspelled words count extra toward the max
      wordsSinceTimeCheck += MISSPELLED_WORD_COUNT_PENALTY;
      AddRange(aSpellCheckSelection, wordRange);

      aStatus->mWordCount ++;
      if (aStatus->mWordCount >= mMaxMisspellingsPerCheck ||
          SpellCheckSelectionIsFull())
        break;
    }

    // see if we've run out of time, only check every N words for perf
    if (wordsSinceTimeCheck >= INLINESPELL_TIMEOUT_CHECK_FREQUENCY) {
      wordsSinceTimeCheck = 0;
      if (PR_Now() > beginTime + INLINESPELL_CHECK_TIMEOUT * PR_USEC_PER_MSEC) {
        // stop checking, our time limit has been exceeded

        // move the range to encompass the stuff that needs checking
        rv = aStatus->mRange->SetStart(endNode, endOffset);
        if (NS_FAILED(rv)) {
          // The range might be unhappy because the beginning is after the
          // end. This is possible when the requested end was in the middle
          // of a word, just ignore this situation and assume we're done.
          return NS_OK;
        }
        *aDoneChecking = PR_FALSE;
        return NS_OK;
      }
    }
  }

  return NS_OK;
}

// mozInlineSpellChecker::ResumeCheck
//
//    Called by the resume event when it fires. We will try to pick up where
//    the last resume left off.

nsresult
mozInlineSpellChecker::ResumeCheck(mozInlineSpellStatus* aStatus)
{
  if (! mSpellCheck)
    return NS_OK; // spell checking has been turned off

  mozInlineSpellWordUtil wordUtil;
  nsresult rv = wordUtil.Init(mEditor);
  if (NS_FAILED(rv))
    return NS_OK; // editor doesn't like us, don't assert

  nsCOMPtr<nsISelection> spellCheckSelection;
  rv = GetSpellCheckSelection(getter_AddRefs(spellCheckSelection));
  NS_ENSURE_SUCCESS(rv, rv);
  CleanupRangesInSelection(spellCheckSelection);

  rv = aStatus->FinishInitOnEvent(wordUtil);
  NS_ENSURE_SUCCESS(rv, rv);
  if (! aStatus->mRange)
    return NS_OK; // empty range, nothing to do

  PRBool doneChecking = PR_TRUE;
  if (aStatus->mOp == mozInlineSpellStatus::eOpSelection)
    rv = DoSpellCheckSelection(wordUtil, spellCheckSelection, aStatus);
  else
    rv = DoSpellCheck(wordUtil, spellCheckSelection, aStatus, &doneChecking);
  NS_ENSURE_SUCCESS(rv, rv);

  if (! doneChecking)
    rv = ScheduleSpellCheck(*aStatus);
  return rv;
}

// mozInlineSpellChecker::IsPointInSelection
//
//    Determines if a given (node,offset) point is inside the given
//    selection. If so, the specific range of the selection that
//    intersects is places in *aRange. (There may be multiple disjoint
//    ranges in a selection.)
//
//    If there is no intersection, *aRange will be NULL.

nsresult
mozInlineSpellChecker::IsPointInSelection(nsISelection *aSelection,
                                          nsIDOMNode *aNode,
                                          PRInt32 aOffset,
                                          nsIDOMRange **aRange)
{
  *aRange = nsnull;

  nsresult rv;
  nsCOMPtr<nsISelection2> sel2 = do_QueryInterface(aSelection, &rv);
  NS_ENSURE_SUCCESS(rv, rv);

  nsCOMArray<nsIDOMRange> ranges;
  rv = sel2->GetRangesForIntervalCOMArray(aNode, aOffset, aNode, aOffset,
                                          PR_TRUE, &ranges);
  NS_ENSURE_SUCCESS(rv, rv);

  if (ranges.Count() == 0)
    return NS_OK; // no matches

  // there may be more than one range returned, and we don't know what do
  // do with that, so just get the first one
  NS_ADDREF(*aRange = ranges[0]);
  return NS_OK;
}

nsresult
mozInlineSpellChecker::CleanupRangesInSelection(nsISelection *aSelection)
{
  // integrity check - remove ranges that have collapsed to nothing. This
  // can happen if the node containing a highlighted word was removed.
  NS_ENSURE_ARG_POINTER(aSelection);

  PRInt32 count;
  aSelection->GetRangeCount(&count);

  for (PRInt32 index = 0; index < count; index++)
  {
    nsCOMPtr<nsIDOMRange> checkRange;
    aSelection->GetRangeAt(index, getter_AddRefs(checkRange));

    if (checkRange)
    {
      PRBool collapsed;
      checkRange->GetCollapsed(&collapsed);
      if (collapsed)
      {
        RemoveRange(aSelection, checkRange);
        index--;
        count--;
      }
    }
  }

  return NS_OK;
}


// mozInlineSpellChecker::RemoveRange
//
//    For performance reasons, we have an upper bound on the number of word
//    ranges  in the spell check selection. When removing a range from the
//    selection, we need to decrement mNumWordsInSpellSelection

nsresult
mozInlineSpellChecker::RemoveRange(nsISelection* aSpellCheckSelection,
                                   nsIDOMRange* aRange)
{
  NS_ENSURE_ARG_POINTER(aSpellCheckSelection);
  NS_ENSURE_ARG_POINTER(aRange);

  nsresult rv = aSpellCheckSelection->RemoveRange(aRange);
  if (NS_SUCCEEDED(rv) && mNumWordsInSpellSelection)
    mNumWordsInSpellSelection--;

  return rv;
}


// mozInlineSpellChecker::AddRange
//
//    For performance reasons, we have an upper bound on the number of word
//    ranges we'll add to the spell check selection. Once we reach that upper
//    bound, stop adding the ranges

nsresult
mozInlineSpellChecker::AddRange(nsISelection* aSpellCheckSelection,
                                nsIDOMRange* aRange)
{
  NS_ENSURE_ARG_POINTER(aSpellCheckSelection);
  NS_ENSURE_ARG_POINTER(aRange);

  nsresult rv = NS_OK;

  if (!SpellCheckSelectionIsFull())
  {
    rv = aSpellCheckSelection->AddRange(aRange);
    if (NS_SUCCEEDED(rv))
      mNumWordsInSpellSelection++;
  }

  return rv;
}

nsresult mozInlineSpellChecker::GetSpellCheckSelection(nsISelection ** aSpellCheckSelection)
{ 
  nsCOMPtr<nsIEditor> editor (do_QueryReferent(mEditor));
  NS_ENSURE_TRUE(editor, NS_ERROR_NULL_POINTER);

  nsCOMPtr<nsISelectionController> selcon;
  nsresult rv = editor->GetSelectionController(getter_AddRefs(selcon));
  NS_ENSURE_SUCCESS(rv, rv); 

  nsCOMPtr<nsISelection> spellCheckSelection;
  return selcon->GetSelection(nsISelectionController::SELECTION_SPELLCHECK, aSpellCheckSelection);
}

nsresult mozInlineSpellChecker::SaveCurrentSelectionPosition()
{
  nsCOMPtr<nsIEditor> editor (do_QueryReferent(mEditor));
  NS_ENSURE_TRUE(editor, NS_OK);

  // figure out the old caret position based on the current selection
  nsCOMPtr<nsISelection> selection;
  nsresult rv = editor->GetSelection(getter_AddRefs(selection));
  NS_ENSURE_SUCCESS(rv, rv);

  rv = selection->GetFocusNode(getter_AddRefs(mCurrentSelectionAnchorNode));
  NS_ENSURE_SUCCESS(rv, rv);
  
  selection->GetFocusOffset(&mCurrentSelectionOffset);

  return NS_OK;
}

// This is a copy of nsContentUtils::ContentIsDescendantOf. Another crime
// for XPCOM's rap sheet
PRBool // static
ContentIsDescendantOf(nsINode* aPossibleDescendant,
                      nsINode* aPossibleAncestor)
{
  NS_PRECONDITION(aPossibleDescendant, "The possible descendant is null!");
  NS_PRECONDITION(aPossibleAncestor, "The possible ancestor is null!");

  do {
    if (aPossibleDescendant == aPossibleAncestor)
      return PR_TRUE;
    aPossibleDescendant = aPossibleDescendant->GetNodeParent();
  } while (aPossibleDescendant);

  return PR_FALSE;
}

// mozInlineSpellChecker::HandleNavigationEvent
//
//    Acts upon mouse clicks and keyboard navigation changes, spell checking
//    the previous word if the new navigation location moves us to another
//    word.
//
//    This is complicated by the fact that our mouse events are happening after
//    selection has been changed to account for the mouse click. But keyboard
//    events are happening before the caret selection has changed. Working
//    around this by letting keyboard events setting forceWordSpellCheck to
//    true. aNewPositionOffset also tries to work around this for the
//    DOM_VK_RIGHT and DOM_VK_LEFT cases.

nsresult
mozInlineSpellChecker::HandleNavigationEvent(nsIDOMEvent* aEvent,
                                             PRBool aForceWordSpellCheck,
                                             PRInt32 aNewPositionOffset)
{
  nsresult rv;

  // If we already handled the navigation event and there is no possibility
  // anything has changed since then, we don't have to do anything. This
  // optimization makes a noticable difference when you hold down a navigation
  // key like Page Down.
  if (! mNeedsCheckAfterNavigation)
    return NS_OK;

  nsCOMPtr<nsIDOMNode> currentAnchorNode = mCurrentSelectionAnchorNode;
  PRInt32 currentAnchorOffset = mCurrentSelectionOffset;

  // now remember the new focus position resulting from the event
  rv = SaveCurrentSelectionPosition();
  NS_ENSURE_SUCCESS(rv, rv);

  PRBool shouldPost;
  mozInlineSpellStatus status(this);
  rv = status.InitForNavigation(aForceWordSpellCheck, aNewPositionOffset,
                                currentAnchorNode, currentAnchorOffset,
                                mCurrentSelectionAnchorNode, mCurrentSelectionOffset,
                                &shouldPost);
  NS_ENSURE_SUCCESS(rv, rv);
  if (shouldPost) {
    rv = ScheduleSpellCheck(status);
    NS_ENSURE_SUCCESS(rv, rv);
  }

  return NS_OK;
}

NS_IMETHODIMP mozInlineSpellChecker::HandleEvent(nsIDOMEvent* aEvent)
{
  return NS_OK;
}

NS_IMETHODIMP mozInlineSpellChecker::Focus(nsIDOMEvent* aEvent)
{
  return NS_OK;
}

NS_IMETHODIMP mozInlineSpellChecker::Blur(nsIDOMEvent* aEvent)
{
  // force spellcheck on blur, for instance when tabbing out of a textbox
  HandleNavigationEvent(aEvent, PR_TRUE);
  return NS_OK;
}

NS_IMETHODIMP mozInlineSpellChecker::MouseClick(nsIDOMEvent *aMouseEvent)
{
  nsCOMPtr<nsIDOMMouseEvent>mouseEvent = do_QueryInterface(aMouseEvent);
  NS_ENSURE_TRUE(mouseEvent, NS_OK);

  // ignore any errors from HandleNavigationEvent as we don't want to prevent 
  // anyone else from seeing this event.
  PRUint16 button;
  mouseEvent->GetButton(&button);
  if (button == 0)
    HandleNavigationEvent(mouseEvent, PR_FALSE);
  else
    HandleNavigationEvent(mouseEvent, PR_TRUE);
  return NS_OK;
}

NS_IMETHODIMP mozInlineSpellChecker::MouseDown(nsIDOMEvent* aMouseEvent)
{
  return NS_OK;
}

NS_IMETHODIMP mozInlineSpellChecker::MouseUp(nsIDOMEvent* aMouseEvent)
{
  return NS_OK;
}

NS_IMETHODIMP mozInlineSpellChecker::MouseDblClick(nsIDOMEvent* aMouseEvent)
{
  return NS_OK;
}

NS_IMETHODIMP mozInlineSpellChecker::MouseOver(nsIDOMEvent* aMouseEvent)
{
  return NS_OK;
}

NS_IMETHODIMP mozInlineSpellChecker::MouseOut(nsIDOMEvent* aMouseEvent)
{
  return NS_OK;
}

NS_IMETHODIMP mozInlineSpellChecker::KeyDown(nsIDOMEvent* aKeyEvent)
{
  return NS_OK;
}


NS_IMETHODIMP mozInlineSpellChecker::KeyUp(nsIDOMEvent* aKeyEvent)
{
  return NS_OK;
}


NS_IMETHODIMP mozInlineSpellChecker::KeyPress(nsIDOMEvent* aKeyEvent)
{
  nsCOMPtr<nsIDOMKeyEvent>keyEvent = do_QueryInterface(aKeyEvent);
  NS_ENSURE_TRUE(keyEvent, NS_OK);

  PRUint32 keyCode;
  keyEvent->GetKeyCode(&keyCode);

  // we only care about navigation keys that moved selection 
  switch (keyCode)
  {
    case nsIDOMKeyEvent::DOM_VK_RIGHT:
    case nsIDOMKeyEvent::DOM_VK_LEFT:
      HandleNavigationEvent(aKeyEvent, PR_FALSE, keyCode == nsIDOMKeyEvent::DOM_VK_RIGHT ? 1 : -1);
      break;
    case nsIDOMKeyEvent::DOM_VK_UP:
    case nsIDOMKeyEvent::DOM_VK_DOWN:
    case nsIDOMKeyEvent::DOM_VK_HOME:
    case nsIDOMKeyEvent::DOM_VK_END:
    case nsIDOMKeyEvent::DOM_VK_PAGE_UP:
    case nsIDOMKeyEvent::DOM_VK_PAGE_DOWN:
      HandleNavigationEvent(aKeyEvent, PR_TRUE /* force a spelling correction */);
      break;
  }

  return NS_OK;
}
