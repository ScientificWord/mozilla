#include "nsVoidArray.h"
#include "nsLiteralString.h"
#include "nsUnicharUtils.h"
#include "nsAutoCompleteStringArray.h"
#include "prerror.h"
#include "nsCOMPtr.h"
#include "nsComponentManagerUtils.h"

/* Implementation file */

NS_IMPL_ADDREF(nsAutoCompleteSearchStringArrayImp)
NS_IMPL_RELEASE(nsAutoCompleteSearchStringArrayImp)
NS_IMPL_QUERY_INTERFACE2(nsAutoCompleteSearchStringArrayImp, nsIAutoCompleteSearchStringArray, nsIAutoCompleteSearch)


// What's going on here?
// Autocomplete text boxes select an autocomplete class by providing a string suffix to determine the class as in
// "@mozilla.org/autocomplete/search;1?name=stringarray". Each text box also provides a string such as, in this case, 'texttag',
// 'paratag', etc.
//
// Our problem is that we need this for each document. Each editor has attached to it a tag manager, which has the data to
// provide a list of strings for each of the 'texttag', 'paratag', etc. specifiers. In order to make the original design work
// in our case, we'd have to come up with a string id for each document and then use strings like 'doc1:texttag', etc.
//
// Instead, we create a unique nsIAutoCompleteSearchStringArray implementation that forwards the implementation of its methods
// to an internal ponsAutoCompleteSearchStringArrayImp pointer to an implementation of an interface for a single document only. When
// the focus changes to another
// document, this internal pointer must be switched. Thus each tag manager implementation owns an autocompletestring implementation
// and we switch between these as the focus of the user changes.
//
// There is in addition a unique global one for things such as macro names, autosubstitute, etc.
//

// IMPLEMENTATION OF nsAutoCompleteSearchStringArray
NS_IMPL_QUERY_INTERFACE2(nsAutoCompleteSearchStringArray, nsIAutoCompleteSearchStringArray, nsIAutoCompleteSearch)
NS_IMPL_ADDREF(nsAutoCompleteSearchStringArray)
NS_IMPL_RELEASE(nsAutoCompleteSearchStringArray)

nsAutoCompleteSearchStringArray * nsAutoCompleteSearchStringArray::sAutoCompleteSearchStringArray;
nsAutoCompleteSearchStringArray * nsAutoCompleteSearchStringArray::sGlobalAutoCompleteSearchStringArray;

nsAutoCompleteSearchStringArray::nsAutoCompleteSearchStringArray()
{
}

nsAutoCompleteSearchStringArray::~nsAutoCompleteSearchStringArray()
{

}

/* static */ nsAutoCompleteSearchStringArray*
nsAutoCompleteSearchStringArray::GetInstance()
{
  if (!sAutoCompleteSearchStringArray) {
    sAutoCompleteSearchStringArray = new nsAutoCompleteSearchStringArray();
    if (!sAutoCompleteSearchStringArray)
      return nsnull;

    NS_ADDREF(sAutoCompleteSearchStringArray);   // addref the global
  }

  NS_ADDREF(sAutoCompleteSearchStringArray);   // addref the return result
  return sAutoCompleteSearchStringArray;
}

/* void startSearch (in AString searchString, in AString searchParam, in nsIAutoCompleteResult previousResult, in nsIAutoCompleteObserver listener); */
NS_IMETHODIMP nsAutoCompleteSearchStringArray::StartSearch(const nsAString & searchString, const nsAString & searchParam, nsIAutoCompleteResult *previousResult, nsIAutoCompleteObserver *listener)
{
    if (m_imp) return m_imp->StartSearch(searchString, searchParam, previousResult, listener);
    printf("nsAutoCompletSearchStringArray uninitialized\n");
    return NS_OK; // is there an NS_UNINITIALIZED ??
}

/* void stopSearch (); */
NS_IMETHODIMP nsAutoCompleteSearchStringArray::StopSearch()
{
  if (m_imp) return m_imp->StopSearch();
  printf("nsAutoCompletSearchStringArray uninitialized\n");
  return NS_OK; // is there an NS_UNINITIALIZED ??
}

/* boolean addString (in AString strCategory, in AString strAdd); */
NS_IMETHODIMP nsAutoCompleteSearchStringArray::AddString(const nsAString & strCategory, const nsAString & strAdd, PRBool *_retval)
{
  if (m_imp) return m_imp->AddString(strCategory, strAdd, _retval);
  printf("nsAutoCompletSearchStringArray uninitialized\n");
  return NS_OK; // is there an NS_UNINITIALIZED ??
}

/* boolean addStringEx(in AString strCategory, in AString strAdd, in AString strDescription, in AString strMathOnly); */
NS_IMETHODIMP nsAutoCompleteSearchStringArray::AddStringEx(const nsAString & strCategory, const nsAString & strAdd,
  const nsAString & strDescription, const nsAString & strMathOnly, PRBool *_retval)
{
  if (m_imp) return m_imp->AddStringEx(strCategory, strAdd, strDescription, strMathOnly, _retval);
  printf("nsAutoCompletSearchStringArray uninitialized\n");
  return NS_OK; // is there an NS_UNINITIALIZED ??
}

/* boolean deleteString (in AString strCategory, in AString strDelete); */
NS_IMETHODIMP nsAutoCompleteSearchStringArray::DeleteString(const nsAString & strCategory, const nsAString & strDelete, PRBool *_retval)
{
  if (m_imp) return m_imp->DeleteString(strCategory, strDelete, _retval);
  printf("nsAutoCompletSearchStringArray uninitialized\n");
  return NS_OK; // is there an NS_UNINITIALIZED ??
}

/* void sortArrays (); */
NS_IMETHODIMP nsAutoCompleteSearchStringArray::SortArrays()
{
  if (m_imp) return m_imp->SortArrays();
  printf("nsAutoCompletSearchStringArray uninitialized\n");
  return NS_OK; // is there an NS_UNINITIALIZED ??
}

/* [noscript] void setMarkedStrings (in nsNativeStringArray sa); */
NS_IMETHODIMP nsAutoCompleteSearchStringArray::SetMarkedStrings(nsStringArray * sa)
{
  if (m_imp) return m_imp->SetMarkedStrings(sa);
  printf("nsAutoCompletSearchStringArray uninitialized\n");
  return NS_OK; // is there an NS_UNINITIALIZED ??
}


/* void setImplementation (in nsISupports imp); */
NS_IMETHODIMP nsAutoCompleteSearchStringArray::SetImplementation(nsIAutoCompleteSearchStringArray *imp)
{
    m_imp = do_QueryInterface(imp);
    if (!m_imp) printf("nsAutoCompletSearchStringArray uninitialized\n");
    return NS_OK; // is there an NS_UNINITIALIZED ??
}


/* nsIAutoCompleteSearchStringArray getNewImplementation (); */
NS_IMETHODIMP nsAutoCompleteSearchStringArray::GetNewImplementation(nsIAutoCompleteSearchStringArray **_retval)
{
    nsCOMPtr<nsIAutoCompleteSearchStringArray> imp;
    imp = do_QueryInterface(new nsAutoCompleteSearchStringArrayImp);
    *_retval = imp;
    NS_ADDREF(*_retval);
   return NS_OK;
}
/* long sizeofArray (in AString strCategory);
   returns size of the array or -1 if not found. */
NS_IMETHODIMP nsAutoCompleteSearchStringArray::SizeofArray(const nsAString & strCategory, PRInt32 *_retval)
{
    if (m_imp) return m_imp->SizeofArray(strCategory, _retval);
    printf("nsAutoCompletSearchStringArray uninitialized\n");
    return NS_OK; // is there an NS_UNINITIALIZED ??
}

/* AString contentsofArray (in AString strCategory, in AString strSep);
   returns contents of the array concatenated with strSep as a separator */
NS_IMETHODIMP nsAutoCompleteSearchStringArray::ContentsofArray(const nsAString & strCategory, const nsAString & strSep, nsAString & _retval)
{
    if (m_imp) return m_imp->ContentsofArray(strCategory, strSep, _retval);
    printf("nsAutoCompletSearchStringArray uninitialized\n");
    return NS_OK; // is there an NS_UNINITIALIZED ??
}
/* AString contentsofArray (in AString strCategory, in AString strSep);
   returns contents of the array concatenated with strSep as a separator */
NS_IMETHODIMP nsAutoCompleteSearchStringArray::ViewArray(const nsAString & strCategory)
{
    if (m_imp) return m_imp->ViewArray(strCategory);
    printf("nsAutoCompletSearchStringArray uninitialized\n");
    return NS_OK; // is there an NS_UNINITIALIZED ??
}


/* void resetArray (in AString strCategory); */
NS_IMETHODIMP nsAutoCompleteSearchStringArray::ResetArray(const nsAString & strCategory)
{
    if (m_imp) return m_imp->ResetArray(strCategory);
    printf("nsAutoCompletSearchStringArray uninitialized\n");
    return NS_OK; // is there an NS_UNINITIALIZED ??
}

/* void resetAll (); */
NS_IMETHODIMP nsAutoCompleteSearchStringArray::ResetAll()
{
    if (m_imp) return m_imp->ResetAll();
    printf("nsAutoCompletSearchStringArray uninitialized\n");
    return NS_OK; // is there an NS_UNINITIALIZED ??
}

/* void sortArray (in AString strCategory); */
NS_IMETHODIMP nsAutoCompleteSearchStringArray::SortArray(const nsAString & strCategory)
{
    if (m_imp) return m_imp->SortArray(strCategory);
    printf("nsAutoCompletSearchStringArray uninitialized\n");
    return NS_OK; // is there an NS_UNINITIALIZED ??
}

/* nsIAutoCompleteSearchStringArray getGlobalSearchStringArray (); */
NS_IMETHODIMP nsAutoCompleteSearchStringArray::GetGlobalSearchStringArray(nsIAutoCompleteSearchStringArray **_retval)
{
    if (m_imp) return m_imp->GetGlobalSearchStringArray(_retval);
    printf("nsAutoCompletSearchStringArray uninitialized\n");
    return NS_OK; // is there an NS_UNINITIALIZED ??
}




// IMPLEMENTATION OF nsAutoCompleteSearchStringArrayImp

#define NS_ERROR_EDITOR_NO_SELECTION NS_ERROR_GENERATE_FAILURE(NS_ERROR_MODULE_EDITOR,1)

nsAutoCompleteSearchStringArrayImp::nsAutoCompleteSearchStringArrayImp()
{
  m_stringArrays = nsnull;
  m_markedStrings = nsnull;
}

nsAutoCompleteSearchStringArrayImp::~nsAutoCompleteSearchStringArrayImp()
{
  stringStringArray * pssa;
  while (m_stringArrays)
  {
     pssa = m_stringArrays;
     m_stringArrays = pssa->next;
     delete pssa->strArray;
     pssa->strArray = nsnull;
     delete pssa;
   }
}

stringStringArray * nsAutoCompleteSearchStringArrayImp::GetPssaForCategory(const nsAString & strCategory)
{
  stringStringArray * pssa = m_stringArrays;
  nsAString::const_iterator start, end;
  nsAString::const_iterator startsave, endsave;
  while (pssa)
  {
    strCategory.BeginReading(start);
    startsave = start;
    strCategory.EndReading(end);
    endsave = end;
    if (FindInReadable(pssa->strCategory, start, end))
    {
      // pssa->strCategory is in the space-separated list; check to see if it is at the beginning
      // or end of the list or bounded by a space.
      if (start == startsave || *(--start) == ' ')
      {
        if ((end == endsave) || (*end == ' '))
        {
          return pssa;
        }
      }
    }
    pssa = pssa->next;
  }
  return nsnull;
}

nsStringArray * nsAutoCompleteSearchStringArrayImp::GetStringArrayForCategory( const nsAString & strCategory, PRBool doCopy)
{
  nsStringArray * psa = nsnull;
  stringStringArray * pssa = m_stringArrays;
  nsAString::const_iterator start, end;
  nsAString::const_iterator startsave, endsave;
  PRInt32 i;
  while (pssa)
  {
    strCategory.BeginReading(start);
    startsave = start;
    strCategory.EndReading(end);
    endsave = end;
    if (FindInReadable(pssa->strCategory, start, end))
    {
      // pssa->strCategory is in the space-separated list; check to see if it is at the beginning
      // or end of the list or bounded by a space.
      if (start == startsave || *(--start) == ' ')
      {
        if ((end == endsave) || (*end == ' '))
        {
          if (psa == nsnull)
          {
            if (doCopy)
            {
              psa = new nsStringArray;
		          for (i = 0; i < pssa->strArray->Count(); i++)
  			        psa->AppendString(*(pssa->strArray->StringAt(i)));
            }
            else psa = pssa->strArray;
          }
          else
          {
		        for (i = 0; i < pssa->strArray->Count(); i++)
  			      psa->AppendString(*(pssa->strArray->StringAt(i)));
          }
        }
      }
    }
    pssa = pssa->next;
  }
  return psa; // returns nsnull if no string array exists for the category.
}

/* boolean addString (in AString strCategory, in AString strAdd); */
NS_IMETHODIMP nsAutoCompleteSearchStringArrayImp::AddString(const nsAString & strCategory, const nsAString & strAdd, PRBool *_retval)
{
  nsAutoString zero;
  return AddStringEx(strCategory, strAdd, zero, zero, _retval);
}

/* boolean AddStringEx (in AString strCategory, in AString strAdd, in AString strComment, in AString strMath); */
NS_IMETHODIMP nsAutoCompleteSearchStringArrayImp::AddStringEx(const nsAString & strCategory, const nsAString & strAdd,
  const nsAString & strComment, const nsAString & strMath, PRBool *_retval)
{
  stringStringArray * pssa = m_stringArrays;
  nsString str;
  *_retval = PR_TRUE;
  nsStringArray * psa = GetStringArrayForCategory(strCategory, PR_FALSE);
  if (!psa) // string category was not found, add a new one
  {
    psa = new nsStringArray;
    pssa = new stringStringArray;
    pssa->strCategory = strCategory;
    pssa->strArray = psa;
    pssa->strComments = new nsStringArray;
    pssa->strMathOnly = new nsStringArray;
    pssa->next = m_stringArrays;
    m_stringArrays = pssa;
  }
  if (!psa) // unable to create new string array
  {
    *_retval = PR_FALSE;
    printf("out of memory in nsAutoCompleteSearchStringArrayImp::AddStringEx\n");
    return NS_OK;
  }
  if (psa && (psa->IndexOf(strAdd)==-1))  // should be true, unless there was a error creating a new one.
  {
    psa->AppendString(strAdd);
    if (strCategory.EqualsLiteral("texttags")) {
      pssa->strComments->AppendString(strComment);
      pssa->strMathOnly->AppendString(strMath);
    }
    // Now read it back out to make sure
    printf(" added (%S)[%d] = %S\n", strCategory.BeginReading(), psa->IndexOf(strAdd), psa->StringAt(psa->IndexOf(strAdd))->BeginReading());
  }
  else
  {
    *_retval = (psa!=nsnull);  // this is the case where IndexOf(strAdd) >= 0)
  }
  return NS_OK;
}


/* boolean deleteString (in AString strCategory, in AString strDelete); */
NS_IMETHODIMP nsAutoCompleteSearchStringArrayImp::DeleteString(const nsAString & strCategory, const nsAString & strDelete, PRBool *_retval)
{
  nsStringArray * psa = GetStringArrayForCategory(strCategory, PR_FALSE);
  if (psa)  // found string array for the category.
    *_retval = psa->RemoveString(strDelete);
  else *_retval = PR_FALSE;
  return NS_OK;
}


/* void sortArrays (); */
NS_IMETHODIMP nsAutoCompleteSearchStringArrayImp::SortArrays(void)
{
  // TODO: do a case-insensitive compare. Pass a comparator function to Sort.
  stringStringArray * pssa = m_stringArrays;

  while (pssa)
  {
     if (pssa->strCategory.EqualsLiteral("texttag"))
       SortStringStringArray(pssa);
     else pssa->strArray->Sort();
     pssa = pssa->next;
  }
  return NS_OK;
}

/* [noscript] void setMarkedStrings (in nsNativeStringArray sa); */
NS_IMETHODIMP nsAutoCompleteSearchStringArrayImp::SetMarkedStrings(nsStringArray * sa)
{
  nsAutoString str;
  nsresult res = NS_OK;
  if (m_markedStrings) m_markedStrings->Clear();
  else m_markedStrings = new nsStringArray[10];
  if (!m_markedStrings) return PR_OUT_OF_MEMORY_ERROR;
  for (int i = 0; i < sa->Count(); i++) {
    sa->StringAt(i, str);
    m_markedStrings->AppendString(str);
  }
  return res;
}


NS_IMETHODIMP nsAutoCompleteSearchStringArrayImp::SetImplementation(nsIAutoCompleteSearchStringArray *imp)
{
    // we delegate this one to the unique nsAutoCompleteSearchStringArray
    nsCOMPtr<nsAutoCompleteSearchStringArray> pACSSA = nsAutoCompleteSearchStringArray::GetInstance();
    return pACSSA->SetImplementation(imp);
}


/* nsIAutoCompleteSearchStringArray getNewImplementation (); */
NS_IMETHODIMP nsAutoCompleteSearchStringArrayImp::GetNewImplementation(nsIAutoCompleteSearchStringArray **_retval)
{
  printf("Error: calling GetNewImplementation in nsAutoCompleteSearchStringArrayImp\n");
  *_retval = nsnull;
  return NS_OK;
}

/* long sizeofArray (in AString strCategory);
   returns size of the array or -1 if not found. */
NS_IMETHODIMP nsAutoCompleteSearchStringArrayImp::SizeofArray(const nsAString & strCategory, PRInt32 *_retval)
{
  nsStringArray * psa = GetStringArrayForCategory(strCategory, PR_FALSE);
  *_retval = psa?psa->Count():-1;
  return NS_OK;
}


/* AString contentsofArray (in AString strCategory, in AString strSep);
   returns contents of the array concatenated with strSep as a separator */
NS_IMETHODIMP nsAutoCompleteSearchStringArrayImp::ContentsofArray(const nsAString & strCategory, const nsAString & strSep, nsAString & _retval)
{
  nsStringArray * psa = GetStringArrayForCategory(strCategory, PR_FALSE);
  PRInt32 i;
  _retval = NS_LITERAL_STRING("");
  PRInt32 length = psa?psa->Count():-1;
  if (length > 0) _retval = *(*psa)[0];
  for (i = 1; i < length; i++)
    _retval += strSep + *(*psa)[i];
  return NS_OK;
}

NS_IMETHODIMP
nsAutoCompleteSearchStringArrayImp::ViewArray(const nsAString & strCategory)
{
  nsStringArray * psa = GetStringArrayForCategory(strCategory, PR_FALSE);
  PRInt32 i;
  PRInt32 length = psa?psa->Count():-1;
  for (i = 0; i < length; i++) {
    *(*psa)[i];
  }
}


/* void resetArray (in AString strCategory); */
NS_IMETHODIMP nsAutoCompleteSearchStringArrayImp::ResetArray(const nsAString & strCategory)
{
  stringStringArray * pssa = m_stringArrays;
  stringStringArray * pPrev = nsnull;
  while (pssa)
  {
    if (pssa->strCategory.Equals(strCategory))
    {
      if (pPrev == nsnull) m_stringArrays = pssa->next;
      else pPrev->next = pssa->next;
      delete pssa->strArray;
      delete pssa->strComments;
      delete pssa->strMathOnly;
      delete pssa;
      return NS_OK;
    }
    pPrev = pssa;
    pssa = pssa->next;
  }
  return NS_OK;
}


/* void resetAll (); */
NS_IMETHODIMP nsAutoCompleteSearchStringArrayImp::ResetAll()
{
  stringStringArray * pssa = m_stringArrays;

  while (pssa)
  {
    if (pssa->strArray)
    {
      delete pssa->strArray;
      pssa->strArray = nsnull;
    }
    if (pssa->strComments)
    {
      delete pssa->strComments;
      pssa->strComments = nsnull;
    }
    if (pssa->strMathOnly)
    {
      delete pssa->strMathOnly;
      pssa->strMathOnly = nsnull;
    }
    pssa = pssa->next;
  }
  return NS_OK;
}


void nsAutoCompleteSearchStringArrayImp::SortStringStringArray(stringStringArray * pssa)
{
  PRUint32 i;
  PRUint32 count;
  PRUint32 length;
  PRUint32 trunclength;
  PRUnichar ch;
  nsString space((PRUnichar)' ');
  if (pssa->strArray) {
    count = pssa->strArray->Count();
    // the for loop puts space + the strMathOnly string which is '0' or '1'
    for (i = 0; i < count; i++) {
      length = (pssa->strArray)->StringAt(i)->Length();
      (pssa->strArray)->StringAt(i)->Insert(space,length);
      if (i < pssa->strMathOnly->Count() ){
         (pssa->strArray)->StringAt(i)->Insert((pssa->strMathOnly)->StringAt(i)->CharAt(0), length + 1);
      }
    }
    pssa->strArray->Sort();
    // Now unpack the above
    for (i = 0; i < count; i++) {
      // str = pssa->strArray[i];
      length = (pssa->strArray)->StringAt(i)->Length();
      ch = (pssa->strArray)->StringAt(i)->CharAt(length-1);
      if (i < pssa->strMathOnly->Count()){
         (pssa->strMathOnly)->StringAt(i)->SetCharAt(ch,0);
         trunclength = 2;
      }
      else trunclength = 1;
      (pssa->strArray)->StringAt(i)->Truncate(length - trunclength);
    }
  }
}

/* void sortArray (in AString strCategory); */
NS_IMETHODIMP nsAutoCompleteSearchStringArrayImp::SortArray(const nsAString & strCategory)
{
  stringStringArray * pssa = GetPssaForCategory(strCategory);
  if (strCategory.EqualsLiteral("texttag")) {
    // Since we added several other arrays, we have to do a hack to sort them
    // consistently. We ignore the description array for now.
    SortStringStringArray(pssa);
  }
  else pssa->strArray->Sort();
  return NS_OK;
}

/* nsIAutoCompleteSearchStringArray getGlobalSearchStringArray (); */
NS_IMETHODIMP nsAutoCompleteSearchStringArrayImp::GetGlobalSearchStringArray(nsIAutoCompleteSearchStringArray **_retval)
{
  if (!nsAutoCompleteSearchStringArray::sGlobalAutoCompleteSearchStringArray) {
    nsAutoCompleteSearchStringArray::sGlobalAutoCompleteSearchStringArray = new nsAutoCompleteSearchStringArray();
    if (!nsAutoCompleteSearchStringArray::sGlobalAutoCompleteSearchStringArray)
    {
      *_retval = nsnull;
      return NS_ERROR_FAILURE;
    }
    else
    {
      NS_ADDREF(nsAutoCompleteSearchStringArray::sGlobalAutoCompleteSearchStringArray);
      nsIAutoCompleteSearchStringArray * p_imp;
      nsAutoCompleteSearchStringArray::sGlobalAutoCompleteSearchStringArray->GetNewImplementation(&p_imp);
      nsAutoCompleteSearchStringArray::sGlobalAutoCompleteSearchStringArray->SetImplementation(p_imp);
    }
  }
  *_retval = nsAutoCompleteSearchStringArray::sGlobalAutoCompleteSearchStringArray;
  NS_ADDREF(*_retval);
  return NS_OK; // is there an NS_UNINITIALIZED ??
}


/* void startSearch (in AString searchString, in AString searchParam, in nsIAutoCompleteResult previousResult, in nsIAutoCompleteObserver listener); */
NS_IMETHODIMP nsAutoCompleteSearchStringArrayImp::StartSearch(
 const nsAString & searchString, const nsAString & searchParam,
 nsIAutoCompleteResult *previousResult,
 nsIAutoCompleteObserver *listener)
{
    // initially, ignore previousResult and searchParam
  nsAString::const_iterator start, end, originalStart;
  PRUint32 index;
  nsString str;
  nsString strcomment;
  nsString strmathonly;
  nsString emptystring;
  nsStringArray * psa = GetStringArrayForCategory(searchParam, PR_TRUE);
  if (!psa) return  PR_INVALID_ARGUMENT_ERROR;
  PRUint32 count = psa->Count();
  nsCOMPtr<nsAutoCompleteResultStringArray> mResult = new nsAutoCompleteResultStringArray(m_markedStrings);
  if (!mResult) return NS_ERROR_FAILURE;
  //stringStringArray * pssa = GetPssaForCategory(NS_LITERAL_STRING("texttag"));
  stringStringArray * pssa = GetPssaForCategory(searchParam);

  mResult->SetSearchString(searchString);
  mResult->SetSearchResult(nsIAutoCompleteResult::RESULT_NOMATCH);

  for (PRUint32 i = 0; i < count; i++) {
    psa->StringAt(i, str);
    if (searchString.IsEmpty()) {   // everything matches the empty string
      index = pssa->strArray->IndexOf(str);
      if (index >= 0) {
        pssa->strComments->StringAt(index,strcomment);
        pssa->strMathOnly->StringAt(index,strmathonly);
        mResult->AppendString(str, strcomment, strmathonly);
      }
      else
        mResult->AppendString(str, emptystring, emptystring);

      mResult->SetSearchResult(nsIAutoCompleteResult::RESULT_SUCCESS);
    }
    else {
      str.BeginReading(start);
      originalStart = start;
      str.EndReading(end);
      if (FindInReadable(searchString,  start, end, nsDefaultStringComparator())) {
        if (start==originalStart) { // pattern was found at the beginning of
                                    // the string
          index = pssa->strArray->IndexOf(str);
          if (index >= 0) {
            pssa->strComments->StringAt(index,strcomment);
            pssa->strMathOnly->StringAt(index,strmathonly);
            mResult->AppendString(str, strcomment, strmathonly);
          }
          else
            mResult->AppendString(str, emptystring, emptystring);

          mResult->SetSearchResult(nsIAutoCompleteResult::RESULT_SUCCESS);
        }
      }
    }
  }
  listener->OnSearchResult(nsAutoCompleteSearchStringArray::sAutoCompleteSearchStringArray, mResult);
  return NS_OK;
}

/* void stopSearch (); */
NS_IMETHODIMP nsAutoCompleteSearchStringArrayImp::StopSearch()
{
    return NS_OK;
}




NS_INTERFACE_MAP_BEGIN(nsAutoCompleteResultStringArray)
  NS_INTERFACE_MAP_ENTRY(nsIAutoCompleteResult)
//  NS_INTERFACE_MAP_ENTRY(nsIAutoCompleteBaseResult)
  NS_INTERFACE_MAP_ENTRY(nsAutoCompleteResultStringArray)
  NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(nsISupports, nsIAutoCompleteResult)
NS_INTERFACE_MAP_END

NS_IMPL_ADDREF(nsAutoCompleteResultStringArray)
NS_IMPL_RELEASE(nsAutoCompleteResultStringArray)


nsAutoCompleteResultStringArray::nsAutoCompleteResultStringArray(nsStringArray * pmarkedStrings):pMarkedTags(pmarkedStrings)
{
  mReturnStrings = new nsStringArray(50);
  mCommentStrings = new nsStringArray(50);
  mMathStrings = new nsStringArray(50);
  mDefaultIndex = 0;
}

nsAutoCompleteResultStringArray::~nsAutoCompleteResultStringArray()
{
  delete mReturnStrings;
  delete mCommentStrings;
  delete mMathStrings;
}

/* readonly attribute AString searchString; */
NS_IMETHODIMP nsAutoCompleteResultStringArray::GetSearchString(nsAString & aSearchString)
{
    aSearchString.Assign(mSearchString);
    return NS_OK;
}

/* readonly attribute unsigned short searchResult; */
NS_IMETHODIMP nsAutoCompleteResultStringArray::GetSearchResult(PRUint16 *aSearchResult)
{
    *aSearchResult = mSearchResult;
    return NS_OK;
}

/* readonly attribute long defaultIndex; */
NS_IMETHODIMP nsAutoCompleteResultStringArray::GetDefaultIndex(PRInt32 *aDefaultIndex)
{
   *aDefaultIndex = mDefaultIndex;
   return NS_OK;
}

/* readonly attribute AString errorDescription; */
NS_IMETHODIMP nsAutoCompleteResultStringArray::GetErrorDescription(nsAString & aErrorDescription)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute unsigned long matchCount; */
NS_IMETHODIMP nsAutoCompleteResultStringArray::GetMatchCount(PRUint32 *aMatchCount)
{
    *aMatchCount = mReturnStrings->Count();
    return NS_OK;
}

/* AString getValueAt (in long index); */
NS_IMETHODIMP nsAutoCompleteResultStringArray::GetValueAt(PRInt32 index, nsAString & _retval)
{
    if (mReturnStrings->Count() > index)
      mReturnStrings->StringAt(index, _retval);
    else _retval = NS_LITERAL_STRING("");
    return NS_OK;
}

/* AString getCommentAt (in long index); */
NS_IMETHODIMP nsAutoCompleteResultStringArray::GetCommentAt(PRInt32 index, nsAString & _retval)
{
  mCommentStrings->StringAt(index, _retval);
  return NS_OK;
}

/* AString getMathAt(in long index); */  // BBM this is wrong
NS_IMETHODIMP nsAutoCompleteResultStringArray::GetMathAt(PRUint32 index, nsAString & _retval)
{
  mMathStrings->StringAt(index, _retval);
  return NS_OK;
}

/* AString getStyleAt (in long index); */
NS_IMETHODIMP nsAutoCompleteResultStringArray::GetStyleAt(PRInt32 index, nsAString & _retval)
{
	nsAutoString strValue;
  nsAutoString strMathOnly;
	nsString str;
  nsString start;
  nsString endsWithSpace;
  GetMathAt(index, strMathOnly);
  if (strMathOnly.EqualsLiteral("1")) {
    strMathOnly = NS_LITERAL_STRING("sw_mathonly");
  }
  else
    strMathOnly = NS_LITERAL_STRING("");
  if (!pMarkedTags) return NS_OK;
	GetValueAt(index, strValue);
	if (!strValue.IsEmpty())
	{
	  for (int i = 0; i < pMarkedTags->Count(); i++)
	  {
	 	  pMarkedTags->StringAt(i, str);
      str.Left(start, strValue.Length()+1);
      endsWithSpace = strValue + NS_LITERAL_STRING(" ");
	    if (endsWithSpace.Equals(start) || strValue.Equals(start))
	 	  {
	       _retval = NS_LITERAL_STRING("sw_checked");
         if (strMathOnly.Length() > 0) {
           _retval = _retval + NS_LITERAL_STRING(" ");
           _retval = _retval + strMathOnly;
         }
	       return NS_OK;
	 	  }
	  }
	}
	_retval = strMathOnly;
  return NS_OK;
}


#ifdef DEBUG_BarryNO
nsString DebTagName;
void DebExamineNode(nsIDOMNode * aNode)
{
  if (aNode)
  {
    nsCOMPtr<nsIDOMElement> element = do_QueryInterface(aNode);
    element->GetTagName(DebTagName);
  }
  else DebTagName = NS_LITERAL_STRING("Null");
}
#endif


/* void removeValueAt (in long rowIndex, in boolean removeFromDb); */
NS_IMETHODIMP nsAutoCompleteResultStringArray::RemoveValueAt(PRInt32 rowIndex, PRBool removeFromDb)
{
  mReturnStrings->RemoveStringAt(rowIndex);
  mCommentStrings->RemoveStringAt(rowIndex);
  mMathStrings->RemoveStringAt(rowIndex);
    //  if (removeFromDb, remove the string from nsAutoCompleteSearchStringArrayImp
  return NS_OK;
}

/* void setSearchString (in AString searchString); */
NS_IMETHODIMP nsAutoCompleteResultStringArray::SetSearchString(const nsAString & searchString)
{
    mSearchString.Assign(searchString);
    return NS_OK;
}

/* void setErrorDescription (in AString errorDescription); */
NS_IMETHODIMP nsAutoCompleteResultStringArray::SetErrorDescription(const nsAString & errorDescription)
{
  return NS_OK;
}

/* void setDefaultIndex (in long defaultIndex); */
NS_IMETHODIMP nsAutoCompleteResultStringArray::SetDefaultIndex(PRInt32 defaultIndex)
{
  mDefaultIndex = defaultIndex;
  return NS_OK;
}

/* void setSearchResult (in unsigned long searchResult); */
NS_IMETHODIMP nsAutoCompleteResultStringArray::SetSearchResult(PRUint32 searchResult)
{
  mSearchResult = searchResult;
  return NS_OK;
}

NS_IMETHODIMP nsAutoCompleteResultStringArray::AppendString(const nsAString & aString, const nsAString & aComment, const nsAString & aMathOnly)
{
  mReturnStrings->AppendString(aString);
  mCommentStrings->AppendString(aComment);
  mMathStrings->AppendString(aMathOnly);
  return NS_OK;
}

NS_IMETHODIMP nsAutoCompleteResultStringArray::Clear()
{
  mReturnStrings->Clear();
  return NS_OK;
}

NS_IMETHODIMP
nsAutoCompleteResultStringArray::GetImageAt(PRInt32 aIndex, nsAString& _retval)
{
  return NS_OK;
}
