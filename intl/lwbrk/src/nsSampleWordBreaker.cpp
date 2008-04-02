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


#include "nsSampleWordBreaker.h"

#include "pratom.h"
#include "nsLWBRKDll.h"
nsSampleWordBreaker::nsSampleWordBreaker()
{
}
nsSampleWordBreaker::~nsSampleWordBreaker()
{
}

NS_IMPL_ISUPPORTS1(nsSampleWordBreaker, nsIWordBreaker)

PRBool nsSampleWordBreaker::BreakInBetween(
  const PRUnichar* aText1 , PRUint32 aTextLen1,
  const PRUnichar* aText2 , PRUint32 aTextLen2)
{
  NS_PRECONDITION( nsnull != aText1, "null ptr");
  NS_PRECONDITION( nsnull != aText2, "null ptr");

  if(!aText1 || !aText2 || (0 == aTextLen1) || (0 == aTextLen2))
    return PR_FALSE;

  return (this->GetClass(aText1[aTextLen1-1]) != this->GetClass(aText2[0]));
}


#define IS_ASCII(c)            (0 == ( 0xFF80 & (c)))
#define ASCII_IS_ALPHA(c)         ((( 'a' <= (c)) && ((c) <= 'z')) || (( 'A' <= (c)) && ((c) <= 'Z')))
#define ASCII_IS_DIGIT(c)         (( '0' <= (c)) && ((c) <= '9'))
#define ASCII_IS_SPACE(c)         (( ' ' == (c)) || ( '\t' == (c)) || ( '\r' == (c)) || ( '\n' == (c)))
#define IS_ALPHABETICAL_SCRIPT(c) ((c) < 0x2E80) 

// we change the beginning of IS_HAN from 0x4e00 to 0x3400 to relfect Unicode 3.0 
#define IS_HAN(c)              (( 0x3400 <= (c)) && ((c) <= 0x9fff))||(( 0xf900 <= (c)) && ((c) <= 0xfaff))
#define IS_KATAKANA(c)         (( 0x30A0 <= (c)) && ((c) <= 0x30FF))
#define IS_HIRAGANA(c)         (( 0x3040 <= (c)) && ((c) <= 0x309F))
#define IS_HALFWIDTHKATAKANA(c)         (( 0xFF60 <= (c)) && ((c) <= 0xFF9F))
#define IS_THAI(c)         (0x0E00 == (0xFF80 & (c) )) // Look at the higest 9 bits

PRUint8 nsSampleWordBreaker::GetClass(PRUnichar c)
{
  // begin of the hack

  if (IS_ALPHABETICAL_SCRIPT(c))  {
	  if(IS_ASCII(c))  {
		  if(ASCII_IS_SPACE(c)) {
			  return kWbClassSpace;
		  } else if(ASCII_IS_ALPHA(c) || ASCII_IS_DIGIT(c)) {
			  return kWbClassAlphaLetter;
		  } else {
			  return kWbClassPunct;
		  }
	  } else if(IS_THAI(c))	{
		  return kWbClassThaiLetter;
	  } else if (c == 0x00A0/*NBSP*/) {
      return kWbClassSpace;
    } else {
		  return kWbClassAlphaLetter;
	  }
  }  else {
	  if(IS_HAN(c)) {
		  return kWbClassHanLetter;
	  } else if(IS_KATAKANA(c))   {
		  return kWbClassKatakanaLetter;
	  } else if(IS_HIRAGANA(c))   {
		  return kWbClassHiraganaLetter;
	  } else if(IS_HALFWIDTHKATAKANA(c))  {
		  return kWbClassHWKatakanaLetter;
	  } else  {
		  return kWbClassAlphaLetter;
	  }
  }
  return 0;
}

nsWordRange nsSampleWordBreaker::FindWord(
  const PRUnichar* aText , PRUint32 aTextLen,
  PRUint32 aOffset)
{
  nsWordRange range;
  NS_PRECONDITION( nsnull != aText, "null ptr");
  NS_PRECONDITION( 0 != aTextLen, "len = 0");
  NS_PRECONDITION( aOffset <= aTextLen, "aOffset > aTextLen");

  range.mBegin = aTextLen + 1;
  range.mEnd = aTextLen + 1;

  if(!aText || aOffset > aTextLen)
    return range;

  PRUint8 c = this->GetClass(aText[aOffset]);
  PRUint32 i;
  // Scan forward
  range.mEnd--;
  for(i = aOffset +1;i <= aTextLen; i++)
  {
     if( c != this->GetClass(aText[i]))
     {
       range.mEnd = i;
       break;
     }
  }

  // Scan backward
  range.mBegin = 0;
  for(i = aOffset ;i > 0; i--)
  {
     if( c != this->GetClass(aText[i-1]))
     {
       range.mBegin = i;
       break;
     }
  }
  if(kWbClassThaiLetter == c)
  {
	// need to call Thai word breaker from here
	// we should pass the whole Thai segment to the thai word breaker to find a shorter answer
  }
  return range;
}

PRInt32 nsSampleWordBreaker::NextWord( 
  const PRUnichar* aText, PRUint32 aLen, PRUint32 aPos) 
{
  PRInt8 c1, c2;
  PRUint32 cur = aPos;
  if (cur == aLen)
    return NS_WORDBREAKER_NEED_MORE_TEXT;
  c1 = this->GetClass(aText[cur]);
 
  for(cur++; cur <aLen; cur++)
  {
     c2 = this->GetClass(aText[cur]);
     if(c2 != c1) 
       break;
  }
  if(kWbClassThaiLetter == c1)
  {
	// need to call Thai word breaker from here
	// we should pass the whole Thai segment to the thai word breaker to find a shorter answer
  }
  if (cur == aLen)
    return NS_WORDBREAKER_NEED_MORE_TEXT;
  return cur;
}

PRInt32 nsSampleWordBreaker::PrevWord(
  const PRUnichar* aText, PRUint32 aLen, PRUint32 aPos) 
{
  PRInt8 c1, c2;
  PRUint32 cur = aPos;
  if (cur == aLen) {
    if (cur == 0)
      return NS_WORDBREAKER_NEED_MORE_TEXT;
    --cur;
  }
  c1 = this->GetClass(aText[cur]);

  for(; cur > 0; cur--)
  {
     c2 = this->GetClass(aText[cur-1]);
     if(c2 != c1)
       break;
  }
  if(kWbClassThaiLetter == c1)
  {
	// need to call Thai word breaker from here
	// we should pass the whole Thai segment to the thai word breaker to find a shorter answer
  }
  if (!cur)
    return NS_WORDBREAKER_NEED_MORE_TEXT;
  return cur;
}
