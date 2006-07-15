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
 * Portions created by the Initial Developer are Copyright (C) 1998-1999
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

#ifndef DeleteTextTxn_h__
#define DeleteTextTxn_h__

#include "EditTxn.h"
#include "nsIEditor.h"
#include "nsIDOMCharacterData.h"
#include "nsCOMPtr.h"

#define DELETE_TEXT_TXN_CID \
{/* 4d3a2720-ac49-11d2-86d8-000064657374 */ \
0x4d3a2720, 0xac49, 0x11d2, \
{0x86, 0xd8, 0x0, 0x0, 0x64, 0x65, 0x73, 0x74} }

class nsRangeUpdater;

/**
 * A transaction that removes text from a content node. 
 */
class DeleteTextTxn : public EditTxn
{
public:

  static const nsIID& GetCID() { static const nsIID iid = DELETE_TEXT_TXN_CID; return iid; }

  /** initialize the transaction.
    * @param aEditor  the provider of basic editing operations
    * @param aElement the content node to remove text from
    * @param aOffset  the location in aElement to begin the deletion
    * @param aNumCharsToDelete  the number of characters to delete.  Not the number of bytes!
    */
  NS_IMETHOD Init(nsIEditor *aEditor,
                  nsIDOMCharacterData *aElement,
                  PRUint32 aOffset,
                  PRUint32 aNumCharsToDelete,
                  nsRangeUpdater *aRangeUpdater);

private:
  DeleteTextTxn();

public:
	virtual ~DeleteTextTxn();

  NS_IMETHOD DoTransaction(void);

  NS_IMETHOD UndoTransaction(void);

  NS_IMETHOD Merge(nsITransaction *aTransaction, PRBool *aDidMerge);

  NS_IMETHOD GetTxnDescription(nsAString& aTxnDescription);

protected:

  /** the provider of basic editing operations */
  nsIEditor* mEditor;

  /** the text element to operate upon */
  nsCOMPtr<nsIDOMCharacterData> mElement;
  
  /** the offset into mElement where the deletion is to take place */
  PRUint32 mOffset;

  /** the number of characters to delete */
  PRUint32 mNumCharsToDelete;

  /** the text that was deleted */
  nsString mDeletedText;

  /** range updater object */
  nsRangeUpdater *mRangeUpdater;
  
  friend class TransactionFactory;

};

#endif
