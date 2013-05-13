#include "msiTagListManager.h"
#include "nsClassHashtable.h"
#include "nsUnicharUtils.h"
#include "nsIDOMElement.h"
#include "nsIDOMNSHTMLElement.h"
#include "nsIDocument.h"
#include "nsIDOMDocument.h"
#include "nsIDOMXMLDocument.h"
#include "nsIXMLHttpRequest.h"
#include "nsIDOMNodeList.h"
#include "nsIDOMRange.h"
#include "nsIHTMLEditor.h"
#include "nsISelection.h"
#include "prerror.h"
#include "nsIContent.h"
#include "nsContentCID.h"
#include "nsComponentManagerUtils.h"
#include "nsISelectionPrivate.h"
#include "nsIRange.h"
#include "nsIDOMTreeWalker.h"
#include "nsIDOMNodeFilter.h"
#include "nsIDOMDocumentTraversal.h"
#include "nsIDOM3Node.h"
#include "../text/nsPlaintextEditor.h"
#include "../html/nsHTMLEditor.h"
#include "../base/nsEditorUtils.h"
#include "nsIAutoCompleteSearchStringArray.h"


/* Implementation file */
NS_INTERFACE_MAP_BEGIN(msiTagListManager)
  NS_INTERFACE_MAP_ENTRY(msiTagListManager)
  NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(nsISupports, msiITagListManager)
NS_INTERFACE_MAP_END

NS_IMPL_ADDREF(msiTagListManager)
NS_IMPL_RELEASE(msiTagListManager)

#if DEBUG_barry || DEBUG_Barry
void DebExamineNode(nsIDOMNode * aNode);
#endif 

nsIAtom * msiTagListManager::htmlnsAtom = nsnull;

static NS_DEFINE_CID(kAutoCompleteStringCID, NS_IAUTOCOMPLETESEARCHSTRINGARRAY_IID);
#define NS_STRINGARRAYAUTOCOMPLETE_CONTRACTID \
  "@mozilla.org/autocomplete/search;1?name=stringarray"

msiTagListManager::msiTagListManager()
:  meditor(nsnull), mparentTags(nsnull), mInitialized(PR_FALSE), plookup(nsnull), pContainsList(nsnull), pBabelList(nsnull),
    mdefaultParagraph(NS_LITERAL_STRING("")), mclearTextTag(NS_LITERAL_STRING("")),
    mclearStructTag(NS_LITERAL_STRING("")), mclearEnvTag(NS_LITERAL_STRING("")), mclearListTag(NS_LITERAL_STRING(""))
{
  nsresult rv;
//  printf("creating tag list manager\n");
  if (!htmlnsAtom) htmlnsAtom  = NS_NewAtom(NS_LITERAL_STRING("http://www.w3.org/1999/xhtml"));
  rv = CallCreateInstance(NS_STRINGARRAYAUTOCOMPLETE_CONTRACTID, nsnull, kAutoCompleteStringCID, (void **)&pACSSA);
//  printf("pACSSA is %x\n",(int)(void*)pACSSA);
// Now replace the singleton autocompletesearchstringarry by an implementation
  if (pACSSA) pACSSA->GetNewImplementation(getter_AddRefs(pACSSA));
//  printf("after'GetNewImplemantation'\n");
  Reset();
}

msiTagListManager * MSI_NewTagListManager()
{
  return nsnull;
  msiTagListManager * pmgr = new msiTagListManager;
  NS_ADDREF(pmgr);
  return pmgr;
}

struct namespaceLookup
{
  nsString namespaceAbbrev;
  nsCOMPtr<nsIAtom> nsAtom;
  namespaceLookup * next;
  namespaceLookup(): nsAtom(nsnull), next(nsnull) {}
};


msiTagListManager::~msiTagListManager()
{
  namespaceLookup * pns = plookup;
  while (pns)
  {
    plookup = pns->next;
    delete(pns);
    pns = plookup;
  }
  delete pContainsList;
	delete pBabelList;
}

// Miscellaneous destructors (constructors are inline
TagKeyList::~TagKeyList()
{
  if (pNext) delete pNext;
}

TagKeyListHead::~TagKeyListHead()
{
  if (pListHead) delete pListHead;
  if (pNext) delete pNext;  
}

NS_IMETHODIMP 
msiTagListManager::Enable() 
{
  // find the nsIAutoCompleteSearchStringArray object and set its current pointer to ours
  if (pACSSA) 
  {
    pACSSA->SetImplementation(pACSSA);
    return NS_OK;
  } 
  else return NS_ERROR_NOT_INITIALIZED;
}

// We convert the XML tree form of the tag info into a hash table form for more efficient lookup.
// Because of multiple name spaces, we can't assume that the tag names are unique without the namespace
// indicator. Thus our key is something like 'sw:sectiontitle' that combines the namespace and the tag name.
// First we define the key class.


TagKey::TagKey(nsString akey)
{
// the key string looks like "sw:bold". If aKey is in the hyphen form ("bold - sw"), we change it to the standard form
  int hyphen = akey.FindChar('-');
  if (hyphen>0 && (hyphen < (int)(akey.Length()) -1) && akey[hyphen-1] == ' ' 
    && akey[hyphen+1] == ' ')
  {
    nsString str;
    akey.Left(key,hyphen-1);
    akey.Right(str, (akey.Length() - hyphen +3));
#ifdef USE_NAMESPACES 
    if (str.Length() > 0) key = str +  NS_LITERAL_STRING(":") +key;
#endif                                
  } else 
  {
#ifndef USE_NAMESPACES 
    int colon = akey.FindChar(':');
    if (colon >= 0) akey.Right(key, (akey.Length() - (colon+1)));
    else    
#endif
    key = akey;
  }
}


nsString 
TagKey::prefix()
{
  int colon = key.FindChar(':');
  if (colon < 0) return NS_LITERAL_STRING("");
  nsString str;
  key.Left(str,colon);
  return str;
}

nsString
TagKey::localName()
{
  int colon = key.FindChar(':');
  nsString str;
  if (colon >= 0) 
  {
    key.Right(str,(key.Length())-(colon+1));
    return str;
  } 
  else return key;
}

nsString
TagKey::altForm()
{
  int colon = key.FindChar(':');
  if (colon < 1) return localName();
  else 
  {
    nsString str;
    key.Left(str,colon);
    return (str.Length()?localName()+ NS_LITERAL_STRING(" - ") + str:localName());
  }
}


/* nsIAtom NameSpaceAtomOfTagKey (in AString key); */
NS_IMETHODIMP msiTagListManager::NameSpaceAtomOfTagKey(const nsAString & key, nsIAtom **_retval)
{
  TagKey tagkey;
  tagkey.key.Assign(key);
  nsString str = tagkey.prefix();
  namespaceLookup * pns = plookup;
  if (!_retval) return NS_ERROR_INVALID_POINTER;
  *_retval = nsnull;
  if (str.Length() == 0) return NS_OK;
  while (pns && !(pns->namespaceAbbrev.Equals(str)))
    pns = pns->next;
  if (pns) *_retval = pns->nsAtom;
  return NS_OK;  
}
 
/* void reset (); */
NS_IMETHODIMP 
msiTagListManager::Reset(void)
{
  mparentTags = nsnull;
  msiTagHashtable.Init(30);
  mInitialized = PR_FALSE;
  mdocTagInfo = nsnull;
  namespaceLookup * pns = plookup;
  while (pns)
  {
    plookup = pns->next;
    delete(pns);
    pns = plookup;
  }
  return NS_OK;
}

static NS_DEFINE_CID( kXMLDocumentCID, NS_XMLDOCUMENT_CID );
static NS_DEFINE_CID( kXMLHttpRequestCID, NS_XMLHTTPREQUEST_CID );

NS_IMETHODIMP 
msiTagListManager::AddTagInfo(const nsAString & strTagInfoPath, PRBool *_retval) 
{
  nsresult rv;
  nsAutoString str;
  nsCOMPtr<nsIDOMXMLDocument> docTagInfo;
  nsCOMPtr<nsIDOMDocument> domdocTagInfo;
  
  // load the XML tag info file 
//***************************************
  *_retval = PR_FALSE;
  nsCOMPtr<nsIXMLHttpRequest> req;
  const nsCString GET=NS_LITERAL_CSTRING("GET");
  const nsCString xml=NS_LITERAL_CSTRING("text/xml");
  req = do_CreateInstance(kXMLHttpRequestCID, &rv);
  if (rv) return rv;
  rv = req->OpenRequest(GET, NS_ConvertUTF16toUTF8(strTagInfoPath), PR_FALSE, nsString(), nsString());
  if (rv) return rv;
  rv = req->OverrideMimeType(xml);
  if (rv) return rv;
  rv = req->Send(nsnull);
  if (rv) return rv;
  rv = req->GetResponseXML(getter_AddRefs(domdocTagInfo));
  if (rv) return rv;
  docTagInfo = do_QueryInterface(domdocTagInfo);
	mdocTagInfo = docTagInfo;
//***************************************
  BuildHashTables(docTagInfo, _retval);
  // get the default paragraph tag
  nsCOMPtr<nsIDOMElement> nodeElement;
  nsString strDefPara;
  nsString strClearTextTag;
  nsString strClearSectionTag;
  nsString strClearEnvTag;
	nsString strClearListTag;
  rv = docTagInfo->GetElementById(NS_LITERAL_STRING("defaultparagraph"), getter_AddRefs(nodeElement));
  if (rv == NS_OK && nodeElement)
    rv = nodeElement->GetAttribute(NS_LITERAL_STRING("nm"), strDefPara);
  if (rv==NS_OK) mdefaultParagraph.key.Assign(strDefPara);
  rv = docTagInfo->GetElementById(NS_LITERAL_STRING("cleartexttag"), getter_AddRefs(nodeElement));
  if (rv == NS_OK && nodeElement)
    rv = nodeElement->GetAttribute(NS_LITERAL_STRING("nm"), strClearTextTag);
  if (rv==NS_OK) mclearTextTag.key.Assign(strClearTextTag);
  rv = docTagInfo->GetElementById(NS_LITERAL_STRING("clearsectiontag"), getter_AddRefs(nodeElement));
  if (rv == NS_OK && nodeElement)
    rv = nodeElement->GetAttribute(NS_LITERAL_STRING("nm"), strClearSectionTag);
  if (rv==NS_OK) mclearStructTag.key.Assign(strClearSectionTag);
  rv = docTagInfo->GetElementById(NS_LITERAL_STRING("clearenvironmenttag"), getter_AddRefs(nodeElement));
  if (rv == NS_OK && nodeElement)
    rv = nodeElement->GetAttribute(NS_LITERAL_STRING("nm"), strClearEnvTag);
  if (rv==NS_OK) mclearEnvTag.key.Assign(strClearEnvTag);
  rv = docTagInfo->GetElementById(NS_LITERAL_STRING("clearlisttag"), getter_AddRefs(nodeElement));
  if (rv == NS_OK && nodeElement)
    rv = nodeElement->GetAttribute(NS_LITERAL_STRING("nm"), strClearListTag);
  if (rv==NS_OK) mclearListTag.key.Assign(strClearListTag);
  // build the name space list
  nsCOMPtr<nsIDOMNodeList> nodeList;
  nsString strNameSpace;
  nsString strAbbrev;
  PRUint32 nodeCount;
  nsCOMPtr<nsIDOM3Node> textNode; 
  nsCOMPtr<nsIDOMNode> node; 
  namespaceLookup * pns;
  rv = docTagInfo->GetElementsByTagName(NS_LITERAL_STRING("ns"), getter_AddRefs(nodeList));
  if (nodeList) nodeList->GetLength(&nodeCount);
  if (nodeCount > 0)
  {
    for (PRUint32 i = 0; i < nodeCount; i++)
    {
      nodeList->Item(i, getter_AddRefs(node));
      nodeElement = do_QueryInterface(node);
      rv = nodeElement->GetAttribute(NS_LITERAL_STRING("abbrev"),strAbbrev);
      // look to see if strAbbrev is already in the namespace list
      pns = plookup;
      while ((pns) && !(pns->namespaceAbbrev.Equals(strAbbrev)))
      {
        pns = pns->next; 
      }     
      if (pns == nsnull) // strAbbrev was not found
      {
        pns = new namespaceLookup;
        textNode=do_QueryInterface(node);
        textNode->GetTextContent(strNameSpace);
        // TODO: trim leading and following spaces
        
        pns->namespaceAbbrev = strAbbrev;
        pns->nsAtom = NS_NewAtom(strNameSpace);
        pns->next = plookup; 
        plookup = pns;      
      }
    }
  }
//  mInitialized = PR_TRUE;
  mInitialized = (pACSSA != nsnull);
  return rv;
}




nsString msiTagListManager::GetStringProperty( const nsAString & str, nsIDOMElement * element)
{
  nsCOMPtr<nsIDOMNodeList> nodeList;
  nsString strResult;
  PRUint32 nodeCount;
  nsCOMPtr<nsIDOM3Node> textNode; 
  nsCOMPtr<nsIDOMNode> node; 
 
  element->GetElementsByTagName(str, getter_AddRefs(nodeList));
  strResult = NS_LITERAL_STRING("");
  if (nodeList) nodeList->GetLength(&nodeCount);
  if (nodeCount > 0)
  {
    nodeList->Item(0, getter_AddRefs(node));
    textNode=do_QueryInterface(node);
    if (str.EqualsLiteral("mathonly"))
      return NS_LITERAL_STRING("1");
    textNode->GetTextContent(strResult);
  }
  else if (str.EqualsLiteral("mathonly"))
  {
    return NS_LITERAL_STRING("0");
  }
  return strResult;
}



nsIDOMElement * GetNodeProperty( const nsAString & str, nsIDOMElement * aElement)
{
  nsCOMPtr<nsIDOMNodeList> nodeList;
  PRUint32 nodeCount = 0;
  nsCOMPtr<nsIDOMNode> node; 
  nsCOMPtr<nsIDOMNode> child; 
  nsCOMPtr<nsIDOMNode> element; 
  nsCOMPtr<nsIDOMElement> elementResult = nsnull;
  PRUint16 nodeType;
  
 
  aElement->GetElementsByTagName(str, getter_AddRefs(nodeList));
  if (nodeList) nodeList->GetLength(&nodeCount);
  if (nodeCount > 0)
  {
    nodeList->Item(0, getter_AddRefs(node));
    element = do_QueryInterface(node);
    element->GetFirstChild(getter_AddRefs(child));
    if (child) child->GetNodeType(&nodeType);
    while (child && nodeType != nsIDOMNode::ELEMENT_NODE)
    {
      child->GetNextSibling(getter_AddRefs(child));
      if (child) child->GetNodeType(&nodeType);
    }
  }
  elementResult = do_QueryInterface(child);
  return elementResult;
}


// Build the contains list for an element

void
msiTagListManager::BuildContainsListForElement(nsIDOMElement * element, const nsAString & strName)
{
  nsresult rv;
  nsCOMPtr<nsIDOMNodeList> tagContains;
  PRUint32 tagContainsCount = 0;
  nsCOMPtr<nsIDOMElement> tagCanContainElement;   
  nsCOMPtr<nsIDOM3Node> tagCanContainNode3;
  nsCOMPtr<nsIDOMNode> tagCanContain;
  nsAutoString strCanContain;
  TagKeyListHead* pTagKeyListHead;
  TagKeyList* pTagKeyList;
  TagKey key;



  nsCOMPtr<nsIDOMNodeList> nodeList;
  PRUint32 nodeCount = 0; 
  nsCOMPtr<nsIDOMNode> node;
  nsCOMPtr<nsIDOMElement> classcontainsElement;
 
  element->GetElementsByTagName(NS_LITERAL_STRING("classcontains"), getter_AddRefs(nodeList));
  
  if (nodeList) 
    nodeList->GetLength(&nodeCount);
  
  if (nodeCount > 0)
  {
    nodeList->Item(0, getter_AddRefs(node));
    classcontainsElement = do_QueryInterface(node);
  } else {
	// it's not a class element; look for children of element
		classcontainsElement = element;
	}
  
  rv = classcontainsElement->GetElementsByTagName(NS_LITERAL_STRING("contains"), getter_AddRefs(tagContains));
  if (tagContains) 
  {
    rv = tagContains->GetLength(&tagContainsCount);
    for (PRUint32 j = 0; j < tagContainsCount; j++)
    {
      tagContains ->Item(j, getter_AddRefs(tagCanContain));
      tagCanContainNode3 = do_QueryInterface(tagCanContain);
      tagCanContainNode3->GetTextContent(strCanContain);
      // now we put strCanContain (a tag key) in the list for strName
      pTagKeyListHead = pContainsList;
      while (pTagKeyListHead && !(pTagKeyListHead->name.Equals(strName))) pTagKeyListHead = pTagKeyListHead->pNext;
      if (!pTagKeyListHead) // there is no list for the class name; make one
      {
        pTagKeyListHead = new TagKeyListHead;
        pTagKeyListHead->name.Assign(strName);
        pTagKeyListHead->pNext = pContainsList;  // we insert at the beginning of the list for simplicity
        pContainsList = pTagKeyListHead;
      } // pTagKeyListHead is not null
      // TODO: check for out of memory
      pTagKeyList = new TagKeyList;
      pTagKeyList->key.key.Assign(strCanContain);
#ifndef USE_NAMESPACES 
      pTagKeyList->key.key.Assign(pTagKeyList->key.localName());
#endif
      if (pTagKeyListHead->pListTail)
        pTagKeyListHead->pListTail->pNext = pTagKeyList;   // we use ListTail to keep the list in the same order as in the XNL file,
      pTagKeyListHead->pListTail = pTagKeyList;   // we use ListTail to keep the list in the same order as in the XNL file,
                                                  // for hand optimizations
      if (!(pTagKeyListHead->pListHead)) pTagKeyListHead->pListHead = pTagKeyListHead->pListTail;
    }
#define DEBUG_CONTAINMENT 1      
#ifdef DEBUG_CONTAINMENT
    // Write out the list
    if (tagContainsCount >0)
    {
      printf("\nContains list for tag %S:\n", pContainsList->name.BeginReading());
      pTagKeyList = pContainsList->pListHead;
      while (pTagKeyList)
      {
        printf("  can contain \"%S\n", ((nsString)pTagKeyList->key).BeginReading());
        pTagKeyList = pTagKeyList->pNext;
      }                                    
    }
#endif        
  }
}


void
msiTagListManager::BuildBabelList(nsIDOMXMLDocument * docTagInfo, TagKeyListHead ** ppBabelList)
{
	nsresult rv;
  nsCOMPtr<nsIDOMNodeList> tags;
	nsCOMPtr<nsIDOMNode> tag;
	nsCOMPtr<nsIDOMElement> tagElement;
  PRUint32 tagCount = 0;
	PRBool isBabel = PR_FALSE;
	*ppBabelList = new TagKeyListHead;
	TagKeyList * pTagKeyList;
	nsAutoString name;
	rv = docTagInfo->GetElementsByTagName(NS_LITERAL_STRING("tag"), getter_AddRefs(tags));
  if (!tags) return;
  tags->GetLength(&tagCount);
	for (PRUint32 i = 0; i < tagCount; i++)
	{
		rv = tags->Item(i, getter_AddRefs(tag));
    tagElement = do_QueryInterface(tag);
		rv = tagElement->HasAttribute(NS_LITERAL_STRING("babel"), &isBabel);
		if (isBabel)	
		{
			pTagKeyList = new TagKeyList;
			rv = tagElement->GetAttribute(NS_LITERAL_STRING("nm"), name);
      pTagKeyList->key.key.Assign(name);
			if ((*ppBabelList)->pListTail)
				(*ppBabelList)->pListTail->pNext = pTagKeyList;
			(*ppBabelList)->pListTail = pTagKeyList;
			if (!((*ppBabelList)->pListHead)) (*ppBabelList)->pListHead = (*ppBabelList)->pListTail;
		}
	}
}


// Build hash tables and also the list of lists that store the 'contains' info
/* bool BuildHashTables (in nsIDOMXMLDocument docTagInfo); */
PRBool
msiTagListManager::BuildHashTables(nsIDOMXMLDocument * docTagInfo, PRBool *_retval)
{
  nsresult rv;
  PRUint32 tagClassCount = 0;
  PRUint32 tagClassNameCount = 0;
  PRUint32 tagNameCount = 0;
  nsCOMPtr<nsIDOMNodeList> tagClasses;
  nsCOMPtr<nsIDOMNode> tagClass;
  nsCOMPtr<nsIDOMElement> tagClassElement;
  nsCOMPtr<nsIDOMNodeList> tagClassNames;
  nsCOMPtr<nsIDOMNode> tagClassName;
  nsCOMPtr<nsIDOMElement> tagClassNameElement;
  nsCOMPtr<nsIDOM3Node> tagClassNameNode;
  nsCOMPtr<nsIDOMNodeList> tagNames;
  nsCOMPtr<nsIDOMNode> tagName;
  nsIDOMXMLDocument * dti = docTagInfo;
  nsAutoString strName;
  nsAutoString strClassName;
  nsCOMPtr<nsIDOMElement> tagNameElement;   
  TagKey key;
  rv = dti->GetElementsByTagName(NS_LITERAL_STRING("tagclass"), getter_AddRefs(tagClasses));
  if (tagClasses) tagClasses->GetLength(&tagClassCount);
  if (tagClassCount > 0)
  {
    for (PRUint32 i = 0; i < tagClassCount; i++)
    {
      tagClasses->Item(i, (nsIDOMNode **) getter_AddRefs(tagClass));
      tagClassElement = do_QueryInterface(tagClass);
      rv = tagClassElement->GetElementsByTagName(NS_LITERAL_STRING("tagclassname"), getter_AddRefs(tagClassNames));
      // there should be only one <tagclassname> per <tagclass>. Thus we look only at the first in the list (and presumably the last)
      if (tagClassNames) tagClassNames->Item(0, getter_AddRefs(tagClassName));
      tagClassNameNode = do_QueryInterface(tagClassName);
      tagClassNameElement = do_QueryInterface(tagClassName);
      rv = tagClassNameNode->GetTextContent(strClassName);
      // strClassName is the name of the class. Now find the <contains> tags
      ///////
      BuildContainsListForElement(tagClassElement, strClassName);
      
      /////////
      rv = tagClassElement->GetElementsByTagName(NS_LITERAL_STRING("tag"), getter_AddRefs(tagNames));
      if (tagNames) 
      {
        rv = tagNames->GetLength(&tagNameCount);
        for (PRUint32 j = 0; j < tagNameCount; j++)
        {
          tagNames->Item(j, getter_AddRefs(tagName));
          tagNameElement = do_QueryInterface(tagName);
          rv = tagNameElement->GetAttribute(NS_LITERAL_STRING("nm"),strName);
          
          BuildContainsListForElement(tagNameElement, strName);
          key = TagKey(strName);
          TagData *pdata = new TagData; 
          tagNameElement->HasAttribute(NS_LITERAL_STRING("hidden"),&(pdata->hidden));              
          pdata->tagClass = strClassName;
          pdata->description = GetStringProperty(NS_LITERAL_STRING("description"), tagNameElement);
          pdata->mathonly = GetStringProperty(NS_LITERAL_STRING("mathonly"), tagNameElement);
          pdata->realTagClass = GetStringProperty(NS_LITERAL_STRING("realtagclass"), tagNameElement);
          pdata->initialContents =
            GetNodeProperty(NS_LITERAL_STRING("initialcontents"), tagNameElement);
          pdata->nextAfterEmptyBlock =
            GetStringProperty(NS_LITERAL_STRING("nextafteremptyblock"), tagNameElement);
          pdata->titleTag =
            GetStringProperty(NS_LITERAL_STRING("titletag"), tagNameElement);
          pdata->htmllist =
            GetStringProperty(NS_LITERAL_STRING("htmllist"), tagNameElement);
          pdata->htmllistparent =
            GetStringProperty(NS_LITERAL_STRING("htmllistparent"), tagNameElement);
					PRBool hasBabelAttribute;
					tagNameElement->HasAttribute(NS_LITERAL_STRING("babel"), &hasBabelAttribute);
					if (hasBabelAttribute)
					  pdata->babel = GetStringProperty(NS_LITERAL_STRING("babel"), tagNameElement);					
          pdata->level =
            GetStringProperty(NS_LITERAL_STRING("level"), tagNameElement);
          if (strClassName.EqualsLiteral("structtag"))
          {
            pdata->prefsub =
              GetStringProperty(NS_LITERAL_STRING("prefsub"), tagNameElement);
          }
          pdata->discardEmptyBlock = 
            GetStringProperty(NS_LITERAL_STRING("discardemptyblock"), tagNameElement)==NS_LITERAL_STRING("true");
          pdata->inclusion = 
            GetStringProperty(NS_LITERAL_STRING("inclusion"), tagNameElement)==NS_LITERAL_STRING("true");
          pdata->wrapper = 
            GetStringProperty(NS_LITERAL_STRING("wrapper"), tagNameElement)==NS_LITERAL_STRING("true");
          // save the key, data pair
          TagKey nextTagKey(GetStringProperty(NS_LITERAL_STRING("nexttag"), tagNameElement));
          pdata->nextTag = nextTagKey.key;
          if (pdata->nextTag.Length() == 0) pdata->nextTag = key.key;
          msiTagHashtable.Put(key.key, pdata);
//		      printf("Added %s: %s %s\n", ToNewCString(key.key), ToNewCString(pdata->description), ToNewCString(pdata->tagClass));
//          printf("hash table has %d entries\n",msiTagHashtable.Count()); 
        }
      }
    }
  }
  BuildStringArray(NS_LITERAL_STRING("texttag"));
  BuildStringArray(NS_LITERAL_STRING("paratag"));
  BuildStringArray(NS_LITERAL_STRING("listtag"));
  BuildStringArray(NS_LITERAL_STRING("structtag"));
  BuildStringArray(NS_LITERAL_STRING("envtag"));
  BuildStringArray(NS_LITERAL_STRING("frontmtag"));
  BuildStringArray(NS_LITERAL_STRING("othertag"));
  pACSSA->SortArrays();
	BuildBabelList(docTagInfo, &pBabelList);
  return PR_TRUE;
}


struct userArgStruct {
//  nsStringArray * stringArray;
  nsString tagClass;
  msiTagListManager * ptlm;
};

PLDHashOperator
nsDEnumRead(const nsAString_internal& aKey, TagData* aData, void* userArg) 
{  // this is a callback to the hash table enumerator that adds to a string array all the data that 
   // matches the ... part of *userArg
  TagKey tk;
  PRBool bres;
  tk.key = aKey;;
  userArgStruct * ua = (userArgStruct *) userArg;
//   printf("  enumerated %s = \"%s\"\n", aKey.get(), aData->tagClass.get());
  if (aData->tagClass == ua->tagClass && !(aData->hidden))
  {
    nsString name = tk.altForm();
    ua->ptlm->pACSSA->AddStringEx(ua->tagClass, name, aData->description, aData->mathonly, &bres);
  }
  return PL_DHASH_NEXT;
}


NS_IMETHODIMP
msiTagListManager::BuildStringArray(const nsAString & strTagClass)
// All the work is done in the enumerator (above)
{
  userArgStruct ua;
  ua.tagClass = strTagClass;
  ua.ptlm = this;
//  printf("%S:\n", strTagClass.BeginReading());
  msiTagHashtable.EnumerateRead(nsDEnumRead, &(ua));
  // the array gets sorted later by a call to SortArrays
  return NS_OK;
}

NS_IMETHODIMP 
msiTagListManager::BuildParentTagList()
{
  nsresult res;
  nsAutoString strTemp;
  nsAutoString strTemp2;
  nsAutoString strTempURI;
  nsAutoString strQName;
  nsAutoString strSetTags;
  nsAutoString strClearedTags;
  nsCOMPtr<nsIAtom> nsAtom; // namespace atom
  nsStringArray * markedTags = new nsStringArray();
  markedTags->Clear();
  if (!mparentTags) mparentTags = new nsStringArray();
  mparentTags->Clear();
 // BBM todo: put in error checking
  if (!meditor)
    return  NS_ERROR_FAILURE;;
  nsHTMLEditor * editor = static_cast<nsHTMLEditor*>(meditor);  
  nsCOMPtr<nsIDOMElement> element;
  nsCOMPtr<nsIDOMNode> node;
  nsCOMPtr<nsIDOMNode> temp;
  // get tags attached to the cursor
  editor->ReadCursorSetProps(strSetTags);
  editor->ReadCursorClearedProps(strClearedTags);
  PRInt32 index = 0;
  PRInt32 newIndex;
  PRInt32 length = strSetTags.Length();
  PRUnichar semi = ';';
  PRUnichar comma = ',';
  nsAutoString substring;
  // strSetTags looks like "tagname,attr,val;tagname2,attr,val;..."
  while (index < length) {
    newIndex = strSetTags.FindChar(comma,index);
    substring = Substring(strSetTags, index, newIndex - index);
    markedTags->AppendString(substring);
    index = newIndex + 1;
    newIndex = strSetTags.FindChar(semi, index);
    if (newIndex == -1) {
      break;
    }
    index = newIndex + 1;
  }
  nsAString::const_iterator start, end;

  strClearedTags.BeginReading(start);
  strClearedTags.EndReading(end);
  
  res = editor->GetSelectionContainer(getter_AddRefs(element));
  node = element;
  while (node)
  {
    node->GetLocalName(strTemp);
	  if (strTemp.Equals(NS_LITERAL_STRING("#document"))) break;
//    node->GetNamespaceURI(strTempURI);
//    if (strTempURI.Length() == 0) nsAtom = nsnull;
//    else nsAtom = NS_NewAtom(strTempURI);
//    if (nsAtom) 
//    {
//      strTemp2 = NS_LITERAL_STRING(" - ")+PrefixFromNameSpaceAtom(nsAtom);
//      strQName = strTemp + strTemp2;
//    }
//    else 
    strQName = strTemp; 
    strClearedTags.BeginReading(start);
    strClearedTags.EndReading(end);
    
    if (!FindInReadable(strQName, start, end) /*&& *(start-1)==';' */) {
      // end now points to the character after the pattern
  	  mparentTags->AppendString(strQName);
      markedTags->AppendString(strQName);
    }
    node->GetParentNode((nsIDOMNode **)(&temp));
    node = temp;
  }
  return pACSSA->SetMarkedStrings(markedTags);
}


NS_IMETHODIMP 
msiTagListManager::GetParentTagList(const nsAString & strSep, PRBool includeTypeInState, PRBool includeNS, nsAString & _retval)
{
// we ignore includeNS for now.
  BuildParentTagList();
  PRInt32 i;
  nsAutoString strSetTags;
  nsAutoString substring;
  _retval = NS_LITERAL_STRING("");
  PRInt32 length, newIndex;
  PRInt32 index = 0;
  PRUnichar semi = ';';
  PRUnichar comma = ',';
  if (includeTypeInState) {
    nsHTMLEditor * editor = static_cast<nsHTMLEditor*>(meditor);  
    editor->ReadCursorSetProps(strSetTags);
    length = strSetTags.Length();
    while (index < length) {
      newIndex = strSetTags.FindChar(comma,index);
      substring = Substring(strSetTags, index, newIndex - index);
      if (_retval.Length() > 0) _retval += strSep;
      _retval+=substring;
      index = newIndex + 1;
      newIndex = strSetTags.FindChar(semi, index);
      if (newIndex == -1) {
        break;
      }
      index = newIndex + 1;
    }
  }
  length = mparentTags?mparentTags->Count():-1;
  if (length > 0 && _retval.Length() > 0) { 
    _retval += strSep;
    _retval += *(*mparentTags)[0];
  }
  for (i = 1; i < length; i++)
    _retval += strSep + *(*mparentTags)[i];
  return NS_OK;
}

/* attribute nsIEditor editor; */
NS_IMETHODIMP 
msiTagListManager::GetEditor(nsEditor **aEditor)
{
    *aEditor = meditor;
    return NS_OK;
}

NS_IMETHODIMP 
msiTagListManager::SetEditor(nsEditor * aEditor)
{
    if (mparentTags && (meditor != aEditor))
      mparentTags->Clear();
    meditor = aEditor;
    return NS_OK;
}

nsCOMPtr<nsIDOMNode> 
msiTagListManager::GetChildAt(nsIDOMNode *aParent, PRInt32 aOffset)
{
  nsCOMPtr<nsIDOMNode> resultNode;
  nsCOMPtr<nsIContent> parent = do_QueryInterface(aParent);
  if (!parent) 
    return resultNode;
  resultNode = do_QueryInterface(parent->GetChildAt(aOffset));
  return resultNode;
}
  

/* AString currentValue (in AString strTagClass, out nsIAtom atomNS); */
// return the nearest tag in the class given by strTagClass; that is, the first node of that class encountered in going up the tree
NS_IMETHODIMP msiTagListManager::CurrentValue(const nsAString & strTagClass, nsIAtom **atomNS, nsAString & _retval)
{
  nsresult res;
  nsAutoString strTemp;
  nsAutoString strURI;
  nsString returnVal;
  PRBool isInList;
  
  if (!meditor)
    return NS_ERROR_NULL_POINTER;
  nsHTMLEditor * editor = static_cast<nsHTMLEditor*>(meditor);  
  nsCOMPtr<nsIDOMElement> element;
  nsCOMPtr<nsIDOMNode> node;
  nsCOMPtr<nsIDOMNode> node2;
  nsCOMPtr<nsIDOMNode> temp;
  nsCOMPtr<nsISelection> selection;
  nsCOMPtr<nsIDOMRange> range;
  nsIAtom * namespaceAtom;
  PRInt32 startoffset = 0;
  PRInt32 endoffset = 0;
  PRBool success = PR_TRUE;
/*
  // if the selection is a node and its contents, GetSelectionContainer doesn't return the node, but the parent.
  // Thus we have the next few lines of code
  res = meditor->GetSelection(getter_AddRefs(selection));
  if (NS_FAILED(res)) { return res; } 
  PRInt32 rangeCount;
  res = selection->GetRangeCount(&rangeCount);
  if (NS_FAILED(res)) { return res; }
  while (--rangeCount >= 0)
  {
    res = selection->GetRangeAt(rangeCount, getter_AddRefs(range));
    res = range->GetStartContainer(getter_AddRefs(node2));
    res = range->GetStartOffset(&startoffset);
    if (node == nsnull) node=node2;
    nsString name;  // for debugging purposes only
    node->GetNodeName(name);
    res = range->GetEndContainer(getter_AddRefs(node2));
    res = range->GetEndOffset(&endoffset);
    node->GetNodeName(name);  // for debugging purposes only
    if (node != node2) 
    {
      success = PR_FALSE;
      break;
    }
  }
  if (success && (endoffset-startoffset) == 1)
  {
    nsCOMPtr<nsIDOMNode> selectedNode = GetChildAt(node, startoffset);
    if (selectedNode)
      node = selectedNode;
    else success = PR_FALSE;
  }
*/
  success = PR_FALSE;  
  if (!success)
  {
    res = editor->GetSelectionContainer(getter_AddRefs(element));
    node = element;
  }
  while (node)
  {
    node->GetLocalName(strTemp);
//  get the namespace atom, too
     node->GetNamespaceURI(strURI);
    namespaceAtom = NS_NewAtom(strURI);
	  if (strTemp.Equals(NS_LITERAL_STRING("#document"))) break;
    GetTagInClass(strTagClass, strTemp, namespaceAtom, &isInList);
    if (isInList)
    {
      _retval = strTemp;
      node->GetNamespaceURI(strURI);
      *atomNS = namespaceAtom;
      return NS_OK;
    }
    node->GetParentNode((nsIDOMNode **)(&temp));
    node = temp;
  }
  _retval = returnVal;
  *atomNS = nsnull;
  return NS_OK;
}

nsString msiTagListManager::PrefixFromNameSpaceAtom(nsIAtom * atomNS)
{
  namespaceLookup * pns = plookup;
  while (pns)
  {
    if (pns->nsAtom == atomNS)
      return pns->namespaceAbbrev;
    pns = pns->next;
  }
  return NS_LITERAL_STRING(""); 
}


/* AString getRealClassOfTag (in AString strTag, in nsIAtom atomNS); */
NS_IMETHODIMP msiTagListManager::GetRealClassOfTag(const nsAString & strTag, nsIAtom *atomNS, nsAString & _retval)
{
  nsAutoString rv;
  GetStringPropertyForTag(strTag, atomNS, NS_LITERAL_STRING("realtagclass"), rv);
  if (rv.Length() > 0)
  {
    _retval = rv;
    return NS_OK;
  } 
  return GetStringPropertyForTag(strTag, atomNS, NS_LITERAL_STRING("tagclass"), _retval);
}  
 
/* AString getClassOfTag (in AString strTag, in nsIAtom atomNS); */
NS_IMETHODIMP msiTagListManager::GetClassOfTag(const nsAString & strTag, nsIAtom *atomNS, nsAString & _retval)
{
  return GetStringPropertyForTag(strTag, atomNS, NS_LITERAL_STRING("tagclass"), _retval);
}  
 


/* PRBool getTagInClass (in AString strTagClass, in AString strTag, in nsIAtom atomNS); */
NS_IMETHODIMP msiTagListManager::GetTagInClass(const nsAString & strTagClass, const nsAString & strTag, nsIAtom *atomNS, PRBool *_retval)
{
  nsAutoString strClass;
  GetClassOfTag(strTag, atomNS, strClass);
  *_retval = (strTagClass.Equals(strClass));
  return NS_OK;  
}

PRBool msiTagListManager::ContainsListForOuterIncludesInner( const nsAString & strOuter, const nsAString & strInner )
{
	// Go down the list of Tag list heads until name == strOuter
	TagKeyListHead * pTKLH = pContainsList;
	while (pTKLH && !(pTKLH->name.Equals(strOuter))) pTKLH = pTKLH->pNext;
	if (!pTKLH) return PR_FALSE;
	// Now go down this list looking for key.key == strInner
	TagKeyList * pTKL = pTKLH->pListHead;
	while (pTKL && !(pTKL->key.key.Equals(strInner))) pTKL = pTKL->pNext;
	if (pTKL) // found a match
	{
		return PR_TRUE;
	}
	return PR_FALSE;	
}


/* PRBool tagCanContainTag (in AString strTagOuter, in nsIAtom atomNSOuter, in AString strTagInner, in nsIAtom atomNSInner); */
NS_IMETHODIMP msiTagListManager::TagCanContainTag(const nsAString & strTagOuter, 
  nsIAtom *atomNSOuter, const nsAString & strTagInner, nsIAtom *atomNSInner, PRBool *_retval)
{
  if (!_retval) return NS_ERROR_INVALID_POINTER;
  if (strTagOuter.Length() == 0)  // nil outer tag --> no restrictions
  {
    *_retval = PR_TRUE;
    return NS_OK;
  }
  *_retval = PR_FALSE;
	PRBool foundit = PR_FALSE;
  nsAutoString classOuter;
  nsresult rv = GetRealClassOfTag(strTagOuter, atomNSOuter, classOuter);
  nsAutoString classInner;
  rv = GetRealClassOfTag(strTagInner, atomNSInner, classInner);
  // structtags are different: the level determines what can contain what.
  if (classOuter.Equals(classInner) && classOuter.EqualsLiteral("structtag"))
  {
    // both tags are structure tags, and so we need to look at the levels. We return true if the outer level or the inner level
    // is '*', or if the outer level is strictly less than the inner level.
    return LevelCanContainLevel( strTagOuter, atomNSOuter, strTagInner, atomNSInner, _retval);
  }
	// implementation of the rule that all unknown tags can go into a paragraph
	if (classOuter.EqualsLiteral("paratag") && classInner.Length() == 0) foundit = PR_TRUE;
  // check to see if the class of inner is in the class of the outer
	if (!foundit)
		foundit = ContainsListForOuterIncludesInner( classOuter, classInner );
	if (!foundit)
		foundit = ContainsListForOuterIncludesInner( classOuter, strTagInner);
	if (!foundit)
		foundit = ContainsListForOuterIncludesInner( strTagOuter, classInner);
	if (!foundit)
		foundit = ContainsListForOuterIncludesInner( strTagOuter, strTagInner);
  if (!foundit)	  
	{
		return NS_OK; // *_retval is false;
	}
	*_retval = PR_TRUE;
	return NS_OK;
}


/* PRBool nodeCanContainTag (in nsIDOMNode node, in AString strTagInner, in nsIAtom atomNSInner); */
NS_IMETHODIMP msiTagListManager::NodeCanContainTag(nsIDOMNode *node, const nsAString & strTagInner, nsIAtom *atomNSInner, PRBool *_retval)
{
  nsCOMPtr<nsIDOMElement> element = do_QueryInterface(node);
  if (!element) return PR_FALSE;
  nsresult res = NS_OK;
  nsAutoString stringTag;
  element->GetLocalName(stringTag);
  nsAutoString strNS;
  element->GetNamespaceURI(strNS);
  nsCOMPtr<nsIAtom> atomNS = NS_NewAtom(strNS);
  res = TagCanContainTag(stringTag, atomNS, strTagInner, atomNSInner, _retval);
  return res;
}


/* PRBool nodeCanContainTag (in nsIDOMNode nodeOuter, in nsIDOMNode nodeInner); */
NS_IMETHODIMP msiTagListManager::NodeCanContainNode(nsIDOMNode *nodeOuter, nsIDOMNode *nodeInner, PRBool *_retval)
{
  nsCOMPtr<nsIDOMElement> elementOuter = do_QueryInterface(nodeOuter);
  if (!elementOuter) return PR_FALSE;
  nsresult res = NS_OK;
  nsAutoString stringTagOuter;
  elementOuter->GetLocalName(stringTagOuter);
  nsAutoString strNSOuter;
  elementOuter->GetNamespaceURI(strNSOuter);
  nsCOMPtr<nsIAtom> atomNSOuter = NS_NewAtom(strNSOuter);
  nsCOMPtr<nsIDOMElement> elementInner = do_QueryInterface(nodeInner);
  if (!elementInner) return PR_FALSE;
  nsAutoString stringTagInner;
  elementInner->GetLocalName(stringTagInner);
  nsAutoString strNSInner;
  elementInner->GetNamespaceURI(strNSInner);
  nsCOMPtr<nsIAtom> atomNSInner = NS_NewAtom(strNSInner);
  res = TagCanContainTag(stringTagOuter, atomNSOuter, stringTagInner, atomNSInner, _retval);
  return res;
}



PRInt32 IntFromString( nsAString & str)
{
  PRBool fMinus = PR_FALSE;
  PRInt32 n = 0;
  const PRUnichar *start, *end;
  start = str.BeginReading();
  end = str.EndReading();
  if (start == end) return -200;   // -200 is less than any conceivable negative level, and so it is the error value.
  if (*start == '-')
  {
    fMinus = PR_TRUE;
    start++;
  }
  while (start != end)
  {
    if (*start >= '0' && *start <= '9') n = 10*n + *start - '0';
    start++;
  }
  if (fMinus) return (-n);
  else return n;
}


// Whether one structure tag can contain another depends on the levels. A tag with a * level
// can be contained in any structure tag. A non-* tag cannot be contained in a * tag.
// If both tags have numerical levels, then the tag with the lower level can contain the tag with the 
// higher level. Thus 'level' corresponds to the degree of indentation in outline form.

/* PRBool levelCanContainLevel (in AString strTagOuter, in nsIAtom atomNSOuter, in AString strTagInner, in nsIAtom atomNSInner); */
NS_IMETHODIMP msiTagListManager::LevelCanContainLevel(const nsAString & strTagOuter, nsIAtom *atomNSOuter, const nsAString & strTagInner, 
      nsIAtom *atomNSInner, PRBool *_retval)
{
  nsresult rv;
  nsString strOuterLevel;
  PRBool fOuterStar;
  nsString strInnerLevel;
  PRBool fInnerStar;
  PRInt32 nOuter;
  PRInt32 nInner;
  
  rv = GetStringPropertyForTag(strTagOuter, atomNSOuter, NS_LITERAL_STRING("level"), strOuterLevel);
//  strOuterLevel.StripWhiteSpace();
  rv = GetStringPropertyForTag(strTagInner, atomNSInner, NS_LITERAL_STRING("level"), strInnerLevel);
//  strInnerLevel.StripWhiteSpace();
  fOuterStar = strOuterLevel.EqualsLiteral("*");
  fInnerStar = strInnerLevel.EqualsLiteral("*");
  if (fInnerStar) *_retval = PR_TRUE;
  else if (fOuterStar) *_retval = PR_FALSE;
  else
  {
    nOuter = IntFromString(strOuterLevel);
    nInner = IntFromString(strInnerLevel);
    if (nOuter == -200 || nInner == -200) // -200 is the error value
    {
      *_retval = PR_FALSE;
      return NS_OK;  //TODO: get FAILURE return code;
    }
    *_retval = nOuter < nInner;
  }
  return NS_OK;
}

/* AString GetTagOfNode (in nsIDOMNode node); */
NS_IMETHODIMP msiTagListManager::GetTagOfNode(nsIDOMNode *node, nsIAtom ** atomNS, nsAString & _retval)
{
  nsresult rv = NS_ERROR_FAILURE;
  nsAutoString nodeName;
  nsAutoString strReturn;
  nsAutoString strNS;
  nsCOMPtr<nsIDOMElement> element = do_QueryInterface(node);
  if (element)
  {
    element->GetLocalName(strReturn);
    if (strReturn.Length() == 0)  // non-qualified name
    {
      *atomNS = nsnull;
      element->GetNodeName(strReturn);
    }
    else
    {
      element->GetNamespaceURI(strNS);
      *atomNS = NS_NewAtom(strNS);
    }
    _retval = strReturn;
    return NS_OK;
  }
  *atomNS = nsnull;
  _retval = NS_LITERAL_STRING("");
  return NS_OK;
}
 
/* AString GetStringPropertyForTag (in AString strTag, in nsIAtom atomNS, in AString propertyName); */
NS_IMETHODIMP msiTagListManager::GetStringPropertyForTag(const nsAString & strTag, nsIAtom *atomNS, const nsAString & propertyName, nsAString & _retval)
{
  // If *atomNS is null, we just use strTag to look up. Otherwise we concat the namespace abbreviation with strTag
  nsString strAbbrev;
  strAbbrev = PrefixFromNameSpaceAtom(atomNS);
  nsString strKey;
  nsString emptyString;
#ifdef USE_NAMESPACES 
  if (strAbbrev.Length() >0 )strKey = strAbbrev + NS_LITERAL_STRING(":") + strTag;
  else 
#endif
    strKey = strTag; 
  TagData * data;    
  PRBool fInHash = msiTagHashtable.Get(strKey, (TagData **)&data);
//  printf("\nFound in hash table is %s, strKey = %S, data->tagClass is ", (fInHash?"true":"false"), strKey.BeginReading());
//  if (fInHash) printf("%S\n", data->tagClass.BeginReading());  
//  else printf("\n");         
  if (fInHash)
  { 
    if (propertyName.EqualsLiteral("description"))
      _retval = data->description;
    else if (propertyName.EqualsLiteral("tagclass"))
      _retval = data->tagClass;
    else if (propertyName.EqualsLiteral("realtagclass"))
      _retval = data->realTagClass;
    else if (propertyName.EqualsLiteral("nextafteremptyblock"))
      _retval = data->nextAfterEmptyBlock;
    else if (propertyName.EqualsLiteral("titletag"))
       _retval = data->titleTag;
    else if (propertyName.EqualsLiteral("htmllist"))
       _retval = data->htmllist;
    else if (propertyName.EqualsLiteral("htmllistparent"))
       _retval = data->htmllistparent;
    else if (propertyName.EqualsLiteral("level"))
      _retval = data->level;
    else if (propertyName.EqualsLiteral("prefsub"))
      _retval = data->prefsub;
    else if (propertyName.EqualsLiteral("discardemptyblock"))
      _retval = data->discardEmptyBlock?NS_LITERAL_STRING("true"):NS_LITERAL_STRING("false");
    else if (propertyName.EqualsLiteral("inclusion"))
      _retval = data->inclusion?NS_LITERAL_STRING("true"):NS_LITERAL_STRING("false");
    else if (propertyName.EqualsLiteral("nexttag"))
      _retval = data->nextTag;
    else if (propertyName.EqualsLiteral("babel"))
			_retval = data->babel;
		else if (propertyName.EqualsLiteral("hidden"))
			_retval = data->hidden ? NS_LITERAL_STRING("1") : NS_LITERAL_STRING("0");
		else if (propertyName.EqualsLiteral("mathonly"))
			_retval = data->mathonly;
		else _retval.Assign(emptyString);
  }
  else _retval.Assign(emptyString);
  return NS_OK;
}


// 
NS_IMETHODIMP msiTagListManager::GetNewInstanceOfNode(const nsAString & strTag, nsIAtom *atomNS, nsIDOMDocument * doc, nsIDOMNode **_retval)
{
  nsresult res = NS_OK;
  nsString strKey;
  nsString emptyString;
  strKey = strTag; 
  TagData * data;    
  nsCOMPtr<nsIDOMNode> retNode;
  nsCOMPtr<nsIDOMNodeList> nodelist;
  PRBool setCursor = PR_FALSE;
  
  PRBool fInHash = msiTagHashtable.Get(strKey, (TagData **)&data);
  if (fInHash && data && doc && data->initialContents)
  {
    res = data->initialContents->CloneNode( PR_TRUE, getter_AddRefs(retNode));
    res = doc->ImportNode(retNode, PR_TRUE, _retval);
    NS_ADDREF(retNode.get());
  }
  return NS_OK;
}

/* void FixTagsAfterSplit (in nsIDOMNode firstNode, inout nsIDOMNode secondNode); */
NS_IMETHODIMP msiTagListManager::FixTagsAfterSplit(nsIDOMNode *firstNode, nsIDOMNode **secondNode)
{
#if DEBUG_barryNo || DEBUG_BarryNo
  DebExamineNode(firstNode);
  DebExamineNode(*secondNode);
  printf("=====FixTagsAfterSplit=============================================\n");
  printf("==firstNode:\n");
  meditor->DumpNode(firstNode);
  printf("==*secondNode:\n");
  meditor->DumpNode(*secondNode);
#endif
// find out what the second node should be
  nsString firstNodeName;
  nsIAtom * nsAtomFirst;
  nsString emptyString1;
  nsString emptyString2;
  nsString emptyString3;
  nsresult rv;
  nsCOMPtr<nsIDOMNode> aNewNode; 
  rv = GetTagOfNode(firstNode, &nsAtomFirst, firstNodeName);
  nsAutoString str;
  if (firstNodeName.Length()== 0) return NS_OK;
  nsIAtom * dummyatom = NS_NewAtom(NS_LITERAL_STRING(""));
  rv = GetStringPropertyForTag(firstNodeName, dummyatom, NS_LITERAL_STRING("nexttag"), str);
  PRBool isEmpty = PR_FALSE;
  nsHTMLEditor * editor = static_cast<nsHTMLEditor*>(meditor);  
  if (editor) editor->IsEmptyNode( *secondNode, &isEmpty);
	if (isEmpty) isEmpty = HasNoSignificantTags( *secondNode, this);
  if (str.EqualsLiteral("none") && isEmpty)
  {
    meditor->DeleteNode(*secondNode);
  }
  else 
  {
    if ((str.Length() > 0) && !str.EqualsLiteral("none")&&!(str.Equals(firstNodeName)))
    {
      meditor->ReplaceContainer(*secondNode, address_of(aNewNode), str, this, nsnull, nsnull, PR_TRUE);
      *secondNode = aNewNode;
  	  NS_ADDREF((nsIDOMNode *)aNewNode);
     	meditor->MarkNodeDirty(*secondNode);
    }
    // Gecko does not display empty paragraphs, so we need to put in a BR, maybe with an attribute
    nsCOMPtr<nsISelection>selection;
    rv = meditor->GetSelection(getter_AddRefs(selection));
    editor->IsEmptyNode( firstNode, &isEmpty);
    // check that there aren't significant tags in it, such as empty tables, etc.
		if (isEmpty) isEmpty = HasNoSignificantTags(firstNode, this);
		
    nsCOMPtr<nsIDOMDocument> doc;
    editor->GetDocument(getter_AddRefs(doc));  
    if (isEmpty)
    {
      nsCOMPtr<nsIDOMNode> brNode;
      rv = editor->CreateBR(firstNode, 0, address_of(brNode));

    }
    nsAutoString strContents;
    nsAutoString secondNodeName;
    nsIAtom * nsAtomSecond;
    rv = GetTagOfNode(*secondNode, &nsAtomSecond, secondNodeName);
    if (isEmpty)
    {
  //    rv = GetStringPropertyForTag(secondNodeName, dummyatom, NS_LITERAL_STRING("initialcontents"), strContents);
      if (strContents.Length() > 0)
      {
        // to make this operation undoable, we create a new node and copy its contents
        nsCOMPtr<nsIDOMNode> newNode;
        nsCOMPtr<nsIDOMNode> parentNode;
        rv = (*secondNode)->CloneNode(PR_FALSE, getter_AddRefs(newNode));
        nsCOMPtr<nsIDOMNSHTMLElement> secondElement(do_QueryInterface(newNode));
        (secondElement)->SetInnerHTML(strContents);
        nsCOMPtr<nsIDOMNode> child;
        PRBool bHasMoreChildren;
        newNode->HasChildNodes(&bHasMoreChildren);
        while (bHasMoreChildren)
        {
          newNode->GetFirstChild(getter_AddRefs(child));
          rv =meditor->DeleteNode(child);
  //        if (NS_FAILED(rv)) return rv;

          rv = meditor->InsertNode(child, *secondNode, -1);
          if (NS_FAILED(rv)) return rv;
          newNode->HasChildNodes(&bHasMoreChildren);
        }
        meditor->MarkNodeDirty(*secondNode);
      }
    }
    else
    {
      nsAutoString theclass;
      nsCOMPtr<nsIDOMNode> secttitle, br;
      GetRealClassOfTag(secondNodeName, nsAtomSecond, theclass);
      if (theclass.EqualsLiteral("structtag"))
      {
        // insert a sectiontitle tag at the beginning of the second half of a split section.
        rv = meditor->CreateNode(NS_LITERAL_STRING("sectiontitle"), *secondNode, 0, 
                                                   getter_AddRefs(secttitle));
        rv = editor->CreateBR(secttitle, 0, address_of(br));
        meditor->MarkNodeDirty(*secondNode);
        rv = selection->Collapse(secttitle, 0);
        return NS_OK;
      }
    }
    nsCOMPtr<nsIDOMNode> pnode = *secondNode;
    if (pnode)
    {
      PRBool setCursor;
      editor->SetSelectionOnCursorTag( pnode, &setCursor);
      if (setCursor)
        return NS_OK;
    }
    rv = selection->Collapse(*secondNode, 0);
  }
// diagnostics:
#if DEBUG_barry || DEBUG_Barry
  printf("==Leaving: firstNode: \n");
  editor->DumpNode(firstNode);
  printf("==*secondNode:\n");
  editor->DumpNode(*secondNode);
  printf("===================================================================\n");
#endif
  return NS_OK;
}

/* PRBool getDiscardEmptyBlockNode (in nsIDOMNode node); */
NS_IMETHODIMP msiTagListManager::GetDiscardEmptyBlockNode(nsIDOMNode *node, PRBool *_retval)
{
  nsresult rv;
  nsIAtom * dummyatom = NS_NewAtom("");
  if (!_retval) return NS_ERROR_NULL_POINTER;
  *_retval = PR_FALSE;
  nsString tag;
  nsIAtom * atomNS;
  rv = GetTagOfNode(node, &atomNS, tag);
  nsString propertyValue;
  if (tag.Length() > 0)
  {
    rv = GetStringPropertyForTag( tag, dummyatom, NS_LITERAL_STRING("discardemptyblock"), propertyValue);
    if (propertyValue.Equals(NS_LITERAL_STRING("yes")) || propertyValue.Equals(NS_LITERAL_STRING("true")))
      *_retval = PR_TRUE;
  }
  return NS_OK;
}



/* AString GetDefaultParagraphTag (out nsIAtom atomNamespace); */
NS_IMETHODIMP msiTagListManager::GetDefaultParagraphTag(nsIAtom **atomNamespace, nsAString & _retval)
{
  _retval = mdefaultParagraph.localName();
  *atomNamespace = NS_NewAtom(mdefaultParagraph.prefix());
  return NS_OK;
}

NS_IMETHODIMP msiTagListManager::GetClearTextTag(nsIAtom **atomNamespace, nsAString & _retval)
{
  _retval = mclearTextTag.localName();
  *atomNamespace = NS_NewAtom(mclearTextTag.prefix());
  return NS_OK;
}


NS_IMETHODIMP msiTagListManager::GetClearStructTag(nsIAtom **atomNamespace, nsAString & _retval)
{
  _retval = mclearStructTag.localName();
  *atomNamespace = NS_NewAtom(mclearStructTag.prefix());
  return NS_OK;
}

NS_IMETHODIMP msiTagListManager::GetClearEnvTag(nsIAtom **atomNamespace, nsAString & _retval)
{
  _retval = mclearEnvTag.localName();
  *atomNamespace = NS_NewAtom(mclearEnvTag.prefix());
  return NS_OK;
}

NS_IMETHODIMP msiTagListManager::GetClearListTag(nsIAtom **atomNamespace, nsAString & _retval)
{
  _retval = mclearListTag.localName();
  *atomNamespace = NS_NewAtom(mclearListTag.prefix());
  return NS_OK;
}


/* boolean selectionContainedInTag (in AString strTag, in nsIAtom atomNS); 
   This should return true if all the characters in the selection are affected the tag.
   This is certainly the case if all the nodes in the selection are descendents
   of a strTag node.
   Another possibility is the the nodes in the selection are contained in two 
   almost-adjacent strTag node.
   The final test is to iterate through all the text nodes in the selection. Each
   one must be either all white space or a descendent of a strTag node or (if the
   first or last node in the selection) all white space after or before the offset), */


PRBool isInTag(nsIDOMNode * node, const nsAString & strTag)
{
  nsAutoString strTemp;
  nsCOMPtr<nsIDOMNode> parent;
  while (node) {
    node->GetLocalName(strTemp);
    if (strTemp.Equals(strTag)) return PR_TRUE;
    node->GetParentNode(getter_AddRefs(parent));
    node = parent;
  }
  return PR_FALSE;
}

NS_IMETHODIMP 
msiTagListManager::SelectionContainedInTag(const nsAString & strTag, nsIAtom *atomNS, PRBool *_retval)
{
  nsAutoString str;
  nsresult rv;
  *_retval = PR_FALSE;
  if (!mparentTags)
  {
    return NS_OK;
  }
  int j = mparentTags->Count();
  for (int i = 0; i < j; i++)
	{
	 	mparentTags->StringAt(i, str);
    TagKey key(str);
	  if (strTag.Equals(key.localName()))
	 	{
	     *_retval = PR_TRUE;
	     return NS_OK;
	 	}
	}
  // The simple test failed. Now check the text nodes of (the first range of) the selection
  nsCOMPtr<nsIDOMRange> range;
  nsCOMPtr<nsIDOMNode> startContainer;
  nsCOMPtr<nsIDOMNode> endContainer;
  nsCOMPtr<nsIDOMNode> ancestor;
  nsCOMPtr<nsIDOMNode> start;
  nsCOMPtr<nsIDOMNode> end;
  nsCOMPtr<nsIDOMNode> lastTextNode;
  nsCOMPtr<nsIDOMNode> firstNode(nsnull);
  nsCOMPtr<nsIDOMNode> lastNode(nsnull);
  PRInt32 startOffset;
  PRInt32 endOffset;
  nsCOMPtr<nsISelection>selection;
  rv = meditor->GetSelection(getter_AddRefs(selection));
  PRBool collapsed;
  rv = selection->GetIsCollapsed(&collapsed);
  if (collapsed) return NS_OK;  //retval is false
  rv = selection->GetRangeAt(0, getter_AddRefs(range));
  rv = range->GetCommonAncestorContainer(getter_AddRefs(ancestor));
  if (ancestor) ancestor->Normalize();
  rv = selection->GetRangeAt(0, getter_AddRefs(range)); // do it again because Normalize might have changed things.
  rv = range->GetStartContainer(getter_AddRefs(startContainer));
  rv = range->GetStartOffset(&startOffset);
  rv = range->GetEndContainer(getter_AddRefs(endContainer));
  rv = range->GetEndOffset(&endOffset);
  // if startContainer is a text node, test it
  PRUint16 nodeType;
  PRUint32 count;
  PRBool lastTextNodeChecked(PR_FALSE);
  nsCOMPtr<nsIDOMNodeList> childNodes;
  startContainer->GetNodeType(&nodeType);
  if (nodeType == nsIDOMNode::TEXT_NODE)
  {
    PRUint32 theEnd = 1000; // any big number
    if (endContainer == startContainer) theEnd = endOffset;
    if (!nodeIsWhiteSpace(startContainer, startOffset, theEnd))
    {
      if (!isInTag(startContainer, strTag)) return NS_OK;
      // if any part if startContainer is covered by the tag, then all of it is
    }
    firstNode = startContainer;
  }
  else
  {
    // Assert nodeType is ELEMENT_NODE
    rv = startContainer->GetChildNodes(getter_AddRefs(childNodes));
    if (!((NS_SUCCEEDED(rv)) && (childNodes))) return rv;
    childNodes->GetLength(&count);
    if (startOffset > count)
      startOffset = count;
    rv = childNodes->Item(startOffset, getter_AddRefs(firstNode));
  }

  endContainer->GetNodeType(&nodeType);
  if (nodeType == nsIDOMNode::TEXT_NODE)
  {
    PRUint32 theStart = 0;
    if (endContainer == startContainer) theStart = startOffset;
    if (!nodeIsWhiteSpace(endContainer, theStart, endOffset))
    {
      if (!isInTag(endContainer, strTag))  return NS_OK;
    }
    lastTextNode = endContainer;
    lastTextNodeChecked = PR_TRUE;
  }
  else
  {
    // Assert nodeType is ELEMENT_NODE
    rv = endContainer->GetChildNodes(getter_AddRefs(childNodes));
    if (!((NS_SUCCEEDED(rv)) && (childNodes))) return rv;
    childNodes->GetLength(&count);
    if (endOffset > count)
      endOffset = count;
    rv = childNodes->Item(endOffset, getter_AddRefs(lastNode));
  }
  // set up a treewalker to iterate through 
  nsCOMPtr<nsIDOMDocument> doc;
  rv = startContainer->GetOwnerDocument(getter_AddRefs(doc));
  nsCOMPtr<nsIDOMTreeWalker> tw;
  nsCOMPtr<nsIDOMDocumentTraversal> doctrav;
  doctrav = do_QueryInterface(doc);
  rv = doctrav->CreateTreeWalker( ancestor, nsIDOMNodeFilter::SHOW_TEXT, nsnull, PR_FALSE, getter_AddRefs(tw));
  if (!(NS_SUCCEEDED(rv) && tw)) return rv;
  // Now find the last text node so we know when to stop

	nsCOMPtr<nsIDOMNode> currentNode;
	if (!lastNode) return NS_OK;
  if (!lastTextNode)
  {
    tw->SetCurrentNode(lastNode);
    tw->LastChild(getter_AddRefs(lastTextNode));
  }

// Now reset the treewalker
  tw->SetCurrentNode(firstNode);
  currentNode = firstNode;
  PRBool done(PR_FALSE);
  while (currentNode && !done)
  {
    
    if (currentNode == firstNode)
    {
      firstNode->GetNodeType(&nodeType);
      if (nodeType != nsIDOMNode::TEXT_NODE)
        tw->NextNode(getter_AddRefs(currentNode));
    }
    if (currentNode) {
      if (currentNode == lastTextNode && lastTextNodeChecked) {
        *_retval = PR_TRUE;
        return NS_OK;
      }
      if (!nodeIsWhiteSpace(currentNode, 0, 1000)) 
        if (!isInTag(currentNode, strTag))  return NS_OK;
    }
    done = (!currentNode || currentNode == lastTextNode);
    tw->NextNode(getter_AddRefs(currentNode));
  }
  // since we passed all the tests ...
	*_retval = PR_TRUE;
  
  return NS_OK;
}


/* AString getTagsInClass (in AString strTag, in boolean includeNS); */
NS_IMETHODIMP 
msiTagListManager::GetTagsInClass(const nsAString & strTagClass, const nsAString & strSep, PRBool includeNS, nsAString & _retval)
{
// we ignore includeNS for now.
  return pACSSA->ContentsofArray( strTagClass, strSep, _retval);  
}

/* Methods for changing the tag list at runtime, to support creating tags dynamically for Polyglossia/Babel
   AString getTagsWithProperty( in AString propertyName, in nsIAtom atomNS);  */
NS_IMETHODIMP
msiTagListManager::GetBabelTags(nsAString & _retval)
{
	nsAutoString returnString;
	PRBool first = PR_TRUE;
	TagKeyList * pTagKeyList = pBabelList->pListHead;
	while (pTagKeyList)
	{
		if (!first) returnString = returnString + NS_LITERAL_STRING(",");
		first = PR_FALSE;
		returnString = returnString + pTagKeyList->key.localName();
		pTagKeyList = pTagKeyList->pNext;
	}
	_retval = returnString;
	return NS_OK;
}

		

/*void    setTagVisibility( in AString strTag, in nsIAtom atomNS, in boolean hidden); 
  We change the entries in the DOM tree, not the hash table. */
NS_IMETHODIMP
msiTagListManager::SetTagVisibility(const nsAString & strTag, nsIAtom *atomNS, PRBool hidden)
{
	nsresult rv;
	nsCOMPtr<nsIDOMNodeList> taglist;
	nsCOMPtr<nsIDOMNode> tag;
	nsCOMPtr<nsIDOMElement> tagElement;
	PRUint32 count;
	PRUint32 i;
	nsAutoString name;
	
	rv = mdocTagInfo->GetElementsByTagName(NS_LITERAL_STRING("tag"), getter_AddRefs(taglist));
	if (taglist)
	{
		taglist->GetLength(&count);
		for (i = 0; i < count; i++)
		{
			rv = taglist->Item(i, getter_AddRefs(tag));
			tagElement= do_QueryInterface(tag);
			rv = tagElement->GetAttribute(NS_LITERAL_STRING("nm"), name);
			if (name.Equals(strTag))
			{
				if (hidden)
				{
					tagElement->SetAttribute(NS_LITERAL_STRING("hidden"), NS_LITERAL_STRING("true"));
				}
				else
				{
					tagElement->RemoveAttribute(NS_LITERAL_STRING("hidden"));
				}
				return NS_OK;
			}		
		}
	}
	return NS_OK;
}


/* void    setTagName( in AString strCurrent, in nsIAtom atomNS, in AString strNew);*/

NS_IMETHODIMP
msiTagListManager::SetTagName(const nsAString & strCurrent, nsIAtom *atomNS, const nsAString & strNew)
{
	nsresult rv;
	nsCOMPtr<nsIDOMNodeList> taglist;
	nsCOMPtr<nsIDOMNode> tag;
	nsCOMPtr<nsIDOMElement> tagElement;
	PRUint32 count;
	PRUint32 i;
	nsAutoString name;
	
	rv = mdocTagInfo->GetElementsByTagName(NS_LITERAL_STRING("tag"), getter_AddRefs(taglist));
	if (taglist)
	{
		taglist->GetLength(&count);
		for (i = 0; i < count; i++)
		{
			rv = taglist->Item(i, getter_AddRefs(tag));
			tagElement= do_QueryInterface(tag);
			rv = tagElement->GetAttribute(NS_LITERAL_STRING("nm"), name);
			if (name.Equals(strCurrent))
			{
				tagElement->SetAttribute(NS_LITERAL_STRING("nm"), strNew);
				return NS_OK;
			}		
		}
	}
	return NS_OK;
}

NS_IMETHODIMP
msiTagListManager::RebuildHash()
{
	PRBool retval;
	nsresult rv;
	BuildHashTables(mdocTagInfo, &retval);
	return NS_OK;
}

/* PRInt32 getMinDepth (); */
NS_IMETHODIMP 
msiTagListManager::GetMinDepth(PRInt32 *_retval)
{
  nsCOMPtr<nsIDOMNodeList> taglist;
  nsCOMPtr<nsIDOMNode> tag;
  nsCOMPtr<nsIDOMElement> tagElement;
  PRUint32 count;
  nsAutoString val;
  nsresult rv;
  PRInt32 errorCode;

  rv = mdocTagInfo->GetElementsByTagName(NS_LITERAL_STRING("mindepth"), getter_AddRefs(taglist));
  taglist->GetLength(&count);
  if (count > 0)
  {
    rv = taglist->Item(0, getter_AddRefs(tag));
    tagElement = do_QueryInterface(tag);
    tagElement->GetAttribute(NS_LITERAL_STRING("value"), val); 
    *_retval = val.ToInteger(&errorCode);   
    return NS_OK;
  }
}
