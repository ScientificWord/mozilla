/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * ***** BEGIN LICENSE BLOCK *****
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
 * IBM Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2000
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Maha Abou El Rous <mahar@eg.ibm.com>
 *   Lina Kemmel <lkemmel@il.ibm.com>
 *   Simon Montagu <smontagu@netscape.com>
 *   Roozbeh Pournader <roozbeh@sharif.edu>
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
#include "nsBidiUtils.h"
#include "symmtable.h"
#include "bidicattable.h"

#define FE_TO_06_OFFSET 0xfe70

static const PRUnichar FE_TO_06 [][2] = {
    {0x064b,0x0000},{0x064b,0x0640},{0x064c,0x0000},
    {0x0000,0x0000},{0x064d,0x0000},{0x0000,0x0000},
    {0x064e,0x0000},{0x064e,0x0640},{0x064f,0x0000},
    {0x064f,0x0640},{0x0650,0x0000},{0x0650,0x0640},
    {0x0651,0x0000},{0x0651,0x0640},{0x0652,0x0000},
    {0x0652,0x0640},{0x0621,0x0000},{0x0622,0x0000},
    {0x0622,0x0000},{0x0623,0x0000},{0x0623,0x0000},
    {0x0624,0x0000},{0x0624,0x0000},{0x0625,0x0000},
    {0x0625,0x0000},{0x0626,0x0000},{0x0626,0x0000},
    {0x0626,0x0000},{0x0626,0x0000},{0x0627,0x0000},
    {0x0627,0x0000},{0x0628,0x0000},{0x0628,0x0000},
    {0x0628,0x0000},{0x0628,0x0000},{0x0629,0x0000},
    {0x0629,0x0000},{0x062a,0x0000},{0x062a,0x0000},
    {0x062a,0x0000},{0x062a,0x0000},{0x062b,0x0000},
    {0x062b,0x0000},{0x062b,0x0000},{0x062b,0x0000},
    {0x062c,0x0000},{0x062c,0x0000},{0x062c,0x0000},
    {0x062c,0x0000},{0x062d,0x0000},{0x062d,0x0000},
    {0x062d,0x0000},{0x062d,0x0000},{0x062e,0x0000},
    {0x062e,0x0000},{0x062e,0x0000},{0x062e,0x0000},
    {0x062f,0x0000},{0x062f,0x0000},{0x0630,0x0000},
    {0x0630,0x0000},{0x0631,0x0000},{0x0631,0x0000},
    {0x0632,0x0000},{0x0632,0x0000},{0x0633,0x0000},
    {0x0633,0x0000},{0x0633,0x0000},{0x0633,0x0000},
    {0x0634,0x0000},{0x0634,0x0000},{0x0634,0x0000},
    {0x0634,0x0000},{0x0635,0x0000},{0x0635,0x0000},
    {0x0635,0x0000},{0x0635,0x0000},{0x0636,0x0000},
    {0x0636,0x0000},{0x0636,0x0000},{0x0636,0x0000},
    {0x0637,0x0000},{0x0637,0x0000},{0x0637,0x0000},
    {0x0637,0x0000},{0x0638,0x0000},{0x0638,0x0000},
    {0x0638,0x0000},{0x0638,0x0000},{0x0639,0x0000},
    {0x0639,0x0000},{0x0639,0x0000},{0x0639,0x0000},
    {0x063a,0x0000},{0x063a,0x0000},{0x063a,0x0000},
    {0x063a,0x0000},{0x0641,0x0000},{0x0641,0x0000},
    {0x0641,0x0000},{0x0641,0x0000},{0x0642,0x0000},
    {0x0642,0x0000},{0x0642,0x0000},{0x0642,0x0000},
    {0x0643,0x0000},{0x0643,0x0000},{0x0643,0x0000},
    {0x0643,0x0000},{0x0644,0x0000},{0x0644,0x0000},
    {0x0644,0x0000},{0x0644,0x0000},{0x0645,0x0000},
    {0x0645,0x0000},{0x0645,0x0000},{0x0645,0x0000},
    {0x0646,0x0000},{0x0646,0x0000},{0x0646,0x0000},
    {0x0646,0x0000},{0x0647,0x0000},{0x0647,0x0000},
    {0x0647,0x0000},{0x0647,0x0000},{0x0648,0x0000},
    {0x0648,0x0000},{0x0649,0x0000},{0x0649,0x0000},
    {0x064a,0x0000},{0x064a,0x0000},{0x064a,0x0000},
    {0x064a,0x0000},{0x0644,0x0622},{0x0644,0x0622},
    {0x0644,0x0623},{0x0644,0x0623},{0x0644,0x0625},
    {0x0644,0x0625},{0x0644,0x0627},{0x0644,0x0627}
};

static const PRUnichar FB_TO_06 [] = {
    0x0671,0x0671,0x067B,0x067B,0x067B,0x067B,0x067E,0x067E, //FB50-FB57
    0x067E,0x067E,0x0680,0x0680,0x0680,0x0680,0x067A,0x067A, //FB58-FB5F
    0x067A,0x067A,0x067F,0x067F,0x067F,0x067F,0x0679,0x0679, //FB60-FB67
    0x0679,0x0679,0x06A4,0x06A4,0x06A4,0x06A4,0x06A6,0x06A6, //FB68-FB6F
    0x06A6,0x06A6,0x0684,0x0684,0x0684,0x0684,0x0683,0x0683, //FB70-FB77
    0x0683,0x0683,0x0686,0x0686,0x0686,0x0686,0x0687,0x0687, //FB78-FB7F
    0x0687,0x0687,0x068D,0x068D,0x068C,0x068C,0x068E,0x068E, //FB80-FB87
    0x0688,0x0688,0x0698,0x0698,0x0691,0x0691,0x06A9,0x06A9, //FB88-FB8F
    0x06A9,0x06A9,0x06AF,0x06AF,0x06AF,0x06AF,0x06B3,0x06B3, //FB90-FB97
    0x06B3,0x06B3,0x06B1,0x06B1,0x06B1,0x06B1,0x06BA,0x06BA, //FB98-FB9F
    0x06BB,0x06BB,0x06BB,0x06BB,0x06C0,0x06C0,0x06C1,0x06C1, //FBA0-FBA7
    0x06C1,0x06C1,0x06BE,0x06BE,0x06BE,0x06BE,0x06D2,0x06D2, //FBA8-FBAF
    0x06D3,0x06D3,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, //FBB0-FBB7
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, //FBB8-FBBF
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, //FBC0-FBC7
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, //FBC8-FBCF
    0x0000,0x0000,0x0000,0x06AD,0x06AD,0x06AD,0x06AD,0x06C7, //FBD0-FBD7
    0x06C7,0x06C6,0x06C6,0x06C8,0x06C8,0x0677,0x06CB,0x06CB, //FBD8-FBDF
    0x06C5,0x06C5,0x06C9,0x06C9,0x06D0,0x06D0,0x06D0,0x06D0, //FBE0-FBE7
    0x0649,0x0649,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, //FBE8-FBEF
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, //FBF0-FBF7
    0x0000,0x0000,0x0000,0x0000,0x06CC,0x06CC,0x06CC,0x06CC  //FBF8-FBFF
};

#define PresentationToOriginal(c, order)                  \
    (((0xFE70 <= (c) && (c) <= 0xFEFC)) ?                 \
         FE_TO_06[(c)- FE_TO_06_OFFSET][order] :                    \
     (((0xFB50 <= (c) && (c) <= 0xFBFF) && (order) == 0) ? \
         FB_TO_06[(c)-0xFB50] : (PRUnichar) 0x0000))

//============ Begin Arabic Basic to Presentation Form B Code ============
// Note: the following code are moved from gfx/src/windows/nsRenderingContextWin.cpp
static const PRUint8 gArabicMap1[] = {
            0x81, 0x83, 0x85, 0x87, 0x89, 0x8D, // 0622-0627
0x8F, 0x93, 0x95, 0x99, 0x9D, 0xA1, 0xA5, 0xA9, // 0628-062F
0xAB, 0xAD, 0xAF, 0xB1, 0xB5, 0xB9, 0xBD, 0xC1, // 0630-0637
0xC5, 0xC9, 0xCD                                // 0638-063A
};

static const PRUint8 gArabicMap2[] = {
      0xD1, 0xD5, 0xD9, 0xDD, 0xE1, 0xE5, 0xE9, // 0641-0647
0xED, 0xEF, 0xF1                                // 0648-064A
};

static const PRUint8 gArabicMapEx[] = {
      0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0671-0677
0x00, 0x66, 0x5E, 0x52, 0x00, 0x00, 0x56, 0x62, // 0678-067F
0x5A, 0x00, 0x00, 0x76, 0x72, 0x00, 0x7A, 0x7E, // 0680-0687
0x88, 0x00, 0x00, 0x00, 0x84, 0x82, 0x86, 0x00, // 0688-068F
0x00, 0x8C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0690-0697
0x8A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0698-069F
0x00, 0x00, 0x00, 0x00, 0x6A, 0x00, 0x6E, 0x00, // 06A0-06A7
0x00, 0x8E, 0x00, 0x00, 0x00, 0xD3, 0x00, 0x92, // 06A8-06AF
0x00, 0x9A, 0x00, 0x96, 0x00, 0x00, 0x00, 0x00, // 06B0-06B7
0x00, 0x00, 0x9E, 0xA0, 0x00, 0x00, 0xAA, 0x00, // 06B8-06BF
0xA4, 0xA6, 0x00, 0x00, 0x00, 0xE0, 0xD9, 0xD7, // 06C0-06C7
0xDB, 0xE2, 0x00, 0xDE, 0xFC, 0x00, 0x00, 0x00, // 06C8-06CF
0xE4, 0x00, 0xAE, 0xB0                          // 06D0-06D3
};

#define PresentationFormB(c, form)                                       \
    (((0x0622<=(c)) && ((c)<=0x063A)) ?                                  \
      (0xFE00|(gArabicMap1[(c)-0x0622] + (form))) :                      \
       (((0x0641<=(c)) && ((c)<=0x064A)) ?                               \
        (0xFE00|(gArabicMap2[(c)-0x0641] + (form))) :                    \
         (((0x0671<=(c)) && ((c))<=0x06D3) && gArabicMapEx[(c)-0x0671]) ? \
          (0xFB00|(gArabicMapEx[(c)-0x0671] + (form))) : (c)))

typedef enum {
   eIsolated,  // or Char N
   eFinal,     // or Char R
   eInitial,   // or Char L
   eMedial     // or Char M
} eArabicForm;

typedef enum {
   eTr = 0, // Transparent
   eRJ = 1, // Right-Joining
   eLJ = 2, // Left-Joining
   eDJ = 3, // Dual-Joining
   eNJ = 4, // Non-Joining
   eJC = 7, // Joining Causing
   eRightJCMask = 2, // bit of Right-Join Causing 
   eLeftJCMask = 1   // bit of Left-Join Causing 
} eArabicJoiningClass;

#define RightJCClass(j) (eRightJCMask&(j))
#define LeftJCClass(j)  (eLeftJCMask&(j))

#define DecideForm(jl,j,jr)                                 \
  (((eRJ == (j)) && RightJCClass(jr)) ? eFinal              \
                                      :                     \
   ((eDJ == (j)) ?                                          \
    ((RightJCClass(jr)) ?                                   \
     (((LeftJCClass(jl)) ? eMedial                          \
                         : eFinal))                         \
                        :                                   \
     (((LeftJCClass(jl)) ? eInitial                         \
                         : eIsolated))                      \
    )                     : eIsolated))                     \

// All letters without an equivalen in the FB50 block are 'eNJ' here. This
// should be fixed after finding some better mechanism for handling Arabic.
static const PRInt8 gJoiningClass[] = {
eNJ, eNJ, eNJ, eNJ, eNJ, eNJ, eNJ, eNJ, // 0600-0607
eNJ, eNJ, eNJ, eNJ, eNJ, eNJ, eNJ, eNJ, // 0608-060F
eTr, eTr, eTr, eTr, eTr, eTr, eNJ, eNJ, // 0610-0617
eNJ, eNJ, eNJ, eNJ, eNJ, eNJ, eNJ, eNJ, // 0618-061F
eNJ, eNJ, eRJ, eRJ, eRJ, eRJ, eDJ, eRJ, // 0620-0627
eDJ, eRJ, eDJ, eDJ, eDJ, eDJ, eDJ, eRJ, // 0628-062F
eRJ, eRJ, eRJ, eDJ, eDJ, eDJ, eDJ, eDJ, // 0630-0637
eDJ, eDJ, eDJ, eNJ, eNJ, eNJ, eNJ, eNJ, // 0638-063F
eJC, eDJ, eDJ, eDJ, eDJ, eDJ, eDJ, eDJ, // 0640-0647
eRJ, eDJ, eDJ, eTr, eTr, eTr, eTr, eTr, // 0648-064F
eTr, eTr, eTr, eTr, eTr, eTr, eTr, eTr, // 0650-0657
eTr, eNJ, eNJ, eNJ, eNJ, eNJ, eNJ, eNJ, // 0658-065F
eNJ, eNJ, eNJ, eNJ, eNJ, eNJ, eNJ, eNJ, // 0660-0667
eNJ, eNJ, eNJ, eNJ, eNJ, eNJ, eNJ, eNJ, // 0668-066F
eTr, eRJ, eNJ, eNJ, eNJ, eNJ, eNJ, eNJ, // 0670-0677
eNJ, eDJ, eDJ, eDJ, eNJ, eNJ, eDJ, eDJ, // 0678-067F
eDJ, eNJ, eNJ, eDJ, eDJ, eNJ, eDJ, eDJ, // 0680-0687
eRJ, eNJ, eNJ, eNJ, eRJ, eRJ, eRJ, eNJ, // 0688-068F
eNJ, eRJ, eNJ, eNJ, eNJ, eNJ, eNJ, eNJ, // 0690-0697
eRJ, eNJ, eNJ, eNJ, eNJ, eNJ, eNJ, eNJ, // 0698-069F
eNJ, eNJ, eNJ, eNJ, eDJ, eNJ, eDJ, eNJ, // 06A0-06A7
eNJ, eDJ, eNJ, eNJ, eNJ, eDJ, eNJ, eDJ, // 06A8-06AF
eNJ, eDJ, eNJ, eDJ, eNJ, eNJ, eNJ, eNJ, // 06B0-06B7
eNJ, eNJ, eDJ, eDJ, eNJ, eNJ, eDJ, eNJ, // 06B8-06BF
eRJ, eDJ, eNJ, eNJ, eNJ, eRJ, eRJ, eRJ, // 06C0-06C7
eRJ, eRJ, eNJ, eRJ, eDJ, eNJ, eNJ, eNJ, // 06C8-06CF
eDJ, eNJ, eRJ, eRJ, eNJ, eNJ, eTr, eTr, // 06D0-06D7
eTr, eTr, eTr, eTr, eTr, eNJ, eNJ, eTr, // 06D8-06DF
eTr, eTr, eTr, eTr, eTr, eNJ, eNJ, eTr, // 06E0-06E7
eTr, eNJ, eTr, eTr, eTr, eTr, eNJ, eNJ, // 06E8-06EF
eNJ, eNJ, eNJ, eNJ, eNJ, eNJ, eNJ, eNJ, // 06F0-06F7
eNJ, eNJ, eNJ, eNJ, eNJ, eNJ, eNJ, eNJ  // 06F8-06FF
};

#define GetJoiningClass(c)                   \
  ((IS_ARABIC_CHAR(c)) ?                     \
      (gJoiningClass[(c) - 0x0600]) :        \
      ((0x200D == (c)) ? eJC : eTr))

static const PRUint16 gArabicLigatureMap[] = 
{
0x82DF, // 0xFE82 0xFEDF -> 0xFEF5
0x82E0, // 0xFE82 0xFEE0 -> 0xFEF6
0x84DF, // 0xFE84 0xFEDF -> 0xFEF7
0x84E0, // 0xFE84 0xFEE0 -> 0xFEF8
0x88DF, // 0xFE88 0xFEDF -> 0xFEF9
0x88E0, // 0xFE88 0xFEE0 -> 0xFEFA
0x8EDF, // 0xFE8E 0xFEDF -> 0xFEFB
0x8EE0  // 0xFE8E 0xFEE0 -> 0xFEFC
};

static nsCharType ebc2ucd[15] = {
  eCharType_OtherNeutral, /* Placeholder -- there will never be a 0 index value */
  eCharType_LeftToRight,
  eCharType_RightToLeft,
  eCharType_RightToLeftArabic,
  eCharType_ArabicNumber,
  eCharType_EuropeanNumber,
  eCharType_EuropeanNumberSeparator,
  eCharType_EuropeanNumberTerminator,
  eCharType_CommonNumberSeparator,
  eCharType_OtherNeutral,
  eCharType_DirNonSpacingMark,
  eCharType_BoundaryNeutral,
  eCharType_BlockSeparator,
  eCharType_SegmentSeparator,
  eCharType_WhiteSpaceNeutral
};

static nsCharType cc2ucd[5] = {
  eCharType_LeftToRightEmbedding,
  eCharType_RightToLeftEmbedding,
  eCharType_PopDirectionalFormat,
  eCharType_LeftToRightOverride,
  eCharType_RightToLeftOverride
};

#define ARABIC_TO_HINDI_DIGIT_INCREMENT (START_HINDI_DIGITS - START_ARABIC_DIGITS)
#define NUM_TO_ARABIC(c) \
  ((((c)>=START_HINDI_DIGITS) && ((c)<=END_HINDI_DIGITS)) ? \
   ((c) - (PRUint16)ARABIC_TO_HINDI_DIGIT_INCREMENT) : \
   (c))
#define NUM_TO_HINDI(c) \
  ((((c)>=START_ARABIC_DIGITS) && ((c)<=END_ARABIC_DIGITS)) ? \
   ((c) + (PRUint16)ARABIC_TO_HINDI_DIGIT_INCREMENT): \
   (c))

// helper function to reverse a PRUnichar buffer
static void ReverseString(PRUnichar* aBuffer, PRUint32 aLen)
{
  PRUnichar *start, *end;
  PRUnichar swapChar;

  for (start = aBuffer, end = aBuffer + aLen - 1; start < end; ++start, --end) {
    swapChar = *start;
    *start = *end;
    *end = swapChar;
  }
}

nsresult ArabicShaping(const PRUnichar* aString, PRUint32 aLen,
                       PRUnichar* aBuf, PRUint32 *aBufLen, 
                       PRBool aInputLogical, PRBool aOutputLogical)
{
  nsAutoString tempString(aString, aLen);
  if (tempString.Length() != aLen)
    return NS_ERROR_OUT_OF_MEMORY;
  PRUnichar *tempBuf = tempString.BeginWriting();
  if (aInputLogical) {
    ReverseString(tempBuf, aLen);
  }
  const PRUnichar* src = tempBuf;
  const PRUnichar* p;
  PRUnichar* dest = aBuf;
  PRUnichar formB;
  PRInt8 leftJ, thisJ, rightJ;
  PRInt8 leftNoTrJ, rightNoTrJ;
  thisJ = leftNoTrJ = eNJ;
  rightJ = GetJoiningClass(*(src));
  while(src<tempBuf+aLen-1) {
    leftJ = thisJ;

    if ((eTr != leftJ) || ((leftJ == eTr) && 
        ( ( (src-1) >= tempBuf) && !IS_ARABIC_CHAR(*(src-1)))))
      leftNoTrJ = thisJ;

    if(src-2 >= (tempBuf)){
      for(p=src-2; (p >= (tempBuf))&& (eTr == leftNoTrJ) && (IS_ARABIC_CHAR(*(p+1))) ; p--)  
        leftNoTrJ = GetJoiningClass(*(p)) ;
    }

    thisJ = rightJ;
    rightJ = rightNoTrJ = GetJoiningClass(*(src+1)) ;

    if(src+2 <= (tempBuf+aLen-1)){
      for(p=src+2; (p <= (tempBuf+aLen-1))&&(eTr == rightNoTrJ) && (IS_ARABIC_CHAR(*(src+1))); p++)
        rightNoTrJ = GetJoiningClass(*(p)) ;
    }

    formB = PresentationFormB(*src, DecideForm(leftNoTrJ, thisJ, rightNoTrJ));
    *dest++ = formB;
    src++;

  }
  if((eTr != thisJ) || 
     ((thisJ == eTr) && (((src-1)>=tempBuf) && (!IS_ARABIC_CHAR(*(src-1))))))
    leftNoTrJ = thisJ;

  if(src-2 >= (tempBuf)){
    for(p=src-2; (src-2 >= (tempBuf)) && (eTr == leftNoTrJ) && (IS_ARABIC_CHAR(*(p+1))); p--)
      leftNoTrJ = GetJoiningClass(*(p)) ;
  }

  formB = PresentationFormB(*src, DecideForm(leftNoTrJ, rightJ, eNJ));
  *dest++ = formB;
  src++;

  PRUnichar *lSrc = aBuf;
  PRUnichar *lDest = aBuf;
  while(lSrc < (dest-1)) {
    PRUnichar next = *(lSrc+1);
    if(((0xFEDF == next) || (0xFEE0 == next)) && 
       (0xFE80 == (0xFFF1 & *lSrc))) {
      PRBool done = PR_FALSE;
      PRUint16 key = ((*lSrc) << 8) | ( 0x00FF & next);
      PRUint16 i;
      for(i=0;i<8;i++) {
        if(key == gArabicLigatureMap[i]) {
          done = PR_TRUE;
          // lam and alef in the source are mapped to a lam-alef ligature in the
          // destination, so lSrc is incremented by 2 here
          *lDest++ = 0xFEF5 + i;
          lSrc+=2;
          break;
        }
      }
      if(! done)
        *lDest++ = *lSrc++; 
    } else if (0x200C == *lSrc || 0x200D == *lSrc)
      // Strip zero-width joining controls ZWJ and ZWNJ from the shaped text
      lSrc++;
    else 
      *lDest++ = *lSrc++; 

  }
  if(lSrc < dest)
    *lDest++ = *lSrc++; 

  *aBufLen = lDest - aBuf;
  NS_ASSERTION(*aBufLen <= aLen, "ArabicShaping() likely did a buffer overflow!");

  if (aOutputLogical) {
    ReverseString(aBuf, *aBufLen);
  }
  return NS_OK;
}

nsresult Conv_FE_06(const nsString& aSrc, nsString& aDst)
{
  PRUnichar *aSrcUnichars = (PRUnichar *)aSrc.get();
  PRUint32 i, size = aSrc.Length();
  aDst.Truncate();
  for (i=0;i<size;i++) { // i : Source
    if (aSrcUnichars[i] == 0x0000) 
      break; // no need to convert char after the NULL
    if (IS_FE_CHAR(aSrcUnichars[i])) {
      //ahmed for lamalf
      PRUnichar ch = (PresentationToOriginal(aSrcUnichars[i], 1));
      if(ch)
        aDst += ch;
      ch=(PresentationToOriginal(aSrcUnichars[i], 0));
      if(ch)
        aDst += ch;
      else //if it is 00, just output what we have in FExx
        aDst += aSrcUnichars[i];
    } else {
      aDst += aSrcUnichars[i]; // copy it even if it is not in FE range
    }
  }// for : loop the buffer
  return NS_OK;
}

nsresult Conv_FE_06_WithReverse(const nsString& aSrc, nsString& aDst)
{
  PRUnichar *aSrcUnichars = (PRUnichar *)aSrc.get();
  PRBool foundArabic = PR_FALSE;
  PRUint32 i, endArabic, beginArabic, size;
  beginArabic = 0;
  size = aSrc.Length();
  aDst.Truncate();
  for (endArabic=0;endArabic<size;endArabic++) {
    if (aSrcUnichars[endArabic] == 0x0000) 
      break; // no need to convert char after the NULL

    while( (IS_FE_CHAR(aSrcUnichars[endArabic]))||
           (IS_ARABIC_CHAR(aSrcUnichars[endArabic]))||
           (IS_ARABIC_DIGIT(aSrcUnichars[endArabic]))||
           (aSrcUnichars[endArabic]==0x0020)) 
    {
      if(! foundArabic ) {
        beginArabic=endArabic;
        foundArabic= PR_TRUE;
      }
      endArabic++;
    }
    if(foundArabic) {
      endArabic--;
      for (i=endArabic; i>=beginArabic; i--) {
        if(IS_FE_CHAR(aSrcUnichars[i])) {
          //ahmed for the bug of lamalf
          aDst += PresentationToOriginal(aSrcUnichars[i], 0);
          if (PresentationToOriginal(aSrcUnichars[i], 1)) {
            // Two characters, we have to resize the buffer :(
             aDst += PresentationToOriginal(aSrcUnichars[i], 1);
          } // if expands to 2 char
        } else {
          // do we need to check the following if ?
          if((IS_ARABIC_CHAR(aSrcUnichars[i]))||
             (IS_ARABIC_DIGIT(aSrcUnichars[i]))||
             (aSrcUnichars[i]==0x0020))
            aDst += aSrcUnichars[i];
        }     
      }
    } else {
      aDst += aSrcUnichars[endArabic]; 
    }
    foundArabic=PR_FALSE;
  }// for : loop the buffer
  return NS_OK;
}

nsresult Conv_06_FE_WithReverse(const nsString& aSrc,
                                nsString& aDst,
                                PRUint32 aDir)
{
  PRUnichar *aSrcUnichars = (PRUnichar *)aSrc.get();
  PRUint32 i, beginArabic, endArabic, size;
  beginArabic = 0;
  size = aSrc.Length();
  aDst.Truncate();
  PRBool foundArabic = PR_FALSE;
  for (endArabic=0;endArabic<size;endArabic++) {
    if (aSrcUnichars[endArabic] == 0x0000) 
      break; // no need to convert char after the NULL

    while( (IS_06_CHAR(aSrcUnichars[endArabic])) || 
           (IS_ARABIC_CHAR(aSrcUnichars[endArabic])) || 
           (aSrcUnichars[endArabic]==0x0020) || 
           (IS_ARABIC_DIGIT(aSrcUnichars[endArabic]))  ) 
    {
      if(! foundArabic) {
        beginArabic=endArabic;
        foundArabic=PR_TRUE;
      }
      endArabic++;
    }
    if(foundArabic) {
      endArabic--;
      PRUnichar buf[8192];
      PRUint32 len=8192;

      ArabicShaping(&aSrcUnichars[beginArabic], endArabic-beginArabic+1,
                    buf, &len, 
                    PR_TRUE, PR_FALSE);
      // to reverse the numerals
      PRUint32 endNumeral, beginNumeral = 0;
      for (endNumeral=0;endNumeral<=len-1;endNumeral++){
        PRBool foundNumeral = PR_FALSE;
        while((endNumeral < len) && (IS_ARABIC_DIGIT(buf[endNumeral]))  ) {
          if(!foundNumeral)
          {
            foundNumeral=PR_TRUE;
            beginNumeral=endNumeral;
          }
          endNumeral++;
        }
        if(foundNumeral){
          endNumeral--;
          PRUnichar numbuf[20];
          for(i=beginNumeral; i<=endNumeral; i++){
            numbuf[i-beginNumeral]=buf[endNumeral-i+beginNumeral];
          }
          for(i=0;i<=endNumeral-beginNumeral;i++){
            buf[i+beginNumeral]=numbuf[i];
          }
        }
      }
      if(aDir==1){//ltr
        for (i=0;i<=len-1;i++){
          aDst+= buf[i];
        } 
      }
      else if(aDir==2){//rtl
        for (i=0;i<=len-1;i++){
          aDst+= buf[len-1-i];
        } 
      }
    } else {
      aDst += aSrcUnichars[endArabic];
    }
    foundArabic=PR_FALSE;
  }// for : loop the buffer
  return NS_OK;
}

nsresult HandleNumbers(PRUnichar* aBuffer, PRUint32 aSize, PRUint32 aNumFlag)
{
  PRUint32 i;
  // IBMBIDI_NUMERAL_NOMINAL *
  // IBMBIDI_NUMERAL_REGULAR
  // IBMBIDI_NUMERAL_HINDICONTEXT
  // IBMBIDI_NUMERAL_ARABIC
  // IBMBIDI_NUMERAL_HINDI

  switch (aNumFlag) {
    case IBMBIDI_NUMERAL_HINDI:
      for (i=0;i<aSize;i++)
        aBuffer[i] = NUM_TO_HINDI(aBuffer[i]);
      break;
    case IBMBIDI_NUMERAL_ARABIC:
      for (i=0;i<aSize;i++)
        aBuffer[i] = NUM_TO_ARABIC(aBuffer[i]);
      break;
    case IBMBIDI_NUMERAL_REGULAR:
    case IBMBIDI_NUMERAL_HINDICONTEXT:
        // for clipboard handling
        //XXX do we really want to convert numerals when copying text?
      for (i=1;i<aSize;i++) {
        if (IS_ARABIC_CHAR(aBuffer[i-1])) 
          aBuffer[i] = NUM_TO_HINDI(aBuffer[i]);
        else 
          aBuffer[i] = NUM_TO_ARABIC(aBuffer[i]);
      }
    case IBMBIDI_NUMERAL_NOMINAL:
    default:
      break;
  }
  return NS_OK;
}

nsresult HandleNumbers(const nsString& aSrc, nsString& aDst)
{
  aDst = aSrc;
  return HandleNumbers((PRUnichar *)aDst.get(),aDst.Length(), IBMBIDI_NUMERAL_REGULAR);
}

PRUint32 SymmSwap(PRUint32 aChar)
{
  return Mirrored(aChar);
}

eBidiCategory GetBidiCategory(PRUint32 aChar)
{
  eBidiCategory oResult = GetBidiCat(aChar);
  if (eBidiCat_CC == oResult)
    oResult = (eBidiCategory)(aChar & 0xFF); /* Control codes have special treatment to keep the tables smaller */
  return oResult;
}

PRBool IsBidiCategory(PRUint32 aChar, eBidiCategory aBidiCategory)
{
  return (GetBidiCategory(aChar) == aBidiCategory);
}

#define LRM_CHAR 0x200e
PRBool IsBidiControl(PRUint32 aChar)
{
  // This method is used when stripping Bidi control characters for
  // display, so it will return TRUE for LRM and RLM as
  // well as the characters with category eBidiCat_CC
  return (eBidiCat_CC == GetBidiCat(aChar) || ((aChar)&0xfffffe)==LRM_CHAR);
}

nsCharType GetCharType(PRUint32 aChar)
{
  nsCharType oResult;
  eBidiCategory bCat = GetBidiCat(aChar);
  if (eBidiCat_CC != bCat) {
    NS_ASSERTION(bCat < (sizeof(ebc2ucd)/sizeof(nsCharType)), "size mismatch");
    if(bCat < (sizeof(ebc2ucd)/sizeof(nsCharType)))
      oResult = ebc2ucd[bCat];
    else 
      oResult = ebc2ucd[0]; // something is very wrong, but we need to return a value
  } else {
    NS_ASSERTION((aChar-0x202a) < (sizeof(cc2ucd)/sizeof(nsCharType)), "size mismatch");
    if((aChar-0x202a) < (sizeof(cc2ucd)/sizeof(nsCharType)))
      oResult = cc2ucd[aChar - 0x202a];
    else 
      oResult = ebc2ucd[0]; // something is very wrong, but we need to return a value
  }
  return oResult;
}
