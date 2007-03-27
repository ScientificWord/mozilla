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

// IMPLEMENTATION OF nsAutoCompleteSearchStringArray
NS_IMPL_QUERY_INTERFACE2(nsAutoCompleteSearchStringArray, nsIAutoCompleteSearchStringArray, nsIAutoCompleteSearch)
NS_IMPL_ADDREF(nsAutoCompleteSearchStringArray)
NS_IMPL_RELEASE(nsAutoCompleteSearchStringArray)

nsAutoCompleteSearchStringArray * nsAutoCompleteSearchStringArray::sAutoCompleteSearchStringArray;

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

nsStringArray * nsAutoCompleteSearchStringArrayImp::GetStringArrayForCategory( const nsAString & strCategory)
{
  stringStringArray * pssa = m_stringArrays;
  nsStringArray * psa = nsnull;
  nsAString::const_iterator start, end;
  nsAString::const_iterator startsave, endsave;
  PRUint32 i;  
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
          if (psa == nsnull) psa = pssa->strArray;
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
  stringStringArray * pssa = m_stringArrays;
  nsString str;
  nsStringArray * psa = GetStringArrayForCategory(strCategory);
  if (!psa) // string category was not found, add a new one
  {
    psa = new nsStringArray;
    pssa = new stringStringArray;
    pssa->strCategory = strCategory;
    pssa->strArray = psa;
    pssa->next = m_stringArrays;
    m_stringArrays = pssa;
  }
  if (!psa) // unable to create new string array
  { 
    *_retval = PR_FALSE;
    printf("out of memory in nsAutoCompleteSearchStringArrayImp::AddString\n");
    return NS_OK;
  }
  if (psa && (psa->IndexOf(strAdd)==-1))  // should be true, unless there was a error creating a new one.
  {
    psa->AppendString(strAdd); 
    // Now read it back out to make sure
//    printf(" added (%S)[%d] = %S\n", strCategory.BeginReading(), psa->IndexOf(strAdd), psa->StringAt(psa->IndexOf(strAdd))->BeginReading());
  }
  else
  {
    *_retval = PR_FALSE;
  }
  return NS_OK;
}


/* boolean deleteString (in AString strCategory, in AString strDelete); */
NS_IMETHODIMP nsAutoCompleteSearchStringArrayImp::DeleteString(const nsAString & strCategory, const nsAString & strDelete, PRBool *_retval)
{
  nsStringArray * psa = GetStringArrayForCategory(strCategory);
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
    if (pssa->strArray) pssa->strArray->Sort();
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



/* void startSearch (in AString searchString, in AString searchParam, in nsIAutoCompleteResult previousResult, in nsIAutoCompleteObserver listener); */
NS_IMETHODIMP nsAutoCompleteSearchStringArrayImp::StartSearch(
 const nsAString & searchString, const nsAString & searchParam, 
 nsIAutoCompleteResult *previousResult, 
 nsIAutoCompleteObserver *listener)
{
    // initially, ignore previousResult and searchParam
  nsAString::const_iterator start, end, originalStart;
  nsString str;
  nsString str2;
  nsStringArray * psa = GetStringArrayForCategory(searchParam);
  if (!psa) return  PR_INVALID_ARGUMENT_ERROR;
  nsCOMPtr<nsAutoCompleteResultStringArray> mResult = new nsAutoCompleteResultStringArray(m_markedStrings);
  if (!mResult) return NS_ERROR_FAILURE;
  mResult->SetSearchString(searchString);
  mResult->SetSearchResult(nsIAutoCompleteResult::RESULT_NOMATCH);

  for (int i = 0; i < psa->Count(); i++) {
    psa->StringAt(i, str);
    if (searchString.IsEmpty()) {   // everything matches the empty string
      mResult->AppendString(str); 
      mResult->SetSearchResult(nsIAutoCompleteResult::RESULT_SUCCESS);
    }
    else {      
      str.BeginReading(start);
      originalStart = start;
      str.EndReading(end);
      if (FindInReadable(searchString,  start, end, nsDefaultStringComparator())) {
        if (start==originalStart) { // pattern was found at the beginning of 
                                    // the string
          mResult->AppendString(str); 
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
  NS_INTERFACE_MAP_ENTRY(nsIAutoCompleteBaseResult)
  NS_INTERFACE_MAP_ENTRY(nsAutoCompleteResultStringArray)
  NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(nsISupports, nsIAutoCompleteResult)
NS_INTERFACE_MAP_END

NS_IMPL_ADDREF(nsAutoCompleteResultStringArray)
NS_IMPL_RELEASE(nsAutoCompleteResultStringArray)


nsAutoCompleteResultStringArray::nsAutoCompleteResultStringArray(nsStringArray * pmarkedStrings):pMarkedTags(pmarkedStrings)
{
  mReturnStrings = new nsStringArray(10);
  mDefaultIndex = 0;
}

nsAutoCompleteResultStringArray::~nsAutoCompleteResultStringArray()
{
  delete mReturnStrings;
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
    mReturnStrings->StringAt(index, _retval);
    return NS_OK;
}

/* AString getCommentAt (in long index); */
NS_IMETHODIMP nsAutoCompleteResultStringArray::GetCommentAt(PRInt32 index, nsAString & _retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* AString getStyleAt (in long index); */
NS_IMETHODIMP nsAutoCompleteResultStringArray::GetStyleAt(PRInt32 index, nsAString & _retval)
{
	nsAutoString strValue;
	nsString str;
  if (!pMarkedTags) return NS_OK;
	GetValueAt(index, strValue);
	if (!strValue.IsEmpty())
	{
	  for (int i = 0; i < pMarkedTags->Count(); i++)
	  {
	 	  pMarkedTags->StringAt(i, str);
	    if (strValue.Equals(str))
	 	  {
	       _retval = NS_LITERAL_STRING("sw_checked");
	       return NS_OK;
	 	  }
	  }
	}
	_retval = NS_LITERAL_STRING("");
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

NS_IMETHODIMP nsAutoCompleteResultStringArray::AppendString( nsString aString )
{
  mReturnStrings->AppendString(aString);
  return NS_OK;
}
  
NS_IMETHODIMP nsAutoCompleteResultStringArray::Clear()
{
  mReturnStrings->Clear();
  return NS_OK;
}

