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
 *   David W. Hyatt <hyatt@netscape.com>
 *   Ben Goodger <ben@netscape.com>
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
#include "nsIPopupBoxObject.h"
#include "nsIRootBox.h"
#include "nsBoxObject.h"
#include "nsIPresShell.h"
#include "nsFrameManager.h"
#include "nsIContent.h"
#include "nsIDocument.h"
#include "nsIDOMDocument.h"
#include "nsIDOMElement.h"
#include "nsIFrame.h"
#include "nsINameSpaceManager.h"
#include "nsGkAtoms.h"
#include "nsMenuPopupFrame.h"


class nsPopupBoxObject : public nsBoxObject,
                         public nsIPopupBoxObject
{
public:
  NS_DECL_ISUPPORTS_INHERITED
  NS_DECL_NSIPOPUPBOXOBJECT

  nsPopupBoxObject() {}
protected:
  virtual ~nsPopupBoxObject() {}

  nsPopupSetFrame* GetPopupSetFrame();
  nsMenuPopupFrame* GetMenuPopupFrame()
  {
    nsIFrame* frame = GetFrame(PR_FALSE);
    if (frame && frame->GetType() == nsGkAtoms::menuPopupFrame)
      return static_cast<nsMenuPopupFrame*>(frame);
    return nsnull;
  }
};

NS_IMPL_ISUPPORTS_INHERITED1(nsPopupBoxObject, nsBoxObject, nsIPopupBoxObject)

nsPopupSetFrame*
nsPopupBoxObject::GetPopupSetFrame()
{
  nsIRootBox* rootBox = nsIRootBox::GetRootBox(GetPresShell(PR_FALSE));
  if (!rootBox)
    return nsnull;

  return rootBox->GetPopupSetFrame();
}

NS_IMETHODIMP
nsPopupBoxObject::HidePopup()
{
  nsXULPopupManager* pm = nsXULPopupManager::GetInstance();
  if (pm)
    pm->HidePopup(mContent, PR_FALSE, PR_TRUE, PR_FALSE);

  return NS_OK;
}

NS_IMETHODIMP
nsPopupBoxObject::ShowPopup(nsIDOMElement* aAnchorElement,
                            nsIDOMElement* aPopupElement,
                            PRInt32 aXPos, PRInt32 aYPos,
                            const PRUnichar *aPopupType,
                            const PRUnichar *aAnchorAlignment,
                            const PRUnichar *aPopupAlignment)
{
  NS_ENSURE_TRUE(aPopupElement, NS_ERROR_INVALID_ARG);
  // srcContent can be null.

  nsXULPopupManager* pm = nsXULPopupManager::GetInstance();
  if (pm) {
    nsCOMPtr<nsIContent> anchorContent(do_QueryInterface(aAnchorElement));
    nsAutoString popupType(aPopupType);
    nsAutoString anchor(aAnchorAlignment);
    nsAutoString align(aPopupAlignment);
    pm->ShowPopupWithAnchorAlign(mContent, anchorContent, anchor, align,
                                 aXPos, aYPos, popupType.EqualsLiteral("context"));
  }

  return NS_OK;
}

NS_IMETHODIMP
nsPopupBoxObject::OpenPopup(nsIDOMElement* aAnchorElement,
                            const nsAString& aPosition,
                            PRInt32 aXPos, PRInt32 aYPos,
                            PRBool aIsContextMenu,
                            PRBool aAttributesOverride)
{
  nsXULPopupManager* pm = nsXULPopupManager::GetInstance();
  if (pm) {
    nsCOMPtr<nsIContent> anchorContent(do_QueryInterface(aAnchorElement));
    pm->ShowPopup(mContent, anchorContent, aPosition, aXPos, aYPos,
                  aIsContextMenu, aAttributesOverride, PR_FALSE, nsnull);
  }

  return NS_OK;
}

NS_IMETHODIMP
nsPopupBoxObject::OpenPopupAtScreen(PRInt32 aXPos, PRInt32 aYPos, PRBool aIsContextMenu)
{
  nsXULPopupManager* pm = nsXULPopupManager::GetInstance();
  if (pm)
    pm->ShowPopupAtScreen(mContent, aXPos, aYPos, aIsContextMenu, nsnull);
  return NS_OK;
}

NS_IMETHODIMP
nsPopupBoxObject::MoveTo(PRInt32 aLeft, PRInt32 aTop)
{
  nsMenuPopupFrame *menuPopupFrame = GetMenuPopupFrame();
  if (menuPopupFrame) {
    menuPopupFrame->MoveTo(aLeft, aTop);
  }

  return NS_OK;
}

NS_IMETHODIMP
nsPopupBoxObject::SizeTo(PRInt32 aWidth, PRInt32 aHeight)
{
  nsAutoString width, height;
  width.AppendInt(aWidth);
  height.AppendInt(aHeight);

  nsCOMPtr<nsIContent> content = mContent;
  content->SetAttr(kNameSpaceID_None, nsGkAtoms::width, width, PR_FALSE);
  content->SetAttr(kNameSpaceID_None, nsGkAtoms::height, height, PR_TRUE);

  return NS_OK;
}

NS_IMETHODIMP
nsPopupBoxObject::GetAutoPosition(PRBool* aShouldAutoPosition)
{
  nsMenuPopupFrame *menuPopupFrame = GetMenuPopupFrame();
  if (menuPopupFrame) {
    *aShouldAutoPosition = menuPopupFrame->GetAutoPosition();
  }

  return NS_OK;
}

NS_IMETHODIMP
nsPopupBoxObject::SetAutoPosition(PRBool aShouldAutoPosition)
{
  nsMenuPopupFrame *menuPopupFrame = GetMenuPopupFrame();
  if (menuPopupFrame) {
    menuPopupFrame->SetAutoPosition(aShouldAutoPosition);
  }

  return NS_OK;
}

NS_IMETHODIMP
nsPopupBoxObject::EnableRollup(PRBool aShouldRollup)
{
  // this does nothing nows
  return NS_OK;
}

NS_IMETHODIMP
nsPopupBoxObject::SetConsumeRollupEvent(PRUint32 aConsume)
{
  nsMenuPopupFrame *menuPopupFrame = GetMenuPopupFrame();
  if (menuPopupFrame) {
    menuPopupFrame->SetConsumeRollupEvent(aConsume);
  }

  return NS_OK;
}

NS_IMETHODIMP
nsPopupBoxObject::EnableKeyboardNavigator(PRBool aEnableKeyboardNavigator)
{
  // Use ignorekeys="true" on the popup instead of using this function.
  if (aEnableKeyboardNavigator)
    mContent->UnsetAttr(kNameSpaceID_None, nsGkAtoms::ignorekeys, PR_TRUE);
  else
    mContent->SetAttr(kNameSpaceID_None, nsGkAtoms::ignorekeys,
                      NS_LITERAL_STRING("true"), PR_TRUE);

  return NS_OK;
}

NS_IMETHODIMP
nsPopupBoxObject::GetPopupState(nsAString& aState)
{
  aState.AssignLiteral("closed");

  nsMenuPopupFrame *menuPopupFrame = GetMenuPopupFrame();
  if (menuPopupFrame) {
    switch (menuPopupFrame->PopupState()) {
      case ePopupShowing:
      case ePopupOpen:
        aState.AssignLiteral("showing");
        break;
      case ePopupOpenAndVisible:
        aState.AssignLiteral("open");
        break;
      case ePopupHiding:
      case ePopupInvisible:
        aState.AssignLiteral("hiding");
        break;
    }
  }

  return NS_OK;
}


// Creation Routine ///////////////////////////////////////////////////////////////////////

nsresult
NS_NewPopupBoxObject(nsIBoxObject** aResult)
{
  *aResult = new nsPopupBoxObject;
  if (!*aResult)
    return NS_ERROR_OUT_OF_MEMORY;
  NS_ADDREF(*aResult);
  return NS_OK;
}
