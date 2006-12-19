// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.


#include "ReplaceScriptBaseTxn.h"
#include "nsIDOMNodeList.h"
#include "ReplaceElementTxn.h"
#include "TransactionFactory.h"
#include "nsEditor.h"
#include "nsSelectionState.h"

// note that aEditor is not refcounted
ReplaceScriptBaseTxn::ReplaceScriptBaseTxn()
: EditAggregateTxn(),
m_editor(nsnull),
m_rangeUpdater(nsnull)
{
}

NS_IMETHODIMP ReplaceScriptBaseTxn::Init(nsIEditor * editor,
                                          nsIDOMNode * script,
                                          nsIDOMNode * newbase,
                                          nsRangeUpdater *rangeUpdater)
{                                   
  if (!editor || !script || !newbase || !rangeUpdater)
    return NS_ERROR_NULL_POINTER;

  m_editor = editor;
  m_rangeUpdater = rangeUpdater;
  m_script = script;
  m_newbase = newbase;
  return NS_OK;

}

ReplaceScriptBaseTxn::~ReplaceScriptBaseTxn()
{
}

NS_IMETHODIMP ReplaceScriptBaseTxn::DoTransaction(void)
{
  if (!m_editor || !m_script || !m_newbase || !m_rangeUpdater)
    return NS_ERROR_NOT_INITIALIZED;
  nsCOMPtr<nsIDOMNode> currBase;
  m_script->GetFirstChild(getter_AddRefs(currBase));
  if (!currBase)
    return NS_ERROR_NOT_INITIALIZED;
  nsresult res(NS_OK);
  ReplaceElementTxn *txn;
  res = TransactionFactory::GetNewTransaction(ReplaceElementTxn::GetCID(), (EditTxn **)&txn);
  if (NS_FAILED(res) || !txn) 
    return NS_ERROR_FAILURE;
  res = txn->Init(m_newbase, currBase, m_script, m_editor, PR_FALSE, m_rangeUpdater);
  if (NS_SUCCEEDED(res)) 
    AppendChild(txn);
  NS_RELEASE(txn);
  res = EditAggregateTxn::DoTransaction();
  return res;
}

NS_IMETHODIMP ReplaceScriptBaseTxn::UndoTransaction(void)
{
  if (!m_editor || !m_script || !m_newbase || !m_rangeUpdater)
    return NS_ERROR_NOT_INITIALIZED;
  return EditAggregateTxn::UndoTransaction();
}

NS_IMETHODIMP ReplaceScriptBaseTxn::RedoTransaction(void)
{
  if (!m_editor || !m_script || !m_newbase || !m_rangeUpdater)
    return NS_ERROR_NOT_INITIALIZED;
  return EditAggregateTxn::RedoTransaction();
}

NS_IMETHODIMP ReplaceScriptBaseTxn::Merge(nsITransaction *aTransaction, PRBool *aDidMerge)
{
  if (aDidMerge)
    *aDidMerge = PR_FALSE;
  return NS_OK;
}

NS_IMETHODIMP ReplaceScriptBaseTxn::GetTxnDescription(nsAString& aString)
{
  aString.Assign(NS_LITERAL_STRING("ReplaceScriptBaseTxn"));
  return NS_OK;
}

