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
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
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

#ifndef nsTextServicesDocument_h__
#define nsTextServicesDocument_h__

#include "nsCOMPtr.h"
#include "nsIPresShell.h"
#include "nsIDOMDocument.h"
#include "nsIDOMRange.h"
#include "nsIContent.h"
#include "nsIContentIterator.h"
#include "nsIEditor.h"
#include "nsIEditActionListener.h"
#include "nsITextServicesDocument.h"
#include "nsVoidArray.h"
#include "nsTSDNotifier.h"
#include "nsISelectionController.h"
#include "nsITextServicesFilter.h"
#include "nsWeakReference.h"

class nsIRangeUtils;

/** implementation of a text services object.
 *
 */
class nsTextServicesDocument : public nsITextServicesDocument
{
private:
  static nsIAtom *sAAtom;
  static nsIAtom *sAddressAtom;
  static nsIAtom *sBigAtom;
  static nsIAtom *sBlinkAtom;
  static nsIAtom *sBAtom;
  static nsIAtom *sCiteAtom;
  static nsIAtom *sCodeAtom;
  static nsIAtom *sDfnAtom;
  static nsIAtom *sEmAtom;
  static nsIAtom *sFontAtom;
  static nsIAtom *sIAtom;
  static nsIAtom *sKbdAtom;
  static nsIAtom *sKeygenAtom;
  static nsIAtom *sNobrAtom;
  static nsIAtom *sSAtom;
  static nsIAtom *sSampAtom;
  static nsIAtom *sSmallAtom;
  static nsIAtom *sSpacerAtom;
  static nsIAtom *sSpanAtom;      
  static nsIAtom *sStrikeAtom;
  static nsIAtom *sStrongAtom;
  static nsIAtom *sSubAtom;
  static nsIAtom *sSupAtom;
  static nsIAtom *sTtAtom;
  static nsIAtom *sUAtom;
  static nsIAtom *sVarAtom;
  static nsIAtom *sWbrAtom;

  typedef enum { eIsDone=0,        // No iterator (I), or iterator doesn't point to anything valid.
                 eValid,           // I points to first text node (TN) in current block (CB).
                 ePrev,            // No TN in CB, I points to first TN in prev block.
                 eNext             // No TN in CB, I points to first TN in next block.
  } TSDIteratorStatus;

  nsCOMPtr<nsIDOMDocument>        mDOMDocument;
  nsCOMPtr<nsISelectionController>mSelCon;
  nsWeakPtr                       mEditor;  // avoid a cycle with the spell checker and editor
  nsCOMPtr<nsIContentIterator>    mIterator;
  TSDIteratorStatus               mIteratorStatus;
  nsCOMPtr<nsIContent>            mPrevTextBlock;
  nsCOMPtr<nsIContent>            mNextTextBlock;
  nsCOMPtr<nsIEditActionListener> mNotifier;
  nsVoidArray                     mOffsetTable;

  PRInt32                         mSelStartIndex;
  PRInt32                         mSelStartOffset;
  PRInt32                         mSelEndIndex;
  PRInt32                         mSelEndOffset;

  nsCOMPtr<nsIDOMRange>           mExtent;

  nsCOMPtr<nsITextServicesFilter> mTxtSvcFilter;

  static nsIRangeUtils* sRangeHelper;

public:

  /** The default constructor.
   */
  nsTextServicesDocument();

  /** The default destructor.
   */
  virtual ~nsTextServicesDocument();

  /** To be called at module init
   */
  static void RegisterAtoms();

  /** To be called at module shutdown
   */
  static void Shutdown();

  /* Macro for AddRef(), Release(), and QueryInterface() */
  NS_DECL_ISUPPORTS

  /* nsITextServicesDocument method implementations. */
  NS_IMETHOD InitWithDocument(nsIDOMDocument *aDOMDocument, nsIPresShell *aPresShell);
  NS_IMETHOD InitWithEditor(nsIEditor *aEditor);
  NS_IMETHOD GetDocument(nsIDOMDocument **aDoc);
  NS_IMETHOD SetExtent(nsIDOMRange* aDOMRange);
  NS_IMETHOD GetExtent(nsIDOMRange** aDOMRange);
  NS_IMETHOD ExpandRangeToWordBoundaries(nsIDOMRange *aRange);
  NS_IMETHOD SetFilter(nsITextServicesFilter *aFilter);
  NS_IMETHOD CanEdit(PRBool *aCanEdit);
  NS_IMETHOD GetCurrentTextBlock(nsString *aStr);
  NS_IMETHOD FirstBlock();
  NS_IMETHOD LastBlock();
  NS_IMETHOD FirstSelectedBlock(TSDBlockSelectionStatus *aSelStatus, PRInt32 *aSelOffset, PRInt32 *aSelLength);
  NS_IMETHOD LastSelectedBlock(TSDBlockSelectionStatus *aSelStatus, PRInt32 *aSelOffset, PRInt32 *aSelLength);
  NS_IMETHOD PrevBlock();
  NS_IMETHOD NextBlock();
  NS_IMETHOD IsDone(PRBool *aIsDone);
  NS_IMETHOD SetSelection(PRInt32 aOffset, PRInt32 aLength);
  NS_IMETHOD ScrollSelectionIntoView();
  NS_IMETHOD DeleteSelection();
  NS_IMETHOD InsertText(const nsString *aText);
  NS_IMETHOD SetDisplayStyle(TSDDisplayStyle aStyle);
  NS_IMETHOD GetDOMRangeFor(PRInt32 aOffset, PRInt32 aLength, nsIDOMRange** aRange);

  /* nsIEditActionListener method implementations. */
  nsresult InsertNode(nsIDOMNode * aNode,
                      nsIDOMNode * aParent,
                      PRInt32      aPosition);
  nsresult DeleteNode(nsIDOMNode * aChild);
  nsresult SplitNode(nsIDOMNode * aExistingRightNode,
                     PRInt32      aOffset,
                     nsIDOMNode * aNewLeftNode);
  nsresult JoinNodes(nsIDOMNode  *aLeftNode,
                     nsIDOMNode  *aRightNode,
                     nsIDOMNode  *aParent);

  /* Helper functions */
  static nsresult ComparePoints(nsIDOMNode *aParent1, PRInt32 aOffset1, nsIDOMNode *aParent2, PRInt32 aOffset2, PRInt32 *aResult);
  static nsresult GetRangeEndPoints(nsIDOMRange *aRange, nsIDOMNode **aParent1, PRInt32 *aOffset1, nsIDOMNode **aParent2, PRInt32 *aOffset2);
  static nsresult CreateRange(nsIDOMNode *aStartParent, PRInt32 aStartOffset, nsIDOMNode *aEndParent, PRInt32 aEndOffset, nsIDOMRange **aRange);

private:

  /* nsTextServicesDocument private methods. */

  nsresult CreateContentIterator(nsIDOMRange *aRange, nsIContentIterator **aIterator);

  nsresult GetDocumentContentRootNode(nsIDOMNode **aNode);
  nsresult CreateDocumentContentRange(nsIDOMRange **aRange);
  nsresult CreateDocumentContentRootToNodeOffsetRange(nsIDOMNode *aParent, PRInt32 aOffset, PRBool aToStart, nsIDOMRange **aRange);
  nsresult CreateDocumentContentIterator(nsIContentIterator **aIterator);

  nsresult AdjustContentIterator();

  static nsresult FirstTextNode(nsIContentIterator *aIterator, TSDIteratorStatus *IteratorStatus);
  static nsresult LastTextNode(nsIContentIterator *aIterator, TSDIteratorStatus *IteratorStatus);

  static nsresult FirstTextNodeInCurrentBlock(nsIContentIterator *aIterator);
  static nsresult FirstTextNodeInPrevBlock(nsIContentIterator *aIterator);
  static nsresult FirstTextNodeInNextBlock(nsIContentIterator *aIterator);

  nsresult GetFirstTextNodeInPrevBlock(nsIContent **aContent);
  nsresult GetFirstTextNodeInNextBlock(nsIContent **aContent);

  static PRBool IsBlockNode(nsIContent *aContent);
  static PRBool IsTextNode(nsIContent *aContent);
  static PRBool IsTextNode(nsIDOMNode *aNode);

  static PRBool DidSkip(nsIContentIterator* aFilteredIter);
  static void   ClearDidSkip(nsIContentIterator* aFilteredIter);

  static PRBool HasSameBlockNodeParent(nsIContent *aContent1, nsIContent *aContent2);

  nsresult SetSelectionInternal(PRInt32 aOffset, PRInt32 aLength, PRBool aDoUpdate);
  nsresult GetSelection(TSDBlockSelectionStatus *aSelStatus, PRInt32 *aSelOffset, PRInt32 *aSelLength);
  nsresult GetCollapsedSelection(TSDBlockSelectionStatus *aSelStatus, PRInt32 *aSelOffset, PRInt32 *aSelLength);
  nsresult GetUncollapsedSelection(TSDBlockSelectionStatus *aSelStatus, PRInt32 *aSelOffset, PRInt32 *aSelLength);

  PRBool SelectionIsCollapsed();
  PRBool SelectionIsValid();

  static nsresult CreateOffsetTable(nsVoidArray *aOffsetTable,
                             nsIContentIterator *aIterator,
                             TSDIteratorStatus *aIteratorStatus,
                             nsIDOMRange *aIterRange,
                             nsString *aStr);
  static nsresult ClearOffsetTable(nsVoidArray *aOffsetTable);

  static nsresult NodeHasOffsetEntry(nsVoidArray *aOffsetTable,
                                     nsIDOMNode *aNode,
                                     PRBool *aHasEntry,
                                     PRInt32 *aEntryIndex);

  nsresult RemoveInvalidOffsetEntries();
  nsresult SplitOffsetEntry(PRInt32 aTableIndex, PRInt32 aOffsetIntoEntry);

  static nsresult FindWordBounds(nsVoidArray *offsetTable, nsString *blockStr,
                                 nsIDOMNode *aNode, PRInt32 aNodeOffset,
                                 nsIDOMNode **aWordStartNode,
                                 PRInt32 *aWordStartOffset,
                                 nsIDOMNode **aWordEndNode,
                                 PRInt32 *aWordEndOffset);

#ifdef DEBUG_kin
  void PrintOffsetTable();
  void PrintContentNode(nsIContent *aContent);
#endif
};

#endif // nsTextServicesDocument_h__
