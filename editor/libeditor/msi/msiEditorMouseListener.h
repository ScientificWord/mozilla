// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.
#ifndef msiEditorMouseListener_h__
#define msiEditorMouseListener_h__

#include "nsCOMPtr.h"
#include "nsIDOMEvent.h"
#include "nsHTMLEditorMouseListener.h"
#include "msiEditor.h"
#include "msiIMouse.h"

class nsIFrame;

 
class msiEditorMouseListener : public nsHTMLEditorMouseListener, public msiIMouse
{
public:
  msiEditorMouseListener(msiEditor *msiEditor);
  virtual ~msiEditorMouseListener();

  /*interfaces for addref and release and queryinterface*/
  NS_DECL_ISUPPORTS_INHERITED

/*BEGIN implementations of mouseevent handler interface*/
//  NS_IMETHOD HandleEvent(nsIDOMEvent* aEvent);
  NS_IMETHOD MouseDown(nsIDOMEvent* aMouseEvent);
  NS_IMETHOD MouseUp(nsIDOMEvent* aMouseEvent);
//  NS_IMETHOD MouseClick(nsIDOMEvent* aMouseEvent);
//  NS_IMETHOD MouseDblClick(nsIDOMEvent* aMouseEvent);
//  NS_IMETHOD MouseOver(nsIDOMEvent* aMouseEvent);
//  NS_IMETHOD MouseOut(nsIDOMEvent* aMouseEvent);
/*END implementations of mouseevent handler interface*/

  // msiIMouse
  NS_DECL_MSIIMOUSE

protected:

  msiEditor * m_msiEditor;
  PRBool m_mayDrag;
  PRBool m_didDrag;
};

//factory for the mouse listener
extern nsresult NS_NewMSIEditorMouseListener(nsIDOMEventListener ** aInstancePtrResult, msiEditor *msiEditor);

#endif //msiEditorMouseListener_h__

