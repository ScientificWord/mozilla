// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.
#include "nsCOMPtr.h"
#include "nsISupportsPrimitives.h"
#include "msiEditor.h"
#include "msiIMathMLInsertion.h"
#include "msiIMathMLCaret.h"
#include "msiIMathMLCoalesce.h"
#include "nsEditorEventListeners.h"
#include "msiEditorMouseListener.h"
#include "msiEditorMouseMotionListener.h"
#include "TransactionFactory.h"
#include "EditAggregateTxn.h"
#include "nsIDOMWindow.h"
#include "nsContentUtils.h"
#include "nsIDOMDocumentFragment.h"
#include "nsIDocument.h"
#include "nsIDOMText.h"
#include "nsIDOMAttr.h"
#include "nsIDOMNamedNodeMap.h"
#include "nsIDOMNodeList.h"
#include "nsIPresShell.h"
#include "nsIDOMNSUIEvent.h"
#include "nsIDOM3EventTarget.h"
#include "nsIDOMEventGroup.h"
#include "nsIServiceManager.h"
#include "nsServiceManagerUtils.h"
#include "nsIEditActionListener.h"
#include "nsIRange.h"
#include "nsIArray.h"
#include "nsArrayUtils.h"
#include "nsCOMArray.h"
#include "DeleteTextTxn.h"
#include "DeleteElementTxn.h"
#include "FlattenMrowTxn.h"
#include "ReplaceScriptBaseTxn.h"
#include "nsILocalFile.h"

#include "msiISelection.h"
#include "msiIEditingManager.h"
#include "msiSelectionManager.h"
#include "msiDeleteRangeTxn.h"
#include "msiEditRules.h"

#include "nsEditorUtils.h"
#include "msiIScriptRunner.h"

//The following may go away if we move the right things to the right interfaces, but for now:
#include "msiUtils.h"
#include "msiEditingAtoms.h"
#include "jcsDumpNode.h"
#include "nsEscape.h"
#include "nsIURI.h"
#include "nsIURL.h"
#include "msiIUtil.h"
#include "nsNetUtil.h"
// #include "msiEditingManager.h"

static PRInt32 instanceCounter = 0;
nsCOMPtr<nsIRangeUtils> msiEditor::m_rangeUtils = nsnull;
nsCOMPtr<msiIAutosub> msiEditor::m_autosub = nsnull;
PRInt32 msiEditor::s_editorCount = 0;




msiEditor::msiEditor()
{
  nsresult res(NS_OK);
  if (m_filter == nsnull) { 
    m_filter = new msiContentFilter(this);
  }
  AddInsertionListener(m_filter);
  SetInComplexTransaction(PR_FALSE);
  m_msiEditingMan = do_CreateInstance(MSI_EDITING_MANAGER_CONTRACTID, &res);
  if (!m_rangeUtils)
    m_rangeUtils = do_GetService("@mozilla.org/content/range-utils;1");
  instanceCounter += 1;
  if (!m_autosub)
    m_autosub = do_GetService("@mozilla.org/autosubstitute;1");
  m_editorID = 0;
}

msiEditor::~msiEditor()
{
  instanceCounter -= 1;
  m_msiEditingMan = nsnull;
  RemoveInsertionListener(m_filter);
  
//  if (instanceCounter <= 0)
//    NS_IF_RELEASE(m_rangeUtils);
}

NS_IMPL_ISUPPORTS_INHERITED1(msiEditor, nsHTMLEditor, msiIMathMLEditor)


nsresult
msiEditor::Init(nsIDOMDocument *aDoc, nsIPresShell *aPresShell,  nsIContent *aRoot, 
                nsISelectionController *aSelCon, PRUint32 aFlags)
{
  // Init the HTML editor
  nsresult res = nsHTMLEditor::Init(aDoc, aPresShell, aRoot, aSelCon, aFlags);
  if (NS_SUCCEEDED(res))
  {
    nsCOMPtr<msiISelection> msiSelection;
    GetMSISelection(msiSelection);
    if (!msiSelection)
      return NS_ERROR_FAILURE;
    res = msiSelection->InitalizeCallbackFunctions(AdjustCaretCB, 
                                                   SetSelectionCB, 
                                                   (void*)this);
  } 
  m_AutoSubEnabled = PR_TRUE;
  return res; 
}                

/* attribute boolean AutoSubEnabled; */
NS_IMETHODIMP msiEditor::GetAutoSubEnabled(PRBool *aAutoSubEnabled)
{
  *aAutoSubEnabled = m_AutoSubEnabled;
  return NS_OK;
}
NS_IMETHODIMP msiEditor::SetAutoSubEnabled(PRBool aAutoSubEnabled)
{
  m_AutoSubEnabled = aAutoSubEnabled;
  return NS_OK;
}

NS_IMETHODIMP msiEditor::GetEditorID(PRUint32* id) 
{ 
  *id = m_editorID;
  return NS_OK; 
}

NS_IMETHODIMP msiEditor::SetEditorID(PRUint32 id) 
{ 
  m_editorID = id;
  return NS_OK; 
}


nsresult
msiEditor::CreateEventListeners()
{
  nsresult rv = NS_OK;

  if (!mMouseListenerP)
  {
    // get a mouse listener
    rv = NS_NewMSIEditorMouseListener(getter_AddRefs(mMouseListenerP), this);
    if (NS_FAILED(rv))
      return rv;
  }
  if (!m_mouseMotionListener) 
  {
    rv = NS_NewMSIEditorMouseMotionListener(getter_AddRefs(m_mouseMotionListener), this);
    if (NS_FAILED(rv))
      return rv;
  }
  return nsHTMLEditor::CreateEventListeners();
}

nsresult
msiEditor::InstallEventListeners()
{
  NS_ENSURE_TRUE(mDocWeak && mPresShellWeak && m_mouseMotionListener,
                 NS_ERROR_NOT_INITIALIZED);

  nsCOMPtr<nsPIDOMEventTarget> piTarget = GetPIDOMEventTarget();

  if (!piTarget) 
  {
    RemoveEventListeners();
    return NS_ERROR_FAILURE;
  }
  nsresult rv = piTarget->AddEventListenerByIID(m_mouseMotionListener, NS_GET_IID(nsIDOMMouseMotionListener));
  if (NS_SUCCEEDED(rv))
    rv = nsHTMLEditor::InstallEventListeners();
  else  
  {
    NS_ERROR("failed to register some event listeners");
    RemoveEventListeners();
  }
  return rv;
}

void
msiEditor::RemoveEventListeners()
{
  if (!mDocWeak)
    return;

  nsCOMPtr<nsPIDOMEventTarget> piTarget = GetPIDOMEventTarget();
  if (piTarget)
  {
    // unregister the event listeners with the DOM event receiver

    if (m_mouseMotionListener)
    {
      piTarget->RemoveEventListenerByIID(m_mouseMotionListener,
                                    NS_GET_IID(nsIDOMMouseMotionListener));
      m_mouseMotionListener = nsnull;
    }
  }
  nsHTMLEditor::RemoveEventListeners();
  return;
}


//Begin msiIMathMLEditor

NS_IMETHODIMP 
msiEditor::GetMathMLEditingBC(nsIDOMNode * node, PRUint32 offset, PRBool clean,
                              msiIMathMLEditingBC** editingBC)
{
  nsresult res(NS_ERROR_FAILURE);
  if (m_msiEditingMan)
    res = m_msiEditingMan->GetMathMLEditingBC(node, offset, clean, editingBC);
  return res;
}

NS_IMETHODIMP 
msiEditor::GetMathMLInsertionInterface(nsIDOMNode * node, PRUint32 offset,
                                       msiIMathMLInsertion** mathml)
{
  nsresult res(NS_ERROR_FAILURE);
  *mathml = nsnull;
  if (m_msiEditingMan)
    res = m_msiEditingMan->GetMathMLInsertionInterface(node, offset, mathml);
  return res;
}

NS_IMETHODIMP 
msiEditor::GetMathMLCaretInterface(nsIDOMNode * node, PRUint32 offset,
                                    msiIMathMLCaret** mathml)
{
  nsresult res(NS_ERROR_FAILURE);
  *mathml = nsnull;
  if (m_msiEditingMan)
    res = m_msiEditingMan->GetMathMLCaretInterface(this, node, offset, mathml);
  if (NS_SUCCEEDED(res) && *mathml)
  {
    nsCOMPtr<msiIMathMLCaret> xxx(do_QueryInterface(*mathml));
    xxx->PrepareForCaret(this);
    PRUint32 mmlType(msiIMathMLEditingBC::MATHML_UNKNOWN);
    nsCOMPtr<msiIMathMLEditingBC> bcEditing(do_QueryInterface(*mathml));
    if (bcEditing)
    {
      bcEditing->GetMathmlType(&mmlType);
      if (mmlType != msiIMathMLEditingBC::MATHML_UNKNOWN && 
          mmlType != msiIMathMLEditingBC::MATHML_MATH)
      {
        nsCOMPtr<nsIDOMNode> mmlNode, parent;
        nsCOMPtr<msiIMathMLCaret> parentEditing;
        res = bcEditing->GetMathmlNode(getter_AddRefs(mmlNode));
        if (NS_SUCCEEDED(res) && mmlNode)
          res = mmlNode->GetParentNode(getter_AddRefs(parent));
        if (NS_SUCCEEDED(res) && parent)
          res = m_msiEditingMan->GetMathMLCaretInterface(this, parent, 
                                                         msiIMathMLEditingBC::INVALID, 
                                                         getter_AddRefs(parentEditing));
        if (NS_SUCCEEDED(res) && parentEditing)
          res = parentEditing->PrepareForCaret(this);
      }
    }
  }  
  return res;
}

NS_IMETHODIMP 
msiEditor::GetMathMLCoalesceInterface(nsIDOMNode * node, PRUint32 offset,
                                      msiIMathMLCoalesce** mathml)
{
  nsresult res(NS_ERROR_NO_INTERFACE);
  *mathml = nsnull;
  if (m_msiEditingMan)
    res = m_msiEditingMan->GetMathMLCoalesceInterface(node, offset, mathml);
  return res;
}



nsresult 
msiEditor::InsertMathNodeAtSelection(nsIDOMElement * aElement)
{
  nsresult res;
  nsCOMPtr<nsISelection> selection;
  nsCOMPtr<nsIDOMNode> startNode, endNode;
  PRInt32 startOffset(0), endOffset(0);
  PRBool bCollapsed(PR_FALSE);
  res = GetNSSelectionData(selection, startNode, startOffset, endNode, 
                         endOffset, bCollapsed);	
  if (bCollapsed)
  {
    return m_msiEditingMan->InsertMathmlElement(this, selection, startNode, startOffset, 0, aElement);
  }
  return NS_OK;
}

nsresult msiEditor::InsertMath(PRBool isDisplay)
{
  nsresult res(NS_OK);
  if (!(mFlags & eEditorPlaintextMask)) // copied from nsHTMLEditor -- I don't know if this is an issue
  {
    nsCOMPtr<nsISelection> selection;
    nsCOMPtr<nsIDOMNode> startNode, endNode;
    PRInt32 startOffset(0), endOffset(0);
    PRBool bCollapsed(PR_FALSE);
    res = GetNSSelectionData(selection, startNode, startOffset, endNode, 
                          endOffset, bCollapsed);
    if (NS_SUCCEEDED(res)) 
    {
      nsCOMPtr<nsIDOMNode> theNode;
      PRInt32 theOffset(0);
      if (!bCollapsed)
     {
// TODO add stuff so that selected stuff is changed to become the base  or the script ?
// current SWP behavoir is to make it the script, but this may not be correct in light
// of the fact that sub and sup have a well defined base in mathml.
// Also need to deal with the case where we are not in math, or part of the selection is not
// in math.
      }
      else
      {
        theNode = startNode;
        theOffset = startOffset;
        nsCOMPtr<nsIEditor> editor;
        QueryInterface(NS_GET_IID(nsIEditor), getter_AddRefs(editor));
        BeginTransaction();
        PRUint32 flags(msiIMathMLInsertion::FLAGS_NONE);
        SaveSelection(selection);
        if (m_msiEditingMan)
          res = m_msiEditingMan->InsertMath(editor, selection, theNode, theOffset, flags, isDisplay);

        EndTransaction();
      }
    }
  }
  return res;
}

NS_IMETHODIMP 
msiEditor::InsertInlineMath()
{
  return InsertMath(PR_FALSE);
}


NS_IMETHODIMP 
msiEditor::InsertDisplay()
{
  nsCOMPtr<nsIDOMNode> mathnode;
  nsCOMPtr<nsIDOMNode> parent;
  nsCOMPtr<nsIDOMNode> msidisplay;
  nsString attr = NS_LITERAL_STRING("display");
  nsString val = NS_LITERAL_STRING("block");
  nsString currentVal;
  nsString parentName;
  nsString strmsidisplay = NS_LITERAL_STRING("msidisplay");
  SelectionInMath(getter_AddRefs(mathnode));
  if (mathnode)
  {
    // skip this if the display attribute is already 'block'.
    // setting the attribute won't hurt, but it generates an undo stack item.
    nsCOMPtr<nsIDOMElement> mathElement = do_QueryInterface(mathnode);
    PRBool isDisplaySet;
    GetAttributeValue(mathElement, attr, currentVal, &isDisplaySet);
    if (!isDisplaySet || !currentVal.Equals(val))
      SetAttribute(mathElement, attr, val);
    // If there is no msidisplay tag above, add one
    mathnode->GetParentNode(getter_AddRefs(parent));
    parent->GetLocalName(parentName);
    if (parentName.Equals(strmsidisplay)) return NS_OK;
    // otherwise insert an msidisplay node above
    InsertContainerAbove(mathnode, address_of(msidisplay), strmsidisplay , nsnull, nsnull);
//    NS_IF_ADDREF((nsIDOMNode*)msidisplay);
     return NS_OK;
    // find the math node and set the display attribute
  }
  else
  return InsertMath(PR_TRUE);
}                                                      

NS_IMETHODIMP 
msiEditor::InsertSuperscript()
{
  return InsertSubOrSup(PR_TRUE);
}


NS_IMETHODIMP 
msiEditor::InsertSubscript()
{
  return InsertSubOrSup(PR_FALSE);
}

NS_IMETHODIMP 
msiEditor::InsertFraction(const nsAString& lineThickness, PRUint32 attrFlags)
{
  nsresult res(NS_ERROR_FAILURE);
  if (!(mFlags & eEditorPlaintextMask)) 
  {
    nsCOMPtr<nsISelection> selection;
    nsCOMPtr<nsIDOMNode> startNode, endNode;
    PRInt32 startOffset(0), endOffset(0);
    PRBool bCollapsed(PR_FALSE);
    res = GetNSSelectionData(selection, startNode, startOffset, endNode, 
                           endOffset, bCollapsed);
    if (NS_SUCCEEDED(res)) 
    {
      nsCOMPtr<nsIDOMNode> theNode;
      PRInt32 theOffset(0);
      theNode = startNode;
      theOffset = startOffset;
      if (NS_SUCCEEDED(res))
      {
        nsCOMPtr<nsIEditor> editor;
        QueryInterface(NS_GET_IID(nsIEditor), getter_AddRefs(editor));
        res = m_msiEditingMan->InsertFraction(editor, selection, theNode, theOffset, lineThickness, attrFlags);
      }  
    }
  }
  return res;
}

NS_IMETHODIMP 
msiEditor::InsertBinomial(const nsAString& opening, const nsAString& closing,
                          const nsAString& lineThickness, PRUint32 attrFlags)
{
  nsresult res(NS_ERROR_FAILURE);
  if (!(mFlags & eEditorPlaintextMask)) // copied from nsHTMLEditor -- I don't know if this is an issue
  {
    nsCOMPtr<nsISelection> selection;
    nsCOMPtr<nsIDOMNode> startNode, endNode;
    PRInt32 startOffset(0), endOffset(0);
    PRBool bCollapsed(PR_FALSE);
    res = GetNSSelectionData(selection, startNode, startOffset, endNode, 
                           endOffset, bCollapsed);
    if (NS_SUCCEEDED(res)) 
    {
      nsCOMPtr<nsIDOMNode> theNode;
      PRInt32 theOffset(0);
      theNode = startNode;
      theOffset = startOffset;
      if (NS_SUCCEEDED(res))
      {
        nsCOMPtr<nsIEditor> editor;
        QueryInterface(NS_GET_IID(nsIEditor), getter_AddRefs(editor));
        res = m_msiEditingMan->InsertBinomial(editor, selection, theNode, theOffset, opening, closing, lineThickness, attrFlags);
      }  
    }
  }
  return res;
}

NS_IMETHODIMP 
msiEditor::InsertSqRoot()
{
  nsresult res(NS_ERROR_FAILURE);
  if (!(mFlags & eEditorPlaintextMask)) // copied from nsHTMLEditor -- I don't know if this is an issue
  {
    nsCOMPtr<nsISelection> selection;
    nsCOMPtr<nsIDOMNode> startNode, endNode;
    PRInt32 startOffset(0), endOffset(0);
    PRBool bCollapsed(PR_FALSE);
    res = GetNSSelectionData(selection, startNode, startOffset, endNode, 
                           endOffset, bCollapsed);
    if (NS_SUCCEEDED(res)) 
    {
      nsCOMPtr<nsIDOMNode> theNode;
      PRInt32 theOffset(0);
      theNode = startNode;
      theOffset = startOffset;
      if (NS_SUCCEEDED(res))
      {
        nsCOMPtr<nsIEditor> editor;
        QueryInterface(NS_GET_IID(nsIEditor), getter_AddRefs(editor));
        res = m_msiEditingMan->InsertSqRoot(editor, selection, theNode, theOffset);
      }  
    }
  }
  return res;
}

NS_IMETHODIMP 
msiEditor::InsertRoot()
{
  nsresult res(NS_ERROR_FAILURE);
  if (!(mFlags & eEditorPlaintextMask)) // copied from nsHTMLEditor -- I don't know if this is an issue
  {
    nsCOMPtr<nsISelection> selection;
    nsCOMPtr<nsIDOMNode> startNode, endNode;
    PRInt32 startOffset(0), endOffset(0);
    PRBool bCollapsed(PR_FALSE);
    res = GetNSSelectionData(selection, startNode, startOffset, endNode, 
                           endOffset, bCollapsed);
    if (NS_SUCCEEDED(res)) 
    {
      nsCOMPtr<nsIDOMNode> theNode;
      PRInt32 theOffset(0);
      theNode = startNode;
      theOffset = startOffset;
      if (NS_SUCCEEDED(res))
      {
        nsCOMPtr<nsIEditor> editor;
        QueryInterface(NS_GET_IID(nsIEditor), getter_AddRefs(editor));
        res = m_msiEditingMan->InsertRoot(editor, selection, theNode, theOffset);
      }  
    }
  }
  return res;
}

NS_IMETHODIMP
msiEditor::InsertSymbol(const nsAString & symbol)
{
  nsresult res(NS_ERROR_FAILURE);
  PRBool bCollapsed(PR_FALSE);
  if (!(mFlags & eEditorPlaintextMask)) 
  {
    nsCOMPtr<nsISelection> selection;
    nsCOMPtr<nsIDOMNode> startNode, endNode;
    PRInt32 startOffset(0), endOffset(0);
    res = GetNSSelectionData(selection, startNode, startOffset, endNode, 
                           endOffset, bCollapsed);
    if (NS_SUCCEEDED(res)) 
    {
      nsCOMPtr<nsIDOMNode> theNode;
      PRInt32 theOffset(0);
      if (!bCollapsed)
      {
        res = DeleteSelection(nsIEditor::eNone);        // TODO add stuff so that selected stuff is changed to become the base  or the script ?
        // current SWP behavoir is to make it the script, but this may not be correct in light
        // of the fact that sub and sup have a well defined base in mathml.
        // Also need to deal with the case where we are not in math, or part of the selection is not
        // in math.
      }
      res = GetNSSelectionData(selection, startNode, startOffset, endNode, 
                             endOffset, bCollapsed);
      theNode = startNode;
      theOffset = startOffset;
      if (NS_SUCCEEDED(res))
        res = InsertSymbolEx(selection, theNode, theOffset, symbol);
    }
  }
  return res;
}

NS_IMETHODIMP
msiEditor::InsertMathname(const nsAString & mathname)
{
  nsresult res(NS_ERROR_FAILURE);
  NS_ASSERTION(mathname.Length() > 0, "Mathname must be at least a single character.");
  if (mathname.Length() > 0 && !(mFlags & eEditorPlaintextMask))
  {
    nsCOMPtr<nsISelection> selection;
    nsCOMPtr<nsIDOMNode> startNode, endNode;
    PRInt32 startOffset(0), endOffset(0);
    PRBool bCollapsed(PR_FALSE);
    res = GetNSSelectionData(selection, startNode, startOffset, endNode, 
                           endOffset, bCollapsed);
    
    //printf("\njcs -- InsertMathName selection:\n");
    //DumpSelection(selection);

    if (NS_SUCCEEDED(res)) 
    {
      // PRInt32 comparison;
      // PRInt32 offset;
      // PRUint16 nodeType;
      // nsCOMPtr<nsIDOMNode> firstNode;
      // nsCOMPtr<nsIDOMNode> parent;
      // ComparePoints(startNode, startOffset, endNode, endOffset, &comparison);
      // if (comparison > 0) firstNode = endNode;
      // else firstNode = startNode;
      // firstNode->GetNodeType(&nodeType);
      // if (nodeType == nsIDOMNode::TEXT_NODE) {
      //   firstNode->GetParentNode(getter_AddRefs(firstNode));
      // }
      // res = GetNodeLocation(firstNode, address_of(parent), &offset);
      nsCOMPtr<nsIDOMNode> theNode;
      PRInt32 theOffset(0);
      if (!bCollapsed)
      {
        res = DeleteSelection(nsIEditor::eNone); 
        // TODO add stuff so that selected stuff is changed to become the base  or the script ?
        // current SWP behavoir is to make it the script, but this may not be correct in light
        // of the fact that sub and sup have a well defined base in mathml.
        // Also need to deal with the case where we are not in math, or part of the selection is not
        // in math.
      }
      res = GetNSSelectionData(selection, startNode, startOffset, endNode, 
                             endOffset, bCollapsed);
      theNode = startNode;
      theOffset = startOffset;
      selection->Collapse(theNode, theOffset);
      
      if (NS_SUCCEEDED(res))
        res = InsertMathnameEx(selection, theNode, theOffset, mathname); // BBM: Why does Larry pass post selection and (node,offset)?
    }
  }
  else if (mathname.Length() == 0)
    res = NS_OK;
  return res;
}


NS_IMETHODIMP
msiEditor::InsertMathunit(const nsAString & mathunit)
{
  nsresult res(NS_ERROR_FAILURE);
  NS_ASSERTION(mathunit.Length() > 0, "Mathunit must be at least a single character.");
  if (mathunit.Length() > 0 && !(mFlags & eEditorPlaintextMask)) // copied from nsHTMLEditor -- I don't know if this is an issue
  {
    nsCOMPtr<nsISelection> selection;
    nsCOMPtr<nsIDOMNode> startNode, endNode;
    PRInt32 startOffset(0), endOffset(0);
    PRBool bCollapsed(PR_FALSE);
    res = GetNSSelectionData(selection, startNode, startOffset, endNode, 
                           endOffset, bCollapsed);
    if (NS_SUCCEEDED(res)) 
    {
      nsCOMPtr<nsIDOMNode> theNode;
      PRInt32 theOffset(0);
      theNode = startNode;
      theOffset = startOffset;
      if (NS_SUCCEEDED(res))
        res = InsertMathunitEx(selection, theNode, theOffset, mathunit);
    }
  }
  else if (mathunit.Length() == 0)
    res = NS_OK;
  return res;
}

NS_IMETHODIMP
msiEditor::InsertEngineFunction(const nsAString & mathname)
{
  nsresult res(NS_ERROR_FAILURE);
  NS_ASSERTION(mathname.Length() > 0, "Function name must be at least a single character.");
  if (mathname.Length() > 0 && !(mFlags & eEditorPlaintextMask)) // copied from nsHTMLEditor -- I don't know if this is an issue
  {
    nsCOMPtr<nsISelection> selection;
    nsCOMPtr<nsIDOMNode> startNode, endNode;
    PRInt32 startOffset(0), endOffset(0);
    PRBool bCollapsed(PR_FALSE);
    res = GetNSSelectionData(selection, startNode, startOffset, endNode, 
                           endOffset, bCollapsed);
    if (NS_SUCCEEDED(res)) 
    {
      nsCOMPtr<nsIDOMNode> theNode;
      PRInt32 theOffset(0);
      if (!bCollapsed)
      {
        res = DeleteSelection(nsIEditor::eNone); 
        // TODO add stuff so that selected stuff is changed to become the base  or the script ?
        // current SWP behavoir is to make it the script, but this may not be correct in light
        // of the fact that sub and sup have a well defined base in mathml.
        // Also need to deal with the case where we are not in math, or part of the selection is not
        // in math.
      }
      theNode = startNode;
      theOffset = startOffset;

      if (NS_SUCCEEDED(res))
        res = InsertEngineFunctionEx(selection, theNode, theOffset, mathname);
    }
  }
  else if (mathname.Length() == 0)
    res = NS_OK;
  return res;
}

NS_IMETHODIMP
msiEditor::InsertFence(const nsAString & open, const nsAString & close)
{
  nsresult res(NS_ERROR_FAILURE);
  if (!(mFlags & eEditorPlaintextMask))
  {
    nsCOMPtr<nsISelection> selection;
    nsCOMPtr<nsIDOMNode> startNode, endNode;
		nsCOMPtr<nsIDOMElement> mtable;
    PRInt32 startOffset(0), endOffset(0);
    PRBool bCollapsed(PR_FALSE);
		PRBool allSelected(PR_FALSE);
		res = GetAllCellsSelected(getter_AddRefs(mtable), &allSelected);
		if (allSelected && mtable)
		{
			SelectTable();
		}

    res = GetNSSelectionData(selection, startNode, startOffset, endNode, 
                           endOffset, bCollapsed);
    if (NS_SUCCEEDED(res)) 
    {
      nsCOMPtr<nsIDOMNode> theNode;
      PRInt32 theOffset(0);
      theNode = startNode;
      theOffset = startOffset;
      nsCOMPtr<nsIEditor> editor;
      QueryInterface(NS_GET_IID(nsIEditor), getter_AddRefs(editor));
      res = m_msiEditingMan->InsertFence(editor, selection, theNode, 
                                         theOffset, open, close);
    }
  }
  return res;
}

NS_IMETHODIMP
msiEditor::InsertMatrix(PRUint32 rows, PRUint32 cols, const nsAString & rowSignature,
  const nsAString & delim)
{
  nsresult res(NS_ERROR_FAILURE);
  if (!(mFlags & eEditorPlaintextMask)) // copied from nsHTMLEditor -- I don't know if this is an issue
  {
    nsCOMPtr<nsISelection> selection;
    nsCOMPtr<nsIDOMNode> startNode, endNode;
    PRInt32 startOffset(0), endOffset(0);
    PRBool bCollapsed(PR_FALSE);
    res = GetNSSelectionData(selection, startNode, startOffset, endNode, 
                           endOffset, bCollapsed);
    if (NS_SUCCEEDED(res)) 
    {
      nsCOMPtr<nsIDOMNode> theNode;
      PRInt32 theOffset(0);
      if (!bCollapsed)
      {
        //res = NS_ERROR_FAILURE;
        res = DeleteSelection(nsIEditor::eNone); 
        // TODO add stuff so that selected stuff is changed to become the base  or the script ?
        // current SWP behavoir is to make it the script, but this may not be correct in light
        // of the fact that sub and sup have a well defined base in mathml.
        // Also need to deal with the case where we are not in math, or part of the selection is not
        // in math.
      }
      theNode = startNode;
      theOffset = startOffset;
      
      if (NS_SUCCEEDED(res))
      {
        nsCOMPtr<nsIEditor> editor;
        QueryInterface(NS_GET_IID(nsIEditor), getter_AddRefs(editor));
        res = m_msiEditingMan->InsertMatrix(editor, selection, theNode, 
                                           theOffset, rows, cols, rowSignature, delim);
      }  
    }
  }
  return res;
}

NS_IMETHODIMP
msiEditor::InsertOperator(const nsAString & symbol, PRUint32 attrFlags,
                          const nsAString & leftspace, const nsAString & rightspace,
                          const nsAString & minsize, const nsAString & maxsize)
{
  nsresult res(NS_ERROR_FAILURE);
  if (!(mFlags & eEditorPlaintextMask)) // copied from nsHTMLEditor -- I don't know if this is an issue
  {
    nsCOMPtr<nsISelection> selection;
    nsCOMPtr<nsIDOMNode> startNode, endNode;
    PRInt32 startOffset(0), endOffset(0);
    PRBool bCollapsed(PR_FALSE);
    res = GetNSSelectionData(selection, startNode, startOffset, endNode, 
                           endOffset, bCollapsed);
    if (NS_SUCCEEDED(res)) 
    {
      nsCOMPtr<nsIDOMNode> theNode;
      PRInt32 theOffset(0);
      if (!bCollapsed)
      {
        //res = NS_ERROR_FAILURE;
        res = DeleteSelection(nsIEditor::eNone); 
        // TODO add stuff to delete and replace the selection?
        // current SWP behavoir is to replace selection by the operator, but since we may want to allow
        // arbitrary math to be the content of an <mo> at some time, this should be considered.
        // For now, should simply replace the selection.
        // Also need to deal with the case where we are not in math, or part of the selection is not
        // in math.
      }
      theNode = startNode;
      theOffset = startOffset;
      
      if (NS_SUCCEEDED(res))
      {
        nsCOMPtr<nsIEditor> editor;
        QueryInterface(NS_GET_IID(nsIEditor), getter_AddRefs(editor));
        res = m_msiEditingMan->InsertOperator(editor, selection, theNode, 
                                              theOffset, symbol, attrFlags,
                                              leftspace, rightspace, 
                                              minsize, maxsize);
      }  
    }
  }
  return res;
}

NS_IMETHODIMP
msiEditor::InsertDecoration(const nsAString & above, const nsAString & below,
                            const nsAString & aroundNotation, const nsAString & aroundType)
{
  nsresult res(NS_ERROR_FAILURE);
  if (!(mFlags & eEditorPlaintextMask)) // copied from nsHTMLEditor -- I don't know if this is an issue
  {
    nsCOMPtr<nsISelection> selection;
    nsCOMPtr<nsIDOMNode> startNode, endNode;
    PRInt32 startOffset(0), endOffset(0);
    PRBool bCollapsed(PR_FALSE);
    res = GetNSSelectionData(selection, startNode, startOffset, endNode, 
                           endOffset, bCollapsed);
    if (NS_SUCCEEDED(res)) 
    {
      nsCOMPtr<nsIDOMNode> theNode;
      PRInt32 theOffset(0);
//      if (!bCollapsed)
//      {
//        res = DeleteSelection(nsIEditor::eNone); 
//      }
      theNode = startNode;
      theOffset = startOffset;
      if (NS_SUCCEEDED(res))
      {
        nsCOMPtr<nsIEditor> editor;
        QueryInterface(NS_GET_IID(nsIEditor), getter_AddRefs(editor));
        res = m_msiEditingMan->InsertDecoration(editor, selection, theNode, 
                                                theOffset, above, below, aroundNotation, aroundType);
      }  
    }
  }
  return res;
}

NS_IMETHODIMP
msiEditor::CreateReplaceTransaction(nsIDOMNode * newKid, nsIDOMNode * oldKid,
                                    nsIDOMNode * parent, nsITransaction ** transaction)
{
  if (!newKid || !oldKid || !parent || !transaction)
    return NS_ERROR_FAILURE;
  return CreateTxnForReplaceElement(newKid, oldKid, parent, PR_TRUE, (ReplaceElementTxn**)transaction);  
}                                    
                                            
NS_IMETHODIMP
msiEditor::CreateDeleteTransaction(nsIDOMNode * node, nsITransaction ** transaction)
{
  if (!node || !transaction)
    return NS_ERROR_FAILURE;
  return CreateTxnForDeleteElement(node, (DeleteElementTxn**)transaction);  
} 

NS_IMETHODIMP
msiEditor::CreateInsertTransaction(nsIDOMNode * node, nsIDOMNode * parent, PRUint32 offset, nsITransaction ** transaction)
{
  if (!node || !transaction || !parent)
    return NS_ERROR_FAILURE;
  return CreateTxnForInsertElement(node, parent, (PRInt32)offset, (InsertElementTxn**)transaction);  
} 

NS_IMETHODIMP
msiEditor::CreateDeleteTextTransaction(nsIDOMCharacterData * node,
                                       PRUint32 offset,
                                       PRUint32 numChar,
                                       nsITransaction ** transaction)
{
  if (!node || !transaction)
    return NS_ERROR_FAILURE;
  return CreateTxnForDeleteText(node, offset, numChar, (DeleteTextTxn**)transaction);  
}         

NS_IMETHODIMP
msiEditor::CreateDeleteChildrenTransaction(nsIDOMNode * parent,
                                           PRUint32 offset,
                                           PRUint32 numToDelete,
                                           nsITransaction ** transaction)
{
  if (!parent || !transaction)
    return NS_ERROR_FAILURE;
  nsCOMPtr<nsIDOMNodeList> childNodes;
  PRUint32 numKids(0);
  nsresult res = parent->GetChildNodes(getter_AddRefs(childNodes));
  if (NS_SUCCEEDED(res) && childNodes)
    res = childNodes->GetLength(&numKids);
  if (offset >= numKids || numToDelete == 0)
    return NS_OK;
  if (offset + numToDelete >= numKids)
    numToDelete = numKids - offset;    
  // allocate the out-param transaction
  EditAggregateTxn * aggTxn = nsnull;
  res = TransactionFactory::GetNewTransaction(EditAggregateTxn::GetCID(), (EditTxn **)&aggTxn);
  if (NS_FAILED(res) || !(aggTxn)) 
    return NS_ERROR_FAILURE;
  for (PRInt32 i=offset+numToDelete-1; ((i >= static_cast<PRInt32>(offset)) && NS_SUCCEEDED(res)); i--) 
  {
    nsCOMPtr<nsIDOMNode> currChild;
    res = childNodes->Item(i, getter_AddRefs(currChild));
    if (NS_SUCCEEDED(res) && currChild)
    {
      DeleteElementTxn *txn;
      res = CreateTxnForDeleteElement(currChild, &txn);
      if (NS_FAILED(res) || !txn)
        res = NS_ERROR_FAILURE;
      else
        aggTxn->AppendChild((EditTxn*)txn);
    }
  }
  if (NS_SUCCEEDED(res))
  {
    *transaction = aggTxn;
    NS_ADDREF(*transaction); 
  }
  return res;
}                                                                                                                  
           
NS_IMETHODIMP
msiEditor::CreateDeleteScriptTransaction(nsIDOMNode * script,
                                         nsIDOMNode * dummyChild,
                                         nsITransaction ** transaction)
{
  if (!script || !dummyChild || !transaction)
    return NS_ERROR_FAILURE;
  // allocate the out-param transaction
  EditAggregateTxn * aggTxn = nsnull;
  nsresult res = TransactionFactory::GetNewTransaction(EditAggregateTxn::GetCID(), (EditTxn **)&aggTxn);
  if (NS_FAILED(res) || !(aggTxn)) 
    return NS_ERROR_FAILURE;
  
  nsCOMPtr<nsIDOMNode> first, parent;
  script->GetFirstChild(getter_AddRefs(first));
  script->GetParentNode(getter_AddRefs(parent));
  
  ReplaceElementTxn * txn1 = nsnull;
  ReplaceElementTxn * txn2 = nsnull;
  PRBool deepRangeUpdate(PR_FALSE); 
  CreateTxnForReplaceElement(dummyChild, first, script, deepRangeUpdate, &txn1);  
  CreateTxnForReplaceElement(first, script, parent, deepRangeUpdate, &txn2);  
  if (txn1 && txn2)
  {
     aggTxn->AppendChild((EditTxn*)txn1);
     aggTxn->AppendChild((EditTxn*)txn2);
  }  
  if (NS_SUCCEEDED(res))
  {
    *transaction = aggTxn;
    NS_ADDREF(*transaction); 
  }
  return res;

}

NS_IMETHODIMP
msiEditor::CreateFlattenMrowTransaction(nsIDOMNode * mrow,
                                        nsITransaction ** transaction)
{
  if (!mrow || !transaction)
    return NS_ERROR_FAILURE;
  // allocate the out-param transaction
  FlattenMrowTxn * txn = nsnull;
  nsresult res = TransactionFactory::GetNewTransaction(FlattenMrowTxn::GetCID(), (EditTxn **)&txn);
  if (NS_FAILED(res) || !(txn)) 
    return NS_ERROR_FAILURE;
  nsCOMPtr<nsIEditor> editor;
  QueryInterface(NS_GET_IID(nsIEditor), getter_AddRefs(editor));
  if (!editor)
    return NS_ERROR_FAILURE;
  res = txn->Init(editor, mrow, &mRangeUpdater);
  if (NS_SUCCEEDED(res))
  {
    *transaction = txn;
    NS_ADDREF(*transaction); 
  }
  return res;
}
    
NS_IMETHODIMP
msiEditor::CreateReplaceScriptBaseTransaction(nsIDOMNode * script,
                                              nsIDOMNode * newbase,
                                              nsITransaction ** transaction)
{
  if (!script || !newbase || !transaction)
    return NS_ERROR_FAILURE;
  // allocate the out-param transaction
  ReplaceScriptBaseTxn * txn = nsnull;
  nsresult res = TransactionFactory::GetNewTransaction(ReplaceScriptBaseTxn::GetCID(), (EditTxn **)&txn);
  if (NS_FAILED(res) || !(txn)) 
    return NS_ERROR_FAILURE;
  nsCOMPtr<nsIEditor> editor;
  QueryInterface(NS_GET_IID(nsIEditor), getter_AddRefs(editor));
  if (!editor)
    return NS_ERROR_FAILURE;
  res = txn->Init(editor, script, newbase, &mRangeUpdater);
  if (NS_SUCCEEDED(res))
  {
    *transaction = txn;
    NS_ADDREF(*transaction); 
  }
  return res;
}    

//End nsIMathMLEditor
//

PRBool SpacesAtEndOfMathAddsSpace()
{
  nsresult rv;
  PRBool thePref;
  nsCOMPtr<nsIPrefBranch> prefBranch =
    do_GetService(NS_PREFSERVICE_CONTRACTID, &rv);

  if (NS_SUCCEEDED(rv) && prefBranch) {
    rv = prefBranch->GetBoolPref("swp.spaces.after.math", &thePref);
    return thePref;
  }
  return PR_FALSE;
}



NS_IMETHODIMP 
msiEditor::HandleKeyPress(nsIDOMKeyEvent * aKeyEvent)
{
  if (! aKeyEvent)  
    return NS_ERROR_NULL_POINTER;
  nsresult res(NS_OK);
  SetInComplexTransaction(PR_FALSE);

  if (!(mFlags & eEditorPlaintextMask)) // copied from nsHTMLEditor -- I don't know if this is an issue
  {
    PRUint32 keyCode(0), symbol(0);
    PRBool isShift(PR_FALSE), ctrlKey(PR_FALSE), altKey(PR_FALSE), metaKey(PR_FALSE);
    res = ExtractDataFromKeyEvent(aKeyEvent, keyCode, symbol, isShift, ctrlKey,
                                  altKey, metaKey);
// BBM
// Commenting the next block of code removes Larry's cursor handling code and restores
// (since preventDefault is not called) the Mozilla cursor handling
// Please note: the horizontal arrow handling is commented out for a reason
    if (//keyCode == nsIDOMKeyEvent::DOM_VK_LEFT  ||  keyCode == nsIDOMKeyEvent::DOM_VK_RIGHT ||
    keyCode == nsIDOMKeyEvent::DOM_VK_UP    ||  keyCode == nsIDOMKeyEvent::DOM_VK_DOWN)
   {
     PRBool preventDefault(PR_FALSE);
     if (keyCode == nsIDOMKeyEvent::DOM_VK_UP    ||  keyCode == nsIDOMKeyEvent::DOM_VK_DOWN) 
     {
       res = HandleArrowKeyPress(keyCode, isShift, ctrlKey, altKey, metaKey, preventDefault); 
       if (NS_SUCCEEDED(res) && preventDefault)
         aKeyEvent->PreventDefault();
       if (preventDefault) return NS_OK;
     }
   }
   // Check that the selection does not include table cells
   nsCOMPtr<nsIDOMElement> tableOrCellElement;
   res = GetFirstSelectedCell(nsnull, getter_AddRefs(tableOrCellElement));
   if (tableOrCellElement) {
     return res;
   }

    // Check for mapped characters -- function keys or one-shot mapping
    
    if (mKeyMap)
    {
      nsAutoString mapName;
      PRBool vk;
      nsAutoString script;
      nsAutoString error;
      PRUint16 mapType;
      PRUint32 newCharCode;
      PRBool fExists = PR_FALSE;
      PRBool reserved = PR_FALSE;
      if (m_fOneShot)
      {
        res = mKeyMap->MapExists(m_oneShotName, &mapType, &fExists );
        if (fExists) mapName.Assign(m_oneShotName);
        m_fOneShot = PR_FALSE;
      }
      else if ((keyCode >= nsIDOMKeyEvent::DOM_VK_F1) && (keyCode <= nsIDOMKeyEvent::DOM_VK_F24))
      {
        nsAutoString FkeyStr;
        FkeyStr = NS_LITERAL_STRING("FKeys");
        res = mKeyMap->MapExists(FkeyStr, &mapType, &fExists );
        if (fExists)
          mapName.Assign(FkeyStr);
      }
      if (fExists)
      {
        PRUint32 code;
        if (keyCode == 0)
        {
          code = symbol;
          vk = PR_FALSE;
          isShift = PR_FALSE;
        }
        else
        {
          code = keyCode;
          vk = PR_TRUE;
        }
        if (mapType == msiIKeyMap::CHARACTER) {
          res = mKeyMap->MapKeyToCharacter( mapName, code, altKey, ctrlKey, isShift, metaKey, vk, &reserved, &newCharCode );
          if (!reserved)
          {
            aKeyEvent->PreventDefault();
            if (!newCharCode) return NS_ERROR_UNEXPECTED;
            nsAutoString key(newCharCode);
            return TypedText(key, eTypedText);
          }
        }
        else
        {
          res = mKeyMap->MapKeyToScript(mapName, code, altKey, ctrlKey, isShift, metaKey, vk, &reserved, script);
          if (res == NS_OK)
          {
            if (!reserved)
            {
              nsCOMPtr<msiIScriptRunner> sr = do_CreateInstance("@mackichan.com/scriptrunner;1", &res);
              if (res == NS_OK)
              {
                sr->SetCtx(m_window);
                sr->Eval(script, error);
#ifdef DEBUG_Barry
                                if (error.Length() > 0) printf("Error in Eval: %S\n", error.BeginReading());
#endif
                aKeyEvent->PreventDefault();
                return NS_OK;
              }
            }
#ifdef DEBUG_Barry
            else printf("key %d is reserved\n", keyCode);
#endif
          }  
        }
      }
    }           
    if (symbol && !ctrlKey && !altKey && !metaKey)
    {
      PRBool collapsed(PR_FALSE);
      nsCOMPtr<msiISelection> msiSelection;
      nsCOMPtr<nsIDOMNode> mathnode;
      res = GetMSISelection(msiSelection);
      if (!msiSelection)
        return NS_ERROR_FAILURE;
      nsCOMPtr<nsISelection> nsSelection(do_QueryInterface(msiSelection));
      if (!nsSelection)
        return NS_ERROR_FAILURE;
      if (NS_SUCCEEDED(res)) 
        res = nsSelection->GetIsCollapsed(&collapsed);
      if (!collapsed)
        res = DeleteSelection(nsIEditor::eNone); // TODO 
      if (NS_SUCCEEDED(res))  
      {
        nsCOMPtr<nsIDOMNode> currFocusNode;
				nsCOMPtr<nsIDOMElement> currFocusElement;
        res = msiSelection->GetMsiFocusNode(getter_AddRefs(currFocusNode));
				nsAutoString name;
				nsCOMPtr<nsIDOMNode> tempNode = currFocusNode;
				PRUint16 type;
				res = tempNode->GetNodeType(& type);
				if (type == 3)
					currFocusNode->GetParentNode(getter_AddRefs(tempNode));
				currFocusElement = do_QueryInterface(tempNode);
				if (currFocusElement) res = currFocusElement->GetTagName(name);
				if (!name.EqualsLiteral("mtext"))
				{
	        res = NodeInMath(currFocusNode, getter_AddRefs(mathnode));
				}
        if (NS_SUCCEEDED(res) && currFocusNode && mathnode)
        {
          PRBool preventDefault(PR_FALSE);
          PRBool prefset(PR_FALSE);
          if (symbol == ' ')
          {
            // SWP actually has some special behavior if you're at the end of math
            prefset = SpacesAtEndOfMathAddsSpace();
            if (!isShift) {
              res = HandleArrowKeyPress(nsIDOMKeyEvent::DOM_VK_RIGHT, isShift, ctrlKey, altKey, metaKey, preventDefault); 
              // if preference is set, and we are now out of math, type a space
              if (prefset) 
              {
                 res = msiSelection->GetMsiFocusNode(getter_AddRefs(currFocusNode));
                 res = NodeInMath(currFocusNode, getter_AddRefs(mathnode));
                 if (!mathnode)
                   HandleKeyPress(aKeyEvent);
              }
            }
          }
          else if (symbol == '\t')
          {
            res = HandleArrowKeyPress(nsIDOMKeyEvent::DOM_VK_TAB, isShift, ctrlKey, altKey, metaKey, preventDefault); 
          }
          else if (symbol == '\'')
          {
            NS_NAMED_LITERAL_STRING(bigprime,"\x2032");
            res = InsertSuperscript();
            res = InsertSymbol(bigprime);  // need 'big prime'
            res = HandleArrowKeyPress(nsIDOMKeyEvent::DOM_VK_RIGHT, isShift, ctrlKey, altKey, metaKey, preventDefault); 
            preventDefault = PR_TRUE;
          }
          else {
            // res = nsEditor::BeginUpdateViewBatch();
            NS_NAMED_LITERAL_STRING(symbolStr," ");
            nsString str(symbolStr);
            PRUnichar * start = str.BeginWriting();
            *start = symbol;
            res = InsertSymbol(str);
            preventDefault = PR_TRUE;
            if (NS_SUCCEEDED(res))
		          res = CheckForAutoSubstitute(PR_TRUE);
            // res = nsEditor::EndUpdateViewBatch();
          }
          if (preventDefault)
            aKeyEvent->PreventDefault();
        }    
      }    
    }    
  }
  // Check that the selection does not include table cells
  nsCOMPtr<nsIDOMElement> tableOrCellElement;
  res = GetFirstSelectedCell(nsnull, getter_AddRefs(tableOrCellElement));
  if (tableOrCellElement) {
    return res;
  }
  // if not handled then pass along to nsHTMLEditor
  nsCOMPtr<nsIDOMNSUIEvent> nsUIEvent = do_QueryInterface(aKeyEvent);
  if(nsUIEvent) 
  {
    PRBool defaultPrevented;
    nsUIEvent->GetPreventDefault(&defaultPrevented);
    if (defaultPrevented)
      return res;
    else 
      // res = nsEditor::BeginUpdateViewBatch();
      res = nsHTMLEditor::HandleKeyPress(aKeyEvent);
      if (NS_SUCCEEDED(res) &&(!(mFlags & eEditorPlaintextMask)))
		    res = CheckForAutoSubstitute(PR_FALSE);
      // res = nsEditor::EndUpdateViewBatch();
      return res;
      
  }
  else
    return NS_ERROR_FAILURE;
}

// nsresult 
// msiEditor::DeleteSelectionImpl(nsIEditor::EDirection aAction)
// {
//   nsCOMPtr<nsISelection>selection;
//   nsresult res = GetSelection(getter_AddRefs(selection));
//   if (NS_FAILED(res)) 
//     return res;
//   msiSelectionManager msiSelMan(selection, this);
//   mRangeUpdater.RegisterSelectionState(msiSelMan);
//   EditAggregateTxn *txn;
//   res = CreateTxnForDeleteSelection(aAction, msiSelMan, &txn);
//   if (NS_FAILED(res)) 
//   {
//     mRangeUpdater.DropSelectionState(msiSelMan);
//     return res;
//   }
//   nsAutoRules beginRulesSniffing(this, kOpDeleteSelection, aAction);

//   PRInt32 i;
//   nsIEditActionListener *listener;
//   if (NS_SUCCEEDED(res))  
//   {
//     for (i = 0; i < mActionListeners.Count(); i++)
//     {
//       listener = (nsIEditActionListener *)mActionListeners[i];
//       if (listener)
//         listener->WillDeleteSelection(selection);
//     }

//     res = DoTransaction(txn);  

//     for (i = 0; i < mActionListeners.Count(); i++)
//     {
//       listener = (nsIEditActionListener *)mActionListeners[i];
//       if (listener)
//         listener->DidDeleteSelection(selection);
//     }
//   }
//   mRangeUpdater.DropSelectionState(msiSelMan);

//   // The transaction system (if any) has taken ownership of txn
//   NS_IF_RELEASE(txn);

//   return res;
// }

nsresult
msiEditor::CreateTxnForDeleteSelection(nsIEditor::EDirection aAction,
                                      msiSelectionManager & msiSelMan,
                                      EditAggregateTxn  ** aTxn)
{
  if (!aTxn)
    return NS_ERROR_NULL_POINTER;
  *aTxn = nsnull;

  nsCOMPtr<nsISelectionController> selCon = do_QueryReferent(mSelConWeak);
  if (!selCon) return NS_ERROR_NOT_INITIALIZED;
  nsCOMPtr<nsISelection> selection;
  nsresult result = selCon->GetSelection(nsISelectionController::SELECTION_NORMAL,
                                         getter_AddRefs(selection));
  if ((NS_SUCCEEDED(result)) && selection)
  {
    // Check whether the selection is collapsed and we should do nothing:
    PRBool isCollapsed;
    result = (selection->GetIsCollapsed(&isCollapsed));
    if (NS_SUCCEEDED(result) && isCollapsed && aAction == eNone)
      return NS_OK;

    // allocate the out-param transaction
    result = TransactionFactory::GetNewTransaction(EditAggregateTxn::GetCID(), (EditTxn **)aTxn);
    if (NS_FAILED(result)) 
      return result;
    PRUint32 rangeCount = msiSelMan.RangeCount();
    for (PRUint32 index = 0; index < rangeCount && NS_SUCCEEDED(result); index++)
    {
      PRBool rangeCollapsed(PR_TRUE);
      result = msiSelMan.IsRangeCollapsed(index, rangeCollapsed);
      if (NS_SUCCEEDED(result))
      {
        if (!rangeCollapsed)
        {
          msiDeleteRangeTxn *txn;
          result = TransactionFactory::GetNewTransaction(msiDeleteRangeTxn::GetCID(), (EditTxn **)&txn);
          nsRangeStore * rangeItem = msiSelMan.GetRangeStoreItem(index);
          if (NS_SUCCEEDED(result) && txn && rangeItem)
          {
            txn->Init(this, &msiSelMan, index, &mRangeUpdater);
            (*aTxn)->AppendChild(txn);
            NS_RELEASE(txn);
          }
          else
            result = NS_ERROR_OUT_OF_MEMORY;
        }
        else // we have an insertion point.  delete the thing in front of it or behind it, depending on aAction
          result = CreateTxnForDeleteInsertionPoint(msiSelMan, index, aAction, *aTxn);
      }
    }
  }

  // if we didn't build the transaction correctly, destroy the out-param transaction so we don't leak it.
  if (NS_FAILED(result))
    NS_IF_RELEASE(*aTxn);
  return result;
}

//ljh comment below from nsEditor's version of this method
//XXX: currently, this doesn't handle edge conditions because GetNext/GetPrior are not implemented
nsresult
msiEditor::CreateTxnForDeleteInsertionPoint(msiSelectionManager & msiSelMan,
                                            PRUint32 index, 
                                            nsIEditor::EDirection aAction,
                                            EditAggregateTxn     *aTxn)
{
  NS_ASSERTION(aAction == eNext || aAction == ePrevious, "invalid action");
  
  nsRangeStore * rangeItem = msiSelMan.GetRangeStoreItem(index);
  if (!rangeItem)
    return NS_ERROR_FAILURE;

  // get the node and offset of the insertion point
  nsCOMPtr<nsIDOMNode> node(rangeItem->startNode);
  PRInt32 offset(rangeItem->startOffset);
  if (!node || offset < 0)
    return NS_ERROR_FAILURE;
  nsresult result(NS_OK);
  // determine if the insertion point is at the beginning, middle, or end of the node
  nsCOMPtr<nsIDOMCharacterData> nodeAsText = do_QueryInterface(node);

  PRUint32 count(0);
  if (nodeAsText)
    nodeAsText->GetLength(&count);
  else
  { 
    // get the child list and count
    nsCOMPtr<nsIDOMNodeList>childList;
    result = node->GetChildNodes(getter_AddRefs(childList));
    if ((NS_SUCCEEDED(result)) && childList)
      childList->GetLength(&count);
  }

  PRBool isFirst = (0 == offset);
  PRBool isLast  = (count == (PRUint32)offset);

  // XXX: if isFirst && isLast, then we'll need to delete the node 
  //      as well as the 1 child

  // build a transaction for deleting the appropriate data
  // XXX: this has to come from rule section
  if ((ePrevious==aAction) && (PR_TRUE==isFirst))
  { // we're backspacing from the beginning of the node.  Delete the first thing to our left
    nsCOMPtr<nsIDOMNode> priorNode;
    result = GetPriorNode(node, PR_TRUE, address_of(priorNode));
    if ((NS_SUCCEEDED(result)) && priorNode)
    { // there is a priorNode, so delete it's last child (if text content, delete the last char.)
      // if it has no children, delete it
      nsCOMPtr<nsIDOMCharacterData> priorNodeAsText = do_QueryInterface(priorNode);
      if (priorNodeAsText)
      {
        PRUint32 length=0;
        priorNodeAsText->GetLength(&length);
        if (0<length)
        {
          DeleteTextTxn *txn;
          result = CreateTxnForDeleteCharacter(priorNodeAsText, length,
                                               ePrevious, &txn);
          if (NS_SUCCEEDED(result)) {
            aTxn->AppendChild(txn);
            NS_RELEASE(txn);
          }
        }
        else
        { // XXX: can you have an empty text node?  If so, what do you do?
#ifdef DEBUG_Barry
          printf("ERROR: found a text node with 0 characters\n");
#endif
          result = NS_ERROR_UNEXPECTED;
        }
      }
      else
      { // priorNode is not text, so tell it's parent to delete it
        DeleteElementTxn *txn;
        result = CreateTxnForDeleteElement(priorNode, &txn);
        if (NS_SUCCEEDED(result)) {
          aTxn->AppendChild(txn);
          NS_RELEASE(txn);
        }
      }
    }
  }
  else if ((nsIEditor::eNext==aAction) && (PR_TRUE==isLast))
  { // we're deleting from the end of the node.  Delete the first thing to our right
    nsCOMPtr<nsIDOMNode> nextNode;
    result = GetNextNode(node, PR_TRUE, address_of(nextNode));
    if ((NS_SUCCEEDED(result)) && nextNode)
    { // there is a nextNode, so delete it's first child (if text content, delete the first char.)
      // if it has no children, delete it
      nsCOMPtr<nsIDOMCharacterData> nextNodeAsText = do_QueryInterface(nextNode);
      if (nextNodeAsText)
      {
        PRUint32 length=0;
        nextNodeAsText->GetLength(&length);
        if (0<length)
        {
          DeleteTextTxn *txn;
          result = CreateTxnForDeleteCharacter(nextNodeAsText, 0, eNext, &txn);
          if (NS_SUCCEEDED(result)) {
            aTxn->AppendChild(txn);
            NS_RELEASE(txn);
          }
        }
        else
        { // XXX: can you have an empty text node?  If so, what do you do?
#ifdef DEBUG_Barry
          printf("ERROR: found a text node with 0 characters\n");
#endif
          result = NS_ERROR_UNEXPECTED;
        }
      }
      else
      { // nextNode is not text, so tell it's parent to delete it
        DeleteElementTxn *txn;
        result = CreateTxnForDeleteElement(nextNode, &txn);
        if (NS_SUCCEEDED(result)) {
          aTxn->AppendChild(txn);
          NS_RELEASE(txn);
        }
      }
    }
  }
  else
  {
    if (nodeAsText)
    { // we have text, so delete a char at the proper offset
      DeleteTextTxn *txn;
      result = CreateTxnForDeleteCharacter(nodeAsText, offset, aAction, &txn);
      if (NS_SUCCEEDED(result)) {
        aTxn->AppendChild(txn);
        NS_RELEASE(txn);
      }
    }
    else
    { // we're either deleting a node or some text, need to dig into the next/prev node to find out
      nsCOMPtr<nsIDOMNode> selectedNode;
      if (ePrevious==aAction)
      {
        result = GetPriorNode(node, offset, PR_TRUE, address_of(selectedNode));
      }
      else if (eNext==aAction)
      {
        result = GetNextNode(node, offset, PR_TRUE, address_of(selectedNode));
      }
      if (NS_FAILED(result)) { return result; }
      if (selectedNode) 
      {
        nsCOMPtr<nsIDOMCharacterData> selectedNodeAsText =
                                             do_QueryInterface(selectedNode);
        if (selectedNodeAsText)
        { // we are deleting from a text node, so do a text deletion
          PRUint32 position = 0;    // default for forward delete
          if (ePrevious==aAction)
          {
            selectedNodeAsText->GetLength(&position);
          }
          DeleteTextTxn *delTextTxn;
          result = CreateTxnForDeleteCharacter(selectedNodeAsText, position,
                                               aAction, &delTextTxn);
          if (NS_FAILED(result))  { return result; }
          if (!delTextTxn) { return NS_ERROR_NULL_POINTER; }
          aTxn->AppendChild(delTextTxn);
          NS_RELEASE(delTextTxn);
        }
        else
        {
          DeleteElementTxn *delElementTxn;
          result = CreateTxnForDeleteElement(selectedNode, &delElementTxn);
          if (NS_FAILED(result))  { return result; }
          if (!delElementTxn) { return NS_ERROR_NULL_POINTER; }
          aTxn->AppendChild(delElementTxn);
          NS_RELEASE(delElementTxn);
        }
      }
    }
  }
  return result;
}

NS_IMETHODIMP msiEditor::InsertText(const nsAString &aStringToInsert)
{
  if (!(mFlags & eEditorPlaintextMask)) // copied from nsHTMLEditor -- I don't know if this is an issue
  {
    nsresult res(NS_OK);
    nsCOMPtr<nsISelection> selection;
    nsCOMPtr<nsIDOMNode> startNode, endNode;
    PRInt32 startOffset(0), endOffset(0);
    PRBool bCollapsed(PR_FALSE);
    nsCOMPtr<nsIDOMNode> mathnode;
    res = GetNSSelectionData(selection, startNode, startOffset, endNode, endOffset, bCollapsed);
    res = NodeInMath(startNode, getter_AddRefs(mathnode));
    if (NS_SUCCEEDED(res) && mathnode && aStringToInsert.Length() > 0)
    {

      nsCOMPtr<nsIDOMNode> theNode;
      PRInt32 theOffset(0);
      if (!bCollapsed)
      {
        res = DeleteSelection(nsIEditor::eNone);
        //NS_ASSERTION(theNode,"need to set theNode");
        //if (NS_FAILED(res)) 
        //  return res;  // TODO -- is it not clear what to do here -- pass along to nsHTMLEditor
      }
      theNode = startNode;  
      theOffset = startOffset;

      if (aStringToInsert.Length() > 1)
        res = InsertMathnameEx(selection, theNode, theOffset, aStringToInsert);
      else  
        res = InsertSymbolEx(selection, theNode, theOffset, Substring(aStringToInsert,0,1));
        // This needs to be revisited. Symbols can be and are 2 or more characters. --BBM
      return res;
    }
    return nsHTMLEditor::InsertText(aStringToInsert);
  }
  return nsPlaintextEditor::InsertText(aStringToInsert);
}

/* nsIDOMNode NodeInMath (in nsIDOMNode node); */
NS_IMETHODIMP msiEditor::NodeInMath(nsIDOMNode *node, nsIDOMNode **_retval)
{
  nsCOMPtr<nsIDOMNode> checkNode;
  nsresult res;
  PRBool isMath;
  nsString name;
  *_retval = nsnull;
  if (IsTextContentNode(node))
    node->GetParentNode(getter_AddRefs(checkNode));
  else
    checkNode = node;  
  if (m_msiEditingMan && checkNode)
  {
    m_msiEditingMan->SupportsMathMLInsertionInterface(checkNode, &isMath);
    if (!isMath) *_retval = nsnull;
    res = checkNode->GetLocalName(name);
    while (checkNode && !name.EqualsLiteral("math"))
    {
	    if (name.EqualsLiteral("mtext"))
			{
				*_retval = nsnull;
				return NS_OK;
			}
      res = checkNode->GetParentNode(getter_AddRefs(checkNode));
      if (checkNode) res = checkNode->GetLocalName(name);
    }
    if (checkNode) *_retval = checkNode;
    NS_IF_ADDREF(*_retval);
  }
  return NS_OK;
}

/* nsIDOMNode RangeInMath (in nsIDOMRange range); */
NS_IMETHODIMP msiEditor::RangeInMath(nsIDOMRange *range, nsIDOMNode **_retval)
{
  nsCOMPtr<nsIArray> arrayOfNodes;
  nsCOMPtr<nsIDOMNode> currentNode; 
  nsCOMPtr<nsIDOMNode> mathNode;
  nsCOMPtr<nsIDOMNode> firstMathNode;
  PRUint32 length;
  nsresult res;
  res = NodesInRange(range, getter_AddRefs(arrayOfNodes));
  arrayOfNodes->GetLength(&length);
  
  if (length == 0) // no nodes, presumably all contained in a text node
  {
    res = range->GetStartContainer(getter_AddRefs(currentNode));
    res = NodeInMath(currentNode,getter_AddRefs(mathNode));
    *_retval = mathNode;
    return NS_OK;
  }
  for (PRInt32 i = (length-1); i>=0; i--)
  {
    currentNode = do_QueryElementAt(arrayOfNodes, i);
    if (!firstMathNode)
    {
      res = NodeInMath(currentNode, getter_AddRefs(firstMathNode));
      mathNode = firstMathNode;
    }
    else {
      res = NodeInMath(currentNode, getter_AddRefs(mathNode));
      if (mathNode && (mathNode != firstMathNode)) // the range is split between two or more math nodes
      {
        *_retval = nsnull;
        return NS_OK;
      }
    }
    if (!mathNode)
    {
      nsCOMPtr<nsIContent> content = do_QueryInterface(currentNode);
      if (!(content->TextIsOnlyWhitespace())) 
      {
        *_retval = nsnull;
        return NS_OK;
      }
      // return false only if there is a non-math, non-whitespace node in the range
    }
  }
  *_retval = firstMathNode;
  NS_IF_ADDREF(*_retval);
  return NS_OK;
}

/* nsIDOMNode SelectionInMath (); */
NS_IMETHODIMP msiEditor::SelectionInMath(nsIDOMNode **_retval)
{
  nsCOMPtr<nsISelection> selection;
  nsresult rv = GetSelection(getter_AddRefs(selection));
  NS_ENSURE_TRUE(selection, nsnull);
  PRInt32 count = 0;
  rv = selection->GetRangeCount(&count);
  NS_ENSURE_SUCCESS(rv, nsnull);

  if (count > 0) {
    nsCOMPtr<nsIDOMRange> range;
    rv = selection->GetRangeAt(0, getter_AddRefs(range));
    NS_ENSURE_SUCCESS(rv, nsnull);
    return RangeInMath(range, _retval);
  }
  return NS_ERROR_FAILURE;
}

nsresult msiEditor::GetMathParent(nsIDOMNode * node,
                                 nsCOMPtr<nsIDOMNode> & mathParent)
{
  nsresult res(NS_ERROR_FAILURE);
  mathParent = nsnull;
  nsCOMPtr<nsIDOMNode> p = node;
  if (!p)
   res = NS_ERROR_NULL_POINTER;
  while (p)
  {
    nsAutoString localName;
    p->GetLocalName(localName);
    if (localName.Equals(NS_LITERAL_STRING("math")))
    {
      mathParent = p;
      p = nsnull;
      res = NS_OK;
    }
    else
    {
      nsCOMPtr<nsIDOMNode> temp = p;
      temp->GetParentNode(getter_AddRefs(p));
    }
  }
  return res;
}   

nsresult msiEditor::ExtractDataFromKeyEvent(nsIDOMKeyEvent * aKeyEvent,
                                            PRUint32 & keyCode, PRUint32 & character,
                                            PRBool & isShift, PRBool & ctrlKey,
                                            PRBool & altKey, PRBool & metaKey)
{
  nsresult res(NS_OK);
  if (!aKeyEvent) 
    res = NS_ERROR_NULL_POINTER;
  else
  {
    aKeyEvent->GetKeyCode(&keyCode);
    aKeyEvent->GetShiftKey(&isShift);
    aKeyEvent->GetCtrlKey(&ctrlKey);
    aKeyEvent->GetAltKey(&altKey);
    aKeyEvent->GetMetaKey(&metaKey);
    // this royally blows: because tabs come in from keyDowns instead
    // of keyPress, and because GetCharCode refuses to work for keyDown
    // i have to play games.
    if (keyCode == nsIDOMKeyEvent::DOM_VK_TAB) 
      character = '\t';
    else 
      aKeyEvent->GetCharCode(&character);
  }
  return res;
}

PRBool msiEditor::IsTextContentNode(nsIDOMNode* node)
{
  nsCOMPtr<nsIContent> text(do_QueryInterface(node));
  return (text && text->GetText()) ? PR_TRUE : PR_FALSE;
}

nsresult msiEditor::GetNSSelectionData(nsCOMPtr<nsISelection> &selection,
                                       nsCOMPtr<nsIDOMNode> &startNode,
                                       PRInt32 &startOffset,
                                       nsCOMPtr<nsIDOMNode> &endNode,
                                       PRInt32 &endOffset,
                                       PRBool  &bCollapsed)
{
  nsresult res = GetSelection(getter_AddRefs(selection));
  if (NS_SUCCEEDED(res)) 
  {
    res = GetStartNodeAndOffset(selection, getter_AddRefs(startNode), &startOffset);
    if (NS_SUCCEEDED(res))
    {
      res = GetEndNodeAndOffset(selection, getter_AddRefs(endNode), &endOffset);
       if (NS_SUCCEEDED(res))

     res = selection->GetIsCollapsed(&bCollapsed);
    }
  }
  return res;
}

PRBool msiEditor::IsSelectionCollapsed()
{
  PRBool isCollapsed(PR_FALSE);
  nsCOMPtr<nsISelection> selection;
  nsresult res = GetSelection(getter_AddRefs(selection));
  if (NS_SUCCEEDED(res) && selection) 
    selection->GetIsCollapsed(&isCollapsed);
  return isCollapsed;
}

nsresult msiEditor::EnsureMathWithSelectionCollapsed(nsCOMPtr<nsIDOMNode> &node,
                                                     PRInt32 & offset)
{
  nsresult res(NS_OK);
  nsCOMPtr<nsIDOMNode> mathnode;
  res = NodeInMath(node, getter_AddRefs(mathnode));
  if (!mathnode)
  {
    //TODO What did Larry have in mind here? Collapsed is not being checked.
  res = NS_ERROR_FAILURE;
  }
  return res;
  
}

nsresult
msiEditor::InsertSubOrSup(PRBool isSup)
{
  nsresult res(NS_OK);
  if (!(mFlags & eEditorPlaintextMask)) // copied from nsHTMLEditor -- I don't know if this is an issue
  {
    nsCOMPtr<nsISelection> selection;
    nsCOMPtr<nsIDOMNode> startNode, endNode;
    PRInt32 startOffset(0), endOffset(0);
    PRBool bCollapsed(PR_FALSE);
    res = GetNSSelectionData(selection, startNode, startOffset, endNode, 
                          endOffset, bCollapsed);
    if (NS_SUCCEEDED(res)) 
    {
      nsCOMPtr<nsIDOMNode> theNode;
      PRInt32 theOffset(0);
      if (!bCollapsed)
     {
// TODO add stuff so that selected stuff is changed to become the base  or the script ?
// current SWP behavoir is to make it the script, but this may not be correct in light
// of the fact that sub and sup have a well defined base in mathml.
// Also need to deal with the case where we are not in math, or part of the selection is not
// in math.
      }
      else
      {
        if (IsTextContentNode(startNode))
        {
          startNode->GetParentNode(getter_AddRefs(theNode));
          theOffset = startOffset;
        }
        else
        {
          theNode = startNode;
          theOffset = startOffset;
        }
        nsCOMPtr<nsIDOMNode> mathnode;
        res = NodeInMath(theNode, getter_AddRefs(mathnode));
        if (!mathnode)
          res = EnsureMathWithSelectionCollapsed(theNode, theOffset);
      }
      if (NS_SUCCEEDED(res))
      {
        PRUint32 flags(msiIMathMLInsertion::FLAGS_NONE);
        nsCOMPtr<nsIEditor> editor;
        QueryInterface(NS_GET_IID(nsIEditor), getter_AddRefs(editor));
        nsAutoString scriptShift;
        res = m_msiEditingMan->InsertScript(editor, selection, theNode, theOffset,
                                            isSup, scriptShift);
      }
    }
  }
  return res;
}

nsresult
msiEditor::InsertSymbolEx(nsISelection * aSelection, nsIDOMNode * aNode, 
                          PRInt32 aOffset, const nsAString & aSymbol)
{
  nsresult res(NS_OK);
  nsCOMPtr<nsIEditor> editor;
  QueryInterface(NS_GET_IID(nsIEditor), getter_AddRefs(editor));
  res = m_msiEditingMan->InsertSymbol(editor, aSelection, aNode, aOffset, aSymbol);
  return res;
}

nsresult
msiEditor::InsertMathnameEx(nsISelection * aSelection, nsIDOMNode * aNode, 
                           PRInt32 aOffset, const nsAString & aMathname)
{
  nsresult res(NS_OK);
  nsCOMPtr<nsIEditor> editor;
  QueryInterface(NS_GET_IID(nsIEditor), getter_AddRefs(editor));
  res = m_msiEditingMan->InsertMathname(editor, aSelection, aNode, aOffset, aMathname);
  return res;
}

nsresult
msiEditor::InsertMathunitEx(nsISelection * aSelection, nsIDOMNode * aNode, 
                           PRInt32 aOffset, const nsAString & aMathunit)
{
  nsresult res(NS_OK);
  nsCOMPtr<nsIEditor> editor;
  QueryInterface(NS_GET_IID(nsIEditor), getter_AddRefs(editor));
  res = m_msiEditingMan->InsertMathunit(editor, aSelection, aNode, aOffset, aMathunit);
  return res;
}


nsresult
msiEditor::InsertEngineFunctionEx(nsISelection * aSelection, nsIDOMNode * aNode, 
                                  PRInt32 aOffset, const nsAString & aName)
{
  nsresult res(NS_OK);
  nsCOMPtr<nsIEditor> editor;
  QueryInterface(NS_GET_IID(nsIEditor), getter_AddRefs(editor));
  res = m_msiEditingMan->InsertEngineFunction(editor, aSelection, aNode, aOffset, aName);
  return res;
}

PRUint32
msiEditor::KeyCodeToCaretOp(PRUint32 keyCode, PRBool isShift, PRBool ctrlKey)
{
  //TODO -- How should this be handled -- data driven?
  PRUint32 rv(msiIMathMLCaret::CARET_NONE);
  if (keyCode == nsIDOMKeyEvent::DOM_VK_LEFT)
  {
    if (ctrlKey || isShift)
      rv = msiIMathMLCaret::CARET_OBJECTLEFT;
    else
      rv = msiIMathMLCaret::CARET_LEFT;
  }
  else if (keyCode == nsIDOMKeyEvent::DOM_VK_RIGHT)
  {
    if (ctrlKey || isShift)
      rv = msiIMathMLCaret::CARET_OBJECTRIGHT;
    else
      rv = msiIMathMLCaret::CARET_RIGHT;
  }
  else if (keyCode == nsIDOMKeyEvent::DOM_VK_UP)
  {
    if (!ctrlKey) //TODO -- hardwired to insert a superscript
    {
      if (isShift)
        rv = msiIMathMLCaret::CARET_OBJECTUP;
      else
        rv = msiIMathMLCaret::CARET_UP;
    }    
  }
  else if (keyCode == nsIDOMKeyEvent::DOM_VK_DOWN)
  {
    if (!ctrlKey) //TODO -- hardwired to insert a subscript
    {
      if (isShift)
        rv = msiIMathMLCaret::CARET_OBJECTDOWN;
      else
        rv = msiIMathMLCaret::CARET_DOWN;
    }    
  }
  else if (keyCode == nsIDOMKeyEvent::DOM_VK_TAB)
  {
    if (!ctrlKey) //TODO -- not sure what to do
    {
      if (isShift)
        rv = msiIMathMLCaret::TAB_LEFT;
      else
        rv = msiIMathMLCaret::TAB_RIGHT;
    }    
  }
  else
    NS_ASSERTION(keyCode==0,"Unknown/unhandled keycode.");
  return rv;
}

nsresult
msiEditor::GetNodeAndOffsetFromMMLCaretOp(PRUint32 caretOp, 
                                          nsCOMPtr<nsIDOMNode> & currNode,
                                          PRUint32 currOffset,
                                          nsCOMPtr<nsIDOMNode>& newNode,
                                          PRUint32 & newOffset)
{
  newNode = nsnull;
  newOffset = msiIMathMLEditingBC::INVALID;
  if (!currNode || currOffset > msiIMathMLEditingBC::LAST_VALID)
    return NS_ERROR_FAILURE;
  nsCOMPtr<msiIMathMLCaret> mathmlCaret;
  nsresult res = GetMathMLCaretInterface(currNode, currOffset, getter_AddRefs(mathmlCaret));
  if (NS_SUCCEEDED(res) && (caretOp != msiIMathMLCaret::CARET_NONE) && mathmlCaret)
  {
    switch(caretOp)
    {
      case msiIMathMLCaret::CARET_LEFT:
        res = mathmlCaret->CaretLeft(this, msiIMathMLCaret::FLAGS_NONE, getter_AddRefs(newNode), &newOffset);
      break;
      case msiIMathMLCaret::CARET_OBJECTLEFT:
        res = mathmlCaret->CaretObjectLeft(this, msiIMathMLCaret::FLAGS_NONE, getter_AddRefs(newNode), &newOffset);
      break;
      case msiIMathMLCaret::CARET_RIGHT:
        res = mathmlCaret->CaretRight(this, msiIMathMLCaret::FLAGS_NONE, getter_AddRefs(newNode), &newOffset);
      break;
      case msiIMathMLCaret::CARET_OBJECTRIGHT:
        res = mathmlCaret->CaretObjectRight(this, msiIMathMLCaret::FLAGS_NONE, getter_AddRefs(newNode), &newOffset);
      break;
      case msiIMathMLCaret::CARET_UP:
        res = mathmlCaret->CaretUp(this, msiIMathMLCaret::FLAGS_NONE, getter_AddRefs(newNode), &newOffset);
      break;
      case msiIMathMLCaret::CARET_OBJECTUP:
        res = mathmlCaret->CaretObjectUp(this, msiIMathMLCaret::FLAGS_NONE, getter_AddRefs(newNode), &newOffset);
      break;
      case msiIMathMLCaret::CARET_DOWN:
        res = mathmlCaret->CaretDown(this, msiIMathMLCaret::FLAGS_NONE, getter_AddRefs(newNode), &newOffset);
      break;
      case msiIMathMLCaret::CARET_OBJECTDOWN:
        res = mathmlCaret->CaretObjectDown(this, msiIMathMLCaret::FLAGS_NONE, getter_AddRefs(newNode), &newOffset);
      break;
      case msiIMathMLCaret::TAB_LEFT:
        res = mathmlCaret->TabLeft(this, getter_AddRefs(newNode), &newOffset);
      break;
      case msiIMathMLCaret::TAB_RIGHT:
        res = mathmlCaret->TabRight(this, getter_AddRefs(newNode), &newOffset);
      break;
      default:
        NS_ASSERTION(0,"Unhandled caretOp.");
        res = NS_ERROR_FAILURE;
      break;
    }
  }
  return res;
}         

nsresult msiEditor::GetMSISelection(nsCOMPtr<msiISelection> & msiSelection)
{
  msiSelection = nsnull;
  nsresult res(NS_ERROR_FAILURE);
  nsCOMPtr<nsISelection> selection;
  res = GetSelection(getter_AddRefs(selection));
  if (NS_SUCCEEDED(res) && selection)
    msiSelection = do_QueryInterface(selection);
  if (!msiSelection)
    res = NS_ERROR_FAILURE;
  return res;    
}

nsresult msiEditor::ComparePoints(nsIDOMNode * node1, PRUint32 offset1,
                                  nsIDOMNode * node2, PRUint32 offset2,
                                  PRInt32 *comparison)
{
  if (m_rangeUtils && node1 && node2 )
  {
    *comparison = m_rangeUtils->ComparePoints(node1, offset1, node2, offset2);
    return NS_OK;
  }
  else
    return NS_ERROR_FAILURE;
}                                  

nsresult msiEditor::AddMatrixRows(nsIDOMNode *aMatrix, PRUint32 insertAt, PRUint32 howMany)
{
  nsresult res(NS_OK);
  nsCOMPtr<nsIEditor> editor;
  QueryInterface(NS_GET_IID(nsIEditor), getter_AddRefs(editor));
  res = m_msiEditingMan->AddMatrixRows(editor, aMatrix, insertAt, howMany);
  return res;
}


nsresult msiEditor::AddMatrixColumns(nsIDOMNode *aMatrix, PRUint32 insertAt, PRUint32 howMany)
{
  nsresult res(NS_OK);
  nsCOMPtr<nsIEditor> editor;
  QueryInterface(NS_GET_IID(nsIEditor), getter_AddRefs(editor));
  res = m_msiEditingMan->AddMatrixColumns(editor, aMatrix, insertAt, howMany);
  return res;
}


nsresult msiEditor::GetMatrixSize(nsIDOMNode *aMatrix, PRInt32 *aRowCount, PRInt32 *aColCount)
{
  return m_msiEditingMan->GetMatrixSize(aMatrix, aRowCount, aColCount);
}


nsresult msiEditor::FindMatrixCell(nsIDOMNode *aMatrix, nsIDOMNode *aCell, PRInt32 *whichRow, PRInt32 *whichCol)
{
  return m_msiEditingMan->FindMatrixCell(aMatrix, aCell, whichRow, whichCol);
}


nsresult msiEditor::GetMatrixCellAt(nsIDOMNode *aMatrix, PRInt32 whichRow, PRInt32 whichCol, nsIDOMNode **_retval)
{
  return m_msiEditingMan->GetMatrixCellAt(aMatrix, whichRow, whichCol, _retval);
}

PRBool msiEditor::PositionIsAtBeginning(nsCOMPtr<nsIDOMNode> & parentNode, PRInt32 offset)
{
  if (offset == 0)
    return PR_TRUE;
  if (IsTextNode(parentNode))
  {
//    PRBool bEmptyTextNode = PR_FALSE;
//    nsresult rv = IsVisTextNode( parentNode, &bEmptyTextNode, PR_FALSE);
//    if (NS_SUCCEEDED(rv))
//      return bEmptyTextNode;
    return PR_FALSE;  //we know offset > 0
  }
  else
  {
    nsCOMPtr<nsIDOMNode> childNode;
    nsCOMPtr<nsIDOMNode> tmpNode;
    PRInt32 tmpOffset;
    nsresult rv = GetFirstEditableChild(childNode, address_of(childNode));
    if (NS_SUCCEEDED(rv))
    {
      rv = GetNodeLocation(childNode, address_of(tmpNode), &tmpOffset);
      if (NS_SUCCEEDED(rv))
        return (offset <= tmpOffset);
    }
  }
  return PR_FALSE;
}

PRBool msiEditor::PositionIsAtEnd(nsCOMPtr<nsIDOMNode> & parentNode, PRInt32 offset)
{
  PRUint32 len;
  GetLengthOfDOMNode(parentNode, len);
  if (offset >= (PRInt32)len)
    return PR_TRUE;
  if (IsTextNode(parentNode))
  {
//    PRBool bEmptyTextNode = PR_FALSE;
//    nsresult rv = IsVisTextNode( parentNode, &bEmptyTextNode, PR_FALSE);
//    if (NS_SUCCEEDED(rv))
//      return bEmptyTextNode;
    return PR_FALSE;    //we know offset < len
  }
  else
  {
    nsCOMPtr<nsIDOMNode> childNode;
    nsCOMPtr<nsIDOMNode> tmpNode;
    PRInt32 tmpOffset;
    nsresult rv = GetLastEditableChild(childNode, address_of(childNode));
    if (NS_SUCCEEDED(rv))
    {
      rv = GetNodeLocation(childNode, address_of(tmpNode), &tmpOffset);
      if (NS_SUCCEEDED(rv))
        return (offset >= tmpOffset);
    }
  }
  return PR_FALSE;
}

void nodeAncestorsOfType(const nsAString& specialTags,
  nsIDOMNode * node, nsAString& foundTags) {
  NS_NAMED_LITERAL_STRING(space, " ");
  NS_NAMED_LITERAL_STRING(text, "#text");
  nsAString::const_iterator start, end;

  foundTags = EmptyString();
  nsAutoString tagName;
  nsAutoString fatTagName;
  nsCOMPtr<nsIDOMNode> nodevar;
  nsCOMPtr<nsIDOMElement> elt;
  nodevar = node;
  if (!nodevar) return;
  while (nodevar) {
    elt = do_QueryInterface(nodevar);
    if (elt) {
      elt->GetTagName(tagName);
      fatTagName = space + tagName + space;
      specialTags.BeginReading(start);
      specialTags.EndReading(end);
      if (FindInReadable(fatTagName, start, end)) {
        foundTags += fatTagName;
      }
    }
    nodevar->GetParentNode(getter_AddRefs(nodevar));
  }
}

NS_IMETHODIMP 
msiEditor::RemoveDisplay( nsIDOMNode * focusNode, nsIDOMNode * anchorNode) {
  nsCOMPtr<nsIDOMElement> displayNode;
  nsresult rv(NS_OK);
  PRUint32 i;
  GetElementOrParentByTagName(NS_LITERAL_STRING("msidisplay"), focusNode, getter_AddRefs(displayNode));
  if (!displayNode) {
    GetElementOrParentByTagName(NS_LITERAL_STRING("msidisplay"), anchorNode, getter_AddRefs(displayNode));
  }
  if (displayNode) {
    BeginTransaction();
    nsCOMPtr<nsIDOMNodeList> nodeList;
    displayNode->GetElementsByTagName(NS_LITERAL_STRING("math"), getter_AddRefs(nodeList));
    PRUint32 listCount = 0;
    nodeList->GetLength(&listCount);

    for (i = 0; i < listCount; i++) {
      nsCOMPtr<nsIDOMNode> mathNode;
      nodeList->Item(i, getter_AddRefs(mathNode));
      nsCOMPtr<nsIDOMElement> item(do_QueryInterface(mathNode));
      if (item) {
        RemoveAttribute(item, NS_LITERAL_STRING("display"));
      } 
    }
    RemoveContainer(displayNode);
    EndTransaction();    
  }
  return rv;
}

NS_IMETHODIMP
msiEditor::CheckListItems(nsIDOMNode * leftBlock, nsIDOMNode * rightBlock, nsIDOMNode ** newLeftBlock, nsIDOMNode ** newRightBlock)
{
  nsresult res;
  *newLeftBlock = leftBlock;
  *newRightBlock = rightBlock;
  nsCOMPtr<nsIDOMElement> leftListItem;
  nsCOMPtr<nsIDOMElement> rightListItem;
  nsCOMPtr<nsIDOMElement> leftList;
  nsCOMPtr<nsIDOMElement> rightList;
  NS_NAMED_LITERAL_STRING(listitemtag, "listtag");
  res = GetElementOrParentByTagClass(listitemtag, leftBlock, getter_AddRefs(leftListItem));
  res = GetElementOrParentByTagClass(listitemtag, rightBlock, getter_AddRefs(rightListItem));
  if (leftListItem && rightListItem && (leftListItem != rightListItem)) {
    // we are spanning a list item. Join the list items instead of the paragraphs
    *newLeftBlock = leftListItem;
    *newRightBlock = rightListItem;
  }
  return NS_OK;
}

nsresult msiEditor::SetSelection(nsCOMPtr<nsIDOMNode> & focusNode, PRUint32 focusOffset, 
                                 PRBool selecting, PRBool & preventDefault)
{
//BBM ToDo Check for other non-text, non-structure, non-para tags.
  nsCOMPtr<nsIDOMNode> oldFocusNode;
  PRUint32 oldFocusOffset(msiIMathMLEditingBC::INVALID);

 if (!focusNode || focusOffset > msiIMathMLEditingBC::LAST_VALID)
    return NS_ERROR_FAILURE;
  nsCOMPtr<msiISelection> msiSelection;
  GetMSISelection(msiSelection);
  if (!msiSelection)
    return NS_ERROR_FAILURE;
    
  nsresult res(NS_OK);  
  PRBool collapse = !selecting;
  preventDefault = PR_FALSE;
  //TODO 
  //if (m_trackingMouse)
  //  m_msiEditor->SaveSelection(selection);
  if (selecting)
  {
    collapse = PR_FALSE;
    PRBool doSet(PR_FALSE);
    nsCOMPtr<nsIDOMNode> commonAncestor, anchorNode, startNode, endNode;
    nsCOMPtr<nsIDOMNodeList> childNodes;
    PRUint32 anchorOffset(msiIMathMLEditingBC::INVALID);
    PRUint32 startOffset(msiIMathMLEditingBC::INVALID), endOffset(msiIMathMLEditingBC::INVALID);
    PRInt32 compareFocusAnchor(0);
    PRInt32 compareOldFocusFocus(0);
    
    msiSelection->GetMsiAnchorNode(getter_AddRefs(anchorNode));
    msiSelection->GetMsiAnchorOffset(&anchorOffset);
    if (NS_SUCCEEDED(res) && anchorNode && anchorOffset <= msiIMathMLEditingBC::LAST_VALID)
    {
      ComparePoints(focusNode, focusOffset, anchorNode, anchorOffset, &compareFocusAnchor);
      res = GetCommonAncestor(anchorNode, focusNode, commonAncestor);
    }
    if (compareFocusAnchor == 0) // same point.
       collapse = PR_TRUE;
    else if (NS_SUCCEEDED(res) && commonAncestor)
    {
      msiSelection->GetMsiFocusNode(getter_AddRefs(oldFocusNode));
      msiSelection->GetMsiFocusOffset(&oldFocusOffset);
      ComparePoints(oldFocusNode, oldFocusOffset, focusNode, focusOffset, &compareOldFocusFocus);
      // BBM: We don't want to hightlight all of an msub when the selection crosses its boundary (for
      // consistency with version 5.5), and the same for msup, msubsup, mover, munder, munderover, mrow.

      NS_NAMED_LITERAL_STRING(specialTags,
        " msub msup msubsup mover munder munderover mrow ");
      nsString foundTags;
      nsString foundTags2;
      PRBool fSkipChanges = PR_FALSE;
      nodeAncestorsOfType(specialTags, focusNode, foundTags);
      nodeAncestorsOfType(specialTags, anchorNode, foundTags2);
      if (!foundTags.Equals(foundTags2)) fSkipChanges = PR_TRUE;
      if (fSkipChanges) return NS_OK;
      // BBM: Now check to see if we have crossed the boundary of an msiDisplay
      // foundTags = EmptyString();
      // foundTags2 = EmptyString();
      // NS_NAMED_LITERAL_STRING(displayTag, " msidisplay ");
      // nodeAncestorsOfType(displayTag, focusNode, foundTags);
      // nodeAncestorsOfType(displayTag, anchorNode, foundTags2);
      // if (!foundTags.Equals(foundTags2)) {
        RemoveDisplay(focusNode, anchorNode);
      //   return NS_OK;
      // }
      nsCOMPtr<msiIMathMLCaret> mathCaret;
      res = GetMathMLCaretInterface(commonAncestor, 0, getter_AddRefs(mathCaret));
      if (NS_SUCCEEDED(res) && mathCaret && !fSkipChanges)
      { 
      if (compareFocusAnchor < 0 ) //focus before anchor
      res = mathCaret->GetSelectableMathFragment(this, 
                                                focusNode, focusOffset,
                                                anchorNode, anchorOffset, 
                                                getter_AddRefs(startNode), &startOffset,
                                                getter_AddRefs(endNode), &endOffset);
      else // focus after anchor
      res = mathCaret->GetSelectableMathFragment(this, 
                                                anchorNode, anchorOffset,
                                                focusNode, focusOffset,
                                                getter_AddRefs(startNode), &startOffset,
                                                getter_AddRefs(endNode), &endOffset);
      if (NS_SUCCEEDED(res) && startNode && startOffset <= msiIMathMLEditingBC::LAST_VALID &&
       endNode && endOffset <= msiIMathMLEditingBC::LAST_VALID)
      doSet = PR_TRUE;                                                              
      }  
      else  // focus and/or anchor may be in math.
      {
      mathCaret = nsnull;  
      PRBool endSet(PR_FALSE), startSet(PR_FALSE);  
      nsCOMPtr<nsIDOMNode> dummyNode;
      PRUint32 dummyOffset(msiIMathMLEditingBC::INVALID);
      res = GetMathMLCaretInterface(focusNode, 0, getter_AddRefs(mathCaret));
      if (NS_SUCCEEDED(res) && mathCaret) //focusNode in math
      {
      mathCaret = nsnull;
      nsCOMPtr<nsIDOMNode>mathParent;
      res = GetMathParent(focusNode, mathParent);
      if (NS_SUCCEEDED(res) && mathParent)
       res = GetMathMLCaretInterface(mathParent, 0, getter_AddRefs(mathCaret));
      if (NS_SUCCEEDED(res) && mathCaret)
      {
//             nsCOMPtr<nsIDOMNode> oldFocusNode;
//             PRUint32 oldFocusOffset(msiIMathMLEditingBC::INVALID);
//    
//             msiSelection->GetMsiFocusNode(getter_AddRefs(oldFocusNode));
//             msiSelection->GetMsiFocusOffset(&oldFocusOffset);
//             ComparePoints(oldFocusNode, oldFocusOffset, focusNode, focusOffset, &compareOldFocusFocus);
             msiSelection->GetMsiAnchorNode(getter_AddRefs(anchorNode));
             msiSelection->GetMsiAnchorOffset(&anchorOffset);
             if (compareFocusAnchor < 0 ) //focus before anchor
             {
               res = mathCaret->GetSelectableMathFragment(this, 
                                                          focusNode, focusOffset,
                                                          nsnull, msiIMathMLEditingBC::INVALID,
                                                          getter_AddRefs(startNode), &startOffset,
                                                          getter_AddRefs(dummyNode), &dummyOffset);
               // The focus node has possibly been expanded to a larger object. We now need to determine whether this
               // will be added or subtracted from the selection (whether the selection is being enlarged or shrunken)
               if (compareOldFocusFocus < 0) // old focus is before the focus. We are shortening the selection.    
               {
                 startNode->GetChildNodes(getter_AddRefs(childNodes));
                 childNodes->GetLength(&startOffset);
               }
               startSet  = NS_SUCCEEDED(res) && startNode && startOffset <= msiIMathMLEditingBC::LAST_VALID;
             }
             else
             {
               res = mathCaret->GetSelectableMathFragment(this, 
                                                          nsnull, msiIMathMLEditingBC::INVALID,
                                                          focusNode, focusOffset,
                                                          getter_AddRefs(dummyNode), &dummyOffset,
                                                          getter_AddRefs(endNode), &endOffset);
               // The focus node has possibly been expanded to a larger object. We now need to determine whether this
               // will be added or subtracted from the selection (whether the selection is being enlarged or shrunken)
               if (compareOldFocusFocus > 0) // old focus is after the focus. We are shortening the selection.    
               {
                 endOffset = 0;
               }
               endSet  = NS_SUCCEEDED(res) && endNode && endOffset <= msiIMathMLEditingBC::LAST_VALID;
             }  
           }                                               
         }
         mathCaret = nsnull;
         res = GetMathMLCaretInterface(anchorNode, 0, getter_AddRefs(mathCaret));
         if (NS_SUCCEEDED(res) && mathCaret) //anchorNode in math
         {
           mathCaret = nsnull;
           nsCOMPtr<nsIDOMNode>mathParent;
           res = GetMathParent(anchorNode, mathParent);
           if (NS_SUCCEEDED(res) && mathParent)
             res = GetMathMLCaretInterface(mathParent, 0, getter_AddRefs(mathCaret));
           if (NS_SUCCEEDED(res) && mathCaret)
           {
             if (compareFocusAnchor < 0 ) //focus before anchor
             {
               res = mathCaret->GetSelectableMathFragment(this,
                                                          nsnull, msiIMathMLEditingBC::INVALID,
                                                          anchorNode, anchorOffset,
                                                          getter_AddRefs(dummyNode), &dummyOffset,
                                                          getter_AddRefs(endNode), &endOffset);
               if (compareOldFocusFocus < 0) // old focus is before the focus. We are shortening the selection.    
               {
                 endNode->GetChildNodes(getter_AddRefs(childNodes));
                 childNodes->GetLength(&endOffset);
               }
               endSet  = NS_SUCCEEDED(res) && endNode && endOffset <= msiIMathMLEditingBC::LAST_VALID;
             }
             else  // focus is after anchor
             {
               res = mathCaret->GetSelectableMathFragment(this, 
                                                          anchorNode, anchorOffset,
                                                          nsnull, msiIMathMLEditingBC::INVALID,
                                                          getter_AddRefs(startNode), &startOffset,
                                                          getter_AddRefs(dummyNode), &dummyOffset);
               if (compareOldFocusFocus > 0) // old focus is after the focus. We are shortening the selection.    
               {
                 startOffset = 0;
               }
               startSet  = NS_SUCCEEDED(res) && startNode && startOffset <= msiIMathMLEditingBC::LAST_VALID;
             }                                             
           }                                               
         }
         PRBool bEndIsFocus = PR_TRUE;
         if (startSet || endSet)
           doSet = PR_TRUE;
         if (!startSet)
         {
           if (compareFocusAnchor < 0 ) //focus before anchor
           {
             startNode = focusNode;
             startOffset = focusOffset;
           }
           else
           {
             startNode = anchorNode;
             startOffset = anchorOffset;
           }                                             
         }
         if(!endSet)
         {
           if (compareFocusAnchor < 0 ) //focus before anchor
           {
             endNode = anchorNode;
             endOffset = anchorOffset;
             bEndIsFocus = PR_FALSE;
           }
           else
           {
             endNode = focusNode;
             endOffset = focusOffset;
           }                                             
         }
         nsCOMPtr<nsIDOMNode> tmpNode;
         nsCOMPtr<nsIDOMNode> targNode;
         PRInt32 tmpOffset, targOffset;
         nsresult rv(NS_OK);
         PRBool bAtEdge = PR_TRUE;
         if (!bEndIsFocus && (compareOldFocusFocus > 0)) // old focus is after the focus; we're extending to the left
           bAtEdge = PR_FALSE;  //so we don't want to drop back to the right
         PRInt32 currOffset = startOffset;
         dummyNode = startNode;
         while (dummyNode && (dummyNode != commonAncestor))
         {
           rv = GetNodeLocation(dummyNode, address_of(tmpNode), &tmpOffset);
           bAtEdge = bAtEdge && PositionIsAtEnd(dummyNode, currOffset);  //If we ever find part of our node in the selection, bAtEdge can't be true
           if (NS_SUCCEEDED(rv) && ShouldSelectWholeObject(dummyNode))
           {
             targNode = tmpNode;  //parent of dummy
             if (bAtEdge)         //selection doesn't include any of our node; move out to right
               targOffset = tmpOffset + 1;
             else                                //mvoe out to left
               targOffset = tmpOffset;
           }
           dummyNode = tmpNode;
           currOffset = tmpOffset;
         }
         if (targNode)
         {
           startNode = targNode;
           startOffset = targOffset;
           doSet = PR_TRUE;
         }
         currOffset = endOffset;
         dummyNode = endNode;
         targOffset = 0;
         targNode = nsnull;
         bAtEdge = PR_TRUE;
         if (bEndIsFocus && (compareOldFocusFocus < 0)) // old focus is before the focus; we're extending to the right
           bAtEdge = PR_FALSE;  //so we don't want to fall back to the left
         while (dummyNode && (dummyNode != commonAncestor))
         {
           rv = GetNodeLocation(dummyNode, address_of(tmpNode), &tmpOffset);
           bAtEdge = bAtEdge && PositionIsAtBeginning(dummyNode, currOffset);
           if (NS_SUCCEEDED(rv) && ShouldSelectWholeObject(dummyNode))
           {
             targNode = tmpNode;  //parent of dummy
             if (bAtEdge)         //selection doesn't include any of our node; move out to left
               targOffset = tmpOffset;
             else                                //mvoe out to right
               targOffset = tmpOffset + 1;
           }
           dummyNode = tmpNode;
           currOffset = tmpOffset;
         }
         if (targNode)
         {
           endNode = targNode;
           endOffset = targOffset;
           doSet = PR_TRUE;
         }
         //Finally check commonAncestor itself to see whether we can select part of it:
         dummyNode = commonAncestor;
         targNode = nsnull;
         while (dummyNode && ShouldSelectWholeObject(dummyNode))
         {
           rv = GetNodeLocation(dummyNode, address_of(tmpNode), &tmpOffset);
           if (NS_SUCCEEDED(rv))
           {
             targNode = tmpNode;
             targOffset = tmpOffset;
           }
           dummyNode = targNode;
         }
         if (targNode)
         {
           startNode = targNode;
           startOffset = targOffset;
           endNode = targNode;
           endOffset = targOffset + 1;
           doSet = PR_TRUE;
         }
       } // end focus and/or anchor maybe in math
       if (doSet)
       {
         msiSelection->Set(startNode, startOffset, endNode, endOffset,
                           focusNode, focusOffset, anchorNode, anchorOffset);
            //               endNode, endOffset, anchorNode, anchorOffset);
         preventDefault = PR_TRUE;
       }  
    }
    else
     collapse = PR_TRUE;
  }    
  if (collapse)
  {
    nsCOMPtr<nsISelection> selection(do_QueryInterface(msiSelection));
    selection->Collapse(focusNode, focusOffset);
    preventDefault = PR_TRUE;
  }
  return res;
}

nsresult msiEditor::GetMayDrag(PRBool * mayDrag)
{
  if (!mMouseListenerP || !mayDrag)
    return NS_ERROR_FAILURE;
  *mayDrag = PR_FALSE;  
  nsresult res(NS_OK);
  nsCOMPtr<msiIMouse> msiMouse(do_QueryInterface(mMouseListenerP));
  if (msiMouse)  
    res = msiMouse->GetMayDrag(mayDrag);
  return res; 
}

nsresult msiEditor::IsPointWithinCurrentSelection(nsCOMPtr<nsIDOMNode> & node, PRUint32 offset, PRBool & withinSelection)
{
  withinSelection = PR_FALSE;
  if (!node || offset > msiIMathMLEditingBC::LAST_VALID)
    return NS_ERROR_FAILURE;
  nsCOMPtr<nsISelection> selection;
  nsresult res = GetSelection(getter_AddRefs(selection));
  if (NS_FAILED(res) || !selection)
    return NS_ERROR_FAILURE;
  
  PRInt32 rangeCount(0);
  res = selection->GetRangeCount(&rangeCount);
  for (PRInt32 i = 0; i < rangeCount && NS_SUCCEEDED(res) && !withinSelection; i++)
  {
    nsCOMPtr<nsIDOMRange> range;
    res = selection->GetRangeAt(i, getter_AddRefs(range));
    if (NS_FAILED(res) || !range)
      res = NS_ERROR_FAILURE;
    nsCOMPtr<nsIDOMNode> start, end;
    PRInt32 startOff(0), endOff(0);
    res = range->GetStartContainer(getter_AddRefs(start));
    if (NS_SUCCEEDED(res) && start)
      res = range->GetEndContainer(getter_AddRefs(end));
    else 
      res = NS_ERROR_FAILURE;
    if (NS_SUCCEEDED(res) && end)
      res = range->GetStartOffset(&startOff);
    else 
      res = NS_ERROR_FAILURE;
    if (NS_SUCCEEDED(res))
      res = range->GetEndOffset(&endOff);
    PRInt32 compareToStart(0), compareToEnd(0); 
    if (NS_SUCCEEDED(res))
      res = ComparePoints(node, offset, start, startOff, &compareToStart);
    if (NS_SUCCEEDED(res))
      res = ComparePoints(node, offset, end, endOff, &compareToEnd);
    if ( compareToStart >= 0 && compareToEnd <= 0)
      withinSelection = PR_TRUE;
  }
  return res;    
}  

nsresult 
msiEditor::HandleArrowKeyPress(PRUint32 keyCode, PRBool isShift, PRBool ctrlDown, 
                               PRBool altDown, PRBool metaDown, PRBool & preventDefault)
{
  preventDefault = PR_FALSE;
  if (altDown || metaDown || ctrlDown)
    return NS_OK;
  nsCOMPtr<msiISelection> msiSelection;
  GetMSISelection(msiSelection);
  if (!msiSelection)
    return NS_ERROR_FAILURE;
  nsCOMPtr<nsISelection> nsSelection(do_QueryInterface(msiSelection));
  if (!nsSelection)
    return NS_ERROR_FAILURE;
  nsresult res(NS_OK);
  PRInt32 compareFocusAnchor(0);  
  PRBool collapsed(PR_FALSE);
  nsCOMPtr<nsIDOMNode> msiFocus, msiAnchor;
  PRUint32 msiFocusOff(msiIMathMLEditingBC::INVALID), msiAnchorOff(msiIMathMLEditingBC::INVALID);
  
  if (NS_SUCCEEDED(res)) 
    res = nsSelection->GetIsCollapsed(&collapsed);
  if (NS_SUCCEEDED(res)) 
    res = msiSelection->GetMsiFocusNode(getter_AddRefs(msiFocus));
  if (NS_SUCCEEDED(res)) 
    res = msiSelection->GetMsiFocusOffset(&msiFocusOff);
  if (NS_SUCCEEDED(res)) 
    res = msiSelection->GetMsiAnchorNode(getter_AddRefs(msiAnchor));
  if (NS_SUCCEEDED(res)) 
    res = msiSelection->GetMsiAnchorOffset(&msiAnchorOff);
  if (NS_SUCCEEDED(res)) 
   res = ComparePoints(msiFocus, msiFocusOff, msiAnchor, msiAnchorOff, &compareFocusAnchor);
  if (NS_FAILED(res))
    return res;
  if (!msiFocus || msiFocusOff > msiIMathMLEditingBC::LAST_VALID || 
      !msiAnchor || msiAnchorOff > msiIMathMLEditingBC::LAST_VALID)
    return NS_ERROR_FAILURE;  
    
  if (!collapsed && !isShift) //not collapsed and not selecting -- so collapse the selection
  {
    nsCOMPtr<nsIDOMNode> left = compareFocusAnchor < 0 ? msiFocus : msiAnchor;
    PRUint32 leftOff = compareFocusAnchor < 0 ? msiFocusOff : msiAnchorOff;
    nsCOMPtr<nsIDOMNode> right = compareFocusAnchor < 0 ? msiAnchor : msiFocus;
    PRUint32 rightOff = compareFocusAnchor < 0 ? msiAnchorOff : msiFocusOff;
    
    
    if (keyCode == nsIDOMKeyEvent::DOM_VK_LEFT || keyCode == nsIDOMKeyEvent::DOM_VK_UP)
    {
      res = nsSelection->Collapse(left, leftOff);
      if (NS_SUCCEEDED(res))
      {
        preventDefault = PR_TRUE;
        collapsed = PR_TRUE;
      }
    }  
    else if (keyCode == nsIDOMKeyEvent::DOM_VK_RIGHT || keyCode == nsIDOMKeyEvent::DOM_VK_DOWN)
    {
      res = nsSelection->Collapse(right, rightOff);
      if (NS_SUCCEEDED(res))
      {
        preventDefault = PR_TRUE;
        collapsed = PR_TRUE;
      }
    }
    if (collapsed)
    { // reset these
      if (NS_SUCCEEDED(res)) 
        res = msiSelection->GetMsiFocusNode(getter_AddRefs(msiFocus));
      if (NS_SUCCEEDED(res)) 
        res = msiSelection->GetMsiFocusOffset(&msiFocusOff);
      if (NS_SUCCEEDED(res)) 
        res = msiSelection->GetMsiAnchorNode(getter_AddRefs(msiAnchor));
      if (NS_SUCCEEDED(res)) 
        res = msiSelection->GetMsiAnchorOffset(&msiAnchorOff);
      if (NS_SUCCEEDED(res)) 
       res = ComparePoints(msiFocus, msiFocusOff, msiAnchor, msiAnchorOff, &compareFocusAnchor);
      if (NS_FAILED(res))
        return res;
    }  
    if (!ctrlDown &&(keyCode == nsIDOMKeyEvent::DOM_VK_LEFT || keyCode == nsIDOMKeyEvent::DOM_VK_RIGHT))  //TODO this is SWP behavior
      return res;
  }
 
  nsCOMPtr<nsIDOMNode>currNode, newFocus;
  PRUint32 currOffset(msiIMathMLEditingBC::INVALID), newOffset(msiIMathMLEditingBC::INVALID);
  if (collapsed)
  {
    currNode = msiAnchor;
    currOffset = msiAnchorOff;
  }
  else
  {
    currNode = msiFocus;
    currOffset = msiFocusOff;
  }
  nsCOMPtr<nsIDOMNode> mathnode;
  res = NodeInMath(currNode, getter_AddRefs(mathnode));
  if (mathnode)
  {
    PRUint32 caretOp = KeyCodeToCaretOp(keyCode, isShift, ctrlDown);
    if (caretOp == msiIMathMLCaret::TAB_LEFT)  // SLS this seems really ugly
      isShift = PR_FALSE;
    res = GetNodeAndOffsetFromMMLCaretOp(caretOp, currNode, currOffset, newFocus, newOffset);
    if ((newFocus) && (newFocus != msiFocus || newOffset != msiFocusOff)) preventDefault = PR_TRUE;
  }
  else
  {
    preventDefault = PR_FALSE;
    return NS_OK;
//    //TODO -- I don't like this code. It uses nsEditor's GetNextNode and GetPriorNode to get a text node
//    //  and then I check if this node is in math -- kinda hacky and not robust.
//    nsCOMPtr<nsIDOMNode> testNode;
//    PRUint32 flags(msiIMathMLCaret::FLAGS_NONE);
//    if (keyCode == nsIDOMKeyEvent::DOM_VK_RIGHT)
//    {
//      if (IsTextContentNode(currNode))
//      {
//        nsCOMPtr<nsIDOMCharacterData> chardata(do_QueryInterface(currNode));
//        PRUint32 length(0);
//        if (chardata)
//        {
//          chardata->GetLength(&length);
//          if (length > 0)
//          {
//            if (currOffset == length)
//            {
//              nsCOMPtr<nsIDOMNode> nextnode;
//              GetNextNode(currNode, PR_FALSE, address_of(nextnode), PR_FALSE); 
//              if (nextnode)
//              {
//                testNode = nextnode;
//                newFocus = nextnode;
//                newOffset = 1;
//              }
//            }
//            else 
//            {
//              newFocus = currNode;
//              newOffset = currOffset+1;
//            }
//          }  
//        }  
//      }
//      else
//      {
//        nsCOMPtr<nsIDOMNode> nextnode;
//        GetNextNode(currNode, currOffset, PR_FALSE, address_of(nextnode), PR_FALSE); 
//        if (nextnode)
//        {
//          testNode = nextnode;
//          newFocus = currNode;
//          newOffset = 1;
//        }
//      }
//    }  
//    else if (keyCode == nsIDOMKeyEvent::DOM_VK_LEFT)
//    {
//      if (IsTextContentNode(currNode))
//      {
//        if (currOffset == 0)
//        {
//          nsCOMPtr<nsIDOMNode> priornode;
//          GetPriorNode(currNode, PR_FALSE, address_of(priornode), PR_FALSE); 
//          if (priornode)
//          {
//            testNode = priornode;
//            newFocus = currNode;
//            newOffset = 0; // BBM fix this
//          } 
//        }
//        else 
//        {
//          newFocus = currNode;
//          newOffset = currOffset-1;
//        }
//      }
//      else
//      {
//        nsCOMPtr<nsIDOMNode> priornode;
//        GetPriorNode(currNode, currOffset, PR_FALSE, address_of(priornode), PR_FALSE); 
//        if (priornode)
//          testNode = priornode;
//      }
//    }
//    if (testNode && NodeInMath(testNode))
//    {
//      nsCOMPtr<msiIMathMLCaret> mathmlEditing;
//      PRUint32 offset(msiIMathMLEditingBC::INVALID);
//      nsCOMPtr<nsIDOMNode> mathParent;
//      GetMathParent(testNode, mathParent);
//      if (mathParent)
//      {
//        if (keyCode == nsIDOMKeyEvent::DOM_VK_LEFT)
//        {
//          PRUint32 number(0);
//          nsCOMPtr<nsIDOMNodeList> childNodes;
//          res = mathParent->GetChildNodes(getter_AddRefs(childNodes));
//          if (NS_SUCCEEDED(res) && childNodes)
//            res = childNodes->GetLength(&number);
//          offset = number;  
//        }
//        else
//          offset = 0;
//        GetMathMLCaretInterface(mathParent, offset, getter_AddRefs(mathmlEditing));
//      }
//      if (mathmlEditing)
//      {
//        PRUint32 flags = keyCode == nsIDOMKeyEvent::DOM_VK_RIGHT ? msiIMathMLCaret::FROM_LEFT : msiIMathMLCaret::FROM_RIGHT;
//        res = mathmlEditing->Accept(this, flags, getter_AddRefs(newFocus), &newOffset);
//      }
//    }  
  }
  if (NS_SUCCEEDED(res))
  {
    if (newFocus && newOffset <= msiIMathMLEditingBC::LAST_VALID)
      res = SetSelection(newFocus, newOffset, isShift, preventDefault); 
  }
  return res;  
}


//ljh  GetCommonAncestor algorithm is copied from nsContentUtils::GetCommonAncestor.
// I decided to duplicate this function instead of bringing in all the baggage need to 
// link with nsContentUtils.

nsresult
msiEditor::GetCommonAncestor(nsIDOMNode * node1,
                             nsIDOMNode * node2,
                             nsCOMPtr<nsIDOMNode> & commonAncestor)
{
  commonAncestor = nsnull;
  if (!node1 || !node2)
    return NS_ERROR_FAILURE;
  
  if (node1 == node2) 
  {
    commonAncestor = node1;
    return NS_OK;
  }
  // Build the chain of parents
  nsCOMPtr<nsIDOMNode> tmpNode = node1;
  nsAutoVoidArray parents1, parents2;
  do {
    parents1.AppendElement(tmpNode);
    nsCOMPtr<nsIDOMNode> parent;
    tmpNode->GetParentNode(getter_AddRefs(parent));
    tmpNode = parent;
  } while (tmpNode);

  tmpNode = node2;
  do {
    parents2.AppendElement(tmpNode);
    nsCOMPtr<nsIDOMNode> parent;
    tmpNode->GetParentNode(getter_AddRefs(parent));
    tmpNode = parent;
  } while (tmpNode);

  // Find where the parent chain differs
  PRUint32 pos1 = parents1.Count();
  PRUint32 pos2 = parents2.Count();
  nsIDOMNode* parent = nsnull;
  PRUint32 len;
  for (len = PR_MIN(pos1, pos2); len > 0; --len) 
  {
    nsIDOMNode* child1 = static_cast<nsIDOMNode*>(parents1.FastElementAt(--pos1));
    nsIDOMNode* child2 = static_cast<nsIDOMNode*>(parents2.FastElementAt(--pos2));
    if (child1 != child2) 
      break;
    parent = child1;
  }
  commonAncestor = parent;
  return NS_OK;
}

//static
nsresult msiEditor::AdjustCaretCB(void* msieditor, nsIDOMEvent * mouseEvent, nsCOMPtr<nsIDOMNode> & node, PRInt32 &offset)
{
  if (msieditor)
    return ((msiEditor*)msieditor)->AdjustCaret(mouseEvent, node, offset);
  else
    return NS_ERROR_NULL_POINTER;
}

//static
nsresult msiEditor::SetSelectionCB(void* msieditor, nsCOMPtr<nsIDOMNode> & focusNode, PRInt32 focusOffset, PRBool selecting, PRBool & preventDefault)
{
  preventDefault = PR_FALSE;
  if (msieditor)
    return ((msiEditor*)msieditor)->SetSelection(focusNode, focusOffset, selecting, preventDefault);
  else
    return NS_ERROR_NULL_POINTER;
}

nsresult msiEditor::AdjustCaret(nsIDOMEvent * aMouseEvent, nsCOMPtr<nsIDOMNode> & node, PRInt32 &offset)
{
  if (!aMouseEvent || !node || offset< 0)
    return NS_OK;
  nsCOMPtr<nsIDOMMouseEvent> mouseEvent (do_QueryInterface(aMouseEvent));
  if (!mouseEvent) 
    return NS_OK;
  
  nsCOMPtr<nsIDOMNode> adjustedNode;
  PRUint32 adjustedOffset(msiIMathMLEditingBC::INVALID);
  nsCOMPtr<msiIMathMLCaret> mathCaret;  
  nsCOMPtr<nsIPresShell> presShell;
  GetPresShell(getter_AddRefs(presShell));
  
  nsresult res = GetMathMLCaretInterface(node, offset, getter_AddRefs(mathCaret));
  if (NS_SUCCEEDED(res) && mathCaret && presShell)
    res = mathCaret->AdjustNodeAndOffsetFromMouseEvent(this, presShell, msiIMathMLCaret::FLAGS_NONE, 
                                                       mouseEvent, getter_AddRefs(adjustedNode), &adjustedOffset);
  else
  {
    //TODO -- may be on a boundary
  }
  if (adjustedNode && IS_VALID_NODE_OFFSET(adjustedOffset))
  {
    node = adjustedNode;
    offset = adjustedOffset;
  }
  return NS_OK;
}  

//NS_IMETHODIMP 
//msiEditor::HandleKeyPress(nsIDOMKeyEvent * aKeyEvent)
//{
//  if (! aKeyEvent)  
//    return NS_ERROR_NULL_POINTER;
//  nsresult res(NS_OK);
//  if (!(mFlags & eEditorPlaintextMask)) // copied from nsHTMLEditor -- I don't know if this is an issue
//  {
//
//    nsCOMPtr<nsISelection> selection;
//    nsCOMPtr<nsIDOMNode> startNode, endNode;
//    PRInt32 startOffset(0), endOffset(0);
//    PRBool bCollapsed(PR_FALSE);
//    res = GetNSSelectionData(selection, startNode, startOffset, endNode, 
//                        endOffset, bCollapsed);
//    if (NS_SUCCEEDED(res)) 
//    {
//      PRUint32 keyCode(0), symbol(0);
//      PRBool isShift(PR_FALSE), ctrlKey(PR_FALSE), altKey(PR_FALSE), metaKey(PR_FALSE);
//      res = ExtractDataFromKeyEvent(aKeyEvent, keyCode, symbol, isShift, ctrlKey,
//                                   altKey, metaKey);
//      nsCOMPtr<nsIDOMNode> theNode;
//      PRInt32 theOffset(0);
//      if (NS_SUCCEEDED(res) && NodeInMath(startNode) && !ctrlKey && !altKey)
//      {
//        if (!bCollapsed)
//        {
//          res = DeleteSelection(nsIEditor::eNone);
//          // need to set "theNode"
//          if (NS_FAILED(res)) 
//            return res;  // TODO -- is it not clear what to do here -- pass along to nsHTMLEditor
//        }
//        else
//        {
//          theNode = startNode;  
//          theOffset = startOffset;
//        }
//        if (symbol)
//          res = InsertSymbol(symbol);
//        else if (keyCode == nsIDOMKeyEvent::DOM_VK_LEFT  || 
//                 keyCode == nsIDOMKeyEvent::DOM_VK_RIGHT ||
//                 keyCode == nsIDOMKeyEvent::DOM_VK_UP    || 
//                 keyCode == nsIDOMKeyEvent::DOM_VK_DOWN)
//        {
//          PRUint32 caretOp = KeyCodeToCaretOperation(keyCode, isShift, ctrlKey, altKey, metaKey);
//          res = ProcessCaretOperation(caretOp, theNode, theOffset, selection);
//        }
//        else if (keyCode)
//        {
//          res = NS_ERROR_NOT_IMPLEMENTED;
//          //TODO -- handle arrow keys in math for example
//        }
//        if (NS_SUCCEEDED(res))
//          aKeyEvent->PreventDefault();
//      }
//      else if (NS_SUCCEEDED(res) && !NodeInMath(startNode) && !altKey && !isShift && 
//               (keyCode == nsIDOMKeyEvent::DOM_VK_LEFT  || keyCode == nsIDOMKeyEvent::DOM_VK_RIGHT ||
//                keyCode == nsIDOMKeyEvent::DOM_VK_UP    || keyCode == nsIDOMKeyEvent::DOM_VK_DOWN  ))
//                                                               
//      {
//        if (!bCollapsed)
//        {
//          if (keyCode == nsIDOMKeyEvent::DOM_VK_LEFT  || keyCode == nsIDOMKeyEvent::DOM_VK_UP)
//          {
//            theNode = startNode;  
//            theOffset = startOffset;
//          }
//          else if (keyCode == nsIDOMKeyEvent::DOM_VK_RIGHT  || keyCode == nsIDOMKeyEvent::DOM_VK_DOWN)
//          {
//            theNode = endNode;  
//            theOffset = endOffset;
//          }
//          if (theNode)
//          {
//            BeginTransaction();
//            SaveSelection(selection);
//            res = selection->Collapse(theNode, theOffset);
//            EndTransaction();
//            if (NS_SUCCEEDED(res))
//              aKeyEvent->PreventDefault();
//          }
//        }
//        else  // selection is collapsed
//        {
//          //TODO -- I don't like this code it use nsEditor's GetNextNode and GetPriorNode to get a text node
//          //  and then I check is this node is in math -- kinda hacky and not robust.
//          nsCOMPtr<msiIMathMLCaret> mathmlEditing;
//          nsCOMPtr<nsIDOMNode> testNode;
//          PRUint32 flags(msiIMathMLCaret::FLAGS_NONE);
//          if (keyCode == nsIDOMKeyEvent::DOM_VK_RIGHT)
//          {
//            if (IsTextContentNode(startNode))
//            {
//              nsCOMPtr<nsIDOMCharacterData> chardata(do_QueryInterface(startNode));
//              PRUint32 length(0);
//              if (chardata)
//              {
//                chardata->GetLength(&length);
//                if (length > 0 && startOffset == length)
//                {
//                  nsCOMPtr<nsIDOMNode> nextnode;
//                  GetNextNode(startNode, PR_FALSE, address_of(nextnode), PR_FALSE); 
//                  if (nextnode)
//                    testNode = nextnode;
//                }
//              }  
//            }
//            else
//            {
//              nsCOMPtr<nsIDOMNode> nextnode;
//              GetNextNode(startNode, startOffset, PR_FALSE, address_of(nextnode), PR_FALSE); 
//              if (nextnode)
//                testNode = nextnode;
//            }
//          }  
//          else if (keyCode == nsIDOMKeyEvent::DOM_VK_LEFT)
//          {
//            if (IsTextContentNode(startNode))
//            {
//              if (startOffset == 0)
//              {
//                nsCOMPtr<nsIDOMNode> priornode;
//                GetPriorNode(startNode, PR_FALSE, address_of(priornode), PR_FALSE); 
//                if (priornode)
//                  testNode = priornode;
//              }
//            }
//            else
//            {
//              nsCOMPtr<nsIDOMNode> priornode;
//              GetPriorNode(startNode, startOffset, PR_FALSE, address_of(priornode), PR_FALSE); 
//              if (priornode)
//                testNode = priornode;
//            }
//          }
//          if (testNode && NodeInMath(testNode))
//          {
//            GetMathParent(testNode, theNode);
//            if (theNode)
//            {
//              if (keyCode == nsIDOMKeyEvent::DOM_VK_LEFT)
//              {
//                PRUint32 number(0);
//                nsCOMPtr<nsIDOMNodeList> childNodes;
//                res = theNode->GetChildNodes(getter_AddRefs(childNodes));
//                if (NS_SUCCEEDED(res) && childNodes)
//                  res = childNodes->GetLength(&number);
//                theOffset = number;  
//              }
//              else
//                theOffset = 0;
//              GetMathMLCaretInterface(theNode, theOffset, getter_AddRefs(mathmlEditing));
//            }
//          }
//          if (mathmlEditing)
//          {
//            nsCOMPtr<nsIEditor> editor;
//            QueryInterface(NS_GET_IID(nsIEditor), getter_AddRefs(editor));
//            if (mathmlEditing && editor)
//            {
//              nsCOMPtr<nsIDOMNode> node;
//              PRUint32 offset(msiIMathMLEditingBC::INVALID);
//              BeginTransaction();
//              SaveSelection(selection);
//              PRUint32 flags = keyCode == nsIDOMKeyEvent::DOM_VK_RIGHT ? msiIMathMLCaret::FROM_LEFT : msiIMathMLCaret::FROM_RIGHT;
//              res = mathmlEditing->Accept(editor, flags, getter_AddRefs(node), &offset);
//              if (NS_SUCCEEDED(res) && node && offset <= msiIMathMLEditingBC::LAST_VALID)
//              {
//                selection->Collapse(node, offset);
//                aKeyEvent->PreventDefault();
//              }  
//              EndTransaction();
//            }  
//          }
//        }    
//      }
//    }
//  }
//  // if not handled then pass along to nsHTMLEditor
//  nsCOMPtr<nsIDOMNSUIEvent> nsUIEvent = do_QueryInterface(aKeyEvent);
//  if(nsUIEvent) 
//  {
//    PRBool defaultPrevented;
//    nsUIEvent->GetPreventDefault(&defaultPrevented);
//    if (defaultPrevented)
//      return res;
//    else 
//      return nsHTMLEditor::HandleKeyPress(aKeyEvent);
//  }
//  else
//    return NS_ERROR_FAILURE;
//}


// a recursive function that walks the DOM backwards from a starting position and returns characters
// in reverse order. There are some conditions that will cause it to quit:
// 1. There are no more patterns that could match
// 2. We have hit the beginning of a tag other than a text formatting tag. We do not allow patterns to span
//    paragraphs or other block object, and we do not go into or out of math if we started on the outside or 
//    inside, respectively.
// 3. In math, we will quit once we encounter a node other than <mi>, <mn> or <mo>. If we encounter a multicharacter <mi>,
//    we set fCanEndHere to false until we are returning the first character in the <mi>. This keeps us from 
//    matching a proper subset of a multicharacter <mi> (but we can include al of the <mi> contents in a match).
// 4. Multiple white space characters will be coalesced into a space, and &invisibletimes is ignored. For this reason
//    we pass the last character matched (actually, a boolean telling if the last character matched was a space would
//    be sufficient).
//
//  If we return STATE_SUCCESS, the node and offset of the last checked character have to be returned.

PRBool TwoSpacesSwitchesToMath()
{
	nsresult rv;
	PRBool thePref;
	nsCOMPtr<nsIPrefBranch> prefBranch =
    do_GetService(NS_PREFSERVICE_CONTRACTID, &rv);

  if (NS_SUCCEEDED(rv) && prefBranch) {
		rv = prefBranch->GetBoolPref("swp.space.after.space", &thePref);
		return thePref;
  }
	return PR_FALSE;
}

nsresult 
msiEditor::GetNextCharacter( nsIDOMNode *nodeIn, PRUint32 offsetIn, nsIDOMNode ** nodeOut, PRUint32& offsetOut, PRBool inMath, PRUnichar prevChar, 
 PRInt32 & _result)
{
  if (!nodeIn) return NS_ERROR_NULL_POINTER;
  nsCOMPtr<nsIDOM3Node> textNode;
  nsCOMPtr<nsIDOMNode> node2;
  nsCOMPtr<nsIDOMNode> node3;
  nsCOMPtr<nsIDOMNode> pnode;
  PRUint32 offset, offset2;
  PRUint32 length = 0;
  PRBool fCanEndHere = PR_TRUE;
  PRBool fValidChar;
  nsAutoString theText;
  nsAutoString tag;
  nsAutoString parentTag;
  nsIAtom * atomNS;
  nsresult rv;
  offset = offsetIn;
  if (IsTextContentNode(nodeIn))
  {
    textNode = do_QueryInterface(nodeIn);
    textNode->GetTextContent(theText);
    if (offset > theText.Length()) offset = theText.Length();
    while ((offset > 0) && (PRInt32)(--offset) >= 0)
    {
//      while (prevChar == ' ' && theText[offset] == ' ') --offset;
      nodeIn->GetParentNode(getter_AddRefs(pnode));
      rv = mtagListManager->GetTagOfNode(pnode, &atomNS, tag);
      fValidChar = PR_TRUE;
      fCanEndHere = PR_TRUE;
      if (tag.EqualsLiteral("mi") || tag.EqualsLiteral("mo")) {
        fCanEndHere = (offset==0);
        nsCOMPtr<nsIDOMElement> nodeElement = do_QueryInterface(pnode);
        nsAutoString val;
        if (nodeElement) {
          // rv = nodeElement->GetAttribute(NS_LITERAL_STRING("msimathname"), val);
          // if (val.EqualsLiteral("true")) 
          // {
          //   fValidChar = PR_FALSE;
          // }
          if (fValidChar) {
            rv = nodeElement->GetAttribute(NS_LITERAL_STRING("msiunit"), val);
            if (val.EqualsLiteral("true")) {
              fValidChar = PR_FALSE;
            }
          }
          if (!fValidChar) {
            _result = msiIAutosub::STATE_FAIL;
            if (*nodeOut) // we did find a match earlier
            {
              // *nodeOut and offsetOut should still be valid
              NS_ADDREF(*nodeOut);
            } 
            return NS_OK;
          }          
        }
      }
      // check for double spaces in text mode; possible to convert to math
      pnode = nsnull;
      if (TwoSpacesSwitchesToMath())
      {
  			if (!inMath && (theText[offset] == ' ' || theText[offset] == 160) && (theText[offset - 1] == 160))
  			{
					*nodeOut = nodeIn;
					offsetOut = offset - 1;
					_result = msiIAutosub::STATE_SPECIAL; 
					return NS_OK;
				}
			}
			prevChar = theText[offset];
      m_autosub->NextChar(inMath, prevChar, & _result);
      if (_result == msiIAutosub::STATE_SUCCESS)
      {
        if (fCanEndHere)
        {
          *nodeOut = nodeIn;
          offsetOut = offset;
        }
        else
        {
          //The autosub code thought it saw a match, but here we are overriding that decision
          //We do not update nodeOut or offsetOut
          _result = msiIAutosub::STATE_FAIL;
        }
      }
      if (_result == msiIAutosub::STATE_FAIL) 
      {
        if (*nodeOut) // we did find a match earlier
        {
          // *nodeOut and offsetOut should still be valid
          NS_ADDREF(*nodeOut);
        } 
        return NS_OK;
      }
    }
  }
  // nodeIn is not a text node, or we have already gone through it
  PRBool fHasChildren;
  if (!nodeIn) return NS_OK;
  nodeIn->HasChildNodes(&fHasChildren);
  nsCOMPtr<nsIDOMNodeList> nodeList;
  if (fHasChildren)  // in particular *pNode is not a text node
  {
    nodeIn->GetChildNodes( getter_AddRefs(nodeList));
    nodeList->GetLength(&length);
    offset2 = (PRUint32)(-1);
    while (length > 0 && --length >= 0)
    {
      nodeList->Item(length, getter_AddRefs(node2));
      GetNextCharacter(node2, offset2, nodeOut, offsetOut, inMath, prevChar,  _result);
      if (_result == msiIAutosub::STATE_FAIL)
      {
        if (*nodeOut) // we did find a match earlier
        {
          // *nodeOut and offsetOut should still be valid
          NS_ADDREF(*nodeOut);
        } 
        return NS_OK;
      }
    }
  }
  node2 = nsnull;
  nsCOMPtr<nsIDOMNode> tempnode;
  nsCOMPtr<nsIDOMNode> parentNode;
  tempnode = nodeIn;
  PRBool validNode = PR_TRUE;
  while (node2 == nsnull)
  {
    tempnode->GetPreviousSibling(getter_AddRefs(node2));

    // if no previous sibling under this node, go up one level and try again
    if (!node2)
    {
      tempnode->GetParentNode(getter_AddRefs(node2));
      if (!node2)
      {
        _result = msiIAutosub::STATE_FAIL;
        if (*nodeOut) // we did find a match earlier
        {
          // *nodeOut and offsetOut should still be valid
          NS_ADDREF(*nodeOut);
        } 
         return NS_OK; // no more nodes available. return _result.
      }
      else
      {
        // there are cases where we can't use the previous sibling, such as when the nodes are children of elements where
        // the children correspond to visually distinct entities, as in fractions, subscripts, superscripts, etc.
        node2->GetParentNode(getter_AddRefs(parentNode));
        rv = mtagListManager->GetTagOfNode(parentNode, &atomNS, parentTag);
        rv = mtagListManager->GetTagOfNode(node2, &atomNS, tag);
        if (tag.EqualsLiteral("mi") || tag.EqualsLiteral("mo")) {
          if (!(parentTag.EqualsLiteral("mrow") || parentTag.EqualsLiteral("math") ||
            parentTag.EqualsLiteral("msqrt") )) {
            validNode = PR_FALSE;
          }
          nsCOMPtr<nsIDOMElement> nodeElement = do_QueryInterface(node2);
          nsAutoString val;
          // rv = nodeElement->GetAttribute(NS_LITERAL_STRING("msimathname"), val);
          // if (val.EqualsLiteral("true")) 
          // {
          //   validNode = PR_FALSE;
          // }
          if (validNode) {
            rv = nodeElement->GetAttribute(NS_LITERAL_STRING("msiunit"), val);
            if (val.EqualsLiteral("true")) {
              validNode = PR_FALSE;
            }
          }
        }
        PRBool fTagIsTextTag;
        rv =  mtagListManager->GetTagInClass(NS_LITERAL_STRING("texttag"),tag,atomNS, &fTagIsTextTag);
        if (!(fTagIsTextTag || ((tag.EqualsLiteral("mi") || tag.EqualsLiteral("mo")) && validNode) || tag.EqualsLiteral("mo") || tag.EqualsLiteral("mn")))  
        {
          _result = msiIAutosub::STATE_FAIL;
          if (*nodeOut) // we did find a match earlier
          {
            // *nodeOut and offsetOut should still be valid
            NS_ADDREF(*nodeOut);
          } 
          return NS_OK;
        }
      }
      tempnode = node2;
      node2 = nsnull; // go around again to get the previous sibling
    }
  }
  offset2 = (PRUint32)(-1);
  if (node2) GetNextCharacter(node2, offset2, nodeOut, offsetOut, inMath, prevChar, _result); 
  return NS_OK;
}
  

nsresult
msiEditor::CheckForAutoSubstitute(PRBool inmath)
{
  nsresult res = NS_OK;
  if (!m_autosub) return NS_ERROR_FAILURE;
  nsCOMPtr<nsISelection> selection;
  GetSelection(getter_AddRefs(selection)); 
  if (!selection) return res;
  // this is called immediately after an insertion, so the selection is collapsed. Thus we can check any of of the
  // nodes.
  nsCOMPtr<nsIDOMNode> node;
  nsCOMPtr<nsIDOM3Node> textNode;
  nsCOMPtr<nsIDOMNode> originalNode = nsnull;
  PRUnichar ch = 0;
  PRInt32 ctx, action; 
  PRInt32 intOffset;
  PRUint32 offset = 0;
  nsAutoString theText;
  PRInt32 lookupResult;
  nsAutoString data, pasteContext, pasteInfo, error;       
  selection->GetAnchorNode((nsIDOMNode **) getter_AddRefs(originalNode));
  if (!originalNode) return res;
  selection->GetAnchorOffset( &intOffset );
  offset = (PRUint32)intOffset;
  m_autosub->Reset();
//  if (IsTextContentNode(node))
//  {
//    textNode = do_QueryInterface(node);
//    textNode->GetTextContent(theText);
//  }
  PRUint32 originalOffset = offset;
  GetNextCharacter(originalNode, originalOffset, getter_AddRefs(node), offset, inmath, ch, lookupResult);
  if (node)  // there was success somewhere
  {
//    SetInComplexTransaction(PR_TRUE);
    if (lookupResult == msiIAutosub::STATE_SPECIAL)
		{
			ctx =	msiIAutosub::CONTEXT_TEXTONLY; 
			action = msiIAutosub::ACTION_EXECUTE;
			data = NS_LITERAL_STRING("inserttext(' '); msiGoDoCommand('cmd_MSImathtext')"); 
			pasteContext = NS_LITERAL_STRING(""); 
			pasteInfo = NS_LITERAL_STRING("");
		}
		else
			m_autosub->GetCurrentData(&ctx, &action, pasteContext, pasteInfo, data);
    if ((ctx!=msiIAutosub::CONTEXT_TEXTONLY) == inmath || 
      inmath != (ctx!=msiIAutosub::CONTEXT_MATHONLY))
    {
      selection->Collapse(node, offset);
      selection->Extend(originalNode,originalOffset);
      if (action == msiIAutosub::ACTION_SUBSTITUTE)
        InsertHTMLWithContext(data, pasteContext, pasteInfo, NS_LITERAL_STRING("text/html"), nsnull, nsnull, 0, PR_TRUE); 
      else if (action == msiIAutosub::ACTION_EXECUTE)
      {
        nsCOMPtr<msiIScriptRunner> sr = do_CreateInstance("@mackichan.com/scriptrunner;1", &res);
        if (res == NS_OK)
        {
          sr->SetCtx(m_window);
          sr->Eval(data, error);
#ifdef DEBUG_Barry
          if (error.Length() > 0) printf("Error in Eval: %S\n", error.BeginReading());
#endif
        }
      }
//      selection->Collapse(node, offset);
//      selection->Extend(originalNode, originalOffset);
//      res = DeleteSelection(nsIEditor::eNone);
    }
//    SetInComplexTransaction(PR_FALSE);
  }
  return res;
}

//nsresult
//msiEditor::CheckForUnicodeNumber(PRBool inmath)
//{
//  nsresult res = NS_OK;
//  nsCOMPtr<nsISelection> selection;
//  GetSelection(getter_AddRefs(selection)); 
//  if (!selection) return res;
//  nsCOMPtr<nsIDOMNode> node;
//  nsCOMPtr<nsIDOM3Node> textNode;
//  nsCOMPtr<nsIDOMNode> originalNode;
//  PRUnichar ch = 0;
//  PRInt32 ctx, action; 
//  PRInt32 intOffset;
//  PRUint32 offset;
//  nsAutoString theText;
//  PRInt32 lookupResult;
//  nsAutoString data, pasteContext, pasteInfo, error;       
//  selection->GetFocusNode((nsIDOMNode **) getter_AddRefs(originalNode));
//  if (!originalNode) return res;
//  selection->GetFocusOffset( &intOffset );
//  offset = (PRUint32)intOffset;
//  PRUint32 originalOffset =ffset;
//  GetNextCharacter(originalNode, originalOffset, getter_AddRefs(node), offset, ch, lookupResult);
//  if (node)  // there was success somewhere
//  {
//    m_autosub->GetCurrentData(&ctx, &action, pasteContext, pasteInfo, data);
//    if ((ctx!=msiIAutosub::CONTEXT_TEXTONLY) == inmath || 
//      inmath != (ctx!=msiIAutosub::CONTEXT_MATHONLY))
//    {
//      selection->Collapse(node, offset);
//      selection->Extend(originalNode,originalOffset);
//      if (action == msiIAutosub::ACTION_SUBSTITUTE)
//        InsertHTMLWithContext(data, pasteContext, pasteInfo, NS_LITERAL_STRING("text/html"), nsnull, nsnull, 0, PR_TRUE); 
//      else if (action == msiIAutosub::ACTION_EXECUTE)
//      {
//        nsCOMPtr<msiIScriptRunner> sr = do_CreateInstance("@mackichan.com/scriptrunner;1", &res);
//        if (res == NS_OK)
//        {
//          sr->SetCtx(m_window);
//          sr->Eval(data, error);
//          if (error.Length() > 0) printf("Error in Eval: %S\n", error.BeginReading());
//        }
//      }
//    }
//  }
//  return res;
//}

NS_IMETHODIMP
msiEditor::InitRules()
{
  nsresult res = NS_NewMSIEditRules(getter_AddRefs(mRules));
  if (NS_FAILED(res)) return res;
  if (!mRules) return NS_ERROR_UNEXPECTED;
  res = mRules->Init(static_cast<nsPlaintextEditor*>(this), mFlags);
  
  return res;
}



NS_IMETHODIMP
msiEditor::AdjustRange(nsIDOMRange * aRange, PRBool isForDeletion, PRUint32 direction)
{
  nsresult res = NS_OK;
  nsCOMPtr<nsIDOMNode> startNode;
  res = aRange->GetStartContainer(getter_AddRefs(startNode));
  nsCOMPtr<nsIDOMNode> endNode;
  res = aRange->GetEndContainer(getter_AddRefs(endNode));
  // BBM: can we assume startNode comes before endNode?
  nsCOMPtr<nsIDOMNode> commonAncestor;
  nsCOMPtr<nsIDOMNode> newContainer;
  nsCOMPtr<nsIDOMNode> tempNode;
  nsCOMPtr<nsINode> node;
  PRUint32 endOffset;
  nsAutoString tagName;
  res = aRange->GetCommonAncestorContainer(getter_AddRefs(commonAncestor));
  while (startNode && (startNode != commonAncestor))
  {
    startNode->GetLocalName(tagName);
    if (  tagName.EqualsLiteral("mfrac")    || tagName.EqualsLiteral("mroot") ||
          tagName.EqualsLiteral("mtable")    || tagName.EqualsLiteral("mtd") ||
          tagName.EqualsLiteral("mtr")    || tagName.EqualsLiteral("msub") ||
          tagName.EqualsLiteral("msup")    || tagName.EqualsLiteral("msubsup") ||
          tagName.EqualsLiteral("msqrt"))
    {
      newContainer = startNode;
    }
    startNode->GetParentNode(getter_AddRefs(tempNode));
    startNode = tempNode;
  }
  if (newContainer)
    aRange->SetStartBefore(newContainer);
  newContainer = nsnull;
  while (endNode && (endNode != commonAncestor))
  {
    endNode->GetLocalName(tagName);
    if (  tagName.EqualsLiteral("mfrac")    || tagName.EqualsLiteral("mroot") ||
          tagName.EqualsLiteral("mtable")    || tagName.EqualsLiteral("mtd") ||
          tagName.EqualsLiteral("mtr")    || tagName.EqualsLiteral("msub") ||
          tagName.EqualsLiteral("msup")    || tagName.EqualsLiteral("msubsup") ||
          tagName.EqualsLiteral("msqrt"))
    {
      newContainer = endNode;
      node = do_QueryInterface(endNode);
      endOffset = 1 + node->GetChildCount();
    }
    endNode->GetParentNode(getter_AddRefs(tempNode));
    endNode = tempNode;
  }
  if (newContainer)
    aRange->SetEndAfter(newContainer);

  return res;
}


NS_IMETHODIMP
msiEditor::AdjustSelectionEnds(PRBool isForDeletion, PRUint32 direction)
{
  nsresult res = NS_OK;
//  PRInt32 rangeCount;
//  PRUint32 i;
  nsCOMPtr<nsISelection> sel;
  nsCOMPtr<nsIDOMRange> range;
  nsCOMPtr<nsIDOMRange> modrange;
  nsCOMPtr<nsIDOMNode>nodeContainerStart;
  nsCOMPtr<nsIDOMNode>nodeContainerEnd;
  PRInt32 offsetStart;
  PRInt32 offsetEnd;
  res = GetSelection(getter_AddRefs(sel));
//  res = sel->GetRangeCount(&rangeCount);
//  rangeCount = 1;
//  for (i = 0; i < rangeCount; i++)
 // {
    sel->GetRangeAt(/*i*/0, getter_AddRefs(range));
  // the code here was causing crashes, possibly because Larry's added info wasn't being sent on to the original
  // ranges in the selection. This is an attempt to build a correct modrange.
    range->CloneRange(getter_AddRefs(modrange));
    GetStartNodeAndOffset(sel, getter_AddRefs(nodeContainerStart), &offsetStart);
    GetEndNodeAndOffset(sel, getter_AddRefs(nodeContainerEnd), &offsetEnd);
    modrange->SetStart(nodeContainerStart, offsetStart);
    modrange->SetEnd(nodeContainerEnd, offsetEnd);
    AdjustRange(modrange, isForDeletion, direction);
    modrange->GetStartContainer(getter_AddRefs(nodeContainerStart));
    modrange->GetStartOffset(&offsetStart);
    modrange->GetEndContainer(getter_AddRefs(nodeContainerEnd));
    modrange->GetEndOffset(&offsetEnd);
    sel->Collapse(nodeContainerStart, offsetStart);
//    sel->Extend(nodeContainerEnd, offsetEnd);
 // }
  return res;
}

//The plan of this function:
//  (1) First we start at the split point and traverse up the tree looking for nodes which will respond to a return by inserting a
//      matrix, or if we find ourselves in a 1-column matrix, by adding a row to an existing one. These are "template" items with
//      a specified number of children. If we find one, then we record the position at which the new matrix should be inserted - either
//      a given child position or "-1" in the case of single-child math nodes (that is, those which create "inferred mrows" within them).
//      If we don't find any such node, then we want to leave the split point intact and allow the usual Split() to proceed.
//  (2) We now know that a matrix or matrix row is to be inserted and need to determine where to split the existing contents to
//      distribute among the new (and existing, in the case that we're within a one-column matrix) cells. 
//  (2) Then we start at the split point and traverse up the tree looking for nodes which do split (in the sense of the function
//      SplitNodesDeep() - that is, into two nodes of the same sort)
nsresult
msiEditor::InsertReturnInMath( nsIDOMNode * splitpointNode, PRInt32 splitpointOffset, PRBool* bHandled)
{
  nsCOMPtr<nsIDOMNode> topMathNode;
  *bHandled = PR_FALSE;
  GetMathParent(splitpointNode, topMathNode);
  if (!topMathNode)
    return NS_OK;  //return this without setting bHandled - insertion should proceed as usual, since we're not in Math

  BeginTransaction();
  PRInt32 nInsertPos(-1);
  PRInt32 nMatrixRowLeft(-1), nMatrixRowRight(-1);
  nsCOMPtr<nsIEditor> editor;
  QueryInterface(NS_GET_IID(nsIEditor), getter_AddRefs(editor));

  //Code here should say something like:

  nsCOMPtr<nsISelection> selection;
  PRBool bCollapsed(PR_FALSE);
  nsCOMPtr<nsIDOMNode> startSelNode, endSelNode;
  PRInt32 startSelOffset(0), endSelOffset(0);
  nsresult res = GetNSSelectionData(selection, startSelNode, startSelOffset, endSelNode, endSelOffset, bCollapsed);

  PRBool bIsDisplay(PR_FALSE), bDisplayNeedsMSIDisplay(PR_FALSE);
  PRInt32 doSplitAt(-1), splitOffset(0);
  nsCOMPtr<nsIDOMNode> splittable, splitParent, splitChild;
  nsCOMPtr<nsIDOMNode> nextNode, nextParent, prevChild, matrixContainer, tempNode, displayContainer;
//  nsCOMPtr<nsIDOMElement> asElement;
  nsCOMPtr<nsIDOMNode> leftNode, rightNode;  //To be filled in by the split, if it occurs
  nsAutoString tagName;
  nsresult dontcare;
  //In order to do a "Math return", which inserts a multi-row vector in place of a linear node (or simply adds a row to an existing vector), we have to be able to find two things.
  //  First, there may be a "splittable", that is, a node whose contents will be split into the new cells.
  //  Second, there must be a "split parent", that is, a node (and offset) to hold the resulting matrix.
  //The split container is most likely an <mrow> or perhaps an <mstyle>. If the splitpointnode passed in is a leaf node, we should move
  //  the split point outside of it. (Left if we're at the beginning, right otherwise?) If the splitppointnode (perhaps after this operation)
  //  is a "template" (an <mfrac>, <mroot>, <msubsup>, <munderover>, or even an <mtd>), then no splitting will occur - the child of the
  //  template before position splitpointOffset (unless it's 0, in which case the first child) will contain the new matrix, 
  //  and the current child will be the content of first cell (except for the 0 case, and then the "split" is envisioned as occurring from the left and
  //  the current child will be the contents of the second cell).
  splitChild = nsnull;
  nextNode = splitpointNode;
  PRInt32 currPos(splitpointOffset), prevPos(0);
  PRInt32 toLeftRight(0);
  PRBool bSplitDone = PR_FALSE;
  PRBool bReplaceNode = PR_FALSE;
  PRInt32 specialInsertPos(-1), skipAtEnd(0);
  while (nextNode && !splitParent)
  {
    nextNode->GetLocalName(tagName);
    dontcare = nextNode->GetParentNode(getter_AddRefs(nextParent));
    if ( IsTextNode(nextNode) || tagName.EqualsLiteral("mi") || tagName.EqualsLiteral("mo") || tagName.EqualsLiteral("mn") )
    {
      //In these cases we must move the split point outside
//      splitChild = nextParent;
      if (msiUtils::IsInputbox(this, nextNode))
        toLeftRight = -1;  //Force it to stay in the previous cell!
      else if (!toLeftRight)
      {
        if (currPos > 0)
          toLeftRight = -1;
        else
          toLeftRight = 1;
      }
//        currPos = GetIndexOf(nextNode, nextParent);
//        if (prevPos > 0)
//          ++currPos;
    }
    else if ( tagName.EqualsLiteral("math") )
    {
      if (nextParent)
      {
        dontcare = nextParent->GetLocalName(tagName);
        if (tagName.EqualsLiteral("msidisplay"))
        {
          displayContainer = nextParent;
          bIsDisplay = PR_TRUE;
        }
        else
        {
          nsAutoString displayVal, displayStr;
          displayStr.AssignASCII("display");
          nsCOMPtr<nsIDOMElement> asElement = do_QueryInterface(nextNode);
          dontcare = asElement->GetAttribute(displayStr, displayVal);
          if (displayVal.EqualsLiteral("block"))
          {
            bIsDisplay = PR_TRUE;
            bDisplayNeedsMSIDisplay = PR_TRUE;
          }
        }
      }
      if (bIsDisplay)
      {
        splitParent = nextNode;
        doSplitAt = currPos;
        if (toLeftRight < 0)
          ++doSplitAt;
        nInsertPos = -1;  //Means the created matrix should be inserted as the entire content of the <math> node.
      }
      else
      {
        splittable = nextNode;
        nextParent = nsnull;  //To stop the loop - in this case everything should go back out to the normal InsertReturn?
      }
      toLeftRight = 0;
    }
    else if ( tagName.EqualsLiteral("mover") || tagName.EqualsLiteral("mfrac") || tagName.EqualsLiteral("munderover") 
         || tagName.EqualsLiteral("munder") || tagName.EqualsLiteral("maction") || tagName.EqualsLiteral("menclose") || tagName.EqualsLiteral("mphantom") 
         || tagName.EqualsLiteral("mroot") || tagName.EqualsLiteral("msub") || tagName.EqualsLiteral("msup") || tagName.EqualsLiteral("msubsup") || tagName.EqualsLiteral("mmultiscripts") )
    { 
      //These are the multi-position templates. currPos is where we'll insert the matrix.
      splitParent = nextNode;
      nInsertPos = currPos;
      bReplaceNode = PR_TRUE;
      if (!splittable)  //we haven't found anything to submit to the SplitNodeDeep function, so all the old contents either move to left or right pieces
      {
        if (toLeftRight < 0)
          leftNode = prevChild;
        else if (toLeftRight > 0)
          rightNode = prevChild;
        else if (prevPos == 0)
          rightNode = GetChildAt(splitParent, currPos);
        else
          leftNode = GetChildAt(splitParent, currPos);
        bSplitDone = PR_TRUE;
      }
//      break;
    }
    else if ( tagName.EqualsLiteral("mtd") || tagName.EqualsLiteral("maction") || tagName.EqualsLiteral("menclose") || tagName.EqualsLiteral("mphantom") 
         || tagName.EqualsLiteral("msqrt") )  //The one-child "templates"
    { 
      splitParent = nextNode;
      nInsertPos = -1;  //This signifies that the inserted matrix is to replace the entire contents
      if (!splittable)
      {
        doSplitAt = currPos;
//        if ((toLeftRight < 0) && (currPos > 0))
        if (toLeftRight < 0)
          ++doSplitAt;
      }
      if (tagName.EqualsLiteral("mtd"))  //Here we want to find out whether we're in a one-column matrix - if so we behave differently below
        dontcare = msiUtils::GetMathTagParent(nextNode, msiEditingAtoms::mtable, matrixContainer);
    }
    else if ( tagName.EqualsLiteral("mtr") || tagName.EqualsLiteral("mlabeledtr") )  //Can only arrive here if we started here!
    {
      //here we have to put the split point inside a cell
      if (currPos > 0)
      {
        splitParent = GetChildAt(nextNode, currPos - 1);
        nInsertPos = -1;
        PRUint32 nLength;
        dontcare = GetLengthOfDOMNode(splitParent, nLength);
        doSplitAt = nLength;
      }
      else
      {
        splitParent = GetChildAt(nextNode, 0);
        nInsertPos = -1;
        doSplitAt = 0;
      }
      dontcare = msiUtils::GetMathTagParent(nextNode, msiEditingAtoms::mtable, matrixContainer);
    }
    else if ( tagName.EqualsLiteral("mrow") )  //Here we must test to see whether this is in fact an mfenced, in which case it will be both the splittable and the split parent.
    {
	    nsCOMPtr<msiIMathMLEditingBC> editingBC; 
      GetMathMLEditingBC(nextNode, 0, true, getter_AddRefs(editingBC));
      if (editingBC)
      {
        PRUint32 mmlType(msiIMathMLEditingBC::MATHML_UNKNOWN);
        editingBC->GetMathmlType(&mmlType);
        switch(mmlType)
        {
          case msiIMathMLEditingBC::MATHML_MFENCED:
            splitParent = nextNode;  //In this case all the contents of splitParent are to be divided up into the two cells
            nInsertPos = -1;
            if (!splittable)
            {
              doSplitAt = currPos;
//            if ( (toLeftRight < 0) && (doSplitAt > 0) )
              if (toLeftRight < 0)
                ++doSplitAt;  //Means the split should be to the right of current child node's (that is, "prevChild") position.
            }
//            specialInsertPos = 1;
//            skipAtEnd = 1;
          break;
          case msiIMathMLEditingBC::MATHML_MROWFENCE:
          {
            PRUint32 nLength;
            dontcare = GetLengthOfDOMNode(nextNode, nLength);
            if (currPos == nLength)  //In this case we're actually outside the right delimiter
              toLeftRight = -1;
            else if (currPos == 0)  //In this case we're actually outside the left delimiter
              toLeftRight = 1;
            else
            {
              splitParent = nextNode;  //In this case all the contents of splitParent are to be divided up into the two cells
              nInsertPos = -1;
              if (!splittable)
              {
                doSplitAt = currPos;
//              if ( (toLeftRight < 0) && (doSplitAt > 0) )
                if (toLeftRight < 0)
                  ++doSplitAt;  //Means the split should be to the right of current child node's (that is, "prevChild") position.
              }
              specialInsertPos = 1;
              skipAtEnd = 1;
            }
          }
          break;
          case msiIMathMLEditingBC::MATHML_MROWBOUNDFENCE:
          {
          //In this case, we're looking at a fence which is really part of content, which is presumably atomic, as in a binomial.
          //We need to move the split point inside the contained object (or outside the fence??)
            if (!toLeftRight)
            {
              if (currPos > 0)
                toLeftRight = -1;
              else
                toLeftRight = 1;
            }
          }
          break;
          default:
            splittable = nextNode;
            if (toLeftRight < 0)
              ++currPos;
            toLeftRight = 0;
          break;
        }
      }
    }
    else if ( tagName.EqualsLiteral("mstyle") )  //Here we must test to see whether this is an object simply wrapped in an mstyle, in which case we don't want to split the mstyle unless the object is already split.
    {
      if (splittable)
      {
        splittable = nextNode;
        if (toLeftRight < 0)
          ++currPos;
        toLeftRight = 0;
      }
      else
      {
        nsCOMPtr<nsIArray> styleKids;
        PRUint32 numStyleKids(0);
        dontcare = msiUtils::GetNonWhitespaceChildren(nextNode, styleKids);
        if (NS_SUCCEEDED(dontcare) && styleKids)
          dontcare = styleKids->GetLength(&numStyleKids);
        if (numStyleKids > 1)
        {
          splittable = nextNode;
          if (toLeftRight < 0)
            ++currPos;
          toLeftRight = 0;
        }
        else //We need to determine whether we're at the beginning or not
        {
          nsCOMPtr<nsIDOMNode> prevSibling;
          tempNode = GetChildAt(nextNode, currPos);
          if (tempNode){
             dontcare = tempNode->GetPreviousSibling(getter_AddRefs(tempNode));
             while (NS_SUCCEEDED(dontcare) && prevSibling && msiUtils::IsWhitespace(prevSibling))
             {
               dontcare = prevSibling->GetPreviousSibling(getter_AddRefs(tempNode));
               prevSibling = tempNode;
             }
             if (!prevSibling)
               toLeftRight = 1;
             else
               toLeftRight = -1;
          }
        }
      }
    }
    else
    {
      splittable = nextNode;
      toLeftRight = 0;
    }
//    }
    prevPos = currPos;
    if (splittable && !splitChild)
    {
      splitChild = splittable;
      splitOffset = currPos;
    }
    if (nextParent)
      currPos = GetIndexOf(nextParent, nextNode);
    prevChild = nextNode;
    nextNode = nextParent;
  }

  if (splittable && !splitChild)
  {
    splitChild = splitpointNode;
    splitOffset = splitpointOffset;
  }

  PRBool bInsertNewMatrix = PR_FALSE;
  nsCOMPtr<nsIDOMNode> leftInsert, rightInsert;
  nsCOMPtr<nsIDOMNode> existingNode;
  if (splitParent)
  {
    nsCOMPtr<nsIDOMNode> caretNode;
    PRUint32 caretPos(0);
    if (!bSplitDone)
    {
      if (splittable)
      {
        PRInt32 postSplitPos;
        dontcare = splittable->GetParentNode(getter_AddRefs(tempNode));
        if (tempNode == splitParent)
        {
          res = SplitNodeDeep(splittable, splitChild, splitOffset, &postSplitPos, PR_TRUE, address_of(leftNode), address_of(rightNode));
          if (nInsertPos == -1)
            doSplitAt = postSplitPos;
        }
        bSplitDone = PR_TRUE;
      }
    }

    if (matrixContainer)  //we're already in a matrix - is it a one-column one?
    {
      PRInt32 nRows, nCols;
      dontcare = m_msiEditingMan->GetMatrixSize(matrixContainer, &nRows, &nCols);
      if (nCols == 1)
      {
        nsIDOMNode* aCell = nsnull;
        if (splittable)
          aCell = splittable;
        else if (splitChild)
          aCell = splitChild;
        else
          aCell = splitpointNode;
        aCell->GetLocalName(tagName);
        while (aCell && !tagName.EqualsLiteral("mtd"))
        {
          dontcare = aCell->GetParentNode(getter_AddRefs(nextParent));
          aCell = nextParent;
          aCell->GetLocalName(tagName);
        }
        dontcare = m_msiEditingMan->FindMatrixCell(matrixContainer, aCell, &nMatrixRowLeft, &nCols);  //Just reusing nCols - it'll be thrown away
        nMatrixRowRight = nMatrixRowLeft + 1;
        res = m_msiEditingMan->AddMatrixRows(editor, matrixContainer, nMatrixRowRight-1, 1);
      }
      else
        matrixContainer = nsnull;
    }
    nsCOMPtr<nsIDOMElement> newMatrix;
    if (!matrixContainer)
    {
      nsAutoString rowSignature;
      PRUint32 flags(0);
      nMatrixRowLeft = 1;
      nMatrixRowRight = 2;
      res = msiUtils::CreateMtable(editor, 2, 1, rowSignature, PR_TRUE, flags, newMatrix, NS_LITERAL_STRING(""));

      if (NS_SUCCEEDED(res) && newMatrix)
      {
        matrixContainer = do_QueryInterface(newMatrix);
        bInsertNewMatrix = PR_TRUE;
      }
    }

    nsCOMPtr<nsIDOMNode> replaceInputBox;
    nsCOMPtr<nsIDOMNode> newLeftContainer, newRightContainer;
    //Now we want to make calls to reparent leftNode and rightNode to the appropriate cells of matrixContainer:
    if (NS_SUCCEEDED(res) && matrixContainer)
    {
      //First find appropriate <td>s using nMatrixRowLeft and Right
      nsCOMPtr<nsIDOMNode> aLeftCell, aRightCell;
      nsCOMPtr<nsIDOMNode> leftRow, rightRow;
      dontcare = m_msiEditingMan->GetMatrixCellAt(matrixContainer, nMatrixRowLeft, 1, getter_AddRefs(aLeftCell));
      newLeftContainer = do_QueryInterface(aLeftCell);
      dontcare = m_msiEditingMan->GetMatrixCellAt(matrixContainer, nMatrixRowRight, 1, getter_AddRefs(aRightCell));
      newRightContainer = do_QueryInterface(aRightCell);
      PRUint32 flags(0);

      if (bSplitDone)
      {
        if (leftNode && (newLeftContainer != splitParent))
        {
          PRInt32 insertAt(-1);
          editor->BeginTransaction();
          editor->SaveSelection(selection);
          //insert leftNode in newLeftContainer
          res = DeleteNode(leftNode);
          if (NS_SUCCEEDED(res))
          {
            replaceInputBox = GetChildAt(newLeftContainer, 0);
            if (!msiUtils::IsInputbox(this, replaceInputBox))
              replaceInputBox = nsnull;
            if (replaceInputBox)
            {
              nsCOMPtr<nsINode> baseNode = do_QueryInterface(newLeftContainer);
              PRBool bWasEditable = baseNode->IsEditable();
              if (!bWasEditable)
                baseNode->SetEditableFlag(PR_TRUE);
              res = SimpleDeleteNode(replaceInputBox);
              if (!bWasEditable)
                baseNode->SetEditableFlag(PR_FALSE);
            }
          }
          if (NS_SUCCEEDED(res))
            res = InsertNode(leftNode, newLeftContainer, insertAt);
  //        res = MoveNode(leftNode, newLeftContainer, 0); //at position 0??? Maybe should be at the end
  //        //else put an input box there - but shouldn't that already be there? And does MoveNode get rid of it??
  //      else if (leftInsert)
  //      {
  //        nsCOMPtr<msiIMathMLInsertion> mathmlEditing;
  //        GetMathMLInsertionInterface(newLeftContainer, 0, getter_AddRefs(mathmlEditing));
  //        if (mathmlEditing)
  //        {
  //          res = mathmlEditing->InsertNode(editor, selection, leftNode, flags);
  //        }
          editor->EndTransaction();
        }
        flags = 0;
        if (rightNode && (newRightContainer != splitParent))
        {
          editor->BeginTransaction();
          editor->SaveSelection(selection);
          res = DeleteNode(rightNode);
          if (NS_SUCCEEDED(res))
          {
            replaceInputBox = GetChildAt(newRightContainer, 0);
            if (!msiUtils::IsInputbox(this, replaceInputBox))
              replaceInputBox = nsnull;
            if (replaceInputBox)
            {
              nsCOMPtr<nsINode> baseNode = do_QueryInterface(newRightContainer);
              PRBool bWasEditable = baseNode->IsEditable();
              if (!bWasEditable)
                baseNode->SetEditableFlag(PR_TRUE);
              res = SimpleDeleteNode(replaceInputBox);
              if (!bWasEditable)
                baseNode->SetEditableFlag(PR_FALSE);
            }
          }
          if (NS_SUCCEEDED(res))
            res = InsertNode(rightNode, newRightContainer, 0);
          //insert rightNode in newRightContainer
  //        res = MoveNode(rightNode, newRightContainer, 0); //at position 0??? Probably
  //        //else put an input box there - but shouldn't that already be there? And does MoveNode get rid of it??
  //      else if (rightInsert)
  //      {
  //        nsCOMPtr<msiIMathMLInsertion> mathmlEditing;
  //        GetMathMLInsertionInterface(newRightContainer, 0, getter_AddRefs(mathmlEditing));
  //        if (mathmlEditing)
  //        {
  //          res = mathmlEditing->InsertNode(editor, selection, rightNode, flags);
  //        }
          editor->EndTransaction();
        }
      }
      else if (splitParent)        //in this case we'll insert the matrix at, but the previous contents of splitParent should be divvied up
      {
        PRUint32 parentLength;
        dontcare = GetLengthOfDOMNode(splitParent, parentLength);
        if (doSplitAt < 0)
          doSplitAt = parentLength;
        editor->BeginTransaction();
        editor->SaveSelection(selection);
        if (parentLength > doSplitAt)
        {
          replaceInputBox = GetChildAt(newRightContainer, 0);
          if (!msiUtils::IsInputbox(this, replaceInputBox))
            replaceInputBox = nsnull;
          if (replaceInputBox)
          {
            nsCOMPtr<nsINode> baseNode = do_QueryInterface(newRightContainer);
            PRBool bWasEditable = baseNode->IsEditable();
            if (!bWasEditable)
              baseNode->SetEditableFlag(PR_TRUE);
            res = SimpleDeleteNode(replaceInputBox);
            if (!bWasEditable)
              baseNode->SetEditableFlag(PR_FALSE);
          }
        }
        if (doSplitAt > 0)
        {
          replaceInputBox = GetChildAt(newLeftContainer, 0);
          if (!msiUtils::IsInputbox(this, replaceInputBox))
            replaceInputBox = nsnull;
          if (replaceInputBox)
          {
            nsCOMPtr<nsINode> baseNode = do_QueryInterface(newLeftContainer);
            PRBool bWasEditable = baseNode->IsEditable();
            if (!bWasEditable)
              baseNode->SetEditableFlag(PR_TRUE);
            res = SimpleDeleteNode(replaceInputBox);
            if (!bWasEditable)
              baseNode->SetEditableFlag(PR_FALSE);
          }
        }
        if (newRightContainer != splitParent)
        {
          for (PRInt32 jx = PRInt32(parentLength)-1-skipAtEnd; NS_SUCCEEDED(res) && (jx >= doSplitAt); --jx)
          {
            rightNode = GetChildAt(splitParent, jx);
            res = DeleteNode(rightNode);
            if (NS_SUCCEEDED(res))
              res = InsertNode(rightNode, newRightContainer, 0);
    //        res = MoveNode(rightNode, newRightContainer, 0);
          }
        }
        if (newLeftContainer != splitParent)
        {
          for (PRInt32 jx = doSplitAt - 1; NS_SUCCEEDED(res) && (jx >= skipAtEnd); --jx)
          {
            leftNode = GetChildAt(splitParent, jx);
            res = DeleteNode(leftNode);
            if (NS_SUCCEEDED(res))
              res = InsertNode(leftNode, newLeftContainer, 0);
    //        res = MoveNode(leftNode, newLeftContainer, 0);
          }
        }
        editor->EndTransaction();
      }

      nsCOMPtr<nsIArray> realLeftChildren, realRightChildren;
      PRUint32 numRealChildren(0);
      nsCOMPtr<nsIDOMElement> inputBoxElt;
      flags = 0;
      if (newLeftContainer)
      {
        dontcare = msiUtils::GetNonWhitespaceChildren(newLeftContainer, realLeftChildren);
        if (NS_SUCCEEDED(dontcare))
          realLeftChildren->GetLength(&numRealChildren);
        if (!numRealChildren)
        {
          res = msiUtils::CreateInputbox(editor, PR_FALSE, PR_FALSE, flags, inputBoxElt);
          if (NS_SUCCEEDED(res))
          {
            replaceInputBox = do_QueryInterface(inputBoxElt);
            res = InsertNode(replaceInputBox, newLeftContainer, 0);
          }
        }
        inputBoxElt = nsnull;
        flags = 0;
      }
      if (newRightContainer)
      {
        dontcare = msiUtils::GetNonWhitespaceChildren(newRightContainer, realRightChildren);
        if (NS_SUCCEEDED(dontcare))
          realRightChildren->GetLength(&numRealChildren);
        if (!numRealChildren)
        {
          res = msiUtils::CreateInputbox(editor, PR_FALSE, PR_FALSE, flags, inputBoxElt);
          if (NS_SUCCEEDED(res))
          {
            replaceInputBox = do_QueryInterface(inputBoxElt);
            res = InsertNode(replaceInputBox, newRightContainer, 0);
            if (NS_SUCCEEDED(res))
            {
              caretNode = replaceInputBox;
              caretPos = 1;
            }
          }
        }
      }

      *bHandled = !bInsertNewMatrix;
    }

    //Finally if the matrix is a new one, insert it:
    if (bInsertNewMatrix)
    {
      PRUint32 nLength;
      dontcare = GetLengthOfDOMNode(splitParent, nLength);
      PRInt32 realInsertPos = nInsertPos;
      if (realInsertPos < 0)
        realInsertPos = nLength;
      if (specialInsertPos > 0)
        realInsertPos = specialInsertPos;
      res = InsertNode(matrixContainer, splitParent, realInsertPos);
//  //      if (nInsertPos < 0 || !bReplaceNode)
//  //      {
//      nsCOMPtr<msiIMathMLInsertion> mathmlEditing;
//      PRUint32 flags(0);
//      GetMathMLInsertionInterface(splitParent, realInsertPos, getter_AddRefs(mathmlEditing));
//      if (mathmlEditing)
//      {
//  //          editor->BeginTransaction();
//  //          editor->SaveSelection(selection);
//        res = mathmlEditing->InsertNode(editor, selection, matrixContainer, flags);
      if (NS_SUCCEEDED(res))
        *bHandled = PR_TRUE;
    }
    if (bIsDisplay)
    {
      nsAutoString eqnArrayStr, typeStr;
      typeStr.AssignASCII("type");
      eqnArrayStr.AssignASCII("eqnarray");
      nsCOMPtr<nsIDOMElement> asElement = do_QueryInterface(matrixContainer);
      SetAttribute(asElement, typeStr, eqnArrayStr);
      if (bDisplayNeedsMSIDisplay)
      {
//        nsCOMPtr<nsIDOMNode> msiDisplayNode;
        nsAutoString msidisplayStr;
        msidisplayStr.AssignASCII("msidisplay");
        dontcare = InsertContainerAbove( topMathNode, address_of(displayContainer), msidisplayStr, nsnull, nsnull);
        NS_ASSERTION(NS_SUCCEEDED(dontcare), "Failed to insert msidisplay node in msiEditor::InsertReturnInMath!");  //Check that this is the newly inserted node
      }
      if (displayContainer)
      {
        nsAutoString eqnsStr, numberStr;
        numberStr.AssignASCII("numbering");
        eqnsStr.AssignASCII("eqns");
        asElement = do_QueryInterface(displayContainer);
        SetAttribute(asElement, numberStr, eqnsStr);  //numbering="eqns" signifies that the equation array numbering is on the table cells
      }
    }

    //May as well set the caret now
    if (NS_SUCCEEDED(res) && newRightContainer)
    {
      dontcare = GetSelection(getter_AddRefs(selection));
      if (NS_SUCCEEDED(dontcare))
      {
        nsCOMPtr<nsIDOMRange> range, newRange;
        selection->GetRangeAt(0, getter_AddRefs(range));
        if (range)
        {
          range->CloneRange(getter_AddRefs(newRange));
          if (!caretNode)
          {
            replaceInputBox = GetChildAt(newRightContainer, 0);
            if (msiUtils::IsInputbox(this, replaceInputBox))
            {
              caretNode = replaceInputBox;
              caretPos = 1;
            }
            else
            {
              caretNode = newRightContainer;
              caretPos = 0;
            }
          }
          newRange->SetStart(caretNode, caretPos);
//          newRange->SetStartOffset(caretPos);
          newRange->Collapse(PR_TRUE);
          selection->RemoveAllRanges();  
          selection->AddRange(newRange);
        }
      }
    }
  }

  EndTransaction();
  return res;
}
  

// Implementation of an nsIContentFilter


NS_IMPL_ISUPPORTS1(msiContentFilter, nsIContentFilter)

msiContentFilter::msiContentFilter(nsIEditor * editor)
{
  m_editor = editor;
  nsCOMArray<nsITimer> m_timerlist;
}


PRBool IsRelativePath(const nsString& path)
{
  nsAString::const_iterator start, end;

  path.BeginReading(start);
  path.EndReading(end);
  if (*start == '/') return PR_FALSE;
  NS_NAMED_LITERAL_STRING(valuePrefix, "://");

  if (FindInReadable(valuePrefix, start, end)) {
    return PR_FALSE;
  }
  return PR_TRUE;
}


NS_IMETHODIMP msiContentFilter::copyfiles( 
  nsIDocument * srcDoc,
  nsIDocument * doc,
  nsIDOMNodeList * objnodes, 
  nsIDOMNode * anode,
  PRUint32 count)
{
  nsresult res(NS_OK);
  nsAutoString dataPath;
  nsAutoString absPath;
  nsAutoString leafname;
  nsAutoString srcDirName;
  nsAutoString attr;
  nsCOMPtr<nsIURL> destURL;
  nsCOMPtr<nsIURL> srcURL;
  nsCOMPtr<nsIURI> destURI;
  nsCOMPtr<nsIURI> srcURI;
  nsCOMPtr<nsIFile> srcIFile;
  nsCOMPtr<nsILocalFile> srcFile;
  nsCOMPtr<nsIFile> srcDir;
  nsCOMPtr<nsIFile> srcFileClone;
  nsCOMPtr<nsIFile> destIFile;
  nsCOMPtr<nsILocalFile> destFile;
  nsCOMPtr<nsIDOMNode> node;
  nsCOMPtr<nsIDOMElement> elem;
  PRBool fExists;
  PRBool bNeedUnique = PR_FALSE;
  PRBool bNotifyGraphSpec = PR_FALSE;
  nsAutoString msiGraphAttr;
  // get the data attribute from each <object>
  if (objnodes) {
    res = objnodes->Item(count, getter_AddRefs(node));
  }
  else {
    node = anode;
  }
  elem = do_QueryInterface(node);
  attr = NS_LITERAL_STRING("msigraph");
  elem->GetAttribute(attr, msiGraphAttr);
  if (msiGraphAttr.EqualsLiteral("true")) {
    bNeedUnique = PR_TRUE;
    bNotifyGraphSpec = PR_TRUE;
  } else {
    msiGraphAttr.Truncate(0);
    attr = NS_LITERAL_STRING("msisnap");
    elem->GetAttribute(attr, msiGraphAttr);
    if (msiGraphAttr.EqualsLiteral("true"))
      bNeedUnique = PR_TRUE;
  }
  if (!bNeedUnique && (srcDoc == doc)) {
    // nothing to do, except make sure recently copied files show up
    // if copying a graphic from another document, we have written a file to 
    // the document directory tree, but it may not be ready to show up yet.
    nsCOMPtr<nsITimer> timer = do_CreateInstance("@mozilla.org/timer;1");
    if (!timer) {
      return NS_OK;
    }
//    str * mystruct = new str();
//    mystruct->elem = elem;
//    mystruct->ed = static_cast<nsEditor*>(m_editor);
    nsIDOMElement* elemPtr = elem;
    NS_ADDREF(elemPtr);  //Have to do this to ensure that the pointer passed to the timer callback sticks around
    timer->InitWithFuncCallback(msiContentFilter::SetDataFromTimer, static_cast<void*>(elemPtr), 200,
                                  nsITimer::TYPE_ONE_SHOT);
    m_timerlist.AppendObject(timer); 
    return NS_OK;
  }
  
  NS_ENSURE_SUCCESS(res, res);
  attr = NS_LITERAL_STRING("data");
  elem->GetAttribute(attr, dataPath);
  if (dataPath.Length() == 0) {
    attr = NS_LITERAL_STRING("src");
    elem->GetAttribute(attr, dataPath);
  }
  if (dataPath.Length() > 0) {
      if (IsRelativePath(dataPath))
      {
        if (!srcDoc) return NS_ERROR_FAILURE;
        srcURI = srcDoc->GetDocumentURI();
        srcURL = do_QueryInterface(srcURI);
        nsCAutoString dirPath;
        nsCAutoString srcSpec;
        res = NS_MakeAbsoluteURI(srcSpec, NS_ConvertUTF16toUTF8(dataPath), srcURI);

//        res = srcURL->GetDirectory(dirPath);
        res = NS_GetFileFromURLSpec(srcSpec, getter_AddRefs(srcIFile));
//        absPath.Truncate(0);
//        AppendASCIItoUTF16(dirPath, absPath);
//        absPath.Append(dataPath);
//        dataPath = absPath;
//        // now dataPath is an absolute path
      }
      else
        res = NS_GetFileFromURLSpec(NS_ConvertUTF16toUTF8(dataPath), getter_AddRefs(srcIFile));
      srcFile = do_QueryInterface(srcIFile);

      destURI = doc->GetDocumentURI();
      destURL = do_QueryInterface(destURI);
      nsCAutoString dirPath;
//      res = destURL->GetDirectory(dirPath);
      res = destURL->GetSpec(dirPath);  //We'll take the parent of the nsILocalFile we create from this
      char * unescaped = strdup(dirPath.get());
      nsUnescape(unescaped);
      dirPath.Assign(unescaped, PR_UINT32_MAX); 
//      res = NS_NewLocalFile(NS_ConvertUTF8toUTF16(dirPath), PR_FALSE, getter_AddRefs(destFile));
      res = NS_GetFileFromURLSpec(dirPath, getter_AddRefs(destIFile));
      nsCOMPtr<nsIFile> dirAsFile;
      res = destIFile->GetParent(getter_AddRefs(dirAsFile));
      destFile = do_QueryInterface(dirAsFile);
//      res = NS_NewLocalFile(dataPath, PR_FALSE, getter_AddRefs(srcFile));
      // Now take apart the source path to get a name for the dest file.
      res = srcFile->Clone(getter_AddRefs(srcFileClone));
      res = srcFileClone->GetLeafName(leafname);
      res = srcFileClone->GetParent(getter_AddRefs(srcDir));
      res = srcDir->GetLeafName(srcDirName);
      res = destFile->Append(srcDirName);
      res = destFile->Exists(&fExists);
      if (!fExists)
        destFile->Create(1, 0755);
      res = destFile->Append(leafname);
      PRBool isSameFile;
      res = destFile->Equals(srcFile, &isSameFile);
      if (isSameFile && !bNeedUnique) {
        nsCOMPtr<nsITimer> timer = do_CreateInstance("@mozilla.org/timer;1");
        if (!timer) {
          return NS_OK;
        }
        nsIDOMElement* elemPtr = elem;
        NS_ADDREF(elemPtr);  //Have to do this to ensure that the pointer passed to the timer callback sticks around
        timer->InitWithFuncCallback(msiContentFilter::SetDataFromTimer, static_cast<void*>(elemPtr), 200,
                                  nsITimer::TYPE_ONE_SHOT);
        m_timerlist.AppendObject(timer); 
        return NS_OK;
      }

      destFile->CreateUnique(nsIFile::NORMAL_FILE_TYPE, 0755);
      nsAutoString newDataAttribute(srcDirName);
      newDataAttribute.Append(NS_LITERAL_STRING("/"));
      res = destFile->GetLeafName(leafname);
      newDataAttribute.Append(leafname);
      elem->SetAttribute(attr, newDataAttribute);
      if (bNotifyGraphSpec) {
        nsCOMPtr<nsIDOMNode> gsNode;
        nsCOMPtr<nsIDOMNode> parent;
        nsCOMPtr<nsIDOMElement> graphSpec;
        nsCOMPtr<nsIDOMNodeList> gsNodes;
        nsAutoString tagName;
        PRUint32 gsCount;
        nsCOMPtr<nsIDOMNode> kidNode = node;
        do {
          res = kidNode->GetParentNode(getter_AddRefs(parent));
          if (NS_SUCCEEDED(res) && parent)
            parent->GetNodeName(tagName);
          kidNode = parent;
        } while (kidNode && !tagName.EqualsLiteral("graph"));
        if (NS_SUCCEEDED(res) && parent) {
          nsCOMPtr<nsIDOMElement> graphNode = do_QueryInterface(kidNode);
          res = graphNode->GetElementsByTagName(NS_LITERAL_STRING("graphSpec"), getter_AddRefs(gsNodes));
          gsNodes->GetLength(&gsCount);
          if (NS_SUCCEEDED(res) && (gsCount > 0)) {
            gsNodes->Item(0, getter_AddRefs(gsNode));
            graphSpec = do_QueryInterface(gsNode);
            attr = NS_LITERAL_STRING("ImageFile");
            graphSpec->SetAttribute(attr, newDataAttribute);
          }
        }
      }
      // now we are ready to copy from srcFile to destFile
      nsCOMPtr<msiIUtil> utils = do_GetService("@mackichan.com/msiutil;1", &res);
      PRBool success;
      res = utils->SynchronousFileCopy(srcFile, destFile, &success);
      // set data attribute using a timer.
      nsCOMPtr<nsITimer> timer = do_CreateInstance("@mozilla.org/timer;1");
      if (!timer) {
        return NS_OK;
      }
//      str * mystruct = new str();
//      mystruct->elem = elem;
//      mystruct->ed = static_cast<nsEditor*>(m_editor);
      nsIDOMElement* elemPtr = elem;
      NS_ADDREF(elemPtr);  //Have to do this to ensure that the pointer passed to the timer callback sticks around
      timer->InitWithFuncCallback(msiContentFilter::SetDataFromTimer, (void*)elemPtr, 200,
                                    nsITimer::TYPE_ONE_SHOT);
      m_timerlist.AppendObject(timer); 
      
  }
  return NS_OK;
}

void msiContentFilter::SetDataFromTimer(nsITimer *aTimer, void *closure)
{
  if (closure) {
//    str * mystruct = static_cast<str*>(closure);
    PRBool fExists;
//    nsCOMPtr<nsIDOMElement> elem(mystruct->elem);
    nsCOMPtr<nsIDOMElement> elem = static_cast<nsIDOMElement*>(closure);
//    nsEditor * editor = mystruct->ed;
    nsCOMPtr<nsIDOMNode> parent;
//    PRInt32 offset;
    
//    nsresult res = editor->GetNodeLocation(elem, address_of(parent), &offset);
//    editor->DeleteNode(elem);
//    editor->InsertNode(elem, parent, offset);
    

    nsAutoString dataPath;
    elem->HasAttribute(NS_LITERAL_STRING("data"), &fExists);
    if (fExists) {
      elem->GetAttribute(NS_LITERAL_STRING("data"), dataPath);
      elem->RemoveAttribute(NS_LITERAL_STRING("data"));
      elem->SetAttribute(NS_LITERAL_STRING("data"), dataPath);
    }
    elem->HasAttribute(NS_LITERAL_STRING("src"), &fExists);
    if (fExists) {
      elem->GetAttribute(NS_LITERAL_STRING("src"), dataPath);
      elem->RemoveAttribute(NS_LITERAL_STRING("src"));
      elem->SetAttribute(NS_LITERAL_STRING("src"), dataPath);
    }
    nsIDOMElement* elemPtr = elem;
    NS_RELEASE(elemPtr);  //Since we had to addref it previously to ensure its persistence
  }
}

msiContentFilter::~msiContentFilter()
{
  ClearTimerList();
}

void msiContentFilter::ClearTimerList()
{
  if (m_timerlist.Count() == 0)
      return;

  PRUint32 n = m_timerlist.Count();
  PRUint32 i;
  for (i=0; i<n; ++i) {
    nsCOMPtr<nsITimer> timer = m_timerlist[i];
    if (timer)
        timer->Cancel();        
  }
  m_timerlist.Clear();
}

NS_IMETHODIMP msiContentFilter::NotifyOfInsertion(
  const nsAString & mimeType, 
  nsIURL *contentSourceURL, 
  nsIDOMDocument *sourceDocument, 
  PRBool willDeleteSelection, 
  nsIDOMNode **docFragment, 
  nsIDOMNode **contentStartNode, 
  PRInt32 *contentStartOffset, 
  nsIDOMNode **contentEndNode,
  PRInt32 *contentEndOffset, 
  nsIDOMNode **insertionPointNode,
  PRInt32 *insertionPointOffset, 
  PRBool *continueWithInsertion)
{
  nsresult res(NS_OK);
  nsCOMPtr<nsIDOMDocument> domDoc;
  ClearTimerList();
  m_editor->GetDocument(getter_AddRefs(domDoc));
  nsCOMPtr<nsIDocument> srcDoc = do_QueryInterface(sourceDocument);
  nsCOMPtr<nsIDocument> doc = do_QueryInterface(domDoc);
  if (!doc) return (NS_ERROR_FAILURE);
//  if (domDoc == sourceDocument) return (NS_OK);  // pasting from same document doesn't require work
  // We look for img and object tags that have relative paths for data or src attributes.
  nsCOMPtr<nsIDOMNode> rootnode = *docFragment;
  nsCOMPtr<nsIDOMNodeList> children;
  nsCOMPtr<nsIDOMNode> node;
  nsCOMPtr<nsIDOMElement> elem;
  PRUint32 childCount;
  nsCOMPtr<nsIDOMNodeList> objnodes;
  nsAutoString nodename;
  PRUint32 count;
  PRUint32 i;
  PRUint32 j;
  res = rootnode->GetChildNodes(getter_AddRefs(children));
  NS_ENSURE_SUCCESS(res, res);
  res = children->GetLength(&childCount);
  NS_ENSURE_SUCCESS(res, res);
  for ( j=0; j < childCount; j++ )
  {
    res = children->Item(j, getter_AddRefs(node));
    NS_ENSURE_SUCCESS(res, res);
    elem = do_QueryInterface(node);
    if (elem) {
      res = elem->GetNodeName(nodename);
      res = elem->GetElementsByTagName(NS_LITERAL_STRING("object"), getter_AddRefs(objnodes));
      NS_ENSURE_SUCCESS(res, res);
      if (nodename.EqualsLiteral("object"))
      {
        copyfiles(srcDoc, doc, nsnull, elem, i);
      }
      else
      {  
        objnodes->GetLength(&count);
        for (i = 0; i < count; i++) {
          copyfiles(srcDoc, doc, objnodes, nsnull, i);
        }
      }
      res = elem->GetElementsByTagName(NS_LITERAL_STRING("img"), getter_AddRefs(objnodes));
      NS_ENSURE_SUCCESS(res, res);
      if (nodename.EqualsLiteral("img"))
      {
        copyfiles(srcDoc, doc, nsnull, elem, i);
      }
      else
      {  
        objnodes->GetLength(&count);
        for (i = 0; i < count; i++) {
          copyfiles(srcDoc, doc, objnodes, nsnull, i);
        }
      }
    }
  }
  return NS_OK;
}




















