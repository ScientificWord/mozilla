/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:expandtab:shiftwidth=2:tabstop=2:
 */
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
 * The Original Code is GNU C Library code (http://www.gnu.org)
 *
 * The Initial Developer of the Original Code is
 * Bruno Haible <bruno@clisp.org>.
 * Portions created by the Initial Developer are Copyright (C) 2002
 * the Free Software Foundation. All Rights Reserved.
 *
 * Contributor(s): 
 *   Jungshik Shin <jshin@mailaps.org> 
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

#include "nsUnicodeToTSCII.h"
#include "nsMemory.h"
#include "tamil.h"

/* 
 *  TSCII is an 8-bit encoding consisting of:
 *  0x00..0x7F:       ASCII
 *  0x80..0x90, 0x95..0x9F, 0xAB..0xFE:
 *                    Tamil letters and glyphs
 *  0xA1..0xA5, 0xAA: Tamil combining letters (after the base character)
 *  0xA6..0xA8:       Tamil combining letters (before the base character)
 *  0x91..0x94:       Punctuation
 *  0xA9:             Symbols
 */

//----------------------------------------------------------------------
// Class nsUnicodeToTSCII [implementation]
  
NS_IMPL_ISUPPORTS2(nsUnicodeToTSCII, nsIUnicodeEncoder, nsICharRepresentable)

/* 
 * During UCS-4 to TSCII conversion, mState contains 
 * the last byte (or sometimes the last two bytes) to be output.
 * This can be:
 *   0x00                     Nothing pending.
 *   0xB8..0xC9, 0x83..0x86   A consonant.
 *   0xEC, 0x8A               A consonant with VIRAMA sign (final or joining).
 *   0x87, 0xC38A             Two consonants combined through a VIRAMA sign. 
 */

static const PRUint8 UnicharToTSCII[] =
{
     0,    0,    0, 0xb7,    0, 0xab, 0xac, 0xfe, // 0x0B80..0x0B87
  0xae, 0xaf, 0xb0,    0,    0,    0, 0xb1, 0xb2, // 0x0B88..0x0B8F
  0xb3,    0, 0xb4, 0xb5, 0xb6, 0xb8,    0,    0, // 0x0B90..0x0B97
     0, 0xb9, 0xba,    0, 0x83,    0, 0xbb, 0xbc, // 0x0B98..0x0B9F
     0,    0,    0, 0xbd, 0xbe,    0,    0,    0, // 0x0BA0..0x0BA7
  0xbf, 0xc9, 0xc0,    0,    0,    0, 0xc1, 0xc2, // 0x0BA8..0x0BAF
  0xc3, 0xc8, 0xc4, 0xc7, 0xc6, 0xc5,    0, 0x84, // 0x0BB0..0x0BB7
  0x85, 0x86,    0,    0,    0,    0, 0xa1, 0xa2, // 0x0BB8..0x0BBF
  0xa3, 0xa4, 0xa5,    0,    0,    0, 0xa6, 0xa7, // 0x0BC0..0x0BC7
  0xa8,    0,    0,    0,    0,    0,    0,    0, // 0x0BC8..0x0BCF
     0,    0,    0,    0,    0,    0,    0, 0xaa, // 0x0BD0..0x0BD7
     0,    0,    0,    0,    0,    0,    0,    0, // 0x0BD8..0x0BDF
     0,    0,    0,    0,    0,    0, 0x80, 0x81, // 0x0BE0..0x0BE7
  0x8d, 0x8e, 0x8f, 0x90, 0x95, 0x96, 0x97, 0x98, // 0x0BE8..0x0BEF
  0x9d, 0x9e, 0x9f,    0,    0,    0,    0,    0, // 0x0BF0..0x0BF7
     0,    0,    0,    0,    0,    0,    0,    0  // 0x0BF8..0x0BFF
};

static const PRUint8 consonant_with_u[] =
{
  0xcc, 0x99, 0xcd, 0x9a, 0xce, 0xcf, 0xd0, 0xd1, 0xd2,
  0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb
};

static const PRUint8 consonant_with_uu[] =
{
  0xdc, 0x9b, 0xdd, 0x9c, 0xde, 0xdf, 0xe0, 0xe1, 0xe2,
  0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb
};

static const PRUint8 consonant_with_virama[18] =
{
  0xec, 0xed, 0xee, 0xef, 0xf0, 0xf1, 0xf2, 0xf3, 0xf4,
  0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd
};


// Modified implementation of Unicode to TSCII converter in glibc by 
// Bruno Haible.  My modifications are based on Unicode 3.0 chap. 9 and 
// the code chart for Tamil. 
NS_IMETHODIMP 
nsUnicodeToTSCII::Convert(const PRUnichar * aSrc, PRInt32 * aSrcLength, 
                          char * aDest, PRInt32 * aDestLength)
{
  const PRUnichar * src = aSrc;
  const PRUnichar * srcEnd = aSrc + *aSrcLength;
  char * dest = aDest;
  char * destEnd = dest + *aDestLength;

  nsresult rv = NS_OK;
                      
  while (src < srcEnd && dest < destEnd) {
    PRUnichar ch = *src;
    if (mBuffer) {                        
      // Attempt to combine the last character with this one.
      PRUint32 last = mBuffer;
                            
      // last : consonant 
      if (IS_TSC_CONSONANT(last)) {                      
        if (ch == UNI_VOWELSIGN_U && IS_TSC_CONSONANT1(last)) {                      
          *dest++ = consonant_with_u[last - TSC_KA];
          mBuffer = 0;                  
          ++src;
          continue;
        }                      
  
        if (ch == UNI_VOWELSIGN_UU && IS_TSC_CONSONANT1(last)) {                      
          *dest++ = consonant_with_uu[last - TSC_KA];          
          mBuffer = 0;                  
          ++src;                  
          continue;                  
        }                      
  
        // reorder. vowel sign goes to the left of consonant
        if (IS_UNI_LEFT_VOWELSIGN(ch)) {                      
          if (dest + 2 > destEnd)
            goto error_more_output;
          *dest++ = TSC_LEFT_VOWELSIGN(ch);
          *dest++ = last;                
          mBuffer = 0;                
          ++src;                  
          continue;                  
        }                      
  
        // split and reorder. consonant goes bet. two parts
        if (IS_UNI_2PARTS_VOWELSIGN(ch)) {                      
          if (dest + 3 > destEnd)
            goto error_more_output;
          *dest++ = TSC_LEFT_VOWEL_PART(ch);
          *dest++ = last;                
          *dest++ = TSC_RIGHT_VOWEL_PART(ch);
          mBuffer = 0;                
          ++src;                  
          continue;                  
        }                      
  
        // Virama
        if (ch == UNI_VIRAMA) {                      
          // consonant KA can form a conjunct with consonant SSA(SHA).
          // buffer dead consonant 'K' for the now.
          if (last == TSC_KA) {                 
            mBuffer = TSC_KA_DEAD;
          }
          // SA can form a conjunct when followed by 'RA'. 
          // buffer dead consonant 'S' for the now.
          else if (last == TSC_SA) {
            mBuffer = TSC_SA_DEAD;                
          }
          else {                    
            *dest++ = IS_TSC_CONSONANT1(last) ?
              consonant_with_virama[last - TSC_KA] : last + 5;
            mBuffer = 0;                
          }                    
          ++src;                  
          continue;                  
        }                      

        // consonant TA forms a ligature with vowel 'I' or 'II'.
        if (last == TSC_TA && (ch == UNI_VOWELSIGN_I || ch == UNI_VOWELSIGN_II)) {                      
          *dest++ = ch - (UNI_VOWELSIGN_I - TSC_TI_LIGA);
          mBuffer = 0;                  
          ++src;                  
          continue;                  
        }                      
      }                      
      else if (last == TSC_KA_DEAD) {                      
        // Kd + SSA =  K.SSA
        if (ch == UNI_SSA) {                      
          mBuffer = TSC_KSSA; 
          ++src;                  
          continue;                  
        }                      
      }                      
      else if (last == TSC_SA_DEAD) {                      
        // Sd + RA = S.RA. Buffer RA + Sd. 
        if (ch == UNI_RA) {                      
          mBuffer = 0xc38a;                
          ++src;                  
          continue;                  
        }                      
      }                      
      else if (last == TSC_KSSA) {                      
        if (ch == UNI_VIRAMA) {
          *dest++ = (char) TSC_KSSA_DEAD;
          mBuffer = 0;                  
          ++src;                  
          continue;                  
        }                      

        // vowel splitting/reordering should be done around conjuncts as well.
        // reorder. vowel sign goes to the left of consonant
        if (IS_UNI_LEFT_VOWELSIGN(ch)) {                      
          if (dest + 2 > destEnd)
            goto error_more_output;
          *dest++ = TSC_LEFT_VOWELSIGN(ch);
          *dest++ = last;                
          mBuffer = 0;                
          ++src;                  
          continue;                  
        }                      
  
        // split and reorder. consonant goes bet. two parts
        if (IS_UNI_2PARTS_VOWELSIGN(ch)) {                      
          if (dest + 3 > destEnd)
            goto error_more_output;
          *dest++ = TSC_LEFT_VOWEL_PART(ch);
          *dest++ = last;                
          *dest++ = TSC_RIGHT_VOWEL_PART(ch);
          mBuffer = 0;                
          ++src;                  
          continue;                  
        }                      
      }                      
      else {
        NS_ASSERTION(last == 0xc38a, "No other value can be buffered");
        if (ch == UNI_VOWELSIGN_II) {                      
          *dest++ = (char) TSC_SRII_LIGA;
          mBuffer = 0;                  
          ++src;                  
          continue;                  
        }                      
        else {
          // put back TSC_SA_DEAD and TSC_RA
          *dest++ = (char) TSC_SA_DEAD;
          mBuffer = TSC_RA;
          ++src;                  
          continue;                  
        }  
      }                      
                          
      /* Output the buffered character.  */              
      if (last >> 8) {                      
        if (dest + 2 >  destEnd)
          goto error_more_output;
        *dest++ = last & 0xff;              
        *dest++ = (last >> 8) & 0xff;              
      }                      
      else                      
        *dest++ = last & 0xff;                
      mBuffer = 0;                    
      continue;                    
    }                        
                        
    if (ch < 0x80)   // Plain ASCII character.
      *dest++ = (char)ch;                    
    else if (IS_UNI_TAMIL(ch)) {                        
      PRUint8 t = UnicharToTSCII[ch - UNI_TAMIL_START];
                            
      if (t != 0) {                      
          if (IS_TSC_CONSONANT(t))
            mBuffer = (PRUint32) t;              
          else                    
            *dest++ = t;                  
      }                      
      else if (IS_UNI_2PARTS_VOWELSIGN(ch)) {   
          // actually this is an illegal sequence.
          if (dest + 2 > destEnd)
            goto error_more_output;

          *dest++ = TSC_LEFT_VOWEL_PART(ch);
          *dest++ = TSC_RIGHT_VOWEL_PART(ch);
      }                      
      else {
        *aDestLength = dest - aDest;
        return NS_ERROR_UENC_NOMAPPING;
      }                      
    }                        
    else if (ch == 0x00A9)                  
      *dest++ = (char)ch;                    
    else if (IS_UNI_SINGLE_QUOTE(ch))
      *dest++ = ch - UNI_LEFT_SINGLE_QUOTE + TSC_LEFT_SINGLE_QUOTE;
    else if (IS_UNI_DOUBLE_QUOTE(ch))
      *dest++ = ch - UNI_LEFT_DOUBLE_QUOTE + TSC_LEFT_DOUBLE_QUOTE;
    else {
      *aDestLength = dest - aDest;
      return NS_ERROR_UENC_NOMAPPING;
    }                        
                        
    /* Now that we wrote the output increment the input pointer.  */        
    ++src;                      
  }

  // flush the buffer
  if (mBuffer >> 8) {                      
    // Write out the last character, two bytes. 
    if (dest + 2 > destEnd)
      goto error_more_output;
    *dest++ = (mBuffer >> 8) & 0xff;            
    *dest++ = mBuffer & 0xff;              
    mBuffer = 0;
  }                      
  else if (mBuffer) {
    // Write out the last character, a single byte.
    if (dest >= destEnd)
      goto error_more_output;
    *dest++ = mBuffer & 0xff;              
    mBuffer = 0;
  }                      

  *aSrcLength = src - aSrc;
  *aDestLength = dest - aDest;
  return rv;

error_more_output:
  *aSrcLength = src - aSrc;
  *aDestLength = dest - aDest;
  return NS_OK_UENC_MOREOUTPUT;
}

NS_IMETHODIMP 
nsUnicodeToTSCII::Finish(char* aDest, PRInt32* aDestLength)
{
  if (!mBuffer) {
    *aDestLength = 0;
    return NS_OK;
  }

  if (mBuffer >> 8) {                      
    // Write out the last character, two bytes. 
    if (*aDestLength < 2) {
      *aDestLength = 0;
      return NS_OK_UENC_MOREOUTPUT;
    }
    *aDest++ = (mBuffer >> 8) & 0xff;            
    *aDest++ = mBuffer & 0xff;              
    mBuffer = 0;
    *aDestLength = 2;
  }                      
  else {                      
    // Write out the last character, a single byte.
    if (*aDestLength < 1) {                    
      *aDestLength = 0;
      return NS_OK_UENC_MOREOUTPUT;
    }
    *aDest++ = mBuffer & 0xff;              
    mBuffer = 0;
    *aDestLength = 1;
  }                      
  return NS_OK;
}

//================================================================
NS_IMETHODIMP 
nsUnicodeToTSCII::Reset()
{
  mBuffer = 0;
  return NS_OK;
}

NS_IMETHODIMP 
nsUnicodeToTSCII::GetMaxLength(const PRUnichar * aSrc, PRInt32 aSrcLength,
                                 PRInt32 * aDestLength)
{
  // Some Tamil letters  can be decomposed into 2 glyphs in TSCII.
  *aDestLength = aSrcLength *  2;
  return NS_OK;
}


NS_IMETHODIMP 
nsUnicodeToTSCII::FillInfo(PRUint32* aInfo)
{
  // Tamil block is so sparse.
  static const PRUint8 coverage[] = {
    0xe8, // 11101000  U+0B87 - U+0B80
    0xc7, // 11000111  U+0B8F - U+0B88
    0x3d, // 00111101  U+0B97 - U+0B90
    0xd6, // 11010110  U+0B9F - U+0B98
    0x18, // 00011000  U+0BA7 - U+0BA0
    0xc7, // 11000111  U+0BAF - U+0BA8
    0xbf, // 10111111  U+0BB7 - U+0BB0
    0xc7, // 11000111  U+0BBF - U+0BB8
    0xc7, // 11000111  U+0BC7 - U+0BC0
    0x3d, // 00111101  U+0BCF - U+0BC8
    0x80, // 10000000  U+0BD7 - U+0BD0
    0x00, // 00000000  U+0BDF - U+0BD8
    0x80, // 10000000  U+0BE7 - U+0BE0
    0xff, // 11111111  U+0BEF - U+0BE8
    0x07, // 00000111  U+0BF7 - U+0BF0
  };

  PRUnichar i;
  for(i = 0; i <  0x78; i++)
    if (coverage[i / 8] & (1 << (i % 8)))
      SET_REPRESENTABLE(aInfo, i + UNI_TAMIL_START);

  // TSCII is a superset of US-ASCII.
  for(i = 0x20; i < 0x7f; i++)
     SET_REPRESENTABLE(aInfo, i);

  // additional characters in TSCII
  SET_REPRESENTABLE(aInfo, 0xA9);   // copyright sign
  SET_REPRESENTABLE(aInfo, UNI_LEFT_SINGLE_QUOTE);
  SET_REPRESENTABLE(aInfo, UNI_RIGHT_SINGLE_QUOTE);
  SET_REPRESENTABLE(aInfo, UNI_LEFT_DOUBLE_QUOTE);
  SET_REPRESENTABLE(aInfo, UNI_RIGHT_DOUBLE_QUOTE);

  return NS_OK;
}

NS_IMETHODIMP 
nsUnicodeToTSCII::SetOutputErrorBehavior(PRInt32 aBehavior, 
                                           nsIUnicharEncoder *aEncoder, 
                                           PRUnichar aChar)
{
  return NS_OK;
}


// same as the mapping of the C1(0x80-0x9f) part of  Windows-1252 to Unicode
const static PRUnichar gTSCIIToTTF[] = {
  0x20AC, 0x0081, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021,
  0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0x008D, 0x017D, 0x008F,
  0x0090, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
  0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0x009D, 0x017E, 0x0178
};

//----------------------------------------------------------------------
// Class nsUnicodeToTamilTTF [implementation]
//
NS_IMPL_ISUPPORTS_INHERITED0(nsUnicodeToTamilTTF, nsUnicodeToTSCII)

NS_IMETHODIMP 
nsUnicodeToTamilTTF::Convert(const PRUnichar * aSrc, 
                             PRInt32 * aSrcLength, char * aDest, 
                             PRInt32 * aDestLength)
{

  PRInt32 medLen, destLen;
  char *med;

  GetMaxLength(aSrc, *aSrcLength, &destLen);
  NS_ASSERTION(destLen  <= *aDestLength, "insufficient dest. buffer size");

  // TSCII converter is a single byte encoder and takes half the space 
  // taken by TamilTTF encoder.
  medLen = destLen / 2; 

  if (medLen > CHAR_BUFFER_SIZE) {
    med = (char *) nsMemory::Alloc(medLen);
    if (!med)
      return NS_ERROR_OUT_OF_MEMORY;
  }
  else 
    med = mStaticBuffer;

  nsresult rv = nsUnicodeToTSCII::Convert(aSrc, aSrcLength, med, &medLen);

  if (NS_FAILED(rv)) {
    if (med != mStaticBuffer)
      nsMemory::Free(med);
    return rv;
  }

  PRInt32 i, j;

  // widen 8bit TSCII to pseudo-Unicode font encoding of TSCII-Tamil font
  for (i = 0, j = 0; i < medLen; i++) {
    // Only C1 part(0x80-0x9f) needs to be mapped as if they're CP1251.
    PRUnichar ucs2 = (med[i] & 0xe0) == 0x80 ? 
                     gTSCIIToTTF[med[i] & 0x7f] : PRUint8(med[i]);
    // A lot of TSCII fonts are still based on TSCII 1.6 so that 
    // they have Tamil vowel 'I' at 0xad instead of 0xfe.
    if (ucs2 == 0xfe) ucs2 = 0xad;
    aDest[j++] = PRUint8((ucs2 & 0xff00) >> 8);
    aDest[j++] = PRUint8(ucs2 & 0x00ff);
  }

  *aDestLength = j;

  if (med != mStaticBuffer)
    nsMemory::Free(med);

  return NS_OK;
}

NS_IMETHODIMP
nsUnicodeToTamilTTF::GetMaxLength(const PRUnichar * aSrc, PRInt32 aSrcLength, PRInt32 * aDestLength)
{
  // Each Tamil character can generate at most two presentation forms,
  // but we're 'extending' them to 16bit shorts, which accounts for 
  // additional factor of 2.
  *aDestLength = (aSrcLength + 1) *  4; 
  
  return NS_OK;
}

NS_IMETHODIMP 
nsUnicodeToTamilTTF::SetOutputErrorBehavior(PRInt32 aBehavior, 
                                            nsIUnicharEncoder *aEncoder, 
                                            PRUnichar aChar)
{
  if (aBehavior == kOnError_CallBack && aEncoder == nsnull)
    return NS_ERROR_NULL_POINTER;
  mErrEncoder = aEncoder;
  mErrBehavior = aBehavior;
  mErrChar = aChar;
  return NS_OK;
}

