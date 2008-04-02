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
 * The Original Code is Mozilla Spellchecker Component.
 *
 * The Initial Developer of the Original Code is
 * David Einstein.
 * Portions created by the Initial Developer are Copyright (C) 2001
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s): David Einstein Deinst@world.std.com
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

#include "mozEnglishWordUtils.h"
#include "nsICharsetAlias.h"
#include "nsReadableUtils.h"
#include "nsIServiceManager.h"
#include "nsUnicharUtilCIID.h"
#include "nsCRT.h"

NS_IMPL_ISUPPORTS1(mozEnglishWordUtils, mozISpellI18NUtil)

mozEnglishWordUtils::mozEnglishWordUtils()
{
  mLanguage.AssignLiteral("en");

  nsresult rv;
  mURLDetector = do_CreateInstance(MOZ_TXTTOHTMLCONV_CONTRACTID, &rv);
  mCaseConv = do_GetService(NS_UNICHARUTIL_CONTRACTID);
  mCategories = do_GetService(NS_UNICHARCATEGORY_CONTRACTID);
}

mozEnglishWordUtils::~mozEnglishWordUtils()
{
}

/* attribute wstring language; */
NS_IMETHODIMP mozEnglishWordUtils::GetLanguage(PRUnichar * *aLanguage)
{
  nsresult rv = NS_OK;
  NS_ENSURE_ARG_POINTER(aLanguage);

  *aLanguage = ToNewUnicode(mLanguage);
  if(!aLanguage) rv = NS_ERROR_OUT_OF_MEMORY;
  return rv;
 }

/* void GetRootForm (in wstring aWord, in PRUint32 type, [array, size_is (count)] out wstring words, out PRUint32 count); */
// return the possible root forms of aWord.
NS_IMETHODIMP mozEnglishWordUtils::GetRootForm(const PRUnichar *aWord, PRUint32 type, PRUnichar ***words, PRUint32 *count)
{
  nsAutoString word(aWord);
  PRUnichar **tmpPtr;
  PRInt32 length = word.Length();

  *count = 0;

  mozEnglishWordUtils::myspCapitalization ct = captype(word);
  switch (ct)
    {
    case HuhCap:
    case NoCap: 
      tmpPtr = (PRUnichar **)nsMemory::Alloc(sizeof(PRUnichar *));
      if (!tmpPtr)
        return NS_ERROR_OUT_OF_MEMORY;
      tmpPtr[0] = ToNewUnicode(word);
      if (!tmpPtr[0]) {
        NS_FREE_XPCOM_ALLOCATED_POINTER_ARRAY(0, tmpPtr);
        return NS_ERROR_OUT_OF_MEMORY;
      }
      *words = tmpPtr;
      *count = 1;
      break;
    

    case AllCap:
      tmpPtr = (PRUnichar **)nsMemory::Alloc(sizeof(PRUnichar *) * 3);
      if (!tmpPtr)
        return NS_ERROR_OUT_OF_MEMORY;
      tmpPtr[0] = ToNewUnicode(word);
      if (!tmpPtr[0]) {
        NS_FREE_XPCOM_ALLOCATED_POINTER_ARRAY(0, tmpPtr);
        return NS_ERROR_OUT_OF_MEMORY;
      }
      mCaseConv->ToLower(tmpPtr[0], tmpPtr[0], length);

      tmpPtr[1] = ToNewUnicode(word);
      if (!tmpPtr[1]) {
        NS_FREE_XPCOM_ALLOCATED_POINTER_ARRAY(1, tmpPtr);
        return NS_ERROR_OUT_OF_MEMORY;
      }
      mCaseConv->ToLower(tmpPtr[1], tmpPtr[1], length);
      mCaseConv->ToUpper(tmpPtr[1], tmpPtr[1], 1);

      tmpPtr[2] = ToNewUnicode(word);
      if (!tmpPtr[2]) {
        NS_FREE_XPCOM_ALLOCATED_POINTER_ARRAY(2, tmpPtr);
        return NS_ERROR_OUT_OF_MEMORY;
      }

      *words = tmpPtr;
      *count = 3;
      break;
 
    case InitCap:  
      tmpPtr = (PRUnichar **)nsMemory::Alloc(sizeof(PRUnichar *) * 2);
      if (!tmpPtr)
        return NS_ERROR_OUT_OF_MEMORY;

      tmpPtr[0] = ToNewUnicode(word);
      if (!tmpPtr[0]) {
        NS_FREE_XPCOM_ALLOCATED_POINTER_ARRAY(0, tmpPtr);
        return NS_ERROR_OUT_OF_MEMORY;
      }
      mCaseConv->ToLower(tmpPtr[0], tmpPtr[0], length);

      tmpPtr[1] = ToNewUnicode(word);
      if (!tmpPtr[1]) {
        NS_FREE_XPCOM_ALLOCATED_POINTER_ARRAY(1, tmpPtr);
        return NS_ERROR_OUT_OF_MEMORY;
      }

      *words = tmpPtr;
      *count = 2;
      break;
    default:
      return NS_ERROR_FAILURE; // should never get here;
    }
  return NS_OK;
}

// This needs vast improvement
PRBool mozEnglishWordUtils::ucIsAlpha(PRUnichar aChar)
{
  // XXX we have to fix callers to handle the full Unicode range
  return nsIUGenCategory::kLetter == mCategories->Get(PRUint32(aChar));
}

/* void FindNextWord (in wstring word, in PRUint32 length, in PRUint32 offset, out PRUint32 begin, out PRUint32 end); */
NS_IMETHODIMP mozEnglishWordUtils::FindNextWord(const PRUnichar *word, PRUint32 length, PRUint32 offset, PRInt32 *begin, PRInt32 *end)
{
  const PRUnichar *p = word + offset;
  const PRUnichar *endbuf = word + length;
  const PRUnichar *startWord=p;
  if(p<endbuf){
    // XXX These loops should be modified to handle non-BMP characters.
    // if previous character is a word character, need to advance out of the word
    if (offset > 0 && ucIsAlpha(*(p-1))) {
      while (p < endbuf && ucIsAlpha(*p))
        p++;
    }
    while((p < endbuf) && (!ucIsAlpha(*p)))
      {
        p++;
      }
    startWord=p;
    while((p < endbuf) && ((ucIsAlpha(*p))||(*p=='\'')))
      { 
        p++;
      }
    
    // we could be trying to break down a url, we don't want to break a url into parts,
    // instead we want to find out if it really is a url and if so, skip it, advancing startWord 
    // to a point after the url.

    // before we spend more time looking to see if the word is a url, look for a url identifer
    // and make sure that identifer isn't the last character in the word fragment.
    if ( (*p == ':' || *p == '@' || *p == '.') &&  p < endbuf - 1) {

        // ok, we have a possible url...do more research to find out if we really have one
        // and determine the length of the url so we can skip over it.
       
        if (mURLDetector)
        {
          PRInt32 startPos = -1;
          PRInt32 endPos = -1;        

          mURLDetector->FindURLInPlaintext(startWord, endbuf - startWord, p - startWord, &startPos, &endPos);

          // ok, if we got a url, adjust the array bounds, skip the current url text and find the next word again
          if (startPos != -1 && endPos != -1) { 
            startWord = p + endPos + 1; // skip over the url
            p = startWord; // reset p

            // now recursively call FindNextWord to search for the next word now that we have skipped the url
            return FindNextWord(word, length, startWord - word, begin, end);
          }
        }
    }

    while((p > startWord)&&(*(p-1) == '\'')){  // trim trailing apostrophes
      p--;
    }
  }
  else{
    startWord = endbuf;
  }
  if(startWord == endbuf){
    *begin = -1;
    *end = -1;
  }
  else{
    *begin = startWord-word;
    *end = p-word;
  }
  return NS_OK;
}

mozEnglishWordUtils::myspCapitalization 
mozEnglishWordUtils::captype(const nsString &word)
{
  if(!mCaseConv) return HuhCap; //punt
  PRUnichar* lword=ToNewUnicode(word);  
  mCaseConv->ToUpper(lword,lword,word.Length());
  if(word.Equals(lword)){
    nsMemory::Free(lword);
    return AllCap;
  }

  mCaseConv->ToLower(lword,lword,word.Length());
  if(word.Equals(lword)){
    nsMemory::Free(lword);
    return NoCap;
  }
  PRInt32 length=word.Length();
  if(Substring(word,1,length-1).Equals(lword+1)){
    nsMemory::Free(lword);
    return InitCap;
  }
  nsMemory::Free(lword);
  return HuhCap;
}

// Convert the list of words in iwords to the same capitalization aWord and 
// return them in owords.
NS_IMETHODIMP mozEnglishWordUtils::FromRootForm(const PRUnichar *aWord, const PRUnichar **iwords, PRUint32 icount, PRUnichar ***owords, PRUint32 *ocount)
{
  nsAutoString word(aWord);
  nsresult rv = NS_OK;

  PRInt32 length;
  PRUnichar **tmpPtr  = (PRUnichar **)nsMemory::Alloc(sizeof(PRUnichar *)*icount);
  if (!tmpPtr)
    return NS_ERROR_OUT_OF_MEMORY;

  mozEnglishWordUtils::myspCapitalization ct = captype(word);
  for(PRUint32 i = 0; i < icount; ++i) {
    length = nsCRT::strlen(iwords[i]);
    tmpPtr[i] = (PRUnichar *) nsMemory::Alloc(sizeof(PRUnichar) * (length + 1));
    if (NS_UNLIKELY(!tmpPtr[i])) {
      NS_FREE_XPCOM_ALLOCATED_POINTER_ARRAY(i, tmpPtr);
      return NS_ERROR_OUT_OF_MEMORY;
    }
    memcpy(tmpPtr[i], iwords[i], (length + 1) * sizeof(PRUnichar));

    nsAutoString capTest(tmpPtr[i]);
    mozEnglishWordUtils::myspCapitalization newCt=captype(capTest);
    if(newCt == NoCap){
      switch(ct) 
        {
        case HuhCap:
        case NoCap:
          break;
        case AllCap:
          rv = mCaseConv->ToUpper(tmpPtr[i],tmpPtr[i],length);
          break;
        case InitCap:  
          rv = mCaseConv->ToUpper(tmpPtr[i],tmpPtr[i],1);
          break;
        default:
          rv = NS_ERROR_FAILURE; // should never get here;
          break;

        }
    }
  }
  if (NS_SUCCEEDED(rv)){
    *owords = tmpPtr;
    *ocount = icount;
  }
  return rv;
}

