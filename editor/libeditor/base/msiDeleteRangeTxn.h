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

#ifndef MSIDeleteRangeTxn_h__
#define MSIDeleteRangeTxn_h__

#include "EditAggregateTxn.h"
#include "nsIDOMNode.h"
#include "nsIDOMRange.h"
#include "nsIEditor.h"
#include "nsCOMPtr.h"

#define MSI_DELETE_RANGE_TXN_CID \
{/* 28f1bdb3-d55f-45b1-bca6-82c718c0056d */ \
0x28f1bdb3, 0xd55f, 0x45b1, \
{0xbc, 0xa6, 0x82, 0xc7, 0x18, 0xc0, 0x05, 0x6d} }

class nsIEditor;
class nsRangeUpdater;
class msiSelectionManager;

/**
 * A transaction that deletes an entire range in the content tree
 */
class msiDeleteRangeTxn : public EditAggregateTxn
{
public:

  static const nsIID& GetCID() { static const nsIID iid = MSI_DELETE_RANGE_TXN_CID; return iid; }

  /** initialize the transaction.
    * @param aEditor the object providing basic editing operations
    * @param aRange  the range to delete
    */
  nsresult Init(nsIEditor *editor,
                msiSelectionManager * msiSelMan,
                PRInt32 rangeIndex,
                nsRangeUpdater *rangeUpdater);

private:
  msiDeleteRangeTxn();

public:

  virtual ~msiDeleteRangeTxn();

  NS_IMETHOD DoTransaction(void);

  NS_IMETHOD UndoTransaction(void);

  NS_IMETHOD RedoTransaction(void);

  NS_IMETHOD Merge(nsITransaction *aTransaction, PRBool *aDidMerge);

  NS_IMETHOD GetTxnDescription(nsAString& aTxnDescription);

protected:

  nsresult CreateTxnsToDeleteBetween(nsIDOMNode *aStartParent, 
                                     PRUint32    aStartOffset, 
                                     PRUint32    aEndOffset);
                                     
  nsresult CreateTxnsToDeleteNodesBetween(nsCOMPtr <nsIDOMRange> & range);
  
  nsresult CreateTxnsToDeleteContent(nsIDOMNode *aParent, 
                                     PRUint32 aOffset, 
                                     nsIEditor::EDirection aAction);
                                     
  nsresult GetMathParent(nsIDOMNode * node, nsCOMPtr<nsIDOMNode> & mathParent);
  
  nsresult GetIndexOfChildInParent(nsIDOMNode * child, PRUint32 &index);
  
protected:
  
  /** pointer to range data */
  msiSelectionManager * m_msiSelMan;
  PRInt32 m_rangeIndex;

  /** the editor for this transaction */
  nsIEditor* m_editor;

  /** range updater object */
  nsRangeUpdater *m_rangeUpdater;
  
  friend class TransactionFactory;

};

#endif
