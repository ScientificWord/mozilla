// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.
#include "msiEditorMouseMotionListener.h"
#include "msiEditorMouseListener.h"
#include "msiLayoutAtoms.h"

#include "msiIMathMLCaret.h"
#include "nsIDOMEventTarget.h"
#include "nsIDOMNode.h"
#include "nsIDOMNSEvent.h"
#include "nsIDOMMouseEvent.h"
#include "nsISelection.h"
#include "nsIEditor.h"
#include "nsIPresShell.h"
#include "nsIFrame.h"
#include "nsIContent.h"
#include "nsIView.h"
#include "nsIPrivateDOMEvent.h"
#include "nsGUIEvent.h"
#include "nsIDOMNSUIEvent.h"
#include "nsIDOMRange.h"
#include "msiISelection.h"
#include "nsIRangeUtils.h"
#include "nsContentUtils.h"

msiEditorMouseMotionListener::msiEditorMouseMotionListener(msiEditor *msiEditor)
  : m_msiEditor(msiEditor)
{
}

msiEditorMouseMotionListener::~msiEditorMouseMotionListener() 
{
}

NS_IMPL_ISUPPORTS2(msiEditorMouseMotionListener, nsIDOMEventListener, nsIDOMMouseMotionListener)

nsresult
msiEditorMouseMotionListener::HandleEvent(nsIDOMEvent* aEvent)
{
  return NS_OK;
}


nsresult
msiEditorMouseMotionListener::MouseMove(nsIDOMEvent* aMouseEvent)
{
  if (!aMouseEvent || !m_msiEditor)  
    return NS_ERROR_NULL_POINTER;
  PRBool mayDrag(PR_FALSE);
  m_msiEditor->GetMayDrag(&mayDrag);
  nsCOMPtr<nsIDOMMouseEvent> mouseEvent (do_QueryInterface(aMouseEvent));
  if (!mouseEvent) 
    return NS_OK;
  PRBool mouseDown(PR_FALSE);
  PRBool preventDefault(PR_FALSE);
  nsresult res = m_msiEditor->GetMouseDown(&mouseDown);
  nsEvent * internalEvent = nsnull;
  nsCOMPtr<nsIPrivateDOMEvent> privateEvent(do_QueryInterface(aMouseEvent));
  if (privateEvent)
     privateEvent->GetInternalNSEvent(&internalEvent);
  if (mayDrag)
  {
    //TODO
    preventDefault = PR_TRUE;
    res = NS_OK;
  }
  else if (NS_SUCCEEDED(res) && mouseDown)
  {  
    PRBool defaultPrevented(PR_FALSE);
    nsCOMPtr<nsIDOMNode> eventNode;
    nsCOMPtr<nsIPresShell> presShell;
    nsCOMPtr<nsIDOMNode> focusNode;
    PRUint32 focusOffset(msiIMathMLEditingBC::INVALID);
    
    nsGUIEvent* guiEvent = nsnull;
    if (internalEvent && internalEvent->eventStructType == NS_MOUSE_EVENT)
      guiEvent = NS_STATIC_CAST(nsGUIEvent*, internalEvent);
    if (!guiEvent)
    {
      NS_ASSERTION(PR_FALSE, "internal event is not a guiEvent.");
      res = NS_ERROR_FAILURE;
    }  
    if (NS_SUCCEEDED(res))
      res = msiEditorMouseListener::GetClosestEditingNode(m_msiEditor, aMouseEvent, eventNode);
    
    // get node (focusNode) and offset of the mouse event
    if (NS_SUCCEEDED(res) && eventNode)
    {
      res = m_msiEditor->GetPresShell(getter_AddRefs(presShell));
      nsCOMPtr<msiIMathMLCaret> mathCaret;  
      if (NS_SUCCEEDED(res) && presShell)
        res = m_msiEditor->GetMathMLCaretInterface(eventNode, msiIMathMLEditingBC::INVALID, getter_AddRefs(mathCaret));
      if (NS_SUCCEEDED(res) && mathCaret)
        res = mathCaret->GetNodeAndOffsetFromMouseEvent(m_msiEditor, presShell, msiIMathMLCaret::FLAGS_NONE, 
                                                        mouseEvent, getter_AddRefs(focusNode), &focusOffset);
      else
      {
        //TODO -- click outside of math -- this may need to be handled also.
      }  
    }  
    if (NS_SUCCEEDED(res) && focusNode && focusOffset <= msiIMathMLEditingBC::LAST_VALID)
    {   
      res = m_msiEditor->SetSelection(focusNode, focusOffset, PR_TRUE, preventDefault);
    }
  }
  if (NS_SUCCEEDED(res) && preventDefault)
  {
    //TODO --ljh hack nsMouseEvent constructor sets NS_EVENT_FLAG_CANT_CANCEL for mouse move events
    // which blocks the setting of prevent default.
    if (internalEvent)
      internalEvent->flags &= ~NS_EVENT_FLAG_CANT_CANCEL; 
    aMouseEvent->PreventDefault();
  }
  return res;
} 

nsresult
msiEditorMouseMotionListener::DragMove(nsIDOMEvent* aMouseEvent)
{
  //TODO
  if (!aMouseEvent || !m_msiEditor)  
    return NS_ERROR_NULL_POINTER;
  return NS_OK;
}    



nsresult
NS_NewMSIEditorMouseMotionListener(nsIDOMEventListener ** aInstancePtrResult, 
                                   msiEditor *msiEditor)
{
  msiEditorMouseMotionListener* listener = new msiEditorMouseMotionListener(msiEditor);
  if (!listener)
    return NS_ERROR_OUT_OF_MEMORY;
  return listener->QueryInterface(NS_GET_IID(nsIDOMEventListener), (void **)aInstancePtrResult);   
}
