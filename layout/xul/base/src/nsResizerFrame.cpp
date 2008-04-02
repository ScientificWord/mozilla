/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Mozilla Communicator client code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Original Author: Eric J. Burley (ericb@neoplanet.com)
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either of the GNU General Public License Version 2 or later (the "GPL"),
 * or the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#include "nsCOMPtr.h"
#include "nsResizerFrame.h"
#include "nsIContent.h"
#include "nsIDocument.h"
#include "nsIDOMXULDocument.h"
#include "nsIDOMNodeList.h"
#include "nsGkAtoms.h"
#include "nsINameSpaceManager.h"

#include "nsIWidget.h"
#include "nsPresContext.h"
#include "nsIDocShell.h"
#include "nsIDocShellTreeItem.h"
#include "nsIDocShellTreeOwner.h"
#include "nsIBaseWindow.h"
#include "nsPIDOMWindow.h"
#include "nsGUIEvent.h"
#include "nsEventDispatcher.h"

//
// NS_NewResizerFrame
//
// Creates a new Resizer frame and returns it
//
nsIFrame*
NS_NewResizerFrame(nsIPresShell* aPresShell, nsStyleContext* aContext)
{
  return new (aPresShell) nsResizerFrame(aPresShell, aContext);
} // NS_NewResizerFrame

nsResizerFrame::nsResizerFrame(nsIPresShell* aPresShell, nsStyleContext* aContext)
:nsTitleBarFrame(aPresShell, aContext)
{
  mDirection = topleft; // by default...
}

NS_IMETHODIMP
nsResizerFrame::Init(nsIContent*      aContent,
                     nsIFrame*        aParent,
                     nsIFrame*        asPrevInFlow)
{
  nsresult rv = nsTitleBarFrame::Init(aContent, aParent, asPrevInFlow);

  GetInitialDirection(mDirection);

  return rv;
}


NS_IMETHODIMP
nsResizerFrame::HandleEvent(nsPresContext* aPresContext,
                            nsGUIEvent* aEvent,
                            nsEventStatus* aEventStatus)
{
  nsWeakFrame weakFrame(this);
  PRBool doDefault = PR_TRUE;

  switch (aEvent->message) {

   case NS_MOUSE_BUTTON_DOWN: {
       if (aEvent->eventStructType == NS_MOUSE_EVENT &&
           static_cast<nsMouseEvent*>(aEvent)->button ==
             nsMouseEvent::eLeftButton)
       {

         nsresult rv = NS_OK;

         // what direction should we go in? 
         // convert eDirection to horizontal and vertical directions
         static const PRInt8 directions[][2] = {
           {-1, -1}, {0, -1}, {1, -1},
           {-1,  0},          {1,  0},
           {-1,  1}, {0,  1}, {1,  1}
         };

         // ask the widget implementation to begin a resize drag if it can
         rv = aEvent->widget->BeginResizeDrag(aEvent, 
             directions[mDirection][0], directions[mDirection][1]);

         if (rv == NS_ERROR_NOT_IMPLEMENTED) {
           // there's no native resize support, 
           // we need to window resizing ourselves

           // we're tracking.
           mTrackingMouseMove = PR_TRUE;

           // start capture.
           aEvent->widget->CaptureMouse(PR_TRUE);
           CaptureMouseEvents(aPresContext,PR_TRUE);

           // remember current mouse coordinates.
           mLastPoint = aEvent->refPoint;
           aEvent->widget->GetScreenBounds(mWidgetRect);
         }

         *aEventStatus = nsEventStatus_eConsumeNoDefault;
         doDefault = PR_FALSE;
       }
     }
     break;


   case NS_MOUSE_BUTTON_UP: {

       if(mTrackingMouseMove && aEvent->eventStructType == NS_MOUSE_EVENT &&
          static_cast<nsMouseEvent*>(aEvent)->button ==
            nsMouseEvent::eLeftButton)
       {
         // we're done tracking.
         mTrackingMouseMove = PR_FALSE;

         // end capture
         aEvent->widget->CaptureMouse(PR_FALSE);
         CaptureMouseEvents(aPresContext,PR_FALSE);

         *aEventStatus = nsEventStatus_eConsumeNoDefault;
         doDefault = PR_FALSE;
       }
     }
     break;

   case NS_MOUSE_MOVE: {
       if(mTrackingMouseMove)
       {
         // get the document and the window - should this be cached?
         nsPIDOMWindow *domWindow =
           aPresContext->PresShell()->GetDocument()->GetWindow();
         NS_ENSURE_TRUE(domWindow, NS_ERROR_FAILURE);

         nsCOMPtr<nsIDocShellTreeItem> docShellAsItem =
           do_QueryInterface(domWindow->GetDocShell());
         NS_ENSURE_TRUE(docShellAsItem, NS_ERROR_FAILURE);

         nsCOMPtr<nsIDocShellTreeOwner> treeOwner;
         docShellAsItem->GetTreeOwner(getter_AddRefs(treeOwner));

         nsCOMPtr<nsIBaseWindow> window(do_QueryInterface(treeOwner));

         if (!window) {
           return NS_OK;
         }

         nsPoint nsMoveBy(0,0),nsSizeBy(0,0);
         nsPoint nsMouseMove(aEvent->refPoint - mLastPoint);

         switch(mDirection)
         {
            case topleft:
              nsMoveBy = nsMouseMove;
              nsSizeBy -= nsMouseMove;
              break;
            case top:
              nsMoveBy.y = nsMouseMove.y;
              nsSizeBy.y = - nsMouseMove.y;
              break;
            case topright:
              nsMoveBy.y = nsMouseMove.y;
              nsSizeBy.x = nsMouseMove.x;
              mLastPoint.x += nsMouseMove.x;
              nsSizeBy.y = -nsMouseMove.y;
              break;
            case left:
              nsMoveBy.x = nsMouseMove.x;
              nsSizeBy.x = -nsMouseMove.x;
              break;
            case right:
              nsSizeBy.x = nsMouseMove.x;
              mLastPoint.x += nsMouseMove.x;
              break;
            case bottomleft:
              nsMoveBy.x = nsMouseMove.x;
              nsSizeBy.y = nsMouseMove.y;
              nsSizeBy.x = -nsMouseMove.x;
              mLastPoint.y += nsMouseMove.y;
              break;
            case bottom:
              nsSizeBy.y = nsMouseMove.y;
              mLastPoint.y += nsMouseMove.y;
              break;
            case bottomright:
              nsSizeBy = nsMouseMove;
              mLastPoint += nsMouseMove;
              break;
         }

         PRInt32 x,y,cx,cy;
         window->GetPositionAndSize(&x,&y,&cx,&cy);

         x+=nsMoveBy.x;
         y+=nsMoveBy.y;
         cx+=nsSizeBy.x;
         cy+=nsSizeBy.y;

         window->SetPositionAndSize(x,y,cx,cy,PR_TRUE); // do the repaint.

         /*
         if(nsSizeBy.x || nsSizeBy.y)
         {
          window->ResizeBy(nsSizeBy.x,nsSizeBy.y);
         }

         if(nsMoveBy.x || nsMoveBy.y)
         {
          window->MoveBy(nsMoveBy.x,nsMoveBy.y);
         }  */

         *aEventStatus = nsEventStatus_eConsumeNoDefault;

         doDefault = PR_FALSE;
       }
     }
     break;



    case NS_MOUSE_CLICK:
      if (NS_IS_MOUSE_LEFT_CLICK(aEvent))
      {
        MouseClicked(aPresContext, aEvent);
      }
      break;
  }

  if (doDefault && weakFrame.IsAlive())
    return nsTitleBarFrame::HandleEvent(aPresContext, aEvent, aEventStatus);
  else
    return NS_OK;
}



/* returns true if aText represented a valid direction
 */
PRBool
nsResizerFrame::EvalDirection(nsAutoString& aText,eDirection& aDir)
{
  PRBool aResult = PR_TRUE;

  if( aText.Equals( NS_LITERAL_STRING("topleft") ) )
  {
    aDir = topleft;
  }
  else if( aText.Equals( NS_LITERAL_STRING("top") ) )
  {
    aDir = top;
  }
  else if( aText.Equals( NS_LITERAL_STRING("topright") ) )
  {
    aDir = topright;
  }
  else if( aText.Equals( NS_LITERAL_STRING("left") ) )
  {
    aDir = left;
  }
  else if( aText.Equals( NS_LITERAL_STRING("right") ) )
  {
    aDir = right;
  }
  else if( aText.Equals( NS_LITERAL_STRING("bottomleft") ) )
  {
    aDir = bottomleft;
  }
  else if( aText.Equals( NS_LITERAL_STRING("bottom") ) )
  {
    aDir = bottom;
  }
  else if( aText.Equals( NS_LITERAL_STRING("bottomright") ) )
  {
    aDir = bottomright;
  }
  else
  {
    aResult = PR_FALSE;
  }

  return aResult;
}


/* Returns true if it was set.
 */
PRBool
nsResizerFrame::GetInitialDirection(eDirection& aDirection)
{
 // see what kind of resizer we are.
  nsAutoString value;

  if (!GetContent())
     return PR_FALSE;

  if (GetContent()->GetAttr(kNameSpaceID_None, nsGkAtoms::dir, value)) {
     return EvalDirection(value,aDirection);
  }

  return PR_FALSE;
}


NS_IMETHODIMP
nsResizerFrame::AttributeChanged(PRInt32 aNameSpaceID,
                                 nsIAtom* aAttribute,
                                 PRInt32 aModType)
{
  nsresult rv = nsTitleBarFrame::AttributeChanged(aNameSpaceID, aAttribute,
                                                  aModType);

  if (aAttribute == nsGkAtoms::dir) {
    GetInitialDirection(mDirection);
  }

  return rv;
}



void
nsResizerFrame::MouseClicked(nsPresContext* aPresContext, nsGUIEvent *aEvent)
{
  // Execute the oncommand event handler.
  nsEventStatus status = nsEventStatus_eIgnore;

  nsXULCommandEvent event(aEvent ? NS_IS_TRUSTED_EVENT(aEvent) : PR_FALSE,
                          NS_XUL_COMMAND, nsnull);

  nsEventDispatcher::Dispatch(mContent, aPresContext, &event, nsnull, &status);
}
