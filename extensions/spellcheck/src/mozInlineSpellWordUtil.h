/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
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
 * The Original Code is inline spellchecker code.
 *
 * The Initial Developer of the Original Code is Google Inc.
 * Portions created by the Initial Developer are Copyright (C) 2004-2006
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Brett Wilson <brettw@gmail.com> (original author)
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

#include "nsCOMPtr.h"
#include "nsIDOMDocument.h"
#include "nsIDOMDocumentRange.h"
#include "nsIDOMViewCSS.h"
#include "nsIDocument.h"
#include "nsString.h"
#include "nsTArray.h"
#include "nsIUGenCategory.h"

//#define DEBUG_SPELLCHECK

class nsIDOMRange;
class nsIDOMNode;

/**
 *    This class extracts text from the DOM and builds it into a single string.
 *    The string includes whitespace breaks whereever non-inline elements begin
 *    and end. This string is broken into "real words", following somewhat
 *    complex rules; for example substrings that look like URLs or
 *    email addresses are treated as single words, but otherwise many kinds of
 *    punctuation are treated as word separators. GetNextWord provides a way
 *    to iterate over these "real words".
 *
 *    The basic operation is:
 *
 *    1. Call Init with the weak pointer to the editor that you're using.
 *    2. Call SetEnd to set where you want to stop spellchecking. We'll stop
 *       at the word boundary after that. If SetEnd is not called, we'll stop
 *       at the end of the document's root element.
 *    3. Call SetPosition to initialize the current position inside the
 *       previously given range.
 *    4. Call GetNextWord over and over until it returns false.
 */

class mozInlineSpellWordUtil
{
public:
  struct NodeOffset {
    nsIDOMNode* mNode;
    PRInt32     mOffset;
    
    NodeOffset(nsIDOMNode* aNode, PRInt32 aOffset) :
      mNode(aNode), mOffset(aOffset) {}
  };

  mozInlineSpellWordUtil()
    : mRootNode(nsnull),
      mSoftBegin(nsnull, 0), mSoftEnd(nsnull, 0),
      mNextWordIndex(-1), mSoftTextValid(PR_FALSE) {}

  nsresult Init(nsWeakPtr aWeakEditor);

  nsresult SetEnd(nsIDOMNode* aEndNode, PRInt32 aEndOffset);

  // sets the current position, this should be inside the range. If we are in
  // the middle of a word, we'll move to its start.
  nsresult SetPosition(nsIDOMNode* aNode, PRInt32 aOffset);

  // Given a point inside or immediately following a word, this returns the
  // DOM range that exactly encloses that word's characters. The current
  // position will be at the end of the word. This will find the previous
  // word if the current position is space, so if you care that the point is
  // inside the word, you should check the range.
  //
  // THIS CHANGES THE CURRENT POSITION AND RANGE. It is designed to be called
  // before you actually generate the range you are interested in and iterate
  // the words in it.
  nsresult GetRangeForWord(nsIDOMNode* aWordNode, PRInt32 aWordOffset,
                           nsIDOMRange** aRange);

  // Moves to the the next word in the range, and retrieves it's text and range.
  // An empty word and a NULL range are returned when we are done checking.
  // aSkipChecking will be set if the word is "special" and shouldn't be
  // checked (e.g., an email address).
  nsresult GetNextWord(nsAString& aText, nsIDOMRange** aRange,
                       PRBool* aSkipChecking);

  // Call to normalize some punctuation. This function takes an autostring
  // so we can access characters directly.
  static void NormalizeWord(nsSubstring& aWord);

  nsIDOMDocumentRange* GetDocumentRange() const { return mDOMDocumentRange; }
  nsIDocument* GetDocument() const { return mDocument; }
  nsIDOMNode* GetRootNode() { return mRootNode; }
  nsIUGenCategory* GetCategories() { return mCategories; }
  
private:

  // cached stuff for the editor, set by Init
  nsCOMPtr<nsIDOMDocumentRange> mDOMDocumentRange;
  nsCOMPtr<nsIDocument>         mDocument;
  nsCOMPtr<nsIDOMViewCSS>       mCSSView;
  nsCOMPtr<nsIUGenCategory>     mCategories;

  // range to check, see SetRange
  nsIDOMNode* mRootNode;
  NodeOffset  mSoftBegin;
  NodeOffset  mSoftEnd;

  // DOM text covering the soft range, with newlines added at block boundaries
  nsString mSoftText;
  // A list of where we extracted text from, ordered by mSoftTextOffset. A given
  // DOM node appears at most once in this list.
  struct DOMTextMapping {
    NodeOffset mNodeOffset;
    PRInt32    mSoftTextOffset;
    PRInt32    mLength;
    
    DOMTextMapping(NodeOffset aNodeOffset, PRInt32 aSoftTextOffset, PRInt32 aLength)
      : mNodeOffset(aNodeOffset), mSoftTextOffset(aSoftTextOffset),
        mLength(aLength) {}
  };
  nsTArray<DOMTextMapping> mSoftTextDOMMapping;
  
  // A list of the "real words" in mSoftText, ordered by mSoftTextOffset
  struct RealWord {
    PRInt32      mSoftTextOffset;
    PRInt32      mLength;
    PRPackedBool mCheckableWord;
    
    RealWord(PRInt32 aOffset, PRInt32 aLength, PRPackedBool aCheckable)
      : mSoftTextOffset(aOffset), mLength(aLength), mCheckableWord(aCheckable) {}
    PRInt32 EndOffset() const { return mSoftTextOffset + mLength; }
  };
  nsTArray<RealWord> mRealWords;
  PRInt32            mNextWordIndex;

  PRPackedBool mSoftTextValid;

  void InvalidateWords() { mSoftTextValid = PR_FALSE; }
  void EnsureWords();
  
  PRInt32 MapDOMPositionToSoftTextOffset(NodeOffset aNodeOffset);
  // Map an offset into mSoftText to a DOM position. Note that two DOM positions
  // can map to the same mSoftText offset, e.g. given nodes A=aaaa and B=bbbb
  // forming aaaabbbb, (A,4) and (B,0) give the same string offset. So,
  // aHintBefore controls which position we return ... if aHint is eEnd
  // then the position indicates the END of a range so we return (A,4). Otherwise
  // the position indicates the START of a range so we return (B,0).
  enum DOMMapHint { HINT_BEGIN, HINT_END };
  NodeOffset MapSoftTextOffsetToDOMPosition(PRInt32 aSoftTextOffset,
                                            DOMMapHint aHint);
  // Finds the index of the real word containing aSoftTextOffset, or -1 if none
  // If it's exactly between two words, then if aHint is HINT_BEGIN, return the
  // later word (favouring the assumption that it's the BEGINning of a word),
  // otherwise return the earlier word (assuming it's the END of a word).
  // If aSearchForward is true, then if we don't find a word at the given
  // position, search forward until we do find a word and return that (if found).
  PRInt32 FindRealWordContaining(PRInt32 aSoftTextOffset, DOMMapHint aHint,
                                 PRBool aSearchForward);
    
  // build mSoftText and mSoftTextDOMMapping
  void BuildSoftText();
  // Build mRealWords array
  void BuildRealWords();

  void SplitDOMWord(PRInt32 aStart, PRInt32 aEnd);

  // Convenience functions, object must be initialized
  nsresult MakeRange(NodeOffset aBegin, NodeOffset aEnd, nsIDOMRange** aRange);
  nsresult MakeRangeForWord(const RealWord& aWord, nsIDOMRange** aRange);
};
