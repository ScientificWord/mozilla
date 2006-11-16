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
m_endOffset(msiIMathMLEditingBC::INVALID)
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
}

NS_IMETHODIMP DeleteMathmlRangeTxn::DoTransaction(void)
{

  if (!m_editor || !m_topCaret)
    return NS_ERROR_NOT_INITIALIZED;
  if (!(m_start||m_end))
    return NS_OK; //nothing to do
//  nsCOMPtr<nsIDOMNode> topMMLNode, startLeft, startRight, endLeft, endRight;
//  PRUint32 startOffset(msiIMathMLEditingBC::INVALID), endOffset(msiIMathMLEditingBC::INVALID);
//  PRUint32 numKids(0);
//  nsresult res(NS_OK);  
//  res = m_topCaret->SplitAtDecendents(m_editor, m_start, m_startOffset,
//                                      m_end, m_endOffset, 
//                                      &startOffset, &endOffset, 
//                                      getter_AddRefs(startLeft), 
//                                      getter_AddRefs(startRight),
//                                      getter_AddRefs(endLeft), 
//                                      getter_AddRefs(endRight));
//  if (NS_FAILED(res) || (startOffset == msiIMathMLEditingBC::INVALID &&
//      endOffset == msiIMathMLEditingBC::INVALID))
//    return NS_ERROR_FAILURE;
  nsCOMPtr<nsIArray> transactionList;
  nsCOMPtr<nsIDOMNode> secondPhaseCoalesceNode;
  PRUint32 secondPhaseCoalesceOffset(msiIMathMLEditingBC::INVALID);
  nsCOMPtr<nsIArray> coalesceNodeList;
  nsresult res = m_topCaret->SetupDeletionTransactions(m_editor, m_start, m_startOffset, m_end, m_endOffset,
                                                       getter_AddRefs(transactionList),
                                                       getter_AddRefs(secondPhaseCoalesceNode), 
                                                       &secondPhaseCoalesceOffset);
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
  return res;
}

NS_IMETHODIMP DeleteMathmlRangeTxn::UndoTransaction(void)
{
  if (!m_editor || !m_topCaret)
    return NS_ERROR_NOT_INITIALIZED;
  return EditAggregateTxn::UndoTransaction();
}

NS_IMETHODIMP DeleteMathmlRangeTxn::RedoTransaction(void)
{
  if (!m_editor || !m_topCaret)
    return NS_ERROR_NOT_INITIALIZED;
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

