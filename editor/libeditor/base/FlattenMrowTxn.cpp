// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.


#include "FlattenMrowTxn.h"
#include "nsIDOMNodeList.h"
#include "DeleteElementTxn.h"
#include "InsertElementTxn.h"
#include "TransactionFactory.h"
#include "nsEditor.h"
#include "nsSelectionState.h"

// note that aEditor is not refcounted
FlattenMrowTxn::FlattenMrowTxn()
: EditAggregateTxn(),
m_editor(nsnull),
m_rangeUpdater(nsnull)
{
}

NS_IMETHODIMP FlattenMrowTxn::Init(nsIEditor * editor,
                                   nsIDOMNode * mrow,
                                   nsRangeUpdater *rangeUpdater)
{
  if (!editor || !mrow || !rangeUpdater)
    return NS_ERROR_NULL_POINTER;

  m_editor = editor;
  m_rangeUpdater = rangeUpdater;
  m_mrow = mrow;
  return NS_OK;

}

FlattenMrowTxn::~FlattenMrowTxn()
{
}

NS_IMETHODIMP FlattenMrowTxn::DoTransaction(void)
{

  if (!m_editor || !m_mrow || !m_rangeUpdater)
    return NS_ERROR_NOT_INITIALIZED;
  nsresult res(NS_OK);  
  nsCOMPtr<nsIDOMNode> parent;
  nsCOMPtr<nsIDOMNodeList> children;
  res = m_mrow->GetParentNode(getter_AddRefs(parent));
  res |= m_mrow->GetChildNodes(getter_AddRefs(children));
  if (NS_FAILED(res) || !parent || !children)
    return NS_ERROR_FAILURE;
  PRInt32 _offset(-1);
  res = nsEditor::GetChildOffset(m_mrow, parent, _offset);  
  if (NS_FAILED(res) || _offset < 0)
    return NS_ERROR_FAILURE;
  PRUint32 offset(static_cast<PRUint32>(_offset));
  
    
  PRUint32 numKids(0);
  children->GetLength(&numKids);
  for (PRUint32 i=0; i < numKids && NS_SUCCEEDED(res); i++)
  {
    nsCOMPtr<nsIDOMNode> child;
    res = children->Item(numKids-1-i, getter_AddRefs(child));
    if (NS_SUCCEEDED(res) && child)
    {
      InsertElementTxn * iTxn = nsnull;
      DeleteElementTxn * dTxn = nsnull;
      res = TransactionFactory::GetNewTransaction(DeleteElementTxn::GetCID(), (EditTxn **)&dTxn);
      if (NS_FAILED(res) || !dTxn) 
        return NS_ERROR_FAILURE;
      res = dTxn->Init(m_editor, child, nsnull); // Want to handle range updates locally -- hence nsnull 
      if (NS_SUCCEEDED(res)) 
        AppendChild(dTxn);
      NS_RELEASE(dTxn);
      res = TransactionFactory::GetNewTransaction(InsertElementTxn::GetCID(), (EditTxn **)&iTxn);
      if (NS_SUCCEEDED(res) && iTxn)
        res = iTxn->Init(child, parent, offset, m_editor);
      else 
        res = NS_ERROR_FAILURE;  
      if (NS_SUCCEEDED(res))
        AppendChild(iTxn);
      NS_RELEASE(iTxn);
    }
    else
      res = NS_ERROR_FAILURE;
  }
  DeleteElementTxn *txn;
  res = TransactionFactory::GetNewTransaction(DeleteElementTxn::GetCID(), (EditTxn **)&txn);
  if (NS_FAILED(res) || !txn) 
    return NS_ERROR_FAILURE;
  res = txn->Init(m_editor, m_mrow, nsnull); // Want to handle range updates locally -- hence nsnull 
  if (NS_SUCCEEDED(res)) 
    AppendChild(txn);
  NS_RELEASE(txn);
  if (NS_SUCCEEDED(res))
  {
    if (m_rangeUpdater) 
    {
      m_rangeUpdater->SelAdjDeleteNode(m_mrow, PR_FALSE); // don't want deep adjustment
      if (numKids > 0)
        m_rangeUpdater->SelAdjInsertNodes(parent, offset, numKids);
    }
    res = EditAggregateTxn::DoTransaction();
  }
  return res;
}

NS_IMETHODIMP FlattenMrowTxn::UndoTransaction(void)
{
  if (!m_editor || !m_mrow || !m_rangeUpdater)
    return NS_ERROR_NOT_INITIALIZED;
  return EditAggregateTxn::UndoTransaction();
}

NS_IMETHODIMP FlattenMrowTxn::RedoTransaction(void)
{
  if (!m_editor || !m_mrow || !m_rangeUpdater)
    return NS_ERROR_NOT_INITIALIZED;
  return EditAggregateTxn::RedoTransaction();
}

NS_IMETHODIMP FlattenMrowTxn::Merge(nsITransaction *aTransaction, PRBool *aDidMerge)
{
  if (aDidMerge)
    *aDidMerge = PR_FALSE;
  return NS_OK;
}

NS_IMETHODIMP FlattenMrowTxn::GetTxnDescription(nsAString& aString)
{
  aString.Assign(NS_LITERAL_STRING("FlattenMrowTxn"));
  return NS_OK;
}

