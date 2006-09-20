// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.
#include "msiEditorMouseListener.h"
#include "nsHTMLEditorMouseListener.h"
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
  m_trackingMouse = PR_TRUE; //TODO -- need a interface to turn off undo of mouse movement
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
  
  nsCOMPtr<nsIDOMMouseEvent> mouseEvent (do_QueryInterface(aMouseEvent));
  if (!mouseEvent) 
    return NS_OK;
  nsresult res(NS_OK);
  PRBool defaultPrevented(PR_FALSE);
  nsCOMPtr<nsIDOMNode> eventNode;
  nsCOMPtr<nsIPresShell> presShell;
  nsCOMPtr<nsIDOMNode> focusNode;
  PRUint32 focusOffset(msiIMathMLEditingBC::INVALID);
  
  m_mayDrag = PR_FALSE;
  m_didDrag = PR_FALSE;
  // get the event point
  nsEvent * internalEvent = nsnull;
  nsCOMPtr<nsIPrivateDOMEvent> privateEvent(do_QueryInterface(aMouseEvent));
  if (privateEvent)
     privateEvent->GetInternalNSEvent(&internalEvent);
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
    // Detect only "context menu" click
    //XXX This should be easier to do!
    // But eDOMEvents_contextmenu and NS_CONTEXTMENU is not exposed in any event interface :-(
    PRUint16 buttonNumber(0);
    PRInt32 clickCount(0);
    PRBool shiftDown(PR_FALSE);
    PRBool isContextClick = IsContextClick(mouseEvent);
    res = mouseEvent->GetButton(&buttonNumber);
    if (NS_SUCCEEDED(res))
      res = mouseEvent->GetShiftKey(&shiftDown);
    if (NS_SUCCEEDED(res))
      res = mouseEvent->GetDetail(&clickCount);
      
    PRBool selectionIsCollapsed = m_msiEditor->IsSelectionCollapsed();   
    if (isContextClick)
    {
     //TODO context menus!!!! 
     res = NS_OK;
    }
    else if (buttonNumber == 0 && clickCount == 1)
    {
      PRBool doSet(PR_TRUE);
      if (!selectionIsCollapsed && !shiftDown)
      {
        PRBool withinSelection(PR_FALSE);
        m_msiEditor->IsPointWithinCurrentSelection(focusNode, focusOffset, withinSelection);
        doSet = !withinSelection;
      }  
      if (doSet)
      {
        PRBool preventDefault(PR_FALSE);
        res = m_msiEditor->SetSelection(focusNode, focusOffset, shiftDown, preventDefault);
        if (NS_SUCCEEDED(res) && preventDefault)
          aMouseEvent->PreventDefault();
      }
      else
      {
        //TODO -- drag and drop
        aMouseEvent->PreventDefault();
        m_mayDrag = !selectionIsCollapsed;
      }    
    }
    
    nsCOMPtr<nsIDOMNSUIEvent> uiEvent(do_QueryInterface(aMouseEvent));
    if (uiEvent)
      uiEvent->GetPreventDefault(&defaultPrevented);
  }
  if (!defaultPrevented)  
    res = nsHTMLEditorMouseListener::MouseDown(aMouseEvent);
  else
    res = m_msiEditor->SetMouseDown(PR_TRUE);
 return res;
}  


nsresult
msiEditorMouseListener::MouseUp(nsIDOMEvent* aMouseEvent)
{
  PRBool mayDrag(m_mayDrag), didDrag(m_didDrag);
  m_mayDrag = PR_FALSE;
  m_didDrag = PR_FALSE;
  if (!aMouseEvent || !m_msiEditor)  
    return NS_ERROR_NULL_POINTER;
  
  nsCOMPtr<nsIDOMMouseEvent> mouseEvent (do_QueryInterface(aMouseEvent));
  if (!mouseEvent) 
    return NS_OK;
  nsresult res(NS_OK);
  PRBool defaultPrevented(PR_FALSE);
  PRBool selectionIsCollapsed = m_msiEditor->IsSelectionCollapsed();   
  if (!selectionIsCollapsed && mayDrag && !didDrag)
  {
    nsCOMPtr<nsIDOMNode> eventNode;
    nsCOMPtr<nsIPresShell> presShell;
    nsCOMPtr<nsIDOMNode> focusNode;
    PRUint32 focusOffset(msiIMathMLEditingBC::INVALID);
    
    // get the event point
    nsEvent * internalEvent = nsnull;
    nsCOMPtr<nsIPrivateDOMEvent> privateEvent(do_QueryInterface(aMouseEvent));
    if (privateEvent)
       privateEvent->GetInternalNSEvent(&internalEvent);
       
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
      PRUint16 buttonNumber(0);
      PRInt32 clickCount(0);
      res = mouseEvent->GetButton(&buttonNumber);
      if (NS_SUCCEEDED(res))
        res = mouseEvent->GetDetail(&clickCount);
      PRBool withinSelection(PR_FALSE);
      m_msiEditor->IsPointWithinCurrentSelection(focusNode, focusOffset, withinSelection);
        
      if (buttonNumber == 0 && clickCount <= 1 && withinSelection)
      {
        PRBool preventDefault(PR_FALSE);
        res = m_msiEditor->SetSelection(focusNode, focusOffset, PR_FALSE, preventDefault);
        if (NS_SUCCEEDED(res) && preventDefault)
          aMouseEvent->PreventDefault();
      }
      nsCOMPtr<nsIDOMNSUIEvent> uiEvent(do_QueryInterface(aMouseEvent));
      if (uiEvent)
        uiEvent->GetPreventDefault(&defaultPrevented);
    }
  }
  if (!defaultPrevented)  
    res = nsHTMLEditorMouseListener::MouseUp(aMouseEvent);
  else
    res = m_msiEditor->SetMouseDown(PR_FALSE);
 return res;
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
msiEditorMouseListener::GetClosestEditingNode(msiEditor* msiEditor, nsIDOMEvent* aMouseEvent, nsCOMPtr<nsIDOMNode> & editingNode)
{
  if (!aMouseEvent || !msiEditor)  
    return NS_ERROR_NULL_POINTER;
  
  nsCOMPtr<nsIDOMMouseEvent> mouseEvent (do_QueryInterface(aMouseEvent));
  if (!mouseEvent) 
    return NS_ERROR_FAILURE;
    
  nsresult res(NS_ERROR_FAILURE);
  nsCOMPtr<nsIDOMEventTarget> target;
  nsCOMPtr<nsIDOMNode> node;
  nsCOMPtr<nsIPresShell> presShell;
  nsPoint eventPoint(0,0);
  // get the event point
  if (aMouseEvent)
  {
    nsEvent * internalEvent = nsnull;
    nsCOMPtr<nsIPrivateDOMEvent> privateEvent(do_QueryInterface(aMouseEvent));
    if (privateEvent)
       privateEvent->GetInternalNSEvent(&internalEvent);
    if (internalEvent)
      eventPoint = internalEvent->refPoint;
  }
  res = msiEditor->GetPresShell(getter_AddRefs(presShell));
  nsCOMPtr<nsIDOMNSEvent> internalEvent = do_QueryInterface(aMouseEvent);
  if (NS_SUCCEEDED(res) && internalEvent)
    res = internalEvent->GetExplicitOriginalTarget(getter_AddRefs(target));
  if (NS_SUCCEEDED(res) && target)
    node = do_QueryInterface(target);
  if (NS_SUCCEEDED(res) && presShell && node)
  { 
    // TODO LJH 4/06 -- layout code works much harder to determine who gets the click
    // I don't think this simplification is sufficant. Layout has block code, line code, peek code...
    // and the final determination will minimize the y-distance.  see PresShell::HandleEventInternal.
    // In particular, if the frame is an areaframe (the whole document?), I probably want to limit the 
    // search.
    
    // get base frame
    nsIFrame *closestFrame = nsnull;
    nsCOMPtr<nsIContent> content(do_QueryInterface(node));
    if (content)
      closestFrame = presShell->GetPrimaryFrameFor(content);
    if (msiEditorMouseListener::IsEditableFrame(closestFrame))
    {
      editingNode = node;
      closestFrame = nsnull;
    }
    //determine closed editing node - if any exist via frame geometry  
    while (closestFrame)
    {
      //ljh 4/06 The algorithm below is taken from nsFrame::GetContentAndOffsetsFromPoint
      // which is used to determine where a mouse down will land.  
      nsIFrame *kid = nsnull;
      kid = closestFrame->GetFirstChild(nsnull);
      closestFrame = nsnull;
      PRInt32 closestXDistance = PR_INT32_MAX;
      PRInt32 closestYDistance = PR_INT32_MAX;
      while (nsnull != kid) 
      {
        // Skip over generated content kid frames, or frames
        // that don't have a proper parent-child relationship!
        PRBool skipThisKid = (kid->GetStateBits() & NS_FRAME_GENERATED_CONTENT) != 0;
        if (skipThisKid) 
        {
          kid = kid->GetNextSibling();
          continue;
        }
        // Kid frame has content that has a proper parent-child
        // relationship. Now see if the eventPoint inside it's bounding
        // rect or close by.
        nsPoint offsetPoint(0,0);
        nsIView * kidView = nsnull;
        kid->GetOffsetFromView(offsetPoint, &kidView);
        nsRect rect = kid->GetRect();
        rect.x = offsetPoint.x;
        rect.y = offsetPoint.y;
        PRInt32 fromLeft = eventPoint.x - rect.x;
        PRInt32 fromRight = eventPoint.x - rect.x - rect.width;
 
        PRInt32 xDistance;
        if (fromLeft > 0 && fromRight < 0)
          xDistance = 0;
        else
          xDistance = PR_MIN(abs(fromLeft), abs(fromRight));
        if (xDistance <= closestXDistance && rect.width > 0 && rect.height > 0)
        {
          if (xDistance < closestXDistance)
            closestYDistance = PR_INT32_MAX;
          PRInt32 fromTop = eventPoint.y - rect.y;
          PRInt32 fromBottom = eventPoint.y - rect.y - rect.height;
 
          PRInt32 yDistance;
          if (fromTop > 0 && fromBottom < 0)
            yDistance = 0;
          else
            yDistance = PR_MIN(abs(fromTop), abs(fromBottom));
          
          if (xDistance == 0 && yDistance == 0)
          {
            closestFrame = kid;
            break;
          }
          if (yDistance < closestYDistance || (yDistance == closestYDistance && rect.x <= eventPoint.x))
          {
            closestXDistance = xDistance;
            closestYDistance = yDistance;
            closestFrame     = kid;
          }
        }
        kid = kid->GetNextSibling();
      }
      if (closestFrame && IsEditableFrame(closestFrame))
      {
        nsIContent * content = closestFrame->GetContent();
        if (content)
          editingNode = do_QueryInterface(content);
        closestFrame = nsnull;
      }
    }
  }
  return res;
}   

PRBool msiEditorMouseListener::IsEditableFrame(nsIFrame * testFrame)
{
  PRBool rv(PR_FALSE);
  if (testFrame)
  {
    //TODO -- I don't know the role of most of the layout frame types!!!! - so I don't know if they will accept the  caret 
    nsIAtom* type = testFrame->GetType();
    if (type == msiLayoutAtoms::textFrame                      ||
        type == msiLayoutAtoms::imageFrame                     ||    
        type == msiLayoutAtoms::operatorOrdinaryMathMLFrame    ||
        type == msiLayoutAtoms::operatorInvisibleMathMLFrame   ||
        type == msiLayoutAtoms::operatorUserDefinedMathMLFrame ||
        type == msiLayoutAtoms::ordinaryMathMLFrame            ||
        type == msiLayoutAtoms::italicIdentifierMathMLFrame    ||
        type == msiLayoutAtoms::uprightIdentifierMathMLFrame   ||
        type == msiLayoutAtoms::innerMathMLFrame                )
      rv = PR_TRUE;
  }
  return rv;
}

PRBool msiEditorMouseListener::IsContextClick(nsCOMPtr<nsIDOMMouseEvent> & mouseEvent)
{
  PRBool isContextClick(PR_FALSE);
  PRUint16 buttonNumber(0);
  PRInt32 clickCount(0);
  nsresult res = mouseEvent->GetButton(&buttonNumber);
  if (NS_SUCCEEDED(res))
  {
    #if defined(XP_MAC) || defined(XP_MACOSX)
    // Ctrl+Click for context menu
      res = mouseEvent->GetCtrlKey(&isContextClick);
    #else
    // Right mouse button for Windows, UNIX
      isContextClick = buttonNumber == 2;
    #endif
    if (NS_SUCCEEDED(res) && !isContextClick)
    {
        res = mouseEvent->GetDetail(&clickCount);
      if (NS_SUCCEEDED(res))
        isContextClick = (buttonNumber == 0 && clickCount == 2);
    }    
  }
  if (NS_FAILED(res))
    isContextClick = PR_FALSE;
  return isContextClick;
}      

 
nsresult
NS_NewMSIEditorMouseListener(nsIDOMEventListener ** aInstancePtrResult, 
                              msiEditor *msiEditor)
{
  msiEditorMouseListener* listener = new msiEditorMouseListener(msiEditor);
  if (!listener)
    return NS_ERROR_OUT_OF_MEMORY;
  return listener->QueryInterface(NS_GET_IID(nsIDOMEventListener), (void **)aInstancePtrResult);   
}
