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
 *   L. David Baron <dbaron@dbaron.org>
 *   Daniel Glazman <glazman@netscape.com>
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

/* tokenization of CSS style sheets */

#include "nsCSSScanner.h"
#include "nsIFactory.h"
#include "nsIInputStream.h"
#include "nsIUnicharInputStream.h"
#include "nsString.h"
#include "nsCRT.h"

// for #ifdef CSS_REPORT_PARSE_ERRORS
#include "nsCOMPtr.h"
#include "nsIServiceManager.h"
#include "nsIComponentManager.h"
#include "nsReadableUtils.h"
#include "nsIURI.h"
#include "nsIConsoleService.h"
#include "nsIScriptError.h"
#include "nsIStringBundle.h"
#include "nsContentUtils.h"

// Don't bother collecting whitespace characters in token's mIdent buffer
#undef COLLECT_WHITESPACE

#define BUFFER_SIZE 256

static const PRUnichar CSS_ESCAPE = PRUnichar('\\');
const PRUint8 nsCSSScanner::IS_DIGIT = 0x01;
const PRUint8 nsCSSScanner::IS_HEX_DIGIT = 0x02;
const PRUint8 nsCSSScanner::START_IDENT = 0x04;
const PRUint8 nsCSSScanner::IS_IDENT = 0x08;
const PRUint8 nsCSSScanner::IS_WHITESPACE = 0x10;

static PRBool gLexTableSetup = PR_FALSE;
PRUint8 nsCSSScanner::gLexTable[256];

#ifdef CSS_REPORT_PARSE_ERRORS
static PRBool gReportErrors = PR_TRUE;
static nsIConsoleService *gConsoleService;
static nsIFactory *gScriptErrorFactory;
static nsIStringBundle *gStringBundle;
#endif

/* static */
void
nsCSSScanner::BuildLexTable()
{
  gLexTableSetup = PR_TRUE;

  PRUint8* lt = gLexTable;
  int i;
  lt[CSS_ESCAPE] = START_IDENT;
  lt['-'] |= IS_IDENT;
  lt['_'] |= IS_IDENT | START_IDENT;
  // XXX add in other whitespace chars
  lt[' '] |= IS_WHITESPACE;   // space
  lt['\t'] |= IS_WHITESPACE;  // horizontal tab
  lt['\v'] |= IS_WHITESPACE;  // vertical tab
  lt['\r'] |= IS_WHITESPACE;  // carriage return
  lt['\n'] |= IS_WHITESPACE;  // line feed
  lt['\f'] |= IS_WHITESPACE;  // form feed
  for (i = 161; i <= 255; i++) {
    lt[i] |= IS_IDENT | START_IDENT;
  }
  for (i = '0'; i <= '9'; i++) {
    lt[i] |= IS_DIGIT | IS_HEX_DIGIT | IS_IDENT;
  }
  for (i = 'A'; i <= 'Z'; i++) {
    if ((i >= 'A') && (i <= 'F')) {
      lt[i] |= IS_HEX_DIGIT;
      lt[i+32] |= IS_HEX_DIGIT;
    }
    lt[i] |= IS_IDENT | START_IDENT;
    lt[i+32] |= IS_IDENT | START_IDENT;
  }
}

nsCSSToken::nsCSSToken()
{
  mType = eCSSToken_Symbol;
}

void 
nsCSSToken::AppendToString(nsString& aBuffer)
{
  switch (mType) {
    case eCSSToken_AtKeyword:
      aBuffer.Append(PRUnichar('@')); // fall through intentional
    case eCSSToken_Ident:
    case eCSSToken_WhiteSpace:
    case eCSSToken_Function:
    case eCSSToken_URL:
    case eCSSToken_InvalidURL:
    case eCSSToken_HTMLComment:
      aBuffer.Append(mIdent);
      break;
    case eCSSToken_Number:
      if (mIntegerValid) {
        aBuffer.AppendInt(mInteger, 10);
      }
      else {
        aBuffer.AppendFloat(mNumber);
      }
      break;
    case eCSSToken_Percentage:
      NS_ASSERTION(!mIntegerValid, "How did a percentage token get this set?");
      aBuffer.AppendFloat(mNumber * 100.0f);
      aBuffer.Append(PRUnichar('%')); // STRING USE WARNING: technically, this should be |AppendWithConversion|
      break;
    case eCSSToken_Dimension:
      if (mIntegerValid) {
        aBuffer.AppendInt(mInteger, 10);
      }
      else {
        aBuffer.AppendFloat(mNumber);
      }
      aBuffer.Append(mIdent);
      break;
    case eCSSToken_String:
      aBuffer.Append(mSymbol);
      aBuffer.Append(mIdent); // fall through intentional
    case eCSSToken_Symbol:
      aBuffer.Append(mSymbol);
      break;
    case eCSSToken_ID:
    case eCSSToken_Ref:
      aBuffer.Append(PRUnichar('#'));
      aBuffer.Append(mIdent);
      break;
    case eCSSToken_Includes:
      aBuffer.AppendLiteral("~=");
      break;
    case eCSSToken_Dashmatch:
      aBuffer.AppendLiteral("|=");
      break;
    case eCSSToken_Error:
      aBuffer.Append(mSymbol);
      aBuffer.Append(mIdent);
      break;
    default:
      NS_ERROR("invalid token type");
      break;
  }
}

nsCSSScanner::nsCSSScanner()
  : mInputStream(nsnull)
  , mReadPointer(nsnull)
#ifdef CSS_REPORT_PARSE_ERRORS
  , mError(mErrorBuf, NS_ARRAY_LENGTH(mErrorBuf), 0)
#endif
{
  MOZ_COUNT_CTOR(nsCSSScanner);
  if (!gLexTableSetup) {
    // XXX need a monitor
    BuildLexTable();
  }
  mPushback = mLocalPushback;
  mPushbackSize = 4;
  // No need to init the other members, since they represent state
  // which can get cleared.  We'll init them every time Init() is
  // called.
}

nsCSSScanner::~nsCSSScanner()
{
  MOZ_COUNT_DTOR(nsCSSScanner);
  Close();
  if (mLocalPushback != mPushback) {
    delete [] mPushback;
  }
}

#ifdef CSS_REPORT_PARSE_ERRORS
#define CSS_ERRORS_PREF "layout.css.report_errors"

PR_STATIC_CALLBACK(int) CSSErrorsPrefChanged(const char *aPref, void *aClosure)
{
  gReportErrors = nsContentUtils::GetBoolPref(CSS_ERRORS_PREF, PR_TRUE);
  return NS_OK;
}
#endif

/* static */ PRBool nsCSSScanner::InitGlobals()
{
#ifdef CSS_REPORT_PARSE_ERRORS
  if (gConsoleService && gScriptErrorFactory)
    return PR_TRUE;
  
  nsresult rv = CallGetService(NS_CONSOLESERVICE_CONTRACTID, &gConsoleService);
  NS_ENSURE_SUCCESS(rv, PR_FALSE);

  rv = CallGetClassObject(NS_SCRIPTERROR_CONTRACTID, &gScriptErrorFactory);
  NS_ENSURE_SUCCESS(rv, PR_FALSE);
  NS_ASSERTION(gConsoleService && gScriptErrorFactory,
               "unexpected null pointer without failure");

  nsContentUtils::RegisterPrefCallback(CSS_ERRORS_PREF, CSSErrorsPrefChanged, nsnull);
  CSSErrorsPrefChanged(CSS_ERRORS_PREF, nsnull);
#endif
  return PR_TRUE;
}

/* static */ void nsCSSScanner::ReleaseGlobals()
{
#ifdef CSS_REPORT_PARSE_ERRORS
  nsContentUtils::UnregisterPrefCallback(CSS_ERRORS_PREF, CSSErrorsPrefChanged, nsnull);
  NS_IF_RELEASE(gConsoleService);
  NS_IF_RELEASE(gScriptErrorFactory);
  NS_IF_RELEASE(gStringBundle);
#endif
}

void nsCSSScanner::Init(nsIUnicharInputStream* aInput, 
                        const PRUnichar * aBuffer, PRInt32 aCount, 
                        nsIURI* aURI, PRUint32 aLineNumber)
{
  NS_PRECONDITION(!mInputStream, "Should not have an existing input stream!");
  NS_PRECONDITION(!mReadPointer, "Should not have an existing input buffer!");

  // Read from stream via my own buffer
  if (aInput) {
    NS_PRECONDITION(!aBuffer, "Shouldn't have both input and buffer!");
    NS_PRECONDITION(aCount == 0, "Shouldn't have count with a stream");
    mInputStream = aInput;
    mReadPointer = mBuffer;
    mCount = 0;
  } else {
    NS_PRECONDITION(aBuffer, "Either aInput or aBuffer must be set");
    // Read directly from the provided buffer
    mInputStream = nsnull;
    mReadPointer = aBuffer;
    mCount = aCount;
  }

#ifdef CSS_REPORT_PARSE_ERRORS
  // If aURI is the same as mURI, no need to reget mFileName -- it
  // shouldn't have changed.
  if (aURI != mURI) {
    mURI = aURI;
    if (aURI) {
      aURI->GetSpec(mFileName);
    } else {
      mFileName.Adopt(nsCRT::strdup("from DOM"));
    }
  }
#endif // CSS_REPORT_PARSE_ERRORS
  mLineNumber = aLineNumber;

  // Reset variables that we use to keep track of our progress through the input
  mOffset = 0;
  mPushbackCount = 0;
  mLastRead = 0;

#ifdef CSS_REPORT_PARSE_ERRORS
  mColNumber = 0;
#endif
}

#ifdef CSS_REPORT_PARSE_ERRORS

// @see REPORT_UNEXPECTED_EOF in nsCSSParser.cpp
#define REPORT_UNEXPECTED_EOF(lf_) \
  ReportUnexpectedEOF(#lf_)

void nsCSSScanner::AddToError(const nsSubstring& aErrorText)
{
  if (mError.IsEmpty()) {
    mErrorLineNumber = mLineNumber;
    mErrorColNumber = mColNumber;
    mError = aErrorText;
  } else {
    mError.Append(NS_LITERAL_STRING("  ") + aErrorText);
  }
}

void nsCSSScanner::ClearError()
{
  mError.Truncate();
}

void nsCSSScanner::OutputError()
{
  if (mError.IsEmpty()) return;
 
#ifdef DEBUG
  fprintf(stderr, "CSS Error (%s :%u.%u): %s\n",
                  mFileName.get(), mErrorLineNumber, mErrorColNumber,
                  NS_ConvertUTF16toUTF8(mError).get());
#endif

  // Log it to the Error console

  if (InitGlobals() && gReportErrors) {
    nsresult rv;
    nsCOMPtr<nsIScriptError> errorObject =
      do_CreateInstance(gScriptErrorFactory, &rv);
    if (NS_SUCCEEDED(rv)) {
      rv = errorObject->Init(mError.get(),
                             NS_ConvertUTF8toUTF16(mFileName).get(),
                             EmptyString().get(),
                             mErrorLineNumber,
                             mErrorColNumber,
                             nsIScriptError::warningFlag,
                             "CSS Parser");
      if (NS_SUCCEEDED(rv))
        gConsoleService->LogMessage(errorObject);
    }
  }
  ClearError();
}

static PRBool InitStringBundle()
{
  if (gStringBundle)
    return PR_TRUE;

  nsCOMPtr<nsIStringBundleService> sbs =
    do_GetService(NS_STRINGBUNDLE_CONTRACTID);
  if (!sbs)
    return PR_FALSE;

  nsresult rv = 
    sbs->CreateBundle("chrome://global/locale/css.properties", &gStringBundle);
  if (NS_FAILED(rv)) {
    gStringBundle = nsnull;
    return PR_FALSE;
  }

  return PR_TRUE;
}

#define ENSURE_STRINGBUNDLE \
  PR_BEGIN_MACRO if (!InitStringBundle()) return; PR_END_MACRO

// aMessage must take no parameters
void nsCSSScanner::ReportUnexpected(const char* aMessage)
{
  ENSURE_STRINGBUNDLE;

  nsXPIDLString str;
  gStringBundle->GetStringFromName(NS_ConvertASCIItoUTF16(aMessage).get(),
                                   getter_Copies(str));
  AddToError(str);
}
  
void nsCSSScanner::ReportUnexpectedParams(const char* aMessage,
                                          const PRUnichar **aParams,
                                          PRUint32 aParamsLength)
{
  NS_PRECONDITION(aParamsLength > 0, "use the non-params version");
  ENSURE_STRINGBUNDLE;

  nsXPIDLString str;
  gStringBundle->FormatStringFromName(NS_ConvertASCIItoUTF16(aMessage).get(),
                                      aParams, aParamsLength,
                                      getter_Copies(str));
  AddToError(str);
}

// aMessage must take no parameters
void nsCSSScanner::ReportUnexpectedEOF(const char* aLookingFor)
{
  ENSURE_STRINGBUNDLE;

  nsXPIDLString innerStr;
  gStringBundle->GetStringFromName(NS_ConvertASCIItoUTF16(aLookingFor).get(),
                                   getter_Copies(innerStr));

  const PRUnichar *params[] = {
    innerStr.get()
  };
  nsXPIDLString str;
  gStringBundle->FormatStringFromName(NS_LITERAL_STRING("PEUnexpEOF").get(),
                                      params, NS_ARRAY_LENGTH(params),
                                      getter_Copies(str));
  AddToError(str);
}

// aMessage must take 1 parameter (for the string representation of the
// unexpected token)
void nsCSSScanner::ReportUnexpectedToken(nsCSSToken& tok,
                                         const char *aMessage)
{
  ENSURE_STRINGBUNDLE;
  
  nsAutoString tokenString;
  tok.AppendToString(tokenString);

  const PRUnichar *params[] = {
    tokenString.get()
  };

  ReportUnexpectedParams(aMessage, params, NS_ARRAY_LENGTH(params));
}

// aParams's first entry must be null, and we'll fill in the token
void nsCSSScanner::ReportUnexpectedTokenParams(nsCSSToken& tok,
                                               const char* aMessage,
                                               const PRUnichar **aParams,
                                               PRUint32 aParamsLength)
{
  NS_PRECONDITION(aParamsLength > 1, "use the non-params version");
  NS_PRECONDITION(aParams[0] == nsnull, "first param should be empty");

  ENSURE_STRINGBUNDLE;
  
  nsAutoString tokenString;
  tok.AppendToString(tokenString);
  aParams[0] = tokenString.get();

  ReportUnexpectedParams(aMessage, aParams, aParamsLength);
}

#else

#define REPORT_UNEXPECTED_EOF(lf_)

#endif // CSS_REPORT_PARSE_ERRORS

void nsCSSScanner::Close()
{
  mInputStream = nsnull;
  mReadPointer = nsnull;
}

#ifdef CSS_REPORT_PARSE_ERRORS
#define TAB_STOP_WIDTH 8
#endif

// Returns -1 on error or eof
PRInt32 nsCSSScanner::Read(nsresult& aErrorCode)
{
  PRInt32 rv;
  if (0 < mPushbackCount) {
    rv = PRInt32(mPushback[--mPushbackCount]);
  } else {
    if (mCount < 0) {
      return -1;
    }
    if (mOffset == mCount) {
      mOffset = 0;
      if (!mInputStream) {
        mCount = 0;
        return -1;
      }
      aErrorCode = mInputStream->Read(mBuffer, CSS_BUFFER_SIZE, (PRUint32*)&mCount);
      if (NS_FAILED(aErrorCode) || mCount == 0) {
        mCount = 0;
        return -1;
      }
    }
    rv = PRInt32(mReadPointer[mOffset++]);
    if (((rv == '\n') && (mLastRead != '\r')) || (rv == '\r')) {
      // 0 is a magical line number meaning that we don't know (i.e., script)
      if (mLineNumber != 0)
        ++mLineNumber;
#ifdef CSS_REPORT_PARSE_ERRORS
      mColNumber = 0;
#endif
    } 
#ifdef CSS_REPORT_PARSE_ERRORS
    else if (rv == '\t') {
      mColNumber = ((mColNumber - 1 + TAB_STOP_WIDTH) / TAB_STOP_WIDTH)
                   * TAB_STOP_WIDTH;
    } else if (rv != '\n') {
      mColNumber++;
    }
#endif
  }
  mLastRead = rv;
//printf("Read => %x\n", rv);
  return rv;
}

PRInt32 nsCSSScanner::Peek(nsresult& aErrorCode)
{
  if (0 == mPushbackCount) {
    PRInt32 savedLastRead = mLastRead;
    PRInt32 ch = Read(aErrorCode);
    mLastRead = savedLastRead;
    if (ch < 0) {
      return -1;
    }
    mPushback[0] = PRUnichar(ch);
    mPushbackCount++;
  }
//printf("Peek => %x\n", mLookAhead);
  return PRInt32(mPushback[mPushbackCount - 1]);
}

void nsCSSScanner::Unread()
{
  NS_PRECONDITION((mLastRead >= 0), "double pushback");
  Pushback(PRUnichar(mLastRead));
  mLastRead = -1;
}

void nsCSSScanner::Pushback(PRUnichar aChar)
{
  if (mPushbackCount == mPushbackSize) { // grow buffer
    PRUnichar*  newPushback = new PRUnichar[mPushbackSize + 4];
    if (nsnull == newPushback) {
      return;
    }
    mPushbackSize += 4;
    memcpy(newPushback, mPushback, sizeof(PRUnichar) * mPushbackCount);
    if (mPushback != mLocalPushback) {
      delete [] mPushback;
    }
    mPushback = newPushback;
  }
  mPushback[mPushbackCount++] = aChar;
}

PRBool nsCSSScanner::LookAhead(nsresult& aErrorCode, PRUnichar aChar)
{
  PRInt32 ch = Read(aErrorCode);
  if (ch < 0) {
    return PR_FALSE;
  }
  if (ch == aChar) {
    return PR_TRUE;
  }
  Unread();
  return PR_FALSE;
}

PRBool nsCSSScanner::EatWhiteSpace(nsresult& aErrorCode)
{
  PRBool eaten = PR_FALSE;
  for (;;) {
    PRInt32 ch = Read(aErrorCode);
    if (ch < 0) {
      break;
    }
    if ((ch == ' ') || (ch == '\n') || (ch == '\r') || (ch == '\t')) {
      eaten = PR_TRUE;
      continue;
    }
    Unread();
    break;
  }
  return eaten;
}

PRBool nsCSSScanner::EatNewline(nsresult& aErrorCode)
{
  PRInt32 ch = Read(aErrorCode);
  if (ch < 0) {
    return PR_FALSE;
  }
  PRBool eaten = PR_FALSE;
  if (ch == '\r') {
    eaten = PR_TRUE;
    ch = Peek(aErrorCode);
    if (ch == '\n') {
      (void) Read(aErrorCode);
    }
  } else if (ch == '\n') {
    eaten = PR_TRUE;
  } else {
    Unread();
  }
  return eaten;
}

/* static */
PRBool
nsCSSScanner::CheckLexTable(PRInt32 aChar, PRUint8 aBit, PRUint8* aLexTable)
{
  NS_ASSERTION(!(aBit & (START_IDENT | IS_IDENT)),
               "can't use CheckLexTable with identifiers");
  return aChar >= 0 && aChar < 256 && (aLexTable[aChar] & aBit) != 0;
}

PRBool nsCSSScanner::Next(nsresult& aErrorCode, nsCSSToken& aToken)
{
  PRInt32 ch = Read(aErrorCode);
  if (ch < 0) {
    return PR_FALSE;
  }
  PRUint8* lexTable = gLexTable;

  // IDENT
  if (StartsIdent(ch, Peek(aErrorCode), lexTable))
    return ParseIdent(aErrorCode, ch, aToken);

  // From this point on, 0 <= ch < 256.
     
  // AT_KEYWORD
  if (ch == '@') {
    PRInt32 nextChar = Read(aErrorCode);
    PRInt32 followingChar = Peek(aErrorCode);
    Pushback(nextChar);
    if (StartsIdent(nextChar, followingChar, lexTable))
      return ParseAtKeyword(aErrorCode, ch, aToken);
  }

  // NUMBER or DIM
  if ((ch == '.') || (ch == '+') || (ch == '-')) {
    PRInt32 nextChar = Peek(aErrorCode);
    if (CheckLexTable(nextChar, IS_DIGIT, lexTable)) {
      return ParseNumber(aErrorCode, ch, aToken);
    }
    else if (('.' == nextChar) && ('.' != ch)) {
      nextChar = Read(aErrorCode);
      PRInt32 followingChar = Peek(aErrorCode);
      Pushback(nextChar);
      if (CheckLexTable(followingChar, IS_DIGIT, lexTable))
        return ParseNumber(aErrorCode, ch, aToken);
    }
  }
  if ((lexTable[ch] & IS_DIGIT) != 0) {
    return ParseNumber(aErrorCode, ch, aToken);
  }

  // ID
  if (ch == '#') {
    return ParseRef(aErrorCode, ch, aToken);
  }

  // STRING
  if ((ch == '"') || (ch == '\'')) {
    return ParseString(aErrorCode, ch, aToken);
  }

  // WS
  if ((lexTable[ch] & IS_WHITESPACE) != 0) {
    aToken.mType = eCSSToken_WhiteSpace;
    aToken.mIdent.Assign(PRUnichar(ch));
    (void) EatWhiteSpace(aErrorCode);
    return PR_TRUE;
  }
  if (ch == '/') {
    PRInt32 nextChar = Peek(aErrorCode);
    if (nextChar == '*') {
      (void) Read(aErrorCode);
#if 0
      // If we change our storage data structures such that comments are
      // stored (for Editor), we should reenable this code, condition it
      // on being in editor mode, and apply glazou's patch from bug
      // 60290.
      aToken.mIdent.SetCapacity(2);
      aToken.mIdent.Assign(PRUnichar(ch));
      aToken.mIdent.Append(PRUnichar(nextChar));
      return ParseCComment(aErrorCode, aToken);
#endif
      return SkipCComment(aErrorCode) && Next(aErrorCode, aToken);
    }
  }
  if (ch == '<') {  // consume HTML comment tags
    if (LookAhead(aErrorCode, '!')) {
      if (LookAhead(aErrorCode, '-')) {
        if (LookAhead(aErrorCode, '-')) {
          aToken.mType = eCSSToken_HTMLComment;
          aToken.mIdent.AssignLiteral("<!--");
          return PR_TRUE;
        }
        Pushback('-');
      }
      Pushback('!');
    }
  }
  if (ch == '-') {  // check for HTML comment end
    if (LookAhead(aErrorCode, '-')) {
      if (LookAhead(aErrorCode, '>')) {
        aToken.mType = eCSSToken_HTMLComment;
        aToken.mIdent.AssignLiteral("-->");
        return PR_TRUE;
      }
      Pushback('-');
    }
  }

  // INCLUDES ("~=") and DASHMATCH ("|=")
  if (( ch == '|' ) || ( ch == '~' ) || ( ch == '^' ) ||
      ( ch == '$' ) || ( ch == '*' )) {
    PRInt32 nextChar = Read(aErrorCode);
    if ( nextChar == '=' ) {
      if (ch == '~') {
        aToken.mType = eCSSToken_Includes;
      }
      else if (ch == '|') {
        aToken.mType = eCSSToken_Dashmatch;
      }
      else if (ch == '^') {
        aToken.mType = eCSSToken_Beginsmatch;
      }
      else if (ch == '$') {
        aToken.mType = eCSSToken_Endsmatch;
      }
      else if (ch == '*') {
        aToken.mType = eCSSToken_Containsmatch;
      }
      return PR_TRUE;
    } else {
      Pushback(nextChar);
    }
  }
  aToken.mType = eCSSToken_Symbol;
  aToken.mSymbol = ch;
  return PR_TRUE;
}

PRBool nsCSSScanner::NextURL(nsresult& aErrorCode, nsCSSToken& aToken)
{
  PRInt32 ch = Read(aErrorCode);
  if (ch < 0) {
    return PR_FALSE;
  }
  if (ch < 256) {
    PRUint8* lexTable = gLexTable;

    // STRING
    if ((ch == '"') || (ch == '\'')) {
      return ParseString(aErrorCode, ch, aToken);
    }

    // WS
    if ((lexTable[ch] & IS_WHITESPACE) != 0) {
      aToken.mType = eCSSToken_WhiteSpace;
      aToken.mIdent.Assign(PRUnichar(ch));
      (void) EatWhiteSpace(aErrorCode);
      return PR_TRUE;
    }
    if (ch == '/') {
      PRInt32 nextChar = Peek(aErrorCode);
      if (nextChar == '*') {
        (void) Read(aErrorCode);
#if 0
        // If we change our storage data structures such that comments are
        // stored (for Editor), we should reenable this code, condition it
        // on being in editor mode, and apply glazou's patch from bug
        // 60290.
        aToken.mIdent.SetCapacity(2);
        aToken.mIdent.Assign(PRUnichar(ch));
        aToken.mIdent.Append(PRUnichar(nextChar));
        return ParseCComment(aErrorCode, aToken);
#endif
        return SkipCComment(aErrorCode) && Next(aErrorCode, aToken);
      }
    }

    // Process a url lexical token. A CSS1 url token can contain
    // characters beyond identifier characters (e.g. '/', ':', etc.)
    // Because of this the normal rules for tokenizing the input don't
    // apply very well. To simplify the parser and relax some of the
    // requirements on the scanner we parse url's here. If we find a
    // malformed URL then we emit a token of type "InvalidURL" so that
    // the CSS1 parser can ignore the invalid input. We attempt to eat
    // the right amount of input data when an invalid URL is presented.

    aToken.mType = eCSSToken_InvalidURL;
    nsString& ident = aToken.mIdent;
    ident.SetLength(0);

    if (ch == ')') {
      Pushback(ch);
      // empty url spec; just get out of here
      aToken.mType = eCSSToken_URL;
    } else {
      // start of a non-quoted url
      Pushback(ch);
      PRBool ok = PR_TRUE;
      for (;;) {
        ch = Read(aErrorCode);
        if (ch < 0) break;
        if (ch == CSS_ESCAPE) {
          ParseAndAppendEscape(aErrorCode, ident);
        } else if ((ch == '"') || (ch == '\'') || (ch == '(')) {
          // This is an invalid URL spec
          ok = PR_FALSE;
        } else if ((256 > ch) && ((gLexTable[ch] & IS_WHITESPACE) != 0)) {
          // Whitespace is allowed at the end of the URL
          (void) EatWhiteSpace(aErrorCode);
          if (LookAhead(aErrorCode, ')')) {
            Pushback(')');  // leave the closing symbol
            // done!
            break;
          }
          // Whitespace is followed by something other than a
          // ")". This is an invalid url spec.
          ok = PR_FALSE;
        } else if (ch == ')') {
          Unread();
          // All done
          break;
        } else {
          // A regular url character.
          ident.Append(PRUnichar(ch));
        }
      }

      // If the result of the above scanning is ok then change the token
      // type to a useful one.
      if (ok) {
        aToken.mType = eCSSToken_URL;
      }
    }
  }
  return PR_TRUE;
}


void
nsCSSScanner::ParseAndAppendEscape(nsresult& aErrorCode, nsString& aOutput)
{
  PRUint8* lexTable = gLexTable;
  PRInt32 ch = Peek(aErrorCode);
  if (ch < 0) {
    aOutput.Append(CSS_ESCAPE);
    return;
  }
  if ((ch <= 255) && ((lexTable[ch] & IS_HEX_DIGIT) != 0)) {
    PRInt32 rv = 0;
    int i;
    for (i = 0; i < 6; i++) { // up to six digits
      ch = Read(aErrorCode);
      if (ch < 0) {
        // Whoops: error or premature eof
        break;
      }
      if (ch >= 256 || (lexTable[ch] & (IS_HEX_DIGIT | IS_WHITESPACE)) == 0) {
        Unread();
        break;
      } else if ((lexTable[ch] & IS_HEX_DIGIT) != 0) {
        if ((lexTable[ch] & IS_DIGIT) != 0) {
          rv = rv * 16 + (ch - '0');
        } else {
          // Note: c&7 just keeps the low three bits which causes
          // upper and lower case alphabetics to both yield their
          // "relative to 10" value for computing the hex value.
          rv = rv * 16 + ((ch & 0x7) + 9);
        }
      } else {
        NS_ASSERTION((lexTable[ch] & IS_WHITESPACE) != 0, "bad control flow");
        // single space ends escape
        if (ch == '\r' && Peek(aErrorCode) == '\n') {
          // if CR/LF, eat LF too
          Read(aErrorCode);
        }
        break;
      }
    }
    if (6 == i) { // look for trailing whitespace and eat it
      ch = Peek(aErrorCode);
      if ((0 <= ch) && (ch <= 255) && 
          ((lexTable[ch] & IS_WHITESPACE) != 0)) {
        ch = Read(aErrorCode);
        // special case: if trailing whitespace is CR/LF, eat both chars (not part of spec, but should be)
        if (ch == '\r') {
          ch = Peek(aErrorCode);
          if (ch == '\n') {
            ch = Read(aErrorCode);
          }
        }
      }
    }
    NS_ASSERTION(rv >= 0, "How did rv become negative?");
    if (rv > 0) {
      AppendUCS4ToUTF16(ENSURE_VALID_CHAR(rv), aOutput);
    }
    return;
  } else {
    // "Any character except a hexidecimal digit can be escaped to
    // remove its special meaning by putting a backslash in front"
    // -- CSS1 spec section 7.1
    if (!EatNewline(aErrorCode)) { // skip escaped newline
      (void) Read(aErrorCode);
      if (ch > 0) {
        aOutput.Append(ch);
      }
    }
    return;
  }
}

/**
 * Gather up the characters in an identifier. The identfier was
 * started by "aChar" which will be appended to aIdent. The result
 * will be aIdent with all of the identifier characters appended
 * until the first non-identifier character is seen. The termination
 * character is unread for the future re-reading.
 */
PRBool nsCSSScanner::GatherIdent(nsresult& aErrorCode, PRInt32 aChar,
                                 nsString& aIdent)
{
  if (aChar == CSS_ESCAPE) {
    ParseAndAppendEscape(aErrorCode, aIdent);
  }
  else if (0 < aChar) {
    aIdent.Append(aChar);
  }
  for (;;) {
    aChar = Read(aErrorCode);
    if (aChar < 0) break;
    if (aChar == CSS_ESCAPE) {
      ParseAndAppendEscape(aErrorCode, aIdent);
    } else if ((aChar > 255) || ((gLexTable[aChar] & IS_IDENT) != 0)) {
      aIdent.Append(PRUnichar(aChar));
    } else {
      Unread();
      break;
    }
  }
  return PR_TRUE;
}

PRBool nsCSSScanner::ParseRef(nsresult& aErrorCode,
                              PRInt32 aChar,
                              nsCSSToken& aToken)
{
  aToken.mIdent.SetLength(0);
  aToken.mType = eCSSToken_Ref;
  PRInt32 ch = Read(aErrorCode);
  if (ch < 0) {
    return PR_FALSE;
  }
  if (ch > 255 || (gLexTable[ch] & IS_IDENT) || ch == CSS_ESCAPE) {
    // First char after the '#' is a valid ident char (or an escape),
    // so it makes sense to keep going
    if (StartsIdent(ch, Peek(aErrorCode), gLexTable)) {
      aToken.mType = eCSSToken_ID;
    }
    return GatherIdent(aErrorCode, ch, aToken.mIdent);
  }

  // No ident chars after the '#'.  Just unread |ch| and get out of here.
  Unread();
  return PR_TRUE;
}

PRBool nsCSSScanner::ParseIdent(nsresult& aErrorCode,
                                PRInt32 aChar,
                                nsCSSToken& aToken)
{
  nsString& ident = aToken.mIdent;
  ident.SetLength(0);
  if (!GatherIdent(aErrorCode, aChar, ident)) {
    return PR_FALSE;
  }

  nsCSSTokenType tokenType = eCSSToken_Ident;
  // look for functions (ie: "ident(")
  if (PRUnichar('(') == PRUnichar(Peek(aErrorCode))) { // this is a function definition
    tokenType = eCSSToken_Function;
  }

  aToken.mType = tokenType;
  return PR_TRUE;
}

PRBool nsCSSScanner::ParseAtKeyword(nsresult& aErrorCode, PRInt32 aChar,
                                    nsCSSToken& aToken)
{
  aToken.mIdent.SetLength(0);
  aToken.mType = eCSSToken_AtKeyword;
  return GatherIdent(aErrorCode, 0, aToken.mIdent);
}

PRBool nsCSSScanner::ParseNumber(nsresult& aErrorCode, PRInt32 c,
                                 nsCSSToken& aToken)
{
  nsString& ident = aToken.mIdent;
  ident.SetLength(0);
  PRBool gotDot = (c == '.') ? PR_TRUE : PR_FALSE;
  if (c != '+') {
    ident.Append(PRUnichar(c));
  }

  // Gather up characters that make up the number
  PRUint8* lexTable = gLexTable;
  for (;;) {
    c = Read(aErrorCode);
    if (c < 0) break;
    if (!gotDot && (c == '.') &&
        CheckLexTable(Peek(aErrorCode), IS_DIGIT, lexTable)) {
      gotDot = PR_TRUE;
    } else if ((c > 255) || ((lexTable[c] & IS_DIGIT) == 0)) {
      break;
    }
    ident.Append(PRUnichar(c));
  }

  // Convert number to floating point
  nsCSSTokenType type = eCSSToken_Number;
  PRInt32 ec;
  float value = ident.ToFloat(&ec);

  // Look at character that terminated the number
  aToken.mIntegerValid = PR_FALSE;
  if (c >= 0) {
    if ((c <= 255) && ((lexTable[c] & START_IDENT) != 0)) {
      ident.SetLength(0);
      if (!GatherIdent(aErrorCode, c, ident)) {
        return PR_FALSE;
      }
      type = eCSSToken_Dimension;
    } else if ('%' == c) {
      type = eCSSToken_Percentage;
      value = value / 100.0f;
      ident.SetLength(0);
    } else {
      // Put back character that stopped numeric scan
      Unread();
      if (!gotDot) {
        aToken.mInteger = ident.ToInteger(&ec);
        aToken.mIntegerValid = PR_TRUE;
      }
      ident.SetLength(0);
    }
  }
  else {  // stream ended
    if (!gotDot) {
      aToken.mInteger = ident.ToInteger(&ec);
      aToken.mIntegerValid = PR_TRUE;
    }
    ident.SetLength(0);
  }
  aToken.mNumber = value;
  aToken.mType = type;
  return PR_TRUE;
}

PRBool nsCSSScanner::SkipCComment(nsresult& aErrorCode)
{
  for (;;) {
    PRInt32 ch = Read(aErrorCode);
    if (ch < 0) break;
    if (ch == '*') {
      if (LookAhead(aErrorCode, '/')) {
        return PR_TRUE;
      }
    }
  }

  REPORT_UNEXPECTED_EOF(PECommentEOF);
  return PR_FALSE;
}

#if 0
PRBool nsCSSScanner::ParseCComment(nsresult& aErrorCode, nsCSSToken& aToken)
{
  nsString& ident = aToken.mIdent;
  for (;;) {
    PRInt32 ch = Read(aErrorCode);
    if (ch < 0) break;
    if (ch == '*') {
      if (LookAhead(aErrorCode, '/')) {
        ident.Append(PRUnichar(ch));
        ident.Append(PRUnichar('/'));
        break;
      }
    }
#ifdef COLLECT_WHITESPACE
    ident.Append(PRUnichar(ch));
#endif
  }
  aToken.mType = eCSSToken_WhiteSpace;
  return PR_TRUE;
}
#endif

#if 0
PRBool nsCSSScanner::ParseEOLComment(nsresult& aErrorCode, nsCSSToken& aToken)
{
  nsString& ident = aToken.mIdent;
  ident.SetLength(0);
  for (;;) {
    if (EatNewline(aErrorCode)) {
      break;
    }
    PRInt32 ch = Read(aErrorCode);
    if (ch < 0) {
      break;
    }
#ifdef COLLECT_WHITESPACE
    ident.Append(PRUnichar(ch));
#endif
  }
  aToken.mType = eCSSToken_WhiteSpace;
  return PR_TRUE;
}
#endif // 0

PRBool nsCSSScanner::ParseString(nsresult& aErrorCode, PRInt32 aStop,
                                 nsCSSToken& aToken)
{
  aToken.mIdent.SetLength(0);
  aToken.mType = eCSSToken_String;
  aToken.mSymbol = PRUnichar(aStop); // remember how it's quoted
  for (;;) {
    if (EatNewline(aErrorCode)) {
      aToken.mType = eCSSToken_Error;
#ifdef CSS_REPORT_PARSE_ERRORS
      ReportUnexpectedToken(aToken, "SEUnterminatedString");
#endif
      return PR_TRUE;
    }
    PRInt32 ch = Read(aErrorCode);
    if (ch < 0) {
      return PR_FALSE;
    }
    if (ch == aStop) {
      break;
    }
    if (ch == CSS_ESCAPE) {
      ParseAndAppendEscape(aErrorCode, aToken.mIdent);
    }
    else if (0 < ch) {
      aToken.mIdent.Append(ch);
    }
  }
  return PR_TRUE;
}
