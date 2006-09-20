// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.
#ifndef msiEditorMouseMotionListener_h__
#define msiEditorMouseMotionListener_h__

#include "nsCOMPtr.h"
#include "nsIDOMEvent.h"
#include "msiEditor.h"
#include "nsIDOMMouseMotionListener.h"

 
class msiEditorMouseMotionListener : public nsIDOMMouseMotionListener
{
public:
  msiEditorMouseMotionListener(msiEditor *msiEditor);
  virtual ~msiEditorMouseMotionListener();

  /*interfaces for addref and release and queryinterface*/
  NS_DECL_ISUPPORTS

/*BEGIN implementations of mouse motion event handler interface*/
  NS_IMETHOD HandleEvent(nsIDOMEvent* aEvent);
  NS_IMETHOD MouseMove(nsIDOMEvent* aMouseEvent);
  NS_IMETHOD DragMove(nsIDOMEvent* aMouseEvent);
/*END implementations of mouseevent handler interface*/

protected:
  msiEditor * m_msiEditor;
};

//factory for the mouse listener
extern nsresult NS_NewMSIEditorMouseMotionListener(nsIDOMEventListener ** aInstancePtrResult, msiEditor *msiEditor);

#endif //msiEditorMouseMotionListener_h__

