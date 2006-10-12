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
  nsCOMPtr<msiISelection> msiSelection;
  m_msiEditor->GetMSISelection(msiSelection);
  if (msiSelection)
    msiSelection->SetDOMEvent(aMouseEvent);
  return NS_OK;
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
