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
 * Contributor(s): Larry Hughes larry.hughes@mackichan.com
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

#include "ReplaceElementTxn.h"
#include "nsISelection.h"
#include "nsIContent.h"
#include "nsIDOMNodeList.h"
#include "nsReadableUtils.h"
#include "nsSelectionState.h"

#ifdef NS_DEBUG
static PRBool gNoisy = PR_FALSE;
#endif


ReplaceElementTxn::ReplaceElementTxn()
  : EditTxn(), m_rangeUpdater(nsnull), m_deepRangeUpdate(PR_TRUE)
{
}

NS_IMETHODIMP ReplaceElementTxn::Init(nsIDOMNode *aNewChild,
                                      nsIDOMNode *aOldChild,
                                      nsIDOMNode *aParent,
                                      nsIEditor  *aEditor,
                                      PRBool deepRangeUpdate,
                                      nsRangeUpdater *aRangeUpdater)
{
  NS_ASSERTION(aParent && aNewChild && aOldChild && aEditor, "bad arg");
  if (!aParent ||  !aNewChild || !aOldChild || !aEditor)
    return NS_ERROR_NULL_POINTER;

  m_parent = do_QueryInterface(aParent);
  m_newChild = do_QueryInterface(aNewChild);
  m_oldChild = do_QueryInterface(aOldChild);
  m_editor = aEditor;
  m_deepRangeUpdate = deepRangeUpdate;
  if (aRangeUpdater)
    m_rangeUpdater = aRangeUpdater;
  return NS_OK;
}


ReplaceElementTxn::~ReplaceElementTxn()
{
}

NS_IMETHODIMP ReplaceElementTxn::DoTransaction(void)
{
#ifdef NS_DEBUG
  if (gNoisy) 
  { 
    nsCOMPtr<nsIContent>newChildAsContent = do_QueryInterface(m_newChild);
    nsCOMPtr<nsIContent>oldChildAsContent = do_QueryInterface(m_oldChild);
    nsCOMPtr<nsIContent>parentAsContent = do_QueryInterface(m_parent);
    nsString newNamestr, oldNamestr;
    m_newChild->GetNodeName(newNamestr);
    m_oldChild->GetNodeName(oldNamestr);
    char* new_nodename = ToNewCString(newNamestr);
    char* old_nodename = ToNewCString(oldNamestr);
    printf("%p Do Replace Element of %p <%s> into parent %p with %p <%s>\n", 
           this, oldChildAsContent.get(), old_nodename,
           parentAsContent.get(), newChildAsContent.get(), new_nodename); 
    nsMemory::Free(new_nodename);
    nsMemory::Free(old_nodename);
  }
#endif
  if (!m_parent) 
    return NS_OK;
  if (!m_newChild || !m_oldChild || !m_editor)
    return NS_ERROR_NOT_INITIALIZED;
  m_editor->MarkNodeDirty(m_parent);
  
  if (m_rangeUpdater)
    m_rangeUpdater->SelAdjReplaceNode(m_newChild, m_oldChild, m_parent, m_deepRangeUpdate);

  nsCOMPtr<nsIDOMNode> resultNode;
  nsresult result = m_parent->ReplaceChild(m_newChild, m_oldChild,
                                            getter_AddRefs(resultNode));
  if (NS_FAILED(result)) 
    return result;
  if (!resultNode) 
    return NS_ERROR_NULL_POINTER;
  return result;
}

NS_IMETHODIMP ReplaceElementTxn::UndoTransaction(void)
{
#ifdef NS_DEBUG
#endif

  if (!m_parent) 
    return NS_OK;
  if (!m_newChild || !m_oldChild || !m_editor)
    return NS_ERROR_NOT_INITIALIZED;

  nsCOMPtr<nsIDOMNode> resultNode;
  nsresult result = m_parent->ReplaceChild(m_oldChild, m_newChild,
                                            getter_AddRefs(resultNode));
  return result;                                            
}


NS_IMETHODIMP ReplaceElementTxn::RedoTransaction(void)
{
  if (!m_parent) 
    return NS_OK;
  if (!m_newChild || !m_oldChild || !m_editor)
    return NS_ERROR_NOT_INITIALIZED;
  
  nsCOMPtr<nsIDOMNode> resultNode;
  nsresult result = m_parent->ReplaceChild(m_newChild, m_oldChild,
                                            getter_AddRefs(resultNode));
  if (NS_FAILED(result)) 
    return result;
  if (!resultNode) 
    return NS_ERROR_NULL_POINTER;
  return result;
}

NS_IMETHODIMP ReplaceElementTxn::Merge(nsITransaction *aTransaction, PRBool *aDidMerge)
{
  if (aDidMerge)
    *aDidMerge = PR_FALSE;
  return NS_OK;
}

NS_IMETHODIMP ReplaceElementTxn::GetTxnDescription(nsAString& aString)
{
  aString.Assign(NS_LITERAL_STRING("ReplaceElementTxn"));
  return NS_OK;
}
