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
PRInt32 FindCursorIndex(nsHTMLEditor* editor, nsIDOMNode* node, nsIDOMNode* caretNode, PRInt32 caretOffset, bool& done, PRInt32 level);
void FindCursorNodeAndOffset(nsHTMLEditor* editor, nsIDOMNode* node, PRInt32& charCount, nsCOMPtr<nsIDOMNode>& theNode, PRInt32& theOffset);

nsCOMPtr<msiISimpleComputeEngine> GetEngine();
nsString SerializeMathNode(nsCOMPtr<nsIDOMNode> mathNode);

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

  if (mathElement){
      mMSIEditor->AdjustSelectionEnds(PR_TRUE, aAction);
      res = WillDeleteMathSelection(aSelection, aAction, aCancel, aHandled);
  } else {
     printf("In msiEditRules::WillDeleteSelection(2)\n");
     DumpSelection(aSelection);
     mMSIEditor->AdjustSelectionEnds(PR_TRUE, aAction);
     printf("In msiEditRules::WillDeleteSelection(3)\n");
     DumpSelection(aSelection);
     res = nsHTMLEditRules::WillDeleteSelection(aSelection, aAction, aCancel, aHandled);
  }
  return res;
}



void DebDisplaySelection(const char* str, nsISelection *aSelection, msiEditor* editor, bool recurse = false)
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
  PRBool bDeleteEntireMath = false;
  
  res = mHTMLEditor->GetStartNodeAndOffset(aSelection, address_of(startNode), &startOffset);
  if (NS_FAILED(res)) return res;
  if (!startNode) return NS_ERROR_FAILURE;

  res = mHTMLEditor->GetEndNodeAndOffset(aSelection, address_of(endNode), &endOffset);
  if (NS_FAILED(res)) return res;
  if (!endNode) return NS_ERROR_FAILURE;

  mMSIEditor->GetMathParent(endNode, mathNode);
  mathElement = do_QueryInterface(mathNode);

  printf("\nMath node:\n");
  mHTMLEditor -> DumpNode(mathNode, 0, true);


  if (!mathElement) return NS_ERROR_FAILURE;

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
     bDeleteEntireMath = bSelectionContainsTheEntireMathNode(mathNode, aSelection);
   
     DebDisplaySelection("\nDeletion not handled yet. Extend selection to:" , aSelection, mMSIEditor, true);

     mMSIEditor->AdjustSelectionEnds(PR_TRUE, aAction);
   
     DebDisplaySelection("\nSelection before calling nsHTMLEditRules::WillDeleteSelection" , aSelection, mMSIEditor, true);

     nsHTMLEditRules::WillDeleteSelection(aSelection, aAction, aCancel, aHandled);

     mMSIEditor->AdjustSelectionEnds(PR_TRUE, aAction);
     DebDisplaySelection("\nSelection after calling nsHTMLEditRules::WillDeleteSelection" , aSelection, mMSIEditor, true);


     if (bDeleteEntireMath)
        return NS_OK;

  }                          
  
   
  mMSIEditor->AdjustSelectionEnds(PR_TRUE, aAction);

  res = mHTMLEditor->GetStartNodeAndOffset(aSelection, address_of(startNode), &startOffset);
  if (NS_FAILED(res)) return res;
  if (!startNode) return NS_ERROR_FAILURE;
  
  res = mHTMLEditor->GetEndNodeAndOffset(aSelection, address_of(endNode), &endOffset);
  if (NS_FAILED(res)) return res;
  if (!endNode) return NS_ERROR_FAILURE;

  DebDisplaySelection("\nSelection before calling FindCursor" , aSelection, mMSIEditor, true);

  bool b = false;
  int idx = FindCursorIndex(mMSIEditor, mathNode, endNode, endOffset, b, 0);

  printf("\nidx is %d\n", idx);
  
  nsString text = SerializeMathNode(mathNode);
    
  nsCOMPtr<msiISimpleComputeEngine>  pEngine = GetEngine();
  
  PRUnichar* result;
  const PRUnichar* inp;
  text.GetData(&inp);

  printf("\nSending for cleanup: %ls\n" , inp);
  pEngine->Perform(inp, 152, &result); // Cleanup function
  printf("\nBack from cleanup: %ls\n", result);

  nsString resString(result);
  
  mHTMLEditor->DeleteNode(mathElement);

  DebDisplaySelection("\nSelection before inserting new math", aSelection, mMSIEditor, true);

  nsCOMPtr<nsIDOMNode> parentOfMath;
  nsCOMPtr<nsIDOMNode> newMath;
  PRInt32 mathOffset;

  res = mHTMLEditor->GetStartNodeAndOffset(aSelection, address_of(parentOfMath), &mathOffset);
  
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


nsString SerializeMathNode(nsCOMPtr<nsIDOMNode> mathNode) {
  nsCOMPtr<nsIDOMSerializer> ds = do_CreateInstance(NS_XMLSERIALIZER_CONTRACTID);
//  NS_ENSURE_STATE(ds);

  nsString text;
  nsresult rv = ds->SerializeToString(mathNode, text);
//  NS_ENSURE_SUCCESS(rv, rv);
  return text;
}


nsCOMPtr<msiISimpleComputeEngine> 
GetEngine() {
  nsCOMPtr<nsIProperties> fileLocator(do_GetService("@mozilla.org/file/directory_service;1"));
  nsCOMPtr<nsILocalFile> iniFile;
  fileLocator->Get("resource:app", NS_GET_IID(nsIFile), getter_AddRefs(iniFile));
  iniFile->Append(NS_LITERAL_STRING("mupInstall.gmr"));

  nsCOMPtr<msiISimpleComputeEngine> engine(do_GetService("@mackichan.com/simplecomputeengine;2"));	
  engine->Startup(iniFile);
  return engine;
}


PRInt32 CharLength(nsCOMPtr<nsIDOMNode> node)
{
    nsString str;
    nsCOMPtr<nsIDOM3Node> m;
    m = do_QueryInterface(node);
    m->GetTextContent(str);
    return str.Length();
}



PRInt32 FindCursorIndex(nsHTMLEditor* editor, 
                        nsIDOMNode* node, 
                        nsIDOMNode* caretNode, 
                        PRInt32 caretOffset, 
                        bool& done,
                        PRInt32 level)
{
  printf("\nFindCursor\n");
  editor->DumpNode(node, 2*level, true);

  if (editor->IsTextNode(node)) {

      if (caretNode == node) {
          done = true;
          printf("\nReturn caret offset %d", caretOffset);
          return caretOffset;
      } else {
          printf("\nReturn char length %d", CharLength(node));
          return CharLength(node); 
      }

  }	else {
      
      nsCOMPtr<msiIMathMLEditingBC> editingBC; 
      PRUint32 dontcare(0);
      PRUint32 mathmltype;

      msiUtils::GetMathMLEditingBC(editor, node, dontcare, editingBC);
      if (editingBC) {
        mathmltype = msiUtils::GetMathmlNodeType(editingBC);
      }

      nsCOMPtr<nsIDOMNodeList> children;
      PRUint32 number;
      nsCOMPtr<nsIDOMNode> child;

      node->GetChildNodes(getter_AddRefs(children));
      msiUtils::GetNumberofChildren(node, number);
    
      //int lastChildIndex = number;

      if (node == caretNode){
         number = caretOffset;
      }

      int count = 0;

      for (int i = 0; i < number; ++i) {
        msiUtils::GetChildNode(node, i, child);
        if (mathmltype == msiIMathMLEditingBC::MATHML_MFRAC && i == 1){
          count += 1;  // to distinguish numerator from denominator
        }
        count += FindCursorIndex(editor, child, caretNode, caretOffset, done, level+1);
        if (done)
          break;
      }
      printf("\nReturn sum %d", count);
      return count;

  }
      
}


void FindCursorNodeAndOffset(nsHTMLEditor* editor, nsIDOMNode* node, PRInt32& charCount, nsCOMPtr<nsIDOMNode>& theNode, PRInt32& theOffset)
{
     printf("\nFindNodeAndOffset of char %d in node\n", charCount);
     editor->DumpNode(node, 0, true);

     if (editor->IsTextNode(node)) {
         PRInt32 len = CharLength(node);

         if (len >= charCount){
           theNode = node;
           theOffset = charCount;
           charCount = 0;
     	     printf("\nFound\n");
         } else {
           charCount -= len;
         }

     }  else {
         theNode = node;
         theOffset = 0;

         nsCOMPtr<msiIMathMLEditingBC> editingBC; 
         PRUint32 dontcare(0);
         PRUint32 mathmltype;

         msiUtils::GetMathMLEditingBC(editor, node, dontcare, editingBC);
         if (editingBC) {
           mathmltype = msiUtils::GetMathmlNodeType(editingBC);
         }
           
         nsCOMPtr<nsIDOMNodeList> children;
         PRUint32 number;
         nsCOMPtr<nsIDOMNode> child;

         node->GetChildNodes(getter_AddRefs(children));
         msiUtils::GetNumberofChildren(node, number);

         for (PRUint32 i = 0; i < number; ++i) {
            printf("\nGet child %d\n", i);
            msiUtils::GetChildNode(node, i, child);

            //printf("\nCharLength is %d\n", CharLength(child));
            if (mathmltype == msiIMathMLEditingBC::MATHML_MFRAC && i == 1){
              charCount -= 1;  // to distinguish numerator from denominator
            }
            if (editor->IsTextNode(child)){
              charCount -= CharLength(child);
              theOffset += 1;
            } else {
              FindCursorNodeAndOffset(editor, child, charCount, theNode, theOffset);
            }
            if (charCount <= 0)
              break;
      }
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

