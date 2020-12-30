/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-831basic-offset: 2 -*- */
/* vim: set ts=2 sw=2 et tw=78: */
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
#include "nsICaret.h"


#include "nsHTMLEditor.h"
#include "nsHTMLEditRules.h"
#include "nsTextEditUtils.h"
#include "nsHTMLEditUtils.h"
#include "nsWSRunObject.h"
#include "nsIStandardURL.h"

#include "nsEditorEventListeners.h"

#include "nsIDOMText.h"
#include "nsIDOMNode.h"
#include "nsIDOMNodeList.h"
#include "nsIDOMDocument.h"
#include "nsIDOMAttr.h"
#include "nsIDocument.h"
#include "nsIDOMEventTarget.h"
#include "nsIDOMNSEvent.h"
#include "nsIDOMKeyEvent.h"
#include "nsIDOMKeyListener.h"
#include "nsIDOMMouseListener.h"
#include "nsIDOMMouseEvent.h"
#include "nsIDOMComment.h"
#include "nsIDOMNamedNodeMap.h"
#include "nsISelection.h"
#include "nsISelectionPrivate.h"
#include "nsIDOMHTMLHtmlElement.h"
#include "nsIDOMHTMLAnchorElement.h"
#include "nsIDOMHTMLImageElement.h"
#include "nsISelectionController.h"
#include "nsIFileChannel.h"
#include "nsIFileURL.h"
#include "nsIController.h"
#include "nsIControllers.h"
#include "nsIDOMXULCommandDispatcher.h"
#include "nsIDOMXULElement.h"
#include "nsICSSLoader.h"
#include "nsICSSStyleSheet.h"
#include "nsIDocumentObserver.h"
#include "nsIDocumentStateListener.h"
#include "nsIDOMWindowInternal.h"

#include "nsIEnumerator.h"
#include "nsIContent.h"
#include "nsIContentIterator.h"
#include "nsIDOMRange.h"
#include "nsIDOMNSRange.h"
#include "nsCOMArray.h"
#include "nsVoidArray.h"
#include "nsIFile.h"
#include "nsIURL.h"
#include "nsIComponentManager.h"
#include "nsIServiceManager.h"
#include "nsIDocumentEncoder.h"
#include "nsIDOMDocumentFragment.h"
#include "nsIDOMNSHTMLElement.h"
#include "nsIPresShell.h"
#include "nsPresContext.h"
#include "nsIParser.h"
#include "nsParserCIID.h"
#include "nsIImage.h"
#include "nsAOLCiter.h"
#include "nsInternetCiter.h"
#include "nsXPCOM.h"
#include "nsISupportsPrimitives.h"
#include "nsLinebreakConverter.h"
#include "nsIFragmentContentSink.h"
#include "nsIContentSink.h"

// netwerk
#include "nsIURI.h"
#include "nsNetUtil.h"

// Drag & Drop, Clipboard
#include "nsIClipboard.h"
#include "nsITransferable.h"
#include "nsIDragService.h"
#include "nsIDOMNSUIEvent.h"
#include "nsIOutputStream.h"
#include "nsIInputStream.h"
#include "nsIStorageStream.h"
#include "nsDirectoryServiceDefs.h"

// for relativization
#include "nsUnicharUtils.h"
#include "nsIDOMDocumentTraversal.h"
#include "nsIDOMTreeWalker.h"
#include "nsIDOMNodeFilter.h"
#include "nsIDOMHTMLLinkElement.h"
#include "nsIDOMHTMLObjectElement.h"
#include "nsIDOMHTMLFrameElement.h"
#include "nsIDOMHTMLIFrameElement.h"
#include "nsIDOMHTMLInputElement.h"
#include "nsIDOMHTMLScriptElement.h"
#include "nsIDOMHTMLEmbedElement.h"
#include "nsIDOMHTMLTableCellElement.h"
#include "nsIDOMHTMLTableRowElement.h"
#include "nsIDOMHTMLTableElement.h"
#include "nsIDOMHTMLBodyElement.h"
#include "nsIDOM3Node.h"
#include "nsIDOMWindow.h"
#include "nsIDOMDocumentView.h"
#include "nsIDOMAbstractView.h"
// Misc
#include "TextEditorTest.h"
#include "nsEditorUtils.h"
#include "nsIPrefBranch.h"
#include "nsIPrefService.h"
#include "nsIContentFilter.h"
#include "nsEventDispatcher.h"
#include "nsEscape.h"
#include "nsIWindowWatcher.h"
#include "../../msiediting/src/msiUtils.h"
#include "../../../content/base/src/nsAttrName.h"
#include "../../msiediting/src/msiEditingAtoms.h"
#include "msiIMathMLEditor.h"
// #include "jcsDumpNode.h"

nsString staticOutput;
#define DEBUG_barry 1
extern "C" {
void DumpNode(nsIDOMNode *aNode, PRInt32 indent = 2, bool recurse = true, nsAString& output = staticOutput);
}
const PRUnichar nbsp = 160;

static NS_DEFINE_CID(kCParserCID,     NS_PARSER_CID);

// private clipboard data flavors for html copy/paste
#define kHTMLContext   "text/_moz_htmlcontext"
#define kHTMLInfo      "text/_moz_htmlinfo"
#define kInsertCookie  "_moz_Insert Here_moz_"
#define kURLPrivateMime  "text/x-moz-url-priv"   // same as kURLDataMime but for private uses
#define NS_FOUND_TARGET NS_ERROR_GENERATE_FAILURE(NS_ERROR_MODULE_EDITOR, 3)

// some little helpers
static PRInt32 FindPositiveIntegerAfterString(const char *aLeadingString, nsCString &aCStr);
static nsresult RemoveFragComments(nsCString &theStr);
static void RemoveBodyAndHead(nsIDOMNode *aNode);
static nsresult FindTargetNode(nsIDOMNode *aStart, nsCOMPtr<nsIDOMNode> &aResult);

#if DEBUG_barry || DEBUG_Barry
nsString DebTagName;
void DebExamineNode(nsIDOMNode * aNode)
{
  if (aNode) nsEditor::GetTagString(aNode, DebTagName);
  else DebTagName = NS_LITERAL_STRING("Null");
}

void dumpTagStack( nsAutoTArray<nsAutoString, 32> sa)
{
  printf("Tag Stack: \n");
  int L = sa.Length();
  for (int i=0; i < L; i++) printf("   %S\n",sa[i].get());
}
#endif

nsCOMPtr<nsIDOMNode> nsHTMLEditor::GetListParent(nsIDOMNode* aNode)
{
  if (!aNode) return nsnull;
  nsCOMPtr<nsIDOMNode> parent, tmp;
  aNode->GetParentNode(getter_AddRefs(parent));
  while (parent)
  {
    if (nsHTMLEditUtils::IsList(parent, mtagListManager)) return parent;
    parent->GetParentNode(getter_AddRefs(tmp));
    parent = tmp;
  }
  return nsnull;
}

nsCOMPtr<nsIDOMNode> nsHTMLEditor::GetMathParent(nsIDOMNode* aNode)
{
  if (!aNode) return nsnull;
  nsCOMPtr<nsIDOMNode> parent, tmp;
  aNode->GetParentNode(getter_AddRefs(parent));
  while (parent)
  {
    if (nsHTMLEditUtils::IsMathNode(parent)) return parent;
    parent->GetParentNode(getter_AddRefs(tmp));
    parent = tmp;
  }
  return nsnull;
}

nsCOMPtr<nsIDOMNode> nsHTMLEditor::GetTableParent(nsIDOMNode* aNode)
{
  if (!aNode) return nsnull;
  nsCOMPtr<nsIDOMNode> parent, tmp;
  aNode->GetParentNode(getter_AddRefs(parent));
  while (parent)
  {
    if (nsHTMLEditUtils::IsTable(parent, mtagListManager)) return parent;
    parent->GetParentNode(getter_AddRefs(tmp));
    parent = tmp;
  }
  return nsnull;
}


NS_IMETHODIMP nsHTMLEditor::LoadHTML(const nsAString & aInputString)
{
  if (!mRules) return NS_ERROR_NOT_INITIALIZED;
  nsAutoTxnsConserveSelection dontSpazMySelection(this);
  // force IME commit; set up rules sniffing and batching
  ForceCompositionEnd();
  nsAutoEditBatch beginBatching(this);
  nsAutoRules beginRulesSniffing(this, kOpLoadHTML, nsIEditor::eNext);

  // Get selection
  nsCOMPtr<nsISelection>selection;
  nsresult res = GetSelection(getter_AddRefs(selection));
  if (NS_FAILED(res)) return res;

  nsTextRulesInfo ruleInfo(nsTextEditRules::kLoadHTML);
  PRBool cancel, handled;
  res = mRules->WillDoAction(selection, &ruleInfo, &cancel, &handled);
  if (NS_FAILED(res)) return res;
  if (cancel) return NS_OK; // rules canceled the operation
  if (!handled)
  {
    PRBool isCollapsed;
    res = selection->GetIsCollapsed(&isCollapsed);
    if (NS_FAILED(res)) return res;

    // Delete Selection, but only if it isn't collapsed, see bug #106269
    if (!isCollapsed)
    {
      res = DeleteSelection(eNone);
      if (NS_FAILED(res)) return res;
    }

    // Get the first range in the selection, for context:
    nsCOMPtr<nsIDOMRange> range;
    res = selection->GetRangeAt(0, getter_AddRefs(range));
    NS_ENSURE_SUCCESS(res, res);
    if (!range)
      return NS_ERROR_NULL_POINTER;
    nsCOMPtr<nsIDOMNSRange> nsrange (do_QueryInterface(range));
    if (!nsrange)
      return NS_ERROR_NO_INTERFACE;

    // create fragment for pasted html
    nsCOMPtr<nsIDOMDocumentFragment> docfrag;
    {
      res = nsrange->CreateContextualFragment(aInputString, getter_AddRefs(docfrag));
      if (!docfrag) return NS_ERROR_NULL_POINTER;
      DumpNode(docfrag);
      NS_ENSURE_SUCCESS(res, res);
    }
    // put the fragment into the document
    nsCOMPtr<nsIDOMNode> parent, junk;
    res = range->GetStartContainer(getter_AddRefs(parent));
    NS_ENSURE_SUCCESS(res, res);
    if (!parent)
      return NS_ERROR_NULL_POINTER;
    PRInt32 childOffset;
    res = range->GetStartOffset(&childOffset);
    NS_ENSURE_SUCCESS(res, res);

    nsCOMPtr<nsIDOMNode> nodeToInsert;
    docfrag->GetFirstChild(getter_AddRefs(nodeToInsert));
    while (nodeToInsert)
    {
      res = InsertNode(nodeToInsert, parent, childOffset++);
      NS_ENSURE_SUCCESS(res, res);
      docfrag->GetFirstChild(getter_AddRefs(nodeToInsert));
    }
  }

  return mRules->DidDoAction(selection, &ruleInfo, res);
}


NS_IMETHODIMP nsHTMLEditor::InsertHTML(const nsAString & aInString)
{
  const nsAFlatString& empty = EmptyString();

  return InsertHTMLWithContext(aInString, empty, empty, empty,
                               nsnull,  nsnull, 0, PR_TRUE);
}

nsresult nsHTMLEditor::NodeContainsOnlyMn(nsIDOMNode * curNode, nsIDOMNode ** textNode)
{
  nsCOMPtr<nsIDOMNode> node = curNode;
  nsCOMPtr<nsIDOMNode> retNode;
  node->Normalize();
  PRUint32 childCount;
  nsAutoString nodeName;
  nsresult res;
  nsCOMPtr<nsIDOMNodeList> nodeList;
  while (node)
  {
    res = GetTagString(node, nodeName);
    node->GetChildNodes(getter_AddRefs(nodeList));
    res = nodeList->GetLength(&childCount);
    if (childCount != 1)
    {
      *textNode = nsnull;
      return NS_OK;
    }
    if (nodeName.EqualsLiteral("mn"))
    {
      node->GetFirstChild(getter_AddRefs(retNode));
      *textNode = retNode;
      return NS_OK;
    }
    node->GetFirstChild(getter_AddRefs(node));
  }
  *textNode = retNode;
  return NS_OK;
}

/**
 * @brief      InsertMathNode. This function does the checking required for inserting a math node (the complicated case is when inserting math in math.)
 *             Some of the rules are: text can't be inserted directly; it must be in an mi, mo, or mn. Nothing but text can go in the mi, mo, or mn. Some
 *             tags have a fixed number of children, so adding a node may require constructing an mrow, etc.
 *
 * @param      insertThisNode   The node to insert
 * @param      dstNode          The parent          This need not be in the current document. We call this function to fill out a subtree before adding
 *                                                   it to the document
 * @param      dstOffset        The offset at which to insert.
 * @param      bDidInsert       Did we insert?      Possibly deprecated
 * @param      lastInsertNode   The last insert node  ""
 *
 * @return     { description_of_the_return_value }
 */

NS_IMETHODIMP
nsHTMLEditor::InsertMathNode
( nsIDOMNode * insertThisNode,
  nsIDOMNode * dstNode,
  PRInt32 dstOffset,
  PRBool bDidInsert,
  nsIDOMNode ** lastInsertNode)
{
  nsresult res;

  nsAutoString strTempTarget;
  nsAutoString tagName;
  nsAutoString nameOfInsert;
  nsAutoString nameOfTarget;
  nsAutoString nameOfParent;
  nsCOMPtr<nsISelection> sel;
  nsCOMPtr<nsIDOMNode> textNode;
  nsCOMPtr<nsIDOMNode> parentNode(dstNode);
  nsCOMPtr<nsIDOMNode> newParentNode = parentNode;
  nsCOMPtr<nsIDOMNode> nodeToInsert;
  nsCOMPtr<nsIDOMNode> nodeTarget;
  nsCOMPtr<nsIDOMNode> nodeParent;
  nsCOMPtr<nsIDOMNode> nodeToSplit;
  nsCOMPtr<nsIDOMNode> nodeSplitLeft;
  nsCOMPtr<nsIDOMNode> nodeSplitRight;
  nsCOMPtr<nsIDOMNode> cursorNode;
  nsCOMPtr<nsIDOMElement> elementTarget;

  PRBool isTempInput = PR_FALSE;
  PRBool isMi = PR_FALSE;
  nsCOMPtr<nsIDOMNode> grandParent;
  nsCOMPtr<nsIDOMCharacterData> characterNode;
  PRUint16 nodeType;
  PRInt32 savedOffset = dstOffset;
  PRInt32 offsetTarget;
  PRInt32 offsetParent;
  PRInt32 newOffset;
  PRInt32 cursorOffset;

  res = insertThisNode->GetNodeType(&nodeType);
  if (nodeType == nsIDOMNode::TEXT_NODE) {
    // We can't insert plain text, only an mi, an mn, an mo or a mtext. This should happen only when pasting, so insertThisNode should
    // have a parent
    res = insertThisNode->GetParentNode(getter_AddRefs(nodeToInsert));
if (!nodeToInsert) return NS_ERROR_INVALID_ARG;
  }
  else {
    nodeToInsert = insertThisNode;
  }
  // nodeToInsert is now the node we are inserting
  cursorNode = nodeToInsert;
  nsCOMPtr<nsINode> node = do_QueryInterface(nodeToInsert);
  cursorOffset = node->GetChildCount();

  nodeTarget = dstNode;
  offsetTarget = dstOffset;
  res = GetTagString(nodeToInsert, nameOfInsert);
  res = GetTagString(nodeTarget, nameOfTarget);
  if (nameOfTarget.EqualsLiteral("#text")) {
    res = GetNodeLocation(nodeTarget, address_of(nodeTarget), &offsetTarget);
  }
  res = GetTagString(nodeTarget, nameOfTarget);
  if (nameOfTarget.EqualsLiteral("mi")) {
    elementTarget = do_QueryInterface(nodeTarget);
    res = elementTarget->GetAttribute(NS_LITERAL_STRING("tempinput"), strTempTarget);
    if (strTempTarget.EqualsLiteral("true")) {
      isTempInput = PR_TRUE;
    }
  }
  if (!isTempInput) {
    nodeTarget = dstNode;
    offsetTarget = dstOffset;
    elementTarget = do_QueryInterface(nodeTarget);
    res = GetTagString(nodeTarget, nameOfTarget);
  }
  // We need to see if nodeToInsert can go into nodeTarget. Perhaps this should be a function in TagListManager

  if ((isMi = nameOfTarget.EqualsLiteral("mi")) || nameOfTarget.EqualsLiteral("#text") || nameOfTarget.EqualsLiteral("mo") || nameOfTarget.EqualsLiteral("mn") ||
      nameOfTarget.EqualsLiteral("mtext"))
  {
    if (isMi) {   // check for tempinput
      res = elementTarget->GetAttribute(NS_LITERAL_STRING("tempinput"), strTempTarget);
      if (strTempTarget.EqualsLiteral("true"))
      {
        // delete the tempinput mi, reset nodeTarget and offsetTarget, and start again
        res = GetNodeLocation(nodeTarget, address_of(nodeParent), &offsetParent);
        res = DeleteNode(nodeTarget);
        nodeTarget = nodeParent;
        offsetTarget = offsetParent;
        elementTarget = do_QueryInterface(nodeTarget);
        res = GetTagString(nodeTarget, nameOfTarget);
      }
    }
    nodeParent = nodeTarget;
    res = GetTagString(nodeParent, nameOfParent);
    if (nameOfParent.EqualsLiteral("mo") || nameOfParent.EqualsLiteral("mi")) {
      res = GetNodeLocation(nodeTarget, address_of(nodeParent), &offsetParent);
      nodeTarget = nodeParent;
      if (offsetTarget > 0) offsetTarget = 1;
      else offsetTarget = 0;
      offsetTarget += offsetParent;
      res = GetTagString(nodeTarget, nameOfTarget);
    }

    while (nodeParent &&(nameOfParent.EqualsLiteral("mi") || nameOfParent.EqualsLiteral("#text") || nameOfParent.EqualsLiteral("mo") || nameOfParent.EqualsLiteral("mn") || nameOfParent.EqualsLiteral("mtext")))
    {
      nodeToSplit = nodeParent;
      nodeToSplit->GetParentNode(getter_AddRefs(nodeParent));
      res = GetTagString(nodeParent, nameOfParent);
    }
    // if nodeParent != nodeTarget, then we need to split some tags to put our new node where it can go

    if (nodeToSplit && (nodeToSplit != nodeTarget)) {
      res = SplitNodeDeep(nodeToSplit, nodeTarget, offsetTarget, &newOffset, PR_TRUE, address_of(nodeSplitLeft), address_of(nodeSplitRight));
      // now we know we can insert at nodeParent, newOffset, so we update stuff:
      nodeTarget = nodeParent;
      offsetTarget = newOffset;
      // Check to see if we need to put in mrow to keep the number of children of nodeTarget the same; check by name
      nsAutoString name;
      nsCOMPtr<nsIDOMElement> mrow;
      PRInt32 offs = 0;
      GetTagString(nodeTarget, name);
      if ((name.EqualsLiteral("mfrac") || name.EqualsLiteral("mover") || name.EqualsLiteral("mprescripts") || name.EqualsLiteral("mroot")
       || name.EqualsLiteral("msqrt") || name.EqualsLiteral("msub") || name.EqualsLiteral("msubsup") || name.EqualsLiteral("msup")
       || name.EqualsLiteral("munder") || name.EqualsLiteral("munderover")) && (nodeSplitLeft || nodeSplitRight))
      {
        GetTagString(nodeToInsert, name);
        if (!name.EqualsLiteral("mrow")) { // make an mrow to hold the new stuff
          msiUtils::CreateMRow(this, nodeToInsert, mrow);
          nodeToInsert = mrow;
          cursorNode = mrow;
        }
        if (nodeSplitLeft) {
          InsertNodeAtPoint(nodeSplitLeft, (nsIDOMNode **)address_of(nodeToInsert), &offs, PR_TRUE);
          offsetTarget--; // we have moved a direct descendent of nodeTarget
        }
        node = do_QueryInterface(nodeToInsert);
        cursorOffset = node->GetChildCount();
        if (nodeSplitRight) {
          InsertNodeAtPoint(nodeSplitRight, (nsIDOMNode **)address_of(nodeToInsert), &offs, PR_TRUE);
        }
      }
      elementTarget = do_QueryInterface(nodeTarget);
      res = GetTagString(nodeTarget, nameOfTarget);
    }
  }
  res = InsertNodeAtPoint(nodeToInsert, (nsIDOMNode **)address_of(nodeTarget), &offsetTarget, PR_TRUE);
  if (lastInsertNode) *lastInsertNode = nodeToInsert;
  res = GetSelection(getter_AddRefs(sel));
  sel->Collapse(cursorNode, cursorOffset);
  return res;
}

// TrimDocFragment. When mathematics is copied, the docfragment for it contains non-math tags from
// the context. These need to be trimmed when copying into math. We do that using the nsIDOMnode interface
// since the editor methods mess with the selection and assumes we are working with part of the
// main document. Returns true if it modifies the fragment.

PRBool TrimDocFragment( nsIDOMNode * fragmentAsNode) {
  PRBool fReturn = PR_FALSE;
  nsCOMPtr<nsIDOMNode> childNode;
  nsCOMPtr<nsIDOMNode> grandchildNode;
  nsCOMPtr<nsIDOMNode> tempChildNode;
  nsCOMPtr<nsIDOMNode> tempGrandchildNode;
  nsCOMPtr<nsIDOMNode> dummyNode;
  PRBool fRepeat = PR_TRUE;
  while (fRepeat) {
    fRepeat = PR_FALSE;
    fragmentAsNode->GetFirstChild(getter_AddRefs(childNode));
    while (childNode) {
      childNode->GetNextSibling(getter_AddRefs(tempChildNode));
      if (childNode && !(nsHTMLEditUtils::IsMath(childNode)))
      {
        // RemoveContainer(childNode);
        childNode->GetFirstChild(getter_AddRefs(grandchildNode));
        while (grandchildNode) {
          childNode->RemoveChild(grandchildNode, getter_AddRefs(tempGrandchildNode));
          fragmentAsNode->InsertBefore(tempGrandchildNode, childNode, getter_AddRefs(dummyNode));
          childNode->GetFirstChild(getter_AddRefs(grandchildNode));
        }
       fRepeat = PR_TRUE;  // if we changed a node, we have to repeat this process again,
                           // starting at the root
       fReturn = PR_TRUE;
       fragmentAsNode->RemoveChild(childNode, getter_AddRefs(dummyNode));
      }
      fragmentAsNode->GetFirstChild(getter_AddRefs(childNode));

      childNode = tempChildNode;
    }
  }
  return fReturn;
}

/*  // Since it is possible to copy parts of mathematics we have to check that the copied text
  // is still valid MathML. We need to check that certain tags which require a fixed number of 
  // children (such as msub, msup and mfrac) which require 2 children) actually have that many in the
  // fragment. If not, we replace the offending tag with an mrow.

  res = z( fragmentAsNode); */

PRUint32 MathChildCount( nsIDOMElement * aElement) {
  nsCOMPtr<nsIDOMNodeList> children;
  nsCOMPtr<nsIDOMNode> child;
  PRUint32 childCount;
  PRUint32 mathChildCount;
  PRInt32 index;
  PRUint16 type;
  aElement->GetChildNodes(getter_AddRefs(children));
  children->GetLength(&childCount);
  mathChildCount = childCount;
  for (index = 0; index < childCount; index++) {
    children->Item(index, getter_AddRefs(child));
    child->GetNodeType(&type);
    if (type == nsIDOMNode::TEXT_NODE)
      mathChildCount--;    
  }
  return mathChildCount;
}

nsresult nsHTMLEditor::ValidateMathSyntax(  nsIDOMElement * mathNode ) {

  nsCOMPtr<nsIDOMElement> node = mathNode;
  nsCOMPtr<nsIDOMNode> mathrow;
  nsCOMPtr<nsIDOMElement> nextNode;
  nsCOMPtr<nsIDOMElement> childElement;
  nsCOMPtr<nsIDOMNode> child;
  nsCOMPtr<nsIDOMNode> parent;
  nsCOMPtr<nsIDOMNodeList> children;
  PRUint32 childCount;
  PRUint32 index;
  PRBool needToSubstitute;
  nsAutoString tagName, parentTagName;
  needToSubstitute = PR_FALSE;
  childCount = 0;
  if (node) {
    node->GetChildNodes(getter_AddRefs(children));
    children->GetLength(&childCount);
    for (index = 0; index < childCount; index++) {
      children->Item(index, getter_AddRefs(child));
      childElement = do_QueryInterface(child);
      if (childElement) {
        ValidateMathSyntax(childElement);
      }
    } 
  }      
  if (nsHTMLEditUtils::IsMath(node)) {
    node->GetLocalName(tagName);
    if (   tagName.EqualsLiteral("mfrac")
        || tagName.EqualsLiteral("mroot")
        || tagName.EqualsLiteral("msub") 
        || tagName.EqualsLiteral("msup")
        || tagName.EqualsLiteral("munder")
        || tagName.EqualsLiteral("mover")
        || tagName.EqualsLiteral("msubsup") 
        || tagName.EqualsLiteral("munderover")) {
      node->GetChildNodes(getter_AddRefs(children));
      if (children) {
        childCount = MathChildCount( node );
        if (  tagName.EqualsLiteral("msubsup") 
           || tagName.EqualsLiteral("munderover"))  {
          if (childCount != 3) needToSubstitute = PR_TRUE;
        } else // tagname is one of the others in the above list
        {
          if (childCount != 2) {
            needToSubstitute = PR_TRUE;
          }
        }
      // replace node with mrow.
        if (needToSubstitute) {
          node->GetParentNode(getter_AddRefs(parent));
          parent->GetLocalName(parentTagName);
          if (parentTagName.EqualsLiteral("math") || parentTagName.EqualsLiteral("mrow")) {
            RemoveContainer(node);  // avoid mrow immediately in math or mrow tag
          }
          else
          {
            ReplaceContainer( node, NS_LITERAL_STRING("mrow"), mtagListManager, EmptyString(), 
              EmptyString(), PR_TRUE, getter_AddRefs(mathrow));
          } 
        }
        // We need to recurse to check the validity of the mrow children
      }
    } 
  }    
  return NS_OK;
}


nsresult
nsHTMLEditor::InsertHTMLWithContext(const nsAString & aInputString,
                                    const nsAString & aContextStr,
                                    const nsAString & aInfoStr,
                                    const nsAString & aFlavor,
                                    nsIDOMDocument *aSourceDoc,
                                    nsIDOMNode *aDestNode,
                                    PRInt32 aDestOffset,
                                    PRBool aDeleteSelection)
{
  if (!mRules) return NS_ERROR_NOT_INITIALIZED;

  // force IME commit; set up rules sniffing and batching
  ForceCompositionEnd();
  nsAutoEditBatch beginBatching(this);
  nsAutoRules beginRulesSniffing(this, kOpHTMLPaste, nsIEditor::eNext);
  nsAutoString tagName;
  nsAutoString output;

  // Get selection
  nsresult res;
  nsCOMPtr<nsISelection>selection;
  res = GetSelection(getter_AddRefs(selection));
  //printf("\n\nStarting selection\n");
  //DumpSelection(selection);

  if (NS_FAILED(res)) return res;

  // create a dom document fragment that represents the structure to paste
  nsCOMPtr<nsIDOMNode> fragmentAsNode, streamStartParent, streamEndParent;
  PRInt32 streamStartOffset = 0, streamEndOffset = 0;

  res = CreateDOMFragmentFromPaste(aInputString, aContextStr, aInfoStr,
                                   address_of(fragmentAsNode),
                                   address_of(streamStartParent),
                                   address_of(streamEndParent),
                                   &streamStartOffset,
                                   &streamEndOffset);

  NS_ENSURE_SUCCESS(res, res);
  nsCOMPtr<nsIDOMNode> targetNode, tempNode;
  PRInt32 targetOffset=0;
  // fragmentAsNode->Normalize();

  if (!aDestNode)
  {
    // if caller didn't provide the destination/target node,
    // fetch the paste insertion point from our selection
    res = GetStartNodeAndOffset(selection, getter_AddRefs(targetNode), &targetOffset);
    if (!targetNode) res = NS_ERROR_FAILURE;
    if (NS_FAILED(res)) return res;
  }
  else
  {
    targetNode = aDestNode;
    targetOffset = aDestOffset;
  }

// New code: don't put non-math inside of <math>, even if there is an <msidisplay>
  // PRBool isMath = nsHTMLEditUtils::IsMath(targetNode);
  // PRBool fTrimmedFragment = nsnull;
  // PRBool useNewCode = PR_TRUE;
  // if (useNewCode && isMath)
  // {
  //   fTrimmedFragment = TrimDocFragment(fragmentAsNode);
  // }

  PRBool doContinue = PR_TRUE;

  res = DoContentFilterCallback(aFlavor, aSourceDoc, aDeleteSelection,
                                (nsIDOMNode **)address_of(fragmentAsNode),
                                (nsIDOMNode **)address_of(streamStartParent),
                                &streamStartOffset,
                                (nsIDOMNode **)address_of(streamEndParent),
                                &streamEndOffset,
                                (nsIDOMNode **)address_of(targetNode),
                                &targetOffset, &doContinue);

#ifdef DEBUG
  printf("Out of DoContentFilterCallback\n");
  DumpNode((nsIDOMNode*)fragmentAsNode);
  // printf(NS_ConvertUTF16toUTF8(output).get());
#endif
  NS_ENSURE_SUCCESS(res, res);
  if (!doContinue)
    return NS_OK;

  // If there's only one top-level node and it's an <svg>, we want to replace it
  // with an <object>:

  if (fragmentAsNode)
  {
    PRUint32 numKids;
    PRInt32 svgIndex = -1;
    PRBool hasOtherContent = PR_FALSE;
    nsCOMPtr<nsIDOMNodeList> kidNodes;
    res = fragmentAsNode->GetChildNodes(getter_AddRefs(kidNodes));
    if (NS_SUCCEEDED(res) && kidNodes)
      res = kidNodes->GetLength(&numKids);
    nsCOMPtr<nsIDOMNode> theKid;
    nsCOMPtr<nsINode> theKidNode;
    nsAutoString kidName;
    for (PRInt32 ii = 0; !hasOtherContent && (ii < numKids); ++ii)
    {
      res = kidNodes->Item(ii, getter_AddRefs(theKid));
      if (NS_SUCCEEDED(res))
      {
        theKidNode = do_QueryInterface(theKid);
        if (theKidNode->IsNodeOfType(nsINode::eSVG))
        {
          theKid->GetLocalName(kidName);
          if (kidName.LowerCaseEqualsLiteral("svg"))
            svgIndex = ii;
          else
            hasOtherContent = PR_TRUE;
        }
        else if (!nodeIsWhiteSpace(theKid, -1, -1))
          hasOtherContent = PR_TRUE;
      }
    }
    if (!hasOtherContent && (svgIndex >= 0))
    {
      //Want to replace entire contents - that is, the <svg> node - by an object, and put the string
      //  representation into an external file to be its data:
      nsCOMPtr<nsILocalFile> fileToUse;
      nsresult rv = GetDocumentGraphicsDir(getter_AddRefs(fileToUse));
      NS_ENSURE_SUCCESS(rv, rv);

      fileToUse->Append(NS_LITERAL_STRING("msi-screenshot-svg.svg"));
      nsCOMPtr<nsILocalFile> path = do_QueryInterface(fileToUse);
      path->CreateUnique(nsIFile::NORMAL_FILE_TYPE, 0755);

      nsCOMPtr<nsIOutputStream> outputStream;
      rv = NS_NewLocalFileOutputStream(getter_AddRefs(outputStream), fileToUse);
      NS_ENSURE_SUCCESS(rv, rv);

      PRUint32 totLength, headerLength, strLength, numWritten;
      nsCOMPtr<nsIOutputStream> bufferedOutputStream;
      NS_NAMED_LITERAL_CSTRING(headerStr, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
      headerLength = headerStr.Length();
      strLength = aInputString.Length();
      totLength = headerLength + strLength;

      rv = NS_NewBufferedOutputStream(getter_AddRefs(bufferedOutputStream), outputStream, totLength);
      NS_ENSURE_SUCCESS(rv, rv);

      const char* headerBuff = headerStr.BeginReading();
      rv = bufferedOutputStream->Write(headerBuff, headerLength, &numWritten);
      NS_ENSURE_SUCCESS(rv, rv);

      rv = bufferedOutputStream->Write(NS_ConvertUTF16toUTF8(aInputString).get(), strLength, &numWritten);
      NS_ENSURE_SUCCESS(rv, rv);

      // force the stream close before we try to insert the image
      // into the document.
      rv = bufferedOutputStream->Close();
      NS_ENSURE_SUCCESS(rv, rv);

      nsAutoString urltext;
      urltext.Append(NS_LITERAL_STRING("graphics/"));
      nsAutoString leafname;
      fileToUse->GetLeafName(leafname);
      urltext.Append(leafname);

      if (!urltext.IsEmpty())
      {
        NS_NAMED_LITERAL_STRING(xmlnsAttr, "http://www.w3.org/1999/xhtml");
        nsCOMPtr<nsIDOMElement> objectElt;
        nsCOMPtr<nsINode> fragNode = do_QueryInterface(fragmentAsNode);
//        rv = CreateElementWithDefaults(NS_LITERAL_STRING("object"), getter_AddRefs(objectElt));
        rv = CreateContentNS(NS_LITERAL_STRING("object"), NS_NewAtom(xmlnsAttr), getter_AddRefs(objectElt));
        SetAttribute(objectElt, NS_LITERAL_STRING("data"), urltext);
        SetAttribute(objectElt, NS_LITERAL_STRING("xmlns"), xmlnsAttr);
        SetAttribute(objectElt, NS_LITERAL_STRING("alt"), EmptyString());
        nsCOMPtr<nsIDOMNode> objNode(do_QueryInterface(objectElt));
        nsCOMPtr<nsIDOMNode> kidNode = GetChildAt(fragmentAsNode, svgIndex);
        nsCOMPtr<nsIDOMNode> ignoreNode;
        ApplyGraphicsDefaults( objectElt, NS_LITERAL_STRING("image") );
        fragmentAsNode->ReplaceChild(objNode, kidNode, getter_AddRefs(ignoreNode));
      }
    }
  }

  // if we have a destination / target node, we want to insert there
  // rather than in place of the selection
  // ignore aDeleteSelection here if no aDestNode since deletion will
  // also occur later; this block is intended to cover the various
  // scenarios where we are dropping in an editor (and may want to delete
  // the selection before collapsing the selection in the new destination)
  PRBool collapsed;
  res = selection->GetIsCollapsed(&collapsed);
  if (aDestNode)
  {
    if (aDeleteSelection && !collapsed)
    {
      // Use an auto tracker so that our drop point is correctly
      // positioned after the delete.
      nsAutoTrackDOMPoint tracker(mRangeUpdater, &targetNode, &targetOffset);
      res = DeleteSelection(eNone);
      NS_ENSURE_SUCCESS(res, res);
    }

    res = selection->Collapse(targetNode, targetOffset);
    NS_ENSURE_SUCCESS(res, res);
  }

  // we need to recalculate various things based on potentially new offsets
  // this is work to be completed at a later date (probably by jfrancis)

  // make a list of what nodes in docFrag we need to move
  PRInt32 j;
  nsCOMArray<nsIDOMNode> nodeList;
  DumpNode(fragmentAsNode);
  // PRBool fUseNewCode = PR_FALSE;
  // if (fUseNewCode && fTrimmedFragment) streamStartParent = nsnull;
  res = CreateListOfNodesToPaste(fragmentAsNode, nodeList,
                                 streamStartParent, streamStartOffset,
                                 streamEndParent, streamEndOffset);
  NS_ENSURE_SUCCESS(res, res);
  PRInt32 listCount = nodeList.Count();
  if (listCount == 0)
    return NS_OK;
 // DumpNode();

  // walk list of nodes; perform surgery on nodes (relativize) with _mozattr
  res = RelativizeURIInFragmentList(nodeList, aFlavor, aSourceDoc, targetNode);
  // ignore results from this call, try to paste/insert anyways

  // are there any table elements in the list?
  // node and offset for insertion
  nsCOMPtr<nsIDOMNode> parentNode;
  PRInt32 offsetOfNewNode;
  // BBM: Why are we not using targetNode and targetOffset? What about when the position is passed in?
  res = GetStartNodeAndOffset(selection, getter_AddRefs(parentNode), &offsetOfNewNode);

  // check for table cell selection mode
  PRBool cellSelectionMode = PR_FALSE;
  nsCOMPtr<nsIDOMElement> cell;
  res = GetFirstSelectedCell(nsnull, getter_AddRefs(cell));
  if (NS_SUCCEEDED(res) && cell)
  {
    cellSelectionMode = PR_TRUE;
    //BBM: we are pasting into a cell.
  }

  if (cellSelectionMode)
  {
    // do we have table content to paste?  If so, we want to delete
    // the selected table cells and replace with new table elements;
    // but if not we want to delete _contents_ of cells and replace
    // with non-table elements.  Use cellSelectionMode bool to
    // indicate results.
    nsIDOMNode* firstNode = nodeList[0];
    if (!nsHTMLEditUtils::IsTableElement(firstNode, mtagListManager))
      cellSelectionMode = PR_FALSE;
  }

  if (!cellSelectionMode)
  {
    PRBool inComplexTransaction;
    GetInComplexTransaction(&inComplexTransaction);
    SetInComplexTransaction(PR_TRUE);
    res = DeleteSelectionAndPrepareToCreateNode(parentNode, offsetOfNewNode);
  //  selection->Collapse(parentNode, offsetOfNewNode);
    NS_ENSURE_SUCCESS(res, res);
    SetInComplexTransaction(inComplexTransaction);


    // pasting does not inherit local inline styles
    // res = RemoveAllInlineProperties();
    // NS_ENSURE_SUCCESS(res, res);
  }
  else
  {
    // delete whole cells: we will replace with new table content
    if (1)
    {
      // Save current selection since DeleteTableCell perturbs it
      nsAutoSelectionReset selectionResetter(selection, this);
      res = DeleteTableCell(1);
      NS_ENSURE_SUCCESS(res, res);
    }
    // collapse selection to beginning of deleted table content
    selection->CollapseToStart();
  }

  // give rules a chance to handle or cancel
  nsTextRulesInfo ruleInfo(nsTextEditRules::kInsertElement);
  PRBool cancel, handled;
  res = mRules->WillDoAction(selection, &ruleInfo, &cancel, &handled);
  if (NS_FAILED(res)) return res;
  if (cancel) return NS_OK; // rules canceled the operation
  if (!handled)
  {
    // The rules code (WillDoAction above) might have changed the selection.
    // refresh our memory...
    res = GetStartNodeAndOffset(selection, getter_AddRefs(parentNode), &offsetOfNewNode);
    if (!parentNode) res = NS_ERROR_FAILURE;
    if (NS_FAILED(res)) return res;

    // Adjust position based on the first node we are going to insert.
    NormalizeEOLInsertPosition(nodeList[0], address_of(parentNode), &offsetOfNewNode);

    // if there are any invisible br's after our insertion point, remove them.
    // this is because if there is a br at end of what we paste, it will make
    // the invisible br visible.
    nsWSRunObject wsObj(this, parentNode, offsetOfNewNode);
    if (nsTextEditUtils::IsBreak(wsObj.mEndReasonNode) &&
        !IsVisBreak(wsObj.mEndReasonNode) )
    {
      res = DeleteNode(wsObj.mEndReasonNode);
      if (NS_FAILED(res)) return res;
    }

    // remember if we are in a link.
    PRBool bStartedInLink = IsInLink(parentNode);

    // are we in a text node or other node that connot accept ?  If so, split it.
    // BBM: Check tag containment rules here?
    if (IsTextNode(parentNode))
    {

        nsCOMPtr<nsIDOMNode> temp;
      // nsCOMPtr<nsIDOMNode> newLeftNode;
      // nsCOMPtr<nsIDOMNode> newRightNode;
      res = SplitNodeDeep(parentNode, parentNode, offsetOfNewNode, &offsetOfNewNode, PR_TRUE);
      // res = SplitNodeDeep(parentNode, parentNode, offsetOfNewNode, &offsetOfNewNode, PR_TRUE,
      //   &newLeftNode, &newRightNode);
      // // upon exit, the insertion point will be in the parent of newLeftNode and newRightNode, between them;
      // // that is the offset will be the index of newLeftNode + 1;
      if (NS_FAILED(res)) return res;
      res = parentNode->GetParentNode(getter_AddRefs(temp));
      if (NS_FAILED(res)) return res;
      parentNode = temp;
    }

    // build up list of parents of first node in list that are either
    // lists or tables or math nodes.  First examine front of paste node list.
    nsCOMArray<nsIDOMNode> startListAndTableArray;
    res = GetListAndTableParents(PR_FALSE, nodeList, startListAndTableArray);
    NS_ENSURE_SUCCESS(res, res);

    // remember number of lists and tables above us
    PRInt32 highWaterMark = -1;
    if (startListAndTableArray.Count() > 0)
    {
      res = DiscoverPartialListsAndTables(nodeList, startListAndTableArray, &highWaterMark);
      NS_ENSURE_SUCCESS(res, res);
    }

    // if we have pieces of tables or lists to be inserted, let's force the paste
    // to deal with table elements right away, so that it doesn't orphan some
    // table or list contents outside the table or list.
    if (highWaterMark >= 0)
    {
      res = ReplaceOrphanedStructure(PR_FALSE, nodeList, startListAndTableArray, highWaterMark);
      NS_ENSURE_SUCCESS(res, res);
    }

    // Now go through the same process again for the end of the paste node list.
    nsCOMArray<nsIDOMNode> endListAndTableArray;
    res = GetListAndTableParents(PR_TRUE, nodeList, endListAndTableArray);
    NS_ENSURE_SUCCESS(res, res);
    highWaterMark = -1;

    // remember number of lists and tables above us
    if (endListAndTableArray.Count() > 0)
    {
      res = DiscoverPartialListsAndTables(nodeList, endListAndTableArray, &highWaterMark);
      NS_ENSURE_SUCCESS(res, res);
    }

    // don't orphan partial list or table structure
    if (highWaterMark >= 0)
    {
      res = ReplaceOrphanedStructure(PR_TRUE, nodeList, endListAndTableArray, highWaterMark);
      NS_ENSURE_SUCCESS(res, res);
    }
// Now do the same for math as we did for tables and list items
    nsCOMPtr<nsIDOMNode> endNode = nodeList[0];
    PRUint32 length = nodeList.Count();
   // nsCOMPtr<nsIDOMNode> mathNode;
    // MathParent( endNode, getter_AddRefs(mathNode));
    // if (mathNode) ReplaceOrphanedMath(PR_FALSE, nodeList, mathNode);
    // // now do the other end.
    // if (length > 1) {
    //   endNode = nodeList[length-1];
    //   MathParent( endNode, getter_AddRefs(mathNode));
    //   if (mathNode) ReplaceOrphanedMath(PR_FALSE, nodeList, mathNode);
    // }
//// Now fix up fragments internal to the math node
////    endNode = nodeList[0];
//// #if DEBUG_barry || DEBUG_Barry
////   printf("\nendNode before FixMath\n");
////   DumpNode(endNode, 0, true);
//// #endif

    FixMathematics(endNode, length > 1, PR_FALSE);
//// #if DEBUG_barry || DEBUG_Barry
////   printf("\nendNode after FixMath\n");
////   DumpNode(endNode, 0, true);
//// #endif
//
    nsCOMPtr<nsIDOMNode> grandParent = parentNode;
    PRInt32 parentOffset = offsetOfNewNode;  // use these values if we don't have to go up to the dstNode
    nsCOMPtr<nsIDOMNode> parentBlock, lastInsertNode, insertedContextParent;

    if (length > 1)
    {

      endNode = nodeList[length-1];
      FixMathematics(endNode, PR_FALSE, PR_TRUE);
    } // Loop over the node list and paste the nodes:
      // However, if we are pasting into mathematics, we need put our entire list into an mrow.
      // We are not allowed to increase the number of children in the dstNode if the dstNode is
      // one of a list of nodes including mfrac, mroot, msqrt, sub, sup, subsup, etc. and we cannot
      // put general math into nodes such as mo, mi, etc.
    if (nsHTMLEditUtils::IsMath(parentNode)) {
      nsCOMPtr<nsIDOMElement> mrow;
      nsCOMPtr<nsIDOMElement>  parentElement;
      nsAutoString name;
      parentNode->GetNodeName(name);
      msiUtils::CreateMRow(this, (nsIDOMNode *)nsnull, mrow);
      PRBool bDidInsert;
      PRBool isTempInput;
      parentElement = do_QueryInterface(parentNode);
      res = parentElement->HasAttribute(NS_LITERAL_STRING("tempinput"), &isTempInput);
      if (name.EqualsLiteral("mo") || name.EqualsLiteral("mn") || (name.EqualsLiteral("mi") && !isTempInput)) {
        GetNodeLocation(parentNode, address_of(grandParent), &parentOffset);
        res = MoveNode(parentNode, mrow, 0);

      }
      res = InsertMathNode( mrow, grandParent, parentOffset, bDidInsert, getter_AddRefs(lastInsertNode));
      parentNode = mrow;
      if (offsetOfNewNode > 0) offsetOfNewNode = 1;
      else offsetOfNewNode = 0;
    }
    listCount = nodeList.Count();
    PRBool bDidInsert = PR_FALSE;
    if (IsBlockNode(parentNode))
      parentBlock = parentNode;
    else
      parentBlock = GetBlockNodeParent(parentNode);

    for (j=0; j<listCount; j++)
    {
      nsCOMPtr<nsIDOMNode> curNode = nodeList[j];


      NS_ENSURE_TRUE(curNode, NS_ERROR_FAILURE);
      NS_ENSURE_TRUE(curNode != fragmentAsNode, NS_ERROR_FAILURE);
      NS_ENSURE_TRUE(!nsTextEditUtils::IsBody(curNode), NS_ERROR_FAILURE);

      if (insertedContextParent)
      {
        // if we had to insert something higher up in the paste hierarchy, we want to
        // skip any further paste nodes that descend from that.  Else we will paste twice.
        if (nsEditorUtils::IsDescendantOf(curNode, insertedContextParent))
          continue;
      }

      // give the user a hand on table element insertion.  if they have
      // a table or table row on the clipboard, and are trying to insert
      // into a table or table row, insert the appropriate children instead.
       if (  (nsHTMLEditUtils::IsTableRow(curNode, mtagListManager) && nsHTMLEditUtils::IsTableRow(parentNode, mtagListManager))
        && (nsHTMLEditUtils::IsTable(curNode, mtagListManager)    || nsHTMLEditUtils::IsTable(parentNode, mtagListManager)) )
      {
        nsCOMPtr<nsIDOMNode> child;
        curNode->GetFirstChild(getter_AddRefs(child));
        while (child)
        {
          printf("Inserting nodes\n");
          res = InsertNodeAtPoint(child, (nsIDOMNode **)address_of(parentNode), &offsetOfNewNode, PR_TRUE);
          if (NS_SUCCEEDED(res))
          {
            bDidInsert = PR_TRUE;
            lastInsertNode = child;
            offsetOfNewNode++;
          }
          curNode->GetFirstChild(getter_AddRefs(child));
        }
      }
      // give the user a hand on list insertion.  if they have
      // a list on the clipboard, and are trying to insert
      // into a list or list item, insert the appropriate children instead,
      // ie, merge the lists instead of pasting in a sublist.
      else if (nsHTMLEditUtils::IsList(curNode, mtagListManager) &&
        (nsHTMLEditUtils::IsList(parentNode, mtagListManager)  || nsHTMLEditUtils::IsListItem(parentNode, mtagListManager)) )
      {
        nsCOMPtr<nsIDOMNode> child, tmp;
        curNode->GetFirstChild(getter_AddRefs(child));
        while (child)
        {
          if (nsHTMLEditUtils::IsListItem(child, mtagListManager) || nsHTMLEditUtils::IsList(child, mtagListManager))
          {
            // check if we are pasting into empty list item. If so
            // delete it and paste into dstNode list instead.
            if (nsHTMLEditUtils::IsListItem(parentNode, mtagListManager))
            {
              PRBool isEmpty;
              res = IsEmptyNode(parentNode, &isEmpty, PR_TRUE, PR_FALSE, PR_FALSE);
              if ((NS_SUCCEEDED(res)) && isEmpty)
              {
                nsCOMPtr<nsIDOMNode> listNode;
                PRInt32 newOffset;
                GetNodeLocation(parentNode, address_of(listNode), &newOffset);
                if (listNode)
                {
                  DeleteNode(parentNode);
                  parentNode = listNode;
                  offsetOfNewNode = newOffset;
                }
              }
            }
            res = InsertNodeAtPoint(child, (nsIDOMNode **)address_of(parentNode), &offsetOfNewNode, PR_TRUE);
            if (NS_SUCCEEDED(res))
            {
              bDidInsert = PR_TRUE;
              lastInsertNode = child;
              offsetOfNewNode++;
            }
          }
          else
          {
            curNode->RemoveChild(child, getter_AddRefs(tmp));
          }
          curNode->GetFirstChild(getter_AddRefs(child));
        }

      }
      // check for pre's going into pre's.
      else if (nsHTMLEditUtils::IsPre(parentBlock, mtagListManager) && nsHTMLEditUtils::IsPre(curNode, mtagListManager))
      {
        nsCOMPtr<nsIDOMNode> child, tmp;
        curNode->GetFirstChild(getter_AddRefs(child));
        while (child)
        {
          res = InsertNodeAtPoint(child, (nsIDOMNode **)address_of(parentNode), &offsetOfNewNode, PR_TRUE);
          if (NS_SUCCEEDED(res))
          {
            bDidInsert = PR_TRUE;
            lastInsertNode = child;
            offsetOfNewNode++;
          }
          curNode->GetFirstChild(getter_AddRefs(child));
        }
      }
      else if ( (nsHTMLEditUtils::IsMath(curNode))  ||
                (nsHTMLEditUtils::IsMath(parentNode)) )
      {
        if ( ! nsHTMLEditUtils::IsMath(curNode) ) {
          // nsCOMPtr<nsIDOM3Node> d3node = do_QueryInterface(curNode);
          // nsCOMPtr<nsISupports> nsEd = do_QueryInterface((nsIHTMLEditor *)this);
          // nsCOMPtr<msiIMathMLEditor> msiEd = do_QueryInterface(nsEd);
          // nsAutoString contents;
          // res = d3node->GetTextContent(contents);
          // res = msiEd->InsertSymbol(contents);
          //  // // putting text into math -- change the text to math and carry on
           // // First wrap in an <mtext>
           // nsCOMPtr<nsIDOMElement> mtextElement;
           // res = msiUtils::CreateMathMLElement(this, msiEditingAtoms::mtext, mtextElement);
           // PRInt32 offset = 0;
           // res = InsertNodeAtPoint(curNode, (nsIDOMNode **)address_of(mtextElement), &offset, PR_TRUE);

           // curNode = mtextElement;
        }

        //putting in math; check to see if it is going into math.
        PRBool parentIsMath = nsHTMLEditUtils::IsMath(parentNode);
        nsCOMPtr<nsIDOMNode> newParentNode;
        nsCOMPtr<nsIDOMNode> sibling;
        nsCOMPtr<nsIDOMNodeList> children;
        nsAutoString name;
        PRUint32 unsignedOffsetOfNewNode;
        PRUint16 type;
        if (!parentIsMath) {
          if (offsetOfNewNode > 0) {
            parentNode->GetChildNodes(getter_AddRefs(children));
            children->Item(offsetOfNewNode - 1, getter_AddRefs(sibling));
          }
          while (sibling) {
            sibling->GetNodeType(&type);
            if (type != nsIDOMNode::TEXT_NODE || !nodeIsWhiteSpace(sibling, -1, -1)) break;
            sibling->GetPreviousSibling(getter_AddRefs(sibling));
          }
          if (sibling) {
            sibling->GetNodeType(&type);
            if (type == nsIDOMNode::ELEMENT_NODE) {
              sibling->GetLocalName(name);
              if (name.EqualsLiteral("math")) {
                parentNode = sibling;
                parentIsMath = PR_TRUE;
                sibling->GetChildNodes(getter_AddRefs(children));
                unsignedOffsetOfNewNode = (PRUint32) offsetOfNewNode;
                children->GetLength(&unsignedOffsetOfNewNode);
                offsetOfNewNode = (PRInt32) unsignedOffsetOfNewNode;
              }
            }
          }
        }
        if (!parentIsMath) {
          if (offsetOfNewNode > 0) {
            parentNode->GetChildNodes(getter_AddRefs(children));
            children->Item(offsetOfNewNode - 1, getter_AddRefs(sibling));
          }
          else parentNode->GetFirstChild(getter_AddRefs(sibling));
          while (sibling && nodeIsWhiteSpace(sibling, -1, -1)) sibling->GetNextSibling(getter_AddRefs(sibling));
          if (sibling) {
            sibling->GetNodeType(&type);
            if (type == nsIDOMNode::ELEMENT_NODE) {
              sibling->GetLocalName(name);
              if (name.EqualsLiteral("math")) {
                parentNode = sibling;
                parentIsMath = PR_TRUE;
                offsetOfNewNode = 0;
              }
            }
          }
        }

        if (parentIsMath)
        {
          nsAutoString name;
          lastInsertNode = nsnull;
          GetTagString(curNode, name);
          if (name.EqualsLiteral("math"))
          {
//            curNode->Normalize();
            nsCOMPtr<nsIDOMNodeList> childNodes;
            nsCOMPtr<nsIDOMNode> cNode;
            res = curNode->GetChildNodes(getter_AddRefs(childNodes));
            if (NS_FAILED(res)) return res;
            if (!childNodes) return NS_ERROR_NULL_POINTER;
            PRUint32 childCount;
            res = childNodes->GetLength(&childCount);
            if (NS_FAILED(res)) return res;
            PRUint32 k;
            nsCOMArray<nsIDOMNode> arr;
            for (k = 0; k < childCount; k++)
            {
              res = childNodes->Item(0, getter_AddRefs(cNode));
              if (!cNode) break;
              nsAutoString s;
              nsCOMPtr<nsIDOM3Node> dom3tempnode;
              dom3tempnode = do_QueryInterface(cNode);
              dom3tempnode->GetTextContent(s);
              NS_ADDREF(cNode.get());
              res = InsertMathNode( cNode,
                 parentNode,
                offsetOfNewNode,
                bDidInsert,
                getter_AddRefs(lastInsertNode));
              offsetOfNewNode++;
            }
          }
          else
          {
            res = InsertMathNode( curNode,
              parentNode,
              offsetOfNewNode,
              bDidInsert,
              getter_AddRefs(lastInsertNode));
            offsetOfNewNode++;
          }
        }
        else
        {
          nsAutoString tagName;
          GetTagString(curNode, tagName);
          if (!tagName.EqualsLiteral("math"))
          {
            nsCOMPtr<nsIDOMElement> mathElement;
            res = msiUtils::CreateMathMLElement(this, msiEditingAtoms::math, mathElement);
            nsCOMPtr<nsIDOMNode> mathNode = do_QueryInterface(mathElement);
            res = InsertNodeAtPoint(mathNode, (nsIDOMNode **)address_of(parentNode), &offsetOfNewNode, PR_TRUE);
            PRInt32 offset = 0;
            res = InsertNodeAtPoint(curNode, (nsIDOMNode **)address_of(mathNode), &offset, PR_TRUE);
            parentNode = mathElement;
            offsetOfNewNode = 1;
          }
          else
          {
            res = InsertNodeAtPoint(curNode, (nsIDOMNode **)address_of(parentNode), &offsetOfNewNode, PR_TRUE);
          }
          if (NS_SUCCEEDED(res))
          {
            bDidInsert = PR_TRUE;
            lastInsertNode = curNode;
          }
        }
//        selection->Collapse(parentNode, offsetOfNewNode);
//        res = mRules->DidDoAction(selection, &ruleInfo, res);
//        return res;
      }
      else
      {

        // try to insert
// #if DEBUG_barry || DEBUG_Barry
//     printf("\nTry to insert\n");
    DumpNode(curNode);
    // DebExamineNode(curNode);
    // DebExamineNode(parentNode);
// #endif
        res = InsertNodeAtPoint(curNode, (nsIDOMNode **)address_of(parentNode), &offsetOfNewNode, PR_TRUE);
        if (NS_SUCCEEDED(res))
        {
          bDidInsert = PR_TRUE;
          insertedContextParent = curNode;
          lastInsertNode = curNode;
        }

        // assume failure means no legal parent in the document heirarchy.
        // try again with the parent of curNode in the paste heirarchy.
        nsCOMPtr<nsIDOMNode> parent;
        while (NS_FAILED(res) && curNode)
        {
          curNode->GetParentNode(getter_AddRefs(parent));
// #if DEBUG_barry || DEBUG_Barry
//     DebExamineNode(parent);
// #endif
          if (parent && !nsTextEditUtils::IsBody(parent))
          {
            res = InsertNodeAtPoint(parent, (nsIDOMNode **)address_of(parentNode), &offsetOfNewNode, PR_TRUE);
            if (NS_SUCCEEDED(res))
            {
              bDidInsert = PR_TRUE;
              insertedContextParent = parent;
              lastInsertNode = parent;
            }
          }
          curNode = parent;
        }
      }
      if (bDidInsert)
      {
        res = GetNodeLocation(lastInsertNode, address_of(parentNode), &offsetOfNewNode);
        NS_ENSURE_SUCCESS(res, res);
        offsetOfNewNode++;
      }
    }

    // Now collapse the selection to the end of what we just inserted:
    if (lastInsertNode)
    {
      // set selection to the end of what we just pasted.
      nsCOMPtr<nsIDOMNode> selNode, tmp, visNode, highTable;
      PRInt32 selOffset;

      // but don't cross tables
      if (!nsHTMLEditUtils::IsTable(lastInsertNode, mtagListManager))
      {
        res = GetLastEditableLeaf(lastInsertNode, address_of(selNode));
        if (NS_FAILED(res)) return res;
        tmp = selNode;
        while (tmp && (tmp != lastInsertNode))
        {
          if (nsHTMLEditUtils::IsTable(tmp, mtagListManager))
            highTable = tmp;
          nsCOMPtr<nsIDOMNode> parent = tmp;
          tmp->GetParentNode(getter_AddRefs(parent));
          tmp = parent;
        }
        if (highTable)
          selNode = highTable;
      }
      if (!selNode)
        selNode = lastInsertNode;
      if (IsTextNode(selNode) || (IsContainer(selNode) && !nsHTMLEditUtils::IsTable(selNode, mtagListManager)))
      {
        res = GetLengthOfDOMNode(selNode, (PRUint32&)selOffset);
        if (NS_FAILED(res)) return res;
      }
      else // we need to find a container for selection.  Look up.
      {
        tmp = selNode;
        res = GetNodeLocation(tmp, address_of(selNode), &selOffset);
        ++selOffset;  // want to be *after* last leaf node in paste
        if (NS_FAILED(res)) return res;
      }

      // make sure we don't end up with selection collapsed after an invisible break node
      nsWSRunObject wsRunObj(this, selNode, selOffset);
      PRInt32 outVisOffset=0;
      PRInt16 visType=0;
      res = wsRunObj.PriorVisibleNode(selNode, selOffset, address_of(visNode), &outVisOffset, &visType);
      if (NS_FAILED(res)) return res;
      if (visType == nsWSRunObject::eBreak)
      {
        // we are after a break.  Is it visible?  Despite the name,
        // PriorVisibleNode does not make that determination for breaks.
        // It also may not return the break in visNode.  We have to pull it
        // out of the nsWSRunObject's state.
        if (!IsVisBreak(wsRunObj.mStartReasonNode))
        {
          // don't leave selection past an invisible break;
          // reset {selNode,selOffset} to point before break
          res = GetNodeLocation(wsRunObj.mStartReasonNode, address_of(selNode), &selOffset);
          // we want to be inside any inline style prior to break
          nsWSRunObject wsRunObj(this, selNode, selOffset);
          res = wsRunObj.PriorVisibleNode(selNode, selOffset, address_of(visNode), &outVisOffset, &visType);
            if (NS_FAILED(res)) return res;
          if (visType == nsWSRunObject::eText ||
              visType == nsWSRunObject::eNormalWS)
          {
            selNode = visNode;
            selOffset = outVisOffset;  // PriorVisibleNode already set offset to _after_ the text or ws
          }
          else if (visType == nsWSRunObject::eSpecial)
          {
            // prior visible thing is an image or some other non-text thingy.
            // We want to be right after it.
            res = GetNodeLocation(wsRunObj.mStartReasonNode, address_of(selNode), &selOffset);
            ++selOffset;
          }
        }
      }

      // if we just pasted a link, discontinue link style
      nsCOMPtr<nsIDOMNode> link;
      if (!bStartedInLink && IsInLink(selNode, address_of(link)))
      {
        // so, if we just pasted a link, I split it.  Why do that instead of just
        // nudging selection point beyond it?  Because it might have ended in a BR
        // that is not visible.  If so, the code above just placed selection
        // inside that.  So I split it instead.
        nsCOMPtr<nsIDOMNode> leftLink;
        PRInt32 linkOffset;
        res = SplitNodeDeep(link, selNode, selOffset, &linkOffset, PR_TRUE, address_of(leftLink));
        if (NS_FAILED(res)) return res;
        res = GetNodeLocation(leftLink, address_of(selNode), &selOffset);
        if (NS_FAILED(res)) return res;
      }
      selection->Collapse(selNode, selOffset);
    }
  }

  res = mRules->DidDoAction(selection, &ruleInfo, res);

  return res;
}

//PRBool IsWhiteSpaceOnly( nsString str )
// Function was never called. Removed 2013-07-18

// InsertReturn -- usually splits a paragraph
nsresult
nsHTMLEditor::InsertReturn( )
{
  nsresult result;
  PRBool bfancy;
  nsCOMPtr<nsIPrefBranch> prefBranch =
    do_GetService(NS_PREFSERVICE_CONTRACTID, &result);
  if (NS_SUCCEEDED(result) && prefBranch)
    result = prefBranch->GetBoolPref("swp.fancyreturn", &bfancy);
  if (NS_FAILED(result)) bfancy = PR_FALSE;
  return InsertReturnImpl(bfancy );
}

// InsertReturnFancy -- usually splits a paragraph
nsresult
nsHTMLEditor::InsertReturnFancy( )
{
  return InsertReturnImpl( PR_TRUE );
}



// InsertReturn -- usually splits a paragraph
nsresult
nsHTMLEditor::InsertReturnImpl( PRBool fFancy )

{
  nsAutoUpdateViewBatch viewBatch(this);
  if (!mRules) return NS_ERROR_NOT_INITIALIZED;

  // force IME commit; set up rules sniffing and batching
  ForceCompositionEnd();
//  SaveSelection();
  nsAutoEditBatch beginBatching(this);
  nsAutoRules beginRulesSniffing(this, kOpHTMLPaste, nsIEditor::eNext);

  // Get selection
  nsresult res;
  nsCOMPtr<nsIDOMNode>splitpointNode;
  PRInt32 splitpointOffset;
  nsCOMPtr<nsISelection>selection;
  res = GetSelection(getter_AddRefs(selection));
  res = GetStartNodeAndOffset(selection, getter_AddRefs(splitpointNode), &splitpointOffset);
  PRBool bHandled = PR_FALSE;
  res = InsertReturnInMath(splitpointNode, splitpointOffset, &bHandled);
  if (NS_FAILED(res)) return res;
  else if (bHandled)
    return res;

  res = InsertReturnAt(splitpointNode, splitpointOffset, fFancy);
  res = nsEditorUtils::JiggleCursor(this, selection, nsIEditor::eNext);

  return res;
}

nsresult
nsHTMLEditor::InsertReturnInMath( nsIDOMNode * splitpointNode, PRInt32 splitpointOffset, PRBool* bHandled)
{
  *bHandled = PR_FALSE;
  return NS_OK;
}

//utility function
PRBool HasNoSignificantTags(nsIDOMNode * node, msiITagListManager * tlm)
{
  // run through the subnodes of node. We know there is no non-whitespace text, so text nodes can be ignored. Text tags can be ignored.
  // We say everything else is significant. Also, text tag nodes cannot have significant subnodes

  nsCOMPtr<nsIDOMNode> child, tmp;
  nsCOMPtr<nsIDOMElement> el;
  node->GetFirstChild(getter_AddRefs(child));
  nsAutoString tagname;
  nsAutoString classname;


  while (child)
  {
    el = do_QueryInterface(child);
    if (el)
    {
      el->GetTagName(tagname);
      if (!(tagname.EqualsLiteral("#text") || tagname.EqualsLiteral("br")))
      {
        tlm->GetRealClassOfTag(tagname, nsnull, classname);
        if (!(classname.EqualsLiteral("texttag") || classname.EqualsLiteral("paratag")
            || classname.EqualsLiteral("structtag") || classname.EqualsLiteral("envtag") || classname.EqualsLiteral("listtag") || classname.EqualsLiteral("listparenttag")))
          return PR_FALSE;
      }
    }
    child->GetNextSibling(getter_AddRefs(tmp));
    child = tmp;
  }
  return PR_TRUE;
}



nsresult nsHTMLEditor::GetDocumentGraphicsDir(nsILocalFile** graphicsDir, PRBool bCreate)
{
  nsresult rv = NS_OK;
  nsCOMPtr<nsILocalFile> graphics;
  nsCOMPtr<nsIDOMDocument> domDoc;
  GetDocument(getter_AddRefs(domDoc));
  if (!domDoc) return NS_ERROR_FAILURE;
  nsCOMPtr<nsIDocument> doc = do_QueryInterface(domDoc);
  if (!doc) return NS_ERROR_FAILURE;
  nsCOMPtr<nsIURI> docURI;
  docURI = doc->GetDocumentURI();
  nsCAutoString docFilePath;
  rv = docURI->GetSpec(docFilePath);
  NS_ENSURE_SUCCESS(rv, rv);
  nsCOMPtr<nsIFile> docFile;
  rv = NS_GetFileFromURLSpec(docFilePath, getter_AddRefs(docFile));
  NS_ENSURE_SUCCESS(rv, rv);
  nsCOMPtr<nsIFile>docDir;
  rv = docFile->GetParent(getter_AddRefs(docDir));
  NS_ENSURE_SUCCESS(rv, rv);
  graphics = do_QueryInterface(docDir);
  graphics->Append(NS_LITERAL_STRING("graphics"));
  if (bCreate)
  {
    PRBool fExists;
    graphics->Exists(&fExists);
    if (!fExists)
      graphics->Create(1,0755);
  }
  *graphicsDir = graphics;
  NS_IF_ADDREF(*graphicsDir);
  return NS_OK;
}

float
nsHTMLEditor::ConvertLengthStringToPixels(const nsAString& lengthStr, float* pOriginal)
{
  float length;
  nsIAtom* unit;
  mHTMLCSSUtils->nsHTMLCSSUtils::ParseLength(lengthStr, &length, &unit);
  *pOriginal = length;
  if (!unit || (nsEditProperty::cssPxUnit == unit))
    return length;

  float rv = length * NS_EDITOR_INDENT_INCREMENT_PX;
  if (nsEditProperty::cssInUnit == unit)
    rv = rv / NS_EDITOR_INDENT_INCREMENT_IN;
  else if (nsEditProperty::cssCmUnit == unit)
    rv = rv / NS_EDITOR_INDENT_INCREMENT_CM;
  else if (nsEditProperty::cssMmUnit == unit)
    rv = rv / NS_EDITOR_INDENT_INCREMENT_MM;
  else if (nsEditProperty::cssPtUnit == unit)
    rv = rv / NS_EDITOR_INDENT_INCREMENT_PT;
  else if (nsEditProperty::cssPcUnit == unit)
    rv = rv / NS_EDITOR_INDENT_INCREMENT_PC;
  else if (nsEditProperty::cssEmUnit == unit)
    rv = rv / NS_EDITOR_INDENT_INCREMENT_EM;
  else if (nsEditProperty::cssExUnit == unit)
    rv = rv / NS_EDITOR_INDENT_INCREMENT_EX;
  else if (nsEditProperty::cssPercentUnit == unit)
    rv = length / NS_EDITOR_INDENT_INCREMENT_PERCENT;
  return rv;
}


NS_IMETHODIMP nsHTMLEditor::ApplyGraphicsDefaults(nsIDOMElement *frame, const nsAString & frametype)
{
  nsresult res = NS_OK;
  // Check user preferences
  nsCOMPtr<nsIPrefBranch> prefBranch = do_GetService(NS_PREFSERVICE_CONTRACTID, &res);

  nsAutoString floatPlacement;
  nsXPIDLCString prefString, unitString;
  nsAutoString setPrefString, setUnitString;
  PRBool prefBool = PR_FALSE;
  float lengthVal, origVal;
  char * prefName;
  nsCOMPtr<nsIDOMNode> child;
  nsCOMPtr<nsIDOMNode> captionElement;
  nsCOMPtr<nsIDOMNode> dummy;
  nsCOMPtr<nsIDOMDocument> doc;
  PRBool bIsImage = frametype.EqualsLiteral("image") || frametype.EqualsLiteral("textframe");
  PRBool bIsText = frametype.EqualsLiteral("textframe");


  // SetAttribute(frame, NS_LITERAL_STRING("xmlns"), NS_LITERAL_STRING("http://www.w3.org/1999/xhtml"));
  if (NS_SUCCEEDED(res) && prefBranch)
  {
    prefName = (char *)(!bIsImage ? "swp.graph.placement" : "swp.graphics.placement");
    res = prefBranch->GetCharPref((const char *)prefName, getter_Copies(prefString));
    if (NS_SUCCEEDED(res))
      CopyASCIItoUTF16(prefString, setPrefString);
    else
      setPrefString = NS_LITERAL_STRING("inline");
    SetAttribute(frame, NS_LITERAL_STRING("pos"), setPrefString);
    if (setPrefString.EqualsLiteral("floating")) {
      prefName = (char *) (!bIsImage ? "swp.graph.floatlocation.forcehere" : "swp.graphics.floatlocation.forcehere");
      res = prefBranch->GetBoolPref((const char *)prefName, &prefBool);
      if (NS_SUCCEEDED(res) && prefBool)
        floatPlacement.Append(PRUnichar('H'));
      prefName = (char *) (!bIsImage ? "swp.graph.floatlocation.here" : "swp.graphics.floatlocation.here");
      res = prefBranch->GetBoolPref((const char *)prefName, &prefBool);
      if (NS_SUCCEEDED(res) && prefBool)
        floatPlacement.Append(PRUnichar('h'));
      prefName = (char *) (!bIsImage ? "swp.graph.floatlocation.pagefloats" : "swp.graphics.floatlocation.pagefloats");
      res = prefBranch->GetBoolPref((const char *)prefName, &prefBool);
      if (NS_SUCCEEDED(res) && prefBool)
        floatPlacement.Append(PRUnichar('p'));
      prefName = (char *) (!bIsImage ? "swp.graph.floatlocation.toppage" : "swp.graphics.floatlocation.toppage");
      res = prefBranch->GetBoolPref((const char *)prefName, &prefBool);
      if (NS_SUCCEEDED(res) && prefBool)
        floatPlacement.Append(PRUnichar('t'));
      prefName = (char *) (!bIsImage ? "swp.graph.floatlocation.bottompage" : "swp.graphics.floatlocation.bottompage");
      res = prefBranch->GetBoolPref((const char *)prefName, &prefBool);
      if (NS_SUCCEEDED(res) && prefBool)
        floatPlacement.Append(PRUnichar('b'));
      SetAttribute(frame, NS_LITERAL_STRING("ltxfloat"), floatPlacement);
    }
    prefName = (char *) (!bIsImage ? "swp.graph.units" : "swp.graphics.units");
    res = prefBranch->GetCharPref((const char *)prefName, getter_Copies(unitString));
    if (NS_SUCCEEDED(res))
      CopyASCIItoUTF16(unitString, setUnitString);
    else
      setUnitString = NS_LITERAL_STRING("in");
    SetAttribute(frame, NS_LITERAL_STRING("units"), setUnitString);



    if (bIsText) prefBool = PR_TRUE; // for text frames, always use pref dimensions
    else {
      prefName = (char *) (!bIsImage ? "swp.graph.usedefaultwidth" : "swp.graphics.usedefaultwidth");
      res = prefBranch->GetBoolPref((const char *)prefName, &prefBool);
    }

    if (NS_SUCCEEDED(res) && prefBool)
    {
      prefName = (char *) (!bIsImage ? "swp.graph.hsize" : "swp.graphics.hsize");
      res = prefBranch->GetCharPref((const char *)prefName, getter_Copies(prefString));
      if (NS_SUCCEEDED(res))
      {
        CopyASCIItoUTF16(prefString, setPrefString);
        SetAttribute(frame, NS_LITERAL_STRING("width"), setPrefString);
        frame->RemoveAttribute(NS_LITERAL_STRING("imageWidth"));
        frame->RemoveAttribute(NS_LITERAL_STRING("ltx_width"));
      }
    }
    if (bIsText) prefBool = PR_TRUE; // for text frames, always use pref dimensions
    else {
      prefName = (char *) (!bIsImage ? "swp.graph.usedefaultheight" : "swp.graphics.usedefaultheight");
      res = prefBranch->GetBoolPref((const char *)prefName, &prefBool);
    }
    if (NS_SUCCEEDED(res) && prefBool)
    {
      prefName = (char *) (!bIsImage ? "swp.graph.vsize" : "swp.graphics.vsize");

      res = prefBranch->GetCharPref((const char *)prefName, getter_Copies(prefString));
      if (NS_SUCCEEDED(res))
      {
        CopyASCIItoUTF16(prefString, setPrefString);
        SetAttribute(frame, NS_LITERAL_STRING("height"), setPrefString);
        frame->RemoveAttribute(NS_LITERAL_STRING("ltx_height"));
        frame->RemoveAttribute(NS_LITERAL_STRING("imageHeight"));
      }
    }
    // Margins
    // hmargin
    prefName = (char *) (!bIsImage ? "swp.graph.hmargin" : "swp.graphics.hmargin");

    res = prefBranch->GetCharPref((const char *)prefName, getter_Copies(prefString));
    if (NS_SUCCEEDED(res))
    {
      CopyASCIItoUTF16(prefString, setPrefString);
      SetAttribute(frame, NS_LITERAL_STRING("sidemargin"), setPrefString);
    }

    // vmargin
    prefName = (char *) (!bIsImage ? "swp.graph.vmargin" : "swp.graphics.vmargin");

    res = prefBranch->GetCharPref((const char *)prefName, getter_Copies(prefString));
    if (NS_SUCCEEDED(res))
    {
      CopyASCIItoUTF16(prefString, setPrefString);
      SetAttribute(frame, NS_LITERAL_STRING("topmargin"), setPrefString);
    }

    // borderwidth
    prefName = (char *) (!bIsImage ? "swp.graph.borderwidth" : "swp.graphics.borderwidth");

    res = prefBranch->GetCharPref((const char *)prefName, getter_Copies(prefString));
    if (NS_SUCCEEDED(res))
    {
      CopyASCIItoUTF16(prefString, setPrefString);
      SetAttribute(frame, NS_LITERAL_STRING("borderw"), setPrefString);
    }

    // paddingwidth
    prefName = (char *) (!bIsImage ? "swp.graph.paddingwidth" : "swp.graphics.paddingwidth");

    res = prefBranch->GetCharPref((const char *)prefName, getter_Copies(prefString));
    if (NS_SUCCEEDED(res))
    {
      CopyASCIItoUTF16(prefString, setPrefString);
      SetAttribute(frame, NS_LITERAL_STRING("padding"), setPrefString);
    }

   // border-color
    prefName = (char *) (!bIsImage ? "swp.graph.bordercolor" : "swp.graphics.bordercolor");

    res = prefBranch->GetCharPref((const char *)prefName, getter_Copies(prefString));
    if (NS_SUCCEEDED(res))
    {
      CopyASCIItoUTF16(prefString, setPrefString);
      SetAttribute(frame, NS_LITERAL_STRING("border-color"), setPrefString);
    }

    // bgcolor
    prefName = (char *) (!bIsImage ? "swp.graph.bgcolor" : "swp.graphics.bgcolor");

    res = prefBranch->GetCharPref((const char *)prefName, getter_Copies(prefString));
    if (NS_SUCCEEDED(res))
    {
      CopyASCIItoUTF16(prefString, setPrefString);
      SetAttribute(frame, NS_LITERAL_STRING("background-color"), setPrefString);
    }
    // caption or not, caption location
    prefName = (char *) (!bIsImage ? "swp.graph.captionplacement" : "swp.graphics.captionplacement");

    res = prefBranch->GetCharPref((const char *)prefName, getter_Copies(prefString));
    if (NS_SUCCEEDED(res))
    {
      CopyASCIItoUTF16(prefString, setPrefString);
      nsAutoString childName;
      if (setPrefString.Length() > 0 && !setPrefString.EqualsLiteral("none"))
      {
        SetAttribute(frame, NS_LITERAL_STRING("captionloc"), setPrefString);
        GetDocument(getter_AddRefs(doc));
        res = frame->GetFirstChild(getter_AddRefs(child));
        if (child != nsnull) child->GetNodeName(childName);
        // the default msiframe contains a bodyText. For graphics and plots we want to replace this
        // with an imagecaption
        while (child && !childName.EqualsLiteral("bodyText")) {
          child->GetNextSibling(getter_AddRefs(child));
          if (child) child->GetNodeName(childName);
        }
        if (child) {
          res = mtagListManager->GetNewInstanceOfNode(NS_LITERAL_STRING("imagecaption"), nsnull, doc, getter_AddRefs(captionElement));
          frame->ReplaceChild(captionElement, child, getter_AddRefs(dummy));
        }
      }
    }

  }
  return res;
}

NS_IMETHODIMP
nsHTMLEditor::GetFrameStyleFromAttributes(nsIDOMElement * frame)
{
  nsresult res = NS_OK;
  PRBool left = PR_FALSE;
  PRBool right = PR_FALSE;
  nsAutoString topmarginStr, sidemarginStr;
  nsAutoString leftmargin, rightmargin;
  // frame attributes
  NS_NAMED_LITERAL_STRING(borderw,"borderw");
  NS_NAMED_LITERAL_STRING(padding,"padding");
  NS_NAMED_LITERAL_STRING(captionloc,"captionloc");
  NS_NAMED_LITERAL_STRING(units,"units");
  NS_NAMED_LITERAL_STRING(sidemargin,"sidemargin");
  NS_NAMED_LITERAL_STRING(topmargin,"topmargin");
  NS_NAMED_LITERAL_STRING(overhang,"overhang");
  NS_NAMED_LITERAL_STRING(pos,"pos");
  NS_NAMED_LITERAL_STRING(ltxfloat,"ltxfloat");
  NS_NAMED_LITERAL_STRING(bordercolor,"border-color");
  NS_NAMED_LITERAL_STRING(backgroundcolor,"background-color");
  NS_NAMED_LITERAL_STRING(textalignment,"textalignment");
  NS_NAMED_LITERAL_STRING(width,"width");
  NS_NAMED_LITERAL_STRING(height,"height");
  NS_NAMED_LITERAL_STRING(aspect, "aspect");
  // style keywords
  NS_NAMED_LITERAL_STRING(border,"border");
  NS_NAMED_LITERAL_STRING(borderwidth,"border-width");
  NS_NAMED_LITERAL_STRING(margin,"margin");
  NS_NAMED_LITERAL_STRING(textalign,"text-align");
  NS_NAMED_LITERAL_STRING(floatatt,"float");
  NS_NAMED_LITERAL_STRING(captionside,"caption-side");
  NS_NAMED_LITERAL_STRING(colon,": ");
  NS_NAMED_LITERAL_STRING(semicolon,"; ");
  NS_NAMED_LITERAL_STRING(space," ");
  NS_NAMED_LITERAL_STRING(defaultunit,"in");
  NS_NAMED_LITERAL_STRING(zero, "0");
  NS_NAMED_LITERAL_STRING(autoStr, "auto");

  // other strings
  nsAutoString style, attrStr, posStr, unitStr, borderwStr, bordercolorStr;
  res = frame->GetAttribute(units, unitStr);
  if (unitStr.Length() == 0) unitStr.Assign(defaultunit);
  res = frame->GetAttribute(borderw, borderwStr);
  res = frame->GetAttribute(bordercolor, bordercolorStr);
  if (borderwStr.Length() > 0 && bordercolorStr.Length() > 0) {
    style += border + colon + borderwStr + unitStr + space + NS_LITERAL_STRING("solid") +
      space + bordercolorStr + semicolon;
  }
  else {
    if (borderwStr.Length() > 0)
      style += borderwidth + colon + borderwStr + unitStr + semicolon;
    if (bordercolorStr.Length() > 0)
      style += bordercolor + colon + bordercolorStr + semicolon;
  }
  res = frame->GetAttribute(width, attrStr);
  if (attrStr.Length() > 0) {
    style += width + colon + attrStr + unitStr + semicolon;
  }

  res = frame->GetAttribute(height, attrStr);
  if (attrStr.Length() > 0) {
    style += height + colon + attrStr + unitStr + semicolon;
  }

  res = frame->GetAttribute(pos, posStr);
  if (posStr.Length() > 0) {
    left = (posStr.EqualsLiteral("L") || posStr.EqualsLiteral("I") || posStr.EqualsLiteral("left") || posStr.EqualsLiteral("inside"));
    right = (posStr.EqualsLiteral("R") || posStr.EqualsLiteral("O") || posStr.EqualsLiteral("right") || posStr.EqualsLiteral("outside"));
    if (left) {
      style += floatatt + colon + NS_LITERAL_STRING("left") + semicolon;
    }
    else if (right) {
      style += floatatt + colon + NS_LITERAL_STRING("right") + semicolon;
    }

  }
  // find topmargin, sidemargin
  res = frame->GetAttribute(sidemargin, sidemarginStr);
  res = frame->GetAttribute(topmargin, topmarginStr);
  if (topmarginStr.Length() == 0) topmarginStr = zero;
  if (sidemarginStr.Length() == 0) sidemarginStr = zero;
  if (posStr.EqualsLiteral("center") || posStr.EqualsLiteral("floating") || posStr.EqualsLiteral("displayed")) {
    leftmargin.Assign(autoStr);
    rightmargin.Assign(autoStr);
    left = PR_FALSE;
    right = PR_FALSE;
  }
  else {
    leftmargin = sidemarginStr + unitStr;
    rightmargin = sidemarginStr + unitStr;
  }
  if (left) {
    leftmargin = zero + unitStr;
  }
  else if (right) {
    rightmargin = zero + unitStr;
  }
  style += margin + colon + topmarginStr + unitStr + space + rightmargin +
  space + topmarginStr + unitStr + space + leftmargin +
    semicolon;
// Overhang
    // come back to this.
  res = frame->GetAttribute(padding, attrStr);
  if (attrStr.Length() > 0) {
    style += padding + colon + attrStr + unitStr + semicolon;
  }
  res = frame->GetAttribute(backgroundcolor, attrStr);
  if (attrStr.Length() > 0) {
    style += backgroundcolor + colon + attrStr + semicolon;
  }
  res = frame->GetAttribute(textalignment, attrStr);
  if (attrStr.Length() > 0) {
    style += textalign + colon + attrStr + semicolon;
  }
  res = frame->GetAttribute(captionloc, attrStr);
  if (attrStr.Length() > 0) {
    style += captionside + colon + attrStr + semicolon;
  }
  res = frame->SetAttribute(NS_LITERAL_STRING("style"), style);
  return res;
}


NS_IMETHODIMP
nsHTMLEditor::GetGraphicsAttributesFromFrame(nsIDOMElement *frame, nsIDOMElement *object)
{
  // height and width on the frame are given in the units specified.
  // height and width on the object are given in CSS pixels; we can use other units in the style
  nsresult res = NS_OK;
  nsAutoString width, height, naturalWidth, naturalHeight,
    originalSrcUrl, aspect, units;
  nsAutoString style(EmptyString());
  NS_NAMED_LITERAL_STRING(semi, "; ");
  object->SetAttribute(NS_LITERAL_STRING("_moz_resizing"), NS_LITERAL_STRING("true"));
  object->SetAttribute(NS_LITERAL_STRING("msi_resize"), NS_LITERAL_STRING("true"));
  res = frame->GetAttribute(NS_LITERAL_STRING("units"), units);
  res = frame->GetAttribute(NS_LITERAL_STRING("height"),height);
  res = frame->GetAttribute(NS_LITERAL_STRING("width"),width);

  style += NS_LITERAL_STRING("height: ") + height + units + NS_LITERAL_STRING("; width: ") + width + units + NS_LITERAL_STRING("; ");
  if (height.Length() > 0) {
     style += NS_LITERAL_STRING("height: ") + height + units + semi;
//     res = object->SetAttribute(NS_LITERAL_STRING("height"), height);
     res = frame->SetAttribute(NS_LITERAL_STRING("height"), height);
     res = frame->SetAttribute(NS_LITERAL_STRING("naturalheight"), height);
   }
   if (width.Length() > 0) {
     style += NS_LITERAL_STRING("width: ") + width + units + semi;
//     res = frame->SetAttribute(NS_LITERAL_STRING("width"), width);
     res = frame->SetAttribute(NS_LITERAL_STRING("naturalwidth"), width);
   }
  res = object->SetAttribute(NS_LITERAL_STRING("style"), style);
  res = object->SetAttribute(NS_LITERAL_STRING("units"), units);
  return res;
}

// nsresult
// nsHTMLEditor::GetGraphicsDefaultsAsString( nsAString& retStr, PRBool bIsPlot )
// {
//   nsresult res = NS_OK;
//   // Check user preferences
//   nsCOMPtr<nsIPrefBranch> prefBranch = do_GetService(NS_PREFSERVICE_CONTRACTID, &res);

//   nsAutoString attribStr, styleStr, floatPlacement;
//   nsXPIDLCString prefString, unitString;
//   nsAutoString setPrefString, setUnitString;
//   PRBool prefBool = PR_FALSE;
//   float lengthVal, origVal;

//   attribStr = NS_LITERAL_STRING("xmlns=\"http://www.w3.org/1999/xhtml\"");
//   if (NS_SUCCEEDED(res) && prefBranch)
//   {
//     if (bIsPlot)
//     {
//       res = prefBranch->GetCharPref("swp.graph.placement", getter_Copies(prefString));
//       if (NS_SUCCEEDED(res))
//         CopyASCIItoUTF16(prefString, setPrefString);
//       else
//         setPrefString = NS_LITERAL_STRING("inline");
//       attribStr.AppendLiteral(" pos=\"");
//       attribStr.Append(setPrefString);
//       attribStr.Append(PRUnichar('\"'));

//       res = prefBranch->GetBoolPref("swp.graph.floatLocation.forceHere", &prefBool);
//       if (NS_SUCCEEDED(res) && prefBool)
//         floatPlacement.Append(PRUnichar('H'));
//       res = prefBranch->GetBoolPref("swp.graph.floatLocation.here", &prefBool);
//       if (NS_SUCCEEDED(res) && prefBool)
//         floatPlacement.Append(PRUnichar('h'));
//       res = prefBranch->GetBoolPref("swp.graph.floatLocation.pageFloats", &prefBool);
//       if (NS_SUCCEEDED(res) && prefBool)
//         floatPlacement.Append(PRUnichar('p'));
//       res = prefBranch->GetBoolPref("swp.graph.floatLocation.topPage", &prefBool);
//       if (NS_SUCCEEDED(res) && prefBool)
//         floatPlacement.Append(PRUnichar('t'));
//       res = prefBranch->GetBoolPref("swp.graph.floatLocation.bottomPage", &prefBool);
//       if (NS_SUCCEEDED(res) && prefBool)
//         floatPlacement.Append(PRUnichar('b'));
//       attribStr.AppendLiteral(" placeLocation=\"");
//       attribStr.Append(floatPlacement);
//       attribStr.Append(PRUnichar('\"'));

//       res = prefBranch->GetCharPref("swp.graph.floatPlacement", getter_Copies(prefString));
//       if (NS_SUCCEEDED(res))
//       {
//         CopyASCIItoUTF16(prefString, setPrefString);
//         attribStr.AppendLiteral("placement=\"");
//         attribStr.Append(setPrefString);
//         attribStr.Append(PRUnichar('\"'));
//       }

//       res = prefBranch->GetCharPref("swp.graph.defaultUnits", getter_Copies(unitString));
//       if (NS_SUCCEEDED(res))
//         CopyASCIItoUTF16(unitString, setUnitString);
//       else
//         setUnitString = NS_LITERAL_STRING("in");
//       attribStr.AppendLiteral(" units=\"");
//       attribStr.Append(setUnitString);
//       attribStr.Append(PRUnichar('\"'));

//       res = prefBranch->GetCharPref("swp.graph.HSize", getter_Copies(prefString));
//       if (NS_SUCCEEDED(res))
//       {
//         CopyASCIItoUTF16(prefString, setPrefString);
//         lengthVal = ConvertLengthStringToPixels(setPrefString, &origVal);
//         attribStr.AppendLiteral(" imageWidth=\"");
//         attribStr.AppendFloat(origVal);
//         attribStr.Append(PRUnichar('\"'));
//         attribStr.AppendLiteral(" width=\"");
//         attribStr.AppendFloat(lengthVal);
//         attribStr.Append(PRUnichar('\"'));
//       }

//       res = prefBranch->GetCharPref("swp.graph.VSize", getter_Copies(prefString));
//       if (NS_SUCCEEDED(res))
//       {
//         CopyASCIItoUTF16(prefString, setPrefString);
//         lengthVal = ConvertLengthStringToPixels(setPrefString, &origVal);
//         attribStr.AppendLiteral(" imageHeight=\"");
//         attribStr.AppendFloat(origVal);
//         attribStr.Append(PRUnichar('\"'));
//         attribStr.AppendLiteral(" height=\"");
//         attribStr.AppendFloat(lengthVal);
//         attribStr.Append(PRUnichar('\"'));
//       }
//     }
//     else
//     {
//       res = prefBranch->GetCharPref("swp.defaultGraphicsPlacement", getter_Copies(prefString));
//       if (NS_SUCCEEDED(res))
//         CopyASCIItoUTF16(prefString, setPrefString);
//       else
//         setPrefString = NS_LITERAL_STRING("inline");
//       attribStr.AppendLiteral(" pos=\"");
//       attribStr.Append(setPrefString);
//       attribStr.Append(PRUnichar('\"'));

//       res = prefBranch->GetBoolPref("swp.defaultGraphicsFloatLocation.forceHere", &prefBool);
//       if (NS_SUCCEEDED(res) && prefBool)
//         floatPlacement.Append(PRUnichar('H'));
//       res = prefBranch->GetBoolPref("swp.defaultGraphicsFloatLocation.here", &prefBool);
//       if (NS_SUCCEEDED(res) && prefBool)
//         floatPlacement.Append(PRUnichar('h'));
//       res = prefBranch->GetBoolPref("swp.defaultGraphicsFloatLocation.pageFloats", &prefBool);
//       if (NS_SUCCEEDED(res) && prefBool)
//         floatPlacement.Append(PRUnichar('p'));
//       res = prefBranch->GetBoolPref("swp.defaultGraphicsFloatLocation.topPage", &prefBool);
//       if (NS_SUCCEEDED(res) && prefBool)
//         floatPlacement.Append(PRUnichar('t'));
//       res = prefBranch->GetBoolPref("swp.defaultGraphicsFloatLocation.bottomPage", &prefBool);
//       if (NS_SUCCEEDED(res) && prefBool)
//         floatPlacement.Append(PRUnichar('b'));
//       attribStr.AppendLiteral(" placeLocation=\"");
//       attribStr.Append(floatPlacement);
//       attribStr.Append(PRUnichar('\"'));

//       res = prefBranch->GetCharPref("swp.defaultGraphicsFloatPlacement", getter_Copies(prefString));
//       if (NS_SUCCEEDED(res))
//       {
//         CopyASCIItoUTF16(prefString, setPrefString);
//         attribStr.AppendLiteral(" placement=\"");
//         attribStr.Append(setPrefString);
//         attribStr.Append(PRUnichar('\"'));
//       }

//       res = prefBranch->GetCharPref("swp.defaultGraphicsSizeUnits", getter_Copies(unitString));
//       if (NS_SUCCEEDED(res))
//         CopyASCIItoUTF16(unitString, setUnitString);
//       else
//         setUnitString = NS_LITERAL_STRING("in");
//       attribStr.AppendLiteral(" units=\"");
//       attribStr.Append(setUnitString);
//       attribStr.Append(PRUnichar('\"'));

//       res = prefBranch->GetCharPref("swp.defaultGraphicsInlineOffset", getter_Copies(prefString));
//       if (NS_SUCCEEDED(res))
//       {
//         CopyASCIItoUTF16(prefString, setPrefString);
//         lengthVal = ConvertLengthStringToPixels(setPrefString, &origVal);
//         if (lengthVal)
//         {
//           attribStr.AppendLiteral(" inlineOffset=\"");
//           attribStr.AppendFloat(origVal);
//           attribStr.Append(PRUnichar('\"'));
//           styleStr.AppendLiteral("position: relative; bottom: ");
//           if (lengthVal > 0)
//             styleStr.Append(PRUnichar('-'));
//           styleStr.AppendFloat(lengthVal);
//           styleStr.Append(PRUnichar(';'));
//         }
//       }

//       res = prefBranch->GetBoolPref("swp.graphicsUseDefaultWidth", &prefBool);
//       if (NS_SUCCEEDED(res) && prefBool)
//       {
//         res = prefBranch->GetCharPref("swp.defaultGraphicsHSize", getter_Copies(prefString));
//         if (NS_SUCCEEDED(res))
//         {
//           CopyASCIItoUTF16(prefString, setPrefString);
//           lengthVal = ConvertLengthStringToPixels(setPrefString, &origVal);
//           attribStr.AppendLiteral(" imageWidth=\"");
//           attribStr.AppendFloat(origVal);
//           attribStr.Append(PRUnichar('\"'));

//           attribStr.AppendLiteral("width");
//           attribStr.AppendFloat(lengthVal);
//           attribStr.Append(PRUnichar('\"'));
//         }
//       }
//       res = prefBranch->GetBoolPref("swp.graphicsUseDefaultHeight", &prefBool);
//       if (NS_SUCCEEDED(res) && prefBool)
//       {
//         res = prefBranch->GetCharPref("swp.defaultGraphicsVSize", getter_Copies(prefString));
//         if (NS_SUCCEEDED(res))
//         {
//           CopyASCIItoUTF16(prefString, setPrefString);
//           lengthVal = ConvertLengthStringToPixels(setPrefString, &origVal);
//           attribStr.AppendLiteral(" imageHeight=\"");
//           attribStr.AppendFloat(origVal);
//           attribStr.Append(PRUnichar('\"'));
//           attribStr.AppendLiteral(" height=\"");
//           attribStr.AppendFloat(lengthVal);
//           attribStr.Append(PRUnichar('\"'));
//         }
//       }
//     }
//   }

//   retStr = attribStr;
//   if (styleStr.Length() > 0)
//   {
//     retStr.AppendLiteral(" style=\"");
//     retStr.Append(styleStr);
//     retStr.Append(PRUnichar('\"'));
//   }
//   return res;
// }

PRBool StructureHasStructureAncestor( nsIDOMNode * node, msiITagListManager * pTlm)
{
  NS_PRECONDITION((node && pTlm), "null arg");
  nsresult res;
  nsString tag, classname;
  nsIAtom * ns = nsnull;
  nsCOMPtr<nsIDOMNode> parent;
  PRBool fIsStruct = PR_FALSE;
  classname = NS_LITERAL_STRING("structtag");
  pTlm->GetTagOfNode( node, &ns, tag);
  while (!fIsStruct && (!tag.EqualsLiteral("body")))
  {
    pTlm->GetTagInClass(classname, tag, ns, &fIsStruct);
//    if (fFirst && fIsStruct) {
//      fIsStruct = PR_FALSE;  // We expect node to be a structure so we pass over the first one we find.
//      fFirst = PR_FALSE;
//    }
    if (!fIsStruct) {
      res = node->GetParentNode(getter_AddRefs(parent));
      node = parent;
      pTlm->GetTagOfNode( node, &ns, tag);
    }
  }
  return fIsStruct;
}

PRBool
nsHTMLEditor::InEmptyCell( nsIDOMNode * node) {
  nsresult res = NS_OK;
  nsCOMPtr<nsIDOMNode> parent;
  PRInt32 offset;
  nsAutoString tagName;
  node->GetNodeName(tagName);
  while (node && !(tagName.EqualsLiteral("td") || tagName.EqualsLiteral("mtd"))) {
    res = GetNodeLocation(node, &parent, &offset);
    node = parent;
    if (node) node->GetNodeName(tagName);
  }
  if (!node) return false;
  nsCOMPtr<nsIDOMElement> el;
  el = do_QueryInterface(node);
  if (!el) return false;
  return IsEmptyCell(el);
}

// InsertReturnAt -- usually splits a paragraph; may call itself recursively
nsresult
nsHTMLEditor::InsertReturnAt( nsIDOMNode * splitpointNode, PRInt32 splitpointOffset, PRBool fFancy)
{
  nsresult res = NS_OK;
  nsCOMPtr<nsISelection>selection;
  nsCOMPtr<nsIDOMDocument> doc;
  nsCOMPtr<nsIDOMNode> splitNode, outLeftNode, outRightNode, tempNode, rightNode, node, parent;
  nsCOMPtr<nsIDOMNode> currentNode, parentOfNewStructure, newNode, newsplitpointNode;
  nsCOMPtr<nsIDOMElement> rightElem, newElement;
  nsCOMPtr<nsIDOMNSHTMLElement> newHTMLElement;
  nsCOMPtr<nsIDOMNodeList> nodeList;
  nsCOMPtr<nsIDOM3Node> dom3node;
  nsAutoString leftName, rightName, sNewNodeName, sInclusion, strContents, strTagName, strTmp, structClassname, envClassname;
  PRInt32 outOffset, newsplitpointOffset, offsetOfNewStructure, offset;
  PRUint32 nodeCount, length;
  PRBool fDiscardNode, isEmpty, success, fCanContain, fInclusion, hasStructureAncestor;
  nsIAtom * atomNS = nsnull;
  PRBool fIsStruct = PR_FALSE;
  PRBool fIsEnv = PR_FALSE;
  NS_PRECONDITION(splitpointNode, "null arg");
  res = GetSelection(getter_AddRefs(selection));

  // With the cursor at splitpointNode and splitpointOffset, we want to split the enclosing paragraph
  // (or block). First if splitpointNode is not a block, move up to the first block.
  if (IsBlockNode(splitpointNode)) {
    splitNode = splitpointNode;
  }
  else {
    splitNode = GetBlockNodeParent(splitpointNode);
  }
  // Get the block's name
  if (splitNode) {
    GetTagString(splitNode, strTagName);
  }
  // If there is no block structure to split, put in a <br/> and return
  if (!IsBlockNode(splitNode)||strTagName.EqualsLiteral("body")
    ||strTagName.EqualsLiteral("#document"))
  {
    nsCOMPtr<nsIDOMNode> para;
    return CreateDefaultParagraph(splitNode, splitpointOffset, PR_TRUE, getter_AddRefs(para));
  }
  else
  // and if we are in verbatim, put in a newline rather than split
  {
    if (strTagName.EqualsLiteral("verbatim"))
    {
      return InsertText(NS_LITERAL_STRING("\n"));
    }
  }
  // Now we should be reduced to the splitting-the-block case
  // If fFancy, we may want to continue splitting up the tree when there is no text in an existing
  // structure.

  // if fFancy and if splitNode has no text, we need to check with
  // the taglistmanager to determine whether to delete splitNode and whether to continue up
  // to the parent.

  dom3node = do_QueryInterface(splitNode);
  if (fFancy)
  {
    res = IsEmptyNode( splitNode, &isEmpty, PR_TRUE, PR_FALSE, PR_TRUE);
    // The booleans are: aSingleBRDoesntCount, aListOrCellNotEmpty, and aSafeToAskFrames.
    // Check that there aren't significant tags in it, such as empty tables, etc.
    if (isEmpty) isEmpty = HasNoSignificantTags(splitNode, mtagListManager);
    if (InEmptyCell(splitNode)) return NS_OK;
    fDiscardNode = PR_FALSE;
    if (isEmpty) mtagListManager->GetDiscardEmptyBlockNode(splitNode, &fDiscardNode);
    // fDiscardNode tells if the this node should be discarded in this case. We don't want to delete it yet;
    // for example, it might be a sectiontitle and the section cannot be deleted because no enclosing section
    // structure exists -- if the sectiontitle is for a section in an article, we can't go up to a chapter,
    // because articles don't have them.
    if (isEmpty && fDiscardNode)
    {
      nsEditor::GetNodeLocation(splitNode, address_of(newsplitpointNode), &newsplitpointOffset);
      mtagListManager->GetStringPropertyForTag(strTagName, atomNS, NS_LITERAL_STRING("nextafteremptyblock"), sNewNodeName);
      mtagListManager->GetStringPropertyForTag(strTagName, atomNS, NS_LITERAL_STRING("inclusion"), sInclusion);
      fInclusion = (sInclusion.EqualsLiteral("true"));
      // fInclusion is true if the object represented by splitNode can just end. An example is a list of some
      // sort. When it ends, the enclosing object just continues. The opposite case is provided by sections
      // and subsections. When a subsection ends, it must be followed by another subsection or else the
      // section must end. In this case the block parent of splitNode has to be split.
      hasStructureAncestor = StructureHasStructureAncestor(splitNode, mtagListManager);

      GetTagString(newsplitpointNode, strTmp);
      structClassname = NS_LITERAL_STRING("structtag");
      envClassname = NS_LITERAL_STRING("envtag");
      mtagListManager->GetTagInClass(structClassname, strTmp, atomNS, &fIsStruct);
      mtagListManager->GetTagInClass(envClassname, strTmp, atomNS, &fIsEnv);
      if (!isEmpty || hasStructureAncestor || !(fIsStruct || fIsEnv));
      {
        return InsertReturnAt(newsplitpointNode, newsplitpointOffset, fFancy);
      }
    }
  }
  PRBool inMath = PR_FALSE;
  if (splitpointNode){
    if (nsHTMLEditUtils::IsMath(splitpointNode)){
      inMath = PR_TRUE;
    }
    else {
      splitpointNode->GetParentNode(getter_AddRefs(parent));
      if (parent &&  nsHTMLEditUtils::IsMath(parent))
        inMath = PR_TRUE;
    }
  }

  if (inMath) {
     res = SplitNodeDeep(splitNode,splitpointNode,splitpointOffset,
       &outOffset, PR_TRUE, address_of(outLeftNode), address_of(outRightNode));
  } else { //is splitNode is wrapped, replace splitNode with its wrapper.
    nsCOMPtr<nsIDOMNode> wrapperNode;
        // Don't allow splitting top level sections when empty
    splitpointNode->GetParentNode(getter_AddRefs(parent));
    hasStructureAncestor = StructureHasStructureAncestor(parent, mtagListManager);
    GetTagString(splitpointNode, strTmp);
      structClassname = NS_LITERAL_STRING("structtag");
      envClassname = NS_LITERAL_STRING("envtag");
      mtagListManager->GetTagInClass(structClassname, strTmp, atomNS, &fIsStruct);
      mtagListManager->GetTagInClass(envClassname, strTmp, atomNS, &fIsEnv);
    if (!isEmpty || hasStructureAncestor || !(fIsStruct || fIsEnv));
    {
      splitpointNode->GetParentNode(getter_AddRefs(parent));
      res = GetWrapper(splitNode, getter_AddRefs(wrapperNode));
      res = SplitNodeDeep(wrapperNode,splitpointNode,splitpointOffset,
         &outOffset, PR_FALSE, &outLeftNode, &outRightNode);
      selection->Collapse(outRightNode, outOffset);

    }
  }
  FixMathematics(outLeftNode, PR_FALSE, PR_FALSE);
  FixMathematics(outRightNode, PR_FALSE, PR_FALSE);
  if (outRightNode) {
    mtagListManager->FixTagsAfterSplit( outLeftNode, (nsIDOMNode **)&outRightNode);
    rightNode = do_QueryInterface(outRightNode);
    rightElem = do_QueryInterface(outRightNode);

    res = GetTagString(outLeftNode, leftName);
    res = GetTagString(outRightNode, rightName);
    if (rightName.EqualsLiteral("bibitem")) {

      nsCOMPtr<nsIDOMDocument> ownerDoc;
      nsCOMPtr<nsIDOMNode> parent;
      PRInt32 offset = 0;
      outRightNode->GetOwnerDocument(getter_AddRefs(ownerDoc));
      NS_ENSURE_STATE(ownerDoc);

      nsCOMPtr<nsIDOMDocumentView> docView = do_QueryInterface(ownerDoc);
      NS_ENSURE_STATE(docView);

      nsCOMPtr<nsIDOMAbstractView> absView;
      docView->GetDefaultView(getter_AddRefs(absView));
      NS_ENSURE_STATE(absView);

      nsCOMPtr<nsIDOMWindowInternal> win = do_QueryInterface(absView);


      nsCOMPtr<nsIControllers> controllers;
      res = win->GetControllers(getter_AddRefs(controllers));
      nsCOMPtr<nsIController> controller;
      const char * command = "cmd_reviseManualBibItemCmd";
      res = controllers->GetControllerForCommand(command, getter_AddRefs(controller));
      if (NS_FAILED(res)) return res;
//      res = nsEditor::GetNodeLocation(outRightNode, address_of(parent), &offset);
//      if (NS_FAILED(res)) return res;
      selection->Collapse(outRightNode, offset);
//      nsEditor::DeleteNode(outRightNode);

      controller->DoCommand(command);
      return res;
    }
    if (!(leftName.Equals(rightName))) {
      // strip attributes off of the right node
      nsCOMPtr<nsIDOMNamedNodeMap> attributemap;

      if (rightNode){
        rightNode->GetAttributes(getter_AddRefs(attributemap));

        PRUint32 length;
        nsCOMPtr<nsIDOMNode> node;
        nsCOMPtr<nsIDOMAttr> attrNode;
        nsCOMPtr<nsIDOMAttr> dummyattrNode;
        attributemap->GetLength(&length);
        for (PRInt32 i = length-1; i >= 0; i--)
        {
         attributemap->Item(i, getter_AddRefs(node));
         attrNode = do_QueryInterface(node);
         rightElem->RemoveAttributeNode(attrNode, getter_AddRefs(dummyattrNode));
        }
      }
    }
      // if rightNode is empty, put in the default content.
    res = IsEmptyNode( rightNode, &isEmpty, PR_TRUE, PR_FALSE, PR_TRUE);
    if (isEmpty)
    {

      res = rightNode->GetLocalName(sNewNodeName);
      mtagListManager->GetStringPropertyForTag(sNewNodeName, atomNS, NS_LITERAL_STRING("inclusion"), sInclusion);
      fInclusion = (sInclusion.EqualsLiteral("true"));
      if (fInclusion) {
        res = GetNodeLocation(outLeftNode, address_of(parent), &offset);
        res = DeleteNode(rightNode);
        GetSelection(getter_AddRefs(selection));
        selection->Collapse(parent, offset+1);
        nsCOMPtr<nsIHTMLEditRules> htmlRules = do_QueryInterface(mRules);
        if (!htmlRules) return NS_ERROR_FAILURE;
        res = htmlRules->InsertBRIfNeeded(selection);
      }
      else {
        res = rightNode->GetOwnerDocument(getter_AddRefs(doc));
        res = rightNode->GetLocalName(sNewNodeName);
        newNode = nsnull;
        res = mtagListManager->GetNewInstanceOfNode(sNewNodeName, atomNS, doc, getter_AddRefs(newNode));
        if (newNode)
        {
          PRBool success = PR_FALSE;
          res = GetNodeLocation(rightNode, address_of(parent), &offset);
          res = InsertNode(newNode, parent, offset);
          res = DeleteNode(rightNode);
          newElement = do_QueryInterface(newNode);
          SetCursorInNewHTML(newElement, &success);
        }
      }
    }
  }
  return res;
}


nsresult
nsHTMLEditor::GetWrapper(nsIDOMNode * node, nsIDOMNode ** wrapperNode)
{
  nsresult res;
  nsString wrapper;
  nsString elementTag;
  nsCOMPtr<nsIDOMNode> parent;
  nsCOMPtr<nsIDOMElement> element;
  nsIAtom * atomNS = nsnull;
  node->GetParentNode(getter_AddRefs(parent));
  element = do_QueryInterface(parent);
  if (element)
    element->GetTagName(elementTag);
  res = mtagListManager->GetStringPropertyForTag(elementTag, atomNS, NS_LITERAL_STRING("wrapper"), wrapper);
  if (wrapper.EqualsLiteral("true"))
    *wrapperNode = parent;
  else
    *wrapperNode = node;
  return NS_OK;
}


nsresult
nsHTMLEditor::SetCursorInNewHTML(nsIDOMElement * newElement, PRBool * success)
{
  nsCOMPtr<nsIDOMNodeList> nodeList;
  nsCOMPtr<nsIDOMNode> node;
  nsCOMPtr<nsIDOMNode> parentNode;
  PRInt32 offset;
  nsCOMPtr<nsISelection>selection;
  nsresult res = NS_OK;
  if (success) *success = PR_FALSE;
  PRUint32 nodeCount = 0;
  res = newElement->GetElementsByTagName(NS_LITERAL_STRING("cursor"), getter_AddRefs(nodeList));
  if (nodeList) nodeList->GetLength(&nodeCount);
  if (nodeCount > 0)
  {
    nodeList->Item(0, getter_AddRefs(node));
    res = GetSelection(getter_AddRefs(selection));
    selection->Collapse(node, 0);
    DeleteNode(node);
    if (success) *success = PR_TRUE;

    // BBM: put a comment in latexdefs about how this works.
  }
//  cmd_updateStructToolbar
  return res;
}

// returns empty string if nothing to modify on node
nsresult
nsHTMLEditor::GetAttributeToModifyOnNode(nsIDOMNode *aNode, nsAString &aAttr)
{
  aAttr.Truncate();

  NS_NAMED_LITERAL_STRING(srcStr, "src");
  nsCOMPtr<nsIDOMHTMLImageElement> nodeAsImage = do_QueryInterface(aNode);
  if (nodeAsImage)
  {
    aAttr = srcStr;
    return NS_OK;
  }

  nsCOMPtr<nsIDOMHTMLAnchorElement> nodeAsAnchor = do_QueryInterface(aNode);
  if (nodeAsAnchor)
  {
    aAttr.AssignLiteral("href");
    return NS_OK;
  }

  NS_NAMED_LITERAL_STRING(bgStr, "background");
  nsCOMPtr<nsIDOMHTMLBodyElement> nodeAsBody = do_QueryInterface(aNode);
  if (nodeAsBody)
  {
    aAttr = bgStr;
    return NS_OK;
  }

  nsCOMPtr<nsIDOMHTMLTableElement> nodeAsTable = do_QueryInterface(aNode);
  if (nodeAsTable)
  {
    aAttr = bgStr;
    return NS_OK;
  }

  nsCOMPtr<nsIDOMHTMLTableRowElement> nodeAsTableRow = do_QueryInterface(aNode);
  if (nodeAsTableRow)
  {
    aAttr = bgStr;
    return NS_OK;
  }

  nsCOMPtr<nsIDOMHTMLTableCellElement> nodeAsTableCell = do_QueryInterface(aNode);
  if (nodeAsTableCell)
  {
    aAttr = bgStr;
    return NS_OK;
  }

  nsCOMPtr<nsIDOMHTMLScriptElement> nodeAsScript = do_QueryInterface(aNode);
  if (nodeAsScript)
  {
    aAttr = srcStr;
    return NS_OK;
  }

  nsCOMPtr<nsIDOMHTMLEmbedElement> nodeAsEmbed = do_QueryInterface(aNode);
  if (nodeAsEmbed)
  {
    aAttr = srcStr;
    return NS_OK;
  }

  nsCOMPtr<nsIDOMHTMLObjectElement> nodeAsObject = do_QueryInterface(aNode);
  if (nodeAsObject)
  {
    aAttr.AssignLiteral("data");
    return NS_OK;
  }

  nsCOMPtr<nsIDOMHTMLLinkElement> nodeAsLink = do_QueryInterface(aNode);
  if (nodeAsLink)
  {
    // Test if the link has a rel value indicating it to be a stylesheet
    nsAutoString linkRel;
    if (NS_SUCCEEDED(nodeAsLink->GetRel(linkRel)) && !linkRel.IsEmpty())
    {
      nsReadingIterator<PRUnichar> start;
      nsReadingIterator<PRUnichar> end;
      nsReadingIterator<PRUnichar> current;

      linkRel.BeginReading(start);
      linkRel.EndReading(end);

      // Walk through space delimited string looking for "stylesheet"
      for (current = start; current != end; ++current)
      {
        // Ignore whitespace
        if (nsCRT::IsAsciiSpace(*current))
          continue;

        // Grab the next space delimited word
        nsReadingIterator<PRUnichar> startWord = current;
        do {
          ++current;
        } while (current != end && !nsCRT::IsAsciiSpace(*current));

        // Store the link for fix up if it says "stylesheet"
        if (Substring(startWord, current).LowerCaseEqualsLiteral("stylesheet"))
        {
          aAttr.AssignLiteral("href");
          return NS_OK;
        }
        if (current == end)
          break;
      }
    }
    return NS_OK;
  }

  nsCOMPtr<nsIDOMHTMLFrameElement> nodeAsFrame = do_QueryInterface(aNode);
  if (nodeAsFrame)
  {
    aAttr = srcStr;
    return NS_OK;
  }

  nsCOMPtr<nsIDOMHTMLIFrameElement> nodeAsIFrame = do_QueryInterface(aNode);
  if (nodeAsIFrame)
  {
    aAttr = srcStr;
    return NS_OK;
  }

  nsCOMPtr<nsIDOMHTMLInputElement> nodeAsInput = do_QueryInterface(aNode);
  if (nodeAsInput)
  {
    aAttr = srcStr;
    return NS_OK;
  }

  return NS_OK;
}

nsresult
nsHTMLEditor::RelativizeURIForNode(nsIDOMNode *aNode, nsIURL *aDestURL)
{
  nsAutoString attributeToModify;
  GetAttributeToModifyOnNode(aNode, attributeToModify);
  if (attributeToModify.IsEmpty())
    return NS_OK;

  nsCOMPtr<nsIDOMNamedNodeMap> attrMap;
  nsresult rv = aNode->GetAttributes(getter_AddRefs(attrMap));
  NS_ENSURE_SUCCESS(rv, NS_OK);
  if (!attrMap) return NS_OK; // assume errors here shouldn't cancel insertion

  nsCOMPtr<nsIDOMNode> attrNode;
  rv = attrMap->GetNamedItem(attributeToModify, getter_AddRefs(attrNode));
  NS_ENSURE_SUCCESS(rv, NS_OK); // assume errors here shouldn't cancel insertion

  if (attrNode)
  {
    nsAutoString oldValue;
    attrNode->GetNodeValue(oldValue);
    if (!oldValue.IsEmpty())
    {
      NS_ConvertUTF16toUTF8 oldCValue(oldValue);
      nsCOMPtr<nsIURI> currentNodeURI;
      rv = NS_NewURI(getter_AddRefs(currentNodeURI), oldCValue);
      if (NS_SUCCEEDED(rv))
      {
        nsCAutoString newRelativePath;
        aDestURL->GetRelativeSpec(currentNodeURI, newRelativePath);
        if (!newRelativePath.IsEmpty())
        {
          NS_ConvertUTF8toUTF16 newCValue(newRelativePath);
          attrNode->SetNodeValue(newCValue);
        }
      }
    }
  }

  return NS_OK;
}

nsresult
nsHTMLEditor::RelativizeURIInFragmentList(const nsCOMArray<nsIDOMNode> &aNodeList,
                                          const nsAString &aFlavor,
                                          nsIDOMDocument *aSourceDoc,
                                          nsIDOMNode *aTargetNode)
{
  // determine destination URL
  nsCOMPtr<nsIDOMDocument> domDoc;
  GetDocument(getter_AddRefs(domDoc));
  nsCOMPtr<nsIDocument> destDoc = do_QueryInterface(domDoc);
  if (!destDoc) return NS_ERROR_FAILURE;

  nsCOMPtr<nsIURL> destURL = do_QueryInterface(destDoc->GetDocumentURI());
  if (!destURL) return NS_ERROR_FAILURE;

  // brade: eventually should look for a base url in the document if present

  nsresult rv;
  nsCOMPtr<nsIDOMDocumentTraversal> trav = do_QueryInterface(domDoc, &rv);
  NS_ENSURE_SUCCESS(rv, NS_ERROR_FAILURE);

  PRInt32 listCount = aNodeList.Count();
  PRInt32 j;
  for (j = 0; j < listCount; j++)
  {
    nsIDOMNode* somenode = aNodeList[j];

    nsCOMPtr<nsIDOMTreeWalker> walker;
    rv = trav->CreateTreeWalker(somenode, nsIDOMNodeFilter::SHOW_ELEMENT,
                                nsnull, PR_TRUE, getter_AddRefs(walker));
    NS_ENSURE_SUCCESS(rv, rv);

    nsCOMPtr<nsIDOMNode> currentNode;
    walker->GetCurrentNode(getter_AddRefs(currentNode));
    while (currentNode)
    {
      rv = RelativizeURIForNode(currentNode, destURL);
      NS_ENSURE_SUCCESS(rv, rv);

      walker->NextNode(getter_AddRefs(currentNode));
    }
  }

  return NS_OK;
}

nsresult
nsHTMLEditor::AddInsertionListener(nsIContentFilter *aListener)
{
  if (!aListener)
    return NS_ERROR_NULL_POINTER;

  // don't let a listener be added more than once
  if (mContentFilters.IndexOfObject(aListener) == -1)
  {
    if (!mContentFilters.AppendObject(aListener))
      return NS_ERROR_FAILURE;
  }

  return NS_OK;
}

nsresult
nsHTMLEditor::RemoveInsertionListener(nsIContentFilter *aListener)
{
  if (!aListener)
    return NS_ERROR_FAILURE;

  if (!mContentFilters.RemoveObject(aListener))
    return NS_ERROR_FAILURE;

  return NS_OK;
}

nsresult
nsHTMLEditor::DoContentFilterCallback(const nsAString &aFlavor,
                                      nsIDOMDocument *sourceDoc,
                                      PRBool aWillDeleteSelection,
                                      nsIDOMNode **aFragmentAsNode,
                                      nsIDOMNode **aFragStartNode,
                                      PRInt32 *aFragStartOffset,
                                      nsIDOMNode **aFragEndNode,
                                      PRInt32 *aFragEndOffset,
                                      nsIDOMNode **aTargetNode,
                                      PRInt32 *aTargetOffset,
                                      PRBool *aDoContinue)
{
  *aDoContinue = PR_TRUE;

  PRInt32 i;
  nsIContentFilter *listener;
  for (i=0; i < mContentFilters.Count() && *aDoContinue; i++)
  {
    listener = (nsIContentFilter *)mContentFilters[i];
    if (listener)
      listener->NotifyOfInsertion(aFlavor, nsnull, sourceDoc,
                                  aWillDeleteSelection, aFragmentAsNode,
                                  aFragStartNode, aFragStartOffset,
                                  aFragEndNode, aFragEndOffset,
                                  aTargetNode, aTargetOffset, aDoContinue);
  }

  return NS_OK;
}

PRBool
nsHTMLEditor::IsInLink(nsIDOMNode *aNode, nsCOMPtr<nsIDOMNode> *outLink)
{
  if (!aNode)
    return PR_FALSE;
  if (outLink)
    *outLink = nsnull;
  nsCOMPtr<nsIDOMNode> tmp, node = aNode;
  while (node)
  {
    if (nsHTMLEditUtils::IsLink(node, mtagListManager))
    {
      if (outLink)
        *outLink = node;
      return PR_TRUE;
    }
    tmp = node;
    tmp->GetParentNode(getter_AddRefs(node));
  }
  return PR_FALSE;
}


nsresult
nsHTMLEditor::StripFormattingNodes(nsIDOMNode *aNode, PRBool aListOnly)
// BBM: This seems to be removing white space that formats the XHTML *source*, not
// the rendered XHTML
{
  NS_ENSURE_TRUE(aNode, NS_ERROR_NULL_POINTER);

  nsresult res = NS_OK;
  nsCOMPtr<nsIContent> content = do_QueryInterface(aNode);
  if (content->TextIsOnlyWhitespace())
  {
    nsCOMPtr<nsIDOMNode> parent, ignored;
    aNode->GetParentNode(getter_AddRefs(parent));
    if (parent)
    {
      if (!aListOnly || nsHTMLEditUtils::IsList(parent, mtagListManager))
        res = parent->RemoveChild(aNode, getter_AddRefs(ignored));
      return res;
    }
  }

  if (!nsHTMLEditUtils::IsPre(aNode, mtagListManager))
  {
    nsCOMPtr<nsIDOMNode> child;
    aNode->GetLastChild(getter_AddRefs(child));

    while (child)
    {
      nsCOMPtr<nsIDOMNode> tmp;
      child->GetPreviousSibling(getter_AddRefs(tmp));
      res = StripFormattingNodes(child, aListOnly);
      NS_ENSURE_SUCCESS(res, res);
      child = tmp;
    }
  }
  return res;
}

NS_IMETHODIMP nsHTMLEditor::PrepareTransferable(nsITransferable **transferable)
{
  return NS_OK;
}

NS_IMETHODIMP nsHTMLEditor::PrepareHTMLTransferable(nsITransferable **aTransferable,
                                                    PRBool aHavePrivFlavor)
{
  // Create generic Transferable for getting the data
  nsresult rv = CallCreateInstance("@mozilla.org/widget/transferable;1", aTransferable);
  if (NS_FAILED(rv))
    return rv;

  // Get the nsITransferable interface for getting the data from the clipboard
  if (aTransferable)
  {
    // Create the desired DataFlavor for the type of data
    // we want to get out of the transferable
    if ((mFlags & eEditorPlaintextMask) == 0)  // This should only happen in html editors, not plaintext
    {
    (*aTransferable)->AddDataFlavor(kJPEGImageMime);
    if (!aHavePrivFlavor)
      {
        (*aTransferable)->AddDataFlavor(kNativeHTMLMime);
      }
      (*aTransferable)->AddDataFlavor(kHTMLMime);
      (*aTransferable)->AddDataFlavor(kFileMime);
      // image pasting from the clipboard is only implemented on Windows & Mac right now.
    }
    (*aTransferable)->AddDataFlavor(kUnicodeMime);
  }

  return NS_OK;
}

PRInt32
FindPositiveIntegerAfterString(const char *aLeadingString, nsCString &aCStr)
{
  // first obtain offsets from cfhtml str
  PRInt32 numFront = aCStr.Find(aLeadingString);
  if (numFront == -1)
    return -1;
  numFront += strlen(aLeadingString);

  PRInt32 numBack = aCStr.FindCharInSet(CRLF, numFront);
  if (numBack == -1)
    return -1;

  nsCAutoString numStr(Substring(aCStr, numFront, numBack-numFront));
  PRInt32 errorCode;
  return numStr.ToInteger(&errorCode);
}

nsresult
RemoveFragComments(nsCString & aStr)
{
  // remove the StartFragment/EndFragment comments from the str, if present
  PRInt32 startCommentIndx = aStr.Find("<!--StartFragment");
  if (startCommentIndx >= 0)
  {
    PRInt32 startCommentEnd = aStr.Find("-->", PR_FALSE, startCommentIndx);
    if (startCommentEnd > startCommentIndx)
      aStr.Cut(startCommentIndx, (startCommentEnd+3)-startCommentIndx);
  }
  PRInt32 endCommentIndx = aStr.Find("<!--EndFragment");
  if (endCommentIndx >= 0)
  {
    PRInt32 endCommentEnd = aStr.Find("-->", PR_FALSE, endCommentIndx);
    if (endCommentEnd > endCommentIndx)
      aStr.Cut(endCommentIndx, (endCommentEnd+3)-endCommentIndx);
  }
  return NS_OK;
}

nsresult
nsHTMLEditor::ParseCFHTML(nsCString & aCfhtml, PRUnichar **aStuffToPaste, PRUnichar **aCfcontext)
{
  // first obtain offsets from cfhtml str
  PRInt32 startHTML     = FindPositiveIntegerAfterString("StartHTML:", aCfhtml);
  PRInt32 endHTML       = FindPositiveIntegerAfterString("EndHTML:", aCfhtml);
  PRInt32 startFragment = FindPositiveIntegerAfterString("StartFragment:", aCfhtml);
  PRInt32 endFragment   = FindPositiveIntegerAfterString("EndFragment:", aCfhtml);

  if ((startHTML<0) || (endHTML<0) || (startFragment<0) || (endFragment<0))
    return NS_ERROR_FAILURE;

  // create context string
  nsCAutoString contextUTF8(Substring(aCfhtml, startHTML, startFragment - startHTML) +
                            NS_LITERAL_CSTRING("<!--" kInsertCookie "-->") +
                            Substring(aCfhtml, endFragment, endHTML - endFragment));

  // validate startFragment
  // make sure it's not in the middle of a HTML tag
  // see bug #228879 for more details
  PRInt32 curPos = startFragment;
  while (curPos > startHTML)
  {
      if (aCfhtml[curPos] == '>')
      {
          // working backwards, the first thing we see is the end of a tag
          // so StartFragment is good, so do nothing.
          break;
      }
      else if (aCfhtml[curPos] == '<')
      {
          // if we are at the start, then we want to see the '<'
          if (curPos != startFragment)
          {
              // working backwards, the first thing we see is the start of a tag
              // so StartFragment is bad, so we need to update it.
              NS_ASSERTION(0, "StartFragment byte count in the clipboard looks bad, see bug #228879");
              startFragment = curPos - 1;
          }
          break;
      }
      else
      {
          curPos--;
      }
  }

  // create fragment string
  nsCAutoString fragmentUTF8(Substring(aCfhtml, startFragment, endFragment-startFragment));

  // remove the StartFragment/EndFragment comments from the fragment, if present
  RemoveFragComments(fragmentUTF8);

  // remove the StartFragment/EndFragment comments from the context, if present
  RemoveFragComments(contextUTF8);

  // convert both strings to usc2
  const nsAFlatString& fragUcs2Str = NS_ConvertUTF8toUTF16(fragmentUTF8);
  const nsAFlatString& cntxtUcs2Str = NS_ConvertUTF8toUTF16(contextUTF8);

  // translate platform linebreaks for fragment
  PRInt32 oldLengthInChars = fragUcs2Str.Length() + 1;  // +1 to include null terminator
  PRInt32 newLengthInChars = 0;
  *aStuffToPaste = nsLinebreakConverter::ConvertUnicharLineBreaks(fragUcs2Str.get(),
                                                           nsLinebreakConverter::eLinebreakAny,
                                                           nsLinebreakConverter::eLinebreakContent,
                                                           oldLengthInChars, &newLengthInChars);
  if (!aStuffToPaste)
  {
    return NS_ERROR_FAILURE;
  }

  // translate platform linebreaks for context
  oldLengthInChars = cntxtUcs2Str.Length() + 1;  // +1 to include null terminator
  newLengthInChars = 0;
  *aCfcontext = nsLinebreakConverter::ConvertUnicharLineBreaks(cntxtUcs2Str.get(),
                                                           nsLinebreakConverter::eLinebreakAny,
                                                           nsLinebreakConverter::eLinebreakContent,
                                                           oldLengthInChars, &newLengthInChars);
  // it's ok for context to be empty.  frag might be whole doc and contain all it's context.

  // we're done!
  return NS_OK;
}


// When this function is called, the graphics file has already been copied to the graphics directory, but it has not been converted.
nsresult
nsHTMLEditor::InsertGraphicsFileAsImage(nsAString& fileLeaf,
                                        nsAString& path,
                                        nsIDOMNode *aDestinationNode,
                                        PRInt32 aDestOffset,
                                        PRBool aDoDeleteSelection)
{
  nsCOMPtr<nsIDOMElement> frame;
  nsCOMPtr<nsIDOMElement> obj;
  nsCOMPtr<nsIDOMNode> framenode;
  nsCOMPtr<nsIDOMNode> objnode;
  nsCOMPtr<nsIDOMNode> destnode;
  nsCOMPtr<nsIDOMNodeList> paranodes;
  nsCOMPtr<nsIDOMNode> dummynode;
  nsCOMPtr<nsIDOMNode> paranode;
  nsCOMPtr<nsIDOMText> text;
  nsCOMPtr<nsIDOMText> text2;
  nsCOMPtr<nsISelection> sel;
  PRUint32 paranodesLength;
  PRInt32 index;
  nsCOMPtr<nsIDOMNode> parent;
  nsresult rv;
  PRUint16 nodetype;

  if (fileLeaf.Length() == 0) return NS_ERROR_FAILURE;
  GetSelection(getter_AddRefs(sel));
  if (aDestinationNode) destnode = aDestinationNode;
  else {
    sel->GetAnchorNode(getter_AddRefs(destnode));
    sel->GetAnchorOffset(&aDestOffset);
  }

  // The following doesn't work if destnode is a text node, so split if necessary
  destnode->GetNodeType(&nodetype);
  if (nodetype == 3 ) { //NODETYPE_TEXT
    text = do_QueryInterface(destnode);
    if (text) {
      destnode->GetParentNode(getter_AddRefs(parent));
      index = GetIndexOf(parent, destnode);
      text->SplitText( aDestOffset, getter_AddRefs(text2) );
      destnode = parent;
      aDestOffset = index+1;
    }
  }

  CreateFrameWithDefaults(NS_LITERAL_STRING("image"), PR_TRUE, destnode, aDestOffset, getter_AddRefs(frame));
  if (frame == nsnull) return NS_ERROR_FAILURE;
  // for images, we don't want any paragraphs in the frame before we put in an optional caption
  frame->GetElementsByTagName(NS_LITERAL_STRING("bodyText"), getter_AddRefs(paranodes));
  paranodes->GetLength(&paranodesLength);
  if (paranodesLength > 0) {
    paranodes->Item(0, getter_AddRefs(paranode));
    frame->RemoveChild(paranode, getter_AddRefs(dummynode));
  }
  GetFrameStyleFromAttributes(frame);
  frame->GetParentNode(getter_AddRefs(parent));
  if (parent == nsnull) return NS_ERROR_FAILURE;
  CreateNode(NS_LITERAL_STRING("object"), frame, 0, getter_AddRefs(objnode));
  obj = do_QueryInterface(objnode);
  obj->SetAttribute(NS_LITERAL_STRING("data"), NS_LITERAL_STRING("graphics/") + fileLeaf);
  InsertNodeAtPoint(objnode, (nsIDOMNode **)address_of(framenode), 0, PR_FALSE);
  if (sel) {
    sel->Collapse(frame, 0);
  }
  obj = do_QueryInterface(objnode);
  obj->SetAttribute(NS_LITERAL_STRING("originalSrcUrl"), path);
  obj->SetAttribute(NS_LITERAL_STRING("copiedSrcUrl"), NS_LITERAL_STRING("graphics/") + fileLeaf);

  // obj = do_QueryInterface(objnode);
  GetGraphicsAttributesFromFrame(frame, obj);

  // call a command implemented in JavaScript
  nsCOMPtr<nsIDOMDocument> ownerDoc;
  destnode->GetOwnerDocument(getter_AddRefs(ownerDoc));
  NS_ENSURE_STATE(ownerDoc);

  nsCOMPtr<nsIDOMDocumentView> docView = do_QueryInterface(ownerDoc);
  NS_ENSURE_STATE(docView);

  nsCOMPtr<nsIDOMAbstractView> absView;
  docView->GetDefaultView(getter_AddRefs(absView));
  NS_ENSURE_STATE(absView);

  nsCOMPtr<nsIDOMWindowInternal> win = do_QueryInterface(absView);

  nsCOMPtr<nsIControllers> controllers;
  rv = win->GetControllers(getter_AddRefs(controllers));
  nsCOMPtr<nsIController> controller;
  const char * command = "cmd_convert_graphics_at_selection";
  rv = controllers->GetControllerForCommand(command, getter_AddRefs(controller));
  if (NS_FAILED(rv)) return rv;
  controller->DoCommand(command);
  return NS_OK;
}

NS_IMETHODIMP nsHTMLEditor::InsertFromTransferable(nsITransferable *transferable,
                                                   nsIDOMDocument *aSourceDoc,
                                                   const nsAString & aContextStr,
                                                   const nsAString & aInfoStr,
                                                   nsIDOMNode *aDestinationNode,
                                                   PRInt32 aDestOffset,
                                                   PRBool aDoDeleteSelection)
{
  nsresult rv = NS_OK;
  PRBool fExists;
  nsCAutoString dirPath;
  nsAutoString path;
  nsXPIDLCString bestFlavor;
  nsCOMPtr<nsISelection> sel;
  nsCOMPtr<nsISupports> genericDataObj;
  PRUint32 len = 0;
  if ( NS_SUCCEEDED(transferable->GetAnyTransferData(getter_Copies(bestFlavor), getter_AddRefs(genericDataObj), &len)) )
  {
    nsAutoTxnsConserveSelection dontSpazMySelection(this);
    nsAutoString flavor;
    flavor.AssignWithConversion(bestFlavor);
    nsAutoString stuffToPaste;
#ifdef DEBUG_clipboard
    printf("Got flavor [%s]\n", bestFlavor);
#endif
    if (0 == nsCRT::strcmp(bestFlavor, kNativeHTMLMime))
    {
      // note cf_html uses utf8, hence use length = len, not len/2 as in flavors below
      nsCOMPtr<nsISupportsCString> textDataObj(do_QueryInterface(genericDataObj));
      if (textDataObj && len > 0)
      {
        nsCAutoString cfhtml;
        textDataObj->GetData(cfhtml);
        NS_ASSERTION(cfhtml.Length() <= (len), "Invalid length!");
        nsXPIDLString cfcontext, cffragment, cfselection; // cfselection left emtpy for now

        rv = ParseCFHTML(cfhtml, getter_Copies(cffragment), getter_Copies(cfcontext));
        if (NS_SUCCEEDED(rv) && !cffragment.IsEmpty())
        {
          nsAutoEditBatch beginBatching(this);
          rv = InsertHTMLWithContext(cffragment,
                                     cfcontext, cfselection, flavor,
                                     aSourceDoc,
                                     aDestinationNode, aDestOffset,
                                     aDoDeleteSelection);
        }
      }
    }
    else if (0 == nsCRT::strcmp(bestFlavor, kHTMLMime))
    {
      nsCOMPtr<nsISupportsString> textDataObj(do_QueryInterface(genericDataObj));
      if (textDataObj && len > 0)
      {
        nsAutoString text;
        textDataObj->GetData(text);
        NS_ASSERTION(text.Length() <= (len/2), "Invalid length!");
        stuffToPaste.Assign(text.get(), len / 2);
        nsAutoEditBatch beginBatching(this);
        rv = InsertHTMLWithContext(stuffToPaste,
                                   aContextStr, aInfoStr, flavor,
                                   aSourceDoc,
                                   aDestinationNode, aDestOffset,
                                   aDoDeleteSelection);
      }
    }
   else if (0 == nsCRT::strcmp(bestFlavor, kFileMime))
    {
      nsCOMPtr<nsIFile> fileObj(do_QueryInterface(genericDataObj));
      if (fileObj && len > 0)
      {

        nsCOMPtr<nsIURI> uri;
        rv = NS_NewFileURI(getter_AddRefs(uri), fileObj);
        if (NS_FAILED(rv))
          return rv;

        nsCOMPtr<nsIFileURL> fileURL(do_QueryInterface(uri));
        if (fileURL)
        {
          PRBool insertAsImage = PR_FALSE;
          PRBool insertAsLink = PR_FALSE; // nothing currently sets this to true. In the future we may allow user prefs
           // to determine if dropping a file inserts a link or opens the file.
          nsCAutoString fileextension;
          rv = fileURL->GetFileExtension(fileextension);

          if (NS_SUCCEEDED(rv) && !fileextension.IsEmpty())
          {
            ToLowerCase(fileextension);
            if ( fileextension.EqualsLiteral("jpg")
              || fileextension.EqualsLiteral("jpeg")
              || fileextension.EqualsLiteral("gif")
              || fileextension.EqualsLiteral("png")
              || fileextension.EqualsLiteral("tif")
              || fileextension.EqualsLiteral("tiff")
              || fileextension.EqualsLiteral("pdf")
              || fileextension.EqualsLiteral("eps")
              || fileextension.EqualsLiteral("ps")
              || fileextension.EqualsLiteral("emf")
              || fileextension.EqualsLiteral("wmf")
              || fileextension.EqualsLiteral("svg")
              || fileextension.EqualsLiteral("mp4")
              || fileextension.EqualsLiteral("avi")
              || fileextension.EqualsLiteral("mov")
              || fileextension.EqualsLiteral("wmv")
#ifdef XP_WIN32
              || fileextension.EqualsLiteral("bmp")
#endif
            )
            {
              insertAsImage = PR_TRUE;
            }
          }

          nsCAutoString urltext;
          rv = fileURL->GetSpec(urltext);
          if (NS_SUCCEEDED(rv) && !urltext.IsEmpty())
          {
            if (insertAsImage || insertAsLink)
            {
              if (insertAsImage)
              {
                // nsCOMPtr<nsIDOMElement> frame;
                // nsCOMPtr<nsIDOMElement> obj;
                // nsCOMPtr<nsIDOMNode> framenode;
                // nsCOMPtr<nsIDOMNode> objnode;
                // nsCOMPtr<nsIDOMText> text;
                // nsCOMPtr<nsIDOMText> text2;
                // PRInt32 index;
                // nsCOMPtr<nsIDOMNode> parent;
                // // copy file to graphics directory. Insert image with path "graphics/imagename.xxx"
                nsresult rv;
                nsAutoString fileLeaf;
                PRUint16 nodetype;
                nsCOMPtr<nsILocalFile> dir;
                fileObj->GetLeafName(fileLeaf);
                rv = GetDocumentGraphicsDir(getter_AddRefs(dir));
                NS_ENSURE_SUCCESS(rv, rv);
                fileObj->CopyTo(dir, NS_LITERAL_STRING(""));
                fileObj->GetPath(path);

                InsertGraphicsFileAsImage( fileLeaf, path, aDestinationNode, aDestOffset, PR_FALSE);
              }
              else // insertAsLink
              {
                stuffToPaste.AssignLiteral("<a xmlns=\"http://www.w3.org/1999/xhtml\" href=\"");
                AppendUTF8toUTF16(urltext, stuffToPaste);
                stuffToPaste.AppendLiteral("\">");
                AppendUTF8toUTF16(urltext, stuffToPaste);
                stuffToPaste.AppendLiteral("</a>");

                nsAutoEditBatch beginBatching(this);

                const nsAFlatString& empty = EmptyString();
                rv = InsertHTMLWithContext(stuffToPaste,
                                           empty, empty, flavor,
                                           aSourceDoc,
                                           aDestinationNode, aDestOffset,
                                           aDoDeleteSelection);
              }
            }
            else // open new document window for file
            {
              nsCOMPtr<nsIWindowWatcher> wwatch(do_GetService(NS_WINDOWWATCHER_CONTRACTID));
              nsCOMPtr<nsISupportsString> urlWrapper(do_CreateInstance(NS_SUPPORTS_STRING_CONTRACTID));
              if (!wwatch || !urlWrapper)
                return NS_ERROR_FAILURE;

              urlWrapper->SetData(NS_ConvertUTF8toUTF16(urltext));
              nsCOMPtr<nsIDOMWindow> window;

              wwatch->OpenWindow(nsnull, "chrome://prince/content/prince.xul",
                                  "_blank", "chrome,dialog=no,all",
                                  urlWrapper, getter_AddRefs(window));
            }
          }
        }
      }
    }



    #define kPNGImageMime               "image/png"
    #define kJPEGImageMime              "image/jpg"
    #define kGIFImageMime               "image/gif"
    #define kFileMime                   "application/x-moz-file"
    #define kSVGImageMime               "image/svg+xml"
    #define kWin32EnhMetafile           "application/x-moz-win32-enhMetafile"
    #define kWin32MetafilePict          "application/x-moz-win32-metafilePict"

    else if (0 == nsCRT::strcmp(bestFlavor, kJPEGImageMime) ||
             0 == nsCRT::strcmp(bestFlavor, kPNGImageMime) ||
             0 == nsCRT::strcmp(bestFlavor, kGIFImageMime) ||
             0 == nsCRT::strcmp(bestFlavor, kSVGImageMime) ||
             0 == nsCRT::strcmp(bestFlavor, kWin32EnhMetafile) ||
             0 == nsCRT::strcmp(bestFlavor, kWin32MetafilePict))
    {
      nsAutoString leaf;
      nsAutoString ext;
      if (0 == nsCRT::strcmp(bestFlavor, kJPEGImageMime)) ext = NS_LITERAL_STRING("jpg");
      else if (0 == nsCRT::strcmp(bestFlavor, kPNGImageMime)) ext = NS_LITERAL_STRING("png");
      else if (0 == nsCRT::strcmp(bestFlavor, kGIFImageMime)) ext = NS_LITERAL_STRING("gif");
      else if (0 == nsCRT::strcmp(bestFlavor, kSVGImageMime)) ext = NS_LITERAL_STRING("svg");
      else if (0 == nsCRT::strcmp(bestFlavor, kWin32EnhMetafile)) ext = NS_LITERAL_STRING("emf");
      else if (0 == nsCRT::strcmp(bestFlavor, kWin32MetafilePict)) ext = NS_LITERAL_STRING("wmf");
      leaf = NS_LITERAL_STRING("copied.") + ext;
      nsCOMPtr<nsIInputStream> imageStream(do_QueryInterface(genericDataObj));
      NS_ENSURE_TRUE(imageStream, NS_ERROR_FAILURE);

      nsCOMPtr<nsILocalFile> fileToUse;
      rv = GetDocumentGraphicsDir(getter_AddRefs(fileToUse));
      NS_ENSURE_SUCCESS(rv, rv);

      fileToUse->Append(leaf);
      nsCOMPtr<nsILocalFile> pathpath = do_QueryInterface(fileToUse);
      pathpath->CreateUnique(nsIFile::NORMAL_FILE_TYPE, 0755);

      nsCOMPtr<nsIOutputStream> outputStream;
      rv = NS_NewLocalFileOutputStream(getter_AddRefs(outputStream), fileToUse);
      NS_ENSURE_SUCCESS(rv, rv);

      PRUint32 length;
      imageStream->Available(&length);

      nsCOMPtr<nsIOutputStream> bufferedOutputStream;
      rv = NS_NewBufferedOutputStream(getter_AddRefs(bufferedOutputStream), outputStream, length);
      NS_ENSURE_SUCCESS(rv, rv);

      PRUint32 numWritten;
      rv = bufferedOutputStream->WriteFrom(imageStream, length, &numWritten);
      NS_ENSURE_SUCCESS(rv, rv);

      // force the stream close before we try to insert the image
      // into the document.
      rv = bufferedOutputStream->Close();
      NS_ENSURE_SUCCESS(rv, rv);
      // at this point the jpeg data has been written to the graphics directory and is ready to embed.
      fileToUse->GetLeafName(leaf);
      fileToUse->GetPath(path);
      // the filename may have been modified to something like 'clipboard_copy-1.jpg'

      InsertGraphicsFileAsImage(leaf, path, aDestinationNode, aDestOffset, PR_FALSE);


//       nsAutoString urltext;
//       urltext.Append(NS_LITERAL_STRING("graphics/"));
//       nsAutoString leafname;
//       fileToUse->GetLeafName(leafname);
//       urltext.Append(leafname);
//       if (NS_SUCCEEDED(rv) && !urltext.IsEmpty())
//       {
// //        stuffToPaste.AssignLiteral("<object xmlns=\"http://www.w3.org/1999/xhtml\" data=\"");
//         stuffToPaste.AssignLiteral("<object data=\"");
//         stuffToPaste.Append(urltext);
//         stuffToPaste.AppendLiteral("\" alt=\"\"");
//         nsAutoString attributeStr;
//         rv = GetGraphicsDefaultsAsString( attributeStr);
//         if (NS_SUCCEEDED(rv) && attributeStr.Length())
//         {
//           stuffToPaste.Append(PRUnichar(' '));
//           stuffToPaste += attributeStr;
//         }
//         stuffToPaste.AppendLiteral(" />");
//         nsAutoEditBatch beginBatching(this);
//         nsCOMPtr<nsIDOMDocument> domDoc;
//         GetDocument(getter_AddRefs(domDoc));

//         rv = InsertHTMLWithContext(stuffToPaste, EmptyString(), EmptyString(),
//                                    NS_LITERAL_STRING(kNativeHTMLMime),
//                                    domDoc, // this says the graphics file is already in this document and doesn't need to be copied.
//                                    aDestinationNode, aDestOffset,
//                                    aDoDeleteSelection);
//       }
    }
    else if (0 == nsCRT::strcmp(bestFlavor, kUnicodeMime))
    {
      nsCOMPtr<nsISupportsString> textDataObj(do_QueryInterface(genericDataObj));
      if (textDataObj && len > 0)
      {
        nsAutoString text;
        textDataObj->GetData(text);
        NS_ASSERTION(text.Length() <= (len/2), "Invalid length!");
        stuffToPaste.Assign(text.get(), len / 2);
        nsAutoEditBatch beginBatching(this);
        // need to provide a hook from this point
        rv = InsertTextAt(stuffToPaste, aDestinationNode, aDestOffset, aDoDeleteSelection);
      }
    }
  }

  // Try to scroll the selection into view if the paste/drop succeeded
  // After ScrollSelectionIntoView(), the pending notifications might be
  // flushed and PresShell/PresContext/Frames may be dead. See bug 418470.
  if (NS_SUCCEEDED(rv))
    ScrollSelectionIntoView(PR_FALSE);

  return rv;
}

NS_IMETHODIMP nsHTMLEditor::InsertFromDrop(nsIDOMEvent* aDropEvent)
{
  ForceCompositionEnd();

  nsresult rv;
  nsCOMPtr<nsIDragService> dragService =
           do_GetService("@mozilla.org/widget/dragservice;1", &rv);
  if (NS_FAILED(rv)) return rv;

  nsCOMPtr<nsIDragSession> dragSession;
  dragService->GetCurrentSession(getter_AddRefs(dragSession));
  if (!dragSession) return NS_OK;

  // transferable hooks here
  nsCOMPtr<nsIDOMDocument> domdoc;
  GetDocument(getter_AddRefs(domdoc));
  if (!nsEditorHookUtils::DoAllowDropHook(domdoc, aDropEvent, dragSession))
    return NS_OK;

  // find out if we have our internal html flavor on the clipboard.  We don't want to mess
  // around with cfhtml if we do.
  PRBool bHavePrivateHTMLFlavor = PR_FALSE;
  rv = dragSession->IsDataFlavorSupported(kHTMLContext, &bHavePrivateHTMLFlavor);
  if (NS_FAILED(rv)) return rv;

  // Get the nsITransferable interface for getting the data from the drop
  nsCOMPtr<nsITransferable> trans;
  rv = PrepareHTMLTransferable(getter_AddRefs(trans), bHavePrivateHTMLFlavor);
  if (NS_FAILED(rv)) return rv;
  if (!trans) return NS_OK;  // NS_ERROR_FAILURE; SHOULD WE FAIL?

  PRUint32 numItems = 0;
  rv = dragSession->GetNumDropItems(&numItems);
  if (NS_FAILED(rv)) return rv;

  // Combine any deletion and drop insertion into one transaction
  nsAutoEditBatch beginBatching(this);

  // We never have to delete if selection is already collapsed
  PRBool deleteSelection = PR_FALSE;
  nsCOMPtr<nsIDOMNode> newSelectionParent;
  PRInt32 newSelectionOffset = 0;

  // Source doc is null if source is *not* the current editor document
  nsCOMPtr<nsIDOMDocument> srcdomdoc;
  rv = dragSession->GetSourceDocument(getter_AddRefs(srcdomdoc));
  if (NS_FAILED(rv)) return rv;

  PRUint32 i;
  PRBool doPlaceCaret = PR_TRUE;
  for (i = 0; i < numItems; ++i)
  {
    rv = dragSession->GetData(trans, i);
    if (NS_FAILED(rv)) return rv;
    if (!trans) return NS_OK; // NS_ERROR_FAILURE; Should we fail?

    // get additional html copy hints, if present
    nsAutoString contextStr, infoStr;
    nsCOMPtr<nsISupports> contextDataObj, infoDataObj;
    PRUint32 contextLen, infoLen;
    nsCOMPtr<nsISupportsString> textDataObj;

    nsCOMPtr<nsITransferable> contextTrans =
                      do_CreateInstance("@mozilla.org/widget/transferable;1");
    NS_ENSURE_TRUE(contextTrans, NS_ERROR_NULL_POINTER);
    contextTrans->AddDataFlavor(kHTMLContext);
    dragSession->GetData(contextTrans, i);
    contextTrans->GetTransferData(kHTMLContext, getter_AddRefs(contextDataObj), &contextLen);

    nsCOMPtr<nsITransferable> infoTrans =
                      do_CreateInstance("@mozilla.org/widget/transferable;1");
    NS_ENSURE_TRUE(infoTrans, NS_ERROR_NULL_POINTER);
    infoTrans->AddDataFlavor(kHTMLInfo);
    dragSession->GetData(infoTrans, i);
    infoTrans->GetTransferData(kHTMLInfo, getter_AddRefs(infoDataObj), &infoLen);

    if (contextDataObj)
    {
      nsAutoString text;
      textDataObj = do_QueryInterface(contextDataObj);
      textDataObj->GetData(text);
      NS_ASSERTION(text.Length() <= (contextLen/2), "Invalid length!");
      contextStr.Assign(text.get(), contextLen / 2);
    }

    if (infoDataObj)
    {
      nsAutoString text;
      textDataObj = do_QueryInterface(infoDataObj);
      textDataObj->GetData(text);
      NS_ASSERTION(text.Length() <= (infoLen/2), "Invalid length!");
      infoStr.Assign(text.get(), infoLen / 2);
    }

    if (doPlaceCaret)
    {
      // check if the user pressed the key to force a copy rather than a move
      // if we run into problems here, we'll just assume the user doesn't want a copy
      PRBool userWantsCopy = PR_FALSE;

      nsCOMPtr<nsIDOMNSUIEvent> nsuiEvent(do_QueryInterface(aDropEvent));
      if (!nsuiEvent) return NS_ERROR_FAILURE;

      nsCOMPtr<nsIDOMMouseEvent> mouseEvent(do_QueryInterface(aDropEvent));
      if (mouseEvent)

#if defined(XP_MACOSX)
        GetIsAltDown(&userWantsCopy);
#else
        GetIsCtrlDown(&userWantsCopy);
#endif

      // Current doc is destination
      nsCOMPtr<nsIDOMDocument>destdomdoc;
      rv = GetDocument(getter_AddRefs(destdomdoc));
      if (NS_FAILED(rv)) return rv;

      nsCOMPtr<nsISelection> selection;
      rv = GetSelection(getter_AddRefs(selection));
      // This selection is the *source* selection
      if (NS_FAILED(rv)) return rv;
      if (!selection) return NS_ERROR_FAILURE;

      PRBool isCollapsed;
      rv = selection->GetIsCollapsed(&isCollapsed);
      if (NS_FAILED(rv)) return rv;

      // Parent and offset under the mouse cursor
      rv = nsuiEvent->GetRangeParent(getter_AddRefs(newSelectionParent));
      if (NS_FAILED(rv)) return rv;
      if (!newSelectionParent) return NS_ERROR_FAILURE;

      rv = nsuiEvent->GetRangeOffset(&newSelectionOffset);
      if (NS_FAILED(rv)) return rv;

      // XXX: This userSelectNode code is a workaround for bug 195957.
      //
      // Check to see if newSelectionParent is part of a "-moz-user-select: all"
      // subtree. If it is, we need to make sure we don't drop into it!

      nsCOMPtr<nsIDOMNode> userSelectNode = FindUserSelectAllNode(newSelectionParent);

      if (userSelectNode)
      {
        // The drop is happening over a "-moz-user-select: all"
        // subtree so make sure the content we insert goes before
        // the root of the subtree.
        //
        // XXX: Note that inserting before the subtree matches the
        //      current behavior when dropping on top of an image.
        //      The decision for dropping before or after the
        //      subtree should really be done based on coordinates.

        rv = GetNodeLocation(userSelectNode, address_of(newSelectionParent),
                             &newSelectionOffset);

        if (NS_FAILED(rv)) return rv;
        if (!newSelectionParent) return NS_ERROR_FAILURE;
      }

      // We never have to delete if selection is already collapsed
      PRBool cursorIsInSelection = PR_FALSE;

      // Check if mouse is in the selection
      if (!isCollapsed)
      {
        PRInt32 rangeCount;
        rv = selection->GetRangeCount(&rangeCount);
        if (NS_FAILED(rv))
          return rv;

        for (PRInt32 j = 0; j < rangeCount; j++)
        {
          nsCOMPtr<nsIDOMRange> range;

          rv = selection->GetRangeAt(j, getter_AddRefs(range));
          if (NS_FAILED(rv) || !range)
            continue;//don't bail yet, iterate through them all

          nsCOMPtr<nsIDOMNSRange> nsrange(do_QueryInterface(range));
          if (NS_FAILED(rv) || !nsrange)
            continue;//don't bail yet, iterate through them all

          rv = nsrange->IsPointInRange(newSelectionParent, newSelectionOffset, &cursorIsInSelection);
          if(cursorIsInSelection)
            break;
        }
        if (cursorIsInSelection)
        {
          // Dragging within same doc can't drop on itself -- leave!
          // (We shouldn't get here - drag event shouldn't have started if over selection)
          if (srcdomdoc == destdomdoc)
            return NS_OK;

          // Dragging from another window onto a selection
          // XXX Decision made to NOT do this,
          //     note that 4.x does replace if dropped on
          //deleteSelection = PR_TRUE;
        }
        else
        {
          // We are NOT over the selection
          if (srcdomdoc == destdomdoc)
          {
            // Within the same doc: delete if user doesn't want to copy
            deleteSelection = !userWantsCopy;
          }
          else
          {
            // Different source doc: Don't delete
            deleteSelection = PR_FALSE;
          }
        }
      }

      // We have to figure out whether to delete/relocate caret only once
      doPlaceCaret = PR_FALSE;
    }

    // handle transferable hooks
    if (!nsEditorHookUtils::DoInsertionHook(domdoc, aDropEvent, trans))
      return NS_OK;

    // Beware! This may flush notifications via synchronous
    // ScrollSelectionIntoView.
    rv = InsertFromTransferable(trans, srcdomdoc, contextStr, infoStr,
                                newSelectionParent,
                                newSelectionOffset, deleteSelection);
  }

  return rv;
}

NS_IMETHODIMP nsHTMLEditor::CanDrag(nsIDOMEvent *aDragEvent, PRBool *aCanDrag)
{
  return nsPlaintextEditor::CanDrag(aDragEvent, aCanDrag);
}

nsresult
nsHTMLEditor::PutDragDataInTransferable(nsITransferable **aTransferable)
{
  NS_ENSURE_ARG_POINTER(aTransferable);
  *aTransferable = nsnull;
  nsCOMPtr<nsIDocumentEncoder> docEncoder;
  nsresult rv = SetupDocEncoder(getter_AddRefs(docEncoder));
  if (NS_FAILED(rv)) return rv;
  NS_ENSURE_TRUE(docEncoder, NS_ERROR_FAILURE);

  // grab a string
  nsAutoString buffer, parents, info;

  // find out if we're a plaintext control or not
  PRUint32 editorFlags = 0;
  rv = GetFlags(&editorFlags);
  if (NS_FAILED(rv)) return rv;

  PRBool bIsPlainTextControl = ((editorFlags & eEditorPlaintextMask) != 0);
  if (!bIsPlainTextControl)
  {
    // encode the selection as html with contextual info
    rv = docEncoder->EncodeToStringWithContext(parents, info, buffer);
    if (NS_FAILED(rv)) return rv;
  }
  else
  {
    // encode the selection
    rv = docEncoder->EncodeToString(buffer);
    if (NS_FAILED(rv)) return rv;
  }

  // if we have an empty string, we're done; otherwise continue
  if ( buffer.IsEmpty() )
    return NS_OK;

  nsCOMPtr<nsISupportsString> dataWrapper, contextWrapper, infoWrapper;

  dataWrapper = do_CreateInstance(NS_SUPPORTS_STRING_CONTRACTID, &rv);
  NS_ENSURE_SUCCESS(rv, rv);
  rv = dataWrapper->SetData(buffer);
  if (NS_FAILED(rv)) return rv;

  /* create html flavor transferable */
  nsCOMPtr<nsITransferable> trans = do_CreateInstance("@mozilla.org/widget/transferable;1");
  NS_ENSURE_TRUE(trans, NS_ERROR_FAILURE);

  if (bIsPlainTextControl)
  {
    // Add the unicode flavor to the transferable
    rv = trans->AddDataFlavor(kUnicodeMime);
    if (NS_FAILED(rv)) return rv;

    // QI the data object an |nsISupports| so that when the transferable holds
    // onto it, it will addref the correct interface.
    nsCOMPtr<nsISupports> genericDataObj(do_QueryInterface(dataWrapper));
    rv = trans->SetTransferData(kUnicodeMime, genericDataObj,
                                buffer.Length() * sizeof(PRUnichar));
    if (NS_FAILED(rv)) return rv;
  }
  else
  {
    contextWrapper = do_CreateInstance(NS_SUPPORTS_STRING_CONTRACTID);
    NS_ENSURE_TRUE(contextWrapper, NS_ERROR_FAILURE);
    infoWrapper = do_CreateInstance(NS_SUPPORTS_STRING_CONTRACTID);
    NS_ENSURE_TRUE(infoWrapper, NS_ERROR_FAILURE);

    contextWrapper->SetData(parents);
    infoWrapper->SetData(info);

    rv = trans->AddDataFlavor(kHTMLMime);
    if (NS_FAILED(rv)) return rv;

    nsCOMPtr<nsIFormatConverter> htmlConverter =
             do_CreateInstance("@mozilla.org/widget/htmlformatconverter;1");
    NS_ENSURE_TRUE(htmlConverter, NS_ERROR_FAILURE);

    rv = trans->SetConverter(htmlConverter);
    if (NS_FAILED(rv)) return rv;

    nsCOMPtr<nsISupports> genericDataObj(do_QueryInterface(dataWrapper));
    rv = trans->SetTransferData(kHTMLMime, genericDataObj,
                                buffer.Length() * sizeof(PRUnichar));
    if (NS_FAILED(rv)) return rv;

    if (!parents.IsEmpty())
    {
      // Add the htmlcontext DataFlavor to the transferable
      trans->AddDataFlavor(kHTMLContext);
      genericDataObj = do_QueryInterface(contextWrapper);
      trans->SetTransferData(kHTMLContext, genericDataObj,
                             parents.Length() * sizeof(PRUnichar));
    }
    if (!info.IsEmpty())
    {
      // Add the htmlinfo DataFlavor to the transferable
      trans->AddDataFlavor(kHTMLInfo);
      genericDataObj = do_QueryInterface(infoWrapper);
      trans->SetTransferData(kHTMLInfo, genericDataObj,
                             info.Length() * sizeof(PRUnichar));
    }
  }

  *aTransferable = trans;
  NS_ADDREF(*aTransferable);
  return NS_OK;
}

NS_IMETHODIMP nsHTMLEditor::DoDrag(nsIDOMEvent *aDragEvent)
{
  return nsPlaintextEditor::DoDrag(aDragEvent);
}

PRBool nsHTMLEditor::HavePrivateHTMLFlavor(nsIClipboard *aClipboard)
{
  // check the clipboard for our special kHTMLContext flavor.  If that is there, we know
  // we have our own internal html format on clipboard.

  if (!aClipboard) return PR_FALSE;
  PRBool bHavePrivateHTMLFlavor = PR_FALSE;

  const char* flavArray[] = { kHTMLContext };

  if (NS_SUCCEEDED(aClipboard->HasDataMatchingFlavors(flavArray,
    NS_ARRAY_LENGTH(flavArray), nsIClipboard::kGlobalClipboard,
    &bHavePrivateHTMLFlavor )))
    return bHavePrivateHTMLFlavor;

  return PR_FALSE;
}


NS_IMETHODIMP nsHTMLEditor::Paste(PRInt32 aSelectionType)
{
  ForceCompositionEnd();

  PRBool preventDefault;
  nsresult rv = FireClipboardEvent(NS_PASTE, &preventDefault);
  if (NS_FAILED(rv) || preventDefault)
    return rv;

  // Get Clipboard Service
  nsCOMPtr<nsIClipboard> clipboard(do_GetService("@mozilla.org/widget/clipboard;1", &rv));
  if (NS_FAILED(rv))
    return rv;

  // find out if we have our internal html flavor on the clipboard.  We don't want to mess
  // around with cfhtml if we do.
  PRBool bHavePrivateHTMLFlavor = HavePrivateHTMLFlavor(clipboard);

  if (!bHavePrivateHTMLFlavor)
    return nsHTMLEditor::PasteNoFormatting(aSelectionType);

  // Get the nsITransferable interface for getting the data from the clipboard
  nsCOMPtr<nsITransferable> trans;
  rv = PrepareHTMLTransferable(getter_AddRefs(trans), bHavePrivateHTMLFlavor);
  if (NS_SUCCEEDED(rv) && trans)
  {
    // Get the Data from the clipboard
    if (NS_SUCCEEDED(clipboard->GetData(trans, aSelectionType)) && IsModifiable())
    {
      // also get additional html copy hints, if present
      nsAutoString contextStr, infoStr;

      // also get additional html copy hints, if present
      if (bHavePrivateHTMLFlavor)
      {
        nsCOMPtr<nsISupports> contextDataObj, infoDataObj;
        PRUint32 contextLen, infoLen;
        nsCOMPtr<nsISupportsString> textDataObj;

        nsCOMPtr<nsITransferable> contextTrans =
                      do_CreateInstance("@mozilla.org/widget/transferable;1");
        NS_ENSURE_TRUE(contextTrans, NS_ERROR_NULL_POINTER);
        contextTrans->AddDataFlavor(kHTMLContext);
        clipboard->GetData(contextTrans, aSelectionType);
        contextTrans->GetTransferData(kHTMLContext, getter_AddRefs(contextDataObj), &contextLen);

        nsCOMPtr<nsITransferable> infoTrans =
                      do_CreateInstance("@mozilla.org/widget/transferable;1");
        NS_ENSURE_TRUE(infoTrans, NS_ERROR_NULL_POINTER);
        infoTrans->AddDataFlavor(kHTMLInfo);
        clipboard->GetData(infoTrans, aSelectionType);
        infoTrans->GetTransferData(kHTMLInfo, getter_AddRefs(infoDataObj), &infoLen);

        if (contextDataObj)
        {
          nsAutoString text;
          textDataObj = do_QueryInterface(contextDataObj);
          textDataObj->GetData(text);
          NS_ASSERTION(text.Length() <= (contextLen/2), "Invalid length!");
          contextStr.Assign(text.get(), contextLen / 2);
        }

        if (infoDataObj)
        {
          nsAutoString text;
          textDataObj = do_QueryInterface(infoDataObj);
          textDataObj->GetData(text);
          NS_ASSERTION(text.Length() <= (infoLen/2), "Invalid length!");
          infoStr.Assign(text.get(), infoLen / 2);
        }
      }

      // handle transferable hooks
      nsCOMPtr<nsIDOMDocument> domdoc;
      GetDocument(getter_AddRefs(domdoc));
      if (!nsEditorHookUtils::DoInsertionHook(domdoc, nsnull, trans))
        return NS_OK;

      // Beware! This may flush notifications via synchronous
      // ScrollSelectionIntoView.
      rv = InsertFromTransferable(trans, nsnull, contextStr, infoStr,
                                  nsnull, 0, PR_TRUE);
    }
  }

  return rv;
}

//
// HTML PasteNoFormatting. Ignore any HTML styles and formating in paste source
//
NS_IMETHODIMP nsHTMLEditor::PasteNoFormatting(PRInt32 aSelectionType)
{
  ForceCompositionEnd();

  // Get Clipboard Service
  nsresult rv;
  nsCOMPtr<nsIClipboard> clipboard(do_GetService("@mozilla.org/widget/clipboard;1", &rv));
  if (NS_FAILED(rv))
    return rv;

  // Get the nsITransferable interface for getting the data from the clipboard.
  // use nsPlaintextEditor::PrepareTransferable() to force unicode plaintext data.
  nsCOMPtr<nsITransferable> trans;
  rv = nsPlaintextEditor::PrepareTransferable(getter_AddRefs(trans));
  if (NS_SUCCEEDED(rv) && trans)
  {
    // Get the Data from the clipboard
    if (NS_SUCCEEDED(clipboard->GetData(trans, aSelectionType)) && IsModifiable())
    {
      const nsAFlatString& empty = EmptyString();
      // Beware! This may flush notifications via synchronous
      // ScrollSelectionIntoView.
      rv = InsertFromTransferable(trans, nsnull, empty, empty, nsnull, 0,
                                  PR_TRUE);
    }
  }

  return rv;
}


NS_IMETHODIMP nsHTMLEditor::CanPaste(PRInt32 aSelectionType, PRBool *aCanPaste)
{
  NS_ENSURE_ARG_POINTER(aCanPaste);
  *aCanPaste = PR_FALSE;

  // can't paste if readonly
  if (!IsModifiable())
    return NS_OK;

  nsresult rv;
  nsCOMPtr<nsIClipboard> clipboard(do_GetService("@mozilla.org/widget/clipboard;1", &rv));
  if (NS_FAILED(rv)) return rv;

  // the flavors that we can deal with
  const char* textEditorFlavors[] = { kUnicodeMime };
  const char* textHtmlEditorFlavors[] = { kUnicodeMime, // BBM. Import HTML as unicode text unless it is Mozilla's internal stuff -- kHTMLMime,
                                          kJPEGImageMime };

  PRUint32 editorFlags;
  GetFlags(&editorFlags);

  PRBool haveFlavors;

  // Use the flavors depending on the current editor mask
  if ((editorFlags & eEditorPlaintextMask))
    rv = clipboard->HasDataMatchingFlavors(textEditorFlavors,
                                           NS_ARRAY_LENGTH(textEditorFlavors),
                                           aSelectionType, &haveFlavors);
  else
    rv = clipboard->HasDataMatchingFlavors(textHtmlEditorFlavors,
                                           NS_ARRAY_LENGTH(textHtmlEditorFlavors),
                                           aSelectionType, &haveFlavors);

  if (NS_FAILED(rv)) return rv;

  *aCanPaste = haveFlavors;
  return NS_OK;
}


//
// HTML PasteAsQuotation: Paste in a blockquote type=cite
//
NS_IMETHODIMP nsHTMLEditor::PasteAsQuotation(PRInt32 aSelectionType)
{
  if (mFlags & eEditorPlaintextMask)
    return PasteAsPlaintextQuotation(aSelectionType);

  nsAutoString citation;
  return PasteAsCitedQuotation(citation, aSelectionType);
}

NS_IMETHODIMP nsHTMLEditor::PasteAsCitedQuotation(const nsAString & aCitation,
                                                  PRInt32 aSelectionType)
{
  nsAutoEditBatch beginBatching(this);
  nsAutoRules beginRulesSniffing(this, kOpInsertQuotation, nsIEditor::eNext);

  // get selection
  nsCOMPtr<nsISelection> selection;
  nsresult res = GetSelection(getter_AddRefs(selection));
  if (NS_FAILED(res)) return res;
  if (!selection) return NS_ERROR_NULL_POINTER;

  // give rules a chance to handle or cancel
  nsTextRulesInfo ruleInfo(nsTextEditRules::kInsertElement);
  PRBool cancel, handled;
  res = mRules->WillDoAction(selection, &ruleInfo, &cancel, &handled);
  if (NS_FAILED(res)) return res;
  if (cancel) return NS_OK; // rules canceled the operation
  if (!handled)
  {
    nsCOMPtr<nsIDOMNode> newNode;
    res = DeleteSelectionAndCreateNode(NS_LITERAL_STRING("blockquote"), getter_AddRefs(newNode));
    if (NS_FAILED(res)) return res;
    if (!newNode) return NS_ERROR_NULL_POINTER;

    // Try to set type=cite.  Ignore it if this fails.
    nsCOMPtr<nsIDOMElement> newElement (do_QueryInterface(newNode));
    if (newElement)
    {
      newElement->SetAttribute(NS_LITERAL_STRING("type"), NS_LITERAL_STRING("cite"));
    }

    // Set the selection to the underneath the node we just inserted:
    res = selection->Collapse(newNode, 0);
    if (NS_FAILED(res))
    {
#ifdef DEBUG_akkana
      printf("Couldn't collapse");
#endif
      // XXX: error result:  should res be returned here?
    }

    res = Paste(aSelectionType);
  }
  return res;
}

//
// Paste a plaintext quotation
//
NS_IMETHODIMP nsHTMLEditor::PasteAsPlaintextQuotation(PRInt32 aSelectionType)
{
  // Get Clipboard Service
  nsresult rv;
  nsCOMPtr<nsIClipboard> clipboard(do_GetService("@mozilla.org/widget/clipboard;1", &rv));
  if (NS_FAILED(rv)) return rv;

  // Create generic Transferable for getting the data
  nsCOMPtr<nsITransferable> trans =
                 do_CreateInstance("@mozilla.org/widget/transferable;1", &rv);
  if (NS_SUCCEEDED(rv) && trans)
  {
    // We only handle plaintext pastes here
    trans->AddDataFlavor(kUnicodeMime);

    // Get the Data from the clipboard
    clipboard->GetData(trans, aSelectionType);

    // Now we ask the transferable for the data
    // it still owns the data, we just have a pointer to it.
    // If it can't support a "text" output of the data the call will fail
    nsCOMPtr<nsISupports> genericDataObj;
    PRUint32 len = 0;
    char* flav = 0;
    rv = trans->GetAnyTransferData(&flav, getter_AddRefs(genericDataObj),
                                   &len);
    if (NS_FAILED(rv))
    {
#ifdef DEBUG_akkana
      printf("PasteAsPlaintextQuotation: GetAnyTransferData failed, %d\n", rv);
#endif
      return rv;
    }

    if (flav && 0 == nsCRT::strcmp((flav), kUnicodeMime))
    {
#ifdef DEBUG_clipboard
    printf("Got flavor [%s]\n", flav);
#endif
      nsCOMPtr<nsISupportsString> textDataObj(do_QueryInterface(genericDataObj));
      if (textDataObj && len > 0)
      {
        nsAutoString stuffToPaste;
        textDataObj->GetData(stuffToPaste);
        NS_ASSERTION(stuffToPaste.Length() <= (len/2), "Invalid length!");
        nsAutoEditBatch beginBatching(this);
        rv = InsertAsPlaintextQuotation(stuffToPaste, PR_TRUE, 0);
      }
    }
    NS_Free(flav);
  }

  return rv;
}

NS_IMETHODIMP
nsHTMLEditor::InsertTextWithQuotations(const nsAString &aStringToInsert)
{
  if (mWrapToWindow)
    return InsertText(aStringToInsert);

  // The whole operation should be undoable in one transaction:
  BeginTransaction();

  // We're going to loop over the string, collecting up a "hunk"
  // that's all the same type (quoted or not),
  // Whenever the quotedness changes (or we reach the string's end)
  // we will insert the hunk all at once, quoted or non.

  static const PRUnichar cite('>');
  PRBool curHunkIsQuoted = (aStringToInsert.First() == cite);

  nsAString::const_iterator hunkStart, strEnd;
  aStringToInsert.BeginReading(hunkStart);
  aStringToInsert.EndReading(strEnd);

  // In the loop below, we only look for DOM newlines (\n),
  // because we don't have a FindChars method that can look
  // for both \r and \n.  \r is illegal in the dom anyway,
  // but in debug builds, let's take the time to verify that
  // there aren't any there:
#ifdef DEBUG
  nsAString::const_iterator dbgStart (hunkStart);
  if (FindCharInReadable('\r', dbgStart, strEnd))
    NS_ASSERTION(PR_FALSE,
            "Return characters in DOM! InsertTextWithQuotations may be wrong");
#endif /* DEBUG */

  // Loop over lines:
  nsresult rv = NS_OK;
  nsAString::const_iterator lineStart (hunkStart);
  while (1)   // we will break from inside when we run out of newlines
  {
    // Search for the end of this line (dom newlines, see above):
    PRBool found = FindCharInReadable('\n', lineStart, strEnd);
    PRBool quoted = PR_FALSE;
    if (found)
    {
      // if there's another newline, lineStart now points there.
      // Loop over any consecutive newline chars:
      nsAString::const_iterator firstNewline (lineStart);
      while (*lineStart == '\n')
        ++lineStart;
      quoted = (*lineStart == cite);
      if (quoted == curHunkIsQuoted)
        continue;
      // else we're changing state, so we need to insert
      // from curHunk to lineStart then loop around.

      // But if the current hunk is quoted, then we want to make sure
      // that any extra newlines on the end do not get included in
      // the quoted section: blank lines flaking a quoted section
      // should be considered unquoted, so that if the user clicks
      // there and starts typing, the new text will be outside of
      // the quoted block.
      if (curHunkIsQuoted)
        lineStart = firstNewline;
    }

    // If no newline found, lineStart is now strEnd and we can finish up,
    // inserting from curHunk to lineStart then returning.
    const nsAString &curHunk = Substring(hunkStart, lineStart);
    nsCOMPtr<nsIDOMNode> dummyNode;
#ifdef DEBUG_akkana_verbose
    printf("==== Inserting text as %squoted: ---\n%s---\n",
           curHunkIsQuoted ? "" : "non-",
           NS_LossyConvertUTF16toASCII(curHunk).get());
#endif
    if (curHunkIsQuoted)
      rv = InsertAsPlaintextQuotation(curHunk, PR_FALSE,
                                      getter_AddRefs(dummyNode));
    else
      rv = InsertText(curHunk);

    if (!found)
      break;

    curHunkIsQuoted = quoted;
    hunkStart = lineStart;
  }

  EndTransaction();

  return rv;
}

NS_IMETHODIMP nsHTMLEditor::InsertAsQuotation(const nsAString & aQuotedText,
                                              nsIDOMNode **aNodeInserted)
{
  if (mFlags & eEditorPlaintextMask)
    return InsertAsPlaintextQuotation(aQuotedText, PR_TRUE, aNodeInserted);

  nsAutoString citation;
  return InsertAsCitedQuotation(aQuotedText, citation, PR_FALSE,
                                aNodeInserted);
}

// Insert plaintext as a quotation, with cite marks (e.g. "> ").
// This differs from its corresponding method in nsPlaintextEditor
// in that here, quoted material is enclosed in a <pre> tag
// in order to preserve the original line wrapping.
NS_IMETHODIMP
nsHTMLEditor::InsertAsPlaintextQuotation(const nsAString & aQuotedText,
                                         PRBool aAddCites,
                                         nsIDOMNode **aNodeInserted)
{
  if (mWrapToWindow)
    return nsPlaintextEditor::InsertAsQuotation(aQuotedText, aNodeInserted);

  nsresult rv;

  // The quotesPreformatted pref is a temporary measure. See bug 69638.
  // Eventually we'll pick one way or the other.
  PRBool quotesInPre = PR_FALSE;
  nsCOMPtr<nsIPrefBranch> prefBranch =
    do_GetService(NS_PREFSERVICE_CONTRACTID, &rv);
  if (NS_SUCCEEDED(rv) && prefBranch)
    prefBranch->GetBoolPref("editor.quotesPreformatted", &quotesInPre);

  nsCOMPtr<nsIDOMNode> preNode;
  // get selection
  nsCOMPtr<nsISelection> selection;
  rv = GetSelection(getter_AddRefs(selection));
  if (NS_FAILED(rv)) return rv;
  if (!selection) return NS_ERROR_NULL_POINTER;
  else
  {
    nsAutoEditBatch beginBatching(this);
    nsAutoRules beginRulesSniffing(this, kOpInsertQuotation, nsIEditor::eNext);

    // give rules a chance to handle or cancel
    nsTextRulesInfo ruleInfo(nsTextEditRules::kInsertElement);
    PRBool cancel, handled;
    rv = mRules->WillDoAction(selection, &ruleInfo, &cancel, &handled);
    if (NS_FAILED(rv)) return rv;
    if (cancel) return NS_OK; // rules canceled the operation
    if (!handled)
    {
      // Wrap the inserted quote in a <pre> so it won't be wrapped:
      nsAutoString tag;
      if (quotesInPre)
        tag.AssignLiteral("pre");
      else
        tag.AssignLiteral("span");

      rv = DeleteSelectionAndCreateNode(tag, getter_AddRefs(preNode));

      // If this succeeded, then set selection inside the pre
      // so the inserted text will end up there.
      // If it failed, we don't care what the return value was,
      // but we'll fall through and try to insert the text anyway.
      if (NS_SUCCEEDED(rv) && preNode)
      {
        // Add an attribute on the pre node so we'll know it's a quotation.
        // Do this after the insertion, so that
        nsCOMPtr<nsIDOMElement> preElement (do_QueryInterface(preNode));
        if (preElement)
        {
          preElement->SetAttribute(NS_LITERAL_STRING("_moz_quote"),
                                   NS_LITERAL_STRING("true"));
          if (quotesInPre)
          {
            // set style to not have unwanted vertical margins
            preElement->SetAttribute(NS_LITERAL_STRING("style"),
                                     NS_LITERAL_STRING("margin: 0 0 0 0px;"));
          }
          else
          {
            // turn off wrapping on spans
            preElement->SetAttribute(NS_LITERAL_STRING("style"),
                                     NS_LITERAL_STRING("white-space: pre;"));
          }
        }

        // and set the selection inside it:
        selection->Collapse(preNode, 0);
      }

      if (aAddCites)
        rv = nsPlaintextEditor::InsertAsQuotation(aQuotedText, aNodeInserted);
      else
        rv = nsPlaintextEditor::InsertText(aQuotedText);
      // Note that if !aAddCites, aNodeInserted isn't set.
      // That's okay because the routines that use aAddCites
      // don't need to know the inserted node.

      if (aNodeInserted && NS_SUCCEEDED(rv))
      {
        *aNodeInserted = preNode;
        NS_IF_ADDREF(*aNodeInserted);
      }
    }
  }

  // Set the selection to just after the inserted node:
  if (NS_SUCCEEDED(rv) && preNode)
  {
    nsCOMPtr<nsIDOMNode> parent;
    PRInt32 offset;
    if (NS_SUCCEEDED(GetNodeLocation(preNode, address_of(parent), &offset)) && parent)
      selection->Collapse(parent, offset+1);
  }
  return rv;
}

NS_IMETHODIMP
nsHTMLEditor::StripCites()
{
  return nsPlaintextEditor::StripCites();
}

NS_IMETHODIMP
nsHTMLEditor::Rewrap(PRBool aRespectNewlines)
{
  return nsPlaintextEditor::Rewrap(aRespectNewlines);
}

NS_IMETHODIMP
nsHTMLEditor::InsertAsCitedQuotation(const nsAString & aQuotedText,
                                     const nsAString & aCitation,
                                     PRBool aInsertHTML,
                                     nsIDOMNode **aNodeInserted)
{
  // Don't let anyone insert html into a "plaintext" editor:
  if (mFlags & eEditorPlaintextMask)
  {
    NS_ASSERTION(!aInsertHTML, "InsertAsCitedQuotation: trying to insert html into plaintext editor");
    return InsertAsPlaintextQuotation(aQuotedText, PR_TRUE, aNodeInserted);
  }

  nsCOMPtr<nsIDOMNode> newNode;
  nsresult res = NS_OK;

  // get selection
  nsCOMPtr<nsISelection> selection;
  res = GetSelection(getter_AddRefs(selection));
  if (NS_FAILED(res)) return res;
  if (!selection) return NS_ERROR_NULL_POINTER;
  else
  {
    nsAutoEditBatch beginBatching(this);
    nsAutoRules beginRulesSniffing(this, kOpInsertQuotation, nsIEditor::eNext);

    // give rules a chance to handle or cancel
    nsTextRulesInfo ruleInfo(nsTextEditRules::kInsertElement);
    PRBool cancel, handled;
    res = mRules->WillDoAction(selection, &ruleInfo, &cancel, &handled);
    if (NS_FAILED(res)) return res;
    if (cancel) return NS_OK; // rules canceled the operation
    if (!handled)
    {
      res = DeleteSelectionAndCreateNode(NS_LITERAL_STRING("blockquote"), getter_AddRefs(newNode));
      if (NS_FAILED(res)) return res;
      if (!newNode) return NS_ERROR_NULL_POINTER;

      // Try to set type=cite.  Ignore it if this fails.
      nsCOMPtr<nsIDOMElement> newElement (do_QueryInterface(newNode));
      if (newElement)
      {
        NS_NAMED_LITERAL_STRING(citestr, "cite");
        newElement->SetAttribute(NS_LITERAL_STRING("type"), citestr);

        if (!aCitation.IsEmpty())
          newElement->SetAttribute(citestr, aCitation);

        // Set the selection inside the blockquote so aQuotedText will go there:
        selection->Collapse(newNode, 0);
      }

      if (aInsertHTML)
        res = LoadHTML(aQuotedText);

      else
        res = InsertText(aQuotedText);  // XXX ignore charset

      if (aNodeInserted)
      {
        if (NS_SUCCEEDED(res))
        {
          *aNodeInserted = newNode;
          NS_IF_ADDREF(*aNodeInserted);
        }
      }
    }
  }

  // Set the selection to just after the inserted node:
  if (NS_SUCCEEDED(res) && newNode)
  {
    nsCOMPtr<nsIDOMNode> parent;
    PRInt32 offset;
    if (NS_SUCCEEDED(GetNodeLocation(newNode, address_of(parent), &offset)) && parent)
      selection->Collapse(parent, offset+1);
  }
  return res;
}

NS_IMETHODIMP nsHTMLEditor::CopySelectionAsImage(PRBool* _retval)
{
  nsresult rv = NS_ERROR_FAILURE;
  nsCOMPtr<nsIPresShell> presShell;
  GetPresShell(getter_AddRefs(presShell));
  nsCOMPtr<nsISelection> selection;
  rv = GetSelection(getter_AddRefs(selection));
//  nsCOMPtr<msiISelection> msiSelection;
//  GetMSISelection(msiSelection);
  NS_ENSURE_SUCCESS(rv, rv);
  if (!selection)
    return NS_ERROR_FAILURE;

  nsCOMPtr<nsITransferable> trans(do_CreateInstance("@mozilla.org/widget/transferable;1", &rv));
  NS_ENSURE_SUCCESS(rv, rv);

//  //first put SVG on clipboard if available
  nsCOMPtr<nsIStorageStream> svgStorage;
  nsresult res = NS_NewStorageStream(4096, PR_UINT32_MAX, getter_AddRefs(svgStorage));
  nsCOMPtr<nsIOutputStream> svgOut;
  if (svgStorage)
    svgStorage->GetOutputStream(0, getter_AddRefs(svgOut));
  if (svgOut)
  {
    NS_NAMED_LITERAL_STRING(svgExt, "svg");
    res = presShell->DrawSelectionToFile(selection, EmptyString(), svgExt, svgOut, _retval);
    if (NS_SUCCEEDED(res))
    {
      nsCAutoString outString;
      nsCOMPtr<nsIInputStream> svgRead;
      res = svgStorage->NewInputStream(0, getter_AddRefs(svgRead));
      PRUint32 available, bytesRead;
      svgRead->Available(&available);
      char* inString = new char[available + 1];
      PRInt32 svgStart = 0;
      res = svgRead->Read(inString, available, &bytesRead);
      inString[available] = 0;  //hand-null-terminate necessary??
      nsDependentCString svgFileStr(inString);
      NS_NAMED_LITERAL_CSTRING(svgEltStart, "<svg");
      svgStart = svgFileStr.Find(svgEltStart,PR_TRUE);
      if (svgStart < 0)
      {
        PRInt32 eltStart = svgFileStr.Find("<", PR_FALSE);
        PRInt32 eltEnd = 0;
        nsCString nodeCnts;
        while (svgStart < 0)
        {
          eltStart = svgFileStr.Find("<", eltEnd, PR_FALSE);
          if (eltStart >= 0)
          {
            eltEnd = svgFileStr.Find(">", eltStart, PR_FALSE);
            if (eltStart < eltEnd)
            {
              nodeCnts = Substring(svgFileStr, eltStart + 1, eltEnd - eltStart - 1);
              nodeCnts.CompressWhitespace();
              if (StringBeginsWith(nodeCnts, NS_LITERAL_CSTRING("svg")))
                svgStart = eltStart;
            }
            if (eltEnd < 0)
              break;
          }
          else
            break;
        }
      }
      if (svgStart < 0)
        svgStart = 0;
      outString = Substring(svgFileStr, svgStart);

      nsCOMPtr<nsISupportsString> svgWrapper;
      svgWrapper = do_CreateInstance(NS_SUPPORTS_STRING_CONTRACTID, &res);
      NS_ENSURE_SUCCESS(res, res);
      res = svgWrapper->SetData(NS_ConvertUTF8toUTF16(outString));

      // copy the svg data onto the transferable
      rv = trans->AddDataFlavor(kHTMLMime);
      nsCOMPtr<nsISupports> genericDataObj(do_QueryInterface(svgWrapper));
      rv = trans->SetTransferData(kHTMLMime, genericDataObj, outString.Length() * sizeof(PRUnichar));
      NS_ENSURE_SUCCESS(rv, rv);
    }
  }

#ifdef XP_WIN
  //Put an enhanced metafile image on clipboard
  void* enhMetafile;
  //enhMetafile is actually a Windows handle (HENHMETAFILE), so it doesn't understand add_ref. Just pass the pointer to fill in.
  res = presShell->RenderSelectionToNativeMetafile(selection, &enhMetafile);
  if (NS_SUCCEEDED(res))
  {
    //Wrap it in nsISupportsVoid
    nsCOMPtr<nsISupportsVoid>
      emfPtr(do_CreateInstance(NS_SUPPORTS_VOID_CONTRACTID, &rv));
    NS_ENSURE_SUCCESS(rv, rv);
    rv = emfPtr->SetData(enhMetafile);
    NS_ENSURE_SUCCESS(rv, rv);
    //Then wrap it in nsISupportsInterfacePointer
    nsCOMPtr<nsISupportsInterfacePointer>
      dataPtr(do_CreateInstance(NS_SUPPORTS_INTERFACE_POINTER_CONTRACTID, &rv));
    NS_ENSURE_SUCCESS(rv, rv);
    rv = dataPtr->SetData(emfPtr);
    NS_ENSURE_SUCCESS(rv, rv);

    // copy the image data onto the transferable
    rv = trans->SetTransferData(kWin32EnhMetafile, dataPtr,
                                sizeof(nsISupports*));
    NS_ENSURE_SUCCESS(rv, rv);
  }
#endif

  //then put a JPEG image (actually, this puts a native bitmap image)
  nsCOMPtr<nsIImage> imageObj;
  res = presShell->RenderSelectionToImage(selection, getter_AddRefs(imageObj));
  if (imageObj)
  {
    //Wrap it in nsISupportsInterfacePointer
    nsCOMPtr<nsISupportsInterfacePointer>
      imgPtr(do_CreateInstance(NS_SUPPORTS_INTERFACE_POINTER_CONTRACTID, &rv));
    NS_ENSURE_SUCCESS(rv, rv);
    rv = imgPtr->SetData(imageObj);
    NS_ENSURE_SUCCESS(rv, rv);

    // copy the image data onto the transferable
    rv = trans->SetTransferData(kNativeImageMime, imgPtr,
                                sizeof(nsISupports*));
    NS_ENSURE_SUCCESS(rv, rv);
  }

  // get clipboard
  nsCOMPtr<nsIClipboard> clipboard(do_GetService("@mozilla.org/widget/clipboard;1", &rv));
  NS_ENSURE_SUCCESS(rv, rv);

  // check whether the system supports the selection clipboard or not.
  PRBool selectionSupported;
  rv = clipboard->SupportsSelectionClipboard(&selectionSupported);
  NS_ENSURE_SUCCESS(rv, rv);

  // put the transferable on the clipboard
  if (selectionSupported) {
    rv = clipboard->SetData(trans, nsnull, nsIClipboard::kSelectionClipboard);
    NS_ENSURE_SUCCESS(rv, rv);
  }

  return clipboard->SetData(trans, nsnull, nsIClipboard::kGlobalClipboard);
}

NS_IMETHODIMP nsHTMLEditor::SaveSelectionAsImage(const nsAString& filepath, PRBool* _retval)
{
  nsCOMPtr<nsIPresShell> presShell;
  GetPresShell(getter_AddRefs(presShell));
  nsCOMPtr<nsISelection> selection;
  nsresult res = GetSelection(getter_AddRefs(selection));
//  nsCOMPtr<msiISelection> msiSelection;
//  GetMSISelection(msiSelection);
  if (!selection)
    return NS_ERROR_FAILURE;

  nsString extension(NS_LITERAL_STRING("png"));
  nsAutoString theFile(filepath);
  PRInt32 lastdot = theFile.RFindChar('.', -1, -1);
  if (lastdot != kNotFound)
    theFile.Right(extension, theFile.Length() - lastdot - 1);

  nsCOMPtr<nsILocalFile> file = do_CreateInstance("@mozilla.org/file/local;1");
  NS_ENSURE_TRUE(file, NS_ERROR_FAILURE);
  nsresult rv = file->InitWithPath(filepath);
  NS_ENSURE_SUCCESS(rv, rv);

  nsCOMPtr<nsIOutputStream> outputStream;
  if (!extension.EqualsLiteral("emf") && !extension.EqualsLiteral("wmf"))  //want to pass a null outputstream for this one; operating system does the work
  {
    rv = NS_NewLocalFileOutputStream(getter_AddRefs(outputStream), file);
    NS_ENSURE_SUCCESS(rv, rv);
  }

  return presShell->DrawSelectionToFile(selection, filepath, extension, outputStream, _retval);
}

void RemoveContextNodes(nsAutoTArray<nsAutoString, 32> &tagStack, nsIDOMNode * fragNode)
{
  if (!fragNode)
    return;
 // printf("Entering RemoveContextNodes\n");
  DumpNode(fragNode);
  nsCOMPtr<nsIDOMNode> tmp, next, child, lastchild, base;
  PRInt32 L = tagStack.Length();
  PRInt32 i = L-1;
  nsAutoString tagName;
  nsAutoString tagName2;

  fragNode->GetFirstChild(getter_AddRefs(child));
  base = child;
  while ((i>=0) && child) {
    nsEditor::GetTagString(child, tagName);
    tagName2 = tagStack[i];
    if (!(tagName.Equals(tagName2))) return;
    i--;
    lastchild = child;
    lastchild->GetFirstChild(getter_AddRefs(child));
  }
  if (i < 0)
  {
    lastchild->GetFirstChild(getter_AddRefs(child));
    while (child)
    {
      child->GetNextSibling(getter_AddRefs(next));
      fragNode->InsertBefore(child, base, getter_AddRefs(tmp));
      child = next;
    }
    fragNode->RemoveChild(base, getter_AddRefs(tmp));
  }
//  printf("============\n");
  DumpNode(fragNode);
}



void RemoveBodyAndHead(nsIDOMNode *aNode)
{
  if (!aNode)
    return;

  nsCOMPtr<nsIDOMNode> tmp, child, body, head, html;
  // find the body and head nodes if any.
  // look only at immediate children of aNode.
  aNode->GetFirstChild(getter_AddRefs(child));
  if (nsEditor::NodeIsType(child, nsEditProperty::html))
  {
    html = child;
    html->GetFirstChild(getter_AddRefs(child));
    while (child)
    {
      aNode->InsertBefore(child, head, getter_AddRefs(tmp));
      html->GetFirstChild(getter_AddRefs(child));
    }
    aNode->RemoveChild(html, getter_AddRefs(tmp));
    aNode->GetFirstChild(getter_AddRefs(child));
  }

  while (child)
  {
    if (nsTextEditUtils::IsBody(child))
    {
      body = child;
    }
    else if (nsEditor::NodeIsType(child, nsEditProperty::head))
    {
      head = child;
    }
    child->GetNextSibling(getter_AddRefs(tmp));
    child = tmp;
  }
  if (head)
  {
    aNode->RemoveChild(head, getter_AddRefs(tmp));
  }
  if (body)
  {
    body->GetFirstChild(getter_AddRefs(child));
    while (child)
    {
      aNode->InsertBefore(child, body, getter_AddRefs(tmp));
      body->GetFirstChild(getter_AddRefs(child));
    }
    aNode->RemoveChild(body, getter_AddRefs(tmp));
  }
}

/**
 * This function finds the target node that we will be pasting into. aStart is
 * the context that we're given and aResult will be the target. Initially,
 * *aResult must be NULL.
 *
 * The target for a paste is found by either finding the node that contains
 * the magical comment node containing kInsertCookie or, failing that, the
 * firstChild of the firstChild (until we reach a leaf).
 */
nsresult FindTargetNode(nsIDOMNode *aStart, nsCOMPtr<nsIDOMNode> &aResult)
{
  if (!aStart)
    return NS_OK;

  nsCOMPtr<nsIDOMNode> child, tmp;

  nsresult rv = aStart->GetFirstChild(getter_AddRefs(child));
  NS_ENSURE_SUCCESS(rv, rv);

  if (!child)
  {
    // If the current result is NULL, then aStart is a leaf, and is the
    // fallback result.
    if (!aResult)
      aResult = aStart;

    return NS_OK;
  }

  do
  {
    // Is this child the magical cookie?
    nsCOMPtr<nsIDOMComment> comment = do_QueryInterface(child);
    if (comment)
    {
      nsAutoString data;
      rv = comment->GetData(data);
      NS_ENSURE_SUCCESS(rv, rv);

      if (data.EqualsLiteral(kInsertCookie))
      {
        // Yes it is! Return an error so we bubble out and short-circuit the
        // search.
        aResult = aStart;

        // Note: it doesn't matter if this fails.
        aStart->RemoveChild(child, getter_AddRefs(tmp));

        return NS_FOUND_TARGET;
      }
    }

    // Note: Don't use NS_ENSURE_* here since we return a failure result to
    // inicate that we found the magical cookie and we don't want to spam the
    // console.
    rv = FindTargetNode(child, aResult);
    if (NS_FAILED(rv))
      return rv;

    rv = child->GetNextSibling(getter_AddRefs(tmp));
    NS_ENSURE_SUCCESS(rv, rv);

    child = tmp;
  } while (child);

  return NS_OK;
}


void nsHTMLEditor::RemoveNode( nsIDOMNode * aNode)
{
  // move all children to be siblings, and then delete the node.
  nsCOMPtr<nsIDOMNodeList> pNodeList;
  nsCOMPtr<nsIDOMNode> pChild;
  nsCOMPtr<nsIDOMNode> pChildClone;
  nsCOMPtr<nsIDOMNode> pParent,ignored;
  PRUint32 offset = 0;
  PRUint32 length;
  PRInt32 i;
  nsresult rv;
  rv = aNode->GetParentNode(getter_AddRefs(pParent));
  rv = pParent->GetChildNodes(getter_AddRefs(pNodeList));
  while (true)
  {
    rv = pNodeList->Item(offset, getter_AddRefs(pChild));
    if (pChild != aNode) offset++;
    else break;
  }
  rv = aNode->GetChildNodes(getter_AddRefs(pNodeList));
  rv = pNodeList->GetLength(&length);
  for (i = length-1; i >= 0; i--)
  {
    rv = pNodeList->Item(i, getter_AddRefs(pChild));
    pChild->CloneNode(PR_TRUE, getter_AddRefs(pChildClone));
    InsertNode(pChildClone, pParent, offset + 1);
  }
  rv = pParent->RemoveChild(aNode, getter_AddRefs(ignored));
}

void nsHTMLEditor::MathParent( nsIDOMNode * aNode, nsIDOMNode **aMathNode)
{  // aNode is a math node (in the MathML namespace). Return aMathNode which is the parent mml:math element
  nsCOMPtr<nsIDOMNode> parent, tempNode;
  nsCOMPtr<nsIDOMElement> element;
  nsAutoString tagName;
  nsresult rv;
  *aMathNode = nsnull;
  rv = aNode->GetParentNode(getter_AddRefs(parent));
  while (parent)
  {
    element = do_QueryInterface(parent);
    if (element)
    {
      element->GetTagName(tagName);
      if (tagName.EqualsLiteral("math"))
      {
        *aMathNode = parent;
        return;
      }
    }
    rv = parent->GetParentNode(getter_AddRefs(tempNode));
    parent = tempNode;
  }
  return;
}

void nsHTMLEditor::FixMathematics( nsIDOMNode * nodelistNode, PRBool fLeftOnly, PRBool fRightOnly)
{
  // When math objects are only partly copied, we may get anomalies like a fraction with
  // only one subnode instead of two. In those cases, we strip out the higher structure (such
  // as the fraction). Since this happens only with partial copies, we need to check only the
  // end nodes of the fragment, and only the first nodes on the left side and the last nodes
  // on the right side
  if (!nodelistNode) return;
  nsAutoString tagName;
  nsAutoString tagNamespace;
//  nsAutoString nsprefix;
  PRUint32 length;
  PRBool isMathNode;
  nsString strMathMLNs = NS_LITERAL_STRING("http://www.w3.org/1998/Math/MathML");
  nsCOMPtr<nsIDOMElement> element = do_QueryInterface(nodelistNode);
  nsCOMPtr<nsIDOMNodeList> pNodeList;
  nsresult res;
  nsCOMPtr<nsIDOMNode> leftEnd;
  nsCOMPtr<nsIDOMNode> rightEnd;
  res = nodelistNode->GetFirstChild(getter_AddRefs(leftEnd));
  res = nodelistNode->GetLastChild(getter_AddRefs(rightEnd));
  if (element)
  {
    element->GetTagName(tagName);
    element->GetNamespaceURI(tagNamespace);
    isMathNode = tagNamespace.Equals(strMathMLNs);
//    element->GetPrefix(nsprefix);
    if (isMathNode && (tagName.EqualsLiteral("mfrac")||tagName.EqualsLiteral("msup")||
      tagName.EqualsLiteral("msub")  || tagName.EqualsLiteral("mroot")||
      /*tagName.EqualsLiteral("msqrt") ||*/ tagName.EqualsLiteral("msubsup")))
    {
      // count the children of element. If there are too few, delete element
      nodelistNode->GetChildNodes(getter_AddRefs(pNodeList));
      pNodeList->GetLength(&length);
      if ((length < 2)
          ||((length==2)&&(tagName.EqualsLiteral("msubsup"))))
        RemoveNode(nodelistNode);

    }
  }
  if (leftEnd && !fRightOnly) FixMathematics(leftEnd, PR_TRUE, PR_FALSE);
  if (rightEnd && (rightEnd != leftEnd) && !fLeftOnly) FixMathematics(rightEnd, PR_FALSE, PR_TRUE);
}



nsresult nsHTMLEditor::CreateDOMFragmentFromPaste(const nsAString &aInputString,
                                                  const nsAString & aContextStr,
                                                  const nsAString & aInfoStr,
                                                  nsCOMPtr<nsIDOMNode> *outFragNode,
                                                  nsCOMPtr<nsIDOMNode> *outStartNode,
                                                  nsCOMPtr<nsIDOMNode> *outEndNode,
                                                  PRInt32 *outStartOffset,
                                                  PRInt32 *outEndOffset)
{
  if (!outFragNode || !outStartNode || !outEndNode)
    return NS_ERROR_NULL_POINTER;
  nsCOMPtr<nsIDOMDocumentFragment> docfrag;
  nsCOMPtr<nsIDOMNode> contextAsNode, tmp;
  nsresult res = NS_OK;

  nsCOMPtr<nsIDOMDocument> domDoc;
  GetDocument(getter_AddRefs(domDoc));

  nsCOMPtr<nsIDocument> doc = do_QueryInterface(domDoc);
  NS_ENSURE_TRUE(doc, NS_ERROR_FAILURE);

  // if we have context info, create a fragment for that
  nsAutoTArray<nsAutoString, 32> tagStack;
  nsCOMPtr<nsIDOMDocumentFragment> contextfrag;
  nsCOMPtr<nsIDOMNode> contextLeaf, junk;
  if (!aContextStr.IsEmpty())
  {
    res = ParseFragment(aContextStr, tagStack, doc, address_of(contextAsNode));
#if DEBUG_barry || DEBUG_Barry
    // dumpTagStack(tagStack);
    DumpNode(contextAsNode);
#endif
    NS_ENSURE_SUCCESS(res, res);
    NS_ENSURE_TRUE(contextAsNode, NS_ERROR_FAILURE);

    res = StripFormattingNodes(contextAsNode);
    NS_ENSURE_SUCCESS(res, res);
#if DEBUG_barry || DEBUG_Barry
    DumpNode(contextAsNode);
#endif
    RemoveBodyAndHead(contextAsNode);
#if DEBUG_barry || DEBUG_Barry
    DumpNode(contextAsNode);
#endif
    res = FindTargetNode(contextAsNode, contextLeaf);
    if (res == NS_FOUND_TARGET)
      res = NS_OK;
    NS_ENSURE_SUCCESS(res, res);
  }

  // get the tagstack for the context
  res = CreateTagStack(tagStack, contextLeaf);
#if DEBUG_barry || DEBUG_Barry
  dumpTagStack(tagStack);
#endif
  NS_ENSURE_SUCCESS(res, res);

  // create fragment for pasted html
  res = ParseFragment(aInputString, tagStack, doc, outFragNode);
#if DEBUG_barry || DEBUG_Barry
  dumpTagStack(tagStack);
  DumpNode(*outFragNode);
#endif
  NS_ENSURE_SUCCESS(res, res);
  NS_ENSURE_TRUE(*outFragNode, NS_ERROR_FAILURE);
  RemoveContextNodes(tagStack, *outFragNode);
  RemoveBodyAndHead(*outFragNode);
// #if DEBUG_barry || DEBUG_Barry
//   DumpNode(*outFragNode, 0, true);
// #endif
// #if DEBUG_barry || DEBUG_Barry
//   printf("Calling FixMath: +++++++++++\n");
//   DumpNode(*outFragNode, 0, true);
// #endif
 // FixMathematics(*outFragNode, PR_FALSE, PR_FALSE, PR_FALSE);
// #if DEBUG_barry || DEBUG_Barry
//   printf("Out of FixMath: ------------\n");
//   DumpNode(*outFragNode, 0, true);
// #endif
  if (contextAsNode)
  {
    // unite the two trees
    contextLeaf->AppendChild(*outFragNode, getter_AddRefs(junk));
    *outFragNode = contextAsNode;
  }
// #if DEBUG_barry || DEBUG_Barry
//   printf("contextAsNode: ------------\n");
//   DumpNode(contextAsNode, 0, true);
// #endif

  res = StripFormattingNodes(*outFragNode, PR_TRUE);
  NS_ENSURE_SUCCESS(res, res);

//  FixMathematics(contextAsNode, PR_FALSE, PR_FALSE, PR_FALSE);
// #if DEBUG_barry || DEBUG_Barry
//   printf("contextAsNode: ------------\n");
//   DumpNode(contextAsNode);
// #endif
  // If there was no context, then treat all of the data we did get as the
  // pasted data.
  if (contextLeaf)
    *outEndNode = *outStartNode = contextLeaf;
  else
    *outEndNode = *outStartNode = *outFragNode;

  *outStartOffset = 0;

  // get the infoString contents
  nsAutoString numstr1, numstr2;
  PRBool useNumStr = PR_TRUE;
  if (useNumStr) {

    if (!aInfoStr.IsEmpty())
    {
      PRInt32 err, sep, num;
      sep = aInfoStr.FindChar((PRUnichar)',');
      numstr1 = Substring(aInfoStr, 0, sep);
      numstr2 = Substring(aInfoStr, sep+1, aInfoStr.Length() - (sep+1));

      // Move the start and end children.
      num = numstr1.ToInteger(&err);
      while (num--)
      {
        (*outStartNode)->GetFirstChild(getter_AddRefs(tmp));
        if (!tmp)
          return NS_ERROR_FAILURE;
        if (nsHTMLEditUtils::IsMath(*outStartNode) || nsHTMLEditUtils::IsMath(tmp)) {
          num = 0;
        }
        else {
          tmp.swap(*outStartNode);
        }
      }

      num = numstr2.ToInteger(&err);
      while (num--)
      {
        (*outEndNode)->GetLastChild(getter_AddRefs(tmp));
        if (!tmp)
          return NS_ERROR_FAILURE;
        if (nsHTMLEditUtils::IsMath(*outStartNode) || nsHTMLEditUtils::IsMath(tmp)) {
          num = 0;
        }
        else {
          tmp.swap(*outEndNode);
        }
      }
    }
  }
  GetLengthOfDOMNode(*outEndNode, (PRUint32&)*outEndOffset);
  return res;
}


nsresult nsHTMLEditor::ParseFragment(const nsAString & aFragStr,
                                     nsTArray<nsAutoString> &aTagStack,
                                     nsIDocument* aTargetDocument,
                                     nsCOMPtr<nsIDOMNode> *outNode)
{
  // figure out if we are parsing full context or not
  PRBool bContext = aTagStack.IsEmpty();

  // create the parser to do the conversion.
  nsresult res;
  nsCOMPtr<nsIParser> parser = do_CreateInstance(kCParserCID, &res);
  NS_ENSURE_SUCCESS(res, res);
  NS_ENSURE_TRUE(parser, NS_ERROR_FAILURE);

  // create the html fragment sink
  nsCOMPtr<nsIContentSink> sink;
  // BBM At one time, a long time ago, the first call was to NS_XMLFRAGMENTSINK_CONTRACTID
  // if (bContext)
  //   sink = do_CreateInstance(NS_XMLFRAGMENTSINK2_CONTRACTID);
  // else
    sink = do_CreateInstance(NS_XMLFRAGMENTSINK2_CONTRACTID);

  NS_ENSURE_TRUE(sink, NS_ERROR_FAILURE);
  nsCOMPtr<nsIFragmentContentSink> fragSink(do_QueryInterface(sink));
  NS_ENSURE_TRUE(fragSink, NS_ERROR_FAILURE);

  fragSink->SetTargetDocument(aTargetDocument);

  // parse the fragment
  parser->SetContentSink(sink);
  if (bContext) {
    res = parser->Parse(aFragStr, (void*)0, NS_LITERAL_CSTRING("text/xml"), PR_TRUE, eDTDMode_fragment);
    if (NS_FAILED(res)) {
      res = parser->Parse(aFragStr, (void*)0, NS_LITERAL_CSTRING("text/html"), PR_TRUE, eDTDMode_fragment);
    }
  }
  else {
    res = parser->ParseFragment(aFragStr, 0, aTagStack, PR_TRUE, NS_LITERAL_CSTRING("text/xml"), eDTDMode_quirks);
    if (NS_FAILED(res)) res = parser->ParseFragment(aFragStr, 0, aTagStack, PR_TRUE, NS_LITERAL_CSTRING("text/html"), eDTDMode_quirks);
  }
  // get the fragment node
  nsCOMPtr<nsIDOMDocumentFragment> contextfrag;
  res = fragSink->GetFragment(getter_AddRefs(contextfrag));
  NS_ENSURE_SUCCESS(res, res);
  *outNode = do_QueryInterface(contextfrag);

  return res;
}

nsresult nsHTMLEditor::CreateTagStack(nsTArray<nsAutoString> &aTagStack, nsIDOMNode *aNode)
{
  nsresult res = NS_OK;
  nsCOMPtr<nsIDOMNode> node= aNode;
  PRBool bSeenBody = PR_FALSE;

  while (node)
  {
    if (nsTextEditUtils::IsBody(node))
      bSeenBody = PR_TRUE;
    nsCOMPtr<nsIDOMNode> temp = node;
    PRUint16 nodeType;

    node->GetNodeType(&nodeType);
    if (nsIDOMNode::ELEMENT_NODE == nodeType)
    {
      nsAutoString* tagName = aTagStack.AppendElement();
      NS_ENSURE_TRUE(tagName, NS_ERROR_OUT_OF_MEMORY);

//       nsAutoString attrnameStr;
//       nsAutoString prefixStr;
//       nsAutoString valueStr;
//       nsCOMPtr<nsIDOMElement> element;
//       nsCOMPtr<nsIContent> content;
// //      nsCOMPtr<nsIDOMNamedNodeMap> attributeMap;
//
//       element = do_QueryInterface(node);
//       content = do_QueryInterface(node);
      node->GetNodeName(*tagName);
      //BBM experimental: add attributes also.
//       count = content->GetAttrCount();
//       for (index = count; index > 0; )
//       {
//         --index;
//         const nsAttrName* name = content->GetAttrNameAt(index);
//         PRInt32 namespaceID = name->NamespaceID();
//         nsIAtom* attrName = name->LocalName();
//         nsIAtom* prefixName = name->GetPrefix();
//
//         attrName->ToString(attrnameStr);
//         if (prefixName) prefixName->ToString(prefixStr);
//         if (attrnameStr.EqualsLiteral("xmlns") && prefixStr.EqualsLiteral("xmlns"))
//           prefixStr.Truncate(0);
//         content->GetAttr(namespaceID, attrName, valueStr);
//         tagName->AppendLiteral(" ");
//         if (!prefixStr.IsEmpty())
//         {
//           tagName->Append(prefixStr);
//           tagName->AppendLiteral(":");
//         }
//         tagName->Append(attrnameStr);
//         tagName->AppendLiteral("=\"");
//         tagName->Append(valueStr);
//         tagName->AppendLiteral("\"");
//       }
    }

    res = temp->GetParentNode(getter_AddRefs(node));
    NS_ENSURE_SUCCESS(res, res);
  }

  if (!bSeenBody)
  {
      aTagStack.AppendElement(NS_LITERAL_STRING("body"));
  }
  return res;
}


nsresult nsHTMLEditor::CreateListOfNodesToPaste(nsIDOMNode  *aFragmentAsNode,
                                                nsCOMArray<nsIDOMNode>& outNodeList,
                                                nsIDOMNode *aStartNode,
                                                PRInt32 aStartOffset,
                                                nsIDOMNode *aEndNode,
                                                PRInt32 aEndOffset)
{
  if (!aFragmentAsNode)
    return NS_ERROR_NULL_POINTER;

  nsresult res;

  // if no info was provided about the boundary between context and stream,
  // then assume all is stream.
  if (!aStartNode)
  {
    PRInt32 fragLen;
    res = GetLengthOfDOMNode(aFragmentAsNode, (PRUint32&)fragLen);
    NS_ENSURE_SUCCESS(res, res);

    aStartNode = aFragmentAsNode;
    aStartOffset = 0;
    aEndNode = aFragmentAsNode;
    aEndOffset = fragLen;
  }

  nsCOMPtr<nsIDOMRange> docFragRange = do_CreateInstance("@mozilla.org/content/range;1");
  if (!docFragRange) return NS_ERROR_OUT_OF_MEMORY;

  res = docFragRange->SetStart(aStartNode, aStartOffset);
  NS_ENSURE_SUCCESS(res, res);
  res = docFragRange->SetEnd(aEndNode, aEndOffset);
  NS_ENSURE_SUCCESS(res, res);

  // now use a subtree iterator over the range to create a list of nodes
  nsTrivialFunctor functor;
  nsDOMSubtreeIterator iter;
  res = iter.Init(docFragRange);
  NS_ENSURE_SUCCESS(res, res);
  res = iter.AppendList(functor, outNodeList);

  return res;
}

nsresult
nsHTMLEditor::GetListAndTableParents(PRBool aEnd,
                                     nsCOMArray<nsIDOMNode>& aListOfNodes,
                                     nsCOMArray<nsIDOMNode>& outArray)
// Now checks for math nodes too
{
  PRInt32 listCount = aListOfNodes.Count();
  if (listCount <= 0)
    return NS_ERROR_FAILURE;  // no empty lists, please
  nsCOMPtr<nsIDOMNode> parent;

  // build up list of parents of first (or last) node in list
  // that are either lists, or tables.
  PRInt32 idx = 0;
  if (aEnd) idx = listCount-1;

  nsCOMPtr<nsIDOMNode>  pNode = aListOfNodes[idx];
  while (pNode)
  {
    if (nsHTMLEditUtils::IsList(pNode, mtagListManager) || nsHTMLEditUtils::IsTable(pNode, mtagListManager)
       )
      // ||
      // nsHTMLEditUtils::IsMath(pNode))
    {
      if (!outArray.AppendObject(pNode))
      {
        return NS_ERROR_FAILURE;
      }
    }
    pNode->GetParentNode(getter_AddRefs(parent));
    pNode = parent;
  }
  return NS_OK;
}

nsresult
nsHTMLEditor::DiscoverPartialListsAndTables(nsCOMArray<nsIDOMNode>& aPasteNodes,
                                            nsCOMArray<nsIDOMNode>& aListsAndTables,
                                            PRInt32 *outHighWaterMark)
{
  NS_ENSURE_TRUE(outHighWaterMark, NS_ERROR_NULL_POINTER);

  *outHighWaterMark = -1;
  PRInt32 listAndTableParents = aListsAndTables.Count();

  // scan insertion list for table elements (other than table).
  PRInt32 listCount = aPasteNodes.Count();
  PRInt32 j;
  for (j=0; j<listCount; j++)
  {
    nsCOMPtr<nsIDOMNode> curNode = aPasteNodes[j];

    NS_ENSURE_TRUE(curNode, NS_ERROR_FAILURE);
    if (nsHTMLEditUtils::IsTableElement(curNode, mtagListManager) && !nsHTMLEditUtils::IsTable(curNode, mtagListManager))
    {
      nsCOMPtr<nsIDOMNode> theTable = GetTableParent(curNode);
      if (theTable)
      {
        PRInt32 indexT = aListsAndTables.IndexOf(theTable);
        if (indexT >= 0)
        {
          *outHighWaterMark = indexT;
          if (*outHighWaterMark == listAndTableParents-1) break;
        }
        else
        {
          break;
        }
      }
    }
    if (nsHTMLEditUtils::IsListItem(curNode, mtagListManager))
    {
      nsCOMPtr<nsIDOMNode> theList = GetListParent(curNode);
      if (theList)
      {
        PRInt32 indexL = aListsAndTables.IndexOf(theList);
        if (indexL >= 0)
        {
          *outHighWaterMark = indexL;
          if (*outHighWaterMark == listAndTableParents-1) break;
        }
        else
        {
          break;
        }
      }
    }
    if (nsHTMLEditUtils::IsMath(curNode))
    {
      nsCOMPtr<nsIDOMNode> theMath = GetMathParent(curNode);
      if (theMath)
      {
        PRInt32 indexL = aListsAndTables.IndexOf(theMath);
        if (indexL >= 0)
        {
          *outHighWaterMark = indexL;
          if (*outHighWaterMark == listAndTableParents-1) break;
        }
        else
        {
          break;
        }
      }
    }
  }
  return NS_OK;
}

nsresult
nsHTMLEditor::ScanForListAndTableStructure( PRBool aEnd,
                                            nsCOMArray<nsIDOMNode>& aNodes,
                                            nsIDOMNode *aListOrTable,
                                            nsCOMPtr<nsIDOMNode> *outReplaceNode)
{
  NS_ENSURE_TRUE(aListOrTable, NS_ERROR_NULL_POINTER);
  // aListOrTable can be a list, table, or math
  NS_ENSURE_TRUE(outReplaceNode, NS_ERROR_NULL_POINTER);

  *outReplaceNode = 0;

  // look upward from first/last paste node for a piece of this list/table
  PRInt32 listCount = aNodes.Count(), idx = 0;
  if (aEnd) idx = listCount-1;
  PRBool bList = nsHTMLEditUtils::IsList(aListOrTable, mtagListManager);
  PRBool bMath = nsHTMLEditUtils::IsMath(aListOrTable);

  nsCOMPtr<nsIDOMNode>  pNode = aNodes[idx];
  nsCOMPtr<nsIDOMNode>  originalNode = pNode;
  while (pNode)
  {
    if ( (bList && nsHTMLEditUtils::IsListItem(pNode, mtagListManager)) ||
      (!bList && /*!bMath && */(nsHTMLEditUtils::IsTableElement(pNode, mtagListManager) && !nsHTMLEditUtils::IsTable(pNode, mtagListManager)))
        || (bMath && nsHTMLEditUtils::IsMath(pNode)) )
    {
      nsCOMPtr<nsIDOMNode> structureNode;
      if (bList) structureNode = GetListParent(pNode);
      else structureNode = GetTableParent(pNode);
      // mtable parts are to be treated as tables parts rather than math parts, so we call the above line first.
      if ((!structureNode) && bMath) structureNode = GetMathParent(pNode);
      if (structureNode == aListOrTable)
      {
//        if (bList)
          *outReplaceNode = structureNode;
//        else if (bMath)
//          *outReplaceNode = structureNode;
//        else
//          *outReplaceNode = structureNode; // BBM- this seems really strange to me -- pNode;
        break;
      }
    }
    nsCOMPtr<nsIDOMNode> parent;
    pNode->GetParentNode(getter_AddRefs(parent));
    pNode = parent;
  }
  return NS_OK;
}


nsresult
nsHTMLEditor::ReplaceOrphanedMath(PRBool aEnd,
                                       nsCOMArray<nsIDOMNode>& aNodeArray,
                                       nsIDOMNode* mathParent)
{
  return NS_OK;
  if (!mathParent) return NS_OK;
  nsCOMPtr<nsIDOMNode> endpoint;
  do {
    endpoint = GetArrayEndpoint(aEnd, aNodeArray);
    if (!endpoint) break;
    if (nsEditorUtils::IsDescendantOf(endpoint, mathParent))
      aNodeArray.RemoveObject(endpoint);
    else
      break;
  } while (endpoint);
  if (aEnd) aNodeArray.AppendObject(mathParent);
  else aNodeArray.InsertObjectAt(mathParent, 0);
  return NS_OK;
}


nsresult
nsHTMLEditor::ReplaceOrphanedStructure(PRBool aEnd,
                                       nsCOMArray<nsIDOMNode>& aNodeArray,
                                       nsCOMArray<nsIDOMNode>& aListAndTableArray,
                                       PRInt32 aHighWaterMark)
{
  nsCOMPtr<nsIDOMNode> curNode = aListAndTableArray[aHighWaterMark];
  NS_ENSURE_TRUE(curNode, NS_ERROR_NULL_POINTER);

  nsCOMPtr<nsIDOMNode> replaceNode, originalNode;

  // find substructure of list or table that must be included in paste.
  nsresult res = ScanForListAndTableStructure(aEnd, aNodeArray,
                                 curNode, address_of(replaceNode));
  NS_ENSURE_SUCCESS(res, res);

  // if we found substructure, paste it instead of its descendants
  if (replaceNode)
  {
    // postprocess list to remove any descendants of this node
    // so that we don't insert them twice.
    nsCOMPtr<nsIDOMNode> endpoint;
    do
    {
      endpoint = GetArrayEndpoint(aEnd, aNodeArray);
      if (!endpoint) break;
      if (nsEditorUtils::IsDescendantOf(endpoint, replaceNode))
        aNodeArray.RemoveObject(endpoint);
      else
        break;
    } while(endpoint);

    // now replace the removed nodes with the structural parent
    if (aEnd) aNodeArray.AppendObject(replaceNode);
    else aNodeArray.InsertObjectAt(replaceNode, 0);
  }
  return NS_OK;
}

nsIDOMNode* nsHTMLEditor::GetArrayEndpoint(PRBool aEnd,
                                           nsCOMArray<nsIDOMNode>& aNodeArray)
{
  PRInt32 listCount = aNodeArray.Count();
  if (listCount <= 0)
    return nsnull;

  if (aEnd)
  {
    return aNodeArray[listCount-1];
  }

  return aNodeArray[0];
}

nsresult
nsHTMLEditor::CreateFrameWithDefaults(const nsAString & frametype, PRBool insertImmediately, nsIDOMNode * parent, PRInt32 offset, nsIDOMElement **_retval)
{
  nsCOMPtr<nsIDOMNode> framenode;
  nsCOMPtr<nsIDOMElement> frame;
  nsCOMPtr<nsIDOMDocument> domDoc;
  nsresult rv = NS_OK;
  if (insertImmediately) {
    CreateNode(NS_LITERAL_STRING("msiframe"), parent, offset, getter_AddRefs(framenode));
    frame = do_QueryInterface(framenode);
  } else {
    GetDocument(getter_AddRefs(domDoc));
    domDoc->CreateElement(NS_LITERAL_STRING("msiframe"),getter_AddRefs(frame));
  }
  if (!frame) return NS_ERROR_FAILURE;
  frame->SetAttribute(NS_LITERAL_STRING("frametype"), frametype);
  frame->SetAttribute(NS_LITERAL_STRING("req"), NS_LITERAL_STRING("wrapfig"));
  ApplyGraphicsDefaults( frame, frametype );
  frame->SetAttribute(NS_LITERAL_STRING("aspect"), frametype.EqualsLiteral("textframe") ? NS_LITERAL_STRING("false") : NS_LITERAL_STRING("true"));
  frame->SetAttribute(NS_LITERAL_STRING("textalignment"), NS_LITERAL_STRING("left"));
  *_retval = frame;
  return rv;
}
















































