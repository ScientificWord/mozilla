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
 *   Original Author: David W. Hyatt (hyatt@netscape.com)
 *   - Mike Pinkerton (pinkerton@netscape.com)
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

#ifndef nsXBLWindowKeyHandler_h__
#define nsXBLWindowKeyHandler_h__

#include "nsWeakPtr.h"
#include "nsIDOMKeyListener.h"

class nsIAtom;
class nsIDOMElement;
class nsIDOMEventTarget;
class nsPIDOMEventTarget;
class nsIXBLDocumentInfo;
class nsXBLSpecialDocInfo;
class nsXBLPrototypeHandler;

class nsXBLWindowKeyHandler : public nsIDOMKeyListener
{
public:
  nsXBLWindowKeyHandler(nsIDOMElement* aElement, nsPIDOMEventTarget* aTarget);
  virtual ~nsXBLWindowKeyHandler();
  
  // nsIDOMetc.
  NS_IMETHOD HandleEvent(nsIDOMEvent* aEvent)
  {
    return NS_OK;
  }

  NS_IMETHOD KeyUp(nsIDOMEvent* aKeyEvent);
  NS_IMETHOD KeyDown(nsIDOMEvent* aKeyEvent);
  NS_IMETHOD KeyPress(nsIDOMEvent* aKeyEvent);
   
  NS_DECL_ISUPPORTS

  // release globals
  static NS_HIDDEN_(void) ShutDown();

protected:
  nsresult WalkHandlers(nsIDOMEvent* aKeyEvent, nsIAtom* aEventType);

  // walk the handlers, looking for one to handle the event
  nsresult WalkHandlersInternal(nsIDOMEvent* aKeyEvent,
                                nsIAtom* aEventType, 
                                nsXBLPrototypeHandler* aHandler);

  // lazily load the handlers. Overridden to handle being attached
  // to a particular element rather than the document
  nsresult EnsureHandlers(PRBool *aIsEditor);

  // check if the given handler cares about the given key event
  PRBool EventMatched(nsXBLPrototypeHandler* inHandler, nsIAtom* inEventType,
                      nsIDOMEvent* inEvent);

  // are we working with editor or browser?
  PRBool IsEditor() ;

  // Returns the element which was passed as a parameter to the constructor,
  // unless the element has been removed from the document.
  already_AddRefed<nsIDOMElement> GetElement();
  // Using weak pointer to the DOM Element.
  nsWeakPtr              mWeakPtrForElement;
  nsPIDOMEventTarget*    mTarget; // weak ref

  // these are not owning references; the prototype handlers are owned
  // by the prototype bindings which are owned by the docinfo.
  nsXBLPrototypeHandler* mHandler;     // platform bindings
  nsXBLPrototypeHandler* mUserHandler; // user-specific bindings

  // holds document info about bindings
  static nsXBLSpecialDocInfo* sXBLSpecialDocInfo;
  static PRUint32 sRefCnt;
};

nsresult
NS_NewXBLWindowKeyHandler(nsIDOMElement* aElement,
                          nsPIDOMEventTarget* aTarget,
                          nsXBLWindowKeyHandler** aResult);

#endif
