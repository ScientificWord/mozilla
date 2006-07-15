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


#ifndef nsIDOMLoadListener_h__
#define nsIDOMLoadListener_h__

#include "nsIDOMEvent.h"
#include "nsIDOMEventListener.h"

/*
 * Document load related event listener
 *
 */
#define NS_IDOMLOADLISTENER_IID \
{ /* d1810238-14f8-4cab-9b96-96bedb9de7be */ \
0xd1810238, 0x14f8, 0x4cab, \
{0x9b, 0x96, 0x96, 0xbe, 0xdb, 0x9d, 0xe7, 0xbe} }

class nsIDOMLoadListener : public nsIDOMEventListener {

public:
  NS_DECLARE_STATIC_IID_ACCESSOR(NS_IDOMLOADLISTENER_IID)
  /**
  * Processes a page or image load event
  * @param aMouseEvent @see nsIDOMEvent.h 
  * @returns whether the event was consumed or ignored. @see nsresult
  */
  NS_IMETHOD Load(nsIDOMEvent* aEvent) = 0;

  /**
   * Processes a page beforeUnload event
   * @param aMouseEvent @see nsIDOMEvent.h
   * @returns whether the event was consumed or ignored. @see nsresult
   */
  NS_IMETHOD BeforeUnload(nsIDOMEvent* aEvent) = 0;

  /**
   * Processes a page unload event
   * @param aMouseEvent @see nsIDOMEvent.h 
   * @returns whether the event was consumed or ignored. @see nsresult
   */
  NS_IMETHOD Unload(nsIDOMEvent* aEvent) = 0;

  /**
   * Processes a load abort event
   * @param aMouseEvent @see nsIDOMEvent.h 
   * @returns whether the event was consumed or ignored. @see nsresult
   *
   */
  NS_IMETHOD Abort(nsIDOMEvent* aEvent) = 0;

  /**
   * Processes an load error event
   * @param aMouseEvent @see nsIDOMEvent.h 
   * @returns whether the event was consumed or ignored. @see nsresult
   */
  NS_IMETHOD Error(nsIDOMEvent* aEvent) = 0;

};

NS_DEFINE_STATIC_IID_ACCESSOR(nsIDOMLoadListener, NS_IDOMLOADLISTENER_IID)

#endif // nsIDOMLoadListener_h__
