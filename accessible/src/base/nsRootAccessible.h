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

#ifndef _nsRootAccessible_H_
#define _nsRootAccessible_H_

#include "nsDocAccessibleWrap.h"
#include "nsHashtable.h"
#include "nsIAccessibleDocument.h"
#include "nsCaretAccessible.h"
#include "nsIDocument.h"
#include "nsIDOMFocusListener.h"
#include "nsIDOMFormListener.h"
#include "nsIDOMXULListener.h"
#include "nsITimer.h"

#define NS_ROOTACCESSIBLE_IMPL_CID                      \
{  /* 7565f0d1-1465-4b71-906c-a623ac279f5d */           \
  0x7565f0d1,                                           \
  0x1465,                                               \
  0x4b71,                                               \
  { 0x90, 0x6c, 0xa6, 0x23, 0xac, 0x27, 0x9f, 0x5d }    \
}

const PRInt32 SCROLL_HASH_START_SIZE = 6;

class nsRootAccessible : public nsDocAccessibleWrap,
                         public nsIDOMEventListener
{
  NS_DECL_ISUPPORTS_INHERITED

  public:
    nsRootAccessible(nsIDOMNode *aDOMNode, nsIWeakReference* aShell);
    virtual ~nsRootAccessible();

    // nsIAccessible
    NS_IMETHOD GetName(nsAString& aName);
    NS_IMETHOD GetParent(nsIAccessible * *aParent);
    NS_IMETHOD GetRole(PRUint32 *aRole);
    NS_IMETHOD GetState(PRUint32 *aState, PRUint32 *aExtraState);
    NS_IMETHOD GetAccessibleRelated(PRUint32 aRelationType,
                                    nsIAccessible **aRelated);

    // ----- nsPIAccessibleDocument -----------------------
    NS_IMETHOD FireDocLoadEvents(PRUint32 aEventType);

    // ----- nsIDOMEventListener --------------------------
    NS_IMETHOD HandleEvent(nsIDOMEvent* aEvent);

    // nsIAccessNode
    NS_IMETHOD Init();
    NS_IMETHOD Shutdown();

    void ShutdownAll();
    
    NS_DECLARE_STATIC_IID_ACCESSOR(NS_ROOTACCESSIBLE_IMPL_CID)

    /**
      * Fire an accessible focus event for the current focusAccssible
      * and attach a new selection listener, if necessary.
      * @param aFocusAccessible  The accessible which has received focus.
      * @param aFocusNode        The DOM Node which has received focus.
      * @param aFocusEvent       DOM focus event that caused the node/accessible to receive focus
      * @param aForceEvent       Fire a focus event even if the last focused item was the same
      * @return                  Boolean -- was a focus event actually fired
      */
    PRBool FireAccessibleFocusEvent(nsIAccessible *aFocusAccessible,
                                    nsIDOMNode *aFocusNode,
                                    nsIDOMEvent *aFocusEvent,
                                    PRBool aForceEvent = PR_FALSE,
                                    PRBool aIsAsynch = PR_FALSE);

    nsCaretAccessible *GetCaretAccessible();

  private:
    nsCOMPtr<nsITimer> mFireFocusTimer;
    static void FireFocusCallback(nsITimer *aTimer, void *aClosure);
    
  protected:
    nsresult AddEventListeners();
    nsresult RemoveEventListeners();
    nsresult HandleEventWithTarget(nsIDOMEvent* aEvent,
                                   nsIDOMNode* aTargetNode);
    static void GetTargetNode(nsIDOMEvent *aEvent, nsIDOMNode **aTargetNode);
    void TryFireEarlyLoadEvent(nsIDOMNode *aDocNode);
    void FireCurrentFocusEvent();
    void GetChromeEventHandler(nsIDOMEventTarget **aChromeTarget);

    /**
     * Handles 'TreeRowCountChanged' event. Used in HandleEventWithTarget().
     */
    nsresult HandleTreeRowCountChangedEvent(nsIDOMEvent *aEvent,
                                            nsIAccessible *aAccessible,
                                            const nsAString& aTargetName);

    /**
     * Handles 'TreeInvalidated' event. Used in HandleEventWithTarget().
     */
    nsresult HandleTreeInvalidatedEvent(nsIDOMEvent *aEvent,
                                        nsIAccessible *aAccessible,
                                        const nsAString& aTargetName);

#ifdef MOZ_XUL
    PRUint32 GetChromeFlags();
#endif
    already_AddRefed<nsIDocShellTreeItem>
           GetContentDocShell(nsIDocShellTreeItem *aStart);
    nsRefPtr<nsCaretAccessible> mCaretAccessible;
    nsCOMPtr<nsIDOMNode> mCurrentARIAMenubar;
};

NS_DEFINE_STATIC_IID_ACCESSOR(nsRootAccessible, NS_ROOTACCESSIBLE_IMPL_CID)

#endif  
