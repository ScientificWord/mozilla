// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef ReplaceScriptBaseTxn_h__
#define ReplaceScriptBaseTxn_h__

#include "EditAggregateTxn.h"
#include "nsIDOMNode.h"
#include "nsCOMPtr.h"

#define REPLACE_SCRIPTBASE_TXN_CID \
{/* d7e2c712-ac9a-4824-8cf4-1297f49a0c83 */ \
0xd7e2c712, 0xac9a, 0x4824, \
{0x8c, 0xf4, 0x12, 0x97, 0xf4, 0x9a, 0x0c, 0x83} }

class nsIEditor;
class nsRangeUpdater;
                                                                                
/**
 * A transaction that flatten a mrow -- that is removes the mrow and dumps its children into the mrow's
 * parent at the offset of the mrow in it's parent.
 */
class ReplaceScriptBaseTxn : public EditAggregateTxn
{
public:

  static const nsIID& GetCID() { static const nsIID iid = REPLACE_SCRIPTBASE_TXN_CID; return iid; }

  /** initialize the transaction.
    * @param aEditor the object providing basic editing operations
    * @param aRange  the range to delete
    */
  NS_IMETHOD Init(nsIEditor * editor, nsIDOMNode * script, nsIDOMNode * newbase, nsRangeUpdater *rangeUpdater);

private:
  ReplaceScriptBaseTxn();

public:

  virtual ~ReplaceScriptBaseTxn();

  NS_IMETHOD DoTransaction(void);

  NS_IMETHOD UndoTransaction(void);

  NS_IMETHOD RedoTransaction(void);

  NS_IMETHOD Merge(nsITransaction *aTransaction, PRBool *aDidMerge);

  NS_IMETHOD GetTxnDescription(nsAString& aTxnDescription);

protected:

  nsCOMPtr<nsIDOMNode> m_script;
  nsCOMPtr<nsIDOMNode> m_newbase;
  nsIEditor * m_editor;
  nsRangeUpdater *m_rangeUpdater;
  
  friend class TransactionFactory;

};

#endif
