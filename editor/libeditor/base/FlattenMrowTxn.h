// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef FlattenMrowTxn_h__
#define FlattenMrowTxn_h__

#include "EditAggregateTxn.h"
#include "nsIDOMNode.h"
#include "nsCOMPtr.h"

#define FLATTEN_MROW_TXN_CID \
{/* fa2f1fbd-1d78-4d64-9808-7707ae63c7a3 */ \
0xfa2f1fbd, 0x1d78, 0x4d64, \
{0x98, 0x08, 0x77, 0x07, 0xae, 0x63, 0xc7, 0xa3} }

class nsIEditor;
class nsRangeUpdater;
                                                                                
/**
 * A transaction that flatten a mrow -- that is removes the mrow and dumps its children into the mrow's
 * parent at the offset of the mrow in it's parent.
 */
class FlattenMrowTxn : public EditAggregateTxn
{
public:

  static const nsIID& GetCID() { static const nsIID iid = FLATTEN_MROW_TXN_CID; return iid; }

  /** initialize the transaction.
    * @param aEditor the object providing basic editing operations
    * @param aRange  the range to delete
    */
  NS_IMETHOD Init(nsIEditor * editor, nsIDOMNode * mrow, nsRangeUpdater *rangeUpdater);

private:
  FlattenMrowTxn();

public:

  virtual ~FlattenMrowTxn();

  NS_IMETHOD DoTransaction(void);

  NS_IMETHOD UndoTransaction(void);

  NS_IMETHOD RedoTransaction(void);

  NS_IMETHOD Merge(nsITransaction *aTransaction, PRBool *aDidMerge);

  NS_IMETHOD GetTxnDescription(nsAString& aTxnDescription);

protected:

  nsCOMPtr<nsIDOMNode> m_mrow;
  nsIEditor * m_editor;
  nsRangeUpdater *m_rangeUpdater;
  
  friend class TransactionFactory;

};

#endif
