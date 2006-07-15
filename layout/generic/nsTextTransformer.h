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
 * class for transformation of text before rendering, including CSS
 * text-transform
 */

#ifndef nsTextTransformer_h___
#define nsTextTransformer_h___

#include "nsTextFragment.h"
#include "nsISupports.h"
#include "nsPresContext.h"
#include "nsIObserver.h"
#ifdef IBMBIDI
#include "nsBidi.h"
#include "nsBidiUtils.h"
#endif

class nsIContent;
class nsIFrame;
class nsILineBreaker;
class nsIWordBreaker;

// XXX I'm sure there are other special characters
#define CH_NBSP   160
#define CH_ENSP   8194 //<!ENTITY ensp    CDATA "&#8194;" -- en space, U+2002 ISOpub -->
#define CH_EMSP   8195 //<!ENTITY emsp    CDATA "&#8195;" -- em space, U+2003 ISOpub -->
#define CH_THINSP 8291 //<!ENTITY thinsp  CDATA "&#8201;" -- thin space, U+2009 ISOpub -->
#define CH_ZWNJ   8204 //<!ENTITY zwnj    CDATA "&#8204;" -- zero width non-joiner, U+200C NEW RFC 2070
#define CH_SHY    173
#define CH_CJKSP  12288 // U+3000 IDEOGRAPHIC SPACE (CJK Full-Width Space)

#ifdef IBMBIDI
#define CH_ZWJ  8205  //<!ENTITY zwj     CDATA "&#8205;" -- zero width joiner, U+200D NEW RFC 2070 -->
#define CH_LRM  8206  //<!ENTITY lrm     CDATA "&#8206;" -- left-to-right mark, U+200E NEW RFC 2070 -->
#define CH_RLM  8207  //<!ENTITY rlm     CDATA "&#8207;" -- right-to-left mark, U+200F NEW RFC 2070 -->
#define CH_LRE  8234  //<!CDATA "&#8234;" -- left-to-right embedding, U+202A -->
#define CH_RLE  8235  //<!CDATA "&#8235;" -- right-to-left embedding, U+202B -->
#define CH_PDF  8236  //<!CDATA "&#8236;" -- pop directional format, U+202C -->
#define CH_LRO  8237  //<!CDATA "&#8237;" -- left-to-right override, U+202D -->
#define CH_RLO  8238  //<!CDATA "&#8238;" -- right-to-left override, U+202E -->

#define IS_BIDI_CONTROL(_ch) \
  (((_ch) >= CH_LRM && (_ch) <= CH_RLM) \
  || ((_ch) >= CH_LRE && (_ch) <= CH_RLO))
#endif // IBMBIDI

// For now, we have only a couple of characters to strip out. If we get
// any more, change this to use a bitset to lookup into.
//   CH_SHY - soft hyphen (discretionary hyphen)
#ifdef IBMBIDI
// added BIDI formatting codes
#define IS_DISCARDED(_ch) \
  (((_ch) == CH_SHY) || ((_ch) == '\r') || IS_BIDI_CONTROL(_ch))
#else
#define IS_DISCARDED(_ch) \
  (((_ch) == CH_SHY) || ((_ch) == '\r'))
#endif

/* Unicode codepoints that could be the first of a lam alef pair converted to
 * a lam alef ligature by ArabicShaping()
 */
#define IS_LAM(_ch) \
  (((_ch) == 0x0644) ||  /* ARABIC LETTER LAM */ \
   ((_ch) == 0xfedf) ||  /* ARABIC LETTER LAM INITIAL FORM */ \
   ((_ch) == 0xfee0))    /* ARABIC LETTER LAM MEDIAL FORM */  \

/* Unicode codepoints that could be the second of a lam alef pair converted to
 * a lam alef ligature by ArabicShaping()
 */
#define IS_ALEF(_ch) \
  (((_ch) == 0x0622) || /* ARABIC LETTER ALEF WITH MADDA ABOVE */ \
   ((_ch) == 0x0623) || /* ARABIC LETTER ALEF WITH HAMZA ABOVE */ \
   ((_ch) == 0x0625) || /* ARABIC LETTER ALEF WITH HAMZA BELOW */ \
   ((_ch) == 0x0627) || /* ARABIC LETTER ALEF */ \
   ((_ch) == 0xfe82) || /* ARABIC LETTER ALEF WITH MADDA ABOVE FINAL FORM */ \
   ((_ch) == 0xfe84) || /* ARABIC LETTER ALEF WITH HAMZA ABOVE FINAL FORM */ \
   ((_ch) == 0xfe88) || /* ARABIC LETTER ALEF WITH HAMZA BELOW FINAL FORM */ \
   ((_ch) == 0xfe8e))   /* ARABIC LETTER ALEF FINAL FORM */

/* Unicode codepoints that could have been converted from a lam alef pair to a
 * lam alef ligature by Arabic Shaping()
 */
#define IS_LAMALEF(_ch) (((_ch) >= 0xfef5) && ((_ch) <= 0xfefc))
  /* FEF5 ARABIC LIGATURE LAM WITH ALEF WITH MADDA ABOVE ISOLATED FORM */
  /* FEF6 ARABIC LIGATURE LAM WITH ALEF WITH MADDA ABOVE FINAL FORM    */
  /* FEF7 ARABIC LIGATURE LAM WITH ALEF WITH HAMZA ABOVE ISOLATED FORM */
  /* FEF8 ARABIC LIGATURE LAM WITH ALEF WITH HAMZA ABOVE FINAL FORM    */
  /* FEF9 ARABIC LIGATURE LAM WITH ALEF WITH HAMZA BELOW ISOLATED FORM */
  /* FEFA ARABIC LIGATURE LAM WITH ALEF WITH HAMZA BELOW FINAL FORM    */
  /* FEFB ARABIC LIGATURE LAM WITH ALEF ISOLATED FORM                  */
  /* FEFC;ARABIC LIGATURE LAM WITH ALEF FINAL FORM                     */

#define IS_ASCII_CHAR(ch) ((ch&0xff80) == 0)

#define NS_TEXT_TRANSFORMER_AUTO_WORD_BUF_SIZE 128 // used to be 256

// Indicates whether the transformed text should be left as ascii
#define NS_TEXT_TRANSFORMER_LEAVE_AS_ASCII					1

// If at any point during GetNextWord or GetPrevWord we
// run across a multibyte (> 127) unicode character.
#define NS_TEXT_TRANSFORMER_HAS_MULTIBYTE					2

// The text in the transform buffer is ascii
#define NS_TEXT_TRANSFORMER_TRANSFORMED_TEXT_IS_ASCII		4

#ifdef IBMBIDI
// The text in the transform buffer needs Arabic shaping
#define NS_TEXT_TRANSFORMER_DO_ARABIC_SHAPING 8

// The text in the transform buffer needs numeric shaping
#define NS_TEXT_TRANSFORMER_DO_NUMERIC_SHAPING 16
#endif

// A growable text buffer that tries to avoid using malloc by having a
// builtin buffer. Ideally used as an automatic variable.
class nsAutoTextBuffer {
public:
  nsAutoTextBuffer();
  ~nsAutoTextBuffer();

  nsresult GrowBy(PRInt32 aAtLeast, PRBool aCopyToHead = PR_TRUE);

  nsresult GrowTo(PRInt32 aNewSize, PRBool aCopyToHead = PR_TRUE);

  PRUnichar* GetBuffer() { return mBuffer; }
  PRUnichar* GetBufferEnd() { return mBuffer + mBufferLen; }
  PRInt32 GetBufferLength() const { return mBufferLen; }

  PRUnichar* mBuffer;
  PRInt32 mBufferLen;
  PRUnichar mAutoBuffer[NS_TEXT_TRANSFORMER_AUTO_WORD_BUF_SIZE];
};

//----------------------------------------

/**
 * This object manages the transformation of text:
 *
 * <UL>
 * <LI>whitespace compression
 * <LI>capitalization
 * <LI>lowercasing
 * <LI>uppercasing
 * <LI>ascii to Unicode (if requested)
 * <LI>discarded characters
 * <LI>conversion of &nbsp that is not part of whitespace into a space
 * <LI>tab and newline characters to space (normal text only)
 * </UL>
 *
 * Note that no transformations are applied that would impact word
 * breaking (like mapping &nbsp; into space, for example). In
 * addition, this logic will not strip leading or trailing whitespace
 * (across the entire run of text; leading whitespace can be skipped
 * for a frames text because of whitespace compression).
 */
class nsTextTransformer {
public:
  // Note: The text transformer does not hold a reference to the line
  // breaker and work breaker objects
  nsTextTransformer(nsPresContext* aPresContext);

  ~nsTextTransformer();

  /**
   * Initialize the text transform. Use GetNextWord() and GetPrevWord()
   * to iterate the text
   *
   * The default is to transform all text to Unicode; however, you can
   * specify that the text should be left as ascii if possible. Note that
   * we don't step the text down from Unicode to ascii (even if it doesn't
   * contain multibyte characters) so this only happens for text fragments
   * that contain 1-byte text.
   * XXX This is currently not implemented for GetPreviousWord()
   * @see TransformedTextIsAscii()
   */
  nsresult Init(nsIFrame* aFrame,
                nsIContent* aContent,
                PRInt32 aStartingOffset,
                PRBool aForceArabicShaping = PR_FALSE,
                PRBool aLeaveAsAscii = PR_FALSE);

  PRInt32 GetContentLength() const {
    return mFrag ? mFrag->GetLength() : 0;
  }

  PRUnichar GetContentCharAt(PRInt32 aIndex) {
    return (mFrag && aIndex < mFrag->GetLength()) ? mFrag->CharAt(aIndex) : 0;
  }

  /**
   * Iterates the next word in the text fragment.
   *
   * Returns a pointer to the word, the number of characters in the word, the
   * content length of the word, whether it is whitespace, and whether the
   * text was transformed (any of the transformations listed above). The content
   * length can be greater than the word length if whitespace compression occurred
   * or if characters were discarded
   *
   * The default behavior is to reset the transform buffer to the beginning,
   * but you can choose to not reste it and buffer across multiple words
   */
  PRUnichar* GetNextWord(PRBool aInWord,
                         PRInt32* aWordLenResult,
                         PRInt32* aContentLenResult,
                         PRBool* aIsWhitespaceResult,
                         PRBool* aWasTransformed,
                         PRBool aResetTransformBuf = PR_TRUE,
                         PRBool aForLineBreak = PR_TRUE,
                         PRBool aIsKeyboardSelect = PR_FALSE);

  PRUnichar* GetPrevWord(PRBool aInWord,
                         PRInt32* aWordLenResult,
                         PRInt32* aContentLenResult,
                         PRBool* aIsWhitespaceResult,
                         PRBool aForLineBreak = PR_TRUE,
                         PRBool aIsKeyboardSelect = PR_FALSE);

  
  // Returns PR_TRUE if the LEAVE_AS_ASCII flag is set
  PRBool LeaveAsAscii() const {
      return (mFlags & NS_TEXT_TRANSFORMER_LEAVE_AS_ASCII) != 0;
  }

  // Returns PR_TRUE if any of the characters are multibyte (greater than 127)
  PRBool HasMultibyte() const {
      return (mFlags & NS_TEXT_TRANSFORMER_HAS_MULTIBYTE) != 0;
  }

  // Returns PR_TRUE if the text in the transform bufer is ascii (i.e., it
  // doesn't contain any multibyte characters)
  PRBool TransformedTextIsAscii() const {
      return (mFlags & NS_TEXT_TRANSFORMER_TRANSFORMED_TEXT_IS_ASCII) != 0;
  }

#ifdef IBMBIDI
  // Returns PR_TRUE if the text in the transform bufer needs Arabic
  // shaping
  PRBool NeedsArabicShaping() const {
    return (mFlags & NS_TEXT_TRANSFORMER_DO_ARABIC_SHAPING) != 0;
  }
  
  // Returns PR_TRUE if the text in the transform bufer needs numeric
  // shaping
  PRBool NeedsNumericShaping() const {
    return (mFlags & NS_TEXT_TRANSFORMER_DO_NUMERIC_SHAPING) != 0;
  }
#endif

  // Set or clears the LEAVE_AS_ASCII bit
  void SetLeaveAsAscii(PRBool aValue) {
      aValue ? mFlags |= NS_TEXT_TRANSFORMER_LEAVE_AS_ASCII : 
               mFlags &= (~NS_TEXT_TRANSFORMER_LEAVE_AS_ASCII);
  }
      
  // Set or clears the NS_TEXT_TRANSFORMER_HAS_MULTIBYTE bit
  void SetHasMultibyte(PRBool aValue) {
      aValue ? mFlags |= NS_TEXT_TRANSFORMER_HAS_MULTIBYTE : 
               mFlags &= (~NS_TEXT_TRANSFORMER_HAS_MULTIBYTE);
  }

  // Set or clears the NS_TEXT_TRANSFORMER_TRANSFORMED_TEXT_IS_ASCII bit
  void SetTransformedTextIsAscii(PRBool aValue) {
      aValue ? mFlags |= NS_TEXT_TRANSFORMER_TRANSFORMED_TEXT_IS_ASCII : 
               mFlags &= (~NS_TEXT_TRANSFORMER_TRANSFORMED_TEXT_IS_ASCII);
  }

#ifdef IBMBIDI
  // Set or clears the NS_TEXT_TRANSFORMER_TRANSFORMED_DO_ARABIC_SHAPING bit
  void SetNeedsArabicShaping(PRBool aValue) {
    aValue ? mFlags |= NS_TEXT_TRANSFORMER_DO_ARABIC_SHAPING : 
             mFlags &= (~NS_TEXT_TRANSFORMER_DO_ARABIC_SHAPING);
  }

  // Set or clears the NS_TEXT_TRANSFORMER_TRANSFORMED_DO_NUMERIC_SHAPING bit
  void SetNeedsNumericShaping(PRBool aValue) {
    aValue ? mFlags |= NS_TEXT_TRANSFORMER_DO_NUMERIC_SHAPING : 
                       mFlags &= (~NS_TEXT_TRANSFORMER_DO_NUMERIC_SHAPING);
  }
#endif
  
  PRUnichar* GetWordBuffer() {
    return mTransformBuf.GetBuffer();
  }

  PRInt32 GetWordBufferLength() const {
    return mTransformBuf.GetBufferLength();
  }

  static PRBool GetWordSelectEatSpaceAfter() {
  	return sWordSelectEatSpaceAfter;
  }
  
  static PRBool GetWordSelectStopAtPunctuation() {
  	return sWordSelectStopAtPunctuation;
  }
  
  static nsresult Initialize();
  static void Shutdown();

protected:
  // Helper methods for GetNextWord (F == forwards)
  PRInt32 ScanNormalWhiteSpace_F();
  PRInt32 ScanNormalAsciiText_F(PRInt32* aWordLen,
                                PRBool*  aWasTransformed);
  PRInt32 ScanNormalAsciiText_F_ForWordBreak(PRInt32* aWordLen,
                                PRBool*  aWasTransformed,
                                PRBool aIsKeyboardSelect);
  PRInt32 ScanNormalUnicodeText_F(PRBool aForLineBreak,
                                  PRInt32* aWordLen,
                                  PRBool*  aWasTransformed);
  PRInt32 ScanPreWrapWhiteSpace_F(PRInt32* aWordLen);
  PRInt32 ScanPreAsciiData_F(PRInt32* aWordLen,
                             PRBool*  aWasTransformed);
  PRInt32 ScanPreData_F(PRInt32* aWordLen,
                        PRBool*  aWasTransformed);

  // Helper methods for GetPrevWord (B == backwards)
  PRInt32 ScanNormalWhiteSpace_B();
  PRInt32 ScanNormalAsciiText_B(PRInt32* aWordLen, PRBool aIsKeyboardSelect);
  PRInt32 ScanNormalUnicodeText_B(PRBool aForLineBreak, PRInt32* aWordLen);
  PRInt32 ScanPreWrapWhiteSpace_B(PRInt32* aWordLen);
  PRInt32 ScanPreData_B(PRInt32* aWordLen);

  // Converts the current text in the transform buffer from ascii to
  // Unicode
  void ConvertTransformedTextToUnicode();
  
  void LanguageSpecificTransform(PRUnichar* aText, PRInt32 aLen,
                                 PRBool* aWasTransformed);

  void DoArabicShaping(PRUnichar* aText, PRInt32& aTextLength, PRBool* aWasTransformed);

  void DoNumericShaping(PRUnichar* aText, PRInt32& aTextLength, PRBool* aWasTransformed);

  void StripZeroWidthJoinControls(PRUnichar* aSource, PRUnichar* aTarget, PRInt32& aTextLength, PRBool* aWasTransformed);

  // The text fragment that we are looking at
  const nsTextFragment* mFrag;

  // Our current offset into the text fragment
  PRInt32 mOffset;

  // The frame's white-space mode we are using to process text
  enum {
    eNormal,
    ePreformatted,
    ePreWrap
  } mMode;
  
  nsLanguageSpecificTransformType mLanguageSpecificTransformType;

#ifdef IBMBIDI
  nsPresContext* mPresContext;
  nsCharType      mCharType;
#endif

  // Buffer used to hold the transformed words from GetNextWord or
  // GetPrevWord
  nsAutoTextBuffer mTransformBuf;

  // Our current position within the buffer. Used when iterating the next
  // word, because we may be requested to buffer across multiple words
  PRInt32 mBufferPos;
  
  // The frame's text-transform state
  PRUint8 mTextTransform;

  // Flag for controlling mLeaveAsAscii, mHasMultibyte, mTransformedTextIsAscii
  PRUint8 mFlags;

  // prefs used to configure the double-click word selection behavior
  static int WordSelectPrefCallback(const char* aPref, void* aClosure);
  static PRBool sWordSelectListenerPrefChecked;  // have we read the prefs yet?
  static PRBool sWordSelectEatSpaceAfter;        // should we include whitespace up to next word? 
  static PRBool sWordSelectStopAtPunctuation;    // should we stop at punctuation?

#ifdef DEBUG
  static void SelfTest(nsPresContext* aPresContext);

  nsresult Init2(const nsTextFragment* aFrag,
                 PRInt32 aStartingOffset,
                 PRUint8 aWhiteSpace,
                 PRUint8 aTextTransform);
#endif
};

#endif /* nsTextTransformer_h___ */
