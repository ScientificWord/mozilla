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

#ifndef nsXBLEventHandler_h__
#define nsXBLEventHandler_h__

#include "nsCOMPtr.h"
#include "nsIDOMEventListener.h"
#include "nsVoidArray.h"

class nsIAtom;
class nsIContent;
class nsIDOM3EventTarget;
class nsIDOMEventReceiver;
class nsXBLPrototypeHandler;

class nsXBLEventHandler : public nsIDOMEventListener
{
public:
  nsXBLEventHandler(nsXBLPrototypeHandler* aHandler);
  virtual ~nsXBLEventHandler();

  NS_DECL_ISUPPORTS

  NS_DECL_NSIDOMEVENTLISTENER

protected:
  nsXBLPrototypeHandler* mProtoHandler;

private:
  nsXBLEventHandler();
  virtual PRBool EventMatched(nsIDOMEvent* aEvent)
  {
    return PR_TRUE;
  }
};

class nsXBLMouseEventHandler : public nsXBLEventHandler
{
public:
  nsXBLMouseEventHandler(nsXBLPrototypeHandler* aHandler);
  virtual ~nsXBLMouseEventHandler();

private:
  PRBool EventMatched(nsIDOMEvent* aEvent);
};

class nsXBLKeyEventHandler : public nsIDOMEventListener
{
public:
  nsXBLKeyEventHandler(nsIAtom* aEventType, PRUint8 aPhase, PRUint8 aType);
  virtual ~nsXBLKeyEventHandler();

  NS_DECL_ISUPPORTS

  NS_DECL_NSIDOMEVENTLISTENER

  void AddProtoHandler(nsXBLPrototypeHandler* aProtoHandler)
  {
    mProtoHandlers.AppendElement(aProtoHandler);
  }

  PRBool Matches(nsIAtom* aEventType, PRUint8 aPhase, PRUint8 aType) const
  {
    return (mEventType == aEventType && mPhase == aPhase && mType == aType);
  }

  void GetEventName(nsAString& aString) const
  {
    mEventType->ToString(aString);
  }

  PRUint8 GetPhase() const
  {
    return mPhase;
  }

  PRUint8 GetType() const
  {
    return mType;
  }

private:
  nsXBLKeyEventHandler();

  nsVoidArray mProtoHandlers;
  nsCOMPtr<nsIAtom> mEventType;
  PRUint8 mPhase;
  PRUint8 mType;
};

nsresult
NS_NewXBLEventHandler(nsXBLPrototypeHandler* aHandler,
                      nsIAtom* aEventType,
                      nsXBLEventHandler** aResult);

nsresult
NS_NewXBLKeyEventHandler(nsIAtom* aEventType, PRUint8 aPhase,
                         PRUint8 aType, nsXBLKeyEventHandler** aResult);

#endif
