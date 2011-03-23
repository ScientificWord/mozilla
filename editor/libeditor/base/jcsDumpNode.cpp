#include "jcsDumpNode.h"

#include "nsIDomElement.h"
#include "nsComPtr.h"
#include "nsString.h"
#include "nsIDomNodeList.h"
#include "nsIDomCharacterData.h"
#include "nsIDomDocumentFragment.h"

// This function is a copy of one that is a member of nsEditor. However, (1) this function
// has nothing to do with nsEditor, and (2) having it be a member makes it more difficult to
// call. Hence this duplicate function. jcs


void
DumpNode(nsIDOMNode *aNode, PRInt32 indent, bool recurse /* = false */)
{
  PRInt32 i;
  for (i=0; i<indent; i++)
    printf("  ");

  if (aNode == 0){
    printf("!NULL!\n");
	  return;
  }
  
  nsCOMPtr<nsIDOMElement> element = do_QueryInterface(aNode);
  nsCOMPtr<nsIDOMDocumentFragment> docfrag = do_QueryInterface(aNode);
  
  if (element || docfrag)
  { 
    if (element)
    {
      nsAutoString tag;
      element->GetTagName(tag);
      printf("<%s>    %x\n", NS_LossyConvertUTF16toASCII(tag).get(), aNode);
    }
    else
    {
      printf("<document fragment>\n");
    }
	  if (recurse) {
         nsCOMPtr<nsIDOMNodeList> childList;
         aNode->GetChildNodes(getter_AddRefs(childList));
         if (!childList) return; // NS_ERROR_NULL_POINTER;
         PRUint32 numChildren;
         childList->GetLength(&numChildren);
         nsCOMPtr<nsIDOMNode> child, tmp;
         aNode->GetFirstChild(getter_AddRefs(child));
         for (i=0; i<numChildren; i++)
         {
           DumpNode(child, indent+2, true);
           child->GetNextSibling(getter_AddRefs(tmp));
           child = tmp;
         }
	  }
  }
  else
  {
    nsCOMPtr<nsIDOMCharacterData> textNode = do_QueryInterface(aNode);
    if (!textNode) return;
    nsAutoString str;
    textNode->GetData(str);
    nsCAutoString cstr;
    LossyCopyUTF16toASCII(str, cstr);
    cstr.ReplaceChar('\n', ' ');
    printf("<textnode> %s     %x\n", cstr.get(), aNode);
  }
}
