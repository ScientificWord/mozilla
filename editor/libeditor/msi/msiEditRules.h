#ifndef msiEditRules_h__
#define msiEditRules_h__

#include "nsHTMLEditRules.h"
#include "msiIEditActionListenerExtension.h"

class msiEditor;

class msiEditRules : public nsHTMLEditRules,
                     public msiIEditActionListenerExtension
{
public:

  NS_DECL_ISUPPORTS_INHERITED
  
            msiEditRules();
  virtual   ~msiEditRules();

  // nsIEditRules methods
  NS_IMETHOD Init(nsPlaintextEditor *aEditor, PRUint32 aFlags);

  // msiIEditActionListenerExtension methods
  NS_IMETHOD WillReplaceNode(nsIDOMNode *aNewNode, nsIDOMNode *aOldNode, nsIDOMNode *aParent);
  NS_IMETHOD DidReplaceNode(nsIDOMNode *aNewNode, nsIDOMNode *aOldNode, nsIDOMNode *aParent, nsresult aResult);

  //nsHTMLEditRules methods
  nsresult WillDeleteSelection(nsISelection *aSelection, nsIEditor::EDirection aAction, 
                               PRBool *aCancel, PRBool *aHandled);
  


  NS_IMETHOD DidDeleteNode(nsIDOMNode *aChild, nsresult aResult);
  NS_IMETHOD DidSplitNode(nsIDOMNode *aExistingRightNode, PRInt32 aOffset, nsIDOMNode *aNewLeftNode, nsresult aResult);
  NS_IMETHOD DidDeleteText(nsIDOMCharacterData *aTextNode, PRInt32 aOffset, PRInt32 aLength, nsresult aResult);
  NS_IMETHOD DidDeleteRange(nsIDOMRange *aRange);
  NS_IMETHOD DidDeleteSelection(nsISelection *aSelection);

  // data members
protected:
  msiEditor           *mMSIEditor;

private:
  nsresult WillDeleteMathSelection(nsISelection *aSelection, nsIEditor::EDirection aAction, 
                               PRBool *aCancel, PRBool *aHandled);


};

nsresult NS_NewMSIEditRules(nsIEditRules** aInstancePtrResult);

#endif //msiEditRules_h__

