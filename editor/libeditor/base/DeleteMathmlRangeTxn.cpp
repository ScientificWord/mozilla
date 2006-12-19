// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.


#include "DeleteMathmlRangeTxn.h"
#include "ReplaceElementTxn.h"
#include "nsIDOMRange.h"
#include "nsIDOMCharacterData.h"
#include "nsIDOMNodeList.h"
#include "nsISelection.h"
#include "DeleteTextTxn.h"
#include "DeleteElementTxn.h"
#include "TransactionFactory.h"
#include "nsIContentIterator.h"
#include "nsIContent.h"
#include "nsComponentManagerUtils.h"
#include "msiEditor.h"
#include "msiIMathMLCaret.h"
#include "msiIMathMLEditor.h"

// note that aEditor is not refcounted
DeleteMathmlRangeTxn::DeleteMathmlRangeTxn()
: EditAggregateTxn(),
m_editor(nsnull),
m_startOffset(msiIMathMLEditingBC::INVALID),
m_endOffset(msiIMathMLEditingBC::INVALID),
m_coalesceTxn(nsnull)
{
}

NS_IMETHODIMP DeleteMathmlRangeTxn::Init(nsIEditor * editor, 
                                         nsCOMPtr<msiIMathMLCaret> & topCaret,
                                         nsCOMPtr<nsIDOMNode> & start, PRUint32 startOffset,
                                         nsCOMPtr<nsIDOMNode> & end, PRUint32 endOffset)
{
  if (!editor || !topCaret)
    return NS_ERROR_NULL_POINTER;

  m_editor = editor;
  m_topCaret = topCaret;
  m_start = start;
  m_startOffset = startOffset;
  m_end = end;
  m_endOffset = endOffset;
  return NS_OK;

}

DeleteMathmlRangeTxn::~DeleteMathmlRangeTxn()
{
  if (m_coalesceTxn)
    NS_RELEASE(m_coalesceTxn);
}

NS_IMETHODIMP DeleteMathmlRangeTxn::DoTransaction(void)
{

  if (!m_editor || !m_topCaret)
    return NS_ERROR_NOT_INITIALIZED;
  if (!(m_start||m_end))
    return NS_OK; //nothing to do
  nsCOMPtr<nsIArray> transactionList;
  nsCOMPtr<nsIDOMNode> coalesceNode;
  PRUint32 coalesceOffset(msiIMathMLEditingBC::INVALID);
  nsCOMPtr<nsIArray> coalesceNodeList;
  nsresult res = m_topCaret->SetupDeletionTransactions(m_editor, m_start, m_startOffset, m_end, m_endOffset,
                                                       getter_AddRefs(transactionList),
                                                       getter_AddRefs(coalesceNode), 
                                                       &coalesceOffset);
  if (NS_FAILED(res) || !transactionList)
    return NS_ERROR_FAILURE;
  nsCOMPtr<nsISimpleEnumerator> enumerator;
  transactionList->Enumerate(getter_AddRefs(enumerator));
  if (enumerator)
  {
    PRBool someMore(PR_FALSE);
    while (NS_SUCCEEDED(res) && NS_SUCCEEDED(enumerator->HasMoreElements(&someMore)) && someMore) 
    {
      nsCOMPtr<nsISupports> isupp;
      if (NS_FAILED(enumerator->GetNext(getter_AddRefs(isupp))))
      {
        res = NS_ERROR_FAILURE; 
        break;
      }
      nsCOMPtr<nsITransaction> txn(do_QueryInterface(isupp));
      if (txn)
      {
        nsITransaction * xxx = txn;
        AppendChild((EditTxn*)xxx);
      }
      else
        res = NS_ERROR_FAILURE;
    }  
  }
  if (NS_SUCCEEDED(res))
    res = EditAggregateTxn::DoTransaction();
  if (NS_SUCCEEDED(res) && coalesceNode &&  IS_VALID_NODE_OFFSET(coalesceOffset))
  {
    nsresult res1(NS_OK);
    nsCOMPtr<msiIMathMLEditor> msiEditor(do_QueryInterface(m_editor));
    nsCOMPtr<msiIMathMLCaret> mmlCaret;
    EditAggregateTxn * coalesceTxn = nsnull;
    res1 = TransactionFactory::GetNewTransaction(EditAggregateTxn::GetCID(), (EditTxn**)&coalesceTxn);
    if (NS_SUCCEEDED(res1) && msiEditor)
      res1 = msiEditor->GetMathMLCaretInterface(coalesceNode, coalesceOffset, getter_AddRefs(mmlCaret));
    else
      res1 = NS_ERROR_FAILURE;
    if (NS_SUCCEEDED(res1) && coalesceTxn && mmlCaret)
    {
      nsCOMPtr<nsIArray> coalesceTxnList;
      res1 = mmlCaret->SetupCoalesceTransactions(m_editor, getter_AddRefs(coalesceTxnList));
      if (NS_SUCCEEDED(res1) && coalesceTxnList)
      {
        nsCOMPtr<nsISimpleEnumerator> enumerator;
        coalesceTxnList->Enumerate(getter_AddRefs(enumerator));
        if (enumerator)
        {
          PRBool someMore(PR_FALSE);
          while (NS_SUCCEEDED(res1) && NS_SUCCEEDED(enumerator->HasMoreElements(&someMore)) && someMore) 
          {
            nsCOMPtr<nsISupports> isupp;
            if (NS_FAILED(enumerator->GetNext(getter_AddRefs(isupp))))
            {
              res1 = NS_ERROR_FAILURE; 
              break;
            }
            nsCOMPtr<nsITransaction> txn(do_QueryInterface(isupp));
            if (txn)
            {
              nsITransaction * xxx = txn;
              coalesceTxn->AppendChild((EditTxn*)xxx);
            }
            else
              res1 = NS_ERROR_FAILURE;
          }  
        }
        if (NS_SUCCEEDED(res1))
        { 
          m_coalesceTxn = coalesceTxn; // keep arount for undo and redo
          m_coalesceTxn->DoTransaction();
        }
        else
          NS_RELEASE(coalesceTxn);
      }  
    }
  }
  return res;
}

NS_IMETHODIMP DeleteMathmlRangeTxn::UndoTransaction(void)
{
  if (!m_editor || !m_topCaret)
    return NS_ERROR_NOT_INITIALIZED;
  if (m_coalesceTxn)
    m_coalesceTxn->UndoTransaction();
  return EditAggregateTxn::UndoTransaction();
}

NS_IMETHODIMP DeleteMathmlRangeTxn::RedoTransaction(void)
{
  if (!m_editor || !m_topCaret)
    return NS_ERROR_NOT_INITIALIZED;
  if (m_coalesceTxn)
    m_coalesceTxn->RedoTransaction();  
  return EditAggregateTxn::RedoTransaction();
}

NS_IMETHODIMP DeleteMathmlRangeTxn::Merge(nsITransaction *aTransaction, PRBool *aDidMerge)
{
  if (aDidMerge)
    *aDidMerge = PR_FALSE;
  return NS_OK;
}

NS_IMETHODIMP DeleteMathmlRangeTxn::GetTxnDescription(nsAString& aString)
{
  aString.Assign(NS_LITERAL_STRING("DeleteMathmlRangeTxn"));
  return NS_OK;
}

