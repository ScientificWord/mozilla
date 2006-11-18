#include "msiTagListManager.h"
#include "nsClassHashtable.h"
#include "nsUnicharUtils.h"
#include "nsIDOMElement.h"
#include "nsIDOMNSHTMLElement.h"
#include "nsIDocument.h"
#include "nsIDOMDocument.h"
#include "nsIDOMXMLDocument.h"
#include "nsIDOMNodeList.h"
#include "nsIDOMRange.h"
#include "nsIHTMLEditor.h"
#include "nsISelection.h"
#include "prerror.h"
#include "nsIContent.h"
#include "nsContentCID.h"
#include "nsComponentManagerUtils.h"
#include "nsISelectionPrivate.h"
#include "nsIDOM3Node.h"
#include "../text/nsPlaintextEditor.h"
#include "../html/nsHTMLEditor.h"
#include "nsIAutoCompleteSearchStringArray.h"

/* Implementation file */
NS_INTERFACE_MAP_BEGIN(msiTagListManager)
  NS_INTERFACE_MAP_ENTRY(msiTagListManager)
  NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(nsISupports, msiITagListManager)
NS_INTERFACE_MAP_END

NS_IMPL_ADDREF(msiTagListManager)
NS_IMPL_RELEASE(msiTagListManager)

/*#if DEBUG_barry || DEBUG_Barry
void DebExamineNode(nsIDOMNode * aNode);
#endif */

nsIAtom * msiTagListManager::htmlnsAtom = NS_NewAtom(NS_LITERAL_STRING("http://www.w3.org/1999/xhtml"));



msiTagListManager::msiTagListManager()
:  meditor(nsnull), mparentTags(nsnull), mInitialized(PR_FALSE), plookup(nsnull), pContainsList(nsnull) 
{
  nsresult rv;
  pACSSA = do_CreateInstance("@mozilla.org/autocomplete/search;1?name=stringarray", &rv);
  // Now replace the singleton autocompletesearchstringarry by an implementation
  pACSSA->GetNewImplementation(getter_AddRefs(pACSSA));
  Reset();
}

msiTagListManager * MSI_NewTagListManager()
{
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
  pACSSA->SetImplementation(pACSSA);
  return NS_OK;
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
    akey.Right(str, (key.Length() - hyphen +3));
    if (str.Length() > 0) key = str +  NS_LITERAL_STRING(":") +key;                                
  } else key = akey;
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

NS_IMETHODIMP 
msiTagListManager::AddTagInfo(const nsAString & strTagInfoPath, PRBool *_retval) 
{
  nsresult rv;
  nsAutoString str;
  nsCOMPtr<nsIDOMXMLDocument> docTagInfo;
  
  // load the XML tag info file 
  *_retval = PR_FALSE;
  docTagInfo = do_CreateInstance(kXMLDocumentCID, &rv);
  if (rv) return rv;
  rv = docTagInfo->SetAsync(PR_FALSE);
  if (rv) return rv;
  rv = docTagInfo->Load( strTagInfoPath, _retval);
  if (rv) return rv;
  BuildHashTables(docTagInfo, _retval);
  // build the name space list
  nsCOMPtr<nsIDOMNodeList> nodeList;
  nsString strNameSpace;
  nsString strAbbrev;
  PRUint32 nodeCount;
  nsCOMPtr<nsIDOM3Node> textNode; 
  nsCOMPtr<nsIDOMNode> node; 
  nsCOMPtr<nsIDOMElement> nodeElement;
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
  mInitialized = PR_TRUE;
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
    textNode->GetTextContent(strResult);
  }
  return strResult;
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
  PRUint32 tagContainsCount = 0;
  nsCOMPtr<nsIDOMNodeList> tagClasses;
  nsCOMPtr<nsIDOMNode> tagClass;
  nsCOMPtr<nsIDOMElement> tagClassElement;
  nsCOMPtr<nsIDOMNodeList> tagClassNames;
  nsCOMPtr<nsIDOMNode> tagClassName;
  nsCOMPtr<nsIDOMElement> tagClassNameElement;
  nsCOMPtr<nsIDOM3Node> tagClassNameNode;
  nsCOMPtr<nsIDOMNodeList> tagNames;
  nsCOMPtr<nsIDOMNodeList> tagContains;
  nsCOMPtr<nsIDOMNode> tagName;
  nsCOMPtr<nsIDOMNode> tagCanContain;
  nsIDOMXMLDocument * dti = docTagInfo;
  nsAutoString strName;
  nsAutoString strClassName;
  nsAutoString strCanContain;
  nsCOMPtr<nsIDOMElement> tagNameElement;   
  nsCOMPtr<nsIDOMElement> tagCanContainElement;   
  nsCOMPtr<nsIDOM3Node> tagCanContainNode3;
  TagKeyListHead * pTagKeyListHead;
  TagKeyList * pTagKeyList;
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
      rv = tagClassElement->GetElementsByTagName(NS_LITERAL_STRING("contains"), getter_AddRefs(tagContains));
      if (tagContains) 
      {
        rv = tagContains->GetLength(&tagContainsCount);
        for (PRUint32 j = 0; j < tagContainsCount; j++)
        {
          tagContains ->Item(j, getter_AddRefs(tagCanContain));
          tagCanContainNode3 = do_QueryInterface(tagCanContain);
          tagCanContainNode3->GetTextContent(strCanContain);
          // now we put strCanContain (a tag key) in the list for strClassName
          pTagKeyListHead = pContainsList;
          while (pTagKeyListHead && !(pTagKeyListHead->name.Equals(strClassName))) pTagKeyListHead = pTagKeyListHead->pNext;
          if (!pTagKeyListHead) // there is no list for the class name; make one
          {
            pTagKeyListHead = new TagKeyListHead;
            pTagKeyListHead->name.Assign(strClassName);
            pTagKeyListHead->pNext = pContainsList;  // we insert at the beginning of the list for simplicity
            pContainsList = pTagKeyListHead;
          } // pTagKeyListHead is not null
          // TODO: check for out of memory
          pTagKeyList = new TagKeyList;
          pTagKeyList->key.key.Assign(strCanContain);
          if (pTagKeyListHead->pListTail)
            pTagKeyListHead->pListTail->pNext = pTagKeyList;   // we use ListTail to keep the list in the same order as in the XNL file,
          pTagKeyListHead->pListTail = pTagKeyList;   // we use ListTail to keep the list in the same order as in the XNL file,
                                                      // for hand optimizations
          if (!(pTagKeyListHead->pListHead)) pTagKeyListHead->pListHead = pTagKeyListHead->pListTail;
        }
      }
      
      // Write out the list
      printf("\nContains list for tag %S:\n", pContainsList->name.BeginReading());
      pTagKeyList = pContainsList->pListHead;
      while (pTagKeyList)
      {
        printf("  can contain \"%S\n", ((nsString)pTagKeyList->key).BeginReading());
        pTagKeyList = pTagKeyList->pNext;
      }
      
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
          key = TagKey(strName);
          TagData *pdata = new TagData;               
          pdata->tagClass = strClassName;
          pdata->description = GetStringProperty(NS_LITERAL_STRING("description"), tagNameElement);
          pdata->initialContentsForEmpty = 
            GetStringProperty(NS_LITERAL_STRING("initialContentsForEmpty"), tagNameElement);
          pdata->initialContents =
            GetStringProperty(NS_LITERAL_STRING("initialContents"), tagNameElement);
          pdata->discardEmptyBlock = 
            GetStringProperty(NS_LITERAL_STRING("discardEmptyBlock"), tagNameElement)==NS_LITERAL_STRING("true");
          // save the key, data pair
          msiTagHashtable.Put(key.key, pdata);
//		      printf("Added %s: %s %s\n", ToNewCString(key.key), ToNewCString(pdata->description), ToNewCString(pdata->tagClass));
//          printf("hash table has %d entries\n",msiTagHashtable.Count()); 
        }
      }
    }
  }
  BuildStringArray(NS_LITERAL_STRING("texttag"));
  BuildStringArray(NS_LITERAL_STRING("paratag"));
  BuildStringArray(NS_LITERAL_STRING("structtag"));
  BuildStringArray(NS_LITERAL_STRING("othertag"));
  pACSSA->SortArrays();
  return PR_TRUE;
}


struct userArgStruct {
//  nsStringArray * stringArray;
  nsString tagClass;
  msiTagListManager * ptlm;
};

/* nsStringArray getStringArray (in AString strTagClass); */
PLDHashOperator
nsDEnumRead(const nsAString_internal& aKey, TagData* aData, void* userArg) 
{  // this is a callback to the hash table enumerator that adds to a string array all the data that 
   // matches the ... part of *userArg
  TagKey tk;
  PRBool bres;
  tk.key = aKey;;
  userArgStruct * ua = (userArgStruct *) userArg;
//   printf("  enumerated %s = \"%s\"\n", aKey.get(), aData->tagClass.get());
  if (aData->tagClass == ua->tagClass)
  {
    nsString name = tk.altForm();
    ua->ptlm->pACSSA->AddString(ua->tagClass, name, &bres);
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
  // now we need to sort the array
  return NS_OK;
}

NS_IMETHODIMP 
msiTagListManager::BuildParentTagList()
{
  // BBM todo: handle namespaces here
  nsresult res;
  nsAutoString strTemp;
  nsAutoString strTemp2;
  nsAutoString strTempURI;
  nsAutoString strQName;
  nsCOMPtr<nsIAtom> nsAtom; // namespace atom
  if (!mparentTags) mparentTags = new nsStringArray();
  mparentTags->Clear();
 // BBM todo: put in error checking
  if (!meditor)
    return  NS_ERROR_FAILURE;;
  nsHTMLEditor * editor = NS_STATIC_CAST(nsHTMLEditor *,meditor);  
  nsCOMPtr<nsIDOMElement> element;
  nsCOMPtr<nsIDOMNode> node;
  nsCOMPtr<nsIDOMNode> temp;
  res = editor->GetSelectionContainer(getter_AddRefs(element));
  node = element;
  while (node)
  {
    node->GetLocalName(strTemp);
	  if (strTemp.Equals(NS_LITERAL_STRING("#document"))) break;
    node->GetNamespaceURI(strTempURI);
    if (strTempURI.Length() == 0) nsAtom = nsnull;
    else nsAtom = NS_NewAtom(strTempURI);
    if (nsAtom) 
    {
      strTemp2 = NS_LITERAL_STRING(" - ")+PrefixFromNameSpaceAtom(nsAtom);
      strQName = strTemp + strTemp2;
    }
    else strQName = strTemp;  
	  mparentTags->AppendString(strQName);
    node->GetParentNode((nsIDOMNode **)(&temp));
    node = temp;
  }
  return pACSSA->SetMarkedStrings(mparentTags);
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
  nsHTMLEditor * editor = NS_STATIC_CAST(nsHTMLEditor *, meditor);  
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


/* AString getClassOfTag (in AString strTag, in nsIAtom atomNS); */
NS_IMETHODIMP msiTagListManager::GetClassOfTag(const nsAString & strTag, nsIAtom *atomNS, nsAString & _retval)
{
  // If *atomNS is null, we just use strTag to look up. Otherwise we concat the namespace abbreviation with strTag
  nsString strAbbrev;
  strAbbrev = PrefixFromNameSpaceAtom(atomNS);
  nsString strKey;
  nsString emptyString;
  if (strAbbrev.Length() >0 )strKey = strAbbrev + NS_LITERAL_STRING(":") + strTag;
  else strKey = strTag; 
  TagData * data;    
  PRBool fInHash = msiTagHashtable.Get(strKey, (TagData **)&data);
//  printf("\nFound in hash table is %s, strKey = %S, data->tagClass is ", (fInHash?"true":"false"), strKey.BeginReading());
//  if (fInHash) printf("%S\n", data->tagClass.BeginReading());  
//  else printf("\n");         
  if (fInHash)
  {
    _retval = data->tagClass;
  }
  else _retval.Assign(emptyString);
  return NS_OK;
}  
 


/* PRBool getTagInClass (in AString strTagClass, in AString strTag, in nsIAtom atomNS); */
NS_IMETHODIMP msiTagListManager::GetTagInClass(const nsAString & strTagClass, const nsAString & strTag, nsIAtom *atomNS, PRBool *_retval)
{
  nsAutoString strClass;
  GetClassOfTag(strTag, atomNS, strClass);
  *_retval = (strTagClass.Equals(strClass));
  return NS_OK;  
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
  nsAutoString classOuter;
  nsresult rv = GetClassOfTag(strTagOuter, atomNSOuter, classOuter);
  nsAutoString classInner;
  rv = GetClassOfTag(strTagInner, atomNSInner, classInner);
  
  // structtags are different: the level determines what can contain what.
  if (classOuter.Equals(classInner) && classOuter.EqualsLiteral("structtag"))
  {
    // both tags are structure tags, and so we need to look at the levels. We return true if the outer level or the inner level
    // is '*', or if the outer level is strictly less than the inner level.
    return LevelCanContainLevel( strTagOuter, atomNSOuter, strTagInner, atomNSInner, _retval);
  }
  // Find the contains list for classOuter
  TagKeyListHead * pTagKeyListHead = pContainsList;
  while (PR_TRUE)
  {
    while (pTagKeyListHead && !(pTagKeyListHead->name.Equals(classOuter))) 
      pTagKeyListHead = pTagKeyListHead->pNext;
    if (!pTagKeyListHead) // there is no list for the class name; treat it like a text tag
    {
      if (classOuter.EqualsLiteral("texttag")) break;
        // we already changed the name and *still* didn't find the list header
      classOuter = NS_LITERAL_STRING("texttag");
      pTagKeyListHead = pContainsList;
    }
    else break;
  }
  if (!pTagKeyListHead) return NS_ERROR_FAILURE;
  // in the list headed by pTagKeyListHead, search for the classInner string, then the 
  // strTagInner string.
  TagKeyList * pTKL = pTagKeyListHead->pListHead;
  while (pTKL && !(classInner.Equals(pTKL->key.key))) pTKL = pTKL->pNext;
  if (pTKL) // a match was found
  {
    *_retval = PR_TRUE;
    return NS_OK; 
  }
  // Broad categories didn't work; now look for a specific tag
  nsString strTemp;
  strTemp.Assign(strTagInner);
  TagKey tagkeyLookup(strTemp, PrefixFromNameSpaceAtom(atomNSInner)); 
  printf("Looking for tag %S\n", tagkeyLookup.key.BeginReading());
  pTKL = pTagKeyListHead->pListHead;
  while (pTKL && !(tagkeyLookup.key.Equals(pTKL->key.key))) pTKL = pTKL->pNext;
  if (pTKL) // a match was found
  {
    *_retval = PR_TRUE;
    return NS_OK; 
  }
  // failed to find anything if we got here.
  *_retval = PR_FALSE;
  return NS_OK;
}


PRInt32 IntFromString( nsAString & str)
{
  PRBool fMinus = PR_FALSE;
  PRInt32 n = 0;
  nsAString::const_iterator start, end; // reading-only iterators for nsAString 
  if (start == end) return -200;   // -200 is less than any conceivable negative level, and so it is the error value.
  if (*start == '-')
  {
    fMinus = PR_TRUE;
    start++;
  }
  while (start != end)
  {
    n = 10*n + *start - '0';
    start++;
  }
  if (fMinus) return (-n);
  else return n;
}

/* PRBool levelCanContainLevel (in AString strTagOuter, in nsIAtom atomNSOuter, in AString strTagInner, in nsIAtom atomNSInner); */
NS_IMETHODIMP msiTagListManager::LevelCanContainLevel(const nsAString & strTagOuter, nsIAtom *atomNSOuter, const nsAString & strTagInner, nsIAtom *atomNSInner, PRBool *_retval)
{
  nsresult rv;
  nsCOMPtr<nsIDOMNodeList> tagClassNames;
  nsCOMPtr<nsIDOMNodeList> tagNames;
  nsCOMPtr<nsIDOMNode> tagClassName;
  nsCOMPtr<nsIDOM3Node> tagClassNameNode;
  nsCOMPtr<nsIDOMNode> tagClass;
  nsCOMPtr<nsIDOMElement> tagClassElement;
  nsCOMPtr<nsIDOMNode> tagName;
  nsCOMPtr<nsIDOM3Node> tagNameNode;
  nsCOMPtr<nsIDOMNode> tagLevel;
  nsCOMPtr<nsIDOM3Node> tagLevelNode;
  PRUint32 count;
  PRInt32 outerLevel = -200;
  PRInt32 innerLevel = -200;
  nsAutoString strTemp;
  // We need to find the levels for these tags.  First find the secttags section
  rv = mdocTagInfo->GetElementsByTagName(NS_LITERAL_STRING("tagclassname"), getter_AddRefs(tagClassNames));
  if (tagClassNames) tagClassNames->GetLength(&count);
  if (count > 0)
  {
    for (PRUint32 i = 0; i < count; i++)
    {
      tagClassNames->Item(i, getter_AddRefs(tagClassName));
      if (tagClassName) 
      {
        tagClassNameNode = do_QueryInterface(tagClassName);
        rv = tagClassNameNode->GetTextContent(strTemp);
        if (strTemp.Equals(NS_LITERAL_STRING("secttag")))   // tagClassNameNode has the taxt "secttag"
        {
          tagClassName->GetParentNode( getter_AddRefs(tagClass));
          tagClassElement = do_QueryInterface(tagClass);
          rv = tagClassElement->GetElementsByTagName(NS_LITERAL_STRING("name"), getter_AddRefs(tagNames));
          if (tagNames) tagNames->GetLength(&count);
          if (count > 0)
          {
            for (PRUint32 j = 0; j < count; j++)
            {
              tagNames->Item(j, getter_AddRefs(tagName));
              if (tagName) 
              {
                tagNameNode = do_QueryInterface(tagName);
                rv = tagNameNode->GetTextContent(strTemp);
                if (strTemp.Equals(strTagOuter))   // the name matches the outer tag name.  Now find the level (it is the sibling
                // following tagName.
                {
                  tagName->GetNextSibling(getter_AddRefs(tagLevel));
                  tagLevelNode = do_QueryInterface(tagLevel);
                  rv = tagLevelNode->GetTextContent(strTemp);
                  if (strTemp.Equals(NS_LITERAL_STRING("*"))) outerLevel = -100;
                    else outerLevel = IntFromString(strTemp);
                  break;
                }
              }
            }
            for (PRUint32 j = 0; j < count; j++)
            {
              tagNames->Item(j, getter_AddRefs(tagName));
              if (tagName) 
              {
                tagNameNode = do_QueryInterface(tagName);
                rv = tagNameNode->GetTextContent(strTemp);
                if (strTemp.Equals(strTagInner))   // the name matches the outer tag name.  Now find the level (it is the sibling
                // following tagName.
                {
                  tagName->GetNextSibling(getter_AddRefs(tagLevel));
                  tagLevelNode = do_QueryInterface(tagLevel);
                  rv = tagLevelNode->GetTextContent(strTemp);
                  if (strTemp.Equals(NS_LITERAL_STRING("*"))) innerLevel = -100;
                    else innerLevel = IntFromString(strTemp);
                  break;
                }
              }
            }
          }
        }
      }
    }
  } 
  if ((outerLevel < -100) || (innerLevel < -100))
    *_retval = PR_FALSE;   // didn't find both level
  else if (innerLevel == -100)
    *_retval = PR_TRUE;    // "*" sections can go anywhere 
  else if (outerLevel = -100)
    *_retval = PR_FALSE;   // don't allow a numbered level inside a * level
  else *_retval = (outerLevel < innerLevel);
  
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
}
 
/* AString GetStringPropertyForTag (in AString strTag, in nsIAtom atomNS, in AString propertyName); */
NS_IMETHODIMP msiTagListManager::GetStringPropertyForTag(const nsAString & strTag, nsIAtom *atomNS, const nsAString & propertyName, nsAString & _retval)
{
  nsAutoString strResult;
  nsCOMPtr<nsIDOMElement> tagElement;
  nsCOMPtr<nsIDOM3Node> textNode; 
  nsCOMPtr<nsIDOMNode> node; 
  nsCOMPtr<nsIDOMNodeList> nodeList;
  PRUint32 nodeCount; 
  nsresult rv; 
  rv = mdocTagInfo->GetElementById(strTag, getter_AddRefs(tagElement));
  if (!rv && tagElement)
  {
    rv = tagElement->GetElementsByTagName(propertyName, getter_AddRefs(nodeList));
    if (!rv)
    {
      if (nodeList) nodeList->GetLength(&nodeCount);
      if (nodeCount > 0)
      {
        nodeList->Item(0, getter_AddRefs(node));
        textNode=do_QueryInterface(node);
        textNode->GetTextContent(strResult);
        _retval = strResult;
        return NS_OK;
      }
    }
  }
  _retval = NS_LITERAL_STRING("");
  return NS_ERROR_FAILURE;
}

/* void FixTagsAfterSplit (in nsIDOMNode firstNode, inout nsIDOMNode secondNode); */
NS_IMETHODIMP msiTagListManager::FixTagsAfterSplit(nsIDOMNode *firstNode, nsIDOMNode **secondNode)
{
#if DEBUG_barry || DEBUG_Barry
//  DebExamineNode(firstNode);
//  DebExamineNode(*secondNode);
  // printf("=====FixTagsAfterSplit=============================================\n");
  // printf("==firstNode:\n");
  // meditor->DumpNode(firstNode);
  // printf("==*secondNode:\n");
  // meditor->DumpNode(*secondNode);
#endif
// find out what the second node should be
  nsString firstNodeName;
  nsIAtom * nsAtomFirst;
  nsString emptyString1;
  nsString emptyString2;
  nsString emptyString3;
  nsresult rv;
  rv = GetTagOfNode(firstNode, &nsAtomFirst, firstNodeName);
  nsAutoString str;
  if (firstNodeName.Length()== 0) return NS_OK;
  nsIAtom * dummyatom = NS_NewAtom(NS_LITERAL_STRING(""));
  rv = GetStringPropertyForTag(firstNodeName, dummyatom, NS_LITERAL_STRING("nexttag"), str);
  if (str.Length() > 0)
  {
    nsCOMPtr<nsIDOMNode> aNewNode; 
    meditor->ReplaceContainer(*secondNode, address_of(aNewNode), str, nsnull, nsnull, PR_TRUE);
    *secondNode = aNewNode;
   	meditor->MarkNodeDirty(*secondNode);
  }
  // Gecko does not display empty paragraphs, so we need to put in a BR, maybe with an attribute
  PRBool isEmpty;
  nsCOMPtr<nsISelection>selection;
  rv = meditor->GetSelection(getter_AddRefs(selection));
  nsCOMPtr<nsIDOMNode> pnode;
  nsHTMLEditor * editor = NS_STATIC_CAST(nsHTMLEditor *,meditor);  
  editor->IsEmptyNode( firstNode, &isEmpty);
  nsCOMPtr<nsIDOMDocument> doc;
  editor->GetDocument(getter_AddRefs(doc));  
  if (isEmpty)
  {
    nsAutoString strContents;
    rv = GetStringPropertyForTag(firstNodeName, dummyatom, NS_LITERAL_STRING("initialcontents"), strContents);
    if (strContents.Length() > 0)
    {
      nsCOMPtr<nsIDOMNode> newNode;
      nsCOMPtr<nsIDOMNode> parentNode;
      firstNode->GetParentNode(getter_AddRefs(parentNode));
      rv = firstNode->CloneNode(PR_FALSE, getter_AddRefs(newNode));
      nsCOMPtr<nsIDOMNSHTMLElement> firstElement(do_QueryInterface(newNode));
      firstElement->SetInnerHTML(strContents);
      NS_ADDREF((nsIDOMNode *)newNode);
      meditor->ReplaceNode(newNode, firstNode, parentNode);
      meditor->MarkNodeDirty(newNode);
      pnode = newNode;
    }
  }
  editor->IsEmptyNode( *secondNode, &isEmpty);
  nsAutoString strContents;
  nsAutoString secondNodeName;
  nsIAtom * nsAtomSecond;
  rv = GetTagOfNode(*secondNode, &nsAtomSecond, secondNodeName);
  if (isEmpty)
  {
    rv = GetStringPropertyForTag(secondNodeName, dummyatom, NS_LITERAL_STRING("initialcontentsforempty"), strContents);
  }
  if (strContents.Length() == 0)
  {
    rv = GetStringPropertyForTag(secondNodeName, dummyatom, NS_LITERAL_STRING("initialcontents"), strContents);
  }
  if (strContents.Length() > 0)
  {
    // to make this operation undoable, we create a new node for and call
    // ReplaceNode, which is transactioned.
    nsCOMPtr<nsIDOMNode> newNode;
    nsCOMPtr<nsIDOMNode> parentNode;
    (*secondNode)->GetParentNode(getter_AddRefs(parentNode));
    rv = (*secondNode)->CloneNode(PR_FALSE, getter_AddRefs(newNode));
    nsCOMPtr<nsIDOMNSHTMLElement> secondElement(do_QueryInterface(newNode));
    secondElement->SetInnerHTML(strContents);
    NS_ADDREF((nsIDOMNode *)newNode);
    meditor->ReplaceNode(newNode, *secondNode, parentNode);
    *secondNode = newNode;
    meditor->MarkNodeDirty(newNode);
  }
  if (!pnode) pnode = *secondNode;
#if DEBUG_barry || DEBUG_Barry
//  editor->DumpNode(pnode,0);
#endif                                                                                                                                        

  nsCOMPtr<nsISelectionPrivate> selPriv(do_QueryInterface(selection));
  if (pnode)
  {
    nsCOMPtr<nsIDOMElement> element;
    nsCOMPtr<nsIDOMNodeList> nodeList;
    nsCOMPtr<nsIDOMNode> node, selNode;
    PRUint32 nodeCount;
    PRInt32 selOffset; 
    element = do_QueryInterface(pnode);
    rv = element->GetElementsByTagName(NS_LITERAL_STRING("cursor"), getter_AddRefs(nodeList));
    if (nodeList) nodeList->GetLength(&nodeCount);
    if (nodeCount > 0)
    {
      nodeList->Item(0, getter_AddRefs(node));
      nsEditor::GetNodeLocation(node, address_of(selNode), &selOffset);
      editor->DeleteNode(node);
      selPriv->SetInterlinePosition(PR_TRUE);
      rv = selection->Collapse(selNode, selOffset);
      rv = selection->Extend( selNode, selOffset+1 );
#if DEBUG_barry || DEBUG_Barry
//      editor->DumpNode(selNode,0);
#endif
      //return NS_OK;
      goto diagnostics;
    }
  }
    
  // now want to set the selection to (outRightNode, 0)
  // Do we want to go down the full depth?
  selPriv->SetInterlinePosition(PR_TRUE);
  rv = selection->Collapse(*secondNode, 0);
diagnostics:
#if DEBUG_barry || DEBUG_Barry
  printf("==Leaving: firstNode: \n");
//  editor->DumpNode(firstNode);
  printf("==*secondNode:\n");
//  editor->DumpNode(*secondNode);
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
