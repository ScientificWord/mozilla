#include "msiAutosub.h"
#include "nsCRT.h"
#include "nsCOMPtr.h"
#include "nsComponentManagerUtils.h"
#include "nsIDOMElement.h"
#include "nsIDOM3Node.h"
#include "nsIDocument.h"
#include "nsIDOMDocument.h"
#include "nsIDOMXMLDocument.h"
#include "nsIXMLHttpRequest.h"
#include "nsIDOMNodeList.h"
#include "nsIIOService.h"
#include "nsIFile.h"
#include "nsILocalFile.h"
#include "nsIFileURL.h"
#include "nsIFileStreams.h"
#include "nsIConverterOutputStream.h"
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

msiAutosub::msiAutosub(): isInitialized(PR_FALSE), disabledInContext(msiIAutosub::CONTEXT_NONE)
{
  /* member initializers and constructor code */
}

msiAutosub::~msiAutosub()
{
  /* destructor code */
}

void reverse( nsString& str )
{
  nsAutoString str2;
  str2.Assign(str);
  nsString::const_iterator start;
  str2.BeginReading(start);
  nsString::const_iterator end;
  str2.EndReading(end);
  nsString::iterator dest_end;
  str.EndWriting(dest_end);
  while (start != end) *(--dest_end) = *(start++);
}

static NS_DEFINE_CID( kXMLDocumentCID, NS_XMLDOCUMENT_CID );
static NS_DEFINE_CID( kXMLHttpRequestCID, NS_XMLHTTPREQUEST_CID );


/* void initialize (in string fileURI); */
NS_IMETHODIMP msiAutosub::Initialize(const nsAString & fileURI)
{
  nsresult rv;
//  PRBool b;
  nsCOMPtr<nsIDOMXMLDocument> docAutosubs;
  nsCOMPtr<nsIDOMDocument> domdocAutosubs;
  PRUint32 autosubCount = 0;
  PRUint32 ctx =  msiIAutosub::CONTEXT_MATHONLY;;
  PRUint32 action = msiIAutosub::ACTION_SUBSTITUTE;
  mFileURI = NS_ConvertUTF16toUTF8(fileURI);
  nsAutoString s;
  nsAutoString data;
  nsAutoString pastectx;
  nsAutoString pasteinfo;
  nsAutoString pattern;
  nsAutoString pattern2;
  nsCOMPtr<nsIDOMNodeList> subsTags;
  nsCOMPtr<nsIDOMNodeList> patternTags;
  nsCOMPtr<nsIDOMNodeList> dataTags, contextTags, infoTags;
  nsCOMPtr<nsIDOMNode> subsNode;
  nsCOMPtr<nsIDOMElement> subsElement;
  nsCOMPtr<nsIDOMElement> docElement;
  nsCOMPtr<nsIDOM3Node> textNode; 
  nsCOMPtr<nsIDOMNode> patternNode;
  nsCOMPtr<nsIDOMNode> dataNode, contextNode, infoNode;
  nsCOMPtr<nsIXMLHttpRequest> req;
  const nsCString GET=NS_LITERAL_CSTRING("GET");
  const nsCString xml=NS_LITERAL_CSTRING("text/xml");
  state = msiIAutosub::STATE_INIT;
  startIndex = 0;
  lastIndex = 0;
  arraylength = 0;
  // load the XML autosubs file 
//  docAutosubs = do_CreateInstance(kXMLDocumentCID, &rv);
//  if (rv) return rv;
  req = do_CreateInstance(kXMLHttpRequestCID, &rv);
  if (rv) return rv;
  rv = req->OpenRequest(GET, NS_ConvertUTF16toUTF8(fileURI), PR_FALSE, nsString(), nsString());
  if (rv) return rv;
  rv = req->OverrideMimeType(xml);
  if (rv) return rv;
  rv = req->Send(nsnull);
  if (rv) return rv;
  rv = req->GetResponseXML(getter_AddRefs(domdocAutosubs));
  if (rv) return rv;
  docAutosubs = do_QueryInterface(domdocAutosubs);
  rv = docAutosubs->GetElementsByTagName(NS_LITERAL_STRING("sub"), getter_AddRefs(subsTags));
  if (subsTags) subsTags->GetLength(&autosubCount);
  if (autosubCount > 0)
  {
    lastIndex = autosubCount - 1;
    autosubarray = new autosubentry[autosubCount];
    arraylength = autosubCount;
    docAutosubs->GetDocumentElement(getter_AddRefs(docElement));
    textNode = do_QueryInterface(docElement);
    textNode->GetTextContent(data);
//    printf("%S\n", data.get());

    for (PRUint32 i = 0; i < autosubCount; i++)
    {
      subsTags->Item(i, (nsIDOMNode **)getter_AddRefs(subsNode));
      if (subsNode)
      {
        subsElement =  do_QueryInterface(subsNode);
        subsElement->GetAttribute(NS_LITERAL_STRING("ctx"),s);
        if (s.EqualsLiteral("text")) ctx = msiIAutosub::CONTEXT_TEXTONLY; 
        else if (s.EqualsLiteral("both")) ctx = msiIAutosub::CONTEXT_MATHANDTEXT; 
        else if (s.EqualsLiteral("math")) ctx = msiIAutosub::CONTEXT_MATHONLY; 
        subsElement->GetAttribute(NS_LITERAL_STRING("tp"),s);
        if (s.EqualsLiteral("sc")) action = msiIAutosub::ACTION_EXECUTE;
		    else if (s.EqualsLiteral("subst")) action = msiIAutosub::ACTION_SUBSTITUTE;
        subsElement->GetElementsByTagName(NS_LITERAL_STRING("pattern"),getter_AddRefs(patternTags));
        if (patternTags)
        {
          patternTags->Item(0,(nsIDOMNode **)getter_AddRefs(patternNode));
          textNode = do_QueryInterface(patternNode);
          textNode->GetTextContent(pattern);
          // we need to reverse the characters in pattern
          reverse(pattern);
        }
        subsElement->GetElementsByTagName(NS_LITERAL_STRING("data"),getter_AddRefs(dataTags));
        if (dataTags)
        {
          dataTags->Item(0,(nsIDOMNode **)getter_AddRefs(dataNode));
          textNode = do_QueryInterface(dataNode);
          textNode->GetTextContent(data); 
        }
        if (action != msiIAutosub::ACTION_EXECUTE)
        { 
          subsElement->GetElementsByTagName(NS_LITERAL_STRING("context"),getter_AddRefs(contextTags));
          if (contextTags)
          {
            contextTags->Item(0,(nsIDOMNode **)getter_AddRefs(contextNode));
            textNode = do_QueryInterface(contextNode);
            textNode->GetTextContent(pastectx); 
          }
          subsElement->GetElementsByTagName(NS_LITERAL_STRING("info"),getter_AddRefs(infoTags));
          if (infoTags)
          {
            infoTags->Item(0,(nsIDOMNode **)getter_AddRefs(infoNode));
            textNode = do_QueryInterface(infoNode);
            textNode->GetTextContent(pasteinfo); 
          }
        }       
        autosubarray[i] = autosubentry(pattern, ctx, action, data, pastectx, pasteinfo);
      } 
    }
    NS_QuickSort(autosubarray, arraylength, sizeof(autosubentry), compare, nsnull);
    isInitialized = PR_TRUE;
    autosubentry entry;
//     for (PRUint32 i = 0; i < autosubCount; i++)
//     {
//       entry = autosubarray[i];
//       printf("%d: %d %S %S\n", i, entry.context, entry.pattern.get(), entry.data.get());
//     } 
  }
  return NS_OK;
}

/* boolean addEntry (in string pattern, in long ctx, in long action, in string data); */
NS_IMETHODIMP msiAutosub::AddEntry(const nsAString & pattern, PRInt32 ctx, PRInt32 action, const nsAString & data, 
  const nsAString & pasteContext, const nsAString & pasteInfo, PRBool *_retval)
{
    autosubentry * newArray = new autosubentry[arraylength+1];
    nsAutoString p(pattern);
    nsAutoString rp = p;
    reverse(rp);                     
    nsAutoString d(data);
    nsAutoString pc(pasteContext);
    nsAutoString pi(pasteInfo);
    PRUint32 i;  
    autosubentry newentry(rp, ctx, action, d, pc, pi);
    for (i = 0; i<arraylength; i++)
    {
      if (autosubarray[i] == rp) 
      {
        autosubarray[i] = newentry;
        *_retval = PR_TRUE;
        return NS_OK;
      }
    }
    for (i = 0; i<arraylength; i++)
      newArray[i] = autosubarray[i];
    newArray[arraylength++] = newentry;
    autosubentry * oldArray = autosubarray;
    autosubarray = newArray;
    delete[] oldArray;
    NS_QuickSort(autosubarray, arraylength, sizeof(autosubentry), compare, nsnull);
    *_retval = PR_TRUE;
    return NS_OK;
}

/* boolean removeEntry (in string pattern); */
NS_IMETHODIMP msiAutosub::RemoveEntry(const nsAString & pattern, PRBool *_retval)
{
    nsAutoString p(pattern);
    PRUint32 i, j, numDelete;
    reverse(p);
    j = 0;
    numDelete = 0;
    for (i = 0; i<arraylength; i++)
    {
      if (autosubarray[i] == p)
        ++numDelete;
    }
    autosubentry * newArray = new autosubentry[arraylength - numDelete];
    j = 0;
    for (i = 0; i < arraylength; ++i)
    {
      if ( !(autosubarray[i] == p) )
        newArray[j++] = autosubarray[i];
    }
    arraylength -= numDelete;
    autosubentry * oldArray = autosubarray;
    autosubarray = newArray;
    delete[] oldArray;
    NS_QuickSort(autosubarray, arraylength, sizeof(autosubentry), compare, nsnull);
    if (numDelete > 0)
      *_retval = PR_TRUE;
    return NS_OK;
//    return NS_ERROR_NOT_IMPLEMENTED;
}

/* string getCurrentData (out long ctx, out long action); */
NS_IMETHODIMP msiAutosub::GetCurrentData(PRInt32 *ctx, PRInt32 *action, nsAString & pasteContext, nsAString & pasteInfo, nsAString & _retval)
{
    autosubentry se = autosubarray[lastSuccessIndex];
    *ctx = se.context;
    *action = se.action;
    pasteContext = se.pastecontext;
    pasteInfo = se.pasteinfo;
    _retval = se.data;
    return NS_OK;
}

PRBool CtxMatches(PRInt32 ctx, PRBool inMath)
{
  if (ctx == msiIAutosub::CONTEXT_NONE)
    return PR_FALSE;
  if (inMath) return (ctx != msiIAutosub::CONTEXT_TEXTONLY);
  else return (ctx != msiIAutosub::CONTEXT_MATHONLY);
}


NS_IMETHODIMP msiAutosub::EnableAutoSubstitution( PRBool bEnable, PRInt32 ctx )
{
    if (bEnable)
      disabledInContext &= ~ctx;
    else
      disabledInContext |= ctx;
    return NS_OK;
}

NS_IMETHODIMP msiAutosub::IsAutoSubstitutionEnabled( PRBool bMath, PRBool * _retval)
{
    if (bMath)
      *_retval = ((disabledInContext & msiIAutosub::CONTEXT_MATHONLY) == 0);
    else
      *_retval = ((disabledInContext & msiIAutosub::CONTEXT_TEXTONLY) == 0);
    return NS_OK;
}

/* long nextChar (in wchar ch); */
NS_IMETHODIMP msiAutosub::NextChar(PRBool inMath, PRUnichar ch, PRInt32 *_retval)
{
  if (!isInitialized)
  {
    *_retval = msiIAutosub::STATE_FAIL;
    return NS_OK;
  }
  PRBool isEnabled;
  IsAutoSubstitutionEnabled( inMath, &isEnabled);
  if (!isEnabled)
  {
    *_retval = msiIAutosub::STATE_FAIL;
    return NS_OK;
  }

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
    if ((autosubarray[startIndex] == (patternSoFar)))
    {
      // we might have hit something that starts with patternSoFar. We need to back up to see if there
      // is a shorter string that starts with patternSoFar.
      while (startIndex > 0 && autosubarray[startIndex-1] == (patternSoFar)) startIndex--;
      if (CtxMatches(autosubarray[startIndex].context, inMath) && nsCRT::strncmp(autosubarray[startIndex].pattern.BeginReading(), patternSoFar.BeginReading(), autosubarray[startIndex].pattern.Length())==0) 
      {
        lastSuccessIndex = startIndex;
        *_retval = msiIAutosub::STATE_SUCCESS;
      }
      else
        *_retval = msiIAutosub::STATE_MATCHESSOFAR;
      return NS_OK;
    }
    if (autosubarray[lastIndex] == (patternSoFar))
    {
      // we might have hit something that starts with patternSoFar again
      while (lastIndex > 0 && autosubarray[lastIndex-1] == (patternSoFar)) lastIndex--;
      if ((nsCRT::strncmp(autosubarray[lastIndex].pattern.BeginReading(), patternSoFar.BeginReading(), autosubarray[lastIndex].pattern.Length())==0)
        && CtxMatches(autosubarray[lastIndex].context, inMath))
      {
        lastSuccessIndex = lastIndex;
        while (lastIndex < arraylength-1 && autosubarray[lastIndex+1] == (patternSoFar)) lastIndex++;
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
    if (autosubarray[m] < patternSoFar) startIndex = m;
    else lastIndex = m;
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
    lastSuccessIndex = 0;
    return NS_OK;
}

/* boolean Save (); */
NS_IMETHODIMP msiAutosub::Save(PRBool *_retval)
{
  nsString sFile = NS_LITERAL_STRING("<?xml version=\"1.0\"?>\n") +
    NS_LITERAL_STRING("<!DOCTYPE subs PUBLIC \"-//W3C//DTD XHTML 1.1 plus MathML 2.0//EN\" \"http://www.w3.org/Math/DTD/mathml2/xhtml-math11-f.dtd\">\n") +
    NS_LITERAL_STRING("<subs>\n");
  nsAutoString line;
  nsAutoString temp;
  nsAutoString pattern;
  nsCString scFile;
  autosubentry* ase;
  PRUint32 i;
  for (i = 0; i < arraylength; i++)
  {
    ase = &(autosubarray[i]);
    line = NS_LITERAL_STRING("  <sub ctx=\"");
    temp.Truncate(0); 
    switch (ase->context) {
      case msiIAutosub::CONTEXT_MATHONLY:    temp = NS_LITERAL_STRING("math");
        break;
      case msiIAutosub::CONTEXT_TEXTONLY:    temp = NS_LITERAL_STRING("text");
        break;
      case msiIAutosub::CONTEXT_MATHANDTEXT: temp = NS_LITERAL_STRING("both");
        break;
      default: break;
    }
    line += temp;
    temp = NS_LITERAL_STRING("\" tp=\"") + (ase->action == msiIAutosub::ACTION_SUBSTITUTE ? NS_LITERAL_STRING("subst") : NS_LITERAL_STRING("sc"));
    line += temp;
    pattern = ase->pattern;
    reverse(pattern); 
    temp = NS_LITERAL_STRING("\"><pattern><![CDATA[") + pattern + NS_LITERAL_STRING("]]></pattern><data><![CDATA[") + ase->data + NS_LITERAL_STRING("]]></data>");
    line += temp;
    if (ase->action == msiIAutosub::ACTION_SUBSTITUTE)
    {
      temp = NS_LITERAL_STRING("<context");
      if (ase->pastecontext.IsEmpty())
        temp += NS_LITERAL_STRING("/>");
      else
        temp += NS_LITERAL_STRING("><![CDATA[") + ase->pastecontext +  NS_LITERAL_STRING("]]></context>");
      temp += NS_LITERAL_STRING("<info>") + ase->pasteinfo +  NS_LITERAL_STRING("</info>");
      line += temp;
    }
    temp = NS_LITERAL_STRING("</sub>\n");
    line += temp;
    sFile += line;
  }
  sFile += NS_LITERAL_STRING("</subs>\n");
  // now write sFile out to a file
  nsresult rv;
  nsCOMPtr<nsIIOService> serv(do_GetService("@mozilla.org/network/io-service;1", &rv));
  if (NS_FAILED(rv))
    return rv;
  nsCOMPtr<nsIURI> uri;
  rv = serv->NewURI(mFileURI, nsnull, nsnull, getter_AddRefs(uri));
  if (NS_FAILED(rv))
    return rv;
  nsCOMPtr<nsIFileURL> fileURL = do_QueryInterface(uri);
  nsCOMPtr<nsIFile> file1;
  nsCOMPtr<nsIFile> file;
  fileURL->GetFile(getter_AddRefs(file1));
  if (NS_FAILED(rv))
    return rv;
  rv = file1->Clone(getter_AddRefs(file));
  if (NS_FAILED(rv))
    return rv;
  PRBool fExists;
  rv = file->Exists(&fExists);
  if (NS_FAILED(rv))
    return rv;
  if (fExists) rv = file->Remove(PR_FALSE);
  if (NS_FAILED(rv))
    return rv;
  rv = file->Create(0, 0755);
  nsCOMPtr<nsIFileOutputStream> fos = do_CreateInstance("@mozilla.org/network/file-output-stream;1", &rv);
  if (NS_FAILED(rv))
    return rv;
  rv = fos->Init(file, -1, -1, PR_FALSE);
  if (NS_FAILED(rv))
    return rv;
  nsCOMPtr<nsIConverterOutputStream> os = do_CreateInstance("@mozilla.org/intl/converter-output-stream;1", &rv);
  if (NS_FAILED(rv))
    return rv;
  rv = os->Init(fos, "UTF-8", 4096, '?');
  if (NS_FAILED(rv))
    return rv;
  PRBool fSuccess;
  rv = os->WriteString(sFile, &fSuccess);
  if (NS_FAILED(rv))
    return rv;
  rv = os->Close();
  *_retval = PR_TRUE;  
  return rv;    
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
