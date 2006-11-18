#ifndef msiTagListManager_h__
#define msiTagListManager_h__

#include "msiITagListManager.h"
#include "nsString.h"
#include "nsCOMPtr.h"
#include "nsHashKeys.h"
#include "nsClassHashtable.h"

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
  nsString initialContentsForEmpty;
  nsString initialContents;
  PRBool discardEmptyBlock;
  TagData()
    : discardEmptyBlock(PR_FALSE)
    {}
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
  TagKeyListHead * pContainsList;
};

msiTagListManager * MSI_NewTagListManager();

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

#endif // msiTagListManager_h__