/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
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
 * Portions created by the Initial Developer are Copyright (C) 1999
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Ben Turner <mozilla@songbirdnest.com>
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

#include "pratom.h"
#include "nsUUDll.h"
#include "nsCaseConversionImp2.h"
#include "casetable.h"

// For gUpperToTitle 
enum {
  kUpperIdx =0,
  kTitleIdx
};

// For gUpperToTitle
enum {
  kLowIdx =0,
  kSizeEveryIdx,
  kDiffIdx
};

#define IS_ASCII(u)       ( 0x0000 == ((u) & 0xFF80))
#define IS_ASCII_UPPER(u) ((0x0041 <= (u)) && ( (u) <= 0x005a))
#define IS_ASCII_LOWER(u) ((0x0061 <= (u)) && ( (u) <= 0x007a))
#define IS_ASCII_ALPHA(u) (IS_ASCII_UPPER(u) || IS_ASCII_LOWER(u))
#define IS_ASCII_SPACE(u) ( 0x0020 == (u) )

#define IS_NOCASE_CHAR(u)  (0==(1&(gCaseBlocks[(u)>>13]>>(0x001F&((u)>>8)))))
  
// Size of Tables

#define CASE_MAP_CACHE_SIZE 0x40
#define CASE_MAP_CACHE_MASK 0x3F

nsCaseConversionImp2* gCaseConv = nsnull;

struct nsCompressedMap {
  const PRUnichar *mTable;
  PRUint32 mSize;
  PRUint32 mCache[CASE_MAP_CACHE_SIZE];
  PRUint32 mLastBase;

  PRUnichar Map(PRUnichar aChar)
  {
    // no need to worry about thread safety since cached values are
    // not objects but primitive data types which could be 
    // accessed in atomic operations. We need to access
    // the whole 32 bit of cachedData at once in order to make it
    // thread safe. Never access bits from mCache directly.

    PRUint32 cachedData = mCache[aChar & CASE_MAP_CACHE_MASK];
    if(aChar == ((cachedData >> 16) & 0x0000FFFF))
      return (cachedData & 0x0000FFFF);

    // try the last index first
    // store into local variable so we can be thread safe
    PRUint32 base = mLastBase; 
    PRUnichar res = 0;
    
    if (( aChar <=  ((mTable[base+kSizeEveryIdx] >> 8) + 
                  mTable[base+kLowIdx])) &&
        ( mTable[base+kLowIdx]  <= aChar )) 
    {
        // Hit the last base
        if(((mTable[base+kSizeEveryIdx] & 0x00FF) > 0) && 
          (0 != ((aChar - mTable[base+kLowIdx]) % 
                (mTable[base+kSizeEveryIdx] & 0x00FF))))
        {
          res = aChar;
        } else {
          res = aChar + mTable[base+kDiffIdx];
        }
    } else {
        res = this->Lookup(0, (mSize/2), mSize-1, aChar);
    }

    mCache[aChar & CASE_MAP_CACHE_MASK] =
        (((aChar << 16) & 0xFFFF0000) | (0x0000FFFF & res));
    return res;
  }

  PRUnichar Lookup(PRUint32 l,
                   PRUint32 m,
                   PRUint32 r,
                   PRUnichar aChar)
  {
    PRUint32 base = m*3;
    if ( aChar >  ((mTable[base+kSizeEveryIdx] >> 8) + 
                  mTable[base+kLowIdx])) 
    {
      if( l > m )
        return aChar;
      PRUint32 newm = (m+r+1)/2;
      if(newm == m)
        newm++;
      return this->Lookup(m+1, newm , r, aChar);
      
    } else if ( mTable[base+kLowIdx]  > aChar ) {
      if( r < m )
        return aChar;
      PRUint32 newm = (l+m-1)/2;
      if(newm == m)
        newm++;
      return this->Lookup(l, newm, m-1, aChar);

    } else  {
      if(((mTable[base+kSizeEveryIdx] & 0x00FF) > 0) && 
        (0 != ((aChar - mTable[base+kLowIdx]) % 
                (mTable[base+kSizeEveryIdx] & 0x00FF))))
      {
        return aChar;
      }
      mLastBase = base; // cache the base
      return aChar + mTable[base+kDiffIdx];
    }
  }
};

static nsCompressedMap gUpperMap = {
  reinterpret_cast<const PRUnichar*>(&gToUpper[0]),
  gToUpperItems
};

static nsCompressedMap gLowerMap = {
  reinterpret_cast<const PRUnichar*>(&gToLower[0]),
  gToLowerItems
};

nsCaseConversionImp2* nsCaseConversionImp2::GetInstance()
{
  if (!gCaseConv)
    NS_NEWXPCOM(gCaseConv, nsCaseConversionImp2);
  return gCaseConv;
}

NS_IMETHODIMP_(nsrefcnt) nsCaseConversionImp2::AddRef(void)
{
  return (nsrefcnt)1;
}

NS_IMETHODIMP_(nsrefcnt) nsCaseConversionImp2::Release(void)
{
  return (nsrefcnt)1;
}

NS_IMPL_THREADSAFE_QUERY_INTERFACE1(nsCaseConversionImp2, nsICaseConversion)

nsresult nsCaseConversionImp2::ToUpper(
  PRUnichar aChar, PRUnichar* aReturn
)
{
  if( IS_ASCII(aChar)) // optimize for ASCII
  {
     if(IS_ASCII_LOWER(aChar))
        *aReturn = aChar - 0x0020;
     else
        *aReturn = aChar;
  } 
  else if( IS_NOCASE_CHAR(aChar)) // optimize for block which have no case
  {
    *aReturn = aChar;
  } 
  else 
  {
    *aReturn = gUpperMap.Map(aChar);
  }
  return NS_OK;
}

// a non-virtual version of ToLower
static PRUnichar FastToLower(
  PRUnichar aChar
)
{
  if( IS_ASCII(aChar)) // optimize for ASCII
  {
     if(IS_ASCII_UPPER(aChar))
        return aChar + 0x0020;
     else
        return aChar;
  }
  else if( IS_NOCASE_CHAR(aChar)) // optimize for block which have no case
  {
     return aChar;
  }

  return gLowerMap.Map(aChar);
}

nsresult nsCaseConversionImp2::ToLower(
  PRUnichar aChar, PRUnichar* aReturn
)
{
  *aReturn = FastToLower(aChar);
  return NS_OK;
}

nsresult nsCaseConversionImp2::ToTitle(
  PRUnichar aChar, PRUnichar* aReturn
)
{
  if( IS_ASCII(aChar)) // optimize for ASCII
  {
    return this->ToUpper(aChar, aReturn);
  }
  else if( IS_NOCASE_CHAR(aChar)) // optimize for block which have no case
  {
    *aReturn = aChar;
  } 
  else
  {
    // First check for uppercase characters whose titlecase mapping is
    //  different, like U+01F1 DZ: they must remain unchanged.
    if( 0x01C0 == ( aChar & 0xFFC0)) // 0x01Cx - 0x01Fx
    {
      for(PRUint32 i = 0 ; i < gUpperToTitleItems; i++) {
        if ( aChar == gUpperToTitle[(i*2)+kUpperIdx]) {
          *aReturn = aChar;
          return NS_OK;
        }
      }
    }

    PRUnichar upper = gUpperMap.Map(aChar);
    
    if( 0x01C0 == ( upper & 0xFFC0)) // 0x01Cx - 0x01Fx
    {
      for(PRUint32 i = 0 ; i < gUpperToTitleItems; i++) {
        if ( upper == gUpperToTitle[(i*2)+kUpperIdx]) {
           *aReturn = gUpperToTitle[(i*2)+kTitleIdx];
           return NS_OK;
        }
      }
    }
    *aReturn = upper;
  }
  return NS_OK;
}

nsresult nsCaseConversionImp2::ToUpper(
  const PRUnichar* anArray, PRUnichar* aReturn, PRUint32 aLen
)
{
  PRUint32 i;
  for(i=0;i<aLen;i++) 
  {
    PRUnichar aChar = anArray[i];
    if( IS_ASCII(aChar)) // optimize for ASCII
    {
       if(IS_ASCII_LOWER(aChar))
          aReturn[i] = aChar - 0x0020;
       else
          aReturn[i] = aChar;
    }
    else if( IS_NOCASE_CHAR(aChar)) // optimize for block which have no case
    {
          aReturn[i] = aChar;
    } 
    else 
    {
      aReturn[i] = gUpperMap.Map(aChar);
    }
  }
  return NS_OK;
}

nsresult nsCaseConversionImp2::ToLower(
  const PRUnichar* anArray, PRUnichar* aReturn, PRUint32 aLen
)
{
  PRUint32 i;
  for(i=0;i<aLen;i++) 
    aReturn[i] = FastToLower(anArray[i]);
  return NS_OK;
}



nsresult nsCaseConversionImp2::ToTitle(
  const PRUnichar* anArray, PRUnichar* aReturn, PRUint32 aLen,
  PRBool aStartInWordBoundary
)
{
  if(0 == aLen)
    return NS_OK;

  //
  // We need to replace this implementation to a real one
  // Currently, it only do the right thing for ASCII
  // Howerver, we need a word breaker to do the right job
  //
  // this->ToLower(anArray, aReturn, aLen); 
  // CSS define Capitalize as 
  //  Uppercases the first character of each word
  // 
  
  PRBool bLastIsSpace =  IS_ASCII_SPACE(anArray[0]);
  if(aStartInWordBoundary)
  {
     this->ToTitle(anArray[0], &aReturn[0]);
  }

  PRUint32 i;
  for(i=1;i<aLen;i++)
  {
    if(bLastIsSpace)
    {
      this->ToTitle(anArray[i], &aReturn[i]);
    }
    else
    {
      aReturn[i] = anArray[i];
    }

    bLastIsSpace = IS_ASCII_SPACE(aReturn[i]);
  }
  return NS_OK;
}

// implementation moved from the old nsCRT routine
NS_IMETHODIMP
nsCaseConversionImp2::CaseInsensitiveCompare(const PRUnichar *aLeft,
                                             const PRUnichar *aRight,
                                             PRUint32 aCount, PRInt32* aResult)
{
  if (!aLeft || !aRight)
    return NS_ERROR_INVALID_POINTER;

  // assume equality. We bail early if no equality
  *aResult = 0;
  
  if (aCount) {
    do {
      PRUnichar c1 = *aLeft++;
      PRUnichar c2 = *aRight++;
      
      if (c1 != c2) {
        c1 = FastToLower(c1);
        c2 = FastToLower(c2);
        if (c1 != c2) {
          if (c1 < c2) {
            *aResult = -1;
            return NS_OK;
          }
          *aResult = 1;
          return NS_OK;
        }
      }
    } while (--aCount != 0);
  }
  return NS_OK;
}
