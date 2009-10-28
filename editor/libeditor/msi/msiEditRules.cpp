#include "msiEditRules.h"
#include "nsCOMPtr.h"
#include "nsIServiceManager.h"
#include "nsIRange.h"
#include "msiEditor.h"
#include "msiUtils.h"
#include "nsIDOMNodeList.h"

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

  PRBool bCollapsed;
  nsresult res = aSelection->GetIsCollapsed(&bCollapsed);
  if (NS_FAILED(res)) return res;
  
  nsCOMPtr<nsIDOMNode> startNode, endNode;
  PRInt32 startOffset, endOffset;
  printf("In msiEditRules::WillDeleteSelection\n");
  DumpSelection(aSelection);
  res = mHTMLEditor->GetStartNodeAndOffset(aSelection, address_of(startNode), &startOffset);
  if (NS_FAILED(res)) return res;
  if (!startNode) return NS_ERROR_FAILURE;
  res = mHTMLEditor->GetEndNodeAndOffset(aSelection, address_of(endNode), &endOffset);
  if (NS_FAILED(res)) return res;
  if (!endNode) return NS_ERROR_FAILURE;
  if (bCollapsed) {
      
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
      //if ( msiUtils::hasMMLType(mHTMLEditor, startNode, msiIMathMLEditingBC::MATHML_MROWFENCE) )	{
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

			 return res;

	  } else if ( mathmltype == msiIMathMLEditingBC::MATHML_MSQRT ) {
			 // Remove the msqrt container 
		     res = mHTMLEditor->RemoveContainer(endNode);
			 *aHandled = PR_TRUE;
			 return res;

	  } else if ( mathmltype == msiIMathMLEditingBC::MATHML_MSUP || mathmltype == msiIMathMLEditingBC::MATHML_MSUB) {
	         // remove the superscript	or subscript
	         nsCOMPtr<nsIDOMNode>  removedChild;
			 res = msiUtils::RemoveIndexedChildNode(mHTMLEditor, endNode, 1, removedChild);
			 
			 // Remove the msup (or msub) container 
		     res = mHTMLEditor->RemoveContainer(endNode);
			 *aHandled = PR_TRUE;
			 return res;
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
			 return res;

	  } else {
      	  aSelection->Extend( endNode, endOffset-1 );
	  }
  }  
  if (mMSIEditor->NodeInMath(startNode)||mMSIEditor->NodeInMath(endNode))
  {
    nsCOMPtr<nsIDOMNode> mathNode;
    nsCOMPtr<nsIDOMElement> mathElement;
    nsCOMPtr<nsIDOMNode> firstChildNode;
    nsCOMPtr<nsIDOMNode> lastChildNode;
    nsCOMPtr<nsIDOMRange> range;
    mMSIEditor->GetMathParent(startNode, mathNode);
    mathElement = do_QueryInterface(mathNode);
	PRBool partlyContained = PR_FALSE;
    PRBool containsNode;

	if (mathNode){
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
    } 
    mMSIEditor->GetMathParent(endNode, mathNode);
    mathElement = do_QueryInterface(mathNode);
	if (mathNode){
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
    } 

    /* do something special here (based on direction) */;
    
  }
  printf("In msiEditRules::WillDeleteSelection(2)\n");
  DumpSelection(aSelection);
  mMSIEditor->AdjustSelectionEnds(PR_TRUE, aAction);
  printf("In msiEditRules::WillDeleteSelection(3)\n");
  DumpSelection(aSelection);
  return nsHTMLEditRules::WillDeleteSelection(aSelection, aAction, aCancel, aHandled);
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
  printf("msiEditRules:DidReplaceNode\n");
 return NS_OK; //nsHTMLEditRules::DidReplaceNode(aNewNode,aOldNode,aParent,aResult);
}

NS_IMETHODIMP 
msiEditRules::DidDeleteNode(nsIDOMNode *aChild, nsresult aResult)
{
  printf("msiEditRules:DidDeleteNode\n");
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
  printf("msiEditRules:DidDeleteText\n");
  return nsHTMLEditRules::DidDeleteText(aTextNode, aOffset, aLength, aResult);  
}


NS_IMETHODIMP
msiEditRules::DidDeleteRange(nsIDOMRange *aRange)
{
  printf("msiEditRules:DidDeleteRange\n");
  return nsHTMLEditRules::DidDeleteRange(aRange);
}


NS_IMETHODIMP
msiEditRules::DidDeleteSelection(nsISelection *aSelection)
{
  printf("msiEditRules:DidDeleteSelection\n");
  return nsHTMLEditRules::DidDeleteSelection(aSelection);
}

