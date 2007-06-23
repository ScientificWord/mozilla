#include "msiIAutosub.h"
#include "nsCOMPtr.h"
#include "nsString.h"

struct autosubentry
{
  nsString pattern;  // the patterns in the autosubstitute list entered backwards
  PRUint32 context;
  PRUint32 action;
  nsString data;
  nsString pastecontext;
  nsString pasteinfo;
  
  autosubentry( nsString p, PRUint32 c, PRUint32 a, nsString d, nsString pastectx, nsString info)
	  : pattern(p), context(c), action(a), data(d), pastecontext(pastectx), pasteinfo(info) {}
  autosubentry() : pattern(NS_LITERAL_STRING("")), context(msiIAutosub::CONTEXT_TEXTONLY), 
    action(msiIAutosub::ACTION_SUBSTITUTE), data(NS_LITERAL_STRING("")), pastecontext(NS_LITERAL_STRING("")), pasteinfo(NS_LITERAL_STRING("")) {}
// Comparison functions for sorting
  PRBool operator< ( autosubentry& b);
  PRBool operator< ( nsString & b);
  PRBool operator== ( autosubentry& b);
  PRBool operator== ( nsString & b);
  PRBool operator> ( autosubentry& b);
  PRBool operator> ( nsString & b);
// A slightly different function to pass to Quicksort
};

PR_BEGIN_EXTERN_C
  int compare( const autosubentry * a, const autosubentry * b, void* notused);
PR_END_EXTERN_C

class msiAutosub : public msiIAutosub
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_MSIIAUTOSUB
  static msiAutosub* GetInstance();
  msiAutosub();

private:
  autosubentry* autosubarray; // an array of autosubentries sorted by pattern in increasing order
  nsString patternSoFar;
  PRUint32 state;
  PRUint32 startIndex; // index into the array of the first entry that starts with patternSoFar
        // (when state == matchesSoFar or success). It is -1 when state == init
  PRUint32 lastIndex;  // an index pointing to the last possible position of patternSoFar
  PRUint32 lastSuccessIndex;
  nsCString mFileURI;  // the file the list was loaded from
  PRUint32 arraylength;
  ~msiAutosub();

protected:
  /* additional members */
};
