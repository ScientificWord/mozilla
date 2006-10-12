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


class msiEditorMouseListener;
class msiISelection;


class msiEditor : 
#ifdef ENABLE_EDITOR_API_LOG
virtual public nsHTMLEditor,
#else
public nsHTMLEditor,
#endif
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
  NS_IMETHOD HandleKeyPress(nsIDOMKeyEvent* aKeyEvent);  //non-scriptable
  NS_IMETHOD InsertText(const nsAString &aStringToInsert);
  NS_IMETHOD Init(nsIDOMDocument *aDoc, nsIPresShell *aPresShell,  nsIContent *aRoot, nsISelectionController *aSelCon, PRUint32 aFlags);
  // End of nsIEditor overrides
  
  

protected:
  virtual nsresult InstallEventListeners();
  virtual nsresult CreateEventListeners();
  virtual void     RemoveEventListeners();
  
protected:
  nsCOMPtr <msiIEditingManager> m_msiEditingMan;
  static nsIRangeUtils * m_rangeUtils;
  nsCOMPtr<nsIDOMEventListener> m_mouseMotionListener;
  
  
  friend class msiEditorMouseListener;
  friend class msiEditorMouseMotionListener;
  
protected:
//Utility functions
PRBool   NodeInMath(nsIDOMNode* node);
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

                            
nsresult EnsureMathWithSelectionCollapsed(nsCOMPtr<nsIDOMNode> &node,
                                          PRInt32 & offset);
nsresult InsertSubOrSup(PRBool isSup);
nsresult InsertSymbolEx(nsISelection * aSelection, nsIDOMNode * aNode, 
                        PRInt32 aOffset, PRUint32 aSymbol);
nsresult InsertMathnameEx(nsISelection * aSelection, nsIDOMNode * aNode, 
                           PRInt32 aOffset, const nsAString & aMathname);
nsresult InsertEngineFunctionEx(nsISelection * aSelection, nsIDOMNode * aNode, 
                                PRInt32 aOffset, const nsAString & aName);
nsresult InsertMath(PRBool isDisplay);
PRUint32 KeyCodeToCaretOp(PRUint32 keycode, PRBool isShift, PRBool ctrlKey);
nsresult GetNodeAndOffsetFromMMLCaretOp(PRUint32 caretOp, nsCOMPtr<nsIDOMNode> & currNode,
                                     PRUint32 currOffset, nsCOMPtr<nsIDOMNode>& newNode,
                                     PRUint32 & newOffset);
                               
nsresult GetMSISelection(nsCOMPtr<msiISelection> & msiSelection); 
nsresult SetSelection(nsCOMPtr<nsIDOMNode> & focusNode, PRUint32 focusOffset, PRBool selecting, PRBool& preventDefault);
nsresult GetMouseDown(PRBool * isDown);
nsresult SetMouseDown(PRBool  isDown);
nsresult GetMayDrag(PRBool *mayDrag);
nsresult HandleArrowKeyPress(PRUint32 keycode, PRBool isShift, PRBool ctrlDown, 
                             PRBool altDown, PRBool metaDown, PRBool & preventDefault);
nsresult GetCommonAncestor(nsIDOMNode * node1, nsIDOMNode * node2, nsCOMPtr<nsIDOMNode> & commonAncestor);
                             
//msiSelection callback functions

static nsresult AdjustCaretCB(void* msieditor, nsIDOMEvent * mouseEvent, nsCOMPtr<nsIDOMNode> & node, PRInt32 &offset);
static nsresult SetSelectionCB(void* msieditor, nsCOMPtr<nsIDOMNode> & focusNode, PRInt32 focusOffset, 
                               PRBool selecting, PRBool & preventDefault);

nsresult AdjustCaret(nsIDOMEvent * mouseEvent, nsCOMPtr<nsIDOMNode> & node, PRInt32 &offset);

};

#endif // msiEditor_h___
