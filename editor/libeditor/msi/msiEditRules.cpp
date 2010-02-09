#include "msiEditRules.h"
#include "nsCOMPtr.h"
#include "nsIServiceManager.h"
#include "nsIRange.h"
#include "msiEditor.h"
#include "msiUtils.h"
#include "nsIDOMNodeList.h"
#include "nsIDOMSerializer.h"
#include "nsIComponentRegistrar.h"
#include "msiISimpleComputeEngine.h"
#include "msiEditingAtoms.h"
#include "nsIDOMText.h"
#include "nsIDocumentEncoder.h"
#include "nsIDOM3Node.h"



void DebExamineNode(nsIDOMNode * aNode);
PRInt32 FindCursorIndex(nsIEditor* editor, nsCOMPtr<nsIDOMNode> node, nsCOMPtr<nsIDOMNode> caretNode, PRInt32 caretOffset, bool& done);







/********************************************************
 *  routine for making new rules instance
 ********************************************************/

nsresult
NS_NewMSIEditRules(nsIEditRules** aInstancePtrResult)
{
  msiEditRules * rules = new msiEditRules();
  if (rules)
    return rules->QueryInterface(NS_GET_IID(nsIEditRules), (void**) aInstancePtrResult);
  return NS_ERROR_OUT_OF_MEMORY;
}





/********************************************************
 *  Constructor/Destructor 
 ********************************************************/

msiEditRules::msiEditRules() :
nsHTMLEditRules()
, mMSIEditor(nsnull)
{
}

msiEditRules::~msiEditRules()
{
}

void DumpSelection( nsISelection * sel);

/********************************************************
 *  XPCOM Cruft
 ********************************************************/

//NS_IMPL_ISUPPORTS1(msiEditRules, nsIEditRules)
NS_IMPL_ADDREF_INHERITED(msiEditRules, nsHTMLEditRules)
NS_IMPL_RELEASE_INHERITED(msiEditRules, nsHTMLEditRules)
NS_IMPL_QUERY_INTERFACE4(msiEditRules, nsIHTMLEditRules, nsIEditRules, nsIEditActionListener, msiIEditActionListenerExtension)

NS_IMETHODIMP
msiEditRules::Init(nsPlaintextEditor *aEditor, PRUint32 aFlags)
{
  mMSIEditor = static_cast<msiEditor*>(aEditor);
  return nsHTMLEditRules::Init(aEditor,aFlags);
}

nsresult
msiEditRules::WillDeleteSelection(nsISelection *aSelection, 
                                  nsIEditor::EDirection aAction, 
                                  PRBool *aCancel,
                                  PRBool *aHandled)
{
  if (!aSelection || !aCancel || !aHandled) { return NS_ERROR_NULL_POINTER; }

  // initialize out params
  *aCancel = PR_FALSE;
  *aHandled = PR_FALSE;

  
  nsCOMPtr<nsIDOMNode> startNode, endNode;
  nsCOMPtr<nsIDOMNode> mathNode;
  nsCOMPtr<nsIDOMElement> mathElement;
  PRInt32 startOffset, endOffset;

  printf("In msiEditRules::WillDeleteSelection\n");
  DumpSelection(aSelection);

  nsresult res = mHTMLEditor->GetStartNodeAndOffset(aSelection, address_of(startNode), &startOffset);
  if (NS_FAILED(res)) return res;
  if (!startNode) return NS_ERROR_FAILURE;
  res = mHTMLEditor->GetEndNodeAndOffset(aSelection, address_of(endNode), &endOffset);
  if (NS_FAILED(res)) return res;
  if (!endNode) return NS_ERROR_FAILURE;

  mMSIEditor->GetMathParent(startNode, mathNode);
  mathElement = do_QueryInterface(mathNode);

  if (mathElement)
      res = WillDeleteMathSelection(aSelection, aAction, aCancel, aHandled);
  else {
     printf("In msiEditRules::WillDeleteSelection(2)\n");
     DumpSelection(aSelection);
     mMSIEditor->AdjustSelectionEnds(PR_TRUE, aAction);
     printf("In msiEditRules::WillDeleteSelection(3)\n");
     DumpSelection(aSelection);
     res = nsHTMLEditRules::WillDeleteSelection(aSelection, aAction, aCancel, aHandled);
  }
  return res;
}  

void DebDisplaySelection(const char* str, nsISelection *aSelection, msiEditor* editor)
{
  printf("%s\n", str);

  nsCOMPtr<nsIDOMNode> startNode, endNode;
  nsCOMPtr<nsIDOMNode> endParent;
  nsCOMPtr<nsIDOMNode> startParent;

  PRInt32 startOffset, endOffset;
  
  editor->GetStartNodeAndOffset(aSelection, address_of(startNode), &startOffset);
  editor->GetEndNodeAndOffset(aSelection, address_of(endNode), &endOffset);
  startNode->GetParentNode(getter_AddRefs(startParent));
  endNode->GetParentNode(getter_AddRefs(endParent));

  printf("===startNode: ");
  editor->DumpNode(startNode);
  DebExamineNode(startNode);

  printf("===start parent node: ");
  editor->DumpNode(startParent);
  DebExamineNode(startParent);

  
  printf("===endNode: ");
  editor->DumpNode(endNode);
  DebExamineNode(endNode);

  printf("===end parent node: ");
  editor->DumpNode(endParent);
  DebExamineNode(endParent);



}



nsresult msiEditRules::WillDeleteMathSelection(nsISelection *aSelection, 
                                 nsIEditor::EDirection aAction, 
                                 PRBool *aCancel,
                                 PRBool *aHandled)
{
  PRBool bCollapsed;
  nsresult res = aSelection->GetIsCollapsed(&bCollapsed);
  if (NS_FAILED(res)) return res;

  nsCOMPtr<nsIDOMNode> startNode, endNode;
  nsCOMPtr<nsIDOMNode> mathNode;
  nsCOMPtr<nsIDOMElement> mathElement;
  PRInt32 startOffset, endOffset;
  
  res = mHTMLEditor->GetStartNodeAndOffset(aSelection, address_of(startNode), &startOffset);
  if (NS_FAILED(res)) return res;
  if (!startNode) return NS_ERROR_FAILURE;

  res = mHTMLEditor->GetEndNodeAndOffset(aSelection, address_of(endNode), &endOffset);
  if (NS_FAILED(res)) return res;
  if (!endNode) return NS_ERROR_FAILURE;

  mMSIEditor->GetMathParent(startNode, mathNode);
  mathElement = do_QueryInterface(mathNode);

  if (!mathElement) return NS_ERROR_FAILURE;

  DebDisplaySelection("Initial", aSelection, mMSIEditor);


  if (bCollapsed) {
      // const nsAFlatString& empty = EmptyString();
	  // mMSIEditor -> InsertHTMLWithContext(NS_LITERAL_STRING("45"), empty, empty, empty,
      //                         nsnull, endNode,  endOffset, PR_FALSE);
      
	  nsCOMPtr<msiIMathMLEditingBC> editingBC; 
      PRUint32 dontcare(0);
	  PRUint32 mathmltype;
	  
	  msiUtils::GetMathMLEditingBC(mHTMLEditor, endNode, dontcare, editingBC);
      if (editingBC) {
	    mathmltype = msiUtils::GetMathmlNodeType(editingBC);
	  }
	  
	  if (mathmltype ==  msiIMathMLEditingBC::MATHML_MATH){
	     if (! msiUtils::IsEmpty(endNode)){
		     nsCOMPtr<nsIDOMNodeList> children;
			 PRUint32 number;
		     nsCOMPtr<nsIDOMNode> rightmostChild;
		     endNode->GetChildNodes(getter_AddRefs(children));
			 msiUtils::GetNumberofChildren(endNode, number);
			 msiUtils::GetChildNode(endNode, number-1, rightmostChild);

             endNode = rightmostChild;             

		     msiUtils::GetMathMLEditingBC(mHTMLEditor, endNode, dontcare, editingBC);
             if (editingBC) {
	           mathmltype = msiUtils::GetMathmlNodeType(editingBC);
	         }

		 }
      }

      if (mathmltype ==  msiIMathMLEditingBC::MATHML_MROWFENCE){      
			 // Remove the first and last children of the mrow -- they should be fences.
             nsCOMPtr<nsIDOMNodeList> children;
			 PRUint32 number;

			 endNode->GetChildNodes(getter_AddRefs(children));
			 msiUtils::GetNumberofChildren(endNode, number);
			 
			 			 
			 nsCOMPtr<nsIDOMNode>  removedChild;
			 res = msiUtils::RemoveIndexedChildNode(mHTMLEditor, endNode, number-1, removedChild);
			 res = msiUtils::RemoveIndexedChildNode(mHTMLEditor, endNode, 0, removedChild);

			 if (NS_FAILED(res)) return res;
			 
			 // Remove the mrow container 
		     res = mHTMLEditor->RemoveContainer(endNode);
			 
			 if (NS_FAILED(res)) return res;

			 *aHandled = PR_TRUE;

			

	  } else if ( mathmltype == msiIMathMLEditingBC::MATHML_MSQRT ) {
			 // Remove the msqrt container 
		     res = mHTMLEditor->RemoveContainer(endNode);
			 *aHandled = PR_TRUE;
			 

	  } else if ( mathmltype == msiIMathMLEditingBC::MATHML_MSUP || mathmltype == msiIMathMLEditingBC::MATHML_MSUB) {
	         // remove the superscript	or subscript
	         nsCOMPtr<nsIDOMNode>  removedChild;
			 res = msiUtils::RemoveIndexedChildNode(mHTMLEditor, endNode, 1, removedChild);
			 
			 // Remove the msup (or msub) container 
		     res = mHTMLEditor->RemoveContainer(endNode);
			 *aHandled = PR_TRUE;
			 
	  }  else if ( mathmltype == msiIMathMLEditingBC::MATHML_MSUBSUP ) {
	         // remove the superscript	and subscript
	         nsCOMPtr<nsIDOMNode>  removedChild;
			 //res = msiUtils::RemoveChildNode(endNode, 2, removedChild);
			 //res = msiUtils::RemoveChildNode(endNode, 1, removedChild);
			 res = msiUtils::RemoveIndexedChildNode(mHTMLEditor, endNode, 2, removedChild);
			 res = msiUtils::RemoveIndexedChildNode(mHTMLEditor, endNode, 1, removedChild);
			 // Remove the msubsup container 
		     res = mHTMLEditor->RemoveContainer(endNode);
			 *aHandled = PR_TRUE;


	  } else {
      	  aSelection->Extend( endNode, endOffset-1 );
		  //mHTMLEditor->DeleteSelection(aAction);
	  }
  }

   
  if (!*aHandled){

     DebDisplaySelection("Not handled" , aSelection, mMSIEditor);
       
     nsCOMPtr<nsIDOMNode> firstChildNode;
     nsCOMPtr<nsIDOMNode> lastChildNode;
     nsCOMPtr<nsIDOMRange> range;
     PRBool partlyContained = PR_FALSE;
     PRBool containsNode;

     	
     mathElement->GetFirstChild(getter_AddRefs(firstChildNode));
     mathElement->GetLastChild(getter_AddRefs(lastChildNode));
     aSelection->ContainsNode(firstChildNode, partlyContained, &containsNode);
     if (containsNode)
     {
       aSelection->ContainsNode(lastChildNode, partlyContained, &containsNode);
       if (containsNode) // all children of the math node are in the selection
       {
         aSelection->GetRangeAt(0, getter_AddRefs(range));
         range->SelectNode(mathNode);
       }
     }
     	
     mathElement->GetFirstChild(getter_AddRefs(firstChildNode));
     mathElement->GetLastChild(getter_AddRefs(lastChildNode));
     aSelection->ContainsNode(firstChildNode, partlyContained, &containsNode);
     if (containsNode)
     {
       aSelection->ContainsNode(lastChildNode, partlyContained, &containsNode);
       if (containsNode) // all children of the math node are in the selection
       {
         aSelection->GetRangeAt(0, getter_AddRefs(range));
         range->SelectNode(mathNode);
       }
     }

     mMSIEditor->AdjustSelectionEnds(PR_TRUE, aAction);
	 DebDisplaySelection("Pre-will delete" , aSelection, mMSIEditor);
     nsHTMLEditRules::WillDeleteSelection(aSelection, aAction, aCancel, aHandled);
	 DebDisplaySelection("Post-will delete" , aSelection, mMSIEditor);
	 mMSIEditor->AdjustSelectionEnds(PR_TRUE, aAction);

  }
  
   
  mMSIEditor->AdjustSelectionEnds(PR_TRUE, aAction);
  res = mHTMLEditor->GetEndNodeAndOffset(aSelection, address_of(endNode), &endOffset);

  bool b = false;
  int i = FindCursorIndex(mMSIEditor, mathNode, endNode, endOffset, b);
  nsCOMPtr<nsIDOMSerializer> ds = do_CreateInstance(NS_XMLSERIALIZER_CONTRACTID);
  NS_ENSURE_STATE(ds);

  nsString text;
  nsresult rv = ds->SerializeToString(mathNode, text);
  NS_ENSURE_SUCCESS(rv, rv);
  
  nsCOMPtr<nsIProperties> fileLocator(do_GetService("@mozilla.org/file/directory_service;1"));
  nsCOMPtr<nsILocalFile> iniFile;
  fileLocator->Get("resource:app", NS_GET_IID(nsIFile), getter_AddRefs(iniFile));
  iniFile->Append(NS_LITERAL_STRING("mupInstall.gmr"));

  nsCOMPtr<msiISimpleComputeEngine> engine(do_GetService("@mackichan.com/simplecomputeengine;2"));	
  engine->Startup(iniFile);

  PRUnichar* result;
  const PRUnichar* inp;
  text.GetData(&inp);

  printf("\nSending for cleanup: %ls\n" , inp);
  engine->Perform(inp, 152, &result); // Cleanup function
  printf("\nBack from cleanup: %ls\n", result);

  mHTMLEditor->DeleteNode(mathElement);

  DebDisplaySelection("Pre-result insertion", aSelection, mMSIEditor);
  
  mMSIEditor -> InsertHTML(nsString(result, wcslen(result)));
  // Need to place cursor.
  *aHandled = PR_TRUE;

  return res;
}



PRInt32 FindCursorIndex(nsIEditor* editor, nsCOMPtr<nsIDOMNode> node, nsCOMPtr<nsIDOMNode> caretNode, PRInt32 caretOffset, bool& done)
{
  if (msiUtils::IsMleaf(editor, node, true)){
      if (caretNode == node){
	      done = true;
	      return caretOffset;
	  } else { 
          nsString str;
          nsCOMPtr<nsIDOM3Node> m;
          m = do_QueryInterface(node);
          m->GetTextContent(str);
	      return str.Length();
	}
  }	else {

      nsCOMPtr<nsIDOMNodeList> children;
	  PRUint32 number;
	  nsCOMPtr<nsIDOMNode> child;

      node->GetChildNodes(getter_AddRefs(children));
	  msiUtils::GetNumberofChildren(node, number);
	  
	  int lastChildIndex = number;

	  if (node == caretNode){
	     lastChildIndex = caretOffset;
	  }

	  int count = 0;

	  for (int i = 0; i < lastChildIndex; ++i){
	    msiUtils::GetChildNode(node, i, child);
		count += FindCursorIndex(editor, child, caretNode, caretOffset, done);
		if (done)
		  break;
	  }
	  return count;

  }
      
}


//ljh TODO -- determine what if any functionality is needed for these two functions.
NS_IMETHODIMP 
msiEditRules::WillReplaceNode(nsIDOMNode *aNewNode, nsIDOMNode *aOldChild, nsIDOMNode *aParent)
{
  return NS_OK;  
}


NS_IMETHODIMP 
msiEditRules::DidReplaceNode(nsIDOMNode *aNewNode, 
                             nsIDOMNode *aOldNode, 
                             nsIDOMNode *aParent, 
                             nsresult    aResult)
{
 return NS_OK; //nsHTMLEditRules::DidReplaceNode(aNewNode,aOldNode,aParent,aResult);
}

NS_IMETHODIMP 
msiEditRules::DidDeleteNode(nsIDOMNode *aChild, nsresult aResult)
{
  return nsHTMLEditRules::DidDeleteNode(aChild, aResult);  
}


NS_IMETHODIMP 
msiEditRules::DidSplitNode(nsIDOMNode *aExistingRightNode, 
                              PRInt32 aOffset, 
                              nsIDOMNode *aNewLeftNode, 
                              nsresult aResult)
{
//  printf("msiEditRules:DidSplitNode\n");
 return nsHTMLEditRules::DidSplitNode(aExistingRightNode,aOffset,aNewLeftNode,aResult);
}


NS_IMETHODIMP 
msiEditRules::DidDeleteText(nsIDOMCharacterData *aTextNode, 
                                  PRInt32 aOffset, 
                                  PRInt32 aLength, 
                                  nsresult aResult)
{
  return nsHTMLEditRules::DidDeleteText(aTextNode, aOffset, aLength, aResult);  
}


NS_IMETHODIMP
msiEditRules::DidDeleteRange(nsIDOMRange *aRange)
{
  return nsHTMLEditRules::DidDeleteRange(aRange);
}


NS_IMETHODIMP
msiEditRules::DidDeleteSelection(nsISelection *aSelection)
{
  return nsHTMLEditRules::DidDeleteSelection(aSelection);
}

