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

#ifndef nsIDOMEventReceiver_h__
#define nsIDOMEventReceiver_h__

#include "nsIDOMEventTarget.h"

class nsIDOMEventListener;
class nsIEventListenerManager;
class nsIDOMEvent;
class nsIDOMEventGroup;

/*
 * DOM event source class.  Object that allow event registration and
 * distribution from themselves implement this interface.
 */
 
/* 2fa04cfb-2494-41e5-ba76-9a79293eeb7e */
#define NS_IDOMEVENTRECEIVER_IID \
{0x2fa04cfb, 0x2494, 0x41e5, \
  { 0xba, 0x76, 0x9a, 0x79, 0x29, 0x3e, 0xeb, 0x7e } }

class nsIDOMEventReceiver : public nsIDOMEventTarget
{
public:
  NS_DECLARE_STATIC_IID_ACCESSOR(NS_IDOMEVENTRECEIVER_IID)

  NS_IMETHOD AddEventListenerByIID(nsIDOMEventListener *aListener,
                                   const nsIID& aIID) = 0;
  NS_IMETHOD RemoveEventListenerByIID(nsIDOMEventListener *aListener,
                                      const nsIID& aIID) = 0;
  NS_IMETHOD GetListenerManager(PRBool aCreateIfNotFound,
                                nsIEventListenerManager** aResult) = 0;
  NS_IMETHOD HandleEvent(nsIDOMEvent *aEvent) = 0;
  NS_IMETHOD GetSystemEventGroup(nsIDOMEventGroup** aGroup) = 0;
};

NS_DEFINE_STATIC_IID_ACCESSOR(nsIDOMEventReceiver, NS_IDOMEVENTRECEIVER_IID)

#endif // nsIDOMEventReceiver_h__
