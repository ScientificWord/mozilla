#include "msiEditRules.h"
#include "nsCOMPtr.h"
#include "nsString.h"
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
//PRInt32 FindCursorIndex(nsHTMLEditor* editor, nsIDOMNode* node, nsIDOMNode* caretNode, PRInt32 caretOffset, bool& done, PRInt32 level);
//void FindCursorNodeAndOffset(nsHTMLEditor* editor, nsIDOMNode* node, PRInt32& charCount, nsCOMPtr<nsIDOMNode>& theNode, PRInt32& theOffset);

//nsCOMPtr<msiISimpleComputeEngine> GetEngine();
//nsString SerializeMathNode(nsCOMPtr<nsIDOMNode> mathNode);

PRBool bSelectionContainsTheEntireMathNode(nsIDOMNode* aNode, nsISelection* aSelection); 







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
msiEditRules::WillInsert(nsISelection *aSelection, PRBool *aCancel)
{
        if (mBogusNode)
        {
          mEditor->DeleteNode(mBogusNode);
          mBogusNode = nsnull;
        }
  
  nsresult res = nsHTMLEditRules::WillInsert(aSelection, aCancel);
  if (NS_FAILED(res)) return res;

  // See if we're in math

  return res;

} 


nsresult
msiEditRules::WillDeleteSelection(nsISelection *aSelection, 
                                  nsIEditor::EDirection aAction, 
                                  PRBool *aCancel,
                                  PRBool *aHandled)
{
  if (!aSelection || !aCancel || !aHandled) { return NS_ERROR_NULL_POINTER; }

  // printf("In msiEditRules::WillDeleteSelection\n");
  //DumpSelection(aSelection);


  // initialize out params
  *aCancel = PR_FALSE;
  *aHandled = PR_FALSE;

  
  nsCOMPtr<nsIDOMNode> startNode, endNode;
  nsIDOMNode * sn;
  nsIDOMNode * en;
  nsCOMPtr<nsIDOMNode> theNode;
  nsCOMPtr<nsIDOMNodeList> kids;
  nsCOMPtr<msiITagListManager> tlmgr;
  PRUint32 childcount;
  nsAutoString name;
  nsAutoString className;
  // nsCOMPtr<nsIDOMNode> startMathNode;
  // nsCOMPtr<nsIDOMNode> endMathNode;
  // PRBool endsInMath;
  PRInt32 startOffset, endOffset;
  nsIAtom * pAtom = nsnull;

  nsresult res;
  
  // If the selection consists of a single node which is a front matter node, and
  // if the node is not empty, change the selection to the contents of that node.
  mHTMLEditor->GetStartNodeAndOffset(aSelection, getter_AddRefs(startNode), &startOffset);
  sn = startNode;
  NS_ADDREF(sn);

  mHTMLEditor->GetEndNodeAndOffset(aSelection, getter_AddRefs(endNode), &endOffset);
  en = endNode;
  NS_ADDREF(en);
  if ((startNode == endNode) && ((startOffset+1) == endOffset))
  {
    msiUtils::GetChildNode(endNode, startOffset, theNode);
    if (! msiUtils::IsEmpty(theNode))
    {
      { 
        mHTMLEditor->GetTagListManager((msiITagListManager**)getter_AddRefs(tlmgr));
        tlmgr->GetTagOfNode(theNode,&pAtom, name);
        tlmgr->GetClassOfTag(name, pAtom, className);
        if (className.EqualsLiteral("frontmtag"))
        {
          msiUtils::GetNumberofChildren(theNode, childcount);
          aSelection->Collapse(theNode,0);
          aSelection->Extend(theNode, childcount);
        }
      }
    }
  }

	nsCOMPtr<nsIDOMElement> cell;
	res = mHTMLEditor->GetFirstSelectedCell(nsnull, getter_AddRefs(cell));
	if (NS_SUCCEEDED(res) && cell)
	{
	  res = mHTMLEditor->DeleteTableCellsForDeleteKey();
	  *aHandled = PR_TRUE;
	  return res;
	}
	mMSIEditor->AdjustSelectionEnds(PR_TRUE, aAction);
	res = nsHTMLEditRules::WillDeleteSelection(aSelection, aAction, aCancel, aHandled);
  return res;
}



void DebDisplaySelection(const char* str, nsISelection *aSelection, msiEditor* editor, bool recurse = false)
{
  printf("%s\n", str);

  nsCOMPtr<nsIDOMNode> startNode, endNode;
  nsCOMPtr<nsIDOMNode> endParent;
  nsCOMPtr<nsIDOMNode> startParent;

  PRInt32 startOffset, endOffset;
  
  editor->GetStartNodeAndOffset(aSelection, getter_AddRefs(startNode), &startOffset);
  editor->GetEndNodeAndOffset(aSelection, getter_AddRefs(endNode), &endOffset);
  startNode->GetParentNode(getter_AddRefs(startParent));
  endNode->GetParentNode(getter_AddRefs(endParent));

  printf("===start parent node: ");
  editor->DumpNode(startParent);
  
  printf("===startNode: ");
  editor->DumpNode(startNode, 0, recurse);
  printf("   start offset = %d", startOffset);

  if (startNode != endNode) {
     printf("===end parent node: ");
     editor->DumpNode(endParent);

     printf("===endNode: ");
     editor->DumpNode(endNode);
  }
  printf("   end offset = %d\n", endOffset);

}

/*

nsresult msiEditRules::WillDeleteMathSelection(nsISelection *aSelection, 
                                 nsIEditor::EDirection aAction, 
                                 PRBool *aCancel,
                                 PRBool *aHandled)
{
  // initialize out params
  *aCancel = PR_FALSE;
  *aHandled = PR_FALSE;

  PRBool bCollapsed;
  nsresult res = aSelection->GetIsCollapsed(&bCollapsed);
  if (NS_FAILED(res)) return res;

  nsCOMPtr<nsIDOMNode> startNode, endNode;
  nsCOMPtr<nsIDOMNode> startMathNode;
  nsCOMPtr<nsIDOMNode> endMathNode;
  nsCOMPtr<nsIDOMElement> mathElement;
  PRInt32 startOffset, endOffset;
  PRBool bDeleteEntireMath = false;
  
  res = mHTMLEditor->GetStartNodeAndOffset(aSelection, getter_AddRefs(startNode), &startOffset);
  if (NS_FAILED(res)) return res;
  if (!startNode) return NS_ERROR_FAILURE;

  res = mHTMLEditor->GetEndNodeAndOffset(aSelection, getter_AddRefs(endNode), &endOffset);
  if (NS_FAILED(res)) return res;
  if (!endNode) return NS_ERROR_FAILURE;

  mMSIEditor->GetMathParent(startNode, startMathNode);
  mMSIEditor->GetMathParent(endNode, endMathNode);

  DebDisplaySelection("\nInitial selection", aSelection, mMSIEditor, true);


  if (bCollapsed) {
      
    nsCOMPtr<msiIMathMLEditingBC> editingBC; 
    PRUint32 dontcare(0);
    PRUint32 mathmltype;
    
    printf("\nPoint of deletion:\n");
    mHTMLEditor -> DumpNode(endNode, 0, true);

      //endNode = rightmostChild;

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
       // This case can also be reached if the node is a <mo> that bounds the mrow.
        
       // Remove the first and last children of the mrow -- they should be fences.
       nsCOMPtr<nsIDOMNodeList> children;
       PRUint32 number;

       endNode->GetChildNodes(getter_AddRefs(children));
       msiUtils::GetNumberofChildren(endNode, number);
       if (number == 1){
          nsCOMPtr<nsIDOMNode> parent;
          endNode->GetParentNode(getter_AddRefs(parent));
          endNode = parent;
          msiUtils::GetNumberofChildren(endNode, number);
       }
       
       			 
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
       res = msiUtils::RemoveIndexedChildNode(mHTMLEditor, endNode, 2, removedChild);
       res = msiUtils::RemoveIndexedChildNode(mHTMLEditor, endNode, 1, removedChild);
       // Remove the msubsup container 
       res = mHTMLEditor->RemoveContainer(endNode);
       *aHandled = PR_TRUE;
    } else if (mathmltype ==  msiIMathMLEditingBC::MSI_INPUTBOX){
       nsCOMPtr<nsIDOMNode> parent;
       endNode->GetParentNode(getter_AddRefs(parent));
       res = mHTMLEditor->RemoveContainer(parent);
       *aHandled = PR_TRUE;
       

    } else {
       aSelection->Extend( endNode, endOffset-1 );
    }
  }

   
  if (!*aHandled){
     bDeleteEntireMath = PR_FALSE;  // if true, this function will not be called.
//     bDeleteEntireMath = bSelectionContainsTheEntireMathNode(mathNode, aSelection);
   
     DebDisplaySelection("\nDeletion not handled yet. Extend selection to:" , aSelection, mMSIEditor, true);

//     mMSIEditor->AdjustSelectionEnds(PR_TRUE, aAction); Done just before call; selection has not changed since then
   
     DebDisplaySelection("\nSelection before calling nsHTMLEditRules::WillDeleteSelection" , aSelection, mMSIEditor, true);

     nsHTMLEditRules::WillDeleteSelection(aSelection, aAction, aCancel, aHandled);

     mMSIEditor->AdjustSelectionEnds(PR_TRUE, aAction);
     DebDisplaySelection("\nSelection after calling nsHTMLEditRules::WillDeleteSelection" , aSelection, mMSIEditor, true);


     if (bDeleteEntireMath)
        return NS_OK;

  }                          
  
   
  mMSIEditor->AdjustSelectionEnds(PR_TRUE, aAction);

  res = mHTMLEditor->GetStartNodeAndOffset(aSelection, getter_AddRefs(startNode), &startOffset);
  if (NS_FAILED(res)) return res;
  if (!startNode) return NS_ERROR_FAILURE;
  
  res = mHTMLEditor->GetEndNodeAndOffset(aSelection, getter_AddRefs(endNode), &endOffset);
  if (NS_FAILED(res)) return res;
  if (!endNode) return NS_ERROR_FAILURE;

  DebDisplaySelection("\nSelection before calling FindCursor" , aSelection, mMSIEditor, true);

  bool b = false;
  int idx = FindCursorIndex(mMSIEditor, startMathNode, endNode, endOffset, b, 0);

  printf("\nidx is %d\n", idx);
  
  nsString text = SerializeMathNode(startMathNode);
    
  nsCOMPtr<msiISimpleComputeEngine>  pEngine = GetEngine();
  
  PRUnichar* result;
  const PRUnichar* inp;
  text.GetData(&inp);

  printf("\nSending for cleanup: %ls\n" , inp);
  pEngine->Perform(inp, 152, &result); // Cleanup function
  printf("\nBack from cleanup: %ls\n", result);

  nsString resString(result);
  
  mathElement = do_QueryInterface(startMathNode);
  mHTMLEditor->DeleteNode(mathElement);

  DebDisplaySelection("\nSelection before inserting new math", aSelection, mMSIEditor, true);

  nsCOMPtr<nsIDOMNode> parentOfMath;
  nsCOMPtr<nsIDOMNode> newMath;
  PRInt32 mathOffset;

  res = mHTMLEditor->GetStartNodeAndOffset(aSelection, getter_AddRefs(parentOfMath), &mathOffset);
  
  //mMSIEditor -> InsertHTML(nsString(result, resString.Length()));
  mMSIEditor -> InsertHTML(resString);

  msiUtils::GetChildNode(parentOfMath, mathOffset, newMath);
 
  printf("\nThe New Math\n ");
  mHTMLEditor->DumpNode(newMath, 0, true);
  printf("\nCursor idx = %d\n", idx);

  if (newMath != NULL){
 

     DebDisplaySelection("\nSelection after inserting new math", aSelection, mMSIEditor, true);
  
     nsCOMPtr<nsIDOMNode> theNode = 0;
     PRInt32 theOffset = 0;
     FindCursorNodeAndOffset(mHTMLEditor, newMath, idx, theNode, theOffset);

     printf("\nThe indicated node\n ");
     mHTMLEditor->DumpNode(theNode, 0, true);
     printf("\nOffset = %d\n", theOffset);

     nsIDOMRange* range;
     aSelection->GetRangeAt(0, &range);

     // if (mMSIEditor->IsTextNode(theNode)){
     //   mMSIEditor->GetMathParent(theNode, mathNode);
     //   theNode = mathNode;
     //   //theOffset = 1;
     // }

     range->SetStart(theNode, theOffset);
     aSelection->CollapseToStart();
     
     mMSIEditor->AdjustSelectionEnds(PR_TRUE, aAction);

     DebDisplaySelection("\nFinal selection", aSelection, mMSIEditor, true);
  }

  *aHandled = PR_TRUE;

  return res;
}

*/

PRBool
bSelectionContainsTheEntireMathNode(nsIDOMNode* aNode, nsISelection* aSelection) 
{
   nsCOMPtr<nsIDOMElement> mathElement;
     nsCOMPtr<nsIDOMNode> firstChildNode;
     nsCOMPtr<nsIDOMNode> lastChildNode;
     nsCOMPtr<nsIDOMRange> range;
     PRBool partlyContained = PR_FALSE;
     PRBool containsNode;

     mathElement = do_QueryInterface(aNode);	
     mathElement->GetFirstChild(getter_AddRefs(firstChildNode));
     mathElement->GetLastChild(getter_AddRefs(lastChildNode));
     aSelection->ContainsNode(firstChildNode, partlyContained, &containsNode);

     if (containsNode)
     {
       aSelection->ContainsNode(lastChildNode, partlyContained, &containsNode);
       if (containsNode) // all children of the math node are in the selection
       {
         aSelection->GetRangeAt(0, getter_AddRefs(range));
         range->SelectNode(aNode);
     return PR_TRUE;
       }
     }
   return PR_FALSE;
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

