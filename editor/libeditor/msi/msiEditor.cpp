// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.


#include "nsCOMPtr.h"
#include "msiEditor.h"
#include "msiIMathMLInsertion.h"
#include "msiIMathMLCaret.h"
#include "msiIMathMLCoalesce.h"
#include "nsEditorEventListeners.h"
#include "msiEditorMouseListener.h"
#include "msiEditorMouseMotionListener.h"
#include "msiLayoutAtoms.h"

#include "nsContentUtils.h"
#include "nsIDOMDocumentFragment.h"
#include "nsIDocument.h"
#include "nsIDomText.h"
#include "nsIDOMAttr.h"
#include "nsIDOMNamedNodeMap.h"
#include "nsIDOMNodeList.h"
#include "nsITextContent.h"
#include "nsIPresShell.h"
#include "nsIDOMNSUIEvent.h"
#include "nsIDOM3EventTarget.h"
#include "nsIDOMEventReceiver.h"
#include "nsIDOMEventGroup.h"
#include "nsIServiceManager.h"

#include "msiISelection.h"
#include "msiIEditingManager.h"

static PRInt32 instanceCounter = 0;
nsIRangeUtils * msiEditor::m_rangeUtils = nsnull;

msiEditor::msiEditor()
{
  nsresult dummy(NS_OK);
  m_msiEditingMan = do_CreateInstance(MSI_EDITING_MANAGER_CONTRACTID, &dummy);
  if (!m_rangeUtils)
    CallGetService("@mozilla.org/content/range-utils;1",  &m_rangeUtils);
  msiLayoutAtoms::AddRefAtoms(); // initalize a copy of the layout and mathml atoms
  instanceCounter += 1;
}

msiEditor::~msiEditor()
{
  instanceCounter -= 1;
  m_msiEditingMan = nsnull;
  if (instanceCounter <= 0)
    NS_IF_RELEASE(m_rangeUtils);
}

NS_IMPL_ISUPPORTS_INHERITED1(msiEditor, nsHTMLEditor, msiIMathMLEditor)


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

  nsCOMPtr<nsIDOMEventReceiver> erP = GetDOMEventReceiver();

  if (!erP) 
  {
    RemoveEventListeners();
    return NS_ERROR_FAILURE;
  }
  nsresult rv = erP->AddEventListenerByIID(m_mouseMotionListener, NS_GET_IID(nsIDOMMouseMotionListener));
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

  nsCOMPtr<nsIDOMEventReceiver> erP = GetDOMEventReceiver();
  if (erP)
  {
    // unregister the event listeners with the DOM event reveiver

    if (m_mouseMotionListener)
    {
      erP->RemoveEventListenerByIID(m_mouseMotionListener,
                                    NS_GET_IID(nsIDOMMouseMotionListener));
      m_mouseMotionListener = nsnull;
    }
  }
  nsHTMLEditor::RemoveEventListeners();
  return;
}

//Begin msiIMathMLEditor

NS_IMETHODIMP 
msiEditor::GetMathMLEditingBC(nsIDOMNode * node, PRUint32 offset,
                              msiIMathMLEditingBC** editingBC)
{
  nsresult res(NS_ERROR_FAILURE);
  if (m_msiEditingMan)
    res = m_msiEditingMan->GetMathMLEditingBC(node, offset, editingBC);
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
        res = NS_ERROR_FAILURE;
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
      }
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
      if (!bCollapsed)
      {
        res = NS_ERROR_FAILURE;
        // TODO add stuff so that selected stuff is changed to become the numerator?
        // current SWP behavoir is to do so.
        // Also need to deal with the case where we are not in math, or part of the selection is not
        // in math.
      }
      else
      {
        theNode = startNode;
        theOffset = startOffset;
      }
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
      if (!bCollapsed)
      {
        res = NS_ERROR_FAILURE;
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
      }
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
      if (!bCollapsed)
      {
        res = NS_ERROR_FAILURE;
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
      }
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
msiEditor::InsertSymbol(PRUint32 symbol)
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
        res = NS_ERROR_FAILURE;
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
      }
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
        res = NS_ERROR_FAILURE;
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
      }
      if (NS_SUCCEEDED(res))
        res = InsertMathnameEx(selection, theNode, theOffset, mathname);
    }
  }
  else if (mathname.Length() == 0)
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
        res = NS_ERROR_FAILURE;
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
      }
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
        res = NS_ERROR_FAILURE;
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
      }
      if (NS_SUCCEEDED(res))
      {
        nsCOMPtr<nsIEditor> editor;
        QueryInterface(NS_GET_IID(nsIEditor), getter_AddRefs(editor));
        res = m_msiEditingMan->InsertFence(editor, selection, theNode, 
                                           theOffset, open, close);
      }  
    }
  }
  return res;
}

NS_IMETHODIMP
msiEditor::InsertMatrix(PRUint32 rows, PRUint32 cols, const nsAString & rowSignature)
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
        res = NS_ERROR_FAILURE;
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
      }
      if (NS_SUCCEEDED(res))
      {
        nsCOMPtr<nsIEditor> editor;
        QueryInterface(NS_GET_IID(nsIEditor), getter_AddRefs(editor));
        res = m_msiEditingMan->InsertMatrix(editor, selection, theNode, 
                                           theOffset, rows, cols, rowSignature);
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
        res = NS_ERROR_FAILURE;
        // TODO add stuff to delete and replace the selection?
        // current SWP behavoir is to replace selection by the operator, but since we may want to allow
        // arbitrary math to be the content of an <mo> at some time, this should be considered.
        // For now, should simply replace the selection.
        // Also need to deal with the case where we are not in math, or part of the selection is not
        // in math.
      }
      else
      {
        theNode = startNode;
        theOffset = startOffset;
      }
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
msiEditor::InsertDecoration(const nsAString & above, const nsAString & below)
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
        res = NS_ERROR_FAILURE;
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
      }
      if (NS_SUCCEEDED(res))
      {
        nsCOMPtr<nsIEditor> editor;
        QueryInterface(NS_GET_IID(nsIEditor), getter_AddRefs(editor));
        res = m_msiEditingMan->InsertDecoration(editor, selection, theNode, 
                                                theOffset, above, below);
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
  return CreateTxnForReplaceElement(newKid, oldKid, parent, (ReplaceElementTxn**)transaction);  
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

//End nsIMathMLEditor


NS_IMETHODIMP 
msiEditor::HandleKeyPress(nsIDOMKeyEvent * aKeyEvent)
{
  if (! aKeyEvent)  
    return NS_ERROR_NULL_POINTER;
  nsresult res(NS_OK);
  if (!(mFlags & eEditorPlaintextMask)) // copied from nsHTMLEditor -- I don't know if this is an issue
  {
  
    PRUint32 keyCode(0), symbol(0);
    PRBool isShift(PR_FALSE), ctrlKey(PR_FALSE), altKey(PR_FALSE), metaKey(PR_FALSE);
    res = ExtractDataFromKeyEvent(aKeyEvent, keyCode, symbol, isShift, ctrlKey,
                                  altKey, metaKey);
    if (keyCode == nsIDOMKeyEvent::DOM_VK_LEFT  ||  keyCode == nsIDOMKeyEvent::DOM_VK_RIGHT ||
        keyCode == nsIDOMKeyEvent::DOM_VK_UP    ||  keyCode == nsIDOMKeyEvent::DOM_VK_DOWN)
    {
      PRBool preventDefault(PR_FALSE);
      res = HandleArrowKeyPress(keyCode, isShift, ctrlKey, altKey, metaKey, preventDefault); 
      if (NS_SUCCEEDED(res) && preventDefault)
        aKeyEvent->PreventDefault();
    }
    else if (symbol)
    {
      PRBool collapsed(PR_FALSE);
      nsCOMPtr<msiISelection> msiSelection;
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
        res = msiSelection->GetMsiFocusNode(getter_AddRefs(currFocusNode));
        if (NS_SUCCEEDED(res) && currFocusNode && NodeInMath(currFocusNode)&& !ctrlKey && !altKey)
        {
          res = InsertSymbol(symbol);
          if (NS_SUCCEEDED(res))
            aKeyEvent->PreventDefault();
        }    
      }    
    }    
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
      return nsHTMLEditor::HandleKeyPress(aKeyEvent);
  }
  else
    return NS_ERROR_FAILURE;
}


//SLS It's not clear who calls this function.  It is accessible from JavaScript.

NS_IMETHODIMP msiEditor::InsertText(const nsAString &aStringToInsert)
{
  if (!(mFlags & eEditorPlaintextMask)) // copied from nsHTMLEditor -- I don't know if this is an issue
  {
    nsresult res(NS_OK);
    nsCOMPtr<nsISelection> selection;
    nsCOMPtr<nsIDOMNode> startNode, endNode;
    PRInt32 startOffset(0), endOffset(0);
    PRBool bCollapsed(PR_FALSE);
    res = GetNSSelectionData(selection, startNode, startOffset, endNode, endOffset, bCollapsed);
    if (NS_SUCCEEDED(res) && NodeInMath(startNode) && aStringToInsert.Length() > 0)
    {

      nsCOMPtr<nsIDOMNode> theNode;
      PRInt32 theOffset(0);
      if (!bCollapsed)
      {
        res = DeleteSelection(nsIEditor::eNone);
        // need to set "theNode"
        if (NS_FAILED(res)) 
          return res;  // TODO -- is it not clear what to do here -- pass along to nsHTMLEditor
      }
      else
      {
        theNode = startNode;  
        theOffset = startOffset;
      }
      if (aStringToInsert.Length() > 1)
        res = InsertMathnameEx(selection, theNode, theOffset, aStringToInsert);
      else  
        res = InsertSymbolEx(selection, theNode, theOffset, aStringToInsert.First());
      return res;
    }
  }
  return nsHTMLEditor::InsertText(aStringToInsert);
}

PRBool msiEditor::NodeInMath(nsIDOMNode* node)
{
  PRBool rv(PR_FALSE);
  nsCOMPtr<nsIDOMNode> checkNode;
  if (IsTextContentNode(node))
    node->GetParentNode(getter_AddRefs(checkNode));
  else
    checkNode = node;  
  if (m_msiEditingMan && checkNode)
    m_msiEditingMan->SupportsMathMLInsertionInterface(checkNode, &rv);
  return rv;
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
  nsCOMPtr<nsITextContent> text(do_QueryInterface(node));
  return text ? PR_TRUE : PR_FALSE;
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
    res = GetStartNodeAndOffset(selection, address_of(startNode), &startOffset);
    if (NS_SUCCEEDED(res))
    {
      res = GetEndNodeAndOffset(selection, address_of(endNode), &endOffset);
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
  if (!NodeInMath(node))
  {
    //TODO
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
        if (!NodeInMath(theNode))
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
                          PRInt32 aOffset, PRUint32 aSymbol)
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
      default:
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




nsresult msiEditor::SetSelection(nsCOMPtr<nsIDOMNode> & focusNode, PRUint32 focusOffset, 
                                 PRBool selecting, PRBool & preventDefault)
{
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
    PRUint32 anchorOffset(msiIMathMLEditingBC::INVALID);
    PRUint32 startOffset(msiIMathMLEditingBC::INVALID), endOffset(msiIMathMLEditingBC::INVALID);
    PRInt32 compareFocusAnchor(0);
    
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
       nsCOMPtr<msiIMathMLCaret> mathCaret;
       res = GetMathMLCaretInterface(commonAncestor, 0, getter_AddRefs(mathCaret));
       if (NS_SUCCEEDED(res) && mathCaret)
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
             if (compareFocusAnchor < 0 ) //focus before anchor
             {
               res = mathCaret->GetSelectableMathFragment(this, 
                                                          focusNode, focusOffset,
                                                          nsnull, msiIMathMLEditingBC::INVALID,
                                                          getter_AddRefs(startNode), &startOffset,
                                                          getter_AddRefs(dummyNode), &dummyOffset);
               startSet  = NS_SUCCEEDED(res) && startNode && startOffset <= msiIMathMLEditingBC::LAST_VALID;
             }
             else
             {
               res = mathCaret->GetSelectableMathFragment(this, 
                                                          nsnull, msiIMathMLEditingBC::INVALID,
                                                          focusNode, focusOffset,
                                                          getter_AddRefs(dummyNode), &dummyOffset,
                                                          getter_AddRefs(endNode), &endOffset);
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
               endSet  = NS_SUCCEEDED(res) && endNode && endOffset <= msiIMathMLEditingBC::LAST_VALID;
             }
             else
             {
               res = mathCaret->GetSelectableMathFragment(this, 
                                                          anchorNode, anchorOffset,
                                                          nsnull, msiIMathMLEditingBC::INVALID,
                                                          getter_AddRefs(startNode), &startOffset,
                                                          getter_AddRefs(dummyNode), &dummyOffset);
               startSet  = NS_SUCCEEDED(res) && startNode && startOffset <= msiIMathMLEditingBC::LAST_VALID;
             }                                             
           }                                               
         }
         if (startSet || endSet)
         {
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
             }
             else
             {
               endNode = focusNode;
               endOffset = focusOffset;
             }                                             
           }
         }
       } // end focus and/or anchor maybe in math
       if (doSet)
       {
         msiSelection->Set(startNode, startOffset, endNode, endOffset,
                           focusNode, focusOffset, anchorNode, anchorOffset);
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

nsresult msiEditor::GetMouseDown(PRBool * isDown)
{
  nsCOMPtr<msiISelection> msiSelection;
  GetMSISelection(msiSelection);
  if (!msiSelection || !isDown)
    return NS_ERROR_FAILURE;
  return msiSelection->GetMouseDown(isDown);
}  

nsresult msiEditor::SetMouseDown(PRBool isDown)
{
  nsCOMPtr<msiISelection> msiSelection;
  GetMSISelection(msiSelection);
  if (!msiSelection)
    return NS_ERROR_FAILURE;
  return msiSelection->SetMouseDown(isDown);
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
  if (altDown || metaDown)
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
  if (NodeInMath(currNode))
  {
    PRUint32 caretOp = KeyCodeToCaretOp(keyCode, isShift, ctrlDown);
    res = GetNodeAndOffsetFromMMLCaretOp(caretOp, currNode, currOffset, newFocus, newOffset);
  }
  else
  {
    //TODO -- I don't like this code it use nsEditor's GetNextNode and GetPriorNode to get a text node
    //  and then I check is this node is in math -- kinda hacky and not robust.
    nsCOMPtr<nsIDOMNode> testNode;
    PRUint32 flags(msiIMathMLCaret::FLAGS_NONE);
    if (keyCode == nsIDOMKeyEvent::DOM_VK_RIGHT)
    {
      if (IsTextContentNode(currNode))
      {
        nsCOMPtr<nsIDOMCharacterData> chardata(do_QueryInterface(currNode));
        PRUint32 length(0);
        if (chardata)
        {
          chardata->GetLength(&length);
          if (length > 0)
          {
            if (currOffset == length)
            {
              nsCOMPtr<nsIDOMNode> nextnode;
              GetNextNode(currNode, PR_FALSE, address_of(nextnode), PR_FALSE); 
              if (nextnode)
                testNode = nextnode;
            }
            else 
            {
              newFocus = currNode;
              newOffset = currOffset+1;
            }
          }  
        }  
      }
      else
      {
        nsCOMPtr<nsIDOMNode> nextnode;
        GetNextNode(currNode, currOffset, PR_FALSE, address_of(nextnode), PR_FALSE); 
        if (nextnode)
          testNode = nextnode;
      }
    }  
    else if (keyCode == nsIDOMKeyEvent::DOM_VK_LEFT)
    {
      if (IsTextContentNode(currNode))
      {
        if (currOffset == 0)
        {
          nsCOMPtr<nsIDOMNode> priornode;
          GetPriorNode(currNode, PR_FALSE, address_of(priornode), PR_FALSE); 
          if (priornode)
            testNode = priornode;
        }
        else 
        {
          newFocus = currNode;
          newOffset = currOffset-1;
        }
      }
      else
      {
        nsCOMPtr<nsIDOMNode> priornode;
        GetPriorNode(currNode, currOffset, PR_FALSE, address_of(priornode), PR_FALSE); 
        if (priornode)
          testNode = priornode;
      }
    }
    if (testNode && NodeInMath(testNode))
    {
      nsCOMPtr<msiIMathMLCaret> mathmlEditing;
      PRUint32 offset(msiIMathMLEditingBC::INVALID);
      nsCOMPtr<nsIDOMNode> mathParent;
      GetMathParent(testNode, mathParent);
      if (mathParent)
      {
        if (keyCode == nsIDOMKeyEvent::DOM_VK_LEFT)
        {
          PRUint32 number(0);
          nsCOMPtr<nsIDOMNodeList> childNodes;
          res = mathParent->GetChildNodes(getter_AddRefs(childNodes));
          if (NS_SUCCEEDED(res) && childNodes)
            res = childNodes->GetLength(&number);
          offset = number;  
        }
        else
          offset = 0;
        GetMathMLCaretInterface(mathParent, offset, getter_AddRefs(mathmlEditing));
      }
      if (mathmlEditing)
      {
        PRUint32 flags = keyCode == nsIDOMKeyEvent::DOM_VK_RIGHT ? msiIMathMLCaret::FROM_LEFT : msiIMathMLCaret::FROM_RIGHT;
        res = mathmlEditing->Accept(this, flags, getter_AddRefs(newFocus), &newOffset);
      }
    }  
  }
  if (NS_SUCCEEDED(res) && newFocus && newOffset <= msiIMathMLEditingBC::LAST_VALID)
  {
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
    nsIDOMNode* child1 = NS_STATIC_CAST(nsIDOMNode*, parents1.FastElementAt(--pos1));
    nsIDOMNode* child2 = NS_STATIC_CAST(nsIDOMNode*, parents2.FastElementAt(--pos2));
    if (child1 != child2) 
      break;
    parent = child1;
  }
  commonAncestor = parent;
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
  