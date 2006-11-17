/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: NPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Netscape Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/NPL/
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
 * Contributor(s):  Larry Hughes larry.hughes@mackichan.com
 *
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the NPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the NPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef ReplaceElementTxn_h__
#define ReplaceElementTxn_h__

#include "EditTxn.h"
#include "nsIEditor.h"
#include "nsIDOMNode.h"
#include "nsCOMPtr.h"

#define REPLACE_ELEMENT_TXN_CID \
{/* {F92967DE-700A-4e1a-9904-8F801B49388D */ \
0xf92967de, 0x700a, 0x4e1a, \
{0x99, 0x4, 0x8f, 0x80, 0x1b, 0x49, 0x38, 0x8d} }

class nsRangeUpdater;

/**
 * A transaction that inserts a single element
 */
class ReplaceElementTxn : public EditTxn
{
public:

  static const nsIID& GetCID() { static const nsIID iid = REPLACE_ELEMENT_TXN_CID; return iid; }

  /** initialize the transaction.
    * @param aNode   the node to insert
    * @param aParent the node to insert into
    * @param aOffset the offset in aParent to insert aNode
    */
  NS_IMETHOD Init(nsIDOMNode *aNewChild,
                  nsIDOMNode *aOldChild,
                  nsIDOMNode *aParent,
                  nsIEditor  *aEditor,
                  nsRangeUpdater * rangeUpdater);

private:
  ReplaceElementTxn();

public:

  virtual ~ReplaceElementTxn();

  NS_IMETHOD DoTransaction(void);

  NS_IMETHOD UndoTransaction(void);
  
  NS_IMETHOD RedoTransaction(void);

  NS_IMETHOD Merge(nsITransaction *aTransaction, PRBool *aDidMerge);

  NS_IMETHOD GetTxnDescription(nsAString& aTxnDescription);

protected:
  
  /** the element to insert */
  nsCOMPtr<nsIDOMNode> m_newChild;
  /* the element to be replaced */
  nsCOMPtr<nsIDOMNode> m_oldChild;

  /** the node into which the new node will be inserted */
  nsCOMPtr<nsIDOMNode> m_parent;

  /** the editor for this transaction */
  nsIEditor*           m_editor;

  /** range updater object */
  nsRangeUpdater *m_rangeUpdater;

  friend class TransactionFactory;

};

#endif
