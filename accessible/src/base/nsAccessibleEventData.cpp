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
 * Portions created by the Initial Developer are Copyright (C) 2003
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Original Author: Aaron Leventhal (aaronl@netscape.com)
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

#include "nsAccessibleEventData.h"
#include "nsIAccessibilityService.h"
#include "nsIAccessNode.h"
#include "nsIServiceManager.h"

NS_IMPL_ISUPPORTS1(nsAccessibleEventData, nsIAccessibleEvent)

nsAccessibleEventData::nsAccessibleEventData(PRUint32 aEventType, nsIAccessible *aAccessible, 
                                             nsIAccessibleDocument *aDocAccessible, 
                                             void *aEventData):
  mEventType(aEventType), mAccessible(aAccessible), mDocAccessible(aDocAccessible), 
  mEventData(aEventData)
{
}

nsAccessibleEventData::nsAccessibleEventData(PRUint32 aEventType, nsIDOMNode *aDOMNode,
                                             nsIAccessibleDocument *aDocAccessible, 
                                             void *aEventData):
  mEventType(aEventType), mDOMNode(aDOMNode), mDocAccessible(aDocAccessible),
  mEventData(aEventData)
{
}

NS_IMETHODIMP nsAccessibleEventData::GetAccessible(nsIAccessible **aAccessible) 
{
  *aAccessible = nsnull;
  if (!mAccessible) {
    NS_ENSURE_TRUE(mDOMNode, NS_ERROR_FAILURE);
    nsCOMPtr<nsIAccessibilityService> accService = 
      do_GetService("@mozilla.org/accessibilityService;1");
    NS_ENSURE_TRUE(accService, NS_ERROR_FAILURE);
    accService->GetAccessibleFor(mDOMNode, getter_AddRefs(mAccessible));
    if (!mAccessible) {
      return NS_OK;
    }
  }
  NS_ADDREF(*aAccessible = mAccessible);
  return NS_OK;
}

NS_IMETHODIMP nsAccessibleEventData::GetDOMNode(nsIDOMNode **aDOMNode)
{
  if (!mDOMNode) {
    nsCOMPtr<nsIAccessNode> accessNode(do_QueryInterface(mAccessible));
    NS_ENSURE_TRUE(accessNode, NS_ERROR_FAILURE);
    accessNode->GetDOMNode(getter_AddRefs(mDOMNode));
  }
  NS_ADDREF(*aDOMNode = mDOMNode);
  return NS_OK;
}
