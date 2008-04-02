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

/*
 * a piece of state that is stored in session history when the document
 * is not
 */

#ifndef nsPresState_h_
#define nsPresState_h_

#include "prtypes.h"
#include "nsStringFwd.h"
#include "nsInterfaceHashtable.h"
#include "nsPoint.h"
#include "nsAutoPtr.h"
#include "nsRect.h"

class nsPresState
{
public:
  NS_HIDDEN_(nsresult) Init();

  NS_HIDDEN_(nsresult) GetStatePropertyAsSupports(const nsAString& aName,
                                                  nsISupports** aResult);

  NS_HIDDEN_(nsresult) SetStatePropertyAsSupports(const nsAString& aName,
                                                  nsISupports* aValue);

  NS_HIDDEN_(nsresult) GetStateProperty(const nsAString& aProperty,
                                        nsAString& aResult);

  NS_HIDDEN_(nsresult) SetStateProperty(const nsAString& aProperty,
                                        const nsAString& aValue);

  NS_HIDDEN_(nsresult) RemoveStateProperty(const nsAString& aProperty);

  NS_HIDDEN_(nsresult) SetScrollState(const nsRect& aState);

  nsRect GetScrollState();

// MEMBER VARIABLES
protected:
  nsInterfaceHashtable<nsStringHashKey,nsISupports> mPropertyTable;
  nsAutoPtr<nsRect> mScrollState;
};

NS_HIDDEN_(nsresult) NS_NewPresState(nsPresState **aState);

#endif /* nsPresState_h_ */
