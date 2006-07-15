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

#ifndef nsIAbsorbingTransaction_h__
#define nsIAbsorbingTransaction_h__

#include "nsISupports.h"

/*
Transaction interface to outside world
*/

#define NS_IABSORBINGTRANSACTION_IID \
{ /* a6cf9116-15b3-11d2-932e-00805f8add32 */ \
    0xa6cf9116, \
    0x15b3, \
    0x11d2, \
    {0x93, 0x2e, 0x00, 0x80, 0x5f, 0x8a, 0xdd, 0x32} }

class nsSelectionState;
class nsIEditor;
class nsIAtom;

/**
 * A transaction interface mixin - for transactions that can support. 
 * the placeholder absorbtion idiom. 
 */
class nsIAbsorbingTransaction  : public nsISupports{
public:

  NS_DECLARE_STATIC_IID_ACCESSOR(NS_IABSORBINGTRANSACTION_IID)

  NS_IMETHOD Init(nsIAtom *aName, nsSelectionState *aSelState, nsIEditor *aEditor)=0;
  
  NS_IMETHOD EndPlaceHolderBatch()=0;
  
  NS_IMETHOD GetTxnName(nsIAtom **aName)=0;

  NS_IMETHOD StartSelectionEquals(nsSelectionState *aSelState, PRBool *aResult)=0;

  NS_IMETHOD ForwardEndBatchTo(nsIAbsorbingTransaction *aForwardingAddress)=0;
  
  NS_IMETHOD Commit()=0;
};

NS_DEFINE_STATIC_IID_ACCESSOR(nsIAbsorbingTransaction,
                              NS_IABSORBINGTRANSACTION_IID)

#endif // nsIAbsorbingTransaction_h__

