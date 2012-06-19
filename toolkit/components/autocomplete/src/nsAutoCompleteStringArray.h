#include "nsIAutoCompleteSearch.h"
#include "nsIAutoCompleteSearchStringArray.h"
#include "nsIAutoCompleteResult.h"
#include "nsIAutoCompleteResultTypes.h"
#include "nsIAutoCompleteResultStringArray.h"
#include "nsCOMPtr.h"

class nsAutoCompleteSearchStringArrayImp;


class nsAutoCompleteSearchStringArray : public nsIAutoCompleteSearchStringArray
{                                
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIAUTOCOMPLETESEARCH
  NS_DECL_NSIAUTOCOMPLETESEARCHSTRINGARRAY
  
  nsAutoCompleteSearchStringArray();
  virtual ~nsAutoCompleteSearchStringArray();
  static nsAutoCompleteSearchStringArray* GetInstance();
  static
    nsAutoCompleteSearchStringArray* sAutoCompleteSearchStringArray;  
  static
    nsAutoCompleteSearchStringArray* sGlobalAutoCompleteSearchStringArray;  
  
protected:  
  nsCOMPtr<nsAutoCompleteSearchStringArrayImp> m_imp; 
private:
  /* additional members */
};


// We expect only a moderate number of string categories (e.g., texttags, paragraph tags) so we will keep a list
// of pairs of (nsString, nsStringArray *). If the number of categories increases, we could have a hash table of nsStringArray *.

struct stringStringArray
{
  nsString strCategory;
  nsStringArray * strArray;
  nsStringArray * strComments;
  nsStringArray * strMathOnly;
  stringStringArray * next;
  stringStringArray(): strArray(nsnull), next(nsnull){}
};

class nsAutoCompleteSearchStringArrayImp : public nsIAutoCompleteSearchStringArray
{                                
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIAUTOCOMPLETESEARCH
  NS_DECL_NSIAUTOCOMPLETESEARCHSTRINGARRAY
  
  nsAutoCompleteSearchStringArrayImp();
  virtual ~nsAutoCompleteSearchStringArrayImp();
protected:
  
  stringStringArray * m_stringArrays;
  nsStringArray * m_markedStrings;
  
protected:  
  nsStringArray * GetStringArrayForCategory( const nsAString & strCategory, PRBool doCopy);
  stringStringArray * GetPssaForCategory(const nsAString & strCategory);
  void SortStringStringArray( stringStringArray * pssa);
private:
  /* additional members */
};



/* Header file */
class nsAutoCompleteResultStringArray : public nsIAutoCompleteResultStringArray
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIAUTOCOMPLETERESULT
  NS_DECL_NSIAUTOCOMPLETEBASERESULT
  
  nsAutoCompleteResultStringArray(nsStringArray * pMarkedStrings);
  ~nsAutoCompleteResultStringArray();
  NS_IMETHOD AppendString ( const nsAString & strString, const nsAString & strComment, const nsAString & strMathOnly );
  NS_IMETHOD GetMathAt( PRUint32 index, nsAString & _retval);
  NS_IMETHOD Clear();
//  NS_IMETHOD SetMarkedTags( nsStringArray * parentTags );

private:
  nsStringArray * mReturnStrings;
  nsStringArray * mCommentStrings;
  nsStringArray * mMathStrings;
  nsString mSearchString;
  PRUint16 mSearchResult;
  PRUint32 mDefaultIndex;
  nsStringArray * pMarkedTags;

protected:
  /* additional members */
};
