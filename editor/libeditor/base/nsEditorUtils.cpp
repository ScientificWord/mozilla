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

PRBool
nsEditorUtils::JiggleCursor(nsIEditor *aEditor, nsISelection * sel, PRBool isForward)
{
  nsCOMPtr<msiITagListManager> tlm;
  nsCOMPtr<nsIDOMDocumentTraversal> doctrav;
  nsCOMPtr<nsIDOMDocument> doc;
  nsCOMPtr<nsIDOMTreeWalker> tw;
  nsCOMPtr<nsIDOMNode> currentNode;
  nsCOMPtr<nsIDOMNode> parentNode;
  nsCOMPtr<nsIDOMNode> startSearchNode;
  nsCOMPtr<nsIDOMNode> startNode;
  nsEditor * ed = static_cast<nsEditor*>(aEditor);
  PRInt32 startOffset;
  nsresult res;
  nsCOMPtr<nsIDOMNodeList> childList;    
  res = aEditor->GetDocument(getter_AddRefs(doc));
  if (doc) doctrav = do_QueryInterface(doc);
  else return PR_FALSE;
  nsCOMPtr<nsIHTMLEditor> htmlEditor = do_QueryInterface(aEditor);
  if (!htmlEditor) return PR_FALSE;
  PRBool canTakeText = PR_FALSE;
  nsIAtom * nsatom = nsnull;
  nsString text = NS_LITERAL_STRING("#text");
  nsCOMPtr<nsIDOMHTMLElement> bodyElement;
  nsCOMPtr<nsIDOMHTMLDocument> htmlDoc = do_QueryInterface(doc);
  if (!htmlDoc) return PR_FALSE;
  nsAutoString name;
  ed->GetStartNodeAndOffset(sel, address_of(startNode), &startOffset); 
  htmlDoc->GetBody(getter_AddRefs(bodyElement));
  if (htmlEditor) {
    htmlEditor -> GetTagListManager(getter_AddRefs(tlm));
    if (ed->IsTextNode(startNode))
      startNode->GetParentNode(getter_AddRefs(parentNode));
    else
      parentNode = startNode;

    tlm -> NodeCanContainTag(parentNode, text, nsatom, &canTakeText);
    if (!canTakeText) {
      // we must move the cursor to a place that can accept text.
      res = doctrav->CreateTreeWalker( bodyElement, nsIDOMNodeFilter::SHOW_ELEMENT | nsIDOMNodeFilter::SHOW_TEXT, nsnull, PR_FALSE, getter_AddRefs(tw));
      // find where to start the search
      if (ed->IsTextNode(startNode)) startSearchNode = startNode;
      else {
        res = startNode->GetChildNodes(getter_AddRefs(childList)); 
        childList->Item(startOffset  + (isForward? 0 : -1), getter_AddRefs(startSearchNode));  
      }
      tw->SetCurrentNode(startSearchNode);
      currentNode = startSearchNode;
      while (currentNode)
      {
//          nsAutoString tagname;
//          res = currentNode->GetNodeName(tagname);
        if (ed->IsTextNode(currentNode)) {
          res = currentNode->GetParentNode(getter_AddRefs(parentNode));
          res = tlm->NodeCanContainTag( parentNode, text, nsatom, &canTakeText);
        }
        else
          res = tlm->NodeCanContainTag( currentNode, text, nsatom, &canTakeText);
        if (canTakeText) // this is where we want the cursor to go
        {
          if (ed->IsTextNode(currentNode)) {
            if (isForward) {
              sel->Collapse(currentNode, 0);
            } else {
              nsAutoString nodeText;
              currentNode->GetNodeValue(nodeText);
              sel->Collapse(currentNode, nodeText.Length());
            }
            return PR_TRUE;
          }
          if (isForward) {
            startOffset = 0;
          }
          else {
            PRUint32 len;
            currentNode->GetChildNodes(getter_AddRefs(childList));   
            //Want to explicitly check the DOM children (rather than the frame ones); if we don't have an image or plot
            //  as a direct DOM child, we'll answer PR_FALSE.
            childList->GetLength(&len);
            startOffset = len;
            nsCOMPtr<nsIDOMNode> maybeBreak;
            childList->Item(startOffset-1, getter_AddRefs(maybeBreak));
            if (maybeBreak) {
              maybeBreak->GetNodeName(name);
              if (name.EqualsLiteral("br")) {
                startOffset -= 1;
              }
            }
          }
          sel->Collapse(currentNode, startOffset);
          return PR_TRUE;
        }
        if (isForward)
          tw->NextNode(getter_AddRefs(currentNode));
        else
          tw->PreviousNode(getter_AddRefs(currentNode));
      }
    }
  }  
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
