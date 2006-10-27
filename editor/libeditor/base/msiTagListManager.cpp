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

msiTagListManager::msiTagListManager()
:  meditor(nsnull), mparentTags(nsnull), mInitialized(PR_FALSE), plookup(nsnull) 
{
  nsresult rv;
  pACSSA = do_CreateInstance("@mozilla.org/autocomplete/search;1?name=stringarray", &rv);
  // Now replace the singleton autocompletesearchstringarry by an implementation
  pACSSA->GetNewImplementation(getter_AddRefs(pACSSA));
  Reset();
}

struct namespaceLookup
{
  nsString namespaceAbbrev;
  nsIAtom * nsAtom;
  namespaceLookup * next;
  // BBM todo: do we need destructor code here?
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


class TagKey // a class castable to and from strings that supports pulling out the tag name and name space atom
{
public:
  nsString key;  // example: sw:sectiontitle
  operator nsString() { return key; }
  TagKey( nsString akey, nsString nsAbbrev) { key = nsAbbrev + NS_LITERAL_STRING(":") + akey;}
  TagKey( nsString akey) :key(akey) {}
  TagKey( ){}
  ~TagKey() {/* todo */};
  nsString altForm(); // if the key is 'sw:sectiontitle', then the altForm is 'sectiontitle(sw)'
  nsIAtom * atomNS();
  nsString localName();
};

nsIAtom * 
TagKey::atomNS()
{
  int colon = key.FindChar(':');
  if (colon < 0) return nsnull;
  nsString str;
  key.Left(str,colon);
  return NS_NewAtom(str);
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
    return localName()+ NS_LITERAL_STRING(" - ") + str;
  }
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
      while (pns)
      {
        if (pns->namespaceAbbrev != strAbbrev)
          pns = pns->next;
        else
          break;
      }
      if (pns == nsnull) // strAbbrev was not found
      {
        pns = new namespaceLookup;
        textNode=do_QueryInterface(node);
        textNode->GetTextContent(strNameSpace);
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
  nsAutoString str2;
  nsAutoString strClassName;
  nsCOMPtr<nsIDOMElement> tagNameElement;   
  TagKey key;
  // BBM todo: get additional namespace declarations
  rv = dti->GetElementsByTagName(NS_LITERAL_STRING("tagclass"), getter_AddRefs(tagClasses));
  if (tagClasses) tagClasses->GetLength(&tagClassCount);
  if (tagClassCount > 0)
  {
    for (PRUint32 i = 0; i < tagClassCount; i++)
    {
      tagClasses->Item(i, (nsIDOMNode **) getter_AddRefs(tagClass));
      tagClassElement = do_QueryInterface(tagClass);
      rv = tagClassElement->GetElementsByTagName(NS_LITERAL_STRING("tagclassname"), getter_AddRefs(tagClassNames));
      if (tagClassNames) tagClassNames->Item(0, getter_AddRefs(tagClassName));
      tagClassNameNode = do_QueryInterface(tagClassName);
      tagClassNameElement = do_QueryInterface(tagClassName);
      rv = tagClassNameNode->GetTextContent(strClassName);
      rv = tagClassElement->GetElementsByTagName(NS_LITERAL_STRING("tag"), getter_AddRefs(tagNames));
//      printf("\n");
      if (tagNames) 
      {
        rv = tagNames->GetLength(&tagNameCount);
        for (PRUint32 j = 0; j < tagNameCount; j++)
        {
          tagNames->Item(j, getter_AddRefs(tagName));
          tagNameElement = do_QueryInterface(tagName);
          rv = tagNameElement->GetAttribute(NS_LITERAL_STRING("nm"),strName);
          rv = tagNameElement->GetAttribute(NS_LITERAL_STRING("ns"),str2);
          key = TagKey(strName, str2);
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
{  // this is a callback to the hash table enumerator that adds to a string array all the data that matches the ... part of *userArg
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
// BBM todo:  We don't really want a string array. We want an array of classes containing tag name, description, and maybe some more.
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
msiTagListManager::FindParentTags(nsStringArray **strarrParents)
{
  // BBM todo: handle namespaces here
  nsresult res;
  nsAutoString strTemp;
  *strarrParents = nsnull;
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
    node->GetNodeName(strTemp);
	  if (strTemp.Equals(NS_LITERAL_STRING("#document"))) break;
	  mparentTags->AppendString(strTemp);
    node->GetParentNode((nsIDOMNode **)(&temp));
    node = temp;
  }
  *strarrParents = mparentTags;
  return  NS_OK;
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
  PRBool isInList;
  _retval = EmptyString();
  
  if (!meditor)
    return NS_ERROR_NULL_POINTER;
  nsHTMLEditor * editor = NS_STATIC_CAST(nsHTMLEditor *, meditor);  
  nsCOMPtr<nsIDOMElement> element;
   nsCOMPtr<nsIDOMNode> node;
  nsCOMPtr<nsIDOMNode> node2;
  nsCOMPtr<nsIDOMNode> temp;
  nsCOMPtr<nsISelection> selection;
  nsCOMPtr<nsIDOMRange> range;
  PRInt32 startoffset = 0;
  PRInt32 endoffset = 0;
  // if the selection is a node and its contents, GetSelectionContainer doesn't return the node, but the parent.
  // Thus we have the next few lines of code
  res = meditor->GetSelection(getter_AddRefs(selection));
  if (NS_FAILED(res)) { return res; } 
  PRInt32 rangeCount;
  res = selection->GetRangeCount(&rangeCount);
  if (NS_FAILED(res)) { return res; }
  PRBool success = PR_TRUE;
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
  
  if (!success)
  {
    res = editor->GetSelectionContainer(getter_AddRefs(element));
    node = element;
  }
  while (node)
  {
    node->GetLocalName(strTemp);
	  if (strTemp.Equals(NS_LITERAL_STRING("#document"))) break;
    GetTagInClass(strTagClass, strTemp, *atomNS, &isInList);
    if (isInList)
    {
      _retval = strTemp;
      node->GetNamespaceURI(strURI);
      *atomNS = NS_NewAtom(strURI);
      return NS_OK;
    }
    node->GetParentNode((nsIDOMNode **)(&temp));
    node = temp;
  }
  _retval = EmptyString();
  *atomNS = nsnull;
  return NS_OK;
}

NS_IMETHODIMP msiTagListManager::abbrevFromAtom(nsIAtom * atomNS, nsAString &_retval)
{
  namespaceLookup * pns = plookup;
  while (pns)
  {
    if (pns->nsAtom == atomNS)
    {
      _retval = pns->namespaceAbbrev;
      return NS_OK;
    }
    pns = pns->next;
  }
  _retval = EmptyString();
  return NS_OK;
}


/* AString getClassOfTag (in AString strTag, in nsIAtom atomNS); */
NS_IMETHODIMP msiTagListManager::GetClassOfTag(const nsAString & strTag, nsIAtom *atomNS, nsAString & _retval)
{
  // If *atomNS is null, we just use strTag to look up. Otherwise we concat the namespace abbreviation with strTag
  nsString strAbbrev;
  abbrevFromAtom(atomNS, strAbbrev);
  nsAutoString strKey = strAbbrev + NS_LITERAL_STRING(":") + strTag; 
  TagData data;               
  if (msiTagHashtable.Get(strKey, (nsClassHashtable<nsStringHashKey, TagData>::UserDataType *)&data))
  {
    _retval = data.tagClass;
    return NS_OK; 
  }
  _retval = EmptyString();
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

// BBM todo: Keep 'contains' information in a set of lists
/* PRBool TagCanContainTag (in string strOuter, in string strOInner);  
Check the current tag info DOM to see if the class of tag(strOuter) has a 'Contains' node with a string 
that includes, as a token, strInner or class(strInner).
*/
/* PRBool tagCanContainTag (in AString strTagOuter, in nsIAtom atomNSOuter, in AString strTagInner, in nsIAtom atomNSInner); */
NS_IMETHODIMP msiTagListManager::TagCanContainTag(const nsAString & strTagOuter, nsIAtom *atomNSOuter, const nsAString & strTagInner, nsIAtom *atomNSInner, PRBool *_retval)
{
  nsAutoString classOuter;
  nsresult rv = GetClassOfTag(strTagOuter, atomNSOuter, classOuter);
  nsAutoString classInner;
  rv = GetClassOfTag(strTagInner, atomNSInner, classInner);
  *_retval = PR_FALSE;
  if (strTagOuter.Equals(NS_LITERAL_STRING("body")))
  {
    *_retval = PR_TRUE;
    return NS_OK;
  }
  // TODO: also check atoms
  if (classOuter.Equals(classInner) && classOuter.Equals(NS_LITERAL_STRING("structtag")))
  {
    // both tags are structure tags, and so we need to look at the levels. We return true if the outer level or the inner level
    // is '*', or if the outer level is strictly less than the inner level.
    return LevelCanContainLevel( strTagOuter, atomNSOuter, strTagInner, atomNSInner, _retval);
  }
  else
  {
  
    //Find the 'Contains' node, if any, for the Outer Class
    nsCOMPtr<nsIDOMNodeList> tagClasses;
    nsCOMPtr<nsIDOMNode> tagClass;
    nsCOMPtr<nsIDOMElement> tagClassElement;
    nsCOMPtr<nsIDOMNodeList> tagClassNames;
    nsCOMPtr<nsIDOMNode> tagClassName;
    nsCOMPtr<nsIDOMElement> tagClassNameElement;
    nsCOMPtr<nsIDOM3Node> tagClassNameNode;
    nsCOMPtr<nsIDOMNodeList> tagContainsList;
    nsCOMPtr<nsIDOMNode> tagContains;
    nsAutoString strTemp;
    PRUint32 tagClassCount, tagContainsCount;
    nsCOMPtr<nsIDOM3Node> tagContainsNode;
    rv = mdocTagInfo->GetElementsByTagName(NS_LITERAL_STRING("tagclass"), getter_AddRefs(tagClasses));
    if (tagClasses) tagClasses->GetLength(&tagClassCount);
    if (tagClassCount > 0)
    {
      for (PRUint32 i = 0; i < tagClassCount; i++)
      {
        tagClasses->Item(i, (nsIDOMNode **) getter_AddRefs(tagClass));
        tagClassElement = do_QueryInterface(tagClass);
        rv = tagClassElement->GetElementsByTagName(NS_LITERAL_STRING("tagclassname"), getter_AddRefs(tagClassNames));
        if (tagClassNames) tagClassNames->Item(0, getter_AddRefs(tagClassName));
        tagClassNameNode = do_QueryInterface(tagClassName);
        tagClassNameElement = do_QueryInterface(tagClassName);
        rv = tagClassNameNode->GetTextContent(strTemp);
        if (strTemp.Equals(classOuter))
        {
          rv = tagClassElement->GetElementsByTagName(NS_LITERAL_STRING("contains"), getter_AddRefs(tagContainsList));
          if (tagContainsList) tagContainsList->GetLength(&tagContainsCount);
          if (tagContainsCount > 0)
          {
            for (PRUint32 j = 0; j < tagContainsCount; j++)
            {tagContainsList->Item(j, getter_AddRefs(tagContains));
              tagContainsNode = do_QueryInterface(tagContains);
              rv = tagContainsNode->GetTextContent(strTemp);
              if (strTemp.Equals(strTagInner) || strTemp.Equals(classInner))
              {
                * _retval = PR_TRUE;
                break;
              }
            }    
            break;
          }
        }
      }
    }
    return NS_OK; 
  } 
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
  _retval = EmptyString();
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
