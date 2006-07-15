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
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
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

#include "nsFileControlFrame.h"

#include "nsIContent.h"
#include "prtypes.h"
#include "nsIAtom.h"
#include "nsPresContext.h"
#include "nsHTMLAtoms.h"
#include "nsWidgetsCID.h"
#include "nsIComponentManager.h"
#include "nsIView.h"
#include "nsHTMLParts.h"
#include "nsIDOMHTMLInputElement.h"
#include "nsIFormControl.h"
#include "nsINameSpaceManager.h"
#include "nsCOMPtr.h"
#include "nsISupportsArray.h"
#include "nsIDOMElement.h"
#include "nsIDOMDocument.h"
#include "nsIDocument.h"
#include "nsIDOMMouseListener.h"
#include "nsIPresShell.h"
#include "nsIDOMHTMLInputElement.h"
#include "nsXPCOM.h"
#include "nsISupportsPrimitives.h"
#include "nsIComponentManager.h"
#include "nsPIDOMWindow.h"
#include "nsIFilePicker.h"
#include "nsIDOMMouseEvent.h"
#include "nsINodeInfo.h"
#include "nsIDOMEventReceiver.h"
#include "nsILocalFile.h"
#include "nsIFileControlElement.h"
#include "nsNodeInfoManager.h"
#include "nsContentCreatorFunctions.h"
#include "nsContentUtils.h"
#include "nsDisplayList.h"

#define SYNC_TEXT 0x1
#define SYNC_BUTTON 0x2
#define SYNC_BOTH 0x3

nsIFrame*
NS_NewFileControlFrame(nsIPresShell* aPresShell, nsStyleContext* aContext)
{
  return new (aPresShell) nsFileControlFrame(aContext);
}

nsFileControlFrame::nsFileControlFrame(nsStyleContext* aContext):
  nsAreaFrame(aContext),
  mTextFrame(nsnull), 
  mCachedState(nsnull)
{
    //Shrink the area around its contents
  SetFlags(NS_BLOCK_SHRINK_WRAP);
}

nsFileControlFrame::~nsFileControlFrame()
{
  // remove ourself as a listener of the button (bug 40533)
  if (mBrowse) {
    nsCOMPtr<nsIDOMEventReceiver> receiver(do_QueryInterface(mBrowse));
    receiver->RemoveEventListenerByIID(this, NS_GET_IID(nsIDOMMouseListener));
  }
  if (mTextContent) {
    nsCOMPtr<nsIDOMEventReceiver> receiver(do_QueryInterface(mTextContent));
    receiver->RemoveEventListenerByIID(this, NS_GET_IID(nsIDOMMouseListener));
  }

  if (mCachedState) {
    delete mCachedState;
    mCachedState = nsnull;
  }
}

void
nsFileControlFrame::Destroy()
{
  mTextFrame = nsnull;
  nsAreaFrame::Destroy();
}

NS_IMETHODIMP
nsFileControlFrame::CreateAnonymousContent(nsPresContext* aPresContext,
                                           nsISupportsArray& aChildList)
{
  // Get the NodeInfoManager and tag necessary to create input elements
  nsCOMPtr<nsIDocument> doc = mContent->GetDocument();

  nsCOMPtr<nsINodeInfo> nodeInfo;
  doc->NodeInfoManager()->GetNodeInfo(nsHTMLAtoms::input, nsnull,
                                      kNameSpaceID_None,
                                      getter_AddRefs(nodeInfo));

  // Create the text content
  nsCOMPtr<nsIContent> content;
  nsresult rv = NS_NewHTMLElement(getter_AddRefs(content), nodeInfo);
  NS_ENSURE_SUCCESS(rv, rv);

  content.swap(mTextContent);

  if (mTextContent) {
    mTextContent->SetAttr(kNameSpaceID_None, nsHTMLAtoms::type, NS_LITERAL_STRING("text"), PR_FALSE);

    nsCOMPtr<nsIDOMHTMLInputElement> textControl = do_QueryInterface(mTextContent);
    if (textControl) {
      nsCOMPtr<nsIFileControlElement> fileControl = do_QueryInterface(mContent);
      if (fileControl) {
        // Initialize value when we create the content in case the value was set
        // before we got here
        nsAutoString value;
        fileControl->GetFileName(value);
        textControl->SetValue(value);
      }
      
      textControl->SetTabIndex(-1);
      textControl->SetDisabled(PR_TRUE);
      textControl->SetReadOnly(PR_TRUE);
    }

    aChildList.AppendElement(mTextContent);

    // register as an event listener of the textbox to open file dialog on mouse click
    nsCOMPtr<nsIDOMEventReceiver> receiver(do_QueryInterface(mTextContent));
    receiver->AddEventListenerByIID(this, NS_GET_IID(nsIDOMMouseListener));
  }

  // Create the browse button
  rv = NS_NewHTMLElement(getter_AddRefs(content), nodeInfo);
  NS_ENSURE_SUCCESS(rv, rv);

  mBrowse = do_QueryInterface(content);
  if (mBrowse) {
    mBrowse->SetAttr(kNameSpaceID_None, nsHTMLAtoms::type, NS_LITERAL_STRING("button"), PR_FALSE);
    nsCOMPtr<nsIDOMHTMLInputElement> fileContent = do_QueryInterface(mContent);
    nsCOMPtr<nsIDOMHTMLInputElement> browseControl = do_QueryInterface(mBrowse);
    if (fileContent && browseControl) {
      PRInt32 tabIndex;
      nsAutoString accessKey;
      
      fileContent->GetAccessKey(accessKey);
      browseControl->SetAccessKey(accessKey);
      fileContent->GetTabIndex(&tabIndex);
      browseControl->SetTabIndex(tabIndex);
    }

    aChildList.AppendElement(mBrowse);

    // register as an event listener of the button to open file dialog on mouse click
    nsCOMPtr<nsIDOMEventReceiver> receiver(do_QueryInterface(mBrowse));
    receiver->AddEventListenerByIID(this, NS_GET_IID(nsIDOMMouseListener));
  }

  SyncAttr(kNameSpaceID_None, nsHTMLAtoms::size,     SYNC_TEXT);
  SyncAttr(kNameSpaceID_None, nsHTMLAtoms::disabled, SYNC_BOTH);

  return NS_OK;
}

// Frames are not refcounted, no need to AddRef
NS_IMETHODIMP
nsFileControlFrame::QueryInterface(const nsIID& aIID, void** aInstancePtr)
{
  NS_PRECONDITION(0 != aInstancePtr, "null ptr");
  if (NULL == aInstancePtr) {
    return NS_ERROR_NULL_POINTER;
  } else if (aIID.Equals(NS_GET_IID(nsIAnonymousContentCreator))) {
    *aInstancePtr = (void*)(nsIAnonymousContentCreator*) this;
    return NS_OK;
  } else if (aIID.Equals(NS_GET_IID(nsIFormControlFrame))) {
    *aInstancePtr = (void*) ((nsIFormControlFrame*) this);
    return NS_OK;
  } else  if (aIID.Equals(NS_GET_IID(nsIDOMMouseListener))) {
    *aInstancePtr = (void*)(nsIDOMMouseListener*) this;
    return NS_OK;
  }
  return nsHTMLContainerFrame::QueryInterface(aIID, aInstancePtr);
}

void 
nsFileControlFrame::SetFocus(PRBool aOn, PRBool aRepaint)
{
  // Fix for Bug 6133 
  if (mTextFrame) {
    nsIContent* content = mTextFrame->GetContent();
    if (content) {
      content->SetFocus(GetPresContext());
    }
  }
}

/**
 * This is called when our browse button is clicked
 */
nsresult 
nsFileControlFrame::MouseClick(nsIDOMEvent* aMouseEvent)
{
  // only allow the left button
  nsCOMPtr<nsIDOMMouseEvent> mouseEvent = do_QueryInterface(aMouseEvent);
  if (mouseEvent) {
    PRUint16 whichButton;
    if (NS_SUCCEEDED(mouseEvent->GetButton(&whichButton))) {
      if (whichButton != 0) {
        return NS_OK;
      }
    }
  }


  nsresult result;

  // Get parent nsIDOMWindowInternal object.
  nsIContent* content = GetContent();
  if (!content)
    return NS_ERROR_FAILURE;

  nsCOMPtr<nsIDocument> doc = content->GetDocument();
  if (!doc)
    return NS_ERROR_FAILURE;

  // Get Loc title
  nsXPIDLString title;
  nsContentUtils::GetLocalizedString(nsContentUtils::eFORMS_PROPERTIES,
                                     "FileUpload", title);

  nsCOMPtr<nsIFilePicker> filePicker = do_CreateInstance("@mozilla.org/filepicker;1");
  if (!filePicker)
    return NS_ERROR_FAILURE;

  result = filePicker->Init(doc->GetWindow(), title, nsIFilePicker::modeOpen);
  if (NS_FAILED(result))
    return result;

  // Set filter "All Files"
  filePicker->AppendFilters(nsIFilePicker::filterAll);

  // Set default directry and filename
  nsAutoString defaultName;
  GetFormProperty(nsHTMLAtoms::value, defaultName);

  nsCOMPtr<nsILocalFile> currentFile = do_CreateInstance("@mozilla.org/file/local;1");
  if (currentFile && !defaultName.IsEmpty()) {
    result = currentFile->InitWithPath(defaultName);
    if (NS_SUCCEEDED(result)) {
      nsAutoString leafName;
      currentFile->GetLeafName(leafName);
      if (!leafName.IsEmpty()) {
        filePicker->SetDefaultString(leafName);
      }

      // set directory
      nsCOMPtr<nsIFile> parentFile;
      currentFile->GetParent(getter_AddRefs(parentFile));
      if (parentFile) {
        nsCOMPtr<nsILocalFile> parentLocalFile = do_QueryInterface(parentFile, &result);
        if (parentLocalFile)
          filePicker->SetDisplayDirectory(parentLocalFile);
      }
    }
  }

  // Tell our textframe to remember the currently focused value
  mTextFrame->InitFocusedValue();

  // Open dialog
  PRInt16 mode;
  result = filePicker->Show(&mode);
  if (NS_FAILED(result))
    return result;
  if (mode == nsIFilePicker::returnCancel)
    return NS_OK;

  if (!mTextFrame) {
    // We got destroyed while the filepicker was up.  Don't do anything here.
    return NS_OK;
  }
  
  // Set property
  nsCOMPtr<nsILocalFile> localFile;
  result = filePicker->GetFile(getter_AddRefs(localFile));
  if (localFile) {
    nsAutoString unicodePath;
    result = localFile->GetPath(unicodePath);
    if (!unicodePath.IsEmpty()) {
      mTextFrame->SetFormProperty(nsHTMLAtoms::value, unicodePath);
      nsCOMPtr<nsIFileControlElement> fileControl = do_QueryInterface(mContent);
      if (fileControl) {
        fileControl->SetFileName(unicodePath);
      }
      
      // May need to fire an onchange here
      mTextFrame->CheckFireOnChange();
      return NS_OK;
    }
  }

  return NS_FAILED(result) ? result : NS_ERROR_FAILURE;
}


NS_IMETHODIMP nsFileControlFrame::Reflow(nsPresContext*          aPresContext, 
                                         nsHTMLReflowMetrics&     aDesiredSize,
                                         const nsHTMLReflowState& aReflowState, 
                                         nsReflowStatus&          aStatus)
{
  DO_GLOBAL_REFLOW_COUNT("nsFileControlFrame", aReflowState.reason);
  DISPLAY_REFLOW(aPresContext, this, aReflowState, aDesiredSize, aStatus);

  aStatus = NS_FRAME_COMPLETE;

  if (eReflowReason_Initial == aReflowState.reason) {
    mTextFrame = GetTextControlFrame(aPresContext, this);
    NS_ENSURE_TRUE(mTextFrame, NS_ERROR_UNEXPECTED);
    if (mCachedState) {
      mTextFrame->SetFormProperty(nsHTMLAtoms::value, *mCachedState);
      delete mCachedState;
      mCachedState = nsnull;
    }
  }

  // The Areaframe takes care of all our reflow 
  // except for when style is used to change its size.
  nsresult rv = nsAreaFrame::Reflow(aPresContext, aDesiredSize, aReflowState, aStatus);
  if (NS_SUCCEEDED(rv) && mTextFrame != nsnull) {
    const nsStyleVisibility* vis = GetStyleVisibility();

    nsIFrame* child = GetFirstChild(nsnull);
    if (child == mTextFrame) {
      child = child->GetNextSibling();
    }
    if (child) {
      nsRect buttonRect = child->GetRect();
      nsRect txtRect = mTextFrame->GetRect();

      // check to see if we must reflow just the area frame again 
      // in order for the text field to be the correct height
      // reflowing just the textfield (for some reason) 
      // messes up the button's rect
      if (txtRect.width + buttonRect.width != aDesiredSize.width ||
          txtRect.height != aDesiredSize.height) {
        nsHTMLReflowMetrics txtKidSize(PR_TRUE);
        nsSize txtAvailSize(aReflowState.availableWidth, aDesiredSize.height);
        nsHTMLReflowState   txtKidReflowState(aPresContext,
                                              *aReflowState.parentReflowState,
                                              this, txtAvailSize,
                                              eReflowReason_Resize);
        txtKidReflowState.mComputedHeight = aDesiredSize.height;
        rv = nsAreaFrame::WillReflow(aPresContext);
        NS_ASSERTION(NS_SUCCEEDED(rv), "Should have succeeded");
        rv = nsAreaFrame::Reflow(aPresContext, txtKidSize, txtKidReflowState, aStatus);
        NS_ASSERTION(NS_SUCCEEDED(rv), "Should have succeeded");
        rv = nsAreaFrame::DidReflow(aPresContext, &txtKidReflowState, aStatus);
        NS_ASSERTION(NS_SUCCEEDED(rv), "Should have succeeded");

        // If LTR then re-calc and set the correct rect
        if (NS_STYLE_DIRECTION_RTL != vis->mDirection) {
          // now adjust the frame positions
          txtRect.y      = aReflowState.mComputedBorderPadding.top;
          txtRect.height = aDesiredSize.height;
          mTextFrame->SetRect(txtRect);
        }
        if (aDesiredSize.mComputeMEW) {
           aDesiredSize.SetMEWToActualWidth(aReflowState.mStylePosition->mWidth.GetUnit());
        }
      }

      // Do RTL positioning
      // for some reason the areaframe does set the X coord of the rects correctly
      // so we must redo them here
      // and we must make sure the text field is the correct height
      if (NS_STYLE_DIRECTION_RTL == vis->mDirection) {
        buttonRect.x      = aReflowState.mComputedBorderPadding.left;
        child->SetRect(buttonRect);
        txtRect.x         = aDesiredSize.width - txtRect.width + aReflowState.mComputedBorderPadding.left;
        txtRect.y         = aReflowState.mComputedBorderPadding.top;
        txtRect.height    = aDesiredSize.height;
        mTextFrame->SetRect(txtRect);
      }

    }
  }
  NS_FRAME_SET_TRUNCATION(aStatus, aReflowState, aDesiredSize);
  return rv;
}

/*
NS_IMETHODIMP
nsFileControlFrame::SetInitialChildList(nsIAtom*        aListName,
                                        nsIFrame*       aChildList)
{
  nsAreaFrame::SetInitialChildList(aListName, aChildList);

  // given that the CSS frame constructor created all our frames. We need to find the text field
  // so we can get info from it.
  mTextFrame = GetTextControlFrame(this);
}
*/

nsNewFrame*
nsFileControlFrame::GetTextControlFrame(nsPresContext* aPresContext, nsIFrame* aStart)
{
  nsNewFrame* result = nsnull;
#ifndef DEBUG_NEWFRAME
  // find the text control frame.
  nsIFrame* childFrame = aStart->GetFirstChild(nsnull);

  while (childFrame) {
    // see if the child is a text control
    nsCOMPtr<nsIFormControl> formCtrl =
      do_QueryInterface(childFrame->GetContent());

    if (formCtrl && formCtrl->GetType() == NS_FORM_INPUT_TEXT) {
      result = (nsNewFrame*)childFrame;
    }

    // if not continue looking
    nsNewFrame* frame = GetTextControlFrame(aPresContext, childFrame);
    if (frame)
       result = frame;
     
    childFrame = childFrame->GetNextSibling();
  }

  return result;
#else
  return nsnull;
#endif
}

PRIntn
nsFileControlFrame::GetSkipSides() const
{
  return 0;
}

void
nsFileControlFrame::SyncAttr(PRInt32 aNameSpaceID, nsIAtom* aAttribute,
                             PRInt32 aWhichControls)
{
  nsAutoString value;
  if (mContent->GetAttr(aNameSpaceID, aAttribute, value)) {
    if (aWhichControls & SYNC_TEXT && mTextContent) {
      mTextContent->SetAttr(aNameSpaceID, aAttribute, value, PR_TRUE);
    }
    if (aWhichControls & SYNC_BUTTON && mBrowse) {
      mBrowse->SetAttr(aNameSpaceID, aAttribute, value, PR_TRUE);
    }
  } else {
    if (aWhichControls & SYNC_TEXT && mTextContent) {
      mTextContent->UnsetAttr(aNameSpaceID, aAttribute, PR_TRUE);
    }
    if (aWhichControls & SYNC_BUTTON && mBrowse) {
      mBrowse->UnsetAttr(aNameSpaceID, aAttribute, PR_TRUE);
    }
  }
}

NS_IMETHODIMP
nsFileControlFrame::AttributeChanged(PRInt32         aNameSpaceID,
                                     nsIAtom*        aAttribute,
                                     PRInt32         aModType)
{
  // propagate disabled to text / button inputs
  if (aNameSpaceID == kNameSpaceID_None &&
      aAttribute == nsHTMLAtoms::disabled) {
    SyncAttr(aNameSpaceID, aAttribute, SYNC_BOTH);
  // propagate size to text
  } else if (aNameSpaceID == kNameSpaceID_None &&
             aAttribute == nsHTMLAtoms::size) {
    SyncAttr(aNameSpaceID, aAttribute, SYNC_TEXT);
  }

  return nsAreaFrame::AttributeChanged(aNameSpaceID, aAttribute, aModType);
}

PRBool
nsFileControlFrame::IsLeaf() const
{
  return PR_TRUE;
}

#ifdef NS_DEBUG
NS_IMETHODIMP
nsFileControlFrame::GetFrameName(nsAString& aResult) const
{
  return MakeFrameName(NS_LITERAL_STRING("FileControl"), aResult);
}
#endif

nsresult
nsFileControlFrame::SetFormProperty(nsIAtom* aName,
                                    const nsAString& aValue)
{
  if (nsHTMLAtoms::value == aName) {
    if (mTextFrame) {
      mTextFrame->SetValue(aValue);
    } else {
      if (mCachedState) delete mCachedState;
      mCachedState = new nsString(aValue);
      NS_ENSURE_TRUE(mCachedState, NS_ERROR_OUT_OF_MEMORY);
    }
  }
  return NS_OK;
}      

nsresult
nsFileControlFrame::GetFormProperty(nsIAtom* aName, nsAString& aValue) const
{
  aValue.Truncate();  // initialize out param

  if (nsHTMLAtoms::value == aName) {
    NS_ASSERTION(!mCachedState || !mTextFrame,
                 "If we have a cached state, we better have no mTextFrame");
    if (mCachedState) {
      aValue.Assign(*mCachedState);
    } else if (mTextContent) {
      nsCOMPtr<nsIFileControlElement> fileControl = do_QueryInterface(mTextContent);
      if (fileControl) {
        fileControl->GetFileName(aValue);
      }
    }
  }
  return NS_OK;
}

NS_IMETHODIMP
nsFileControlFrame::BuildDisplayList(nsDisplayListBuilder*   aBuilder,
                                     const nsRect&           aDirtyRect,
                                     const nsDisplayListSet& aLists)
{
  // Our background is inherited to the text input, and we don't really want to
  // paint it or out padding and borders (which we never have anyway, per
  // styles in forms.css) -- doing it just makes us look ugly in some cases and
  // has no effect in others.
  nsDisplayListCollection tempList;
  nsresult rv = nsAreaFrame::BuildDisplayList(aBuilder, aDirtyRect, tempList);
  if (NS_FAILED(rv))
    return rv;

  tempList.BorderBackground()->DeleteAll();
  tempList.MoveTo(aLists);
  
  // Disabled file controls don't pass mouse events to their children, so we
  // put an invisible item in the display list above the children
  // just to catch events
  // REVIEW: I'm not sure why we do this, but that's what nsFileControlFrame::
  // GetFrameForPoint was doing
  if (mContent->HasAttr(kNameSpaceID_None, nsHTMLAtoms::disabled) && 
      IsVisibleForPainting(aBuilder)) {
    nsDisplayItem* item = new (aBuilder) nsDisplayEventReceiver(this);
    if (!item)
      return NS_ERROR_OUT_OF_MEMORY;
    aLists.Content()->AppendToTop(item);
  }

  return DisplaySelectionOverlay(aBuilder, aLists);
}
