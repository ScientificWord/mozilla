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
#include "nsHTMLAreaAccessible.h"

#include "nsIDOMHTMLCollection.h"
#include "nsIDocument.h"
#include "nsIHTMLDocument.h"
#include "nsIImageLoadingContent.h"
#include "nsIPresShell.h"
#include "nsIServiceManager.h"
#include "nsIDOMHTMLImageElement.h"
#include "nsIDOMDocument.h"
#include "nsPIDOMWindow.h"

// --- image -----

const PRUint32 kDefaultImageCacheSize = 256;

nsHTMLImageAccessible::nsHTMLImageAccessible(nsIDOMNode* aDOMNode, nsIWeakReference* aShell):
nsLinkableAccessible(aDOMNode, aShell), mAccessNodeCache(nsnull)
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

  if (mMapElement) {
    mAccessNodeCache = new nsAccessNodeHashtable();
    mAccessNodeCache->Init(kDefaultImageCacheSize);
  }
}

NS_IMPL_ISUPPORTS_INHERITED1(nsHTMLImageAccessible, nsAccessible, nsIAccessibleImage)

NS_IMETHODIMP
nsHTMLImageAccessible::GetState(PRUint32 *aState, PRUint32 *aExtraState)
{
  // The state is a bitfield, get our inherited state, then logically OR it with
  // STATE_ANIMATED if this is an animated image.

  nsresult rv = nsLinkableAccessible::GetState(aState, aExtraState);
  NS_ENSURE_SUCCESS(rv, rv);
  if (!mDOMNode)
    return NS_OK;

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
      *aState |= nsIAccessibleStates::STATE_ANIMATED;
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
  *_retval = mMapElement ? nsIAccessibleRole::ROLE_IMAGE_MAP :
                           nsIAccessibleRole::ROLE_GRAPHIC;
  return NS_OK;
}


already_AddRefed<nsIAccessible>
nsHTMLImageAccessible::GetAreaAccessible(PRInt32 aAreaNum)
{
  if (!mMapElement)
    return nsnull;

  nsCOMPtr<nsIDOMHTMLCollection> mapAreas;
  mMapElement->GetAreas(getter_AddRefs(mapAreas));
  if (!mapAreas)
    return nsnull;

  nsCOMPtr<nsIDOMNode> domNode;
  mapAreas->Item(aAreaNum,getter_AddRefs(domNode));
  if (!domNode)
    return nsnull;

  nsCOMPtr<nsIAccessNode> accessNode;
  GetCacheEntry(*mAccessNodeCache, (void*)(aAreaNum),
                getter_AddRefs(accessNode));

  if (!accessNode) {
    accessNode = new nsHTMLAreaAccessible(domNode, this, mWeakShell);
    if (!accessNode)
      return nsnull;

    nsCOMPtr<nsPIAccessNode> privateAccessNode(do_QueryInterface(accessNode));
    NS_ASSERTION(privateAccessNode,
                 "Accessible doesn't implement nsPIAccessNode");

    nsresult rv = privateAccessNode->Init();
    if (NS_FAILED(rv))
      return nsnull;

    PutCacheEntry(*mAccessNodeCache, (void*)(aAreaNum), accessNode);
  }

  nsCOMPtr<nsIAccessible> accessible(do_QueryInterface(accessNode));
  nsIAccessible *accPtr;
  NS_IF_ADDREF(accPtr = accessible);
  return accPtr;
}


void nsHTMLImageAccessible::CacheChildren()
{
  if (!mWeakShell) {
    // This node has been shut down
    mAccChildCount = eChildCountUninitialized;
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
  PRInt32 childCount = 0;
  
  nsCOMPtr<nsIAccessible> areaAccessible;
  nsCOMPtr<nsPIAccessible> privatePrevAccessible;
  while (childCount < (PRInt32)numMapAreas && 
         (areaAccessible = GetAreaAccessible(childCount)) != nsnull) {
    if (privatePrevAccessible) {
      privatePrevAccessible->SetNextSibling(areaAccessible);
    }
    else {
      SetFirstChild(areaAccessible);
    }

    ++ childCount;

    privatePrevAccessible = do_QueryInterface(areaAccessible);
    NS_ASSERTION(privatePrevAccessible, "nsIAccessible impl's should always support nsPIAccessible as well");
    privatePrevAccessible->SetParent(this);
  }
  mAccChildCount = childCount;
}

NS_IMETHODIMP nsHTMLImageAccessible::DoAction(PRUint8 index)
{
  if (index == eAction_ShowLongDescription) {
    //get the long description uri and open in a new window
    nsCOMPtr<nsIDOMHTMLImageElement> element(do_QueryInterface(mDOMNode));
    NS_ENSURE_TRUE(element, NS_ERROR_FAILURE);
    nsAutoString longDesc;
    nsresult rv = element->GetLongDesc(longDesc);
    NS_ENSURE_SUCCESS(rv, rv);
    nsCOMPtr<nsIDOMDocument> domDocument;
    rv = mDOMNode->GetOwnerDocument(getter_AddRefs(domDocument));
    NS_ENSURE_SUCCESS(rv, rv);
    nsCOMPtr<nsIDocument> document(do_QueryInterface(domDocument));
    nsCOMPtr<nsPIDOMWindow> piWindow = document->GetWindow();
    nsCOMPtr<nsIDOMWindowInternal> win(do_QueryInterface(piWindow));
    NS_ENSURE_TRUE(win, NS_ERROR_FAILURE);
    nsCOMPtr<nsIDOMWindow> tmp;
    return win->Open(longDesc, NS_LITERAL_STRING(""), NS_LITERAL_STRING(""),
                     getter_AddRefs(tmp));
  }
  return nsLinkableAccessible::DoAction(index);
}

NS_IMETHODIMP
nsHTMLImageAccessible::GetImagePosition(PRUint32 aCoordType,
                                        PRInt32 *aX, PRInt32 *aY)
{
  PRInt32 width, height;
  nsresult rv = GetBounds(aX, aY, &width, &height);
  if (NS_FAILED(rv))
    return rv;

  return nsAccUtils::ConvertScreenCoordsTo(aX, aY, aCoordType, this);
}

NS_IMETHODIMP
nsHTMLImageAccessible::GetImageSize(PRInt32 *aWidth, PRInt32 *aHeight)
{
  PRInt32 x, y;
  return GetBounds(&x, &y, aWidth, aHeight);
}

NS_IMETHODIMP
nsHTMLImageAccessible::Shutdown()
{
  nsLinkableAccessible::Shutdown();

  if (mAccessNodeCache) {
    ClearCache(*mAccessNodeCache);
    delete mAccessNodeCache;
    mAccessNodeCache = nsnull;
  }

  return NS_OK;
}

