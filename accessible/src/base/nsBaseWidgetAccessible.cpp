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
 *   John Gaunt (jgaunt@netscape.com)
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

#include "nsBaseWidgetAccessible.h"
#include "nsAccessibilityAtoms.h"
#include "nsIAccessibilityService.h"
#include "nsIAccessibleDocument.h"
#include "nsAccessibleWrap.h"
#include "nsAccessibilityUtils.h"
#include "nsIDOMNSHTMLElement.h"
#include "nsGUIEvent.h"
#include "nsHyperTextAccessibleWrap.h"
#include "nsILink.h"
#include "nsIFrame.h"
#include "nsINameSpaceManager.h"
#include "nsPresContext.h"
#include "nsIPresShell.h"
#include "nsIServiceManager.h"
#include "nsIURI.h"

//-------------
// nsLeafAccessible
//-------------

nsLeafAccessible::nsLeafAccessible(nsIDOMNode* aNode, nsIWeakReference* aShell):
nsAccessibleWrap(aNode, aShell)
{
}

NS_IMPL_ISUPPORTS_INHERITED0(nsLeafAccessible, nsAccessible)

/* nsIAccessible getFirstChild (); */
NS_IMETHODIMP nsLeafAccessible::GetFirstChild(nsIAccessible **_retval)
{
  *_retval = nsnull;
  return NS_OK;
}

/* nsIAccessible getLastChild (); */
NS_IMETHODIMP nsLeafAccessible::GetLastChild(nsIAccessible **_retval)
{
  *_retval = nsnull;
  return NS_OK;
}

/* long getAccChildCount (); */
NS_IMETHODIMP nsLeafAccessible::GetChildCount(PRInt32 *_retval)
{
  *_retval = 0;
  return NS_OK;
}

/* readonly attribute boolean allowsAnonChildAccessibles; */
NS_IMETHODIMP
nsLeafAccessible::GetAllowsAnonChildAccessibles(PRBool *aAllowsAnonChildren)
{
  *aAllowsAnonChildren = PR_FALSE;
  return NS_OK;
}

//----------------
// nsLinkableAccessible
//----------------

nsLinkableAccessible::nsLinkableAccessible(nsIDOMNode* aNode, nsIWeakReference* aShell) :
  nsHyperTextAccessibleWrap(aNode, aShell),
  mActionContent(nsnull),
  mIsLink(PR_FALSE),
  mIsOnclick(PR_FALSE)
{
}

NS_IMPL_ISUPPORTS_INHERITED0(nsLinkableAccessible, nsHyperTextAccessibleWrap)

NS_IMETHODIMP nsLinkableAccessible::TakeFocus()
{ 
  if (mActionContent && mActionContent->IsFocusable()) {
    nsCOMPtr<nsIDOMNSHTMLElement> htmlElement(do_QueryInterface(mActionContent));
    if (htmlElement) {
      // HTML Elements also set the caret position
      // in order to affect tabbing order
      return htmlElement->Focus();
    }
    NS_WARNING("Has action content that is not an HTML element");
  }
  
  return NS_OK;
}

/* long GetState (); */
NS_IMETHODIMP
nsLinkableAccessible::GetState(PRUint32 *aState, PRUint32 *aExtraState)
{
  nsresult rv = nsHyperTextAccessibleWrap::GetState(aState, aExtraState);
  NS_ENSURE_SUCCESS(rv, rv);
  if (!mDOMNode)
    return NS_OK;

  if (mIsLink) {
    *aState |= nsIAccessibleStates::STATE_LINKED;
    nsCOMPtr<nsILink> link = do_QueryInterface(mActionContent);
    if (link) {
      nsLinkState linkState;
      link->GetLinkState(linkState);
      if (linkState == eLinkState_Visited) {
        *aState |= nsIAccessibleStates::STATE_TRAVERSED;
      }
    }
  }

  return NS_OK;
}

NS_IMETHODIMP nsLinkableAccessible::GetValue(nsAString& aValue)
{
  aValue.Truncate();
  nsHyperTextAccessible::GetValue(aValue);
  if (!aValue.IsEmpty())
    return NS_OK;

  if (mIsLink) {
    nsCOMPtr<nsIDOMNode> linkNode(do_QueryInterface(mActionContent));
    nsCOMPtr<nsIPresShell> presShell(do_QueryReferent(mWeakShell));
    if (linkNode && presShell)
      return presShell->GetLinkLocation(linkNode, aValue);
  }
  return NS_ERROR_NOT_IMPLEMENTED;
}


/* PRUint8 getAccNumActions (); */
NS_IMETHODIMP nsLinkableAccessible::GetNumActions(PRUint8 *aNumActions)
{
  *aNumActions = mActionContent ? 1 : 0;
  return NS_OK;
}

/* nsAString GetActionName (in PRUint8 Aindex); */
NS_IMETHODIMP nsLinkableAccessible::GetActionName(PRUint8 aIndex, nsAString& aName)
{
  // Action 0 (default action): Jump to link
  aName.Truncate();
  if (aIndex == eAction_Jump) {   
    if (mIsLink) {
      aName.AssignLiteral("jump");
      return NS_OK;
    }
    else if (mIsOnclick) {
      aName.AssignLiteral("click");
      return NS_OK;
    }
    return NS_ERROR_NOT_IMPLEMENTED;
  }
  return NS_ERROR_INVALID_ARG;
}

/* void accDoAction (in PRUint8 index); */
NS_IMETHODIMP nsLinkableAccessible::DoAction(PRUint8 index)
{
  // Action 0 (default action): Jump to link
  if (index == eAction_Jump) {
    if (mActionContent) {
      return DoCommand(mActionContent);
    }
  }
  return NS_ERROR_INVALID_ARG;
}

NS_IMETHODIMP nsLinkableAccessible::GetKeyboardShortcut(nsAString& aKeyboardShortcut)
{
  if (mActionContent) {
    nsCOMPtr<nsIDOMNode> actionNode(do_QueryInterface(mActionContent));
    if (actionNode && mDOMNode != actionNode) {
      nsCOMPtr<nsIAccessible> accessible;
      nsCOMPtr<nsIAccessibilityService> accService = 
        do_GetService("@mozilla.org/accessibilityService;1");
      accService->GetAccessibleInWeakShell(actionNode, mWeakShell,
                                           getter_AddRefs(accessible));
      if (accessible) {
        accessible->GetKeyboardShortcut(aKeyboardShortcut);
      }
      return NS_OK;
    }
  }
  return nsAccessible::GetKeyboardShortcut(aKeyboardShortcut);
}

void nsLinkableAccessible::CacheActionContent()
{
  for (nsCOMPtr<nsIContent> walkUpContent(do_QueryInterface(mDOMNode));
       walkUpContent;
       walkUpContent = walkUpContent->GetParent()) {
    PRBool isOnclick = nsAccUtils::HasListener(walkUpContent, NS_LITERAL_STRING("click"));
    nsIAtom *tag = walkUpContent->Tag();
    if ((tag == nsAccessibilityAtoms::a || tag == nsAccessibilityAtoms::area) &&
        walkUpContent->IsNodeOfType(nsINode::eHTML)) {
      nsCOMPtr<nsILink> link = do_QueryInterface(walkUpContent);
      if (link) {
        // Currently we do not expose <link> tags, because they are not typically
        // in <body> and rendered.
        // We do not yet support xlinks
        nsCOMPtr<nsIURI> uri;
        link->GetHrefURI(getter_AddRefs(uri));
        if (uri) {
          mActionContent = walkUpContent;
          mIsLink = PR_TRUE;
          break;
        }
      }
      if (SameCOMIdentity(mDOMNode, walkUpContent)) {
        // This is the element that caused the creation of a linkable accessible
        // Don't let it keep walking up, otherwise we may report the wrong container
        // as the action node
        mActionContent = walkUpContent;
        mIsOnclick = isOnclick;
        break;
      }
    }
    if (isOnclick) {
      mActionContent = walkUpContent;
      mIsOnclick = PR_TRUE;
      break;
    }
  }
}

// nsIAccessibleHyperLink::GetURI()
NS_IMETHODIMP nsLinkableAccessible::GetURI(PRInt32 aIndex, nsIURI **aURI)
{
  // XXX Also implement this for nsHTMLImageAccessible file names
  *aURI = nsnull;
  if (aIndex != 0 || !mIsLink || !SameCOMIdentity(mDOMNode, mActionContent)) {
    return NS_ERROR_FAILURE;
  }

  nsCOMPtr<nsILink> link(do_QueryInterface(mActionContent));
  if (link) {
    return link->GetHrefURI(aURI);
  }

  return NS_ERROR_FAILURE;
}

NS_IMETHODIMP nsLinkableAccessible::Init()
{
  CacheActionContent();
  return nsHyperTextAccessibleWrap::Init();
}

NS_IMETHODIMP nsLinkableAccessible::Shutdown()
{
  mActionContent = nsnull;
  return nsHyperTextAccessibleWrap::Shutdown();
}

//---------------------
// nsEnumRoleAccessible
//---------------------

nsEnumRoleAccessible::nsEnumRoleAccessible(nsIDOMNode* aNode, nsIWeakReference* aShell, PRUint32 aRole) :
  nsAccessibleWrap(aNode, aShell),
  mRole(aRole)
{
}

NS_IMPL_ISUPPORTS_INHERITED0(nsEnumRoleAccessible, nsAccessible)
