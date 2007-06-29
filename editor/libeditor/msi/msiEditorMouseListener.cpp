// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.
#include "msiEditorMouseListener.h"
#include "nsHTMLEditorMouseListener.h"

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
#include "nsPresContext.h"
#include "nsIPrivateDOMEvent.h"
#include "nsGUIEvent.h"
#include "nsIDOMNSUIEvent.h"
#include "nsIDOMRange.h"
#include "msiISelection.h"
#include "nsIRangeUtils.h"
#include "nsContentUtils.h"

msiEditorMouseListener::msiEditorMouseListener(msiEditor *msiEditor)
  : nsHTMLEditorMouseListener(msiEditor), m_msiEditor(msiEditor)
{
  SetEditor(msiEditor); // Tell the base class about the editor.
  m_mayDrag = PR_FALSE;  //TODO
  m_didDrag = PR_FALSE;  //TODO
}

msiEditorMouseListener::~msiEditorMouseListener() 
{
}

NS_IMPL_ISUPPORTS_INHERITED1(msiEditorMouseListener, nsHTMLEditorMouseListener, msiIMouse)


nsresult
msiEditorMouseListener::MouseDown(nsIDOMEvent* aMouseEvent)
{
  if (!aMouseEvent || !m_msiEditor)  
    return NS_ERROR_NULL_POINTER;
  nsCOMPtr<msiISelection> msiSelection;
  m_msiEditor->GetMSISelection(msiSelection);
  if (msiSelection)
    msiSelection->SetDOMEvent(aMouseEvent);
  return nsHTMLEditorMouseListener::MouseDown(aMouseEvent);
}  


nsresult
msiEditorMouseListener::MouseUp(nsIDOMEvent* aMouseEvent)
{
  if (!aMouseEvent || !m_msiEditor)  
    return NS_ERROR_NULL_POINTER;
  nsCOMPtr<msiISelection> msiSelection;
  m_msiEditor->GetMSISelection(msiSelection);
  if (msiSelection)
    msiSelection->SetDOMEvent(aMouseEvent);
  return nsHTMLEditorMouseListener::MouseUp(aMouseEvent);
}

//msiIMouse
NS_IMETHODIMP
msiEditorMouseListener::GetMayDrag(PRBool *mayDrag)
{ 
  if (!mayDrag)
    return NS_ERROR_NULL_POINTER;
  *mayDrag = m_mayDrag;
  return NS_OK;
}
//end msiIMouse

nsresult
NS_NewMSIEditorMouseListener(nsIDOMEventListener ** aInstancePtrResult, 
                              msiEditor *msiEditor)
{
  msiEditorMouseListener* listener = new msiEditorMouseListener(msiEditor);
  if (!listener)
    return NS_ERROR_OUT_OF_MEMORY;
  return listener->QueryInterface(NS_GET_IID(nsIDOMEventListener), (void **)aInstancePtrResult);   
}
