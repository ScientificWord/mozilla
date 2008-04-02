#include "msiEditRules.h"
#include "nsCOMPtr.h"
#include "nsIServiceManager.h"
#include "msiEditor.h"

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
  
  if (bCollapsed) {
    nsCOMPtr<nsIDOMNode> startNode, selNode;
    PRInt32 startOffset, selOffset;
    
    res = mHTMLEditor->GetStartNodeAndOffset(aSelection, address_of(startNode), &startOffset);
    if (NS_FAILED(res)) return res;
    if (!startNode) return NS_ERROR_FAILURE;
    if (mMSIEditor->NodeInMath(startNode))
      /* do something special here (based on direction) */;
  }  
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
 return NS_OK;
}



