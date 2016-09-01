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
 * Portions created by the Initial Developer are Copyright (C) 1998
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


#include "nsEditorUtils.h"
#include "nsIDOMDocument.h"
#include "nsIDOMRange.h"
#include "nsIContent.h"
#include "nsLayoutCID.h"
#include "nsIDOMHTMLDocument.h"
#include "nsIDOMDocumentTraversal.h"
#include "nsIDOMNodeFilter.h"
#include "nsIDOMTreeWalker.h"
#include "nsIDOMNodeList.h"
#include "nsIDOM3Node.h"
#include "nsISelection.h"// hooks
#include "nsIClipboardDragDropHooks.h"
#include "nsIClipboardDragDropHookList.h"
#include "nsISimpleEnumerator.h"
#include "nsIDocShell.h"
#include "nsIDocument.h"
#include "nsIInterfaceRequestorUtils.h"
#include "nsIHTMLDocument.h"
#include "nsIDOMHTMLElement.h"
#include "nsIHTMLEditor.h"
#include "msiEditor.h"
#include "nsEditor.h"
#include "nsIAtom.h"





/******************************************************************************
 * nsAutoSelectionReset
 *****************************************************************************/

nsAutoSelectionReset::nsAutoSelectionReset(nsISelection *aSel, nsEditor *aEd) :
mSel(nsnull)
,mEd(nsnull)
{
  if (!aSel || !aEd) return;    // not much we can do, bail.
  if (aEd->ArePreservingSelection()) return;   // we already have initted mSavedSel, so this must be nested call.
  mSel = do_QueryInterface(aSel);
  mEd = aEd;
  if (mSel)
  {
    mEd->PreserveSelectionAcrossActions(mSel);
  }
}

nsAutoSelectionReset::~nsAutoSelectionReset()
{
  NS_ASSERTION(!mSel || mEd, "mEd should be non-null when mSel is");
  if (mSel && mEd->ArePreservingSelection())   // mSel will be null if this was nested call
  {
    mEd->RestorePreservedSelection(mSel);
  }
}

void
nsAutoSelectionReset::Abort()
{
  NS_ASSERTION(!mSel || mEd, "mEd should be non-null when mSel is");
  if (mSel)
    mEd->StopPreservingSelection();
}


/******************************************************************************
 * some helper classes for iterating the dom tree
 *****************************************************************************/

nsDOMIterator::nsDOMIterator() :
mIter(nsnull)
{
}

nsDOMIterator::~nsDOMIterator()
{
}

nsresult
nsDOMIterator::Init(nsIDOMRange* aRange)
{
  nsresult res;
  mIter = do_CreateInstance("@mozilla.org/content/post-content-iterator;1", &res);
  if (NS_FAILED(res)) return res;
  if (!mIter) return NS_ERROR_FAILURE;
  return mIter->Init(aRange);
}

nsresult
nsDOMIterator::Init(nsIDOMNode* aNode)
{
  nsresult res;
  mIter = do_CreateInstance("@mozilla.org/content/post-content-iterator;1", &res);
  if (NS_FAILED(res)) return res;
  if (!mIter) return NS_ERROR_FAILURE;
  nsCOMPtr<nsIContent> content = do_QueryInterface(aNode);
  return mIter->Init(content);
}

void
nsDOMIterator::ForEach(nsDomIterFunctor& functor) const
{
  nsCOMPtr<nsIDOMNode> node;

  // iterate through dom
  while (!mIter->IsDone())
  {
    node = do_QueryInterface(mIter->GetCurrentNode());
    if (!node)
      return;

    functor(node);
    mIter->Next();
  }
}

nsresult
nsDOMIterator::AppendList(nsBoolDomIterFunctor& functor,
                          nsCOMArray<nsIDOMNode>& arrayOfNodes) const
{
  nsCOMPtr<nsIDOMNode> node;

  // iterate through dom and build list
  while (!mIter->IsDone())
  {
    node = do_QueryInterface(mIter->GetCurrentNode());
    if (!node)
      return NS_ERROR_NULL_POINTER;

    if (functor(node))
    {
      arrayOfNodes.AppendObject(node);
    }
    mIter->Next();
  }
  return NS_OK;
}

nsDOMSubtreeIterator::nsDOMSubtreeIterator()
{
}

nsDOMSubtreeIterator::~nsDOMSubtreeIterator()
{
}

nsresult
nsDOMSubtreeIterator::Init(nsIDOMRange* aRange)
{
  nsresult res;
  mIter = do_CreateInstance("@mozilla.org/content/subtree-content-iterator;1", &res);
  if (NS_FAILED(res)) return res;
  if (!mIter) return NS_ERROR_FAILURE;
  return mIter->Init(aRange);
}

nsresult
nsDOMSubtreeIterator::Init(nsIDOMNode* aNode)
{
  nsresult res;
  mIter = do_CreateInstance("@mozilla.org/content/subtree-content-iterator;1", &res);
  if (NS_FAILED(res)) return res;
  if (!mIter) return NS_ERROR_FAILURE;
  nsCOMPtr<nsIContent> content = do_QueryInterface(aNode);
  return mIter->Init(content);
}

/******************************************************************************
 * some general purpose editor utils
 *****************************************************************************/

PRBool
nsEditorUtils::IsDescendantOf(nsIDOMNode *aNode, nsIDOMNode *aParent, PRInt32 *aOffset)
{
  if (!aNode && !aParent) return PR_FALSE;
  if (aNode == aParent) return PR_FALSE;

  nsCOMPtr<nsIDOMNode> parent, node = do_QueryInterface(aNode);
  nsresult res;

  do
  {
    if (!node) return PR_FALSE; // jcs
    res = node->GetParentNode(getter_AddRefs(parent));
    if (NS_FAILED(res)) return PR_FALSE;
    if (parent == aParent)
    {
      if (aOffset)
      {
        nsCOMPtr<nsIContent> pCon(do_QueryInterface(parent));
        nsCOMPtr<nsIContent> cCon(do_QueryInterface(node));
        if (pCon)
        {
          *aOffset = pCon->IndexOf(cCon);
        }
      }
      return PR_TRUE;
    }
    node = parent;
  } while (parent);

  return PR_FALSE;
}

PRBool
nsEditorUtils::IsLeafNode(nsIDOMNode *aNode)
{
  PRBool hasChildren = PR_FALSE;
  if (aNode)
    aNode->HasChildNodes(&hasChildren);
  return !hasChildren;
}


PRBool nsEditorUtils::Acceptable(nsEditor * ed, msiITagListManager * tlm, nsIDOMNode * node)
// We are looking for a node that can accept text. Mostly these are text nodes and texttag nodes.
// Mathematics is acceptable; junk whitespace is not.
{
  nsCOMPtr<nsIDOMNode> parent;
  PRBool canContainText;
  nsIAtom * nsatom = nsnull;
  nsString text = NS_LITERAL_STRING("#text");
  if (ed->IsTextNode(node))
  {
    node->GetParentNode(getter_AddRefs(parent));
    if (!parent) return PR_FALSE;
    return nsEditorUtils::Acceptable(ed, tlm, parent);
  }
  else
  {
    parent = node;
  }
  tlm -> NodeCanContainTag(parent, text, nsatom, &canContainText);
  return canContainText;
}

PRBool skipThisTextNode( nsEditor * ed, msiITagListManager * tlm, nsIDOMNode * node) {
  if (!node || !(ed->IsTextNode(node))) return PR_FALSE;
  if (nodeIsWhiteSpace(node, 0, (PRInt32)(PRUint32)(-1))) {
    return PR_TRUE;
  }
  nsCOMPtr<nsIDOMNode> parent;
  node->GetParentNode(getter_AddRefs(parent));
  if (parent) {
    return !(nsEditorUtils::Acceptable(ed, tlm, parent));
  }
  return PR_TRUE;
}


PRBool LookForward(nsEditor * ed, msiITagListManager * tlm, nsISelection * sel, nsIDOMNode * nodeIn, nsIDOMNode ** nodeOut, PRInt32& offset)
{
  // do one step of a depth first forward traversal of the dom, looking for a node which accepts text. This includes paragraph tags, text tags,
  // math tags, and text nodes provided their parent accepts text.
  // Then recurse.
  nsCOMPtr<nsIDOMNode> newNode;
  PRInt32 newOffset;
  // Precondition: Acceptable(ed, tlm, node) is false.
  if (nsEditorUtils::Acceptable(ed, tlm, nodeIn)) {
    sel->Collapse(nodeIn, offset);
    return PR_TRUE;
  }

  if (offset == 0) // a common case
  {
    nodeIn->GetFirstChild(getter_AddRefs(newNode));
  }
  else {
    nsCOMPtr<nsIDOMNodeList> nl;
    nodeIn->GetChildNodes(getter_AddRefs(nl));
    nl->Item(offset, getter_AddRefs(newNode));
  }

  if (!newNode) {
    // we have run off the parent node
    return PR_FALSE;
  }

  // We have just gone to a new node, and we are going forward, so the offset is now 0.

  newOffset = 0;

  while (newNode && skipThisTextNode(ed, tlm, newNode)) {
    newNode->GetNextSibling(getter_AddRefs(newNode));
  }

  if (!newNode) { // went off the end of *node. Go back up the dom tree.
    offset = offset + 1;
    return LookForward(ed, tlm, sel, nodeIn, nodeOut, offset);
  }

  return LookForward(ed, tlm, sel, newNode, nodeOut, newOffset);
}

PRBool LookBackward(nsEditor * ed, msiITagListManager * tlm, nsISelection * sel, nsIDOMNode * nodeIn, nsIDOMNode ** node, PRInt32 offset)
{

  // do one step of a depth first forward traversal of the dom, looking for a node which accepts text. This includes paragraph tags, text tags,
  // math tags, and text nodes provided their parent accepts text.
  // Then recurse.
  nsCOMPtr<nsIDOMNode> newNode;
  PRInt32 newOffset;

  if (nodeIn == nsnull) return PR_FALSE;
  // Precondition: Acceptable(ed, tlm, node) is false.
  if (nsEditorUtils::Acceptable(ed, tlm, nodeIn)) {
    sel->Collapse(nodeIn, offset);
    return PR_TRUE;
  }

  nsCOMPtr<nsIDOMNodeList> nl;
  nodeIn->GetChildNodes(getter_AddRefs(nl));
  nl->Item(offset-1, getter_AddRefs(newNode));


  while (newNode && skipThisTextNode(ed, tlm, newNode))
  {
    newNode->GetPreviousSibling(getter_AddRefs(newNode));
  }

  if (!newNode) {
    // we have run off the parent node
    return PR_FALSE;
  }

  // We have just gone to a new node, and we are going backward, so the offset now points to the last child.
  if (ed->IsTextNode(newNode))
  {
    nsAutoString content;
    nsCOMPtr<nsIDOM3Node> domNode3 = do_QueryInterface(newNode);
    domNode3->GetTextContent(content);
    newOffset = content.Length();
  }
  else {
    newNode->GetChildNodes(getter_AddRefs(nl));
    PRUint32 offs;
    nl->GetLength(&offs);
    newOffset = offs;
  }

  if (!newNode) { // went off the end of *node. Go back up the dom tree.
    return LookBackward(ed, tlm, sel, nodeIn, node, offset);
  }

  return LookBackward(ed, tlm, sel, newNode, node, newOffset);
}



PRBool
nsEditorUtils::JiggleCursor(nsIEditor *aEditor, nsISelection * sel, nsIEditor::EDirection dir)
{
  return true;
  PRBool forward;
  PRBool success = PR_FALSE;
  PRInt32 startOffset;
  PRInt32 offset;
  nsCOMPtr<nsIDOMNode> startNode;
  nsCOMPtr<nsIDOMNode> node;
  nsCOMPtr<nsIDOMNode> nodeOut;
  nsCOMPtr<msiITagListManager> tlm;
  nsEditor * ed;
  nsresult res;




  switch (dir) {
    case nsIEditor::ePrevious:
    case nsIEditor::ePreviousWord:
    case nsIEditor::eToBeginningOfLine:
      forward = PR_FALSE;
      break;
    default:
      forward = PR_TRUE;
  }


  nsCOMPtr<msiIMathMLEditor> msiEditor = do_QueryInterface(aEditor);
  nsCOMPtr<nsIHTMLEditor> htmlEditor = do_QueryInterface(aEditor);
  if (!msiEditor) return PR_FALSE;
  htmlEditor -> GetTagListManager(getter_AddRefs(tlm));
  if (!tlm) return PR_FALSE;
  ed = static_cast<nsEditor*>(aEditor);

  ed->GetStartNodeAndOffset(sel, getter_AddRefs(startNode), &startOffset);
  if (nsEditorUtils::Acceptable(ed, tlm, startNode))
  {
    return PR_TRUE;  // current position is OK. Do nothing.
  }
  node = startNode;
  offset = startOffset;

  if (!success && forward) {
    success = LookForward(ed, tlm, sel, node, getter_AddRefs(nodeOut), offset);
    if (success) {
      msiEditor->InsertHelperBR(sel);
      return PR_TRUE;  // LookForward has set the selection
    }
  }

  if (!success) {  // either forward is false, or LookForward failed. Now look backward.
//    success = LookBackward(ed, tlm, sel, node, getter_AddRefs(nodeOut), offset);
    success = LookBackward(ed, tlm, sel, node, getter_AddRefs(nodeOut), offset);
    if (success) {
      msiEditor->InsertHelperBR(sel);
      return PR_TRUE;
    }
  }

  if (!success && !forward)  // Looking backward failed, and we haven't tried looking forward yet
  {
    success = LookForward(ed, tlm, sel, node, getter_AddRefs(nodeOut), offset);
    if (success) {
      msiEditor->InsertHelperBR(sel);
      return PR_TRUE;
    }
  }

  //If we get here, we haven't found anything. What to do?  TODO: BBM

  return PR_FALSE;
}



/******************************************************************************
 * utility methods for drag/drop/copy/paste hooks
 *****************************************************************************/

nsresult
nsEditorHookUtils::GetHookEnumeratorFromDocument(nsIDOMDocument *aDoc,
                                                 nsISimpleEnumerator **aResult)
{
  nsCOMPtr<nsIDocument> doc = do_QueryInterface(aDoc);
  if (!doc) return NS_ERROR_FAILURE;

  nsCOMPtr<nsISupports> container = doc->GetContainer();
  nsCOMPtr<nsIDocShell> docShell = do_QueryInterface(container);
  nsCOMPtr<nsIClipboardDragDropHookList> hookObj = do_GetInterface(docShell);
  if (!hookObj) return NS_ERROR_FAILURE;

  return hookObj->GetHookEnumerator(aResult);
}

PRBool
nsEditorHookUtils::DoAllowDragHook(nsIDOMDocument *aDoc, nsIDOMEvent *aDragEvent)
{
  nsCOMPtr<nsISimpleEnumerator> enumerator;
  GetHookEnumeratorFromDocument(aDoc, getter_AddRefs(enumerator));
  if (!enumerator)
    return PR_TRUE;

  PRBool hasMoreHooks = PR_FALSE;
  while (NS_SUCCEEDED(enumerator->HasMoreElements(&hasMoreHooks)) && hasMoreHooks)
  {
    nsCOMPtr<nsISupports> isupp;
    if (NS_FAILED(enumerator->GetNext(getter_AddRefs(isupp))))
      break;

    nsCOMPtr<nsIClipboardDragDropHooks> override = do_QueryInterface(isupp);
    if (override)
    {
      PRBool canDrag = PR_TRUE;
      nsresult hookres = override->AllowStartDrag(aDragEvent, &canDrag);
      NS_ASSERTION(NS_SUCCEEDED(hookres), "hook failure in AllowStartDrag");
      if (!canDrag)
        return PR_FALSE;
    }
  }

  return PR_TRUE;
}

PRBool
nsEditorHookUtils::DoDragHook(nsIDOMDocument *aDoc, nsIDOMEvent *aEvent,
                              nsITransferable *aTrans)
{
  nsCOMPtr<nsISimpleEnumerator> enumerator;
  GetHookEnumeratorFromDocument(aDoc, getter_AddRefs(enumerator));
  if (!enumerator)
    return PR_TRUE;

  PRBool hasMoreHooks = PR_FALSE;
  while (NS_SUCCEEDED(enumerator->HasMoreElements(&hasMoreHooks)) && hasMoreHooks)
  {
    nsCOMPtr<nsISupports> isupp;
    if (NS_FAILED(enumerator->GetNext(getter_AddRefs(isupp))))
      break;

    nsCOMPtr<nsIClipboardDragDropHooks> override = do_QueryInterface(isupp);
    if (override)
    {
      PRBool canInvokeDrag = PR_TRUE;
      nsresult hookResult = override->OnCopyOrDrag(aEvent, aTrans, &canInvokeDrag);
      NS_ASSERTION(NS_SUCCEEDED(hookResult), "hook failure in OnCopyOrDrag");
      if (!canInvokeDrag)
        return PR_FALSE;
    }
  }

  return PR_TRUE;
}

PRBool
nsEditorHookUtils::DoAllowDropHook(nsIDOMDocument *aDoc, nsIDOMEvent *aEvent,
                                   nsIDragSession *aSession)
{
  nsCOMPtr<nsISimpleEnumerator> enumerator;
  GetHookEnumeratorFromDocument(aDoc, getter_AddRefs(enumerator));
  if (!enumerator)
    return PR_TRUE;

  PRBool hasMoreHooks = PR_FALSE;
  while (NS_SUCCEEDED(enumerator->HasMoreElements(&hasMoreHooks)) && hasMoreHooks)
  {
    nsCOMPtr<nsISupports> isupp;
    if (NS_FAILED(enumerator->GetNext(getter_AddRefs(isupp))))
      break;

    nsCOMPtr<nsIClipboardDragDropHooks> override = do_QueryInterface(isupp);
    if (override)
    {
      PRBool allowDrop = PR_TRUE;
      nsresult hookResult = override->AllowDrop(aEvent, aSession, &allowDrop);
      NS_ASSERTION(NS_SUCCEEDED(hookResult), "hook failure in AllowDrop");
      if (!allowDrop)
        return PR_FALSE;
    }
  }

  return PR_TRUE;
}

PRBool
nsEditorHookUtils::DoInsertionHook(nsIDOMDocument *aDoc, nsIDOMEvent *aDropEvent,
                                   nsITransferable *aTrans)
{
  nsCOMPtr<nsISimpleEnumerator> enumerator;
  GetHookEnumeratorFromDocument(aDoc, getter_AddRefs(enumerator));
  if (!enumerator)
    return PR_TRUE;

  PRBool hasMoreHooks = PR_FALSE;
  while (NS_SUCCEEDED(enumerator->HasMoreElements(&hasMoreHooks)) && hasMoreHooks)
  {
    nsCOMPtr<nsISupports> isupp;
    if (NS_FAILED(enumerator->GetNext(getter_AddRefs(isupp))))
      break;

    nsCOMPtr<nsIClipboardDragDropHooks> override = do_QueryInterface(isupp);
    if (override)
    {
      PRBool doInsert = PR_TRUE;
      nsresult hookResult = override->OnPasteOrDrop(aDropEvent, aTrans, &doInsert);
      NS_ASSERTION(NS_SUCCEEDED(hookResult), "hook failure in OnPasteOrDrop");
      if (!doInsert)
        return PR_FALSE;
    }
  }

  return PR_TRUE;
}


PRBool nodeIsWhiteSpace( nsIDOMNode * node, PRUint32 firstindex, PRUint32 lastindex)
/* return whether all the text (or all the text before index or all the text after index) is white space */
{
  // \f\n\r\t\v\ u00A0\u2028\u2029 are the white space characters
  nsAutoString theText;
  nsAutoString text;
  PRUint16 nodeType;
  node->GetNodeType(&nodeType);

//  if(nodeType != nsIDOMNode::TEXT_NODE) return false;
//  get the string from the node
  node->GetNodeValue(theText);
  PRUint32 length = theText.Length();
  if ((PRInt32)firstindex >= 0 && (PRInt32)lastindex >= 0)
    text = Substring(theText, firstindex, lastindex);
  else text = theText;

//  set up the iterators
  nsAString::const_iterator cur, end;

  text.BeginReading(cur);
  text.EndReading(end);

  for (; cur != end; cur++)
  {
    if ((*cur == PRUnichar(' ')) ||
        (*cur == PRUnichar('\f')) ||
        (*cur == PRUnichar('\n')) ||
        (*cur == PRUnichar('\r')) ||
        (*cur == PRUnichar('\t')) ||
        (*cur == PRUnichar('\v')) ||
        (*cur == PRUnichar(0x00A0)) ||
        (*cur == PRUnichar(0x2028)) ||
        (*cur == PRUnichar(0x2029)))
    {}
    else return PR_FALSE;
  }
  return PR_TRUE;
}
