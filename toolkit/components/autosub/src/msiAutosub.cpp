#include "msiAutosub.h"
#include "nsCRT.h"
#include "nsCOMPtr.h"
#include "nsComponentManagerUtils.h"
#include "nsIDOMElement.h"
#include "nsIDOM3Node.h"
#include "nsIDocument.h"
#include "nsIDOMDocument.h"
#include "nsIDOMXMLDocument.h"
#include "nsIDOMNodeList.h"
#include "nsContentCID.h"
#include "nsIGenericFactory.h"
#include "../../build/nsToolkitCompsCID.h"
#include "nsQuickSort.h"

/* Implementation file */
NS_IMPL_ISUPPORTS1(msiAutosub, msiIAutosub)


PRBool
autosubentry::operator< ( nsString & b )
{
  int result = nsCRT::strncmp( pattern.BeginReading(), b.BeginReading(), b.Length());
  return (result < 0);
} 
  

PRBool
autosubentry::operator< ( autosubentry& b )
{
  return *this < b.pattern;
} 



PRBool
autosubentry::operator== ( nsString & b )
{
  int result = nsCRT::strncmp( pattern.BeginReading(), b.BeginReading(), b.Length());
  return (result == 0);
} 
  

PRBool
autosubentry::operator== ( autosubentry& b )
{
  return *this == b.pattern;
} 


PRBool
autosubentry::operator> ( nsString & b )
{
  int result = nsCRT::strncmp( pattern.BeginReading(), b.BeginReading(), b.Length());
  return (result > 0);
} 
  

PRBool
autosubentry::operator> ( autosubentry& b )
{
  return *this == b.pattern;
} 

int 
compare( const void * a, const void * b, void * notused)
{
  if (((autosubentry *)a)->pattern < ((autosubentry *)b)->pattern) return -1;
  if (((autosubentry *)a)->pattern > ((autosubentry *)b)->pattern) return 1;
  return 0;
}

msiAutosub::msiAutosub()
{
  /* member initializers and constructor code */
}

msiAutosub::~msiAutosub()
{
  /* destructor code */
}

static NS_DEFINE_CID( kXMLDocumentCID, NS_XMLDOCUMENT_CID );

/* void initialize (in string fileURI); */
NS_IMETHODIMP msiAutosub::Initialize(const nsAString & fileURI)
{
  nsresult rv;
  PRBool b;
  nsCOMPtr<nsIDOMXMLDocument> docAutosubs;
  PRUint32 autosubCount = 0;
  PRUint32 ctx =  msiIAutosub::CONTEXT_MATHONLY;;
  PRUint32 action = msiIAutosub::ACTION_SUBSTITUTE;
  nsAutoString s;
  nsAutoString data;
  nsAutoString pattern;
  nsAutoString pattern2;
  nsCOMPtr<nsIDOMNodeList> subsTags;
  nsCOMPtr<nsIDOMNodeList> patternTags;
  nsCOMPtr<nsIDOMNodeList> dataTags;
  nsCOMPtr<nsIDOMNode> subsNode;
  nsCOMPtr<nsIDOMElement> subsElement;
  nsCOMPtr<nsIDOM3Node> textNode; 
  nsCOMPtr<nsIDOMNode> patternNode;
  nsCOMPtr<nsIDOMNode> dataNode;
  state = msiIAutosub::STATE_INIT;
  startIndex = 0;
  lastIndex = 0;
  // load the XML autosubs file 
  docAutosubs = do_CreateInstance(kXMLDocumentCID, &rv);
  if (rv) return rv;
  rv = docAutosubs->SetAsync(PR_FALSE);
  if (rv) return rv;
  rv = docAutosubs->Load( fileURI, &b);
  if (rv) return rv;
  rv = docAutosubs->GetElementsByTagName(NS_LITERAL_STRING("sub"), getter_AddRefs(subsTags));
  if (subsTags) subsTags->GetLength(&autosubCount);
  if (autosubCount > 0)
  {
    lastIndex = autosubCount - 1;
    autosubarray = new autosubentry[autosubCount];
    arraylength = autosubCount;
    for (PRUint32 i = 0; i < autosubCount; i++)
    {
      subsTags->Item(i, (nsIDOMNode **)getter_AddRefs(subsNode));
      if (subsNode)
      {
        subsElement =  do_QueryInterface(subsNode);
        subsElement->GetAttribute(NS_LITERAL_STRING("ctx"),s);
        if (s.EqualsLiteral("text")) ctx = msiIAutosub::CONTEXT_TEXTONLY; 
        if (s.EqualsLiteral("textmath")) ctx = msiIAutosub::CONTEXT_MATHANDTEXT; 
        subsElement->GetAttribute(NS_LITERAL_STRING("act"),s);
        if (s.EqualsLiteral("exec")) action = msiIAutosub::ACTION_EXECUTE;
        subsElement->GetElementsByTagName(NS_LITERAL_STRING("pattern"),getter_AddRefs(patternTags));
        if (patternTags)
        {
          patternTags->Item(0,(nsIDOMNode **)getter_AddRefs(patternNode));
          textNode = do_QueryInterface(patternNode);
          textNode->GetTextContent(pattern);
          // we need to reverse the characters in pattern
          pattern2 = pattern;
          
          nsString::const_iterator start;
          pattern2.BeginReading(start);
          nsString::const_iterator end;
          pattern2.EndReading(end);
          nsString::iterator dest_end;
          pattern.EndWriting(dest_end);
          while (start != end) *(--dest_end) = *(start++);
        }
        subsElement->GetElementsByTagName(NS_LITERAL_STRING("data"),getter_AddRefs(dataTags));
        if (dataTags)
        {
          dataTags->Item(0,(nsIDOMNode **)getter_AddRefs(dataNode));
          textNode = do_QueryInterface(dataNode);
          textNode->GetTextContent(data); 
        }        
        autosubarray[i] = autosubentry(pattern, ctx, action, data);
      } 
    }
    NS_QuickSort(autosubarray, arraylength, sizeof(autosubentry), compare, nsnull);
  }
  NS_OK;
}

/* boolean addEntry (in string pattern, in long ctt, in long action, in string data); */
NS_IMETHODIMP msiAutosub::AddEntry(const nsAString & pattern, PRInt32 ctt, PRInt32 action, const nsAString & data, PRBool *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* boolean removeEntry (in string pattern); */
NS_IMETHODIMP msiAutosub::RemoveEntry(const nsAString & pattern, PRBool *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* string getCurrentData (out long ctx, out long action); */
NS_IMETHODIMP msiAutosub::GetCurrentData(PRInt32 *ctx, PRInt32 *action, nsAString & _retval)
{
    autosubentry se = autosubarray[startIndex];
    *ctx = se.context;
    *action = se.action;
    _retval = se.data;
    return NS_OK;
}

/* long nextChar (in wchar ch); */
NS_IMETHODIMP msiAutosub::NextChar(PRUnichar ch, PRInt32 *_retval)
{
  nsAutoString s;
  s.SetLength(1);
  nsString::iterator start;
  s.BeginWriting(start);
  *start = ch;
  patternSoFar.Append(s);
  if ( lastIndex < 0 || autosubarray[startIndex] > patternSoFar || autosubarray[lastIndex ] <  patternSoFar ) 
  {
    *_retval = msiIAutosub::STATE_FAIL;
    return NS_OK;
  }
  while (lastIndex - startIndex > 0)
  {
    if (autosubarray[startIndex] == (patternSoFar))
    {
      // we might have hit something that starts with patternSoFar. We need to back up to see if there
      // is a shorter string that starts with patternSoFar.
      while (startIndex > 0 && autosubarray[startIndex-1] == (patternSoFar)) startIndex--;
      if (nsCRT::strncmp(autosubarray[startIndex].pattern.BeginReading(), patternSoFar.BeginReading(), autosubarray[startIndex].pattern.Length())==0) 
        *_retval = msiIAutosub::STATE_SUCCESS;
      else
        *_retval = msiIAutosub::STATE_MATCHESSOFAR;
      return NS_OK;
    }
    if (autosubarray[lastIndex] == (patternSoFar))
    {
      // we might have hit something that starts with patternSoFar again
      while (lastIndex > 0 && autosubarray[lastIndex-1] == (patternSoFar)) lastIndex--;
      if (nsCRT::strncmp(autosubarray[lastIndex].pattern.BeginReading(), patternSoFar.BeginReading(), autosubarray[lastIndex].pattern.Length())==0)
      {
        startIndex = lastIndex;
        *_retval = msiIAutosub::STATE_SUCCESS;
        return NS_OK;
      }
      while (lastIndex < arraylength-1 && autosubarray[lastIndex+1] == (patternSoFar)) lastIndex++;
      *_retval = msiIAutosub::STATE_MATCHESSOFAR;
      return NS_OK;
      
    }
	  if (lastIndex - startIndex == 1) 
    {
      *_retval = msiIAutosub::STATE_FAIL;
      return NS_OK;
    }
    int m = (startIndex + lastIndex)/2;
    autosubarray[m] < patternSoFar ? startIndex = m : lastIndex = m;
 }
  *_retval = msiIAutosub::STATE_FAIL;
  return NS_OK;
}

/* void Reset (); */
NS_IMETHODIMP msiAutosub::Reset()
{
    patternSoFar.Truncate(0);
    state = msiIAutosub::STATE_INIT;
    startIndex = 0;
    lastIndex = arraylength - 1;
    return NS_OK;
}

/* boolean Save (); */
NS_IMETHODIMP msiAutosub::Save(PRBool *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

msiAutosub* sMsiAutosub;

/* static */ msiAutosub*
msiAutosub::GetInstance()
{
  if (!sMsiAutosub) {
    sMsiAutosub = new msiAutosub();
    if (!sMsiAutosub)
      return nsnull;

    NS_ADDREF(sMsiAutosub);   // addref the global
  }

  NS_ADDREF(sMsiAutosub);   // addref the return result
  return sMsiAutosub;
}


NS_GENERIC_FACTORY_SINGLETON_CONSTRUCTOR(msiAutosub, msiAutosub::GetInstance)

static const nsModuleComponentInfo components[] =
{

  { "Autosubstitute",
    MSI_AUTOSUBSTITUTE_CID ,
    MSI_AUTOSUBSTITUTE_CONTRACTID,
    msiAutosubConstructor },
};


NS_IMPL_NSGETMODULE(tkAutosubModule, components)
