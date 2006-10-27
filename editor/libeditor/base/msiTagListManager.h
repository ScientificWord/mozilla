#include "msiITagListManager.h"
#include "nsString.h"
#include "nsCOMPtr.h"
#include "nsHashKeys.h"
#include "nsClassHashtable.h"

class nsIDOMXMLDocument;
class nsIDOMElement;
class nsIAutoCompleteSearchStringArray;
struct namespaceLookup;

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
    
protected:  
//  nsStringArray mstrTagInfoPath; // do we save this here?
  nsStringArray* mparentTags;
  nsEditor * meditor; // this is a back pointer, hence not ref counted
  PRBool mInitialized;
  nsCOMPtr<nsIDOMXMLDocument> mdocTagInfo;
  nsCOMPtr<nsIDOMNode> GetChildAt(nsIDOMNode *aParent, PRInt32 aOffset);
  nsresult MergeIntoTagInfo( const nsIDOMXMLDocument* docTagInfo, PRBool *_retval );
  NS_IMETHOD abbrevFromAtom(nsIAtom * atomNS, nsAString & _retval);
  namespaceLookup * plookup;
  nsClassHashtable<nsStringHashKey, TagData> msiTagHashtable;
};

