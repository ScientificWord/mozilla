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

/*
 * nsIContentSerializer implementation that can be used with an
 * nsIDocumentEncoder to convert a DOM into plaintext in a nice way
 * (eg for copy/paste as plaintext).
 */

#ifndef nsPlainTextSerializer_h__
#define nsPlainTextSerializer_h__

#include "nsIContentSerializer.h"
#include "nsIHTMLContentSink.h"
#include "nsHTMLTags.h"
#include "nsCOMPtr.h"
#include "nsString.h"
#include "nsILineBreaker.h"
#include "nsIContent.h"
#include "nsIAtom.h"
#include "nsIHTMLToTextSink.h"
#include "nsIDocumentEncoder.h"
#include "nsVoidArray.h"


class nsPlainTextSerializer : public nsIContentSerializer,
                              public nsIHTMLContentSink,
                              public nsIHTMLToTextSink
{
public:
  nsPlainTextSerializer();
  virtual ~nsPlainTextSerializer();

  NS_DECL_ISUPPORTS

  // nsIContentSerializer
  NS_IMETHOD Init(PRUint32 flags, PRUint32 aWrapColumn,
                  const char* aCharSet, PRBool aIsCopying,
                  PRBool aIsWholeDocument);

  NS_IMETHOD AppendText(nsIDOMText* aText, PRInt32 aStartOffset,
                        PRInt32 aEndOffset, nsAString& aStr);
  NS_IMETHOD AppendCDATASection(nsIDOMCDATASection* aCDATASection,
                                PRInt32 aStartOffset, PRInt32 aEndOffset,
                                nsAString& aStr);
  NS_IMETHOD AppendProcessingInstruction(nsIDOMProcessingInstruction* aPI,
                                         PRInt32 aStartOffset,
                                         PRInt32 aEndOffset,
                                         nsAString& aStr)  { return NS_OK; }
  NS_IMETHOD AppendComment(nsIDOMComment* aComment, PRInt32 aStartOffset,
                           PRInt32 aEndOffset, nsAString& aStr)  { return NS_OK; }
  NS_IMETHOD AppendDoctype(nsIDOMDocumentType *aDoctype,
                           nsAString& aStr)  { return NS_OK; }
  NS_IMETHOD AppendElementStart(nsIDOMElement *aElement,
                                nsIDOMElement *aOriginalElement,
                                nsAString& aStr); 
  NS_IMETHOD AppendElementEnd(nsIDOMElement *aElement,
                              nsAString& aStr);
  NS_IMETHOD Flush(nsAString& aStr);

  NS_IMETHOD AppendDocumentStart(nsIDOMDocument *aDocument,
                                 nsAString& aStr);

  // nsIContentSink
  NS_IMETHOD WillTokenize(void) { return NS_OK; }
  NS_IMETHOD WillBuildModel(void) { return NS_OK; }
  NS_IMETHOD DidBuildModel(void) { return NS_OK; }
  NS_IMETHOD WillInterrupt(void) { return NS_OK; }
  NS_IMETHOD WillResume(void) { return NS_OK; }
  NS_IMETHOD SetParser(nsIParser* aParser) { return NS_OK; }
  NS_IMETHOD OpenContainer(const nsIParserNode& aNode);
  NS_IMETHOD CloseContainer(const nsHTMLTag aTag);
  NS_IMETHOD AddLeaf(const nsIParserNode& aNode);
  NS_IMETHOD AddComment(const nsIParserNode& aNode) { return NS_OK; }
  NS_IMETHOD AddProcessingInstruction(const nsIParserNode& aNode) { return NS_OK; }
  NS_IMETHOD AddDocTypeDecl(const nsIParserNode& aNode) { return NS_OK; }
  virtual void FlushPendingNotifications(mozFlushType aType) { }
  NS_IMETHOD SetDocumentCharset(nsACString& aCharset) { return NS_OK; }
  virtual nsISupports *GetTarget() { return nsnull; }

  // nsIHTMLContentSink
  NS_IMETHOD OpenHead();
  NS_IMETHOD IsEnabled(PRInt32 aTag, PRBool* aReturn);
  NS_IMETHOD NotifyTagObservers(nsIParserNode* aNode) { return NS_OK; }
  NS_IMETHOD_(PRBool) IsFormOnStack() { return PR_FALSE; }

  NS_IMETHOD BeginContext(PRInt32 aPosition) { return NS_OK; }
  NS_IMETHOD EndContext(PRInt32 aPosition) { return NS_OK; }
  NS_IMETHOD WillProcessTokens(void) { return NS_OK; }
  NS_IMETHOD DidProcessTokens(void) { return NS_OK; }
  NS_IMETHOD WillProcessAToken(void) { return NS_OK; }
  NS_IMETHOD DidProcessAToken(void) { return NS_OK; }

  // nsIHTMLToTextSink
  NS_IMETHOD Initialize(nsAString* aOutString,
                        PRUint32 aFlags, PRUint32 aWrapCol);

protected:
  nsresult GetAttributeValue(const nsIParserNode* node, nsIAtom* aName, nsString& aValueRet);
  void AddToLine(const PRUnichar* aStringToAdd, PRInt32 aLength);
  void EndLine(PRBool softlinebreak);
  void EnsureVerticalSpace(PRInt32 noOfRows);
  void FlushLine();
  void OutputQuotesAndIndent(PRBool stripTrailingSpaces=PR_FALSE);
  void Output(nsString& aString);
  void Write(const nsAString& aString);
  PRBool IsBlockLevel(PRInt32 aId);
  PRBool IsContainer(PRInt32 aId);
  PRBool IsInPre();
  PRBool IsInOL();
  PRBool IsCurrentNodeConverted(const nsIParserNode* aNode);
  static PRInt32 GetIdForContent(nsIContent* aContent);
  nsresult DoOpenContainer(const nsIParserNode* aNode, PRInt32 aTag);
  nsresult DoCloseContainer(PRInt32 aTag);
  nsresult DoAddLeaf(const nsIParserNode* aNode,
                     PRInt32 aTag,
                     const nsAString& aText);

  // Inlined functions
  inline PRBool MayWrap()
  {
    return mWrapColumn &&
      ((mFlags & nsIDocumentEncoder::OutputFormatted) ||
       (mFlags & nsIDocumentEncoder::OutputWrap));
  }

  inline PRBool DoOutput()
  {
    return mHeadLevel == 0;
  }

  // Stack handling functions
  PRBool GetLastBool(const nsVoidArray& aStack);
  void SetLastBool(nsVoidArray& aStack, PRBool aValue);
  void PushBool(nsVoidArray& aStack, PRBool aValue);
  PRBool PopBool(nsVoidArray& aStack);
  
protected:
  nsString         mCurrentLine;
  PRUint32         mHeadLevel;
  PRPackedBool     mAtFirstColumn;

  // Handling of quoted text (for mail):
  // Quotes need to be wrapped differently from non-quoted text,
  // because quoted text has a few extra characters (e.g. ">> ")
  // which makes the line length longer.
  // Mail can represent quotes in different ways: it can wrap
  // quotes in a <pre> (if editor.quotesPreformatted is set),
  // or not wrapped in any special tag (if mail.compose.wrap_to_window_width)
  // or in a <span> (if neither of the above are set).
  PRPackedBool     mQuotesPreformatted; // expect quotes wrapped in <pre>
  PRPackedBool     mDontWrapAnyQuotes;  // no special quote markers

  PRPackedBool     mStructs;            // Output structs (pref)

  // If we've just written out a cite blockquote, we need to remember it
  // so we don't duplicate spaces before a <pre wrap> (which mail uses to quote
  // old messages).
  PRPackedBool     mHasWrittenCiteBlockquote;

  PRInt32          mIndent;
  // mInIndentString keeps a header that has to be written in the indent.
  // That could be, for instance, the bullet in a bulleted list.
  nsString         mInIndentString;
  PRInt32          mCiteQuoteLevel;
  PRInt32          mFlags;
  PRInt32          mFloatingLines; // To store the number of lazy line breaks

  // The wrap column is how many standard sized chars (western languages)
  // should be allowed on a line. There could be less chars if the chars
  // are wider than latin chars of more if the chars are more narrow.
  PRUint32         mWrapColumn;

  // The width of the line as it will appear on the screen (approx.) 
  PRUint32         mCurrentLineWidth; 

  // Treat quoted text as though it's preformatted -- don't wrap it.
  // Having it on a pref is a temporary measure, See bug 69638.
  PRInt32          mSpanLevel;


  PRInt32          mEmptyLines; // Will be the number of empty lines before
                                // the current. 0 if we are starting a new
                                // line and -1 if we are in a line.

  PRPackedBool     mInWhitespace;
  PRPackedBool     mPreFormatted;
  PRPackedBool     mStartedOutput; // we've produced at least a character

  // While handling a new tag, this variable should remind if any line break
  // is due because of a closing tag. Setting it to "TRUE" while closing the tags.
  // Hence opening tags are guaranteed to start with appropriate line breaks.
  PRPackedBool     mLineBreakDue; 

  nsString         mURL;
  PRInt32          mHeaderStrategy;    /* Header strategy (pref)
                                          0 = no indention
                                          1 = indention, increased with
                                              header level (default)
                                          2 = numbering and slight indention */
  PRInt32          mHeaderCounter[7];  /* For header-numbering:
                                          Number of previous headers of
                                          the same depth and in the same
                                          section.
                                          mHeaderCounter[1] for <h1> etc. */

  nsCOMPtr<nsIContent> mContent;

  // For handling table rows
  nsAutoVoidArray mHasWrittenCellsForRow; // really an array of bools
  
  // Values gotten in OpenContainer that is (also) needed in CloseContainer
  nsAutoVoidArray     mCurrentNodeIsConverted; // really an array of bools
  nsAutoVoidArray     mIsInCiteBlockquote; // really an array of bools

  // The output data
  nsAString*            mOutputString;

  // The tag stack: the stack of tags we're operating on, so we can nest:
  nsHTMLTag       *mTagStack;
  PRUint32         mTagStackIndex;

  // Content in the stack above this index should be ignored:
  PRUint32          mIgnoreAboveIndex;

  // The stack for ordered lists:
  PRInt32         *mOLStack;
  PRUint32         mOLStackIndex;

  PRUint32         mULCount;

  nsString                     mLineBreak;
  nsCOMPtr<nsILineBreaker>     mLineBreaker;

  // Conveniance constant. It would be nice to have it as a const static
  // variable, but that causes issues with OpenBSD and module unloading.
  const nsString          kSpace;
};

nsresult
NS_NewPlainTextSerializer(nsIContentSerializer** aSerializer);

#endif
