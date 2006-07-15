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
 *   Author: Aaron Leventhal (aaronl@netscape.com)
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

#include "imgIContainer.h"
#include "imgIRequest.h"
#include "nsHTMLImageAccessible.h"
#include "nsAccessibilityAtoms.h"
#include "nsIAccessibilityService.h"
#include "nsIDOMHTMLCollection.h"
#include "nsIDocument.h"
#include "nsIHTMLDocument.h"
#include "nsIImageLoadingContent.h"
#include "nsIPresShell.h"
#include "nsIServiceManager.h"

// --- image -----

nsHTMLImageAccessible::nsHTMLImageAccessible(nsIDOMNode* aDOMNode, nsIWeakReference* aShell):
nsLinkableAccessible(aDOMNode, aShell)
{ 
  nsCOMPtr<nsIDOMElement> element(do_QueryInterface(aDOMNode));
  nsCOMPtr<nsIPresShell> shell(do_QueryReferent(mWeakShell));
  if (!shell)
    return;

  nsIDocument *doc = shell->GetDocument();
  nsAutoString mapElementName;

  if (doc && element) {
    nsCOMPtr<nsIHTMLDocument> htmlDoc(do_QueryInterface(doc));
    element->GetAttribute(NS_LITERAL_STRING("usemap"),mapElementName);
    if (htmlDoc && !mapElementName.IsEmpty()) {
      if (mapElementName.CharAt(0) == '#')
        mapElementName.Cut(0,1);
      mMapElement = htmlDoc->GetImageMap(mapElementName);
    }
  }
}

NS_IMETHODIMP nsHTMLImageAccessible::GetState(PRUint32 *_retval)
{
  // The state is a bitfield, get our inherited state, then logically OR it with STATE_ANIMATED if this
  // is an animated image.

  nsLinkableAccessible::GetState(_retval);

  nsCOMPtr<nsIImageLoadingContent> content(do_QueryInterface(mDOMNode));
  nsCOMPtr<imgIRequest> imageRequest;

  if (content) 
    content->GetRequest(nsIImageLoadingContent::CURRENT_REQUEST,
                        getter_AddRefs(imageRequest));
  
  nsCOMPtr<imgIContainer> imgContainer;
  if (imageRequest) 
    imageRequest->GetImage(getter_AddRefs(imgContainer));

  if (imgContainer) {
    PRUint32 numFrames;
    imgContainer->GetNumFrames(&numFrames);
    if (numFrames > 1)
      *_retval |= STATE_ANIMATED;
  }

  return NS_OK;
}


/* wstring getName (); */
NS_IMETHODIMP nsHTMLImageAccessible::GetName(nsAString& aName)
{
  nsCOMPtr<nsIContent> content(do_QueryInterface(mDOMNode));
  if (!content) {
    return NS_ERROR_FAILURE;  // Node has been shut down
  }

  if (!content->GetAttr(kNameSpaceID_None, nsAccessibilityAtoms::alt,
                        aName)) {
    if (mRoleMapEntry) {
      // Use HTML label or DHTML accessibility's labelledby attribute for name
      // GetHTMLName will also try title attribute as a last resort
      return GetHTMLName(aName, PR_FALSE);
    }
    if (!content->GetAttr(kNameSpaceID_None, nsAccessibilityAtoms::title,
                          aName)) {
      aName.SetIsVoid(PR_TRUE); // No alt or title
    }
  }

  return NS_OK;
}

/* wstring getRole (); */
NS_IMETHODIMP nsHTMLImageAccessible::GetRole(PRUint32 *_retval)
{
  *_retval = ROLE_GRAPHIC;
  return NS_OK;
}


already_AddRefed<nsIAccessible> nsHTMLImageAccessible::CreateAreaAccessible(PRInt32 areaNum)
{
  if (!mMapElement) 
    return nsnull;

  nsCOMPtr<nsIDOMHTMLCollection> mapAreas;
  mMapElement->GetAreas(getter_AddRefs(mapAreas));
  if (!mapAreas) 
    return nsnull;

  nsCOMPtr<nsIDOMNode> domNode;
  mapAreas->Item(areaNum,getter_AddRefs(domNode));
  if (!domNode)
    return nsnull;

  nsCOMPtr<nsIAccessibilityService> accService(do_GetService("@mozilla.org/accessibilityService;1"));
  if (!accService)
    return nsnull;
  if (accService) {
    nsIAccessible* acc = nsnull;
    accService->GetCachedAccessible(domNode, mWeakShell, &acc);
    if (!acc) {
      accService->CreateHTMLAreaAccessible(mWeakShell, domNode, this, &acc);
      nsCOMPtr<nsPIAccessNode> accessNode(do_QueryInterface(acc));
      if (accessNode) {
        accessNode->Init();
      }
    }
    return acc;
  }
  return nsnull;
}


void nsHTMLImageAccessible::CacheChildren(PRBool aWalkAnonContent)
{
  if (!mWeakShell) {
    // This node has been shut down
    mAccChildCount = -1;
    return;
  }

  if (mAccChildCount != eChildCountUninitialized) {
    return;
  }

  mAccChildCount = 0;
  nsCOMPtr<nsIDOMHTMLCollection> mapAreas;
  if (mMapElement) {
    mMapElement->GetAreas(getter_AddRefs(mapAreas));
  }
  if (!mapAreas) {
    return;
  }

  PRUint32 numMapAreas;
  mapAreas->GetLength(&numMapAreas);

  nsCOMPtr<nsIAccessible> areaAccessible;
  nsCOMPtr<nsPIAccessible> privatePrevAccessible;
  while (mAccChildCount < (PRInt32)numMapAreas && 
         (areaAccessible = CreateAreaAccessible(mAccChildCount)) != nsnull) {
    if (privatePrevAccessible) {
      privatePrevAccessible->SetNextSibling(areaAccessible);
    }
    else {
      SetFirstChild(areaAccessible);
    }

    ++mAccChildCount;

    privatePrevAccessible = do_QueryInterface(areaAccessible);
    NS_ASSERTION(privatePrevAccessible, "nsIAccessible impl's should always support nsPIAccessible as well");
    privatePrevAccessible->SetParent(this);
  }
}

