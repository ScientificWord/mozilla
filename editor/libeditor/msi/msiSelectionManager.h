#ifndef msiSelectionManager_h__
#define msiSelectionManager_h__

#include "nsCOMPtr.h"
#include "nsSelectionState.h"
#include "nsString.h"

class nsISelection;
class nsVoidArray;
class nsIDOMRange;
class msiEditor;


class msiSelectionManager : public nsSelectionState
{
public:
  msiSelectionManager(nsISelection * selection, msiEditor * msiEditor);
  PRUint32 RangeCount();
  nsresult GetRange(PRUint32 index, nsCOMPtr<nsIDOMRange> & range);
  nsRangeStore * GetRangeStoreItem(PRUint32 index);
  nsresult IsRangeCollapsed(PRUint32 index, PRBool &collapsed);
  void     Cleanup();
   
 protected:
  nsresult NormalizeSelection(nsISelection * selection, nsVoidArray & normalizeRangeStore); //ljh TODO -- removed nested ranges, merge intersecting ranges  
  void SetMathmlSurrogates(nsRangeStore * rangeItem);
  nsresult GetParentAndIndexOfChildInParent(nsIDOMNode * child, nsCOMPtr<nsIDOMNode> & parent, PRUint32 &index);
  
  nsresult InitalizeRangeStore(const nsVoidArray & rangeStore);
  nsresult SetNodeIDs(nsRangeStore * rangeItem);
  nsresult ClearNodeIDs(nsRangeStore * rangeItem);
  nsresult Delete(nsVoidArray & rangeData);
   
 public:
   static PRUint32 m_nodeID;  
 protected :
   nsString m_attributeName;
   nsString m_surrogateAttributeName;
   msiEditor * m_msiEditor;
   
};



#endif //msiSelectionManager_h__
