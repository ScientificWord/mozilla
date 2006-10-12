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
  nsCOMPtr<msiISelection> msiSelection;
  m_msiEditor->GetMSISelection(msiSelection);
  if (msiSelection)
    msiSelection->SetDOMEvent(aMouseEvent);
  return NS_OK;
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
  return NS_OK;
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
    
    PRInt32 screenX(NS_MAXSIZE), screenY(NS_MAXSIZE);
    mouseEvent->GetScreenX(&screenX);
    mouseEvent->GetScreenY(&screenY);
    nsPoint eventPoint(screenX, screenY);
    
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
        // relationship. Now see if the mouseEvent occured inside it's bounding
        // rect or close by.
        nsRect rect = kid->GetScreenRectExternal();
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
