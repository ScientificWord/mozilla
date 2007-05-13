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

  // data members
protected:
  msiEditor           *mMSIEditor;

};

nsresult NS_NewMSIEditRules(nsIEditRules** aInstancePtrResult);

#endif //msiEditRules_h__

