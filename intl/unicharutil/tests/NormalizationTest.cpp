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
 *  Simon Montagu <smontagu@smontagu.org>
 * Portions created by the Initial Developer are Copyright (C) 2006
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

#include <stdio.h>
#include "nsISupports.h"
#include "nsXPCOM.h"
#include "nsIServiceManager.h"
#include "nsNetUtil.h"
#include "nsCOMPtr.h"
#include "nsIUnicodeNormalizer.h"
#include "nsStringAPI.h"
#include "nsCharTraits.h"

struct testcaseLine {
  wchar_t* c1;
  wchar_t* c2;
  wchar_t* c3;
  wchar_t* c4;
  wchar_t* c5;
  char* description;
};

#ifdef DEBUG_smontagu
#define DEBUG_NAMED_TESTCASE(t, s) \
  printf(t ": "); \
  for (PRUint32 i = 0; i < s.Length(); ++i) \
    printf("%x ", s.CharAt(i)); \
  printf("\n")
#else
#define DEBUG_NAMED_TESTCASE(t, s)
#endif

#define DEBUG_TESTCASE(x) DEBUG_NAMED_TESTCASE(#x, x)

#define NORMALIZE_AND_COMPARE(base, comparison, form, description) \
   normalized.Truncate();\
   normalizer->NormalizeUnicode##form(comparison, normalized);\
   DEBUG_NAMED_TESTCASE(#form "(" #comparison ")", normalized);\
   if (!base.Equals(normalized)) {\
     rv = PR_FALSE;\
     showError(description, #base " != " #form "(" #comparison ")\n");\
   }

NS_DEFINE_CID(kUnicodeNormalizerCID, NS_UNICODE_NORMALIZER_CID);

nsIUnicodeNormalizer *normalizer;
PRBool verboseMode = PR_FALSE;

#include "NormalizationData.h"

void showError(const char* description, const char* errorText)
{
  if (verboseMode)
    printf("%s failed: %s", description, errorText);
}

PRBool TestInvariants(testcaseLine* testLine)
{
  nsAutoString c1, c2, c3, c4, c5, normalized;
  c1 = nsDependentString((PRUnichar*)testLine->c1);
  c2 = nsDependentString((PRUnichar*)testLine->c2);
  c3 = nsDependentString((PRUnichar*)testLine->c3);
  c4 = nsDependentString((PRUnichar*)testLine->c4);
  c5 = nsDependentString((PRUnichar*)testLine->c5);
  PRBool rv = PR_TRUE;
 
  /*
    1. The following invariants must be true for all conformant implementations

    NFC
      c2 ==  NFC(c1) ==  NFC(c2) ==  NFC(c3)
  */
  DEBUG_TESTCASE(c2);
  NORMALIZE_AND_COMPARE(c2, c1, NFC, testLine->description);  
  NORMALIZE_AND_COMPARE(c2, c2, NFC, testLine->description);
  NORMALIZE_AND_COMPARE(c2, c3, NFC, testLine->description);

  /*
      c4 ==  NFC(c4) ==  NFC(c5)
  */
  DEBUG_TESTCASE(c4);
  NORMALIZE_AND_COMPARE(c4, c4, NFC, testLine->description);
  NORMALIZE_AND_COMPARE(c4, c5, NFC, testLine->description);

  /*
    NFD
      c3 ==  NFD(c1) ==  NFD(c2) ==  NFD(c3)
  */
  DEBUG_TESTCASE(c3);
  NORMALIZE_AND_COMPARE(c3, c1, NFD, testLine->description);
  NORMALIZE_AND_COMPARE(c3, c2, NFD, testLine->description);
  NORMALIZE_AND_COMPARE(c3, c3, NFD, testLine->description);
  /*
      c5 ==  NFD(c4) ==  NFD(c5)
  */
  DEBUG_TESTCASE(c5);
  NORMALIZE_AND_COMPARE(c5, c4, NFD, testLine->description);
  NORMALIZE_AND_COMPARE(c5, c5, NFD, testLine->description);

  /*
    NFKC
      c4 == NFKC(c1) == NFKC(c2) == NFKC(c3) == NFKC(c4) == NFKC(c5)
  */
  DEBUG_TESTCASE(c4);
  NORMALIZE_AND_COMPARE(c4, c1, NFKC, testLine->description);
  NORMALIZE_AND_COMPARE(c4, c2, NFKC, testLine->description);
  NORMALIZE_AND_COMPARE(c4, c3, NFKC, testLine->description);
  NORMALIZE_AND_COMPARE(c4, c4, NFKC, testLine->description);
  NORMALIZE_AND_COMPARE(c4, c5, NFKC, testLine->description);

  /*
    NFKD
      c5 == NFKD(c1) == NFKD(c2) == NFKD(c3) == NFKD(c4) == NFKD(c5)
  */
  DEBUG_TESTCASE(c5);
  NORMALIZE_AND_COMPARE(c5, c1, NFKD, testLine->description);
  NORMALIZE_AND_COMPARE(c5, c2, NFKD, testLine->description);
  NORMALIZE_AND_COMPARE(c5, c3, NFKD, testLine->description);
  NORMALIZE_AND_COMPARE(c5, c4, NFKD, testLine->description);
  NORMALIZE_AND_COMPARE(c5, c5, NFKD, testLine->description);

  return rv;
}

PRUint32 UTF32CodepointFromTestcase(testcaseLine* testLine)
{
  if (!IS_SURROGATE(testLine->c1[0]))
    return testLine->c1[0];

  NS_ASSERTION(NS_IS_HIGH_SURROGATE(testLine->c1[0]) &&
               NS_IS_LOW_SURROGATE(testLine->c1[1]),
               "Test data neither in BMP nor legal surrogate pair");
  return SURROGATE_TO_UCS4(testLine->c1[0], testLine->c1[1]);
}

PRBool TestUnspecifiedCodepoint(PRUint32 codepoint)
{
  PRBool rv = PR_TRUE;
  PRUnichar unicharArray[3];
  nsAutoString X, normalized;
  char description[9];

  if (IS_IN_BMP(codepoint)) {
    unicharArray[0] = codepoint;
    unicharArray[1] = 0;
    X = nsDependentString(unicharArray);
  }
  else {
    unicharArray[0] = H_SURROGATE(codepoint);
    unicharArray[1] = L_SURROGATE(codepoint);
    unicharArray[2] = 0;
    X = nsDependentString(unicharArray);
  }

  /*
 2. For every code point X assigned in this version of Unicode that is not specifically
    listed in Part 1, the following invariants must be true for all conformant
    implementations:

      X == NFC(X) == NFD(X) == NFKC(X) == NFKD(X)
  */
  DEBUG_TESTCASE(X);
  sprintf(description, "U+%04X", codepoint);
  NORMALIZE_AND_COMPARE(X, X, NFC, description);
  NORMALIZE_AND_COMPARE(X, X, NFD, description);
  NORMALIZE_AND_COMPARE(X, X, NFKC, description);
  NORMALIZE_AND_COMPARE(X, X, NFKD, description);
  return rv;
}

void TestPart0()
{
  printf("Test Part0: Specific cases\n");

  PRUint32 i = 0;
  PRUint32 numFailed = 0;
  PRUint32 numPassed = 0;

  while (Part0TestData[i].c1[0] != 0) {
    if (TestInvariants(&Part0TestData[i++]))
      ++numPassed;
    else
      ++numFailed;
  }
  printf(" %d cases passed, %d failed\n\n", numPassed, numFailed);
}

void TestPart1()
{
  printf("Test Part1: Character by character test\n");

  PRUint32 i = 0;
  PRUint32 numFailed = 0;
  PRUint32 numPassed = 0;
  PRUint32 codepoint;
  PRUint32 testDataCodepoint = UTF32CodepointFromTestcase(&Part1TestData[i]);

  for (codepoint = 1; codepoint < 0x110000; ++codepoint) {
    if (testDataCodepoint == codepoint) {
      if (TestInvariants(&Part1TestData[i]))
        ++numPassed;
      else
        ++numFailed;
      testDataCodepoint = UTF32CodepointFromTestcase(&Part1TestData[++i]);
    } else {
      if (TestUnspecifiedCodepoint(codepoint))
        ++numPassed;
      else
        ++numFailed;
    }
  }
  printf(" %d cases passed, %d failed\n\n", numPassed, numFailed);
}

void TestPart2()
{
  printf("Test Part2: Canonical Order Test\n");

  PRUint32 i = 0;
  PRUint32 numFailed = 0;
  PRUint32 numPassed = 0;

  while (Part2TestData[i].c1[0] != 0) {
    if (TestInvariants(&Part2TestData[i++]))
      ++numPassed;
    else
      ++numFailed;
  }
  printf(" %d cases passed, %d failed\n\n", numPassed, numFailed);
}

void TestPart3()
{
  printf("Test Part3: PRI #29 Test\n");

  PRUint32 i = 0;
  PRUint32 numFailed = 0;
  PRUint32 numPassed = 0;

  while (Part3TestData[i].c1[0] != 0) {
    if (TestInvariants(&Part3TestData[i++]))
      ++numPassed;
    else
      ++numFailed;
  }
  printf(" %d cases passed, %d failed\n\n", numPassed, numFailed);
}

int main(int argc, char** argv) {
  if (sizeof(wchar_t) != 2) {
    printf("This test can only be run where sizeof(wchar_t) == 2\n");
    return 1;
  }
  if (strlen(versionText) == 0) {
    printf("No testcases: to run the tests generate the header file using\n");
    printf(" perl genNormalizationData.pl\n");
    printf("in intl/unichar/tools and rebuild\n");
    return 1;
  }

  printf("NormalizationTest: test nsIUnicodeNormalizer. UCD version: %s\n", 
         versionText); 
  if (argc <= 1)
    verboseMode = PR_FALSE;
  else if ((argc == 2) && (!strcmp(argv[1], "-v")))
    verboseMode = PR_TRUE;
  else {
    printf("                   Usage: NormalizationTest [OPTION]..\n");
    printf("Options:\n");
    printf("        -v   Verbose mode\n");
    return 1;
  }

  nsresult rv = NS_InitXPCOM2(nsnull, nsnull, nsnull);
  if (NS_FAILED(rv)) {
    printf("NS_InitXPCOM2 failed\n");
    return 1;
  }
  
  normalizer = NULL;
  nsresult res;
  res = CallGetService(kUnicodeNormalizerCID, &normalizer);
  
 if(NS_FAILED(res) || ( normalizer == NULL ) ) {
    printf("GetService failed\n");
    return 1;
  }

  TestPart0();
  TestPart1();
  TestPart2();
  TestPart3();
  
  NS_RELEASE(normalizer);

  printf("Test finished \n");
  return 0;
}
