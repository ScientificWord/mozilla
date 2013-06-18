// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiEditor_h___
#define msiEditor_h___

#include "nsCOMPtr.h"
#include "msiIMathMLEditor.h"
//#include "msiIEditingManager.h"
#include "msiEditingManager.h"
#include "nsHTMLEditor.h"
#include "nsIRangeUtils.h"
#include "msiILayoutUtils.h"
#include "msiISelection.h"
#include "msiIAutosub.h"
#include "nsIContentFilter.h"
#include "nsITimer.h"

class msiEditorMouseListener;
class msiISelection;
class msiSelectionManager;

class msiEditor : 
///#ifdef ENABLE_EDITOR_API_LOG
///virtual public nsHTMLEditor,
///#else
public nsHTMLEditor,
///#endif
public msiIMathMLEditor
{
public:
           msiEditor();
   virtual ~msiEditor();

  // nsISupports

 NS_DECL_ISUPPORTS_INHERITED
  
  //msiIMathMLEditor
  NS_DECL_MSIIMATHMLEDITOR
  
  // nsIEditor overrides
  NS_IMETHOD Init(nsIDOMDocument *aDoc, nsIPresShell *aPresShell,  nsIContent *aRoot, nsISelectionController *aSelCon, PRUint32 aFlags);
  NS_IMETHOD HandleKeyPress(nsIDOMKeyEvent* aKeyEvent);  //non-scriptable
  NS_IMETHOD InsertText(const nsAString &aStringToInsert);
  // End of nsIEditor overrides
  
  // nsEditor method
  NS_IMETHOD DeleteSelectionImpl(EDirection aAction);


protected:
  NS_IMETHOD  InitRules();

  virtual nsresult InstallEventListeners();
  virtual nsresult CreateEventListeners();
  virtual void     RemoveEventListeners();
  
protected:
  nsCOMPtr<msiIEditingManager> m_msiEditingMan;
  nsCOMPtr<nsIDOMEventListener> m_mouseMotionListener;
  static nsCOMPtr<nsIRangeUtils> m_rangeUtils;
  static nsCOMPtr<msiIAutosub> m_autosub;
  nsCOMPtr<nsIContentFilter> m_filter;
  
  friend class msiEditorMouseListener;
  friend class msiEditorMouseMotionListener;
  friend class msiSelectionManager;
  
public:
//Utility functions
nsresult GetMathParent(nsIDOMNode * node, nsCOMPtr<nsIDOMNode> & mathParent);

nsresult ExtractDataFromKeyEvent(nsIDOMKeyEvent * aKeyEvent,
                                 PRUint32 & keyCode, PRUint32 & character,
                                 PRBool & isShift, PRBool & ctrlKey,
                                 PRBool & altKey, PRBool & metaKey);
PRBool IsTextContentNode(nsIDOMNode* node);

nsresult GetNSSelectionData(nsCOMPtr<nsISelection> &selection,
                            nsCOMPtr<nsIDOMNode> &startNode,
                            PRInt32 &startOffset,
                            nsCOMPtr<nsIDOMNode> &endNode,
                            PRInt32 &endOffset,
                            PRBool  &bCollapsed);
                            
PRBool IsSelectionCollapsed();
nsresult IsPointWithinCurrentSelection(nsCOMPtr<nsIDOMNode> & node, PRUint32 offset, 
                                       PRBool & withinSelection);

PRBool PositionIsAtBeginning(nsCOMPtr<nsIDOMNode> & parentNode, PRInt32 offset);
PRBool PositionIsAtEnd(nsCOMPtr<nsIDOMNode> & parentNode, PRInt32 offset);
                            
nsresult EnsureMathWithSelectionCollapsed(nsCOMPtr<nsIDOMNode> &node,
                                          PRInt32 & offset);
nsresult InsertSubOrSup(PRBool isSup);
nsresult InsertSymbolEx(nsISelection * aSelection, nsIDOMNode * aNode, 
                        PRInt32 aOffset, const nsAString & aSymbol);
nsresult InsertMathnameEx(nsISelection * aSelection, nsIDOMNode * aNode, 
                           PRInt32 aOffset, const nsAString & aMathname);
nsresult InsertMathunitEx(nsISelection * aSelection, nsIDOMNode * aNode, 
                           PRInt32 aOffset, const nsAString & aMathunit);
nsresult InsertEngineFunctionEx(nsISelection * aSelection, nsIDOMNode * aNode, 
                                PRInt32 aOffset, const nsAString & aName);
nsresult InsertMath(PRBool isDisplay);
PRUint32 KeyCodeToCaretOp(PRUint32 keycode, PRBool isShift, PRBool ctrlKey);
nsresult GetNodeAndOffsetFromMMLCaretOp(PRUint32 caretOp, nsCOMPtr<nsIDOMNode> & currNode,
                                     PRUint32 currOffset, nsCOMPtr<nsIDOMNode>& newNode,
                                     PRUint32 & newOffset);
                               
nsresult GetMSISelection(nsCOMPtr<msiISelection> & msiSelection); 
nsresult SetSelection(nsCOMPtr<nsIDOMNode> & focusNode, PRUint32 focusOffset, PRBool selecting, PRBool& preventDefault);
nsresult GetMayDrag(PRBool *mayDrag);
nsresult HandleArrowKeyPress(PRUint32 keycode, PRBool isShift, PRBool ctrlDown, 
                             PRBool altDown, PRBool metaDown, PRBool & preventDefault);
nsresult GetCommonAncestor(nsIDOMNode * node1, nsIDOMNode * node2, nsCOMPtr<nsIDOMNode> & commonAncestor);
                             
//msiSelection callback functions

static nsresult AdjustCaretCB(void* msieditor, nsIDOMEvent * mouseEvent, nsCOMPtr<nsIDOMNode> & node, PRInt32 &offset);
static nsresult SetSelectionCB(void* msieditor, nsCOMPtr<nsIDOMNode> & focusNode, PRInt32 focusOffset, 
                               PRBool selecting, PRBool & preventDefault);
//end msiSelection callback functions

nsresult AdjustCaret(nsIDOMEvent * mouseEvent, nsCOMPtr<nsIDOMNode> & node, PRInt32 &offset);

nsresult CreateTxnForDeleteSelection(nsIEditor::EDirection aAction,
                                     msiSelectionManager & msiSelMan,
                                     EditAggregateTxn  ** aTxn);
                                     
nsresult CreateTxnForDeleteInsertionPoint(msiSelectionManager & msiSelMan,
                                          PRUint32 index, 
                                          nsIEditor::EDirection aAction,
                                          EditAggregateTxn     *aTxn);
nsresult GetNextCharacter( nsIDOMNode * nodeIn, PRUint32 offsetIn, nsIDOMNode ** pnodeOut, PRUint32& offsetOut, PRBool inMath, PRUnichar prevChar, PRInt32 & _result);
nsresult CheckForAutoSubstitute(PRBool inmath);

protected:
  virtual nsresult InsertReturnInMath( nsIDOMNode * splitpointNode, 
                                       PRInt32 splitpointOffset, 
                                       PRBool* bHandled);
                                       

};

class msiContentFilter : public nsIContentFilter
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSICONTENTFILTER
  msiContentFilter( nsIEditor * editor);
  NS_IMETHOD copyfiles( nsIDocument * srcDoc,nsIDocument * doc, nsIDOMNodeList * objnodes, 
    nsIDOMNode * anode, PRUint32 count);

  static void SetDataFromTimer(nsITimer *aTimer, void *closure);
  void ClearTimerList();
private:
  nsIEditor * m_editor;
  nsCOMArray<nsITimer> m_timerlist;
  ~msiContentFilter();
protected:
};

#endif // msiEditor_h___
