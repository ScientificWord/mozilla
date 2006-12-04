// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef DeleteMMLRangeTxn_h__
#define DeleteMMlRangeTxn_h__

#include "EditAggregateTxn.h"
#include "nsIDOMNode.h"
#include "nsCOMPtr.h"

#define DELETE_MATHML_RANGE_TXN_CID \
{/* 965f2db4-e3bc-462d-9f1c-e7199cda6fc6 */ \
0x965f2db4, 0xe3bc, 0x462d, \
{0x9f, 0x1c, 0xe7, 0x19, 0x9c, 0xda, 0x6f, 0xc6} }

class nsIDOMRange;
class nsIEditor;
class msiIMathMLCaret;
                                                                                
/**
 * A transaction that deletes an entire range in the content tree
 */
class DeleteMathmlRangeTxn : public EditAggregateTxn
{
public:

  static const nsIID& GetCID() { static const nsIID iid = DELETE_MATHML_RANGE_TXN_CID; return iid; }

  /** initialize the transaction.
    * @param aEditor the object providing basic editing operations
    * @param aRange  the range to delete
    */
  NS_IMETHOD Init(nsIEditor * editor, 
                  nsCOMPtr<msiIMathMLCaret> & topCaret,
                  nsCOMPtr<nsIDOMNode> & start, PRUint32 startOffset,
                  nsCOMPtr<nsIDOMNode> & end, PRUint32 endOffset);

private:
  DeleteMathmlRangeTxn();

public:

  virtual ~DeleteMathmlRangeTxn();

  NS_IMETHOD DoTransaction(void);

  NS_IMETHOD UndoTransaction(void);

  NS_IMETHOD RedoTransaction(void);

  NS_IMETHOD Merge(nsITransaction *aTransaction, PRBool *aDidMerge);

  NS_IMETHOD GetTxnDescription(nsAString& aTxnDescription);

protected:

 nsIEditor * m_editor;
 nsCOMPtr<msiIMathMLCaret> m_topCaret;
 nsCOMPtr<nsIDOMNode> m_start;
 PRUint32 m_startOffset;
 nsCOMPtr<nsIDOMNode> m_end;
 PRUint32 m_endOffset;
 EditAggregateTxn * m_coalesceTxn;
  
  friend class TransactionFactory;

};

#endif
