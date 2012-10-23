#ifndef msiTagListManager_h__
#define msiTagListManager_h__

#include "msiITagListManager.h"
#include "nsString.h"
#include "nsCOMPtr.h"
#include "nsHashKeys.h"
#include "nsClassHashtable.h"
#include "nsComponentManagerUtils.h"

class nsIDOMXMLDocument;
class nsIDOMElement;
class nsIAutoCompleteSearchStringArray;
struct namespaceLookup;
struct TagKey;
struct TagKeyList;
struct TagKeyListHead;

class TagData // other fields can be added to this
{
public:
  nsString description;
  nsString tagClass;
  nsString realTagClass;
  nsString nextAfterEmptyBlock;
  nsString titleTag;
  nsString nextTag;
  nsString level;
  nsString htmllistparent;
  nsString htmllist;
  nsString prefsub;
  nsString babel;
  nsString mathonly;
  nsCOMPtr<nsIDOMElement> initialContents;
  PRBool   wrapper;
  PRBool   discardEmptyBlock;
  PRBool   inclusion;
  PRBool   hidden;
  TagData()
    : discardEmptyBlock(PR_FALSE), inclusion(PR_FALSE)
    {}
};


struct TagKey // a class castable to and from strings that supports pulling out the tag name and name space atom
{
  nsString key;  // example: sw:sectiontitle
  operator nsString() { return key; }
  TagKey( const nsString akey, const nsString nsAbbrev) { key = (nsAbbrev.Length()?nsAbbrev + NS_LITERAL_STRING(":") + akey:akey);}
  TagKey( const nsString akey);
  TagKey( ){}
  ~TagKey() {/* todo */};
  nsString altForm(); // if the key is 'sw:sectiontitle', then the altForm is 'sectiontitle - sw' (notice the spaces)
  nsString prefix();
  nsString localName();
};


class msiTagListManager : public msiITagListManager
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_MSIITAGLISTMANAGER

  msiTagListManager();
  ~msiTagListManager();
  nsString GetStringProperty( const nsAString & str, nsIDOMElement * element);
  PRBool BuildHashTables(nsIDOMXMLDocument * docTagInfo, PRBool *_retval);
  void BuildContainsListForElement(nsIDOMElement * element, const nsAString & name);
  void BuildBabelList(nsIDOMXMLDocument * docTagInfo, TagKeyListHead ** ppBabelList);
  PRBool ContainsListForOuterIncludesInner( const nsAString & strOuter, const nsAString & strInner );


  nsCOMPtr<nsIAutoCompleteSearchStringArray> pACSSA;
  static nsIAtom * htmlnsAtom; // an atom corresponding to  "http://www.w3.org/1999/xhtml"
protected:
  nsStringArray* mparentTags;
  nsEditor * meditor; // this is a back pointer, hence not ref counted
  PRBool mInitialized;
  nsCOMPtr<nsIDOMXMLDocument> mdocTagInfo;
  nsCOMPtr<nsIDOMNode> GetChildAt(nsIDOMNode *aParent, PRInt32 aOffset);
  nsresult MergeIntoTagInfo( const nsIDOMXMLDocument* docTagInfo, PRBool *_retval );
  nsString PrefixFromNameSpaceAtom(nsIAtom * atomNS);
  namespaceLookup * plookup;
  nsClassHashtable<nsStringHashKey, TagData> msiTagHashtable;
  TagKey mdefaultParagraph;
  TagKey mclearTextTag;
  TagKey mclearStructTag;
  TagKey mclearEnvTag;
  TagKey mclearListTag;
  TagKeyListHead * pBabelList; // tags that Babel or Polyglossia need to change at run time.
  TagKeyListHead * pContainsList;
};

msiTagListManager * MSI_NewTagListManager();
struct TagKeyList
{
  TagKey key;
  TagKeyList * pNext;
  TagKeyList(): pNext(nsnull) {}
  ~TagKeyList();
};

struct TagKeyListHead
{
  TagKeyList *pListHead;
  TagKeyList *pListTail;
  nsString name;
  TagKeyListHead * pNext;
  TagKeyListHead(): pListHead(nsnull), pListTail(nsnull), pNext(nsnull) {}
  ~TagKeyListHead();
};


/* #define USE_NAMESPACES 1 -- we start out not using name spaces */

#endif // msiTagListManager_h__
