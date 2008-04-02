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
#include "nsJapaneseToUnicode.h"

#include "nsUCSupport.h"

#include "nsIPrefBranch.h"
#include "nsIPrefService.h"

#include "japanese.map"

#include "nsICharsetConverterManager.h"
#include "nsIServiceManager.h"
static NS_DEFINE_CID(kCharsetConverterManagerCID, NS_ICHARSETCONVERTERMANAGER_CID);

#define SJIS_INDEX mMapIndex[0]
#define JIS0208_INDEX mMapIndex[1]
#define JIS0212_INDEX gJIS0212Index

void nsJapaneseToUnicode::setMapMode()
{
  nsresult res;

  mMapIndex = gIndex;

  nsCOMPtr<nsIPrefBranch> prefBranch = do_GetService(NS_PREFSERVICE_CONTRACTID);
  if (!prefBranch) return;
  nsXPIDLCString prefMap;
  res = prefBranch->GetCharPref("intl.jis0208.map", getter_Copies(prefMap));
  if (!NS_SUCCEEDED(res)) return;
  nsCaseInsensitiveCStringComparator comparator;
  if ( prefMap.Equals(NS_LITERAL_CSTRING("cp932"), comparator) ) {
    mMapIndex = gCP932Index;
  } else if ( prefMap.Equals(NS_LITERAL_CSTRING("ibm943"), comparator) ) {
    mMapIndex = gIBM943Index;
  }
}

NS_IMETHODIMP nsShiftJISToUnicode::Convert(
   const char * aSrc, PRInt32 * aSrcLen,
     PRUnichar * aDest, PRInt32 * aDestLen)
{
   static const PRUint8 sbIdx[256] =
   {
     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,  /* 0x00 */
     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,  /* 0x08 */
     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,  /* 0x10 */
     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,  /* 0x18 */
     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,  /* 0x20 */
     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,  /* 0x28 */
     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,  /* 0x30 */
     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,  /* 0x38 */
        0,    1,    2,    3,    4,    5,    6,    7,  /* 0x40 */
        8,    9,   10,   11,   12,   13,   14,   15,  /* 0x48 */
       16,   17,   18,   19,   20,   21,   22,   23,  /* 0x50 */
       24,   25,   26,   27,   28,   29,   30,   31,  /* 0x58 */
       32,   33,   34,   35,   36,   37,   38,   39,  /* 0x60 */
       40,   41,   42,   43,   44,   45,   46,   47,  /* 0x68 */
       48,   49,   50,   51,   52,   53,   54,   55,  /* 0x70 */
       56,   57,   58,   59,   60,   61,   62, 0xFF,  /* 0x78 */
       63,   64,   65,   66,   67,   68,   69,   70,  /* 0x80 */
       71,   72,   73,   74,   75,   76,   77,   78,  /* 0x88 */
       79,   80,   81,   82,   83,   84,   85,   86,  /* 0x90 */
       87,   88,   89,   90,   91,   92,   93,   94,  /* 0x98 */
       95,   96,   97,   98,   99,  100,  101,  102,  /* 0xa0 */
      103,  104,  105,  106,  107,  108,  109,  110,  /* 0xa8 */
      111,  112,  113,  114,  115,  116,  117,  118,  /* 0xb0 */
      119,  120,  121,  122,  123,  124,  125,  126,  /* 0xb8 */
      127,  128,  129,  130,  131,  132,  133,  134,  /* 0xc0 */
      135,  136,  137,  138,  139,  140,  141,  142,  /* 0xc8 */
      143,  144,  145,  146,  147,  148,  149,  150,  /* 0xd0 */
      151,  152,  153,  154,  155,  156,  157,  158,  /* 0xd8 */
      159,  160,  161,  162,  163,  164,  165,  166,  /* 0xe0 */
      167,  168,  169,  170,  171,  172,  173,  174,  /* 0xe8 */
      175,  176,  177,  178,  179,  180,  181,  182,  /* 0xf0 */
      183,  184,  185,  186,  187, 0xFF, 0xFF, 0xFF,  /* 0xf8 */
   };

   const unsigned char* srcEnd = (unsigned char*)aSrc + *aSrcLen;
   const unsigned char* src =(unsigned char*) aSrc;
   PRUnichar* destEnd = aDest + *aDestLen;
   PRUnichar* dest = aDest;
   while((src < srcEnd))
   {
       switch(mState)
       {

          case 0:
          if(*src & 0x80)
          {
            mData = SJIS_INDEX[*src & 0x7F];
            if(mData < 0xE000 )
            {
               mState = 1; // two bytes 
            } else {
               if( mData > 0xFF00)
               {
                 if(0xFFFD == mData) {
                   // IE-compatible handling of undefined codepoints:
                   // 0x80 --> U+0080
                   // 0xa0 --> U+F8F0
                   // 0xfd --> U+F8F1
                   // 0xfe --> U+F8F2
                   // 0xff --> U+F8F3
                   switch (*src) {
                     case 0x80:
                       *dest++ = (PRUnichar) *src;
                       break;

                     case 0xa0:
                       *dest++ = (PRUnichar) 0xf8f0;
                       break;

                     case 0xfd:
                     case 0xfe:
                     case 0xff:
                       *dest++ = (PRUnichar) 0xf8f1 + 
                                   (*src - (unsigned char)(0xfd));
                       break;

                     default:
                       *dest++ = 0x30FB;
                   }
                   if(dest >= destEnd)
                     goto error1;
                 } else {
                   *dest++ = mData; // JIS 0201
                   if(dest >= destEnd)
                     goto error1;
                 }
               } else {
                 mState = 2; // EUDC
               }
            }
          } else {
            // ASCII
            *dest++ = (PRUnichar) *src;
            if(dest >= destEnd)
              goto error1;
          }
          break;

          case 1: // Index to table
          {
            PRUint8 off = sbIdx[*src];
            if(0xFF == off) {
               *dest++ = 0x30FB;
            } else {
               PRUnichar ch = gJapaneseMap[mData+off];
               if(ch == 0xfffd) 
                 ch = 0x30fb;
               *dest++ = ch;
            }
            mState = 0;
            if(dest >= destEnd)
              goto error1;
          }
          break;

          case 2: // EUDC
          {
            PRUint8 off = sbIdx[*src];
            if(0xFF == off) {
               *dest++ = 0x30fb;
            } else {
               *dest++ = mData + off;
            }
            mState = 0;
            if(dest >= destEnd)
              goto error1;
          }
          break;

       }
       src++;
   }
   *aDestLen = dest - aDest;
   return NS_OK;
error1:
   *aDestLen = dest-aDest;
   src++;
   if ((mState == 0) && (src == srcEnd)) {
     return NS_OK;
   }
   *aSrcLen = src - (const unsigned char*)aSrc;
   return NS_OK_UDEC_MOREOUTPUT;
}




NS_IMETHODIMP nsEUCJPToUnicodeV2::Convert(
   const char * aSrc, PRInt32 * aSrcLen,
     PRUnichar * aDest, PRInt32 * aDestLen)
{
   static const PRUint8 sbIdx[256] =
   {
/* 0x0X */
     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
/* 0x1X */
     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
/* 0x2X */
     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
/* 0x3X */
     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
/* 0x4X */
     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
/* 0x5X */
     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
/* 0x6X */
     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
/* 0x7X */
     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
/* 0x8X */
     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
/* 0x9X */
     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
/* 0xAX */
     0xFF, 0,    1,    2,    3,    4,    5,    6,  
     7,    8 ,   9,    10,   11,   12,   13,   14,
/* 0xBX */
     15,   16,   17,   18,   19,   20,   21,   22, 
     23,   24,   25,   26,   27,   28,   29,   30, 
/* 0xCX */
     31,   32,   33,   34,   35,   36,   37,   38, 
     39,   40,   41,   42,   43,   44,   45,   46, 
/* 0xDX */
     47,   48,   49,   50,   51,   52,   53,   54, 
     55,   56,   57,   58,   59,   60,   61,   62, 
/* 0xEX */
     63,   64,   65,   66,   67,   68,   69,   70, 
     71,   72,   73,   74,   75,   76,   77,   78, 
/* 0xFX */
     79,   80,   81,   82,   83,   84,   85,   86, 
     87,   88,   89,   90,   91,   92,   93,   0xFF, 
   };

   const unsigned char* srcEnd = (unsigned char*)aSrc + *aSrcLen;
   const unsigned char* src =(unsigned char*) aSrc;
   PRUnichar* destEnd = aDest + *aDestLen;
   PRUnichar* dest = aDest;
   while((src < srcEnd))
   {
       switch(mState)
       {
          case 0:
          if(*src & 0x80  && *src != (unsigned char)0xa0)
          {
            mData = JIS0208_INDEX[*src & 0x7F];
            if(mData != 0xFFFD )
            {
               mState = 1; // two byte JIS0208
            } else {
               if( 0x8e == *src) {
                 // JIS 0201
                 mState = 2; // JIS0201
               } else if(0x8f == *src) {
                 // JIS 0212
                 mState = 3; // JIS0212
               } else {
                 // others 
                 *dest++ = 0xFFFD;
                 if(dest >= destEnd)
                   goto error1;
               }
            }
          } else {
            // ASCII
            *dest++ = (PRUnichar) *src;
            if(dest >= destEnd)
              goto error1;
          }
          break;

          case 1: // Index to table
          {
            PRUint8 off = sbIdx[*src];
            if(0xFF == off) {
              *dest++ = 0xFFFD;
               // if the first byte is valid for EUC-JP but the second 
               // is not while being a valid US-ASCII(i.e. < 0xc0), save it
               // instead of eating it up !
               if ( ! (*src & 0xc0)  )
                 *dest++ = (PRUnichar) *src;;
            } else {
               *dest++ = gJapaneseMap[mData+off];
            }
            mState = 0;
            if(dest >= destEnd)
              goto error1;
          }
          break;

          case 2: // JIS 0201
          {
            if((0xA1 <= *src) && (*src <= 0xDF)) {
              *dest++ = (0xFF61-0x00A1) + *src;
            } else {
              *dest++ = 0xFFFD;             
              // if 0x8e is not followed by a valid JIS X 0201 byte
              // but by a valid US-ASCII, save it instead of eating it up.
              if ( (PRUint8)*src < (PRUint8)0x7f )
                 *dest++ = (PRUnichar) *src;
            }
            mState = 0;
            if(dest >= destEnd)
              goto error1;
          }
          break;

          case 3: // JIS 0212
          {
            if(*src & 0x80)
            {
              mData = JIS0212_INDEX[*src & 0x7F];
              if(mData != 0xFFFD )
              {
                 mState = 4; 
              } else {
                 mState = 5; // error
              }
            } else {
              mState = 5; // error
            }
          }
          break;
          case 4:
          {
            PRUint8 off = sbIdx[*src];
            if(0xFF == off) {
               *dest++ = 0xFFFD;
            } else {
               *dest++ = gJapaneseMap[mData+off];
            }
            mState = 0;
            if(dest >= destEnd)
              goto error1;
          }
          break;
          case 5: // two bytes undefined
          {
            *dest++ = 0xFFFD;
            mState = 0;
            if(dest >= destEnd)
              goto error1;
          }
          break;
       }
       src++;
   }
   *aDestLen = dest - aDest;
   return NS_OK;
error1:
   *aDestLen = dest-aDest;
   src++;
   if ((mState == 0) && (src == srcEnd)) {
     return NS_OK;
   } 
   *aSrcLen = src - (const unsigned char*)aSrc;
   return NS_OK_UDEC_MOREOUTPUT;
}



NS_IMETHODIMP nsISO2022JPToUnicodeV2::Convert(
   const char * aSrc, PRInt32 * aSrcLen,
     PRUnichar * aDest, PRInt32 * aDestLen)
{
   static const PRUint16 fbIdx[128] =
   {
/* 0x8X */
     0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
     0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
/* 0x9X */
     0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
     0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
/* 0xAX */
     0xFFFD, 0,      94,     94* 2,  94* 3,  94* 4,  94* 5,  94* 6,  
     94* 7,  94* 8 , 94* 9,  94*10,  94*11,  94*12,  94*13,  94*14,
/* 0xBX */
     94*15,  94*16,  94*17,  94*18,  94*19,  94*20,  94*21,  94*22,
     94*23,  94*24,  94*25,  94*26,  94*27,  94*28,  94*29,  94*30,
/* 0xCX */
     94*31,  94*32,  94*33,  94*34,  94*35,  94*36,  94*37,  94*38,
     94*39,  94*40,  94*41,  94*42,  94*43,  94*44,  94*45,  94*46,
/* 0xDX */
     94*47,  94*48,  94*49,  94*50,  94*51,  94*52,  94*53,  94*54,
     94*55,  94*56,  94*57,  94*58,  94*59,  94*60,  94*61,  94*62,
/* 0xEX */
     94*63,  94*64,  94*65,  94*66,  94*67,  94*68,  94*69,  94*70,
     94*71,  94*72,  94*73,  94*74,  94*75,  94*76,  94*77,  94*78,
/* 0xFX */
     94*79,  94*80,  94*81,  94*82,  94*83,  94*84,  94*85,  94*86,
     94*87,  94*88,  94*89,  94*90,  94*91,  94*92,  94*93,  0xFFFD,
   };
   static const PRUint8 sbIdx[256] =
   {
/* 0x0X */
     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
/* 0x1X */
     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
/* 0x2X */
     0xFF, 0,    1,    2,    3,    4,    5,    6,  
     7,    8 ,   9,    10,   11,   12,   13,   14,
/* 0x3X */
     15,   16,   17,   18,   19,   20,   21,   22, 
     23,   24,   25,   26,   27,   28,   29,   30, 
/* 0x4X */
     31,   32,   33,   34,   35,   36,   37,   38, 
     39,   40,   41,   42,   43,   44,   45,   46, 
/* 0x5X */
     47,   48,   49,   50,   51,   52,   53,   54, 
     55,   56,   57,   58,   59,   60,   61,   62, 
/* 0x6X */
     63,   64,   65,   66,   67,   68,   69,   70, 
     71,   72,   73,   74,   75,   76,   77,   78, 
/* 0x7X */
     79,   80,   81,   82,   83,   84,   85,   86, 
     87,   88,   89,   90,   91,   92,   93,   0xFF, 
/* 0x8X */
     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
/* 0x9X */
     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
/* 0xAX */
     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
/* 0xBX */
     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
/* 0xCX */
     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
/* 0xDX */
     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
/* 0xEX */
     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
/* 0xFX */
     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
   };

   const unsigned char* srcEnd = (unsigned char*)aSrc + *aSrcLen;
   const unsigned char* src =(unsigned char*) aSrc;
   PRUnichar* destEnd = aDest + *aDestLen;
   PRUnichar* dest = aDest;
   while((src < srcEnd))
   {
     
       switch(mState)
       {
          case mState_ASCII:
            if(0x1b == *src)
            {
              mLastLegalState = mState;
              mState = mState_ESC;
            } else if(*src & 0x80) {
              goto error2;
            } else {
              *dest++ = (PRUnichar) *src;
              if(dest >= destEnd)
                goto error1;
            }
          break;
          
          case mState_ESC:
            if( '(' == *src) {
              mState = mState_ESC_28;
            } else if ('$' == *src)  {
              mState = mState_ESC_24;
            } else if ('.' == *src)  { // for ISO-2022-JP-2
              mState = mState_ESC_2e;
            } else if ('N' == *src)  { // for ISO-2022-JP-2
              mState = mState_ESC_4e;
            } else  {
              if((dest+2) >= destEnd)
                goto error1;
              *dest++ = (PRUnichar) 0x1b;
              if(0x80 & *src)
                goto error2;
              *dest++ = (PRUnichar) *src;
              mState = mLastLegalState;
            }
          break;

          case mState_ESC_28: // ESC (
            if( 'B' == *src) {
              mState = mState_ASCII;
              if (mRunLength == 0) {
                if((dest+1) >= destEnd)
                  goto error1;
                *dest++ = 0xFFFD;
              }
              mRunLength = 0;
            } else if ('J' == *src)  {
              mState = mState_JISX0201_1976Roman;
              if (mRunLength == 0 && mLastLegalState != mState_ASCII) {
                if((dest+1) >= destEnd)
                  goto error1;
                *dest++ = 0xFFFD;
              }
              mRunLength = 0;
            } else if ('I' == *src)  {
              mState = mState_JISX0201_1976Kana;
              mRunLength = 0;
            } else  {
              if((dest+3) >= destEnd)
                goto error1;
              *dest++ = (PRUnichar) 0x1b;
              *dest++ = (PRUnichar) '(';
              if(0x80 & *src)
                goto error2;
              *dest++ = (PRUnichar) *src;
              mState = mLastLegalState;
            }
          break;

          case mState_ESC_24: // ESC $
            if( '@' == *src) {
              mState = mState_JISX0208_1978;
              mRunLength = 0;
            } else if ('A' == *src)  {
              mState = mState_GB2312_1980;
              mRunLength = 0;
            } else if ('B' == *src)  {
              mState = mState_JISX0208_1983;
              mRunLength = 0;
            } else if ('(' == *src)  {
              mState = mState_ESC_24_28;
            } else  {
              if((dest+3) >= destEnd)
                goto error1;
              *dest++ = (PRUnichar) 0x1b;
              *dest++ = (PRUnichar) '$';
              if(0x80 & *src)
                goto error2;
              *dest++ = (PRUnichar) *src;
              mState = mLastLegalState;
            }
          break;

          case mState_ESC_24_28: // ESC $ (
            if( 'C' == *src) {
              mState = mState_KSC5601_1987;
              mRunLength = 0;
            } else if ('D' == *src) {
              mState = mState_JISX0212_1990;
              mRunLength = 0;
            } else  {
              if((dest+4) >= destEnd)
                goto error1;
              *dest++ = (PRUnichar) 0x1b;
              *dest++ = (PRUnichar) '$';
              *dest++ = (PRUnichar) '(';
              if(0x80 & *src)
                goto error2;
              *dest++ = (PRUnichar) *src;
              mState = mLastLegalState;
            }
          break;

          case mState_JISX0201_1976Roman:
            if(0x1b == *src) {
              mLastLegalState = mState;
              mState = mState_ESC;
            } else if(*src & 0x80) {
              goto error2;
            } else {
              // XXX We need to  decide how to handle \ and ~ here
              // we may need a if statement here for '\' and '~' 
              // to map them to Yen and Overbar
              *dest++ = (PRUnichar) *src;
              ++mRunLength;
              if(dest >= destEnd)
                goto error1;
            }
          break;

          case mState_JISX0201_1976Kana:
            if(0x1b == *src) {
              mLastLegalState = mState;
              mState = mState_ESC;
            } else {
              if((0x21 <= *src) && (*src <= 0x5F)) {
                *dest++ = (0xFF61-0x0021) + *src;
                ++mRunLength;
              } else {
                goto error2;
              }
              if(dest >= destEnd)
                goto error1;
            }
          break;

          case mState_JISX0208_1978:
            if(0x1b == *src) {
              mLastLegalState = mState;
              mState = mState_ESC;
            } else if(*src & 0x80) {
              mLastLegalState = mState;
              mState = mState_ERROR;
            } else {
              mData = JIS0208_INDEX[*src & 0x7F];
              if(0xFFFD == mData)
                goto error2;
              mState = mState_JISX0208_1978_2ndbyte;
            }
          break;

          case mState_GB2312_1980:
            if(0x1b == *src) {
              mLastLegalState = mState;
              mState = mState_ESC;
            } else if(*src & 0x80) {
              mLastLegalState = mState;
              mState = mState_ERROR;
            } else {
              mData = fbIdx[*src & 0x7F];
              if(0xFFFD == mData)
                goto error2;
              mState = mState_GB2312_1980_2ndbyte;
            }
          break;

          case mState_JISX0208_1983:
            if(0x1b == *src) {
              mLastLegalState = mState;
              mState = mState_ESC;
            } else if(*src & 0x80) {
              mLastLegalState = mState;
              mState = mState_ERROR;
            } else {
              mData = JIS0208_INDEX[*src & 0x7F];
              if(0xFFFD == mData)
                goto error2;
              mState = mState_JISX0208_1983_2ndbyte;
            }
          break;

          case mState_KSC5601_1987:
            if(0x1b == *src) {
              mLastLegalState = mState;
              mState = mState_ESC;
            } else if(*src & 0x80) {
              mLastLegalState = mState;
              mState = mState_ERROR;
            } else {
              mData = fbIdx[*src & 0x7F];
              if(0xFFFD == mData)
                goto error2;
              mState = mState_KSC5601_1987_2ndbyte;
            }
          break;

          case mState_JISX0212_1990:
            if(0x1b == *src) {
              mLastLegalState = mState;
              mState = mState_ESC;
            } else if(*src & 0x80) {
              mLastLegalState = mState;
              mState = mState_ERROR;
            } else {
              mData = JIS0212_INDEX[*src & 0x7F];
              if(0xFFFD == mData)
                goto error2;
              mState = mState_JISX0212_1990_2ndbyte;
            }
          break;

          case mState_JISX0208_1978_2ndbyte:
          {
            PRUint8 off = sbIdx[*src];
            if(0xFF == off) {
               goto error2;
            } else {
               // XXX We need to map from JIS X 0208 1983 to 1987 
               // in the next line before pass to *dest++
               *dest++ = gJapaneseMap[mData+off];
               ++mRunLength;
            }
            mState = mState_JISX0208_1978;
            if(dest >= destEnd)
              goto error1;
          }
          break;

          case mState_GB2312_1980_2ndbyte:
          {
            PRUint8 off = sbIdx[*src];
            if(0xFF == off) {
               goto error2;
            } else {
              if (!mGB2312Decoder) {
                // creating a delegate converter (GB2312)
                nsresult rv;
                nsCOMPtr<nsICharsetConverterManager> ccm = 
                         do_GetService(kCharsetConverterManagerCID, &rv);
                if (NS_SUCCEEDED(rv)) {
                  rv = ccm->GetUnicodeDecoderRaw("GB2312", &mGB2312Decoder);
                }
              }
              if (!mGB2312Decoder) {// failed creating a delegate converter
                goto error2;
              } else {
                unsigned char gb[2];
                PRUnichar uni;
                PRInt32 gbLen = 2, uniLen = 1;
                // ((mData/94)+0x21) is the original 1st byte.
                // *src is the present 2nd byte.
                // Put 2 bytes (one character) to gb[] with GB2312 encoding.
                gb[0] = ((mData / 94) + 0x21) | 0x80;
                gb[1] = *src | 0x80;
                // Convert GB2312 to unicode.
                mGB2312Decoder->Convert((const char *)gb, &gbLen,
                                        &uni, &uniLen);
                *dest++ = uni;
                ++mRunLength;
              }
            }
            mState = mState_GB2312_1980;
            if(dest >= destEnd)
              goto error1;
          }
          break;

          case mState_JISX0208_1983_2ndbyte:
          {
            PRUint8 off = sbIdx[*src];
            if(0xFF == off) {
               goto error2;
            } else {
               *dest++ = gJapaneseMap[mData+off];
               ++mRunLength;
            }
            mState = mState_JISX0208_1983;
            if(dest >= destEnd)
              goto error1;
          }
          break;

          case mState_KSC5601_1987_2ndbyte:
          {
            PRUint8 off = sbIdx[*src];
            if(0xFF == off) {
               goto error2;
            } else {
              if (!mEUCKRDecoder) {
                // creating a delegate converter (EUC-KR)
                nsresult rv;
                nsCOMPtr<nsICharsetConverterManager> ccm = 
                         do_GetService(kCharsetConverterManagerCID, &rv);
                if (NS_SUCCEEDED(rv)) {
                  rv = ccm->GetUnicodeDecoderRaw("EUC-KR", &mEUCKRDecoder);
                }
              }
              if (!mEUCKRDecoder) {// failed creating a delegate converter
                goto error2;
              } else {              
                unsigned char ksc[2];
                PRUnichar uni;
                PRInt32 kscLen = 2, uniLen = 1;
                // ((mData/94)+0x21) is the original 1st byte.
                // *src is the present 2nd byte.
                // Put 2 bytes (one character) to ksc[] with EUC-KR encoding.
                ksc[0] = ((mData / 94) + 0x21) | 0x80;
                ksc[1] = *src | 0x80;
                // Convert EUC-KR to unicode.
                mEUCKRDecoder->Convert((const char *)ksc, &kscLen,
                                       &uni, &uniLen);
                *dest++ = uni;
                ++mRunLength;
              }
            }
            mState = mState_KSC5601_1987;
            if(dest >= destEnd)
              goto error1;
          }
          break;

          case mState_JISX0212_1990_2ndbyte:
          {
            PRUint8 off = sbIdx[*src];
            if(0xFF == off) {
               goto error2;
            } else {
               *dest++ = gJapaneseMap[mData+off];
               ++mRunLength;
            }
            mState = mState_JISX0212_1990;
            if(dest >= destEnd)
              goto error1;
          }
          break;

          case mState_ESC_2e: // ESC .
            // "ESC ." will designate 96 character set to G2.
            mState = mLastLegalState;
            if( 'A' == *src) {
              G2charset = G2_ISO88591;
            } else if ('F' == *src) {
              G2charset = G2_ISO88597;
            } else  {
              if((dest+3) >= destEnd)
                goto error1;
              *dest++ = (PRUnichar) 0x1b;
              *dest++ = (PRUnichar) '.';
              if(0x80 & *src)
                goto error2;
              *dest++ = (PRUnichar) *src;
            }
          break;

          case mState_ESC_4e: // ESC N
            // "ESC N" is the SS2 sequence, that invoke a G2 designated
            // character set.  Since SS2 is effective only for next one
            // character, mState should be returned to the last status.
            mState = mLastLegalState;
            if((0x20 <= *src) && (*src <= 0x7F)) {
              if (G2_ISO88591 == G2charset) {
                *dest++ = *src | 0x80;
                ++mRunLength;
              } else if (G2_ISO88597 == G2charset) {
                if (!mISO88597Decoder) {
                  // creating a delegate converter (ISO-8859-7)
                  nsresult rv;
                  nsCOMPtr<nsICharsetConverterManager> ccm = 
                           do_GetService(kCharsetConverterManagerCID, &rv);
                  if (NS_SUCCEEDED(rv)) {
                    rv = ccm->GetUnicodeDecoderRaw("ISO-8859-7", &mISO88597Decoder);
                  }
                }
                if (!mISO88597Decoder) {// failed creating a delegate converter
                  goto error2;
                } else {
                  // Put one character with ISO-8859-7 encoding.
                  unsigned char gr = *src | 0x80;
                  PRUnichar uni;
                  PRInt32 grLen = 1, uniLen = 1;
                  // Convert ISO-8859-7 to unicode.
                  mISO88597Decoder->Convert((const char *)&gr, &grLen,
                                            &uni, &uniLen);
                  *dest++ = uni;
                  ++mRunLength;
                }
              } else {// G2charset is G2_unknown (not designated yet)
                goto error2;
              }
              if(dest >= destEnd)
                goto error1;
            } else {
              if((dest+3) >= destEnd)
                goto error1;
              *dest++ = (PRUnichar) 0x1b;
              *dest++ = (PRUnichar) 'N';
              if(0x80 & *src)
                goto error2;
              *dest++ = (PRUnichar) *src;
            }
          break;

          case mState_ERROR:
             mState = mLastLegalState;
             mRunLength = 0;
             goto error2;
          break;

       } // switch
       src++;
   }
   *aDestLen = dest - aDest;
   return NS_OK;
error1:
   *aDestLen = dest-aDest;
   src++;
   if ((mState == 0) && (src == srcEnd)) {
     return NS_OK;
   }
   *aSrcLen = src - (const unsigned char*)aSrc;
   return NS_OK_UDEC_MOREOUTPUT;
error2:
   *aSrcLen = src - (const unsigned char*)aSrc;
   *aDestLen = dest-aDest;
   return NS_ERROR_UNEXPECTED;
}
