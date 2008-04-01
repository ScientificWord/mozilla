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
 *   Original Author: David W. Hyatt (hyatt@netscape.com)
 *   Dean Tessman <dean_tessman@hotmail.com>
 *   Pierre Phaneuf <pp@ludusdesign.com>
 *   Robert O'Callahan <roc+moz@cs.cmu.edu>
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

/**
 * This is the popup listener implementation for popup menus and context menus.
 */

#ifndef nsXULPopupListener_h___
#define nsXULPopupListener_h___

#include "nsCOMPtr.h"

#include "nsIContent.h"
#include "nsIDOMElement.h"
#include "nsIDOMMouseEvent.h"
#include "nsIFrame.h"
#include "nsIDOMMouseListener.h"
#include "nsIDOMContextMenuListener.h"
#include "nsCycleCollectionParticipant.h"

class nsXULPopupListener : public nsIDOMMouseListener,
                           public nsIDOMContextMenuListener
{
public:
    // aElement is the element that the popup is attached to. If aIsContext is
    // false, the popup opens on left click on aElement or a descendant. If
    // aIsContext is true, the popup is a context menu which opens on a
    // context menu event.
    nsXULPopupListener(nsIDOMElement *aElement, PRBool aIsContext);
    virtual ~nsXULPopupListener(void);

    // nsISupports
    NS_DECL_CYCLE_COLLECTING_ISUPPORTS
    NS_DECL_CYCLE_COLLECTION_CLASS_AMBIGUOUS(nsXULPopupListener,
                                             nsIDOMMouseListener)

    // nsIDOMMouseListener
    NS_IMETHOD MouseDown(nsIDOMEvent* aMouseEvent);
    NS_IMETHOD MouseUp(nsIDOMEvent* aMouseEvent) { return NS_OK; }
    NS_IMETHOD MouseClick(nsIDOMEvent* aMouseEvent) { return NS_OK; }
    NS_IMETHOD MouseDblClick(nsIDOMEvent* aMouseEvent) { return NS_OK; }
    NS_IMETHOD MouseOver(nsIDOMEvent* aMouseEvent) { return NS_OK; }
    NS_IMETHOD MouseOut(nsIDOMEvent* aMouseEvent) { return NS_OK; }

    // nsIDOMContextMenuListener
    NS_IMETHOD ContextMenu(nsIDOMEvent* aContextMenuEvent);

    // nsIDOMEventListener
    NS_IMETHOD HandleEvent(nsIDOMEvent* anEvent) { return NS_OK; }

protected:

    // open the popup. aEvent is the event that triggered the popup such as
    // a mouse click and aTargetContent is the target of this event.
    virtual nsresult LaunchPopup(nsIDOMEvent* aEvent, nsIContent* aTargetContent);

    // close the popup when the listener goes away
    virtual void ClosePopup();

private:

    // PreLaunchPopup is called before LaunchPopup to ensure that the event is
    // suitable and to initialize the XUL document's popupNode to the event
    // target.
    nsresult PreLaunchPopup(nsIDOMEvent* aMouseEvent);

    // When a context menu is opened, focus the target of the contextmenu event.
    nsresult FireFocusOnTargetContent(nsIDOMNode* aTargetNode);

    // |mElement| is the node to which this listener is attached.
    nsCOMPtr<nsIDOMElement> mElement;

    // The popup that is getting shown on top of mElement.
    nsCOMPtr<nsIContent> mPopupContent; 

    // true if a context popup
    PRBool mIsContext;
};

// Construct a new nsXULPopupListener and return in aListener. See the
// nsXULPopupListener constructor for details about the aElement and
// aIsContext arguments.
nsresult
NS_NewXULPopupListener(nsIDOMElement* aElement, PRBool aIsContext,
                       nsIDOMEventListener** aListener);

#endif // nsXULPopupListener_h___
